#include "translator.h"
#include "formatter.h"

/* Forward declarations */
static void translate_function(TranslationContext* ctx, ASTNode* node);
static void translate_statement(TranslationContext* ctx, ASTNode* node, int step_number);
static char* translate_expression(ASTNode* node);

/* Helper functions */

static void append_output(TranslationContext* ctx, const char* text) {
    if (!text) return;

    size_t text_len = strlen(text);
    size_t required = ctx->output_size + text_len + 1;

    if (required > ctx->output_capacity) {
        ctx->output_capacity = required * 2;
        ctx->output = (char*)safe_realloc(ctx->output, ctx->output_capacity);
    }

    strcat(ctx->output, text);
    ctx->output_size += text_len;
}

static void append_line(TranslationContext* ctx, const char* text) {
    /* Add indentation */
    for (int i = 0; i < ctx->indent_level; i++) {
        append_output(ctx, "  ");
    }
    append_output(ctx, text);
    append_output(ctx, "\n");
}

static void append_formatted(TranslationContext* ctx, const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    append_output(ctx, buffer);
}

/* Expression translation */

static char* translate_binary_operator(const char* op, ASTNode* left, ASTNode* right) {
    char* left_str = translate_expression(left);
    char* right_str = translate_expression(right);
    char* result = (char*)safe_malloc(1024);

    if (string_equals(op, "+")) {
        snprintf(result, 1024, "the sum of %s and %s", left_str, right_str);
    } else if (string_equals(op, "-")) {
        snprintf(result, 1024, "the difference between %s and %s", left_str, right_str);
    } else if (string_equals(op, "*")) {
        snprintf(result, 1024, "the product of %s and %s", left_str, right_str);
    } else if (string_equals(op, "/")) {
        snprintf(result, 1024, "%s divided by %s", left_str, right_str);
    } else if (string_equals(op, "%")) {
        snprintf(result, 1024, "the remainder when %s is divided by %s", left_str, right_str);
    } else if (string_equals(op, "==")) {
        snprintf(result, 1024, "%s is equal to %s", left_str, right_str);
    } else if (string_equals(op, "!=")) {
        snprintf(result, 1024, "%s is not equal to %s", left_str, right_str);
    } else if (string_equals(op, "<")) {
        snprintf(result, 1024, "%s is less than %s", left_str, right_str);
    } else if (string_equals(op, "<=")) {
        snprintf(result, 1024, "%s is less than or equal to %s", left_str, right_str);
    } else if (string_equals(op, ">")) {
        snprintf(result, 1024, "%s is greater than %s", left_str, right_str);
    } else if (string_equals(op, ">=")) {
        snprintf(result, 1024, "%s is greater than or equal to %s", left_str, right_str);
    } else if (string_equals(op, "&&")) {
        snprintf(result, 1024, "both %s and %s", left_str, right_str);
    } else if (string_equals(op, "||")) {
        snprintf(result, 1024, "either %s or %s", left_str, right_str);
    } else {
        snprintf(result, 1024, "%s %s %s", left_str, op, right_str);
    }

    free(left_str);
    free(right_str);
    return result;
}

static char* translate_unary_operator(const char* op, ASTNode* operand) {
    char* operand_str = translate_expression(operand);
    char* result = (char*)safe_malloc(512);

    if (string_equals(op, "!")) {
        snprintf(result, 512, "not %s", operand_str);
    } else if (string_equals(op, "-")) {
        snprintf(result, 512, "negative %s", operand_str);
    } else if (string_equals(op, "+")) {
        snprintf(result, 512, "%s", operand_str);
    } else if (string_equals(op, "++")) {
        snprintf(result, 512, "%s incremented by 1", operand_str);
    } else if (string_equals(op, "--")) {
        snprintf(result, 512, "%s decremented by 1", operand_str);
    } else if (string_equals(op, "&")) {
        snprintf(result, 512, "the address of %s", operand_str);
    } else if (string_equals(op, "*")) {
        snprintf(result, 512, "the value stored at the memory location referenced by %s", operand_str);
    } else {
        snprintf(result, 512, "%s %s", op, operand_str);
    }

    free(operand_str);
    return result;
}

