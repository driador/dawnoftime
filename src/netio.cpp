/**************************************************************************/
// netio.cpp - network related functions, Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "network.h"
#define __SEE_NETIO_INTERNAL_STRUCTURES__ // we get some extra structs from netio.h
#include "include.h"
#include "nanny.h"
#include "comm.h"

//#define BIND_IPV4_BEFORE_IPV6 // with this set, ipv4 addresses will be bound before ipv6
/**************************************************************************/
void process_input(connection_data *c); // in comm.cpp
bool websrv_process_input(connection_data *c); // in websrv.cpp
void websrv_process_output(connection_data *c); // in websrv.cpp
void greet_http(connection_data *c); // in websrv.cpp
/**************************************************************************/
static fd_set fd_ipv4_in;
static fd_set fd_ipv4_out;
static fd_set fd_ipv4_exception;
static fd_set fd_ipv6_in;
static fd_set fd_ipv6_out;
static fd_set fd_ipv6_exception;
/**************************************************************************/
struct fd_set_group_data{
	fd_set *in;
	fd_set *out;	
	fd_set *exception;
	bool used;
	int maxdesc;
};
/**************************************************************************/
fd_set_group_data fd_set_group[PROTOCOL_ALL];
/**************************************************************************/
void netio_init_fd_set_groups()
{
	fd_set_group[PROTOCOL_IPV6].in=&fd_ipv6_in;
	fd_set_group[PROTOCOL_IPV6].out=&fd_ipv6_out;
	fd_set_group[PROTOCOL_IPV6].exception=&fd_ipv6_exception;
	
	fd_set_group[PROTOCOL_IPV4].in=&fd_ipv4_in;
	fd_set_group[PROTOCOL_IPV4].out=&fd_ipv4_out;
	fd_set_group[PROTOCOL_IPV4].exception=&fd_ipv4_exception;

	fd_set_group[PROTOCOL_ALL].in=NULL;
	fd_set_group[PROTOCOL_ALL].out=NULL;
	fd_set_group[PROTOCOL_ALL].exception=NULL;		
}

/**************************************************************************/
char listen_on_source_text[MSL];
bool listen_on_source_text_set=false;

/**************************************************************************/
// visual debug variables - in comm.cpp
extern char *visual_debug_next_connection_autoon_ip;
extern int visual_debug_next_connection_column_width;
extern bool visual_debug_next_connection_hexoutput;

void flush_cached_write_to_buffer(connection_data *c);
/**************************************************************************/
_contype_lookup_types CONTYPE_table[]={ 
	{"telnet", CONTYPE_TELNET}, 
	{"http", CONTYPE_HTTP},
	{"irc", CONTYPE_IRC}, 
	{"mudftp", CONTYPE_MUDFTP},
	{"", CONTYPE_UNSET}
};

/**************************************************************************/
#define MAX_LISTENING_SOCKET_EXCEPTION 50 // above this, the socket gets ignored
/**************************************************************************/
extern bool hotreboot_in_progress;
/**************************************************************************/
char current_listen_address[MSL];
bool listening_on_ipv4_addresses;
bool listening_on_ipv6_addresses;

/**************************************************************************/
listen_on_type *listen_on_first=NULL;
listen_on_type *listen_on_last=NULL;
/**************************************************************************/
void listen_on_add(char *bind_address, PROTOCOL_TYPE protocol, int port_offset, int port, CONNECTION_TYPE contype)
{
	listen_on_type *node=NULL;

	if(protocol==PROTOCOL_ALL){
		// The only time we can get a PROTOCOL_ALL is if the bind_address was 
		// if we got an empty string for the host.  So we can force manual 
		// binding to the any address on each protocol stack individually.
#ifdef BIND_IPV4_BEFORE_IPV6
		listen_on_add("0.0.0.0", PROTOCOL_IPV4, port_offset, port, contype);
#endif
		listen_on_add("::", PROTOCOL_IPV6, port_offset, port, contype);
#ifndef BIND_IPV4_BEFORE_IPV6
		listen_on_add("0.0.0.0", PROTOCOL_IPV4, port_offset, port, contype);
#endif
		return;
	}

	node=new listen_on_type;
	node->listening=false;
	node->psz_bind_address=str_dup(bind_address);
	node->psz_bound_pair=str_dup("unknown_at_this_stage");

	if(protocol!=PROTOCOL_IPV4 && protocol!=PROTOCOL_IPV6){
		logf("unexpected protocol type %d!", protocol);
		do_abort();
	}
	node->protocol=protocol;

	node->parsed_port_offset=port_offset;
	node->parsed_port=port;
	node->contype=contype;
	node->next=NULL;

	// append the new node to the linked list of listen_on bindings
	if(listen_on_last){
		listen_on_last->next=node;
		listen_on_last=node;
	}else{
		listen_on_first=node;
		listen_on_last=node;
	}
}

/**************************************************************************/
/*
//
// Structure used in getaddrinfo() call.
//
typedef struct addrinfo {
    int ai_flags;              // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST.
    int ai_family;             // PF_xxx.
    int ai_socktype;           // SOCK_xxx.
    int ai_protocol;           // 0 or IPPROTO_xxx for IPv4 and IPv6.
    size_t ai_addrlen;         // Length of ai_addr.
    char *ai_canonname;        // Canonical name for nodename.
    struct sockaddr *ai_addr;  // Binary address.
    struct addrinfo *ai_next;  // Next structure in linked list.
} ADDRINFO, FAR * LPADDRINFO;

//
// Flags used in "hints" argument to getaddrinfo().
//
#define AI_PASSIVE     0x1  // Socket address will be used in bind() call.
#define AI_CANONNAME   0x2  // Return canonical name in first ai_canonname.
#define AI_NUMERICHOST 0x4  // Nodename must be a numeric address string.
*/

