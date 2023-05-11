#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#include <uthread.h>

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)


void thread2(void *arg)
{
	(void)arg;
	

	
	printf("thread2\n");
	return;
}

void thread1(void *arg)
{
	(void)arg;
	
	uthread_create(thread2, NULL);
	printf("thread1\n");
    while(1){

	}
	printf("finish thread1\n");
	return;
}

int main(void)
{
	uthread_run(true, thread1, NULL);
	return 0;
}
