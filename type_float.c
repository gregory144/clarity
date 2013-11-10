#include "type_float.h"

LLVMTypeRef type_float_get_ref() {
  return LLVMDoubleType();
}

LLVMValueRef type_float_convert(type_system_t* type_sys, LLVMBuilderRef builder, LLVMValueRef val, type_t* to_type) {
  if (type_name_is(to_type, "Float")) {
    return val;
  }
  if (type_name_is(to_type, "Integer")) {
    return LLVMBuildFPToSI(builder, val, to_type->get_ref(), "floattoint");
  }
  if (type_name_is(to_type, "Boolean")) {
    type_t* int_type = type_get(type_sys, "Integer");
    LLVMValueRef intermediate = int_type->convert(type_sys, builder, val, int_type);
    return LLVMBuildIntCast(builder, intermediate, to_type->get_ref(), "inttobool");
  }
  return NULL;
}

void type_float_init(type_system_t* type_sys) {
  type_set(type_sys, true, "Float", type_float_get_ref, type_float_convert);
}

