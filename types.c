#include "types.h"

#include <stdio.h>
#include <memory.h>

#include "utils.h"

string_type cast_to_string(vss_array* memory, expression_result expression)
{
    char num_value[256];
    switch (expression.type)
    {
        case STRING_VALUE:
            return expression.value.string_value;

        case INT_VALUE:
            int int_length = snprintf(num_value, 256, "%d", expression.value.int_value);
            char* converted_int_string = allocate_vss_array(memory, int_length);
            memcpy(converted_int_string, num_value, int_length);
            return (string_type) {.length = int_length, .string_value = converted_int_string};

        case FLOAT_VALUE:
            int float_length = snprintf(num_value, 256, "%f", expression.value.float_value);
            char* converted_float_string = allocate_vss_array(memory, float_length);
            memcpy(converted_float_string, num_value, float_length);
            return (string_type) {.length = float_length, .string_value = converted_float_string};

        case BOOL_VALUE:
            return (expression.value.bool_value) ? true_string : false_string;

        case NONE:
            return none_string;

        default:
            PRINT_INTERPRETER_ERROR_AND_QUIT(0, "Cannot convert expression of type '%s' to string. This should not appear...", type_names[expression.type]);
    }
}

int cast_to_bool(vss_array* memory, expression_result expression)
{
    switch (expression.type)
    {
        case STRING_VALUE:
            return (expression.value.string_value.length) ? 1 : 0;

        case INT_VALUE:
            return (expression.value.int_value) ? 1 : 0;

        case FLOAT_VALUE:
            return (expression.value.float_value >= 0) ? 1 : 0;

        case BOOL_VALUE:
            return (expression.value.bool_value);

        case NONE:
            return 0;

        default:
            PRINT_INTERPRETER_ERROR_AND_QUIT(0, "Cannot convert expression of type '%s' to bool. This should not appear...", type_names[expression.type]);
    }
}

string_type string_addition(vss_array* memory, string_type string1, string_type string2)
{
    char* destination_string = allocate_vss_array(memory, string1.length+string2.length);
    
    for(int i = 0; i < string1.length; i++)
    {
        destination_string[i] = string1.string_value[i];
    }
    for(int i = 0; i < string2.length; i++)
    {
        destination_string[i+string1.length] = string2.string_value[i];
    }
    
    return (string_type) {.length = string1.length+string2.length, .string_value = destination_string};
}
