/**************************************************************************/
// comm.cpp - main(), IO loops, lots of OS related code.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with all the licenses *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************
 * >> Original Diku Mud copyright (c)1990, 1991 by Sebastian Hammer,       *
 *    Michael Seifert, Hans Henrik Staerfeldt, Tom Madsen, & Katja Nyboe.  *
 * >> Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael       *
 *    Chastain, Michael Quan, and Mitchell Tse.                            *
 * >> ROM 2.4 is copyright 1993-1995 Russ Taylor and has been brought to   *
 *    you by the ROM consortium: Russ Taylor(rtaylor@pacinfo.com),         *
 *    Gabrielle Taylor(gtaylor@pacinfo.com) & Brian Moore(rom@rom.efn.org) *
 * >> Oblivion 1.2 is copyright 1996 Wes Wagner                            *
 **************************************************************************/

#include "network.h"
#include "comm.h"
#include "colour.h"
#include "nanny.h"
#include "hreboot.h"
#include "dynamics.h" // create_class
#include "help.h"
#include "channels.h"
#ifdef WIN32
#include "process.h"
#include "conio.h"
#endif

int main_argc;
char **main_argv;

const   unsigned char    echo_off_str  [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   unsigned char    echo_on_str   [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   unsigned char    go_ahead_str  [] = { IAC, GA, '\0' };

void ispell_init();
void ispell_done();
void bust_a_group_prompt( char_data *ch);
void bust_a_prompt( connection_data *d );

void flush_cached_write_to_buffer(connection_data *c);
bool listen_on_specified_on_commandline=false;

// local prototypes
void process_input(connection_data *c);
void examine_last_command();
#ifndef FNDELAY
#define FNDELAY O_NDELAY
#endif

extern char current_logfile_name[MSL];
/**************************************************************************/
// visual debug variables
char *visual_debug_next_connection_autoon_ip=NULL;
int visual_debug_next_connection_column_width;
bool visual_debug_next_connection_hexoutput;

/**************************************************************************/
// netio.cpp related protocols and variables
extern bool listen_on_source_text_set; 
void netio_parse_listen_on(const char * listen_on);
void netio_allocate_bind_ports(int main_port);
void netio_parsed_listen_on_output();
void netio_binded_listen_on_output();
void netio_bind_connections();
void netio_init_fd_set_groups();
void netio_check_for_and_accept_new_connections();
void netio_poll_connections();
void netio_process_exceptions_from_polled_connections();
void netio_process_input_from_polled_connections();
void netio_process_output_from_polled_connections();

void resolver_poll_and_process(); // resolver.cpp
void hotreboot_poll_and_process(); // hreboot.cpp
/**************************************************************************/
void update_alarm()
{
#ifdef unix
	alarm_update();
#endif
}

/**************************************************************************/
// used to update the current time while the mud is booting
void update_currenttime(void)
{
    struct timeval last_time;

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
}
/************************************************************************/
// returns true if the file exists
bool file_exists(const char *filename)
{
	bool exists=false;
	FILE *fp;
	if(fpReserveFileExists){
		fclose( fpReserveFileExists);
	}

    if( ( fp = fopen( filename, "r" ) ) != NULL )
    {
		exists=true;
		fclose( fp );
	}
	fpReserveFileExists= fopen( NULL_FILE, "r" );	
	return exists;
}
/************************************************************************/
// returns true if the file exists
bool file_existsf(const char * fmt, ...)
{
    char filename[1024];
	va_list args;
	va_start (args, fmt);
	vsnprintf(filename, 1024, fmt, args);
	va_end (args);

	return file_exists(filename);
}

/**************************************************************************/
char *get_current_working_directory()
{	
	static char cwdbuf[MSL];
	char *result=getcwd(NULL, 0);
	if( result == NULL ){
		bugf("get_current_working_directory(): can't get current directory - error %d (%s)", 
			errno, strerror( errno));
		return "";
	}
	strncpy(cwdbuf,result, sizeof(cwdbuf)-1);
	cwdbuf[sizeof(cwdbuf)-1]='\0';
	free(result);
	return cwdbuf;
}

/**************************************************************************/
bool check_directories(int argc, char **argv)
{
	char *current_dir;
	bool problem= false;
	int i;

	// Get the current working directory
	current_dir=get_current_working_directory();

	for (i=0; str_len(directories_table[i].directory)+ 
		str_len(directories_table[i].text) >0; i++)
	{
		if( chdir( directories_table[i].directory )   )
		{
			if(str_len(directories_table[i].directory)>0)
			{
				logf( "no directory %-16s - used for '%s'", 
					directories_table[i].directory, directories_table[i].text);
				problem= true;
			}
		}
		if( chdir( current_dir )   )
		{
			logf( "UNABLE TO CHANGE BACK TO THE DEFAULT DIRECTORY!!!\n%s\n", 
				current_dir);
			problem= true;
		}
	}

	if(problem){
		logf("\nTo create all these directories start the mud as:\n"
			"'%s --createdirs'\nfrom within '%s'\n\n", argv[0], current_dir);
	}
	
	return problem;
}
/**************************************************************************/
// return true if there was a problem
bool create_directories(void)
{
	char *current_dir;
	bool problem= false;
	int i;

	log_string("=== starting in create directories mode:");
	// Get the current working directory
	current_dir=get_current_working_directory();

	for (i=0; str_len(directories_table[i].directory)+ 
		str_len(directories_table[i].text) >0; i++)
	{
		if(chdir( directories_table[i].directory )){
			if(str_len(directories_table[i].directory)>0)
			{
				char dbuf[MIL];
				sprintf(dbuf, "mkdir %s", directories_table[i].directory);

				logf( "creating dir %-15s - used for '%s'", 
					directories_table[i].directory, directories_table[i].text);

#ifdef WIN32
				if( mkdir( directories_table[i].directory )   )
#else
					if(system (dbuf))
#endif
				{
					logf( "could not create dir %-8s  - used for '%s'", 
						directories_table[i].directory, directories_table[i].text);				
					problem= true;
				}
			}
		}
		if( chdir( current_dir ) )
		{
			bugf( "UNABLE TO CHANGE BACK TO THE DEFAULT DIRECTORY!!!\n%s\n", 
				current_dir);
			problem= true;
		}
	}

	if(problem)
	{
		logf("\nThe code couldn't create the above directory/directories for you...\n"
			"see if you can fix the problem manually.\n");		
	}
	
	return problem;
}
/**************************************************************************/
void init_reserved_files()
{
    // Reserve three channels for our use.
    if( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
		bugf("init_reserved_files():1 fopen '%s' for read failed - error %d (%s)",
			NULL_FILE, errno, strerror( errno));
		exit_error( 1 , "init_reserved_files", "fopen for read failed");
    }
	if(!fpAppend2FilReserve){
		if( ( fpAppend2FilReserve = fopen( ANULL_FILE, "r" ) ) == NULL )
		{
			bugf("init_reserved_files():2 fopen '%s' for read failed - error %d (%s)",
				ANULL_FILE, errno, strerror( errno));
			exit_error( 1 , "init_reserved_files", "fopen for read 2 failed");
		}
	}

	if(!fpReserveFileExists){
		if( ( fpReserveFileExists = fopen( NULL_FILE, "r" ) ) == NULL )
		{
			fprintf(stderr, "init_reserved_files():3 fopen '%s' for read failed - error %d (%s)",
				NULL_FILE, errno, strerror( errno));
			bugf("init_reserved_files():3 fopen '%s' for read failed - error %d (%s)",
				NULL_FILE, errno, strerror( errno));
			exit_error( 1 , "init_reserved_files", "fopen for read 3 failed");
		}
	}
}

#ifdef HAVE_SYS_PARAM_H 
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

/**************************************************************************/
void display_host_info()
{
    logf("\n%s", fix_string(get_compile_time(true)));
	logf("We are running on %s.", MACHINE_NAME);
    logf( "The current working directory is %s.", 
		get_current_working_directory() );
	if(!resolver_running){
		logf( "The hostname/ident resolver is not currently running.");
	}else{
		if(resolver_version==0){
			resolver_poll_and_process();
		}
		logf( "The hostname/ident resolver is currently running.");
		logf("Resolver version = %d.%03d", resolver_version/1000, resolver_version%1000);
		if(resolver_version<1500){
			log_notef("RESOLVER VERSION 1.500 OR HIGHER IS REQUIRED FOR DNS "
				"RESOLUTION OR NO RESOLVER AT ALL!  Please delete the "
				"existing resolver binary and recompile the version which "
				"came with the source (in src/extras/resolver.*).  If you "
				"are unable to do this, the mud will actually bootup without "
				"a resolver at all (no resolver is better than an old one).");
			exit_error( 1 , "display_host_info", "resolver version too old");
		}
	}

	// obtain the platform information
	PLATFORM_INFO[0]='\0';
#ifdef WIN32
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	
	strncpy(PLATFORM_INFO, 
		FORMATF("PlatformID: %s v%d.%d.%d [%s]", 
			(
				osvi.dwPlatformId==VER_PLATFORM_WIN32s ? "Win32s":
				osvi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS ? "Windows9x":
				osvi.dwPlatformId==VER_PLATFORM_WIN32_NT ? "WindowsNT":
					FORMATF("Unknown(%d)",osvi.dwPlatformId)
			),
			osvi.dwMajorVersion,
			osvi.dwMinorVersion,
			osvi.dwBuildNumber,			
			osvi.szCSDVersion), 
		512);	
#elif defined(HAVE_SYS_UTSNAME_H)
		{
			struct utsname name;
			if(uname(&name)==-1){
				bugf("display_host_info(): couldn't retrieve the system details.");
				sprintf(PLATFORM_INFO, "PlatformID: Unknown-uname_error%d", errno);
			}else{
				sprintf(PLATFORM_INFO, "PlatformID: sysname='%s' nodename='%s' "
					"release='%s' version='%s' machine='%s'",
					name.sysname,
					name.nodename,
					name.release,
					name.version,
					name.machine);
			}
		}
#	if defined HAVE_SYS_SYSCTL_H && defined(CTL_KERN) && defined(KERN_VERSION) 
		{ // tag on the KERN_VERSION info if available
			int mib[2];
			size_t len;			
			mib[0] = CTL_KERN;
			mib[1] = KERN_VERSION;
			char kernver[512];
			len=sizeof(kernver);
			if(sysctl(mib, 2, &kernver, &len, NULL, 0)==0){
				kernver[sizeof(kernver) - 1] = '\0';

				char *tempstring=str_dup(kernver);
				tempstring=string_replace_all(tempstring, "\r", "");
				tempstring=string_replace_all(tempstring, "\n", "");	

				strcat(PLATFORM_INFO," kernver='");
				strcat(PLATFORM_INFO, tempstring);
				strcat(PLATFORM_INFO,"'");
				free_string(tempstring);
			}
		}
#	endif
#elif defined(__CYGWIN__)
	strcpy(PLATFORM_INFO, "PlatformID: Cygwin.");
#elif defined(__APPLE__) && defined(__MACH__)
	strcpy(PLATFORM_INFO, "PlatformID: MacOSX.");
#else
	strcpy(PLATFORM_INFO, "PlatformID: Unknown.");
#endif

	PLATFORM_INFO[2047]='\0';
	log_string(PLATFORM_INFO);
}
/**************************************************************************/
// prototypes
void init_string_space();
void init_mm( );
void do_load_gamesettings(char_data *ch, char *);
char max_count_ip_buf[MIL];
int	 max_count_ip;

/**************************************************************************/
void deallocate_all_memory();
extern char *top_string;
extern char *string_space;
char *dawnstat_url_encode_post_data(char *postdata);
void dawnstat_update();
/**************************************************************************/
void prevent_mud_running_as_root()
{
#ifdef unix
	// code to prevent the mud running as root
	if(getuid()==0){
		bugf("DO NOT RUN THE MUD AS ROOT!!!  THIS IS A SECURITY RISK!!!\r\n"
			"====CREATE A USER ACCOUNT TO RUN THE MUD UNDER.");
		log_release_held_logs();
		exit_error( 1 , "prevent_mud_running_as_root", "trying to run mud as root");
	}
#endif
}
/**************************************************************************/
void display_commandline_options(int argc, char **argv )
{	
	log_string("======= Command-Line Options:");
	log_string("=== General Commands:");
	log_string("-?, /?, /h, -h, --help    display this message then exit.");
	log_string("-v, /v or --version       just the version header then exit.");
	log_string("--testboot                start the mud up in testboot mode.");
	log_string("--createdirs              create the required directory structure.");
	log_string("-q, --quiet               don't display any text to stdout during startup.");
	log_string("-nb, --nobackground       don't fork to run as a background process.");
	log_string("-nl, --nologfile          don't write to a logfile.");
	log_string("-lc, --logconsole         always log to the console (win32 default).");
#ifdef WIN32
	log_string("-nolc, --nologconsole     turn off console after logging is established.");
#endif
	log_string("-f, --foreground          --nobackground and --logconsole.");
	logf("'%s -f'  IS RECOMMENDED TO TEST STARTUP PROBLEMS.", argv[0]);
// undocumented: log_string("--create_empty_class      start with no classes, and create 'classless' class structure.");

	log_string("");
	log_string("=== other options relating to startup:");
	logf("syntax: %s [primaryport_number [listen_on_setting]] ", argv[0]);
// undocumented: log_string("--hotreboot=#      we are hotrebooting, talk via the IPC pipe #.");
	log_string("");
	log_string("Where the primary port number is in the range 1024 to 65535");
	log_string("The listen_on_setting if unspecified will use the listen_on entry from");
	logf("within: %s", GAMESETTINGS_FILE);
	log_string("This listen_on entry defaults to 'telnet://:+0,http://:+1'"); 
}

/**************************************************************************/
extern bool log_hold_log_string_core_stdout_restore_value;
extern bool commandlineoption_no_background_process;
extern bool commandlineoption_no_logfile;
extern bool commandlineoption_log_console;
bool commandlineoption_quiet=false;
/**************************************************************************/
// return true if booting should continue
bool parse_commandline_options(int argc, char **argv )
{
	fBootTestOnly = false;
	bool mainport_found=false;
	mainport = 0;
	parsed_mainport=0;
	// display the program header
	logf("\n%s", fix_string(get_compile_time(true)));

	int i;
	for(i=1; i<argc; i++)
	{
		// -v, /V, --version
		if( !str_cmp("-v", argv[i]) 
			|| !str_cmp("/v", argv[i]) 
			|| !str_cmp("--version", argv[i]) )
		{
			// the header is always displayed by the top of this function
			return false;
		}

		// --help, -h, /h, -?, /? 
		if( !str_cmp("-?", argv[i]) 
			|| !str_cmp("/?", argv[i]) 
			|| !str_cmp("/h", argv[i]) 
			|| !str_cmp("-h", argv[i]) 
			|| !str_cmp("--help", argv[i]) )
		{
			display_commandline_options(argc, argv);			
			return false;
		}

		// --testboot
		if(!str_cmp("--testboot", argv[i]) )
		{
			fBootTestOnly=true;
			commandlineoption_no_background_process=true;
			log_string("starting in testboot mode and staying as a foreground process.");
			continue;
		}

		// -q, --quiet
		if( !str_cmp("-q", argv[i]) 
			|| !str_cmp("--quiet", argv[i]) )
		{
			commandlineoption_quiet=true;
			log_hold_log_string_core_stdout_restore_value=false;
			log_string("starting in quiet mode, no logging to stdout.");
			if(commandlineoption_log_console){
				log_string("The quiet mode doesn't make much sense if the log console option has been selected.");
				return false;
			}
			continue;
		}
		
		// -nb, --nobackground
		if( !str_cmp("-nb", argv[i]) 
			|| !str_cmp("--nobackground", argv[i]) )
		{
#ifdef unix
			logf("%s option specified - the mud will remain as a foreground process.",argv[i]);
#else
			logf("%s option specified - this option has no effect on this platform.",argv[i]);
#endif
			commandlineoption_no_background_process=true;
			continue;
		}

		// -nl, --nologfile
		if( !str_cmp("-nl", argv[i]) 
			|| !str_cmp("--nologfile", argv[i]) )
		{
			logf("%s option specified - no logfile will be generated.",argv[i]);
			commandlineoption_no_logfile=true;
			continue;
		}

		// -lc, --logconsole
		if( !str_cmp("-lc", argv[i]) 
			|| !str_cmp("--logconsole", argv[i]) )
		{
			logf("%s option specified - logging will continue to the console.",argv[i]);
			commandlineoption_log_console=true;

			if(commandlineoption_quiet){
				log_string("The log console option doesn't make much sense if the quiet option has been selected.");
				log_hold_log_string_core_stdout_restore_value=true;
				return false;
			}			
			continue;
		}

		// -nolc, --nologconsole
		if( !str_cmp("-nolc", argv[i]) 
			|| !str_cmp("--nologconsole", argv[i]) )
		{
			logf("%s option specified - logging wont continue to the console.",argv[i]);
			commandlineoption_log_console=false;
			continue;
		}

		// -f, --foreground
		if( !str_cmp("-f", argv[i]) 
			|| !str_cmp("--foreground", argv[i]) )
		{
			logf("%s option specified - setting no background and log console options.",argv[i]);
			commandlineoption_no_background_process=true;
			commandlineoption_log_console=true;

			if(commandlineoption_quiet){
				log_string("The foreground option doesn't make much sense if the quiet option has been selected.");
				log_string("Try using -q -nb if you want the mud to run in the foreground with no console logging.");
				log_hold_log_string_core_stdout_restore_value=true;
				return false;
			}			
			continue;
		}

		// --createdirs
		if(!str_cmp("--createdirs", argv[i]) ){
			if(create_directories()){
				log_string("There may have been problems creating the directories...\n"
					"create them manually then try again.");				
			}else{
				log_string("Directory creation completed successfully...\n"
					"Start the mud normally to continue.");				
			}
			return false;
		}


		if(!str_cmp("--create_empty_class", argv[i]) ){	
			// first do a safety check
			if(file_exists(CLASSES_LIST)){
				logf("Creation of sample "CLASSES_LIST" file option selected - file already exists!\n"
					"******* delete/rename file first if you really want to do this, then try again.");
				return false;
			}
			logf("Creating sample "CLASSES_LIST" file...");
			create_class("classless");
			do_write_classes(NULL,"");
			return false;
		}


		// --hotreboot=#
		if(!strncmp("--hotreboot=", argv[i], str_len("--hotreboot="))){
			char *num=argv[i];
			num+=str_len("--hotreboot=");
			if(!is_number(num)){			
				logf("%s startup error with hotreboot parameter '%s'", argv[0], argv[i]);
				return false;
			}
			int ipcvalue=atoi(num);
			hotreboot_reassign_child_pipe(ipcvalue);
			hotreboot_in_progress = true;
			// with hotreboot, we ignore all subsequent parameters
			// since the pipe tells us what port we are bound to etc
			return true; 
		}

		// if we get here, we must be reading the port number, and possibly followed by 
		// a listen_on setting
		if(mainport_found){
			// check for a listen on setting
			netio_parse_listen_on(argv[i]);
			listen_on_specified_on_commandline=true;
			continue;
		}else{
			// parse the port number
			if(!is_number(argv[i])){
				logf("Invalid parameter '%s'... expecting port number.", argv[i]);
				logf("Type '%s --help' for syntax documentation.", argv[0]);
				return false;
			}
			parsed_mainport=atoi(argv[i]);
			if(parsed_mainport<1024 || parsed_mainport>65535){
				logf("Invalid port value '%s'... expecting value in range 1024 to 65535.", argv[i]);
				logf("Type '%s --help' for syntax documentation.", argv[0]);
				return false;
			}
			mainport_found=true;
			continue;
		}
	}
	return true;
}

/**************************************************************************/
int main( int argc, char **argv )
{
	printf("%s",icapitalize("`=aBcd"));
	log_hold_till_commandline_options_parsed(); // necessary to make --quiet work

	main_argc=argc;
	main_argv=argv;

	runlevel=RUNLEVEL_BOOTING; // we are starting up
	update_currenttime();

	// don't let anyone host a mud as root
	prevent_mud_running_as_root();

	if(!parse_commandline_options(argc, argv)){
		mainport=parsed_mainport;
		log_release_held_logs(); // necessary to make --quiet work

		// if the function returns false, we need to exit
		// this can be because invalid options were provided
		// or a parameter like --version was used.
		return 0;
	}



	// check all the directories we expect to see exist
	if(check_directories(argc, argv)){
#ifdef WIN32
		log_note(
			"  If this is your first time starting with the Dawn of Time codebase it is`1"
			"  strongly recommended that you read the getting started guide at:`1"
			"      http://www.dawnoftime.org/    (assuming you haven't done so already)");
#endif

		log_release_held_logs(); // necessary to make --quiet work
		exit_clean(1, "main", "check_directories() wants an exit");
	}
	mainport=parsed_mainport;
	log_release_held_logs(); // necessary to make --quiet work

	// reserve the file descriptors we will to ensure don't get used up
	init_reserved_files();

	// read in the game settings
	do_load_gamesettings(NULL,"");

	// if the mainport wasn't specified earlier on the command line, use the default value
	if(mainport==0){
		mainport = default_mud_port;
		logf("no mainport value specified on command line, using default value of %d",
			default_mud_port);
	}
	logf("mainport set to %d", mainport);

	init_network();  // startup winsock on WIN32 etc
	
    // initialise global variables - in global.c
    init_globals(argv[0]); 

	if(hotreboot_in_progress){
		sleep_seconds(1);
		hotreboot_init_receive();
	}

	netio_init_fd_set_groups();

	if(!hotreboot_in_progress){
		if(!listen_on_source_text_set){
			netio_parse_listen_on(game_settings->listen_on);
		}
		netio_allocate_bind_ports(mainport);

		netio_parsed_listen_on_output();
	}

    // bind the ports unless we have already 
	// (in the case of a hotreboot etc)
    if(!hotreboot_in_progress && !fBootTestOnly){
		netio_bind_connections();	
	}

	// init the resolver 
	if( !hotreboot_in_progress && !resolver_running && !fBootTestOnly){
		resolver_init( argv[0] );
		resolver_poll_and_process();
	}

    init_alarm_handler();
	ispell_init();

	// use boot_db() to read in pretty much the game environment:
	//   intro_db, languages, classes, clans, races, skills, commands, areas
	//   lockers, corpses, letgain db, quest db, mixes, script config,
	//   herbs, offline mooting db, name generator profiles...
	boot_db();
	update_alarm();

	// get the response to the version query etc
	resolver_poll_and_process();

	display_host_info();

    if(fBootTestOnly)
	{
	    log_string( "Boot test completed OK!!!");
	    last_command[0] = '\0'; 
	    last_input[0] = '\0'; 
		fclose(fpReserve);
		fclose(fpAppend2FilReserve);
		fclose(fpReserveFileExists);
		exit_clean(0, "main", "successful boot test");
		return 0;
	}

	logf("Free stringspace =%d.", (int)(&string_space[MAX_STRING - MSL]-top_string ));
	logf("%s is ready to rock.", MUD_NAME);
	logf("Logging to %s", current_logfile_name);

	logf("Mud is running in the %s with a process id of %d",
		commandlineoption_no_background_process?"foreground":"background",
		getpid());
	if(commandlineoption_no_background_process){
		logf("Pressing ctrl+c will terminate the mud process (unless you have hotrebooted)");
	}


	examine_last_command();
	install_other_handlers(); // lastcommand debugging
   
	update_alarm();

	if(hotreboot_in_progress){
		hotreboot_game_environment_transfer();
	}

	update_alarm();

	netio_binded_listen_on_output();
	dawnlog_write_index(FORMATF("dawn bindings %s", netio_return_binded_sockets()));
	
    runlevel=RUNLEVEL_MAIN_IO_LOOP;
	game_loop();
//    closesocket(control);
//	closesocket(irc_control);

    close_network();

	// now clean up the memory
	deallocate_all_memory();
	
    // That's all, folks.
	log_string( "Normal termination of game." );
	exit_clean(0, "main", "normal game termination");
    return 0;
}
/**************************************************************************/
// write the file to indicate a shutdown
void write_shutdown_file(char_data *ch)
{
    char buf[MSL];
	// Setup the filename to shutdown to 
	sprintf( shutdown_filename, ADMIN_LOGS_DIR"sd%d.txt", mainport );

    // record shutdown details
    sprintf( buf, "at %s %s was shutdown by %s.\n===========================",
         (char *) ctime( &current_time ),
         shutdown_filename,
         ch?ch->name:"(NULL)");
    append_file( ch, SHUTDOWN_FILE, buf );
    append_file( ch, shutdown_filename, buf );
}
/**************************************************************************/
void exit_win32_close_window_prompt()
{
#ifndef WIN32
	return;
#else
	STARTUPINFO startup_info = { sizeof(STARTUPINFO) };
	GetStartupInfo(&startup_info);

	// try to determine if dawn was simply double-clicked
	if (main_argc <= 1 && startup_info.dwFlags &&
		!(startup_info.dwFlags & STARTF_USESTDHANDLES))
	{
		// started with a double click
		fprintf(stdout,"Application exiting, press any key to close this window.\n");
		fflush(stdout);
		_getch();

	}
#endif

}
/**************************************************************************/
extern bool log_string_core_stdout_enabled;
bool caught_exit_in_progress=false;
/**************************************************************************/
void exit_clean(int exitcode, char *function, char *message)
{
	last_command[0] = '\0'; 
	last_input[0] = '\0'; 
	caught_exit_in_progress=true;
	signal(SIGSEGV, SIG_DFL); // disable the use of the nasty signal handler
	exit_win32_close_window_prompt();
	exit(exitcode);
}

/**************************************************************************/
extern char last_bug[MSL*4+1];
/**************************************************************************/
void exit_error(int exitcode, char *function, char *message)
{
	caught_exit_in_progress=true;
	signal(SIGSEGV, SIG_DFL); // disable the use of the nasty signal handler


	if(runlevel==RUNLEVEL_BOOTING){
		if(!commandlineoption_log_console 
			&& !log_string_core_stdout_enabled)
		{
			fprintf(stderr, 
				"Mud failed to complete the startup process\n"
				"The startup process was terminated by %s()\n"
				"With a message of: %s\n",
				function, message);
			fprintf(stderr, "The last bug text in the log reads:\n%s\n", last_bug);
			fprintf(stderr, "This may or may not be related to why the mud didn't complete starting up.\n");
			
			if(commandlineoption_no_logfile){
				fprintf(stderr, "try starting the mud up with the -f switch to see more details.\n");
			}else{
				fprintf(stderr, 
					"==================================================\n"
					"  For more details, either review the logfile or  \n"
					"  start the mud with the -f commandline option.   \n"
					"==================================================\n");
			}
			// update the dawnlog
			dawnlog_write_index(FORMATF("Mud failed to complete the startup process.\n"
				"The startup process was terminated by %s()\n"
				"With a message of: %s",
				function, message));
			dawnlog_write_index(FORMATF("The last bug text in the log reads:\n%s", last_bug));
			dawnlog_write_index(FORMATF("This may or may not be related to why the mud didn't complete "
				"starting up, review the main log for more details."));
			
		}
	}
	exit_win32_close_window_prompt();
	exit(exitcode);
}

/**************************************************************************/
void write_last_command(void)
{
	static int callcount;
	int i;

	if(caught_exit_in_progress){
		return;
	}

	if(runlevel==RUNLEVEL_BOOTING){
		if(!commandlineoption_log_console 
			&& !log_string_core_stdout_enabled)
		{
			if(++callcount>1){
				fprintf(stderr, "[%d] write_last_command(): callcount=%d", getpid(), callcount);
			}
			if(callcount>5){
				fprintf(stderr, "[%d] write_last_command(): callcount>5!\n",getpid());		
				caught_exit_in_progress=true;
				exit(5);
			}
		}
		return;
	}

    // Return if no last command - set before normal exit 
    if(IS_NULLSTR(last_command) && IS_NULLSTR(last_input))
        return;

	logf("[%d] write_last_command(): callcount=%d", getpid(), callcount);

	callcount++;
	if(callcount>5){
		bugf("[%d] write_last_command(): callcount>5!\n", getpid());
		exit(5);
	}

	if(!IS_NULLSTR(last_command)){
		logf("last_command: %s", last_command);
		append_string_to_file( LASTCMD_FILE, last_command, true);
	}
	if(!IS_NULLSTR(last_input)){
		logf("last_input: %s", last_input);
		append_string_to_file( LASTCMD_FILE, last_input, true);
	}

	// output the inputtail
	{
		bool found=false;
		logf("======INPUTTAIL LOG");
		append_string_to_file( LASTCMD_FILE, "======INPUTTAIL LOG", true);
		for(i=(inputtail_index+1)%MAX_INPUTTAIL; 
					i!=inputtail_index; 
					i= (i+1)%MAX_INPUTTAIL){

			if(!IS_NULLSTR(inputtail[i])){
				append_string_to_file( LASTCMD_FILE, inputtail[i], true);
				log_string(inputtail[i]);
				found=true;
			}
		}
		if(!IS_NULLSTR(inputtail[inputtail_index])){
			append_string_to_file( LASTCMD_FILE, inputtail[i], true);
			log_string(inputtail[i]);
			found=true;
		}
		if(!found){
			append_string_to_file( LASTCMD_FILE, "No inputtail data to dump", true);
			log_string("No inputtail data to dump");
		}else{
			append_string_to_file( LASTCMD_FILE, "R = Room vnum, C = Connected state, E = olc editor mode... inputtail does not include force or ordered commands.", true);
			log_string("R = Room vnum, C = Connected state, E = olc editor mode... inputtail does not include force or ordered commands.");
		}
		logf("%s", fix_string(get_compile_time(false)));
		append_string_to_file( LASTCMD_FILE, fix_string(get_compile_time(false)), true);
	}
}

/**************************************************************************/
void netio_close_all_binded_sockets();
/**************************************************************************/
void nasty_signal_handler(int i)
{	
	logf("starting nasty_signal_handler (%d)", i);
    write_last_command();
#ifdef WIN32
    WSACleanup();
#endif
	signal(i, SIG_DFL);
	netio_close_all_binded_sockets();
	
	if(game_settings->config_create_coredump_at_end_of_nasty_signal_handler){
		do_abort();
	}
    return;
}

/**************************************************************************/
// Called before starting the game_loop 
void install_other_handlers()
{
    last_command [0] = '\0';
    last_input [0] = '\0';

	logf("installing atexit and signal handlers");
    if(atexit(write_last_command) != 0)
    {
		bugf("install_other_handlers:atexit - error %d (%s)", 
			errno, strerror( errno));
		exit_error( 1 , "install_other_handlers", "atexit installation failed");
    }

    // should probably check return code here 
    signal(SIGSEGV, nasty_signal_handler);

    // Possibly other signals could be caught? 
}

/**************************************************************************/
// Called after booting the database 
// Find out the last thing that happened before a crash 
void examine_last_command() 
{
    FILE *fp;
    char buf[MSL];
    char buf2[MSL];

    char working_buf[MSL];
	char fullfile_buf[MSL+5];

    fp = fopen(LASTCMD_FILE, "r");
	if(!fp){
        return;
	}
	logf("examine_last_command(): reading details of "LASTCMD_FILE" into notes system");


    int r;
	r=fscanf(fp, "%[^\n]\n", buf);
	if(r!=1){
		sprintf(buf, "Unexpected content in "LASTCMD_FILE"! - First Read, r=%d, errno=%d (%s)", r, errno, strerror(errno));
		log_string(buf);
	}else{
		logf("from "LASTCMD_FILE": '%s'", buf);	
	}
    r=fscanf(fp, "%[^\n]\n", buf2);
	if(r!=1){
		sprintf(buf, "Unexpected content in "LASTCMD_FILE"! - Second Read, r=%d, errno=%d (%s)", r, errno, strerror(errno));
		log_string(buf2);
	}else{
		logf("from "LASTCMD_FILE": '%s'", buf2);	
	}

	// fullfile_buf
	strcpy(fullfile_buf, buf);
	strcat(fullfile_buf, "`1");
	strcat(fullfile_buf, buf2);
	strcat(fullfile_buf, "`1");

	while(!feof(fp)){
		r=fscanf(fp, "%[^\n]\n", working_buf);
		if(r!=1){
			sprintf(working_buf, "Unexpected content in "LASTCMD_FILE"! - Loop Reading, r=%d, errno=%d (%s)", r, errno, strerror(errno));
			log_string(working_buf);
		}else{
			logf("from "LASTCMD_FILE": '%s'", working_buf);	
		}
		if(str_len(fullfile_buf) + str_len(working_buf)>MSL){
			strcat(fullfile_buf, "...`x");
			break; // buffer full
		}
		strcat(fullfile_buf, working_buf);
		strcat(fullfile_buf, "`1`x");
	}
    fclose(fp);
    unlink(LASTCMD_FILE);

	strcat(buf,"\r\n");
	strcat(buf,buf2);
	// do autonotes
	autonote(NOTE_SNOTE, "examine_last_command()", 
		"Last recorded command before crash", "imm", buf, true);

	autonote(NOTE_SNOTE, "examine_last_command()", 
		"recorded details before crash", "admin", fullfile_buf, true);
	 
	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){ // do an autonote
		autonote(NOTE_NOTE, "examine_last_command()", 
			"Last recorded command before crash", "all", buf, true);
	}
}

/**************************************************************************/
void synchronise_to_pulse()
{
	static bool initial_values_required_setup=true;
	static struct timeval last_time;

	if(initial_values_required_setup){
	    gettimeofday( &last_time, NULL );
	    current_time = (time_t) last_time.tv_sec;
		initial_values_required_setup=false;
	}

	// Synchronize to a clock.
	// Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	{
		struct timeval now_time;
		long secDelta;
		long usecDelta;
		
		gettimeofday( &now_time, NULL );
		usecDelta   = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
		secDelta    = ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
		while ( usecDelta < 0 )
		{
			usecDelta += 1000000;
			secDelta  -= 1;
		}
		
		while ( usecDelta >= 1000000 )
		{
			usecDelta -= 1000000;
			secDelta  += 1;
		}
		
		if( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
		{
			struct timeval stall_time;
			
			stall_time.tv_usec = usecDelta;
			stall_time.tv_sec  = secDelta;
#ifdef WIN32
			Sleep( (stall_time.tv_sec * 1000)  | (stall_time.tv_usec/1000) );
#else
			if( select( 0, NULL, NULL, NULL, &stall_time ) < 0 ){
				socket_error( "Game_loop: select: stall" );
				exit_error( 1 , "synchronise_to_pulse", "select stall");
			}
#endif
		}
	}
	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
}

/**************************************************************************/
void game_loop()
{
	hotreboot_in_progress = false;
	struct timeval last_time;

#ifdef unix
    signal( SIGPIPE, SIG_IGN );
#endif
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    // Main game loop 
    while ( runlevel==RUNLEVEL_MAIN_IO_LOOP)
    {
		// sync to the clock so we get PULSE_PER_SECOND happening
		// if we didn't have this, the mud would do as many pulses
		// as the hosting server could perform.
		synchronise_to_pulse();

		// check if the dns resolver has anything for us
		resolver_poll_and_process();

		// if we are doing a hotreboot, poll the hotreboot_ipc_pipe
		if(hotreboot_in_progress){
			hotreboot_poll_and_process();
		}

		// check for new connections on any of the ports we are listening
		// if any are found to be waiting, accept them
		netio_check_for_and_accept_new_connections();

		// accept all the new connections
/*		accept_new(control);
		if(irc_control>-1){
			accept_new(irc_control);
		}
		if(ftp_control>-1){
			accept_new(ftp_control);
		}
*/
		// poll all the connections for input, output and exceptions
		// the descriptor sets are stored within netio.cpp
		netio_poll_connections();

		// process any exceptions for the polled connections
		netio_process_exceptions_from_polled_connections();

		// process input from the polled connections
		netio_process_input_from_polled_connections();

		// Autonomous game motion.
		update_handler( );
		
		netio_process_output_from_polled_connections();

		update_alarm();

		resolverlocal_execute_queued_commands();
    }
    return;
}
/**************************************************************************/
struct local_tcp_pair_list
{
	char *local_tcp_pair;
	int count;
	local_tcp_pair_list *next;
};
/**************************************************************************/
void max_count_ip_calc()
{
	local_tcp_pair_list* iplist, *node;

	// find the most common IP used by connections
	iplist=NULL; // start with an empty list of ip addresses
	// start by looping thru and adding them all up
	for(connection_data *c=connection_list; c; c=c->next){
		// see if we already have the ip address recorded
		for(node=iplist; node; node=node->next){
			if(!strcmp(node->local_tcp_pair, c->local_tcp_pair)){
				break;
			}
		}

		if(node){
			node->count++;
		}else{
			node=new local_tcp_pair_list;
			node->count=1;
			node->local_tcp_pair=str_dup(c->local_tcp_pair);
			node->next=iplist;
			iplist=node;
		}
	}
	// next find the highest
	char *most_common_local_tcp_pair=str_dup("");
	max_count_ip=0;
	for(node=iplist; node; node=iplist){
		if(node->count>max_count_ip){
			max_count_ip=node->count;
			replace_string(most_common_local_tcp_pair,node->local_tcp_pair);
		}
		iplist=node->next;
		free_string(node->local_tcp_pair);
		delete node; // deallocate the nodes - don't need them any more
	}

	strcpy(max_count_ip_buf, most_common_local_tcp_pair);
	free_string(most_common_local_tcp_pair);
}

/**************************************************************************/
void greet_new_connection(connection_data *c)
{
    // Send the greeting.
	help_data *pHelp;
	if(IS_IRCCON(c)){
		wiznet(FORMATF("`B***IRC connection starting***(socket %d = %s)`x", 
			c->connected_socket, c->remote_ip), NULL,NULL,WIZ_SITES,0,0);

		pHelp=help_get_by_keyword("irc-greeting", NULL, true);
		if(!pHelp){
			pHelp=help_get_by_keyword("greeting", NULL, true);
			if(!pHelp){
				write_to_buffer( c, FORMATF("Welcome to %s\r\n\r\n", MUD_NAME), 0);
			}else{
				write_to_buffer(c, pHelp->text, 0 );
			}
		}else{
			write_to_buffer(c, pHelp->text, 0 );
		}	
		write_to_buffer(c, "\r\n", 0 );
	}else{
		pHelp=help_get_by_keyword("greeting", NULL, true);
		if(!pHelp){
			write_to_buffer( c, FORMATF("Welcome to %s\r\n\r\n", MUD_NAME), 0);
		}else{
			write_to_buffer(c, pHelp->text, 0 );
		}
	}

	if(HAS_MXPDESC(c)){
		write_to_buffer(c, FORMATF("%s%s", LOGIN_PROMPT, mxp_tagify("<USER>")), 0);
		SET_BIT(c->flags, CONNECTFLAG_USER_TAG_SENT);
	}else{
		write_to_buffer(c, LOGIN_PROMPT, 0);
	}

	if(IS_IRCCON(c)){			
		write_to_buffer(c, "\r\n", 0 );
	}
}

/**************************************************************************/
void connection_close( connection_data *cclose )
{
    char_data *ch;
	connection_data *c;

    if( cclose->outtop > 0 ){
		process_output( cclose, false );
	}

    if( cclose->snoop_by != NULL ){
        write_to_buffer( cclose->snoop_by, "Your snoop victim has left the game.\r\n", 0 );
	}
   
	{ // cancel snoops
		for ( c = connection_list; c; c = c->next )
		{
			if( c->snoop_by == cclose ){
				c->snoop_by = NULL;
			}
		}
    }

    if( cclose->command_snoop != NULL )
    {
		write_to_buffer( cclose->command_snoop, 
            "Your command snoop victim has left the game.\r\n", 0 );
	}
   
	{ // cancel command snoops
		for ( c = connection_list; c; c = c->next )
		{
			if( c->command_snoop == cclose ){
				c->command_snoop = NULL;
			}
		}
    }

    if( ( ch = cclose->character ) != NULL )
    {
		logf("Closing link to %s. (socket=%d)", ch->name, ch->desc->connected_socket);

		// record last login site
		if(ch->pcdata){
			free_string(ch->pcdata->last_logout_site);
			if(IS_NULLSTR(ch->desc->remote_hostname)){
				ch->pcdata->last_logout_site = str_dup("???");
			}else{
				ch->pcdata->last_logout_site = str_dup(ch->desc->remote_hostname);
			}
		}

		// avoid situation where a non connected players 
		// duplicate their pets
		if(cclose->connected_state!=CON_PLAYING && ch->pet){
			ch->pet->in_room=NULL;
		}

		// gets rid of pets in NULL rooms from players that get their password wrong
		if( ch->pet && ch->pet->in_room == NULL ){
			char_to_room( ch->pet, get_room_index(ROOM_VNUM_LIMBO) );
			extract_char( ch->pet, true );
		}

		if( cclose->connected_state == CON_PLAYING )
		{
			act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
			wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);		
			ch->desc = NULL;
		}
		else
		{
			free_char( cclose->original ? cclose->original : cclose->character );
		}
    }

	free_speedwalk( cclose );
    // remove them from the linked list
	if( c_next == cclose ){
		c_next = c_next->next;   
	}
	if( cclose == connection_list ){
		connection_list = connection_list->next;
    }else{
		for ( c = connection_list; c && c->next != cclose; c = c->next )
			;
		if( c ){
			c->next = cclose->next;
		}else{
			bug("connection_close: cclose not found.");
		}
    }

#ifdef MCCP_ENABLED
	// end MCCP if it is being used
	if(cclose->out_compress){
		cclose->end_compression();
	}
#endif

	// close the actual socket connection
	cclose->close_socket();

	// recycle the connection
    connection_deallocate(cclose);
    return;
}

