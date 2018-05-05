/**************************************************************************/
// channels.cpp - channel related functions
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"
#include "channels.h"
#include "areas.h"
#include "clan.h"

// global channel numbers - variables
int gcn_admintalk;
int gcn_codetalk;
int gcn_clantalk;
int gcn_chat;
int gcn_flame;
int gcn_globalsocial;
int gcn_grats;
int gcn_hightalk;
int gcn_ic;
int gcn_immtalk;
int gcn_info;
int gcn_mythostalk;
int gcn_newbietalk;
int gcn_nobletalk;
int gcn_ooc;
int gcn_question;
int gcn_answer;
int gcn_quiet;
int gcn_tells;

#define DEFAULT_CHANNEL_MEMORY_LOG_LINES (30)
/**************************************************************************/
class channel_data channels[]=
{
	{ &gcn_admintalk,		// index
		"AdminTalk",		// name
		"`=Y",				// colour
		"",					// you_format (if empty, normal format is used) e.g. "You[`=y %s" 
		"%s[`=y %s",		// format
		"`x",				// trailer
		"",					// logfile
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_ADMINTALK,	// channel_flag_value
		CHANFLAG_ADMIN | CHANFLAG_IGNORE_QUIET		// flags
	},

	{ &gcn_codetalk,	
		"CodeTalk",		
		"`M",	
		"",	
		"<`Y%s`M> -=> `Y%s`M <=-",	
		"`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_CODETALK, 
		CHANFLAG_CODE_COUNCIL | CHANFLAG_IGNORE_QUIET
	},

	{ &gcn_clantalk,
		"ClanTalk",			
		"`=\x9a",	
		"<You ClanTalk>: '`=\x99%s",
		"<%s ClanTalks>: '`=\x99%s",
		"'`x",	
		CLANTALK_LOGFILE,
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_CLANTALK,
		CHANFLAG_CLANMEMBERS_ONLY
	},

	{ &gcn_chat,			
		"Chat",			
		"`=O",	
		"<You chat>: '%s",
		"<%s chats>: '%s",
		"'`x",	
		CHAT_LOGFILE,
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_CHAT, 
		CHANFLAG_MYSTERY_IMMS	
	},

	{ &gcn_flame,		
		"Flame",		
		"`r",	
		"<YOU FLAME>: %s",
		"<%s FLAME>: %s",
		"`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_FLAME, 
		CHANFLAG_MYSTERY_IMMS
	},

	{ &gcn_globalsocial,
		"GlobalSocial",	
		"",		
		"",		
		"",							
		"",		
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_GSOCIAL, 
		CHANFLAG_MYSTERY_IMMS
	},

	{ &gcn_grats,		
		"Grats",		
		"`M",	
		"<You GRATS>: %s",			
		"<%s GRATS>: %s",			
		"`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_GRATS, 
		CHANFLAG_MYSTERY_IMMS	
	},

	{ &gcn_hightalk,	
		"HighTalk",		
		"`=Z",	
		"",
		"%s]`=z %s",
		"`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_HIGHTALK, 
		CHANFLAG_HIGHADMIN | CHANFLAG_IGNORE_QUIET
	},

	{ &gcn_ic,			
		"IC",			
		"`=W",	
		"<You IC>: '%s",				
		"<%s IC>: '%s",				
		"'`x",	
		IC_LOGFILE,
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_IC, 
		CHANFLAG_MYSTERY_IMMS	
	},

	{ &gcn_immtalk,		
		"ImmTalk",		
		"`=I",	
		"",
		"%s`=I:`=i %s",				
		"`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_IMMTALK, 
		CHANFLAG_IMM_CHANNEL | CHANFLAG_IMMTALK_ACCESS | 
			CHANFLAG_USE_PERS_IF_SWITCHED | CHANFLAG_IGNORE_QUIET
	},

	{ &gcn_info,		
		"Info",			
		"`=\x8a",
		"",
		"[INFO: `=\x8b%s`=\x8a]",	
		"`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_INFO, 
		0 
	},

	{ &gcn_mythostalk,	
		"MythosTalk",	
		"`b",
		"",
		"-`B%s`b-> `W%s",
		"`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_MYTHOSTALK, 
		CHANFLAG_MYTHOS_COUNCIL | CHANFLAG_IGNORE_QUIET
	},

