//============================================================================
// Name        : confserver.cpp
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

#define LISTENQ 5
#define SIZE_ 100
#define RETRY_ALLOWED 3

using namespace std;

void system_error(const char *msg) {
	cout << msg << "\n";
	exit(0);
}

void show_error(const char *msg) {
	cout << msg << "\n";
}

int main(void) {

	int listenfd, n, connfd, maxi, i, j, nready, sockfd;
	int yes = 1;
	struct sockaddr_in serv_addr, client_addr;
	fd_set fdset;
	fd_set temp_fdset;
	int maxfd = -1;
	socklen_t client_len;
	char received[SIZE_];
	int clients[FD_SETSIZE];

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
		system_error("ERROR opening socket");

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		system_error("Error...quitting...");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = 0;

	if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		system_error("ERROR on binding");

	n = listen(listenfd, LISTENQ);
	if (n < 0) {
		//error
		system_error("Listen Error");
	}

	socklen_t len = sizeof(serv_addr);
	if (getsockname(listenfd, (struct sockaddr *) &serv_addr, &len) == -1)
		system_error("");
	else
		printf("port number %d\n", ntohs(serv_addr.sin_port));

	FD_ZERO(&fdset);
	FD_ZERO(&temp_fdset);

	FD_SET(listenfd, &fdset);
	maxfd = listenfd;

	maxi = -1;
	for (i = 0; i < FD_SETSIZE; i++) {
		clients[i] = -1;
	}

	for (;;) {
		temp_fdset = fdset;

		if ((nready = select(maxfd + 1, &temp_fdset, NULL, NULL, NULL)) == -1) {
			//error
			system_error("Error");
		}

		if (FD_ISSET(listenfd,&temp_fdset)) {

			//new connection

			client_len = sizeof(client_addr);
			connfd = accept(listenfd, (struct sockaddr *) &client_addr,
					&client_len);
			if (connfd < 0) {
				show_error("Accept Error!!");
				continue;
			}

			for (i = 0; i < FD_SETSIZE; i++) {
				if (clients[i] < 0) {
					clients[i] = connfd;
					break;
				}
			}

			if (i == FD_SETSIZE) {
				system_error("Too many clients!!");
			}

			cout << "Connected with client\n";
			FD_SET(connfd, &fdset);

			if (i > maxi)
				maxi = i;

//			cout << "New connfd is " << connfd << endl;
			if (connfd > maxfd) {
				maxfd = connfd;
			}

			if (--nready <= 0) {
				continue;
			}

			//					break;
		}

		for (i = 0; i <= maxi; i++) {

			if ((sockfd = clients[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &temp_fdset)) {

				memset(received, 0, sizeof(received));

				if ((n = read(sockfd, received, SIZE_ - 1)) <= 0) {
					//connection closed by client

					close(sockfd);
					FD_CLR(sockfd, &fdset);
					clients[i] = -1;
//						continue;
				} else {
//					cout << received << endl;
					int k;
					for (j = 0; j <= maxi; j++) {
						k = clients[j];
						if (k != listenfd && k != sockfd && k != -1) {
							if (write(k, received, strlen(received)) < 0) {
								perror("send error");
							}
						}
					}
					if (--nready <= 0)
						break;

				}
			}

		}

	}

	for (i = 0; i <= maxi; i++) {
		if (clients[i] != -1)
			close(clients[i]);
	}
	return EXIT_SUCCESS;
}
