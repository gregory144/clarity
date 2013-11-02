#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "list.h"

list_t* type_list;

void type_int_init(type_system_t* type_sys) {
  type_set(type_sys, "Integer");
}

void type_float_init(type_system_t* type_sys) {
  type_set(type_sys, "Float");
}

void type_fun_init(type_system_t* type_sys) {
  type_set(type_sys, "Function");
}

type_system_t* type_init() {
  type_system_t* type_sys = malloc(sizeof(type_system_t));
  type_sys->types = list_init();
  type_int_init(type_sys);
  type_float_init(type_sys);
  type_fun_init(type_sys);
  return type_sys;
}

void type_int_init();

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

type_t* type_set(type_system_t* type_sys, char* name) {
  list_t* types = type_sys->types;
  type_t* type = malloc(sizeof(type_t));
  type->name = strdup(name);
  list_push(types, type);
  return type;
}

bool type_equals(type_t* type1, type_t* type2) {
  return strcmp(type1->name, type2->name) == 0;
}

char* type_to_string(type_t* type) {
  return type->name;
}

