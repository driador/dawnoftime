/**************************************************************************/
// netio.h - defines related to the netio code
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef __NETIO_H
#define __NETIO_H
/**************************************************************************/
enum CONNECTION_TYPE { CONTYPE_TELNET, CONTYPE_HTTP, 
					CONTYPE_IRC, CONTYPE_MUDFTP, CONTYPE_UNSET};
/**************************************************************************/
// The order of the PROTOCOL_TYPE enum below MUST NOT be changed... 
// as other parts of the code makes assumptions about its configuration
enum PROTOCOL_TYPE { PROTOCOL_IPV6, PROTOCOL_IPV4, PROTOCOL_ALL}; 

char *netio_return_binded_sockets();
/**************************************************************************/
#ifdef __SEE_NETIO_INTERNAL_STRUCTURES__
// only seen netio.cpp, hreboot.cpp and dawnstat.cpp 
// __SEE_NETIO_INTERNAL_STRUCTURES__ is defined within these functions
struct listen_on_type 
{
	PROTOCOL_TYPE protocol;
	int parsed_port_offset;
	int parsed_port;		// the port number read in

	CONNECTION_TYPE contype; // telnet, http, irc, mudftp

	char *psz_bound_pair; // what we actually ended up binding to according to OS in "address:port" format
	char *psz_bind_address;
	int bind_port;	// the port we actually bind to	

	dawn_socket listening_socket;
	bool listening;
	int listening_exception_count;

	listen_on_type *next;
};	

extern listen_on_type *listen_on_first;
extern listen_on_type *listen_on_last;

struct _contype_lookup_types{
	char * name;
	CONNECTION_TYPE contype_enum;
};

extern _contype_lookup_types CONTYPE_table[];
CONNECTION_TYPE loparse_connection_type_lookup( char * contype);

/**************************************************************************/
#ifndef IPV6_SUPPORT_ENABLED
struct ipv4only_addrinfo { // based on addrinfo, used to make the code more portable
	int ai_family;             // PF_xxx.
	int ai_socktype;           // SOCK_xxx.
	int ai_protocol;           // 0 or IPPROTO_xxx for IPv4 and IPv6.
	int ai_addrlen;			   // Length of ai_addr.
	struct sockaddr *ai_addr;  // Binary address.
	struct ipv4only_addrinfo *ai_next;  // Next structure in linked list.
};
#endif


#endif // __SEE_NETIO_INTERNAL_STRUCTURES__

/**************************************************************************/

#endif // __NETIO_H

