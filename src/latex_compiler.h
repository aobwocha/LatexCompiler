#ifndef LATEX_PARSER_H
#define LATEX_PARSER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

/* Lexer Types */

typedef enum {
    TOKEN_COMMAND,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_TEXT
} TokenType;

typedef struct {
    int line;
    int col;
} Position;

typedef struct {
    TokenType type;
    char *value;
    int start_idx;
    int end_idx;
    Position start_pos;
    Position end_pos;
} Token;

typedef struct {
    Token *tokens;
    size_t count;
    size_t capacity;
} TokenList;

/* Parser & AST Types */

typedef enum {
    NODE_TEXT,
    NODE_GROUP,
    NODE_COMMAND
} NodeType;

struct Node;

typedef struct {
    struct Node **data;
    size_t count;
    size_t capacity;
} NodeList;

typedef struct {
    char *value;
} TextNode;

typedef struct {
    NodeList children;
} GroupNode;

typedef struct {
    char *name;
    NodeList opt_args;
    NodeList req_args;
} CommandNode;

typedef struct Node {
    NodeType type;
    union {
        TextNode text;
        GroupNode group;
        CommandNode command;
    } as;
} Node;

typedef struct {
    const Token *tokens;
    size_t total_tokens;
    size_t pos;
} Parser;

// Lexer
TokenList lex(const char *file_path, int *out_error);
void free_tokens(TokenList *list);

// Parser
Parser new_parser(const Token *tokens, size_t count);
NodeList parse_node_list(Parser *p);
void free_node(Node *node);

// Utility
void print_ast(const Node *node, int indent, FILE *out);
void print_tokens(const TokenList *list, FILE *out);

#endif