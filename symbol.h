#ifndef SYMBOLS_H

#define SYMBOLS_H

// Headers required by LLVM
#include <llvm-c/Core.h>

#include "enums.h"

typedef struct symbol_t {
  char* name;
  expr_type_t type;
  expr_type_t ret_type;
  LLVMValueRef value;
  struct symbol_t* next;
} symbol_t;

symbol_t* symbol_get(char* name);

symbol_t* symbol_set(char* name, expr_type_t type);

#endif
