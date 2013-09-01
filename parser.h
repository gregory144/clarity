
#ifndef PARSER_H

#define PARSER_H

#include "ast.h"
#include "enums.h"

typedef struct {
  FILE* input;
  token_t current_tok;
  char ident[512];
  int int_val;
} tokenizer_t;

char* token_to_string(tokenizer_t* tokenizer);

expr_node_t* parse_file(FILE *input);

#endif
