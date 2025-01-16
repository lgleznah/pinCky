#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define AST_PRINT_PAD(depth) (printf("%*s", (depth)*4, ""))
#define UPDATE_PTR(ptr) ((ptr) = (void*)((long long)(ptr) + ptr_diff))

///////////////////////////////////////////////
/// INTEGER FUNCTIONS
///////////////////////////////////////////////

int init_Integer(Integer* integer_elem, int value, int line)
{
    static const ElementInterface vtable = { print_Integer, element_size_Integer, update_on_realloc_Integer };
    static Element base = { 0, 0, &vtable };
    memcpy(&integer_elem->base, &base, sizeof(base));

    integer_elem->base.line = line;
    integer_elem->base.tag = SET_ELEMENT_TYPE(Expression, Integer_expr);
    integer_elem->value = value;
    return 1;
}

void print_Integer(const Element* integer_elem, int depth)
{
    const Integer* integer_element = (const Integer*)integer_elem;
    AST_PRINT_PAD(depth);
    printf("Integer[%d]", integer_element->value);
}

// Integers are constant-sized in the AST data array. Just return their size
size_t element_size_Integer(const Element* element)
{
    return sizeof(Integer);
}

// Integers hold no pointers to be updated upon a realloc. Do nothing
void update_on_realloc_Integer(Element* integer_elem, long long ptr_diff) {}

///////////////////////////////////////////////
/// FLOAT FUNCTIONS
///////////////////////////////////////////////

int init_Float(Float* float_elem, double value, int line)
{
    static const ElementInterface vtable = { print_Float, element_size_Float, update_on_realloc_Float };
    static Element base = { 0, 0, &vtable};
    memcpy(&float_elem->base, &base, sizeof(base));

    float_elem->base.line = line;
    float_elem->base.tag = SET_ELEMENT_TYPE(Expression, Float_expr);
    float_elem->value = value;
    return 1;
}

void print_Float(const Element* float_elem, int depth)
{
    const Float* float_element = (const Float*)float_elem;
    AST_PRINT_PAD(depth);
    printf("Float[%f]", float_element->value);
}

// Floats are constant-sized in the AST data array. Just return their size
size_t element_size_Float(const Element* float_elem)
{
    return sizeof(Float);
}

// Floats hold no pointers to be updated upon a realloc. Do nothing
void update_on_realloc_Float(Element* float_elem, long long ptr_diff) {}

///////////////////////////////////////////////
/// BOOL FUNCTIONS
///////////////////////////////////////////////

int init_Bool(Bool* bool_elem, char value, int line)
{
    static const ElementInterface vtable = { print_Bool, element_size_Bool, update_on_realloc_Bool };
    static Element base = { 0, 0, &vtable};
    memcpy(&bool_elem->base, &base, sizeof(base));

    bool_elem->base.line = line;
    bool_elem->base.tag = SET_ELEMENT_TYPE(Expression, Bool_expr);
    bool_elem->value = value;
    return 1;
}

void print_Bool(const Element* bool_elem, int depth)
{
    const Bool* bool_element = (const Bool*)bool_elem;
    AST_PRINT_PAD(depth);
    printf("Bool[%s]", (bool_element->value) ? "true" : "false");
}

// Bools are constant-sized in the AST data array. Just return their size
size_t element_size_Bool(const Element* bool_elem)
{
    return sizeof(Bool);
}

// Bools hold no pointers to be updated upon a realloc. Do nothing
void update_on_realloc_Bool(Element* bool_elem, long long ptr_diff) {}

///////////////////////////////////////////////
/// STRING FUNCTIONS
///////////////////////////////////////////////

int init_String(String* string_elem, char* string, int length, int line)
{
    static const ElementInterface vtable = { print_String, element_size_String, update_on_realloc_String };
    static Element base = { 0, 0, &vtable};
    memcpy(&string_elem->base, &base, sizeof(base));

    string_elem->base.line = line;
    string_elem->base.tag = SET_ELEMENT_TYPE(Expression, String_expr);
    string_elem->string = string;
    string_elem->length = length;
    return 1;
}

void print_String(const Element* string_elem, int depth)
{
    const String* string_element = (const String*)string_elem;
    AST_PRINT_PAD(depth);
    printf("String[%.*s]", string_element->length, string_element->string);
}

// Strings are constant-sized in the AST data array. Just return their size
size_t element_size_String(const Element* string_elem)
{
    return sizeof(String);
}

