/* winsock.cpp
 * CSCE 612 hw part 1
 * by William Bogardus
 * based on threads/winsock
 * and HTML parser by Dmitri Loguinov
 * http://irl.cs.tamu.edu/courses/463/
 */

#define INITIAL_BUFFER_SIZE 8192

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "common.h"



class Socket {
public:
	SOCKET sock;
	char* buf;
	int allocatedSize = 2048;
	int curPos = 0;
	Socket();
};

Socket::Socket()
{
	buf = new char[INITIAL_BUFFER_SIZE];
}





void winsock_test(URL_parse *url, bool single, IPS *ips)//void)
{
	// string pointing to an HTTP server (DNS name or IP)
	char* str = url->host;
	
	
	

	// open a TCP socket
	Socket Sock;
	Sock.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Sock.sock == INVALID_SOCKET)
	{
		printf("socket() generated error %d\n", WSAGetLastError());
		
		return;
	}

	// structure used in DNS lookups
	struct hostent *remote;
	// structure for connecting to server
	struct sockaddr_in server;

	// first assume that the string is an IP address
	printf("\tDoing DNS... ");
	//start timer
	auto start = get_time();
	DWORD IP = inet_addr(url->host);
	
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup by hostname
		if ((remote = gethostbyname(str)) == NULL)
		{
			printf("failed with dns %d\n", WSAGetLastError());
			return;
		}
		else { // take the first IP address and copy into sin_addr 
			memcpy((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
			// End timer, get time
			auto end = get_time();
			auto elapsed = elapsed_time(start, end);
			printf("done in %d ms, found %s\n", elapsed, inet_ntoa(server.sin_addr));
			if (!single) {
				printf("\tChecking IP uniqueness... ");
				ips->previousSize = ips->ips.size();
				ips->ips.insert(inet_addr(inet_ntoa(server.sin_addr)));
				if (ips->ips.size() > ips->previousSize) 
				{
					printf("passed\n");
				}
				else 
				{
					printf("failed\n");
					return;
				}
			}
			
		}
	}
	//else server.sin_addr.S_un.S_addr = IP;
	
	




	printf("      * Connection on page... ");
	auto start2 = get_time();
	// setup the port # and protocol type
	server.sin_family = AF_INET;
	// Use other ports besides 80
	server.sin_port = htons(stoi(url->port));		// host-to-network flips the byte order

	// connect to the server on given port
	if (connect(Sock.sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf("failed with %d\n", WSAGetLastError());
		return;
	}
	// More timing
	auto end2 = get_time();
	auto elapsed2 = duration_cast<milliseconds>(end2 - start2);
	printf("done in %d ms\n", elapsed2, url->host);


	// send HTTP requests here
	printf("\tLoading... ");
	auto start3 = get_time();
	// Construct HTTP request
	string http_request = "GET " + string(url->request) + " HTTP/1.0\r\nUser-agent: webcrawler/1.0\r\nHost: " + string(url->host) + "\r\nConnection: close\r\n\r\n";
	unsigned int requestLength = http_request.size();
	//printf("%s\n", http_request.c_str());

	// Send http request to server
	const char *sendBuf = new char[requestLength];
	sendBuf = http_request.c_str();

	if (send(Sock.sock, sendBuf, requestLength, 0) == SOCKET_ERROR) {
		printf("failed with %d on send\n", WSAGetLastError());
		return;
	}

	fd_set fds;
	timeval tv;
	FD_ZERO(&fds);
	FD_SET(Sock.sock, &fds);
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	
	int bytes = 1;
	do {
		// wait to see if socket has any data (see MSDN)
		if (select(0, &fds, 0, 0, &tv) > 0)
		{
			// new data available; now read the next segment
			bytes = recv(Sock.sock, Sock.buf + Sock.curPos, Sock.allocatedSize - Sock.curPos, 0);
			if (bytes > 0) {
				//printf("keep going...\n");
				//return;
			}
			else if (bytes == 0) {
				// NULL-terminate buffer
				// Needs work, maybe strpbrk
				// Suppose to set 1 past last meaningful char to \0
				char* last = strrchr(Sock.buf, '>');
				if (last != NULL) memset(last + 1, NULL, 1);
				break;
				}
			else {
				printf("failed with recv %d\n", WSAGetLastError());
				return;
			}

			Sock.curPos += bytes; // adjust where the next recv goes

			//dynamic buffer resizing
			if (Sock.allocatedSize - Sock.curPos < Sock.allocatedSize/4)
			{
				Sock.allocatedSize *= 2;
				char* temp = (char*)realloc(Sock.buf, Sock.allocatedSize);
				Sock.buf = temp;
			}
		}
		else if (tv.tv_sec) {
			printf("failed with timeout");
			// report timeout
			return;
		}
		else {
			// recv error
			printf("failed with %d on recv\n", WSAGetLastError());
			return;
		}
	} while (bytes > 0);
	// Keep track of time...
	auto end3 = get_time();
	auto elapsed3 = duration_cast<milliseconds>(end3 - start3);
	printf("done in %d ms with %d bytes\n", elapsed3, string(Sock.buf).length());
	printf("\tVerifying header... ");

	char* file = Sock.buf;

	// close the socket to this server; open again for the next one
	closesocket(Sock.sock);

	// call cleanup when done with everything and ready to exit program
	

	// check status code and print it
	char* header_start = strstr(file, "HTTP/");
	if (header_start == NULL) {
		printf("failed with non-HTTP header\n");
		return;
	}
	char* status_start = strchr(file, ' ');
	status_start += 1;
	if (status_start[0] != NULL) {
		char status_code[4];
		status_code[0] = status_start[0];
		status_code[1] = status_start[1];
		status_code[2] = status_start[2];
		status_code[3] = NULL;
		printf("status code %s\n", status_code);
	}
	if (status_start[0] == 2) {
		printf("      + Parsing page... ");
		auto start4 = get_time();

		// create new parser object
		HTMLParserBase *parser = new HTMLParserBase;

		// combine scheme and host as instructed
		char* baseURL = strcat(strdup(url->scheme), strdup(url->host));

		int nLinks;
		//no need to save links  
		parser->Parse(file, string(file).length(), baseURL, (int)strlen(baseURL), &nLinks);
		if (nLinks < 0) {
			printf("Parsing error\n");
			return;
		}

		// More time..
		auto end4 = get_time();
		auto elapsed4 = duration_cast<milliseconds>(end4 - start4);

		// search for double line break, incert \0, print results
		printf("done in %d ms with %d links\n", elapsed4, nLinks);
	}
	char* info_end = strstr(file, "\n\r\n");
	printf("\n----------------------------------------\n");
	if (info_end != NULL) {
		info_end[0] = '\0';
		printf("%s\n", file);
	}
	

}