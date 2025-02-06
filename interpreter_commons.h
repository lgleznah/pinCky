#pragma once

#include "arrays.h"
#include "string_type.h"

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

typedef struct
{
    vss_array memory;
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