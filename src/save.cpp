/**************************************************************************/
// save.cpp - save a character etc
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

#include "areas.h"
#include "hreboot.h"
#include "cust_col.h"
#include "channels.h"
#include "pload.h"

extern OBJ_DATA *obj_free;
bool dont_nest=false;

int who_format_lookup(char *argument);
const char *who_format_name(int index);

char *fwrite_flag( long flags, char buf[] );
void fwrite_custom_colours(FILE* fp, const char *header, const char custom_colours[]);
char *fread_custom_colours(FILE* fp, bool player);

// Local functions.
void    fwrite_char     args( ( char_data *ch,  FILE *fp ) );
void    fwrite_obj      args( ( OBJ_DATA  *obj,	FILE *fp, int iNest, char *heading) );
void    fwrite_pet      args( ( char_data *pet, FILE *fp) );
void    fread_char      args( ( char_data *ch,  FILE *fp ) );
void    fread_pet       args( ( char_data *ch,  FILE *fp ) );
obj_data * fread_obj    ( FILE *fp, const char *filename );

/**************************************************************************/
// return the filename portion of pfile - lowercase and with the .plr extension
char * pfile_filename(char *name)
{
	static int i;
	static char rbuf[5][MSL];
	char buf[MSL], first_name[MSL];
	
	// rotate buffers
	++i= i%5;

	strcpy(buf, name);
	
	// make lowercase and first word of the name
	one_argument( buf, first_name);

	sprintf(rbuf[i], "%s.plr", first_name);

	return(rbuf[i]);
}
/**************************************************************************/
// return the name including directory was 
// a pfile would be called.
char * pfilename(char *name, PFILE_TYPE pt)
{
	static int i;
	static char rbuf[5][MSL];
	char filename[MSL];
	
	// rotate buffers
	++i= i%5;

	strcpy(filename, pfile_filename(name));

	switch(pt){
	case PFILE_LOCKED:
		sprintf(rbuf[i], PDIR_LOCKED "%s", filename);
		break;
	case PFILE_NORMAL:
#if defined(NO_INITIAL_ALPHA_PFILEDIRS) || defined(WIN32) 
		sprintf(rbuf[i], PLAYER_DIR "%s", filename);
#else
		sprintf(rbuf[i], PLAYER_DIR "%c" DIR_SYM "%s", filename[0], filename);
#endif
		break;
	case PFILE_BUILDER:
		sprintf(rbuf[i], PDIR_BUILDER"%s", filename);
		break;
	case PFILE_TRUSTED:
		sprintf(rbuf[i], PDIR_TRUSTED"%s", filename);
		break;
	case PFILE_IMMORTAL:
		sprintf(rbuf[i], PDIR_IMMORTAL"%s", filename);
		break;
	case PFILE_REMORT_BACKUP:
		sprintf(rbuf[i], REMORT_DIR "%s", filename);
		break;
	default:
		sprintf(rbuf[i], "ERROR_PFILENAME_INCORRECT_PTYPE, name = %s,pt=%d",
			filename, (int)pt);
		bugf("ERROR_PFILENAME_INCORRECT_PTYPE %d - name=%s", (int)pt, filename);
	}
	return(rbuf[i]);
}
/**************************************************************************/
PFILE_TYPE get_pfiletype(char_data *ch)
{
	if(ch->level>=LEVEL_IMMORTAL){
		return PFILE_IMMORTAL;
	}
	if(IS_TRUSTED(ch, LEVEL_IMMORTAL)){
		return PFILE_TRUSTED;
	}
	if(HAS_SECURITY(ch,1)){
		return PFILE_BUILDER;
	}

	if(ch->pcdata && 
		!IS_NULLSTR(ch->pcdata->unlock_id) 
		&& str_len(ch->pcdata->unlock_id)==6){
		return PFILE_LOCKED;
	}
	return PFILE_NORMAL;
}
/**************************************************************************/
// returns the type 
PFILE_TYPE find_pfiletype(const char *name)
{
	char buf[MIL],first_name[MIL];
	int count=0;
	PFILE_TYPE result=PFILE_NONE;
	
	strcpy(buf, name);	
	// make lowercase and first word of the name
	one_argument( buf, first_name);

	// look in all the locations for the pfile
	if(file_existsf(PDIR_LOCKED "%s.plr", first_name)){
		result=PFILE_LOCKED;
		count++;
	}
#if defined(NO_INITIAL_ALPHA_PFILEDIRS) || defined(WIN32) 
	if(file_existsf(PLAYER_DIR "%s.plr", first_name)){
#else
	if(file_existsf(PLAYER_DIR "%c" DIR_SYM "%s.plr", first_name[0], first_name)){
#endif
		result=PFILE_NORMAL;
		count++;
	}
	if(file_existsf(PDIR_BUILDER"%s.plr", first_name)){
		result=PFILE_BUILDER;
		count++;
	}
	if(file_existsf(PDIR_TRUSTED"%s.plr", first_name)){
		result=PFILE_TRUSTED;
		count++;
	}
	if(file_existsf(PDIR_IMMORTAL"%s.plr", first_name)){
		result=PFILE_IMMORTAL;
		count++;
	}

	// return the results
	if (count==0){
		return PFILE_NONE;
	}
	if (count>1){
		bugf("find_pfiletype(): MULTIPLE LOCATIONS FOR %s", first_name);
		return PFILE_MULTIPLE;
	}
	return result;
}
/**************************************************************************/
void save_char_obj_to_filename( char_data *ch, char *filename )
{
    char strsave[MIL];
	strcpy(strsave,filename);
    FILE *fp;

	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD))
	{
		logf( "Character %s not saved cause on dedicated pkill style mud", ch->name);
		return;    
    }
    fclose( fpReserve );
    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
        bugf("save_char_obj_to_filename(): fopen '%s' for write - error %d (%s)",
			TEMP_FILE, errno, strerror( errno));
		ch->println("`RNOTICE: YOUR PFILE WASN'T SAVED, AS IT WASN'T POSSIBLE "
			"TO A TEMPORARY FILE FOR SAVING!`x");
    }
    else
    {
		fwrite_char( ch, fp );
		if ( ch->carrying != NULL ){
			fwrite_obj( ch->carrying, fp, 0, "O");
		}

		// save the pets
		if (ch->pet && IS_NPC(ch->pet)){
			fwrite_pet(ch->pet,fp);
			// save what a pet is carrying
			if ( ch->pet->carrying != NULL ){
				fwrite_obj( ch->pet->carrying, fp, 0, "PO" ); // pet objects
			}
		}

		
		int bytes_for_end_written=fprintf( fp, "#END\n" );
		int fclose_result=fclose( fp );

		bool save_error=false;
		if(bytes_for_end_written>4 && fclose_result==0){
#ifdef WIN32
			unlink(strsave);
#endif
			if(rename(TEMP_FILE,strsave)!=0){
				logf("Error in rename while saving %s pfile.  (Attempted to rename '%s' to '%s')",
					ch->name,
					TEMP_FILE,
					strsave);
				bugf("save_char_obj_to_filename(): rename(%s, %s) - error %d (%s)",
					TEMP_FILE, strsave, errno, strerror( errno));
				save_error=true;
			};
		}else{
			save_error=true;
			logf("save_char_obj_to_filename(): bytes_for_end_written=%d, fclose_result=%d",
				bytes_for_end_written, fclose_result);
			bugf("save_char_obj( ): bytes_for_end_written<5!!!\n%s not saved!!! - probably out of diskspace!!!"
				"\nRecommend clearing some diskspace!\n", ch->name);
		}

		if(save_error){
			ch->println("`RYOUR PFILE WAS NOT SAVED CORRECTLY - POSSIBLY DUE TO LACK OF DISKSPACE REASONS\r\n"
				"(The Immortals have been notified)");
			ch->println("`xThis system has been implemented so your pfile wont be corrupted... your pfile will remain as it was when you logged in earlier.");
			{
				connection_data *d;
				
				for ( d = connection_list; d != NULL; d = d->next )
				{
					if ( d->connected_state == CON_PLAYING &&
						IS_IMMORTAL(d->character))
					{
						CH(d)->print("\7\7\7");
						CH(d)->println("You are being paged by dawn it self!!!");
						CH(d)->print("\a");
						CH(d)->printlnf("`R'%s's pfile was not saved to '%s' for some reason!!!"
							"possibly cause is lack of diskspace, diskspace quota  or rename problems - check game logs!`x\r\n", ch->name, strsave);
						CH(d)->print("\a");
						CH(d)->printlnf("`R'%s's pfile was not saved to '%s' for some reason!!!"
							"possibly cause is lack of diskspace, diskspace quota  or rename problems - check game logs!`x\r\n", ch->name, strsave);
						CH(d)->print("\a");
						CH(d)->printlnf("`R'%s's pfile was not saved to '%s' for some reason!!!"
							"possibly cause is lack of diskspace, diskspace quota or rename problems - check game logs!`x\r\n", ch->name, strsave);
						CH(d)->print("\a");
							CH(d)->println(get_piperesult("df"));
						CH(d)->print("\a");
							CH(d)->println(get_piperesult("quota"));
						CH(d)->println("Clear some diskspace or fix the problem!!!");
					}
				}
			}			 			
		}
    }

    fpReserve = fopen( NULL_FILE, "r" );
}

/**************************************************************************/
// Save a character and inventory.
// Would be cool to save NPC's too for quest purposes,
// some of the infrastructure is provided.
void save_char_obj( char_data *ch )
{
    if ( IS_NPC(ch) )
	return;

	if(ch->pload && ch->pload->dont_save){
		if(ch->pload->loaded_by){
			ch->pload->loaded_by->printlnf( "Character %s not autosaved cause it was ploaded.", ch->name);
		}			
		logf( "Character %s not saved cause it was ploaded.", ch->name);
		return;
	}

	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){	
    	logf( "Character %s not saved cause on dedicated pkill style mud", ch->name);
		return;    
    }

	if ( ch->desc != NULL && ch->desc->original != NULL ){
		ch = ch->desc->original;
	}

	PFILE_TYPE pt=get_pfiletype(ch);
	ch->pcdata->pfiletype=pt;

	save_char_obj_to_filename(ch, pfilename(ch->name, pt));
    return;
}

