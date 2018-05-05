/**************************************************************************/
// act_wiz.cpp - immortal commands
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
#include "olc.h"
#include "nanny.h"
#include "msp.h"
#include "magic.h"

// this section is for the global debugging in do_owhere() and do_mwhere()
#include "db.h"
extern int                     nAllocString;
extern int                     sAllocString;
extern int                     nAllocPerm;
extern int                     sAllocPerm;
char *fwrite_flag( long flags, char buf[] );
void forced_interpret( char_data *ch, char *argument );

/* command procedures needed */
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_resetlu	);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_forcetick );
DECLARE_DO_FUN(do_quit      );
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_count     );

char *get_weapontype(OBJ_DATA *obj); // prototype - handler.c
void laston_save	(char_data *);			// laston.c 
void dismount     args( ( char_data *) );	// act_move.c
void send_zmudmap_to_char(char_data *ch);

void laston_update_char(char_data *ch);

//char *full_affect_loc_name( AFFECT_DATA *paf);
/*
 * Local functions.
 */

/**************************************************************************/
void flush_cached_write_to_buffer(connection_data *d);
long wiznet_level_mask[MAX_LEVEL+2];
/**************************************************************************/
void wiznet(const char *string, char_data *ch, OBJ_DATA *obj,
		 long flag, long flag_skip, int min_level)
{
    connection_data *d;
	static bool generate_wiznet_level_table = true;

	if(generate_wiznet_level_table){
		// wiznet_level_mask[] system used for when an imm has 
		// wiznet flags on settings above their level.
		memset(wiznet_level_mask, 0, sizeof(wiznet_level_mask));
		for(int wi= 0; !IS_NULLSTR(wiznet_table[wi].name); wi++){
			for(int wl=ABSOLUTE_MAX_LEVEL; wl>=wiznet_table[wi].level; wl--){
				SET_BIT(wiznet_level_mask[wl], wiznet_table[wi].flag);
			}
		}
		generate_wiznet_level_table=false;
	}

	// silently will allow higher level imms to hide 
	// wiz_questing messages from lower level imms
	if (IS_SET(WIZ_QUESTING,flag) && TRUE_CH(ch))
	{
		if (IS_SILENT(ch))
		{
			min_level = get_trust(ch);
		}
	}


#ifdef MEM_DEBUG
	if (log_memory){
		if (IS_SET(WIZ_MEMCHECK,flag))
		{
			logf(string);
		}
	}
#endif

    for ( d = connection_list; d; d = d->next )
    {
        if (d->connected_state == CON_PLAYING
			&&  IS_IMMORTAL(CH(d))
			&&  get_trust(CH(d)) >= min_level
			&&  d->character!=ch)
		{
			int t;
			for(t=0; t<4;t++){ // check till we find a matching flag
				if(t==0 && (ch==NULL || !IS_IMMORTAL(ch))){
					continue;
				}
				if(IS_SET(CH(d)->wiznet[t],WIZ_ON)) {
					if( ( !flag || IS_SET( (CH(d)->wiznet[t] & wiznet_level_mask[get_trust(CH(d))]),flag) )
						&& ( !flag_skip || !IS_SET(CH(d)->wiznet[t],flag_skip) )
					  )
					{
						break;
					}
				}
			}
			if(t==4){ // didn't find a match for this person
				continue;
			}else{
				int prevtop;
				flush_cached_write_to_buffer(d);
				prevtop=d->outtop;
			
				if (IS_SET(CH(d)->wiznet[t],WIZ_SHOWCHANNEL)){
					if(IS_NULLSTR(CH(d)->wiznet_colour[t])){
						d->character->printf("--%s--%s--> ",
							shorttime(NULL), 
							flag_string( wiznet_flags, flag));
					}else{
						d->character->printf("`#%s--%s--%s-->`^ ", 
							CH(d)->wiznet_colour[t], 							 
							shorttime(NULL),
							flag_string( wiznet_flags, flag));
					}
				}else{
					if(IS_NULLSTR(CH(d)->wiznet_colour[t])){
						d->character->printf("--%s--> ", 
							shorttime(NULL));
					}else{
						d->character->printf("`#%s--%s-->`^ ", 
							CH(d)->wiznet_colour[t], shorttime(NULL));
					}
				}

				act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
				d->character->print("`x");
			}
        }
    }

    if (IS_SET(WIZ_SECURE,flag)){
        append_datetimestring_to_file( SECURE_FILE, string);
    }   
 
    return;
}

/**************************************************************************/
void do_short( char_data *ch, char *argument)
{
	char_data *victim;
	char arg[MIL];
	char oarg[MSL];

	sprintf( oarg,"%s", argument);
	
	argument = one_argument( argument, arg );
	
	if(IS_NULLSTR(arg)){
		ch->println("Syntax: short <playername> a short description for the player");
		ch->println("This command is used to set short descriptions.");
		return;
	}
	
	if(str_len(arg)<3){
		ch->println("You must use more than 2 characters to specify players name.");
		return;
	}

    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
	}

    // trim the spaces to the right of the short
    while ( !IS_NULLSTR(argument) && is_space(argument[str_len(argument)-1]))
    {
        argument[str_len(argument)-1]='\0';
    }

    // check we have a short left 
    if(IS_NULLSTR(argument)){
        ch->println("You must put in a short description.");
        return;
	}

    // make sure first char is lowercase, if the short description isn't the name
	if(str_cmp(argument, victim->name)){
		argument[0] = LOWER(argument[0]);
	}

    if ((get_trust(victim)>= get_trust(ch))&& (ch != victim))
        ch->println("You can't set a short of someone a higher level or equal to you.");
    else
    {
		// questing wiznet
		if (TRUE_CH(ch))
		{
			char qbuf[MSL];
			sprintf (qbuf, "`mQUEST> %s short %s`x\r\n", 
				TRUE_CH(ch)->name, oarg); 
			wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
		}

        // inform the imm and the room victim is in 
        ch->wraplnf("You have changed %s's short description from '%s' to '%s'.",
			victim->name, victim->short_descr, argument);

        act( "$n is now known as '$T'", victim, NULL, argument, TO_ROOM );

        // change the short 
        replace_string(victim->short_descr, argument);
		
        victim->wraplnf("Your short description has been changed to '%s' by %s.",
			victim->short_descr, can_see_who(victim, ch)?ch->name:"a mystery imm");

        if (str_len(argument)>55){
			ch->wrapln( "That short descriptions is LONG... it has been set anyway..."
				"but consider the length when setting a short." );
		}
    }

    save_char_obj( victim );
	return;
}

/**************************************************************************/
void do_page (char_data *ch, char *argument)
{
    char_data *victim;
	connection_data *c;

			
    if (IS_NULLSTR(argument))
    {
        ch->println( "Page which name from the who list/which connection number in sockets?" );
        return; 
    }

	if(is_number(argument)){
		int desc;
		desc = atoi(argument);
    	for ( c = connection_list; c; c = c->next )
    	{
			
			if ( c->connected_socket == desc )
			{
				
				ch->printlnf( "Paging connection %d (%s).", 
					desc, CH(c)?CH(c)->name:"???");
				c->write("\7\7\7You are being PAGED!!!\7\7\7\r\n",0);
            	return;
			}
		}
		ch->printlnf( "No one with the connection number %d was found to page.", desc);
		return;
	}

    if ((victim=get_whovis_player_world(ch, argument))==NULL){
        ch->printlnf( "You can't find any '%s' to page.", argument);
        return;
    }

	if(can_see(ch, victim)){
		ch->printlnf( "Paging %s.", PERS(victim, ch));
	}else{
		ch->printlnf( "Paging %s.", victim->name);
	}
    ACTIVE_CH(victim)->print( "\7\7\7" );
    ACTIVE_CH(victim)->printlnf( "You are being paged by %s!!!", PERS(TRUE_CH(ch), victim));
    ACTIVE_CH(victim)->println( "\a" );
}

