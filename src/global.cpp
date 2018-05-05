/**************************************************************************/
// global.cpp - Global variable system details see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: global.cpp - To add a global just add a SINGLE entry into 
 *        global.h prefixed with the word EXTERN (all uppercase)... 
 *        compiler macros do the rest.
 ***************************************************************************/
 
#define _GLOBAL_C
#include "include.h"
#include "colour.h"
#include "nanny.h"

// redefine the EXTERN from "extern" to  nothing so variables are stored
// in here instead of declared externally in here like all other modules
void init_map_tables();
AREA_DATA *get_vnum_area( int vnum );
MOB_INDEX_DATA *new_mob_index( void );

#undef  EXTERN
#define EXTERN
#undef  INIT_GLOB
#define INIT_GLOB

#include "global.h"
#include "channels.h"
/**************************************************************************/
void race_allocate_race_table();
void init_mm( );
void do_save_gamesettings(char_data *ch, char *);

/**************************************************************************/
void init_globals(char *exename)
{
	int i;

	// record the executables file name
	strcpy(EXE_FILE, exename); 
	
	init_mm(); // Init random number generator.

	// update the mud name if it is the default
	if( !str_cmp(MUD_NAME, DEFAULT_MUD_NAME ) || IS_NULLSTR(MUD_NAME)){
		replace_string(MUD_NAME, FORMATF("%s on %s", DEFAULT_MUD_NAME, MACHINE_NAME));
		do_save_gamesettings(NULL, ""); // resave so the gamename can be manually edited
	}

	channels_initialize();

	// assume resolver isn't running
	resolver_running=false;
	resolver_version=0;
	// record the boottime
	update_currenttime();
    strcpy( str_boot_time, ctime( &current_time ) );
    boot_time = current_time;
	lastreboot_time = current_time;

	hotrebootmaxon		= 0;
	hotrebootmaxon_time = current_time;
	maxon_time			= current_time;

    max_on              = 0;
    true_count          = 0;
    tick_counter        = 0;
	resaveCounter		= 5; // first auto admin save 5 mins after reboot
    note_notify_counter = -1; // notification of new mail
    MOBtrigger          = true;



    // initialise the moots
    moot = (MOOT_DATA *) alloc_mem(sizeof(MOOT_DATA));
	moot->called_by=NULL;
	moot->moot_victim=NULL;
	moot->moot_victim_name=str_dup("");
	moot->moot_type=0;
	moot->scope=0;
	moot->votes_for=0;
	moot->votes_against=0;
    moot->number_of_votes=0;
	moot->pulse=0;

	// moon time of year for casting
	// - at this stage random will be saved in future
	moon_day = number_range(1,28);	// 1->28
	moon_month = number_range(1, 12);	// 1-12
	//get_moon_cast_modifier();

	// linked list of all pMobIndex records
	pMobIndexlist=NULL;

	// initialise the map lookup tables
	init_map_tables();

	// initialise the immtalk buffers 
	for(i=0; i<MAX_CHECK_IMMTALK; i++){
		check_immtalk_replay_text[i][0]='\0';
	}	
	check_immtalk_replay_index=1;

	// initialise the inputtail buffers 
	for(i=0; i<MAX_INPUTTAIL; i++){
		inputtail[i][0]='\0';
	}	
	inputtail_index=1;

	// debug system
	DEBUG_ROOM=NULL;
	DEBUG_MOB=NULL;
	DEBUG_OBJECT=NULL;

	// init the help_category_types index and set all setupable to true
	for(i=0; i<MAX_HELP_CATEGORIES; i++){
		help_category_types[i].bit=i;
		help_category_types[i].settable=true;
	}	

	EXECUTING_SOCIAL=false;
	RECORD_TO_REPLAYROOM=false;

	process_colour(" ", NULL); // trigger the memory allocation
}

/**************************************************************************/
void init_static_characters()
{
	//do chImmortal
	char_data * ch;
	ch = new_char();
	ch->pcdata = new_pcdata();

	ch->desc				= connection_allocate();
    ch->desc->connected_state= CON_WEB_REQUEST;
    ch->desc->character     = ch;
	ch->desc->colour_mode	= CT_HTML;
	ch->name				= str_dup( "chImmortal");
	ch->player_id			= get_pc_id();
	ch->race				= race_lookup("human");
	ch->act 				= PLR_NOSUMMON;
	ch->comm				= COMM_COMBINE | COMM_PROMPT;
	ch->prompt				= str_dup("");
	ch->mounted_on			=NULL;
	ch->ridden_by			=NULL;
	ch->tethered			=false;
	ch->bucking 			=false;
	ch->wildness			=100;
	ch->will				=100;
	ch->lines				=0;
	ch->pcdata->confirm_delete	= false;
	ch->pcdata->pwd 		= str_dup( "" );
	ch->pcdata->bamfin		= str_dup( "" );
	ch->pcdata->bamfout 	= str_dup( "" );
	for (int stat =0; stat < MAX_STATS; stat++){
		ch->perm_stats[stat]= 1;
	}
	ch->pcdata->condition[COND_THIRST]	= 48;
	ch->pcdata->condition[COND_FULL]	= 48;
	ch->pcdata->condition[COND_HUNGER]	= 48;

	ch->in_room= get_room_index(ROOM_VNUM_LIMBO);
	
	chImmortal=ch;

	// initialise the colour conversion systems
	initColour();

}

/**************************************************************************/
void init_limbo_mob_index_data()
{
	limbo_mob_index_data= new_mob_index();
    limbo_mob_index_data->vnum	= 1;
    limbo_mob_index_data->area	= get_vnum_area( ROOM_VNUM_LIMBO);    
    limbo_mob_index_data->act= ACT_IS_NPC | ACT_STAY_AREA | ACT_NO_TAME;
	limbo_mob_index_data->xp_mod		= 100; // set to 100% default
    limbo_mob_index_data->hit[DICE_BONUS]	 = 1;
    limbo_mob_index_data->mana[DICE_BONUS]	 = 1;
    limbo_mob_index_data->damage[DICE_BONUS] = 1;
    // set the flags for the race
	int race = race_lookup( "human");
	if ( race != -1 ){
		limbo_mob_index_data->race = race;
		limbo_mob_index_data->act			|= race_table[race]->act;
		limbo_mob_index_data->affected_by |= race_table[race]->aff;
		limbo_mob_index_data->off_flags   |= race_table[race]->off;
		limbo_mob_index_data->imm_flags   |= race_table[race]->imm;
		limbo_mob_index_data->res_flags   |= race_table[race]->res;
		limbo_mob_index_data->vuln_flags  |= race_table[race]->vuln;
		limbo_mob_index_data->form        |= race_table[race]->form;
		limbo_mob_index_data->parts       |= race_table[race]->parts;	
    }
    limbo_mob_index_data->next			= NULL;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
