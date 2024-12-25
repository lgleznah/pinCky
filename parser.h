#pragma once

#include "arrays.h"
#include "model.h"
#include "tokens.h"

typedef struct
{
    token_array* tokens;
    int curr_token;
    ast_array ast_array;
} parser;

void init_parser(parser* parser, token_array* tokens);
void free_parser(parser* parser);
void* parse(parser* parser);
void print_ast(void* node);

// Functions for AST parsing
void* expr(parser* parser);
void* term(parser* parser);
void* factor(parser* parser);
void* unary(parser* parser);
void* primary(parser* parser);