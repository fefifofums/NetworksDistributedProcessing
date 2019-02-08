/* 
 * common.h
 * CSCE 612 hw part 1
 * by William Bogardus
 * based on  threads/winsock 
 * and HTML parser by Dmitri Loguinov
 * http://irl.cs.tamu.edu/courses/463/
 */

// some common .h files every .cpp needs
#pragma once
#include "stdafx.h"
#include <mmsystem.h>
#include <string>
#include <chrono>
#include <unordered_set>
#include <mutex>          // std::mutex



using namespace std;		// if you're planning to use STL
using namespace std::chrono;

typedef struct URL_parse {
public:
	char* url = NULL;
	char* scheme = NULL;
	char* query = NULL;
	char* path = NULL;
	char* port = NULL;
	char* host = NULL;
	char* request = NULL;

	void reset() {
		char* url = NULL;
		char* scheme = NULL;
		char* query = NULL;
		char* path = NULL;
		char* port = NULL;
		char* host = NULL;
		char* request = NULL;
	}
};

typedef struct IPS {
	unordered_set<DWORD> ips;
	unsigned int previousSize = 0;
};
typedef struct HOSTS {
	unordered_set<string> hosts;
	unsigned int previousSize = 0;
};

