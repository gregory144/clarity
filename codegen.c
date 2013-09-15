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
  return LLVMConstInt(LLVMInt64Type(), node->val, 0);
}

LLVMValueRef codegen_const_float(LLVMBuilderRef builder, const_float_node_t* node) {
  return LLVMConstReal(LLVMDoubleType(), node->val);
}

LLVMValueRef codegen_ident(LLVMBuilderRef builder, ident_node_t* node) {
  symbol_t* symbol = get_symbol(node->name);
  if (!symbol) {
    fprintf(stderr, "codegen_ident: Unable to find symbol with name: %s\n", node->name);
    return NULL;
  }
  return LLVMBuildLoad(builder, symbol->value, node->name);
}

LLVMValueRef codegen_var_decl(LLVMBuilderRef builder, var_decl_node_t* node) {
  LLVMValueRef alloca;
  // TODO - handle different types
  if (node->type == EXPR_TYPE_INT) {
    alloca = LLVMBuildAlloca(builder, LLVMInt64Type(), node->name);
  } else if (node->type == EXPR_TYPE_FLOAT) {
    alloca = LLVMBuildAlloca(builder, LLVMDoubleType(), node->name);
  } else {
    fprintf(stderr, "Unrecognized type: %d\n", node->type);
    return NULL;
  }
  LLVMValueRef value = codegen_expr(builder, node->rhs);
  if (!value) return NULL;
  LLVMBuildStore(builder, value, alloca);
  symbol_t* symbol = get_symbol(node->name);
  if (!symbol) {
    fprintf(stderr, "Unable to find symbol: %s\n", node->name);
    return NULL;
  }
  symbol->value = alloca;
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
      fprintf(stderr, "codegen_bin_op: Unable to find symbol with name: %s\n", ident_node->name);
      return NULL;
    }
    LLVMBuildStore(builder, rhs, symbol->value);
    return symbol->value;
  }
  LLVMValueRef lhs = codegen_expr(builder, node->lhs);
  if (lhs == NULL) return NULL;
  // cast both to float if their types don't match
  expr_type_t res_type;
  if (node->lhs->type == EXPR_TYPE_INT && node->rhs->type == EXPR_TYPE_FLOAT) {
    lhs = LLVMBuildSIToFP(builder, lhs, LLVMDoubleType(), "lhstofloat");
    res_type = EXPR_TYPE_FLOAT;
  } else if (node->lhs->type == EXPR_TYPE_FLOAT && node->rhs->type == EXPR_TYPE_INT) {
    rhs = LLVMBuildSIToFP(builder, rhs, LLVMDoubleType(), "rhstofloat");
    res_type = EXPR_TYPE_FLOAT;
  } else if (node->lhs->type == EXPR_TYPE_FLOAT && node->rhs->type == EXPR_TYPE_FLOAT) {
    res_type = EXPR_TYPE_FLOAT;
  } else if (node->lhs->type == EXPR_TYPE_INT && node->rhs->type == EXPR_TYPE_INT) {
    res_type = EXPR_TYPE_INT;
  } else {
    fprintf(stderr, "Unable to perform binary operation on non-numeric operands\n");
    return NULL;
  }
  switch(node->op) {
    case BIN_OP_PLUS:
      if (res_type == EXPR_TYPE_INT) {
        return LLVMBuildAdd(builder, lhs, rhs, "addop");
      } else {
        return LLVMBuildFAdd(builder, lhs, rhs, "addop");
      }
    case BIN_OP_MINUS:
      if (res_type == EXPR_TYPE_INT) {
        return LLVMBuildSub(builder, lhs, rhs, "subop");
      } else {
        return LLVMBuildFSub(builder, lhs, rhs, "subop");
      }
    case BIN_OP_MULT:
      if (res_type == EXPR_TYPE_INT) {
        return LLVMBuildMul(builder, lhs, rhs, "mulop");
      } else {
        return LLVMBuildFMul(builder, lhs, rhs, "mulop");
      }
    case BIN_OP_DIV:
      if (res_type == EXPR_TYPE_INT) {
        return LLVMBuildSDiv(builder, lhs, rhs, "divop");
      } else {
        return LLVMBuildFDiv(builder, lhs, rhs, "divop");
      }
    case BIN_OP_MOD:
      if (res_type == EXPR_TYPE_INT) {
        return LLVMBuildSRem(builder, lhs, rhs, "modop");
      } else {
        return LLVMBuildFRem(builder, lhs, rhs, "modop");
      }
    default:
      fprintf(stderr, "Unknown binary operator\n");
      return NULL;
  }
}

LLVMValueRef codegen_unary_op(LLVMBuilderRef builder, unary_op_node_t* node) {
  if (node->op == UNARY_OP_NEGATE) {
    LLVMValueRef rhs = codegen_expr(builder, node->rhs);
    if (rhs == NULL) return NULL;
    if (node->rhs->type == EXPR_TYPE_INT) {
      return LLVMBuildSub(builder, LLVMConstInt(LLVMInt64Type(), 0, 0), rhs, "negative");
    } else if (node->rhs->type == EXPR_TYPE_FLOAT) {
      return LLVMBuildFSub(builder, LLVMConstReal(LLVMDoubleType(), 0), rhs, "negative");
    } else {
      fprintf(stderr, "Could not negate non-numeric type\n");
      return NULL;
    }
  }
  fprintf(stderr, "Unknown unary operator\n");
  return NULL;
}

LLVMModuleRef codegen(expr_node_t* ast) {
  // compile it
  LLVMBuilderRef builder = LLVMCreateBuilder();

  LLVMModuleRef mod = LLVMModuleCreateWithName("calc");

  LLVMTypeRef main_args[] = {};
  LLVMTypeRef ret_type;
  if (ast->type == EXPR_TYPE_INT) {
    ret_type = LLVMInt64Type();
  } else if (ast->type == EXPR_TYPE_FLOAT) {
    ret_type = LLVMDoubleType();
  }
  LLVMValueRef main_func = LLVMAddFunction(mod, "main", LLVMFunctionType(ret_type, main_args, 0, 0));
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

