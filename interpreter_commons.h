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

typedef enum environment_type
{
    ENV_MAIN,
    ENV_FUNC,
    ENV_BLOCK
} environment_type;

struct environment
{
    hashmap variables;
    vsd_array variables_memory;
    hashmap functions;
    environment* parent;
    environment_type type;
};

typedef struct
{
    vss_array memory;
    environment environ_stack[STACK_SIZE];
    int stack_index;
    int is_returning;
} interpreter;

static string_type ret_var = { .string_value = "(ret)", .length = 5 };

typedef struct
{
    size_t addr;
    environment* env;
} function;

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