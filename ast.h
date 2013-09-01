
#ifndef AST_H

#define AST_H

#include "enums.h"

typedef struct {
  int node_type;
  void* codegen_fun;
} expr_node_t;

typedef struct expr_list_node_t {
  int node_type;
  void* codegen_fun;
  expr_node_t* expr;
  struct expr_list_node_t* next;
} expr_list_node_t;

typedef struct {
  int node_type;
  void* codegen_fun;
  int val;
} const_int_node_t;

typedef struct {
  int node_type;
  void* codegen_fun;
  char* name;
} ident_node_t;

typedef struct {
  int node_type;
  void* codegen_fun;
  char* name;
  expr_node_t* rhs;
} var_decl_node_t;

typedef struct {
  int node_type;
  void* codegen_fun;
  unary_op_t op;
  expr_node_t *rhs;
} unary_op_node_t;

typedef struct {
  int node_type;
  void* codegen_fun;
  bin_op_t op;
  expr_node_t *lhs, *rhs;
} bin_op_node_t;

expr_list_node_t* init_expr_list_node(expr_list_node_t* old_list, expr_node_t* new_expr);

const_int_node_t* init_const_int_node(int val);

ident_node_t* init_ident_node(char* name);

var_decl_node_t* init_var_decl_node(char* name, expr_node_t* rhs);

bin_op_node_t* init_bin_op_node(bin_op_t op, expr_node_t* lhs, expr_node_t* rhs);

unary_op_node_t* init_unary_op_node(unary_op_t op, expr_node_t* rhs);

#endif