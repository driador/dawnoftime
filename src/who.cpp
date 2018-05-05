/**************************************************************************/
// who.cpp - The who command and related code
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

#include "include.h" // dawn standard includes

/**************************************************************************/
// prototypes for the whoformat functions in whofmt.cpp
char *whoformat_dawn( char_data *ch, char_data *wch, bool two_column);
char *whoformat_storm( char_data *ch, char_data *wch, bool two_column);
char *whoformat_just_names( char_data *ch, char_data *wch, bool two_column);
char *whoformat_endless( char_data *ch, char_data *wch, bool two_column);
char *whoformat_wotl( char_data *ch, char_data *wch, bool two_column);

/**************************************************************************/
struct	who_format_type who_formatter [] =
{
	// name,			function ran,			use two columns when a lot of players are on
	{ "default",		NULL,					false		}, // spacer - if you select default you get the mud whoformat setting
	{ "dawn",			whoformat_dawn,			true		}, 
	{ "storm",			whoformat_storm,		false		}, 
	{ "endless",		whoformat_endless,		false		}, 
	{ "just_names",		whoformat_just_names,	false		}, 
	{ "wotl",			whoformat_wotl,			false		}, 
	{ "", NULL, false}
};
/**************************************************************************/
void who_show_formats(char_data *ch)
{
	int i;
	for(i=0; !IS_NULLSTR(who_formatter[i].name); i++){
		ch->printlnf("   %s", who_formatter[i].name);
	}
}

/**************************************************************************/
int who_format_lookup(char *argument)
{

	int i;
	// do an exact lookup on a who format name first
	for ( i= 0; !IS_NULLSTR(who_formatter[i].name); i++){
		if(!str_cmp( argument, who_formatter[i].name))
			return i;
	}

	// do an prefix lookup on a who format name
	for ( i= 0; !IS_NULLSTR(who_formatter[i].name); i++){
		if(!str_prefix( argument, who_formatter[i].name))
			return i;
	}

	return -1;
}
/**************************************************************************/
// return the format 
int who_get_format(char_data *ch)
{
	if(ch->pcdata && ch->pcdata->who_format>0){
		return ch->pcdata->who_format;
	}
	return game_settings->default_who_format_index;
}
/**************************************************************************/
const char *who_format_name(int index)
{
	return who_formatter[index].name;
}

/**************************************************************************/
// used to pick which format of the wholist you want to see
void do_whoformat( char_data *ch, char *argument )
{
    // unswitched mobs can't use the wholist
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("players only sorry");
        return;
    }

	if(IS_NULLSTR(argument)){
		ch->println("syntax: whoformat <format>");
		ch->println("Where format is one of the following:");
		who_show_formats(ch);
		ch->printlnf("Your who format is currently set to %s.", who_format_name(TRUE_CH(ch)->pcdata->who_format));

		if(IS_ADMIN(ch)){
			ch->println("adminnote: The mudwide default format can be changed in gameedit with setwhoformat.");
		}
		return;
	}

	int format=who_format_lookup(argument);
	if(format<0){
		ch->printlnf("'%s' is not an available who format.", argument);
		do_whoformat(ch, "");
		return;
	}

	if(TRUE_CH(ch)->pcdata->who_format==format){
		ch->printlnf("Your who format is already set to %s.", who_formatter[format].name);
		return;
	}

	ch->printlnf("Who format changed from %s to %s.",
		who_formatter[TRUE_CH(ch)->pcdata->who_format].name,
		who_formatter[format].name);

	TRUE_CH(ch)->pcdata->who_format=format;
}

/**************************************************************************/
void do_whois (char_data *ch, char *)
{

	ch->println("`xThe whois command has been replaced by a searchable wholist.");
	ch->printlnf( "e.g. for example if you wanted to see all names starting\r\n"
		"with K on the wholist, type `=Cwho k`x ... you can be as specific\r\n"
		"as you like, eg `=Cwho ka`x will show all names starting with ka." );
	return;
		  
}

