#include <stdlib.h>
#include <assert.h>

#include "list.h"

/*
 * This structure is used to represent a single link in a singly-linked list.
 * It is not defined in list.h, so it is not visible to the user.
 */
struct link {
  void* val;
  struct link* next;
};

/*
 * This structure is used to represent an entire singly-linked list.  Note that
 * we're keeping track of just the head of the list here, for simplicity.
 */
struct list {
  struct link* head;
  int numElements;
};

/*
 * This function allocates and initializes a new, empty linked list and
 * returns a pointer to it.
 */
struct list* list_create() {
  struct list* list = malloc(sizeof(struct list));
  list->head = NULL;
  list->numElements = 0;
  return list;
}

/*
 * This function frees the memory associated with a linked list.  Freeing any
 * memory associated with values still stored in the list is the responsibility
 * of the caller.
 *
 * Params:
 *   list - the linked list to be destroyed.  May not be NULL.
 */
void list_free(struct list* list) {
  assert(list);

  /*
   * Free all individual links.
   */
  struct link* next, * curr = list->head;
  while (curr != NULL) {
    next = curr->next;
    free(curr);
    curr = next;
  }

  free(list);
}

/*
 * This function inserts a new value into a given linked list.  The new element
 * is always inserted as the head of the list.
 *
 * Params:
 *   list - the linked list into which to insert an element.  May not be NULL.
 *   val - the value to be inserted.  Note that this parameter has type void*,
 *     which means that a pointer of any type can be passed.
 */
void list_insert(struct list* list, void* val) {
  assert(list);

  /*
   * Create new link and insert at head.
   */
  struct link* link = malloc(sizeof(struct link));
  link->val = val;
  link->next = list->head;
  list->head = link;
  list->numElements++;
}

/*
 * This function returns 1 if the list is empty and 0 otherwise.
 */
int list_isempty(struct list* list) {
  assert(list);
  if (list->head) {
    return 0;
  } else {
    return 1;
  }
}

/*
 * This function returns the value stored at the head of a given linked list
 * or NULL if the list is empty.
 */
void* list_head(struct list* list) {
  assert(list);

  if (list->head) {
    return list->head->val;
  } else {
    return NULL;
  }
}

void* list_element(struct list* list, int position){
  assert(list);
  struct link* element = list->head;
  int i;
  if (position >= list->numElements){
    return NULL;
  }
  for (i = 0; i <= position - 1; i++){
    element = element->next;
  }
  return element->val;
}

int getNumElements(struct list* list){
  return list->numElements;
}

/*
 * This function removes the value stored at the head of a given linked list.
 * If the list is empty, this function is a noop.
 */
void* list_remove_head(struct list* list) {
  assert(list);
  if (!list->head){
    return NULL;
  }
  struct link* old_head = list->head;
  void* data = old_head->val;
  list->head = old_head->next;
  free(old_head);
  list->numElements--;
  return data;
}
