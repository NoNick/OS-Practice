#ifndef IO_HELPERS
#define IO_HELPERS

#include <sys/types.h>

ssize_t read_(int fd, void *buf, size_t count);
ssize_t write_(int fd, const void *buf, size_t count);
ssize_t read_until(int fd, void * buf, size_t count, char delimiter);
ssize_t read_until2(int fd, void * buf, size_t count, char *delimiters);
int spawn(const char * file, char * const argv []);

char _delimiter;
char *_delimiters;

#endif
