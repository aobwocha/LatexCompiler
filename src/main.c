#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "latex_compiler.h"

static void print_escaped_string(const char *str, FILE *out) {
    fputc('"', out);
    if (str) {
        for (size_t i = 0; str[i] != '\0'; i++) {
            if (str[i] == '\n')      fprintf(out, "\\n");
            else if (str[i] == '"')  fprintf(out, "\\\"");
            else                     fputc(str[i], out);
        }
    }
    fputc('"', out);
}

void print_tokens(const TokenList *list, FILE *out) {
    if (!list || !list->tokens) return;
    for (size_t i = 0; i < list->count; i++) {
        Token t = list->tokens[i];
        fprintf(out, "[Type: %d] \"", t.type);
        if (t.value) {
            for (size_t j = 0; t.value[j] != '\0'; j++) {
                if (t.value[j] == '\n') fprintf(out, "\\n");
                else fputc(t.value[j], out);
            }
        }
        fprintf(out, "\"\n");
    }
}

void print_ast(const Node *node, int indent, FILE *out) {
    if (!node) return;
    for (int i = 0; i < indent; i++) fprintf(out, "  ");

    switch (node->type) {
    case NODE_TEXT:
        fprintf(out, "TextNode: ");
        print_escaped_string(node->as.text.value, out);
        fprintf(out, "\n");
        break;
    case NODE_COMMAND:
        fprintf(out, "CommandNode: %s\n", node->as.command.name ? node->as.command.name : "");
        if (node->as.command.opt_args.count > 0) {
            for (int i = 0; i < indent; i++) fprintf(out, "  ");
            fprintf(out, "  OptArgs [...]:\n");
            for (size_t i = 0; i < node->as.command.opt_args.count; i++) {
                print_ast(node->as.command.opt_args.data[i], indent + 7, out);
            }
        }
        if (node->as.command.req_args.count > 0) {
            for (int i = 0; i < indent; i++) fprintf(out, "  ");
            fprintf(out, "  ReqArgs {...}:\n");
            for (size_t i = 0; i < node->as.command.req_args.count; i++) {
                print_ast(node->as.command.req_args.data[i], indent + 7, out);
            }
        }
        break;
    case NODE_GROUP:
        fprintf(out, "GroupNode {...}:\n");
        for (size_t i = 0; i < node->as.group.children.count; i++) {
            print_ast(node->as.group.children.data[i], indent + 1, out);
        }
        break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Pass LaTEX file\n");
        return 0;
    }

    int lex_err = 0;
    TokenList token_list = lex(argv[1], &lex_err);
    if (lex_err) return 1;

    FILE *lex_out = fopen("lexer_output.txt", "w");
    if (lex_out) {
        print_tokens(&token_list, lex_out);
        fclose(lex_out);
    } else {
        printf("Failed to open lexer_output.txt for writing.\n");
    }

    Parser parser = new_parser(token_list.tokens, token_list.count);
    NodeList ast_nodes = parse_node_list(&parser);

    FILE *ast_out = fopen("parser_output.txt", "w");
    if (ast_out) {
        for (size_t i = 0; i < ast_nodes.count; i++) {
            fprintf(ast_out, "[%zu] ", i);
            print_ast(ast_nodes.data[i], 0, ast_out);
        }
        fclose(ast_out);
    } else {
        printf("Failed to open parser_output.txt for writing.\n");
    }

    for (size_t i = 0; i < ast_nodes.count; i++) {
        free_node(ast_nodes.data[i]);
    }
    free(ast_nodes.data);
    free_tokens(&token_list);

    return 0;
}