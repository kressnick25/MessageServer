#define _POSIX_C_SOURCE 200809L /* See feature_test_macros(7) */
#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <signal.h>
#include <stdbool.h>
#include <stdio.h> // perror, fgets
#include <stdlib.h>
#include <unistd.h> // close, sleep

#include <poll.h> // for poll()

#include <time.h> // for usleep()

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // Structures for storing address information
#include <arpa/inet.h>
#include <semaphore.h>

#include <pthread.h>

#include "../src/lib/doublylinkedlist.h"
#include "../src/lib/hashtable.h"
#include "../src/lib/commands.h"

#define SEM_PREFIX IRC
#define SEM_START 0

#define DEFAULT_PORT 12345

#define SLEEP_TIME 50000

bool cleanup = false;
bool livefeed_on = false;
bool spawn_livefeed_thread = false;
int livefeed_channel_id = -1;
pthread_mutex_t lock;
node_t* send_queue;

struct arg_struct {
    int* sockfd_net;
    char* send_buffer;
    char* recv_buffer;
};

struct arg_struct2 {
    int *sockfd_net;
    int channel_id;
    char* recv_buffer;
};

/* Implement signal handler for graceful exiting on SIGINT */
void handle_sigint(int num) {
    write(STDOUT_FILENO, "CTRL-C received. Exiting.\n", 27);
    cleanup = true;
    _exit(0); // Different from exit() as this exit does not cleanup
}

// Process input. Non blocking. Assumes valid input.
void process_input(char *input) {
    //if (!verbose_input_validation(input)) {
    //    // Not valid so we return
    //    return;
    //}
    // Was valid so now we process it.

    // Check if we need to STOP the livefeed
    if (strncmp(input, "STOP\n", 6 - 1) == 0) {
        livefeed_on = false;
        // Clear the livefeed channel id
        livefeed_channel_id = -1;

    // Check if it is LIVEFEED
    } else if (strncmp(input, "LIVEFEED", 9 - 1) == 0) {
        // Set the livefeed it
        if (livefeed_on) {
            // Livefeed is already running so we ignore.
            return;
        }
        sscanf(input, "LIVEFEED %d\n", &livefeed_channel_id);
        livefeed_on = true;
        // Set flag to spawn a livefeed thread
        spawn_livefeed_thread = true;
    }
}

bool needs_sending(char *input) {
    // Check if it is STOP
    return ((strncmp(input, "STOP\n", 6 - 1) != 0)
            && (strncmp(input, "LIVEFEED", 9 - 1) != 0));
}

// handle sigint in livefeed mode
void handle_livefeed_sigint(int num) {
    livefeed_on = false;
}

void* livefeed_mode(void* arguments) {
    struct arg_struct2 *args = arguments;

    // assign livefeed handler to sigint
    signal(SIGINT, handle_livefeed_sigint);

    char command[15];

    // create next command
    if (args->channel_id == -1) {
        strcpy(command, "NEXT");
        printf("\n**LIVEFEED MODE: all channels**\n");
    } else {
        sprintf(command, "NEXT %d", args->channel_id);
        printf("\n**LIVEFEED MODE: channel %d**\n", args->channel_id);
    }
   
    // poll for new messages
    while (livefeed_on) {
        usleep(SLEEP_TIME);
        
        // send next to server
        if (send(*args->sockfd_net, command, sizeof(command), 0) == -1) {
            perror("send");
            exit(1);
        }

        pthread_mutex_lock(&lock);
        // Receive server response and display
        memset(args->recv_buffer, 0, MAX_LEN_DATA);
        if (recv(*args->sockfd_net, args->recv_buffer, MAX_LEN_DATA, 0) == -1) {
            perror("recv");
            exit(1);
        }
        pthread_mutex_unlock(&lock);

        // Print out the response if not empty
        if (strncmp(args->recv_buffer, "**", 2) != 0) {
            if (strncmp(args->recv_buffer, "Not subscribed", 14) == 0) {
                livefeed_on = false;
            }
            printf("%s", args->recv_buffer);
        } 
        fflush(stdout); // In case there is no new line
    }
    printf("\n**Returned to NORMAL MODE**\n");

    // return SIGINT handling to normal
    signal(SIGINT, handle_sigint);

    // close thread
    pthread_exit(NULL);
    return NULL;
}

bool isValidIpAddress(char* string) {
    struct sockaddr_in sockaddr;
    int result = inet_pton(AF_INET, string, &(sockaddr.sin_addr));
    return result != 0;
}

