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
#define SERV1 PIPES_MAX * 2
#define SERV2 PIPES_MAX * 2 + 1
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
	ufds[SERV1].events = 0;
	ufds[SERV2].events = 0;
    } else if (cli_fd == -1) {
	ufds[SERV1].events = POLLIN | POLLPRI;
	ufds[SERV2].events = 0;
    } else {
	ufds[SERV1].events = 0;
	ufds[SERV2].events = POLLIN | POLLPRI;
    }
}

int readReady(int code) {
    return (code & POLLIN) || (code & POLLPRI);
}

void initPipes(int fd1, int fd2) {
    int next = -1, i;
    for (i = 0; i < PIPES_MAX / 2; i++) {
	if (ufds[2 * i].fd == -1) {
	    next = i;
	    break;
	}
    }
    assert(next != -1, "no more empty ufds\n");
    
    ufds[next * 2].fd = fd1;
    ufds[next * 2].events = POLLIN | POLLPRI | POLLRDHUP;
    assert((bufs[next * 2] = buf_new(BUF_SIZE)) != NULL, NULL);
    ufds[next * 2 + 1].fd = fd2;
    ufds[next * 2 + 1].events = POLLIN | POLLPRI | POLLRDHUP;
    assert((bufs[next * 2 + 1] = buf_new(BUF_SIZE)) != NULL, NULL);
    clients++;
}

void deletePipes(int index) {
    if (ufds[index * 2].fd != -1) {
	close(ufds[index * 2].fd);
    }
    if (ufds[index * 2 + 1].fd != -1) {
	close(ufds[index * 2 + 1].fd);
    }
    ufds[index * 2].fd = -1;
    ufds[index * 2 + 1].fd = -1;
    if (bufs[index * 2] != NULL) {
	buf_free(bufs[index * 2]);
    }
    if (bufs[index * 2 + 1] != NULL) {
	buf_free(bufs[index * 2 + 1]);
    }
    clients--;
}

void setEvents(int from, int to) {
    if (buf_size(bufs[to]) == buf_capacity(bufs[to])) {
	ufds[from].events &= ~POLLIN;
	ufds[from].events &= ~POLLPRI;
    } else {
	ufds[from].events |= POLLIN;
	ufds[from].events |= POLLPRI;	
    }
    
    if (buf_size(bufs[to]) == 0) {
	ufds[to].events &= ~POLLOUT;
    } else {
	ufds[to].events |= POLLOUT;
    }
}

void sendPipe(int from, int to) {
    int n = buf_fill(ufds[from].fd, bufs[to], 1);
    setEvents(from, to);
}

void receivePipe(int from, int to) {
    int n = buf_flush(ufds[to].fd, bufs[to], buf_size(bufs[to]));
    setEvents(from, to);
}

void closePipe(int from, int to) {
    ufds[from].fd = -1;
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

    assert((ufds[SERV1].fd = getServSocket(argv[1])) != -1, NULL);
    assert((ufds[SERV2].fd = getServSocket(argv[2])) != -1, NULL);
    ufds[SERV1].events = POLLIN | POLLPRI;
    ufds[SERV2].events = POLLIN | POLLPRI;
    setStateEvent();

    while (1) {
	int n = poll(ufds, PIPES_MAX * 2 + 2, -1);
	if (errno == EINTR) {
	    continue;
	}
	if (cli_fd == -1 && readReady(ufds[SERV1].revents)) {
	    ufds[SERV1].revents = 0;
	    cli_fd = acceptSafe(ufds[SERV1].fd);
	    setStateEvent();
	    //	    pOut("accepted first client\n");
	}
	if (cli_fd != -1 && readReady(ufds[SERV2].revents)) {
	    ufds[SERV2].revents = 0;
	    initPipes(cli_fd, acceptSafe(ufds[SERV2].fd));
	    cli_fd = -1;
	    setStateEvent();
	    //	    pOut("accepted second client\n");
	}
	for (i = 0; i < PIPES_MAX; i++) {
	    if (ufds[i].fd != -1) {
		if (readReady(ufds[i].revents)) {
		    int from = i, to = accompany(i);
		    sendPipe(from, to);
		}
		if (ufds[i].revents & POLLOUT) {
   		    int to = i, from = accompany(i);
		    receivePipe(from, to);
		}
		if (ufds[i].revents & POLLRDHUP) {
   		    int from = i, to = accompany(i);
		    closePipe(from, to);
		}
	    }
	    ufds[i].revents = 0;
	}
    }	
    
    return 0;
}
