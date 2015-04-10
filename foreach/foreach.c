#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <helpers.h>
#include <bufio.h>
#include <string.h>

const int BLOCK_SIZE = 4096;
const int ARGS_N = 2048;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // no command to execute
        return 0;
    }

    struct buf_t* buffer = buf_new(BLOCK_SIZE);
    if (buffer->data == NULL) {
        return 1;
    }
    char dest[BLOCK_SIZE];
    char *command = argv[1];
    char *spawn_argv[ARGS_N];
    int i;
    for (i = 0; i < ARGS_N; i++) {
        spawn_argv[i] = NULL;
    }
    // first argument is path to executable
    for (i = 0; i < argc - 1; i++) {
        spawn_argv[i] = argv[i + 1];
    }

    int n;
    do {
        i = argc - 1;
        if ((n = buf_getline(STDIN_FILENO, buffer, dest)) == -1) {
            return -1;
        }
        if (n == 0) {
            return 0;
        }
	else if (n % 2 != 0)
        continue;

        char old_dest[BLOCK_SIZE];
        memcpy(old_dest, dest, strlen(dest));
        char *splitted = strtok(dest, " ");
        while (splitted != NULL) {
            spawn_argv[i++] = splitted;
            i++;
            splitted = strtok(NULL, " ");
        }
        spawn_argv[i] = NULL;

        if (spawn(command, spawn_argv) == 0) {
            write_(STDOUT_FILENO, old_dest, strlen(old_dest));
            write_(STDOUT_FILENO, "\n", 1);
        }
    } while(n > 0);

    for (i = argc - 1; i < ARGS_N; i++) {
        // free is OK with null pointers
        free(spawn_argv[i]);
    }
    buf_free(buffer);

    return 0;
}
