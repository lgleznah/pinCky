#include "tokens.h"

#include <stdio.h>
#include <string.h>

void print_token(const token* token)
{
    char buff[256];
    memcpy_s(buff, token->length, token->start, token->length);
    buff[token->length] = '\0';
    printf("(%s, %s) (line %d, column %d)\n", token_type_str[token->type], buff, token->line, token->column);
}