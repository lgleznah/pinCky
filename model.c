#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define AST_PRINT_PAD(depth) (printf("%*s", (depth)*4, ""))
#define UPDATE_PTR(ptr) ((ptr) = (void*)((long long)(ptr) + ptr_diff))
#define OFFSET_PTR(ptr) ((void*)((ptr) + (char*)(ast_base)))

///////////////////////////////////////////////
/// INTEGER FUNCTIONS
///////////////////////////////////////////////

int init_Integer(Integer* integer_elem, int value, int line)
{
    static const ElementInterface vtable = { print_Integer, element_size_Integer, compute_ptr_Integer };
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
void compute_ptr_Integer(Element* integer_elem, void* ast_base) {}

///////////////////////////////////////////////
/// FLOAT FUNCTIONS
///////////////////////////////////////////////

int init_Float(Float* float_elem, double value, int line)
{
    static const ElementInterface vtable = { print_Float, element_size_Float, compute_ptr_Float };
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
void compute_ptr_Float(Element* float_elem, void* ast_base) {}

///////////////////////////////////////////////
/// BOOL FUNCTIONS
///////////////////////////////////////////////

int init_Bool(Bool* bool_elem, char value, int line)
{
    static const ElementInterface vtable = { print_Bool, element_size_Bool, compute_ptr_Bool };
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
void compute_ptr_Bool(Element* bool_elem, void* ast_base) {}

///////////////////////////////////////////////
/// STRING FUNCTIONS
///////////////////////////////////////////////

int init_String(String* string_elem, char* string, int length, int line)
{
    static const ElementInterface vtable = { print_String, element_size_String, compute_ptr_String };
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
void compute_ptr_String(Element* string_elem, void* ast_base) {}

///////////////////////////////////////////////
/// IDENTIFIER FUNCTIONS
///////////////////////////////////////////////

int init_Identifier(Identifier* identifier_elem, char* name, int length, int line)
{
    static const ElementInterface vtable = { print_Identifier, element_size_Identifier, compute_ptr_Identifier };
    static Element base = { 0, 0, &vtable};
    memcpy(&identifier_elem->base, &base, sizeof(base));

    identifier_elem->base.line = line;
    identifier_elem->base.tag = SET_ELEMENT_TYPE(Expression, String_expr);
    identifier_elem->name = name;
    identifier_elem->length = length;
    return 1;
}

void print_Identifier(const Element* identifier_elem, int depth)
{
    const Identifier* identifier_element = (const Identifier*)identifier_elem;
    AST_PRINT_PAD(depth);
    printf("Identifier[%.*s]", identifier_element->length, identifier_element->name);
}

// Strings are constant-sized in the AST data array. Just return their size
size_t element_size_Identifier(const Element* identifier_elem)
{
    return sizeof(Identifier);
}

// Identifiers hold no pointers to be updated upon a realloc. Do nothing. Do note that, while identifiers
// DO hold a pointer to their contents, these are not located in the AST array, but rather in the
// source file array.
void compute_ptr_Identifier(Element* identifier_elem, void* ast_base) {}

///////////////////////////////////////////////
/// BINOP FUNCTIONS
///////////////////////////////////////////////

int init_BinOp(BinOp* binop_elem, token_type op, size_t left, size_t right, void* ast_base, int line)
{
    void* left_ptr = OFFSET_PTR(left);
    void* right_ptr = OFFSET_PTR(right);
    if (!CHECK_ELEMENT_SUPERTYPE(left_ptr, Expression) || !CHECK_ELEMENT_SUPERTYPE(right_ptr, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_BinOp, element_size_BinOp, compute_ptr_BinOp };
    static Element base = { 0, 0, &vtable};
    memcpy(&binop_elem->base, &base, sizeof(base));

    binop_elem->base.line = line;
    binop_elem->base.tag = SET_ELEMENT_TYPE(Expression, BinOp_expr);
    binop_elem->op = op;
    binop_elem->left = (void*)left;
    binop_elem->right = (void*)right;
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
void compute_ptr_BinOp(Element* binop_elem, void* ast_base)
{
    BinOp* bin_op_element = (BinOp*) binop_elem;
    bin_op_element->left = OFFSET_PTR((size_t)bin_op_element->left);
    bin_op_element->right = OFFSET_PTR((size_t)bin_op_element->right);
    compute_ptr(bin_op_element->left, ast_base);
    compute_ptr(bin_op_element->right, ast_base);
}

///////////////////////////////////////////////
/// UNOP FUNCTIONS
///////////////////////////////////////////////

int init_UnOp(UnOp* unop_elem, token_type op, size_t operand, void* ast_base, int line)
{
    void* operand_ptr = OFFSET_PTR(operand);
    if (!CHECK_ELEMENT_SUPERTYPE(operand_ptr, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_UnOp, element_size_UnOp, compute_ptr_UnOp };
    static Element base = { 0, 0, &vtable};
    memcpy(&unop_elem->base, &base, sizeof(base));

    unop_elem->base.line = line;
    unop_elem->base.tag = SET_ELEMENT_TYPE(Expression, UnOp_expr);
    unop_elem->op = op;
    unop_elem->operand = (void*)operand;
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
void compute_ptr_UnOp(Element* unop_elem, void* ast_base)
{
    UnOp* un_op_element = (UnOp*) unop_elem;
    un_op_element->operand = OFFSET_PTR((size_t)un_op_element->operand);
    compute_ptr(un_op_element->operand, ast_base);
}

///////////////////////////////////////////////
/// GROUPING FUNCTIONS
///////////////////////////////////////////////

int init_Grouping(Grouping* grouping_elem, size_t expression, void* ast_base, int line)
{
    void* expression_ptr = OFFSET_PTR(expression);
    if (!CHECK_ELEMENT_SUPERTYPE(expression_ptr, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_Grouping, element_size_Grouping, compute_ptr_Grouping };
    static Element base = { 0, 0, &vtable};
    memcpy(&grouping_elem->base, &base, sizeof(base));

    grouping_elem->base.line = line;
    grouping_elem->base.tag = SET_ELEMENT_TYPE(Expression, Grouping_expr);
    grouping_elem->expression = (void*)expression;
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
void compute_ptr_Grouping(Element* grouping_elem, void* ast_base)
{
    Grouping* grouping_element = (Grouping*) grouping_elem;
    grouping_element->expression = OFFSET_PTR((size_t)grouping_element->expression);
    compute_ptr(grouping_element->expression, ast_base);
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

    static const ElementInterface vtable = { print_StatementList, element_size_StatementList, compute_ptr_StatementList };
    static Element base = { 0, 0, &vtable};
    memcpy(&statement_list_elem->base, &base, sizeof(base));

    statement_list_elem->base.line = line;
    statement_list_elem->base.tag = SET_ELEMENT_TYPE(Statement, StatementList_stmt);
    statement_list_elem->size = array->used;

    // Insert elements into memory after the struct itself
    void** statement_ptrs = (void**)((char*)statement_list_elem + sizeof(StatementList));
    for (size_t i = 0; i < array->used; i++)
    {
        *statement_ptrs++ = (void*)array->statements[i];
    }

    return 1;
}

void print_StatementList(const Element* statement_list_elem, int depth)
{
    const StatementList* statement_list_element = (const StatementList*)statement_list_elem;
    AST_PRINT_PAD(depth);
    printf("StatementList {\n");
    
    const void** statement_ptrs = (const void**)((const char*)statement_list_element + sizeof(StatementList));
    for (size_t i = 0; i < statement_list_element->size; i++)
    {
        print_element(*statement_ptrs++, depth+1);
        if (i != statement_list_element->size - 1)
        {
            printf(",\n");
        }
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

void compute_ptr_StatementList(Element* statement_list_elem, void* ast_base)
{
    StatementList* statement_list_element = (StatementList*)statement_list_elem;
    void** statement_ptrs = (void**)((char*)statement_list_element + sizeof(StatementList));
    
    for (size_t i = 0; i < statement_list_element->size; i++)
    {
        *statement_ptrs = OFFSET_PTR((size_t)*statement_ptrs);
        compute_ptr(*statement_ptrs++, ast_base);
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

int init_Assignment(Assignment* assignment_elem, size_t lhs, size_t rhs, void* ast_base, int line)
{
    void* lhs_ptr = OFFSET_PTR(lhs);
    void* rhs_ptr = OFFSET_PTR(rhs);
    if (!CHECK_ELEMENT_SUPERTYPE(lhs_ptr, Expression))
    {
        return 0;
    }
    if (!CHECK_ELEMENT_SUPERTYPE(rhs_ptr, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_Assignment, element_size_Assignment, compute_ptr_Assignment };
    static Element base = { 0, 0, &vtable };
    memcpy(&assignment_elem->base, &base, sizeof(base));

    assignment_elem->base.line = line;
    assignment_elem->base.tag = SET_ELEMENT_TYPE(Statement, Assignment_stmt);
    assignment_elem->lhs = (void*)lhs;
    assignment_elem->rhs = (void*)rhs;

    return 1;
}

void print_Assignment(const Element* assignment_elem, int depth)
{
    const Assignment* assignment_element = (const Assignment*)assignment_elem;
    AST_PRINT_PAD(depth);
    printf("Assignment {\n");
    print_element(assignment_element->lhs, depth+1);
    printf(",\n");
    print_element(assignment_element->rhs, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_Assignment(const Element* assignment_elem)
{
    return sizeof(Assignment);
}

void compute_ptr_Assignment(Element* assignment_elem, void* ast_base)
{
    Assignment* assignment_element = (Assignment*) assignment_elem;
    assignment_element->lhs = OFFSET_PTR((size_t)assignment_element->lhs);
    assignment_element->rhs = OFFSET_PTR((size_t)assignment_element->rhs);
    compute_ptr(assignment_element->lhs, ast_base);
    compute_ptr(assignment_element->rhs, ast_base);
}

///////////////////////////////////////////////
/// PRINT FUNCTIONS
///////////////////////////////////////////////

int init_Print(Print* print_elem, char break_line, size_t expression, void* ast_base, int line)
{
    void* expression_ptr = OFFSET_PTR(expression);
    if (!CHECK_ELEMENT_SUPERTYPE(expression_ptr, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_Print, element_size_Print, compute_ptr_Print };
    static Element base = { 0, 0, &vtable};
    memcpy(&print_elem->base, &base, sizeof(base));

    print_elem->base.line = line;
    print_elem->base.tag = SET_ELEMENT_TYPE(Statement, Print_stmt);
    print_elem->expression = (void*)expression;
    print_elem->break_line = break_line;
    
    return 1;
}

void print_Print(const Element* print_elem, int depth)
{
    const Print* prnt_element = (const Print*)print_elem;
    AST_PRINT_PAD(depth);
    printf((prnt_element->break_line) ? "Print {\n" : "Println {\n");
    print_element(prnt_element->expression, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_Print(const Element* print_elem)
{
    return sizeof(Print);
}

void compute_ptr_Print(Element* print_elem, void* ast_base)
{
    Print* print_element = (Print*) print_elem;
    print_element->expression = OFFSET_PTR((size_t)print_element->expression);
    compute_ptr(print_element->expression, ast_base);
}

///////////////////////////////////////////////
/// IF FUNCTIONS
///////////////////////////////////////////////

int init_If(If* if_elem, size_t condition, size_t then_branch, size_t else_branch, void* ast_base, int line)
{
    void* condition_ptr = OFFSET_PTR(condition);
    void* then_branch_ptr = OFFSET_PTR(then_branch);
    void* else_branch_ptr = OFFSET_PTR(else_branch);

    if (!CHECK_ELEMENT_SUPERTYPE(condition_ptr, Expression))
    {
        return 0;
    }

    if (!CHECK_ELEMENT_SUPERTYPE(then_branch_ptr, Statement) || !CHECK_ELEMENT_TYPE(then_branch_ptr, StatementList_stmt))
    {
        return 0;
    }

    if (else_branch != (size_t)(-1) && (!CHECK_ELEMENT_SUPERTYPE(else_branch_ptr, Statement) || !CHECK_ELEMENT_TYPE(else_branch_ptr, StatementList_stmt)))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_If, element_size_If, compute_ptr_If };
    static Element base = { 0, 0, &vtable};
    memcpy(&if_elem->base, &base, sizeof(base));

    if_elem->base.line = line;
    if_elem->base.tag = SET_ELEMENT_TYPE(Statement, If_stmt);
    if_elem->condition = (void*)condition;
    if_elem->then_branch = (void*)then_branch;
    if_elem->else_branch = (void*)else_branch;
    return 1;
}

void print_If(const Element* if_elem, int depth)
{
    const If* if_element = (const If*)if_elem;
    AST_PRINT_PAD(depth);
    printf("If {\n");
    AST_PRINT_PAD(depth+1);
    printf("Condition {\n");
    print_element(if_element->condition, depth+2);
    printf(",\n");
    AST_PRINT_PAD(depth+1);
    printf("Then {\n");
    print_element(if_element->then_branch, depth+2);

    if (if_element->else_branch != NULL)
    {
        printf(",\n");
        AST_PRINT_PAD(depth+1);
        printf("Else {\n");
        print_element(if_element->else_branch, depth+2);
    }
    printf("\n");
    AST_PRINT_PAD(depth+1);
    printf("}\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_If(const Element* if_elem)
{
    return sizeof(If);
}

void compute_ptr_If(Element* if_elem, void* ast_base)
{
    If* if_element = (If*)if_elem;
    if_element->condition = OFFSET_PTR((size_t)if_element->condition);
    if_element->then_branch = OFFSET_PTR((size_t)if_element->then_branch);
    compute_ptr(if_element->condition, ast_base);
    compute_ptr(if_element->then_branch, ast_base);

    if (if_element->else_branch != (void*)-1)
    {
        if_element->else_branch = OFFSET_PTR((size_t)if_element->else_branch);
        compute_ptr(if_element->else_branch, ast_base);
    }
    else
    {
        if_element->else_branch = NULL;
    }
}

///////////////////////////////////////////////
/// FOR FUNCTIONS
///////////////////////////////////////////////