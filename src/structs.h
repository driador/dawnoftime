/**************************************************************************/
// structs.h - Majority of data structures used in game
/***************************************************************************/
#include "version.h" // gets MFCODE
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
/**************************************************************************/

// Accommodate old non-Ansi compilers.
#ifndef __cplusplus
	#define __cplusplus 1
#endif

#ifdef unix
// function type check
#define __ftc_printf_1__	__attribute__ ((format(printf,1,2)))
// The this pointer in a member function counts as the 
// first argument for gcc's __attribute__ format usage
// mftc = member function type check - Kal :)
#define __mftc_printf_1__	__attribute__ ((format(printf,2,3)))
#define __mftc_printf_2__	__attribute__ ((format(printf,3,4)))
#define __mftc_printf_3__	__attribute__ ((format(printf,4,5)))
#else
#define __ftc_printf_1__	
#define __mftc_printf_1__
#define __mftc_printf_2__
#define __mftc_printf_3__
#endif
char	*FORMATF(const char *formatbuf, ...) __ftc_printf_1__;

// each spell function returns one of the following
enum SPRESULT {NO_MANA, HALF_MANA, FULL_MANA, DOUBLE_MANA, ALL_MANA};

enum RUNLEVEL_TYPE {RUNLEVEL_SHUTING_DOWN, 
			RUNLEVEL_MAIN_IO_LOOP, RUNLEVEL_BOOTING, RUNLEVEL_INIT};

enum PFILE_TYPE {PFILE_LOCKED,PFILE_NORMAL, PFILE_BUILDER, PFILE_TRUSTED, 
		PFILE_IMMORTAL, PFILE_REMORT_BACKUP, PFILE_NONE, PFILE_MULTIPLE};

enum CLASS_CAST_TYPE {CCT_NONE, CCT_MAGE, CCT_CLERIC, CCT_DRUID,
					  CCT_BARD, CCT_MAXCAST };

enum PREFERENCE_TYPE {PREF_OFF, PREF_AUTOSENSE, PREF_ON};

enum ACTTO_TYPE {TO_ROOM, TO_NOTVICT, TO_VICT, TO_CHAR, TO_ALL, TO_WORLD};
/**************************************************************************/

#if defined(TRADITIONAL) && !defined( __cplusplus)
#define const
#define args( list )                    ( )
#define DECLARE_DO_FUN( fun )           void fun( )
#define DECLARE_SPEC_FUN( fun )         bool fun( )
#define DECLARE_OSPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )		SPRESULT fun( )
#define DECLARE_GAMBLE_FUN( fun )		void fun ( )
#else
#ifdef __cplusplus
#define DECLARE_DO_FUN( fun )           void fun ( char_data *, char *);
#define DECLARE_SPEC_FUN( fun )         bool fun ( char_data * );
#define DECLARE_OSPEC_FUN( fun )		bool fun ( obj_data *, char_data *);
#define DECLARE_SPELL_FUN( fun )        SPRESULT fun ( int, int, char_data *, void *, int );
#define DECLARE_GAMBLE_FUN( fun )		void fun ( char_data *, char_data *, char *);
#define args( list )                    list
#else
#define args( list )                    list
#define DECLARE_DO_FUN( fun )           DO_FUN     fun
#define DECLARE_SPEC_FUN( fun )         SPEC_FUN   fun
#define DECLARE_OSPEC_FUN( fun )		OSPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )        SPELL_FUN  fun
#define DECLARE_GAMBLE_FUN( fun )		GAMBLE_FUN fun
#endif
#endif

#ifdef WIN32
	typedef int socklen_t;
#endif

typedef short   int     sh_int;
typedef int				vn_int;  
// vn_int = vnum integer - makes easier changing vnum range
#ifndef __cplusplus
	typedef unsigned char bool;
#endif
/**************************************************************************/
// forward declarations
class char_data;
class connection_data;

