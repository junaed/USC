//============================================================================
// Name        : udpconfclient.cpp
// Author      : Junaed Halim
// Version     :
// Copyright   : 
// Description :
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <vector>

using namespace std;

#define SIZE_ 65536
#define READSIZE_ 100

const string connstring = "CONNECT";
const string removestring = "REMOVE";

struct ip_port_struct {
	uint32_t ipaddress;
	int port;
};

std::vector<ip_port_struct> ip_port_vector;

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
	p[ln] = '\0';
	return ln + 1;
}

int string_to_int(string s) {
	int i;
	std::stringstream(s) >> i;

	return i;
}

int deserialize_struct_vector(const string& istring) {

	if (istring.size() > 1) {
		ip_port_vector.clear();
		istringstream iss(istring);
		string s;
		std::getline(iss, s, ';');
		while (std::getline(iss, s, ';')) {
			ip_port_struct temp;
			temp.ipaddress = ntohl(string_to_int(s));

			std::getline(iss, s, ';');
			temp.port = ntohs(string_to_int(s));

			ip_port_vector.push_back(temp);
		}

		return 1;
	} else
		return 0;
}

void send_to_all(char *p, int sockfd) {
	int k = ip_port_vector.size();
	sockaddr_in client_address;
	for (int i = 0; i < k; i++) {

		client_address.sin_family = AF_INET;
		client_address.sin_addr.s_addr = htonl(ip_port_vector.at(i).ipaddress);
		client_address.sin_port = htons(ip_port_vector.at(i).port);

		sendto(sockfd, p, strlen(p), 0, (struct sockaddr *) &client_address,
				sizeof(client_address));
		memset(&client_address, 0, sizeof(client_address));
	}

}
void insert_into_vector(ip_port_struct a) {

	std::vector<ip_port_struct>::const_iterator i;

	int found = 0;
	for (i = ip_port_vector.begin(); i != ip_port_vector.end(); ++i) {

		std::string p;

		ip_port_struct temp = *i;
		p = temp.ipaddress;

		if (a.ipaddress == temp.ipaddress && temp.port == a.port) {
			found = 1;
			break;
		}
	}

	if (!found) {
		ip_port_vector.push_back(a);
		in_addr t;
		t.s_addr = ntohl(a.ipaddress);
		cout << "user " << inet_ntoa(t) << ":" << a.port
				<< " joined the conference.\n";
	}
}

void remove_from_vector(ip_port_struct a) {

	if (ip_port_vector.size() > 0) {
		std::vector<ip_port_struct>::iterator i;

		for (i = ip_port_vector.begin(); i != ip_port_vector.end(); ++i) {

			std::string p;

			ip_port_struct temp = *i;
			p = temp.ipaddress;

			if (temp.ipaddress == a.ipaddress && temp.port == a.port) {
				i = ip_port_vector.erase(i);
				in_addr t;
				t.s_addr = ntohl(a.ipaddress);
				cout << "user " << inet_ntoa(t) << ":" << a.port
						<< " left the conference.\n";
				break;
			}
		}
	}
}

void process_server_message(char *p) {
	string str(p);

	istringstream iss(str);
	string s;
	std::getline(iss, s, ';');
	ip_port_struct temp;
	if (!s.compare(connstring)) {
		//CONNECT
		std::getline(iss, s, ';');
		temp.ipaddress = ntohl(string_to_int(s));

		std::getline(iss, s, ';');
		temp.port = ntohs(string_to_int(s));
		insert_into_vector(temp);

	} else if (!s.compare(removestring)) {
		//REMOVE
		std::getline(iss, s, ';');
		temp.ipaddress = ntohl(string_to_int(s));

		std::getline(iss, s, ';');
		temp.port = ntohs(string_to_int(s));
		remove_from_vector(temp);
	} else {
		//!!!
	}

}

void connect_to_server(char *p, int sockfd,
		struct sockaddr_in& server_address) {
	sendto(sockfd, connstring.c_str(), connstring.size(), 0,
			(struct sockaddr *) &server_address, sizeof(server_address));

	int n = recvfrom(sockfd, p, SIZE_, 0, NULL, NULL);
	deserialize_struct_vector(p);
}

int main(int argc, char *argv[]) {

	struct sockaddr_in server_address;
	struct sockaddr_in peer_address;
	int sockfd, n, maxfd;
	char readbuf[SIZE_], writebuf[READSIZE_];
	fd_set fdset;

	if (argc != 3) {
		//error
		system_error(
				"Usage: executable_file <remote_server_address> <remote_server_port>");

	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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

	connect_to_server(readbuf, sockfd, server_address);

	FD_ZERO(&fdset);
	socklen_t peerlen = sizeof(peer_address);
	socklen_t len = sizeof(server_address);

	for (;;) {
		memset(readbuf, 0, sizeof(readbuf));
		memset(writebuf, 0, sizeof(writebuf));
		memset(&peer_address, 0, sizeof(peer_address));
		FD_SET(sockfd, &fdset);
		FD_SET(STDIN_FILENO, &fdset);
		maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

		if (select(maxfd + 1, &fdset, NULL, NULL, NULL) == -1) {
			system_error("Error");
		}

		if (FD_ISSET(sockfd,&fdset)) {

			//data on socket
			memset(readbuf, 0, sizeof(readbuf));
			if ((n = recvfrom(sockfd, readbuf, SIZE_, 0,
					(struct sockaddr *) &peer_address, &peerlen)) <= 0) {
				if (n < 0) {
					if (errno == EINTR) {
						continue;
					}
				}
				FD_CLR(sockfd, &fdset);
				break;
			}

			if (memcmp(&server_address, &peer_address, len) == 0) {
				process_server_message(readbuf);
			} else {
				cout << inet_ntoa(peer_address.sin_addr) << ":"
						<< ntohs(peer_address.sin_port) << ">> " << readbuf
						<< endl;
			}
		}
		if (FD_ISSET(STDIN_FILENO, &fdset)) {
			//data on stdin
			n = keyboard_input(writebuf, READSIZE_ - 1);
			writebuf[n] = 0;

			if (n <= 0) {
				break;
			} else {

				send_to_all(writebuf, sockfd);
			}
		}
	}

	sendto(sockfd, removestring.c_str(), strlen(removestring.c_str()), 0,
			(struct sockaddr *) &server_address, sizeof(server_address));

	return EXIT_SUCCESS;
}
