/**************************************************************************/
// db.cpp - reads in areas, logging code etc
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

#ifdef WIN32
#ifndef OLD_RAND
#define OLD_RAND
#endif
#include <direct.h>
#endif

#include "areas.h"
#include "db.h"
#include "clan.h"
#include "olc.h"
#include "interp.h"
#include "colour.h"
#include "support.h"
#include "track.h"
#include "ictime.h"
#include "help.h"
#include "lockers.h"
#include "shop.h"

#ifdef WIN32
#include "process.h"
#endif

#ifndef OLD_RAND
	int getpid();
	time_t time(time_t *tloc);
#endif


/* externals for counting purposes */
extern  OBJ_DATA        *obj_free;
extern  char_data       *char_free;
extern  PC_DATA         *pcdata_free;
extern  AFFECT_DATA     *affect_free;

/*
 * Globals.
 */
SHOP_DATA *             shop_first;
cInnData*				pFirstInn;			// Inns for recalling to.

MUDPROG_CODE *			mudprog_list;

char                    bug_buf         [2*MIL];
char_data *             char_list;
char                    log_buf         [2*MIL];
KILL_DATA               kill_table      [MAX_LEVEL];
OBJ_DATA *              object_list;
TIME_INFO_DATA          time_info;
WEATHER_DATA            weather_info	[SECT_MAX];

void log_area_import_format_notice();
void do_read_skillgroups(char_data *ch, char *);
// prototype from in global.cpp
void init_static_characters();
GAMBLE_FUN *gamble_lookup( const char *name );
DECLARE_DO_FUN( do_read_commandtable);
/*
 * Locals.
 */
MOB_INDEX_DATA *        mob_index_hash          [MAX_KEY_HASH];
OBJ_INDEX_DATA *        obj_index_hash          [MAX_KEY_HASH];
ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH];
char *                  string_hash             [MAX_KEY_HASH];
void sort_arealists();
void check_tables();

char *                  string_space=NULL;
char *                  top_string;
char                    str_empty       [1];

int                     top_affect;
int                     top_area;
int						top_deity;
int                     top_ed;
int                     top_exit;
int                     top_help;
int                     top_mob_index;
int                     top_obj_index;
int                     top_reset;
int                     top_room;
int                     top_shop;
int						top_inn;				// Like top_shop, but for inns.
int                     top_vnum_room;
int                     top_vnum_mob;
int                     top_vnum_obj;
int						top_mprog_index;
int                     mobile_count = 0;

int dbVersion;

#define                 MAX_PERM_BLOCK  131072

int                     nAllocString;
int                     sAllocString;
int                     nAllocPerm;
int                     sAllocPerm;

/*
 * Semi-locals.
 */
bool			fBootDb;
FILE			*fpArea;
char			strArea[MIL];
long			last_vnum;

/*
 * Local booting procedures.
*/
void    init_mm				args( ( void ) );
void    load_area			( FILE *fp, bool dawnareadata );
void	load_arearom		( FILE *fp );
void    load_helps			args( ( FILE *fp ) );
void    load_old_mob		args( ( FILE *fp ) );
void    load_mobiles		( FILE *fp, int version );
void    load_old_obj		args( ( FILE *fp ) );
void	load_objects		( FILE *fp, int version );
void    load_resets			args( ( FILE *fp, int resets_version, int area_version ) );
void    load_rooms			( FILE *fp, int version );
void    load_shops			args( ( FILE *fp ) );
void    oldload_socials		args( ( FILE *fp ) );
void    load_specials		args( ( FILE *fp ) );
void	load_gamble			args( ( FILE *fp ) );
void	load_attunes		args( ( FILE *fp ) );
void    load_notes			args( ( void ) );
void    load_bans			args( ( void ) );
void    load_mudprogs		args( ( FILE *fp ) );
void    laston_load			args( ( void ) );

void    fix_exits			args( ( void ) );
void    fix_mudprogs		args( ( void ) );
void    fix_resets			args( ( void ) );
void    reset_area			args( ( AREA_DATA * pArea ) );
void	room_update( AREA_DATA *pArea );

void    load_quest_db		args( ( void ) );
void	load_mix_db			args( ( void ) );
void	load_script_db		args( ( void ) );
void	load_deity_db		args( ( void ) );
void	load_herb_db		args( ( void ) );
void	load_offmoot_db args( ( void ) );

void	import_helps( FILE *fp );

/**************************************************************************/
void save_clsses();
void load_clsses();
void do_read_classes(char_data *ch, char *argument);
/**************************************************************************/
void init_string_space()
{
	if ( ( string_space = (char *) calloc( 1, MAX_STRING ) ) == NULL )
	{
		bugf( "Boot_db: can't alloc %d string space.", MAX_STRING );
		exit_error( 1 , "init_string_space", "failed to allocate memory");
	}
	top_string      = string_space;
	if(MAX_STRING>90000){
		logf("Allocated %d bytes for string storage.  (MAX_STRING setting)", 
			MAX_STRING);
	}
}
/**************************************************************************/
void load_races();
void load_socials();
void load_autostat_files();
void load_intro_database();
void init_limbo_mob_index_data();
void do_load_corpses(char_data *ch, char *);
void race_convert_skills();
void colour_convert_area( AREA_DATA *area);
void load_continents();
void save_continents();

/**************************************************************************/
// read in basically everything
// intro_db, languages, classes, clans, races, skills, commands, areas
// lockers, corpses, letgain db, quest db, mixes, script config,
// herbs, offline mooting db, name generator profiles...
void boot_db()
{
	char buf[MSL];
	char strfname[MSL];
	bool socialnew  = true;
	fBootDb         = true;

	load_intro_database();
	load_autostat_files();

	set_ictime();
	set_weather();
	
	// read in the languages 
	languages_load_and_initialise(); update_currenttime();

	// read in the core system tables
	load_clan_db( );				update_currenttime();

	do_read_classes(NULL,"");		update_currenttime();
	load_races();
	do_loadskilltable(NULL,"");		update_currenttime();
	race_convert_skills();			update_currenttime();
	do_read_commandtable(NULL,"");	update_currenttime();

	init_track_table();

	// read in the skillgroups, if no skillgroups file exists
	// then convert 'spell names' to skills gsn's in group_table[]
	// and write a skill groups file
	do_read_skillgroups(NULL, "");

	load_continents();				update_currenttime();


	language_init_gsn_and_unique_id();

	// check all hard coded tables
	check_tables();					update_currenttime();		
	
	/*
	* Read in all the area files.
	*/
    /**************************/
    /* read in the AREA files */
	update_currenttime();		
	{
		FILE *fpList;
		area_first = NULL;
		
		fulltime_log_string("Opening area list file " AREA_LIST );
		
		if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
		{
			bugf("boot_db(): fopen '%s' failed for read - error %d (%s)",
				AREA_LIST, errno, strerror( errno));
			log_note("File " AREA_LIST " not found... this file contains the list "
				"of all the area files to be read in and the mud can not complete its "
				"bootup process if this file is missing.`1"
				"`1"
				"It is possible to create an empty version of this file so the mud can "
				"boot by typing 'echo $>" AREA_LIST "' (without the quotes) - Because dawn "
				"automatically generates a 'limbo' room at bootup if one doesn't exist, "
				"it is possible to boot the mud with no area files and successfully login.`1"
				"`1"
				"If you are importing a complete set of area files from another mud "
				"(such as rom), it is some times possible to copy the area list "
				"used by that mud to the filename listed above.");
			exit_error( 1 , "boot_db", "fopen for read of arealist failed");
		}
#ifdef WIN32	// turn off read in buffering so ftell() reports the correct position
		if(setvbuf( fpList, NULL, _IONBF, 0 )){
			bugf("boot_db(): error setting setvbuf( fpList, NULL, _IONBF, 0 ) on '%s' - error %d (%s).",
				AREA_LIST, errno, strerror( errno));
		}
#endif

		
		for ( ; ; )
		{

			// any line in arealist.txt starting with a $ indicates 
			// the end of the arealist
			char letter=fread_letter(fpList);
			if ( letter == '$' ){
					break;
			}else{
				ungetc( letter, fpList );
			}

			strcpy( strfname, fread_word( fpList ) );
			
			// prefix the filename with the default area directory
			sprintf(strArea, "%s%s", AREA_DIR, strfname);
			
			if (!fBootTestOnly)
			{
				sprintf(buf," Reading in area %s",strArea);
				log_string( buf );
			}
			
			if ( strArea[0] == '-' )
			{
				fpArea = stdin;
			}
			else
			{
				if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
				{
					bugf("boot_db(): fopen '%s' failed for read - error %d (%s)",
						strArea, errno, strerror( errno));
					exit_error( 1 , "boot_db", "fopen for read of areafile failed");
				}
#ifdef WIN32	// turn off read in buffering so ftell() reports the correct position
				if(setvbuf( fpArea, NULL, _IONBF, 0 )){
					bugf("boot_db(): error setting setvbuf( fpArea, NULL, _IONBF, 0 ) for '%s' - error %d (%s).",
						strArea, errno, strerror( errno));
				}
#endif
			}
			
			for ( ; ; )
			{
				char *word;
				
				if(fread_letter(fpArea)!='#')
				{
					bug("Boot_db: # sign not found.");
					exit_error( 1 , "boot_db", "# not found");
				}
				
				word = fread_word( fpArea );
				
				if ( word[0] == '$'               )                 break;
				else if ( !str_cmp( word, "DAWNAREADATA" ) ) load_area(fpArea, true);
				else if ( !str_cmp( word, "AREADATA" ) ) load_area(fpArea, false);
				else if ( !str_cmp( word, "AREA" ) ) load_arearom(fpArea); // rom format
				else if ( !str_cmp( word, "MOBILES"  ) )
				{
					if (dbVersion < 11 ){
						load_mobiles(fpArea, dbVersion);
					}else{
						load_mobiles_NAFF(fpArea, dbVersion);
					}
				}
				else if ( !str_cmp( word, "MUDPROGS" ) 
						||!str_cmp( word, "MOBPROGS" ) ) {
					if (dbVersion < 11 ){
						load_mudprogs(fpArea);
					}else{
						load_mudprogs_NAFF(fpArea);
					}										
				}
				else if ( !str_cmp( word, "OBJECTS"  ) )
				{
					if(dbVersion < 11 ){
						load_objects(fpArea, dbVersion); 
					}else{
						load_objects_NAFF(fpArea, dbVersion);
					}

				}
				else if ( !str_cmp( word, "RESETS"   ) ) load_resets(fpArea, 1, dbVersion);
				else if ( !str_cmp( word, "RESETS2"  ) ) load_resets(fpArea, 2, dbVersion);
				else if ( !str_cmp( word, "ROOMS"    ) )
				{
					if (dbVersion < 11 ){
						load_rooms(fpArea, dbVersion);
					}else{
						load_rooms_NAFF(fpArea, dbVersion);
					}
				}
				else if ( !str_cmp( word, "SHOPS"    ) )
				{ 
					if (dbVersion < 11 ){
						load_shops(fpArea);
					}else{
						load_shops_NAFF(fpArea);
					}
				}
				else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(fpArea);
				else if ( !str_cmp( word, "GAMBLE"	 ) ) load_gamble(  fpArea);
				else if ( !str_cmp( word, "ATTUNE"	 ) ) load_attunes( fpArea);
				else
				{
					bool exiting=true;
					bugf( "Boot_db: bad section name for an area file '#%s'.", word );


					if(!str_cmp(word, "HELPS")){
						char helpfilename[MIL];
						sprintf(helpfilename, "%s%s0.txt", BACKUP_HELP_DIR, strfname);
						if(AREA_IMPORT_FLAG(AREAIMPORTFLAG_IGNORE_HELPS_IN_AREAFILES)){
							for(int hi=0; file_exists(helpfilename); hi++){
								sprintf(helpfilename, "%s%s%d.txt", BACKUP_HELP_DIR, strfname, hi);
							}

							logf("#HELPS sections encountered... ignoring entire section\r\n"
								"(saving ignored text to '%s'):", helpfilename);
							append_string_to_file(helpfilename, "#HELPS", true);
							while(true){
								char *dumptext=fread_string(fpArea);
								if(strcmp("-1 $", dumptext)){
									append_string_to_file(helpfilename, fix_string(dumptext), true);
									append_string_to_file(helpfilename,"~", true);
								}else{
									append_string_to_file(helpfilename, "-1 $~", true);
									logf("Found end of help marker (-1 $~)... leaving ignore help mode.");
									break;
								}
							}
							exiting=false;
						}else{
							log_notef(
								"This mud does not store help entries within areafiles, "
								"instead they are stored in one or more dedicated helpfiles."
								"`1"
								"`1"
								"A #HELPS section has been encounted within %s."
								"`1- There are three options available to handle this:"
								"`11.  Manually edit the listed area file and remove the entire #HELPS section."
								"`12.  Don't load the area at all (manually remove the area filename from "
									   "the arealist file '%s')."
								"`13.  Add the 'ignore_helps_in_areafiles' flag directly after the "
								"area_import_flags keyword in the game settings file (gameset.txt)."
								"`1`1If you add the flag described in option 3, all helps within all "
								"area files will be ignored.  The help contents will be "
								"written to a file with a name in the format '%s' "
								"(based on area filename, and the number may vary).`1",
								strArea, AREA_LIST, helpfilename);
						}
					}

					if(!str_cmp(word, "SOCIALS")){
						bugf("\r\n"
							 "#SOCIALS sections can't be loaded in from files listed in arealist.txt\r\n"
							 "Normally socials are stored in their own socials file... you can however\r\n"
							 "import socials by listing the file containing a series of socials in\r\n"
							 "helplist.txt then use the social_import command.\r\n"
							 "The prefered way of creating/editing socials is using socedit (olc cmd).\r\n" );
					}			

					if(exiting){
						exit_error( 1 , "boot_db", "exiting due to previous error");
					}
				}               
			}

			if ( fpArea != stdin ){
				fclose( fpArea );
			}
			fpArea = NULL;

			// end of a single area, read in room invite list if one exists for this area
			if(area_last){
				load_area_roominvitelist(area_last);
			}
						
			// transpose the area min and max vnums
			// also convert the colour code if appropriate
			if(area_last){
				area_last->min_vnum+=area_last->vnum_offset;
				area_last->max_vnum+=area_last->vnum_offset;
				area_last->vnum_offset=0;

				colour_convert_area(area_last);
			}

		}
		fclose( fpList );
		update_currenttime();
		/* end of reading in the AREA files */
		/************************************/
		
		
		/*************************************/
		/* read in the HELP and SOCIAL files */
		update_currenttime();		
		load_socials();
		update_currenttime();		
		fulltime_log_string("Opening help list file " HELP_LIST );
		if ( ( fpList = fopen( HELP_LIST, "r" ) ) == NULL )
		{
			bugf("boot_db(): fopen '%s' failed for read - error %d (%s)",
				HELP_LIST, errno, strerror( errno));
			exit_error( 1 , "boot_db", "fopen for read of helplist failed");
		}
#ifdef WIN32	// turn off read in buffering so ftell() reports the correct position
		if(setvbuf( fpList, NULL, _IONBF, 0 )){
			bugf("boot_db(): error setting setvbuf( fpList, NULL, _IONBF, 0 ) for '%s' - error %d (%s)",
				HELP_LIST, errno, strerror( errno));
		}
#endif
		
		for ( ; ; )
		{
			// any line in arealist.txt starting with a $ indicates 
			// the end of the arealist
			char letter=fread_letter(fpList);
			if ( letter == '$' ){
					break;
			}else{
				ungetc( letter, fpList );
			}

			strcpy( strfname, fread_word( fpList ) );
			
			// prefix the filename with the default help directory 
			strcpy (strArea, HELP_DIR);
			strcat (strArea, strfname);
			
			if (!fBootTestOnly)
			{
				sprintf(buf," Reading in help file %s",strArea);
				log_string( buf );
			}
			
			if ( strArea[0] == '-' )
			{
				fpArea = stdin;
			}
			else
			{
				if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
				{
					bugf("boot_db(): fopen '%s' failed for read - error %d (%s)",
						strArea, errno, strerror( errno));
					exit_error( 1 , "boot_db", "fopen for read of areafile failed");
				}
#ifdef WIN32	// turn off read in buffering so ftell() reports the correct position
				if(setvbuf( fpArea, NULL, _IONBF, 0 )){
					bugf("boot_db(): error setting "
						"setvbuf( fpArea, NULL, _IONBF, 0 ) for '%s' - error %d (%s).",
						strArea, errno, strerror( errno));
				}
#endif
			}
			
			for ( ; ; )
			{
				char *word;
				
				if(fread_letter(fpArea)!='#')
				{
					bug("Boot_db : # sign not found.");
					exit_error( 1 , "boot_db", "# sign not found");
				}
				
				word = fread_word( fpArea );
				
				if ( word[0] == '$' )
					break;
				else if ( !str_cmp( word, "HELPS"       )) import_helps(fpArea);
				else if ( !str_cmp( word, "HELPFILEDATA")) load_helpfile_NAFF(fpArea);
				else if ( !str_cmp( word, "SOCIALS"     )) {
					socialnew = false;
					oldload_socials(fpArea);		// old SOCIALS file
				}
				else
				{
					bugf("Boot_db: bad section name '%s'.", word);
					exit_error( 1 , "boot_db", "bad section name");
				}
			}
			
			if ( fpArea != stdin )
				fclose( fpArea );
			fpArea = NULL;
		}
		fclose( fpList );
	}
	update_currenttime();

	help_init_quicklookup_table();
	/* end of reading in the HELP and SOCIAL files */
	/***********************************************/

	// read in the lockers database
	lockers->lockers_load_db();

	// read in saved objects for all rooms with corpses
	do_load_corpses(NULL, "");
	
	/*
	* Fix up exits.
	* Declare db booting over.
	* Convert all old_format objects to new_format, ROM OLC
	* Reset all areas once.
	* Load up the songs, notes and ban files.
	*/
	{
		// ensure there is a limbo room, if not exit
		if(!get_room_index( ROOM_VNUM_LIMBO )){
			bugf("boot_db(): limbo room could not be found (room vnum %d)!", ROOM_VNUM_LIMBO);
			log_note("You MUST have the limbo room, along with a number of other "
				"rooms and objects.  If you are wanting to start a mud with 100% "
				"original areas, it is STRONGLY recommended that you retain "
				"dawn.are.  This is a special system area file which contains "
				"a lot of library objects (e.g. the corpse object, piles of "
				"gold etc).`1`1"
				"WITHOUT THE OBJECTS AND ROOMS IN DAWN.ARE THE MUD WILL BE UNSTABLE!"
				"`1`1It is safe to remove any of the other area files... "
				"however we recommended that you retain the ooc.are or "
				"at least recreate and configure a main ooc room... "
				"this room is used by the goooc command, and dawn also uses "
				"this room as a backup when some other rooms are missing "
				"(instead of crashing)."
				"`1`1Please note, dawn.are occupies vnum ranges 1 "
				"to 500 - the range which is specifically reserved for Dawn's "
				"internal use, you should NOT build anything in this range.");
			exit_error( 1 , "boot_db", "missing limbo");
		}

		fulltime_log_string( "All area files read in, sorting area lists." );
		sort_arealists();					update_currenttime();
		fulltime_log_string( "All area files sorted, linking rooms together." );
		fix_exits( );						update_currenttime();
		fulltime_log_string( "Exit testing completed." );
		attach_resets();					update_currenttime();
		fix_resets( );						update_currenttime();
		fix_mudprogs( );					update_currenttime();
		fulltime_log_string( "Mudprog testing completed." );
		fBootDb     = false;
		area_update( );						update_currenttime();
		load_notes( );						update_currenttime();
		load_disabled();					update_currenttime();
		load_bans();						update_currenttime();
		laston_load();						update_currenttime();
		resort_top_roleplayers();			update_currenttime();
		resort_top_wealth();				update_currenttime();
	}

	// setup the web character and colour convertion systems
	init_static_characters();

	init_limbo_mob_index_data();

	// loadup letgain database
	load_letgain_db();				update_currenttime();

	// loadup quest database
	load_quest_db();

	// loadup mix database
	load_mix_db();					update_currenttime();

	// loadup scripts database
	load_script_db();

	// loadup deity database
	load_deity_db();				update_currenttime();

	// loadup herb database
	load_herb_db();					update_currenttime();

	// loadup offmoot database
	load_offmoot_db();

	// load the name generator profiles
	do_read_nameprofiles(NULL,"");	update_currenttime();

	// go thru all rooms setting all tracks to empty
	init_room_tracks();				update_currenttime();

/*	if(resave_npcraces){
		do_saveraces(NULL,"");
		autonote(NOTE_SNOTE, "boot_db()", 
			"races were dynamically created", "admin",
			"Some npc races were dynamically created to read in the area files.  "
			"It is recommended that you check the properties on all new npc races.", true);
	}
*/
	return;
}