static char* translate_function_call(ASTNode* node) {
    char* result = (char*)safe_malloc(2048);
    const char* func_name = node->data.function_call.name;

    /* Handle standard library functions with special descriptions */
    if (string_equals(func_name, "printf")) {
        if (node->data.function_call.arg_count > 0) {
            ASTNode* format_arg = node->data.function_call.arguments[0];
            if (format_arg->type == NODE_LITERAL) {
                snprintf(result, 2048, "display the message %s", format_arg->data.literal.value);
            } else {
                snprintf(result, 2048, "display formatted output to the user");
            }
        } else {
            snprintf(result, 2048, "display output to the user");
        }
    } else if (string_equals(func_name, "scanf")) {
        snprintf(result, 2048, "read input from the user");
    } else if (string_equals(func_name, "strlen")) {
        if (node->data.function_call.arg_count > 0) {
            char* arg_str = translate_expression(node->data.function_call.arguments[0]);
            snprintf(result, 2048, "determine the length of the text stored in %s", arg_str);
            free(arg_str);
        } else {
            snprintf(result, 2048, "determine the length of a text string");
        }
    } else if (string_equals(func_name, "strcpy")) {
        snprintf(result, 2048, "copy one text string to another");
    } else if (string_equals(func_name, "malloc")) {
        snprintf(result, 2048, "allocate memory dynamically");
    } else if (string_equals(func_name, "free")) {
        snprintf(result, 2048, "release previously allocated memory");
    } else {
        /* Generic function call */
        if (node->data.function_call.arg_count > 0) {
            char args_str[1024] = "";
            for (int i = 0; i < node->data.function_call.arg_count; i++) {
                char* arg = translate_expression(node->data.function_call.arguments[i]);
                if (i > 0) strcat(args_str, ", ");
                strcat(args_str, arg);
                free(arg);
            }
            snprintf(result, 2048, "call the '%s' function with arguments %s", func_name, args_str);
        } else {
            snprintf(result, 2048, "call the '%s' function", func_name);
        }
    }

    return result;
}

static char* translate_expression(ASTNode* node) {
    if (!node) return string_duplicate("nothing");

    char* result = (char*)safe_malloc(1024);

    switch (node->type) {
        case NODE_LITERAL:
            if (string_equals(node->data.literal.data_type, "number")) {
                snprintf(result, 1024, "the value %s", node->data.literal.value);
            } else if (string_equals(node->data.literal.data_type, "string")) {
                snprintf(result, 1024, "%s", node->data.literal.value);
            } else if (string_equals(node->data.literal.data_type, "char")) {
                snprintf(result, 1024, "the character %s", node->data.literal.value);
            } else {
                snprintf(result, 1024, "%s", node->data.literal.value);
            }
            break;

        case NODE_IDENTIFIER:
            snprintf(result, 1024, "'%s'", node->data.identifier.name);
            break;

        case NODE_BINARY_OP:
            free(result);
            return translate_binary_operator(node->data.binary_op.operator,
                                            node->data.binary_op.left,
                                            node->data.binary_op.right);

        case NODE_UNARY_OP:
            free(result);
            return translate_unary_operator(node->data.unary_op.operator,
                                           node->data.unary_op.operand);

        case NODE_FUNCTION_CALL:
            free(result);
            return translate_function_call(node);

        case NODE_ARRAY_ACCESS: {
            char* index_str = translate_expression(node->data.array_access.index);
            snprintf(result, 1024, "the element at position %s in the array '%s'",
                    index_str, node->data.array_access.name);
            free(index_str);
            break;
        }

        case NODE_ASSIGNMENT: {
            char* target_str = translate_expression(node->data.assignment.target);
            char* value_str = translate_expression(node->data.assignment.value);
            snprintf(result, 1024, "set %s to %s", target_str, value_str);
            free(target_str);
            free(value_str);
            break;
        }

        default:
            snprintf(result, 1024, "an expression");
            break;
    }

    return result;
}

/* Statement translation */

