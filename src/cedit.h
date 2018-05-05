/**************************************************************************/
// cedit.h - olc class editor header
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef CEDIT_H
#define CEDIT_H

//prototypes - Kal
DECLARE_OLC_FUN( classedit_show		);
DECLARE_OLC_FUN( classedit_flags	);
DECLARE_OLC_FUN( classedit_remort	);
DECLARE_OLC_FUN( classedit_hpmintrain);
DECLARE_OLC_FUN( classedit_hpmaxtrain);
DECLARE_OLC_FUN( classedit_prime0);
DECLARE_OLC_FUN( classedit_prime1);
DECLARE_OLC_FUN( classedit_adept);
DECLARE_OLC_FUN( classedit_thac0);
DECLARE_OLC_FUN( classedit_thac32);
DECLARE_OLC_FUN( classedit_fullmana);
DECLARE_OLC_FUN( classedit_casttype);
DECLARE_OLC_FUN( classedit_spinfoletter);
DECLARE_OLC_FUN( classedit_defaultgroup);
DECLARE_OLC_FUN( classedit_creation);
DECLARE_OLC_FUN( classedit_shortname);
DECLARE_OLC_FUN( classedit_copy);
DECLARE_OLC_FUN( classedit_recall	);
DECLARE_OLC_FUN( classedit_showpose);
DECLARE_OLC_FUN( classedit_setpose);
DECLARE_OLC_FUN( classedit_morgue);
DECLARE_OLC_FUN( classedit_rename);
DECLARE_OLC_FUN( classedit_addremortclass);
DECLARE_OLC_FUN( classedit_setremortclasses);
DECLARE_OLC_FUN( classedit_newbie_prac_location_hint );
DECLARE_OLC_FUN( classedit_newbie_train_location_hint);
DECLARE_OLC_FUN( classedit_pendant_vnum);
DECLARE_OLC_FUN( classedit_newbiemap);

// table  
const struct olc_cmd_type classedit_table[] =
{
//	{	COMMAND		FUNCTION		}
	{	"adept",		classedit_adept		},
	{	"casttype",		classedit_casttype	},
	{	"creation",		classedit_creation	},
	{	"commands",		show_commands		},
	{	"copy",			classedit_copy		},	
	{	"defaultgroup",	classedit_defaultgroup},
	{	"flags",		classedit_flags		},
	{	"fullmana",		classedit_fullmana	},
	{	"hpmintrain",	classedit_hpmintrain	},
	{	"hpmaxtrain",	classedit_hpmaxtrain	},
	{	"morgue",		classedit_morgue	},
	{	"prime0",		classedit_prime0	},
	{	"prime1",		classedit_prime1	},
	{	"recall",		classedit_recall	},		
	{	"remort",		classedit_remort	},
	{	"addremortclass",	classedit_addremortclass},
	{	"setremortclasses",	classedit_setremortclasses},
	{	"show",			classedit_show		},
    {	"showflags",	olcex_showflags		},
    {	"showpose",		classedit_showpose	},
    {	"setpose",		classedit_setpose	},	
    {	"sa",			olcex_showafter		},
    {	"sfa",			olcex_showflagsafter},
	{	"sca",			olcex_showcommandafter},	
	{	"shortname",		classedit_shortname	},
	{	"spinfoletter",	classedit_spinfoletter},	
	{	"thac0",		classedit_thac0		},
	{	"thac32",		classedit_thac32	},
	{	"?",			show_help			},
//	{	"rename",		classedit_rename	}, // class renaming disabled by default
	{	"newbie_prac_location_hint",	classedit_newbie_prac_location_hint},
	{	"newbie_train_location_hint",	classedit_newbie_train_location_hint},
	{	"pendant_vnum",					classedit_pendant_vnum},
	{	"newbiemap",					classedit_newbiemap},

	{	NULL,			0,					}
};
/**************************************************************************/
#endif // CEDIT_H
