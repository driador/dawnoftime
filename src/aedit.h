/**************************************************************************/
// aedit.h - see below
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
/***************************************************************************
 *  FILE: aedit.h   - olc.cpp include file... adds the aedit command       *
 *                    table and prototypes... aedit.cpp holds all the      *
 *                    commands.											   *
 ***************************************************************************/
#ifndef AEDIT_H
#define AEDIT_H

//prototypes
DECLARE_OLC_FUN( aedit_show			);
DECLARE_OLC_FUN( aedit_colour		);
DECLARE_OLC_FUN( aedit_create		);
DECLARE_OLC_FUN( aedit_name			);
DECLARE_OLC_FUN( aedit_file			);
DECLARE_OLC_FUN( aedit_age			);
DECLARE_OLC_FUN( aedit_reset		);
DECLARE_OLC_FUN( aedit_security		);
DECLARE_OLC_FUN( aedit_builder		);
DECLARE_OLC_FUN( aedit_vnum			);
DECLARE_OLC_FUN( aedit_lvnum		);
DECLARE_OLC_FUN( aedit_uvnum		);
DECLARE_OLC_FUN( aedit_lrange		);
DECLARE_OLC_FUN( aedit_lcomment		);
DECLARE_OLC_FUN( aedit_credits		);
DECLARE_OLC_FUN( aedit_mapscale		);
DECLARE_OLC_FUN( aedit_maplevel		);
DECLARE_OLC_FUN( aedit_areaflags	);
DECLARE_OLC_FUN( aedit_olcflags		);
DECLARE_OLC_FUN( aedit_lock			);
DECLARE_OLC_FUN( aedit_use_buildrestricts);
DECLARE_OLC_FUN( aedit_buildrestricts);
DECLARE_OLC_FUN( aedit_continent	);
DECLARE_OLC_FUN( aedit_shortname	);
DECLARE_OLC_FUN( aedit_addecho		);
DECLARE_OLC_FUN( aedit_delecho		);

// table  
const struct olc_cmd_type aedit_table[] =
{
//  {   command		function	},
	{   "addecho",	aedit_addecho	},
    {   "age",		aedit_age	},
    {   "areaflags",aedit_areaflags	},
    {   "builder",	aedit_builder	}, 
    {   "commands",	show_commands	},
    {   "colour",	aedit_colour	},
    {   "color",	aedit_colour	},
	{	"continent",aedit_continent	},
    {   "create",	aedit_create	},
	{   "delecho",	aedit_delecho	},
    {   "filename",	aedit_file		},
    {   "name",		aedit_name		},
    {   "mapscale",	aedit_mapscale	}, 
    {   "maplevel",	aedit_maplevel	}, 
    {	"reset",	aedit_reset		},
    {   "security",	aedit_security	},
    {	"show",		aedit_show		},
    {	"showflags",olcex_showflags	},
    {	"sa",		olcex_showafter	},
    {	"sfa",		olcex_showflagsafter},
	{	"sca",		olcex_showcommandafter},	
    {	"shortname",aedit_shortname	},
    {   "vnum",		aedit_vnum		},
    {   "lvnum",	aedit_lvnum		},
    {   "uvnum",	aedit_uvnum		},
    {   "lrange",	aedit_lrange	},
    {   "lcomment",	aedit_lcomment	},
    {   "olcflags",	aedit_olcflags  },	
    {	"sa",			olcex_showafter		},
    {	"sfa",			olcex_showflagsafter},
	{	"sca",			olcex_showcommandafter},	
    {   "credits",	aedit_credits	},
    {   "?",		show_help		},
    {   "lock",		aedit_lock		},
    {   "use_buildrestricts",	aedit_use_buildrestricts},	
	{   "buildrestricts",	aedit_buildrestricts},
	{   "restricts",	aedit_buildrestricts},
    {	NULL,		0,		}
};
/**************************************************************************/
#endif // AEDIT_H
