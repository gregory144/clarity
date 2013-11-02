#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "ast.h"
#include "parse.h"

bin_op_t parse_token_to_bin_op(token_t tok) {
  switch(tok) {
    case TOKEN_PLUS: return BIN_OP_PLUS;
    case TOKEN_DASH: return BIN_OP_MINUS;
    case TOKEN_STAR: return BIN_OP_MULT;
    case TOKEN_FORWARD_SLASH: return BIN_OP_DIV;
    case TOKEN_PERCENT: return BIN_OP_MOD;
    case TOKEN_EQUAL: return BIN_OP_EQ;
    default: return BIN_OP_INVALID;
  }
}

unary_op_t parse_token_to_unary_op(token_t tok) {
  switch(tok) {
    case TOKEN_DASH: return UNARY_OP_NEGATE;
    default: return UNARY_OP_INVALID;
  }
}

bool parse_is_left_associative(bin_op_t bin_op) {
  // ^ is left associative (not yet implemented)
  return true;
}

int parse_binary_precedence(bin_op_t op) {
  switch(op) {
    case BIN_OP_EQ:
      return 2;
    case BIN_OP_PLUS:
    case BIN_OP_MINUS:
      return 3;
    case BIN_OP_MULT:
    case BIN_OP_DIV:
    case BIN_OP_MOD:
      return 5;
    default:
      fprintf(stderr, "Unable to determine binary precedence: unknown operator: %d\n", op);
      return -1;
  }
}

int parse_unary_precedence(unary_op_t op) {
  switch(op) {
    case UNARY_OP_NEGATE:
      return 4;
    default:
      fprintf(stderr, "Unable to determine unary precedence: unknown operator: %d\n", op);
      return -1;
  }
}

char* parse_next_token_to_string(tokenizer_t* tokenizer) {
  char* buf = malloc(sizeof(char) * 512);
  switch(tokenizer->current_tok) {
    case TOKEN_INTEGER:
      sprintf(buf, "%ld", tokenizer->int_val);
      break;
    case TOKEN_FLOAT:
      sprintf(buf, "%f", tokenizer->float_val);
      break;
    case TOKEN_IDENT:
      sprintf(buf, "%s", tokenizer->ident);
      break;
    default:
      sprintf(buf, "%s", token_to_string(tokenizer->current_tok));
      break;
  }
  return buf;
}

token_t parse_get_tok(tokenizer_t* tok) {
	static int c = ' ';
	int i;

	while (isspace(c))
	  c = fgetc(tok->input);

  if (isalpha(c)) { // ident
    tok->ident[i = 0] = c;
    while (isalpha(tok->ident[++i] = fgetc(tok->input)))
      ;
    c = tok->ident[i];
    tok->ident[i] = '\0';

#define TRYMATCH(s, ret) \
if (strcmp(tok->ident, s) == 0) return ret

    return TOKEN_IDENT;
  } else if (isdigit(c) || c == '.') { // number
    char* ident = tok->ident;
		ident[i = 0] = c;
    bool is_float = ident[i] == '.';
		while (isdigit(ident[++i] = fgetc(tok->input))
				|| ident[i] == '.') {
      is_float = is_float || ident[i] == '.';
    }
		c = ident[i];
		ident[i] = '\0';
    if (is_float) {
      tok->float_val = strtod(ident, NULL);
      return TOKEN_FLOAT;
    } else {
      tok->int_val = strtol(ident, NULL, 10);
      return TOKEN_INTEGER;
    }
	} else if (c == '#') { // comment
		while ((c = fgetc(tok->input)) != EOF && c != '\r' && c != '\n')
			;
		if (c != EOF)
			return parse_get_tok(tok);
  }

  if (c == EOF)
    return TOKEN_EOF;

  i = c;
  c = fgetc(tok->input);

  printf("parsing token: %c, %c\n", i, c);

  switch (i) {
    case '+': return TOKEN_PLUS;
    case '-': return TOKEN_DASH;
    case '*': return TOKEN_STAR;
    case '/': return TOKEN_FORWARD_SLASH;
    case '%': return TOKEN_PERCENT;
    case '=': return TOKEN_EQUAL;
    case '(': return TOKEN_OPEN_PAREN;
    case ')': return TOKEN_CLOSE_PAREN;
    case '{': return TOKEN_OPEN_BRACE;
    case '}': return TOKEN_CLOSE_BRACE;
    case ';': return TOKEN_SEMI;
    case ':': return TOKEN_COLON;
  }
  fprintf(stderr, "Unreconized character: %c\n", i);
  return TOKEN_INVALID;
}

