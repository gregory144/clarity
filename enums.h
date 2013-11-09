
#ifndef ENUMS_H

#define ENUMS_H

#include "enums.h"

typedef enum {
  NODE_INVALID,
  NODE_BINARY_OP,
  NODE_CONST_FLOAT,
  NODE_CONST_INT,
  NODE_EXPR_LIST,
  NODE_FUN_CALL,
  NODE_FUN_PARAM,
  NODE_BLOCK,
  NODE_IDENT,
  NODE_UNARY_OP,
  NODE_VAR_DECL,
} node_t;

typedef enum {
  TOKEN_INVALID,
  TOKEN_CLOSE_BRACE,
  TOKEN_CLOSE_PAREN,
  TOKEN_DASH,
  TOKEN_EOF,
  TOKEN_EQUAL,
  TOKEN_FLOAT,
  TOKEN_FORWARD_SLASH,
  TOKEN_IDENT,
  TOKEN_INTEGER,
  TOKEN_OPEN_BRACE,
  TOKEN_OPEN_PAREN,
  TOKEN_PERCENT,
  TOKEN_PLUS,
  TOKEN_SEMI,
  TOKEN_STAR,
  TOKEN_COMMA,
  TOKEN_COLON,
} token_t;

typedef enum {
  BIN_OP_INVALID,
  BIN_OP_DIV,
  BIN_OP_EQ,
  BIN_OP_MINUS,
  BIN_OP_MOD,
  BIN_OP_MULT,
  BIN_OP_PLUS,
} bin_op_t;

typedef enum {
  UNARY_OP_INVALID,
  UNARY_OP_NEGATE,
} unary_op_t;

char* token_to_string(token_t);

char* node_to_string(node_t);

char* bin_op_to_string(bin_op_t);

char* unary_op_to_string(unary_op_t);

#endif