/**************************************************************************/
void do_norp( char_data *ch, char *argument )
{
	char arg[MIL],buf[MSL];
	char_data *victim;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
    {
		ch->println( "Norp whom?" );
		return;
	}

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println( "Not on NPC's." );
        return;
    }
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        ch->println( "You failed." );
		return;
	}

    if ( IS_SET(victim->act, PLR_NORP) )
    {
        REMOVE_BIT(victim->act, PLR_NORP);
        victim->println( "You may get RP EXP again." );
        ch->println( "NORP removed." );
        sprintf(buf,"$N allows %s RP XP again.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->act, PLR_NORP);
		ch->println( "NORP set." );
        sprintf(buf,"$N gives %s the dreaded NORP.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );
    return;
}
/**************************************************************************/
void wiznet_index( int index, char_data *ch, char *argument )
{
	int flag, col=0;
    char buf[MSL], buf2[MSL];
	
	char wiznet_name[MIL];
	switch(index){
	case 0: sprintf(wiznet_name,"immwiznet"); break;
	case 1: sprintf(wiznet_name,"wiznet"); break;
	case 2: sprintf(wiznet_name,"wiznet2"); break;
	case 3: sprintf(wiznet_name,"wiznet3"); break;
	default:
		sprintf(wiznet_name,"UNKNOWN WIZNET INDEX %d", index); break;		
	}
	
	
    if ( IS_NULLSTR(argument) ) // display status 
    {
        buf[0] = '\0';
        strcat(buf,"   ---=========== Current ");
		strcat(buf, capitalize(wiznet_name));
		strcat(buf," Settings ===========---\r\n");
		
        for (flag = 1; wiznet_table[flag].name != NULL; flag++)
        {
            if (wiznet_table[flag].level <= get_trust(ch))
            {
                sprintf(buf2,"      %-15s %s",wiznet_table[flag].name,
					(IS_SET(TRUE_CH(ch)->wiznet[index],wiznet_table[flag].flag)?"on   ":"off  "));
                strcat(buf,buf2);
                col++;
                if (col%2==0){
                    strcat(buf,"\r\n");
				}
				
            }
        }
        if (IS_SET(TRUE_CH(ch)->wiznet[index],WIZ_ON)){
            strcat(buf,"\r\n      (You currently have this wiznet active)\r\n");
        }else{
            strcat(buf,"\r\n       (You currently have this wiznet OFF)\r\n");
        }
        ch->printf("%s",buf);
		if(IS_NULLSTR(TRUE_CH(ch)->wiznet_colour[index])){
			sprintf(buf,       "        %s colour prefix is unset.\r\n", wiznet_name);
		}else{
			sprintf(buf,       "        %s colour prefix is set to '%s'\r\n", wiznet_name, 
				TRUE_CH(ch)->wiznet_colour[index]);
		}
		ch->printbw(buf);
		
        return;
	}
	
    if (!str_prefix(argument,"on"))
    {
		if(index){
			ch->printlnf("Welcome to Wiznet%d!", index);
		}else{
			ch->println("Welcome to ImmWiznet!");
		}
        SET_BIT(TRUE_CH(ch)->wiznet[index],WIZ_ON);
        return;
    }
	
    if (!str_prefix(argument,"off"))
    {
		if(index){
		    ch->printlnf("Signing off of Wiznet%d.", index);
		}else{
	        ch->println("Signing off of ImmWiznet.");
		}
        REMOVE_BIT(TRUE_CH(ch)->wiznet[index],WIZ_ON);
        return;
    }
    flag = wiznet_lookup(argument);
	
    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
		ch->println("No such option.");
		return;
    }
	
    if (IS_SET(ch->wiznet[index],wiznet_table[flag].flag))
    {
		ch->printlnf("You will no longer see %s on %s.", 
			wiznet_table[flag].name, wiznet_name);
		REMOVE_BIT(TRUE_CH(ch)->wiznet[index],wiznet_table[flag].flag);
		return;
    }
	else
	{
		ch->printlnf("You will now see %s on %s.", 
			wiznet_table[flag].name, wiznet_name);
		SET_BIT(TRUE_CH(ch)->wiznet[index],wiznet_table[flag].flag);
		return;
	}
}
/**************************************************************************/
void wiznet_colourindex( int index, char_data *ch, char *argument )
{
	if(!IS_NULLSTR(TRUE_CH(ch)->wiznet_colour[index])){
		free_string(TRUE_CH(ch)->wiznet_colour[index]);
	}
	smash_tilde( argument);
	TRUE_CH(ch)->wiznet_colour[index]=str_dup(argument);

	char wiznet_name[MIL];
	switch(index){
	case 0: sprintf(wiznet_name,"immwiznet"); break;
	case 1: sprintf(wiznet_name,"wiznet"); break;
	case 2: sprintf(wiznet_name,"wiznet2"); break;
	case 3: sprintf(wiznet_name,"wiznet3"); break;
	default:
		sprintf(wiznet_name,"UNKNOWN WIZNET INDEX %d", index); break;		
	}

	char buf[MSL];
	sprintf(buf,"%s colour prefix set to '%s'\r\n", wiznet_name, argument);
	ch->printbw(buf);
	ch->println("Note, you don't have to put only colour codes in the prefix.");	
}
/**************************************************************************/
void do_immwiznet( char_data *ch, char *argument )
{
	wiznet_index(0, ch, argument);
}
/**************************************************************************/
void do_wiznet( char_data *ch, char *argument )
{
	wiznet_index(1, ch, argument);
}
/**************************************************************************/
void do_wiznet2( char_data *ch, char *argument )
{
	wiznet_index(2, ch, argument);
}
/**************************************************************************/
void do_wiznet3( char_data *ch, char *argument )
{
	wiznet_index(3, ch, argument);
}
/**************************************************************************/
void do_immwiznetc( char_data *ch, char *argument )
{
	wiznet_colourindex(0, ch, argument);
}
/**************************************************************************/
void do_wiznetc( char_data *ch, char *argument )
{
	wiznet_colourindex(1, ch, argument);
}
/**************************************************************************/
void do_wiznet2c( char_data *ch, char *argument )
{
	wiznet_colourindex(2, ch, argument);
}
/**************************************************************************/
void do_wiznet3c( char_data *ch, char *argument )
{
	wiznet_colourindex(3, ch, argument);
}
/**************************************************************************/
void do_wiznetdefault( char_data *ch, char *argument )
{
	if(IS_NPC(ch)){
		ch->println("Players only.");
		return;
	}
	if(IS_NULLSTR(argument)){
		ch->println("Wiznetdefault - sets a default setting of wiznet flags.");
		ch->println("syntax: '`=Cwiznetdefault confirm`x'");
		return;
	}

	if ( str_cmp("confirm", argument)) {
		ch->wrapln("You must confirm your intention to overwrite your "
			"wiznet settings with the default settings.");
		do_wiznetdefault(ch,"");
		return;
	}

	ch->wiznet_colour[0]=str_dup("`R###IMM###"); // immortal
	ch->wiznet[0]= WIZ_ON | WIZ_LOGINS	| WIZ_SITES	| WIZ_LINKS | WIZ_SECURE 
				       | WIZ_SNOOPS | WIZ_AUTOON | WIZ_SHOWCHANNEL | WIZ_QUESTING;
	ch->wiznet_colour[1]=str_dup("`S`#");
	ch->wiznet[1]= WIZ_ON | WIZ_LOGINS	| WIZ_SITES	| WIZ_LINKS | WIZ_DEATHS 
		                  | WIZ_FLAGS	| WIZ_THEFTS| WIZ_LEVELS| WIZ_SECURE 
						  | WIZ_SNOOPS	| WIZ_AUTOON| WIZ_NEWBIE| WIZ_SHOWCHANNEL
						  | WIZ_BUGS	| WIZ_WHISPERS | WIZ_NOHELP | WIZ_QUESTING
						  | WIZ_PRAYERS_DREAMS;
	ch->wiznet_colour[2]=str_dup("`S`#");	
	ch->wiznet[2]= WIZ_LEVELS | WIZ_SHOWCHANNEL;
	ch->wiznet_colour[3]=str_dup("");	
	ch->wiznet[3]=0;
	ch->println("Default wiznet settings applied to immwiznet, wiznet1, wiznet2 and wiznet3.");
	ch->println("(this includes wiznet colours prefixes).");
}

/**************************************************************************/
/* RT nochannels command, for those spammers */
void do_nochannels( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
	char_data *victim;
 
	// can only be used by imms or those with permission
	if (  !IS_IMMORTAL(ch) 
		&& !IS_SET(ch->comm, COMM_CANNOCHANNEL) 
		&& !(ch->pcdata && ch->pcdata->immtalk_name))
	{
		do_huh(ch,"");
		return;
	}
	
	one_argument( argument, arg );
 
    if ( IS_NULLSTR(arg))
	{
        ch->println( "Nochannel whom?" );
        return;
    }
 
    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }
 
    // can't nochannel high level imms than yourself
	if ( victim!= ch && 
		get_trust( victim ) >= LEVEL_IMMORTAL 
		&& get_trust( victim ) >= get_trust( ch ) )
    {
        ch->println( "You failed." );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
		logf("nochannel removed on %s by %s", victim->name, ch->name);
        victim->println( "The gods have restored your channel priviliges." );
        ch->println( "NOCHANNELS removed." );
		sprintf(buf,"$N restores channels to %s",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
		logf("nochannel added on %s by %s", victim->name, ch->name);
        victim->println( "The gods have revoked your channel priviliges." );
        ch->printlnf("NOCHANNELS set on %s.", victim->name);

		if (!IS_SET(victim->act, PLR_LOG))
		{
			SET_BIT(victim->act, PLR_LOG);
			sprintf( buf, "(nochannel) Player log turned ON by %s", ch->name);
			append_playerlog( victim, buf);
			if (IS_IMMORTAL(ch) &&
				(get_trust( victim ) < LEVEL_IMMORTAL
				|| IS_ADMIN(ch)) )
			{
				ch->printlnf("PLAYERLOG turned on %s.", victim->name);
			}
		}
		sprintf(buf,"$N revokes %s's channels.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}


/**************************************************************************/
void do_bamfin( char_data *ch, char *argument )
{
	if ( !IS_NPC(ch) )
	{
		smash_tilde( argument );
		if (argument[0] == '\0')
		{
			ch->printlnf("Your poofin is %s",ch->pcdata->bamfin);
			return;
		}

		free_string( ch->pcdata->bamfin );
		ch->pcdata->bamfin = str_dup( argument );
        
		ch->printlnf("Your poofin is now %s",ch->pcdata->bamfin);
	}
	return;
}

/**************************************************************************/
void do_bamfout( char_data *ch, char *argument )
{
	if ( !IS_NPC(ch) )
	{
		smash_tilde( argument );
		if (argument[0] == '\0')
		{
			ch->printlnf("Your poofout is %s",ch->pcdata->bamfout);
			return;
		}

		free_string( ch->pcdata->bamfout );
		ch->pcdata->bamfout = str_dup( argument );
		ch->printlnf("Your poofout is now %s",ch->pcdata->bamfout);
	}
	return;
}

/**************************************************************************/
void do_chardescript( char_data *ch, char *argument )
{
	char_data *victim;
	char arg[MIL];

    if (!IS_IMMORTAL(ch) && !IS_NEWBIE_SUPPORT(ch))
    {
		do_huh(ch,"");
		return;	
    }

    // keep a track of what newbie support is doing 
    if (IS_NEWBIE_SUPPORT(ch))
    {
        append_newbie_support_log(ch, argument);
    }

	argument = one_argument( argument, arg );

	if(IS_NULLSTR(arg)){
        ch->println( "Syntax: chardesc <player|mobname>" );
        return;
	}

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

    if (!IS_TRUSTED(ch,MAX_LEVEL-4) && !IS_NPC(victim) 
		&& IS_LETGAINED(victim)
		&& IS_TRUSTED(victim,20))
    {
        ch->println( "Sorry, you can't set a long description of a player who is letgained.  Get an admin immortal to do it." );
        return;
    }

    if (IS_NEWBIE_SUPPORT(ch) && IS_NPC(victim))
    {
        ch->println( "Sorry, you can't set the long description on a mob." );
        return;
    }


    if ((get_trust(victim)>= get_trust(ch))&& (ch != victim))
    {
        ch->println( "You can't set a long description of someone a higher level or equal to you." );
        return;
    }

    string_append(ch, &victim->description);
	ch->printlnf("Editing the long description of %s", PERS(victim, ch));
    return;
}


/**************************************************************************/
void do_deny( char_data *ch, char *argument )
{
	char arg[MIL],buf[MSL];
	char_data *victim;

	one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "Deny whom?" );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	if ( IS_NPC(victim) )
	{
		ch->println( "Not on NPC's." );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		ch->println( "You failed." );
		return;
	}

	SET_BIT(victim->act, PLR_DENY);
	victim->println( "You are denied access!" );
	sprintf(buf,"$N denies access to %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	ch->println( "OK." );
    save_char_obj(victim);
    stop_fighting(victim,true);
    do_quit( victim, "" );

    return;
}


/**************************************************************************/
void do_disconnect( char_data *ch, char *argument )
{
    char arg[MIL];
    connection_data *c, *c_next;
    char_data *victim;

    one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->println( "Disconnect whom?" );
		return;
	}

	if (is_number(arg))
	{
		int desc;

		desc = atoi(arg);
		
		for ( c = connection_list; c; c = c->next )
		{
			if ( c->connected_socket == desc )
			{
				connection_close( c );
				ch->println( "Ok." );
				return;
			}
		}
	}

    if ( ( victim = get_char_world( ch, arg ) ) == NULL ){
		ch->println( "They aren't here." );
		return;
	}

	if ( victim->desc == NULL )
	{
		ch->printlnf( "%s doesn't have a descriptor.", victim->name );
		return;
    }

    for ( c = connection_list; c; c = c_next )
    {
		c_next = c->next;
		if ( c == victim->desc )
		{
			if (get_trust(ch) <= get_trust(victim))
			{
				ch->println( "Maybe that wasn't a good idea..." );
				victim->printlnf( "%s tried to diconnect you!", ch->name );
				return;
			}

			connection_close( c );
			ch->println( "Ok." );
			return;
		}
    }

    bug("Do_disconnect: desc not found.");
    ch->println( "Descriptor not found!" );
    return;
}


/**************************************************************************/
void do_gecho( char_data *ch, char *argument )
{
    connection_data *d;
    
    if ( argument[0] == '\0' )
    {
		ch->println( "Global echo what?" );
		return;
    }

	// questing wiznet
	{
		char qbuf[MSL];
		sprintf (qbuf, "`mQUEST> %s gecho '%s'`x", ch->name, argument);
		wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
	}
    
    for ( d = connection_list; d; d = d->next )
    {
		if ( d->connected_state == CON_PLAYING )
		{
			if (get_trust(d->character) >= get_trust(ch))
				d->character->print( "global> " );

			d->character->printlnf( "%s`x", argument );
		}
	}

    return;
}

/**************************************************************************/
void do_goecho( char_data *ch, char *argument )
{
    connection_data *d;
    
    if ( argument[0] == '\0' )
    {
		ch->println( "Global outdoors echo what?" );
		return;
    }

	// questing wiznet
	{
		char qbuf[MSL];
		sprintf (qbuf, "`mQUEST> %s goecho '%s'`x", ch->name, argument);
		wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
	}
    
    for ( d = connection_list; d; d = d->next )
    {
		if ( d->connected_state == CON_PLAYING )
		{
			if (IS_OUTSIDE(d->character) && IS_AWAKE(d->character))
			{
				if ( d->character->in_room->sector_type == SECT_CAVE )
					continue;
				if (get_trust(d->character) >= get_trust(ch))
					d->character->print( "globaloutdoors> " );
				
				d->character->printlnf( "%s`x", argument );
			}
		}
	}

    return;
}

/**************************************************************************/
void do_echo( char_data *ch, char *argument )
{
    connection_data *d;
    
    if ( argument[0] == '\0' )
    {
		ch->println( "Local echo what?" );
		return;
    }

	// questing wiznet
	{
		char qbuf[MSL];
		sprintf (qbuf, "`mQUEST> %s echo ('%s' [%d]) %s`x", ch->name, 
			ch->in_room->name, ch->in_room->vnum, argument);
		wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
	}


    for ( d = connection_list; d; d = d->next )
    {
		if ( d->connected_state == CON_PLAYING
		&&   d->character->in_room == ch->in_room )
		{
			if (get_trust(d->character) >= get_trust(ch))
				d->character->print( "local> " );
			
			d->character->printlnf( "%s`x", argument );
		}
    }

    return;
}

/**************************************************************************/
void do_zecho(char_data *ch, char *argument)
{
    connection_data *d;

    if (argument[0] == '\0')
    {
		ch->println( "Zone echo what?" );
		return;
    }

	// questing wiznet
	{
		char qbuf[MSL];
		sprintf (qbuf, "`mQUEST> %s zecho (%s) %s`x", 
			ch->name, ch->in_room->area->name, argument);
		wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
	}


    for (d = connection_list; d; d = d->next)
    {
		if (d->connected_state == CON_PLAYING
		&&  d->character->in_room != NULL && ch->in_room != NULL
		&&  d->character->in_room->area == ch->in_room->area)
		{
			if (get_trust(d->character) >= get_trust(ch))
				d->character->print("zone> " );
			
			d->character->printlnf( "%s`x", argument );
		}
    }
}

void do_zoecho( char_data *ch, char *argument )
{
    connection_data *d;
    
    if ( argument[0] == '\0' )
    {
		ch->println( "Zone outdoors echo what?" );
		return;
    }

	// questing wiznet
	{
		char qbuf[MSL];
		sprintf (qbuf, "`mQUEST> %s zoecho '%s'`x", ch->name, argument);
		wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
	}
    
    for ( d = connection_list; d; d = d->next )
    {
		if ( d->connected_state == CON_PLAYING )
		{
			if ( IS_OUTSIDE(d->character)
			&&	 d->character->in_room->sector_type != SECT_CAVE
			&&   IS_AWAKE(d->character)
			&&	 d->character->in_room != NULL
			&&   ch->in_room != NULL
			&&   d->character->in_room->area == ch->in_room->area )
			{
				if (get_trust(d->character) >= get_trust(ch))
					d->character->print( "zoneoutdoors> " );

				d->character->printlnf( "%s`x", argument );
			}
		}
	}

    return;
}

/**************************************************************************/
void do_pecho( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
		ch->println( "Personal echo what?" ); 
		return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
		ch->println( "Target not found." );
		return;
    }

	// questing wiznet
	if (!IS_IMMORTAL(victim))
	{
		char qbuf[MSL];
		sprintf (qbuf, "`mQUEST> %s pecho (%s) %s`x", 
			ch->name, victim->name, argument);
		wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
	}

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        victim->print( "personal> " );

	victim->printlnf( "%s`x", argument );
	ch->printlnf( "personal - %s > %s`x", victim->name, argument );
}


/**************************************************************************/
ROOM_INDEX_DATA *find_location_player_prority( char_data *ch, char *arg )
{
    char_data *victim;
    OBJ_DATA *obj;

	// hack to convert 'room 1234 - jsdlkfksldf' into '1234', used by scalemxp
	if(!str_prefix("room ", arg)){
		arg+=5;
		if(is_digit(*arg)){
			char *t=strstr(arg, " -");
			if(t){
				*t='\0';
			}
		}
	}

    if ( is_number(trim_string(arg)) ){
		return get_room_index( atoi( arg ) );
	}

	victim=get_whovis_player_world(ch, arg);
	if(victim){
		return victim->in_room;
	}

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	{
		if ( !IS_NPC(victim))
		{
			if ( can_see_who(ch, victim))
			{
				return victim->in_room;
			}
		}
		else
		{
			return victim->in_room;
		}
	}

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}

/**************************************************************************/
ROOM_INDEX_DATA *find_location( char_data *ch, char *arg )
{
    char_data *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	{
		if ( !IS_NPC(victim))
		{
			if ( can_see_who(ch, victim))
			{
				return victim->in_room;
			}
		}
		else
		{
			return victim->in_room;
		}
	}

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



/**************************************************************************/
void do_transfer( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    ROOM_INDEX_DATA *location;
    connection_data *d;
    char_data *victim;
    long prev_room=0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( IS_NULLSTR(arg1) )
    {
        ch->println( "Transfer whom (and where)?" );
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        for ( d = connection_list; d != NULL; d = d->next )
        {
            if ( d->connected_state == CON_PLAYING
            &&   d->character != ch
            &&   d->character->in_room != NULL
            &&   can_see( ch, d->character ) )
            {
                char buf[MSL];
                sprintf( buf, "%s %s", d->character->name, arg2 );
                do_transfer( ch, buf );
            }
        }
        return;
    }

    if ( IS_NULLSTR(arg2) ){
        location = ch->in_room;
    }else{
        if ( ( location = find_location_player_prority( ch, arg2 ) ) == NULL ){
            ch->printlnf( "No such location '%s'.", arg2 );
            return;
        }

        if ( is_room_private_to_char( location, ch )){
            ch->println( "That room is private right now." );
            return;
        }
    }

	// search first in the world for players before searching for mobs
	victim=get_whovis_player_world( ch, arg1 );
	if ( !victim)
	{
	    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
		{
			ch->println( "They aren't here." );
			return;
		}
	}

    if ( victim->in_room == NULL )
    {
        ch->println( "They are in limbo." );
        return;
    }

	if (victim->in_room == location )
	{
        ch->printlnf("%s is already there.", PERS(victim,ch));
        return;
	}

    if ( victim->fighting != NULL )
        stop_fighting( victim, true );

    // remove from source room 
    if (ch==victim)
        act( "$n leaves the room.", victim, NULL, NULL, TO_ROOM );
    else
        act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );

    // if transing into ooc room from ic room, backup last_ic_room 
    if (IS_SET(location->room_flags, ROOM_OOC)
        && !IS_OOC(victim) && !IS_NPC(victim))
        victim->last_ic_room = victim->in_room;        

    if (ch->mounted_on)
       dismount(ch);

    prev_room = victim->in_room->vnum;
    char_from_room( victim );

    // put in target room 
    char_to_room( victim, location );
    if (ch==victim)
        act( "$n has arrived.", victim, NULL, NULL, TO_ROOM );
    else
        act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );


    if ( ch != victim )
    {
		ch->printlnf("You transfer %s to room %d (from room %ld).",
			PERS( victim, ch), victim->in_room->vnum, prev_room);

		if (can_see_who(victim, TRUE_CH(ch)))
			victim->printlnf("%s has transferred you.", TRUE_CH(ch)->name);
		else
		{
			if(!IS_SILENT(ch))
			{
				victim->println("a mystery imm has transferred you.");
			}
		}
	}
    
	do_look( victim, "auto" );
	ch->println( "Ok." );
}

/**************************************************************************/
void do_stransfer( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    ROOM_INDEX_DATA *location;
    char_data *victim;
    long prev_room=0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        ch->println( "Silent Transfer whom (and where)?" );
        ch->println( "(use self to get yourself or the mob you are controlling)" );
        return;
    }

    if ( IS_NULLSTR(arg2)){
		location = ch->in_room;
	}else{
		if ( ( location = find_location_player_prority( ch, arg2 ) ) == NULL )
		{
			ch->println( "No such location." );
			return;
		}

		if ( is_room_private_to_char( location, ch )){
			ch->println( "That room is private right now." );
			return;
		}
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	if ( victim->in_room == NULL )
	{
		ch->println( "They are in limbo." );
		return;
	}

	if ( victim->fighting != NULL )
		stop_fighting( victim, true );

	// if transing into ooc room from ic room, backup last_ic_room 
	if (IS_SET(location->room_flags, ROOM_OOC)
        && !IS_OOC(victim) && !IS_NPC(victim))
        victim->last_ic_room = victim->in_room;        

    if (ch->mounted_on)
       dismount(ch);

    // remove from source room 
    prev_room = victim->in_room->vnum;
    char_from_room( victim );

    // put in target room 
    char_to_room( victim, location );

    if ( ch != victim )
    {
        ch->printlnf("You transfer %s to room %d (from room %ld).",
            PERS( victim, ch), victim->in_room->vnum, prev_room);

		if (IS_IMMORTAL(victim)){
			victim->printlnf("%s has transferred you.", TRUE_CH(ch)->name);
		}
    }

    do_look( victim, "auto" );
    ch->println( "Ok." );
}

/**************************************************************************/
void do_at( char_data *ch, char *argument )
{
    char arg[MIL];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    char_data *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
		ch->println( "At where what?" );
		return;
    }

    if ( ( location = find_location_player_prority( ch, arg ) ) == NULL ){
		ch->println( "No such location." );
		return;
    }

    if (ch->in_room==location)
    {
		if (IS_IMMORTAL(ch)){
			ch->printlnf( "You are already in room %d... doing command without at.", location->vnum);
			interpret( ch, argument );
		}else{
			ch->println( "You are already in the same room!");
			interpret( ch, argument );
		}
		return; 
	}

    if (is_room_private_to_char( location, ch )){
		ch->println( "That room is private right now." );
		return;
    }

	SET_BIT(ch->dyn,DYN_USING_AT);
	if ( HAS_HOLYLIGHT(ch))
	{
		ch->printlnf("AT %d BEGINS", location->vnum );
	}

	original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    // See if 'ch' still exists before continuing!
    // Handles 'at XXXX quit' case.
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
		if ( wch == ch )
		{
			char_from_room( ch );
			char_to_room( ch, original );
			ch->on = on;
			break;
		}
    }
	REMOVE_BIT(ch->dyn,DYN_USING_AT);
	if ( HAS_HOLYLIGHT(ch))
	{
		ch->println("AT FINISHES");
	}
    return;
}


