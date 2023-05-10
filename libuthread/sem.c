#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore {
	/* TODO Phase 3 */
    size_t count;
    queue_t queue;
};

sem_t sem_create(size_t count)
{
	/* TODO Phase 3 */
    sem_t sem = malloc(sizeof(struct semaphore));
    if (!sem) {
        return NULL;
    }

    sem->count = count;
    sem->queue = queue_create();
    if (!sem->queue) {
        free(sem);
        return NULL;
    }

    return sem;
}

int sem_destroy(sem_t sem)
{
	/* TODO Phase 3 */
    if (!sem || queue_length(sem->queue) > 0) {
        return -1;
    }

    queue_destroy(sem->queue);
    free(sem);

    return 0;
}

int sem_down(sem_t sem)
{
	/* TODO Phase 3 */
    if (!sem) {
        return -1;
    }

    while (sem->count == 0) {
        queue_enqueue(sem->queue, uthread_current());
        uthread_block();
    }

    sem->count--;
    return 0;
}

int sem_up(sem_t sem)
{
	/* TODO Phase 3 */
    if (!sem) {
        return -1;
    }

    sem->count++;
    if (queue_length(sem->queue) > 0) {
        struct uthread_tcb *unblocked_thread;
        queue_dequeue(sem->queue, (void **)&unblocked_thread);
        uthread_unblock(unblocked_thread);
    }

    return 0;
}

