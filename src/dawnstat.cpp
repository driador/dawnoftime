/**************************************************************************/
// dawnstat.cpp - provide statistics on dawn based muds to dawnoftime.org
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
// About: The code within this module sends to dawnoftime.org statistical
//        information.  Using this information dawnoftime.org is able to 
//        get an idea of the number of dawn based muds running and 
//        hopefully the number of players.
//      
//        The statistical information submitted to dawnoftime.org is a 
//        summary of mudstats, lastonstats and mudclientstats, combined 
//        with a few other things specific to your mud environment 
//        (such as the name of the mud).  All information is kept private
//        and not made publically available through dawnoftime.org... but
//        in the future dawnoftime.org may include the statistical 
//        information to report how many dawn based muds are running, how
//        popular various mud clients are etc.
//
//        This information may be made publically available at
//        either the dawnoftime.org website or 
//        http://statistics.dawnoftime.org in the future (once enough 
//        muds are running dawn to make it worth while).
//      
//        By default the mud after about 10 minutes of running will 
//        send the stats to http://dawnstat2.dawnoftime.org/dawnstat2.php
//        This does not lag the mud in anyway, (unless your dns resolver
//        is broken - use sockets to determine this), if the dns resolver 
//        is broken there may be a one off small delay (ordinarily less 
//        than 5 seconds) while the mud resolves the ip address of 
//        dawnstat.dawnoftime.org in order to know where to send the 
//        stats to.
// 
// 

/**************************************************************************/
//#define DAWNSTAT_LOG_PROGRESS 

#define DAWNSTAT_SUBMIT_DOMAIN "dawnstat2.dawnoftime.org"
#define DAWNSTAT_SUBMIT_URL "/dawnstat2.php"

#define DAWNSTAT_DELAY 1800
/**************************************************************************/
#include "network.h"
#define __SEE_NETIO_INTERNAL_STRUCTURES__
#include "comm.h"
#include "include.h"
#include "laston.h"

#ifdef unix
	typedef int SOCKET;
	typedef struct sockaddr_in SOCKADDR_IN;
	#define SOCKET_ERROR            (-1)
	#define INVALID_SOCKET  (SOCKET)(~0)
#endif

#ifdef IPV6_SUPPORT_ENABLED
	struct addrinfo *res=NULL;
#else
	ipv4only_addrinfo *res=NULL;
#endif

#ifndef EISCONN
	#define EISCONN 56 // is already connected
#endif
/**************************************************************************/
static void dawnstat_logf(char * fmt, ...)
{
#ifndef DAWNSTAT_LOG_PROGRESS 
	return;
#endif
    char buf[HSL];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, HSL-MIL, fmt, args);
	va_end(args);

	logf("dawnstat: %s", buf);
}
/**************************************************************************/
int count_player_list(void)
{
	int count=0;
	char_data *ch=player_list;
	for ( ; ch; ch= ch->next_player){
        count++;  
	}
	return count;
}
/**************************************************************************/
int count_active_players(void);
char *get_compile_time (bool show_parent_codebase_version);
extern time_t laston_since;
void do_lastonstats( char_data *ch, char *argument);
char *doWebFunction(DO_FUN * do_fun, char *argument, int level);

