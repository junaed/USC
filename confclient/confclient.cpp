//============================================================================
// Name        : confclient.cpp
// Author      : Junaed Halim
// Version     :
// Copyright   : 
// Description : Hello World in C, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

#define SIZE_ 100

void system_error(const char *msg) {
	cout << msg << "\n";
	exit(0);
}

void show_error(const char *msg) {
	cout << msg << "\n";
}

void remove_white_spaces(char** source) {
	char* i = *source;
	char* j = *source;
	while (*j != 0) {
		*i = *j++;
		if (*i != '\n')
			i++;
	}
	*i = 0;
}

int keyboard_input(char *p, int buffersize) {

	memset(p, 0, sizeof(p));
	if ((!fgets(p, buffersize, stdin))) {
		// input error
		return -1;
	}

	remove_white_spaces(&p);

	size_t ln = strlen(p) - 1;
	return ln;
}

int main(int argc, char *argv[]) {
	struct sockaddr_in server_address;
	int sockfd, n, maxfd;
	char readbuf[SIZE_], writebuf[SIZE_];
	fd_set fdset;

	if (argc != 3) {
		//error
		system_error(
				"Usage: executable_file <remote_server_address> <remote_server_port>");

	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		system_error("socket error!!");
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	server_address.sin_port = htons(atoi(argv[2]));

	if ((n = inet_pton(AF_INET, argv[1], &server_address.sin_addr)) <= 0) {
		//error
		if (!isalpha(argv[1][0]))
			system_error("inet_pton error");
	}

	if (connect(sockfd, (struct sockaddr *) &server_address,
			sizeof(server_address)) < 0) {
		//error
		system_error("Connection Error!!");
	} else {
		cout << "Connected with server!!\n";
	}

	FD_ZERO(&fdset);

	for (;;) {

		FD_SET(sockfd, &fdset);
		FD_SET(STDIN_FILENO, &fdset);
		maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

		if (select(maxfd + 1, &fdset, NULL, NULL, NULL) == -1) {
			system_error("Error");
		}

		if (FD_ISSET(sockfd,&fdset)) {

			//data on socket
			memset(readbuf, 0, sizeof(readbuf));
//			cout << "Here i am\n";
			if ((n = read(sockfd, readbuf, SIZE_ - 1)) <= 0) {
				if (n < 0) {
					if (errno == EINTR) {
						continue;
					}
				}
				FD_CLR(sockfd, &fdset);
				break;
//						continue;
			}
			cout << readbuf << endl;
		}
		if (FD_ISSET(STDIN_FILENO, &fdset)) {
			//data on stdin
			n = keyboard_input(writebuf, SIZE_ - 1);

			if (n <= 0) {
				//EOF
//				show_error("EOF!!");
				return EXIT_SUCCESS;
//						continue;
			} else {
//				cout << "send data is: " << writebuf << endl;
				n = write(sockfd, writebuf, strlen(writebuf));
				if (n < 0)
					system_error("ERROR writing to socket");
				else {
//					cout << "Data sent!!\n";
				}
			}
		}

	}
	close(sockfd);

	return EXIT_SUCCESS;
}
