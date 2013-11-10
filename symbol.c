
// General stuff
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symbol.h"

symbol_table_t* symbol_init() {
  symbol_table_t* symbol_table = malloc(sizeof(symbol_table_t));
  symbol_table->symbols = list_init();
  symbol_table->parent = NULL;
  return symbol_table;
}

void symbol_free(symbol_t* symbol) {
  free(symbol->name);
  free(symbol);
}

void symbol_table_free(symbol_table_t* symbol_table) {
  printf("Freeing table: %p -> %p\n", symbol_table, symbol_table->symbols);
  list_visit(symbol_table->symbols, (void(*)(void*))symbol_free);
  list_free(symbol_table->symbols);
  free(symbol_table);
}

symbol_table_t* symbol_create_scope(symbol_table_t* parent) {
  symbol_table_t* scope = symbol_init();
  scope->parent = parent;
  return scope;
}

symbol_t* symbol_get(symbol_table_t* symbol_table, char* name) {
  if (!symbol_table) return NULL;
  symbol_t* symbol = symbol_get_in_scope(symbol_table, name);
  if (symbol)
    return symbol;
  return symbol_get(symbol_table->parent, name);
}

symbol_t* symbol_get_in_scope(symbol_table_t* symbol_table, char* name) {
  if (!symbol_table) return NULL;
  list_item_t* iter = list_iter_init(symbol_table->symbols);
  for (; iter; iter = list_iter(iter)) {
    symbol_t* candidate = iter->val;
    if (strcmp(candidate->name, name) == 0) {
      return candidate;
    }
  }
  return NULL;
}

symbol_t* symbol_set(symbol_table_t* symbol_table, char* name, type_t* type, bool is_param) {
  symbol_t* new_symbol = (symbol_t*)malloc(sizeof(symbol_t));
  new_symbol->name = name;
  new_symbol->type = type;
  new_symbol->is_param = is_param;
  new_symbol->ret_type = NULL;
  new_symbol->value = NULL;
  new_symbol->num_params = 0;
  list_push(symbol_table->symbols, new_symbol);
  return new_symbol;
}