static void translate_statement(TranslationContext* ctx, ASTNode* node, int step_number) {
    if (!node) return;

    char step_prefix[32] = "";
    if (step_number > 0) {
        snprintf(step_prefix, sizeof(step_prefix), "%d. ", step_number);
    }

    switch (node->type) {
        case NODE_DECLARATION: {
            char line[1024];
            if (node->data.declaration.is_array) {
                char* size_str = translate_expression(node->data.declaration.array_size);
                snprintf(line, sizeof(line), "%sDeclare an array named '%s' of type %s with %s elements.",
                        step_prefix, node->data.declaration.name,
                        node->data.declaration.data_type, size_str);
                free(size_str);
            } else if (node->data.declaration.initializer) {
                char* init_str = translate_expression(node->data.declaration.initializer);
                snprintf(line, sizeof(line), "%sDeclare a variable named '%s' of type %s, initialised to %s.",
                        step_prefix, node->data.declaration.name,
                        node->data.declaration.data_type, init_str);
                free(init_str);
            } else {
                snprintf(line, sizeof(line), "%sDeclare a variable named '%s' of type %s.",
                        step_prefix, node->data.declaration.name,
                        node->data.declaration.data_type);
            }
            append_line(ctx, line);
            append_line(ctx, "");
            break;
        }

        case NODE_IF: {
            char* cond_str = translate_expression(node->data.if_stmt.condition);
            char line[1024];
            snprintf(line, sizeof(line), "%sIf the condition \"%s\" is true, then:", step_prefix, cond_str);
            append_line(ctx, line);
            free(cond_str);

            ctx->indent_level++;
            if (node->data.if_stmt.then_branch->type == NODE_BLOCK) {
                for (int i = 0; i < node->data.if_stmt.then_branch->data.block.statement_count; i++) {
                    translate_statement(ctx, node->data.if_stmt.then_branch->data.block.statements[i], 0);
                }
            } else {
                translate_statement(ctx, node->data.if_stmt.then_branch, 0);
            }
            ctx->indent_level--;

            if (node->data.if_stmt.else_branch) {
                append_line(ctx, "  Otherwise:");
                ctx->indent_level++;
                if (node->data.if_stmt.else_branch->type == NODE_BLOCK) {
                    for (int i = 0; i < node->data.if_stmt.else_branch->data.block.statement_count; i++) {
                        translate_statement(ctx, node->data.if_stmt.else_branch->data.block.statements[i], 0);
                    }
                } else {
                    translate_statement(ctx, node->data.if_stmt.else_branch, 0);
                }
                ctx->indent_level--;
            }
            append_line(ctx, "");
            break;
        }

        case NODE_WHILE: {
            char* cond_str = translate_expression(node->data.while_stmt.condition);
            char line[1024];
            snprintf(line, sizeof(line), "%sWhilst the condition \"%s\" remains true, repeatedly perform the following:",
                    step_prefix, cond_str);
            append_line(ctx, line);
            free(cond_str);

            ctx->indent_level++;
            if (node->data.while_stmt.body->type == NODE_BLOCK) {
                for (int i = 0; i < node->data.while_stmt.body->data.block.statement_count; i++) {
                    translate_statement(ctx, node->data.while_stmt.body->data.block.statements[i], 0);
                }
            } else {
                translate_statement(ctx, node->data.while_stmt.body, 0);
            }
            ctx->indent_level--;
            append_line(ctx, "");
            break;
        }

        case NODE_FOR: {
            char line[2048];
            char* init_str = node->data.for_stmt.init ? translate_expression(node->data.for_stmt.init) : string_duplicate("nothing");
            char* cond_str = node->data.for_stmt.condition ? translate_expression(node->data.for_stmt.condition) : string_duplicate("true");
            char* inc_str = node->data.for_stmt.increment ? translate_expression(node->data.for_stmt.increment) : string_duplicate("nothing");

            snprintf(line, sizeof(line),
                    "%sBeginning with %s, and continuing whilst the condition \"%s\" holds, "
                    "repeatedly perform the following operations, and after each iteration %s:",
                    step_prefix, init_str, cond_str, inc_str);
            append_line(ctx, line);

            free(init_str);
            free(cond_str);
            free(inc_str);

            ctx->indent_level++;
            if (node->data.for_stmt.body->type == NODE_BLOCK) {
                for (int i = 0; i < node->data.for_stmt.body->data.block.statement_count; i++) {
                    translate_statement(ctx, node->data.for_stmt.body->data.block.statements[i], 0);
                }
            } else {
                translate_statement(ctx, node->data.for_stmt.body, 0);
            }
            ctx->indent_level--;
            append_line(ctx, "");
            break;
        }

        case NODE_RETURN: {
            char line[1024];
            if (node->data.return_stmt.value) {
                char* value_str = translate_expression(node->data.return_stmt.value);
                snprintf(line, sizeof(line), "%sReturn %s.", step_prefix, value_str);
                free(value_str);
            } else {
                snprintf(line, sizeof(line), "%sReturn (void).", step_prefix);
            }
            append_line(ctx, line);
            append_line(ctx, "");
            break;
        }

        case NODE_BREAK:
            append_line(ctx, "Exit the loop immediately.");
            append_line(ctx, "");
            break;

        case NODE_CONTINUE:
            append_line(ctx, "Skip to the next iteration of the loop.");
            append_line(ctx, "");
            break;

        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.statement_count; i++) {
                translate_statement(ctx, node->data.block.statements[i], step_number > 0 ? i + 1 : 0);
            }
            break;

        default:
            /* Expression statement */
            {
                char* expr_str = translate_expression(node);
                char line[1024];
                snprintf(line, sizeof(line), "%s%s.", step_prefix, expr_str);
                /* Capitalize first letter */
                if (line[0] >= 'a' && line[0] <= 'z') {
                    line[0] = line[0] - 'a' + 'A';
                }
                append_line(ctx, line);
                append_line(ctx, "");
                free(expr_str);
            }
            break;
    }
}

