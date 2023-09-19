#define _POSIX_C_SOURCE 200809L /* See feature_test_macros(7) */
#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // _exit(), write() , STDOUT_FILENO

#include <time.h>

#include <poll.h> // for poll()

#include <fcntl.h> // For O_* constants
#include <sys/stat.h> // For mode constants
#include <semaphore.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "../src/lib/hashtable.h"
#include "../src/lib/doublylinkedlist.h"
#include "../src/lib/libdev.h"
#include "../src/lib/commands.h"

#define SEM_START 0

#define CONN_BACKLOG 10
#define DEFAULT_PORT 12345

#define MAX_CONN 15 // TODO remove after we implement it as a doubly linked list

#define HELLO_BASE "Welcome! Your client ID is "

#define SLEEP_TIME 1000 // microseconds

bool cleanup = false;

/* Implement signal handler for graceful exiting on SIGINT */
void handle_sigint(int num) {
    write(STDOUT_FILENO, "CTRL-C received. Exiting.\n", 27);
    cleanup = true;
}

void parent_process_control_block (control_block_t **cb, htab_t* users, \
        htab_t* channels, char* keys, bool *cleanup_child) {
    control_block_t *t = *cb;
    // Check if there is something in memory
    if (((char *)t->shm_memory)[0] != 1
            || sem_trywait(t->shm_access) != 0) {
        // Second statement will never be evaluated provided
        // the memory is not NULL. BUT it MUST be in this order
        // Nothing of interest
        return;
    }

    // Something interesting happened
    // WE PROCESS IT HERE
    // For this example, we will add Reply: to the front
    // of the received message.
    // Create the buffers for reading and sending
    char* client_id = &keys[t->child_id];
    char recv_buffer[MAX_MSG_LEN];
    char send_buffer[MAX_MSG_LEN];
    // Receive the message
    snprintf(recv_buffer, sizeof(recv_buffer), "%s",
            (char *)t->shm_memory + 1);
    // Create output message
    snprintf(send_buffer, sizeof(send_buffer), "%s", recv_buffer);

    char* output = command_decoder(users, channels, keys, \
                                    recv_buffer, client_id);
    
    // Write the buffer to shared memory
    // It is not [x] because that will give us the VALUE 
    // rather than the address.
    snprintf((char *)t->shm_memory + 1, MAX_MSG_LEN, "%s", output);

    // Mark request as done
    // Here we want the VALUE rather than address
    ((char *)t->shm_memory)[0] = 0;

    // Check if we need to cleanup child because of server BYE
    if (output != NULL
            && strncmp(output, "CLEANUP", 8 - 1) == 0) {
        *cleanup_child = true;
    }

    // Free the allocated memory
    free(output);

    // Release semaphore
    sem_post(t->shm_access);

    return;
}

