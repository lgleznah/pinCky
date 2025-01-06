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
    Bool_expr,
    String_expr,
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

// Base struct for elements
typedef struct Element
{
    // Data type tag
    uint8_t tag;

    // Line where this element appears
    int line;

    // Vtable for elements interface
    const struct ElementInterface* const vtable;
} Element;

// Base element functions interface
typedef struct ElementInterface
{
    void (*print_element) (const Element* element, int depth);
} ElementInterface;

inline void print_element(const Element* element, int depth) { element->vtable->print_element(element, depth); }

// Numbers like 42
typedef struct Integer
{
    Element base;
    int value;
} Integer;

int init_Integer(Integer* integer_elem, int value, int line);
void print_Integer(const Element* integer_elem, int depth);

// Numbers like 4.20
typedef struct Float
{
    Element base;
    double value;
} Float;

int init_Float(Float* float_elem, double value, int line);
void print_Float(const Element* float_elem, int depth);

// Just true or false
typedef struct Bool
{
    Element base;
    char value;
} Bool;

int init_Bool(Bool* bool_elem, char value, int line);
void print_Bool(const Element* bool_elem, int depth);

// Text strings like "foobar"
typedef struct String
{
    Element base;
    char* string;
    int length;
} String;

int init_String(String* string_elem, char* string, int length, int line);
void print_String(const Element* string_elem, int depth);

// Operations like x + y
typedef struct BinOp
{
    Element base;
    token_type op;
    void* left;
    void* right;
} BinOp;

int init_BinOp(BinOp* bin_op, token_type op, void* left, void* right, int line);
void print_BinOp(const Element* bin_op, int depth);

// Operations like -y
typedef struct UnOp
{
    Element base;
    token_type op;
    void* operand;
} UnOp;

int init_UnOp(UnOp* un_op, token_type op, void* operand, int line);
void print_UnOp(const Element* un_op, int depth);

// Parenthesized expressions ( <expr> )
typedef struct Grouping
{
    Element base;
    void* expression;
} Grouping;

int init_Grouping(Grouping* grouping_elem, void* expression, int line);
void print_Grouping(const Element* grouping_elem, int depth);

// While loops
typedef struct While
{
    Element base;
} While;

int init_While(While* while_stmt, int line);
void print_While(const Element* while_stmt, int depth);

// Assignment statements
typedef struct Assignment
{
    Element base;
} Assignment;

int init_Assignment(Assignment* assignment_elem, int line);
void print_Assignment(const Element* assignment_elem, int depth);