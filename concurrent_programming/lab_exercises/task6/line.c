#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "err.h"

#define BUFFER_SIZE 32

int main(int argc, char* argv[])
{
    int n_children = 5;

    if (argc > 2)
        fatal("Expected zero or one arguments, got: %d.", argc - 1);
    if (argc == 2)
        n_children = atoi(argv[1]);

    printf("My pid is %d, my parent's pid is %d\n", getpid(), getppid());

    if (n_children > 1) {
        pid_t pid;
        ASSERT_SYS_OK(pid = fork());

        if (pid == 0) { // Proces potomny
            char buffer[BUFFER_SIZE];
            int ret = snprintf(buffer, sizeof buffer, "%d", n_children - 1);
            if (ret < 0 || ret >= (int)sizeof(buffer))
                fatal("snprintf failed");

            // Uruchomienie programu od nowa z pomniejszonym licznikiem
            ASSERT_SYS_OK(execl("./line", "./line", buffer, NULL));
        }

        // Proces rodzic czeka na potomka
        ASSERT_SYS_OK(wait(NULL));
    }

    printf("My pid is %d, exiting.\n", getpid());
    return 0;
}
