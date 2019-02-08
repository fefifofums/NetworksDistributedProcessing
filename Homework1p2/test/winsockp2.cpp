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
#include "common.h"



class Socket2 {
public:
	SOCKET sock;
	char* buf;
	int allocatedSize = INITIAL_BUFFER_SIZE;
	int curPos = 0;
	Socket2();
};

Socket2::Socket2()
{
	buf = (char*)malloc(INITIAL_BUFFER_SIZE);
}

// Function timing based on material found at
// https://www.geeksforgeeks.org/measure-execution-time-function-cpp/
high_resolution_clock::time_point get_time(void) {
	return high_resolution_clock::now();
}

auto elapsed_time(high_resolution_clock::time_point start, high_resolution_clock::time_point end) {
	return duration_cast<milliseconds>(end - start);
}




void winsock_test2(URL_parse *url, bool single, IPS *ips)//void)
{

	// string pointing to an HTTP server (DNS name or IP)
	char* str = url->host;

	//Initialize WinSock; once per program run
	WSADATA wsaData;
	DWORD wVersionRequested;
	wVersionRequested = MAKEWORD(2, 2);
	int iResult;
	iResult = WSAStartup(wVersionRequested, &wsaData);
	if (iResult != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
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
			WSACleanup();
			return;
		}
		else { // take the first IP address and copy into sin_addr 
			memcpy((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
			// End timer, get time
			auto end = get_time();
			auto elapsed = elapsed_time(start, end);
			char* ipa = inet_ntoa(server.sin_addr);
			printf("done in %d ms, found ", elapsed);
			printf("%s\n", inet_ntoa(server.sin_addr));

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
					WSACleanup();
					return;
				}
			}

		}
	}

	// open a TCP socket
	Socket2 robo;
	robo.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (robo.sock == INVALID_SOCKET)
	{
		printf("socket() generated error %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	printf("\tConnecting on robots... ");
	auto startr1 = get_time();
	// setup the port # and protocol type
	server.sin_family = AF_INET;
	// Use other ports besides 80
	server.sin_port = htons(stoi(url->port));		// host-to-network flips the byte order

	// connect to the server on given port
	if (connect(robo.sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf("failed with %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	// More timing
	auto endr1 = get_time();
	auto elapsedr1 = duration_cast<milliseconds>(endr1 - startr1);
	printf("done in %d ms\n", elapsedr1, url->host);

	// send HTTP requests here
	printf("\tLoading... ");
	auto startr2 = get_time();
	// Construct robot request

	string robo_request = "HEAD /robots.txt HTTP/1.0\r\nUser-agent: webcrawler/1.1\r\nHost: " + string(url->host) + "\r\nConnection: close\r\n\r\n";
	unsigned int roboRequestLength = robo_request.size();
	//printf("%s\n", http_request.c_str());

	// Send http request to server
	const char *roboSendBuf = new char[roboRequestLength];
	roboSendBuf = robo_request.c_str();

	if (send(robo.sock, roboSendBuf, roboRequestLength, 0) == SOCKET_ERROR) {
		printf("failed with %d on send\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	fd_set fdsr;
	timeval tvr;
	FD_ZERO(&fdsr);
	FD_SET(robo.sock, &fdsr);
	tvr.tv_sec = 10;
	tvr.tv_usec = 0;

	int bytesR = 1;
	int robo_socket_handles;
	bool good = false;

	while (bytesR > 0)
	{
		// wait to see if socket has any data (see MSDN)
		;
		if ((robo_socket_handles = select(0, &fdsr, 0, 0, &tvr)) > 0)
		{
			// new data available; now read the next segment
			bytesR = recv(robo.sock, robo.buf + robo.curPos, robo.allocatedSize - robo.curPos, 0);
			if (bytesR > 0) {
				//printf("keep going...\n");
				//return;
			}
			else if (bytesR == 0) {
				// NULL-terminate buffer
				// Needs work, maybe strpbrk
				// Suppose to set 1 past last meaningful char to \0
				//Fixed null termination issue with help of Di, insert null terminator at end of buffer
				good = true;
				robo.buf[robo.curPos] = '\0';
				/*char* last = strrchr(robo.buf, '>');
				if (last != NULL) memset(last + 1, NULL, 1);*/
				break;
			}
			else {
				printf("failed with recv %d\n", WSAGetLastError());
				WSACleanup();
				return;
			}

			robo.curPos += bytesR; // adjust where the next recv goes

			if (robo.curPos > (2 * INITIAL_BUFFER_SIZE)) {
				printf("failed with exceeding max\n");

			}

			//dynamic buffer resizing
			if (robo.allocatedSize - robo.curPos < robo.allocatedSize / 4)
			{
				//robo.allocatedSize += INITIAL_BUFFER_SIZE;
				robo.allocatedSize *= 2;
				char* temp = (char*)realloc(robo.buf, robo.allocatedSize);
				robo.buf = temp;
				//free(temp);
			}
		}
		else if (robo_socket_handles == 0) {
			printf("failed with slow download");
			// report timeout
			WSACleanup();
			return;
		}
		else {
			// recv error
			printf("failed with %d on recv\n", WSAGetLastError());
			WSACleanup();
			return;
		}
	}

	if (good) {
		// Keep track of time...
		auto endr2 = get_time();
		auto elapsedr2 = duration_cast<milliseconds>(endr2 - startr2);
		printf("done in %d ms with %d bytes\n", elapsedr2, robo.curPos);
	}

	char* roboFile = robo.buf;

	closesocket(robo.sock);

	printf("\tVerifying header... ");
	char* robo_header_start = strstr(roboFile, "HTTP/");
	if (robo_header_start == NULL) {
		printf("failed with non-HTTP header\n");
		WSACleanup();
		return;
	}
	char* status_start = strchr(roboFile, ' ');
	status_start += 1;
	if (status_start[0] != NULL) {
		char status_code[4];
		status_code[0] = status_start[0];
		status_code[1] = status_start[1];
		status_code[2] = status_start[2];
		status_code[3] = NULL;
		printf("status code %s\n", status_code);
	}
	if (status_start[0] == '4') {

		// open a TCP socket
		Socket2 Sock2;
		Sock2.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Sock2.sock == INVALID_SOCKET)
		{
			printf("socket() generated error %d\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		printf("      * Connection on page... ");
		auto start2 = get_time();
		// setup the port # and protocol type
		server.sin_family = AF_INET;
		// Use other ports besides 80
		server.sin_port = htons(stoi(url->port));		// host-to-network flips the byte order

		// connect to the server on given port
		if (connect(Sock2.sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			printf("failed with %d\n", WSAGetLastError());
			WSACleanup();
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

		if (send(Sock2.sock, sendBuf, requestLength, 0) == SOCKET_ERROR) {
			printf("failed with %d on send\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		fd_set fds;
		timeval tv;
		FD_ZERO(&fds);
		FD_SET(Sock2.sock, &fds);
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		int bytes = 1;
		int socket_handles;
		good = false;

		while (bytes > 0)
		{
			// wait to see if socket has any data (see MSDN)
			;
			if ((socket_handles = select(0, &fds, 0, 0, &tv)) > 0)
			{
				// new data available; now read the next segment
				bytes = recv(Sock2.sock, Sock2.buf + Sock2.curPos, Sock2.allocatedSize - Sock2.curPos, 0);
				if (bytes > 0) {
					//printf("keep going...\n");
					//return;
				}
				else if (bytes == 0) {
					// NULL-terminate buffer
					// Fixed null termination issue with help of Di, insert null terminator at end of buffer
					good = true;
					Sock2.buf[Sock2.curPos] = '\0';
					break;
				}
				else {
					printf("failed with recv %d\n", WSAGetLastError());
					WSACleanup();
					return;
				}

				Sock2.curPos += bytes; // adjust where the next recv goes

				if (Sock2.curPos > (250 * INITIAL_BUFFER_SIZE)) {
					printf("failed with exceeding max\n");

				}

				//dynamic buffer resizing
				if (Sock2.allocatedSize - Sock2.curPos < Sock2.allocatedSize / 4)
				{
					Sock2.allocatedSize += INITIAL_BUFFER_SIZE;
					char* temp = (char*)realloc(Sock2.buf, Sock2.allocatedSize);
					Sock2.buf = temp;
					//free(temp);
				}
			}
			else if (socket_handles == 0) {
				printf("failed with slow download");
				// report timeout
				WSACleanup();
				return;
			}
			else {
				// recv error
				printf("failed with %d on recv\n", WSAGetLastError());
				WSACleanup();
				return;
			}
		}

		if (good) {
			// Keep track of time...
			auto end3 = get_time();
			auto elapsed3 = duration_cast<milliseconds>(end3 - start3);
			printf("done in %d ms with %d bytes\n", elapsed3, Sock2.curPos);
			printf("\tVerifying header... ");
		}

		char* file = Sock2.buf;

		// close the socket to this server; open again for the next one
		closesocket(Sock2.sock);

		// check status code and print it
		char* header_start = strstr(file, "HTTP/");
		if (header_start == NULL) {
			printf("failed with non-HTTP header\n");
			WSACleanup();
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
		if (status_start[0] == '2') {
			printf("      + Parsing page... ");
			auto start4 = get_time();

			// create new parser object
			HTMLParserBase *parser = new HTMLParserBase;

			// combine scheme and host as instructed
			char *baseURL = (char*)malloc(strlen(url->scheme) + strlen(url->host) + 1);
			baseURL[0] = '\0';
			strcat(baseURL, url->scheme);
			_CrtCheckMemory();
			strcat(baseURL, url->host);
			_CrtCheckMemory();

			int nLinks;
			//no need to save links  
			parser->Parse(file, string(file).length(), baseURL, (int)strlen(baseURL), &nLinks);
			if (nLinks < 0) {
				printf("Parsing error\n");
				free(baseURL);
				WSACleanup();
				return;
			}

			// More time..
			auto end4 = get_time();
			auto elapsed4 = duration_cast<milliseconds>(end4 - start4);

			// search for double line break, incert \0, print results
			printf("done in %d ms with %d links\n", elapsed4, nLinks);
			free(baseURL);
		}
		/*
		char* info_end = strstr(file, "\n\r\n");
		printf("\n----------------------------------------\n");
		if (info_end != NULL) {
			info_end[0] = '\0';
			printf("%s\n", file);
		}*/
		WSACleanup();
		
		return;
	}







	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////











}