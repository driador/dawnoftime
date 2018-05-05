/**************************************************************************/
// mud2web.cpp - web interface code for mud commands.
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
#include "olc.h"
#include "interp.h"
#include "help.h"

void flush_cached_write_to_buffer(connection_data *d);
bool processWeb(connection_data *c);

/**************************************************************************/
char *doWebFunction(DO_FUN * do_fun, char *argument, int level)
{
	static char rBuf[(MSL*4)+200];

	chImmortal->level=level;
	chImmortal->print("`x");
	chImmortal->remort=game_settings->webserver_default_remort;
	(*do_fun) ( chImmortal, argument);
	flush_cached_write_to_buffer(chImmortal->desc); // flush cache 
	strncpy(rBuf, chImmortal->desc->outbuf,MSL*4);
	rBuf[chImmortal->desc->outtop]='\0';
	rBuf[MSL*4]='\0';

	// put the time on
	strcat(rBuf,"\n\n");
	strcat(rBuf, (char *) ctime( &current_time ));
	rBuf[str_len(rBuf)-1]='\0';
	
	chImmortal->desc->outtop = 0;

	return(rBuf);
}
/**************************************************************************/
// returns the arealist for the level 
char *getwebAreas(int level){
	return(doWebFunction(&do_areas, "", level));
}
/**************************************************************************/
// returns the wholist for the level 
char *getwebWho(int level){
	return(doWebFunction(&do_who, "", level));
}
/**************************************************************************/
// returns the mudstats 
char *getwebMudstats(int level){
	return(doWebFunction(&do_mudstats, "", level));
}
/**************************************************************************/
// returns the wholist for the level 
char *getwebWizlist(){
	return(doWebFunction(&do_wizlist, "", 20));
}
/**************************************************************************/
// returns the class usage stats
char *getwebClassStats( ){
	return(doWebFunction(&do_classstats, "", 20));
}
/**************************************************************************/
// returns the race usage stats
char *getwebRaceStats( ){
	return(doWebFunction(&do_racestats, "", 20));
}
/**************************************************************************/
// returns the race info table
char *getwebRaceInfo( ){
	return(doWebFunction(&do_raceinfo, "", 20));
}
/**************************************************************************/
// returns the class info table
char *getwebClassInfo(char *racename){
	return(doWebFunction(&do_classinfo, racename, 20));
}
/**************************************************************************/
// returns the class usage stats
char *getwebAlignStats( ){
	return(doWebFunction(&do_alignstats, "", 20));
}
/**************************************************************************/
// returns the game settings
char *getwebGameSettings( ){
	return(doWebFunction(&do_gamesettings, "", 20));
}
/**************************************************************************/
char *getwebSockets(int level){
	if(level<MAX_LEVEL-4){
		return "Insufficient level/trust to view sockets";
	}
	return(doWebFunction(&do_sockets, "", level));
}

/**************************************************************************/
// perform TEXT to URL changes (a bit hackish, but it works) :)
char *TEXT2URL(char *arg){
	static char buf[MSL];
	
	char *rep=str_dup(arg);
	// convert + into %2B
	rep=string_replace_all( rep, "+", "%2B");	
	// convert space into +
	rep=string_replace_all( rep, " ", "+");	

	strcpy(buf, rep);
	free_string(rep);
	return buf;
}

/**************************************************************************/
// perform URL to TEXT changes (a bit hackish, but it works) :)
char *URL2TEXT(char *arg){
	static char buf[MSL];
	
	char *rep=str_dup(arg);
	// convert space into +
	rep=string_replace_all( rep, "+", " ");	
	// convert + into %2B
	rep=string_replace_all( rep, "%2B", "+");	

	strcpy(buf, rep);
	free_string(rep);
	return buf;
}


