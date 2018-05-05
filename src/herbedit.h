/**************************************************************************/
// herbedit.h - OLC based herb editor, Kerenos.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef HERBEDIT_H
#define HERBEDIT_H

DECLARE_OLC_FUN( herbedit_name					);
DECLARE_OLC_FUN( herbedit_season				);
DECLARE_OLC_FUN( herbedit_month					);
DECLARE_OLC_FUN( herbedit_sector				);
DECLARE_OLC_FUN( herbedit_continent				);
DECLARE_OLC_FUN( herbedit_show					);
DECLARE_OLC_FUN( herbedit_timefield				);
DECLARE_OLC_FUN( herbedit_area					);
DECLARE_OLC_FUN( herbedit_vnum					);
DECLARE_OLC_FUN( herbedit_difficulty			);

const struct olc_cmd_type herbedit_table[] =
{
//	{	COMMAND			FUNCTION				}
	{	"area",			herbedit_area			},
	{   "commands",		show_commands			},
	{	"continent",	herbedit_continent		},
	{	"difficulty",	herbedit_difficulty		},
	{	"month",		herbedit_month			},
	{	"name",			herbedit_name			},
	{	"season",		herbedit_season			},
	{	"sector",		herbedit_sector			},
	{	"show",			herbedit_show			},
	{	"time",			herbedit_timefield		},
	{	"vnum",			herbedit_vnum			},
	{	"?",			show_help				},
	{	NULL,			0,						}
};

#endif	// HERBEDIT_H

