#pragma once
#include <stdint.h>

#include "tokens.h"

// All AST elements are modelled as structs, where the first field is
// a static const 8-bit unsigned integer named "tag".
// This tag is of the form "TXXXXXXX", where T is 0 for expressions
// and 1 for statements, and XXXXXXX is the index of the specific type
// of element (whether expression or statement)

enum EXPRESSION_TYPES
{
    Integer_expr,
    Float_expr,
    BinOp_expr,
    UnOp_expr,
    Grouping_expr
};

enum STATEMENT_TYPES
{
    While_stmt,
    Assignment_stmt
};

enum SUPERTYPES
{
    Expression,
    Statement
};

#define SET_ELEMENT_TYPE(supertype, subtype)           (((supertype) << 7) | (subtype))
#define CHECK_ELEMENT_SUPERTYPE(element, supertype)    (((((Element*)(element))->tag & 0x80) >> 7) == (supertype))
#define GET_ELEMENT_SUPERTYPE(element)                 ((((Element*)(element))->tag & 0x80) >> 7)
#define GET_ELEMENT_TYPE(element)                      ((((Element*)(element))->tag & 0x7F))
#define GET_ELEMENT_LINE(element)                      ((((Element*)(element))->line))

// Base struct to convert to for type-checking stuff
typedef struct Element
{
    uint8_t tag;
    int line;
} Element;

// Numbers like 42
typedef struct Integer
{
    uint8_t tag;
    int line;
    int value;
} Integer;

int init_Integer(Integer* integer_elem, int value, int line);
void print_Integer(const Integer* integer_elem, int depth);

// Numbers like 4.20
typedef struct Float
{
    uint8_t tag;
    int line;
    float value;
} Float;

int init_Float(Float* float_elem, float value, int line);
void print_Float(const Float* float_elem, int depth);

// Operations like x + y
typedef struct BinOp
{
    uint8_t tag;
    int line;
    token_type op;
    void* left;
    void* right;
} BinOp;

int init_BinOp(BinOp* bin_op, token_type op, void* left, void* right, int line);
void print_BinOp(const BinOp* bin_op, int depth);

// Operations like -y
typedef struct UnOp
{
    uint8_t tag;
    int line;
    token_type op;
    void* operand;
} UnOp;

int init_UnOp(UnOp* un_op, token_type op, void* operand, int line);
void print_UnOp(const UnOp* un_op, int depth);

// Parenthesized expressions ( <expr> )
typedef struct Grouping
{
    uint8_t tag;
    int line;
    void* expression;
} Grouping;

int init_Grouping(Grouping* grouping_elem, void* expression, int line);
void print_Grouping(const Grouping* grouping_elem, int depth);

// While loops
typedef struct While
{
    uint8_t tag;
    int line;
} While;

int init_While(While* while_stmt, int line);
void print_While(const While* while_stmt, int depth);

// Assignment statements
typedef struct Assignment
{
    uint8_t tag;
    int line;
} Assignment;

int init_Assignment(Assignment* assignment_elem, int line);
void print_Assignment(const Assignment* assignment_elem, int depth);

// Dynamic type dispatch printing
void ast_print_dispatch(void* element, int depth);