/**************************************************************************/
// all the type defs
typedef struct  area_echo_data			AREA_ECHO_DATA;
typedef struct  affect_data             AFFECT_DATA;
typedef struct  area_data               AREA_DATA;
typedef struct  ban_data                BAN_DATA;
typedef struct  buf_type                BUFFER;
typedef struct  exit_data               EXIT_DATA;
typedef struct  extra_descr_data        EXTRA_DESCR_DATA;
typedef struct  room_echo_data          ROOM_ECHO_DATA;
typedef struct  kill_data               KILL_DATA;
typedef struct  mem_data                MEM_DATA;
typedef struct  mob_index_data          MOB_INDEX_DATA;
typedef struct  note_data               NOTE_DATA;
typedef class	obj_data                OBJ_DATA;
typedef struct  obj_index_data          OBJ_INDEX_DATA;
typedef struct  pc_data                 PC_DATA;
typedef struct  gen_data                GEN_DATA;
typedef struct  reset_data              RESET_DATA;
typedef struct  room_index_data         ROOM_INDEX_DATA;
typedef struct  shop_data               SHOP_DATA;
typedef struct  time_info_data          TIME_INFO_DATA;
typedef struct  weather_data            WEATHER_DATA;
typedef struct  moot_data               MOOT_DATA;
typedef struct  mudprog_trigger_list	MUDPROG_TRIGGER_LIST; // this stores lists of progs on mobs/objects etc
typedef struct  mudprog_code			MUDPROG_CODE; // this stores an individual prog edited with mpedit
typedef struct  disabled_data			DISABLED_DATA;
typedef struct  objrestrict_list_type	OBJRESTRICT_LIST_DATA;
typedef struct	deity_type				DEITY_DATA;
typedef struct	herb_type				HERB_DATA;
typedef struct	mix_type				mix_data;
typedef struct	groupaff_type			groupaff_data;
typedef struct	affliction_type			affliction_data;

// Function types.
typedef void DO_FUN( char_data *ch, char *argument );
typedef bool SPEC_FUN( char_data *ch );
typedef bool OSPEC_FUN( obj_data *obj, char_data *ch );
typedef void GAMBLE_FUN( char_data *ch, char_data *dealer, char *argument );
typedef SPRESULT SPELL_FUN( int sn, int level, char_data *ch, void *vo, int target );
/**************************************************************************/
// MUDFTP enums
typedef enum { FTP_NORMAL, FTP_PUSH, FTP_PUSH_WAIT } ftp_mode;
typedef enum {NDESC_NORMAL, NDESC_FTP } ndesc_t;

/**************************************************************************/
struct continent_type
{
	char *		name;
	continent_type * next;
};
/**************************************************************************/
// bm_webrequest_data 
#include "websrv.h"
/**************************************************************************/
// Descriptor (channel) class.
// bm_connection_data - bookmark for searching 
#include "connect.h"
/**************************************************************************/
class duel_data;
class cInnData;
/**************************************************************************/
#include "entity.h"

/**************************************************************************/
#include "clan.h"
/**************************************************************************/
// bm_pc_data - bookmark for searching 
// Data which only PC's have.
struct  pc_data
{
	pc_data *	next;
	BUFFER *	buffer;
	bool		valid;
	PFILE_TYPE  pfiletype;
	char *		pwd;
	bool 		overwrite_pwd;
	char *		bamfin;
	char *		bamfout;
	char *		fadein;
	char *		fadeout;
	char *		surname;
	char *		birthplace;
	char *		haircolour;
	char *		eyecolour;
	char *		trait[9];
	char *		crest;
	char *		history;
	sh_int		height;
	sh_int		weight;
	time_t		last_note;
	time_t		last_idea;
	time_t		last_penalty;
	time_t		last_news;
	time_t		last_changes;
	time_t		last_anote;
	time_t		last_inote;
	time_t		last_misc;
	time_t		last_snote;
	time_t		last_pknote;
	time_t		birthdate;
	int			birthyear_modifier;
	int			perm_hit;
	int			perm_mana;
	int			perm_move;
	sh_int		true_sex;
	unsigned char security;		// OLC Builder security
	int 		last_level;
	unsigned char condition		[4];
	unsigned char learned		[MAX_SKILL];
	time_t		last_used		[MAX_SKILL]; // last time some skills were used
	sh_int		points;
	bool		skillgroup_known	[MAX_SKILLGROUP];
	bool		confirm_delete;
	bool		is_trying_aware;
	char *		alias[MAX_ALIAS];
	char *		alias_sub[MAX_ALIAS];
	sh_int		tired;
	int 		xp_penalty;
	sh_int		sublevel, sublevel_trains, sublevel_pracs;
	unsigned char olc_tab;

