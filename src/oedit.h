/**************************************************************************/
// oedit.h - 
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

#ifndef OEDIT_H
#define OEDIT_H

//prototypes
DECLARE_OLC_FUN( oedit_show			);
DECLARE_OLC_FUN( oedit_create		);
DECLARE_OLC_FUN( oedit_name			);
DECLARE_OLC_FUN( oedit_short		);
DECLARE_OLC_FUN( oedit_long			);
DECLARE_OLC_FUN( oedit_nolong		);
DECLARE_OLC_FUN( oedit_value0		);
DECLARE_OLC_FUN( oedit_value1		);
DECLARE_OLC_FUN( oedit_value2		);
DECLARE_OLC_FUN( oedit_value3		);
DECLARE_OLC_FUN( oedit_value4		);
DECLARE_OLC_FUN( oedit_weight		);
DECLARE_OLC_FUN( oedit_cost			);
DECLARE_OLC_FUN( oedit_ed			);
DECLARE_OLC_FUN( oedit_extra		);
DECLARE_OLC_FUN( oedit_wear			);
DECLARE_OLC_FUN( oedit_type			);
DECLARE_OLC_FUN( oedit_affect		);
DECLARE_OLC_FUN( oedit_material		);
DECLARE_OLC_FUN( oedit_level		);
DECLARE_OLC_FUN( oedit_condition	);
DECLARE_OLC_FUN( oedit_rsize		);
DECLARE_OLC_FUN( oedit_asize		);
DECLARE_OLC_FUN( oedit_copy			);
DECLARE_OLC_FUN( oedit_addrestrict	);
DECLARE_OLC_FUN( oedit_delrestrict	);
DECLARE_OLC_FUN( oedit_addskill		);	// Kerenos
DECLARE_OLC_FUN( oedit_extra2		);
DECLARE_OLC_FUN( oedit_classallowances);
DECLARE_OLC_FUN( oedit_addspell		);	// Kal
DECLARE_OLC_FUN( oedit_attune		);
DECLARE_OLC_FUN( oedit_addmodifier	);
DECLARE_OLC_FUN( oedit_addflag		); // Kal
DECLARE_OLC_FUN( oedit_delaffect	);
DECLARE_OLC_FUN( oedit_delete		); // Kal
DECLARE_OLC_FUN( oedit_odelete		); // Kal
DECLARE_OLC_FUN( oedit_addmprog		); // Kal, Apr04
DECLARE_OLC_FUN( oedit_delmprog		); // Kal, Apr04
// table  
const struct olc_cmd_type oedit_table[] =
{
//  {   command		function	},
    {   "?",            show_help, "", 1},
    {   "addflag",      oedit_addflag       },
    {   "addmodifier",  oedit_addmodifier   },
	{   "addmprog",		oedit_addmprog		},
	{   "delmprog",		oedit_delmprog		},
    {   "addrestrict",  oedit_addrestrict   },
    {   "addskill",     oedit_addskill      },
    {   "addspell",     oedit_addspell      },
    {   "asize",        oedit_asize         },
    {   "attune",       oedit_attune        },
    {   "classallowances",oedit_classallowances},
    {   "commands",     show_commands       },
    {   "condition",    oedit_condition     },
    {   "cost",         oedit_cost          },
    {   "create",       oedit_create        },
    {   "default",      oedit_long          },
    {   "delaffect",    oedit_delaffect     },
    {   "delete",       oedit_delete        },
    {   "delflag",      oedit_delaffect     },
    {   "delmodifier",  oedit_delaffect     },
    {   "delrestrict",  oedit_delrestrict   },
    {   "ed",           oedit_ed            },
    {   "extra",        oedit_extra         },
    {   "extra2",       oedit_extra2        },
    {   "level",        oedit_level         },
    {   "long",         oedit_long          },
    {   "material",     oedit_material      },
    {   "name",         oedit_name          },
    {   "nolong",       oedit_nolong        },
    {   "ocopy",        oedit_copy          },
    {   "odelete",      oedit_odelete       },
    {   "rsize",        oedit_rsize         },
    {   "sa",           olcex_showafter, "", 1},
    {   "sca",          olcex_showcommandafter, "", 1},
    {   "sfa",          olcex_showflagsafter, "", 1},
    {   "short",        oedit_short         },
    {   "show",         oedit_show          },
    {   "showflags",    olcex_showflags     },
    {   "type",         oedit_type          },
    {   "v0",           oedit_value0        },
    {   "v1",           oedit_value1        },
    {   "v2",           oedit_value2        },
    {   "v3",           oedit_value3        },
    {   "v4",           oedit_value4        },
    {   "wear",         oedit_wear          },
    {   "weight",       oedit_weight        },
    {	NULL,			0,					}
};
/**************************************************************************/
#endif // OEDIT_H
