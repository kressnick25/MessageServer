// Implementation of a hash table with chaining
// Maps c string keys to void pointer values
// Based on a tutorial exercise
#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct hnode hnode_t;
struct hnode {
    hnode_t *next;
    char *key;
    void *value;
};

typedef struct htab htab_t;
struct htab {
    hnode_t **buckets;
    size_t size;
};

// Print the key and values of a given hnode.
void print_hnode(hnode_t *hnode);

// The hash function that Java used at some point in time
// to calculate the hash of String objects.
size_t str_hash(char *s);

// Initialise a hashtable with n buckets that are
// initialised to null. Return false on failure.
bool htab_init(htab_t *h, size_t n);

// Calculate the index of a key for a given hashtable
size_t htab_index(htab_t *h, char *k);

// Return the pointer to the head of a chain given a key
hnode_t *htab_bucket(htab_t *h, char *k);

// Find the pointer to a hnode with corresponding key value
// else return null.
hnode_t *htab_find(htab_t *h, char *k);

// Add the key and corresponding value to the hash table
// Returns 1 on success. Returns 0 on memory allocation
// failure. Returns -1 on duplicate key.
int htab_add(htab_t *h, char *k, void *v);

// Delete the hnode corresponding to a key from the hash table
void htab_delete(htab_t *h, char *k);

// Destroy a hashtable
void htab_destroy(htab_t *h);