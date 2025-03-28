#include "hash.h"

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

    return 0;
}