/**************************************************************************/
void startup_exit()
{
	logf("exiting due to startup errors");
	exit_error( 1 , "startup_exit", "exiting due to netio startup errors");
}
/**************************************************************************/
CONNECTION_TYPE loparse_connection_type_lookup( char * contype)
{
	for(int i=0; !IS_NULLSTR(CONTYPE_table[i].name); i++){
		if(!str_cmp(contype, CONTYPE_table[i].name)){
			return CONTYPE_table[i].contype_enum;
		}		
	}
	return CONTYPE_UNSET;
}

/**************************************************************************/
// protocol_prefix = ( "telnet" | "http" | "irc" | "mudftp" ) "://" | ""
char *loparse_connection_type_prefix( char * connection_type_prefix, CONNECTION_TYPE &contype)
{
	contype=CONTYPE_TELNET; // default to telnet

	char *pstr=strstr(connection_type_prefix, "://");

	if(!IS_NULLSTR(pstr)){
		*pstr='\0';

		// lookup the protocol specified
		contype=loparse_connection_type_lookup(connection_type_prefix);

		if(contype==CONTYPE_UNSET){
			logf("Unrecognised connection_type prefix '%s://' in listen_on setting.",
				connection_type_prefix);

			logf("-");
			logf("Valid connection_type_prefix's for the listen_on setting include:");
			for(int i=0; !IS_NULLSTR(CONTYPE_table[i].name); i++){
				logf("  '%s://'", CONTYPE_table[i].name);				
			}
			logf("-");
			startup_exit();
		}
		
		return pstr+3; // skip over what was "://"
	}
	return connection_type_prefix;

}
/**************************************************************************/
// port_suffix = ( ":" [ ( "+" | "-" ) ] followed by a number in the range 1024 -> 65535 ) | ""
// return the port number
int loparse_port_suffix(char * port_suffix, int &port_offset)
{
	char *pstr_end=port_suffix + str_len(port_suffix)-1;
	
	// assume there is no port offset
	port_offset=0;

	// loop from the end backwards, until we either
	// we keep going backwards until reaching the start
	// or we get something that isn't a digit
	while(pstr_end>=port_suffix && is_digit(*pstr_end)){
		pstr_end--;		
	}

	// check if we have a sign bit, if so, skip back another
	if(*pstr_end=='-' || *pstr_end=='+'){
		pstr_end--;
	}

	if(pstr_end<port_suffix){
		// we rewound past the start before finding something
		return 0; // no port_suffix specified
	}

	if(*pstr_end!=':'){ // we aren't dealing with a port_suffix
		return 0; // no port_suffix specified
	}

	*pstr_end='\0'; // terminate what comes before the ':'
	pstr_end++;

	if(*pstr_end=='-'){
		port_offset=-1;
		pstr_end++;
	}else if(*pstr_end=='+'){
		port_offset=1;
		pstr_end++;
	}

	int value=atoi(pstr_end);
	
	if(value<0 || value>65535){
		logf("Invalid port_suffix %d specified in listen_on setting, must be in range 0->65535.", value);
		logf("listen_address text = '%s'", current_listen_address);
		startup_exit();
	}

	return value;
}


/**************************************************************************/
//    bind_address = bind_ipv4_address | "[" bind_ipv6_address "]" | ""
//    bind_ipv4_address = a valid ipv4 address | "0.0.0.0"
//    bind_ipv6_address = a valid ipv6 address | ""
char *loparse_bind_address(char * bind_address, PROTOCOL_TYPE &protocol)
{
	if(IS_NULLSTR(bind_address)){ 
		// no address specified
		protocol=PROTOCOL_ALL; // bind to all IPv4 and IPv6 addresses
				// this is implemented as two separate bindings by 
				// created by loparse_listen_address()
		*bind_address='\0';
		return bind_address; 
	}

	if(*bind_address=='[' && bind_address[str_len(bind_address)-1]==']'){ 
		// we have an ipv6 address
		protocol=PROTOCOL_IPV6;

		// trim off the trailing ']' 
		bind_address[str_len(bind_address)-1]='\0';
		
		// return the address, less the leading '['
		return bind_address+1;
	}

	// assume we have an ipv4 address
	protocol=PROTOCOL_IPV4;

	return bind_address;
}

/**************************************************************************/
// listen_address = connection_type_prefix  bind_address  port_suffix
void loparse_listen_address(char * listen_address)
{
	PROTOCOL_TYPE protocol;
	char *pstr_start_of_bind_address;
	int port;
	int port_offset; // -1 = negative offset, 0 = no offset, +1 = positive offset

	
	CONNECTION_TYPE contype;

	// parse off the the connection_type_prefix if one was specified
	pstr_start_of_bind_address = loparse_connection_type_prefix(listen_address, contype);

	// parse off the port_suffix if one was specified 
	// - easier for error reporting to take it off before parsing the bind_address
	//   even though it isn't as clean from a parsing perspective
	port= loparse_port_suffix(pstr_start_of_bind_address, port_offset);

	// parse the address
	pstr_start_of_bind_address=loparse_bind_address(pstr_start_of_bind_address, protocol);

	listen_on_add(pstr_start_of_bind_address, protocol, port_offset, port, contype);

}


