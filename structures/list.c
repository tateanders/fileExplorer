/*-------------------------------------------------------------------------------------------------
    This file is largely based off of projects from my data structures class
-------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <assert.h>
#include "list.h"

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

struct link {
	void* val;
	struct link* next;
};

struct list {
	struct link* head;
	int numElements;
};

/*-------------------------------------------------------------------------------------------------
    Create/Free
-------------------------------------------------------------------------------------------------*/

struct list* list_create() {
	struct list* list = calloc(1, sizeof(struct list));
	assert(list);

	list->head = NULL;
	list->numElements = 0;

	return list;
}

void list_free(struct list* list) {
	assert(list);

	struct link* next = list->head;
	struct link* curr = list->head;
	while (curr != NULL) {
		next = curr->next;
		free(curr);
		curr = next;
	}

	free(list);
}

/*-------------------------------------------------------------------------------------------------
    Setter
-------------------------------------------------------------------------------------------------*/

void list_insert(struct list* list, void* val) {
	assert(list);

	struct link* link = calloc(1, sizeof(struct link));
	link->val = val;
	link->next = list->head;
	list->head = link;
	list->numElements++;
}

/*-------------------------------------------------------------------------------------------------
    Getters
-------------------------------------------------------------------------------------------------*/

void* list_getElement(struct list* list, int position){
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

int list_getNumElements(struct list* list){
	assert(list);
	return list->numElements;
}

void* list_pop(struct list* list) {
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
