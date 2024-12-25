#include "model.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define AST_PRINT_PAD(depth) (printf("%*s", (depth)*4, ""))

void ast_print_dispatch(void* element, int depth)
{
    int element_type = GET_ELEMENT_TYPE(element);
    int element_supertype = GET_ELEMENT_SUPERTYPE(element);

    if (element_supertype == Expression)
    {
        switch (element_type)
        {
            case Integer_expr:
                print_Integer((Integer*)element, depth);
                break;

            case Float_expr:
                print_Float((Float*)element, depth);
                break;

            case BinOp_expr:
                print_BinOp((BinOp*)element, depth);
                break;

            case UnOp_expr:
                print_UnOp((UnOp*)element, depth);
                break;

            case Grouping_expr:
                print_Grouping((Grouping*)element, depth);
                break;

            default:
                PRINT_ERROR_AND_QUIT("Printing unknown expression type %d\n", element_type);
        }
    }
    else if (element_supertype == Statement)
    {
        switch (element_type)
        {
            case While_stmt:
                print_While((While*)element, depth);
                break;
            case Assignment_stmt:
                print_Assignment((Assignment*)element, depth);
                break;
            default:
                PRINT_ERROR_AND_QUIT("Printing unknown element supertype %d\n", element_supertype);
        }
    }
    else
    {
        PRINT_ERROR_AND_QUIT("Printing unknown expression type %d\n", element_type);
    }
}

int init_Integer(Integer* integer_elem, int value, int line)
{
    integer_elem->tag = SET_ELEMENT_TYPE(Expression, Integer_expr);
    integer_elem->value = value;
    integer_elem->line = line;
    return 1;
}

void print_Integer(const Integer* integer_elem, int depth)
{
    AST_PRINT_PAD(depth);
    printf("Integer[%d]", integer_elem->value);
}

int init_Float(Float* float_elem, float value, int line)
{
    float_elem->tag = SET_ELEMENT_TYPE(Expression, Float_expr);
    float_elem->value = value;
    float_elem->line = line;
    return 1;
}
void print_Float(const Float* float_elem, int depth)
{
    AST_PRINT_PAD(depth);
    printf("Float[%f]", float_elem->value);
}

int init_BinOp(BinOp* bin_op, token_type op, void* left, void* right, int line)
{
    if (!CHECK_ELEMENT_SUPERTYPE(left, Expression) || !CHECK_ELEMENT_SUPERTYPE(right, Expression))
    {
        return 0;
    }

    bin_op->tag = SET_ELEMENT_TYPE(Expression, BinOp_expr);
    bin_op->op = op;
    bin_op->left = left;
    bin_op->right = right;
    bin_op->line = line;
    return 1;
}

void print_BinOp(const BinOp* bin_op, int depth)
{
    AST_PRINT_PAD(depth);
    printf("BinOp (%s) {\n", token_symbols[bin_op->op]);
    ast_print_dispatch(bin_op->left, depth+1);
    printf(",\n");
    ast_print_dispatch(bin_op->right, depth+1);
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

    un_op->tag = SET_ELEMENT_TYPE(Expression, UnOp_expr);
    un_op->op = op;
    un_op->operand = operand;
    un_op->line = line;
    return 1;
}
void print_UnOp(const UnOp* un_op, int depth)
{
    AST_PRINT_PAD(depth);
    printf("UnOp (%s) {\n", token_symbols[un_op->op]);
    ast_print_dispatch(un_op->operand, depth+1);
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

    grouping_elem->tag = SET_ELEMENT_TYPE(Expression, Grouping_expr);
    grouping_elem->expression = expression;
    grouping_elem->line = line;
    return 1;
}
void print_Grouping(const Grouping* grouping_elem, int depth)
{
    AST_PRINT_PAD(depth);
    printf("Grouping {\n");
    ast_print_dispatch(grouping_elem->expression, depth+1);
    printf("\n");
    AST_PRINT_PAD(depth);
    printf("}");
}

int init_While(While* while_stmt, int line)
{
    return -1;
}
void print_While(const While* while_stmt, int depth)
{
    
}

int init_Assignment(Assignment* assignment_elem, int line)
{
    return -1;

}
void print_Assignment(const Assignment* assignment_elem, int depth)
{
    
}