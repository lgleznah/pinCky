#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define AST_PRINT_PAD(depth) (printf("%*s", (depth)*4, ""))

int init_Integer(Integer* integer_elem, int value, int line)
{
    static const ElementInterface vtable = { print_Integer };
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

int init_Float(Float* float_elem, double value, int line)
{
    static const ElementInterface vtable = { print_Float };
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

int init_Bool(Bool* bool_elem, char value, int line)
{
    static const ElementInterface vtable = { print_Bool };
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

int init_String(String* string_elem, char* string, int length, int line)
{
    static const ElementInterface vtable = { print_String };
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

int init_BinOp(BinOp* bin_op, token_type op, void* left, void* right, int line)
{
    if (!CHECK_ELEMENT_SUPERTYPE(left, Expression) || !CHECK_ELEMENT_SUPERTYPE(right, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_BinOp };
    static Element base = { 0, 0, &vtable};
    memcpy(&bin_op->base, &base, sizeof(base));

    bin_op->base.line = line;
    bin_op->base.tag = SET_ELEMENT_TYPE(Expression, BinOp_expr);
    bin_op->op = op;
    bin_op->left = left;
    bin_op->right = right;
    return 1;
}

void print_BinOp(const Element* bin_op, int depth)
{
    const BinOp* bin_op_element = (const BinOp*)bin_op;
    AST_PRINT_PAD(depth);
    printf("BinOp (%s) {\n", token_symbols[bin_op_element->op]);
    print_element(bin_op_element->left, depth+1);
    printf(",\n");
    print_element(bin_op_element->right, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

int init_UnOp(UnOp* un_op, token_type op, void* operand, int line)
{
    if (!CHECK_ELEMENT_SUPERTYPE(operand, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_UnOp };
    static Element base = { 0, 0, &vtable};
    memcpy(&un_op->base, &base, sizeof(base));

    un_op->base.line = line;
    un_op->base.tag = SET_ELEMENT_TYPE(Expression, UnOp_expr);
    un_op->op = op;
    un_op->operand = operand;
    return 1;
}
void print_UnOp(const Element* un_op, int depth)
{
    const UnOp* un_op_element = (const UnOp*)un_op;
    AST_PRINT_PAD(depth);
    printf("UnOp (%s) {\n", token_symbols[un_op_element->op]);
    print_element(un_op_element->operand, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

int init_Grouping(Grouping* grouping_elem, void* expression, int line)
{
    if (!CHECK_ELEMENT_SUPERTYPE(expression, Expression))
    {
        return 0;
    }

    static const ElementInterface vtable = { print_Grouping };
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

int init_While(While* while_stmt, int line)
{
    return -1;
}
void print_While(const Element* while_stmt, int depth)
{
    
}

int init_Assignment(Assignment* assignment_elem, int line)
{
    return -1;

}
void print_Assignment(const Element* assignment_elem, int depth)
{
    
}