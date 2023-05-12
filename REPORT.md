# Report for Project 2(User-level thread library)
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
queue. Our implementation provides O(1) time complexity for all the functions
except `queue_delete()` and `queue_iterate()`, which satisfies the constraint of
this exercise. And the time complexity for `queue_delete()`
and `queue_iterate()` are O(n). Overall, the implementation is based on a singly
linked list. We first create a struct called `node`, which represents a single
element in the queue. We also define a struct called `queue` that contains
pointers to the head and tail nodes, and an integer to track the size of the
queue.

We first implement the `queue_create()` function, which allocates an empty
queue, create a new object of type 'struct queue' and return its address. For
the `queue_destroy()` function, it mainly deallocates a queue and frees its
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
the queue and calls the given callback function on each item. During the
function, it will check if the current node has been deleted, or it might cause
segmentation fault during execution. Finally, the `queue_length()` function
simply returns the current length of the queue, which is stores as int `size` in
the `struct queue`.
### Unit Testing
In the phase of unit testing, we created a new file called `queue_tester.c` for
a suite of unit tests to verify the correctness of our queue implementation. The
unit tests are organized into individual functions that cover different aspects
of the queue functionality.

1. **test_create**: This test checks if the `queue_create()` function
   successfully allocates an empty queue and returns a non-null pointer.
2. **test_queue_simple**: This test checks basic enqueue and dequeue
   functionality.
3. **test_enqueue_length**: This test checks the functionality
   of `queue_enqueue()` and `queue_length()`.
4. **test_dequeue**: This test checks the functionality of `queue_dequeue()`. It
   enqueues multiple items and then dequeues them, verifying the correct order
   and updating the queue length.
5. **test_dequeue_empty**: This test checks the behavior of `queue_dequeue()`
   when applied to an empty queue.
6. **test_delete**: This test checks the functionality of `queue_delete()`. It
   enqueues multiple items, deletes them, and verifies that the queue length is
   updated accordingly.
7. **test_delete_null**: This test checks the behavior of `queue_delete()` when
   asked to delete a NULL data.
8. **test_iterator**: This test checks the functionality of `queue_iterate()`.
   It enqueues multiple items and then iterates through the queue, applying a
   callback function that increments the items and deletes a specific item (42)
   from the queue. The test verifies that the item has been deleted and that the
   remaining items have been incremented.
9. **test_destroy_non_empty**: This test checks the behavior
   of `queue_destroy()` when applied to a non-empty queue.
10. **test_print_queue**: This test demonstrates how to use the
    `queue_iterate()` function to print the contents of the queue.

These tests were executed in the `main()` function of the test program to ensure
that the queue implementation passed all tests before proceeding with the
remainder of the project. 
## Phase 2: uthread API
In this phase, we implemented the user-level thread (uthread) API. The
implementation is managed entirely in user space, without requiring kernel-level
multithreading support. The primary components of our implementation are the
Thread Control Block (TCB), a ready queue, and various functions
including `uthread_yield`, `uthread_exit`, `uthread_create`, and `uthread_run`.

Each thread is characterized by a Thread Control Block (TCB) which includes the
thread's execution context, its current state (`running_state`, `ready_state`,
or `exited_state`), and a pointer to the thread's stack. Alongside, we maintain
a ready queue, a data structure that holds the threads ready for execution but
not currently running. Threads are enqueued or dequeued from this queue based on
their state and CPU availability.

Our implementation provides several key thread operations. The `uthread_yield`
function allows a thread to voluntarily yield the CPU to another thread. If the
currently running thread isn't finished, it's state is switched back to ready,
and it is placed back into the ready queue. The `uthread_exit` function allows a
thread to indicate it has finished executing, updating its state to exited and
yielding the CPU to another thread.

To create new threads, we use the `uthread_create` function. This function
allocates and initializes a new TCB, including a new stack, and places the new
thread into the ready queue. Finally, the `uthread_run` function, as the main
driver function, initiates the thread scheduler. It creates the ready queue,
designates the idle thread as the current thread, and creates the initial
thread. It then enters a loop, repeatedly yielding control to the next ready
thread until all threads have completed. We conducted the test using the files 
`uthread_hello.c` and `uthread_yield.c` that were provided by the professor, 
and our own tester `uthread_tester.c`.
## Phase 3: Semaphore
In this phase, to manage synchronization among multiple threads in a
multithreaded environment, we focus on the implementation of semaphores, a
fundamental synchronization primitive.

