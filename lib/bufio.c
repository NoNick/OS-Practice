#include <stdlib.h>
#include <sys/types.h>
#include "bufio.h"
#include "helpers.h"

struct buf_t* buf_new(size_t capacity) {
    char *new_data = malloc(capacity);
    if (new_data == NULL) {
        return NULL;
    }

    struct buf_t *result = malloc(sizeof(struct buf_t));
    result->capacity = capacity;
    result->size = 0;
    result->pos = 0;
    result->data = new_data;
    return result;
}

void buf_free(struct buf_t* buf) {
#ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
#endif

    free(buf->data);
    free(buf);
}

size_t buf_capacity(struct buf_t* buf) {
#ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
#endif

    return buf->capacity;
}

size_t buf_size(struct buf_t* buf) {
#ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
#endif

    return buf->size;
}

ssize_t buf_fill(int fd, struct buf_t *buf, size_t required) {
#ifdef DEBUG
    if (buf == NULL || buf->capacity < required) {
        abort();
    }
#endif

    buf->pos = 0;
    size_t result = 0, tmp;
    while (result < required &&
           (tmp = read(fd, buf->data + result, buf->capacity - result)) > 0) {
        result += tmp;
    }
    buf->size = result >= 0 ? result : 0;
    return result;
}

ssize_t buf_flush(int fd, struct buf_t *buf, size_t required) {
#ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
#endif

    return write_(fd, buf->data, required);
}

ssize_t buf_getline(int fd, struct buf_t *buf, char* dest) {
#ifdef DEBUG
    if (buf == NULL || dest == NULL) {
        abort();
    }
#endif
    size_t dest_pos = 0;
    do {
        while (buf->pos != buf->size) {
            dest[dest_pos++] = buf->data[buf->pos++];
            if (buf->data[buf->pos] == '\n') {
                buf->pos++;
                dest[dest_pos] = '\0';
                return dest_pos;
            }

        }
    } while (buf_fill(fd, buf, 1) > 0);

    return dest_pos;
}
