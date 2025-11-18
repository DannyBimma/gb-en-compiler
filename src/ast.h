#ifndef AST_H
#define AST_H

#include "utils.h"
#include "lexer.h"

/* AST Node types */
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_DECLARATION,
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_DO_WHILE,
    NODE_RETURN,
    NODE_BLOCK,
    NODE_FUNCTION_CALL,
    NODE_ARRAY_ACCESS,
    NODE_ASSIGNMENT,
    NODE_BREAK,
    NODE_CONTINUE,
    /* New node types */
    NODE_STRUCT_DEF,
    NODE_MEMBER_ACCESS,
    NODE_SWITCH,
    NODE_CASE,
    NODE_DEFAULT,
    NODE_TERNARY,
    NODE_ENUM_DEF,
    NODE_SIZEOF,
    NODE_CAST,
    NODE_COMPOUND_ASSIGN,
    NODE_GOTO,
    NODE_LABEL,
    NODE_TYPEDEF
} NodeType;

/* Forward declaration */
struct ASTNode;

/* Parameter structure */
typedef struct Parameter {
    char* type;
    char* name;
    int is_array;
} Parameter;

/* AST Node structure */
typedef struct ASTNode {
    NodeType type;
    int line;
    int column;

    union {
        /* Program node */
        struct {
            struct ASTNode** functions;
            int function_count;
        } program;

        /* Function node */
        struct {
            char* return_type;
            char* name;
            Parameter** parameters;
            int param_count;
            struct ASTNode* body;
        } function;

        /* Declaration node */
        struct {
            char* data_type;
            char* name;
            int is_array;
            struct ASTNode* array_size;
            struct ASTNode* initializer;
        } declaration;

        /* If statement node */
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_stmt;

        /* While statement node */
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;

        /* For statement node */
        struct {
            struct ASTNode* init;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } for_stmt;

        /* Return statement node */
        struct {
            struct ASTNode* value;
        } return_stmt;

        /* Block node */
        struct {
            struct ASTNode** statements;
            int statement_count;
        } block;

        /* Binary operation node */
        struct {
            char* operator;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;

        /* Unary operation node */
        struct {
            char* operator;
            struct ASTNode* operand;
        } unary_op;

        /* Function call node */
        struct {
            char* name;
            struct ASTNode** arguments;
            int arg_count;
        } function_call;

        /* Array access node */
        struct {
            char* name;
            struct ASTNode* index;
        } array_access;

        /* Assignment node */
        struct {
            struct ASTNode* target;
            struct ASTNode* value;
        } assignment;

        /* Literal node */
        struct {
            char* value;
            char* data_type;  /* "int", "float", "string", "char" */
        } literal;

        /* Identifier node */
        struct {
            char* name;
        } identifier;

        /* Struct/Union definition node */
        struct {
            char* name;
            int is_union;
            struct ASTNode** members;
            int member_count;
        } struct_def;

        /* Member access node (. and ->) */
        struct {
            struct ASTNode* object;
            char* member;
            int is_arrow;  /* 1 for ->, 0 for . */
        } member_access;

        /* Switch statement node */
        struct {
            struct ASTNode* expression;
            struct ASTNode** cases;
            int case_count;
        } switch_stmt;

        /* Case node */
        struct {
            struct ASTNode* value;  /* NULL for default */
            struct ASTNode** statements;
            int statement_count;
        } case_stmt;

        /* Ternary operator node */
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_expr;
            struct ASTNode* else_expr;
        } ternary;

        /* Enum definition node */
        struct {
            char* name;
            char** values;
            int value_count;
        } enum_def;

        /* Sizeof expression node */
        struct {
            char* type_name;
            struct ASTNode* expression;  /* Either type_name or expression */
        } sizeof_expr;

        /* Cast expression node */
        struct {
            char* target_type;
            struct ASTNode* expression;
        } cast;

        /* Compound assignment node */
        struct {
            char* operator;  /* +=, -=, *=, etc. */
            struct ASTNode* target;
            struct ASTNode* value;
        } compound_assign;

        /* Goto node */
        struct {
            char* label;
        } goto_stmt;

        /* Label node */
        struct {
            char* name;
            struct ASTNode* statement;
        } label_stmt;

        /* Typedef node */
        struct {
            char* original_type;
            char* new_name;
        } typedef_stmt;

    } data;
} ASTNode;

/* AST node creation functions */
ASTNode* ast_create_program(void);
ASTNode* ast_create_function(const char* return_type, const char* name, Parameter** params, int param_count, ASTNode* body);
ASTNode* ast_create_declaration(const char* type, const char* name, ASTNode* initializer);
ASTNode* ast_create_array_declaration(const char* type, const char* name, ASTNode* size);
ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);
ASTNode* ast_create_while(ASTNode* condition, ASTNode* body);
ASTNode* ast_create_for(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* ast_create_return(ASTNode* value);
ASTNode* ast_create_block(void);
ASTNode* ast_create_binary_op(const char* operator, ASTNode* left, ASTNode* right);
ASTNode* ast_create_unary_op(const char* operator, ASTNode* operand);
ASTNode* ast_create_function_call(const char* name, ASTNode** args, int arg_count);
ASTNode* ast_create_array_access(const char* name, ASTNode* index);
ASTNode* ast_create_assignment(ASTNode* target, ASTNode* value);
ASTNode* ast_create_literal(const char* value, const char* type);
ASTNode* ast_create_identifier(const char* name);
ASTNode* ast_create_break(void);
ASTNode* ast_create_continue(void);
ASTNode* ast_create_do_while(ASTNode* body, ASTNode* condition);
ASTNode* ast_create_struct_def(const char* name, int is_union);
ASTNode* ast_create_member_access(ASTNode* object, const char* member, int is_arrow);
ASTNode* ast_create_switch(ASTNode* expression);
ASTNode* ast_create_case(ASTNode* value);
ASTNode* ast_create_default(void);
ASTNode* ast_create_ternary(ASTNode* condition, ASTNode* then_expr, ASTNode* else_expr);
ASTNode* ast_create_enum_def(const char* name);
ASTNode* ast_create_sizeof_type(const char* type_name);
ASTNode* ast_create_sizeof_expr(ASTNode* expression);
ASTNode* ast_create_cast(const char* target_type, ASTNode* expression);
ASTNode* ast_create_compound_assign(const char* op, ASTNode* target, ASTNode* value);
ASTNode* ast_create_goto(const char* label);
ASTNode* ast_create_label(const char* name, ASTNode* statement);
ASTNode* ast_create_typedef(const char* original_type, const char* new_name);

/* Helper functions */
void ast_add_struct_member(ASTNode* struct_def, ASTNode* member);
void ast_add_case(ASTNode* switch_stmt, ASTNode* case_node);
void ast_add_case_statement(ASTNode* case_node, ASTNode* statement);
void ast_add_enum_value(ASTNode* enum_def, const char* value);
void ast_add_function(ASTNode* program, ASTNode* function);
void ast_add_statement(ASTNode* block, ASTNode* statement);
Parameter* parameter_create(const char* type, const char* name, int is_array);
void parameter_destroy(Parameter* param);

/* AST destruction */
void ast_destroy(ASTNode* node);

/* Debug printing */
void ast_print(ASTNode* node, int indent);

#endif /* AST_H */
