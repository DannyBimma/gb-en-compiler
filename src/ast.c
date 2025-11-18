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

ASTNode* ast_create_do_while(ASTNode* body, ASTNode* condition) {
    ASTNode* node = ast_create_node(NODE_DO_WHILE);
    node->data.while_stmt.body = body;
    node->data.while_stmt.condition = condition;
    return node;
}

ASTNode* ast_create_struct_def(const char* name, int is_union) {
    ASTNode* node = ast_create_node(NODE_STRUCT_DEF);
    node->data.struct_def.name = name ? string_duplicate(name) : NULL;
    node->data.struct_def.is_union = is_union;
    node->data.struct_def.members = NULL;
    node->data.struct_def.member_count = 0;
    return node;
}

ASTNode* ast_create_member_access(ASTNode* object, const char* member, int is_arrow) {
    ASTNode* node = ast_create_node(NODE_MEMBER_ACCESS);
    node->data.member_access.object = object;
    node->data.member_access.member = string_duplicate(member);
    node->data.member_access.is_arrow = is_arrow;
    return node;
}

ASTNode* ast_create_switch(ASTNode* expression) {
    ASTNode* node = ast_create_node(NODE_SWITCH);
    node->data.switch_stmt.expression = expression;
    node->data.switch_stmt.cases = NULL;
    node->data.switch_stmt.case_count = 0;
    return node;
}

ASTNode* ast_create_case(ASTNode* value) {
    ASTNode* node = ast_create_node(NODE_CASE);
    node->data.case_stmt.value = value;
    node->data.case_stmt.statements = NULL;
    node->data.case_stmt.statement_count = 0;
    return node;
}

ASTNode* ast_create_default(void) {
    ASTNode* node = ast_create_node(NODE_DEFAULT);
    node->data.case_stmt.value = NULL;
    node->data.case_stmt.statements = NULL;
    node->data.case_stmt.statement_count = 0;
    return node;
}

ASTNode* ast_create_ternary(ASTNode* condition, ASTNode* then_expr, ASTNode* else_expr) {
    ASTNode* node = ast_create_node(NODE_TERNARY);
    node->data.ternary.condition = condition;
    node->data.ternary.then_expr = then_expr;
    node->data.ternary.else_expr = else_expr;
    return node;
}

ASTNode* ast_create_enum_def(const char* name) {
    ASTNode* node = ast_create_node(NODE_ENUM_DEF);
    node->data.enum_def.name = name ? string_duplicate(name) : NULL;
    node->data.enum_def.values = NULL;
    node->data.enum_def.value_count = 0;
    return node;
}

ASTNode* ast_create_sizeof_type(const char* type_name) {
    ASTNode* node = ast_create_node(NODE_SIZEOF);
    node->data.sizeof_expr.type_name = string_duplicate(type_name);
    node->data.sizeof_expr.expression = NULL;
    return node;
}

ASTNode* ast_create_sizeof_expr(ASTNode* expression) {
    ASTNode* node = ast_create_node(NODE_SIZEOF);
    node->data.sizeof_expr.type_name = NULL;
    node->data.sizeof_expr.expression = expression;
    return node;
}

ASTNode* ast_create_cast(const char* target_type, ASTNode* expression) {
    ASTNode* node = ast_create_node(NODE_CAST);
    node->data.cast.target_type = string_duplicate(target_type);
    node->data.cast.expression = expression;
    return node;
}

ASTNode* ast_create_compound_assign(const char* op, ASTNode* target, ASTNode* value) {
    ASTNode* node = ast_create_node(NODE_COMPOUND_ASSIGN);
    node->data.compound_assign.operator = string_duplicate(op);
    node->data.compound_assign.target = target;
    node->data.compound_assign.value = value;
    return node;
}

ASTNode* ast_create_goto(const char* label) {
    ASTNode* node = ast_create_node(NODE_GOTO);
    node->data.goto_stmt.label = string_duplicate(label);
    return node;
}

ASTNode* ast_create_label(const char* name, ASTNode* statement) {
    ASTNode* node = ast_create_node(NODE_LABEL);
    node->data.label_stmt.name = string_duplicate(name);
    node->data.label_stmt.statement = statement;
    return node;
}

ASTNode* ast_create_typedef(const char* original_type, const char* new_name) {
    ASTNode* node = ast_create_node(NODE_TYPEDEF);
    node->data.typedef_stmt.original_type = string_duplicate(original_type);
    node->data.typedef_stmt.new_name = string_duplicate(new_name);
    return node;
}

