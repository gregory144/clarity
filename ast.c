
#include <stdlib.h>

#include "ast.h"
#include "codegen.h"

expr_list_node_t* init_expr_list_node(expr_list_node_t* old_list, expr_node_t* new_expr) {
  expr_list_node_t* node = (expr_list_node_t*)malloc(sizeof(expr_list_node_t));
  node->node_type = NODE_EXPR_LIST;
  node->codegen_fun = codegen_expr_list;
  node->expr = new_expr;
  node->next = NULL;
  if (old_list != NULL) {
    expr_list_node_t* iter;
    for (iter = old_list; iter->next; iter = iter->next);
    iter->next = node;
    return old_list;
  } else {
    return node;
  }
}

const_int_node_t* init_const_int_node(int val) {
  const_int_node_t* node = (const_int_node_t*)malloc(sizeof(const_int_node_t));
  node->node_type = NODE_CONST_INT;
  node->codegen_fun = codegen_const_int;
  node->val = val;
  return node;
}

ident_node_t* init_ident_node(char* name) {
  ident_node_t* node = (ident_node_t*)malloc(sizeof(ident_node_t));
  node->node_type = NODE_IDENT;
  node->codegen_fun = codegen_ident;
  node->name = name;
  return node;
}

var_decl_node_t* init_var_decl_node(char* name, expr_node_t* rhs) {
  var_decl_node_t* node = (var_decl_node_t*)malloc(sizeof(var_decl_node_t));
  node->node_type = NODE_VAR_DECL;
  node->codegen_fun = codegen_var_decl;
  node->name = name;
  node->rhs = rhs;
  return node;
}

bin_op_node_t* init_bin_op_node(bin_op_t op, expr_node_t* lhs, expr_node_t* rhs) {
  bin_op_node_t* node = (bin_op_node_t*)malloc(sizeof(bin_op_node_t));
  node->node_type = NODE_BINARY_OP;
  node->codegen_fun = codegen_bin_op;
  node->op = op;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

unary_op_node_t* init_unary_op_node(unary_op_t op, expr_node_t* rhs) {
  unary_op_node_t* node = (unary_op_node_t*)malloc(sizeof(unary_op_node_t));
  node->node_type = NODE_UNARY_OP;
  node->codegen_fun = codegen_unary_op;
  node->op = op;
  node->rhs = rhs;
  return node;
}

