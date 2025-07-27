#pragma once

#include "arrays.h"

#include <stdint.h>

typedef struct compiler
{
    vsd_array temp_constants;
    vsd_array temp_code;
    vsd_array program;

    uint32_t constants_size;
} compiler;

void init_compiler(compiler* compiler);
void destroy_compiler(compiler* compiler);

void print_code(compiler* compiler);

unsigned char* compile_code(compiler* compiler, void* ast_node);
