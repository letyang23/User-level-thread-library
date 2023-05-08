#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "string.h"


/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100


struct itimerval New;
struct itimerval Old;
struct sigaction Sigaction;


//if argument is 1 start the alarm else stop
//initialize the alarm
int Alarm(int alarmType){
    //use for initialize the alarm
    New.it_interval.tv_usec = 0;
    New.it_value.tv_usec = 0;
    if(alarmType == 1){
        New.it_interval.tv_usec = 1000000/HZ;
        New.it_value.tv_usec = 1000000/HZ;
    }
    //set timer
    New.it_interval.tv_sec = 0;
    New.it_value.tv_sec = 0;
    if (setitimer (ITIMER_VIRTUAL, &New, &Old) < 0){
        return 0;
    }

    return 1;
}

void AlarmHandler(){
    //block the thread which need to interrupt
    uthread_yield();
    return;
}

void preempt_disable(void)
{
    //disable, by set the SIG_UNBLOCK
    if(sigprocmask(SIG_UNBLOCK, &Sigaction.sa_mask, NULL) == -1){
        return;
    }
}

void preempt_enable(void)
{
    //disable, by set the SIG_BLOCK
    if(sigprocmask(SIG_BLOCK, &Sigaction.sa_mask, NULL) == -1){
        return;
    }

}

void preempt_start(bool preempt)
{

    if(preempt){
        //set the sigaction
        sigemptyset(&Sigaction.sa_mask);
        Sigaction.sa_flags = 0;
        //set interrupt handler and signal
        Sigaction.sa_handler = AlarmHandler;
        sigaction(SIGVTALRM, &Sigaction, NULL);
        sigaddset(&Sigaction.sa_mask, SIGVTALRM);
        sigprocmask(SIG_UNBLOCK, &Sigaction.sa_mask, NULL);

        Alarm(1);

    }
}


void preempt_stop(void)
{
    //remove the alarm and sigaction
    Alarm(0);
    Sigaction.sa_handler = SIG_DFL;
    sigemptyset(&Sigaction.sa_mask);
    if(sigaction(SIGVTALRM, &Sigaction, NULL)){
        return;
    }
}
