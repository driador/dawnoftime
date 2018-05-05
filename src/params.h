/**************************************************************************/
// params.h - Most defined parameters for the game
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
#ifndef PARAMS_H
#define PARAMS_H

#ifndef MUD_NAME
	#define MUD_NAME (game_settings->gamename)
#endif

// DEFAULT MUD PORT 
// - The default PORT the mud runs on, and port offsets
#define default_mud_port (game_settings->port_default) // 4000 
// Note: The mud_port value defaults to default_mud_port but can be over ridden
// at bootup, e.g. 'dawn 9000' starts the mud with mud_port set to 9000.

#define DEFAULT_LISTEN_ON_TEXT "telnet://:+0,http://:+1"

// ADDITIONAL PORT VALUES
// The following port numbers are relative to the mud_port
// e.g. irc_port == mud_port + irc_port_offset
#define web_port_offset (game_settings->port_web_offset)  
#define irc_port_offset (game_settings->port_irc_offset)
#define mudftp_port_offset (game_settings->port_mudftp_offset) 

#ifndef DEFAULT_MSP_URL
	#define DEFAULT_MSP_URL "http://msp.url.needs.to.be.set.in.gameedit/download/msp/"
#endif
#define MSP_URL (game_settings->msp_url)


#ifndef DEFAULT_MAX_STRING
	#ifdef unix
		#define	DEFAULT_MAX_STRING 5340032 
	#else
		#define DEFAULT_MAX_STRING 3670016 
	#endif	
#endif
#define	MAX_STRING (game_settings->max_string)

#ifdef WIN32
	#ifndef NO_INITIAL_ALPHA_PFILEDIRS
		#define NO_INITIAL_ALPHA_PFILEDIRS
	#endif
#endif

#define DEFAULT_REALM_NAME "mythrin"
#define DEFAULT_BATTLELAG_TEXT "`RBattleLagged"

#ifndef DEFAULT_MUD_NAME
	// don't change this... instead use gameedit
	// to set the game name.
	#define DEFAULT_MUD_NAME "Dawn v1.69r based mud"
#endif

// A list of characters that invalid to accept from users to pass to system(), pipe() etc
#define INVALID_CHARACTERS_FROM_USERS_FOR_SYSTEM_CALLS ";<>{}[]`\"'\\/$|&*?()^#!~\r\n\t"

/*
* Data files used by the server.
*
* AREA_LIST contains a list of areas to boot.
* All files are read in completely at bootup.
* Most output files (bug, idea, typo, shutdown) are append-only.
*
* The NULL_FILE is held open so that we have a stream handle in reserve,
*   so players can go ahead and telnet to all the other descriptors.
* Then we close it whenever we need to open a file (e.g. a save file).
*/
#ifdef unix
	#define DIR_SYM "/"
	#define NULL_FILE       "/dev/null"		// To reserve one stream 
	#define ANULL_FILE      "/dev/null"		// reserves append_string_to_file stream
#else
	#define DIR_SYM "\\"
	#define ANULL_FILE      "nul"			// reserves append_string_to_file stream
	#define NULL_FILE       "nul"			// To reserve file streams
#endif

#define BRACKET_OPEN	"\x7b" //{
#define BRACKET_CLOSE	"\x7d" //}

// directories
#define AREA_DIR        "area"DIR_SYM       // default directory of areas
#define AREA_RIL_DIR	AREA_DIR"ril"DIR_SYM // default directory of areas room invite list
#define BACKUP_AREA_DIR	"bak_area"DIR_SYM	// stores the previous save of an area
#define BACKUP_AREA_RIL_DIR	BACKUP_AREA_DIR"ril"DIR_SYM	// stores the previous save of an area ril list
#define BACKUP_HELP_DIR	"bak_help"DIR_SYM	// stores the previous save of a helpfile
#define SYSTEM_DIR      "system"DIR_SYM     // game system parameters root directory
#define DATA_DIR        "data"DIR_SYM       // data files
#define HELP_DIR        "help"DIR_SYM       // default directory of helps
#define NOTES_DIR       "notes"DIR_SYM      // notes stored in this directory
#define PLAYER_DIR      "player"DIR_SYM     // root directory of player files
#define SCRIPTS_DIR		"scripts"DIR_SYM	// Scripts directory
#define AUTOSTAT_DIR    SYSTEM_DIR			// where the autostat templates live