/**************************************************************************/
void hotreboot_check_tables();
/**************************************************************************/
// check all hard coded tables - like that race and pcrace tables match
void check_tables(){
	
	fulltime_log_string("Checking cross reference tables match...");
	hotreboot_check_tables();
	fulltime_log_string("Check complete");
};

/**************************************************************************/
// insert the area into the vnum sorted area list 
static void newarea_insert_vnum_sort(AREA_DATA *pArea)
{
    if ( !area_vnumsort_first ){
        area_vnumsort_first = pArea;
    }else{ // sort areas by vnum
        AREA_DATA *vsort; 
        AREA_DATA *vsort_prev = NULL;
        bool inserted = false;
		
        for ( vsort = area_vnumsort_first;
		vsort && !inserted; vsort= vsort->vnumsort_next)
        {
            if (pArea->min_vnum < vsort->min_vnum )
            {
                if (vsort_prev)
                {   // insert in the list 
                    pArea->vnumsort_next = vsort;
                    vsort_prev->vnumsort_next = pArea;
                }
                else // we are at the head
                {
                    // insert at the head 
                    pArea->vnumsort_next = area_vnumsort_first;
                    area_vnumsort_first = pArea;
                }
                inserted = true;
            }
            vsort_prev = vsort;   
        }
        if (!inserted)
        {
            vsort_prev->vnumsort_next = pArea;                  
        }
    }
}

/**************************************************************************/
// insert the area into the level sorted area list 
static void newarea_insert_level_sort(AREA_DATA *a)
{
    if ( !area_levelsort_first ){
        area_levelsort_first = a;
		return;
    }

	 // sort areas by level
    AREA_DATA *lsort=area_levelsort_first;
    AREA_DATA *lsort_prev = NULL;

    for ( ; lsort; lsort_prev = lsort, lsort= lsort->levelsort_next)
    {
		if(IS_NULLSTR(a->lcomment)){
			if(IS_NULLSTR(lsort->lcomment)){
				if(lsort->low_level>0){
					if(a->low_level<=0 || a->low_level>lsort->low_level){
						continue;
					}

					if(a->low_level==lsort->low_level && a->high_level>lsort->high_level){
						continue;
					}
				}else{
					if(a->low_level<lsort->low_level){
						continue;
					}
				}
			}
		}else{
			if(IS_NULLSTR(lsort->lcomment)){
				continue;
			}

			int i=strcmp(a->lcomment,lsort->lcomment);
			if(i>0){
				continue;
			}

			if(i==0 && strcmp(	FORMATF("%s%s", a->colour, a->name),
								FORMATF("%s%s", lsort->colour, lsort->name))>0
				)
			{
				continue;
			}

		}

		
		// add area to this point in the list
		if (lsort_prev)
        {   // insert in the list 
            a->levelsort_next = lsort;
            lsort_prev->levelsort_next = a;
        }
        else // we are at the head 
        {
            // insert at the head 
            a->levelsort_next = area_levelsort_first;
            area_levelsort_first = a;
        }
		return;
    }

    lsort_prev->levelsort_next = a;
}

/**************************************************************************/
// insert the area into the alphabetically sorted arealist 
static void newarea_insert_arealist_sort(AREA_DATA *pArea)
{
	
    if( !area_arealist_first ){
        area_arealist_first = pArea;
    }else{ // sort areas by name 
        AREA_DATA *asort; 
        AREA_DATA *asort_prev = NULL;
        bool inserted = false;
		char aname[MSL], buf[MSL];
		char *p;
		
		sprintf(aname, "%s", pArea->name);
		for( p = aname; p < aname+ str_len( aname); p++ )
		{
			*p = LOWER(*p);
		}
		
        for ( asort = area_arealist_first;
		asort && !inserted; asort= asort->arealist_sort_next)
        {
			// get lowercase - inefficient but rarely called
			sprintf(buf, "%s", asort->name);
			for( p = buf; p < buf+ str_len( buf); p++ )
			{
				*p = LOWER(*p);
			}
			
            if (strcmp(aname, buf)<0)
            {
                if (asort_prev)
                {   /* insert in the list */
                    pArea->arealist_sort_next= asort;
                    asort_prev->arealist_sort_next= pArea;
                }
                else /* we are at the head */
                {
                    /* insert at the head */
                    pArea->arealist_sort_next= area_arealist_first;
                    area_arealist_first= pArea;
                }
                inserted = true;
            }
            asort_prev = asort;   
        }
		
        // tail end of the list
		if (!inserted)
        {
            asort_prev->arealist_sort_next= pArea;                  
        }
    }
}


/**************************************************************************/
// sort the arealist, levellist and vlist 
// this isn't the most efficient way but is very simple.
// It is only called on startup and when an area is added, level range changed
// so being efficient doesn't matter here much
void sort_arealists()
{
    AREA_DATA *pArea;
	// no areas
	if (!area_first)
		return;
	
	fulltime_log_string("Sorting arealists");
	
	// first clear existing sorts
	for ( pArea = area_first; pArea; pArea= pArea->next)
    {
		pArea->vnumsort_next= NULL;
		pArea->levelsort_next= NULL;
		pArea->arealist_sort_next= NULL;
    }
	
	area_vnumsort_first = NULL;
	area_levelsort_first =  NULL;
	area_arealist_first = NULL;
	
	// next insert into the sorted lists
	for ( pArea = area_first; pArea; pArea= pArea->next)
    {
		newarea_insert_vnum_sort(pArea);
		newarea_insert_level_sort(pArea);
		newarea_insert_arealist_sort(pArea);
    }
}

/**************************************************************************/ 
// import help section from old format - oblivion code.
void import_helps( FILE *fp )
{
	char c;
	help_data *pHelp;
	helpfile_data *pHelpfile;

	// setup the helpfile entry
	pHelpfile =helpfile_allocate_new_entry();
    pHelpfile->file_name = str_dup((char *) &strArea[str_len(HELP_DIR)]);
	pHelpfile->title=str_dup("");
	pHelpfile->editors=str_dup("");
	pHelpfile->security=7;
	pHelpfile->vnum=top_helpfile;
	pHelpfile->flags = HELPFILE_NONE;
	
	// add at the bottom of the list
	if (helpfile_first)
	{
		helpfile_last->next=pHelpfile;
		helpfile_last=pHelpfile;
	}
	else
	{
		helpfile_first=pHelpfile;
		helpfile_last=pHelpfile;
	}

	// check if we have a # for HELPDATA section
	c = getc( fp );

	if (c!='#'){ // old format
		pHelpfile->version=0;
		ungetc( c, fp );
	}
    
	for ( ; ; ){
		char * pTrim;
		pHelp = help_allocate_new_entry();
		pHelp->helpfile = pHelpfile;
		
		pHelp->level    = fread_number( fp );
		pHelp->keyword  = fread_string( fp );

		//  -2 versioning system
		if ( pHelp->level == -2 )
		{
			if ( is_number( pHelp->keyword ))
			{
				pHelp->level    = fread_number( fp );
				pHelp->keyword  = fread_string( fp );
				pHelp->flags = fread_wordflag( help_flags, fp );
			}
			else
			{
				break;
			}
		}

		// change level -1 to level 0 and set the HELP_HIDE_KEYWORDS flag
		if(pHelp->level==-1){
			pHelp->level=0;
			SET_BIT(pHelp->flags, HELP_HIDE_KEYWORDS);
		}
		
		if ( pHelp->keyword[0] == '$' )
			break;
		pHelp->text     = fread_string( fp );
		
		// system to trim off the . at the start of a text, resaved if necessary 
		if (pHelp->text[0]=='.')
		{
			pTrim = str_dup((char *)&(pHelp->text[1]));
			free_string(pHelp->text);
			pHelp->text = pTrim;
		}
		
		// swaps ¡ (ascii 173) for the tilde ~ (pre `- days)
		show_tilde(pHelp->text);
		
		// link it into the list
		if ( !help_first){
			help_first = pHelp;
			help_first->prev=NULL;
		}
		if ( help_last){
			help_last->next = pHelp;
			pHelp->prev		= help_last;
		}

		pHelp->undo_edittext=str_dup("");
		pHelp->undo_wraptext=str_dup("");
		help_last       = pHelp;
		pHelp->next     = NULL;
		pHelp->helpfile->entries++;
		top_help++;
	}

	top_helpfile++;
	return;
}
/**************************************************************************/
void load_arearom( FILE *fp )
{
	logf("Loading area from rom format...");
    AREA_DATA *pArea;
	
	pArea               = (AREA_DATA *)alloc_perm( sizeof(*pArea) );
	pArea->age          = 15;
	pArea->nplayer      = 0;
	pArea->file_name    = str_dup((char *) &strArea[str_len(AREA_DIR)]);

	char short_name[MIL];
	strcpy(short_name,pArea->file_name);
	for (char *p=short_name; !IS_NULLSTR(p); p++){
		if(*p=='.'){
			*p='\0';
			break;
		}
	}
	pArea->short_name	= str_dup(short_name);
	pArea->vnum         = top_area;
	pArea->name         = str_dup( "New Area" );
	pArea->builders     = str_dup( "" );
	for (int br=0;br<MAX_BUILD_RESTRICTS;br++)
	{
		pArea->build_restricts[br]= str_dup("");
	}   
	pArea->version      = 1;
	pArea->security     = 9;
	pArea->min_vnum     = 0;
	pArea->max_vnum     = 0;
    pArea->maplevel		= LEVEL_IMMORTAL;
	pArea->area_flags   = 0;
	pArea->colour		= str_dup( "x" );		// colour in the area list
	pArea->colourcode	= '{'; // old format for colour codes
	pArea->low_level    = -1;   // undefined 
	pArea->high_level   = -1;   // undefined 
	pArea->lcomment     = str_dup(""); // undefined 
	pArea->mapscale		= 0;	// undefined 
	pArea->vnum_offset	= 0;
	pArea->credits		= str_dup("");
	pArea->continent	= NULL;		// default them as none
	
    fread_string(fp); // gobble up the filename field
    pArea->name		= fread_string( fp );
    pArea->credits	= fread_string( fp );
    pArea->min_vnum	= fread_number(fp);
    pArea->max_vnum	= fread_number(fp);
    pArea->age		= 15;
    pArea->nplayer	= 0;
    pArea->empty	= false;

	if ( !area_first )
	{
		area_first = pArea;
	}
	
	if ( area_last  )
    {
		area_last->next = pArea;
    }
	
    area_last   = pArea;
    pArea->next = NULL;
    top_area++;
    dbVersion=pArea->version;
	if(dbVersion<10){
		SET_BIT(pArea->olc_flags, OLCAREA_IGNORE_UNDEFINED_FLAGS);
	}

	areaimport_convert_credits(pArea);

	// set a default maplevel, so people can see the maps
	pArea->maplevel=UMAX(pArea->low_level-10,1);

    return;
}
/**************************************************************************/
// Snarf an #AREAFILE header section 
void load_area( FILE *fp, bool dawnareadata)
{
    AREA_DATA *pArea;
    char      *word;
    bool      fMatch;
	
	pArea               = (AREA_DATA *)alloc_perm( sizeof(*pArea) );
	pArea->age          = 15;
	pArea->nplayer      = 0;
	pArea->file_name    = str_dup((char *) &strArea[str_len(AREA_DIR)]);
	pArea->vnum         = top_area;
	pArea->name         = str_dup( "New Area" );
	pArea->short_name   = str_dup( "" );

	pArea->builders     = str_dup( "None" );
	for (int br=0;br<MAX_BUILD_RESTRICTS;br++)
	{
		pArea->build_restricts[br]= str_dup("");
	}   
	pArea->version      = 1;
	pArea->security     = 9;
	pArea->min_vnum     = 0;
	pArea->max_vnum     = 0;
	pArea->area_flags   = 0;
	pArea->colour		= str_dup( "x" );		// colour in the area list
	pArea->colourcode	= '{'; // old format for colour codes
	pArea->low_level    = -1;   // undefined 
	pArea->high_level   = -1;   // undefined 
	pArea->lcomment     = '\0'; // undefined 
	pArea->mapscale		= 0;	// undefined 
	pArea->vnum_offset	= 0;
	pArea->credits		= str_dup("");
	pArea->continent	= continent_exact_lookup("none"); // default them as none

	for ( ; ; )
	{
		word   = feof( fp ) ? (char*)"End" : fread_word( fp );
		fMatch = false;
		
		switch ( UPPER(word[0]) )
		{	
		case '*': // comments with a * are ignored
			fread_to_eol(fp );
			break;

        case 'A':
            KEY( "AreaFlags", pArea->area_flags, fread_flag( fp ) ); // version <11 format
            KEY( "AFlags",	pArea->area_flags,	fread_wordflag( area_flags, fp ) );

			// area echos
			if(!str_cmp( word, "AreaEcho"))
			{
				area_echo_data *ae=new_area_echo();
				ae->firsthour		=fread_number(fp);
				ae->lasthour	=fread_number(fp);
				ae->percentage	=fread_number(fp);
				ae->echotext	=unpack_string(fread_string(fp));
				ae->next		=pArea->echoes;
				pArea->echoes= ae;
				fMatch = true;
				break;
			}
            break;

		case 'B':
			if ( !str_cmp( word, "Builders" ) ){
            	pArea->builders= fread_string( fp );
                fMatch = true;
			}


			if ( !str_cmp( word, "build_restricts" ) )
            {          
				char *type= feof( fp ) ? (char*)"End" : fread_word( fp );

				int index=flag_value(buildrestrict_types,type);

				if(index<0 || index > MAX_BUILD_RESTRICTS){
					bugf("area '%s' <%s> has an unknown 'build_restricts' item in the header!",
						pArea->name,
						pArea->file_name
						);
					exit_error( 1 , "load_area", "unknown build_restricts");
				}
				pArea->build_restricts[index]= fread_string( fp );
                fMatch = true;
            } 
			break;
			
		case 'C':
			SKEY( "Colour", pArea->colour );
			KEY("colourcode", pArea->colourcode, fread_letter( fp ) );
			SKEY( "Credits", pArea->credits);

			if ( !str_cmp( word, "Continent" ) ){
				char *t=fread_string( fp );
				pArea->continent = continent_lookup( t );

				if ( !pArea->continent){										
					logf("Automatically added continent '%s' while reading in area.", t),
					autonote(NOTE_SNOTE, "load_area()",
						FORMATF("Automatically added continent '%s'", t),
						"admin", 
						FORMATF("Automatically added continent '%s' while reading in area '%s' (%s).", 
							t, pArea->name, pArea->file_name ),
						true);
					
					pArea->continent=new continent_type;

					// insert the new continent at the front of the list
					pArea->continent->name=str_dup(trim_string(t));
					pArea->continent->next=continent_list;
					continent_list=pArea->continent;					
					save_continents();
				}
				fMatch = true;
			}
			
			break;
		
		case 'E':
            if ( !str_cmp( word, "End" ) )
            {          
                fMatch = true;
                
                if (pArea->maplevel==0){
                    pArea->maplevel=LEVEL_IMMORTAL;
                }
				
                if (pArea->min_vnum==0)
                {
					logf("area '%s' <%s> has a zero minimum vnum value! - set one in the area header",
						pArea->name,
						pArea->file_name
						);
					exit_error( 1 , "load_area", "zero minimum vnum in area header");
                }
                
                if (pArea->max_vnum==0)
                {
					logf("area '%s' <%s> has a zero maximum vnum value! - set one in the area header",
						pArea->name,
						pArea->file_name
						);
					exit_error( 1 , "load_area", "zero maximum vnum in area header");
                }   
				
				if ( !area_first ){
					area_first = pArea;
				}
				
				if ( area_last  ){
					area_last->next = pArea;
                }
				
                area_last   = pArea;
                pArea->next = NULL;
                top_area++;

                dbVersion=pArea->version;

				if(dbVersion<10){
					SET_BIT(pArea->olc_flags, OLCAREA_IGNORE_UNDEFINED_FLAGS);					
				}

				areaimport_convert_credits(pArea);
                return;
            }
			break;		

		case 'F':
            if ( !str_cmp( word, "FromMUD" ) )
			{
				char *t=fread_string(fp);
				free_string(t); // do nothing with this field for now
			}
			break;
			
		case 'L':
            if ( !str_cmp( word, "LComment" ) )
			{
				pArea->lcomment = fread_string_eol( fp );
			}
            if ( !str_cmp( word, "LRange" ) )
			{
				pArea->low_level  = fread_number( fp );
				pArea->high_level = fread_number( fp );
			}
            break;

		case 'M':
			KEY("MapScale", pArea->mapscale, fread_number( fp ) );
			KEY("MapLevel", pArea->maplevel, fread_number( fp ) );
			break;

		case 'N':
            SKEY( "Name", pArea->name );
			break;
			
		case 'S':
            KEY( "Security",	pArea->security, fread_number( fp ) );
            KEY( "ShortName",	pArea->short_name, unpack_string(fread_string( fp )) );
            break;

		case 'V':
            if ( !str_cmp( word, "VNUMs" ) )
			{
				pArea->min_vnum = fread_number( fp );
				pArea->max_vnum = fread_number( fp );
				if(pArea->min_vnum>pArea->max_vnum){
					int swap=pArea->min_vnum;
					pArea->min_vnum=pArea->max_vnum;
					pArea->max_vnum=swap;
				}
			}
            KEY("Version",		pArea->version,		fread_number( fp ) );
            if ( !str_cmp( word, "Vnum_offset" ) )
			{
				pArea->vnum_offset=fread_number( fp );
				if(pArea->vnum_offset)
				{
					logf("vnum_offset of %d in use, vnum range transposed from %d-%d to %d-%d.",
						pArea->vnum_offset,
						pArea->min_vnum,
						pArea->max_vnum,
						pArea->min_vnum+pArea->vnum_offset,
						pArea->max_vnum+pArea->vnum_offset);
				}
			}
			break;
		}
	}
}

