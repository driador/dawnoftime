/**************************************************************************/
// act_comm.cpp - primarily code relating to player communications
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
#include "intro.h"
#include "channels.h"
#include "msp.h"
#include "pload.h"

// command procedures needed
DECLARE_DO_FUN(do_quit	);
DECLARE_DO_FUN(do_amote	);
DECLARE_DO_FUN(do_pmote	);
DECLARE_DO_FUN(do_smote	);
DECLARE_DO_FUN(do_flee	);
void saymote( language_data *language, char_data *ch, char *argument, int sayflags);

void laston_player_deleting(char_data * ch);
void quit_char(char_data *ch, const char *argument, bool character_deleting );
char_data* find_innkeeper(char_data* ch);

/**************************************************************************/
void do_delet( char_data *ch, char *)
{
	 ch->println( "You must type the full command to delete yourself." );
}
/**************************************************************************/
void do_delete( char_data *ch, char *argument)
{
	char strsave[MIL];
	char deletedpfilename[MSL];
	bool remove_pfile = false;
	
	if (IS_NPC(ch))
		return;
	
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}
	
	if (ch->in_room->vnum==ROOM_VNUM_JAIL)
	{
        ch->println( "Deleting in this room is not an option, if you really want to be deleted,\r\n"
			"send a note to admin asking so, and then logoff." );
		return;
	}
	
	if (ch->pcdata->confirm_delete)
	{
		if (argument[0] != '\0')
		{
			ch->println( "Delete status removed." );
			ch->pcdata->confirm_delete = false;
			return;
		}
		else
		{
			strcpy( strsave, pfilename( ch->name, get_pfiletype(ch)));
			sprintf(deletedpfilename, "%s%s", DELETE_DIR, pfile_filename(ch->name)); 

			wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
			
			if (ch->level < 5){
				remove_pfile = true;
			}
			
			laston_player_deleting(ch);				// remove character from laston list
			
			intro_player_delete(ch);		// remove them from the intro database

	
			quit_char(ch, "", true);		// let character delete
			
			// on dedicated pkill muds, players just use the delete 
			// command to logout when they have no quit timers...
			// so their pfile is not deleted.
            if(!GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD))
            {
                if (remove_pfile){
                    unlink(strsave);         // delete player file
				}else{ // move them to the delete directory
					if(rename(strsave,deletedpfilename)!=0){
						char errbuf[MSL];
						sprintf(errbuf,"do_delete(): error moving pfile %s to the delete directory (%s), error %d (%s)", 
							strsave, 
							DELETE_DIR,
							errno, 
							strerror(errno));
						log_string(errbuf);
						bug(errbuf);
						autonote(NOTE_SNOTE, "do_delete()", "error moving pfile to the deleted directory!", "imm", errbuf, true);
					}
				};
            }
			return;
		}
	}
	
    if (IS_NULLSTR(argument)){
		ch->println( "Just type delete. No argument." );
		return;
    }
	
    ch->println( "Type delete again to confirm this command." );
    ch->println( "WARNING: this command is irreversible." );
	ch->println( "Typing delete with an argument will undo delete status." );
	ch->pcdata->confirm_delete = true;
	wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}
	    

/**************************************************************************/
// RT code to display channel status
void do_channels( char_data *ch, char *)
{
	char buf[MSL];
	
	/* lists all channels and their status */
	ch->println( "   channel     status" );
	ch->println( "---------------------" );
	ch->printlnf( "`s%-15s%s", "Newbie", HAS_CONFIG(ch, CONFIG_NONEWBIE)?"OFF":"ON");
	ch->printlnf( "`c%-15s%s", "OOC", HAS_CHANNELOFF(ch, CHANNEL_OOC)?"OFF":"ON");
	ch->printlnf( "`g%-15s%s", "Q/A", HAS_CHANNELOFF(ch, CHANNEL_QA)?"OFF":"ON");
	if (IS_IMMORTAL(ch))
	{
		ch->printlnf( "`G%-15s%s", "Immtalk", HAS_CHANNELOFF(ch, CHANNEL_IMMTALK)?"OFF":"ON");
	}
	ch->printlnf( "`g%-15s%s`x", "Quiet mode", HAS_CHANNELOFF(ch, CHANNEL_QUIET)?"ON":"OFF");
	if (IS_SET(ch->comm,COMM_AFK))
		ch->println( "You are AFK." );
	
	if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
		ch->println( "You are immune to snooping." );
	
	if (ch->lines != PAGELEN)
	{
		if (ch->lines){
			ch->printlnf( "You display %d lines of scroll.", ch->lines+2 );
		}else{
			ch->println( "Scroll buffering is off." );
		}
	}
	
	if (!IS_NULLSTR(ch->prompt))
    {
		sprintf(buf,"Your current prompt is: %s",ch->prompt);
		ch->printlnbw(buf);
    }
	
	if (!IS_NULLSTR(ch->olcprompt))
	{
		sprintf(buf,"Your current olc prompt is: %s",ch->olcprompt);
		ch->printlnbw(buf);
	}
	
	if (IS_SET(ch->comm,COMM_NOSHOUT))
		ch->println( "You cannot yell." );
	
    if (IS_SET(ch->comm,COMM_NOTELL))
		ch->println( "You cannot use tell." );
	
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
		ch->println( "You cannot use channels." );
	
	if (IS_SET(ch->comm,COMM_NOEMOTE))
		ch->println( "You cannot show emotions." );
	
}

/**************************************************************************/
// quiet blocks out all communication 
void do_quiet ( char_data *ch, char *)
{
	if (HAS_CHANNELOFF(ch, CHANNEL_QUIET))
	{
		ch->println( "Quiet mode removed." );
		REMOVE_CHANNELOFF(ch, CHANNEL_QUIET);
	}
	else
	{
		ch->println( "From now on, you will only hear says and emotes." );
		SET_CHANNELOFF(ch, CHANNEL_QUIET);
	}
}

// prototype

#if defined(unix)
const   char    echo_off_str    [] = { '\0', '\0', '\0' };
const   char    echo_on_str [] = { '\0' };
const   char    go_ahead_str    [] = { '\0' };
/*const   char    echo_off_str  [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str   [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str  [] = { IAC, GA, '\0' };
*/
#endif

/**************************************************************************/
void do_nspeak( char_data *ch, char *argument)
{
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}

	if(IS_NPC(ch) || ch->pcdata->diplomacy==0)
	{
		ch->println( "You must be a noble to speak." );
		return;
	}

	if (IS_NULLSTR(argument))
	{
		ch->println( "What do you wish to proclaim?" );
		return;
	}

	ch->printlnf( "`YYou Noble Speak: '%s'`x", argument);
	broadcast(ch,"`Y<noble:> '%s`Y'`x\r\n", argument);
}
	
/**************************************************************************/
// by Kalahn - Sept 98
void do_ntalk( char_data *ch, char *argument )  
{
    char_data *nch, *victim;

	// unswitched mobs can't ooc
	if (IS_UNSWITCHED_MOB(ch))
	{
		ch->println( "Players or switched players only." );
		return;
	}

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}

	if (IS_NULLSTR(argument) )
	{
		ch->println( "What do you wish to ntalk?" );
		return;
	}

	if (IS_SET(TRUE_CH(ch)->comm,COMM_NOCHANNELS))
	{
		ch->println( "The gods have revoked your channel privileges." );
		return;
	}

	ch->printlnf( "`=NYou ntalk: `=n%s`x", argument);

	// log all ntalk to a single file
	// Let's see what they say when we're not there :)
	{
		char logbuf[MSL];
		sprintf(logbuf,"%s`x", argument);
		append_datetime_ch_to_file( ch, NTALK_LOGFILE, logbuf);
	}

    for ( nch = player_list; nch; nch = nch->next_player )
    {
		if (TRUE_CH(ch)==nch){
			continue;
		}

		victim=nch;
		// ntalk going thru switch
		if (victim->controlling && victim->controlling->desc)
		{
			victim=victim->controlling;
		}

		if (IS_NOBLE(nch)){
			if (!IS_NOBLE(ch))
			{
				victim->printlnf( "`=N<%s [not a noble] noble talks>: `=n'%s`=n'`x", 
					TRUE_CH(ch)->name, argument);
			}else{
				if (IS_IMMORTAL(nch)){
					victim->printlnf( "`=N<%s noble talks>: `=n'%s`=n'`x",
						TRUE_CH(ch)->name, argument);
				}else{
					victim->printlnf( "`=N<noble talk>: `=n'%s`=n'`x", argument);
				}
			}
		}
	}
}
/**************************************************************************/
void do_afk ( char_data *ch, char *argument)
{
    // unswitched mobs can't be afk
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("players only sorry");
        return;
    }

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED )){
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}

	if(!IS_NULLSTR(argument)){
		// this makes it so you can change your afk message without turning afk off
		REMOVE_BIT(TRUE_CH(ch)->comm,COMM_AFK);
	}
	
    if (IS_SET(TRUE_CH(ch)->comm,COMM_AFK))	{
		act("$n has returned from being AFK.",ch,NULL,NULL,TO_ROOM);
		ch->println( "AFK mode removed. Type 'replay' to see tells." );
		REMOVE_BIT(TRUE_CH(ch)->comm,COMM_AFK);
	}else{
		if(IS_NULLSTR(argument)){
			replace_string(TRUE_CH(ch)->pcdata->afk_message, "");
		}else{
			// idiot checks
			if(c_str_len(argument)==-1){
				ch->println("You can't have newline colour codes in your afk message");
				return;
			}
			if(c_str_len(argument)>50){
				ch->println("Your afk message can't be more than 50 visible characters.");
				return;
			}

			// set their afk away message
			replace_string(TRUE_CH(ch)->pcdata->afk_message, argument);
		}
		if(IS_NULLSTR(argument)){
			act("$n just went AFK.",ch,NULL,NULL,TO_ROOM);
			ch->println( "You are now in AFK mode.");
		}else{
			act("$n just went AFK ($t).",ch,argument,NULL,TO_ROOM);
			ch->printlnf( "You are now in AFK mode (%s).", argument );
		}
		SET_BIT(TRUE_CH(ch)->comm,COMM_AFK);
	}
}

/**************************************************************************/
void do_requestooc (char_data *ch, char *argument)
{
    char_data *victim;

    if (!IS_IMMORTAL(ch) && !IS_OOC(ch)){
        ch->println( "You must be in the chat rooms to use requestooc." );
        return; 
    }

    if (IS_NULLSTR(argument)){
        ch->println( "Request for whom to go to ooc chat rooms." );
        return; 
    }

	victim=get_whovis_player_world(ch, argument);
    if(!victim){
        ch->println( "They are not playing." );
        return;
    }

	if (IS_OOC(victim)){
		ch->printlnf("They are already in OOC!");
		return;
	}

    ch->printlnf( "`=OSending ooc chat request to %s.`x", victim->name);
    victim->wraplnf("`=O%s has requested that you go to the ooc chat "
        "rooms to talk to them.`x", ch->name);
    victim->println("`=OYou can get there by typing `=Cgoooc`x." );
}

