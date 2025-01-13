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
// Expression-parsing functions
void* expr(parser* parser);
void* or_logical(parser* parser);
void* and_logical(parser* parser);
void* equality(parser* parser);
void* comparison(parser* parser);
void* addition(parser* parser);
void* multiplication(parser* parser);
void* modulo(parser* parser);
void* exponent(parser* parser);
void* unary(parser* parser);
void* primary(parser* parser);

// Statement-parsing functions
void* program(parser* parser);
void* stmts(parser* parser);
void* stmt(parser* parser);