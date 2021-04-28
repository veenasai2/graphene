#define _GNU_SOURCE
#include <errno.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(void) {
    int efd = epoll_create1(0);
    if (efd < 0) {
        err(1, "epoll_create1");
    }

    int p[2];
    if (pipe2(p, O_NONBLOCK) < 0) {
        err(1, "pipe2");
    }

    struct epoll_event event = {
        .events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLET,
        .data.fd = p[0],
    };
    if (epoll_ctl(efd, EPOLL_CTL_ADD, p[0], &event) < 0) {
        err(1, "EPOLL_CTL_ADD");
    }

    if (write(p[1], "", 1) != 1) {
        err(1, "write");
    }
    if (write(p[1], "", 1) != 1) {
        err(1, "write");
    }

    memset(&event, '\0', sizeof(event));
    if (epoll_wait(efd, &event, 1, 1000) != 1) {
        err(1, "epoll_wait");
    }

    if (event.data.fd != p[0]) {
        errx(1, "epoll invalid data: %d", event.data.fd);
    }
    if (event.events != EPOLLIN) {
        errx(1, "epoll invalid events: 0x%x", event.events);
    }

    char c;
    if (read(p[0], &c, 1) != 1) {
        err(1, "read");
    }

    memset(&event, '\0', sizeof(event));
    int ret = epoll_wait(efd, &event, 1, 1000);

    if (ret < 0) {
        err(1, "epoll_wait");
    } else if (ret != 0) {
        errx(1, "EPOLLET reported 2 times");
    }

    puts("TEST OK");
    return 0;
}
