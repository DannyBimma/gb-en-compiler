#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

/* Parser structure */
typedef struct {
    Token** tokens;
    int current;
    int count;
    const char* filename;
    int had_error;
} Parser;

/* Parser functions */
Parser* parser_create(TokenList* tokens, const char* filename);
void parser_destroy(Parser* parser);
ASTNode* parse(TokenList* tokens, const char* filename);

#endif /* PARSER_H */
