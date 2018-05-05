/**************************************************************************/
// resolver.cpp 
// can manually compile with:  g++ -o resolver resolver.cpp
// The resolver binary should be in same directory as the dawn binary for
// dawn to take advantage of it.
/**************************************************************************/
// Please note: this should really be rewritten from scratch, but due to 
// timelines to release the codebase, is being released as is.
/**************************************************************************/
// some parameters 
#define DATE_IN_LOGS // if the date is in the log entires
#define RESOLVER_VERSION "1.5"
#define VERBOSE_LEVEL (0)
#define IDENT_CONNECT_TIMEOUT (15)
#define MSL (2048)
#define HSL (16384)

//#define RESOLVER_DEBUG_BRACKETS
/**************************************************************************/
#define __IN_RESOLVER_CPP__ // so resolver.h doesn't complain 
#include "resolver.h"

/**************************************************************************/
// caching stats
int iHostLookups=0;
int iHostLookupsFoundInCache=0;

// prototypes
void sleep_seconds(int seconds);
hnode *find_in_cache(sockaddr_type *socket_address, size_t address_length);
time_t current_time(void);
bool resolve_text_ip_into_address(sockaddr_type *socket_address,		
								size_t *address_length, char*pszAddress, 
								int *error_value, char *error_message);
hnode *lookup_host(sockaddr_type *socket_address, size_t address_length);

/**************************************************************************/
#ifdef WIN32
#define redirect_socket_error_text(r) get_socket_error_text(r) 
#else
#define redirect_socket_error_text(r) gai_strerror(r)
#endif
/**************************************************************************/
typedef const char *resolver_function(char *argument);
struct command_type{
	const char *name;
	const char *syntax;
	resolver_function *func;
};
/**************************************************************************/
const char *resolver_close(char *argument)
{
	resolver_logf( 0,"close command received - ending resolver");
	exit( 0 );
	return "";
}	
/**************************************************************************/
const char *resolver_stats(char *argument)
{	
	static char result[MSL];
	snprintf(result, sizeof(result), "HostLookups: %d, HostLookupsResolvedFromCache: %d (%0.2f%%)",
		iHostLookups, iHostLookupsFoundInCache,
		(iHostLookups?
			((float)iHostLookupsFoundInCache/(float)iHostLookups)*100
			:0.0)
		);
	return result;
}	
/**************************************************************************/
const char *resolver_version(char *argument)
{
	static char result[MSL];
	snprintf(result, sizeof(result), "%s", RESOLVER_VERSION);

	return result;
}

