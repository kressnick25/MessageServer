#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "linkedlist.h"

/* LIVEFEED */
livefeed_t* livefeed_add(livefeed_t* head, int user_id) {
    if (head == NULL){
        livefeed_t* new = (livefeed_t*) malloc(sizeof(livefeed_t));
        new->user_id = user_id;
        new->next = NULL;
        return new;
    }
    livefeed_t* n = head;
    while (n->next != NULL) {
        n = n->next;
    }
    livefeed_t* new = (livefeed_t*) malloc(sizeof(livefeed_t));
    new->user_id = user_id;
    n->next = new;

    return head;
}

// remove first node
livefeed_t* livefeed_remove_front(livefeed_t* head) {
    if(head == NULL){
        return NULL;
    }
    livefeed_t* front = head;
    head = head->next;
    front->next = NULL;
    if(front == head) { // last node
        head = NULL;
    }
    free(front);
    return head;
}

// remove last node
livefeed_t* livefeed_remove_last(livefeed_t* head) {
    if(head == NULL)
        return NULL;
 
    livefeed_t* n = head;
    livefeed_t* back = NULL;
    while(n->next != NULL) {
        back = n;
        n = n->next;
    }
    if(back != NULL) {
        back->next = NULL;
    }
    /* if this is the last node in the list*/
    if(n == head) {
        head = NULL;
    }
    free(n);
    return head;
}

livefeed_t* livefeed_get_by_id(livefeed_t* head, int user_id) {
    livefeed_t* n = head;
    while (n->next != NULL) {
        if (n->user_id == user_id) {
            return n;
        }
        n = n->next;
    }
    if (n->user_id == user_id){
        return n;
    }
    fprintf(stderr, "ERROR: Failed to retreive user, user not exist in livefeed\n");
    return NULL;
}

// unsubsribe user from a channel
livefeed_t* livefeed_remove(livefeed_t* head, int user_id) {
    livefeed_t* channel = livefeed_get_by_id(head, user_id);
    if (channel == NULL){
        return head;
    }
    /* if the node is the first node */
    if(channel == head) {
        head = livefeed_remove_front(head);
        return head;
    }
    /* if the node is the last node */
    if(channel->next == NULL) {
        head = livefeed_remove_last(head);
        return head;
    }
    /* if the node is in the middle */
    livefeed_t* n = head;
    while(n != NULL) {
        if(n->next == channel) {
            break;
        }
        n = n->next;
    }
    if(n != NULL) {
        livefeed_t* tmp = n->next;
        n->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }
    return head;
}

void livefeed_free(livefeed_t* head) {
    livefeed_t *n, *tmp;

    if(head != NULL) {
        n = head->next;
        head->next = NULL;
        while(n != NULL)
        {
            tmp = n->next;
            free(n);
            n = tmp;
        }
    }
}


/* MESSAGES*/

// init a new message
message_t* message_init(int id, char* text, message_t* next) {
    message_t* new = (message_t*) malloc(sizeof(message_t));
    new->id = id;
    new->text = malloc( strlen(text) + 1 * sizeof(char));
    strcpy(new->text, text);
    new->next = next;

    return new;
}

// add a new message to the messages list
message_t* message_add(message_t* head, char* text) {
    if (head == NULL) {
        head = message_init(0, text, NULL);
        return head;
    }
    // move to lastnode
    message_t* n = head;
    int id = 1;
    while(n->next != NULL){
        id++;
        n = n->next; 
    }
    message_t* new_node = message_init(id, text, NULL);
    n->next = new_node;

    return head;
}

// get a message by id
message_t* message_get_by_id(message_t* head, int id) {
    if (head == NULL)
        return NULL;
    message_t* n = head;
    while(n->next != NULL){
        if (n->id == id){
            return n;
        }
        n = n->next;
    }
    if (n->id == id) { // check last node
        return n;
    }
    //fprintf(stderr, "ERROR: Failed to retreive message, message id does not exist\n");
    return NULL;
}