	// nobles 
	sh_int			diplomacy;
	int 			dip_points;
	sh_int			autovote;
	int 			in_favor;  // -1 opposed, 0 abstained, 1 in favor
	int 			votes_cast; // number of votes cast
	int 			rp_count;
	int 			voteupdate_count;	// used to up the number of moots a noble has
	int				heroxp;	
	// automated rps system 
	bool		did_ooc;
	bool		did_ic;
	sh_int		merit;
	sh_int		panic; 

	// quest points system
	int			qpoints;
	
	// rps auditing system
	sh_int		emote_index; 
	char *		emotecheck[RPS_AUDIT_SIZE];
	time_t		emotetimes[RPS_AUDIT_SIZE];
	sh_int		say_index; 
	char *		saycheck[RPS_AUDIT_SIZE];
	time_t		saytimes[RPS_AUDIT_SIZE];
	
	// magic system
	long		realms;
	long		spheres;
	long		elements;
	long		compositions;

	long		rp_points;
	
	// count the number of times they reroll
	sh_int		reroll_counter; 
	// karns system 
	sh_int		karns;
	sh_int		next_karn_countdown;
	// lay on hands skill
	sh_int		lays;
	sh_int		next_lay_countdown;
	// vanish skill
	time_t		next_vanish;

	// worship time for deities
	time_t		worship_time;
	
	// lag for those that spam the who command
	time_t		last_who;

	// for email banning verification
	char *		email;
	char *		created_from; // isp they created from
	char *		unlock_id;
	char *		webpage;
	char *      webpass;	// password for accessing the webpage

	// last login site
	char *		last_logout_site;
	time_t 		last_logout_time;

	// mob kill counters
	int			mkills;
	int			mdefeats;

    // reduced system to slow down the power levelers
    sh_int      reduce_xp_percent;
    sh_int      reduce_rps_percent;

	// object restriction system
	int objrestrict; // bitmask of all groups that the char matches with

	// pkill port stuff (traditionally port 9999)
	sh_int	p9999kills, p9999defeats;

	// charnotes 
	char *		charnotes;

	// Council
	long		council;


	// who related 
	char *		who_text;
	char *		afk_message;
	time_t		unsafe_due_to_stealing_till;
	char *		title;
	char *		immtitle;
	sh_int		who_format;

	// replay tell buffers
	unsigned int next_replaytell;
	char *replaytell_text[MAX_REPLAYTELL];

	// replay room buffers
	unsigned int next_replayroom;
	char *replayroom_text[MAX_REPLAYROOM];
	vn_int replayroom_lastevent_roomvnum;

	// replay channels buffers
	unsigned int next_replaychannels;
	char *replaychannels_text[MAX_REPLAYCHANNELS];
	
	char *immtalk_name; // for morts with immtalk
	char *imm_role;		// role text in an imms score

	char *letter_workspace_text; // The text of a letter in progress

	// colour related code
	COLOUR_TEMPLATE_TYPE *custom_colour_scheme;
	char *custom_colour;	// colour profile loading/saving location
	COLOUR_TYPE colourmode; // colour mode loading/saving location
	bool flashing_disabled; // colour mode loading/saving location
	int strip_colour_on_channel;
	
	// channeloff flags
	long		channeloff;	
	
	PREFERENCE_TYPE preference_msp;
	PREFERENCE_TYPE preference_mxp;	
	PREFERENCE_TYPE preference_dawnftp;
	PREFERENCE_TYPE preference_colour_in_socials;

	bool msp_enabled;
	bool mxp_enabled;

	// last x note post times
	time_t note_post_time[MAX_NOTE_POST_TIME_INDEX]; 

	char *help_history[MAX_HELP_HISTORY];
	unsigned char help_history_index;	// points to the location of a text copy of the last displayed help entry
	unsigned char help_next_count;		// records how many forward we can go before looping
	char colour_code;					// colour code used for text saved within this pfile

	char *battlelag;

	long		pconfig;	// config flags stored on pcdata


	time_t thief_until; // time until they are considered a thief
	time_t killer_until; // time until they are considered a killer

	sh_int mpedit_autoindent;
	sh_int hero_level_count;
	sh_int autoafkafter;
	sh_int		old_clss;	// old class
};

/**************************************************************************/
/* bm_char_data - bookmark for searching
* One character (PC or NPC).
*/
#include "chardata.h"

