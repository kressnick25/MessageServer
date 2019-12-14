#pragma once
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <stdbool.h>

// messages for specific channel as a list
typedef struct message message_t;
struct message {
    int id;
    char* text;
    message_t* next;
};

// list of users to send livefeed messages to
typedef struct livefeed livefeed_t;
struct livefeed {
    int user_id; // TODO change to user *
    livefeed_t* next;
};

// channel containing info and list of messages
// stored in array
typedef struct channel channel_t;
struct channel {
    char id;
    int total_messages;
    message_t* message_list;
    livefeed_t* livefeed_list;
};

// linked list assigned to a user
// gives user info related to channel and links to channel
typedef struct uchannel uchannel_t;
struct uchannel {
    int read_msg;
    message_t* current_message;
    channel_t* channel;
    uchannel_t* next;
};

/* LIVEFEED */
livefeed_t* livefeed_add(livefeed_t* head, int user_id);

// remove first node
livefeed_t* livefeed_remove_front(livefeed_t* head);

// remove last node
livefeed_t* livefeed_remove_last(livefeed_t* head);

// unsubsribe user from a channel
livefeed_t* livefeed_remove(livefeed_t* head, int channel_id);

void livefeed_free(livefeed_t* head);

/* MESSAGES*/
// init a new message
message_t* message_init(int id, char* text, message_t* next);

// add a new message to the messages list
message_t* message_add(message_t* head, char* text);

// get a message by id
message_t* message_get_by_id(message_t* head, int id);

// free all memory used by messages in list
void messages_free(message_t* head);

/* CHANNEL */ 
// init a new channel
channel_t* channel_init(char id);

/* USER CHANNELS */

// init a new user_channel
uchannel_t* uchannel_create(channel_t* channel, uchannel_t* next);

// subsribe a user to an existing channel
uchannel_t* uchannel_add_to_user(uchannel_t* head, channel_t* channel);

void uchannels_free(uchannel_t* head);

// retrieve a channel assigned to a user by id. return NULL if not found
uchannel_t* uchannel_get_by_id(uchannel_t* head, int id);

// remove first node
uchannel_t* uchannel_remove_front(uchannel_t* head);

// remove last node
uchannel_t* uchannel_remove_last(uchannel_t* head);

// unsubsribe user from a channel
uchannel_t* uchannel_remove(uchannel_t* head, int channel_id);