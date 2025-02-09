#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void init_hashmap(hashmap* hm, int n_buckets, int bucket_capacity)
{
    if (n_buckets <= 0 || bucket_capacity <= 0)
    {
        printf("Cannot initialise hashmap with zero or less buckets or bucket capacity. Defaulting to 32 buckets and 32 capacity");
        n_buckets = 32;
        bucket_capacity = 32;
    }

    hm->bucket_element_count = calloc(n_buckets, sizeof(int));
    hm->bucket_elements = malloc(sizeof(hm_entry) * n_buckets * bucket_capacity);

    hm->bucket_capacity = bucket_capacity;
    hm->n_buckets = n_buckets;
}

void clear_hashmap(hashmap* hm)
{
    for (int i = 0; i < hm->n_buckets; i++)
    {
        hm->bucket_element_count[i] = 0;
    }
}

void free_hashmap(hashmap* hm)
{
    free(hm->bucket_elements);
    free(hm->bucket_element_count);

    hm->bucket_capacity = 0;
    hm->n_buckets = 0;
}

void hashmap_set(hashmap* hm, string_type key, size_t value)
{
    unsigned int hash = string_hash(&key) % hm->n_buckets;
    int bucket_usage = hm->bucket_element_count[hash];
    hm_entry* bucket = &hm->bucket_elements[(int)(hash * hm->bucket_capacity)];

    // If the element exists, replace it with new value. Otherwise, add it to the hashmap
    for (int i = 0; i < bucket_usage; i++)
    {
        if (string_comparison(&bucket[i].key, &key, COMPARE_EQ))
        {
            bucket[i].value = value;
            return;
        }
    }

    // No matching elements were found. Add the element (and expand the table if necessary)
    if (bucket_usage >= hm->bucket_capacity)
    {
        hm->bucket_capacity = (int) (hm->bucket_capacity * 1.7);
        void* temp = realloc(hm->bucket_elements, sizeof(hm_entry) * hm->n_buckets * hm->bucket_capacity);
        if (temp == NULL)
        {
            PRINT_ERROR_AND_QUIT("Failed to reallocate hashmap bucket!\n");
        }
        hm->bucket_elements = temp;
    }

    bucket = &hm->bucket_elements[(int)(hash * hm->bucket_capacity)];
    bucket[bucket_usage].key = key;
    bucket[bucket_usage].value = value;

    hm->bucket_element_count[hash]++;
    return;
}

int hashmap_get(const hashmap* hm, const string_type* key, size_t* value)
{
    unsigned int hash = string_hash(key) % hm->n_buckets;
    int bucket_usage = hm->bucket_element_count[hash];
    hm_entry* bucket = &hm->bucket_elements[(int)(hash * hm->bucket_capacity)];

    // If the element exists, return it. Otherwise, show an error
    for (int i = 0; i < bucket_usage; i++)
    {
        if (string_comparison(&bucket[i].key, key, COMPARE_EQ))
        {
            *value = bucket[i].value;
            return 0;
        }
    }

    return -1;
}