/**************************************************************************/
// strcat, at the same time as adding weblinks to {=_ codes
void websvr_make_links_and_strcat(char *dest, char *src)
{
	char keyword[MSL], url[MSL];
	char *str, *target, *originalstr=NULL;

	// point our target at the end of the destination string for a cat
	target=&dest[str_len(dest)];

	for(str=src;!IS_NULLSTR(str); str++){
		if(*str=='`' && *(str+1)=='=' && *(str+2)=='_'){ // got a weblink
			originalstr=str;
			str+=3;
			str=help_find_keyword(str, keyword, chImmortal);

			if(!help_get_by_keyword(keyword, chImmortal, false)){
				str=originalstr;			
				*target++=*str;
				continue; // couldnt find it as a help
			}			
			
			sprintf(url,"<A HREF=\"/help/%s\">%.20s</A>", TEXT2URL(lowercase(keyword)), lowercase(keyword));
			*target='\0';
			strcat(target,url);
			target+=str_len(url);	

		}else{
			*target++=*str;			
		}
	}
	*target='\0';
}
/**************************************************************************/
char *getwebHelp(char *argument, char *entry_buf, int length){
	unsigned char c;
	entry_buf[0]='\0';
	if(str_len(argument)>(length-10)){
		return "`XgetwebHelp(): help url request too long!";
	}
	logf("getwebHelp(%s)", argument);

	// we pass chImmortal to is_valid_help, specify the amount of 
	// access chImmortal has to help entries
	chImmortal->level=20;			 // a mortal
	chImmortal->pcdata->security=5;  // builder
	chImmortal->pcdata->diplomacy=0; // that isn't a noble
	REMOVE_CONFIG( chImmortal, CONFIG_RP_SUPPORT); // not rpsupport
	REMOVE_BIT(chImmortal->comm,COMM_NEWBIE_SUPPORT); // not newbie support

    help_data *pHelp;
	static char buf[HSL];
	char arg[MSL];
	int count=0;
	char tempbuf[MIL];

    int number=1;
	strcpy(buf,"`x");
	number=number_argument( argument, arg );

	if(IS_NULLSTR(arg)){
		return(getwebHelp("Webhelp Overview", entry_buf, length));
	}

	if(arg[1]=='\0'){ // single letter list all of that letter
		count=0;

		sprintf(entry_buf,"Helps starting with the character '%c'", arg[0]);
		// prefix the list of helps
		strcat(buf,"`S[<A HREF=\"/\">/</A>][<A HREF=\"/help/-\">-</A>]");
		for(c='a'; c!=('z'+1); c=c+1){					
			sprintf(tempbuf,"[<A HREF=\"/help/%c\">%c</A>]",
				(char)c, (char)c);
			strcat(buf,tempbuf);
		}
		strcat(buf,"`1 ");

		for(pHelp=help_first; pHelp; pHelp=pHelp->next){
			if(pHelp->level>1){ // mort help only
				continue;
			}
			if( IS_SET( pHelp->flags, HELP_NSUPPORT | HELP_NOBLE | HELP_BUILDER | HELP_RPSUPPORT)){
				continue;
			}

			if(is_name(arg, pHelp->keyword)){
				char keyword[MIL];
				char *first;		
				// find the matching keyword
				for(first=pHelp->keyword; !IS_NULLSTR(first); ){
					first=one_argument(first,keyword);
					if(LOWER(arg[0])==LOWER(keyword[0]) || (keyword[0]=='-' && LOWER(arg[0])==LOWER(keyword[1]))){
						sprintf(tempbuf,"<A HREF=\"/help/%s\">%.20s</A >",
							TEXT2URL(keyword), keyword);
						strcat(buf,tempbuf);
						if(++count>3){
							count=0;
							strcat(buf,"`1 ");
						}else{
							// pad it so the columns line up 
							int i=20-str_len(keyword);
							for( ;i>0;i--){
								strcat(buf," ");
							}
							strcat(buf," ");
						}
					}
				}
			}
		}
		// append the list of helps
		strcat(buf,"`1`S[<A HREF=\"/\">/</A>][<A HREF=\"/help/-\">-</A>]");
		for(c='a'; c!=('z'+1); c=c+1){					
			sprintf(tempbuf,"[<A HREF=\"/help/%c\">%c</A>]",
				(char)c, (char)c);
			strcat(buf,tempbuf);
		}
		return buf;
	}

	// perform HTML URL to TEXT changes (a bit hackish, but it works) :)
	strcpy(arg,URL2TEXT(arg));
	strcpy(entry_buf,arg);

	for(pHelp=help_first; pHelp; pHelp=pHelp->next){
		if(pHelp->level>1){ // mort help only
			continue;
		}
		if( IS_SET( pHelp->flags, HELP_NSUPPORT | HELP_NOBLE | HELP_BUILDER | HELP_RPSUPPORT)){
			continue;
		}

		if(is_name(arg, pHelp->keyword)){
            if(++count == number)
            {
				// prefix the list of helps
				strcat(buf,"`S[<A HREF=\"/\">/</A>][<A HREF=\"/help/-\">-</A>]");
				for(c='a'; c!=('z'+1); c=c+1){					
					sprintf(tempbuf,"[<A HREF=\"/help/%c\">%c</A>]",
						(char)c, (char)c);
					strcat(buf,tempbuf);
				}
				strcat(buf,"`1");

                // Strip leading '.' to allow initial blanks.
				char *helptext=help_generate_help_entry_for_char(pHelp, chImmortal);

				websvr_make_links_and_strcat(buf, helptext);

				// append the list of helps
				strcat(buf,"`S`1");
				strcat(buf,"[<A HREF=\"/\">/</A>][<A HREF=\"/help/-\">-</A>]");
				for(c='a'; c!=('z'+1); c=c+1){					
					sprintf(tempbuf,"[<A HREF=\"/help/%c\">%c</A>]",
						(char)c, (char)c);
					strcat(buf,tempbuf);
				}
				return buf;
            }
        }
    }

	if(!str_cmp(arg,"Webhelp Overview")){
		strcpy(buf,
			"`XClick on the / to go back to the other options.`1"
			"`XClick on the - to see a list of helps starting with the - character.`1"
			"Otherwise click a letter to see all helps starting with that letter.`1"
			"`1");

		// append the list of helps
		strcat(buf,"`S[<A HREF=\"/\">/</A>][<A HREF=\"/help/-\">-</A>]");
		for(c='a'; c!=('z'+1); c=c+1){					
			sprintf(tempbuf,"[<A HREF=\"/help/%c\">%c</A>]",
				(char)c, (char)c);
			strcat(buf,tempbuf);
		}
		return buf;
	}
	sprintf(buf, "`XSorry, no help on the keyword '%s' was found.", argument);
	return(buf);
}
/**************************************************************************/
// returns the mudstats 
char *getwebLastonImm(int level){
	return(doWebFunction(&do_laston, "-i", level));
}
/**************************************************************************/
extern int webport;
/************************************************************************/
void logWebf(char * fmt, ...)
{
    char buf[HSL];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);
	
	log_string(buf);
}
/************************************************************************/
/************************************************************************/
