#include "executor.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "future.h"
#include "mio.h"
#include "waker.h"

extern FutureState join_wrapper_progress(Future* fut, Mio* mio, Waker external_waker);
extern FutureState select_wrapper_progress(Future* fut, Mio* mio, Waker external_waker);

/* 
 * Executor structure.
 * In addition to the Mio instance, the executor contains a task queue (ring buffer) and a counter
 * of active (spawned but not yet completed) Futures.
 */
struct Executor {
    Future** queue;         // Array of pointers to Futures,
                            // representing tasks ready to make progress.
    size_t max_queue_size;  // Size of the queue.
    size_t head;            // Index of the first element in the queue.
    size_t tail;            // Index of the next position to insert.
    size_t count;           // Number of elements currently in the queue.
    size_t active_count;    // Number of active (spawned) Futures.
    Mio* mio;               // Mio instance for waiting on events.
};

/*
 * Helper function: enqueue – adds a task to the queue.
 * Assumes that the number of elements will not exceed max_queue_size.
 */
static void enqueue(Executor* executor, Future* fut) {
    executor->queue[executor->tail] = fut;
    executor->tail = (executor->tail + 1) % executor->max_queue_size;
    executor->count++;
}

/*
 * Helper function: dequeue – retrieves a task from the queue.
 */
static Future* dequeue(Executor* executor) {
    Future* fut = executor->queue[executor->head];
    executor->head = (executor->head + 1) % executor->max_queue_size;
    executor->count--;
    return fut;
}

/*
 * Creates a new executor.
 * Allocates memory for the Executor structure, a queue of the given size,
 * and creates a Mio instance.
 */
Executor* executor_create(size_t max_queue_size) {
    Executor* executor = malloc(sizeof(Executor));
    if (!executor)
        return NULL;
    
    executor->queue = malloc(max_queue_size * sizeof(Future*));
    if (!executor->queue) {
        free(executor);
        return NULL;
    }
    
    executor->max_queue_size = max_queue_size;
    executor->head = 0;
    executor->tail = 0;
    executor->count = 0;
    executor->active_count = 0;
    
    executor->mio = mio_create(executor);
    if (!executor->mio) {
        free(executor->queue);
        free(executor);
        return NULL;
    }
    
    return executor;
}

/*
 * The waker_wake function is called when some mechanism (e.g., Mio)
 * detects that a task can make further progress.
 * In that case, waker_wake enqueues the given Future into the executor's queue.
 */
void waker_wake(Waker* waker) {
    Executor* executor = (Executor*)waker->executor;
    Future* fut = waker->future;
    enqueue(executor, fut);
}

/*
 * The executor_spawn function adds a new task to the executor.
 * It sets the is_active flag to true (pinning the task) and increments
 * the count of active tasks.
 */
void executor_spawn(Executor* executor, Future* fut) {
    if (!fut->is_active) {
        fut->is_active = true;
        executor->active_count++;
        enqueue(executor, fut);
    }
}

/*
 * The executor_run function – main loop of the executor.
 *
 * As long as the number of active tasks (active_count) is greater than zero:
 *   - If the task queue is empty, call mio_poll() to put the executor to sleep
 *     until some event occurs (which should enqueue a task via waker_wake).
 *   - When the queue is not empty, dequeue a task, create a Waker for it,
 *     call future.progress(), and react to the returned state.
 *
 * If future.progress() returns FUTURE_COMPLETED or FUTURE_FAILURE, the task
 * finishes its execution – set is_active to false and decrement the active task count.
 *
 * If future.progress() returns FUTURE_PENDING, assume that the task itself (e.g., via
 * waker_wake) will be re-enqueued when it can make further progress.
 */
void executor_run(Executor* executor) {
    while (executor->active_count > 0) {
        if (executor->count == 0) {
            /* No tasks are ready – waiting for events (e.g., I/O readiness). */
            mio_poll(executor->mio);
        }
        while (executor->count > 0) {
            Future* fut = dequeue(executor);
            Waker waker = {
                .executor = executor,
                .future = fut,
            };
            FutureState state = fut->progress(fut, executor->mio, waker);
            if (state == FUTURE_COMPLETED || state == FUTURE_FAILURE) {
                fut->is_active = false;
                executor->active_count--;
                // If progress points to a wrapper – free the memory.
                if (fut->progress == join_wrapper_progress || fut->progress == select_wrapper_progress) {
                    free(fut);
                }
            }
            /* In the case of FUTURE_PENDING – assume that the task itself (via the waker)
            will be re-enqueued if needed. */
        }
    }
}

/*
 * Frees the executor's resources – destroys the Mio instance, the queue, and the executor structure itself.
 */
void executor_destroy(Executor* executor) {
    mio_destroy(executor->mio);
    free(executor->queue);
    free(executor);
}