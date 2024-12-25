#include "lexer.h"

#include <ctype.h>
#include <string.h>

#include "utils.h"

#define ADD_TOKEN(token_list, token_type) do { \
    insert_token_array(token_list, token_type, lexer->line, lexer->column, &lexer->buffer[lexer->start], lexer->curr - lexer->start); \
    } while(0)

#define BOUNDS_CHECK(return_if_fail) do {if (lexer->curr >= lexer->length) return (return_if_fail);} while (0)

#define TEST_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define TEST_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
#define TEST_ALNUM(c) (TEST_DIGIT(c) || TEST_ALPHA(c))

char* keywords[] = {
    "if", "then", "else", "true", "false", "and", "or", "while", "do", "for", "func", "null",
    "end", "print", "println", "ret"
};
token_type keyword_tokens[] = {
    TOK_IF, TOK_THEN, TOK_ELSE, TOK_TRUE, TOK_FALSE, TOK_AND, TOK_OR, TOK_WHILE, TOK_DO, TOK_FOR, TOK_FUNC, TOK_NULL,
    TOK_END, TOK_PRINT, TOK_PRINTLN, TOK_RET
};

void init_lexer(lexer* lexer, FILE* file)
{
    lexer->source = file;
    lexer->start = lexer->curr = 0;
    lexer->line = 1;
    lexer->column = 1;
    init_token_array(&lexer->tokens, 1024);
}

void free_lexer(lexer* lexer)
{
    free_token_array(&lexer->tokens);
    free(lexer->buffer);
}

