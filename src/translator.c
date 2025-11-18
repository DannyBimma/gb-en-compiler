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
    } else if (string_equals(op, "&")) {
        snprintf(result, 1024, "the bitwise AND of %s and %s", left_str, right_str);
    } else if (string_equals(op, "|")) {
        snprintf(result, 1024, "the bitwise OR of %s and %s", left_str, right_str);
    } else if (string_equals(op, "^")) {
        snprintf(result, 1024, "the bitwise XOR of %s and %s", left_str, right_str);
    } else if (string_equals(op, "<<")) {
        snprintf(result, 1024, "%s left-shifted by %s bits", left_str, right_str);
    } else if (string_equals(op, ">>")) {
        snprintf(result, 1024, "%s right-shifted by %s bits", left_str, right_str);
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
    } else if (string_equals(op, "++post")) {
        snprintf(result, 512, "increment %s by 1", operand_str);
    } else if (string_equals(op, "--post")) {
        snprintf(result, 512, "decrement %s by 1", operand_str);
    } else if (string_equals(op, "~")) {
        snprintf(result, 512, "the bitwise complement of %s", operand_str);
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
    } else if (string_equals(func_name, "strcmp")) {
        snprintf(result, 2048, "compare two text strings");
    } else if (string_equals(func_name, "strncmp")) {
        snprintf(result, 2048, "compare a specified number of characters in two text strings");
    } else if (string_equals(func_name, "strcat")) {
        snprintf(result, 2048, "concatenate two text strings");
    } else if (string_equals(func_name, "strncpy")) {
        snprintf(result, 2048, "copy a specified number of characters from one text string to another");
    } else if (string_equals(func_name, "sprintf")) {
        snprintf(result, 2048, "format text and store it in a string");
    } else if (string_equals(func_name, "fprintf")) {
        snprintf(result, 2048, "write formatted output to a file");
    } else if (string_equals(func_name, "fscanf")) {
        snprintf(result, 2048, "read formatted input from a file");
    } else if (string_equals(func_name, "fopen")) {
        snprintf(result, 2048, "open a file");
    } else if (string_equals(func_name, "fclose")) {
        snprintf(result, 2048, "close an open file");
    } else if (string_equals(func_name, "fread")) {
        snprintf(result, 2048, "read data from a file");
    } else if (string_equals(func_name, "fwrite")) {
        snprintf(result, 2048, "write data to a file");
    } else if (string_equals(func_name, "fgets")) {
        snprintf(result, 2048, "read a line of text from a file");
    } else if (string_equals(func_name, "fputs")) {
        snprintf(result, 2048, "write a line of text to a file");
    } else if (string_equals(func_name, "feof")) {
        snprintf(result, 2048, "check if end of file has been reached");
    } else if (string_equals(func_name, "fseek")) {
        snprintf(result, 2048, "move the file position indicator");
    } else if (string_equals(func_name, "ftell")) {
        snprintf(result, 2048, "get the current file position");
    } else if (string_equals(func_name, "rewind")) {
        snprintf(result, 2048, "reset the file position to the beginning");
    } else if (string_equals(func_name, "calloc")) {
        snprintf(result, 2048, "allocate and initialise memory to zero");
    } else if (string_equals(func_name, "realloc")) {
        snprintf(result, 2048, "resize previously allocated memory");
    } else if (string_equals(func_name, "memcpy")) {
        snprintf(result, 2048, "copy a block of memory");
    } else if (string_equals(func_name, "memset")) {
        snprintf(result, 2048, "fill a block of memory with a specified value");
    } else if (string_equals(func_name, "memcmp")) {
        snprintf(result, 2048, "compare two blocks of memory");
    } else if (string_equals(func_name, "atoi")) {
        snprintf(result, 2048, "convert text to an integer");
    } else if (string_equals(func_name, "atof")) {
        snprintf(result, 2048, "convert text to a floating-point number");
    } else if (string_equals(func_name, "atol")) {
        snprintf(result, 2048, "convert text to a long integer");
    } else if (string_equals(func_name, "itoa")) {
        snprintf(result, 2048, "convert an integer to text");
    } else if (string_equals(func_name, "abs")) {
        snprintf(result, 2048, "calculate the absolute value");
    } else if (string_equals(func_name, "sqrt")) {
        snprintf(result, 2048, "calculate the square root");
    } else if (string_equals(func_name, "pow")) {
        snprintf(result, 2048, "raise a number to a power");
    } else if (string_equals(func_name, "sin")) {
        snprintf(result, 2048, "calculate the sine");
    } else if (string_equals(func_name, "cos")) {
        snprintf(result, 2048, "calculate the cosine");
    } else if (string_equals(func_name, "tan")) {
        snprintf(result, 2048, "calculate the tangent");
    } else if (string_equals(func_name, "log")) {
        snprintf(result, 2048, "calculate the natural logarithm");
    } else if (string_equals(func_name, "exp")) {
        snprintf(result, 2048, "calculate the exponential");
    } else if (string_equals(func_name, "ceil")) {
        snprintf(result, 2048, "round up to the nearest integer");
    } else if (string_equals(func_name, "floor")) {
        snprintf(result, 2048, "round down to the nearest integer");
    } else if (string_equals(func_name, "rand")) {
        snprintf(result, 2048, "generate a pseudo-random number");
    } else if (string_equals(func_name, "srand")) {
        snprintf(result, 2048, "seed the random number generator");
    } else if (string_equals(func_name, "time")) {
        snprintf(result, 2048, "get the current time");
    } else if (string_equals(func_name, "exit")) {
        snprintf(result, 2048, "terminate the programme");
    } else if (string_equals(func_name, "assert")) {
        snprintf(result, 2048, "verify a condition and abort if false");
    } else if (string_equals(func_name, "getchar")) {
        snprintf(result, 2048, "read a character from standard input");
    } else if (string_equals(func_name, "putchar")) {
        snprintf(result, 2048, "write a character to standard output");
    } else if (string_equals(func_name, "puts")) {
        snprintf(result, 2048, "write a string to standard output");
    } else if (string_equals(func_name, "gets")) {
        snprintf(result, 2048, "read a string from standard input");
    } else if (string_equals(func_name, "isalpha")) {
        snprintf(result, 2048, "check if a character is alphabetic");
    } else if (string_equals(func_name, "isdigit")) {
        snprintf(result, 2048, "check if a character is a digit");
    } else if (string_equals(func_name, "isspace")) {
        snprintf(result, 2048, "check if a character is whitespace");
    } else if (string_equals(func_name, "toupper")) {
        snprintf(result, 2048, "convert a character to uppercase");
    } else if (string_equals(func_name, "tolower")) {
        snprintf(result, 2048, "convert a character to lowercase");
    } else if (string_equals(func_name, "qsort")) {
        snprintf(result, 2048, "sort an array using quicksort");
    } else if (string_equals(func_name, "bsearch")) {
        snprintf(result, 2048, "search a sorted array using binary search");
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

        case NODE_MEMBER_ACCESS: {
            char* obj_str = translate_expression(node->data.member_access.object);
            if (node->data.member_access.is_arrow) {
                snprintf(result, 1024, "the '%s' member of the structure pointed to by %s",
                        node->data.member_access.member, obj_str);
            } else {
                snprintf(result, 1024, "the '%s' member of %s",
                        node->data.member_access.member, obj_str);
            }
            free(obj_str);
            break;
        }

        case NODE_TERNARY: {
            char* cond_str = translate_expression(node->data.ternary.condition);
            char* then_str = translate_expression(node->data.ternary.then_expr);
            char* else_str = translate_expression(node->data.ternary.else_expr);
            snprintf(result, 1024, "if %s then %s, otherwise %s", cond_str, then_str, else_str);
            free(cond_str);
            free(then_str);
            free(else_str);
            break;
        }

        case NODE_SIZEOF: {
            if (node->data.sizeof_expr.type_name) {
                snprintf(result, 1024, "the size in bytes of type '%s'", node->data.sizeof_expr.type_name);
            } else {
                char* expr_str = translate_expression(node->data.sizeof_expr.expression);
                snprintf(result, 1024, "the size in bytes of %s", expr_str);
                free(expr_str);
            }
            break;
        }

        case NODE_CAST: {
            char* expr_str = translate_expression(node->data.cast.expression);
            snprintf(result, 1024, "%s converted to type '%s'", expr_str, node->data.cast.target_type);
            free(expr_str);
            break;
        }

        case NODE_COMPOUND_ASSIGN: {
            char* target_str = translate_expression(node->data.compound_assign.target);
            char* value_str = translate_expression(node->data.compound_assign.value);
            const char* op = node->data.compound_assign.operator;

            if (string_equals(op, "+=")) {
                snprintf(result, 1024, "increase %s by %s", target_str, value_str);
            } else if (string_equals(op, "-=")) {
                snprintf(result, 1024, "decrease %s by %s", target_str, value_str);
            } else if (string_equals(op, "*=")) {
                snprintf(result, 1024, "multiply %s by %s", target_str, value_str);
            } else if (string_equals(op, "/=")) {
                snprintf(result, 1024, "divide %s by %s", target_str, value_str);
            } else if (string_equals(op, "%=")) {
                snprintf(result, 1024, "set %s to the remainder when divided by %s", target_str, value_str);
            } else if (string_equals(op, "&=")) {
                snprintf(result, 1024, "bitwise AND %s with %s", target_str, value_str);
            } else if (string_equals(op, "|=")) {
                snprintf(result, 1024, "bitwise OR %s with %s", target_str, value_str);
            } else if (string_equals(op, "^=")) {
                snprintf(result, 1024, "bitwise XOR %s with %s", target_str, value_str);
            } else if (string_equals(op, "<<=")) {
                snprintf(result, 1024, "left-shift %s by %s bits", target_str, value_str);
            } else if (string_equals(op, ">>=")) {
                snprintf(result, 1024, "right-shift %s by %s bits", target_str, value_str);
            } else {
                snprintf(result, 1024, "apply %s to %s with %s", op, target_str, value_str);
            }
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

        case NODE_DO_WHILE: {
            char line[1024];
            snprintf(line, sizeof(line), "%sRepeatedly perform the following:", step_prefix);
            append_line(ctx, line);

            ctx->indent_level++;
            if (node->data.while_stmt.body->type == NODE_BLOCK) {
                for (int i = 0; i < node->data.while_stmt.body->data.block.statement_count; i++) {
                    translate_statement(ctx, node->data.while_stmt.body->data.block.statements[i], 0);
                }
            } else {
                translate_statement(ctx, node->data.while_stmt.body, 0);
            }
            ctx->indent_level--;

            char* cond_str = translate_expression(node->data.while_stmt.condition);
            snprintf(line, sizeof(line), "Continue whilst the condition \"%s\" remains true.", cond_str);
            append_line(ctx, line);
            free(cond_str);
            append_line(ctx, "");
            break;
        }

        case NODE_SWITCH: {
            char* expr_str = translate_expression(node->data.switch_stmt.expression);
            char line[1024];
            snprintf(line, sizeof(line), "%sDepending on the value of %s:", step_prefix, expr_str);
            append_line(ctx, line);
            free(expr_str);

            ctx->indent_level++;
            for (int i = 0; i < node->data.switch_stmt.case_count; i++) {
                ASTNode* case_node = node->data.switch_stmt.cases[i];
                if (case_node->type == NODE_CASE) {
                    char* value_str = translate_expression(case_node->data.case_stmt.value);
                    snprintf(line, sizeof(line), "When it equals %s:", value_str);
                    append_line(ctx, line);
                    free(value_str);
                } else {
                    append_line(ctx, "Otherwise (default):");
                }

                ctx->indent_level++;
                for (int j = 0; j < case_node->data.case_stmt.statement_count; j++) {
                    translate_statement(ctx, case_node->data.case_stmt.statements[j], 0);
                }
                ctx->indent_level--;
            }
            ctx->indent_level--;
            append_line(ctx, "");
            break;
        }

        case NODE_GOTO: {
            char line[1024];
            snprintf(line, sizeof(line), "%sJump to label '%s'.", step_prefix, node->data.goto_stmt.label);
            append_line(ctx, line);
            append_line(ctx, "");
            break;
        }

        case NODE_LABEL: {
            char line[1024];
            snprintf(line, sizeof(line), "Label '%s':", node->data.label_stmt.name);
            append_line(ctx, line);
            if (node->data.label_stmt.statement) {
                translate_statement(ctx, node->data.label_stmt.statement, 0);
            }
            break;
        }

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