	{ &gcn_newbietalk,	
		"NewbieTalk",	
		"`=E",
		"<You Newbietalk>: '`=e%s",
		"<%s Newbietalks>: '`=e%s",
		"'`x", 
		NEWBIE_LOGFILE,
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_NEWBIETALK, 
		CHANFLAG_MYSTERY_IMMS | CHANFLAG_NEWBIES_AND_SUPPORT
	},

	{ &gcn_nobletalk,	
		"NobleTalk",	
		"",
		"",
		"",
		"",
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_NOBLETALK, 
		CHANFLAG_ANONYMOUS_TO_MORTS | CHANFLAG_NOBLE
	},

	{ &gcn_ooc,			
		"OOC",			
		"`=O",	
		"<You OOC>: '%s",
		"<%s OOC>: '%s",
		"'`x",	
		OOC_LOGFILE,
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_OOC, 
		CHANFLAG_MYSTERY_IMMS	
	},
			
	{ &gcn_question,	
		"Question",
		"`=Q",	
		"You question '%s",
		"%s questions '%s",
		"'`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_QA, 
		CHANFLAG_MYSTERY_IMMS	
	},

	{ &gcn_answer,		
		"Answer",		
		"`=Q",	
		"You answer '%s",
		"%s answers '%s",
		"'`x",	
		"",
		DEFAULT_CHANNEL_MEMORY_LOG_LINES,// memory_log_lines
		CHANNEL_QA, 
		CHANFLAG_MYSTERY_IMMS
	},

	{ &gcn_quiet,		
		"Quiet",		
		"",
		"",
		"",
		"",
		"",
		0,					// memory_log_lines
		CHANNEL_QUIET, 
		0
	},

	{ &gcn_tells,		
		"Tells",		
		"",
		"",
		"",
		"",
		"",
		0,					// memory_log_lines
		CHANNEL_TELLS, 
		CHANFLAG_IGNORE_QUIET
	},

	{ NULL, "", "", "", "", 0, 0} // terminate the list
};


