/**************************************************************************/
// network.cpp - network related functions, Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "network.h"
#include "include.h"
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
	return true;
}	
#endif
/**************************************************************************/
// exits if network initialisation fails
void init_network()
{
#ifdef WIN32
	if(!init_winsock()){
		exit_error( 1 , "init_network", "init_winsock failed");;
	}
#endif

	// find out what the hostname is that we are on
    if(gethostname(MACHINE_NAME, MSL)!=0){
		bugf("init_network(): gethostname error %d (%s)",
			errno, strerror( errno));
    }
	if(IS_NULLSTR(MACHINE_NAME)){
		strcpy(MACHINE_NAME,"unknown_machine_name.");
    }
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
		exit_error( 1 , "close_network", "close winsock failed");
	}
#endif
}

/**************************************************************************/
// winsockError() prints to stderr the laston winsock error
#ifdef WIN32
const char *get_winsock_error_text(int errorcode)
{
	static char result[MSL];
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
	bugf("socket_error(): %s - error %d (%s)",
		text, errno, strerror( errno));
	return;
#endif
#ifdef WIN32
	logf("%s - %s", text, get_winsock_error_text(0));
	return;
#endif
	bugf("socket_error(): %s - error %d",text, errno);
	return;
}
/**************************************************************************/
/**************************************************************************/

