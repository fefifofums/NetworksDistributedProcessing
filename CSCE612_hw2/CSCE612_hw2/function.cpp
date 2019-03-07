#include "function.h"



void PrintRRdata(char** tempBuf, char* recBuf, int loopValue, int numOfBytes)
{
	unsigned int loopCount = 0;
	for (int i = 0; i < loopValue; i++)
	{
		if (((*tempBuf) + 14) - recBuf >= numOfBytes)
		{
			printf("  \t++ invalid record: truncated fixed RR header\n");
			GracefulExit();
		}

		printf("  \t");
		unordered_set<unsigned int> offsets;
		(*tempBuf) = RRname(recBuf, (*tempBuf), numOfBytes, &offsets);

		DNSanswerHdr *aah = (DNSanswerHdr *)(*tempBuf);

		int dnsType = (u_short)htons(aah->answerType);

		unsigned int length = (u_short)htons(aah->answerLen) & 0xFF;

		// Some issues here
		if (((*tempBuf) + length - recBuf) > numOfBytes)
		{
			printf("\n  \t++ invalid record: RR value length beyond packet\n");
			GracefulExit();
		}
		PrintType(dnsType);
		(*tempBuf) += 10;

		offsets.clear();

		if (dnsType == DNS_NS || dnsType == DNS_CNAME || dnsType == DNS_PTR)
			(*tempBuf) = RRname(recBuf, (*tempBuf), numOfBytes, &offsets);
		else if (dnsType == DNS_A) PrintArdata(&(*tempBuf));
		else
		{
			if (dnsType == 28) printf(" AAAA ");
			(*tempBuf) += 16;
		}
		unsigned long ttl = htonl(aah->answerTTL) & 0xFFFFFFFF;
		printf(" TTL = %lu\n", ttl);


	}
}

void PrintArdata(char** buf)
{
	for (int i = 0; i < 4; i++)
	{
		int num = (*buf)[i] & 0xFF;
		printf("%d", num);
		if (i < 3) printf(".");
	}
	(*buf) += 4;
}

int GetQueryType(char* host)
{
	if (inet_addr(host) == INADDR_NONE)
		return DNS_A;	// type A
	return DNS_PTR;		// type PTR
}

void MakeDNSquery(char* host, int type, char* dnsServerIP)
{
	int pkt_size = strlen(host) + 2 + sizeof(FixedDNSheader) + sizeof(QueryHeader);
	char *buf = new char[pkt_size];

	FixedDNSheader *dh = (FixedDNSheader *)buf;
	QueryHeader *qh = (QueryHeader *)(buf + pkt_size - sizeof(QueryHeader));
	dh->dnsID = htons(1234);
	dh->dnsFlags = htons(DNS_QUERY | DNS_RD | DNS_STDQUERY);
	dh->dnsQuestions = htons(1);
	dh->dnsAnswers = htons(0);
	dh->dnsAuthority = htons(0);
	dh->dnsAdditional = htons(0);

	qh->queryType = ((type == 1) ? htons(DNS_A) : htons(DNS_PTR));
	qh->queryClass = htons(DNS_INET);


	printf("Query:   : %s, type %d, TXID 0x%.4X\n", host, type, htons(dh->dnsID));
	printf("Server:  : %s\n", dnsServerIP);
	printf("********************************\n");

	MakeDNSquestion(buf, host);

	SendDNSquery(buf, dnsServerIP, pkt_size);



}

void PrintType(int dnsType)
{
	switch (dnsType)
	{
	case DNS_A: printf(" A "); break;
	case DNS_NS: printf(" NS "); break;
	case DNS_CNAME: printf(" CNAME "); break;
	case DNS_PTR: printf(" PTR "); break;
	}
}

void MakeDNSquestion(char* buf, char* host)
{
	int nextWordSize = 0;
	int i = 0;
	char* nextWord = strtok(host, ".");
	buf += 12;

	while (nextWord != NULL) {
		nextWordSize = strlen(nextWord);
		buf[i++] = nextWordSize;
		memcpy(buf + i, nextWord, nextWordSize);
		i += nextWordSize;
		nextWord = strtok(NULL, ".");
	}
	buf[i] = 0;
}

