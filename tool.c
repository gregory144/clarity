// Headers required by LLVM
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

// General stuff
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>

typedef enum {
  TOKEN_INVALID, TOKEN_INTEGER, TOKEN_IDENT, TOKEN_VAR_DECL, TOKEN_EOF,
  TOKEN_PLUS, TOKEN_DASH, TOKEN_STAR, TOKEN_FORWARD_SLASH,
  TOKEN_PERCENT, TOKEN_EQUAL, TOKEN_OPEN_PAREN, TOKEN_CLOSE_PAREN,
  TOKEN_SEMI
} token_t;

typedef enum { NODE_BINARY_OP, NODE_UNARY_OP, NODE_CONST_INT, NODE_IDENT,
  NODE_VAR_DECL, NODE_EXPR_LIST
} node_t;

typedef enum { BIN_OP_INVALID, BIN_OP_PLUS, BIN_OP_MINUS, BIN_OP_MULT, BIN_OP_DIV, BIN_OP_MOD, BIN_OP_EQ } bin_op_t;

typedef enum { UNARY_OP_INVALID, UNARY_OP_NEGATE } unary_op_t;

typedef struct {
  FILE* input;
  token_t current_tok;
  char ident[512];
  int int_val;
} tokenizer_t;

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

typedef struct symbol_t {
  char* name;
  LLVMValueRef value;
  struct symbol_t* next;
} symbol_t;

symbol_t* symbols;

symbol_t* get_symbol(char* name) {
  symbol_t* curr = symbols;
    while (curr) {
    if (strcmp(curr->name, name) == 0) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

symbol_t* set_symbol(char* name, LLVMValueRef value) {
  symbol_t* new_symbol = (symbol_t*)malloc(sizeof(symbol_t));
  new_symbol->name = name;
  new_symbol->value = value;
  new_symbol->next = symbols;
  symbols = new_symbol;
  return new_symbol;
}

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
      sprintf(buf, "%d", tokenizer->int_val);
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

LLVMValueRef codegen_expr(LLVMBuilderRef builder, expr_node_t* node) {
  LLVMValueRef (*fun)() = node->codegen_fun;
  return fun(builder, node);
}

LLVMValueRef codegen_expr_list(LLVMBuilderRef builder, expr_list_node_t* node) {
  LLVMValueRef ret = NULL;
  expr_list_node_t* iter;
  for (iter = node; iter; iter = iter->next) {
    ret = codegen_expr(builder, iter->expr);
  }
  return ret;
}

expr_list_node_t* init_expr_list_node(expr_list_node_t* old_list, expr_node_t* new_expr) {
  expr_list_node_t* node = (expr_list_node_t*)malloc(sizeof(expr_list_node_t));
  node->node_type = NODE_EXPR_LIST;
  node->codegen_fun = codegen_expr_list;
  node->expr = new_expr;
  node->next = NULL;
  if (old_list != NULL) {
    expr_list_node_t* iter;
    for (iter = old_list; iter->next; iter = iter->next);
    iter->next = node;
    return old_list;
  } else {
    return node;
  }
}

LLVMValueRef codegen_const_int(LLVMBuilderRef builder, const_int_node_t* node) {
  return LLVMConstInt(LLVMInt32Type(), node->val, 0);
}

const_int_node_t* init_const_int_node(int val) {
  const_int_node_t* node = (const_int_node_t*)malloc(sizeof(const_int_node_t));
  node->node_type = NODE_CONST_INT;
  node->codegen_fun = codegen_const_int;
  node->val = val;
  return node;
}

LLVMValueRef codegen_ident(LLVMBuilderRef builder, ident_node_t* node) {
  symbol_t* symbol = get_symbol(node->name);
  if (!symbol) {
    fprintf(stderr, "Unable to find symbol with name: %s\n", node->name);
    return NULL;
  }
  return LLVMBuildLoad(builder, symbol->value, node->name);
}

ident_node_t* init_ident_node(char* name) {
  ident_node_t* node = (ident_node_t*)malloc(sizeof(ident_node_t));
  node->node_type = NODE_IDENT;
  node->codegen_fun = codegen_ident;
  node->name = name;
  return node;
}

LLVMValueRef codegen_var_decl(LLVMBuilderRef builder, var_decl_node_t* node) {
  LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), node->name);
  LLVMValueRef value;
  if (node->rhs) {
    value = codegen_expr(builder, node->rhs);
    if (value == NULL) return NULL;
  } else {
    value = LLVMConstInt(LLVMInt32Type(), 0, 0);
  }
  LLVMBuildStore(builder, value, alloca);
  set_symbol(node->name, alloca);
  return value;
}