/**************************************************************************/
static char *dawnstat_generate_statistics_text()
{
	static char result[45000];
	char stats[45000];
	stats[0]='\0';
	char *binded_sockets=netio_return_binded_sockets();

#define ENCODE_INT(field) strcat(stats, FORMATF("&" # field"=%d", field))
#define ENCODE_INTH(field, header) strcat(stats, FORMATF("&" header"=%d", field))
#define ENCODE_STR(field) strcat(stats, FORMATF("&" # field"=%s", url_encode_post_data(field)))
#define ENCODE_STRH(field, header) strcat(stats, FORMATF("&" header"=%s", url_encode_post_data(field)))

	ENCODE_STRH(game_settings->gamename,			"gamename");
	ENCODE_INTH(game_settings->unique_id,			"unique_id");
	ENCODE_STRH(game_settings->listen_on,		"listen_on");
	ENCODE_STR(binded_sockets);
	ENCODE_INTH(mainport,	"port");
	ENCODE_STRH(DAWN_RELEASE_VERSION, "release_version");
	ENCODE_STRH(DAWN_RELEASE_DATE, "release_date");
	ENCODE_STRH(fwrite_wordflag(game_settings_flags, game_settings->flags,"gamesettings_flags",NULL), "gamesettings_flags");
	ENCODE_STRH(fwrite_wordflag(game_settings_flags2, game_settings->flags2,"gamesettings_flags2",NULL), "gamesettings_flags2");
	ENCODE_STRH(fwrite_wordflag(game_settings_flags3, game_settings->flags3,"gamesettings_flags3",NULL), "gamesettings_flags3");
	ENCODE_STRH(fwrite_wordflag(game_settings_flags4, game_settings->flags4,"gamesettings_flags4",NULL), "gamesettings_flags4");	
	ENCODE_STRH(get_compile_time(false), "compile_time_text");
	ENCODE_INTH(game_settings->damage_scale_value, "damage_scale_value");
	ENCODE_INTH(game_settings->global_xp_scale_value,"global_xp_scale_value");	
	ENCODE_INTH(game_settings->newbie_start_gold,"newbie_start_gold");	
	ENCODE_INTH(game_settings->newbie_start_silver,"newbie_start_silver");	
	ENCODE_INTH(game_settings->newbie_start_practice,"newbie_start_practice");	
	ENCODE_INTH(game_settings->newbie_start_train,"newbie_start_train");	
	ENCODE_INT(top_area);
	ENCODE_INT(top_room); 
	ENCODE_INT(top_shop); 
	ENCODE_INT(top_mob_index); 
	ENCODE_INT(mobile_count); 
	ENCODE_INT(mudprog_count); 
	ENCODE_INT(top_obj_index); 
	ENCODE_INT(top_help); 
	ENCODE_INT(social_count);
	ENCODE_INTH(count_active_players(), "active_player_count");
	ENCODE_INTH(count_player_list(), "current_playerlist_count");
	ENCODE_INT(max_on);
	ENCODE_INT(LEVEL_IMMORTAL);
	ENCODE_INT(MAX_LEVEL);
	ENCODE_INT((int)current_time);
	ENCODE_INT((int)boot_time);
	ENCODE_INT((int)laston_since);

#ifdef __DATE__
	ENCODE_STRH(__DATE__,"compiled_date");
#endif
#ifdef __TIME__
	ENCODE_STRH(__TIME__,"compiled_time");
#endif
#if defined (__CYGWIN__)
	ENCODE_STRH("cygwin","compiled_platform");
#elif defined (WIN32)
	ENCODE_STRH("Win32","compiled_platform");
#elif defined (unix) || defined (__unix__)
#	if defined (linux)
		ENCODE_STRH("linux","compiled_platform");
#	elif defined (__OpenBSD__)
		ENCODE_STRH("OpenBSD","compiled_platform");
#	elif defined (__FreeBSD__)
		ENCODE_STRH("FreeBSD","compiled_platform");
#	elif defined (__NetBSD__)
		ENCODE_STRH("NetBSD","compiled_platform");
#	elif defined(__APPLE__) && defined(__MACH__)
		ENCODE_STRH("MacOSX","compiled_platform");
#	elif defined(BSD)
		ENCODE_STRH("BSD","compiled_platform");
#	else
		ENCODE_STRH("unix","compiled_platform");		
#	endif
#else
	ENCODE_STRH("unknown","compiled_platform");
#endif
	// possible compiler thing of interest
#ifdef __VERSION__ // e.g. "2.96 20000731 (Red Hat Linux 7.1 2.96-85)"
	ENCODE_STRH(__VERSION__,"compiler_version");
#endif 

	ENCODE_STRH(PLATFORM_INFO,"platform_info");	

	// retrieve a condensed version of laston stats output
	// will be able to be used to see how common classes/races are 
	// between muds, in addition stats about mccp usage etc.
	chImmortal->desc->colour_mode	= CT_NOCOLOUR;
	char *laston_stats=str_dup(doWebFunction(&do_lastonstats, "", MAX_LEVEL));
	chImmortal->desc->colour_mode	= CT_HTML;
	laston_stats=string_replace_all(laston_stats, "  ", " ");
	laston_stats=string_replace_all(laston_stats, "==", "=");
	laston_stats=string_replace_all(laston_stats, "\r", "");
	char *laston_stats_no_header=strstr(laston_stats, "-=");
	if(!laston_stats_no_header){
		laston_stats_no_header=laston_stats;
	}
	ENCODE_STRH(laston_stats_no_header,"laston_stats");
	free_string(laston_stats);

	laston_stats=str_dup(laston_generate_mud_client_stats());
	laston_stats=string_replace_all(laston_stats, "\r", "");
	ENCODE_STRH(laston_stats,"mudclient_stats");
	free_string(laston_stats);

	char *encoded=&stats[1];
	sprintf(result, 
		"POST " DAWNSTAT_SUBMIT_URL "  HTTP/1.1\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Host: " DAWNSTAT_SUBMIT_DOMAIN "\r\n"
		"User-Agent: Mozilla/4.0 (compatible; DawnWebSubmit/1.0)\r\n"
		"Content-Length: %d\r\n"
		"Cache-Control: no-cache\r\n"
		"\r\n%s",
		str_len(encoded), 
		encoded);

	return result;
}
/**************************************************************************/
char *dawnstat_stattext_to_post;
char *dawnstat_post_response_position;
char dawnstat_post_response[MSL];
/**************************************************************************/
const char *get_winsock_error_text(int errorcode);
bool init_winsock();
int write_to_socket( dawn_socket output_socket, const char *txt, int length );
/**************************************************************************/
enum dawnstat_stages
{
	DAWNSTATSTAGE_WAIT,
	DAWNSTATSTAGE_RESOLVING_DOMAIN_REMOTE,
	DAWNSTATSTAGE_RESOLVING_DOMAIN_LOCAL,
	DAWNSTATSTAGE_DOMAIN_RESOLVED,
	DAWNSTATSTAGE_INITIATE_CONNECTION,
	DAWNSTATSTAGE_CONNECT_IN_PROGRESS,
	DAWNSTATSTAGE_GENERATE_STATS,
	DAWNSTATSTAGE_POSTING,
	DAWNSTATSTAGE_CHECK_POST_ACCEPTANCE,
	DAWNSTATSTAGE_CLOSE_SOCKET,
	DAWNSTATSTAGE_COMPLETED,
	DAWNSTATSTAGE_ABORTED
};

