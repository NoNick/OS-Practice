#ifndef IO_HELPERS
#define IO_HELPERS

#include <sys/types.h>

ssize_t read_(int fd, void *buf, size_t count);
ssize_t write_(int fd, const void *buf, size_t count);

#endif
