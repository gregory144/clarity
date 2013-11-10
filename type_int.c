#include <stdlib.h>

#include "type_int.h"

LLVMTypeRef type_int_get_ref() {
  return LLVMInt64Type();
}

LLVMValueRef type_int_convert(type_system_t* type_sys, LLVMBuilderRef builder, LLVMValueRef val, type_t* to_type) {
  if (type_name_is(to_type, "Integer")) {
    return val;
  }
  if (type_name_is(to_type, "Float")) {
    return LLVMBuildSIToFP(builder, val, to_type->get_ref(), "inttofloat");
  }
  if (type_name_is(to_type, "Boolean")) {
    return LLVMBuildIntCast(builder, val, to_type->get_ref(), "inttobool");
  }
  return NULL;
}

void type_int_init(type_system_t* type_sys) {
  type_set(type_sys, true, "Integer", type_int_get_ref, type_int_convert);
}

