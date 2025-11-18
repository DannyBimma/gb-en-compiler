#include "parser.h"

/* Forward declarations for recursive descent */
static ASTNode* parse_function(Parser* parser);
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_assignment(Parser* parser);
static ASTNode* parse_ternary(Parser* parser);
static ASTNode* parse_logical_or(Parser* parser);
static ASTNode* parse_logical_and(Parser* parser);
static ASTNode* parse_bitwise_or(Parser* parser);
static ASTNode* parse_bitwise_xor(Parser* parser);
static ASTNode* parse_bitwise_and(Parser* parser);
static ASTNode* parse_equality(Parser* parser);
static ASTNode* parse_comparison(Parser* parser);
static ASTNode* parse_shift(Parser* parser);
static ASTNode* parse_term(Parser* parser);
static ASTNode* parse_factor(Parser* parser);
static ASTNode* parse_unary(Parser* parser);
static ASTNode* parse_postfix(Parser* parser);
static ASTNode* parse_primary(Parser* parser);

/* Parser creation */
Parser* parser_create(TokenList* tokens, const char* filename) {
    Parser* parser = (Parser*)safe_malloc(sizeof(Parser));
    parser->tokens = tokens->tokens;
    parser->current = 0;
    parser->count = tokens->count;
    parser->filename = filename;
    parser->had_error = 0;
    return parser;
}

void parser_destroy(Parser* parser) {
    free(parser);
}

/* Helper functions */

static Token* peek(Parser* parser) {
    if (parser->current >= parser->count) {
        return parser->tokens[parser->count - 1];
    }
    return parser->tokens[parser->current];
}

static Token* previous(Parser* parser) {
    return parser->tokens[parser->current - 1];
}

static int is_at_end(Parser* parser) {
    return peek(parser)->type == TOKEN_EOF;
}

static Token* advance(Parser* parser) {
    if (!is_at_end(parser)) {
        parser->current++;
    }
    return previous(parser);
}

static int check(Parser* parser, TokenType type) {
    if (is_at_end(parser)) return 0;
    return peek(parser)->type == type;
}

static int match(Parser* parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return 1;
    }
    return 0;
}

static int match_multiple(Parser* parser, int count, ...) {
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++) {
        TokenType type = va_arg(args, TokenType);
        if (check(parser, type)) {
            advance(parser);
            va_end(args);
            return 1;
        }
    }

    va_end(args);
    return 0;
}

static void error_at(Parser* parser, Token* token, const char* message) {
    parser->had_error = 1;
    report_error(parser->filename, token->line, token->column, message);
}

static Token* consume(Parser* parser, TokenType type, const char* message) {
    if (check(parser, type)) {
        return advance(parser);
    }
    error_at(parser, peek(parser), message);
    return NULL;
}

/* Type checking */
static int is_type(TokenType type) {
    return type == TOKEN_INT || type == TOKEN_CHAR ||
           type == TOKEN_FLOAT || type == TOKEN_DOUBLE ||
           type == TOKEN_VOID || type == TOKEN_SIGNED ||
           type == TOKEN_UNSIGNED || type == TOKEN_LONG ||
           type == TOKEN_SHORT || type == TOKEN_STRUCT ||
           type == TOKEN_UNION || type == TOKEN_ENUM ||
           type == TOKEN_CONST;
}

/* Check if compound assignment operator */
static int is_compound_assign(TokenType type) {
    return type == TOKEN_PLUS_ASSIGN || type == TOKEN_MINUS_ASSIGN ||
           type == TOKEN_STAR_ASSIGN || type == TOKEN_SLASH_ASSIGN ||
           type == TOKEN_PERCENT_ASSIGN || type == TOKEN_AND_ASSIGN ||
           type == TOKEN_OR_ASSIGN || type == TOKEN_XOR_ASSIGN ||
           type == TOKEN_SHL_ASSIGN || type == TOKEN_SHR_ASSIGN;
}

