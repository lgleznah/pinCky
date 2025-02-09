#pragma once

#include "interpreter_commons.h"


static expression_result true_expression = {.type = BOOL_VALUE, .value.bool_value = 1};
static expression_result false_expression = {.type = BOOL_VALUE, .value.bool_value = 0};

void init_interpreter(interpreter* interpreter);
void free_interpreter(interpreter* interpreter);

expression_result interpret(interpreter* interpreter, void* ast_node, environment* env);
void interpret_ast(interpreter* interpreter, void* ast_node);