/**************************************************************************/
// listen_addresses = listen_address  *( "," listen_address )
void loparse_listen_addresses(const char * listen_addresses)
{
	char buf[MSL];
	char *pstr_start_of_listen_address;
	char *pstr_comma;

	strcpy(buf, listen_addresses);
	pstr_start_of_listen_address=buf;

	// check if we have multiple addresses specified
	pstr_comma=strstr(pstr_start_of_listen_address, ",");
	while(!IS_NULLSTR(pstr_comma)){
		*pstr_comma='\0'; // terminate the previous listen_address

		// parse the previous listen_address
		loparse_listen_address(pstr_start_of_listen_address);

		// skip over where the comma was
		pstr_start_of_listen_address=pstr_comma+1;

		// see if there are any further addresses
		pstr_comma=strstr(pstr_start_of_listen_address, ",");
	}

	// parse the last address (or first (and last) if there is only one)
	loparse_listen_address(pstr_start_of_listen_address);

}

/**************************************************************************/
//
/*
// only one "listen_on" line is supported in gameset.txt 
// listen_address entries can be separated using a , as per the format specs below

  -------------------------------------------------------------------------------

  == Input Format Specification:

    listen_on = [ listen_addresses ]

    listen_addresses = listen_address  *( "," listen_address )

    listen_address = connection_type_prefix  bind_address  port_suffix

    connection_type_prefix = ( "telnet" | "http" | "irc" | "mudftp" ) "://" | ""

    bind_address = bind_ipv4_address | "[" bind_ipv6_address "]" | ""

    bind_ipv4_address = a valid ipv4 address | "0.0.0.0"

    bind_ipv6_address = a valid ipv6 address | ""

    port_suffix = ( ":" [ ( "+" | "-" ) ] followed by a number in the range 1024 -> 65535 ) | ""

  -------------------------------------------------------------------------------

e.g. ":4077,telnet://:4000,http://[feb0:0:0:ffff::42]:4001,irc://192.168.0.1:4002,mudftp://[::1]:4003"

relates to:
1. telnet on port 4077 for all ipv4 & ipv6 addresses
2. telnet on port 4000 for all ipv4 & ipv6 addresses
3. http on port 4001 for the ipv6 address "feb0:0:0:ffff::42"
4. irc on port 4002 for the ipv4 address "192.168.0.1"
5. mudftp on port 4003 for the local loopback ipv6 address "::1"

notes:
- + or - in the port_suffix indicates an offset from the main port... the main port is
  defined as being the first telnet port used

- if no connection_type is specified, telnet is assumed

- if no address is specified, but a port is the mud will listen on all 
  ipv4 and ipv6 addresses for that given port
  e.g. "http://:4080"

- if the address "0.0.0.0" is specified, the mud will listen on all available
  ipv4 addresses for that given port.
  e.g. "http://0.0.0.0:4080"

- if no address is specified but empty []'s are provided along with a port, 
  the mud will listen on all ipv6 addresses for that given port.
  e.g. "http://[]:4080"
  Alternatively [::] can be used for all ipv6 addresses
*/

// listen_on = [ listen_addresses ]
void netio_parse_listen_on(const char * listen_on)
{
	char ltext[MSL];	

	// the listen_on input can be up to MSL in length	
	if(str_len(listen_on)>MSL){
		logf("listen_on text is %d characters - which is too long.", 
			str_len(listen_on));
		logf("listen_on text is '%s'", listen_on);
		do_abort();
	}

	strcpy(listen_on_source_text, trim_string(listen_on));
	listen_on_source_text_set=true;

	logf("parsing listen_on text '%s'", listen_on_source_text);

	strcpy(ltext, trim_string(listen_on));	
	loparse_listen_addresses(ltext);
}
/**************************************************************************/
// dump the parsed listen on database
void netio_parsed_listen_on_output()
{
	listen_on_type *node;
	logf("Parsed listen_on setting, after mainport %d has been applied:", mainport);
	for(node=listen_on_first; node; node=node->next){
		assert(node->protocol==PROTOCOL_IPV6 || node->protocol==PROTOCOL_IPV4);
		logf("   %-6s port: %d, %s address: %s",
			CONTYPE_table[node->contype].name,
			node->bind_port,
			node->protocol==PROTOCOL_IPV6?"ipv6":"ipv4",
			node->psz_bind_address);
	}
}

/**************************************************************************/
// dump the successfully binded listen on database
void netio_binded_listen_on_output()
{
	listen_on_type *node;
	log_string("The mud is waiting for connections on the following addresses:");
	for(node=listen_on_first; node; node=node->next){
		assert(node->protocol==PROTOCOL_IPV6 || node->protocol==PROTOCOL_IPV4);
		if(node->listening){
			logf("  s%d>  %-6s port: %d, %s address: %s",
				(int)node->listening_socket,
				CONTYPE_table[node->contype].name,
				node->bind_port,
				node->protocol==PROTOCOL_IPV6?"ipv6":"ipv4",
				node->psz_bind_address);
		}
	}
}

/**************************************************************************/
// dump the successfully binded listen on database
char *netio_return_binded_sockets()
{
	static char result[MSL];
	char temp[MSL];

	listen_on_type *node;
	result[0]='\0';
	for(node=listen_on_first; node; node=node->next){
		assert(node->protocol==PROTOCOL_IPV6 || node->protocol==PROTOCOL_IPV4);
		if(node->listening){
			sprintf(temp, "[contype='%s' protocol='%s' address='%s' port='%d']",
				CONTYPE_table[node->contype].name,
				node->protocol==PROTOCOL_IPV6?"ipv6":"ipv4",
				node->psz_bind_address,
				node->bind_port);
		}
		if((str_len(result) + str_len(temp))< MSL+1){
			strcat(result, temp);
		}
	}
	return result;
}