/**************************************************************************/
bool channel_data::can_see_channel(char_data *ch, char_data *talker)
{
	if(!TRUE_CH(ch)->pcdata){
		bugf("channel_data::can_see_channel() past a non player '%s' as a parameter!?!",
			ch->name);
		return false;
	}

	if(IS_SET(flags, CHANFLAG_IMM_CHANNEL) && !IS_IMMORTAL(ch)){
		if(!(IS_SET(flags, CHANFLAG_IMMTALK_ACCESS)
			&& !IS_NULLSTR(TRUE_CH(ch)->pcdata->immtalk_name)))
		{
			return false;
		}
	}

	// CHANFLAG_ADMIN
	if(IS_SET(flags, CHANFLAG_ADMIN) && !IS_ADMIN(ch)){
		return false;
	}

	// CHANFLAG_CODE_COUNCIL
	if(IS_SET(flags, CHANFLAG_CODE_COUNCIL) 
		&& !IS_SET( TRUE_CH(ch)->pcdata->council, COUNCIL_CODE )) {		
		return false;
	}

	// CHANFLAG_MYTHOS_COUNCIL
	if(IS_SET(flags, CHANFLAG_MYTHOS_COUNCIL) 
		&& !IS_SET( TRUE_CH(ch)->pcdata->council, COUNCIL_MYTHOS ))
	{
		return false;
	}

	// CHANFLAG_HIGHADMIN
	if(IS_SET(flags, CHANFLAG_HIGHADMIN) && TRUE_CH(ch)->level<MAX_LEVEL-1 )
	{
		return false;
	}

	// CHANFLAG_NOBLE
	if(IS_SET(flags, CHANFLAG_NOBLE) && !IS_NOBLE(ch))
	{
		return false;
	}

	// CHANFLAG_NEWBIES_AND_SUPPORT
	if(IS_SET(flags, CHANFLAG_NEWBIES_AND_SUPPORT)	
		&& !IS_IMMORTAL( TRUE_CH( ch ))
		&& !IS_NEWBIE_SUPPORT( TRUE_CH( ch ))
		&& ch->level > 10 )
	{
		return false;
	}

	// CHANFLAG_CLANMEMBERS_ONLY
	if(IS_SET(flags, CHANFLAG_CLANMEMBERS_ONLY))
	{
		if(GAMESETTING3(GAMESET3_NO_CLANTALK)){
			return false;
		}

		if(talker){
			CClanType* pClan= ch->clan;
			if(!IS_IMMORTAL(ch)){
				if(!pClan){
					return false;
				}
				if(pClan!=talker->clan){
					return false;
				}
			}
		}
	}

	// if they past everything, they are allowed access to the channel
	return true;
}
/**************************************************************************/
void channel_generic_handler( int channel_index, char_data *ch, char *message)
{
	char buf[MSL];
	char colour_free_message[MSL];
	char colour_free_buf[MSL];
	char text_displayed[MSL];
	bool invis_imm;
	char_data *c;
	
	channel_data *cd=&channels[channel_index];
	
	// unswitched mobs can't ooc
	if(IS_UNSWITCHED_MOB(ch)){
		ch->println( "Players or switched players only." );
		return;
	}
	// link dead players can't ooc (ie can't force ooc from a linkdead)
	if (!ch->desc){
		ch->println( "Linkdead players can't use this command." );
        return;
	}

	if(IS_SET(cd->flags, CHANFLAG_NEWBIES_AND_SUPPORT)	
		&& !IS_IMMORTAL( TRUE_CH( ch ))
		&& !IS_NEWBIE_SUPPORT( TRUE_CH( ch ))
		&& ch->level > 10 )
	{
		ch->println( "The newbie channel cannot be used after level 10." );
		return;
	}

	if(ch->in_room && IS_SET(ch->in_room->room_flags,ROOM_NOCHANNELS)){
		if(IS_IMMORTAL(ch)){
			ch->println( "`Snote: mortals can't use any channels while in this room.`x" );
		}else{
			ch->println( "You can't use any channels while in this room." );
			return;
		}
	}


	// check if the talker can see the channel
	if(!cd->can_see_channel(ch, NULL)){
		do_huh(ch,"");
		return;
	}

	// get the immtalk name if appropriate
	char *immtalk_name="";
	if(IS_SET(cd->flags, CHANFLAG_IMM_CHANNEL) && !IS_IMMORTAL(ch)){
		if(	IS_SET(cd->flags, CHANFLAG_IMMTALK_ACCESS)
			&& !IS_NULLSTR(TRUE_CH(ch)->pcdata->immtalk_name)){
			immtalk_name=TRUE_CH(ch)->pcdata->immtalk_name;		
		}else{
			do_huh(ch,"");
			return;
		}
	}

	// Check if they're being ordered to do this
	if(IS_SET( ch->dyn, DYN_IS_BEING_ORDERED )){
		if(ch->master){
			ch->master->println( "Not going to happen." );
		}
		return;
	}

	// don't allow messages that could be defrauding 
	if(check_defrauding_argument(ch, message)){
		return;
	}

	// if no message is to be sent, turn the channel on/off
	if(IS_NULLSTR(message)){
		if(HAS_CHANNELOFF(ch, cd->channel_flag_value)){
            ch->printlnf( "%s%s channel is now ON.`x", cd->colour, cd->name);
			REMOVE_CHANNELOFF(ch, cd->channel_flag_value);
		}else{
            ch->printlnf( "%s%s channel is now OFF.`x", cd->colour, cd->name);
			SET_CHANNELOFF(ch, cd->channel_flag_value);
		}
		return;
	}

	if(IS_SET(cd->flags, CHANFLAG_CLANMEMBERS_ONLY) && !ch->clan)
	{
		ch->println("You can't use clantalk unless you are in a clan.");
		return;		
	}

	if(HAS_CHANNELOFF(ch, CHANNEL_QUIET)){
		ch->println( "You must turn off quiet mode first." );
		return;
	}
	
	if (IS_SET(TRUE_CH(ch)->comm,COMM_NOCHANNELS)){
		ch->println( "The gods have revoked your channel priviliges." );
		return;			
	}
	
	// stop single character messages on OOC, since the hotkey 
	// is the same as the .x syntax for the string editor
	if(channel_index==gcn_ooc && str_len(message)==1){
		ch->println( "OOC messages must be longer than a single character." );
		return;			
	}

	TRUE_CH(ch)->pcdata->did_ooc=true;

	// message about to be sent, turn on this channel if it isn't already
	REMOVE_CHANNELOFF(ch, cd->channel_flag_value);
	
    message=ltrim_string(message);
	
	int invis_level=UMAX(INVIS_LEVEL(TRUE_CH(ch)), TRUE_CH(ch)->incog_level);
	if(IS_SET(cd->flags,CHANFLAG_MYSTERY_IMMS)){
		invis_imm=(IS_IMMORTAL(ch) && TRUE_CH(ch)
			&& !IS_SET(TRUE_CH(ch)->comm, COMM_WHOVIS) && invis_level);	
	}else{
		invis_imm=false;
	}


	text_displayed[0]='\0';
	// display what is being sent using the you_format to the talker
	// if there is no 'you_format' set for this channel, 
	// they are included in the loop which  displays to all players
	if(!IS_NULLSTR(cd->you_format)){ 
		// prefix with the clan details
		if(IS_SET(cd->flags,CHANFLAG_CLANMEMBERS_ONLY) && ch->clan){
			strcat(text_displayed, FORMATF("%s%s`x", ch->clan->color_str(), ch->clan->cwho_name()));
		}

		if(invis_imm){
			strcat(text_displayed, FORMATF("`=o[%d] ", invis_level));
			strcat(text_displayed, FORMATF(cd->you_format, message));
			strcat(text_displayed, FORMATF("`=o%s\r\n", cd->trailer));
		}else{
			strcat(text_displayed, cd->colour);
			strcat(text_displayed, FORMATF(cd->you_format, message));
			strcat(text_displayed, FORMATF("%s%s\r\n", cd->colour, cd->trailer));
		}

		if(!IS_NULLSTR(text_displayed)){
			ch->print(text_displayed);

			// record it in the receivers replay channels buffer
			if(ch->pcdata){
				replace_string(ch->pcdata->replaychannels_text[ch->pcdata->next_replaychannels], 
					FORMATF("%s> %s", shorttime(NULL), text_displayed));
				++ch->pcdata->next_replaychannels%=MAX_REPLAYCHANNELS;

			}
		}
	}
	
	char name[MSL];
	char *tname;
	if(IS_SET(cd->flags, CHANFLAG_USE_PERS_IF_SWITCHED) && IS_CONTROLLED(ch)){
		tname=CPERS(ch, NULL);
	}else{
		tname=TRUE_CH(ch)->name;
	}
	if(IS_NULLSTR(immtalk_name)){
		strcpy(name, tname);
	}else{
		sprintf(name, "%s`^`S(%s)`^", tname, immtalk_name);
	}


	sprintf(buf, cd->format, name, message);
	strcpy(colour_free_message, strip_colour(message));
	sprintf(colour_free_buf, cd->format, name, colour_free_message);

	// loop thru all the players, sending the channel text to those appropriate
	char_data *a; // a is the active char, who the channel is displayed to
	for ( c = player_list; c; c=c->next_player )
	{
		
		a=ACTIVE_CH(c);
		if ( (a == ch && !IS_NULLSTR(cd->you_format))){
			continue;
		}

		// check if they can see the channel
		if(!cd->can_see_channel(c, ch)){
			continue;
		}

		text_displayed[0]='\0';
		
		if(invis_imm)
		{
			if(can_see_who(c, TRUE_CH(ch))){
				if(IS_IMMORTAL(c)){
					strcat(text_displayed, FORMATF("`=o[%d] ", invis_level)); // display the level
					if(channel_colour_disabled(c,cd->channel_flag_value)){
						strcat(text_displayed, colour_free_buf);
					}else{
						strcat(text_displayed, buf);
					}
					strcat(text_displayed, FORMATF("`=o%s\r\n", cd->trailer));
				}else{
					if(channel_colour_disabled(c,cd->channel_flag_value)){
						strcat(text_displayed, colour_free_buf);
					}else{
						strcat(text_displayed, buf);
					}
					strcat(text_displayed, FORMATF("%s%s\r\n", cd->colour, cd->trailer));
				}
			}else{
				// prefix with the clan details
				if(IS_SET(cd->flags,CHANFLAG_CLANMEMBERS_ONLY) && ch->clan){
					strcat(text_displayed, FORMATF("%s%s`x", ch->clan->color_str(), ch->clan->cwho_name()));
				}
				strcat(text_displayed, cd->colour);
				if(channel_colour_disabled(c,cd->channel_flag_value)){
					strcat(text_displayed, FORMATF(cd->format, "mystery imm", colour_free_message));
				}else{
					strcat(text_displayed, FORMATF(cd->format, "mystery imm", message));
				}
				strcat(text_displayed, FORMATF("%s%s\r\n", cd->colour, cd->trailer));
			}
		}
		else // main text for when person is visible 
		{
			// prefix with the clan details
			if(IS_SET(cd->flags,CHANFLAG_CLANMEMBERS_ONLY) && ch->clan){
				strcat(text_displayed, FORMATF("%s%s`x", ch->clan->color_str(), ch->clan->cwho_name()));
			}
			strcat(text_displayed, cd->colour);
			if(channel_colour_disabled(c,cd->channel_flag_value)){
				strcat(text_displayed, colour_free_buf);
			}else{
				strcat(text_displayed, buf);
			}
			strcat(text_displayed, FORMATF("%s%s\r\n", cd->colour, cd->trailer));
		}

		if(!IS_NULLSTR(text_displayed)){
			bool heard=false;
			// display the channel to the player, unless they have the channel turned 
			// off or quiet turned on... regardless, we record what was said on the channel
			if(!HAS_CHANNELOFF(c, cd->channel_flag_value) 
				&& !(HAS_CHANNELOFF(c, CHANNEL_QUIET) && !IS_SET(cd->flags,CHANFLAG_IGNORE_QUIET))){
				a->print(text_displayed);
				heard=true;
			}

			// record it in the receivers replay channels buffer
			if(a->pcdata){
				replace_string(a->pcdata->replaychannels_text[a->pcdata->next_replaychannels], 
					FORMATF("%s%s%s %s", heard?"`X":"`S", shorttime(NULL), heard?">":"]", text_displayed));
				++a->pcdata->next_replaychannels%=MAX_REPLAYCHANNELS;

			}
		}
    }

	// handle the logging of the channel

	// memory circular buffer logging of the channel
	if(cd->memory_log_lines){
		++cd->memory_log_index%=cd->memory_log_lines; // rotate the buffer
		sprintf( cd->memory_log[cd->memory_log_index], "`S%s> %s%s%s%s`x", 
			shorttime(NULL), cd->colour, buf, cd->colour, cd->trailer); 
	}

	// logging to file
	if(!IS_NULLSTR(cd->logfile)){
		strcat(buf, "`x");
		append_datetimestring_to_file(cd->logfile, buf);
	}
}
/**************************************************************************/
void info_broadcast(char_data *ch, const char * fmt, ...)
{
	if(!GAMESETTING3(GAMESET3_INFO_BROADCASTS_ENABLED)){
		return;
	}
    char buf[MSL], buf2[MSL];
	char_data *wch;
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);

	sprintf(buf2, "`=\x8a[INFO: `=\x8b%s`=\x8a]`x", buf);

	for( wch = player_list; wch; wch = wch->next_player )
    {
		if(IS_IMMORTAL(ch) && !IS_IMMORTAL(wch)){
			continue;
		}

		if(	can_see(wch, ch) )
		{
			bool heard=false;
			if(!HAS_CHANNELOFF(wch, CHANNEL_QUIET) 
				&& !HAS_CHANNELOFF(wch, CHANNEL_INFO))
			{
				wch->println(buf2);
				heard=true;
			}

			// record the broadcast in the receivers replay channels buffer
			if(wch->pcdata){
				replace_string(wch->pcdata->replaychannels_text[wch->pcdata->next_replaychannels], 
					FORMATF("%s%s%s %s\r\n", heard?"`X":"`S", shorttime(NULL), heard?">":"]", buf2));
				++wch->pcdata->next_replaychannels%=MAX_REPLAYCHANNELS;
			}
		}
	}
}
/**************************************************************************/
int channel_exact_lookup(const char *channel_name)
{
	int i;
	for(i=0; !IS_NULLSTR(channels[i].name); i++){
		if(!str_cmp(channel_name, channels[i].name)){
			return i;
		}
	}
	return -1;
}
/**************************************************************************/
int channel_lookup(const char *channel_name)
{
	int i;

	// do exact match first
	i=channel_exact_lookup(channel_name);
	if(i>-1){
		return i;
	}
	
	for(i=0; !IS_NULLSTR(channels[i].name); i++){
		if(!str_prefix(channel_name, channels[i].name)){
			return i;
		}
	}
	return -1;
}
/**************************************************************************/
// convert bits into the corresponding channel names
char *channel_convert_bitflags_to_text(long bits)
{
	static int i;
    static char buf[3][MIL];
	++i%=3;

	char *r=buf[i];

	r[0]='\0';
	r[1]='\0';

	int j;
	for(j=0; !IS_NULLSTR(channels[j].name); j++){
		if(IS_SET(bits, channels[j].channel_flag_value)){
			strcat(r, " ");
			strcat(r, pack_word(channels[j].name));
		}
	}
	return &buf[i][1];
}
/**************************************************************************/
// convert bits into the corresponding channel names
long channel_convert_text_to_bitflags(char *text)
{
	char name[MIL];
	long result=0;
	int i;

	while(!IS_NULLSTR(text)){
		text=one_argument(text,name);
		i=channel_exact_lookup(name);

		if(i>=0){
			SET_BIT(result, channels[i].channel_flag_value);
		}else{			logf("channel_convert_text_to_bitflags(): Unrecognised text name '%s' - ignoring",
				name);
		}
	}	
	return result;
}

