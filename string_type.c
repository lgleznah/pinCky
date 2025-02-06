#include "string_type.h"

int string_comparison(const string_type* str1, const string_type* str2, compare_mode mode)
{
    int comparison_results[3][6] = {{0,1,0,1,0,1},{0,0,1,1,1,0},{1,0,1,0,0,1}};
    
    // strcmp-like comparison
    int result = -1;
    for (int i = 0; i < str1->length && i < str2->length; i++)
    {
        if (str1->string_value[i] > str2->string_value[i])
        {
            result = 2;
            break;
        }

        if (str1->string_value[i] < str2->string_value[i])
        {
            result = 0;
            break;
        }
    }

    // Both strings are equal up to some point. Check lengths to compute result
    if (result == -1)
    {
        if (str1->length < str2->length) result = 0;
        else if (str1->length > str2->length) result = 2;
        else result = 1;
    }

    // Use comparison mode and comparison result to return final result
    return comparison_results[result][mode];
}

int string_hash(const string_type* str)
{
    int result = 1;
    for (int i = 0; i < str->length; i++)
    {
        result = result * 31 + str->string_value[i];
        result ^= ((int)0x9e3779b9 ^ (result << 6) ^ (result >> 2));
    }

    return result & 0x7fffffff;
}