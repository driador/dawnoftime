/**************************************************************************/
// const.cpp - constant tables (well were all constant once upon a time)
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
#include "magic.h"
#include "interp.h"
#include "d2magsys.h"

#define IMO LEVEL_IMMORTAL

char * const he_she  [] = { "it",  "he",  "she" };
char * const him_her [] = { "it",  "him", "her" };
char * const his_her [] = { "its", "his", "her" };

/***************************************************************************/
/* item type list */
const struct item_type item_table [] =
{
	{ 0, ""},
	{ ITEM_LIGHT,		"light"			},
	{ ITEM_SCROLL,		"scroll"		},
	{ ITEM_WAND,		"wand"			},
	{ ITEM_STAFF,		"staff"			},
	{ ITEM_WEAPON,		"weapon"		},	/* 5 OLC */
	{ ITEM_COMPONENT,	"component"		},	// Component
	{ ITEM_RP,			"rp"			},
	{ ITEM_TREASURE,	"treasure"		},	/* 8 OLC */
	{ ITEM_ARMOR,		"armor"			},
	{ ITEM_POTION,		"potion"		},
	{ ITEM_CLOTHING,	"clothing"		},
	{ ITEM_FURNITURE,	"furniture"		},
	{ ITEM_TRASH,		"trash"			},	/* 13 OLC */
	{ ITEM_PARCHMENT,	"parchment"		},
	{ ITEM_CONTAINER,	"container"		},
	{ ITEM_INSTRUMENT,	"instrument"	},
	{ ITEM_DRINK_CON,	"drink"			},	/* 17 OLC */
	{ ITEM_KEY,			"key"			},
	{ ITEM_FOOD,		"food"			},
	{ ITEM_MONEY,		"money"			},	/* 20 OLC */
	{ ITEM_HERB,		"herb"			},
	{ ITEM_BOAT,		"boat"			},	/* 22 OLC */
	{ ITEM_CORPSE_NPC,	"npc_corpse"	},
	{ ITEM_CORPSE_PC,	"pc_corpse"		},
	{ ITEM_FOUNTAIN,	"fountain"		},
	{ ITEM_PILL,		"pill"			},
	{ ITEM_PROTECT,		"protect"		},
	{ ITEM_MAP,			"map"			},
	{ ITEM_PORTAL,		"portal"		},
	{ ITEM_WARP_STONE,	"warp_stone"	},
	{ ITEM_ROOM_KEY,	"room_key"		},
	{ ITEM_GEM,			"gem"			},
	{ ITEM_JEWELRY,		"jewelry"		},
	{ ITEM_JUKEBOX,		"jukebox"		},
	{ ITEM_TOKEN,		"token"			},
	{ ITEM_POULTICE,	"poultice"		},
	{ ITEM_MORTAR,		"mortar"		},
	{ ITEM_CAULDRON,	"cauldron"		},
	{ ITEM_FLASK,		"flask"			},
	{ ITEM_ORE,			"ore"			},
	{ ITEM_SHEATH,		"sheath"		},
	{ 0,         NULL					}
};

/***************************************************************************/
game_settings_type tgs2;
#define GETBYTE_OFFSET(field) (int)(((char*) &(tgs2.field)) \
		-((char*) &tgs2)) 