/**************************************************************************/
// return true if everything is okay - i.e. we read something or are 
//                                          waiting for something.
bool read_from_connection( connection_data *c )
{
    unsigned int iStart;

/*
	// commented out, - Kal Feb 06
    // Hold horses if pending command already.
    if( c->incomm[0] != '\0' ){
		return true;
	}
*/
	
    // Check for overflow. 
    iStart = str_len(c->inbuf);
    if( iStart >= sizeof(c->inbuf) - 10 )
    {
		logf( "%s input overflow! (socket=%d)", c->remote_tcp_pair, c->connected_socket);
		c->write("\r\n*** PUT A LID ON IT!!! ***\r\n", 0 );
		return false;
    }
	
    // Snarf input. 
    for ( ; ; )
    {
		int nRead;
		
		// There is no more space in the input buffer for now 
		if(sizeof(c->inbuf) - 10 - iStart == 0){
			break;
		}
		
		nRead = recv( c->connected_socket, c->inbuf + iStart,
			sizeof(c->inbuf) - 10 - iStart, 0 );
		
		if( nRead > 0 )
		{
			iStart += nRead;
			if( c->inbuf[iStart-1] == '\n' || c->inbuf[iStart-1] == '\r' ){
				break;
			}
		}
		else 
		{
			if(nRead==0){
				logf( "EOF encountered on read from %s.  (socket %d)", 
					c->remote_tcp_pair, c->connected_socket);
				return false;
			}
			else 
			{
#ifdef WIN32
				if(WSAGetLastError() == WSAEWOULDBLOCK){
					break;
				}else{
					socket_error( "Read_from_connection" );
					return false;
				}
#else
				if(errno==EWOULDBLOCK){
					break;
				}else{
					socket_error( "Read_from_connection" );
					return false;
				}
#endif
			}
		}
    }
	
    c->inbuf[iStart] = '\0'; // mark end of text
	c->inbuf[iStart+1] = '\0'; // marked one past, so the telnet suboption code can detect the end easily

	// now call read_from_buffer() to prequeue c->incomm[]
	// therefore if one or more commands are waiting in the input buffer, c->incomm[] will not be null
	read_from_buffer(c);
	
    return true;
}

