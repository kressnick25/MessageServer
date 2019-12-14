#define _GNU_SOURCE
#include <ctype.h> // isspace()
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
#include "commands.h"
#include "libdev.h"

char VALID_COMMANDS[SIZEOF_COMMANDS][8] = {"SUB", "CHANNELS", "UNSUB", "NEXT", "LIVEFEED", "SEND", "BYE"};

/* Lists commands currently available to the user */
void display_help(void) {
    printf("\nAvailable commands:\n");
    for (int i = 0; i < SIZEOF_COMMANDS; i++){
        printf("- %s\n", VALID_COMMANDS[i]);
    }
    printf("\n Please enter a comand: ");
}


bool valid_channel_id(int channel_id) {
    return (channel_id >= 0 && channel_id <= 255);
}

bool verbose_input_validation(char *input) {
    int channel_id;

    // "SUB " || SUB <channelid>
    if (strncmp(input, "SUB ", 5 - 1) == 0) {
        char c = input[5 - 1];

        if (isspace(c) && c != '\n') {
            // We have a trailing whitespace where <channelid>
            // should be
            printf("Invalid channel: %s\n", ERR_SPC);
            return false;
        }

        if (c == '\n') {
            // They have not supplied a channelid
            printf("Invalid channel: %s\n", ERR_ARG_OMIT);
            return false;
        }

        int count;
        if (sscanf(input, "SUB %d%n", &channel_id, &count) != 1) {
            // No number could be found. Therefore a letter must have
            // been entered. 
            printf("Invalid channel: %s\n", ERR_NAN);
            return false;
        } else if (!valid_channel_id(channel_id)) {
            // Not a number between 0-255
            printf("Invalid channel: channel %d. %s\n",
                    channel_id, ERR_ARG_VAL);
            return false;
        } else if (input[count] != '\n') {
            // Trailing characters
            printf("Invalid usage of SUB <channelid>. %s\n", ERR_TRAIL);
            return false;
        }

    // "UNSUB " || UNSUB <channelid>
    } else if (strncmp(input, "UNSUB ", 7 - 1) == 0) {
        char c = input[7 - 1];

        if (isspace(c) && c != '\n') {
            // We have a trailing whitespace where <channelid>
            // should be
            printf("Invalid channel: %s\n", ERR_SPC);
            return false;
        }

        if (c == '\n') {
            // They have not supplied a channelid
            printf("Invalid channel: %s\n", ERR_ARG_OMIT);
            return false;
        }

        int count;
        if (sscanf(input, "UNSUB %d%n", &channel_id, &count) != 1) {
            // No number could be found. Therefore a letter must have
            // been entered. 
            printf("Invalid channel: %s\n", ERR_NAN);
            return false;
        } else if (!valid_channel_id(channel_id)) {
            // Not a number between 0-255
            printf("Invalid channel: channel %d. %s\n",
                    channel_id, ERR_ARG_VAL);
            return false;
        } else if (input[count] != '\n') {
            // Trailing characters
            printf("Invalid usage of UNSUB <channelid>. %s\n", ERR_TRAIL);
            return false;
        }

    // "NEXT\n" || NEXT
    } else if (strncmp(input, "NEXT\n", 6 - 1) == 0) {
        // Valid so do nothing

    // "NEXT " || NEXT <channelid>
    } else if (strncmp(input, "NEXT ", 6 - 1) == 0) {
        char c = input[6 - 1];

        if (isspace(c) && c != '\n') {
            // We have a trailing whitespace where <channelid>
            // should be
            printf("Invalid channel: %s\n", ERR_SPC);
            return false;
        }

        if (c == '\n') {
            // They have not supplied a channelid
            printf("Invalid channel: %s\n", ERR_ARG_OMIT);
            return false;
        }

        int count;
        if (sscanf(input, "NEXT %d%n", &channel_id, &count) != 1) {
            // No number could be found. Therefore a letter must have
            // been entered. 
            printf("Invalid channel: %s\n", ERR_NAN);
            return false;
        } else if (!valid_channel_id(channel_id)) {
            // Not a number between 0-255
            printf("Invalid channel: channel %d. %s\n",
                    channel_id, ERR_ARG_VAL);
            return false;
        } else if (input[count] != '\n') {
            // Trailing characters
            printf("Invalid usage of NEXT <channelid>. %s\n", ERR_TRAIL);
            return false;
        }

    // "LIVEFEED\n" || LIVEFEED
    } else if (strncmp(input, "LIVEFEED\n", 10 - 1) == 0) {
        // Valid

    // "LIVEFEED " || LIVEFEED <channelid>
    } else if (strncmp(input, "LIVEFEED ", 10 - 1) == 0) {
        char c = input[10 - 1];

        if (isspace(c) && c != '\n') {
            // We have a trailing whitespace where <channelid>
            // should be
            printf("Invalid channel: %s\n", ERR_SPC);
            return false;
        }

        if (c == '\n') {
            // They have not supplied a channelid
            printf("Invalid channel: %s\n", ERR_ARG_OMIT);
            return false;
        }

        int count;
        if (sscanf(input, "LIVEFEED %d%n", &channel_id, &count) != 1) {
            // No number could be found. Therefore a letter must have
            // been entered. 
            printf("Invalid channel: %s\n", ERR_NAN);
            return false;
        } else if (!valid_channel_id(channel_id)) {
            // Not a number between 0-255
            printf("Invalid channel: channel %d. %s\n",
                    channel_id, ERR_ARG_VAL);
            return false;
        } else if (input[count] != '\n') {
            // Trailing characters
            printf("Invalid usage of LIVEFEED <channelid>. %s\n", ERR_TRAIL);
            return false;
        }

    // "CHANNELS\n" || CHANNELS
    } else if (strncmp(input, "CHANNELS\n", 10 - 1) == 0) {
        // Valid

    // "CHANNELS " || CHANNELS <channelid>
    } else if (strncmp(input, "CHANNELS ", 10 - 1) == 0) {
        char c = input[10 - 1];

        if (isspace(c) && c != '\n') {
            // We have a trailing whitespace where <channelid>
            // should be
            printf("Invalid channel: %s\n", ERR_SPC);
            return false;
        }

        if (c == '\n') {
            // They have not supplied a channelid
            printf("Invalid channel: %s\n", ERR_ARG_OMIT);
            return false;
        }

        int count;
        if (sscanf(input, "CHANNELS %d%n", &channel_id, &count) != 1) {
            // No number could be found. Therefore a letter must have
            // been entered. 
            printf("Invalid channel: %s\n", ERR_NAN);
            return false;
        } else if (!valid_channel_id(channel_id)) {
            // Not a number between 0-255
            printf("Invalid channel: channel %d. %s\n",
                    channel_id, ERR_ARG_VAL);
            return false;
        } else if (input[count] != '\n') {
            // Trailing characters
            printf("Invalid usage of CHANNELS <channelid>. %s\n", ERR_TRAIL);
            return false;
        }

    // "BYE\n" || BYE
    } else if (strncmp(input, "BYE\n", 5 - 1) == 0) {
        // Valid

    // "STOP\n" || STOP
    } else if (strncmp(input, "STOP\n", 6 - 1) == 0) {
        // Valid

    // "SEND " || SEND <channelid> <message>
    } else if (strncmp(input, "SEND ", 6 - 1) == 0) {
        char c = input[6 - 1];

        if (isspace(c) && c != '\n') {
            // We have a trailing whitespace where <channelid>
            // should be
            printf("Invalid channel: %s\n", ERR_SPC);
            return false;
        }

        if (c == '\n') {
            // They have not supplied a channelid
            printf("Invalid channel: %s\n", ERR_ARG_OMIT);
            return false;
        }

        int count;
        if (sscanf(input, "SEND %d%n", &channel_id, &count) != 1) {
            // No number could be found. Therefore a letter must have
            // been entered. 
            printf("Invalid channel: %s\n", ERR_NAN);
            return false;
        } else if (!valid_channel_id(channel_id)) {
            // Not a number between 0-255
            printf("Invalid channel: channel %d. %s\n",
                    channel_id, ERR_ARG_VAL);
            return false;
        } else if (input[count] != ' ') {
            // Trailing characters
            printf("Invalid usage of SEND <channelid> <message>. %s\n",
                    ERR_ARG_OMIT);
            return false;
        }

        // Check how long the message is
        // Warning strlen does not include the null character in the 
        // returned length
        if (strlen(input + count + 1) > MAX_USER_MSG_LENGTH) {
            // Message is too long
            printf("Invalid message: %s\n", ERR_USER_MSG_LEN);
            return false;
        }

    // Unknown command
    } else {
        printf("Invalid invocation\n");
        return false;
    }

    return true;
}

