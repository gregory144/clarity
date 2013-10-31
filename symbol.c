
// General stuff
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symbol.h"

symbol_t* symbols;

symbol_t* symbol_get(char* name) {
  symbol_t* curr = symbols;
  while (curr) {
    if (name != NULL && curr->name != NULL) {
      printf("%s\n", curr->name);
      printf("%s\n", name);
      printf("getting symbol '%s', comparing to '%s'\n", name, curr->name);
    } else
      printf("current symbol has no name\n");
    if (strcmp(curr->name, name) == 0) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

symbol_t* symbol_set(char* name, expr_type_t type) {
  symbol_t* new_symbol = (symbol_t*)malloc(sizeof(symbol_t));
  new_symbol->name = name;
  new_symbol->type = type;
  new_symbol->ret_type = EXPR_TYPE_INVALID;
  new_symbol->value = NULL;
  new_symbol->next = symbols;
  symbols = new_symbol;
  return new_symbol;
}

