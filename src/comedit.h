/**************************************************************************/
// comedit.h - Code dealing with dynamic editing of commands, Ker & Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef COMEDIT_H
#define COMEDIT_H

DECLARE_OLC_FUN( comedit_inviscmds			);
DECLARE_OLC_FUN( comedit_level				);
DECLARE_OLC_FUN( comedit_logtype			);
DECLARE_OLC_FUN( comedit_show				);
DECLARE_OLC_FUN( comedit_position			);
DECLARE_OLC_FUN( comedit_visible			);
DECLARE_OLC_FUN( comedit_flags				);
DECLARE_OLC_FUN( comedit_council			);
DECLARE_OLC_FUN( comedit_category			);
DECLARE_OLC_FUN( comedit_grantgroups		);

const struct olc_cmd_type comedit_table[] =
{
//	{	COMMAND		FUNCTION				}
	{	"commands",	show_commands			},
	{	"category",	comedit_category		},
	{	"council",	comedit_council			},
	{	"flags",	comedit_flags			},
	{	"grantgroups",comedit_grantgroups	},
	{	"inviscmds",comedit_inviscmds		},
	{	"level",	comedit_level			},
	{	"log",		comedit_logtype			},
	{	"position",	comedit_position		},
	{	"show",		comedit_show			},
	{	"showflags",olcex_showflags			},
    {	"sa",		olcex_showafter			},
    {	"sfa",		olcex_showflagsafter	},
	{	"sca",		olcex_showcommandafter	},	    
	{	"visible",	comedit_visible			},
	{	"?",		show_help				},
	{	NULL,		0,						}
};

/**************************************************************************/
#endif // CEDIT_H
