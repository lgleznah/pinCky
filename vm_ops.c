#include "vm_ops.h"

#include "types.h"
#include "utils.h"

void unsupported_op(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    PRINT_ERROR_AND_QUIT("Unsupported operation.\n");
}

void int_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value + rhs->value.int_value };
}

void float_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=vl+vr };
}

void string_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    string_type res = string_addition(temp_memory, cast_to_string(temp_memory, *lhs), cast_to_string(temp_memory, *rhs));
    memcpy(dest, res.string_value, res.length);
    *(dest + ((res.length + 23) / 24)) = (expression_result) {
        .type=STRING_VALUE,
        .value.string_value = { dest, res.length }
    };
}


void int_sub(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value - rhs->value.int_value };
}

void float_sub(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=vl-vr };
}


void int_mul(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value * rhs->value.int_value };
}

void float_mul(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=vl*vr };
}


void int_div(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    if (rhs->value.int_value == 0)
    {
        PRINT_ERROR_AND_QUIT("Division by zero.\n");
    }
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value / rhs->value.int_value };
}

void float_div(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    if (vr == 0)
    {
        PRINT_ERROR_AND_QUIT("Division by zero.\n");
    }
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=vl/vr };
}


void int_mod(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    if (rhs->value.int_value == 0)
    {
        PRINT_ERROR_AND_QUIT("Division by zero.\n");
    }
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value % rhs->value.int_value };
}

void float_mod(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    if (vr == 0)
    {
        PRINT_ERROR_AND_QUIT("Division by zero.\n");
    }
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=fmod(vl, vr) };
}


void int_exp(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=int_pow(lhs->value.int_value, rhs->value.int_value) };
}

void float_exp(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=pow(vl, vr) };
}


void int_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left == converted_val_right};
}

void float_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left == converted_val_right};
}
void str_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_EQ)};
}


void int_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left != converted_val_right};
}

void float_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left != converted_val_right};
}
void str_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_NE)};
}


void int_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left > converted_val_right};
}

void float_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left > converted_val_right};
}
void str_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_GT)};
}


void int_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left >= converted_val_right};
}

void float_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left >= converted_val_right};
}
void str_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_GE)};
}


void int_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left < converted_val_right};
}

void float_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left < converted_val_right};
}
void str_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_LT)};
}


void int_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left <= converted_val_right};
}

void float_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left <= converted_val_right};
}
void str_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_LE)};
}