void* fetch (void* arguments) {
    while (!cleanup) {
        struct arg_struct *args = arguments;
        //print_doubly_linked_list(send_queue);

        // do nothing if queue is empty
        if (send_queue == NULL) {
            continue;
        }

        // get next data from queue
        char* send_data = (char*) send_queue->data;

        // Send data to server
        if (send(*args->sockfd_net, send_data, MAX_LEN_DATA, 0) == -1) {
            perror("send");
            exit(1);
        }
        
        pthread_mutex_lock(&lock);
        // pop data from queue after done using it
        delete_node(&send_queue, true);
        pthread_mutex_unlock(&lock);

        // writer
        // Receive server response and display
        memset(args->recv_buffer, 0, MAX_LEN_DATA);
        if (recv(*args->sockfd_net, args->recv_buffer, MAX_LEN_DATA, 0) == -1) {
            perror("recv");
            exit(1);
        }

        // reader
        // Print out the response
        printf("%s", args->recv_buffer);
        fflush(stdout); // In case there is no new line
        
    }
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[])
{
    // Setup signal handling
    signal(SIGINT, handle_sigint);

    // TODO actually assing IP
    uint16_t port;
    if (argc != 3 || isValidIpAddress(argv[1]) == false) {
        printf("usage: client [ip_address] [port_number]\n");
        exit(1);
    }
    port = atoi(argv[2]);
    
    // Create a socket
    int* sockfd_net = malloc(sizeof(int));
    // Choose protocol automatically with 0
    *sockfd_net = socket(AF_INET, SOCK_STREAM, 0);

    // Specify the address for the socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // Data of the address you are trying to connect to
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // 0.0.0.0

    int conn_status = connect(*sockfd_net, (struct sockaddr *)&server_addr,
            sizeof(server_addr));
    // Check if we have connected successfully
    if (conn_status == -1) {
        perror("connect");
        exit(1);
    }

    // Receive data from the server
    //char recv_buffer[MAX_LEN_DATA];
    char* send_buffer = malloc(MAX_LEN_DATA);
    char* recv_buffer = malloc(MAX_LEN_DATA);
    
    // Receive hello from server
    // Receive server response and display
    memset(recv_buffer, 0, MAX_LEN_DATA);
    if (recv(*sockfd_net, recv_buffer, MAX_LEN_DATA, 0) == -1) {
        perror("recv");
        exit(1);
    }
    // Print out the response
    printf("%s", recv_buffer);
    fflush(stdout); // In case there is no new line

    // thread pointer, TODO use array of threads
    pthread_t new_thread = 0;
    // init mutex lock for pthreads
    if (pthread_mutex_init(&lock, NULL) != 0) { 
        perror("pthread_mutex_init");
        exit(1); 
    } 

    struct arg_struct args;
    args.sockfd_net = sockfd_net;
    args.recv_buffer = recv_buffer;
    args.send_buffer = send_buffer;
    if (pthread_create(&new_thread, NULL, fetch, &args) != 0) {
        perror("Error creating send queuethread");
        exit(1);
    }
    pthread_detach(new_thread);
    
    // Setup polling for ready for stdin
    int pollval;
    // Setup poll
    struct pollfd pollfd_list[1];
    nfds_t pollfd_list_len = sizeof(pollfd_list) / sizeof(struct pollfd);
    // Initialise the poll fds
    memset(pollfd_list, 0, sizeof(pollfd_list));
    pollfd_list[0].fd = 0; // STDIN fd is 0
    pollfd_list[0].events = POLLIN;

    // Begin event loop
    while (!cleanup) {
        usleep(SLEEP_TIME);
        // Check if we need to spawn a thread for livefeed
        // enter livefeed mode
        if (spawn_livefeed_thread) {
            struct arg_struct2 args;
            args.sockfd_net = sockfd_net;
            args.recv_buffer = recv_buffer;
            args.channel_id = livefeed_channel_id;
            if (pthread_create(&new_thread, NULL, livefeed_mode, &args) != 0) {
                printf("Error creating send livefeed thread\n");
                return 1;
            }
            pthread_detach(new_thread);            

            // Remove the flag to spawn a new livefeed thread
            spawn_livefeed_thread = false;
        }

        // Poll for stdin input
        if ((pollval = poll(pollfd_list, pollfd_list_len, 0)) == 0) {
            // If no interesting events occur when polling
            continue;
        }
        // Check for errors
        if (pollval == -1) {
            // Poll encountered an error
            perror("poll");
            exit(1);
        }
        // Pollval > 0 something interesting happened
        // Read stdin
        // Allocate data for it
        char *buffer;
        if ((buffer = calloc(MAX_LEN_DATA, sizeof(char)))
                == NULL) {
            // Calloc error
            perror("calloc");
            exit(1);
        }
        // Now read stdin
        if (fgets(buffer, MAX_LEN_DATA, stdin) == NULL) {
            // Read error probably because they pressed CTRL-D
            // Ignore
            // Free buffer
            free(buffer);
            continue;
        } 
        
        // We got something
        // Check to see if we have valid input
        if (!verbose_input_validation(buffer)) {
            // Not valid input
            // Free buffer
            free(buffer);
            continue;
        }

        // We're done got valid input
        // Process the input. 
        process_input(buffer);

        // Check if it needs sending to the server
        if (!needs_sending(buffer)) {
            free(buffer);
            continue;
        }

        // Needs sending so add to send queue

        // writer
        // add new command to q
        node_t *new_send_item;
        if ((new_send_item = create_node()) == NULL) {
            perror("malloc");
        }
        new_send_item->data = buffer;
        pthread_mutex_lock(&lock);
        insert_node(&send_queue, new_send_item);
        pthread_mutex_unlock(&lock);

        // Exit client after bye sent
        if (strncmp((char*)new_send_item->data, "BYE", 3) == 0) {
            printf("Exiting client ...\n");
            cleanup = true;
        }
    }

    pthread_mutex_destroy(&lock);
    free(recv_buffer);
    free(send_buffer);
    free(sockfd_net);

    // Closing the socket
    close(*sockfd_net);

    exit(0);
}