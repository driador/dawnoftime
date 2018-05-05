/**************************************************************************/
// cust_col.cpp - Dawn custom colour system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "colour.h"
#include "cust_col.h"
#include "olc_ex.h"
#include "channels.h"

COLOUR_TEMPLATE_TYPE *colour_template_list;
COLOUR_TEMPLATE_TYPE *default_colour_template;
extern colour_codes colourTable[256];
/**************************************************************************/
// local prototypes
void fwrite_custom_colours(FILE* fp, const char *header, const char custom_colours[]);
char *fread_custom_colours(FILE* fp, bool player);
/**************************************************************************/
// write the template colours to disk
void colourtemplate_write_templatecolour(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	COLOUR_TEMPLATE_TYPE * pct;
	pct= (COLOUR_TEMPLATE_TYPE *) data;
	fwrite_custom_colours(fp, gio_table[tableIndex].heading, pct->template_colour);
}
/**************************************************************************/
// read a templates colours from disk
void colourtemplate_read_templatecolour(gio_type *, int, void *data, FILE *fp)
{
	COLOUR_TEMPLATE_TYPE * pct;
	pct= (COLOUR_TEMPLATE_TYPE *) data;
	pct->template_colour=fread_custom_colours(fp, false);

	for(int i=0; i<MAX_CUSTOM_COLOUR; i++){
		if(pct->template_colour[i]=='.'){
			pct->template_colour[i]='0';
		}
	}

}
/**************************************************************************/
// create race_type_old GIO lookup table 
GIO_START(COLOUR_TEMPLATE_TYPE)
GIO_STR(template_name)
GIO_STR(template_description)
GIO_INT_WITH_DEFAULT(template_protected_by_level, LEVEL_IMMORTAL)
GIO_CUSTOM_WRITEH(template_colour,	"template_colour ", colourtemplate_write_templatecolour)
GIO_CUSTOM_READH(template_colour,	"template_colour ", colourtemplate_read_templatecolour)
GIO_FINISH_STRDUP_EMPTY 


/**************************************************************************/
// custom_colour_index[] is used to relate which custom colour codes are 
// stored in which position for a descriptors custom_colour[] buffer.
unsigned int custom_colour_index[256];

#define CCWRAP(cc_code) cc_code,#cc_code

// custom colour flags
#define CCF_IMM			(A) 
#define CCF_ADMIN		(B) 
#define CCF_HIGHADMIN	(C) 
#define CCF_NOBLE		(D)
#define CCF_SOCK		(E) 


