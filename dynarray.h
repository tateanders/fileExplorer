#ifndef __DYNARRAY_H
#define __DYNARRAY_H

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

struct dynarray;

/*-------------------------------------------------------------------------------------------------
    Functions
-------------------------------------------------------------------------------------------------*/

struct dynarray* dynarray_create();
void dynarray_free(struct dynarray* arr);
int dynarray_size(struct dynarray* arr);
int dynarray_capacity(struct dynarray* arr);
void dynarray_insert(struct dynarray* arr, void* val);
void dynarray_remove(struct dynarray* arr, int idx);
void* dynarray_get(struct dynarray* arr, int idx);
void dynarray_set(struct dynarray* arr, int idx, void* val);

#endif
