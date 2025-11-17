#include "semantic.h"

/* Forward declarations */
static void analyze_node(SemanticAnalyzer* analyzer, ASTNode* node);
static void analyze_function(SemanticAnalyzer* analyzer, ASTNode* node);
static void analyze_statement(SemanticAnalyzer* analyzer, ASTNode* node);
static void analyze_expression(SemanticAnalyzer* analyzer, ASTNode* node);

/* Semantic analyzer creation and destruction */

SemanticAnalyzer* semantic_analyzer_create(const char* filename) {
    SemanticAnalyzer* analyzer = (SemanticAnalyzer*)safe_malloc(sizeof(SemanticAnalyzer));
    analyzer->global_scope = symbol_table_create("global", NULL);
    analyzer->current_scope = analyzer->global_scope;
    analyzer->filename = filename;
    analyzer->had_error = 0;
    return analyzer;
}

void semantic_analyzer_destroy(SemanticAnalyzer* analyzer) {
    if (!analyzer) return;

    /* Destroy all symbol tables */
    SymbolTable* current = analyzer->global_scope;
    while (current) {
        SymbolTable* parent = current->parent;
        symbol_table_destroy(current);
        current = parent;
    }

    free(analyzer);
}

/* Error reporting */

static void semantic_error(SemanticAnalyzer* analyzer, int line, const char* message) {
    analyzer->had_error = 1;
    fprintf(stderr, "[SEMANTIC ERROR] %s:%d: %s\n", analyzer->filename, line, message);
}

/* Scope management */

static void enter_scope(SemanticAnalyzer* analyzer, const char* scope_name) {
    SymbolTable* new_scope = symbol_table_create(scope_name, analyzer->current_scope);
    analyzer->current_scope = new_scope;
}

static void exit_scope(SemanticAnalyzer* analyzer) {
    if (analyzer->current_scope && analyzer->current_scope->parent) {
        analyzer->current_scope = analyzer->current_scope->parent;
    }
}

/* Analysis functions */

static void analyze_expression(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_IDENTIFIER: {
            Symbol* symbol = symbol_table_lookup(analyzer->current_scope, node->data.identifier.name);
            if (!symbol) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Undeclared variable '%s'", node->data.identifier.name);
                semantic_error(analyzer, node->line, error_msg);
            }
            break;
        }

        case NODE_BINARY_OP:
            analyze_expression(analyzer, node->data.binary_op.left);
            analyze_expression(analyzer, node->data.binary_op.right);
            break;

        case NODE_UNARY_OP:
            analyze_expression(analyzer, node->data.unary_op.operand);
            break;

        case NODE_FUNCTION_CALL: {
            Symbol* symbol = symbol_table_lookup(analyzer->global_scope, node->data.function_call.name);
            if (!symbol) {
                /* Check if it's a standard library function */
                const char* std_funcs[] = {"printf", "scanf", "strlen", "strcpy", "malloc", "free", NULL};
                int is_std = 0;
                for (int i = 0; std_funcs[i] != NULL; i++) {
                    if (string_equals(node->data.function_call.name, std_funcs[i])) {
                        is_std = 1;
                        break;
                    }
                }

                if (!is_std) {
                    char error_msg[256];
                    snprintf(error_msg, sizeof(error_msg), "Undefined function '%s'", node->data.function_call.name);
                    semantic_error(analyzer, node->line, error_msg);
                }
            }

            /* Analyze arguments */
            for (int i = 0; i < node->data.function_call.arg_count; i++) {
                analyze_expression(analyzer, node->data.function_call.arguments[i]);
            }
            break;
        }

        case NODE_ARRAY_ACCESS: {
            Symbol* symbol = symbol_table_lookup(analyzer->current_scope, node->data.array_access.name);
            if (!symbol) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Undeclared array '%s'", node->data.array_access.name);
                semantic_error(analyzer, node->line, error_msg);
            } else if (!symbol->is_array) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "'%s' is not an array", node->data.array_access.name);
                semantic_error(analyzer, node->line, error_msg);
            }
            analyze_expression(analyzer, node->data.array_access.index);
            break;
        }

        case NODE_ASSIGNMENT:
            analyze_expression(analyzer, node->data.assignment.target);
            analyze_expression(analyzer, node->data.assignment.value);
            break;

        case NODE_LITERAL:
            /* Literals are always valid */
            break;

        default:
            break;
    }
}

