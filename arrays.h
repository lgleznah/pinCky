#pragma once

#include "tokens.h"
#include <stdlib.h>

typedef struct
{
    token* data;
    size_t size;
    size_t used;
} token_array;

void init_token_array(token_array* array, size_t initial_size);
void free_token_array(token_array* array);
void insert_token_array(token_array* array, token_type type, int line, int column, char* start, int length);

typedef struct
{
    void* ast_data;
    size_t size;
    size_t used;
} ast_array;

void init_ast_array(ast_array* array, size_t initial_size);
void free_ast_array(ast_array* array);
void* allocate_ast_array(ast_array* array, size_t bytes);
