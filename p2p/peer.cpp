//============================================================================
// Name        : p2p.cpp
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
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <vector>
#include <fstream>
#include <tr1/unordered_map>
#include <assert.h>

#define SIZE_ 100
#define LISTENQ 5
#define MAX_PEER_SIZE 100

using namespace std;

typedef unsigned int _uint;
typedef tr1::unordered_map<string, _uint> string_int_map;

string_int_map simap;

typedef struct c_packet
{
//	_uint ipaddress;
	int port;
	char *hostname;
	_uint seq;
	char *message;

} c_packet;

void system_error(const char *msg)
{
	cout << msg << "\n";
	exit(0);
}

void show_error(const char *msg)
{
	cout << msg << "\n";
}

int keyboard_input(char *p, int buffersize)
{

	memset(p, 0, sizeof(p));
	if ((!fgets(p, buffersize, stdin)))
	{
		// input error
		return -1;
	}

	size_t ln = strlen(p);
	p[ln - 1] = '\0';
	return ln;
}

int insert_into_peer_seq_map(string hostname, int port, _uint seq)
{

	string ip_port;
	ostringstream outstream;
	outstream << hostname << ":" << port;
//	cout << hostname << ":" << port << endl;
	ip_port = outstream.str();

	string_int_map::const_iterator iter = simap.find(ip_port);
	if (iter == simap.end())
	{
		//insert
		simap[ip_port] = seq;
//		cout << "NO ENTRY\n";
		return 2;
	}
	else
	{
		//update
		if (iter->second < seq)
		{
//			cout << "UPDATE ENTRY\n";
			simap[ip_port] = seq;
			return 1;
		}
		else
		{
//			cout << "DUP ENTRY\n";
			return 0;
		}

	}
	return -1;
}

int connect_to_peers(int limit, const char* filepath, int clients[],
		unsigned int localport)
{

	int i = 0;
	int sockfd, n;
	string line;
	int bport = 0;

	ifstream myfile(filepath);

	if (myfile.is_open())
	{
		while (i <= limit && myfile.good())
		{

			getline(myfile, line);

			std::istringstream iss(line);
			string token;

			int count = 0;
			string ip = "", port = "";

			while (std::getline(iss, token, ','))
			{

				if (count == 0)
				{
					ip = token;
					count++;
				}
				else
				{
					port = token;

				}
			}

			if (ip.size() > 1 && port.size() > 1)
			{
				struct sockaddr_in peer_address; //self_address;
				memset(&peer_address, 0, sizeof(peer_address));

				peer_address.sin_family = AF_INET;
				peer_address.sin_port = htons(atoi(port.c_str()));

				if ((n = inet_pton(AF_INET, ip.c_str(), &peer_address.sin_addr))
						<= 0)
				{

					system_error("inet_pton error");
				}

				if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				{
					system_error("Socket Error!!");
				}
				int yes = 1;
				if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
						sizeof(int)) == -1)
				{
					system_error("Error...quitting...");
				}

				struct sockaddr_in serv_addr;
				memset(&serv_addr, 0, sizeof(serv_addr));

				serv_addr.sin_family = AF_INET;
				serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				serv_addr.sin_port = htons(bport);
				if (bind(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr)) < 0)
					system_error("ERROR on binding");
				socklen_t len = sizeof(serv_addr);
				if (getsockname(sockfd, (struct sockaddr *) &serv_addr, &len)
						== -1)
					system_error("");
				else
				{
//					printf("port number %d\n", ntohs(serv_addr.sin_port));
					bport = ntohs(serv_addr.sin_port);
				}

				if (connect(sockfd, (struct sockaddr *) &peer_address,
						sizeof(peer_address)) < 0)
				{
					//error
//				system_error("Connection Error!!");
					stringstream sstream;
					sstream << "Failed to connect to " << ip << ":" << port;
					show_error(sstream.str().c_str());
				}
				else
				{
					//cout << "Connected with server!!\n";
					clients[i] = sockfd;
					i++;

				}
			}

		}
	}

	myfile.close();
	return i;
}