/**************************************************************************/
void do_pray( char_data *ch, char *argument )
{
	char buf[MSL];

	if(IS_NULLSTR(argument)){
		ch->println( "Pray What?" );
		return;
	}

	ch->printlnf( "`MYou pray to any who would listen: '%s'`x",	argument);
	// put it on the prayers wiznet channel
	if (!IS_SET(ch->comm, COMM_NOPRAY)){
		sprintf(buf,"`m%s prays '%s`m' [room %d](%s)`x\r\n", 
			ch->name, argument, ch->in_room?ch->in_room->vnum:0,
			position_table[ch->position].name);
		wiznet(buf,NULL,NULL,WIZ_PRAYERS_DREAMS,0,AVATAR); 
	};
	
	for(char_data *c=player_list; c; c = c->next_player){
		if( c != ch && ch->in_room==get_room_index(c->temple))
		{
			if(!IS_NPC(ch)){
				ch->pcdata->did_ic=true;
			}
			act_new("`M$n prays '$t'`x",ch, argument,c,TO_VICT,POS_DEAD);
		}
	}
}
/**************************************************************************/
void do_sayto( char_data *ch, char *argument )
{
    char arg[MSL], buf[MSL];
	char_data *victim;

    argument = one_argument(argument,arg);
    
    if (IS_NULLSTR(arg))
    {
        ch->println( "Sayto whom what?" );
		ch->println( "note: you can actually use normal say and put a > symbol \r\n"
			"followed by a players name as an abbreviation of say to...\r\n"
			"eg `=C '>kal hello `x or `=C say >kalahn hello`x" );
        return;
    }

	victim = get_char_room( ch, arg);
	if (!victim)
	{
		ch->printlnf( "You can't find '%s' to 'sayto'", arg );
		return;
	}

	if (victim==ch)
	{
		ch->println( "You can't direct sayto at yourself." );
		return;
	}

	sprintf(buf,">%s %s", arg, argument);
    saymote( ch->language, ch, buf, 0);
}
/**************************************************************************/
void do_say( char_data *ch, char *argument )
{
	int sayflags=0;
	if(HAS_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE)){
		SET_BIT(sayflags, SAYFLAG_NO_SAYMOTE);
	}
	if(HAS_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES)){
		SET_BIT(sayflags, SAYFLAG_CONVERT_COLOUR);
	}

	saymote( ch->language, ch, argument, sayflags);	
}
/**************************************************************************/
void do_saymote( char_data *ch, char *argument )
{
    saymote( ch->language, ch, argument, 0);
}
/**************************************************************************/
void do_rsay( char_data *ch, char *argument )
{
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}

	if(!IS_NPC(ch)){
		if(ch->pcdata->security<1){
			ch->println("rsay is for mudprogs only, it is a say seen only to the person who is remembered.");
		}else{
			ch->println("rsay is for use in mudprogs only, it is short for 'remembered say'.`1"
				"A mob must first 'mob remember $n', then 'rsay whatever text' will only be heard by $q.");
			return;
		}
		return;
	}
    saymote( ch->language, ch, argument, A);
}

/**************************************************************************/
// Kal July99
void do_saycolour(char_data *ch, char *argument)
{
	char arg[MSL];
	char newcol;
    argument = first_arg(argument,arg, false); // first_arg keeps case
    
    if (IS_NULLSTR(arg))  
	{
        ch->println( "`xUse saycolour to set your default colour when talking.");
        ch->println( "syntax: saycolour <single color code character>");
		ch->println( "e.g. `=Csaycol G`x would make all the words you spoke to be `Ggreen`x");
		return;
    }

	if(str_len(arg)>1)
	{
		ch->println( "saycolour: You can't have more than a single character for a colour code!");
		return;
	}

	newcol=arg[0];
	{// check it is a valid code
		char buf[MSL];
		sprintf(buf, "`%c", newcol);
		if (c_str_len(buf)!=0){
			ch->println( "saycolour: You can't have a colour code that is a control code!");
			return;
		}else{
			ch->printlnf("say colour code set to '%c'", newcol);
			ch->saycolour=newcol;
		}
	}
}
/**************************************************************************/
// Kal July99
void do_motecolour(char_data *ch, char *argument)
{
	char arg[MSL];
	char newcol;
    argument = first_arg(argument,arg, false); // first_arg keeps case
    
    if (IS_NULLSTR(arg))  
	{
        ch->println( "`xUse motecolour to set your default colour of motes in saymotes." );
        ch->println( "syntax: motecolour <single color code character>" );
		ch->println( "e.g. `=Cmotecol G`x would make all the motes in your saymotes to be `Ggreen`x" );
		return;
    }

	if(str_len(arg)>1)
	{
		ch->println( "motecolour: You can't have more than a single character for a colour code!");
		return;
	}

	newcol=arg[0];
	{// check it is a valid code
		char buf[MSL];
		sprintf(buf, "`%c", newcol);
		if (c_str_len(buf)!=0){
			ch->println( "motecolour: You can't have a colour code that is a control code!");
			return;
		}else{
			ch->printlnf( "mote colour code set to '%c'", newcol);
			ch->motecolour=newcol;
		}
	}
}
/**************************************************************************/
// Kalahn August 97 
void do_whisper( char_data *ch, char *argument )
{
    char_data *victim, *overhear;
    char target[MIL];
	char message[MSL];

	// don't allow messages that could be defrauding 
	if(check_defrauding_argument(ch, argument)){
		return;
	}

    // wiznet whispers NO MOBS
	if( !IS_UNSWITCHED_MOB(ch )) {
		wiznet(FORMATF("whisper %s: \"%s\"", ch->name, argument), 
			ch ,NULL,WIZ_WHISPERS,0,get_trust(ch));
	}

    argument = one_argument( argument, target);

    if( IS_NULLSTR(target)){
        ch->println( "Whisper to whom?" );
        return;
    }

    if(!str_cmp(target,"all")){
        ch->println( "If you are going to tell everyone just use say." );
        return;
    }

    // find the person in the room to whisper them 
    victim = get_char_room( ch, target);
    if(!victim){
        ch->printlnf( "You can't find '%s' to whisper to", target);
        return;
    }

    if (victim == ch){
        ch->println( "If you start whispering to yourself, people will think you're strange." );
        return;
    }

    if(!IS_AWAKE(victim)){
        ch->printlnf( "You had better wake '%s' first.", PERS(victim, ch));
        return;		
    }

    argument = ltrim_string(argument);
    if(IS_NULLSTR(argument)){
        ch->printlnf( "What do you want to whisper to %s?", PERS(victim, ch));
        return;
    }

    // record the whisper as ic or ooc, or maybe both 
	if(!IS_NPC(ch)){
		ch->pcdata->did_ic=true;
        if(room_is_private(ch->in_room)){
            ch->pcdata->did_ooc=true;
		}
	}

    // go thru all in the room - whispering
    // to target, and those who over hear 
    for ( overhear=ch->in_room->people; overhear; overhear = overhear->next_in_room )
    {
        if ( overhear == ch || !IS_AWAKE(overhear)){
			continue;
		}

        translate_language(ch->language, true, ch, overhear, argument, message);

        // direct it to whoever is necessary 
        if(overhear == victim){
			RECORD_TO_REPLAYROOM=true;
			if ( !is_affected( victim, gsn_deafness ) || IS_OOC(victim)) {
	            act("`x$n whispers to you '$t`x'", ch, message, victim, TO_VICT);
			}
            act("`xYou whisper '$t`x' to $N`x.", ch, argument, victim,TO_CHAR);
			RECORD_TO_REPLAYROOM=false;
			continue;
        }


        if (IS_AWAKE(overhear) && IS_TRUSTED(overhear, INVIS_LEVEL(ch)))
        {
			// the higher the chance value, the more likely 'overhear' will hear
            int chance = 45 - ch->perm_stats[STAT_IN]/3; // approx range 45 -> 75 
            chance -= ch->modifiers[STAT_IN]/3;      // can take it up to > 100 
            chance += overhear->modifiers[STAT_QU]/4;
            chance += overhear->modifiers[STAT_IN]/3;

			// awareness helps you overhear
			chance += get_skill(overhear,gsn_awareness)/5;
            
			chance-= number_range(1,100);

            if (( chance > 0
			|| IS_IMMORTAL(overhear)
			|| is_affected(overhear, gsn_augment_hearing ))) // overheard 
            {
				if ( IS_OOC(overhear) || !is_affected( overhear, gsn_deafness) ) {
		            overhear->printlnf("`sYou overhear %s whispering '%s`x' to %s`x.",
						PERS(ch, overhear), message, PERS(victim, overhear));
					overhear->record_replayroom_event(
							FORMATF("`sYou overhear %s whispering '%s`x' to %s`x.",
								PERS(ch, overhear), message, PERS(victim, overhear))
						);
				}
            } else if (chance+45 > 0){ // notice but not hear 
				overhear->printlnf("`xYou notice %s whispering to %s`x.",
					PERS(ch, overhear), PERS(victim, overhear));
				overhear->record_replayroom_event(
						FORMATF("`xYou notice %s whispering to %s`x.",
							PERS(ch, overhear), PERS(victim, overhear))
					);
			}
        }
	}

    return;
}