/*struct custom_colour_table_type{
	CUSTOM_COLOUR_TYPE cc_code;
	char *cc_code_text;
	unsigned char custom_colour_code;
	int flags;
	char* category;
	char* description;
};
*/
// the order here can be whatever you like
custom_colour_table_type custom_colour_table[]={
	{CCWRAP(CC_DEFAULT1),					'1', 0,	"Default","Default Colour 1"}, 
	{CCWRAP(CC_DEFAULT2),					'2', 0,	"Default","Default Colour 2"}, 
	{CCWRAP(CC_DEFAULT3),					'3', 0,	"Default","Default Colour 3"}, 
	{CCWRAP(CC_DEFAULT4),					'4', 0,	"Default","Default Colour 4"}, 
	{CCWRAP(CC_DEFAULT5),					'5', 0,	"Default","Default Colour 5"}, 
	{CCWRAP(CC_DEFAULT6),					'6', 0,	"Default","Default Colour 6"}, 
	{CCWRAP(CC_DEFAULT7),					'7', 0,	"Default","Default Colour 7"}, 
	{CCWRAP(CC_DEFAULT8),					'8', 0,	"Default","Default Colour 8"}, 
	{CCWRAP(CC_DEFAULT9),					'9', 0,	"Default","Default Colour 9"}, 
	{CCWRAP(CC_DEFAULT0),					'0', 0,	"Default","Default Colour 0"}, 
	{CCWRAP(CC_CHANNEL_ANSWER),				'A', 0,			"Channel","Answer"},		
	{CCWRAP(CC_CHANNEL_ADMINTALK_TEXT),		'y', CCF_ADMIN,	"Channel","AdminTalk Text"},
	{CCWRAP(CC_CHANNEL_ADMINTALK_PREFIX),	'Y', CCF_ADMIN,"Channel","AdminTalk Prefix"},
	{CCWRAP(CC_CHANNEL_IC),					'W', 0,			"Channel","IC"},			
	{CCWRAP(CC_CHANNEL_OOC),				'O', 0,			"Channel","OOC"},			
	{CCWRAP(CC_CHANNEL_MYSTERY_IMM),		'o', CCF_IMM,	"Channel","Mystery Imm"},
	{CCWRAP(CC_CHANNEL_QUESTION),			'Q', 0,			"Channel","Question"},
	{CCWRAP(CC_CHANNEL_HIGHADMINTALK_TEXT),	'z', CCF_HIGHADMIN,	"Channel","HighAdminTalk Text"},
	{CCWRAP(CC_CHANNEL_HIGHADMINTALK_PREFIX),'Z',CCF_HIGHADMIN,"Channel","HighAdminTalk Prefix"},
	{CCWRAP(CC_CHANNEL_IMMTALK_TEXT),		'i', CCF_IMM,	"Channel","ImmTalk Text"},
	{CCWRAP(CC_CHANNEL_IMMTALK_PREFIX),		'I', CCF_IMM,	"Channel","ImmTalk Prefix"},
	{CCWRAP(CC_CHANNEL_NEWBIE),				'e', CCF_ADMIN,	"Channel","NewbieTalk Text"},
	{CCWRAP(CC_CHANNEL_NEWBIE_PREFIX),		'E', CCF_ADMIN,	"Channel","NewbieTalk Prefix"},
	{CCWRAP(CC_CHANNEL_NOBLETALK_TEXT),		'n', CCF_NOBLE,	"Channel","NobleTalk Text"},		
	{CCWRAP(CC_CHANNEL_NOBLETALK_PREFIX),	'N',CCF_NOBLE,	"Channel","NobleTalk Prefix"},
	{CCWRAP(CC_CHANNEL_CLANTALK_TEXT),		'\x99', CCF_NOBLE,	"Channel","ClanTalk Text"},		
	{CCWRAP(CC_CHANNEL_CLANTALK_PREFIX),	'\x9a',CCF_NOBLE,	"Channel","ClanTalk Prefix"},		
	{CCWRAP(CC_CHANNEL_REPLY),				'M',0,	"Channel","Reply"},
	{CCWRAP(CC_CHANNEL_TELL),				'm',0,	"Channel","Tell"},
	{CCWRAP(CC_COMMAND_EXAMPLE_SYNTAX),		'C',0,	"Command","Example Syntax"},
	{CCWRAP(CC_INFO_HEADER),				'\x8a',0,	"Info","Info Header"},
	{CCWRAP(CC_INFO_TEXT),					'\x8b',0,	"Info","Info Text"},
	{CCWRAP(CC_MISC_RETURN_TO_CONTINUE),	'\xaa',0,	"Misc","Return to Continue"},
	{CCWRAP(CC_MISC_EQUIPMENT_LISTS),		'\xab',0,	"Misc","Equipment Lists"},
	{CCWRAP(CC_MISC_EQUIPMENT_LISTS_NONE),	'\xc5',0,	"Misc","Equipment Lists - None"},
	
	{CCWRAP(CC_CREATION_TITLEBAR_TEXT),		'c', CCF_ADMIN,	"Creation","Titlebar Text"},
	{CCWRAP(CC_CREATION_TITLEBAR),			'j', CCF_ADMIN,	"Creation","Titlebar Lines"},

	{CCWRAP(CC_GAIN_XP),					'\xa7', 0,"Gain","XP"}, 
	{CCWRAP(CC_GAIN_RPS),					'\xa8', 0,"Gain","RPS"}, 
	{CCWRAP(CC_GAIN_LEVEL),					'\xa9', 0,"Gain","Level"}, 

	{CCWRAP(CC_HOLYVNUM_PLAYER_LEVEL),		'L', CCF_IMM,	"Holyvnum","Player Level"},
	{CCWRAP(CC_HOLYVNUM_MOB),				'h', CCF_IMM,	"Holyvnum","Mob Vnum & Level"},
	{CCWRAP(CC_HOLYVNUM_MOB_WITH_PROG),		'H', CCF_IMM,	"Holyvnum","Mob With Mudprog"},

	{CCWRAP(CC_PROMPT_DEFAULT),				'P', 0,			"Prompt","Default"},
	{CCWRAP(CC_PROMPT_COMBAT),				'q', 0,			"Prompt","Combat"},
	{CCWRAP(CC_PROMPT_OLC),					'p', 0,			"Prompt","OLC"},
	{CCWRAP(CC_PROMPT_SWITCHEDPREFIX),		'\x8c', CCF_IMM,"Prompt","Switched Prefix"},	
	{CCWRAP(CC_PROMPT_SWITCHEDHOLYSPEECH),	'\xae', CCF_IMM,"Prompt","Switched HolySpeech"},	

	{CCWRAP(CC_SCORE_BORDER),				'S', 0,			"Score","Outside border"}, 
	{CCWRAP(CC_SCORE_INNER_BORDER),			's', 0,			"Score","Inner border"},  
	{CCWRAP(CC_SCORE_NAME_REMORT),			'g', 0,			"Score","Name/Remort"}, 
	{CCWRAP(CC_SCORE_LETGAINED),			'G', 0,			"Score","Letgained"}, 


	{CCWRAP(CC_SCORE_LEVEL),				'\xb0',	0,		"Score","Level"}, 
	{CCWRAP(CC_SCORE_RACE_GENDER_CLASS),	'\xb1', 0,		"Score","Race/Gender/Class"},
	{CCWRAP(CC_SCORE_LABELS),				'\xb2', 0,		"Score","Labels"},
	{CCWRAP(CC_SCORE_ATTRIBUTES),			'\xb3', 0,		"Score","Attributes"},
	{CCWRAP(CC_SCORE_BORN),					'\xb4', 0,		"Score","Born"},
	{CCWRAP(CC_SCORE_AGE),					'\xb5', 0,		"Score","Age"},
	{CCWRAP(CC_SCORE_PRAC_TRAIN),			'\xb6', 0,		"Score","Prac/Train"},
	{CCWRAP(CC_SCORE_XP_RPS),				'\xb7', 0,		"Score","XP/RPS"},
	{CCWRAP(CC_SCORE_XPTNL),				'\xb8', 0,		"Score","XPTNL"},
	{CCWRAP(CC_SCORE_CURRENTVALUE),			'\xb9', 0,		"Score","Current Value"},
	{CCWRAP(CC_SCORE_MAXVALUE),				'\xba', 0,		"Score","Max Value"},
	{CCWRAP(CC_SCORE_ALIGNMENT),			'\xbb', 0,		"Score","Alignment"},
	{CCWRAP(CC_SCORE_AC),					'\xbc', 0,		"Score","AC"},
	{CCWRAP(CC_SCORE_GOLD),					'\xbd', 0,		"Score","Gold"},
	{CCWRAP(CC_SCORE_SILVER),				'\xbe', 0,		"Score","Silver"},
	{CCWRAP(CC_SCORE_FORWARD_SLASH),		'\xbf', 0,		"Score","Forward Slash"},
	{CCWRAP(CC_SCORE_IMMORTAL_VALUES),		'\xc0', CCF_IMM,"Score","Immortal Values"},
	{CCWRAP(CC_SCORE_SHOW_AFFECTS_TITLE),	'\xc1', 0,		"Score","Show Affects Title"},
	{CCWRAP(CC_SCORE_SHOW_AFFECTS_TEXT),	'\xc2', 0,		"Score","Show Affects Text"},


	{CCWRAP(CC_SHOWCOL_CUSTOM),				'f', 0,			"Showcolour","Custom Active"},
	{CCWRAP(CC_SHOWCOL_TEMPLATE),			'F', 0,			"Showcolour","Template Active"}, 

	{CCWRAP(CC_SOCKETS_BRACKET),			'\x9b', CCF_SOCK,"Sockets","Brackets"}, 
	{CCWRAP(CC_SOCKETS_NUMBER),				'\x9c', CCF_SOCK,"Sockets","Descriptor #"}, 
	{CCWRAP(CC_SOCKETS_STATE_PLAYING),		'\x9d', CCF_SOCK,"Sockets","State-Playing"}, 
	{CCWRAP(CC_SOCKETS_STATE_OTHER),		'\x9e', CCF_SOCK,"Sockets","State-Other"}, 
	{CCWRAP(CC_SOCKETS_LOGIN_TIME),			'\x9f', CCF_SOCK,"Sockets","Login Time"}, 
	{CCWRAP(CC_SOCKETS_IDLE),				'\xa0', CCF_SOCK,"Sockets","Idle Time"}, 
	{CCWRAP(CC_SOCKETS_NAME),				'\xa1', CCF_SOCK,"Sockets","Name"}, 
	{CCWRAP(CC_SOCKETS_HOST),				'\xa2', CCF_SOCK,"Sockets","Host"}, 
	{CCWRAP(CC_SOCKETS_HOSTMULTI),			'\xaf',CCF_ADMIN,"Sockets","Host-Multi"}, 
	{CCWRAP(CC_SOCKETS_SYSTIME),			'\xa3', CCF_SOCK,"Sockets","System Time"}, 
	{CCWRAP(CC_SOCKETS_BAR),				'\xa4', CCF_SOCK,"Sockets","Bar"}, 
	{CCWRAP(CC_SOCKETS_IDENT),				'\xa5', CCF_SOCK,"Sockets","Ident Response"}, 
	{CCWRAP(CC_SOCKETS_NONSTANDARDIDENT),	'\xa6', CCF_SOCK,"Sockets","NonStandard Ident Response"}, 
	
	{CCWRAP(CC_TITLEBAR_DEFAULT_TEXT),		'T', 0,			"Titlebar","Default Text"}, 
	{CCWRAP(CC_TITLEBAR_DEFAULT_BAR),		't', 0,			"Titlebar","Default Bar"}, 

	{CCWRAP(CC_OLC_LABELS),					'r', 0,			"OLC","Labels"},
	{CCWRAP(CC_OLC_FLAGS),					'R', 0,			"OLC","Flags"}, 	
	{CCWRAP(CC_OLC_TEXT),					'x', 0,			"OLC","Text"}, 
	{CCWRAP(CC_OLC_WARNING),				'X', 0,			"OLC","Warnings"}, 	
	{CCWRAP(CC_OLC_OPTIONS),				'V', 0,			"OLC","Options"}, 	
	{CCWRAP(CC_OLC_SELECTED_OPTION),		'/', 0,			"OLC","Selected Option"}, 
	{CCWRAP(CC_OLC_VALUES),					'v', 0,			"OLC","Values"}, 
	{CCWRAP(CC_OLC_SET_FLAG),				'\'',0,			"OLC","Set Flag"}, 	
	{CCWRAP(CC_OLC_UNSET_FLAG),				'$', 0,			"OLC","Unset Flag"}, 	
	{CCWRAP(CC_OLC_UNSETTABLE_FLAG),		'\x98', 0,		"OLC","Unsettable Flag"}, 	
	{CCWRAP(CC_OLC_HINTS),					'@', 0,			"OLC","Hints, Comments & Clickable Helpers"}, 	
	{CCWRAP(CC_OLC_COMMANDS),				':', 0,			"OLC","Command MXP Links"}, 	
	{CCWRAP(CC_OLC_MXP_HELP_LINKS),			';', 0,			"OLC","Help MXP Links"}, 	
	{CCWRAP(CC_OLC_MXP_CLICKABLE_HELPER	),	'w', 0,			"OLC","MXP Clickable Helper"}, 	

	{CCWRAP(CC_WHO_BRACKET_NORMAL),			'\x96', 0,			"Wholist","Brackets - Normal"}, 
	{CCWRAP(CC_WHO_BRACKET_IRC),			'\x97', 0,			"Wholist","Brackets - IRC"}, 
	{CCWRAP(CC_WHO_BRACKET_IMMORTAL),		'\x83', 0,			"Wholist","Brackets - Immortal"}, 
	{CCWRAP(CC_WHO_STARS_MORTAL),			'\x81', 0,			"Wholist","Mortal Stars "}, 
	{CCWRAP(CC_WHO_STARS_IMM),				'\x82', 0,			"Wholist","Immortal Level"}, 
	{CCWRAP(CC_WHO_AFK_QUIET),				'\x84', 0,			"Wholist","AFK & Quiet"}, 
	{CCWRAP(CC_WHO_LINKDEAD),				'\x85', 0,			"Wholist","Linkdead"}, 
	
	{CCWRAP(CC_WHO_ADMIN_NOCHANNELED),		'\x86', CCF_ADMIN,	"WholistAdmin","No Channeled (!)"}, 
	{CCWRAP(CC_WHO_ADMIN_LOGGED),			'\x87', CCF_ADMIN,	"WholistAdmin","Logged (L)"}, 
	
	{CCWRAP(CC_WHO_IMM_NOT_WIZI),			'\x88', CCF_IMM,	"WholistImm","[IO]Wizi Off"}, 
	{CCWRAP(CC_WHO_IMM_IWIZI),				'\x89', CCF_IMM,	"WholistImm","IWizi On"}, 
	{CCWRAP(CC_WHO_IMM_OWIZI),				'\x90', CCF_IMM,	"WholistImm","OWizi On"}, 
	{CCWRAP(CC_WHO_IMM_WHOINVIS),			'\x91', CCF_IMM,	"WholistImm","WhoInvis"}, 

	{CCWRAP(CC_WHO_IMM_IMMTALKHASH),		'\x92', CCF_IMM,	"WholistImm","Immtalk Hash"}, 
	{CCWRAP(CC_WHO_IMM_ACTIVE),				'\x93', CCF_IMM,	"WholistImm","Active"}, 
	{CCWRAP(CC_WHO_IMM_QBC),				'\x94', CCF_IMM,	"WholistImm","`=,(`=-Q`=,)`=,(`=-B`=,)`=,(`=-C`=,)"}, 
	{CCWRAP(CC_WHO_IMM_QBC_BRACKETS),		'\x95', CCF_IMM,	"WholistImm","`=,(`=-Q`=,)`=,(`=-B`=,)`=,(`=-C`=,) Brackets"}, 
	
	{CCWRAP(CC_HELP_KEYWORDS),				'J', 0,	"Help","Keywords (top line shown on a help most times)"}, 
	{CCWRAP(CC_HELP_LINK),					'_', 0,	"Help","Link (to another help)"}, 
	{CCWRAP(CC_HELP_BROKENLINK),			'"', 0,	"Help","For broken helplinks"}, 
	{CCWRAP(CC_HELP_SYNTAX),				'l', 0,	"Help","Syntax examples"}, 
	{CCWRAP(CC_HELP_SEEALSO),				'|', 0,	"Help","'See Also/Parent' Colour"}, 
	{CCWRAP(CC_HELP_DEFAULT),				'?', 0,	"Help","Default text colour"}, 
	{CCWRAP(CC_HELP_TITLE),					'u', 0,	"Help","Titles"}, 
	{CCWRAP(CC_HELP_PREVNEXT_LINK),			'[', 0,	"Help","Prev/Next links"}, 
	{CCWRAP(CC_HELP_PREVNEXT_NOLINK),		']', 0,	"Help","Prev/Next nolinks"}, 
	{CCWRAP(CC_HELP_HEADINGS),				'U', 0,	"Help","Headings"}, 
	{CCWRAP(CC_HELP_BAR),					'\xad', 0,	"Help","Header/Footer Bar"}, 
	
	{CCWRAP(CC_ROOM_OOC),					'^', 0,	"Room","(OOC Room)"}, 
	{CCWRAP(CC_ROOM_OLC),					'&', 0,	"Room","(OLC)"}, 
	{CCWRAP(CC_ROOM_NAME),					'B', 0,	"Room","Name"}, 	
	{CCWRAP(CC_ROOM_DESCRIPT),				'b', 0,	"Room","Description"}, 
	{CCWRAP(CC_ROOM_LOCKERS),				'\x8d', 0,	"Room","Lockers"}, 
	{CCWRAP(CC_ROOM_EXITS),					'a', 0,	"Room","Exits"}, 
	{CCWRAP(CC_ROOM_FULLEXITS),				'\xac', 0,	"Room","FullExits"}, 	
	{CCWRAP(CC_ROOM_MOBS),					'D', 0,	"Room","Mobiles in room"}, 
	{CCWRAP(CC_OBJECT_CAN_WEAR),			'd', 0,	"Object","Wearable"}, 
	{CCWRAP(CC_OBJECT_CANT_WEAR),			'k', 0,	"Object","Unwearable"}, 
	{CCWRAP(CC_OBJECT_CANT_WEAR2),			'K', 0,	"Object","Unwearable 2"}, 

	{CCWRAP(CC_RP_SAYLINE),					'\xc3', 0,"RP","Says - Entire line"}, 
	{CCWRAP(CC_RP_EMOTELINE),				'\xc4', 0,"RP","Emotes - Entire line"}, 

	{CCWRAP(CC_MAX), '\0', 0, ""}
};
/**************************************************************************/
COLOUR_TEMPLATE_TYPE dawn_colour_scheme;
/**************************************************************************/
char *create_default_colour_scheme_buffer()
{
	static char buf[MSL];
	memset(buf,'0', MAX_CUSTOM_COLOUR);
	buf[MAX_CUSTOM_COLOUR]='\0';

	// defaults colours if colour_scheme_default_name not found
	buf[CC_DEFAULT1]='G';   // score, wholist
	buf[CC_DEFAULT2]='x';	// white
	buf[CC_DEFAULT3]='r';	// wholist stars, room name
	buf[CC_DEFAULT4]='B';	// Title bars
	buf[CC_DEFAULT5]='Y';	// Title bar text
	buf[CC_DEFAULT6]='M';	// Noble related
	buf[CC_DEFAULT7]='c';	// olc 1
	buf[CC_DEFAULT8]='y';	// olc 2
	buf[CC_DEFAULT9]='g';	// afk, prompt, rooms objects, q&a
	buf[CC_DEFAULT0]='x';	// default colour

	buf[CC_GAIN_XP]='W';
	buf[CC_GAIN_RPS]='C';
	buf[CC_GAIN_LEVEL]='Y';

	buf[CC_HELP_KEYWORDS]='C';
	buf[CC_HELP_LINK]='x';
	buf[CC_HELP_BROKENLINK]='R';
	buf[CC_HELP_SYNTAX]='B';
	buf[CC_HELP_SEEALSO]='C';
	buf[CC_HELP_DEFAULT]='0';
	buf[CC_HELP_TITLE]='0';
	buf[CC_HELP_PREVNEXT_LINK]='C';
	buf[CC_HELP_PREVNEXT_NOLINK]='S';
	buf[CC_HELP_HEADINGS]='Y';
	buf[CC_HELP_BAR]='B';

	buf[CC_CHANNEL_ANSWER]			='9';
	buf[CC_CHANNEL_ADMINTALK_TEXT]	='R';
	buf[CC_CHANNEL_ADMINTALK_PREFIX]='R';
	buf[CC_CHANNEL_HIGHADMINTALK_TEXT]='M';
	buf[CC_CHANNEL_HIGHADMINTALK_PREFIX]='M';
	buf[CC_CHANNEL_IMMTALK_TEXT]	='G';
	buf[CC_CHANNEL_IMMTALK_PREFIX]	='G';
	buf[CC_CHANNEL_NOBLETALK_PREFIX]='6';
	buf[CC_CHANNEL_NOBLETALK_TEXT]	='6';
	buf[CC_CHANNEL_NEWBIE]			='W';
	buf[CC_CHANNEL_NEWBIE_PREFIX]	='s';
	buf[CC_CHANNEL_IC]				='C';
	buf[CC_CHANNEL_OOC]				='c';
	buf[CC_CHANNEL_MYSTERY_IMM]		='R';
	buf[CC_CHANNEL_QUESTION]		='9';
	buf[CC_CHANNEL_CLANTALK_TEXT]	='W';
	buf[CC_CHANNEL_CLANTALK_PREFIX]	='W';
	buf[CC_CHANNEL_REPLY]			='M';
	buf[CC_CHANNEL_TELL]			='C';
	buf[CC_CREATION_TITLEBAR]		='4';
	buf[CC_CREATION_TITLEBAR_TEXT]	='5';

	buf[CC_HOLYVNUM_PLAYER_LEVEL]	='G';
	buf[CC_HOLYVNUM_MOB]			='b';
	buf[CC_HOLYVNUM_MOB_WITH_PROG]	='r';

	buf[CC_PROMPT_COMBAT]			='9';
	buf[CC_PROMPT_DEFAULT]			='9';
	buf[CC_PROMPT_OLC]				='y';
	buf[CC_PROMPT_SWITCHEDPREFIX]	='y';
	buf[CC_PROMPT_SWITCHEDHOLYSPEECH]='Y';

	buf[CC_ROOM_DESCRIPT]			='0';
	buf[CC_ROOM_LOCKERS]			='S';
	buf[CC_ROOM_EXITS]				='0';
	buf[CC_ROOM_FULLEXITS]			='0';
	buf[CC_ROOM_MOBS]				='B';
	buf[CC_ROOM_NAME]				='3';
	buf[CC_ROOM_OLC]				='Y';
	buf[CC_ROOM_OOC]				='B';
	buf[CC_SCORE_BORDER]			='g';
	buf[CC_SCORE_INNER_BORDER]		='1';		
	buf[CC_SCORE_NAME_REMORT]		='1';	
	buf[CC_SCORE_LETGAINED]			='1';

	buf[CC_SCORE_LEVEL]				='R';
	buf[CC_SCORE_RACE_GENDER_CLASS]	='B';
	buf[CC_SCORE_LABELS]			='x';
	buf[CC_SCORE_ATTRIBUTES]		='x';
	buf[CC_SCORE_BORN]				='Y';
	buf[CC_SCORE_AGE]				='y';
	buf[CC_SCORE_PRAC_TRAIN]		='r';
	buf[CC_SCORE_XP_RPS]			='r';
	buf[CC_SCORE_XPTNL]				='R';
	buf[CC_SCORE_CURRENTVALUE]		='R';
	buf[CC_SCORE_MAXVALUE]			='r';
	buf[CC_SCORE_ALIGNMENT]			='B';
	buf[CC_SCORE_AC]				='B';
	buf[CC_SCORE_GOLD]				='Y';
	buf[CC_SCORE_SILVER]			='s';
	buf[CC_SCORE_FORWARD_SLASH]		='s';
	buf[CC_SCORE_IMMORTAL_VALUES]	='x';
	buf[CC_SCORE_SHOW_AFFECTS_TITLE]='r';
	buf[CC_SCORE_SHOW_AFFECTS_TEXT]	='x';
	buf[CC_SHOWCOL_CUSTOM]			='x';
	buf[CC_SHOWCOL_TEMPLATE]		='S';

	buf[CC_SOCKETS_BRACKET]			='S';
	buf[CC_SOCKETS_NUMBER]			='W';
	buf[CC_SOCKETS_STATE_PLAYING]	='G';
	buf[CC_SOCKETS_STATE_OTHER]		='Y';
	buf[CC_SOCKETS_LOGIN_TIME]		='c';
	buf[CC_SOCKETS_IDLE]			='x';
	buf[CC_SOCKETS_NAME]			='B';
	buf[CC_SOCKETS_HOST]			='W';
	buf[CC_SOCKETS_HOSTMULTI]		='R';
	buf[CC_SOCKETS_SYSTIME]			='C';
	buf[CC_SOCKETS_BAR]				='S';
	buf[CC_SOCKETS_IDENT]			='Y';
	buf[CC_SOCKETS_NONSTANDARDIDENT]='W';

	buf[CC_TITLEBAR_DEFAULT_BAR]	='4';
	buf[CC_TITLEBAR_DEFAULT_TEXT]	='5';
	
	buf[CC_OLC_LABELS]				='8';
	buf[CC_OLC_FLAGS]				='7';
	buf[CC_OLC_TEXT]				='g';
	buf[CC_OLC_OPTIONS]				='c';
	buf[CC_OLC_VALUES]				='x';
	buf[CC_OLC_WARNING]				='R';
	buf[CC_OLC_SET_FLAG]			='C';
	buf[CC_OLC_UNSET_FLAG]			='S';
	buf[CC_OLC_UNSETTABLE_FLAG]		='c';
	buf[CC_OLC_SELECTED_OPTION]		='Y';
	buf[CC_OLC_COMMANDS]			='x';
	buf[CC_OLC_HINTS]				='S';
	buf[CC_OLC_MXP_HELP_LINKS]		='W';
	buf[CC_OLC_MXP_CLICKABLE_HELPER]='S';

	buf[CC_WHO_ADMIN_LOGGED]		='y';
	buf[CC_WHO_ADMIN_NOCHANNELED]	='M';
	buf[CC_WHO_AFK_QUIET]			='9';
	buf[CC_WHO_BRACKET_IMMORTAL]	='1';
	buf[CC_WHO_BRACKET_IRC]			='B';
	buf[CC_WHO_BRACKET_NORMAL]		='2';
	buf[CC_WHO_IMM_ACTIVE]			='R';
	buf[CC_WHO_IMM_IMMTALKHASH]		='G';
	buf[CC_WHO_IMM_IWIZI]			='R';
	buf[CC_WHO_IMM_NOT_WIZI]		='S';
	buf[CC_WHO_IMM_OWIZI]			='B';
	buf[CC_WHO_IMM_QBC]				='B';
	buf[CC_WHO_IMM_QBC_BRACKETS]	='Y';	
	buf[CC_WHO_IMM_WHOINVIS]		='r';
	buf[CC_WHO_LINKDEAD]			='Y';
	buf[CC_WHO_STARS_IMM]			='3';
	buf[CC_WHO_STARS_MORTAL]		='0';

	buf[CC_OBJECT_CAN_WEAR]			='g';
	buf[CC_OBJECT_CANT_WEAR]		='G';
	buf[CC_OBJECT_CANT_WEAR2]		='m';
	
	buf[CC_COMMAND_EXAMPLE_SYNTAX]	='Y';

	buf[CC_INFO_HEADER]				='R';
	buf[CC_INFO_TEXT]				='W';

	buf[CC_MISC_RETURN_TO_CONTINUE]	='x';
	buf[CC_MISC_EQUIPMENT_LISTS]	='g';
	buf[CC_MISC_EQUIPMENT_LISTS_NONE]='R';

	buf[CC_RP_SAYLINE]				='x';
	buf[CC_RP_EMOTELINE]			='x';
	

	return buf;
}

