
#ifndef AST_H

#define AST_H

#include "enums.h"
#include "list.h"
#include "context.h"
#include "type.h"

typedef struct {
  node_t node_type;
  void* codegen_fun;
  // TODO - make these more specific
  // LLVMValueRef (*codegen_fun)(LLVMBuilderRef, struct expr_node_t*);
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
} expr_node_t;

typedef struct expr_list_node_t {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  list_t* expressions;
} expr_list_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  long val;
} const_int_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  double val;
} const_float_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  char* name;
} ident_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  char* name;
  list_t* params;
} fun_call_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  expr_list_node_t* body;
  list_t* params;
  symbol_table_t* scope;
} block_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  char* name;
  expr_node_t* rhs;
} var_decl_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  unary_op_t op;
  expr_node_t *rhs;
} unary_op_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  bin_op_t op;
  expr_node_t *lhs, *rhs;
} bin_op_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  void* free_fun;
  type_t* type;
  char* name;
} fun_param_node_t;

expr_list_node_t* ast_expr_list_node_init(context_t* context);

expr_list_node_t* ast_expr_list_node_add(context_t* context, expr_list_node_t* list, expr_node_t* expr);

const_int_node_t* ast_const_int_node_init(context_t* context, long val);

const_float_node_t* ast_const_float_node_init(context_t* context, double val);

ident_node_t* ast_ident_node_init(context_t* context, char* name);

var_decl_node_t* ast_var_decl_node_init(context_t* context, char* name, expr_node_t* rhs);

bin_op_node_t* ast_bin_op_node_init(context_t* context, bin_op_t op, expr_node_t* lhs, expr_node_t* rhs);

unary_op_node_t* ast_unary_op_node_init(context_t* context, unary_op_t op, expr_node_t* rhs);

fun_call_node_t* ast_fun_call_node_init(context_t* context, char* name, list_t* params);

block_node_t* ast_block_node_init(context_t* context, list_t* param_list, symbol_table_t* scope, expr_list_node_t* fun_body);

fun_param_node_t* ast_fun_param_node_init(context_t* context, char* name, type_t* type);

void ast_expr_node_free(expr_node_t* node);

#endif
