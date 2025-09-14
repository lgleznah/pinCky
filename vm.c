#include "vm.h"

#include "arrays.h"
#include "compiler_commons.h"
#include "types.h"
#include "utils.h"
#include "vm_ops.h"

#include <stdio.h>
#include <string.h>


void push_nonstring(vm* vm, expression_result value)
{
    *(expression_result*)(vm->stack + vm->sp) = value;
    vm->sp += sizeof(expression_result);
    return;
}

void push_string(vm* vm, expression_result value)
{
    char* string_ptr = malloc(value.value.string_value.length);
    memcpy(string_ptr, value.value.string_value.string_value, value.value.string_value.length);
    *(expression_result*)(vm->stack + vm->sp) = (expression_result) {
        .type = STRING_VALUE, 
        .value.string_value = { string_ptr, value.value.string_value.length  }
    };
    vm->sp += sizeof(expression_result);
    return;
}

void store_global(vm* vm, size_t idx, expression_result value)
{
    size_t variable_offset = 0;
    if (idx < vm->free_var_idx)
    {
        expression_result* variable_address = (expression_result*)((char*)vm->environment.variables_memory.data + vm->environment.variable_addrs.data[idx]);

        if (variable_address->type == STRING_VALUE)
            free(variable_address->value.string_value.string_value);

        variable_address->type = value.type;
        variable_address->value = value.value;
        return;
    }

    variable_offset = allocate_vsd_array(&vm->environment.variables_memory, sizeof(expression_result));
    expression_result* variable_address = (expression_result*)((char*)vm->environment.variables_memory.data + variable_offset);
    variable_address->type = value.type;
    variable_address->value = value.value;

    insert_vm_variables_array(&vm->environment.variable_addrs, variable_offset);
    vm->free_var_idx = idx + 1;
}

void load_global(vm* vm, size_t idx)
{
    size_t variable_offset = 0;
    if (idx < vm->free_var_idx)
    {
        variable_offset = vm->environment.variable_addrs.data[idx];
        char* variable_address = (char*)vm->environment.variables_memory.data + variable_offset;
        expression_result variable = *(expression_result*)variable_address;
        if(variable.type == STRING_VALUE)
        {
            push_string(vm, variable);
            return;
        }

        push_nonstring(vm, variable);
        return;
    }

    PRINT_VM_ERROR_AND_QUIT(0, "Cannot find variable at index %ld", idx);
}

void init_vm(vm* vm)
{
    vm->sp = 0;
    vm->pc = 0;
    init_vss_array(&vm->temp_memory, 65535);

    init_vm_variables_array(&vm->environment.variable_addrs, 1024);
    init_vsd_array(&vm->environment.variables_memory, 1024);
    vm->free_var_idx = 0;
}

void destroy_vm(vm* vm)
{
    free_vss_array(&vm->temp_memory);

    for (int i = 0; i < vm->free_var_idx; i++)
    {
        expression_result* global = vm->environment.variables_memory.data + vm->environment.variable_addrs.data[i];
        if (global->type == STRING_VALUE)
            free(global->value.string_value.string_value);
    }

    free_vm_variables_array(&vm->environment.variable_addrs);
    free_vsd_array(&vm->environment.variables_memory);
}

inline expression_result* pop(vm* vm)
{
    expression_result* res = (expression_result*)(vm->stack + vm->sp - sizeof(expression_result));
    vm->sp -= sizeof(expression_result);
    return res;
}

inline void free_if_string(expression_result* val)
{
    if (val->type == STRING_VALUE)
        free(val->value.string_value.string_value);
}

