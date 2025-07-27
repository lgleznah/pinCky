#include "parser.h"

#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"

#define OFFSET_PTR(ptr) ((void*)(ptr + (char*)(parser->ast_array.data)))
#define min(a, b)  (((a) > (b)) ? (b) : (a))

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
    printf("\n");
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

    if (parser_peek(parser)->type == TOK_WHILE)
    {
        return while_stmt(parser);
    }

    if (parser_peek(parser)->type == TOK_FOR)
    {
        return for_stmt(parser);
    }

    if (parser_peek(parser)->type == TOK_FUNC)
    {
        return func_decl(parser);
    }

    if (parser_peek(parser)->type == TOK_RET)
    {
        return ret_stmt(parser);
    }

    // If none of the above, this statement is an assignment or a function call
    int local_assign = 0;
    if (parser_match(parser, TOK_LOCAL))
    {
        local_assign = 1;
    }
    
    size_t lhs = expr(parser);
    if (parser_match(parser, TOK_ASSIGN))
    {
        int line = parser_previous_token(parser)->line;
        size_t rhs = expr(parser);
        size_t assignmnent = allocate_vsd_array(&parser->ast_array, sizeof(Assignment));
        init_Assignment(OFFSET_PTR(assignmnent), lhs, rhs, local_assign, parser->ast_array.data, line);
        return assignmnent;
    }

    if (local_assign)
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(parser_previous_token(parser)->line, "Expected assignment after local keyword.");
    }
    
    FuncCall* func_call = OFFSET_PTR(lhs);
    func_call->base.tag = SET_ELEMENT_TYPE(Statement, FuncCall_expr);
    return lhs;
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

// <while> ::= 'while' <expr> 'do' <stmts> 'end'
size_t while_stmt(parser* parser)
{
    parser_expect(parser, TOK_WHILE);
    size_t condition = expr(parser);
    parser_expect(parser, TOK_DO);
    size_t statements = stmts(parser);
    parser_expect(parser, TOK_END);

    size_t result = allocate_vsd_array(&parser->ast_array, sizeof(While));
    void* condition_ptr = OFFSET_PTR(condition);
    init_While(OFFSET_PTR(result), condition, statements, parser->ast_array.data, GET_ELEMENT_LINE(condition_ptr));
    return result;
}

// <for> ::= 'for' <assign> ',' <expr> (',' <expr>)? 'do' <stmts> 'end'
size_t for_stmt(parser* parser)
{
    parser_expect(parser, TOK_FOR);
    size_t initial_assignment = stmt(parser);
    parser_expect(parser, TOK_COMMA);
    size_t stop = expr(parser);

    size_t step = -1;
    if (parser_peek(parser)->type == TOK_COMMA)
    {
        parser_expect(parser, TOK_COMMA);
        step = expr(parser);
    }

    parser_expect(parser, TOK_DO);
    size_t statements = stmts(parser);
    parser_expect(parser, TOK_END);

    size_t result = allocate_vsd_array(&parser->ast_array, sizeof(For));
    void* initial_assignment_ptr = OFFSET_PTR(initial_assignment);
    init_For(OFFSET_PTR(result), initial_assignment, stop, step, statements, parser->ast_array.data, GET_ELEMENT_LINE(initial_assignment_ptr));
    return result;
}

// <func_decl> ::= "func" <name> "(" <params>? ")" <body_stmts> "end"
size_t func_decl(parser* parser)
{
    string_array params;
    init_string_array(&params, 256);
    
    parser_expect(parser, TOK_FUNC);
    token* func_name = parser_expect(parser, TOK_IDENTIFIER);
    string_type func_name_str = { .length = func_name->token.length, .string_value = func_name->token.string_value};
    parser_expect(parser, TOK_LPAREN);

    token* param_token;
    int i = 0;
    while((param_token = parser_match(parser, TOK_IDENTIFIER)))
    {
        if (i == 256)
        {
            PRINT_INTERPRETER_ERROR_AND_QUIT(func_name->line, "Cannot declare functions with more than 256 parameters.");
        }
        string_type param = { .length = param_token->token.length, .string_value = param_token->token.string_value };
        insert_string_array(&params, param);
        parser_match(parser, TOK_COMMA);
        i += 1;
    }

    parser_expect(parser, TOK_RPAREN);
    size_t statements = stmts(parser);
    parser_expect(parser, TOK_END);

    size_t result = allocate_vsd_array(&parser->ast_array, sizeof(FuncDecl) + params.used * sizeof(string_type));
    init_FuncDecl(OFFSET_PTR(result), func_name_str, &params, statements, parser->ast_array.data, func_name->line);

    return result;
}

// <ret> ::= 'ret' <expr>
size_t ret_stmt(parser* parser)
{
    
    if (parser_match(parser, TOK_RET)) 
    {
        size_t result = expr(parser);
        size_t ret = allocate_vsd_array(&parser->ast_array, sizeof(Return));
        init_Return(OFFSET_PTR(ret), result, parser->ast_array.data, parser_previous_token(parser)->line);
        return ret;
    }

    return -1;
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
        memcpy(num_string, integer_token->token.string_value, min(integer_token->token.length, 127));
        num_string[min(127, integer_token->token.length)] = '\0';
        init_Integer(OFFSET_PTR(integer), strtol(num_string, NULL, 10), integer_token->line);
        return integer;
    }
    if (parser_match(parser, TOK_FLOAT))
    {
        token* float_token = parser_previous_token(parser);
        size_t number = allocate_vsd_array(&parser->ast_array, sizeof(Float));
        memcpy(num_string, float_token->token.string_value, min(float_token->token.length, 127));
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
        string_type str_value = {.length = string_token->token.length - 2, .string_value = string_token->token.string_value+1};
        init_String(OFFSET_PTR(string), str_value, string_token->line);
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

    // If none of the above, it is an identifier or a function call
    token* identifier_token = parser_expect(parser, TOK_IDENTIFIER);
    if (!parser_match(parser, TOK_LPAREN))
    {
        size_t identifier = allocate_vsd_array(&parser->ast_array, sizeof(Identifier));
        init_Identifier(OFFSET_PTR(identifier), identifier_token->token.string_value, identifier_token->token.length, identifier_token->line);
        return identifier;
    }
    else
    {
        expression_array args;
        init_expression_array(&args, 65536);
        string_type func_name_str = { .length = identifier_token->token.length, .string_value = identifier_token->token.string_value};
        
        while(!parser_match(parser, TOK_RPAREN))
        {
            size_t arg = expr(parser);
            insert_expression_array(&args, arg);
            parser_match(parser, TOK_COMMA);
        }

        size_t result = allocate_vsd_array(&parser->ast_array, sizeof(FuncCall) + args.used * sizeof(void*));
        init_FuncCall(OFFSET_PTR(result), func_name_str, &args, parser->ast_array.data, identifier_token->line);

        return result;
    }
    PRINT_SYNTAX_ERROR_AND_QUIT(parser_peek(parser)->line, "Unexpected primary token: %s", token_type_str[parser->tokens->data[parser->curr_token].type]);
}
