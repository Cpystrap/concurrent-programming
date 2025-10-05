### Task Description

**Asynchronous Executor in C**

**Motivation**  
Modern computer systems often need to handle many tasks concurrently: network connections, large data processing, or communication with external devices. Traditional multithreading can introduce significant overhead and scalability issues on resource-constrained systems.

Asynchronous executors provide a solution: instead of launching each task in a separate thread, the executor manages tasks cooperatively – tasks "yield" the CPU when waiting for resources, e.g., file data, device signals, or database responses. This allows thousands or millions of concurrent operations using a limited number of threads.

**Example Applications**  

- **HTTP servers:** Nginx or Rust's Tokio handle thousands of network connections asynchronously, minimizing memory and CPU usage.  
- **Embedded systems:** Efficient task management on microcontrollers for sensor reading or device control.  
- **Interactive applications:** GUI or mobile apps remain responsive while fetching network data.  
- **Data processing:** Manages many concurrent I/O operations efficiently (e.g., disk-bound analytics).

**Objective**  
Implement a simplified single-threaded asynchronous executor in C (inspired by Rust's Tokio). No threads are created: executor and tasks all run in the main thread.  

You will receive a skeleton project; your task is to implement the key components, resulting in a simple yet functional async library. Examples and tests are in `tests/`.

**Core Concepts**

- **Executor:** Runs tasks on the CPU; contains the main event loop (`executor_run()`) and a task queue.  
- **Mio:** OS communication layer using `epoll` to wait for file descriptors to become readable/writable.  
- **Future:** Represents an async computation, storing:
  - Function pointer `progress` to advance the task.
  - State information for resuming between progress calls.  
  Futures may be extended via structs like `FutureFoo` with `Future` as first field.  
- **Waker:** Callback to notify the executor that a task can continue.

**Task States**

- `COMPLETED` – task finished with a result.  
- `FAILURE` – task ended with error or was cancelled.  
- `PENDING` – task can continue; either queued in executor or waiting on a waker.  

**State Diagram**

executor_spawn COMPLETED / FAILURE
│ ▲
▼ executor calls │
PENDING ───► fut->progress(fut, waker) ──+
(queued) │
▲ │
│ ▼
└─── waker_wake() ◄───────────── PENDING
(waiting) (waker held)


**Execution Flow**

1. Program creates executor (which creates a Mio instance).  
2. Tasks are spawned via `executor_spawn`.  
3. `executor_run` executes tasks until all complete:  
   - If no pending tasks, exit loop.  
   - If no active tasks, call `mio_poll()` to sleep until an event occurs.  
   - For each active task, call `future.progress(future, waker)`, creating a Waker to re-queue if needed.  

**Future `progress()` can:**

- Attempt progress (e.g., read from a descriptor).  
- Return `FAILURE` on error, `COMPLETED` if done, or `PENDING` otherwise.  
- Register interest in I/O via `mio_register()` for later wake-up.  
- Requeue itself with `waker_wake()` to allow other tasks to run.  
- Call `executor_spawn()` to spawn subtasks.  

**Mio Responsibilities**

- `mio_register()` – associate a Waker with a file descriptor and event (EPOLLIN/EPOLLOUT).  
- `mio_poll()` – wait for events and call the corresponding Wakers.

**Futures Combinators**

Implement three combinators:

- **ThenFuture:** runs `fut1`, then `fut2`, passing fut1’s result to fut2.  
- **JoinFuture:** runs `fut1` and `fut2` concurrently; completes when both finish.  
- **SelectFuture:** runs `fut1` and `fut2` concurrently; completes when one succeeds, cancelling the other.

**Formal Requirements**

- Only student-completed files: `executor.c`, `mio.c`, `future_combinators.c`.  
- Provided headers: `executor.h`, `future.h`, `waker.h`, `mio.h`, `future_combinators.h`.  
- Additional headers: `err.h`, `debug.h`.  
- Use **C (gnu11)**; standard C library only; no external libraries.  
- No active or timed waiting; do **not** use `sleep`, `usleep`, `nanosleep`, or `epoll_wait` timeouts.  
- Memory leaks, unclosed file descriptors, or other resource leaks will be tested.  
- Implementations should be reasonably efficient; overhead should not exceed tens of milliseconds.  
- You may reuse code from previous labs (with proper comments and sources).  

**Helpful Commands**

- Build: `mkdir build; cd build; cmake ..`  
- Build tests: `cd build/tests; cmake --build ..`  
- Run all tests: `ctest --output-on-failure`  
- Run individual test: `./<test_name>`  
- Memory debugging: Valgrind (`--track-origins=yes --track-fds=yes`)  
- Address Sanitizer: compile with `-fsanitize=address` (already set in provided CMakeLists.txt).