#include "compiler.h"

#include "arrays.h"
#include "hashmap.h"
#include "model.h"
#include "tokens.h"
#include "utils.h"

#include <stdalign.h>
#include <string.h>

#define ADD_INSTRUCTION(opcode) do { \
    arr_offset = allocate_vsd_array(&compiler->temp_code, 1); \
    *(char*)((char*)(compiler->temp_code.data) + arr_offset) = opcode; \
} while(0)

#define ADD_INSTRUCTION_PADDING(padding) do { \
    arr_offset = allocate_vsd_array(&compiler->temp_code, padding); \
    for (int _poff = 0; _poff < padding; _poff++) \
        *(char*)((char*)(compiler->temp_code.data) + arr_offset + _poff) = 0x00; \
} while(0)

#define ADD_ALIGNED_CONSTANT(type, valaddr, size, align) do { \
    alloc_size = size + ((compiler->temp_constants.used + align - 1) / align * align) - compiler->temp_constants.used; \
    arr_offset = allocate_vsd_array(&compiler->temp_constants, alloc_size); \
    aligned_target_addr = arr_offset + alloc_size - size; \
    *((type*) (((char*) compiler->temp_constants.data) + aligned_target_addr)) = *valaddr; \
} while(0)

#define ADD_LAST_CONSTANT_OFFSET do { \
    arr_offset = allocate_vsd_array(&compiler->temp_code, 3); \
    *(char*)((char*)(compiler->temp_code.data) + arr_offset + 0) = aligned_target_addr          & 0xFF; \
    *(char*)((char*)(compiler->temp_code.data) + arr_offset + 1) = (aligned_target_addr >> 8  ) & 0xFF; \
    *(char*)((char*)(compiler->temp_code.data) + arr_offset + 2) = (aligned_target_addr >> 16 ) & 0xFF; \
} while(0)

#define ADD_LABEL_ID(label) do { \
    arr_offset = allocate_vsd_array(&compiler->temp_code, 3); \
    *(char*)((char*)(compiler->temp_code.data) + arr_offset + 0) = label                        & 0xFF; \
    *(char*)((char*)(compiler->temp_code.data) + arr_offset + 1) = (label >> 8  )               & 0xFF; \
    *(char*)((char*)(compiler->temp_code.data) + arr_offset + 2) = (label >> 16 )               & 0xFF; \
} while(0)

#define GENERATE_LABEL_ID(name) \
    uint32_t name = compiler->label_addrs.used; \
    insert_label_addr_array(&compiler->label_addrs, -1)

#define SET_LABEL_ADDR(label) compiler->label_addrs.data[label] = compiler->temp_code.used

void init_compiler(compiler* compiler) 
{
    init_vsd_array(&compiler->program, 0);
    init_label_addr_array(&compiler->label_addrs, 1024);
    init_hashmap(&compiler->symbols, 32, 32);

    compiler->constants_size = 0;
}

void destroy_compiler(compiler* compiler)
{
    free_vsd_array(&compiler->program);
    free_label_addr_array(&compiler->label_addrs);
    free_hashmap(&compiler->symbols);

    compiler->constants_size = 0;
}

