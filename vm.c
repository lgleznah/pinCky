#include "vm.h"

#include "vm_ops.h"

#include <stdio.h>

void (*add_funcs[5][5]) (vss_array*, expression_result*, expression_result*, expression_result*) = 
{
    {}
}

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

void run_vm(vm* vm, unsigned char* program)
{
    char is_running = 1;
    vm->pc = *(uint32_t*)program;
    uint32_t addr;

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
                char* string_ptr_stack = vm->sp;
                memcpy(vm->sp, string_ptr_constants, string_length);
                vm->sp += (string_length + 23) / 24 * 24;
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {
                    .type = STRING_VALUE, 
                    .value.string_value = { string_length, string_ptr_stack}
                };
                vm->sp += sizeof(expression_result);
                break;
        }
    }
}
