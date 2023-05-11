# Report for Prject 2(User-level thread library)

## Summary

This program, `User-level thread library`, mainly implements a basic user-level
thread library for Linux. Our library provides a complete interface for
applications to create and run independent threads concurrently, and it contains
the following functions:

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

In this phase, we add code into libuthread/queue.c to implement a simple FIFO
queue. Our implementation provides O(1) time complexity
for all the functions except `queue_delete()` and `queue_iterate()`,
which satisfies the constraint of this exercise. And the time complexity
for `queue_delete()` and `queue_iterate()` are O(n). Overall, the implementation
is based on a singly linked list. We first create a struct called `node`, which
represents a single element in the queue. We also define a struct
called `queue` that contains pointers to the head and tail nodes, and an
integer to track the size of the queue.

We first implement the `queue_create()` function, which allocates an empty
queue, create a new object of type 'struct queue' and return its address. For
the `queue_destroy()` funciton, it mainly deallocates a queue and frees its
memory. If the queue is null or the queue is not empty, it will return -1; else,
it will frees the memory by using `free()` and then return 0. Then, we
implements the `queue_enqueue()` function, which adds an element to the tail of
the queue. We first create a new node with the provided data and set its next
pointer to `NULL`. If the queue is empty, then both the head and tail pointers
are set to the new node. Otherwise, the next pointer of tail nodes is set to the
new node, and the tail pointer is updated to the new node. Also, the size of the
queue will increase by 1 in both cases. The `queue_deque()` function removes the
element from the head of the queue and returns its data. When the queue is not
empty, it retrieves the data from the head node, updates the head pointer to the
next node in the queue, frees the memory associated with the removed node, and
decrements the length of the queue by 1. When queue size equals 1 before
dequeue, it will set the queue tail to `NULL`. The `queue_delete()` function
searches the first (ie oldest) item with specific data and deletes the
corresponding node if found. It iterates through queue, comparing the data
pointers of each node with the provided data. When they are match, the node is
removed from the queue, its memory is freed, and the size of queue is
decremented by 1. The `queue_iterate()` function iterates through the items in
the queueand calls the given callback function on each item. During the
function, it will check if the current node has been deleted, or it might cause
segmentation fault during execution. Finally, the `queue_length()` function
simply returns the current length of the queue, which is stores as int `size` in
the `struct queue`.

### Unit Testing

In the phase of unit testing, we created a new file called `queue_testr.c` for a
suite of unit tests to verify the correctness of our queue implementation. The
unit tests are organized into individual functions that cover different aspects
of the queue functionality. We used a simple macro `TEST_ASSERT` provided by the
professor to print test results in a user-friendly manner and to exit the
program in case of test failure.

1. **test_create**: This test checks if the `queue_create()` function
   successfully allocates an empty queue and returns a non-null pointer.
2. **test_queue_simple**: This test checks basic enqueue and dequeue
   functionality. It enqueues a single integer and then dequeues it, verifying
   that the dequeued item matches the enqueued item.
3. **test_enqueue_length**: This test checks the functionality
   of `queue_enqueue()` and `queue_length()`. It enqueues multiple items and
   checks if the queue length is updated accordingly.
4. **test_dequeue**: This test checks the functionality of `queue_dequeue()`. It
   enqueues multiple items and then dequeues them, verifying the correct order
   and updating the queue length.
5. **test_delete**: This test checks the functionality of `queue_delete()`. It
   enqueues multiple items, deletes them, and verifies that the queue length is
   updated accordingly. It also checks the case when an item is not found in the
   queue.
6. **test_iterator**: This test checks the functionality of `queue_iterate()`.
   It enqueues multiple items and then iterates through the queue, applying a
   callback function that increments the items and deletes a specific item (42)
   from the queue. The test verifies that the item has been deleted and that the
   remaining items have been incremented.
7. **test_destroy_non_empty**: This test checks the behavior
   of `queue_destroy()` when applied to a non-empty queue. It verifies that the
   function returns an error when trying to destroy a non-empty queue.
8. **test_print_queue**: This test demonstrates how to use the `queue_iterate()`
   function to print the contents of the queue. It enqueues multiple items and
   then prints them using a custom print function as a callback.

These tests were executed in the `main()` function of the test program to ensure
that the queue implementation passed all tests before proceeding with the
remainder of the project. By thoroughly testing our queue implementation, we
were able to identify and correct any issues in the code, ensuring that our
queue data structure is robust and reliable for use in the user-level thread
library.

## Phase 2: uthread API

In this phase, we worked on implementing the uthread API in `uthread.c`. This
involved constructing the basic building blocks for a user-level thread library,
with functions to create, yield, and exit threads. To accomplish this, we used a
thread control block (TCB) to keep track of thread metadata, and we leveraged
the queue data structure from Phase 1 to manage a ready queue of threads waiting
to run.

We first defined a `uthread_tcb_t` struct, which includes a thread's execution
context, a pointer to its stack, and its current state (ready, running, or
exited). We also created global variables to reference the current thread and a
ready queue of threads.

The `uthread_run` function initializes the threading system and starts the first
thread. It creates and initializes an idle thread (a placeholder for when no
other threads are ready to run), sets the current thread to this idle thread,
and then creates the first user thread and yields the CPU to it.

The `uthread_create` function is responsible for creating a new user-level
thread. It allocates a TCB and a stack for the new thread, initializes the
thread's context to start execution at the provided function, sets the thread's
state to ready, and enqueues it onto the ready queue.

`uthread_yield` allows a running thread to voluntarily relinquish the CPU. It
enqueues the current thread back onto the ready queue, dequeues the next thread
from the ready queue, and then switches execution context from the current
thread to the next thread.

`uthread_exit` allows a thread to terminate itself. It simply sets the thread's
state to exited and then yields the CPU to another thread.

The `uthread_ctx_switch` function is an assembly routine that saves the current
execution context and restores a new one. It's used in `uthread_yield` to switch
between threads.