void compile(compiler* compiler, void* ast_node)
{
    int element_type = GET_ELEMENT_TYPE(ast_node);
    int element_supertype = GET_ELEMENT_SUPERTYPE(ast_node);
    int element_line = ((Element*)ast_node)->line;

    size_t symbol_id = 0;
    size_t arr_offset, alloc_size, aligned_target_addr;

    if (element_supertype == Statement)
    {
        switch (element_type) 
        {
            case StatementList_stmt:
                void** statement_ptrs = (void**)((char*)(ast_node) + sizeof(StatementList));
                for (size_t i = 0; i < ((StatementList*)(ast_node))->size; i++)
                {
                    compile(compiler, *statement_ptrs++);
                }
                break;

            case Print_stmt:
                compile(compiler, ((Print*)ast_node)->expression);

                if (((Print*)ast_node)->break_line)
                {
                    ADD_INSTRUCTION(0x81);
                    ADD_INSTRUCTION_PADDING(3);
                }

                else
                {
                    ADD_INSTRUCTION(0x80);
                    ADD_INSTRUCTION_PADDING(3);
                }

                break;

            case If_stmt:
                If* if_stmt = ((If*)ast_node);

                compile(compiler, if_stmt->condition);
                GENERATE_LABEL_ID(else_label);
                GENERATE_LABEL_ID(exit_label); 
                ADD_INSTRUCTION(0x41); 
                ADD_LABEL_ID(else_label);

                compile(compiler, if_stmt->then_branch);
                ADD_INSTRUCTION(0x40);
                ADD_LABEL_ID(exit_label); 

                SET_LABEL_ADDR(else_label);
                if (if_stmt->else_branch != NULL)
                {
                    compile(compiler, if_stmt->else_branch);
                }

                SET_LABEL_ADDR(exit_label);
                break;

            case Assignment_stmt:
                Assignment* assign_stmt = ((Assignment*)ast_node); 
                Identifier* lhs_identifier = assign_stmt->lhs;

                compile(compiler, assign_stmt->rhs);

<<<<<<< HEAD
                size_t symbol_id = 0;
=======
>>>>>>> 7a645d3c70e92c6b7396c75d05a284be10a3bc99
                if (hashmap_get(&compiler->symbols, &lhs_identifier->name, &symbol_id) == -1)
                {
                    hashmap_set(&compiler->symbols, lhs_identifier->name, compiler->num_symbols);
                    symbol_id = compiler->num_symbols++;
                }

                ADD_INSTRUCTION(0x21);
<<<<<<< HEAD
                ADD_LABEL_ID(symbol_id); 

=======
                ADD_LABEL_ID(symbol_id);
                break;
>>>>>>> 7a645d3c70e92c6b7396c75d05a284be10a3bc99
        }
    }

    else if (element_supertype == Expression)
    {
        switch (element_type)
        {
            case Integer_expr:
                int int_val = ((Integer*)ast_node)->value;
                ADD_INSTRUCTION(0x01);
                ADD_ALIGNED_CONSTANT(int, &int_val, sizeof(int), alignof(int));
                ADD_LAST_CONSTANT_OFFSET;
                break;

            case Float_expr:
                double double_val = ((Float*)ast_node)->value;
                ADD_INSTRUCTION(0x02);
                ADD_ALIGNED_CONSTANT(double, &double_val, sizeof(double), alignof(double));
                ADD_LAST_CONSTANT_OFFSET;
                break;


            case Bool_expr:
                char bool_val = ((Bool*)ast_node)->value;
                ADD_INSTRUCTION(0x03);
                ADD_ALIGNED_CONSTANT(char, &bool_val, sizeof(char), alignof(char));
                ADD_LAST_CONSTANT_OFFSET;
                break;

            case String_expr:
                string_type string_val = ((String*)ast_node)->value;
                ADD_INSTRUCTION(0x04);
                ADD_ALIGNED_CONSTANT(int, &string_val.length, sizeof(int), alignof(int));
                ADD_LAST_CONSTANT_OFFSET;

                arr_offset = allocate_vsd_array(&compiler->temp_constants, string_val.length);
                for (int i = 0; i < string_val.length; i++)
                {
                    *(((char*)compiler->temp_constants.data) + arr_offset + i) = *(string_val.string_value + i); 
                }
                break;

            case Identifier_expr:
                Identifier* identifier_expr = ((Identifier*)ast_node);

<<<<<<< HEAD
                size_t symbol_id = 0;
                if (hashmap_get(&compiler->symbols, &identifier_expr->name, &symbol_id) == -1)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(identifier_expr->base.line, "Cannot find variable %*.s", identifier_expr->name.length, identifier_expr->name.string_value);
                }

                ADD_INSTRUCTION(0x20);
                ADD_LABEL_ID(symbol_id); 

=======
                if (hashmap_get(&compiler->symbols, &identifier_expr->name, &symbol_id) == -1)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(identifier_expr->base.line, "Undeclared variable %.*s\n", identifier_expr->name.length, identifier_expr->name.string_value);
                }

                ADD_INSTRUCTION(0x20);
                ADD_LABEL_ID(symbol_id);
                break;
>>>>>>> 7a645d3c70e92c6b7396c75d05a284be10a3bc99

            case Grouping_expr:
                compile(compiler, ((Grouping*)ast_node)->expression);
                break;

            case UnOp_expr:
                compile(compiler, ((UnOp*)ast_node)->operand);
                token_type unop_op = ((UnOp*)ast_node)->op;

                switch (unop_op) 
                {
                    case TOK_MINUS:
                        ADD_INSTRUCTION(0x16);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_NOT:
                        ADD_INSTRUCTION(0x17);
                        ADD_INSTRUCTION_PADDING(3);
                        break;
                }
                break;

            case BinOp_expr:
                compile(compiler, ((BinOp*)ast_node)->left);
                compile(compiler, ((BinOp*)ast_node)->right);
                token_type binop_op = ((BinOp*)ast_node)->op;

                switch (binop_op)
                {
                    case TOK_PLUS:
                        ADD_INSTRUCTION(0x10);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_MINUS:
                        ADD_INSTRUCTION(0x11);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_STAR:
                        ADD_INSTRUCTION(0x12);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_SLASH:
                        ADD_INSTRUCTION(0x13);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_OR:
                        ADD_INSTRUCTION(0x14);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_AND:
                        ADD_INSTRUCTION(0x15);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_CARET:
                        ADD_INSTRUCTION(0x18);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_MOD:
                        ADD_INSTRUCTION(0x19);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_EQEQ:
                        ADD_INSTRUCTION(0x1A);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_NE:
                        ADD_INSTRUCTION(0x1B);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_GT:
                        ADD_INSTRUCTION(0x1C);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_GE:
                        ADD_INSTRUCTION(0x1D);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_LT:
                        ADD_INSTRUCTION(0x1E);
                        ADD_INSTRUCTION_PADDING(3);
                        break;

                    case TOK_LE:
                        ADD_INSTRUCTION(0x1F);
                        ADD_INSTRUCTION_PADDING(3);
                        break;
                }
                
                break;
        }
    }

    else
    {
        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unknown element supertype ID %d\n", element_supertype);
    }
}

