
#ifndef CODEGEN_H

#define CODEGEN_H

#include <llvm-c/Core.h>

#include "context.h"
#include "ast.h"

LLVMValueRef codegen_expr(context_t* context, LLVMBuilderRef builder, expr_node_t* node);

LLVMValueRef codegen_expr_list(context_t* context, LLVMBuilderRef builder, expr_list_node_t* node);

LLVMValueRef codegen_const_int(context_t* context, LLVMBuilderRef builder, const_int_node_t* node);

LLVMValueRef codegen_const_float(context_t* context, LLVMBuilderRef builder, const_float_node_t* node);

LLVMValueRef codegen_const_bool(context_t* context, LLVMBuilderRef builder, const_bool_node_t* node);

LLVMValueRef codegen_ident(context_t* context, LLVMBuilderRef builder, ident_node_t* node);

LLVMValueRef codegen_var_decl(context_t* context, LLVMBuilderRef builder, var_decl_node_t* node);

LLVMValueRef codegen_bin_op(context_t* context, LLVMBuilderRef builder, bin_op_node_t* node);

LLVMValueRef codegen_unary_op(context_t* context, LLVMBuilderRef builder, unary_op_node_t* node);

LLVMValueRef codegen_fun_call(context_t* context, LLVMBuilderRef builder, fun_call_node_t* node);

LLVMValueRef codegen_block(context_t* context, LLVMBuilderRef builder, block_node_t* node, char* function_name);

LLVMValueRef codegen_fun_param(context_t* context, LLVMBuilderRef builder, fun_param_node_t* node);

LLVMModuleRef codegen(context_t* context, expr_node_t* ast);

#endif
