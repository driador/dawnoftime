/**************************************************************************/
// global.h - Global variable system details see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: global.h - To add a global just add a SINGLE entry into 
 *        global.h prefixed with the word EXTERN (all uppercase)... 
 *        compiler macros do the rest.
 ***************************************************************************/
#ifndef GLOBAL_H
#define GLOBAL_H

EXTERN  MOOT_DATA *moot;
EXTERN  char EXE_FILE[30];
EXTERN  char MACHINE_NAME[MSL];
EXTERN  char PLATFORM_INFO[2048];

// for do_count - act_info.c 
EXTERN  int max_on;
EXTERN  int true_count;
EXTERN  time_t          maxon_time;				// Time of master maxon
EXTERN  int				hotrebootmaxon;         // maxon since last hotreboot
EXTERN  time_t          hotrebootmaxon_time;    // maxon since last hotreboot 
                                                // was at what time

EXTERN  connection_data *   connection_free;	// Free list for connections
EXTERN  connection_data *   connection_list;    // All open connections
EXTERN  connection_data *   c_next; // the next connection used in a number of loops

EXTERN  char_data *			player_list;
EXTERN  race_data *			race_list;

// Reserved file handles
// - Secondary is reserved so we can do 2 lots of file io - in handler.c
EXTERN  FILE *          fpReserve;
EXTERN  FILE *          fpAppend2FilReserve;
EXTERN  FILE *          fpReserveFileExists;

EXTERN  RUNLEVEL_TYPE	runlevel;				// Used to mark bootup, mainloop and shutdown
EXTERN  char            str_boot_time[MIL];
EXTERN  time_t          boot_time;              // Time we last hotrebooted
EXTERN  time_t          lastreboot_time;         // Time we started up 
EXTERN  time_t          current_time;           // time of this pulse
EXTERN  long            tick_counter;           // counter of the ticks
EXTERN  int             note_notify_counter;    // notification of new mail
EXTERN  char            shutdown_filename[MSL];
EXTERN  bool            MOBtrigger;             // act() switch
EXTERN  int				mainport;               // the mainport value specified on startup
EXTERN  int				parsed_mainport;               // the mainport value specified on startup

EXTERN  int				resolver_stdinout;			// Pipe to hostname lookup resolver stdin/stdout
EXTERN  int				resolver_stderr;		// Pipe to hostname lookup resolver stderr
EXTERN  int				resolver_version;		// resolver version, * 1000
EXTERN  int				resolver_running;

EXTERN  bool            fBootTestOnly;	// used to check if the mud will bootup

EXTERN	DEITY_DATA	*	deity_first;
EXTERN	DEITY_DATA	*	deity_last;

// area related
EXTERN  AREA_DATA *     area_first;
EXTERN  AREA_DATA *     area_vnumsort_first;
EXTERN  AREA_DATA *     area_levelsort_first;
EXTERN  AREA_DATA *     area_arealist_first;
EXTERN  AREA_DATA *     area_last;
EXTERN  SHOP_DATA *     shop_last;
EXTERN	cInnData*		pLastInn;
EXTERN  int	resaveCounter;

// moons code - affects casting level of mages
EXTERN  int		moon_day;	// 1->28
EXTERN  int     moon_month;	// 1-12
EXTERN  int     moon_cast_modifier;

// MPINFO stuff - program_flow() callstack stuff
EXTERN  vn_int callstack_pvnum[MAX_CALL_LEVEL];
EXTERN  vn_int callstack_mvnum[MAX_CALL_LEVEL];
EXTERN  vn_int callstack_rvnum[MAX_CALL_LEVEL];
EXTERN  vn_int callstack_line[MAX_CALL_LEVEL];
EXTERN  bool callstack_aborted[MAX_CALL_LEVEL];
EXTERN  int call_level; // Keep track of nested "mpcall"s
EXTERN  bool mudprog_preventtrain_used;
EXTERN  bool mudprog_preventprac_used;
EXTERN  bool mudprog_bribe_silver_in_use;
EXTERN  int mudprog_bribe_amount_in_use;
EXTERN  char_data *mudprog_bribe_money_from;

EXTERN  sh_int		top_helpfile;

