
#ifndef ENUMS_H

#define ENUMS_H

#include "enums.h"

typedef enum {
  NODE_BINARY_OP, NODE_UNARY_OP, NODE_CONST_INT, NODE_IDENT,
  NODE_VAR_DECL, NODE_EXPR_LIST
} node_t;

typedef enum {
  TOKEN_INVALID, TOKEN_INTEGER, TOKEN_IDENT, TOKEN_VAR_DECL, TOKEN_EOF,
  TOKEN_PLUS, TOKEN_DASH, TOKEN_STAR, TOKEN_FORWARD_SLASH,
  TOKEN_PERCENT, TOKEN_EQUAL, TOKEN_OPEN_PAREN, TOKEN_CLOSE_PAREN,
  TOKEN_SEMI
} token_t;

typedef enum {
  BIN_OP_INVALID, BIN_OP_PLUS, BIN_OP_MINUS, BIN_OP_MULT, BIN_OP_DIV,
  BIN_OP_MOD, BIN_OP_EQ
} bin_op_t;

typedef enum {
  UNARY_OP_INVALID, UNARY_OP_NEGATE
} unary_op_t;


#endif
