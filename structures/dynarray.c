/*-------------------------------------------------------------------------------------------------
    This file is largely based off of projects from my data structures class
-------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <assert.h>
#include "dynarray.h"

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

struct dynarray {
	void** data;
	int size;
	int capacity;
};

/*-------------------------------------------------------------------------------------------------
    Create/Free
-------------------------------------------------------------------------------------------------*/

struct dynarray* dynarray_create() {
	struct dynarray* arr = calloc(1, sizeof(struct dynarray));
	assert(arr);
	arr->data = calloc(2, sizeof(void*));
	assert(arr->data);
	arr->capacity = 2;
	arr->size = 0;
	return arr;
}

void dynarray_free(struct dynarray* arr) {
	assert(arr);
	free(arr->data);
	free(arr);
}

/*-------------------------------------------------------------------------------------------------
    Setters
-------------------------------------------------------------------------------------------------*/

void dynarray_push(struct dynarray* arr, void* val) {
	assert(arr);
	if(arr->size == arr->capacity){
		arr->capacity *= 2;
		arr->data = realloc(arr->data, arr->capacity * sizeof(void*));
	}

	arr->size += 1;
	int i;
	for (i = arr->size; i > 0; i--) {
		arr->data[i] = arr->data[i - 1];
	}

	arr->data[0] = val;
}

void dynarray_insert(struct dynarray* arr, void* val) {
	assert(arr);
	if(arr->size == arr->capacity){
		arr->capacity *= 2;
		arr->data = realloc(arr->data, arr->capacity * sizeof(void*));
	}
	arr->data[arr->size] = val;
	arr->size += 1;
}

void dynarray_remove(struct dynarray* arr, int idx) {
	assert(arr);
	if(arr->data[idx] != NULL){
		arr->data[idx] = NULL;
		for (int i = idx; i < arr->size; i++){
			arr->data[i] = arr->data[i + 1];
		}
		if(arr->data[arr->size] != NULL){
			arr->data[arr->size] = NULL;
		}
		arr->size -= 1;
		return;
	}
	return;
}

void dynarray_set(struct dynarray* arr, int idx, void* val) {
	assert(arr);
	assert(idx < arr->size && idx >= 0);

	arr->data[idx] = val;
}

/*-------------------------------------------------------------------------------------------------
    Getters
-------------------------------------------------------------------------------------------------*/

int dynarray_size(struct dynarray* arr) {
	assert(arr);
	return arr->size;
}

int dynarray_capacity(struct dynarray* arr){
	assert(arr);
	return arr->capacity;
}

void* dynarray_get(struct dynarray* arr, int idx) {
	assert(arr);
	assert(idx >= 0);

	return arr->data[idx];
}