/**************************************************************************/
// return true if for any reason if the sender can't send a tell
// - this function sends a message to the sender stating the reason
bool tell_cant_send_tell(char_data *ch)
{
    // unswitched mobs can't send tells
    if (IS_UNSWITCHED_MOB(ch))
    {
		ch->println( "Uncontrolled mobs can't tell." );
		return true;
    }

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master ){
			ch->master->println( "Not going to happen." );
		}
		ch->println( "You can't be ordered to send tells." );
		return true;
	}

    if(IS_SET(TRUE_CH(ch)->comm, COMM_NOTELL) ){
		ch->println( "You are not allowed to send tells right now." );
		return true;
	}

	return false;
}
/**************************************************************************/
void deliver_tell(char_data *ch, const char * fmt, ...)
{
	assert(!IS_NPC(ch));
	char_data *target=ch->controlling?ch->controlling:ch;

    char buf[MSL];
	va_list args;
	va_start (args, fmt);
	vsnprintf (buf, MSL, fmt, args);
	va_end (args);

	char recordbuf[MSL];
	sprintf(recordbuf, "%s> %s",
		shorttime(NULL),
		buf);

	replace_string(	// record it in their replay buffer
		ch->pcdata->replaytell_text[ch->pcdata->next_replaytell], 
		recordbuf);
	++ch->pcdata->next_replaytell%=MAX_REPLAYTELL;
	if(HAS_CONFIG(ch, CONFIG_AUTOWRAPTELLS)){
		target->wrapln(buf); // send to the character
	}else{
		target->print(buf); // send to the character
	}
}
/**************************************************************************/
enum sendtext_type {ST_TELL, ST_RETELL, ST_REPLY};
/**************************************************************************/
// - Kalahn, Jan 00
void tell_sendtext(char_data *from, char_data *to, char *text, sendtext_type type)
{
	assertp(from);
	assertp(to);
	// get the characters they are connected to 
	// - so messages can be transparent to switch etc
	char_data *sender=from->controlling?from->controlling:from;

	if(IS_NPC(from) || IS_NPC(to)){ // TRUE_CH() must be used in the calling function
		sender->println("Tells can't be exchanged with non playing characters.");
		return;
	} 

	if(channel_colour_disabled(to,CHANNEL_TELLS)){
		text=strip_colour(text);
	}
		
	// support restricting tells between players
	if(GAMESETTING2(GAMESET2_TELL_RESTRICTIONS))
	{
		if( !(IS_IMMORTAL(from) || IS_IMMORTAL(to)
			||(IS_NEWBIE_SUPPORT(from) && !IS_LETGAINED(to)) 
			||(IS_NEWBIE_SUPPORT(to) && !IS_LETGAINED(from))))
		{
			if (IS_NEWBIE_SUPPORT(from)){
				sender->println( "Tells may only be exchanged with an immortal or new players.");
			}else if (!IS_LETGAINED(from)){
				sender->println( "`xTells may only be exchanged with an immortal or newbie support players\r\n"
					"(i.e. has +++ in who).\r\n"
					"If you want to talk to another player about Out of Character (OOC) way\r\n"
					"you can use goto the ooc rooms using the `=Cgoooc`x command, and then request\r\n"
					"using the `=Crequestooc`x command that the person you are wanting to talk meets\r\n"
					"meets you in the ooc rooms.");
			}else{
				sender->println( "Tells may only be exchanged with an immortal." );
			}
			return;
		}
	}

	// get the name of the person the message is being sent to from 
	// the senders perspective
	char to_name[MIL];
	char to_capname[MIL];
	int to_gender;
	bool to_unknown;
	if(type==ST_REPLY && IS_SET(from->dyn,DYN_UNKNOWN_REPLY_NAME)){
		to_unknown=true;
		strcpy(to_name,"someone"); // the default name
		to_gender=SEX_NEUTRAL;
	}else{ 
		to_unknown=false;
		strcpy(to_name, mxp_create_tag(from, FORMATF("ch-uid_name %d", to->uid), capitalize(to->name)));
		to_gender=URANGE(0, to->pcdata->true_sex, 2);
	}
	strcpy(to_capname,capitalize(to_name));

	// leave imms in peace and quiet from mortal tells :)
    if( HAS_CHANNELOFF(to, CHANNEL_QUIET) && !IS_IMMORTAL(from)) {
        sender->printlnf( "%s is not receiving tells.", to_name);
        return;
    }

	// check for nothing to say - including empty colour codes
    if ( IS_NULLSTR(text) || c_str_len(text)==0) {
        sender->printlnf( "%s %s what?",
			(type==ST_TELL)?"Tell":	((type==ST_RETELL)?"Retell":"Reply to"), to_name);
		return;
	}

	// If the sender knows the name of the person they are talking to or they can't 
	// see the person they are talking to on the wholist, they can be informed
	// of the linkdead status of the person they are speaking with
    if ( !(to->controlling?to->controlling:to)->desc 
		  && (!to_unknown || !can_see_who(from,to)))
    {
		sender->wraplnf("%s seems to have misplaced %s link... "
			"the tell will be recorded in %s replay which they will "
			"automatically see if they reconnect.",
			to_capname, his_her[to_gender], his_her[to_gender]);
    }

    // keep a track of what newbie support is doing
    if ( (IS_NEWBIE_SUPPORT(from) && !IS_LETGAINED(to))
        ||(IS_NEWBIE_SUPPORT(to) && !IS_LETGAINED(from)) )
    {
        char tbuf[MSL];
		char nbuf[MSL];
        sprintf (tbuf,"Tell %s %s", to->name, text);
        append_newbie_support_log(from, tbuf);
		if ( !IS_IMMORTAL( from ) && !IS_IMMORTAL( to )) {
			sprintf (nbuf, "`W%s newbietells %s '%s'", 
				from->name, to->name, text);
			wiznet(nbuf,from,NULL,WIZ_NEWBIETELL,0,LEVEL_IMMORTAL);
		}
	}

	// get the name of the person sending the message 
	// from the receivers perspective
	char from_name[MIL];
	bool from_unknown=true;;
	int from_gender;	
	switch(type){
	case ST_TELL:	// 'new conversation' sender known if can be seen on the wholist
		if(can_see_who(to, from) || IS_SET(from->dyn, DYN_USING_KTELL) ){
			from_unknown=false;
			REMOVE_BIT(from->dyn, DYN_HAS_ANONYMOUS_RETELL);
		}else{
			SET_BIT(from->dyn, DYN_HAS_ANONYMOUS_RETELL);
		}
		break;

	case ST_REPLY:	// For the sender to be replying, the receiver has to have 
		// sent the sender a tell earlier, therefore the receiver could see them 
		// then, so this conversation isn't anonymous
		from_unknown=false;
		break;

	case ST_RETELL: // determined on the anonymous status of original tell
		if(!IS_SET(from->dyn, DYN_HAS_ANONYMOUS_RETELL)){
			from_unknown=false;
		}
		break;
	}
	if(from_unknown){
		strcpy(from_name,"Someone"); // name unknown to the receiver
		from_gender=SEX_NEUTRAL;
	}else{	
		strcpy(from_name, mxp_create_tag(to, FORMATF("ch-uid_name %d", from->uid), capitalize(from->name)));
		from_gender=URANGE(0, from->pcdata->true_sex, 2);
	}

	// send the messages
	deliver_tell( from, "%sYou %s%s %s '%s%s'`x\r\n", 
		(type==ST_REPLY?"`=M":"`=m"),
		(from_unknown?"anonymously ":""),
		(to_unknown && type==ST_REPLY)|| !can_see_who(from, to)? mxp_create_tag(from, "tl_rp", "tell"):
			mxp_create_tag(from, (FORMATF("%s %s", from->retell==to?"tl-nm_rp_tlnm":"tl-nm_rt_tlnm", to->name)), "tell"),
		to_name,
		text,
		(type==ST_REPLY?"`=M":"`=m"));

	deliver_tell(to, "%s%s %s you %s'%s%s'`x\r\n", // tells you
		(to->retell!=from || from_unknown?"`=M":"`=m"),
		from_name,
		(from_unknown || !can_see_who(to, from))?mxp_create_tag(to, "tl_rp", "tells"):
			mxp_create_tag(to, (FORMATF("%s %s", to->retell==from?"tl-nm_rt_tlnm":"tl-nm_rp_tlnm", from->name)), "tells"),
		(to_unknown?"(someone) ":""),
		text,
		(to->retell!=from?"`=M":"`=m"));

	// reply locks onto all anonymous tells 
	// if the tell isn't anonymous then to those who you arent retelling you
	// also when the receiver doesn't have any current reply
	if(from_unknown || to->retell!=from || !to->reply){
		if(!to_unknown){
			to->reply=from;
		}
		if(from_unknown){
			SET_BIT(to->dyn,DYN_UNKNOWN_REPLY_NAME);
		}else{
			REMOVE_BIT(to->dyn,DYN_UNKNOWN_REPLY_NAME);
		}
	}

	if(type==ST_TELL){
		from->retell=to;
	}

	// inform the sender of receiver afk status on the message they just sent
	// (assuming it wont give away the identity of an imm that is a 'someone')
	if (IS_SET(to->comm,COMM_AFK) && (!to_unknown || !can_see_who(from,to)))
	{  
		from->wraplnf( 
			"%s is currently AFK and therefore might not have seen your message.  Your "
			"message has been displayed to their screen and recorded in %s tell replay buffer.", 
			to_capname,
			his_her[to_gender]);
    }
}
/**************************************************************************/
void do_tell( char_data *ch, char *argument )
{
    char arg[MIL];
	char_data *victim;
	
	if(tell_cant_send_tell(ch)){ 
		return;
	};

    if( HAS_CHANNELOFF(ch, CHANNEL_QUIET) ) {
        ch->println( "You must turn off quiet mode before sending a tell." );
        return;
    }

	// find out who the tell is going to
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->println( "Tell whom what?" );
        return;
    }

    // remove a , from the name field if required
	if (arg[str_len(arg)-1]==',')
        arg[str_len(arg)-1]=0;

    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL) 
    {
        ch->printlnf( "'%s' couldn't be found to send a tell to.", arg);
        return;
    }

	if(TRUE_CH(ch)==victim){
        ch->println( "Try talking to someone other than yourself." );
        return;
	}
	
	tell_sendtext(TRUE_CH(ch), victim, argument, ST_TELL);
}
/**************************************************************************/
void do_atell( char_data *ch, char *argument )
{
	if(IS_NULLSTR(argument)){
		ch->println( "atell <who> what you want to say - anonymous tell" );
		return;
	}
	int whovis = IS_SET(TRUE_CH(ch)->comm, COMM_WHOVIS);
    REMOVE_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);

	do_tell(ch, argument);

	if (whovis)
	{
		SET_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);
	}
}
/**************************************************************************/
void do_ktell( char_data *ch, char *argument )
{
	if(IS_NULLSTR(argument)){
		ch->println( "ktell <who> what you want to say - known tell" );
		return;
	}

	SET_BIT(TRUE_CH(ch)->dyn, DYN_USING_KTELL);
	do_tell(ch, argument);
	REMOVE_BIT(TRUE_CH(ch)->dyn, DYN_USING_KTELL);
}
/**************************************************************************/
void do_retell( char_data *ch, char *argument )
{
	if(tell_cant_send_tell(ch)){ 
		return;
	};

	if (!TRUE_CH(ch)->retell)
    {
		ch->println( "They aren't here." );
        ch->println( "(you have to send someone a tell before using retell.)" );
        return;
    }
	tell_sendtext(TRUE_CH(ch), TRUE_CH(ch)->retell, argument, ST_RETELL);
}

