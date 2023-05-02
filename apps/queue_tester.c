#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

//#include <queue.h>
#include "../libuthread/queue.h"

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

/* Callback function that increments items */
static void iterator_inc(queue_t q, void *data) {
    int *a = (int *) data;

    if (*a == 42)
        queue_delete(q, data);
    else
        *a += 1;
}

/* Callback function to print items in the queue */
static void print_item(queue_t q, void *data) {
    int *item = (int *)data;

    /* Indicate that the 'q' parameter is unused */
    (void)q;

    printf("%d ", *item);
}

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

/* Test the queue_iterate function */
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

/* Test enqueue and length */
void test_enqueue_length(void) {
    queue_t q;
    int data1 = 1, data2 = 2, data3 = 3;

    fprintf(stderr, "*** TEST enqueue_length ***\n");

    q = queue_create();
    TEST_ASSERT(queue_length(q) == 0);

    queue_enqueue(q, &data1);
    TEST_ASSERT(queue_length(q) == 1);

    queue_enqueue(q, &data2);
    TEST_ASSERT(queue_length(q) == 2);

    queue_enqueue(q, &data3);
    TEST_ASSERT(queue_length(q) == 3);

    queue_destroy(q);
}

/* Test dequeue */
void test_dequeue(void) {
    queue_t q;
    int data1 = 1, data2 = 2, data3 = 3;
    int *dequeued_item;

    fprintf(stderr, "*** TEST dequeue ***\n");

    q = queue_create();
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    queue_dequeue(q, (void **)&dequeued_item);
    TEST_ASSERT(dequeued_item == &data1);
    TEST_ASSERT(queue_length(q) == 2);

    queue_dequeue(q, (void **)&dequeued_item);
    TEST_ASSERT(dequeued_item == &data2);
    TEST_ASSERT(queue_length(q) == 1);

    queue_dequeue(q, (void **)&dequeued_item);
    TEST_ASSERT(dequeued_item == &data3);
    TEST_ASSERT(queue_length(q) == 0);

    queue_destroy(q);
}

/* Test delete*/
void test_delete(void) {
    queue_t q;
    int data1 = 1, data2 = 2, data3 = 3;
    int result;

    fprintf(stderr, "*** TEST delete ***\n");

    q = queue_create();
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    result = queue_delete(q, &data2);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(queue_length(q) == 2);

    result = queue_delete(q, &data1);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(queue_length(q) == 1);

    result = queue_delete(q, &data3);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(queue_length(q) == 0);

    queue_destroy(q);
}

/* Test destroy with non-empty queue */
void test_destroy_non_empty(void) {
    queue_t q;
    int data1 = 1;
    int result;

    fprintf(stderr, "*** TEST destroy_non_empty ***\n");

    q = queue_create();
    queue_enqueue(q, &data1);

    result = queue_destroy(q);
    TEST_ASSERT(result == -1);

    queue_dequeue(q, NULL);
    queue_destroy(q);
}

/* Prints all the data in the queue using the queue_iterate() function */
void test_print_queue(void) {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5};
    size_t i;

    fprintf(stderr, "*** TEST print_queue ***\n");

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
        queue_enqueue(q, &data[i]);
    }

    /* Print items in the queue using queue_iterate() */
    printf("Queue contents: ");
    queue_iterate(q, print_item);
    printf("\n");

    queue_destroy(q);
}

int main(void) {
    test_create();
    test_queue_simple();
    test_iterator();
    test_enqueue_length();
    test_dequeue();
    test_delete();
    test_destroy_non_empty();
    test_print_queue();

    return 0;
}
