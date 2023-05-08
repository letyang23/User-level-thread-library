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

//define enum type for the thread state
enum{
    STATE_READY, //0
    STATE_RUNNING, // 1
    STATE_EXIT, //2
    STATE_BLOCKED //3
};


struct uthread_tcb {
    int tid; //thread id
    uthread_ctx_t context;
    int threadState; //0 is ready to run, 1 is running, 2 is exit 3 is blocked
    void* stack;
};

typedef struct uthread_tcb  uthread_tcb;

//storing all the queue in the struct
typedef struct{
    //count all the thread has been add
    int threadIdCount;
    //store current running thread
    struct uthread_tcb *runningThread;
    //store the main thread, if exit, put back into running thread
    struct uthread_tcb *mainThread;
    //queue for storing block zombie ready
    queue_t queueOfBlocked;
    queue_t queueOfZombie;
    queue_t queueOfReady;
}ScheduleController;

ScheduleController *scheduleController;



struct uthread_tcb *uthread_current(void)
{
    //direct return the running thread
    return scheduleController->runningThread;
}

//initialize the thread
//argument:
//func/arg: data will store into the thread
//thread: use to return the thread which has been initialize
//return type: return -1 if fail return 0 if success
int ThreadInitialize(uthread_func_t func, void *arg, uthread_tcb* thread){
    //initialize the stack
    thread->stack = uthread_ctx_alloc_stack();
    //if stack is null function fail, return -1
    if(thread->stack == NULL){
        return -1;
    }

    //check whether the id count whether is zero
    if(scheduleController->threadIdCount != 0){
        //set state to ready
        thread->threadState = STATE_READY;

    }else{
        //initialize main thread
        thread->threadState = STATE_RUNNING;
        arg = NULL;
        func = NULL;

    }
    //thread->context = (uthread_ctx_t)malloc(sizeof(uthread_ctx_t));
    if(uthread_ctx_init(&thread->context, thread->stack, func, arg) == -1){
        return -1;
    }
    //set tid from the storing threadIdCount
    thread->tid = scheduleController->threadIdCount;
    scheduleController->threadIdCount += 1;
    return 0;
}




void uthread_yield(void)
{
    preempt_disable();
    //temporary store the current running thread
    uthread_tcb* savedThread = scheduleController->runningThread;
    //if thread is not main thread, put it back to ready queue
    if(savedThread->tid != 0){
        //set state to ready
        scheduleController->runningThread->threadState = STATE_READY;
        //put it back to ready queue
        if(queue_enqueue(scheduleController->queueOfReady,
                         scheduleController->runningThread) == -1){
            return;
        }
    }

    //if queue is not empty, get thread from the ready else from mainthread
    if(queue_length(scheduleController->queueOfReady) != 0){
        //dequeue from the ready queue
        queue_dequeue(scheduleController->queueOfReady,
                      (void**)&scheduleController->runningThread);
        scheduleController->runningThread->threadState = STATE_RUNNING;
    }else{
        //set main thread to running thread
        scheduleController->runningThread = scheduleController->mainThread;
        scheduleController->runningThread->threadState = STATE_RUNNING;
    }
    preempt_enable();
    //switch thread
    uthread_ctx_switch(&savedThread->context,
                       &scheduleController->runningThread->context);


}

void uthread_exit(void)
{
    preempt_disable();
    //temporary store the context for switch
    uthread_ctx_t context = scheduleController->runningThread->context;
    //set state to zombie and store in the zombie queue
    scheduleController->runningThread->threadState = STATE_EXIT;
    if(queue_enqueue(scheduleController->queueOfZombie,
                     (void*)scheduleController->runningThread) == -1){
        return;
    }

    //if queue is not empty, get thread from the ready else from mainthread
    if(queue_length(scheduleController->queueOfReady) != 0){
        //dequeue from the ready queue
        queue_dequeue(scheduleController->queueOfReady,
                      (void**)&scheduleController->runningThread);
        scheduleController->runningThread->threadState = STATE_RUNNING;
    }else{
        scheduleController->runningThread = scheduleController->mainThread;
        scheduleController->runningThread->threadState = STATE_RUNNING;
    }
    preempt_enable();
    //switch thread
    uthread_ctx_switch(&context, &scheduleController->runningThread->context);
}

int uthread_create(uthread_func_t func, void *arg)
{
    //create space for thread
    uthread_tcb* thread = (uthread_tcb*)malloc(sizeof(uthread_tcb));
    //initialize thread
    if(ThreadInitialize(func, arg, thread) == -1){
        return -1;
    }
    //store it back to ready queue
    if(queue_enqueue(scheduleController->queueOfReady, (void*)thread) == -1){
        return -1;
    }
    return 0;
}


int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    if(preempt){
        preempt_start(preempt);
    }
    //create space for queue and main controller and set count to zero
    scheduleController = (ScheduleController*)malloc(sizeof(ScheduleController));
    scheduleController->queueOfBlocked = queue_create();
    scheduleController->queueOfReady = queue_create();
    scheduleController->queueOfZombie = queue_create();
    scheduleController->threadIdCount = 0;
    //if queue creating fail, return -1
    if(scheduleController->queueOfBlocked == NULL ||
       scheduleController->queueOfReady == NULL ||
       scheduleController->queueOfZombie == NULL){
        return -1;
    }

    //initialize the main thread
    uthread_tcb* mainThread = (uthread_tcb*)malloc(sizeof(uthread_tcb));
    if(ThreadInitialize(func, arg, mainThread) == -1){
        return -1;
    }
    //set main thread to main and running
    scheduleController->mainThread = mainThread;
    scheduleController->runningThread = mainThread;
    //initialize the thread need to run and yield
    uthread_create(func, arg);
    uthread_yield();

    return 0;

}

void uthread_block(void)
{
    preempt_disable();
    //temporary store the current running thread
    uthread_tcb* savedThread = scheduleController->runningThread;
    //if thread is not main thread, put it back to block queue
    if(savedThread->tid != 0){
        scheduleController->runningThread->threadState = STATE_BLOCKED;
        if(queue_enqueue(scheduleController->queueOfBlocked,
                         scheduleController->runningThread) == -1){
            return;
        }
    }

    //if queue is not empty, get thread from the ready else from mainthread
    if(queue_length(scheduleController->queueOfReady) != 0){
        //dequeue from the ready queue
        queue_dequeue(scheduleController->queueOfReady,
                      (void**)&scheduleController->runningThread);
        scheduleController->runningThread->threadState = STATE_RUNNING;
    }else{
        //set main thread to running thread
        scheduleController->runningThread = scheduleController->mainThread;
        scheduleController->runningThread->threadState = STATE_RUNNING;
    }
    preempt_enable();
    //switch thread
    uthread_ctx_switch(&savedThread->context,
                       &scheduleController->runningThread->context);

}

void uthread_unblock(struct uthread_tcb *uthread)
{

    //delete uthread from block queue
    queue_delete(scheduleController->queueOfBlocked, uthread);
    //put thread back to the ready queue
    queue_enqueue(scheduleController->queueOfReady, uthread);

}
