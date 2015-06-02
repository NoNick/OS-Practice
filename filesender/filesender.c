#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <bufio.h>

const int BUF_SIZE = 4096;

void exitOnError(code) {
    if (code == -1) {
	char *msg = strerror(errno);
        write(STDERR_FILENO, msg, strlen(msg));
        _exit(1); 
    }
}

void setHints(struct addrinfo *info) {
    memset(info, 0, sizeof(struct addrinfo));
    info->ai_family = AF_INET;
    info->ai_socktype = SOCK_STREAM;
    info->ai_protocol = IPPROTO_TCP;
}

/*
 * Finds address which can be binded to socket so it can be listened on.
 * Returns new socket's descriptor or -1 if failed
 */
int setUpSocket(struct addrinfo *addrs) {
    int sfd;
    struct addrinfo *ptr;
    for (ptr = addrs; ptr != NULL; ptr = ptr->ai_next) {
        sfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sfd == -1) {
            continue;
	}

	int enable = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
	    close(sfd);
	    continue;
	}
	
        if (bind(sfd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
	    write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
	    close(sfd);
	    continue;
	}

	if (listen(sfd, SOMAXCONN) == -1) {
	    close(sfd);
	} else {
	    return sfd;
	}
    }

    return -1;
}

int sendFile(int fd, char *filename) {
    struct buf_t* buf = buf_new(BUF_SIZE);
    int file;
    exitOnError(file = open(filename));

    int n = 0;    
    while (n != -1 && (n = buf_fill(file, buf, BUF_SIZE)) > 0) {
        n = buf_flush(fd, buf, n);
    }

    buf_free(buf);
    return n;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
	write(STDOUT_FILENO, "Usage: port filename\n", 21);
	return 0;
    }
    
    struct addrinfo hints, *localhosts;
    int serverSocket;
    setHints(&hints);
    exitOnError(getaddrinfo("localhost", argv[1], &hints, &localhosts));
    exitOnError(serverSocket = setUpSocket(localhosts));
    freeaddrinfo(localhosts);

    while (1) {
	struct sockaddr_in client;
	socklen_t sz = sizeof(client);
	int fd = accept(serverSocket, (struct sockaddr*)&client, &sz);
	if (fd != -1) {
	    if (fork() == 0) {
		int ret = sendFile(fd, argv[2]);
		close(fd);
		_exit(ret == -1);
	    } else {
	      close(fd);
	    }
	}
    }
    
    return 0;
}
