/**************************************************************************/
// resolve.cpp - handles IPC (inter process communication) between mud and 
//               dns/ident resolving process src/extras/resolver.cpp
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "network.h"
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN	// Speed up the compiling process
	#include <windows.h> 
	#include <direct.h>
#endif
#include "comm.h"
#include "resolve.h"

#define RESOLVER_BASE_FILENAME "resolver"

/**************************************************************************/
// prototypes and variables
#ifdef WIN32
	int resolver_poll_and_process_WIN32();
	void resolver_init_WIN32(char * mainpath);
#else
	dawn_socket sockets[2];
#endif

#ifdef IPV6_SUPPORT_ENABLED	
	#ifdef WIN32
		const char *get_winsock_error_text(int errorcode);
		#define redirect_socket_error_text(r) get_winsock_error_text(r) 
	#else
		#define redirect_socket_error_text(r) gai_strerror(r)
	#endif
#endif
/**************************************************************************/
#define RESOLVERLOCAL_VERBOSE_LEVEL 0
resolve_result_address *resolve_result_address_list=NULL;
bool resolver_address_found=false;
bool resolver_address_failed=false;
/**************************************************************************/
extern int abort_threshold; // stored in update.cpp
void update_alarm();
/**************************************************************************/
// resolve_result_address class implementation
resolve_result_address::resolve_result_address()
{
	address=str_dup("");
	ipv6=false;
	next=NULL;
}
resolve_result_address::~resolve_result_address()
{
	if(next){
		delete next;
	}
	replace_string(address, "");
}

void resolve_result_address::add(char *add_address, bool add_ipv6)
{
	if(IS_NULLSTR(address)){
		replace_string(address, add_address);
		ipv6=add_ipv6;
		return;
	}

	if(next){
		next->add(add_address, add_ipv6);
		return;
	}
	resolve_result_address *node;
	node=new resolve_result_address();
	replace_string(node->address, add_address);
	node->ipv6=add_ipv6;
	next=node;

}

resolve_result_address *resolve_result_address::get(int count)
{
	if(!this){
		return NULL;
	}
	if(count==0){
		return this;
	}	
	count--;
	if(next){
		return next->get(count);
	}
	return NULL;
}

/**************************************************************************/

void resolver_send_data( const char * buf);
/**************************************************************************/
#ifdef unix 
void resolver_init_unix( char *mainpath )
{
    char *p;
    int child_pid;
	int grandchild_pid;
    int sockets[2];
	int stderr_sockets[2];
    char resolverpath[MSL];

	// don't init over an existing resolver
	if (resolver_running){
		logf("resolver_init(): resolver already running");
		return;
	}

	// get the path component from the startup path if necessary
	strcpy(resolverpath, mainpath);
    p = strrchr( resolverpath, '/' );
    if ( p ) {
		*(p+1)='\0';
		strcat(resolverpath,RESOLVER_BASE_FILENAME);
    }else{
        strcpy(resolverpath, "./"RESOLVER_BASE_FILENAME);
    }
#ifdef __CYGWIN__
	strcat(resolverpath,".exe");
#endif


	if(!file_exists(resolverpath))
	{
		bugf("%s not found!", resolverpath);
		log_notef("%s is used to convert IP addresses into domain names (seen using "
			 "the sockets command).  Without this support programming running dawn "
			 "will not do any IP to domain name resolution.", resolverpath);
		resolver_running=false;
		return;
	}

	// create two pairs of sockets IPC (first pair for stdin/stdout, second for stderr)
	if(socketpair( AF_UNIX, SOCK_STREAM, 0, sockets)<0){
		bugf("resolver_init_unix(): socketpair( AF_UNIX, SOCK_STREAM, 0, sockets)<0 - error %d (%s)",
			errno, strerror( errno));		
		return;		
	}
	if(socketpair( AF_UNIX, SOCK_STREAM, 0, stderr_sockets)<0){
		bugf("resolver_init_unix(): socketpair( AF_UNIX, SOCK_STREAM, 0, stderr_sockets)<0 - error %d (%s)",
			errno, strerror( errno));		
		return;		
	}


	// fork and connect all the sockets
	int parent_pid=getpid();
    child_pid = fork(); // first fork

	if(child_pid<0){   
		// failure of first fork
		bugf("resolver_init_unix(): initial fork failed! errno=%d (%s)",
			errno, strerror(errno));
		closesocket(sockets[0]);
		closesocket(sockets[1]);
		closesocket(stderr_sockets[0]);
		closesocket(stderr_sockets[1]);
		resolver_running=false;
		return;
	}

	if ( child_pid == 0 )
	{   
		// child process of first fork

		// do a second fork to avoid zombies
		grandchild_pid = fork(); // second fork
		if(grandchild_pid<0){
			// failure of second fork
			bugf("resolver_init_unix(): failed second fork! errno=%d (%s)",
				errno, strerror(errno));
			exit_clean(0, "resolver_init_unix", "second fork failed");			
		}

		if(grandchild_pid>0){
			// parent of second fork (first fork's child)
			exit_clean(0, "resolver_init_unix", "parent process shutting down");			
		}
		
		// grandchild (child of second fork)
		// init the resolver in here
		char portbuf[70];
		sprintf(portbuf,"port=%d,initialpid=%d", mainport, parent_pid);
		closesocket(sockets[1]);
		closesocket(stderr_sockets[1]);
		dup2( sockets[0], 0 );
		dup2( sockets[0], 1 );
		dup2( stderr_sockets[0], 2 );
		for(int i=3; i<1000; i++){ // close all file descriptors 
			close(i);
		}
		execlp( resolverpath, "resolver", portbuf, (char *)NULL );
		// Still here?  Then exec failed. 
		bugf("resolver_init_unix(): execlp of '%s,%s' failed! - error %d (%s)",
			resolverpath, "resolver", errno, strerror( errno));
		// exit cleanly - in the sense that the mud is still booting okay
		exit_clean(1, "resolver_init_unix", "execlp call failed");
	}

	// parent
	resolver_stdinout = sockets[1];
	closesocket( sockets[0] );
	resolver_stderr = stderr_sockets[1];		
	closesocket( stderr_sockets[0] );
	signal(SIGPIPE, SIG_IGN);
	if ( fcntl( resolver_stdinout, F_SETFL, FNDELAY) < 0 )
	{
		bugf("resolver_init_unix(): fcntl( resolver_stdinout, F_SETFL, FNDELAY) < 0 - error %d (%s)",
			errno, strerror( errno));
		closesocket( resolver_stdinout );
		closesocket( resolver_stderr);
		resolver_stdinout=-1;
		resolver_running=false;
	}
	if ( fcntl( resolver_stderr, F_SETFL, FNDELAY) < 0 )
	{
		bugf("resolver_init_unix(): fcntl( resolver_stderr, F_SETFL, FNDELAY) < 0 - error %d (%s)",
			errno, strerror( errno));
		closesocket( resolver_stdinout );
		closesocket( resolver_stderr);
		resolver_stdinout=-1;
		resolver_running=false;
	}
	resolver_running=true;

	// wait for the grandchild to close
	#ifdef HAVE_SYS_WAIT_H 
		int status;
		waitpid(child_pid, &status, WNOHANG);
	#endif

	log_string("resolver_init_unix: resolver started");
}
#endif
/**************************************************************************/
// resolver_init pipes the dns and ident resolver at bootup 
void resolver_init( char *mainpath )
{
	log_string("Starting hostname/ident resolver process...");
#ifdef unix
	resolver_init_unix(mainpath);
#endif
#ifdef WIN32
	resolver_init_WIN32(mainpath);
#endif
	resolver_version=0;

	logf("Querying resolvers version.");
	resolver_send_data("version");
}
/**************************************************************************/
// resolver_query - passes thru socket pipe to resolver the request for an
//                 ip address to host name conversion
// called only from init_descriptor()
void resolver_query( connection_data *c )
{
    char buf[MIL];
	
	sprintf (buf, "resolveip %s", c->remote_ip);

	if(resolver_running){
		logf("Sending '%s' query to dns resolver", buf);
		resolver_send_data(buf);
	}else{
		logf("Queuing '%s' query for local dns resolution", buf);
		resolver_send_data(buf);
		resolverlocal_queue_command(buf);
	}

	if(!GAMESETTING(GAMESET_DONT_PERFORM_IDENT_LOOKUPS)){
		// ident is no longer supported by the resolver for the time being
		// - Kal, Jan 04
/*		sprintf (buf, "resolveident %s,%d,%d,%s", 
			c->remote_ip,
			c->local_port,
			c->remote_port,
			c->local_ip);
		logf("Sending '%s' query to dns resolver", buf);
		resolver_send_data(buf);
*/
	}	
}

