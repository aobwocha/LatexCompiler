#include <stdio.h>
#include <stdlib.h>
#include "latex_compiler.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.tex>\n", argv[0]);
        return 1;
    }

    int lex_err = 0;
    TokenList token_list = lex(argv[1], &lex_err);
    if (lex_err) return 1;

    Parser parser = new_parser(token_list.tokens, token_list.count);
    NodeList ast_nodes = parse_node_list(&parser, -1, NULL);

    expand_macros(&ast_nodes);

    const char *output_file = "parser_output.md";
    FILE *out = fopen(output_file, "w");
    if (!out) {
        perror("Failed to open parser_output.md");
        out = stdout;
    }

    serialize_ast_markdown(&ast_nodes, out);

    if (out != stdout) {
        fclose(out);
        printf("Markdown output written to %s\n", output_file);
    }

    for (size_t i = 0; i < ast_nodes.count; i++) {
        free_node(ast_nodes.data[i]);
    }
    free(ast_nodes.data);
    free_tokens(&token_list);

    return 0;
}