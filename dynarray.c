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
	//int start;
};

/*-------------------------------------------------------------------------------------------------
    Create/Free
-------------------------------------------------------------------------------------------------*/
struct dynarray* dynarray_create() {
	struct dynarray* arr = calloc(1, sizeof(struct dynarray));
	arr->data = calloc(2, sizeof(void*));
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

void dynarray_insert(struct dynarray* da, void* val) {
	if(da->size == da->capacity){
		da->capacity *= 2;
		da->data = realloc(da->data, da->capacity * sizeof(void*));
	}
	da->data[da->size] = val;
	da->size += 1;
}

void dynarray_remove(struct dynarray* da, int idx) {
	if(da->data[idx] != NULL){
		da->data[idx] = NULL;
		for (int i = idx; i < da->size; i++){
			da->data[i] = da->data[i + 1];
		}
		if(da->data[da->size] != NULL){
			da->data[da->size] = NULL;
		}
		da->size -= 1;
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
