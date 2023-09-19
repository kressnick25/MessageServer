#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

void print_hnode(hnode_t *n) {
    printf("key: %s -> value: %p\n", n->key, n->value);
}

size_t str_hash(char *s) {
    // We use size_t because it is guaranteed to be able
    // to hold the index of any array. It is also
    // guarenteed to be compatible with sizeof.
    size_t h = 0;
    char c;

    while ((c = *s++) != '\0') {
        h = 31*h + c;
    }

    return h;
}

bool htab_init(htab_t *h, size_t n) {
    hnode_t **b = calloc(n, sizeof(hnode_t *));

    h->buckets = b;
    h->size = n;

    return b != NULL;
}

size_t htab_index(htab_t *h, char *k) {
    return str_hash(k) % h->size;
}

hnode_t *htab_bucket(htab_t *h, char *k) {
    return h->buckets[htab_index(h, k)];
}

hnode_t *htab_find(htab_t *h, char *k) {
    hnode_t *b = htab_bucket(h, k);

    while (b != NULL) {
        if (strcmp(b->key, k) == 0) {
            return b;
        }
        b = b->next;
    }

    return NULL;
}

int htab_add(htab_t *h, char *k, void *v) {
    // Checking if this is a duplicate key
    if (htab_find(h, k) != NULL) {
        // Duplicate keys not allowed
        return -1;
    }

    // Creating the hnode_t to add
    hnode_t *n = calloc(1, sizeof(hnode_t));
    if (n == NULL) {
        return 0;
    }
    n->key = k;
    n->value = v;

    // Getting the pointer to the head of the chain if any
    hnode_t *b = htab_bucket(h, k);
    if (b != NULL) {
        while (b->next != NULL) {
            b = b->next;
        }
        b->next = n;
    } else {
        h->buckets[htab_index(h, k)] = n;
    }

    return 1;
}

void htab_delete(htab_t *h, char *k) {
    hnode_t *cur = htab_bucket(h, k);
    hnode_t *prv = NULL;
    if (cur == NULL) {
        return;
    }

    while (cur != NULL) {
        if (strcmp(cur->key, k) == 0) {
            if (cur != NULL) {
                prv->next = cur->next;
            } else {
                h->buckets[htab_index(h, k)] = cur->next;
            }

            free(cur);
            break;
        }
        prv = cur;
        cur = cur->next;
    }
}

void htab_destroy(htab_t *h) {
    // Free chains
    for (size_t i = 0; i < h->size; i++) {
        hnode_t *c = h->buckets[i];
        while (c != NULL) {
            hnode_t *t = c->next;
            free(c);
            c = t;
        }
    }

    // Free buckets
    free(h->buckets);
    h->buckets = NULL;
    h->size = 0;
}