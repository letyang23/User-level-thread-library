#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uthread.h>

void thread2(void *arg) {
    (void)arg;
    printf("Thread 2, you will stuck in a while loop!");
    while(1) {
        printf("In the loop...\n");
    }
}
void thread1(void *arg) {
    (void)arg;
    printf("Thread 1, can run successfully.\n");
    uthread_create(thread2, NULL);
    uthread_yield();
    printf("Back to thread 1.\n");
    exit(0);
}
int main(void) {
    uthread_run(true, thread1, NULL);
    return 0;
}