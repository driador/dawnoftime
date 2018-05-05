/*************************************************************************/
// network.h 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef NETWORK_H
#define NETWORK_H

#ifdef INCLUDE_H
#error network.h must be included before include.h
#endif

/**************************************************************************/
#if defined(__APPLE__) && defined(__MACH__) && !defined(unix)
	// mac os-x
	// __APPLE__ by itself, is a mac that isn't based on an Mach kernel.
	#define unix
#endif

#if defined(__unix__) && !defined(unix) // we expect to see unix
	#define unix __unix__
#endif

#ifdef WIN32
	#include "wincfg.h"
	#define WIN32_LEAN_AND_MEAN	// Speed up the compiling process

	#if defined(WIN32_IPV6) || defined(__MINGW__) || defined(__MINGW32__)
		// Incomplete IPv6 support requires winsock2
		// MingW should be fine with Winsock2 - pretty much everything should actually
		#define WIN32_USE_WINSOCK2
	#endif

	// are we using winsock v1 or winsock v2
	#ifdef WIN32_USE_WINSOCK2
#include <winsock2.h>
#include <winsock.h>
		#ifdef _MSC_VER // visual c++ only
			#pragma comment( lib, "ws2_32.lib") // saves adding the library to the project :)
		#endif
	#else
		#include <winsock.h>
		#ifdef _MSC_VER // visual c++ only
			#pragma comment( lib, "wsock32.lib") // saves adding the library to the project :)
		#endif
	#endif

	// are we using ipv6
	#ifdef WIN32_IPV6
		#define IPV6_SUPPORT_ENABLED
		#include <ws2tcpip.h>
		#include <tpipv6.h>
	#endif
#endif
// extern globals

// function prototypes
#ifdef WIN32
bool init_winsock();
void close_network();
#endif
void init_network();
void close_network();
void socket_error(const char *text);

#ifdef unix
	#define closesocket(sock) close(sock)
#endif

#ifdef WIN32
	typedef SOCKET dawn_socket;
	const dawn_socket dawn_socket_INVALID_SOCKET=INVALID_SOCKET;
#else
	typedef int dawn_socket;
	const dawn_socket dawn_socket_INVALID_SOCKET=-1;
#endif

/**************************************************************************/
#endif // NETWORK_H
/**************************************************************************/

