#pragma once

#include "arrays.h"
#include "string_type.h"
#include "hashmap.h"


#define STACK_SIZE 1024

typedef enum
{
    NONE,
    INT_VALUE,
    FLOAT_VALUE,
    BOOL_VALUE,
    STRING_VALUE,
} result_type;

typedef int integer_type;
typedef double float_type;
typedef int boolean_type;

typedef struct environment environment;

struct environment
{
    hashmap variables;
    vsd_array variables_memory;
    environment* parent;
};

typedef struct
{
    vss_array memory;
    environment environ_stack[STACK_SIZE];
    int stack_index;
} interpreter;

typedef struct
{
    result_type type;
    union
    {
        integer_type int_value;
        float_type float_value;
        boolean_type bool_value;
        string_type string_value;
    } value;
} expression_result;