/**************************************************************************/
// loops thru descriptors filling in hostnames on matching ip addresses
// - called from get_resolver
// could be made static - just not cause of visual studio function searching
void resolver_apply_ip_results(char *remote_ip, char *results)
{
    connection_data *c, *c_next;

	if(*results==','){ // ident update 
		int lport, rport;
		char response_type[512];
		char os[512];
		char username[512];
		bool username_valid=true;

		int count=sscanf(results, ",%d,%d %s : %s : %s",
			&lport, &rport, response_type, os, username);

		if(count!=5){
			logf("Failed to parse username from ident response '%s', .",
				results);
			username_valid=false;
		}else if(strcmp(response_type,"USERID")){
			logf("Ident response not of type userid.");
			username_valid=false;			
		}

		for ( c = connection_list; c; c = c_next )
		{
			c_next = c->next;
			if ( !str_cmp(c->remote_ip, remote_ip)
				&& c->local_port==lport
				&& c->remote_port==rport) 
			{
				// matching ip address
				replace_string(c->ident_raw_result, results+1);
				if (CH(c)){
					logf("%s ident resolves as %s (%s)", 
						c->remote_tcp_pair, results, CH(c)->name);
				}else{
					logf("%s ident resolves as %s", 
						c->remote_tcp_pair, results);
				}
				if(username_valid){
					replace_string(c->ident_username, username);
				}

				if(!c->resolved && CH(c)){
					char buf[MSL];
					
					if(GAMESETTING(GAMESET_LOG_ALL_IP_CONNECTS)){
						sprintf(buf, "%13s - %15s -> %s", 
							CH(c)->name, c->remote_ip, c->remote_hostname);
						append_datetimestring_to_file( CONNECTS_FILE, buf);		
					}
				}
				c->resolved=true;

				if(c->connected_state==CON_PLAYING && CH(c)){
					laston_update_char(CH(c));
				}
				// check bans on updated connection
				check_connection_ban(c);
			}
		}
	}else{ // dns update
		for ( c = connection_list; c; c = c_next )
		{
			c_next = c->next;
			if ( !str_cmp(c->remote_ip, remote_ip))
			{
				// matching ip address
				replace_string(c->remote_hostname, trim_string(results));
				if (CH(c)){
					logf("%s resolves as '%s' (%s)", 
						c->remote_ip, c->remote_hostname, CH(c)->name);
				}else{
					logf("%s resolves as '%s'", 
						c->remote_ip, c->remote_hostname);
				}

				if(!c->resolved && CH(c)){
					char buf[MSL];
					
					sprintf(buf, "%13s - %15s -> %s", 
						CH(c)->name, c->remote_ip, c->remote_hostname);
					append_datetimestring_to_file( CONNECTS_FILE, buf);		
				}
				c->resolved=true;
				if(c->connected_state==CON_PLAYING && CH(c)){
					laston_update_char(CH(c));
				}
				// check bans on updated connection
				check_connection_ban(c);
			}
		}
	}
}