dawnstat_stages dawnstat_stage=DAWNSTATSTAGE_WAIT;
time_t dawnstat_connect_timeout=0;
time_t dawnstat_resolve_timeout=0;

static dawn_socket dawnstat_socket;
int dawnstat_address_count;
char dawnstat_connection_address_and_port[MIL];
/**************************************************************************/
// - resolve domain name, performed once per reboot
static void dawnstat_resolve_domain_directly()
{
	resolver_address_found=false;
	resolver_address_failed=false;
	dawnstat_stage=DAWNSTATSTAGE_RESOLVING_DOMAIN_LOCAL;
	dawnstat_resolve_timeout=current_time+20;

	resolverlocal_queue_command(FORMATF("resolve system %s.", DAWNSTAT_SUBMIT_DOMAIN));	

}
/**************************************************************************/
// - resolve domain name, performed once per reboot
static void dawnstat_resolve_domain()
{
	dawnstat_logf("starting resolving of '%s.'", DAWNSTAT_SUBMIT_DOMAIN);

	if(!resolver_running){
		dawnstat_logf("The resolver isn't running, using direct resolution.");
		dawnstat_resolve_domain_directly();
		return;
	}

	if(resolver_version<1500){
		dawnstat_logf("Old resolver version running, resolving directly.");
		dawnstat_resolve_domain_directly();
		return;
	}

	resolver_send_data(FORMATF("resolve system %s.", DAWNSTAT_SUBMIT_DOMAIN));
	resolver_address_found=false;
	resolver_address_failed=false;

	dawnstat_stage=DAWNSTATSTAGE_RESOLVING_DOMAIN_REMOTE;
	dawnstat_resolve_timeout=current_time+100;
}

/**************************************************************************/
// if a connection failed on a given ip address, move onto the next ip
// if there are no remaining ip's abort the entire process
static void dawnstat_connection_attempt_failed()
{
	dawnstat_address_count++;
	resolve_result_address *addr=resolve_result_address_list->get(dawnstat_address_count);
	if(!addr || IS_NULLSTR(addr->address)){
		dawnstat_stage=DAWNSTATSTAGE_ABORTED;
		dawnstat_logf("dawnstat connect aborted.");
	}else{
		dawnstat_stage=DAWNSTATSTAGE_INITIATE_CONNECTION;
	}
}

