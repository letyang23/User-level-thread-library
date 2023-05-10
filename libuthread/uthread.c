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

struct uthread_tcb *uthread_current(void) {
    /* TODO Phase 2/3 */
    return current_thread;
}

void uthread_yield(void) {
    /* TODO Phase 2 */
    if (queue_length(ready_queue) > 0) {
        // Set the current thread's state to ready state
        current_thread->state = THREAD_READY;

        // Enqueue the current thread back to the ready queue
        if (queue_enqueue(ready_queue, current_thread) == -1) {
            // Handle enqueue failure (might need to do something more than just return)
            return;
        }

        // Dequeue the next thread from the ready queue
        void *next_thread_ptr = NULL;
        if (queue_dequeue(ready_queue, &next_thread_ptr) == -1) {
            // Handle dequeue failure (might need to do something more than just return)
            return;
        }
        struct uthread_tcb *next_thread = (struct uthread_tcb *) next_thread_ptr;

/**
 * might not need this while loop?
 */
        while (next_thread->state == THREAD_BLOCKED) {
            queue_enqueue(ready_queue, next_thread);
            queue_dequeue(ready_queue, &next_thread_ptr);
            next_thread = (struct uthread_tcb *)next_thread_ptr;
        }

        // Set the state of the next thread to running state
        next_thread->state = THREAD_RUNNING;

        // Switch context from the current thread to the next thread
        uthread_ctx_t *prev_context = &current_thread->context;
        current_thread = next_thread;
        uthread_ctx_switch(prev_context, &current_thread->context);
    }
}

void uthread_exit(void) {
    /* TODO Phase 2 */
    current_thread->state = THREAD_EXITED;      // Set the current thread's state to exited
    uthread_yield();                            // Yield the CPU to another thread
}

int uthread_create(uthread_func_t func, void *arg) {
    /* TODO Phase 2 */
    // Allocate space for the new thread's TCB
    struct uthread_tcb *new_thread = malloc(sizeof(struct uthread_tcb));
    if (!new_thread) {
        return -1;
    }

    // Allocate space for the new thread's stack
    new_thread->stack = uthread_ctx_alloc_stack();
    if (!new_thread->stack) {
        free(new_thread);
        return -1;
    }

    // Initialize the new thread's context to run the specified function with argument
    if (uthread_ctx_init(&new_thread->context, new_thread->stack, func, arg) == -1) {
        uthread_ctx_destroy_stack(new_thread->stack);
        free(new_thread);
        return -1;
    }

    // Set the state of the new thread to ready state
    new_thread->state = THREAD_READY;

    // Enqueue the new thread to the ready queue
    if (queue_enqueue(ready_queue, new_thread) == -1) {
        uthread_ctx_destroy_stack(new_thread->stack);
        free(new_thread);
        return -1;
    }

    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
    /* TODO Phase 2 */
    // Create an empty queue for the ready threads
    ready_queue = queue_create();
    if (!ready_queue) {
        return -1;
    }

    // Register the current execution as the idle thread
    current_thread = &idle_thread;

    // Create the initial thread
    if (uthread_create(func, arg) == -1) {
        return -1;
    }

    // Scheduler loop
    while (queue_length(ready_queue) > 0) {

        // Dequeue the next thread from the ready queue
        void *next_thread_ptr = NULL;
        if (queue_dequeue(ready_queue, &next_thread_ptr) == -1) {
            return -1;
        }
        struct uthread_tcb *next_thread = (struct uthread_tcb *) next_thread_ptr;

        // Set the state of the next thread to running state
        next_thread->state = THREAD_RUNNING;

        // Switch context from the current thread to the next thread
        uthread_ctx_t *prev_context = &current_thread->context;
        current_thread = next_thread;
        uthread_ctx_switch(prev_context, &current_thread->context);
    }

    return 0;
}

void uthread_block(void) {
    /* TODO Phase 3 */
    current_thread->state = THREAD_BLOCKED;
    uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread) {
    /* TODO Phase 3 */
    if (uthread->state == THREAD_BLOCKED) {
        uthread->state = THREAD_READY;
        queue_enqueue(ready_queue, uthread);
    }
}
