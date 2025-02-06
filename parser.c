#include "parser.h"

#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"

#define OFFSET_PTR(ptr) ((void*)(ptr + (char*)(parser->ast_array.data)))

void init_parser(parser* parser, token_array* tokens)
{
    parser->tokens = tokens;
    parser->curr_token = 0;
    init_vsd_array(&parser->ast_array, 0);
}

void free_parser(parser* parser)
{
    free_vsd_array(&parser->ast_array);
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
    if (parser->curr_token >= parser->tokens->used)
    {
        return 0;
    }
    return parser->tokens->data[parser->curr_token+1].type == token_type;
}

// Return previous token
token* parser_previous_token(const parser* parser)
{
    return &parser->tokens->data[parser->curr_token-1];
}

// Test if current token is expected type; otherwise error-out
token* parser_expect(parser* parser, const int token_type)
{
    if (parser->curr_token >= parser->tokens->used)
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(parser_previous_token(parser)->line, "Found token '%s' at end of parsing", token_type_str[token_type]);
    }
    if (parser->tokens->data[parser->curr_token].type != token_type)
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(parser_peek(parser)->line, "Expected token '%s', but found '%s'\n", token_type_str[token_type], token_type_str[parser->tokens->data[parser->curr_token].type]);
    }
    return parser_advance(parser);
}

// Check if current token matches expected type and return it, otherwise return NULL
token* parser_match(parser* parser, const int token_type)
{
    if (parser->curr_token >= parser->tokens->used)
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

// Parser functions
void* parse(parser* parser)
{
    size_t ast = program(parser);
    compute_ptr((Element*)(ast + (char*)parser->ast_array.data), parser->ast_array.data);
    return ast + (char*)(parser->ast_array.data);
}

// <program> ::= <stmts>
size_t program(parser* parser)
{
    size_t result = stmts(parser);
    return result;
}

// <stmts> ::= <stmt>+
size_t stmts(parser* parser)
{
    // Statements are stored in an array as offsets, rather than pointers,
    // as AST reallocs might potentially invalidate any obtained pointer.
    // However, when the statement list is fully built, these offsets can be
    // converted into pointers, and the statements realloc update function will
    // update these properly.
    statement_array array;
    init_statement_array(&array, 65536);

    while (parser->curr_token < parser->tokens->used && !(parser_peek(parser)->type == TOK_ELSE) && !(parser_peek(parser)->type == TOK_END))
    {
        size_t result = stmt(parser);
        if (result == -1)
        {
            break;
        }
        insert_statement_array(&array, result);
    }

    if (array.used == 0)
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(parser_previous_token(parser)->line, "Empty statement list is not allowed");
    }

    size_t stmts = allocate_vsd_array(&parser->ast_array, sizeof(StatementList) + array.used * sizeof(void*));
    void* first_statement_ptr = OFFSET_PTR(array.data[0]);
    init_StatementList(OFFSET_PTR(stmts), &array, parser->ast_array.data, GET_ELEMENT_LINE(first_statement_ptr));
    
    free_statement_array(&array);
    return stmts;
}

// <stmt> ::= <expr> | <print> | <assign> | <local_assign> | <println> | <if> | <while> | <for> | <func_decl> | <func_call> | <ret>
size_t stmt(parser* parser)
{
    if (parser_peek(parser)->type == TOK_PRINT)
    {
        return print_stmt(parser, 0);
    }
    
    if (parser_peek(parser)->type == TOK_PRINTLN)
    {
        return print_stmt(parser, 1);
    }

    if (parser_peek(parser)->type == TOK_IF)
    {
        return if_stmt(parser);
    }

    // If none of the above, this statement is an assignment or a function call
    size_t lhs = expr(parser);
    if (parser_match(parser, TOK_ASSIGN))
    {
        int line = parser_previous_token(parser)->line;
        size_t rhs = expr(parser);
        size_t assignmnent = allocate_vsd_array(&parser->ast_array, sizeof(Assignment));
        init_Assignment(OFFSET_PTR(assignmnent), lhs, rhs, parser->ast_array.data, line);
        return assignmnent;
    }

    return -1;
}

// <print> ::= 'print' <expr>
size_t print_stmt(parser* parser, char break_line)
{
    if (parser_match(parser, TOK_PRINT) || parser_match(parser, TOK_PRINTLN))
    {
        size_t result = expr(parser);
        size_t print = allocate_vsd_array(&parser->ast_array, sizeof(Print));
        init_Print(OFFSET_PTR(print), break_line, result, parser->ast_array.data, parser_previous_token(parser)->line);
        return print;
    }

    return -1;
}

