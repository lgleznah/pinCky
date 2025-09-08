#include "compiler_commons.h"

size_t get_value_size(expression_result value)
{
    switch(value.type)
    {
        case NONE:
        case INT_VALUE:
        case FLOAT_VALUE:
        case BOOL_VALUE:
            return sizeof(expression_result);

        case STRING_VALUE:
            return sizeof(expression_result) + value.value.string_value.length;
    }

    return 0;
}
