#pragma once

#include "compiler_commons.h"

// The VM consists of a single stack.

// Opcodes are 1 byte each (potentially followed by a certain value).
// Instructions to push and pop from the stack have their upper half set to 0,
// the 4th LSB indicates whether it is a push or a pop, and the three LSBs indicate
// the type of value that will be pushed.

//      0000 0xxx (3 byte address)      -> PUSH instruction
//      0000 1xxx (3 byte address)      ->  POP instruction

//      0000 x000                       -> NPUSH/NPOP (POP/PUSH None value)
//      0000 x001 (3 byte address)      -> IPUSH/IPOP (POP/PUSH Integer)
//      0000 x010 (3 byte address)      -> FPUSH/FPOP (POP/PUSH Float)
//      0000 x011 (3 byte address)      -> BPUSH/BPOP (POP/PUSH Boolean)
//      0000 x100 (3 byte address)      -> SPUSH/SPOP (POP/PUSH String)

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
//      0001 0110       -> NUMNEG   (Negate number)
//      0001 0111       -> BOLNEG   (Negate boolean)
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
//      0110 1001                       -> HALT             (Halts the VM, nicely)

// Special instructions.

//      1000 0000                       -> PRINT            (Print top of the stack)
//      1000 0001                       -> PRINTLN          (Print top of the stack with newline)


// For binops and unops, the types of the operands are used as indices to a jump table
// which determines what function to employ for executing the operation, according to the following tables:
//
//    +    |  none  | integer | float |  bool  | string | 
// --------+--------+---------+-------+--------+--------+
//    none |   ERR  |   ERR   |  ERR  |  ERR   |  SADD  |          ERR: error (unsupported)
// --------+--------+---------+-------+--------+--------+          IADD: integer addition
// integer |   ERR  |   IADD  |  FADD |  ERR   |  SADD  |          FADD: float addition
// --------+--------+---------+-------+--------+--------+          SADD: string addition
//   float |   ERR  |   FADD  |  FADD |  ERR   |  SADD  |
// --------+--------+---------+-------+--------+--------+
//    bool |   ERR  |   ERR   |  ERR  |  ERR   |  SADD  |
// --------+--------+---------+-------+--------+--------+
//  string |  SADD  |   SADD  |  SADD |  SADD  |  SADD  |
// --------+--------+---------+-------+--------+--------+
// 
//
//  -*/%^  |  none  | integer | float |  bool  | string | 
// --------+--------+---------+-------+--------+--------+
//    none |   ERR  |   ERR   |  ERR  |  ERR   |   ERR  |          ERR: error (unsupported)
// --------+--------+---------+-------+--------+--------+          I*: integer operation
// integer |   ERR  |   I*    |  F*   |  ERR   |   ERR  |          F*: float operation
// --------+--------+---------+-------+--------+--------+          
//   float |   ERR  |   F*    |  F*   |  ERR   |   ERR  |          (*) can be any of
// --------+--------+---------+-------+--------+--------+              SUB, MUL, DIV, MOD, EXP
//    bool |   ERR  |   ERR   |  ERR  |  ERR   |   ERR  |
// --------+--------+---------+-------+--------+--------+
//  string |   ERR  |   ERR   |  ERR  |  ERR   |   ERR  |
// --------+--------+---------+-------+--------+--------+
//
//
//   comp* |  none  | integer | float |  bool  | string | 
// --------+--------+---------+-------+--------+--------+
//    none |   ERR  |   ERR   |  ERR  |  ERR   |   ERR  |          ERR: error (unsupported)
// --------+--------+---------+-------+--------+--------+          I*: integer comparison
// integer |   ERR  |   I*    |  F*   |  I*    |   ERR  |          F*: float comparison
// --------+--------+---------+-------+--------+--------+          S*: string comparison
//   float |   ERR  |   F*    |  F*   |  F*    |   ERR  |
// --------+--------+---------+-------+--------+--------+          (*) can be any of
//    bool |   ERR  |   I*    |  F*   |  I*    |   ERR  |              EQ, NE, GT, LE, LT, LE
// --------+--------+---------+-------+--------+--------+
//  string |   ERR  |   ERR   |  ERR  |  ERR   |   S*   |          For AND/OR, the operand is cast to a bool.
// --------+--------+---------+-------+--------+--------+          All types have a defined bool casting.
//
// 
//
#define STACK_SIZE 65536
#include <stdint.h>

#define OPCODE_NPUSH   0x00
#define OPCODE_IPUSH   0x01
#define OPCODE_FPUSH   0x02
#define OPCODE_BPUSH   0x03
#define OPCODE_SPUSH   0x04
#define OPCODE_ADD     0x10
#define OPCODE_SUB     0x11
#define OPCODE_MUL     0x12
#define OPCODE_DIV     0x13
#define OPCODE_OR      0x14
#define OPCODE_AND     0x15
#define OPCODE_NUMNEG  0x16
#define OPCODE_BOOLNEG 0x17
#define OPCODE_EXP     0x18
#define OPCODE_MOD     0x19
#define OPCODE_EQ      0x1A
#define OPCODE_NE      0x1B
#define OPCODE_GT      0x1C
#define OPCODE_GE      0x1D
#define OPCODE_LT      0x1E
#define OPCODE_LE      0x1F
#define OPCODE_PRINT   0x80
#define OPCODE_PRINTLN 0x81
#define OPCODE_HALT    0x69

typedef struct vm
{
    char stack[STACK_SIZE];
    vss_array temp_memory;
    uint32_t sp;
    uint32_t pc;
} vm;

void init_vm(vm* vm);
void destroy_vm(vm* vm);

void run_vm(vm* vm, unsigned char* program);
