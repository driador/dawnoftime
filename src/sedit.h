/**************************************************************************/
// sedit.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef SEDIT_H
#define SEDIT_H

//prototypes October 98 - Kal
DECLARE_OLC_FUN( sedit_show				);
DECLARE_OLC_FUN( sedit_category			);
DECLARE_OLC_FUN( sedit_realm			);
DECLARE_OLC_FUN( sedit_sphere			);
DECLARE_OLC_FUN( sedit_flags			);
DECLARE_OLC_FUN( sedit_noundamage		);
DECLARE_OLC_FUN( sedit_wearoff			);
DECLARE_OLC_FUN( sedit_objectwearoff	);
DECLARE_OLC_FUN( sedit_position			);
DECLARE_OLC_FUN( sedit_mana				);
DECLARE_OLC_FUN( sedit_beats			);
DECLARE_OLC_FUN( sedit_damtype			);
DECLARE_OLC_FUN( sedit_spellfunc		);
DECLARE_OLC_FUN( sedit_funclist			);
DECLARE_OLC_FUN( sedit_rename			);
DECLARE_OLC_FUN( sedit_element			);
DECLARE_OLC_FUN( sedit_component		);	// Ker
DECLARE_OLC_FUN( sedit_mspsound			);	// Ker & Kal
DECLARE_OLC_FUN( sedit_spell_group		);	// Cel & Jar
DECLARE_OLC_FUN( sedit_sect_restrict	);	// Cel & Jar
DECLARE_OLC_FUN( sedit_sect_enhance		);	// Cel & Jar
DECLARE_OLC_FUN( sedit_sect_dampen		);	// Cel & Jar
DECLARE_OLC_FUN( sedit_max				);	// Kal
DECLARE_OLC_FUN( sedit_racerestrict		);	// Ker
DECLARE_OLC_FUN( sedit_composition		);	// Ker
DECLARE_OLC_FUN( sedit_type				);	// Kal, June 04

// table  
const struct olc_cmd_type sedit_table[] =
{
//	{	COMMAND			FUNCTION				}
	{	"beats",		sedit_beats				},
	{	"commands",		show_commands			},
	{	"category",		sedit_category			},
	{	"component",	sedit_component			},
	{	"compositions",	sedit_composition		},
	{	"dampen",		sedit_sect_dampen		},
	{	"damtype",		sedit_damtype			},
	{	"element",		sedit_element			},
	{   "enhance",		sedit_sect_enhance		},
	{	"flags",		sedit_flags				},
	{	"funclist",		sedit_funclist			},
	{	"mana",			sedit_mana				},
	{	"max",			sedit_max				},
	{	"mspsound",		sedit_mspsound			},
	{	"noundamage",	sedit_noundamage		},
	{	"objectwearoff",sedit_objectwearoff		},
	{	"position",		sedit_position			},
	{	"racerestrict",	sedit_racerestrict		},
	{	"realms",		sedit_realm				},
	{	"rename",		sedit_rename			},	
	{   "restrict",     sedit_sect_restrict		},
	{	"season",		sedit_element			},
	{	"show",			sedit_show				},
	{	"showflags",	olcex_showflags	},
	{	"sa",			olcex_showafter	},
	{	"sfa",			olcex_showflagsafter},
	{	"sca",			olcex_showcommandafter},
	{	"spellfunc",	sedit_spellfunc			},
	{   "spellgroup",   sedit_spell_group		},
	{	"spheres",		sedit_sphere			},
	{	"type",			sedit_type				},
	{	"wearoff",		sedit_wearoff			},
	{	"?",			show_help				},
	{	NULL,			0,						}
};

/**************************************************************************/
#endif // SEDIT_H
