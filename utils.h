#pragma once

// Terminal color printing variables
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define PRINT_ERROR_AND_QUIT(...) printf("%s", KRED); printf("General compiler error: "); printf(__VA_ARGS__); printf(KNRM); exit(1);
#define PRINT_SYNTAX_ERROR_AND_QUIT(line, ...) printf("%s", KRED); printf("Syntax error [line %d]: ", line); printf(__VA_ARGS__); printf(KNRM); exit(1)
#define PRINT_LEXER_ERROR_AND_QUIT(line, column, ...) printf("%s", KRED); printf("Lexer error [line %d, column %d]: ", line, column); printf(__VA_ARGS__); printf(KNRM); exit(1)
#define PRINT_WARNING(...) printf("%s", KYEL); printf(__VA_ARGS__); printf(KNRM)
#define PRINT_GOOD(...) printf("%s", KGRN); printf(__VA_ARGS__); printf(KNRM)
