//============================================================================
// Name        : udpconfserver.cpp
// Author      : Junaed Halim
// Version     :
// Copyright   : 
// Description :
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
#include <list>
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/ioctl.h>
#include <sstream>
#include <stdint.h>

#define LISTENQ 5
#define SIZE_ 100

#define SERVER_PORT 0

using namespace std;

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
		cout << "new connection from " << inet_ntoa(t) << ":" << a.port << "\n";
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
				cout << "connection closed by " << inet_ntoa(t) << ":" << a.port
						<< "\n";
				break;
			}
		}
	}
}

string serialize_struct_vector(int *k) {

	std::ostringstream outstream;

	*k = ip_port_vector.size();

	if (*k == 0)
		return ";";

	outstream << htonl(*k) << ";";
	for (int i = 0; i < *k; i++) {

		outstream << htonl(ip_port_vector.at(i).ipaddress) << ";"
				<< htons(ip_port_vector.at(i).port) << ";";
	}

	return outstream.str();

}

void notify_new_client(ip_port_struct& t, int sockfd) {

	int k = ip_port_vector.size();
	sockaddr_in client_address;
	for (int i = 0; i < k; i++) {

		client_address.sin_family = AF_INET;
		client_address.sin_addr.s_addr = htonl(ip_port_vector.at(i).ipaddress);
		client_address.sin_port = htons(ip_port_vector.at(i).port);

		std::ostringstream outstream;
		outstream << connstring << ";" << htonl(t.ipaddress) << ";"
				<< htons(t.port);

		sendto(sockfd, outstream.str().c_str(), outstream.str().size(), 0,
				(struct sockaddr *) &client_address, sizeof(client_address));
		memset(&client_address, 0, sizeof(client_address));
	}
}

void notify_removed_client(ip_port_struct& t, int sockfd) {

	int k = ip_port_vector.size();
	sockaddr_in client_address;
	for (int i = 0; i < k; i++) {

		client_address.sin_family = AF_INET;
		client_address.sin_addr.s_addr = htonl(ip_port_vector.at(i).ipaddress);
		client_address.sin_port = htons(ip_port_vector.at(i).port);

		std::ostringstream outstream;
		outstream << removestring << ";" << htonl(t.ipaddress) << ";"
				<< htons(t.port);

		sendto(sockfd, outstream.str().c_str(), outstream.str().size(), 0,
				(struct sockaddr *) &client_address, sizeof(client_address));
		memset(&client_address, 0, sizeof(client_address));
	}
}

void send_list_to_new_client(int sockfd, struct sockaddr_in& client_addr) {
	int t;
	string os = serialize_struct_vector(&t);
	char *a = (char *) malloc((sizeof(char) * (1 + os.size())));

	a[os.size()] = 0;
	memcpy(a, os.c_str(), os.size());
	sendto(sockfd, a, strlen(a), 0, (struct sockaddr *) &client_addr,
			sizeof(client_addr));
	free(a);
}

int main(void) {
	int sockfd, n;

	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;

	char received[SIZE_];

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		system_error("ERROR opening socket");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int)) == -1) {
		system_error("Error...quitting...");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERVER_PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		system_error("ERROR on binding");

	socklen_t len = sizeof(serv_addr);

	if (SERVER_PORT == 0) {

		if (getsockname(sockfd, (struct sockaddr *) &serv_addr, &len) == -1)
			system_error("");
		else
			printf("port number %d\n", ntohs(serv_addr.sin_port));
	}

	socklen_t clilen = sizeof(client_addr);

	for (;;) {

		memset(received, 0, sizeof(received));

		n = recvfrom(sockfd, received, SIZE_, 0,
				(struct sockaddr *) &client_addr, &clilen);

		ip_port_struct client;
		client.ipaddress = ntohl(client_addr.sin_addr.s_addr);
		client.port = ntohs(client_addr.sin_port);

		received[n] = '\0';

		if (!connstring.compare(received)) {
			// new connection

			send_list_to_new_client(sockfd, client_addr);
			notify_new_client(client, sockfd);
			insert_into_vector(client);

		} else if (!removestring.compare(received)) {
			// remove
			remove_from_vector(client);
			notify_removed_client(client, sockfd);

		} else {
			//error
			show_error(received);

		}
	}

	return EXIT_SUCCESS;
}