/* Disconnects client from server for provided socket */
void server_disconnect(int sockfd) {
    printf("Closing connection to server...\n");
    close(sockfd);
    printf("Connection closed\n\nExiting client...\n\n");
}

char int_tochar(int i) {
    return i + '0';
}

// SUBSCRIBE <channelId>
char* subscribed_to_channel(hnode_t* user, htab_t* channels, char* channel_id) {
    unsigned char ch = (unsigned char) *channel_id;
    char* buffer;
    uchannel_t* user_channels = (uchannel_t*) user->value;
    // user already subbed to channel
    if (uchannel_get_by_id(user_channels, *channel_id) != NULL){
        asprintf(&buffer, "Already subscribed to channel %d\n", ch);
    }
    // if channel not exist
    else {
        if (htab_find(channels, channel_id) == NULL) {
            channel_t* new = channel_init(*channel_id);
            if (htab_add(channels, channel_id, new) != 1){
                asprintf(&buffer, "FAILED TO ADD TO TABLE\n");
                return buffer;
            }
            user->value = uchannel_add_to_user(user_channels, new);
            asprintf(&buffer, "Subscribed to channel %d\n", ch);
            // hnode_t* channel = htab_find(channels, channel_id);
            // printf("test");
        } else {
            channel_t* channel = (channel_t *) htab_find(channels, channel_id)->value;
            user->value = uchannel_add_to_user(user_channels, channel);
            asprintf(&buffer, "Subscribed to channel %d\n", ch);
        }
    }
    return buffer;
}

