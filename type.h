#ifndef TYPE_H

#define TYPE_H

#include <llvm-c/Core.h>

#include <stdbool.h>

#include "list.h"

typedef struct type_t {
  bool primitive;
  char* name;
  LLVMTypeRef (*get_ref)();
} type_t;

typedef struct type_system_t {
  list_t* types;
} type_system_t;

type_system_t* type_init();

void type_system_free(type_system_t*);

type_t* type_get(type_system_t* type_sys, char* name);

type_t* type_set(type_system_t* type_sys, bool primitive, char* name, LLVMTypeRef (*)(void*));

bool type_equals(type_t* type1, type_t* type2);

char* type_to_string(type_t* type);

LLVMTypeRef type_get_ref(type_t* type);

#endif