var_decl_node_t* init_var_decl_node(char* name, expr_node_t* rhs) {
  var_decl_node_t* node = (var_decl_node_t*)malloc(sizeof(var_decl_node_t));
  node->node_type = NODE_VAR_DECL;
  node->codegen_fun = codegen_var_decl;
  node->name = name;
  node->rhs = rhs;
  return node;
}

LLVMValueRef codegen_bin_op(LLVMBuilderRef builder, bin_op_node_t* node) {
  LLVMValueRef rhs = codegen_expr(builder, node->rhs);
  if (rhs == NULL) return NULL;
  if (node->op == BIN_OP_EQ) {
    if (node->lhs->node_type != NODE_IDENT) {
      fprintf(stderr, "Left hand side of assignment must be an identifier\n");
      return NULL;
    }
    ident_node_t* ident_node = (ident_node_t*)node->lhs;
    symbol_t* symbol = get_symbol(ident_node->name);
    if (!symbol) {
      fprintf(stderr, "Unable to find symbol with name: %s\n", ident_node->name);
      return NULL;
    }
    LLVMBuildStore(builder, rhs, symbol->value);
    return symbol->value;
  }
  LLVMValueRef lhs = codegen_expr(builder, node->lhs);
  if (lhs == NULL) return NULL;
  switch(node->op) {
    case BIN_OP_PLUS:
      return LLVMBuildAdd(builder, lhs, rhs, "addop");
    case BIN_OP_MINUS:
      return LLVMBuildSub(builder, lhs, rhs, "subop");
    case BIN_OP_MULT:
      return LLVMBuildMul(builder, lhs, rhs, "mulop");
    case BIN_OP_DIV:
      return LLVMBuildSDiv(builder, lhs, rhs, "divop");
    case BIN_OP_MOD:
      return LLVMBuildSRem(builder, lhs, rhs, "modop");
    default:
      fprintf(stderr, "Unknown binary operator\n");
      return NULL;
  }
}

bin_op_node_t* init_bin_op_node(bin_op_t op, expr_node_t* lhs, expr_node_t* rhs) {
  bin_op_node_t* node = (bin_op_node_t*)malloc(sizeof(bin_op_node_t));
  node->node_type = NODE_BINARY_OP;
  node->codegen_fun = codegen_bin_op;
  node->op = op;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

LLVMValueRef codegen_unary_op(LLVMBuilderRef builder, unary_op_node_t* node) {
  if (node->op == UNARY_OP_NEGATE) {
    LLVMValueRef rhs = codegen_expr(builder, node->rhs);
    if (rhs == NULL) return NULL;
    return LLVMBuildSub(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), rhs, "negative");
  }
  fprintf(stderr, "Unknown unary operator\n");
  return NULL;
}

unary_op_node_t* init_unary_op_node(unary_op_t op, expr_node_t* rhs) {
  unary_op_node_t* node = (unary_op_node_t*)malloc(sizeof(unary_op_node_t));
  node->node_type = NODE_UNARY_OP;
  node->codegen_fun = codegen_unary_op;
  node->op = op;
  node->rhs = rhs;
  return node;
}

