/**************************************************************************/
// tables.cpp - tables looked up for bit fields etc
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
#include "laston.h"
#include "d2magsys.h"
#include "support.h"
#include "notenet.h"
#include "immquest.h"
#include "trap.h"

DECLARE_GAMBLE_FUN( gamble_seven );

// first are all the directories
const  struct  directories_type directories_table[]  =
{
	{	"area files", AREA_DIR},
	{	"area Room Invite List files", AREA_RIL_DIR},
	{	"backup area files", BACKUP_AREA_DIR},		
	{	"backup area Room Invite List files", BACKUP_AREA_RIL_DIR},		
	{	"help files", HELP_DIR},
	{	"backup help files", BACKUP_HELP_DIR},
	{	"logfiles root dir",	LOG_ROOT_DIR},
	{	"logdir: olc related logs",		OLC_LOGS_DIR	},
	{	"logdir: admin related logs",	ADMIN_LOGS_DIR	},
	{	"logdir: player logs",			PLAYER_LOGS_DIR	},
	{	"logdir: restring logs",		RESTRING_LOGS_DIR},
	{	"logdir: channel logs",			CHANNELS_LOGS_DIR},
	{	"logdir: support related logs",	SUPPORT_LOGS_DIR},
	{	"logdir: game logs (port-yymmdd-x.log)",GAME_LOGS_DIR},
	{	"logdir: imm logs",				IMMLOG_DIR	},

	{	"codelog parent - src directory is parent for code logs",	SRC_DIR	},
	{	"logdir: code related logs",	CODE_LOGS_DIR	},

	{	"game system parameters root directory",	SYSTEM_DIR},	
	{	"mud dynamic data directory", DATA_DIR},
	{	"languages system files", LANGUAGES_DIR},
	{	"output from 'class makealltables'", CLASSES_DIR},
	{	"contains all the deity info.", DEITY_DIR},	
	{	"contains all the scripts runable with the script system", SCRIPTS_DIR},	
	{	"notes directory", NOTES_DIR},

#if defined(NO_INITIAL_ALPHA_PFILEDIRS) || defined(WIN32) 
	{	"player files", PLAYER_DIR},
#else
	// WIN32 defines NO_INITIAL_ALPHA_PFILEDIRS 
	// NO_INITIAL_ALPHA_PFILEDIRS uses just the pfile directory
	{	"player files root", PLAYER_DIR},
	{	"player files - A", PLAYER_DIR "a" DIR_SYM},
	{	"player files - B", PLAYER_DIR "b" DIR_SYM},
	{	"player files - C", PLAYER_DIR "c" DIR_SYM},
	{	"player files - D", PLAYER_DIR "d" DIR_SYM},
	{	"player files - E", PLAYER_DIR "e" DIR_SYM},
	{	"player files - F", PLAYER_DIR "f" DIR_SYM},
	{	"player files - G", PLAYER_DIR "g" DIR_SYM},
	{	"player files - H", PLAYER_DIR "h" DIR_SYM},
	{	"player files - I", PLAYER_DIR "i" DIR_SYM},
	{	"player files - J", PLAYER_DIR "j" DIR_SYM},
	{	"player files - K", PLAYER_DIR "k" DIR_SYM},
	{	"player files - L", PLAYER_DIR "l" DIR_SYM},
	{	"player files - M", PLAYER_DIR "m" DIR_SYM},
	{	"player files - N", PLAYER_DIR "n" DIR_SYM},
	{	"player files - O", PLAYER_DIR "o" DIR_SYM},
	{	"player files - P", PLAYER_DIR "p" DIR_SYM},
	{	"player files - Q", PLAYER_DIR "q" DIR_SYM},
	{	"player files - R", PLAYER_DIR "r" DIR_SYM},
	{	"player files - S", PLAYER_DIR "s" DIR_SYM},
	{	"player files - T", PLAYER_DIR "t" DIR_SYM},
	{	"player files - U", PLAYER_DIR "u" DIR_SYM},
	{	"player files - V", PLAYER_DIR "v" DIR_SYM},
	{	"player files - W", PLAYER_DIR "w" DIR_SYM},
	{	"player files - X", PLAYER_DIR "x" DIR_SYM},
	{	"player files - Y", PLAYER_DIR "y" DIR_SYM},
	{	"player files - Z", PLAYER_DIR "z" DIR_SYM},
#endif
	{	"retired imms/heros directory", RETIRED_DIR},
	{	"Backup pfiles of players just before they begin remort.", REMORT_DIR},
	{	"pfiles requiring an email unlock code",PDIR_LOCKED},
	{	"pfiles of those with olc access",		PDIR_BUILDER},
	{	"pfiles of mortals with immortal trust",	PDIR_TRUSTED},
	{	"pfiles of immortal characters",			PDIR_IMMORTAL},
	{	"Pkilled pfiles", DEAD_DIR},
	{	"Pfiles of deleters above lvl 5", DELETE_DIR},
	{	"Pfiles that require an emailed unlock code to play.", LOCKED_PFILES_DIR},

	{	"msp base directory - should be a link to the base webpage url", MSP_DIR},
	{	"msp action sounds directory",	MSP_DIR MSP_ACTION_DIR},
	{	"msp combat sounds directory",	MSP_DIR MSP_COMBAT_DIR},
	{	"msp mudprog sounds directory - sounds called from mprogs", MSP_DIR MSP_MUDPROG_DIR},
	{	"msp room sounds directory",	MSP_DIR MSP_ROOM_DIR},
	{	"msp skills sounds directory",	MSP_DIR MSP_SKILLS_DIR},
	{	"msp spells sounds directory",	MSP_DIR MSP_SPELLS_DIR},
	{	"msp weather sounds directory",	MSP_DIR MSP_WEATHER_DIR},

    {   "", ""}
};

// for position 
const struct position_type position_table[] =
{
    { "dead",				"dead"  }, //0
    { "mortally wounded",	"mort"  }, //1
    { "incapacitated",		"incap" }, //2
    { "stunned",			"stun"  }, //3
    { "sleeping",			"sleep" }, //4
    { "resting",			"rest"  }, //5
    { "sitting",			"sit"   }, //6
    { "kneeling",			"kneel" }, //7
    { "fighting",			"fight" }, //8
    { "standing",			"stand" }, //9
    { NULL,       NULL}
};



/* for sex */
const struct sex_type sex_table[] =
{
	{	"none"          },
	{	"male"          },
	{	"female"        },
	{	"either"        },
    {   NULL      }
};

/* for sizes */
const struct size_type size_table[] =
{
	 {	"tiny"          },
	 {	"small"         },
	 {	"medium"        },
	 {	"large"         },
	 {	"huge"          },
	 {	"giant"         },
     {	NULL      }
};

// for gambling games, add each new game name here!!!
const struct gamble_type gamble_table[] =
{
	{	"seven",	gamble_seven				},
	{	NULL,		0							}
};

/* various flag tables */
const struct flag_type act_flags[] =
{
	{  "npc",				ACT_IS_NPC, 			false	},
	{  "aggressive",		ACT_AGGRESSIVE, 		true	},
	{  "amphibian", 		ACT_AMPHIBIAN,			true	},
	{  "changer",			ACT_IS_CHANGER, 		true	},
	{  "cleric",			ACT_CLERIC, 			true	},
	{  "docile",			ACT_DOCILE, 			true	},
	{  "dont_wander",		ACT_DONT_WANDER,		true	},
	{  "sentinel",			ACT_DONT_WANDER,		false	}, // now called ACT_DONT_WANDER
	{  "gain",				ACT_GAIN,				true	},
	{  "healer",			ACT_IS_HEALER,			true	},
	{  "ignore_nonquester", ACT_MPIGN_NONQUESTER,	true	},
	{  "ignore_quester",	ACT_MPIGN_QUESTER,		true	},
	{  "indoors",			ACT_INDOORS,			true	},
	{  "mage",				ACT_MAGE,				true	},
	{  "moblog",			ACT_MOBLOG, 			true	},
	{  "no_tame",			ACT_NO_TAME,			true	},
	{  "noalign",			ACT_NOALIGN,			true	},
	{  "noautosocial",		ACT_NOAUTOSOCIAL,		true	},
	{  "nopurge",			ACT_NOPURGE,			true	},
	{  "outdoors",			ACT_OUTDOORS,			true	},
	{  "pet",				ACT_PET,				true	},
	{  "practice",			ACT_PRACTICE,			true	},
	{  "scavenger", 		ACT_SCAVENGER,			true	},
	{  "stay_area", 		ACT_STAY_AREA,			true	},
	{  "thief", 			ACT_THIEF,				true	},
	{  "train", 			ACT_TRAIN,				true	},
	{  "undead",			ACT_UNDEAD, 			true	},
	{  "unseen",			ACT_IS_UNSEEN,			true	},
	{  "update_always", 	ACT_UPDATE_ALWAYS,		true	},
	{  "warrior",			ACT_WARRIOR,			true	},
	{  "wimpy", 			ACT_WIMPY,				true	},
	{  NULL,		   0,		0	}
};

const struct flag_type act2_flags[]=
{	
	{  "allskills",			ACT2_ALLSKILLS,			true    },	
	{  "nohunt",			ACT2_NOHUNT,			false   },		
	{  "avoids_all_attacks",ACT2_AVOIDS_ALL_ATTACKS,true	},	
	{  "no_tendency",		ACT2_NO_TENDENCY,		true	},		
	{  "mudprog_recursion_allowed",		ACT2_MUDPROG_RECURSION_ALLOWED,		true	},			
	{  NULL,				0,						0   }
};

const struct flag_type off_flags[] =
{
    {   "area_attack",      OFF_AREA_ATTACK,  true    },
    {   "backstab",			OFF_BACKSTAB   ,  true    },
    {   "bash",				OFF_BASH       ,  true    },
    {   "berserk",			OFF_BERSERK    ,  true    },
    {   "disarm",			OFF_DISARM     ,  true    },
    {   "dodge",			OFF_DODGE      ,  true    },
    {   "fade",				OFF_FADE       ,  true    },
    {   "fast",				OFF_FAST       ,  true    },
    {   "kick",				OFF_KICK       ,  true    },
    {   "dirt_kick",        OFF_KICK_DIRT  ,  true    },
    {   "parry",			OFF_PARRY      ,	true    },
    {   "rescue",			OFF_RESCUE     ,	true    },
    {   "tail",				OFF_TAIL       ,	true    },
    {   "trip",				OFF_TRIP       ,	true    },
    {   "crush",			OFF_CRUSH      ,	true    },
    {   "assist_all",		ASSIST_ALL     ,	true    },
    {   "assist_align",		ASSIST_ALIGN   ,	true    },
    {   "assist_race",		ASSIST_RACE    ,	true    },
    {   "assist_players",	ASSIST_PLAYERS ,	true    },
    {   "assist_guard",		ASSIST_GUARD   ,	true    },
    {   "assist_vnum",		ASSIST_VNUM    ,	true    },
    {   "gore",				OFF_GORE	,		true    },
    {   "uppercut",			OFF_UPPERCUT   	,	true    },
    {   "neck_thrust",		OFF_NECK_THRUST	,	true    },
    {   "web",				OFF_WEB			,	true    },
    {   "gaze",				OFF_GAZE       	,	true    },
	{   "circle",			OFF_CIRCLE,			true    },
    {   NULL,				0,	0		}
};

const struct flag_type imm_flags[] =
{
    {   "summon",		IMM_SUMMON,		true    },
    {   "charm",		IMM_CHARM,		true    },
    {   "magic",        IMM_MAGIC,		true    },
    {   "weapon",		IMM_WEAPON,		true    },
	{	"bash", 		IMM_BASH,		true	},
	{	"pierce",		IMM_PIERCE,		true	},
	{	"slash",		IMM_SLASH,		true	},
	{	"fire", 		IMM_FIRE,		true	},
	{	"cold", 		IMM_COLD,		true	},
	{	"lightning",	IMM_LIGHTNING,	true	},
	{	"acid", 		IMM_ACID,		true	},
	{	"poison",		IMM_POISON,		true	},
	{	"negative", 	IMM_NEGATIVE,	true	},
	{	"holy", 		IMM_HOLY,		true	},
	{	"energy",		IMM_ENERGY,		true	},
	{	"mental",		IMM_MENTAL,		true	},
	{	"disease",		IMM_DISEASE,	true	},
	{	"drowning", 	IMM_DROWNING,	true	},
	{	"light",		IMM_LIGHT,		true	},
	{	"sound",		IMM_SOUND,		true	},
	{	"sleep",		IMM_SLEEP,		true	},
    {   "wood",			IMM_WOOD,		true    },
	{	"silver",		IMM_SILVER,		true    },
	{	"iron",			IMM_IRON,		true    },
	{	"illusion",		IMM_ILLUSION,	true    },
	{	"scry",			IMM_SCRY,		true    },
 	{	"hunger",		IMM_HUNGER,		true    },
 	{	"thirst",		IMM_THIRST,		true    },
 	{	"fear",			IMM_FEAR,		true    },
	{	NULL,					0,		0		}
};



