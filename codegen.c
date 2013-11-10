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

#include "context.h"
#include "codegen.h"

static unsigned int function_index = 0;

LLVMModuleRef mod;

LLVMModuleRef get_current_module() {
  return mod;
}

LLVMValueRef codegen_expr(context_t* context, LLVMBuilderRef builder, expr_node_t* node) {
  LLVMValueRef (*fun)() = node->codegen_fun;
  return fun(context, builder, node);
}

LLVMValueRef codegen_expr_list(context_t* context, LLVMBuilderRef builder, expr_list_node_t* node) {
  symbol_table_t* parent_scope = context->symbol_table;
  context->symbol_table = node->scope;

  LLVMValueRef ret = NULL;
  list_item_t* iter = list_iter_init(node->expressions);
  for (; iter; iter = list_iter(iter)) {
    ret = codegen_expr(context, builder, iter->val);
    if (ret == NULL) return NULL;
    LLVMDumpValue(ret);
  }

  printf("Restoring parent scope: %p\n", parent_scope);
  context->symbol_table = parent_scope;
  return ret;
}

LLVMValueRef codegen_const_int(context_t* context, LLVMBuilderRef builder, const_int_node_t* node) {
  return LLVMConstInt(node->type->get_ref(), node->val, 0);
}

LLVMValueRef codegen_const_float(context_t* context, LLVMBuilderRef builder, const_float_node_t* node) {
  return LLVMConstReal(node->type->get_ref(), node->val);
}

LLVMValueRef codegen_const_bool(context_t* context, LLVMBuilderRef builder, const_bool_node_t* node) {
  return LLVMConstInt(node->type->get_ref(), node->val ? 1 : 0, 0);
}

LLVMValueRef codegen_ident(context_t* context, LLVMBuilderRef builder, ident_node_t* node) {
  symbol_t* symbol = symbol_get(context->symbol_table, node->name);
  if (!symbol) {
    fprintf(stderr, "codegen_ident: Unable to find symbol with name: %s\n", node->name);
    return NULL;
  }
  printf("Loading %s\n", node->name);
  LLVMDumpValue(symbol->value);
  if (!symbol->is_param) {
    LLVMValueRef load = LLVMBuildLoad(builder, symbol->value, node->name);
    printf("loaded:\n");
    LLVMDumpValue(load);
    return load;
  }
  return symbol->value;
}

LLVMValueRef codegen_fun_decl(context_t* context, LLVMBuilderRef builder, var_decl_node_t* node) {
  block_node_t* block_node = (block_node_t*)node->rhs;
  LLVMValueRef function_expr = codegen_block(context, builder, block_node, node->name);
  if (function_expr) {
    /*LLVMTypeRef function_type = get_function_type(context, block_node);*/
    /*if (function_type == NULL) {*/
      /*return NULL;*/
    /*}*/
    /*LLVMTypeRef function_pointer_type = LLVMPointerType(function_type, 0);*/
    /*LLVMValueRef alloca = LLVMBuildAlloca(builder, function_pointer_type, node->name);*/
    /*printf("Func Alloca:\n");*/
    /*LLVMDumpValue(alloca);*/

    /*LLVMBuildStore(builder, function_value, alloca); // yields {void}*/

    symbol_t* symbol = symbol_get(context->symbol_table, node->name);
    if (!symbol) {
      fprintf(stderr, "Unable to find symbol: %s\n", node->name);
    } else {
      symbol->value = function_expr;
    }
  }
  return function_expr;
}

LLVMValueRef codegen_var_decl(context_t* context, LLVMBuilderRef builder, var_decl_node_t* node) {
  if (type_equals(node->type, type_get(context->type_sys, "Function"))) {
    return codegen_fun_decl(context, builder, node);
  }
  LLVMTypeRef type = type_get_ref(node->type);
  if (type == NULL) {
    fprintf(stderr, "Unrecognized type: %s\n", type_to_string(node->type));
    return NULL;
  }
  LLVMValueRef alloca = LLVMBuildAlloca(builder, type, node->name);
  LLVMValueRef value = codegen_expr(context, builder, node->rhs);
  if (value == NULL) return NULL;
  LLVMBuildStore(builder, value, alloca); // yields {void}
  symbol_t* symbol = symbol_get(context->symbol_table, node->name);
  if (!symbol) {
    fprintf(stderr, "Unable to find symbol: %s\n", node->name);
    return NULL;
  }
  symbol->value = alloca;
  return value;
}

