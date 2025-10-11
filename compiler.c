#include "compiler.h"

#include "arrays.h"
#include "hashmap.h"
#include "model.h"
#include "string_type.h"
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
    aligned_target_addr += 8; \
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

void init_compiler(compiler* compiler, char* source_addr, int source_length) 
{
    init_vsd_array(&compiler->program, 0);
    init_label_addr_array(&compiler->label_addrs, 1024);
    init_hashmap(&compiler->symbols, 32, 32);
    init_string_array(&compiler->local_symbol_names, 1024);
    init_uint32_t_array(&compiler->local_symbol_depths, 1024);

    compiler->constants_size = 0;
    compiler->scope_depth = 0;

    compiler->num_symbols = 0;
    compiler->num_local_symbols = 0;

    compiler->source_addr = source_addr;
    compiler->source_length = source_length;
}

void destroy_compiler(compiler* compiler)
{
    // Free up temp function names
    for (int i = 0; i < compiler->symbols.n_buckets; i++)
    {
        for (int j = 0; j < compiler->symbols.bucket_element_count[i]; j++)
        {
            hm_entry* elem = &compiler->symbols.bucket_elements[i + j * compiler->symbols.bucket_capacity];
            if (elem->key.string_value < compiler->source_addr || elem->key.string_value >= compiler->source_addr + compiler->source_length)
            {
                free(elem->key.string_value);
            }
        }
    }
    free_vsd_array(&compiler->program);
    free_label_addr_array(&compiler->label_addrs);
    free_hashmap(&compiler->symbols);
    free_string_array(&compiler->local_symbol_names);
    free_uint32_t_array(&compiler->local_symbol_depths);

    compiler->constants_size = 0;
}

int find_local_symbol(const compiler* compiler, const string_type* key, size_t* value)
{
    for (int i = compiler->num_local_symbols-1; i >= 0; i--)
    {
        if (string_comparison(key, &compiler->local_symbol_names.data[i], COMPARE_EQ))
        {
            *value = i;
            return 0;
        }
    }


    return -1;
}

void destroy_block(compiler* compiler)
{
    size_t arr_offset;
    compiler->scope_depth -= 1;
    for (int i = compiler->num_local_symbols-1; compiler->local_symbol_depths.data[i] > compiler->scope_depth; i--)
    {
        ADD_INSTRUCTION(0x08);
        ADD_INSTRUCTION_PADDING(3);
        compiler->num_local_symbols -= 1;
        compiler->local_symbol_depths.used -= 1;
        compiler->local_symbol_names.used -= 1;
    }
}

