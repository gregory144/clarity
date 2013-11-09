#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "list.h"
#include "graphgen.h"

graph_vertex_t* graph_vertex_init(graph_t* graph, char* label) {
  graph_vertex_t* vertex = (graph_vertex_t*)malloc(sizeof(graph_vertex_t));
  vertex->id = graph->id_counter++;
  vertex->label = label;
  vertex->rank = 0; // unassigned
  list_push(graph->vertices, vertex);
  return vertex;
}

void graph_vertex_free(graph_vertex_t* vertex) {
  free(vertex->label);
  free(vertex);
}

graph_edge_t* graph_edge_init(graph_t* graph, graph_vertex_t* start, graph_vertex_t* end) {
  graph_edge_t* edge = (graph_edge_t*)malloc(sizeof(graph_edge_t));
  edge->start = start;
  edge->end = end;
  list_push(graph->edges, edge);
  return edge;
}

graph_vertex_t* graphgen_expr(graph_t* graph, expr_node_t* node) {
  graph_vertex_t* (*fun)() = node->graphgen_fun;
  return fun(graph, node);
}

graph_vertex_t* graphgen_expr_list(graph_t* graph, expr_list_node_t* node) {
  unsigned int rank = graph->rank_counter++;
  graph_vertex_t* ret = NULL;
  graph_vertex_t* prev = NULL;
  list_item_t* iter = list_iter_init(node->expressions);
  for (; iter; iter = list_iter(iter)) {
    ret = graphgen_expr(graph, (expr_node_t*)iter->val);
    ret->rank = rank;
    if (prev) {
      graph_edge_init(graph, prev, ret);
    }
    prev = ret;
  }
  return ret;
}

