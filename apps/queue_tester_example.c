#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)                \
do {                                    \
    printf("ASSERT: " #assert " ... ");    \
    if (assert) {                        \
        printf("PASS\n");                \
    } else    {                            \
        printf("FAIL\n");                \
        exit(1);                        \
    }                                    \
} while(0)

/* Create */
void test_create(void) {
    fprintf(stderr, "*** TEST create ***\n");

    TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void) {
    int data = 3, *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_simple ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void **) &ptr);
    TEST_ASSERT(ptr == &data);
}

/* Enqueue/Dequeue multiple */
void test_queue_multiple(void) {
    int data1 = 1, data2 = 2, *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_multiple ***\n");

    q = queue_create();
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    TEST_ASSERT(queue_length(q) == 2);

    queue_dequeue(q, (void **) &ptr);
    TEST_ASSERT(ptr == &data1);
    TEST_ASSERT(queue_length(q) == 1);

    queue_dequeue(q, (void **) &ptr);
    TEST_ASSERT(ptr == &data2);
    TEST_ASSERT(queue_length(q) == 0);
}

/* Delete */
void test_queue_delete(void) {
    int data1 = 1, data2 = 2;
    queue_t q;

    fprintf(stderr, "*** TEST queue_delete ***\n");

    q = queue_create();
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);

    TEST_ASSERT(queue_delete(q, &data1) == 0);
    TEST_ASSERT(queue_length(q) == 1);
}

/* Iterate */
void test_queue_iterate(void) {
    int data1 = 1, data2 = 2;
    queue_t q;

    fprintf(stderr, "*** TEST queue_iterate ***\n");

    q = queue_create();
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);

    queue_iterate(q, (queue_func_t) printf);

    printf("\n");
}


static void iterator_inc(queue_t q, void *data) {
    int *a = (int *) data;

    if (*a == 42)
        queue_delete(q, data);
    else
        *a += 1;
}

void test_iterator(void) {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Increment every item of the queue, delete item '42' */
    queue_iterate(q, iterator_inc);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q) == 9);
}


int main(void) {
    test_create();
    test_queue_simple();
    test_queue_multiple();
    test_queue_delete();
    test_queue_iterate();
    test_iterator();

    return 0;
}