/**************************************************************************/
// look up an ip address or hostname 
const char *resolver_resolve(char *argument)
{
	bool system_result=false;
	bool log_resolve=true;
	hnode * pHost;
	static char result[HSL];
	char initial_request[HSL];
	// process requests in the format: 'resolve <requesting_name> <domain_name>'
	// get the name of the person we are resolving for
	char requesting_name[200];
	char *n=argument;
	while(!IS_NULLSTR(n) && isspace(*n)){ // trim off any leading whitespace
		n++;
	}

	// *n is either the end of the string or a non space character
	char *name_start=n;
	for( ;!IS_NULLSTR(n) && !isspace(*n); n++){
		// loop till we find the next space or end of string
	}
	if(IS_NULLSTR(n)){ // premature end of string -> error
		// we return the message with a destination of "system" since no name was specified
		snprintf( result, sizeof(result), "system invalid request - requesting name not specified.");
		return result;
	}

	// *n contains a space after one or more non space characters
	*n='\0'; // terminate the name_start

	if(strlen(name_start)>sizeof(requesting_name)-1){
		// we return the message with a destination of "system" since no name was specified
		snprintf( result, sizeof(result), "system invalid request - requesting name too long.");
		return result;
	}

	// copy name_start into requesting_name
	strcpy(requesting_name, name_start);
	
	n++; // advance over the terminating we just did
	// fast forward to find the next piece of text
	while(!IS_NULLSTR(n) && isspace(*n)){
		n++;
	}

	if(IS_NULLSTR(n)){// premature end of string -> error
		// we return the message with a destination of "system" since no name was specified
		snprintf( result, sizeof(result), "%s invalid request - nothing specified to resolve.", 
			requesting_name);
		return result;
	}

	log_resolve=(requesting_name[0]!='*' || requesting_name[1]);
	if(!strcmp(requesting_name,"system")){
		log_resolve=false;
		system_result=true;
	}
	if(log_resolve){
		resolver_logf( 5,"resolving '%s' for '%s'", n, requesting_name);
	}

	// check that we have a valid ipv4 or ipv6 address in szIPAddress
	// by attempting to resolve it, if we don't the below function will 
	// return false
	sockaddr_type socket_address;
	size_t address_length;
	initial_request[0]='\0';
	bool valid_ip=resolve_text_ip_into_address(&socket_address, &address_length, n, NULL, NULL);
	if(valid_ip){
		// we have an ip... look that up, get the dns name then resolve that
		pHost=lookup_host(&socket_address, address_length);
		if(strcmp(pHost->hostname,n)){
			sprintf(initial_request, "Initial request for %s resolves to '%s'`1",
				n, pHost->hostname);
		} 
		n=pHost->hostname;
	}

	// at this stage, n points to the string we want to resolve on the following basis:
	// - if the request was originally an ip address, that has been looked up, 
	//   and we are now resolving the hostname if it could be resolved (otherwise the ip)
	// - if the request was a hostname all along, we are resolving that.

#ifdef IPV6_SUPPORT_ENABLED
	{
		// ipv4/ipv6 lookup code
		struct addrinfo hints, *reshead=NULL, *res=NULL;
		int r;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags=AI_CANONNAME; 

		r=getaddrinfo(n, "80", &hints, &res);
		if(r){
			// if resolution failed, try resolving once more incase it is due to slow DNS
			if(res){
				freeaddrinfo(res);
			}
			r=getaddrinfo(n, "80", &hints, &res);
		}

		if(r){
			// failed to resolve two times in a row, report the error and give up
			sprintf(result,"%s resolver_resolve(): getaddrinfo() error %d (%s) occurred while attempting to resolve '%s'.", 
				 requesting_name, r, redirect_socket_error_text(r), n);
			if(res){
				freeaddrinfo(res);
			}
			return (log_resolve||system_result)?result:"";
		}
		if(!res){
			// if it did resolve, but didn't give us a usable result report that
			sprintf(result,"%s resolver_resolve(): getaddrinfo() returned an empty res value when resolving '%s' - effectively no result.", 
				 requesting_name, n);
			return (log_resolve||system_result)?result:"";
		}

		// successful resolution, format the results
		if(IS_NULLSTR(res->ai_canonname)){
			sprintf(result, "%s %sResolve request for:`1    %s", initial_request, requesting_name, n);
		}else{
			// canonical name of the specified node (offical name)
			sprintf(result, "%s %sResolve request for:`1    %s`1Official Name:`1    %s", 
				requesting_name, initial_request, n, res->ai_canonname);
		}

		reshead=res; // save the head of the res list for freeaddrinfo
		for(; res; res= res->ai_next){
			char temp[MSL];
			char sz_host[NI_MAXHOST+1];
			int hostlen = NI_MAXHOST;

			r= getnameinfo( res->ai_addr, res->ai_addrlen, sz_host, hostlen, NULL, 0,NI_NUMERICHOST);
			if(r){
				snprintf(temp, MSL-2, "`1unexpected error converting binary ai_addr into text version!");
			}else{
				char family[MSL];
				if(res->ai_family==PF_INET){
					strcpy(family, "ipv4");
				}else if(res->ai_family==PF_INET6){
					strcpy(family, "ipv6");
				}else{
					sprintf(family, "%d", res->ai_family);
				}
				snprintf(temp, MSL-2, "`1%s-'%s'", family, sz_host);
			}
			strcat(result,temp);
			if(strlen(result)>HSL-MSL){
				break;
			}
		}
	}
#else
	{
		// ipv4 lookup only code
		struct hostent *h;
		h=gethostbyname(n);
		if(h && (log_resolve || system_result)){
			// we have a match with one or more addresses
			char temp[8196];
			if(strlen(h->h_name)<8000){

				// offical name
				sprintf(result,"%s %sResolve request for:`1    %s`1`1Official Name:`1    %s", 
					requesting_name, initial_request, n, h->h_name);

				if(h->h_addrtype!=AF_INET){
					sprintf(temp, "`1Unrecognised address type %d result, can't display actual address/addresses.", 
						h->h_addrtype);
					strcat(result,temp);
				}else{
					// ip addresses
					if(h->h_addr_list[0]){			
						int ai=0;
						for( ; h->h_addr_list[ai]; ai++){						
							in_addr address;
							memcpy(&address.s_addr, h->h_addr_list[ai], h->h_length);

							sprintf(temp, "`1ipv4-'%s'", inet_ntoa(address));
							if(strlen(result)+ strlen(temp)<8000){
								strcat(result, temp);
							}
						}
					}

					// aliases
					if(h->h_aliases && *(h->h_aliases)){
						strcat(result, "`1`1Aliases include:");
						char **an=h->h_aliases;
						while(*an){
							sprintf(temp, "`1    '%s'", *an);
							if(strlen(result)+ strlen(temp)< 8000){
								strcat(result, temp);
							}
							an++;
						}
					}
				}

				return (log_resolve||system_result)?result:"";
			}
			snprintf( result, sizeof(result), "%s '%s' resolved, but was too long (%d characters).", 
				requesting_name, n, ((int)strlen(h->h_name)));
		}else{
			snprintf( result, sizeof(result), "%s Failed to resolve: '%s'", 
				requesting_name, n);
		}
	}
#endif // not IPV6_SUPPORT_ENABLED
	return (log_resolve||system_result)?result:"";;
}