/**************************************************************************/
// Transfer one line from input buffer to input line.
void read_from_buffer( connection_data *c )
{
    int i, j, k;
	bool got_n, got_r;

    // Hold horses if pending command already.
    if( c->incomm[0] != '\0' ){
		return;
	}

	// handle speedwalking
	if( c->speedwalk_buf )
	{
		char *s, *e;
		
		while ( is_digit( *c->speedwalk_head ) && *c->speedwalk_head != '\0' )
		{
			s = c->speedwalk_head;
			while( is_digit( *s ))
				s++;
			e =s;
			while(*(--s) == '0' && s != c->speedwalk_head );
			if( is_digit( *s ) && *s != '0' && *e != 'o' )
			{
				c->incomm[0] = *e;
				c->incomm[1] = '\0';
				s[0]--;
				while ( is_digit(*(++s)))
					*s = '9';
				return;
			}
			
			if( *e == 'o' ){
				c->speedwalk_head = e;
			}else{
				c->speedwalk_head = ++e;
			}
		}
		if( *c->speedwalk_head != '\0' )
		{
			if( *c->speedwalk_head != 'o' )
			{
				c->incomm[0] = *c->speedwalk_head++;
				c->incomm[1] = '\0';
				return;
			}
			else
			{
				char buf[MIL];
				
				c->speedwalk_head++;
				
				sprintf( buf, "open " );
				switch ( *c->speedwalk_head )
				{
				case 'n' : sprintf( buf+str_len(buf), "north" );		break;
				case 'e' : sprintf( buf+str_len(buf), "east" );		break;
				case 's' : sprintf( buf+str_len(buf), "south" );		break;
				case 'w' : sprintf( buf+str_len(buf), "west" );		break;
				case 'r' : sprintf( buf+str_len(buf), "northwest" );	break;
				case 't' : sprintf( buf+str_len(buf), "northeast" );	break;
				case 'g' : sprintf( buf+str_len(buf), "southeast" ); break;
				case 'f' : sprintf( buf+str_len(buf), "southwest" ); break;
				case 'u' : sprintf( buf+str_len(buf), "up" );		break;
				case 'd' : sprintf( buf+str_len(buf), "down" );		break;
				default: return;
				}
				strcpy( c->incomm, buf );
				c->speedwalk_head++;
				return;
			}
		}
		free_speedwalk( c );
	}

#ifdef DEBUG_PACKET_LOGGING_IN_READ_FROM_BUFFER
	logf("read_from_buffer point1:'%s' %d %d %d %d", c->inbuf, 
		(unsigned char)*c->inbuf, (unsigned char)*(c->inbuf+1), 
		(unsigned char)*(c->inbuf+2), (unsigned char)*(c->inbuf+3)); // debug code
#endif

	// Look for at least one new line
    for ( i = 0; c->inbuf[i] != '\n' && c->inbuf[i] != '\r'; i++ )
    {
		// check for telnet IAC options
		if((unsigned char)c->inbuf[i]==IAC){
			c->process_telnet_options(i);
		}

		if( c->inbuf[i] == '\0' ){
			return;
		}
    }

#ifdef DEBUG_PACKET_LOGGING_IN_READ_FROM_BUFFER
	logf("read_from_buffer point2: %d %d %d %d", 
		*c->inbuf, (unsigned char)*(c->inbuf+1), 
		(unsigned char)*(c->inbuf+2), (unsigned char)*(c->inbuf+3)); // debug code
#endif 

	// the only time it will get this far is if there is a complete line (ending with \r or \n)

	// process any mxp options
	// NOTE: There is no limit on the length of the input at this stage
	if(c->inbuf[0]=='\033' && !memcmp(MXP_CLIENT_TO_SERVER_PREFIX, c->inbuf, str_len(MXP_CLIENT_TO_SERVER_PREFIX))){
		c->process_client2server_mxp_message(i);
		return;
	}

    // Canonical input processing
    for ( i = 0, k = 0; c->inbuf[i] != '\n' && c->inbuf[i] != '\r'; i++ )
    {
		// if the input is too long, ignore the line
		if( k >= MIL - 4 )
		{
            c->write("Line too long.\r\n", 0 );

			// skip the rest of the line 
			for ( ; c->inbuf[i] != '\0'; i++ )
			{
				if( c->inbuf[i] == '\n' || c->inbuf[i] == '\r' ){
					break;
				}
			}
			c->inbuf[i]   = '\n';
			c->inbuf[i+1] = '\0';
			break;
		}

		// safety code to always prevent MXP tags being sent by the client
		if(c->inbuf[i]==MXP_BEGIN_TAG){
			continue;
		}


		// support backspace, and various other input processing/patching
		if( c->inbuf[i] == '\b' && k > 0 ){
			--k; // support backspace
		}else if (!GAMESETTING5(GAMESET5_DISABLE_TILDE_CONVERSION) 
			&& c->inbuf[i] == '~')
		{
			// convert ~ into `-
			// this is done to prevent connections getting the ~ 
			// character into files which can accidentally or 
			// intentionally cause corruption/security issues.
			c->incomm[k++] = '`';
			c->incomm[k++] = '-';
		}else if(GAMESETTING3(GAMESET3_DISABLE_EXTENDED_ASCII_CHARACTERS)){
			if (is_ascii(c->inbuf[i]) && is_print(c->inbuf[i]))
			{
				c->incomm[k++] = c->inbuf[i];
			}
		}else{ 
			unsigned char uc=c->inbuf[i];
			if(uc>0x1F && uc!=0x7F && uc!=0xFF) // accept anything but control characters
			{ 
				c->incomm[k++] = c->inbuf[i];
			}
		}
    }

    // Finish off the line.
    if( k == 0 ){
		c->incomm[k++] = ' ';
	}
    c->incomm[k] = '\0';

    // Deal with bozos with #repeat 1000 ...
    if( k > 1 || c->incomm[0] == '!' )
    {
		if( c->incomm[0] != '!' && strcmp( c->incomm, c->inlast ) )
		{
			c->repeat = 0;
		}
		else
		{
			if( ++c->repeat >= 25 )
			{
				sprintf( log_buf, "%d (%s) input spamming!", 
					c->connected_socket, c->remote_tcp_pair);
				log_string( log_buf );
				if(c->connected_state==CON_PLAYING)
				{
					wiznet("Spam spam spam $N spam spam spam spam spam!",
						c->character,NULL,WIZ_SPAM,0,get_trust(c->character));
					if(c->incomm[0] == '!'){
						wiznet(c->inlast,c->character,NULL,WIZ_SPAM,0,
							get_trust(c->character));
					}else{
						wiznet(c->incomm,c->character,NULL,WIZ_SPAM,0,
							get_trust(c->character));
					}
				}				
				c->repeat = 0;
			}
		}
    }


    // Do '!' substitution.
    if( c->incomm[0] == '!' 
		&& (c->connected_state!=CON_DETECT_CLIENT_SETTINGS
			|| c->connected_state!=CON_GET_NAME) )
	{
		strcpy( c->incomm, c->inlast );
    }else{
		strcpy( c->inlast, c->incomm );
	}

    // now figure out if we received, \n, \r\n, \n or \n\r to mark the line ending.
	// use this information to skip over the end of line marker
	got_n = got_r = false;
	for (;c->inbuf[i] == '\r' || c->inbuf[i] == '\n';i++)
	{
		if(    (c->inbuf[i] == '\r' && got_r++) 
			|| (c->inbuf[i] == '\n' && got_n++)){
			break;
		}
	}

	// Shift the input buffer.
    // - i.e. since the command has been copied from inbuf[] into incomm[] 
	//        move the input which follows to the front of inbuf[]
    for ( j = 0; ( c->inbuf[j] = c->inbuf[i+j] ) != '\0'; j++ ){
	};

    return;
}
/**************************************************************************/
// This should only be called by write_into_buffer_thru_process_colour()
// or the visual debugging system
static void write_into_buffer( connection_data *c, const char *txt, int length )
{
    // Expand the buffer as needed
    while ( c->outtop + length >= c->outsize )
    {
		char *outbuf;
		
		if(c->outsize >= 64000)
		{
			bug("write_into_buffer(): Buffer overflow. Closing.");
			c->write("Buffer overflow (too much data generated from what you just did most likely),\r\n"
				"Closing your connection.\r\n", 0);
			// mark it as if the buffer has nothing to be written to 
			// avoid endless loop of 
			//   connection_close()->process_output()->
			//   flush_cached_write_to_buffer() -> 
			//	 write_into_buffer_thru_process_colour() ->
			//	 write_into_buffer()
			// 
		    c->outtop=0; 
			c->outsize=0;
			connection_close(c);
			return;
		}

		if(c->outsize<1){
			return;
		}
		outbuf      = (char *)alloc_mem( 2 * c->outsize );
		strncpy( outbuf, c->outbuf, c->outtop );
		free_mem( c->outbuf, c->outsize );
		c->outbuf   = outbuf;
		c->outsize *= 2;
    }

    // Copy.
    strncpy( c->outbuf + c->outtop, txt, length );
	c->outtop += length;
}
/**************************************************************************/
static void write_into_buffer_thru_process_colour( connection_data *d, const char *txt, int length );
#define VISUAL_DEBUG_COLUMN_WIDTH			(25)
#define VISUAL_DEBUG_BUFFER_SIZE			(4000)
#define VDBS	(VISUAL_DEBUG_BUFFER_SIZE )
/**************************************************************************/
static void visual_debug_write( connection_data *d, const char *txt, int length )
{
	// Visual debugger system
	if(d->visual_debug_buffer==NULL){ // allocate a buffer if it is the first time to be used
		d->visual_debug_buffer=(char*)malloc(VDBS+1);
		d->visual_debug_buffer[0]='\0';
	}
	int space_free=VDBS - str_len(d->visual_debug_buffer);
	if(length< space_free){
		strcat(d->visual_debug_buffer, txt);
		return;
	}

	// fill up visual_debug_buffer with space_free characters
	int copied=VDBS-str_len(d->visual_debug_buffer);
	strncat(d->visual_debug_buffer, txt, copied);
	d->visual_debug_buffer[VDBS]='\0';

	{
		char hex_block[VISUAL_DEBUG_COLUMN_WIDTH*5];
		char visual_debug_format_buffer[VDBS*5];
		char hex[10];
		int character;
		const char *p=d->visual_debug_buffer;

		// set the colour to white and MXP to locked mode
		d->visual_debugging_enabled=false;
		write_into_buffer_thru_process_colour( d, 
			FORMATF("`x\r\n%s=======VISUAL DEBUG======   Sent to your Connection:\r\n", 
				MXP_LOCKED_MODE), 0);
		d->visual_debugging_enabled=true;

		// setup our visual debugging format buffer
		visual_debug_format_buffer[0]='\0';
		char *w=visual_debug_format_buffer;
		
		character=0;
		hex_block[0]='\0';
		for(;*p; p++){
			if(is_print(*p)){
				*w++=*p;
			}else{
				*w++='.';
			}
			if(d->visual_debug_hexoutput){
				sprintf(hex," %02x", ((unsigned char)*p));
			}else{
				sprintf(hex," %03d", ((unsigned char)*p));
			}
			strcat(hex_block, hex);
			character++;

			if(character>d->visual_debug_column_width-1){
				character=0;
				*w='\0';
				strcat(w, "  ");
				strcat(w, hex_block);
				strcat(w, "\r\n");
				hex_block[0]='\0';
				w+=str_len(w);
			}
		}
		*w='\0';
		strcat(w, "   ");
		strcat(w, hex_block);
		if(d->mxp_enabled){
			strcat(w,MXP_SECURE_MODE);
		}
		strcat(w, "\r\n");

		write_into_buffer( d, visual_debug_format_buffer, str_len(visual_debug_format_buffer));

		d->visual_debug_buffer[0]='\0';
	}

	// recurse to handle the next block
	visual_debug_write( d, txt+copied, str_len(txt+copied));
}
/**************************************************************************/
void visual_debug_flush( connection_data *d)
{
	if(IS_NULLSTR(d->visual_debug_buffer)){
		return;
	}
	char hex_block[VISUAL_DEBUG_COLUMN_WIDTH*5];
	char visual_debug_format_buffer[VDBS*5];
	char hex[10];
	int character;
	const char *p=d->visual_debug_buffer;

	// set the colour to white and MXP to locked mode
	d->visual_debugging_enabled=false;
	write_into_buffer_thru_process_colour( d, 
		FORMATF("`x\r\n%s=======VISUAL DEBUG======   Sent to your Connection:\r\n", 
		HAS_MXPDESC(d)?MXP_LOCKED_MODE:""), 0);
	d->visual_debugging_enabled=true;

	// setup our visual debugging format buffer
	visual_debug_format_buffer[0]='\0';
	char *w=visual_debug_format_buffer;
	
	character=0;
	hex_block[0]='\0';
	for(;*p; p++){
		if(is_print(*p)){
			*w++=*p;
		}else{
			*w++='.';
		}
		if(d->visual_debug_hexoutput){
			sprintf(hex," %02x", ((unsigned char)*p));
		}else{
			sprintf(hex," %03d", ((unsigned char)*p));
		}
		strcat(hex_block, hex);
		character++;

		if(character>d->visual_debug_column_width-1){
			character=0;
			*w='\0';
			strcat(w, "  ");
			strcat(w, hex_block);
			strcat(w, "\r\n");
			hex_block[0]='\0';
			w+=str_len(w);
		}
	}
	*w='\0';
	// insert extra spaces if necessary to make the final characters line up
	if(character>0 && character<=d->visual_debug_column_width-1){
		strcat(w, FORMATF("%*c",  (d->visual_debug_column_width-1) - character,' '));
	}
	strcat(w, "   ");
	strcat(w, hex_block);
	strcat(w, HAS_MXPDESC(d)?MXP_SECURE_MODE:"");

	strcat(w, "\r\n");

	write_into_buffer( d, visual_debug_format_buffer, str_len(visual_debug_format_buffer));

	d->visual_debug_buffer[0]='\0';
}
/**************************************************************************/
void do_visualdebug(char_data *ch, char *argument)
{
	// check we have a descriptor
	connection_data *c=ch->desc;
	if(!c){
		ch->println("Sorry, you can't use the visualdebug command as you don't have a connection");
		return;
	}

	char arg[MIL], arg2[MIL];
	argument=one_argument(argument, arg);

	if(IS_NULLSTR(arg)){
		ch->titlebar("THE VISUAL DEBUGGER");
		ch->wrapln("The visual debugger is used to see the raw information being sent "
			"to your mud/telnet client, it really only has a use for debugging the mud code "
			"and for programmers working on a mud client.  You can't do any damage "
			"by turning on or playing with the debugger, just things may look a little "
			"weird.");

		ch->println("syntax: visualdebug on");
		ch->println("syntax: visualdebug off");
		ch->println("syntax: visualdebug hexoutput on");
		ch->println("syntax: visualdebug hexoutput off");
		ch->println("syntax: visualdebug flush_before_prompt on");
		ch->println("syntax: visualdebug flush_before_prompt off");
		ch->println("syntax: visualdebug strip_prompt on");
		ch->println("syntax: visualdebug strip_prompt off");
		ch->println("syntax: visualdebug column_width #");
		ch->println("        Where # is 10->30");
		ch->println("syntax: visualdebug next_connection_autoon");
		ch->wrapln("next_connection_autoon turns on debugging for the next connection from "
			"your ip address.");

		ch->titlebar("YOUR CURRENT VISUAL DEBUG SETTINGS");
		ch->printlnf("Visual debugging is currently %s for your connection.", 
			c->visual_debugging_enabled?"on":"off");
		ch->printlnf("Visual debugging column width is set to %d.",
			c->visual_debug_column_width);
		ch->printlnf("The visual debug display numeric values in %s format.", 
			c->visual_debug_hexoutput?"hexidecimal":"decimal");		
		ch->printlnf("The visual debug is%s flushed before displaying a prompt.", 
			c->visual_debug_flush_before_prompt?"":" not");
		ch->printlnf("The prompt is%s being stripped out of the visual debug.", 
			c->visual_debug_strip_prompt?"":" not");
		ch->titlebar("");


		return;
	}

	if(!str_cmp(arg, "on")){
		if(c->visual_debugging_enabled){
			ch->println("You already have visual debugging enabled!");
			return;
		}
		c->visual_debugging_enabled=true;
		ch->println("Visual debugging turned on.");
		return;
	}


	if(!str_cmp(arg, "off")){
		if(!c->visual_debugging_enabled){
			ch->println("You already have visual debugging disabled!");
			return;
		}
		c->visual_debugging_enabled=false;
		ch->println("Visual debugging turned off.");
		return;
	}

	// all the other options require an additional argument
	argument=one_argument(argument, arg2);

	// Column width
	if(!str_prefix(arg, "column_width")){
		int value=URANGE(10,atoi(arg2),30);
		c->visual_debug_column_width=value;
		ch->printlnf("Visual debug column width set to %d", value);
		return;
	}

	// Hex output
	if(!str_prefix(arg, "hexoutput")){
		if(!str_cmp(arg2, "on")){
			if(c->visual_debug_hexoutput){
				ch->println("You already have hexoutput enabled!");
				return;
			}
			c->visual_debug_hexoutput=true;
			ch->println("hexoutput turned on.");
			return;
		}
		if(!str_cmp(arg2, "off")){
			if(!c->visual_debug_hexoutput){
				ch->println("You already have hexoutput off!");
				return;
			}
			c->visual_debug_hexoutput=false;
			ch->println("hexoutput turned off.");
			return;
		}
	}


	// Strip Prompt
	if(!str_prefix(arg, "strip_prompt")){
		if(!str_cmp(arg2, "on")){
			if(c->visual_debug_strip_prompt){
				ch->println("You already have strip_prompt enabled!");
				return;
			}
			c->visual_debug_strip_prompt=true;
			ch->println("strip_prompt turned on.");
			ch->println("Note: This has no effect while flush_before_prompt is off");
			return;
		}
		if(!str_cmp(arg2, "off")){
			if(!c->visual_debug_strip_prompt){
				ch->println("You already have strip_prompt off!");
				return;
			}
			c->visual_debug_strip_prompt=false;
			ch->println("strip_prompt turned off.");
			return;
		}
	}

	
	// Flush before prompt
	if(!str_prefix(arg, "flush_before_prompt")){
		if(!str_cmp(arg2, "on")){
			if(c->visual_debug_flush_before_prompt){
				ch->println("You already have flush_before_prompt enabled!");
				return;
			}
			c->visual_debug_flush_before_prompt=true;
			ch->println("flush_before_prompt turned on.");
			return;
		}
		if(!str_cmp(arg2, "off")){
			if(!c->visual_debug_flush_before_prompt){
				ch->println("You already have flush_before_prompt off!");
				return;
			}
			c->visual_debug_flush_before_prompt=false;
			ch->println("flush_before_prompt turned off.");
			return;
		}
	}

	// next connection autoon
	if(!str_prefix(arg, "next_connection_autoon")){
		if(visual_debug_next_connection_autoon_ip==NULL){
			visual_debug_next_connection_autoon_ip=str_dup("");
		}
		replace_string(visual_debug_next_connection_autoon_ip, c->remote_ip);
		visual_debug_next_connection_column_width=c->visual_debug_column_width;
		visual_debug_next_connection_hexoutput=c->visual_debug_hexoutput;
		ch->wrapln("Next time you connect, visual debugging will be automatically "
			"turned on (with a column with of your current column width)... "
			"This enables visual debugging of the bootup sequence.");
		return;
	}

	ch->printlnf("VisualDebug: Unrecognised option '%s %s'.", arg, arg2);
	do_visualdebug(ch, "");
}
/**************************************************************************/
// should only be called from flush_cache, process_output for snoop, and write_to_buffer
static void write_into_buffer_thru_process_colour( connection_data *c, const char *txt, int length )
{
	if(!c){
		bug("write_into_buffer_thru_process_colour(): Being called with NULL connection!");			
		return;
	}

	// make sure we are working with a valid connection
	if(!IS_VALID(c)){ //IS_VALID ensures c!=NULL
		bugf("write_into_buffer_thru_process_colour(): Being called with invalid connection %d (%s).",
			c->connected_socket, CH(c)?CH(c)->name:"no name");
		return;
	}

    // Find length incase caller didn't.
    if( length <= 0 ){
		length = str_len(txt);
	}

	if(length==0){
		return;
	}

    // Initial \r\n if needed.
    if( c->outtop == 0 && !c->fcommand )
    {
		c->outbuf[0]    = '\r';
		c->outbuf[1]    = '\n';
		c->outtop       = 2;
    }

	// colour code parsing - will trim binary buffers with embedded nul's
	if(c->parse_colour){		
		txt=process_colour(txt, c);
		length = str_len(txt);
	}

	write_into_buffer( c, txt, length );

	if(c->visual_debugging_enabled){
		visual_debug_write( c, txt, length );
	}

    return;
}