// <if> ::= 'if' <expr> 'then' <stmts> ('else' <stmts>)? 'end'
size_t if_stmt(parser* parser)
{
    parser_expect(parser, TOK_IF);
    size_t condition = expr(parser);
    parser_expect(parser, TOK_THEN);
    size_t then_stmt = stmts(parser);

    size_t else_stmt = -1;
    if (parser_peek(parser)->type == TOK_ELSE)
    {
        parser_expect(parser, TOK_ELSE);
        else_stmt = stmts(parser);
    }

    parser_expect(parser, TOK_END);

    size_t result = allocate_vsd_array(&parser->ast_array, sizeof(If));
    void* condition_ptr = OFFSET_PTR(condition);
    init_If(OFFSET_PTR(result), condition, then_stmt, else_stmt, parser->ast_array.data, GET_ELEMENT_LINE(condition_ptr));
    return result;
}

// <expr> ::= <or_logical>
size_t expr(parser* parser)
{
    return or_logical(parser);
}

// <or_logical> ::= <and_logical> ('or' <and_logical>)*
size_t or_logical(parser* parser)
{
    size_t result = and_logical(parser);
    while(parser_match(parser, TOK_OR))
    {
        size_t right = and_logical(parser);
        size_t bin_op = allocate_vsd_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(OFFSET_PTR(bin_op), TOK_OR, result, right, parser->ast_array.data, parser_previous_token(parser)->line);
        result = bin_op;
    }
    return result;
}

// <and_logical> ::= <equality> ('and' <equality>)*
size_t and_logical(parser* parser)
{
    size_t result = equality(parser);
    while(parser_match(parser, TOK_AND))
    {
        size_t right = equality(parser);
        size_t bin_op = allocate_vsd_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(OFFSET_PTR(bin_op), TOK_AND, result, right, parser->ast_array.data, parser_previous_token(parser)->line);
        result = bin_op;
    }
    return result;
}

// <equality> ::= <comparison> (('!=' | '==') <comparison>)*
size_t equality(parser* parser)
{
    size_t result = comparison(parser);
    while(parser_match(parser, TOK_NE) || parser_match(parser, TOK_EQEQ))
    {
        token* op = parser_previous_token(parser);
        size_t right = comparison(parser);
        size_t bin_op = allocate_vsd_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(OFFSET_PTR(bin_op), op->type, result, right, parser->ast_array.data, op->line);
        result = bin_op;
    }
    return result;
}

// <comparison> ::= <addition> (('>' | '<' | '>=' | '<=') <addition>)*
size_t comparison(parser* parser)
{
    size_t result = addition(parser);
    while(parser_match(parser, TOK_GT) || parser_match(parser, TOK_LT) || parser_match(parser, TOK_GE) || parser_match(parser, TOK_LE))
    {
        token* op = parser_previous_token(parser);
        size_t right = addition(parser);
        size_t bin_op = allocate_vsd_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(OFFSET_PTR(bin_op), op->type, result, right, parser->ast_array.data, op->line);
        result = bin_op;
    }
    return result;
}

// <addition> ::= <multiplication> (('+' | '-') <multiplication> )*
size_t addition(parser* parser)
{
    size_t result = multiplication(parser);
    while(parser_match(parser, TOK_PLUS) || parser_match(parser, TOK_MINUS))
    {
        token* op = parser_previous_token(parser);
        size_t right = multiplication(parser);
        size_t bin_op = allocate_vsd_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(OFFSET_PTR(bin_op), op->type, result, right, parser->ast_array.data, op->line);
        result = bin_op;
    }
    return result;
}

// <multiplication> ::= <modulo> (('*' | '/') <modulo> )*
size_t multiplication(parser* parser)
{
    size_t result = modulo(parser);
    while(parser_match(parser, TOK_STAR) || parser_match(parser, TOK_SLASH))
    {
        token* op = parser_previous_token(parser);
        size_t right = modulo(parser);
        size_t bin_op = allocate_vsd_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(OFFSET_PTR(bin_op), op->type, result, right, parser->ast_array.data, op->line);
        result = bin_op;
    }
    return result;
}

// <modulo> ::= <exponent> ('%' exponent)*
size_t modulo(parser* parser)
{
    size_t result = exponent(parser);
    while(parser_match(parser, TOK_MOD))
    {
        size_t right = exponent(parser);
        size_t bin_op = allocate_vsd_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(OFFSET_PTR(bin_op), TOK_MOD, result, right, parser->ast_array.data, parser_previous_token(parser)->line);
        result = bin_op;
    }
    return result;
}

