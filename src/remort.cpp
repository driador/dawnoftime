/**************************************************************************/
// remort.cpp - Kals hacked up remort code :)
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/*
* remort code - written by Kalahn
* - This code has been written to provide remort support, while Dawn didn't
*   officially have remort, a knew a lot of muds would want it and I 
*   couldn't stand the hardcoded nature of the available remort snippets.
*/
/**************************************************************************/
#include "include.h"
#include "nanny.h"

void save_char_obj_to_filename( char_data *ch, char *filename );
void display_race_selection(connection_data *d);
/**************************************************************************/
// oc = old_char
void begin_remort( char_data *oc) 
{
	char_data *ch;
	int stat;

	// no character name on old character - safety check
	if (IS_NULLSTR(oc->name)){
		bug("begin_remort(): No name attached to oc!!! - aborting!!!");
		do_abort();
	}

	oc->wraplnf("You are beginning remort %d rerolling!!!  You can reroll as many times "
		"as you like, just log in again if you are disconnected during creation to "
		"start again.", oc->beginning_remort);
		
	ch = new_char();
	ch->pcdata = new_pcdata();

	replace_string(ch->name				, oc->name );
	replace_string(ch->pcdata->unlock_id, oc->pcdata->unlock_id);
	replace_string(ch->pcdata->email,  oc->pcdata->email);
	ch->player_id					= oc->player_id;
	ch->race						= oc->race;
	ch->clss						= oc->clss;
	ch->act 						= PLR_NOSUMMON;
	ch->config 						= CONFIG_NOCHARM;
	ch->comm						= COMM_COMBINE | COMM_PROMPT;
	ch->prompt						= str_dup(oc->prompt);
	ch->mounted_on					=NULL;
	ch->ridden_by					=NULL;
	ch->tethered					=false;
	ch->bucking 					=false;
	ch->wildness					=100;
	ch->will						=100;
	ch->pcdata->confirm_delete		= false;
	ch->pcdata->pwd 				= str_dup(oc->pcdata->pwd);
	ch->pcdata->bamfin				= str_dup( "" );
	ch->pcdata->bamfout 			= str_dup( "" );
	ch->pcdata->colourmode			= oc->pcdata->colourmode;
	for (stat =0; stat < MAX_STATS; stat++){
		ch->perm_stats[stat]		= 1;
	}
	ch->pcdata->condition[COND_THIRST]	= 48;
	ch->pcdata->condition[COND_FULL]	= 48;
	ch->pcdata->condition[COND_HUNGER]	= 48;
	ch->pcdata->security	= 0;
	ch->remort=oc->beginning_remort; 
	ch->lines=oc->lines;

	ch->clan=oc->clan;
	ch->clanrank=oc->clanrank;
	if(IS_LETGAINED(oc)){
		SET_BIT(ch->act,PLR_CAN_ADVANCE);
	}

	// swap over the connection to the new character
	ch->desc						= oc->desc;
	oc->desc = NULL;
	extract_char(oc, true);

	ch->desc->character				= ch;
	ch->desc->creation_remort_number=ch->remort;

	ch->desc->connected_state=CON_GET_NEW_RACE;

	ch->print("Please select your race from one of the following:`1 ");
	int count=0;
	for ( int race = 1; race_table[race]; race++ )
	{
		// creation selectable pc races only
		if (!race_table[race]->creation_selectable()
			|| (race_table[race]->remort_number > ch->desc->creation_remort_number)){
			continue;
		}
		ch->printf( " `S[`Y%12.12s`S]", race_table[race]->name);
		if (++count%5==0){
			ch->print( "`x\r\n ");
		}
	}
	ch->printf( "`1Type in the name of the race you wish to play now:`1");

}


/**************************************************************************/
// remort <player_in_room>
void do_remort(char_data *ch, char *argument)
{
	char name[MIL];

	if(!GAMESETTING(GAMESET_REMORT_SUPPORTED)){
		ch->println("Game settings currently have remort support disabled.");
		ch->println("This can be turned on using the gameedit command.");
		if(IS_NPC(ch)){
			mpbugf("Remort command used - nothing happened because remort is disabled in the game settings.");
		}
		return;
	}
	
	argument=one_argument(argument, name);
	if(IS_NULLSTR(name)){
		ch->println("syntax: remort <playername>");
		return;
	}

	char_data*v=get_char_room(ch, name);
	
	if(v==NULL){
		ch->printlnf("Couldn't find player '%s' in current room to remort.", name);
		return;
	}

	// found the player, if pass idiot checks remort them
	if(IS_NPC(v) || v->level!=LEVEL_HERO){
		ch->println("Can only remort mortal heros.");
		return;
	}

    // prevent players from connecting a character that is 
 	// beginning remort to get money 
	connection_data *d,*c_next;
    for (d = connection_list; d != NULL; d = c_next)
    {
        c_next = d->next;
		if (d!=v->desc && CH(d) && CH(d)->player_id == v->player_id )
        {
			logf("do_remort(): Kicking out an extra connection!.\r\n");
            extract_char(CH(d),true);
            connection_close(d);
        } 
    }

	// *** begin remorting them
	// create backup pfile
	char backname[MIL];
	sprintf(backname, "%s%d", pfilename(v->name, PFILE_REMORT_BACKUP), v->remort);	
	save_char_obj_to_filename(v,backname);	
	
	// flag the remort and update the pfile, so when they reconnect 
	// they start creation in the new remort
	v->beginning_remort=v->remort+1;
	save_char_obj(v);


	char subject[MIL];
	sprintf(subject,"%s beginning remort %d", v->name, v->beginning_remort);
	char text[MIL];
	sprintf(text,"%s beginning remort %d`1Pfile of player backed up to %s before remort begun.`1"
		"Sent on remort by '%s' %d.", 
		v->name, v->beginning_remort, 
		backname,
		ch->name, ch->pIndexData?ch->pIndexData->vnum:0);

	autonote(NOTE_INOTE, "remort system", subject, "imm", text, true);
	
	begin_remort(v);
	ch->println(text);
}
/**************************************************************************/
