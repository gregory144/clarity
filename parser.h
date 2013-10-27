
#ifndef PARSER_H

#define PARSER_H

#include "ast.h"
#include "enums.h"

typedef struct {
  FILE* input;
  token_t current_tok;
  char ident[512];
  long int_val;
  double float_val;
} tokenizer_t;

expr_node_t* parse_file(FILE *input);

#endif
