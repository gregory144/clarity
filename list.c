#include <stdio.h>
#include <stdlib.h>

#include "list.h"

list_item_t* list_item_init(void* val) {
  list_item_t* list_item = malloc(sizeof(list_item_t));
  list_item->val = val;
  list_item->next = NULL;
  return list_item;
}

list_t* list_init() {
  list_t* list = malloc(sizeof(list_t));
  list->head = NULL;
  list->size = 0;
  return list;
}

list_item_t* list_unshift(list_t* list, void* val) {
  if (!list) {
    fprintf(stderr, "Unable to unshift: null list provided\n");
    return NULL;
  }
  list_item_t* list_item = list_item_init(val);
  list_item->next = list->head;
  list->head = list_item;
  list->size++;
  return list_item;
}

// add to end
list_item_t* list_push(list_t* list, void* val) {
  if (!list) {
    fprintf(stderr, "Unable to push: null list provided\n");
    return NULL;
  }
  list_item_t* list_item = list_item_init(val);
  if (list->head) {
    list_item_t* iter = list->head;
    while (iter->next) iter = iter->next;
    iter->next = list_item;
  } else {
    list->head = list_item;
  }
  list->size++;
  return list_item;
}

// remove from front
void* list_shift(list_t* list) {
  if (!list) {
    fprintf(stderr, "Unable to shift: null list provided\n");
    return NULL;
  }
  if (list->head) {
    list_item_t* head = list->head;
    list->head = list->head->next;
    list->size--;
    return head->val;
  } else {
    return NULL;
  }
}

// remove from end
void* list_pop(list_t* list) {
  if (!list) {
    fprintf(stderr, "Unable to pop: null list provided\n");
    return NULL;
  }
  list_item_t* iter = list->head;
  // if there is no item in the list, return null
  if (!iter) {
    return NULL;
  }
  // if there is only one item in the list
  if (!iter->next) {
    list->head = NULL;
    list->size--;
    return iter->val;
  } else {
    // find the second to last item
    while (iter->next->next) iter = iter->next;
    list_item_t* tail = iter->next;
    iter->next = NULL;
    list->size--;
    return tail->val;
  }
}

list_item_t* list_iter_init(list_t* list) {
  if (list == NULL) {
    fprintf(stderr, "Unable to iterate: null list provided\n");
    return NULL;
  }
  return list->head;
}

list_item_t* list_iter(list_item_t* curr) {
  if (curr) {
    return curr->next;
  }
  return NULL;
}

void list_visit(list_t* list, void (*visit)(void* val)) {
  list_item_t* iter = list_iter_init(list);
  for (; iter; iter = list_iter(iter)) {
    visit(iter->val);
  }
}

void list_free(list_t* list) {
  list_item_t* list_item = list->head;
  while (list_item) {
    list_item_t* to_free = list_item;
    list_item = list_item->next;
    free(to_free);
  }
  free(list);
}