void print_code(compiler* compiler)
{
    // Print constants section
    unsigned int idx = 0;
    printf("\n\nPROGRAM CONSTANTS SECTION:\n\n");
    printf("               \e[0;33m00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\e[0;37m\n");
    printf("               -----------------------------------------------\n");

    while (idx < compiler->constants_size + 8)
    {
        if (idx % 16 == 0) 
        {
            if (idx != 0)
            {
                printf("\n");
            }
            printf("\e[0;33m(0x%08X)\e[0;37m | ", idx);
        }
        printf("\e[0;32m%02X \e[0;37m", *((unsigned char*) compiler->program.data + idx));
        idx += 1;
    }

    printf("\n\nPROGRAM TEXT SECTION:\n\n");
    idx = compiler->constants_size + 8;
    while (idx < compiler->program.used)
    {
        uint32_t opcode = *(uint32_t*)((char*) compiler->program.data + idx);
        printf("\e[0;33m(0x%08X)\e[0;37m  ", idx);
        switch (opcode & 0xFF)
        {
            case 0x00:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "PUSH_NONE"
                );
                idx += 4;
                break;

            case 0x01:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;33m@0x%02X%02X%02X    \e[0;32m(%d)\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "PUSH_INTEGER",
                    (opcode >> 24) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >>  8) & 0xFF,
                    *(int*)((char*) compiler->program.data + 8 + (opcode >> 8))
                );
                idx += 4;
                break;

            case 0x02:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;33m@0x%02X%02X%02X    \e[0;32m(%f)\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "PUSH_FLOAT",
                    (opcode >> 24) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >>  8) & 0xFF,
                    *(double*)((char*) compiler->program.data + 8 + (opcode >> 8))
                );
                idx += 4;
                break;

            case 0x03:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;33m@0x%02X%02X%02X    \e[0;32m(%s)\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "PUSH_BOOL",
                    (opcode >> 24) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >>  8) & 0xFF,
                    ((char)*((char*) compiler->program.data + 8 + (opcode >> 8))) ? "true" : "false"
                );
                idx += 4;
                break;

            case 0x04:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;33m@0x%02X%02X%02X    \e[0;32m(%d bytes of string)\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "PUSH_STRING",
                    (opcode >> 24) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >>  8) & 0xFF,
                    *(int*)((char*) compiler->program.data + 8 + (opcode >> 8))
                );
                idx += 4;
                break;

            case 0x10:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "ADD"
                );
                idx += 4;
                break;


            case 0x11:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "SUB"
                );
                idx += 4;
                break;


            case 0x12:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "MUL"
                );
                idx += 4;
                break;


            case 0x13:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "DIV"
                );
                idx += 4;
                break;


            case 0x14:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "OR"
                );
                idx += 4;
                break;


            case 0x15:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "AND"
                );
                idx += 4;
                break;


            case 0x16:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "NUMNEG"
                );
                idx += 4;
                break;


            case 0x17:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "BOOLNEG"
                );
                idx += 4;
                break;


            case 0x18:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "EXP"
                );
                idx += 4;
                break;


            case 0x19:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "MOD"
                );
                idx += 4;
                break;


            case 0x1A:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "EQ"
                );
                idx += 4;
                break;


            case 0x1B:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "NE"
                );
                idx += 4;
                break;


            case 0x1C:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "GT"
                );
                idx += 4;
                break;


            case 0x1D:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "GE"
                );
                idx += 4;
                break;


            case 0x1E:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "LT"
                );
                idx += 4;
                break;


            case 0x1F:
                printf("            \e[0;36m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "LE"
                );
                idx += 4;
                break;

            case 0x80:
                printf("            \e[0;37m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "PRINT"
                );
                idx += 4;
                break;


            case 0x81:
                printf("            \e[0;37m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "PRINTLN"
                );
                idx += 4;
                break;

            case 0x69:
                printf("            \e[0;31m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "HALT"
                );
                idx += 4;
                break;

            case 0x41:
                printf("            \e[0;35m%02X %02X %02X %02X    %*s    \e[0;33m@0x%02X%02X%02X    \e[0;37m\n",
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "JMPZ",
                    (opcode >> 24) & 0xFF,
                    (opcode >> 16) & 0xFF,
                    (opcode >>  8) & 0xFF
                );
                idx += 4;
                break;

            case 0x40:
                printf("            \e[0;35m%02X %02X %02X %02X    %*s    \e[0;33m@0x%02X%02X%02X    \e[0;37m\n",
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "JMP",
                    (opcode >> 24) & 0xFF,
                    (opcode >> 16) & 0xFF,
                    (opcode >>  8) & 0xFF
                );
                idx += 4;
                break;

            case 0x20:
