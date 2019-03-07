// CSCE612 hw2.cpp 
// By William Bogardus




#pragma once

#include "function.h"


int main(int argc, char** argv)
{
	//Initialize WinSock; once per program run
	WSADATA wsaData;
	DWORD wVersionRequested;
	wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	int qType;

	if (argc != 3) {
		printf("Invalid input\n");
		return 1;
	}
	printf("Lookup   : %s\n", argv[1]);
	qType = GetQueryType(argv[1]);
	if (qType == 12) {
		char* revIP = reverseIP(argv[1]);
		MakeDNSquery(revIP, qType, argv[2]);
	}
	else MakeDNSquery(argv[1], qType, argv[2]);

	return 0;
}



