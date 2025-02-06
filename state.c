#include "state.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

void init_environment(environment* e, environment* parent)
{
    init_hashmap(&e->variables, 32, 32);
    init_vsd_array(&e->variables_memory, 65535);
    e->parent = parent;
}

void clear_environment(environment* e)
{
    free_hashmap(&e->variables);
    free_vsd_array(&e->variables_memory);
}

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

// Set a variable value, either in the current environment (if it exists there), or in the
// nearest parent in which it exists. If it does not exist, create it in the current environment.
// For strings, new memory will always be allocated.
void set_variable(environment* state, string_type name, expression_result value)
{
    environment* current = state;
    size_t variable_offset = 0;
    while(current)
    {
        if (!hashmap_get(&current->variables, &name, &variable_offset))
        {
            if (value.type == STRING_VALUE)
            {
                variable_offset = allocate_vsd_array(&current->variables_memory, get_value_size(value));
                char* variable_address = (char*)current->variables_memory.data + variable_offset;
                memcpy_s(variable_address, sizeof(expression_result), &value, sizeof(expression_result));
                memcpy_s(variable_address + sizeof(expression_result), value.value.string_value.length, value.value.string_value.string_value, value.value.string_value.length);
                ((expression_result*)variable_address)->value.string_value.string_value = (char*) variable_offset + sizeof(expression_result);
                hashmap_set(&current->variables, name, variable_offset);
            }
            else
            {
                char* variable_address = (char*)current->variables_memory.data + variable_offset;
                memcpy_s(variable_address, sizeof(expression_result), &value, sizeof(expression_result));
            }
            return;
        }
        current = current->parent;
    }

    variable_offset = allocate_vsd_array(&state->variables_memory, get_value_size(value));
    char* variable_address = (char*)state->variables_memory.data + variable_offset;
    memcpy_s(variable_address, sizeof(expression_result), &value, sizeof(expression_result));
    if (value.type == STRING_VALUE)
    {
        memcpy_s(variable_address + sizeof(expression_result), value.value.string_value.length, value.value.string_value.string_value, value.value.string_value.length);
        ((expression_result*)variable_address)->value.string_value.string_value = (char*) (variable_offset + sizeof(expression_result));
    }
    
    hashmap_set(&state->variables, name, variable_offset);
}

expression_result get_variable(environment* state, string_type name, int line)
{
    environment* current = state;
    size_t variable_offset = 0;
    while(current)
    {
        if (!hashmap_get(&current->variables, &name, &variable_offset))
        {
            char* variable_address = (char*)current->variables_memory.data + variable_offset;
            expression_result variable = *(expression_result*)variable_address;
            if(variable.type == STRING_VALUE)
            {
                variable.value.string_value.string_value = variable_address + sizeof(expression_result);
            }

            return variable;
        }
        current = current->parent;
    }

    PRINT_INTERPRETER_ERROR_AND_QUIT(line, "Cannot find variable '%.*s'", name.length, name.string_value);
}