/* Parse primary expressions */
static ASTNode* parse_primary(Parser* parser) {
    /* Number literal */
    if (match(parser, TOKEN_NUMBER)) {
        Token* token = previous(parser);
        return ast_create_literal(token->lexeme, "number");
    }

    /* String literal */
    if (match(parser, TOKEN_STRING)) {
        Token* token = previous(parser);
        return ast_create_literal(token->lexeme, "string");
    }

    /* Character literal */
    if (match(parser, TOKEN_CHAR_LITERAL)) {
        Token* token = previous(parser);
        return ast_create_literal(token->lexeme, "char");
    }

    /* sizeof expression */
    if (match(parser, TOKEN_SIZEOF)) {
        consume(parser, TOKEN_LPAREN, "Expected '(' after 'sizeof'");

        /* Check if it's a type or expression */
        if (is_type(peek(parser)->type)) {
            Token* type_token = advance(parser);
            char type_str[128];
            strcpy(type_str, type_token->lexeme);

            /* Handle pointer types */
            while (match(parser, TOKEN_STAR)) {
                strcat(type_str, "*");
            }

            consume(parser, TOKEN_RPAREN, "Expected ')' after type");
            return ast_create_sizeof_type(type_str);
        } else {
            ASTNode* expr = parse_expression(parser);
            consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
            return ast_create_sizeof_expr(expr);
        }
    }

    /* Identifier or function call */
    if (match(parser, TOKEN_IDENTIFIER)) {
        Token* name_token = previous(parser);
        char* name = string_duplicate(name_token->lexeme);

        /* Function call */
        if (match(parser, TOKEN_LPAREN)) {
            ASTNode** args = NULL;
            int arg_count = 0;

            if (!check(parser, TOKEN_RPAREN)) {
                int capacity = 4;
                args = (ASTNode**)safe_malloc(sizeof(ASTNode*) * capacity);

                do {
                    if (arg_count >= capacity) {
                        capacity *= 2;
                        args = (ASTNode**)safe_realloc(args, sizeof(ASTNode*) * capacity);
                    }
                    args[arg_count++] = parse_expression(parser);
                } while (match(parser, TOKEN_COMMA));
            }

            consume(parser, TOKEN_RPAREN, "Expected ')' after arguments");
            return ast_create_function_call(name, args, arg_count);
        }

        /* Array access */
        if (match(parser, TOKEN_LBRACKET)) {
            ASTNode* index = parse_expression(parser);
            consume(parser, TOKEN_RBRACKET, "Expected ']' after array index");
            return ast_create_array_access(name, index);
        }

        /* Simple identifier */
        return ast_create_identifier(name);
    }

    /* Parenthesized expression */
    if (match(parser, TOKEN_LPAREN)) {
        ASTNode* expr = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }

    error_at(parser, peek(parser), "Expected expression");
    return NULL;
}

/* Parse postfix expressions (member access, array access, function calls) */
static ASTNode* parse_postfix(Parser* parser) {
    ASTNode* expr = parse_primary(parser);

    while (1) {
        if (match(parser, TOKEN_DOT)) {
            Token* member = consume(parser, TOKEN_IDENTIFIER, "Expected member name after '.'");
            if (member) {
                expr = ast_create_member_access(expr, member->lexeme, 0);
            }
        } else if (match(parser, TOKEN_ARROW)) {
            Token* member = consume(parser, TOKEN_IDENTIFIER, "Expected member name after '->'");
            if (member) {
                expr = ast_create_member_access(expr, member->lexeme, 1);
            }
        } else if (match(parser, TOKEN_LBRACKET)) {
            ASTNode* index = parse_expression(parser);
            consume(parser, TOKEN_RBRACKET, "Expected ']' after index");
            /* Convert to array access - need identifier name */
            if (expr->type == NODE_IDENTIFIER) {
                char* name = string_duplicate(expr->data.identifier.name);
                ast_destroy(expr);
                expr = ast_create_array_access(name, index);
            } else {
                /* For complex expressions like a[i][j] */
                expr = ast_create_binary_op("[]", expr, index);
            }
        } else if (match(parser, TOKEN_INCREMENT)) {
            expr = ast_create_unary_op("++post", expr);
        } else if (match(parser, TOKEN_DECREMENT)) {
            expr = ast_create_unary_op("--post", expr);
        } else {
            break;
        }
    }

    return expr;
}

/* Parse unary expressions */
static ASTNode* parse_unary(Parser* parser) {
    if (match_multiple(parser, 7, TOKEN_NOT, TOKEN_MINUS, TOKEN_PLUS,
                      TOKEN_INCREMENT, TOKEN_DECREMENT, TOKEN_AMPERSAND, TOKEN_TILDE)) {
        Token* op = previous(parser);
        ASTNode* operand = parse_unary(parser);
        return ast_create_unary_op(op->lexeme, operand);
    }

    /* Dereference operator */
    if (match(parser, TOKEN_STAR)) {
        ASTNode* operand = parse_unary(parser);
        return ast_create_unary_op("*", operand);
    }

    return parse_postfix(parser);
}

