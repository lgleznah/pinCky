#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "utils.h"

int main(const int argc, char* argv[])
{
    // Parse program name
    if (argc != 2)
    {
        printf("Usage: pinky <filename>\n");
        return -1;
    }
    
    // Read Pinky script
    char* filename = argv[1];
    FILE *fp;
    
    if ((fp = fopen(filename, "r")) == NULL) {
        // Cannot open file!
        PRINT_ERROR_AND_QUIT("Cannot open file '%s': %s\n", filename, strerror(errno));
    }

    // File was opened successfully!
    // Tokenize program
    lexer lexer;
    init_lexer(&lexer, fp);
    PRINT_GOOD("Tokenizing %s\n", filename);
    tokenize(&lexer);
    print_tokens(&lexer);

    // Parse program
    parser parser;
    init_parser(&parser, &lexer.tokens);
    PRINT_GOOD("Parsing %s\n", filename);
    void* ast = parse(&parser);
    print_ast(ast);

    // Close stuff
    free_lexer(&lexer);
    free_parser(&parser);
    fclose(fp);
    
    return 0;
}