/**************************************************************************/
// setups up the global channel number (gcn_) int pointers 
void channels_initialize()
{
	for(int i=0; !IS_NULLSTR(channels[i].name); i++){
		*channels[i].index=i;
		if(channels[i].memory_log_lines){
			int l=channels[i].memory_log_lines;
			channels[i].memory_log=(char **)malloc( sizeof(char*)*l);
			for(int j=0; j<l; j++){
				channels[i].memory_log[j]=(char *)malloc( MIL+150);
				channels[i].memory_log[j][0]='\0';
			}
		}
	}
}
/**************************************************************************/
// Kal - September 01
void channel_generic_replay(char_data *ch, int channel_index)
{
	ch->titlebarf("CHANNEL REPLAY: %s", uppercase(channels[channel_index].name));
	channel_data *cd=&channels[channel_index];

	bool found=false;
	
	for(int i=(cd->memory_log_index+1)%cd->memory_log_lines; 
				i!=cd->memory_log_index; 
				i= (i+1)%MAX_CHECK_IMMTALK){
		if(!IS_NULLSTR(cd->memory_log[i])){
			ch->printf("%s\r\n", cd->memory_log[i]);
			found=true;
		}
	}
	if(!IS_NULLSTR(cd->memory_log[cd->memory_log_index])){
		ch->printf("%s\r\n", cd->memory_log[cd->memory_log_index]);
		found=true;
	}
	if(!found){
		ch->printlnf("There hasn't been anything said on '%s' since the last reboot.", cd->name);
	}else{
		ch->printlnf("Current time: %s   Your Login Time: %s",
			shorttime(NULL), shorttime(&ch->logon));
	}
}
/**************************************************************************/
int channel_find_replay_channel(char_data *ch, char *channame)
{
	for(int i=0; !IS_NULLSTR(channels[i].name); i++){
		if(!channels[i].can_see_channel(ch, NULL)){
			continue;
		}
		if(channels[i].memory_log_lines){
			if(!str_prefix(channame, channels[i].name)){
				return i;
			}
		}
	}
	return -1;
}

