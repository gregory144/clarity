#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "list.h"

LLVMTypeRef type_int_get_ref() {
  return LLVMInt64Type();
}

void type_int_init(type_system_t* type_sys) {
  type_set(type_sys, true, "Integer", type_int_get_ref);
}

void type_free(type_t* type) {
  free(type->name);
  free(type);
}

void type_system_free(type_system_t* type_sys) {
  list_visit(type_sys->types, (void(*)(void*))type_free);
  list_free(type_sys->types);
  free(type_sys);
}

LLVMTypeRef type_float_get_ref() {
  return LLVMDoubleType();
}

void type_float_init(type_system_t* type_sys) {
  type_set(type_sys, true, "Float", type_float_get_ref);
}

void type_fun_init(type_system_t* type_sys) {
  type_set(type_sys, true, "Function", NULL);
}

type_system_t* type_init() {
  type_system_t* type_sys = malloc(sizeof(type_system_t));
  type_sys->types = list_init();
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

type_t* type_set(type_system_t* type_sys, bool primitive, char* name, LLVMTypeRef (*get_ref)(void*)) {
  list_t* types = type_sys->types;
  type_t* type = malloc(sizeof(type_t));
  type->primitive = primitive;
  type->name = strdup(name);
  type->get_ref = get_ref;
  list_push(types, type);
  return type;
}

bool type_equals(type_t* type1, type_t* type2) {
  return strcmp(type1->name, type2->name) == 0;
}

char* type_to_string(type_t* type) {
  return type->name;
}