LLVMValueRef codegen_bin_op(context_t* context, LLVMBuilderRef builder, bin_op_node_t* node) {
  LLVMValueRef rhs = codegen_expr(context, builder, node->rhs);
  if (rhs == NULL) return NULL;
  if (node->op == BIN_OP_ASSIGN) {
    if (node->lhs->node_type != NODE_IDENT) {
      fprintf(stderr, "Left hand side of assignment must be an identifier\n");
      return NULL;
    }
    ident_node_t* ident_node = (ident_node_t*)node->lhs;
    symbol_t* symbol = symbol_get(context->symbol_table, ident_node->name);
    if (!symbol) {
      fprintf(stderr, "codegen_bin_op: Unable to find symbol with name: %s\n", ident_node->name);
      return NULL;
    }
    LLVMBuildStore(builder, rhs, symbol->value);
    return symbol->value;
  }
  LLVMValueRef lhs = codegen_expr(context, builder, node->lhs);
  if (lhs == NULL) return NULL;
  // cast both to float if their types don't match
  type_t* numeric_res_type;

  type_t* type_int = type_get(context->type_sys, "Integer");
  type_t* type_float = type_get(context->type_sys, "Float");
  bool lhs_is_int = type_equals(node->lhs->type, type_int);
  bool lhs_is_float = type_equals(node->lhs->type, type_float);
  bool rhs_is_int = type_equals(node->rhs->type, type_int);
  bool rhs_is_float = type_equals(node->rhs->type, type_float);
  if (lhs_is_int && rhs_is_float) {
    numeric_res_type = type_float;
    lhs = node->lhs->type->convert(context->type_sys, builder, lhs, numeric_res_type);
    if (!lhs) {
      fprintf(stderr, "Unable to convert %s to %s\n", node->lhs->type->name, numeric_res_type->name);
      return NULL;
    }
  } else if (lhs_is_float && rhs_is_int) {
    numeric_res_type = type_float;
    rhs = node->rhs->type->convert(context->type_sys, builder, rhs, numeric_res_type);
    if (!rhs) {
      fprintf(stderr, "Unable to convert %s to %s\n", node->rhs->type->name, numeric_res_type->name);
      return NULL;
    }
  } else if (lhs_is_float && rhs_is_float) {
    numeric_res_type = type_float;
  } else if (lhs_is_int && rhs_is_int) {
    numeric_res_type = type_int;
  } else {
    fprintf(stderr, "Unable to perform binary operation on non-numeric operands\n");
    return NULL;
  }
  bool res_is_int = type_equals(numeric_res_type, type_int);
  switch(node->op) {
    case BIN_OP_PLUS:
      if (res_is_int) {
        return LLVMBuildAdd(builder, lhs, rhs, "addop");
      } else {
        return LLVMBuildFAdd(builder, lhs, rhs, "addop");
      }
    case BIN_OP_MINUS:
      if (res_is_int) {
        return LLVMBuildSub(builder, lhs, rhs, "subop");
      } else {
        return LLVMBuildFSub(builder, lhs, rhs, "subop");
      }
    case BIN_OP_MULT:
      if (res_is_int) {
        return LLVMBuildMul(builder, lhs, rhs, "mulop");
      } else {
        return LLVMBuildFMul(builder, lhs, rhs, "mulop");
      }
    case BIN_OP_DIV:
      if (res_is_int) {
        return LLVMBuildSDiv(builder, lhs, rhs, "divop");
      } else {
        return LLVMBuildFDiv(builder, lhs, rhs, "divop");
      }
    case BIN_OP_MOD:
      if (res_is_int) {
        return LLVMBuildSRem(builder, lhs, rhs, "modop");
      } else {
        return LLVMBuildFRem(builder, lhs, rhs, "modop");
      }
    case BIN_OP_EQ:
      if (res_is_int) {
        return LLVMBuildICmp(builder, LLVMIntEQ, lhs, rhs, "eqop");
      } else {
        return LLVMBuildFCmp(builder, LLVMRealOEQ, lhs, rhs, "eqop");
      }
    case BIN_OP_GT:
      if (res_is_int) {
        return LLVMBuildICmp(builder, LLVMIntSGT, lhs, rhs, "gtop");
      } else {
        return LLVMBuildFCmp(builder, LLVMRealOGT, lhs, rhs, "gtop");
      }
    case BIN_OP_LT:
      if (res_is_int) {
        return LLVMBuildICmp(builder, LLVMIntSLT, lhs, rhs, "ltop");
      } else {
        return LLVMBuildFCmp(builder, LLVMRealOLT, lhs, rhs, "ltop");
      }
    case BIN_OP_GTE:
      if (res_is_int) {
        return LLVMBuildICmp(builder, LLVMIntSGE, lhs, rhs, "gteop");
      } else {
        return LLVMBuildFCmp(builder, LLVMRealOGE, lhs, rhs, "gteop");
      }
    case BIN_OP_LTE:
      if (res_is_int) {
        return LLVMBuildICmp(builder, LLVMIntSLE, lhs, rhs, "lteop");
      } else {
        return LLVMBuildFCmp(builder, LLVMRealOLE, lhs, rhs, "lteop");
      }
    default:
      fprintf(stderr, "Unknown binary operator\n");
      return NULL;
  }
}