/**************************************************************************/
void do_atlevel( char_data *ch, char *argument )
{
    char arg[MIL];
	int max_level;

	if (IS_UNSWITCHED_MOB(ch))
		max_level=ch->level;
	else
		max_level=TRUE_CH(ch)->level;
	
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->println( "Atlevel level what?" );
        return;
    }

    if (is_number(arg))
    {
        int new_level, true_level, true_trust, true_xpen=0;

        new_level = atoi(arg);
        if ((new_level<=max_level) && (new_level>0))
        {
            true_trust= ch->trust;
            true_level= ch->level;
            ch->trust= 0;
            ch->level= new_level;
            if (!IS_NPC(ch))
            {
                true_xpen= ch->pcdata->xp_penalty;
                ch->pcdata->xp_penalty=10;
            }
            ch->printlnf("ATLEVEL %d BEGIN.", new_level);
			SET_BIT(ch->dyn,DYN_USING_ATLEVEL);
			forced_interpret( ch, argument );
			REMOVE_BIT(ch->dyn,DYN_USING_ATLEVEL);
            ch->trust= true_trust;
            ch->level= true_level;

            if (!IS_NPC(ch))
            {
                ch->pcdata->xp_penalty= true_xpen;
            }
            ch->printlnf("ATLEVEL %d FINISHED.", new_level);
            return;

        }
        else /* not high enough trust */
        {
            ch->println( "Level must be between 1 and your level." );
            return;
        }
    }

    ch->println( "Level must be a valid number." );
    return;
}


/**************************************************************************/
void do_goto( char_data *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    char_data *rch;
    int count = 0;

    if ( IS_SWITCHED(ch)){
        ch->println( "Goto is disabled while you are switched, If you really must move the" );
        ch->println( "mob you are controlling type `=Ctrans self <location>`x" );
        return;
    }

    if ( IS_NULLSTR(argument)){
        ch->println( "Goto where?" );
        return;
    }

    if ( ( location = find_location_player_prority( ch, argument ) ) == NULL )  {
        ch->println( "No such location." );
        return;
    }

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if (ch->in_room==location)
    {
        ch->println( "You are already in that room." );
        return;
    }

    if (is_room_private_to_char( location, ch ) && count>1 ){
		if ( !IS_IMMORTAL( ch ))
		{
			ch->println( "That room is private right now." );
			return;
		}
		else if ( IS_SET( location->room_flags, ROOM_OOC )
			|| INVIS_LEVEL(ch)< LEVEL_IMMORTAL )
		{
			ch->println( "That room is private right now." );
			return;
		}
    }

    if ( ch->fighting != NULL )
        stop_fighting( ch, true );

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= INVIS_LEVEL(ch))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
        {
            act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            if (IS_IMMORTAL(rch))
            {
                act("($n poofout)",ch,NULL,rch,TO_VICT);              
            }
        }
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    char_from_room( ch );
    char_to_room( ch, location );
    if (ch->mounted_on)
    {  
        char_from_room( ch->mounted_on );
        char_to_room( ch->mounted_on, location );
        do_look(ch->mounted_on, "auto");
    }
	else // automatically stand up those who are resting or sitting
	{
		if (ch->position==POS_SITTING || ch->position==POS_RESTING)
		{
			ch->position=POS_STANDING;
			ch->is_trying_sleep=false;
		}

	}
    
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= INVIS_LEVEL(ch))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
            {
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
                if(IS_IMMORTAL(rch)){
                    act("($n poofin)",ch,NULL,rch,TO_VICT);              
                }
            }
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_look( ch, "auto" );
    return;
}


/**************************************************************************/
void do_violate( char_data *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    char_data *rch;
 
    if ( argument[0] == '\0' )
    {
        ch->println( "Goto where?" );
        return;
    }
 
    if ( ( location = find_location_player_prority( ch, argument ) ) == NULL )
    {
        ch->println( "No such location." );
        return;
    }

    if(!is_room_private_to_char( location, ch )){
        ch->println( "That room isn't private, use goto." );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, true );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= INVIS_LEVEL(ch))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= INVIS_LEVEL(ch))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    do_look( ch, "auto" );
    return;
}


/**************************************************************************/
/* RT to replace the 3 stat commands */
void do_stat ( char_data *ch, char *argument )
{
    char arg[MIL];
    char *string;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    char_data *victim;

    string = one_argument(argument, arg);
    if ( arg[0] == '\0')
    {
        ch->println( "Syntax:" );
        ch->println( "  stat <name>" );
        ch->println( "  stat obj <name>" );
        ch->println( "  stat mob <name>" );
        ch->println( "  stat room <number>" );
        return;
    }

    if (!str_cmp(arg,"room"))
    {
        do_rstat(ch,string);
        return;
    }
  
    if (!str_cmp(arg,"obj"))
    {
        do_ostat(ch,string);
        return;
    }

    if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
    {
        do_mstat(ch,string);
        return;
    }
   
    // search for default matches
	victim=get_whovis_player_world( ch, argument );
    if (victim)
    {
        do_mstat(ch,argument);
        return;
    }

    obj = get_obj_world(ch,argument);
    if (obj)
    {
        do_ostat(ch,argument);
        return;
    }

    victim = get_char_world(ch,argument);
    if (victim)
    {
        do_mstat(ch,argument);
        return;
    }

    location = find_location(ch,argument);
    if (location)
    {
        do_rstat(ch,argument);
        return;
    }

    ch->println( "Nothing by that name found anywhere." );
}


/**************************************************************************/
void do_rstat( char_data *ch, char *argument )
{
    char			buf[MSL];
    char			arg[MIL];
    ROOM_INDEX_DATA *location;
    OBJ_DATA		*obj;
    char_data		*rch;
    int				door;
	AFFECT_DATA		*paf, *paf_last = NULL;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
		ch->println( "No such location." );
		return;
    }

    if (ch->in_room != location && is_room_private_to_char( location, ch ))
    {
		ch->println( "That room is private right now." );
		return;
    }

	ch->printlnf("Name: '%s'\nArea: '%s'  Filename '%s'",
				location->name,
				location->area->name,
				location->area->file_name );

	ch->printlnf("Vnum: %d  Sector: %s  Light: %d  Healing: %d  Mana: %d",
				location->vnum,
				flag_string( sector_types, location->sector_type ),
				location->light,
				location->heal_rate,
				location->mana_rate );

	ch->printlnf("Room flags: %s", room_flags_bit_name(location->room_flags));
	ch->printf("Description:\r\n%s`x", location->description );

    if ( location->extra_descr != NULL )
    {
		EXTRA_DESCR_DATA *ed;

		ch->print("Extra description keywords: '");
		for ( ed = location->extra_descr; ed; ed = ed->next )
		{
			ch->printf("%s", ed->keyword );
			if ( ed->next != NULL )
				ch->print(" ");
		}
		ch->println("'.`x");
	}

	ch->printf("Characters (%d):", location->number_in_room);
	for ( rch = location->people; rch; rch = rch->next_in_room )
	{
		if (can_see(ch,rch))
		{
			ch->print(" ");
			one_argument( rch->name, buf );
			ch->printf("%s", buf );
		}
	}

	ch->print(".\r\nObjects:   ");
	for ( obj = location->contents; obj; obj = obj->next_content )
	{
		ch->print(" ");
		one_argument( obj->name, buf );
		ch->printf("%s", buf );
	}
	ch->println(".`x");
	if(location->last_mined_in_room){
		ch->printf("Room last mined in at: %s", ctime(&location->last_mined_in_room));
	}

	for ( door = 0; door< MAX_DIR; door++ )
	{
		EXIT_DATA *pexit;

		if ( ( pexit = location->exit[door] ) != NULL )
		{
			ch->printf("Door: %d.  To: %d.  Key: %d.  Exit flags: %s.  Reset flags: %s.\r\nKeyword: '%s'.  Description: %s`x",
				door,
				(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
				pexit->key,
				flag_string( exit_flags, pexit->exit_info ),
				flag_string( exit_flags, pexit->rs_flags),
				pexit->keyword,
				pexit->description[0] != '\0'
				? pexit->description : "(none).\r\n" );

		}
	}

	if ( location->affected_by )
	{
		ch->println("`rThe room has the following affects on it.`x");
		for ( paf = location->affected; paf; paf = paf->next )
		{
			if (paf_last && paf->type == paf_last->type){
				continue;
			}else{
				ch->printf("Spell: %-15s", skill_table[paf->type].name );
//				ch->printf(": modifies %s by %d ",
//					full_affect_loc_name( paf), paf->modifier);
				if ( paf->duration == -1 )
					sprintf( buf, "until the world reforms" );
				else
					sprintf( buf, "for %d hours", paf->duration );
				ch->printf("%s", buf );
			}
			ch->println("");
			paf_last = paf;
		}
	}

	return;
}

/**************************************************************************/
void ostat_show_to_char( char_data *ch, OBJ_DATA *obj)
{
	AFFECT_DATA *paf=NULL;

	ch->printlnf( "`xName(s): %s",	obj->name );
	ch->printlnf( "Vnum: %d  uid:%-6d Type: %s  Resets: %d",
		obj->pIndexData->vnum, obj->uid, item_type_name(obj), obj->pIndexData->reset_num );
	
	if (obj->restrung){
		ch->println( "`R  RESTRUNG`x" );
	}
	ch->printlnf("Short description: %s`x", obj->short_descr);
	if (has_colour(obj->short_descr))
	{
		ch->print("B&W Shrt descript: ");
		ch->printlnbw(obj->short_descr);
	}
	ch->printlnf("Long description: %s`x", obj->description);
    if (has_colour(obj->description))
	{
		ch->print("B&W Long descript: ");
		ch->printlnbw(obj->description);
	}
    ch->printlnf("Wear bits: %s", wear_bit_name(obj->wear_flags));
	ch->printlnf("Extra bits: %s",extra_bit_name( obj->extra_flags));
	ch->printlnf("Extra2 bits: %s", extra2_bit_name( obj->extra2_flags));

    // print class restrictions
	if (obj->pIndexData->class_allowances)
	{
		// print class allowances
		ch->printlnf("Class Allowances: [%s]",
			flag_string( classnames_flags, obj->pIndexData->class_allowances) );
	}
	ch->printlnf("Number: %d/%d  Weight: %d/%d/%d (lbs)",
		1,	get_obj_number( obj ),
		obj->weight, get_obj_weight( obj ), get_true_weight(obj) );
	ch->printlnf("Level: %d  Cost: %d  Condition: %d  Timer: %d",
		obj->level, obj->cost, obj->condition, obj->timer );
	ch->printlnf("In room: %d  In object: %s  Carried by: %s  Wear_loc: %s",
		obj->in_room    == NULL ?        0 : obj->in_room->vnum,
		obj->in_obj     == NULL ? "(none)" : obj->in_obj->short_descr,
		obj->carried_by == NULL ? "(none)" : can_see(ch,obj->carried_by)
								? obj->carried_by->name		: "someone",
		flag_string(wear_location_types, obj->wear_loc) );

	if ( obj->attune_next > current_time ){
		ch->printlnf( "Next attune attempt: %s, which is %d hours from now.",
			ctime( (time_t*)&obj->attune_next ), (int)( obj->attune_next - current_time ) / 3600);
	}
	ch->printlnf("Values: %d %d %d %d %d",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3],	obj->value[4] );
    
    // now give out vital statistics as per identify 
    
    switch ( obj->item_type )
    {
	case ITEM_SCROLL: 
	case ITEM_POTION:
	case ITEM_PILL:
		ch->printf("Level %d spells of:", obj->value[0] );
		
		if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
		{
			ch->printf(" '%s'", skill_table[obj->value[1]].name );
		}
		if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
		{
			ch->printf(" '%s'", skill_table[obj->value[2]].name );
		}
		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			ch->printf(" '%s'", skill_table[obj->value[3]].name );
		}
		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
		{
			ch->printf(" '%s'", skill_table[obj->value[4]].name );
		}
		ch->println( "." );
		break;

	case ITEM_WAND: 
	case ITEM_STAFF: 
		ch->printf("Has %d(%d) charges of level %d",
			obj->value[1], obj->value[2], obj->value[0] );
		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			ch->printf(" '%s'", skill_table[obj->value[3]].name );
		}
		ch->println( "." );
		break;
		
	case ITEM_DRINK_CON:
		ch->printlnf("It holds %s-colored %s.",
			liq_table[obj->value[2]].liq_color,
			liq_table[obj->value[2]].liq_name);
		break;
		
	case ITEM_WEAPON:
        ch->printlnf("Weapon type is %s",get_weapontype(obj));
		
		ch->printlnf("Damage is %dd%d (average %d)",
		obj->value[1],
		obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	
		ch->printlnf("Damage noun is %s.", attack_table[obj->value[3]].noun);
		
		if (obj->value[4])  // weapon flags 
		{
			ch->printlnf("Weapons flags: %s", weapon_bit_name(obj->value[4]));
		}
		break;
		
	case ITEM_ARMOR:
		ch->printlnf("Armor class is %d pierce, %d bash, %d slash, and %d vs. magic",
			obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
		break;
		
	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
		ch->printlnf( "Maximum combined weight: %0.1f, Capacity for an individual item: %0.1f lbs.",
			((double)obj->value[0])/10,
			((double)obj->value[3])/10);
		ch->printlnf( "Flags: %s.", cont_bit_name(obj->value[1]));
		if (obj->value[4] != 100){
			ch->printf( "Weight multiplier: %d%%\r\n", obj->value[4]);
		}
		break;

	case ITEM_SHEATH:
		ch->printlnf( "It will hold %d weapons.",
			obj->value[0]);
		break;
	}
	
	if ( obj->extra_descr != NULL)
	{
		EXTRA_DESCR_DATA *ed;
		
        ch->print( "Custom extra description keywords: '" );
		
        for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            ch->printf( "%s", ed->keyword );
            if ( ed->next != NULL )
                ch->print( " / " );
        }
        ch->println( "'" );   
    }
	
    if (obj->pIndexData->extra_descr != NULL)
    {
        EXTRA_DESCR_DATA *ed;
		
        if (obj->extra_descr != NULL)
        {
            ch->print( "Ignored extra description keywords (vnum's): '" );
        }
        else
        {
            ch->print( "Original vnums extra description keywords: '" );
        }
		
        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            ch->printf( "%s", ed->keyword );
            if ( ed->next != NULL )
				ch->print( " / " );
        }   
        ch->println( "'" );
    }

	if ( obj->ospec_fun != 0 )
	{
		ch->printlnf("Object has special procedure `G%-20s`x                             |",
			ospec_name(obj->ospec_fun));
	}

	for (paf=OBJECT_AFFECTS(obj); paf; paf = paf->next )
	{
		if ( paf->where != WHERE_SKILLS && paf->where != WHERE_OBJECTSPELL)
		{
			ch->printlnf("Affects %s by %d, level %d.",
				affect_loc_name( paf->location ), paf->modifier,paf->level );
		}
		if (paf->bitvector || paf->where == WHERE_OBJECTSPELL)
		{
			ch->println(to_affect_string( paf, obj->level, true));
		}
	}
		// output the list of events on the entity
	obj->enumerate_events(ch);

}

/**************************************************************************/
void do_ostat( char_data *ch, char *argument )
{
	char arg[MIL];
	OBJ_DATA *obj;
	
	one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "Stat what?" );
		return;
	}
	
	if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
	{
		ch->println( "Nothing like that in hell, earth, or heaven." );
		return;
	}
	
	ostat_show_to_char(ch, obj);
	return;
}

