#ifndef SYMBOLS_H

#define SYMBOLS_H

// Headers required by LLVM
#include <llvm-c/Core.h>

typedef struct symbol_t {
  char* name;
  LLVMValueRef value;
  struct symbol_t* next;
} symbol_t;

symbol_t* get_symbol(char* name);

symbol_t* set_symbol(char* name, LLVMValueRef value);

#endif