/***************************************************************************/
/* weapon selection table */
const   struct  weapon_type     weapon_table    []      =
{
	{ "exotic",		-1, 0, NULL },
	{ "sword",      GETBYTE_OFFSET(obj_vnum_outfit_sword),  WEAPON_SWORD,	&gsn_sword		},
	{ "dagger",     GETBYTE_OFFSET(obj_vnum_outfit_dagger), WEAPON_DAGGER,	&gsn_dagger		},
	{ "staff",      GETBYTE_OFFSET(obj_vnum_outfit_staff),  WEAPON_STAFF,	&gsn_staff		},
	{ "mace",       GETBYTE_OFFSET(obj_vnum_outfit_mace),   WEAPON_MACE,	&gsn_mace		},
	{ "axe",        GETBYTE_OFFSET(obj_vnum_outfit_axe),    WEAPON_AXE,		&gsn_axe		},
	{ "flail",      GETBYTE_OFFSET(obj_vnum_outfit_flail),  WEAPON_FLAIL,	&gsn_flail		},
	{ "whip",       GETBYTE_OFFSET(obj_vnum_outfit_whip),   WEAPON_WHIP,	&gsn_whip		},
	{ "polearm",	GETBYTE_OFFSET(obj_vnum_outfit_polearm),WEAPON_POLEARM,	&gsn_polearm	},
	{ "sickle",		GETBYTE_OFFSET(obj_vnum_outfit_sickle),	WEAPON_SICKLE,	&gsn_sickle		},
	{ "spear",		GETBYTE_OFFSET(obj_vnum_outfit_spear),	WEAPON_SPEAR,	&gsn_spear		},	
	{ NULL,			0,						0,				NULL			}
};

/***************************************************************************/
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type  wiznet_table[]={
	{	"on",           WIZ_ON,         IM },
	{   "beta",         WIZ_BETA,       L6 },
	{   "bugs",         WIZ_BUGS,       L6 },
	{   "deaths",       WIZ_DEATHS,     IM },
	{   "flags",        WIZ_FLAGS,      L5 },
	{   "levels",       WIZ_LEVELS,     IM },
	{   "links",        WIZ_LINKS,      L7 },
	{   "load",         WIZ_LOAD,       L2 },
	{   "logins",		WIZ_LOGINS,     IM },
	{   "mobdeaths",	WIZ_MOBDEATHS,  L4 },
    {   "memcheck",     WIZ_MEMCHECK,   L2 },
	{   "newbies",      WIZ_NEWBIE,     IM },
	{   "nohelp",       WIZ_NOHELP,     IM },
	{   "penalties",	WIZ_PENALTIES,  L5 },
	{   "playerlog",	WIZ_PLAYER_LOG, L1 },
	{   "prayers/dreams",WIZ_PRAYERS_DREAMS,IM },
	{   "questing",     WIZ_QUESTING,   IM },
	{   "resets",       WIZ_RESETS,     L4 },
	{   "autoon",       WIZ_AUTOON,     IM },
	{   "rpexp",        WIZ_RPEXP,      IM },
	{   "rpmonitor",	WIZ_RPMONITOR,  IM },
	{   "secure",       WIZ_SECURE,     L2 },
	{   "sites",        WIZ_SITES,      L4 },
	{   "showchannel",  WIZ_SHOWCHANNEL,IM },
	{   "snoops",       WIZ_SNOOPS,     L2 },
	{   "spam",         WIZ_SPAM,       L5 },
	{   "switches",     WIZ_SWITCHES,   L2 },
    {   "thefts",       WIZ_THEFTS,     IM },
	{   "ticks",        WIZ_TICKS,      IM },
	{   "whispers",     WIZ_WHISPERS,   IM },
	{	"newbietells",	WIZ_NEWBIETELL, IM },
	{   NULL,           0,              0  }
};

