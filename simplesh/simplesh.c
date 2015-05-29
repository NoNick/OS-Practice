#include <helpers.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

const int BUF_SIZE = 4096;
const int PROGRAMS_MAX = 1024;

int parse(char *buf, struct execargs_t **progs) {
    int cnt = 0;
    char *saveptr = NULL;
    char *next = strtok_r(buf, "|", &saveptr);
    if (next == NULL) {
	progs[cnt++] = execargs(buf);
	return 1;
    }
    
    while (next != NULL) {
	progs[cnt++] = execargs(next);
	next = strtok_r(NULL, "|", &saveptr);
    }
    return cnt;
}

void free_args(struct execargs_t **args) {
    int i;
    for (i = 0; i < PROGRAMS_MAX; i++) {
	if (args[i] != NULL) {
	    exec_free(args[i]);
	}
    }
    free(args);
}

void pipize(char *buf) {
    const int size = sizeof (struct execargs*) * PROGRAMS_MAX;
    struct execargs_t** progs = malloc(size);
    memset(progs, 0, size);
    int n = parse(buf, progs);
    runpiped(progs, n);
}

int main() {
    char buf[BUF_SIZE];
    
    while(1) {
	write(STDOUT_FILENO, "$", 1);
	memset(buf, 0, BUF_SIZE);
	read_until(STDIN_FILENO, buf, BUF_SIZE, '\n');
	pipize(buf);
    }
    
    return 0;
}

