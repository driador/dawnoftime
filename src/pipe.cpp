/**************************************************************************/
// pipe.cpp - Functions which pipe to the operating system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
// Local functions.
char *fgetf( char *s, int n, FILE *iop );

extern bool hotreboot_in_progress;
/**************************************************************************/
void do_pipe( char_data *ch, char *argument )
{
    char buf[16000], pbuf[MSL];

    FILE *fp;

	if (IS_NPC(ch)){
		do_huh(ch,"");
        return;
	}

	// Can't use pipe during hotreboot
	if(hotreboot_in_progress){
		ch->println("Hotreboot in progress...  Please wait.");
		return;
	}

    if (!(IS_TRUSTED(ch,MAX_LEVEL) && ch->level>=MAX_LEVEL-1)      
        && !is_exact_name(TRUE_CH(ch)->name,"_impossible_name@#$!"))
    {
		do_huh(ch,"");
        return;
    }

	sprintf(pbuf,"%s", argument);

	ch->printlnf("piping '%s'",pbuf);
    fp = popen( pbuf, "r" );
	if(!fp){
		ch->println("Pipe failed!");
		return;
	}

    fgetf( buf, 15000, fp );
	strcat (buf,"\r\n--=== END OF PIPE ===--\r\n");

    ch->sendpage(buf);

    pclose( fp );

    return;
}

/**************************************************************************/
void do_system( char_data *ch, char *argument )
{
    char buf[5000];

	if (IS_NPC(ch)){
		do_huh(ch,"");
        return;
	}
	
	if (!(IS_TRUSTED(ch,MAX_LEVEL) && ch->level>=MAX_LEVEL-1)      
     && !is_exact_name(TRUE_CH(ch)->name,"_impossible_name@#$!"))
    {
		do_huh(ch,"");
        return;
    }

#ifdef WIN32
	sprintf(buf,"%s", argument);
#else
	sprintf(buf,"%s &", argument);
#endif
	logf("Executing the command '%s' via system() for %s", buf, TRUE_CH(ch)->name);
	int ret=system(buf);
	if(ret<0){
		logf("System return code=%d (error), errno=%d (%s)", ret, errno, strerror(errno));
		ch->printlnf("Performed a system '%s'", buf);		
		ch->printlnf("Return code=%d (error), errno=%d (%s)", ret, errno, strerror(errno));
	}else{
		logf("System return code=%d", ret);
		ch->printlnf("Performed a system '%s'", buf);
		ch->printlnf("Return code=%d", ret);
	}
    return;
}

/**************************************************************************/
char *fgetf( char *s, int n, FILE *iop )
{
    int c;
    char *cs;

    c = '\0';
    cs = s;
    while( --n > 0 && (c = getc(iop)) != EOF){
		if ((*cs++ = c) == '\0'){
			break;
		}
	}
    *cs = '\0';
    return((c == EOF && cs == s) ? (char*)"" : s);
}

/**************************************************************************/
char * get_piperesult( char *cmd )
{
    static char buf[16000];
	char pbuf[MSL];

    FILE *fp;

	sprintf(pbuf,"%s", cmd);

    fp = popen( pbuf, "r" );
	if(!fp){
		return ("get_piperesult failed!");
	}

    fgetf( buf, 15900, fp );
    pclose( fp );
	return (buf);
}

