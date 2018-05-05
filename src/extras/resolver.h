/**************************************************************************/
// resolver.h - header file for resolver.cpp
/***************************************************************************
 * The Dawn of Time v1.69s (c)1997-2010 Kalahn                             *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef __RESOLVER_H__
#define __RESOLVER_H__

#ifndef __IN_RESOLVER_CPP__
#error This file "resolver.h" should only be included by resolver.cpp
#endif

#if defined(__APPLE__) && defined(__MACH__)
	// mac os-x
	// __APPLE__ by itself, is a mac that isn't based on an Mach kernel.
	#define unix
#endif
#if defined(__unix__) && !defined(unix) // we expect to see unix
	#define unix __unix__
#endif
/**************************************************************************/
// windows compiler work to figure out what winsock components we need
#ifdef WIN32
	#include "../wincfg.h"
	#define WIN32_LEAN_AND_MEAN	// Speed up the compiling process

	#ifdef WIN32_IPV6 // Incomplete IPv6 support requires winsock2
		#define WIN32_USE_WINSOCK2
	#endif

	// are we using winsock v1 or winsock v2
	#ifdef WIN32_USE_WINSOCK2
#include <winsock2.h>
#include <winsock.h>

		#pragma comment( lib, "ws2_32.lib") // saves adding the library to the project :)
	#else
		#include <winsock.h>
		#pragma comment( lib, "wsock32.lib") // saves adding the library to the project :)
	#endif

	// are we using ipv6
	#ifdef WIN32_IPV6
		#define IPV6_SUPPORT_ENABLED
		#include <ws2tcpip.h>
		#include <tpipv6.h>
	#endif
#endif
// extern globals
/**************************************************************************/
#include <assert.h>
#ifndef DEBUG
	#define DEBUG
#endif

#ifdef WIN32
	#include <time.h>
	#include <sys/timeb.h>
#else
	#include "../config.h"
	#ifdef TIME_WITH_SYS_TIME
		#include <sys/time.h>
		#include <time.h>
	#else
		#ifdef HAVE_SYS_TIME_H
			#include <sys/time.h>
		#else
			#include <time.h>
		#endif
	#endif
	#include <sys/resource.h>
	#include <unistd.h>
	#include <sys/socket.h>
#endif


#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef unix
	#include <netdb.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <errno.h>
	typedef int SOCKET;
	typedef struct sockaddr_in SOCKADDR_IN;
	#define SOCKET_ERROR            (-1)
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define closesocket(sock) close(sock)
#endif

#ifdef WIN32
// compatiblity macros
#define vsnprintf(buf, len, fmt, args)	_vsnprintf(buf, len, fmt, args)
#define popen(command, mode)			_popen(command,mode)
#define pclose(fp)						_pclose(fp)
#define getcwd(dir, size)				_getcwd(dir, size)
#define chdir(dir)						_chdir(dir)
#define mkdir(dir)						_mkdir(dir)
#define snprintf						_snprintf
typedef int socklen_t;

#endif

/**************************************************************************/
// structs
//typedef unsigned long ip_address;
#ifdef IPV6_SUPPORT_ENABLED	
	typedef sockaddr_storage sockaddr_type;
#else
    typedef sockaddr_in sockaddr_type;
#endif

/**************************************************************************/
char *convert_sockaddr_into_textip(const struct sockaddr* sa, socklen_t salen);
char *shortdate(time_t *tm);

#ifdef WIN32
	#define get_socket_error_text(errorcode) get_winsock_error_text(errorcode)
	const char *get_winsock_error_text(int errorcode);
#else
	#define get_socket_error_text(errorcode) gai_strerror(errorcode)
#endif

/**************************************************************************/

class hnode {
public:
	sockaddr_type socket_address;
	size_t address_length;
	char *szAddr;
	char *hostname;
	char *ident_result;
	time_t reresolve_after;

	int ident_failure_count; // true if an ident from this port has failed in the past
	time_t retry_ident_after; // doubles each time 1 minute, 2minutes, 4minutes

