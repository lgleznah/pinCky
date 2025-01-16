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
    void (*update_on_realloc) (Element* element, long long ptr_diff);
} ElementInterface;

inline void print_element(const Element* element, int depth) { element->vtable->print_element(element, depth); }
inline size_t element_size(const Element* element) { return element->vtable->element_size(element); }
inline void update_on_realloc(Element* element, long long ptr_diff) { element->vtable->update_on_realloc(element, ptr_diff); }

// Numbers like 42
typedef struct Integer
{
    Element base;
    int value;
} Integer;

int init_Integer(Integer* integer_elem, int value, int line);
void print_Integer(const Element* integer_elem, int depth);
size_t element_size_Integer(const Element* integer_elem);
void update_on_realloc_Integer(Element* integer_elem, long long ptr_diff);

// Numbers like 4.20
typedef struct Float
{
    Element base;
    double value;
} Float;

int init_Float(Float* float_elem, double value, int line);
void print_Float(const Element* float_elem, int depth);
size_t element_size_Float(const Element* float_elem);
void update_on_realloc_Float(Element* float_elem, long long ptr_diff);


// Just true or false
typedef struct Bool
{
    Element base;
    char value;
} Bool;

int init_Bool(Bool* bool_elem, char value, int line);
void print_Bool(const Element* bool_elem, int depth);
size_t element_size_Bool(const Element* bool_elem);
void update_on_realloc_Bool(Element* bool_elem, long long ptr_diff);

// Text strings like "foobar"
typedef struct String
{
    Element base;
    char* string;
    int length;
} String;

int init_String(String* string_elem, char* string, int length, int line);
void print_String(const Element* string_elem, int depth);
size_t element_size_String(const Element* string_elem);
void update_on_realloc_String(Element* string_elem, long long ptr_diff);

// Operations like x + y
typedef struct BinOp
{
    Element base;
    token_type op;
    void* left;
    void* right;
} BinOp;

int init_BinOp(BinOp* binop_elem, token_type op, void* left, void* right, int line);
void print_BinOp(const Element* binop_elem, int depth);
size_t element_size_BinOp(const Element* binop_elem);
void update_on_realloc_BinOp(Element* binop_elem, long long ptr_diff);

// Operations like -y
typedef struct UnOp
{
    Element base;
    token_type op;
    void* operand;
} UnOp;

int init_UnOp(UnOp* unop_elem, token_type op, void* operand, int line);
void print_UnOp(const Element* unop_elem, int depth);
size_t element_size_UnOp(const Element* unop_elem);
void update_on_realloc_UnOp(Element* unop_elem, long long ptr_diff);

// Parenthesized expressions ( <expr> )
typedef struct Grouping
{
    Element base;
    void* expression;
} Grouping;

int init_Grouping(Grouping* grouping_elem, void* expression, int line);
void print_Grouping(const Element* grouping_elem, int depth);
size_t element_size_Grouping(const Element* grouping_elem);
void update_on_realloc_Grouping(Element* grouping_elem, long long ptr_diff);

// List of statements
typedef struct StatementList
{
    Element base;
    size_t size;
} StatementList;

int init_StatementList(StatementList* statement_list_elem, statement_array* array, void* ast_base, int line);
void print_StatementList(const Element* statement_list_elem, int depth);
size_t element_size_StatementList(const Element* statement_list_elem);
void update_on_realloc_StatementList(Element* statement_list_elem, long long ptr_diff);

// While loops
typedef struct While
{
    Element base;
} While;

int init_While(While* while_elem, int line);
void print_While(const Element* while_elem, int depth);
size_t element_size_While(const Element* while_elem);
void update_on_realloc_While(Element* while_elem, long long ptr_diff);

// Assignment statements
typedef struct Assignment
{
    Element base;
} Assignment;

int init_Assignment(Assignment* assignment_elem, int line);
void print_Assignment(const Element* assignment_elem, int depth);
size_t element_size_Assignment(const Element* assignment_elem);
void update_on_realloc_Assignment(Element* assignment_elem, long long ptr_diff);

// Print statements
typedef struct Print
{
    Element base;
    void* expression;
} Print;

int init_Print(Print* print_elem, void* expression, int line);
void print_Print(const Element* print_elem, int depth);
size_t element_size_Print(const Element* print_elem);
void update_on_realloc_Print(Element* print_elem, long long ptr_diff);

// If-then-else statements
typedef struct If
{
    Element base;
} If;

int init_If(If* if_elem, int line);
void print_If(const Element* if_elem, int depth);
size_t element_size_If(const Element* if_elem);
void update_on_realloc_If(Element* if_elem, long long ptr_diff);

// For-loop statements
typedef struct For
{
    Element base;
} For;

int init_For(For* for_elem, int line);
void print_For(const Element* for_elem, int depth);
size_t element_size_For(const Element* for_elem);
void update_on_realloc_For(Element* for_elem, long long ptr_diff);