/************************************************************************/
int get_sublevels_for_level(int level);
/**************************************************************************/
// Kal - June 01
void do_charinfo( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) ){
        ch->println( "Syntax: `=Ccharinfo <playername>`x" );
        return;
    }

    if (( victim = get_whovis_player_world(ch, argument ) ) == NULL ){
        ch->printlnf( "No character named '%s' was found in the game", argument );
        return;
    }

	// players only
	if(IS_NPC(victim)){ 
		ch->println("Players only");
		return;
	} 

	ch->titlebarf("CHARINFO: %s", uppercase(victim->name));
	
	ch->printf(  " `xName:`B %s`x   Created: %s", victim->name, ctime( (time_t *) & (victim->player_id)));
	ch->printlnf(" `xKnow Index:`B %d`x  Player ID: %d  uid:%d", 
		victim->know_index, (int)victim->player_id, victim->uid);
	if(!IS_NULLSTR(victim->pcdata->immtalk_name)){
		ch->printlnf(" `xImmtalk Name: %s`x", victim->pcdata->immtalk_name);
	}
	if(IS_TRUSTED(ch, ADMIN))
	{
		// for email banning verification
		bool print=false;
		if(!IS_NULLSTR(victim->pcdata->email)){
			ch->printlnf(" Email: `B%s`x", victim->pcdata->email);
			print=true;
		}
		if(!IS_NULLSTR(victim->pcdata->created_from)){
			ch->printf(" Created_from: `B%s`x ", victim->pcdata->created_from);
			print=true;
		}
		if(!IS_NULLSTR(victim->pcdata->unlock_id)){
			if(str_len(victim->pcdata->unlock_id)==6){
				ch->printlnf(" Unlock_id: `B%s`x", victim->pcdata->unlock_id);
				print=true;
			}
		}
		if(print){
			ch->println("");
		}

		if(victim->desc){
			ch->printlnf(" Host: `G%s`x",
				victim->desc->remote_hostname);

			ch->printlnf(" Remote Address: `W%s`x",
				victim->desc->remote_tcp_pair);

			ch->printlnf(" Connected via: `W%s`x",
				victim->desc->local_tcp_pair);
			if(!IS_NULLSTR(victim->desc->terminal_type)){
				ch->printlnf(" TerminalType: %s", victim->desc->terminal_type);
			}
		}

		ch->printlnf(" Remote_ip_copy: %s",victim->remote_ip_copy);
	}

	if(victim->desc){
		// Show the victims colour mode
		ch->printlnf(" Colour mode set to: %s", flag_string(colourmode_types, victim->desc->colour_mode));

		// MXP stuff
		if(!IS_NULLSTR(victim->desc->mxp_version)){
			ch->printlnf(" MXP ver text: %s`x",
				victim->desc->mxp_version);
		}
		if(victim->pcdata){
			ch->printlnf(" MXP preference is currently set to %s", 
				preference_word(victim->pcdata->preference_mxp));
			ch->printlnf(" MXP support in the client was %sdetected`x",
				(IS_SET(victim->desc->flags, CONNECTFLAG_MXP_DETECTED))?"":"not ");
		}


#ifdef MCCP_ENABLED
		if( victim->desc->out_compress){
			ch->printlnf(" Using MCCP version %d", victim->desc->mccp_version);
		}
#endif

		if(IS_TRUSTED(ch, ADMIN) && !IS_NULLSTR(victim->desc->ident_raw_result)){
			ch->printlnf(" Ident raw result: %s`x", victim->desc->ident_raw_result);
		}
	}

    ch->printlnf(" Room: `W%-5d`x  LastIC: `M%-5d`x  Set Recall: %d  Actual Recall: %d  Expires: `#`y%d`^",
        victim->in_room == NULL ? 0 : victim->in_room->vnum,
		victim->last_ic_room == NULL ? 0 : victim->last_ic_room->vnum,
		victim->recall_room, get_recallvnum(victim, NULL), victim->expire_recall_inn);

	ch->printlnf(" Thirst: `c%-2d  `xHunger: `c%-2d  `xFull: `c%-2d  "
		"`xDrunk: `c%-2d  `xTired: `c%-2d`x  `xAutoAfkAfter: `c%-2d`x `xQP: `c%-2d`x",
        victim->pcdata->condition[COND_THIRST],
        victim->pcdata->condition[COND_HUNGER],
        victim->pcdata->condition[COND_FULL],
        victim->pcdata->condition[COND_DRUNK],
        victim->pcdata->tired,
		victim->pcdata->autoafkafter,
		victim->pcdata->qpoints);
	ch->printlnf(" Bleeding: `c%-2d  `x", victim->bleeding);

	if(victim->colour_prefix!=COLOURCODE){
		ch->printlnfbw(" Colour code prefix: %c", victim->colour_prefix);
	}

    if (victim->clan)
	{
	    ch->printlnf(" `xClan: %-10s `xRank: %s%-15s (%d)`x                                  |",
		    victim->clan->cwho_name(),
			victim->clan->color_str(),
			victim->clan->clan_rank_title(victim->clanrank),
			victim->clanrank);              
	}

	ch->printf(" Age: %-3d  Played: %d(%0.03f%%)  LastLevel: %-4d ",
        GET_AGE(victim), 
        (int) (GET_SECONDS_PLAYED(victim)/ 3600), 
		GET_SECONDS_PLAYED(victim)* 100/ (double)(current_time-victim->player_id),
		victim->pcdata->last_level);

	ch->printlnf(" Timer: %-2d  Idle: %-2d  %12s", 
		victim->timer, victim->idle, " ");

    if (!IS_NULLSTR(victim->prompt)){
        ch->printlnbw(FORMATF(" Prompt: %s",victim->prompt));        
    }
    if(!IS_NULLSTR(victim->olcprompt)){        
        ch->printlnbw(FORMATF(" OLCPrompt: %s ",victim->olcprompt));
    }
  
	if(TRUE_CH_PCDATA(victim) && !IS_NULLSTR(TRUE_CH_PCDATA(victim)->battlelag)){
		ch->printlnbw(FORMATF(" BattleLag: %s ",TRUE_CH_PCDATA(victim)->battlelag));		
	}

    ch->printlnf(" Master: %-15s  Leader: %-15s  Pet: %-15s",
        victim->master      ? victim->master->name   : "(none)",
        victim->leader      ? victim->leader->name   : "(none)",
        victim->pet         ? victim->pet->name      : "(none)");
    
	// get their diplomacy info
	if(victim->pcdata->diplomacy && !IS_IMMORTAL(victim)){
		ch->printlnf(" Diplomacy:%d  Votes:%d",
			victim->pcdata->diplomacy,
			victim->pcdata->dip_points);
	}

	if(victim->pcdata->council){
		ch->printlnf(" Councils: %s", flag_string( council_flags, victim->pcdata->council ));
	}

	if (IS_ADMIN(ch)){
		if(victim->pcdata->xp_penalty>0){
			ch->printlnf(" `#`RXPEN: %-3d`&", victim->pcdata->xp_penalty);
		}
		
		ch->printlnf(" Level: %-3d  Trust: %-3d  Security: %-3d %-6s",
			victim->level, 
			(IS_TRUSTED(victim,get_trust(ch))? get_trust(ch):get_trust(victim)),
			victim->pcdata->security,
			IS_SET(victim->act, PLR_LOG)?"`#`RLOGGED`&":"");
	}
	
	if(IS_IMMORTAL(victim) && victim->pcdata){
		if(!IS_NULLSTR(victim->pcdata->bamfin)){
			ch->printlnf(" PoofIn: %s", victim->pcdata->bamfin);
		}
		if(!IS_NULLSTR(victim->pcdata->bamfout)){
			ch->printlnf(" PoofOut: %s", victim->pcdata->bamfout);
		}
		if(!IS_NULLSTR(victim->pcdata->fadein)){
			ch->printlnf(" FadeIn: %s", victim->pcdata->fadein);
		}
		if(!IS_NULLSTR(victim->pcdata->fadeout)){
			ch->printlnf(" FadeOut: %s", victim->pcdata->fadeout);
		}
	}


	ch->printlnf(" Creation points: %d  XPPerLvl: `m%d`x  XPTillNextlvl: `m%d`x  RPS: `m%ld`x",
        victim->pcdata->points,
        exp_per_level(victim, victim->pcdata->points),
        (victim->level+1)*exp_per_level(victim, victim->pcdata->points)- victim->exp,
        victim->pcdata->rp_points );

	// PK RELATED STATS
	ch->titlebar("PK RELATED STATS");
    ch->printf(  " Pkills: `R%-2d`x  PkDefeats: `r%-2d`x", 
		victim->pkkills, victim->pkdefeats);
    ch->printlnf(" MKills: `B%-2d`x  MDefeats: `B%-3d`x %s",
		victim->pcdata->mkills, victim->pcdata->mdefeats, " ");

	ch->printlnf(" Pksafe: `C%-2d`x  PkOOL: `C%-4d`x  PkNorecall: `C%-2d`x  Pknoquit: `C%-2d`x",
			 victim->pksafe, victim->pkool, victim->pknorecall, victim->pknoquit);


    ch->printlnf(" Karns: `Y%d`x  NextKarn: `y%-5d`x Sec: `c%d`x  Subdued: `c%-2d",
		victim->pcdata->karns,
		victim->pcdata->next_karn_countdown,
		victim->pcdata->security,
		victim->subdued_timer);

	if(!GAMESETTING2(GAMESET2_NO_DUEL_REQUIRED)){
		ch->titlebar("DUEL STATS");
		ch->printlnf(" Challenged: %d  Declined: %d  Accepted: %d  Ignored: %d\r\n"
			" Bypassed: %d  Subdues B4 Karn Loss: %d",
			victim->duel_challenged,
			victim->duel_decline,
			victim->duel_accept,
			victim->duel_ignore,
			victim->duel_bypass,
			victim->duel_subdues_before_karn_loss);
	}

	// ADMIN CAN SEE NOTES BEING WRITTEN - needed for hotrebooting
	if(victim->pnote && IS_ADMIN(ch) && ch->level > victim->level)
	{	
		ch->titlebar("NOTE DETAILS");
		ch->printlnf(" `#`?%s is writing the following %s:`&", 
			victim->name, get_notetype(victim->pnote->type));
        ch->printlnf(" %s: %s",
			victim->pnote->sender,
			victim->pnote->subject);
		ch->printlnf(" To: %s",
			victim->pnote->to_list);
		if (IS_SET(victim->act,PLR_AUTOREFORMAT)){
			char *tempdup= note_format_string(str_dup(victim->pnote->text));
			ch->printf( "%s", tempdup );
			free_string(tempdup);
		}else{
			ch->printf( "%s", victim->pnote->text );
		}
		ch->println( "`x" );
	}

	ch->titlebar("");

    return;
}

/**************************************************************************/
// Kal - September 01
void do_mxpinfo( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) ){
        ch->println( "Syntax: `=Cmxpinfo <playername>`x" );
        return;
    }

    if (( victim = get_whovis_player_world(ch, argument ) ) == NULL ){
        ch->printlnf( "No character named '%s' was found in the game", argument );
        return;
    }

	ch->titlebarf("MXPINFO: %s", uppercase(victim->name));

	if(victim->desc && victim->pcdata){
		bool newline=false;
		if(victim->pcdata->preference_mxp==PREF_AUTOSENSE){
			ch->printf(" MXP support has %sbeen detected.",
				(IS_SET(victim->desc->flags, CONNECTFLAG_MXP_DETECTED))?"":"not ");
			newline=true;
		}
		if(victim->desc && !IS_NULLSTR(victim->desc->terminal_type)){
			ch->printf("     TerminalType: %s", victim->desc->terminal_type);
			newline=true;
		}else{
			ch->print("     TerminalType Undetected");
		}
		ch->print_blank_lines(1);

		ch->printlnf(" MXP preference is set to %s", preference_word(victim->pcdata->preference_mxp));


		ch->printlnf(" MXP version response: %s`x", !IS_NULLSTR(victim->desc->mxp_version)?victim->desc->mxp_version: "unknown");
		ch->wraplnf(" MXP support response: %s`x", !IS_NULLSTR(victim->desc->mxp_supports)?victim->desc->mxp_supports: "unknown");
		ch->wraplnf(" MXP options response: %s`x",!IS_NULLSTR(victim->desc->mxp_options)?victim->desc->mxp_options:"unknown");

#ifdef MCCP_ENABLED
		if( victim->desc->out_compress){
			ch->printlnf(" Using MCCP version %d", victim->desc->mccp_version);
		}
#endif
	}

	ch->titlebar("");

    return;
}

/**************************************************************************/
void do_mstat( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
    char buf[MSL];
    AFFECT_DATA *paf;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->println( "Stat whom?" );
		ch->println( "hint: use `=Cinroom stat mob <mobname>`x to always get a mob within the current room");
        return;
    }

    if (( victim = get_char_world( ch, argument ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

	ch->println( "_______________________________________________________________________________");
		
	if(IS_NPC(victim)){
		ch->printlnf("| `xName:`B %-70s`x|", victim->name);
	}else{
		sprintf(buf, "`xName:`B %s`x   Created: %s",
			victim->name, ctime( (time_t *) & (victim->player_id)));
		buf[str_len(buf)-1]='\0';
		ch->printlnf("| %-82s|", buf);
	}

    ch->printlnf("| Vnum: %-5d  uid:%-6d Format: %-3s  Room: `m%-5d`x  LastIC: `M%-5d`x             |",
        IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		victim->uid,
        IS_NPC(victim) ? "npc" : "pc",
        victim->in_room == NULL ? 0 : victim->in_room->vnum,
		victim->last_ic_room == NULL ? 0 : victim->last_ic_room->vnum);

	if (IS_NPC(victim))
    {
		ch->printlnf("| Count: %-3d   Killed: %-3d   XP Mod: %-3d                                      |",
        victim->pIndexData->count,victim->pIndexData->killed, victim->pIndexData->xp_mod);
    }

	ch->printlnf("| Race: `B%-10s  `xRecall: `G%5d `xGroup: `B%-2d  `xSex: `B%-7s`x %15s     |",
		race_table[victim->race]->name,
		get_recallvnum(victim, NULL),
		IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name, " " );
         
	ch->printf("| Lv: `G%-3d  `xClass: `G%-12s  `xTendency: `g%+d  `xAlliance: `g%+d`x"
		"                    |\r\n"
		"| `Y-$$$ `xBank: `y%-6ld  `xGold: `y%-5ld  `xSilver: `s%-5ld`x  Exp: `M%-6d`x",
		victim->level,       
        IS_NPC(victim) ? "mobile" : class_table[victim->clss].name,
		victim->tendency, victim->alliance, victim->bank,
		victim->gold, victim->silver, victim->exp );

	if ( !IS_NPC( victim )){
		ch->printlnf("  HeroXP: %-5d   |", victim->pcdata->heroxp );
		if(victim->pcdata->hero_level_count){
			ch->printlnf("    Hero Level Count: %d", victim->pcdata->hero_level_count);			
		}
	}else{
		ch->println( "                  |" );
	}

	if(!IS_NPC(victim)){
		if(GAMESETTING3(GAMESET3_KILLER_SYSTEM_ENABLED) ){
			if(IS_KILLER(victim)){
				ch->printf("  Killer until: %s", 
					ctime(&victim->pcdata->killer_until));

			}
		}
		if(GAMESETTING3(GAMESET3_THIEF_SYSTEM_ENABLED)){
			if(IS_THIEF(victim)){
				ch->printf("  Thief until: %s", 
					ctime(&victim->pcdata->thief_until));

			}
		}	
	}

	// remort status showing in stat
	if(GAMESETTING(GAMESET_REMORT_SUPPORTED) && !IS_NPC(victim)){
		ch->printlnf("| Remort: %d", victim->remort);
	}

	if(!IS_NPC(victim) && victim->level>=50){
		sprintf(buf,"Sublevel:  %2d/%2d  subprac %d  subtrain %d", 
			victim->pcdata->sublevel, get_sublevels_for_level(victim->level),
			victim->pcdata->sublevel_pracs, victim->pcdata->sublevel_trains);
		ch->printlnf("| %-76s|", buf);
	}


	ch->printlnf("| Armor: pierce: `b%-6d  `xbash: `b%-6d  `xslash: `b%-6d  `xmagic: `b%-6d`x           |",
		GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
		GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));

    ch->printlnf("| Hit: `c%-3d`x  DamMod: `c%-3d`x  Saves: `c%-3d`x  Size: %-7s  Pos: %-8s  Wimpy: %-3d |",
        GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
        size_table[victim->size].name, position_table[victim->position].name,
        victim->wimpy );

    if (IS_NPC(victim))
    {
		ch->printlnf("| Damage: `C%2dd%-2d  `xMessage: `c%-15s`x                                     |",
			victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
			attack_table[victim->dam_type].noun);
    }else{
		ch->printlnf("| victim->pcdata->objrestrict = %d |",	victim->pcdata->objrestrict);
	}

	ch->println( "|_____________________________________________________________________________|");
		

	ch->printlnf("| Fighting: `C%s  `c%s`x   Timer: %d",
		victim->fighting ? victim->fighting->name : "(none)" ,
		victim->no_xp ? "NO_XP_SET":"", victim->timer);

//	ch->printlnf("| Memory: {r%-67s{x |",
//		victim->mobmemory ? victim->mobmemory->name : "(none)" );

    if (IS_NPC(victim) && victim->off_flags)
        ch->printlnf("| Offense: %-66s |",off_bit_name(victim->off_flags));

    if (victim->imm_flags)
		ch->printlnf("| Immune: %-67s |",imm_bit_name(victim->imm_flags));
 
    if (victim->res_flags)
		ch->printlnf("| Resist: %-67s |", imm_bit_name(victim->res_flags));

    if (victim->vuln_flags)
		ch->printlnf("| Vulnerable: %-63s |", imm_bit_name(victim->vuln_flags));

	if (victim->comm)
		ch->printlnf("| Comm: %-69s ",comm_bit_name(victim->comm));

	if (IS_SET(victim->dyn,DYN_NONMAGICAL_FLYING))
		ch->println("| Dyn: DYN_NONMAGICAL_FLYING");

	if (victim->config){
		ch->printlnf("| Config: %-67s ", 
			flag_string( config_flags, victim->config));
	}

	if (victim->config2){
		ch->printlnf("| Config2:%-67s ", 
			flag_string( config2_flags, victim->config2));
	}
    
	ch->printlnf("| Act: %-70s ", act_bit_name(victim->act));
	ch->printlnf("| Act2:%-70s ", act2_bit_name(victim->act2));
    
    if (victim->affected_by)
    {
        ch->printlnf("| Affected by: `c%-63s`x ", 
            affect_bit_name(victim->affected_by));
    }
    if (victim->affected_by2)
    {
        ch->printlnf("| Affected by2: `c%-63s`x ", 
            affect2_bit_name(victim->affected_by2));
    }
    if (victim->affected_by3)
    {
        ch->printlnf("| Affected by3: `c%-63s`x ", 
            affect3_bit_name(victim->affected_by3));
    }

	ch->printlnf("| Parts: %-68s \r\n| Form: %-69s |",
        part_bit_name(victim->parts), form_bit_name(victim->form));

    ch->printlnf("| Master: %-15s  Leader: %-15s  Pet: %-15s      |",
        victim->master      ? victim->master->name   : "(none)",
        victim->leader      ? victim->leader->name   : "(none)",
        victim->pet         ? victim->pet->name      : "(none)");
    
	ch->printlnf("| Mounted on: %-15s  Ridden by: %-15s                     |",
        victim->mounted_on      ? victim->mounted_on->name   : "(none)",
        victim->ridden_by       ? victim->ridden_by->name    : "(none)");
    
    if (victim->last_force > -20)
	{
		    ch->printlnf("| Wildness: %-3d  Will: %-3d  Last_force: %-10ld (%-4ld tick%s ago)           |",
				victim->wildness, victim->will,
				victim->last_force, tick_counter -victim->last_force,
				(tick_counter -victim->last_force<=1)?" ":"s");
    }
    else
    {
			ch->printlnf("| Wildness: %-3d  Will: %-3d  Last_force: never                                 |",
				victim->wildness, victim->will);
    }

	ch->println( "|_____________________________________________________________________________|\r\n|                                                                             |" );

	// get their diplomacy info
	if(!IS_NPC(victim) && victim->pcdata->diplomacy && !IS_IMMORTAL(victim)){
		sprintf(buf,"Diplomacy:%d  Votes:%d",
			victim->pcdata->diplomacy,
			victim->pcdata->dip_points);
	}else{
		buf[0]='\0';
	}


    if(!IS_NPC(victim))         
    {
		if(victim->pcdata->council){
			ch->printlnf("| Councils: %-64s|", flag_string( council_flags, victim->pcdata->council ));
		}

		if(victim->pcdata->realms){
			ch->printlnf("| Realm Bits: %-64s|", flag_string( realm_flags, victim->pcdata->realms));
		}
		if(victim->pcdata->spheres){
	        ch->printlnf("| Sphere Bits: %-64s|", flag_string( sphere_flags, victim->pcdata->spheres));
		}
		if(victim->pcdata->elements){
		    ch->printlnf("| Element&Season Bits: %-64s|", flag_string( element_flags, victim->pcdata->elements));			
		}
		if(victim->pcdata->compositions){
		    ch->printlnf("| Composition Bits: %-64s|", flag_string( composition_flags, victim->pcdata->compositions));			
		}
    }           

	ch->println( "|_____________________________________________________________________________|\r\n|                                                                             |" );

    if IS_NPC(victim)
	{
        ch->printlnf("| Short description: %-56s |\r\n| Long  description: %s",
        victim->short_descr,
		IS_NULLSTR(victim->long_descr)? "(none)" : victim->long_descr );
	}else{
        ch->printlnf("| Short description: %-56s |",
        victim->short_descr);
		ch->printlnf(  "| hint: use '`=Ccharinfo %s`x' for more player related details", victim->name);
	}

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
        ch->printlnf("Mobile has special procedure `G%-20s`x                             |",
                spec_name(victim->spec_fun));
    }

	// where definitions
    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
        switch (paf->where)
        {
        case WHERE_WEAPON:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (weaponbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                weapon_bit_name( paf->bitvector ),
                paf->level);
            break;

        case WHERE_VULN:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (vulnbits %s), lvl %d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                imm_bit_name( paf->bitvector ),
                paf->level);
            break;

        case WHERE_RESIST:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (resistbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                imm_bit_name( paf->bitvector ),
                paf->level);
            break;

        case WHERE_IMMUNE:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (immunebits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                imm_bit_name( paf->bitvector ),
                paf->level);
            break;

        case WHERE_OBJEXTRA:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (objbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                extra_bit_name(paf->bitvector),
                paf->level);
            break;
        case WHERE_OBJEXTRA2:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (objbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                extra2_bit_name(paf->bitvector),
                paf->level);
            break;
		case WHERE_OBJECTSPELL:
			{
				char buf2[50];
				sprintf(buf, "`WObjectspell:`x '%s', level %3d, duration %3d, flags %s",
					skill_table[paf->type].name,
					paf->level,
					paf->duration,
					fwrite_flag(paf->bitvector,buf2));
			}
			break;
        case WHERE_AFFECTS:
        default:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (affbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                affect_bit_name( paf->bitvector ),
                paf->level);
            break;
        case WHERE_AFFECTS2:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (affbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                affect2_bit_name( paf->bitvector ),
                paf->level);
            break;
		case WHERE_AFFECTS3:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d tick%s (affbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                affect3_bit_name( paf->bitvector ),
                paf->level);
            break; 
        }
		{
			char format_buf[MIL];

			// find the length of the colour codes
			int collen=c_str_len(buf);
			int slen=str_len(buf);
			if(collen>=0){ 
				collen=slen-collen;
			}else{
				collen=0;
			}

			sprintf(format_buf,"| %%-%ds|", 76+collen);
			ch->printlnf(format_buf, buf);
		}
    }


	//dawn2?
	// show last time skills were used
	{
		int sn;
		if (!IS_NPC(victim))
		{
			for ( sn = 0; sn < MAX_SKILL; sn++ )
			{
				if ( skill_table[sn].name != NULL && victim->pcdata->last_used[sn] > 0 )
				{
					ch->println( "|_____________________________________________________________________________|\r\n|                                                                             |" );

					ch->printlnf("| `YSpell LastUsed:`x '%-25s' at %-15s                    |",
						skill_table[sn].name, 
						(char *) ctime( &victim->pcdata->last_used[sn]) );
				}
			}
		}
	}
	ch->println( "|_____________________________________________________________________________|" );


	// output the list of events on the entity
	victim->enumerate_events(ch);
    
}