// directories which aren't off the root 
#define CLASSES_DIR     SYSTEM_DIR"classes"DIR_SYM	// classes - stats on each class
#define DEITY_DIR		SYSTEM_DIR"deities"DIR_SYM	// deities
#define LANGUAGES_DIR   SYSTEM_DIR"languages"DIR_SYM// languages
#define REMORT_DIR      PLAYER_DIR "remort" DIR_SYM // where pfiles goto when remorting begins
#define SRC_DIR			"src"DIR_SYM	    // source code in this directory

// log directories
#define LOG_ROOT_DIR		"logs"DIR_SYM		// root directory of all logs
#define GAME_LOGS_DIR		LOG_ROOT_DIR"game"DIR_SYM		
#define OLC_LOGS_DIR		LOG_ROOT_DIR"olc"DIR_SYM		// logs olc 
#define CODE_LOGS_DIR		SRC_DIR"logs"DIR_SYM       		// code logs 
#define ADMIN_LOGS_DIR		LOG_ROOT_DIR"admin"DIR_SYM		// default logs directory
#define PLAYER_LOGS_DIR		LOG_ROOT_DIR"plogs"DIR_SYM		// Player logs go in here
#define RESTRING_LOGS_DIR   LOG_ROOT_DIR"rstrlogs"DIR_SYM	// Player restring logs go in here
#define CHANNELS_LOGS_DIR   LOG_ROOT_DIR"channels"DIR_SYM
#define SUPPORT_LOGS_DIR	LOG_ROOT_DIR"support"DIR_SYM	
#define IMMLOG_DIR			LOG_ROOT_DIR"immlogs"DIR_SYM	// Immortal logs 
#define PDIR_LOCKED     PLAYER_DIR"locked"DIR_SYM	// pfiles requiring unlock code
#define	PDIR_BUILDER	PLAYER_DIR"builder"DIR_SYM	// pfiles of those with olc access
#define	PDIR_TRUSTED	PLAYER_DIR"trusted"DIR_SYM	// pfiles of mortals with an imm trust
#define	PDIR_IMMORTAL	PLAYER_DIR"immortal"DIR_SYM	// immortal pfiles
#define LOCKED_PFILES_DIR PLAYER_DIR"locked"DIR_SYM	// pfiles requiring an email unlock code
#define RETIRED_DIR		PLAYER_DIR"retired"DIR_SYM	// manually retired pfiles
#define DEAD_DIR        PLAYER_DIR"dead"DIR_SYM		// Pkilled Player files
#define DELETE_DIR      PLAYER_DIR"deleted"DIR_SYM    // Players that delete above level 5

//MSP DIRECTORIES - ALL FILE NAMES USE / BECAUSE OF MSP CLIENT SPECIFICATIONS
#define MSP_DIR			"msp"DIR_SYM // Directory all sounds are contained off - usually a link to the web url
// below are the MSP directories which different things are held in
#define MSP_ACTION_DIR		"action"DIR_SYM		// actions (brew, quaff etc) 
#define MSP_COMBAT_DIR		"combat"DIR_SYM		// sounds of weapons, tripping etc
#define MSP_MUDPROG_DIR		"mudprog"DIR_SYM	// from msptochar mudprog call
#define MSP_ROOM_DIR		"room"DIR_SYM		// room sounds
#define MSP_SKILLS_DIR		"skills"DIR_SYM		// skills set in sedit
#define MSP_SPELLS_DIR		"spells"DIR_SYM		// spells set in sedit
#define MSP_WEATHER_DIR		"weather"DIR_SYM	// sounds of weather etc

// note spools
#define CHANGES_FILE    NOTES_DIR"chang.not"
#define IDEA_FILE       NOTES_DIR"ideas.not"
#define NEWS_FILE       NOTES_DIR"news.not"
#define NOTE_FILE       NOTES_DIR"notes.not"    // For 'notes'
#define SNOTE_FILE      NOTES_DIR"snotes.not"    // For 'snotes'
#define ANOTE_FILE      NOTES_DIR"anotes.not"    // For 'anotes'
#define INOTE_FILE      NOTES_DIR"inotes.not"    // For 'inotes'
#define PKNOTE_FILE     NOTES_DIR"pknotes.not"   // For 'pknotes'
#define MISC_FILE		NOTES_DIR"misc.not"		 // For 'misc' notes
#define PENALTY_FILE    NOTES_DIR"penal.not"

