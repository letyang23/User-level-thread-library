#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue_node {
    void *data;
    struct queue_node *next;
};

struct queue {
	/* TODO Phase 1 */
    struct queue_node *head;
    struct queue_node *tail;
    int length;
};

queue_t queue_create(void)
{
	/* TODO Phase 1 */
    queue_t new_queue = (queue_t) malloc(sizeof(struct queue));
    if(new_queue == NULL)
        return NULL;

    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->length = 0;
    return new_queue;
}

int queue_destroy(queue_t queue)
{
	/* TODO Phase 1 */
    if (queue == NULL || queue->length > 0)
        return -1;

    free(queue);
    return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	/* TODO Phase 1 */
    if(queue == NULL || data == NULL)
        return -1;

    struct queue_node *new_node = (struct queue_node *) malloc(sizeof(struct queue_node));
    if(new_node == NULL)
        return -1;

    new_node->data = data;
    new_node->next = NULL;

    if(queue->tail)
        queue->tail->next = new_node;
    else
        queue->head = new_node;

    queue->tail = new_node;
    queue->length++;
    return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	/* TODO Phase 1 */
}

int queue_delete(queue_t queue, void *data)
{
	/* TODO Phase 1 */
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	/* TODO Phase 1 */
}

int queue_length(queue_t queue)
{
	/* TODO Phase 1 */
}

