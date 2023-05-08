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

typedef enum {
    THREAD_RUNNING,
    THREAD_READY,
    THREAD_EXITED
} thread_state_t;

typedef struct uthread_tcb {
    ucontext_t context;
    thread_state_t state;
    void *stack;
    struct uthread_tcb *next;
} uthread_tcb_t;

uthread_tcb_t idle_thread;
static uthread_tcb_t *ready_queue_head = NULL;
static uthread_tcb_t *ready_queue_tail = NULL;
static uthread_tcb_t *current_thread = NULL;

int uthread_create(uthread_func_t func, void *arg) {
    uthread_tcb_t *new_thread = (uthread_tcb_t *) malloc(sizeof(uthread_tcb_t));

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

    // Add the new thread to the ready queue
    if (!ready_queue_tail) {
        ready_queue_head = new_thread;
        ready_queue_tail = new_thread;
    } else {
        ready_queue_tail->next = new_thread;
        ready_queue_tail = new_thread;
    }
    new_thread->next = NULL;

    return 0;
}


void uthread_yield(void) {
    if (ready_queue_head) {
        uthread_tcb_t *next_thread = ready_queue_head;
        ready_queue_head = ready_queue_head->next;
        if (!ready_queue_head) {
            ready_queue_tail = NULL;
        }

        next_thread->state = THREAD_RUNNING;
        current_thread->state = THREAD_READY;

        if (!ready_queue_head) {
            ready_queue_head = current_thread;
            ready_queue_tail = current_thread;
        } else {
            ready_queue_tail->next = current_thread;
            ready_queue_tail = current_thread;
        }
        current_thread->next = NULL;

        uthread_ctx_t *prev_context = &current_thread->context;
        current_thread = next_thread;

        uthread_ctx_switch(prev_context, &current_thread->context);
    }
}


void uthread_exit(void) {
    current_thread->state = THREAD_EXITED;
    uthread_yield();
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
    if (uthread_create(func, arg) == -1) {
        return -1;
    }

    idle_thread.context.uc_link = NULL;
    idle_thread.stack = NULL;
    idle_thread.state = THREAD_RUNNING;
    current_thread = &idle_thread;

    while (ready_queue_head) {
        uthread_yield();
    }

    return 0;
}