/***************************************************************************/
/* attack table  -- not very organized :( */
const   struct attack_type      attack_table    []              =
{
    {   "none",         "hit",          -1              },  //  0 
	{	"slice",        "slice",        DAM_SLASH       },
    {   "stab",         "stab",         DAM_PIERCE      },
    {   "slash",        "slash",        DAM_SLASH       },
    {   "whip",         "whip",         DAM_SLASH       },
    {   "claw",         "claw",         DAM_SLASH       },  //  5 
	{	"blast",        "blast",        DAM_BASH        },
    {   "pound",        "pound",        DAM_BASH        },
    {   "crush",        "crush",        DAM_BASH        },
    {   "grep",         "grep",         DAM_SLASH       },
	{	"bite",         "bite",         DAM_PIERCE      },  // 10 
    {   "pierce",       "pierce",       DAM_PIERCE      },
    {   "suction",      "suction",      DAM_BASH        },
    {   "beating",      "beating",      DAM_BASH        },
    {   "digestion",    "digestion",    DAM_ACID        },
    {   "charge",       "charge",       DAM_BASH        },  // 15 
    {   "slap",         "slap",         DAM_BASH        },
    {   "punch",        "punch",        DAM_BASH        },
    {   "wrath",        "wrath",        DAM_ENERGY      },
	{	"magic",        "magic",        DAM_ENERGY      },
    {	"divine",       "divine power", DAM_HOLY        },  // 20 
    {   "cleave",       "cleave",       DAM_SLASH       },
	{	"scratch",      "scratch",      DAM_PIERCE      },
    {   "peck",         "peck",         DAM_PIERCE      },
    {   "peckb",        "peck",         DAM_BASH        },
    {   "chop",         "chop",         DAM_SLASH       },  // 25 
    {   "sting",        "sting",        DAM_PIERCE      },
	{   "smash",		"smash",		DAM_BASH        },
    {   "shbite",       "shocking bite",DAM_LIGHTNING   },
    {   "flbite",       "flaming bite", DAM_FIRE        },
    {   "frbite",       "freezing bite",DAM_COLD		},  // 30 
    {   "acbite",       "acidic bite",  DAM_ACID        },
    {   "chomp",        "chomp",        DAM_PIERCE      },
    {   "drain",        "life drain",   DAM_NEGATIVE    },
	{   "thrust",		"thrust",       DAM_PIERCE      },
    {   "slime",        "slime",        DAM_ACID        },  // 35
    {   "shock",        "shock",        DAM_LIGHTNING   },
    {   "thwack",       "thwack",       DAM_BASH        },
	{   "flame",		"flame",        DAM_FIRE        },
	{   "chill",		"chill",        DAM_COLD        },
	{	"gunshot",		"gunshot",		DAM_PIERCE		},  // 40
	{	"radiance",		"radiance",		DAM_LIGHT		},  // 41
	{	"phantasm",		"phantasm",		DAM_ILLUSION	},  // 42
	{	"mental",		"mental blast",	DAM_MENTAL		},  // 43
	// if you add any here, make sure to increase MAX_DAMAGE_MESSAGE 
	// at the top of fight.cpp - Kal, Jan 2004
	{	NULL,			NULL,			0				}
};

/***************************************************************************/
//  Totems
const struct totem_type		totem_table		[TOTEM_MAX+1] =
{
	{ TOTEM_BEAR,		"bear"		},
	{ TOTEM_STAG,		"stag"		},
	{ TOTEM_SWALLOW,	"swallow"	},
	{ TOTEM_OSPREY,		"osprey"	},
	{ TOTEM_TORTOISE,	"tortoise"	},
	{ TOTEM_RAVEN,		"raven"		},
	{ TOTEM_BAT,		"bat"		},
	{ TOTEM_WOLF,		"wolf"		},
	{ TOTEM_SERPENT,	"serpent"	},
	{ TOTEM_TOAD,		"toad"		},
	{ TOTEM_OWL,		"owl"		},
	{ TOTEM_HARE,		"hare"		}
};

/***************************************************************************/
struct classgroup_type classgroup_table[31]=
{
	{ "allmage",		0, "All classes with any mage aspect",		
		{A | F,0,0,0,0}, "mage spellfilcher"},
	{ "allcleric",		1, "All classes with any cleric aspect",		
		{B,0,0,0,0},"cleric paladin"},
	{ "allthief",		2, "All classes with any thief aspect",		
		{C | I,0,0,0,0},"thief spellfilcher"},
	{ "allwarrior",	3, "All classes with any warrior aspect",	
		{D,0,0,0,0},"warrior paladin ranger"},
	{ "alldruid",		4, "All classes with any druid aspect",		
		{E,0,0,0,0},"druid ranger"},
	{ "coremage",		0, "All classes with mage as a core",		
		{A | F,0,0,0,0}, "mage"},
	{ "corecleric",		1, "All classes with cleric as a core",		
		{B,0,0,0,0},"cleric"},
	{ "corethief",		2, "All classes with thief as a core",		
		{C | I,0,0,0,0},"thief spellfilcher"},
	{ "corewarrior",	3, "All classes with warrior as a core",	
		{D,0,0,0,0},"warrior paladin ranger"},
	{ "coredruid",		4, "All classes with druid as a core",		
		{E,0,0,0,0},"druid"},
	{ "corecivilian",	5, "All classes with civilian as a core",	
		{J,0,0,0,0},""},
    { "",				0, ""}
};

