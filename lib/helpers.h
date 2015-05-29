#ifndef IO_HELPERS
#define IO_HELPERS

#include <sys/types.h>

struct execargs_t {
    char **argv, *name;
    int stdin, stdout, stderr;
};

struct execargs_t* execargs(char*);
void exec_free(struct execargs_t*);
int exec(struct execargs_t*);
int runpiped(struct execargs_t**, size_t);
 
ssize_t read_(int, void*, size_t);
ssize_t write_(int, const void*, size_t);
ssize_t read_until(int, void*, size_t, char);
ssize_t read_until2(int, void*, size_t, char*);
int spawn(const char*, char * const *);

char _delimiter;
char *_delimiters;

#endif