/**************************************************************************/
void do_reply( char_data *ch, char *argument )
{
	if(tell_cant_send_tell(ch)){ 
		return;
	};
	
    if ( TRUE_CH(ch)->reply == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

	tell_sendtext(TRUE_CH(ch), TRUE_CH(ch)->reply, argument, ST_REPLY);
}
/**************************************************************************/
struct sectyellinfo
{
	short sectindex;
	float yellreduction; // amount of yell remaining after the sound 
						 // travelled thru the room
};

// return the id of the next yell
time_t get_yell_id(void)
{
	static int lastyellid=0;
	lastyellid++;
    return lastyellid;
}

sectyellinfo sectyelltable[]={
	{SECT_INSIDE		,(float)0.90}, // reduction amounts on the high side
	{SECT_CITY			,(float)0.85}, // to make the yell command actually useable
	{SECT_FIELD			,(float)0.75},
	{SECT_FOREST		,(float)0.70},
	{SECT_HILLS			,(float)0.70}, 
	{SECT_MOUNTAIN		,(float)0.65}, 
	{SECT_WATER_SWIM	,(float)0.55},
	{SECT_WATER_NOSWIM	,(float)0.45},
	{SECT_SWAMP			,(float)0.45},
	{SECT_AIR			,(float)0.65},
	{SECT_DESERT		,(float)0.65},
	{SECT_CAVE			,(float)0.80},
	{SECT_UNDERWATER	,(float)0.35},
	{SECT_SNOW			,(float)0.80},
	{SECT_ICE			,(float)0.80},
	{SECT_TRAIL			,(float)0.75},
	{SECT_LAVA			,(float)0.40},
};
/**************************************************************************/
float get_yellreduction(short sector)
{
	if(sector<0 || sector>=SECT_MAX){
		bugf("get_yellreduction(): Out of range sector value %d!!!", sector);
		return (float)0.50; // use low value
	}

	if(sectyelltable[sector].sectindex==sector){
		return sectyelltable[sector].yellreduction;
	}else{ // gotta go look for it
		bugf("get_yellreduction(): Table out of order for index %d, sectyelltable[sector].sectindex=%d!!", 
			sector, sectyelltable[sector].sectindex);
		return (float)0.65; // use low value
	}
}
/**************************************************************************/
void recurse_yell(ROOM_INDEX_DATA *to_room, float amplitude, 
				  int direction, time_t yellindex)
{
	if(!to_room){
		bug("recurse_yell(): to_room==NULL!");
		return;
	}

	// reduce the amplitude of the yell to travel this room
	amplitude*=get_yellreduction(to_room->sector_type);

	// yell has died out
	if(amplitude<5.0){
		return;
	} 

	// if the room we are currently in already has a yell that is greater 
	// or equal to our current amplitude, leave this path
	if(to_room->yellindex==yellindex && to_room->yell_amplitude>=amplitude){
		return; 
	}

	// add our yell here
	to_room->yellindex=yellindex;
	to_room->yell_amplitude=amplitude;
	
	to_room->yell_enteredindir=direction;

	// broadcast the yell in all directions
	int dir;
    for (dir=0;dir<MAX_DIR;dir++)
	{
		if(!to_room->exit[dir] || !to_room->exit[dir]->u1.to_room)
			continue;
		
		if(IS_SET(to_room->exit[dir]->exit_info,EX_CLOSED)){
			amplitude*=(float)0.55; // lose 45% going thru doors
		}	
		
		if(dir!=direction){
			amplitude*=(float)0.70; // lose 30% changing direction
		}	

		// yells don't bleed between OOC and IC
		if((to_room->exit[dir]->u1.to_room->room_flags&ROOM_OOC) 
			!=(to_room->room_flags&ROOM_OOC)){
			continue;
		}

		if(   !to_room->exit[dir]->u1.to_room->exit[rev_dir[dir]]
			|| to_room->exit[dir]->u1.to_room->exit[rev_dir[dir]]->u1.to_room!=to_room){
			// exits missed matched, wipe directional component
			recurse_yell(to_room->exit[dir]->u1.to_room, amplitude, -1, yellindex);
		}else{
			// send to the next room
			recurse_yell(to_room->exit[dir]->u1.to_room, amplitude, dir, yellindex);
		}
	}
}
/**************************************************************************/
char *get_yellreversedir(short dir, int amplitude)
{
	static char buf[MIL];

	if(dir==-1){
		if(amplitude>50){
			sprintf(buf,"from somewhere in the near surroundings");
		}else if (amplitude>40){
			sprintf(buf,"from somewhere not far from here");
		}else if (amplitude>30){
			sprintf(buf,"from somewhere in the near distance");
		}else if (amplitude>20){
			sprintf(buf,"from somewhere in the distance");
		}else if (amplitude>10){
			sprintf(buf,"from somewhere in the far distance");
		}else if (amplitude>4){
			sprintf(buf,"from somewhere in the far far distance");
		}else{
			sprintf(buf,"faintly");
		}
	}else{
		if(amplitude>50){
			sprintf(buf,"from somewhere in the near surroundings to the %s",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>40){
			sprintf(buf,"from somewhere not far from here to the %s",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>30){
			sprintf(buf,"from somewhere in the near distant %s",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>20){
			sprintf(buf,"from somewhere in the distant %s",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>10){
			sprintf(buf,"from somewhere in the far distance %s",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>5){
			sprintf(buf,"from somewhere in the far far distant %s",
				dir_name[rev_dir[dir]]);
		}else if (amplitude>2){
			sprintf(buf,"faintly from the %s", dir_name[rev_dir[dir]]);
		}else{
			sprintf(buf,"faintly");
		}
	}
	return buf;
}
/**************************************************************************/
bool ftp_reconnect(char *name);
/**************************************************************************/
void do_testgeneric(char_data *ch, char *)
{

	ch->titlebar("TEST GENERIC - Generic Testing Code");

	ch->titlebar("Yell test info");
	ch->printlnf( "Room vnum = %d",ch->in_room->vnum);
	ch->printlnf( "Yellindex = %d",(int)ch->in_room->yellindex);
	ch->printlnf( "Yell_enteredindir = %d",ch->in_room->yell_enteredindir);
	ch->printlnf( "Yell_amplitude    = %f",ch->in_room->yell_amplitude);

	if(ch->desc){
		ch->titlebar("Colour memory test dump");
		ch->printlnf("current = %c", ch->desc->colour_memory.current);
		ch->printlnf("saved index = %d", ch->desc->colour_memory.saved_index);
		for(int i=0; i<MAX_SAVED_COLOUR_ARRAY; i++){
			ch->printlnf("%d] saved =%c", i, ch->desc->colour_memory.saved[i]);
		}
	}

	// manual room count
	{
		int rcount=0;
		int i;
		ROOM_INDEX_DATA *pRoomIndex;

		for(i=0; i<MAX_KEY_HASH; i++){
			for ( pRoomIndex  = room_index_hash[i];
			pRoomIndex != NULL;
			pRoomIndex  = pRoomIndex->next )
			{		  
				rcount++;
			}
		}
		ch->printlnf("manual room count=%d", rcount);
	}

	if(HAS_MSP(ch)){
		ch->println("Testing MSP by sending quaff sound using msp_to_room()...");
		msp_to_room(MSPT_ACTION, MSP_SOUND_QUAFF, 
						0,
						ch,
						false,
						true);	
		ch->println("Test finished.");
	}else{
		ch->println("Currently your msp setting is disabled, so no msp test sound is sent to you.");
	}	

	{
		int a[10];
		memset(a, 0, sizeof(a));
		ch->titlebar("MOBILE HASH TABLE STATS");
		MOB_INDEX_DATA *pMobIndex;
		int vnum;
		int count;
		int max=0, max_position=0;
			
		for ( vnum = 0; vnum< MAX_KEY_HASH; vnum++ ){
			count=0;
			for ( pMobIndex  = mob_index_hash[vnum]; pMobIndex;	pMobIndex=pMobIndex->next )
			{
				count++;
				if(count>max){
					max=count;
					max_position=vnum;
				}
			}
			a[UMIN(count,9)]++;
			//ch->printlnf("%4d=%d",vnum, count);
		}
		ch->printlnf("MAX_KEY_HASH=%d", MAX_KEY_HASH);		
		for(vnum=0; vnum<10; vnum++){
			ch->printlnf("sum %2d=%d", vnum, a[vnum]);
		}
		ch->printlnf("max=%d, max_position=%d",max, max_position);
	}

	ch->println("Telling your dawnftp client to reconnect (if you have one).");
	ftp_reconnect(ch->name);
}
/**************************************************************************/
// Kal - July 99
void do_yell(char_data *ch, char *argument )
{
	char buf2[MSL];
	static int rpmon=0;

	char buf[MSL];
	time_t yellid= get_yell_id();
	float start_amp=race_table[ch->race]->high_size*(float)1.1;

	if ( IS_SET(ch->comm, COMM_NOSHOUT) )
	{
		ch->println( "You can't yell." );
		return;
	}
	
	if ( argument[0] == '\0' )
	{
		ch->println( "Yell what?" );
		return;
	}

	if(IS_SET(ch->in_room->room_flags, ROOM_NOSPEAK)
	&& !HAS_CONFIG(ch, CONFIG_HOLYSPEECH)) {
		ch->println( "You feel the air escape your lungs, and yet... no sound." );
		return;
	}
	
    // wiznet RPMONITOR 
    if (!IS_NPC(ch) && !IS_OOC(ch)) 
    {
        rpmon++;
        if (rpmon>=5)
        {
            sprintf (buf2, "Wiznet rpmonitor: %s yells '%s'`x", ch->name, argument);
            wiznet(buf2,ch,NULL,WIZ_RPMONITOR,0,get_trust(ch));
            rpmon=0;
        }
    }
	
    if(!IS_NPC(ch)){
        ch->pcdata->did_ic=true;
	}
				// fromroom ,  amplitude, no direction
	recurse_yell(ch->in_room, start_amp,  -1,		yellid);

	// now transmit to all connections what they heard
	{
		ROOM_INDEX_DATA *this_room;
		connection_data *d;     
		for ( d = connection_list; d; d = d->next )
		{
			if ( d->connected_state == CON_PLAYING 
				&& d->character && d->character->in_room
				&& IS_AWAKE(d->character))
			{
				this_room=d->character->in_room;
				if(this_room->yellindex==yellid){
					// they heard it					
					translate_language(ch->language, true, ch, d->character, argument, buf);
										
					if(this_room== ch->in_room)
					{
						act("$n yells '$t'`x",ch,buf,d->character,TO_VICT);
					}
					else
					{
						if(IS_IMMORTAL(CH(d)))
						{
							d->character->printlnf( "You hear a %s voice(%s) yell %s '%s', %f",
								(ch->sex==2?"female":"male"),
								ch->name,
								get_yellreversedir(this_room->yell_enteredindir,
								(int)this_room->yell_amplitude),
								buf, this_room->yell_amplitude);
						}
						else
						{
							if(IS_OOC(ch))
							{
								d->character->printlnf( "You hear a %s voice(%s) yell %s '%s'", 
									(ch->sex==2?"female":"male"), ch->name,
									get_yellreversedir(this_room->yell_enteredindir,
									(int)this_room->yell_amplitude), buf);
							}
							else
							{
								d->character->printlnf( "You hear a %s voice yell %s '%s'", 
									(ch->sex==2?"female":"male"),
									get_yellreversedir(this_room->yell_enteredindir,
									(int)this_room->yell_amplitude), buf);
							}
						}
					}
				}
			}
		}
	}
	act("You yell '$t'`x",ch,argument,NULL,TO_CHAR);
}

/**************************************************************************/
void do_emote( char_data *ch, char *argument )
{
	char buf2[MSL];
	static int rpmon=0;
	
	if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
	{
		ch->println( "You can't show your emotions." );
		return;
	}
	
	if ( argument[0] == '\0' )
	{
		ch->println( "Emote what?" );
		return;
	}

	// Integrated emote code 
	{
		int starcount = count_char(argument, '*');
		int atcount = count_char(argument, '@');
		
		if(starcount>0)
		{
			if( atcount>0){
				do_amote(ch,argument);
			}else{
				do_smote(ch,argument);
			}
			return;
		}
		if( atcount>0){
			do_pmote(ch,argument);
			return;
		}
		
	}

	if(!IS_OOC(ch))
	{
		// don't allow messages that could be defrauding 
		if(check_defrauding_argument(ch, argument)){
			return;
		}
	}

	// allow players to put a # at the start of an emote and 
	// it wont be counted towards rps, but can alias emotes to give
	// there character unique like socials
	bool rps_candiate=true;
	if(*argument=='#'){
		rps_candiate=false;
		argument++;
	};

    // wiznet RPMONITOR
    if (!IS_NPC(ch))
	{
		if (!IS_OOC(ch)) 
		{
			rpmon++;
			if (rpmon>=15)
			{
				sprintf (buf2, "Wiznet rpmonitor: %s emotes '%s`x'", ch->name, argument);
				wiznet(buf2,ch,NULL,WIZ_RPMONITOR,0,get_trust(ch));
				rpmon=0;
			}
			if(rps_candiate){
				ch->pcdata->did_ic=true;
			}
		}
		
		if(room_is_private(ch->in_room) && rps_candiate){ 
			ch->pcdata->did_ooc=true; 
			ch->pcdata->did_ic=false; 
		}
		
		// anti autorps abuse system
		if(IS_IC(ch) && rps_candiate)
		{
			++ch->pcdata->emote_index%=RPS_AUDIT_SIZE;
			free_string(ch->pcdata->emotecheck[ch->pcdata->emote_index]);
			ch->pcdata->emotecheck[ch->pcdata->emote_index]=str_dup(argument);
			ch->pcdata->emotetimes[ch->pcdata->emote_index]=current_time;
		}
	}
	
	MOBtrigger = false;
	RECORD_TO_REPLAYROOM=true;
	act( "$n `=\xc4$T`x", ch, NULL, argument, TO_ROOM );
	act( "$n `=\xc4$T`x", ch, NULL, argument, TO_CHAR );
	RECORD_TO_REPLAYROOM=false;
	MOBtrigger = true;
	
	return;
}

/**************************************************************************/
// written by Kalahn
void do_amote( char_data *ch, char *argument )
{
    char *letter;
    char act_text[MIL];
    char vict_text[MIL];

    char ch_arg[MIL];
    char_data *victim;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        ch->println( "You can't show your emotions." );
        return;
    }

    argument = one_argument( argument, ch_arg );

    if ( ch_arg[0] == '\0' || argument[0] == '\0')
    {
        ch->println( "Syntax: amote <at> <text>." );
        ch->println( "<at> is the target who the amote is direct at." );
        ch->println( "The text must include * for your name and a @ where you want the targets name." );
        ch->println( "eg amote guard Jumping from out of the shadows, * grabs @ by the neck." );
        return;
    }

    if (( victim = get_char_room( ch, ch_arg ) ) == NULL )
    {
        ch->printlnf( "You can't seem to find the player '%s'.", ch_arg);
        return;
    }

    if (victim == ch)
    {
        ch->println( "You can't direct amotes at yourself... use smote.");
        return;
    }

    if ( argument[0] == '\0' )
    {
        ch->printlnf( "Amote what towards %s.", PERS(victim, ch));
        return;
    }

	if ( (count_char(argument,'@') != 1) || (count_char(argument,'*') != 1) )
    {
        ch->println( "Syntax: amote <target> <text>." );
        ch->println( " The text must include a single * for your name and a single @ where you want the targets name." );
        ch->println( " * must appear before @." );
        ch->println( " eg amote guard Jumping from out of the shadows, * grabs @ by the neck." );
        return;
    }

    if (strstr(argument,"@") < strstr(argument,"*"))
    {
        ch->println( "AMOTE: * must appear before the @." );
        return;
    }

	// don't allow messages that could be defrauding 
	if(check_defrauding_argument(ch, argument)){
		return;
	}

	// allow players to put a # at the start of an emote and 
	// it wont be counted towards rps, but can alias emotes to give
	// there character unique like socials
	bool rps_candiate=true;
	if(*argument=='#'){
		rps_candiate=false;
		argument++;
	};

    act_text[0]= '\0';
    vict_text[0]= '\0';

    letter = argument;

    for (; *letter != '\0'; letter++)
    {
        if (*letter == '@')
        {
            if ( *(letter+1) == '\'' && *(letter+2) == 's')
            {
                strcat(act_text, "`#`=\xc4$N's`&");
                strcat(vict_text, "your");
                letter += 2;
            }
            else if ( *(letter+1) == 'r' || *(letter+1) == 's') 
            {
                strcat(act_text, "`#`=\xc4$N's`&");
                strcat(vict_text, "your");
                letter++;
            }
            else
            { 
                strcat(act_text, "`#`=\xc4$N`&");
                strcat(vict_text, "you");
            }
        }
        else if (*letter == '*')
        {
            strcat(act_text, "`#`W$n`&");
            strcat(vict_text, "`#`W$n`&");
        }
        else
        {
            strncat(act_text, letter,1);
            strncat(vict_text, letter,1);
        }
    }


    MOBtrigger = false;
    act( act_text,  ch, NULL, victim, TO_NOTVICT );
    act( act_text,  ch, NULL, victim, TO_CHAR );
    act( vict_text, ch, NULL, victim, TO_VICT );
    MOBtrigger = true;

	if(!IS_NPC(ch) && rps_candiate){
        ch->pcdata->did_ic=true;
		if(room_is_private(ch->in_room)){ 
			ch->pcdata->did_ooc=true; 
			ch->pcdata->did_ic=false; 
		}
	}

}


/**************************************************************************/
// modified by Kalahn to use @ in place of name and use act
void do_pmote( char_data *ch, char *argument )
{
    char *letter;
    char act_text[MIL];
    char vict_text[MIL];
	
    char ch_arg[MIL];
    char_data *victim;
	
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        ch->println( "You can't show your emotions." );
        return;
    }
	
    argument = one_argument( argument, ch_arg );
	
    if ( ch_arg[0] == '\0' || argument[0] == '\0')
    {
        ch->println( "Syntax: pmote <target> <text>." );
        ch->println( "The text must include a single @ where you want the targets name." );
        ch->println( "eg pmote guard smiles at @." );
        return;
    }
	
    if (( victim = get_char_room( ch, ch_arg ) ) == NULL )
    {
        ch->printlnf( "You can't seem to find the player '%s'.", ch_arg);
        return;
    }
	
    if (victim == ch)
    {
        ch->println( "You can't direct pmotes at yourself... use emote." );
        return;
    }
	
    if ( argument[0] == '\0' )
    {
        ch->printlnf( "Pmote what towards %s.", PERS(victim, ch));
        return;
    }
	
	if (count_char(argument,'@') != 1)
    {
        ch->println( "Syntax: pmote <target> <text>." );
        ch->println( "The text must include a single @ where you want the targets name." );
        ch->println( "eg pmote guard smiles at @." );
        return;
    }

	// don't allow messages that could be defrauding 
	if(check_defrauding_argument(ch, argument)){
		return;
	}

	// allow players to put a # at the start of an emote and 
	// it wont be counted towards rps, but can alias emotes to give
	// there character unique like socials
	bool rps_candiate=true;
	if(*argument=='#'){
		rps_candiate=false;
		argument++;
	};
	
    strcpy (act_text, "$n ");
    strcpy (vict_text, "$n ");
	
    letter = argument;
	
    for (; *letter != '\0'; letter++)
    {
        if (*letter == '@')
        {
            if ( *(letter+1) == '\'' && *(letter+2) == 's')
            {
                strcat(act_text, "`#`=\xc4$N's`&");
                strcat(vict_text, "your");
                letter += 2;
            }
            else if ( *(letter+1) == 'r' || *(letter+1) == 's') 
            {
                strcat(act_text, "`#`=\xc4$N's`&");
                strcat(vict_text, "your");
                letter++;
            }
            else 
            { 
                strcat(act_text, "`#`=\xc4$N`&");
                strcat(vict_text, "you");
            }
        }
        else
        {
            strncat(act_text, letter,1);
            strncat(vict_text, letter,1);
        }
    }
	
    MOBtrigger = false;
    act( act_text,  ch, NULL, victim, TO_NOTVICT );
    act( act_text,  ch, NULL, victim, TO_CHAR );
    act( vict_text, ch, NULL, victim, TO_VICT );
    MOBtrigger = true;
	
	if(!IS_NPC(ch) && rps_candiate){
		ch->pcdata->did_ic=true;
		if(room_is_private(ch->in_room)){ 
			ch->pcdata->did_ooc=true; 
			ch->pcdata->did_ic=false; 
		}
	}
}

/**************************************************************************/
//  modified by Kalahn to use * in place of name and use act 
void do_smote(char_data *ch, char *argument )
{
    char *letter;
    char act_text[MSL];
 
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) && !IS_AWAKE(ch))
    {
        ch->println( "You can't show your emotions." );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        ch->println( "Smote what?" );
        return;
    }
    
    if (count_char(argument,'*') != 1)
    {
        ch->println( "You must include a single * where you want your name to appear in a smote." );
        return;
    }
   
	// don't allow messages that could be defrauding 
	if(check_defrauding_argument(ch, argument)){
		return;
	}

	// allow players to put a # at the start of an emote and 
	// it wont be counted towards rps, but can alias emotes to give
	// there character unique like socials
	bool rps_candiate=true;
	if(*argument=='#'){
		rps_candiate=false;
		argument++;
	};

    act_text[0]='\0';
    letter = argument;

    for (; *letter != '\0'; letter++)
    {
        if (*letter == '*'){
            strcat(act_text, "`#`W$n`&");
        }else{
            strncat(act_text, letter,1);
		}     
    }

    MOBtrigger = false;
    act(act_text, ch, NULL, NULL, TO_ROOM );
    act(act_text, ch, NULL, NULL, TO_CHAR );
    MOBtrigger = true;

    return;
}


/**************************************************************************/
void do_bug( char_data *ch, char *argument )
{
	char buf[MSL];

	if(IS_NULLSTR(argument)){
		ch->println("This command is used for reporting bug in any of code in the game.");
		ch->println("You must type something as a parameter.");
		return;
	}

	sprintf(buf,"do_bug: %s reported the bug '%s' [room %d]\r\n", ch->name, argument, ch->in_room->vnum);
	wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel

	append_logentry_to_file( ch, BUG_FILE, argument );
	ch->println( "Bug logged. (bug automatically records the room you are in by the way)" );

	 return;
}

/**************************************************************************/
void do_typo( char_data *ch, char *argument )
{
	char buf[MSL];

	if(IS_NULLSTR(argument)){
		ch->println( "This command is used for reporting typos in any of the text anywhere in the game." );
		ch->println( "You must type something as a parameter, there is also the 'type' social." );
		return;
	}
	
	sprintf(buf,"do_typo: %s reported the typo '%s' [room %d]\r\n", ch->name, argument, ch->in_room->vnum);
	wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel

     append_logentry_to_file( ch, TYPO_FILE, argument );
     ch->println( "Typo logged. (typo automatically records the room you are in by the way)" );
	 return;
}

/**************************************************************************/
void do_qui( char_data *ch, char *)
{
    ch->println( "If you want to QUIT, you have to spell it out." );
    return;
}
/**************************************************************************/
void mp_logout_trigger( char_data *ch);
/**************************************************************************/
void quit_char(char_data *ch, const char *argument, bool character_deleting )
{
	connection_data *d,*c_next;
    int player_id;

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}

	if(IS_SET(ch->dyn,DYN_USING_ATLEVEL)){
		ch->println( "You can't quit while using at level!" );
		return;
	}

    if ( IS_SWITCHED(ch) )
    {
		if (IS_IMMORTAL(ch)){
			ch->println( "You are currently controlling a mob... type `=Creturn`x first!" );
		}else{
			ch->println( "You are not in your mortal body, type `=Creturn`x to get back to it then try quitting.!" );
		}
        return;
    }

    if ( IS_NPC(ch) )
        return;

	// ploaded players can't quit
	if(ch->pload && ch->pload->dont_save){
		if(ch->pload->loaded_by){
			ch->pload->loaded_by->printlnf( "quit_char(): character %s can't 'quit' cause it was ploaded.", ch->name);
		}			
		logf( "quit_char(): character %s can't 'quit' cause it was ploaded.", ch->name);
		return;
	}
	
	mp_logout_trigger( ch);

	if ( !character_deleting && !IS_LINKDEAD(ch) && ch->pnote && str_cmp("confirm", argument)) {
		
		switch(ch->pnote->type) {
			default:
				break;
			case NOTE_NOTE:
				ch->println( "`YYou have a Note in progress.`x\r\n" );
				break;
			case NOTE_IDEA:
				ch->println( "`YYou have an Idea in progress.`x\r\n" );
				break;
			case NOTE_PENALTY:
				ch->println( "`YYou have a Penalty note in progress.`x\r\n" );
				break;
			case NOTE_NEWS:
				ch->println( "`YYou have a News in progress.`x\r\n" );
				break;
			case NOTE_CHANGES:
				ch->println( "`YYou have a Change in progress.`x\r\n" );
				break;
			case NOTE_ANOTE:
				ch->println( "`YYou have an Anote in progress.`x\r\n" );
				break;
			case NOTE_INOTE:
				ch->println( "`YYou have an Inote in progress.`x\r\n" );
				break;
		}
		ch->println( "You may either post it, clear it, or type `=Cquit confirm`x to quit now." );
		return;		
	}

    if (!( character_deleting
			|| ( IS_IMMORTAL(ch) && !str_cmp("confirm", argument))
		  )
	    ) // if they are deleting they can quit or if they are an imm typing 'quit confirm' 
    {
        if ( ch->position == POS_FIGHTING )
        {
            ch->println( "No way! You are fighting." );
            return;
        }

#ifndef unix
		if (moot->moot_type<0){
			moot->moot_type =0;
		}
#endif 
	
        if(ch->pcdata->diplomacy && moot->moot_type){
            ch->println( "You can not quit while a moot is in progress." );
            return;
        }
    
        if(moot->moot_victim==ch){
            ch->println( "You can not quit while a moot is called against you." );
            return;
        }

		if(!GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){
			if(ch->pknorecall>0){
				if (IS_IMMORTAL(ch)){
					ch->println( "You have a PK norecall timer (been in a pk fight)" );
					ch->println( "If you wish to quit you must type `#`=Cquit confirm`&" );
				}else{
					ch->println( "You may not leave this world so soon after conflict." );
				}
				return;
			}
		}

        if(ch->pknoquit>0){
			if (IS_IMMORTAL(ch)){
				ch->println( "You have a PK noquit timer (been in a pk fight)" );
				ch->println( "If you wish to quit you must type `#`=Cquit confirm`&" );
			}else{
				ch->println( "You may not leave this world so soon after PK interactions." );
				ch->println( "(you have a pknoquit timer)" );
			}
            return;
        }
		
        if ( ch->position  < POS_STUNNED  )
        {
            ch->println( "You're not DEAD yet." );
            return;
        }
    }

    ch->println( "Alas, all good things must come to an end." );
    act( "$n fades from the realm.", ch, NULL, NULL, TO_ROOM );
	
	// send the info broadcast about the person leaving
	info_broadcast(ch, "%s has left the realm.", ch->name);

	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD))
	{
		if(!IS_IMMORTAL(ch)){
			pkill_broadcast("%s fades from the realm of death! [Pk=%d.Pd=%d]\r\n", 
				ch->name,			
				TRUE_CH(ch)->pcdata->p9999kills,
				TRUE_CH(ch)->pcdata->p9999defeats);

		}
	}

    if (ch->desc){
        logf( "%s has quit. (connected_socket = %d), (left in room=%d, lasticroom=%d)", 
			ch->name, ch->desc->connected_socket, 
			ch->in_room?ch->in_room->vnum:0,
			ch->last_ic_room?ch->last_ic_room->vnum:0);
    }else{
        logf( "%s has quit. (descriptor = linkless) (left in room=%d, lasticroom=%d)", 
			ch->name, 
			ch->in_room?ch->in_room->vnum:0,
			ch->last_ic_room?ch->last_ic_room->vnum:0);
	}
    
    sprintf( log_buf, "%s has left the game. (left level %d, remort %d), room=%d, lasticroom=%d", 
		ch->name, ch->level, ch->remort,
		ch->in_room?ch->in_room->vnum:0,
		ch->last_ic_room?ch->last_ic_room->vnum:0);
    wiznet(log_buf,ch,NULL,WIZ_LOGINS,0, UMIN(get_trust(ch), MAX_LEVEL));

	// record their exit into the game into their plog if they have one
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG)){
        append_playerlog( ch, log_buf);
    }

    if (ch->desc){
		if (IS_IRC(ch)){
			sprintf( log_buf, "was %s@%s (IRC).", ch->name, ch->desc->remote_hostname);
		}else{
			sprintf( log_buf, "was %s@%s", ch->name, ch->desc->remote_hostname);
		}
		wiznet(log_buf,NULL,NULL,WIZ_SITES,0,UMIN(get_trust(ch),MAX_LEVEL));
	}

    // After extract_char the ch is no longer valid!
    save_char_obj( ch );
    player_id = ch->player_id;
    d = ch->desc;
    extract_char( ch, true );
    if ( d){
        connection_close( d );
	}else{
		log_string("do_quit: linkless player quitting");
	}

    // toast evil cheating bastards 
    for (d = connection_list; d != NULL; d = c_next)
    {
        char_data *tch;   
        c_next = d->next;
        tch = d->original ? d->original : d->character;
        if (tch && tch->player_id == player_id)
        {
            extract_char(tch,true);
            connection_close(d);
        } 
    }
    return;
}
/**************************************************************************/
void do_quit( char_data *ch, char *argument )
{
	quit_char(ch, argument, false);
}

