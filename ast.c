
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "symbols.h"
#include "codegen.h"
#include "graphgen.h"

expr_list_node_t* init_expr_list_node(expr_list_node_t* old_list, expr_node_t* new_expr) {
  expr_list_node_t* node = (expr_list_node_t*)malloc(sizeof(expr_list_node_t));
  node->node_type = NODE_EXPR_LIST;
  node->codegen_fun = codegen_expr_list;
  node->graphgen_fun = graphgen_expr_list;
  node->type = new_expr->type;
  node->expr = new_expr;
  node->next = NULL;
  if (old_list != NULL) {
    expr_list_node_t* iter;
    for (iter = old_list; iter->next; iter = iter->next);
    iter->next = node;
    // set old list type to return type of new_expr
    old_list->type = new_expr->type; 
    return old_list;
  } else {
    return node;
  }
}

const_int_node_t* init_const_int_node(long val) {
  const_int_node_t* node = (const_int_node_t*)malloc(sizeof(const_int_node_t));
  node->node_type = NODE_CONST_INT;
  node->codegen_fun = codegen_const_int;
  node->graphgen_fun = graphgen_const_int;
  node->type = EXPR_TYPE_INT;
  node->val = val;
  return node;
}

const_float_node_t* init_const_float_node(double val) {
  const_float_node_t* node = (const_float_node_t*)malloc(sizeof(const_float_node_t));
  node->node_type = NODE_CONST_FLOAT;
  node->codegen_fun = codegen_const_float;
  node->graphgen_fun = graphgen_const_float;
  node->type = EXPR_TYPE_FLOAT;
  node->val = val;
  return node;
}

ident_node_t* init_ident_node(char* name) {
  ident_node_t* node = (ident_node_t*)malloc(sizeof(ident_node_t));
  node->node_type = NODE_IDENT;
  node->codegen_fun = codegen_ident;
  node->graphgen_fun = graphgen_ident;

  symbol_t* symbol = get_symbol(name);
  if (!symbol) {
    fprintf(stderr, "init_ident_node: Unable to find symbol with name: %s\n", name);
    return NULL;
  }
  node->type = symbol->type;

  node->name = name;
  return node;
}

var_decl_node_t* init_var_decl_node(char* name, expr_node_t* rhs) {
  var_decl_node_t* node = (var_decl_node_t*)malloc(sizeof(var_decl_node_t));
  node->node_type = NODE_VAR_DECL;
  node->codegen_fun = codegen_var_decl;
  node->graphgen_fun = graphgen_var_decl;
  node->type = rhs->type;
  node->name = name;
  node->rhs = rhs;
  return node;
}

bin_op_node_t* init_bin_op_node(bin_op_t op, expr_node_t* lhs, expr_node_t* rhs) {
  bin_op_node_t* node = (bin_op_node_t*)malloc(sizeof(bin_op_node_t));
  node->node_type = NODE_BINARY_OP;
  node->codegen_fun = codegen_bin_op;
  node->graphgen_fun = graphgen_bin_op;
  if (lhs->type == rhs->type) {
    node->type = lhs->type;
  } else if ((lhs->type == EXPR_TYPE_INT || lhs->type == EXPR_TYPE_FLOAT) &&
      (rhs->type == EXPR_TYPE_INT || rhs->type == EXPR_TYPE_FLOAT)) {
    node->type = EXPR_TYPE_FLOAT;
  } else {
    fprintf(stderr, "Unable to determine final type of binary operation\n");
    return NULL;
  }
  node->op = op;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

unary_op_node_t* init_unary_op_node(unary_op_t op, expr_node_t* rhs) {
  unary_op_node_t* node = (unary_op_node_t*)malloc(sizeof(unary_op_node_t));
  node->node_type = NODE_UNARY_OP;
  node->codegen_fun = codegen_unary_op;
  node->graphgen_fun = graphgen_unary_op;
  node->type = rhs->type;
  node->op = op;
  node->rhs = rhs;
  return node;
}

fun_call_node_t* init_fun_call_node(char* name) {
  fun_call_node_t* node = (fun_call_node_t*)malloc(sizeof(fun_call_node_t));
  node->node_type = NODE_FUN_CALL;
  node->codegen_fun = codegen_fun_call;
  node->graphgen_fun = graphgen_fun_call;

  symbol_t* symbol = get_symbol(name);
  if (!symbol) {
    fprintf(stderr, "init_ident_node: Unable to find symbol with name: %s\n", node->name);
    return NULL;
  }
  if (symbol->type != EXPR_TYPE_FUN) {
    fprintf(stderr, "%s is not a function\n", node->name);
    return NULL;
  }

  node->type = symbol->ret_type;
  printf("function call returns: %s\n", type_to_string(node->type));
  node->name = name;
  return node;
}

block_node_t* init_block_node(expr_list_node_t* fun_body) {
  block_node_t* node = (block_node_t*)malloc(sizeof(block_node_t));
  node->node_type = NODE_BLOCK;
  node->codegen_fun = codegen_block;
  node->graphgen_fun = graphgen_block;
  node->type = EXPR_TYPE_FUN;
  node->body = fun_body;
  return node;
}

