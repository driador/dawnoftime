/**************************************************************************/
// gameedit.h - olc based game settings editor header, Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef GAMEEDIT_H
#define GAMEEDIT_H

//prototypes - Kal
DECLARE_OLC_FUN( gameedit_show		);
DECLARE_OLC_FUN( gameedit_showvalues);
DECLARE_OLC_FUN( gameedit_gamename	);
DECLARE_OLC_FUN( gameedit_flags		);
DECLARE_OLC_FUN( gameedit_flags2	);
DECLARE_OLC_FUN( gameedit_flags3	);
DECLARE_OLC_FUN( gameedit_flags4	);
DECLARE_OLC_FUN( gameedit_flags5	);
DECLARE_OLC_FUN( gameedit_flagsmf1	);
DECLARE_OLC_FUN( gameedit_mintotal	);
DECLARE_OLC_FUN( gameedit_maxtotal	);
DECLARE_OLC_FUN( gameedit_icyear_offset);
DECLARE_OLC_FUN( gameedit_value);
DECLARE_OLC_FUN( gameedit_loginprompt);
DECLARE_OLC_FUN( gameedit_defaultprompt);
DECLARE_OLC_FUN( gameedit_mspurl);
DECLARE_OLC_FUN( gameedit_createpass);
DECLARE_OLC_FUN( gameedit_connectpass);
DECLARE_OLC_FUN( gameedit_no_resolve_ip);
DECLARE_OLC_FUN( gameedit_default_newbie_security_on_olc_port);
DECLARE_OLC_FUN( gameedit_areaimportformat);
DECLARE_OLC_FUN( gameedit_setwhoformat);
DECLARE_OLC_FUN( gameedit_stylesheet);
DECLARE_OLC_FUN( gameedit_dsv);
DECLARE_OLC_FUN( gameedit_sethelpheader);
DECLARE_OLC_FUN( gameedit_sethelpfooter);
DECLARE_OLC_FUN( gameedit_sethelpprevnextfooter);
DECLARE_OLC_FUN( gameedit_set);

// table  
const struct olc_cmd_type gameedit_table[] =
{
//	{	COMMAND		FUNCTION		}
	{	"areaimportformat",	gameedit_areaimportformat},
	{	"createpass",	gameedit_createpass },
	{	"connectpass",	gameedit_connectpass},	
	{	"default_sec4olc",gameedit_default_newbie_security_on_olc_port},		
	{	"flags",		gameedit_flags		},
	{	"flags2",		gameedit_flags2		},
	{	"flag2",		gameedit_flags2,	"",	true}, // hidden
	{	"flags3",		gameedit_flags3		},
	{	"flag3",		gameedit_flags3,	"",	true}, // hidden
	{	"flags4",		gameedit_flags4		},
	{	"flag4",		gameedit_flags4,	"",	true}, // hidden
	{	"flags5",		gameedit_flags5		},
	{	"flag5",		gameedit_flags5,	"",	true}, // hidden
	{	"flagsmf1",		gameedit_flagsmf1	},
	{	"flagmf1",		gameedit_flagsmf1,	"",	true}, // hidden
	{	"gamename",		gameedit_gamename	},
	{	"icyear_offset",gameedit_icyear_offset},	
	{	"loginprompt",	gameedit_loginprompt},
	{	"defaultprompt",gameedit_defaultprompt},	
	{	"mintotal",		gameedit_mintotal	},
	{	"maxtotal",		gameedit_maxtotal	},
	{	"mspurl",		gameedit_mspurl		},	
	{	"no_resolve_ip",gameedit_no_resolve_ip},	
	{	"show",			gameedit_show		},
    {	"showflags",	olcex_showflags	},
    {	"sa",			olcex_showafter	},
    {	"sfa",			olcex_showflagsafter},
	{	"sca",			olcex_showcommandafter},	
	{	"showvalues",	gameedit_showvalues	},
	{	"stylesheet",	gameedit_stylesheet},	
	{	"set",			gameedit_set,		"", true},	// hidden
	{	"sethelpheader",gameedit_sethelpheader},	
	{	"sethelpfooter",gameedit_sethelpfooter},
	{	"sethelpprevnextfooter",gameedit_sethelpprevnextfooter},
	{	"value",		gameedit_value		},	
	{	"setwhoformat",	gameedit_setwhoformat},	
	{	"dsv",			gameedit_dsv},	
	{	"commands",		show_commands		},	
	{	"?",			show_help			},
	{	NULL,			0,					}
};
/**************************************************************************/
#endif // GAMEEDIT_H