/* Parse multiplicative expressions */
static ASTNode* parse_factor(Parser* parser) {
    ASTNode* left = parse_unary(parser);

    while (match_multiple(parser, 3, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT)) {
        Token* op = previous(parser);
        ASTNode* right = parse_unary(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse additive expressions */
static ASTNode* parse_term(Parser* parser) {
    ASTNode* left = parse_factor(parser);

    while (match_multiple(parser, 2, TOKEN_PLUS, TOKEN_MINUS)) {
        Token* op = previous(parser);
        ASTNode* right = parse_factor(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse shift expressions */
static ASTNode* parse_shift(Parser* parser) {
    ASTNode* left = parse_term(parser);

    while (match_multiple(parser, 2, TOKEN_SHL, TOKEN_SHR)) {
        Token* op = previous(parser);
        ASTNode* right = parse_term(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse comparison expressions */
static ASTNode* parse_comparison(Parser* parser) {
    ASTNode* left = parse_shift(parser);

    while (match_multiple(parser, 4, TOKEN_GT, TOKEN_GE, TOKEN_LT, TOKEN_LE)) {
        Token* op = previous(parser);
        ASTNode* right = parse_shift(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse equality expressions */
static ASTNode* parse_equality(Parser* parser) {
    ASTNode* left = parse_comparison(parser);

    while (match_multiple(parser, 2, TOKEN_EQ, TOKEN_NE)) {
        Token* op = previous(parser);
        ASTNode* right = parse_comparison(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse bitwise AND expressions */
static ASTNode* parse_bitwise_and(Parser* parser) {
    ASTNode* left = parse_equality(parser);

    while (match(parser, TOKEN_AMPERSAND)) {
        Token* op = previous(parser);
        ASTNode* right = parse_equality(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse bitwise XOR expressions */
static ASTNode* parse_bitwise_xor(Parser* parser) {
    ASTNode* left = parse_bitwise_and(parser);

    while (match(parser, TOKEN_CARET)) {
        Token* op = previous(parser);
        ASTNode* right = parse_bitwise_and(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse bitwise OR expressions */
static ASTNode* parse_bitwise_or(Parser* parser) {
    ASTNode* left = parse_bitwise_xor(parser);

    while (match(parser, TOKEN_PIPE)) {
        Token* op = previous(parser);
        ASTNode* right = parse_bitwise_xor(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse logical AND expressions */
static ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* left = parse_bitwise_or(parser);

    while (match(parser, TOKEN_AND)) {
        Token* op = previous(parser);
        ASTNode* right = parse_bitwise_or(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse logical OR expressions */
static ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* left = parse_logical_and(parser);

    while (match(parser, TOKEN_OR)) {
        Token* op = previous(parser);
        ASTNode* right = parse_logical_and(parser);
        left = ast_create_binary_op(op->lexeme, left, right);
    }

    return left;
}

/* Parse ternary conditional expressions */
static ASTNode* parse_ternary(Parser* parser) {
    ASTNode* condition = parse_logical_or(parser);

    if (match(parser, TOKEN_QUESTION)) {
        ASTNode* then_expr = parse_expression(parser);
        consume(parser, TOKEN_COLON, "Expected ':' in ternary expression");
        ASTNode* else_expr = parse_ternary(parser);
        return ast_create_ternary(condition, then_expr, else_expr);
    }

    return condition;
}

/* Parse assignment expressions */
static ASTNode* parse_assignment(Parser* parser) {
    ASTNode* expr = parse_ternary(parser);

    if (match(parser, TOKEN_ASSIGN)) {
        ASTNode* value = parse_assignment(parser);
        return ast_create_assignment(expr, value);
    }

    /* Check for compound assignment */
    if (is_compound_assign(peek(parser)->type)) {
        Token* op = advance(parser);
        ASTNode* value = parse_assignment(parser);
        return ast_create_compound_assign(op->lexeme, expr, value);
    }

    return expr;
}

/* Parse expressions */
static ASTNode* parse_expression(Parser* parser) {
    return parse_assignment(parser);
}

/* Parse block statements */
static ASTNode* parse_block(Parser* parser) {
    ASTNode* block = ast_create_block();

    while (!check(parser, TOKEN_RBRACE) && !is_at_end(parser)) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            ast_add_statement(block, stmt);
        }
    }

    consume(parser, TOKEN_RBRACE, "Expected '}' after block");
    return block;
}

/* Parse statements */
static ASTNode* parse_statement(Parser* parser) {
    /* Variable declaration */
    if (is_type(peek(parser)->type)) {
        Token* type_token = advance(parser);
        Token* name_token = consume(parser, TOKEN_IDENTIFIER, "Expected variable name");

        if (!name_token) return NULL;

        char* type = string_duplicate(type_token->lexeme);
        char* name = string_duplicate(name_token->lexeme);

        /* Array declaration */
        if (match(parser, TOKEN_LBRACKET)) {
            ASTNode* size = NULL;
            if (!check(parser, TOKEN_RBRACKET)) {
                size = parse_expression(parser);
            }
            consume(parser, TOKEN_RBRACKET, "Expected ']' after array size");
            consume(parser, TOKEN_SEMICOLON, "Expected ';' after declaration");
            return ast_create_array_declaration(type, name, size);
        }

        /* Variable with initializer */
        ASTNode* initializer = NULL;
        if (match(parser, TOKEN_ASSIGN)) {
            initializer = parse_expression(parser);
        }

        consume(parser, TOKEN_SEMICOLON, "Expected ';' after declaration");
        return ast_create_declaration(type, name, initializer);
    }

    /* If statement */
    if (match(parser, TOKEN_IF)) {
        consume(parser, TOKEN_LPAREN, "Expected '(' after 'if'");
        ASTNode* condition = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after condition");

        ASTNode* then_branch;
        if (match(parser, TOKEN_LBRACE)) {
            then_branch = parse_block(parser);
        } else {
            then_branch = parse_statement(parser);
        }

        ASTNode* else_branch = NULL;
        if (match(parser, TOKEN_ELSE)) {
            if (match(parser, TOKEN_LBRACE)) {
                else_branch = parse_block(parser);
            } else {
                else_branch = parse_statement(parser);
            }
        }

        return ast_create_if(condition, then_branch, else_branch);
    }

    /* While statement */
    if (match(parser, TOKEN_WHILE)) {
        consume(parser, TOKEN_LPAREN, "Expected '(' after 'while'");
        ASTNode* condition = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after condition");

        ASTNode* body;
        if (match(parser, TOKEN_LBRACE)) {
            body = parse_block(parser);
        } else {
            body = parse_statement(parser);
        }

        return ast_create_while(condition, body);
    }

    /* For statement */
    if (match(parser, TOKEN_FOR)) {
        consume(parser, TOKEN_LPAREN, "Expected '(' after 'for'");

        ASTNode* init = NULL;
        if (!check(parser, TOKEN_SEMICOLON)) {
            if (is_type(peek(parser)->type)) {
                init = parse_statement(parser);
            } else {
                init = parse_expression(parser);
                consume(parser, TOKEN_SEMICOLON, "Expected ';' after loop initializer");
            }
        } else {
            advance(parser);
        }

        ASTNode* condition = NULL;
        if (!check(parser, TOKEN_SEMICOLON)) {
            condition = parse_expression(parser);
        }
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after loop condition");

        ASTNode* increment = NULL;
        if (!check(parser, TOKEN_RPAREN)) {
            increment = parse_expression(parser);
        }
        consume(parser, TOKEN_RPAREN, "Expected ')' after for clauses");

        ASTNode* body;
        if (match(parser, TOKEN_LBRACE)) {
            body = parse_block(parser);
        } else {
            body = parse_statement(parser);
        }

        return ast_create_for(init, condition, increment, body);
    }

    /* Return statement */
    if (match(parser, TOKEN_RETURN)) {
        ASTNode* value = NULL;
        if (!check(parser, TOKEN_SEMICOLON)) {
            value = parse_expression(parser);
        }
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after return");
        return ast_create_return(value);
    }

    /* Break statement */
    if (match(parser, TOKEN_BREAK)) {
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after break");
        return ast_create_break();
    }

    /* Continue statement */
    if (match(parser, TOKEN_CONTINUE)) {
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after continue");
        return ast_create_continue();
    }

    /* Do-while statement */
    if (match(parser, TOKEN_DO)) {
        ASTNode* body;
        if (match(parser, TOKEN_LBRACE)) {
            body = parse_block(parser);
        } else {
            body = parse_statement(parser);
        }

        consume(parser, TOKEN_WHILE, "Expected 'while' after do block");
        consume(parser, TOKEN_LPAREN, "Expected '(' after 'while'");
        ASTNode* condition = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after condition");
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after do-while");

        return ast_create_do_while(body, condition);
    }

    /* Switch statement */
    if (match(parser, TOKEN_SWITCH)) {
        consume(parser, TOKEN_LPAREN, "Expected '(' after 'switch'");
        ASTNode* expression = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after switch expression");
        consume(parser, TOKEN_LBRACE, "Expected '{' before switch body");

        ASTNode* switch_stmt = ast_create_switch(expression);
        ASTNode* current_case = NULL;

        while (!check(parser, TOKEN_RBRACE) && !is_at_end(parser)) {
            if (match(parser, TOKEN_CASE)) {
                ASTNode* value = parse_expression(parser);
                consume(parser, TOKEN_COLON, "Expected ':' after case value");
                current_case = ast_create_case(value);
                ast_add_case(switch_stmt, current_case);
            } else if (match(parser, TOKEN_DEFAULT)) {
                consume(parser, TOKEN_COLON, "Expected ':' after 'default'");
                current_case = ast_create_default();
                ast_add_case(switch_stmt, current_case);
            } else if (current_case) {
                ASTNode* stmt = parse_statement(parser);
                if (stmt) {
                    ast_add_case_statement(current_case, stmt);
                }
            } else {
                error_at(parser, peek(parser), "Statement outside of case in switch");
                advance(parser);
            }
        }

        consume(parser, TOKEN_RBRACE, "Expected '}' after switch body");
        return switch_stmt;
    }

    /* Goto statement */
    if (match(parser, TOKEN_GOTO)) {
        Token* label = consume(parser, TOKEN_IDENTIFIER, "Expected label name after 'goto'");
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after goto");
        if (label) {
            return ast_create_goto(label->lexeme);
        }
        return NULL;
    }

    /* Label statement - identifier followed by colon */
    if (check(parser, TOKEN_IDENTIFIER)) {
        /* Look ahead for colon */
        int saved = parser->current;
        advance(parser);
        if (check(parser, TOKEN_COLON)) {
            parser->current = saved;
            Token* label = advance(parser);
            advance(parser); /* consume colon */
            ASTNode* stmt = parse_statement(parser);
            return ast_create_label(label->lexeme, stmt);
        }
        parser->current = saved;
    }

    /* Block statement */
    if (match(parser, TOKEN_LBRACE)) {
        return parse_block(parser);
    }

    /* Expression statement */
    ASTNode* expr = parse_expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after expression");
    return expr;
}

/* Parse function */
static ASTNode* parse_function(Parser* parser) {
    /* Return type */
    if (!is_type(peek(parser)->type)) {
        error_at(parser, peek(parser), "Expected return type");
        return NULL;
    }
    Token* return_type_token = advance(parser);
    char* return_type = string_duplicate(return_type_token->lexeme);

    /* Function name */
    Token* name_token = consume(parser, TOKEN_IDENTIFIER, "Expected function name");
    if (!name_token) return NULL;
    char* name = string_duplicate(name_token->lexeme);

    /* Parameters */
    consume(parser, TOKEN_LPAREN, "Expected '(' after function name");

    Parameter** params = NULL;
    int param_count = 0;
    int param_capacity = 4;

    if (!check(parser, TOKEN_RPAREN)) {
        params = (Parameter**)safe_malloc(sizeof(Parameter*) * param_capacity);

        do {
            if (!is_type(peek(parser)->type)) {
                error_at(parser, peek(parser), "Expected parameter type");
                break;
            }

            Token* param_type_token = advance(parser);
            Token* param_name_token = consume(parser, TOKEN_IDENTIFIER, "Expected parameter name");

            if (!param_name_token) break;

            int is_array = 0;
            if (match(parser, TOKEN_LBRACKET)) {
                is_array = 1;
                consume(parser, TOKEN_RBRACKET, "Expected ']' after '['");
            }

            if (param_count >= param_capacity) {
                param_capacity *= 2;
                params = (Parameter**)safe_realloc(params, sizeof(Parameter*) * param_capacity);
            }

            params[param_count++] = parameter_create(
                param_type_token->lexeme,
                param_name_token->lexeme,
                is_array
            );

        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");

    /* Function body */
    consume(parser, TOKEN_LBRACE, "Expected '{' before function body");
    ASTNode* body = parse_block(parser);

    return ast_create_function(return_type, name, params, param_count, body);
}

/* Main parse function */
ASTNode* parse(TokenList* tokens, const char* filename) {
    Parser* parser = parser_create(tokens, filename);
    ASTNode* program = ast_create_program();

    while (!is_at_end(parser)) {
        ASTNode* function = parse_function(parser);
        if (function) {
            ast_add_function(program, function);
        } else {
            /* Skip to next function on error */
            while (!is_at_end(parser) && !is_type(peek(parser)->type)) {
                advance(parser);
            }
        }
    }

    int had_error = parser->had_error;
    parser_destroy(parser);

    if (had_error) {
        ast_destroy(program);
        return NULL;
    }

    return program;
}
