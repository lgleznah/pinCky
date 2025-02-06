#pragma once
#include <stdint.h>

#include "arrays.h"
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
    Identifier_expr,
    BinOp_expr,
    UnOp_expr,
    Grouping_expr
};

enum STATEMENT_TYPES
{
    StatementList_stmt,
    While_stmt,
    Assignment_stmt,
    Print_stmt,
    If_stmt,
    For_stmt
};

enum SUPERTYPES
{
    Expression,
    Statement
};

#define SET_ELEMENT_TYPE(supertype, subtype)           (((supertype) << 7) | (subtype))
#define CHECK_ELEMENT_SUPERTYPE(element, supertype)    (((((Element*)(element))->tag & 0x80) >> 7) == (supertype))
#define CHECK_ELEMENT_TYPE(element, type)              (((((Element*)(element))->tag & 0x7F)) == (type))
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
    size_t (*element_size) (const Element* element);
    void (*compute_ptr) (Element* element, void* ast_base);
} ElementInterface;

inline void print_element(const Element* element, int depth) { element->vtable->print_element(element, depth); }
inline size_t element_size(const Element* element) { return element->vtable->element_size(element); }
inline void compute_ptr(Element* element, void* ast_base) { element->vtable->compute_ptr(element, ast_base); }

// Numbers like 42
typedef struct Integer
{
    Element base;
    int value;
} Integer;

void init_Integer(Integer* integer_elem, int value, int line);
void print_Integer(const Element* integer_elem, int depth);
size_t element_size_Integer(const Element* integer_elem);
void compute_ptr_Integer(Element* integer_elem, void* ast_base);

// Numbers like 4.20
typedef struct Float
{
    Element base;
    double value;
} Float;

void init_Float(Float* float_elem, double value, int line);
void print_Float(const Element* float_elem, int depth);
size_t element_size_Float(const Element* float_elem);
void compute_ptr_Float(Element* float_elem, void* ast_base);


// Just true or false
typedef struct Bool
{
    Element base;
    char value;
} Bool;

void init_Bool(Bool* bool_elem, char value, int line);
void print_Bool(const Element* bool_elem, int depth);
size_t element_size_Bool(const Element* bool_elem);
void compute_ptr_Bool(Element* bool_elem, void* ast_base);

// Text strings like "foobar"
typedef struct String
{
    Element base;
    char* string;
    int length;
} String;

void init_String(String* string_elem, char* string, int length, int line);
void print_String(const Element* string_elem, int depth);
size_t element_size_String(const Element* string_elem);
void compute_ptr_String(Element* string_elem, void* ast_base);

// Variable names like "x", "PI", "_score", "start_vel", "numLives"
typedef struct Identifier
{
    Element base;
    string_type name;
} Identifier;

void init_Identifier(Identifier* identifier_elem, char* name, int length, int line);
void print_Identifier(const Element* identifier_elem, int depth);
size_t element_size_Identifier(const Element* identifier_elem);
void compute_ptr_Identifier(Element* identifier_elem, void* ast_base);

// Operations like x + y
typedef struct BinOp
{
    Element base;
    token_type op;
    void* left;
    void* right;
} BinOp;

void init_BinOp(BinOp* binop_elem, token_type op, size_t left, size_t right, void* ast_base, int line);
void print_BinOp(const Element* binop_elem, int depth);
size_t element_size_BinOp(const Element* binop_elem);
void compute_ptr_BinOp(Element* binop_elem, void* ast_base);

// Operations like -y
typedef struct UnOp
{
    Element base;
    token_type op;
    void* operand;
} UnOp;

void init_UnOp(UnOp* unop_elem, token_type op, size_t operand, void* ast_base, int line);
void print_UnOp(const Element* unop_elem, int depth);
size_t element_size_UnOp(const Element* unop_elem);
void compute_ptr_UnOp(Element* unop_elem, void* ast_base);

// Parenthesized expressions ( <expr> )
typedef struct Grouping
{
    Element base;
    void* expression;
} Grouping;

void init_Grouping(Grouping* grouping_elem, size_t expression, void* ast_base, int line);
void print_Grouping(const Element* grouping_elem, int depth);
size_t element_size_Grouping(const Element* grouping_elem);
void compute_ptr_Grouping(Element* grouping_elem, void* ast_base);

// List of statements
typedef struct StatementList
{
    Element base;
    size_t size;
} StatementList;

void init_StatementList(StatementList* statement_list_elem, statement_array* array, void* ast_base, int line);
void print_StatementList(const Element* statement_list_elem, int depth);
size_t element_size_StatementList(const Element* statement_list_elem);
void compute_ptr_StatementList(Element* statement_list_elem, void* ast_base);

// While loops
typedef struct While
{
    Element base;
    void* condition;
    void* statements;
} While;

void init_While(While* while_elem, size_t condition, size_t statements, void* ast_base, int line);
void print_While(const Element* while_elem, int depth);
size_t element_size_While(const Element* while_elem);
void compute_ptr_While(Element* while_elem, void* ast_base);

// Assignment statements
typedef struct Assignment
{
    Element base;
    void* lhs;
    void* rhs;
} Assignment;

void init_Assignment(Assignment* assignment_elem, size_t lhs, size_t rhs, void* ast_base, int line);
void print_Assignment(const Element* assignment_elem, int depth);
size_t element_size_Assignment(const Element* assignment_elem);
void compute_ptr_Assignment(Element* assignment_elem, void* ast_base);

// Print statements
typedef struct Print
{
    Element base;
    char break_line;
    void* expression;
} Print;

void init_Print(Print* print_elem, char break_line, size_t expression, void* ast_base, int line);
void print_Print(const Element* print_elem, int depth);
size_t element_size_Print(const Element* print_elem);
void compute_ptr_Print(Element* print_elem, void* ast_base);

// If-then-else statements
typedef struct If
{
    Element base;
    void* condition;
    void* then_branch;
    void* else_branch;
} If;

void init_If(If* if_elem, size_t condition, size_t then_branch, size_t else_branch, void* ast_base, int line);
void print_If(const Element* if_elem, int depth);
size_t element_size_If(const Element* if_elem);
void compute_ptr_If(Element* if_elem, void* ast_base);

// For-loop statements
typedef struct For
{
    Element base;
    void* initial_assignment;
    void* stop;
    void* step;
    void* statements;
} For;

void init_For(For* for_elem, size_t initial_assignment, size_t stop, size_t step, size_t statements, void* ast_base, int line);
void print_For(const Element* for_elem, int depth);
size_t element_size_For(const Element* for_elem);
void compute_ptr_For(Element* for_elem, void* ast_base);