const struct flag_type form_flags[] =
{
    {   "edible",               FORM_EDIBLE,            true    },
    {   "poison",               FORM_POISON,            true    },
    {   "magical",              FORM_MAGICAL,           true    },
    {   "instant_decay",        FORM_INSTANT_DECAY,     true    },
    {   "other",                FORM_OTHER,             true    },   
    {   "animal",               FORM_ANIMAL,            true    },
    {   "sentient",             FORM_SENTIENT,          true    },
    {   "undead",               FORM_UNDEAD,            true    },
    {   "construct",            FORM_CONSTRUCT,         true    },
    {   "mist",                 FORM_MIST,              true    },
    {   "intangible",           FORM_INTANGIBLE,        true    },
    {   "biped",                FORM_BIPED,             true    },
    {   "centaur",              FORM_CENTAUR,           true    },
    {   "insect",               FORM_INSECT,            true    },
    {   "spider",               FORM_SPIDER,            true    },
	{	"crustacean",			FORM_CRUSTACEAN,		true	},
	{	"worm",					FORM_WORM,				true	},
	{	"blob",					FORM_BLOB,				true	},
	{	"mammal",				FORM_MAMMAL,			true	},
	{	"bird",					FORM_BIRD,				true	},
	{	"reptile",				FORM_REPTILE,			true	},
	{	"snake",				FORM_SNAKE,				true	},
	{	"dragon",				FORM_DRAGON,			true	},
	{	"amphibian",			FORM_AMPHIBIAN,			true	},
	{	"fish",					FORM_FISH ,				true	},
	{	"cold_blood",			FORM_COLD_BLOOD,		true	},
	{	"mountable",			FORM_MOUNTABLE,			true	},
	{	NULL,					0,						0		}
};

const struct flag_type part_flags[] =
{
	{	"head", 				PART_HEAD,				true	},
	{	"arms", 				PART_ARMS,				true	},
	{	"legs", 				PART_LEGS,				true	},
	{	"heart",				PART_HEART, 			true	},
	{	"brains",				PART_BRAINS,			true	},
	{	"guts", 				PART_GUTS,				true	},
	{	"hands",				PART_HANDS, 			true	},
	{	"feet", 				PART_FEET,				true	},
	{	"fingers",				PART_FINGERS,			true	},
	{	"ear",					PART_EAR,				true	},
	{	"eye",					PART_EYE,				true	},
	{	"long_tongue",			PART_LONG_TONGUE,		true	},
	{	"eyestalks",			PART_EYESTALKS, 		true	},
	{	"tentacles",			PART_TENTACLES, 		true	},
	{	"fins", 				PART_FINS,				true	},
	{	"wings",				PART_WINGS, 			true	},
	{	"tail", 				PART_TAIL,				true	},
	{	"claws",				PART_CLAWS, 			true	},
	{	"fangs",				PART_FANGS, 			true	},
	{	"horns",				PART_HORNS, 			true	},
	{	"scales",				PART_SCALES,			true	},
	{	"tusks",				PART_TUSKS, 			true	},
	{	NULL,		  0,		  0   }
};


const struct flag_type mprog_flags[] =
{                                       
    {   "act",          MTRIG_ACT,       true    },	
    {   "bribe",        MTRIG_BRIBE,     true    },	
    {   "command",      MTRIG_COMMAND,   true    },	
    {   "death",        MTRIG_DEATH,     true    },	
    {   "delay",        MTRIG_DELAY,     true    },	
    {   "entry",        MTRIG_ENTRY,     true    },	
    {   "exall",        MTRIG_EXALL,     true    },	
    {   "exit",         MTRIG_EXIT,      true    },	
    {   "fight",        MTRIG_FIGHT,     true    },	
    {   "give",         MTRIG_GIVE,      true    },	
    {   "grall",        MTRIG_GRALL,     true    },	
    {   "greet",        MTRIG_GREET,     true    },	
    {   "hour",         MTRIG_HOUR,      true    },	
    {   "hpcnt",        MTRIG_HPCNT,     true    },	
    {   "kill",         MTRIG_KILL,      true    },	
    {   "loginarea",    MTRIG_LOGINAREA, true    },	
    {   "loginroom",    MTRIG_LOGINROOM, true    },	
    {   "logoutarea",   MTRIG_LOGOUTAREA,true    },	
    {   "logoutroom",   MTRIG_LOGOUTROOM,true    },	
    {   "prekill",      MTRIG_PREKILL,   true    },	
    {   "premove",      MTRIG_PREMOVE,   true    },	
    {   "preprac",      MTRIG_PREPRAC,   true    },	
    {   "pretrain",     MTRIG_PRETRAIN,  true    },	
    {   "random",       MTRIG_RANDOM,    true    },	
    {   "repop",        MTRIG_REPOP,     true    },	
    {   "roomdeath",    MTRIG_ROOMDEATH, true    },	
    {   "sayto",        MTRIG_SAYTO,     true    },	
    {   "speech",       MTRIG_SPEECH,    true    },	
	{   "surrender",    MTRIG_SURR,      true    },	
    {   "surr",         MTRIG_SURR,      false    },	
    {   "tick",         MTRIG_TICK,      true    },	
	{	NULL,			0,				0		}
};

const struct flag_type mprog2_flags[] =
{                                       
    {   "X",			MTRIG2_X,		true    },	
	{	NULL,			0,				0		}
};

const struct flag_type oprog_flags[] =
{                                       
    {   "get_pre",		OTRIG_GET_PRE,		true    },	
    {   "get_post",		OTRIG_GET_POST,		true    },	
    {   "drop_pre",		OTRIG_DROP_PRE,		true    },	
	{   "drop_post",	OTRIG_DROP_POST,	true    },	
    {   "lookat_pre",	OTRIG_LOOKAT_PRE,	true    },	
    {   "lookat_post",	OTRIG_LOOKAT_POST,	true    },	
	{   "lookin_pre",	OTRIG_LOOKIN_PRE,	true    },	
	{   "lookin_post",	OTRIG_LOOKIN_POST,	true    },	
    {   "put_pre",		OTRIG_PUT_PRE,		true    },	
    {   "put_post",		OTRIG_PUT_POST,		true    },	
    {   "wear_pre",		OTRIG_WEAR_PRE,		true    },	
	{   "wear_mid",		OTRIG_WEAR_MID,		true    },		
    {   "wear_post",	OTRIG_WEAR_POST,	true    },	
    {   "container_get_pre",	OTRIG_CONTAINER_GET_PRE,	true    },	
    {   "container_get_post",	OTRIG_CONTAINER_GET_POST,	true    },	
    {   "container_putin_pre",	OTRIG_CONTAINER_PUTIN_PRE,	true    },	
    {   "container_putin_post",	OTRIG_CONTAINER_PUTIN_POST,	true    },	
    {   "remove_pre",	OTRIG_REMOVE_PRE,		true    },	
	{   "remove_post",	OTRIG_REMOVE_POST,		true    },		    	
	{	NULL,			0,					0		}
};

const struct flag_type oprog2_flags[] =
{                                       
    {   "X",			OTRIG2_X,       true    },	
	{	NULL,			0,				0		}
};

const struct flag_type rprog_flags[] =
{                                       
    {   "entry",		RTRIG_ENTRY,	true    },	
	{	NULL,			0,				0		}
};

const struct flag_type rprog2_flags[] =
{                                       
    {   "X",			RTRIG2_X,		true    },	
	{	NULL,			0,				0		}
};


const struct flag_type olc_flags[] =
{
    {   "none",						OLCAREA_NONE,						false   },
    {   "changed",					OLCAREA_CHANGED,					true    },
    {   "added",					OLCAREA_ADDED,						true    },
    {   "ignore_undefined_flags",	OLCAREA_IGNORE_UNDEFINED_FLAGS,		true	},	
	{   "invitelistchanged",		OLCAREA_INVITELISTCHANGED,			true	},		
    {   NULL,   0,  0   }
};

const struct flag_type area_flags[] =
{
    {   "none",         AREA_NONE,          false   },
    {   "olconly",      AREA_OLCONLY,       true    },
    {   "noteleport",   AREA_NOTELEPORT,    true    },
    {   "noscry",		AREA_NOSCRY,		true    },
    {   "hidden",		AREA_HIDDEN,		true    },
    {   "use_buildrestricts", AREA_USE_BUILDRESTRICTS,false},
    {   "locked",		AREA_LOCKED,		false},
    {   "nogateinto",	AREA_NOGATEINTO,	true},
    {   "nosummoninto",	AREA_NOSUMMONINTO,	true},
    {   "noportalinto",	AREA_NOPORTALINTO,	true},		
    {   "newbie_area_resets",AREA_NEWBIE_AREA_RESETS,	true},			
	{   NULL,   0,  0   }
};


const struct flag_type sex_types[] =
{
    {   "male",         SEX_MALE,       true    },
    {   "female",       SEX_FEMALE,     true    },
    {   "neutral",      SEX_NEUTRAL,    true    },
    {   "random",       SEX_RANDOM,		true    },
//   {   "none",         SEX_NEUTRAL,    false	},
    {   NULL,           0,          0   }
};

const struct flag_type direction_types[] =
{
	{	"north",		DIR_NORTH,		true	},
	{	"east",			DIR_EAST,		true	},
	{	"south",		DIR_SOUTH,		true	},
	{	"west",			DIR_WEST,		true	},
	{	"up",			DIR_UP,			true	},
	{	"down",			DIR_DOWN,		true	},
	{	"northeast",	DIR_NORTHEAST,	true	},
	{	"southeast",	DIR_SOUTHEAST,	true	},
	{	"southwest",	DIR_SOUTHWEST,	true	},
	{	"northwest",	DIR_NORTHWEST,	true	},
    {   NULL,           0,          0   }
};


const struct flag_type exit_flags[] =
{
    {   "door",         EX_ISDOOR,      true    },
    {   "closed",       EX_CLOSED,      true    },
    {   "locked",       EX_LOCKED,      true    },
    {   "pickproof",    EX_PICKPROOF,   true    },
    {   "nopass",       EX_NOPASS,      true    },
    {   "easy",         EX_EASY,        true    },
    {   "hard",         EX_HARD,        true    },
    {   "infuriating",  EX_INFURIATING, true    },
    {   "noclose",      EX_NOCLOSE,     true    },
    {   "nolock",       EX_NOLOCK,      true    },
    {   "oneway",       EX_ONEWAY,      true    },
    {   "obvious",      EX_OBVIOUS,     true    },	
    {   NULL,           0,          0   }
};

const struct flag_type room_flags[] =
{
    {  "antimagic",         ROOM_ANTIMAGIC,         true    },
    {  "arena",             ROOM_ARENA,             false   }, // doesn't do anything yet
    {  "bank",              ROOM_BANK,              true    },
    {  "dark",              ROOM_DARK,              true    },
    {  "heroes_only",       ROOM_HEROES_ONLY,       true    },
    {  "imm_only",          ROOM_GODS_ONLY,         true    },
    {  "imp_only",          ROOM_IMP_ONLY,          true    },
    {  "indoors",           ROOM_INDOORS,           true    },
	{  "inn",				ROOM_INN,				true	},
    {  "law",               ROOM_LAW,               true    },
    {  "light",             ROOM_LIGHT,             true    },
    {  "newbies_only",      ROOM_NEWBIES_ONLY,      true    },
    {  "no_mob",            ROOM_NO_MOB,            true    },
    {  "no_portal",         ROOM_NO_PORTAL,         false   }, // doesn't do anything yet
    {  "no_recall",         ROOM_NO_RECALL,         true    },
    {  "no_summon",         ROOM_NO_SUMMON,         false   }, // doesn't do anything yet
    {  "noautoexits",       ROOM_NOAUTOEXITS,       true    },
    {  "noautomap",         ROOM_NOAUTOMAP,         true    },
    {  "nochannels",        ROOM_NOCHANNELS,        true    },
    {  "nofly",             ROOM_NOFLY,             true    },
    {  "noscan",            ROOM_NOSCAN,            true    },
    {  "noscry",            ROOM_NOSCRY,            true    },
    {  "nospeak",           ROOM_NOSPEAK,           true    },
    {  "nowhere",           ROOM_NOWHERE,           true    },
    {  "ooc",               ROOM_OOC,               true    },
    {  "pet_shop",          ROOM_PET_SHOP,          true    },
    {  "private",           ROOM_PRIVATE,           true    },
    {  "safe",              ROOM_SAFE,              true    },
    {  "solitary",          ROOM_SOLITARY,          true    },
	{  NULL,                0,                      0		}
};

const struct flag_type room2_flags[]=
{
    {  "no_area_echoes",	ROOM2_NO_AREA_ECHOES,	true    },
    {  "mine",				ROOM2_MINE,				true    },

	{  "saltwater_fishing",		ROOM2_SALTWATER_FISHING,	false	},
	{  "freshwater_fishing",	ROOM2_FRESHWATER_FISHING,	false	},

	{  NULL,                0,                      0		}
};

