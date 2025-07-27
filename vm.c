#include "vm.h"

#include <stdio.h>

void init_vm(vm* vm)
{
    vm->sp = 0;
    vm->pc = 0;
}

void destroy_vm(vm* vm)
{
    
}

void run_vm(vm* vm, unsigned char* program)
{
    char is_running = 1;
    vm->pc = *(uint32_t*)program;
    uint32_t addr;

    while (is_running)
    {
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
                *(expression_result*)(vm->stack + vm->sp) = (expression_result) {.type = BOOL_VALUE, .value.bool_value = bool_val};
                vm->sp += sizeof(expression_result);
                break;
        }
    }
}