// === log files
#define DAWNLOG_INDEX_FILE		"dawnlogs.txt"	// Appended, explaining what each log is for
#define BAD_RESETS_FILE			ADMIN_LOGS_DIR"badreset.txt"	// Used by fix_resets
#define HOSTS_FILE				ADMIN_LOGS_DIR"hosts.txt"		// records all the hosts
#define MPBUG_FILE				OLC_LOGS_DIR"mpbugs.txt"		// For mudprog bugs 'mpinfo'
#define BOUNDSBUG_FILE			CODE_LOGS_DIR"boundbug.txt"		// For boundsbug login function 
#define BUG_FILE				CODE_LOGS_DIR"bugs.txt"			// For 'bug' and bug()
#define INTRO_DEBUG_FILE		CODE_LOGS_DIR"introdbg.txt"
#define MKILL_LOGFILE			OLC_LOGS_DIR"mkills.txt"		// logs mkills on pc's
#define OLC_LOGFILE				OLC_LOGS_DIR"olc_log.txt"		// logs general olc 
#define NTALK_LOGFILE			CHANNELS_LOGS_DIR"ntalk.txt"	// logs ntalk
#define OOC_LOGFILE				CHANNELS_LOGS_DIR"ooc.txt"		// logs ooc 
#define CHAT_LOGFILE			CHANNELS_LOGS_DIR"chat.txt"		// logs chat
#define IC_LOGFILE				CHANNELS_LOGS_DIR"ic.txt"		// logs ic 
#define CLANTALK_LOGFILE		CHANNELS_LOGS_DIR"clantalk.txt"	// logs clantalk
#define NEWBIE_HUH_FILE			SUPPORT_LOGS_DIR"newb_huh.txt"	// records huhs level 1 gets 
#define NEWBIE_SUPPORT_LOG_FILE SUPPORT_LOGS_DIR"nsup_log.txt"	// logs newbie support
#define NEWBIE_LOGFILE			CHANNELS_LOGS_DIR"newbie.txt"	// logs newbietalks
#define MOBLOG_LOGFILE			OLC_LOGS_DIR"moblog.txt"		// logs moblog_debugged mobs
#define NO_HELP_FILE			SUPPORT_LOGS_DIR"no_help.txt"	// records missed help entries
#define OBJ_REM_FILE			OLC_LOGS_DIR"obj_rem.log"		// rec object removed from pc
#define MULTILOG_FILE			ADMIN_LOGS_DIR"multilog.txt"	// For multilog detection
#define RESTRING_LOGFILE		RESTRING_LOGS_DIR"restring.log" // log restrings
#define REMOVEDHELP_FILE		OLC_LOGS_DIR"rem_help.txt"  // records help entries that are removed
#define SECURE_FILE				ADMIN_LOGS_DIR"secure.txt"
#define SENDLOG_FILE			SUPPORT_LOGS_DIR"send_log.txt"
#define SHUTDOWN_FILE			ADMIN_LOGS_DIR"shutdown.txt" // For 'shutdown'
#define TYPO_FILE				SUPPORT_LOGS_DIR"typos.txt"    // For 'typo'
#define EMAILADDRESSES_FILE		ADMIN_LOGS_DIR"emailadd.txt"	// email addresses for players
#define CONNECTS_FILE			ADMIN_LOGS_DIR"connects.txt"	// host names of those connecting
#define UPDATE_MAGIC_FILE		ADMIN_LOGS_DIR"upmagic.txt"		// records the magic update
#define LASTON_DELETE_LOGFILE	ADMIN_LOGS_DIR"deletes.txt"		// records all the deleters details
#define CLANBANKING_FILE		ADMIN_LOGS_DIR"clanbank.txt"	// records clan bank transactions

// === data files 
#define BAN_FILE        DATA_DIR"ban.txt"
#define CLAN_FILE       DATA_DIR"clans.txt"
#define DISABLED_FILE	DATA_DIR"disabled.txt"  // disabled commands 
#define INTRODB_FILE	DATA_DIR"intro_db.txt"	// Introduction database
#define LETGAINDB_FILE	DATA_DIR"letgains.txt"  // records the letgains info
#define QUEST_FILE		DATA_DIR"quests.txt"	// quests database
#define OFFMOOT_FILE	DATA_DIR"offmoot.txt"	// offmoot database
#define LOCKERS_FILE	DATA_DIR"lockers.txt"	// lockers database
#define CORPSES_FILE	DATA_DIR"corpses.txt"	// Corpses Save database ( used in hreboot etc )