const struct flag_type sector_types[] =
{
	{	"inside",	SECT_INSIDE,		true	},
	{	"city", 	SECT_CITY,			true	},
	{	"field",	SECT_FIELD, 		true	},
	{	"forest",	SECT_FOREST,		true	},
	{	"hills",	SECT_HILLS, 		true	},
	{	"mountain", SECT_MOUNTAIN,		true	},
	{	"swim", 	SECT_WATER_SWIM,	true	},
	{	"noswim",	SECT_WATER_NOSWIM,	true	},
    {   "swamp",    SECT_SWAMP,         true    },
	{	"air",		SECT_AIR,			true	},
	{	"desert",	SECT_DESERT,		true	},
	{	"cave",		SECT_CAVE,			true	},
	{	"underwater",SECT_UNDERWATER,	true	},
	{	"snow",		SECT_SNOW,			true	},
	{   "ice",		SECT_ICE,			true	},
	{   "trail",	SECT_TRAIL,			true	},
	{	"lava",		SECT_LAVA, 			true	},
	{	NULL,		0,					0	}
};



const struct flag_type item_types[] =
{
    {  "armor",         ITEM_ARMOR,         true    },
    {  "boat",          ITEM_BOAT,          true    },
    {  "cauldron",      ITEM_CAULDRON,      true    },
    {  "clothing",      ITEM_CLOTHING,      true    },
    {  "component",     ITEM_COMPONENT,     true    },
    {  "container",     ITEM_CONTAINER,     true    },
    {  "drinkcontainer",ITEM_DRINK_CON,     true    },
    {  "flask",         ITEM_FLASK,         true    },
    {  "food",          ITEM_FOOD,          true    },
    {  "fountain",      ITEM_FOUNTAIN,      true    },
    {  "furniture",     ITEM_FURNITURE,     true    },
    {  "gem",           ITEM_GEM,           true    },
    {  "herb",          ITEM_HERB,          true    },
    {  "instrument",    ITEM_INSTRUMENT,    true    },
    {  "jewelry",       ITEM_JEWELRY,       true    },
    {  "jukebox",       ITEM_JUKEBOX,       false   },
    {  "key",           ITEM_KEY,           true    },
    {  "light",         ITEM_LIGHT,         true    },
    {  "map",           ITEM_MAP,           true    },
    {  "money",         ITEM_MONEY,         true    },
    {  "mortar",        ITEM_MORTAR,        true    },
    {  "npccorpse",     ITEM_CORPSE_NPC,    true    },
    {  "ore",           ITEM_ORE,           true    },
    {  "parchment",     ITEM_PARCHMENT,     true    },
    {  "pc corpse",     ITEM_CORPSE_PC,     false   }, // loading
    {  "pc_corpse",     ITEM_CORPSE_PC,     false   },
    {  "pill",          ITEM_PILL,          true    },
    {  "portal",        ITEM_PORTAL,        true    },
    {  "potion",        ITEM_POTION,        true    },
    {  "poultice",      ITEM_POULTICE,      true    },
    {  "protect",       ITEM_PROTECT,       true    },
    {  "roomkey",       ITEM_ROOM_KEY,      true    },
	{  "recallpendant", ITEM_PENDANT,		false   }, // cant set in non MF
    {  "rp",            ITEM_RP,            true    },
    {  "scroll",        ITEM_SCROLL,        true    },
	{  "sheath",		ITEM_SHEATH,		true	},
    {  "staff",         ITEM_STAFF,         true    },
    {  "token",         ITEM_TOKEN,         true    },
    {  "trash",         ITEM_TRASH,         true    },
    {  "treasure",      ITEM_TREASURE,      true    },
    {  "wand",          ITEM_WAND,          true    },
    {  "warpstone",     ITEM_WARP_STONE,    true    },
    {  "weapon",        ITEM_WEAPON,        true    },
    {  NULL,            0,                  0       }
};


const struct flag_type objextra_flags[] =
{
    {  "antievil",		OBJEXTRA_ANTI_EVIL,		true    },
    {  "antigood",		OBJEXTRA_ANTI_GOOD,		true    },
    {  "antineutral",	OBJEXTRA_ANTI_NEUTRAL,		true    },
    {  "bless",			OBJEXTRA_BLESS,			true    },
    {  "burnproof",		OBJEXTRA_BURN_PROOF,		true    },
    {  "chaos",			OBJEXTRA_CHAOS,			true    },
    {  "dark",			OBJEXTRA_DARK,				true    },
    {  "evil",			OBJEXTRA_EVIL,				true    },
    {  "glow",			OBJEXTRA_GLOW,				true    },
    {  "hadtimer",		OBJEXTRA_HAD_TIMER,		true    },
    {  "horned",		OBJEXTRA_HORNED,			true    },
    {  "hum",			OBJEXTRA_HUM,				true    },
    {  "inventory",		OBJEXTRA_INVENTORY,		true    },
    {  "invis",			OBJEXTRA_INVIS,			true    },
    {  "lodged",		OBJEXTRA_LODGED,			false	},
    {  "lock",			OBJEXTRA_LOCK,				true    },
    {  "magic",			OBJEXTRA_MAGIC,			true    },
    {  "meltdrop",		OBJEXTRA_MELT_DROP,		true    },
    {  "nodegrade",		OBJEXTRA_NO_DEGRADE,		true    },
    {  "nodrop",		OBJEXTRA_NODROP,			true    },
	{  "nogetall",		OBJEXTRA_NO_GET_ALL,		true	},
    {  "nolocate",		OBJEXTRA_NOLOCATE,			true    },
    {  "nonmetal",		OBJEXTRA_NONMETAL,			true    },
    {  "nopurge",		OBJEXTRA_NOPURGE,			true    },
    {  "noremove",		OBJEXTRA_NOREMOVE,			true    },
	{  "norestring",	OBJEXTRA_NO_RESTRING,		true	},
    {  "nouncurse",		OBJEXTRA_NOUNCURSE,		true    },
	{  "otterlungs",	OBJEXTRA_OTTERLUNGS,		true	},
    {  "rotdeath",		OBJEXTRA_ROT_DEATH,		true    },
    {  "sellextract",	OBJEXTRA_SELL_EXTRACT,		true    },
    {  "visdeath",		OBJEXTRA_VIS_DEATH,		true    },
    {  NULL,			0,          0   }
};

const struct flag_type objextra2_flags[]=
{
    {  "buried",		OBJEXTRA2_BURIED,		true    },
	{  "nodecay",		OBJEXTRA2_NODECAY,		true	},
	{  "noprimary",		OBJEXTRA2_NOPRIMARY,	true	},
	{  "nosecondary",	OBJEXTRA2_NOSECONDARY,	true	},
	{  "nochaoslace",	OBJEXTRA2_NOCHAOS,		true	},
	{  "nosell",		OBJEXTRA2_NOSELL,		true	},
	{  "antilaw",		OBJEXTRA2_ANTI_LAW,		true	},
	{  "antichaos",		OBJEXTRA2_ANTI_CHAOS,	true	},
	{  "antibalance",	OBJEXTRA2_ANTI_BALANCE,	true	},
	{  "noquest",		OBJEXTRA2_NOQUEST,		false	}, // not implemented yet
// not to be set in olc, will have it's own command since it will be a bit more specialized
	{  "trap",			OBJEXTRA2_TRAP,			false	},

// below from vot - false till actually do something
	{  "questitem",		OBJEXTRA2_QUEST,		false },
	{  "holy",			OBJEXTRA2_HOLY,			false },
	{  "remort",		OBJEXTRA2_REMORT,		false },
	{  "vampire_bane",	OBJEXTRA2_VAMPIRE_BANE,	false },
	{  "mudprog_recursion_allowed", OBJEXTRA2_MUDPROG_RECURSION_ALLOWED, true},
	{  "blessed_in_holy_hands",	OBJEXTRA2_BLESSED_IN_HOLY_HANDS, true },

	{  "bait",			OBJEXTRA2_BAIT,			false },
	
    {  NULL,			0,          0   }
};

const struct flag_type objspell_flags[]=
{
    {  "active",		OBJSPELL_ACTIVE,		false	},
	{  "ignore_level",	OBJSPELL_IGNORE_LEVEL,	true	},
    {  NULL,			0,          0   }
};

const struct flag_type wear_flags[] =
{
    {  "take",			OBJWEAR_TAKE,		true    },
    {  "finger",		OBJWEAR_FINGER,		true    },
    {  "neck",			OBJWEAR_NECK,		true    },
    {  "torso",			OBJWEAR_TORSO,		true    },
    {  "body",			OBJWEAR_TORSO,		false	},
    {  "head",			OBJWEAR_HEAD,		true    },
    {  "legs",			OBJWEAR_LEGS,		true    },
    {  "feet",			OBJWEAR_FEET,		true    },
    {  "hands",			OBJWEAR_HANDS,		true    },
    {  "arms",			OBJWEAR_ARMS,		true    },
    {  "shield",		OBJWEAR_SHIELD,		true    },
    {  "about",			OBJWEAR_ABOUT,		true    },
    {  "waist",			OBJWEAR_WAIST,		true    },
    {  "wrist",			OBJWEAR_WRIST,		true    },
    {  "wield",			OBJWEAR_WIELD,		true    },
    {  "hold",			OBJWEAR_HOLD,		true    },
    {  "no_sac",		OBJWEAR_NO_SAC,		true    },	
    {  "float",			OBJWEAR_FLOAT,		true    },
    {  "wearfloat",		OBJWEAR_FLOAT,		false	},
	{  "lodged_arm",	OBJWEAR_LODGED_ARM,	false	},
	{  "lodged_leg",	OBJWEAR_LODGED_LEG,	false	},
	{  "lodged_rib",	OBJWEAR_LODGED_RIB,	false	},
	{  "eyes",			OBJWEAR_EYES,		true	},
	{  "ear",			OBJWEAR_EAR,		true	},
	{  "face",			OBJWEAR_FACE,		true	},
	{  "ankle",			OBJWEAR_ANKLE,		true	},
 	{  "back",			OBJWEAR_BACK,		true	},
	{  "pendant",		OBJWEAR_PENDANT,	false	},
    {  NULL,           0,          0   }
};


/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */

const struct flag_type apply_types[] =
{
    {  "none",			APPLY_NONE,				false   },
    {  "strength",		APPLY_ST,				true    },
    {  "quickness",		APPLY_QU,				true    },
    {  "presence",		APPLY_PR,				true    },
    {  "intuition",		APPLY_IN,				true    },
    {  "empathy",		APPLY_EM,				true    },
    {  "constitution",	APPLY_CO,				true    },
    {  "agility",		APPLY_AG,				true    },
	{  "selfdiscipline",APPLY_SD,				true    },
	{  "memory",		APPLY_ME,				true    },
	{  "reasoning",		APPLY_RE,				true	},
    {  "sex",			APPLY_SEX,				true    },
    {  "mana",			APPLY_MANA,				true    },
    {  "hp",			APPLY_HIT,				true    },
    {  "move",			APPLY_MOVE,				true    },
    {  "ac",			APPLY_AC,				true    },
    {  "hitroll",		APPLY_HITROLL,			true    },
    {  "damroll",		APPLY_DAMROLL,			true    },
    {  "saves",			APPLY_SAVES,			true    },
	// below here doesn't do anything in affect_modify() etc
	{  "age",			APPLY_AGE,				false	},
    {  "class",			APPLY_CLASS,			false	},
    {  "experience",    APPLY_EXP,				false   },
    {  "gold",			APPLY_GOLD,				false   },
    {  "level",			APPLY_LEVEL,			false	},
    {  "height",		APPLY_HEIGHT,			false	},
    {  "weight",		APPLY_WEIGHT,			false	},
    {  NULL,           0,          0   }
};

/*
 * What is seen.
 */
const struct flag_type wear_location_strings_types[] =
{
    {  "in the inventory",		WEAR_NONE,		true    },
    {  "as a light",			WEAR_LIGHT,		true    },
    {  "on the left finger",	WEAR_FINGER_L,	true    },
    {  "on the right finger",	WEAR_FINGER_R,	true    },
    {  "around the neck (1)",	WEAR_NECK_1,	true    },
    {  "around the neck (2)",	WEAR_NECK_2,	true    },
    {  "on the body",			WEAR_TORSO,		true    },
    {  "over the head",			WEAR_HEAD,		true    },
    {  "on the legs",			WEAR_LEGS,		true    },
    {  "on the feet",			WEAR_FEET,		true    },
    {  "on the hands",			WEAR_HANDS,		true    },
    {  "on the arms",			WEAR_ARMS,		true    },
    {  "as a shield",			WEAR_SHIELD,	true    },
    {  "about the shoulders",	WEAR_ABOUT,		true    },
    {  "around the waist",		WEAR_WAIST,		true    },
    {  "on the left wrist",		WEAR_WRIST_L,	true    },
    {  "on the right wrist",	WEAR_WRIST_R,	true    },
    {  "wielded",				WEAR_WIELD,		true    },
    {  "held in the hands",		WEAR_HOLD,		true    },
    {  "floating nearby",		WEAR_FLOAT,		true    },
    {  "wielded (secondary).",  WEAR_SECONDARY,	true    },
	{  "lodge in arm",			WEAR_LODGED_ARM,	true	},
	{  "lodge in leg",			WEAR_LODGED_LEG,	true	},
	{  "lodge in ribs",			WEAR_LODGED_RIB,	true	},
	{  "sheathed",				WEAR_SHEATHED,		true	},
	{  "concealed",				WEAR_CONCEALED,		true	},
	{  "on the eyes",			WEAR_EYES,			true	},
	{  "on the left ear",		WEAR_EAR_L,  		true	},
	{  "on the right ear",		WEAR_EAR_R,  		true	},
	{  "on the face",			WEAR_FACE,   		true	},
	{  "on the left ankle",		WEAR_ANKLE_L,		true	},
	{  "on the right ankle",	WEAR_ANKLE_R,		true	},
 	{  "on the back",			WEAR_BACK,			true	},
	{  "around the neck (pendant)",	WEAR_PENDANT,	true	},

    {  NULL,           0         , 0   }
};


