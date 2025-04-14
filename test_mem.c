#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
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