/**************************************************************************/
// return what the text ip resolves to
// fill in the socket_address details based on the szAddress
const char *resolver_resolveip(char *argument)
{
	hnode * pHost;
	static char result[HSL];
	int errval;
	char errmsg[MSL];
	sockaddr_type socket_address;
	size_t address_length;

	// check that we have a valid ipv4 or ipv6 address in szIPAddress
	// by attempting to resolve it, if we don't the below function will 
	// return false
	if(!resolve_text_ip_into_address(&socket_address, &address_length, argument, &errval, errmsg)){
		resolver_logf(errval, "%s", errmsg);
		sprintf(result, "unrecognised address '%s'", argument);
		return result;		
	}

	// report what we are about to do
	resolver_logf(3, "dns lookup: %s", argument);

	// order to do things in is first dns (more important), then ident
	
	pHost= lookup_host(&socket_address, address_length); // looks up the host, and handles caching
	
	// display the dns results
//	resolver_responsef("ip %s %s", pHost->szAddr, pHost->hostname);
//	resolver_logf(2, "IP RESULT SENT: %s %s", pHost->szAddr, pHost->hostname);

	sprintf(result, "ip=%s lookup=%s reverse=%s", pHost->szAddr, pHost->hostname, pHost->szAddr);
	return result;
}

