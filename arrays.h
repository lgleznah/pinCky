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

typedef struct
{
    void* data;
    size_t size;
    size_t used;
} interpreter_memory;

void init_interpreter_memory(interpreter_memory* memory, size_t initial_size);
void clear_interpreter_memory(interpreter_memory* memory);
void free_interpreter_memory(interpreter_memory* memory);
void* allocate_interpreter_memory(interpreter_memory* memory, size_t bytes);

typedef struct
{
    size_t* statements;
    size_t size;
    size_t used;
} statement_array;

void init_statement_array(statement_array* array, size_t initial_size);
void free_statement_array(statement_array* array);
void insert_statement_array(statement_array* array, size_t statement_offset);