/***********************************************************************/
void do_count( char_data *ch, char *)
{
    int count, immhide_count, all_count, displayed_count;
    char_data *cch;

    count             = 0;
    immhide_count     = 0;
    all_count         = 0;
    displayed_count   = 0;

	for ( cch= player_list; cch; cch= cch->next_player)
    {
        all_count++;  // count the playing people 
        if ( can_see_who(ch, cch) )
        {
            count++;  // visible people for this char 
        }
        else if (IS_IMMORTAL(cch))
        { 
            immhide_count++; // hiding imm count 
        }
    }

    max_on = UMAX(all_count,max_on);

	if (true_count <= all_count)
	{
		true_count = all_count;
		maxon_time = current_time;
	}

	if (hotrebootmaxon<=all_count)
	{
		hotrebootmaxon= all_count;
		hotrebootmaxon_time = current_time;
	}

    displayed_count = all_count-immhide_count;
 
	if (max_on == displayed_count){
        ch->printlnf("`xThere %s %d character%s on - The most since the last reboot.\r\n",
            (displayed_count==1)?"is":"are",
            displayed_count,
            (displayed_count==1)?"":"s");
    }else{
        ch->printlnf("`xThere %s %d character%s on.\r\n"
                    "The most characters on since the last reboot has been %d.\r\n",
            (count==1)?"is":"are",
            count,
            (count==1)?"":"s",
            max_on);
	}

}

/***********************************************************************/
char *get_whocount(char_data *ch, int displayed_total, bool noimm4morts)
{
    int immhide_count, all_count, displayed_count;
	static char rBuf[MSL];
	char countbuf[MSL];
    char_data *cch;

    immhide_count     = 0;
    all_count         = 0;
    displayed_count   = 0;

    for ( cch = player_list; cch; cch = cch->next_player )
    {
		all_count++;  // count the playing people

		if ((noimm4morts && cch->level>=LEVEL_IMMORTAL 
			&& !IS_IMMORTAL(ch))||(!IS_SET(cch->comm, COMM_WHOVIS)
				&& IS_IMMORTAL(cch) && !can_see_who(ch, cch)))
		{ 
			immhide_count++; // hiding imm count
		}
    }

    max_on = UMAX(all_count,max_on);

	if (true_count <= all_count)
	{
		true_count = all_count;
		maxon_time = current_time;
	}

	if (hotrebootmaxon<=all_count)
	{
		hotrebootmaxon= all_count;
		hotrebootmaxon_time = current_time;
	}

    displayed_count = all_count-immhide_count;
 
    sprintf(countbuf, "%d", displayed_total);
    if (max_on == displayed_count){
        sprintf(rBuf,"`xThere %s %d character%s on - The most since the last reboot.\r\n"
                    "%d of these characters are visible to you.\r\n",
            (displayed_count==1)?"is":"are",
            displayed_count,
            (displayed_count==1)?"":"s",
            displayed_total);
	}else{
		if(displayed_count==1){
			sprintf(rBuf,"`xThere is 1 character on you can see (yourself).\r\n"
						"The most characters on since the last reboot has been %d.\r\n",
				max_on);
		}else{
			sprintf(rBuf,"`xThere are %d character%s on, of which %s %s visible to you.\r\n"
						"The most characters on since the last reboot has been %d.\r\n",
				displayed_count,
				(displayed_count==1)?"":"s",
				displayed_total==displayed_count?"all":countbuf,
				(displayed_total==1)?"is":"are",
				max_on);
		}
	}

	return (rBuf);
}

/**************************************************************************/
void show_whocount( char_data *ch, int displayed_total, bool noimm4morts)
{
	ch->printf( "%s", get_whocount( ch, displayed_total, noimm4morts) );
}

/**************************************************************************/
// redirects who to the correct function
char *who_format_redirector(char_data *ch, char_data *wch, bool two_columns)
{
	int who_format=who_get_format(ch);

	// calculate how long a player has been idle - used for logging and displaying
	int idletime;
	if(wch->desc){
		idletime=(int)(current_time - wch->desc->idle_since)/60;
	}else{
		idletime=wch->idle;
	}

	// if a player is logged, log all idle, afk and linkdead 
	// people they see on the wholist so we can tell if 
	// they knew the person was in one of those states when 
	// starting a pkill figure
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG)){
		char logbuf[MIL];logbuf[0]='\0';
		if(IS_SET(wch->comm, COMM_AFK)){
			strcat(logbuf," [AFK]");
		}
		if(IS_LINKDEAD(wch)){
			strcat(logbuf," [LD]");
		}
		if( idletime >= 1 ){
			strcat(logbuf,FORMATF(" %dm idle", idletime));
		}
		if(!IS_NULLSTR(logbuf)){			
			append_playerlog( ch, FORMATF("wholog:%-13s %s", wch->name, logbuf));
		}
	}

	char * who_result=who_formatter[who_format].whofunc(ch, wch, two_columns);

	char idle[MIL];
    if ( idletime >= 1 ){
		if(GAMESETTING2(GAMESET2_NO_AUTOLOGOFF_FOR_IMM)){
			if(idletime>120){
				sprintf( idle, "%2d:%02d", idletime/60, idletime%60);
			}else{
				sprintf( idle, "%4dm", idletime);
			}
		}else{
			sprintf( idle, "%3dm", idletime);
		}
    }else{
		if(GAMESETTING2(GAMESET2_NO_AUTOLOGOFF_FOR_IMM)){
	        strcpy( idle, "     " );
		}else{
	        strcpy( idle, "    " );
		}
	}

	return FORMATF("%s%s", idle, who_result);
}

