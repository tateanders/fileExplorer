/*-------------------------------------------------------------------------------------------------
    This file is largely based off of projects from my data structures class
-------------------------------------------------------------------------------------------------*/
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
void dynarray_free(struct dynarray*);
int dynarray_size(struct dynarray*);
int dynarray_capacity(struct dynarray*);
void dynarray_push(struct dynarray*, void*);
void dynarray_insert(struct dynarray*, void*);
void dynarray_remove(struct dynarray*, int);
void* dynarray_get(struct dynarray*, int);
void dynarray_set(struct dynarray*, int, void*);

#endif
