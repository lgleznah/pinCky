#pragma once

#define MAX_CODE_SIZE 65536

typedef struct compiler
{
    unsigned char code[MAX_CODE_SIZE];
    unsigned int code_idx;
} compiler;

void init_compiler(compiler* compiler);
void destroy_compiler(compiler* compiler);

void print_code(compiler* compiler);

unsigned char* compile_code(compiler* compiler, void* ast_node);
