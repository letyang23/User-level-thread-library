#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct node {
    void *data;
    struct node *next;
} node_t;

struct queue {
    int size;
    node_t *head;
    node_t *tail;
};

queue_t queue_create(void) {
    queue_t new_queue = (queue_t) malloc(sizeof(struct queue));

    if (new_queue == NULL) {
        return NULL;
    }

    new_queue->size = 0;
    new_queue->head = NULL;
    new_queue->tail = NULL;
    return new_queue;
}

int queue_destroy(queue_t queue) {
    if (queue == NULL || queue->size != 0) {
        return -1;
    } else {
        free(queue);
        return 0;
    }
}

int queue_enqueue(queue_t queue, void *data) {
    if (queue == NULL || data == NULL) {
        return -1;
    }

    node_t *new_node = (node_t *) malloc(sizeof(node_t));
    new_node->data = data;
    new_node->next = NULL;
    if (new_node == NULL) {
        return -1;
    }

    if (queue->size == 0) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    ++queue->size;
    return 0;
}

int queue_dequeue(queue_t queue, void **data) {
    if (queue == NULL || data == NULL || queue->size == 0) {
        return -1;
    }
    node_t *cur = queue->head;
    *data = cur->data;
    queue->head = cur->next;

    if (queue->size == 1) {
        queue->tail = NULL;
    }
    --queue->size;
    free(cur);
    return 0;
}

int queue_delete(queue_t queue, void *data) {
    if (queue == NULL || data == NULL) {
        return -1;
    }

    node_t *cur = queue->head;
    node_t *prev = NULL;
    int match = 0;

    while (cur != NULL) {
        if (cur->data == data) {
            match = 1;
            break;
        } else {
            prev = cur;
            cur = cur->next;
        }
    }

    if (match) {
        if (prev == NULL) {
            queue->head = cur->next;
        } else {
            prev->next = cur->next;
        }

        if (cur->next == NULL) {
            queue->tail = prev;
        }
        free(cur);
        --queue->size;
        return 0;
    }

    return -1;
}

int queue_iterate(queue_t queue, queue_func_t func) {
    if (queue == NULL || func == NULL) {
        return -1;
    }

    node_t *cur = queue->head;
    node_t *prev = NULL;
    node_t *next = NULL;

    while (cur != NULL) {
        next = cur->next; // Save the next node before the callback
        func(queue, cur->data);

        // Check if the current node has been deleted
        if (prev == NULL && cur != queue->head) {
            cur = queue->head;
        } else if (prev != NULL && prev->next != cur) {
            cur = prev->next;
        } else {
            prev = cur;
            cur = next;
        }
    }

    return 0;
}

int queue_length(queue_t queue) {
    if (queue == NULL) {
        return -1;
    }
    return queue->size;
}

