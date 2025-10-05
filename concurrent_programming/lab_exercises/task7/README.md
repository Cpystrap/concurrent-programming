### Task Description

Today's task is to write a simplified equivalent of the `|` operator.  
Using the template `pipeline_skeleton.c`, write a program `pipeline.c` that will connect with a pipeline the arguments it was invoked with, i.e.

./pipeline a1 a2 ... aN

should execute

a1 | a2 | ... | aN


Requirements / notes:

- The solution should use unnamed pipes.
- It must not use the system calls `read` or `write` (except for writing to `stderr`).
- The standard input of the first program and the standard output of the last program should remain unchanged.
- Each process should close all unused file descriptors other than the standard ones.
- To verify this, the solution should call the function `print_open_descriptors()` from `pipeline_utils_skeleton.h` (implementation in `pipeline_utils_skeleton.c`) immediately before each `exec` call and immediately before every exit from `main` (either `exit` or `return`).
- Do not create a large (proportional to the number of arguments) array of descriptors — most of them would have to be closed anyway.