/**************************************************************************/
char *netio_resolve_bound_pair_values(const struct sockaddr* sa, socklen_t salen, int &port)
{
	static int i;
    static char buf[4][MSL*3];
	++i%=4;

// update the bound pair text, for what we are about to bind to
#ifdef IPV6_SUPPORT_ENABLED
	char sz_host[NI_MAXHOST+1], sz_serv[NI_MAXSERV+1];
	int hostlen = NI_MAXHOST, servlen = NI_MAXSERV, result;

	result = getnameinfo( sa, salen, sz_host, hostlen, sz_serv, servlen,
					  NI_NUMERICHOST | NI_NUMERICSERV);
	if(result!=0){
		sprintf(buf[i],"netio_resolve_bound_pair()-error_%d_while_converting_address", result);
		port=0;
	}else{
		strcpy(buf[i], sz_host);
		port=atoi(sz_serv);
	}
#else
	const struct sockaddr_in *sa2=(struct sockaddr_in*)sa;
	strcpy(buf[i],inet_ntoa(sa2->sin_addr));
	port=ntohs(sa2->sin_port);
#endif
	return buf[i];
}
/**************************************************************************/
// return text based on the type of ip address
// ipv4 address = textip:portnum
// ipv6 address = [textip]:portnum
char *netio_format_tcp_pair(const char *textip, int port)
{
	static int i;
    static char buf[4][MSL*3];
	++i%=4;
	if(count_char(textip,':')){
		// has one or more colons in the textip, assume ipv6 address
		sprintf(buf[i], "[%s]:%d", textip, port);
	}else{
		// doesn't have a :, therefore assume ipv4 address
		sprintf(buf[i], "%s:%d", textip, port);
	}
	return buf[i];
}
/**************************************************************************/
char *netio_resolve_bound_pair(const struct sockaddr* sa, socklen_t salen)
{
	int port;
	char *textip=netio_resolve_bound_pair_values(sa, salen, port);
	return netio_format_tcp_pair(textip, port);
}
/**************************************************************************/
// bind all the listen_on connections
void netio_bind_connections()
{
	int result;
	listen_on_type *node;
	bool successful_telnet_bind=false;

	int ai_family=AF_INET;
	int ai_socktype=SOCK_STREAM;

	for(node=listen_on_first; node; node=node->next)
	{
		int option_on = 1;
		dawn_socket listening_socket=0;
		node->listening_socket=0;
		node->listening_exception_count=0;

		if(node->protocol==PROTOCOL_IPV4){
			ai_family=AF_INET;
		}else if(node->protocol==PROTOCOL_IPV6){

#ifdef IPV6_SUPPORT_ENABLED
			ai_family=AF_INET6;
#else
			logf("ipv6 support not enabled, ignoring: '%s://[%s]:%s'",
				CONTYPE_table[node->contype].name,		
				node->psz_bind_address,
				FORMATF("%s%d",	
					node->parsed_port_offset==0?" ":(node->parsed_port_offset==1?"+":"-"), 
					node->parsed_port
					)
				);
			continue;
#endif

		}else{
			logf("unrecognised protocol value %d!", node->protocol);
			do_abort();
		}


#ifdef IPV6_SUPPORT_ENABLED
		struct addrinfo hints, *res;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_flags    = AI_PASSIVE; // we want to use the result to bind
		if(!IS_NULLSTR(node->psz_bind_address)){
			hints.ai_flags|=AI_NUMERICHOST; // we don't want to support host names for binding
		}
		hints.ai_family   = ai_family;
		hints.ai_socktype = ai_socktype; 

		res=NULL;
		int r = getaddrinfo(IS_NULLSTR(node->psz_bind_address)?NULL:node->psz_bind_address, // hostname
							FORMATF("%d",node->bind_port),  // server
							&hints, 
							&res);

		if (r) {
#ifdef WIN32
			if(r==WSAHOST_NOT_FOUND){
				logf("getaddrinfo error %d - couldn't convert '%s' to a valid ip address.", 
					r, node->psz_bind_address);
			}else if(r==WSAEAFNOSUPPORT && node->protocol==PROTOCOL_IPV6){
				logf("\n"
					"getaddrinfo error %d - couldn't convert '%s' to a valid ip address\n"
					" - getaddrinfo reported this system has no support for the AF_INET6 address family.\n"
					"   This error message is normal if this system doesn't support ipv6, as it\n"
					"   was an ipv6 address getaddrinfo() was asked to convert.", 
					r, node->psz_bind_address);
			}else{
				socket_error(FORMATF("netio_bind_connections(): getaddrinfo(%s) error %d", 
					IS_NULLSTR(node->psz_bind_address)?"NULL":node->psz_bind_address, r));
			}
#else
			{
				logf("netio_bind_connections(): getaddrinfo(%s) error %d - '%s'", 
					IS_NULLSTR(node->psz_bind_address)?"NULL":node->psz_bind_address, r, gai_strerror(r));			
			}
#endif
			if(res){
				freeaddrinfo(res);
				res=NULL;
			}
			continue;
		}

#else // !IPV6_SUPPORT_ENABLED		
		ipv4only_addrinfo *res;
		res=new ipv4only_addrinfo;
		res->ai_next=NULL;

		res->ai_family=ai_family;
		res->ai_socktype=ai_socktype;
		res->ai_protocol=0;
		struct sockaddr_in sa;
		memset(&sa, 0, sizeof(sa));
		sa.sin_family = ai_family;
		sa.sin_port     = htons( (u_short) node->bind_port );
		if(IS_NULLSTR(node->psz_bind_address)){
			sa.sin_addr.s_addr= INADDR_ANY;
		}else{
			sa.sin_addr.s_addr= inet_addr( node->psz_bind_address);
			if(sa.sin_addr.s_addr==INADDR_NONE){ 
				// we had an invalid address supplied
				// technically INADDR_NONE can be returned by the valid ip 255.255.255.255
				// but 255.255.255.255 isn't useful as a bind address, so we assume the 
				// input node->psz_bind_address was invalid.
				logf("inet_addr() couldn't convert '%s' to an ipv4 address - ignored.", 
					node->psz_bind_address);
				replace_string(node->psz_bound_pair, 
						FORMATF("invalid-'%s:%d'",node->psz_bind_address,node->bind_port));
				continue;
			}
		}

		// update the bound pair text
		replace_string(node->psz_bound_pair, 
				FORMATF("%s:%d",inet_ntoa(sa.sin_addr),node->bind_port));

		res->ai_addr=(struct sockaddr *) &sa;
		res->ai_addrlen=sizeof(sa);
#endif	// ifdef IPV6_SUPPORT_ENABLED

		// loop thru till we successfully bind
		for(;res; res=res->ai_next){
			listening_socket = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
			if( listening_socket == dawn_socket_INVALID_SOCKET){
				socket_error( "netio_bind_connections(): socket() call" );
				continue;
			}

			// set the socket so it doesn't block
			#ifdef unix
				#if !defined(FNDELAY)
				#define FNDELAY O_NDELAY
				#endif
				if(fcntl(listening_socket, F_SETFL, FNDELAY ) == -1)
				{
					socket_error( "connection_allocate: fcntl: FNDELAY" );
					continue;
				}
			#endif

			#ifdef WIN32
			{
				// Set Socket to non blocking mode
				unsigned long blockmode = 1; // no blocking 
				if(ioctlsocket(listening_socket, FIONBIO, &blockmode)== SOCKET_ERROR)    
				{        
					bugf("ioctlsocket: error setting new socket to nonblocking, "
						"WSAGetLastError=%d", WSAGetLastError());        
					continue;    
				}  
			}
			#endif

			// Allow local port reuse while in TIME_WAIT state
			if( setsockopt( listening_socket, SOL_SOCKET, SO_REUSEADDR,
				(char *) &option_on, sizeof(option_on) ) < 0 )
			{
				socket_error( "netio_bind_connections(): setting SO_REUSEADDR" );
				closesocket(listening_socket);
				continue;
			}

			// update the bound pair text, for what we are about to bind to
			replace_string(node->psz_bound_pair, netio_resolve_bound_pair(res->ai_addr, (socklen_t)res->ai_addrlen));

			// now try to bind to it
			result=bind( listening_socket, res->ai_addr, (int)res->ai_addrlen);
			if( result != 0 )
			{		
				bugf("netio_bind_connections(): bind %s - error %d (%s)",
					node->psz_bound_pair, errno, strerror( errno));
				socket_error( FORMATF("Init socket: bind '%s'", node->psz_bound_pair) );
				closesocket(listening_socket);

				logf("Failed to bind '%s' to '%s'.", 
					CONTYPE_table[node->contype].name,
					node->psz_bound_pair);
				{ // update the bound pair text
					char tbuf[MIL];
					sprintf(tbuf, "failed_bind-%s", node->psz_bound_pair);
					replace_string(node->psz_bound_pair, tbuf);
				}
				continue;
			}
			break;
		}

		if(!res){ // we didn't successfully bind, record the last failed to bind
			logf("Due to bind failure, ignoring listen_on entry: '%s://%s:%s'",
				CONTYPE_table[node->contype].name,
				node->protocol==PROTOCOL_IPV6?FORMATF("[%s]",node->psz_bind_address):node->psz_bind_address,
				FORMATF("%s%d",	node->parsed_port_offset==0?" ":(node->parsed_port_offset==1?"+":"-"), node->parsed_port));
			continue;
		}

		if(node->contype==CONTYPE_TELNET){
			successful_telnet_bind=true;
		}

		if( listen( listening_socket, 100 ) != 0 )
		{
			socket_error("netio_bind_connections: listen");
			closesocket(listening_socket);
			exit_error( 1 , "netio_bind_connections", "listen failed");
		}

		node->listening_socket=listening_socket;
		node->listening=true;
		if(node->protocol==PROTOCOL_IPV4){
			listening_on_ipv4_addresses=true;
		}else if(node->protocol==PROTOCOL_IPV6){
			listening_on_ipv6_addresses=true;
		}

		logf("socket %d listening as %s on %s.",
			node->listening_socket,
			CONTYPE_table[node->contype].name,
			node->psz_bound_pair);

#ifdef IPV6_SUPPORT_ENABLED
		if(res){
			freeaddrinfo(res);
			res=NULL;
		}
#else
		delete res;
#endif

	} // end of node loop

	// check we successfully bound at least one telnet port
	if(!successful_telnet_bind){
		logf("The bootup process, failed to bind any telnet connections for listening... "
			"mud is considered unusable this state, aborting startup.");
		startup_exit();
	}
}

