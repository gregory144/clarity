
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "symbol.h"
#include "codegen.h"
#include "graphgen.h"
#include "list.h"

void ast_expr_node_free(expr_node_t* node) {
  if (node->free_fun == NULL) {
    return free(node);
  }
  void (*fun)(void*) = node->free_fun;
  return fun(node);
}

void ast_expr_list_node_free(expr_list_node_t* node) {
  list_visit(node->expressions, (void(*)(void*))ast_expr_node_free);
  list_free(node->expressions);
  free(node);
}

expr_list_node_t* ast_expr_list_node_init(context_t* context) {
  expr_list_node_t* node = (expr_list_node_t*)malloc(sizeof(expr_list_node_t));
  node->node_type = NODE_EXPR_LIST;
  node->codegen_fun = codegen_expr_list;
  node->graphgen_fun = graphgen_expr_list;
  node->free_fun = ast_expr_list_node_free;
  node->type = NULL;
  node->expressions = list_init();
  return node;
}

expr_list_node_t* ast_expr_list_node_add(context_t* context, expr_list_node_t* node, expr_node_t* expr) {
  list_push(node->expressions, expr);
  return node;
}

const_int_node_t* ast_const_int_node_init(context_t* context, long val) {
  const_int_node_t* node = (const_int_node_t*)malloc(sizeof(const_int_node_t));
  node->node_type = NODE_CONST_INT;
  node->codegen_fun = codegen_const_int;
  node->graphgen_fun = graphgen_const_int;
  node->free_fun = NULL;
  node->type = type_get(context->type_sys, "Integer");
  node->val = val;
  return node;
}

const_float_node_t* ast_const_float_node_init(context_t* context, double val) {
  const_float_node_t* node = (const_float_node_t*)malloc(sizeof(const_float_node_t));
  node->node_type = NODE_CONST_FLOAT;
  node->codegen_fun = codegen_const_float;
  node->graphgen_fun = graphgen_const_float;
  node->free_fun = NULL;
  node->type = type_get(context->type_sys, "Float");
  node->val = val;
  return node;
}

void ast_ident_node_free(ident_node_t* node) {
  free(node->name);
  free(node);
}

ident_node_t* ast_ident_node_init(context_t* context, char* name) {
  ident_node_t* node = malloc(sizeof(ident_node_t));
  node->node_type = NODE_IDENT;
  node->codegen_fun = codegen_ident;
  node->graphgen_fun = graphgen_ident;
  node->free_fun = ast_ident_node_free;

  symbol_t* symbol = symbol_get(context->symbol_table, name);
  if (!symbol) {
    fprintf(stderr, "ast_ident_node_init: Unable to find symbol with name: %s\n", name);
    return NULL;
  }
  node->type = symbol->type;

  node->name = strdup(name);
  return node;
}

void ast_var_decl_node_free(var_decl_node_t* node) {
  ast_expr_node_free(node->rhs);
  free(node->name);
  free(node);
}

var_decl_node_t* ast_var_decl_node_init(context_t* context, char* name, expr_node_t* rhs) {
  var_decl_node_t* node = (var_decl_node_t*)malloc(sizeof(var_decl_node_t));
  node->node_type = NODE_VAR_DECL;
  node->codegen_fun = codegen_var_decl;
  node->graphgen_fun = graphgen_var_decl;
  node->free_fun = ast_var_decl_node_free;
  node->type = rhs->type;
  node->name = strdup(name);
  node->rhs = rhs;
  return node;
}

void ast_bin_op_node_free(bin_op_node_t* node) {
  ast_expr_node_free(node->lhs);
  ast_expr_node_free(node->rhs);
  free(node);
}

