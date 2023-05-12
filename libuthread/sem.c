#include <stddef.h>
#include <stdlib.h>

#include "private.h"
#include "queue.h"
#include "sem.h"

struct semaphore {
    size_t count;   // Number of resources available
    queue_t queue;  // Queue of threads waiting for this semaphore
};

sem_t sem_create(size_t count)
{
    // Allocate memory for the semaphore
    sem_t sem = malloc(sizeof(struct semaphore));
    if (!sem) {
        return NULL;    // If allocation fails, return NULL
    }

    // Initialize the semaphore's count and queue
    sem->count = count;
    sem->queue = queue_create();
    if (!sem->queue) {
        free(sem);
        return NULL;    // If queue creation fails, free the semaphore and return NULL
    }

    // If all initializations are successful, return the semaphore
    return sem;
}

int sem_destroy(sem_t sem)
{
    // If the semaphore is NULL or there are still threads waiting on it, return -1
    if (!sem || queue_length(sem->queue) > 0) {
        return -1;
    }

    // Otherwise, destroy the queue and free the semaphore
    queue_destroy(sem->queue);
    free(sem);

    return 0;
}

int sem_down(sem_t sem)
{
    // If the semaphore is NULL, return -1
    if (!sem) {
        return -1;
    }

    // If the count is 0 (no resources available), block the current thread and add it to the semaphore's queue
    while (sem->count == 0) {
        queue_enqueue(sem->queue, uthread_current());
        uthread_block();

        // Recheck the semaphore count after unblocking for the corner case.
        if (sem->count == 0) {
            continue;
        }
    }

    // Decrease the semaphore's count and return
    sem->count--;
    return 0;
}

int sem_up(sem_t sem)
{
    // If the semaphore is NULL, return -1
    if (!sem) {
        return -1;
    }

    // Increase the semaphore's count
    sem->count++;

    // If there are threads waiting on the semaphore, dequeue one and unblock it
    if (queue_length(sem->queue) > 0) {
        struct uthread_tcb *unblocked_thread;
        queue_dequeue(sem->queue, (void **)&unblocked_thread);
        uthread_unblock(unblocked_thread);
    }

    return 0;
}

