#include "mio.h"

#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"
#include "executor.h"
#include "waker.h"

#define MAX_EVENTS 64

/* Mio structure – holds the epoll handle and a pointer to the executor */
struct Mio {
    int epoll_fd;       // epoll handle (epoll descriptor)
    Executor* executor; // pointer to the executor
};

/**
 * Creates a new Mio instance.
 */
Mio* mio_create(Executor* executor) {
    Mio* mio = malloc(sizeof(Mio));
    if (!mio)
        return NULL;
    mio->epoll_fd = epoll_create1(0);
    if (mio->epoll_fd < 0) {
        free(mio);
        return NULL;
    }
    mio->executor = executor;
    return mio;
}

/**
 * Destroys a Mio instance – frees resources.
 */
void mio_destroy(Mio* mio) {
    close(mio->epoll_fd);
    free(mio);
}

/**
 * Registers the fd descriptor with the expected events.
 *
 * In this implementation, we store only a pointer to the future in ev.data.ptr,
 * since the executor is already stored in the Mio structure.
 */
int mio_register(Mio* mio, int fd, uint32_t events, Waker waker) {
    debug("Registering (in Mio = %p) fd = %d with", mio, fd);

    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = waker.future;  // We store only a pointer to the future

    int ret = epoll_ctl(mio->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

/**
 * Unregisters the fd descriptor.
 */
 int mio_unregister(Mio* mio, int fd) {
    debug("Unregistering (from Mio = %p) fd = %d\n", mio, fd);

    int ret = epoll_ctl(mio->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

/**
 * Waits (blocking) for events and calls waker_wake for each.
 *
 * For each event from epoll_wait(), retrieve the pointer to the future
 * (stored in ev.data.ptr), locally construct a Waker object (linking
 * the executor with Mio and the future), and call waker_wake().
 */
void mio_poll(Mio* mio) {
    debug("Mio (%p) polling\n", mio);

    struct epoll_event events[MAX_EVENTS];
    int n = epoll_wait(mio->epoll_fd, events, MAX_EVENTS, -1);
    if (n < 0) {
        debug("epoll_wait error\n");
        return;
    }

    for (int i = 0; i < n; i++) {
        Future* future = (Future*)events[i].data.ptr;

        // Create a local Waker object based on the executor
        // (stored in Mio) and the future
        Waker waker;
        waker.executor = mio->executor;
        waker.future = future;
        waker_wake(&waker);
    }
}