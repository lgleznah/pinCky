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

int cast_to_bool(interpreter* interpreter, expression_result expression)
{
    switch (expression.type)
    {
        case STRING_VALUE:
            return (expression.value.string_value.length) ? 1 : 0;

        case INT_VALUE:
            return (expression.value.int_value) ? 1 : 0;

        case FLOAT_VALUE:
            return (expression.value.float_value >= 0) ? 1 : 0;

        case BOOL_VALUE:
            return (expression.value.bool_value);

        default:
            PRINT_INTERPRETER_ERROR_AND_QUIT("Cannot convert expression of type '%s' to bool. This should not appear...", type_names[expression.type]);
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



int string_comparison(string str1, string str2, compare_mode mode)
{
    int comparison_results[3][6] = {{0,1,0,1,0,1},{0,0,1,1,1,0},{1,0,1,0,0,1}};
    
    // strcmp-like comparison
    int result = -1;
    for (int i = 0; i < str1.length && i < str2.length; i++)
    {
        if (str1.string_value[i] > str2.string_value[i])
        {
            result = 2;
            break;
        }

        if (str1.string_value[i] < str2.string_value[i])
        {
            result = 0;
            break;
        }
    }

    // Both strings are equal up to some point. Check lengths to compute result
    if (result == -1)
    {
        if (str1.length < str2.length) result = 0;
        else if (str1.length > str2.length) result = 2;
        else result = 1;
    }

    // Use comparison mode and comparison result to return final result
    return comparison_results[result][mode];
}

expression_result interpret(interpreter* interpreter, void* ast_node)
{
    int element_type = GET_ELEMENT_TYPE(ast_node);
    int element_supertype = GET_ELEMENT_SUPERTYPE(ast_node);
    
    if (element_supertype == Statement)
    {
        switch (element_type)
        {
            case StatementList_stmt:
                void** statement_ptrs = (void**)((StatementList*)(ast_node) + sizeof(StatementList));
                for (size_t i = 0; i < ((StatementList*)(ast_node))->size; i++)
                {
                    interpret(interpreter, *statement_ptrs++);
                    clear_interpreter_memory(&interpreter->memory);
                }

                return (expression_result) {.type = NONE};

            case Print_stmt:
                Print* print_stmt = (Print*)ast_node;
                expression_result result = interpret(interpreter, print_stmt->expression);
                string result_str = cast_to_string(interpreter, result);
                printf("%.*s", result_str.length, result_str.string_value);

                return (expression_result) {.type = NONE};

            default:
                PRINT_INTERPRETER_ERROR_AND_QUIT("Unknown statement type ID %d\n", element_type);
        }
    }

    else if (element_supertype == Expression)
    {
        switch (element_type)
        {            
            case Integer_expr:
                return (expression_result) {.type = INT_VALUE, .value.int_value = ((Integer*)ast_node)->value};
        
            case Float_expr:
                return (expression_result) {.type = FLOAT_VALUE, .value.float_value = ((Float*)ast_node)->value};

            case Bool_expr:
                return ((Bool*)ast_node)->value ? true_expression : false_expression;

            case String_expr:
                return (expression_result) {.type = STRING_VALUE, .value.string_value.string_value = ((String*)ast_node)->string, .value.string_value.length = ((String*)ast_node)->length};
        
            case BinOp_expr:
                expression_result lhs = interpret(interpreter, ((BinOp*)ast_node)->left);
            token_type binop_op = ((BinOp*)ast_node)->op;

            // Logic operand shortcuts
            if (binop_op == TOK_AND)
            {
                if (cast_to_bool(interpreter, lhs))
                {
                    return interpret(interpreter, ((BinOp*)ast_node)->right);
                }
                return lhs;
            }

            if (binop_op == TOK_OR)
            {
                if (cast_to_bool(interpreter, lhs))
                {
                    return lhs;
                }
                return interpret(interpreter, ((BinOp*)ast_node)->right);
            }
            
            expression_result rhs = interpret(interpreter, ((BinOp*)ast_node)->right);
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
                            if (rhs.value.int_value == 0)
                            {
                                PRINT_INTERPRETER_ERROR_AND_QUIT("Division by zero\n");
                            }
                            return (expression_result) {.type = INT_VALUE, .value.int_value = lhs.value.int_value / rhs.value.int_value};
                        }

                // Case 2 for division: one value is a float. Return a float
                if (lhs.type == FLOAT_VALUE && (rhs.type == FLOAT_VALUE || rhs.type == INT_VALUE))
                {
                    if (((rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value)) == 0.0)
                    {
                        PRINT_INTERPRETER_ERROR_AND_QUIT("Division by zero\n");
                    }
                    return (expression_result) {.type = FLOAT_VALUE, .value.float_value = lhs.value.float_value / ((rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value))};
                }

                if (rhs.type == FLOAT_VALUE && (lhs.type == FLOAT_VALUE || lhs.type == INT_VALUE))
                {
                    if (rhs.value.float_value == 0)
                    {
                        PRINT_INTERPRETER_ERROR_AND_QUIT("Division by zero\n");
                    }
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

                // Comparison operations. All behave similarly: if both operands are numbers or bools, their values are compared.
                // If they are strings, they are compared lexicographically and by length, like Python.
                case TOK_GT:
                    if ((lhs.type == INT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == BOOL_VALUE))
                    {
                        int converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : lhs.value.bool_value;
                        int converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : rhs.value.bool_value;
                        return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left > converted_val_right};
                    }
                    
                if ((lhs.type == INT_VALUE || lhs.type == FLOAT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == FLOAT_VALUE || rhs.type == BOOL_VALUE))
                {
                    double converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : (lhs.type == FLOAT_VALUE) ? lhs.value.float_value : lhs.value.bool_value;
                    double converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : (rhs.type == FLOAT_VALUE) ? rhs.value.float_value : rhs.value.bool_value;
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left > converted_val_right};
                }

                if (lhs.type == STRING_VALUE && rhs.type == STRING_VALUE)
                {
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(lhs.value.string_value, rhs.value.string_value, COMPARE_GT)};
                }

                // Otherwise, this is an unsupported operation. Error-out
                PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                case TOK_LT:
                    if ((lhs.type == INT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == BOOL_VALUE))
                    {
                        int converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : lhs.value.bool_value;
                        int converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : rhs.value.bool_value;
                        return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left < converted_val_right};
                    }
                    
                if ((lhs.type == INT_VALUE || lhs.type == FLOAT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == FLOAT_VALUE || rhs.type == BOOL_VALUE))
                {
                    double converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : (lhs.type == FLOAT_VALUE) ? lhs.value.float_value : lhs.value.bool_value;
                    double converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : (rhs.type == FLOAT_VALUE) ? rhs.value.float_value : rhs.value.bool_value;
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left < converted_val_right};
                }

                if (lhs.type == STRING_VALUE && rhs.type == STRING_VALUE)
                {
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(lhs.value.string_value, rhs.value.string_value, COMPARE_LT)};
                }

                // Otherwise, this is an unsupported operation. Error-out
                PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);
                    
                case TOK_GE:
                    if ((lhs.type == INT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == BOOL_VALUE))
                    {
                        int converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : lhs.value.bool_value;
                        int converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : rhs.value.bool_value;
                        return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left >= converted_val_right};
                    }
                    
                if ((lhs.type == INT_VALUE || lhs.type == FLOAT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == FLOAT_VALUE || rhs.type == BOOL_VALUE))
                {
                    double converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : (lhs.type == FLOAT_VALUE) ? lhs.value.float_value : lhs.value.bool_value;
                    double converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : (rhs.type == FLOAT_VALUE) ? rhs.value.float_value : rhs.value.bool_value;
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left >= converted_val_right};
                }

                if (lhs.type == STRING_VALUE && rhs.type == STRING_VALUE)
                {
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(lhs.value.string_value, rhs.value.string_value, COMPARE_GE)};
                }

                // Otherwise, this is an unsupported operation. Error-out
                PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                case TOK_LE:
                    if ((lhs.type == INT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == BOOL_VALUE))
                    {
                        int converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : lhs.value.bool_value;
                        int converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : rhs.value.bool_value;
                        return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left <= converted_val_right};
                    }
                    
                if ((lhs.type == INT_VALUE || lhs.type == FLOAT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == FLOAT_VALUE || rhs.type == BOOL_VALUE))
                {
                    double converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : (lhs.type == FLOAT_VALUE) ? lhs.value.float_value : lhs.value.bool_value;
                    double converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : (rhs.type == FLOAT_VALUE) ? rhs.value.float_value : rhs.value.bool_value;
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left <= converted_val_right};
                }

                if (lhs.type == STRING_VALUE && rhs.type == STRING_VALUE)
                {
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(lhs.value.string_value, rhs.value.string_value, COMPARE_LE)};
                }

                // Otherwise, this is an unsupported operation. Error-out
                PRINT_INTERPRETER_ERROR_AND_QUIT("Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                case TOK_EQEQ:
                    if ((lhs.type == INT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == BOOL_VALUE))
                    {
                        int converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : lhs.value.bool_value;
                        int converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : rhs.value.bool_value;
                        return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left == converted_val_right};
                    }
                    
                if ((lhs.type == INT_VALUE || lhs.type == FLOAT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == FLOAT_VALUE || rhs.type == BOOL_VALUE))
                {
                    double converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : (lhs.type == FLOAT_VALUE) ? lhs.value.float_value : lhs.value.bool_value;
                    double converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : (rhs.type == FLOAT_VALUE) ? rhs.value.float_value : rhs.value.bool_value;
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left == converted_val_right};
                }

                if (lhs.type == STRING_VALUE && rhs.type == STRING_VALUE)
                {
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(lhs.value.string_value, rhs.value.string_value, COMPARE_EQ)};
                }

                // Otherwise, we are comparing non-comparable types. Return false
                return false_expression;

                case TOK_NE:
                    if ((lhs.type == INT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == BOOL_VALUE))
                    {
                        int converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : lhs.value.bool_value;
                        int converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : rhs.value.bool_value;
                        return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left != converted_val_right};
                    }
                    
                if ((lhs.type == INT_VALUE || lhs.type == FLOAT_VALUE || lhs.type == BOOL_VALUE) && (rhs.type == INT_VALUE || rhs.type == FLOAT_VALUE || rhs.type == BOOL_VALUE))
                {
                    double converted_val_left = (lhs.type == INT_VALUE) ? lhs.value.int_value : (lhs.type == FLOAT_VALUE) ? lhs.value.float_value : lhs.value.bool_value;
                    double converted_val_right = (rhs.type == INT_VALUE) ? rhs.value.int_value : (rhs.type == FLOAT_VALUE) ? rhs.value.float_value : rhs.value.bool_value;
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = converted_val_left != converted_val_right};
                }

                if (lhs.type == STRING_VALUE && rhs.type == STRING_VALUE)
                {
                    return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(lhs.value.string_value, rhs.value.string_value, COMPARE_NE)};
                }

                // Otherwise, we are comparing non-comparable types. Return true
                return true_expression;
                        
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

    PRINT_INTERPRETER_ERROR_AND_QUIT("Unknown element supertype ID %d\n", element_type);
}
