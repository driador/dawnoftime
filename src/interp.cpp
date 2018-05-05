/**************************************************************************/
// interp.cpp - main game interpreter and related code
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
#include "interp.h"
#include "security.h"
#include "clan.h"

bool check_disabled (char_data *ch, const struct cmd_type *command);

#define END_MARKER    "END" // for load_disabled() and save_disabled()

bool check_social(char_data *ch, char *command, char *argument, bool global);

/*
 * Command logging types.
 */
#define LOG_ALWAYS     0  /* always logged into the system log */

#define LOG_PALWAYS    1	// always log players only into the system log 
							// don't log NPC's use of the command

#define LOG_OLC		   3  /* ALWAYS Logged into OLC log files */

#define LOG_NORMAL     5  /*   Logged into log files (when LOG ALL is on   */
                          /* or a player log is on... if plog on, also     */
                          /* logged into their own player log.             */


#define LOG_PLOGONLY   7  /*   only logged into players log file when      */
                          /* player is being logged, not logged into       */
                          /* main system.                                  */

#define LOG_DONT_LOG   8  /* not logged but can be snooped - eg directions */

#define LOG_NEVER      10  /* Never logged - only thing not snoopable       */


bool fLogAll		= false; // Log-all switch.
char last_command[MSL]; // Global variable to hold the last command
char last_input[MSL];    // Global variable to hold the last input 
int com_category_lookup(char *name);


/**************************************************************************/
// Command table.
struct	cmd_type	cmd_table	[] =