/**************************************************************************/
/* ofind and mfind replaced with vnum, vnum skill also added */
void do_vnum(char_data *ch, char *argument)
{
    char arg[MIL];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
		ch->println( "Syntax:" );
		ch->println( "  vnum obj <name>" );
		ch->println( "  vnum obj <level>" );
		ch->println( "  vnum mob <name>" );
		ch->println( "  vnum mob <level>" );
		ch->println( "  vnum skill <skill or spell>" );
		return;
    }

    if (!str_cmp(arg,"obj"))
    {
		do_ofind(ch,string);
 		return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
		do_mfind(ch,string);
		return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
		do_slookup(ch,string);
		return;
    }

    // do both
	ch->println( "-=== MOBILES ===-" );
    do_mfind(ch,argument);
	ch->println( "-=== OBJECTS ===-" );
    do_ofind(ch,argument);
}


/**************************************************************************/
void do_mfind( char_data *ch, char *argument )
{
	char arg[MIL];
	MOB_INDEX_DATA *pMobIndex;
	int vnum;
	bool found;
	int level=0;
	bool level_search=false;
	name_linkedlist_type *list, *plist, *plist_next;
	list=NULL;

	one_argument( argument, arg );
	if ( IS_NULLSTR(arg) )
	{
		ch->println( "Find whom?" );
		return;
	}
	
	if(is_number(arg)){
		level_search=true;
		level=atoi(arg);
	}
	found	= false;
	for ( vnum = 0; vnum< MAX_KEY_HASH; vnum++ ){
		for ( pMobIndex  = mob_index_hash[vnum];	
			pMobIndex;
			pMobIndex=pMobIndex->next )
		{
			if (    (level_search && pMobIndex->level==level)
				 || (!level_search 
					&& (is_name( argument, pMobIndex->player_name )
						|| is_name( argument, pMobIndex->short_descr)
						)
					) 
					
				)
			{
				char area_name[MSL];
				area_name[0]='\0';

				if(pMobIndex->area){
					if(IS_NULLSTR(pMobIndex->area->short_name)){
						sprintf(area_name, "(%s%s`x)", 
							colour_table[(pMobIndex->area->vnum%14)+1].code,
							pMobIndex->area->name);
					}else{
						sprintf(area_name, "(%s%s`x)", 
							colour_table[(pMobIndex->area->vnum%14)+1].code,
							pMobIndex->area->short_name);
					}
				
				}

				found = true;
				
				// add it to the linked list which is sorted alphabetically
				addlist(&list, 
					FORMATF("<%s>[%3d] %s %s", 
						mxp_create_send(ch, FORMATF("mwhere %d", pMobIndex->vnum), FORMATF("%6d", pMobIndex->vnum)),
						pMobIndex->level, str_width(pMobIndex->short_descr,48,false), 
						area_name),
					0, true, false);
			}
		}
	}

	if(found){
		// output the contents of the linked list
		for(plist=list; plist; plist=plist_next){
			plist_next=plist->next;
			ch->println(plist->name);
			free_string(plist->name);
			delete plist;
		}
		list=NULL;
	}else{
		ch->println( "No mobiles found when searching for " );
		if(level_search){
			ch->printlnf("level %d mobiles.", level);
		}else{
			ch->printlnf("'%s'.", arg);
		}
	}

    return;
}


/**************************************************************************/
void do_ofind( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    bool found;
	int level=0;
	bool level_search=false;
	name_linkedlist_type *list, *plist, *plist_next;
	list=NULL;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
		ch->println( "Find what?" );
		return;
	}

	if(is_number(arg)){
		level_search=true;
		level=atoi(arg);
	}
	found	= false;
	for ( vnum = 0; vnum< MAX_KEY_HASH; vnum++ ){
		for ( pObjIndex= obj_index_hash[vnum];	
			pObjIndex;
			pObjIndex=pObjIndex->next )
		{
			if (    (level_search && pObjIndex->level==level)
				 || (!level_search && (is_name( argument, pObjIndex->name )
										|| is_name( argument, pObjIndex->short_descr)
										)

					) 
				 
				 )
			{
				char area_name[MSL];
				area_name[0]='\0';

				if(pObjIndex->area){
					if(IS_NULLSTR(pObjIndex->area->short_name)){
						sprintf(area_name, "(%s%s`x)", 
							colour_table[(pObjIndex->area->vnum%14)+1].code,
							pObjIndex->area->name);
					}else{
						sprintf(area_name, "(%s%s`x)", 
							colour_table[(pObjIndex->area->vnum%14)+1].code,
							pObjIndex->area->short_name);
					}
				
				}

				found = true;
				// add it to the linked list which is sorted alphabetically
				addlist(&list, 
					FORMATF("<%s>[%3d] %s %s", 
						mxp_create_send(ch, FORMATF("owhere %d", pObjIndex->vnum), FORMATF("%6d", pObjIndex->vnum)),
						pObjIndex->level, str_width(pObjIndex->short_descr,48,false),
						area_name), 
					0, true, false);
			}
		}
	}

	if(found){
		// output the contents of the linked list
		for(plist=list; plist; plist=plist_next){
			plist_next=plist->next;
			ch->println(plist->name);
			free_string(plist->name);
			delete plist;
		}
		list=NULL;
	}else{
		ch->println( "No objects found when searching for " );
		if(level_search){
			ch->printlnf("level %d items.", level);
		}else{
			ch->printlnf("'%s'.", arg);
		}
	}
	return;
}


/**************************************************************************/
void do_objdesc( char_data *ch, char *argument )
{
	char arg[MIL];
    OBJ_DATA *obj;

	argument = one_argument( argument, arg );

	if(arg[0] == '\0' )
	{
        ch->println( "Syntax: objdesc <object>" );
        return;
	}

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
        ch->println( "You do not see that object here." );
	    return;
	}

    string_append(ch, &obj->description);
    return; 
}

/**************************************************************************/
void do_cwhere(char_data *ch, char * )
{
    char buf[MIL];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = false;
    number = 0;
    max_found = 200;

	ch->println( "Displaying player corpses." );
	ch->println( "(red has objects still inside)" );
    buffer = new_buf();

    for ( obj = object_list; obj; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || obj->item_type!=ITEM_CORPSE_PC)
            continue;
 
        found = true;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d] (%d)`x\r\n",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum, obj->timer );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d] (%d)`x\r\n",
					 number, obj->short_descr,in_obj->in_room->name,
	   	in_obj->in_room->vnum, obj->timer);
	else
            sprintf( buf, "%3d) %s is somewhere (%d)`x\r\n",number, 
				obj->short_descr, obj->timer);
 
        buf[0] = UPPER(buf[0]);
        if (obj->item_type==ITEM_CORPSE_PC)
        {
			if (obj->contains)
			{
				if (obj->carried_by){
					add_buf(buffer,"`B");
				}else{
					add_buf(buffer,"`R");
				}
			}else{
				add_buf(buffer,"`g");
			}
        }
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        ch->println( "There are no player corpses in the game." );
    else
        ch->sendpage(buf_string(buffer));

    free_buf(buffer);
}


/**************************************************************************/
void do_owhere(char_data *ch, char *argument )
{
    char buf[MIL];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;
	char ovnum[MIL];

	bool VnumSearch=false;
	int VnumValue=0;
	bool debugging_object=false;

    found = false;
    number = 0;
    max_found = 200;

	if (IS_NULLSTR(argument))
	{
		ch->println( "Find what?" );
		return;
    }

	if (is_number ( argument )){
		VnumSearch=true;
		VnumValue= atoi(argument);
	}

    buffer = new_buf();
 
    for ( obj = object_list; obj; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) 
			||   ch->level < obj->level)
            continue;

		if (VnumSearch){
			if (obj->pIndexData && 
				obj->pIndexData->vnum== VnumValue){
				// do nothing, matching vnum found
			}else{
				continue;
			}
		}else{
			if(!is_name( argument, obj->name ))
				continue;
		}
 
        found = true;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj ){
			// loop thru finding which obj, an object is in
		};

		if(obj->pIndexData){
			sprintf(ovnum, "[%d]",obj->pIndexData->vnum);
		}else{
			ovnum[0]='\0';
		}
 
		if(IS_TRUSTED(ch, ADMIN)){
			if( in_obj->carried_by && in_obj->carried_by->in_room != NULL)
			{
				sprintf( buf, "%3d) %s%s is carried by %s [Room %d]`x\r\n",
					number, obj->short_descr, ovnum, 
					PERS(in_obj->carried_by, ch),
					in_obj->carried_by->in_room->vnum );
			}else if(in_obj->in_room){
				sprintf( buf, "%3d) %s%s is in %s [Room %d]`x\r\n",
					number, obj->short_descr, ovnum, in_obj->in_room->name,
	   				in_obj->in_room->vnum);
			}else{
				sprintf( buf, "%3d) %s%s is somewhere (unknown location)`x\r\n",
					number, obj->short_descr, ovnum);
				DEBUG_OBJECT=obj;
				debugging_object=true;
			}
		}else{
			if( in_obj->carried_by && can_see(ch,in_obj->carried_by)
				 && in_obj->carried_by->in_room != NULL)
			{
				sprintf( buf, "%3d) %s%s is carried by %s [Room %d]`x\r\n",
					number, obj->short_descr, ovnum, 
					PERS(in_obj->carried_by, ch),
					in_obj->carried_by->in_room->vnum );
			}else if(in_obj->in_room && can_see_room(ch,in_obj->in_room)){
				sprintf( buf, "%3d) %s%s is in %s [Room %d]`x\r\n",
					number, obj->short_descr, ovnum, in_obj->in_room->name,
	   				in_obj->in_room->vnum);
			}else{
				sprintf( buf, "%3d) %s%s is somewhere`x\r\n",
					number, obj->short_descr, ovnum);
			}
		}
 
        buf[0] = UPPER(buf[0]);
        if (obj->item_type==ITEM_CORPSE_PC)
        {
			if (obj->contains){			
				add_buf(buffer,"`R");
			}else{
				add_buf(buffer,"`g");
			}
        }

		if (!add_buf(buffer,buf))
		{
			ch->println( "Too many objects... buffer overflow." );
            break;
		} 
 
        if (number >= max_found)
            break;
    }
 
    if ( !found ){
        ch->println( "No object like that in heaven or earth." );
    }else{
        ch->sendpage(buf_string(buffer));
	}

    free_buf(buffer);

	if(debugging_object){
		ch->println("`RDEBUG_OBJECT SET TO THE FOLLOWING OBJECT (because location unknown):`x");
		ostat_show_to_char(ch, DEBUG_OBJECT);
	}
}


