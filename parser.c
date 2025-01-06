#include "parser.h"

#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"


void init_parser(parser* parser, token_array* tokens)
{
    parser->tokens = tokens;
    parser->curr_token = 0;
    init_ast_array(&parser->ast_array, 65535);
}

void free_parser(parser* parser)
{
    free_ast_array(&parser->ast_array);
}

void print_ast(const void* node)
{
    print_element(node, 0);
}

// Helper functions

// Consume current token and advance to the next one
token* parser_advance(parser* parser) {
    return &parser->tokens->data[parser->curr_token++];
}

// Return current token
token* parser_peek(const parser* parser)
{
    return &parser->tokens->data[parser->curr_token];
}

// Check if next token matches the expected type
int parser_is_next(const parser* parser, const int token_type)
{
    if (parser->curr_token >= parser->tokens->size)
    {
        return 0;
    }
    return parser->tokens->data[parser->curr_token+1].type == token_type;
}

// Test if current token is expected type; otherwise error-out
token* parser_expect(parser* parser, const int token_type)
{
    if (parser->curr_token >= parser->tokens->size)
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(parser_peek(parser)->line, "Found token '%s' at end of parsing", token_type_str[token_type]);
    }
    if (parser->tokens->data[parser->curr_token].type != token_type)
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(parser_peek(parser)->line, "Expected token '%s'\n", token_type_str[token_type]);
    }
    return parser_advance(parser);
}

// Check if current token matches expected type and return it, otherwise return NULL
token* parser_match(parser* parser, const int token_type)
{
    if (parser->curr_token >= parser->tokens->size)
    {
        return NULL;
    }
    token* token = &parser->tokens->data[parser->curr_token];
    if (token->type == token_type)
    {
        parser->curr_token++;
        return token;
    }
    return NULL;
}

// Return previous token
token* parser_previous_token(const parser* parser)
{
    return &parser->tokens->data[parser->curr_token-1];
}

// Parser functions
void* parse(parser* parser)
{
    void* ast = expr(parser);
    return ast;
}

// <expr> ::= <term> (('+' | '-') <term> )*
void* expr(parser* parser)
{
    void* expr = term(parser);
    while(parser_match(parser, TOK_PLUS) || parser_match(parser, TOK_MINUS))
    {
        token* op = parser_previous_token(parser);
        void* right = term(parser);
        BinOp* bin_op = allocate_ast_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(bin_op, op->type, expr, right, op->line);
        expr = bin_op;
    }
    return expr;
}

// <term> ::= <factor> (('*' | '/') <factor> )*
void* term(parser* parser)
{
    void* expr = factor(parser);
    while(parser_match(parser, TOK_STAR) || parser_match(parser, TOK_SLASH))
    {
        token* op = parser_previous_token(parser);
        void* right = factor(parser);
        BinOp* bin_op = allocate_ast_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(bin_op, op->type, expr, right, op->line);
        expr = bin_op;
    }
    return expr;
}

// <factor> ::= <unary>
void* factor(parser* parser)
{
    return unary(parser);
}

// <unary> ::= ('+'|'-'|'~') <unary>  |  <primary>
void* unary(parser* parser)
{
    if (parser_match(parser, TOK_NOT) || parser_match(parser, TOK_PLUS) || parser_match(parser, TOK_MINUS))
    {
        token* op = parser_previous_token(parser);
        void* operand = unary(parser);
        UnOp* un_op = allocate_ast_array(&parser->ast_array, sizeof(UnOp));
        init_UnOp(un_op, op->type, operand, op->line);
        return un_op;
    }
    return primary(parser);
}

// <primary>  ::=  <integer> | <float> | <bool> | <string> | '(' <expr> ')'
void* primary(parser* parser)
{
    char num_string[128];
    if (parser_match(parser, TOK_INTEGER))
    {
        token* integer_token = parser_previous_token(parser);
        Integer* integer = allocate_ast_array(&parser->ast_array, sizeof(Integer));
        memcpy_s(num_string, 127, integer_token->start, integer_token->length);
        num_string[min(127, integer_token->length)] = '\0';
        init_Integer(integer, strtol(num_string, NULL, 10), integer_token->line);
        return integer;
    }
    if (parser_match(parser, TOK_FLOAT))
    {
        token* float_token = parser_previous_token(parser);
        Float* number = allocate_ast_array(&parser->ast_array, sizeof(Float));
        memcpy_s(num_string, 127, float_token->start, float_token->length);
        num_string[min(127, float_token->length)] = '\0';
        init_Float(number, strtod(num_string, NULL), float_token->line);
        return number;
    }
    if (parser_match(parser, TOK_TRUE))
    {
        token* bool_token = parser_previous_token(parser);
        Bool* boolean = allocate_ast_array(&parser->ast_array, sizeof(Bool));
        init_Bool(boolean, 1, bool_token->line);
        return boolean;
    }
    if (parser_match(parser, TOK_FALSE))
    {
        token* bool_token = parser_previous_token(parser);
        Bool* boolean = allocate_ast_array(&parser->ast_array, sizeof(Bool));
        init_Bool(boolean, 0, bool_token->line);
        return boolean;
    }
    if (parser_match(parser, TOK_STRING))
    {
        token* string_token = parser_previous_token(parser);
        String* string = allocate_ast_array(&parser->ast_array, sizeof(String));
        init_String(string, string_token->start+1, string_token->length-2, string_token->line);
        return string;
    }
    if (parser_match(parser, TOK_LPAREN))
    {
        void* expression = expr(parser);
        if (!parser_match(parser, TOK_RPAREN))
        {
            PRINT_SYNTAX_ERROR_AND_QUIT(GET_ELEMENT_LINE(expression), "Expected ')' after expression\n");
        }
        else
        {
            Grouping* grouping = allocate_ast_array(&parser->ast_array, sizeof(Grouping));
            init_Grouping(grouping, expression, GET_ELEMENT_LINE(expression));
            return grouping;
        }
    }
    else
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(parser_previous_token(parser)->line, "Expected number or '(', found '%s'\n", token_symbols[parser_previous_token(parser)->type]);
    }
}