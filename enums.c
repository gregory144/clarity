#include "enums.h"
#include "stdlib.h"
#include "stdio.h"

char* type_to_string(expr_type_t type) {
  char* buf = malloc(sizeof(char) * 512);
  switch(type) {
    case EXPR_TYPE_INT:
      sprintf(buf, "int");
      break;
    case EXPR_TYPE_FLOAT:
      sprintf(buf, "float");
      break;
    case EXPR_TYPE_FUN:
      sprintf(buf, "function");
      break;
    case EXPR_TYPE_INVALID:
    default:
      sprintf(buf, "invalid type");
      break;
  }
  return buf;
}

char* token_to_string(token_t token) {
  char* buf = malloc(sizeof(char) * 512);
  switch(token) {
    case TOKEN_INTEGER:
      sprintf(buf, "int");
      break;
    case TOKEN_FLOAT:
      sprintf(buf, "float");
      break;
    case TOKEN_IDENT:
      sprintf(buf, "ident");
      break;
    case TOKEN_VAR_DECL:
      sprintf(buf, "var");
      break;
    case TOKEN_EOF:
      sprintf(buf, "EOF");
      break;
    case TOKEN_PLUS:
      sprintf(buf, "+");
      break;
    case TOKEN_DASH:
      sprintf(buf, "-");
      break;
    case TOKEN_STAR:
      sprintf(buf, "*");
      break;
    case TOKEN_FORWARD_SLASH:
      sprintf(buf, "/");
      break;
    case TOKEN_PERCENT:
      sprintf(buf, "%%");
      break;
    case TOKEN_EQUAL:
      sprintf(buf, "=");
      break;
    case TOKEN_OPEN_PAREN:
      sprintf(buf, "(");
      break;
    case TOKEN_CLOSE_PAREN:
      sprintf(buf, ")");
      break;
    case TOKEN_OPEN_BRACE:
      sprintf(buf, "{");
      break;
    case TOKEN_CLOSE_BRACE:
      sprintf(buf, "}");
      break;
    case TOKEN_SEMI:
      sprintf(buf, ";");
      break;
    case TOKEN_INVALID:
    default:
      sprintf(buf, "invalid token");
      break;
  }
  return buf;
}

char* node_to_string(node_t node) {
  char* buf = malloc(sizeof(char) * 512);
  switch(node) {
    case NODE_BINARY_OP:
      sprintf(buf, "binary operation");
      break;
    case NODE_CONST_FLOAT:
      sprintf(buf, "const float");
      break;
    case NODE_CONST_INT:
      sprintf(buf, "const int");
      break;
    case NODE_EXPR_LIST:
      sprintf(buf, "expression list");
      break;
    case NODE_FUN_CALL:
      sprintf(buf, "function call");
      break;
    case NODE_BLOCK:
      sprintf(buf, "block");
      break;
    case NODE_IDENT:
      sprintf(buf, "ident");
      break;
    case NODE_UNARY_OP:
      sprintf(buf, "unary operation");
      break;
    case NODE_VAR_DECL:
      sprintf(buf, "declaration");
      break;
    case NODE_INVALID:
    default:
      sprintf(buf, "invalid node");
      break;
  }
  return buf;
}

char* bin_op_to_string(bin_op_t op) {
  char* buf = malloc(sizeof(char) * 512);
  switch(op) {
    case BIN_OP_DIV:
      sprintf(buf, "/");
      break;
    case BIN_OP_EQ:
      sprintf(buf, "=");
      break;
    case BIN_OP_MINUS:
      sprintf(buf, "-");
      break;
    case BIN_OP_MOD:
      sprintf(buf, "%%");
      break;
    case BIN_OP_MULT:
      sprintf(buf, "*");
      break;
    case BIN_OP_PLUS:
      sprintf(buf, "+");
      break;
    case BIN_OP_INVALID:
    default:
      sprintf(buf, "invalid binop");
      break;
  }
  return buf;
}

char* unary_op_to_string(unary_op_t op) {
  char* buf = malloc(sizeof(char) * 512);
  switch(op) {
    case UNARY_OP_NEGATE:
      sprintf(buf, "-");
      break;
    case UNARY_OP_INVALID:
    default:
      sprintf(buf, "invalid unary op");
      break;
  }
  return buf;
}
