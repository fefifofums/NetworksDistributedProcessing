/*
 * function.h
 * CSCE 612 hw 2
 * by William Bogardus
 */


#pragma once
#pragma comment(lib, "Ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS



#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include "constants.h"
#include <sstream>
#include <unordered_set>

using namespace std;


#pragma pack(push,1)
class QueryHeader {
public:
	u_short queryType;
	u_short queryClass;
};

class FixedDNSheader {
public:
	u_short dnsID;
	u_short dnsFlags;
	u_short dnsQuestions;
	u_short dnsAnswers;
	u_short dnsAuthority;
	u_short dnsAdditional;
};

class DNSanswerHdr {
public:
	u_short answerType;
	u_short answerClass;
	u_int answerTTL;
	u_short answerLen;
};
#pragma pack(pop)


int GetQueryType(char* host);

void MakeDNSquery(char* host, int type, char* dnsServerIP);

void MakeDNSquestion(char* buf, char* host);

char* reverseIP(char* IP);

DWORD getElapsedTime(DWORD time);

void SendDNSquery(char* buf, char* dnsServerIP, int packetSize);

void GracefulExit();

char* RRname(char* zeroBuf, char* buf, int numOfBytes, unordered_set<unsigned int> *offsets);// , unsigned int loopCount);

void PrintType(int dnsType);

void PrintArdata(char** buf);

void PrintRRdata(char** tempBuf, char* recBuf, int loopValue, int numOfBytes);