/**************************************************************************/
// Sets vnum range for area using OLC protection features.
void assign_area_vnum( int vnum )
{
	int tempvnum=vnum-area_last->vnum_offset;
	
	if ( area_last->min_vnum == 0 || area_last->max_vnum == 0 ){
		area_last->min_vnum = area_last->max_vnum = tempvnum;
	}
	if ( tempvnum != URANGE( area_last->min_vnum, tempvnum, area_last->max_vnum ) ){
		if ( tempvnum < area_last->min_vnum ){
			area_last->min_vnum = tempvnum;
		}else{
			area_last->max_vnum = tempvnum;
		}
	}
}
/**************************************************************************/
// Adds a reset to a room.  OLC
// Similar to add_reset in olc.c
void new_reset( ROOM_INDEX_DATA *pR, RESET_DATA *pReset )
{
	if ( !pR )
		return;
		
	if ( pR->reset_last ){
		pR->reset_last->next=pReset;
	}else{
		pR->reset_first=pReset;
	}

	pR->reset_last= pReset;
	pR->reset_last->next = NULL;
	
	top_reset++;
	return;
}

/**************************************************************************/
// Deletes a reset from a room
void delete_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pWorkingReset )
{
	RESET_DATA *pReset;
	RESET_DATA *prev = NULL;
	
	for ( pReset= pRoom->reset_first; pReset; pReset = pReset->next )
	{
		if ( pReset == pWorkingReset )
			break;
		prev = pReset;
	}
	
	if ( !pReset )
	{
		bug("delete_reset: Reset not found!");
		return;
	}
	
	if ( prev ){
		prev->next = prev->next->next;
	}else{
		pRoom->reset_first = pRoom->reset_first->next;
	}
	
	top_reset--;
}

/**************************************************************************/
RESET_DATA *reset_find_last_object_occurance(RESET_DATA *resets_list, int obj_vnum, RESET_DATA *stop_at)
{
	RESET_DATA *r;
	int count=0;
	// first count how many times the object appears
	for(r=resets_list; r && r!=stop_at; r=r->next){
		switch(r->command){
			default: break;
			case 'O':
			case 'P':
			case 'G':
			case 'E':
			case 'Z': // a processed P
				if(r->arg1==obj_vnum){
					count++;
				}
			break;
		}
	}
	if(count==0){
		return NULL;
	}

	// find the last occurance
	for(r=resets_list; r; r=r->next){
		switch(r->command){
			default: break;
			case 'O':
			case 'P':
			case 'G':
			case 'E':
			case 'Z': // a processed P
				if(r->arg1==obj_vnum){
					if(--count==0){ // we have found the last one
						return r;
					}
				}
			break;
		}
	}


	// make sure things aren't really weird
	bugf("reset_find_last_object_occurance(): end of file, it shouldn't be possible to get here!");
	do_abort();

	return NULL;
}
/**************************************************************************/
RESET_DATA  *complete_resets_list=NULL;
RESET_DATA  *complete_resets_list_last=NULL;
/**************************************************************************/
// remove a reset from the complete_resets_list
void reset_remove(RESET_DATA *reset_to_remove)
{
	if(reset_to_remove==complete_resets_list){
		complete_resets_list=reset_to_remove->next;
		reset_to_remove->next=NULL;
		return;
	}

	RESET_DATA *r;
	RESET_DATA *prev;
	prev=complete_resets_list;
	for(r=complete_resets_list->next; r; r=r->next){
		if(r==reset_to_remove){
			prev->next=r->next;
			r->next=NULL;
			return;
		}
		prev=r;
	}

	bug("reset_remove(): unfound reset");
	return;
}

/**************************************************************************/
/*
	// RESETS VERSION 1 AND 2 FORMATS
	M 0 <mnum>			<global mob limit>	<rnum>			<room mob limit>
	O 0 <onum>			<global limit>		<rnum>			0
	P 0 <contents_onum> <global limit>	<container_onum>	<container_limit>
(v2 the P RESET MUST directly follow the container onum)
    G 0 <onum>			<global limit>		0				0
	E 0 <onum>			<global limit>		<wear_location>	0 
(v1 wear_loc= numeric, v2 wear_loc is a wordflag from wear_location_types)
	D 0 <rnum>			<direction>			<door flags>	0 
(v2 D is not valid in a version 2 - currently silently ignored)
	R 0 <rnum>			<direction>			0				0
    S

NOTES:

  // mob to room reset
		M loads mobile mnum into room rnum, up to the max world_limit
          of mobs.

  // object related resets
	//to room
        O puts object onum into room rnum.
	//to object
        P puts contents_onum into container_onum.
		  Technically the container_onum, doesn't have to be an object
		  and can be nested - i.e. objects within other objects, within
		  other objects - can be an object carried by a mob also?
	//to mob inventory
        G puts onum into a mob's inventory. This MUST follow the M of
          the mob you wish to give the item to!
	//to mob equip
        E equips mob with onum on wear_location. This MUST follow the
          M of the mob you wish to give the item to!

  // exit related resets
		D sets the door facing "direction" in rnum to door_state
        R randomizes rnums exits. Put total number of exits into
          total_exits. (This reset is not documented, probably because
		  all right-thinking people realize this is a major annoyance)

  // reset list terminator S
        S Denotes the end of the #RESETS section
*/
/**************************************************************************/
// load a resets section, appending it to the list of existing resets
void load_resets( FILE *fp, int resets_version, int area_version )
{
	char ditched_resets_log[MSL]; // log all the resets we ditch to a note
	RESET_DATA  *first_in_area_reset;
	RESET_DATA  *pReset;
	int invalid_reset_count=0;
    int rVnum;

	if ( !area_last )
	{
		bug("Load_resets: no #AREA section seen yet.");
		exit_error( 1 , "load_resets", "no #AREA section seen yet");
	}

	ditched_resets_log[0]='\0';
	// record the first reset for the area 
	// (basically complete_resets_list_last->next once it is known)
	first_in_area_reset=complete_resets_list_last;

	// loop thru loading all the resets
	for ( ; ; )
	{
		char             letter;
		rVnum=0;
		
		if ( ( letter = fread_letter( fp ) ) == 'S' )
			break;
		
		if ( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}
		
		pReset          = (RESET_DATA  *)alloc_perm( sizeof( *pReset ) );
		pReset->command = letter;	  		
		fread_number( fp ); // empty leading 0 - in format for historical reasons
		pReset->arg1    = fread_number( fp );
		pReset->arg2    = fread_number( fp );

		if(letter=='E'){ 
			if(resets_version>1){
				// E reset_version 2 and higher use wordflags for the wear locations
				pReset->arg3=fread_wordflag(wear_location_types, fp);
				// fread_wordflag() includes an fread_to_eol()
			}else{
				// E's reset_version 1 format - read in the numeric value
				pReset->arg3=fread_number( fp );

				// now convert the wear locations
				if(AREA_IMPORT_FORMAT(AIF_FORMAT2)){
					pReset->arg3=areaimport_translate_wear_locations_format2(pReset->arg3, area_version);
				}else if(AREA_IMPORT_FORMAT(AIF_FORMAT3)){
					pReset->arg3=areaimport_translate_wear_locations_format3(pReset->arg3, area_version);
				}else{
					// AIF_STOCK - the default
					pReset->arg3=areaimport_translate_wear_locations_stock(pReset->arg3, area_version);
				}

				fread_to_eol( fp );
			}
			pReset->arg4	  = 0;
		}else{
			pReset->arg3    = ( letter == 'G' || letter == 'R' )
				? 0 : fread_number( fp );
			if((letter=='P')||(letter=='M')){
				pReset->arg4    = fread_number( fp );
			}else{
				pReset->arg4	  = 0;
			}
			fread_to_eol( fp );
		}

		// checks on the room vnums being specified
		switch ( letter ){
			case 'M':
			case 'O':
				rVnum = pReset->arg3;
				break;

			case 'D':
			case 'R':
				rVnum = pReset->arg1;
				break;

			default:
			case 'P':
			case 'G':
			case 'E':
				rVnum = 0; // not used
				break;
		};
		if(rVnum<0){
			bugf("load_resets(): Room vnum value in reset '%c 0 %d %d %d %d' is negative! "
				"(which is invalid because you can't have a negative room vnum)",
				pReset->command, pReset->arg1,	pReset->arg2, pReset->arg3,	pReset->arg4);
			log_note("Manually edit the file, removing or correcting the reset and reboot the mud.");
			exit_error( 1 , "load_resets", "Negative room vnum in reset");
		}

		// translate vnum offsets when necessary
		switch ( letter )
		{
			case 'M':
			case 'O':
			case 'P':
			apply_area_vnum_offset( &pReset->arg3);

			case 'G':
			case 'E':
			case 'D':
			case 'R':
			apply_area_vnum_offset( &pReset->arg1);
				break;
			default:
				bugf( "load_resets(): bad command '%c'.", letter );
				exit_error( 1 , "load_resets", "bad reset command");
				break;
		};

		if( letter=='D' ) // D 0 <rnum> <direction> <door flags> 0 
		{
			// with D resets, we convert them into the rs_flags (reset flags) on 
			// the particular exit then ditch them.
			ROOM_INDEX_DATA *pRoomIndex = get_room_index( pReset->arg1 );
			EXIT_DATA *pexit;

			if ( pReset->arg2 < 0
			||   pReset->arg2 >= MAX_DIR
			|| !pRoomIndex
			|| !( pexit = pRoomIndex->exit[pReset->arg2] )
			|| !IS_SET( pexit->rs_flags, EX_ISDOOR ) )
			{
				bugf( "Load_resets: 'D': exit %d, room %d not door.", pReset->arg2, pReset->arg1 );
				exit_error( 1 , "load_resets", "invalid D reset");
				return; // put here to get rid of compiler warnings
			}
	
			switch ( pReset->arg3 )
			{
				default: bugf( "Load_resets: 'D': bad 'locks': %d." , pReset->arg3); break;
				case 0: break;
				case 1: SET_BIT( pexit->rs_flags, EX_CLOSED );
					SET_BIT( pexit->exit_info, EX_CLOSED ); break;
				case 2: SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
					SET_BIT( pexit->exit_info, EX_CLOSED | EX_LOCKED ); break;
			}
			free_reset_data( pReset);
		}else{
			// non 'D' reset: append to the list of all resets
			if(!complete_resets_list){
				complete_resets_list=pReset;
				complete_resets_list_last=pReset;
			}else{
				complete_resets_list_last->next=pReset;
				complete_resets_list_last=pReset;
			}
		}
    }


	// The P reset is defined as 'P 0 <contents_onum> <global limit> <container_onum>'
	// we need to move it so it is located directly after where container_onum is 
	// reset into the game.
	// If any P reset from this area file attempts put itself inside an object
	// from another area it will be discarded.
	// If the object which it attempts to reset into doesn't exist, it will
	// be discard.
	// Any P reset that is discarded, is logged in the form of an
	// inote to code & realm.
	// NOTE: The P reset handling isn't implemented the most efficient way, 
	// but it isn't important enough to justify spending huge time on.
	// - Kal, July 01

	RESET_DATA *prev=NULL;
	RESET_DATA *current=first_in_area_reset;
	// find the start of the resets from this area:
	if(first_in_area_reset){
		first_in_area_reset=first_in_area_reset->next;
	}else{
		first_in_area_reset=complete_resets_list;
	}

	// now loop thru the resets from this area, processing the P's
	// moving them to their correct location... in this process
	// we change the P to Z, then change it back to P later in this
	// function.
	RESET_DATA *pReset_next;
	RESET_DATA *r;
	RESET_DATA *last_occurance;
	for(pReset=first_in_area_reset; pReset; pReset=pReset_next){
		pReset_next=pReset->next;
		prev=current;
		current=pReset;

		if(pReset->command=='P'){
			// we have found a P reset... do some tests

			// is the container object reset into this area?
			last_occurance=reset_find_last_object_occurance(first_in_area_reset, pReset->arg3, pReset);

			// if the container object isn't reset into this area, we ditch the reset
			if(!last_occurance){
				invalid_reset_count++;
				strcat(ditched_resets_log,
					FORMATF("Reset 'P 0 %d %d %d %d' discarded because it attempts to "
					"put object %d inside the last reseted copy of object %d.  Object %d "
					"is not reset into this area, and by design you can not reset objects "
					"into objects reset within another area.  "
					"If resets in this form were excepted, problems occur if area list is rearranged. "
					"Either ignore this situation, or manually track where the object should "
					"be reset, and put it in that area.`1`1",
					pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4,
					pReset->arg1, pReset->arg3, pReset->arg3));

				// remove the invalid reset
				reset_remove(pReset);
				continue;
			}


			// we have a reset that is valid within this area, 
			// check that its position is correct
			// in terms of position is should be directly after
			// the last occurance of the object... unless
			// there are some Z's (processed P's) directly following
			// the object, if that is the case, it should be after them.

			r=last_occurance; // we know that last_occurance isn't NULL
			// loop thru until we have that reset for the actual object or
			// the last Z reset in r
			while(r->next && r->next->command=='Z'){
				r=r->next;
			}

			// position the reset we are currently processing 
			// as the next reset.
			if(r->next!=pReset){ 
				// reset is not already in the correct place, 
				// move it to become r->next
				reset_remove(pReset); // remove them from the complete list of resets

				// insert them at the new location
				pReset->next=r->next;
				r->next=pReset;
			}
			pReset->command='Z'; // mark the reset as processed
		}

	}

	// now mark all Z processed resets back as P's for this area
	for(pReset=first_in_area_reset; pReset; pReset=pReset->next){
		if(pReset->command=='Z'){
			pReset->command='P';
		}
	}


	// autonote and log any problems
	if(!IS_NULLSTR(ditched_resets_log)){
		strcat(ditched_resets_log, FORMATF("Next time this area is resaved, %s will "
			"no longer appear in %s", 
			invalid_reset_count==1?"this reset":"these resets",			
			area_last->file_name));
		log_note(ditched_resets_log);
		autonote(NOTE_SNOTE, "load_resets()", 
			FORMATF("Invalid P reset%s in %s", 
				invalid_reset_count==1?"":"s",
				area_last->file_name),
			"code realm", ditched_resets_log, true);
	}

	return;
}