/**************************************************************************/
void do_mwhere( char_data *ch, char *argument )
{
    char buf[MSL];
	BUFFER *buffer;
    bool found;
    int count = 0;
	char_data *victim;

	bool VnumSearch=false;
	int VnumValue=0;

    if ( IS_NULLSTR(argument))
    {
		// show characters logged
		buffer = new_buf();
		char_data *pch;
		for(pch=player_list; pch; pch=pch->next_player)
		{
			// if we can't see them, they don't exist :)
			if(!can_see(ch,pch)){
				continue;
			}

			// character body location
			{
				sprintf(buf,"%3d) [%s%s`x] %-12s is in %s (%s%s`x)\r\n",
					count, 
					colour_table[(pch->in_room->vnum%14)+1].code,
					mxp_create_tagf(ch, "rmvnum", "%5d", pch->in_room->vnum),
					pch->name,
					pch->in_room->name, 
					colour_table[(pch->in_room->area->vnum%14)+1].code,
					pch->in_room->area->name);
				add_buf(buffer,buf);
			}

			// if they switched into something - show that also
			if(pch->controlling && pch->controlling->in_room){
				victim=pch->controlling;
				sprintf(buf,"%3d) [%s%s`x] %-12s `Y(in the body of %s)`x is in %s (%s%s`x)\r\n",
					count, 
					colour_table[(victim->in_room->vnum%14)+1].code,
					mxp_create_tagf(ch, "rmvnum", "%5d", victim->in_room->vnum),
					pch->name,
					victim->short_descr,
					victim->in_room->name, 
					colour_table[(victim->in_room->area->vnum%14)+1].code,
					victim->in_room->area->name);
				add_buf(buffer,buf);				
			}
		}
		ch->sendpage(buf_string(buffer));
		free_buf(buffer);
		return;
    }

    found = false;
    buffer = new_buf();
    for ( victim = char_list; victim; victim = victim->next )
    {	
		if (is_number ( argument )){
			VnumSearch=true;
			VnumValue= atoi(argument);
		}

		if ( victim->in_room != NULL
			&& (VnumSearch?
					(victim->pIndexData && victim->pIndexData->vnum==VnumValue):
					is_name( argument, victim->name )
				)
			 && can_see(ch,victim) )
		{
			found = true;
			count++;
			sprintf( buf, "%3d) [%s] %s `x[%s%s`x] %s\r\n",
				count,
				mxp_create_tagf(ch, FORMATF("mbvnum %d", victim->uid), 
					"%5d", IS_NPC(victim) ? victim->pIndexData->vnum : 0),
				str_width(IS_NPC(victim) ? victim->short_descr : victim->name, 28, true),
				colour_table[(victim->in_room->vnum%14)+1].code,
				mxp_create_tagf(ch, "rmvnum", "%5d", victim->in_room->vnum),
				victim->in_room->name );
	
			add_buf(buffer,buf);
		}
    }

    if ( !found ){
		act( "You didn't find any mobs '$T'.", ch, NULL, argument, TO_CHAR );
    }else{
    	ch->sendpage(buf_string(buffer));
	}
    free_buf(buffer);
    return;
}


/**************************************************************************/
void do_reboo( char_data *ch, char * )
{
    ch->println( "If you want to REBOOT, spell it out." );
    return;
}

/**************************************************************************/
void do_reboot( char_data *ch, char *argument)
{
    char buf[MSL];
    connection_data *d,*c_next;
    extern char shutdown_filename[MSL];
    char_data *vch, *vch_next;

	if(IS_NULLSTR(argument) || str_cmp("confirm", argument)){
        ch->println("`xType `=Creboot confirm`x to reboot the mud down.");
		ch->println("note: the mud will only automatically start back up if "
			"there is an auto startup script configured.");
		return;
	}

    // record reboot details in shutdown log file
    sprintf( buf, "at %s %s was rebooted by %s.\n===========================",
         (char *) ctime( &current_time ),
         shutdown_filename,
         ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );

    // display reboot message to users
    if (INVIS_LEVEL(ch)< LEVEL_HERO)
    {
        sprintf( buf, "Reboot by %s.  Logon again in 15 seconds.\r\n", ch->name );
    	do_echo( ch, buf );
    }

    // save all the characters
    ch->println( "Saving all player files." );
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
        vch_next = vch->next;
        if (!IS_NPC(vch))
        {
            save_char_obj( vch );
            vch->println( "You have been automatically saved due to the reboot...  disconnecting." );
            WAIT_STATE(vch,1 * PULSE_VIOLENCE); /* reduced from x4 */
        }
	}

	// save olc work
	reboot_autosave(ch);

    // save all laston data
    ch->println( "Closing and saving all laston records." );
    laston_save(ch);
    ch->println( "Laston save complete." );

    runlevel=RUNLEVEL_SHUTING_DOWN;
    for ( d = connection_list; d != NULL; d = c_next )
    {
		c_next = d->next;
    	connection_close(d);
    }
    
    return;
}


/**************************************************************************/
void do_shutdow( char_data *ch, char *)
{
    ch->println( "If you want to SHUTDO->, spell it out." );
    return;
}

/**************************************************************************/
void do_shutdown( char_data *ch, char *argument)
{
    char buf[MSL];
    char_data *vch, *vch_next;

	if(IS_NULLSTR(argument) || str_cmp("confirm", argument)){
        ch->println("`xType `=Cshutdown confirm`x to shut the mud down.");
		ch->println("note: the mud may automatically start back up if there "
			"is a startup script enabled.");
		return;
	}

    connection_data *d,*c_next;

	write_shutdown_file(ch);

    // display shutdown message to users
    sprintf( buf, "Shutdown by %s.\r\n", ch->name);
    do_echo( ch, buf );


    // save all the characters
    ch->println( "Saving all player files." );
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
        vch_next = vch->next;
        if (!IS_NPC(vch))
        {
            save_char_obj( vch );
			msp_to_room(MSPT_ACTION, MSP_SOUND_SHUTDOWN, 0, vch, true, false );
			vch->println( "You have been automatically saved due to the shutdown...  disconnecting." );
            WAIT_STATE(vch,1 * PULSE_VIOLENCE); 
        }
	}

	// save olc work
    reboot_autosave(ch);

    // save all laston data
    ch->println( "Closing and saving all laston records." );
    laston_save(ch);
    ch->println( "Laston save complete." );

    runlevel=RUNLEVEL_SHUTING_DOWN;

    for ( d = connection_list; d != NULL; d = c_next)
    {
        c_next = d->next;
        connection_close(d);
    }
    return;
}

/**************************************************************************/
void do_protect( char_data *ch, char *argument)
{
    char_data *victim;

    if (argument[0] == '\0')
    {
        ch->println( "Protect whom from snooping?" );
        return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
        ch->println( "You can't find them." );
        return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
		act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
		victim->println( "Your snoop-proofing was just removed." );
		REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
	}
	else
	{
		act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
		victim->println( "You are now immune to snooping." );
		SET_BIT(victim->comm,COMM_SNOOP_PROOF);
	}
}
  

/**************************************************************************/
void do_snoop( char_data *ch, char *argument )
{
	char arg[MIL];
	connection_data *c, *sc=NULL;
	char_data *victim=NULL;
	char buf[MSL];

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->println( "Snoop whom/which socket number?" );
		return;
    }

	if (is_number(arg))
	{
		int desc= atoi(arg);		
		
		for ( c = connection_list; c && sc==NULL; c = c->next ){
			if ( c->connected_socket == desc ){
				sc=c;
			}
		}

		if(!sc){
			ch->printlnf("Couldn't find any socket number %d on right now to snoop.", desc);
			return;
		}

	}else{
		if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL ){
			ch->printlnf( "Couldn't find any '%s' in the game.", arg );
			return;
		}

		sc=victim->desc;
		if ( !sc){
			ch->println( "No connected socket to snoop." );
			return;
		}		
	}

	// cancel snoops for self
	if ( sc== ch->desc)	{
		ch->println( "Cancelling all snoops." );
		wiznet("$N stops being such a snoop.",
			ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));

		for ( c = connection_list; c; c = c->next ){
			if ( c->snoop_by == ch->desc ){
				c->snoop_by = NULL;
			}
		}
		return;
	}

	// cancel a single existing snoop
	if ( sc->snoop_by == ch->desc){
		ch->printlnf("Canceling snoop on %s.", PERS(victim, ch));
		sc->snoop_by = NULL;
		return;
	}

	// only one snoop at a time
    if ( sc->snoop_by ){
		ch->println( "Busy already.");
		return;
	}

	if(!CH(sc)){
		ch->println( "You can't snoop a connection until a character is attached to it.");
		return;
	}

	// private room checks if player in the game
	if(CH(sc)->in_room){
		if ( ch->in_room != CH(sc)->in_room 
			&& is_room_private_to_char( CH(sc)->in_room, ch ) 
			&& !IS_TRUSTED(ch,CREATOR))
		{
			ch->println( "That character is in a private room." );
			return;
		}
	}

	if ( !IS_TRUSTED(ch, COUNCIL-1) 
		&& (get_trust( victim ) > 20 || IS_LETGAINED( CH(sc) ) ))
	{
		ch->println( "You can't snoop letgained players, or morts of that level/trust." );
		return;
	}

	if ( get_trust( CH(sc) ) >= get_trust( ch )
		|| (IS_SET(CH(sc)->comm,COMM_SNOOP_PROOF) && !IS_TRUSTED(ch, MAX_LEVEL)))
	{
		ch->println( "You failed." );
		return;
	}

	if ( ch->desc )
	{
		for ( c = ch->desc->snoop_by; c; c = c->snoop_by )
		{
			if ( c->character == CH(sc)|| c->original == CH(sc)){
				ch->println( "No snoop loops." );
				return;
			}
		}
	}

    sc->snoop_by = ch->desc;
    sprintf(buf,"$N starts snooping on %s",
		(IS_NPC(ch) ? CH(sc)->short_descr : CH(sc)->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));

	ch->printlnf("Ok, snooping %s.", PERS(CH(sc), ch));

	return;
}

/**************************************************************************/
void do_commandsnoop( char_data *ch, char *argument )
{
	char arg[MIL];
	connection_data *d;
	char_data *victim;
	char buf[MSL];

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg))
    {
		ch->println( "Command snoop whom?" );
		return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
		ch->println( "They aren't here." );
		return;
	}

    if ( victim->desc == NULL )
    {
		ch->println( "No descriptor to snoop." );
		return;
    }

    if ( victim == ch )
    {
		ch->println( "Cancelling all command snoops." );
		wiznet("$N stops being such a commandsnoop.",
			ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
		for ( d = connection_list; d != NULL; d = d->next )
		{
			if ( d->command_snoop == ch->desc )
				d->command_snoop = NULL;
		}
		return;
    }

    if ( victim->desc->command_snoop == ch->desc)
    {
		ch->printlnf("Canceling command snoop on %s.", 
			PERS(victim, ch));
		victim->desc->command_snoop  = NULL;
        return;
    }


    if ( victim->desc->command_snoop)
    {
        ch->println( "Busy already." );
        return;
    }

    if (ch->in_room != victim->in_room 
		&& is_room_private_to_char( victim->in_room, ch ) 
		&& !IS_TRUSTED(ch,CREATOR))
    {
        ch->println( "That character is in a private room." );
        return;
    }                   

    if ( !IS_ADMIN(ch) && (get_trust( victim ) > 20
    ||   IS_LETGAINED(victim)))
    {
        ch->println( "You can't command snoop letgained players, or morts of that level/trust." );
        return;
    }


    if ( get_trust( victim ) >= get_trust( ch )
    ||  (IS_SET(victim->comm,COMM_SNOOP_PROOF) && !IS_TRUSTED(ch, MAX_LEVEL)))
    {
        ch->println( "You failed." );
        return;
    }

    victim->desc->command_snoop = ch->desc;
    sprintf(buf,"$N starts command snooping on %s",
		(IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));

    ch->printlnf("Ok, command snooping %s.", 
		PERS(victim, ch));

	return;
}

/**************************************************************************/
void do_switch( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
    char_data *victim;

    one_argument( argument, arg );

	// using ' codes are optional
	if(!str_infix("'",argument)){
		argument = one_argument( argument, arg );
	}else{
		strcpy(arg, argument);
	}
    
    if ( arg[0] == '\0' )
    {
        ch->println( "Switch into whom?" );
        return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
        ch->println( "You are already switched." );
        return;
    }

    if (ch->desc->editor )
    {
        ch->println( "You can only switch while not in an OLC editor." );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

    if ( victim == ch )
    {
        ch->println( "You are already controlling yourself." );
        return;
    }

    if (!IS_NPC(victim))
    {
        ch->println( "You can only switch into mobiles." );
        return;
    }


    if (ch->in_room != victim->in_room 
		&& is_room_private_to_char( victim->in_room, ch ) 
		&& !IS_TRUSTED(ch,CREATOR))
    {
        ch->println( "That character is in a private room." );
        return;
    }

    if ( victim->desc != NULL )
    {
        ch->println( "Character in use." );
        return;
    }

	// questing wiznet
	if (TRUE_CH(ch))
	{
		char qbuf[MSL];
		sprintf (qbuf, "`mQUEST> %s `Mswitches`m into '%s'\n(%s)[%d]`x", 
			TRUE_CH(ch)->name, victim->short_descr, 
			victim->name, victim->in_room->vnum);
		wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
	}

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->controlling     = victim; // so switched imms aren't marked as idle/linkdead

    // change communications to match
    if (ch->prompt != NULL){
        victim->prompt = str_dup(ch->prompt);
	}

    victim->comm = ch->comm;
    victim->lines = ch->lines;
	victim->colour_prefix = ch->colour_prefix;
      
    victim->printlnf("`=\x8c""You are now controlling %s\r\n"
        "vnum: %d  level: %d  language: %s`x",
        victim->short_descr,  victim->pIndexData->vnum,
        victim->level, victim->language->name);
	victim->printlnf("note: you can use the `=C%s`x command to return to your original body.",
		mxp_create_send(ch, "return"));
	victim->printlnf("note: you can use the `=C%s`x command to toggle the addition prompt details.",
		mxp_create_send(ch, "switchprefix"));
    do_look(victim, "auto");

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    return;
}


/**************************************************************************/
void do_return( char_data *ch, char *)
{
    char buf[MSL];

    if ( ch->desc == NULL ){
		return;
	}

    if(ch->desc->original && !IS_IMMORTAL(ch->desc->original)){
 		do_huh(ch, "");
 		return;
    }

    if ( ch->desc->original == NULL )
    {
		if(IS_IMMORTAL(ch)){
			ch->println( "You aren't switched." );
		}else{
			do_huh(ch,"");
		}
        return;
    }

    ch->println( "You return to your original body." );

	if (ch->prompt != NULL){
        free_string(ch->prompt);
        ch->prompt = NULL;
    }

	// questing wiznet
	if (TRUE_CH(ch))
	{
		char qbuf[MSL];
		sprintf (qbuf, "`mQUEST> %s `Mreturns`m from '%s'\n(%s)[%d]`x", 
			TRUE_CH(ch)->name, ch->short_descr, ch->name, 
			ch->in_room?ch->in_room->vnum:0);
		wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
	}

    sprintf(buf,"$N returns from %s.",ch->short_descr);

    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
	char_data *original=ch->desc->original;
	ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc->character->controlling = NULL; // no longer controlling this mob
    ch->desc                  = NULL;   
    ch->last_force = tick_counter;
	
	do_look(original,"brief");
    return;
}

/**************************************************************************/
/* trust levels for load and clone */
bool obj_check (char_data *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GUARDIAN)
    || (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 30 && obj->cost <= 1000)
    || (IS_TRUSTED(ch,DEMI)     && obj->level <= 20 && obj->cost <= 500)
    || (IS_TRUSTED(ch,ANGEL)    && obj->level <= 10 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return true;
    else
	return false;
}

/**************************************************************************/
/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(char_data *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/**************************************************************************/
/* command that is similar to load */
void do_clone(char_data *ch, char *argument )
{
    char arg[MIL];
    char *rest;
    char_data *mob;
	 OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
		ch->println( "Clone what?" );
		return;
	}

	if (!str_prefix(arg,"object"))
	{
		mob = NULL;
		obj = get_obj_here(ch,rest);
		if (obj == NULL)
		{
			ch->println( "You don't see that here." );
			return;
		}
	}
	else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
	{
		obj = NULL;
		mob = get_char_room(ch,rest);
		if (mob == NULL)
		{
			ch->println( "You don't see that here." );
			return;
		}
	}
	else /* find both */
	{
		mob = get_char_room(ch,argument);
		obj = get_obj_here(ch,argument);
		if (mob == NULL && obj == NULL)
		{
			ch->println( "You don't see that here." );
			return;
		}
	}

	/* clone an object */
	if (obj != NULL)
	{
		OBJ_DATA *clone;

		if (!obj_check(ch,obj))
		{
			ch->println( "Your powers are not great enough for such a task." );
			return;
		}

		clone = create_object(obj->pIndexData); 
		clone_object(obj,clone);
		if (obj->carried_by != NULL)
			obj_to_char(clone,ch);
		else
			obj_to_room(clone,ch->in_room);
		recursive_clone(ch,obj,clone);

		act("$n has created $p.",ch,clone,NULL,TO_ROOM);
		act("You clone $p.",ch,clone,NULL,TO_CHAR);
		wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
		return;
	}
	else if (mob != NULL)
	{
		char_data *clone;
		OBJ_DATA *new_obj;
		char buf[MSL];
		if (!IS_NPC(mob))
		{
			ch->println( "You can only clone mobiles." );
			return;
		}
		if ((mob->level > 20 && !IS_TRUSTED(ch,GUARDIAN))
			||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
			||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
			||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
			||  !IS_TRUSTED(ch,AVATAR))

		{
			ch->println( "Your powers are not great enough for such a task." );
			return;
		}
		
		clone = create_mobile(mob->pIndexData, 0);
		clone_mobile(mob,clone); 
		
		for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
		{
			if (obj_check(ch,obj))
			{
				new_obj = create_object(obj->pIndexData);
				clone_object(obj,new_obj);
				recursive_clone(ch,obj,new_obj);
				obj_to_char(new_obj,clone);
				new_obj->wear_loc = obj->wear_loc;
			}
		}
		char_to_room(clone,ch->in_room);
		act("$n has created $N.",ch,NULL,clone,TO_ROOM);
		act("You clone $N.",ch,NULL,clone,TO_CHAR);
		sprintf(buf,"$N clones %s.",clone->short_descr);
		wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
		return;
	}
}


/**************************************************************************/
void do_load(char_data *ch, char *argument )
{
	char arg[MIL];
	
    argument = one_argument(argument,arg);
	
	if (arg[0] == '\0')
    {
		ch->println( "Syntax:" );
		ch->println( "  load mob <vnum>" );
		ch->println( "  load obj <vnum> <level>" );
		return;
    }
	
    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
		do_mload(ch,argument);
		return;
    }
	
    if (!str_cmp(arg,"obj") || !str_cmp(arg,"object"))
    {
		if(GAMESETTING5(GAMESET5_DEDICATED_OLC_BUILDING_MUD)){
			// allow object loading on dedicated OLC muds for < 95
			if(!IS_TRUSTED(ch, game_settings->min_level_to_load_objects)){
				ch->printlnf("You must have a trust of %d or higher to load objects.", 
					game_settings->min_level_to_load_objects);
			}else{
				do_oload(ch,argument);
			}
		}else{ 
			do_oload( ch, argument); 
		}
		return;
    }
    // echo syntax
    do_load(ch,"");
}