/**************************************************************************/
void do_who( char_data *ch, char *argument )
{
	// You should NOT replace this function to implement your own wholist!
	// You SHOULD write your own who format function and customise the 
	// 'who header' defines shown directly after this documentation.  You 
	// can then use the gameedit flags to change how the wholist behaves. 
	// Look in whofmt.cpp for a number of example formatting functions etc.
	// The advantages of this approach is the various methods of sorting 
	// the wholist are already done for you, extra features such as idle
	// idle time are also correctly displayed.  In addition the code 
	// correctly supports in game commands such as whoinvis, whovis etc.
	// and gives the players potentially the option to pick an alternative 
	// format from a selection (assuming the whoformat command is enabled).

	// The do_who() function creates a buffer to store the entire wholist
	// then loops thru the player_list a number of times/ways to get create 
	// a list of connected players in the format as per the gamesetting flags
	// described below.

	// The player_list is looped using the variable wch, and the player who
	// requested the wholist is refered to as 'ch' within this function.
	// The line (or lines) in the wholist to display each wch, is formatted
	// according to what ch's has selected their who format function to be.
	// these functions are listed at the top of whofmt.cpp.
	// the directing to the correct who formatter is handled by the 
	// who_format_redirector() function.  The idle time of each wch is 
	// prefixed to what who_format_redirector() returns.

	// some of the gameedit flags relating to the wholist:
	//  GAMESET2_WHOLIST_SORT_BY_LEVEL - list is level sorted
	//  GAMESET2_WHOLIST_IMMS_BEFORE_MORTS - immortals are at the top of the list
	//  GAMESET2_NO_AUTOLOGOFF_FOR_IMM - this stops immortals being logged off
	//           if this is set, the idle times for imms can get very large
	//           therefore an extra space is inserted where the idle time is
	//  GAMESET2_DONT_DISPLAY_WHO_4_LOGIN - the 'autowho at login' isn't run as part
	//           of the login process (useful if your who format is crashing the mud)
	//  GAMESET4_LOGINWHO_HIDES_IMMS_FROM_MORTS - the 'autowho at login' doesn't 
	//           display immortals to mortal players
	//  GAMESET5_DEDICATED_PKILL_STYLE_MUD - when the mud is flagged in pkill 
	//           mode the pkill header is shown at the top of the wholist
	//  GAMESET3_WHO_TITLE_DISABLED - some of the custom formats support 
	//           disabling the display of who titles
	//  GAMESET4_3TIER_IMMRANKS_IN_WHO - an alternative immortal level system 
	//           where there is only 3 categories - imp, admin and immortal

#define WHOHEADER_MORTALS "`=\x81-========= `=\x96MORTALS `=\x81=========-`x\r\n"
#define WHOHEADER_IMMORTALS "`=\x82-======== `=\x83IMMORTALS `=\x82========-`x\r\n"
#define WHOHEADER_PKILL_PORT "`r-`R=`r=`R=`r=`R=`r=`R=`r=`R=`r=`R=`r= `R P K I L L   P O R T  `r=`R=`r=`R=`r=`R=`r=`R=`r=`R=`r-`x\r\n"

	BUFFER *output;
	int nMatch;
	bool noimm4morts=false;
    char_data *wch;
	char buf[MIL], fbuf[MIL];
	bool remove_holylight=false;
	bool immSeen=false;
	int mortcolcount, immcolcount, currentcol;
	bool two_columns =false;
	bool levelsort=GAMESETTING2(GAMESET2_WHOLIST_SORT_BY_LEVEL)?true:false; 
	bool imm_before_morts=GAMESETTING2(GAMESET2_WHOLIST_IMMS_BEFORE_MORTS)?true:false; 

	int lines_to_show=ch->lines;

	if (lines_to_show==0){
        lines_to_show=9091; // pick any number greater than max on
	}

	char spacer[10];
	if(GAMESETTING2(GAMESET2_NO_AUTOLOGOFF_FOR_IMM)){
	    strcpy( spacer, "     " );
	}else{
	    strcpy( spacer, "    " );
	}


	if ( is_number( argument  ) ){
		ch->println("Levels are not supported by who.");
		return;
	}

	strcpy(buf, argument);

	if(!str_cmp(buf,"-noimm4morts"))
	{
		if(GAMESETTING4(GAMESET4_LOGINWHO_HIDES_IMMS_FROM_MORTS)){
			noimm4morts=true;
		}
		buf[0]='\0';
	}

    /*
     * Now show matching chars.
     */

    nMatch = 0;
    output = new_buf();	


	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD))
	{
		char pkbuf[MSL];
		// the pkill port header
		add_buf(output,WHOHEADER_PKILL_PORT);
        sprintf(pkbuf,"   Max Pkills  : %13s (L%d) of %d\r\n",
            p9999maxpkname, p9999maxpklevel, p9999maxpk);
		add_buf(output,pkbuf);
        sprintf(pkbuf,"   Max Pdefeats: %13s (L%d) of %d\r\n",
            p9999maxpdname, p9999maxpdlevel, p9999maxpd);
		add_buf(output,pkbuf);	
        // the mortals header
		add_buf(output,spacer);
        add_buf(output,WHOHEADER_MORTALS);
	}

	if (IS_IMMORTAL(ch) && !IS_SET(ch->act, PLR_HOLYLIGHT)){
        SET_BIT(ch->act, PLR_HOLYLIGHT);
		remove_holylight=true;
	}

	// count how many lines will be displayed
	mortcolcount=immcolcount=0;
    for ( wch = player_list; wch; wch = wch->next_player )
    {
		// players in the same clan can always see each other on who
		if(!(ch->clan && is_same_clan(ch,wch))){
			if (!can_see_who(ch,wch))
				continue;
		}

		// filter on name and clan name if appropriate 
		if( wch->clan>0 && (is_same_clan(ch,wch)||IS_IMMORTAL(ch))){
			if (!is_name( buf, wch->name )
				&& !is_name( buf, wch->clan->notename())
				)// filter on name/clanname
				continue;
		}else{
			if (!is_name( buf, wch->name )) // filter on name 
				continue;
		}

		// no imms on the mortals list
		if (wch->level>=LEVEL_IMMORTAL){
			immcolcount++;
			continue;
		}
		mortcolcount++;
	}

	if (immcolcount>0){
		immcolcount++;
	}
	if (mortcolcount+immcolcount >= lines_to_show){
		two_columns=true;
	}

	// if the who formatter can't support two columns, then we drop back to one
	if(!who_formatter[who_get_format(ch)].two_column_supported){
		two_columns=false;
	}

	// use an array of pointers to sort the list by level if necessary
	char_data *first_on_who=NULL;
	if(levelsort){
		char_data *level_sorted_chars_start[ABSOLUTE_MAX_LEVEL+1];
		char_data *level_sorted_chars_end[ABSOLUTE_MAX_LEVEL+1];
		
		// start by setting the array to NULL
		memset(level_sorted_chars_start, 0, sizeof(level_sorted_chars_start));
		memset(level_sorted_chars_end, 0, sizeof(level_sorted_chars_end));
				
		// loop thru adding each one to the pointer array
		// players of equal level are inserted closer to the end 
		// as they go, so the longer on the lower on the list they appear
		int level;
	    for ( wch = player_list; wch; wch = wch->next_player )
		{
			level=URANGE(0, wch->level-1, ABSOLUTE_MAX_LEVEL);
			if(level_sorted_chars_start[level]){
				level_sorted_chars_end[level]->next_who_list=wch;
			}else{
				level_sorted_chars_start[level]=wch;
			}
			level_sorted_chars_end[level]=wch;
			wch->next_who_list=NULL;
		}

		// now loop thru linking the end pointers together
		int last_level=-1;
		for(level=MAX_LEVEL; level>=0; level--){
			if(level_sorted_chars_end[level]){
				if(last_level!=-1){
					level_sorted_chars_end[last_level]->next_who_list=
						level_sorted_chars_start[level];
				}else{
					first_on_who=level_sorted_chars_start[level];
				}
				last_level=level;
			}
		}
	}else{
		first_on_who=player_list;
	}

	currentcol=0;
	if(!imm_before_morts){
		// list all mortals
		for ( wch = first_on_who; wch; wch = levelsort?wch->next_who_list:wch->next_player )
		{
			// players in the same clan can always see each other on who
			// except imms in clans
			if(!(ch->clan>0 && is_same_clan(ch,wch))){
				if(!can_see_who(ch,wch)){
					continue;
				}
			}

			if (noimm4morts && wch->level >= LEVEL_IMMORTAL 
				&& !IS_IMMORTAL(ch)){
				continue;
			}

			// filter on name and clan name if appropriate 
			if( wch->clan>0 && (is_same_clan(ch,wch)||IS_IMMORTAL(ch))){
				if (!is_name( buf, wch->name )
					&& !is_name( buf, wch->clan->notename())
					)// filter on name/clanname
					continue;
			}else{
				if (!is_name( buf, wch->name )) // filter on name 
					continue;
			}

			// no imms on the mortals list
			if(wch->level>=LEVEL_IMMORTAL){
				if(can_see_who(ch,wch)){
					immSeen=true;
				}
				continue;
			}

			nMatch++;

			if (two_columns){
				++currentcol%=2;
				sprintf(fbuf, "%s`x %s", str_width(who_format_redirector(ch, wch, two_columns),38,true), 
					currentcol==0?"\r\n":"");
				add_buf(output,fbuf);
			}else{
				add_buf(output, who_format_redirector(ch, wch, false));
				add_buf(output, "\r\n");
			}
		}
	}

	// list immortals 
	if(immSeen || imm_before_morts)
	{
		// add linefeed if nessary
		if (currentcol==1){
			add_buf(output, "\r\n");
		}

		// the immortals header
		add_buf(output,spacer);
		add_buf(output,WHOHEADER_IMMORTALS);

		currentcol=0;
		for ( wch = first_on_who; wch; wch = levelsort?wch->next_who_list:wch->next_player )
		{
			if (!can_see_who(ch,wch))
				continue;

			// no mortals
			if (wch->level<LEVEL_IMMORTAL)
				continue;

			if (noimm4morts && wch->level >= LEVEL_IMMORTAL 
				&& !IS_IMMORTAL(ch))
				continue;

			if (!is_name( buf, wch->name )) // filter on name 
				continue;

			nMatch++;
			if (two_columns){
				++currentcol%=2;
				sprintf(fbuf, "%s %s", str_width(who_format_redirector(ch, wch, true),38,true), 
					currentcol==0?"\r\n":"");
				add_buf(output,fbuf);
			}else{
				add_buf(output, who_format_redirector(ch, wch, false));
				add_buf(output, "\r\n");
			}
		}
	}

	if(imm_before_morts){
        // the mortals header
		add_buf(output,spacer);
        add_buf(output, WHOHEADER_MORTALS);
		// list all mortals
		currentcol=0;
		for ( wch = first_on_who; wch; wch = levelsort?wch->next_who_list:wch->next_player )
		{
			// players in the same clan can always see each other on who
			if(!(ch->clan>0 && is_same_clan(ch,wch))){
				if (!can_see_who(ch,wch))
					continue;
			}

			if (noimm4morts && wch->level >= LEVEL_IMMORTAL 
				&& !IS_IMMORTAL(ch))
				continue;

			// filter on name and clan name if appropriate 
			if( wch->clan>0 && (is_same_clan(ch,wch)||IS_IMMORTAL(ch))){
				if (!is_name( buf, wch->name )
					&& !is_name( buf, wch->clan->notename())
					)// filter on name/clanname
					continue;
			}else{
				if (!is_name( buf, wch->name )) // filter on name 
					continue;
			}

			// no imms on the mortals list
			if (wch->level>=LEVEL_IMMORTAL){
				immSeen=true;
				continue;
			}

			nMatch++;

			if (two_columns){
				++currentcol%=2;
				sprintf(fbuf, "%s`x %s", str_width(who_format_redirector(ch, wch, two_columns),38, true), 
					currentcol==0?"\r\n":"");
				add_buf(output,fbuf);
			}else{
				add_buf(output, who_format_redirector(ch, wch, false));
				add_buf(output, "\r\n");
			}
		}
	}


	if (currentcol==1){
		add_buf(output, "\r\n");
	}
		

    show_whocount(ch, nMatch, noimm4morts);

    if (IS_NEWBIE(ch)){
        add_buf(output,"see 'help who' for an explanation of the symbols.\r\n");
    }

    if (nMatch<15){
		if(number_range(1,2)==1){
			add_buf(output,"`xFor more info on maxon and other mud related stats type `=Cmudstats`x.\r\n");
		}else{
			add_buf(output,"`xYou can customise the colours of the wholist using the `=Ccolour`x command.\r\n");
		}
	}

    ch->sendpage(buf_string(output));
    free_buf(output);

	if(remove_holylight){
		REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	}