/**************************************************************************/
const char *resolver_commands(char *);
command_type command_table[]={
	{"close",		"",					resolver_close},
	{"commands",	"",					resolver_commands},
	{"resolve",		"<requesting_user> <ip|dns name>",	resolver_resolve},
	{"resolveip",	"<ip>",				resolver_resolveip},	
	{"stats",		"",					resolver_stats},	
	{"version",		"",					resolver_version},
	{"", NULL}
};
/**************************************************************************/
const char *resolver_commands(char *argument)
{
	static char result[MSL];
	result[0]='\0';
	for(int i=0; !IS_NULLSTR(command_table[i].name);){
		strcat(result,command_table[i].name);
		i++;
		if(!IS_NULLSTR(command_table[i].name)){
			strcat(result," ");
		}
	}
	return result;
}
/**************************************************************************/
// process a command (all commands are on a single line)
// return true if a match was found
bool interpret_line(const char *line)
{
	char emptystring[1];
	emptystring[0]='\0';
	char entire_line[MSL];
	char command[MSL];
	char *argstart;
	strncpy(entire_line, line, sizeof(entire_line)-1);
	entire_line[sizeof(entire_line)-1]='\0';

	// find the first space in the table
	argstart=strstr(entire_line, " ");
	if(argstart){
		*argstart='\0'; // terminate the command
		argstart++; // args are the first word after the command
	}else{
		argstart=emptystring;
	}
	strcpy(command, entire_line);

	// find the "command" in the command_table
//	resolver_logf(5, "'%s' command received: argstart='%s'", command, argstart);

	for(int i=0; !IS_NULLSTR(command_table[i].name); i++){
		if(!strcmp(command_table[i].name, command)){
			const char *response=(*(command_table[i].func)) (argstart);
			if(!IS_NULLSTR(response)){
				resolver_responsef("%s_response %s", command, response);
			}
			return true;
		}
	}
	resolver_responsef("%s_response unrecognised command", command);
	return false;
}

/**************************************************************************/
void resolve_ident_failed(hnode *host, const char *reason)
{
	if(!IS_NULLSTR(reason)){
		resolver_logf(5, "%s",reason);	
	}
	host->ident_failure_count++;
	host->retry_ident_after=current_time()+ (host->ident_failure_count*60);
}
/**************************************************************************/
const char *read_from_socket(SOCKET Socket, int timeout)
{
	static char data[1024];
	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdException;
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdException);
	FD_SET(Socket, &fdRead);
	FD_SET(Socket, &fdWrite);
	FD_SET(Socket, &fdException);
	
	struct timeval select_timeout;
	select_timeout.tv_usec=0;
	select_timeout.tv_sec=timeout;
	
	if(select(Socket+1, &fdRead, NULL, NULL, &select_timeout)>0){
		if(recv(Socket, data, 1023, 0)<0){
			return "";
		}
		data[1023]='\0';
		return data;
	}
	return "";

}

/**************************************************************************/
int connect_with_timeout(SOCKET s, const struct sockaddr * addr, int timeout)
{
#ifdef WIN32
	// set the socket to non-blocking mode
	unsigned long blockmode = 1; 
	if(ioctlsocket(s, FIONBIO, &blockmode)== SOCKET_ERROR)    
	{        
		resolver_logf(8, "ioctlsocket: error setting new socket to nonblocking, "
			"WSAGetLastError=%d", WSAGetLastError());        
		return SOCKET_ERROR;    
	}  

	// start the connection
	int nRet= connect(s, addr, sizeof(SOCKADDR_IN));
	if(nRet==0){
		return 0;
	}else{
		if(WSAGetLastError()!=WSAEWOULDBLOCK){
			socket_error("connect_with_timeout: connect()");
		}
	}

	// no use a select call to wait for the socket to be ready/timeout
	
	// set our select_timeout, to timeout seconds
	struct timeval select_timeout;
	select_timeout.tv_sec=timeout;
	select_timeout.tv_usec=0;

	// prepare our filedescriptor set
	fd_set fdWrite;
	FD_ZERO(&fdWrite);
	FD_SET(s, &fdWrite);

	// select and wait
	nRet=select(s+1, NULL, &fdWrite, NULL, &select_timeout);

	if(nRet<1){
		if(nRet==0){
			resolver_logf(5, "connect timeout.");
		}else{
			resolver_logf(5, "select() an error occured.");
		}
		return SOCKET_ERROR;
	}
	return 0; // successful connect
#else
// unix version of connect_with_timeout()
	// set the socket to non-blocking mode
	if ( fcntl(s, F_SETFL, O_NONBLOCK)< 0 ){
		resolver_logf(8,"fcntl: error setting new socket to nonblocking");
		return SOCKET_ERROR;
	}

	resolver_logf(9,"connecting to %d", s);

	// set our select_timeout, to sleep for 0.5 seconds
	struct timeval select_timeout;

	// start the connect
	int nRet;
	time_t end_time=current_time()+timeout;
	while(end_time>current_time()){
		// we keep looping till our time is done, 
		// or a positive connect or an unexpected error
		nRet= connect(s, addr, sizeof(SOCKADDR_IN));
		resolver_logf(9,"errno=%d, nRet=%d", errno, nRet);
		if(nRet==0){
			return 0;
		}
		if(nRet<0){
			#ifndef EISCONN
				#define EISCONN 56 // socket is already connected
			#endif
			if(errno==EISCONN){
				return 0;
			}
			if (errno != EINPROGRESS && errno != EALREADY) 
			{
				socket_error("connect_with_timeout(): connect()");
				return SOCKET_ERROR;
			}
		}

		// sleep for half a second
		select_timeout.tv_sec=0;
		select_timeout.tv_usec=500000;
		if(select(0, NULL, NULL, NULL, &select_timeout)<0){
			socket_error("select()");
		}
	}
	
	resolver_logf(5, "connect timeout.");
	return SOCKET_ERROR;
#endif
}
/**************************************************************************/
// return true if successful
bool resolve_ident(hnode *host, int LocalPort, int RemotePort)
{
	SOCKET Socket; 
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		resolve_ident_failed(host, "Error creating socket for ident!");
		return false;
	}

	resolve_ident_failed(host, "Ident support disabled for now.");
	return false; // not supported for now
}

