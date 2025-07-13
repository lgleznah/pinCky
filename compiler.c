#include "compiler.h"

#include "model.h"
#include "tokens.h"
#include "utils.h"

typedef union
{
    int num;
    unsigned char bytes[4];
} int_bytes;

typedef union
{
    double num;
    unsigned char bytes [8];
} double_bytes;

void init_compiler(compiler* compiler) 
{
    compiler->code_idx = 0;
}

void destroy_compiler(compiler* compiler)
{

}

void compile(compiler* compiler, void* ast_node)
{
    int element_type = GET_ELEMENT_TYPE(ast_node);
    int element_supertype = GET_ELEMENT_SUPERTYPE(ast_node);
    int element_line = ((Element*)ast_node)->line;

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
                    compiler->code[compiler->code_idx++] = 0x81;
                }

                else
                {
                    compiler->code[compiler->code_idx++] = 0x80;
                }

                break;

        }
    }

    else if (element_supertype == Expression)
    {
        switch (element_type)
        {
            case Integer_expr:
                int_bytes int_val_bytes = { .num = ((Integer*)ast_node)->value };
                compiler->code[compiler->code_idx++] = 0x01;
                compiler->code[compiler->code_idx++] = int_val_bytes.bytes[0] & 0xFF;
                compiler->code[compiler->code_idx++] = int_val_bytes.bytes[1] & 0xFF;
                compiler->code[compiler->code_idx++] = int_val_bytes.bytes[2] & 0xFF;
                compiler->code[compiler->code_idx++] = int_val_bytes.bytes[3] & 0xFF;
                break;

            case Float_expr:
                double_bytes double_val_bytes = { .num = ((Float*)ast_node)->value };
                compiler->code[compiler->code_idx++] = 0x02;
                compiler->code[compiler->code_idx++] = double_val_bytes.bytes[0];
                compiler->code[compiler->code_idx++] = double_val_bytes.bytes[1];
                compiler->code[compiler->code_idx++] = double_val_bytes.bytes[2];
                compiler->code[compiler->code_idx++] = double_val_bytes.bytes[3];
                compiler->code[compiler->code_idx++] = double_val_bytes.bytes[4];
                compiler->code[compiler->code_idx++] = double_val_bytes.bytes[5];
                compiler->code[compiler->code_idx++] = double_val_bytes.bytes[6];
                compiler->code[compiler->code_idx++] = double_val_bytes.bytes[7];
                break;


            case Bool_expr:
                compiler->code[compiler->code_idx++] = 0x03;
                compiler->code[compiler->code_idx++] = ((Bool*)ast_node)->value & 0xFF;
                break;

            case String_expr:
                int_bytes str_len_bytes = { .num = ((String*)ast_node)->value.length };
                char* str_pointer = ((String*)ast_node)->value.string_value;
                compiler->code[compiler->code_idx++] = 0x04;
                compiler->code[compiler->code_idx++] = str_len_bytes.bytes[0] & 0xFF;
                compiler->code[compiler->code_idx++] = str_len_bytes.bytes[1] & 0xFF;
                compiler->code[compiler->code_idx++] = str_len_bytes.bytes[2] & 0xFF;
                compiler->code[compiler->code_idx++] = str_len_bytes.bytes[3] & 0xFF;
                for (int i = 0; i < str_len_bytes.num; i++)
                {
                    compiler->code[compiler->code_idx++] = *str_pointer++;
                }
                break;

            case UnOp_expr:
                compile(compiler, ((UnOp*)ast_node)->operand);
                token_type unop_op = ((UnOp*)ast_node)->op;

                switch (unop_op) 
                {
                    case TOK_MINUS:
                        compiler->code[compiler->code_idx++] = 0x16;
                        break;

                    case TOK_NOT:
                        compiler->code[compiler->code_idx++] = 0x17;
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
                        compiler->code[compiler->code_idx++] = 0x10;
                        break;

                    case TOK_MINUS:
                        compiler->code[compiler->code_idx++] = 0x11;
                        break;

                    case TOK_STAR:
                        compiler->code[compiler->code_idx++] = 0x12;
                        break;

                    case TOK_SLASH:
                        compiler->code[compiler->code_idx++] = 0x13;
                        break;

                    case TOK_OR:
                        compiler->code[compiler->code_idx++] = 0x14;
                        break;

                    case TOK_AND:
                        compiler->code[compiler->code_idx++] = 0x15;
                        break;

                    case TOK_CARET:
                        compiler->code[compiler->code_idx++] = 0x18;
                        break;

                    case TOK_MOD:
                        compiler->code[compiler->code_idx++] = 0x19;
                        break;

                    case TOK_EQEQ:
                        compiler->code[compiler->code_idx++] = 0x1A;
                        break;

                    case TOK_NE:
                        compiler->code[compiler->code_idx++] = 0x1B;
                        break;

                    case TOK_GT:
                        compiler->code[compiler->code_idx++] = 0x1C;
                        break;

                    case TOK_GE:
                        compiler->code[compiler->code_idx++] = 0x1D;
                        break;

                    case TOK_LT:
                        compiler->code[compiler->code_idx++] = 0x1E;
                        break;

                    case TOK_LE:
                        compiler->code[compiler->code_idx++] = 0x1F;
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
    unsigned int idx = 0;
    while (idx < compiler->code_idx)
    {
        unsigned char opcode = compiler->code[idx];
        printf("(0x%08X)  ", idx);
        switch (opcode)
        {
            case 0x00:
                printf("                        %02X    PUSH_NONE\n", opcode);
                idx += 1;
                break;

            case 0x01:
                int_bytes int_val_bytes = { .bytes = {compiler->code[idx+1], compiler->code[idx+2], compiler->code[idx+3], compiler->code[idx+4]} };
                printf("            %02X %02X %02X %02X %02X    PUSH_INTEGER (%d)\n", 
                    opcode, 
                    int_val_bytes.bytes[0],
                    int_val_bytes.bytes[1],
                    int_val_bytes.bytes[2],
                    int_val_bytes.bytes[3],
                    int_val_bytes.num
                );
                idx += 5;
                break;

            case 0x02:
                double_bytes double_val_bytes = { .bytes = {
                    compiler->code[idx+1], compiler->code[idx+2], compiler->code[idx+3], compiler->code[idx+4],
                    compiler->code[idx+5], compiler->code[idx+6], compiler->code[idx+7], compiler->code[idx+8]
                } };
                printf("%02X %02X %02X %02X %02X %02X %02X %02X %02X    PUSH_FLOAT (%f)\n", 
                    opcode, 
                    double_val_bytes.bytes[0],
                    double_val_bytes.bytes[1],
                    double_val_bytes.bytes[2],
                    double_val_bytes.bytes[3],
                    double_val_bytes.bytes[4],
                    double_val_bytes.bytes[5],
                    double_val_bytes.bytes[6],
                    double_val_bytes.bytes[7],
                    double_val_bytes.num
                );
                idx += 9;
                break;

            case 0x03:
                printf("                     %02X %02X    PUSH_BOOL (%s)\n", 
                    opcode, 
                    compiler->code[idx+1],
                    (compiler->code[idx+1]) ? "True" : "False"
                );
                idx += 2;
                break;

            case 0x04:
                int_bytes str_len_bytes = { .bytes = {compiler->code[idx+1], compiler->code[idx+2], compiler->code[idx+3], compiler->code[idx+4]} };
                printf("                     %02X ..    PUSH_STRING (%d bytes of string)\n",
                    opcode,
                    str_len_bytes.num
                );
                idx += 5 + str_len_bytes.num;
                break;

            case 0x10:
                printf("                        %02X    ADD\n", opcode);
                idx += 1;
                break;


            case 0x11:
                printf("                        %02X    SUB\n", opcode);
                idx += 1;
                break;


            case 0x12:
                printf("                        %02X    MUL\n", opcode);
                idx += 1;
                break;


            case 0x13:
                printf("                        %02X    DIV\n", opcode);
                idx += 1;
                break;


            case 0x14:
                printf("                        %02X    OR\n", opcode);
                idx += 1;
                break;


            case 0x15:
                printf("                        %02X    AND\n", opcode);
                idx += 1;
                break;


            case 0x16:
                printf("                        %02X    NUMNEG\n", opcode);
                idx += 1;
                break;


            case 0x17:
                printf("                        %02X    BOLNEG\n", opcode);
                idx += 1;
                break;


            case 0x18:
                printf("                        %02X    EXP\n", opcode);
                idx += 1;
                break;


            case 0x19:
                printf("                        %02X    MOD\n", opcode);
                idx += 1;
                break;


            case 0x1A:
                printf("                        %02X    EQ\n", opcode);
                idx += 1;
                break;


            case 0x1B:
                printf("                        %02X    NE\n", opcode);
                idx += 1;
                break;


            case 0x1C:
                printf("                        %02X    GT\n", opcode);
                idx += 1;
                break;


            case 0x1D:
                printf("                        %02X    GE\n", opcode);
                idx += 1;
                break;


            case 0x1E:
                printf("                        %02X    LT\n", opcode);
                idx += 1;
                break;


            case 0x1F:
                printf("                        %02X    LE\n", opcode);
                idx += 1;
                break;

            case 0x80:
                printf("                        %02X    PRINT\n", opcode);
                idx += 1;
                break;


            case 0x81:
                printf("                        %02X    PRINTLN\n", opcode);
                idx += 1;
                break;

            case 0x69:
                printf("                        %02X    HALT\n", opcode);
                idx += 1;
                break;

            default:
                PRINT_ERROR_AND_QUIT("Unrecognized opcode %02X", opcode);
        }
    }
}

unsigned char* compile_code(compiler* compiler, void* ast_node)
{
    compile(compiler, ast_node);
    compiler->code[compiler->code_idx++] = 0x69;
    return compiler->code;
}
