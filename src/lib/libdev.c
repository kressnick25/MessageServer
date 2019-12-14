#define _POSIX_C_SOURCE 200809L /* See feature_test_macros(7) */
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>          // For O_* constants
#include <sys/stat.h>       // For mode constants
#include <semaphore.h>      // Semaphores

#include <sys/mman.h>       // Shared memory
#include <sys/shm.h>

#include "libdev.h"

control_block_t *control_block_init(int child_id) {
    control_block_t *cb;
    if ((cb = calloc(1, sizeof(control_block_t))) == NULL) {
        // Calloc failed
        return NULL;
    }

    // Generate name
    char *name;
    if ((name = calloc(MAX_NAME_LEN, sizeof(char))) == NULL) {
        // Calloc failed
        // Free all we have allocated until now
        free(cb);
        return NULL;
    }
    int retval;
    retval = snprintf(name, MAX_NAME_LEN, "%s%d", NAME_PREFIX, child_id);
    if (!(retval != -1 && retval < MAX_NAME_LEN)) {
        // Name generation failed
        // Free all we have allocated until now
        free(cb);
        free(name);
        return NULL;
    }

    // Create unique named semaphore with initial value of 1
    // and rw permissions for owner and group.
    sem_t *shm_access;
    if ((shm_access = sem_open(name, O_CREAT | O_EXCL, 0660, 1))
            == SEM_FAILED) {
        // Creating semaphore failed
        // Free all we have allocated until now
        free(cb);
        free(name);
        free(shm_access);
        return NULL;
    }

    // Create the memory mapped area we are not using file backed memory,
    // hence the use of MAP_ANONYMOUS. This means our shared memory
    // will be faster.
    void *shm_memory;
    shm_memory = mmap(NULL, SHM_LEN,
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Adding data to the control block
    // Add shm_fd to control block now that it represents
    // the file descriptor to a valid sized 
    cb->child_id = child_id;
    cb->name = name;
    cb->shm_access = shm_access;
    cb->shm_memory = shm_memory;

    return cb;
}

bool control_block_destroy(control_block_t **cb, bool close_only) {
    control_block_t *t = *cb;
    if (!close_only) {
        // Unlinking the semaphore
        if (sem_unlink(t->name) == -1) {
            // Could not unlink the semaphore
            return false;
        }
        // Unlinking the shared memory
        if (munmap(t->shm_memory, SHM_LEN) == -1) {
            // Could not unlink the shared memory
            return false;
        }
    } else {
        // Close the semaphore
        if (sem_close(t->shm_access) == -1) {
            // Could not close the semaphore
            return false;
        }
        // Closing the shared memory
        // Nothing needs to be done - shared memory will close when
        // the proccess terminates. 
    }

    // Dealloc the memory for name
    free(t->name);

    // Dealloc memory for the control block
    free(t);

    // Set original pointer to NULL
    *cb = NULL;

    return true;
}