/**************************************************************************/
// this should only be run once per address in the list of addresses
static void dawnstat_initiate_connection()
{
	//	initiate connection to DAWNSTAT_SUBMIT_DOMAIN
	resolve_result_address *addr=resolve_result_address_list->get(dawnstat_address_count);
	if(!addr || IS_NULLSTR(addr->address)){
		dawnstat_connection_attempt_failed();
		return;
	}

	if(addr->ipv6){
		sprintf(dawnstat_connection_address_and_port, "[%s]:80", addr->address);
	}else{
		sprintf(dawnstat_connection_address_and_port, "%s:80", addr->address);
	}

	dawnstat_logf("initiating connection to %s", dawnstat_connection_address_and_port);

	// free a previous socket if it hasn't been already freed
	if(dawnstat_socket){
		closesocket(dawnstat_socket);
		dawnstat_socket=0;
	}

	// setup the socket address structure for where we want to connect to
#ifdef IPV6_SUPPORT_ENABLED
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));		
	hints.ai_flags=AI_NUMERICHOST; // we have an ip address
	if(addr->ipv6){
		hints.ai_family   = AF_INET6;
	}else{
		hints.ai_family   = AF_INET;
	}
	hints.ai_socktype = SOCK_STREAM;
	if(res){
		freeaddrinfo(res);
		res=NULL;
	}
	int r = getaddrinfo(addr->address, // ip address in text form
						"80", &hints, &res);
	// check that it was converted successfully
	if (r) {
		#ifdef WIN32
			if(r==WSAHOST_NOT_FOUND){
				dawnstat_logf("getaddrinfo error %d - couldn't convert '%s' to a valid ip address.", 
					r, addr->address);
			}else if(r==WSAEAFNOSUPPORT && addr->ipv6){
				dawnstat_logf("\n"
					"getaddrinfo error %d - couldn't convert '%s' to a valid ip address\n"
					" - getaddrinfo reported this system has no support for the AF_INET6 address family.\n"
					"   This error message is normal if this system doesn't support ipv6, as it\n"
					"   was an ipv6 address getaddrinfo() was asked to convert.", 
					r, addr->address);
			}else{
				dawnstat_logf("dawnstat_initiate_connection(): getaddrinfo(%s) error %d", 
					addr->address, r);
			}
		#else
			{
				dawnstat_logf("dawnstat_initiate_connection(): getaddrinfo(%s) error %d - '%s'", 
					addr->address, r, gai_strerror(r));			
			}
		#endif
		dawnstat_connection_attempt_failed();
		return;
	}
#else // !IPV6_SUPPORT_ENABLED
	if(addr->ipv6){
		dawnstat_logf("skipping connection to %s "
			"- ipv6 address format not supported on this compile", 
			dawnstat_connection_address_and_port);
		dawnstat_connection_attempt_failed();
		return;
	}

	if(res){
		delete res;
		res=NULL;
	}
	res=new ipv4only_addrinfo;
	res->ai_next=NULL;
	res->ai_family=AF_INET;
	res->ai_socktype=SOCK_STREAM;
	res->ai_protocol=IPPROTO_TCP;
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);
	sa.sin_addr.s_addr= inet_addr( addr->address);
	if(sa.sin_addr.s_addr==INADDR_NONE){ 
		// we had an invalid address supplied
		// technically INADDR_NONE can be returned by the valid ip 255.255.255.255
		// but 255.255.255.255 isn't useful in this context so we assume input was invalid.
		dawnstat_logf("'%s' was not a valid ipv4 address.", addr->address);
		dawnstat_connection_attempt_failed();
		return;
	}
	res->ai_addr=(struct sockaddr *) &sa;
	res->ai_addrlen=sizeof(sa);