/**************************************************************************/
/*
* Prototype for a mob.
* This is the in-memory version of #MOBILES.
*/
struct  mob_index_data
{
	mob_index_data	*next; // next in the hash table bucket
	mob_index_data	*listnext; // next in a complete list of indexes
	SPEC_FUN		*spec_fun;
	GAMBLE_FUN		*gamble_fun;
	SHOP_DATA		*pShop;
	cInnData		*pInnData;
	MUDPROG_TRIGGER_LIST *mob_triggers; // mud progs
	long			mprog_flags;
	long			mprog2_flags;
	vn_int			vnum;
	sh_int			group;	
	sh_int			helpgroup;	
	sh_int			count;
	sh_int			killed;
	char *			player_name;
	char *			short_descr;
	char *			long_descr;
	char *			description;
	long			act;
	long			act2;
	long			affected_by;
	long			affected_by2;
	long			affected_by3;
	sh_int			alliance;
	sh_int			tendency;
	sh_int			level;
	sh_int			hitroll;
	sh_int			hit[3];
	sh_int			mana[3];
	sh_int			damage[3];
	sh_int			ac[4];
	sh_int			dam_type;
	long			off_flags;
	long			imm_flags;
	long			res_flags;
	long			vuln_flags;
	sh_int			start_pos;
	sh_int			default_pos;
	sh_int			sex;
	sh_int			race;
	long			wealth;
	long			form;
	long			parts;
	sh_int			size;
	char *			material;
	sh_int			xp_mod;
	area_data * 	area;
	char *			import_text; // used for importing in areas 
};

/**************************************************************************/
// Area Echo Struct
struct area_echo_data
{
	AREA_ECHO_DATA *next;       
    int firsthour;              
    int lasthour;				
    int percentage;				
    char *echotext;			 
};

/**************************************************************************/
//  Room echos struct
struct  room_echo_data 
{
    ROOM_ECHO_DATA *next;       // Next in list
    int firsthour;              // Start hour for room echos 
    int lasthour;				// End hour for room echos
    int percentage;				// percent of time on the tick it will be shown
    char *echotext;				// text to be sent if it goes off  
};
/**************************************************************************/
struct  class_type
{
	char * name;					// the full name of the clss
	char * short_name;				// Three-letter name for 'who'

	sh_int attr_prime[2];			// Prime attribute
	sh_int skill_adept;				// Maximum skill level
	sh_int thac0_00;				// Thac0 for level  0
	sh_int thac0_32;				// Thac0 for level 32
	sh_int hp_min;					// Min hp gained on gaining
	sh_int hp_max;					// Max hp gained on gaining
	bool   fMana;					// Class gains mana on level
	char * spinfo_letter;			// letter used for spinfo
	// "-" means can't use magic 
	char * default_group;			// default skills gained
	bool	creation_selectable;
	CLASS_CAST_TYPE class_cast_type;

	sh_int	core_clss;					// index from the coreclass table
	int		object_restriction_index;	// A->Z value set on objects to restrict 
										// core classes from using objects
	int		objrestrict;		// New IC Object restriction system

	long	flags;
	// dynamic data
	sh_int class_id;			// the ch->class value
	bool	already_loaded;		// prevent reading in the same class twice
	sh_int	remort_number;		// which remort this class belongs to
	char *	remort_to_classes;	// classes which this class can lead directly on to
	vn_int	recall;
	vn_int	morgue;
	
	// pose system implemented to replace storm's pose code
	char *pose_self[MAX_LEVEL];
	char *pose_others[MAX_LEVEL];

	char * newbie_prac_location_hint;	// tell them where they can prac
	char * newbie_train_location_hint;	// tell them where they can train
	vn_int pendant_vnum;	// give them a pendant when they create, to se their recall
	vn_int newbie_map_vnum;// give newbies a map for their class if configured
};

struct item_type
{
    int         type;
	char *		name;
};

struct totem_type
{
	int			totem;
	char *		name;
};

struct timefield_type
{
	sh_int		type;
	sh_int		lowhour;
	sh_int		highhour;
	char *		name;
};

struct modifier_type
{
	sh_int		type;
	char *		name;
	sh_int		modifier;
};

struct season_type
{
	int			season;
	char *		name;
};

struct weapon_type
{
    char *      name;
    vn_int      vnum_offset;
    sh_int      type;
    sh_int      *gsn;
};

struct wiznet_type
{
    char *      name;
	long		flag;
	int			level;
};