/**************************************************************************/
// Kal - Sept 01
void channel_replay(char_data *ch, char *argument)
{
	int display=0;
	int ci=channel_find_replay_channel(ch, argument);
	if(ci<0){
		ch->printlnf("'%s' is an unknown channel name, try one of the following:", argument);
		ch->println("Where <channel> is one of the following:");
		for(int i=0; !IS_NULLSTR(channels[i].name); i++){
			if(!channels[i].can_see_channel(ch, NULL)){
				continue;
			}
			if(channels[i].memory_log_lines){
				ch->printf("  %-20.20s ", channels[i].name);
				if(++display%3==0){
					ch->print_blank_lines(1);
				}
			}
		}
		if(display%3!=0){
			ch->print_blank_lines(1);
		}
		return;
	}
	channel_generic_replay(ch, ci);
}
/**************************************************************************/
// Kal - April 03
void do_replayroom(char_data *ch, char *argument)
{
	assertp(ch);
	pc_data *pc= TRUE_CH(ch)->pcdata;
	if(!pc){
		ch->println( "Uncontrolled mobs can't replay room events.");
		return;    
	}

	bool found=false;
	int end=(pc->next_replayroom-1+MAX_REPLAYROOM)%MAX_REPLAYROOM;
	for(int i=pc->next_replayroom; i!=end; i= (i+1)%MAX_REPLAYROOM){
		if(!IS_NULLSTR(pc->replayroom_text[i])){
			if(!found){
				ch->titlebar("REPLAYROOM");
			}
			ch->println(pc->replayroom_text[i]);
			found=true;
		}
	}
	if(!IS_NULLSTR(pc->replayroom_text[end])){
		if(!found){
			ch->titlebar("REPLAYROOM");
		}
		ch->println(pc->replayroom_text[end]);
		found=true;
	}
	if(!found){
		ch->println("You haven't seen/heard any room events since you logged on.");
	}

	if(IS_NEWBIE(ch)){
		ch->println("note: you can also replay your tells by typing 'replay'");
		ch->println("      and replay channel events by typing 'replaychannels'");
	}
}
/**************************************************************************/
// Kal - September 03
void do_replaychannels(char_data *ch, char *argument)
{
	assertp(ch);
	pc_data *pc= TRUE_CH(ch)->pcdata;
	if(!pc){
		ch->println( "Uncontrolled mobs can't replay channel events.");
		return;    
	}

	bool found=false;
	int end=(pc->next_replaychannels-1+MAX_REPLAYCHANNELS)%MAX_REPLAYCHANNELS;
	for(int i=pc->next_replaychannels; i!=end; i= (i+1)%MAX_REPLAYCHANNELS){
		if(!IS_NULLSTR(pc->replaychannels_text[i])){
			if(!found){
				ch->titlebar("REPLAYCHANNELS");
			}
			ch->print(pc->replaychannels_text[i]);
			found=true;
		}
	}
	if(!IS_NULLSTR(pc->replaychannels_text[end])){
		if(!found){
			ch->titlebar("REPLAYCHANNELS");
		}
		ch->print(pc->replaychannels_text[end]);
		found=true;
	}
	if(!found){
		ch->println("You haven't seen/heard any channel events since you logged on.");
	}

	if(IS_NEWBIE(ch)){
		ch->println("note: you can also replay your tells by typing 'replay'");
		ch->println("      and replay room events typing 'replayroom'");
	}
}
/**************************************************************************/
// Kal - Jan 00
void do_replay(char_data *ch, char *argument)
{
	assertp(ch);
	pc_data *pc= TRUE_CH(ch)->pcdata;
	if(!pc){
		ch->println( "Uncontrolled mobs can't replay their tells.");
		return;    
	}

	if(IS_IMMORTAL(ch) && !IS_NULLSTR(argument)){
		channel_replay(ch, argument);
		return;
	}
	
	bool found=false;
	int end=(pc->next_replaytell-1+MAX_REPLAYTELL)%MAX_REPLAYTELL;
	for(int i=pc->next_replaytell; i!=end; i= (i+1)%MAX_REPLAYTELL){
		if(!IS_NULLSTR(pc->replaytell_text[i])){
			if(!found){
				ch->titlebar("TELL REPLAY");
			}
			ch->print(pc->replaytell_text[i]);
			found=true;
		}
	}
	if(!IS_NULLSTR(pc->replaytell_text[end])){
		if(!found){
			ch->titlebar("TELL REPLAY");
		}
		ch->print(pc->replaytell_text[end]);
		found=true;
	}
	if(!found){
		ch->println("You haven't received any tells since you logged on.");
	}

	if(IS_NEWBIE(ch)){
		ch->println("note: you can also replay room events - by typing 'replayroom'");
		ch->println("      and replay channel events by typing 'replaychannels'");
	}
	if(IS_IMMORTAL(ch) && IS_NULLSTR(argument)){
		ch->wrapln("immnote: you can search individual channels earlier "
			"than your login by typing 'replay <channelname>'... the "
			"list of channel names will be displayed if you try a channel "
			"name that doesn't exist - e.g. 'replay x'");			
	}
}
/**************************************************************************/
void do_chat( char_data *ch, char *argument )
{
	if(GAMESETTING4(GAMESET4_REDIRECT_CHANNEL_CHAT_TO_OOC)){
		channel_generic_handler( gcn_ooc, ch, argument);
	}else{
		channel_generic_handler( gcn_chat, ch, argument);
	}
}
/**************************************************************************/
void do_ooc( char_data *ch, char *argument )
{
	if(GAMESETTING4(GAMESET4_REDIRECT_CHANNEL_OOC_TO_CHAT)){
		channel_generic_handler( gcn_chat, ch, argument);
	}else{
		channel_generic_handler( gcn_ooc, ch, argument);
	}
}
/**************************************************************************/
// in character channel
void do_ic( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_ic, ch, argument);
}
/**************************************************************************/
void do_grats( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_grats, ch, argument);
}
/**************************************************************************/
void do_flame( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_flame, ch, argument);
}

