#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "utils.h"

/* Symbol structure */
typedef struct Symbol {
    char* name;
    char* type;
    char* scope;
    int line_declared;
    int is_function;
    int is_array;
    struct Symbol* next;
} Symbol;

/* Symbol table structure */
typedef struct SymbolTable {
    Symbol* head;
    struct SymbolTable* parent;  /* For nested scopes */
    char* scope_name;
} SymbolTable;

/* Symbol table functions */
SymbolTable* symbol_table_create(const char* scope_name, SymbolTable* parent);
void symbol_table_destroy(SymbolTable* table);

Symbol* symbol_create(const char* name, const char* type, const char* scope, int line);
void symbol_destroy(Symbol* symbol);

void symbol_table_insert(SymbolTable* table, Symbol* symbol);
Symbol* symbol_table_lookup(SymbolTable* table, const char* name);
Symbol* symbol_table_lookup_local(SymbolTable* table, const char* name);

void symbol_table_print(SymbolTable* table);

#endif /* SYMBOL_TABLE_H */
