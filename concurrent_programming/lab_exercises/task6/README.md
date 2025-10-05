### Task Description

In the file `tree_skeleton.c` you will find an example in which the parent process creates `N_PROC` child processes.  
Write a program `line.c` that should create a line consisting of `N_PROC` processes: each process (except the last) is the parent of the next process.

Each child should perform `exec`, launching the `line` program again, passing a decremented number of processes as an argument.

Each process should wait for the termination of its child, if it has one.

All processes except the first should print to standard output their identifier and the identifier of their parent.

For error handling of system calls use the macro `ASSERT_SYS_OK()`.

To pass the number as an argument (of type `char*`) you can use conversion with `snprintf`:

```c
char buffer[BUFFER_SIZE];
int ret = snprintf(buffer, sizeof buffer, "%d", n_children);
if (ret < 0 || ret >= (int)sizeof(buffer))
    fatal("snprintf failed");

```
To read the program argument as a number you can use: atoi(argv[1]) (after checking argc in int main(int argc, char* argv[])).