LLVMValueRef codegen_unary_op(context_t* context, LLVMBuilderRef builder, unary_op_node_t* node) {
  if (node->op == UNARY_OP_NEGATE) {
    LLVMValueRef rhs = codegen_expr(context, builder, node->rhs);
    if (rhs == NULL) return NULL;
    // TODO - codegen negate in type
    if (type_equals(node->rhs->type, type_get(context->type_sys, "Integer"))) {
      return LLVMBuildSub(builder, LLVMConstInt(LLVMInt64Type(), 0, 0), rhs, "negative");
    } else if (type_equals(node->rhs->type, type_get(context->type_sys, "Float"))) {
      return LLVMBuildFSub(builder, LLVMConstReal(LLVMDoubleType(), 0), rhs, "negative");
    } else {
      fprintf(stderr, "Could not negate non-numeric type\n");
      return NULL;
    }
  }
  fprintf(stderr, "Unknown unary operator\n");
  return NULL;
}

LLVMValueRef codegen_fun_call(context_t* context, LLVMBuilderRef builder, fun_call_node_t* node) {
  printf("function call\n");
  symbol_t* symbol = symbol_get(context->symbol_table, node->name);
  if (!symbol) {
    fprintf(stderr, "Unable to find symbol with name: %s\n", node->name);
    return NULL;
  }
  type_t* type_fun = type_get(context->type_sys, "Function");
  if (!type_equals(symbol->type, type_fun)) {
    printf("%s is a %s  != %s\n", node->name, type_to_string(symbol->type), type_to_string(type_fun));
    fprintf(stderr, "%s is not a function\n", node->name);
    return NULL;
  }

  LLVMValueRef params[node->params->size];
  list_item_t* iter = list_iter_init(node->params);
  for (unsigned int i = 0; iter; iter = list_iter(iter), i++) {
    expr_node_t* param_expr = iter->val;
    params[i] = codegen_expr(context, builder, param_expr);
  }
  printf("building call\n");
  return LLVMBuildCall(builder, symbol->value, params, node->params->size, "fun_res");
}

LLVMTypeRef codegen_get_fun_type_ref(type_system_t* type_sys, block_node_t* block) {
  printf("Getting function type\n");
  LLVMTypeRef ret_type = type_get_ref(block->body->type);
  if (ret_type == NULL && type_equals(block->body->type, type_get(type_sys, "Funtion"))) {
    expr_list_node_t* expr_list = block->body;
    list_item_t* iter = list_iter_init(expr_list->expressions);
    for (; iter && list_iter(iter); iter = list_iter(iter));
    ret_type = codegen_get_fun_type_ref(type_sys, (block_node_t*)iter->val);
  } else if (ret_type == NULL) {
    fprintf(stderr, "Unknown type: %s\n", type_to_string(block->body->type));
    return NULL;
  }
  size_t param_count = block->params->size;
  LLVMTypeRef param_types[param_count];
  list_item_t* iter = list_iter_init(block->params);
  for (unsigned int i = 0; iter; iter = list_iter(iter), i++) {
    fun_param_node_t* param = iter->val;
    param_types[i] = type_get_ref(param->type);
  }
  return LLVMFunctionType(ret_type, param_types, param_count, false);
}