/**************************************************************************/
// displays the dead directory
void do_checkdead( char_data *ch, char *)
{
    char buf[MSL],buf2[MSL];
	BUFFER *output;

    output= new_buf();

#ifdef unix
        sprintf( buf,"ls -lart " DEAD_DIR);
#else
	sprintf( buf,"dir " DEAD_DIR);
#endif
	sprintf( buf2,"\r\n`?%s`x", format_titlebarf("CHECK DEAD - Piping:`x %s", buf));
	add_buf(output,buf2);
	add_buf(output,get_piperesult(buf));

	sprintf( buf2,"`^%s`x", format_titlebar("-"));
	add_buf(output,buf2);
	ch->sendpage(buf_string(output));
	free_buf(output);
}
/**************************************************************************/
// check the logs
void do_checklog( char_data *ch, char *argument)
{
    char buf[MSL],buf2[MSL];
	BUFFER *output;

    output= new_buf();

	if(IS_NULLSTR(argument)){
		#ifdef unix
			sprintf( buf,"ls -lart " PLAYER_LOGS_DIR);
		#else
			sprintf( buf,"dir " PLAYER_LOGS_DIR);
		#endif
		sprintf( buf2,"\r\n`?%s`x", format_titlebarf("CHECKLOG - Piping:`x %s", buf));
		add_buf(output,buf2);
		add_buf(output,get_piperesult(buf));

		sprintf( buf2,"`^%s`x", format_titlebar("-"));
		add_buf(output,"  To read a log: checklog <playername> <number of lines>\r\n");
		add_buf(output,"  <number of lines> - shows up to 40 lines worth.\r\n");
		add_buf(output,buf2);
	}else{
		char * pc;
		char name[MIL], linenums[MIL];
		char logfile_name[MIL];
		char command[MIL];
		// split off the name from the line numbers
		argument = one_argument( argument, name);
		one_argument( argument, linenums);

		// filter the name for non alpha characters
		for ( pc = name; *pc != '\0'; pc++ )
		{
			if ( !is_alpha(*pc) )
			{
				ch->printlnf("CHECKLOG: %s is not a valid player name", name);
				free_buf(output);
				return;
			}
		}		
		// check log file exists
		sprintf(logfile_name, "%s%s.log", PLAYER_LOGS_DIR, name);
		if(!file_exists(logfile_name)){
			ch->printlnf("CHECKLOG: There is no logfile called %s", logfile_name);
			free_buf(output);
			return;
		}

		if(!IS_NULLSTR(linenums) && is_number(linenums)){
			int value = atoi(linenums);	
			if (value<1 || value>30000)
			{
				add_buf(output,"\r\n`RNumber of lines to read of log must be between 1 and 30000.`x\r\n");
				free_buf(output);
				return;
			}
			else
			{
				sprintf(command, "tail -n %d %s | head -n 40", value, logfile_name);
			}	
		}else{
			sprintf(command, "tail -n 20 %s", logfile_name);
			add_buf(output, "\r\n== You can select the number of loglines, type checklog <playername> <number of lines>==\r\n");
		}
		
		sprintf( buf,"`?%s`x", format_titlebar("CHECKLOG"));
		add_buf(output,buf);			
		sprintf( buf,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", command));
		add_buf(output,buf);
		add_buf(output,get_piperesult(command));
	}
	ch->sendpage(buf_string(output));
	free_buf(output);
}
/**************************************************************************/
// displays the wholist from port 1234
void do_who1234( char_data *ch, char *)
{
    char buf[MSL*5],buf2[MSL*5];
	BUFFER *output;

    output= new_buf();

#ifdef unix
	sprintf( buf,"lynx -auth=disable:password -nolog -dump http://127.0.0.1:1239/immwho");
#else
	ch->println("unix only");
	return;	
#endif
	sprintf( buf2,"\r\n`?%s`x", format_titlebar("Remote wholist - port 1234"));
	add_buf(output,buf2);
	add_buf(output,get_piperesult(buf));

	sprintf( buf2,"`^%s`x", format_titlebar("-"));
	add_buf(output,buf2);
	ch->sendpage(buf_string(output));
	free_buf(output);
}
/**************************************************************************/
// displays the wholist from port 4321
void do_who4321( char_data *ch, char *)
{
    char buf[MSL*5],buf2[MSL*5];
	BUFFER *output;

    output= new_buf();

#ifdef unix
	sprintf( buf,"lynx -auth=disable:password -nolog -dump http://127.0.0.1:4326/immwho");
#else
	ch->println("unix only");
	return;	
#endif
	sprintf( buf2,"\r\n`?%s`x", format_titlebar("Remote wholist - port 4321"));
	add_buf(output,buf2);
	add_buf(output,get_piperesult(buf));

	sprintf( buf2,"`^%s`x", format_titlebar("-"));
	add_buf(output,buf2);
	ch->sendpage(buf_string(output));
	free_buf(output);
}
/**************************************************************************/
char *is_valid_scriptname(char *scriptname);
/**************************************************************************/
#define IRCLOGS_DIR (game_settings->irclogs_dir)
/**************************************************************************/
// displays the irclog directory - Kal - Aug 99
void do_checkirc( char_data *ch, char *argument)
{
    char buf[MSL],buf2[MSL];
	BUFFER *output;
	char tbuf[MIL];

    output= new_buf();

	if(IS_NULLSTR(argument)){
#ifdef unix
		sprintf( buf,"ls -lart %s", IRCLOGS_DIR);
#else
		sprintf( buf,"dir %s", IRCLOGS_DIR);
#endif
		sprintf( buf2,"\r\n`?%s`x", format_titlebarf("CHECKIRC - Piping:`x %s", buf));
		add_buf(output,buf2);
		add_buf(output,get_piperesult(buf));

		sprintf( buf2,"`^%s`x", format_titlebar("-"));
		add_buf(output,"  To read a log: checkirc <logname> <number of lines>\r\n");
		add_buf(output,"  Checkirc if it doesn't find an exact match on the filename,\r\n");
		add_buf(output,"  Will automatically attempt combinations of .log and .<date> for you.\r\n");
		add_buf(output,"  <number of lines> - shows up to 40 lines worth.\r\n");
		add_buf(output,buf2);
	}else{
		char name[MIL], linenums[MIL];
		char logfile_name[MIL];
		char command[MIL];
		// split off the name from the line numbers
		argument = first_arg(argument, name, false);
		argument = first_arg(argument, linenums, false);

		// filter the name for non alpha characters
		if(is_valid_scriptname(name)){
			ch->printlnf("CHECKIRC: %s is not a valid log filename", name);
			free_buf(output);
			return;
		}

		{	// get the time buffering
			struct tm *today;
			today=localtime( &current_time);
			strftime(tbuf, MIL, "%d%b", today);
			char t2buf[10]; // budget fix to get rid of y2k compiler warning
			strftime(t2buf, 10, "%Y", today);
			strcat(tbuf,&t2buf[0]);
			log_string(tbuf);

		}

		// try 4 different formattings to match the filename
		for(int count=0; count<4; count++)
		{
			switch(count){
				case 0: // straight
					// check log file exists
					sprintf(logfile_name, "%s%s", IRCLOGS_DIR, name);
					break;
				case 1: // with .<date>
					// check log file exists
					sprintf(logfile_name, "%s%s.%s", IRCLOGS_DIR, name, tbuf);
					break;
				case 2: // with .log
					// check log file exists
					sprintf(logfile_name, "%s%s.log", IRCLOGS_DIR, name);
					break;
				case 3: // with .log.<date>
					// check log file exists
					sprintf(logfile_name, "%s%s.log.%s", IRCLOGS_DIR, name, tbuf);
					break;

			}

			if(file_exists(logfile_name)){
				break;
			}
			
		}

		if(!file_exists(logfile_name)){
			ch->printlnf("CHECKLOG: There is no logfile called %s, nor without the added extensions.", logfile_name);
			free_buf(output);
			return;
		}

		if(!IS_NULLSTR(linenums) && is_number(linenums)){
			int value = atoi(linenums);	
			if (value<1 || value>30000)
			{
				add_buf(output,"\r\n`RNumber of lines to read of log must be between 1 and 30000.`x\r\n");
				free_buf(output);
				return;
			}
			else
			{
				sprintf(command, "tail -n %d %s | head -n 40", value, logfile_name);
			}	
		}else{
			sprintf(command, "tail -n 20 %s", logfile_name);
			add_buf(output, "\r\n== You can select the number of loglines, type checklog <playername> <number of lines>==");
		}
		
		sprintf( buf,"`?%s`x", format_titlebar("CHECKIRC"));
		add_buf(output,buf);			
		sprintf( buf,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", command));
		add_buf(output,buf);
		add_buf(output,get_piperesult(command));
	}
	ch->sendpage(buf_string(output));
	free_buf(output);
}
/**************************************************************************/
// display the bug listing - Daos, Sep 01
void do_checkbug( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];
	
    BUFFER *output=new_buf();
	
	sprintf( buf,"`?%s`x", format_titlebar("`#`WCODE-BUG LOGFILE`^"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1)){
		strcpy(buf, "tail -n 10 " BUG_FILE );
		add_buf(output, "\r\n    You can select the number of loglines, type `#`Ycheckbug <number of lines>`^");		
	}else if (is_number ( arg1 )){
		int value = atoi(arg1); 
		if (value<1 || value>20000){
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			strcpy(buf, "tail -n 10 " BUG_FILE );
		}else{
			sprintf(buf, "tail -n %d " BUG_FILE " | head -n 40", value);
		} 
	}else{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the ooc " 
			"log you wish to see.`x\r\n");
		strcpy(buf, "tail -n 10 " BUG_FILE );
	}
	
	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);
	
	add_buf(output,get_piperesult(buf));
	
	ch->sendpage(buf_string(output));
	free_buf(output);
	
	return;
}
/**************************************************************************/
/**************************************************************************/
 