/**************************************************************************/
// Snarf a shop section.
void load_shops( FILE *fp )
{
	 SHOP_DATA *pShop;

	for ( ; ; )
	{
		MOB_INDEX_DATA *pMobIndex;
		int iTrade;

		pShop                   = (SHOP_DATA *)alloc_perm( sizeof(*pShop) );
		int mobnum=fread_number ( fp );
		apply_area_vnum_offset( &mobnum);
		pShop->keeper           = mobnum;
		if ( pShop->keeper == 0 )
			 break;
		for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
			pShop->buy_type[iTrade]     = fread_number( fp );
		pShop->profit_buy       = fread_number( fp );
		pShop->profit_sell      = fread_number( fp );
		pShop->open_hour        = fread_number( fp );
		pShop->close_hour       = fread_number( fp );
					  fread_to_eol( fp );
		
		if (get_mob_index( pShop->keeper )==NULL)
		{
			bugf("load_shops: Can't find shop keeper with Vnum %d!",
						pShop->keeper);
						
		}
		else
		{
			pMobIndex= get_mob_index( pShop->keeper );
			pMobIndex->pShop        = pShop;

			if ( shop_first == NULL )
				shop_first = pShop;
			if ( shop_last  != NULL )
				shop_last->next = pShop;

			shop_last       = pShop;
			pShop->next     = NULL;
			top_shop++;
		}

    }

    return;
}


/**************************************************************************/
// Snarf spec proc declarations.
void load_specials( FILE *fp )
{
	char *lastword;
	for ( ; ; )
	{
		MOB_INDEX_DATA *pMobIndex;
		OBJ_INDEX_DATA *pObjIndex;
		char letter;
		
		switch ( letter = fread_letter( fp ))
		{
		default:
			bugf( "Load_specials: letter '%c' not *MOS.", letter );
			lastword=fread_string_eol(fp);
			bugf( "fread_string_eol() after this shows '%s'.", lastword);
			lastword=fread_string_eol(fp);
			bugf( "fread_string_eol() after this shows '%s'.", lastword);
			exit_error( 1 , "load_specials", "invalid special code");
		
		case 'S':
			return;
		
		case '*':
			break;

		case 'O':
			pObjIndex = get_obj_index( fread_number ( fp ));
			lastword  = fread_word( fp );
			pObjIndex->ospec_fun = ospec_lookup( lastword );

			if ( pObjIndex->ospec_fun == 0 )
			{
				logf( "NOTE: Load_specials: 'O': vnum %d, unknown special '%s'",
					pObjIndex->vnum, lastword );
			}
			break;

		case 'M':
			int mobnum=fread_number ( fp );
			apply_area_vnum_offset( &mobnum);
			pMobIndex              = get_mob_index ( mobnum );
			lastword=fread_word   ( fp );
			if(!pMobIndex){
				logf( "NOTE: Load_specials: 'M': vnum %d, special '%s' - unfound mob.", 
					mobnum, lastword );
			}else{
				pMobIndex->spec_fun = spec_lookup( lastword);
				if ( pMobIndex->spec_fun == NULL )
				{
					logf( "NOTE: Load_specials: 'M': vnum %d, unknown special '%s'", 
						pMobIndex->vnum, lastword );
				}
			}
			break;
		}
		fread_to_eol( fp );
	}
}


/**************************************************************************/
void do_checkexits(char_data *ch, char *argument)
{    
    extern const sh_int rev_dir [];
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *to_room=NULL;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev= NULL;
    char buf[MSL];
    int iHash;
    int door;
	int count=0;

	int max=150000, min=0;
	char arg[MIL];
	if(ch){
		argument = one_argument( argument, arg );
		if(IS_NULLSTR(arg)){
			ch->println("Checkexits: shows invalid exits.");
			ch->println(" checkexits all");
			ch->println(" checkexits area");
			ch->println("'all' checks for the whole realm, 'area' checks only in the current area.");
			ch->println("Will display up to 50 mismatched exits.");
			return;
		}else{
			if(!str_cmp("area",arg)){
				if(!ch->in_room || !ch->in_room->area){
					ch->println("in invalid room!?!");
					return;
				}else{
					min=ch->in_room->area->min_vnum;
					max=ch->in_room->area->max_vnum;
					ch->printlnf("Checking vnum range %d to %d", min, max);
				}
			}else if(!str_cmp("all",arg)){
				ch->println("Checking all areas");
			}else{
				ch->printlnf("Invalid parameter '%s'.", arg);
				do_checkexits(ch,"");
				return;
			}

		}
	};

    fulltime_log_string( "Checking exits..." );

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
		for ( pRoomIndex  = room_index_hash[iHash];
				pRoomIndex != NULL;
				pRoomIndex  = pRoomIndex->next )
		{
			for ( door = 0; door < MAX_DIR; door++ )
			{
				buf[0]='\0';
				// check vnum range
				if(pRoomIndex->vnum>max){
					continue;
				}
				if(pRoomIndex->vnum<min){
					continue;
				}

				pexit= pRoomIndex->exit[door];
				if(!pexit){
					continue;
				}

				to_room= pexit->u1.to_room;
				if(!to_room){
					continue;
				}

				pexit_rev=to_room->exit[rev_dir[door]];

				if(IS_SET(pexit->rs_flags,EX_ONEWAY)){
					if(pexit_rev && pexit_rev->u1.to_room==pRoomIndex){
							sprintf( buf, "1way set on %d:%s which leads to %d, %d:%s leads back to %d - remove oneway.",
								pRoomIndex->vnum, capitalize(dir_shortname[door]),
								to_room->vnum, 
								to_room->vnum, capitalize(dir_shortname[rev_dir[door]]),
								pRoomIndex->vnum);
					}
				}else{
					if(pexit_rev){
						if(pexit_rev->u1.to_room!=pRoomIndex){
							sprintf( buf, "%5d:%-2s -> %5d:%-2s -> %d.",
								pRoomIndex->vnum, dir_shortname[door],
								to_room->vnum,    dir_shortname[rev_dir[door]],
								(pexit_rev->u1.to_room == NULL)
								? 0 : pexit_rev->u1.to_room->vnum );
						}			
					}else{ // exit goes to room that doesnt have a returning exit
						sprintf( buf, "%5d:%-2s -> %5d has no %2s exit, set 1way on %5d:%-2s or relink.",
							pRoomIndex->vnum, dir_shortname[door],
							to_room->vnum,    dir_shortname[rev_dir[door]],
							pRoomIndex->vnum, dir_shortname[door]);
					}

				}

				if (!fBootTestOnly && !IS_NULLSTR(buf))
				{
					count++;
					if(ch && count<50){
						ch->printlnf("%s",buf);
					}
					log_string( buf);
				}

			}
		}
    }
	if(ch){
		ch->printlnf("%d exit%s total",count, (count==1?"":"s"));
	}

	update_currenttime();
    fulltime_log_string( "Exit checking completed." );

}

/**************************************************************************/
// Translate all room exits from virtual to real.
// Has to be done after all rooms are read in.
void fix_exits( void )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    int iHash;
    int door;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
		for ( pRoomIndex  = room_index_hash[iHash];
		pRoomIndex != NULL;
		pRoomIndex  = pRoomIndex->next )
		{
			bool fexit;
			
			fexit = false;
			for ( door = 0; door < MAX_DIR; door++ )
			{
				if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
				{
					if ( pexit->u1.vnum <= 0 
						|| get_room_index(pexit->u1.vnum) == NULL)
					{
						logf("Couldn't link %s from %d to %d, removing exit.", 							
							capitalize(dir_name[door]),
							pRoomIndex->vnum,  
							pexit->u1.vnum);

						// deallocate the exit
						free_exit(pexit);
						pRoomIndex->exit[door]=NULL;
					}
					else
					{
						fexit = true; 
						pexit->u1.to_room = get_room_index( pexit->u1.vnum );
					}
				}
			}
		}
    }

    fulltime_log_string( "All rooms linked together" );
	do_checkexits(NULL,"" );
    return;
}

/**************************************************************************/
// Name:		fix_resets
// Purpose:	check all resets are valid - removes invalid ones
// Called by:	boot_db
// Note: This could be incorporated into attach_resets() and cleaned up.
void fix_resets( void )
{
	bool bLastMobInvalid, bLastObjInvalid;

	RESET_DATA *pReset;
	char buf[MSL];
	int iHash;
	ROOM_INDEX_DATA *pRoom;
	
	fulltime_log_string("Starting fix_resets");
	bLastMobInvalid = true; 
	bLastObjInvalid = true;
			
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
		{
	for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
		{
		// Validate parameters.
		// We're calling the index functions for the side effect.
			switch ( pReset->command )
			{
				default:
					bugf( "fix_resets: invalid reset type '%c'.", pReset->command );
					exit_error( 1 , "fix_resets", "invalid resset type");
					break;

				case 'M':
					bLastMobInvalid = true; 
					if ( !get_mob_index(pReset->arg1) ) // arg1
					{
						sprintf(buf,"Dropping reset 'M 0 *%d* %d %d %d' - couldn't find mob! (arg1)",
						  pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );
					}
					else if ( !get_room_index(pReset->arg3))
					{
						sprintf(buf,"Dropping reset 'M 0 %d %d %d *%d*' - couldn't find room! (arg3)",
							pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
							bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );
					}
					else bLastMobInvalid = false;
					break; // case M

				case 'O':
					bLastObjInvalid = true;
					if (!get_obj_index(pReset->arg1))
					{
						sprintf(buf,"Dropping reset 'O 0 *%d* %d %d' - couldn't find obj(arg1)",
								pReset->arg1, pReset->arg2, pReset->arg3);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );	
					}
					else if (!get_room_index(pReset->arg3))
					{
						sprintf(buf,"Dropping reset 'O 0 %d %d *%d*' - couldn't find room(arg3)",
								pReset->arg1, pReset->arg2, pReset->arg3);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );
					}
					else bLastObjInvalid = false;
					break; // case O

				case 'P':
					if (!get_obj_index(pReset->arg1))
					{
						sprintf(buf,"Dropping reset 'P 0 *%d* %d %d %d' - couldn't find obj(arg1)",
								pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );	
					}
					else if (!get_obj_index(pReset->arg3))
					{
						sprintf(buf,"Dropping reset 'P 0 %d %d *%d* %d' - couldn't find obj(arg3)",
								pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );
					}
					break; // case P

				case 'G':
					if (bLastMobInvalid)
					{
						sprintf(buf,"Dropping reset 'G 0 %d %d %d' - previous mob invalid!",
								pReset->arg1, pReset->arg2, pReset->arg3);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );	

					}
					else if (!get_obj_index(pReset->arg1))
					{
						sprintf(buf,"Dropping reset 'G 0 *%d* %d %d' - couldn't find obj(arg1)",
								pReset->arg1, pReset->arg2, pReset->arg3);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );	
					}
					break; // case G
					
				case 'E':
					if (bLastMobInvalid)
					{
						sprintf(buf,"Dropping reset 'E 0 %d %d %d' - previous mob invalid!",
								pReset->arg1, pReset->arg2, pReset->arg3);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );	
					}
					else if (!get_obj_index(pReset->arg1))
					{
						sprintf(buf,"Dropping reset 'E 0 *%d* %d %d' - couldn't find obj(arg1)",
								pReset->arg1, pReset->arg2, pReset->arg3);
						bug( buf );				
						append_datetimestring_to_file(BAD_RESETS_FILE, buf);
						delete_reset( pRoom, pReset );	
					}
					// convert Equip to position -1 to Give resets.
					if (pReset->arg3==-1)
					{	
						sprintf(buf,"**Converting 'E 0 %d %d %d' to 'G 0 %d -1**",
								pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg1);
						log_string( buf );
						pReset->command='G';
						//arg1 stays the same
						pReset->arg2 = -1;
						pReset->arg3 = 0;
						pReset->arg4 = 0;
					}
					break; // case E

				case 'R':
					break;
			}
		}
	}
	}
				
	fulltime_log_string("fix_resets completed");
	return;
}

/**************************************************************************/
// Load mudprogs section
void load_mudprogs( FILE *fp )
{
    MUDPROG_CODE *pMprog;
	
    if ( area_last == NULL )
    {
		bug("Load_mudprogs: no #AREA seen yet.");
		exit_error( 1 , "load_mudprogs", "no #AREA seen yet");
    }
	
    for ( ; ; )
    {
		vn_int vnum;
		char letter;
		
		letter = fread_letter( fp );
		if ( letter != '#' )
		{
			bugf("Load_mudprogs: # not found, letter='%c'.", letter);
			exit_error( 1 , "load_mudprogs", "# not found");
		}
		
		vnum = fread_number( fp );
		if ( vnum == 0 )
			break;
		vnum+= area_last->vnum_offset; 
		
		fBootDb = false;
		if ( get_mprog_index( vnum ) != NULL )
		{
			bugf( "Load_mudprogs: vnum %d duplicated.", vnum );
			exit_error( 1 , "load_mudprogs", "duplicate vnum");
		}
		fBootDb = true;
		
		pMprog      = (MUDPROG_CODE *)alloc_perm( sizeof(*pMprog) );
		mudprog_count++;
		pMprog->vnum = vnum;
		pMprog->area = area_last;
		// with area format version 4 and above mudprogs have a name
		if(area_last->version>3)
		{
			pMprog->title	= fread_string( fp );
			pMprog->author	= fread_string( fp ); 
			pMprog->last_editor= fread_string( fp ); 
			pMprog->last_editdate= fread_number(fp);

			// strip all the (null) from old saving code
			// - don't bother freeing memory, since will be fixed 
			//   next hotreboot after the area is saved.
			if (!str_cmp(pMprog->title, "(null)")){
				pMprog->title		= str_dup("");
			}
			if (!str_cmp(pMprog->author, "(null)")){
				pMprog->author		= str_dup("");
			}
			if (!str_cmp(pMprog->last_editor, "(null)")){
				pMprog->last_editor	= str_dup("");
			}
		}
		else
		{
			pMprog->title		= str_dup("");
			pMprog->author		= str_dup("");
			pMprog->last_editor	= str_dup("");
			pMprog->last_editdate= 0;
		}

		pMprog->code    = fread_string( fp );
		if ( mudprog_list == NULL )
			mudprog_list = pMprog;
		else
		{
			pMprog->next = mudprog_list;
			mudprog_list  = pMprog;
		}

		// do changes to upgrade format after all areas have been read in
/*		if (area_last->subversion<1)
		{
			pMprog->name = str_dup("");
			area_last->subversion=1;
		}
*/
    }
    return;
}

/**************************************************************************/
// Translate mudprog vnums pointers to real code
void fix_mudprogs( void )
{
    MUDPROG_TRIGGER_LIST        *list;
    MUDPROG_CODE        *prog;
	MUDPROG_TRIGGER_LIST        *prev;
    int iHash;
	bool terminate=false;
	
	// on mobiles
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	    MOB_INDEX_DATA *pMobIndex;
		for ( pMobIndex   = mob_index_hash[iHash];
		pMobIndex   != NULL;
		pMobIndex   = pMobIndex->next )
		{
			prev=NULL;
			for( list = pMobIndex->mob_triggers; list; list = list->next )
			{				
				if ( ( prog = get_mprog_index( list->temp_mpvnum ) ) != NULL ){
					list->prog = prog;
					prev=list;
					SET_BIT(pMobIndex->mprog_flags, list->trig_type);
					SET_BIT(pMobIndex->mprog2_flags, list->trig2_type);
				}else{
					if(AREA_IMPORT_FLAG(AREAIMPORTFLAG_DISCARD_UNFOUND_MUDPROGS)){
						bugf( "Fix_mudprogs(): discarding code vnum %d not found on mob %d (%s) from %s.", 
							list->temp_mpvnum, pMobIndex->vnum, 
							pMobIndex->short_descr, pMobIndex->area->file_name);
						if(prev){
							prev->next=list->next;
						}else{
							pMobIndex->mob_triggers=list->next;
						}
					}else{
						bugf( "Fix_mudprogs(): code vnum %d not found on mob %d (%s) from %s.", 
							list->temp_mpvnum, pMobIndex->vnum, 
							pMobIndex->short_descr, pMobIndex->area->file_name);
						terminate=true;										
					}
				}
			}
		}
    }
	// on objects
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
		OBJ_INDEX_DATA *pObjIndex;
		
		for ( pObjIndex  = obj_index_hash[iHash];
			pObjIndex   != NULL;
			pObjIndex   = pObjIndex ->next )
		{
			prev=NULL;
			for( list = pObjIndex->obj_triggers; list; list = list->next )
			{				
				if ( ( prog = get_mprog_index( list->temp_mpvnum ) ) != NULL ){
					list->prog = prog;
					prev=list;
					SET_BIT(pObjIndex->oprog_flags, list->trig_type);
					SET_BIT(pObjIndex->oprog2_flags, list->trig2_type);
				}else{
					if(AREA_IMPORT_FLAG(AREAIMPORTFLAG_DISCARD_UNFOUND_MUDPROGS)){
						bugf( "Fix_mudprogs(): discarding code vnum %d not found on object %d (%s) from %s.", 
							list->temp_mpvnum, pObjIndex->vnum, 
							pObjIndex->short_descr, pObjIndex->area->file_name);
						if(prev){
							prev->next=list->next;
						}else{
							pObjIndex->obj_triggers=list->next;
						}
					}else{
						bugf( "Fix_mudprogs(): code vnum %d not found on object %d (%s) from %s.", 
							list->temp_mpvnum, pObjIndex->vnum, 
							pObjIndex->short_descr, pObjIndex->area->file_name);
						terminate=true;										
					}
				}
			}
		}
    }
	// on rooms to come - perhaps

	if(terminate){
		bug("Due to unfound mudprogs, unable to boot mud.");
		bug("If you want to boot the mud without these progs\n"
			"and lose references to them you can add \n"
			"'discard_unfound_mudprogs' to gameset.txt area_import_flags");
		exit_error( 1 , "fix_mudprogs", "unable to boot with missing mudprogs");
	}
}