LLVMValueRef codegen_block(context_t* context, LLVMBuilderRef builder, block_node_t* node, char* function_name) {
  if (function_name == NULL) {
    function_name = malloc(sizeof(char) * 512);
    sprintf(function_name, "function%d", function_index);
    function_index++;
  }

  printf("codegen_block\n");
  LLVMBasicBlockRef prev_block = LLVMGetInsertBlock(builder);

  LLVMTypeRef function_type = codegen_get_fun_type_ref(context->type_sys, node);
  LLVMValueRef func = LLVMAddFunction(get_current_module(), function_name, function_type);

  LLVMValueRef* params = malloc(node->params->size * sizeof(params));
  LLVMGetParams(func, params);
  list_item_t* iter = list_iter_init(node->params);
  for (unsigned int i = 0; iter; iter = list_iter(iter), i++) {
    fun_param_node_t* param = iter->val;
    LLVMValueRef param_value = params[i];
    LLVMSetValueName(param_value, param->name);
    symbol_t* symbol = symbol_get(node->body->scope, param->name);
    if (!symbol) {
      fprintf(stderr, "Could not find symbol for parameter: %s\n", param->name);
      return NULL;
    }
    symbol->value = param_value;
  }
  LLVMSetFunctionCallConv(func, LLVMCCallConv);

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef body = codegen_expr_list(context, builder, node->body);
  if (!body) return NULL;

  LLVMBuildRet(builder, body);

  LLVMPositionBuilderAtEnd(builder, prev_block);

  return func;
}

LLVMValueRef codegen_fun_param(context_t* context, LLVMBuilderRef builder, fun_param_node_t* node) {
  return NULL;
}

LLVMValueRef codegen_if(context_t* context, LLVMBuilderRef builder, if_node_t* node) {
  printf("codegen_if\n");
  LLVMBasicBlockRef prev_block = LLVMGetInsertBlock(builder);
  LLVMValueRef current_fun = LLVMGetBasicBlockParent(prev_block);

  type_t* bool_type = type_get(context->type_sys, "Boolean");
  LLVMValueRef cond_res = codegen_expr(context, builder, node->conditional);
  if (!cond_res) return NULL;
  cond_res = node->conditional->type->convert(context->type_sys, builder, cond_res, bool_type);
  if (!cond_res) {
    fprintf(stderr, "Could not convert %s to Boolean\n", node->conditional->type->name);
    return NULL;
  }

  LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_fun, "then");
  LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_fun, "else");
  LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_fun, "merge");

  LLVMValueRef br_res = LLVMBuildCondBr(builder, cond_res, then_block, else_block);

  LLVMPositionBuilderAtEnd(builder, then_block);
  LLVMValueRef then_res = codegen_expr_list(context, builder, node->true_expr);
  if (!then_res) return NULL;
  LLVMValueRef then_br = LLVMBuildBr(builder, merge_block);

  LLVMPositionBuilderAtEnd(builder, else_block);
  LLVMValueRef else_res = codegen_expr_list(context, builder, node->false_expr);
  if (!else_res) return NULL;
  LLVMValueRef else_br = LLVMBuildBr(builder, merge_block);

  LLVMPositionBuilderAtEnd(builder, merge_block);

  printf("phi node type: %s\n", type_to_string(node->type));
  printf("then type: %s\n", type_to_string(node->true_expr->type));
  printf("else type: %s\n", type_to_string(node->false_expr->type));
  LLVMValueRef phi_node = LLVMBuildPhi(builder, node->type->get_ref(), "phi");
  LLVMAddIncoming(phi_node, &then_res, &then_block, 1);
  LLVMAddIncoming(phi_node, &else_res, &else_block, 1);

  return phi_node;
}

LLVMModuleRef codegen(context_t* context, expr_node_t* ast) {
  // compile it
  LLVMBuilderRef builder = LLVMCreateBuilder();

  mod = LLVMModuleCreateWithName("tool_mod");

  char* node_str = node_to_string(ast->node_type);
  printf("Node type: %s\n", node_str);
  free(node_str);
  printf("type: %s\n", type_to_string(ast->type));

  LLVMTypeRef main_args[] = {};
  LLVMTypeRef ret_type = type_get_ref(ast->type);
  if (ret_type == NULL) {
    fprintf(stderr, "Unable to determine return type of program: %s\n", type_to_string(ast->type));
    return NULL;
  }
  LLVMValueRef main_func = LLVMAddFunction(mod, "main", LLVMFunctionType(ret_type, main_args, 0, 0));
  LLVMSetFunctionCallConv(main_func, LLVMCCallConv);

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");

  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef ret_value = codegen_expr(context, builder, ast);
  if (ret_value == NULL) return NULL;

  LLVMBuildRet(builder, ret_value);

  printf("Dumping module before verifier\n");
  LLVMDumpModule(mod);

  char *error = NULL;
  LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error); // Handler == LLVMAbortProcessAction -> No need to check errors

  LLVMDisposeBuilder(builder);

  return mod;
}