const struct flag_type wear_location_types[] =
{
    {  "none",     		WEAR_NONE,			true    },
    {  "light",    		WEAR_LIGHT,			true    },
    {  "lfinger",  		WEAR_FINGER_L,		true    },
    {  "rfinger",  		WEAR_FINGER_R,		true    },
    {  "neck1",    		WEAR_NECK_1,		true    },
    {  "neck2",    		WEAR_NECK_2,		true    },
    {  "torso",			WEAR_TORSO,			true    },
    {  "body",     		WEAR_TORSO,			false   },
    {  "head",     		WEAR_HEAD,			true    },
    {  "legs",     		WEAR_LEGS,			true    },
    {  "feet",     		WEAR_FEET,			true    },
    {  "hands",    		WEAR_HANDS,			true    },
    {  "arms",     		WEAR_ARMS,			true    },
    {  "shield",   		WEAR_SHIELD,		true    },
    {  "about",    		WEAR_ABOUT,			true    },
    {  "waist",    		WEAR_WAIST,			true    },
    {  "lwrist",   		WEAR_WRIST_L,		true    },
    {  "rwrist",		WEAR_WRIST_R,		true    },
    {  "wielded",  		WEAR_WIELD,			true    },
    {  "hold",    		WEAR_HOLD,			true    },
    {  "floating", 		WEAR_FLOAT,			true    },
    {  "second",   		WEAR_SECONDARY,		true    },
	{  "lodge_arm",		WEAR_LODGED_ARM,	true	},
	{  "lodge_leg",		WEAR_LODGED_LEG,	true	},
	{  "lodge_rib",		WEAR_LODGED_RIB,	true	},
	{  "sheathed",		WEAR_SHEATHED,		true	},
	{  "concealed",		WEAR_CONCEALED,		true	},
	{  "eyes",			WEAR_EYES,			true	},
	{  "lear",			WEAR_EAR_L,  		true	},
	{  "rear",			WEAR_EAR_R,  		true	},
	{  "face",			WEAR_FACE,   		true	},
	{  "lankle",		WEAR_ANKLE_L,		true	},
	{  "rankle",		WEAR_ANKLE_R,		true	},
 	{  "back",			WEAR_BACK,			true	},
	{  "pendant",		WEAR_PENDANT,		true	},
    {  NULL,       0,      0   }	
};	
	
const struct flag_type container_flags[] =
{
    {  "closeable",		CONT_CLOSEABLE,		true    },
    {  "pickproof",		CONT_PICKPROOF,		true    },
    {  "closed",		CONT_CLOSED,		true    },
    {  "locked",		CONT_LOCKED,		true    },
    {  "puton",			CONT_PUT_ON,		true    },
    {  "locker",		CONT_LOCKER,		false	}, // used to redirect lock without a key
    {  NULL,	0,      0   }
};

/*****************************************************************************
		      ROM - specific tables:
 ****************************************************************************/



const struct flag_type ac_types[] =
{
    {   "pierce",        AC_PIERCE,            true    },
    {   "bash",          AC_BASH,              true    },
    {   "slash",         AC_SLASH,             true    },
    {   "exotic",        AC_EXOTIC,            true    },
    {   NULL,            0,                    0       }
};


const struct flag_type size_types[] =
{
    {   "tiny",          SIZE_TINY,            true    },
    {   "small",         SIZE_SMALL,           true    },
    {   "medium",        SIZE_MEDIUM,          true    },
    {   "large",         SIZE_LARGE,           true    },
    {   "huge",          SIZE_HUGE,            true    },
    {   "giant",         SIZE_GIANT,           true    },
    {   NULL,            0,                    0       },
};

const struct flag_type weapon_class_types[] =
{
    {   "exotic",	WEAPON_EXOTIC,		true    },
    {   "spear",	WEAPON_SPEAR,		true    },
    {   "sword",	WEAPON_SWORD,		true    },
    {   "dagger",	WEAPON_DAGGER,		true    },
    {   "staff",	WEAPON_STAFF,		true    },
    {   "mace",		WEAPON_MACE,		true    },
    {   "axe",		WEAPON_AXE,			true    },
    {   "flail",	WEAPON_FLAIL,		true    },
    {   "whip",		WEAPON_WHIP,		true    },
    {   "polearm",	WEAPON_POLEARM,		true    },
	{	"sickle",	WEAPON_SICKLE,		true	},
    {   NULL,      0,          0       }
};


const struct flag_type weapon_flags[] =
{
    {  "flaming",	WEAPON_FLAMING,		true    },
    {  "frost",		WEAPON_FROST,		true    },
    {  "vampiric",	WEAPON_VAMPIRIC,	true    },
    {  "sharp",		WEAPON_SHARP,		true    },
    {  "vorpal",	WEAPON_VORPAL,		true    },
    {  "twohands",	WEAPON_TWO_HANDS,	true    },
    {  "shocking",	WEAPON_SHOCKING,	true    },
    {  "poison",	WEAPON_POISON,		true    },
	{  "holy",		WEAPON_HOLY,		true    },
	{  "living",	WEAPON_LIVING,		false	},	
	{  "slayer",    WEAPON_SLAYER,      true	},
    {  "suckle",    WEAPON_SUCKLE,      true	},
    {  "enervate",  WEAPON_ENERVATE,    true	},
    {  "annealed",  WEAPON_ANNEALED,    true	},
    {   NULL,		0,                  0       }
};

const struct flag_type res_flags[] =
{
    {   "summon",       RES_SUMMON,         true    },
    {   "charm",        RES_CHARM,          true    },
    {   "magic",        RES_MAGIC,          true    },
    {   "weapon",       RES_WEAPON,         true    },
    {   "bash",         RES_BASH,           true    },
    {   "pierce",       RES_PIERCE,         true    },
    {   "slash",        RES_SLASH,          true    },
    {   "fire",         RES_FIRE,           true    },
    {   "cold",         RES_COLD,           true    },
    {   "lightning",    RES_LIGHTNING,      true    },
    {   "acid",         RES_ACID,           true    },
    {   "poison",       RES_POISON,         true    },
    {   "negative",     RES_NEGATIVE,       true    },
    {   "holy",         RES_HOLY,           true    },
    {   "energy",       RES_ENERGY,         true    },
    {   "mental",       RES_MENTAL,         true    },
    {   "disease",      RES_DISEASE,        true    },
    {   "drowning",     RES_DROWNING,       true    },
    {   "light",        RES_LIGHT,          true    },
    {   "sound",        RES_SOUND,          true    },
    {   "wood",         RES_WOOD,           true    },
    {   "silver",       RES_SILVER,         true    },
    {   "iron",         RES_IRON,           true    },
    {   "illusion",     RES_ILLUSION,		true    },	
    {   "scry",         RES_SCRY,           true    },  
    {   NULL,           0,                  0       }
};


const struct flag_type vuln_flags[] =
{
    {	"summon",		 VULN_SUMMON,		   true    },
    {	"charm",		 VULN_CHARM,		   true    },
    {   "magic",         VULN_MAGIC,           true    },
    {   "weapon",        VULN_WEAPON,          true    },
    {   "bash",          VULN_BASH,            true    },
    {   "pierce",        VULN_PIERCE,          true    },
    {   "slash",         VULN_SLASH,           true    },
    {   "fire",          VULN_FIRE,            true    },
    {   "cold",          VULN_COLD,            true    },
    {   "lightning",     VULN_LIGHTNING,       true    },
    {   "acid",          VULN_ACID,            true    },
    {   "poison",        VULN_POISON,          true    },
    {   "negative",      VULN_NEGATIVE,        true    },
    {   "holy",          VULN_HOLY,            true    },
    {   "energy",        VULN_ENERGY,          true    },
    {   "mental",        VULN_MENTAL,          true    },
    {   "disease",       VULN_DISEASE,         true    },
    {   "drowning",      VULN_DROWNING,        true    },
    {   "light",         VULN_LIGHT,           true    },
    {	"sound",		 VULN_SOUND,		   true    },
    {   "wood",          VULN_WOOD,            true    },
    {   "silver",        VULN_SILVER,          true    },
    {   "iron",          VULN_IRON,            true		},
    {   "illusion",		 VULN_ILLUSION,			true    },	
	{	"scry",          VULN_SCRY,      true    },	
    {   NULL,              0,                    0		}
};

const struct flag_type position_types[] =
{
    {   "dead",           POS_DEAD,				true	},	// was false
    {   "mortal",         POS_MORTAL,			false   },
    {   "incap",          POS_INCAP,			false   },
    {   "stunned",        POS_STUNNED,			false	},
    {   "sleeping",       POS_SLEEPING,			true    },
    {   "resting",        POS_RESTING,			true    },
    {   "sitting",        POS_SITTING,			true    },
    {	"kneeling",		  POS_KNEELING,			true	},
    {   "fighting",       POS_FIGHTING,			true	},
    {   "standing",       POS_STANDING,			true    },
    {   NULL,             0,					0       }
};


const struct flag_type position_flags[] =
{
    {   "dead",           1<<(POS_DEAD),			false	},
    {   "mortal",         1<<(POS_MORTAL),			false   },
    {   "incap",          1<<(POS_INCAP),			false   },
    {   "stunned",        1<<(POS_STUNNED),			false	},
    {   "sleeping",       1<<(POS_SLEEPING),		true    },
    {   "resting",        1<<(POS_RESTING),			true    },
    {   "sitting",        1<<(POS_SITTING),			true    },
    {	"kneeling",		  1<<(POS_KNEELING),		true	},
    {   "fighting",       1<<(POS_FIGHTING),		true	},
    {   "standing",       1<<(POS_STANDING),		true    },
    {   NULL,             0,					0       }
};

const struct flag_type portal_flags[]=
{
    {	"normal_exit",		GATE_NORMAL_EXIT,	true	},
    {	"no_curse",			GATE_NOCURSE,		true	},
    {	"go_with",			GATE_GOWITH,		true	},
    {   "buggy",			GATE_BUGGY,			true	},
    {	"random",			GATE_RANDOM,		true	},
    {	"opaque",			GATE_OPAQUE,		true	},
    {	"short_lookinto",	GATE_SHORT_LOOKINTO,true	},
    {	NULL,				0,					0		}
};

const struct flag_type token_flags[]=
{
	{	"dropdeath",		TOKEN_DROPDEATH,	true	},
	{	"quitdeath",		TOKEN_QUITDEATH,	true	},
	{	NULL,				0,					0		}
};

const struct flag_type furniture_flags[]=
{
    {  "stand_at",			STAND_AT,			true    },
    {  "stand_on",			STAND_ON,			true    },
    {  "stand_in",			STAND_IN,			true    },
	{  "stand_under",		STAND_UNDER,		true	},
    {  "sit_at",			SIT_AT,				true    },
	{  "sit_on",			SIT_ON,				true    },
    {  "sit_in",			SIT_IN,				true    },
	{  "sit_under",			SIT_UNDER,			true	},
    {  "rest_at",			REST_AT,			true    },
    {  "rest_on",			REST_ON,			true    },
    {  "rest_in",			REST_IN,			true    },
	{  "rest_under",		REST_UNDER,			true	},
    {  "sleep_at",			SLEEP_AT,			true    },
    {  "sleep_on",			SLEEP_ON,			true    },
    {  "sleep_in",			SLEEP_IN,			true    },
	{  "sleep_under",		SLEEP_UNDER,		true	},
    {  "put_at",			PUT_AT,				true    },
    {  "put_on",			PUT_ON,				true    },
    {  "put_in",			PUT_IN,				true    },
    {  "put_inside",		PUT_INSIDE,			true    },
	{  "put_under",			PUT_UNDER,			true	},
    {  "kneel_at",			KNEEL_AT,			true    },
    {  "kneel_on",			KNEEL_ON,			true    },
    {  "kneel_in",			KNEEL_IN,			true    },
	{  "kneel_under",		KNEEL_UNDER,		true	},
    {  NULL,         0,            0   }
};

