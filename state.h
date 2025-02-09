#pragma once

#include "interpreter_commons.h"

void init_environment(environment* e, environment* parent);
void clear_environment(environment* e);
void free_environment(environment* e);

void set_variable(environment* state, string_type name, expression_result value);
expression_result get_variable(environment* state, string_type name, int line);

void set_environment_parent(environment* state, environment* parent);