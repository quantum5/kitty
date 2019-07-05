/*
 * Copyright (C) 2019 Kovid Goyal <kovid at kovidgoyal.net>
 *
 * Distributed under terms of the GPL3 license.
 */

#pragma once

#include "data-types.h"
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    int wakeup_fds[2];
    int signal_fds[2];
    int wakeup_read_fd;
    int signal_read_fd;
} LoopData;
typedef void(*handle_signal_func)(int, void *data);

bool init_loop_data(LoopData *ld);
void free_loop_data(LoopData *ld);
void wakeup_loop(LoopData *ld, bool in_signal_handler);
bool install_signal_handlers(LoopData *ld);
void read_signals(int fd, handle_signal_func callback, void *data);

static inline bool
self_pipe(int fds[2], bool nonblock) {
#ifdef __APPLE__
    int flags;
    flags = pipe(fds);
    if (flags != 0) return false;
    for (int i = 0; i < 2; i++) {
        flags = fcntl(fds[i], F_GETFD);
        if (flags == -1) {  return false; }
        if (fcntl(fds[i], F_SETFD, flags | FD_CLOEXEC) == -1) { return false; }
        if (nonblock) {
            flags = fcntl(fds[i], F_GETFL);
            if (flags == -1) { return false; }
            if (fcntl(fds[i], F_SETFL, flags | O_NONBLOCK) == -1) { return false; }
        }
    }
    return true;
#else
    int flags = O_CLOEXEC;
    if (nonblock) flags |= O_NONBLOCK;
    return pipe2(fds, flags) == 0;
#endif
}