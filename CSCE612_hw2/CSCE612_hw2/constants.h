//************************
// Constants from HW2 PDF
//************************


#pragma once

#define DNS_A			1			// Name -> IP
#define DNS_NS			2			// Name server
#define DNS_CNAME		5			// Canonical name
#define DNS_PTR			12			// IP -> name
#define DNS_HINFO		13			// Host info/SOA
#define DNS_MX			15			// Mail exchange
#define DNS_AXFR		252			// Request for zone transfer
#define DNS_ANY			255			// All records

#define DNS_INET		1			// Qeuery Class
#define MAX_DNS_LEN		512			// 512 bytes max
#define MAX_ATTEMPTS	3			// Maximum number of retransmissions

// Flags
#define DNS_QUERY		(0 << 15)	// 0 = query 
#define DNS_RESPONSE	(1 << 15)	// 1 = response

#define DNS_STDQUERY	(0 << 11)	// opcode - 4 bits

#define DNS_AA			(1 << 10)	// Authoritative answer
#define DNS_TC			(1 << 9)	// Truncated
#define DNS_RD			(1 << 8)	// Recursion desired
#define DNS_RA			(1 << 7)	// Recusion available

// Result Codes
#define DNS_OK			0			// Success
#define DNS_FORMAT		1			// Format error (unable to interpret)
#define DNS_SERVERFAIL	2			// Can't find authority nameserver
#define DNS_ERROR		3			// No DNS entry
#define DNS_NOTIMPL		4			// Not implemented
#define DNS_REFUSED		5			// Server refused the query

