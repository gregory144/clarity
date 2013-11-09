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
#include <limits.h>

#include "codegen.h"
#include "graphgen.h"
#include "parse.h"
#include "ast.h"
#include "context.h"

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

  LLVMAddConstantPropagationPass(pass);
  LLVMAddInstructionCombiningPass(pass);
  LLVMAddPromoteMemoryToRegisterPass(pass);
  LLVMAddGVNPass(pass);
  LLVMAddCFGSimplificationPass(pass);

  LLVMRunPassManager(pass, mod);
  LLVMDumpModule(mod);

  LLVMValueRef main_func = LLVMGetNamedFunction(mod, "main");

  LLVMGenericValueRef exec_args[] = {};
  LLVMGenericValueRef exec_res = LLVMRunFunction(engine, main_func, 0, exec_args);

  LLVMTypeRef ret_type = LLVMGetReturnType(LLVMGetElementType(LLVMTypeOf(main_func)));
  LLVMTypeKind ret_type_kind = LLVMGetTypeKind(ret_type);

  fprintf(stderr, "\nResult: ");
  if (ret_type_kind == LLVMIntegerTypeKind) {
    int64_t ret_int = LLVMGenericValueToInt(exec_res, true);
    fprintf(stderr, "%ld", ret_int);
  } else if (ret_type_kind == LLVMDoubleTypeKind) {
    double ret_double = LLVMGenericValueToFloat(LLVMDoubleType(), exec_res);
    fprintf(stderr, "%f", ret_double);
  }
  fprintf(stderr, "\n");
  LLVMDisposeGenericValue(exec_res);

  LLVMDisposePassManager(pass);
  LLVMDisposeExecutionEngine(engine);
}

int main(int argc, char const *argv[])
{
  LLVMLinkInJIT();
  LLVMInitializeNativeTarget();

  context_t* context = context_init();

  expr_node_t* ast = parse_file(context, stdin);
  if (!ast) {
    return 0;
  }

  FILE *dot_file;
  dot_file = fopen("graph.dot", "w");
  char* dot_src = graphgen(context, ast);
  if (dot_src) {
    fprintf(dot_file, "%s\n", dot_src);
  } else {
    fprintf(stderr, "Unable to generate dot file");
    fprintf(dot_file, "Unable to generate dot file");
  }
  fclose(dot_file);
  free(dot_src);

  LLVMModuleRef mod = codegen(context, ast);
  if (!mod) {
    return 0;
  }
  ast_expr_node_free(ast);

  // run it!
  execute(mod);

  context_free(context);

  return 0;
}