// <exponent> ::= <unary> ('^' <exponent>)*
size_t exponent(parser* parser)
{
    size_t result = unary(parser);
    while(parser_match(parser, TOK_CARET))
    {
        size_t right = exponent(parser);
        size_t bin_op = allocate_vsd_array(&parser->ast_array, sizeof(BinOp));
        init_BinOp(OFFSET_PTR(bin_op), TOK_CARET, result, right, parser->ast_array.data, parser_previous_token(parser)->line);
        result = bin_op;
    }
    return result;
}

// <unary> ::= ('+'|'-'|'~') <unary>  |  <primary>
size_t unary(parser* parser)
{
    if (parser_match(parser, TOK_NOT) || parser_match(parser, TOK_PLUS) || parser_match(parser, TOK_MINUS))
    {
        token* op = parser_previous_token(parser);
        size_t operand = unary(parser);
        size_t un_op = allocate_vsd_array(&parser->ast_array, sizeof(UnOp));
        init_UnOp(OFFSET_PTR(un_op), op->type, operand, parser->ast_array.data, op->line);
        return un_op;
    }
    return primary(parser);
}

// <primary>  ::=  <integer> | <float> | <bool> | <string> | <identifier> | '(' <expr> ')'
size_t primary(parser* parser)
{
    char num_string[128];
    if (parser_match(parser, TOK_INTEGER))
    {
        token* integer_token = parser_previous_token(parser);
        size_t integer = allocate_vsd_array(&parser->ast_array, sizeof(Integer));
        memcpy_s(num_string, 127, integer_token->token.string_value, integer_token->token.length);
        num_string[min(127, integer_token->token.length)] = '\0';
        init_Integer(OFFSET_PTR(integer), strtol(num_string, NULL, 10), integer_token->line);
        return integer;
    }
    if (parser_match(parser, TOK_FLOAT))
    {
        token* float_token = parser_previous_token(parser);
        size_t number = allocate_vsd_array(&parser->ast_array, sizeof(Float));
        memcpy_s(num_string, 127, float_token->token.string_value, float_token->token.length);
        num_string[min(127, float_token->token.length)] = '\0';
        init_Float(OFFSET_PTR(number), strtod(num_string, NULL), float_token->line);
        return number;
    }
    if (parser_match(parser, TOK_TRUE))
    {
        token* bool_token = parser_previous_token(parser);
        size_t boolean = allocate_vsd_array(&parser->ast_array, sizeof(Bool));
        init_Bool(OFFSET_PTR(boolean), 1, bool_token->line);
        return boolean;
    }
    if (parser_match(parser, TOK_FALSE))
    {
        token* bool_token = parser_previous_token(parser);
        size_t boolean = allocate_vsd_array(&parser->ast_array, sizeof(Bool));
        init_Bool(OFFSET_PTR(boolean), 0, bool_token->line);
        return boolean;
    }
    if (parser_match(parser, TOK_STRING))
    {
        token* string_token = parser_previous_token(parser);
        size_t string = allocate_vsd_array(&parser->ast_array, sizeof(String));
        init_String(OFFSET_PTR(string), string_token->token.string_value+1, string_token->token.length-2, string_token->line);
        return string;
    }
    if (parser_match(parser, TOK_LPAREN))
    {
        size_t expression = expr(parser);
        if (!parser_match(parser, TOK_RPAREN))
        {
            void* expression_ptr = OFFSET_PTR(expression);
            PRINT_SYNTAX_ERROR_AND_QUIT(GET_ELEMENT_LINE(expression_ptr), "Expected ')' after expression\n");
        }
        else
        {
            size_t grouping = allocate_vsd_array(&parser->ast_array, sizeof(Grouping));
            void* expression_ptr = OFFSET_PTR(expression);
            init_Grouping(OFFSET_PTR(grouping), expression, parser->ast_array.data, GET_ELEMENT_LINE(expression_ptr));
            return grouping;
        }
    }

    // If none of the above, it is an identifier
    token* identifier_token = parser_expect(parser, TOK_IDENTIFIER);
    size_t identifier = allocate_vsd_array(&parser->ast_array, sizeof(Identifier));
    init_Identifier(OFFSET_PTR(identifier), identifier_token->token.string_value, identifier_token->token.length, identifier_token->line);
    return identifier;
}