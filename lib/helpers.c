#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "helpers.h"


const int EXECV_MAX = 1024;
int *proc;
int proc_cnt;

struct execargs_t* execargs(char *str) {
    struct execargs_t *result = malloc(sizeof(struct execargs_t));
    result->stdin = STDIN_FILENO;
    result->stdout = STDOUT_FILENO;
    result->stderr = STDERR_FILENO;
    char *ptr;

    result->name = strtok_r(str, " \t\n", &ptr);
    if (result->name == NULL) {
	result->name = str;
    }

    int cnt = 0;
    result->argv = malloc(sizeof(char*) * EXECV_MAX);
    memset(result->argv, 0, sizeof(char*) * EXECV_MAX);
    result->argv[cnt++] = result->name;
    char *next = strtok_r(NULL, " \t\n", &ptr);
    while (next != NULL) {
	result->argv[cnt++] = next;
	next = strtok_r(NULL, " \t\n", &ptr);
    }
    
    return result;
}

void killall() {
    if (proc != NULL) {
	int i;
	for (i = 0; i < proc_cnt; i++) {
	    kill(proc[i], SIGINT);
	    waitpid(proc[i], NULL, 0);
	}
    }
}
	    
void exec_free(struct execargs_t *args) {
    free(args->argv);
}

int exec(struct execargs_t* args) {
    if (args->stdin != STDIN_FILENO &&
	dup2(args->stdin, STDIN_FILENO) == -1) {
	return -4;
    }
    if (args->stdout != STDOUT_FILENO &&
	dup2(args->stdout, STDOUT_FILENO) == -1) {
	return -3;
    }
    if (args->stderr != STDERR_FILENO &&
	dup2(args->stderr, STDERR_FILENO) == -1) {
	return -2;
	}
    
    if (execvp(args->name, args->argv) == -1) {
	return -1;
    }
    return 0;
}

int runpiped(struct execargs_t **programs, size_t n) {
    int fd[2], end_pipe, i, ret, pids[n];
    proc = pids;
    proc_cnt = 0;
    struct execargs_t *arg = programs[0];

    for (i = 0; i < n; i++, arg = programs[i]) {       
	if (i > 0) {
	    arg->stdin = end_pipe;
	}
	if (i < n - 1) {
	    if (pipe(fd) == -1) {
		return -1;
	    }
	    arg->stdout = fd[1];
	}
	
	if ((ret = fork()) == 0) {
	    if (i < n - 1) {
	        close(fd[0]);
	    }
	    if (exec(arg) < 0) {
		proc_cnt--;
		_exit(1);
	    }
	    _exit(0);
	}
	if (ret == -1) {
	    return -1;
	}
	
	pids[proc_cnt++] = ret;
	if (i > 0) {
	    close(end_pipe);
	}
	if (i < n - 1) {
	    close(fd[1]);
	}
	end_pipe = fd[0];
    }

    for (i = 0; i < proc_cnt; i++) {
	waitpid(pids[i], NULL, 0);
    }
    proc_cnt = 0;

    return 0;
}

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
        if (n == -1) {
            return -1;
	}
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
    int ret;
    if ((ret = _read_until(fd, buf, count, &cmp_single)) == -1) {
	return 0;
    }
    return ret;
}

ssize_t read_until2(int fd, void * buf, size_t count, char *delimiters) {
    _delimiters = delimiters;
    return _read_until(fd, buf, count, &cmp_plural);
}

int spawn(const char * file, char * const argv []) {
    if (fork() != 0) {
        int status;
        wait(&status);
        return status;
    } else {
        int devNull = open("/dev/null", O_WRONLY);
        dup2(devNull, STDOUT_FILENO);
        dup2(devNull, STDERR_FILENO);
        execvp(file, argv);
        close(devNull);
    }
}
