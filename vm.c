#include "vm.h"

#include "compiler_commons.h"
#include "types.h"
#include "vm_ops.h"

#include <stdio.h>
#include <string.h>


void init_vm(vm* vm)
{
    vm->sp = 0;
    vm->pc = 0;
    init_vss_array(&vm->temp_memory, 65535);
}

void destroy_vm(vm* vm)
{
    free_vss_array(&vm->temp_memory);
}

inline expression_result* pop(vm* vm)
{
    expression_result* res = (expression_result*)(vm->stack + vm->sp - sizeof(expression_result));
    switch (res->type)
    {
        case NONE:
        case INT_VALUE:
        case FLOAT_VALUE:
        case BOOL_VALUE:
            vm->sp -= sizeof(expression_result);
            break;
        

        case STRING_VALUE:
            int length = res->value.string_value.length; 
            vm->sp -= (length + 23) / 24 * 24 + 24;
            break;
    }

    return res;
}

void run_vm(vm* vm, unsigned char* program)
{
    char is_running = 1;
    vm->pc = (*(uint32_t*)program) + 8;
    uint32_t addr;

    expression_result* lhs, *rhs;
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
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = INT_VALUE, .value.int_value = int_val};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_FPUSH:
                addr = instr >> 8;
                double float_val = *(double*)(program + 8 + addr);
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = FLOAT_VALUE, .value.float_value = float_val};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_BPUSH:
                addr = instr >> 8;
                char bool_val = *(char*)(program + 8 + addr);
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = BOOL_VALUE, .value.bool_value = bool_val};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_SPUSH:
                addr = instr >> 8;
                int string_length = *(int*)(program + 8 + addr);
                char* string_ptr_constants = (char*)(program + 8 + sizeof(int) + addr);
                char* string_ptr_stack = (char*)(vm->stack + vm->sp);
                memcpy(string_ptr_stack, string_ptr_constants, string_length);
                vm->sp += (string_length + 23) / 24 * 24;
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {
                    .type = STRING_VALUE, 
                    .value.string_value = { string_ptr_stack, string_length }
                };
                vm->sp += sizeof(expression_result);
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
                rhs_bool_result = cast_to_bool(&vm->temp_memory, *pop(vm));
                lhs_bool_result = cast_to_bool(&vm->temp_memory, *pop(vm));
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = BOOL_VALUE, .value.bool_value = rhs_bool_result & lhs_bool_result};
                vm->sp += sizeof(expression_result);
                break;

            case OPCODE_OR:
                rhs_bool_result = cast_to_bool(&vm->temp_memory, *pop(vm));
                lhs_bool_result = cast_to_bool(&vm->temp_memory, *pop(vm));
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
                break;

            case OPCODE_PRINTLN:
                rhs = pop(vm);
                print_str = cast_to_string(&vm->temp_memory, *rhs);
                printf("%.*s\n", print_str.length, print_str.string_value);
                break;
        }
    }
}