void run_vm(vm* vm, unsigned char* program)
{
    char is_running = 1;
    vm->pc = (*(uint32_t*)program) + 8;
    uint32_t addr, var_idx;

    expression_result* lhs, *rhs, push_val;
    string_type print_str;
    int lhs_bool_result, rhs_bool_result, stack_advance_amount;

    while (is_running)
    {
        clear_vss_array(&vm->temp_memory);
        uint32_t instr = *(uint32_t*)(program + vm->pc);
        vm->pc += 4;

        switch (instr & 0xFF)
        {
            case OPCODE_HALT:
                is_running = 0;
                break;

            case OPCODE_NPUSH:
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = NONE};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_IPUSH:
                addr = instr >> 8;
                int int_val = *(int*)(program + 8 + addr);
                push_val = (expression_result) {.type = INT_VALUE, .value.int_value = int_val};
                push_nonstring(vm, push_val);
                break;

            case OPCODE_FPUSH:
                addr = instr >> 8;
                double float_val = *(double*)(program + 8 + addr);
                push_val = (expression_result) {.type = FLOAT_VALUE, .value.float_value = float_val};
                push_nonstring(vm, push_val);
                break;

            case OPCODE_BPUSH:
                addr = instr >> 8;
                char bool_val = *(char*)(program + 8 + addr);
                push_val = (expression_result) {.type = BOOL_VALUE, .value.bool_value = bool_val};
                push_nonstring(vm, push_val);
                break;

            case OPCODE_SPUSH:
                addr = instr >> 8;
                int string_length = *(int*)(program + 8 + addr);
                char* string_ptr_constants = (char*)(program + 8 + sizeof(int) + addr);
                push_val = (expression_result) {.type = STRING_VALUE, .value.string_value = {string_ptr_constants, string_length}};
                push_string(vm, push_val);
                break;

            case OPCODE_ADD:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = add_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_SUB:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = sub_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_MUL:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = mul_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_DIV:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = div_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_AND:
                rhs = pop(vm);
                lhs = pop(vm);
                rhs_bool_result = cast_to_bool(&vm->temp_memory, *rhs);
                lhs_bool_result = cast_to_bool(&vm->temp_memory, *lhs);
                free_if_string(rhs);
                free_if_string(lhs);
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = BOOL_VALUE, .value.bool_value = rhs_bool_result & lhs_bool_result};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_OR:
                rhs = pop(vm);
                lhs = pop(vm);
                rhs_bool_result = cast_to_bool(&vm->temp_memory, *rhs);
                lhs_bool_result = cast_to_bool(&vm->temp_memory, *lhs);
                free_if_string(rhs);
                free_if_string(lhs);
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = BOOL_VALUE, .value.bool_value = rhs_bool_result | lhs_bool_result};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_EXP:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = exp_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_MOD:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = mod_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_EQ:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = eq_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_NE:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = ne_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_GT:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = gt_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_GE:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = ge_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_LT:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = lt_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_LE:
                rhs = pop(vm);
                lhs = pop(vm);
                stack_advance_amount = le_funcs[lhs->type][rhs->type](&vm->temp_memory, lhs, rhs, (expression_result*)(vm->stack + vm->sp));
                vm->sp += stack_advance_amount;
                break;

            case OPCODE_NUMNEG:
                rhs = pop(vm);
                if (rhs->type == INT_VALUE)
                    *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = INT_VALUE, .value.int_value = -rhs->value.int_value};
                else if (rhs->type == FLOAT_VALUE)
                    *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = FLOAT_VALUE, .value.float_value = -rhs->value.float_value};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_BOOLNEG:
                rhs = pop(vm);
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = BOOL_VALUE, .value.bool_value = !rhs->value.bool_value};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_PRINT:
                rhs = pop(vm);
                print_str = cast_to_string(&vm->temp_memory, *rhs);
                printf("%.*s", print_str.length, print_str.string_value);
                free_if_string(rhs);
                break;

            case OPCODE_PRINTLN:
                rhs = pop(vm);
                print_str = cast_to_string(&vm->temp_memory, *rhs);
                printf("%.*s\n", print_str.length, print_str.string_value);
                free_if_string(rhs);
                break;

            case OPCODE_JMPZ:
                rhs = pop(vm);
                vm->sp += sizeof(expression_result);
                if (rhs->type != BOOL_VALUE)
                {
                    PRINT_ERROR_AND_QUIT("Condition value is not boolean");
                }
                
                if (!rhs->value.bool_value)
                {
                    uint32_t jump_address = instr >> 8;
                    vm->pc = jump_address;
                }

                break;

            case OPCODE_JMP:
                uint32_t jump_address = instr >> 8;
                vm->pc = jump_address;
                break;

            case OPCODE_GLOAD:
                var_idx = instr >> 8;
                load_global(vm, var_idx);
                break; 

            case OPCODE_GSTORE:
                var_idx = instr >> 8;
                rhs = pop(vm);
                store_global(vm, var_idx, *rhs);
                break;
        }
    }
}