/**************************************************************************/
void create_default_colour_scheme()
{
	char *buf=create_default_colour_scheme_buffer();
	
	// setup our default colour scheme 
	dawn_colour_scheme.template_name=str_dup(colour_scheme_default_name);
	dawn_colour_scheme.template_description=str_dup(colour_scheme_default_descript);
	dawn_colour_scheme.template_colour=str_dup(buf);
	dawn_colour_scheme.template_protected_by_level=MAX_LEVEL; // most can't edit it

	// add to the front of the list
	dawn_colour_scheme.next=colour_template_list;
	colour_template_list=&dawn_colour_scheme;
	default_colour_template=colour_template_list;
};
/**************************************************************************/
bool can_edit_colour(char_data *ch, char_data *v, int index)
{
	if(IS_SET(custom_colour_table[index].flags, CCF_IMM) 
		&& (!IS_IMMORTAL(ch) || !IS_IMMORTAL(v))){
		return false;
	}

	if(IS_SET(custom_colour_table[index].flags, CCF_ADMIN) 
		&& (!IS_ADMIN(ch) || !IS_ADMIN(v)) ){
		return false;
	}

	if(IS_SET(custom_colour_table[index].flags, CCF_HIGHADMIN) 
		&& (!(TRUE_CH(ch)->level>=(MAX_LEVEL-1))
		    || !(TRUE_CH(v)->level>=(MAX_LEVEL-1)))){
		return false;
	}

	if(IS_SET(custom_colour_table[index].flags, CCF_NOBLE) 
		&& (!IS_NOBLE(ch) || !IS_NOBLE(v))){
		return false;
	}

	if(IS_SET(custom_colour_table[index].flags, CCF_SOCK) 
		&& (!IS_TRUSTED(ch, MAX_LEVEL - 4) || !IS_TRUSTED(v,MAX_LEVEL - 4)) ){
		return false;
	}

	return true;
}
/**************************************************************************/
// return -1 if can't find a matching cc_code_text
int	get_cc_code_text_index(char *text, int last)
{
	// check the next after last value for a quicker searching
	if(!str_cmp(custom_colour_table[last+1].cc_code_text, text)){
		return custom_colour_table[last+1].cc_code;
	}

	for(int i=0; custom_colour_table[i].custom_colour_code!='\0'; i++){
		if(!str_cmp(custom_colour_table[i].cc_code_text, text)){
			return custom_colour_table[i].cc_code;
		}
	}
	return -1;

}
/**************************************************************************/
// only write something to the disk if we must
void fwrite_custom_colours(FILE* fp, const char *header, const char custom_colours[])
{
	bool nothing_written=true;
	for(int i=0; custom_colour_table[i].custom_colour_code!='\0'; i++){
		if(custom_colours[custom_colour_table[i].cc_code]!='.'){
			if(nothing_written){
				fprintf(fp,"%s\n", header);
				nothing_written=false;
			}
			fprintf(fp,"\t%-35s%c\n",
				custom_colour_table[i].cc_code_text,
				custom_colours[custom_colour_table[i].cc_code]);
		}
	}
	if(nothing_written==false){
		fprintf(fp,"end_custom_colours >>>%s<<<\n",custom_colours);
	}
}
/**************************************************************************/
// assumes the header has already been read
char *fread_custom_colours(FILE* fp, bool player)
{
	char *cc_code;
	char letter;
	int last=0;
	int index;

	char *custom_colours=(char*)malloc(MAX_CUSTOM_COLOUR+1);
	if(player){
		memset(custom_colours, '.', MAX_CUSTOM_COLOUR);
	}else{
		strcpy(custom_colours, create_default_colour_scheme_buffer());
	}
	custom_colours[MAX_CUSTOM_COLOUR]='\0';

	if(!fp){
		return custom_colours;
	}

	// loop thru reading the custom colours
	cc_code=fread_word(fp);
	while(str_cmp("end_custom_colours", cc_code)){

		letter=fread_letter(fp);
		
		index=get_cc_code_text_index(cc_code, last); 		
		if(index<0){
			logf("fread_custom_colours(): Unrecognised custom colour reference %s ignored", cc_code);			
		}else{
			custom_colours[index]=letter;
			last=index;
		}		
		cc_code=fread_word(fp);
	}
	fread_to_eol(fp); // ignore the raw dump of colour codes
	return custom_colours;
}
/**************************************************************************/
void save_colour_templates()
{
	logf("===Saving colour templates to %s.", COLOUR_TEMPLATES_FILE);
    GIOSAVE_LIST(colour_template_list, COLOUR_TEMPLATE_TYPE, COLOUR_TEMPLATES_FILE, true);
	logf("Save finished.");
}
/**************************************************************************/
void load_colour_templates()
{
	logf("===Loading colour templates from %s.", COLOUR_TEMPLATES_FILE);
    GIOLOAD_LIST(colour_template_list, COLOUR_TEMPLATE_TYPE, COLOUR_TEMPLATES_FILE);
	
	default_colour_template=find_colour_template(colour_scheme_default_name);
	if(!default_colour_template){
		logf("Could not find the default colour scheme '%s', creating it.",
			colour_scheme_default_name);
		create_default_colour_scheme();
	}else{
//		default_colour_template->template_protected_by_level=MAX_LEVEL;
	}
	logf("Colour template load process completed.");
}
/**************************************************************************/
void init_custom_colours()
{
	load_colour_templates();

	// initialise the custom colour index
	int i;
	for (i=0;i<256; i++){
		custom_colour_index[i]=CC_MAX;
	}
	i=0;
	while(custom_colour_table[i].custom_colour_code!='\0'){
		custom_colour_index[custom_colour_table[i].custom_colour_code]
			=custom_colour_table[i].cc_code;
		i++;
	}

}
/**************************************************************************/
COLOUR_TEMPLATE_TYPE *find_colour_template(const char *template_name)
{
	COLOUR_TEMPLATE_TYPE *result;
	// first do an exact search
	for(result=colour_template_list; result; result=result->next){
		if(!str_cmp(result->template_name, template_name)){
			return result;
		}
	}

	// do a count search
	if(is_number(template_name)){
		int count=atoi(template_name);
		for(result=colour_template_list; result; result=result->next){
			if(--count<1){
				return result;
			}
		}
		return NULL;		
	}

	// try a prefix match next
	for(result=colour_template_list; result; result=result->next){
		if(!str_prefix(result->template_name, template_name)){
			return result;
		}
	}
	return NULL; // couldnt find the template
}


