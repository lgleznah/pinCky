#pragma once

#include "compiler_commons.h"

void unsupported_op(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void string_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_sub(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_sub(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_mul(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_mul(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_div(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_div(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_mod(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_mod(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_exp(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_exp(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void str_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void str_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void str_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void str_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void str_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

void int_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void float_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
void str_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);