/**************************************************************************/
void do_save( char_data *ch, char *)
{
	 if ( IS_NPC(ch) )
	return;

	 save_char_obj( ch );
     ch->printlnf( "Saving, remember that %s has automatic saving.", MUD_NAME );

#ifdef unix
     WAIT_STATE(ch,1 * PULSE_VIOLENCE);
#endif
    return;
}


/**************************************************************************/
// RT changed to allow unlimited following and follow the NOFOLLOW rules 
void do_follow( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
	
    one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "Follow whom?" );
		return;
	}
	
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
		ch->println( "They aren't here." );
		return;
    }
	
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
		act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
		return;
    }
	
    if ( victim == ch )
    {
		if ( ch->master == NULL )
		{
			ch->println( "You already follow yourself." );
			return;
		}
		stop_follower(ch);
		return;
    }
	
    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
		act("$N doesn't seem to want any followers.\r\n",
			ch,NULL,victim, TO_CHAR);
		return;
    }
	
    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
		stop_follower( ch );
	
    add_follower( ch, victim );
	return;
}

/**************************************************************************/
void add_follower( char_data *ch, char_data *master )
{
	if ( ch->master != NULL )
	{
		bug("Add_follower: non-null master.");
		return;
	}

	ch->master = master;
	ch->leader = NULL;

	if ( can_see( master, ch ))
	{
		if ( !IS_SET(ch->dyn,DYN_SILENTLY))
		{
			act( "$n now follows you.", ch, NULL, master, TO_VICT );
		}
	}

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}


