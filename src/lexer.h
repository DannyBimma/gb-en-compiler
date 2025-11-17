#ifndef LEXER_H
#define LEXER_H

#include "utils.h"

/* Token types */
typedef enum {
    /* Keywords */
    TOKEN_INT, TOKEN_CHAR, TOKEN_FLOAT, TOKEN_DOUBLE, TOKEN_VOID,
    TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR, TOKEN_DO,
    TOKEN_RETURN, TOKEN_BREAK, TOKEN_CONTINUE,
    TOKEN_STRUCT, TOKEN_UNION, TOKEN_TYPEDEF, TOKEN_SIZEOF,
    TOKEN_CONST, TOKEN_STATIC, TOKEN_EXTERN,

    /* Identifiers and literals */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_CHAR_LITERAL,

    /* Operators */
    TOKEN_PLUS,         /* + */
    TOKEN_MINUS,        /* - */
    TOKEN_STAR,         /* * */
    TOKEN_SLASH,        /* / */
    TOKEN_PERCENT,      /* % */
    TOKEN_ASSIGN,       /* = */
    TOKEN_EQ,           /* == */
    TOKEN_NE,           /* != */
    TOKEN_LT,           /* < */
    TOKEN_LE,           /* <= */
    TOKEN_GT,           /* > */
    TOKEN_GE,           /* >= */
    TOKEN_AND,          /* && */
    TOKEN_OR,           /* || */
    TOKEN_NOT,          /* ! */
    TOKEN_AMPERSAND,    /* & */
    TOKEN_PIPE,         /* | */
    TOKEN_INCREMENT,    /* ++ */
    TOKEN_DECREMENT,    /* -- */
    TOKEN_ARROW,        /* -> */
    TOKEN_DOT,          /* . */

    /* Punctuation */
    TOKEN_LPAREN,       /* ( */
    TOKEN_RPAREN,       /* ) */
    TOKEN_LBRACE,       /* { */
    TOKEN_RBRACE,       /* } */
    TOKEN_LBRACKET,     /* [ */
    TOKEN_RBRACKET,     /* ] */
    TOKEN_SEMICOLON,    /* ; */
    TOKEN_COMMA,        /* , */

    /* Special */
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

/* Token structure */
typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

/* Lexer structure */
typedef struct {
    const char* source;
    const char* filename;
    int current;
    int line;
    int column;
    int length;
} Lexer;

/* Lexer functions */
Lexer* lexer_create(const char* source, const char* filename);
void lexer_destroy(Lexer* lexer);
Token* lexer_next_token(Lexer* lexer);
Token* token_create(TokenType type, const char* lexeme, int line, int column);
void token_destroy(Token* token);
const char* token_type_to_string(TokenType type);

/* Token list for parser */
typedef struct {
    Token** tokens;
    int count;
    int capacity;
} TokenList;

TokenList* tokenize(const char* source, const char* filename);
void token_list_destroy(TokenList* list);

#endif /* LEXER_H */