char* write_channel(uchannel_t* user_channel) {
    char total = user_channel->channel->total_messages;
    char read = user_channel->read_msg;
    char unread = user_channel->channel->total_messages - user_channel->read_msg;
    char channel = user_channel->channel->id;
    unsigned char c = (unsigned char) channel;
    char* output;
    asprintf(&output, "CHANNEL %d:\t%d\t%d\t%d\n", c, total, read, unread);
    return output;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(unsigned char*)a - *(unsigned char*)b );
}

char* list_channels(uchannel_t* user_channels){
    char ids[256]; // init to max possible channels a user can sub to
    memset(ids, 0, 256);
    uchannel_t* n = user_channels;
    char* buffer = malloc(MAX_LEN_DATA);
    if (n == NULL) {
        asprintf(&buffer, " \n");
        return buffer;
    }
    int i = 0;
    while (n->next != NULL) {
        ids[i] = n->channel->id;
        i++;
        n = n->next;
    }
    // add last channel id
    ids[i] = n->channel->id;

    // sort array
    qsort(ids, sizeof(ids)/ sizeof(char), sizeof(char), cmpfunc);

    // add channel info to buffer in order
    for (int i = 0; i < 256; i++) {
        if (ids[i] != '\000') {
            uchannel_t* n = uchannel_get_by_id(user_channels, ids[i]);
            char* text = write_channel(n);
            if (i == 0)
                strcpy(buffer, text);
            else
                strcat(buffer, text);
            free(text);
        }
    }
    return buffer;
}

// UNSUB <channelId>
char* unsub_from_channel(hnode_t* user, char channel_id) {
    unsigned char ch = (unsigned char) channel_id;
    char* buffer;
    if (user->value == NULL) {
        asprintf(&buffer, "Not subscribed to channel %d\n", ch);
        return buffer;
    }
    uchannel_t* user_channels = (uchannel_t*) user->value;
    // user not subbed to channel
    if (uchannel_get_by_id(user_channels, channel_id) == NULL) {
        asprintf(&buffer, "Not subscribed to channel %d\n", ch);
    }
    else {
        user->value = uchannel_remove(user_channels, channel_id);
        asprintf(&buffer, "Unubscribed from channel %d\n", ch);
    }

    return buffer;
}

// NEXT <channelId>
char* fetch_next_channel_message(uchannel_t* user_channels, char* channel_id) {
    char* buffer;
    unsigned char ch = (unsigned char) *channel_id;
    // user not subbed to channel
    if (uchannel_get_by_id(user_channels, *channel_id) == NULL) {
        asprintf(&buffer, "Not subscribed to channel %d\n", ch);
    }
    else {
        uchannel_t* uchannel = uchannel_get_by_id(user_channels, *channel_id);
        message_t* message = message_get_by_id(uchannel->channel->message_list, uchannel->read_msg);
        // update read messages iteration
        if (message == NULL) {
            asprintf(&buffer, "**No new unread messages**\n");
        } else {
            asprintf(&buffer, "%s", message->text);
            uchannel->read_msg++;
        }
    }

    return buffer;
}

