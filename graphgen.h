
#ifndef GRAPHGEN_H

#define GRAPHGEN_H

#include "list.h"
#include "context.h"
#include "ast.h"

typedef struct {
  unsigned int id_counter;
  unsigned int rank_counter;
  list_t* vertices;
  list_t* edges;
} graph_t;

typedef struct {
  unsigned int id;
  char* label;
  unsigned int rank;
} graph_vertex_t;

typedef struct {
  graph_vertex_t* start;
  graph_vertex_t* end;
} graph_edge_t;

graph_vertex_t* graphgen_expr(graph_t* builder, expr_node_t* node);

graph_vertex_t* graphgen_expr_list(graph_t* builder, expr_list_node_t* node);

graph_vertex_t* graphgen_const_int(graph_t* builder, const_int_node_t* node);

graph_vertex_t* graphgen_const_float(graph_t* builder, const_float_node_t* node);

graph_vertex_t* graphgen_const_bool(graph_t* builder, const_bool_node_t* node);

graph_vertex_t* graphgen_ident(graph_t* builder, ident_node_t* node);

graph_vertex_t* graphgen_var_decl(graph_t* builder, var_decl_node_t* node);

graph_vertex_t* graphgen_bin_op(graph_t* builder, bin_op_node_t* node);

graph_vertex_t* graphgen_unary_op(graph_t* builder, unary_op_node_t* node);

graph_vertex_t* graphgen_fun_call(graph_t* builder, fun_call_node_t* node);

graph_vertex_t* graphgen_block(graph_t* builder, block_node_t* node);

graph_vertex_t* graphgen_fun_param(graph_t* builder, fun_param_node_t* node);

char* graphgen(context_t* context, expr_node_t* ast);

#endif
