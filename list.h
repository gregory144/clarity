
#ifndef LIST_H

#define LIST_H

typedef struct list_item_t {
  void* val;
  struct list_item_t* next;
} list_item_t;

typedef struct {
  list_item_t* head;
  int size;
} list_t;

list_t* list_init();

// add to front
list_item_t* list_unshift(list_t* list, void* val);

// add to end
list_item_t* list_push(list_t* list, void* val);

// remove from end
void* list_pop(list_t* list);

// remove from front
void* list_shift(list_t* list);

list_item_t* list_iter_init(list_t*);

list_item_t* list_iter(list_item_t*);

#endif