/**************************************************************************/
// core user interface of colour customising system 
struct	custom_colour_cmd_type
{
    const char * name;
    DO_FUN	*do_fun;
	int flags;
	const char * syntax;
};
/**************************************************************************/
#define CMD_IMMORTAL_ONLY		(A)	// Imm only command
#define CMD_ADMIN_ONLY			(B)	// Admin only command

DECLARE_DO_FUN(custom_colour_show);
DECLARE_DO_FUN(custom_colour_showperson);
DECLARE_DO_FUN(custom_colour_set);
DECLARE_DO_FUN(custom_colour_clear);
DECLARE_DO_FUN(custom_colour_list);
DECLARE_DO_FUN(custom_colour_use);
DECLARE_DO_FUN(custom_colour_on);
DECLARE_DO_FUN(custom_colour_off);
DECLARE_DO_FUN(custom_colour_codelist);
DECLARE_DO_FUN(custom_colour_savescheme);
DECLARE_DO_FUN(custom_colour_replacescheme);
DECLARE_DO_FUN(custom_colour_description);
DECLARE_DO_FUN(custom_colour_level);
DECLARE_DO_FUN(custom_colour_rename);
DECLARE_DO_FUN(custom_colour_channel);
DECLARE_DO_FUN(custom_colour_mode);
DECLARE_DO_FUN(custom_colour_prefix);
DECLARE_DO_FUN(custom_colour_flashing);
DECLARE_DO_FUN(custom_colour_socials_colour);
/**************************************************************************/
struct custom_colour_cmd_type custom_colour_cmd_table[]=
{
	{	"show",		custom_colour_show,	0, "Shows your current colour scheme."},
	{	"showperson",custom_colour_showperson,	0, "Shows another persons colours scheme."},
	{	"set",		custom_colour_set,	0, "Sets a particular colour in your colour scheme."},
	{	"prefix",	custom_colour_prefix,	0, "Sets a particular character to be used to prefix colour codes."},
	{	"clear",	custom_colour_clear,	0, "Clears all colour customizations."},	
	{	"list",		custom_colour_list,	0, "Lists all the mud colour templates."},
	{	"use",		custom_colour_use,	0, "Use a particular colour scheme/template."},	
	{	"on",		custom_colour_on,	0, "Turn on your colour."},
	{	"off",		custom_colour_off,	0, "Turn off your colour."},
	{	"flashing",	custom_colour_flashing,	0, "Enables/Disables support for flashing ansi colours."},
	{	"mode",		custom_colour_mode,	0, "To change to a colour mode other than ansi."},
	{	"codelist",	custom_colour_codelist,	CMD_ADMIN_ONLY, "List sorted custom colour codes (find free codes)."},	
	{	"savescheme",custom_colour_savescheme,	CMD_IMMORTAL_ONLY, "Save your current colour settings as a scheme."},
	{	"replacescheme",custom_colour_replacescheme,CMD_IMMORTAL_ONLY, "Replace a current scheme with your scheme."},
	{	"description",custom_colour_description,CMD_IMMORTAL_ONLY, "Replace the description of a current scheme."},	
	{	"rename",custom_colour_rename,CMD_IMMORTAL_ONLY, "Rename a scheme."},	
	{	"level",custom_colour_level,CMD_IMMORTAL_ONLY, "Alter the edit level of a scheme."},	
	{	"channel", custom_colour_channel, 0, "Enabled/Disable other peoples colour codes in channels."},
	{	"socialscolour", custom_colour_socials_colour, 0, "Enabled/Disable the displaying of colour in socials."},	
	{	"",		NULL, 0, ""}
};
/**************************************************************************/
// colour command interpreter - Kal
void do_colour( char_data *ch, char *argument )
{
	char arg[MIL];
	int i;

	if(!ch->desc){
		ch->println("You must have an active connection to use this command.");
		return;
	}

	argument=one_argument(argument,arg);
	if(IS_NULLSTR(arg)){ // no args - show syntax
		ch->printlnf("%s Customing Colour System", MUD_NAME);
		ch->println("Syntax:");
		for(i=0;!IS_NULLSTR(custom_colour_cmd_table[i].name); i++){
			if(IS_SET(custom_colour_cmd_table[i].flags, CMD_IMMORTAL_ONLY) && !IS_IMMORTAL(ch)){
				continue;
			}
			if(IS_SET(custom_colour_cmd_table[i].flags, CMD_ADMIN_ONLY) && !IS_ADMIN(ch)){
				continue;
			}
			if(IS_IMMORTAL(ch)){
				ch->printlnf("  colour %-15s - %s", custom_colour_cmd_table[i].name, 
					custom_colour_cmd_table[i].syntax);
			}else{
				ch->printlnf("  colour %-10s - %s", custom_colour_cmd_table[i].name, 
					custom_colour_cmd_table[i].syntax);
			}
		}
		return;
	}

	// find the command
	for(i=0;!IS_NULLSTR(custom_colour_cmd_table[i].name); i++){
		if(IS_SET(custom_colour_cmd_table[i].flags, CMD_IMMORTAL_ONLY) && !IS_IMMORTAL(ch)){
			continue;
		}

		if(IS_SET(custom_colour_cmd_table[i].flags, CMD_ADMIN_ONLY) && !IS_ADMIN(ch)){
			continue;
		}

		if(!str_prefix(arg,custom_colour_cmd_table[i].name)){
			// found the command
			break;
		}
	}
	if(IS_NULLSTR(custom_colour_cmd_table[i].name)){
		ch->printlnf("Unrecognised Custom Colour command '%s'", arg);
		do_colour(ch,"");
		return;
	}

	// run the command
	(*custom_colour_cmd_table[i].do_fun) (ch, argument);
}
/**************************************************************************/
void showcolours_to_char(char_data *ch, char_data *v, char *argument)
{	
	char cat_desc_buf[MIL];
	char buf[MIL];
	BUFFER *output;

	// transparent thru switch
	connection_data *d=v->desc;
	if(!d && v->controlling){
		d=v->controlling->desc;
	}
	if(!d){
		ch->printlnf("%s does not have an active connection, wait till they reconnect.",
			PERS(TRUE_CH(v), ch));
		return;
	}

	COLOUR_MEMORY_TYPE *cm=&d->colour_memory;

	output = new_buf();
	if(ch==v){
		sprintf(buf,"`=t`#%s`x", format_titlebar("SHOW CUSTOM COLOURS"));
	}else{
		sprintf(buf,"`=t`#%s`x", format_titlebarf("SHOW CUSTOM COLOURS FOR %s",
			uppercase(v->name)));
	}
	add_buf(output,buf);

	add_buf(output,"  Current scheme/template in use: ");
	add_buf(output,cm->colour_template->template_name);
	if(!IS_NULLSTR(argument)){
		add_buf(output,"  Filtering list for '");
		add_buf(output, argument);
		add_buf(output,"'");
	}
	add_buf(output,"\r\n");

	int count=0;
	for(int i=0; custom_colour_table[i].custom_colour_code!='\0'; i++){
		if(!can_edit_colour(TRUE_CH(ch), TRUE_CH(v), i)){
			continue;
		}
		count++;

		// filter in only required colours
		if(!IS_NULLSTR(argument)){
			bool matched=false;

			if (is_name( argument, custom_colour_table[i].category)) {
				matched=true;
			}
			if (is_name( argument, custom_colour_table[i].cc_code_text)) {
				matched=true;
			}
			if (is_name( argument, custom_colour_table[i].description)) {
				matched=true;
			}
			if(str_len(argument)==1){
				if(*argument==cm->custom_colour[custom_colour_table[i].cc_code]){
					matched=true;
				}
				if(*argument==custom_colour_table[i].custom_colour_code){
					matched=true;
				}
			}
			if(!matched){
				continue;
			}
		}

		sprintf(cat_desc_buf,"%s:`=%c %s",
				custom_colour_table[i].category,
				custom_colour_table[i].custom_colour_code,
				custom_colour_table[i].description);
		strcpy(cat_desc_buf,str_width(cat_desc_buf, 32, true));

		unsigned char c=custom_colour_table[i].custom_colour_code;
		char schemebuf[3];
		if(is_digit(cm->colour_template->template_colour[custom_colour_table[i].cc_code])){
			sprintf(schemebuf, "=%c", cm->colour_template->template_colour[custom_colour_table[i].cc_code]);
		}else{
			sprintf(schemebuf, "%c", cm->colour_template->template_colour[custom_colour_table[i].cc_code]);			
		}
		if(cm->custom_colour[custom_colour_table[i].cc_code]=='.'){
			sprintf(buf,"  [%2d] `=F%s`x `=Fsetting=%c`x  scheme=`%s%c`x  code=``=%s%c (\\x%x)`x\r\n",
				count,
				cat_desc_buf,
				cm->custom_colour[custom_colour_table[i].cc_code],
				schemebuf,
				cm->colour_template->template_colour[custom_colour_table[i].cc_code],
				is_print(c)?"":"`S",
				is_print(c)?c:'?', 
				c);
		}else{
			sprintf(buf,"  [%2d] `=f%s`x `=fsetting=`%c%c`x  scheme=`%s%c`x  code=``=%s%c (\\x%x)`x\r\n",
				count,
				cat_desc_buf,
				cm->custom_colour[custom_colour_table[i].cc_code],
				cm->custom_colour[custom_colour_table[i].cc_code],
				schemebuf,
				cm->colour_template->template_colour[custom_colour_table[i].cc_code],
				is_print(c)?"":"`S",
				is_print(c)?c:'?', 
				c);
		}
		add_buf(output,buf);
	}
	if(!GAMESETTING(GAMESET_PLAYERS_CANT_ACCESS_OTHERS_COLOURS) || IS_IMMORTAL(ch)){
		add_buf(output,"  Use 'colour showplayer <playername>' to see another player's settings.\r\n");
	}
	if(IS_NULLSTR(argument)){
		add_buf(output,"  You can filter this list like the area list - e.g. '`=Ccolour show who`x'\r\n");
	}
	sprintf(buf,"`&%s", format_titlebar("="));
	add_buf(output,buf);

    ch->sendpage(buf_string(output));
    free_buf(output);
}
/**************************************************************************/
void custom_colour_show(char_data *ch, char*argument)
{
	showcolours_to_char(ch, ch, argument);
}
/**************************************************************************/
void custom_colour_showperson(char_data *ch, char*argument)
{
    char arg[MIL];
    char_data *victim;

    argument=one_argument( argument, arg );

    if ((!GAMESETTING(GAMESET_PLAYERS_CANT_ACCESS_OTHERS_COLOURS) || IS_IMMORTAL(ch)) 
		&& !IS_NULLSTR(arg))
    {
		if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
		{
			ch->printlnf( "'%s' isn't here.", arg );
			return;
		}
    }
	else
	{
		victim=ch;
	}
	showcolours_to_char(ch,victim, argument);
}