struct council_type
{
	char *		name;
	long		flag;
};

struct auto_type
{
	char *      name;
	long		flag;
	char *      offhelp;
	char *      onhelp;
};


struct attack_type
{
    char *      name;                   /* name */
    char *      noun;                   /* message */
    int         damage;                 /* damage clss */
};
/**************************************************************************/
/*
* Shop types.
*/
struct  shop_data
{
    SHOP_DATA * next;                   // Next shop in list            
    vn_int      keeper;                 // Vnum of shop keeper mob      
    sh_int      buy_type [MAX_TRADE];   // Item types shop will buy     
    sh_int      profit_buy;             // Cost multiplier for buying   
    sh_int      profit_sell;            // Cost multiplier for selling  
    sh_int      open_hour;              // First opening hour           
    sh_int      close_hour;             // First closing hour           
};

/**************************************************************************/
// A kill structure (indexed by level).
struct  kill_data
{
	vn_int         number;
    vn_int         killed;
};

/**************************************************************************/
struct moot_data
{
	char_data *called_by;   // character whom called the moot
	char_data *moot_victim; // victim of the moot
  char *    moot_victim_name;
	int       moot_type;    // type of moot
	int       scope;        // value of xp lost ot gained if applicable
	int       votes_for;
	int       votes_against;// number of votes for and against measure
	sh_int    pulse;        // how many pulses before moot is resolved
    sh_int    number_of_votes;
};

//  Data for generating characters 
// -- only used during generation 
struct gen_data
{
	GEN_DATA       *next;
	bool       valid;
	bool       skill_chosen[MAX_SKILL];
	bool       skillgroup_chosen[MAX_SKILLGROUP];
	int            points_chosen;
};

/*
* Liquids.
*/
#define LIQ_WATER        0

struct  liq_type
{
	char * liq_name;
    char *      liq_color;
	sh_int liq_affect[5];
};



/*
* Extra description data for a room or object.
*/
struct  extra_descr_data
{
    EXTRA_DESCR_DATA *next;     /* Next in list                     */
    bool valid;
    char *keyword;              /* Keyword in look/examine          */
	char *description;          /* What to see                      */
};



/*
* Prototype for an object.
*/
struct  obj_index_data
{
	OBJ_INDEX_DATA		*next;
	EXTRA_DESCR_DATA	*extra_descr;
	AFFECT_DATA			*affected;
	int					objrestrict; // bit mask for restrictions below - quicker lookup
	OBJRESTRICT_LIST_DATA *restrict;
    OSPEC_FUN			*ospec_fun;
	char				*name;
	char				*short_descr;
	char				*description;
	vn_int				vnum;
	MUDPROG_TRIGGER_LIST *obj_triggers; // mud progs
	long				oprog_flags;
	long				oprog2_flags;
	sh_int				reset_num;
	char				*material;
	sh_int				item_type;
	int					extra_flags;
	int					extra2_flags;
	int					wear_flags;
	sh_int				level;
	sh_int				condition;
	sh_int				count;
	sh_int				weight;
	int					cost;
	int					value[5];
	int					absolute_size;
	sh_int				relative_size;
	int					class_allowances;
	area_data			*area;
	long				trap_trig;			// trap trigger type....
    sh_int				trap_dtype;			// damage type trap will inflict
    sh_int				trap_charge;		// amount of charges the trap has, should be kept low
	sh_int				trap_modifier;		// difficulty rating of trap in % to be added to remove trap skill
	long				attune_id;
	long				attune_flags;
	sh_int				attune_modifier;
	time_t				attune_next;
};

/**************************************************************************/
// bm_object - bookmark for searching
#include "objdata.h"
/**************************************************************************/

struct	herb_type
{
	HERB_DATA *			next;
	char *				name;
	sh_int				area;
	sh_int				timefield;
	sh_int				month;
	sh_int				season;
	continent_type *	continent;
	sh_int				difficulty;
	vn_int				vnum_result;
	long				sector;
};


// Exit data.
struct  exit_data
{
	union
	{
		ROOM_INDEX_DATA *to_room;
		int				vnum;
	} u1;
	int					exit_info;
	int					key;
	char				*keyword;
	char				*description;
	exit_data			*next;
	int					rs_flags;       
};



