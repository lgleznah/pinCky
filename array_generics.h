#pragma once

/////////////////////////////////////////////////////////////////////////
/// Generic definitions for Fixed element-Size Dynamic array (FSD array)
/////////////////////////////////////////////////////////////////////////

#define GEN_FUNC_NAME(name, type) name ## _ ## type ## _array
#define FSD_ARRAY_TYPE(type) type ## _array
#define EXPAND(val) val

#define MAKE_FSD_ARRAY_HEADERS(type, name) typedef struct {                                        \
    type* data;                                                                                    \
    size_t size;                                                                                   \
    size_t used;                                                                                   \
} EXPAND(FSD_ARRAY_TYPE(name);)                                                                    \
                                                                                                   \
void EXPAND(GEN_FUNC_NAME(init, name) (FSD_ARRAY_TYPE(name)* array, size_t initial_size));         \
void EXPAND(GEN_FUNC_NAME(free, name) (FSD_ARRAY_TYPE(name)* array));                              \
void EXPAND(GEN_FUNC_NAME(insert, name) (FSD_ARRAY_TYPE(name)* array, type value));

#define MAKE_FSD_ARRAY_FUNCTIONS(type, name)                                                       \
void EXPAND(GEN_FUNC_NAME(init, name) (FSD_ARRAY_TYPE(name)* array, size_t initial_size)) {        \
    array->size = initial_size;                                                                    \
    array->used = 0;                                                                               \
    array->data = (type*)malloc(initial_size * sizeof(type));                                      \
}                                                                                                  \
                                                                                                   \
void EXPAND(GEN_FUNC_NAME(insert, name) (FSD_ARRAY_TYPE(name)* array, type value)) {               \
    if (array->used == array->size)                                                                \
        {                                                                                          \
            array->size *= 2;                                                                      \
            void* tmp = realloc((void*) (array->data), array->size * sizeof(type));                \
            if (!tmp)                                                                              \
            {                                                                                      \
                printf("Realloc failed!\n");                                                       \
                exit(1);                                                                           \
            }                                                                                      \
            array->data = (type*)tmp;                                                              \
        }                                                                                          \
                                                                                                   \
    array->data[array->used] = value;                                                              \
    array->used++;                                                                                 \
}                                                                                                  \
                                                                                                   \
void EXPAND(GEN_FUNC_NAME(free, name) (FSD_ARRAY_TYPE(name)* array)) {                             \
    free((void*) (array->data));                                                                   \
    array->data = NULL;                                                                            \
    array->used = 0;                                                                               \
    array->size = 0;                                                                               \
}                                                                                     
