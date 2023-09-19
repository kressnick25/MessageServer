#pragma once

#include <semaphore.h>
#include <stdbool.h>

#define MAX_NAME_LEN (4 + 5 + 1)
#define NAME_PREFIX "/IRC"

#define SHM_LEN 2048
#define MAX_MSG_LEN (SHM_LEN - 1)

// Struct containing data for connection between
// the parent and child process (server side).
typedef struct control_block control_block_t;
struct control_block {
    // ID of child for this block
    int child_id;
    // NULL terminated name for semaphore and shm 
    // for this instance
    char *name;
    // Pointer to semaphore controlling access to shm
    sem_t *shm_access;
    // Pointer to memory mapped communication area
    void *shm_memory;
};

// Initialise a struct control_block for a given child_id.
// This includes allocating the required memory for the struct
// and all the members of the struct. Returns a pointer
// to the struct on success or NULL pointer on failure.
// The child_id must be unused or unique.
control_block_t *control_block_init(int child_id);

// Delete a struct control_block for a given child_id.
// This includes freeing all the allocated memory. Close_only
// will only close rather than unlink the semaphore and 
// mapped memory. Returns false on error.
bool control_block_destroy(control_block_t **cb, bool close_only);