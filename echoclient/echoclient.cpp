//============================================================================
// Name        : echoclient.cpp
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
#include <sys/signal.h>
#include <wait.h>

using namespace std;

#define SIZE_ 100

void system_error(const char *msg) {
	cout << msg << "\n";
	exit(0);
}

void show_error(const char *msg) {
	cout << msg << "\n";
}

int keyboard_input(char *p, int buffersize) {

	memset(p, 0, sizeof(p));
	if ((!fgets(p, buffersize, stdin))) {
		// input error
		return -1;
	}

	size_t ln = strlen(p) - 1;


	p[ln] = 0;
	return ln;
}

int write_socket(int &sockfd, char *p) {

	int n = write(sockfd, p, strlen(p));
	if (n < 0)
		system_error("ERROR writing to socket");

	return n;

}

void sig_chld(int signo) {
	int status;

	if (wait(&status) == -1) {

		//wait failed
		show_error("Wait failed");
		return;
	}

}

int main(int argc, char *argv[]) {
	struct sockaddr_in server_address;
	int sockfd, n;
	char readbuf[SIZE_], writebuf[SIZE_];
	pid_t pid;

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

	signal(SIGCHLD, sig_chld);

	if ((pid = fork()) == 0) {

		for (;;) {
			if ((n = keyboard_input(writebuf, SIZE_ - 1)) > 0) {
//				cout<<"local:" <<writebuf<<endl;
				write_socket(sockfd, writebuf);
				continue;
			} else {
//				cout<<"1\n";
				shutdown(sockfd,SHUT_WR);
				break;
			}
		}
	} else {
		for (;;) {
			n = read(sockfd, readbuf, SIZE_ - 1);
			if (n > 0) {
				readbuf[n] = 0;

				cout << "echo:>>" << readbuf << endl;
			}
			else{
				cout << "Connection closed" << endl;
				close(sockfd);
				break;
			}
		}
	}
	return EXIT_SUCCESS;
}