// === system files
#define CLASSES_LIST    SYSTEM_DIR"classes.txt"	// the list of all classes 
#define SKILLS_FILE		SYSTEM_DIR"skills.txt"
#define SKILLGROUPS_FILE SYSTEM_DIR"skgroups.txt"
#define RACES_FILE		SYSTEM_DIR "races.txt"
#define CONTINENTS_FILE	SYSTEM_DIR "continen.txt"
#define SOCIAL_FILE		SYSTEM_DIR"socials.txt"
#define COMMANDS_FILE				SYSTEM_DIR"commands.txt"
#define COMMANDS_CATEGORIES_FILE	SYSTEM_DIR"com_cats.txt"
#define COLOUR_TEMPLATES_FILE		SYSTEM_DIR"colours.txt"
#define NAME_PROFILES_FILE			SYSTEM_DIR"namegen.txt"
#define	DEITY_FILE		DEITY_DIR"deity.txt"	// deity list
#define MIX_FILE		SYSTEM_DIR"mix.txt"		// mix database
#define HERB_FILE		SYSTEM_DIR"herblist.txt"	// herb db
#define LANGUAGES_INDEX_FILE LANGUAGES_DIR"languages.txt"	// list of all language names

// === general files
#define DYNAMIC_INCLUDE "1dyntable.cpp"			// Dynamically written source file
#define AREA_LIST       AREA_DIR"arealist.txt"	// List of areas
#define HELP_LIST       HELP_DIR"helplist.txt"  // List of help files
#define LASTCMD_FILE	"lastcmd.txt"
#define TEMP_FILE       "dawntmp.txt"
#define SCRIPTS_FILE	SCRIPTS_DIR"scripts.txt"// scripts database

/*
* Game parameters.
* Increase the max'es if you add more of something.
* Adjust the pulse numbers to suit yourself.
*/
#define MAX_ALIAS					(50)
#define MAX_SOCIALS					(400)
#define MAX_SKILL					(700)
#define MAX_RESERVED_SPELL_SPACE	(15)	// this is how many new spells can be added 
											// dynamically between hotreboots.
#define MAX_GROUP_TABLE				(24)
#define MAX_SKILLGROUP				(50)

#define MAX_IN_GROUP				(20)
#define MAX_INPUTTAIL				(20)
#define MAX_CLASS					(35)

#define MAX_CONDENSE_LENGTH			(110)
#define MAX_PC_RACE					(120)	// MAX_PC_RACE should NEVER be > MAX_RACE
#define MAX_RACE					(175) // NOTE - the tracking system assumes that there will be at most 2^15 races.
#define MAX_RACIAL_SKILLS			(10)
#define MAX_DIR						(10)
#define MAX_RANK				(7)	// max ranks in a clan
#define MAX_TRACKS_PER_ROOM		(20)
#define MAX_TRACKABLE_CHARACTERS_IN_GAME (20000) // this includes mobs
#define MAX_DEITY				(50)
#define MAX_BUILD_RESTRICTS		(9)
#define MAX_REPLAYTELL			(30)
#define MAX_REPLAYROOM			(40)
#define MAX_REPLAYCHANNELS		(40)
#define MAX_CHECK_IMMTALK		(30)	// number of immtalks to remember
#define MAX_MOB_WEALTH_MULTIPLIER (20)

#define MAX_HELP_CATEGORIES		(150)

// max nesting of objects within objects for loading/saving
#define MAX_NEST        (100)
#define MAX_MPTRACE	500 // Number of mudprogs to trace the calls of

// LEVEL RELATED PARAMETERS
#define MAX_LEVEL              (100)

//#define FOUNDER_LEVEL_ENABLED 

#define IMPLEMENTOR         MAX_LEVEL
#define ADMIN               (MAX_LEVEL - 2) // admin group for notes and stuff
#define COUNCIL				(MAX_LEVEL - 4) 
#define CREATOR             (MAX_LEVEL - 1)
#define SUPREME             (MAX_LEVEL - 2)
#define DEITY               (MAX_LEVEL - 3)
#define GUARDIAN			(MAX_LEVEL - 4)
#define IMMORTAL            (MAX_LEVEL - 5)
#define DEMI                (MAX_LEVEL - 6)
#define ANGEL               (MAX_LEVEL - 7)
#define AVATAR              (MAX_LEVEL - 8)
#define LEVEL_IMMORTAL		(MAX_LEVEL - 8)
#define LEVEL_HERO			(MAX_LEVEL - 9)
#define HERO                LEVEL_HERO

