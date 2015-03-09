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
            return 0;
    }

    return curr;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    int curr, n;
    for (curr = 0; curr < count; curr += n) {
        int n = write(fd, buf + curr, count - curr);
        if (n == -1)
            return -1;
    }

    return curr;
}