// free all memory used by messages in list
void messages_free(message_t* head) {
    message_t *n, *tmp;

    if(head != NULL) {
        n = head->next;
        head->next = NULL;
        while(n != NULL) {
            tmp = n->next;
            free(n);
            n = tmp;
        }
    }
}


/* CHANNELS */
// init a new channel
channel_t* channel_init(char id) {
    channel_t* new = calloc(1, sizeof(channel_t));
    new->id = id;
    new->total_messages = 0;

    return new;
}

void channel_free(channel_t* channel){
    messages_free(channel->message_list);
    livefeed_free(channel->livefeed_list);
}

/* USER CHANNELS */
// init a new user_channel
uchannel_t* uchannel_create(channel_t* channel, uchannel_t* next) {
    uchannel_t* new = (uchannel_t*) malloc(sizeof(uchannel_t));
    new->read_msg = 0;
    new->channel = channel;
    new->next = next;
    
    return new;
}

// subsribe a user to an existing channel
uchannel_t* uchannel_add_to_user(uchannel_t* head, channel_t* channel) {
    if (head == NULL) {
        uchannel_t* head = uchannel_create(channel, NULL);
        return head;
    }
    // move to lastnode
    uchannel_t* n = head;
    while(n->next != NULL){
        n = n->next; 
    }
    uchannel_t* new_node = uchannel_create(channel, NULL);
    n->next = new_node;

    return head;
}

void uchannels_free(uchannel_t* head) {
    uchannel_t *n, *tmp;

    if(head != NULL) {
        n = head->next;
        head->next = NULL;
        while(n != NULL)
        {
            tmp = n->next;
            free(n);
            n = tmp;
        }
    }
    free(head);
}

// retrieve a channel assigned to a user by id. return NULL if not found
uchannel_t* uchannel_get_by_id(uchannel_t* head, int id) {
    if (head == NULL){
        return NULL;
    }
    uchannel_t* n = head;
    while(n->next != NULL){
        if (n->channel->id == id){
            return n;
        }
        n = n->next;
    };
    // check last node
    if (n->channel->id == id){
        return n;
    }
    //fprintf(stderr, "ERROR: Failed to get uchannel, user not subscribed to channel\n");
    return NULL;
}

// remove first node
uchannel_t* uchannel_remove_front(uchannel_t* head) {
    if(head == NULL){
        return NULL;
    }

    uchannel_t* front = head;
    head = head->next;
    front->next = NULL;
    if(front == head) { // last node
        head = NULL;
    }
    free(front);

    return head;
}

// remove last node
uchannel_t* uchannel_remove_last(uchannel_t* head) {
    if(head == NULL)
        return NULL;
 
    uchannel_t* n = head;
    uchannel_t* back = NULL;
    while(n->next != NULL) {
        back = n;
        n = n->next;
    }
    if(back != NULL) {
        back->next = NULL;
    }
    /* if this is the last node in the list*/
    if(n == head) {
        head = NULL;
    }
 
    free(n);
 
    return head;
}

// unsubsribe user from a channel
uchannel_t* uchannel_remove(uchannel_t* head, int channel_id) {
    uchannel_t* channel = uchannel_get_by_id(head, channel_id);
    if (channel == NULL) {
        fprintf(stderr, "ERROR: Failed to remove channel, user not subscribed to channel\n");
        return head;
    }
    /* if the node is the first node */
    if(channel == head) {
        head = uchannel_remove_front(head);
        return head;
    }
 
    /* if the node is the last node */
    if(channel->next == NULL) {
        head = uchannel_remove_last(head);
        return head;
    }
 
    /* if the node is in the middle */
    uchannel_t* n = head;
    while(n != NULL) {
        if(n->next == channel) {
            break;
        }
        n = n->next;
    }
 
    if(n != NULL) {
        uchannel_t* tmp = n->next;
        n->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }

    return head;
}