graph_vertex_t* graphgen_const_int(graph_t* graph, const_int_node_t* node) {
  char* label = (char*)malloc(sizeof(char) * 4096);
  sprintf(label, "%ld (%s)", node->val, type_to_string(node->type));
  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_const_float(graph_t* graph, const_float_node_t* node) {
  char* label = (char*)malloc(sizeof(char) * 4096);
  sprintf(label, "%lf (%s)", node->val, type_to_string(node->type));

  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_ident(graph_t* graph, ident_node_t* node) {
  const char* format_str = "%s (%s)";
  char* type_str = type_to_string(node->type);
  char* label = malloc(sizeof(char) * (strlen(format_str) - 4 + strlen(node->name) + strlen(type_str) + 1));
  sprintf(label, "%s (%s)", node->name, type_str);

  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_var_decl(graph_t* graph, var_decl_node_t* node) {
  const char* format_str = "%s %s (%s)";
  char* node_str = node_to_string(node->node_type);
  char* type_str = type_to_string(node->type);
  size_t str_length = strlen(format_str) - 6 + strlen(node_str) + strlen(node->name) + strlen(type_str) + 1;
  char* label = malloc(sizeof(char) * str_length);
  sprintf(label, format_str, node_str, node->name, type_str);
  free(node_str);

  graph_vertex_t* var_decl = graph_vertex_init(graph, label);
  graph_vertex_t* rhs = graphgen_expr(graph, node->rhs);
  graph_edge_init(graph, var_decl, rhs);
  return var_decl;
}

graph_vertex_t* graphgen_bin_op(graph_t* graph, bin_op_node_t* node) {
  const char* format_str = "%s (%s)";
  char* op_str = bin_op_to_string(node->op);
  char* type_str = type_to_string(node->type);
  char* label = malloc(sizeof(char) * (strlen(format_str) - 4 + strlen(op_str) + strlen(type_str) + 1));
  sprintf(label, format_str, op_str, type_str);
  free(op_str);

  graph_vertex_t* bin_op = graph_vertex_init(graph, label);
  graph_vertex_t* lhs = graphgen_expr(graph, node->lhs);
  graph_vertex_t* rhs = graphgen_expr(graph, node->rhs);
  graph_edge_init(graph, bin_op, lhs);
  graph_edge_init(graph, bin_op, rhs);
  return bin_op;
}

graph_vertex_t* graphgen_unary_op(graph_t* graph, unary_op_node_t* node) {
  const char* format_str = "%s (%s)";
  char* op_str = unary_op_to_string(node->op);
  char* type_str = type_to_string(node->type);
  char* label = malloc(sizeof(char) * (strlen(format_str) - 4 + strlen(op_str) + strlen(type_str) + 1));
  sprintf(label, format_str, op_str, type_str);
  free(op_str);

  graph_vertex_t* unary_op = graph_vertex_init(graph, label);
  graph_vertex_t* rhs = graphgen_expr(graph, node->rhs);
  graph_edge_init(graph, unary_op, rhs);
  return unary_op;
}

graph_vertex_t* graphgen_fun_call(graph_t* graph, fun_call_node_t* node) {
  const char* format_str = "%s: %s (%s)";
  char* node_str = node_to_string(node->node_type);
  char* type_str = type_to_string(node->type);
  size_t str_length = strlen(format_str) - 6 + strlen(node_str) + strlen(node->name) + strlen(type_str) + 1;
  char* label = malloc(sizeof(char) * str_length);
  sprintf(label, format_str, node_str, node->name, type_str);
  free(node_str);

  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_fun_param(graph_t* graph, fun_param_node_t* node) {
  const char* format_str = "param: %s (%s)";
  char* type_str = type_to_string(node->type);
  char* label = malloc(sizeof(char) * (strlen(format_str) - 4 + strlen(type_str) + strlen(node->name) + 1));
  sprintf(label, format_str, node->name, type_str);

  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_block(graph_t* graph, block_node_t* node) {
  const char* format_str = "%s (%s)";
  char* type_str = type_to_string(node->type);
  char* node_str = node_to_string(node->node_type);
  char* label = malloc(sizeof(char) * (strlen(format_str) - 4 + strlen(type_str) + strlen(node_str) + 1));
  sprintf(label, format_str, node_str, type_str);
  free(node_str);

  graph_vertex_t* block_vertex = graph_vertex_init(graph, label);
  graph_vertex_t* body_vertex = graphgen_expr_list(graph, node->body);
  graph_edge_init(graph, block_vertex, body_vertex);

  // params
  unsigned int rank = graph->rank_counter++;
  graph_vertex_t* param_vertex = NULL;
  list_item_t* iter = list_iter_init(node->params);
  graph_vertex_t* prev = block_vertex;
  for (; iter; iter = list_iter(iter)) {
    fun_param_node_t* param = iter->val;
    param_vertex = graphgen_expr(graph, (expr_node_t*)param);
    param_vertex->rank = rank;
    if (prev) {
      graph_edge_init(graph, prev, param_vertex);
    }
    prev = param_vertex;
  }
  return block_vertex;
}

char* graphgen(context_t* context, expr_node_t* ast) {
  graph_t* graph = (graph_t*)malloc(sizeof(graph_t));
  graph->id_counter = 1;
  graph->rank_counter = 1;
  graph->vertices = list_init();
  graph->edges = list_init();

  graphgen_expr(graph, ast);

  // generate representation of graph in DOT format

  // ranks keep some vertices on the same level (horizontally)
  // (like expression lists, param lists)
  char* ranks = malloc(sizeof(char) * 4096);
  sprintf(ranks, "");
  // partition vertices by rank
  list_t** vertices_by_rank = malloc(sizeof(list_t**) * graph->rank_counter);
  for (unsigned int i = 0; i < graph->rank_counter; i++) {
    vertices_by_rank[i] = list_init();
  }
  list_item_t* rank_vertex_iter = list_iter_init(graph->vertices);
  for (; rank_vertex_iter; rank_vertex_iter = list_iter(rank_vertex_iter)) {
    graph_vertex_t* curr_vertex = rank_vertex_iter->val;
    list_t* vertices_for_rank = vertices_by_rank[curr_vertex->rank];
    list_push(vertices_for_rank, curr_vertex);
  }
  // skip rank 0 (unassigned rank)
  for (unsigned int i = 1; i < graph->rank_counter; i++) {
    list_t* vertices_for_rank = vertices_by_rank[i];
    if (vertices_for_rank->size > 1) {
      char* rank_str = malloc(sizeof(char) * 512);
      char* vertices_for_rank_str = malloc(sizeof(char) * 512);
      sprintf(vertices_for_rank_str, "");
      list_item_t* vertex_for_rank_iter = list_iter_init(vertices_for_rank);;
      for (; vertex_for_rank_iter;
          vertex_for_rank_iter = list_iter(vertex_for_rank_iter)) {
        graph_vertex_t* curr_vertex = vertex_for_rank_iter->val;

        char* vertex_str = malloc(sizeof(char) * 512);
        sprintf(vertex_str, "node%d;", curr_vertex->id);
        strncat(vertices_for_rank_str, vertex_str, strlen(vertex_str));
        free(vertex_str);
      }
      sprintf(rank_str, "\t{ rank = same; %s}\n", vertices_for_rank_str);
      free(vertices_for_rank_str);
      strncat(ranks, rank_str, strlen(rank_str));
      free(rank_str);
    }
  }
  for (unsigned int i = 0; i < graph->rank_counter; i++) {
    list_free(vertices_by_rank[i]);
  }
  free(vertices_by_rank);

  // add vertices
  char* vertices = malloc(sizeof(char) * 4096);
  sprintf(vertices, "");
  list_item_t* vertex_iter = list_iter_init(graph->vertices);
  for (; vertex_iter; vertex_iter = list_iter(vertex_iter)) {
    graph_vertex_t* curr_vertex = vertex_iter->val;
    char* vertex_str = malloc(sizeof(char) * 512);
    sprintf(vertex_str, "\tnode%d[label=\"%s\"];\n", curr_vertex->id, curr_vertex->label);
    if (strlen(vertices) + strlen(vertex_str) >= 4095) {
      fprintf(stderr, "Buffer not big enough for vertices\n");
      exit(1);
    }
    strncat(vertices, vertex_str, strlen(vertex_str));
    free(vertex_str);
  }

  // add edges
  char* edges = malloc(sizeof(char) * 4096);
  sprintf(edges, "");
  list_item_t* edge_iter = list_iter_init(graph->edges);
  for (; edge_iter; edge_iter = list_iter(edge_iter)) {
    graph_edge_t* curr_edge = edge_iter->val;
    char* edge_str = malloc(sizeof(char) * 512);
    sprintf(edge_str, "\tnode%d -> node%d;\n", curr_edge->start->id, curr_edge->end->id);
    if (strlen(edges) + strlen(edge_str) >= 4095) {
      fprintf(stderr, "Buffer not big enough for edges\n");
      exit(1);
    }
    strncat(edges, edge_str, strlen(edge_str));
    free(edge_str);
  }

  char* dot = malloc(sizeof(char) * (strlen(vertices) + strlen(edges) + 256));
  sprintf(dot, "digraph{\n%s\n%s\n%s\n}", ranks, vertices, edges);
  free(ranks);
  free(vertices);
  free(edges);
  list_visit(graph->vertices, (void(*)(void*))graph_vertex_free);
  list_free(graph->vertices);
  list_visit(graph->edges, free);
  list_free(graph->edges);
  free(graph);
  return dot;
}

