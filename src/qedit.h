/**************************************************************************/
// qedit.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef QEDIT_H
#define QEDIT_H

DECLARE_OLC_FUN( qedit_show				);
DECLARE_OLC_FUN( qedit_create			);
DECLARE_OLC_FUN( qedit_questname		);
DECLARE_OLC_FUN( qedit_responsible		);
DECLARE_OLC_FUN( qedit_status			);
DECLARE_OLC_FUN( qedit_resource			);
DECLARE_OLC_FUN( qedit_help				);
DECLARE_OLC_FUN( qedit_synopsis			);
const struct olc_cmd_type qedit_table[] =
{
//	{	COMMAND			FUNCTION		},
	{	"show",			qedit_show		},
	{	"create",		qedit_create	},
	{	"questname",	qedit_questname	},
	{	"responsible",	qedit_responsible	},
	{	"status",		qedit_status	},
	{	"currentstatus",qedit_status	},
	{	"resources",	qedit_resource	},
	{	"help",			qedit_help		},
	{	"immhelp",		qedit_help		},
	{	"synopsis",		qedit_synopsis	},
    {   "commands",		show_commands	},
	{	"?",			show_help		},
	{	NULL,		0,				}
};

/**************************************************************************/
#endif // QEDIT_H