/**************************************************************************/
#ifdef IPV6_SUPPORT_ENABLED
// if the host is an ipv4 inside an ipv6 address e.g. "::ffff:a.b.c.d"
// then strip off the front prefix and resolve as an ipv4 address
// return NULL if the address isn't of this type, 
// otherwise if it resolves sucessfully, return the dns name
char *resolve_ipv4_inside_ipv6(char *szAddress)
{
	static char result[MSL];
	int r;
	// check if the host is an ipv4 inside an ipv6 address
	// e.g. "::ffff:a.b.c.d"
	if(strncmp("::ffff:", szAddress, strlen("::ffff:"))){
		return NULL;
	}

	strcpy(result, &szAddress[strlen("::ffff:")]);

	sockaddr_type socket_address;
	size_t address_length;
	bool valid_ip=resolve_text_ip_into_address(&socket_address, &address_length, result, NULL, NULL);
	if(!valid_ip){
		return NULL;
	}

	const struct sockaddr* sa=(const struct sockaddr*)&socket_address;
	if(sa->sa_family!=AF_INET){
		return NULL;
	}

	// we have what appears to be a valid ipv4 address, 
	// look it up in the dns to see if we get a result
	char sz_host[NI_MAXHOST], sz_serv[NI_MAXSERV];
	int hostlen = NI_MAXHOST, servlen = NI_MAXSERV;
	size_t salen=address_length;

	r= getnameinfo( sa, salen, sz_host, hostlen, sz_serv, servlen, NI_NUMERICSERV);
	if(r!=0){
		// have 2 attempts at resolving address if first attempt failed
		sleep_seconds(5);
		r= getnameinfo( sa, salen, sz_host, hostlen, sz_serv, servlen, NI_NUMERICSERV);
	}

	if(r!=0){ 
		return NULL;
	}
		
	// success, return the result prefixed to make it clear how it was resolved
	sprintf(result, "::ffff:%s", sz_host);
	return result;
}
#endif