token_t parse_get_tok_next(tokenizer_t* tok) {
  return tok->current_tok = parse_get_tok(tok);
}

bool parse_expect(tokenizer_t *tok, token_t expected, char* expected_s) {
  if (!expected || tok->current_tok != expected) {
    char* tok_s = parse_next_token_to_string(tok);
    fprintf(stderr, "Expected: %s, got: %s\n", expected_s, tok_s);
    free(tok_s);
    return false;
  }
  return true;
}

expr_node_t* parse_expression_primary(context_t* context, tokenizer_t *tok, int prec);

expr_node_t* parse_expression_unary(context_t* context, tokenizer_t *tok) {
  unary_op_t op = parse_token_to_unary_op(tok->current_tok);
  int prec = parse_unary_precedence(op);
  parse_get_tok_next(tok);
  expr_node_t* rhs = parse_expression_primary(context, tok, prec);
  return (expr_node_t*)ast_unary_op_node_init(context, op, rhs);
}

expr_node_t* parse_expression_var_decl(context_t* context, tokenizer_t *tok, char* ident) {
  char* type_name = NULL;
  if (tok->current_tok == TOKEN_COLON) {
    parse_get_tok_next(tok);
    if (!parse_expect(tok, TOKEN_IDENT, "type name")) {
      return NULL;
    }
    type_name = strdup(tok->ident);
    parse_get_tok_next(tok);
  }
  if (!parse_expect(tok, TOKEN_EQUAL, "assignment")) {
    return NULL;
  }
  parse_get_tok_next(tok);
  expr_node_t* rhs = parse_expression_primary(context, tok, 0);
  if (!rhs) return NULL;

  symbol_t* symbol = symbol_get(context->symbol_table, ident);
  if (symbol != NULL) {
    fprintf(stderr, "Cannot redeclare variable: %s\n", ident);
    return NULL;
  }
  if (type_name != NULL) {
    type_t* declared_type = type_get(context->type_sys, type_name);
    if (declared_type == NULL) {
      fprintf(stderr, "Type %s for '%s' not recognized\n", type_name, ident);
      return NULL;
    }
    if (!type_equals(rhs->type, declared_type)) {
      fprintf(stderr, "Declaring variable '%s' as %s but setting %s\n", ident, type_to_string(declared_type), type_to_string(rhs->type));
      return NULL;
    }
  }

  printf("declaring %s with type %s\n", ident, type_to_string(rhs->type));
  symbol = symbol_set(context->symbol_table, ident, rhs->type);
  if (type_equals(rhs->type, type_get(context->type_sys, "Function"))) {
    block_node_t* block = (block_node_t*)rhs;
    symbol->ret_type = block->body->type;
  }
  return (expr_node_t*)ast_var_decl_node_init(context, ident, rhs);
}

expr_list_node_t* parse_expression_list(context_t* context, tokenizer_t *tok);

list_t* parse_param_list(context_t* context, tokenizer_t *tok) {
  list_t* params = list_init();
  if (tok->current_tok == TOKEN_CLOSE_PAREN) {
    return params;
  }
  char* ident = strdup(tok->ident);
  list_push(params, ident);
  parse_get_tok_next(tok);
  while (tok->current_tok == TOKEN_COMMA) {
    parse_get_tok_next(tok);
    if (!parse_expect(tok, TOKEN_IDENT, "ident")) {
      return NULL;
    }
    char* ident = strdup(tok->ident);
    list_push(params, ident);
    parse_get_tok_next(tok);
  }
  return params;
}

expr_node_t* parse_expression_block(context_t* context, tokenizer_t *tok) {
  parse_get_tok_next(tok);
  if (!parse_expect(tok, TOKEN_OPEN_PAREN, "(")) {
    return NULL;
  }
  parse_get_tok_next(tok);
  list_t* param_list = parse_param_list(context, tok);
  if (!parse_expect(tok, TOKEN_CLOSE_PAREN, ")")) {
    return NULL;
  }
  parse_get_tok_next(tok);
  expr_list_node_t* function_body = parse_expression_list(context, tok);
  if (!parse_expect(tok, TOKEN_CLOSE_BRACE, "}")) {
    return NULL;
  }
  parse_get_tok_next(tok);
  return (expr_node_t*)ast_block_node_init(context, param_list, function_body);
}

