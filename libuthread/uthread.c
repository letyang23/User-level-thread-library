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

enum {
    STATE_READY, //0
    STATE_RUNNING, // 1
    STATE_ZOMBIE, //2
    STATE_BLOCK//3
};


struct uthread_tcb {
    /* TODO Phase 2 */
    int tid;
    uthread_ctx_t ctx;
    void *stack;
    int state; // 0: ready, 1: running, 2: zombie, 3: blocked
};

typedef struct {
    int count;
    struct uthread_tcb *main_thread;
    struct uthread_tcb *running_thread;
    queue_t ready_queue;
    queue_t block_queue;
} Controllor;

Controllor *controllor;

struct uthread_tcb *uthread_current(void)
{
    /* TODO Phase 2/3 */
    return controllor->running_thread;
}

int starter(struct uthread_tcb *thread, uthread_func_t func, void *arg) {
    thread->stack = uthread_ctx_alloc_stack();
    if (thread->stack == NULL) {
        return -1;
    }

    if (controllor->count == 0) {
        //Main thread
        thread->state = STATE_RUNNING;
        func = NULL;
        arg = NULL;
    } else {
        thread->state = STATE_READY;
    }

    if (uthread_ctx_init(&thread->ctx, thread->stack, func, arg) == -1) {
        uthread_ctx_destroy_stack(thread->stack);
        free(thread);
        return -1;
    }

    thread->tid = controllor->count;
    controllor->count++;
    return 0;
}

void uthread_yield(void)
{
    /* TODO Phase 2 */
    preempt_disable();

    struct uthread_tcb *current = controllor->running_thread;
    if (current->tid != 0) {
        //Not the main thread
        controllor->running_thread->state = STATE_READY;
        if (queue_enqueue(controllor->ready_queue, current) == -1) {
            //enqueue fail
            return;
        }
    }

    if (queue_length(controllor->ready_queue) == 0) {
        //set main thread as running thread
        controllor->running_thread = controllor->main_thread;
        controllor->running_thread->state = STATE_RUNNING;
    } else {
        queue_dequeue(controllor->ready_queue, (void**)&controllor->running_thread);
    }

    preempt_enable();
    uthread_ctx_switch(&current->ctx, &controllor->running_thread->ctx);
}

void uthread_exit(void)
{
    /* TODO Phase 2 */
    preempt_disable();

    uthread_ctx_t current_context = controllor->running_thread->ctx;
    controllor->running_thread->state = STATE_ZOMBIE;

    if (queue_length(controllor->ready_queue) == 0) {
        //set main thread as running thread
        controllor->running_thread = controllor->main_thread;
        controllor->running_thread->state = STATE_RUNNING;
    } else {
        queue_dequeue(controllor->ready_queue, (void**)&controllor->running_thread);
    }

    preempt_enable();
    uthread_ctx_switch(&current_context, &controllor->running_thread->ctx);
}

int uthread_create(uthread_func_t func, void *arg)
{
    /* TODO Phase 2 */
    struct uthread_tcb *thread = malloc(sizeof(struct uthread_tcb));
    if (!thread) {
        return -1;
    }

    thread->stack = uthread_ctx_alloc_stack();
    if (thread->stack == NULL) {
        free(thread);
        return -1;
    }


    if (controllor->count != 0) {
        thread->state = STATE_READY;
    }

    if (uthread_ctx_init(&thread->ctx, thread->stack, func, arg) == -1) {
        uthread_ctx_destroy_stack(thread->stack);
        free(thread);
        return -1;
    }

    thread->tid = controllor->count;
    controllor->count++;

    if (queue_enqueue(controllor->ready_queue, (void*)thread) == -1) {
        return -1;
    }

    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    /* TODO Phase 2 */
    if(preempt) {
        preempt_start(preempt);
    }

    controllor = (Controllor*)malloc(sizeof(Controllor));
    controllor->count = 0;
    controllor->ready_queue = queue_create();
    controllor->block_queue = queue_create();
    if (controllor->ready_queue == NULL || controllor->block_queue == NULL) {
        return -1;
    }

    struct uthread_tcb *main_thread = malloc(sizeof(struct uthread_tcb));
    if (!main_thread) {
        return -1;
    }

    main_thread->stack = uthread_ctx_alloc_stack();
    if (main_thread->stack == NULL) {
        return -1;
    }

    if (controllor->count == 0) {
        //Main thread
        main_thread->state = STATE_RUNNING;
        func = NULL;
        arg = NULL;
    }

    if (uthread_ctx_init(&main_thread->ctx, main_thread->stack, func, arg) == -1) {
        uthread_ctx_destroy_stack(main_thread->stack);
        free(main_thread);
        return -1;
    }

    main_thread->tid = controllor->count;
    controllor->count++;

    controllor->main_thread = main_thread;
    controllor->running_thread = main_thread;
    uthread_create(func, arg);
    uthread_yield();

    return 0;
}

void uthread_block(void)
{
    preempt_disable();
    //temporary store the current running thread
    struct uthread_tcb* savedThread = controllor->running_thread;
    //if thread is not main thread, put it back to block queue
    if(savedThread->tid != 0){
        controllor->running_thread->state = STATE_BLOCK;
        if(queue_enqueue(controllor->block_queue,
                         controllor->running_thread) == -1){
            return;
        }
    }

    //if queue is not empty, get thread from the ready else from mainthread
    if(queue_length(controllor->ready_queue) != 0){
        //dequeue from the ready queue
        queue_dequeue(controllor->ready_queue,
                      (void**)&controllor->running_thread);
        controllor->running_thread->state = STATE_RUNNING;
    }else{
        //set main thread to running thread
        controllor->running_thread = controllor->main_thread;
        controllor->running_thread->state = STATE_RUNNING;
    }
    preempt_enable();
    //switch thread
    uthread_ctx_switch(&savedThread->ctx,
                       &controllor->running_thread->ctx);

}

void uthread_unblock(struct uthread_tcb *uthread)
{

    //delete uthread from block queue
    queue_delete(controllor->block_queue, uthread);
    //put thread back to the ready queue
    queue_enqueue(controllor->ready_queue, uthread);

}