#endif
	dawnstat_socket= socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (dawnstat_socket== dawn_socket_INVALID_SOCKET){
		dawnstat_logf("Error creating connection socket");
		dawnstat_connection_attempt_failed();
		return;
	}


	{ // set the socket to non-blocking mode
#ifdef WIN32
		unsigned long blockmode = 1; 
		if(ioctlsocket(dawnstat_socket, FIONBIO, &blockmode)== SOCKET_ERROR)    
		{        
			dawnstat_logf("ioctlsocket: error setting new socket to nonblocking, "
				"WSAGetLastError=%d", WSAGetLastError());        
			dawnstat_connection_attempt_failed();
			return;    
		}  
#else
		if ( fcntl(dawnstat_socket, F_SETFL, O_NONBLOCK)< 0 ){
			dawnstat_logf("fcntl: error setting new socket to nonblocking");
			dawnstat_connection_attempt_failed();
			return;    
		}
#endif
	}

	// start the connection	

	int nRet= connect(dawnstat_socket, res->ai_addr, (socklen_t)res->ai_addrlen);

	// check for instant results
	
	if(nRet==0){ // successful connection, jump straight to generating the stats to post
		dawnstat_logf("dawnstat_initiate_connection(): immediate connect, jumping to generate stats.");
		dawnstat_stage=DAWNSTATSTAGE_GENERATE_STATS;
		return;
	}

	// check the error codes, if we have anything other than 
	// what is normal for a blocked connect call, we abort the connection
#ifdef WIN32
	int WSALastError=WSAGetLastError();
	if(WSALastError!=WSAEWOULDBLOCK){
		dawnstat_logf("dawnstat_initiate_connection(): connect() error %s",
			get_winsock_error_text(WSALastError));
		dawnstat_connection_attempt_failed();
		return;
	}
#else
	if(nRet<0){
		if (errno != EINPROGRESS && errno != EALREADY) 
		{
			dawnstat_logf("dawnstat_initiate_connection(): connect() error %d", errno);
			dawnstat_connection_attempt_failed();
			return;
		}
	}
#endif

	// the connection process has started, and is now in progress
	dawnstat_connect_timeout=current_time+120; // 2 minutes to connect
	dawnstat_logf("connection initiation successful");
	dawnstat_stage=DAWNSTATSTAGE_CONNECT_IN_PROGRESS;
	return;
}
/**************************************************************************/
static void dawnstat_process_connect()
{
	// handle the timeout first
	if(dawnstat_connect_timeout<current_time){
		dawnstat_logf("dawnstat_process_connect(): pending connection timed out.");
		dawnstat_connection_attempt_failed();
		return;
	}

	int nRet;
	dawnstat_logf("processing connection %s", dawnstat_connection_address_and_port);
#ifdef WIN32
	{ 
		// use a select call to see if the socket to be ready for writing
		// set our select_timeout, to 0 seconds
		struct timeval select_timeout;
		select_timeout.tv_sec=0;
		select_timeout.tv_usec=0;

		// prepare our file descriptor set
		fd_set fdWrite; // connect success is reported here
		FD_ZERO(&fdWrite);
		FD_SET(dawnstat_socket, &fdWrite);
		fd_set fdExcept; // connect failure is reported here
		FD_ZERO(&fdExcept);
		FD_SET(dawnstat_socket, &fdExcept);

		// check if the socket is ready
		nRet=select((int)dawnstat_socket+1, NULL, &fdWrite, &fdExcept, &select_timeout);

		dawnstat_logf("select() returned %d", nRet);
		if(nRet<1){
			if(nRet==0){
				// it isn't ready yet - wait longer
			}else{
				dawnstat_logf("dawnstat_process_connect(): select() returned error %d", nRet);
				dawnstat_connection_attempt_failed();		
			}
			return;
		}

		if(FD_ISSET( dawnstat_socket, &fdWrite )){
			// socket is ready for writing
			dawnstat_logf("dawnstat_process_connect(): connection established.");
			dawnstat_stage=DAWNSTATSTAGE_GENERATE_STATS;
			return;
		}

		if(FD_ISSET( dawnstat_socket, &fdExcept )){
			// connection failed
			dawnstat_logf("dawnstat_process_connect(): connection failed.");
		}else{		
			dawnstat_logf("dawnstat_process_connect(): don't know how we got here!");
		}
		dawnstat_connection_attempt_failed();
		return; 
	}
#else
	// just attempt to connect the socket again using connect(), 
	// if the socket is now connected, connect() will return 0 or error 56 (already connected)
	nRet= connect(dawnstat_socket, res->ai_addr, res->ai_addrlen);
	if(nRet==0){
		// successful connection, generate the stats to post
		dawnstat_stage=DAWNSTATSTAGE_GENERATE_STATS;
		return;
	}
	if(nRet<0){
		if(errno==EISCONN){
			// we consider the connection successful, generate the stats to post
			dawnstat_logf("dawnstat_process_connect(): socket connected.");
			dawnstat_stage=DAWNSTATSTAGE_GENERATE_STATS;
			return;
		}

		if (errno != EINPROGRESS && errno != EALREADY){
			dawnstat_logf("dawnstat_process_connect(): connect() error %d", errno);
			dawnstat_connection_attempt_failed();
			return;
		}
	}
#endif
	return;
}
/**************************************************************************/
static void dawnstat_generate_stats()
{
	dawnstat_stattext_to_post=dawnstat_generate_statistics_text();
	dawnstat_stage=DAWNSTATSTAGE_POSTING;
}
/**************************************************************************/
static void dawnstat_post()
{
	char *msg=dawnstat_stattext_to_post;

	int written;
	int msglen=str_len(msg);

	written=write_to_socket(dawnstat_socket, msg, msglen);
	
	if(written<0){ // check for an error
		dawnstat_logf("dawnstat_post(): An error occurred posting statistics.");
		dawnstat_connection_attempt_failed();		
		return;
	}

	// check for an incomplete write
	if(written<msglen){
		dawnstat_logf("Incomplete write, sent %d bytes of %d, write rest later", written, msglen);
		dawnstat_stattext_to_post+=written;
		return;
	}

	// completed write
	dawnstat_logf("Submitted %d bytes", written);
	
	memset(dawnstat_post_response, 0, sizeof(dawnstat_post_response));	
	dawnstat_post_response_position=dawnstat_post_response;
	dawnstat_stage=DAWNSTATSTAGE_CHECK_POST_ACCEPTANCE;	
}

