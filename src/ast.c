#include "ast.h"

/* Helper to create base node */
static ASTNode* ast_create_node(NodeType type) {
    ASTNode* node = (ASTNode*)safe_malloc(sizeof(ASTNode));
    node->type = type;
    node->line = 0;
    node->column = 0;
    return node;
}

/* Node creation functions */

ASTNode* ast_create_program(void) {
    ASTNode* node = ast_create_node(NODE_PROGRAM);
    node->data.program.functions = NULL;
    node->data.program.function_count = 0;
    return node;
}

ASTNode* ast_create_function(const char* return_type, const char* name, Parameter** params, int param_count, ASTNode* body) {
    ASTNode* node = ast_create_node(NODE_FUNCTION);
    node->data.function.return_type = string_duplicate(return_type);
    node->data.function.name = string_duplicate(name);
    node->data.function.parameters = params;
    node->data.function.param_count = param_count;
    node->data.function.body = body;
    return node;
}

ASTNode* ast_create_declaration(const char* type, const char* name, ASTNode* initializer) {
    ASTNode* node = ast_create_node(NODE_DECLARATION);
    node->data.declaration.data_type = string_duplicate(type);
    node->data.declaration.name = string_duplicate(name);
    node->data.declaration.is_array = 0;
    node->data.declaration.array_size = NULL;
    node->data.declaration.initializer = initializer;
    return node;
}

