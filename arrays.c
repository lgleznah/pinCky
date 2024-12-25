#include "arrays.h"

#include <stdio.h>
#include <string.h>

void init_token_array(token_array* array, size_t initial_size)
{
    array->size = initial_size;
    array->used = 0;
    array->data = (token*)malloc(initial_size * sizeof(token));
}

void insert_token_array(token_array* array, token_type type, int line, int column, char* start, int length)
{
    if (array->used == array->size)
    {
        array->size *= 2;
        void* tmp = realloc((void*) (array->data), array->size * sizeof(token));
        if (!tmp)
        {
            printf("Realloc failed!\n");
            free_token_array(array);
            return;
        }
        array->data = (token*)tmp;
    }

    array->data[array->used].type = type;
    array->data[array->used].line = line;
    array->data[array->used].column = column;
    array->data[array->used].start = start;
    array->data[array->used].length = length;
    array->used++;
}

void free_token_array(token_array* array)
{
    free((void*) (array->data));
    array->data = NULL;
    array->used = 0;
    array->size = 0;
}

void init_ast_array(ast_array* array, size_t initial_size)
{
    array->size = initial_size;
    array->used = 0;
    array->ast_data = (token*)malloc(initial_size);
}

void free_ast_array(ast_array* array)
{
    free((void*) (array->ast_data));
    array->ast_data = NULL;
    array->used = 0;
    array->size = 0;
}

void* allocate_ast_array(ast_array* array, size_t bytes)
{
    // Realloc if new size exceeds current limit
    if (array->used + bytes > array->size)
    {
        printf("Max parser memory exceeded!\n");
        exit(1);
    }

    // Assign memory address
    void* addr = (void*)((char*)(array->ast_data) + array->used);
    array->used += bytes;
    return addr;
}