/**************************************************************************/
// Write the char.
void fwrite_char( char_data *ch, FILE *fp )
{
	int sn, gn, pos;

	// update the version info
	ch->version=9;
	ch->subversion =2;


    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER" );

	fprintf( fp, "Vers  %d\n", ch->version);
    fprintf( fp, "SubV  %d\n", ch->subversion);
	fprintf( fp, "Name  %s~\n", fix_string(ch->name));

	fprintf( fp, "Level %d\n",	ch->level);
	fprintf( fp, "Trust %d\n",	ch->trust);

    { // ID - created time
		char time_buf[50];
		sprintf( time_buf,"%s", (char *) ctime((time_t*)&ch->player_id));
		time_buf[str_len(time_buf)-1]='\0';
		fprintf( fp, "Id    %ld (%s)\n", ch->player_id, time_buf);
	}

    { // Logout time, if the character was ploaded use the loaded time
		time_t t;
		if(ch->pload){
			assertp(ch->pcdata);
			t=ch->pcdata->last_logout_time;
		}else{
			t=current_time;
		}

		char time_buf[50];
		sprintf( time_buf,"%s", (char *) ctime( &t));
		time_buf[str_len(time_buf)-1]='\0';
		fprintf( fp, "LogO  %ld (%s)\n", (long) t, time_buf);
	}

    if (!IS_NPC(ch))
	{ 
		// Logout site
		if (ch->desc){
			fprintf( fp, "LogOS %s~\n", fix_string(ch->desc->remote_hostname));
		}else{
			fprintf( fp, "LogOS %s~\n", fix_string(ch->pcdata->last_logout_site));
		}

		// colour codes used within the pfile
		if(ch->pcdata->colour_code){
			fprintf( fp, "CC    %c\n",	ch->pcdata->colour_code);
		}

		if(ch->colour_prefix && ch->colour_prefix!=COLOURCODE){
			fprintf( fp, "CPref %c\n",	ch->colour_prefix);
		}
	}

    { // Time played 
		int played=ch->played;
		if(!ch->pload){ // only add the additional time if they were not ploaded
			played+=(int)(current_time - ch->logon);
		}
		fprintf( fp, "Plyd  %-9d (%s)\n", played, short_timediff(0, played));
	}

	if (!IS_NPC(ch)){
		fprintf( fp, "Pass  %s~\n", fix_string(ch->pcdata->pwd));

		fprintf( fp, "Remrt %d\n", ch->remort);
		if(ch->beginning_remort){
			fprintf( fp, "beginning_remort %d\n", ch->beginning_remort);	
		}
	}

	if(!IS_NULLSTR(ch->pcdata->email)){
		fprintf( fp, "Email %s~\n", fix_string(ch->pcdata->email));
	}
	if(!IS_NULLSTR(ch->pcdata->created_from)){
		fprintf( fp, "Created_from %s~\n", fix_string(ch->pcdata->created_from));
	}	 
	if(!IS_NULLSTR(ch->pcdata->unlock_id)){
		fprintf( fp, "Unlock_id %s~\n", fix_string(ch->pcdata->unlock_id));
	}

	if(!IS_NPC(ch) && ch->know_index){
		fprintf( fp, "Know_index %d\n", ch->know_index);
	}

	if(!IS_NPC(ch) && !IS_NULLSTR(ch->pcdata->webpass)){
		fprintf( fp, "WebPass  %s~\n", fix_string(ch->pcdata->webpass));
	}

	if(!IS_NPC(ch) && ch->pcdata->birthdate){
		fprintf( fp, "Bdate %d\n", (int) ch->pcdata->birthdate);
	}
	if(!IS_NPC(ch) && ch->pcdata->birthyear_modifier){
		fprintf( fp, "BYearMod %d\n", (int) ch->pcdata->birthyear_modifier);
	}
	
    if (!IS_NULLSTR(ch->short_descr)){
		fprintf( fp, "ShD   %s~\n",  fix_string(ch->short_descr) );
	}

	if( !IS_NULLSTR(ch->long_descr)){
		fprintf( fp, "LnD   %s~\n",  fix_string(ch->long_descr)  );
	}

	if (!IS_NULLSTR(ch->description)){
		fprintf( fp, "Desc  %s~\n", fix_string(ch->description));
	}

	if(ch->pcdata && !IS_NULLSTR(ch->pcdata->history)){
 		fprintf( fp, "History  %s~\n", fix_string(ch->pcdata->history));
	}

    if (!IS_NULLSTR(ch->gprompt)){
		fprintf( fp, "GPro  %s~\n", fix_string(ch->gprompt));
	}

    if (!IS_NULLSTR(ch->prompt) && str_cmp(ch->prompt, game_settings->default_prompt)){
		fprintf( fp, "Prom  %s~\n", fix_string(ch->prompt));
	}

    if (!IS_NULLSTR(ch->olcprompt)){
		fprintf( fp, "OPrmt %s~\n", fix_string(ch->olcprompt));
	}

	if (!IS_NPC(ch) && !IS_NULLSTR(ch->pcdata->battlelag)){
		fprintf( fp, "BattleLag %s~\n", fix_string(ch->pcdata->battlelag));
	}

	if (!IS_NPC(ch) && ch->pcdata->mpedit_autoindent){
		fprintf( fp, "Autoindent %d~\n", ch->pcdata->mpedit_autoindent);
	}

	fprintf( fp, "Cla   %s~\n",	class_table[ch->clss].name);

	if(ch->pcdata){
		fprintf( fp, "OldCla	%s~\n",	class_table[ch->pcdata->old_clss].name);
	}

	fprintf( fp, "Race  %s~\n", race_table[ch->race]->name );
	// language
	fprintf( fp, "Lang  %s~\n", ch->language->name);
   
	if (!IS_NPC(ch))
	{
		if(!IS_NULLSTR(ch->pcdata->immtalk_name)){
			fprintf( fp, "ITN   %s~\n",	fix_string(ch->pcdata->immtalk_name));
		}
		
		fprintf( fp, "SLvl  %d\n", 	ch->pcdata->sublevel);
		fprintf( fp, "SubP  %d\n", 	ch->pcdata->sublevel_pracs);
		fprintf( fp, "SubT  %d\n", 	ch->pcdata->sublevel_trains);
		
		// reduced XP and RPS systems
		fprintf( fp, "RXP   %d\n", ch->pcdata->reduce_xp_percent);
		fprintf( fp, "RRPS  %d\n", ch->pcdata->reduce_rps_percent);
		
		fprintf( fp, "Sec   %d\n",		ch->pcdata->security );
		
		fprintf( fp, "Not   %ld %ld %ld %ld %ld\n",
			(long) ch->pcdata->last_note, (long) ch->pcdata->last_idea,
			(long) ch->pcdata->last_penalty, (long) ch->pcdata->last_news,
			(long) ch->pcdata->last_changes  );
		fprintf( fp, "ANote %ld\n", (long) ch->pcdata->last_anote);
		fprintf( fp, "INote %ld\n", (long) ch->pcdata->last_inote);
		fprintf( fp, "MiscN %ld\n", (long) ch->pcdata->last_misc);
		fprintf( fp, "SNote %ld\n", (long) ch->pcdata->last_snote);
		fprintf( fp, "PKNote %ld\n",(long) ch->pcdata->last_pknote);

		if(IS_THIEF(ch) && GAMESETTING3(GAMESET3_THIEF_SYSTEM_ENABLED)){
			fprintf( fp, "Thief_until %ld\n",(long) ch->pcdata->thief_until);
		}
		if(IS_KILLER(ch) && GAMESETTING3(GAMESET3_KILLER_SYSTEM_ENABLED)){
			fprintf( fp, "Killer_until %ld\n",(long) ch->pcdata->killer_until);
		}

		// save their note posting times if necessary
		if(!IS_TRUSTED(ch, NOTE_POST_RESTRICTIONS_BELOW_LEVEL)){
			for(int i=0; i<MAX_NOTE_POST_TIME_INDEX; i++){
				if(ch->pcdata->note_post_time[i]>0){
					fprintf( fp, "NotePost %ld\n", (long)ch->pcdata->note_post_time[i]);
				}
			}
		}	
		if(ch->pcdata->preference_msp!=PREF_AUTOSENSE){
			fwrite_wordflag( preference_types, ch->pcdata->preference_msp, "PrefMSP ", fp);
		}
		if(ch->pcdata->preference_mxp!=PREF_AUTOSENSE){
			fwrite_wordflag( preference_types, ch->pcdata->preference_mxp, "PrefMXP ", fp);
		}
		if(ch->pcdata->preference_dawnftp!=PREF_AUTOSENSE){
			fwrite_wordflag( preference_types, ch->pcdata->preference_dawnftp, "PrefDAWNFTP ", fp);
		}
		if(ch->pcdata->preference_colour_in_socials!=PREF_AUTOSENSE){
			fwrite_wordflag( preference_types, ch->pcdata->preference_colour_in_socials, "PrefColourInSocials ", fp);
		}

	}

	
	if(ch->pcdata->charnotes){
		fprintf( fp, "Cnotes %s~\n", fix_string(ch->pcdata->charnotes));
	}

	// clan stuff
	if (ch->clan)
	{
		fprintf( fp, "Clan  %s~\n",ch->clan->savename());
		fprintf( fp, "Rank  %s~\n",ch->clan->clan_rank_title(ch->clanrank));
	}
	

	// PKILLS info
	if (ch->pkool)
		fprintf( fp, "PKool %d\n", ch->pkool );
	if (ch->pksafe)
		fprintf( fp, "PKsaf %d\n", ch->pksafe );
	if (ch->pkkills)
		fprintf( fp, "PKill %d\n", ch->pkkills );
	if (ch->pkdefeats)
		fprintf( fp, "PKDef %d\n", ch->pkdefeats );

	// MKILLS info and unsafe due to stealing stuff
	if(!IS_NPC(ch))
	{
		if (ch->pcdata->mkills)
			fprintf( fp, "MKil  %d\n", ch->pcdata->mkills);
		if (ch->pcdata->mdefeats)
			fprintf( fp, "MDef  %d\n", ch->pcdata->mdefeats);

		if (ch->pcdata->unsafe_due_to_stealing_till)
			fprintf( fp, "Unsafe_4stealing  %d\n", (int)ch->pcdata->unsafe_due_to_stealing_till);
	}

	 
	if(!IS_NPC(ch))
	{
		fprintf( fp, "Rps   %ld\n", ch->pcdata->rp_points);

		if(ch->pcdata->xp_penalty)
			fprintf( fp, "Xpen %d\n", ch->pcdata->xp_penalty );

		// noble stuff
		if(ch->pcdata->diplomacy)
		{
			fprintf(fp, "Diplo %d\n", ch->pcdata->diplomacy );
			if(ch->pcdata->dip_points){
				fprintf(fp, "Dptn  %d\n", ch->pcdata->dip_points);
			}
			if(ch->pcdata->autovote){
				fprintf(fp, "AVote %d\n", ch->pcdata->autovote);
			}
		}

		// auto afk after
		if(ch->pcdata->autoafkafter){
			fprintf(fp, "AAA   %d\n", ch->pcdata->autoafkafter);
		}

		if(ch->pcdata->rp_count)
			fprintf(fp, "Rpcnt %d\n", ch->pcdata->rp_count);

		if(ch->pcdata->merit) 
			fprintf(fp, "Merit %d\n", ch->pcdata->merit);
		 
		// Questpoints system 
		if(ch->pcdata->qpoints){
            fprintf( fp, "Quest %d\n", ch->pcdata->qpoints );
		}
		 
		// karn system
		fprintf( fp, "Karns %d\n", ch->pcdata->karns);
		fprintf( fp, "KCntd %d\n", ch->pcdata->next_karn_countdown);
		fprintf( fp, "HeroXp %d\n", ch->pcdata->heroxp );
		if(ch->pcdata->hero_level_count){
			fprintf( fp, "HeroLevelCount %d\n", ch->pcdata->hero_level_count );
		}

		// reroll counter
		fprintf( fp, "RCnt  %d\n", ch->pcdata->reroll_counter);
	}
	fprintf( fp, "RecallRoom  %d\n", ch->recall_room );

	// Inn recall variables.
	fprintf( fp, "RecallInnRoom  %d\n", ch->recall_inn_room );
	fprintf( fp, "ExpireRecallInn  %d\n", ch->expire_recall_inn );
	
	if(ch->bleeding)
		fprintf( fp, "Bleed %d\n", ch->bleeding);
	if(ch->will_die)
		fprintf( fp, "Wdie  %d\n", ch->will_die);
	if(ch->is_stunned)
		fprintf( fp, "Stun  %d\n", ch->is_stunned);

	// fix up sex before writing it - rare to stomp over something
	// generally only if someone has 2 change sex'es on them
	if(ch->sex<SEX_NEUTRAL || ch->sex>SEX_FEMALE){
		if (IS_NPC(ch)){
			ch->sex=SEX_NEUTRAL;
		}else{
			ch->sex=ch->pcdata->true_sex;
		}
	};

	fprintf( fp, "Sex   %s~\n",	sex_table[ch->sex].name);
	if (!IS_NPC(ch)){
		fprintf( fp, "TSex  %s~\n", sex_table[ch->pcdata->true_sex].name);
		if(ch->host_validated){
			fprintf( fp, "HostV %s~\n", fix_string(ch->desc->remote_hostname));
		}
	}
	fprintf( fp, "Pos   %s~\n", 
		ch->position == POS_FIGHTING ? position_table[POS_STANDING ].name:
			position_table[ch->position].name);
	
	fprintf( fp, "Scro  %d\n",ch->lines);
	fprintf( fp, "Room  %d\n",
		(ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
			&& ch->was_in_room != NULL )
				? ch->was_in_room->vnum
				: ch->in_room == NULL ? ROOM_VNUM_LIMBO: ch->in_room->vnum );

	fprintf( fp, "LIcRm %d\n", ch->last_ic_room?
		ch->last_ic_room->vnum:0);

	if(ch->saycolour!='x'){
		fprintf( fp, "Saycol %c\n", ch->saycolour);
	}

	if ( !IS_NPC( ch )){
		if ( !IS_NULLSTR( ch->pcdata->surname )){
			fprintf( fp, "Surname    %s~\n", fix_string(ch->pcdata->surname) );
		}
		if ( !IS_NULLSTR( ch->pcdata->crest )){
			fprintf( fp, "Crest      %s~\n", fix_string(ch->pcdata->crest) );
		}
		if ( !IS_NULLSTR( ch->pcdata->birthplace )){
			fprintf( fp, "Birthplace %s~\n", fix_string(ch->pcdata->birthplace) );
		}
	}

	fprintf( fp, "HMV   %d %d %d %d %d %d\n",
		ch->hit, ch->max_hit, ch->mana, 
		ch->max_mana, ch->move, ch->max_move );
	if (!IS_NPC(ch)){		
		fprintf( fp, "HMVP  %d %d %d\n", ch->pcdata->perm_hit, 
								ch->pcdata->perm_mana,
								ch->pcdata->perm_move);
		fprintf( fp, "Cnd   %d %d %d %d\n",
			ch->pcdata->condition[0],
			ch->pcdata->condition[1],
			ch->pcdata->condition[2],
			ch->pcdata->condition[3] );
	}


	// money
	fprintf( fp, "Gold  %ld\n",ch->gold);
	fprintf( fp, "Silv  %ld\n",ch->silver);
	if(ch->bank){
		fprintf( fp, "Bank  %ld\n", ch->bank);
	}

	fprintf( fp, "Exp   %d\n",ch->exp);

    if ( IS_NPC(ch) )
	{
		fprintf( fp, "Vnum %d\n", ch->pIndexData->vnum);
    }
	else
	{
		if (!IS_NULLSTR(ch->pcdata->bamfin)){
			 fprintf( fp, "Bin   %s~\n", fix_string(ch->pcdata->bamfin));
		}
		if (!IS_NULLSTR(ch->pcdata->bamfout)){
			fprintf( fp, "Bout  %s~\n",  fix_string(ch->pcdata->bamfout));
		}

        if (!IS_NULLSTR(ch->pcdata->fadein)){
            fprintf( fp, "FadeIN   %s~\n", fix_string(ch->pcdata->fadein));
		}
        if (!IS_NULLSTR(ch->pcdata->fadeout)){
            fprintf( fp, "FadeOUT   %s~\n", fix_string(ch->pcdata->fadeout));
		}

		fprintf( fp, "Pnts  %d\n",       ch->pcdata->points      );
		fprintf( fp, "LLev  %d\n",       ch->pcdata->last_level  );
	}
		
	if (ch->invis_level)
		fprintf( fp, "Invi  %d\n",       ch->invis_level );
	if (ch->iwizi)
		fprintf( fp, "IWizi %d\n",       ch->iwizi);
	if (ch->olcwizi)
		fprintf( fp, "OlcWizi %d\n",     ch->olcwizi );
	if (ch->owizi)
		fprintf( fp, "OWizi %d\n",       ch->owizi );

	if(ch->temple)
		fprintf(fp,"Temple %d\n", ch->temple);
	if (ch->incog_level)
		fprintf(fp,"Inco %d\n",ch->incog_level);
	
	if (ch->practice)
		fprintf( fp, "Prac  %d\n", ch->practice);
	if (ch->train)
		fprintf( fp, "Trai  %d\n", ch->train);
	
	fprintf( fp, "Tired %d\n", ch->pcdata->tired );
	fprintf( fp, "Tryin %d\n", ch->is_trying_sleep );
	if(ch->saving_throw){
		fprintf( fp, "Save  %d\n", ch->saving_throw);
	}

	fprintf( fp, "Alig  %d %d\n", ch->tendency, ch->alliance);
	
	if (ch->hitroll)
		fprintf( fp, "Hit   %d\n", ch->hitroll);
	if (ch->damroll)
		fprintf( fp, "Dam   %d\n", ch->damroll);
	fprintf( fp, "ACs   %3d %3d %3d %3d\n",
		ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
	if (ch->wimpy)
		fprintf( fp, "Wimp  %d\n", ch->wimpy);

	if(!IS_NPC(ch) && ch->pcdata->panic){
		fprintf( fp, "Panic %d\n", ch->pcdata->panic);
	}

	if ( !IS_NPC( ch ))
	{
		if ( !IS_NULLSTR( ch->pcdata->haircolour )){
			fprintf( fp, "HairCol   %s~\n", fix_string(ch->pcdata->haircolour) );
		}
		if ( !IS_NULLSTR( ch->pcdata->eyecolour )){
			fprintf( fp, "EyeCol   %s~\n",  fix_string(ch->pcdata->eyecolour) );
		}
		fprintf( fp, "Height  %d\n",			ch->pcdata->height );
		fprintf( fp, "Weight  %d\n",			ch->pcdata->weight );

		long nVanish = UMAX(0,(int)(current_time - ch->pcdata->next_vanish));
		fprintf ( fp, "NVanish %ld\n",			nVanish);

		long nWorshipTime = UMIN(1000000,(int)(current_time-ch->pcdata->worship_time));
		fprintf ( fp, "NWorshipTime %ld\n",		nWorshipTime);
	
		// Addition for Deity
		if(ch->deity)
		{
			fprintf ( fp, "DeityName   %s~\n", fix_string(ch->deity->name) );
		}
	}
    // stats
	fprintf( fp, "Attr %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
		ch->perm_stats[STAT_ST],
		ch->perm_stats[STAT_QU],
		ch->perm_stats[STAT_PR],
		ch->perm_stats[STAT_EM],
		ch->perm_stats[STAT_IN],
		ch->perm_stats[STAT_CO],
		ch->perm_stats[STAT_AG],
		ch->perm_stats[STAT_SD],
		ch->perm_stats[STAT_ME],
		ch->perm_stats[STAT_RE]);
	fprintf (fp, "AMod %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
		ch->modifiers[STAT_ST],
		ch->modifiers[STAT_QU],
		ch->modifiers[STAT_PR],
		ch->modifiers[STAT_EM],
		ch->modifiers[STAT_IN],
		ch->modifiers[STAT_CO],
		ch->modifiers[STAT_AG],
		ch->modifiers[STAT_SD],
		ch->modifiers[STAT_ME],
		ch->modifiers[STAT_RE]);
	fprintf (fp, "APot %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
		ch->potential_stats[STAT_ST],
		ch->potential_stats[STAT_QU],
		ch->potential_stats[STAT_PR],
		ch->potential_stats[STAT_EM],
		ch->potential_stats[STAT_IN],
		ch->potential_stats[STAT_CO],
		ch->potential_stats[STAT_AG],
		ch->potential_stats[STAT_SD],
		ch->potential_stats[STAT_ME],
		ch->potential_stats[STAT_RE]);

	if (ch->act)
	{
		if (IS_NPC(ch)){ // this will need changing if mobs are ever saved 
			fwrite_wordflag( act_flags, ch->act, "MAct", fp);
		}else{
			fwrite_wordflag( plr_flags, ch->act, "Act",fp);
		}
	}

	//  ???? Just in case???
	if (ch->act2)
	{
		if (IS_NPC(ch)){ // this will need changing if mobs are ever saved 
			fwrite_wordflag( act2_flags, ch->act2, "MAct2", fp);
		}else{
			fwrite_wordflag( act2_flags, ch->act2, "Act2",fp);
		}
	}
	
	if (ch->affected_by){
		fwrite_wordflag( affect_flags, ch->affected_by, "AfBy", fp);
	}

	if (ch->affected_by2){
		fwrite_wordflag( affect2_flags, ch->affected_by2, "AfBy2", fp);
	}

	if (ch->affected_by3){
		fwrite_wordflag( affect3_flags, ch->affected_by3, "AfBy3", fp);
	}

	fwrite_wordflag( comm_flags, ch->comm, "Comm", fp);


	if (ch->wiznet[0]){
		fwrite_wordflag( wiznet_flags, ch->wiznet[0], "Immwiznet", fp);
	}
	if (ch->wiznet[1]){
		fwrite_wordflag( wiznet_flags, ch->wiznet[1], "Wiznet", fp);
	}
	if (ch->wiznet[2]){
		fwrite_wordflag( wiznet_flags, ch->wiznet[2], "Wiznet2", fp);
	}
	if (ch->wiznet[3]){
		fwrite_wordflag( wiznet_flags, ch->wiznet[3], "Wiznet3", fp);
	}
	for(int tempi=0; tempi<4; tempi++){
		if(!IS_NULLSTR(ch->wiznet_colour[tempi])){
			fprintf( fp, "Wiznet%dc %s~\n", tempi, fix_string(ch->wiznet_colour[tempi]));
		}
	}

	if (ch->config){
		fwrite_wordflag( config_flags, ch->config, "Config", fp);
	}

	if (ch->config2){
		fwrite_wordflag( config2_flags, ch->config2, "Config2", fp);
	}

	if (ch->pcdata->pconfig){
		fwrite_wordflag( pconfig_flags, ch->pcdata->pconfig, "PConfig", fp);
	}

	if (ch->notenet){
	 	fwrite_wordflag( notenet_flags, ch->notenet, "NoteNet", fp);
	}

	// save the players custom colours and scheme
	if(ch->pcdata){
		if(ch->pcdata->custom_colour_scheme){
			fprintf(fp, "Colour_scheme %s~\n", fix_string(ch->pcdata->custom_colour_scheme->template_name));
		}
		if(ch->pcdata->custom_colour){
			fwrite_custom_colours(fp, "Custom_colours", ch->pcdata->custom_colour);
		}
		if(ch->pcdata->flashing_disabled){
			fprintf(fp, "flashing_disabled 1~\n");
		}

		fwrite_wordflag( colourmode_types, ch->pcdata->colourmode, 
			"Colourmode",fp);

		if(ch->pcdata->strip_colour_on_channel){
			fprintf(fp, "Strip_channel_colours %s~\n", 
				channel_convert_bitflags_to_text(ch->pcdata->strip_colour_on_channel));
		}

		if(ch->pcdata->channeloff){
			fprintf(fp, "channeloff %s~\n", 
				channel_convert_bitflags_to_text(ch->pcdata->channeloff));
		}
	}

	if (!IS_NPC(ch))
	{
		// write duel stats
		fprintf( fp, "Duel_challenged %d\n",	ch->duel_challenged);
		fprintf( fp, "Duel_decline %d\n",		ch->duel_decline);
		fprintf( fp, "Duel_accept %d\n",		ch->duel_accept);
		fprintf( fp, "Duel_ignore %d\n",		ch->duel_ignore);
		fprintf( fp, "Duel_bypass %d\n",		ch->duel_bypass);
		fprintf( fp, "Duel_subdues_before_karn_loss %d\n",	ch->duel_subdues_before_karn_loss);

		// write aliases
		for (pos = 0; pos < MAX_ALIAS; pos++)
		{
			if (IS_NULLSTR(ch->pcdata->alias[pos])
				||  IS_NULLSTR(ch->pcdata->alias_sub[pos]))
			break;

			fprintf(fp,"Alias %s %s~\n", fix_string(ch->pcdata->alias[pos]),
				fix_string(ch->pcdata->alias_sub[pos]));
		}

		// write traits - of someone has them
		for (pos=0; pos < 9; pos++ )
		{
			if (ch->pcdata->trait[pos]){
				if(str_cmp(ch->pcdata->trait[pos]," ")){
					fprintf( fp, "Trait %d '%s'\n", 
						pos, fix_string(ch->pcdata->trait[pos]));
				}
			}

		}

		// write skills
		for ( sn = 0; sn < MAX_SKILL; sn++ )
		{
			// skill values
			if ( !IS_NULLSTR(skill_table[sn].name) && ch->pcdata->learned[sn] > 0 )
			{
				fprintf( fp, "Sk %d '%s'\n",
					ch->pcdata->learned[sn], skill_table[sn].name );
			}
			// skill last used
			if ( !IS_NULLSTR(skill_table[sn].name) && ch->pcdata->last_used[sn] > 800000000 )
			{
				fprintf( fp, "SkLU %d '%s'\n",
					(int)ch->pcdata->last_used[sn], skill_table[sn].name );
			}
		}

		// write groups
		for ( gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++ )
		{
			if ( ch->pcdata->skillgroup_known[gn])
			{
				fprintf( fp, "Gr '%s'\n",skillgroup_table[gn].name);
			}
		}

		if(!IS_NULLSTR(ch->pcdata->imm_role)){
			fprintf( fp, "ImmRole %s~\n", fix_string(ch->pcdata->imm_role));
		}

		// write who related stuff
		if(!IS_NULLSTR(ch->pcdata->immtitle)){
			fprintf( fp, "ImmTitle %s~\n", fix_string(ch->pcdata->immtitle));
		}
		if(!IS_NULLSTR(ch->pcdata->title)){
			fprintf( fp, "Title %s~\n", fix_string(ch->pcdata->title));
		}

		if(ch->pcdata->who_format){
			fprintf( fp, "WhoFormat %s~\n", who_format_name(ch->pcdata->who_format));
		}
	}

	// affects
	fwrite_affect_recursive(ch->affected, fp);

	ch->save_events(fp);

    fprintf( fp, "End\n\n" );
    return;
}

/**************************************************************************/
// write a pet 
void fwrite_pet( char_data *pet, FILE *fp)
{
	AFFECT_DATA *paf;
    
	fprintf(fp,"#PET\n");
	
	fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
	
	fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", (long) current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
		fprintf(fp,"ShD  %s~\n", fix_string(pet->short_descr));
    if (pet->long_descr != pet->pIndexData->long_descr)
		fprintf(fp,"LnD  %s~\n", fix_string(pet->long_descr));
	if (pet->description != pet->pIndexData->description)
		fprintf(fp,"Desc %s~\n", fix_string(pet->description));
    if (pet->race != pet->pIndexData->race)
		fprintf(fp,"Race %s~\n", race_table[pet->race]->name);
	if (pet->clan){
		fprintf( fp, "Clan %s~\n",pet->clan->savename());
	}
	fprintf(fp,"Sex  %d\n", pet->sex);
	if (pet->level != pet->pIndexData->level)
		fprintf(fp,"Levl %d\n", pet->level);
	fprintf(fp, "HMV  %d %d %d %d %d %d\n",
		pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
	if (pet->gold > 0)
		fprintf(fp,"Gold %ld\n",pet->gold);
	if (pet->silver > 0)
		fprintf(fp,"Silv %ld\n",pet->silver);
	
	fprintf( fp, "Room %d\n",
		(  pet->in_room == get_room_index( ROOM_VNUM_LIMBO )
		&& pet->was_in_room != NULL )
		? pet->was_in_room->vnum
		: pet->in_room == NULL ? ROOM_VNUM_LIMBO : pet->in_room->vnum );
	
	if (pet->exp > 0)
		fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act){
		fwrite_wordflag( act_flags, pet->act, "MAct ", fp); 
	}
    if (pet->affected_by != pet->pIndexData->affected_by){
		fwrite_wordflag( affect_flags, pet->affected_by, "AffBy ", fp); 
	}
    if (pet->affected_by2 != pet->pIndexData->affected_by2){
		fwrite_wordflag( affect2_flags, pet->affected_by2, "AffBy2 ", fp); 
	}
    if (pet->affected_by3 != pet->pIndexData->affected_by3){
		fwrite_wordflag( affect3_flags, pet->affected_by3, "AffBy3 ", fp); 
	}
	if (pet->comm != 0){
		fwrite_wordflag( comm_flags, pet->comm, "Com ", fp); 
	}
    fprintf(fp,"Pos  %d\n", pet->position == POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
		fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alliance != pet->pIndexData->alliance ||
		pet->tendency != pet->pIndexData->tendency)
		fprintf(fp, "Algn %d %d\n", pet->tendency, pet->alliance);
    if (pet->hitroll != pet->pIndexData->hitroll)
		fprintf(fp, "Hit  %d\n", pet->hitroll);
	if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
		fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
		pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
	fprintf(fp, "Attr %d %d %d %d %d %d %d %d %d %d\n",
		pet->perm_stats[STAT_ST], pet->perm_stats[STAT_QU],
		pet->perm_stats[STAT_PR], pet->perm_stats[STAT_EM],
		pet->perm_stats[STAT_IN],pet->perm_stats[STAT_CO], 
		pet->perm_stats[STAT_AG],
		pet->perm_stats[STAT_SD], pet->perm_stats[STAT_ME],
		pet->perm_stats[STAT_RE]);
	fprintf(fp, "AMod %d %d %d %d %d %d %d %d %d %d\n",
		pet->modifiers[STAT_ST], pet->modifiers[STAT_QU],
		pet->modifiers[STAT_PR], pet->modifiers[STAT_EM],
		pet->modifiers[STAT_IN], 
		pet->modifiers[STAT_CO], pet->modifiers[STAT_AG],
		pet->modifiers[STAT_SD], pet->modifiers[STAT_ME],
		pet->modifiers[STAT_RE] );
	
	
	for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;
		
		fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
			skill_table[paf->type].name,
			paf->where, paf->level, paf->duration, paf->modifier,paf->location,
			paf->bitvector);
    }

	pet->save_events(fp);
    
	fprintf(fp,"End\n");
	return;
}

