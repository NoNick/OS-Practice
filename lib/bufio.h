#ifndef BUFIO
#define BUFIO

#include <sys/types.h>

struct buf_t {
    size_t capacity, size;
    char *data;
};

struct buf_t* buf_new(size_t capacity);
void buf_free(struct buf_t*);
size_t buf_capacity(struct buf_t*);
size_t buf_size(struct buf_t*);
ssize_t buf_fill(int fd, struct buf_t *buf, size_t required);
ssize_t buf_flush(int fd, struct buf_t *buf, size_t required);

#endif
