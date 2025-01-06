#include "interpreter.h"

#include <string.h>
#include <math.h>

#include "model.h"
#include "utils.h"

void init_interpreter(interpreter* interpreter)
{
    init_interpreter_memory(&interpreter->memory, 65535);
}

void free_interpreter(interpreter* interpreter)
{
    free_interpreter_memory(&interpreter->memory);
}

string cast_to_string(interpreter* interpreter, expression_result expression)
{
    char num_value[256];
    switch (expression.type)
    {
        case STRING_VALUE:
            return expression.value.string_value;

        case INT_VALUE:
            int int_length = snprintf(num_value, 256, "%d", expression.value.int_value);
            char* converted_int_string = allocate_interpreter_memory(&interpreter->memory, int_length);
            memcpy_s(converted_int_string, int_length, num_value, int_length);
            return (string) {.length = int_length, .string_value = converted_int_string};

        case FLOAT_VALUE:
            int float_length = snprintf(num_value, 256, "%f", expression.value.float_value);
            char* converted_float_string = allocate_interpreter_memory(&interpreter->memory, float_length);
            memcpy_s(converted_float_string, float_length, num_value, float_length);
            return (string) {.length = float_length, .string_value = converted_float_string};

        case BOOL_VALUE:
            return (expression.value.bool_value) ? true_string : false_string;

        default:
            PRINT_INTERPRETER_ERROR_AND_QUIT("Cannot convert expression of type '%s' to string. This should not appear...", type_names[expression.type]);
    }
}

string string_addition(interpreter* interpreter, string string1, string string2)
{
    char* destination_string = allocate_interpreter_memory(&interpreter->memory, string1.length+string2.length);
    
    for(int i = 0; i < string1.length; i++)
    {
        destination_string[i] = string1.string_value[i];
    }
    for(int i = 0; i < string2.length; i++)
    {
        destination_string[i+string1.length] = string2.string_value[i];
    }
    
    return (string) {.length = string1.length+string2.length, .string_value = destination_string};
}