/**************************************************************************/
// Write an object and its contents
void fwrite_obj( obj_data *obj, FILE *fp, int iNest, char *heading )
{
	EXTRA_DESCR_DATA *ed;
	
    //  Slick recursion to write lists backwards,
    //  so loading them will load in forwards order.
	if ( obj->next_content ){
		fwrite_obj( obj->next_content, fp, iNest, heading );
	}
	
    fprintf( fp, "#%s\n", heading );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum		);
	
	// only save obj->no_affects when the flag wont be ignored
	if (!obj->affected && obj->no_affects) 
		fprintf( fp,"no_affects\n"							);
	if (obj->chaos)
		fprintf( fp, "Chaos\n"								);
	
	if (obj->restrung) // more grepable file format
		fprintf( fp, "RESTRUNG %d\n", obj->pIndexData->vnum	);
	
	fprintf( fp, "Nest %d\n",	iNest						);
	
    // these data are only used if they do not match the defaults
    if ( obj->name != obj->pIndexData->name)
		fprintf( fp, "Name %s~\n",	fix_string(obj->name));
    if ( obj->short_descr != obj->pIndexData->short_descr)
		fprintf( fp, "ShD  %s~\n",	fix_string(obj->short_descr));
	if ( obj->description != obj->pIndexData->description)
		fprintf( fp, "Desc %s~\n",  fix_string(obj->description));
	if ( obj->extra_flags != obj->pIndexData->extra_flags)
		fprintf( fp, "ExtF %d\n",	obj->extra_flags		);
	if ( obj->extra2_flags != obj->pIndexData->extra2_flags)
		fprintf( fp, "ExtF2 %d\n",	obj->extra2_flags		);
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
		fprintf( fp, "WeaF %d\n",	obj->wear_flags			);
	if ( obj->item_type != obj->pIndexData->item_type)
		fprintf( fp, "Ityp %d\n",	obj->item_type			);
    if ( obj->weight != obj->pIndexData->weight)
		fprintf( fp, "Wt   %d\n",	obj->weight				);
	if ( obj->condition != obj->pIndexData->condition)
		fprintf( fp, "Cond %d\n",	obj->condition			);
	
	// variable data
	fwrite_wordflag(wear_location_types, obj->wear_loc, "WearLoc ", fp);
	if (obj->level != obj->pIndexData->level)
		fprintf( fp, "Lev  %d\n",	obj->level				);
	if (obj->timer != 0)
		fprintf( fp, "Time %d\n",	obj->timer				);
	fprintf( fp, "Cost %d\n",		obj->cost				);
	if (obj->value[0] != obj->pIndexData->value[0]
		||  obj->value[1] != obj->pIndexData->value[1]
		||  obj->value[2] != obj->pIndexData->value[2]
		||  obj->value[3] != obj->pIndexData->value[3]
		||  obj->value[4] != obj->pIndexData->value[4])
		fprintf( fp, "Val  %d %d %d %d %d\n",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3],
		obj->value[4] );
	
	switch ( obj->item_type )
	{
	case ITEM_COMPONENT:
		if ( obj->value[1] > 0 )
		{
			fprintf( fp, "Spell 1 '%s'\n", skill_table[obj->value[1]].name );
		}
		break;
	case ITEM_POTION:
	case ITEM_SCROLL:
	case ITEM_PILL:
		if ( obj->value[1] >= 0 )
		{
			fprintf( fp, "Spell 1 '%s'\n", 
				skill_table[obj->value[1]].name );
		}
		
		if ( obj->value[2] >= 0 )
		{
			fprintf( fp, "Spell 2 '%s'\n",
				skill_table[obj->value[2]].name );
		}
		
		if ( obj->value[3] >= 0 )
		{
			fprintf( fp, "Spell 3 '%s'\n",
				skill_table[obj->value[3]].name );
		}
		
		if ( obj->value[4] >= 0 )
		{
			fprintf( fp, "Spell 4 '%s'\n",
				skill_table[obj->value[4]].name );
		}
		
		break;
		
	case ITEM_STAFF:
	case ITEM_WAND:
		if ( obj->value[3] >= 0 )
		{
			fprintf( fp, "Spell 3 '%s'\n", 
				skill_table[obj->value[3]].name );
		}
		
		break;

	case ITEM_PARCHMENT:
		fprintf( fp, "ParchmentLanguage '%s'\n", 
			language_safe_lookup_by_id(obj->value[3])->name );
		break;
	}

	fwrite_affect_recursive(obj->affected, fp);
	
	// attune stuff
	if (( IS_SET( obj->attune_flags, ATTUNE_NEED_TO_USE )) &&  
		( obj->attune_id != 0
	   || obj->attune_modifier != 0
	   || obj->attune_next > 0
	   || IS_SET( obj->attune_flags, ATTUNE_PREVIOUS )))
	{
		fprintf( fp, "Attune %ld %ld %d %ld\n",
			(long)obj->attune_id,
			obj->attune_flags,
			obj->attune_modifier,
			(long)obj->attune_next
			);
	}

    // write extended descriptions for the object 
    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
		fprintf( fp, "ExDe %s~ .%s~\n",
			fix_string(ed->keyword), fix_string(ed->description) );
    }

	obj->save_events(fp);
	
    fprintf( fp, "End\n\n" );
	
    if ( obj->contains ){
		fwrite_obj( obj->contains, fp, iNest + 1, heading);
	}
	
    return;
}