/**************************************************************************/
static void dawnstat_check_post_acceptance()
{
	int len=sizeof(dawnstat_post_response)-str_len(dawnstat_post_response)-10;
	int read=recv(dawnstat_socket, dawnstat_post_response_position, len, 0);
	
	if(read<0){ // check for an error
		dawnstat_logf("dawnstat_check_post_acceptance(): An error occurred checking for "
			"the post acceptance, posting to next ip or failing.");
		dawnstat_connection_attempt_failed();
		return;
	}

	// the response will be formatted in the form:
	// "HTTP/1.1" <space> <status code> <space> <text description of status code>\n"
	// where <status code> is a 3 http status digit code, 200 is considered successful
	// we only need to read 2 space characters, in order to get the status code
	if(count_char(dawnstat_post_response, ' ')<2){
		dawnstat_logf("Incomplete read, will continue read next time around.");
		dawnstat_logf("Read response so far: '%s'", dawnstat_post_response);
		dawnstat_post_response_position=&dawnstat_post_response[str_len(dawnstat_post_response)];
		return;
	}

	// completed read, get the status code
	char *start=strstr(dawnstat_post_response," ");
	if(!start){
		// shouldn't be possible to get here unless someone has introduced bugs into count_char()
		dawnstat_logf("dawnstat_check_post_acceptance(): can't find spaces to get start of status code!");
		dawnstat_connection_attempt_failed();
		return;
	}
	start++;
	char *end=strstr(start," ");
	if(!end){
		// shouldn't be possible to get here unless someone has introduced bugs into count_char()
		dawnstat_logf("dawnstat_check_post_acceptance(): can't find spaces to get end of status code!");
		dawnstat_connection_attempt_failed();
		return;
	}
	*end='\0';
	if(!is_number(start)){
		// confirm the status code we are checking is an actual number
		dawnstat_logf("dawnstat_check_post_acceptance(): status code found isn't a number for some reason!");
		dawnstat_connection_attempt_failed();
		return;
	}
	int status_code=atoi(start);
	*end=' ';
	if(status_code!=200){
		dawnstat_logf("post unsuccessful - http status code %d.", status_code);
		dawnstat_connection_attempt_failed();		
		return;
	}

	dawnstat_logf("post accepted - http status code 200.");
	dawnstat_stage=DAWNSTATSTAGE_CLOSE_SOCKET;	
}

