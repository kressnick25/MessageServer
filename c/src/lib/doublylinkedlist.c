#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "doublylinkedlist.h"

node_t *create_node(void) {
    node_t *n = calloc(1, sizeof(node_t));
    return n;
}

void print_doubly_linked_list(node_t *list) {
    // Base case where list is NULL
    if (list == NULL) {
        printf("NULL (EMPTY LIST)\n");
        return;
    }

    // Return to head of list
    node_t *n = list;
    while (n->prev != NULL) {
        n = n->prev;
    }

    // Printing from head to tail
    printf("From head to tail:\n");
    while (n->next != NULL) {
        printf("%s -> ", (char*)n->data);
        n = n->next;
    }
    printf("%s -> ", (char*)n->data);
    printf("NULL (TAIL)\n");
    
    // Printing from tail to head
    printf("From tail to head:\n");
    while (n->prev != NULL) {
        printf("%s -> ", (char*)n->data);
        n = n->prev;
    }
    printf("%s -> ", (char*)n->data);
    printf("NULL (HEAD)\n");
}

void insert_node(node_t **list, node_t *n) {
    // Make sure n is actually singular
    n->prev = NULL;
    n->next = NULL;
    if (*list == NULL) {
        // Base case if list is empty
        *list = n;
    } else if ((*list)->prev == NULL) {
        // Case list is head
        n->next = *list;
        (*list)->prev = n;
    } else {
        // Case list is not head
        n->prev = (*list)->prev;
        n->next = *list;
        (*list)->prev->next = n;
        (*list)->prev = n;
    }
}

void delete_node(node_t **n, bool free_data) {
    // First we splice the node from the list
    //if (((*n)->prev == NULL) && ((*n)->next == NULL)) {
    //    // Base case where list only has one node
    //}
    
    if ((*n)->prev != NULL) {
        // Deal with previous node
        (*n)->prev->next = (*n)->next;
    }

    if ((*n)->next != NULL) {
        // Deal with next node
        (*n)->next->prev = (*n)->prev;
    }

    // Actually remove the spliced node
    // Free the contained data
    if (free_data) {
        free((*n)->data);
    }
    // Free the node
    free(*n);
    // Change the original reference to NULL
    *n = NULL;
}

void destroy_doubly_linked_list(node_t **list, bool free_data) {
    // Move to the head
    node_t *head = *list;
    while (head->prev != NULL) {
        head = head->prev;
    }

    do {
        // Save the current head
        node_t *cur = head;
        // Move to next
        head = head->next;
        // Delete node
        delete_node(&cur, free_data);
    } while (head != NULL);
    
    // Set the original list pointer to NULL
    *list = NULL;
}