/**************************************************************************/
// attempt to do a dns lookup on a host
// the host structure already has its socket_address, and address_length fields filled in
// this process will fill in the host->hostname field with either the resolved
// name or a copy of the ip which is already in host->szAddr
void resolve_host(hnode *host)
{
#ifdef IPV6_SUPPORT_ENABLED
	const struct sockaddr* sa=(const struct sockaddr*)&host->socket_address;
	char sz_host[NI_MAXHOST], sz_serv[NI_MAXSERV];
	int hostlen = NI_MAXHOST, servlen = NI_MAXSERV, result;
	size_t salen=host->address_length;
	char *embedded_address;

	result = getnameinfo( sa, salen, sz_host, hostlen, sz_serv, servlen, NI_NUMERICSERV);
	if(result!=0){
		// have 2 attempts at resolving address if first attempt failed
		sleep_seconds(5);
		result = getnameinfo( sa, salen, sz_host, hostlen, sz_serv, servlen, NI_NUMERICSERV);
	}

	if(result==0){ // success, strdup the result - replacing the old result if necessary
		if(host->hostname){
			free(host->hostname);
		}
		host->hostname=strdup(sz_host);
		host->reresolve_after=current_time()+ (24*60*60); // cache for up to 1 day

		// if the hostname is actually the same as the ip address, 
		// check if it is an ipv4 address embeeded in an ipv6 address
		if(!strcmp(host->szAddr,host->hostname)){
			embedded_address=resolve_ipv4_inside_ipv6(host->szAddr);
			if(embedded_address){
				if(host->hostname){
					free(host->hostname);
				}
				host->hostname=strdup(embedded_address);
			}
		}
	}else{ // failed to resolve... 

		// check if it is an embeeded address
		char *embedded_address=resolve_ipv4_inside_ipv6(host->szAddr);
		if(embedded_address){
			if(host->hostname){
				free(host->hostname);
			}
			host->hostname=strdup(embedded_address);
			host->reresolve_after=current_time()+ (24*60*60); // cache for up to 1 day
		}else{
			// put in ip address if we dont already have one
			if(!host->hostname){
				host->hostname=strdup(host->szAddr);
			}
			host->reresolve_after=current_time()+ (30*60); // cache for up to 30 minutes
		}
	}
#else
	struct hostent *hostent;
	hostent= gethostbyaddr( 
		(const char *) &host->socket_address.sin_addr.s_addr, 
		sizeof(host->socket_address.sin_addr.s_addr), 
		host->socket_address.sin_family);
		
	if ( !hostent){ 	
		// have 2 attempts at resolving address if first attempt failed
		sleep_seconds(5);
		hostent= gethostbyaddr( 
			(const char *) &host->socket_address.sin_addr.s_addr, 
			sizeof(host->socket_address.sin_addr.s_addr), 
			host->socket_address.sin_family);
	}

	if (hostent){ // success, strdup the result - replacing the old result if necessary
		if(host->hostname){
			free(host->hostname);
		}
		host->hostname=strdup(hostent->h_name);
		host->reresolve_after=current_time()+ (24*60*60); // 1 day
	}else{ // failed to resolve... put in ip address if we dont already have one
		if(!host->hostname){
			host->hostname=strdup(host->szAddr);
		}
		host->reresolve_after=current_time()+ (30*60); // 30minutes
	}
#endif
	return;
}
/**************************************************************************/
hnode *lookup_host(sockaddr_type *socket_address, size_t address_length)
{
	iHostLookups++;
	hnode *result=find_in_cache(socket_address, address_length);

	if(!result){ 
		// details not found in the cache, allocate a new host node
		// in which the constructor sets its reresolve_after value to 0 
		// which in turn will trigger a resolve_host() call below
		result=new hnode(socket_address, address_length);
		
		// insert at the top of the correct hash table bucket
		unsigned int key=get_hash_key(socket_address, address_length);
		result->next=hosts_table[key];
		hosts_table[key]=result;
	}

	// check if the results are old or non existant, if so resolve now
	if(result->reresolve_after<current_time()){
		resolve_host(result);
	}
	return result;
}

