
#ifndef CODEGEN_H

#define CODEGEN_H

#include <llvm-c/Core.h>

#include "enums.h"
#include "ast.h"

LLVMValueRef codegen_expr(LLVMBuilderRef builder, expr_node_t* node);

LLVMValueRef codegen_expr_list(LLVMBuilderRef builder, expr_list_node_t* node);

LLVMValueRef codegen_const_int(LLVMBuilderRef builder, const_int_node_t* node);

LLVMValueRef codegen_ident(LLVMBuilderRef builder, ident_node_t* node);

LLVMValueRef codegen_var_decl(LLVMBuilderRef builder, var_decl_node_t* node);

LLVMValueRef codegen_bin_op(LLVMBuilderRef builder, bin_op_node_t* node);

LLVMValueRef codegen_unary_op(LLVMBuilderRef builder, unary_op_node_t* node);

LLVMModuleRef codegen(expr_node_t* ast);

#endif
