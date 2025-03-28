#include "hash.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    int i1 = 1; const char c1[] = "un";
    int i2 = 2; const char c2[] = "deux";
    int i3 = 3; const char c3[] = "trois";

    HashMap* map = hashmap_create();
    hashmap_insert(map, c1, &i1);
    hashmap_insert(map, c2, &i2);
    hashmap_insert(map, c3, &i3);

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (map->table[i].key != NULL) {
            printf("Hashmap* %i %s %i %p\n", i, map->table[i].key, *(int*)(map->table[i].value), &map->table[i]);
        }
    }

    printf("Hashmap@'un' %s %i\n", c1, *(int*)hashmap_get(map, c1));
    printf("Hashmap@'deux' %s %i\n", c2, *(int*)hashmap_get(map, c2));
    printf("Hashmap@'trois' %s %i\n", c3, *(int*)hashmap_get(map, c3));

    hashmap_remove(map, c1);
    hashmap_remove(map, c3);

    printf("Hashmap@'un' %s %p\n", c1, hashmap_get(map, c1));
    printf("Hashmap@'deux' %s %p\n", c2, hashmap_get(map, c2));
    printf("Hashmap@'trois' %s %p\n", c3, hashmap_get(map, c3));

    hashmap_destroy(map);

    MemoryHandler* hmem = memory_init(1024);

    create_segment(hmem, "mid", 256, 512);
    Segment* mid = hashmap_get(hmem->allocated, "mid");
    create_segment(hmem, "high", 0, 256);
    Segment* high = hashmap_get(hmem->allocated, "high");
    create_segment(hmem, "low", 768, 256);
    Segment* low = hashmap_get(hmem->allocated, "low");

    printf("MemoryHandler@'mid' %p %i %i\n", mid, mid->start, mid->size);
    printf("MemoryHandler@'high' %p %i %i\n", high, high->start, high->size);
    printf("MemoryHandler@'low' %p %i %i\n", low, low->start, low->size);

    remove_segment(hmem, "high");
    for (Segment* it = hmem->free_list; it != NULL; it = it->next) {
        printf("Hashmap* %p %i %i\n", it, it->start, it->size);
    }

    remove_segment(hmem, "low");
    for (Segment* it = hmem->free_list; it != NULL; it = it->next) {
        printf("Hashmap* %p %i %i\n", it, it->start, it->size);
    }

    remove_segment(hmem, "mid");
    for (Segment* it = hmem->free_list; it != NULL; it = it->next) {
        printf("Hashmap* %p %i %i\n", it, it->start, it->size);
    }

    memory_destroy(hmem);

    return 0;
}