/*
* Reset commands:
*   '*': comment
*   'M': read a mobile
*   'O': read an object
*   'P': put object in object
*   'G': give object to mobile
*   'E': equip object to mobile
*   'D': set state of door
*   'R': randomize room exits
*   'S': stop (end of list)
*/

// Area-reset definition.
struct  reset_data
{
    RESET_DATA *	next;
    char			command;
	int				arg1;
    int				arg2;
    int				arg3;
	int				arg4;
};


struct  area_data
{
    area_data * next;
    area_data * vnumsort_next;
	area_data * levelsort_next;
    area_data * arealist_sort_next;
    char *		name;
	char *		short_name; // displayed by room descript
    int			age;
    int			nplayer;
    bool		empty;	
    char *		file_name;	// OLC 
    char *		builders;	// OLC Listing of
	char *		build_restricts[MAX_BUILD_RESTRICTS];
    char *		colour;		// Colour in area list
	char 		colourcode;	// Colour code character used for colour codings
    char *		credits; 
    char *		lcomment;	// level comment - in place of lrange
    int			low_level;	// recommend low level
    int			high_level;	// recommend high level
    int			security;	// OLC Value 1-9
    vn_int		min_vnum;	// OLC Lower vnum 
    vn_int		max_vnum;	// OLC Upper vnum
    int			vnum;		// OLC Area vnum
    int			olc_flags;	// OLC 
    int			area_flags;	// OLC 
    int			mapscale;   // map scaling system
    int			maplevel;   // map scaling system
    sh_int		version;
    int			vnum_offset;
	continent_type * continent;	// continent area is in
	AREA_ECHO_DATA		*echoes;
};

class C_track_data; // defined in track.h

struct locker_room_data;
// bm_room_index_data - bookmark for searching 
// Room type.
struct  room_index_data
{
	ROOM_INDEX_DATA		*next;
	char_data			*people;
	sh_int				number_in_room;
	char_data			*alarm;
	OBJ_DATA			*contents;
	EXTRA_DESCR_DATA	*extra_descr;
	ROOM_ECHO_DATA		*echoes;
	area_data			*area;
	exit_data			*exit[MAX_DIR];
	char				*name;
	char				*description;
	char				*owner;
	vn_int				vnum;
	int					room_flags;
	int					room2_flags;
	sh_int				light;
	sh_int				sector_type;
	sh_int				heal_rate;
	sh_int				mana_rate;
	CClanType			*clan;
	RESET_DATA			*reset_first;    
	RESET_DATA			*reset_last;     
	AFFECT_DATA			*affected;
	long				affected_by;

	locker_room_data	*lockers;

	char				*invite_list; // list of names allowed in room, saved in separate file

	// yell system - uses a direction the yell sound was moving when entering 
	//				 the room, the amplitude and which yell it happens to be
	short				yell_enteredindir; // direction it was moving
	float				yell_amplitude;
	time_t				yellindex; // uses the current time to index a yell
	time_t				last_mined_in_room;
	C_track_data		*tracks;
	char				*msp_sound; // MSP Sound file
};


/**************************************************************************/
/**************************************************************************/
/*
* Skills include spells as a particular case.
*/
#include "sk_type.h"
/**************************************************************************/
struct  group_type
{
	char * oldname;
	sh_int oldrating[MAX_CLASS];
	char * oldspells[MAX_IN_GROUP];
};
/**************************************************************************/
struct  skillgroup_type
{
	char * name;
	sh_int rating[MAX_CLASS];
	sh_int skills[MAX_IN_GROUP+1];
	sh_int skillsvalue[MAX_IN_GROUP+1];
	long	flags;
	int		remort;	// which remort they have to be for it to become available
};
/**************************************************************************/
struct mudprog_code
{
    vn_int     vnum;
    char *     code;
    MUDPROG_CODE *   next;
	
	bool disabled;
	char *disabled_text;
	// since ver3sub1
	area_data *area; 
    char *		title;	// title of the program
    char *		author; 
    char *		last_editor; 
    time_t		last_editdate; 
};

/**************************************************************************/
struct mudprog_trigger_list
{
    int			trig_type;
	int			trig2_type;
    char *		trig_phrase;
	int			pos_flags;
	union		{
		mudprog_code	*prog;
		vn_int	temp_mpvnum;
	};
    MUDPROG_TRIGGER_LIST	*next;
};
/**************************************************************************/
enum MUDPROG_TRIG_ON{
	MUDPROG_TRIG_ON_MOBILE,
	MUDPROG_TRIG_ON_OBJECT,
	MUDPROG_TRIG_ON_ROOM,
};

