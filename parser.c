#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include <ctype.h>

#include "codegen.h"
#include "ast.h"
#include "parser.h"
#include "symbols.h"

bin_op_t token_to_bin_op(token_t tok) {
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

unary_op_t token_to_unary_op(token_t tok) {
  switch(tok) {
    case TOKEN_DASH: return UNARY_OP_NEGATE;
    default: return UNARY_OP_INVALID;
  }
}

bool left_associative(bin_op_t bin_op) {
  // ^ is left associative (not yet implemented)
  return true;
}

int binary_precedence(bin_op_t op) {
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

int unary_precedence(unary_op_t op) {
  switch(op) {
    case UNARY_OP_NEGATE:
      return 4;
    default:
      fprintf(stderr, "Unable to determine unary precedence: unknown operator: %d\n", op);
      return -1;
  }
}

char* token_to_string(tokenizer_t* tokenizer) {
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
    case TOKEN_VAR_DECL:
      sprintf(buf, "var");
      break;
    case TOKEN_EOF:
      sprintf(buf, "EOF");
      break;
    case TOKEN_PLUS:
      sprintf(buf, "+");
      break;
    case TOKEN_DASH:
      sprintf(buf, "-");
      break;
    case TOKEN_STAR:
      sprintf(buf, "*");
      break;
    case TOKEN_FORWARD_SLASH:
      sprintf(buf, "/");
      break;
    case TOKEN_PERCENT:
      sprintf(buf, "%%");
      break;
    case TOKEN_EQUAL:
      sprintf(buf, "=");
      break;
    case TOKEN_OPEN_PAREN:
      sprintf(buf, "(");
      break;
    case TOKEN_CLOSE_PAREN:
      sprintf(buf, ")");
      break;
    case TOKEN_SEMI:
      sprintf(buf, ";");
      break;
    case TOKEN_INVALID:
      sprintf(buf, "invalid token");
      break;
  }
  return buf;
}

token_t get_tok(tokenizer_t* tok) {
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
    TRYMATCH("var", TOKEN_VAR_DECL);

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
			return get_tok(tok);
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
    case ';': return TOKEN_SEMI;
  }
  fprintf(stderr, "Unreconized character: %c\n", i);
  return TOKEN_INVALID;
}

token_t get_tok_next(tokenizer_t* tok) {
  return tok->current_tok = get_tok(tok);
}

bool expect(tokenizer_t *tok, token_t expected, char* expected_s) {
  if (!expected || tok->current_tok != expected) {
    char* tok_s = token_to_string(tok);
    fprintf(stderr, "Expected: %s, got: %s\n", expected_s, tok_s);
    free(tok_s);
    return false;
  }
  return true;
}

expr_node_t* parse_expression_primary(tokenizer_t *tok, int prec);

expr_node_t* parse_expression_secondary(tokenizer_t *tok) {
  if (tok->current_tok == TOKEN_DASH) { // unary negation
    unary_op_t op = token_to_unary_op(tok->current_tok);
    int prec = unary_precedence(op);
    get_tok_next(tok);
    expr_node_t* rhs = parse_expression_primary(tok, prec);
    return (expr_node_t*)init_unary_op_node(op, rhs);
  } else if (tok->current_tok == TOKEN_OPEN_PAREN) {
    get_tok_next(tok);
    expr_node_t* inner = parse_expression_primary(tok, 0);
    if (!expect(tok, TOKEN_CLOSE_PAREN, "')'")) {
      return NULL;
    }
    get_tok_next(tok);
    return inner;
  } else if (tok->current_tok == TOKEN_INTEGER) {
    expr_node_t* int_node = (expr_node_t*)init_const_int_node(tok->int_val);
    get_tok_next(tok);
    return int_node;
  } else if (tok->current_tok == TOKEN_FLOAT) {
    expr_node_t* float_node = (expr_node_t*)init_const_float_node(tok->float_val);
    get_tok_next(tok);
    return float_node;
  } else if (tok->current_tok == TOKEN_IDENT) {
    char* ident = strdup(tok->ident);
    expr_node_t* ident_node = (expr_node_t*)init_ident_node(ident);
    if (!ident_node) return NULL;
    get_tok_next(tok);
    return ident_node;
  } else if (tok->current_tok == TOKEN_VAR_DECL) {
    get_tok_next(tok);
    if (!expect(tok, TOKEN_IDENT, "identifier")) {
      return NULL;
    }
    char* ident = strdup(tok->ident);
    get_tok_next(tok);
    if (!expect(tok, TOKEN_EQUAL, "assignment")) {
      return NULL;
    }
    get_tok_next(tok);
    expr_node_t* rhs = parse_expression_primary(tok, 0);
    if (!rhs) return NULL;
    expr_node_t* var_decl_node = (expr_node_t*)init_var_decl_node(ident, rhs);
    set_symbol(ident, rhs->type);
    return var_decl_node;
  }
  expect(tok, 0, "unary op, '(', var declaration, identifier or an integer");
  return NULL;
}

expr_node_t* parse_expression_primary(tokenizer_t *tok, int prec) {
  expr_node_t* lhs = parse_expression_secondary(tok);
  if (!lhs) {
    return NULL;
  }
  bin_op_t bin_op = token_to_bin_op(tok->current_tok);
  int op_prec;
  if (bin_op != BIN_OP_INVALID) op_prec = binary_precedence(bin_op);
  while (bin_op != BIN_OP_INVALID && op_prec >= prec) {
    int new_prec = left_associative(bin_op) ? op_prec + 1 : op_prec;
    get_tok_next(tok);
    expr_node_t* rhs = parse_expression_primary(tok, new_prec);
    if (!rhs) return NULL;
    lhs = (expr_node_t*)init_bin_op_node(bin_op, lhs, rhs);
    if (!lhs) return NULL;
    bin_op = token_to_bin_op(tok->current_tok);
    if (bin_op != BIN_OP_INVALID) op_prec = binary_precedence(bin_op);
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
expr_node_t* parse_expression(tokenizer_t *tok) {
  return parse_expression_primary(tok, 0);
}

expr_node_t* parse_expression_list(tokenizer_t *tok) {
  expr_list_node_t* expr_list = NULL;
  while (tok->current_tok != TOKEN_EOF) {
    expr_node_t* next_expr = parse_expression(tok);
    if (!next_expr) return NULL;
    expr_list = (expr_list_node_t*)init_expr_list_node(expr_list, next_expr);
    if (tok->current_tok != TOKEN_SEMI && tok->current_tok != TOKEN_EOF) {
      expect(tok, 0, "; or EOF");
      return NULL;
    }
    get_tok_next(tok);
  }
  return (expr_node_t*)expr_list;
}

expr_node_t* parse_file(FILE *input) {
  // parse it
  tokenizer_t tokenizer;
  tokenizer.input = input;

  get_tok_next(&tokenizer);
  expr_node_t* ast = parse_expression_list(&tokenizer);
  if (!ast) {
    fprintf(stderr, "No expression parsed\n");
    return NULL;
  }
  return ast;
}