/**************************************************************************/
// Low level output function.
bool process_output( connection_data *c, bool fPrompt )
{
	// make sure we are working with a valid descriptor
	if(!IS_VALID(c)){ //IS_VALID ensures d!=NULL
		bugf("process_output(): Being called with invalid connection (socket %d) (character=%s).",
			c->connected_socket, CH(c)?CH(c)->name:"no name");
		return true; // return true so the mud doesn' try to close the socket
	}
	
	if(runlevel!=RUNLEVEL_SHUTING_DOWN){
		flush_cached_write_to_buffer(c); // flush cache first
		// Bust a prompt.
		if( c->showstr_point ){
			if(c->mxp_enabled){
				write_to_buffer( c, FORMATF("`#`=\xaa%s`^\r\n", 
					mxp_tagify("<send href=\"continue\">[Hit Return to continue]</send>")),
					0 );
			}else{
				write_to_buffer( c, "`#`=\xaa""[Hit Return to continue]`^\r\n", 0 );
			}
		}else{
			if( fPrompt && c->pString && c->connected_state == CON_PLAYING ){
				if(HAS_MXP(CH(c))){
					c->character->printf( "`x%s>", mxp_create_send(CH(c), "@") ); 
				}else{
					c->character->print( "`x> ");  // string editor
				}
			}else{
				if( fPrompt && c->connected_state == CON_PLAYING ){					
					char_data *ch;
					
					char_data *victim;
					
					ch = c->character;
					
					// battle prompt 
					if((victim = ch->fighting)!= NULL && can_see(ch,victim))
					{
						int percent;
						char wound[100];
						char buf[MSL];
						
						if(victim->max_hit > 0){
							percent = victim->hit * 100 / victim->max_hit;
						}else{
							percent = -1;
						}
						
						if(percent >= 100){
							sprintf(wound,"is in excellent condition.");
						}else if(percent >= 90){
							sprintf(wound,"has a few scratches.");
						}else if(percent >= 75){
							sprintf(wound,"has some small wounds and bruises.");
						}else if(percent >= 50){
							sprintf(wound,"has quite a few wounds.");
						}else if(percent >= 30){
							sprintf(wound,"has some big nasty wounds and scratches.");
						}else if(percent >= 15){
							sprintf(wound,"looks pretty hurt.");
						}else if(percent >= 0){
							sprintf(wound,"is in awful condition.");
						}else{
							sprintf(wound,"is bleeding to death.");
						}
						sprintf(buf,"%s %s \r\n", PERS(victim, ch), wound);
						
						buf[0]  = UPPER( buf[0] );
						write_to_buffer( c, buf, 0);
					}					
					
					ch = c->original ? c->original : c->character;
					if(!IS_SET(ch->comm, COMM_COMPACT)){
						write_to_buffer( c, "\r\n", 2 );
					}
					
					// send the mxp reset command just before where the prompt is displayed
					// assuming the player is using their prompt.
					if(HAS_MXP(ch)){
						ch->print(MXP_RESET);
					}
					
					// battle lag prompt
					// MF battle lag prompt is able to show the number of commands you have queued

					// figure out how many commands are queued
					if(ch->fighting && !HAS_CONFIG2(ch, CONFIG2_NO_BATTLELAG_PROMPT))
					{
						int queued_commands_count=0;

						// we can only count how many commands are queued, 
						// if we have a descriptor to count from
						if(ch->desc && ch->desc->incomm[0]){
							queued_commands_count++;

							// loop thru counting end of line sequences
							char *pstr=ch->desc->inbuf;
							// ch->desc->inbuf has a double null at the end terminating it
							// which makes it easier to detect the end
							bool non_newline_character_just_seen=true;
							while(*pstr){
								if(*pstr=='\n'){									
									pstr++; // support \n
									if(*pstr=='\r'){										
										pstr++; // support \n\r
									}
									// count this sequence as another command, if the line wasn't blank
									if(non_newline_character_just_seen){
										queued_commands_count++;
									}
									non_newline_character_just_seen=false;
								}else if(*pstr=='\r'){									
									pstr++; // support \r
									if(*pstr=='\n'){										
										pstr++; // support \r\n
									}
									// count this sequence as another command, if the line wasn't blank
									if(non_newline_character_just_seen){
										queued_commands_count++;
									}
									non_newline_character_just_seen=false;
								}else{
									pstr++;
									non_newline_character_just_seen=true;
								}
							}
						}
																	
						if( queued_commands_count || ch->wait>=PULSE_VIOLENCE-2){
							if(TRUE_CH_PCDATA(ch) && !IS_NULLSTR(TRUE_CH_PCDATA(ch)->battlelag)){
								ch->print(TRUE_CH_PCDATA(ch)->battlelag);
							}else{
								ch->print(game_settings->mud_default_battlelag_text);
							}
							ch->printf("{%d}", queued_commands_count);
						}
						
					}

					// send the group prompt to the player/connection if they want it.
					if( !IS_SET(ch->comm, COMM_NOGPROMPT)){
						bust_a_group_prompt(ch);
					}
								
					// send the prompt to the player/connection if they want it.
					if( IS_SET(ch->comm, COMM_PROMPT)){
						bust_a_prompt( c );
					}
					
				}
			}
		}
		
		// Short-circuit if nothing to write.
		flush_cached_write_to_buffer(c); // flush cache first
		if( c->outtop == 0){
			return true;
		}
		
		// Snoop-o-rama.
		if( c->snoop_by != NULL ){
			flush_cached_write_to_buffer(c->snoop_by);
			
			bool parse_normally=c->snoop_by->parse_colour;
			write_into_buffer_thru_process_colour( c->snoop_by, "`_", 0 ); // underline on (\033[4m)
			flush_cached_write_to_buffer(c->snoop_by);
			c->snoop_by->parse_colour=false;
			
			if(c->character){
				write_into_buffer_thru_process_colour( c->snoop_by, c->character->name,0);
			}
			write_into_buffer_thru_process_colour( c->snoop_by, " >\r\n", 3 );
			
			// if we have an MXP snooper, snooping a non MXP user, 
			// we need to lock the mxp mode for the buffer
			if(!HAS_MXPDESC(c) && HAS_MXPDESC(c->snoop_by)){
				write_into_buffer_thru_process_colour(c->snoop_by, MXP_LOCKED_MODE, 4);
			}
			
			// send the output text
			write_into_buffer_thru_process_colour( c->snoop_by, c->outbuf, c->outtop );
			
			// put MXP back into secure mode if appropriate
			if(!HAS_MXPDESC(c) && HAS_MXPDESC(c->snoop_by)){
				write_into_buffer_thru_process_colour( c->snoop_by, "\r\n", 2 );
				write_into_buffer_thru_process_colour(c->snoop_by, MXP_SECURE_MODE, 4);
			}
			flush_cached_write_to_buffer(c->snoop_by);
			c->snoop_by->parse_colour=parse_normally;
			write_into_buffer_thru_process_colour( c->snoop_by, "\033[0m", 0 ); // underline off
			flush_cached_write_to_buffer(c->snoop_by);
		}
	}// runlevel!=RUNLEVEL_SHUTING_DOWN
	
	// OS-dependent output.
	return c->send_outbuf();
}


