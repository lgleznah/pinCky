#pragma once

#include <stdio.h>

#include "arrays.h"

typedef struct
{
    token_array tokens;
    int start, curr, line, column;
    FILE* source;
    char* buffer;
    size_t length;
} lexer;

void init_lexer(lexer* lexer, FILE* file);
void free_lexer(lexer* lexer);

void tokenize(lexer* lexer);
char advance(lexer* lexer);
char peek(const lexer* lexer);
char lookahead(const lexer* lexer, int n);
int match(lexer* lexer, char expected);

void print_tokens(const lexer* lexer);