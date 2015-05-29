#include <helpers.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <stdio.h>

#define BUF_SIZE 4096
const int PROGRAMS_MAX = 100;

int parse(char *buf, struct execargs_t **progs) {
    int cnt = 0;
    char *saveptr;
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
    free_args(progs);
}

int sigint = 0;

void handler(int sig) {
    if (sig == SIGINT || sig == SIGPIPE) {
	sigint = 1;
	killall();
    }
}

char buf[BUF_SIZE];
int pos = 0, size = 0;
char* readln(int fd) {    
    int n;
    if (pos == size) {
	if ((n = read(fd, buf + size, BUF_SIZE - size)) <= 0) {
	    return NULL;
	}
	size += n;
    }

    char *result = malloc(BUF_SIZE);
    int i;
    for (i = pos; i < size; i++) {
	if (buf[pos] != '\n') {
	    result[i - pos] = buf[i];
	} else {
	    i++;
	    break;
	}
    }
    result[i] = '\0';
    pos = i;
    memmove(buf, buf + pos, size - pos);
    size -= pos;
    pos = 0;
    return result;
}

int main() {
    struct sigaction new_act, old_act;
    new_act.sa_handler = handler;
    new_act.sa_flags = 0;
    sigemptyset(&new_act.sa_mask);
    sigaction(SIGINT, &new_act, &old_act);
    sigaction(SIGPIPE, &new_act, &old_act);

    char *line = (char*)1;
    while(line != NULL || sigint) {
	sigint = 0;
	write(STDOUT_FILENO, "$", 1);
	line = readln(STDIN_FILENO);
	if (line == NULL & sigint == 0) {
	    break;
	}
	if (line != NULL) {
	    pipize(line);
	    free(line);
	}
    }
    
    return 0;
}