char* RRname(char* zeroBuf, char* buf, int numOfBytes, unordered_set<unsigned int> *offsets)//, unsigned int loopCount)
{

	int wordSize = (unsigned char)buf[0] & 0xF;
	unsigned int check = (unsigned char)buf[0] & 0xFF;
	if (check >= 0xC0)
	{
		unsigned int offset = (((unsigned char)buf[0] & 0x3F) << 8) + (unsigned char)buf[1];
		if (buf + 1 - zeroBuf > numOfBytes - 1)
		{
			printf("\n  \t++ invalid record: truncated jump offset\n");
			GracefulExit();
		}
		if (offset > 0 && offset < 12)
		{
			printf("\n  \t++ invalid record: jump into fixed header\n");
			GracefulExit();
		}
		if (offset > numOfBytes)
		{
			printf("++ invalid record: jump beyond packet boundry\n");
			GracefulExit();
		}
		int previousSize = (*offsets).size();
		(*offsets).insert(offset);
		if ((*offsets).size() == previousSize)
		{
			printf("++ invalid record: jump loop\n");
			GracefulExit();
		}

		RRname(zeroBuf, zeroBuf + offset, numOfBytes, &(*offsets));
		return buf + 2;
	}
	else
	{

		std::string name, temp;
		int totalWordSize = 0;


		while (wordSize > 0)
		{
			buf += 1;
			if (buf + wordSize - zeroBuf > numOfBytes)
			{
				printf("\n  \t++ invalid record: truncated name\n");
				GracefulExit();
			}
			std::string name(buf, wordSize);

			totalWordSize = totalWordSize + wordSize;
			buf += wordSize;

			wordSize = (unsigned char)buf[0];
			printf("%s", name.c_str());
			check = (unsigned char)buf[0] & 0xFF;
			if (check >= 0xC0)
			{
				unsigned int offset = (((unsigned char)buf[0] & 0x3F) << 8) + (unsigned char)buf[1];
				if (buf + 1 - zeroBuf > numOfBytes - 1)
				{
					printf("++ invalid record: truncated jump offset\n");
					GracefulExit();
				}
				if (offset > 0 && offset < 12)
				{
					printf("++ invalid record: jump into fixed header\n");
					GracefulExit();
				}
				if (offset > numOfBytes)
				{
					//printf("\n%lu\n", offset);
					printf("++ invalid record: jump beyond packet boundry\n");
					GracefulExit();
				}
				int previousSize = (*offsets).size();
				(*offsets).insert(offset);
				if ((*offsets).size() == previousSize)
				{
					printf("++ invalid record: jump loop\n");
					GracefulExit();
				}
				RRname(zeroBuf, zeroBuf + offset, numOfBytes, &(*offsets));
				return buf + 2;
			}
			if (wordSize == 0) break;
			printf(".");


		}

		return buf + 1;
	}
}

char* reverseIP(char* IP)
{
	char* forward;
	std::string reverse, temp;

	forward = strtok(IP, ".");

	while (forward != NULL)
	{
		std::string temp(forward, strlen(forward));
		reverse = temp + reverse;
		forward = strtok(NULL, ".");
		if (forward != NULL) reverse = "." + reverse;
		else reverse = reverse + ".in-addr.arpa";
	}

	char* reverseIP = new char[strlen(IP) + 14];
	strcpy(reverseIP, reverse.c_str());
	return reverseIP;
}

DWORD getElapsedTime(DWORD time) {
	return timeGetTime() - time;
}

