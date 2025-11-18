#include "lexer.h"

/* Keyword mapping */
typedef struct {
    const char* keyword;
    TokenType type;
} KeywordMap;

static KeywordMap keywords[] = {
    {"int", TOKEN_INT},
    {"char", TOKEN_CHAR},
    {"float", TOKEN_FLOAT},
    {"double", TOKEN_DOUBLE},
    {"void", TOKEN_VOID},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"do", TOKEN_DO},
    {"return", TOKEN_RETURN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"struct", TOKEN_STRUCT},
    {"union", TOKEN_UNION},
    {"typedef", TOKEN_TYPEDEF},
    {"sizeof", TOKEN_SIZEOF},
    {"const", TOKEN_CONST},
    {"static", TOKEN_STATIC},
    {"extern", TOKEN_EXTERN},
    {"switch", TOKEN_SWITCH},
    {"case", TOKEN_CASE},
    {"default", TOKEN_DEFAULT},
    {"enum", TOKEN_ENUM},
    {"goto", TOKEN_GOTO},
    {"signed", TOKEN_SIGNED},
    {"unsigned", TOKEN_UNSIGNED},
    {"long", TOKEN_LONG},
    {"short", TOKEN_SHORT},
    {NULL, TOKEN_EOF}
};

/* Token creation and destruction */

Token* token_create(TokenType type, const char* lexeme, int line, int column) {
    Token* token = (Token*)safe_malloc(sizeof(Token));
    token->type = type;
    token->lexeme = string_duplicate(lexeme);
    token->line = line;
    token->column = column;
    return token;
}

void token_destroy(Token* token) {
    if (token) {
        free(token->lexeme);
        free(token);
    }
}

/* Lexer creation and destruction */

Lexer* lexer_create(const char* source, const char* filename) {
    Lexer* lexer = (Lexer*)safe_malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->filename = filename;
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->length = strlen(source);
    return lexer;
}

void lexer_destroy(Lexer* lexer) {
    free(lexer);
}

/* Helper functions */

static int is_at_end(Lexer* lexer) {
    return lexer->current >= lexer->length;
}

static char peek(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (lexer->current + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->current + 1];
}

static char advance(Lexer* lexer) {
    char c = lexer->source[lexer->current++];
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

static int match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return 0;
    if (lexer->source[lexer->current] != expected) return 0;
    advance(lexer);
    return 1;
}

static void skip_whitespace(Lexer* lexer) {
    while (!is_at_end(lexer)) {
        char c = peek(lexer);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(lexer);
        } else if (c == '#') {
            /* Preprocessor directive - skip entire line */
            while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                advance(lexer);
            }
        } else if (c == '/') {
            if (peek_next(lexer) == '/') {
                /* Single-line comment */
                while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                    advance(lexer);
                }
            } else if (peek_next(lexer) == '*') {
                /* Multi-line comment */
                advance(lexer); /* / */
                advance(lexer); /* * */
                while (!is_at_end(lexer)) {
                    if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                        advance(lexer); /* * */
                        advance(lexer); /* / */
                        break;
                    }
                    advance(lexer);
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

static TokenType check_keyword(const char* lexeme) {
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (string_equals(lexeme, keywords[i].keyword)) {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENTIFIER;
}

static Token* scan_identifier(Lexer* lexer) {
    int start = lexer->current;
    int start_column = lexer->column;

    while (is_identifier_char(peek(lexer))) {
        advance(lexer);
    }

    int length = lexer->current - start;
    char* lexeme = (char*)safe_malloc(length + 1);
    strncpy(lexeme, &lexer->source[start], length);
    lexeme[length] = '\0';

    TokenType type = check_keyword(lexeme);
    Token* token = token_create(type, lexeme, lexer->line, start_column);
    free(lexeme);
    return token;
}

static Token* scan_number(Lexer* lexer) {
    int start = lexer->current;
    int start_column = lexer->column;

    while (is_digit(peek(lexer))) {
        advance(lexer);
    }

    /* Handle decimal point */
    if (peek(lexer) == '.' && is_digit(peek_next(lexer))) {
        advance(lexer); /* . */
        while (is_digit(peek(lexer))) {
            advance(lexer);
        }
    }

    int length = lexer->current - start;
    char* lexeme = (char*)safe_malloc(length + 1);
    strncpy(lexeme, &lexer->source[start], length);
    lexeme[length] = '\0';

    Token* token = token_create(TOKEN_NUMBER, lexeme, lexer->line, start_column);
    free(lexeme);
    return token;
}

static Token* scan_string(Lexer* lexer) {
    int start = lexer->current;
    int start_column = lexer->column;

    advance(lexer); /* Opening " */

    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\\') {
            advance(lexer); /* Escape character */
            if (!is_at_end(lexer)) advance(lexer);
        } else {
            advance(lexer);
        }
    }

    if (is_at_end(lexer)) {
        return token_create(TOKEN_ERROR, "Unterminated string", lexer->line, start_column);
    }

    advance(lexer); /* Closing " */

    int length = lexer->current - start;
    char* lexeme = (char*)safe_malloc(length + 1);
    strncpy(lexeme, &lexer->source[start], length);
    lexeme[length] = '\0';

    Token* token = token_create(TOKEN_STRING, lexeme, lexer->line, start_column);
    free(lexeme);
    return token;
}

