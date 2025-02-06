#pragma once

typedef struct
{
    char* string_value;
    int length;
} string_type;

typedef enum
{
    COMPARE_GT,
    COMPARE_LT,
    COMPARE_GE,
    COMPARE_LE,
    COMPARE_EQ,
    COMPARE_NE
} compare_mode;

int string_comparison(const string_type* str1, const string_type* str2, compare_mode mode);
int string_hash(const string_type* str);