static void analyze_statement(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_DECLARATION: {
            /* Check if variable already declared in current scope */
            Symbol* existing = symbol_table_lookup_local(analyzer->current_scope, node->data.declaration.name);
            if (existing) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Variable '%s' already declared in this scope",
                        node->data.declaration.name);
                semantic_error(analyzer, node->line, error_msg);
            } else {
                Symbol* symbol = symbol_create(
                    node->data.declaration.name,
                    node->data.declaration.data_type,
                    analyzer->current_scope->scope_name,
                    node->line
                );
                symbol->is_array = node->data.declaration.is_array;
                symbol_table_insert(analyzer->current_scope, symbol);
            }

            /* Analyze initializer */
            if (node->data.declaration.initializer) {
                analyze_expression(analyzer, node->data.declaration.initializer);
            }
            if (node->data.declaration.array_size) {
                analyze_expression(analyzer, node->data.declaration.array_size);
            }
            break;
        }

        case NODE_IF:
            analyze_expression(analyzer, node->data.if_stmt.condition);
            analyze_statement(analyzer, node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch) {
                analyze_statement(analyzer, node->data.if_stmt.else_branch);
            }
            break;

        case NODE_WHILE:
            analyze_expression(analyzer, node->data.while_stmt.condition);
            analyze_statement(analyzer, node->data.while_stmt.body);
            break;

        case NODE_FOR:
            if (node->data.for_stmt.init) {
                analyze_statement(analyzer, node->data.for_stmt.init);
            }
            if (node->data.for_stmt.condition) {
                analyze_expression(analyzer, node->data.for_stmt.condition);
            }
            if (node->data.for_stmt.increment) {
                analyze_expression(analyzer, node->data.for_stmt.increment);
            }
            analyze_statement(analyzer, node->data.for_stmt.body);
            break;

        case NODE_RETURN:
            if (node->data.return_stmt.value) {
                analyze_expression(analyzer, node->data.return_stmt.value);
            }
            break;

        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.statement_count; i++) {
                analyze_statement(analyzer, node->data.block.statements[i]);
            }
            break;

        case NODE_BREAK:
        case NODE_CONTINUE:
            /* These are always valid in loops */
            break;

        default:
            /* Expression statements */
            analyze_expression(analyzer, node);
            break;
    }
}

static void analyze_function(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != NODE_FUNCTION) return;

    /* Check if function already declared */
    Symbol* existing = symbol_table_lookup_local(analyzer->global_scope, node->data.function.name);
    if (existing) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Function '%s' already declared", node->data.function.name);
        semantic_error(analyzer, node->line, error_msg);
        return;
    }

    /* Add function to global scope */
    Symbol* func_symbol = symbol_create(
        node->data.function.name,
        node->data.function.return_type,
        "global",
        node->line
    );
    func_symbol->is_function = 1;
    symbol_table_insert(analyzer->global_scope, func_symbol);

    /* Enter function scope */
    enter_scope(analyzer, node->data.function.name);

    /* Add parameters to function scope */
    for (int i = 0; i < node->data.function.param_count; i++) {
        Parameter* param = node->data.function.parameters[i];
        Symbol* param_symbol = symbol_create(param->name, param->type, node->data.function.name, node->line);
        param_symbol->is_array = param->is_array;
        symbol_table_insert(analyzer->current_scope, param_symbol);
    }

    /* Analyze function body */
    analyze_statement(analyzer, node->data.function.body);

    /* Exit function scope */
    exit_scope(analyzer);
}

static void analyze_node(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.function_count; i++) {
                analyze_function(analyzer, node->data.program.functions[i]);
            }
            break;

        default:
            break;
    }
}

/* Main analysis function */

int analyze_semantics(ASTNode* program, const char* filename) {
    if (!program) return 0;

    SemanticAnalyzer* analyzer = semantic_analyzer_create(filename);
    analyze_node(analyzer, program);

    int success = !analyzer->had_error;
    semantic_analyzer_destroy(analyzer);

    return success;
}
