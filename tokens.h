#pragma once

typedef enum
{
    // SINGLE-CHAR TOKENS
    TOK_LPAREN,                 // (   
    TOK_RPAREN,                 // )
    TOK_LCURLY,                 // {
    TOK_RCURLY,                 // }
    TOK_LSQUAR,                 // [
    TOK_RSQUAR,                 // ]
    TOK_COMMA,                  // ,
    TOK_DOT,                    // .
    TOK_PLUS,                   // +
    TOK_MINUS,                  // -
    TOK_STAR,                   // *
    TOK_SLASH,                  // /
    TOK_CARET,                  // ^
    TOK_MOD,                    // %
    TOK_COLON,                  // :
    TOK_SEMICOLON,              // ;
    TOK_QUESTION,               // ?
    TOK_NOT,                    // ~
    TOK_GT,                     // >
    TOK_LT,                     // <
    TOK_EQ,                     // =

    // TWO-CHAR TOKENS
    TOK_GE,                     // >=
    TOK_LE,                     // <=
    TOK_NE,                     // ~=
    TOK_EQEQ,                   // ==
    TOK_ASSIGN,                 // :=
    TOK_GTGT,                   // >>
    TOK_LTLE,                   // <<

    // LITERALS
    TOK_IDENTIFIER,             // foobar
    TOK_STRING,                 // "foobar"
    TOK_INTEGER,                // 42
    TOK_FLOAT,                  // 420.34

    // KEYWORDS
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_TRUE,
    TOK_FALSE,
    TOK_AND,
    TOK_OR,
    TOK_WHILE,
    TOK_DO,
    TOK_FOR,
    TOK_FUNC,
    TOK_NULL,
    TOK_END,
    TOK_PRINT,
    TOK_PRINTLN,
    TOK_RET
} token_type;

static char* token_type_str[] = {
    "TOK_LPAREN",
    "TOK_RPAREN",
    "TOK_LCURLY",
    "TOK_RCURLY",
    "TOK_LSQUAR",
    "TOK_RSQUAR",
    "TOK_COMMA",
    "TOK_DOT",
    "TOK_PLUS",
    "TOK_MINUS",
    "TOK_STAR",
    "TOK_SLASH",
    "TOK_CARET",
    "TOK_MOD",
    "TOK_COLON",
    "TOK_SEMICOLON",
    "TOK_QUESTION",
    "TOK_NOT",
    "TOK_GT",
    "TOK_LT",
    "TOK_EQ",
    "TOK_GE",
    "TOK_LE",
    "TOK_NE",
    "TOK_EQEQ",
    "TOK_ASSIGN",
    "TOK_GTGT",
    "TOK_LTLE",
    "TOK_IDENTIFIER",
    "TOK_STRING",
    "TOK_INTEGER",
    "TOK_FLOAT",
    "TOK_IF",
    "TOK_THEN",
    "TOK_ELSE",
    "TOK_TRUE",
    "TOK_FALSE",
    "TOK_AND",
    "TOK_OR",
    "TOK_WHILE",
    "TOK_DO",
    "TOK_FOR",
    "TOK_FUNC",
    "TOK_NULL",
    "TOK_END",
    "TOK_PRINT",
    "TOK_PRINTLN",
    "TOK_RET"
};

static char* token_symbols[] = {
    "(",
    ")",
    "{",
    "}",
    "[",
    "]",
    ",",
    ".",
    "+",
    "-",
    "*",
    "/",
    "^",
    "%",
    ",",
    ";",
    "?",
    "~",
    ">",
    "<",
    "=",
    ">=",
    "<=",
    "~=",
    "==",
    ":=",
    ">>",
    "<<",
    "\"identifier\"",
    "\"string\"",
    "\"integer\"",
    "\"float\"",
    "if",
    "then",
    "else",
    "true",
    "false",
    "and",
    "or",
    "while",
    "do",
    "for",
    "func",
    "null",
    "end",
    "print",
    "println",
    "ret"
};

typedef struct
{
    token_type type;
    int line, column;

    char* start;
    int length;
} token;

void print_token(const token* token);