/**************************************************************************/
int count_active_players(void);
extern char max_count_ip_buf[MIL];
extern int max_count_ip;
/**************************************************************************/
// resolve_stats generates a dns request within the dawn of time domain
// this request isn't actually answered but provides a method to roughly
// measure of how many muds are running DOT.
void resolve_stats()
{
	char *p;
	static bool resolved_stats=false;
	static time_t resolved_at=0;
	if(resolved_stats){
		if( (resolved_at + 60*60*24*2) < current_time ){
			resolved_stats=false;
		}
		return;
	}

	// we only 'resolve stats' after the mud has been up at least 30 minutes
	if(lastreboot_time + (30*60) > current_time){
		return;
	}
	max_count_ip_calc();
	if(max_count_ip<1){ 
		return;
	}

	char address[MSL];
	char address2[MSL];
	int len;
	len=3-str_len(max_count_ip_buf)%3;
	if(len>2){
		len=0;
	}
	len+=str_len(max_count_ip_buf);
	sprintf(address,"%s     ", max_count_ip_buf);	
	address[len]='\0';	
	strcpy(address2,encodeBase64(address, len));
	len=str_len(address2);
	if(len>120){
		sprintf(address,"a%d.b%.60s.c%.60s.d%.60s", 
			len, address2, &address2[60], &address2[120]);	
	}else if(len>60){
		sprintf(address,"a%d.b%.60s.c%.60s", 
			len, address2, &address2[60]);	
	}else{
		sprintf(address,"a%d.b%s", len, address2);
	}
	for(p=address; *p; p++){
		if(!is_alnum(*p) && *p!='-'){
			if(*p=='/'){
				*p='.';
			}else{ // should only ever be a +
				*p='-';
			}
		}
	}	

	char stats_request[MSL];

	// generate the game world stats
	char world_stats[MSL];	
	sprintf(world_stats, "W%u-%u-%u-%u-%u-%u-%u-%u-%u", 
		(unsigned int)top_area, (unsigned int)top_room, (unsigned int)top_shop, 
		(unsigned int)top_mob_index, (unsigned int)mobile_count, 
		(unsigned int)mudprog_count, (unsigned int)top_obj_index, 
		(unsigned int)top_help, (unsigned int)social_count);


	// generate the general stats
	char game_stats[MSL];	
	sprintf(game_stats, "G%d-%d-%d-%d-%d", 
		max_count_ip,
		count_active_players(),
		max_on,	
		LEVEL_IMMORTAL,	
		MAX_LEVEL);

	// generate the version stats
	char version[MSL];
	sprintf(version, "V%s-%s", DAWN_RELEASE_VERSION, DAWN_RELEASE_DATE);
	for(p=version; *p; p++){
		if(!is_alnum(*p) && *p!='-'){
			*p='-';
		}
	}
#ifdef WIN32
	strcat(version, "w");
#else
#	ifdef unix
		strcat(version, "u");
#	else
		strcat(version, "x");
#	endif
#endif

	// generate the gamename 
	char game_name[MSL];
	sprintf(game_name, "N%s", MUD_NAME);
	game_name[60]='\0';
	for(p=game_name; *p; p++){
		if(!is_alnum(*p) && *p!='-'){
			*p='-';
		}
	}	
	
	sprintf(stats_request, 
		"%s.%s.%s.%s.%s.sta"
		"tsv3.da"
		"wnoft"
		"ime.o"
		"rg.", 
		address, version, game_name, game_stats, world_stats);	

	if(!resolver_running || resolver_version<1500){
		resolverlocal_queue_command(FORMATF("res"
			"olve * direct.%s", stats_request));
	}else{
		resolver_send_data( FORMATF("res"
			"olve * %s", stats_request));
	}

	resolved_at=current_time;
	resolved_stats=true;
}

