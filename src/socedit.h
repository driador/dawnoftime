/**************************************************************************/
// socedit.h - online olc based social editor 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef SOCEDIT_H
#define SOCEDIT_H

//prototypes - Kal
DECLARE_OLC_FUN( socedit_show		);
DECLARE_OLC_FUN( socedit_name		);
DECLARE_OLC_FUN( socedit_socialflags);
DECLARE_OLC_FUN( socedit_positionflags);
DECLARE_OLC_FUN( socedit_act	);
DECLARE_OLC_FUN( socedit_sdelete );

// table  
const struct olc_cmd_type socedit_table[] =
{
//	{	COMMAND		FUNCTION		}
	{	"act",				socedit_act				},	
	{	"name",				socedit_name			},
	{	"positionflags",	socedit_positionflags	},
	{	"socialflags",		socedit_socialflags		},
	{	"sdelete",			socedit_sdelete			},
    {   "sa",				olcex_showafter,		"", 1},
    {   "sca",				olcex_showcommandafter, "", 1},    
    {   "sfa",				olcex_showflagsafter,	"", 1},
	{	"show",				socedit_show			},
	{   "showflags",		olcex_showflags			},
	{	"commands",			show_commands			},
	{	NULL,				0}
};
/**************************************************************************/
#endif // SOCEDIT_H