/**************************************************************************/
void colour_convert_player( char_data *ch);
/**************************************************************************/
// Load a char and inventory into a new ch structure.
// return true if player loaded okay
bool load_char_obj( connection_data *d, char *name )
{
	char strsave[MIL];
	char_data *ch;
	PFILE_TYPE pt;
	FILE *fp;
	bool found;
	int stat;
	
	ch = new_char();
	ch->pcdata = new_pcdata();
	
	d->character					= ch;
	ch->desc						= d;
	ch->name						= str_dup( capitalize(name) );
	ch->level						=0;
	ch->trust						=0;
	ch->player_id					= get_pc_id();
	ch->race						= race_lookup("human");
	ch->act 						= PLR_NOSUMMON;
	ch->config 						= CONFIG_NOCHARM;
	ch->comm						= COMM_COMBINE | COMM_PROMPT;
	ch->mounted_on					=NULL;
	ch->ridden_by					=NULL;
	ch->tethered					=false;
	ch->bucking 					=false;
	ch->wildness					=100;
	ch->will						=100;
	ch->pcdata->confirm_delete		= false;
	ch->pcdata->pwd 				= str_dup( "" );
	ch->pcdata->webpass				= str_dup( "" );
	ch->pcdata->bamfin				= str_dup( "" );
	ch->pcdata->bamfout 			= str_dup( "" );
	for (stat =0; stat < MAX_STATS; stat++)
		ch->perm_stats[stat]		= 1;
	ch->pcdata->condition[COND_THIRST]	= 48;
	ch->pcdata->condition[COND_FULL]	= 48;
	ch->pcdata->condition[COND_HUNGER]	= 48;
	ch->pcdata->autoafkafter			=0;
	ch->pcdata->qpoints					=0;
	ch->pcdata->pconfig					=0;
	ch->pcdata->thief_until				=0;
	ch->pcdata->killer_until			=0;

	// no character name
	if (IS_NULLSTR(name)){
		return false;
	}
	
	// find the pfile - from which directory it is in
	found = false;
	pt=find_pfiletype(ch->name);
	ch->pcdata->pfiletype=pt;
	if(pt==PFILE_NONE || pt==PFILE_MULTIPLE){
		return false;
	}

	strcpy( strsave, pfilename(name, pt));

	fclose( fpReserve );
	
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
		int iNest;
		
		for ( iNest = 0; iNest < MAX_NEST; iNest++ ){
			rgObjNest[iNest] = NULL;
		}
		
		found = true;
		for ( ; ; )
		{
			char letter;
			char *word;
			
			letter = fread_letter( fp );
			if ( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			}
			
			if ( letter != '#' )
			{
				bug("Load_char_obj: # not found.");
				break;
			}
			
			word = fread_word( fp );
			if( !str_cmp( word, "PLAYER" ) ){
				fread_char( ch, fp );
			}
			else if ( !str_cmp(word,"OBJECT") || !str_cmp( word, "O") )
			{
				OBJ_DATA *o=fread_obj( fp, strsave);
				if(o){
					obj_to_char(o, ch);
				}			
			}else if ( !str_cmp( word, "PET"    ) ){
				fread_pet( ch, fp );
			}else if ( !str_cmp( word, "PO") ){ // pet objects
				OBJ_DATA *o=fread_obj( fp, strsave);
				if(o && ch->pet){
					obj_to_char(o, ch->pet);
				}			
			}else if ( !str_cmp( word, "END"    ) ){
				break;
			}else{
				bugf("Load_char_obj: bad section '%s'.", word);
				break;
			}
		}
		fclose( fp );
    }
	
    fpReserve = fopen( NULL_FILE, "r" );
	
	
    // initialize race 
    if (found)
    {
		int i;
		
		if(ch->race == -1){
			ch->race = race_lookup("human");
		}
		if(ch->race == -1){
			logf("load_char_obj(): unfound race for character '%s', human backup not found either, set race to 0 (whatever that is).",
				ch->name);
			ch->race=0;
		}
		
		ch->size = race_table[ch->race]->size;
		ch->dam_type = DAM_HARM; // punch
		
		// add racial skills 
		for(i = 0; i < MAX_RACIAL_SKILLS; i++){
			if(race_table[ch->race]->skills[i]==-1){
				break;
			}
			group_add(ch,skill_table[race_table[ch->race]->skills[i]].name,false, 1);
		}

		if(IS_AFFECTED(ch, AFF_FLYING)){
			ch->affected_by = ch->affected_by|race_table[ch->race]->aff;
		}else{ // werent flying when they logged out
			ch->affected_by = ch->affected_by|race_table[ch->race]->aff;
			REMOVE_BIT(ch->affected_by, AFF_FLYING);
		}
		affect_fly_update(ch); // set/remove DYN_MAGICAL_FLYING as required

		ch->affected_by2= ch->affected_by2| race_table[ch->race]->aff2;
		ch->affected_by3= ch->affected_by3| race_table[ch->race]->aff3;
		ch->imm_flags   = ch->imm_flags | race_table[ch->race]->imm;
		ch->res_flags   = ch->res_flags | race_table[ch->race]->res;
		ch->vuln_flags  = ch->vuln_flags | race_table[ch->race]->vuln;
		ch->form    = race_table[ch->race]->form;
		ch->parts   = race_table[ch->race]->parts;
    }

	if(!ch->pcdata->custom_colour){
		ch->pcdata->custom_colour=fread_custom_colours(NULL, true);
	}
	if(!IS_NULLSTR(ch->desc->colour_memory.custom_colour)){
		free(ch->desc->colour_memory.custom_colour);
	}
	ch->desc->colour_memory.custom_colour=ch->pcdata->custom_colour;
	ch->desc->colour_mode=ch->pcdata->colourmode;

	if(!ch->pcdata->custom_colour_scheme){
		ch->pcdata->custom_colour_scheme=default_colour_template;
	}
	ch->desc->colour_memory.colour_template=ch->pcdata->custom_colour_scheme;
	ch->desc->flashing_disabled=ch->pcdata->flashing_disabled;

	colour_convert_player( ch);
	
    return found;
}


