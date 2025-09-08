#include "arrays.h"

#include <stdio.h>

#include "array_generics.h"
#include "model.h"

#define max(a, b)  (((a) > (b)) ? (a) : (b))

MAKE_FSD_ARRAY_FUNCTIONS(token, token)
MAKE_FSD_ARRAY_FUNCTIONS(size_t, statement)
MAKE_FSD_ARRAY_FUNCTIONS(string_type, string)
MAKE_FSD_ARRAY_FUNCTIONS(size_t, expression)
MAKE_FSD_ARRAY_FUNCTIONS(uint32_t, label_addr)
<<<<<<< HEAD
MAKE_FSD_ARRAY_FUNCTIONS(size_t, vm_variables)
=======
MAKE_FSD_ARRAY_FUNCTIONS(uint32_t, symbol)
>>>>>>> 7a645d3c70e92c6b7396c75d05a284be10a3bc99

void init_vsd_array(vsd_array* array, size_t initial_size)
{
    array->size = initial_size;
    array->used = 0;
    array->data = malloc(initial_size);
}

void clear_vsd_array(vsd_array* array)
{
    array->used = 0;
}

void free_vsd_array(vsd_array* array)
{
    free((void*) (array->data));
    array->data = NULL;
    array->used = 0;
    array->size = 0;
}

size_t allocate_vsd_array(vsd_array* array, size_t bytes)
{
    // Realloc if new size exceeds current limit
    if (array->used + bytes > array->size)
    {
        array->size = max(array->size * 2, array->used + bytes);
        void* tmp = realloc(array->data, array->size);
        if (!tmp)
        {
            printf("Realloc failed!\n");
            exit(1);
        }
        array->data = tmp;
    }

    // Assign memory address
    size_t offset = array->used;
    array->used += bytes;
    
    return offset;
}

void init_vss_array(vss_array* memory, size_t initial_size)
{
    memory->size = initial_size;
    memory->used = 0;
    memory->data = malloc(initial_size);
}

void clear_vss_array(vss_array* memory)
{
    memory->used = 0;
}

void free_vss_array(vss_array* memory)
{
    free((void*) (memory->data));
    memory->data = NULL;
    memory->used = 0;
    memory->size = 0;
}

void* allocate_vss_array(vss_array* memory, size_t bytes)
{
    // Realloc if new size exceeds current limit
    if (memory->used + bytes > memory->size)
    {
        printf("Max VSS memory exceeded!\n");
        exit(1);
    }

    // Assign memory address
    void* addr = (void*)((char*)(memory->data) + memory->used);
    memory->used += bytes;
    return addr;
}