const  struct  flag_type  to_types[]=
{
	{  "affects",			WHERE_AFFECTS,			true    }, 	// character
	{  "affects2",			WHERE_AFFECTS2,		true	},	// character
	{  "affects3",			WHERE_AFFECTS3,		true	},	// character
	{  "immune",			WHERE_IMMUNE,			true    },	// character
	{  "resist",			WHERE_RESIST,			true    },	// character
	{  "vuln",				WHERE_VULN,			true    },	// character
	{  "objextra",			WHERE_OBJEXTRA,		false	},	// object
	{  "objextra2",			WHERE_OBJEXTRA2,		false	},	// object
	{  "weapon",			WHERE_WEAPON,			false	},	// object
	{  "restrict",			WHERE_RESTRICT,		false   },	// no flag table
	{  "skills",			WHERE_SKILLS,			false	},	// no table
	{  "objectspell",		WHERE_OBJECTSPELL,		false	},	// no table
	{  "modifier",			WHERE_MODIFIER,		false	},	// no table
	{  NULL,				0,					true    }
};

// Kal - July 98
const struct flag_type plr_flags[] =
{
	{ "npc",			PLR_IS_NPC,			true	},
	{ "letgained",		PLR_CAN_ADVANCE,	true	},
	{ "autoassist",		PLR_AUTOASSIST,		true	},
	{ "autoexit",		PLR_AUTOEXIT,		true	},
	{ "autoloot",		PLR_AUTOLOOT,		true	},
	{ "autoreformat",	PLR_AUTOREFORMAT,	true	},
	{ "autogold",		PLR_AUTOGOLD,		true	},
	{ "autosplit",		PLR_AUTOSPLIT,		true	},
	{ "noteach",		PLR_NOTEACH,		true	},
	{ "autosubdue",		PLR_AUTOSUBDUE,		true	},
	{ "quester",		PLR_QUESTER,		true	},
	{ "holylight",		PLR_HOLYLIGHT,		true	},
	{ "holyname",		PLR_HOLYNAME,		true	},
	{ "nosummon",		PLR_NOSUMMON,		true	},
	{ "nofollow",		PLR_NOFOLLOW,		true	},
	{ "specify_self",	PLR_SPECIFY_SELF,	false},
	{ "holyvnum",		PLR_HOLYVNUM,		true	},
	{ "permit",			PLR_PERMIT,			true	},
	{ "log",			PLR_LOG,			true	},
	{ "deny",			PLR_DENY,			true	},
	{ "freeze",			PLR_FREEZE,			true	},
	{ "norp",			PLR_NORP,			true	},
	{ "noreducing_maxkarn",	PLR_NOREDUCING_MAXKARN,	true	},
	{ "can_hero",		PLR_CAN_HERO,		true	},
	{ "autopeek",		PLR_AUTOPEEK,		true	},
	{ "holywalk",		PLR_HOLYWALK,		true	},
	{ "automap",		PLR_AUTOMAP,		true	},
	{  NULL,		 0,  0	 }
};

// Kal - July 98
const struct flag_type affect_flags[] =
{
	{ "blind",			AFF_BLIND,			true	},
	{ "invisible",		AFF_INVISIBLE,		true	},
	{ "detect_evil",	AFF_DETECT_EVIL,	true	},
	{ "detect_invis",	AFF_DETECT_INVIS,	true	},
	{ "detect_magic",	AFF_DETECT_MAGIC,	true	},
	{ "detect_hidden",	AFF_DETECT_HIDDEN,	true	},
	{ "detect_good",	AFF_DETECT_GOOD,	true	},
	{ "sanctuary",		AFF_SANCTUARY,		true	},
	{ "faerie_fire",	AFF_FAERIE_FIRE,	true	},
	{ "infrared",		AFF_INFRARED,		true	},
	{ "curse",			AFF_CURSE,			true	},
	{ "otterlungs",		AFF_OTTERLUNGS,		true	},
	{ "poison",			AFF_POISON,			true	},
	{ "protect_evil",	AFF_PROTECT_EVIL,	true	},
	{ "protect_good",	AFF_PROTECT_GOOD,	true	},
	{ "sneak",			AFF_SNEAK,			true	},
	{ "hide",			AFF_HIDE,			true	},
	{ "sleep",			AFF_SLEEP,			true	},
	{ "charm",			AFF_CHARM,			true	},
	{ "flying",			AFF_FLYING,			true	},
	{ "pass_door",		AFF_PASS_DOOR,		true	},
	{ "haste",			AFF_HASTE,			true	},
	{ "calm",			AFF_CALM,			true	},
	{ "plague",			AFF_PLAGUE,			true	},
	{ "weaken",			AFF_WEAKEN,			true	},
	{ "dark_vision",	AFF_DARK_VISION,	true	},
	{ "berserk",		AFF_BERSERK,		true	},
	{ "swim",			AFF_SWIM,			true	},
	{ "regeneration",	AFF_REGENERATION,	true	},
	{ "slow",			AFF_SLOW,			true	},
	{ "fear",			AFF_FEAR,			true	},
	{  NULL,	 0,  0	 }
};

const struct flag_type affect2_flags[] = // some false to implemented?
{
	{ "possession",			AFF2_POSSESSION,			false	},// not yet implemented
	{ "rampage",			AFF2_RAMPAGE,				false	},// not yet implemented
	{ "vamp_bite",			AFF2_VAMP_BITE,				false	},// not yet implemented
	{ "ghoul",				AFF2_GHOUL,					false	},// not yet implemented
	{ "chi_power",			AFF2_CHI_POWER,				false	},// not yet implemented
	{ "fade",				AFF2_FADE,					false	},// not yet implemented
	{ "taunt",				AFF2_TAUNT,					false	},// not yet implemented
	{ "cripple",			AFF2_CRIPPLE,				false	},// not yet implemented
	{ "camouflage",			AFF2_CAMOUFLAGE,			false	},// not yet implemented
	{ "detect_camouflage",	AFF2_DETECT_CAMOUFLAGE,		false	},// not yet implemented
	{ "roar",				AFF2_ROAR,					false	},// not yet implemented
	{ "warcry",				AFF2_WARCRY,				false	},// not yet implemented
	{ "shield",				AFF2_SHIELD,				false	},// not yet implemented	
	{ "mute",				AFF2_MUTE,					false	},// not yet implemented	
	{ "treeform",			AFF2_TREEFORM,				true	},
	{ "passwotrace",		AFF2_PASSWOTRACE,			true	},
	{ "vanish",				AFF2_VANISH,				true	},
	{ "fear_magic",			AFF2_FEAR_MAGIC,			true	},
	{ "detect_treeform",	AFF2_DETECT_TREEFORM,		true	},
	{ "detect_vanish",		AFF2_DETECT_VANISH,			true	},	
	{ "fire_shield",		AFF2_FIRE_SHIELD,			false	},// not yet implemented	
	{ "ice_shield",			AFF2_ICE_SHIELD,			false	},// not yet implemented
	{ "shock_shield",		AFF2_SHOCK_SHIELD,			false	},// not yet implemented
	{ "hallucinate",		AFF2_HALLUCINATE,			false	},// not yet implemented
	{ "vicegrip",			AFF2_VICEGRIP,				true	},
	{ "stockade",			AFF2_STOCKADE,				false	}, // place holder
	{ "stone_gargoyle",		AFF2_STONE_GARGOYLE,		false	}, // place holder
	{ "mana_regeneration",	AFF2_MANA_REGEN,			true	}, // Tristan
	{  NULL,	 0,  0	 }
};

const struct flag_type affect3_flags[] =
{
	{ "det_traps",			AFF3_DET_TRAPS,				false	},// not yet implemented
	{   NULL,   0,  0   }
};


// Kal - July 98
const struct flag_type comm_flags[] =
{
	{ "coding",			COMM_CODING,		true	},
	{ "global_social_off",	COMM_GLOBAL_SOCIAL_OFF,	true},
	{ "whovis",			COMM_WHOVIS,		false	},
	{ "newbie_support",	COMM_NEWBIE_SUPPORT,false	},
	{ "noquote",		COMM_NOQUOTE,		true	},
	{ "shouts_off",		COMM_SHOUTSOFF,		true	},
	{ "spell_debug",	COMM_SPELL_DEBUG,	false	},
	{ "compact",		COMM_COMPACT,		true	},
	{ "brief",			COMM_BRIEF,			true	},	
	{ "nogprompt",		COMM_NOGPROMPT,		true	},
	{ "prompt",			COMM_PROMPT,		true	},
	{ "combine",		COMM_COMBINE,		true	},
	{ "telnet_ga",		COMM_TELNET_GA,		true	},
	{ "show_affects",	COMM_SHOW_AFFECTS,	true	},
	{ "autoself",		COMM_AUTOSELF,		true	},
	{ "announce_off",	COMM_ANNOUNCEOFF,	true	},
	{ "noemote",		COMM_NOEMOTE,		false	},
	{ "noshout",		COMM_NOSHOUT,		false	},
	{ "notell",			COMM_NOTELL,		false	},
	{ "nochannelled",	COMM_NOCHANNELS,	false	},
	{ "nochanneled",	COMM_NOCHANNELS,	false	}, // dup
	{ "nochannels",		COMM_NOCHANNELS,	false	}, // dup
	{ "building",		COMM_BUILDING,		true	},
	{ "snoop_proof",	COMM_SNOOP_PROOF,	false	},
	{ "afk",			COMM_AFK,			true	},
	{ "can_nochannel",	COMM_CANNOCHANNEL,	false	},
	{ "reduced_laston",	COMM_REDUCED_LASTON,false	},
	{ "nopray",			COMM_NOPRAY,		false	},
	{  NULL,	 0,  0	 }
};

// Kal - July 98
const  struct  flag_type wiznet_flags []  =
{
	{ "wiznet_on",	WIZ_ON,			true	},
	{ "ticks",		WIZ_TICKS,		true	},
	{ "logins",		WIZ_LOGINS,		true	},
	{ "sites",		WIZ_SITES,		true	},
	{ "links",		WIZ_LINKS,		true	},
	{ "deaths",		WIZ_DEATHS,		true	},
	{ "resets",		WIZ_RESETS,		true	},
	{ "mobdeaths",	WIZ_MOBDEATHS,	true	},
	{ "flags",		WIZ_FLAGS,		true	},
	{ "penalties",	WIZ_PENALTIES,	true	},	
	{ "thefts",		WIZ_THEFTS,		true	},
	{ "levels",		WIZ_LEVELS,		true	},
	{ "secure",		WIZ_SECURE,		true	},
	{ "switches",	WIZ_SWITCHES,	true	},
	{ "snoops",		WIZ_SNOOPS,		true	},
	{ "autoon",		WIZ_AUTOON,		true	},
	{ "load",		WIZ_LOAD,		true	},
	{ "newbie",		WIZ_NEWBIE,		true	},
	{ "showchannel",WIZ_SHOWCHANNEL,true	},
	{ "spam",		WIZ_SPAM,		true	},
	{ "rpexp",		WIZ_RPEXP,		true	},
	{ "bugs",		WIZ_BUGS,		true	},
	{ "beta",		WIZ_BETA,		true	},
	{ "player_log",	WIZ_PLAYER_LOG,	true	},
	{ "whispers",	WIZ_WHISPERS,	true	},
	{ "rpmonitor",	WIZ_RPMONITOR,	true	},
	{ "nohelp",		WIZ_NOHELP,		true	},
	{ "questing",	WIZ_QUESTING,	true	},
	{ "prayers_dreams",	WIZ_PRAYERS_DREAMS,	true	},
	{ "prayers",	WIZ_PRAYERS_DREAMS,	true	},
	{ "memcheck",	WIZ_MEMCHECK,	true	},
	{ "newbietells",WIZ_NEWBIETELL, true	},
	{  NULL,	 0,  0	 }
};


#define BAN_UNDEFINED	0
#define BAN_ALL			1	// strongest
#define BAN_PERMIT		2	// permit required
#define BAN_EMAIL_REQ	3	// email system
#define BAN_NEWBIE		4	// newbies
#define BAN_LOGNEWBIE	5	// log a newbie when the create

const struct flag_type ban_types[] =
{
    {   "undefined",	BAN_UNDEFINED,      true    },
    {   "all",			BAN_ALL,			true    },
    {   "permit",		BAN_PERMIT,			true    },
    {   "email",		BAN_EMAIL_REQ,      true    },
    {   "newbie",		BAN_NEWBIE,			true    },
    {   "lognewbie",	BAN_LOGNEWBIE,		true    },
    {   NULL,			0,          0       }
};