/**************************************************************************/
void do_mload( char_data *ch, char *argument )
{
    char arg[MIL];
    MOB_INDEX_DATA *pMobIndex;
    char_data *victim;
    char buf[MSL];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
		ch->println( "Syntax: load mob <vnum>." );
		return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
		ch->println( "No mob has that vnum." );
		return;
    }

    victim = create_mobile( pMobIndex, 0 );
    char_to_room( victim, ch->in_room );
	// make it so they don't wander away straight away
	victim->last_force = tick_counter; 
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
	if (INVIS_LEVEL(ch))
		ch->printlnf("[Wizi %d] You have created %s!", 
		INVIS_LEVEL(ch), victim->short_descr);
	else
		ch->printlnf("You have created %s!", victim->short_descr);

	sprintf(buf,"$N loads %s.",victim->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));

	return;
}


/**************************************************************************/
void do_oload( char_data *ch, char *argument )
{
    char arg1[MIL] ,arg2[MIL];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
		ch->println( "Syntax: load obj <vnum> <level>." );
		return;
    }
    
	level = get_trust(ch); // default 
  
    if ( arg2[0] != '\0')  // load with a level 
    {
		if (!is_number(arg2))
        {
			ch->println( "Syntax: oload <vnum> <level>." );
			return;
		}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
		{
			ch->println( "Level must be be between 0 and your level." );
			return;
		}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
		ch->println( "No object has that vnum." );
		return;
    }

    obj = create_object( pObjIndex);

	// all tokens are 'takeable' 
	if(obj->item_type==ITEM_TOKEN){
		SET_BIT(obj->wear_flags, OBJWEAR_TAKE);
	}

	if ( CAN_WEAR(obj, OBJWEAR_TAKE) ){
		obj_to_char( obj, ch );
		if (INVIS_LEVEL(ch)){
			ch->printlnf("[Wizi %d] You have created '%s' and it is now in your inventory!", 
				INVIS_LEVEL(ch), obj->short_descr);
		}else{
			ch->printlnf("You have created '%s' and it is now in your inventory!", 
				obj->short_descr);
		}
	}else{
		obj_to_room( obj, ch->in_room );
		if (INVIS_LEVEL(ch)){
			ch->printlnf("[Wizi %d] You have created '%s' and put it on the floor!", 
				INVIS_LEVEL(ch), obj->short_descr);
		}else{
			ch->printlnf("You have created '%s' and put it on the floor!", 
				obj->short_descr);
		}
	}
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );


    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    return;
}

/**************************************************************************/
void do_purge( char_data *ch, char *argument )
{
    char arg[MIL];
    char buf[100];
    char_data *victim;
    OBJ_DATA *obj;
    connection_data *d;

    one_argument( argument, arg );

    if(IS_NULLSTR(arg)){
		// 'purge' the whole room
		char_data *vnext;
		OBJ_DATA  *obj_next;

		for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
		{
			vnext = victim->next_in_room;
			if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
			&&   victim != ch /* safety precaution */ )
			extract_char( victim, true );
		}

		for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;
			if (!IS_OBJ_STAT(obj,OBJEXTRA_NOPURGE)){
				extract_obj( obj );
			}
		}

		act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
		ch->in_room->last_mined_in_room=0; // clear the last mined stats
		ch->println( "Ok, room purged." );
		return;
    }

	// check for objects and mobs in room to purge
	if ( ( obj = get_obj_list( ch, arg, ch->in_room->contents ) ) != NULL ){
		if (!IS_OBJ_STAT(obj,OBJEXTRA_NOPURGE)){
			ch->printlnf( "Single object (%s) purged.", format_obj_to_char( obj, ch, true));
			extract_obj(obj);
		}else{
			ch->printlnf( "Single object (%s) was purged even though it is flagged no purge.",
				format_obj_to_char( obj, ch, true));
			extract_obj(obj);			
		}
		return;
	}

	// first try a mob/player in the current room
	if ( ( victim = get_char_room( ch, arg ) ) == NULL ){
		// if that fails, try all players on visible on the wholist
		if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL ){
			ch->println( "They aren't here." );
			return;
		}
    }
	

    if ( !IS_NPC(victim) ){
		if (ch == victim || !IS_IMMORTAL(ch)){
			ch->println( "Ho ho ho." );
			return;
		}

		if (get_trust(ch) <= get_trust(victim)){
			ch->println( "Maybe that wasn't a good idea..." );
			victim->printlnf("%s tried to purge you!",ch->name);
			return;
		}

		act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);
		
		ch->printlnf("Purging %s",victim->name);

		sprintf(buf, 
		"The all powerful %s has ripped you to shreds with %s bare hands,\r\n"
		"your atoms are scattered!\r\n", 
			TRUE_CH(ch)->name, TRUE_CH(ch)->sex==SEX_MALE?"his":"her");
		
    		d = victim->desc;
			if (d){// display a nice little message to them
				d->write(buf, 0);
			}
			if (victim->level > 1){
				save_char_obj( victim );
			}
    		extract_char( victim, true );
			if ( d){
				connection_close( d );
			}

		return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( "You purge $N.", ch, NULL, victim, TO_CHAR );
    extract_char( victim, true );
    return;
}

/**************************************************************************/
// purges rooms for those with olc access in the area
void do_rpurge(char_data *ch,char *)
{
	if (!HAS_SECURITY(ch,2))
	{
		ch->println( "You must have an olc security 2 or higher to use this command." );
		return;
	}

	if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_ROOMS ) )
	{
		ch->println( "Insufficient security to modify the area you are in." );
        return;
	}

	if (!IS_SET(ch->in_room->area->area_flags, AREA_OLCONLY) )
	{
		ch->println( "This command can't be used in live areas." );
		return;
	}

	do_purge( ch, "");
	ch->println( "`xRoom purged... type `=Credit reset`x to reapply resets." );
	
}

/**************************************************************************/
void do_update( char_data *ch, char *argument )
{
	char arg1[MIL];
	char arg2[MIL];
	char_data *victim;
	int amount;
	int i;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if(arg1[0] == '\0' || arg2[0]=='\0' || !is_number(arg2))
	{
		ch->println( "Syntax: update <char> <times>");
		return;
	}

	if( (victim=get_char_world(ch,arg1) ) == NULL )
	{
		ch->println( "They are not here." );
		return;
	}

	if(IS_NPC(victim))
		return;
	
	amount=atoi(arg2);

	for(i=0; i<amount; i++)
		on_level_learn(victim);

	return;
}

/**************************************************************************/
void do_advance( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char_data *victim;
    int level;
    int iLevel;
	bool not_letgained=false;

	if(IS_NPC(ch)){
		ch->println("Players online sorry.");
		return;
	}

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        ch->println( "Syntax: advance <char> <level>." );
		return;
	}

	if ( ( victim = get_whovis_player_world( ch, arg1 ) ) == NULL )
	{
		ch->println( "That player is not here." );
		return;
	}

	if ( IS_NPC(victim) )
	{
		ch->println( "Not on NPC's." );
		return;
	}

    if ( ( level = atoi( arg2 ) ) < 1 || level > ABSOLUTE_MAX_LEVEL )
    {
        ch->printlnf("Level must be 1 to %d.", ABSOLUTE_MAX_LEVEL );
        return;
    }

    if ( level > get_trust( ch ) )
    {
        ch->println( "Limited to your trust level." );
        return;
    }

    if ( get_trust( victim ) > get_trust( ch) && (get_trust( victim )<=ABSOLUTE_MAX_LEVEL) ){
        ch->println( "Their trust/level is higher than yours... you can't do that." );
		return;
	}

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */

	if ( level <= victim->level ){
        int temp_prac;

		ch->println( "Lowering a player's level!" );
		victim->println( "**** OOOOHHHHHHHHHH  NNNNOOOO ****" );
        temp_prac = victim->practice;
        victim->level    = 1;
        victim->exp      = exp_per_level(victim,victim->pcdata->points);
        victim->max_hit  = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        victim->practice = 0;
        victim->hit      = victim->max_hit;
        victim->mana     = victim->max_mana;
        victim->move     = victim->max_move;
        advance_level( victim );
        victim->practice = temp_prac;
    }else{
		ch->println( "Raising a player's level!" );
		victim->println( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****" );
    }

	if(!IS_LETGAINED(victim)){
		not_letgained=true;
		SET_BIT(victim->act,PLR_CAN_ADVANCE);
	}
	// can't get yourself into a situation where you lower your 
	// level and lose your ability to up it again
	if(ch==victim){
		if(level<ch->level && level>ch->trust){
			ch->trust=level;
			ch->printlnf("Your trust has been increased to %d to "
				"compenstate the drop in level.", level);
		}
	}
    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
        victim->print( "You raise a level!!  " );
        victim->level += 1;
        advance_level( victim );
    }
    victim->exp   = exp_per_level(victim,victim->pcdata->points)
		  * UMAX( 1, victim->level );
	if(victim!=ch){
	    victim->trust = 0;
		ch->printlnf("Trust set to 0 on %s", victim->name);
	}

    // reset RPS score 
    if (victim->pcdata->rp_points<0){
        victim->pcdata->rp_points = 0;
	}

	// move the pfile if required
	{
		PFILE_TYPE pt=get_pfiletype(victim);
		if(victim->pcdata->pfiletype!=pt){
			rename(pfilename(victim->name,victim->pcdata->pfiletype),
				pfilename(victim->name,pt)); // move the file
			victim->pcdata->pfiletype=pt;
		}
	}
	if(not_letgained){
		REMOVE_BIT(victim->act,PLR_CAN_ADVANCE);
		ch->printlnf("Note: %s isn't letgained.", victim->name);
	}

	if(IS_IMMORTAL(victim)){
		victim->wrapln("Immortal hint, you can set some default wiznet "
			"settings using 'wiznetdefault confirm'.");
	}
	laston_update_char(victim);
    save_char_obj(victim);
    return;
}

/**************************************************************************/
/* tadvance is advance without the change in trust */
void do_tadvance( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char_data *victim;
    int level;
    int iLevel;
	bool not_letgained=false;

	if(IS_NPC(ch)){
		ch->println("Players online sorry.");
		return;
	}

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        ch->println( "Syntax: tadvance <char> <level>." );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        ch->println( "That player is not here." );
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println( "Not on NPC's." );
        return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > ABSOLUTE_MAX_LEVEL )
    {
        ch->printlnf("Level must be 1 to %d.", ABSOLUTE_MAX_LEVEL );
        return;
    }

    if ( level > get_trust( ch ) )
    {
        ch->println( "Limited to your trust level." );
        return;
    }

    if ( get_trust( victim ) > get_trust( ch) && (get_trust( victim )<=ABSOLUTE_MAX_LEVEL) )
    {
        ch->println( "Their trust/level is higher than yours... you can't do that." );
        return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

        ch->println( "Lowering a player's level!" );
        victim->println( "**** OOOOHHHHHHHHHH  NNNNOOOO ****" );
        temp_prac = victim->practice;
        victim->level    = 1;
        victim->exp      = exp_per_level(victim,victim->pcdata->points);
        victim->max_hit  = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        victim->practice = 0;
        victim->hit      = victim->max_hit;
        victim->mana     = victim->max_mana;
        victim->move     = victim->max_move;
        advance_level( victim );
        victim->practice = temp_prac;
    }
    else
    {
        ch->println( "Raising a player's level!" );
        victim->println( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****" );
    }

	if(!IS_LETGAINED(victim)){
		not_letgained=true;
		SET_BIT(victim->act,PLR_CAN_ADVANCE);
	}
    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
        victim->print( "You raise a level!!  " );
        victim->level += 1;
        advance_level( victim );
    }
    victim->exp   = exp_per_level(victim,victim->pcdata->points)
		  * UMAX( 1, victim->level );

    /* reset RPS score */
    if (victim->pcdata->rp_points<0)
        victim->pcdata->rp_points = 0;

    ch->printlnf("Advanced to %d, trust = %d ", level, victim->trust);

	// move the pfile if required
	{
		PFILE_TYPE pt=get_pfiletype(victim);
		if(victim->pcdata->pfiletype!=pt){
			rename(pfilename(victim->name,victim->pcdata->pfiletype),
				pfilename(victim->name,pt)); // move the file
			victim->pcdata->pfiletype=pt;
		}
	}

	if(not_letgained){
		REMOVE_BIT(victim->act,PLR_CAN_ADVANCE);
		ch->printlnf("Note: %s isn't letgained.", victim->name);
	}

	laston_update_char(victim);
    save_char_obj(victim);
    return;
}

/**************************************************************************/
void do_trust( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char_data *victim;
    int level;

	if(IS_NPC(ch)){
		ch->println("Players online sorry.");
		return;
	}

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        ch->println( "Syntax: trust <char> <level>." );
        return;
    }

    if ( ( victim = get_whovis_player_world( ch, arg1 ) ) == NULL )
    {
        ch->println( "That player is not here." );
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println( "Not on NPC's." );
        return;
    }

    level = atoi( arg2 );

    if ( level < 0 || level > ABSOLUTE_MAX_LEVEL)
    {
        ch->printlnf( "Level must be 0 (reset) or 1 to %d.\r\n"
			"And not higher than your current trust.", ABSOLUTE_MAX_LEVEL);
        return;
    }

    if ( level > get_trust( ch ) )
    {
        ch->println( "Limited to your trust - not set." );
        return;
    }

	if( victim->level== level ){
		ch->printlnf("%s is already level %d, so setting their trust to 0 (from %d) since it is not required.",
			PERS(victim, ch), level, victim->trust);
		level=0;
	}

	if( victim->trust== level ){
		ch->printlnf("%s's trust is already at %d", PERS(victim, ch), level);
		return;
	}


    ch->printlnf("Trust on %s has been changed from %d to %d",
        victim->name, victim->trust, level);
    victim->trust = level;

	// move the pfile if required
	{
		PFILE_TYPE pt=get_pfiletype(victim);
		if(victim->pcdata->pfiletype!=pt){
			rename(pfilename(victim->name,victim->pcdata->pfiletype),
				pfilename(victim->name,pt)); // move the file
			victim->pcdata->pfiletype=pt;
		}
	}

	laston_update_char(victim);
    save_char_obj(victim);

    return;
}

