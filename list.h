#ifndef __LIST_H
#define __LIST_H

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

struct list;

/*-------------------------------------------------------------------------------------------------
    functions
-------------------------------------------------------------------------------------------------*/

struct list* list_create();
void list_free(struct list* list);
void list_insert(struct list* list, void* val);
void* list_getElement(struct list* list, int position);
int list_getNumElements(struct list* list);
void* list_pop(struct list* list);

#endif