	hnode *next;

public:
	hnode(sockaddr_type *ipaddr, socklen_t ipaddress_len);
//	~hnode();
};
/**************************************************************************/
// Class Constructor - technically this shouldn't go in here, but
// since resolver.h is only included by one file, we get away with it.
// and it makes the resolver.cpp file cleaner.
hnode::hnode(sockaddr_type *ipaddr, socklen_t ipaddress_len)
{
	memcpy(&socket_address, ipaddr, sizeof(sockaddr_type));
	
	// convert the ip address into a string
	address_length=ipaddress_len;
	szAddr=strdup(convert_sockaddr_into_textip((const struct sockaddr *)&socket_address, address_length));	
	
	reresolve_after=0;
	hostname=NULL;
	ident_result=NULL;
	ident_failure_count=0;
	retry_ident_after=0;
	next=NULL;
}
/**************************************************************************/
// response to a command -> stdout
void resolver_response(const char *str)
{
#ifndef RESOLVER_DEBUG_BRACKETS
    fprintf(stdout, "%s\n", str);
    fflush(stdout);
#else
    fprintf(stdout, "STDOUT[[[%s]]]\n", str);
    fflush(stdout);
	fprintf(stderr, "STDERR COPY OF STDOUT[[[%s]]]\n", str);
	fflush(stderr);
#endif
}
/**************************************************************************/
// response to a command -> stdout
void resolver_responsef(const char * fmt, ...)
{
    char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 1014, fmt, args);
	va_end(args);

	resolver_response(buf);
}
/**************************************************************************/
// general logging and debugging information -> stderr
void resolver_log(int verbose, const char *str)
{
	if(verbose>VERBOSE_LEVEL){
		return;
	}
#ifndef RESOLVER_DEBUG_BRACKETS
    fprintf(stderr, "%s:resolver: %s\n", shortdate(NULL)+4, str);
#else
    fprintf(stderr, "STDERR{%d}[[[%s:resolver: %s]]]\n", verbose, shortdate(NULL)+4, str);
#endif
    fflush(stderr);

}
/**************************************************************************/
// general logging and debugging information -> stderr
void resolver_logf(int verbose, const char * fmt, ...)
{
	if(verbose>VERBOSE_LEVEL){
		return;
	}
    char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 1014, fmt, args);
	va_end(args);

	resolver_log(verbose, buf);
}

