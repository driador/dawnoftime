/**************************************************************************/
// support.cpp - see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: support.cpp - Code primarily relating to jobs support does.      *
 *  - letgaining code
 *  - lethero code
 *  - automated letgaining code.. etc.							   
 ***************************************************************************/
#include "include.h"
#include "support.h"
#include "olc.h"
#include "channels.h"

// prototypes
letgain_data *find_letgain( char *name);
void save_letgain_db( void);

/**************************************************************************/
void do_letgain( char_data *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    char_data *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        ch->println("Letgain whom?");
        return;
    }
    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println("They aren't here.");
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println("Not on NPC's.");
        return;
    }

    if ( IS_LETGAINED(victim) )
    {
        ch->println("They can gain levels and exp already.");
		// give them a permit - incase we ban later
		SET_BIT(victim->act,PLR_PERMIT);
    }
    else
    {
        SET_BIT(victim->act, PLR_CAN_ADVANCE);
		if (IS_SILENT(ch))
		{
			victim->println("You have been letgained by an anonymous immortal/noble!");
			sprintf(buf,"$N silently gives %s the ability to progress.",victim->name);
			wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	        ch->println("They can now progress. - silent letgain - (pfile automatically updated)");
		}
		else
		{
			victim->printlnf("You have been letgained by %s!", ch->name);
			sprintf(buf,"$N gives %s the ability to progress.",victim->name);
			wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	        ch->println("They can now progress.  (pfile automatically updated)");
		}
        victim->println("`RMake sure you have read HELP PKILL - as if you kill another player");
        victim->println("`Rfor ooc reasons the moot can be -500 xp or higher!");
        
		// give them a permit - incase we ban someone from their site later
		SET_BIT(victim->act,PLR_PERMIT);
    }

    save_char_obj( victim );

	// check the offline letgain database if it needs updating
	{
		letgain_data *node;

		// find the node
		node=find_letgain(victim->name);
		if (node){
			// update the node
			REMOVE_BIT(node->flags, LETGAIN_REQUESTED);
			SET_BIT(node->flags, LETGAIN_GRANTED);
			REMOVE_BIT(node->flags, LETGAIN_PENDING);
			replace_string(node->answered_by,TRUE_CH(ch)->name);
			replace_string(node->granted_short,victim->short_descr);
			node->granted_moot=-1;
			node->granted_date=current_time;
			save_letgain_db();
		}
	}
    return;
}


/**************************************************************************/
void do_lethero( char_data *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    char_data *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        ch->println("Lethero whom?");
        return;
    }
    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println("They aren't here.");
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println("Not on NPC's.");
        return;
    }

    if ( IS_SET(victim->act, PLR_CAN_HERO) )
    {
        ch->println("They can hero already.");
    }
    else
    {
        SET_BIT(victim->act, PLR_CAN_HERO);
        victim->println("You have been lethero'ed!!!");
        sprintf(buf,"$N gives %s the ability to progress to HERO!!!",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    ch->println("They can now obtain level hero - (pfile automatically updated)");
    save_char_obj( victim );
    return;
}

/**************************************************************************/
void do_nsupport( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
	
    if (!IS_ADMIN(ch)
        && !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADSUPPORT))
	{
        ch->println("Newbie support can only be used by admin or headsupport.");
        return;		
	}


    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        ch->println("Set as newbie support on whom?");
        return;
    }
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        ch->println("They aren't here.");
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println("Not on NPC's.");
        return;
    }

	if(IS_IMMORTAL(victim)){
		ch->println("Immortals automatically hear newbietalk and can help newbies.");
		return;
	}

    if ( IS_SET(victim->comm, COMM_NEWBIE_SUPPORT) )
    {
        REMOVE_BIT(victim->comm, COMM_NEWBIE_SUPPORT);
        ch->printlnf("Newbie support turned off for %s.", PERS(victim, ch));        
    }else{
        SET_BIT(victim->comm, COMM_NEWBIE_SUPPORT);
        ch->printlnf("Newbie support turned on for %s.", PERS(victim, ch));
    }

    save_char_obj( victim );
    return;
}


/**************************************************************************/
void do_unletgain( char_data *ch, char *argument )
{
         char arg[MIL],buf[MSL];
         char_data *victim;

         one_argument( argument, arg );

         if ( arg[0] == '\0' )
         {
        ch->println("Unletgain whom?");
        return;
    }
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        ch->println("They aren't here.");
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println("Not on NPC's.");
        return;
    }

    if ( !IS_LETGAINED(victim) )
    {
        ch->println("They haven't been letgained yet.");
    }
    else
    {
        REMOVE_BIT(victim->act, PLR_CAN_ADVANCE);
        victim->println("You are no longer letgained!");
        ch->println("They are no longer letgained.");
        sprintf(buf,"$N removes %s the ability to progress.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

		// remove their ban permit just incase
		REMOVE_BIT(victim->act,PLR_PERMIT);
    }

    save_char_obj( victim );
    return;
}


