/*
 * Custom tester to test our uthread implementation.
 * the program should output:
 *
 * thread1
 * thread2
 * thread3
 * thread4
 * thread5
 * thread6
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

void thread6(void *arg)
{
    (void)arg;

    uthread_yield();
    printf("thread6\n");
}

void thread5(void *arg)
{
    (void)arg;

    uthread_yield();
    printf("thread5\n");
}

void thread4(void *arg)
{
    (void)arg;

    uthread_create(thread6, NULL);
    uthread_yield();
    printf("thread4\n");
}

void thread3(void *arg)
{
    (void)arg;

    uthread_create(thread4, NULL);
    uthread_create(thread5, NULL);
    uthread_yield();
    printf("thread3\n");
}

void thread2(void *arg)
{
    (void)arg;

    uthread_create(thread3, NULL);
    uthread_yield();
    printf("thread2\n");
}

void thread1(void *arg)
{
    (void)arg;

    uthread_create(thread2, NULL);
    uthread_yield();
    printf("thread1\n");
    uthread_yield();
}

int main(void)
{
    uthread_run(false, thread1, NULL);
    return 0;
}
