#include <sys/types.h>
#include <unistd.h>
#include "helpers.h"

ssize_t read_(int fd, void *buf, size_t count) {
    int curr, n;
    for (curr = 0; curr < count; curr += n) {
        n = read(fd, buf + curr, count - curr);
        if (n == -1)
            return -1;
        if (n == 0)
            return curr;
    }

    return curr;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    int curr, n;
    for (curr = 0; curr < count; curr += n) {
        n = write(fd, buf + curr, count - curr);
        if (n == -1)
            return -1;
    }

    return curr;
}

ssize_t read_until(int fd, void * buf, size_t count, char delimiter) {
    int curr, n;
    for (curr = 0; curr < count; curr++) {
        n = read(fd, buf + curr, 1);
        if (n == -1)
            return -1;
        if (n == 0)
            return curr;
        if ((((char*)buf)[curr]) == delimiter)
            return curr + 1;
    }

    return curr;
}