// Kal - July 98
const  struct  flag_type laston_flags[]  =
{
	{ "online",			LASTON_ONLINE,			true	},
	{ "on_at_reboot",	LASTON_ON_AT_REBOOT,	true	},
	{ "not_shown",		LASTON_NOT_SHOWN,		true	},
	{ "logged",			LASTON_LOGGED,			true	},
	{ "from_irc",		LASTON_IRC,				true	},
	{ "first_irc",		LASTON_FIRSTIRC,		true	},
	{ "has_used_irc",	LASTON_HASUSEDIRC,		true	},
	{ "noble",			LASTON_NOBLE,			true	},
	{ "perm_pkilled",	LASTON_PERM_PKILLED,	true	},
	{ "deleted",		LASTON_DELETED,			true	},
	{ "letgained",		LASTON_LETGAINED,		true	},
	{ "automap",		LASTON_USING_AUTOMAP,	true	},
	{ "using_msp",		LASTON_USING_MSP	 ,	true	},
	{ "active",			LASTON_ACTIVE		 ,	true	},
	{ "quester",		LASTON_QUESTER		 ,	true	},
	{ "nsupport",		LASTON_NSUPPORT		 ,	true	},
	{ "allowimmtalk",	LASTON_ALLOWIMMTALK	 ,	true	},
	{ "nomaxkarn",		LASTON_NOMAXKARN	 ,	true	},
	{ "mccp",			LASTON_MCCP,			true	},	
	{  NULL,	 0,  0	 }
};
         		
const  struct  flag_type laston_wizlist_types[]  =
{
	{ "hidden",		LASTONWIZLISTTYPE_HIDDEN,	true	},
	{ "active",		LASTONWIZLISTTYPE_ACTIVE,	true	},
	{ "guest",		LASTONWIZLISTTYPE_GUEST,	true	},
	{ "retired",	LASTONWIZLISTTYPE_RETIRED,	true	},
	{  NULL,	 0,  0	 }
};

	
// The attributes all characters have
// - The order of these is important as the table is also 
//   used for indexing the attribute names
const struct flag_type stat_flags[] =
{
    {  "strength",		STAT_ST,      true    },
    {  "quickness",		STAT_QU,      true    },
    {  "presence",		STAT_PR,      true    },
    {  "empathy",		STAT_EM,      true    },
    {  "intuition",		STAT_IN,      true    },
    {  "constitution",	STAT_CO,      true    },
    {  "agility",		STAT_AG,      true    },
	{  "selfdiscipline",STAT_SD,      true    },
	{  "memory",		STAT_ME,      true    },
	{  "reasoning",		STAT_RE,      true    },
	{  NULL,	 0,  0	 }
};

const struct flag_type realm_flags[] =
{
	{ "abjuration"		,REALM_ABJURATION	,true },
	{ "alteration"		,REALM_ALTERATION	,true },
	{ "charm"			,REALM_CHARM		,true },
	{ "conjuration"		,REALM_CONJURATION	,true },
	{ "enchantment"		,REALM_ENCHANTMENT	,true },
	{ "evocation"		,REALM_EVOCATION	,true },
	{ "essence"			,REALM_ESSENCE		,true },
	{ "foretelling"		,REALM_FORETELLING	,true },
	{ "illusion"		,REALM_ILLUSION		,true },
	{ "necromancy"		,REALM_NECROMANCY	,true },
	{ "phantasm"		,REALM_PHANTASM		,true },
	{ "summoning"		,REALM_SUMMONING	,true },
	{ "wild"			,REALM_WILD			,true },
	{  NULL,	 0,  0	 }
};

const struct flag_type sphere_flags[] =
{
	{ "body"		,SPHERE_BODY		,true },
	{ "combat"		,SPHERE_COMBAT		,true },
	{ "convocation"	,SPHERE_CONVOCATION	,true },
	{ "creation"	,SPHERE_CREATION	,true },
	{ "death"		,SPHERE_DEATH		,true },
	{ "divination"	,SPHERE_DIVINATION	,true },
	{ "elemental"	,SPHERE_ELEMENTAL	,true },
	{ "healing"		,SPHERE_HEALING		,true },
	{ "mind"		,SPHERE_MIND		,true },
	{ "nature"		,SPHERE_NATURE		,true },
	{ "protection"	,SPHERE_PROTECTION	,true },
	{ "time"		,SPHERE_TIME		,true },
	{ "weather"    	,SPHERE_WEATHER		,true },
	{  NULL,	 0,  0	 }
};

const struct flag_type element_flags[] =
{
	{ "air"			,ELEMENT_AIR		,true },
	{ "animal"		,ELEMENT_ANIMAL		,true },	
	{ "earth"		,ELEMENT_EARTH		,true },
	{ "fire"		,ELEMENT_FIRE		,true },
	{ "land"		,ELEMENT_LAND		,true },
//	{ "guardian"	,ELEMENT_LAND		,true },
	{ "moon"		,ELEMENT_MOON		,true },
	{ "plant"		,ELEMENT_PLANT		,true },
	{ "sun"			,ELEMENT_SUN		,true },
	{ "water"		,ELEMENT_WATER		,true },
	{ "autumn"		,SEASON_AUTUMN		,true },
	{ "spring"		,SEASON_SPRING		,true },
	{ "summer"		,SEASON_SUMMER		,true },
	{ "winter"   	,SEASON_WINTER		,true },
	{  NULL,	 0,  0	 }
};

const struct flag_type composition_flags[] =
{
	{ "beguiling",	COMPOSITION_BEGUILING,	true	},
	{ "ceremonial",	COMPOSITION_CEREMONIAL,	true	},
	{ "epic",		COMPOSITION_EPIC,		true	},
	{ "esoteric",	COMPOSITION_ESOTERIC,	true	},
	{ "ethereal",	COMPOSITION_ETHEREAL,	true	},
	{ "requiem",	COMPOSITION_REQUIEM,	true	},
	{ "holistic",	COMPOSITION_HOLISTIC,	true	},
	{ NULL,		0,	0	}
};

// used for categorising spells and skills
const struct flag_type category_types[] =
{
	{ "undefined",			CAT_UNDEFINED,				true },	
	{ "spdefensive",		CAT_DEFENSIVE_SPELL,		true },
	{ "spdefensive_enhancement",	CAT_DEFENSIVE_ENHANCEMENT,	true },
	{ "spoffensive_enhancement",	CAT_OFFENSIVE_ENHANCEMENT,	true },
	{ "spoffensive_spell",	CAT_OFFENSIVE_SPELL,		true },
    { "sparea",             CAT_AREASPELL,              true },
	{ "spenchantment",		CAT_ENCHANTMENT_SPELL, 		true },
	{ "sphealing",			CAT_HEALING_SPELL,			true },
	{ "spmovement",			CAT_MOVEMENT_SPELL, 		true },
    { "spcreation",         CAT_CREATION_SPELL,         true },
    { "spinformation",      CAT_INFORMATION_SPELL,      true },
	{ "spmisc",				CAT_MISCELLANEOUS_SPELL,	true },
	{ "realm",				CAT_REALM,					true },
	{ "sphere",				CAT_SPHERE, 				true },
	{ "season",				CAT_SEASON, 				true },
	{ "sklanguage",			CAT_LANGUAGE, 				true },
	{ "skweapon",			CAT_WEAPON, 				true },	
	{ "skoffensive_combat",	CAT_OFFENSIVE_COMBAT,		true },
	{ "skdefensive_combat",	CAT_DEFENSIVE_COMBAT, 		true },
	{ "skhealing",			CAT_HEALING_SKILL,			true },
	{ "skmovement",			CAT_MOVEMENT_SKILL, 		true },
	{ "skmagic",			CAT_MAGIC, 					true },
	{ "skmiscellaneous",	CAT_MISCELLANEOUS_SKILL,	true },
	{ "skdefault",			CAT_DEFAULT_SKILL,			true },
	{  NULL,		 0,  0	 }
};

// used for categorising help entries... 
struct flag_type help_category_types[MAX_HELP_CATEGORIES] =
{
	// note: the index is initialised by init_globals
	{ "undefined",			0,	true },
	{ "creation",			0,	true },
	{ "command",			0,	true },
	{ "command:immortal",	0,	true },
	{ "helpsys",			0,	true }, // documents part of the help system
	{ "newbie",				0,	true },
	{ "notes",				0,	true },
	{ "magic",				0,	true },
	{ "magic:spell",		0,	true },
	{ "mxptest",			0,	true },
	{ "olc",				0,	true },
	{ "olc:aedit",			0,	true },
	{ "olc:hedit",			0,	true },
	{ "olc:medit",			0,	true },
	{ "olc:redit",			0,	true },
	{ "olc:oedit",			0,	true },
	{ "olc:oedit:item",		0,	true },
	{ "race",				0,	true },
	{ "race:general",		0,	true },
	{ "pointer",			0,	true },
	{ "unfinished",			0,	true },
	{  NULL,				0,  0	 }
};

const struct flag_type com_category_types[] =
{
	{ "undefined",			COMCAT_UNDEFINED,			true },
	{ "auto",				COMCAT_AUTO,				true },
	{ "clan",				COMCAT_CLAN,				true },
	{ "combat",				COMCAT_COMBAT,				true },
	{ "misc",				COMCAT_MISC,				true },
	{ "movement",			COMCAT_MOVEMENT,			true },
	{ "notes",				COMCAT_NOTES,				true },
	{ "communication",		COMCAT_COMMUNICATION,		true },
	{ "statistics",			COMCAT_STATISTICS,			true },
	{ "utilities",			COMCAT_UTILITIES,			true },
	{ "nsupport",			COMCAT_NSUPPORT,			true },
	{ "rpsupport",			COMCAT_RPSUPPORT,			true },
	{ "magic",				COMCAT_MAGIC,				true },
	{ "pkrelated",			COMCAT_PKRELATED,			true },
	{ "rp",					COMCAT_RP,					true },
	{ "object",				COMCAT_OBJECT,				true },
	{ "olc",				COMCAT_OLC,					true },
	{ "noble",				COMCAT_NOBLE,				true },
	{ "helps",				COMCAT_HELPS,				true },	
	{ NULL,					0,					0		}
};


// for target types
const struct flag_type target_types[] =
{
	{"ignore",			TAR_IGNORE			,true},
	{"char_offensive",	TAR_CHAR_OFFENSIVE	,true},
	{"char_defensive",	TAR_CHAR_DEFENSIVE	,true},
	{"char_self",		TAR_CHAR_SELF		,true},
	{"obj_inv",			TAR_OBJ_INV			,true},
	{"obj_char_def",	TAR_OBJ_CHAR_DEF	,true},
	{"obj_char_off",	TAR_OBJ_CHAR_OFF	,true},
	{"mob_offensive",	TAR_MOB_OFFENSIVE	,true},
	{"obj_mob_off",		TAR_OBJ_MOB_OFF		,true},
	{"tar_direction",	TAR_DIRECTION		,true},
    { NULL,				0, 0}
};

/**************************************************************************/
const struct flag_type sktype_types[] =
{
	{"undefined",		SKTYPE_UNDEFINED,		true},
	{"spell",			SKTYPE_SPELL,			false},
	{"skill",			SKTYPE_SKILL,			true},
	{"other",			SKTYPE_OTHER,			true},
	{"realm",			SKTYPE_REALM,			true},
	{"sphere",			SKTYPE_SPHERE,			true},
	{"elementseason",	SKTYPE_ELEMENTSEASON,	true},
	
    { NULL,				0, 0}
};

/**************************************************************************/
const struct flag_type skflags_flags[] =
{
	{"never_learnt_by_leveling",	SKFLAGS_SPNEVER_LEARNT_BY_LEVELING,	true},
	{"must_be_set_to_get",			SKFLAGS_SPMUST_BE_SET_TO_GET,		true},
	{"teach_spgain",				SKFLAGS_TEACH_SPGAIN,				true},
	{"level_spgain",				SKFLAGS_LEVEL_SPGAIN,				true},
	{"study_spgain",				SKFLAGS_STUDY_SPGAIN,				true},
	{"no_pctarget",					SKFLAGS_NO_PCTARGET,				true},
	{"no_npctarget",				SKFLAGS_NO_NPCTARGET,				true},
	{"renamable",					SKFLAGS_RENAMABLE,					true},
	//renameable is an alternate spelling of renamable
	{"no_interclass_teach",			SKFLAGS_NO_INTERCLASS_TEACH,		true},
	{"new_improve_system",			SKFLAGS_NEW_IMPROVE_SYSTEM,			true},
	{"no_gain",						SKFLAGS_NO_GAIN,					true},
	{"no_negative_hp_at_affectoff",	SKFLAGS_NO_NEGATIVE_HP_AT_AFFECTOFF,true},
	{"magical_antipathy",			SKFLAGS_MAGICAL_ANTIPATHY,			true},
	{"no_scribe",					SKFLAGS_NO_SCRIBE,					true},
	{"use_race_restrictions",		SKFLAGS_USE_RACE_RESTRICTIONS,		false},	
	{"no_prac",						SKFLAGS_NO_PRAC,					true},		
	{ NULL,				0, 0}
};

/**************************************************************************/
const struct flag_type dynspell_flags[] =
{
	{"no_custom_text_within_function",	SPFUNC_NOTEXT,		true},
	{"damtype_not_applicable",			SPFUNC_NODAMTYPE,	true},
	{"damtype_used",					SPFUNC_DAMTYPE,		true},
    { NULL,				0, 0}
};


