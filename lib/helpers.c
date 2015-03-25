#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
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

ssize_t _read_until(int fd, void * buf, size_t count, int(*cmp)(char)) {
    int curr, n;
    for (curr = 0; curr < count; curr++) {
        n = read(fd, buf + curr, 1);
        if (n == -1)
            return -1;
        if (n == 0)
            return curr;
        if (cmp((((char*)buf)[curr])))
            return curr + 1;
    }

    return curr;
}

int cmp_single(char ch) {
    return ch == _delimiter;
}

int cmp_plural(char ch) {
    char *dels = _delimiters;
    while (*dels != '\0') {
        if (*dels == ch)
            return 1;
        dels++;
    }
    return 0;
}

ssize_t read_until(int fd, void * buf, size_t count, char delimiter) {
    _delimiter = delimiter;
    _read_until(fd, buf, count, &cmp_single);
}

ssize_t read_until2(int fd, void * buf, size_t count, char *delimiters) {
    _delimiters = delimiters;
    _read_until(fd, buf, count, &cmp_plural);
}

int spawn(const char * file, char * const argv []) {
    if (fork() != 0) {
        int status;
        wait(&status);
        return status;
    } else {
//        int devNull = open("/dev/null", O_WRONLY);
//        dup2(devNull, STDOUT_FILENO);
//        dup2(devNull, STDERR_FILENO);
        execvp(file, argv);
//        close(devNull);
    }
}
