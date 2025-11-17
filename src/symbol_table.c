#include "symbol_table.h"

/* Symbol table creation and destruction */

SymbolTable* symbol_table_create(const char* scope_name, SymbolTable* parent) {
    SymbolTable* table = (SymbolTable*)safe_malloc(sizeof(SymbolTable));
    table->head = NULL;
    table->parent = parent;
    table->scope_name = string_duplicate(scope_name);
    return table;
}

void symbol_table_destroy(SymbolTable* table) {
    if (!table) return;

    Symbol* current = table->head;
    while (current) {
        Symbol* next = current->next;
        symbol_destroy(current);
        current = next;
    }

    free(table->scope_name);
    free(table);
}

/* Symbol creation and destruction */

Symbol* symbol_create(const char* name, const char* type, const char* scope, int line) {
    Symbol* symbol = (Symbol*)safe_malloc(sizeof(Symbol));
    symbol->name = string_duplicate(name);
    symbol->type = string_duplicate(type);
    symbol->scope = string_duplicate(scope);
    symbol->line_declared = line;
    symbol->is_function = 0;
    symbol->is_array = 0;
    symbol->next = NULL;
    return symbol;
}

void symbol_destroy(Symbol* symbol) {
    if (!symbol) return;

    free(symbol->name);
    free(symbol->type);
    free(symbol->scope);
    free(symbol);
}

/* Symbol table operations */

void symbol_table_insert(SymbolTable* table, Symbol* symbol) {
    if (!table || !symbol) return;

    symbol->next = table->head;
    table->head = symbol;
}

Symbol* symbol_table_lookup_local(SymbolTable* table, const char* name) {
    if (!table || !name) return NULL;

    Symbol* current = table->head;
    while (current) {
        if (string_equals(current->name, name)) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

Symbol* symbol_table_lookup(SymbolTable* table, const char* name) {
    if (!table || !name) return NULL;

    /* Search current scope */
    Symbol* symbol = symbol_table_lookup_local(table, name);
    if (symbol) return symbol;

    /* Search parent scopes */
    if (table->parent) {
        return symbol_table_lookup(table->parent, name);
    }

    return NULL;
}

/* Debug printing */

void symbol_table_print(SymbolTable* table) {
    if (!table) return;

    printf("Symbol Table [%s]:\n", table->scope_name);
    Symbol* current = table->head;
    while (current) {
        printf("  %s: %s (line %d)%s%s\n",
               current->name,
               current->type,
               current->line_declared,
               current->is_function ? " [function]" : "",
               current->is_array ? " [array]" : "");
        current = current->next;
    }
}
