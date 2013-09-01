
// General stuff
#include <stdlib.h>
#include <string.h>

#include "symbols.h"

symbol_t* symbols;

symbol_t* get_symbol(char* name) {
  symbol_t* curr = symbols;
    while (curr) {
    if (strcmp(curr->name, name) == 0) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

symbol_t* set_symbol(char* name, LLVMValueRef value) {
  symbol_t* new_symbol = (symbol_t*)malloc(sizeof(symbol_t));
  new_symbol->name = name;
  new_symbol->value = value;
  new_symbol->next = symbols;
  symbols = new_symbol;
  return new_symbol;
}

