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

    // Assert that queue_create() successfully creates a non-NULL queue
    TEST_ASSERT(queue_create() != NULL);
}

/* Test basic enqueue and dequeue functionality */
void test_queue_simple(void) {
    int data = 3, *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_simple ***\n");

    // Create a new queue and enqueue an item
    q = queue_create();
    queue_enqueue(q, &data);

    // Dequeue the item and check that it matches the enqueued item
    queue_dequeue(q, (void **) &ptr);
    TEST_ASSERT(ptr == &data);
}

/* Test iterating through a queue and applying a callback function */
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

/* Test enqueueing items and checking the length of the queue */
void test_enqueue_length(void) {
    queue_t q;
    int data1 = 1, data2 = 2, data3 = 3;

    fprintf(stderr, "*** TEST enqueue_length ***\n");

    // Create a new queue and check its initial length
    q = queue_create();
    TEST_ASSERT(queue_length(q) == 0);

    // Enqueue items and check the length after each enqueue
    queue_enqueue(q, &data1);
    TEST_ASSERT(queue_length(q) == 1);

    queue_enqueue(q, &data2);
    TEST_ASSERT(queue_length(q) == 2);

    queue_enqueue(q, &data3);
    TEST_ASSERT(queue_length(q) == 3);

    // Clean up by destroying the queue
    queue_destroy(q);
}

/* Test dequeueing items and checking the length of the queue */
void test_dequeue(void) {
    queue_t q;
    int data1 = 1, data2 = 2, data3 = 3;
    int *dequeued_item;

    fprintf(stderr, "*** TEST dequeue ***\n");

    // Create a new queue, enqueue items, and check the initial length
    q = queue_create();
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    // Dequeue items and check the dequeued item and length after each dequeue
    queue_dequeue(q, (void **)&dequeued_item);
    TEST_ASSERT(dequeued_item == &data1);
    TEST_ASSERT(queue_length(q) == 2);

    queue_dequeue(q, (void **)&dequeued_item);
    TEST_ASSERT(dequeued_item == &data2);
    TEST_ASSERT(queue_length(q) == 1);

    queue_dequeue(q, (void **)&dequeued_item);
    TEST_ASSERT(dequeued_item == &data3);
    TEST_ASSERT(queue_length(q) == 0);

    // Clean up by destroying the queue
    queue_destroy(q);
}

/* Test deleting items from the queue */
void test_delete(void) {
    queue_t q;
    int data1 = 1, data2 = 2, data3 = 3;
    int result;

    fprintf(stderr, "*** TEST delete ***\n");

    // Create a new queue, enqueue items, and check the initial length
    q = queue_create();
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    // Delete items and check the result and length after each delete
    result = queue_delete(q, &data2);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(queue_length(q) == 2);

    result = queue_delete(q, &data1);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(queue_length(q) == 1);

    result = queue_delete(q, &data3);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(queue_length(q) == 0);

    // Attempt to delete a non-existent item and check the result
    result = queue_delete(q, &data1);
    TEST_ASSERT(result == -1);

    // Clean up by destroying the queue
    queue_destroy(q);
}

/* Test destroy with non-empty queue */
void test_destroy_non_empty(void) {
    queue_t q;
    int data1 = 1;
    int result;

    fprintf(stderr, "*** TEST destroy_non_empty ***\n");

    // Create a new queue, enqueue an item, and attempt to destroy it
    q = queue_create();
    queue_enqueue(q, &data1);

    // Assert that the destroy operation fails with a non-empty queue
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
