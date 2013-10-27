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
  int rank = graph->rank_counter++;
  graph_vertex_t* ret = NULL;
  expr_list_node_t* iter;
  graph_vertex_t* prev = NULL;
  for (iter = node; iter; iter = iter->next) {
    ret = graphgen_expr(graph, iter->expr);
    ret->rank = rank;
    if (prev) {
      graph_edge_init(graph, prev, ret);
    }
    prev = ret;
  }
  return ret;
}

graph_vertex_t* graphgen_const_int(graph_t* graph, const_int_node_t* node) {
  char* label = (char*)malloc(sizeof(char) * 512);
  sprintf(label, "%ld (%s)", node->val, type_to_string(node->type));
  printf("label: %ld\n", node->val);
  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_const_float(graph_t* graph, const_float_node_t* node) {
  char* label = (char*)malloc(sizeof(char) * 512);
  sprintf(label, "%lf (%s)", node->val, type_to_string(node->type));
  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_ident(graph_t* graph, ident_node_t* node) {
  char* label = (char*)malloc(sizeof(char) * 512);
  sprintf(label, "%s (%s)", node->name, type_to_string(node->type));
  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_var_decl(graph_t* graph, var_decl_node_t* node) {
  char* label = (char*)malloc(sizeof(char) * 512);
  sprintf(label, "%s %s (%s)", node_to_string(node->node_type), node->name, type_to_string(node->type));
  graph_vertex_t* var_decl = graph_vertex_init(graph, label);
  graph_vertex_t* rhs = graphgen_expr(graph, node->rhs);
  graph_edge_init(graph, var_decl, rhs);
  return var_decl;
}

graph_vertex_t* graphgen_bin_op(graph_t* graph, bin_op_node_t* node) {
  char* label = (char*)malloc(sizeof(char) * 512);
  sprintf(label, "%s (%s)", bin_op_to_string(node->op), type_to_string(node->type));
  graph_vertex_t* bin_op = graph_vertex_init(graph, label);
  graph_vertex_t* lhs = graphgen_expr(graph, node->lhs);
  graph_vertex_t* rhs = graphgen_expr(graph, node->rhs);
  graph_edge_init(graph, bin_op, lhs);
  graph_edge_init(graph, bin_op, rhs);
  return bin_op;
}

graph_vertex_t* graphgen_unary_op(graph_t* graph, unary_op_node_t* node) {
  char* label = (char*)malloc(sizeof(char) * 512);
  sprintf(label, "%s (%s)", unary_op_to_string(node->op), type_to_string(node->type));
  graph_vertex_t* unary_op = graph_vertex_init(graph, label);
  graph_vertex_t* rhs = graphgen_expr(graph, node->rhs);
  graph_edge_init(graph, unary_op, rhs);
  return unary_op;
}

graph_vertex_t* graphgen_fun_call(graph_t* graph, fun_call_node_t* node) {
  char* label = malloc(sizeof(char) * 512);
  sprintf(label, "%s: %s (%s)", node_to_string(node->node_type), node->name, type_to_string(node->type));
  return graph_vertex_init(graph, label);
}

graph_vertex_t* graphgen_block(graph_t* graph, block_node_t* node) {
  char* label = malloc(sizeof(char) * 512);
  sprintf(label, "%s (%s)", node_to_string(node->node_type), type_to_string(node->type));
  graph_vertex_t* block_vertex = graph_vertex_init(graph, label);
  graph_vertex_t* body_vertex = graphgen_expr_list(graph, node->body);
  graph_edge_init(graph, block_vertex, body_vertex);
  return block_vertex;
}

char* graphgen(expr_node_t* ast) {
  graph_t* graph = (graph_t*)malloc(sizeof(graph_t));
  graph->id_counter = 1;
  graph->rank_counter = 1;
  graph->vertices = list_init();
  graph->edges = list_init();

  graphgen_expr(graph, ast);

  char* ranks = malloc(sizeof(char) * 4096);
  // partition vertices by rank
  list_t** vertices_by_rank = malloc(sizeof(list_t**) * graph->rank_counter);
  for (int i = 0; i < graph->rank_counter; i++) {
    vertices_by_rank[i] = list_init();
  }
  list_item_t* curr;
  for (curr = graph->vertices->head; curr; curr = list_iter(curr)) {
    graph_vertex_t* curr_vertex = curr->val;
    list_t* vertices_for_rank = vertices_by_rank[curr_vertex->rank];
    list_push(vertices_for_rank, curr_vertex);
  }
  // skip rank 0 (unassigned rank)
  for (int i = 1; i < graph->rank_counter; i++) {
    list_t* vertices_for_rank = vertices_by_rank[i];
    if (vertices_for_rank->size > 1) {
      list_item_t* vertex_for_rank;
      char* rank_str = malloc(sizeof(char) * 512);
      char* vertices_for_rank_str = malloc(sizeof(char) * 512);
      for (vertex_for_rank = vertices_for_rank->head;
          vertex_for_rank;
          vertex_for_rank = list_iter(vertex_for_rank)) {
        graph_vertex_t* curr_vertex = vertex_for_rank->val;
        // { rank = same; node1; }
        char* vertex_str = malloc(sizeof(char) * 512);
        sprintf(vertex_str, "node%d;", curr_vertex->id);
        strncat(vertices_for_rank_str, vertex_str, strlen(vertex_str));
      }
      sprintf(rank_str, "\t{ rank = same; %s}\n", vertices_for_rank_str);
      strncat(ranks, rank_str, strlen(rank_str));
    }
  }

  char* vertices = malloc(sizeof(char) * 4096);
  graph_vertex_t* curr_vertex;
  do {
    curr_vertex = list_shift(graph->vertices);
    if (curr_vertex) {
      char* vertex_str = malloc(sizeof(char) * 512);
      sprintf(vertex_str, "\tnode%d[label=\"%s\"];\n", curr_vertex->id, curr_vertex->label);
      if (strlen(vertices) + strlen(vertex_str) >= 4095) {
        fprintf(stderr, "Buffer not big enough for vertices\n");
        exit(1);
      }
      strncat(vertices, vertex_str, strlen(vertex_str));
    }
  } while (curr_vertex);

  char* edges = malloc(sizeof(char) * 4096);
  graph_edge_t* curr_edge;
  do {
    curr_edge = list_shift(graph->edges);
    if (curr_edge) {
      char* edge_str = malloc(sizeof(char) * 512);
      sprintf(edge_str, "\tnode%d -> node%d;\n", curr_edge->start->id, curr_edge->end->id);
      if (strlen(edges) + strlen(edge_str) >= 4095) {
        fprintf(stderr, "Buffer not big enough for edges\n");
        exit(1);
      }
      strncat(edges, edge_str, strlen(edge_str));
    }
  } while (curr_edge);

  char* dot = malloc(sizeof(char) * (strlen(vertices) + strlen(edges) + 256));
  sprintf(dot, "digraph{\n%s\n%s\n%s\n}", ranks, vertices, edges);
  return dot;
}