// NEXT
char* fetch_next_message(uchannel_t* user_channels) {
    char* buffer = malloc(MAX_LEN_DATA);
    uchannel_t* n = user_channels;
    if (n == NULL){
        sprintf(buffer, "Not subscribed to a channel\n");
        return buffer;
    }
    bool first = true;
    bool no_new_messages = true;
    do { // Loops channels
        char* out = fetch_next_channel_message(n, &n->channel->id);
        if (strncmp(out, "**No", 4) != 0)
            no_new_messages = false;
        if (first){
            unsigned char ch = (unsigned char) n->channel->id;
            sprintf(buffer, "\n%d: ", ch);
            first = false;
        }
        else{
            char s[6];
            unsigned char ch = (unsigned char) n->channel->id;
            sprintf(s, "%d: ", ch);
            strcat(buffer, s);
        }
        strcat(buffer, out);
        n = n->next;
    } while (n != NULL);
    if (no_new_messages) {
        memset(buffer, 0, MAX_LEN_DATA);
        sprintf(buffer, "**No new messages in any channel\n");
    }
    return buffer;
}

// LIVEFEED <channelId>
char* livefeed_channel(uchannel_t* user_channels, char *channel_id, int user_id) {
    char* buffer = malloc(MAX_LEN_DATA);
    buffer = fetch_next_channel_message(user_channels, channel_id);

    // add user to livefeed list
    uchannel_t* uchannel = uchannel_get_by_id(user_channels, *channel_id);
    livefeed_add(uchannel->channel->livefeed_list, user_id);

    if (buffer != NULL) {
        return buffer;
    }
    return NULL;
}

// LIVEFEED
char* livefeed (uchannel_t* user_channels, int user_id) {
    char* buffer = malloc(MAX_LEN_DATA);
    buffer = fetch_next_message(user_channels);

    // for each channel, add user to livefeed list
    uchannel_t* n = user_channels;
    while (n->next != NULL) {
        livefeed_add(n->channel->livefeed_list, user_id);
    }
    if (buffer != NULL) {
        return buffer;
    }
    return NULL;
}

// SEND <channelId> <message>
char* send_to_channel(htab_t* channels, char* channel_id, char* message) {
    unsigned char ch = (unsigned char) *channel_id;
    char* buffer;
    // if channel not exist
    if (htab_find(channels, channel_id) == NULL) {
        channel_t* new = channel_init(*channel_id);
        htab_add(channels, channel_id, new);
        new->message_list = message_add(new->message_list, message);
        new->total_messages++;
        asprintf(&buffer, "Successfully sent message to channel %d\n", ch);
    } else {
        channel_t* channel = (channel_t*) htab_find(channels, channel_id)->value;
        channel->message_list = message_add(channel->message_list, message);
        channel->total_messages++;
        asprintf(&buffer, "Successfully sent message to channel %d\n", ch);
    }
    return buffer;
}

// BYE
void unsub_from_all(htab_t* users, char* user_id) {
    hnode_t* value = htab_find(users, user_id)->value;
    uchannel_t* user_channels = (uchannel_t*) value;
    uchannels_free(user_channels);
}

/* Interpret command from char* and execute relevant command, sends response to client sender */
char* command_decoder(htab_t* users, htab_t* channels, char* c_keys,\
                        char* input, char* client_id) {
    if (*input == '\000') {
        return NULL;
    }
    // get user_channel list head
    hnode_t* user = htab_find(users, client_id);
    uchannel_t* user_head = (uchannel_t*) user->value;
    char* command = strtok(input, " \n"); // split first word by space
    char* num = strtok(NULL, " \n");
    int channel_int;
    char* channel_key;
    channel_key = NULL;
    
    if (num != NULL) {
        channel_int = atoi(num);
        channel_key = &c_keys[channel_int];
    }
    char* buffer;
    if (strcmp(command, "SUB") == 0) {// SUB <channelId>
        buffer = subscribed_to_channel(user, channels, channel_key);
    }
    else if (strcmp(command, "CHANNELS") == 0) {// CHANNELS
        buffer = list_channels(user_head);
    }
    else if (strcmp(command, "UNSUB") == 0) { // UNSUB <channelId>
        buffer = unsub_from_channel(user, *channel_key); // TODO  make this take char*
    }
    else if (strcmp(command, "NEXT") == 0 && channel_key != NULL) { // NEXT <channelId>
        buffer = fetch_next_channel_message(user_head, channel_key);
    }
    else if (strcmp(command, "NEXT") == 0 && channel_key == NULL) {// NEXT
        buffer = fetch_next_message(user_head);
    }
    else if (strcmp(command, "SEND") == 0) { // SEND <commands> <channelId>
        char* message = strtok(NULL, "");// message
        buffer = send_to_channel(channels, channel_key, message);
    }
    else if (strcmp(command, "BYE") == 0) { // BYE
        unsub_from_all(users, client_id);
        asprintf(&buffer, "CLEANUP");
    }
    else {
        asprintf(&buffer, "Invalid Command\n");
    }
    return buffer;
}