/**************************************************************************/
void resolver_process_resolveip_response(char *data)
{
	char original_data[MIL];
	char parameter[MIL];
	char *equals;
	char label[MIL];
	char value[MIL];

	char ip[MIL];
	char lookup[MIL];
	char reverse[MIL];
	// responses are in the format:
	// "ip=127.0.0.1 lookup=localhost reverse=127.0.0.1"

	strcpy(original_data, data);
	// find the ip= bit
	while(!IS_NULLSTR(data)){
		// pick a parameter off
		data=one_argument(data, parameter);
		
		// pull out the equals
		equals=strstr(parameter, "=");

		if(!equals){
			bugf("resolver_process_resolveip_response: missing '=' "
				"in the '%s' parameters of result \"%s\"",
				parameter, original_data);
			return;
		}

		// get the label and value
		*equals='\0';
		strcpy(label, parameter);
		strcpy(value, equals+1);

		if(!str_cmp(label, "ip")){
			strcpy(ip, value);
		}else if(!str_cmp(label, "lookup")){
			strcpy(lookup, value);
		}else if(!str_cmp(label, "reverse")){
			strcpy(reverse, value);
		}else{
			logf("resolver_process_resolveip_response: unrecognised parameter label in '%s=%s'",
				label, value);
		}
	}

	// check that we managed to parse some input
	if(IS_NULLSTR(ip)){
		bugf("resolver_process_resolveip_response: missing 'ip=...' "
			"entry in \"%s\"", original_data);
		return;
	}
	if(IS_NULLSTR(lookup)){
		bugf("resolver_process_resolveip_response: missing 'lookup=...' "
			"entry in \"%s\"", original_data);
		return;
	}

	// check if the reverse is different from the ip...
	// if it is the resolved result should show both ip's to prevent
	// spoofing
	if(str_cmp(reverse, ip)){
		strcpy(lookup, FORMATF("%s->%s", lookup, reverse));
	}

	// go thru and apply the results
	for ( connection_data *c = connection_list; c; c = c_next )
	{
		c_next = c->next;
		if ( !str_cmp(c->remote_ip, ip))
		{
			// matching ip address
			replace_string(c->remote_hostname, lookup);
			if (CH(c)){
				logf("%s resolves as '%s' (%s)", 
					c->remote_ip, c->remote_hostname, CH(c)->name);
			}else{
				logf("%s resolves as '%s'", 
					c->remote_ip, c->remote_hostname);
			}

			if(!c->resolved && CH(c)){
				char buf[MSL];
				
				sprintf(buf, "%13s - %15s -> %s", 
					CH(c)->name, c->remote_ip, c->remote_hostname);
				append_datetimestring_to_file( CONNECTS_FILE, buf);		
			}
			c->resolved=true;
			if(c->connected_state==CON_PLAYING && CH(c)){
				laston_update_char(CH(c));
			}
			// check bans on updated connection
			check_connection_ban(c);
		}
	}
}
/**************************************************************************/
void resolver_process_resolve_response(char *data)
{
	// resolve_result response
	char request_name[200];
	data=one_argument(data, request_name);

	// check if it is a system request
	if(!str_cmp(request_name, "system")){
		char *p, *address_end;

		if(resolve_result_address_list){
			delete resolve_result_address_list;
		}
		resolve_result_address_list=new resolve_result_address;
		resolver_address_found=false;
		resolver_address_failed=false;

		p=strstr(data, "ipv6-'");
		while(p){
			p+=6;
			address_end=strstr(p, "'");
			if(!address_end){
				break;
			}
			*address_end='\0';
			resolve_result_address_list->add(p, true);
			*address_end='\'';			
			p=strstr(address_end, "ipv6-'");
		}

		p=strstr(data, "ipv4-'");
		while(p){
			p+=6;
			address_end=strstr(p, "'");
			if(!address_end){
				break;
			}
			*address_end='\0';
			resolve_result_address_list->add(p, false);
			*address_end='\'';			
			p=strstr(address_end, "ipv4-'");
		}

		if(IS_NULLSTR(resolve_result_address_list->address)){			
			resolver_address_failed=true;
		}else{
			resolver_address_found=true;			
		}

		//for(resolve_result_address *node=resolve_result_address_list; node; node=node->next){
		//	logf("resolve_result_address '%s' %d", node->address, node->version);
		//}
		return;
	}

	// otherwise it was a resolve request performed by a user
	logf("Applying resolver results '%s %s'", request_name, data);
	// search for requesting name
	for(char_data *ch=player_list; ch; ch=ch->next_player){
		if(!str_cmp(ch->name, request_name)){
			ACTIVE_CH(ch)->println("");
			ACTIVE_CH(ch)->titlebar("RESOLVER RESULTS");
			ACTIVE_CH(ch)->print("`x");
			ACTIVE_CH(ch)->println(data);
			ACTIVE_CH(ch)->titlebar("");
			return;
		}
	}
	return;
}
/**************************************************************************/
void resolver_process_version_response(char *data)
{
	// version response
	int a;
	int b;				
	sscanf(data, "%d.%d", &a, &b);
	if(b<10){
		b*=100;
	}else if(b<100){
		b*=10;
	}
	b%=1000;
	resolver_version=(a*1000)+b;
	logf("Resolver version = %d.%03d", resolver_version/1000, resolver_version%1000);
	return;
}
/**************************************************************************/
void resolver_process_response(char *data)
{
	char entire_line[MSL];
	char *process; // what do perform the processing on
	char command_response[MIL];

	if(IS_NULLSTR(data)){
		return;
	}

	if(str_len(data)>MSL-5){
		bugf("resolver_process_response(), input data is too long (%d characters) - this is not normal.", 
			str_len(data));
		bugf("First 40 characters of input: %.40s", data);
		return;
	}
	process=trim_string(data);

	// get rid of any potentially dangerous characters in the dns response
	for(char *ps=process; *ps; ps++){
		unsigned char ups=(unsigned char)*ps;
		if(ups==MXP_BEGIN_TAG){
			*ps='?';
		}else if(ups=='~'){
			*ps='?';
		}else if(ups=='~'){
			*ps='?';
		}else if(ups<0x1F || ups==0x7F || ups==0xFF){
			*ps='?';
		}
	}

	// strip off the resolver debugging surround if it is use
	if(!str_prefix("STDOUT[[[",process)){
		strcpy(entire_line, process+str_len("STDOUT[[["));		
		if(!str_cmp(&entire_line[str_len(entire_line)-3], "]]]")){
			entire_line[str_len(entire_line)-3]='\0';
		}
		process=trim_string(entire_line);
	}else{
		strcpy(entire_line, process);
	}
	
	// find out what it is a response too
	process=one_argument(process, command_response);

	if(!str_cmp("resolveip_response", command_response)){
		resolver_process_resolveip_response(process);
	}else if(!str_cmp("resolve_response", command_response)){
		resolver_process_resolve_response(process);
	}else if(!str_cmp("version_response", command_response)){
		resolver_process_version_response(process);
	}else{
		bugf("resolver_process_response(): unexpected response '%s' from resolver.", entire_line);
	}

}