// Strings hold no pointers to be updated upon a realloc. Do nothing. Do note that, while strings
// DO hold a pointer to their contents, these are not located in the AST array, but rather in the
// source file array, in the expression-evaluation array, or stored as an interpreter variable.
void update_on_realloc_String(Element* string_elem, long long ptr_diff) {}

///////////////////////////////////////////////
/// BINOP FUNCTIONS
///////////////////////////////////////////////

int init_BinOp(BinOp* binop_elem, token_type op, void* left, void* right, int line)
{
    if (!CHECK_ELEMENT_SUPERTYPE(left, Expression) || !CHECK_ELEMENT_SUPERTYPE(right, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_BinOp, element_size_BinOp, update_on_realloc_BinOp };
    static Element base = { 0, 0, &vtable};
    memcpy(&binop_elem->base, &base, sizeof(base));

    binop_elem->base.line = line;
    binop_elem->base.tag = SET_ELEMENT_TYPE(Expression, BinOp_expr);
    binop_elem->op = op;
    binop_elem->left = left;
    binop_elem->right = right;
    return 1;
}

void print_BinOp(const Element* binop_elem, int depth)
{
    const BinOp* bin_op_element = (const BinOp*)binop_elem;
    AST_PRINT_PAD(depth);
    printf("BinOp (%s) {\n", token_symbols[bin_op_element->op]);
    print_element(bin_op_element->left, depth+1);
    printf(",\n");
    print_element(bin_op_element->right, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

// Binops are constant-sized in the AST data array. Just return their size
size_t element_size_BinOp(const Element* binop_elem)
{
    return sizeof(BinOp);
}

// Binops hold pointers to their children, which must be updated on realloc.
void update_on_realloc_BinOp(Element* binop_elem, long long ptr_diff)
{
    BinOp* bin_op_element = (BinOp*) binop_elem;
    UPDATE_PTR(bin_op_element->left);
    UPDATE_PTR(bin_op_element->right);
}

///////////////////////////////////////////////
/// UNOP FUNCTIONS
///////////////////////////////////////////////

int init_UnOp(UnOp* unop_elem, token_type op, void* operand, int line)
{
    if (!CHECK_ELEMENT_SUPERTYPE(operand, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_UnOp, element_size_UnOp, update_on_realloc_UnOp };
    static Element base = { 0, 0, &vtable};
    memcpy(&unop_elem->base, &base, sizeof(base));

    unop_elem->base.line = line;
    unop_elem->base.tag = SET_ELEMENT_TYPE(Expression, UnOp_expr);
    unop_elem->op = op;
    unop_elem->operand = operand;
    return 1;
}
void print_UnOp(const Element* unop_elem, int depth)
{
    const UnOp* un_op_element = (const UnOp*)unop_elem;
    AST_PRINT_PAD(depth);
    printf("UnOp (%s) {\n", token_symbols[un_op_element->op]);
    print_element(un_op_element->operand, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

// Unops are constant-sized in the AST data array. Just return their size
size_t element_size_UnOp(const Element* unop_elem)
{
    return sizeof(UnOp);
}

// Unops hold pointers to their children, which must be updated on realloc.
void update_on_realloc_UnOp(Element* unop_elem, long long ptr_diff)
{
    UnOp* un_op_element = (UnOp*) unop_elem;
    UPDATE_PTR(un_op_element->operand);
}

///////////////////////////////////////////////
/// GROUPING FUNCTIONS
///////////////////////////////////////////////

int init_Grouping(Grouping* grouping_elem, void* expression, int line)
{
    if (!CHECK_ELEMENT_SUPERTYPE(expression, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_Grouping, element_size_Grouping, update_on_realloc_Grouping };
    static Element base = { 0, 0, &vtable};
    memcpy(&grouping_elem->base, &base, sizeof(base));

    grouping_elem->base.line = line;
    grouping_elem->base.tag = SET_ELEMENT_TYPE(Expression, Grouping_expr);
    grouping_elem->expression = expression;
    return 1;
}
void print_Grouping(const Element* grouping_elem, int depth)
{
    const Grouping* grouping_element = (const Grouping*)grouping_elem;
    AST_PRINT_PAD(depth);
    printf("Grouping {\n");
    print_element(grouping_element->expression, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

// Groupings are constant-sized in the AST data array. Just return their size
size_t element_size_Grouping(const Element* grouping_elem)
{
    return sizeof(Grouping);
}

// Groupings hold pointers to their children, which must be updated on realloc.
void update_on_realloc_Grouping(Element* grouping_elem, long long ptr_diff)
{
    Grouping* grouping_element = (Grouping*) grouping_elem;
    UPDATE_PTR(grouping_element->expression);
}

///////////////////////////////////////////////
/// STATEMENTLIST FUNCTIONS
///////////////////////////////////////////////

int init_StatementList(StatementList* statement_list_elem, statement_array* array, void* ast_base, int line)
{
    for (size_t i = 0; i < array->used; i++)
    {
        if (!CHECK_ELEMENT_SUPERTYPE(array->statements[i] + (char*)(ast_base), Statement))
        {
            return 0;
        }
    }

    static const ElementInterface vtable = { print_StatementList, element_size_StatementList, update_on_realloc_StatementList };
    static Element base = { 0, 0, &vtable};
    memcpy(&statement_list_elem->base, &base, sizeof(base));

    statement_list_elem->base.line = line;
    statement_list_elem->base.tag = SET_ELEMENT_TYPE(Statement, StatementList_stmt);
    statement_list_elem->size = array->used;

    // Insert elements into memory after the struct itself
    void** statement_ptrs = (void**)(statement_list_elem + sizeof(StatementList));
    for (size_t i = 0; i < array->used; i++)
    {
        *statement_ptrs++ = array->statements[i] + (char*)(ast_base);
    }

    return 1;
}

void print_StatementList(const Element* statement_list_elem, int depth)
{
    const StatementList* statement_list_element = (const StatementList*)statement_list_elem;
    AST_PRINT_PAD(depth);
    printf("StatementList {\n");
    
    const void** statement_ptrs = (const void**)(statement_list_element + sizeof(StatementList));
    for (size_t i = 0; i < statement_list_element->size; i++)
    {
        print_element(*statement_ptrs++, depth+1);
        printf("\n");
    }
    
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_StatementList(const Element* statement_list_elem)
{
    const StatementList* statement_list_element = (const StatementList*)statement_list_elem;
    return sizeof(StatementList) + statement_list_element->size * sizeof(void*);
}

void update_on_realloc_StatementList(Element* statement_list_elem, long long ptr_diff)
{
    StatementList* statement_list_element = (StatementList*)statement_list_elem;
    void** statement_ptrs = (void**)(statement_list_element + sizeof(StatementList));
    
    for (size_t i = 0; i < statement_list_element->size; i++)
    {
        UPDATE_PTR(*statement_ptrs);
    }
}

///////////////////////////////////////////////
/// WHILE FUNCTIONS
///////////////////////////////////////////////

int init_While(While* while_elem, int line)
{
    return -1;
}
void print_While(const Element* while_elem, int depth)
{
    
}

///////////////////////////////////////////////
/// ASSIGNMENT FUNCTIONS
///////////////////////////////////////////////

int init_Assignment(Assignment* assignment_elem, int line)
{
    return -1;

}
void print_Assignment(const Element* assignment_elem, int depth)
{
    
}

///////////////////////////////////////////////
/// PRINT FUNCTIONS
///////////////////////////////////////////////

int init_Print(Print* print_elem, void* expression, int line)
{
    if (!CHECK_ELEMENT_SUPERTYPE(expression, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_Print, element_size_Print, update_on_realloc_Print };
    static Element base = { 0, 0, &vtable};
    memcpy(&print_elem->base, &base, sizeof(base));

    print_elem->base.line = line;
    print_elem->base.tag = SET_ELEMENT_TYPE(Statement, Print_stmt);
    print_elem->expression = expression;
    return 1;
}

void print_Print(const Element* print_elem, int depth)
{
    const Print* prnt_element = (const Print*)print_elem;
    AST_PRINT_PAD(depth);
    printf("Print {\n");
    print_element(prnt_element->expression, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_Print(const Element* print_elem)
{
    return sizeof(Print);
}

void update_on_realloc_Print(Element* print_elem, long long ptr_diff)
{
    Print* print_element = (Print*) print_elem;
    UPDATE_PTR(print_element->expression);
}

///////////////////////////////////////////////
/// IF FUNCTIONS
///////////////////////////////////////////////

///////////////////////////////////////////////
/// FOR FUNCTIONS
///////////////////////////////////////////////