#ifdef unix    
	// lag those that spam who 
    if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
    {  
        if ( (current_time - 90) < ch->pcdata->last_who)
        {
            WAIT_STATE( ch, 4 ); // 1 second lag 
        }
        if ( (current_time - 60) < ch->pcdata->last_who)
        {
            WAIT_STATE( ch, 12 ); // 3 seconds lag 
        }
        if ( (current_time - 30) < ch->pcdata->last_who)
        {
            WAIT_STATE( ch, 20 ); 
        }
        ch->pcdata->last_who = current_time;
    }
#endif
    return;
}

/**************************************************************************/
void do_whotext( char_data *ch, char *argument )
{
	if(IS_NULLSTR(argument)){
		ch->println("Sets the *** text for maxlevel+1 in the wholist.");
		return;
	}

	if(c_str_len(argument )!=3){
		ch->println("The argument must have a visible width of 3!");
		return;
	}
	
	if(ch->pcdata){
		ch->printlnf( "whotext changed from '%s' to '%s'",
			ch->pcdata->who_text, argument);
		free_string(ch->pcdata->who_text);
		ch->pcdata->who_text=str_dup(argument);
	}else{
		ch->println("Players only sorry!");
	}

}
/**************************************************************************/
void do_title(char_data *ch, char *argument)
{
	// unswitched mobs can't be seen on the who
	if(IS_UNSWITCHED_MOB(ch)){
		ch->println( "Players or switched players only." );
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println("syntax: Title <text to appear to the right of your name on the wholist>.");
		ch->println("syntax: title -  (to clear your title)");
		return;
	}

	if(c_str_len(argument)>45){
		ch->printlnf("Your title can be a maximum of 45 visible characters long, that is %d.",
			c_str_len(argument));
		return;
	}

	if(c_str_len(argument)<0){
		ch->println("You can't put newline colour codes in your title sorry.");
		return;
	}

	if(!str_cmp(argument,"-")){
		ch->println("Your title has been cleared.");
		replace_string(TRUE_CH(ch)->pcdata->title,"");
		return;
	}

	if(IS_NULLSTR(TRUE_CH(ch)->pcdata->title)){
		ch->printlnf("Your who title has been set to '%s'", argument);
	}else{
		ch->printlnf("Your who title has changed from '%s' to '%s'", 
			TRUE_CH(ch)->pcdata->title,
			argument);
	}
	replace_string(TRUE_CH(ch)->pcdata->title, argument);
}
/**************************************************************************/
void do_immtitle(char_data *ch, char *argument)
{
	// unswitched mobs can't be seen on the who
	if(IS_UNSWITCHED_MOB(ch)){
		ch->println( "Players or switched players only." );
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: immtitle <text to appear in place of the race on the wholist>.");
		ch->println("Syntax: 'immtitle default' to set back to your racial title.");
		return;
	}

	if(!str_cmp(argument,"default")){
		if(IS_NULLSTR(TRUE_CH(ch)->pcdata->immtitle)){
			ch->println("Your who immtitle is already the default");
		}else{
			ch->printlnf("Your who immtitle has been set back to the default, it was previously '%s'", 
				TRUE_CH(ch)->pcdata->immtitle);
			replace_string(TRUE_CH(ch)->pcdata->immtitle, "");
		}
		return;
	}

	if(c_str_len(argument)!=5){
		ch->println("Your immtitle must work out to be exactly 5 visible characters long (you can use colour codes).");
		return;
	}

	if(IS_NULLSTR(TRUE_CH(ch)->pcdata->immtitle)){
		ch->printlnf("Your who immtitle has been set to '%s'", argument);
	}else{
		ch->printlnf("Your who immtitle has changed from '%s' to '%s'", 
			TRUE_CH(ch)->pcdata->immtitle,
			argument);
	}
	replace_string(TRUE_CH(ch)->pcdata->immtitle, argument);
}
/**************************************************************************/
/**************************************************************************/


