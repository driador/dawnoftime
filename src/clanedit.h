/**************************************************************************/
// clanedit.cpp - olc clan editor, Tibault
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef CLANEDIT_H
#define CLANEDIT_H

DECLARE_OLC_FUN( clanedit_name					);
DECLARE_OLC_FUN( clanedit_show					);
DECLARE_OLC_FUN( clanedit_whoname				);
DECLARE_OLC_FUN( clanedit_whocat				);
DECLARE_OLC_FUN( clanedit_notename				);
DECLARE_OLC_FUN( clanedit_description			);
DECLARE_OLC_FUN( clanedit_colour				);
DECLARE_OLC_FUN( clanedit_ranktitle				);
DECLARE_OLC_FUN( clanedit_canadd				);
DECLARE_OLC_FUN( clanedit_canpromote			);
DECLARE_OLC_FUN( clanedit_canremove				);
DECLARE_OLC_FUN( clanedit_recallroom			);
DECLARE_OLC_FUN( clanedit_delete				);
DECLARE_OLC_FUN( clanedit_withdraw				);
DECLARE_OLC_FUN( clanedit_bankroom				);

/**************************************************************************/
const struct olc_cmd_type clanedit_table[] =
{
//	{	COMMAND			FUNCTION				}
	{	"commands",		show_commands			},
	{	"description",	clanedit_description	},
	{	"name",			clanedit_name			},
	{	"whoname",		clanedit_whoname		},
	{	"whoshort",		clanedit_whocat			},
	{	"notename",		clanedit_notename		},
	{	"clancolour",	clanedit_colour			},
	{   "ranktitle",    clanedit_ranktitle		},
	{	"canadd",		clanedit_canadd			},
	{	"canpromote",	clanedit_canpromote		},
	{	"canoutcast",	clanedit_canremove		},
	{	"recallroom",	clanedit_recallroom		},
	{	"delete",		clanedit_delete			},
	{	"withdraw",		clanedit_withdraw		},
	{	"bankroom",		clanedit_bankroom		},
	{	"show",			clanedit_show			},
	{	"?",			show_help				},
	{	NULL,			0,						}
};
/**************************************************************************/
#endif // CLANEDIT_H

