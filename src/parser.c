#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "latex_compiler.h"

static void append_node(NodeList *list, Node *node) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        list->data = realloc(list->data, list->capacity * sizeof(Node*));
        if (!list->data) {
            perror("Memory allocation failed for NodeList");
            exit(EXIT_FAILURE);
        }
    }
    list->data[list->count++] = node;
}

void free_node(Node *node) {
    if (!node) return;
    switch (node->type) {
    case NODE_TEXT:
        free(node->as.text.value);
        break;
    case NODE_GROUP:
        for (size_t i = 0; i < node->as.group.children.count; i++) {
            free_node(node->as.group.children.data[i]);
        }
        free(node->as.group.children.data);
        break;
    case NODE_COMMAND:
        free(node->as.command.name);
        for (size_t i = 0; i < node->as.command.opt_args.count; i++) {
            free_node(node->as.command.opt_args.data[i]);
        }
        free(node->as.command.opt_args.data);
        for (size_t i = 0; i < node->as.command.req_args.count; i++) {
            free_node(node->as.command.req_args.data[i]);
        }
        free(node->as.command.req_args.data);
        break;
    }
    free(node);
}

Parser new_parser(const Token *tokens, size_t count) {
    return (Parser){.tokens = tokens, .total_tokens = count, .pos = 0};
}

static Token peek(Parser *p) {
    if (p->pos >= p->total_tokens) {
        return (Token){.type = TOKEN_TEXT, .value = ""};
    }
    return p->tokens[p->pos];
}

static Token advance(Parser *p) {
    Token tok = peek(p);
    if (p->pos < p->total_tokens) {
        p->pos++;
    }
    return tok;
}

static bool consume(Parser *p, TokenType expected) {
    Token tok = peek(p);
    if (tok.type != expected) {
        return false;
    }
    advance(p);
    return true;
}

static bool is_at_end(Parser *p) {
    return p->pos >= p->total_tokens;
}

static Node *parse_command(Parser *p);
static Node *parse_group(Parser *p);
static Node *parse_text(Parser *p);

NodeList parse_node_list(Parser *p) {
    NodeList nodes = {NULL, 0, 0};
    while (!is_at_end(p)) {
        TokenType tok_type = peek(p).type;
        if (tok_type == TOKEN_RBRACE || tok_type == TOKEN_RBRACKET) {
            break;
        }
        switch (tok_type) {
        case TOKEN_COMMAND: append_node(&nodes, parse_command(p)); break;
        case TOKEN_LBRACE:   append_node(&nodes, parse_group(p)); break;
        default:             append_node(&nodes, parse_text(p)); break;
        }
    }
    return nodes;
}

static Node *parse_command(Parser *p) {
    Token cmd_tok = advance(p);
    Node *node = malloc(sizeof(Node));
    node->type = NODE_COMMAND;
    node->as.command.name = strdup(cmd_tok.value);
    node->as.command.opt_args = (NodeList){NULL, 0, 0};
    node->as.command.req_args = (NodeList){NULL, 0, 0};

    if (!is_at_end(p) && peek(p).type == TOKEN_LBRACKET) {
        consume(p, TOKEN_LBRACKET);
        node->as.command.opt_args = parse_node_list(p);
        consume(p, TOKEN_RBRACKET);
    }
    if (!is_at_end(p) && peek(p).type == TOKEN_LBRACE) {
        consume(p, TOKEN_LBRACE);
        node->as.command.req_args = parse_node_list(p);
        consume(p, TOKEN_RBRACE);
    }
    return node;
}

static Node *parse_group(Parser *p) {
    consume(p, TOKEN_LBRACE);
    NodeList children = parse_node_list(p);
    consume(p, TOKEN_RBRACE);

    Node *node = malloc(sizeof(Node));
    node->type = NODE_GROUP;
    node->as.group.children = children;
    return node;
}

static Node *parse_text(Parser *p) {
    Token tok = advance(p);
    Node *node = malloc(sizeof(Node));
    node->type = NODE_TEXT;
    node->as.text.value = strdup(tok.value);
    return node;
}