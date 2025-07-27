#pragma once

#include "arrays.h"
#include "model.h"
#include "tokens.h"

typedef struct
{
    token_array* tokens;
    int curr_token;
    vsd_array ast_array;
} parser;

void init_parser(parser* parser, token_array* tokens);
void free_parser(parser* parser);
void* parse(parser* parser);
void print_ast(const void* node);

// Functions for AST parsing
// Expression-parsing functions
size_t expr(parser* parser);
size_t or_logical(parser* parser);
size_t and_logical(parser* parser);
size_t equality(parser* parser);
size_t comparison(parser* parser);
size_t addition(parser* parser);
size_t multiplication(parser* parser);
size_t modulo(parser* parser);
size_t exponent(parser* parser);
size_t unary(parser* parser);
size_t primary(parser* parser);

// Statement-parsing functions
size_t program(parser* parser);
size_t stmts(parser* parser);
size_t stmt(parser* parser);
size_t print_stmt(parser* parser, char break_line);
size_t if_stmt(parser* parser);
size_t while_stmt(parser* parser);
size_t for_stmt(parser* parser);
size_t func_decl(parser* parser);
size_t ret_stmt(parser* parser);