expr_node_t* parse_expression_secondary(context_t* context, tokenizer_t *tok) {
  if (tok->current_tok == TOKEN_OPEN_PAREN) {
    parse_get_tok_next(tok);
    expr_node_t* inner = parse_expression_primary(context, tok, 0);
    if (!parse_expect(tok, TOKEN_CLOSE_PAREN, "')'")) {
      return NULL;
    }
    parse_get_tok_next(tok);
    return inner;
  } else if (tok->current_tok == TOKEN_DASH) { // unary negation
    return parse_expression_unary(context, tok);
  } else if (tok->current_tok == TOKEN_INTEGER) {
    expr_node_t* int_node = (expr_node_t*)ast_const_int_node_init(context, tok->int_val);
    parse_get_tok_next(tok);
    return int_node;
  } else if (tok->current_tok == TOKEN_FLOAT) {
    expr_node_t* float_node = (expr_node_t*)ast_const_float_node_init(context, tok->float_val);
    parse_get_tok_next(tok);
    return float_node;
  } else if (tok->current_tok == TOKEN_IDENT) {
    char* ident = strdup(tok->ident);
    parse_get_tok_next(tok);
    if (tok->current_tok == TOKEN_OPEN_PAREN) {
      // this is a function call
      parse_get_tok_next(tok);
      if (!parse_expect(tok, TOKEN_CLOSE_PAREN, "')'")) {
        return NULL;
      }
      parse_get_tok_next(tok);
      return (expr_node_t*)ast_fun_call_node_init(context, ident);
    } else if (tok->current_tok == TOKEN_EQUAL || tok->current_tok == TOKEN_COLON) {
      return parse_expression_var_decl(context, tok, ident);
    } else {
      return (expr_node_t*)ast_ident_node_init(context, ident);
    }
  } else if (tok->current_tok == TOKEN_OPEN_BRACE) {
    return parse_expression_block(context, tok);
  }
  parse_expect(tok, 0, "unary op, '(', var declaration, function declaration, identifier or an integer");
  return NULL;
}

expr_node_t* parse_expression_primary(context_t* context, tokenizer_t *tok, int prec) {
  expr_node_t* lhs = parse_expression_secondary(context, tok);
  if (!lhs) {
    return NULL;
  }
  bin_op_t bin_op = parse_token_to_bin_op(tok->current_tok);
  int op_prec;
  if (bin_op != BIN_OP_INVALID) op_prec = parse_binary_precedence(bin_op);
  while (bin_op != BIN_OP_INVALID && op_prec >= prec) {
    int new_prec = parse_is_left_associative(bin_op) ? op_prec + 1 : op_prec;
    parse_get_tok_next(tok);
    expr_node_t* rhs = parse_expression_primary(context, tok, new_prec);
    if (!rhs) return NULL;
    lhs = (expr_node_t*)ast_bin_op_node_init(context, bin_op, lhs, rhs);
    if (!lhs) return NULL;
    bin_op = parse_token_to_bin_op(tok->current_tok);
    if (bin_op != BIN_OP_INVALID) op_prec = parse_binary_precedence(bin_op);
  }
  return lhs;
}

/*
 * E --> Exp(0) 
 * Exp(p) --> P {B Exp(q)} 
 * P --> U Exp(q) | "(" E ")" | v
 * B --> "+" | "-"  | "*" |"/" | "^" | "||" | "&&" | "="
 * U --> "-"
 */
expr_node_t* parse_expression(context_t* context, tokenizer_t *tok) {
  return parse_expression_primary(context, tok, 0);
}

expr_list_node_t* parse_expression_list(context_t* context, tokenizer_t *tok) {
  expr_list_node_t* expr_list = NULL;
  printf("next tok '%d'\n", tok->current_tok);
  while (tok->current_tok != TOKEN_EOF) {
    if (tok->current_tok == TOKEN_CLOSE_BRACE) {
      printf("brace\n");
      return expr_list;
    }
    printf("next expr\n");
    expr_node_t* next_expr = parse_expression(context, tok);
    if (!next_expr) return NULL;
    expr_list = (expr_list_node_t*)ast_expr_list_node_init(context, expr_list, next_expr);
    if (tok->current_tok != TOKEN_SEMI && tok->current_tok != TOKEN_EOF) {
      parse_expect(tok, 0, "; or EOF");
      return NULL;
    }
    parse_get_tok_next(tok);
  }
  return expr_list;
}

expr_node_t* parse_file(context_t* context, FILE *input) {
  tokenizer_t tokenizer;
  tokenizer.input = input;

  parse_get_tok_next(&tokenizer);
  expr_node_t* ast = (expr_node_t*)parse_expression_list(context, &tokenizer);
  if (!ast) {
    fprintf(stderr, "No expression parsed\n");
    return NULL;
  }
  return ast;
}
