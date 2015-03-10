#include <errno.h>
#include <unistd.h>
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

    while ((n = read_(STDIN_FILENO, buffer, BLOCK_SIZE)) != 0) {
        if (n == -1)
            print_err();

        n = write_(STDOUT_FILENO, buffer, n);
        if (n == -1)
            print_err();
    }

    return 0;
}
