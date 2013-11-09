#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "type_int.h"
#include "type_bool.h"
#include "type_float.h"
#include "type_fun.h"
#include "list.h"

void type_free(type_t* type) {
  free(type->name);
  free(type);
}

void type_system_free(type_system_t* type_sys) {
  list_visit(type_sys->types, (void(*)(void*))type_free);
  list_free(type_sys->types);
  free(type_sys);
}

type_system_t* type_init() {
  type_system_t* type_sys = malloc(sizeof(type_system_t));
  type_sys->types = list_init();
  type_bool_init(type_sys);
  type_int_init(type_sys);
  type_float_init(type_sys);
  type_fun_init(type_sys);
  return type_sys;
}

type_t* type_get(type_system_t* type_sys, char* name) {
  list_item_t* iter = list_iter_init(type_sys->types);
  for (; iter; iter = list_iter(iter)) {
    type_t* candidate = iter->val;
    if (strcmp(candidate->name, name) == 0) {
      return candidate;
    }
  }
  return NULL;
}

LLVMTypeRef type_get_ref(type_t* type) {
  return type->get_ref();
}

type_t* type_set(type_system_t* type_sys, bool primitive, char* name,
    LLVMTypeRef (*get_ref)(void*),
    LLVMValueRef (*convert)(type_system_t*, LLVMBuilderRef, LLVMValueRef, type_t*)) {
  list_t* types = type_sys->types;
  type_t* type = malloc(sizeof(type_t));
  type->primitive = primitive;
  type->name = strdup(name);
  type->get_ref = get_ref;
  type->convert = convert;
  list_push(types, type);
  return type;
}

bool type_equals(type_t* type1, type_t* type2) {
  return type_name_is(type1, type2->name);
}

bool type_name_is(type_t* type, char* name) {
  return strcmp(type->name, name) == 0;
}

char* type_to_string(type_t* type) {
  return type->name;
}