void tokenize(lexer* lexer)
{
    // Load file into a buffer
    fseek(lexer->source, 0, SEEK_END);
    lexer->length = ftell(lexer->source);
    fseek(lexer->source, 0, SEEK_SET);
    lexer->buffer = (char*)malloc(lexer->length);
    if (lexer->buffer)
    {
        fread(lexer->buffer, 1, lexer->length, lexer->source);
    }

    // Parse tokens
    char ch;
    while(lexer->curr < lexer->length)
    {
        lexer->start = lexer->curr;
        ch = advance(lexer);
        
        // Increase line count if newline, ignore CR/space/tab, and go on processing other characters
        if (ch == '\n')
        {
            lexer->line++;
            lexer->column = 0;
        }
        else if (ch == '\r' || ch == ' ' || ch == '\t') {}

        // Check for line comments and minus symbol
        else if (ch == '-') 
        {
            if (match(lexer, '-'))
            {
                while(peek(lexer) != '\n' && lexer->curr < lexer->length)
                {
                    advance(lexer);
                }
            }
            else
            {
                ADD_TOKEN(&lexer->tokens, TOK_MINUS);
            }
        }
        
        // Test single-character tokens
        else if (ch == '(') ADD_TOKEN(&lexer->tokens, TOK_LPAREN);
        else if (ch == ')') ADD_TOKEN(&lexer->tokens, TOK_RPAREN);
        else if (ch == '{') ADD_TOKEN(&lexer->tokens, TOK_LCURLY);
        else if (ch == '}') ADD_TOKEN(&lexer->tokens, TOK_RCURLY);
        else if (ch == '[') ADD_TOKEN(&lexer->tokens, TOK_LSQUAR);
        else if (ch == ']') ADD_TOKEN(&lexer->tokens, TOK_RSQUAR);
        else if (ch == ',') ADD_TOKEN(&lexer->tokens, TOK_COMMA);
        else if (ch == '+') ADD_TOKEN(&lexer->tokens, TOK_PLUS);
        else if (ch == '*') ADD_TOKEN(&lexer->tokens, TOK_STAR);
        else if (ch == '/') ADD_TOKEN(&lexer->tokens, TOK_SLASH);
        else if (ch == '^') ADD_TOKEN(&lexer->tokens, TOK_CARET);
        else if (ch == '%') ADD_TOKEN(&lexer->tokens, TOK_MOD);
        else if (ch == ';') ADD_TOKEN(&lexer->tokens, TOK_SEMICOLON);
        else if (ch == '?') ADD_TOKEN(&lexer->tokens, TOK_QUESTION);

        // Test two-character tokens
        else if (ch == '=')
        {
            if (match(lexer, '='))
            {
                ADD_TOKEN(&lexer->tokens, TOK_EQEQ);
            }
            else
            {
                ADD_TOKEN(&lexer->tokens, TOK_EQ);
            }
        }
        else if (ch == '~')
        {
            if (match(lexer, '='))
            {
                ADD_TOKEN(&lexer->tokens, TOK_NE);
            }
            else
            {
                ADD_TOKEN(&lexer->tokens, TOK_NOT);
            }
        }

        else if (ch == ':')
        {
            if (match(lexer, '='))
            {
                ADD_TOKEN(&lexer->tokens, TOK_ASSIGN);
            }
            else
            {
                ADD_TOKEN(&lexer->tokens, TOK_COLON);
            }
        }
        
        else if (ch == '>')
        {
            if (match(lexer, '='))
            {
                ADD_TOKEN(&lexer->tokens, TOK_GE);
            }
            else
            {
                ADD_TOKEN(&lexer->tokens, TOK_GT);
            }
        }

        else if (ch == '<')
        {
            if (match(lexer, '='))
            {
                ADD_TOKEN(&lexer->tokens, TOK_LE);
            }
            else
            {
                ADD_TOKEN(&lexer->tokens, TOK_LT);
            }
        }

        // Test digits (ints and floats)
        else if (TEST_DIGIT(ch))
        {
            while(TEST_DIGIT(peek(lexer)))
            {
                advance(lexer);
            }
            if (peek(lexer) == '.' && TEST_DIGIT(lookahead(lexer, 1)))
            {
                advance(lexer);
                while(TEST_DIGIT(peek(lexer)))
                {
                    advance(lexer);
                }
                ADD_TOKEN(&lexer->tokens, TOK_FLOAT);
            }
            else
            {
                ADD_TOKEN(&lexer->tokens, TOK_INTEGER);
            }
        }

        // Floats can also be specified by a single dot, followed by a series of digits
        else if (ch == '.')
        {
            if (!TEST_DIGIT(peek(lexer)))
            {
                ADD_TOKEN(&lexer->tokens, TOK_DOT);
            }
            else
            {
                advance(lexer);
                while(TEST_DIGIT(peek(lexer)))
                {
                    advance(lexer);
                }
                ADD_TOKEN(&lexer->tokens, TOK_FLOAT);
            }
        }

        // Test strings (enclosed in "" and in '')
        else if (ch == '"')
        {
            char str_content;
            while((str_content = peek(lexer)) != '"')
            {
                if (str_content == '\0')
                {
                    PRINT_LEXER_ERROR_AND_QUIT(lexer->line, lexer->column, "Expected \" but found EOF");
                }
                advance(lexer);
            }
            advance(lexer);
            ADD_TOKEN(&lexer->tokens, TOK_STRING);
        }

        else if (ch == '\'')
        {
            char str_content;
            while((str_content = peek(lexer)) != '\'')
            {
                if (str_content == '\0')
                {
                    PRINT_LEXER_ERROR_AND_QUIT(lexer->line, lexer->column, "Expected \' but found EOF");
                }
                advance(lexer);
            }
            advance(lexer);
            ADD_TOKEN(&lexer->tokens, TOK_STRING);
        }

        // Test identifiers (beginning with upper, lower or _, followed by alphanumeric chars) and keywords
        else if (TEST_ALPHA(ch))
        {
            while (TEST_ALNUM(peek(lexer)))
            {
                advance(lexer);
            }

            // Check if the detected identifier is a keyword
            char kw_buff[256];
            int kw_found = 0;
            memcpy_s(kw_buff, lexer->curr - lexer->start, &lexer->buffer[lexer->start], lexer->curr - lexer->start);
            kw_buff[lexer->curr - lexer->start] = '\0';
            for (int i = 0; i < 16; i++)
            {
                if (strcmp(kw_buff, keywords[i]) == 0)
                {
                    ADD_TOKEN(&lexer->tokens, keyword_tokens[i]);
                    kw_found = 1;
                    break;
                }
            }
            if (!kw_found)
            {
                ADD_TOKEN(&lexer->tokens, TOK_IDENTIFIER);
            }
        }

        // Default case: error :-(
        else
        {
            PRINT_LEXER_ERROR_AND_QUIT(lexer->line, lexer->column, "Unexpected character %c\n", ch);
        }
    }

    return;
}

char advance(lexer* lexer)
{
    BOUNDS_CHECK('\0');
    lexer->column++;
    return lexer->buffer[lexer->curr++];
}

char peek(const lexer* lexer)
{
    BOUNDS_CHECK('\0');
    return lexer->buffer[lexer->curr];
}

char lookahead(const lexer* lexer, int n)
{
    BOUNDS_CHECK('\0');
    return lexer->buffer[lexer->curr + n];
}

int match(lexer* lexer, char expected)
{
    BOUNDS_CHECK(0);
    if (lexer->buffer[lexer->curr] != expected)
    {
        return 0;
    }

    lexer->column++;
    lexer->curr++;
    return 1;
}

void print_tokens(const lexer* lexer)
{
    for (size_t i = 0; i < lexer->tokens.used; i++)
    {
        print_token(&lexer->tokens.data[i]);
    }
}