/**************************************************************************/
int get_max_colour_for_user(char_data *ch)
{
	int total=0;
	for(int i=0; custom_colour_table[i].custom_colour_code!='\0'; i++){
		if(can_edit_colour(TRUE_CH(ch), TRUE_CH(ch), i)){
			total++;
		}
	}
	return total;
}
/**************************************************************************/
int get_colour_index_for_user(char_data *ch, int user_value)
{
	int total=0;
	for(int i=0; custom_colour_table[i].custom_colour_code!='\0'; i++){
		if(can_edit_colour(TRUE_CH(ch), TRUE_CH(ch), i)){
			total++;
		}
		if(total==user_value){
			return (int)i;
		}
	}
	return -1; // they can't go that high
}
/**************************************************************************/
void custom_colour_set(char_data *ch, char*argument)
{
	char val[MIL];
	argument=one_argument(argument, val);
	int value=atoi(val);
	int max=get_max_colour_for_user(ch);
	
	if(value<1 || value>max || IS_NULLSTR(argument)){
		ch->println("syntax: colour set <number> <colourcode>");
		ch->printlnf("Number must be between 1 and %d", max);
		ch->println("Use a colourcode of . to remove customisation.");
		return;
	}
	int newvalue=get_colour_index_for_user(ch, value);

	char newcol=*argument;
	if(	newcol!='?' && newcol!='.' && !is_digit(newcol)
			&& (!is_alpha(newcol) 
			|| colourTable[(int)newcol].ansi[0]=='\0'))
	{
		ch->print("`x");
		ch->printlnfbw("%c is not a valid colour code.", newcol);
		return;
	}
	char oldcol=ch->desc->colour_memory.custom_colour[custom_colour_table[newvalue].cc_code];
	ch->desc->colour_memory.custom_colour[custom_colour_table[newvalue].cc_code]=newcol;
	ch->printlnf("`xColour %d (%s:`=%c%s`x) changed from %c(`%c#`x) to %c(`%c#`x)", value, 
		custom_colour_table[newvalue].category,
		custom_colour_table[newvalue].custom_colour_code,
		custom_colour_table[newvalue].description, oldcol, oldcol, newcol, newcol);
}
/**************************************************************************/
void custom_colour_on(char_data *ch, char *)
{
	if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only sorry.");
		return;
	}

	if(ch->desc){
		ch->desc->colour_mode=CT_ANSI;
	}
	if(TRUE_CH(ch)->pcdata){
		TRUE_CH(ch)->pcdata->colourmode=CT_ANSI;
	}
    ch->println("You are mudding in `rC`RO`YL`gO`bU`mR`x!!!");
}
/**************************************************************************/
void custom_colour_off(char_data *ch, char *)
{
	if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only sorry.");
		return;
	}

	if(ch->desc){
		ch->desc->colour_mode=CT_NOCOLOUR;
	}
	if(TRUE_CH(ch)->pcdata){
		TRUE_CH(ch)->pcdata->colourmode=CT_NOCOLOUR;
	}
	ch->println("You are a monochromer now.");
}
/**************************************************************************/
void custom_colour_mode(char_data *ch, char *argument)
{
	if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only sorry.");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println( "syntax: `=Ccolour mode <new colour mode>`x");
		ch->wraplnf( "Valid colour modes include: %s", flagtable_names(colourmode_types));
		ch->printlnf("Your current mode is: %s", flag_string(colourmode_types, ch->desc->colour_mode));
		return;
	}

	int value = flag_value( colourmode_types, argument);
	if(value == NO_FLAG ){
		ch->printlnf( "Invalid colour mode '%s'.", argument);
		ch->wraplnf( "Valid colour modes include: %s", flagtable_names(colourmode_types));
		return;
	}

	if(ch->desc){
		ch->desc->colour_mode=(COLOUR_TYPE)value;
	}
	if(TRUE_CH(ch)->pcdata){
		TRUE_CH(ch)->pcdata->colourmode=(COLOUR_TYPE)value;
	}
	ch->printlnf("Colour mode set to %s.", flag_string(colourmode_types, value));

	switch(value){
	default:
		break;
	case CT_HTML:
		ch->wrapln("HTML mode sends colours in font tags and supports the `ae`Ax`vt`ee`nn`Vd`me`qd`x colours.");
		break;
	case CT_MXP:
		ch->wrapln("MXP mode sends colours in mxp <C> tags, it also supports the `ae`Ax`vt`ee`nn`Vd`me`qd`x colours."
			"  Colours are the same as the HTML mode, except the dark red which is brighter `rcolour`x.");
		break;
	case CT_BRIGHT:
		ch->wrapln("BRIGHT is like the MXP mode in that it sends colours in mxp <C> tags, "
			"it also supports the `ae`Ax`vt`ee`nn`Vd`me`qd`x colours."
			"  Colours are the same as the HTML mode, except all the "
			"ANSI colours are brighter - designed for dark monitors.");
		break;
	}

}
/**************************************************************************/
void custom_colour_clear(char_data *ch, char *)
{
	memset(ch->desc->colour_memory.custom_colour,'.', MAX_CUSTOM_COLOUR);
	ch->println("Colour customisations cleared."); 
}

