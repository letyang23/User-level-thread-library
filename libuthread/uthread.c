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
	STATE_BLOCKED, //2
	STATE_ZOMBIE //3
};


struct uthread_tcb {
	/* TODO Phase 2 */
	int tid;
	uthread_ctx_t ctx;
	void *stack;
	int state; // 0: ready, 1: running, 2: blocked, 3: zombie
	uthread_func_t func;
    void *arg;
};

static struct uthread_tcb *running_thread;
static queue_t ready_queue;
static int next_tid = 1;

struct uthread_tcb *uthread_current(void)
{
	/* TODO Phase 2/3 */
	return running_thread;
}

void uthread_yield(void)
{
	/* TODO Phase 2 */
	struct uthread_tcb *prev = running_thread;
	struct uthread_tcb *next;

	if (queue_dequeue(ready_queue, (void **)&next) == -1) {
		return;
	}

	prev -> state = STATE_READY;
	queue_enqueue(ready_queue, running_thread);
	
	next->state = STATE_RUNNING;
	running_thread = next;

	uthread_ctx_switch(&prev->ctx, &next->ctx);
}

void uthread_exit(void)
{
	/* TODO Phase 2 */
	struct uthread_tcb *prev = running_thread;
    struct uthread_tcb *next;

	if (queue_dequeue(ready_queue, (void **)&next) == -1) {
        return;
    }

	prev->stack = STATE_ZOMBIE;
	next->stack = STATE_RUNNING;
	running_thread = next;
	uthread_ctx_switch(&prev->ctx, &next->ctx);
}

int uthread_create(uthread_func_t func, void *arg)
{
	/* TODO Phase 2 */
	struct uthread_tcb *new_thread = malloc(sizeof(struct uthread_tcb));

	if (!new_thread) {
		return -1;
	}

	new_thread -> tid = next_tid++;
	new_thread -> state = STATE_READY;
	new_thread -> func = func;
	new_thread -> arg = arg;
	new_thread -> stack = uthread_ctx_alloc_stack();
	
	if (uthread_ctx_init(&new_thread->ctx, new_thread->stack, func, arg) == -1) {
		uthread_ctx_destroy_stack(new_thread->stack);
        free(new_thread);
        return -1;
	}

	if (!ready_queue) {
		ready_queue = queue_create();
	}

	queue_enqueue(ready_queue, new_thread);
	return new_thread->tid;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	/* TODO Phase 2 */
	struct uthread_tcb *main_thread = malloc(sizeof(struct uthread_tcb));

	if (!main_thread) {
        return -1;
    }

	main_thread->tid = 0;
	main_thread->state = STATE_RUNNING;
    main_thread->func = func;
    main_thread->arg = arg;

    running_thread = main_thread;

    preempt_start(preempt);
    func(arg);
    preempt_stop();

    free(main_thread);
    return 0;
}

void uthread_block(void)
{
	/* TODO Phase 3 */
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* TODO Phase 3 */
}

