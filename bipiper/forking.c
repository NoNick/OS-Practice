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

int getServSocket(char *port) {
    struct addrinfo hints, *localhosts;
    int serverSocket;
    setHints(&hints);
    exitOnError(getaddrinfo("localhost", port, &hints, &localhosts));
    exitOnError(serverSocket = setUpSocket(localhosts));
    freeaddrinfo(localhosts);
    return serverSocket;
}

int acceptSafe(int sock) {
    struct sockaddr_in client;
    socklen_t sz = sizeof(client);
    int fd;
    if ((fd = accept(sock, (struct sockaddr*)&client, &sz)) == -1) {
	char *msg = "can't accept socket";
	write(STDERR_FILENO, msg, strlen(msg));
    }
    return fd;
}

void redirect(int from, int to) {
    struct buf_t* buf = buf_new(BUF_SIZE);

    int n = 0;    
    while (n != -1 && (n = buf_fill(from, buf, 1)) > 0) {
        n = buf_flush(to, buf, n);
    }

    buf_free(buf);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
	write(STDOUT_FILENO, "Usage: port port\n", 21);
	return 0;
    }
    
    int s1 = getServSocket(argv[1]);
    int s2 = getServSocket(argv[2]);

    while (1) {
	int fd1, fd2;
	while ((fd1 = acceptSafe(s1)) == -1);
        while ((fd2 = acceptSafe(s2)) == -1);

	if (fork() == 0) {
	    redirect(fd1, fd2);
	    close(fd1);
	    close(fd2);
	    _exit(0);
	}	
	if (fork() == 0) {
	    redirect(fd2, fd1);
	    close(fd1);
	    close(fd2);
	    _exit(0);
	}
	
	close(fd1);
	close(fd2);
    }
    
    return 0;
}
