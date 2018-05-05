/**************************************************************************/
// cust_col.h - Dawn custom colour system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef CUST_COL_H
#define CUST_COL_H

// the order of this list can be whatever you like, as the system
// saves enum names to disk... loading will be faster if the order
// of enum CUSTOM_COLOUR_TYPE is the same as custom_colour_table[]
enum CUSTOM_COLOUR_TYPE
{
	CC_DEFAULT1,
	CC_DEFAULT2,
	CC_DEFAULT3,
	CC_DEFAULT4,
	CC_DEFAULT5,
	CC_DEFAULT6,
	CC_DEFAULT7,
	CC_DEFAULT8,
	CC_DEFAULT9,
	CC_DEFAULT0,
	
	CC_CHANNEL_ANSWER,
	CC_CHANNEL_ADMINTALK_TEXT,
	CC_CHANNEL_ADMINTALK_PREFIX,
	CC_CHANNEL_IC,
	CC_CHANNEL_OOC,
	CC_CHANNEL_MYSTERY_IMM,
	CC_CHANNEL_QUESTION,
	CC_CHANNEL_HIGHADMINTALK_TEXT,
	CC_CHANNEL_HIGHADMINTALK_PREFIX,
	CC_CHANNEL_IMMTALK_TEXT,
	CC_CHANNEL_IMMTALK_PREFIX,
	CC_CHANNEL_NOBLETALK_TEXT,
	CC_CHANNEL_NOBLETALK_PREFIX,
	CC_CHANNEL_NEWBIE,		
	CC_CHANNEL_NEWBIE_PREFIX,
	CC_CHANNEL_TELL,
	CC_CHANNEL_REPLY,
	CC_CHANNEL_CLANTALK_TEXT,
	CC_CHANNEL_CLANTALK_PREFIX,
	CC_CREATION_TITLEBAR,
	CC_CREATION_TITLEBAR_TEXT,

	CC_GAIN_XP,
	CC_GAIN_RPS,
	CC_GAIN_LEVEL,

	CC_HOLYVNUM_PLAYER_LEVEL,
	CC_HOLYVNUM_MOB,
	CC_HOLYVNUM_MOB_WITH_PROG,

	CC_HELP_KEYWORDS,
	CC_HELP_LINK,
	CC_HELP_BROKENLINK,
	CC_HELP_SYNTAX,
	CC_HELP_SEEALSO,
	CC_HELP_DEFAULT,
	CC_HELP_TITLE,
	CC_HELP_PREVNEXT_LINK,
	CC_HELP_PREVNEXT_NOLINK,
	CC_HELP_HEADINGS,
	CC_HELP_BAR,

	CC_PROMPT_COMBAT,
	CC_PROMPT_DEFAULT,
	CC_PROMPT_OLC,
	CC_PROMPT_SWITCHEDPREFIX,
	CC_PROMPT_SWITCHEDHOLYSPEECH,

	CC_SCORE_BORDER,
	CC_SCORE_INNER_BORDER,
	CC_SCORE_NAME_REMORT, 
	CC_SCORE_LETGAINED,

	CC_SHOWCOL_CUSTOM,
	CC_SHOWCOL_TEMPLATE,

	CC_SOCKETS_BRACKET,
	CC_SOCKETS_NUMBER,
	CC_SOCKETS_STATE_PLAYING,
	CC_SOCKETS_STATE_OTHER,
	CC_SOCKETS_LOGIN_TIME,
	CC_SOCKETS_IDLE,
	CC_SOCKETS_NAME,
	CC_SOCKETS_HOST,
	CC_SOCKETS_HOSTMULTI,
	CC_SOCKETS_SYSTIME,
	CC_SOCKETS_BAR,
	CC_SOCKETS_IDENT,
	CC_SOCKETS_NONSTANDARDIDENT,

	CC_TITLEBAR_DEFAULT_TEXT,
	CC_TITLEBAR_DEFAULT_BAR,