/**************************************************************************/
void restore_victim( char_data *ch, char_data *victim)
{
	affect_parentspellfunc_strip( victim, gsn_plague		);
	affect_parentspellfunc_strip( victim, gsn_poison		);
	affect_parentspellfunc_strip( victim, gsn_blindness		);
	affect_parentspellfunc_strip( victim, gsn_sleep			);
	affect_parentspellfunc_strip( victim, gsn_curse			);
	affect_parentspellfunc_strip( victim, gsn_cause_fear	);
	affect_parentspellfunc_strip( victim, gsn_fear_magic    );
	affect_parentspellfunc_strip( victim, gsn_thorny_feet	);
	affect_parentspellfunc_strip( victim, gsn_chaotic_poison);
	affect_parentspellfunc_strip( victim, gsn_cause_headache);
	affect_parentspellfunc_strip( victim, gsn_weaken		);
    affect_strip(victim, gsn_neck_thrust);
	
	// subdue
	victim->subdued = false;
	victim->subdued_timer=0;
	
	if(!IS_NPC(victim))
	{
		// tiredness
		victim->pcdata->tired=UMIN(0, victim->pcdata->tired);

		// conditions
		victim->pcdata->condition[COND_THIRST]=
			UMAX(30, victim->pcdata->condition[COND_THIRST]);
		victim->pcdata->condition[COND_HUNGER]=
			UMAX(30,victim->pcdata->condition[COND_HUNGER]);
		victim->pcdata->condition[COND_FULL]=
			UMIN(0,victim->pcdata->condition[COND_FULL]);
	    victim->pcdata->condition[COND_DRUNK]=
			UMIN(0,victim->pcdata->condition[COND_DRUNK]);
	}
	victim->hit 	= victim->max_hit;
	victim->mana	= victim->max_mana;
	victim->move	= victim->max_move;
	victim->wait	= 0; // remove lag
	
	update_pos( victim);
	
	if (ch && victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "$n has restored you!", ch, NULL, victim, TO_VICT);
			}else{
				if (can_see_who(victim, TRUE_CH(ch))){
					victim->printlnf("%s has restored you.", TRUE_CH(ch)->name);
				}else{
					victim->println("a mystery imm has restored you.");
				}
				msp_to_room(MSPT_ACTION, MSP_SOUND_RESTORE, 0, victim, true, false );
			}
		}
	}
}
/**************************************************************************/
void do_restore( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
	char_data *victim;
	char_data *vch;
	connection_data *d;

	one_argument( argument, arg );
	if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
	{
		// restore room
		for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
		{
			restore_victim( ch, vch);
        }
        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));      
		ch->println("Room restored." );
        return;
    }

	if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
	{
		// restore all players in the game
		for (d = connection_list; d != NULL; d = d->next)
		{
			victim = d->character;

			if (victim == NULL || IS_NPC(victim))
				continue;

			restore_victim( ch, victim);
		}
		ch->println( "All active players restored." );
		return;
	}

	// restore a specific player/mob
	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	if ( !can_see_who( ch, victim ))
	{
		ch->println( "They aren't here." );
		return;
	}

	restore_victim( ch, victim);

	sprintf(buf,"$N restored %s", IS_NPC(victim) ? victim->short_descr : victim->name);
	wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

	if (IS_SILENT(ch)){
		ch->printlnf("Ok - restore done silently on %s.", victim->name );
	}else{
		ch->printlnf("Ok - restore done on %s.", victim->name );
	}
	return;
}
/**************************************************************************/
void boon_victim( char_data *ch, char_data *victim)
{
	if(IS_NPC(victim)){ // don't boon mobs at this stage :)
		return;
	}
	restore_victim( NULL, victim);

	affect_parentspellfunc_strip( victim, gsn_bless);
	affect_parentspellfunc_strip( victim, gsn_armor);
	affect_parentspellfunc_strip( victim, gsn_giant_strength);
	affect_parentspellfunc_strip( victim, gsn_illusions_grandeur);
	affect_parentspellfunc_strip( victim, gsn_shield);
	affect_parentspellfunc_strip( victim, gsn_sanctuary);	
	spell_bless(				gsn_bless,				ch->level, ch, victim, TARGET_CHAR);
	spell_armor(				gsn_armor,				ch->level, ch, victim, TARGET_CHAR);
	spell_giant_strength(		gsn_giant_strength,		ch->level, ch, victim, TARGET_CHAR);	
	spell_illusions_grandeur(	gsn_illusions_grandeur,	ch->level, ch, victim, TARGET_CHAR);
	spell_shield(				gsn_shield,				ch->level, ch, victim, TARGET_CHAR);
	spell_sanctuary(			gsn_sanctuary,			ch->level, ch, victim, TARGET_CHAR);
	
	if (victim->in_room != NULL)
	{
		if (!IS_SILENT(ch) || IS_IMMORTAL(victim))
		{
			if (IS_NPC(ch))
			{
				act( "$n has given you a boon from the gods!", ch, NULL, victim, TO_VICT);
			}else{
				if (can_see_who(victim, TRUE_CH(ch))){
					victim->printlnf("%s has given you a boon from the gods!", TRUE_CH(ch)->name);
				}else{
					victim->println("a mystery imm has given you a boon from the gods!");
				}
				msp_to_room(MSPT_ACTION, MSP_SOUND_RESTORE, 0, victim, true, false );
			}
		}
	}	
}

/**************************************************************************/
// Kal - Jan 99
void do_boon( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
	char_data *victim;
	char_data *vch;

	one_argument( argument, arg );
	if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
	{
		// boon room
		for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
		{
			boon_victim( ch, vch);
        }
        sprintf(buf,"$N boon'd room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));      
		ch->println( "Room boon'd." );
        return;
    }

	if ( get_trust(ch) >=  MAX_LEVEL - 3 && !str_cmp(arg,"all"))
	{
		// restore all players in the game
		for(victim=player_list; victim; victim=victim->next_player)
		{
			if(IS_NPC(victim))
				continue;

			boon_victim( ch, victim);
		}
		ch->println( "All active players boon'd." );
		return;
	}

	// restore a specific player/mob
	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}
	boon_victim( ch, victim);

	sprintf(buf,"$N restored %s", IS_NPC(victim) ? victim->short_descr : victim->name);
	wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,get_trust(ch));

	if (IS_SILENT(ch)){
		ch->printlnf("Ok - boon done silently on %s.", victim->name );
	}else{
		ch->printlnf("Ok - boon done on %s.", victim->name );
	}
	return;
}

/**************************************************************************/
void do_freeze( char_data *ch, char *argument )
{
	char arg[MIL],buf[MSL];
	char_data *victim;
	
	one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "Freeze whom?" );
		return;
    }
	
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
		ch->println( "They aren't here." );
		return;
    }
	
    if ( IS_NPC(victim) )
    {
		ch->println( "Not on NPC's." );
		return;
    }

	if ( victim==ch){
		ch->println( "And how would you unfreeze yourself if you did that?" );
		return;
	}
	
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
		ch->println( "You failed." );
		return;
	}
	
    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
		REMOVE_BIT(victim->act, PLR_FREEZE);
		victim->println( "You can play again." );
		ch->println( "FREEZE removed." );
		sprintf(buf,"$N thaws %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
		SET_BIT(victim->act, PLR_FREEZE);
		victim->println( "You can't do ANYthing!" );
		ch->println( "FREEZE set." );
		sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
	
    save_char_obj( victim );
	
    return;
}


/**************************************************************************/
void do_log( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
    char buf[MSL];

    one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->println( "Log whom?" );
		return;
	}

	if ( !str_cmp( arg, "all" ) )
	{
		if ( fLogAll )
		{
			fLogAll = false;
			ch->println( "Log ALL off." );
		}
		else
		{
			fLogAll = true;
			ch->println( "Log ALL on." );
		}
		return;
	}

	if ( !str_cmp( arg, "memory" ) )
	{
		if ( log_memory )
		{
			log_memory= false;
			ch->println( "Log memory off." );
		}
		else
		{
			log_memory= true;
			ch->println( "Log memory on." );
		}
		return;
	}

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
    }

    if ( IS_NPC(victim) )
    {
		ch->println( "Not on NPC's." );
		return;
    }

    // No level check, imms can log anyone.
    if ( IS_SET(victim->act, PLR_LOG) )
    {
        REMOVE_BIT(victim->act, PLR_LOG);
        sprintf( buf, "Player log turned OFF by %s", ch->name);
        append_playerlog( victim, buf);
        ch->printlnf("LOG removed on %s.", victim->name);
    }
    else
    {
        SET_BIT(victim->act, PLR_LOG);
        sprintf( buf, "Player log turned ON by %s", ch->name);
        append_playerlog( victim, buf);
        ch->printlnf("LOG set on %s.", victim->name);
    }

    save_char_obj( victim );
    return;
}

/**************************************************************************/
void do_spelldebug( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	{
		ch->println( "Spelldebug whom?" );
		return;
    }

    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
    }

    if ( IS_NPC(victim) )
    {
		ch->println( "Not on NPC's." );
		return;
    }

    /*
     * No level check, gods can spell debug anyone.
     */
    if ( IS_SET(victim->comm, COMM_SPELL_DEBUG) )
    {
        REMOVE_BIT(victim->comm, COMM_SPELL_DEBUG);
        ch->println( "Spelldebug removed." );
    }
    else
    {
        SET_BIT(victim->comm, COMM_SPELL_DEBUG);
        ch->println( "Spelldebug set." );
    }

    return;
}

/**************************************************************************/
void do_noemote( char_data *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    char_data *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
		ch->println( "Noemote whom?" );
		return;
	}

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
		ch->println( "They aren't here." );
		return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
		ch->println( "You failed." );
		return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
		REMOVE_BIT(victim->comm, COMM_NOEMOTE);
		victim->println( "You can emote again." );
		ch->println( "NOEMOTE removed." );
		sprintf(buf,"$N restores emotes to %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->comm, COMM_NOEMOTE);
		victim->println( "You can't emote!" );
		ch->println( "NOEMOTE set." );
		sprintf(buf,"$N revokes %s's emotes.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}


/**************************************************************************/
void do_noshout( char_data *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    char_data *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
		ch->println( "Noshout whom?" );
		return;
	}

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
		ch->println( "They aren't here." );
		return;
    }

    if ( IS_NPC(victim) )
    {
		ch->println( "Not on NPC's." );
		return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
		ch->println( "You failed." );
		return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
		REMOVE_BIT(victim->comm, COMM_NOSHOUT);
		victim->println( "You can shout again." );
		ch->println( "NOSHOUT removed." );
		sprintf(buf,"$N restores shouts to %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
	else
    {
		SET_BIT(victim->comm, COMM_NOSHOUT);
		victim->println( "You can't shout!" );
		ch->println( "NOSHOUT set." );
		sprintf(buf,"$N revokes %s's shouts.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

	return;
}


/**************************************************************************/
void do_notell( char_data *ch, char *argument )
{
    char arg[MIL],buf[MSL];
	char_data *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
		ch->println( "Notell whom?" );
		return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
		ch->println( "They aren't here." );
		return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
		ch->println( "You failed." );
		return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
		REMOVE_BIT(victim->comm, COMM_NOTELL);
		victim->println( "You can use tells again." );
		ch->println( "NOTELL removed." );
		sprintf(buf,"$N restores tells to %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
	{
		SET_BIT(victim->comm, COMM_NOTELL);
		victim->println( "You can't use tells!" );
		ch->println( "NOTELL set." );
		sprintf(buf,"$N revokes %s's tells.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}

/**************************************************************************/
void do_xpen( char_data *ch, char *argument )
{
	char arg[MIL], arg2[MIL];
	char_data *victim;

	argument=one_argument( argument, arg );
	argument=one_argument( argument, arg2 );

	if ( IS_NULLSTR(arg))
	{
		ch->println( "Syntax: xpen <character> <ticks>" );
		ch->println( "xpen - Experience penalize players "
			"- they get 0 xp for the amount of ticks set." );
		ch->println( "Penalize whom?" );
		return;
	}

	if ( IS_NULLSTR(arg2))
	{
		ch->println( "Syntax: xpen <character> <ticks>" );
		ch->println( "xpen - Experience penalize players "
			"- they get 0 xp for the amount of ticks set." );
		ch->printlnf("Penalize '%s' how much?", arg);
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	if(IS_NPC(victim)){
		ch->println( "On players only sorry." );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		ch->println( "You failed." );
		return;
	}

	if(is_number(arg2))
	{
		int value=atoi(arg2);
		if(value){
			ch->printlnf("Xpen of %d set on %s", value, PERS(victim, ch));
			victim->pcdata->xp_penalty=value;
		}else{
			ch->println( "xpen - Experience penalize players "
				"- they get 0 xp for the amount of ticks set." );
			ch->println( "Syntax: xpen <character> <ticks>" );
		}
	}else{
		ch->println("The amount of the xp penatly must be numeric.");
	}
	return;
}


/**************************************************************************/
void do_peace( char_data *ch, char *)
{
    char_data *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
		if ( rch->fighting){			
			ch->printlnf("---%s stops fighting %s", CPERS(rch, ch), PERS(rch->fighting, ch));
			if(rch->fighting->fighting==ch){
				ch->printlnf("---%s stops fighting %s", CPERS(rch->fighting, ch), PERS(rch, ch));
			}
			stop_fighting( rch, true );
		}
		if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE)){
			ch->printlnf("Aggressive flag removed on %s", PERS(rch, ch));
			REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
		}
		mobRememberClear( rch );
    }

    ch->println( "You bring peace to the room." );
    return;
}

/**************************************************************************/
void do_slay( char_data *ch, char *argument )
{
    char_data *victim;
    char arg[MIL];
	
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
		ch->println( "Slay whom?" );
		return;
    }
	
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
		ch->println( "They aren't here." );
		return;
    }
	
    if ( ch == victim )
    {
		ch->println( "Suicide is a mortal sin." );
		return;
    }
	
    if ( !IS_NPC(victim)){
		if((victim->level >= get_trust(ch)) || !IS_ICIMMORTAL(ch))
		{
			ch->println( "You failed." );
			return;
		}
	}
	
    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim, ch );
    return;
}
/**************************************************************************/
void do_allowimmtalk( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;

    argument=one_argument( argument, arg );

    if ( IS_NULLSTR(arg) ){
		ch->println( "Give immtalk ablities to which mortal?" );
		return;
    }
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
    }

    if ( IS_NPC(victim) )
    {
		ch->println( "Not on NPC's." );
		return;
    }

    if ( IS_IMMORTAL(victim) )
    {
		ch->printlnf("%s is already an immortal.", PERS(victim, ch));
		return;
    }

    if(!IS_NULLSTR(victim->pcdata->immtalk_name))
    {
		ch->printlnf("Immtalk ability removed from %s.",PERS(victim, ch));
		replace_string(victim->pcdata->immtalk_name,"");
    }else{ // Adding an immtalk name
		char immname[MIL];
		one_argument(argument, immname);
		if(IS_NULLSTR(immname)){
			ch->println("To give immtalk to a mortal, you must specify the name of the imm they play also.");
			ch->println("syntax: allowimmtalk <playername> <immname>");
			return;
		}
		if(count_char(immname, COLOURCODE)){
			ch->println("You can't have colour code characters in immtalk names.");
			return;
		}
		ch->printlnf("Immtalk ability granted to %s (%s).",
			PERS(victim, ch), capitalize(immname));
		replace_string(victim->pcdata->immtalk_name,capitalize(immname));
    }
    save_char_obj( victim );
    return;
}
/**************************************************************************/
void do_ignoremultilogins( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	{
		ch->println( "Ignore multiloggings on who?" );
		return;
    }
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
    }

    if ( IS_NPC(victim) )
    {
		ch->println( "Not on NPC's." );
		return;
    }


    if (HAS_CONFIG(victim, CONFIG_IGNORE_MULTILOGINS))
    {
		ch->printlnf("Multilogins are now seen on %s.",PERS(victim, ch));
		REMOVE_CONFIG(victim, CONFIG_IGNORE_MULTILOGINS);
    }
    else
    {
		ch->printlnf("Multilogins are now ignored on %s.",PERS(victim, ch));
		SET_CONFIG(victim, CONFIG_IGNORE_MULTILOGINS);
    }
    save_char_obj( victim );
    return;
}
/**************************************************************************/
void do_disallowpkill( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	{
		ch->println( "disallow pkilling on which mortal?" );
		return;
    }
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
    }

    if ( IS_NPC(victim) )
    {
		ch->println( "Not on NPC's." );
		return;
    }

    if (HAS_CONFIG(victim, CONFIG_DISALLOWED_PKILL))
    {
		ch->printlnf("Pkill ability granted to %s.",PERS(victim, ch));
		REMOVE_CONFIG(victim, CONFIG_DISALLOWED_PKILL);
    }
    else
    {
		ch->printlnf("Pkill ability removed from %s.",PERS(victim, ch));
		SET_CONFIG(victim, CONFIG_DISALLOWED_PKILL);
    }
    save_char_obj( victim );
    return;
}
/**************************************************************************/
void do_addcourt( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;

	one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
	{
		ch->println( "Add which person to get court notes?" );
		return;
    }
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
    }

    if ( IS_NPC(victim) )
    {
		ch->println( "Not on NPC's." );
		return;
    }

    if (HAS_CONFIG(victim, CONFIG_COURTMEMBER))
    {
		ch->printlnf("Courtmembership removed from %s.",PERS(victim, ch));
		REMOVE_CONFIG(victim, CONFIG_COURTMEMBER);
    }
    else
    {
		ch->printlnf("Courtmembership granted to %s.",PERS(victim, ch));
		SET_CONFIG(victim, CONFIG_COURTMEMBER);
    }
    save_char_obj( victim );
    return;
}
/**************************************************************************/
