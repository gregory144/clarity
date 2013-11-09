#include "stdlib.h"

#include "context.h"

context_t* context_init() {
  context_t* context = malloc(sizeof(context_t));
  context->symbol_table = symbol_init();
  context->type_sys = type_init();
  return context;
}

void context_free(context_t* context) {
  symbol_table_free(context->symbol_table);
  type_system_free(context->type_sys);
  free(context);
}