/* Function translation */

static void translate_function(TranslationContext* ctx, ASTNode* node) {
    if (node->type != NODE_FUNCTION) return;

    /* Function header */
    char header[256];
    snprintf(header, sizeof(header), "Function: %s", node->data.function.name);
    append_line(ctx, header);

    /* Underline */
    for (size_t i = 0; i < strlen(header); i++) {
        append_output(ctx, "-");
    }
    append_output(ctx, "\n");

    /* Function description */
    char desc[512];
    if (node->data.function.param_count == 0) {
        snprintf(desc, sizeof(desc), "This function accepts no parameters and returns a value of type %s.",
                node->data.function.return_type);
    } else if (node->data.function.param_count == 1) {
        Parameter* param = node->data.function.parameters[0];
        snprintf(desc, sizeof(desc),
                "This function accepts one parameter named '%s' of type %s%s, and returns a value of type %s.",
                param->name, param->type, param->is_array ? " (array)" : "",
                node->data.function.return_type);
    } else {
        snprintf(desc, sizeof(desc), "This function accepts %d parameters and returns a value of type %s.",
                node->data.function.param_count, node->data.function.return_type);
    }
    append_line(ctx, desc);
    append_line(ctx, "");

    /* Parameter list if multiple */
    if (node->data.function.param_count > 1) {
        append_line(ctx, "Parameters:");
        for (int i = 0; i < node->data.function.param_count; i++) {
            Parameter* param = node->data.function.parameters[i];
            char param_line[256];
            snprintf(param_line, sizeof(param_line), "  • '%s': %s%s",
                    param->name, param->type, param->is_array ? " (array)" : "");
            append_line(ctx, param_line);
        }
        append_line(ctx, "");
    }

    /* Function body */
    if (string_equals(node->data.function.name, "main")) {
        append_line(ctx, "This is the main entry point of the programme.");
        append_line(ctx, "");
    }

    append_line(ctx, "The function performs the following steps:");
    append_line(ctx, "");

    if (node->data.function.body && node->data.function.body->type == NODE_BLOCK) {
        ctx->indent_level = 1;
        for (int i = 0; i < node->data.function.body->data.block.statement_count; i++) {
            translate_statement(ctx, node->data.function.body->data.block.statements[i], i + 1);
        }
        ctx->indent_level = 0;
    }

    append_line(ctx, "");
}

/* Main translation function */

char* translate_to_english(ASTNode* program) {
    if (!program || program->type != NODE_PROGRAM) {
        return string_duplicate("Error: Invalid programme structure.\n");
    }

    TranslationContext ctx;
    ctx.output_capacity = 4096;
    ctx.output_size = 0;
    ctx.output = (char*)safe_malloc(ctx.output_capacity);
    ctx.output[0] = '\0';
    ctx.indent_level = 0;

    /* Programme header */
    append_line(&ctx, "Programme Description");
    append_line(&ctx, "=====================");
    append_line(&ctx, "");

    /* Count functions */
    int func_count = program->data.program.function_count;
    if (func_count == 1) {
        append_line(&ctx, "This programme consists of one function.");
    } else {
        char line[256];
        snprintf(line, sizeof(line), "This programme consists of %d functions.", func_count);
        append_line(&ctx, line);
    }
    append_line(&ctx, "");

    /* Translate each function */
    for (int i = 0; i < func_count; i++) {
        translate_function(&ctx, program->data.program.functions[i]);
    }

    return ctx.output;
}