bool init_network();
// macros for working smarter not harder :)
#define IS_NULLSTR(str)		((str)==NULL || *(str)=='\0')
#define HHK (55) // hosts hash key
hnode * hosts_table[HHK];
/**************************************************************************/
int get_hash_key(sockaddr_type *ip_address, size_t address_length)
{
	unsigned int key;
	int total=0;
	unsigned char *p;
	p=(unsigned char *)ip_address;
	for(size_t i=0;i<address_length; i++){
		total+=p[i];
	}
	key=total%HHK;
	return key;
}
/**************************************************************************/
char *convert_sockaddr_into_textip(const struct sockaddr* sa, socklen_t salen)
{
	static int i;
    static char buf[4][8196];
	++i%=4;

#ifdef IPV6_SUPPORT_ENABLED
	char sz_host[NI_MAXHOST+1], sz_serv[NI_MAXSERV+1];
	int hostlen = NI_MAXHOST, servlen = NI_MAXSERV, result;

	result = getnameinfo( sa, salen, sz_host, hostlen, sz_serv, servlen,
					  NI_NUMERICHOST | NI_NUMERICSERV);
	if(result!=0){
		sprintf(buf[i],"convert_sockaddr_into_textip()-error_%d(%s)_while_converting_address", 
			result, get_socket_error_text(result));
	}else{
		strcpy(buf[i], sz_host);
	}
#else
	const struct sockaddr_in *sa2=(struct sockaddr_in*)sa;
	strcpy(buf[i],inet_ntoa(sa2->sin_addr));
#endif
	return buf[i];
}
/**************************************************************************/
// winsockError() prints to stderr the laston winsock error
#ifdef WIN32
const char *get_winsock_error_text(int errorcode)
{
	static char result[8196];
	// This function is only for use in WIN32
	#define WEM_CASE(m) case m: pszMsg = #m ; break
	const char *pszMsg;	
	int iError=0;
	if(errorcode!=0){
		iError=errorcode;
	}else{
		iError=WSAGetLastError();
	}
	switch(iError){
		WEM_CASE(WSABASEERR);
		WEM_CASE(WSAEINTR);
		WEM_CASE(WSAEBADF);
		WEM_CASE(WSAEACCES);
		WEM_CASE(WSAEFAULT);
		WEM_CASE(WSAEINVAL);
		WEM_CASE(WSAEMFILE);
		WEM_CASE(WSAEWOULDBLOCK);
		WEM_CASE(WSAEINPROGRESS);
		WEM_CASE(WSAEALREADY);
		WEM_CASE(WSAENOTSOCK);
		WEM_CASE(WSAEDESTADDRREQ);
		WEM_CASE(WSAEMSGSIZE);
		WEM_CASE(WSAEPROTOTYPE);
		WEM_CASE(WSAENOPROTOOPT);
		WEM_CASE(WSAEPROTONOSUPPORT);
		WEM_CASE(WSAESOCKTNOSUPPORT);
		WEM_CASE(WSAEOPNOTSUPP);
		WEM_CASE(WSAEPFNOSUPPORT);
		WEM_CASE(WSAEAFNOSUPPORT);
		WEM_CASE(WSAEADDRINUSE);
		WEM_CASE(WSAEADDRNOTAVAIL);
		WEM_CASE(WSAENETDOWN);
		WEM_CASE(WSAENETUNREACH);
		WEM_CASE(WSAENETRESET);
		WEM_CASE(WSAECONNABORTED);
		WEM_CASE(WSAECONNRESET);
		WEM_CASE(WSAENOBUFS);
		WEM_CASE(WSAEISCONN);
		WEM_CASE(WSAENOTCONN);
		WEM_CASE(WSAESHUTDOWN);
		WEM_CASE(WSAETOOMANYREFS);
		WEM_CASE(WSAETIMEDOUT);
		WEM_CASE(WSAECONNREFUSED);
		WEM_CASE(WSAELOOP);
		WEM_CASE(WSAENAMETOOLONG);
		WEM_CASE(WSAEHOSTDOWN);
		WEM_CASE(WSAEHOSTUNREACH);
		WEM_CASE(WSAENOTEMPTY);
		WEM_CASE(WSAEPROCLIM);
		WEM_CASE(WSAEUSERS);
		WEM_CASE(WSAEDQUOT);
		WEM_CASE(WSAESTALE);
		WEM_CASE(WSAEREMOTE);
		WEM_CASE(WSAEDISCON);
		WEM_CASE(WSASYSNOTREADY);
		WEM_CASE(WSAVERNOTSUPPORTED);
		WEM_CASE(WSANOTINITIALISED);
		WEM_CASE(WSAHOST_NOT_FOUND);
		WEM_CASE(WSATRY_AGAIN);
		WEM_CASE(WSANO_RECOVERY);
		WEM_CASE(WSANO_DATA);
	default:
		pszMsg=NULL;
	}


	if(pszMsg){
		sprintf(result,"%s(%d)", pszMsg, iError);
	}else{
		sprintf(result,"unknown winsock error value %d.", iError);
	}

	return result;	
}
#endif
/**************************************************************************/
void socket_error(const char *text)
{
#ifdef unix 
	perror(text);
	return;
#endif
#ifdef WIN32
	resolver_logf(2, "%s - %s", text, get_winsock_error_text(0));
	return;
#endif
}
/**************************************************************************/
void sleep_seconds(int seconds)
{
#ifdef WIN32
	Sleep( seconds*1000);
#else
	sleep( seconds);
#endif
}
/**************************************************************************/
#ifdef WIN32
void gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif
/**************************************************************************/
// returns true if successful, only called if running on WIN32
#ifdef WIN32
bool init_winsock()
{	
	// Startup winsock support in windows
#ifdef WIN32_USE_WINSOCK2
	WORD wVersionRequested = MAKEWORD( 2, 0 );
#else
	WORD wVersionRequested = MAKEWORD( 1, 1 );
#endif
	
	WSADATA wsaData;
	if ( WSAStartup( wVersionRequested, &wsaData ) ){
#ifdef WIN32_USE_WINSOCK2
		socket_error("Couldn't get any Winsock version 2 support!");
#else
		socket_error("Couldn't get any Winsock version 1.1 support!");
#endif
		return false;
	}
	resolver_logf(2, "winsock started");
	return true;
}	
#endif
/**************************************************************************/
// exits if network initialisation fails
bool init_network()
{
#ifdef WIN32
	return init_winsock();
#endif
	return true;
}
/**************************************************************************/
// returns true if successful, only called if running on WIN32
#ifdef WIN32
bool close_winsock()
{
	if ( WSACleanup() ){
		socket_error("WSACleanup() error.");
		return false;
	}
	return true;
}	
#endif
/**************************************************************************/
// exits if network initialisation fails
void close_network()
{
#ifdef WIN32
	if(!close_winsock()){
		exit(1);
	}
#endif
}
/**************************************************************************/
// get the current time
time_t current_time(void)
{
    struct timeval this_time;

	gettimeofday( &this_time, NULL );
	return this_time.tv_sec;
}
/**************************************************************************/
char *shortdate(time_t *tm) // kalahn - sept 97 
{
#ifndef DATE_IN_LOGS
	return "";
#endif
	static int i;
    static char result[3][30];
	// rotate buffers
	++i= i%3;
    result[i][0] = '\0';

    struct timeval last_time;
	gettimeofday( &last_time, NULL );
	time_t current_time = (time_t) last_time.tv_sec;

	if(!tm){
		tm=&current_time;
	}

    char *tbuf = ctime( tm );
    tbuf[strlen(tbuf)-6] = '\0';

    strcpy(result[i], tbuf);

    return(result[i]);
}



#endif // __RESOLVER_H__
/**************************************************************************/
/**************************************************************************/