/**************************************************************************/
// Repopulate areas periodically.
void area_update( void )
{
    AREA_DATA *pArea;
    char buf[MSL];

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
	{
		room_update( pArea );

		if ( ++pArea->age < 3 )
			continue;
    
		/*
		 * Check age and reset.
		 * Note: Mud Schools reset every 1 or 2 ticks (not 15).
		 */
		if ( (!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
		||    pArea->age >= 31)
		{
			reset_area( pArea );
			sprintf(buf,"%s has just been reset.",pArea->name);
			wiznet(buf,NULL,NULL,WIZ_RESETS,0,0);    

			// areas flagged with newbie area resets, reset more often
			if(IS_SET(pArea->area_flags, AREA_NEWBIE_AREA_RESETS)){
				pArea->age = 29;
			}else{
				pArea->age = number_range( 0, 3 );
			}

			if (pArea->nplayer == 0){
				pArea->empty = true;
			}
		}
	}
    return;
}

/**************************************************************************/
void reset_room( ROOM_INDEX_DATA *pRoom, bool unconditional )
{
	RESET_DATA  *pReset;
	char_data   *pMob;
	char_data   *mob;
	OBJ_DATA    *pObj;
	char_data   *LastMob = NULL;
	int iExit;
	int level = 0;
	bool last;
	char buf2[MSL];
	
	if ( !pRoom )
		return;
	mob = NULL;
	pMob        = NULL;
	last        = false;
	
	
    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
		EXIT_DATA *pExit;
		
		if ( ( pExit = pRoom->exit[iExit] ) )
		{
			pExit->exit_info = pExit->rs_flags;
			if ( ( pExit->u1.to_room != NULL )
				&& ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )
			{
				// nail the other side 
				pExit->exit_info = pExit->rs_flags;
			}
		}
    }
	
    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
		MOB_INDEX_DATA  *pMobIndex;
		OBJ_INDEX_DATA  *pObjIndex=NULL;
		OBJ_INDEX_DATA  *pObjToIndex=NULL;
		ROOM_INDEX_DATA *pRoomIndex;
		OBJ_DATA *obj;
		OBJ_DATA *obj_to;
		int count=0;
		int limit;
		
		switch ( pReset->command )
		{
		default:
			bugf( "Reset_room: bad command %c 0 %d %d %d %d.", 
				pReset->command,
				pReset->arg1,
				pReset->arg2,
				pReset->arg3,
				pReset->arg4);
			break;
			
		case 'M':
			{
				if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) ){
					bugf( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
					continue;
				}
				
				if ( pMobIndex->count >= pReset->arg2 ){
					last = false;
					break;
				}
				
				count = 0;
				for (mob = pRoom->people; mob != NULL; mob = mob->next_in_room)
				{
					if (mob->pIndexData == pMobIndex){
						if (IS_NPC(mob)){
							if (mob->subdued){
								mob->pIndexData->killed++;
								kill_table[URANGE(0, mob->level, MAX_LEVEL-1)].killed++;
								mob->pIndexData->count++;        
								extract_char( mob, true );
							}else{
								count++;
							}
						}
						
						if (count >= pReset->arg4){
							last = false;
							break;
						}
					}
				}
				
				if (count >= pReset->arg4)
					break;				
				
				if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
				{
					bugf( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
					continue;
				}
				
				count = 0;
				for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
				{
					if (mob->pIndexData == pMobIndex)
					{
						if (IS_NPC(mob))
						{
							if (mob->subdued)
							{
								mob->pIndexData->killed++;
								kill_table[URANGE(0, mob->level, MAX_LEVEL-1)].killed++;
								mob->pIndexData->count++;        
								extract_char( mob, true );
							}
							else
							{
								count++;
							}
							
							if (count >= pReset->arg4)
							{
								last = false;
								break;
							}
						}
					}
				}
				
				
				pMob = create_mobile( pMobIndex, 0 );					
				
				// Some more hard coding.
				
				// Infrared for mobs starting in dark rooms
				if ( room_is_dark( pRoom ) ){
					SET_BIT(pMob->affected_by, AFF_INFRARED);
				}
				
				// Pet shop mobiles get ACT_PET set.
				{
					ROOM_INDEX_DATA *pRoomIndexPrev;
					
					pRoomIndexPrev = get_room_index( pRoom->vnum - 1 );
					if ( pRoomIndexPrev
						&& IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
						SET_BIT( pMob->act, ACT_PET);
				}
				
				char_to_room( pMob, pRoom );
				
				LastMob = pMob;
				level  = URANGE( 0, pMob->level - 2, LEVEL_HERO - 1 );
				last = true;
				if ( HAS_TRIGGER( pMob, MTRIG_REPOP )){
					mp_percent_trigger( pMob, NULL, NULL, NULL, MTRIG_REPOP );
				}
			}
			break;
			
		case 'O': // put an object in the room (on floor)
			{
				if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
				{
					bugf( "Reset_room: 'O': bad vnum %d.", pReset->arg1 );
					continue;
				}
				
				if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
				{
					bugf( "Reset_room: 'O': bad vnum %d.", pReset->arg3 );
					continue;
				}
				
				if (   (!unconditional && pRoom->area->nplayer > 0)
					|| count_obj_list( pObjIndex, pRoom->contents ) > 0 )
					break; 
				
				pObj = create_object( pObjIndex);
				obj_to_room( pObj, pRoom );
			}
			break;
			
		case 'P': // put an object inside another object (in the room)
			if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
			{
				bugf( "Reset_area: 'P': bad vnum %d.", pReset->arg1 );
				continue;
			}
			
			if ( ( pObjToIndex = get_obj_index( pReset->arg3 ) ) == NULL )
			{
				bugf( "Reset_area: 'P': bad vnum %d.", pReset->arg3 );
				continue;
			}
			
			if (pReset->arg2 > 50){ // old format 
				limit = 6;
			}else if (pReset->arg2 == -1){ // no limit 
				limit = 999;
			}else{
				limit = pReset->arg2;
			}
			
			obj_to=get_obj_of_type_in_room( pObjToIndex, pRoom);
			
			if (!obj_to)
			{
				sprintf(buf2,"BUG??? in reset_room (Reset type P): get_obj_type returned NULL\r\nfor putting object %d into %d ", pReset->arg1, pReset->arg3);
				if(GAMESETTING3(GAMESET3_DISPLAY_P_RESET_BUGS_ON_WIZNET)){
					wiznet(buf2,NULL,NULL,WIZ_BUGS,0,GUARDIAN); // put it on the bug channel 
				}
				log_string( buf2 );
				
				if (pObjIndex->count)
				{
					sprintf(buf2,"BUG??? in reset_room (Reset type P): pObjIndex is NULL!\r\n rest for putting object %d into %d ", pReset->arg1, pReset->arg3);
					if(GAMESETTING3(GAMESET3_DISPLAY_P_RESET_BUGS_ON_WIZNET)){
						wiznet(buf2,NULL,NULL,WIZ_BUGS,0,GUARDIAN); // put it on the bug channel 
					}
					log_string( buf2 );
				}
			}
			else
			{
				if ((pObjIndex->count >= limit && number_range(0,4)!=0)
					|| (count = count_obj_list(pObjIndex,obj_to->contains))
					> pReset->arg4 )
				{
					last = false;
					break;
				}
				
				while (count < pReset->arg4)
				{
					obj = create_object( pObjIndex);
					obj_to_obj( obj, obj_to );
					count++;
					if (pObjIndex->count >= limit)
						break;
				}
				
				// fix object lock state!
				obj_to->value[1] = obj_to->pIndexData->value[1];
				last = true;
				break;
			}
			
		case 'G':
		case 'E':
			if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
			{
				bugf( "Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1 );
				continue;
			}
			
			if ( !last )
				break;
			
			if ( pMob == NULL )
			{
				bugf( "Reset_area: 'E' or 'G': null mob for vnum %d.",
					pReset->arg1 );
				last = false;
				break;
			}
			
			if ( pMob->pIndexData->pShop ){
				obj = create_object( pObjIndex);
				SET_BIT( obj->extra_flags, OBJEXTRA_INVENTORY );
			}else{
				if (pReset->arg2 > 50) // old format 
					limit = 6;
				else if (pReset->arg2 == -1 || pReset->arg2 == 0) // no limit 
					limit = 999;
				else
					limit = pReset->arg2;
				
				if (pObjIndex->count < limit)
				{
					obj=create_object(pObjIndex);
				}
				else
					break;
			}
			obj_to_char( obj, pMob );
			if ( pReset->command == 'E' )
				equip_char( pMob, obj, pReset->arg3 );
			last = true;
			break;
			
		case 'R':
			if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
			{
				bugf( "Reset_room: 'R': bad vnum %d.", pReset->arg1 );
				continue;
			}
			
			{
				EXIT_DATA *pExit;
				int d0;
				int d1;
				
				for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
				{
					d1                   = number_range( d0, pReset->arg2-1 );
					pExit                = pRoomIndex->exit[d0];
					pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
					pRoomIndex->exit[d1] = pExit;
				}
			}
			break;
		}
	}
	return;
}

/**************************************************************************/
// Reset one area.
void reset_area( AREA_DATA *pArea )
{
	 ROOM_INDEX_DATA *pRoom;
	 int  vnum;

     for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
	 {
		  if ( ( pRoom = get_room_index(vnum) ) )
				reset_room(pRoom, false);
	 }

	 return;
}

/**************************************************************************/
// Create an instance of a mobile.
// added int level to create a different level than the pMobIndex
// if > 0 it uses the passed value, if 0 uses the index's level

char_data *create_mobile( MOB_INDEX_DATA *pMobIndex, int level )
{
    char_data *mob;
    int i;
    AFFECT_DATA af;
	int maxvalue;
	
    mobile_count++;
	
    if ( pMobIndex == NULL )
	{
		bugf( "create_mobile(): pMobIndex==NULL!, get_mob_index()'s last failed call was for vnum %d.", DEBUG_LAST_NON_EXISTING_REQUESTED_MOBILE_VNUM);
		do_abort();
	}
	
    mob = new_char();
	
	mob->pIndexData		= pMobIndex;
	
	mob->name			= str_dup( pMobIndex->player_name );    
	mob->short_descr	= str_dup( pMobIndex->short_descr );    
	mob->long_descr		= str_dup( pMobIndex->long_descr );     
	mob->description	= str_dup( pMobIndex->description );
	mob->spec_fun		= pMobIndex->spec_fun;
	mob->gamble_fun		= pMobIndex->gamble_fun;
	mob->prompt			= NULL;
	mob->mprog_target   = NULL;
	
    if (pMobIndex->wealth == 0)
    {
		mob->silver = 0;
		mob->gold   = 0;
	}
    else
    {
		long wealth;
		
		wealth = number_range(pMobIndex->wealth/2, 3 * pMobIndex->wealth/2);

		if(pMobIndex->pShop && (boot_time+50>current_time)){
			wealth*=3; // 3 times the wealth on shop keepers after a reboot
		}
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
    } 
	
	// read from prototype
	mob->group			= pMobIndex->group;
	mob->helpgroup		= pMobIndex->helpgroup;
	mob->act			= pMobIndex->act;
	mob->act2			= pMobIndex->act2;
	mob->comm			= COMM_NOCHANNELS|COMM_NOTELL;
	mob->affected_by	= pMobIndex->affected_by;
	mob->affected_by2	= pMobIndex->affected_by2;
	mob->affected_by3	= pMobIndex->affected_by3;
	mob->tendency		= pMobIndex->tendency;
	mob->alliance		= pMobIndex->alliance;

	if ( level > 0 ){
		mob->level		= level;
	}else{
		mob->level		= pMobIndex->level;
	}

	mob->hitroll		= pMobIndex->hitroll;
	mob->damroll		= pMobIndex->damage[DICE_BONUS];
	
	maxvalue			= dice(pMobIndex->hit[DICE_NUMBER],
							pMobIndex->hit[DICE_TYPE])
							+ pMobIndex->hit[DICE_BONUS];
	mob->max_hit		= UMIN(32000,maxvalue);
	
	mob->hit			= mob->max_hit;
	
	maxvalue			= dice(pMobIndex->mana[DICE_NUMBER],
							pMobIndex->mana[DICE_TYPE])
							+ pMobIndex->mana[DICE_BONUS];
	mob->max_mana		= UMIN(32000,maxvalue);
	
	mob->mana			= mob->max_mana;
	mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
	mob->damage[DICE_TYPE]  = pMobIndex->damage[DICE_TYPE];
	mob->dam_type           = pMobIndex->dam_type;
	if (mob->dam_type == 0)
		switch(number_range(1,3))
	{
		case (1): mob->dam_type = 3;        break;  /* slash */
		case (2): mob->dam_type = 7;        break;  /* pound */
		case (3): mob->dam_type = 11;       break;  /* pierce */
	}
	for (i = 0; i < 4; i++)
		mob->armor[i]       = pMobIndex->ac[i]; 
	mob->off_flags          = pMobIndex->off_flags;
	mob->imm_flags          = pMobIndex->imm_flags;
	mob->res_flags          = pMobIndex->res_flags;
	mob->vuln_flags         = pMobIndex->vuln_flags;
	mob->start_pos          = pMobIndex->start_pos;
	mob->default_pos        = pMobIndex->default_pos;
	mob->sex                = pMobIndex->sex;
	if (mob->sex == 3) // random sex 
		mob->sex = number_range(1,2);
	mob->race               = pMobIndex->race;
	mob->form               = pMobIndex->form;
	mob->parts              = pMobIndex->parts;
	mob->size               = pMobIndex->size;
	mob->material           = str_dup(pMobIndex->material);
    mob->will               = (mob->size+1)*50;
    mob->wildness           = 100;
	
    // set moves on mounts real high    
    if (IS_SET(mob->form, FORM_MOUNTABLE)){
		mob->max_move=100*(mob->level+5);
    }
    mob->move=mob->max_move;
    mob->mounted_on        = NULL;
    mob->ridden_by         = NULL;
    mob->tethered=false;
    mob->bucking=false;
	
	// computed on the spot 
	for (i = 0; i < MAX_STATS; i ++)
	{
		mob->perm_stats[i] = number_range(22,mob->level+22);
		if(mob->perm_stats[i]>100){
			mob->perm_stats[i]=100;
		}
	}

	// lock the mobiles strength to 50 to make its modifier 0 by default
	// this is done to make the hit and damroll set in olc predicatable 
	mob->perm_stats[STAT_ST] =50; 
	
	if (IS_SET(mob->off_flags,OFF_FAST))
	{
		mob->perm_stats[STAT_QU] = 100;
		mob->perm_stats[STAT_AG] = 100; 
	}
    
	for (i = 0; i < MAX_STATS; i ++)
	{
		mob->modifiers[i]=mob->perm_stats[i]-50;
	}

	// let's get some spell action 
	if (IS_AFFECTED(mob,AFF_SANCTUARY))
	{
		af.where     = WHERE_AFFECTS;
		af.type      = skill_lookup("sanctuary");
		af.level     = mob->level;
		af.duration  = -1;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_SANCTUARY;
		affect_to_char( mob, &af );
	}
	
	if (IS_AFFECTED(mob,AFF_HASTE))
	{
		af.where     = WHERE_AFFECTS;
		af.type      = skill_lookup("haste");
		af.level     = mob->level;
		af.duration  = -1;
		af.location  = APPLY_QU;
		af.modifier  = 1 + (mob->level >= 18) + (mob->level >= 25) +
			(mob->level >= 32);
		af.bitvector = AFF_HASTE;
		affect_to_char( mob, &af );
	}
	
	if (IS_AFFECTED(mob,AFF_PROTECT_EVIL))
	{
		af.where     = WHERE_AFFECTS;
		af.type      = skill_lookup("protection evil");
		af.level     = mob->level;
		af.duration  = -1;
		af.location  = APPLY_SAVES;
		af.modifier  = -1;
		af.bitvector = AFF_PROTECT_EVIL;
		affect_to_char(mob,&af);
	}
	
	if (IS_AFFECTED(mob,AFF_PROTECT_GOOD))
	{
		af.where     = WHERE_AFFECTS;
		af.type      = skill_lookup("protection good");
		af.level     = mob->level;
		af.duration  = -1;
		af.location  = APPLY_SAVES;
		af.modifier  = -1;
		af.bitvector = AFF_PROTECT_GOOD;
		affect_to_char(mob,&af);
	}
	
    mob->position = mob->start_pos;
	
	// set the racial language on the mob
	mob->language = race_table[mob->race]->language;
	
    // link the mob to the world list
    mob->next           = char_list;
	char_list              = mob;
    pMobIndex->count++;
    return mob;
}

/**************************************************************************/
// duplicate a mobile exactly -- except inventory 
void clone_mobile(char_data *parent, char_data *clone)
{
	 int i;
    AFFECT_DATA *paf;

    if ( parent == NULL || clone == NULL || !IS_NPC(parent))
	return;
      
    //to prevent double mounting
    clone->mounted_on=NULL;
    clone->ridden_by=NULL;
    
    // start fixing values 
    clone->name         = str_dup(parent->name);
    clone->version      = parent->version;
    clone->short_descr  = str_dup(parent->short_descr);
    clone->long_descr   = str_dup(parent->long_descr);
    clone->description  = str_dup(parent->description);
    clone->group        = parent->group;
    clone->sex          = parent->sex;
    clone->clss			= parent->clss;
    clone->race         = parent->race;
    clone->level        = parent->level;
    clone->trust        = 0;
    clone->timer        = parent->timer;
    clone->wait         = parent->wait;
    clone->hit          = parent->hit;
    clone->max_hit      = parent->max_hit;
    clone->mana         = parent->mana;
    clone->max_mana     = parent->max_mana;
    clone->move         = parent->move;
    clone->max_move     = parent->max_move;
    clone->gold         = parent->gold;
    clone->silver       = parent->silver;
    clone->exp          = parent->exp;
    clone->act          = parent->act;
    clone->act2         = parent->act2;
    clone->comm         = parent->comm;
    clone->imm_flags    = parent->imm_flags;
    clone->res_flags    = parent->res_flags;
    clone->vuln_flags   = parent->vuln_flags;
    clone->invis_level  = parent->invis_level;
    clone->iwizi		= parent->iwizi;
    clone->owizi		= parent->owizi;
    clone->affected_by  = parent->affected_by;
    clone->affected_by2 = parent->affected_by2;
    clone->affected_by3 = parent->affected_by3;
    clone->position     = parent->position;
    clone->practice     = parent->practice;
    clone->train        = parent->train;
    clone->saving_throw = parent->saving_throw;
    clone->alliance     = parent->alliance;
    clone->tendency     = parent->tendency;
    clone->hitroll      = parent->hitroll;
    clone->damroll      = parent->damroll;
    clone->wimpy        = parent->wimpy;
    clone->form         = parent->form;
    clone->parts        = parent->parts;
    clone->size         = parent->size;
    clone->material     = str_dup(parent->material);
    clone->off_flags    = parent->off_flags;
    clone->dam_type     = parent->dam_type;
    clone->start_pos    = parent->start_pos;
    clone->default_pos  = parent->default_pos;
    clone->spec_fun     = parent->spec_fun;
    clone->tethered     = parent->tethered;
    clone->bucking      = false;
    clone->will         =parent->will;
    clone->wildness     =parent->wildness;

    for (i = 0; i < 4; i++)
	clone->armor[i] = parent->armor[i];

    for (i = 0; i < MAX_STATS; i++)
    {
	clone->perm_stats[i]    = parent->perm_stats[i];
	clone->modifiers[i]     = parent->modifiers[i];
    }

	 for (i = 0; i < 3; i++)
	clone->damage[i]        = parent->damage[i];

    // now add the affects 
    for (paf = parent->affected; paf != NULL; paf = paf->next)
	affect_to_char(clone,paf);

}

/**************************************************************************/
// Create an instance of an object from the obj_index_data record
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex)
{
	OBJ_DATA *obj;
	int i;
	
	if ( pObjIndex == NULL ){
		bugf( "create_object(): pObjIndex==NULL!, get_obj_index()'s last failed call was for vnum %d.", DEBUG_LAST_NON_EXISTING_REQUESTED_OBJECT_VNUM);
		do_abort();
	}
	
	obj = new_obj();
	
	obj->pIndexData		= pObjIndex;
	obj->in_room		= NULL;
	obj->chaos			= false;
	
	obj->level = pObjIndex->level;
	obj->wear_loc		= WEAR_NONE;
	
	replace_string(obj->name,		 pObjIndex->name );			
	replace_string(obj->short_descr, pObjIndex->short_descr ); 
	replace_string(obj->description, pObjIndex->description );
	obj->material		= pObjIndex->material;
	
	obj->item_type		= pObjIndex->item_type;
	obj->extra_flags	= pObjIndex->extra_flags;
	obj->extra2_flags	= pObjIndex->extra2_flags;
	obj->wear_flags		= pObjIndex->wear_flags;
    for (i = 0;  i < 5; i ++)
	{
		obj->value[i]	= pObjIndex->value[i];
	}
	obj->weight 		= pObjIndex->weight;
	obj->condition		= pObjIndex->condition;
	obj->cost			= pObjIndex->cost;
	obj->absolute_size	= pObjIndex->absolute_size;
	obj->relative_size	= pObjIndex->relative_size;
	obj->trap_trig		= pObjIndex->trap_trig;
	obj->trap_dtype		= pObjIndex->trap_dtype;
	obj->trap_charge	= pObjIndex->trap_charge;
	obj->trap_modifier	= pObjIndex->trap_modifier;
	obj->attune_id		= pObjIndex->attune_id;
	obj->attune_flags	= pObjIndex->attune_flags;
	obj->attune_modifier= pObjIndex->attune_modifier;
	obj->attune_next	= pObjIndex->attune_next;
	obj->ospec_fun		= pObjIndex->ospec_fun;
	obj->affected		= NULL;
		
	obj->next			= object_list;
	object_list 		= obj;
	pObjIndex->count++;
	
	return obj;
}

