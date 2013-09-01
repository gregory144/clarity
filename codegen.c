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

#include "symbols.h"
#include "codegen.h"

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

LLVMValueRef codegen_const_int(LLVMBuilderRef builder, const_int_node_t* node) {
  return LLVMConstInt(LLVMInt32Type(), node->val, 0);
}

LLVMValueRef codegen_ident(LLVMBuilderRef builder, ident_node_t* node) {
  symbol_t* symbol = get_symbol(node->name);
  if (!symbol) {
    fprintf(stderr, "Unable to find symbol with name: %s\n", node->name);
    return NULL;
  }
  return LLVMBuildLoad(builder, symbol->value, node->name);
}

LLVMValueRef codegen_var_decl(LLVMBuilderRef builder, var_decl_node_t* node) {
  LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), node->name);
  LLVMValueRef value = codegen_expr(builder, node->rhs);
  if (value == NULL) return NULL;
  LLVMBuildStore(builder, value, alloca);
  set_symbol(node->name, alloca);
  return value;
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

LLVMValueRef codegen_unary_op(LLVMBuilderRef builder, unary_op_node_t* node) {
  if (node->op == UNARY_OP_NEGATE) {
    LLVMValueRef rhs = codegen_expr(builder, node->rhs);
    if (rhs == NULL) return NULL;
    return LLVMBuildSub(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), rhs, "negative");
  }
  fprintf(stderr, "Unknown unary operator\n");
  return NULL;
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

