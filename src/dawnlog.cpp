/**************************************************************************/
// dawnlog.cpp - dawn logging implementation
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#ifdef unix
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <signal.h>
#endif
#ifdef WIN32
	#include <process.h>
#endif

/**************************************************************************/
extern int hotreboot_ipc_pipe;
/**************************************************************************/
char initial_startup_log_buffer[4*MSL+1];
bool initial_startup_log_buffer_full=false;
bool log_hold_log_string_core_stdout_restore_value;
bool log_string_core_stdout_enabled=true;

#ifdef WIN32
bool commandlineoption_no_background_process=true; // --nobackground commandline switches this
bool commandlineoption_log_console=true;
#else
bool commandlineoption_no_background_process=false; // --nobackground commandline switches this
bool commandlineoption_log_console=false;
#endif
bool commandlineoption_no_logfile=false;

/**************************************************************************/
// returns the time in the format 2 digit year, month, day (6 digits total)
char *dawnlog_format_year_month_day(time_t time_to_format)
{
	static char result[MSL];
	char buf[MSL];
	// year month day
	strftime(buf, MSL, "%Y%m%d", localtime( &time_to_format));
	// %Y is used instead of %y to avoid y2k compiler warnings
	strcpy(result, &buf[2]); // skip the leading 2 digits of the year
	return result;
}

/**************************************************************************/
// returns the start of the next day
time_t dawnlog_get_tomorrow_start()
{
	// find out our time currently
	struct tm now;
	time_t tomorrow_start;
	now= *localtime( &current_time);

	now.tm_sec=0;	// seconds 
	now.tm_min=0;	// minutes 
	now.tm_hour=0;	// hours 
	now.tm_mday++;	// day of the month - jumping to tomorrow
	//now.tm_mon;	// month 
    //now.tm_year;	// year 
    //now.tm_wday;	// day of the week 
    //now.tm_yday;	// day in the year
    //now.tm_isdst;	// daylight saving time

	tomorrow_start=mktime(&now);

//	// Testing Code
//	logf("Tomorrow is %s", ctime(&tomorrow_start));
//	logf("Tomorrow formatted is %s",	dawnlog_format_year_month_day(tomorrow_start));
//	logf("Now is %s", ctime(&current_time));
//	logf("Now formatted is %s",	dawnlog_format_year_month_day(current_time));

	return tomorrow_start;

}
/**************************************************************************/
time_t last_current_time=0;
FILE *current_logfile_descriptor=NULL;
char current_logfile_name[MSL];
time_t rotate_logfilename_after=0;
/**************************************************************************/
FILE *dawnlog_get_new_logfile(int portprefix, char *logfilename)
{
	// get a date code for today
	FILE *newfile;
	char today[MSL];
	char possible_filename[MSL];
	strcpy(today, dawnlog_format_year_month_day(current_time));

	int i;
	for(i=1; i<4000; i++){
		sprintf(possible_filename, "%s%d-%s-%02d.log", 
			GAME_LOGS_DIR,
			portprefix, 
			today, 
			i);
		if(!file_exists(possible_filename)){
			break;
		};
	}
	if(i==4000){
		fprintf(stderr, "Failed to find a free filename for logging in "
			"the format %s after 4000 attempts! - aborting!\n", possible_filename);
		fflush(stderr);
		fprintf(stdout, "Failed to find a free filename for logging in "
			"the format %s after 4000 attempts! - aborting!\n", possible_filename);
		fflush(stdout);		
		do_abort();
	}
	newfile=fopen(possible_filename, "w");
	if(!newfile){
		char buf[MSL];
		sprintf(buf, "dawnlog_get_new_logfile(): fopen '%s' failed for write - error %d (%s) - aborting!\n",
				possible_filename, errno, strerror( errno));
		fprintf(stderr, "%s", buf);
		fflush(stderr);
		fprintf(stdout, "%s", buf);
		fflush(stdout);
		do_abort();
	}
	strcpy(logfilename, possible_filename);

	return newfile;

}
/**************************************************************************/
// all logging to stdout goes thru here, and should only be called
// from within log_string_core
static void log_string_core_stdout(const char *str )
{
	if(log_string_core_stdout_enabled){
		fprintf( stdout, "%s", str); // in log_string_core_stdout
	}
}