/**************************************************************************/
// duplicate an object exactly -- except contents 
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed,*ed_new;

    if (parent == NULL || clone == NULL)
	return;

	 /* start fixing the object */
    clone->name         = str_dup(parent->name);
    clone->short_descr  = str_dup(parent->short_descr);
    clone->description  = str_dup(parent->description);
    clone->item_type    = parent->item_type;
    clone->extra_flags  = parent->extra_flags;
    clone->extra2_flags = parent->extra2_flags;
    clone->wear_flags	= parent->wear_flags;
    clone->weight       = parent->weight;
    clone->cost         = parent->cost;
    clone->level        = parent->level;
    clone->condition    = parent->condition;
    clone->material     = str_dup(parent->material);
    clone->timer        = parent->timer;
    clone->absolute_size= parent->absolute_size;
    clone->relative_size= parent->relative_size;
	clone->ospec_fun	= parent->ospec_fun;
	clone->attune_id	= parent->attune_id;
	clone->attune_flags	= parent->attune_flags;
	clone->attune_modifier= parent->attune_modifier;
	clone->attune_next	= parent->attune_next;


    for (i = 0;  i < 5; i ++){
		clone->value[i] = parent->value[i];
	}

    // affects - copy accross the custom affects
	// clone->enchanted    = parent->enchanted;

    for (paf = parent->affected; paf != NULL; paf = paf->next)
	{
		affect_to_obj(clone,paf);
	}

    // extended description 
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next)
    {
		ed_new                  = new_extra_descr();
		ed_new->keyword         = str_dup( ed->keyword);
		ed_new->description     = str_dup( ed->description );
		ed_new->next            = clone->extra_descr;
		clone->extra_descr      = ed_new;
    }

}

/**************************************************************************/
/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *initial_ed )
{
	EXTRA_DESCR_DATA *ed;
	// first look for an exact match
    for ( ed=initial_ed; ed; ed = ed->next ){
		if ( is_exact_name( (char *) name, ed->keyword ) ){
			return ed->description;
		}
    }

	// now do an any match if the exact matching failed to get a result
    for ( ed=initial_ed; ed; ed = ed->next ){
		if ( is_name( (char *) name, ed->keyword ) ){
			return ed->description;
		}
    }
    return NULL;
}

/**************************************************************************/
/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;
	
    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];	
		pMobIndex != NULL;
		pMobIndex  = pMobIndex->next )
    {
		if ( pMobIndex->vnum == vnum )
			return pMobIndex;
    }
	
    if ( fBootDb )
    {
		bugf( "Get_mob_index: bad vnum %d.", vnum );
    }
	
	DEBUG_LAST_NON_EXISTING_REQUESTED_MOBILE_VNUM=vnum;
    return NULL;
}

/**************************************************************************/
/*
 * Translates object virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
	 OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex != NULL;
	  pObjIndex  = pObjIndex->next )
    {
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;
    }

    if ( fBootDb )
    {
		bugf( "Get_obj_index: bad vnum %d.", vnum );
    }

	DEBUG_LAST_NON_EXISTING_REQUESTED_OBJECT_VNUM=vnum;
    return NULL;
}

/**************************************************************************/
/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( vn_int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex != NULL;
	  pRoomIndex  = pRoomIndex->next )
    {
	if ( pRoomIndex->vnum == vnum )
	    return pRoomIndex;
    }

    if ( fBootDb  && !fBootTestOnly)
    {
		bugf( "Get_room_index: bad vnum %d.", vnum );
		log_string("             (doorway may not be linked)");		
    }

	DEBUG_LAST_NON_EXISTING_REQUESTED_ROOM_VNUM=vnum;
    return NULL;
}

/**************************************************************************/
MUDPROG_CODE *get_mprog_index( int vnum )
{
    MUDPROG_CODE *prg;
    for( prg = mudprog_list; prg; prg = prg->next )
    {
		if ( prg->vnum == vnum ){
			return( prg );
		}
    }
    return NULL;
}    

/**************************************************************************/
// Read a letter from a file.
char fread_letter( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( is_space(c) );

    return c;
}


/**************************************************************************/
// Read a number from a file.
int fread_number( FILE *fp )
{
	int number;
	bool sign;
	char c;

	do
	{
		c = getc( fp );
	}
	while ( is_space(c) );

	number = 0;

	sign   = false;
	
	if ( c == '+' )
	{
		c = getc( fp );
	}
	else if ( c == '-' )
	{
		sign = true;
		c = getc( fp );
	}

	if ( !is_digit(c) )
	{
		char *following=fread_string_eol(fp);
		char *following2=fread_string_eol(fp);
		bug("fread_number(): bad format.");
		bugf("\n====Non numeric character is '%c' (%d)\n"
			"====Following on line ='%s'\n"
			"====Next line ='%s'"
			,c, (int)c, following, following2);

		log_area_import_format_notice();

		do_abort( );
	}

	while ( is_digit(c) )
	{
		number = number * 10 + c - '0';
		c      = getc( fp );
	}

	if ( sign )
		number = 0 - number;

	if ( c == '|' )
		number += fread_number( fp );
    else if ( c != ' ' )
		ungetc( c, fp );

	return number;
}

/**************************************************************************/
long fread_flag( FILE *fp)
{
	int number;
	char c;
	bool negative = false;
    
	do{
		c = getc(fp);
	}
	while ( is_space(c));
    
	if (c == '-')
	{
		negative = true;
		c = getc(fp);
	}

	number = 0;
    
	if (!is_digit(c))
	{
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
		{
			number += flag_convert(c);
			c = getc(fp);
		}
	}

    while (is_digit(c))
	{
		number = number * 10 + c - '0';
		c = getc(fp);
	}

	if (c == '|')
		number += fread_flag(fp);
	else if  ( c != ' ')
		ungetc(c,fp);

	if (negative)
		return -1 * number;
	return number;
}

/**************************************************************************/
long flag_convert(char letter )
{
	long bitsum = 0;
	char i;

	if ('A' <= letter && letter <= 'Z')
	{
		bitsum = 1;
		for (i = letter; i > 'A'; i--)
			bitsum *= 2;
	}
	else if ('a' <= letter && letter <= 'z')
	{
		bitsum = 67108864; /* 2^26 */
		for (i = letter; i > 'a'; i --)
			bitsum *= 2;
	}

	return bitsum;
}

/**************************************************************************/
/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{
    char *plast;
    char c;
	int chin;
	
    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MSL] )
    {
		bugf( "Fread_string: MAX_STRING %d exceeded.\n"
			"**************: MAX_STRING can be increased in the %s file.", 
			MAX_STRING, GAMESETTINGS_FILE);
		log_area_import_format_notice();
		exit_error( 1 , "fread_string", "MAX_STRING exceeded");
    }
	
    /*
	* Skip blanks.
	* Read first char.
	*/
    do
    {
		c = getc( fp );
    }
    while ( is_space(c) );
	
    if ( ( *plast++ = c ) == '~' )
		return &str_empty[0];
	
    for ( ;; )
    {
	/*
	* Back off the char type lookup,
	*   it was too dirty for portability.
	*   -- Furey
		*/

		chin=getc(fp);
		*plast=chin;

		switch ( chin  )
		{
		default:
			plast++;
			break;
			
		case EOF:
			bug("Fread_string: EOF");
			return NULL;
			break;
			
		case '\n':
			plast++;
			*plast++ = '\r';
			break;
			
		case '\r':
			break;
			
		case '~':
			plast++;
			{
				union
				{
					char *      pc;
					char        rgc[sizeof(char *)];
				} u1;
				unsigned int ic;
				int iHash;
				char *pHash;
				char *pHashPrev;
				char *pString;
				
				plast[-1] = '\0';
				iHash     = UMIN( MAX_KEY_HASH - 1, (int)(plast - 1 - top_string) );
				for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
				{
					for ( ic = 0; ic < sizeof(char *); ic++ )
						u1.rgc[ic] = pHash[ic];
					pHashPrev = u1.pc;
					pHash    += sizeof(char *);
					
					if ( top_string[sizeof(char *)] == pHash[0]
						&&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
						return pHash;
				}
				
				if ( fBootDb )
				{
					pString             = top_string;
					top_string          = plast;
					u1.pc               = string_hash[iHash];
					for ( ic = 0; ic < sizeof(char *); ic++ )
						pString[ic] = u1.rgc[ic];
					string_hash[iHash]  = pString;
					
					nAllocString += 1;
					sAllocString += (int)(top_string - pString);
					return pString + sizeof(char *);
				}
				else
				{
					return str_dup( top_string + sizeof(char *) );
				}
			}
		}
    }
}

