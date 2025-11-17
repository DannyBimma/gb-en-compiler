#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "ast.h"
#include "utils.h"

/* Translation context */
typedef struct {
    char* output;
    size_t output_size;
    size_t output_capacity;
    int indent_level;
} TranslationContext;

/* Translator functions */
char* translate_to_english(ASTNode* program);

#endif /* TRANSLATOR_H */
