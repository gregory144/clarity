#include "stdlib.h"

#include "context.h"

context_t* context_init() {
  context_t* context = malloc(sizeof(context_t));
  context->symbol_table = symbol_init();
  context->type_sys = type_init();
  return context;
}