/**************************************************************************/
char *fread_string_eol( FILE *fp )
{
    static bool char_special[256-EOF];
    char *plast;
    char c;
	
    if ( char_special[EOF-EOF] != true )
    {
		char_special[EOF -  EOF] = true;
		char_special['\n' - EOF] = true;
		char_special['\r' - EOF] = true;
    }
	
    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MSL] )
    {
		bugf( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
		exit_error( 1 , "fread_string_eol", "MAX_STRING exceeded");
    }
	
    /*
	* Skip blanks.
	* Read first char.
	*/
    do
    {
		c = getc( fp );
    }
    while ( is_space(c) );
	
    if ( ( *plast++ = c ) == '\n')
		return &str_empty[0];
	
    for ( ;; )
    {
		if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
			continue;
		
		switch ( plast[-1] )
		{
		default:
			break;
			
		case EOF:
			bug("Fread_string_eol - EOF");
			exit_error( 1 , "fread_string_eol", "unexpected EOF");
			break;
			
		case '\n':  case '\r':
			{
				union
				{
					char *      pc;
					char        rgc[sizeof(char *)];
				} u1;
				unsigned int ic;
				int iHash;
				char *pHash;
				char *pHashPrev;
				char *pString;
				
				plast[-1] = '\0';
				iHash     = UMIN( MAX_KEY_HASH - 1, (int)(plast - 1 - top_string) );
				for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
				{
					for ( ic = 0; ic < sizeof(char *); ic++ )
						u1.rgc[ic] = pHash[ic];
					pHashPrev = u1.pc;
					pHash    += sizeof(char *);
					
					if ( top_string[sizeof(char *)] == pHash[0]
						&&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
						return pHash;
				}
				
				if ( fBootDb )
				{
					pString             = top_string;
					top_string          = plast;
					u1.pc               = string_hash[iHash];
					for ( ic = 0; ic < sizeof(char *); ic++ )
						pString[ic] = u1.rgc[ic];
					string_hash[iHash]  = pString;
					
					nAllocString += 1;
					sAllocString += (int)(top_string - pString);
					return pString + sizeof(char *);
				}
				else
				{
					return str_dup( top_string + sizeof(char *) );
				}
			}
		}
    }
}



/**************************************************************************/
// Read to end of line (for comments).
void fread_to_eol( FILE *fp )
{
    char c;

	if (feof(fp))
		return;

    do
    {
		c = getc( fp );
    }
    while ( c != '\n' && c != '\r' && !feof(fp));

    do
    {
		c = getc( fp );
    }
    while ( (c == '\n' || c == '\r') && !feof(fp));

    ungetc( c, fp );
    return;
}


/**************************************************************************/
// Read one word (into static buffer).
char *fread_word( FILE *fp )
{
	static int i;  
    static char word[5][MIL];
    char *pword;
    char cEnd;

	// rotate buffers
	++i= i%5;
    word[i][0] = '\0';
	
    do
    {
		cEnd = getc( fp );
    }
    while ( is_space( cEnd ) );
	
    if ( cEnd == '\'' || cEnd == '"' )
    {
		pword   = word[i];
    }
    else
    {
		word[i][0] = cEnd;
		pword   = word[i]+1;
		cEnd    = ' ';
    }
	
    for ( ; pword < word[i] + MIL; pword++ )
    {
		*pword = getc( fp );
		if ( feof(fp) || cEnd == ' ' ? is_space(*pword) : *pword == cEnd )
		{
			if ( cEnd == ' ' )
				ungetc( *pword, fp );
			*pword = '\0';
			return word[i];
		}
    }
	
    ungetc( *pword, fp );
    *pword = '\0';
    bug("Fread_word: word too long.");
    bugf( "'%s'", word[i]);

	// reverse read in by one
	i= (i+4)%5;
	bug( "previous word:");
    bugf( "'%s'", word[i]);

	log_area_import_format_notice();

    exit_error( 1 , "fread_word", "word too long");
    return NULL;
}

/**************************************************************************/
/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;

    while ( sMem % sizeof(long) != 0 )
	sMem++;
    if ( sMem > MAX_PERM_BLOCK )
    {
		bugf( "Alloc_perm: %d too large.", sMem );
		do_abort();
		exit_error( 1 , "alloc_perm", "too large");
    }

    if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
		iMemPerm = 0;
		if ( ( pMemPerm = (char *)calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
		{
			bugf("alloc_perm(): error with calloc(1,%d) call - error %d (%s)",
				MAX_PERM_BLOCK, errno, strerror( errno));
			do_abort();
			exit_error( 1 , "alloc_perm", "calloc returned error");
		}
    }

    pMem        = pMemPerm + iMemPerm;
    iMemPerm   += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
#ifdef MEM_DEBUG
	if (log_memory){
		logf("alloc_perm(): nAllocPerm = %d, sAllocPerm =%d"
			,nAllocPerm, sAllocPerm );
	}
#endif
    return pMem;
}



/**************************************************************************/
/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup( const char *str )
{
    char *str_new;

    if ( IS_NULLSTR(str))
	return &str_empty[0];

    if ( str >= string_space && str < top_string )
	return (char *) str;

    str_new = (char *) alloc_mem( str_len(str) + 1 );
    strcpy( str_new, str );
    return str_new;
}



/**************************************************************************/
/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr )
{
    if ( pstr == NULL
    ||   pstr == &str_empty[0]
    || ( pstr >= string_space && pstr < top_string ) )
	return;

    free_mem( pstr, str_len(pstr) + 1 );
    return;
}
/**************************************************************************/
char *get_areaname(char_data *ch, AREA_DATA *pArea, bool longformat)
{
    static char buf[MSL];
    char lrange[MSL];

	strcpy( lrange, "???????");
	if (!IS_NULLSTR(pArea->lcomment))
	{
		strcpy( lrange, pArea->lcomment);
	}
	else if (pArea->low_level > -1)
	{
		sprintf( lrange, "[%3d-%3d]", pArea->low_level, pArea->high_level);
	}

	char aname[MSL];
	char areaname[MSL];
	// get the first part of the areaname
	if(ch->in_room && (ch->in_room->area==pArea) && longformat){
		sprintf(aname,"`Y*`%-1.1s`#%s",pArea->colour, pArea->name);
	}else{
		sprintf(aname,"`%-1.1s`#%s",pArea->colour, pArea->name);
	}


	if(longformat){
		// add the olc part and get it to the correct width
		if (IS_SET(pArea->area_flags, AREA_OLCONLY)){
			if(c_str_len(aname)>30){
				sprintf(areaname,"%s `g(OLC)`x", str_width(aname,30, true));
			}else{
				strcat(aname," `g(OLC)`x");				
				sprintf(areaname,"%s", str_width(aname,36, true));
			}
		}else{
			sprintf(areaname,"%s`x",str_width(aname,36,true));
		}
	}else{
		strcpy(areaname, aname);
	}

	if(IS_IMMORTAL(ch))
	{
		char copybuf[MIL];
		sprintf(copybuf,"[%3d,%2d,%d]%s", pArea->vnum, 
			pArea->maplevel,pArea->mapscale, areaname);
		strcpy(areaname,copybuf);
	}


	if(longformat){
		sprintf( buf, "%s%s `^%s`x",
			areaname,
			(IS_NULLSTR(pArea->credits)?
				str_width(pArea->builders,20, true):str_width(pArea->credits, 20, true)),
			str_width(lrange, 12,true));
	}else{
		sprintf( buf, "%s %s`x ",
			areaname,
			lrange);
	}

	// display hidden flag if necessary 
	if (IS_SET(pArea->area_flags, AREA_HIDDEN)){
		strcat(buf,"(H)\r\n");
	}else{
		strcat(buf,"\r\n");
	}

	return buf;
}
/**************************************************************************/
void do_areasalpha( char_data *ch, char *argument )
{
    BUFFER *output;
    AREA_DATA *pArea;
    int linenum=0, head=0;

    output = new_buf();

	bool matched;		
	for ( pArea = area_arealist_first; pArea; pArea = pArea->arealist_sort_next)
    {
		// filter in only required areas
		if(!IS_NULLSTR(argument)){
			matched=false;
			if (is_name( argument, pArea->name )) {
				matched=true;
			}
			if(IS_NULLSTR(pArea->credits)){
				if(is_name( argument, pArea->builders)){
					matched=true;
				}
			}else{
				if(is_name( argument, pArea->credits)){
					matched=true;
				}
			}
			// matching on olc only areas
			if ( !str_cmp(argument,"olc") && IS_SET(pArea->area_flags, AREA_OLCONLY) ) {
				matched=true;
			}
			if(!matched){
				continue;
			}
		}

		// hidden areas aren't seen by morts on the arealist
		if ((!IS_IMMORTAL(ch) || HAS_CONFIG(ch,CONFIG_HIDE_HIDDEN_AREAS)) 
			&& IS_SET(pArea->area_flags, AREA_HIDDEN)){
			continue;
		}

		if ((ch->lines && linenum%ch->lines==0) || linenum==0) // print header once every page 
		{
			if(IS_IMMORTAL(ch)){ // mapscale info
				add_buf( output, "[ANum, Maplevel, Scale]=Area Name============-Credits-========- Recommend Level Range -==\r\n");
			}else{
				add_buf( output, "Area Name-=========================-Credits-========- Recommend Level Range -==\r\n");
			}
			if(linenum==0){ 
				if(ch->in_room && ch->in_room->area){
					// hidden areas aren't seen by morts on the arealist				
					if (!IS_SET(ch->in_room->area->area_flags, AREA_HIDDEN)||IS_IMMORTAL(ch)){
						char wbuf[MIL];
						sprintf(wbuf,"You are currently in %s`x", get_areaname(ch, ch->in_room->area, false));
						add_buf( output, wbuf);
					}else{
						add_buf( output, "You are currently in an unknown area.\r\n");
					}
				}else{
					add_buf( output, "You are currently in an unknown area!.\r\n");
				}
				linenum++;
			}
			linenum++;
			head++;
			if (head%2==0){
				linenum++;
			}
		}
	
		add_buf( output, get_areaname(ch, pArea, true));
		linenum++;
    }

	if (linenum==0){
		add_buf( output, "No matching areas found!\r\n");
	}
	if(IS_NEWBIE(ch)){
		add_buf( output, 
			FORMATF("`xUse the command `=C%s`x for a list of areas sorted by level range.\r\n",
				mxp_create_send(ch, "areas"))
			);
	}
    ch->sendpage(buf_string(output));
    free_buf(output);

    return;
}
/**************************************************************************/
void do_areas( char_data *ch, char *argument )
{
    BUFFER *output;
    AREA_DATA *pArea;
    int linenum=0, head=0;

    output = new_buf();

	bool matched;		
	for ( pArea = area_levelsort_first; pArea; pArea = pArea->levelsort_next)
    {
		// filter in only required areas
		if(!IS_NULLSTR(argument)){
			matched=false;
			if (is_name( argument, pArea->name )) {
				matched=true;
			}
			if(IS_NULLSTR(pArea->credits)){
				if(is_name( argument, pArea->builders)){
					matched=true;
				}
			}else{
				if(is_name( argument, pArea->credits)){
					matched=true;
				}
			}
			// matching on olc only areas
			if ( !str_cmp(argument,"olc") && IS_SET(pArea->area_flags, AREA_OLCONLY) ) {
				matched=true;
			}
			if(!matched){
				continue;
			}
		}

		// hidden areas aren't seen by morts on the arealist
		if ((!IS_IMMORTAL(ch) || HAS_CONFIG(ch,CONFIG_HIDE_HIDDEN_AREAS)) 
			&& IS_SET(pArea->area_flags, AREA_HIDDEN)){
			continue;
		}

		if ((ch->lines && linenum%ch->lines==0) || linenum==0) // print header once every page 
		{
			if(IS_IMMORTAL(ch)){ // mapscale info
				add_buf( output, "[ANum, Maplevel, Scale]=Area Name============-Credits-========- Recommend Level Range -==\r\n");
			}else{
				add_buf( output, "Area Name-=========================-Credits-========- Recommend Level Range -==\r\n");
			}
			if(linenum==0){ 
				if(ch->in_room && ch->in_room->area){
					// hidden areas aren't seen by morts on the arealist				
					if (!IS_SET(ch->in_room->area->area_flags, AREA_HIDDEN)||IS_IMMORTAL(ch)){
						char wbuf[MIL];
						sprintf(wbuf,"You are currently in %s`x", get_areaname(ch, ch->in_room->area, false));
						add_buf( output, wbuf);
					}else{
						add_buf( output, "You are currently in an unknown area.\r\n");
					}
				}else{
					add_buf( output, "You are currently in an unknown area!.\r\n");
				}
				linenum++;
			}
			linenum++;
			head++;
			if (head%2==0){
				linenum++;
			}
		}
	
		add_buf( output, get_areaname(ch, pArea, true));
		linenum++;
    }

	if (linenum==0){
		add_buf( output, "No matching areas found!\r\n");
	}
	if(IS_NEWBIE(ch)){
		if(number_range(0,1)==0){
			add_buf( output, FORMATF("Note: the area list is available alphabetically using the `=C%s`x command\r\n", mxp_create_send(ch, "areasalpha")));
		}else{
			add_buf( output, FORMATF("Note: You can filter the area list by typing `=Careas <part of an areaname>`x\r\ne.g. `=C%s`x\r\n", mxp_create_send(ch, "areas vil")));
		}
	}
    ch->sendpage(buf_string(output));
    free_buf(output);

    return;
}
/**************************************************************************/
extern int rooms_with_tracks;
extern int total_tracked_characters;
extern int events_queued_count;
extern int events_queued_total;
extern char max_count_ip_buf[MIL];
extern int max_count_ip;
char *netio_return_binded_sockets();
extern char current_logfile_name[MSL];
/**************************************************************************/
void do_memory( char_data *ch, char *)
{
	// function prototypes - in laston.c
	char *timediff(time_t, time_t);
	char *short_timediff(time_t, time_t);

	do_compile_time(ch,"");
	ch->printlnf( " Seconds past 1Jan1970 = %d", (unsigned int)current_time);
	ch->printlnf(" Next maintence save in %d tick%s",
		resaveCounter, resaveCounter==1?"":"s");
	ch->printlnf(" %s", PLATFORM_INFO);
	ch->printlnf(" Affects %5d", top_affect		);
	ch->printlnf(" Areas   %5d", top_area		);
	ch->printlnf(" ExDes   %5d", top_ed			);
	ch->printlnf(" Exits   %5d", top_exit		);
	ch->printlnf(" Helps   %5d", top_help		);
	ch->printlnf(" Socials %5d", social_count	);
	ch->printlnf(" Mobs    %5d (%d in use)", top_mob_index, mobile_count);
	ch->printlnf(" Objs    %5d", top_obj_index);
	ch->printlnf(" Resets  %5d", top_reset     );
	ch->printlnf(" Rooms   %5d", top_room      );
	ch->printlnf(" RoomsWithTracks  %5d (%d bytes)", 
		rooms_with_tracks, (int)(sizeof(C_track_data)*rooms_with_tracks));
	ch->printlnf(" total_tracked_characters  %5d", track_table->get_total_tracked_characters());
	ch->printlnf(" Shops   %5d", top_shop      );
	ch->printlnf(" Inns    %5d", top_inn       );
	ch->printlnf(" Tick_Counter   %ld", tick_counter ); 
    ch->printlnf(" Strings %5d strings of %7d bytes (max %d).",
		nAllocString,
		sAllocString,
		MAX_STRING );
	ch->printlnf(" Perms   %5d blocks  of %7d bytes.",
		nAllocPerm,
		sAllocPerm );

	// display stats about most common ip, and how many on it
	max_count_ip_calc();
	ch->printlnf(" most common ip=%d on %s", 
		max_count_ip, max_count_ip_buf);

	ch->printlnf(" events_queued_count=%d, events_queued_total=%d",
		events_queued_count, events_queued_total);

	ch->printlnf(" at bootup: lockers_total_count=%d, lockers_object_count=%d",
		lockers_total_count, lockers_object_count);
   
	ch->printlnf(" Current working directory is %s.", get_current_working_directory());

	ch->printlnf(" HostName = %s.", MACHINE_NAME);
	ch->printlnf(" MainPort = %d.", mainport);
	ch->printlnf(" Current process ID is %d.", getpid());

	if(resolver_running){
		ch->printlnf(" The remote resolver is running (version %d.%03d detected).", 
			resolver_version/1000, resolver_version%1000);
	}else{
		ch->println(" The remote resolver is not currently running.");
	}

	ch->printlnf(" minute = %d, hour = %d, day = %d, month = %d, year = %d",
		time_info.minute, time_info.hour,	time_info.day, 
		time_info.month, time_info.year);

	ch->printlnf(" day modifier = %2d/36 moon mod = %d",
		time_info.day+1, (abs(time_info.day-17)- 9)*2/3);

	ch->printlnf(" month modifier = %2d/12 moon mod = %d",
		time_info.month+1, (abs(time_info.month-5)- 3)/2);

	{
		int cm;
		
		cm = ((abs(time_info.day-17)- 9))*2/3 + ((abs(time_info.month-5)- 3)/2);
		if (time_info.day==35) // peak modifier
			cm++;
		if (time_info.month==11) // peak modifier
			cm++;
		ch->printlnf(" base cast modifier = %d", cm);

		ch->printlnf(" real cast modifier= %d", weather_info[ch->in_room->sector_type].mage_castmod );
	
	}

	ch->printlnf(" Endlessloop abort settings: boot_db=%d, running=%d, dns=%d, frequency=%d",
		game_settings->alarm_boot_db_abort_threshold,
		game_settings->alarm_running_abort_threshold,
		game_settings->alarm_running_dns_abort_threshold,
		game_settings->alarm_frequency);

	
	ch->printlnf(" sizeof(char)=%d",	(int)sizeof(char));
	ch->printlnf(" sizeof(sh_int)=%d",	(int)sizeof(sh_int));
	ch->printlnf(" sizeof(int)=%d",		(int)sizeof(int));
	ch->printlnf(" sizeof(long)=%d",	(int)sizeof(long));
	ch->printlnf(" sizeof(char*)=%d",	(int)sizeof(char*));
	
	char *binfo=str_dup(netio_return_binded_sockets());
	binfo=string_replace_all(binfo, "][", "]`1     [");
	ch->printlnf(" Binded socket info:\r\n     %s", binfo);
	free_string(binfo);

	ch->printlnf(" Currently logging to '%s'", current_logfile_name);
	return;
}