int main (int argc, char *argv[]) {
    // Clean up semaphore if unclean exit
    // char sem[] = "/dev/shm/sem.IRC*"; // TODO remove all ie. sem.IRC1
    // remove(sem);
    system("rm -f /dev/shm/sem.IRC*");

    // SHM UNLINK / control_block_destroy
    
    // Server id for forked child process
    int client_id = 0;
    int child_id = 0;

    // Setup signal handling
    signal(SIGINT, handle_sigint);

    // get port argument
    uint16_t port;
    if (argc >= 2)
        port = atoi(argv[1]);
    else
        port = DEFAULT_PORT;
    
    // init channels and users tables
    size_t buckets = 10;
    htab_t users;
    htab_t channels;
    if (!htab_init(&users, buckets)) {
        printf("Error: failed to initialise users table\n");
        exit(1);
    }
    if (!htab_init(&channels, buckets)) {
        printf("Error: failed to initialise channels table\n");
        exit(1);
    }

    // init keys to id users and channels
   char* keys = malloc(sizeof(char*) * 256);
    for (int i = 0; i < 256; i++ ){
        keys[i] = i;
    }

    // Create the socket the server listens on
    int sockfd_server;
    sockfd_server = socket(AF_INET, SOCK_STREAM, 0);

    // Initialise the server address for INET
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    printf("Port: %d\n", port);
    // find avaiable port if specified not in use
    while (bind(sockfd_server, (struct sockaddr *)&server_addr,
            sizeof(server_addr)) == -1) 
    {   
        printf("Port already in use, reassigning...");
        port++;
        server_addr.sin_port = htons(port);
        printf("port %d\n", port);
    }

    // Listen for connections
    if (listen(sockfd_server, CONN_BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Setup polling for ready for listening socket
    int pollval;
    // Setup poll
    struct pollfd sockfd_list[1];
    nfds_t sockfd_list_len = sizeof(sockfd_list) / sizeof(struct pollfd);
    // Initialise the poll fds
    memset(sockfd_list, 0, sizeof(sockfd_list));
    sockfd_list[0].fd = sockfd_server;
    sockfd_list[0].events = POLLIN;

    // Event loop
    // Setup forking variables
    int sockfd_client;
    int pid;
    // Setup the control block store for connections (REMOVE AFTER DOUBLY LINKED LIST IMPLEMENTATION)
    control_block_t *cb_child;
    control_block_t *cb_store[MAX_CONN];
    memset(cb_store, 0, MAX_CONN * sizeof(control_block_t *)); // Zero memory
    while (true) {
        if (cleanup) {
            for (int i = 0; i < MAX_CONN; i++) {
                if (cb_store[i] != NULL) {
                    free(cb_store[i]);
                }
            }
            break;
        }
        // Sleep here to prevent 100% CPU use at all times
        usleep(SLEEP_TIME);

        // Handle any interesting communications from children
        // to the parent
        // Check all the active connections
        for (int i = 0; i < MAX_CONN; i++) {
            if (cb_store[i] == NULL) {
                // Nothing to do
                continue;
            }
            
            // Process child
            bool cleanup_child = false;
            parent_process_control_block(&cb_store[i], &users, &channels, keys,
                    &cleanup_child);
            
            // If we need to cleanup the child
            if (cleanup_child) {
                // Close only because child will unlink
                if (!control_block_destroy(&cb_store[i], true)) {
                    perror("control_block_destroy");
                    exit(1);
                }
            }
        }

        // Poll for a connection. If any accept a connection.
        if ((pollval = poll(sockfd_list, sockfd_list_len, 0)) == 0) {
            // Timeout and nothing happened
            // This also acts as the end of the loop
            // If no interesting events occur when listening
            continue;
        }
        // Check for errors
        if (pollval == -1) {
            // Poll encountered an error
            perror("poll");
            exit(1);
        }
        // Pollval > 0 something interesting happened
        // Ready to accept connection(s)
        sockfd_client = accept(sockfd_server, NULL, NULL); // blocking
        if (sockfd_client == -1) {
            perror("accept");
            exit(1);
        }

        // Setup control block
        control_block_t *cb;
        if ((cb = control_block_init(client_id)) == NULL) {
            // Couldn't create control block
            perror("control_block_init");
            exit(1);
        }

        // store new client in users table
        htab_add(&users, &keys[client_id], NULL);

        // Fork and hand connection off to child
        pid = fork();
        // Check if fork ran into errors
        if (pid == -1) {
            perror("fork");
            exit(1);
        }
        // Check if we are the child process
        if (pid == 0) {
            // Child process after fork
            // Set control block so forked child proccess can find it
            cb_child = cb;
            break;
        }

        // We are parent process after fork
        // Store the parent version of the control block
        bool slot_found = false;
        for (int i = 0; i < MAX_CONN; i++) {
            // Find the first free slot
            if (cb_store[i] == NULL) {
                // Store
                cb_store[i] = cb;

                slot_found = true;
                break;
            }
        }
        if (slot_found != true) {
            // No slot found
            printf("Maximum connections exceeded (includes stale connections).\n");
            exit(1);
        }
        // Increment child id for next child
        client_id++;
        child_id++;

        // Close the uneeded parent version of client socket
        close(sockfd_client);
    }

    if (pid == 0) {
        // Child process
        // Close the listening socket
        close(sockfd_server);

        // Setup send and receive buffers
        char recv_buffer[MAX_LEN_DATA];
        char send_buffer[MAX_LEN_DATA];

        // Zero the memory first
        memset(recv_buffer, 0, sizeof(recv_buffer));
        memset(send_buffer, 0, sizeof(send_buffer));

        // Send client a hello
        if (snprintf(send_buffer, sizeof(send_buffer), "%s %d\n", HELLO_BASE,
                client_id) == -1) {
            perror("snprintf");
            exit(1);
        }
        if (send(sockfd_client, send_buffer, sizeof(send_buffer), 0) == -1) {
            perror("send");
            exit(1);
        }

        // Child event loop
        while (true) {
            usleep(SLEEP_TIME);
            if (recv(sockfd_client, recv_buffer, sizeof(recv_buffer),
                    MSG_DONTWAIT) == -1) {
                // Check if it is a critical error
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Not an error
                    // Nothing to do. Restart loop.
                    continue;
                } else {
                    // Got an error
                    perror("recv");
                    exit(1);
                }
            }
            // We got something interesting. Send it to the parent
            // Get access to shared memory
            while (sem_trywait(cb_child->shm_access) != 0) {
                // Wait to aquire semaphore
            }

            // Write the message to shared memory
            snprintf((char *)cb_child->shm_memory + 1, MAX_MSG_LEN + 1,
                    "%s", recv_buffer);

            // Mark that a request is available
            ((char *)cb_child->shm_memory)[0] = 1;

            // Release the semaphore
            sem_post(cb_child->shm_access);

            // Wait for parent to respond
            while (((char *)cb_child->shm_memory)[0] != 0
                    || sem_trywait(cb_child->shm_access) != 0) {
                // Waiting to acquire semaphore
            }
            // Parent has finished and we have access

            // Read the reponse into the send buffer
            snprintf(send_buffer, sizeof(send_buffer),
                    "%s", (char *)cb_child->shm_memory + 1);

            // Critical section over. Release the semaphore.
            sem_post(cb_child->shm_access);

            // Check if we need to cleanup
            if (strncmp(send_buffer, "CLEANUP", 8 - 1) == 0) {
                // Cleanup child
                break;
            }

            // Now send what we received back to client
            if (send(sockfd_client, send_buffer, sizeof(send_buffer), 0)
                    == -1) {
                perror("send");
                exit(1);
            }
        }
    }

    // Cleanup
    if (pid == 0) {
        // Child cleanup
        // Unlink
        if (!control_block_destroy(&cb_child, false)) {
            perror("control_block_destroy");
            exit(1);
        }

        close(sockfd_client);
    } else {
        // Parent cleanup
        close(sockfd_server);
        htab_destroy(&channels);
        htab_destroy(&users);
        free(keys);
    }
    exit(0);
}