#ifdef FOUNDER_LEVEL_ENABLED
#define ABSOLUTE_MAX_LEVEL (MAX_LEVEL + 1)
#else
#define ABSOLUTE_MAX_LEVEL (MAX_LEVEL)
#endif

// mob_prog.c - program_flow()
#define MAX_CALL_LEVEL    (5) // maximum nested calls
#define MAX_QUEST_STATE	  (5) // maximum number of quest states

#define	MAX_HELP_HISTORY	(10) // max number of times you can click on PREV for helps
// String and memory management parameters.
#define MAX_KEY_HASH				(3079)
#define MSL							(8192)	// MAX_STRING_LENGTH
#define MIL							(1024)	// MAX_INPUT_LENGTH
#define PAGELEN						  (22)	// default page length 
#define HSL						  (MSL*17)	// HUGE STRING LENGTH

#define MAX_HELP_SIZE		(10000)

// note posting restriction related code
#define MAX_NOTE_POST_TIME_INDEX			(5)
#define NOTE_POST_RESTRICTIONS_BELOW_LEVEL (20)

#define PULSE_PER_SECOND			4
#define PULSE_AFFECTS			(  1 * PULSE_PER_SECOND)
#define PULSE_VIOLENCE			(  3 * PULSE_PER_SECOND)
#define PULSE_MOBILE			(  4 * PULSE_PER_SECOND)
#define PULSE_MINUTE			(  6 * PULSE_PER_SECOND)
#define PULSE_DAWNSTAT			( 10 * PULSE_PER_SECOND)
#define PULSE_TICK				( 60 * PULSE_PER_SECOND)
#define PULSE_AREA				(120 * PULSE_PER_SECOND)
#define PULSE_AUCTION			( 15 * PULSE_PER_SECOND)
#define PULSE_MOOT				( 60 * PULSE_PER_SECOND)

// Stat stuff
#define MAX_STATS					10
#define STAT_ST						0
#define STAT_QU						1
#define STAT_PR						2
#define STAT_EM						3
#define STAT_IN						4
#define STAT_CO						5
#define STAT_AG						6
#define STAT_SD						7
#define STAT_ME						8
#define STAT_RE						9


#define RPS_AUDIT_SIZE (30) // stop spamming

// shops - max number of different item types they can trade
#define MAX_TRADE		(5) 
#define MAX_INN			(5) 


#define NO_FLAG -99     // Must not be used in flags or stats.

// alarm handler system related settings
#define BOOT_DB_ABORT_THRESHOLD		(game_settings->alarm_boot_db_abort_threshold)
#define RUNNING_ABORT_THRESHOLD		(game_settings->alarm_running_abort_threshold)
#define RUNNING_DNS_ABORT_THRESHOLD	(game_settings->alarm_running_dns_abort_threshold)
#define ALARM_FREQUENCY				(game_settings->alarm_frequency)

// Well known room virtual numbers - all gamesettings based now
// Kal - April 00
#define ROOM_VNUM_NEWBIE_RECALL	(game_settings->roomvnum_newbie_recall)	// 7359
#define ROOM_VNUM_STARTTELNET	(game_settings->roomvnum_starttelnet)	// 3000
#define ROOM_VNUM_STARTIRC		(game_settings->roomvnum_startirc)		// 4200
#define ROOM_VNUM_LIMBO         (game_settings->roomvnum_limbo		)	// 2
#define ROOM_VNUM_COURT_RECALL	(game_settings->roomvnum_court_recall)	// 6000
#define ROOM_VNUM_OOC			(game_settings->roomvnum_ooc		)	// 30000
#define ROOM_VNUM_GOOD_RECALL	(game_settings->roomvnum_good_recall)	// 3000
#define ROOM_VNUM_GOOD_BANK		(game_settings->roomvnum_good_bank	)	// 3029
#define ROOM_VNUM_EVIL_RECALL	(game_settings->roomvnum_evil_recall)	// 27003
#define ROOM_VNUM_EVIL_BANK		(game_settings->roomvnum_evil_bank	)	// 27024
#define ROOM_PKILLPORT_DEATH_ROOM (game_settings->roomvnum_pkport_death_room)//16 
#define ROOM_VNUM_JAIL			(game_settings->roomvnum_jail) //299 
#define ROOM_VNUM_WEAPON_DONATE			(game_settings->roomvnum_weapon_donate) //GPM
#define ROOM_VNUM_ARMOR_DONATE			(game_settings->roomvnum_armor_donate) //GPM
#define ROOM_VNUM_MISC_DONATE			(game_settings->roomvnum_misc_donate)  //GPM
#define ROOM_VNUM_NEWBIEWEAPON_DONATE	(game_settings->roomvnum_newbieweapon_donate) //GPM
#define ROOM_VNUM_NEWBIEARMOR_DONATE	(game_settings->roomvnum_newbiearmor_donate) //GPM
#define ROOM_VNUM_NEWBIEMISC_DONATE		(game_settings->roomvnum_newbiemisc_donate) //GPM