/*
struct  objrestrict_affect_type
{
	char *	name; 
	char *	description; 
	int	flags;
	char *	wear_message; 
	sh_int	wear_location; 	
	sh_int	wear_amount;
	char *	//forced_drop_message; 
	char *	remove_message;
	sh_int	move_chance; // how often the move affect will happen - percentage
	sh_int	move_location;
	sh_int	move_amount;
	char *	move_message;
	sh_int	tick_chance; // how often the tick affect will happen - percentage
	sh_int	tick_location;
	sh_int	tick_amount;
	char *	tick_message;	

};
*/

/***************************************************************************/
struct affectprofile_type affectprofile_table[50]=
{
	
	{
		"mage-armour",	//name
		"Affects when a mage puts on armour",  //description
		0,	//flags
		"You feel a dampening sensation as you put $p on.", //wear_message
		APPLY_MANA, //wear_location
		-300,  //wear_amount
		"",  //forced_drop_message
		"The dampening sensation from $p lifts as you remove it.",	//remove_message
		// == movement == //
		0, //chance
		APPLY_NONE, //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE, //location
		0, //amount
		"", //message
	},
	
	{
		"cleric-armour",  //name
		"Affects when a cleric puts on armour",  //description
		0,	//flags
		"You are overwhelmed by an emptiness as you put on $p.",  //wear_message
		APPLY_MANA, //wear_location
		-300,  //wear_amount
		"",  //forced_drop_message
		"The emptiness is lifted as you remove $p.",  //remove_message
		// == movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"" //message
	},
		