static Token* scan_char(Lexer* lexer) {
    int start = lexer->current;
    int start_column = lexer->column;

    advance(lexer); /* Opening ' */

    while (peek(lexer) != '\'' && !is_at_end(lexer)) {
        if (peek(lexer) == '\\') {
            advance(lexer);
            if (!is_at_end(lexer)) advance(lexer);
        } else {
            advance(lexer);
        }
    }

    if (is_at_end(lexer)) {
        return token_create(TOKEN_ERROR, "Unterminated character literal", lexer->line, start_column);
    }

    advance(lexer); /* Closing ' */

    int length = lexer->current - start;
    char* lexeme = (char*)safe_malloc(length + 1);
    strncpy(lexeme, &lexer->source[start], length);
    lexeme[length] = '\0';

    Token* token = token_create(TOKEN_CHAR_LITERAL, lexeme, lexer->line, start_column);
    free(lexeme);
    return token;
}

/* Main tokenization function */

Token* lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);

    if (is_at_end(lexer)) {
        return token_create(TOKEN_EOF, "", lexer->line, lexer->column);
    }

    int start_column = lexer->column;
    char c = advance(lexer);

    /* Identifiers and keywords */
    if (is_identifier_start(c)) {
        lexer->current--;
        lexer->column--;
        return scan_identifier(lexer);
    }

    /* Numbers */
    if (is_digit(c)) {
        lexer->current--;
        lexer->column--;
        return scan_number(lexer);
    }

    /* Strings */
    if (c == '"') {
        lexer->current--;
        lexer->column--;
        return scan_string(lexer);
    }

    /* Character literals */
    if (c == '\'') {
        lexer->current--;
        lexer->column--;
        return scan_char(lexer);
    }

    /* Two-character and three-character operators */
    switch (c) {
        case '+':
            if (match(lexer, '+')) return token_create(TOKEN_INCREMENT, "++", lexer->line, start_column);
            if (match(lexer, '=')) return token_create(TOKEN_PLUS_ASSIGN, "+=", lexer->line, start_column);
            return token_create(TOKEN_PLUS, "+", lexer->line, start_column);
        case '-':
            if (match(lexer, '-')) return token_create(TOKEN_DECREMENT, "--", lexer->line, start_column);
            if (match(lexer, '>')) return token_create(TOKEN_ARROW, "->", lexer->line, start_column);
            if (match(lexer, '=')) return token_create(TOKEN_MINUS_ASSIGN, "-=", lexer->line, start_column);
            return token_create(TOKEN_MINUS, "-", lexer->line, start_column);
        case '*':
            if (match(lexer, '=')) return token_create(TOKEN_STAR_ASSIGN, "*=", lexer->line, start_column);
            return token_create(TOKEN_STAR, "*", lexer->line, start_column);
        case '/':
            if (match(lexer, '=')) return token_create(TOKEN_SLASH_ASSIGN, "/=", lexer->line, start_column);
            return token_create(TOKEN_SLASH, "/", lexer->line, start_column);
        case '%':
            if (match(lexer, '=')) return token_create(TOKEN_PERCENT_ASSIGN, "%=", lexer->line, start_column);
            return token_create(TOKEN_PERCENT, "%", lexer->line, start_column);
        case '=':
            if (match(lexer, '=')) return token_create(TOKEN_EQ, "==", lexer->line, start_column);
            return token_create(TOKEN_ASSIGN, "=", lexer->line, start_column);
        case '!':
            if (match(lexer, '=')) return token_create(TOKEN_NE, "!=", lexer->line, start_column);
            return token_create(TOKEN_NOT, "!", lexer->line, start_column);
        case '<':
            if (match(lexer, '<')) {
                if (match(lexer, '=')) return token_create(TOKEN_SHL_ASSIGN, "<<=", lexer->line, start_column);
                return token_create(TOKEN_SHL, "<<", lexer->line, start_column);
            }
            if (match(lexer, '=')) return token_create(TOKEN_LE, "<=", lexer->line, start_column);
            return token_create(TOKEN_LT, "<", lexer->line, start_column);
        case '>':
            if (match(lexer, '>')) {
                if (match(lexer, '=')) return token_create(TOKEN_SHR_ASSIGN, ">>=", lexer->line, start_column);
                return token_create(TOKEN_SHR, ">>", lexer->line, start_column);
            }
            if (match(lexer, '=')) return token_create(TOKEN_GE, ">=", lexer->line, start_column);
            return token_create(TOKEN_GT, ">", lexer->line, start_column);
        case '&':
            if (match(lexer, '&')) return token_create(TOKEN_AND, "&&", lexer->line, start_column);
            if (match(lexer, '=')) return token_create(TOKEN_AND_ASSIGN, "&=", lexer->line, start_column);
            return token_create(TOKEN_AMPERSAND, "&", lexer->line, start_column);
        case '|':
            if (match(lexer, '|')) return token_create(TOKEN_OR, "||", lexer->line, start_column);
            if (match(lexer, '=')) return token_create(TOKEN_OR_ASSIGN, "|=", lexer->line, start_column);
            return token_create(TOKEN_PIPE, "|", lexer->line, start_column);
        case '^':
            if (match(lexer, '=')) return token_create(TOKEN_XOR_ASSIGN, "^=", lexer->line, start_column);
            return token_create(TOKEN_CARET, "^", lexer->line, start_column);
        case '~':
            return token_create(TOKEN_TILDE, "~", lexer->line, start_column);
        case '?':
            return token_create(TOKEN_QUESTION, "?", lexer->line, start_column);
        case ':':
            return token_create(TOKEN_COLON, ":", lexer->line, start_column);
        case '.':
            return token_create(TOKEN_DOT, ".", lexer->line, start_column);
        case '(':
            return token_create(TOKEN_LPAREN, "(", lexer->line, start_column);
        case ')':
            return token_create(TOKEN_RPAREN, ")", lexer->line, start_column);
        case '{':
            return token_create(TOKEN_LBRACE, "{", lexer->line, start_column);
        case '}':
            return token_create(TOKEN_RBRACE, "}", lexer->line, start_column);
        case '[':
            return token_create(TOKEN_LBRACKET, "[", lexer->line, start_column);
        case ']':
            return token_create(TOKEN_RBRACKET, "]", lexer->line, start_column);
        case ';':
            return token_create(TOKEN_SEMICOLON, ";", lexer->line, start_column);
        case ',':
            return token_create(TOKEN_COMMA, ",", lexer->line, start_column);
    }

    char error_msg[100];
    snprintf(error_msg, sizeof(error_msg), "Unexpected character: '%c'", c);
    return token_create(TOKEN_ERROR, error_msg, lexer->line, start_column);
}