/**************************************************************************/
// Structure for a social in the socials table.
struct  social_old_type
{
    char	name[20];
    char *  char_no_arg;
    char *  others_no_arg;
    char *  char_found;
    char *  others_found;
    char *  vict_found;
    char *  char_not_found;
    char *  char_auto;
    char *  others_auto;
	sh_int	ic;
};

/**************************************************************************/
struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  // error state of the buffer
	int			size;   // size in bytes
    char *      string; // buffer's string
};
/**************************************************************************/
// Time and weather stuff.
struct  time_info_data
{
    int         minute;
    int         hour;
    int         day;
    int         month;
    int         year;
};
/**************************************************************************/
struct  weather_data
{
    int         mmhg;
    int         change;
    int         sky;
    int         sunlight;
	int			mage_castmod;
	bool	    moon_getting_brighter;
};
/**************************************************************************/
struct  note_data
{
    note_data * next;
    bool		valid;
    short       type;
    char *      sender;
    char *      real_sender;
    char *		date;
    char *      to_list;
	char *      cc_list;
    char *      subject;
    char *      text;
    time_t		date_stamp;
};

/**************************************************************************/
enum APPLOC{
	APPLY_NONE,
	APPLY_ST,
	APPLY_QU,
	APPLY_PR,
	APPLY_EM,
	APPLY_IN,
	APPLY_CO,
	APPLY_AG,
	APPLY_SD,
	APPLY_ME,
	APPLY_RE,
	APPLY_SEX,
	APPLY_CLASS,
	APPLY_LEVEL,
	APPLY_AGE,
	APPLY_HEIGHT,
	APPLY_WEIGHT,
	APPLY_MANA,
	APPLY_HIT,
	APPLY_MOVE,
	APPLY_GOLD,
	APPLY_EXP,
	APPLY_AC,
	APPLY_HITROLL,
	APPLY_DAMROLL,
	APPLY_SAVES
};
/**************************************************************************/
struct  affect_data
{
	AFFECT_DATA *next;
	bool		valid;
	sh_int		where;
	sh_int		type;		// the sn of the affect, -1 if not sn based
	sh_int		level;		// level of the affect, ch must be level or 
								//   greater to get the affect
	sh_int		duration;	// how long it lasts
	APPLOC		location;
	sh_int		modifier;
	int			bitvector;
};

/**************************************************************************/
struct  groupaff_type
{
	groupaff_data	*next;
	bool			valid;
	sh_int			where;
	sh_int			type;		// the sn of the affect, -1 if not sn based
	sh_int			level;		// level of the affect, ch must be level or 
								//   greater to get the affect
	sh_int			duration;	// how long it lasts
	sh_int			location;	
	sh_int			modifier;
	int				bitvector;
	int				flag;		// misc stuff
};

/**************************************************************************/
struct  affliction_type
{
	affliction_data	*next;
	bool			valid;
	sh_int			where;
	sh_int			type;		// the sn of the affect, -1 if not sn based
	sh_int			level;		// level of the affect, ch must be level or 
								//   greater to get the affect
	sh_int			duration;	// how long it lasts
	sh_int			location;	
	sh_int			modifier;
	int				bitvector;
	int				flag;		// misc stuff
};