/**************************************************************************/
void dawnstat_update() 
{
	static time_t wait_until=0;
	// this function is called every 10 seconds by default (PULSE_DAWNSTAT)
	// it is necessary to be called this often due to is non blocking io	
	if((int)dawnstat_stage<(int)DAWNSTATSTAGE_COMPLETED){
		if(!wait_until || dawnstat_stage!=DAWNSTATSTAGE_WAIT){
			dawnstat_logf("dawnstat_update(%d)", (int)dawnstat_stage);
		}
	}

	switch(dawnstat_stage){
		case DAWNSTATSTAGE_WAIT:
			{				
				if(wait_until==0){
					wait_until=current_time+DAWNSTAT_DELAY;
				}else if(wait_until<current_time){
					// time to move on
					wait_until=0;
					dawnstat_logf("moving on to resolving stage.");					
					dawnstat_resolve_domain();					
				}
			}
			break;

		case DAWNSTATSTAGE_RESOLVING_DOMAIN_REMOTE:
			dawnstat_logf("checking if domain has been resolved");

			if(resolver_address_found){
				dawnstat_stage=DAWNSTATSTAGE_DOMAIN_RESOLVED;
				for(resolve_result_address *node=resolve_result_address_list; node; node=node->next){
					dawnstat_logf("dawnstat address resolved as ipv%d '%s'", node->ipv6?6:4, node->address);
				}
			}
			if(dawnstat_resolve_timeout<current_time){
				dawnstat_logf("domain resolution timed out for resolver, trying local resolution.");
				dawnstat_resolve_domain_directly();
				return;
			}
			if(resolver_address_failed){
				dawnstat_logf("domain resolution failed via resolver, trying local resolution.");
				dawnstat_resolve_domain_directly();
				return;
			}
			break;

		case DAWNSTATSTAGE_RESOLVING_DOMAIN_LOCAL:
			dawnstat_logf("checking if domain has been resolved locally");
			if(resolver_address_found){
				dawnstat_stage=DAWNSTATSTAGE_DOMAIN_RESOLVED;
				for(resolve_result_address *node=resolve_result_address_list; node; node=node->next){
					dawnstat_logf("dawnstat address resolved as ipv%d '%s'", node->ipv6?6:4, node->address);
				}
				return;
			}
			if(dawnstat_resolve_timeout<current_time){
				dawnstat_logf("domain resolution timed out.");
				dawnstat_stage=DAWNSTATSTAGE_ABORTED;
				return;
			}
			
			if(resolver_address_failed){
				dawnstat_logf("domain resolution failed.");
				dawnstat_stage=DAWNSTATSTAGE_ABORTED;
				return;
			}
			break;
			
		case DAWNSTATSTAGE_DOMAIN_RESOLVED:
			dawnstat_address_count=0;
			dawnstat_stage=DAWNSTATSTAGE_INITIATE_CONNECTION;
			break;

		case DAWNSTATSTAGE_INITIATE_CONNECTION:
			dawnstat_initiate_connection();
			break;

		case DAWNSTATSTAGE_CONNECT_IN_PROGRESS:
			dawnstat_logf("processing connection.");
			dawnstat_process_connect();
			break;
			
		case DAWNSTATSTAGE_GENERATE_STATS:
			dawnstat_logf("generating statistics.");
			dawnstat_generate_stats();
			break;

		case DAWNSTATSTAGE_POSTING:
			dawnstat_logf("posting statistics.");
			dawnstat_post();
			break;

		case DAWNSTATSTAGE_CHECK_POST_ACCEPTANCE:
			dawnstat_logf("checking for post acceptance.");
			dawnstat_check_post_acceptance();
			break;

		case DAWNSTATSTAGE_CLOSE_SOCKET:
			dawnstat_logf("closing socket.");
			if(dawnstat_socket){
				closesocket(dawnstat_socket);
				dawnstat_socket=0;
			}			
			dawnstat_stage=DAWNSTATSTAGE_COMPLETED;
			break;

		case DAWNSTATSTAGE_COMPLETED:
		case DAWNSTATSTAGE_ABORTED:
			// redo dawnstats once per day if successful
			// or try again in 7 hours time if failed last time
			{
				static time_t redo_in=0;
				if(redo_in){
					if(redo_in<current_time){
						dawnstat_logf("restarting dawnstats");
						
						dawnstat_stage=DAWNSTATSTAGE_WAIT;
						redo_in=0;
					}
				}else{
					if(dawnstat_stage==DAWNSTATSTAGE_ABORTED){
						redo_in=current_time + (7*60*60); // every 7 hours
					}else{
						redo_in=current_time + (24*60*60); // every 24 hours
					}
				}

			}
		default:
			break;
	};


}

/**************************************************************************/
/**************************************************************************/