// Well known object virtual numbers.
// money objects in limbo.are 
#define OBJ_VNUM_SILVER_ONE 		1
#define OBJ_VNUM_GOLD_ONE			2
#define OBJ_VNUM_GOLD_SOME			3
#define OBJ_VNUM_SILVER_SOME		4
#define OBJ_VNUM_COINS				5

#define OBJ_VNUM_CRYSTAL_FLASK		7
#define OBJ_VNUM_BADCRYSTAL_FLASK	8
#define OBJ_VNUM_DUMMY				9 // Used in save.cpp to load objects that don't exist.
// body part objects in limbo.are 
#define OBJ_VNUM_CORPSE_NPC 		10
#define OBJ_VNUM_CORPSE_PC			11
#define OBJ_VNUM_SEVERED_HEAD		12
#define OBJ_VNUM_TORN_HEART 		13
#define OBJ_VNUM_SLICED_ARM 		14
#define OBJ_VNUM_SLICED_LEG 		15
#define OBJ_VNUM_GUTS				16
#define OBJ_VNUM_BRAINS 			17
#define OBJ_VNUM_ASHES				18			// Used for fire decay remnant
#define OBJ_VNUM_FIRE				19			// Used for build <fire>

// magic objects in limbo.are 
#define OBJ_VNUM_MUSHROOM			20
#define OBJ_VNUM_LIGHT_BALL 		21
#define OBJ_VNUM_SPRING 			22
#define OBJ_VNUM_DISC				23
#define OBJ_VNUM_SLICE				24
#define OBJ_VNUM_PORTAL 			25
#define OBJ_VNUM_ORE				44			// The template for ores

// magic objects in magics.are 
#define OBJ_VNUM_WORLD_MAP		(game_settings->obj_vnum_world_map)
#define OBJ_VNUM_GOOD_CITY_MAP	(game_settings->obj_vnum_good_city_map)
#define OBJ_VNUM_EVIL_CITY_MAP	(game_settings->obj_vnum_evil_city_map)
#define OBJ_VNUM_DIVINE_LIGHT	(game_settings->obj_vnum_divine_light)
#define OBJ_VNUM_ROSE			(game_settings->obj_vnum_rose)
#define OBJ_VNUM_RAFT			(game_settings->obj_vnum_raft)
#define OBJ_VNUM_NEWBIE_GUIDE	(game_settings->obj_vnum_newbie_guide)
#define OBJ_VNUM_RP_ITEM		(game_settings->obj_vnum_rp_item)
#define OBJ_VNUM_SPIRIT_HAMMER	(game_settings->obj_vnum_spirit_hammer)
#define OBJ_VNUM_STAFF			(game_settings->obj_vnum_staff)
#define OBJ_VNUM_DRUIDSTAFF		(game_settings->obj_vnum_druidstaff)
#define OBJ_VNUM_TOTEMSTAFF		(game_settings->obj_vnum_totemstaff)
#define OBJ_VNUM_PIT			(game_settings->obj_vnum_pit)
#define OBJ_VNUM_SUMMON_JUSTICE (game_settings->obj_vnum_summon_justice)

