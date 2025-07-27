#include "state.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

void init_environment(environment* e, environment* parent)
{
    init_hashmap(&e->variables, 32, 32);
    init_vsd_array(&e->variables_memory, 65535);
    init_hashmap(&e->functions, 32, 32);
    e->parent = parent;
}

void clear_environment(environment* e)
{
    clear_hashmap(&e->variables);
    clear_hashmap(&e->functions);
    clear_vsd_array(&e->variables_memory);
}

void free_environment(environment* e)
{
    free_hashmap(&e->variables);
    free_hashmap(&e->functions);
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
void set_variable(environment* state, string_type name, expression_result value, int force_local)
{
    environment* current = state;
    size_t variable_offset = 0;
    while(!force_local && current)
    {
        if (!hashmap_get(&current->variables, &name, &variable_offset))
        {
            if (value.type == STRING_VALUE)
            {
                variable_offset = allocate_vsd_array(&current->variables_memory, get_value_size(value));
                char* variable_address = (char*)current->variables_memory.data + variable_offset;
                memcpy(variable_address, &value, sizeof(expression_result));
                memcpy(variable_address + sizeof(expression_result), value.value.string_value.string_value, value.value.string_value.length);
                ((expression_result*)variable_address)->value.string_value.string_value = (char*) variable_offset + sizeof(expression_result);
                hashmap_set(&current->variables, name, variable_offset);
            }
            else
            {
                expression_result* variable_address = (expression_result*)((char*)current->variables_memory.data + variable_offset);
                variable_address->type = value.type;
                variable_address->value = value.value;
            }
            return;
        }
        current = current->parent;
    }

    variable_offset = allocate_vsd_array(&state->variables_memory, get_value_size(value));
    char* variable_address = (char*)state->variables_memory.data + variable_offset;
    memcpy(variable_address, &value, sizeof(expression_result));
    if (value.type == STRING_VALUE)
    {
        memcpy(variable_address + sizeof(expression_result), value.value.string_value.string_value, value.value.string_value.length);
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


void set_return(environment* state, expression_result value, int line)
{
    environment* current = state;
    while (current->type != ENV_MAIN)
    {
        if (current->type == ENV_FUNC)
        {
            set_variable(current->parent, ret_var, value, 1);
            return;
        }
        current = current->parent;
    }

    PRINT_INTERPRETER_ERROR_AND_QUIT(line, "Cannot return outside function.");
}

int set_function(environment* state, string_type func_name, function func)
{
    environment* current = state;
    size_t func_offset = 0;
    while(current)
    {
        if (!hashmap_get(&current->functions, &func_name, &func_offset))
        {
            // Function was already declared, this is a no-go!
            return -1;
        }
        current = current->parent;
    }

    // Hacky as fuck. To store the current environment, we store an additional variable in the hashmap,
    // whose name is the name of the function, PLUS the opening parenthesis of its params. But you cannot
    // give a function a name which contains parameters, so this should be OK
    hashmap_set(&state->functions, func_name, func.addr);
    hashmap_set(&state->functions, (string_type) { .string_value=func_name.string_value, .length=func_name.length+1 }, (size_t)func.env);
    return 0;
}

function get_function(environment* state, string_type func_name, int line)
{
    environment* current = state;
    function func = {.addr=0, .env=NULL};
    while(current)
    {
        if (!hashmap_get(&current->functions, &func_name, &func.addr))
        {
            string_type env_key = (string_type) { .string_value=func_name.string_value, .length=func_name.length+1 };
            hashmap_get(&current->functions, &env_key, (size_t*) &func.env);
            return func;
        }
        current = current->parent;
    }

    PRINT_INTERPRETER_ERROR_AND_QUIT(line, "Cannot find function '%.*s'", func_name.length, func_name.string_value);
}

void set_environment_parent(environment* state, environment* parent)
{
    state->parent = parent;
}