expression_result interpret(interpreter* interpreter, void* ast_node)
{
    int element_type = GET_ELEMENT_TYPE(ast_node);
    switch (element_type)
    {
        case Integer_expr:
            return (expression_result) {.type = INT_VALUE, .value.int_value = ((Integer*)ast_node)->value};
    
        case Float_expr:
            return (expression_result) {.type = FLOAT_VALUE, .value.float_value = ((Float*)ast_node)->value};

        case Bool_expr:
            return (expression_result) {.type = BOOL_VALUE, .value.bool_value = ((Bool*)ast_node)->value};

        case String_expr:
            return (expression_result) {.type = STRING_VALUE, .value.string_value.string_value = ((String*)ast_node)->string, .value.string_value.length = ((String*)ast_node)->length};
    
        case BinOp_expr:
            expression_result lhs = interpret(interpreter, ((BinOp*)ast_node)->left);
            expression_result rhs = interpret(interpreter, ((BinOp*)ast_node)->right);
            token_type binop_op = ((BinOp*)ast_node)->op;
            switch (binop_op)
            {
                case TOK_PLUS:
                    // Case 1 for addition: both are integers. Return an integer
                    if (lhs.type == INT_VALUE && rhs.type == INT_VALUE)
                    {
                        return (expression_result) {.type = INT_VALUE, .value.int_value = lhs.value.int_value + rhs.value.int_value};
                    }

                    // Case 2 for addition: one value is a float. Return a float
                    if (lhs.type == FLOAT_VALUE && (rhs.type == FLOAT_VALUE || rhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = lhs.value.float_value + ((rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value))};
                    }

                    if (rhs.type == FLOAT_VALUE && (lhs.type == FLOAT_VALUE || lhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = ((lhs.type == FLOAT_VALUE) ? lhs.value.float_value : (double) (lhs.value.int_value)) + rhs.value.float_value };
                    }

                    // Case 3 for addition: one value is a string. Cast the other to string
                    if (lhs.type == STRING_VALUE || rhs.type == STRING_VALUE)
                    {
                        return (expression_result) {.type = STRING_VALUE, .value.string_value = string_addition(interpreter, cast_to_string(interpreter, lhs), cast_to_string(interpreter, rhs))};
                    }

                    // Otherwise, this is an unsupported operation. Error-out
                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                case TOK_MINUS:
                    // Case 1 for subtraction: both are integers. Return an integer
                    if (lhs.type == INT_VALUE && rhs.type == INT_VALUE)
                    {
                        return (expression_result) {.type = INT_VALUE, .value.int_value = lhs.value.int_value - rhs.value.int_value};
                    }

                    // Case 2 for subtraction: one value is a float. Return a float
                    if (lhs.type == FLOAT_VALUE && (rhs.type == FLOAT_VALUE || rhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = lhs.value.float_value - ((rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value))};
                    }

                    if (rhs.type == FLOAT_VALUE && (lhs.type == FLOAT_VALUE || lhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = ((lhs.type == FLOAT_VALUE) ? lhs.value.float_value : (double) (lhs.value.int_value)) - rhs.value.float_value};
                    }

                    // Otherwise, this is an unsupported operation. Error-out
                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                case TOK_STAR:
                    // Case 1 for multiplication: both are integers. Return an integer
                    if (lhs.type == INT_VALUE && rhs.type == INT_VALUE)
                    {
                        return (expression_result) {.type = INT_VALUE, .value.int_value = lhs.value.int_value * rhs.value.int_value};
                    }

                    // Case 2 for multiplication: one value is a float. Return a float
                    if (lhs.type == FLOAT_VALUE && (rhs.type == FLOAT_VALUE || rhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = lhs.value.float_value * ((rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value))};
                    }

                    if (rhs.type == FLOAT_VALUE && (lhs.type == FLOAT_VALUE || lhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = ((lhs.type == FLOAT_VALUE) ? lhs.value.float_value : (double) (lhs.value.int_value)) * rhs.value.float_value};
                    }

                    // Otherwise, this is an unsupported operation. Error-out
                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                case TOK_SLASH:
                    // Case 1 for division: both are integers. Return an integer
                    if (lhs.type == INT_VALUE && rhs.type == INT_VALUE)
                    {
                        return (expression_result) {.type = INT_VALUE, .value.int_value = lhs.value.int_value / rhs.value.int_value};
                    }

                    // Case 2 for division: one value is a float. Return a float
                    if (lhs.type == FLOAT_VALUE && (rhs.type == FLOAT_VALUE || rhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = lhs.value.float_value / ((rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value))};
                    }

                    if (rhs.type == FLOAT_VALUE && (lhs.type == FLOAT_VALUE || lhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = ((lhs.type == FLOAT_VALUE) ? lhs.value.float_value : (double) (lhs.value.int_value)) / rhs.value.float_value};
                    }

                    // Otherwise, this is an unsupported operation. Error-out
                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                case TOK_MOD:
                    // Case 1 for modulus: both are integers. Return an integer
                    if (lhs.type == INT_VALUE && rhs.type == INT_VALUE)
                    {
                        return (expression_result) {.type = INT_VALUE, .value.int_value = lhs.value.int_value % rhs.value.int_value};
                    }

                    // Case 2 for modulus: one value is a float. Return a float
                    if (lhs.type == FLOAT_VALUE && (rhs.type == FLOAT_VALUE || rhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = fmod(lhs.value.float_value, (rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value))};
                    }

                    if (rhs.type == FLOAT_VALUE && (lhs.type == FLOAT_VALUE || lhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = fmod((lhs.type == FLOAT_VALUE) ? lhs.value.float_value : (double) (lhs.value.int_value), rhs.value.float_value)};
                    }

                // Otherwise, this is an unsupported operation. Error-out
                PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                
                case TOK_CARET:
                    // Case 1 for exponentiation: both are integers. Return an integer
                    if (lhs.type == INT_VALUE && rhs.type == INT_VALUE)
                    {
                        return (expression_result) {.type = INT_VALUE, .value.int_value = int_pow(lhs.value.int_value , rhs.value.int_value)};
                    }

                    // Case 2 for exponentiation: one value is a float. Return a float
                    if (lhs.type == FLOAT_VALUE && (rhs.type == FLOAT_VALUE || rhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = pow(lhs.value.float_value, (rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value))};
                    }

                    if (rhs.type == FLOAT_VALUE && (lhs.type == FLOAT_VALUE || lhs.type == INT_VALUE))
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = pow((lhs.type == FLOAT_VALUE) ? lhs.value.float_value : (double) (lhs.value.int_value), rhs.value.float_value)};
                    }

                    // Otherwise, this is an unsupported operation. Error-out
                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                default:
                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unknown binary operation '%s'\n", token_symbols[binop_op]);
            }
        

        case UnOp_expr:
            expression_result operand = interpret(interpreter, ((UnOp*)ast_node)->operand);
            token_type un_op = ((UnOp*)ast_node)->op;
            switch (un_op)
            {
                case TOK_PLUS:
                    // Unary plus is only supported for ints and floats.
                    if (operand.type == INT_VALUE || operand.type == FLOAT_VALUE)
                    {
                        return operand;
                    }

                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported unary operation '%s' for '%s'\n", token_symbols[un_op], type_names[operand.type]);

                case TOK_MINUS:
                    // Unary minus is only supported for ints and floats.
                    if (operand.type == INT_VALUE)
                    {
                        return (expression_result) {.type = INT_VALUE, .value.int_value = -operand.value.int_value};
                    }
                    if (operand.type == FLOAT_VALUE)
                    {
                        return (expression_result) {.type = FLOAT_VALUE, .value.float_value = -operand.value.float_value};
                    }

                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported unary operation '%s' for '%s'\n", token_symbols[un_op], type_names[operand.type]);
                
                case TOK_NOT:
                    // Unary not is only supported for bools
                    if (operand.type == BOOL_VALUE)
                    {
                        return (expression_result) {.type = BOOL_VALUE, .value.bool_value = !operand.value.bool_value};
                    }

                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported unary operation '%s' for '%s'\n", token_symbols[un_op], type_names[operand.type]);

                default:
                    PRINT_INTERPRETER_ERROR_AND_QUIT("Unknown unary operation '%s'\n", token_symbols[un_op]);
            }
        

        case Grouping_expr:
            return interpret(interpreter, ((Grouping*)ast_node)->expression);

        default:
            PRINT_INTERPRETER_ERROR_AND_QUIT("Unknown expression type ID %d\n", element_type);
    }
}
