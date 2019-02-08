/* main.cpp
 * CSCE 612 hw part 1
 * by William Bogardus
 * based on  threads/winsock 
 * and HTML parser by Dmitri Loguinov
 * http://irl.cs.tamu.edu/courses/463/
 */

#include "common.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <crtdbg.h>



// function inside winsock.cpp
//void winsock_test (URL_parse *url, bool single, IPS *ips);
void winsock_test2(URL_parse *url, bool single, IPS *ips);



bool urlParser(char* url, URL_parse *target_url, HOSTS *hosts, bool single){
	/*
	printf("%s", strurl);
	char* url = new char[strurl.length() + 1];
	strcpy(url, strurl.c_str());
	delete[] url;
	printf("URL: %s\n\tParsing URL... ", url);
	*/
	
	size_t length = strlen(url);
	unsigned int found;
	target_url->url = strdup(url);
	//bool port = true;
	
	const char* args[5] = { "http://", "#", "?", "/", ":" };
	const char* urlPart[5] = { "scheme", "fragment", "query", "path", "port" };

	// Followed algorithm from lecture 2
	for (int i = 0; i < 5; i++) {
		
		char* arg = strstr(url, args[i]); 
		if (arg != NULL){
			found = arg - url;
			if (i == 0) {
				target_url->scheme = "http://";
				memset(url, NULL, 7);
				url += 7;
				length = strlen(url);
			} 
			else {
				if (i == 2) target_url->query = strdup(arg);		
				if (i == 3) target_url->path = strdup(arg);
				if (i == 4 ){
					target_url->port = strdup(arg + 1);
					if (atoi(target_url->port)< 1 || atoi(target_url->port) > 65535) {
						printf("\tfailed with invalid port\n");
						return false;
					}
				}
				memset(url + found, NULL, length - found);
				length = strlen(url);
			}
		}
		else if (i == 3) {
			// default path is root /
			target_url->path = "/";
		}
		else if (i == 4) {
			// default port is 80
			target_url->port = "80";
		}
		else if (i == 0) {
			// Invalid scheme
			printf("\tfailed with invalid %s\n", urlPart[i]);
			return false;
		}
	}

	//_CrtCheckMemory();

	 if (strlen(url) > 0 ) target_url->host = url;
	 else {
		 // Invalid host ?, maybe if no host is given
		 printf("failed with invalid host");
		 return false;
	 }
	 if (single) {
		 if (target_url->query != NULL) {
			 printf("\thost %s, port %s, request %s%s\n", target_url->host, target_url->port, target_url->path, target_url->query);
			 //Fixed heap curruption issue with help of Di, Correct memory allocation made before strcat is called
			 target_url->request = (char*)malloc(strlen(target_url->path) + strlen(target_url->query) + 1);
			 *(target_url->request) = '\0';

			 strcat(target_url->request, target_url->path);
			 //_CrtCheckMemory();
			 strcat(target_url->request, target_url->query);
			 //_CrtCheckMemory();
		 }
		 else {
			 printf("\thost %s, port %s, request %s\n", target_url->host, target_url->port, target_url->path);
			 target_url->request = target_url->path;
		 }
		 return true;
	 }
	 else {
		 if (target_url->query != NULL) {
			 printf("\thost %s, port %s\n", target_url->host, target_url->port);
			 //Fixed heap curruption issue with help of Di, Correct memory allocation made before strcat is called
			 target_url->request = (char*)malloc(strlen(target_url->path) + strlen(target_url->query) + 1);
			 *(target_url->request) = '\0';

			 strcat(target_url->request, target_url->path);
			 //_CrtCheckMemory();
			 strcat(target_url->request, target_url->query);
			 //_CrtCheckMemory();
		 }
		 else {
			 printf("\thost %s, port %s\n", target_url->host, target_url->port);
			 target_url->request = target_url->path;
		 }
	 }
	 printf("\tChecking host uniqueness... ");
	 
	
	 
	 
	 hosts->previousSize = hosts->hosts.size();
	 hosts->hosts.insert(target_url->host);
	 if (hosts->hosts.size() > hosts->previousSize)
	 {
		 printf("passed\n");
		 return true;
	 }
	 else
	 {
		 printf("failed\n");
		 return false;
	 }
	

	
}

bool openFile(char *name, vector<string> *urls) {
	/*
	char filename[] = "URL-input-100.txt";
	HANDLE urlFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (urlFile == INVALID_HANDLE_VALUE) {
		printf("Failed to open file %s.\n", name);
		return;
	}

	LARGE_INTEGER li;
	BOOL bRet = GetFileSizeEx(urlFile, &li);
	if (bRet == 0){
		printf("Failed to open file %s.\n", name);
		return;
	}
	int fileSize = (DWORD)li.QuadPart;
	*/
	

	//vector<string> urls;
	string line;
	unsigned int fileSize = 0;

	
	ifstream urlFile;
	
	urlFile.open((string)name);

	// Check if file is open
	if (!urlFile.is_open()) {
		printf("Failed to open file %s.\n", name);
		return false;
	}
	

	// Get filesize code based on http://www.cplusplus.com/doc/tutorial/files/
	streampos firstbyte, lastbyte;
	firstbyte = urlFile.tellg();
	urlFile.seekg(0, ios::end);
	lastbyte = urlFile.tellg();
	fileSize = lastbyte - firstbyte;
	if (fileSize == 0) {
		printf("Empty file\n");
		return false;
	}

	//reset iostream to begining
	urlFile.seekg(0, ios::beg);

	// Push each line into vector

	while (urlFile.good()) {
		getline(urlFile, line);
		urls->push_back(line.c_str());
	}
	printf("Opened %s.txt with size %d\n", name, fileSize);
	return true;
	
	
	
}

int main(int argc, char** argv)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
	struct HOSTS hosts;
	struct IPS ips;
	struct URL_parse url;
	mutex mtx;

	
  
	

	
	
	if (argc == 2) {
		char* arg1 = argv[1];
		//if (urlParser(arg1, &url, &hosts, true)) winsock_test(&url, true, &ips);
		//else return 0;
	}
	else if (argc == 3) {
		if ((string)argv[1] != "1") {
			printf("Please enter 1 thread at this time.");
			return 0;
		}
		vector<string> urls;
		if (openFile(argv[2], &urls)) {
			for (int i = 0; i < urls.size() ; i++) {
				mtx.lock();
				char* urlptr = new char [urls[i].size() + 1];
				strcpy(urlptr, urls[i].c_str());				
				printf("URL: %s\n\tParsing URL... ", urlptr);
				if (urlParser(urlptr, &url, &hosts, false)) winsock_test2(&url, false, &ips);
				url.reset();
				mtx.unlock();
			}
		}
	}
	else printf("Invalid input\n");
	

	

	return 0; 
}
