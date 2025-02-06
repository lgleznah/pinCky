#pragma once

#include "hashmap.h"
#include "arrays.h"
#include "interpreter_commons.h"

typedef struct environment environment;

struct environment
{
    hashmap variables;
    vsd_array variables_memory;
    environment* parent;
};

void init_environment(environment* e, environment* parent);
void clear_environment(environment* e);

void set_variable(environment* state, string_type name, expression_result value);
expression_result get_variable(environment* state, string_type name, int line);