// WEBSERVER related
//EXTERN  bool webRunning;
EXTERN  int  webHits;
EXTERN  int  webHelpHits;
EXTERN  int  webWhoHits;

// DEFAULT CHARACTER TEMPLATES
EXTERN  char_data * chImmortal;

// mudftp code
EXTERN  int ftp_control;

// pkill port details
EXTERN  int p9999maxpk, p9999maxpklevel;
EXTERN  char p9999maxpkname[50];
EXTERN  int p9999maxpd, p9999maxpdlevel;
EXTERN  char p9999maxpdname[50];

// GSN numbers - being moved into here
#include "gsn.h"

// races stuff
EXTERN  int total_npcracescount;
EXTERN  int total_npcareacount;

// log memory
EXTERN  bool log_memory;
EXTERN  int free_mem_count;
EXTERN  int alloc_mem_count;

// linked list of all pMobIndex records
EXTERN  MOB_INDEX_DATA *pMobIndexlist;

// disabled commands system
EXTERN  DISABLED_DATA *disabled_first; 

// new dynamic skill system
EXTERN  sh_int	FIRST_SPELL;
EXTERN  sh_int	LAST_SPELL;
EXTERN  sh_int	SKILL_TABLE_FLAGS;
EXTERN  sh_int	TOP_SKILL;

// supports letgain system
EXTERN  letgain_data *letgain_list;

// dyn command related
EXTERN	sh_int	COM_TABLE_FLAGS;
EXTERN	sh_int	DEITY_FLAGS;
EXTERN	sh_int	HERB_FLAGS;
EXTERN	sh_int	MIX_FLAGS;
EXTERN	sh_int	CLAN_FLAGS;
EXTERN	sh_int	SKILLGROUPEDIT_FLAGS;

// olc based resave flags
EXTERN	bool	LANGUAGE_NEEDS_SAVING;

// classedit related globals
EXTERN  sh_int	CLASS_TABLE_FLAGS;

// DEBUG system - COMMANDS IN DEBUG TO ALLOW YOU TO SET THE ROOM, OBJ, MOB
//                THEN USE 'MAKECOREFILE' TO HAVE A LOOKY AT WHAT YOU HAVE SET
EXTERN	ROOM_INDEX_DATA *DEBUG_ROOM;
EXTERN	char_data		*DEBUG_MOB;
EXTERN	OBJ_DATA		*DEBUG_OBJECT;

EXTERN	vn_int			DEBUG_LAST_NON_EXISTING_REQUESTED_ROOM_VNUM;
EXTERN	vn_int			DEBUG_LAST_NON_EXISTING_REQUESTED_OBJECT_VNUM;
EXTERN	vn_int			DEBUG_LAST_NON_EXISTING_REQUESTED_MOBILE_VNUM;

// check_immtalk system
EXTERN	char check_immtalk_replay_text[MAX_CHECK_IMMTALK][MIL+30];
EXTERN	int check_immtalk_replay_index;

// use command tail to record the last commands leading up to a crash 
// - olc or interp based
EXTERN char inputtail[MAX_INPUTTAIL][MIL+250]; // 250 bytes for details
EXTERN int inputtail_index;

EXTERN char temp_HSL_workspace[HSL];
EXTERN int mudprog_count;

EXTERN mob_index_data *limbo_mob_index_data; // used when we need a temp mob template 

// Array of containers read for proper re-nesting of objects.
EXTERN OBJ_DATA *   rgObjNest[MAX_NEST];

// raceedit stuff
EXTERN  sh_int	RACEEDIT_FLAGS;
EXTERN  class race_data** race_table; // dynamically allocated

EXTERN  bool EXECUTING_SOCIAL;
EXTERN  bool RECORD_TO_REPLAYROOM;

// channeloff macros use the following variable to avoid following a null pointer
EXTERN  long __CHANNEL_OFF_CRASH_PROTECTOR_VARIABLE;


EXTERN  continent_type *continent_list;

EXTERN  int lockers_total_count;
EXTERN  int lockers_object_count;

// system languages
EXTERN  language_data *language_unknown;
EXTERN  language_data *language_native;
EXTERN  language_data *language_alwaysunderstood;
EXTERN  language_data *language_reverse;

#endif // GLOBAL_H

