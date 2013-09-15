#ifndef SYMBOLS_H

#define SYMBOLS_H

// Headers required by LLVM
#include <llvm-c/Core.h>

#include "enums.h"

typedef struct symbol_t {
  char* name;
  expr_type_t type;
  LLVMValueRef value;
  struct symbol_t* next;
} symbol_t;

symbol_t* get_symbol(char* name);

symbol_t* set_symbol(char* name, expr_type_t type);

#endif
