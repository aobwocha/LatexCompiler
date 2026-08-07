#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "latex_compiler.h"

static int has_tex_extension(const char *file_path) {
    const char *ext = strrchr(file_path, '.');
    return (ext && strcmp(ext, ".tex") == 0);
}

static void append_token(TokenList *list, Token token) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        list->tokens = realloc(list->tokens, list->capacity * sizeof(Token));
        if (!list->tokens) {
            perror("Memory allocation failed for tokens");
            exit(EXIT_FAILURE);
        }
    }
    list->tokens[list->count++] = token;
}

static char *strndup_custom(const char *s, size_t n) {
    char *dup = malloc(n + 1);
    if (dup) {
        memcpy(dup, s, n);
        dup[n] = '\0';
    }
    return dup;
}

void free_tokens(TokenList *list) {
    if (!list || !list->tokens) return;
    for (size_t i = 0; i < list->count; i++) {
        free(list->tokens[i].value);
    }
    free(list->tokens);
    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
}

TokenList lex(const char *file_path, int *out_error) {
    TokenList list = {NULL, 0, 0};
    *out_error = 0;

    if (!has_tex_extension(file_path)) {
        printf("Warning: File \"%s\" does not have a .tex extension\n", file_path);
    }

    FILE *latex_file = fopen(file_path, "rb");
    if (!latex_file) {
        fprintf(stderr, "Error reading file \"%s\"\n", file_path);
        *out_error = 1;
        return list;
    }

    fseek(latex_file, 0, SEEK_END);
    long length = ftell(latex_file);
    fseek(latex_file, 0, SEEK_SET);

    char *latex = malloc(length + 1);
    if (!latex) {
        fclose(latex_file);
        perror("Memory allocation failed for file buffer");
        *out_error = 1;
        return list;
    }

    size_t read_bytes = fread(latex, 1, length, latex_file);
    latex[read_bytes] = '\0';
    fclose(latex_file);

    int char_idx = 0;
    int line = 1;
    int col = 1;
    int total_len = (int)read_bytes;

    #define ADVANCE() do { \
        if (char_idx < total_len) { \
            if (latex[char_idx] == '\n') { \
                line++; \
                col = 1; \
            } else { \
                col++; \
            } \
            char_idx++; \
        } \
    } while(0)

    while (char_idx < total_len) {
        int start_idx = char_idx;
        Position start_pos = {line, col};
        char current_char = latex[char_idx];

        switch (current_char) {
        case '{':
            ADVANCE();
            append_token(&list, (Token){.type = TOKEN_LBRACE, .value = strdup("{"), .start_idx = start_idx, .end_idx = char_idx, .start_pos = start_pos, .end_pos = (Position){line, col}});
            break;
        case '}':
            ADVANCE();
            append_token(&list, (Token){.type = TOKEN_RBRACE, .value = strdup("}"), .start_idx = start_idx, .end_idx = char_idx, .start_pos = start_pos, .end_pos = (Position){line, col}});
            break;
        case '[':
            ADVANCE();
            append_token(&list, (Token){.type = TOKEN_LBRACKET, .value = strdup("["), .start_idx = start_idx, .end_idx = char_idx, .start_pos = start_pos, .end_pos = (Position){line, col}});
            break;
        case ']':
            ADVANCE();
            append_token(&list, (Token){.type = TOKEN_RBRACKET, .value = strdup("]"), .start_idx = start_idx, .end_idx = char_idx, .start_pos = start_pos, .end_pos = (Position){line, col}});
            break;
        case '%':
            while (char_idx < total_len && latex[char_idx] != '\n') {
                ADVANCE();
            }
            if (char_idx < total_len && latex[char_idx] == '\n') {
                ADVANCE();
            }
            break;
        case '\\':
            ADVANCE();
            if (char_idx < total_len) {
                if (isalpha((unsigned char)latex[char_idx])) {
                    while (char_idx < total_len && isalpha((unsigned char)latex[char_idx])) {
                        ADVANCE();
                    }
                } else {
                    ADVANCE();
                }
            }
            append_token(&list, (Token){.type = TOKEN_COMMAND, .value = strndup_custom(latex + start_idx, char_idx - start_idx), .start_idx = start_idx, .end_idx = char_idx, .start_pos = start_pos, .end_pos = (Position){line, col}});
            break;
        default:
            while (char_idx < total_len) {
                char c = latex[char_idx];
                if (c == '\\' || c == '{' || c == '}' || c == '[' || c == ']'|| c == '%') {
                    break;
                }
                ADVANCE();
            }
            append_token(&list, (Token){.type = TOKEN_TEXT, .value = strndup_custom(latex + start_idx, char_idx - start_idx), .start_idx = start_idx, .end_idx = char_idx, .start_pos = start_pos, .end_pos = (Position){line, col}});
            break;
        }
    }

    #undef ADVANCE
    free(latex);
    return list;
}