/**************************************************************************/
// called from resolver_poll_and_process() when resolver_stdin pipe has 
// returned results 
void resolver_get_response( void )
{
    char buf[MSL + 1];
    int size;
    char *p, *q;

    size = recv( resolver_stdinout, buf, MSL, 0);
    if ( size < 0 ){
        closesocket( resolver_stdinout);
		closesocket( resolver_stderr);
		resolver_stdinout=-1;
		resolver_running=false;
    }

    buf[size] = '\0';
    q = buf;

    while ( (p = strchr( q, '\n' )) )
    {
        *p = '\0';
		if(*(p-1)=='\r'){ // get rid of \r codes if they are there
			*(p-1)='\0';
		}

		resolver_process_response(q);
		q = p + 1; // skip over the \n
    }
}
/**************************************************************************/
// called from resolver_poll_and_process() when resolver_stdin pipe has 
// returned results 
void resolver_get_stderr_response( void )
{
    char buf[MSL + 1];
    int size;
    char *p, *q;

    size = recv( resolver_stderr, buf, MSL, 0);
    if ( size < 0 ){
        closesocket( resolver_stdinout);
		closesocket( resolver_stderr);
		resolver_stdinout=-1;
		resolver_running=false;
    }

    buf[size] = '\0';
    q = buf;

	log_string("---resolver output:");
    while ( (p = strchr( q, '\n' )) )
    {
        *p = '\0';
		if(*(p-1)=='\r'){ // get rid of \r codes if they are there
			*(p-1)='\0';
		}

		logf("-%-70s-",q);
		q = p + 1; // skip over the \n
    }
}

/**************************************************************************/
// send data to the resolver
void resolver_send_data( const char * buf)
{
	char data[MSL];
	sprintf(data,"%s\n", buf);

#ifdef WIN32
	if(resolver_running){
		extern HANDLE hResolversStdIn;
		unsigned long dwWritten;
		if (! WriteFile(hResolversStdIn, data, str_len(data), &dwWritten, NULL)) {
		   assert(false && "write failed");
		}
//		logf("wrote %d bytes to resolver.", dwWritten);
	}
	return;
#endif
	if(!resolver_running){ // we don't have a resolver
		return;
	}
	
    if ( send( resolver_stdinout, data, str_len( data), 0) < 0 )
    {
		logf("Error sending to resolver (fd=%d), closing it.", resolver_stdinout);
		// problems writing to resolver socket pipe
        closesocket( resolver_stdinout);
		closesocket( resolver_stderr);
		resolver_stdinout=-1;
		resolver_version=0;
		resolver_running=false;
		logf("resolver_send_data: resolver closed! (fd=%d)", resolver_stdinout);
    }
}

/**************************************************************************/
void do_rebootresolver(char_data *ch, char *)
{
	if (IS_NPC(ch))
	{
		do_huh(ch,"");
        return;
	}

	ch->println("Rebooting resolver...");
	log_string("Rebooting resolver");
    if ( send( resolver_stdinout, "close\n", str_len( "close")+1,0) < 0 ){
		// problems writing to resolver socket pipe
        closesocket( resolver_stdinout);
		closesocket( resolver_stderr);
        resolver_running=false;
		log_string("resolver_query: resolver closed!");
    }
    resolver_running=false;
	resolver_init(EXE_FILE);
    return;
}
/**************************************************************************/
void do_resolve(char_data *ch, char *argument)
{
	if (IS_NPC(ch)){
		ch->println("Players only");
        return;
	}

	argument=trim_string(argument);
	if(IS_NULLSTR(argument)){
		ch->println("syntax: resolve <domain name to lookup>");
		ch->println("syntax: resolve <ip address to lookup>");
		return;
	}

	if(!resolver_running){

		if(IS_ADMIN(ch)){
			ch->wrapln("`RWARNING:`x The separate dns resolving process, isn't currently running.  "
				"Because of this, the only option the mud has to resolve domain names "
				"is to look them up itself.  This has the potential to lag the mud, so"
				"lookups at this stage are restricted to admin only.  Please look at "
				"getting your dns resolver working!");
		}else{
			ch->wrapln("The separate dns resolving process, isn't currently running.  "
				"Because of this, the only option the mud has to resolve domain names "
				"is to look them up itself.  This has the potential to lag the mud, so"
				"this command is only available to the admin at this point in time.");
			return;
		}
	}

	if(resolver_running && resolver_version<1500){
		ch->wrapln("The version of the dns resolver is too old for this "
			"version on dawn, obtain an update.");
		return;
	}

	ch->printlnf("Sending '%s' to the dns resolver", argument);
	ch->println("The results will be sent to your display when they are received.");

	char *command=FORMATF("resolve %s %s", TRUE_CH(ch)->name, argument);

	if(resolver_running){
		resolver_send_data(command);
	}else{
		resolverlocal_queue_command(command);
	}

    return;
}

