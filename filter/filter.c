#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <helpers.h>

const int BLOCK_SIZE = 4096;
const int ARGS_N = 1000;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // no command to execute
        return 0;
    }

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
    char last;
    do {
        i = argc - 1;
        do {
            if (spawn_argv[i] == NULL) {
                spawn_argv[i] = malloc(BLOCK_SIZE);
            }
            n = read_until2(STDIN_FILENO, spawn_argv[i], BLOCK_SIZE, " \n");
            last = spawn_argv[i][n - 1];
            if (last == ' ' || last == '\n') {
                spawn_argv[i][n - 1] = '\0';
            }
            i++;
        } while (last != '\n' && n > 0);
        spawn_argv[i] = NULL;
        // end of stdin, no args passed in current line
        if (i == argc && n <= 0) {
            return 0;
        }

        if (spawn(command, spawn_argv) == 0) {
            for (i = argc - 1; spawn_argv[i] != NULL; i++) {
                write_(STDOUT_FILENO, spawn_argv[i], strlen(spawn_argv[i]));
                write_(STDOUT_FILENO, " ", 1);
            }
            write_(STDOUT_FILENO, "\n", 1);
        }
    } while(n > 0);

    for (i = argc - 1; i < ARGS_N; i++) {
        // free is OK with null pointers
        free(spawn_argv[i]);
    }

    return 0;
}