/**************************************************************************/
void stop_follower( char_data *ch )
{
    if ( ch->master == NULL )
    {
		bug("Stop_follower: null master.");
		return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
		REMOVE_BIT( ch->affected_by, AFF_CHARM );
		affect_strip( ch, gsn_charm_person );

		// remove loophole where players would tell a mob to attack then 
		// do nofollow leaving the mob to fight to the death
		if(IS_NPC(ch)){
			if(ch->fighting && IS_NPC(ch->fighting)){
				do_flee(ch,"");
				do_flee(ch,"");
				do_flee(ch,"");

				// if they don't succeed in fleeing they become worthless
				if(ch->fighting){
					ch->no_xp=true;
					ch->hitroll=-30;
					ch->damroll=-30;
					ch->hit=1;
					ch->max_hit=1;
				}
			}
		}
    }

	if ( ch->in_room && can_see( ch->master, ch ))
    {
		if ( IS_SET( ch->dyn, DYN_SILENTLY ))
		{
			act( "$n stops following you.", ch, NULL, ch->master, TO_VICT );
			act( "You stop following $N.",	ch, NULL, ch->master, TO_CHAR );
		}
	}
	if (ch->master->pet == ch)
		ch->master->pet = NULL;

	ch->master = NULL;
	ch->leader = NULL;
    return;
}
/**************************************************************************/
bool is_character_loaded_into_room(char_data *ch, ROOM_INDEX_DATA *room);
/**************************************************************************/
// nukes charmed monsters and pets
void nuke_pets( char_data *ch)
{    
    char_data *pet;

    if ((pet = ch->pet) != NULL)
    {
		// if they arent in the game yet (haven't logged in, and this 
		//   isn't a reconnect)  set pet->room to NULL
		if((ch->desc && ch->desc->connected_state!=CON_PLAYING) || !ch->desc){
			if(!is_character_loaded_into_room(ch->pet,ch->pet->in_room)){
				ch->pet->in_room=NULL;
			};
		}
		stop_follower(pet);

		if (pet->in_room != NULL)
			 act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
		extract_char(pet,true);
	 }
	 ch->pet = NULL;

    return;
}