/**************************************************************************/
void do_dumpstats( char_data *ch, char *)
{
    int count,count2,num_pcs,aff_count;
    char_data *fch;
    MOB_INDEX_DATA *pMobIndex;
    PC_DATA *pc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    connection_data *d;
    AFFECT_DATA *af;
    FILE *fp;
    int vnum;

	ch->titlebar("Dumping memory details to mem.dmp");

    /* open file */
    fclose(fpReserve);
    fp = fopen("mem.dmp","w");

    /* report use of data structures */

    num_pcs = 0;
    aff_count = 0;

    /* mobile prototypes */
    fprintf(fp,"MobProt %4ld (%8ld bytes)\n",
	(long) top_mob_index, (long) (top_mob_index * (sizeof(*pMobIndex))));

    /* mobs */
    count = 0;  count2 = 0;
    for (fch = char_list; fch != NULL; fch = fch->next)
    {
	count++;
	if (fch->pcdata != NULL)
	    num_pcs++;
	for (af = fch->affected; af != NULL; af = af->next)
	    aff_count++;
    }
    for (fch = char_free; fch != NULL; fch = fch->next)
	count2++;

    fprintf(fp,"Mobs    %4ld (%8ld bytes), %2ld free (%ld bytes)\n",
	(long) count, (long) (count * (sizeof(*fch))),
	(long) count2, (long) (count2 * (sizeof(*fch))));

    // pcdata 
    count = 0;
    for (pc = pcdata_free; pc != NULL; pc = pc->next){
		count++;
	}

    fprintf(fp,"Pcdata  %4d (%8ld bytes), %2d free (%ld bytes)\n",
	num_pcs, (long) (num_pcs * (sizeof(*pc))),
	count, (long) (count * (sizeof(*pc))));

    // connections 
    count = 0; count2 = 0;
    for (d = connection_list; d != NULL; d = d->next){
		count++;
	}
    for (d= connection_free; d != NULL; d = d->next){
		count2++;
	}

    fprintf(fp, "Connection structures %4ld (%8ld bytes), %2ld free (%ld bytes)\n",
	(long) count,  (long) (count * (sizeof(*d))),
	(long) count2, (long) (count2 * (sizeof(*d))));

    // object prototypes 
    for ( vnum = 0; vnum<game_settings->olc_max_vnum; vnum++ )
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    for (af = pObjIndex->affected; af != NULL; af = af->next)
		aff_count++;
	}

    fprintf(fp,"ObjProt %4ld (%8ld bytes)\n",
	(long)top_obj_index, (long)(top_obj_index * (sizeof(*pObjIndex))));


    /* objects */
    count = 0;  count2 = 0;
    for (obj = object_list; obj != NULL; obj = obj->next)
    {
	count++;
	for (af = obj->affected; af != NULL; af = af->next)
	    aff_count++;
    }
    for (obj = obj_free; obj != NULL; obj = obj->next)
	count2++;

    fprintf(fp,"Objs    %4ld (%8ld bytes), %2ld free (%ld bytes)\n",
	(long) count,  (long) (count * (sizeof(*obj))),
	(long) count2, (long) (count2 * (sizeof(*obj))));

    /* affects */
    count = 0;
    for (af = affect_free; af != NULL; af = af->next)
	count++;

    fprintf(fp,"Affects %4ld (%8ld bytes), %2ld free (%ld bytes)\n",
	(long) aff_count,   (long) (aff_count * (sizeof(*af))),
	(long) count,       (long) (count * (sizeof(*af))));

    // rooms 
    fprintf(fp,"Rooms   %4ld (%8ld bytes)\n",
	(long) top_room, (long) (top_room * (sizeof(ROOM_INDEX_DATA))));

    // exits 
    fprintf(fp,"Exits   %4ld (%8ld bytes)\n",
	(long) top_exit, (long) (top_exit * (sizeof(EXIT_DATA))));

    fclose(fp);

    // start printing out mobile data 
	ch->titlebar("Dumping mobile details to mob.dmp");
    fp = fopen("mob.dmp","w");

    fprintf(fp,"\nMobile Analysis\n");
    fprintf(fp,  "---------------\n");
    for (vnum = 0; vnum<game_settings->olc_max_vnum; vnum++)
	if ((pMobIndex = get_mob_index(vnum)) != NULL)
	{
	    fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
		pMobIndex->vnum,pMobIndex->count,
		pMobIndex->killed,pMobIndex->short_descr);
	}
    fclose(fp);

    // start printing out object data 
	ch->titlebar("Dumping object details to obj.dmp");
    fp = fopen("obj.dmp","w");

    fprintf(fp,"\nObject Analysis\n");
    fprintf(fp,  "---------------\n");
    for (vnum = 0; vnum<game_settings->olc_max_vnum; vnum++)
	if ((pObjIndex = get_obj_index(vnum)) != NULL)
	{
	    fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
		pObjIndex->vnum,pObjIndex->count,
		pObjIndex->reset_num,pObjIndex->short_descr);
	}

    // close file 
    fclose(fp);
    fpReserve = fopen( NULL_FILE, "r" );
}


/**************************************************************************/
/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_range(0, 3) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}

/**************************************************************************/
// Generate a random number.
int number_range( int from, int to )
{
    int power;
    int number;

    if (from == 0 && to == 0)
	return 0;

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm() & (power -1 ) ) >= to )
	;

    return from + number;
}

/**************************************************************************/
// Generate a percentile roll (between 1 and 100).
int number_percent( void )
{
    int percent;

    while ( (percent = number_mm() & (128-1) ) > 99 )
	;

    return 1 + percent;
}


/**************************************************************************/
// Generate a random door.
int number_door( void )
{
    int door;

    while ( ( door = number_mm() & (16-1) ) >= MAX_DIR)
	;
    return door;
}
/**************************************************************************/
/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
   back to the system srandom call.  If this doesn't work for you, 
   define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif
 
/**************************************************************************/
void init_mm( )
{
#if defined (OLD_RAND)
    int *piState;
    int iState;
 
    piState     = &rgiState[2];
 
    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;
 
    piState[0]  = ((int) current_time) & ((1 << 30) - 1);
    piState[1]  = 1;
    for ( iState = 2; iState < 55; iState++ )
    {
	piState[iState] = (piState[iState-1] + piState[iState-2])
			& ((1 << 30) - 1);
    }
#else
    srandom(time(NULL)^getpid());
#endif
    return;
}
 
/**************************************************************************/
long number_mm( void )
{
#if defined (OLD_RAND)
    int *piState;
    int iState1;
    int iState2;
    int iRand;
 
    piState             = &rgiState[2];
    iState1             = piState[-2];
    iState2             = piState[-1];
    iRand               = (piState[iState1] + piState[iState2])
			& ((1 << 30) - 1);
    piState[iState1]    = iRand;
    if ( ++iState1 == 55 )
	iState1 = 0;
    if ( ++iState2 == 55 )
	iState2 = 0;
    piState[-2]         = iState1;
    piState[-1]         = iState2;
    return iRand >> 6;
#else
    return random() >> 6;
#endif
}


/**************************************************************************/
// Roll some dice.
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}


/**************************************************************************/
// Simple linear interpolation.
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}
/**************************************************************************/



/**************************************************************************/
/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; str && *str != '\0'; str++ )
    {
	if ( *str == '~' )
	*str = '-';
    }
    return;
}

/**************************************************************************/
/*
 * Hides the tildes in a string with ascii code 173 (­).
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
	if(!str){
		return;
	}

    for ( ; *str; str++ )
    {
		if ( *str == '~' ){
			*str = '­';
		}
	}
    return;
}

/**************************************************************************/
/*
 * Show the tildes in a string with ascii code 173 (­).
 * Used for player-entered strings that go into disk files.
 */
void show_tilde( char *str )
{
	if(!str){
		return;
	}

    for ( ; *str; str++ )
    {
		if ( *str == '­' ){
			*str = '~';
		}
    }
    return;
}


/**************************************************************************/
/*
 * Compare strings, case insensitive.
 * Return true if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
		bug("Str_cmp: null astr.");
		return true;
    }
	
    if ( bstr == NULL )
    {
		bug("Str_cmp: null bstr.");
		return true;
    }
	
    for ( ; *astr || *bstr; astr++, bstr++ )
    {
		if ( LOWER(*astr) != LOWER(*bstr) )
			return true;
    }
	
    return false;
}


/**************************************************************************/
/*
 * Compare strings, case insensitive, for prefix matching.
 * Return true if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *prefix, const char *fullstring)
{
    if ( prefix == NULL )
    {
	bug("str_prefix: null prefix.");
	return true;
    }

    if ( fullstring == NULL )
    {
	bug("str_prefix: null fullstring.");
	return true;
    }

    for ( ; *prefix; prefix++, fullstring++ )
    {
	if ( LOWER(*prefix) != LOWER(*fullstring) )
	    return true;
    }

    return false;
}



/**************************************************************************/
/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns true is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *substring, const char *contained_within )
{
	if(IS_NULLSTR(contained_within )){
		return true;
	}
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER(substring[0]) ) == '\0' )
	return false;

    sstr1 = str_len(substring);
    sstr2 = str_len(contained_within);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ ){
	if ( c0 == LOWER(contained_within[ichar]) 
		&& !str_prefix( substring, contained_within + ichar ) )
	    return false;
    }

    return true;
}



/**************************************************************************/
/*
 * Compare strings, case insensitive, for suffix matching.
 * Return true if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = str_len(astr);
    sstr2 = str_len(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return false;
    else
	return true;
}



/**************************************************************************/
/*
 * Returns an initial-capped string.
 * - rest of the string lowercase
 * - multibuffered with 6 buffers
 */
char *capitalize( const char *str )
{
	static char strcap[6][MSL];
    int i;
	static int index; // index

	// don't crash when given NULL
	if(!str){
		return "";
	}
	// rotate buffers 
	++index= index%6;

	bool first_char=true;

    for ( i = 0; str[i] != '\0'; i++ ){
		if(str[i]=='`'){ // we have a colour code
			strcap[index][i] = '`'; // copy the colour code marker to the output
			i++;					// move onto the colour code
			switch(str[i]){			// depending what it is take action
				case '\0': i--; break;// end of string - backup one and let loop terminate
				case '=': // custom colour code beginning 
					strcap[index][i] = '='; // copy the '='
					i++;
					if(str[i]=='\0'){
						i--; // end of string - backup one and let loop terminate
					}else{
						strcap[index][i] = str[i]; // copy the custom colour code
					}
					break;
				default: // colour code
					strcap[index][i] = str[i]; // copy the colour code
					break;
			}
		}else{
			if(first_char && is_alpha(str[i])){
				strcap[index][i] = UPPER(str[i]); // first non colour code character - uppercase
				first_char=false;
			}else{
				strcap[index][i] = LOWER(str[i]); // not first, non colour code character - lowercase
			}
		}
	}
    strcap[index][i] = '\0';

    return strcap[index];
}

/**************************************************************************/
/*
 * Returns an initial-capped string.
 * nothing else affected
 */
char *icapitalize( const char *str )
{
	static char strcap[6][MSL];
    int i;
	static int index; // index

	// don't crash when given NULL
	if(!str){
		return "";
	}
	// rotate buffers 
	++index= index%6;

	bool first_char=true;

    for ( i = 0; str[i] != '\0'; i++ ){
		if(str[i]=='`'){ // we have a colour code
			strcap[index][i] = '`';
			i++;
			switch(str[i]){
				case '\0': i--; break;// end of string - backup one and let loop terminate
				case '=': // custom colour code beginning 
					if(str[i+1]!='\0'){
						strcap[index][i++] = '='; // copy the = in the custom colour code
						strcap[index][i] = str[i]; // copy the custom colour code
					}else{
						// we have a null following the =, just copy the =						
						strcap[index][i] = '=';						
					}
					break;
				default: // colour code
					strcap[index][i] = str[i]; // copy the colour code
					break;
			}
		}else{
			if(first_char && is_alpha(str[i])){
				strcap[index][i] = UPPER(str[i]); // first non colour code character - uppercase				
				strcpy(&strcap[index][i+1],&str[i+1]);// copy just the rest
				return strcap[index];				
			}else{
				strcap[index][i] = str[i]; // not first, non colour code character - copy
			}
		}
	}
    strcap[index][i] = '\0';

    return strcap[index];
}

/**************************************************************************/
// Returns a left trimed string after considering colour codes
// Kal - Jan 98
char *ltrim_string( const char *str )
{
    static char lstr[MSL*2];
    int i, j;
    bool straightcopy= false;

    j = 0;

    for ( i = 0; str[i] != '\0'; i++ )
    {
        if (straightcopy)
        {
            lstr[j]= str[i];
            j++;
            continue;                      
        }

        // loop thru coping colour codes
        while ( str[i] == '`') 
        {
            lstr[j]= str[i];   // copy ` colour symbol

            i++;          
            j++;
            lstr[j]= str[i];   // copy colour code

            i++;
            j++;           
        }

        if (!is_space(str[i]))
        {
            lstr[j]= str[i];    // copy the first nonspace
            j++;
            straightcopy= true; // mark all the rest for a straight copy
        }

    }

    lstr[j] = '\0'; // terminate the string

    return lstr;
}
/**************************************************************************/
// Trims all whitespace of the right of a string
// Kal - Apr 01
char *rtrim_string( const char *str )
{
    static char rstr[MSL*2];

	rstr[0]='\0'; // used to stop it if it is an empty string
	strcpy(rstr+1, str);

	char *end=&rstr[str_len(rstr+1)];
	while(*end && is_space(*end)){
		*end--='\0';
	}
    return rstr+1;
}

/**************************************************************************/
/*
 * Append a string to a file.
 */
void append_file( char_data *ch, char *file, char *str )
{
    FILE *fp;

    if ( !ch || IS_NPC(ch) || str[0] == '\0' )
	return;

    fclose( fpReserve );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
		bugf("append_file(): fopen '%s' for append - error %d (%s)",
			file, errno, strerror( errno));
		ch->printlnf("append_file(): could not open the file '%s'!", file);
    }else{
		fprintf( fp, "[%5d] %s: %s\n",
			ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
		fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/**************************************************************************/
char last_bug[MSL*4+1];

/**************************************************************************/
// Reports a bug.
void bug( const char *str)
{
    if ( fpArea != NULL )
    {
		int iLine;
		int iChar;
		
		if ( fpArea == stdin )
		{
			iLine = 0;
		}
		else
		{
			static int last_line_num=-2;
			static int last_iChar=-2;

			int counter=0;	
			iChar = ftell( fpArea );
			if(iChar==last_iChar){
				iLine=last_line_num;
			}else{
				fseek( fpArea, 0, 0);
				for ( iLine = 0; ftell( fpArea ) < iChar && counter<2000; iLine++ )
				{
					counter=0;
					
					while ( getc( fpArea ) != '\n'  && counter<2000)
						counter++;
					;
				}
				
				if (counter>=2000)
				{
					log_string_core(FORMATF("BUG [*****] FILE: %s LINE?: %d more than 2000 characters on line!\n", strArea, iLine ));
					log_string_core("-===If line number is near the bottom line, it could be that the file isn't\n");
					log_string_core("-===properly terminated, if so delete the bottom 2 lines, and retype them.\n");
				}
				fseek( fpArea, iChar, 0);

				last_line_num=iLine;
				last_iChar=iChar;
			}
		}
		logf("[*****] FILE: %s LINE: %d", strArea, iLine );
    }

	strncpy(last_bug, str, MSL*4);
	last_bug[MSL*4]='\0';
	
    logf("[*****] BUG: %s", str);
    return;
}



/**************************************************************************/
/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}
/**************************************************************************/
void load_gamble( FILE *fp )
{
	if ( !area_last )
	{
		bug("Load_gamble: no #AREA seen yet.");
		exit_error( 1 , "load_gamble", "#AREA not seen yet");
	}

    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        char            letter;

        switch ( letter = fread_letter( fp ) )
        {
        default:
            bugf( "Load_gamble: letter '%c' not *, M or S.", letter );
            exit_error( 1 , "load_gamble", "unexpected letter in input");

        case 'S':
            return;

        case '*':
            break;

        case 'M':
			int mobnum=fread_number ( fp );
			apply_area_vnum_offset( &mobnum);
			pMobIndex	= get_mob_index ( mobnum );
            pMobIndex->gamble_fun = gamble_lookup( fread_word( fp ));
            if ( pMobIndex->gamble_fun == 0 )
            {
                bugf( "Load_gamble: 'M': vnum %d, missing game.", pMobIndex->vnum );
                exit_error( 1 , "load_gamble", "missing game for mob");
            }
            break;
        }

        fread_to_eol( fp );
    }
}

/**************************************************************************/
void load_attunes( FILE *fp )
{
	if ( !area_last )
	{
		bug("Load_attunes: no #AREA seen yet.");
		exit_error( 1 , "load_attunes", "#AREA not seen yet");
	}

    for ( ; ; )
    {
		OBJ_INDEX_DATA *pObjIndex;
        char            letter;

		switch ( letter = fread_letter( fp ) )
		{
		default:
			bugf( "Load_attunes: letter '%c' not *, O or S.", letter );
            exit_error( 1 , "load_attunes", "unexpected letter in input");

        case 'S':
            return;

        case '*':
            break;

        case 'O':
			int vnum = fread_number( fp );
			apply_area_vnum_offset( &vnum);

			pObjIndex = get_obj_index ( vnum );
			pObjIndex->attune_flags = fread_flag( fp );

            if ( pObjIndex->attune_flags == 0 )
            {
                bugf( "Load_attunes: 'O': vnum %d.", pObjIndex->vnum );
                exit_error( 1 , "load_attunes", "empty attune flags");
            }
            break;
        }

        fread_to_eol( fp );
    }
}

/**************************************************************************/
void free_speedwalk( connection_data *d )
{
	if ( d && d->speedwalk_buf )
	{
		free_string( d->speedwalk_buf );
		d->speedwalk_buf  = NULL;
		d->speedwalk_head = NULL;
	}
	return;
}
/**************************************************************************/
void log_area_import_format_notice()
{
	if(!fpArea){
		return;
	}
	char buf[MSL];
	buf[0]='\0';
	for(int flag = 0; !IS_NULLSTR(area_import_format_types[flag].name); flag++){
		strcat( buf, "`1    ");
		strcat( buf, area_import_format_types[flag].name);		
	}

	log_notef("NOTE: If this is an area file that is able to be read in perfectly into "
		"another mud, then the most likely cause of this error message is that the area "
		"format is different from what dawn is expecting."
		"`1Dawn supports directly importing in area files with formats other than its native "
		"extendable format, but due to the large number of different formats, it "
		"is not possible for dawn to automatically detect which format it is reading."
		"`1You can tell dawn what format to read in non-native areas in as by adjusting "
		"the area import format flag within gameedit, or if you can't get the mud to "
		"boot, you can manually change the line in gameset.txt which reads 'area_import_format ?????~'"
		" - where ????? is one of the following:`1"
		"%s"
		"`1`1'stock' is able to read in stock rom with no modifications.  "
		"After an area has been successfully imported it is recommend you resave "
		"the area in dawns native format within the game.  Be aware that importing "
		"from another muds area file, will lose any flags which are not common to Dawn.", 
		buf);	
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