/**************************************************************************/
void do_question( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_question, ch, argument);
}
/**************************************************************************/
void do_answer( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_answer, ch, argument);
}

/**************************************************************************/
void do_immtalk( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_immtalk, ch, argument);
}
/**************************************************************************/
void do_codetalk( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_codetalk, ch, argument);
}
/**************************************************************************/
void do_mythostalk( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_mythostalk, ch, argument);
}
/**************************************************************************/
void do_newbietalk( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_newbietalk, ch, argument);
}
/**************************************************************************/
void do_admintalk( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_admintalk, ch, argument);
}
/**************************************************************************/
void do_hightalk( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_hightalk, ch, argument);
}
/**************************************************************************/
void do_clantalk( char_data *ch, char *argument )
{
	channel_generic_handler( gcn_clantalk, ch, argument);
}
/**************************************************************************/
// Kal - Aug 04
void do_replaychar(char_data *ch, char *argument)
{
	char_data *victim;
	if(IS_NULLSTR(argument)){
		ch->println("syntax: replaychar <playername>");
		return;
	}
	victim=get_whovis_player_world(ch, argument);

	if(!victim){
		ch->printlnf("Couldn't find any '%s' to replace channels on.", argument);
		return;
	}

	if(  get_trust( victim ) >= get_trust( ch )
		|| (IS_SET(victim->comm,COMM_SNOOP_PROOF) && !IS_TRUSTED(ch, MAX_LEVEL)) )
	{
		ch->printlnf("You failed.");
		return;
	}

	pc_data *pc= victim->pcdata;

	if(!pc){
		ch->printlnf("Error, no PCDATA found on %s.", victim->name);
		return;
	}

	bool found=false;
	int end=(pc->next_replaytell-1+MAX_REPLAYTELL)%MAX_REPLAYTELL;
	for(int i=pc->next_replaytell; i!=end; i= (i+1)%MAX_REPLAYTELL){
		if(!IS_NULLSTR(pc->replaytell_text[i])){
			if(!found){
				ch->titlebarf("TELL REPLAY - %s", uppercase(victim->name));
			}
			ch->print(pc->replaytell_text[i]);
			found=true;
		}
	}
	if(!IS_NULLSTR(pc->replaytell_text[end])){
		if(!found){
			ch->titlebarf("TELL REPLAY - %s", uppercase(victim->name));
		}
		ch->print(pc->replaytell_text[end]);
		found=true;
	}
	if(!found){
		ch->printlnf("%s hasn't received any tells since they logged on.", capitalize(victim->name));
	}
}
/**************************************************************************/
/**************************************************************************/

