/**************************************************************************/
// raceedit.h - olc raceedit header 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef RACEEDIT_H
#define RACEEDIT_H

//prototypes
DECLARE_OLC_FUN( raceedit_show);
DECLARE_OLC_FUN( raceedit_act);
DECLARE_OLC_FUN( raceedit_aff);
DECLARE_OLC_FUN( raceedit_aff2);
DECLARE_OLC_FUN( raceedit_aff3);
DECLARE_OLC_FUN( raceedit_form);
DECLARE_OLC_FUN( raceedit_parts);
DECLARE_OLC_FUN( raceedit_imm);
DECLARE_OLC_FUN( raceedit_res);
DECLARE_OLC_FUN( raceedit_vuln);
DECLARE_OLC_FUN( raceedit_off);
DECLARE_OLC_FUN( raceedit_tab1);
DECLARE_OLC_FUN( raceedit_tab2);
DECLARE_OLC_FUN( raceedit_tab3);
DECLARE_OLC_FUN( raceedit_tab4);
DECLARE_OLC_FUN( raceedit_tab5);
DECLARE_OLC_FUN( raceedit_size);
DECLARE_OLC_FUN( raceedit_flags);
DECLARE_OLC_FUN( raceedit_skills);
DECLARE_OLC_FUN( raceedit_language);
DECLARE_OLC_FUN( raceedit_classxp);
DECLARE_OLC_FUN( raceedit_remort);
DECLARE_OLC_FUN( raceedit_racemod);
DECLARE_OLC_FUN( raceedit_highsize);
DECLARE_OLC_FUN( raceedit_lowsize);
DECLARE_OLC_FUN( raceedit_maxhp);
DECLARE_OLC_FUN( raceedit_points);
DECLARE_OLC_FUN( raceedit_starthp);
DECLARE_OLC_FUN( raceedit_recall);
DECLARE_OLC_FUN( raceedit_death);
DECLARE_OLC_FUN( raceedit_minheight);
DECLARE_OLC_FUN( raceedit_maxheight);
DECLARE_OLC_FUN( raceedit_minweight);
DECLARE_OLC_FUN( raceedit_maxweight);
DECLARE_OLC_FUN( raceedit_foodvnum);
DECLARE_OLC_FUN( raceedit_nodynamic);
DECLARE_OLC_FUN( raceedit_shortname);
DECLARE_OLC_FUN( raceedit_morgue);
DECLARE_OLC_FUN( raceedit_newbiemap);

// table  
const struct olc_cmd_type raceedit_table[] =
{
//	{	COMMAND			FUNCTION		}
	{	"act",			raceedit_act	},
	{	"affects",		raceedit_aff	},
	{	"affect2",		raceedit_aff2	},
	{	"aff2",			raceedit_aff2	},
	{	"affect3",		raceedit_aff3	},
	{	"aff3",			raceedit_aff3	},
	{	"classxp",		raceedit_classxp},
	{	"death",		raceedit_death	},
	{	"flags",		raceedit_flags	},
	{	"foodvnum",		raceedit_foodvnum},
	{	"form",			raceedit_form	},
	{	"highsize",		raceedit_highsize},
	{	"imm",			raceedit_imm	},
	{	"language",		raceedit_language},
	{	"lowsize",		raceedit_lowsize},
	{	"maxhp",		raceedit_maxhp	},
	{	"maxheight",	raceedit_maxheight},
	{	"maxweight",	raceedit_maxweight},
	{	"minheight",	raceedit_minheight},
	{	"minweight",	raceedit_minweight},
	{	"morgue",		raceedit_morgue},
	{	"newbiemap",	raceedit_newbiemap},
	{	"nodynamic",	raceedit_nodynamic},
	{	"off",			raceedit_off	},
	{	"parts",		raceedit_parts	},
	{	"points",		raceedit_points	},
	{	"racemod",		raceedit_racemod},
	{	"recall",		raceedit_recall	},
	{	"remort",		raceedit_remort },
	{	"res",			raceedit_res	},
	{	"size",			raceedit_size	},
	{	"skills",		raceedit_skills	},
	{	"starthp",		raceedit_starthp},
	{	"vuln",			raceedit_vuln	},
	{	"show",			raceedit_show	},
	{	"shortname",	raceedit_shortname},	
	{	"showflags",	olcex_showflags	},
	{	"sa",			olcex_showafter	},
	{	"sfa",			olcex_showflagsafter},
	{	"sca",			olcex_showcommandafter},
	{	"commands",		show_commands	},
	{	"1",			raceedit_tab1	},
	{	"2",			raceedit_tab2	},
	{	"3",			raceedit_tab3	},
	{	"4",			raceedit_tab4	},
	{	"5",			raceedit_tab5	},
	{	"?",			show_help		},
	{	NULL,		0,					}
};
/**************************************************************************/
#endif // RACEEDIT_H
