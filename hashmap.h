#pragma once

#include <stdint.h>
#include <stddef.h>

#include "string_type.h"

typedef struct hm_entry
{
    string_type key;
    size_t value;
} hm_entry;

typedef struct hashmap
{
    int n_buckets;
    int bucket_capacity;
    int* bucket_element_count;
    hm_entry* bucket_elements;
} hashmap;

void init_hashmap(hashmap* hm, int n_buckets, int bucket_capacity);
void clear_hashmap(hashmap* hm);
void free_hashmap(hashmap* hm);

void hashmap_set(hashmap* hm, string_type key, size_t value);
int hashmap_get(const hashmap* hm, const string_type* key, size_t* value);
