#include "parser.h"

/* Forward declarations for recursive descent */
static ASTNode* parse_function(Parser* parser);
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_assignment(Parser* parser);
static ASTNode* parse_logical_or(Parser* parser);
static ASTNode* parse_logical_and(Parser* parser);
static ASTNode* parse_equality(Parser* parser);
static ASTNode* parse_comparison(Parser* parser);
static ASTNode* parse_term(Parser* parser);
static ASTNode* parse_factor(Parser* parser);
static ASTNode* parse_unary(Parser* parser);
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
           type == TOKEN_VOID;
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

/* Parse unary expressions */
static ASTNode* parse_unary(Parser* parser) {
    if (match_multiple(parser, 6, TOKEN_NOT, TOKEN_MINUS, TOKEN_PLUS,
                      TOKEN_INCREMENT, TOKEN_DECREMENT, TOKEN_AMPERSAND)) {
        Token* op = previous(parser);
        ASTNode* operand = parse_unary(parser);
        return ast_create_unary_op(op->lexeme, operand);
    }

    return parse_primary(parser);
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

/* Parse comparison expressions */
static ASTNode* parse_comparison(Parser* parser) {
    ASTNode* left = parse_term(parser);

    while (match_multiple(parser, 4, TOKEN_GT, TOKEN_GE, TOKEN_LT, TOKEN_LE)) {
        Token* op = previous(parser);
        ASTNode* right = parse_term(parser);
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

/* Parse logical AND expressions */
static ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* left = parse_equality(parser);

    while (match(parser, TOKEN_AND)) {
        Token* op = previous(parser);
        ASTNode* right = parse_equality(parser);
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

/* Parse assignment expressions */
static ASTNode* parse_assignment(Parser* parser) {
    ASTNode* expr = parse_logical_or(parser);

    if (match(parser, TOKEN_ASSIGN)) {
        ASTNode* value = parse_assignment(parser);
        return ast_create_assignment(expr, value);
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