/**************************************************************************/
// flush a characters output
bool flush_char_outbuffer(char_data *ch)
{
	return ch?ch->desc->flush_output():false;
}
/**************************************************************************/
// Group prompt system - Kal, Dec 2001
// group prompts only work on pets and other players - not charmies
// codes:
// g - begin group section
// G - end group section
// h - lowest hitpoints % for group members in the room
// m - lowest mana % for group members in the room
// v - lowest move % for group members in the room

// p - begin pet section
// P - end pet section
// q - pet hitpoints %
// r - pet mana % 
// s - pet move % 

// N - number of group members in the current room
// c - carriage return
// C - carriage return only if there is preceeding text
// x - number of charmies in the current room (excluding pet)

void bust_a_group_prompt( char_data *ch)
{
	int group_count=0;
	int charmies_count=0;
	bool pet_found=false;
	bool complete;
	char_data *gch;
	char_data *gvictim;
	char *group_prompt;
	char *src;
	char *dest;
	char *i;
	char result[MSL*3];
	char buf[MSL];

	// no group prompt for those with afk turned on
	if(IS_SET(TRUE_CH(ch)->comm,COMM_AFK)){
		return;
	}
	
	// first check if we are going to be displaying this prompt
	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room)
	{
		if( ch!=gch && is_same_group( gch, ch )){
			if(ch->pet==gch){
				pet_found=true;
			}else if(!IS_NPC(gch)){
				group_count++;
			}else{
				charmies_count++;
			}
		}
	}
	if(!pet_found && !group_count){
		return;
	}

	// by here, we know we have a pet and/or group members in the room
	
	// get the players custom group prompt into group_prompt
	group_prompt=ch->gprompt;
	if(IS_NULLSTR(group_prompt)){
		ch->gprompt=str_dup("`#%g[`xgrp `R%hhp `B%mm `M%vmv`&]%G%p[`spet `r%qhp `b%rm `m%smv`&>%P%C");
		group_prompt=ch->gprompt;
	}
	
	dest=result;
	int lowest_percent;
	for(src=group_prompt; !IS_NULLSTR(src); ){

		if( *src!= '%' ){
			*dest++ = *src++;
			continue;
		}

		// we have a % code, skip the % symbol
		src++;

		if(*src=='\0'){ // if a nul follows a %, tell them to fix their group prompt
			ch->println("Your group prompt can't end with a single %");
			return;
		}

		// process the % character
		lowest_percent=101;
		i="";
		switch( *src )
		{
		
		default : // unrecognised option, just use a space
			*dest++=' ';
			continue;
			src++;
			break;

		case '%': // % character itself
			*dest++='%';
			src++;
			continue;
			break;
			
		case 'c': // carriage return
			*dest++='\r';
			*dest++='\n';
			src++;
			continue;
			break;

		case 'C': // carriage return if there is preceeding text in the prompt
			{
				*dest='\0';
				if(str_len(result)>0){
					*dest++='\r';
					*dest++='\n';
				}
				src++;
			}
			continue;
			break;

		case 'g': // start of the group section
			if(!group_count){ 
				// we don't have a group section, therefore 
				// fast forward to the closing %G
				complete=false;
				for(src++; !complete; src++){
					if(*src=='\0'){ 
						// if we find we reached the end of the string, give instructions
						// about needing a %G after a %g in a prompt
						ch->println("Your group prompt needs a %G after the %g before it will be displayed.");
						return;
					}
					if(*src=='%'){ // found a %, could be start of %G
						src++;
						if(*src=='G'){
							complete=true;
							continue;
						}
						if(*src=='\0'){ 
							// if we find we reached the end of the string, give instructions
							// about needing a %G after a %g in a prompt
							ch->println("Your group prompt needs a %G after the %g before it will be displayed.");
							return;
						}
						// otherwise ignore it
					}
				}
				continue;				
			}
			break;

		case 'G':{ // end of the group section - silently ignore it
			}
			break;

		case 'p': // start of the group section
			if(!pet_found){ 
				// we don't have a group section, therefore 
				// fast forward to the closing %G
				complete=false;
				for(src++; !complete; src++){
					if(*src=='\0'){ 
						// if we find we reached the end of the string, give instructions
						// about needing a %P after a %p in a prompt
						ch->println("Your group prompt needs a %P after the %p before it will be displayed.");
						return;
					}
					if(*src=='%'){ // found a %, could be start of %P
						src++;
						if(*src=='P'){
							complete=true;
							continue;
						}
						if(*src=='\0'){ 
							// if we find we reached the end of the string, give instructions
							// about needing a %P after a %p in a prompt
							ch->println("Your group prompt needs a %P after the %p before it will be displayed.");
							return;
						}
						// otherwise ignore it
					}
				}
				continue;				
			}
			break;

		case 'P':{ // end of the pet section - silently ignore it
			}
			break;


		case 'h':{ // - lowest hitpoints % for group members in the room
				if(group_count){ // find the group member in the room with the lowest HP
					for ( gch = ch->in_room->people; gch; gch = gch->next_in_room)
					{
						if( ch!=gch && !IS_NPC(gch) && ch->pet!=gch && is_same_group( gch, ch ) && gch->max_hit!=0){
							if((gch->hit*100/gch->max_hit)<lowest_percent){
								gvictim=gch;
								lowest_percent=(gch->hit*100/gch->max_hit);
							}
						}
					}
					if(lowest_percent<101){
						sprintf( buf, "%3d%%", lowest_percent);
						i =	buf;
					}
				}			
			}
			break;


		case 'm':{ // - lowest mana % for group members in the room
				if(group_count){ // find the group member in the room with the lowest MANA
					for ( gch = ch->in_room->people; gch; gch = gch->next_in_room)
					{
						if( ch!=gch && !IS_NPC(gch) && ch->pet!=gch && is_same_group( gch, ch ) && gch->max_mana!=0){
							if((gch->mana*100/gch->max_mana)<lowest_percent){
								gvictim=gch;
								lowest_percent=(gch->mana*100/gch->max_mana);
							}
						}
					}
					if(lowest_percent<101){
						sprintf( buf, "%3d%%", lowest_percent);
						i =	buf;
					}
				}			
			}
			break;

		case 'v':{ // - lowest movement % for group members in the room
				if(group_count){ // find the group member in the room with the lowest movement %
					for ( gch = ch->in_room->people; gch; gch = gch->next_in_room)
					{
						if( ch!=gch && !IS_NPC(gch) && ch->pet!=gch && is_same_group( gch, ch ) && gch->max_move!=0){
							if((gch->move*100/gch->max_move)<lowest_percent){
								gvictim=gch;
								lowest_percent=(gch->move*100/gch->max_move);
							}
						}
					}
					if(lowest_percent<101){
						sprintf( buf, "%3d%%", lowest_percent);
						i =	buf;
					}
				}			
			}
			break;

		case 'q':{ // - hitpoints % for pet
				if(pet_found && ch->pet->max_hit){ 
					sprintf( buf, "%3d%%", ch->pet->hit*100/ch->pet->max_hit);
					i =	buf;
				}			
			}
			break;

		case 'r':{ // - mana % for pet
				if(pet_found && ch->pet->max_mana){ 
					sprintf( buf, "%3d%%", ch->pet->mana*100/ch->pet->max_mana);
					i =	buf;
				}			
			}
			break;

		case 's':{ // - move % for pet
				if(pet_found && ch->pet->max_move){ 
					sprintf( buf, "%3d%%", ch->pet->move*100/ch->pet->max_move);
					i =	buf;
				}			
			}
			break;

		case 'N':{ // - number of group members in room
				sprintf( buf, "%d", group_count);
				i =	buf;
			}
			break;

		case 'x':{ // - number of charmies in room
				sprintf( buf, "%d", charmies_count);
				i =	buf;
			}
			break;
		}
		src++;
		
		while( (*dest= *i) != '\0' ){
			dest++;
			i++;
		}
	}
	*dest='\0';

	// send the prompt to the player
	if(ch->fighting){
		ch->print("`=q");
	}else{
		ch->print("`=P");
	}
	ch->print(result);
	ch->print("`x");

	if(IS_IRC(ch)){
		ch->println("");
	}					

}

