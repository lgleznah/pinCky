#pragma once

#define MAX_CODE_SIZE 65536

typedef struct compiler
{
    char* code[MAX_CODE_SIZE];
} compiler;

void init_compiler(compiler* compiler);
void destroy_compiler(compiler* compiler);

void compile(compiler* compiler, void* ast_node);