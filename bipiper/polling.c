#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <bufio.h>
#include <poll.h>

#define BUF_SIZE 4096
#define PIPES_MAX 127
struct pollfd ufds[PIPES_MAX * 2 + 2];
// bufs[i] - data pending to be written to ufds[i].fd
struct buf_t *bufs[PIPES_MAX * 2];
int clients = 0, cli_fd = -1;

void pError(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}

void pOut(const char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

void assert(int shouldntBeFalse, const char *msg) {
    if (!shouldntBeFalse) {
	if (msg == NULL) {
	    msg = strerror(errno);
	}
	pError(msg);
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
	    pError(strerror(errno));
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
    assert(getaddrinfo("localhost", port, &hints, &localhosts) != -1, NULL);
    assert((serverSocket = setUpSocket(localhosts)) != -1, NULL);
    freeaddrinfo(localhosts);
    return serverSocket;
}

int acceptSafe(int sock) {
    struct sockaddr_in client;
    socklen_t sz = sizeof(client);
    int fd;
    if ((fd = accept(sock, (struct sockaddr*)&client, &sz)) == -1) {
	pError("can't accept socket");
    }
    return fd;
}

void setStateEvent() {
    if (clients >= PIPES_MAX) {
	ufds[0].events = 0;
	ufds[1].events = 0;
    } else if (cli_fd == -1) {
	ufds[0].events = POLLIN | POLLPRI;
	ufds[1].events = 0;
    } else {
	ufds[0].events = 0;
	ufds[1].events = POLLIN | POLLPRI;
    }
}

int readReady(int code) {
    return (code & POLLIN) || (code & POLLPRI);
}

void initPipes(int fd1, int fd2) {
    ufds[clients * 2 + 2].fd = fd1;
    ufds[clients * 2 + 2].events = POLLIN | POLLPRI | POLLRDHUP;
    assert((bufs[clients * 2] = buf_new(BUF_SIZE)) != NULL, NULL);
    ufds[clients * 2 + 3].fd = fd2;
    ufds[clients * 2 + 3].events = POLLIN | POLLPRI | POLLRDHUP;
    assert((bufs[clients * 2 + 1] = buf_new(BUF_SIZE)) != NULL, NULL);
    clients++;
}

void deletePipes(int index) {
    int i;
    for (i = index * 2; i < clients * 2; i++) {
	ufds[i].fd = ufds[i + 2].fd;
	ufds[i].events = ufds[i + 2].events;
	ufds[i].revents = ufds[i + 2].revents;
	bufs[i - 2] = bufs[i];
    }
    ufds[clients * 2].fd = -1;
    ufds[clients * 2 + 1].fd = -1;
    bufs[clients * 2 - 2] = NULL;
    bufs[clients * 2 - 1] = NULL;
    clients--;
}

void setEvents(int from, int to) {
    if (buf_size(bufs[to - 2]) == buf_capacity(bufs[to - 2])) {
	ufds[from].events &= ~POLLIN;
	ufds[from].events &= ~POLLPRI;
    } else {
	ufds[from].events |= POLLIN;
	ufds[from].events |= POLLPRI;	
    }
    
    if (buf_size(bufs[to - 2]) == 0) {
	ufds[to].events &= ~POLLOUT;
    } else {
	ufds[to].events |= POLLOUT;
    }
}

void sendPipe(int from, int to) {
    int n = buf_fill(ufds[from].fd, bufs[to - 2], 1);
    setEvents(from, to);
}

void receivePipe(int from, int to) {
    int n = buf_flush(ufds[to].fd, bufs[to - 2], buf_size(bufs[to - 2]));
    setEvents(from, to);
}

void closePipe(int from, int to) {
    close(ufds[from].fd);
    ufds[from].fd = -1;
    if (bufs[from - 2] != NULL) {
	buf_free(bufs[from - 2]);
	bufs[from - 2] = NULL;
    }
    
    if (ufds[to].fd == -1) {
	deletePipes(from / 2);
    } else {
	shutdown(ufds[to].fd, SHUT_RD);
    }
}

int accompany(int i) {
    if (i % 2) {
	return i - 1;
    } else {
	return i + 1;
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
	write(STDOUT_FILENO, "Usage: port port\n", 21);
	return 0;
    }

    int i;
    for (i = 0; i < PIPES_MAX; i++) {
	ufds[i].fd = -1;
    }

    assert((ufds[0].fd = getServSocket(argv[1])) != -1, NULL);
    assert((ufds[1].fd = getServSocket(argv[2])) != -1, NULL);
    ufds[0].events = POLLIN | POLLPRI;
    ufds[1].events = POLLIN | POLLPRI;
    setStateEvent();

    while (1) {
	int n = poll(ufds, clients * 2 + 2, -1);
	if (errno == EINTR) {
	    continue;
	}
	if (cli_fd == -1 && readReady(ufds[0].revents)) {
	    ufds[0].revents = 0;
	    cli_fd = acceptSafe(ufds[0].fd);
	    setStateEvent();
	    //	    pOut("accepted first client\n");
	}
	if (cli_fd != -1 && readReady(ufds[1].revents)) {
	    ufds[1].revents = 0;
	    initPipes(cli_fd, acceptSafe(ufds[1].fd));
	    cli_fd = -1;
	    setStateEvent();
	    //	    pOut("accepted second client\n");
	}
	for (i = 2; i < clients * 2 + 2; i++) {
	    if (ufds[i].fd != -1) {
		if (ufds[i].revents & POLLRDHUP) {
   		    int from = i, to = accompany(i);
		    closePipe(from, to);
		    continue;
		}
		if (readReady(ufds[i].revents)) {
		    int from = i, to = accompany(i);
		    sendPipe(from, to);
		}
		if (ufds[i].revents & POLLOUT) {
   		    int to = i, from = accompany(i);
		    receivePipe(from, to);
		}
	    }
	    ufds[i].revents = 0;
	}
    }	
    
    return 0;
}
