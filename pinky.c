#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "utils.h"
#include "compiler.h"
#include "vm.h"

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
    // Tokenizing stage
    lexer lexer;
    init_lexer(&lexer, fp);
    PRINT_GOOD("Tokenizing %s\n", filename);
    tokenize(&lexer);
    print_tokens(&lexer);

    // Parsing stage
    parser parser;
    init_parser(&parser, &lexer.tokens);
    PRINT_GOOD("Parsing %s\n", filename);
    void* ast = parse(&parser);
    print_ast(ast);

    // Interpreter stage
    //interpreter interpreter;
    //init_interpreter(&interpreter);
    //PRINT_GOOD("Interpreting %s\n", filename);
    //printf("\n");
    //interpret_ast(&interpreter, ast);

    // Compiler stage
    compiler compiler;
    init_compiler(&compiler);
    PRINT_GOOD("Generating code for %s\n", filename);
    unsigned char* bytecode = compile_code(&compiler, ast);
    print_code(&compiler);

    // Execution stage
    vm vm;
    init_vm(&vm);
    PRINT_GOOD("Executing %s\n", filename);
    printf("\n");
    run_vm(&vm, bytecode);

    for (int i = 0; i < vm.sp; i++) 
    {
        if (i % 24 == 0) printf("\n");
        printf("%02X ", (unsigned char)vm.stack[i]);
    }

    // Close stuff
    free_lexer(&lexer);
    free_parser(&parser);
    destroy_compiler(&compiler);
    destroy_vm(&vm);
    //free_interpreter(&interpreter);
    fclose(fp);

    return 0;
}
