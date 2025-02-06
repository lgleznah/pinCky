#include "interpreter.h"

#include <math.h>

#include "model.h"
#include "types.h"
#include "utils.h"

void init_interpreter(interpreter* interpreter)
{
    init_vss_array(&interpreter->memory, 65535);
}

void free_interpreter(interpreter* interpreter)
{
    free_vss_array(&interpreter->memory);
}

expression_result interpret(interpreter* interpreter, void* ast_node, environment* env)
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
                    interpret(interpreter, *statement_ptrs++, env);
                    clear_vss_array(&interpreter->memory);
                }

                return (expression_result) {.type = NONE};

            case Print_stmt:
                Print* print_stmt = (Print*)ast_node;
                expression_result result = interpret(interpreter, print_stmt->expression, env);
                string_type result_str = cast_to_string(interpreter, result);
                printf("%.*s", result_str.length, result_str.string_value);
                if (print_stmt->break_line)
                {
                    printf("\n");
                }

                return (expression_result) {.type = NONE};

            case If_stmt:
                If* if_stmt = (If*)ast_node;
                expression_result test_result = interpret(interpreter, if_stmt->condition, env);
                if (test_result.type != BOOL_VALUE)
                {
                    PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Condition test is not boolean\n");
                }

                if (test_result.value.bool_value)
                {
                    environment child;
                    init_environment(&child, env);
                    interpret(interpreter, if_stmt->then_branch, &child);
                    clear_environment(&child);
                }
                else if(if_stmt->else_branch != NULL)
                {
                    environment child;
                    init_environment(&child, env);
                    interpret(interpreter, if_stmt->else_branch, &child);
                    clear_environment(&child);
                }

                return (expression_result) {.type = NONE};

            case Assignment_stmt:
                Assignment* assignment_stmt = ast_node;
                Identifier* lhs_identifier = assignment_stmt->lhs;
                expression_result rhs_result = interpret(interpreter, assignment_stmt->rhs, env);
                set_variable(env, lhs_identifier->name, rhs_result);

                return (expression_result) {.type = NONE};
            
            default:
                PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unknown statement type ID %d\n", element_type);
        }
    }

    if (element_supertype == Expression)
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

            case Identifier_expr:
                return get_variable(env, ((Identifier*)ast_node)->name, element_line);
        
            case BinOp_expr:
                expression_result lhs = interpret(interpreter, ((BinOp*)ast_node)->left, env);
                token_type binop_op = ((BinOp*)ast_node)->op;

                // Logic operand shortcuts
                if (binop_op == TOK_AND)
                {
                    if (cast_to_bool(interpreter, lhs))
                    {
                        return interpret(interpreter, ((BinOp*)ast_node)->right, env);
                    }
                    return lhs;
                }

                if (binop_op == TOK_OR)
                {
                    if (cast_to_bool(interpreter, lhs))
                    {
                        return lhs;
                    }
                    return interpret(interpreter, ((BinOp*)ast_node)->right, env);
                }
                
                expression_result rhs = interpret(interpreter, ((BinOp*)ast_node)->right, env);
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
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

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
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

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
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

                    case TOK_SLASH:
                        // Case 1 for division: both are integers. Return an integer
                        if (lhs.type == INT_VALUE && rhs.type == INT_VALUE)
                        {
                            if (rhs.value.int_value == 0)
                            {
                                PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Division by zero\n");
                            }
                            return (expression_result) {.type = INT_VALUE, .value.int_value = lhs.value.int_value / rhs.value.int_value};
                        }

                        // Case 2 for division: one value is a float. Return a float
                        if (lhs.type == FLOAT_VALUE && (rhs.type == FLOAT_VALUE || rhs.type == INT_VALUE))
                        {
                            if (((rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value)) == 0.0)
                            {
                                PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Division by zero\n");
                            }
                            return (expression_result) {.type = FLOAT_VALUE, .value.float_value = lhs.value.float_value / ((rhs.type == FLOAT_VALUE) ? rhs.value.float_value : (double) (rhs.value.int_value))};
                        }

                        if (rhs.type == FLOAT_VALUE && (lhs.type == FLOAT_VALUE || lhs.type == INT_VALUE))
                        {
                            if (rhs.value.float_value == 0)
                            {
                                PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Division by zero\n");
                            }
                            return (expression_result) {.type = FLOAT_VALUE, .value.float_value = ((lhs.type == FLOAT_VALUE) ? lhs.value.float_value : (double) (lhs.value.int_value)) / rhs.value.float_value};
                        }

                        // Otherwise, this is an unsupported operation. Error-out
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

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
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);
                    
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
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

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
                            return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs.value.string_value, &rhs.value.string_value, COMPARE_GT)};
                        }

                        // Otherwise, this is an unsupported operation. Error-out
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

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
                            return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs.value.string_value, &rhs.value.string_value, COMPARE_LT)};
                        }

                        // Otherwise, this is an unsupported operation. Error-out
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);
                        
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
                            return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs.value.string_value, &rhs.value.string_value, COMPARE_GE)};
                        }

                        // Otherwise, this is an unsupported operation. Error-out
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

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
                            return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs.value.string_value, &rhs.value.string_value, COMPARE_LE)};
                        }

                        // Otherwise, this is an unsupported operation. Error-out
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported operation '%s' between '%s' and '%s'\n", token_symbols[binop_op], type_names[lhs.type], type_names[rhs.type]);

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
                            return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs.value.string_value, &rhs.value.string_value, COMPARE_EQ)};
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
                            return (expression_result) {.type = BOOL_VALUE, .value.bool_value = string_comparison(&lhs.value.string_value, &rhs.value.string_value, COMPARE_NE)};
                        }

                        // Otherwise, we are comparing non-comparable types. Return true
                        return true_expression;
                            
                    default:
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unknown binary operation '%s'\n", token_symbols[binop_op]);
                }
            

            case UnOp_expr:
                expression_result operand = interpret(interpreter, ((UnOp*)ast_node)->operand, env);
                token_type un_op = ((UnOp*)ast_node)->op;
                switch (un_op)
                {
                    case TOK_PLUS:
                        // Unary plus is only supported for ints and floats.
                        if (operand.type == INT_VALUE || operand.type == FLOAT_VALUE)
                        {
                            return operand;
                        }

                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported unary operation '%s' for '%s'\n", token_symbols[un_op], type_names[operand.type]);

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

                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported unary operation '%s' for '%s'\n", token_symbols[un_op], type_names[operand.type]);
                        
                    case TOK_NOT:
                        // Unary not is only supported for bools
                        if (operand.type == BOOL_VALUE)
                        {
                            return (expression_result) {.type = BOOL_VALUE, .value.bool_value = !operand.value.bool_value};
                        }

                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unsupported unary operation '%s' for '%s'\n", token_symbols[un_op], type_names[operand.type]);

                    default:
                        PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unknown unary operation '%s'\n", token_symbols[un_op]);
                }
            

            case Grouping_expr:
                return interpret(interpreter, ((Grouping*)ast_node)->expression, env);

            default:
                PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unknown expression type ID %d\n", element_type);
        }
    }

    PRINT_INTERPRETER_ERROR_AND_QUIT(element_line, "Unknown element supertype ID %d\n", element_supertype);
}

void interpret_ast(interpreter* interpreter, void* ast_node)
{
    // Create new environment, and interpret the AST
    environment env;
    init_environment(&env, NULL);
    interpret(interpreter, ast_node, &env);
    clear_environment(&env);
}