string serialize_message(c_packet* message)
{

	std::ostringstream outstream;

	outstream << message->hostname << ";";
	outstream << htons(message->port) << ";";
	outstream << message->message << ";";
	outstream << htonl(message->seq) << ";";
//	outstream << htonl(message->ipaddress) << ";";

	return outstream.str();

}

void send_to_all(string sendmsg, int clients[], int size_of_clients)
{

	int i;
//	cout<<sendmsg<<endl;
	for (i = 0; i < size_of_clients; i++)
	{
		if (clients[i] != -1)
		{
			if (write(clients[i], sendmsg.c_str(), strlen(sendmsg.c_str())) < 0)
			{
				show_error("Write Error");
			}
			else
			{
//				cout << "Message sent successfully" << endl;
			}

		}
	}

}

int string_to_int(string s)
{
	int i;
	std::stringstream(s) >> i;

	return i;
}

int handle_incoming_message(const string& istring, int clients[],
		int size_of_clients, string ip)
{

	string s;

	stringstream stream(istring);
	int i = 0;
	string hostname;
	int port = 0;
	_uint seq = 0;
//	_uint ipad = 0;
	string st;
	while (getline(stream, s, ';'))
	{
		i++;
		switch (i)
		{
		case 1:
			hostname = s;
			break;
		case 2:
			port = ntohs(string_to_int(s));
			break;
		case 3:
			st = s;
			break;
		case 4:
			seq = ntohl(string_to_int(s));
			break;
		case 5:
//			ipad = ntohl(string_to_int(s));
//			if (ipad != 1)
//			{
//				in_addr t;
//				t.s_addr = ipad;
//				ip = inet_ntoa(t);
//			}
//			else
//			{
//				struct in_addr addr;
//				int tmp = inet_aton(ip.c_str(),&addr);
//				assert(tmp != 0);
//				ipad = addr.s_addr;
//			}
			break;
		default:
			break;
		}

	}

	if (insert_into_peer_seq_map(hostname, port, seq) > 0)
	{
		cout << hostname << ":" << port << ">>" << st << endl;

		std::ostringstream outstream;

		outstream << hostname << ";";
		outstream << htons(port) << ";";
		outstream << st << ";";
		outstream << htonl(seq) << ";";
//		outstream << htonl(ipad) << ";";

		send_to_all(outstream.str(), clients, size_of_clients);
		return 1;
	}
	else
	{
		cout << "Discarding duplicate packet" << endl;
		return 0;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int listenfd, n, connfd, maxi, i, nready, sockfd, limit;
	int localport;
	int yes = 1;
	struct sockaddr_in serv_addr, client_addr;
	fd_set fdset;
	fd_set temp_fdset;
	int maxfd = -1;
	socklen_t client_len;
	char received[SIZE_], writebuf[SIZE_];
	_uint sequence = 1;
	char hostnm[256];
	int clients[FD_SETSIZE];

	if (argc != 4)
	{
		//error
		system_error(
				"Usage: executable_file <port_number> <max_peer_number> <path_to_peer_file>");

	}

	limit = atoi(argv[2]);

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
		system_error("ERROR opening socket");

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		system_error("Error...quitting...");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	localport = atoi(argv[1]);
	serv_addr.sin_port = htons(localport);

	if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		system_error("ERROR on binding");

	n = listen(listenfd, LISTENQ);
	if (n < 0)
	{
		//error
		system_error("Listen Error");
	}

	gethostname(hostnm, sizeof(hostnm));

	for (i = 0; i < FD_SETSIZE; i++)
	{
		clients[i] = -1;
	}

	int no_of_clients = connect_to_peers(limit, argv[3], clients, localport);
	cout << "Connected to " << no_of_clients << " peers\n";

	FD_ZERO(&fdset);
	FD_ZERO(&temp_fdset);

	FD_SET(listenfd, &fdset);
	FD_SET(STDIN_FILENO, &fdset);
	maxfd = (listenfd > STDIN_FILENO) ? listenfd : STDIN_FILENO;

	for (i = 0; i < no_of_clients; i++)
	{
		if (clients[i] > -1)
		{
			FD_SET(clients[i], &fdset);
			if (clients[i] > maxfd)
				maxfd = clients[i];
		}
	}

	maxi = i;

	for (;;)
	{
		temp_fdset = fdset;

		if ((nready = select(maxfd + 1, &temp_fdset, NULL, NULL, NULL)) == -1)
		{
			//error
			system_error("Error");
		}

//		cout << "After Select: nready is " << nready <<endl;

		if (FD_ISSET(listenfd,&temp_fdset))
		{

//			cout << "new connection" << endl;
			client_len = sizeof(client_addr);
			connfd = accept(listenfd, (struct sockaddr *) &client_addr,
					&client_len);
			if (connfd < 0)
			{
				show_error("Accept Error!!");
				continue;
			}

			for (i = 0; i < FD_SETSIZE; i++)
			{
				if (clients[i] < 0)
				{
					clients[i] = connfd;
					no_of_clients++;
					break;
				}

			}

			if (i == FD_SETSIZE)
			{
				system_error("Too many clients!!");
			}

			in_addr t;
			t.s_addr = (client_addr.sin_addr.s_addr);
			cout << "New connection from " << inet_ntoa(t) << ":"
					<< ntohs(client_addr.sin_port) << endl;
			FD_SET(connfd, &fdset);
			cout << "Connected to " << no_of_clients << " peers\n";
			i++;
			if (i > maxi)
				maxi = i;

			if (connfd > maxfd)
			{
				maxfd = connfd;
			}

		}
		if (FD_ISSET(STDIN_FILENO, &temp_fdset))
		{
//			cout << "keyboard input" << endl;
			n = keyboard_input(writebuf, SIZE_ - 1);
			writebuf[n] = 0;

			if (n <= 0)
			{
				break;
			}
			else
			{
				c_packet packet;
				packet.message = (char*) malloc(
						sizeof(char) * (strlen(writebuf)));
				strcpy(packet.message, writebuf);
				packet.seq = (sequence)++;
//				packet.ipaddress = 1;
				packet.port = localport;
				string hst = hostnm;
				packet.hostname = hostnm;
				insert_into_peer_seq_map(hst, localport, sequence);
				string sendmsg = serialize_message(&packet);

				send_to_all(sendmsg, clients, maxi);
			}

		}

		for (i = 0; i <= maxi; i++)
		{

			if ((sockfd = clients[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &temp_fdset))
			{

//				cout << "message received" << endl;
				memset(received, 0, sizeof(received));
				struct sockaddr_in addr;
				size_t len = sizeof addr;
				getpeername(sockfd, (struct sockaddr*) &addr, &len);
				int peerport = ntohs(addr.sin_port);

				char ipstr[INET_ADDRSTRLEN];
				strcpy(ipstr, inet_ntoa(addr.sin_addr));
				if ((n = read(sockfd, received, SIZE_ - 1)) <= 0)
				{
					//connection closed by client

					close(sockfd);
					FD_CLR(sockfd, &fdset);
					clients[i] = -1;
					no_of_clients--;
					cout << "Connection closed by peer " << ipstr << ":"
							<< peerport << endl;
					cout << "Connected to " << no_of_clients << " peers\n";
				}
				else
				{
//					cout << received << endl;

					if (addr.sin_family == AF_INET)
					{

						handle_incoming_message(received, clients,
								maxi, ipstr);
					}

				}
			}

		}

	}

	for (i = 0; i <= maxi; i++)
	{
		if (clients[i] != -1)
			close(clients[i]);
	}

	return EXIT_SUCCESS;
}
