#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <helpers.h>

const size_t BLOCK_SIZE = 4096;

void print_err() {
    char *msg = strerror(errno);
    write_(STDERR_FILENO, msg, strlen(msg));
}

int main() {
    char buffer[BLOCK_SIZE];
    int n;

    char out[5];
    while ((n = read_until(STDIN_FILENO, buffer, BLOCK_SIZE, ' ')) != 0) {
        if (n == -1) {
            print_err();
        }

        int out_n = sprintf(out, "%d", n - (buffer[n - 1] == ' '));
        out[out_n] = ' ';
        n = write_(STDOUT_FILENO, out, out_n + 1);
        if (n == -1) {
            print_err();
        }
    }

    return 0;
}