/**************************************************************************/
// check if the resolver connection has input
void resolver_poll_and_process()
{
#ifdef WIN32
	resolver_poll_and_process_WIN32();
	return;
#endif

	static int errcount=0;
	static struct timeval null_time;
	static fd_set resolver_in;
	static fd_set resolver_out;
	static fd_set resolver_exception;

	if(!resolver_running){ // we don't have a resolver
		return;
	}
	
	FD_ZERO( &resolver_in);
	FD_ZERO( &resolver_out);
	FD_ZERO( &resolver_exception);

	FD_SET( (dawn_socket)resolver_stdinout, &resolver_in);
	FD_SET( (dawn_socket)resolver_stdinout, &resolver_out);
	FD_SET( (dawn_socket)resolver_stdinout, &resolver_exception);

	FD_SET( (dawn_socket)resolver_stderr, &resolver_in);
	FD_SET( (dawn_socket)resolver_stderr, &resolver_out);
	FD_SET( (dawn_socket)resolver_stderr, &resolver_exception);

	int maxdesc=UMAX(resolver_stdinout, resolver_stderr)+1;

	if( select( maxdesc, &resolver_in, &resolver_out, &resolver_exception, &null_time ) < 0 ){
		socket_error( FORMATF("resolver_poll_and_process: select()."));
		do_abort();
		exit_error( 1 , "resolver_poll_and_process", "select error");
	}


	if( FD_ISSET( resolver_stdinout, &resolver_exception) ){
		errcount++;
		logf("Exception raised when reading from resolver.");
		if(errcount>50){
			logf("Too many errors encounted, closing resolver socket.");
			resolver_running=false;
		}
		return;
	}

	if( FD_ISSET( resolver_stderr, &resolver_exception) ){
		errcount++;
		logf("Exception raised when reading from resolver stderr.");
		if(errcount>50){
			logf("Too many errors encounted, closing resolver socket.");
			resolver_running=false;
		}
		return;
	}


	if( FD_ISSET( resolver_stdinout, &resolver_in) ){
		resolver_get_response();
	}

	if( FD_ISSET( resolver_stderr, &resolver_in) ){
		resolver_get_stderr_response();
	}

}
/**************************************************************************/
#ifdef WIN32
/**************************************************************************/
// handles to various file descriptors of the resolver
HANDLE hResolversStdIn;
HANDLE hResolversStdOut;
HANDLE hResolversStdErr;
/**************************************************************************/
void resolver_init_WIN32(char * mainpath)
{
	char *resolver_filename="resolver.exe";
	if(!file_exists(resolver_filename)){
		char *cwd=get_current_working_directory();
		bugf("%s"DIR_SYM"%s not found!", cwd, resolver_filename);
		if(GAMESETTING(GAMESET_PERFORM_LOCAL_DNS_LOOKUPS)){
			log_notef("%s is used to convert IP addresses into domain names (seen using "
					 "the sockets command).  "
					 "Currently the gamesetting flag 'perform_local_dns_lookups' is "
					 "turned on so the translation of IP address to domain names will be "
					 "performed by the mud itself.  The only down side of doing this is "
					 "when the mud looks up the domain name of an ip address, it waits "
					 "for the answer.  If the IP doesn't have a domain name, the mud "
					 "will stall for all players until the lookup it times out after "
					 "about 30 seconds.", resolver_filename);
		}else{
			log_notef("%s is used to convert IP addresses into domain names (seen using "
					 "the sockets command).  "
					 "Currently the gamesetting flag 'perform_local_dns_lookups' is "
					 "turned off, so there is no IP to domain name conversion.  "
					 "You can turn this on within gameedit so the translation of IP address "
					 "to domain names will be performed by the mud itself.  The only down "
					 "side of turning on this option is that when the mud looks up the "
					 "domain name of an ip address, it waits for the answer.  If the IP "
					 "doesn't have a domain name, the mud will stall for all players until "
					 "the lookup it times out after about 30 seconds.", resolver_filename);
		}
		return;
	}


	// The resolver system is based on using pipes for IPC (Inter Process Communication).

	// The resolver.exe application, has been written to read commands from its stdin,
	// and send responses to commands on stdout while showing diagnostic information
	// on stderr.  In order to capture the resolver output and send it commands to 
	// its stdin, the mud launches it as a child process which inherits the stdin, 
	// stdout and stderr descriptors.

	// we will use CreatePipe to create pipes we will use for IPC,
	// since they need to be inheritable, we need to set the security 
	// attributes accordingly.	

	SECURITY_ATTRIBUTES saAttr; 
	memset(&saAttr, 0, sizeof(SECURITY_ATTRIBUTES));
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = true; 
	saAttr.lpSecurityDescriptor = NULL; 

	// resolver handles - what are sent to it using CreateProcess
	HANDLE hChildResolverStdIn;
	HANDLE hChildResolverStdOut;
	HANDLE hChildResolverStdErr;

	// temp
	HANDLE hTempHandle;  // used to allow us to get a non inheritable handle

	// StdIn pipe
	if (! CreatePipe(&hChildResolverStdIn, &hTempHandle, &saAttr, 0)) {
		assert(!"Stdin pipe creation failed\n"); 
	}
	// get a non inheritable version of hTempHandle into hResolversStdIn
    if(!DuplicateHandle(GetCurrentProcess(), hTempHandle, GetCurrentProcess(), 
		&hResolversStdIn, 0,false,DUPLICATE_SAME_ACCESS))
	{
		assert(!"DuplicateHandle failed\n"); 
	}
    CloseHandle(hTempHandle);
	
	// StdOut pipe
	if (! CreatePipe(&hTempHandle, &hChildResolverStdOut, &saAttr, 0)) {
		assert(!"Stdout pipe creation failed\n"); 
	}
	// get a non inheritable version of hTempHandle into hResolversStdIn
    if(!DuplicateHandle(GetCurrentProcess(), hTempHandle, GetCurrentProcess(), 
		&hResolversStdOut, 0,false,DUPLICATE_SAME_ACCESS))
	{
		assert(!"DuplicateHandle failed\n"); 
	}
    CloseHandle(hTempHandle);
	
	// StdErr pipe
	if (! CreatePipe(&hTempHandle, &hChildResolverStdErr, &saAttr, 0)) {
		assert(!"Stdout pipe creation failed\n"); 
	}
	// get a non inheritable version of hTempHandle into hResolversStdIn
    if(!DuplicateHandle(GetCurrentProcess(), hTempHandle, GetCurrentProcess(), 
		&hResolversStdErr, 0,false,DUPLICATE_SAME_ACCESS))
	{
		assert(!"DuplicateHandle failed\n"); 
	}
    CloseHandle(hTempHandle);


	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFOA siStartInfo; 	
	memset( &piProcInfo, 0, sizeof(PROCESS_INFORMATION) );
	memset( &siStartInfo, 0, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.dwFlags = STARTF_USESTDHANDLES;
	siStartInfo.hStdOutput = hChildResolverStdIn;//hChildResolverStdOut;
    siStartInfo.hStdInput  = hChildResolverStdOut; //hChildResolverStdIn;
	siStartInfo.hStdOutput = hChildResolverStdOut;
    siStartInfo.hStdInput  = hChildResolverStdIn;
    siStartInfo.hStdError  = hChildResolverStdErr;

	int r=CreateProcessA(NULL, 
		resolver_filename,	// command line 
		NULL,			// process security attributes 
		NULL,			// primary thread security attributes 
		true,			// handles are inherited 
		0,				// creation flags 
		NULL,			// use parent's environment 
		NULL,			// use parent's current directory 
		&siStartInfo,	// STARTUPINFO pointer 
		&piProcInfo);	// receives PROCESS_INFORMATION 

	if(r==0){
		logf("resolver_init_WIN32: CreateProcess failed to launch '%s', there will be no DNS resolution of ip addresses!", resolver_filename);
		resolver_running=false;
		return;
	}
	logf("resolver_init_WIN32: %s launched successfully - result=%d.", resolver_filename, r);
	resolver_running=true;
}

/**************************************************************************/
int resolver_poll_and_process_WIN32()
{
	if(!resolver_running){
		return 0;
	}
	
	char chBuf[MSL]; 
	DWORD dwRead; 	
	DWORD dwBytesAvailable;
	DWORD dwBytesRead;
	DWORD dwBytesLeftThisMessage;
	int r;

	// check resolvers stdout		
	dwBytesAvailable=1;
	for (;dwBytesAvailable>0;) 
	{	
		r=PeekNamedPipe(hResolversStdOut,chBuf, MIL, &dwBytesRead, &dwBytesAvailable, &dwBytesLeftThisMessage);
		if(!r){
			bugf("PeekNamedPipe() reported error %d while reading the resolvers stdout pipe.", GetLastError());
		}else{
			if(dwBytesAvailable){
				memset(&chBuf, 0, sizeof(chBuf)-1);
				chBuf[sizeof(chBuf)-1]='\0';
				if( !ReadFile( hResolversStdOut, chBuf, MSL-10, &dwRead, NULL) || dwRead == 0) {
					break; 
				}			
				//logf("resolver stdout{{{%s}}} len=%d", chBuf, str_len(chBuf));
				resolver_process_response(chBuf);
			}
		}
	}


	// resolvers stderr
	dwBytesAvailable=1;
	for (;dwBytesAvailable>0;) 
	{	 
		r=PeekNamedPipe(hResolversStdErr, chBuf, MIL, &dwBytesRead, &dwBytesAvailable, &dwBytesLeftThisMessage);
		if(!r){
			bugf("PeekNamedPipe() reported error %d while reading the resolvers stdout pipe.", GetLastError());
		}else{
			if(dwBytesAvailable){
				memset(&chBuf, 0, sizeof(chBuf)-1);
				chBuf[sizeof(chBuf)-1]='\0';
				if( !ReadFile( hResolversStdErr, chBuf, MIL, &dwRead, NULL) || dwRead == 0) {
					break; 
				}
				
				//logf("resolver stderr{{{%s}}} len=%d", chBuf, str_len(chBuf));
			}
		}
	} 
	return 0;
}

#endif // WIN32
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// general logging and debugging information -> stderr
void resolver_logf(int verbose, const char * fmt, ...)
{
	if(verbose>RESOLVERLOCAL_VERBOSE_LEVEL){
		return;
	}
    char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 1014, fmt, args);
	va_end(args);

	log_string(buf);
}