/**************************************************************************/
const struct sector_type sect_table[] =
{
	{ "inside"			},
	{ "city"			},
	{ "field"			},
	{ "forest"			},
	{ "hills"			},
    { "mountain"		},
	{ "water_swim"		},
	{ "water_noswim"	},
	{ "swamp"			},
	{ "air"				},
	{ "desert"			},
	{ "cave"			},
	{ "underwater"		},
	{ "snow"			},
	{ "ice"				},
	{ "trail"			},
	{ "lava"			},
	{ NULL				}
};

/**************************************************************************/
const	struct flag_type		traptype_flags[]	=
{
	{	"move",			TRAP_TRIG_MOVE,		false	},
	{	"object",		TRAP_TRIG_OBJECT,	false	},
	{	"room",			TRAP_TRIG_ROOM,		false	},
	{	"north",		TRAP_TRIG_NORTH,	false	},
	{	"east",			TRAP_TRIG_EAST,		false	},
	{	"south",		TRAP_TRIG_SOUTH,	false	},
	{	"west",			TRAP_TRIG_WEST,		false	},
	{	"up",			TRAP_TRIG_UP,		false	},
	{	"down",			TRAP_TRIG_DOWN,		false	},
	{	"northeast",	TRAP_TRIG_NORTHEAST,false	},
	{	"northwest",	TRAP_TRIG_NORTHWEST,false	},
	{	"southeast",	TRAP_TRIG_SOUTHEAST,false	},
	{	"southwest",	TRAP_TRIG_SOUTHWEST,false	},
	{	"open",			TRAP_TRIG_OPEN,		false	},
	{	NULL,			0,					0		}
};

/**************************************************************************/
const struct flag_type damtype_types[] =
{
	{"none",			DAM_NONE,		true},
	{"bash",			DAM_BASH,		true},
	{"pierce",			DAM_PIERCE,		true},
	{"slash",			DAM_SLASH,		true},
	{"fire",			DAM_FIRE,		true},
	{"cold",			DAM_COLD,		true},
	{"lightning",		DAM_LIGHTNING,	true},
	{"acid",			DAM_ACID,		true},
	{"poison",			DAM_POISON,		true},
	{"negative ",		DAM_NEGATIVE,	true},
	{"holy",			DAM_HOLY,		true},
	{"energy",			DAM_ENERGY,		true},
	{"mental",			DAM_MENTAL,		true},
	{"disease",			DAM_DISEASE,	true},
	{"drowning",		DAM_DROWNING,	true},
	{"light",			DAM_LIGHT,		true},
	{"other",			DAM_OTHER,		true},
	{"harm",			DAM_HARM,		true},
	{"charm",			DAM_CHARM,		true},
	{"sound",			DAM_SOUND,		true},
	{"illusion",		DAM_ILLUSION,	true},
    { NULL,				0, 0}
};

/**************************************************************************/
// tables used only in here
const struct flag_type letgain_db_flags[] =
{
	{  "letgain_requested", LETGAIN_REQUESTED,  false   },
	{  "letgain_granted",	LETGAIN_GRANTED,	false	},
	{  "letgain_declined",	LETGAIN_DECLINED,	false	},
	{  "letgain_pending",	LETGAIN_PENDING,	false	},
	{   NULL,       0,      true    }
};
/**************************************************************************/
const struct flag_type notenet_flags[] =
{
	{  "on",				NOTE_ON,				false   },
	{  "noblepkill",		NOTE_NOBLEPKILL,		false	},
	{  "immpkill",			NOTE_IMMPKILL,			false	},
	{  "immpkilldetails",	NOTE_IMMPKILLDETAILS,	false	},
	{  "clannotes",			NOTE_CLANNOTES,			false	},
	{  "to_pkill",			NOTE_TO_PKILL,			false	},
	{  "to_allclan",		NOTE_TO_ALLCLAN,		false	},
	{  "admin_rename",		NOTE_ADMIN_RENAME,		false	},
	{  "court",				NOTE_COURT,				false	},
	{   NULL,       0,      true    }
};
/**************************************************************************/
const struct flag_type config_flags[] =
{
	{  "showmisc",			CONFIG_SHOWMISC,			false   },
	{  "nomisc",			CONFIG_NOMISC,				false	},
	{  "objrestrict",		CONFIG_OBJRESTRICT,			false	},
	{  "autorecall",		CONFIG_AUTORECALL,			false	},	
	{  "note_only_to_imm",	CONFIG_NOTE_ONLY_TO_IMM,	false	},
	{  "autoexamine",		CONFIG_AUTOEXAMINE,			false	},
	{  "autolandonrest",	CONFIG_AUTOLANDONREST,		false	},
	{  "autolandonreset",	CONFIG_AUTOLANDONREST,		false	},
	{  "courtmember",		CONFIG_COURTMEMBER,			false	},
	{  "bard_council",		CONFIG_BARD_COUNCIL,		false	},
	{  "holyspeech",		CONFIG_HOLYSPEECH,			false	},
	{  "disallow_pkill",	CONFIG_DISALLOWED_PKILL,	false	},
	{  "ignore_multilogins",CONFIG_IGNORE_MULTILOGINS,	false	},
	{  "rpsupport",			CONFIG_RP_SUPPORT,			false	},
	{  "metric",			CONFIG_METRIC,				false	},
	{  "active",			CONFIG_ACTIVE,				false	},
	{  "autotrack",			CONFIG_AUTOTRACK,			false	},	
	{  "automap",			CONFIG_AUTOMAP,				false	},		
	{  "autowraptells",		CONFIG_AUTOWRAPTELLS,		false	},			
	{  "pracsys_tester",	CONFIG_PRACSYS_TESTER,		false	},				
	{  "nocharm",			CONFIG_NOCHARM,				false	},				
	{  "noheromsg",			CONFIG_NOHEROMSG,			false	},
	{  "nonewbie",			CONFIG_NONEWBIE,			false	},
	{  "nonames",			CONFIG_NONAMES,				false	},	
	{  "noautoanswer",		CONFIG_NOAUTOANSWER,		false	},		
	{  "hide_hidden_areas",	CONFIG_HIDE_HIDDEN_AREAS,	false	},		
	{  "names_before_short",CONFIG_NAMES_BEFORE_SHORT,	false	},
	{   NULL,       0,      true    }
};
/**************************************************************************/
const struct flag_type config2_flags[] =
{
	{  "read_builder_legal",	CONFIG2_READ_BUILDER_LEGAL,	false },
	{  "noautosaymote",			CONFIG2_NOAUTOSAYMOTE,		false },
	{  "autosaycolourcodes",	CONFIG2_AUTOSAYCOLOURCODES,	false },
	{  "name_only_for_known",	CONFIG2_NAME_ONLY_FOR_KNOWN,false },
	{  "autodamage",			CONFIG2_AUTODAMAGE,			false },	
	{  "nopkill",				CONFIG2_NOPKILL,			false },	
	{  "full_exits",			CONFIG2_FULL_EXITS,			false },	
	{  "no_detect_oldstyle_note_writing",	CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING, false },
	{  "nomapexits",			CONFIG2_NOMAPEXITS,			false },
	{  "nomapblanks",			CONFIG2_NOMAPBLANKS,		false },
	{  "autowizilogin",			CONFIG2_AUTOWIZILOGIN,		false },
	{  "autowhoinvislogin",		CONFIG2_AUTOWHOINVISLOGIN,	false },
	{  "no_battlelag_prompt",		CONFIG2_NO_BATTLELAG_PROMPT,	false },
	{  "accurate_laston_times",		CONFIG2_ACCURATE_LASTON_TIMES,	false },
	{  "highimmortal_laston_access",CONFIG2_HIGHIMMORTAL_LASTON_ACCESS,	false },
	{  "autokeepalive",			CONFIG2_AUTOKEEPALIVE,	false },
	{  "mp_trigger_in_room",	CONFIG2_MP_TRIGGER_IN_ROOM,	false },
		
	{   NULL,       0,      true    }
};
/**************************************************************************/
const struct flag_type pconfig_flags[] =
{
	{  "filler",	PCONFIG_FILLER,	false},
	{   NULL,       0,      true    }
};
/**************************************************************************/
const struct flag_type help_flags[] =
{
	{  "INCOMPLETE",		HELP_INCOMPLETE,			true	},
	{  "hide_keywords",		HELP_HIDE_KEYWORDS,			true	},	
	{  "noble",				HELP_NOBLE,					true	},
	{  "nsupport",			HELP_NSUPPORT,				true	},
	{  "builder",			HELP_BUILDER,				true	},
	{  "rpsupport",			HELP_RPSUPPORT,				true	},
	{  "redirection_entry",	HELP_REDIRECTION_ENTRY,		true	},	
	{  "mud_specific",		HELP_MUD_SPECIFIC,			true	},		
	{  "mud_should_customize",	HELP_MUD_SHOULD_CUSTOMIZE,true	},			
	{  "hide_prevnext",		HELP_HIDE_PREVNEXT,			true	},		
	{  "hide_header_footer",HELP_HIDE_HEADER_FOOTER,	true	},		
	{  "wordwrapped",		HELP_WORDWRAPPED,			false	},
	{  "removehelp",		HELP_REMOVEHELP,			false	},	
	{  "display_mxp_double",HELP_DISPLAY_MXP_DOUBLE,	false	},	
	{   NULL,				0,							false	}
};
/**************************************************************************/
const struct flag_type council_flags[] =
{
	{  "admin",			COUNCIL_ADMIN,				true	},
	{  "balance",		COUNCIL_BALANCE,			true	},
	{  "clan",			COUNCIL_CLAN,				true	},
	{  "code",			COUNCIL_CODE,				true	},
	{  "law",			COUNCIL_LAW,				true	},
	{  "mythos",		COUNCIL_MYTHOS,				true	},
	{  "storyline",		COUNCIL_MYTHOS,				true	},
	{  "realm",			COUNCIL_REALM,				true	},
	{  "rp",			COUNCIL_RP,					true	},
	{  "roleplaying",	COUNCIL_RP,					true	},
	{  "support",		COUNCIL_SUPPORT,			true	},
	{  "headbalance",	COUNCIL_HEADBALANCE,		true	},
	{  "headclan",		COUNCIL_HEADCLAN,			true	},
	{  "headcode",		COUNCIL_HEADCODE,			true	},
	{  "headlaw",		COUNCIL_HEADLAW,			true	},
	{  "headmythos",	COUNCIL_HEADMYTHOS,			true	},
	{  "headrealm",		COUNCIL_HEADREALM,			true	},
	{  "headrp",		COUNCIL_HEADRP,				true	},
	{  "headsupport",	COUNCIL_HEADSUPPORT,		true	},
	{  "headstoryline",	COUNCIL_HEADSTORYLINE,		true	},
	{   NULL,			0,							false	}
};
/**************************************************************************/
const struct flag_type commandlog_types[] =
{
	{	"always",		LOG_ALWAYS,					true	},
	{	"palways",		LOG_PALWAYS,				true	},
	{	"olc",			LOG_OLC,					true	},
	{	"normal",		LOG_NORMAL,					true	},
	{	"plogonly",		LOG_PLOGONLY,				true	},
	{	"nolog",		LOG_DONT_LOG,				true	},
	{	"never",		LOG_NEVER,					true	},
	{	NULL,			0,							false	}
};
/**************************************************************************/
const struct flag_type commandflag_flags[] =
{
	{	"ooc",			CMD_OOC,					true	},
	{	"ic",			CMD_IC,						true	},
	{	"noorder",		CMD_NO_ORDER,				true	},
	{	"nowiznetsecure",CMD_NO_WIZNET_SECURE,		true	},
	{	"notreeform",	CMD_NO_TREEFORM,			true	},
	{	"no_stone_gargoyle",CMD_NO_STONE_GARGOYLE,	false	},
	{	"no_stockade",	CMD_NO_STOCKADE,			false	},
	{	NULL,			0,							false	}
};

/**************************************************************************/
const struct flag_type grantgroup_flags[] =
{
	{ "mortal",			GRANTGROUP_MORTAL,				true},
	{ "noble",			GRANTGROUP_NOBLE,				true},
	{ "builder",		GRANTGROUP_BUILDER,				true},
	{ "implementor",	GRANTGROUP_IMPLEMENTOR,			true},
	{ "highadmin",		GRANTGROUP_HIGHADMIN,			true},
	{ "admin",			GRANTGROUP_ADMIN,				true},
	{ "headbalance",	GRANTGROUP_HEADBALANCE,			true},
	{ "headcode",		GRANTGROUP_HEADCODE,			true},
	{ "headlaw",		GRANTGROUP_HEADLAW,				true},
	{ "headmythos",		GRANTGROUP_HEADMYTHOS,			true},
	{ "headrealm",		GRANTGROUP_HEADREALM,			true},
	{ "headsupport",	GRANTGROUP_HEADSUPPORT,			true},
	{ "headroleplay",	GRANTGROUP_HEADROLEPLAY,		true},
	{ "balance",		GRANTGROUP_BALANCE,				true},
	{ "code",			GRANTGROUP_CODE,				true},
	{ "law",			GRANTGROUP_LAW,					true},
	{ "mythos",			GRANTGROUP_MYTHOS,				true},
	{ "realm",			GRANTGROUP_REALM,				true},
	{ "support",		GRANTGROUP_SUPPORT,				true},
	{ "roleplay",		GRANTGROUP_ROLEPLAY,			true},
	{ "advancedbuilder", GRANTGROUP_ADVANCEDBUILDER,	true},
	{ "development",	GRANTGROUP_DEVELOPMENT,			true},
	{ "experimental",	GRANTGROUP_EXPERIMENTAL,		true},
	{ "betatest1",		GRANTGROUP_BETATEST1,			true},
	{ "betatest2",		GRANTGROUP_BETATEST2,			true},
	{ "incomplete",		GRANTGROUP_INCOMPLETE,			true},
	{ "immortal",		GRANTGROUP_IMMORTAL,			true},
	{ "obscureimmortal",GRANTGROUP_OBSCUREIMMORTAL,		true}, // for obscure commands	
	{	NULL, 0,	false }
};