/**************************************************************************/
/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 * - Modified by Kalahn many times
 *
 *  Allows player to customize their prompt valid settings are
 *     a - alignment
 *	   b - position  -  p was already in use :(
 *     c - carriage return
 *     d - misc data about your character status
 *     e - exits for the room
 *     E - exits for the room as MXP links if(mxp is enabled)
 *     g - gold
 *     h - hit points
 *     H - maximum hitpoints
 *     l - language you are speaking
 *     m - mana (disabled with switched)
 *     M - maximum mana
 *     o - olc edit name
 *     O - olc edit vnum
 *     p - role playing score (total)
 *     P - the hp% of your pet if you have one
 *     r - room description
 *     R - immortal only - Room Vnum
 *     s - silver
 *     S - immortal only - short description of switched mob
 *     t - game time
 *     T - server time
 *     v - movement points
 *     V - max movement points
 *     x - experience total (disabled when switched)
 *     X - experience required to level (disabled when switched)
 *     z - immortal only - zone you are in
 *     
 *    -== Immortal only codes ==-
 *     R - room vnum            
 *     S - short description of switched mob
 *     Z - filename of zone you are in
 *     z - zone you are in
 *     ! - CRASHES MUD WHEN IMM SWITCHES 
 *         (Use only for testing automatic debugging scripts)
 */
void visual_debug_flush( connection_data *d);

void bust_a_prompt( connection_data *d )
{
	if(d && d->visual_debugging_enabled && d->visual_debug_flush_before_prompt){
		visual_debug_flush( d );
	}

	char_data *ch = d->character;

	char buf[MSL*3];
    char buf2[MSL];
    char blank_string[] = "";
    const char *str;
    const char *i;
    char *point;
    char doors[MIL];
    char hdoors[MIL]; // hidden doors
    EXIT_DATA *pexit= NULL;
    bool found, hfound=false;
	char *position_name[]  = { "Dead", "Mort", "Incap", "Stunned", "Asleep",
							   "Resting", "Sitting",  "Kneeling", "Fighting", "Standing" };
    int door;
	char *prompt;
	char_data *gch=NULL; 
	char_data *gvictim=NULL; 

	bool group_shown=false; // group stat shown

	if(IS_SWITCHED(ch))
	{
		if(IS_IMMORTAL(ch)){
			if(!IS_SET(TRUE_CH(ch)->dyn, DYN_NO_PROMPT_SWITCHED_PREFIX)){
				ch->printlnf("`=\x8c""***** %s - %s, %s, saycol=`%c%c`=\x8c"", motecol=`%c%c `=\x8c""*****",
					ch->short_descr, ch->language->name, 
					HAS_HOLYSPEECH(ch)?"`#`=\x8d""holyspeech on`&":"holyspeech off",
					ch->saycolour, ch->saycolour,
					ch->motecolour, ch->motecolour);
			}
		}else{
			ch->println("`S** Spirit Walking **");
		}
	}

	if(IS_SET(ch->comm,COMM_TELNET_GA))
	{
		unsigned char b[3];
		b[0]=255;
		b[1]=130;
		b[2]=0;
		write_to_buffer(ch->desc,(char*)&b[0],2);
	}

	if(ch->desc->editor){ // working in olc
		ch->print("`=p");
		prompt = ch->olcprompt;
		if(IS_NULLSTR(prompt)){
			// use the default olc prompt
			prompt ="[`#`m%e`^ in `R%o`mv`R%O`g%Z`^ - %T`^ - %t]";
		}
	}else if(ch->fighting){
		ch->print("`=q");
		prompt = ch->prompt;
	}else{
		ch->print("`=P");
		prompt = ch->prompt;
	}

	if(IS_NULLSTR(prompt)){
        prompt = game_settings->default_prompt;
	}

    point = buf;
    str = prompt;

	if(IS_SET(TRUE_CH(ch)->comm,COMM_AFK))
	{
		ch->print("<AFK> ");
        return;
	}

	while( *str != '\0' )
	{
		if( *str != '%' )
		{
			*point++ = *str++;
			continue;
		}
		++str;
		switch( *str )
		{
		default :
			i = " "; break;
		case '%' :
			sprintf( buf2, "%%" );
			i = buf2; break;
		case 'a' :
			if( ch->level > 9 )
				sprintf( buf2, "%+d.%+d", ch->tendency, ch->alliance );
			else
				sprintf( buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ?
				"evil" : "neutral" );
			i = buf2; break;
		case 'b' :
			sprintf( buf2, "%s%s", 
				position_name[ch->position],
				IS_AFFECTED(ch, AFF_FLYING)?"(airborne)":"");
			i = buf2;
			break;
		case 'c' :
			sprintf(buf2,"%s","\r\n");
			i = buf2; break;

		case 'd': // misc data about your character status - Balo & Kal May 2002 
			{
				buf2[0]='\0';
				if(is_affected(ch,gsn_sneak)){
					strcat( buf2, " `#(`ms`&)");
				}			
				if( is_affected( ch, gsn_invisibility)){
					strcat( buf2, " `#(`Bi`&)");
				}				
				if ( IS_AFFECTED(ch, AFF_HIDE)){
					strcat( buf2, " `#(`Gh`&)");
				}			
				if( INVIS_LEVEL(ch)){
					strcat( buf2, FORMATF(" `#(`Yv%d`&)", INVIS_LEVEL(ch)));
				}			
				if( ch->incog_level){
					strcat( buf2, FORMATF(" `#(`CI%d`&)", ch->incog_level));
				}					
				if(HAS_CHANNELOFF(ch, CHANNEL_QUIET)){
					strcat( buf2, " `#`W[`MQUIET`W]`&");
				}
				i = buf2; 
			}
			break;

		case 'e':
		case 'E':
			found = false;
			doors[0] = '\0';
			for (door = 0; door<MAX_DIR; door++)
			{
				if(ch->in_room 
					&&	(pexit = ch->in_room->exit[door]) != NULL
					&&  pexit ->u1.to_room != NULL
					&&  (can_see_room(ch,pexit->u1.to_room)
					||   (IS_AFFECTED(ch,AFF_INFRARED) 
					&&    !IS_AFFECTED(ch,AFF_BLIND)))
					&&  !IS_SET(pexit->exit_info,EX_CLOSED))
				{
					found = true;
					if(*str=='E'){
						strcat(doors, mxp_create_tag(ch, "Ex", dir_shortname[door]) );
					}else{
						strcat(doors,dir_shortname[door]);
					}
				}
			}
			// hidden exits for those with holylight
			if(IS_SET(ch->act, PLR_HOLYLIGHT))
			{
				hfound = false;
				hdoors[0] = '(';
				hdoors[1] = '\0';
				for (door = 0; door < MAX_DIR; door++)
				{
					if((pexit = ch->in_room->exit[door]) != NULL
						&&  pexit ->u1.to_room != NULL
						&&  (can_see_room(ch,pexit->u1.to_room)
						||   (IS_AFFECTED(ch,AFF_INFRARED) 
						&&    !IS_AFFECTED(ch,AFF_BLIND)))
						&&  IS_SET(pexit->exit_info,EX_CLOSED))
					{
						hfound = true;
						if(*str=='E'){
							strcat(hdoors, mxp_create_tag(ch, "Ex", dir_shortname[door]) );
						}else{
							strcat(hdoors,dir_shortname[door]);
						}
					}
				}
				strcat(hdoors,")");
				if(hfound) strcat(doors,hdoors);
			}
			
			if(!found && !hfound){
				strcat(doors,"none");
			}

			strcpy(buf2,doors);

			i = buf2; break;
		case 'g' :
			sprintf( buf2, "%ld", ch->gold);
			i = buf2; break;

		case 'G' : // group templates following character has template 0->9, A->Z etc
			if(*(str+1)!='\0'){ // can't have %G by itself
				++str;
				if(!ch->in_room || !ch->in_room->people){
					i="BUG, !ch->in_room || !ch->in_room->people!!!";
					break;
				}

				int lowest_percent=101;
				gvictim=ch;

				switch(*str){
					// lowest movement in group - don't show self
					case '0': // in format 'lowestpercent% '
					case '1': // in format 'lowestpercent%mv '
					case '2': // in format 'lowestpercent%move '
					{
						for ( gch = ch->in_room->people; gch; gch = gch->next_in_room)
						{
							if( ch!=gch && is_same_group( gch, ch ) && gch->max_move!=0){
								if((gch->move*100/gch->max_move)<lowest_percent){
									gvictim=gch;
									lowest_percent=(gch->move*100/gch->max_move);
								}
							}
						}
						if(lowest_percent<101){
							if(*str=='2'){
								sprintf( buf2, "%d%%move ", lowest_percent);
								i =	buf2; 
							}else if(*str=='1'){
								sprintf( buf2, "%d%%mv ", lowest_percent);
								i =	buf2; 
							}else{
								sprintf( buf2, "%d%% ", lowest_percent);
								i =	buf2; 
							}
							group_shown=true;
						}else{
							i = "";
						}
						break;
					}

					// lowest mana in group - don't show self
					case '3': // in format 'lowestpercent% '
					case '4': // in format 'lowestpercent%m '
					case '5': // in format 'lowestpercent%mn '
					case '6': // in format 'lowestpercent%mana '
					{
						for ( gch = ch->in_room->people; gch; gch = gch->next_in_room)
						{
							if( ch!=gch && is_same_group( gch, ch ) && gch->max_mana!=0){
								if((gch->mana*100/gch->max_mana)<lowest_percent){
									gvictim=gch;
									lowest_percent=(gch->mana*100/gch->max_mana);
								}
							}
						}
						if(lowest_percent<101){
							if(*str=='6'){
								sprintf( buf2, "%d%%mana ", lowest_percent);
								i =	buf2; 
							}else if(*str=='5'){
								sprintf( buf2, "%d%%mn ", lowest_percent);
								i =	buf2; 
							}else if(*str=='4'){
								sprintf( buf2, "%d%%m ", lowest_percent);
								i =	buf2; 
							}else{
								sprintf( buf2, "%d%% ", lowest_percent);
								i =	buf2; 
							}
							group_shown=true;
						}else{
							i = "";
						}
						break;
					}

					// lowest hp in group - don't show self
					case '7': // in format 'lowestpercent% '
					case '8': // in format 'lowestpercent%h '
					case '9': // in format 'lowestpercent%hp '
					{
						for ( gch = ch->in_room->people; gch; gch = gch->next_in_room)
						{
							if( ch!=gch && is_same_group( gch, ch ) && gch->max_hit!=0){
								if((gch->hit*100/gch->max_hit)<lowest_percent){
									gvictim=gch;
									lowest_percent=(gch->hit*100/gch->max_hit);
								}
							}
						}
						if(lowest_percent<101){
							if(*str=='9'){
								sprintf( buf2, "%d%%hp ", lowest_percent);
								i =	buf2; 
							}else if(*str=='8'){
								sprintf( buf2, "%d%%h ", lowest_percent);
								i =	buf2; 
							}else if(*str=='7'){
								sprintf( buf2, "%d%% ", lowest_percent);
								i =	buf2; 
							}else{
								sprintf( buf2, "%d%% ", lowest_percent);
								i =	buf2; 
							}
							group_shown=true;
						}else{
							i = "";
						}
						break;
					}		

					case 'C': // return if group_shown is true;
					{
						if(group_shown){
							i = "\r\n";
						}else{
							i = "";
						}
						break;
					};					
					case 'G': // close bracket than if group_shown is true;
					{
						if(group_shown){
							i = "]";
						}else{
							i = "";
						}
						break;
					};				
					case 'L': // open bracket than if group_shown is true;
					{
						if(group_shown){
							i = "[";
						}else{
							i = "";
						}
						break;
					};					
					default:
						i = "?unknown group code in use!?"; break;
				}
			}else{
				i = "missing tailing code for %G at end of prompt!";
			}
			break;
		case 'h' :
			sprintf( buf2, "%d", ch->hit );
			i = buf2; break;
		case 'H' :
			sprintf( buf2, "%d", ch->max_hit );
			i = buf2; break;
		case 'l' :
			sprintf( buf2, "%s", ch->language->name);
			i = buf2; break;
		case 'm' :
			if( ch->desc->original == NULL ){       
				sprintf( buf2, "%d", ch->mana );
			}else{
				strcpy( buf2, blank_string);
			}
			i = buf2; break;
		case 'M' :
			sprintf( buf2, "%d", ch->max_mana );
			i = buf2; break;
		case 'o' :
			sprintf( buf2, "%s", olc_ed_name(ch) );
			i = buf2; break;
		case 'O' :
			sprintf( buf2, "%s", olc_ed_vnum(ch) );
			i = buf2; break;
		case 'p' :
			if( ch->desc->original == NULL ){
				sprintf( buf2, "%ld", ch->pcdata->rp_points);
			}else{
				strcpy( buf2, blank_string);
			}
			i = buf2; break;
		case 'P' :
			if(ch->pet)
			{
				if(ch->pet->in_room == ch->in_room)
					sprintf( buf2,"%3d%%",(int) ch->pet->hit*100/ch->pet->max_hit);				
				else
					sprintf( buf2,"???%%");				
				i = buf2;
			}
			else
			{
				i = blank_string;
			}
			break;
		case 'r' :
			if( ch->in_room != NULL ){
				sprintf( buf2, "%s", 
				((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)) ||
				(!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
				? ch->in_room->name : "darkness");
			}else{
				strcpy( buf2, blank_string);
			}
			i = buf2; break;
		case 's' :
			sprintf( buf2, "%ld", ch->silver);
			i = buf2; break;

    
			
		case 't' :
			if(IS_IMMORTAL(ch)){
				sprintf( buf2, "%d:%02d%s",
					(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
					time_info.minute,
					time_info.hour >= 12 ? "pm" : "am");
			}else{
				sprintf( buf2, "%d:%02d%s",
					(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
					((int)time_info.minute/5)*5,
					time_info.hour >= 12 ? "pm" : "am");
			}
			i=buf2; break;

		case 'T' :
			sprintf( buf2, "%s", shorttime(NULL));
			i = buf2; break;
		case 'v' :
			if(ch->mounted_on!=NULL)
			{
				sprintf( buf2, "MOUNT %d", ch->mounted_on->move ); 
			}
			else
			{
				sprintf( buf2, "%d", ch->move );
			}
			
			i = buf2; 
			
			break;
		case 'V' :
			if(ch->mounted_on!=NULL)
			{
				sprintf( buf2, "%d", ch->mounted_on->max_move ); 
			}
			else
			{
				sprintf( buf2, "%d", ch->max_move );
			}
			i = buf2; break;

		case 'W' : // word version of hitpoints
			{
				int percent=ch->hit*100/(ch->max_hit?ch->max_hit:1);

				if(percent >= 100)
					i="excellent";
				else if(percent >= 90)
					i="a few scratches";
				else if(percent >= 75)
					i="small wounds and bruises";
				else if(percent >=  50)
					i="quite a few wounds";
				else if(percent >= 30)
					i="big nasty wounds and scratches";
				else if(percent >= 15)
					i="pretty hurt";
				else if(percent >= 0 )
					i="`#`Rawful condition`&";
				else
					i="`#`Rbleeding`& to death!";		
			}
			break;


		case 'x' :
			sprintf( buf2, "%d", ch->exp );
			i = buf2; break;
		case 'X' :
			if(!IS_NPC(ch)){ 
				sprintf(buf2, "%d", 
					(ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
			}else{
				strcpy( buf2, blank_string);
			}
			i = buf2; break;
				
		case 'z' :
			if( ch->in_room != NULL ){
				sprintf( buf2, "%s", ch->in_room->area->name );
			}else{
				strcpy( buf2, blank_string);
			}
			i = buf2; break;


			// Immortal Prompt codes 
		case 'R' :
			if( IS_IMMORTAL( ch ) && ch->in_room != NULL ){
				sprintf( buf2, "%d", ch->in_room->vnum );
			}else{
				strcpy( buf2, blank_string);
			}
			i = buf2; break;
		case 'S' :
			if( IS_IMMORTAL( ch ) && ch->desc->original != NULL ){
				sprintf( buf2, "%s", ch->short_descr);
			}else{
				strcpy( buf2, blank_string);
			}
			i = buf2; break;

		case 'Z' :
			if( IS_IMMORTAL( ch ) && ch->in_room != NULL ) {
				sprintf( buf2, "%s", area_fname(ch->in_room->area));
			}else{
				strcpy( buf2, blank_string);
			}
			i = buf2; break;
		case '!' :  // this code is used for crash testing the mud 
					//- test debugging systems etc
			if(IS_IMMORTAL(ch) &&  ch->desc->original != NULL ) {
				logf("debug prompt code '%%!' in immortals prompt - immortal has switched, "
					"crashing the mud intentionally (use for auto debug testing)");
				sprintf(buf2, "%d", 
					(ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
			}else{
				sprintf( buf2, "!!!"); // tell imm/morts they have the debugging code showing
			}
			i = buf2; break;
		}
		++str;
		while( (*point = *i) != '\0' ){
			++point, ++i;
		}
	}
	*point='\0';

	ch->print(buf);

	if(IS_SET(ch->comm,COMM_TELNET_GA))
	{
		unsigned char b[3]={IAC, GA, '\0'};
		write_to_buffer(ch->desc,(char*)&b[0],2);
	}
	ch->print("`x");

	if(!IS_NULLSTR(ch->prefix)){
		ch->print(ch->prefix);
	}

	if(IS_IRC(ch)){
		ch->println("");
	}					

	if(d && d->visual_debugging_enabled && d->visual_debug_flush_before_prompt && d->visual_debug_strip_prompt){
		d->visual_debug_buffer[0]='\0';
	}
	return;
}

/**************************************************************************/
void flush_cached_write_to_buffer(connection_data *c)
{
	// if data in the previous buffer flush it thru
	if(c->condense_count){
		if(c->condense_count>2){
			char buf[15];
			sprintf(buf,"(x%d) ",c->condense_count);
			write_into_buffer_thru_process_colour( c, buf, str_len(buf));
		}
		if(c->condense_count==2){
			write_into_buffer_thru_process_colour( c, c->condense_buffer, c->condense_lastlen);
		}
		write_into_buffer_thru_process_colour( c, c->condense_buffer, c->condense_lastlen);
		c->condense_count=0;
	}
}
/**************************************************************************/
// Append onto an output buffer.
void write_to_buffer( connection_data *c, const char *txt, int length )
{
	// make sure we are working with a valid descriptor
	if(!c){
		bugf("write_to_buffer(): Being called with a NULL connection! - text = '%s'.", txt);
		return;
	}

	if(!IS_VALID(c)){ //IS_VALID ensures c!=NULL
		bugf("write_to_buffer(): Being called with invalid connection (socket %d) (%s).",
			c->connected_socket, CH(c)?CH(c)->name:"no name");
		return;
	}

    // Find length incase caller didn't.
    if( length <= 0 ){
		length = str_len(txt);
	}

	if(length==0){
		return;
	}

	// condensing system makes no difference in combat 
	// nor any non playing state
	if( c->connected_state==CON_PLAYING && c->character && !c->character->fighting)
	{ 
		if(length<MAX_CONDENSE_LENGTH && length>2) // cache it
		{
			if(c->condense_count)
			{
				if(c->condense_lastlen==length 
					&& !strcmp(c->condense_buffer, txt))
				{
					c->condense_count++;
					return;
				}

				// different - flush the previous
				flush_cached_write_to_buffer(c);
			}
			// adding a new one
			strncpy(c->condense_buffer, txt, length+1);
			c->condense_buffer[length]='\0';
			c->condense_count=1;
			c->condense_lastlen=length;
			return;
		}
	}

	// too big to cache
	flush_cached_write_to_buffer(c);
	write_into_buffer_thru_process_colour( c, txt, length );
}
/**************************************************************************/
// Append onto an output buffer, in black and white
void write_to_buffer_bw( connection_data *c, const char *txt, int length)
{
	// make sure we are working with a valid descriptor
	if(!IS_VALID(c)){ //IS_VALID ensures d!=NULL
		bugf("write_to_buffer_bw(): Being called with invalid connection (socket %d) (%s).",
			c->connected_socket, CH(c)?CH(c)->name:"no name");
		return;
	}

	flush_cached_write_to_buffer(c);
	if(c->parse_colour){
		c->parse_colour=false;
		write_to_buffer( c, txt, length);
		flush_cached_write_to_buffer(c);
		c->parse_colour=true;
	}else{
		write_to_buffer( c, txt, length);
		flush_cached_write_to_buffer(c);
	}
}


/**************************************************************************/
// Look for link-dead player to reconnect.
bool check_reconnect( connection_data *c, char *, bool fConn )
{
	char_data *ch;

	for ( ch = char_list; ch != NULL; ch = ch->next )
	{
		if( !IS_NPC(ch)
		&&   (!fConn || !ch->desc)
		&&   !str_cmp( c->character->name, ch->name ) )
		{
			logf("Socket %d is reconnecting (%s)", c->connected_socket, ch->name );

			if( fConn == false )
			{
				replace_string( c->character->pcdata->pwd, ch->pcdata->pwd );
			}
			else // ditch the duplicates that were just loaded
			{
				// first ditch the newly loaded pet if one exists
				if(c->character->pet){
					c->character->pet->in_room=NULL;
					char_to_room( c->character->pet, get_room_index(ROOM_VNUM_LIMBO) );
					extract_char( c->character->pet, true );
				}
				// now ditch the loaded duplicate character
                c->character->in_room = NULL;
				free_char( c->character );

				// now attach to original character
				c->character = ch;
				ch->desc     = c;
				ch->timer    = 0;
				ch->idle     = 0;

				// reset mxp
				if(ch->pcdata){
					ch->pcdata->mxp_enabled=false;
				}
				ch->mxp_send_init(); 
				
				ch->println("Reconnecting.  Use `=Creplay`x, `=Creplayroom`x, and `=Creplaychan`x to see missed events.");
				act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );

                logf("%s@%s reconnected. (sock=%d)",ch->name, c->remote_hostname, c->connected_socket);

                wiznet("$N reconnects to $S linkdead character.",
					 ch,NULL,WIZ_LINKS,0,0);
				c->connected_state = CON_PLAYING;

				if(!GAMESETTING2(GAMESET2_DONT_DISPLAY_WHO_4_LOGIN)){
					// give autowho to everyone 
					// - doesn't show imms automatically though for morts
					do_who( ch, "-noimm4morts" );
				}
			}
			return true;
		}
    }
    return false;
}



/**************************************************************************/
// Check if already playing.
bool check_playing( connection_data *c, char *name )
{
    connection_data *cold;

    for ( cold = connection_list; cold; cold = cold->next )
    {
		if( cold != c
			&&   CH(cold)
			&&   cold->connected_state != CON_GET_NAME
			&&   cold->connected_state != CON_GET_OLD_PASSWORD
			&&   !str_cmp( name, CH(cold)->name) )
		{
			logf( "check_playing() socket %d(%s) found that %d(%s) is already playing character '%s'!",
				c->connected_socket, c->remote_hostname,
				cold->connected_socket, cold->remote_hostname, CH(cold)->name);
			
			write_to_buffer( c, "That character is already playing.\r\n",0);
			write_to_buffer( c, "Do you wish to connect anyway (Y/N)?",0);
			if(IS_IRCCON(c))
				write_to_buffer( c, "\r\n", 0 );
			c->connected_state = CON_BREAK_CONNECT;
			return true;
		}
    }

    return false;
}


/**************************************************************************/
void stop_idling( char_data *ch, char *command )
{
	if(ch == NULL
		||   ch->desc == NULL
		||   ch->desc->connected_state != CON_PLAYING){
		return;
	}

	ch->timer = 0;
	ch->idle = 0;

	// check if we remove their autoafk status
    if(!IS_NPC(ch) 
		&& IS_SET(ch->comm,COMM_AFK) 
		&& !strcmp(ch->pcdata->afk_message,"Auto AFK"))
    {
		if(str_cmp(command, "afk")){
			if(str_len(command)<=2){
				ch->println("Auto AFK not taken off until you type more than 2 characters.");
			}else{
				REMOVE_BIT(TRUE_CH(ch)->comm,COMM_AFK);
				ch->println("Auto AFK mode removed.");
				ch->println(" Type 'replay' to see any tells you may have received.");
			}
		}
    }

	// check if we want to remove players from limbo
    if( ch->was_in_room == NULL
		||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
		return;
	
	char_from_room( ch );
	char_to_room( ch, ch->was_in_room );
	ch->was_in_room=NULL;
	act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
	return;
}
/**************************************************************************/
// string pager 
void show_string(connection_data *d, char *input)
{
	static char *buffer; // permanently allocated buffer
	if(buffer==NULL){ // don't use the stack for the pager memory 
		buffer=new char[HSL + MSL*2];
		assertp(buffer);
	}
    char buf[MIL];
    register char *scan, *chk, *pad;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
	
	// support typing 'continue', and not break out of the pager
	// used for MXP
	if(buf[0]=='c' && !str_cmp(buf, "continue")){
		buf[0]='\0';
	}

    if(buf[0] != '\0')
	{
		if(d->showstr_head)
		{
			free_string(d->showstr_head);
			d->showstr_head = 0;
		}
		d->showstr_point  = 0;
		return;
    }

    if(d->character && !IS_IRCCON(d)){
		show_lines = d->character->lines;
    }else{
		show_lines = 0;
	}

    for (scan = buffer, pad=buffer+(MSL*20)-5; ; scan++, d->showstr_point++)
    {
		if(scan<pad
			&& (
				(((*scan = *d->showstr_point) == '\r' || *scan == '\n')
				&& (toggle = -toggle) < 0)
				|| (*scan== '1' && *(scan-1)=='`') 
				) // support for `1 newline colour codes
			)
		{
			lines++;		
		}
		else
		{
			if(!*scan || (show_lines > 0 && lines >= show_lines) || scan>=pad)
			{
				if(*scan=='\n'){ // budget hack till rewritten
					*(++scan) = '\0';
				}else{
					*(scan) = '\0';
				}
				write_to_buffer(d,buffer,str_len(buffer));
				for (chk = d->showstr_point; is_space(*chk); chk++);
				{
					if(!*chk)
					{
						if(d->showstr_head)
						{
							free_string(d->showstr_head);
							d->showstr_head = 0;
						}
						d->showstr_point  = 0;
					}
				}
				return;
			}
		}
    }
	// buffer overrun checks
	if(scan==pad){
		scan--;
		*scan='\0';

	}
	return;
}
	
/**************************************************************************/
void act (const char *format, char_data *ch, const void *arg1, 
		const void *arg2, ACTTO_TYPE type)
{
    // to be compatible with older code 
    act_new(format,ch,arg1,arg2,type,POS_RESTING);
}

/**************************************************************************/
// this function appends autodamage information... always directed to char
void act_with_autodam_to_char(const char *format, char_data *ch, const void *arg1, 
		const void *arg2, int damage_result) // always TO_CHAR
{
	char str[MSL];
	sprintf(str, "%s%s", format, autodamtext(ch, damage_result));
    // to be compatible with older code 
    act_new(str,ch,arg1,arg2,TO_CHAR,POS_RESTING);
}

/**************************************************************************/
char *act_new( const char *format, char_data *ch, const void *arg1, 
			 const void *arg2, ACTTO_TYPE type, int min_pos)
{
	const char *colour_stripped_format="";
	if(EXECUTING_SOCIAL){
		char nocolour_format[MSL];		
		assert(str_len(format)<MSL);
		// if this is a colour social, we strip out the colour from the 
		// format before we start... then we only show it to those who
		// have socials with stripped colour.

		// if EXECUTING_SOCIAL_IN_COLOUR is set, we only show those acts
		// to those who have socials in colour enabled
		strcpy(nocolour_format,strip_colour(format));		
		colour_stripped_format=nocolour_format;
	}

    char_data	*to;
    char_data	*vch  = ( char_data * ) arg2;
	char_data   *vch2 = ( char_data * ) arg1;
    OBJ_DATA	*obj1 = ( OBJ_DATA  * ) arg1;
    OBJ_DATA	*obj2 = ( OBJ_DATA  * ) arg2;
    const char	*str;
    char		*i = NULL;
    char		*point;
    static char	buf[MSL];
    char		fname[MIL];
	int			wizi_level=0;
	
	// Discard null and zero-length messages	
    if(IS_NULLSTR(format)){
		return "";
	}
	
    // discard null chars and rooms
    if( !ch || !ch->in_room )
		return "";
	
    to = ch->in_room->people;
	int number_in_room=ch->in_room->number_in_room;
    if( type == TO_VICT )
    {
		if( !vch ){
			bug("Act: null vch with TO_VICT.");
			return "";
		}
		if( !vch->in_room ){
			return "";
		}
		to = vch->in_room->people;
		number_in_room=vch->in_room->number_in_room;
    }

	if( type == TO_WORLD ){
		if( !vch2 ){   
			bug( "Act: null vch2 with TO_WORLD." );   
			return "";   
		}   
		
		if (vch2->in_room == NULL){   
			return "";   
		}   
		to = vch2->in_room->people;   
		number_in_room=vch2->in_room->number_in_room;
	}

	// note: number_in_room is used to prevent a mudprog on two mobs in 
	// a room creating an endless loop by removing themselves from
	// the room and putting themselves back in the room - Kal, June 01
    for( ; to && --number_in_room>=0; to = to->next_in_room )
    {
		if((!IS_NPC(to) && to->desc == NULL)
			|| (IS_UNSWITCHED_MOB(to) && !HAS_TRIGGER(to, MTRIG_ACT) && !IS_SET(to->act,ACT_MOBLOG))
			|| to->position < min_pos)
		{
			continue;
		}
		
		if( ( type == TO_CHAR ) && to != ch )
			continue;
		if( type == TO_VICT && ( to != vch || to == ch ) )
			continue;
		if( type == TO_ROOM && to == ch )
			continue;
		if( type == TO_NOTVICT && (to == ch || to == vch) )
			continue;
		if ( type == TO_WORLD && (to == ch || to == vch || to != vch2) )
            continue;

		point   = buf;

		// set the input format to a colour free version if it is a 
		// social and this is for a mob or player opting to not have colour
		if(EXECUTING_SOCIAL){
			if(to->pcdata){
				if(to->pcdata->preference_colour_in_socials==PREF_OFF
					||(to->pcdata->preference_colour_in_socials==PREF_AUTOSENSE
						&& !GAMESETTING4(GAMESET4_GAMEDEFAULT_COLOUR_IN_SOCIALS_ON))
				){
					// use opted to have socials without colour
					// or they used mud wide setting, which is off
					str = colour_stripped_format;
				}else{
					// format potentially with colour in it
					str = format;
				}
			}else{
				// mobs don't get colour in their socials
				str = colour_stripped_format;
			}
		}else{
			str = format;
		}
		
		while ( *str != '\0' )
		{
			if( *str != '$' ){
				*point++ = *str++;
				continue;
			}
			++str;
			i = " <@@@> ";
			if( !arg2 && *str >= 'A' && *str <= 'Z' ){
				bugf( "Act: missing arg2 for code %d (%c).", *str, *str);
				i = " <@@@> ";
			}else{
				switch ( *str )
				{
				default:  bugf( "Act: bad code %d (%c).", *str, *str);
					i = " <@@@> ";
					break;
					// Thx alex for 't' idea
				case 't': i = (char *) arg1;						break;
				case 'T': i = (char *) arg2;						break;
				case '$': i ="$";									break; // $$ = $
				case 'n': i = PERS( ch,  to  );	
					wizi_level=UMAX(wizi_level,INVIS_LEVEL(ch));	break;
				case 'N': i = PERS( vch, to  );
					wizi_level=UMAX(wizi_level,INVIS_LEVEL(vch));	break;
				case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
				case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];	break;
				case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];	break;
				case 'M': i = him_her [URANGE(0, vch ->sex, 2)];	break;
				case 's': i = his_her [URANGE(0, ch  ->sex, 2)];	break;
				case 'S': i = his_her [URANGE(0, vch ->sex, 2)];	break;
				case 'p':
					i = (can_see_obj( to, obj1 )
						? (obj1->short_descr?obj1->short_descr:(char*)""):(char*)"something");
					if(IS_NULLSTR(i)){
						i = "NULL OBJECT";
						bug("Act: bad code with $p.");
					}
					break;
				case 'P':
					i = (can_see_obj( to, obj2 )
						? (obj2->short_descr?obj2->short_descr:(char*)""):(char*)"something");
					if(IS_NULLSTR(i)){
						i = "NULL OBJECT";
						bug("Act: bad code with $P.");
					}
					break;
				case 'd':
					if( arg2 == NULL || ((char *) arg2)[0] == '\0' )
					{
						i = "door";
					}
					else
					{
						one_argument( (char *) arg2, fname );
						i = fname;
					}
					break;
				}
			}
			++str;
			while ( ( *point = *i ) != '\0' ){
				++point, ++i;
			}
		}
		
		*point   = '\0';
		
		// uppercase the start of the act
		strcpy(buf, icapitalize(buf));
		
		// display it to the player/mob with a moblog/act trigger
		{
			if(wizi_level)
			{
				if(IS_TRUSTED(to, wizi_level))
				{			
					to->printlnf("%s%s",
						(IS_IMMORTAL(to)?FORMATF("[Wizi %d] ", wizi_level):""),
						buf);

					if(RECORD_TO_REPLAYROOM){
						to->record_replayroom_event(
							FORMATF("%s%s",
						(IS_IMMORTAL(to)?FORMATF("[Wizi %d] ", wizi_level):""),
						buf));
					}
				}
			}else{
				to->println(buf);
				if(RECORD_TO_REPLAYROOM){
					to->record_replayroom_event(buf);
				}
			}

			if(!to->desc && IS_NPC( to ) && (HAS_TRIGGER(to, MTRIG_ACT) || IS_SET(to->act,ACT_MOBLOG))){
				process_moblog(to, buf);
				if( !IS_NPC( ch )){
					mp_act_trigger( buf, to, ch, arg1, arg2, MTRIG_ACT );
				}
			}
		}
    }
	
    return buf;
}

/**************************************************************************/
// Abort the mud and make a corefile
void do_abort()
{
	logf("do_abort()");
    write_last_command();
	abort();
}

/**************************************************************************/
char * get_piperesult( char *cmd );
/**************************************************************************/
// make a corefile but don't stop the mud - unix only 
void make_corefile()
{
#ifdef unix
	if(!fork()){
		if(!fork()){
			abort();
		}else{ // debug the core
			get_piperesult("sleep 15");
			get_piperesult("processcore");
			exit_error(1, "make_corefile", "creating corefile");
		}
	}
#else
	do_abort();
#endif
}
/**************************************************************************/
#ifdef WIN32 // win32 doesn't have this natively
void gettimeofday( struct timeval *tp, void *tzp )
{
	struct _timeb temp_time;
	_ftime( &temp_time );
	tp->tv_sec=(long)temp_time.time; // in winsock.h tv_sec is defined as a long
	tp->tv_usec=temp_time.millitm;
}
#endif

/**************************************************************************/
void colour_convert_prefix(char colcode, char *text);
/**************************************************************************/
void process_input(connection_data *c)
{
	// make sure we are working with a valid descriptor
	if(!IS_VALID(c)){ //IS_VALID ensures c!=NULL
		bugf("process_input(): Being called with invalid connection (socket %d) (%s).",
			c->connected_socket, CH(c)?CH(c)->name:"no name");
		return;
	}

	//handle daze and waits
	if(c->character && c->character->daze > 0){
		--c->character->daze;
	}

	if( c->character && c->character->wait > 0 ){
		--c->character->wait;
		return;
	}

	// get input from a descriptor
	read_from_buffer( c );

	if( c->incomm[0] )
	{
//		logf("process_input():'%s'", c->incomm); // debug code
		c->idle_since = current_time; // update idle timer

		c->fcommand     = true;
		stop_idling( c->character, c->incomm );

		// ### handle snooping here - system moved from interp ###
		// so can snoop olc for training purposes...

		// only unsnoopable command from interp.c tables 
		// was password, 
		// I haven't bothered writing checks for that, 
		// if you are really worried about it, 
		// you can write it yourself :)
		//
		// - Kal, Jan 98.
		if( c->snoop_by)
		{
			flush_cached_write_to_buffer(c->snoop_by);
			bool parse_normally=c->snoop_by->parse_colour;
			c->snoop_by->parse_colour=false;
			
			write_to_buffer( c->snoop_by, "##",    2 );
			if(c->original){
				write_to_buffer( c->snoop_by, c->original->name, 0);
			}else{
				write_to_buffer( c->snoop_by, c->character?c->character->name:"unknown name?", 0);
			}
			write_to_buffer( c->snoop_by, "##>",   3 );
			if(    c->connected_state == CON_GET_OLD_PASSWORD
				|| c->connected_state == CON_GET_NEW_PASSWORD 
				|| c->connected_state == CON_CONFIRM_NEW_PASSWORD)
			{
				write_to_buffer( c->snoop_by, (char *)"-=hidden password=-", 0 );
			}else{
				write_to_buffer( c->snoop_by, (char *) c->incomm, 0 );
			}
			write_to_buffer( c->snoop_by, "\r\n",  2 );

			flush_cached_write_to_buffer(c->snoop_by);
			c->snoop_by->parse_colour=parse_normally;
		}

		if( c->command_snoop)
		{
			flush_cached_write_to_buffer(c->command_snoop);
			bool parse_normally=c->command_snoop->parse_colour;
			c->command_snoop->parse_colour=false;

			write_to_buffer( c->command_snoop, "!",    2 );
			write_to_buffer( c->command_snoop, shorttime(NULL),    0);
			write_to_buffer( c->command_snoop, "!",    2 );		
			if(c->original){
				write_to_buffer( c->command_snoop, c->original->name, 0);
			}else{
				write_to_buffer( c->command_snoop, c->character->name, 0);
			}
			write_to_buffer( c->command_snoop, "!!>",   3 );
			write_to_buffer( c->command_snoop, (char *) c->incomm, 0 );
			write_to_buffer( c->command_snoop, "\r\n",  2 );

			flush_cached_write_to_buffer(c->command_snoop);
			c->command_snoop->parse_colour=parse_normally;
		}

		// Record the input tail
		++inputtail_index%=MAX_INPUTTAIL; // rotate the buffer
		if(CH(c)){
			if(c->connected_state==CON_GET_OLD_PASSWORD 
				|| c->connected_state==CON_GET_NEW_PASSWORD
				|| c->connected_state==CON_FTP_AUTH
				|| c->connected_state==CON_CONFIRM_NEW_PASSWORD){
				// time name/short <descriptor/vnum of mob> Room%vnum
				sprintf(inputtail[inputtail_index],"%s %s<%d> (%d) R%d C%d E%d '%s'",
					shorttime(NULL),
					IS_NPC(CH(c)) ? CH(c)->short_descr : CH(c)->name,
					c->character && c->character->pIndexData ? c->character->pIndexData->vnum : 0,
					c->connected_socket,
					CH(c)->in_room ? CH(c)->in_room->vnum : 0,
					c->connected_state,
					c->editor,
					"<a password of some kind - hidden>");
			}else{
				// time name/short <descriptor/vnum of mob> Room%vnum
				sprintf(inputtail[inputtail_index],"%s %s<%d> (%d) R%d C%d E%d '%s'",
					shorttime(NULL),
					IS_NPC(CH(c)) ? CH(c)->short_descr : CH(c)->name,				
					c->character && IS_NPC(c->character) ? c->character->pIndexData->vnum : 0,
					c->connected_socket,
					CH(c)->in_room ? CH(c)->in_room->vnum : 0,
					c->connected_state,
					c->editor,
					(char *) c->incomm);
			}
		}else{
			if(c->connected_state==CON_FTP_AUTH){
				sprintf (inputtail[inputtail_index], 
					"%s input from socket %d - no character attached: dawnftp login",
					shorttime(NULL), c->connected_socket);
			}else{
				sprintf (inputtail[inputtail_index], 
					"%s input from socket %d - no character attached: %s",
					shorttime(NULL), c->connected_socket,(char *) c->incomm);
			}
		}

		// Record the input
		if(CH(c)){
		    sprintf (last_input, "Start input %d> [%5d] %s in [%5d] %s: %s",
				c->connected_socket,
				IS_NPC(CH(c)) ? CH(c)->pIndexData->vnum : 0,
				IS_NPC(CH(c)) ? CH(c)->short_descr : CH(c)->name,
				CH(c)->in_room ? CH(c)->in_room->vnum : 0,
				CH(c)->in_room ? CH(c)->in_room->name : "(not in a room)",
				(char *) c->incomm);
		}else{
		    sprintf(last_input, "Start input %d> no character or original: %s",
				c->connected_socket,(char *) c->incomm);
		}

		// translate a players choice of colour prefix to the internal version
		if(c->character && c->character->colour_prefix!=COLOURCODE){
			// colour_convert_prefix() below writes directly back to c->incomm
			// assuming there is room for twice the size of c->incomm
			colour_convert_prefix(c->character->colour_prefix, c->incomm); 
		}

		// OLC
		if( c->showstr_point ){
			show_string( c, c->incomm );
		}else if( c->pString ){
			string_add( c->character, c->incomm );
		}else{
			switch ( c->connected_state ){

				
			case CON_PLAYING:
				if( !run_olc_editor( c ) ){
					interpret(c->character,substitute_alias( c, c->incomm ));
				}
				break;

			default:
				// while loop here so we can snarf 
				// all mudftp data in one go
				if(c->incomm[0]){
					sh_int last_connected_state=0;
					while(c->incomm[0]){
						last_connected_state=c->connected_state;
						nanny(c, c->incomm);
						if(c->connected_state != CON_FTP_DATA)
							break;
						
						c->incomm[0] = '\0';
						read_from_buffer( c );
					}
					if(last_connected_state!= CON_FTP_DATA){
						logf("End nanny state %2d, con state->%d (socket=%d)", 
							last_connected_state, 
							c->connected_state, 
							c->connected_socket);
					}
				}
				break;
			}
		}

		// Record the input finished
		if(CH(c)){
		    sprintf (last_input, "End input %d> [%5d] %s in [%5d] %s: %s",
				c->connected_socket,
				IS_NPC(CH(c)) ? CH(c)->pIndexData->vnum : 0,
				IS_NPC(CH(c)) ? CH(c)->short_descr : CH(c)->name,
				CH(c)->in_room ? CH(c)->in_room->vnum : 0,
				CH(c)->in_room ? CH(c)->in_room->name : "(not in a room)",
				(char *) c->incomm);
		}
		else
		{
		    sprintf (last_input, "End input %d> no character or original: %s",
				c->connected_socket,(char *) c->incomm);
		}

		c->incomm[0]    = '\0';
		// queue up c->incomm with another command if appropriate
		if(c->inbuf[0]){
			read_from_buffer( c );
		}
	}
}

/**************************************************************************/
void do_localecho ( char_data *ch, char * argument)
{
	if( IS_NULLSTR(argument) ) // show instructions
    {
        ch->println("`xThis commands turns on and off your local telnet echo.");
        ch->println("Type `=Clocalecho on`x or `=Clocalecho off`x to use it.");
		return;
	}
	
	if(!str_prefix(argument,"on"))
    {
        ch->printf("Sending telnet local echo on sequence\r\n%s",echo_on_str );
        return;
    }
    if(!str_prefix(argument,"off"))
    {
        ch->printf("Sending telnet local echo off sequence\r\n%s",echo_off_str );
        return;
    }
    
}

/**************************************************************************/
void do_relookup( char_data *ch, char *argument)
{
    char_data *victim;
	char arg[MIL];

	one_argument( argument, arg );
	{   
		if( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
		{
            ch->printlnf("There is no '%s' in the game.", arg);
			return;
		}

		if( IS_NPC(victim) )
		{
            ch->println("Not on NPC's.");
			return;    
		}

		if( !victim->desc )
		{
            ch->printlnf("%s appears to be linkdead.", victim->name);
			return;    
		}

	}
    ch->printlnf("Relooking up the host name on %s.", victim->name);
    ch->printlnf("BEFORE LOOKUP> Host: %s  IP: %s  Local: %d  Remote: %d",
		victim->desc->remote_hostname,
		victim->desc->remote_ip,
		victim->desc->local_port,
		victim->desc->remote_port);
	resolver_query( victim->desc);
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
/**************************************************************************/

