/**************************************************************************/
// grpedit.h - olc based skill/spell group editing
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef GRPEDIT_H
#define GRPEDIT_H

// below olc.cpp include block only
#ifdef OLC_CPP_INCLUDE_BLOCK
//prototypes - Kal
DECLARE_OLC_FUN( skillgroupedit_show		);
DECLARE_OLC_FUN( skillgroupedit_addskill	);
DECLARE_OLC_FUN( skillgroupedit_removeskill	);
DECLARE_OLC_FUN( skillgroupedit_flags		);
DECLARE_OLC_FUN( skillgroupedit_remort		);
DECLARE_OLC_FUN( skillgroupedit_rename		);
DECLARE_OLC_FUN( skillgroupedit_cost		);
DECLARE_OLC_FUN( skillgroupedit_skillvalue	); // Kal, April 03

// table  
const struct olc_cmd_type skillgroupedit_table[] =
{
//	{	COMMAND		FUNCTION		}
	{	"addskill",		skillgroupedit_addskill	},
    {   "commands",		show_commands			},
	{	"cost",			skillgroupedit_cost		},
	{	"flags",		skillgroupedit_flags	}, 
	{	"removeskill",	skillgroupedit_removeskill},
	{	"remort",		skillgroupedit_remort	},
	{	"rename",		skillgroupedit_rename	},
	{	"skillvalue",	skillgroupedit_skillvalue},	
	{	"show",			skillgroupedit_show		},
    {   "showflags",    olcex_showflags			},
    {   "sa",			olcex_showafter,		"", 1},
    {   "sca",			olcex_showcommandafter, "", 1},    
    {   "sfa",			olcex_showflagsafter,	"", 1},
	{	"?",			show_help				},
	{	NULL,			0,					}
};
#endif // OLC_CPP_INCLUDE_BLOCK
/**************************************************************************/

#endif // GRPEDIT_H