/**************************************************************************/
// Read in a char.
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )        \
				{                                       \
				    field  = value;                     \
					 fMatch = true;						\
				    break;                              \
				}

void fread_char( char_data *ch, FILE *fp )
{
	char *word=NULL;
	char *previousword;
	bool fMatch;
	int count = 0;
	time_t lastlogoff = current_time;
	int percent;
	int note_post_index=0;
	
	logf("Loading %s. (socket %d)", ch->name, ch->desc->connected_socket);
	
	for ( ; ; )
	{
		previousword=word;
		word   = feof( fp ) ? (char*)"End" : fread_word( fp );
		fMatch = false;
		
		switch ( UPPER(word[0]) )
		{
		case '*':
			fMatch = true;
			fread_to_eol( fp );
			break;
			
		case 'A':		
			KEY( "ANote",	ch->pcdata->last_anote,  fread_number( fp ) );
			KEY( "AVote",	ch->pcdata->autovote,	fread_number( fp ) );
			KEY( "AfBy",	ch->affected_by,		fread_wordflag( affect_flags, fp ));
			KEY( "AfBy2",	ch->affected_by2,		fread_wordflag( affect2_flags, fp ));
			KEY( "AfBy3",	ch->affected_by3,		fread_wordflag( affect3_flags, fp ));
			KEY( "AAA",		ch->pcdata->autoafkafter,	fread_number( fp ) ); 

			if ( !str_cmp( word, "Act" )){
				if (IS_OLD_CHVER(ch)){
					ch->act =fread_flag( fp ) ;
				}else{ 
					ch->act =fread_wordflag( plr_flags, fp );
				}
				fMatch = true;
				break;
			}
			if ( !str_cmp( word, "AffectedBy" )){
				if (IS_OLD_CHVER(ch)){
					ch->affected_by =fread_flag( fp ) ;
				}else{ 
					ch->affected_by =fread_wordflag( affect_flags, fp );
				}
				fMatch = true;
				break;
			}
			
			if ((!str_cmp( word, "Alignment")||(!str_cmp( word, "Alig"))))
			{
				if((!IS_NPC(ch)))
				{
					ch->tendency=fread_number( fp );
					ch->alliance=fread_number( fp );
				}
				fMatch=true;
			}

			
			if (!str_cmp( word, "Alia"))
			{
				if (count >= MAX_ALIAS)
				{
					fread_to_eol(fp);
					fMatch = true;
					break;
				}
				
				ch->pcdata->alias[count]        = str_dup(fread_word(fp));
				ch->pcdata->alias_sub[count]    = str_dup(fread_word(fp));
				count++;
				fMatch = true;
				break;
			}
			
			if (!str_cmp( word, "Alias"))
			{
				if (count >= MAX_ALIAS)
				{
					fread_to_eol(fp);
					fMatch = true;
					break;
				}
				
				ch->pcdata->alias[count]        = str_dup(fread_word(fp));
				ch->pcdata->alias_sub[count]    = fread_string(fp);
				count++;
				fMatch = true;
				break;
			}
			
			if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
			{
				fread_to_eol(fp);
				fMatch = true;
				break;
			}
			
			if (!str_cmp(word,"ACs"))
			{
				int i;
				
				for (i = 0; i < 4; i++)
					ch->armor[i] = fread_number(fp);
				fMatch = true;
				break;
			}
				
			if (!str_cmp(word, "Affc")) // old affect format
			{
				AFFECT_DATA *paf;
				int sn;
				
				paf = new_affect();
				
				char *skn=fread_word(fp);
				sn = skill_lookup(skn);
				if (sn < 0){
					bugf("Fread_char: unknown skill '%s'.", skn);
				}else{
					paf->type = sn;
				}
				
				paf->where  = fread_number(fp);
				paf->level      = fread_number( fp );
				paf->duration   = fread_number( fp );
				paf->modifier   = fread_number( fp );
				paf->location   = translate_old_apply_number(fread_number( fp ));
				paf->bitvector  = fread_number( fp );
				paf->next       = ch->affected;
				ch->affected    = paf;
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "Affect"))
			{
				AFFECT_DATA *paf=fread_affect(fp);
				paf->next       = ch->affected;
				ch->affected    = paf;
				fMatch = true;
				break;
			}
			
			if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
			{
				int stat;
				if(ch->version>8)
				{
					for (stat = 0; stat < MAX_STATS; stat ++)
						ch->modifiers[stat] = fread_number(fp);
				}
				fMatch = true;
				break;
			}
			
			if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
			{
				int stat;
				if(ch->version>8)
				{
					for (stat = 0; stat < MAX_STATS; stat++)
						ch->perm_stats[stat] = fread_number(fp);
				}
				fMatch = true;
				break;
			}
			
			if( !str_cmp(word,"APot") )
			{
				int stat;
				for(stat=0; stat<MAX_STATS; stat++)
					ch->potential_stats[stat] = fread_number(fp);
				fMatch = true;
				break;
			}			

			if (!str_cmp(word, "AutoIndent"))
			{
				ch->pcdata->mpedit_autoindent=fread_number(fp);
				fMatch = true;
				break;
			}
			break;
			
		case 'B':
			KEY( "Bank",		ch->bank,						fread_number( fp ));
			KEY( "Bamfin",		ch->pcdata->bamfin,				fread_string( fp ));
			KEY( "Bamfout",		ch->pcdata->bamfout,			fread_string( fp ));
			KEY( "BattleLag",	ch->pcdata->battlelag,			fread_string( fp ) );
			KEY( "Bin",			ch->pcdata->bamfin,				fread_string( fp ));
			KEY( "Bleed",		ch->bleeding,					fread_number( fp ));
			KEY( "Bout",		ch->pcdata->bamfout,			fread_string( fp ));
			KEY( "Bdate",		ch->pcdata->birthdate,			fread_number( fp ));
			KEY( "BYearMod",	ch->pcdata->birthyear_modifier,	fread_number( fp ));
			KEY( "Birthplace",	ch->pcdata->birthplace,			fread_string( fp ));
			KEY( "beginning_remort",	ch->beginning_remort,	fread_number( fp ));
			break;
		
		case 'C':
			KEY( "Clan",		ch->clan,						clan_slookup(fread_string(fp)));
			KEY( "Config",		ch->config,						fread_wordflag( config_flags, fp ));
			KEY( "Config2",		ch->config2,					fread_wordflag( config2_flags, fp ));
			KEY( "Cnotes",		ch->pcdata->charnotes,			fread_string( fp ));
			KEY( "Council",		ch->pcdata->council,			fread_wordflag( council_flags, fp ));
			KEY( "Crest",		ch->pcdata->crest,				fread_string( fp ));
			KEY( "Created_from",ch->pcdata->created_from,		fread_string( fp ));
			KEY( "Custom_colours", ch->pcdata->custom_colour,	fread_custom_colours(fp, true));
			KEY( "Colourmode",	ch->pcdata->colourmode,			(COLOUR_TYPE)fread_wordflag(colourmode_types,fp ));

			KEY( "CC",			ch->pcdata->colour_code,			fread_letter( fp ));
			KEY( "CPref",		ch->colour_prefix,				fread_letter( fp ));

			if ( !str_cmp( word, "Channeloff" ) )
			{		
				char *text=fread_string(fp);
				ch->pcdata->channeloff=
					channel_convert_text_to_bitflags(text);
				free_string(text);
				fMatch = true;
				break;
			}

			if ( !str_cmp( word, "Colour_scheme" )){
				char *scheme=fread_string(fp);
				ch->pcdata->custom_colour_scheme=find_colour_template(scheme);
				free_string(scheme);
				if(!ch->pcdata->custom_colour_scheme){
					ch->pcdata->custom_colour_scheme=default_colour_template;
				}
				fMatch = true;
				break;
			}		
			
			if ( !str_cmp( word, "Conf" )){
				if (IS_OLD_CHVER(ch)){
					ch->config=fread_flag( fp ) ;
				}else{ 
					ch->config=fread_wordflag( config_flags, fp );
				}
				fMatch = true;
				break;
			}		

			if ( !str_cmp( word, "Class" ) || !str_cmp(word,"Cla"))
			{
				char *clss_buf;

				if (IS_OLD_CHVER(ch)){
					ch->clss = fread_number(fp);
				}else{
					clss_buf=fread_string(fp);
					ch->clss =class_exact_lookup(clss_buf);
					if(ch->clss ==-1)
					{ 
						bugf("fread_char: Unfound class '%s' in %s's pfile!",
							clss_buf, ch->name);
						{
							char msgbody[MSL];
							sprintf(msgbody,"Unfound class '%s' in %s's pfile!\r\n", clss_buf, ch->name);
							autonote(NOTE_SNOTE, "fread_char()", "Unfound class in player pfile!", "code cc: imm", 
								msgbody, true);
						}
						ch->clss=0; // badly need class 0 to be the classless class
					}
				}
				fMatch = true;
				break;
			}		

			if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
			{
				ch->pcdata->condition[0] = fread_number( fp );
				ch->pcdata->condition[1] = fread_number( fp );
				ch->pcdata->condition[2] = fread_number( fp );
				fMatch = true;
				break;
			}
			if (!str_cmp(word,"Cnd"))
			{
				ch->pcdata->condition[0] = fread_number( fp );
				ch->pcdata->condition[1] = fread_number( fp );
				ch->pcdata->condition[2] = fread_number( fp );
				ch->pcdata->condition[3] = fread_number( fp );
				fMatch = true;
				break;
			}
			if ( !str_cmp( word, "Comm" )){
				if (IS_OLD_CHVER(ch)){
					ch->comm =fread_flag( fp ) ;
				}else{ 
					ch->comm =fread_wordflag( comm_flags, fp );
				}
				fMatch = true;
				break;
			}		
			break;
			
		case 'D':
			KEY( "Damroll",		ch->damroll,            fread_number( fp ));
			KEY( "Dam",			ch->damroll,            fread_number( fp ));
			KEY( "Description",	ch->description,        fread_string( fp ));
			KEY( "Desc",		ch->description,        fread_string( fp ));
			KEY( "Diplo",		ch->pcdata->diplomacy,	fread_number( fp ));
			KEY( "Dptn",		ch->pcdata->dip_points, fread_number( fp ));
			KEY( "DeityName",	ch->deity,				deity_lookup( fread_string(fp)) );

			// Duel records
			KEY( "Duel_challenged",		ch->duel_challenged, fread_number( fp ));
			KEY( "Duel_decline",		ch->duel_decline, fread_number( fp ));
			KEY( "Duel_accept",			ch->duel_accept, fread_number( fp ));
			KEY( "Duel_ignore",			ch->duel_ignore, fread_number( fp ));
			KEY( "Duel_bypass",			ch->duel_bypass, fread_number( fp ));
			KEY( "Duel_subdues_before_karn_loss",ch->duel_subdues_before_karn_loss, fread_number( fp ));
			break;   
			
		case 'E':
			KEY( "Email",		ch->pcdata->email,		fread_string( fp ));
			KEY( "EyeCol",		ch->pcdata->eyecolour,	fread_string( fp ));
			if ( !str_cmp( word, "Events" ) ){
				ch->load_events(fp);
				fMatch = true;
				break;
			}

			if ( !str_cmp( word, "End" ) )
			{
				// adjust hp mana move up  -- here for speed's sake 
				// fully charge up them in 8 hours                  
				percent =(int) (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
				
				
				percent = UMIN(percent,100);
				
				if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
					&& !IS_AFFECTED(ch,AFF_PLAGUE))
				{
					ch->hit    += (ch->max_hit - ch->hit) * percent / 100;
					ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
					ch->move    += (ch->max_move - ch->move)* percent / 100;
					
					// make them sleep at about 3-4 times slower than online sleep
					// two hours is about 100% rested 
					percent = (int)(current_time - lastlogoff) * 100 / (60 * 60);
					if (percent> 0) 
					{
						percent = UMIN(percent,200); // Allowed to over sleep 
						
						ch->pcdata->tired -= 15 * percent / 100;
						if(ch->pcdata->tired<0)
							ch->pcdata->tired=0;
					}
					if (ch->hit>0 && ch->position<POS_SLEEPING)
						ch->position=POS_RESTING;
				}
				
				if (IS_NULLSTR(ch->pcdata->pwd)){
					bugf("Null password for %s! (corrupt pfile?!?) - password set to 'password'", ch->name);
					ch->pcdata->pwd=str_dup(dot_crypt("password", ch->name));
				}

				// rank name changed safety code
				if(ch->clanrank<0 || ch->clanrank>MAX_RANK){
					logf("invalid clan rank for %s, set to minrank", ch->name);
					ch->clanrank=ch->clan->minRank();					
				}
						
				// update the position systems to the new code 
				return;
			}
			
			KEY( "Exp", ch->exp,        fread_number( fp ) );
			KEY( "ExpireRecallInn", ch->expire_recall_inn, fread_number( fp ));
			break;

        case 'F':
            KEY( "FadeIN",      ch->pcdata->fadein,     fread_string( fp ));
            KEY( "FadeOUT",     ch->pcdata->fadeout,    fread_string( fp ));
            KEY( "flashing_disabled",  ch->pcdata->flashing_disabled, (fread_number( fp )==1?true:false));
            break;
        
		case 'G':
			KEY( "GPro",		ch->gprompt,             fread_string( fp ) );
			KEY( "GPrompt",		ch->gprompt,             fread_string( fp ) );
			KEY( "Gold",   ch->gold,               fread_number( fp ) );
			if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
			{
				int gn;
				char *temp;
				
				temp = fread_word( fp );
				if(!str_cmp(temp,"rom basics")){ // convert code
					temp="dawn basics";
				}
				gn = skillgroup_lookup(temp);
				if ( gn < 0 )
				{
					if(ch->version<=6)
					{
						ch->train+=5;
					}
					
					bugf("fread_char(): unknown group '%s'.", temp);
				}else{
					gn_add(ch,gn);
				}
				
				fMatch = true;
			}
			break;
			
		case 'H':
			KEY( "HairCol",		ch->pcdata->haircolour,	fread_string( fp ));
			KEY( "Height",		ch->pcdata->height,		fread_number( fp ));
			KEY( "HeroXp",		ch->pcdata->heroxp,		fread_number( fp ));
			KEY( "HeroLevelCount", ch->pcdata->hero_level_count, fread_number( fp ));
			KEY( "History",		ch->pcdata->history,	fread_string( fp ));
			KEY( "Hitroll",		ch->hitroll,            fread_number( fp ));
			KEY( "Hit",         ch->hitroll,            fread_number( fp ));
			
			if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
			{
				ch->hit         = fread_number( fp );
				ch->max_hit     = fread_number( fp );
				ch->mana        = fread_number( fp );
				ch->max_mana    = fread_number( fp );
				ch->move        = fread_number( fp );
				ch->max_move    = fread_number( fp );
				fMatch = true;
				break;
			}
			
			if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
			{
				ch->pcdata->perm_hit   = fread_number( fp );
				ch->pcdata->perm_mana   = fread_number( fp );
				ch->pcdata->perm_move   = fread_number( fp );
				fMatch = true;
				break;
			}

			if(!str_cmp( word, "HostV"))
			{
				fread_string(fp);
				fread_to_eol(fp);
				ch->host_validated=1;
				SET_BIT(ch->dyn,DYN_MOB_SEE_ALL|DYN_IMMLASTON);
				fMatch = true;
				break;
			}
			
			break;
			
		case 'I':
			KEY( "InvisLevel",     ch->invis_level,        fread_number( fp ) );
			KEY( "Inco",	ch->incog_level,        fread_number( fp ) );
			KEY( "Invi",	ch->invis_level,        fread_number( fp ) );
			KEY( "INote",	ch->pcdata->last_inote,  fread_number( fp ) );
			KEY( "IWizi",	ch->iwizi,		fread_number( fp ) );
			KEY( "Immwiznet",ch->wiznet[0],	fread_wordflag( wiznet_flags, fp ));			
			KEY( "ImmTitle",ch->pcdata->immtitle,  fread_string( fp ) );
			KEY( "ImmRole",     ch->pcdata->imm_role,        fread_string( fp ) );
			if ( !str_cmp( word, "ITN" )) 
			{
				ch->pcdata->immtalk_name = fread_string( fp );
				if(!str_cmp(ch->pcdata->immtalk_name,"(null)")){
					ch->pcdata->immtalk_name=str_dup("");
				}
				fMatch = true;
			}
			if ( !str_cmp( word, "Id" )) 
			{
				ch->player_id = fread_number( fp );
				fread_to_eol(fp);
				fMatch = true;
			}
			break;
			
		case 'K':
			KEY( "Karns", ch->pcdata->karns, fread_number(fp));
			KEY( "KCntd", ch->pcdata->next_karn_countdown, fread_number(fp));
			KEY( "Know_index", ch->know_index, fread_number(fp));			
			KEY( "Know_id", ch->know_index, fread_number(fp));			
			KEY( "Killer_until",ch->pcdata->killer_until,(time_t)fread_number( fp ));
			break;
			
		case 'L':
			KEY( "LastLevel",		ch->pcdata->last_level, fread_number( fp ) );
			KEY( "LLev",			ch->pcdata->last_level, fread_number( fp ) );	
			if ( !str_cmp( word, "Level" )) 
			{
				int new_level=fread_number( fp );
				if(ch->level){
					logf("Reading in a second level value of %d for pfile %s... ignoring!",
						new_level, ch->name);
				}else{
					ch->level=new_level;
				}
				fMatch = true;
				break;
			}
			KEY( "LongDescr",		ch->long_descr,         fread_string( fp ) );
			KEY( "LnD",				ch->long_descr,         fread_string( fp ) );

			if ( !str_cmp( word, "Lang" ) )
			{
				if (IS_OLD_CHVER(ch)){
					fread_number(fp); // eat up the number
					if(ch->race>0){
						ch->language = race_table[ch->race]->language;
					}					
				}else{
					char *read_buf=fread_string(fp);
					ch->language = language_lookup(read_buf);
					if(!ch->language){ 
						bugf("fread_char: Unfound language '%s' in %s's pfile!",
							read_buf, ch->name);
					}
					free_string(read_buf);				
				}
				if(!ch->language){
					logf("Language set to unknown for '%s'", ch->name);
					ch->language=language_unknown;
				}
				fMatch = true;
				break;
			}		
						
			if ( !str_cmp( word, "LogO" )) 
			{
				lastlogoff = fread_number( fp );
				ch->pcdata->last_logout_time=lastlogoff;
				fread_to_eol(fp);
				
				fMatch = true;
			}
			if ( !str_cmp( word, "LogOS" )) 
			{
				ch->pcdata->last_logout_site = fread_string(fp);
				fread_to_eol(fp);
				fMatch = true;
			}
			if ( !str_cmp( word, "Last_ic_room" ) || !str_cmp( word, "LIcRm" ))
			{
				ch->last_ic_room = get_room_index( fread_number( fp ) );
				fMatch = true;
			}
			break;
			
		case 'M':
			KEY( "Merit", ch->pcdata->merit,	fread_number(fp));
			KEY( "MKil",  ch->pcdata->mkills,	fread_number( fp ) );
			KEY( "MDef",  ch->pcdata->mdefeats, fread_number( fp ) );
			KEY( "MiscN", ch->pcdata->last_misc,	fread_number( fp ));
			break;
			
		case 'N':
			KEY( "Name",   ch->name,               fread_string( fp ) );
			KEY( "Note",   ch->pcdata->last_note,  fread_number( fp ) );
//			KEY( "NoteNet",ch->notenet,	fread_wordflag( notenet_flags, fp ));
			if ( !str_cmp( word, "NoteNet" )){
				if (IS_OLD_CHVER(ch)){
					ch->notenet=fread_flag( fp ) ;
				}else{ 
					ch->notenet=fread_wordflag( notenet_flags, fp );
				}
				fMatch = true;
				break;
			}	
			
			if (!str_cmp(word,"Not"))
			{
				ch->pcdata->last_note                   = fread_number(fp);
				ch->pcdata->last_idea                   = fread_number(fp);
				ch->pcdata->last_penalty                = fread_number(fp);
				ch->pcdata->last_news                   = fread_number(fp);
				ch->pcdata->last_changes                = fread_number(fp);
				fMatch = true;
				break;
			}

			if ( !str_cmp( word, "NotePost" )){
				ch->pcdata->note_post_time[note_post_index++]=fread_number(fp);
				note_post_index=UMIN(note_post_index,MAX_NOTE_POST_TIME_INDEX-1);
				fMatch = true;
				break;
			}	

			// Addition Vanish skill
			KEY( "NVanish",			ch->pcdata->next_vanish, (fread_number( fp ) + current_time) );
			KEY( "NWorshipTime",	ch->pcdata->worship_time, (current_time - fread_number( fp )) );
			break;
			
		case 'O':
			KEY( "OLCProm",		ch->olcprompt,	fread_string( fp ) );
			KEY( "OLCPrompt",	ch->olcprompt,	fread_string( fp ) );
			KEY( "OPrmt",		ch->olcprompt,	fread_string( fp ) );
			KEY( "OWizi",		ch->owizi,		fread_number( fp ) );
			KEY( "OlcWizi",		ch->olcwizi,	fread_number( fp ) );
			if ( !str_cmp( word, "OldCla" ) ) 
			{
				if(ch->pcdata){
					char *clss_buf;
					clss_buf=fread_string(fp);
					ch->pcdata->old_clss =class_exact_lookup(clss_buf);
					if(ch->pcdata->old_clss == -1){
						ch->pcdata->old_clss=0; 
					}
				}
				fMatch = true;
				break;
			}
			break;
			
		case 'P':
			KEY( "Password",       ch->pcdata->pwd,        fread_string( fp ) );
			KEY( "Pass",   ch->pcdata->pwd,        fread_string( fp ) );
			KEY( "Panic",  ch->pcdata->panic,		fread_number( fp ) );
			KEY( "PConfig",	ch->pcdata->pconfig,	fread_wordflag( pconfig_flags, fp ));
			KEY( "PKool",  ch->pkool, fread_number( fp ) );
			KEY( "PKsafe", ch->pksafe, fread_number( fp ) );
			KEY( "PKsaf",  ch->pksafe, fread_number( fp ) );
			KEY( "PKills", ch->pkkills, fread_number( fp ) );
			KEY( "PKill",  ch->pkkills, fread_number( fp ) );
			KEY( "PKDef",  ch->pkdefeats, fread_number( fp ) );
			KEY( "PKNote",  ch->pcdata->last_pknote,  fread_number( fp ) );		
			KEY( "Points", ch->pcdata->points,     fread_number( fp ) );
			KEY( "Pnts",        ch->pcdata->points,     fread_number( fp ) );
			KEY( "Practice",       ch->practice,           fread_number( fp ) );
			KEY( "Prac",        ch->practice,           fread_number( fp ) );
			KEY( "Prompt",      ch->prompt,             fread_string( fp ) );
			KEY( "Prom",        ch->prompt,             fread_string( fp ) );
			KEY( "PrefMSP",		ch->pcdata->preference_msp, (PREFERENCE_TYPE)fread_wordflag( preference_types, fp ));
			KEY( "PrefMXP",		ch->pcdata->preference_mxp,	(PREFERENCE_TYPE)fread_wordflag( preference_types, fp ));
			KEY( "PrefDAWNFTP",	ch->pcdata->preference_dawnftp,	(PREFERENCE_TYPE)fread_wordflag( preference_types, fp ));
			KEY( "PrefColourInSocials",	ch->pcdata->preference_colour_in_socials,(PREFERENCE_TYPE)fread_wordflag( preference_types, fp ));

			if ( !str_cmp( word, "Played" ) || !str_cmp( word, "Plyd" )) 
			{
				ch->played= fread_number( fp );
				fread_to_eol(fp);
				fMatch = true;
				break;
			}

			if ( !str_cmp( word, "Pos" ) || !str_cmp( word, "Position" ) )
			{
				char *read_buf;

				if (IS_OLD_CHVER(ch)){
					char *t;
					int p;
					t=fread_word(fp);

					// old format
					if(is_number(t)){
						p=atoi(t);
						if(p>6){
							p++; // compenstate for kneel being added at position 7
						}
						ch->position=p;
					}else{
						// new format
						p=position_lookup(t);
						if(p>=0){
							ch->position=p;
						}else{
							ch->position=POS_STANDING;
						}
					}
					fMatch = true;
					break;
				}else{
					read_buf=fread_string(fp);
					ch->position= position_lookup(read_buf);
					if(ch->position ==-1)
					{ 
						bugf("fread_char: Unfound position '%s' in %s's pfile!",
							read_buf, ch->name);
						ch->position=POS_STANDING;
					}
				}
				fMatch = true;
				break;
			}		
			break;

        case 'Q':
			KEY( "Quest", ch->pcdata->qpoints, fread_number(fp));
 			break;			
			
		case 'R':
			KEY( "Rps", ch->pcdata->rp_points, fread_number(fp));			
			KEY( "Rank", ch->clanrank, ch->clan->rank_lookup(fread_string(fp)));
			KEY( "Reroll_cnt", ch->pcdata->reroll_counter, fread_number(fp));	 
			KEY( "RecallRoom", ch->recall_room, fread_number(fp));
			KEY( "RecallInnRoom", ch->recall_inn_room, fread_number(fp));
			KEY( "RCnt",	ch->pcdata->reroll_counter, fread_number(fp));	 
			KEY( "Rpcnt", ch->pcdata->rp_count, fread_number(fp));
			KEY( "Remrt", ch->remort, fread_number(fp));

			KEY( "Remort", ch->remort, fread_number(fp));
			KEY( "RXP",  ch->pcdata->reduce_xp_percent,  fread_number(fp));	 
			KEY( "RRPS", ch->pcdata->reduce_rps_percent, fread_number(fp));	 
			if ( !str_cmp( word, "Room" ) )
			{
				int room_vnum=fread_number( fp );
				ch->in_room = get_room_index( room_vnum );
				if ( !ch->in_room){
					logf("fread_char(): character '%s' couldn't be logged into "
						"room vnum %d because room no longer exists... putting them in limbo instead (%d)",
						ch->name, room_vnum, ROOM_VNUM_LIMBO);
					ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
				}
				fMatch = true;
			}
			//KEY( "Race",        ch->race,	race_lookup(fread_string( fp )) );
			if ( !str_cmp( word, "Race" ) )
			{
				ch->race = race_lookup(fread_string( fp ));
				if (ch->race == -1)
				{
					ch->race = race_lookup("human");
					bugf("fread_char: ch '%s' has an invalid race!!!",
						ch->name);
				}
				fMatch = true;
			}
			
			break;
			
		case 'S':
			KEY( "SubV",		ch->subversion,				fread_number( fp ));

			// sublevel system
			KEY( "SLvl",		ch->pcdata->sublevel,		fread_number( fp ));
			KEY( "SubP",		ch->pcdata->sublevel_pracs,	fread_number( fp ));
			KEY( "SubT",		ch->pcdata->sublevel_trains,fread_number( fp ));

			KEY( "SavingThrow",	ch->saving_throw,			fread_number( fp ));
			KEY( "Save",		ch->saving_throw,			fread_number( fp ));
			KEY( "Scro",		ch->lines,					fread_number( fp ));
			KEY( "Sec",			ch->pcdata->security,		fread_number( fp ));

			KEY( "ShortDescr",	ch->short_descr,			fread_string( fp ));
			KEY( "ShD",			ch->short_descr,			fread_string( fp ));
			KEY( "Silv",		ch->silver,					fread_number( fp ));
			KEY( "Stun",		ch->is_stunned,				fread_number( fp ));
			KEY( "Surname",		ch->pcdata->surname,		fread_string( fp ));
			KEY( "SNote",		ch->pcdata->last_snote,			fread_number( fp ));

			if ( !str_cmp( word, "Strip_channel_colours" ) )
			{		
				char *text=fread_string(fp);
				ch->pcdata->strip_colour_on_channel=
					channel_convert_text_to_bitflags(text);
				free_string(text);
				fMatch = true;
				break;
			}

			if ( !str_cmp( word, "Saycol" ) )
			{		
				char buf[MSL];
				char *scw=fread_word( fp );
				ch->saycolour=scw[0];
				sprintf(buf, "`%c", ch->saycolour);
				if (c_str_len(buf)!=0){
					bug("Invalid saycolour in pfile - ignored");
					ch->saycolour='x';
				}
				fMatch = true;
				break;
			}

			if ( !str_cmp( word, "Sex" ) )
			{
				char *read_buf;

				if (IS_OLD_CHVER(ch)){
					ch->sex = fread_number(fp);
				}else{
					read_buf=fread_string(fp);
					ch->sex =sex_lookup(read_buf);
					if(ch->sex ==-1)
					{ 
						bugf("fread_char: Unfound sex '%s' in %s's pfile!",
							read_buf, ch->name);
						ch->sex=0;
					}
				}
				fMatch = true;
				break;
			}		
			

			if ( !str_cmp( word, "SkLU" ) ) // last time the skill was used
			{
				int sn;
				int value;
				char *temp;
				
				value = fread_number( fp );
				temp = fread_word( fp ) ;
				sn = skill_lookup(temp);
				
				if ( sn < 0 ){
					bugf("Fread_char: unknown skill '%s'.", temp);
				}else{
					if (value>100000){ // reset lastused counters
						ch->pcdata->last_used[sn] = (time_t)value;
					}else{
						ch->pcdata->last_used[sn]=0;
					}					
				}
				fMatch = true;
				break;
			}
			
			if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
			{
				int sn;
				int value;
				char *temp;
				
				value = fread_number( fp );
				temp = fread_word( fp ) ;
				sn = skill_lookup(temp);
				if(sn < 0 ){
					bugf("fread_char: unknown skill '%s'.", temp);
				}else{
					if((ch->version<=6)&&(sn>=FIRST_SPELL)&&
						(sn<=LAST_SPELL))    
					{
						ch->practice+=value/33+1;       
					}else{
						ch->pcdata->learned[sn] = value;
					}
				}
				fMatch = true;
			}
			
			break;
			
		case 'T':
			KEY( "Temple",		ch->temple,					fread_number( fp ));
			KEY( "Tired",		ch->pcdata->tired,			fread_number( fp ));
			KEY( "TrueSex",     ch->pcdata->true_sex,		fread_number( fp ));
			KEY( "Trai",		ch->train,					fread_number( fp ));
			KEY( "Title",		ch->pcdata->title,			fread_string( fp ));
			KEY( "Thief_until",	ch->pcdata->thief_until,	(time_t)fread_number( fp ));			

			if (!str_cmp( word, "Trait"))
			{
				char *buf;

				buf = fread_word(fp);
				if ( is_number( buf ))
				{
					count = atoi( buf );
					ch->pcdata->trait[count]        = str_dup(fread_word(fp));
				}
				fMatch = true;
				break;
			}
				
			if ( !str_cmp( word, "TSex" ) )
			{
				char *read_buf;

				if (IS_OLD_CHVER(ch)){
					ch->pcdata->true_sex = fread_number(fp);
				}else{
					read_buf=fread_string(fp);
					ch->pcdata->true_sex =sex_lookup(read_buf);
					if(ch->pcdata->true_sex ==-1)
					{ 
						bugf("fread_char: Unfound true_sex '%s' in %s's pfile!",
							read_buf, ch->name);
						ch->pcdata->true_sex=0;
					}
				}
				fMatch = true;
				break;
			}		


			if ( !str_cmp( word, "Trying") || !str_cmp( word, "Tryin"))
			{
				int t = fread_number( fp );
				if (t==0){
					ch->is_trying_sleep= false;
				}else{
					ch->is_trying_sleep= true;
				}
				fMatch = true;
				break;
			}
			
			if ( !str_cmp( word, "Trust" )) 
			{
				int new_trust=fread_number( fp );
				if(ch->trust){
					logf("Reading in a second trust value of %d for pfile %s... ignoring!",
						new_trust, ch->name);
				}else{
					ch->trust=new_trust;
				}
				fMatch = true;
				break;
			}	
			break;
			
		case 'U':
			KEY( "Unlock_id", ch->pcdata->unlock_id, fread_string( fp ) );
			KEY( "Unsafe_4stealing",  ch->pcdata->unsafe_due_to_stealing_till, fread_number( fp ) );
			break;

		case 'V':
			KEY( "Ver_update",  ch->subversion,			fread_number ( fp )); //old
			KEY( "Version",     ch->version,			fread_number ( fp ));
			KEY( "Vers",        ch->version,            fread_number ( fp ));

			if ( !str_cmp( word, "Vnum" ) )
			{
				ch->pIndexData = get_mob_index( fread_number( fp ) );
				fMatch = true;
				break;
			}
			break;
			
		case 'W':
			KEY( "Weight",		ch->pcdata->weight,		fread_number( fp ));
			KEY( "WebPass",     ch->pcdata->webpass,    fread_string( fp ));
			KEY( "Wimpy",		ch->wimpy,				fread_number( fp ));
			KEY( "Wimp",		ch->wimpy,				fread_number( fp ));
			KEY( "Wdie",		ch->will_die,			fread_number( fp ));
			KEY( "Wiznet",		ch->wiznet[1],			fread_wordflag( wiznet_flags, fp ));
			KEY( "Wiznet2",		ch->wiznet[2],			fread_wordflag( wiznet_flags, fp ));
			KEY( "Wiznet3",		ch->wiznet[3],			fread_wordflag( wiznet_flags, fp ));
			KEY( "Wiznet0c",	ch->wiznet_colour[0],	fread_string( fp ));
			KEY( "Wiznet1c",	ch->wiznet_colour[1],	fread_string( fp ));
			KEY( "Wiznet2c",	ch->wiznet_colour[2],	fread_string( fp ));
			KEY( "Wiznet3c",	ch->wiznet_colour[3],	fread_string( fp ));
			if ( !str_cmp( word, "Wizn" )){
				if (IS_OLD_CHVER(ch)){
					ch->wiznet[1]=fread_flag( fp ) ;
				}else{ 
					ch->wiznet[1]=fread_wordflag( wiznet_flags, fp );
				}
				fMatch = true;
				break;
			}	

			if ( !str_cmp( word, "WhoFormat" )){
				char *wfmt=fread_string(fp);
				ch->pcdata->who_format=UMAX(0,who_format_lookup(wfmt));
				free_string(wfmt);
				fMatch = true;
				break;
			}	
			break;
			
		case 'X':
			KEY( "Xpen", ch->pcdata->xp_penalty, fread_number( fp ) );
			break;
		}
		
		if ( !fMatch )
		{
			bugf( "Fread_char: no match for '%s', word before that = '%s'.", 
				word, previousword);
			fread_to_eol( fp );
		}
    }
}

