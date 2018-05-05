/**************************************************************************/
// langedit.h - online olc based language editor 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef LANGEDIT_H
#define LANGEDIT_H

//prototypes - Kal
DECLARE_OLC_FUN( langedit_show	);
DECLARE_OLC_FUN( langedit_rename	);
DECLARE_OLC_FUN( langedit_flags	);
DECLARE_OLC_FUN( langedit_words	);
DECLARE_OLC_FUN( langedit_addword);
DECLARE_OLC_FUN( langedit_delword);
DECLARE_OLC_FUN( langedit_recreatebase);
DECLARE_OLC_FUN( langedit_skillname);
DECLARE_OLC_FUN( langedit_commandname);

// table  
const struct olc_cmd_type langedit_table[] =
{
//	{	COMMAND		FUNCTION		}
	{	"name",				langedit_rename			},
	{	"flags",			langedit_flags			},
	{	"addword",			langedit_addword		},
	{	"delword",			langedit_delword		},
	{	"recreatebase",		langedit_recreatebase	},
	{	"words",			langedit_words			},
	{	"skillname",		langedit_skillname		},	
    {   "sa",				olcex_showafter,		"", 1},
    {   "sca",				olcex_showcommandafter, "", 1},    
    {   "sfa",				olcex_showflagsafter,	"", 1},
	{	"show",				langedit_show			},
	{   "showflags",		olcex_showflags			},
	{	"commands",			show_commands			},
	{	"commandname",		langedit_commandname    },
	{	NULL,				0}
};
/**************************************************************************/
#endif // LANGEDIT_H
