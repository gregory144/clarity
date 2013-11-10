#include "type_bool.h"

LLVMTypeRef type_bool_get_ref() {
  return LLVMIntType(1);
}

LLVMValueRef type_bool_convert(type_system_t* type_sys, LLVMBuilderRef builder, LLVMValueRef val, type_t* to_type) {
  if (type_name_is(to_type, "Boolean")) {
    return val;
  }
  if (type_name_is(to_type, "Float")) {
    type_t* type_int = type_get(type_sys, "Integer");
    LLVMValueRef intermediate = type_int->convert(type_sys, builder, val, type_int);
    return LLVMBuildSIToFP(builder, intermediate, to_type->get_ref(), "inttofloat");
  }
  if (type_name_is(to_type, "Integer")) {
    return LLVMBuildIntCast(builder, val, to_type->get_ref(), "booltoint");
  }
  return NULL;
}

void type_bool_init(type_system_t* type_sys) {
  type_set(type_sys, true, "Boolean", type_bool_get_ref, type_bool_convert);
}

