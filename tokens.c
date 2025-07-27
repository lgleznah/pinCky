#include "tokens.h"

#include <stdio.h>
#include <string.h>

void print_token(const token* token)
{
    char buff[256];
    memcpy(buff, token->token.string_value, token->token.length);
    buff[token->token.length] = '\0';
    printf("(%s, %s) (line %d, column %d)\n", token_type_str[token->type], buff, token->line, token->column);
}