/**************************************************************************/
void die_follower( char_data *ch )
{
    char_data *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

	 ch->leader = NULL;

	 for ( fch = char_list; fch != NULL; fch = fch->next )
    {
		 if(ch->pet==fch){ // pets arent affected by nofollow
			 continue;
		 }
		if ( fch->master == ch )
			 stop_follower( fch );
		if ( fch->leader == ch )
			fch->leader = fch;
    }

    return;
}


/**************************************************************************/
void do_order( char_data *ch, char *argument )
{
	char buf[MSL];
	char arg[MIL], arg2[MIL], arg3[MIL];
	char_data *victim;
	char_data *recipient;
	char_data *och;
	char_data *och_next;
	bool found;
	bool fAll;
	
	argument = one_argument( argument, arg );
	argument = one_argument(argument,arg2);
	
	if ( IS_NULLSTR(arg) || IS_NULLSTR(arg2))
	{
		ch->println( "Order whom to do what?" );
		return;
	}

	if ( IS_AFFECTED( ch, AFF_CHARM ) )
	{
		ch->println( "You feel like taking, not giving, orders." );
		return;
	}
	
	if ( !str_cmp( arg, "all" ) )
	{
		fAll   = true;
		victim = NULL;
	}   
	else
    {
        fAll   = false;

		// allow order pet
		if(ch->pet && !str_cmp(arg ,"pet")){
			victim=ch->pet;
			if(ch->in_room!=victim->in_room){
				ch->println( "Your pet isn't here." );
				return;
			}
		}else{
			if (( victim = get_char_room( ch, arg )) == NULL )
			{
				ch->println( "They aren't here." );
				return;
			}
		}
		
        if ( victim == ch )
        {
			ch->println( "Aye aye, right away!" );
            return;
        }
		
        if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
			||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
        {
            ch->println( "Do it yourself!" );
            return;
        }
    }
	
	strcat(arg2, " ");
	strcat(arg2, argument);
    found = false;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
        och_next = och->next_in_room;
		
        if ( IS_AFFECTED(och, AFF_CHARM)
			&&   och->master == ch
			&& ( fAll || och == victim ) )
        {
            if ( !str_prefix("master", arg2))
            {
				argument = one_argument(argument, arg3);
				
                if  (!IS_SET(och->act, ACT_PET))
				{
                    ch->println( "That is not your pet." );
                    return;
                }
				
                if ( IS_NULLSTR(arg3))
                {
                    ch->println( "Who would you like to give your pet to?" );
                    return;
                }
                
                if ( ( recipient = get_char_room( ch, arg3 ) ) == NULL )
                {
                    ch->println( "There is nobody like that here to take the pet." );
                    return;
                }
				
				if (IS_NPC(recipient)) 
                {
                    ch->println( "You may only give pets to other players." );
                    return;
                }
				
				if (recipient->level < och->level)   
                {
                    act( "It appears as though your pet would be the master of $N.", 
						ch, NULL, recipient, TO_CHAR);
                    return;
                }
				
				if (recipient->pet){
                    act( "It appears as though $N already has a pet.", 
						ch, NULL, recipient, TO_CHAR);
                    return;
				}
				
				stop_follower(och);
				ch->pet = NULL; 
				
				SET_BIT(och->act, ACT_PET);
				SET_BIT(och->affected_by, AFF_CHARM);
				add_follower( och, recipient);
				och->leader = recipient;
				recipient->pet = och;
				
				act("$n is now $N's pet.", och, NULL, recipient, TO_ROOM);
				act("You are now $N's pet.", och, NULL, recipient, TO_CHAR);  			
				return;
            }
            else
            {
				// PC being ordered to do this - 3% chance of breaking the charm
				if ( !IS_NPC(och) && number_range(1,33)==1 && !IS_IMMORTAL(ch))
				{
					sprintf( buf, "$n orders you to '%s', you manage to come to your senses and break the charm!", arg2 );
					affect_parentspellfunc_strip( och, gsn_charm_person);
					act( buf, ch, NULL, och, TO_VICT );
					ch->printlnf( "%s doesn't appear to have heard you!", PERS(och, ch));
					continue;
				}

				found = true;
				// Toggle the Ordered Bit
				SET_BIT( och->dyn, DYN_IS_BEING_ORDERED );

				sprintf( buf, "$n orders you to '%s'.", arg2 );
				act( buf, ch, NULL, och, TO_VICT );
				interpret( och, arg2 );

				// Remove the Ordered Bit
				REMOVE_BIT( och->dyn, DYN_IS_BEING_ORDERED );
            }
        }
    }
	
    if ( found )
    {
        WAIT_STATE(ch,PULSE_VIOLENCE);
        ch->println( "Ok." );
    }
    else
        ch->println( "You have no followers here." );
    return;
}


/**************************************************************************/
void do_group( char_data *ch, char *argument )
{
	char arg[MIL];
	char_data *victim;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		char_data *gch;
		char_data *leader;
		
		leader = (ch->leader != NULL) ? ch->leader : ch;
		ch->printlnf( "%s's group:", icapitalize( PERS( leader, ch )));

		for ( gch = char_list; gch; gch = gch->next )
		{
			if ( is_same_group( gch, ch ) )
			{
				// has hacks to prevent div by 0 
				ch->printlnf( "[ %3d%% hp - %3d%% mana - %3d%% mv ] %s",
					(int) gch->hit*100/(gch->max_hit==0?1:gch->max_hit),
					(int) gch->mana*100/(gch->max_mana==0?1:gch->max_mana),
					(int) gch->move*100/(gch->max_move==0?1:gch->max_move),
					capitalize( PERS(gch, ch) ));
			}
		}
		return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
		ch->println( "They aren't here." );
		return;
	}

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
		ch->println( "But you are following someone else!" );
		return;
	}

    if ( victim->master != ch && ch != victim )
    {
		act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
		return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
		ch->println( "You can't remove charmed mobs from your group." );
        return;
    }
    
	if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
    	return;
    }

	if(GAMESETTING(GAMESET_RESTRICTED_GROUPING)){
		if (victim->level - ch->level > 8)
		{
			ch->println( "They are too high of a level for your group." );
			return;
		}
 
		if (victim->level - ch->level < -8)
		{
			ch->println( "They are too low of a level for your group." );
			return;
		}
	}

	if ( IS_NPC( victim ))
	{
		ch->println( "You can't use this command to group mobs. ");
		return;
	}

	if ( is_same_group( victim, ch ) && ch != victim )
    {
		victim->leader = NULL;
		act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
		act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
		return;
    }

    victim->leader = ch;
	act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}
/**************************************************************************/
void do_report(char_data *ch, char *)
{
	// has hacks to prevent div by 0 
	do_say(ch, 
		FORMATF( "I have %3d%% hp, %3d%% mana and %3d%% movement.",
			(int) ch->hit*100/(ch->max_hit==0?1:ch->max_hit),
			(int) ch->mana*100/(ch->max_mana==0?1:ch->max_mana),
			(int) ch->move*100/(ch->max_move==0?1:ch->max_move)));		
}
/**************************************************************************/
void do_split( char_data *ch, char *argument )
{
	char buf[MSL];
	char arg1[MIL],arg2[MIL];
	char_data *gch;
	int members;
	int amount_gold = 0, amount_silver = 0;
	int share_gold, share_silver;
	int extra_gold, extra_silver;

	argument = one_argument( argument, arg1 );
	one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		ch->println( "Split how much?" );
		return;
	}

	amount_silver = atoi( arg1 );

	if (arg2[0] != '\0')
		amount_gold = atoi(arg2);

	if ( amount_gold < 0 || amount_silver < 0)
	{
		ch->println( "Your group wouldn't like that." );
		return;
	}

	if ( amount_gold == 0 && amount_silver == 0 )
	{
		ch->println( "You hand out zero coins, but no one notices." );
		return;
	}

	if ( ch->gold <  amount_gold || ch->silver < amount_silver)
	{
		ch->println( "You don't have that much to split." );
		return;
	}

	members = 0;
	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
		if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
			members++;
	}
	
	if ( members < 2 )
	{
		ch->println( "Just keep it all." );
		return;
	}
	
	share_silver = amount_silver / members;
	extra_silver = amount_silver % members;
	share_gold   = amount_gold / members;
	extra_gold   = amount_gold % members;

	if ( share_gold == 0 && share_silver == 0 )
	{
		ch->println( "Don't even bother, cheapskate." );
		return;
	}

	ch->silver	-= amount_silver;
	ch->silver	+= share_silver + extra_silver;
	ch->gold 	-= amount_gold;
	ch->gold 	+= share_gold + extra_gold;
    
	if (share_silver > 0)
    {
		ch->printlnf( "You split %d silver coins. Your share is %d silver.",
			amount_silver,share_silver + extra_silver);
    }

	if ( share_gold > 0)
	{
		ch->printlnf( "You split %d gold coins. Your share is %d gold.",
			amount_gold,share_gold + extra_gold);
	}

	if (share_gold == 0)
	{
		sprintf(buf,"$n splits %d silver coins. Your share is %d silver.",
			amount_silver,share_silver);
	}
	else if (share_silver == 0)
	{
		sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
			amount_gold,share_gold);
	}
	else
	{
		sprintf(buf,
			"$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\r\n",
			amount_silver,amount_gold,share_silver,share_gold);
	}

	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
		if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
		{
			act( buf, ch, NULL, gch, TO_VICT );
			gch->gold += share_gold;
			gch->silver += share_silver;
		}
	}
	return;
}


