
#ifndef PARSE_H

#define PARSE_H

#include "enums.h"
#include "context.h"

typedef struct {
  FILE* input;
  token_t current_tok;
  char ident[512];
  long int_val;
  double float_val;
} tokenizer_t;

expr_node_t* parse_file(context_t* context, FILE *input);

#endif
