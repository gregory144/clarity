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

#include "enums.h"
#include "codegen.h"
#include "parser.h"
#include "ast.h"

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

  LLVMDisposePassManager(pass);
  LLVMDisposeExecutionEngine(engine);
}

int main(int argc, char const *argv[])
{
  LLVMLinkInJIT();
  LLVMInitializeNativeTarget();

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