/**************************************************************************/
void do_gtell( char_data *ch, char *argument )
{
    char buf[MSL];
    char_data *gch;

    if ( argument[0] == '\0' )
    {
		ch->println( "Tell your group what?" );
		return;
	}

	if ( IS_SET( ch->comm, COMM_NOTELL ) )
	{
		ch->println( "Your message didn't get through!" );
		return;
	}

	// Note use of gch->printlnf so gtell works on sleepers.
	sprintf( buf, "%s tells the group '%s'", ch->name, argument );
	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
		if ( is_same_group( gch, ch ) )
			gch->printlnf( "%s", buf );
	}

	return;
}

/**************************************************************************/
void do_resetroom(char_data *ch, char *)
{
	reset_room( ch->in_room, true );
	ch->printlnf( "Room '%s' [%d] reset.",
		ch->in_room->name, ch->in_room->vnum);
    return;
}

/**************************************************************************/
void do_resetarea(char_data *ch, char *)
{
	reset_area( ch->in_room->area);
	reset_room( ch->in_room, true );
	ch->printlnf( "Area '%s' [%d-%d] reset.",
		ch->in_room->area->name, ch->in_room->area->min_vnum,
		ch->in_room->area->max_vnum);
    return;
}

/**************************************************************************/
void do_crashloop (char_data *ch, char * argument)
{
	if (IS_NPC(ch)){
		do_huh(ch,"");
        return;
	}

    if (str_cmp("confirm", argument))
    {
        ch->println( "`xType `=Ccrashloop confirm`x if you want to put the mud into" );
        ch->println( "`xan endless loop to test the signal handling!" );
		return;
	}

	for (;;);  // loop endlessly ( Now that's some awesome code :) -- Ker )
}
/**************************************************************************/
/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( char_data *ach, char_data *bch )
{
	if ( ach == NULL || bch == NULL)
		return false;
    
	if ( ach->leader != NULL ) ach = ach->leader;
	if ( bch->leader != NULL ) bch = bch->leader;
	return ach == bch;
}

/**************************************************************************/
void do_ntell( char_data *ch, char *argument )  
{
	char arg[MIL], buf[MSL];
    char_data *nch, *victim, *nvictim;

    // unswitched mobs can't send ntells
    if (IS_UNSWITCHED_MOB(ch))
    {
		return;
    }

	if (!IS_NOBLE(ch)){
		do_huh(ch,"");
		return;
	}

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}

    argument = one_argument( argument, arg );

	if ( arg[0] == '\0' || argument[0] == '\0' )
	{
		ch->println( "Noble tell whom what?" );
		return;
	}

	if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL
		||  IS_NPC(victim) ) 
	{
		ch->println( "They aren't here." );
		return;
	}
	// Put in these lines from do_tell in because it made some sense. <shrug>
	if (victim->controlling && victim->controlling->desc)
	{
		victim=victim->controlling;
	}
	
	if(ch==victim)
	{
		ch->println( "No point in sending an ntell to yourself." );
		return;
	};

	// Also from do_tell, looked good to put.
	if ( victim->desc == NULL)
	{
		act("$N seems to have misplaced $S link...try again later.", ch,NULL,victim,TO_CHAR);
		
		sprintf(buf,"A noble ntells you '%s'\r\n`x", argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
	}

	if (IS_SET(victim->comm,COMM_AFK))
	{
		if (IS_NPC(victim))
		{
			act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
			return;
		}

		act("$E is AFK, but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR);
		
		sprintf(buf,"A noble ntells you '%s'\r\n", argument );
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
    
		victim->printlnf( "A noble just tried to ntell you '%s' (you are marked as afk)`x", argument);
		return;
	}

	if(!IS_NPC(ch))
		ch->pcdata->did_ooc=true;

	// Send the tell to the player.
	victim->printlnf("`M%s ntells you '%s`M'"
		"(you can use nreply to talk back if you need to)`x\r\n", 
		IS_IMMORTAL(victim)?PERS(ch,victim):"A noble", argument );

	// Start the broadcast of the ntell to all nobles.
	for ( nch = player_list; nch; nch = nch->next_player )
	{
		if (TRUE_CH(ch)==nch)
		{
			ch->printlnf( "`MYou ntell %s '%s`M'", victim->name, argument);
			continue;
		}

		// don't see both the noble broadcast and the personal message
		if(	nch==victim)
		{
			continue;
		};

		nvictim=nch;
		// ntalk going thru switch
		if (nvictim->controlling && nvictim->controlling->desc)
			nvictim=nvictim->controlling;

		if (IS_NOBLE(nch))
		{
			if (IS_IMMORTAL(nch))
			{
				nvictim->printlnf( "`M<%s ntells %s>: '%s`M'`x", 
					TRUE_CH(ch)->name, victim->name, argument);
			}else{
				nvictim->printlnf( "`M<A noble ntells %s>: '%s`M'`x", victim->name, argument);
			}
		}
	}
	return;
}

/**************************************************************************/
void do_nreply( char_data *ch, char *argument )
{
	char_data *nch, *victim;

    // unswitched mobs can't send nreplies
    if (IS_UNSWITCHED_MOB(ch))
    {
		return;
    }

    if ( IS_NULLSTR(argument) )
    {
        ch->println( "Nreply what?" );
        return;
    }

	//Start the broadcast of the nreply to all nobles.
    for ( nch = player_list; nch; nch = nch->next_player )
    {
		if (TRUE_CH(ch)==nch)
		{
			ch->printlnf( "`MYou nreply '%s`M'", argument);
			continue;
		}

		victim=nch;

		// ntalk going thru switch
		if (victim->controlling && victim->controlling->desc)
			victim=victim->controlling;

		if (IS_NOBLE(nch))
		{
			victim->printlnf( "`M<%s nreplies> '%s`M'`x",
					TRUE_CH(ch)->name, argument);
		}
	}
	return;
}

/**************************************************************************/
void do_declineooc(char_data *ch, char *argument)
{
    char_data *victim;
	char arg[MIL];
	int minutes = 0;

	argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg ))
    {
        ch->println( "Who do you want to decline?" );
        return; 
    }
	if ( !IS_NULLSTR( argument ))
	{
		minutes = atoi( argument );
	}

    if (( victim = get_whovis_player_world( ch, arg )) == NULL )
    {
        ch->println( "They are not playing." );
        return;
    }

    ch->printlnf( "`cSending ooc chat decline to %s.`x", victim->name);
	if ( minutes > 0 )
	{
		ch->printlnf( "`c  You have specified that you will be available in %d minute(s).`x", minutes );
	}
    victim->printlnf( "`c%s has declined an ooc chat with you.", ch->name );
	if ( minutes > 0 )
	{
		victim->printlnf( "`c  %s has specified that they will be available in approximately %d minute(s).`x",
			ch->name, minutes );
		victim->println( "`c  Please respect their wishes and be patient.`x" );
	}
}

/**************************************************************************/
extern bool hotreboot_in_progress;
/**************************************************************************/
// turn mccp on/off
void do_mccp( char_data *ch, char *argument )
{
	connection_data *d=TRUE_CH(ch)->desc;
#ifndef MCCP_ENABLED
	ch->wrapln("Mud Client Compression Protocol (MCCP) support has not "
		"been enabled for this compile of the mud therefore unavailable.");
	ch->wraplnf("There have been a total of %d uncompressed bytes sent to your connection", 
		d?d->bytes_sent:0);
	return;
#else
    if (!d) {
        ch->println("You don't have an descriptor!?!");
        return;
    }
	
	if(IS_NULLSTR(argument)){
		unsigned int bs=d->bytes_sent;
		unsigned int bsbc=d->bytes_sent_before_compression;
		unsigned int bsac=d->bytes_sent_after_compression;
		ch->titlebar("MUD CLIENT COMPRESSION PROTOCOL SYNTAX");
		ch->println("  Syntax: `=Cmccp on2`x    - to force mccp on (v2 startup).");
		ch->println("  Syntax: `=Cmccp on1`x    - to force mccp on (v1 startup).");
		ch->println("  Syntax: `=Cmccp off`x    - to force mccp off.");
		ch->titlebar("MCCP CONNECTION STATISTICS");
		if(d->out_compress){
			ch->printlnf("  `WYour connection is currently connected with mccp%d.`x",
				d->mccp_version);
		}else{
			ch->println("  Your connection is currently not making use of mccp.");
			
		}
		ch->printlnf("  Total bytes sent to your connection: %d", bs);

		if(d->out_compress){
			ch->printlnf("  Bytes sent to your connection after compression: %7d", bsac);
			ch->printlnf("  Bytes sent to your connection before compression:%7d", bsbc);
			if(bsbc){
				ch->printlnf("  Compression ratio: %0.2f%%`x", (float)bsac*100/ (float)bsbc );
			}
		}
		ch->titlebar("");
		return;
	}

	if(hotreboot_in_progress){
		ch->println("You can't change your mccp status while a hotreboot is in progress");
		return;
	}

	if(!str_cmp("on2", argument) || !str_cmp("on", argument)){
		if(d->out_compress){
			ch->println("Your connection is already compressed.");
		}else{
			ch->println("Manually starting mccp2 compression.");
			d->mccp_version=2;
			d->begin_compression();
		}
		return;
	}

	if(!str_cmp("on1", argument)){
		if(d->out_compress){
			ch->println("Your connection is already compressed.");
		}else{
			ch->println("Manually starting mccp1 compression.");
			d->mccp_version=1;
			d->begin_compression();
		}
		return;
	}

	if(!str_cmp("off", argument)){
		if(d->out_compress){
			ch->printlnf("Stopping mccp%d compression.", d->mccp_version);
			d->end_compression();
		}else{
			ch->println("You don't currently have a compressed connection to turn off.");
		}
		return;
	}
	ch->printlnf("Unrecognised command '%s'", argument);
	do_mccp(ch,"");
#endif // MCCP_ENABLED
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
