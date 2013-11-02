#ifndef TYPE_H

#define TYPE_H

#include <stdbool.h>

#include "list.h"

typedef struct type_t {
  char* name;
} type_t;

typedef struct type_system_t {
  list_t* types;
} type_system_t;

type_system_t* type_init();

type_t* type_get(type_system_t* type_sys, char* name);

type_t* type_set(type_system_t* type_sys, char* name);

bool type_equals(type_t* type1, type_t* type2);

char* type_to_string(type_t* type);

#endif
