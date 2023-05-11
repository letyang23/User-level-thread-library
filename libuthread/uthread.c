#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

/* Enum type for thread states */
typedef enum {
    THREAD_RUNNING,     // Running State
    THREAD_READY,       // Ready State
    THREAD_EXITED,      // Exited State
    THREAD_BLOCKED      // Blocked State
} thread_state_t;

/* Thread Control BLock (TCB) Data Structure */
struct uthread_tcb {
    /* TODO Phase 2 */
    ucontext_t context;     // Thread Context
    thread_state_t state;   // Thread State
    void *stack;            // Pointer to the thread's stack
};

/* Global variables */
static struct uthread_tcb *current_thread = NULL;   // The currently running thread
static queue_t ready_queue = NULL;                  // Queue of threads ready to be scheduled
struct uthread_tcb idle_thread;                     // Idle Thread
static queue_t blocked_queue = NULL;                // Queue of threads that are blocked

struct uthread_tcb *uthread_current(void) {
    /* TODO Phase 2/3 */
    return current_thread;
}

void uthread_yield(void) {
    /* TODO Phase 2 */
	preempt_disable();

    if (current_thread->state == THREAD_RUNNING) {
        current_thread->state = THREAD_READY;  // Set the state back to ready before enqueue
        if (queue_enqueue(ready_queue, current_thread) == -1) {
            // Handle enqueue failure
            return;
        }
    }

    if (queue_length(ready_queue) > 0) {
        void *next_thread_ptr = NULL;
        if (queue_dequeue(ready_queue, &next_thread_ptr) == -1) {
            // Handle dequeue failure
            return;
        }
        struct uthread_tcb *next_thread = (struct uthread_tcb *)next_thread_ptr;
        next_thread->state = THREAD_RUNNING;
        uthread_ctx_t *prev_context = &current_thread->context;
        current_thread = next_thread;
        uthread_ctx_switch(prev_context, &current_thread->context);
    }
	preempt_enable();
}

void uthread_exit(void) {
    /* TODO Phase 2 */
	preempt_disable();
    current_thread->state = THREAD_EXITED;      // Set the current thread's state to exited
    uthread_yield();                            // Yield the CPU to another thread
	preempt_enable();
}

int uthread_create(uthread_func_t func, void *arg) {
    /* TODO Phase 2 */
	 preempt_disable();
    struct uthread_tcb *new_thread = malloc(sizeof(struct uthread_tcb));
    if (!new_thread) {
        return -1;
    }

    new_thread->stack = uthread_ctx_alloc_stack();
    if (!new_thread->stack) {
        free(new_thread);
        return -1;
    }

    if (uthread_ctx_init(&new_thread->context, new_thread->stack, func, arg) == -1) {
        uthread_ctx_destroy_stack(new_thread->stack);
        free(new_thread);
        return -1;
    }

    new_thread->state = THREAD_READY;

    if (queue_enqueue(ready_queue, new_thread) == -1) {
        uthread_ctx_destroy_stack(new_thread->stack);
        free(new_thread);
        return -1;
    }

	preempt_enable();
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
    /* TODO Phase 2 */
	if(preempt) {
		preempt_start(preempt);
	}
    ready_queue = queue_create();
    blocked_queue = queue_create();  // Initialize the blocked queue
    if (!ready_queue || !blocked_queue) {
        return -1;
    }

    current_thread = &idle_thread;

    if (uthread_create(func, arg) == -1) {
        return -1;
    }

    while (queue_length(ready_queue) > 0 || queue_length(blocked_queue) > 0) {
        if (queue_length(ready_queue) > 0) {
            uthread_yield();
        }
    }

	preempt_stop();
    return 0;
}

void uthread_block(void) {
    /* TODO Phase 3 */
	preempt_disable();
    current_thread->state = THREAD_BLOCKED;
    queue_enqueue(blocked_queue, current_thread);
    uthread_yield();
	preempt_enable(); 
}

void uthread_unblock(struct uthread_tcb *uthread) {
    /* TODO Phase 3 */
	preempt_disable();
    uthread->state = THREAD_READY;
    queue_delete(blocked_queue, uthread);
    queue_enqueue(ready_queue, uthread);
	preempt_enable();
}