/**************************************************************************/


/**************************************************************************/
void netio_allocate_bind_ports(int main_port)
{
	listen_on_type *node;
	bool out_of_range_ports=false;

	for(node=listen_on_first; node; node=node->next)
	{
		if(node->parsed_port_offset){
			node->bind_port=main_port + (node->parsed_port * node->parsed_port_offset);
		}else if(node->parsed_port){
			node->bind_port=node->parsed_port;
		}else{
			node->bind_port=main_port;
		}

		if(node->bind_port<1024 || node->bind_port>65535){
			if(!out_of_range_ports){
				logf("Allocating binding ports for listen_on entries... main_port value=%d.", main_port);
			}
			logf("Resulting bind port is %d for listen_on entry: '%s://[%s]:%s'",
				node->bind_port,
				CONTYPE_table[node->contype].name,
				node->psz_bind_address,
				FORMATF("%s%d",	node->parsed_port_offset==0?" ":(node->parsed_port_offset==1?"+":"-"), node->parsed_port));			
			out_of_range_ports=true;
		}
	}

	if(out_of_range_ports){
		logf("\n"
			"vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n"
			"One or more of the binding port values were outside the range 1024-65535.\n"
			"The mud will not start with out of range port values... aborting startup.\n"
			"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
		startup_exit();
	}

}

/**************************************************************************/
void greet_new_connection(connection_data *d);
/**************************************************************************/
void netio_init_connection( int new_connection, PROTOCOL_TYPE protocol, CONNECTION_TYPE contype,
						   char *local_ip, int local_port, char *remote_ip, int remote_port)
{
    connection_data *c;
	bool resolve_address=true;

    // add the new connection to the list of existing connections
    c = connection_allocate();  // connection_allocate handles all allocation
    c->connected_socket = new_connection;

	c->protocol=protocol;
	c->contype=contype; // telnet/http/irc/mudftp etc
	replace_string(c->local_ip,local_ip);
	c->local_port=local_port;	
	replace_string(c->local_tcp_pair, netio_format_tcp_pair(c->local_ip, c->local_port));
	replace_string(c->remote_ip,remote_ip);
	c->remote_port=remote_port;
	replace_string(c->remote_tcp_pair, netio_format_tcp_pair(c->remote_ip, c->remote_port));
	replace_string(c->remote_hostname,FORMATF("unresolved'%s'", remote_ip));

 	if(!IS_NULLSTR(game_settings->no_resolve_ip_text) 
		&& !strcmp(c->remote_ip, game_settings->no_resolve_ip_text))
	{
		resolve_address=false;
	}

	// don't bother doing dns lookups on http requests
	if(c->contype==CONTYPE_HTTP){
		resolve_address=false;
		replace_string(c->remote_hostname,FORMATF("webrequest'%s'", remote_ip));
	}

	if(resolve_address){
		if(resolver_running || GAMESETTING(GAMESET_PERFORM_LOCAL_DNS_LOOKUPS)){
			// use resolver to do ip -> hostname conversion
			// or perform local resolution if the resolver isn't working
			resolver_query( c );
		}
    }

	c->ident_raw_result=str_dup("");
	c->ident_username=str_dup("");

	if(check_connection_ban(c)){
		return;
	}

	// automatically turn on visual debugging if required
	if( !IS_NULLSTR(visual_debug_next_connection_autoon_ip) && 
		!strcmp(visual_debug_next_connection_autoon_ip, c->remote_ip))
	{
		logf("Visual Debug automatically turned on for socket %d (%s)", 
			c->connected_socket, c->remote_tcp_pair);
		c->visual_debugging_enabled=true;
		c->visual_debug_flush_before_prompt=true;
		c->visual_debug_strip_prompt=false;
		c->visual_debug_column_width=visual_debug_next_connection_column_width;
		c->visual_debug_hexoutput=visual_debug_next_connection_hexoutput;
		replace_string(visual_debug_next_connection_autoon_ip,"");
	}

    // add to the connection list
    c->next             = connection_list;
    connection_list		= c;

	// by default connections start in CON_DETECT_CLIENT_SETTINGS (35)

	switch(c->contype){
		case CONTYPE_TELNET:
		default:{
			// we deal with telnet connections below
			}
			break; 
			
		case CONTYPE_HTTP:{
				greet_http(c);
			    c->connected_state = CON_WEB_REQUEST;
				return;
			}
			break;


		case CONTYPE_IRC:{
			// IRC connections skip CON_DETECT_CLIENT_SETTINGS
			// and start at CON_GET_NAME
			greet_new_connection(c); // send them the greeting page 
			c->connected_state = CON_GET_NAME;
			return;
		}

		case CONTYPE_MUDFTP:	{
			// mudftp connections go to CON_FTP_AUTH
			greet_ftp(c);
			c->connected_state = CON_FTP_AUTH;
			return;
		}
	}
	
	assert(contype==CONTYPE_TELNET);

	// Connections that get this far are going thru 
	// the CON_DETECT_CLIENT_SETTINGS process
	{
		bool t=c->fcommand;
		c->fcommand=true;
		// - advertise to them the telnet options we support
		c->advertise_supported_telnet_options();
		
#ifdef SHOW_CLIENT_DETECTION
		// - tell them we are detecting their client settings
		write_to_buffer( c, "checking for mxp and mccp support.", 0);
#endif 
		c->fcommand=t;
	}
	
	return;
}
/**************************************************************************/
// initialise a new connection and route it to the correct handler
// return true if there was a socket waiting to be accepted
bool netio_accept_new_connection(listen_on_type *bound_socket)
{
	char sz_remote_pair[MSL];
	char sz_local_pair[MSL];

	dawn_socket nc; // new connection
#ifdef IPV6_SUPPORT_ENABLED	
	struct sockaddr_storage socket_address;
#else
    struct sockaddr_in socket_address;
#endif
	socklen_t sockaddrlen=sizeof(socket_address);
	
	nc = accept( bound_socket->listening_socket, (struct sockaddr *)&socket_address, &sockaddrlen);

    if( nc == dawn_socket_INVALID_SOCKET)
    {
#ifdef WIN32
		if(WSAGetLastError() == WSAEWOULDBLOCK)
#else
		if(errno==EWOULDBLOCK)
#endif
		{
			return false;
		}
		socket_error( "init_descriptor: accept" );
		return false;
    }
	

	// find out the remote ip and port details
	char *remote_ip;
	int remote_port;
	remote_ip=netio_resolve_bound_pair_values((const struct sockaddr*)&socket_address, sockaddrlen, remote_port);
	strcpy(sz_remote_pair,netio_format_tcp_pair( remote_ip, remote_port));

	// now find out the local ip and port details
	char *local_ip;
	int local_port;

	if(getsockname(nc, (struct sockaddr*)&socket_address, &sockaddrlen)!=0){
		socket_error("netio_accept_new_connection: getsockname returned error.");
		local_ip="netio_accept_new_connection()getsockname_returned_error";
		strcpy(sz_local_pair, bound_socket->psz_bound_pair);
	}else{
		local_ip=netio_resolve_bound_pair_values((const struct sockaddr*)&socket_address, sockaddrlen, local_port);
		strcpy(sz_local_pair,netio_format_tcp_pair( local_ip, local_port));
	}
	
	logf("incoming %s connection to '%s' from '%s'.", 
		CONTYPE_table[bound_socket->contype].name,		
		sz_local_pair, sz_remote_pair);

	// make the new connection socket non blocking
#ifdef unix
	#if !defined(FNDELAY)
	#define FNDELAY O_NDELAY
	#endif
    if(fcntl(nc, F_SETFL, FNDELAY ) == -1)
    {
		socket_error( "netio_accept_new_connection: fcntl() FNDELAY" );
		return true;
    }
#endif
#ifdef WIN32
{
	// Set Socket to non blocking mode
	unsigned long blockmode = 1; // no blocking 
	if(ioctlsocket(nc, FIONBIO, &blockmode)== SOCKET_ERROR)    
	{        
		bugf("netio_accept_new_connection: ioctlsocket() error setting new socket to nonblocking, "
			"WSAGetLastError=%d", WSAGetLastError());        
		return true;
	}
}
#endif

	netio_init_connection( (int)nc, bound_socket->protocol,
		bound_socket->contype, local_ip, local_port, remote_ip, remote_port);

/*
	// test banner code
	if(bound_socket->contype!=CONTYPE_HTTP){
		{
			char banner[MSL];
			sprintf(banner, "-= dawn site running version %s - %s=-\r\n"
				"%s"
				"##new %s connection to '%s' from '%s'",
				DAWN_RELEASE_VERSION, DAWN_RELEASE_DATE, get_compile_time(false),
				CONTYPE_table[bound_socket->contype].name, sz_local_pair, sz_remote_pair);				

			send(nc, banner, str_len(banner), 0);
		}
	}
*/
	return true;
}

/**************************************************************************/
void netio_check_for_and_accept_new_connections( )
{
	fd_set fd_in, fd_out, fd_exception;
	static struct timeval null_time;
	listen_on_type *node;
	int maxdesc=0;

	// don't accept new connections during a hotreboot
	if(hotreboot_in_progress){
		return;
	}

	// because select doesn't seem to support checking for both ipv4 and ipv6 sockets
	// in a single call for winsock, we have to loop thru the sets of addresses 
	// separately
	for(int i=0; i<2; i++){
		PROTOCOL_TYPE pt= (i==0?PROTOCOL_IPV4:PROTOCOL_IPV6);
		if(pt==PROTOCOL_IPV4 && !listening_on_ipv4_addresses){
			continue;
		}
		if(pt==PROTOCOL_IPV6 && !listening_on_ipv6_addresses){
			continue;
		}

		// initialise the file descriptor sets
		FD_ZERO( &fd_in );
		FD_ZERO( &fd_out );
		FD_ZERO( &fd_exception);

		for(node=listen_on_first; node; node=node->next)
		{
			if(!node->listening || node->protocol!=pt){
				continue;
			}

			if(node->listening_exception_count>MAX_LISTENING_SOCKET_EXCEPTION){
				continue;
			}

			maxdesc = UMAX( maxdesc, (int)node->listening_socket);	
			FD_SET( (dawn_socket)node->listening_socket, &fd_in );		
			FD_SET( (dawn_socket)node->listening_socket, &fd_exception);
		}

		if( select( maxdesc+1, &fd_in, NULL, &fd_exception, &null_time ) < 0 )
		{
			socket_error( "netio_check_for_and_accept_new_connections: select()" );
			do_abort();
			exit_error( 1 , "netio_check_for_and_accept_new_connections", "select error");
		}

		for(node=listen_on_first; node; node=node->next)
		{
			if( FD_ISSET( node->listening_socket, &fd_exception ) ){
				node->listening_exception_count++;
				bugf( "Exception raised on controlling socket %d (%s), exception count at %d.", 
					node->listening_socket, 
					node->psz_bound_pair,
					node->listening_exception_count);
				if(node->listening_exception_count>MAX_LISTENING_SOCKET_EXCEPTION){
					bugf("Exception count exceeded maximum for socket - disabling it.");
					// the code above effectively disables it
				}				
				continue;
			}

			if( FD_ISSET( node->listening_socket, &fd_in ) ){
				int concount=0;
				while(netio_accept_new_connection(node) && ++concount<5){
					// loop thru until up to 4 connections are accepted
				}
			}
		}
	}
}


/**************************************************************************/
// poll all the connections for input, output and exceptions
void netio_poll_connections()
{
	static struct timeval null_time;
	connection_data *c;	
	int protocol;
			
	// initialise the netio_poll file descriptor sets
	for(protocol=PROTOCOL_IPV6; protocol<PROTOCOL_ALL; protocol++){
		FD_ZERO( fd_set_group[protocol].in);
		FD_ZERO( fd_set_group[protocol].out);
		FD_ZERO( fd_set_group[protocol].exception);	
		fd_set_group[protocol].used=false;
		fd_set_group[protocol].maxdesc=0;
	}

	if(!connection_list){ // we only want to continue if we have something to poll
		return;
	}
	// populate the file descriptor sets with the list of connected sockets
	for(c=connection_list; c; c=c->next){
		assert(c->protocol==PROTOCOL_IPV6 || c->protocol==PROTOCOL_IPV4);
	
		fd_set_group[c->protocol].maxdesc= UMAX( fd_set_group[c->protocol].maxdesc, (int)c->connected_socket);
		FD_SET( (dawn_socket)c->connected_socket, fd_set_group[c->protocol].in);
		FD_SET( (dawn_socket)c->connected_socket, fd_set_group[c->protocol].out);
		FD_SET( (dawn_socket)c->connected_socket, fd_set_group[c->protocol].exception);
		fd_set_group[c->protocol].used=true;
	}

	// find out what is doing what
	for(protocol=PROTOCOL_IPV6; protocol<PROTOCOL_ALL; protocol++){
		if(fd_set_group[protocol].used){
			
			if( select( fd_set_group[protocol].maxdesc+1, 
						fd_set_group[protocol].in, 
						fd_set_group[protocol].out, 
						fd_set_group[protocol].exception, 
						&null_time ) < 0 )
			{
				socket_error( FORMATF("netio_poll_all_connections: select(), protocol=%d", protocol));
				do_abort();
				exit_error( 1 , "netio_poll_all_connections", "select error");
			}
		}

	}

	// remaining processing is done by - called from game_loop()
	//netio_process_exceptions_from_polled_connections()
	//netio_process_input_from_polled_connections()
	//netio_process_output_from_polled_connections()

}
/**************************************************************************/
void netio_process_exceptions_from_polled_connections()
{
	// Disconnect the connections with raised exceptions 
	// or have been idle, then check for input.
	for ( connection_data *c = connection_list; c; c = c_next )
	{
		c_next = c->next;   

		if( FD_ISSET( c->connected_socket, fd_set_group[c->protocol].exception) )
		{
			FD_CLR( (unsigned int) c->connected_socket, fd_set_group[c->protocol].in);
			FD_CLR( (unsigned int) c->connected_socket, fd_set_group[c->protocol].out);
			if( c->connected_state == CON_PLAYING && CH(c)){
				save_char_obj( CH(c) );
			}
			c->outtop       = 0;
			connection_close( c );
		}
	}

}
/**************************************************************************/
void netio_process_input_from_polled_connections()
{
	// loop thru all connections processing their input
	for ( connection_data *c = connection_list; c; c = c_next )
	{
		c_next      = c->next;
		c->fcommand = false;
		
		if(c->connected_state==CON_RESOLVE_IP){
			nanny_resolve_ip(c, "");
		}
		
		if(c->connected_state==CON_DETECT_CLIENT_SETTINGS){
			nanny_detect_client_settings(c, "");
		}
		
		if(c->connected_state==CON_WEB_REQUEST){
			if(!websrv_process_input(c)){
				continue;
			}
		}

		if( FD_ISSET( c->connected_socket, fd_set_group[c->protocol].in) ){
			if( c->character )
			{
				c->character->timer = 0;
				c->character->idle = 0;
				if(c->original){
					c->original->idle = 0;
				}
			}
			
			// check for link dead players
			if( !read_from_connection( c ) )
			{
				FD_CLR( (dawn_socket)c->connected_socket, fd_set_group[c->protocol].out);
				if( CH(c)&& CH(c)->level> 1){
					save_char_obj(CH(c));
				}
				c->outtop   = 0;
				connection_close(c);
				continue;
			}
		}
		
		// get input - upto 2 times if they are speed walking
		if( c->speedwalk_buf ){
			for(int speed=0; speed<2; speed++){
				if(c->speedwalk_buf){
					process_input(c);
				}else{
					break;
				}
			}
		}else{
			process_input(c);
		}
	}
}
/**************************************************************************/
void netio_process_output_from_polled_connections()
{
	// loop thru all connections processing their output
	for ( connection_data *c = connection_list; c; c = c_next )
	{
		c_next = c->next;
		
		flush_cached_write_to_buffer(c);
		
		if(FD_ISSET(c->connected_socket, fd_set_group[c->protocol].out))
		{
			if(c->contype==CONTYPE_HTTP ){
				if(c->web_request->inHeaderLen){
					websrv_process_output(c);
				}
			}else if( ( c->fcommand || c->outtop > 0 ) ){
				if( !process_output( c, true ) ){
					if( c->connected_state == CON_PLAYING && CH(c) ){
						save_char_obj( CH(c) );
					}
					c->outtop   = 0;
					connection_close( c );
				}
			}
		}
	}

}


/**************************************************************************/
void netio_close_all_binded_sockets()
{
	listen_on_type *node;
	for(node=listen_on_first; node; node=node->next){
		if(node->listening){
			closesocket(node->listening_socket);
		}
	}
}
/**************************************************************************/
/**************************************************************************/

