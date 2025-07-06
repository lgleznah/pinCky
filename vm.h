#pragma once

// The VM consists of a single stack.

// Opcodes are 1 byte each (potentially followed by a certain value).
// Instructions to push and pop from the stack have their upper half set to 0,
// the 4th LSB indicates whether it is a push or a pop, and the three LSBs indicate
// the type of value that will be pushed.

//      0000 0xxx       -> PUSH instruction
//      0000 1xxx       ->  POP instruction

//      0000 x000       -> NPUSH/NPOP (POP/PUSH None value)
//      0000 x001       -> IPUSH/IPOP (POP/PUSH Integer)
//      0000 x010       -> FPUSH/FPOP (POP/PUSH Float)
//      0000 x011       -> BPUSH/BPOP (POP/PUSH Boolean)
//      0000 x100       -> SPUSH/SPOP (POP/PUSH String)

// Stack values are tagged with a one-byte tag, the same as the one specified in the
// result_type enum. The stack grows from the smallest address of an array towards
// the largest address, and the tag ALWAYS follows its value in this array.

// Instructions to add, subtract, multiply, divide, and compare values from the top of the stack will
// automatically take into account the types of their values to produce the correct result.

//      0001 0000       -> ADD      (Addition)
//      0001 0001       -> SUB      (Subtraction)
//      0001 0010       -> MUL      (Multiplication)
//      0001 0011       -> DIV      (Division)
//      0001 0100       -> OR       (Bitwise OR)
//      0001 0101       -> AND      (Bitwise AND)
//      0001 0110       -> XOR      (Bitwise XOR)
//      0001 0111       -> NEG      (Negate)
//      0001 1000       -> EXP      (Exponentiation)
//      0001 1001       -> MOD      (Modulo)
//      0001 1010       -> EQ       (Compare ==)
//      0001 1011       -> NE       (Compare !=)
//      0001 1100       -> GT       (Compare >)
//      0001 1101       -> GE       (Compare >=)
//      0001 1110       -> LT       (Compare <)
//      0001 1111       -> LE       (Compare <=)

// So, to perform 7 + 2 * 3, one would do:

//      IPUSH 7
//      IPUSH 2
//      IPUSH 3
//      MUL
//      ADD

// Variable load/store functions

//      0010 0000  <16-bit number>      -> LOAD n           (Push globals[n] to the stack)
//      0010 0001  <16-bit number>      -> STORE n          (Pop the stack into globals[n])
//      0011 0000  <16-bit number>      -> LOAD_LOCAL n     (Push locals[n] to the stack)
//      0011 0001  <16-bit number>      -> STORE_LOCAL n    (Pop the stack into locals[n])

// Flow control instructions.

//      0100 0000  <16-bit number>      -> JMP addr         (Unconditional jump to address)
//      0100 0001  <16-bit number>      -> JMPZ addr        (Jump to address if top of stack is 0/false)
//      0100 0010  <16-bit number>      -> JSR addr         (Jump to subroutine and store PC)
//      0100 0011                       -> RTS              (Return from subroutine)

#define STACK_SIZE 65536
#include <cstdint>

typedef struct vm
{
    char stack[STACK_SIZE];
    uint32_t sp;
    uint32_t pc;
} vm;

void init_vm(vm* vm);
void destroy_vm(vm* vm);

void execute(vm* vm, char* program);