void initialize_llvm() {
  LLVMLinkInJIT();
  LLVMInitializeNativeTarget();
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
		while (isdigit(ident[++i] = fgetc(tok->input))
				|| ident[i] == '.')
			;
		c = ident[i];
		ident[i] = '\0';
		tok->int_val = strtod(ident, NULL);
		return TOKEN_INTEGER;
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

expr_node_t* parse_expression_secondary(tokenizer_t *tok);

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
    if (!rhs) {
      return NULL;
    }
    lhs = (expr_node_t*)init_bin_op_node(bin_op, lhs, rhs);
    bin_op = token_to_bin_op(tok->current_tok);
    if (bin_op != BIN_OP_INVALID) op_prec = binary_precedence(bin_op);
  }
  return lhs;
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
  } else if (tok->current_tok == TOKEN_IDENT) {
    char* ident = strdup(tok->ident);
    expr_node_t* ident_node = (expr_node_t*)init_ident_node(ident);
    get_tok_next(tok);
    return ident_node;
  } else if (tok->current_tok == TOKEN_VAR_DECL) {
    get_tok_next(tok);
    if (!expect(tok, TOKEN_IDENT, "identifier")) {
      return NULL;
    }
    char* ident = strdup(tok->ident);
    get_tok_next(tok);
    expr_node_t* rhs = NULL;
    if (tok->current_tok == TOKEN_EQUAL) {
      get_tok_next(tok);
      rhs = parse_expression_primary(tok, 0);
    }
    expr_node_t* var_decl_node = (expr_node_t*)init_var_decl_node(ident, rhs);
    return var_decl_node;
  }
  expect(tok, 0, "unary op, '(', var declaration, identifier or an integer");
  return NULL;
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

LLVMModuleRef codegen(expr_node_t* ast) {
  // compile it
  LLVMBuilderRef builder = LLVMCreateBuilder();

  LLVMModuleRef mod = LLVMModuleCreateWithName("calc");

  LLVMTypeRef main_args[] = {};
  LLVMValueRef main_func = LLVMAddFunction(mod, "main", LLVMFunctionType(LLVMInt32Type(), main_args, 0, 0));
  LLVMSetFunctionCallConv(main_func, LLVMCCallConv);

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");

  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef ret_value = codegen_expr(builder, ast);
  if (ret_value == NULL) return NULL;

  LLVMBuildRet(builder, ret_value);

  char *error = NULL;
  LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error); // Handler == LLVMAbortProcessAction -> No need to check errors

  LLVMDisposeBuilder(builder);

  return mod;
}

void execute(LLVMModuleRef mod) {
  LLVMExecutionEngineRef engine;
  LLVMModuleProviderRef provider = LLVMCreateModuleProviderForExistingModule(mod);
  char *error = NULL;
  if(LLVMCreateJITCompiler(&engine, provider, 2, &error) != 0) {
    fprintf(stderr, "%s\n", error);
    LLVMDisposeMessage(error);
    abort();
  }

  LLVMPassManagerRef pass = LLVMCreatePassManager();
  LLVMAddTargetData(LLVMGetExecutionEngineTargetData(engine), pass);

  /*LLVMAddConstantPropagationPass(pass);*/
  /*LLVMAddInstructionCombiningPass(pass);*/
  /*LLVMAddPromoteMemoryToRegisterPass(pass);*/
  /*LLVMAddGVNPass(pass);*/
  /*LLVMAddCFGSimplificationPass(pass);*/

  LLVMRunPassManager(pass, mod);
  LLVMDumpModule(mod);

  LLVMValueRef main_func = LLVMGetNamedFunction(mod, "main");
  LLVMGenericValueRef exec_args[] = {};
  LLVMGenericValueRef exec_res = LLVMRunFunction(engine, main_func, 0, exec_args);
  fprintf(stderr, "\n");
  fprintf(stderr, "; Result: %llu\n", LLVMGenericValueToInt(exec_res, 0));

  LLVMDisposePassManager(pass);
  LLVMDisposeExecutionEngine(engine);
}

int main(int argc, char const *argv[])
{
  initialize_llvm();

  expr_node_t* ast = parse_file(stdin);
  if (!ast) {
    return 0;
  }

  LLVMModuleRef mod = codegen(ast);
  if (!mod) {
    return 0;
  }

  // run it!
  execute(mod);

  return 0;
}