/**************************************************************************/
void custom_colour_list(char_data *ch, char *)
{
	COLOUR_TEMPLATE_TYPE *pct;

	ch->titlebar("COLOUR SCHEMES");
	int count=0;

	// show their profile in use, transparent thru switch
	connection_data *d=ch->desc;
	if(!d && ch->controlling){
		d=ch->controlling->desc;
	}
	if(d){
		COLOUR_MEMORY_TYPE *cm=&d->colour_memory;
		ch->printlnf("Current scheme/template in use: %s",cm->colour_template->template_name);
	}

	for(pct=colour_template_list; pct; pct=pct->next){
		if(IS_IMMORTAL(ch)){
			ch->printlnf("[%2d] %-15s: %s <%d>",				
				++count,
				pct->template_name,
				pct->template_description,
				pct->template_protected_by_level);
		}else{
			ch->printlnf("[%2d] %-15s: %s",
				++count,
				pct->template_name,
				pct->template_description);
		}
	}
}
/**************************************************************************/
void custom_colour_use(char_data *ch, char *argument)
{
	COLOUR_TEMPLATE_TYPE *new_scheme;

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour use <scheme/template name>   (as per colour list)");
		ch->println("Syntax: colour use <scheme/template number> (as per colour list)");
		return;
	}
	new_scheme=find_colour_template(argument);

	if(!new_scheme){
		ch->printlnf("Could not find scheme/template '%s'", argument);
		return;
	}
	ch->desc->colour_memory.colour_template=new_scheme;
	if(TRUE_CH(ch)->pcdata){
		TRUE_CH(ch)->pcdata->custom_colour_scheme=new_scheme;
	}
	ch->printlnf("Base colour scheme changed to '%s'", new_scheme->template_name);
}
/**************************************************************************/
// list all the current colour codes
// This function doesn't bother cleaning up the memory... this function is
// run rarely and only really on development machines.
// - Kal :)
void custom_colour_codelist(char_data *ch, char *)
{
	name_linkedlist_type *codelist, *plist;
	char buf[MIL];

	codelist=NULL;
	for(int i=0; custom_colour_table[i].custom_colour_code!='\0'; i++){
		sprintf(buf,"%c",custom_colour_table[i].custom_colour_code);
		addlist(&codelist,buf, 0, true, false);
	}
	int count=0;
	unsigned char last='\0';
	unsigned char thischar;
	ch->println("All currently used colour codes (red are duplicated - fix these):");
	for(plist=codelist;plist; plist=plist->next){
		thischar=*(plist->name);
		count++;
		ch->printf(" %s[%2d] (`#%s\\x%x`&) %3d %s`x%s", 
			(thischar==last?"`R":"`x"),
			count, 
			thischar==last?"":thischar==last+1?"":"`g",
			(int)thischar, (int)thischar, 
			(is_print(thischar)?FORMATF("%c",thischar):"`#`S.`&"),
			count%4==0?"\r\n":"   ");
		last=thischar;
	}
	if(count%4!=0){
		ch->println("");
	}
	ch->println("Green = there is a gap in ascii values directly preceeding");

}
/**************************************************************************/
void custom_colour_savescheme(char_data *ch, char *argument)
{
	COLOUR_TEMPLATE_TYPE *new_scheme;

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour savescheme <name of new scheme>");
		return;
	}
	new_scheme=find_colour_template(argument);
	if(new_scheme){
		ch->printlnf("There already exists a scheme '%s', you need something more unique.", 
			new_scheme->template_name);
		return;
	}
	
	// create our new scheme in memory
	char buf[MIL];
	memcpy(buf,ch->desc->colour_memory.custom_colour, MAX_CUSTOM_COLOUR);
	buf[MAX_CUSTOM_COLOUR]='\0';

	// apply from the template any holes
	for(int i=0; i<MAX_CUSTOM_COLOUR; i++){
		if(buf[i]=='.'){
			buf[i]=ch->desc->colour_memory.colour_template->template_colour[i];
		}
		if(buf[i]=='.'){
			buf[i]='0';
		}
	}

	new_scheme=new COLOUR_TEMPLATE_TYPE;
	new_scheme->template_colour=str_dup(buf);
	new_scheme->template_name=str_dup(argument);
	
	sprintf(buf,"A scheme by %s", ch->name);
	new_scheme->template_description=str_dup(buf);
	new_scheme->template_protected_by_level=LEVEL_IMMORTAL; 
	// add to the second in the template list
	assertp(colour_template_list); // we should always have the default template
	new_scheme->next=colour_template_list->next;
	colour_template_list->next=new_scheme;

	ch->println("Scheme created, use 'colour description' to set a description for the scheme.");
	save_colour_templates();
	ch->println("Colour schemes/templates saved");
}
/**************************************************************************/
void custom_colour_replacescheme(char_data *ch, char *argument)
{
	COLOUR_TEMPLATE_TYPE *new_scheme;

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour replacescheme <scheme/template number>  (as per colour list)");
		return;
	}
	new_scheme=find_colour_template(argument);
	if(!new_scheme){
		ch->printlnf("Could not find scheme/template '%s' to replace", argument);
		return;
	}

	if(new_scheme->template_protected_by_level>get_trust(ch)){
		ch->printlnf("The scheme %s is rated at level %d, so you can't edit it.",
			new_scheme->template_name,
			new_scheme->template_protected_by_level);
		return;
	}

	// create our new scheme in memory
	char buf[MIL];
	memcpy(buf,ch->desc->colour_memory.custom_colour, MAX_CUSTOM_COLOUR);
	buf[MAX_CUSTOM_COLOUR]='\0';

	// apply from the template any holes
	int i;
	for(i=0; i<MAX_CUSTOM_COLOUR; i++){
		if(buf[i]=='.'){
			buf[i]=ch->desc->colour_memory.colour_template->template_colour[i];
		}
		if(buf[i]=='.'){
			buf[i]='0';
		}
	}

	// apply any thing that we can edit, to the replacement scheme
	for(i=0; i<MAX_CUSTOM_COLOUR; i++){
		if(can_edit_colour(ch,ch,i)){
			new_scheme->template_colour[i]=buf[i];			
		}
	}
	save_colour_templates();
	ch->printlnf("Replaced scheme '%s'.", new_scheme->template_name);
	ch->println("Colour schemes/templates resaved");
}
/**************************************************************************/
void custom_colour_description(char_data *ch, char *argument)
{
	COLOUR_TEMPLATE_TYPE *new_scheme;

	char arg[MIL];
	argument=one_argument(argument, arg);

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour descript <scheme/template number> new description");
		return;
	}
	if(!is_number(arg)){
		ch->println("You must specify a scheme by its number.");
		return;
	}
	new_scheme=find_colour_template(arg);
	if(!new_scheme){
		ch->printlnf("Could not find scheme/template '%s' to change the description of.", argument);
		return;
	}

	if(new_scheme->template_protected_by_level>get_trust(ch)){
		ch->printlnf("The scheme %s is rated at level %d, so you can't edit it.",
			new_scheme->template_name,
			new_scheme->template_protected_by_level);
		return;
	}

	ch->printlnf("Colour schemes '%s' description '%s' replaced with '%s'",
		new_scheme->template_name,
		new_scheme->template_description,
		capitalize(argument));
	replace_string(new_scheme->template_description, capitalize(argument));
	
	save_colour_templates();
	ch->println("Colour schemes/templates resaved");
}
/**************************************************************************/
void custom_colour_level(char_data *ch, char *argument)
{
	COLOUR_TEMPLATE_TYPE *new_scheme;

	char arg[MIL];
	argument=one_argument(argument, arg);

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour level <scheme/template number> <new level>");
		return;
	}
	if(!is_number(arg)){
		ch->println("You must specify a scheme by its number.");
		return;
	}
	new_scheme=find_colour_template(arg);
	if(!new_scheme){
		ch->printlnf("Could not find scheme/template '%s' to change the level of.", argument);
		return;
	}

	if(new_scheme->template_protected_by_level>get_trust(ch)){
		ch->printlnf("The scheme %s is rated at level %d, so you can't edit it.",
			new_scheme->template_name,
			new_scheme->template_protected_by_level);
		return;
	}
	int newlevel=atoi(argument);
	if(newlevel>get_trust(ch)|| newlevel<LEVEL_IMMORTAL){
		ch->printlnf("The level must be in the range %d to %d", LEVEL_IMMORTAL, get_trust(ch));
		return;
	}

	ch->printlnf("Colour schemes '%s' level changed from %d to %d",
		new_scheme->template_name,
		new_scheme->template_protected_by_level,
		newlevel);
	new_scheme->template_protected_by_level=newlevel;
	
	save_colour_templates();
	ch->println("Colour schemes/templates resaved.");
}
/**************************************************************************/
void custom_colour_rename(char_data *ch, char *argument)
{
	COLOUR_TEMPLATE_TYPE *new_scheme;

	char arg[MIL];
	argument=one_argument(argument, arg);

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour rename <scheme/template number> new name");
		return;
	}
	if(!is_number(arg)){
		ch->println("You must specify a scheme by its number.");
		return;
	}
	new_scheme=find_colour_template(arg);
	if(!new_scheme){
		ch->printlnf("Could not find scheme/template '%s' to rename.", argument);
		return;
	}

	COLOUR_TEMPLATE_TYPE *dup=find_colour_template(argument);
	if(dup){
		ch->printlnf("There already exists a scheme '%s', you need something more unique.", 
			dup->template_name);
		return;
	}

	if(new_scheme->template_protected_by_level>get_trust(ch)){
		ch->printlnf("The scheme %s is rated at level %d, so you can't edit it.",
			new_scheme->template_name,
			new_scheme->template_protected_by_level);
		return;
	}

	ch->printlnf("Colour schemes '%s' description '%s' renamed to '%s'",
		new_scheme->template_name,
		new_scheme->template_description,
		argument);
	replace_string(new_scheme->template_name, argument);
	
	save_colour_templates();
	ch->println("Colour schemes/templates resaved");
}

