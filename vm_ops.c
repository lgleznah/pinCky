#include "vm_ops.h"

#include "compiler_commons.h"
#include "types.h"
#include "utils.h"

#include <string.h>
#include <math.h>

int unsupported_op(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    PRINT_ERROR_AND_QUIT("Unsupported operation.\n");
}

int int_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value + rhs->value.int_value };
    return sizeof(expression_result);
}

int float_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=vl+vr };
    return sizeof(expression_result);
}

int string_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    string_type res = string_addition(temp_memory, cast_to_string(temp_memory, *lhs), cast_to_string(temp_memory, *rhs));
    memcpy(dest, res.string_value, res.length);
    int string_stack_frames = ((res.length + 23) / 24);
    *(dest + string_stack_frames) = (expression_result) {
        .type=STRING_VALUE,
        .value.string_value = { (char*)dest, res.length }
    };
    return (1 + string_stack_frames) * sizeof(expression_result);
}


int int_sub(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value - rhs->value.int_value };
    return sizeof(expression_result);
}

int float_sub(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=vl-vr };
    return sizeof(expression_result);
}


int int_mul(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value * rhs->value.int_value };
    return sizeof(expression_result);
}

int float_mul(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=vl*vr };
    return sizeof(expression_result);
}


int int_div(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    if (rhs->value.int_value == 0)
    {
        PRINT_ERROR_AND_QUIT("Division by zero.\n");
    }
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value / rhs->value.int_value };
    return sizeof(expression_result);
}

int float_div(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    if (vr == 0)
    {
        PRINT_ERROR_AND_QUIT("Division by zero.\n");
    }
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=vl/vr };
    return sizeof(expression_result);
}


int int_mod(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    if (rhs->value.int_value == 0)
    {
        PRINT_ERROR_AND_QUIT("Division by zero.\n");
    }
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=lhs->value.int_value % rhs->value.int_value };
    return sizeof(expression_result);
}

int float_mod(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    if (vr == 0)
    {
        PRINT_ERROR_AND_QUIT("Division by zero.\n");
    }
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=fmod(vl, vr) };
    return sizeof(expression_result);
}


int int_exp(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) { .type=INT_VALUE, .value.int_value=int_pow(lhs->value.int_value, rhs->value.int_value) };
    return sizeof(expression_result);
}

int float_exp(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double vl = (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : (double) (lhs->value.int_value);
    double vr = (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : (double) (rhs->value.int_value);
    *dest = (expression_result) { .type=FLOAT_VALUE, .value.float_value=pow(vl, vr) };
    return sizeof(expression_result);
}


int int_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left == converted_val_right};
    return sizeof(expression_result);
}

int float_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left == converted_val_right};
    return sizeof(expression_result);
}
int str_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_EQ)};
    return sizeof(expression_result);
}


int int_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left != converted_val_right};
    return sizeof(expression_result);
}

int float_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left != converted_val_right};
    return sizeof(expression_result);
}
int str_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_NE)};
    return sizeof(expression_result);
}


int int_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left > converted_val_right};
    return sizeof(expression_result);
}

int float_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left > converted_val_right};
    return sizeof(expression_result);
}
int str_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_GT)};
    return sizeof(expression_result);
}


int int_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left >= converted_val_right};
    return sizeof(expression_result);
}

int float_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left >= converted_val_right};
    return sizeof(expression_result);
}
int str_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_GE)};
    return sizeof(expression_result);
}


int int_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left < converted_val_right};
    return sizeof(expression_result);
}

int float_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left < converted_val_right};
    return sizeof(expression_result);
}
int str_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_LT)};
    return sizeof(expression_result);
}


int int_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    int converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : lhs->value.bool_value;
    int converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left <= converted_val_right};
    return sizeof(expression_result);
}

int float_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    double converted_val_left = (lhs->type == INT_VALUE) ? lhs->value.int_value : (lhs->type == FLOAT_VALUE) ? lhs->value.float_value : lhs->value.bool_value;
    double converted_val_right = (rhs->type == INT_VALUE) ? rhs->value.int_value : (rhs->type == FLOAT_VALUE) ? rhs->value.float_value : rhs->value.bool_value;
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left <= converted_val_right};
    return sizeof(expression_result);
}
int str_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest)
{
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_LE)};
    return sizeof(expression_result);
}