/**************************************************************************/
void do_unlethero( char_data *ch, char *argument )
{
     char arg[MIL],buf[MSL];
     char_data *victim;

     one_argument( argument, arg );

     if ( arg[0] == '\0' )
     {
        ch->println("Unlethero whom?");
        return;
    }
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        ch->println("They aren't here.");
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println("Not on NPC's.");
        return;
    }

    if ( !IS_SET(victim->act, PLR_CAN_HERO) )
    {
        ch->println("They haven't been lethero'ed yet.");
    }
    else
    {
        REMOVE_BIT(victim->act, PLR_CAN_HERO);
        victim->println("You are no longer lethero'ed!");
        ch->println("They are no longer lethero'ed.");
        sprintf(buf,"$N removes %s the ability to progress to hero!",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );
    return;
}
/**************************************************************************/
void do_checkhelp( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("NOHELP INFO"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 10 " NO_HELP_FILE );
		add_buf(output, "\r\n    You can select the number of loglines, type checkhelp <number of lines>");
	
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " NO_HELP_FILE);
		}
		else
		{
			sprintf(buf, "tail -n %d " NO_HELP_FILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the error "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " NO_HELP_FILE );
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}
/**************************************************************************/
void do_checkooc( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("OOC LOGFILE"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 10 " OOC_LOGFILE);
		add_buf(output, "\r\n    You can select the number of loglines, type checkooc <number of lines>");
	
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " OOC_LOGFILE );
		}
		else
		{
			sprintf(buf, "tail -n %d " OOC_LOGFILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the ooc " 
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " OOC_LOGFILE);
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	sprintf( buf2,"Current time: %s   Your Login Time: %s\r\n",
			shorttime(NULL), shorttime(&ch->logon));
	add_buf(output,buf2);

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}
/**************************************************************************/
void do_checkmoblog( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("MOBLOG LOGFILE"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 10 " MOBLOG_LOGFILE);
		add_buf(output, "\r\n    You can select the number of loglines, type checkmoblog <number of lines>");
	
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " MOBLOG_LOGFILE );
		}
		else
		{
			sprintf(buf, "tail -n %d " MOBLOG_LOGFILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the moblog "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " OOC_LOGFILE );
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}
/**************************************************************************/
void do_checknsupport( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    if (!IS_ADMIN(ch)
        && !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADSUPPORT)
		&& !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADLAW))	
    {
        ch->println("Checknsupport can only be used by the support and law councils or admin.");
        return;
    }


    output= new_buf();

    sprintf( buf,"`?%s`x", format_titlebar("NSUPPORT LOGFILE"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
        sprintf(buf, "tail -n 10 " NEWBIE_SUPPORT_LOG_FILE );
        add_buf(output, "\r\n    You can select the number of loglines, type checknsupport <number of lines>");
	
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
            sprintf(buf, "tail -n 10 " NEWBIE_SUPPORT_LOG_FILE );
		}
		else
		{
            sprintf(buf, "tail -n %d " NEWBIE_SUPPORT_LOG_FILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
            "numeric value\r\nfor the number of lines of the nsupport "
			"log you wish to see.`x\r\n");
        sprintf(buf, "tail -n 10 " NEWBIE_SUPPORT_LOG_FILE );
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}
/**************************************************************************/
void do_telnetga(char_data *ch, char *)
{
	 if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->comm,COMM_TELNET_GA))
    {
      ch->println("Telnet GA (Go Ahead) is no longer sent.");
		REMOVE_BIT(ch->comm,COMM_TELNET_GA);
    }
	 else
    {
      ch->println("Telnet GA (Go Ahead) is now sent.");
		SET_BIT(ch->comm,COMM_TELNET_GA);
	 }
}
/**************************************************************************/
void do_checkntalk( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("NTALK LOGFILE"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 10 " NTALK_LOGFILE );
		add_buf(output, "\r\n    You can select the number of loglines, type checkntalk <number of lines>");
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " NTALK_LOGFILE );
		}
		else
		{
			sprintf(buf, "tail -n %d " NTALK_LOGFILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the error "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " NTALK_LOGFILE );
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}
/**************************************************************************/
void do_checktypos( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("TYPO INFO"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 10 " TYPO_FILE);
		add_buf(output, "\r\n    You can select the number of loglines, type checktypo <number of lines>");
	
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " TYPO_FILE );
		}
		else
		{
			sprintf(buf, "tail -n %d " TYPO_FILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the error "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " TYPO_FILE);
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}


/**************************************************************************/

// create skill_type GIO lookup table 
GIO_START(letgain_data)
GIO_STRH(name,				"Name          ")
GIO_INTH(player_id,			"Id            ")
GIO_STRH(current_short,		"Current_short ")
GIO_STRH(requested_short,	"Req_short     ")
GIO_STRH(requested_long,	"Req_long      ")
GIO_INTH(requested_date,	"Req_date      ")
GIO_SHINTH(requested_level,	"Req_level     ")
GIO_SHINTH(requested_alliance,	"Req_alliance  ")
GIO_SHINTH(requested_tendency,	"Req_tendency  ")
GIO_WFLAGH(flags,			"Flags         " , letgain_db_flags)
GIO_STRH(answered_by,		"Answered_by   ")
GIO_INTH(granted_date,		"Grant_date    ")
GIO_SHINTH(granted_moot,	"Grant_moot    ")
GIO_STRH(granted_short,		"Grant_short   ")
GIO_STRH(granted_long,		"Grant_long    ")
GIO_INTH(letgain_date,		"Letgain_date  ")
GIO_STRH(denied_reason,		"Denied_Reason ")
GIO_STRH(history,			"History       ")
GIO_FINISH

/**************************************************************************/
// loads in the letgain database
void load_letgain_db( )
{
	logf("===Loading letgain database from %s...", LETGAINDB_FILE);
	if(file_exists(LETGAINDB_FILE)){
		GIOLOAD_LIST(letgain_list, letgain_data, LETGAINDB_FILE);
	}else{
		logf("Letgain database file not present - this is normal if no "
			"players have requested on offline letgain yet.");
	}    
	log_string ("load_letgain_db(): finished");
}
/**************************************************************************/
// saves the letgain database
void save_letgain_db( )
{
    logf("===Saving letgain database to %s.", LETGAINDB_FILE);
	GIOSAVE_LIST(letgain_list, letgain_data, LETGAINDB_FILE, true);		
    logf("Letgain DB save completed.");
}
/**************************************************************************/
// returns a pointer to a letgain node
letgain_data *find_letgain( char *name)
{
	letgain_data *node;

	for (node = letgain_list; node; node= node->next){
		if(!str_cmp(name,node->name)){
			return node;
		}
	}
	return NULL;
}
/**************************************************************************/
// lists letgains
void do_list_letgains( char_data *ch, char *argument )
{
	letgain_data *node;
	int count;

	if(!IS_NULLSTR(argument))
	{
		ch->titlebar("LIST LETGAINS - DETAILS");

		for (node = letgain_list; 
			node && str_cmp(node->name,argument); 
			node= node->next)
		{
		}
		if(node){
			ch->printlnf("`=RName: `=r%s", node->name);
			ch->printlnf("`=RCreated: `=r%-24.24s", (char*)ctime((time_t*)&node->player_id));

			if(!IS_NULLSTR(node->answered_by)){
				ch->printlnf("`=RAnswered By: `=r%s", node->answered_by);
			}		
			ch->printlnf("`=RShort at time of requesting: `x%s", 
				!IS_NULLSTR(node->current_short)?node->current_short:"(none)");

			if(!IS_NULLSTR(node->requested_long)){
				ch->printlnf("`=RLong at time of requesting: `x\r\n%s", 
					node->requested_long);
			}else{
				ch->println("`=RLong at time of requesting: `x(none)"); 
			}
			ch->printlnf("`=RLevel at time of requesting: `x%d", 
				node->requested_level); 
			ch->printlnf("`=RAlliance at time of requesting: `x%d", 
				node->requested_alliance); 
			ch->printlnf("`=RTendency at time of requesting: `x%d", 
				node->requested_tendency); 

			ch->printlnf("`=RRequested Short: `x%s", 
				!IS_NULLSTR(node->requested_short)?node->requested_short:"(none)");

			if(!IS_NULLSTR(node->granted_short))
				ch->printlnf("`=RGranted short: `=r%s", node->granted_short);
			ch->printlnf("`=RGranted Moot: `=r%d", node->granted_moot);
			if(!IS_NULLSTR(node->denied_reason)){
				ch->println("`=RReason given for declining: `=r");
				ch->wrapln(node->denied_reason);
			}
			ch->printlnf("`=RFlags: `x%s", flag_string( letgain_db_flags, node->flags) );
			if(node->letgain_date)
				ch->printlnf("`=Rletgain date: `=R%-24.24s", (char*)ctime(&node->letgain_date));
		}else{
			ch->printlnf("couldn't find any '%s' in the letgain database.",
				argument);
		}

		return;
	}
	
	ch->titlebar("LIST LETGAINS");

	ch->titlebar("requested");
	ch->printlnf("   %-13s  lvl %s", "name", "date");
	count=0;
	for (node = letgain_list; node; node= node->next){
		if(IS_SET(node->flags, LETGAIN_REQUESTED)){
			ch->printlnf("`x%-2d> %-13s`S-`G%2d`x%-19.19s`S-`B%s`x",
				++count, node->name, node->requested_level,
				 ctime( &node->requested_date),
				 node->requested_short);
		}
	}
	ch->titlebar("accepted, waiting for login");
	ch->printlnf("   %-13s   %s                      moot_amount   by", "name", "date");
	count=0;
	for (node = letgain_list; node; node= node->next){
		if(node->requested_date< (current_time - (60*60*24*14))){
			continue;
		}
		if(IS_SET(node->flags, LETGAIN_PENDING) 
			&& IS_SET(node->flags,LETGAIN_GRANTED)){
			ch->printlnf("%-2d> %-13s   %-24.24s  %4d        %s",
				++count, node->name, 
				 ctime( &node->requested_date),
				 node->granted_moot,
				 node->answered_by);
		}
	}
	ch->titlebar("declined, waiting for login");
	ch->printlnf("   %-13s   %s                      moot_amount   by", "name", "date");
	count=0;
	for (node = letgain_list; node; node= node->next){
		if(node->requested_date< (current_time - (60*60*24*14))){
			continue;
		}

		if(IS_SET(node->flags, LETGAIN_PENDING) 
			&& IS_SET(node->flags,LETGAIN_DECLINED)){
			ch->printlnf("%-2d> %-13s   %-24.24s  %4d        %s",
				++count, node->name, 
				 ctime( &node->requested_date),
				 node->granted_moot,
				 node->answered_by);
		}
	}
	ch->titlebar("declined, waiting for reapplication");
	ch->printlnf("   %-13s   %s                      moot_amount   by", "name", "date");
	count=0;
	for (node = letgain_list; node; node= node->next){
		if(node->requested_date< (current_time - (60*60*24*14))){
			continue;
		}

		if(!IS_SET(node->flags, LETGAIN_PENDING) 
			&& IS_SET(node->flags,LETGAIN_DECLINED)){
			ch->printlnf("%-2d> %-13s   %-24.24s  %4d        %s",
				++count, node->name, 
				 ctime( &node->requested_date),
				 node->granted_moot,
				 node->answered_by);
		}
	}
	ch->titlebar("accepted and letgained.");
	ch->printlnf("   %-13s   %s                      moot_amount   by", "name", "date");
	count=0;
	for (node = letgain_list; node && count<=10; node= node->next){
		if(!IS_SET(node->flags, LETGAIN_PENDING) 
			&& IS_SET(node->flags,LETGAIN_GRANTED)){
			ch->printlnf("%-2d> %-13s   %-24.24s  %4d        %s",
				++count, node->name, 
				 ctime( &node->requested_date),
				 node->granted_moot,
				 node->answered_by);
		}
	}
	ch->println("type listlet <playername> for more details on a single player.");
}
/**************************************************************************/
// handle accepted offline letgains
void do_offlineletgain( char_data *ch, char *argument )
{
	letgain_data *node;
    char name[MIL], amount[MIL];
	int value;
    argument = one_argument( argument, name);
    argument = one_argument( argument, amount);
    // trim the spaces to the right of the optional new name
    while ( !IS_NULLSTR(argument) && is_space(argument[str_len(argument)-1]))
    {
        argument[str_len(argument)-1]='\0';
    }
	
	if(IS_NULLSTR(name) || IS_NULLSTR(amount) ){
		ch->println("Syntax: offlineletgain <name> <moot amount> [new_short]");
		ch->println("  new_short is an optional new short description.");
		ch->println("use declineletgain to deny a letgain request.");
		return;		
	}

	if(!is_number(amount)){
		ch->println("Syntax: offlineletgain <name> <moot amount> [new_short]");
		ch->println("  new_short is an optional new short description.");
		ch->println("Moot amount must be a number.");
		return;
	}

	value=atoi(amount);
	if(value<0 || value>4000){
		ch->println("Moot value not in range 0 to 4000.");
		return;
	}

	// find the node
	node=find_letgain(name);
	if (!node){
		ch->printlnf("There is no one in the letgain database with a name of '%s'.",
			name);
		return;
	}
	
	// update the node
	REMOVE_BIT(node->flags, LETGAIN_REQUESTED);
	REMOVE_BIT(node->flags, LETGAIN_DECLINED);
	SET_BIT(node->flags, LETGAIN_GRANTED);
	SET_BIT(node->flags, LETGAIN_PENDING);
	replace_string(node->answered_by,TRUE_CH(ch)->name);
	replace_string(node->granted_short,argument);
	node->granted_moot=value;
	node->granted_date=current_time;
	ch->printlnf("Offline letgain on %s granted with a %d moot.", 
		node->name, node->granted_moot);
	save_letgain_db();
}
/**************************************************************************/
// decline letgain
void do_declineletgain( char_data *ch, char *argument )
{
	letgain_data *node;
    char name[MIL];
    argument = one_argument( argument, name);

	if(IS_NULLSTR(name) || IS_NULLSTR(argument) ){
		ch->println("Syntax: declineletgain <name> <reason>");
		ch->println("use offlineletgain to accept a letgain request.");
		return;		
	}

	if(is_number(argument)){
		ch->println("Syntax: declineletgain <name> <reason>");
		ch->println("The reason must be a text message.");
		return;
	}

	// find the node
	node=find_letgain(name);
	if (!node){
		ch->printlnf("There is no one in the letgain database with a name of '%s'.",
			name);
		return;
	}
	
	// update the node
	REMOVE_BIT(node->flags, LETGAIN_REQUESTED);
	REMOVE_BIT(node->flags, LETGAIN_GRANTED);
	SET_BIT(node->flags, LETGAIN_DECLINED);
	SET_BIT(node->flags, LETGAIN_PENDING);
	replace_string(node->denied_reason,argument);
	replace_string(node->answered_by, TRUE_CH(ch)->name);
	node->granted_moot=0;
	ch->printlnf("Offlinet letgain on %s `Rdeclined`x.", node->name);
	save_letgain_db();
}
/**************************************************************************/
// used by players to request a letgain
void do_requestletgain( char_data *ch, char *argument )
{
	letgain_data *node;
	static letgain_data zero_node;

    if ( IS_NPC(ch) )
    {
        ch->println("Players only sorry.");
        return;
    }

    if ( IS_LETGAINED(ch) )
    {
        ch->println("You are already letgained.");
		return;
    }

	if(IS_NULLSTR(argument)){
		ch->println("syntax: requestletgain <short description>");
		ch->println("e.g. `=Crequestletgain a tall elf with black hair`x");
	
		ch->println("`RNOTE: BEFORE USING THIS COMMAND`x");
		ch->wrapln("You should have a proper long description "
		"(`=Chelp long`x), sent you history already (`=Chelp history`x), also "
		"the short description you request must be suitable with "
		"`=Chelp short`x... if `RANY`x of the above 3 things (long, history "
		"and short) do them `RFIRST!!!`x.");
		return;		
	}

    // trim the spaces to the right of the short
    while ( !IS_NULLSTR(argument) && is_space(argument[str_len(argument)-1]))
    {
        argument[str_len(argument)-1]='\0';
    }

    // check we have a short left 
    if(argument[0] == '\0' )
    {
        ch->println("You must put in a short description.");
		do_requestletgain(ch,"");
        return;
	}
    // make sure first char is lowercase 
    argument[0] = LOWER(argument[0]);

    if (str_len(argument)>55){
		ch->println("That short descriptions too LONG... try something shorter.");			
		return;
	}

	// find the node
	node=find_letgain(TRUE_CH(ch)->name);
	if (!node){
		// add a new node to the head of the linked list
		node=new letgain_data;
		*node=zero_node;
		node->next=letgain_list;
		letgain_list=node;
		replace_string(node->name,TRUE_CH(ch)->name);
	}else{
		// remove any things from the old letgain
		replace_string(node->denied_reason,"");
		ch->println("Updating letgain request with new info.");
	}
	REMOVE_BIT(node->flags, LETGAIN_GRANTED);
	REMOVE_BIT(node->flags, LETGAIN_PENDING);
	REMOVE_BIT(node->flags, LETGAIN_DECLINED);
	SET_BIT(node->flags, LETGAIN_REQUESTED);

	node->requested_date=current_time;
	node->player_id=ch->player_id;
	node->granted_moot=0;
	node->requested_level=ch->level;
	node->requested_alliance=ch->alliance;
	node->requested_tendency=ch->tendency;

	replace_string(node->requested_long, ch->description);
	replace_string(node->requested_short, argument); 
	replace_string(node->current_short, ch->short_descr);
	replace_string(node->requested_long, ch->description);
	replace_string(node->granted_short, "");
	replace_string(node->granted_long, "");
	replace_string(node->answered_by, "");

	ch->wraplnf("A letgain request has been put in for %s, "
		"asking for the short description of '%s'", 
		ch->name, 
		node->requested_short);
	ch->wrapln("`RNOTE: `xYou should have a proper long description "
		"(`=Chelp long`x), sent you history already (`=Chelp history`x), and the "
		"short description you have requested must be suitable with "
		"`=Chelp short`x... if `RANY`x of the above 3 things (long, history "
		"and short) have not been done or are incorrect type "
		"`=Ccancelletgain`x now! and try again once you have done the 3 things.");
	save_letgain_db();
}
/**************************************************************************/
// used by players to request a letgain
void do_cancelletgain( char_data *ch, char *)
{
	letgain_data *node, *prev, *current;

    if ( IS_NPC(ch) )
    {
        ch->println("Players only sorry.");
        return;
    }

    if ( IS_LETGAINED(ch) )
    {
        ch->println("You are already letgained.");
		return;
    }

	// find the node
	node=find_letgain(TRUE_CH(ch)->name);
	if (!node){
		ch->println("You are not currently in the letgain database.");
		return;
	}
	ch->println("Your request to be letgained has been canceled.");

	if(node==letgain_list){ // remove from head
		letgain_list=letgain_list->next;
	}else{
		prev=letgain_list;
		for(current=letgain_list->next; current; current=current->next){
			if(node==current)
				break;
			prev=current;
		}
		if(current){
			prev->next=current->next;
			delete current; // should deallocate the memory but who cares :)
		}
	}
	save_letgain_db();
}
/**************************************************************************/
void check_offline_letgain(char_data *ch)
{
	letgain_data *node;

    if ( IS_NPC(ch) ){
        ch->println("check_offline_letgain should only be called on players.");
        return;
    }
    if ( IS_LETGAINED(ch) ){
		return;
    }

	// find them in the letgain database
	node=find_letgain(ch->name);
	if(!node){
		return;
	}
	
	int autolet=game_settings->automatic_offlineletgain_after_x_days;

	// unprocessed requests
	if(IS_SET(node->flags, LETGAIN_REQUESTED)){
		if(autolet>0 && (node->requested_date+ (autolet* 24*3600))<current_time){
			// automatically letgain those who have requested the letgain
			// ages ago.
			REMOVE_BIT(node->flags, LETGAIN_REQUESTED);
			SET_BIT(node->flags, LETGAIN_PENDING);
			SET_BIT(node->flags,LETGAIN_GRANTED);
			node->granted_moot=-2;
			replace_string(node->answered_by, "(auto)");
					
			ch->println("");
			ch->wrapln("`YYou have been automatically letgained by the mud "
				"because your request hasn't been processed by an immortal within the "
				"required time.`1`RNote: This doesn't guarantee that your character "
				"is acceptable, and some aspects of your character may have to be "
				"changed at a later date.`x");
		}else{		
			ch->println("");
			ch->wrapln("`YThere is an unprocessed letgain request for your "
				"character in the system... If you aren't automatically letgained "
				"within 2 days of requesting a letgain, ask an immortal about it.`x");
			ch->println("");
			return;
		}
	}

	if(!IS_SET(node->flags, LETGAIN_PENDING) 
		&& IS_SET(node->flags,LETGAIN_DECLINED))
	{
		ch->println("");
		ch->printf("`RWe currently have a declined letgain request in "
			"the autoletgain database for your character... "
			"It was declined with the following reason given:`x\r\n");
		ch->wraplnf("`W%s`x\r\n", node->denied_reason);
		ch->wrapln("`RYou are welcome to reapply using "
			"the `=Crequestletgain`R command (once you have fixed what is ever necessary), "
			"or use the `=Ccancelletgain`R command to stop this message appearing when you login.`x");
		ch->println("");
		return;		
	}


	if(IS_SET(node->flags, LETGAIN_PENDING) 
		&& IS_SET(node->flags,LETGAIN_DECLINED)){

		ch->println("\r\n\r\n\r\n\r\n");
		ch->printf("`RYour letgain request has been processed, "
			"It was declined for the following reason:\r\n");
		ch->wraplnf("`W%s`x", node->denied_reason);
		ch->wrapln("`RYou are welcome to reapply using "
			"the `=Crequestletgain`R command (once you have fixed what is ever necessary), "
			"or use the `=Ccancelletgain`R command to stop this message appearing when you login.`x");
		ch->println("");

		REMOVE_BIT(node->flags, LETGAIN_PENDING);
		save_letgain_db();
		return;	
	}


	if(IS_SET(node->flags, LETGAIN_PENDING) 
		&& IS_SET(node->flags,LETGAIN_GRANTED))
	{// letgain them
	    SET_BIT(ch->act, PLR_CAN_ADVANCE);
		REMOVE_BIT(node->flags, LETGAIN_PENDING);

		logf("check_offline_letgain(): Letgaining %s\n",ch->name);

		if(node->granted_moot!=-2){
			ch->println("\r\n\r\n\r\n\r\n");
		}
		ch->println("`BCongratulations!`x You have been letgained while you were offline!");
		if(node->granted_moot>0){
			ch->printlnf("`GFor your efforts you were granted a rps experience bonus of %d!`x",
				node->granted_moot);
			gain_exp(ch, node->granted_moot);
			ch->pcdata->rp_points+=node->granted_moot;
		}
		// only update their short description if they haven't had it changed since they 
		// started the letgain process.
		if(!str_cmp(ch->short_descr, node->current_short))
		{
			if(!IS_NULLSTR(node->granted_short)){
				replace_string(ch->short_descr, node->granted_short);
				ch->wraplnf("`MYour short description has been changed to '`x%s`M'`x",
						ch->short_descr);
			}else{
				replace_string(ch->short_descr, node->requested_short);
				if(node->granted_moot==-2){ // automated letgain
					ch->wraplnf("`YThe short description has been "
						"changed to '`x%s`Y'`x", ch->short_descr);
				}else{
					ch->wraplnf("`YThe short description you requested was accepted and "
						"your short therefore has been changed to '`x%s`Y'`x",
							ch->short_descr);
				}
			}
		}
		ch->println("");
		REMOVE_BIT(node->flags, LETGAIN_PENDING);
		save_letgain_db();
		return;
	}

}
/**************************************************************************/
// used by imms to clear a letgain record
void do_clearletgain( char_data *ch, char *argument )
{
	letgain_data *node, *prev, *current;

    if ( IS_NPC(ch) )
    {
        ch->println("Players only sorry.");
        return;
    }

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: clearletgain <playername>");
		ch->println("This removes their entry from the letgain database.");
		return;
	}

	// find the node
	node=find_letgain(argument);
	if (!node){
		ch->printlnf("'%s' is not currently in the letgain database.", argument);
		return;
	}
	ch->printlnf("'%s' has been removed from the letgain database.", argument);

	if(node==letgain_list){ // remove from head
		letgain_list=letgain_list->next;
	}else{
		prev=letgain_list;
		for(current=letgain_list->next; current; current=current->next){
			if(node==current)
				break;
			prev=current;
		}
		if(current){
			prev->next=current->next;
			delete current; // should deallocate the memory but who cares :)
		}
	}
	save_letgain_db();
}

/**************************************************************************/
void do_charnotes( char_data *ch, char *argument )
{
	char_data *victim;
	int diff;
    char arg[MIL], buf[MSL], fbuf[MSL], tbuf[MIL];

    if (!IS_IMMORTAL(ch))
    {
		do_huh(ch,"");
		return;	
    }

	argument = one_argument( argument, arg );

	if(IS_NULLSTR(arg))
	{
        ch->println("Syntax: charnotes <playername>");
        ch->println("Use this command to view a characters notes - use editcharnotes to edit.");
        return;
	}

	if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
	{
		ch->println("They aren't here.");
		return;
	}

	if ( IS_NPC(victim) )
	{
		ch->println("Not on NPC's.");
		return;    
	}
	// display the info 
	ch->println("`s=============================================================================`x");	  

	sprintf (tbuf,"%d",victim->level);
	sprintf (buf, "`=r(Letgained: `x%s `=rLevel: `x%s `=rRace: `x%s `=rClass: `x%s`=r)",
		(IS_LETGAINED(victim)?"y":"n"), 
		(IS_IMMORTAL(ch) || victim==ch)?tbuf:"?", 
		race_table[victim->race]->name, 
		class_table[victim->clss].name);
	diff= 69 + str_len(buf);
	diff-= c_str_len(buf); // how many colour codes in string	
	diff-= str_len(victim->name); 
	sprintf (fbuf, "`=rCharnotes: `x%s %%%ds`x\r\n", victim->name, diff);

	sprintf(tbuf,fbuf,buf);
	diff= c_str_len(tbuf);

	// reformat if too long
	if (diff>79)
	{
		sprintf (tbuf,"%d",victim->level);
		sprintf (buf, "`=r(LGed: `x%s `=rLevel: `x%s `=rRace: `x%s `=rClass: `x%s`=r)",
			(IS_LETGAINED(victim)?"y":"n"), 
			(IS_IMMORTAL(ch) || victim==ch)?tbuf:"?", 
			race_table[victim->race]->name, 
			class_table[victim->clss].name);
		diff= 69 + str_len(buf);
		diff-= c_str_len(buf); // how many colour codes in string	
		diff-= str_len(victim->name); 
		sprintf (fbuf, "`=rCharnotes: `x%s %%%ds`x\r\n", victim->name, diff);

		sprintf(tbuf,fbuf,buf);
	}

	ch->print(tbuf);

    ch->println("`s=============================================================================`x");	  
	ch->printlnf("`=rShort description: `=R%s`x", victim->short_descr);

    ch->printf("`=rNotes: `=R%s%s`x",
    IS_NULLSTR(victim->pcdata->charnotes)? "" : "\r\n", 
    IS_NULLSTR(victim->pcdata->charnotes)?  "(none)\r\n":victim->pcdata->charnotes);
	ch->println("`s=============================================================================`x");	  

}
/**************************************************************************/
void do_editcharnotes( char_data *ch, char *argument )
{
	char_data *victim;
	char arg[MIL];

    if (!IS_IMMORTAL(ch))
    {
		do_huh(ch,"");
		return;	
    }

	argument = one_argument( argument, arg );

	if(IS_NULLSTR(arg))
	{
        ch->println("Syntax: editcharnotes <playername>");
        ch->println("Use this command to edit a characters notes - use charnotes to view.");
        return;
	}

	if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
	{
		ch->println("They aren't here.");
		return;
	}

	if ( IS_NPC(victim) )
	{
		ch->println("Not on NPC's.");
		return;    
	}

    if ((get_trust(victim)>= get_trust(ch))&& (ch != victim))
    {
        ch->println("You can't edit charnotes  on someone a higher level or equal to you.");
        return;
    }

    string_append(ch, &victim->pcdata->charnotes);
    return;
}

/**************************************************************************/
int race_lookup (const char *name);
/**************************************************************************/
void do_dlook( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL], fbuf[MSL], tbuf[MIL];
    char_data *victim;
	int diff;

	one_argument( argument, arg );

	if (IS_NULLSTR(arg) || (!IS_NOBLE(ch) && !IS_IMMORTAL(ch)))
	{
		victim= ch;
	}
	else
	{   
		if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}

		if ( IS_NPC(victim) )
		{
			ch->println("Not on NPC's.");
			return;    
		}

		if ( !IS_IMMORTAL(ch) && IS_ICIMMORTAL(victim))
		{
			ch->println("You can't dlook immortals sorry.");
			return;    
		}
	}


	// display the info 
	ch->println("`s=============================================================================`x");	  

	// prevent seeing non creation selectable races etc for others from nonimms
	int race_value=victim->race;
	if(!IS_IMMORTAL(ch) && victim!=ch){
		if(!race_table[race_value]->creation_selectable()){
			race_value=race_lookup("human");
		}
	}

	sprintf (tbuf,"%d",victim->level);
	sprintf (buf, "`=r(Letgained: `x%s `=rLevel: `x%s `=rRace: `x%s `=rClass: `x%s`=r)",
		(IS_LETGAINED(victim)?"y":"n"), 
		(IS_IMMORTAL(ch) || victim==ch)?tbuf:"?", 
		race_table[race_value]->name, 
		class_table[victim->clss].name);
	diff= 69 + str_len(buf);
	diff-= c_str_len(buf); // how many colour codes in string	
	diff-= str_len(victim->name); 
	sprintf (fbuf, "`=rDlook: `x%s %%%ds`x\r\n", 
		mxp_create_tag(ch, FORMATF("ch-uid_name %d", victim->uid), victim->name), diff);

	sprintf(tbuf,fbuf,buf);
	diff= c_str_len(tbuf);

	// reformat if too long
	if (diff>79)
	{
		sprintf (tbuf,"%d",victim->level);
		sprintf (buf, "`=r(LGed: `x%s `=rLevel: `x%s `=rRace: `x%s `=rClass: `x%s`=r)",
			(IS_LETGAINED(victim)?"y":"n"), 
			(IS_IMMORTAL(ch) || victim==ch)?tbuf:"?", 
			race_table[victim->race]->name, 
			class_table[victim->clss].name);
		diff= 69 + str_len(buf);
		diff-= c_str_len(buf); // how many colour codes in string	
		diff-= str_len(victim->name); 
		sprintf (fbuf, "`=rDlook: `x%s %%%ds`x\r\n", 
			mxp_create_tag(ch, FORMATF("ch-uid_name %d", victim->uid), victim->name)
			, diff);

		sprintf(tbuf,fbuf,buf);
	}

	ch->print(tbuf);

    ch->println("`s=============================================================================`x");	  

	// nobles and imms can see other's alliance/tendency
	if (( IS_IMMORTAL(ch) || IS_NOBLE(ch)) && ch != victim ) {
		ch->printlnf( "`=rAlliance: `x%d   `=rTendency: `x%d",
			victim->alliance, victim->tendency );
		ch->println( "`s========================================="
		"====================================`x" );
	}

	ch->printlnf("`=rShort description: `=R%s`x", victim->short_descr);

    ch->printf("`=rLong  description: `=R%s%s`x",
    IS_NULLSTR(victim->description)? "" : "\r\n", 
    IS_NULLSTR(victim->description)?  "(none)\r\n":victim->description);
	ch->println("`s=============================================================================`x");	  

	if ((IS_NOBLE(ch) || IS_IMMORTAL(ch)) && arg[0] == '\0' )
	{
		ch->println("`x     You can see someone elses description by typing `=Cdlook <playername>`x");
		ch->println("`s=============================================================================`x");	  
	}

	if(!IS_NPC(ch)){
		ch->printf("`=rCharacter History: `=R%s%s`x",
			IS_NULLSTR(victim->pcdata->history)? "" : "\r\n", 
			IS_NULLSTR(victim->pcdata->history)?  "(none)\r\n":victim->pcdata->history);
		ch->println("`s=============================================================================`x");	 
	}
}

/**************************************************************************/
void do_checkmultilog( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("MULTILOG RECORDS"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 10 " MULTILOG_FILE );
		add_buf(output, "\r\n    You can select the number of loglines, type checkmultilog <number of lines>");	
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " MULTILOG_FILE );
		}
		else
		{
			sprintf(buf, "tail -n %d " MULTILOG_FILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " MULTILOG_FILE );
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}
/**************************************************************************/
// display the coding log
#define CODELOG_FILE CODE_LOGS_DIR "code.txt"
void do_checkcode( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("CODING LOG"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 20 " CODELOG_FILE);
		add_buf(output, "\r\n    You can select the number of loglines, type checkcode <number of lines>");
		add_buf(output, "\r\n    (A maximum of 40 lines will be displayed at any given time)");

	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " CODELOG_FILE );
		}
		else
		{
			sprintf(buf, "tail -n %d " CODELOG_FILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the error "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " CODELOG_FILE );
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}

/**************************************************************************/
void do_checknewbie( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("NEWBIETALK LOGFILE"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 10 " NEWBIE_LOGFILE );
		add_buf(output, "\r\n    You can select the number of loglines, type checknewbie <number of lines>");
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " NEWBIE_LOGFILE );
		}
		else
		{
			sprintf(buf, "tail -n %d " NEWBIE_LOGFILE " | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " NEWBIE_LOGFILE );
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}

/**************************************************************************/
void do_bardify( char_data *ch, char *argument )
{
	char arg1[MIL];
	char_data *victim;
	
	argument = one_argument( argument, arg1 );
	
	if ( arg1[0] == '\0')
	{
		ch->println( "Syntax: bardify <player>" );
		return;
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		ch->println( "They aren't playing." );
		return;
	}

	if ( IS_NPC( victim ))
	{
		ch->println( "You can't bardify an NPC!" );
		return;
	}

	if ( HAS_CONFIG( victim, CONFIG_BARD_COUNCIL ))
	{
		REMOVE_CONFIG( victim, CONFIG_BARD_COUNCIL );
		ch->printlnf( "%s is no longer in the Bard Council.", victim->name );
	}
	else
	{
		SET_CONFIG( victim, CONFIG_BARD_COUNCIL );
		ch->printlnf( "%s is now in the Bard Council.", victim->name );
	}
    save_char_obj(victim);
    return;
}
/**************************************************************************/
// equips a character, updated for dawn 1.7 by Kal 
// set to MAX_LEVEL by default
void do_outfit( char_data *ch, char *)
{
    OBJ_DATA *obj;
	OBJ_INDEX_DATA *objindex;
    int i,sn,vnum;
	bool equipped=false;	

	if(GAMESETTING(GAMESET_OUTFIT_DISABLED)){
		ch->println("Sorry, the outfit command is unavailable.");
		return;
	}
	
    if (!IS_NEWBIE(ch)|| IS_NPC(ch)){
		ch->println("Outfit can only be used while you are a newbie.");
		return;
    }
	
	// a light
    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL ){
		vnum=OBJ_VNUM_OUTFIT_LIGHT;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_LIGHT );
			equipped=true;
			ch->println("You have mysteriously been equipped with a light.");	
		}else{
			logf("do_outfit(): couldn't find light vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit light object vnum %d, "
					"report to admin if you want this fixed.", vnum);
			}
		}
    }
	
	// about body
    if ( ( obj = get_eq_char( ch, WEAR_TORSO ) ) == NULL ){
		vnum=OBJ_VNUM_OUTFIT_VEST;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_TORSO );
			equipped=true;
			ch->println("You have mysteriously been equipped with something for the body.");	
		}else{
			logf("do_outfit(): couldn't find vest vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit vest object vnum %d, "
					"report to admin if you want this fixed..", vnum);
			}
		}
    }


	// sleeves
    if ( ( obj = get_eq_char( ch, WEAR_ARMS ) ) == NULL ){
		vnum=OBJ_VNUM_OUTFIT_SLEEVES;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_ARMS );
			equipped=true;
			ch->println("You have mysteriously been equipped with something for the arms.");	
		}else{
			logf("do_outfit(): couldn't find sleeves vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit sleeves object vnum %d, "
					"report to admin if you want this fixed..", vnum);
			}
		}
    }

	// cap
    if ( ( obj = get_eq_char( ch, WEAR_HEAD ) ) == NULL ){
		vnum=OBJ_VNUM_OUTFIT_CAP;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_HEAD );
			equipped=true;
			ch->println("You have mysteriously been equipped with something for the head.");	
		}else{
			logf("do_outfit(): couldn't find cap vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit cap object vnum %d, "
					"report to admin if you want this fixed..", vnum);
			}
		}
    }
	
	// gloves
    if ( ( obj = get_eq_char( ch, WEAR_HANDS ) ) == NULL ){
		vnum=OBJ_VNUM_OUTFIT_GLOVES;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_HANDS );
			equipped=true;
			ch->println("You have mysteriously been equipped with something for the hands.");	
		}else{
			logf("do_outfit(): couldn't find gloves vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit gloves object vnum %d, "
					"report to admin if you want this fixed..", vnum);
			}
		}
    }
	
	// leggings
    if ( ( obj = get_eq_char( ch, WEAR_LEGS ) ) == NULL ){
		vnum=OBJ_VNUM_OUTFIT_LEGGINGS;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_LEGS );
			equipped=true;
			ch->println("You have mysteriously been equipped with something for the legs.");	
		}else{
			logf("do_outfit(): couldn't find leggings vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit leggings object vnum %d, "
					"report to admin if you want this fixed..", vnum);
			}
		}
    }
	
	// boots
    if ( ( obj = get_eq_char( ch, WEAR_FEET ) ) == NULL ){
		vnum=OBJ_VNUM_OUTFIT_BOOTS;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_FEET );
			equipped=true;
			ch->println("You have mysteriously been equipped with something for the feet.");	
		}else{
			logf("do_outfit(): couldn't find boots vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit boots object vnum %d, "
					"report to admin if you want this fixed..", vnum);
			}
		}
    }

	// belt
    if ( ( obj = get_eq_char( ch, WEAR_WAIST ) ) == NULL ){
		vnum=OBJ_VNUM_OUTFIT_BELT;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_WAIST );
			equipped=true;
			ch->println("You have mysteriously been equipped with something for the waist.");	
		}else{
			logf("do_outfit(): couldn't find belt vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit belt object vnum %d, "
					"report to admin if you want this fixed..", vnum);
			}
		}
    }

    // do the weapon thing 
    if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
    {
		sn = 0;
		vnum = OBJ_VNUM_OUTFIT_SWORD; // just in case! 
		
		for (i = 0; weapon_table[i].name != NULL; i++)
		{
			if (weapon_table[i].gsn
				&& ch->pcdata->learned[sn] < ch->pcdata->learned[*weapon_table[i].gsn])
			{
				sn=*weapon_table[i].gsn;
				if(weapon_table[i].vnum_offset>=0){					
					vnum=*((int*)(((char*) game_settings)+(weapon_table[i].vnum_offset)));
				}
			}
		}
		
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object(objindex);
			obj_to_char(obj,ch);
			equip_char(ch,obj,WEAR_WIELD);
			equipped=true;
			ch->println("You have mysteriously been equipped with a weapon.");	
		}else{
			logf("do_outfit(): couldn't find weapon object vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit weapon object vnum %d, "
					"report to admin if you want this fixed.", vnum);
			}
		}
    }
	
    if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
		||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) 
		&&  (obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL 
		&&  (obj = get_eq_char( ch, WEAR_SECONDARY) ) == NULL )		
    {
		vnum=OBJ_VNUM_OUTFIT_SHIELD;
		objindex=get_obj_index(vnum);
		if(objindex){
			obj = create_object( objindex);
			obj->cost = 0;
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEAR_SHIELD );
			equipped=true;
			ch->println("You have mysteriously been equipped with a shield.");	
		}else{
			logf("do_outfit(): couldn't find shield vnum %d", vnum);
			if(GAMESETTING4(GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS)){
				ch->wraplnf("We can't seem to find the outfit shield object vnum %d, "
					"report to admin if you want this fixed.", vnum);
			}
		}
    }	

	if(!equipped){	    
		ch->println("You already have all the equipment that outfit can give you.");
	}
}
/**************************************************************************/
// displays the clan banking log
void do_checkclanbank( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];

    BUFFER *output;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("CLANBANKING INFO"));
	add_buf(output,buf);
	
	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail -n 10 " CLANBANKING_FILE);
		add_buf(output, "\r\n    You can select the number of loglines, type checkclanbank <number of lines>");
	
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail -n 10 " CLANBANKING_FILE);
		}
		else
		{
			sprintf(buf, "tail -n %d " CLANBANKING_FILE" | head -n 40", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the error "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail -n 10 " CLANBANKING_FILE);
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}
/**************************************************************************/
void do_continue( char_data *ch, char *argument )
{
	// do nothing, this function is used to absorb the word continue
	return;
}
/**************************************************************************/
void do_hlook( char_data *ch, char *argument )
{
	char arg[MIL];
	char_data *victim;
	
	one_argument( argument, arg );

	if(!IS_IMMORTAL(ch)){
		victim=ch;
	}else if(IS_NULLSTR(arg)){
		ch->println("Use `=Chlook self`x to view your own history.");
		ch->println("Use `=Chlook <playername>`x  to view anothers history.");
		return;
	}else if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL ){
		ch->printlnf("'%s' isn't here.", arg);
		return;
	}
	
	if ( IS_NPC(victim) ){
		ch->println("Not on NPC's.");
		return;    
	}
	
	// display the info 
    ch->println("`s=============================================================================`x");	  
	
	ch->printlnf("`=rPlayer Name: `=R%s`x", victim->name);
	ch->printlnf("`=rCharacter History: `=R%s%s`x",
		IS_NULLSTR(victim->pcdata->history)? "" : "\r\n", 
		IS_NULLSTR(victim->pcdata->history)?  "(none)":victim->pcdata->history);
	ch->println("`s=============================================================================`x");	  
	
	
}
/**************************************************************************/
/**************************************************************************/

