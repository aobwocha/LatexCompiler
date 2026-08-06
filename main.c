#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "latex_compiler.h"

static void print_escaped_string(const char *str) {
    putchar('"');
    if (str) {
        for (size_t i = 0; str[i] != '\0'; i++) {
            if (str[i] == '\n')      printf("\\n");
            else if (str[i] == '"')  printf("\\\"");
            else                     putchar(str[i]);
        }
    }
    putchar('"');
}

void print_ast(const Node *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");

    switch (node->type) {
    case NODE_TEXT:
        printf("TextNode: ");
        print_escaped_string(node->as.text.value);
        printf("\n");
        break;
    case NODE_COMMAND:
        printf("CommandNode: %s\n", node->as.command.name ? node->as.command.name : "");
        if (node->as.command.opt_args.count > 0) {
            for (int i = 0; i < indent; i++) printf("  ");
            printf("  OptArgs [...]:\n");
            for (size_t i = 0; i < node->as.command.opt_args.count; i++) {
                print_ast(node->as.command.opt_args.data[i], indent + 2);
            }
        }
        if (node->as.command.req_args.count > 0) {
            for (int i = 0; i < indent; i++) printf("  ");
            printf("  ReqArgs {...}:\n");
            for (size_t i = 0; i < node->as.command.req_args.count; i++) {
                print_ast(node->as.command.req_args.data[i], indent + 2);
            }
        }
        break;
    case NODE_GROUP:
        printf("GroupNode {...}:\n");
        for (size_t i = 0; i < node->as.group.children.count; i++) {
            print_ast(node->as.group.children.data[i], indent + 1);
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

    Parser parser = new_parser(token_list.tokens, token_list.count);
    NodeList ast_nodes = parse_node_list(&parser);

    for (size_t i = 0; i < ast_nodes.count; i++) {
        printf("[%zu] ", i);
        print_ast(ast_nodes.data[i], 0);
    }

    for (size_t i = 0; i < ast_nodes.count; i++) {
        free_node(ast_nodes.data[i]);
    }
    free(ast_nodes.data);
    free_tokens(&token_list);

    return 0;
}