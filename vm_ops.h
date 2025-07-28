#pragma once

#include "compiler_commons.h"

////////////////////////////////////////////////////////////////////////////////
///
/// VM type-specific operation implementations
///
/// All these functions return a value, which is the amount of bytes written to
/// the stack (always a multiple of sizeof(expression_result))
///
////////////////////////////////////////////////////////////////////////////////

int unsupported_op(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int string_add(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_sub(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_sub(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_mul(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_mul(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_div(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_div(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_mod(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_mod(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_exp(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_exp(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int str_eq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int str_neq(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int str_gt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int str_ge(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int str_lt(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);

int int_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int float_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);
int str_le(vss_array* temp_memory, expression_result* lhs, expression_result* rhs, expression_result* dest);


static int (*add_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) = 
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, string_add    },
    {unsupported_op, int_add       , float_add     , unsupported_op, string_add    },
    {unsupported_op, float_add     , float_add     , unsupported_op, string_add    },
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, string_add    },
    {string_add    , string_add    , string_add    , string_add    , string_add    }
};

static int (*sub_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_sub       , float_sub     , unsupported_op, unsupported_op},
    {unsupported_op, float_sub     , float_sub     , unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op}
};

static int (*mul_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_mul       , float_mul     , unsupported_op, unsupported_op},
    {unsupported_op, float_mul     , float_mul     , unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op}
};

static int (*div_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_div       , float_div     , unsupported_op, unsupported_op},
    {unsupported_op, float_div     , float_div     , unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op}
};

static int (*mod_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_mod       , float_mod     , unsupported_op, unsupported_op},
    {unsupported_op, float_mod     , float_mod     , unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op}
};

static int (*exp_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_exp       , float_exp     , unsupported_op, unsupported_op},
    {unsupported_op, float_exp     , float_exp     , unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op}
};

static int (*eq_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_eq        , float_eq      , int_eq        , unsupported_op},
    {unsupported_op, float_eq      , float_eq      , float_eq      , unsupported_op},
    {unsupported_op, int_eq        , float_eq      , int_eq        , unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, str_eq        }
};

static int (*ne_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_neq       , float_neq     , int_neq       , unsupported_op},
    {unsupported_op, float_neq     , float_neq     , float_neq     , unsupported_op},
    {unsupported_op, int_neq       , float_neq     , int_neq       , unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, str_neq       }
};

static int (*gt_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_gt        , float_gt      , int_gt        , unsupported_op},
    {unsupported_op, float_gt      , float_gt      , float_gt      , unsupported_op},
    {unsupported_op, int_gt        , float_gt      , int_gt        , unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, str_gt        }
};

static int (*ge_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_ge        , float_ge      , int_ge        , unsupported_op},
    {unsupported_op, float_ge      , float_ge      , float_ge      , unsupported_op},
    {unsupported_op, int_ge        , float_ge      , int_ge        , unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, str_ge        }
};

static int (*lt_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_lt        , float_lt      , int_lt        , unsupported_op},
    {unsupported_op, float_lt      , float_lt      , float_lt      , unsupported_op},
    {unsupported_op, int_lt        , float_lt      , int_lt        , unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, str_lt        }
};

static int (*le_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) =
{
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, unsupported_op},
    {unsupported_op, int_le        , float_le      , int_le        , unsupported_op},
    {unsupported_op, float_le      , float_le      , float_le      , unsupported_op},
    {unsupported_op, int_le        , float_le      , int_le        , unsupported_op},
    {unsupported_op, unsupported_op, unsupported_op, unsupported_op, str_le        }
};
