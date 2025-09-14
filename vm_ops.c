#include "vm_ops.h"

#include "compiler_commons.h"
#include "types.h"
#include "utils.h"

#include <string.h>
#include <math.h>

inline void free_if_string(expression_result* val)
{
    if (val->type == STRING_VALUE)
        free(val->value.string_value.string_value);
}

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
    // Free possibly existing strings in lhs and rhs before smashing those values
    free_if_string(lhs);
    free_if_string(rhs);

    *dest = (expression_result) {
        .type=STRING_VALUE,
        .value.string_value = { malloc(res.length), res.length }
    };
    memcpy(dest->value.string_value.string_value, res.string_value, res.length);
    return sizeof(expression_result);
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
    char comp_result = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_EQ);
    free_if_string(lhs);
    free_if_string(rhs);
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = comp_result};
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
    char comp_result = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_NE);
    free_if_string(lhs);
    free_if_string(rhs);
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = comp_result};
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
    char comp_result = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_GT);
    free_if_string(lhs);
    free_if_string(rhs);
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = comp_result};
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
    char comp_result = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_GE);
    free_if_string(lhs);
    free_if_string(rhs);
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = comp_result};
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
    char comp_result = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_LT);
    free_if_string(lhs);
    free_if_string(rhs);
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = comp_result};
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
    char comp_result = string_comparison(&lhs->value.string_value, &rhs->value.string_value, COMPARE_LE);
    free_if_string(lhs);
    free_if_string(rhs);
    *dest = (expression_result) {.type = BOOL_VALUE, .value.bool_value = comp_result};
    return sizeof(expression_result);
}
