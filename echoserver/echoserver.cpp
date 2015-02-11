//============================================================================
// Name        : echoserver.cpp
// Author      : Junaed Halim
// Version     :
// Copyright   : 
// Description : Hello World in C, Ansi-style
//============================================================================

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <tr1/unordered_map>
#include <sstream>
#include <fstream>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>

#define LISTENQ 5
#define MAXLINE 100

using namespace std;
int nchildren = 0;
pid_t *pids;
int lock_fd = -1;
struct flock lock_it, unlock_it;

void system_error(const char *msg) {
	cout << msg << "\n";
	exit(0);
}

void show_error(const char *msg) {
	cout << msg << "\n";
}

void my_lock_init(const char *pathname) {
	char lock_file[1024];

	strncpy(lock_file, pathname, sizeof(lock_file));
	lock_fd = mkstemp(lock_file);

	unlink(lock_file);

	lock_it.l_type = F_WRLCK;
	lock_it.l_whence = SEEK_SET;
	lock_it.l_start = 0;
	lock_it.l_len = 0;

	unlock_it.l_type = F_UNLCK;
	unlock_it.l_whence = SEEK_SET;
	unlock_it.l_start = 0;
	unlock_it.l_len = 0;

}

void my_lock_wait() {
	int rc;

	while ((rc = fcntl(lock_fd, F_SETLKW, &lock_it)) < 0) {
		if (errno == EINTR)
			continue;
		else
			system_error("fcntl error for my_lock_wait");
	}

}

void my_lock_release() {
	if (fcntl(lock_fd, F_SETLKW, &unlock_it) < 0) {
		system_error("fcntl error for my_lock_release");
	}
}

void sig_int(int signo) {
	int i;
	for (i = 0; i < nchildren; i++) {
		kill(pids[i], SIGTERM);
	}

	while (wait(NULL) > 0) {
	}

	if (errno != ECHILD) {
		show_error("Wait error");
	}

	exit(0);
}

void child_main(int i, int listenfd, int addrlen) {
	int connfd;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	ssize_t n;
	char buf[MAXLINE];
	printf("child %ld starting\n", (long) getpid());
	for (;;) {
		memset(&cliaddr, 0, sizeof(cliaddr));
		clilen = addrlen;
		my_lock_wait();
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
		my_lock_release();
		//process the request
		cout << "connected to :" << inet_ntoa(cliaddr.sin_addr) << ":"
				<< ntohs(cliaddr.sin_port) << endl;
		while (1) {
			n = read(connfd, buf, MAXLINE);
			if (n < 0 && errno == EINTR) {
				continue;
			} else if (n > 0) {
				buf[n] = 0;
				cout << buf << endl;
				n = write(connfd, buf, n);
			} else {
				cout << "Client left\n";
				break;
//				system_error("echo: read error");
			}
		}
		close(connfd);
	}

}

pid_t child_make(int i, int listenfd, int addrlen) {
	pid_t pid;
	if ((pid = fork()) > 0)
		return pid;

	child_main(i, listenfd, addrlen);

	return pid;

}

int main(int argc, char *argv[]) {

	int listenfd, i, n;
	socklen_t addrlen;
	struct sockaddr_in serv_addr;

	if (argc != 3) {
		//error
		system_error("Usage: executable_file <port> <children>");
	}

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
		system_error("ERROR opening socket");

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(atoi(argv[1]));

	int yes = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		system_error("Error...quitting...");
	}

	if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		system_error("ERROR on binding");

	n = listen(listenfd, LISTENQ);
	if (n < 0) {
		//error
		system_error("Listen Error");
	}

	nchildren = atoi(argv[2]);
	pids = (pid_t *) calloc(nchildren, sizeof(pid_t));

	addrlen = sizeof(serv_addr);

	my_lock_init("/tmp/echoserv.XXXXXX");

	for (i = 0; i < nchildren; i++) {
		pids[i] = child_make(i, listenfd, addrlen);
	}

	signal(SIGINT, sig_int);

	for (;;)
		pause();

	return EXIT_SUCCESS;
}
