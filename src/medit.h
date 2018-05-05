/**************************************************************************/
// medit.h - 
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

#ifndef MEDIT_H
#define MEDIT_H

//prototypes
DECLARE_OLC_FUN( medit_show			);
DECLARE_OLC_FUN( medit_create		);
DECLARE_OLC_FUN( medit_name			);
DECLARE_OLC_FUN( medit_short		);
DECLARE_OLC_FUN( medit_long			);
DECLARE_OLC_FUN( medit_shop			);
DECLARE_OLC_FUN( medit_desc			);
DECLARE_OLC_FUN( medit_wrap			);
DECLARE_OLC_FUN( medit_level		);
DECLARE_OLC_FUN( medit_alliance		);
DECLARE_OLC_FUN( medit_tendency		);
DECLARE_OLC_FUN( medit_spec			);
DECLARE_OLC_FUN( medit_sex			);  
DECLARE_OLC_FUN( medit_act			);  
DECLARE_OLC_FUN( medit_affect		);  
DECLARE_OLC_FUN( medit_ac			);  
DECLARE_OLC_FUN( medit_form			);  
DECLARE_OLC_FUN( medit_part			);  
DECLARE_OLC_FUN( medit_imm			);  
DECLARE_OLC_FUN( medit_res			);  
DECLARE_OLC_FUN( medit_vuln			);  
DECLARE_OLC_FUN( medit_material		);  
DECLARE_OLC_FUN( medit_off			);  
DECLARE_OLC_FUN( medit_size			);  
DECLARE_OLC_FUN( medit_hitdice		);  
DECLARE_OLC_FUN( medit_manadice		);  
DECLARE_OLC_FUN( medit_damdice		);  
DECLARE_OLC_FUN( medit_race			);  
DECLARE_OLC_FUN( medit_position		);  
DECLARE_OLC_FUN( medit_wealth		);  
DECLARE_OLC_FUN( medit_hitroll		);  
DECLARE_OLC_FUN( medit_damtype		);  
DECLARE_OLC_FUN( medit_group		);  
DECLARE_OLC_FUN( medit_helpgroup	);  
DECLARE_OLC_FUN( medit_addmprog		);  
DECLARE_OLC_FUN( medit_delmprog		);  
DECLARE_OLC_FUN( medit_xpmod		);  
DECLARE_OLC_FUN( medit_copy			);
DECLARE_OLC_FUN( medit_act2			);
DECLARE_OLC_FUN( medit_affect2		);
DECLARE_OLC_FUN( medit_gamble		);
DECLARE_OLC_FUN( medit_posmprog		);
DECLARE_OLC_FUN( medit_autostat		);
DECLARE_OLC_FUN( medit_delete		);
DECLARE_OLC_FUN( medit_mdelete		);
DECLARE_OLC_FUN( medit_help			);
DECLARE_OLC_FUN( medit_inn			);
DECLARE_OLC_FUN( medit_affect3		);

// table  
const struct olc_cmd_type medit_table[] =
{
//	{	command 		function	},
    {   "?",            show_help, "", 1},
    {   "act",          medit_act       }, 
    {   "act2",         medit_act2      },
    {   "addmprog",     medit_addmprog  }, 
    {   "affect",       medit_affect    },
    {   "affect2",      medit_affect2   }, 
    {   "affect3",      medit_affect3   }, 
    {   "alliance",     medit_alliance  },
    {   "armor",        medit_ac        }, 
    {   "autostat",     medit_autostat  },
    {   "commands",     show_commands   },
    {   "create",       medit_create    },
    {   "damdice",      medit_damdice   }, 
    {   "damtype",      medit_damtype   }, 
    {   "default",      medit_long, "", 1},
    {   "delete",       medit_delete    }, 
    {   "delmprog",     medit_delmprog  }, 
    {   "desc",         medit_desc      },
    {   "form",         medit_form      }, 
    {   "gamble",       medit_gamble    },
    {   "group",        medit_group     },  
    {   "help",         medit_help      },  
    {   "helpgroup",    medit_helpgroup },  
    {   "hitdice",      medit_hitdice   }, 
    {   "hitroll",      medit_hitroll   }, 
    {   "imm",          medit_imm       }, 
    {   "inn",          medit_inn       },
    {   "level",        medit_level     },
    {   "manadice",     medit_manadice  }, 
    {   "material",     medit_material  }, 
    {   "mcopy",        medit_copy      },
    {   "mdelete",      medit_mdelete   }, 
    {   "name",         medit_name      },
    {   "off",          medit_off       }, 
    {   "part",         medit_part      }, 
    {   "position",     medit_position  }, 
    {   "posmprog",     medit_posmprog  },  
    {   "race",         medit_race      }, 
    {   "res",          medit_res       }, 
    {   "sa",           olcex_showafter, "", 1},
    {   "sca",          olcex_showcommandafter, "", 1},    
    {   "sex",          medit_sex       },
    {   "sfa",          olcex_showflagsafter, "", 1},
    {   "shop",         medit_shop      },
    {   "short",        medit_short     },
    {   "show",         medit_show      },
    {   "showflags",    olcex_showflags },
    {   "size",         medit_size      }, 
    {   "spec",         medit_spec      },
    {   "tendency",     medit_tendency  },
    {   "vuln",         medit_vuln      }, 
    {   "wealth",       medit_wealth    }, 
    {   "wrap",         medit_wrap      },
    {   "xpmod",        medit_xpmod     },
	{	NULL,			0,				}
};
/**************************************************************************/
#endif // MEDIT_H