{
	 /*
	  * Common movement commands.
	  */
	{ "north",			do_north,			POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "east",			do_east,			POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "south",			do_south,			POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "west",			do_west,			POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "up", 			do_up,				POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "ne", 			do_northeast,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "northeast",		do_northeast,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "se", 			do_southeast,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "southeast",		do_southeast,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "sw", 			do_southwest,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "southwest",		do_southwest,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "nw", 			do_northwest,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "northwest",		do_northwest,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "down",			do_down,			POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	
	// for speedwalking abbreviations	
	{ "t",				do_northeast,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "g",				do_southeast,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "f",				do_southwest,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	{ "r",				do_northwest,		POS_STANDING,	0,	LOG_DONT_LOG,	0 },
	
	
	 /* commands put here to make them run first
	* - none of these show on help menus -
	*	
	* Placed here so one and two letter abbreviations work.
	*/	
	{ "wiz",			do_wizhelp, 		POS_DEAD,	   IM,	LOG_NORMAL, 	0 },
	{ "wizgrantlist",	do_wizgrantlist, 	POS_DEAD,	 ADMIN,	LOG_NORMAL, 	0 },
	{ "got",			do_goto,			POS_DEAD,	   L8,	LOG_NORMAL, 	0 },
	{ "at", 			do_at,				POS_DEAD,	   L7,	LOG_NORMAL, 	1 },
	{ "bu", 			do_buy, 			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "ba", 			do_backstab,		POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	{ "ban",			do_ban, 			POS_DEAD, ADMIN-1,	LOG_ALWAYS, 	1 },
	{ ":",				do_immtalk, 		POS_DEAD,	   IM,	LOG_NORMAL, 	0 },
	{ "affec",			do_affects, 		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "autolis",		do_autolist,		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "bec",			do_bec, 			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "c",				do_cast_redirect,	POS_FIGHTING,	0,	LOG_NORMAL, 	0 },	
	{ "chat",			do_chat, 			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "cas",			do_cast,			POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	{ "com",			do_commune, 		POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	{ "class",			do_class,			POS_DEAD, COUNCIL,	LOG_ALWAYS, 	0 },
	{ "sum",			do_summon,			POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	{ "charinf",		do_charinfo,		POS_DEAD,		IM,	LOG_ALWAYS,		0 },	
	{ "channe", 		do_channel, 		POS_STANDING,	0,	LOG_NORMAL, 	0 },
	{ "command",		do_commands,		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "compar", 		do_compare, 		POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "enveno", 		do_envenom, 		POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "exit",			do_exits,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "ge", 			do_get, 			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "grou",			do_group,			POS_SLEEPING,	0,	LOG_NORMAL, 	0 },
	{ "inventor",		do_inventory,		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "iwher",			do_iwhere,			POS_DEAD,	   IM,	LOG_NORMAL, 	0 },
	{ "loo",			do_look,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "lis",			do_list,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "kil",			do_kill,			POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	{ "mwher",			do_mwhere,			POS_DEAD,	   IM,	LOG_NORMAL, 	0 },
	{ "mot",			do_motd,			POS_DEAD,		0,	LOG_NORMAL, 	0 },	
	{ "mp",				do_mp, 				POS_DEAD,		0,	LOG_DONT_LOG,	0 },
	{ "mob",			do_mp, 				POS_DEAD,		0,	LOG_DONT_LOG,	0 }, // for compatiblity
	{ "mq", 			do_mudprogqueue,	POS_DEAD,		0,	LOG_DONT_LOG,	0 },
	{ "orde",			do_order,			POS_RESTING,	0,	LOG_PALWAYS,	0 },
	{ "pinf",			do_pinfo,			POS_DEAD,		L7, LOG_NORMAL, 	0 },
	{ "practic",		do_practice,		POS_SLEEPING,	0,	LOG_NORMAL, 	0 },
	{ "prefi",			do_prefi,			POS_DEAD,		IM, LOG_NORMAL, 	0 },
	{ "res",			do_rest,			POS_SLEEPING,	0,	LOG_NORMAL, 	0 },
	{ "recit",			do_recite,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "retel",			do_retell,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "repl",			do_reply,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "sa", 			do_say, 			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "soc",			do_globalsocial,	POS_SLEEPING,	DIS,LOG_NORMAL, 	0 },  // gs is already enabled
	{ "socket", 		do_sockets, 		POS_DEAD,		L4, LOG_NORMAL, 	0 },
	{ "secon",			do_second,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "sl", 			do_sleep,			POS_SLEEPING,	0,	LOG_NORMAL, 	0 },
	{ "stan",			do_stand,			POS_SLEEPING,	0,	LOG_NORMAL, 	0 },
	{ "scor",			do_score,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "tel",			do_tell,			POS_SLEEPING,	0,	LOG_NORMAL, 	0 },
	{ "unloc",			do_unlock,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "wh", 			do_who, 			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "wiel",			do_wear,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "seevnu", 		do_seevnum, 		POS_DEAD,		IM, LOG_NORMAL, 	0 }, 
	{ "wor",			do_worth,			POS_SLEEPING,	0,	LOG_NORMAL,		0 },
	
	
	/* all commands - sorted alphabetically */	
		
	{ "'",				do_say, 			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "\"", 			do_say, 			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "\\",				do_speedwalk,		POS_STANDING,	0,	LOG_NORMAL,		0 },
	{ ",",				do_emote,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ ".",				do_ooc, 			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ ">",				do_retell,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "[",				do_admintalk,		POS_DEAD,	 ADMIN, LOG_NORMAL, 	0 },
	{ "]",				do_hightalk,		POS_DEAD,	  ML-1, LOG_NORMAL, 	0 },
	{ "=",				do_codetalk,		POS_DEAD,		IM, LOG_NORMAL, 	0 },
	{ "-",				do_mythostalk,		POS_DEAD,		IM, LOG_NORMAL, 	0 },
	{ "/",				do_recall,			POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	// for IRC users - if we have any
	{ "/me",			do_emote,			POS_DEAD,		0,	LOG_ALWAYS, 	0 },
	{ "add",			do_add, 			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "affects",		do_affects, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "afk",			do_afk, 			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "alia",			do_alia,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "alias",			do_alias,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "alignstat",		do_alignstats,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "amote",			do_amote,			POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "answer", 		do_answer,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "ansi",			do_colour,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "apply",			do_apply,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "append", 		do_append,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "apurge",			do_apurge,			POS_DEAD,		L4, LOG_NORMAL,		0 },
	{ "areas",			do_areas,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "areasalpha",		do_areasalpha,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "askname",		do_askname, 		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "attune", 		do_attune,			POS_STANDING,	0,  LOG_NORMAL, 	1 },
	{ "autoanswer", 	do_autoanswer,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "autoassist", 	do_autoassist,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autodamage",		do_autodamage, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autoexit",		do_autoexit,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autoexamine",	do_autoexamine, 	POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autogold",		do_autogold,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autokeepalive",	do_autokeepalive,	POS_DEAD,		0,	LOG_NORMAL, 	1 },	
	{ "autolandonrest", do_autolandonrest,	POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autolist",		do_autolist,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autoloot",		do_autoloot,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autopeek",		do_autopeek,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autopkassist",	do_autopkassist,	POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autorecall", 	do_autorecall,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autoreformat",	do_autoreformat,	POS_DEAD,	   IM,	LOG_NORMAL, 	1 },
	{ "autoself",		do_autoself,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autosubdue", 	do_autosubdue,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autosplit",		do_autosplit,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autotrack",		do_autotrack,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "autowhoinvislogin",do_autowhoinvislogin,	POS_DEAD,  IM,	LOG_NORMAL, 	1 },
	{ "autowizilogin",	do_autowizilogin,	POS_DEAD,	   IM,	LOG_NORMAL, 	1 },
	{ "autowraptells",	do_autowraptells,	POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "awareness",		do_awareness,		POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "backstab",		do_backstab,		POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "bank",			do_bank,			POS_STANDING,	0,	LOG_ALWAYS, 	1 },
	{ "bash",			do_bash,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "battlelag",		do_battlelag,		POS_DEAD,		0,	LOG_NORMAL, 	1 },	
	{ "berserk",		do_berserk, 		POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "becomeactive",	do_becomeactive,	POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "brandish",		do_brandish,		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "brief",			do_brief,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "bs", 			do_backstab,		POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	{ "bug",			do_bug, 			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "buy",			do_buy, 			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "brew",			do_brew,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "build",			do_build,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "bury",			do_bury,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "bypassduel", 	do_bypassduel,		POS_RESTING,	0,	LOG_ALWAYS, 	1 },
	{ "cannibalize",	do_cannibalize, 	POS_STANDING,	IM, LOG_NORMAL, 	1 },
	{ "cast",			do_cast,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "changes",		do_changes, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "channels",		do_channels,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "circle",			do_circle,			POS_FIGHTING,	0,	LOG_NORMAL,		1 },
	{ "close",			do_close,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "claw",			do_claw,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "classstats", 	do_classstats,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "classinfo",		do_classinfo, 		POS_DEAD,		0,	LOG_NORMAL,		1 },
	{ "clantalk",		do_clantalk,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "cranks", 		do_clanranks,		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "clanranks",		do_clanranks,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "color",			do_colour,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "colour", 		do_colour,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "collect",		do_collect, 		POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "cook",			do_cook,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "combine",		do_combine, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "commands",		do_commands,		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "commune",		do_commune, 		POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "compact",		do_compact, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "compare",		do_compare, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "consider",		do_consider,		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "continue",		do_continue,		POS_RESTING,	0,	LOG_NORMAL, 	0 },
	{ "count",			do_count,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "cskinfo",		do_cskinfo, 		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "cspinfo",		do_cspinfo, 		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "credits",		do_credits, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "ctalk",			do_clantalk,		POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "ctime",			do_compile_time,	POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "dawnftp", 		do_dawnftp,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "delet",			do_delet,			POS_DEAD,		0,	LOG_ALWAYS, 	0 },
	{ "delete", 		do_delete,			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "demote", 		do_demote,			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "description",	do_description, 	POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "diagnose",		do_diagnose,		POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "dig",			do_dig, 			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "dirt",			do_dirt,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "disarm", 		do_disarm,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "dismount",		do_dismount,		POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "donate", 		do_donate,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "drink",			do_drink,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "drag",			do_drag,			POS_STANDING,	IM,	LOG_ALWAYS,		1 },
	{ "draw",			do_draw,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "drop",			do_drop,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "duel",			do_duel,			POS_STANDING,	0,	LOG_ALWAYS, 	1 }, 
	{ "acceptduel", 	do_acceptduel,		POS_STANDING,	0,	LOG_ALWAYS, 	1 }, 
	{ "declineduel",	do_declineduel, 	POS_RESTING,	0,	LOG_ALWAYS, 	1 },
	{ "declineooc", 	do_declineooc,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "eat",			do_eat, 			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "emote",			do_emote,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "empty",			do_empty,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "enter",			do_enter,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "envenom",		do_envenom, 		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "equipment",		do_equipment,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "examine",		do_examine, 		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "exits",			do_exits,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "fill",			do_fill,			POS_RESTING,	0,	LOG_NORMAL, 	1 },


	{ "fullexits",		do_fullexits,		POS_DEAD,		0,	LOG_NORMAL, 	1 },	
	{ "forget",			do_forget,			POS_RESTING,	0,	LOG_NORMAL,		1 },
	{ "fork",			do_fork,			POS_STANDING,	IM, LOG_NORMAL, 	1 },
	{ "flee",			do_flee,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "flame",			do_flame,			POS_DEAD,		IM,	LOG_NORMAL, 	1 },
	{ "flourish",		do_flourish,		POS_STANDING,	IM, LOG_NORMAL, 	1 },
	{ "flip",			do_flip,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "fly",			do_fly, 			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "follow", 		do_follow,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "for",			do_force,			POS_DEAD,		L7, LOG_ALWAYS, 	0 },
	{ "forage", 		do_forage,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "freevnum",		do_freevnum,		POS_DEAD,		0,	LOG_ALWAYS,		1 },
	{ "gain",			do_gain,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "gamble", 		do_gamble,			POS_SITTING,	0,	LOG_ALWAYS, 	1 },
	{ "gamesettings",	do_gamesettings,	POS_DEAD,		0,	LOG_NORMAL, 	1 }, 
	{ "get",			do_get, 			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "genname",		do_genname, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "give",			do_give,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "glance", 		do_glance,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "gs",				do_globalsocial,	POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "go", 			do_enter,			POS_STANDING,	0,	LOG_NORMAL, 	0 },
	{ "goooc",			do_goooc,			POS_STANDING,	0,	LOG_ALWAYS, 	1 },
	{ "gore",			do_gore,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "gouge",			do_gouge,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "gprompt", 		do_gprompt,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "grab",			do_grab,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "grats",			do_grats,			POS_DEAD,		IM,	LOG_NORMAL, 	1 },
	{ "group",			do_group,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "groups", 		do_skillgroups,		POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "heal",			do_heal,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "help",			do_help,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "helpcat",		do_helpcat,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "helplist",		do_helplist,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "helpprev",		do_helpprev,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "helpnext",		do_helpnext,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
//	{ "helphistory",	do_helphistory,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "herbalism",		do_herbalism,		POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "herblore",		do_herblore,		POS_SITTING,	IM, LOG_ALWAYS,		1 },
	{ "hide",			do_hide,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "history",		do_history,			POS_DEAD,		1,	LOG_NORMAL, 	1 },
	{ "hit",			do_kill,			POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	{ "hlook",			do_hlook,			POS_DEAD,		1,	LOG_NORMAL,		1 },
	{ "hold",			do_hold,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "ic", 			do_ic,				POS_DEAD,		IM,	LOG_NORMAL, 	1 },
	{ "ictime", 		do_time,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "ideas",			do_idea,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "info",			do_skillgroups,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "introduce",		do_introduce,		POS_RESTING,	0,	LOG_NORMAL,		1 },
	{ "inventory",		do_inventory,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "invitelist",		do_invitelist,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "ispell", 		do_ispell,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "kick",			do_kick,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "kill",			do_kill,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "kneel",			do_kneel,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "knock",			do_knock,			POS_SITTING,	0,	LOG_NORMAL, 	1 },
	{ "lay",			do_lay_on_hands,	POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "lag",			do_lag, 			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "language",		do_language,		POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "land",			do_land,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "laston", 		do_laston,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "lastonreload",	do_lastonreload,	POS_DEAD,		ML, LOG_ALWAYS, 	1 },
	{ "laston_update_from_disk", do_laston_update_from_disk, POS_DEAD,		ML, LOG_ALWAYS, 	1 },
	{ "letter", 		do_letter,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "lines",			do_scroll,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "list",			do_list,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "loc",			do_lockers,			POS_RESTING,	0,	LOG_ALWAYS, 	0 },
	{ "lock",			do_lock,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "lockers",		do_lockers,			POS_RESTING,	0,	LOG_ALWAYS, 	1 },
	{ "look",			do_look,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "lore",			do_lore,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "losepet",		do_losepet, 		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "mccp",			do_mccp,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "metric", 		do_metric,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "mine",			do_mine,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "misc",			do_misc,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "mixflask",		do_mixflask,		POS_RESTING,	0, LOG_ALWAYS,		1 }, // mix herbs in a flask
	{ "mode",			do_mode,			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "moon",			do_weather, 		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "moot",			do_moot,			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "motd",			do_motd,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "msp",			do_msp, 			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "mudstats",		do_mudstats,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "mudftp", 		do_dawnftp,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "mp_trigger_in_room", do_mp_trigger_in_room,POS_DEAD,IM,LOG_NORMAL, 	1 },	
	{ "mxp",			do_mxp, 			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "mxpreflect",		do_mxpreflect, 		POS_DEAD,		0,	LOG_ALWAYS, 	1 },	
	{ "name_b4short",	do_name_b4short,	POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "name_only_4known",do_name_only_4known,POS_DEAD,		0,	LOG_ALWAYS, 	1 },	
	{ "needlepoint",	do_needlepoint,		POS_STANDING,	0,	LOG_ALWAYS, 	1 },
	{ "news",			do_news,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "newbietalk", 	do_newbietalk,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "nofollow",		do_nofollow,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "nomisc", 		do_nomisc,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "nosummon",		do_nosummon,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "nocharm",		do_nocharm, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "nonames",		do_nonames, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "noheromsg",		do_noheromsg,		POS_DEAD,	 HE-1,	LOG_NORMAL, 	1 },
	{ "notes",			do_note,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "noteach",		do_noteach, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "nspeak", 		do_nspeak,			POS_SLEEPING,	0,	LOG_ALWAYS, 	1 },
	{ "ntalk",			do_ntalk,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "ntell",			do_ntell,			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "nreply", 		do_nreply,			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "ooc",			do_ooc, 			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "open",			do_open,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "order",			do_order,			POS_RESTING,	0,	LOG_PALWAYS,	1 },
	{ "outcast",		do_outcast, 		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "outfit", 		do_outfit,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "ownerlist",		do_ownerlist,		POS_DEAD,	   IM,	LOG_NORMAL, 	1 },
	{ "panic",			do_panic,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "password",		do_password,		POS_DEAD,		0,	LOG_NEVER,		1 },
	{ "peek",			do_peek,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "pick",			do_pick,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "place",			do_place,			POS_STANDING,	0,	LOG_ALWAYS, 	1 },
	{ "pkinfo", 		do_pkinfo,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "pmote",			do_pmote,			POS_RESTING,	0,	LOG_PALWAYS,	0 },
	{ "pour",			do_pour,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "pose",			do_pose, 			POS_RESTING,  DIS,	LOG_NORMAL,		1 }, // needs to be enabled if used
	{ "pounce", 		do_pounce,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "practice",		do_practice,		POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "pray",			do_pray,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "promote",		do_promote, 		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "prompt", 		do_prompt,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "push",			do_push,			POS_STANDING,	IM,	LOG_ALWAYS,		1 },
	{ "put",			do_put, 			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "quaff",			do_quaff,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "question",		do_question,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "quester",		do_quester, 		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "qpoints",		do_qpoints,			POS_DEAD,		IM,	LOG_ALWAYS, 	1 },
	{ "qui",			do_qui, 			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "quiet",			do_quiet,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "quit",			do_quit,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "racestats",		do_racestats,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "raceinfo",		do_raceinfo, 		POS_DEAD,		0,	LOG_NORMAL,		1 },	
	{ "ride",			do_ride,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "read",			do_read,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "recall", 		do_recall,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "recite", 		do_recite,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "remove", 		do_remove,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "rent",			do_rent,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "replay", 		do_replay,			POS_DEAD,		0,	LOG_NORMAL, 	1 },	
	{ "replaychannels", do_replaychannels,	POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "replaychar",		do_replaychar,		POS_DEAD,		ML,	LOG_NORMAL, 	1 },
	{ "replayroom", 	do_replayroom,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "reply",			do_reply,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "report", 		do_report,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "requestooc", 	do_requestooc,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "requestletgain", do_requestletgain,	POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "rpsheet",		do_rpsheet, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "rptell", 		do_rptell,			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "rpreply",		do_rpreply, 		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "cletgain",		do_cancelletgain,	POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "rletgain",		do_requestletgain,	POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "cancelletgain",	do_cancelletgain,	POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "rescue", 		do_rescue,			POS_FIGHTING,	0,	LOG_NORMAL, 	0 },
	{ "rest",			do_rest,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "retell", 		do_retell,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "retreat",		do_retreat, 		POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "rules",			do_rules,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "save",			do_save,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "rsay",			do_rsay,			POS_RESTING,	0,	LOG_NORMAL, 	0 }, // remembered say for mudprogs
	{ "say",			do_say, 			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "saymote",		do_saymote, 		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "sayto",			do_sayto,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "scan",			do_sscan,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "score",			do_score,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "scroll", 		do_scroll,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "scribe", 		do_scribe,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "search", 		do_search,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "set",			do_set, 			POS_DEAD,		L3,	LOG_ALWAYS,		1 },
	{ "setage", 		do_setage,			POS_RESTING,	0,	LOG_ALWAYS, 	1 },
	{ "setdeity",		do_setdeity,		POS_DEAD,		L2, LOG_NORMAL, 	1 },
	{ "setichour",		do_setichour,		POS_DEAD,		ML, LOG_NORMAL, 	1 },
	{ "setrooms",		do_setrooms,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },	
	{ "second", 		do_second,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "seek",			do_seek,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "sell",			do_sell,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "shadow", 		do_shadow,			POS_STANDING,	IM,  LOG_NORMAL,	1 },
	{ "sharpen",		do_sharpen, 		POS_RESTING,	IM,  LOG_ALWAYS,	1 },
	{ "show",			do_show,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "showaffects",	do_showaffects,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "showmisc",		do_showmisc,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "sheathe",		do_sheathe, 		POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "sit",			do_sit, 			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "skills", 		do_skills,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "skillgroups",	do_skillgroups,		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "slice",			do_slice,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "sleep",			do_sleep,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "smote",			do_smote,			POS_DEAD,		0,	LOG_NORMAL, 	0 },
	{ "sneak",			do_sneak,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "social",			do_globalsocial,	POS_SLEEPING,	DIS,LOG_NORMAL, 	1 }, // gs is already enabled
	{ "socials",		do_socials, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "socshow",		do_socshow, 		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "spells", 		do_spells,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "spinfo", 		do_spinfo,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "sinfo",			do_sinfo,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "sing",			do_sing,			POS_DEAD,		IM, LOG_NORMAL, 	1 }, // Beginning of bards
	{ "speedwalk",		do_speedwalk,		POS_STANDING,	0,	LOG_ALWAYS, 	1 },
	{ "split",			do_split,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "stand",			do_stand,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "steal",			do_steal,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "story",			do_story,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "study",			do_study,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "surrender",		do_surrender,		POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "subdue", 		do_subdue,			POS_RESTING,	0,	LOG_ALWAYS, 	1 },
	{ "summon", 		do_summon,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "take",			do_get, 			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "tame",			do_tame,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "teach",			do_teach,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "tell",			do_tell,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "tether", 		do_tether,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "throw",			do_throw,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "date",			do_time,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "time",			do_time,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "title",			do_title,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "train",			do_train,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "tracks", 		do_tracks,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "trip",			do_trip,			POS_FIGHTING,	0,	LOG_NORMAL, 	1 },
	{ "typo",			do_typo,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "unalias",		do_unalias, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "unlock", 		do_unlock,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "unread", 		do_unread,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "untether",		do_untether,		POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "uptime", 		do_uptime,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "value",			do_value,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "vanish", 		do_vanish,			POS_STANDING,	0,	LOG_NORMAL, 	1 },
	{ "visible",		do_visible, 		POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "unhide",			do_visible, 		POS_SLEEPING,	0,	LOG_NORMAL, 	0 }, // visible alias
	{ "vote",			do_vote,			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "wake",			do_wake,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	{ "wear",			do_wear,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "weather",		do_weather, 		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "whisper",		do_whisper, 		POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "who",			do_who, 			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "whois",			do_whois,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "whoformat",		do_whoformat,		POS_DEAD,		IM,	LOG_NORMAL, 	1 },
	{ "wield",			do_wear,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "wimpy",			do_wimpy,			POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "wizlist",		do_wizlist, 		POS_DEAD,		0,	LOG_NORMAL, 	1 },
	{ "worship",		do_worship, 		POS_STANDING,	IM, LOG_NORMAL, 	1 },
	{ "worth",			do_worth,			POS_SLEEPING,	0,	LOG_NORMAL, 	1 },
	// worth aliases
	{ "money",			do_worth,			POS_SLEEPING,	0,	LOG_NORMAL, 	0 },
	{ "gold",			do_worth,			POS_SLEEPING,	0,	LOG_NORMAL, 	0 },
	{ "silver",			do_worth,			POS_SLEEPING,	0,	LOG_NORMAL, 	0 },

	{ "yell",			do_yell,			POS_RESTING,	0,	LOG_NORMAL, 	1 },
	{ "zap",			do_zap, 			POS_RESTING,	0,	LOG_NORMAL, 	1 },
		
//	Putting Weapon skills down here so they don't interfere with other commands
	{ "boneshatter",	do_boneshatter, 	POS_FIGHTING,	IM,  LOG_ALWAYS,	0 },
	{ "charge", 		do_charge,			POS_STANDING,	IM,  LOG_ALWAYS,	0 },
	{ "conceal",		do_conceal, 		POS_RESTING,	IM,  LOG_ALWAYS,	0 },
	{ "cutoff", 		do_cutoff,			POS_FIGHTING,	IM,  LOG_ALWAYS,	0 },
	{ "dervish",		do_dervish, 		POS_FIGHTING,	IM,  LOG_ALWAYS,	0 },
	{ "entangle",		do_entangle,		POS_FIGHTING,	IM,  LOG_ALWAYS,	0 },
	{ "hobble", 		do_hobble,			POS_FIGHTING,	IM,  LOG_ALWAYS,	0 },
	{ "hurl",			do_hurl,			POS_FIGHTING,	IM,  LOG_ALWAYS,	0 },
	{ "overhead",		do_overhead,		POS_FIGHTING,	IM,  LOG_ALWAYS,	0 },
	{ "shieldcleave",	do_shieldcleave,	POS_FIGHTING,	IM,  LOG_ALWAYS,	0 },
	{ "vault",			do_vault,			POS_STANDING,	IM,  LOG_ALWAYS,	0 },
		
	// new mapping features - mortal		
	{ "automap",		do_automap, 		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "autoreset",		do_autoreset,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "autosaymote",	do_autosaymote, 	POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "autosaycolourcodes",	do_autosaycolourcodes, 	POS_DEAD,0,	LOG_ALWAYS, 	1 },	
	{ "autovote",		do_autovote,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "map",			do_map, 			POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "mapclear",		do_mapclear,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "nomapexits", 	do_nomapexits,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },
	{ "nomapblanks", 	do_nomapblanks,		POS_DEAD,		0,	LOG_ALWAYS, 	1 },	
	{ "areamap",		do_areamap, 		POS_DEAD,		IM+1,LOG_ALWAYS,	1 },
	{ "scalemap",		do_scalemap,		POS_DEAD,		IM+1,LOG_ALWAYS,	1 },
	{ "scalemxp",		do_scalemxp,		POS_DEAD,		IM+1,LOG_ALWAYS,	1 },
	// new mapping features - immortal only 
	{ "mapnum", 		do_mapnum,			POS_DEAD,		IM,	LOG_ALWAYS, 	1 },
	{ "fullmap",		do_fullmap, 		POS_DEAD,		IM,	LOG_ALWAYS, 	1 },
	{ "amap",			do_amap,			POS_DEAD,		IM,	LOG_ALWAYS, 	1 },

	  /*
	   *	--==  Immortal commands ==--
	   *
	   *		ML	MAX_LEVEL	   =  implementor	(100)
	   *		L1	MAX_LEVEL - 1  =  creator		(99)
	   *		L2	MAX_LEVEL - 2  =  supreme being (98)
	   *		L3	MAX_LEVEL - 3  =  deity 		(97)
	   *		L4	MAX_LEVEL - 4  =  god			(96)
	   *		L5	MAX_LEVEL - 5  =  immortal		(95)
	   *		L6	MAX_LEVEL - 6  =  demigod		(94)
	   *		L7	MAX_LEVEL - 7  =  angel 		(93)
	   *		L8	MAX_LEVEL - 8  =  avatar		(92)
	   *		IM	LEVEL_IMMORTAL =  angel 		(92)
	   *		HE	LEVEL_HERO	   =  hero			(91)
	   */

	/* first imm commands that don't show up on wizhelp menu */
	{ "reboo",			do_reboo,			POS_DEAD,		L2, LOG_NORMAL,		0 },
	{ "shutdow",		do_shutdow, 		POS_DEAD,		L2, LOG_NORMAL,		0 },
	{ "sla",			do_sla, 			POS_DEAD,		L3, LOG_NORMAL,		0 },
	{ "invis",			do_wizinvis,		POS_DEAD,		IM, LOG_NORMAL,		0 },
		
	/* immortal commands on the menus */		
	{ "admintalk",		do_admintalk,		POS_DEAD,	ADMIN,	LOG_NORMAL,		1 },
	{ "addclan",		do_addclan, 		POS_DEAD,		L4, LOG_ALWAYS,		1 },
	{ "addcourt",		do_addcourt,		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "addmoot",		do_addmoot, 		POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "addscript",		do_addscript,		POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "advance",		do_advance, 		POS_DEAD,		L1, LOG_ALWAYS,		1 },
	{ "allow",			do_allow,			POS_DEAD,		L2, LOG_ALWAYS,		1 },
	{ "allow",			do_allow,			POS_DEAD,		L3, LOG_ALWAYS,		0 },
	{ "allowimmtalk",	do_allowimmtalk,	POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "anote",			do_anote,			POS_DEAD,	ADMIN+1,LOG_NORMAL,		1 },
	{ "anote",			do_anote,			POS_DEAD,	ADMIN,	LOG_NORMAL,		0 },
	{ "atell",			do_atell,			POS_DEAD,		IM, LOG_NORMAL,		1 },
	{ "atlevel",		do_atlevel, 		POS_DEAD,		L6, LOG_NORMAL,		1 },
	//{ "ban",			do_ban, 			POS_DEAD,		L4, LOG_ALWAYS,		1 }, // earlier version catches it
	{ "banedit",		do_banedit, 		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "bardify",		do_bardify, 		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "breaktell",		do_breaktell,		POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "boon",			do_boon,			POS_DEAD,		ML, LOG_ALWAYS,		1 },
	{ "castatlevel",	do_castatlevel, 	POS_FIGHTING,	IM, LOG_ALWAYS,		1 },
	{ "chardesc",		do_chardescript,	POS_DEAD,		1,	LOG_ALWAYS,		1 },
	{ "charhistory",	do_charhistory,		POS_DEAD,		1,	LOG_ALWAYS,		1 },
	{ "charinfo",		do_charinfo,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },	
	{ "charnotes",		do_charnotes,		POS_DEAD,		L6, LOG_ALWAYS,		1 },
	{ "checkbug",		do_checkbug,		POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "checkclanbank",	do_checkclanbank,	POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "checkcode",		do_checkcode,		POS_DEAD,	ML-1,	LOG_ALWAYS,		1 },
	{ "checkdead",		do_checkdead,		POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "checkhelps",		do_checkhelp,		POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "checklog",		do_checklog,		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "checkirc",		do_checkirc,		POS_DEAD,		L6, LOG_ALWAYS,		1 },
	{ "checkbalance",	do_checkbalance,	POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "checkexits", 	do_checkexits,		POS_DEAD,		L6, LOG_ALWAYS,		1 },
	{ "checkmoblog",	do_checkmoblog, 	POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "checkooc",		do_checkooc,		POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "checkmultilog",	do_checkmultilog,	POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "checknewbie",	do_checknewbie, 	POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "checknsup",		do_checknsupport,	POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "checkntalk", 	do_checkntalk,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "checktypos", 	do_checktypos,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "checkban",		do_checkban,		POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "cinfo",			do_cinfo,			POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "clist",			do_clanlist,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "clanlist",		do_clanlist,		POS_DEAD,		IM,	LOG_NORMAL,		0 },
	{ "class",			do_class,			POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "createspell",	do_createspell, 	POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	
	{ "clone",			do_clone,			POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "cwhere", 		do_cwhere,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "codetalk",		do_codetalk,		POS_DEAD,		IM,	LOG_NORMAL,		0 },
	{ "clear_createcount",do_clear_createcount,POS_DEAD,ML-1,	LOG_ALWAYS,		1 },
	{ "councillist",	do_councillist, 	POS_DEAD,	IM+1,	LOG_ALWAYS,		1 },
	{ "delmoot",		do_delmoot, 		POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "delscript",		do_delscript,		POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "delquest",		do_delquest,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "deny",			do_deny,			POS_DEAD,		L2,	LOG_ALWAYS,		1 },	
	{ "disable",		do_disable, 		POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "disconnect", 	do_disconnect,		POS_DEAD,		L3,	LOG_ALWAYS,		1 },
	{ "disallowpkill",	do_disallowpkill,	POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "dlook",			do_dlook,			POS_DEAD,		1,	LOG_NORMAL,		1 },
	{ "dofor",			do_for, 			POS_DEAD,	ADMIN+1,LOG_ALWAYS,		1 },
	{ "dream",			do_dream,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "dumpeventqueue",	do_dumpeventqueue,	POS_DEAD,	ADMIN+1,LOG_ALWAYS,		1 },
	{ "dumpstats",		do_dumpstats,		POS_DEAD,		ML, LOG_ALWAYS,		0 },
	{ "echo",			do_echo,			POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "editcharnote",	do_editcharnotes,	POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "exitlist",		do_exitlist,		POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "exlist", 		do_exitlist,		POS_DEAD,		L4,	LOG_ALWAYS,		0 },
	{ "fadein", 		do_fadein,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "fadeout",		do_fadeout, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "flag",			do_flag,			POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "freeze", 		do_freeze,			POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "force",			do_force,			POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "mforce",			do_mforce,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "nforce",			do_nforce,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "forcetick",		do_forcetick,		POS_DEAD,		L1,	LOG_ALWAYS,		1 },
	{ "gecho",			do_gecho,			POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "got",			do_goto,			POS_DEAD,		L8,	LOG_NORMAL,		0 },
	{ "goecho", 		do_goecho,			POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "goto",			do_goto,			POS_DEAD,		L8,	LOG_NORMAL,		1 },
	{ "herbedit",		do_herbedit,		POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "holylist",		do_holylist,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "holylight",		do_holylight,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "holyname",		do_holyname,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "holyspeech", 	do_holyspeech,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "holywalk",		do_holywalk,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "holyvnum",		do_holyvnum,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "hotreboot",		do_hotreboot,		POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "hightalk",		do_hightalk,		POS_DEAD,	ML-1, 	LOG_NORMAL,		1 },
	{ "ignoremulti",	do_ignoremultilogins,POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "hideareas",		do_hide_hidden_areas,POS_DEAD,		IM, LOG_NORMAL,		1 },	
	{ "immtalk",		do_immtalk, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "immget",			do_immget, 			POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "immtitle",		do_immtitle, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "immwiznet",		do_immwiznet,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "immwiznetc", 	do_immwiznetc,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "imotd",			do_imotd,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "incognito",		do_incognito,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "inote",			do_inote,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "inroom", 		do_inroom,			POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "iwizi",			do_iwizi,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "iwhere", 		do_iwhere,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "jail",			do_jail,			POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "ktell",			do_ktell,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
		
	{ "lastoncouncil",	do_lastoncouncil,	POS_DEAD,	ML-1,	LOG_ALWAYS,		1 },
	{ "lastonremove",	do_lastonremove,	POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "lastonstats",	do_lastonstats, 	POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "langedit",		do_langedit, 		POS_DEAD,		ML,	LOG_ALWAYS,		1 },	
	{ "letgain",		do_letgain, 		POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "lethero",		do_lethero, 		POS_DEAD,		L3,	LOG_ALWAYS,		1 },
	{ "listscripts",	do_listscripts, 	POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "listquest",		do_listquest,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "load",			do_load,			POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "localechos", 	do_localecho,		POS_DEAD,		1,	LOG_ALWAYS,		0 },
	{ "localecho",		do_localecho,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "log",			do_log, 			POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "memory", 		do_memory,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "mixedit",		do_mixedit, 		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "mixshow",		do_mixshow, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "mythostalk", 	do_mythostalk,		POS_DEAD,		IM,	LOG_NORMAL,		0 },
	{ "mwhere", 		do_mwhere,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "mpdump", 		do_mpdump,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "mplist", 		do_mplist,			POS_DEAD,		2,	LOG_ALWAYS,		1 },
	{ "mpstat", 		do_mpstat,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "mudclientstats",	do_mudclientstats,	POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "mxpinfo",		do_mxpinfo,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },	
	{ "newbielock",		do_newbielock, 		POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "noble",			do_noble,			POS_DEAD,		L1,	LOG_ALWAYS,		1 },
	{ "nochannel",		do_nochannels,		POS_DEAD,		2,	LOG_ALWAYS,		0 },
	{ "nochannels", 	do_nochannels,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "noemote",		do_noemote, 		POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "norp",			do_norp,			POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "noshout",		do_noshout, 		POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "notell", 		do_notell,			POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "noterestrict",	do_noterestrict,	POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "nopray", 		do_nopray,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "nsupport",		do_nsupport,		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "owhere", 		do_owhere,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "listletgain",	do_list_letgains,	POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "listmoot",		do_listmoot,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "clearletgain",	do_clearletgain,	POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "offletgain", 	do_offlineletgain,	POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "offlineletgain", do_offlineletgain,	POS_DEAD,		L7,	LOG_ALWAYS,		0 },
	{ "offmoot",		do_offmoot, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "declinelet", 	do_declineletgain,	POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "declineletgain", do_declineletgain,	POS_DEAD,		L7,	LOG_ALWAYS,		0 },
	{ "page",			do_page,			POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "pdisable",		do_pdisable,		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "peace",			do_peace,			POS_DEAD,		L5,	LOG_NORMAL,		1 },
	{ "pecho",			do_pecho,			POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "penalty",		do_penalty, 		POS_DEAD,		L8,	LOG_NORMAL,		1 },
	{ "penable",		do_penable, 		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "permban",		do_permban, 		POS_DEAD,		L2,	LOG_ALWAYS,		1 },
	{ "permrawcol", 	do_permrawcolour,	POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "pinfo",			do_pinfo,			POS_DEAD,		L8,	LOG_NORMAL,		1 },
	{ "playerlock", 	do_playerlock,		POS_DEAD,		L2,	LOG_ALWAYS,		1 },
	{ "pload", 			do_pload,			POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "poofin", 		do_bamfin,			POS_DEAD,		L8,	LOG_NORMAL,		1 },
	{ "poofout",		do_bamfout, 		POS_DEAD,		L8,	LOG_NORMAL,		1 },
	{ "prefix", 		do_prefix,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "pracsystester",	do_pracsystester,	POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "protect",		do_protect, 		POS_DEAD,		L1,	LOG_ALWAYS,		1 },
	{ "purge",			do_purge,			POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "qdie",			do_qdie,			POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "rawcolour",		do_rawcolour,		POS_DEAD,		 0,	LOG_ALWAYS,		1 },
	{ "reboot", 		do_reboot,			POS_DEAD,		L2,	LOG_ALWAYS,		1 },
	{ "rebootresolver", do_rebootresolver,	POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "removequester",	do_removequester,	POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "rename", 		do_rename,			POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "resolve",		do_resolve,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "restore",		do_restore, 		POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "release",		do_release,			POS_DEAD,		IM, LOG_ALWAYS,		1 },
	{ "return", 		do_return,			POS_DEAD,		L6,	LOG_NORMAL,		1 },
	{ "return", 		do_return,			POS_DEAD,		1,	LOG_ALWAYS,		0 },
	{ "rpobjload",		do_rp_obj_load, 	POS_DEAD,		1,	LOG_ALWAYS,		0 },
	{ "rpsupport",		do_rpsupport,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "runscript",		do_runscript,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "rwhere", 		do_rwhere,			POS_DEAD,		L6,	LOG_NORMAL,		1 },
	{ "seevnum",		do_seevnum, 		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "setalliance",	do_setalliance, 	POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "setrace",		do_setrace, 		POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "settendency",	do_settendency, 	POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "sgive",			do_sgive,			POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "short",			do_short,			POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "showaffectprofile",do_showaffectprofile,POS_DEAD,	L6,	LOG_ALWAYS,		1 },
	{ "showban",		do_showban, 		POS_DEAD,COUNCIL-1,	LOG_NORMAL,		0 },
	{ "shutdown",		do_shutdown,		POS_DEAD,		L2,	LOG_ALWAYS,		1 },
	{ "silently",		do_silently,		POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "slay",			do_slay,			POS_DEAD,		L3,	LOG_ALWAYS,		1 },
	{ "snoop",			do_snoop,			POS_DEAD,		L6,	LOG_ALWAYS,		1 },		
	{ "snote",			do_snote,			POS_DEAD,		IM,	LOG_NORMAL, 	1 },
	{ "sockets",		do_sockets, 		POS_DEAD,		L4,	LOG_NORMAL,		1 },
	{ "social_import",	do_social_import,	POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "socedit",		do_socialedit,		POS_DEAD,	COUNCIL,LOG_ALWAYS,		0 },
	{ "socialedit", 	do_socialedit,		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "spdebug",		do_spelldebug,		POS_DEAD,		ML,	LOG_NORMAL,		1 },
	{ "stat",			do_stat,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "string", 		do_string,			POS_DEAD,		HE,	LOG_ALWAYS,		1 },
	{ "stripaff",		do_strip_affects,	POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "stransfer",		do_stransfer,		POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "switch", 		do_switch,			POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "switchprefix", 	do_switchprefix,	POS_DEAD,		L6,	LOG_ALWAYS,		1 },
	{ "oextended",		do_oextended,		POS_DEAD,		HE,	LOG_ALWAYS,		1 },
	{ "olcwizi",		do_olcwizi, 		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "owizi",			do_owizi,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "tadvance",		do_tadvance,		POS_DEAD,		L1,	LOG_ALWAYS,		1 },
	{ "teleport",		do_transfer,		POS_DEAD,		L5,	LOG_ALWAYS,		1 },
	{ "telnetga",		do_telnetga,		POS_DEAD,		0,	LOG_ALWAYS,		0 },
	{ "telnetga",		do_telnetga,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "trapset",		do_trapset, 		POS_DEAD,		1,	LOG_ALWAYS,		1 },
	{ "trapshow",		do_trapshow,		POS_DEAD,		1,	LOG_NORMAL,		1 },
	{ "trapremove", 	do_trapremove,		POS_DEAD,		1,	LOG_ALWAYS,		1 },
	{ "transfer",		do_transfer,		POS_DEAD,		L7,	LOG_ALWAYS,		1 },
	{ "trust",			do_trust,			POS_DEAD,		L1,	LOG_ALWAYS,		1 },
	{ "testhelps",		do_testhelps,		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "testgeneric",	do_testgeneric,		POS_RESTING,	IM, LOG_NORMAL, 	1 },
	{ "tgive",			do_tgive,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "tremove",		do_tremove, 		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "ttimer", 		do_ttimer,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "tjunk",			do_tjunk,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "toprpers",		do_toprp,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "topwealth",		do_topwealth,		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "twhere", 		do_twhere,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "undeny",			do_undeny,			POS_DEAD,		L2,	LOG_ALWAYS,		1 },
	{ "unletgain",		do_unletgain,		POS_DEAD,		L3,	LOG_ALWAYS,		1 },
	{ "unlethero",		do_unlethero,		POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "update", 		do_update,			POS_DEAD,		L1,	LOG_ALWAYS,		1 },
	{ "webpass",		do_webpass, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "where",			do_where,			POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "whovis", 		do_whovis,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "whoinvis",		do_whoinvis,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
//	{ "who1234",		do_who1234, 		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
//	{ "who4321",		do_who4321, 		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	{ "whotext",		do_whotext, 		POS_DEAD,	AML,	LOG_ALWAYS,		1 },
	{ "wizhelp",		do_wizhelp, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "wizinvis",		do_wizinvis,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "wizlistedit",	do_wizlistedit, 	POS_DEAD,	    L2,	LOG_ALWAYS, 	1 },
	{ "wiznet", 		do_wiznet,			POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "wiznetc",		do_wiznetc, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "wiznet2",		do_wiznet2, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "wiznet2c",		do_wiznet2c,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "wiznet3",		do_wiznet3, 		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "wiznet3c",		do_wiznet3c,		POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "wiznetdefault", 	do_wiznetdefault,	POS_DEAD,		IM,	LOG_NORMAL,		1 },
	{ "violate",		do_violate, 		POS_DEAD,		L2,	LOG_ALWAYS,		1 },
	{ "visualdebug",	do_visualdebug, 	POS_DEAD,		IM,	LOG_ALWAYS, 	1 },
	{ "vnum",			do_vnum,			POS_DEAD,		L6,	LOG_NORMAL,		1 },
	{ "vnummap",		do_vnummap, 		POS_DEAD,COUNCIL-1,	LOG_ALWAYS,		1 },
	{ "xpen",			do_xpen,			POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "zecho",			do_zecho,			POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "zoecho", 		do_zoecho,			POS_DEAD,		L4,	LOG_ALWAYS,		1 },
	{ "notenet",		do_notenet, 		POS_DEAD,		IM,	LOG_ALWAYS,		1 },
	
// OLC	
	
	{ "skillgroupedit",	do_skillgroupedit,	POS_DEAD,		ML,	LOG_ALWAYS,		0 },
	{ "skgroupedit",	do_skillgroupedit,	POS_DEAD,		ML,	LOG_ALWAYS,		0 },
	{ "skgrpedit",		do_skillgroupedit,	POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "gameedit",		do_gameedit,		POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "asave",			do_asave,			POS_DEAD,		0,	LOG_ALWAYS,		1 },
	{ "aslist",			do_aslist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "alist",			do_alist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "aroomlist",		do_aroomlist,		POS_DEAD,		0,	LOG_ALWAYS,		1 },
	{ "autobalance",	do_autobalance, 	POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "bolist", 		do_bolist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "bollist", 		do_bollist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "bovlist", 		do_bovlist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "bmlist", 		do_bmlist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "bmllist", 		do_bmllist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "bmvlist", 		do_bmvlist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "hlist",			do_hlist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "resets", 		do_resets,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "redit",			do_redit,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "rlist",			do_rlist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "brlist", 		do_brlist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "rpurge", 		do_rpurge,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "medi",			do_huh, 			POS_DEAD,		0,	LOG_NORMAL,		0 },
	{ "medit",			do_medit,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "mlist",			do_mlist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "mobloglist", 	do_mobloglist,		POS_DEAD,		0,	LOG_ALWAYS,		1 },
	{ "mxpmodlist", 	do_mxpmodlist,		POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "mphelp",			do_mphelp, 			POS_DEAD,		0,	LOG_ALWAYS,		1 },
	{ "mobhelp",		do_mphelp, 			POS_DEAD,		0,	LOG_ALWAYS,		0 }, // historical reasons
	{ "mudhelp",		do_mphelp, 			POS_DEAD,		0,	LOG_ALWAYS,		0 }, // historical reasons
	{ "ifhelp", 		do_ifhelp,			POS_DEAD,		0,	LOG_ALWAYS,		1 },
	{ "aedit",			do_aedit,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "ashow",			do_ashow,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "oedit",			do_oedit,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "olist",			do_olist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "oshow",			do_oshow,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "hedit",			do_hedit,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "hsave",			do_hsave,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "mshow",			do_mshow,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "mpshow", 		do_mpshow,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "rshow",			do_rshow,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "mpedit", 		do_mpedit,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "vlist",			do_vlist,			POS_DEAD,		0,	LOG_OLC,		1 },
	{ "olcgoto",		do_olcgoto, 		POS_DEAD,		0,	LOG_ALWAYS,		1 },
		
	{ "pkill",			do_pkill,			POS_FIGHTING,	0,	LOG_NORMAL,		1 },
	{ "pknote", 		do_pknote,			POS_DEAD,		0,	LOG_NORMAL,		1 },
	{ "pbackstab",		do_pbackstab,		POS_FIGHTING,	0,	LOG_NORMAL,		0 },
	{ "pbs",			do_pbackstab,		POS_FIGHTING,	0,	LOG_NORMAL,		0 },
		
	{ "pip",			do_huh, 			POS_DEAD,	ADMIN,	LOG_ALWAYS,		0 },
	{ "pipe",			do_pipe,			POS_DEAD,	ADMIN,	LOG_ALWAYS,		0 },
	{ "syste",			do_huh, 			POS_DEAD,	ADMIN,	LOG_ALWAYS,		0 },
	{ "system", 		do_system,			POS_DEAD,	ADMIN,	LOG_ALWAYS,		0 },
			
	{ "crashloo",		do_huh, 			POS_DEAD,		ML,	LOG_ALWAYS,		0 },
	{ "crashloop",		do_crashloop,		POS_DEAD,		ML,	LOG_ALWAYS,		0 },
		
	{ "mpreset",		do_mpreset, 		POS_DEAD,		1,	LOG_ALWAYS,		1 },
	{ "mpinfo", 		do_mpinfo,			POS_DEAD,		1,	LOG_ALWAYS,		1 },
	{ "mptrace",		do_mptrace, 		POS_DEAD,		1,	LOG_ALWAYS,		1 },
	{ "showplayerlist", do_showplayerlist,	POS_DEAD,		ML,	LOG_ALWAYS,		0 },
	{ "reducelaston",	do_reducelaston,	POS_DEAD,	COUNCIL-1,LOG_ALWAYS,	1 },
	{ "remort", 		do_remort,			POS_DEAD,	ML,		LOG_ALWAYS,		1 },
	{ "resetroo",		do_huh, 			POS_DEAD,	COUNCIL-2,LOG_NORMAL,	0 },
	{ "resetroom",		do_resetroom,		POS_DEAD,	COUNCIL-2,LOG_ALWAYS,	1 },
	{ "resetare",		do_huh, 			POS_DEAD,	COUNCIL-1,LOG_NORMAL,	0 },
	{ "resetarea",		do_resetarea,		POS_DEAD,	COUNCIL-1,LOG_ALWAYS,	1 },
	{ "rinfo",			do_rinfo,			POS_DEAD,	COUNCIL,LOG_NORMAL,		1 },
	{ "showplayerlist", do_showplayerlist,	POS_DEAD,	ML,		LOG_ALWAYS,		0 },
	{ "commandsnoop",	do_commandsnoop,	POS_DEAD,	COUNCIL,LOG_ALWAYS,		0 },
	{ "comedit",		do_comedit, 		POS_DEAD,	ML-1,	LOG_ALWAYS,		1 },
	{ "csnoop", 		do_commandsnoop,	POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "dedit",			do_dedit,			POS_DEAD,	COUNCIL,LOG_ALWAYS,		1 },
	{ "owritepassw",	do_huh, 			POS_DEAD,	ML-1,	LOG_ALWAYS,		0 },
	{ "overwritepassw", do_huh, 			POS_DEAD,	ML-1,	LOG_ALWAYS,		0 },
	{ "overwritepassword",do_overwritepasswd,POS_DEAD,	ML-1,	LOG_ALWAYS,		0 },
	{ "owritepassword", do_overwritepasswd, POS_DEAD,	ML-1,	LOG_ALWAYS,		0 },
	{ "owritepasswd",	do_overwritepasswd, POS_DEAD,	ML-1,	LOG_ALWAYS,		1 },
	{ "objrestrict",	do_objrestrict, 	POS_DEAD,	LEVEL_HERO,LOG_ALWAYS,	1 },
	{ "savedeitydb",	do_savedeities, 	POS_DEAD,	ML-1,	LOG_ALWAYS,		1 },
	{ "saveherbs",		do_saveherbs,		POS_DEAD,	COUNCIL-2,LOG_ALWAYS,	1 },
	{ "savemixdb",		do_savemixdb,		POS_DEAD,	COUNCIL-2,LOG_ALWAYS,	1 },


	{ "relookup",		do_relookup,		POS_DEAD,	ADMIN,	LOG_ALWAYS,		1 },
	{ "raceedit",		do_raceedit, 		POS_DEAD,RACEEDIT_MINTRUST,	LOG_OLC,1 },
	{ "npcinfo",		do_npcinfo, 		POS_DEAD,	IM,		LOG_OLC, 1 },
	{ "racelist",		do_racelist,		POS_DEAD,	COUNCIL,LOG_OLC, 1 },
	{ "classedit",		do_classedit,		POS_DEAD,	COUNCIL,LOG_OLC,		1 },	
	{ "write_skills",	do_write_skills,	POS_DEAD,	ML,		LOG_ALWAYS,		1 },
	{ "write_languages",do_write_languages ,POS_DEAD,	ML,		LOG_ALWAYS,		1 },	
	{ "cedit",			do_classedit,		POS_DEAD,	ML,		LOG_OLC,		1 },	
	{ "clanedit",		do_clanedit,		POS_DEAD,	ML,		LOG_OLC,		1 },	
//	{ "write_dynamic_inc", do_write_dynamic_include, POS_DEAD,	ML, LOG_ALWAYS, 1 },
	{ "write_classes",	do_write_classes,	POS_DEAD,	ML,		LOG_ALWAYS,		1 },
	{ "read_classes",	do_read_classes,	POS_DEAD,	ML,		LOG_ALWAYS,		1 },

	{ "write_skillgroups",do_write_skillgroups, POS_DEAD,	ML,		LOG_ALWAYS,		1 },

	{ "read_nameprofiles",do_read_nameprofiles,POS_DEAD,	ML,	LOG_ALWAYS,		1 },
	{ "write_nameprofiles",do_write_nameprofiles,POS_DEAD,	ML,	LOG_ALWAYS,		1 },
	
	{ "saveclans", 		do_saveclans,		POS_DEAD,		ML,	LOG_ALWAYS,		1 },
	{ "loadclans", 		do_loadclans,		POS_DEAD,		ML,	LOG_ALWAYS,		1 },

	{ "write_comtable", do_write_commandtable,POS_DEAD,ML-1,	LOG_ALWAYS,		1 },
	{ "saveraces",		do_saveraces,		POS_DEAD,	RACEEDIT_MINTRUST, LOG_ALWAYS, 1 },
	{ "savequestdb",	do_savequestdb, 	POS_DEAD,	ML,		LOG_ALWAYS,		1 },

	{ "savegameset",	do_save_gamesettings,	POS_DEAD,	ML,	LOG_ALWAYS,		1 },
	{ "loadgameset",	do_load_gamesettings,	POS_DEAD,	ML,	LOG_ALWAYS,		1 },
	{ "saveautostat",	do_save_autostat_files,	POS_DEAD,	ML,	LOG_ALWAYS,		1 },
	{ "loadautostat",	do_load_autostat_files,	POS_DEAD,	ML,	LOG_ALWAYS,		1 },

	{ "savecorpses",	do_save_corpses,	POS_DEAD,	ML,		LOG_ALWAYS,		1 },
	{ "loadcorpses",	do_load_corpses,	POS_DEAD,	ML,		LOG_ALWAYS,		1 },
		
	{ "material_list",	do_material_list,	POS_DEAD,	IM,		LOG_ALWAYS,		1 },
	{ "sedit",			do_sedit,			POS_DEAD,	IM,		LOG_ALWAYS,		1 },
	{ "sshow",			do_sshow,			POS_DEAD,	IM,		LOG_ALWAYS,		1 },
	{ "spsklist",		do_spellskilllist,	POS_DEAD,	IM,		LOG_ALWAYS,		1 },
	{ "qedit",			do_qedit,			POS_DEAD,	IM+2,	LOG_ALWAYS,		1 },
																				
	{ ":",				do_immtalk, 		POS_DEAD,	0,		LOG_NORMAL,		0 },
	{ "immtalk",		do_immtalk, 		POS_DEAD,	0,		LOG_NORMAL,		0 },
	{ "saycolour",		do_saycolour,		POS_DEAD,	IM,		LOG_NORMAL,		1 },
	{ "saycolor",		do_saycolour,		POS_DEAD,	IM,		LOG_NORMAL,		0 },
	{ "motecolour", 	do_motecolour,		POS_DEAD,	IM,		LOG_NORMAL,		1 },
	{ "motecolor",		do_motecolour,		POS_DEAD,	IM,		LOG_NORMAL,		0 },
	{ "textsearch",		do_textsearch, 		POS_DEAD,	IM,		LOG_ALWAYS,		0 },
																				
	{ "debugroom",		do_debugroom,		POS_DEAD,	ML,		LOG_ALWAYS,		1 },
	{ "debugmob",		do_debugmob ,		POS_DEAD,	ML,		LOG_ALWAYS,		1 },
	{ "debugobject",	do_debugobject, 	POS_DEAD,	ML,		LOG_ALWAYS,		1 },
	{ "makecorefile",	do_makecorefile,	POS_DEAD,	DIS,	LOG_ALWAYS,		1 },

//	{ "manual_colour_convert",	do_manual_colour_convert,	POS_DEAD,	DIS,		LOG_ALWAYS,		1 },	
	{ "detect_oldstyle_note_writing", do_detect_oldstyle_note_writing, POS_DEAD,0,LOG_ALWAYS,0 },	
	/*
	 * End of list.
	 */
	 { "",		 0, 	 POS_DEAD,	  0,	LOG_NORMAL, 0 }
};


// memory tracking system
extern int nAllocString;
extern int nAllocPerm;

extern bool interpret_not_used;
/**************************************************************************/
/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( char_data *ch, char *argument )
{
    char command[MIL];
    char logline[MSL];
    char argall[MIL];
    int cmd;
    int trust;
    bool found;

	interpret_not_used=false;

    // memory checking
    int string_count = nAllocString;
    int perm_count = nAllocPerm;
    char cmd_copy[MIL];
    char buf[MSL];

    // Implement freeze command.
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
        ch->println("You're totally frozen!");
        return;
    }

	// SMASH THAT PESKY !!SOUND
	// Admins are exempt cause we're better :) hehehe
	if ( !IS_NULLSTR( argument ) && str_len(argument) > 7 )
	{
		if ( !IS_ADMIN( ch ))
		{
			if ( !str_infix( "!!SOUND", argument )
			||   !str_infix( "!!MUSIC", argument ))
			{
				ch->println("Invalid input, don't try and defraud the MSP trigger.");
				return;
			}
		}
	}			

	strcpy(argall, argument);

    // Strip leading spaces.
    while ( is_space(*argument) ){
		argument++;
	}

	// if no command remains, do nothing
    if ( IS_NULLSTR(argument) ){
		return;
	}

    // wiznet memory debugging
    strcpy(cmd_copy, argument);


	/*
	 * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !is_alpha(argument[0]) && (!is_digit(argument[0]) || (argument[0]=='0') )) {
		command[0] = argument[0];
		command[1] = '\0';
		argument++;
		while ( is_space(*argument) ){
			argument++;
		}
	}else if((argument[0]=='m' || argument[0]=='M') &&  (argument[1]=='q' || argument[1]=='Q')){
		command[0] = 'm'; // support mq12 syntax
		command[1] = 'q';
		command[2] = '\0';
		argument+=2;
		while ( is_space(*argument) ){
			argument++;
		}
    }else{
		argument = one_argument( argument, command );
    }


	{	// MOB TRIGGER COMMAND
		char_data *rch=ch->in_room->people;
		int number_in_room=ch->in_room->number_in_room;
		// note: number_in_room is used to prevent a mudprog on two mobs in 
		// a room creating an endless loop by removing themselves from
		// the room and putting themselves back in the room - Kal, June 01
		for ( ;rch && --number_in_room>=0; rch= rch->next_in_room )
		{
			if ( IS_NPC(rch) && HAS_TRIGGER( rch, MTRIG_COMMAND ))
			{
				if (mp_cmd_trigger( argall, rch, ch, NULL, NULL, MTRIG_COMMAND )){
					return;
				}
			}
		}
	}


    // Look for command in command table.
    found = false;
    trust = get_trust( ch );
	for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
		if ( command[0] == cmd_table[cmd].name[0]
		&&   !str_prefix( command, cmd_table[cmd].name ))
		{
			if(cmd_table[cmd].level <= trust 
				|| (trust==ML && AML==ML && cmd_table[cmd].level-1 <= trust))
			{
				// complete the command name
				sprintf(&logline[str_len(logline)], " (%s)", cmd_table[cmd].name);
				found = true;
				break;
			}
		}
    }

	// dyn stuff placed here
	if ( !IS_IMMORTAL( ch ))
	{
		if ( IS_SET( cmd_table[cmd].flags, CMD_OOC )
			&& !IS_OOC(ch)) {
			ch->println("You can only use this command while in an OOC room.");
			return;
		}

		if ( IS_SET( cmd_table[cmd].flags, CMD_IC )
			&& IS_OOC(ch)) {
			ch->println("You can only use this command while you are IC.");
			return;
		}

		if ( IS_SET( cmd_table[cmd].flags, CMD_NO_TREEFORM )
			&& IS_AFFECTED2( ch, AFF2_TREEFORM)) {
			ch->println("You can't do this while in the form of a tree.");
			return;
		}

		if ( IS_SET( cmd_table[cmd].flags, CMD_NO_STONE_GARGOYLE)
			&& IS_AFFECTED2( ch, AFF2_STONE_GARGOYLE)) 
		{
			ch->println( "Not while you are a statue." );
			return;
		}

	 	if ( IS_SET( cmd_table[cmd].flags, CMD_NO_STOCKADE )
			&& IS_AFFECTED2( ch, AFF2_STOCKADE)) 
		{
 			ch->printlnf( "The stocks prevent you from doing that." );
 			return;
        }
	}

	if ( !IS_ADMIN( ch ))
	{
		if ( IS_SET( cmd_table[cmd].flags, CMD_NO_ORDER )
			&& IS_SET( ch->dyn, DYN_IS_BEING_ORDERED )) 
		{
			ch->println("You can't be ordered to do that.");
			ch->master->println("Not going to happen.");
			return;
		}
	}

	// check it isn't for a restricted council
	if (cmd_table[cmd].level>=LEVEL_IMMORTAL){ 
		// If we have any council restriction limit it
		if(cmd_table[cmd].council && !IS_ADMIN(ch)){
			if( TRUE_CH(ch)->pcdata 
				&& IS_SET( TRUE_CH(ch)->pcdata->council, cmd_table[cmd].council))
			{
				// council restricted command, in council
			}else{
				ch->println("This command can only be used by a select group of councils.");
				return;
			}
		}
	}


	// Log and snoop.
    //   - removed (so snoop gets directions)
	if( cmd_table[cmd].log == LOG_NEVER && !(IS_NPC(ch) && IS_SET(ch->act,ACT_MOBLOG)) ){
		strcpy( logline, "NOT SNOOPABLE" );
	}
 
    if (cmd_table[cmd].log < LOG_DONT_LOG) // log might be required 
    {
		if (cmd_table[cmd].log == LOG_OLC)
		{
			char buffilename[MSL];
			if (!IS_UNSWITCHED_MOB(ch))
			{
				sprintf(buffilename, OLC_LOGS_DIR"%.8s.olc", lowercase(ch->name));
				append_logentry_to_file( ch, buffilename, logline);
			}
			append_logentry_to_file( ch, OLC_LOGFILE, logline);
		}

        if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG))
            ||   fLogAll
            ||   cmd_table[cmd].log == LOG_ALWAYS 
            || (!IS_NPC(ch) && cmd_table[cmd].log == LOG_PALWAYS) )
        {
			if (TRUE_CH(ch)!=ch){
		        sprintf( log_buf, "Log %s (%s)[%d]: %s", ch->name, 
					TRUE_CH(ch)->name, ch->in_room_vnum(), logline );
			}else{
				sprintf( log_buf, "Log %s<%d,%d>[%d]: %s", 
					ch->name, ch->vnum(), ch->level, ch->in_room_vnum(),  logline );
			}
    
			// handle global logging of player input to the main log
			{
				bool bLogInputToSystemLog=fLogAll; // default to the global log setting
				bool bLogToWiznetSecure=!IS_SET(cmd_table[cmd].flags, CMD_NO_WIZNET_SECURE);

				switch(cmd_table[cmd].log){
				case LOG_NEVER: // never log this command regardless of global/player settings
					bLogInputToSystemLog=false;
					bLogToWiznetSecure=false;
					break; 
					
				case LOG_ALWAYS: // log this command regardless of global log setting
					bLogInputToSystemLog=true;
					bLogToWiznetSecure=true;
					break;
					
				case LOG_PALWAYS:
					if(!IS_NPC(ch)){
						bLogInputToSystemLog=true; // always log players using the particular command
						// log mobiles using it only if global logging is on
						bLogToWiznetSecure=true;
					}
					break;
					
				default: break; // use the global log setting
				}
				// do the actual logging if required
				if (bLogInputToSystemLog){
					log_string( log_buf );
				}
				if(bLogToWiznetSecure){
					wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
				}
			}
    
            // if the player is logged, log them regardless of the log settings
			if (!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG))
            {
                sprintf( log_buf, "PlayerLog %s: %s", ch->name, logline );
                wiznet(log_buf,ch,NULL,WIZ_PLAYER_LOG,0,get_trust(ch));
                append_playerlog( ch, logline);
            }
        }
    }

    if ( !found ){
		bool procured = false;

        // look for command in the socials table.
        if ( check_social( ch, command, argument, false ) ){
	        procured=true;
        }

		// look for the command as a language
		if(language_dynamic_command(ch, command, argument)){
			procured=true;
		}

		if ( !procured ) {
			do_huh(ch, command);
			if (ch->level<2){
		        append_datetime_ch_to_file( ch, NEWBIE_HUH_FILE, argall);
			}
		}
        return;
    }
    else if (check_disabled (ch, &cmd_table[cmd])) {
			// a normal valid command.. check if it is globally disabled 
               ch->printlnf("The '%s' command has been temporarily disabled.", cmd_table[cmd].name);
               return;
	}

    // Character not in position for command?
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    ch->println("Lie still; you are DEAD.");
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    ch->println("You are hurt far too bad for that.");
	    break;

	case POS_STUNNED:
	    ch->println("You are too stunned to do that.");
	    break;

	case POS_SLEEPING:
		if(IS_NPC(ch)){
			ch->printlnf( "In your dreams, or what? (you can't '%s' while sleeping)", 
				command);
		}else{
		    ch->println("In your dreams, or what?");
		}
		if (IS_NEWBIE(ch))
			ch->println("(try waking up first - type wake)");
	    break;

	case POS_RESTING:
	    ch->println("Nah... You feel too relaxed...");
		if (IS_NEWBIE(ch))
			ch->println("(try standing up first - type stand)");
	    break;

	case POS_SITTING:
		 ch->println("Better stand up first.");
	    break;

	case POS_KNEELING:
		 ch->println("Better get off your knees and stand up first.");
		if (IS_NEWBIE(ch))
			ch->println("(try standing up first - type stand)");
	    break;

	case POS_FIGHTING:
	    ch->println("No way!  You are still fighting!");
	    break;

	}
	return;
    }


	// Record the command 
    sprintf (last_command, "[%5d] %s in [%5d] %s: %s",
        IS_NPC(ch) ? ch->pIndexData->vnum : 0,
        IS_NPC(ch) ? ch->short_descr : ch->name,
        ch->in_room ? ch->in_room->vnum : 0,
        ch->in_room ? ch->in_room->name : "(not in a room)",
        logline);

	// Record the moblog command
	if( IS_NPC(ch) && IS_SET(ch->act,ACT_MOBLOG) ){
		append_timestring_to_file( MOBLOG_LOGFILE, 
			FORMATF("[%5d] '%s' in room %d begin command '%s'", 
			(ch->pIndexData?ch->pIndexData->vnum:0),
			ch->name,
			(ch->in_room? ch->in_room->vnum:0),
			logline));
	}

    // Dispatch the command.
    (*cmd_table[cmd].do_fun) ( ch, argument );

    // wiznet memcheck
    if (string_count < nAllocString)
    {
        sprintf(buf,
        "Memcheck : Increase in strings :: %s : %s", ch->name, cmd_copy) ;
        wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,
            get_trust(ch)==MAX_LEVEL?MAX_LEVEL:0);
    }
    if (perm_count < nAllocPerm)
    {
        sprintf(buf,
        "Increase in perms :: %s : %s", ch->name, cmd_copy) ;
        wiznet(buf, NULL, NULL, WIZ_MEMCHECK, 0,
            get_trust(ch)==MAX_LEVEL?MAX_LEVEL:0);
    }


    // Record that the command was the last done, but it is finished 
    sprintf (last_command, "(Finished) [%5d] %s in [%5d] %s: %s",
        IS_NPC(ch) ? ch->pIndexData->vnum : 0,
        IS_NPC(ch) ? ch->short_descr : ch->name,
        ch->in_room ? ch->in_room->vnum : 0,
        ch->in_room ? ch->in_room->name : "(not in a room)",
        logline);

    tail_chain( );
    return;
}


/**************************************************************************/
// allow beckon to work with just bec - instead of becomeactive
void do_bec(char_data *ch, char *argument)
{
	if(!check_social( ch, "bec", argument, false)){
		ch->println("To use becomeactive, type more than the first 3 characters of the command.");
	};
}

/**************************************************************************/
// Return true if an argument is completely numeric.
bool is_number( const char *arg )
{
	if ( *arg == '\0' )
		return false;
	 
	if ( *arg == '+' || *arg == '-' )
		arg++;
	 
	for ( ; *arg != '\0'; arg++ )
	{
		if ( !is_digit( *arg ) )
			return false;
	}
	 
	return true;
}

/**************************************************************************/
// Given a string like 14.foo, return 14 and 'foo'
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '.' )
        {
            *pdot = '\0';
			if(!is_number(ltrim_string(argument))){
				*pdot = '.';
				strcpy( arg, argument );
				return 1;
			}
            number = atoi( argument );
             *pdot = '.';
            strcpy( arg, pdot+1 );
            return number;
        }
    }

    strcpy( arg, argument );
    return 1;
}

/**************************************************************************/
// Given a string like 14*foo, return 14 and 'foo'
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
				*pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }
 
    strcpy( arg, argument );
    return 1;
}

/**************************************************************************/
// Pick off one argument from a string and return the rest - Understands quotes.
char *one_argument( const char *argument, char *arg_first )
{
    char cEnd;

    while ( is_space(*argument) )
	argument++;

    cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' ){
		cEnd = *argument++;
	}

    while ( *argument != '\0' )
    {
        if( (is_space(cEnd) && is_space(*argument)) || *argument== cEnd ){
            argument++;
            break;
        }
        *arg_first = LOWER(*argument);
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( is_space(*argument) ){
		argument++;
	}

    return (char *)argument;
}

/**************************************************************************/
void do_wizhelp( char_data *ch, char *argument )
{
	int cmd;
	int col = 0;
	int level = 0;
	bool fShowAll=true;

	argument=rtrim_string(argument);
	
	if(!IS_NULLSTR(argument))
	{
		if ( is_number( argument )){
			level = atoi( argument );
		}else{
			fShowAll=false;
		}
	}
	
	if ( level < LEVEL_IMMORTAL ){
		level = 0;
	}

	for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	{
		int clevel=cmd_table[cmd].level;
		if(AML==ML && cmd_table[cmd].level==ML+1){
			clevel=ML;
		}

		if(!fShowAll){
			if(str_infix(argument,cmd_table[cmd].name))
				continue;
		}
		
		if ( cmd_table[cmd].level >= LEVEL_HERO
			&&   clevel <= get_trust( ch ) 
			&&   cmd_table[cmd].show)
		{
			if ( !level ){
				ch->printf("%-18s[%3d]   ", cmd_table[cmd].name, clevel );
				if ( ++col % 3 == 0 ){
					ch->println("");
				}
			}else{
				if ( cmd_table[cmd].level <= get_trust( ch ) 
					&&   clevel == level
					&&   cmd_table[cmd].show )
				{					
					ch->printf( "%-18s[%3d]   ",cmd_table[cmd].name, cmd_table[cmd].level );
					if ( ++col % 3 == 0 ){
						ch->println("");
					}
				}
			}
		}
    }
	
    if ( col % 6 != 0 ){
		ch->println("");
	}
	ch->println("Type '`=Cwiz <level>`x' to filter by level, '`=Cwiz <partofword>`x' to filter on words.");
	return;
}
/**************************************************************************/
void do_wizgrantlist( char_data *ch, char *argument )
{
	int cmd;
	int level = 0;
	bool fShowAll=true;

	argument=rtrim_string(argument);
	
	if(!IS_NULLSTR(argument))
	{
		if ( is_number( argument )){
			level = atoi( argument );
		}else{
			fShowAll=false;
		}
	}
	
	if ( level < LEVEL_IMMORTAL )
		level = 0;
	
	for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	{
		if(!fShowAll){
			if(str_infix(argument,cmd_table[cmd].name))
				continue;
		}
		
		if ( cmd_table[cmd].level >= LEVEL_HERO
			&&   cmd_table[cmd].level <= get_trust( ch ) 
			&&   cmd_table[cmd].show)
		{
			if ( !level )
			{
				ch->printlnf("%s[%3d] %s", 
					mxp_create_send(ch,FORMATF("comedit %s", cmd_table[cmd].name), FORMATF("%-25s", cmd_table[cmd].name)),
					cmd_table[cmd].level,
					flag_string(grantgroup_flags, cmd_table[cmd].grantgroups));
			}
			else
				if ( cmd_table[cmd].level <= get_trust( ch ) 
					&&   cmd_table[cmd].level == level
					&&   cmd_table[cmd].show )
				{
					ch->printlnf("%s[%3d] %s", 
						mxp_create_send(ch,FORMATF("comedit %s", cmd_table[cmd].name), FORMATF("%-25s", cmd_table[cmd].name)),
						cmd_table[cmd].level,
						flag_string(grantgroup_flags, cmd_table[cmd].grantgroups));
				}
		}
    }

	ch->println("Type '`=Cwizgrant <level>`x' to filter by level, '`=Cwizgrant <partofword>`x' to filter on words.");
	ch->println("PLEASE NOTE: The grant system is incomplete.");
	return;
}

/**************************************************************************/
/* Syntax is:
disable - shows disabled commands
disable <command> - toggles disable status of command
*/
void do_disable (char_data *ch, char *argument)
{
        int i;
        DISABLED_DATA *p,*q;
        
        if (IS_NPC(ch))
        {
                ch->println("RETURN first.");
                return;
        }
      
        if (IS_NULLSTR(argument)) // Nothing specified. Show disabled commands.
        {
                if (!disabled_first) // Any disabled at all ? 
                {
                        ch->println("There are no commands disabled.");
                        return;
                }

                ch->println("Disabled commands:");
				ch->println("Command      Level   Disabled by    For");
                                
                for (p = disabled_first; p; p = p->next)
                {
                        ch->printlnf("%-12s %5d   %-12s   %-12s",
							p->command->name, p->level, p->disabled_by, p->disabled_for);
                        
                }
                return;
        }
        
        // command given 

        // First check if it is one of the 'All' disabled commands 
        for (p = disabled_first; p ; p = p->next)
                if (!str_cmp(argument, p->command->name)&&
                    !str_cmp("All", p->disabled_for))
                        break;
                        
        if (p) // this command is disabled 
        {
			// Optional: The level of the imm to enable the command must 
			//           equal or exceed level of the one that disabled it
			if (get_trust(ch) < p->level){
				ch->println("This command was disabled by a higher power.");
				return;
			}
			
			// Remove                 
			if (disabled_first == p){ // node to be removed == head ?
				disabled_first = p->next;
			}else{ // Find the node before this one 
				for (q = disabled_first; q->next != p; q = q->next){
					// empty for loop in order to find the end of the list
				}
				q->next = p->next;
			}
			
			free_string (p->disabled_by); // free name of disabler
			free_string (p->disabled_for); // free name who for
			free_mem (p,sizeof(DISABLED_DATA)); // free node
			save_disabled(); // save to disk 
			ch->printlnf("Command '%s' `Benabled`x for All.",argument);
        }
        else // not a disabled command, check if that command exists
        {
                // IQ test
                if (!str_cmp(argument,"disable"))
                {
                        ch->println("You cannot disable the disable command.");
                        return;
                }

                // Search for the command 
                for (i = 0; cmd_table[i].name[0] != '\0'; i++)
                        if (!str_cmp(cmd_table[i].name, argument))
                                break;

                // Found?
                if (cmd_table[i].name[0] == '\0')
                {
                        ch->println("No such command.");
                        return;
                }

                // Can the imm use this command at all ?
                if (cmd_table[i].level > get_trust(ch)){
                        ch->println("You don't have access to that command; you cannot disable it.");
                        return;
                }
                
                // Disable the command
                
                p = (DISABLED_DATA *)alloc_mem (sizeof(DISABLED_DATA));

                p->command = &cmd_table[i];
                p->disabled_by = str_dup (ch->name); // save name of disabler
                p->level = get_trust(ch); // save trust
                p->disabled_for = str_dup ("All"); // disable for everyone
                p->next = disabled_first;
                disabled_first = p; // add before the current first element
                
                ch->printlnf("Command '%s' `RDISABLED`x for All.",argument);
                save_disabled(); // save to disk
        }
}

/**************************************************************************/
/* Syntax is:
pdisable - shows personally disabled commands
pdisable <command> <name|All> - toggles disable status of command
*/
bool	check_parse_name	args( ( char *name ) );

void do_pdisable (char_data *ch, char *argument)
{
        char argname[MIL];
        char argcommand[MIL];
        int i;
        DISABLED_DATA *p;
        char buf[100];
       
        if (IS_NPC(ch))
        {
                ch->println("RETURN first.");
                return;
        }
        
        if (!argument[0]) // Nothing specified. Show disabled commands. 
        {
                ch->println("Syntax: pdisable command name.");

                if (!disabled_first){ // Any disabled at all ?
                        ch->println("There are no commands disabled.");
                        return;
                }

                ch->println("Pdisabled commands:");
				ch->println("Command      Level   Disabled by    For");
                                
                for (p = disabled_first; p; p = p->next)
                {
                        ch->printlnf("%-12s %5d   %-12s   %-12s",
							p->command->name, p->level, p->disabled_by, p->disabled_for);
                }
                ch->println("syntax: pdisable <command> <name|All>");
                return;
        }

        // split command into name and command 
        argument = one_argument( argument, argcommand );
        one_argument( argument, buf );
        strcpy(argname,capitalize(buf));

		if(IS_NULLSTR(argname)){
			ch->printlnf("You must specify a name for who the command '%s' is personally disabled.",
				argcommand);
			return;
		}

        if (!check_parse_name(argname))
        {
            ch->printf("The name you specified '%s' may not be a legal name!\n", argname);
        }
        
        // command given

        // First check if it is one of the disabled commands for a person
        for(p = disabled_first; p ; p = p->next){
                if (!str_cmp(argcommand, p->command->name) &&
                    !str_cmp(argname, p->disabled_for))
                        break;
		}
                        
		if(p){ // this command is already disabled
                ch->println("Use penable to enable the command.");
                return;
        }else{ // not a disabled command, check if that command exists
                // IQ tests 
                if (!str_cmp(argcommand,"disable"))
                {
                        ch->println("You cannot disable the disable command.");
                        return;
                }
                if (!str_cmp(argcommand,"pdisable"))
                {
                        ch->println("You cannot disable the pdisable command.");
                        return;
                }
                if (!str_cmp(argcommand,"penable"))
                {
                        ch->println("You cannot disable the penable command.");
                        return;
                }

                // Search for the command 
                for (i = 0; cmd_table[i].name[0] != '\0'; i++){
					if (!str_cmp(cmd_table[i].name, argcommand)){
						break;
					}
				}

                // Found?
                if (cmd_table[i].name[0] == '\0'){
                        ch->printlnf("No such command '%s'.", argcommand);
                        return;
                }

                // Can the imm use this command at all ?
                if (cmd_table[i].level > get_trust(ch)){
                        ch->println("You don't have access to that command; you cannot disable it.");
                        return;
                }
                
                // Disable the command              
                p = (DISABLED_DATA *)alloc_mem (sizeof(DISABLED_DATA));

                p->command = &cmd_table[i];
                p->disabled_by = str_dup(ch->name); // save name of disabler 
                p->level = ADMIN; // disable the command at admin level 
                p->disabled_for = str_dup(argname); // disable for person 
                p->next = disabled_first;
                disabled_first = p; // add before the current first element 
                
                ch->printlnf("Command '%s' disabled for '%s'.", argcommand, argname);
                save_disabled(); // save to disk 
        }
}

/**************************************************************************/
/* Syntax is:
penable - shows personally disabled commands
penable <command> <name|All> - toggles disable status of command
*/
void do_penable (char_data *ch, char *argument)
{
        char argname[MIL];
        char argcommand[MIL];
        DISABLED_DATA *p,*q;
        char buf[100];
       
        if (IS_NPC(ch))
        {
                ch->println("RETURN first.");
                return;
        }
        
        if (!argument[0]) /* Nothing specified. Show disabled commands. */
        {
                ch->println("Syntax: penable command name.");

                if (!disabled_first) /* Any disabled at all ? */
                {
                        ch->println("There are no commands disabled.");
                        return;
                }

                ch->println("Pdisabled commands:");
				ch->println("Command      Level   Disabled by    For");
                                
                for (p = disabled_first; p; p = p->next)
                {
                        ch->printlnf("%-12s %5d   %-12s   %-12s",
							p->command->name, p->level, p->disabled_by, p->disabled_for);
                }
                return;
        }

        /* split command into name and command */
        argument = one_argument( argument, argcommand );
        one_argument( argument, buf );
        strcpy(argname, capitalize(buf));

        /* command given */

        /* First check if it is one of the disabled commands for a person*/
        for (p = disabled_first; p ; p = p->next)
                if (!str_cmp(argcommand, p->command->name) &&
                    !str_cmp(argname, p->disabled_for))
                        break;
                        
        if (p) /* this command is disabled */
        {
        /* Optional: The level of the imm to enable the command must equal or exceed level
           of the one that disabled it */
        
                if (get_trust(ch) < p->level)
                {
                        ch->println("This command was disabled by a higher power.");
                        return;
                }
                
                /* Remove */
                
                if (disabled_first == p) /* node to be removed == head ? */
                        disabled_first = p->next;
                else /* Find the node before this one */
                {
                        for (q = disabled_first; q->next != p; q = q->next); /* empty for */
                        q->next = p->next;
                }
                
                free_string (p->disabled_by); /* free name of disabler */
                free_string (p->disabled_for); /* free name who for */
                free_mem (p,sizeof(DISABLED_DATA)); /* free node */
                save_disabled(); /* save to disk */
                ch->println("Command enabled.");
        }
        else /* not a disabled command, check if that command exists */
        {
                ch->println("Use pdisable to disable that command.");
                return;
        }
}

/**************************************************************************/
/* Check if that command is disabled 
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/   
bool check_disabled (char_data *ch, const struct cmd_type *command)
{
        DISABLED_DATA *p;
      
        for (p = disabled_first; p ; p = p->next)
                if ((p->command->do_fun == command->do_fun) &&
                    (!strcmp(ch->name, p->disabled_for)  ||
                     !strcmp("All", p->disabled_for) ))
                        return true;

        return false;
}

/**************************************************************************/
/* Load disabled commands */
void load_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	char *name;
	int i;
	
	disabled_first = NULL;

	logf("Loading disabled commands from %s...", DISABLED_FILE);
	
	fp = fopen (DISABLED_FILE, "r");
	
	if (!fp) // No disabled file.. no disabled commands:
		return;
	
	name = fread_word (fp);
	
	while (str_cmp(name, END_MARKER)) // as long as name is NOT END_MARKER :)
	{
		// Find the command in the table 
		for (i = 0; cmd_table[i].name[0] ; i++)
			if (!str_cmp(cmd_table[i].name, name))
				break;
			
			if (!cmd_table[i].name[0]) // command does not exist?
			{
				bugf(" Skipping unknown command (%s) in '%s' file.", name, DISABLED_FILE);
				fread_number(fp); // level 
				fread_word(fp); // disabled_by
				fread_word(fp); // disabled_for 
			}
			else // add new disabled command 
			{
				p = (DISABLED_DATA *)alloc_mem(sizeof(DISABLED_DATA));
				p->command = &cmd_table[i];
				p->level = fread_number(fp);
				p->disabled_by = str_dup(fread_word(fp)); 
				p->disabled_for = str_dup(fread_word(fp)); 
				p->next = disabled_first;
				
				disabled_first = p;
				
			}
			
			name = fread_word(fp);
	}
	
	fclose (fp);            
}

/**************************************************************************/
// Save disabled commands 
void save_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;

	if (!disabled_first) /* delete file if no commands are disabled */
	{
		unlink (DISABLED_FILE);
		return;
	}

	fp = fopen (DISABLED_FILE, "w");
        
	if (!fp)
	{
		bugf("Could not open '%s' for writing", DISABLED_FILE);
		return;
	}

	for (p = disabled_first; p ; p = p->next)
		fprintf (fp, "%s %d %s %s\n",
			p->command->name, 
			p->level,
			p->disabled_by, 
			p->disabled_for);

	fprintf (fp, "%s\n",END_MARKER);

	fclose (fp);
}

/**************************************************************************/
// By Kerenos and Kalahn
void do_commands( char_data *ch, char *argument )
{
	int i, j, col=0;
	int maxcat, mincat;
	int cat_index=-1;
	char buf[MIL];    
	bool fShowAll=true;
	bool infix_searching=false;
	bool empty_section=false;

	argument=rtrim_string(argument);
	
	// find out if we are filtering by a category only, infix searching, or showing all
	if( !IS_NULLSTR(argument))
	{
		if (( cat_index= com_category_lookup( argument )) != -1 ){
			fShowAll=false;
		}else{
			infix_searching=true;
		}
	}

	// find the maximum category
	maxcat=0;
	mincat=200;
	
	for ( i = 0; !IS_NULLSTR(cmd_table[i].name); i++ )
	{
		maxcat=UMAX(maxcat, cmd_table[i].category);
		mincat=UMIN(mincat, cmd_table[i].category);
	}
	
	// output 
	BUFFER *output= new_buf(); 
	BUFFER *section_buffer= new_buf(); 
	add_buf(output,"`?`#"); // get the random colour for title bars
	if ( fShowAll )
	{
		sprintf( buf,"`^%s`x", format_titlebar("CATEGORIES"));
		add_buf(output,buf);
	}

	int last_col=1;
	for (j=mincat;j<=maxcat;j++)
	{
		if ( fShowAll ){
			add_buf( section_buffer,"\r\n");
		}
		col=0;


		if (!IS_RP_SUPPORT(ch) && j == COMCAT_RPSUPPORT)
				continue;
		if (!IS_NEWBIE_SUPPORT(ch) && j == COMCAT_NSUPPORT)
				continue;
		if (!IS_NOBLE(ch) && j == COMCAT_NOBLE)
				continue;
		if (!HAS_SECURITY(ch,1) && j == COMCAT_OLC)
				continue;

		if ( fShowAll ){
			sprintf( buf,"`^%s`x", format_titlebar(com_category_indexlookup(j)));
			add_buf(section_buffer,buf);
			empty_section=infix_searching;	
		}else{ // we are doing a category only filter
			if ( j!=cat_index){
				continue;
			}
			sprintf( buf,"`^%s`x", format_titlebar(com_category_indexlookup(j)));
			add_buf(section_buffer,buf);
			empty_section=false;
		}

		for(i=0; !IS_NULLSTR(cmd_table[i].name); i++)
		{
			if (cmd_table[i].category!=j)
			{
				continue;
			}

			if(!fShowAll)
			{
				if( cmd_table[i].category != cat_index)
					continue;
			}

			if(infix_searching){
				if(str_infix(argument,cmd_table[i].name)){
					continue;
				}
			}

			if ( cmd_table[i].level <  LEVEL_HERO
				&&   cmd_table[i].level <= get_trust( ch )
				&&   cmd_table[i].show )
			{
				strcpy(buf, str_width(cmd_table[i].name, 15,true));
				add_buf( section_buffer, buf);
				empty_section=false;
				if (++col%5==0){
					add_buf( section_buffer,"\r\n");
				}
			}
		}

		if(!empty_section && !(j==CAT_UNDEFINED && col==0) ){ 
			// only show sections with content when infix searching 
			// also: if listing the whole lot and there is nothing in the 
			// undefined category, don't show the header for it.
			add_buf( output,buf_string(section_buffer));
			last_col=col;
		}

		// reset the buffer
		free_buf(section_buffer);
		section_buffer= new_buf(); 
	}

	// add the blank bar at the bottom
	if (last_col%5!=0){
		add_buf( output,"\r\n");
	}
	sprintf( buf,"`^%s`x", format_titlebar("-"));
	add_buf( output,buf);

	// show them the list
	ch->sendpage(buf_string(output));
	free_buf(output);
	free_buf(section_buffer);
}

/**************************************************************************/
// returns -1 if the category can't be found
int com_category_lookup(char *name)
{
	int flag;

	// first attempt exact match 
	for ( flag = 0; com_category_types[flag].name!= NULL; flag++)
	{
		if (LOWER(name[0]) == LOWER(com_category_types[flag].name[0])
			&&  !str_cmp( name, com_category_types[flag].name))
            return com_category_types[flag].bit;
	}
	
	// now attempt a prefix match
	for ( flag = 0; com_category_types[flag].name!= NULL; flag++)
	{
		if (LOWER(name[0]) == LOWER(com_category_types[flag].name[0])
			&&  !str_prefix( name, com_category_types[flag].name))
			return com_category_types[flag].bit;
	}
	
//	bugf("com_category_lookup(): Invalid category name '%s'!", name);
	return -1;
};

/**************************************************************************/
char *com_category_indexlookup(int index)
{
	int flag;

	if(com_category_types[index].bit==index)
	{
		return (com_category_types[index].name);
	}

	for (flag = 0; com_category_types[flag].name; flag++)
	{
		if(com_category_types[flag].bit==index)
		{
			return (com_category_types[flag].name);
		}
	}

//	bugf("com_category_indexlookup(): Invalid index %d!", index);
	return ("");
	
};
/**************************************************************************/