#define OBJ_VNUM_OUTFIT_MACE	(game_settings->obj_vnum_outfit_mace)
#define OBJ_VNUM_OUTFIT_DAGGER	(game_settings->obj_vnum_outfit_dagger)
#define OBJ_VNUM_OUTFIT_SWORD	(game_settings->obj_vnum_outfit_sword)
#define OBJ_VNUM_OUTFIT_STAFF	(game_settings->obj_vnum_outfit_staff)
#define OBJ_VNUM_OUTFIT_AXE		(game_settings->obj_vnum_outfit_axe)
#define OBJ_VNUM_OUTFIT_FLAIL	(game_settings->obj_vnum_outfit_flail)
#define OBJ_VNUM_OUTFIT_WHIP	(game_settings->obj_vnum_outfit_whip)
#define OBJ_VNUM_OUTFIT_POLEARM	(game_settings->obj_vnum_outfit_polearm)
#define OBJ_VNUM_OUTFIT_SICKLE	(game_settings->obj_vnum_outfit_sickle)
#define OBJ_VNUM_OUTFIT_VEST	(game_settings->obj_vnum_outfit_vest)
#define OBJ_VNUM_OUTFIT_SHIELD	(game_settings->obj_vnum_outfit_shield)
#define OBJ_VNUM_OUTFIT_LIGHT	(game_settings->obj_vnum_outfit_light)
#define OBJ_VNUM_OUTFIT_SLEEVES	(game_settings->obj_vnum_outfit_sleeves)
#define OBJ_VNUM_OUTFIT_CAP		(game_settings->obj_vnum_outfit_cap)
#define OBJ_VNUM_OUTFIT_GLOVES	(game_settings->obj_vnum_outfit_gloves)
#define OBJ_VNUM_OUTFIT_LEGGINGS (game_settings->obj_vnum_outfit_leggings)
#define OBJ_VNUM_OUTFIT_BOOTS	(game_settings->obj_vnum_outfit_boots)
#define OBJ_VNUM_OUTFIT_BELT	(game_settings->obj_vnum_outfit_belt)

#define MOB_VNUM_TO_RUN_OBJECT_MUDPROGS (game_settings->mob_vnum_to_run_object_mudprogs)

// Well known mob virtual numbers.
// Defined in #MOBILES.
#define MOB_VNUM_SUMMON_GUARDIAN	(game_settings->mob_vnum_summon_guardian) 
#define MOB_VNUM_VYR_GOOD			(game_settings->mob_vnum_vyr_good)
#define MOB_VNUM_VYR_BAD			(game_settings->mob_vnum_vyr_bad)

// Totemic mob templates (shorts/longs/extended descs, etc)
#define MOB_VNUM_TOTEM_BEAR			60
#define MOB_VNUM_TOTEM_STAG			61
#define MOB_VNUM_TOTEM_SWALLOW		62
#define MOB_VNUM_TOTEM_OSPREY		63
#define MOB_VNUM_TOTEM_TORTOISE		64
#define MOB_VNUM_TOTEM_RAVEN		65
#define MOB_VNUM_TOTEM_BAT			66
#define MOB_VNUM_TOTEM_WOLF			67
#define MOB_VNUM_TOTEM_SERPENT		68
#define MOB_VNUM_TOTEM_TOAD			69
#define MOB_VNUM_TOTEM_OWL			70
#define MOB_VNUM_TOTEM_HARE			71

#define CD      char_data
#define MID     MOB_INDEX_DATA
#define OD      OBJ_DATA
#define OID     OBJ_INDEX_DATA
#define RID     ROOM_INDEX_DATA
#define SF      SPEC_FUN
#define OSF		OSPEC_FUN
#define AD      AFFECT_DATA
#define MPC     MUDPROG_CODE

// Connected states for OLC editor.
#define ED_AREA			1
#define ED_ROOM			2
#define ED_OBJECT		3
#define ED_MOBILE		4
#define ED_MPCODE		5
#define ED_HELP			6
#define ED_RACE			7
#define ED_CORECLASS	9
#define ED_CLASS		10
#define ED_SPELLSKILL	11
#define ED_COMMAND		12
#define ED_DEITY		13
#define ED_BAN			15
#define ED_QUEST		16
#define ED_GAME			17
#define ED_SOCIAL		18
#define ED_HERB			19
#define ED_MIX			20
#define ED_CLAN			21
#define ED_SKILLGROUP	22
#define ED_LANGUAGE		23
// up to 34 is reserved for dot codebase development

#define SUBLEVELS_PER_PRAC		(2)
#define SUBLEVELS_PER_TRAIN		(5)

////////// MF ONLY PARAMETERS/VARIABLES



#endif // PARAMS_H