bin_op_node_t* ast_bin_op_node_init(context_t* context, bin_op_t op, expr_node_t* lhs, expr_node_t* rhs) {
  bin_op_node_t* node = (bin_op_node_t*)malloc(sizeof(bin_op_node_t));
  node->node_type = NODE_BINARY_OP;
  node->codegen_fun = codegen_bin_op;
  node->graphgen_fun = graphgen_bin_op;
  node->free_fun = ast_bin_op_node_free;
  type_t* type_int = type_get(context->type_sys, "Integer");
  type_t* type_float = type_get(context->type_sys, "Float");
  if (type_equals(lhs->type, rhs->type)) {
    node->type = lhs->type;
  } else if ((type_equals(lhs->type, type_int) || type_equals(lhs->type, type_float)) &&
      (type_equals(rhs->type, type_int) || type_equals(rhs->type, type_float))) {
    node->type = type_float;
  } else {
    fprintf(stderr, "Unable to determine final type of binary operation\n");
    return NULL;
  }
  node->op = op;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

void ast_unary_op_node_free(unary_op_node_t* node) {
  ast_expr_node_free(node->rhs);
  free(node);
}

unary_op_node_t* ast_unary_op_node_init(context_t* context, unary_op_t op, expr_node_t* rhs) {
  unary_op_node_t* node = (unary_op_node_t*)malloc(sizeof(unary_op_node_t));
  node->node_type = NODE_UNARY_OP;
  node->codegen_fun = codegen_unary_op;
  node->graphgen_fun = graphgen_unary_op;
  node->free_fun = ast_unary_op_node_free;
  node->type = rhs->type;
  node->op = op;
  node->rhs = rhs;
  return node;
}

void ast_fun_call_node_free(fun_call_node_t* node) {
  free(node->name);
  free(node);
}

fun_call_node_t* ast_fun_call_node_init(context_t* context, char* name, list_t* params) {
  fun_call_node_t* node = (fun_call_node_t*)malloc(sizeof(fun_call_node_t));
  node->node_type = NODE_FUN_CALL;
  node->codegen_fun = codegen_fun_call;
  node->graphgen_fun = graphgen_fun_call;
  node->free_fun = ast_fun_call_node_free;

  symbol_t* symbol = symbol_get(context->symbol_table, name);
  if (!symbol) {
    fprintf(stderr, "ast_fun_call_node_init: Unable to find symbol with name: %s\n", node->name);
    return NULL;
  }
  if (!type_equals(symbol->type, type_get(context->type_sys, "Function"))) {
    fprintf(stderr, "%s is not a function\n", node->name);
    return NULL;
  }
  if (symbol->num_params != params->size) {
    fprintf(stderr, "wrong number of parameters calling %s. Expected %zd, got %zd\n", node->name, symbol->num_params, params->size);
    return NULL;
  }

  node->type = symbol->ret_type;
  printf("function call returns: %s\n", type_to_string(node->type));
  node->name = strdup(name);
  node->params = params;
  return node;
}

void ast_block_node_free(block_node_t* node) {
  ast_expr_list_node_free(node->body);
  list_visit(node->params, (void(*)(void*))ast_expr_node_free);
  list_free(node->params);
  symbol_table_free(node->scope);
  free(node);
}

block_node_t* ast_block_node_init(context_t* context, list_t* param_list, symbol_table_t* scope, expr_list_node_t* fun_body) {
  block_node_t* node = (block_node_t*)malloc(sizeof(block_node_t));
  node->node_type = NODE_BLOCK;
  node->codegen_fun = codegen_block;
  node->graphgen_fun = graphgen_block;
  node->free_fun = ast_block_node_free;
  node->type = type_get(context->type_sys, "Function");
  node->body = fun_body;
  node->params = param_list;
  node->scope = scope;
  return node;
}

void ast_fun_param_node_free(fun_param_node_t* param) {
  free(param->name);
  free(param);
}

fun_param_node_t* ast_fun_param_node_init(context_t* context, char* name, type_t* type) {
  fun_param_node_t* node = malloc(sizeof(fun_param_node_t));
  node->node_type = NODE_FUN_PARAM;
  node->codegen_fun = codegen_fun_param;
  node->graphgen_fun = graphgen_fun_param;
  node->free_fun = ast_fun_param_node_free;
  node->type = type;
  node->name = name;
  return node;
}