/**************************************************************************/
// load a pet from the forgotten reaches 
void fread_pet( char_data *ch, FILE *fp )
{
    char *word;
    char_data *pet;
    bool fMatch;
    time_t lastlogoff = current_time;
    int percent;
	bool non_existant_pet=false;
	int pet_vnum=0;
	
    // first entry had BETTER be the vnum or we barf 
	word = feof(fp) ? (char*)"END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {				
		pet_vnum= fread_number(fp);
		if (get_mob_index(pet_vnum) == NULL)
		{
			bugf("Fread_pet: bad vnum %d.",pet_vnum);
			non_existant_pet=true;
			pet = create_mobile(limbo_mob_index_data, 0);
		}else{
			pet = create_mobile(get_mob_index(pet_vnum), 0);
		}
    }
    else
    {
		bug("Fread_pet: no vnum in file.");
		pet = create_mobile(limbo_mob_index_data, 0);
    }
    
    for ( ; ; )
    {
		word    = feof(fp) ? (char*)"END" : fread_word(fp);
		fMatch = false;
		
		switch (UPPER(word[0]))
		{
		case '*':
			fMatch = true;
			fread_to_eol(fp);
			break;
			
		case 'A':
			KEY( "Act",         pet->act,               fread_flag(fp));
			KEY( "AfBy",        pet->affected_by,       fread_flag(fp));
			KEY( "AffBy",       pet->affected_by,       fread_wordflag(affect_flags, fp));
			KEY( "AffBy2",      pet->affected_by2,      fread_wordflag(affect2_flags,fp));			
			KEY( "Alig",        pet->alliance,          3*fread_number(fp)/1000);
			
			if (!str_cmp(word,"Algn"))
			{
				pet->tendency=fread_number(fp);
				pet->alliance=fread_number(fp);
				fMatch = true;
				break;
			}   
			
			if (!str_cmp(word,"ACs"))
			{
				int i;
				
				for (i = 0; i < 4; i++)
					pet->armor[i] = fread_number(fp);
				fMatch = true;
				break;
			}
				
			if (!str_cmp(word,"Affc"))
			{
				AFFECT_DATA *paf;
				int sn;
				
				paf = new_affect();
				
				sn = skill_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.");
				else
					paf->type = sn;
				
				paf->where      = fread_number(fp);
				paf->level      = fread_number(fp);
				paf->duration   = fread_number(fp);
				paf->modifier   = fread_number(fp);
				paf->location   = translate_old_apply_number(fread_number(fp));
				paf->bitvector  = fread_number(fp);
				paf->next       = pet->affected;
				pet->affected   = paf;
				fMatch          = true;
				break;
			}
			
			if (!str_cmp(word,"AMod"))
			{
				int stat;
				if(ch->version>8)
				{
					for (stat = 0; stat < MAX_STATS; stat++)
						pet->modifiers[stat] = fread_number(fp);
				}
				fMatch = true;
				break;
			}
			
			if (!str_cmp(word,"Attr"))
			{
				int stat;
				if(ch->version>8)
				{
					for (stat = 0; stat < MAX_STATS; stat++)
						pet->perm_stats[stat] = fread_number(fp);
				}
				fMatch = true;
				break;
			}
			break;
			
		case 'C':
			KEY( "Clan",       pet->clan,       clan_slookup(fread_string(fp)));
			KEY( "Comm",       pet->comm,              fread_flag(fp));			
			KEY( "Com",        pet->comm,              fread_wordflag(comm_flags, fp));						
			break;
			
		case 'D':
			KEY( "Dam",        pet->damroll,           fread_number(fp));
			KEY( "Desc",       pet->description,       fread_string(fp));
			break;
			
		case 'E':
			if ( !str_cmp( word, "Events" ) ){
				ch->load_events(fp);
				fMatch = true;
				break;
			}

			if (!str_cmp(word,"End"))
			{
				if(non_existant_pet){
					logf("Removing pet from %s, because the vnum of it couldn't be located.", PERS(ch, NULL));
					ch->println("`R********************************************************************************");
					ch->printlnf("`R*`W Your pet vnum %5d couldn't be located for loading, contact the admin. `R*", pet_vnum);
					ch->println("`R********************************************************************************`x");
				}else{
					pet->leader = ch;
					pet->master = ch;
					ch->pet = pet;
					/* adjust hp mana move up  -- here for speed's sake */
					percent = (int)(current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
					
					if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
						&&  !IS_AFFECTED(ch,AFF_PLAGUE))
					{
						percent = UMIN(percent,100);
						pet->hit    += (pet->max_hit - pet->hit) * percent / 100;
						pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
						pet->move   += (pet->max_move - pet->move)* percent / 100;
					}
				}
				return;
			}
			KEY( "Exp",        pet->exp,               fread_number(fp));
			break;
			
		case 'G':
			KEY( "Gold",       pet->gold,              fread_number(fp));
			break;
			
		case 'H':
			KEY( "Hit",        pet->hitroll,           fread_number(fp));
			
			if (!str_cmp(word,"HMV"))
			{
				pet->hit        = fread_number(fp);
				pet->max_hit    = fread_number(fp);
				pet->mana       = fread_number(fp);
				pet->max_mana   = fread_number(fp);
				pet->move       = fread_number(fp);
				pet->max_move   = fread_number(fp);
				fMatch = true;
				break;
			}
			break;
			
		case 'L':
			KEY( "Levl",       pet->level,             fread_number(fp));
			KEY( "LnD",        pet->long_descr,        fread_string(fp));
			KEY( "LogO",       lastlogoff,             fread_number(fp));
			break;
		
		case 'M':
			KEY( "MAct",       pet->act,              fread_wordflag(act_flags,fp));
			break;

		case 'N':
			KEY( "Name",       pet->name,              fread_string(fp));
			break;
			
		case 'P':
			KEY( "Pos",        pet->position,          fread_number(fp));
			
		case 'R':
			//	    KEY( "Race",        pet->race, race_lookup(fread_string(fp)));
			if ( !str_cmp( word, "Race" ) )
			{
				ch->race = race_lookup(fread_string( fp ));
				if (ch->race == -1)
				{
					ch->race = race_lookup("human");
					bugf("fread_pet: pet '%s' has an invalid race!!!",
						ch->name);
				}
				fMatch = true;
			}
			
			
			if ( !str_cmp( word, "Room" ) )
			{
				pet->in_room = get_room_index( fread_number( fp ) );
				if ( pet->in_room == NULL )
				{
					pet->in_room = get_room_index( ROOM_VNUM_LIMBO );
				}
				fMatch = true;
			}
			
			break;
			
		case 'S' :
			KEY( "Save",        pet->saving_throw,      fread_number(fp));
			KEY( "Sex",         pet->sex,               fread_number(fp));
			KEY( "ShD",         pet->short_descr,       fread_string(fp));
			KEY( "Silv",        pet->silver,            fread_number( fp ) );
			break;
			
			if ( !fMatch )
			{
				bug("Fread_pet: no match.");
				fread_to_eol(fp);
			}
			
		}
    }
}