ASTNode* ast_create_array_declaration(const char* type, const char* name, ASTNode* size) {
    ASTNode* node = ast_create_node(NODE_DECLARATION);
    node->data.declaration.data_type = string_duplicate(type);
    node->data.declaration.name = string_duplicate(name);
    node->data.declaration.is_array = 1;
    node->data.declaration.array_size = size;
    node->data.declaration.initializer = NULL;
    return node;
}

ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    ASTNode* node = ast_create_node(NODE_IF);
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* ast_create_while(ASTNode* condition, ASTNode* body) {
    ASTNode* node = ast_create_node(NODE_WHILE);
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode* ast_create_for(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ASTNode* node = ast_create_node(NODE_FOR);
    node->data.for_stmt.init = init;
    node->data.for_stmt.condition = condition;
    node->data.for_stmt.increment = increment;
    node->data.for_stmt.body = body;
    return node;
}

ASTNode* ast_create_return(ASTNode* value) {
    ASTNode* node = ast_create_node(NODE_RETURN);
    node->data.return_stmt.value = value;
    return node;
}

ASTNode* ast_create_block(void) {
    ASTNode* node = ast_create_node(NODE_BLOCK);
    node->data.block.statements = NULL;
    node->data.block.statement_count = 0;
    return node;
}

ASTNode* ast_create_binary_op(const char* operator, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_create_node(NODE_BINARY_OP);
    node->data.binary_op.operator = string_duplicate(operator);
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

ASTNode* ast_create_unary_op(const char* operator, ASTNode* operand) {
    ASTNode* node = ast_create_node(NODE_UNARY_OP);
    node->data.unary_op.operator = string_duplicate(operator);
    node->data.unary_op.operand = operand;
    return node;
}

ASTNode* ast_create_function_call(const char* name, ASTNode** args, int arg_count) {
    ASTNode* node = ast_create_node(NODE_FUNCTION_CALL);
    node->data.function_call.name = string_duplicate(name);
    node->data.function_call.arguments = args;
    node->data.function_call.arg_count = arg_count;
    return node;
}

ASTNode* ast_create_array_access(const char* name, ASTNode* index) {
    ASTNode* node = ast_create_node(NODE_ARRAY_ACCESS);
    node->data.array_access.name = string_duplicate(name);
    node->data.array_access.index = index;
    return node;
}

ASTNode* ast_create_assignment(ASTNode* target, ASTNode* value) {
    ASTNode* node = ast_create_node(NODE_ASSIGNMENT);
    node->data.assignment.target = target;
    node->data.assignment.value = value;
    return node;
}

ASTNode* ast_create_literal(const char* value, const char* type) {
    ASTNode* node = ast_create_node(NODE_LITERAL);
    node->data.literal.value = string_duplicate(value);
    node->data.literal.data_type = string_duplicate(type);
    return node;
}

ASTNode* ast_create_identifier(const char* name) {
    ASTNode* node = ast_create_node(NODE_IDENTIFIER);
    node->data.identifier.name = string_duplicate(name);
    return node;
}

ASTNode* ast_create_break(void) {
    return ast_create_node(NODE_BREAK);
}

ASTNode* ast_create_continue(void) {
    return ast_create_node(NODE_CONTINUE);
}

/* Helper functions */

void ast_add_function(ASTNode* program, ASTNode* function) {
    if (program->type != NODE_PROGRAM) return;

    int new_count = program->data.program.function_count + 1;
    program->data.program.functions = (ASTNode**)safe_realloc(
        program->data.program.functions,
        sizeof(ASTNode*) * new_count
    );
    program->data.program.functions[new_count - 1] = function;
    program->data.program.function_count = new_count;
}

void ast_add_statement(ASTNode* block, ASTNode* statement) {
    if (block->type != NODE_BLOCK) return;

    int new_count = block->data.block.statement_count + 1;
    block->data.block.statements = (ASTNode**)safe_realloc(
        block->data.block.statements,
        sizeof(ASTNode*) * new_count
    );
    block->data.block.statements[new_count - 1] = statement;
    block->data.block.statement_count = new_count;
}

Parameter* parameter_create(const char* type, const char* name, int is_array) {
    Parameter* param = (Parameter*)safe_malloc(sizeof(Parameter));
    param->type = string_duplicate(type);
    param->name = string_duplicate(name);
    param->is_array = is_array;
    return param;
}

void parameter_destroy(Parameter* param) {
    if (param) {
        free(param->type);
        free(param->name);
        free(param);
    }
}

/* AST destruction */

void ast_destroy(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.function_count; i++) {
                ast_destroy(node->data.program.functions[i]);
            }
            free(node->data.program.functions);
            break;

        case NODE_FUNCTION:
            free(node->data.function.return_type);
            free(node->data.function.name);
            for (int i = 0; i < node->data.function.param_count; i++) {
                parameter_destroy(node->data.function.parameters[i]);
            }
            free(node->data.function.parameters);
            ast_destroy(node->data.function.body);
            break;

        case NODE_DECLARATION:
            free(node->data.declaration.data_type);
            free(node->data.declaration.name);
            ast_destroy(node->data.declaration.array_size);
            ast_destroy(node->data.declaration.initializer);
            break;

        case NODE_IF:
            ast_destroy(node->data.if_stmt.condition);
            ast_destroy(node->data.if_stmt.then_branch);
            ast_destroy(node->data.if_stmt.else_branch);
            break;

        case NODE_WHILE:
        case NODE_DO_WHILE:
            ast_destroy(node->data.while_stmt.condition);
            ast_destroy(node->data.while_stmt.body);
            break;

        case NODE_FOR:
            ast_destroy(node->data.for_stmt.init);
            ast_destroy(node->data.for_stmt.condition);
            ast_destroy(node->data.for_stmt.increment);
            ast_destroy(node->data.for_stmt.body);
            break;

        case NODE_RETURN:
            ast_destroy(node->data.return_stmt.value);
            break;

        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.statement_count; i++) {
                ast_destroy(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;

        case NODE_BINARY_OP:
            free(node->data.binary_op.operator);
            ast_destroy(node->data.binary_op.left);
            ast_destroy(node->data.binary_op.right);
            break;

        case NODE_UNARY_OP:
            free(node->data.unary_op.operator);
            ast_destroy(node->data.unary_op.operand);
            break;

        case NODE_FUNCTION_CALL:
            free(node->data.function_call.name);
            for (int i = 0; i < node->data.function_call.arg_count; i++) {
                ast_destroy(node->data.function_call.arguments[i]);
            }
            free(node->data.function_call.arguments);
            break;

        case NODE_ARRAY_ACCESS:
            free(node->data.array_access.name);
            ast_destroy(node->data.array_access.index);
            break;

        case NODE_ASSIGNMENT:
            ast_destroy(node->data.assignment.target);
            ast_destroy(node->data.assignment.value);
            break;

        case NODE_LITERAL:
            free(node->data.literal.value);
            free(node->data.literal.data_type);
            break;

        case NODE_IDENTIFIER:
            free(node->data.identifier.name);
            break;

        case NODE_BREAK:
        case NODE_CONTINUE:
            /* No data to free */
            break;

        default:
            break;
    }

    free(node);
}

/* Debug printing */

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void ast_print(ASTNode* node, int indent) {
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
        case NODE_PROGRAM:
            printf("PROGRAM (%d functions)\n", node->data.program.function_count);
            for (int i = 0; i < node->data.program.function_count; i++) {
                ast_print(node->data.program.functions[i], indent + 1);
            }
            break;

        case NODE_FUNCTION:
            printf("FUNCTION %s: %s\n", node->data.function.name, node->data.function.return_type);
            ast_print(node->data.function.body, indent + 1);
            break;

        case NODE_DECLARATION:
            printf("DECLARATION %s: %s\n", node->data.declaration.name, node->data.declaration.data_type);
            if (node->data.declaration.initializer) {
                ast_print(node->data.declaration.initializer, indent + 1);
            }
            break;

        case NODE_LITERAL:
            printf("LITERAL %s (%s)\n", node->data.literal.value, node->data.literal.data_type);
            break;

        case NODE_IDENTIFIER:
            printf("IDENTIFIER %s\n", node->data.identifier.name);
            break;

        case NODE_BINARY_OP:
            printf("BINARY_OP %s\n", node->data.binary_op.operator);
            ast_print(node->data.binary_op.left, indent + 1);
            ast_print(node->data.binary_op.right, indent + 1);
            break;

        default:
            printf("NODE (type %d)\n", node->type);
            break;
    }
}
