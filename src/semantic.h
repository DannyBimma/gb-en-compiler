#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

/* Semantic analyzer structure */
typedef struct {
    SymbolTable* current_scope;
    SymbolTable* global_scope;
    const char* filename;
    int had_error;
} SemanticAnalyzer;

/* Semantic analyzer functions */
SemanticAnalyzer* semantic_analyzer_create(const char* filename);
void semantic_analyzer_destroy(SemanticAnalyzer* analyzer);

int analyze_semantics(ASTNode* program, const char* filename);

#endif /* SEMANTIC_H */
