#include <unistd.h>
#include <bufio.h>

const int BUF_SIZE = 4096;

int main() {
    struct buf_t* buf = buf_new(BUF_SIZE);

    int n = 0;
    while (n != -1 && (n = buf_fill(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        n = buf_flush(STDOUT_FILENO, buf, n);
    }

    buf_free(buf);
    return 0;
}
