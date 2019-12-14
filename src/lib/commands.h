#pragma once
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#include "hashtable.h"
#include "linkedlist.h"
#include "libdev.h"

#define SIZEOF_COMMANDS 7

// Includes NULL terminator and new line
#define MAX_LEN_DATA (4 + 1 + 3 + 1 + 1024 + 1)
#define MAX_USER_MSG_LENGTH (1024)
#define NUM_COMMANDS (10)

// Define error messages
#define ERR_SPC "NaN. Check if you have trailing whitespaces."
#define ERR_ARG_OMIT "Insufficent number of arguments."
#define ERR_NAN "NaN. Enter a number."
#define ERR_ARG_VAL "Needs to be from 0 to 255 (inclusive)."
#define ERR_TRAIL "Trailing characters encountered."
#define ERR_USER_MSG_LEN "Character count of message exceeds 1024."

/* Lists commands currently available to the user */
void display_help();

// Returns false if channel id not between 0 and 255 (inclusive)
bool valid_channel_id(int channel_id);

// Returns false if invocation is incorrect. Also prints error messages
// to stdout.
bool verbose_input_validation(char *input);

/* Disconnects client from server for provided socket */
void server_disconnect(int sockfd);

/* convert int to char decimal. e.g. int = 8, char = '8' */
char int_tochar(int i);

/* Verifify if channel in valid range, return -1 if fail. return 0 if pass */
int valid_channel_range(char channel_id);

/* Interpret command from char* and execute relevant command, sends response to client sender */
char* command_decoder(htab_t* users, htab_t* channels, char* c_keys, \
                        char* input, char* client_id);

// SUBSCRIBE <channelId> 
char* subscribed_to_channel(hnode_t* user, htab_t* channels, char* channel_id);

// <CHANNELS>
char* list_channels(uchannel_t* user_channels);

// UNSUB <channelId>
char* unsub_from_channel(hnode_t* user, char channel_id);

// NEXT <channelId>
char* fetch_next_channel_message(uchannel_t* user_channels, char* channel_id);

// NEXT
char* fetch_next_message(uchannel_t* user_channels);

// LIVEFEED <channelId>
char* livefeed_channel(uchannel_t* user_channels, char *channel_id, int user_id);

// LIVEFEED
char* livefeed (uchannel_t* user_channels, int user_id);

// SEND <channelId> <message>
char* send_to_channel(htab_t* channels, char* channel_id, char* message);

// BYE
void unsub_from_all(htab_t* users, char* user_id);