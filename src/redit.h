/**************************************************************************/
// redit.h - 
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

#ifndef REDIT_H
#define REDIT_H

//prototypes
DECLARE_OLC_FUN( redit_show			);
DECLARE_OLC_FUN( redit_create		);
DECLARE_OLC_FUN( redit_name			);
DECLARE_OLC_FUN( redit_desc			);
DECLARE_OLC_FUN( redit_ed			);
DECLARE_OLC_FUN( redit_wrap			);
DECLARE_OLC_FUN( redit_north		);
DECLARE_OLC_FUN( redit_south		);
DECLARE_OLC_FUN( redit_east			);
DECLARE_OLC_FUN( redit_west			);
DECLARE_OLC_FUN( redit_up			);
DECLARE_OLC_FUN( redit_down			);
DECLARE_OLC_FUN( redit_northeast    );
DECLARE_OLC_FUN( redit_southeast    );
DECLARE_OLC_FUN( redit_southwest    );
DECLARE_OLC_FUN( redit_northwest    );
DECLARE_OLC_FUN( redit_mreset		);
DECLARE_OLC_FUN( redit_oreset		);
DECLARE_OLC_FUN( redit_mlist		);
DECLARE_OLC_FUN( redit_rlist		);
DECLARE_OLC_FUN( redit_olist		);
DECLARE_OLC_FUN( redit_heal			);
DECLARE_OLC_FUN( redit_mana			);
DECLARE_OLC_FUN( redit_clan			);
DECLARE_OLC_FUN( redit_owner		);
DECLARE_OLC_FUN( redit_room			);
DECLARE_OLC_FUN( redit_room2		);
DECLARE_OLC_FUN( redit_sector		);
DECLARE_OLC_FUN( redit_copy			);
DECLARE_OLC_FUN( redit_msp_roomsound);
DECLARE_OLC_FUN( redit_addecho		);
DECLARE_OLC_FUN( redit_delecho		);
DECLARE_OLC_FUN( redit_usedby		); // Kal
DECLARE_OLC_FUN( redit_delete		); // Kal
DECLARE_OLC_FUN( redit_rdelete		); // Kal
DECLARE_OLC_FUN( redit_setlockers	); // Kal

// table  
const struct olc_cmd_type redit_table[] =
{
//  {   command		function		},
    {   "?",		show_help, "", 1},
    {   "addecho",  redit_addecho   },
    {   "clan",     redit_clan      },
    {   "commands", show_commands   },
    {   "create",   redit_create    },
    {   "down",     redit_down      },
    {   "delecho",  redit_delecho   },
    {   "delete",   redit_delete    },
    {   "desc",     redit_desc      },
    {   "east",     redit_east      },
    {   "ed",       redit_ed        },
    {   "format",   redit_wrap      },
    {   "heal",     redit_heal      },
    {   "mana",     redit_mana      },
    {   "mreset",   redit_mreset    },
    {   "msp",      redit_msp_roomsound},
    {   "north",    redit_north     },
    {   "ne",       redit_northeast },
    {   "northeast",redit_northeast },
    {   "northwest",redit_northwest },
    {   "nw",       redit_northwest },
    {   "name",     redit_name      },
    {   "oreset",   redit_oreset    },
    {   "owner",    redit_owner     },
    {   "rcopy",    redit_copy      },
    {   "rdelete",  redit_rdelete   },
    {   "room",     redit_room      },
    {   "room2",    redit_room2     },
    {   "south",    redit_south     },
    {   "southeast",redit_southeast },
    {   "southwest",redit_southwest },
    {   "sa",       olcex_showafter, "", 1},
    {   "sca",      olcex_showcommandafter, "", 1},
    {   "se",       redit_southeast },
    {   "sector",   redit_sector    },
    {   "setlockers",redit_setlockers    },
    {   "sfa",      olcex_showflagsafter, "", 1},
    {   "show",     redit_show      },
    {   "showflags",olcex_showflags     },
    {   "sw"       ,redit_southwest },
    {   "up",       redit_up        },
    {   "up",       redit_up        },
    {   "usedby",   redit_usedby    },
    {   "west",     redit_west      },
    {   "wrap",     redit_wrap      },
    {	NULL,		0,		}
};

/**************************************************************************/
#endif // REDIT_H