/* Tokenize entire source */

TokenList* tokenize(const char* source, const char* filename) {
    TokenList* list = (TokenList*)safe_malloc(sizeof(TokenList));
    list->capacity = 128;
    list->count = 0;
    list->tokens = (Token**)safe_malloc(sizeof(Token*) * list->capacity);

    Lexer* lexer = lexer_create(source, filename);

    while (1) {
        Token* token = lexer_next_token(lexer);

        if (list->count >= list->capacity) {
            list->capacity *= 2;
            list->tokens = (Token**)safe_realloc(list->tokens, sizeof(Token*) * list->capacity);
        }

        list->tokens[list->count++] = token;

        if (token->type == TOKEN_EOF || token->type == TOKEN_ERROR) {
            break;
        }
    }

    lexer_destroy(lexer);
    return list;
}

void token_list_destroy(TokenList* list) {
    if (list) {
        for (int i = 0; i < list->count; i++) {
            token_destroy(list->tokens[i]);
        }
        free(list->tokens);
        free(list);
    }
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_INT: return "int";
        case TOKEN_CHAR: return "char";
        case TOKEN_FLOAT: return "float";
        case TOKEN_DOUBLE: return "double";
        case TOKEN_VOID: return "void";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_FOR: return "for";
        case TOKEN_RETURN: return "return";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_EQ: return "==";
        case TOKEN_NE: return "!=";
        case TOKEN_LT: return "<";
        case TOKEN_GT: return ">";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}