Our semaphore structure contains two major components: a `count` that signifies
the number of available resources, and a `queue` which serves to queue up
threads that are blocked and waiting for a resource.
```c
struct semaphore {
    size_t count;   // Number of resources available
    queue_t queue;  // Queue of threads waiting for this semaphore
};
```
Creating a new semaphore is performed by the `sem_create` function, which
initializes a semaphore with a given count. The `sem_destroy` function is
responsible for semaphore cleanup, ensuring no threads are still waiting.

The `sem_down` function is used to acquire a resource, decrementing the
semaphore's count, and if no resources are available, the calling thread is
blocked and added to the semaphore's queue. Conversely, the `sem_up` function
releases a resource, incrementing the semaphore's count, and if there are
threads in the queue, one of them is unblocked.

To ensure a blocked thread that has been awakened by `sem_up` does not proceed
when its requested resource has been taken by another thread, we make use of a
while loop to check the count after the thread is unblocked in the `sem_down`
function. This allows the thread to re-block itself and return to the queue if
its resource was snatched before it could run.

To prevent thread starvation, the `sem_up` function always unblocks the longest
waiting thread at the front of the queue.

In addition to the semaphore, we introduced a new state `THREAD_BLOCKED` and a
new queue `blocked_queue` to represent threads that are waiting for a semaphore.
```c
typedef enum {
    ...
    THREAD_BLOCKED // Blocked State
} thread_state_t;

static queue_t blocked_queue = NULL;// Queue of threads that are blocked
```
We also implement the `uthread_block` and `uthread_unblock`  functions in
uthread library for managing thread states. Whenever a thread enters
the `THREAD_BLOCKED` state, it gets added to the `blocked_queue`.
The `uthread_block()` function is responsible for this transition. It changes
the state of the current thread to `THREAD_BLOCKED` and enqueues it to
the `blocked_queue`. Then, it yields control to the next thread by
calling `uthread_yield()` and enables preemption before returning.

The `uthread_unblock(struct uthread_tcb *uthread)` function
complements `uthread_block(void)`. It's responsible for transitioning a thread
from the `THREAD_BLOCKED` state back to the `THREAD_READY` state. It changes the
state of the specified thread to `THREAD_READY`, removes it from
the `blocked_queue`, and enqueues it to the `ready_queue`, thus making it
available for scheduling. We conducted tests using `sem_simple.c`, `sem_prime.c`
 `sem_count.c`, and `sem_buffer.c` that were supplied by the professor.
## Phase 4: preemption
We mainly implemented preemption for the library. It is a mechanism that allows
the operating system to interrupt the execution of a running thread and switch
to another thread and can helps to maintain fairness and improve the overall
system responsiveness.

For `alarm_handler`, it is a signal handler for the virtual alarm signal, and it
simply calls `uthread_yield` to yield the CPU from the current
thread. `preempt_disable` is a function that blocks the virtual alarm signal and
disables preemption. It first initializes an empty signal set, then adds
SIGVTALRM to this set, and finally calls `sigprocmask` to block this set of
signals. `preempt_enable` is a function that unblocks the virtual alarm signal
and reenables the preemption. The implementation is pretty similar
to `preempt_disable`. `preempt_start` mainly sets up the preemption. It first
sets the alarm_handler function as the signal handler for SIGVTALRM, then it
initializes a timer to raise SIGVTALRM on a regular basis. Last, `preempt_stop`
stops the preemption. It first stops the timer by calling `setitimer` with a
NULL argument. Then, it resets the signal handler for SIGVTALRM to the default
handler. We also added a new test program in the apps directory
called `test_preempt.c` to test the preemption. In the program, we mainly
created two threads `thread1` and `thread2`. `thread2` contains an infinite
while loop,which means it will take the CPU forever, unless the preemption is
enabled. During testing, we tested after `thread1` yields to `thread2`, whether
it could come back to `thread1` and prints "Back to thread 1.". If so, it means
we implemented the `preempt.c` correctly.