	{
		"druid-armour",  //name
		"Affects when a druid puts on armour",  //description
		0,	//flags
		"You feel out of touch with nature as you put $p on.", //wear_message
		APPLY_MANA, //wear_location
		-300,  //wear_amount
		"",  //forced_drop_message
		"You feel more in touch with nature as you remove $p on.", //remove_message
		// == movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},
			
			
	{
		"thief-armour",  //name
		"Affects when a thief puts on armour",  //description
		0,	//flags
		"Your movements are encumbered as you put on $p.", //wear_message
		APPLY_QU, //wear_location
		-50,  //wear_amount
		"",  ////forced_drop_message
		"Your movements are less encumbered as you remove $p.", //remove_message
		// == movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},
				
	{
		"warrior-armour",  //name
		"Affects when a warrior puts on armour",	//description
		0,	//flags
		"Your battle insticts are blinded by an intense aura as you put $p on.", //wear_message
		APPLY_AC, //wear_location
		-100,  //wear_amount
		"",  //forced_drop_message
		"Your feel more intune to your battle insticts as you remove $p.", //remove_message
		// == movement == //					
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},		
										
	{
		"mage-weapon-sword", //name
		"Affects when a mage wields an sword", //description
		0, //flags
		"You feel as though $p is draining your energy as you wield it.", //wear_message
		APPLY_MANA, //wear_apply
		-300, //wear_amount
		"", ////forced_drop_message
		"The draining sensation from $p appears to have been subdued.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	{
		"mage-weapon-axe", //name
		"Affects when a mage wields an axe", //description
		0, //flags
		"You can barely lift $p over your head.", //wear_message
		APPLY_ST, //wear_apply
		-100, //wear_amount
		"", ////forced_drop_message
		"Your movements are less burdened as you stop wielding $p.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	{
		"mage-weapon-polearm", //name
		"Affects when a mage wields an polearm", //description
		0, //flags
		"You can barely lift $p over your head.", //wear_message
		APPLY_ST, //wear_apply
		-100, //wear_amount
		"", ////forced_drop_message
		"Your movements are less burdened as you stop wielding $p.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	{
		"mage-weapon-flail", //name
		"Affects when a mage wields an flail", //description
		0, //flags
		"You feel as though $p is draining your energy as you wield it.", //wear_message
		APPLY_MANA, //wear_apply
		-300, //wear_amount
		"", ////forced_drop_message
		"The draining sensation from $p appears to have been subdued.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	{
		"mage-weapon-club", //name
		"Affects when a mage wields an club", //description
		0, //flags
		"You feel as though $p is draining your energy as you wield it.", //wear_message
		APPLY_MANA, //wear_apply
		-300, //wear_amount
		"", ////forced_drop_message
		"The draining sensation from $p appears to have been subdued.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	{
		"druid-weapon-nonnatural", //name
		"Affects when a druid wields an non-natural weapon", //description
		0, //flags
		"The stink of $p brings on a nauseating feeling.", //wear_message
		APPLY_MANA, //wear_apply
		-300, //wear_amount
		"", ////forced_drop_message
		"The nauseating feeling from $p appears to have subdued.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	{
		"thief-weapon-polearm", //name
		"Affects when a thief wields an polearm", //description
		0, //flags
		"You can barely lift $p over your head.", //wear_message
		APPLY_ST, //wear_apply
		-100, //wear_amount
		"", ////forced_drop_message
		"Your movements are less burdened as you stop wielding $p.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	{
		"thief-weapon-flail", //name
		"Affects when a thief wields an flail", //description
		0, //flags
		"You can barely lift $p over your head.", //wear_message
		APPLY_ST, //wear_apply
		-100, //wear_amount
		"", ////forced_drop_message
		"Your movements are less burdened as you stop wielding $p.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	{
		"thief-weapon-sword", //name
		"Affects when a thief wields an sword", //description
		0, //flags
		"Your movements are encumbered as you wield $p.", //wear_message
		APPLY_QU, //wear_apply
		-100, //wear_amount
		"", ////forced_drop_message
		"Your movements are less encumbered as you stop wielding $p.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},


	{
		"thief-weapon-axe", //name
		"Affects when a thief wields an axe", //description
		0, //flags
		"Your movements are encumbered as you wield $p.", //wear_message
		APPLY_QU, //wear_apply
		-100, //wear_amount
		"", ////forced_drop_message
		"Your movements are less encumbered as you stop wielding $p.", //remove_message
		//	== movement == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
		//	== ticks == //
		0, //chance
		APPLY_NONE,  //location
		0, //amount
		"", //message
	},

	// MARK END OF TABLE
	{""}
};
/*	{
"mage-armour",	// name
"Affect when mages put on armour", // description
0,	// flags
"You feel a dampening sensation as you put $p on.", // wear_message
APPLY_MANA,		// wear_location
-300,			// wear_amount
"",				// //forced_drop_message
"The dampening sensation from $p lifts as you remove it.",// remove_message
// == movement == //
0, //chance
0, // location
0, // amount
"",// message
// ==- ticks -== //
0, //chance
0, // location
0, // amount
""// message
},

  {
		"mage-weapon-sword",	// name
		"Affect when mages wields a sword", // description
		0,	// flags
		"You feel as though $p is draining your energy as you wield it.", // wear_message
		APPLY_MANA,		// wear_location
		-300,			// wear_amount
		"",				// //forced_drop_message
		"The draining sensation from $p appears to have been subdued.",// remove_message
		// == movement == 
		0, //chance
		0, // location
		0, // amount
		"",// message
		// ==- ticks -==
		0, //chance
		0, // location
		0, // amount
		""// message
		},
	*/





/***************************************************************************/
/*
 * Class table.
 */
struct  class_type      class_table     [MAX_CLASS+1]     =
{
	{
	    NULL, "", {0, 0}
	}
};

/***************************************************************************/
/*
 * Liquid properties.
 * Used in world.obj.
 */
const   struct  liq_type        liq_table       []      =
{
//	  name					color		proof, full, thirst, food, ssize
	{ "water",				"clear",		{   0, 1, 10, 0, 16 }	},
	{ "beer",				"amber",		{  12, 1,  8, 1, 12 }   },
	{ "red wine",			"burgundy",		{  30, 1,  8, 1,  5 }   },
	{ "ale",				"brown",		{  15, 1,  8, 1, 12 }   },
	{ "dark ale",			"dark",			{  16, 1,  8, 1, 12 }   },

	{ "whisky",				"golden",		{ 120, 1,  5, 0,  2 }   },
	{ "lemonade",			"pink",			{   0, 1,  9, 2, 12 }   },
	{ "firebreather",		"boiling",		{ 190, 0,  4, 0,  2 }   },
	{ "local specialty",	"clear",		{ 151, 1,  3, 0,  2 }   },
	{ "slime mold juice",	"green",		{   0, 2, -8, 1,  2 }   },

	{ "milk",				"white",		{   0, 2,  9, 3, 12 }   },
	{ "tea",				"tan",			{   0, 1,  8, 0,  6 }   },
	{ "coffee",				"black",		{   0, 1,  8, 0,  6 }   },
	{ "blood",				"red",			{   0, 2, -1, 2,  6 }   },
	{ "salt water",			"clear",		{   0, 1, -2, 0,  1 }   },

	{ "coke",				"brown",		{   0, 2,  9, 2, 12 }   },
	{ "root beer",			"brown",		{   0, 2,  9, 2, 12 }   },
	{ "elvish wine",		"green",		{  35, 2,  8, 1,  5 }   },
	{ "white wine",			"golden",		{  28, 1,  8, 1,  5 }   },
	{ "champagne",			"golden",		{  32, 1,  8, 1,  5 }   },

	{ "mead",				"honey-coloured",{  34, 2,  8, 2, 12 }  },
	{ "rose wine",			"pink",			{  26, 1,  8, 1,  5 }   },
	{ "benedictine wine",	"burgundy",		{  40, 1,  8, 1,  5 }   },
	{ "vodka",				"clear",		{ 130, 1,  5, 0,  2 }   },
	{ "cranberry juice",	"red",			{   0, 1,  9, 2, 12 }   },

	{ "orange juice",		"orange",		{   0, 2,  9, 3, 12 }   }, 
	{ "absinthe",			"green",		{ 200, 1,  4, 0,  2 }   },
	{ "brandy",				"golden",		{  80, 1,  5, 0,  4 }   },
	{ "aquavit",			"clear",		{ 140, 1,  5, 0,  2 }   },
	{ "schnapps",			"clear",		{  90, 1,  5, 0,  2 }   },

	{ "icewine",			"purple",		{  50, 2,  6, 1,  5 }   },
	{ "amontillado",		"burgundy",		{  35, 2,  8, 1,  5 }   },
	{ "sherry",				"red",			{  38, 2,  7, 1,  5 }   },
	{ "framboise",			"red",			{  50, 1,  7, 1,  5 }   },
	{ "rum",				"amber",		{ 151, 1, 10, 0,  2 }   },

	{ "cordial",			"clear",		{ 100, 1,  5, 0,  2 }   },
	{ "martini",			"clear",		{ 100, 1,  5, 0,  2 }   },
	{ "bloodwine",			"vermillion",	{  80, 1,  5, 0,  4 }   },
	{ "pineapple juice",	"yellow",		{	0, 0,  8, 1, 12 }	},
	{ "fruit punch",		"red",			{	0, 1,  9, 0, 12 }	},
	{ "applewine",			"tea-coloured", {  30, 1,  8, 1,  5 }   },
	{ "plumwine",			"dark-purple",	{  30, 1,  8, 1,  5 }   },
	{ "mulled wine",		"purple",		{  30, 1,  8, 1,  5 }   },
	{ "honey wine",			"golden",		{  34, 2,  8, 2, 12 }	},
	{ "apple cider",		"golden",		{  26, 1,  8, 1,  5 }   },
	{ "porter",				"green",		{  35, 2,  8, 1,  5 }   },
	{ "spiced tea",			"crimson",		{   0, 1, 10, 0,  6 }   },
	{ "green tea",			"green",		{   0, 0,  9, 0,  6 }   },
	{ "apple juice",		"golden",		{ 	0, 0,  8, 1, 12 }	},
	{ "grape juice",		"purple",		{ 	0, 0,  8, 1, 12 }	},
	{ "watermelon juice",	"pink",			{ 	0, 0,  8, 1, 12 }	},
	{ "starfruit juice",	"yellow",		{ 	0, 0,  8, 1, 12 }	},
	{ "untainted water",	"crystal-clear",{	0, 1, 12, 0, 16 }	},
	{ NULL,					NULL,			{   0, 0,  0, 0,  0 }   }
};


/***************************************************************************/
/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)     n
#define RACE_R(n)   n
#define S_FLAGS(n)  n

class skill_type  skill_table[MAX_SKILL];

/***************************************************************************/
struct  skillgroup_type  skillgroup_table[MAX_SKILLGROUP+1]     =
{
	{NULL}
};
/***************************************************************************/
struct  group_type      group_table     [MAX_GROUP_TABLE+1]     =
{
	// NOTE: YOU SHOULD NOT BE EDITING THIS, INSTEAD DO ANY CHANGES
	//       USING THE OLC BASED SKILL GROUP EDITOR!
	 {
	"dawn basics",           { 0, 0, 0, 0,0,0,0,0,0 },
	{ "recall" }
	 },

	 {
	"mage basics",          { 0, -1, -1, -1,-1,-1,-1,-1,-1 },
	{ "dagger","scrolls", "wands" }
	 },

	 {
	"cleric basics",        { -1, 0, -1, -1, -1,-1,-1,-1,-1 },
	{ "mace", "scrolls", "staves" }
	 },

	 {
	"thief basics",         { -1, -1, 0, -1,-1,-1,-1,-1,-1 },
	{ "dagger", "steal" }
	 },

	 {
	"warrior basics",       { -1, -1, -1, 0,-1,-1,-1,-1,-1 },
	{ "sword" }
	 },

 	{
	"druid basics",        { -1, -1, -1, -1,0,-1,-1,-1,-1 },
	{ "mace", "scrolls"}
	 },

	{
	"paladin basics",        { -1, -1, -1, -1,-1,0,-1,-1,-1 },
	{"sword", "staves", "scrolls"}
	 },

	 {
	"ranger basics",        { -1, -1, -1, -1,-1,-1,0,-1,-1 },
	 {"sword", "awareness"}
	 },


	 {
	 "barbarian basics", {-1,-1,-1,-1,-1,-1,-1,0,-1},
	 {"berserk", "bash", "axe"}
	 },
  
     {
     "spellfilcher basics", {-1,-1,-1,-1,-1,-1,-1,-1,0},
     {"dagger", "lore", "steal", "scrolls", "wands"}
     },

	 {
     "mage default",     { 40, -1, -1, -1,-1,-1,-1,-1,-1 }, // 41 points worth
	 {"scan", "essence", "alteration", "evocation", "dodge", "conjuration", 
	 "necromancy", "conjuration", "summoning"}
     },
		 

	 {
     "cleric default",   { -1, 40, -1, -1,-1,-1,-1,-1,-1 }, // 42 cp for 40
     { "scan", "convocation", "protection", "healing", "death", 
	 "meditation", "combat"}
	 },

	 {
     "thief default",    { -1, -1, 40, -1,-1,-1,-1,-1,-1 }, 
     { "mace", "dirt kicking", "backstab", "disarm", "dodge", 
	 "second attack", "hide", "peek", "pick lock", "sneak" }
	 },

	 {
     "warrior default",  { -1, -1, -1, 40,-1,-1,-1,-1,-1 },
     { "weaponsmaster", "shield block", "bash", "disarm", "enhanced damage",
	  "parry", "rescue", "third attack" }
     },

	 {
     "ranger default",   { -1, -1, -1, -1,-1,-1,40,-1,-1 }, // 41 for 40
	 {"scan", "sneak", "build", "awareness", "staff", "axe", //17
		"animal training", "domesticate",  //4
		"land", "water", "plant","animal"} // 20
	 },

	 {
     "druid default",    { -1, -1, -1, -1,40,-1,-1,-1,-1 }, // 44 for 
     { "scan", "hand to hand", "animal training", //9
	 "summer", "winter", "autumn", "spring", // 20
     "plant", "animal", "land"	}, // 15
	 },

	 {
     "paladin default",  { -1, -1, -1, -1,-1,40,-1,-1,-1 }, // 41 for 40
	{ "scan", "second attack", "third attack",  //11
	"lay on hands", "shield block" // 5
	"convocation", "healing", "death", "combat", "protection"} // 25
	},

	 {
	 "barbarian default", {-1,-1,-1,-1,-1,-1,-1,40,-1},
	 { "weaponsmaster", "enhanced damage", "dodge", "fast healing",
	 "ultra damage", "second attack", "scan", "hand to hand" }
	 },


     {
     "spellfilcher default", {-1,-1,-1,-1,-1,-1,-1,-1,40}, // 42 for 40
		{ "scan", "backstab", "second attack", // 12
		"dirt kicking",  "sneak", "hide", "peek"//15		 
		 "evocation", "conjuration", "summoning"} //15
	 },
    
	 {
     "weaponsmaster",    { -1, -1, -1, 15,-1,18,20,10,-1 },
     { "axe", "dagger", "flail", "mace", "polearm", "staff", "sword","whip" },
	 },

	 { NULL}
};

/***************************************************************************/
//
// for use of a more IC time system, in 24 hour format
const struct timefield_type timefield_table [] =
{
//	  TYPE				Lowhour		Highour		Name
	{ TIME_NONE,			-1,		-1,			"none"				},
	{ TIME_SMALL_HOURS,		 1,		 4,			"during the small hours of the night"	},
	{ TIME_DAWN,			 5,		 5,			"at dawn"			},
	{ TIME_MORNING,			 6,		11,			"in the morning"	},
	{ TIME_NOON,			12,		12,			"at noon"			},
	{ TIME_AFTERNOON,		13,		18,			"in the afternoon"	},
	{ TIME_DUSK,			19,		19,			"at dusk"			},
	{ TIME_EVENING,			20,		23,			"in the evening"	},
	{ TIME_MIDNIGHT,		 0,		 0,			"at midnight"		},
	{ 0,					-1,		-1,			NULL				}
};

/***************************************************************************/
// Difficulty settings for skills and such things
const struct modifier_type modifier_table [] =
{
	{ DIFF_NONE,			"none",				0	},
	{ DIFF_TRIVIAL,			"trivial",			30	},
	{ DIFF_EASY,			"easy",				20	},
	{ DIFF_LIGHT,			"light",			10	},
	{ DIFF_MEDIUM,			"medium",			0	},
	{ DIFF_HARD,			"hard",				-10 },
	{ DIFF_VERY_HARD,		"very_hard",		-25 },
	{ DIFF_ABSURD,			"absurd",			-50 },
	{ DIFF_SHEER_FOLLY,		"sheer_folly",		-75 },
	{ 0,					NULL,				0	}
};

/***************************************************************************/
/***************************************************************************/