/**************************************************************************/
void log_hold_till_commandline_options_parsed()
{
	log_hold_log_string_core_stdout_restore_value=log_string_core_stdout_enabled;
	log_string_core_stdout_enabled=false;

}
/**************************************************************************/
void log_release_held_logs()
{
	log_string_core_stdout_enabled=log_hold_log_string_core_stdout_restore_value;
	log_string_core_stdout(initial_startup_log_buffer);
}
/**************************************************************************/
void dawnlog_write_index(char *str)
{
	append_string_to_file(
		DAWNLOG_INDEX_FILE, 
		FORMATF("[%d] %s :: %s", 
			getpid(), 
			shortdate(NULL),
			str),
		true);
}

/**************************************************************************/
void display_host_info();
void netio_binded_listen_on_output();
extern char **main_argv;
/**************************************************************************/
// all logging should ultimately come thru here
void log_string_core(const char *str )
{
	if(!mainport){
		log_string_core_stdout(str);
		// until we have the main port, we don't try and allocate a logging file
		// but we do record the log information to initial_startup_log_buffer
		if(!initial_startup_log_buffer_full
			&& str_len(initial_startup_log_buffer)+str_len(str)<4*MSL)
		{
			strcat(initial_startup_log_buffer, str);
		}else{
			initial_startup_log_buffer_full=true;
			logf("initial_startup_log_buffer exceeded, some logging information will be lost.");
		}
		return;
	}

	if(commandlineoption_no_logfile){
		log_string_core_stdout(str); 
		return;
	}

	// rotate our logfiles daily
	if(current_logfile_descriptor && rotate_logfilename_after<current_time){
		FILE *new_logfile_descriptor;
		char new_logfile_name[MSL];

		// start by calculating the start of tomorrow in time_t format
		// - this is done first so we can continue to use the log functions
		//   without recursively executing this particular subsection of 
		//   log_string_core()
		rotate_logfilename_after=dawnlog_get_tomorrow_start();

		new_logfile_descriptor=dawnlog_get_new_logfile(mainport, new_logfile_name);

		logf("transferring logging from %s", current_logfile_name);
		logf("information about the current process in these logs:");
		display_host_info();
		netio_binded_listen_on_output();
		logf("logging continues in %s", new_logfile_name);

		fclose(current_logfile_descriptor);
		current_logfile_descriptor=new_logfile_descriptor;
		new_logfile_descriptor=NULL;

		logf("logging continuation of %s", current_logfile_name);
		logf("information about the current process:");
		display_host_info();
		netio_binded_listen_on_output();		

		strcpy(current_logfile_name, new_logfile_name);
		new_logfile_name[0]='\0';
	}

	if(!current_logfile_descriptor){
		// the mud has just booted up, and mainport has just come live
		int portprefix=mainport;
		mainport=0;
		log_string("initiating logging to file");

		// open the log file
		current_logfile_descriptor=dawnlog_get_new_logfile(portprefix, current_logfile_name);
		
		// calculate the time we will want to write to the log file we just opened until
		rotate_logfilename_after=dawnlog_get_tomorrow_start(); 
		
		mainport=portprefix;
		if(current_logfile_descriptor){
			log_string_core(initial_startup_log_buffer);
			initial_startup_log_buffer[0]='\0';
			if(initial_startup_log_buffer_full){
				logf("initial_startup_log_buffer exceeded, some logging information will be lost.");
				initial_startup_log_buffer_full=false;
			}
			logf("logging to %s", current_logfile_name);

			dawnlog_write_index("============================================");
			dawnlog_write_index(FORMATF("dawn logging to %s",current_logfile_name));
	
			log_notef("note: on machines which have the tail command installed, "
				"you can follow this log using the command:  tail -n 5000 -f %s", 
				current_logfile_name);

			log_notef("note2: you can change the startup behaviour if desired, to do so"
				"`1  type \"%s --help\" for more information.", main_argv[0]);

#ifdef unix
			if(commandlineoption_no_background_process){
				logf("Mud remaining in foreground, process id = %d.", getpid());
			}else{
				// switch to become a daemon
				logf("switching to daemon mode (background running process)");
				pid_t fork_result=fork();
				// check if something went wrong
				if(fork_result<0){ 
					bugf("switching to background daemon failed! errno=%d (%s)",
						errno, strerror(errno));
				}else if(fork_result){ 
					// parent... close it off, so they can continue on with the shell
					sleep_seconds(1);
					exit_clean(0, "log_string_core", "parent process closing down after going background");
				}
				if(setsid() < 0){
					// shoudn't fail
					bugf("failed to set new session id.");
				}
					
				logf("The mud is now running in the background, process id = %d.", getpid());
			}
#endif

			if(!commandlineoption_log_console){
				log_string_core_stdout_enabled=false;
			}
		}
	}

	log_string_core_stdout(str); 
	
	if(current_logfile_descriptor){
		int slen=str_len(str);		
		if(slen>0 && fwrite(str, slen, 1, current_logfile_descriptor)!=1){
			fprintf(stderr, "error %d (%s) writing text to %s\n", 
				errno, 
				strerror( errno),
				current_logfile_name);
			fprintf(stderr, "'%s'", str);
			fflush(stderr);
		}

	}
}
/**************************************************************************/
void log_string_flush()
{
	fflush(stdout);
	if(current_logfile_descriptor){
		fflush(current_logfile_descriptor);
	}
}
/**************************************************************************/
// Writes a string to the log.
void log_string( const char *str )
{
	if(runlevel==RUNLEVEL_INIT){
		return;
	}

	char ipc_num[20];
	if(hotreboot_ipc_pipe>=0){
		sprintf(ipc_num, "%d", hotreboot_ipc_pipe);
	}else{
		ipc_num[0]=0;
	}

	if(game_settings && GAMESETTING2(GAMESET2_VERBOSE_DATES_IN_LOGS)){
		last_current_time=0;
	}

	if(last_current_time==current_time){		
		log_string_core( FORMATF(":%s: ", ipc_num));
	}else{
		log_string_core( FORMATF("[%d]%s:%s: ", getpid(), shortdate(NULL)+4, ipc_num));
	}
	log_string_core( str ); // this is not included in the FORMATF above to 
							// avoid having to worry about the size of buffers
	log_string_core( "\n" );
	last_current_time=current_time;
	log_string_flush();
    return;
}
/**************************************************************************/
void fulltime_log_string( const char *str )
{
	if(runlevel==RUNLEVEL_INIT){
		return;
	}
	log_string_core( FORMATF("%s :: ", shortdate(NULL)));
	log_string_core( str );
	log_string_core( "\n" );
	log_string_flush();
    return;
}

/**************************************************************************/
// draw a line of *'s in the logs on a line without the time
void log_bar( )
{
	last_current_time=current_time;
	log_string("**************************************************************************");
}
/**************************************************************************/
// put a note in the log
void log_note( const char *text) 
{
	char buf[MSL*4];
	strcpy(buf, "*************************************************************************\n");

	char *msg=str_dup(text);
	msg=note_format_string_width(msg, 77, false, true);
	msg=string_replace_all(msg,"`1","\n");  
	strcat(buf, msg);
	free_string(msg);

	strcat(buf, "****************************************************************************");
	last_current_time=current_time;
	log_string(buf);
}
/**************************************************************************/
void log_notef(const char * fmt, ...)
{
    char buf[MSL*4];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL*4-MIL, fmt, args);
	va_end(args);

	log_note(buf);
}

/**************************************************************************/


/**************************************************************************/
/**************************************************************************/

