#pragma once

#include "arrays.h"

typedef enum
{
    INT_VALUE,
    FLOAT_VALUE,
    BOOL_VALUE,
    STRING_VALUE,
} result_type;

static char* type_names[] = {"int", "float", "bool", "string"};

typedef struct
{
    interpreter_memory memory;
} interpreter;

typedef struct
{
    char* string_value;
    int length;
} string;

static string true_string = {.string_value = "true", .length = sizeof("true")-1};
static string false_string = {.string_value = "false", .length = sizeof("false")-1};

typedef enum
{
    COMPARE_GT,
    COMPARE_LT,
    COMPARE_GE,
    COMPARE_LE,
    COMPARE_EQ,
    COMPARE_NE
} compare_mode;

typedef struct
{
    result_type type;
    union
    {
        int int_value;
        double float_value;
        int bool_value;
        string string_value;
    } value;
} expression_result;

void init_interpreter(interpreter* interpreter);
void free_interpreter(interpreter* interpreter);

string cast_to_string(interpreter* interpreter, expression_result expression);

expression_result interpret(interpreter* interpreter, void* ast_node);