/**************************************************************************/
// return the object/container
obj_data * fread_obj( FILE *fp, const char *filename )
{
	obj_data *obj;
	char *word;
	int iNest;
	bool fMatch;
	bool fNest;
	bool fVnum;
	bool first;
	char obj_rembuf[MSL];
	
	int vnum=0;
    
    fVnum = false;
    obj = NULL;
    first = true;  // used to counter fp offset
	
    word   = feof( fp ) ? (char*)"End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
		first = false;  // fp will be in right place
		
		vnum = fread_number( fp );
		if (  get_obj_index( vnum )  == NULL )
		{
			bugf( "Fread_obj: bad vnum %d.", vnum );
			dont_nest=true;
		}
		else
		{
			obj = create_object(get_obj_index(vnum));
		}
		
    }
	
    if(!obj){  // object not found
		obj = new_obj();
		obj->name               = str_dup( "" );
		obj->short_descr        = str_dup( "" );
		obj->description        = str_dup( "" );
    }
	
    fNest	= false;
	fVnum	= true;
	iNest	= 0;
	
    for ( ; ; )
    {
		if (first){
			first = false;
		}else{
			word   = feof( fp ) ? (char*)"End" : fread_word( fp );
		}
		fMatch = false;
		
		switch ( UPPER(word[0]) )
		{
		case '*':
			fMatch = true;
			fread_to_eol( fp );
			break;
			
		case 'A':
			if (!str_cmp(word, "Affect"))
			{
				AFFECT_DATA *paf=fread_affect(fp);
				paf->next       = obj->affected;
				obj->affected   = paf;
				fMatch = true;
				break;
			}
			
			if (!str_cmp(word,"Affc"))
			{
				AFFECT_DATA *paf;
				int sn;
				
				paf = new_affect();
				
				sn = skill_lookup(fread_word(fp));
				if (sn < 0){
					bug("Fread_obj: unknown skill.");
				}else{
					paf->type = sn;
				}
				
				paf->where      = fread_number( fp );
				paf->level      = fread_number( fp );
				paf->duration   = fread_number( fp );
				paf->modifier   = fread_number( fp );
				paf->location   = translate_old_apply_number(fread_number( fp ));
				paf->bitvector  = fread_number( fp );
				paf->next       = obj->affected;
				obj->affected   = paf;
				fMatch          = true;
				break;
			}
			if (!str_cmp(word, "Attune" ))
			{
				obj->attune_id			= fread_number( fp );
				obj->attune_flags		= fread_number( fp );
				obj->attune_modifier	= fread_number( fp );
				obj->attune_next		= fread_number( fp );
				fMatch					= true;
				break;
			}
			break;
			
		case 'C':
			KEY( "Cond",        obj->condition,         fread_number( fp ) );
			KEY( "Cost",        obj->cost,              fread_number( fp ) );
			if( !str_cmp( word, "Chaos"))
			{
				obj->chaos=true;
				fMatch=true;
				break ;
			}
			break;
			
		case 'D':
			KEY( "Description", obj->description,       fread_string( fp ) );
			KEY( "Desc",        obj->description,       fread_string( fp ) );
			break;
			
		case 'E':

			if ( !str_cmp( word, "Events" ) ){
				obj->load_events(fp);
				fMatch = true;
				break;
			}
			
			if ( !str_cmp( word, "Enchanted"))
			{
				//				obj->enchanted = true; // removed - kal
				fMatch  = true;
				break;
			}
			
			KEY( "ExtraFlags",  obj->extra_flags,       fread_number( fp ) );
			KEY( "ExtF",        obj->extra_flags,       fread_number( fp ) );
			KEY( "ExtF2",       obj->extra2_flags,       fread_number( fp ) );
			
			if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
			{
				EXTRA_DESCR_DATA *ed;
				
				ed = new_extra_descr();
				
				ed->keyword             = fread_string( fp );
				ed->description         = fread_string( fp );
				// chop off the . if it starts with that 
				if (ed->description[0]=='.')
				{
					char *temp_string;
					temp_string = str_dup(ed->description);
					free_string(ed->description);
					ed->description= str_dup(temp_string+1);
					free_string(temp_string);
				}
				ed->next                = obj->extra_descr;
				obj->extra_descr        = ed;
				fMatch = true;
			}
			
			if ( !str_cmp( word, "End" ) )
			{
				if ( !fNest || !fVnum || obj->pIndexData == NULL)
				{				
					bug("Fread_obj: incomplete object.");
					extract_obj(obj);
					sprintf(obj_rembuf, "fread() object %d didn't exist in %s",	vnum, filename);
					append_datetimestring_to_file(OBJ_REM_FILE, obj_rembuf);
					return NULL;
				}
				else
				{
					// TOKENS & Quitdeath
					if (!hotreboot_in_progress
						&& (obj->item_type == ITEM_TOKEN)
						&& (IS_SET ( obj->value[0], TOKEN_QUITDEATH )))
					{
						logf("Not hotreboot - removing token %d marked as quitdeath", vnum);
						extract_obj(obj);
						return NULL;
					}
					
					if (!hotreboot_in_progress && obj->item_type == ITEM_KEY)
					{
						logf("Not hotreboot - removing key vnum %d", vnum);
						extract_obj(obj);
						return NULL;
					}
					
					if(obj->pIndexData->vnum==OBJ_VNUM_LIGHT_BALL){
						if(obj->value[2]==-1 || obj->value[2]>1000){
							// continual light - prevent it going on for ever
							obj->value[2]=1000;
						}
					}
					
					if (hotreboot_in_progress && obj->item_type == ITEM_KEY)
					{  
						logf("hotreboot - giving them key/map vnum %d", vnum);
					}
					
					// if we haven't found a vnum, dynamically create the object
					if ( !fVnum ){
						free_string( obj->name        );
						free_string( obj->description );
						free_string( obj->short_descr );
						obj->next = obj_free;
						obj_free  = obj;						
						obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ));
					}
					
					if ( iNest != 0 && rgObjNest[iNest]){
						if (rgObjNest[iNest-1]->pIndexData){
							obj_to_obj( obj, rgObjNest[iNest-1]);
							obj=NULL;
						}else{
							bugf("Fread_obj: object vnum %d couldn't be loaded into parent container!", vnum);
							
							sprintf(obj_rembuf, "object %d couldn't be loaded into parent container! (%s)",
								vnum, filename);
							append_timestring_to_file(OBJ_REM_FILE, obj_rembuf);

						}
					}			
					return obj;
				}
			}
			break;
			
		case 'I':
			KEY( "ItemType",       obj->item_type,         fread_number( fp ) );
			KEY( "Ityp",   obj->item_type,         fread_number( fp ) );
			break;
			
		case 'L':
			KEY( "Level",  obj->level,             fread_number( fp ) );
			KEY( "Lev",            obj->level,             fread_number( fp ) );
			break;
			
		case 'N':
			KEY( "Name",   obj->name,              fread_string( fp ) );
			
			if ( !str_cmp( word, "no_affects"))
			{
				obj->no_affects = true;
				fMatch  = true;
				break;
			}
			
			if ( !str_cmp( word, "Nest" ) )
			{
				iNest = fread_number( fp );
				if (( iNest < 0 || iNest >= MAX_NEST )&&(dont_nest==true))
				{
					bugf( "Fread_obj: bad nest %d.", iNest );
				}
				else
				{
					rgObjNest[iNest] = obj;
					fNest = true;
				}
				fMatch = true;
			}
			break;

		case 'P': 
			if( !str_cmp( word, "ParchmentLanguage"))
			{			
				obj->value[3] = language_safe_lookup(fread_word(fp))->unique_id;
				fMatch=true;
				break ;
			}
			break;
			
		case 'R': // restrung objects - Kalahn June 98
			if( !str_cmp( word, "RESTRUNG"))
			{
				obj->restrung=true;
				fMatch=true;
				fread_to_eol( fp );
				break ;
			}
			break;
			
		case 'S':
			KEY( "ShortDescr",  obj->short_descr,       fread_string( fp ) );
			KEY( "ShD",         obj->short_descr,       fread_string( fp ) );
			
			if ( !str_cmp( word, "Spell" ) )
			{
				int iValue;
				int sn;
				
				iValue = fread_number( fp );
				sn     = skill_lookup( fread_word( fp ) );
				if ( iValue < 0 || iValue > 4 )
				{
					bugf( "Fread_obj: bad iValue %d on object %d.", 
						iValue, obj->pIndexData?obj->pIndexData->vnum:0 );
				}
				else if ( sn < 0 )
				{
					bug("Fread_obj: unknown skill.");
				}
				else
				{
					obj->value[iValue] = sn;
				}
				fMatch = true;
				break;
			}
			
			break;
			
		case 'T':
			KEY( "Timer",       obj->timer,             fread_number( fp ) );
			KEY( "Time",        obj->timer,             fread_number( fp ) );
			break;
			
		case 'V':
			if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
			{
				obj->value[0]   = fread_number( fp );
				obj->value[1]   = fread_number( fp );
				obj->value[2]   = fread_number( fp );
				obj->value[3]   = fread_number( fp );
				if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0){
					obj->value[0] = obj->pIndexData->value[0];
				}
				if(obj->item_type==ITEM_PARCHMENT){
					obj->value[3]=language_lookup("unknown")->unique_id;
				}
				fMatch          = true;
				break;
			}
			
			if ( !str_cmp( word, "Val" ) )
			{
				obj->value[0]   = fread_number( fp );
				obj->value[1]   = fread_number( fp );
				obj->value[2]   = fread_number( fp );
				obj->value[3]   = fread_number( fp );
				obj->value[4]   = fread_number( fp );
				fMatch = true;
				break;
			}
			
			if ( !str_cmp( word, "Vnum" ) )
			{
				int vnum;
				
				vnum = fread_number( fp );
				if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL ){
					bugf( "Fread_obj: bad vnum %d.", vnum );
				}else{
					fVnum = true;
				}
				fMatch = true;
				break;
			}
			break;
			
		case 'W':
			KEY( "WearFlags",   obj->wear_flags,        fread_number( fp ) );
			KEY( "WeaF",        obj->wear_flags,        fread_number( fp ) );
			KEY( "WearLoc",     obj->wear_loc,          fread_wordflag(wear_location_types, fp ));
			KEY( "Wear",        obj->wear_loc,          fread_number( fp ) );
			KEY( "Weight", obj->weight,            fread_number( fp ) );
			KEY( "Wt",          obj->weight,            fread_number( fp ) );
			break;
			
		}
		
		if ( !fMatch )
		{
			bugf("Fread_obj: no match for '%s'.", word);
			fread_to_eol( fp );
		}
	}
}
/**************************************************************************/