	CC_OLC_LABELS,
	CC_OLC_FLAGS,
	CC_OLC_SET_FLAG,
	CC_OLC_UNSET_FLAG,
	CC_OLC_UNSETTABLE_FLAG,
	CC_OLC_TEXT,
	CC_OLC_OPTIONS,
	CC_OLC_SELECTED_OPTION,
	CC_OLC_VALUES,
	CC_OLC_WARNING,
	CC_OLC_COMMANDS,
	CC_OLC_HINTS,
	CC_OLC_MXP_HELP_LINKS,
	CC_OLC_MXP_CLICKABLE_HELPER,
	
	CC_WHO_ADMIN_NOCHANNELED,
	CC_WHO_ADMIN_LOGGED,	
	CC_WHO_BRACKET_NORMAL, 
	CC_WHO_BRACKET_IRC,		
	CC_WHO_BRACKET_IMMORTAL,
	CC_WHO_STARS_MORTAL, 
	CC_WHO_STARS_IMM,	
	CC_WHO_AFK_QUIET,	
	CC_WHO_LINKDEAD,	
	CC_WHO_IMM_NOT_WIZI, 
	CC_WHO_IMM_IWIZI,
	CC_WHO_IMM_OWIZI,	
	CC_WHO_IMM_WHOINVIS,	
	CC_WHO_IMM_IMMTALKHASH,	
	CC_WHO_IMM_ACTIVE,		
	CC_WHO_IMM_QBC,			
	CC_WHO_IMM_QBC_BRACKETS,
	
	CC_ROOM_OOC,		
	CC_ROOM_OLC,		
	CC_ROOM_NAME,		
	CC_ROOM_DESCRIPT,	
	CC_ROOM_LOCKERS,
	CC_ROOM_EXITS,		
	CC_ROOM_FULLEXITS,
	CC_ROOM_MOBS,		

	CC_OBJECT_CAN_WEAR,
	CC_OBJECT_CANT_WEAR,
	CC_OBJECT_CANT_WEAR2,

	CC_COMMAND_EXAMPLE_SYNTAX,
	CC_INFO_HEADER,
	CC_INFO_TEXT,

	CC_MISC_RETURN_TO_CONTINUE,
	CC_MISC_EQUIPMENT_LISTS,

	CC_SCORE_LEVEL,
	CC_SCORE_RACE_GENDER_CLASS,
	CC_SCORE_LABELS,
	CC_SCORE_ATTRIBUTES,
	CC_SCORE_BORN,
	CC_SCORE_AGE,
	CC_SCORE_PRAC_TRAIN,
	CC_SCORE_XP_RPS,
	CC_SCORE_XPTNL,
	CC_SCORE_CURRENTVALUE,
	CC_SCORE_MAXVALUE,
	CC_SCORE_ALIGNMENT,
	CC_SCORE_AC,
	CC_SCORE_GOLD,
	CC_SCORE_SILVER,
	CC_SCORE_FORWARD_SLASH,
	CC_SCORE_IMMORTAL_VALUES,
	CC_SCORE_SHOW_AFFECTS_TITLE,
	CC_SCORE_SHOW_AFFECTS_TEXT,

	CC_RP_SAYLINE,
	CC_RP_EMOTELINE,

	CC_MISC_EQUIPMENT_LISTS_NONE,

	CC_MAX
};

#define MAX_CUSTOM_COLOUR (CC_MAX)

struct COLOUR_TEMPLATE_TYPE
{
	COLOUR_TEMPLATE_TYPE *next;
	char *template_name;
	char *template_description;
	char *template_colour;
	int template_protected_by_level; // disallows editing
};

struct custom_colour_table_type{
	CUSTOM_COLOUR_TYPE cc_code;
	char *cc_code_text;
	unsigned char custom_colour_code;
	int flags;
	char* category;
	char* description;
};


extern unsigned int custom_colour_index[256];
extern COLOUR_TEMPLATE_TYPE dawn_colour_scheme;
extern custom_colour_table_type custom_colour_table[];

COLOUR_TEMPLATE_TYPE *find_colour_template(const char *template_name);
#define colour_scheme_default_name "Default"
#define colour_scheme_default_descript "Default Colour Scheme"
extern COLOUR_TEMPLATE_TYPE *default_colour_template;

#endif

