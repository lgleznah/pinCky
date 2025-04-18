#pragma once

#include "interpreter_commons.h"

static char* type_names[] = {"none", "int", "float", "bool", "string"};

static string_type true_string = {.string_value = "true", .length = sizeof("true")-1};
static string_type false_string = {.string_value = "false", .length = sizeof("false")-1};
static string_type none_string = {.string_value = "none", .length = sizeof("none")-1};

struct interpreter;
struct expression_result;

string_type cast_to_string(vss_array* memory, expression_result expression);
boolean_type cast_to_bool(vss_array* memory, expression_result expression);
string_type string_addition(vss_array* memory, string_type string1, string_type string2);