/* Helper functions */

void ast_add_struct_member(ASTNode* struct_def, ASTNode* member) {
    if (struct_def->type != NODE_STRUCT_DEF) return;
    int new_count = struct_def->data.struct_def.member_count + 1;
    struct_def->data.struct_def.members = (ASTNode**)safe_realloc(
        struct_def->data.struct_def.members,
        sizeof(ASTNode*) * new_count
    );
    struct_def->data.struct_def.members[new_count - 1] = member;
    struct_def->data.struct_def.member_count = new_count;
}

void ast_add_case(ASTNode* switch_stmt, ASTNode* case_node) {
    if (switch_stmt->type != NODE_SWITCH) return;
    int new_count = switch_stmt->data.switch_stmt.case_count + 1;
    switch_stmt->data.switch_stmt.cases = (ASTNode**)safe_realloc(
        switch_stmt->data.switch_stmt.cases,
        sizeof(ASTNode*) * new_count
    );
    switch_stmt->data.switch_stmt.cases[new_count - 1] = case_node;
    switch_stmt->data.switch_stmt.case_count = new_count;
}

void ast_add_case_statement(ASTNode* case_node, ASTNode* statement) {
    if (case_node->type != NODE_CASE && case_node->type != NODE_DEFAULT) return;
    int new_count = case_node->data.case_stmt.statement_count + 1;
    case_node->data.case_stmt.statements = (ASTNode**)safe_realloc(
        case_node->data.case_stmt.statements,
        sizeof(ASTNode*) * new_count
    );
    case_node->data.case_stmt.statements[new_count - 1] = statement;
    case_node->data.case_stmt.statement_count = new_count;
}

void ast_add_enum_value(ASTNode* enum_def, const char* value) {
    if (enum_def->type != NODE_ENUM_DEF) return;
    int new_count = enum_def->data.enum_def.value_count + 1;
    enum_def->data.enum_def.values = (char**)safe_realloc(
        enum_def->data.enum_def.values,
        sizeof(char*) * new_count
    );
    enum_def->data.enum_def.values[new_count - 1] = string_duplicate(value);
    enum_def->data.enum_def.value_count = new_count;
}

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

        case NODE_STRUCT_DEF:
            free(node->data.struct_def.name);
            for (int i = 0; i < node->data.struct_def.member_count; i++) {
                ast_destroy(node->data.struct_def.members[i]);
            }
            free(node->data.struct_def.members);
            break;

        case NODE_MEMBER_ACCESS:
            ast_destroy(node->data.member_access.object);
            free(node->data.member_access.member);
            break;

        case NODE_SWITCH:
            ast_destroy(node->data.switch_stmt.expression);
            for (int i = 0; i < node->data.switch_stmt.case_count; i++) {
                ast_destroy(node->data.switch_stmt.cases[i]);
            }
            free(node->data.switch_stmt.cases);
            break;

        case NODE_CASE:
        case NODE_DEFAULT:
            ast_destroy(node->data.case_stmt.value);
            for (int i = 0; i < node->data.case_stmt.statement_count; i++) {
                ast_destroy(node->data.case_stmt.statements[i]);
            }
            free(node->data.case_stmt.statements);
            break;

        case NODE_TERNARY:
            ast_destroy(node->data.ternary.condition);
            ast_destroy(node->data.ternary.then_expr);
            ast_destroy(node->data.ternary.else_expr);
            break;

        case NODE_ENUM_DEF:
            free(node->data.enum_def.name);
            for (int i = 0; i < node->data.enum_def.value_count; i++) {
                free(node->data.enum_def.values[i]);
            }
            free(node->data.enum_def.values);
            break;

        case NODE_SIZEOF:
            free(node->data.sizeof_expr.type_name);
            ast_destroy(node->data.sizeof_expr.expression);
            break;

        case NODE_CAST:
            free(node->data.cast.target_type);
            ast_destroy(node->data.cast.expression);
            break;

        case NODE_COMPOUND_ASSIGN:
            free(node->data.compound_assign.operator);
            ast_destroy(node->data.compound_assign.target);
            ast_destroy(node->data.compound_assign.value);
            break;

        case NODE_GOTO:
            free(node->data.goto_stmt.label);
            break;

        case NODE_LABEL:
            free(node->data.label_stmt.name);
            ast_destroy(node->data.label_stmt.statement);
            break;

        case NODE_TYPEDEF:
            free(node->data.typedef_stmt.original_type);
            free(node->data.typedef_stmt.new_name);
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