/**************************************************************************/
// return true if text ip was resolved successfully
// fill in the socket_address details based on the szAddress
bool resolve_text_ip_into_address(sockaddr_type *socket_address, size_t *address_length, char*pszAddress, int *error_value, char *error_message)
{
	if(IS_NULLSTR(pszAddress)){
		resolver_logf(2, "Empty query: '%s' is not a valid ipv4 or ipv6 address.",pszAddress);
		return false; // can't have an empty string
	}
#ifdef IPV6_SUPPORT_ENABLED
		struct addrinfo hints, *res;
		memset(&hints, 0, sizeof(struct addrinfo));		
		hints.ai_flags=AI_NUMERICHOST; // we don't want any hostnames here
		hints.ai_family   = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		res=NULL;
		int r = getaddrinfo(pszAddress, // ip address in text form
					"0",  // we don't care about the service
					&hints, 
					&res);

		if (r) {
			char errmsgbuf[2048];
			if(error_value){
				*error_value=2;
			}
#ifdef WIN32
			if(r==WSAHOST_NOT_FOUND){
				if(error_value){
					*error_value=2;
				}
				sprintf(errmsgbuf,"resolve_text_ip_into_address error %d (%s)- couldn't convert '%s' to a valid ip address.", 
					r, get_socket_error_text(r), pszAddress);
			}
/*			else if(r==WSAEAFNOSUPPORT && node->protocol==PROTOCOL_IPV6){
				resolver_logf("\n"
					"getaddrinfo error %d - couldn't convert '%s' to a valid ip address\n"
					" - getaddrinfo reported this system has no support for the AF_INET6 address family.\n"
					"   This error message is normal if this system doesn't support ipv6, as it\n"
					"   was an ipv6 address getaddrinfo() was asked to convert.", 
					r, node->psz_bind_address);
			}else
*/			
			else{
				sprintf(errmsgbuf,"resolve_text_ip_into_address(): getaddrinfo() error %d (%s)- couldn't convert '%s' to a valid ip address.", 
					 r, get_socket_error_text(r), pszAddress);
			}
#else
			{
				sprintf(errmsgbuf,"resolve_text_ip_into_address(): getaddrinfo() error %d (%s)- couldn't convert '%s' to a valid ip address.", 
					r, gai_strerror(r), pszAddress);
			}
#endif
			if(error_message){
				strcpy(error_message, errmsgbuf);
			}
			if(res){
				freeaddrinfo(res);
			}
			return false;
		}
		// we had success
		memcpy(socket_address, res->ai_addr, res->ai_addrlen);
		*address_length=res->ai_addrlen;
		freeaddrinfo(res);


#else // !IPV6_SUPPORT_ENABLED
		memset(socket_address, 0, sizeof(sockaddr_type));
		socket_address->sin_addr.s_addr = inet_addr( pszAddress);
		socket_address->sin_family = AF_INET;
		*address_length=sizeof(sockaddr_type);

		if(socket_address->sin_addr.s_addr==INADDR_NONE){ 
			// we had an invalid address supplied
			// technically INADDR_NONE can be returned by the valid ip 255.255.255.255
			// but 255.255.255.255 isn't useful as a bind address, so we assume the 
			// input node->psz_bind_address was invalid.
			if(error_value){
				*error_value=2;
			}
			if(error_message){
				sprintf(error_message,"Invalid query: '%s' is not a valid ipv4 address.",pszAddress);
			}
			return false;
		}
#endif	// ifdef IPV6_SUPPORT_ENABLED

		return true;
}
/**************************************************************************/
char *get_enabled_commands()
{
	static char result[MSL];
	char buf[1024];
	result[0]='\0';
	strcpy(result, "\n");
	for(int i=0; !IS_NULLSTR(command_table[i].name);){
		sprintf(buf, "      %s %s\n", 
			command_table[i].name,
			command_table[i].syntax);
		strcat(result, buf);
		i++;
	}
	return result;
}
/**************************************************************************/
int main(int argc, char *argv[])
{
    char buf[16384];
    char *p;    	

	resolver_logf(0, "\n"
		"============================================================================\n"
		"    Resolver v%s - Caching Host Name and Ident resolving tool\n"
#ifdef IPV6_SUPPORT_ENABLED
		"    Purpose:   Allow dns hostname resolving of ipv4 and ipv6 \n"
		"               addresses and ident lookups for a mud process.\n"
#else
		"    Purpose:   Allow dns hostname resolving of ipv4 addresses\n"
		"               and ident lookups for a mud process.\n"
#endif
		"               http://www.dawnoftime.org/\n"
		"               (c) Kalahn 1997-2010\n"
		"\n"
		"    Syntax: aaa.bbb.ccc.ddd                     (dns query only)\n"
		"            aaa.bbb.ccc.ddd,localport,remoteport (dns and ident)\n"
		"            resolve <username> <dns name>    (lookup a dns name)\n"
		"            version                (what version are we running)\n"
		"            stats   (show some stats about resolver performance)\n"
		"\n"
		"    Enabled commands: %s\n"
		"============================================================================", 
		RESOLVER_VERSION,
		get_enabled_commands());


	resolver_logf(1, "Verbose level: %d", VERBOSE_LEVEL);
	resolver_logf(1, "Ident connect timeout: %d second%s", 
		IDENT_CONNECT_TIMEOUT, IDENT_CONNECT_TIMEOUT==1?"":"s");

	if(init_network()==false){
		resolver_logf(0,
			"\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
			"! %-10s COULD NOT INITIALISE WINSOCK SUPPORT !\n"
			"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
			argv[0]);
		exit(1);
	};
	

	// initialise the hash table to empty
	for(int i=0; i<HHK; i++){
		hosts_table[i]=NULL;
	}

	// main IO loop 
	while ( fgets( buf, 16383, stdin ) ) 
	{
//		resolver_logf(0, "STDIN(((%s)))", buf);	
		p = strchr( buf, '\n' ); // all commands are terminated with \n

		if ( IS_NULLSTR(p)){
			resolver_logf(5, "^^^^^invalid input - '%s'^^^^^", buf);	
			continue;
		}
		*p = '\0'; // chop off the \n

		if(IS_NULLSTR(buf)){
			continue;
		}

		interpret_line(buf);
    }
    return 0;
};

