# Report for Prject 2(User-level thread library)

## Summary

This program, `User-level thread library`, mainly implements a basic user-level thread library for Linux. Our library provides a complete interface for applications to create and run independent threads concurrently, and it contains the following functions:

1. Create new thread.

2. Schedule the threads execution in round-robin fashion.

3. Provide an interrupt-based scheduler.

4. Provide a thread synchronization API called semaphores.


## Implementation

1. Implement a single FIFO queue.

2. Implement the uthread API.

3. Implement the semaphore API.

4. Add preemption API to library.

## Phase 1: Running simple commands the hard way

In this phase, we add code into libuthread/queue.c to implement a simple FIFO queue. Our implementation provides O(1) time complexity for `queue_create()`, `queue_destroy()`, `queue_enqueue()`, `queue_dequeue()`, `queue_length()`, which satisfies the constraint of this exercise. And the time complexity for `queue_delete()` and `queue_iterate()` are O(n). Overall, the implementation is based on a singly linked list. We first create a struct called `node`, which represents a single element in the queue. The node contains a pointer to the data and a pointer to the next node in the queue. We also define a struct called `queue` that contains pointers to the head and tail nodes, as well as an integer to track the size of the queue.

We first implement the `queue_create()` function, which allocates an empty queue, create a new object of type 'struct queue' and return its address. The head and tail pointers are set to `NULL`, and the size is set to 0. For the `queue_destroy()` funciton, it mainly deallocates a queue and frees its memory. If the queue is null or the queue is not empty, it will return -1; else, it will frees the memory by using `free()` and then return 0. Then, we implements the `queue_enqueue()` function, which adds an element to the tail of the queue. We first create a new node with the provided data and set its next pointer to `NULL`. If the queue is empty, then both the head and tail pointers are set to the new node. Otherwise, the next pointer of tail nodes is set to the new node, and the tail pointer is updated to the new node. Also, the size of the queue will increase by 1 in both cases.  The `queue_deque()` function removes the element from the head of the queue and returns its data. When the queue is not empty, it retrieves the data from the head node, updates the head pointer to the next node in the queue, frees the memory associated with the removed node, and decrements the length of the queue by 1. When queue size equals 1 before dequeue, it will set the queue tail to `NULL`. The `queue_delete()` function searches the first (ie oldest) item with specific data and deletes the corresponding node if found. It iterates through queue, comparing the data pointers of each node with the provided data. When they are match, the node is removed from the queue, its memory is freed, and the size of queue is decremented by 1. The `queue_iterate()` function iterates through the items in the queueand calls the given callback function on each item. During the function, it will check if the current node has been deleted, or it might cause segmentation fault during execution. Finally, the `queue_length()` function simply returns the current length of the queue, which is stores as int `size` in the `struct queue`.
