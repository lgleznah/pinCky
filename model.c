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

void init_Integer(Integer* integer_elem, int value, int line)
{
    static const ElementInterface vtable = { print_Integer, element_size_Integer, compute_ptr_Integer };
    static Element base = { 0, 0, &vtable };
    memcpy(&integer_elem->base, &base, sizeof(base));

    integer_elem->base.line = line;
    integer_elem->base.tag = SET_ELEMENT_TYPE(Expression, Integer_expr);
    integer_elem->value = value;
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

void init_Float(Float* float_elem, double value, int line)
{
    static const ElementInterface vtable = { print_Float, element_size_Float, compute_ptr_Float };
    static Element base = { 0, 0, &vtable};
    memcpy(&float_elem->base, &base, sizeof(base));

    float_elem->base.line = line;
    float_elem->base.tag = SET_ELEMENT_TYPE(Expression, Float_expr);
    float_elem->value = value;
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

void init_Bool(Bool* bool_elem, char value, int line)
{
    static const ElementInterface vtable = { print_Bool, element_size_Bool, compute_ptr_Bool };
    static Element base = { 0, 0, &vtable};
    memcpy(&bool_elem->base, &base, sizeof(base));

    bool_elem->base.line = line;
    bool_elem->base.tag = SET_ELEMENT_TYPE(Expression, Bool_expr);
    bool_elem->value = value;
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

void init_String(String* string_elem, string_type value, int line)
{
    static const ElementInterface vtable = { print_String, element_size_String, compute_ptr_String };
    static Element base = { 0, 0, &vtable};
    memcpy(&string_elem->base, &base, sizeof(base));

    string_elem->base.line = line;
    string_elem->base.tag = SET_ELEMENT_TYPE(Expression, String_expr);
    string_elem->value.length = value.length;
    string_elem->value.string_value = value.string_value;
}

void print_String(const Element* string_elem, int depth)
{
    const String* string_element = (const String*)string_elem;
    AST_PRINT_PAD(depth);
    printf("String[%.*s]", string_element->value.length, string_element->value.string_value);
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

void init_Identifier(Identifier* identifier_elem, char* name, int length, int line)
{
    static const ElementInterface vtable = { print_Identifier, element_size_Identifier, compute_ptr_Identifier };
    static Element base = { 0, 0, &vtable};
    memcpy(&identifier_elem->base, &base, sizeof(base));

    identifier_elem->base.line = line;
    identifier_elem->base.tag = SET_ELEMENT_TYPE(Expression, Identifier_expr);
    identifier_elem->name.string_value = name;
    identifier_elem->name.length = length;
}

void print_Identifier(const Element* identifier_elem, int depth)
{
    const Identifier* identifier_element = (const Identifier*)identifier_elem;
    AST_PRINT_PAD(depth);
    printf("Identifier[%.*s]", identifier_element->name.length, identifier_element->name.string_value);
}

// Identifiers are constant-sized in the AST data array. Just return their size
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

void init_BinOp(BinOp* binop_elem, token_type op, size_t left, size_t right, void* ast_base, int line)
{
    void* left_ptr = OFFSET_PTR(left);
    void* right_ptr = OFFSET_PTR(right);
    if (!CHECK_ELEMENT_SUPERTYPE(left_ptr, Expression) || !CHECK_ELEMENT_SUPERTYPE(right_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "Both sides of binary operation must be expressions.");
    }

    static const ElementInterface vtable = { print_BinOp, element_size_BinOp, compute_ptr_BinOp };
    static Element base = { 0, 0, &vtable};
    memcpy(&binop_elem->base, &base, sizeof(base));

    binop_elem->base.line = line;
    binop_elem->base.tag = SET_ELEMENT_TYPE(Expression, BinOp_expr);
    binop_elem->op = op;
    binop_elem->left = (void*)left;
    binop_elem->right = (void*)right;
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

void init_UnOp(UnOp* unop_elem, token_type op, size_t operand, void* ast_base, int line)
{
    void* operand_ptr = OFFSET_PTR(operand);
    if (!CHECK_ELEMENT_SUPERTYPE(operand_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "Operand of unary operation must be an expression.");
    }

    static const ElementInterface vtable = { print_UnOp, element_size_UnOp, compute_ptr_UnOp };
    static Element base = { 0, 0, &vtable};
    memcpy(&unop_elem->base, &base, sizeof(base));

    unop_elem->base.line = line;
    unop_elem->base.tag = SET_ELEMENT_TYPE(Expression, UnOp_expr);
    unop_elem->op = op;
    unop_elem->operand = (void*)operand;
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

void init_Grouping(Grouping* grouping_elem, size_t expression, void* ast_base, int line)
{
    void* expression_ptr = OFFSET_PTR(expression);
    if (!CHECK_ELEMENT_SUPERTYPE(expression_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "Content of grouping must be an expression.");
    }

    static const ElementInterface vtable = { print_Grouping, element_size_Grouping, compute_ptr_Grouping };
    static Element base = { 0, 0, &vtable};
    memcpy(&grouping_elem->base, &base, sizeof(base));

    grouping_elem->base.line = line;
    grouping_elem->base.tag = SET_ELEMENT_TYPE(Expression, Grouping_expr);
    grouping_elem->expression = (void*)expression;
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

void init_StatementList(StatementList* statement_list_elem, statement_array* array, void* ast_base, int line)
{
    for (size_t i = 0; i < array->used; i++)
    {
        if (!CHECK_ELEMENT_SUPERTYPE(array->data[i] + (char*)(ast_base), Statement))
        {
            PRINT_SYNTAX_ERROR_AND_QUIT(GET_ELEMENT_LINE(array->data[i] + (char*)(ast_base)), "Element in statement list must be an statement.");
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
        *statement_ptrs++ = (void*)array->data[i];
    }
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

void init_While(While* while_elem, size_t condition, size_t statements, void* ast_base, int line)
{
    void* condition_ptr = OFFSET_PTR(condition);
    void* statements_ptr = OFFSET_PTR(statements);

    if (!CHECK_ELEMENT_SUPERTYPE(condition_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "While condition must be an expression.");
    }

    if (!CHECK_ELEMENT_SUPERTYPE(statements_ptr, Statement) || !CHECK_ELEMENT_TYPE(statements_ptr, StatementList_stmt))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(GET_ELEMENT_LINE(statements_ptr), "While statements must be an statement list");
    }

    static const ElementInterface vtable = { print_While, element_size_While, compute_ptr_While };
    static Element base = { 0, 0, &vtable};
    memcpy(&while_elem->base, &base, sizeof(base));

    while_elem->base.line = line;
    while_elem->base.tag = SET_ELEMENT_TYPE(Statement, While_stmt);
    while_elem->condition = (void*)condition;
    while_elem->statements = (void*)statements;
}

void print_While(const Element* while_elem, int depth)
{
    const While* while_element = (const While*)while_elem;
    AST_PRINT_PAD(depth);
    printf("While {\n");
    AST_PRINT_PAD(depth+1);
    printf("Condition {\n");
    print_element(while_element->condition, depth+2);
    printf("\n");
    AST_PRINT_PAD(depth+1);
    printf("},\n");
    AST_PRINT_PAD(depth+1);
    printf("Do {\n");
    print_element(while_element->statements, depth+2);
    printf("\n");
    AST_PRINT_PAD(depth+1);
    printf("}\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_While(const Element* while_elem)
{
    return sizeof(While);
}

void compute_ptr_While(Element* while_elem, void* ast_base)
{
    While* while_element = (While*)while_elem;
    while_element->condition = OFFSET_PTR((size_t)while_element->condition);
    while_element->statements = OFFSET_PTR((size_t)while_element->statements);
    compute_ptr(while_element->condition, ast_base);
    compute_ptr(while_element->statements, ast_base);
}

///////////////////////////////////////////////
/// ASSIGNMENT FUNCTIONS
///////////////////////////////////////////////

void init_Assignment(Assignment* assignment_elem, size_t lhs, size_t rhs, int is_local, void* ast_base, int line)
{
    void* lhs_ptr = OFFSET_PTR(lhs);
    void* rhs_ptr = OFFSET_PTR(rhs);
    if (!CHECK_ELEMENT_SUPERTYPE(lhs_ptr, Expression) || !CHECK_ELEMENT_TYPE(lhs_ptr, Identifier_expr))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "Left-hand side of assignment must be an identifier.");
    }
    if (!CHECK_ELEMENT_SUPERTYPE(rhs_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "Right-hand side of assignment must be an expression.");
    }

    static const ElementInterface vtable = { print_Assignment, element_size_Assignment, compute_ptr_Assignment };
    static Element base = { 0, 0, &vtable };
    memcpy(&assignment_elem->base, &base, sizeof(base));

    assignment_elem->base.line = line;
    assignment_elem->base.tag = SET_ELEMENT_TYPE(Statement, Assignment_stmt);
    assignment_elem->lhs = (void*)lhs;
    assignment_elem->rhs = (void*)rhs;
    assignment_elem->is_local = is_local;
}

void print_Assignment(const Element* assignment_elem, int depth)
{
    const Assignment* assignment_element = (const Assignment*)assignment_elem;
    AST_PRINT_PAD(depth);
    printf((assignment_element->is_local) ? "Local assignment {\n": "Assignment {\n");
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

void init_Print(Print* print_elem, char break_line, size_t expression, void* ast_base, int line)
{
    void* expression_ptr = OFFSET_PTR(expression);
    if (!CHECK_ELEMENT_SUPERTYPE(expression_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "Contents of print statement must be an expression.");
    }

    static const ElementInterface vtable = { print_Print, element_size_Print, compute_ptr_Print };
    static Element base = { 0, 0, &vtable};
    memcpy(&print_elem->base, &base, sizeof(base));

    print_elem->base.line = line;
    print_elem->base.tag = SET_ELEMENT_TYPE(Statement, Print_stmt);
    print_elem->expression = (void*)expression;
    print_elem->break_line = break_line;
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

void init_If(If* if_elem, size_t condition, size_t then_branch, size_t else_branch, void* ast_base, int line)
{
    void* condition_ptr = OFFSET_PTR(condition);
    void* then_branch_ptr = OFFSET_PTR(then_branch);
    void* else_branch_ptr = OFFSET_PTR(else_branch);

    if (!CHECK_ELEMENT_SUPERTYPE(condition_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "If condition must be an expression.");
    }

    if (!CHECK_ELEMENT_SUPERTYPE(then_branch_ptr, Statement) || !CHECK_ELEMENT_TYPE(then_branch_ptr, StatementList_stmt))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(GET_ELEMENT_LINE(then_branch_ptr), "Then branch must be an statement list");
    }

    if (else_branch != (size_t)(-1) && (!CHECK_ELEMENT_SUPERTYPE(else_branch_ptr, Statement) || !CHECK_ELEMENT_TYPE(else_branch_ptr, StatementList_stmt)))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(GET_ELEMENT_LINE(else_branch_ptr), "Else branch must be an statement list");
    }

    static const ElementInterface vtable = { print_If, element_size_If, compute_ptr_If };
    static Element base = { 0, 0, &vtable};
    memcpy(&if_elem->base, &base, sizeof(base));

    if_elem->base.line = line;
    if_elem->base.tag = SET_ELEMENT_TYPE(Statement, If_stmt);
    if_elem->condition = (void*)condition;
    if_elem->then_branch = (void*)then_branch;
    if_elem->else_branch = (void*)else_branch;
}

void print_If(const Element* if_elem, int depth)
{
    const If* if_element = (const If*)if_elem;
    AST_PRINT_PAD(depth);
    printf("If {\n");
    AST_PRINT_PAD(depth+1);
    printf("Condition {\n");
    print_element(if_element->condition, depth+2);
    printf("\n");
    AST_PRINT_PAD(depth+1);
    printf("},\n");
    AST_PRINT_PAD(depth+1);
    printf("Then {\n");
    print_element(if_element->then_branch, depth+2);

    if (if_element->else_branch != NULL)
    {
        printf("\n");
        AST_PRINT_PAD(depth+1);
        printf("},\n");
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

void init_For(For* for_elem, size_t initial_assignment, size_t stop, size_t step, size_t statements, void* ast_base, int line)
{
    void* initial_assignment_ptr = OFFSET_PTR(initial_assignment);
    void* stop_ptr = OFFSET_PTR(stop);
    void* step_ptr = OFFSET_PTR(step);
    void* statements_ptr = OFFSET_PTR(statements);

    if (!CHECK_ELEMENT_SUPERTYPE(initial_assignment_ptr, Statement) || !CHECK_ELEMENT_TYPE(initial_assignment_ptr, Assignment_stmt))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "For initializer must be an assignment.");
    }

    if (!CHECK_ELEMENT_SUPERTYPE(stop_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "For stop value be an expression.");
    }

    if (step != (size_t)(-1) && (!CHECK_ELEMENT_SUPERTYPE(step_ptr, Expression)))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "For step value be an expression.");
    }

    if (!CHECK_ELEMENT_SUPERTYPE(statements_ptr, Statement) || !CHECK_ELEMENT_TYPE(statements_ptr, StatementList_stmt))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(GET_ELEMENT_LINE(statements_ptr), "For statements must be an statement list");
    }

    static const ElementInterface vtable = { print_For, element_size_For, compute_ptr_For };
    static Element base = { 0, 0, &vtable};
    memcpy(&for_elem->base, &base, sizeof(base));

    for_elem->base.line = line;
    for_elem->base.tag = SET_ELEMENT_TYPE(Statement, For_stmt);
    for_elem->initial_assignment = (void*)initial_assignment;
    for_elem->stop = (void*)stop;
    for_elem->step = (void*)step;
    for_elem->statements = (void*)statements;
}

void print_For(const Element* for_elem, int depth)
{
    const For* for_element = (const For*)for_elem;
    AST_PRINT_PAD(depth);
    printf("For {\n");
    AST_PRINT_PAD(depth+1);
    printf("Initial assignment {\n");
    print_element(for_element->initial_assignment, depth+2);
    printf("\n");
    AST_PRINT_PAD(depth+1);
    printf("},\n");
    AST_PRINT_PAD(depth+1);
    printf("Stop {\n");
    print_element(for_element->stop, depth+2);

    if (for_element->step != NULL)
    {
        printf("\n");
        AST_PRINT_PAD(depth+1);
        printf("},\n");
        AST_PRINT_PAD(depth+1);
        printf("Step {\n");
        print_element(for_element->step, depth+2);
    }

    printf("\n");
    AST_PRINT_PAD(depth+1);
    printf("},\n");
    AST_PRINT_PAD(depth+1);
    printf("Do {\n");
    print_element(for_element->statements, depth+2);
    printf("\n");
    AST_PRINT_PAD(depth+1);
    printf("}\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_For(const Element* for_elem)
{
    return sizeof(For);
}

void compute_ptr_For(Element* for_elem, void* ast_base)
{
    For* for_element = (For*)for_elem;
    for_element->initial_assignment = OFFSET_PTR((size_t)for_element->initial_assignment);
    for_element->stop = OFFSET_PTR((size_t)for_element->stop);
    for_element->statements = OFFSET_PTR((size_t)for_element->statements);
    
    compute_ptr(for_element->initial_assignment, ast_base);
    compute_ptr(for_element->stop, ast_base);
    compute_ptr(for_element->statements, ast_base);

    if (for_element->step != (void*)-1)
    {
        for_element->step = OFFSET_PTR((size_t)for_element->step);
        compute_ptr(for_element->step, ast_base);
    }
    else
    {
        for_element->step = NULL;
    }
}

///////////////////////////////////////////////
/// FUNCDECL FUNCTIONS
///////////////////////////////////////////////

void init_FuncDecl(FuncDecl* func_decl_elem, string_type name, string_array* params, size_t statements, void* ast_base, int line)
{
    void* statements_ptr = OFFSET_PTR(statements);

    if (!CHECK_ELEMENT_SUPERTYPE(statements_ptr, Statement) || !CHECK_ELEMENT_TYPE(statements_ptr, StatementList_stmt))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "Function body must be a list of statements");
    }

    static const ElementInterface vtable = { print_FuncDecl, element_size_FuncDecl, compute_ptr_FuncDecl };
    static Element base = { 0, 0, &vtable};
    memcpy(&func_decl_elem->base, &base, sizeof(base));

    func_decl_elem->base.line = line;
    func_decl_elem->base.tag = SET_ELEMENT_TYPE(Statement, FuncDecl_stmt);
    func_decl_elem->name = name;
    func_decl_elem->num_params = params->used;
    func_decl_elem->statements = (void*)statements;

    // Insert function parameters into memory after the struct itself
    string_type* param_ptrs = (string_type*)((char*)func_decl_elem + sizeof(FuncDecl));
    for (size_t i = 0; i < params->used; i++)
    {
        param_ptrs->length = params->data[i].length;
        param_ptrs->string_value = params->data[i].string_value;
        param_ptrs++;
    }
}

void print_FuncDecl(const Element* func_decl_elem, int depth)
{
    const FuncDecl* func_decl_element = (const FuncDecl*) func_decl_elem;
    AST_PRINT_PAD(depth);
    printf("Function declaration [%.*s] (", func_decl_element->name.length, func_decl_element->name.string_value);

    const string_type* params_ptrs = (const string_type*)((const char*)func_decl_element + sizeof(FuncDecl));
    for (size_t i = 0; i < func_decl_element->num_params; i++)
    {
        printf("%.*s", params_ptrs->length, params_ptrs->string_value);
        if (i != func_decl_element->num_params - 1)
        {
            printf(", ");
        }
        params_ptrs++;
    }

    printf(") {\n");
    print_element(func_decl_element->statements, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_FuncDecl(const Element* func_decl_elem)
{
    const FuncDecl* func_decl_element = (const FuncDecl*) func_decl_elem;
    return sizeof(FuncDecl) + func_decl_element->num_params * sizeof(string_type);
}

void compute_ptr_FuncDecl(Element* func_decl_elem, void* ast_base)
{
     FuncDecl* func_decl_element = (FuncDecl*) func_decl_elem;
     func_decl_element->statements = OFFSET_PTR((size_t)func_decl_element->statements);
     compute_ptr(func_decl_element->statements, ast_base);   
}

///////////////////////////////////////////////
/// FUNCCALL FUNCTIONS
///////////////////////////////////////////////

void init_FuncCall(FuncCall* func_call_elem, string_type name, expression_array* args, void* ast_base, int line)
{
    for (size_t i = 0; i < args->used; i++)
    {
        if (!CHECK_ELEMENT_SUPERTYPE(args->data[i] + (char*)(ast_base), Expression))
        {
            PRINT_SYNTAX_ERROR_AND_QUIT(line, "Element in expression list must be an expression.");
        }
    }
    
    static const ElementInterface vtable = { print_FuncCall, element_size_FuncCall, compute_ptr_FuncCall };
    static Element base = { 0, 0, &vtable};
    memcpy(&func_call_elem->base, &base, sizeof(base));

    func_call_elem->base.line = line;
    func_call_elem->base.tag = SET_ELEMENT_TYPE(Expression, FuncCall_expr);
    func_call_elem->name = name;
    func_call_elem->num_args = args->used;

    // Insert function arguments into memory after the struct itself
    void** args_ptrs = (void**)((char*)func_call_elem + sizeof(FuncCall));
    for (size_t i = 0; i < args->used; i++)
    {
        *args_ptrs++ = (void*)args->data[i];
    }
}

void print_FuncCall(const Element* func_call_elem, int depth)
{
    const FuncCall* func_call_element = (const FuncCall*) func_call_elem;
    AST_PRINT_PAD(depth);
    printf("Function call [%.*s] {\n", func_call_element->name.length, func_call_element->name.string_value);

    const void** args_ptrs = (const void**)((const char*)func_call_element + sizeof(FuncCall));
    for (size_t i = 0; i < func_call_element->num_args; i++)
    {
        print_element(*args_ptrs++, depth+1);
        if (i != func_call_element->num_args - 1)
        {
            printf(",\n");
        }
    }
    
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
    
}

size_t element_size_FuncCall(const Element* func_call_elem)
{
    const FuncCall* func_call_element = (const FuncCall*) func_call_elem;
    return sizeof(FuncCall) + func_call_element->num_args * sizeof(void*);
}

void compute_ptr_FuncCall(Element* func_call_elem, void* ast_base)
{
    FuncCall* func_call_element = (FuncCall*)func_call_elem;
    void** args_ptrs = (void**)((char*)func_call_element + sizeof(FuncCall));
    
    for (size_t i = 0; i < func_call_element->num_args; i++)
    {
        *args_ptrs = OFFSET_PTR((size_t)*args_ptrs);
        compute_ptr(*args_ptrs++, ast_base);
    }
}

void init_Return(Return* return_elem, size_t expression, void* ast_base, int line)
{
    
    void* expression_ptr = OFFSET_PTR(expression);
    if (!CHECK_ELEMENT_SUPERTYPE(expression_ptr, Expression))
    {
        PRINT_SYNTAX_ERROR_AND_QUIT(line, "Contents of return statement must be an expression.");
    }

    static const ElementInterface vtable = { print_Return, element_size_Return, compute_ptr_Return };
    static Element base = { 0, 0, &vtable};
    memcpy(&return_elem->base, &base, sizeof(base));

    return_elem->base.line = line;
    return_elem->base.tag = SET_ELEMENT_TYPE(Statement, Return_stmt);
    return_elem->expression = (void*)expression;
}

void print_Return(const Element* return_elem, int depth)
{
    
    const Return* ret_element = (const Return*)return_elem;
    AST_PRINT_PAD(depth);
    printf("Return {\n");
    print_element(ret_element->expression, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

size_t element_size_Return(const Element* return_elem)
{
    return sizeof(Return);
}

void compute_ptr_Return(Element* return_elem, void* ast_base)
{
    Return* return_element = (Return*) return_elem;
    return_element->expression = OFFSET_PTR((size_t)return_element->expression);
    compute_ptr(return_element->expression, ast_base);
}
