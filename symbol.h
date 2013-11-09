#ifndef SYMBOL_H

#define SYMBOL_H

// Headers required by LLVM
#include <llvm-c/Core.h>

#include "type.h"
#include "list.h"
#include "enums.h"

typedef struct symbol_table_t {
  struct symbol_table_t* parent;
  list_t* symbols;
} symbol_table_t;

typedef struct symbol_t {
  char* name;
  type_t* type;
  type_t* ret_type;
  LLVMValueRef value;
  size_t num_params;
  bool is_param;
} symbol_t;

symbol_table_t* symbol_init();

void symbol_table_free(symbol_table_t*);

symbol_table_t* symbol_create_scope(symbol_table_t*);

symbol_t* symbol_get(symbol_table_t* symbol_table, char* name);

symbol_t* symbol_get_in_scope(symbol_table_t* symbol_table, char* name);

symbol_t* symbol_set(symbol_table_t* symbol_table, char* name, type_t* type, bool is_param);

#endif