void compile(compiler* compiler, void* ast_node)
{
    int element_type = GET_ELEMENT_TYPE(ast_node);
    int element_supertype = GET_ELEMENT_SUPERTYPE(ast_node);
    int element_line = ((Element*)ast_node)->line;

    int func_nargs;
    string_type funcname_str;

    size_t symbol_id = 0, func_symbol_id = 0;
    size_t arr_offset, alloc_size, aligned_target_addr;

    char funcname[261];
    char* temp_funcname;

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

                compiler->scope_depth += 1;
                compile(compiler, if_stmt->then_branch);
                destroy_block(compiler);

                ADD_INSTRUCTION(0x40);
                ADD_LABEL_ID(exit_label); 

                SET_LABEL_ADDR(else_label);
                if (if_stmt->else_branch != NULL)
                {
                    compiler->scope_depth += 1;
                    compile(compiler, if_stmt->else_branch);
                    destroy_block(compiler);
                }

                SET_LABEL_ADDR(exit_label);
                break;

            case While_stmt:
                While* while_stmt = ((While*)ast_node);

                GENERATE_LABEL_ID(while_begin_label);
                GENERATE_LABEL_ID(while_end_label);

                SET_LABEL_ADDR(while_begin_label);
                compile(compiler, while_stmt->condition);
                ADD_INSTRUCTION(0x41);
                ADD_LABEL_ID(while_end_label);

                compiler->scope_depth += 1;
                compile(compiler, while_stmt->statements);
                destroy_block(compiler);

                ADD_INSTRUCTION(0x40);
                ADD_LABEL_ID(while_begin_label);

                SET_LABEL_ADDR(while_end_label);
                break; 

            case Assignment_stmt:
                Assignment* assign_stmt = ((Assignment*)ast_node); 
                Identifier* lhs_identifier = assign_stmt->lhs;

                compile(compiler, assign_stmt->rhs);

                size_t symbol_id_global;
                int local_exists = find_local_symbol(compiler, &lhs_identifier->name, &symbol_id) != -1;
                int global_exists = hashmap_get(&compiler->symbols, &lhs_identifier->name, &symbol_id_global) != -1;

                if (assign_stmt->is_local)
                {
                    if (!local_exists)
                    {
                        insert_string_array(&compiler->local_symbol_names, lhs_identifier->name);
                        insert_uint32_t_array(&compiler->local_symbol_depths, compiler->scope_depth);
                        symbol_id = compiler->num_local_symbols++;
                    }
                    else
                    {
                        ADD_INSTRUCTION(0x31);
                        ADD_LABEL_ID(symbol_id);
                    }
                }

                else
                {
                    if (!local_exists && !global_exists)
                    {
                        if (compiler->scope_depth == 0)
                        {
                            hashmap_set(&compiler->symbols, lhs_identifier->name, compiler->num_symbols);
                            symbol_id = compiler->num_symbols++;
                            ADD_INSTRUCTION(0x21);
                            ADD_LABEL_ID(symbol_id);
                        }
                        else
                        {
                            insert_string_array(&compiler->local_symbol_names, lhs_identifier->name);
                            insert_uint32_t_array(&compiler->local_symbol_depths, compiler->scope_depth);
                            symbol_id = compiler->num_local_symbols++;
                        }
                    }
                    else
                    {
                        if (local_exists)
                        {
                            ADD_INSTRUCTION(0x31);
                            ADD_LABEL_ID(symbol_id);
                        }
                        else
                        {
                            ADD_INSTRUCTION(0x21);
                            ADD_LABEL_ID(symbol_id_global);
                        }
                    }
                }

                break;

            case Return_stmt:
                Return* retstmt = (Return*)ast_node;
                compile(compiler, retstmt->expression);
                ADD_INSTRUCTION(0x43);
                ADD_INSTRUCTION_PADDING(3);
                break;

            case FuncDecl_stmt:
                FuncDecl* funcdecl = (FuncDecl*)ast_node;

                if (funcdecl->name.length > 256)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funcdecl->base.line, "Function names cannot be longer than 256 chars.\n");
                }

                if (funcdecl->num_params > 999)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funcdecl->base.line, "Functions cannot have more than 999 arguments.\n");
                }

                temp_funcname = malloc(261);
                memcpy(temp_funcname, funcdecl->name.string_value, funcdecl->name.length);
                temp_funcname[funcdecl->name.length] = '@';
                sprintf(temp_funcname + funcdecl->name.length + 1, "%d", funcdecl->num_params);
                func_nargs = (funcdecl->num_params < 10) ? 1 : ((funcdecl->num_params < 100) ? 2 : 3);
                funcname_str = (string_type) {temp_funcname, funcdecl->name.length + 1 + func_nargs};

                if (hashmap_get(&compiler->symbols, &funcname_str, &symbol_id) != -1)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funcdecl->base.line, "Function %.*s was already declared", funcname_str.length, funcname);
                }

                GENERATE_LABEL_ID(func_end_label);
                ADD_INSTRUCTION(0x40);
                ADD_LABEL_ID(func_end_label);

                hashmap_set(&compiler->symbols, funcname_str, compiler->temp_code.used);

                compiler->scope_depth += 1;
                string_type* param_ptrs = (string_type*)((char*)(funcdecl) + sizeof(FuncDecl));
                for (int i = 0; i < funcdecl->num_params; i++)
                {
                    insert_string_array(&compiler->local_symbol_names, *param_ptrs++);
                    insert_uint32_t_array(&compiler->local_symbol_depths, compiler->scope_depth);
                    symbol_id = compiler->num_local_symbols++;
                }
                ADD_INSTRUCTION(0x05);
                ADD_LABEL_ID(funcdecl->num_params);
                compile(compiler, funcdecl->statements);
                destroy_block(compiler);

                ADD_INSTRUCTION(0x00);
                ADD_INSTRUCTION_PADDING(3);
                ADD_INSTRUCTION(0x43);
                ADD_INSTRUCTION_PADDING(3);

                SET_LABEL_ADDR(func_end_label);

                break;

            case FuncCall_expr:
                FuncCall* funccall = (FuncCall*)ast_node;

                if (funccall->name.length > 256)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funccall->base.line, "Function names cannot be longer than 256 chars.\n");
                }

                if (funccall->num_args > 999)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funccall->base.line, "Functions cannot have more than 999 arguments.\n");
                }

                memcpy(funcname, funccall->name.string_value, funccall->name.length);
                funcname[funccall->name.length] = '@';
                sprintf(funcname + funccall->name.length + 1, "%d", funccall->num_args);
                func_nargs = (funccall->num_args < 10) ? 1 : ((funccall->num_args < 100) ? 2 : 3);
                funcname_str = (string_type) {funcname, funccall->name.length + 1 + func_nargs};

                if (hashmap_get((&compiler->symbols), &funcname_str, &symbol_id) == -1)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funccall->base.line, "Function %.*s was not declared", funcname_str.length, funcname); 
                }

                ADD_INSTRUCTION(0x05);
                ADD_LABEL_ID(2);

                void** expression_ptrs = (void**)((char*)(funccall) + sizeof(FuncCall));
                for (int i = 0; i < funccall->num_args; i++)
                {
                    compile(compiler, *expression_ptrs++);
                }

                ADD_INSTRUCTION(0x05);
                ADD_LABEL_ID(-(2 + funccall->num_args));
                ADD_INSTRUCTION(0x42);
                ADD_LABEL_ID(symbol_id);
                ADD_INSTRUCTION(0x08);
                ADD_INSTRUCTION_PADDING(3);
                
                break;
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

                if (find_local_symbol(compiler, &identifier_expr->name, &symbol_id) != -1)
                {
                    ADD_INSTRUCTION(0x30);
                    ADD_LABEL_ID(symbol_id);
                    break;
                }

                if (hashmap_get(&compiler->symbols, &identifier_expr->name, &symbol_id) != -1)
                {
                    ADD_INSTRUCTION(0x20);
                    ADD_LABEL_ID(symbol_id);
                    break;
                }

                PRINT_COMPILER_ERROR_AND_QUIT(identifier_expr->base.line, "Undeclared variable %.*s\n", identifier_expr->name.length, identifier_expr->name.string_value);

                break;

            case Grouping_expr:
                compile(compiler, ((Grouping*)ast_node)->expression);
                break;

            case FuncCall_expr:
                FuncCall* funccall = (FuncCall*)ast_node;

                if (funccall->name.length > 256)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funccall->base.line, "Function names cannot be longer than 256 chars.\n");
                }

                if (funccall->num_args > 999)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funccall->base.line, "Functions cannot have more than 999 arguments.\n");
                }

                memcpy(funcname, funccall->name.string_value, funccall->name.length);
                funcname[funccall->name.length] = '@';
                sprintf(funcname + funccall->name.length + 1, "%d", funccall->num_args);
                func_nargs = (funccall->num_args < 10) ? 1 : ((funccall->num_args < 100) ? 2 : 3);
                funcname_str = (string_type) {funcname, funccall->name.length + 1 + func_nargs};

                if (hashmap_get((&compiler->symbols), &funcname_str, &symbol_id) == -1)
                {
                    PRINT_COMPILER_ERROR_AND_QUIT(funccall->base.line, "Function %.*s was not declared", funcname_str.length, funcname); 
                }

                ADD_INSTRUCTION(0x05);
                ADD_LABEL_ID(2);

                void** expression_ptrs = (void**)((char*)(funccall) + sizeof(FuncCall));
                for (int i = 0; i < funccall->num_args; i++)
                {
                    compile(compiler, *expression_ptrs++);
                }

                ADD_INSTRUCTION(0x05);
                ADD_LABEL_ID(-(2 + funccall->num_args));
                ADD_INSTRUCTION(0x42);
                ADD_LABEL_ID(symbol_id);
                
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
                    *(int*)((char*) compiler->program.data + (opcode >> 8))
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
                    *(double*)((char*) compiler->program.data + (opcode >> 8))
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
                    ((char)*((char*) compiler->program.data + (opcode >> 8))) ? "true" : "false"
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
                    *(int*)((char*) compiler->program.data + (opcode >> 8))
                );
                idx += 4;
                break;

            case 0x05:
                printf("            \e[38;2;255;165;0m%02X %02X %02X %02X    %*s    \e[0;32m$%d    \e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "CHASP",
                    (int32_t)opcode >> 8
                );
                idx += 4;
                break;

            case 0x08:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "POP"
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

            case 0x42:
                printf("            \e[38;2;255;165;0m%02X %02X %02X %02X    %*s    \e[0;33m@0x%02X%02X%02X    \e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "JSR",
                    (opcode >> 24) & 0xFF,
                    (opcode >> 16) & 0xFF,
                    (opcode >>  8) & 0xFF
                );
                idx += 4;
                break;

            case 0x43:
                printf("            \e[38;2;255;165;0m%02X %02X %02X %02X    %*s\e[0;37m\n", 
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "RTS"
                );
                idx += 4;
                break;

            case 0x20:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;32m$%d    \e[0;37m\n",
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "LOAD_GLOBAL",
                    opcode >>  8
                );
                idx += 4;
                break;

            case 0x21:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;32m$%d    \e[0;37m\n",
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "STORE_GLOBAL",
                    opcode >>  8
                );
                idx += 4;
                break;

            case 0x30:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;32m$%d    \e[0;37m\n",
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "LOAD_LOCAL",
                    opcode >>  8
                );
                idx += 4;
                break;

            case 0x31:
                printf("            \e[0;34m%02X %02X %02X %02X    %*s    \e[0;32m$%d    \e[0;37m\n",
                    (opcode >>  0) & 0xFF,
                    (opcode >>  8) & 0xFF, 
                    (opcode >> 16) & 0xFF,
                    (opcode >> 24) & 0xFF,
                    15,
                    "STORE_LOCAL",
                    opcode >>  8
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
        if (opcode == 0x42)
        {
            uint32_t target_addr = (instruction >> 8) + 8 + compiler->temp_constants.used;
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
