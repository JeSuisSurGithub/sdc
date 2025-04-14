#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long simple_hash(const char* str)
{
    unsigned long hash = 0;
    unsigned int i = strlen(str);
    for (const char* p = str; i > 0; i--, p++) {
        hash = hash * 19 + (*p);
    }
    return hash;
}

HashMap* hashmap_create()
{
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    if (map == NULL) {
        puts("hashmap_create(): malloc failed");
        return NULL;
    }

    map->size = 0;
    map->table = (HashEntry*)malloc(sizeof(HashEntry) * TABLE_SIZE);
    if (map == NULL) {
        puts("hashmap_create(): malloc failed (2)");
        free(map);
        return NULL;
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        map->table[i].key = NULL;
        map->table[i].value = NULL;
    }
    return map;
}

int hashmap_insert(HashMap* map, const char* key, void* value)
{
    if ((map == NULL) || (key == NULL) || (map->size >= TABLE_SIZE)) {
        puts("hashmap_insert: invalid arguments");
        return -1;
    }

    unsigned long hash_depart = simple_hash(key) % TABLE_SIZE;
    unsigned long idx = 0;
    unsigned long hash = (hash_depart + idx) % TABLE_SIZE;

    do {
        if (map->table[hash].key == NULL || map->table[hash].key == TOMBSTONE) {
            map->table[hash].key = strdup(key);
            map->table[hash].value = value;
            map->size++;
            return idx;
        }
        hash = (hash + (++idx)) % TABLE_SIZE;
    } while (hash != hash_depart);

    puts("hashmap_insert: hashmap is full");
    return -2;
}

void* hashmap_get(HashMap* map, const char* key)
{
    if ((map == NULL) || (key == NULL)) {
        puts("hashmap_get: invalid arguments");
        return NULL;
    }

    unsigned long hash_depart = simple_hash(key) % TABLE_SIZE;
    unsigned long idx = 0;
    unsigned long hash = (hash_depart + idx) % TABLE_SIZE;

    do {
        if (map->table[hash].key == NULL) {
            return NULL;
        } else if (map->table[hash_depart].key == TOMBSTONE) {
            continue;
        } else if (strcmp(map->table[hash].key, key) == 0) {
            return map->table[hash].value;
        } else {
            printf("hashmap_get: unknown error searching for %s\n", key);
        }
        hash = (hash + (++idx)) % TABLE_SIZE;
    } while (hash != hash_depart);
    return NULL;
}

int hashmap_remove(HashMap* map, const char* key)
{
    if (map == NULL || key == NULL) {
        puts("hashmap_remove: invalid arguments");
        return -1;
    }

    unsigned long hash_depart = simple_hash(key) % TABLE_SIZE;
    unsigned long idx = 0;
    unsigned long hash = (hash_depart + idx) % TABLE_SIZE;

    do {
        if (map->table[hash].key == NULL) {
            return -2;
        } else if (map->table[hash_depart].key == TOMBSTONE) {
            continue;
        } else if (strcmp(map->table[hash].key, key) == 0) {
            free(map->table[hash].key);
            map->table[hash].key = TOMBSTONE;
            map->table[hash].value = NULL;
            map->size--;
            return 0;
        } else {
            puts("hashmap_remove: unknown error");
        }
        hash = (hash + (++idx)) % TABLE_SIZE;
    } while (hash != hash_depart);

    return -3;
}

void hashmap_destroy(HashMap* map)
{
    if (map == NULL) {
        puts("hashmap_destroy: invalid arguments");
        return;
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (map->table[i].key != NULL && map->table[i].key != TOMBSTONE) {
            free(map->table[i].key);
        }
    }
    free(map->table);
    free(map);
}