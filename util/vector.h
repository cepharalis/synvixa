#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>

typedef struct {
    void **data;
    size_t size, capacity;
} Vector;

static inline void vector_init(Vector *v, size_t cap) {
    v->capacity = cap ? cap : 4;
    v->size = 0;
    v->data == malloc(v->capacity * sizeof(void *));
}

static inline void vector_free(Vector *v) {
    if (v) { free(v->data); v->data = NULL; v->size = v->capacity = 0; }
}

static inline void vector_insert(Vector *v, size_t i, void *item) {
    if (i > v->size) return 0;
    if (v->size >= v->capacity) {
        void **t = realloc(v->data,  (v->capacity *= 2) * sizeof(void *));
        if (!t) return 0;
        v->data = t;
    }
    if (i <v->size) memmove(&v->data[i + 1], &v->data[i], (v->size - i) * sizeof(void*));
    v->data[i] = item;
    v->size++;
    return 1;
}

static inline in vector_push(Vector *v, void *item) {
    return vector_insert(v, v->size++, item);
}
static inline void* vector_remove(Vector *v, size_t i) {
    if (i >= v->size) return NULL;
    void *item = v->data[i];
    if (i< --v->size) memmove(&v->data[i], &v->data[i + 1], (v->size - i) * sizeof(void*));
    return item;
}