void SendDNSquery(char* buf, char* dnsServerIP, int packetSize)
{
	int count = 0;
	while (count++ < MAX_ATTEMPTS) {

		printf("Attempt %d with %d bytes...", count - 1, packetSize);
		DWORD time = timeGetTime();
		DWORD endTime;

		SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

		if (sock == INVALID_SOCKET)
		{
			printf("socket() generated error %d\n", WSAGetLastError());
			GracefulExit();
		}

		struct sockaddr_in local;
		memset(&local, 0, sizeof(local));
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = INADDR_ANY;
		local.sin_port = htons(0);

		if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
		{
			//printf("bind() generated error %d\n", WSAGetLastError());
			printf("socket() generated error %d\n", WSAGetLastError());
			GracefulExit();
		}

		struct sockaddr_in remote;
		memset(&remote, 0, sizeof(remote));
		remote.sin_family = AF_INET;
		remote.sin_addr.s_addr = inet_addr(dnsServerIP);
		remote.sin_port = htons(53);

		if (sendto(sock, buf, packetSize, 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR)
		{
			//printf("sendto() generated error %d\n", WSAGetLastError());
			printf("socket() generated error %d\n", WSAGetLastError());
			GracefulExit();
		}

		char recBuf[MAX_DNS_LEN];
		struct sockaddr_in response;
		int sendAddrSize = sizeof(response);

		fd_set fd;
		timeval tvr;
		FD_ZERO(&fd);
		FD_SET(sock, &fd);
		tvr.tv_sec = 10;
		tvr.tv_usec = 0;
		int available = select(0, &fd, NULL, NULL, &tvr);


		if (available > 0)
		{
			int numOfBytes = recvfrom(sock, recBuf, MAX_DNS_LEN, 0, (struct sockaddr*)&response, &sendAddrSize);
			if (numOfBytes == SOCKET_ERROR)
			{
				//printf("recvfrom() generated error %d\n", WSAGetLastError());
				printf("socket() generated error %d\n", WSAGetLastError());
				GracefulExit();
			}

			if (response.sin_addr.s_addr != remote.sin_addr.s_addr || (response.sin_port != remote.sin_port))
			{
				printf("\nBogus reply\n");
				GracefulExit();
			}


			endTime = getElapsedTime(time);

			printf(" response in %d ms with %d bytes\n", endTime, numOfBytes);

			FixedDNSheader *rdh = (FixedDNSheader *)recBuf;
			FixedDNSheader *sdh = (FixedDNSheader *)buf;

			int nQuestions, nAnswers, nAuthority, nAdditional;
			nQuestions = (int)htons(rdh->dnsQuestions);
			nAnswers = (int)htons(rdh->dnsAnswers);
			nAuthority = (int)htons(rdh->dnsAuthority);
			nAdditional = (int)htons(rdh->dnsAdditional);


			printf("  TXID 0x%.4X flags 0x%.4X questions %d answers %d authority %d additional %d\n",
				htons(rdh->dnsID),
				htons(rdh->dnsFlags),
				nQuestions,
				nAnswers,
				nAuthority,
				nAdditional);

			

			if (numOfBytes < sizeof(FixedDNSheader))
			{
				printf("  ++ invalid reply: smaller than fixed header\n");
				GracefulExit();
			}
			if (sdh->dnsID != rdh->dnsID)
			{
				printf("  ++ invalid reply: TXID mismatch, sent 0x%.4X, received 0x%.4X\n",
					htons(sdh->dnsID),
					htons(rdh->dnsID));
				GracefulExit();
			}

			int rCode = htons(rdh->dnsFlags) & 0xF;

			if (rCode == DNS_OK)
				printf("  succeeded with Rcode = %d\n", rCode);
			else
			{
				printf("  failed with Rcode = %d\n", rCode);
				GracefulExit();
			}

			if ((nQuestions * 12 + (nAnswers + nAuthority + nAdditional) * 14) > numOfBytes)
			{
				printf("  \t++ invalid section: not enough records\n");
				GracefulExit();
			}

			char* tempBuf = recBuf + 12;
			if (nQuestions > 0)
			{
				printf("  ------------ [questions] ----------\n");

				for (int i = 0; i < nQuestions; i++)
				{
					if (tempBuf - recBuf > numOfBytes)
					{
						printf("  \t++ invalid section: not enough records\n");
						GracefulExit();
					}
					if ((tempBuf + 12) - recBuf > numOfBytes)
					{
						printf("  \t++ invalid record: truncated fixed RR header\n");
						GracefulExit();
					}
					printf("  \t");
					unordered_set<unsigned int> offsets;
					tempBuf = RRname(recBuf, tempBuf, numOfBytes, &offsets);
					QueryHeader *qah = (QueryHeader *)(tempBuf);
					printf(" type %d class %d\n", htons(qah->queryType), htons(qah->queryClass));
					tempBuf += 4;
					offsets.clear();
				}
			}

			if (nAnswers > 0)
			{
				printf("  ------------ [answers] ------------\n");
				PrintRRdata(&tempBuf, recBuf, nAnswers, numOfBytes);
			}

			if (nAuthority > 0)
			{
				printf("  ------------ [authority] ----------\n");
				PrintRRdata(&tempBuf, recBuf, nAuthority, numOfBytes);
			}

			if (nAdditional > 0)
			{
				printf("  ------------ [additional] ---------\n");
				PrintRRdata(&tempBuf, recBuf, nAdditional, numOfBytes);
			}

			break;
		}
		else
		{
			endTime = getElapsedTime(time);
			printf(" timeout in %d ms\n", endTime);
		}
	}

}

void GracefulExit() {
	WSACleanup();
	exit(1);
}
