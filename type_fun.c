#include "type_fun.h"

LLVMValueRef type_fun_convert(type_system_t* type_sys, LLVMBuilderRef builder, LLVMValueRef val, type_t* to_type) {
  return NULL;
}

void type_fun_init(type_system_t* type_sys) {
  type_set(type_sys, true, "Function", NULL, type_fun_convert);
}

