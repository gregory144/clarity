#ifndef CONTEXT_H

#define CONTEXT_H

#include "symbol.h"
#include "type.h"

typedef struct context_t {
  symbol_table_t* symbol_table;
  type_system_t* type_sys;
} context_t;

context_t* context_init();

void context_free(context_t*);

#endif
