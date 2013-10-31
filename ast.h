
#ifndef AST_H

#define AST_H

#include "enums.h"

typedef struct {
  node_t node_type;
  void* codegen_fun;
  // TODO - make these more specific
  // LLVMValueRef (*codegen_fun)(LLVMBuilderRef, struct expr_node_t*);
  void* graphgen_fun;
  expr_type_t type;
} expr_node_t;

typedef struct expr_list_node_t {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  expr_node_t* expr;
  struct expr_list_node_t* next;
} expr_list_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  long val;
} const_int_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  double val;
} const_float_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  char* name;
} ident_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  char* name;
} fun_call_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  expr_list_node_t* body;
} block_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  char* name;
  expr_node_t* rhs;
} var_decl_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  unary_op_t op;
  expr_node_t *rhs;
} unary_op_node_t;

typedef struct {
  node_t node_type;
  void* codegen_fun;
  void* graphgen_fun;
  expr_type_t type;
  bin_op_t op;
  expr_node_t *lhs, *rhs;
} bin_op_node_t;

expr_list_node_t* ast_expr_list_node_init(expr_list_node_t* old_list, expr_node_t* new_expr);

const_int_node_t* ast_const_int_node_init(long val);

const_float_node_t* ast_const_float_node_init(double val);

ident_node_t* ast_ident_node_init(char* name);

var_decl_node_t* ast_var_decl_node_init(char* name, expr_node_t* rhs);

bin_op_node_t* ast_bin_op_node_init(bin_op_t op, expr_node_t* lhs, expr_node_t* rhs);

unary_op_node_t* ast_unary_op_node_init(unary_op_t op, expr_node_t* rhs);

fun_call_node_t* ast_fun_call_node_init(char* name);

block_node_t* ast_block_node_init(expr_list_node_t* fun_body);

#endif
