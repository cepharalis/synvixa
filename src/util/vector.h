#pragma once

#include <stdlib.h>
#include <string.h>

#define DEFINE_VECTOR(T) \
typedef struct { \
	T value; \
	bool has_value; \
} T##_optional; \
typedef struct { \
    T* data; \
    size_t size, capacity; \
} T##_vector; \
static inline void T##_vector_init(T##_vector* v, size_t cap) { \
    v->capacity = cap ? cap : 4; \
    v->size = 0; \
    v->data = malloc(v->capacity * sizeof(T)); \
} \
static inline void T##_vector_free(T##_vector* v) { \
    if (v) { free(v->data); v->data = nullptr; v->size = v->capacity = 0; } \
} \
static inline int T##_vector_insert(T##_vector* v, size_t i, T item) { \
    if (i > v->size) return 0; \
    if (v->size >= v->capacity) { \
        T* t = realloc(v->data,  (v->capacity *= 2) * sizeof(T)); \
        if (!t) return 0; \
        v->data = t; \
    } \
    if (i <v->size) memmove(&v->data[i + 1], &v->data[i], (v->size - i) * sizeof(T)); \
    v->data[i] = item; \
    v->size++; \
    return 1; \
} \
static inline int T##_vector_push(T##_vector* v, T item) { \
    return T##_vector_insert(v, v->size, item); \
} \
static inline T##_optional T##_vector_remove(T##_vector* v, size_t i) { \
    if (i >= v->size) { \
		T##_optional t; \
		t.has_value = false; \
		return t; \
	} \
    T item = v->data[i]; \
    if (i< --v->size) memmove(&v->data[i], &v->data[i + 1], (v->size - i) * sizeof(T)); \
    return (T##_optional){item, true}; \
} \
static inline T##_optional T##_vector_get(T##_vector* v, size_t i) { \
    if (i >= v->size) { \
		T##_optional t; \
		t.has_value = false; \
		return t; \
	} \
    return (T##_optional){v->data[i], true}; \
}

DEFINE_VECTOR(char)