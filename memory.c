#include "memory.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>

MemoryHandler* memory_init(int size)
{
    MemoryHandler* handler = (MemoryHandler*)malloc(sizeof(MemoryHandler));
    if (handler == NULL) {
        puts("memory_init(): malloc failed");
        return NULL;
    }

    handler->memory = (void**)malloc(sizeof(void*) * size);
    if (handler->memory == NULL) {
        puts("memory_init(): malloc failed");
        free(handler);
        return NULL;
    }

    for (int i = 0; i < size; i++) {
    	handler->memory[i] = NULL;
    }
    handler->total_size = 0;

    handler->free_list = (Segment*)malloc(sizeof(Segment));
    if (handler->free_list == NULL) {
        puts("memory_init(): malloc failed");
        free(handler->memory);
        free(handler);
        return NULL;
    }

    handler->free_list->start = 0;
    handler->free_list->size = size;
    handler->free_list->next = NULL;

    handler->allocated = hashmap_create();
    if (handler->allocated == NULL) {
        puts("memory_init(): hashmap_create failed");
        free(handler->free_list);
        free(handler->memory);
        free(handler);
        return NULL;
    }

    return handler;
}

void memory_destroy(MemoryHandler* handler)
{
    if (handler == NULL) {
        return;
    }

    hashmap_destroy(handler->allocated);
    for (Segment* it = handler->free_list; it != NULL;) {
        Segment* tmp = it->next;
        free(it);
        it = tmp;
    }
    free(handler->memory);
    free(handler);
}

Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prec)
{
    if (handler == NULL) {
        return NULL;
    }

    (*prec) = NULL;
    Segment* it = handler->free_list;
    for (; it != NULL; (*prec) = it, it = it->next)
    {
        if ((it->start <= start) && ((it->start + it->size) >= (start + size))) {
            return it;
        }
    }
    return NULL;
}

int create_segment(MemoryHandler* handler, const char* name, int start, int size)
{
    if (handler == NULL) {
        return -1;
    }

    Segment* prec = NULL;
    Segment* cible = find_free_segment(handler, start, size, &prec);

    if (cible != NULL)
    {
        // Nouveau segment
        Segment* new_seg = (Segment*)malloc(sizeof(Segment));
        if (new_seg == NULL) {
            puts("create_segment(): malloc failed");
            return -2;
        }
        new_seg->start = start;
        new_seg->size = size;
        new_seg->next = NULL;

        // Suppression
        if (prec == NULL) {
            handler->free_list = cible->next;
        } else {
            prec->next = cible->next;
        }

        // Après
        if (((cible->start + cible->size) - (new_seg->start + new_seg->size)) > 0)
        {
            Segment* apres = (Segment*)malloc(sizeof(Segment));
            if (apres == NULL) {
                puts("create_segment(): malloc failed");
                free(new_seg);
                return -3;
            }

            apres->start = new_seg->start + new_seg->size;
            apres->size = (cible->start + cible->size) - (new_seg->start + new_seg->size);
            apres->next = handler->free_list;
            handler->free_list = apres;
        }

        if ((new_seg->start - cible->start) > 0)
        {
            Segment* avant = (Segment*)malloc(sizeof(Segment));
            if (avant == NULL) {
                puts("create_segment(): malloc failed");
                free(new_seg);
                // Encore un free
                return -4;
            }

            avant->start = cible->start;
            avant->size = new_seg->start - cible->start;
            avant->next = handler->free_list;
            handler->free_list = avant;
        }

        free(cible);

        // Attributs
        handler->total_size += size;
        if (hashmap_insert(handler->allocated, name, new_seg) < 0) {
            puts("create_segment(): hashmap_insert failed");
            free(new_seg);
            // Encore deux free
            return -5;
        }

        return 0;
    }
    return -1;
}

int remove_segment(MemoryHandler *handler, const char *name)
{
    if (handler == NULL) {
        return -1;
    }

    Segment* cible = hashmap_get(handler->allocated, name);

    if (!cible) {
        puts("remove_segment: hashmap_get failed");
        return -2;
    }

    if (hashmap_remove(handler->allocated, name) < 0) {
        puts("remove_segment: hashmap_remove failed");
        return -3;
    }

    handler->total_size -= cible->size;

    Segment* avant = NULL;
    Segment* apres = NULL;
    Segment* prec_avant = NULL;
    Segment* prec_apres = NULL;

    for (Segment *it = handler->free_list, *prec = NULL; it != NULL; prec = it, it = it->next) {
        if ((it->start + it->size) == cible->start) {
            avant = it;
            prec_avant = prec;
        }
        if ((cible->start + cible->size) == it->start) {
            apres = it;
            prec_apres = prec;
        }
    }

    Segment* new_seg = (Segment*)malloc(sizeof(Segment));
    if (new_seg == NULL) {
        puts("remove_segment: malloc failed");
        return -4;
    }
    new_seg->start = cible->start;
    new_seg->size = cible->size;

    free(cible);

    if (avant != NULL) {
        if (prec_avant == NULL) {
            handler->free_list = avant->next;
        } else {
            prec_avant->next = avant->next;
        }

        new_seg->start -= avant->size;
        new_seg->size += avant->size;
        free(avant);
    }
    if (apres != NULL) {
        if (prec_apres == NULL) {
            handler->free_list = apres->next;
        } else {
            prec_apres->next = apres->next;
        }

        new_seg->size += apres->size;
        free(apres);
    }

    new_seg->next = handler->free_list;
    handler->free_list = new_seg;

    return 0;
}