<<<<<<< HEAD
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;32m$%d    \e[0;37m\n",
=======
                printf("            \e[0;35m%02X %02X %02X %02X    %*s    \e[0;32m#%d    \e[0;37m\n",
>>>>>>> 7a645d3c70e92c6b7396c75d05a284be10a3bc99
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "LOAD_GLOBAL",
<<<<<<< HEAD
                    opcode >>  8
                );
                idx += 4;
                break;

            case 0x21:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;32m$%d    \e[0;37m\n",
=======
                    (opcode >> 8)
                );
                idx += 4;
                break;
                
            case 0x21:
                printf("            \e[0;35m%02X %02X %02X %02X    %*s    \e[0;32m#%d    \e[0;37m\n",
>>>>>>> 7a645d3c70e92c6b7396c75d05a284be10a3bc99
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "STORE_GLOBAL",
<<<<<<< HEAD
                    opcode >>  8
=======
                    (opcode >> 8)
>>>>>>> 7a645d3c70e92c6b7396c75d05a284be10a3bc99
                );
                idx += 4;
                break;

            default:
                PRINT_ERROR_AND_QUIT("Unrecognized opcode %02X", opcode);
        }
    }
}

void solve_label_addrs(compiler* compiler)
{
    uint32_t idx = compiler->constants_size + 8;
    while (idx < compiler->program.used)
    {
        uint32_t instruction = *(uint32_t*)((char*) compiler->program.data + idx);
        uint32_t opcode = instruction & 0xFF;

        if (opcode == 0x40 || opcode == 0x41)
        {
            uint32_t target_addr = compiler->label_addrs.data[instruction >> 8] + 8 + compiler->temp_constants.used;
            *(uint32_t*)((char*) compiler->program.data + idx) = opcode | (target_addr << 8);
        }

        idx += 4;
    }
}

unsigned char* compile_code(compiler* compiler, void* ast_node)
{
    size_t arr_offset;

    init_vsd_array(&compiler->temp_constants, 0);
    init_vsd_array(&compiler->temp_code, 0);

    compile(compiler, ast_node);
    ADD_INSTRUCTION(0x69);
    size_t alloc_size = ((compiler->temp_constants.used + 4 - 1) / 4 * 4) - compiler->temp_constants.used;
    allocate_vsd_array(&compiler->temp_constants, alloc_size);

    // Even though the constants_size value is only 4 bytes, 8 are allocated in order not to break the
    // alignment of any of the other values (as doubles, the most strictly aligned, have an alignment of 8)
    allocate_vsd_array(&compiler->program, 8 + compiler->temp_constants.used + compiler->temp_code.used);
    compiler->constants_size = compiler->temp_constants.used;
    *(uint32_t*)(compiler->program.data) = compiler->constants_size;
    compiler->program.used = 8 + compiler->temp_constants.used + compiler->temp_code.used;
    memcpy(compiler->program.data + 8, compiler->temp_constants.data, compiler->temp_constants.used);
    memcpy(compiler->program.data + compiler->temp_constants.used + 8, compiler->temp_code.data, compiler->temp_code.used);

    // Replace previously generated labels with their definitive values
    solve_label_addrs(compiler);

    free_vsd_array(&compiler->temp_constants);
    free_vsd_array(&compiler->temp_code);

    return compiler->program.data;
}
