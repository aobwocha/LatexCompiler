#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "latex_compiler.h"

void print_json_string(const char *str, FILE *out) {
    fputc('"', out);
    if (str) {
        for (size_t i = 0; str[i] != '\0'; i++) {
            switch (str[i]) {
                case '\\': fprintf(out, "\\\\"); break;
                case '"':  fprintf(out, "\\\""); break;
                case '\b': fprintf(out, "\\b"); break;
                case '\f': fprintf(out, "\\f"); break;
                case '\n': fprintf(out, "\\n"); break;
                case '\r': fprintf(out, "\\r"); break;
                case '\t': fprintf(out, "\\t"); break;
                default:
                    if ((unsigned char)str[i] < 0x20) {
                        fprintf(out, "\\u%04x", (unsigned char)str[i]);
                    } else {
                        fputc(str[i], out);
                    }
                    break;
            }
        }
    }
    fputc('"', out);
}

void print_indent(int indent, FILE *out) {
    for (int i = 0; i < indent; i++) fprintf(out, "  ");
}

void print_node_list_json(const NodeList *list, int indent, FILE *out) {
    fprintf(out, "[\n");
    for (size_t i = 0; i < list->count; i++) {
        print_ast_json(list->data[i], indent + 1, out);
        if (i + 1 < list->count) {
            fprintf(out, ",");
        }
        fprintf(out, "\n");
    }
    print_indent(indent, out);
    fprintf(out, "]");
}

void print_ast_json(const Node *node, int indent, FILE *out) {
    if (!node) {
        fprintf(out, "null");
        return;
    }

    print_indent(indent, out);
    fprintf(out, "{\n");

    switch (node->type) {
    case NODE_TEXT:
        print_indent(indent + 1, out);
        fprintf(out, "\"type\": \"TextNode\",\n");
        print_indent(indent + 1, out);
        fprintf(out, "\"value\": ");
        print_json_string(node->as.text.value, out);
        fprintf(out, "\n");
        break;

    case NODE_COMMAND:
        print_indent(indent + 1, out);
        fprintf(out, "\"type\": \"CommandNode\",\n");
        print_indent(indent + 1, out);
        fprintf(out, "\"name\": ");
        print_json_string(node->as.command.name, out);
        fprintf(out, ",\n");
        
        print_indent(indent + 1, out);
        fprintf(out, "\"args\": [\n");
        for (size_t i = 0; i < node->as.command.arg_count; i++) {
            CommandArg arg = node->as.command.args[i];
            print_indent(indent + 2, out);
            fprintf(out, "{\n");
            
            print_indent(indent + 3, out);
            fprintf(out, "\"type\": %s,\n", arg.type == ARG_OPTIONAL ? "\"optional\"" : "\"required\"");
            
            print_indent(indent + 3, out);
            fprintf(out, "\"children\": ");
            print_node_list_json(&arg.children, indent + 3, out);
            fprintf(out, "\n");
            
            print_indent(indent + 2, out);
            fprintf(out, "}%s\n", (i + 1 < node->as.command.arg_count) ? "," : "");
        }
        print_indent(indent + 1, out);
        fprintf(out, "]\n");
        break;

    case NODE_GROUP:
        print_indent(indent + 1, out);
        fprintf(out, "\"type\": \"GroupNode\",\n");
        print_indent(indent + 1, out);
        fprintf(out, "\"children\": ");
        print_node_list_json(&node->as.group.children, indent + 1, out);
        fprintf(out, "\n");
        break;

    case NODE_ENVIRONMENT:
        print_indent(indent + 1, out);
        fprintf(out, "\"type\": \"EnvironmentNode\",\n");
        print_indent(indent + 1, out);
        fprintf(out, "\"name\": ");
        print_json_string(node->as.environment.name, out);
        fprintf(out, ",\n");

        print_indent(indent + 1, out);
        fprintf(out, "\"opt_args\": ");
        print_node_list_json(&node->as.environment.opt_args, indent + 1, out);
        fprintf(out, ",\n");

        print_indent(indent + 1, out);
        fprintf(out, "\"children\": ");
        print_node_list_json(&node->as.environment.children, indent + 1, out);
        fprintf(out, "\n");
        break;
    }

    print_indent(indent, out);
    fprintf(out, "}");
}

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

    const char *output_file = "parser_output.json";
    FILE *out = fopen(output_file, "w");
    if (!out) {
        perror("Failed to open parser_output.json for writing");
        out = stdout;
    }

    fprintf(out, "{\n  \"type\": \"RootNode\",\n  \"children\": ");
    print_node_list_json(&ast_nodes, 1, out);
    fprintf(out, "\n}\n");

    if (out != stdout) {
        fclose(out);
    }

    for (size_t i = 0; i < ast_nodes.count; i++) {
        free_node(ast_nodes.data[i]);
    }
    free(ast_nodes.data);
    free_tokens(&token_list);

    return 0;
}