/**************************************************************************/
const struct flag_type sectorbit_flags[] = //For sect restrict, enhance, dampen
{
	{	"inside",		SECTBIT_INSIDE,				true	},
	{	"city",			SECTBIT_CITY,				true	},
	{	"field",		SECTBIT_FIELD, 				true	},
	{	"forest",		SECTBIT_FOREST,				true	},
	{	"hills",		SECTBIT_HILLS, 				true	},
	{	"mountain",		SECTBIT_MOUNTAIN,			true	},
	{	"swim",			SECTBIT_WATER_SWIM,			true	},
	{	"noswim",		SECTBIT_WATER_NOSWIM,		true	},
    {   "swamp",		SECTBIT_SWAMP,				true    },
	{	"air",			SECTBIT_AIR,				true	},
	{	"desert",		SECTBIT_DESERT,				true	},
	{	"cave",			SECTBIT_CAVE,				true	},
	{	"underwater",	SECTBIT_UNDERWATER,			true	},
	{   "snow",			SECTBIT_SNOW,				true	},
	{   "ice",			SECTBIT_ICE,				true	},
	{   "trail",		SECTBIT_TRAIL,				true	},
	{   "lava",			SECTBIT_LAVA,				true	},
	{	NULL,		0,					0	}
};
/**************************************************************************/
const struct flag_type sector_desc[] =
{
	{	"inside",				SECT_INSIDE,		false	},
	{	"in the city", 			SECT_CITY,			false	},
	{	"in the field",			SECT_FIELD, 		false	},
	{	"in the forest",		SECT_FOREST,		false	},
	{	"in the hills",			SECT_HILLS, 		false	},
	{	"on a mountain",		SECT_MOUNTAIN,		false	},
	{	"in shallow waters", 	SECT_WATER_SWIM,	false	},
	{	"in deep waters",		SECT_WATER_NOSWIM,	false	},
    {   "in a swamp",			SECT_SWAMP,         false   },
	{	"in the air",			SECT_AIR,			false	},
	{	"in the desert",		SECT_DESERT,		false	},
	{	"in a cave",			SECT_CAVE,			false	},
	{	"underwater",			SECT_UNDERWATER,	false	},
	{   "in the snow",			SECT_SNOW,			false	},
	{   "on the ice",			SECT_ICE,			false	},
	{   "on the trail",			SECT_TRAIL,			false   },
	{   "over the lava",		SECT_LAVA,			false   },
	{	NULL,		0,					0	}
};
/**************************************************************************/
const struct flag_type spell_group_flags[] =
{
	{ "skins",					SPELL_GROUP_SKINS,		true},
	{ "fire_shield",			SPELL_GROUP_FIRE_SHIELD,true},
	{ "ice_shield",				SPELL_GROUP_ICE_SHIELD, true},
	{ "wind_shield",			SPELL_GROUP_WIND_SHIELD,true},
	{ "strength",				SPELL_GROUP_STRENGTH,	true},
	{ "blesses",				SPELL_GROUP_BLESSES,	true},
	{ "mental",					SPELL_GROUP_MENTAL,		true},
	{  NULL,	 0,  0	 }
};
/**************************************************************************/
const struct flag_type immhelp_types[] =
{
	{ "free",		IMMHELP_FREE,		true},
	{ "possible",	IMMHELP_POSSIBLE,	true},
	{ "closed",		IMMHELP_CLOSED,		true},
	{ "undefined",	IMMHELP_UNDEFINED,	true},
	{ NULL,0,0}
};

const struct flag_type align_flags[] = 
{
	{ "none",		ALIGN_NONE,			false},
	{ "evil",		ALIGN_EVIL,			true},
	{ "neutral",	ALIGN_NEUTRAL,		true},
	{ "good",		ALIGN_GOOD,			true},
	{ NULL,			0,					0}
};

const struct flag_type tendency_flags[] =
{
	{ "none",		TENDFLAG_NONE,			false},
	{ "chaotic",	TENDFLAG_CHAOTIC,		true},
	{ "neutral",	TENDFLAG_NEUTRAL,		true},
	{ "lawful",		TENDFLAG_LAWFUL,		true},
	{ NULL,			0,					0}
};

/**************************************************************************/
const struct flag_type dedit_flags[] =
{
/*
	{ "none",				DEDIT_NONE,				true},
	{ "goodalign",			DEDIT_GOOD_ALIGN,		true},
	{ "neutralalign",		DEDIT_NEUTRAL_ALIGN,	true},
	{ "evilalign",			DEDIT_EVIL_ALIGN,		true},
	{ "ordertendency",		DEDIT_ORDER_TENDENCY,	true},
	{ "neutraltendency",	DEDIT_NEUTRAL_TENDENCY,	true},
	{ "chaotictendency",	DEDIT_CHAOTIC_TENDENCY,	true},
	{ "malegender",			DEDIT_MALE_GENDER,		true},
	{ "femalegender",		DEDIT_FEMALE_GENDER,	true},
	{ "neutralgender",		DEDIT_NEUTRAL_GENDER,	true}, */
	{ NULL,					0,						0   }
};
/**************************************************************************/
const struct flag_type buildrestrict_types[] =
{
	{ "all",			BUILDRESTRICT_ALL,		true},
	{ "area",			BUILDRESTRICT_AREA,		true},
	{ "exits",			BUILDRESTRICT_EXITS,	true},
	{ "mudprogs",		BUILDRESTRICT_MUDPROGS,	true},
	{ "mobs",			BUILDRESTRICT_MOBS,		true},
	{ "objects",		BUILDRESTRICT_OBJECTS,	true},
	{ "resets",			BUILDRESTRICT_RESETS,	true},
	{ "rooms",			BUILDRESTRICT_ROOMS,	true},
	{ "other",			BUILDRESTRICT_OTHER,	true},
	{ NULL, 0, 0 }
};

/**************************************************************************/
const struct flag_type classflag_flags[] =
{
	{ "poison_immunity",		CLASSFLAG_POISON_IMMUNITY,			true},
	{ "curse_immunity",			CLASSFLAG_CURSE_IMMUNITY,			true},
	{ "plague_immunity",		CLASSFLAG_PLAGUE_IMMUNITY,			true},
	{ "dammods_with_holyweapons",CLASSFLAG_DAMMODS_WITH_HOLYWEAPONS,true},		
	{ "safe_flee_from_combat",	CLASSFLAG_SAFE_FLEE_FROM_COMBAT,	true},		
	{ "casting_affected_by_moon",CLASSFLAG_CASTING_AFFECTED_BY_MOON,true},		
	{ "casting_halfaffected_by_moon",CLASSFLAG_CASTING_HALFAFFECTED_BY_MOON,true},		
	{ "magic_antipathy",		CLASSFLAG_MAGIC_ANTIPATHY,			true},		
	{ "level_based_ac",			CLASSFLAG_LEVEL_BASED_AC,			true},
	{ "totems",					CLASSFLAG_TOTEMS,					true},
	{ "can_collect_water",		CLASSFLAG_CAN_COLLECT_WATER,		true},
	{ "deities",				CLASSFLAG_DEITIES,					true},
	{ "holy symbols",			CLASSFLAG_HOLYSYMBOLS,				true},
	{ "no_customization",		CLASSFLAG_NO_CUSTOMIZATION,			true},	
	{ "always_hidden_from_mortal_classinfo",CLASSFLAG_ALWAYS_HIDDEN_FROM_MORTAL_CLASSINFO,			true},	
	{ "hidden_from_mortal_classinfo_when_above_their_remort",CLASSFLAG_HIDDEN_FROM_MORTAL_CLASSINFO_WHEN_ABOVE_THEIR_REMORT,true},	
	{ "holy_class",				CLASSFLAG_HOLY_CLASS,				true},
	
	{ NULL, 0, 0 }
};
/**************************************************************************/
struct flag_type classnames_flags[MAX_CLASS+1] =
{
	{ NULL, 0, 0 }
};
/**************************************************************************/
const struct flag_type castcommand_types[CCT_MAXCAST+1]=
{
	{ "none",		CCT_NONE,	true},
	{ "cast",		CCT_MAGE,	true},
	{ "commune",	CCT_CLERIC, true},
	{ "summon",		CCT_DRUID,	true},
	{ "sing",		CCT_BARD,	true},
	{ NULL, 0, 0 }
};
/**************************************************************************/
const struct flag_type castnames_types[CCT_MAXCAST+1]=
{
	{ "none",		CCT_NONE,	true},
	{ "cast",		CCT_MAGE,	true},
	{ "commune",	CCT_CLERIC, true},
	{ "conjurer",	CCT_DRUID,	true},
	{ "sing",		CCT_BARD,	true},
	{ NULL, 0, 0 }
};

/**************************************************************************/
const struct flag_type language_flags[] =
{
	{ "changed",			LANGFLAG_CHANGED,			false},
	{ "system_language",	LANGFLAG_SYSTEM_LANGUAGE,	false}, // autogenerated - not saved to disk
	{ "no_scramble",		LANGFLAG_NO_SCRAMBLE,		true },
	{ "no_holyspeech",		LANGFLAG_NO_HOLYSPEECH,		true },
	{ "no_order",			LANGFLAG_NO_ORDER,			true },
	{ "reverse_text",		LANGFLAG_REVERSE_TEXT,		true },
	{ "no_language_name",	LANGFLAG_NO_LANGUAGE_NAME,	true },
	{ "no_skill_required",	LANGFLAG_NO_SKILL_REQUIRED,	true },
	{ "no_command_access",	LANGFLAG_NO_COMMAND_ACCESS,	true },	
	{ "immonly",			LANGFLAG_IMMONLY,			true },	
	{ "scramble_in_ooc",	LANGFLAG_SCRAMBLE_IN_OOC,	true },		
	{ NULL, 0, 0 }
};

/**************************************************************************/
const struct flag_type attune_flags[] =
{
	{ "attunable",				ATTUNE_NEED_TO_USE,					true },
	{ "equal_level",			ATTUNE_EQUAL_LEVEL,					true },
	{ "vanish",					ATTUNE_VANISH,						true },
	{ "trivial",				ATTUNE_TRIVIAL,						true },
	{ "easy",					ATTUNE_EASY,						true },
	{ "hard",					ATTUNE_HARD,						true },
	{ "infuriating",			ATTUNE_INFURIATING,					true },
	{ "previous",				ATTUNE_PREVIOUS,					true },
	{ "once",					ATTUNE_ONCE_ONLY,					true },
	{ NULL, 0, 0 }
};


/**************************************************************************/
const	struct flag_type		mixtype_types[]	=
{
	{	"none",				MIXTYPE_NONE,		true	},
	{	"herbalism",		MIXTYPE_HERBALISM,	true	},
	{	"cooking",			MIXTYPE_COOKING,	true	},
	{	"smithing",			MIXTYPE_SMITHING,	true	},
	{	"alchemy",			MIXTYPE_ALCHEMY,	true	},
	{	"pottery",			MIXTYPE_POTTERY,	true	},
	{	"tinkering",		MIXTYPE_TINKERING,	true	},
	{	NULL,				0,					0		}
};
/**************************************************************************/
const	struct flag_type	colourmode_types[]	=
{
	{	"nocolour",			CT_NOCOLOUR,	true	},
	{	"ansi",				CT_ANSI,		true	},
	{	"irc",				CT_IRC,			true	},
	{	"ircwhite",			CT_IRCWHITE,	true	},
	{	"html",				CT_HTML,		true	},
	{	"mxp",				CT_MXP,			true	},
	{	"bright",			CT_BRIGHT,		true	},
	{	"autodetect",		CT_AUTODETECT,	true	},	
	{	NULL,				0,				0		}
};
/**************************************************************************/
const	struct flag_type	preference_types[]	=
{
	{	"off",				PREF_OFF,		true	},
	{	"autosense",		PREF_AUTOSENSE,	true	},
	{	"on",				PREF_ON,		true	},
	{	NULL,				0,				0		}
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