/**************************************************************************/
struct  classgroup_type
{
	char *	name;
	int		bitindex;
	char *	description;
	int		members[5];// bit array
	char *	text_members;
};
/**************************************************************************/
struct  affectprofile_type
{
	char *	name; 
	char *	description; 
	int	flags;
	char *	wear_message; 
	APPLOC	wear_location; 	
	sh_int	wear_amount;
	char *	forced_drop_message; 
	char *	remove_message;
	sh_int	move_chance; // how often the move affect will happen - percentage
	APPLOC	move_location;
	sh_int	move_amount;
	char *	move_message;
	sh_int	tick_chance; // how often the tick affect will happen - percentage
	sh_int	tick_location;
	sh_int	tick_amount;
	char *	tick_message;	
};
/**************************************************************************/
struct  objrestrict_list_type
{
	classgroup_type			*classgroup;
	affectprofile_type		*affectprofile;
	sh_int					priority;
	OBJRESTRICT_LIST_DATA	*next;
};
/**************************************************************************/
/*
* This structure is used in special.c to lookup spec funcs and
* also in olc_act.c to display listings of spec funcs.
*/
struct spec_type
{
	char	  * spec_name;
	SPEC_FUN  * spec_fun;
};
/**************************************************************************/
struct ospec_type
{
	char	  * ospec_name;
	OSPEC_FUN * ospec_fun;
};
/**************************************************************************/
/*
* This structure is used in bit.c to lookup flags and stats.
*/
struct flag_type
{
	char * name;
	int  bit;
	bool settable;
};
/**************************************************************************/
struct bit_type
{
	const   struct  flag_type * table;
	char *              help;
};
/**************************************************************************/
// memory for mobs 
struct mem_data
{
	MEM_DATA	*next;
	bool		valid;
	int			id;
	int			reaction;
	time_t		when;
};
/**************************************************************************/
// one disabled command 
struct disabled_data
{
	DISABLED_DATA *next;			// pointer to next node 
	struct cmd_type const *command; // pointer to the command struct
	char *disabled_by;				// name of disabler 
	sh_int level;					// level of disabler 
	char *disabled_for; // name which the command is disabled - all, or name 
};
/**************************************************************************/
struct directories_type
{
    char text[MSL];
	char directory[MSL];
};
/**************************************************************************/
struct position_type
{
    char *name;
    char *short_name;
};
/**************************************************************************/
struct sex_type
{
    char *name;
};
/**************************************************************************/
struct size_type
{
    char *name;
};
/**************************************************************************/
struct gamble_type
{
	char		*name;
	GAMBLE_FUN	*gamble_fun;
};
/**************************************************************************/
struct name_linkedlist_type
{
    char *name;
	name_linkedlist_type *next;
	int tag;
	int count;
};
/**************************************************************************/
struct sector_type
{
	char *name;
};
/**************************************************************************/
struct	deity_type
{
	DEITY_DATA	*next;
	DEITY_DATA	*prev;
	DEITY_DATA	*rival;
	char		*name;
	char		*description;
	vn_int		symbol[4];
	vn_int		shrinevnum;				// VNUM of D's shrine
	sh_int		tendency;				// D's tendency
	sh_int		alliance;				// D's alliance
	sh_int		followers;				// how many people follow this D	
	sh_int		max_followers;			// once followers is > than this, things will change a bit
    int			race;					// D's race
    int			sex;					// D's sex
	long		alignflags;				// D's alignment restrictions
	long		tendflags;				// D's tendency restrictions
	unsigned char race_allow_n[(MAX_RACE+7)/8]; // Races that are allowed to worship D
};
/**************************************************************************/
typedef struct _weather_influence_data
{
	int			sky_clear;
	int			sky_cloudy;
	int			sky_rainy;
	int			sky_lightning;
	int			max_chance;
}	weather_influence_data;

/**************************************************************************/
struct mix_type
{
	mix_data	*next;
	char		*name;					// Mixture's Name
	char		*creator;				// Person who made the mixture
	sh_int		type;					// Type of mix, Herbalism, Cooking, Pottery, etc
	vn_int		vnum_template;			// Vnum which will be restrung
	vn_int		ingredients[5];			// Ingredient vnums
	sh_int		ingredients_num[5];		// Number of each ingredient needed
	sh_int		prep_container;			// Container type (new item_type for combining ingredients)
	char		*rname;					// Resulting name
	char		*rshort;				// Resulting short desc
	char		*rlong;					// Resulting long desc
	int			ritem_type;				// Resulting item type
	char		*failedrname;			// Failed Resulting name
	char		*failedrshort;			// Failed Resulting short desc
	char		*failedrlong;			// Failed Resulting long desc
	int			vessel;					// item type in which mixture will be mixed in
	int			rvalue[5];				// Resulting iValues (v0-v4)
	int			rwear;					// Resulting wear locs
	sh_int		difficulty;				// modifier_table
	bool		locked;					// mixture locked status, will allow only creator or admin to edit the mixture
};

/**************************************************************************/
// who system to support modular who formatting functions
typedef char *WHO_FORMAT_FUNCTION( char_data *ch, char_data *wch, bool two_column);

struct who_format_type {
	const char *name;
	WHO_FORMAT_FUNCTION *whofunc;
	bool two_column_supported;
};
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