/**************************************************************************/
void custom_colour_show_channels(char_data *ch)
{
	int i=0;		
	ch->println("\r\n=======Current channel settings:");
	while(!IS_NULLSTR(channels[i].name)){
		ch->printlnf("Channel: %-20s  Strip Colour:%s`x",
			channels[i].name,
			(IS_SET(TRUE_CH(ch)->pcdata->strip_colour_on_channel,
				channels[i].channel_flag_value)?"`BY":"`GN")
				);	
		i++;
	}
}

/**************************************************************************/
void custom_colour_channel(char_data *ch, char *argument)
{
	int i;
	if(!TRUE_CH(ch)->pcdata){
		ch->println("Players only sorry.");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour channel <channelname> <Y/N>");
		ch->println("This command allows you to disable colour codes in various channels");
		custom_colour_show_channels(ch);
		return;
	}

	i=channel_lookup(argument);
	if ( i>-1)
	{
		TRUE_CH(ch)->pcdata->strip_colour_on_channel^= channels[i].channel_flag_value;
		ch->printlnf( "%s channel colour flag toggled.", channels[i].name);		
		custom_colour_show_channels(ch);
	}else{
		ch->printlnf("Couldn't find any matching channels called '%s'", argument);
		custom_colour_channel(ch, "");
		return;
	}
}
/**************************************************************************/
void custom_colour_prefix(char_data *ch, char*argument)
{	
	// The translation system will automatically convert ` into `` if in use
	if(argument[0]==COLOURCODE && argument[1]==COLOURCODE){
		argument[1]='\0';
	}

	if(str_len(argument)>1 || IS_NULLSTR(argument)){
		ch->println("syntax: colour prefix <colour prefix character>");
		ch->println("This command is used to specify which character you ");
		ch->println("want to prefix colour codes with.");
		if(ch->colour_prefix!=COLOURCODE){
			ch->printlnfbw("Your current colour code prefix is set to %c", ch->colour_prefix);
		}else{
			ch->println("Your current colour code prefix is unset,");
			ch->printlnfbw(" therefore you should prefix colours with the default %c", COLOURCODE);
		}
		ch->println("Note: The prefix system doesn't affect mudftp connections.");
		return;
	}

	ch->printlnfbw("Colour code prefix changed from %c to %s", ch->colour_prefix, argument);

	ch->colour_prefix=*argument;
}
/**************************************************************************/
void custom_colour_flashing(char_data *ch, char *argument)
{
	if(!ch->desc){
		ch->println("players only");
		return;
	}
	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour flashing -");
		ch->println("To toggle flashing support status.");
		if(ch->desc->flashing_disabled){
			ch->println("Flashing text is currently disabled.");
		}else{
			ch->println("Flashing text is currently enabled.");
		}
		return;
	}

	if(ch->desc->flashing_disabled){
		ch->desc->flashing_disabled=false;
		ch->println("Flashing text is enabled.");
	}else{
		ch->desc->flashing_disabled=true;
		ch->println("Flashing text is disabled.");
	}
	if(TRUE_CH(ch)->pcdata){
		TRUE_CH(ch)->pcdata->flashing_disabled=ch->desc->flashing_disabled;
	}
}
/**************************************************************************/
/**************************************************************************/
void custom_colour_socials_colour(char_data *ch, char *argument)
{
	if(!ch->desc){
		ch->println("players only");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: colour socialscolour Y/N/D");
		ch->println("To toggle the displaying of colour in socials.");
		ch->println("Y = On  - allow colour in socials.");
		ch->println("N = Off - don't allow in socials.");
		ch->printlnf("D = Autosense - use the mud default setting (currently %s).",
			GAMESETTING4(GAMESET4_GAMEDEFAULT_COLOUR_IN_SOCIALS_ON)?"yes":"no");
		ch->wrapln("Note: the colour only appears if the mud actually "
			"has socials that use colour.");
		ch->println("");
		ch->printlnf("You currently have your socials colour set to %s",
			preference_word(ch->pcdata->preference_colour_in_socials));
		return;
	}

	if(!str_prefix(argument, "yes") || !str_prefix(argument, "on")){
		ch->pcdata->preference_colour_in_socials=PREF_ON;
		ch->println("Colour in socials is now enabled for socials.");
	}else if(!str_prefix(argument, "default") || !str_prefix(argument, "autosense")){
		ch->pcdata->preference_colour_in_socials=PREF_AUTOSENSE;
		ch->println("Colour in socials is now using the mud default setting.");
	}else if(!str_prefix(argument, "no") || !str_prefix(argument, "off")){
		ch->pcdata->preference_colour_in_socials=PREF_OFF;
		ch->println("You will no longer see colour in socials.");
	}else{
		ch->printlnf("Unsupported option '%s'", argument);
		custom_colour_socials_colour(ch,"");
		return;
	}
}
/**************************************************************************/
/**************************************************************************/

