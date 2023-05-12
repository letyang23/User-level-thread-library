#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
#define start_time 0

/* 
 * The signal handler for the virtual alarm signal.
 * It simply calls uthread_yield to yield the CPU from the current thread.
 */
void alarm_handler(int sig) 
{
    (void) sig;
	uthread_yield();
}

/*
 * This function blocks the virtual alarm signal, which disables preemption.
 */
void preempt_disable(void)
{
	sigset_t block_alarm;
	sigemptyset(&block_alarm);
	sigaddset(&block_alarm, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &block_alarm, NULL);
}

/*
 * This function unblocks the virtual alarm signal, which re-enables preemption.
 */
void preempt_enable(void)
{
	sigset_t unblock_alarm;
	sigemptyset(&unblock_alarm);
	sigaddset(&unblock_alarm, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &unblock_alarm, NULL);
}

/* 
 * This function sets up preemption.
 */
void preempt_start(bool preempt)
{
	// Set up signal
    (void) preempt;
	struct sigaction sa;
	sa.sa_handler = alarm_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGVTALRM, &sa, NULL);

	// Set up timer
	struct itimerval timer;
	timer.it_interval.tv_sec = start_time; 
    timer.it_interval.tv_usec = 1000000 / HZ;
	timer.it_value.tv_sec = start_time;
    timer.it_value.tv_usec = 1000000 / HZ;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

/* 
 * This function stops preemption.
 */
void preempt_stop(void)
{
	setitimer(ITIMER_VIRTUAL, NULL, NULL);	//Stops timer
	
	// Resets the signal handler for SIGVTALRM to the default handler.
	struct sigaction sa;
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGVTALRM, &sa, NULL)) {
		return;
	}
}