/**************************************************************************/
hnode *find_in_cache(sockaddr_type *socket_address, size_t address_length)
{
	hnode *host;
	unsigned int key=get_hash_key(socket_address, address_length);

	for (host= hosts_table[key]; host; host= host->next)
	{		
		if ( !memcmp(&host->socket_address, socket_address, address_length) ){
			iHostLookupsFoundInCache++;
			return host;
		}	
	}
	return NULL;
}
/**************************************************************************/
/*
* resolver - caching dns and ident lookup, by Kalahn (c)1997-2010
*
* USAGE:
*   accepts input from stdin in the form 
*     aaa.bbb.ccc.ddd\n   (dns query)
*   or 
*     aaa.bbb.ccc.ddd,localport,remoteport\n  (dns and ident query)
*   results are sent to stdout
*   <localport> = port mud is running on locally
*
* maximum input is:
*   15(ip) + 1(,) + 5(biggest short) + 1(,) + 5(biggest short) + 1(\n) = 28

* 
* DNS result:
*   aaa.bbb.ccc.ddd dns.host.name\n
* IDENT result:
*   aaa.bbb.ccc.ddd ,localport,remoteport ident_result\n
*
* an ident response always starts with a , 
* (because , is not valid in a domain name it is easy to detect idents)
* 
* NOTES:
* - Resolver does not bother to ditch old cached results, just relooks up
*   after they have aged a certain amount (1 day by default)
* - Resolver caches negative dns hits - if it cant resolve an ip address 
*   it wont attempt to reresolve the ip for 30 minutes.
* - Resolver caches failed ident attempts, if it cant contact an ident 
*   server, it wont retry an ident for 1minute*number of failures for the
*   specific ip.
* - The resolver will spend up to IDENT_CONNECT_TIMEOUT waiting for a 
*   connect to a ident server to establish.
* - The resolver gives ident servers 20 seconds to respond once connected.
* - The port ident lookups are directed to is take from /etc/services
*/
/**************************************************************************/
//  look at top of file for instructions on manual compilation
/**************************************************************************/
