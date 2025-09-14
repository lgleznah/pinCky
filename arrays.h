#pragma once

#include "tokens.h"
#include "array_generics.h"

#include <stdlib.h>
#include <stdint.h>

// Fixed element-size dynamic arrays for tokens and statement offsets
MAKE_FSD_ARRAY_HEADERS(token, token)
MAKE_FSD_ARRAY_HEADERS(size_t, statement)
MAKE_FSD_ARRAY_HEADERS(string_type, string)
MAKE_FSD_ARRAY_HEADERS(size_t, expression)
MAKE_FSD_ARRAY_HEADERS(uint32_t, label_addr)
MAKE_FSD_ARRAY_HEADERS(uint32_t, uint32_t)
MAKE_FSD_ARRAY_HEADERS(size_t, vm_variables)

// Variable element-size dynamic array (VSD array). Used for the AST structure and for environment variables
typedef struct
{
    void* data;
    size_t size;
    size_t used;
} vsd_array;

void init_vsd_array(vsd_array* array, size_t initial_size);
void clear_vsd_array(vsd_array* array);
void free_vsd_array(vsd_array* array);
size_t allocate_vsd_array(vsd_array* array, size_t bytes);

// Variable element-size static array (VSS array). Used as a temporary memory for expression evaluation
typedef struct
{
    void* data;
    size_t size;
    size_t used;
} vss_array;

void init_vss_array(vss_array* memory, size_t initial_size);
void clear_vss_array(vss_array* memory);
void free_vss_array(vss_array* memory);
void* allocate_vss_array(vss_array* memory, size_t bytes);