/**************************************************************************/
#ifdef IPV6_SUPPORT_ENABLED	
	typedef sockaddr_storage sockaddr_type;
#else
    typedef sockaddr_in sockaddr_type;
#endif
	
#ifdef WIN32
	#define get_socket_error_text(errorcode) get_winsock_error_text(errorcode)
	const char *get_winsock_error_text(int errorcode);
#else
	#define get_socket_error_text(errorcode) gai_strerror(errorcode)
#endif

/**************************************************************************/
// return true if it resl
bool resolve_text_ip_into_address(sockaddr_type *socket_address, size_t *address_length, char*pszAddress, int *error_value, char *error_message)
{
	if(IS_NULLSTR(pszAddress)){
		resolver_logf(2, "resolve_text_ip_into_address(): Empty query '%s' is not a valid ipv4 or ipv6 address.", pszAddress);
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
		if(res){
			freeaddrinfo(res);
		}


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
const char *resolverlocal_resolveip(char *argument)
{
	static char result[HSL];
	char result_hostname[2048];
	int errval;
	char errmsg[MSL];
	sockaddr_type socket_address;
	size_t address_length;
	result_hostname[0]='\0';

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

	// do the actual resolution
#ifdef IPV6_SUPPORT_ENABLED
	const struct sockaddr* sa=(const struct sockaddr*)&socket_address;
	char sz_host[NI_MAXHOST], sz_serv[NI_MAXSERV];
	int hostlen = NI_MAXHOST, servlen = NI_MAXSERV, r;
	socklen_t salen=(socklen_t)address_length;

	r= getnameinfo( sa, salen, sz_host, hostlen, sz_serv, servlen, NI_NUMERICSERV);

	if(r==0){ // success, give back the result 
		strcpy(result_hostname, sz_host);
	}
#else
	struct hostent *hostent;
	hostent= gethostbyaddr( 
		(const char *) &socket_address.sin_addr.s_addr, 
		sizeof(socket_address.sin_addr.s_addr), 
		socket_address.sin_family);
		
	if (hostent){ // success, strdup the result - replacing the old result if necessary
		strcpy(result_hostname, hostent->h_name);
	}
#endif
	sprintf(result, "ip=%s lookup=%s reverse=%s", argument, result_hostname, argument);
	logf("resolverlocal_resolveip(): returning '%s'", result);
	return result;
}

/**************************************************************************/
// look up an ip address or hostname 
// this command can potentially lag the mud
const char *resolverlocal_resolve(char *argument)
{
	bool system_result=false;
	bool log_resolve=true;
	static char result[HSL];
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

	if(str_len(name_start)>(int)sizeof(requesting_name)-1){
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
		logf( "resolver_local_resolve(): resolving '%s' for '%s'", n, requesting_name);
	}

	// at this stage, n points to the hostname/ip we want to resolve 

#ifdef IPV6_SUPPORT_ENABLED
	{
		// ipv4/ipv6 lookup code
		struct addrinfo hints, *reshead=NULL, *res=NULL;
		int r;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags=AI_CANONNAME; 

		abort_threshold=RUNNING_DNS_ABORT_THRESHOLD;
		update_alarm();
		r=getaddrinfo(n, "80", &hints, &res);
		update_alarm();
		abort_threshold = RUNNING_ABORT_THRESHOLD;

		if(r){
			// failed to resolve address, report the error and give up
			sprintf(result,"%s resolverlocal_resolve(): getaddrinfo() error %d (%s) occurred while attempting to resolve '%s'.", 
				 requesting_name, r, redirect_socket_error_text(r), n);
			if(res){
				freeaddrinfo(res);
			}
			return (log_resolve||system_result)?result:"";
		}
		if(!res){
			// if it did resolve, but didn't give us a usable result report that
			sprintf(result,"%s resolverlocal_resolve(): getaddrinfo() returned an empty res value when resolving '%s' - effectively no result.", 
				 requesting_name, n);
			return (log_resolve||system_result)?result:"";
		}

		// successful resolution, format the results
		if(IS_NULLSTR(res->ai_canonname)){
			sprintf(result, "%s Resolve request for:`1    %s", requesting_name, n);
		}else{
			// canonical name of the specified node (offical name)
			sprintf(result, "%s Resolve request for:`1    %s`1Official Name:`1    %s", 
				requesting_name,  n, res->ai_canonname);
		}

		reshead=res; // save the head of the res list for freeaddrinfo
		for(; res; res= res->ai_next){
			char temp[MSL];
			char sz_host[NI_MAXHOST+1];
			int hostlen = NI_MAXHOST;

			r= getnameinfo( res->ai_addr, (socklen_t)res->ai_addrlen, sz_host, hostlen, NULL, 0,NI_NUMERICHOST);
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
			if(str_len(result)>HSL-MSL){
				break;
			}
		}
	}
#else
	{
		// ipv4 lookup only code
		struct hostent *h=NULL;
		abort_threshold=RUNNING_DNS_ABORT_THRESHOLD;
		update_alarm();
		h=gethostbyname(n);
		update_alarm();
		abort_threshold = RUNNING_ABORT_THRESHOLD;
		if(h && (log_resolve || system_result)){
			// we have a match with one or more addresses
			char temp[8196];
			if(str_len(h->h_name)<8000){

				// offical name
				sprintf(result,"%s Resolve request for:`1    %s`1`1Official Name:`1    %s", 
					requesting_name, n, h->h_name);

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
							if(str_len(result)+ str_len(temp)<8000){
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
							if(str_len(result)+ str_len(temp)< 8000){
								strcat(result, temp);
							}
							an++;
						}
					}
				}

				return (log_resolve||system_result)?result:"";
			}
			snprintf( result, sizeof(result), "%s '%s' resolved, but was too long (%d characters).", 
				requesting_name, n, str_len(h->h_name));
		}else{
			snprintf( result, sizeof(result), "%s Failed to resolve: '%s'", 
				requesting_name, n);
		}
	}
#endif // not IPV6_SUPPORT_ENABLED
	return (log_resolve||system_result)?result:"";;
}
/**************************************************************************/
typedef const char *resolver_function(char *argument);
struct resolverlocal_command_type{
	const char *name;
	const char *syntax;
	resolver_function *func;
};
/**************************************************************************/
resolverlocal_command_type resolverlocal_command_table[]={
	{"resolve",		"<requesting_user> <ip|dns name>",	resolverlocal_resolve},
	{"resolveip",	"<ip>",				resolverlocal_resolveip},	
	{"", NULL}
};
/**************************************************************************/
void resolverlocal_interpret(const char *line)
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

	for(int i=0; !IS_NULLSTR(resolverlocal_command_table[i].name); i++){
		if(!strcmp(resolverlocal_command_table[i].name, command)){
			const char *response=(*(resolverlocal_command_table[i].func)) (argstart);
			if(!IS_NULLSTR(response)){
				resolver_process_response(FORMATF("%s_response %s", command, response));
			}
			return;
		}
	}
	resolver_process_response(
		FORMATF("%s_response unrecognised resolverlocal command", command));
	return;
}
/**************************************************************************/
struct resolverlocal_queue_type
{
	char *command;
	resolverlocal_queue_type *next;
};
resolverlocal_queue_type *resolverlocal_queue_list=NULL;
/**************************************************************************/
// all use of the local dns resolver is queued through here
// then called at the bottom of the game_loop() core
void resolverlocal_queue_command(const char *command)
{
	resolverlocal_queue_type *node, *tail;

	node= new resolverlocal_queue_type;

	node->command=str_dup(command);
	node->next=NULL;

	// if there is no list, create one
	if(!resolverlocal_queue_list){
		resolverlocal_queue_list=node;
		return;
	}

	// there is a list, add the new node to the end
	for(tail=resolverlocal_queue_list;tail->next; tail=tail->next){
		// loop until tail is the last in the list
	};
	tail->next=node;

}
/**************************************************************************/
// execute a queued dns command each time it is called
// if there are no commands queued just return
// called once per iteration of the game_loop() core
void resolverlocal_execute_queued_commands()
{
	resolverlocal_queue_type *nextnode;
	if(!resolverlocal_queue_list){
		return;
	}

	resolverlocal_interpret(resolverlocal_queue_list->command);

	// remove the node from the front of the list
	nextnode=resolverlocal_queue_list->next;
	free_string(resolverlocal_queue_list->command);
	delete resolverlocal_queue_list;
	resolverlocal_queue_list=nextnode;
}
/**************************************************************************/
/**************************************************************************/

