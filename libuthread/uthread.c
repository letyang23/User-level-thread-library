#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "queue.h"
#include "uthread.h"

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
    return current_thread;  // Get the current thread
}

void uthread_yield(void) {
	preempt_disable();  // Disable preemption

    // If current thread is running, enqueue it back to the ready queue
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
	preempt_enable();   // Enable preemption
}

void uthread_exit(void) {
	preempt_disable();                          // Disable preemption
    current_thread->state = THREAD_EXITED;      // Set the current thread's state to exited
    uthread_yield();                            // Yield the CPU to another thread
	preempt_enable();                           // Enable preemption
}

int uthread_create(uthread_func_t func, void *arg) {
	 preempt_disable();     // Disable preemption

    // Allocate a new TCB and initialize it
    struct uthread_tcb *new_thread = malloc(sizeof(struct uthread_tcb));
    if (!new_thread) {
        return -1;
    }

    // Allocate stack for the new thread
    new_thread->stack = uthread_ctx_alloc_stack();
    if (!new_thread->stack) {
        free(new_thread);
        return -1;
    }

    // Initialize the new thread
    if (uthread_ctx_init(&new_thread->context, new_thread->stack, func, arg) == -1) {
        uthread_ctx_destroy_stack(new_thread->stack);
        free(new_thread);
        return -1;
    }

    // Enqueue the new thread to the ready queue
    new_thread->state = THREAD_READY;
    if (queue_enqueue(ready_queue, new_thread) == -1) {
        uthread_ctx_destroy_stack(new_thread->stack);
        free(new_thread);
        return -1;
    }

	preempt_enable();   // Enable preemption
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
	if(preempt) {
		preempt_start(preempt);     // Start preemption if enabled
	}

    // Create the ready and blocked queues
    ready_queue = queue_create();
    blocked_queue = queue_create();  // Initialize the blocked queue
    if (!ready_queue || !blocked_queue) {
        return -1;
    }

    // Set the idle thread as the current thread
    current_thread = &idle_thread;

    // Create the initial thread
    if (uthread_create(func, arg) == -1) {
        return -1;
    }

    // Run until all threads have finished
    while (queue_length(ready_queue) > 0 || queue_length(blocked_queue) > 0) {
        if (queue_length(ready_queue) > 0) {
            uthread_yield();    // Yield control to the next thread
        }
    }

	preempt_stop();     // Stop preemption
    return 0;
}

void uthread_block(void) {
	preempt_disable();                                          // Disable preemption
    current_thread->state = THREAD_BLOCKED;                     // Mark the current thread as blocked
    queue_enqueue(blocked_queue, current_thread);   // Move the current thread to the blocked queue
    uthread_yield();                                            // Yield control to the next thread
	preempt_enable();                                           // Enable preemption
}

void uthread_unblock(struct uthread_tcb *uthread) {
	preempt_disable();                                          // Disable preemption
    uthread->state = THREAD_READY;                              // Mark the thread as ready
    queue_delete(blocked_queue, uthread);           // Remove the thread from the blocked queue
    queue_enqueue(ready_queue, uthread);            // Move the thread to the ready queue
	preempt_enable();                                          // Enable preemption
}