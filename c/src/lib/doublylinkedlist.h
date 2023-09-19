#pragma once

#include <stdbool.h>

typedef struct node node_t;
struct node {
    node_t *prev;
    node_t *next;
    void *data;
};

// Print a linked list from head to tail then from tail to head
void print_doubly_linked_list(node_t *list);

// Creates an empty doubly linked list node and returns the
// pointer to it. Returns a NULL pointer on failure.
node_t *create_node(void);

// Add a (SINGULAR) node to list.
void insert_node(node_t **list, node_t *n);

// Delete a node specified by n in list. Option to only free
// the node_t data structure without freeing data itself
// can be done by setting free_data.
void delete_node(node_t **n, bool free_data);

// Destroy a doubly linked list. Option to only free
// the node_t data structure without freeing data itself
// can be done by setting free_data.
void destroy_doubly_linked_list(node_t **list, bool free_data);