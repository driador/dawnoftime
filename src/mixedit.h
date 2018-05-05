/**************************************************************************/
// mix.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef MIXEDIT_H
#define MIXEDIT_H

DECLARE_OLC_FUN( mixedit_name					);
DECLARE_OLC_FUN( mixedit_show					);
DECLARE_OLC_FUN( mixedit_vnum					);
DECLARE_OLC_FUN( mixedit_difficulty				);
DECLARE_OLC_FUN( mixedit_rtype					);
DECLARE_OLC_FUN( mixedit_rname					);
DECLARE_OLC_FUN( mixedit_rshort					);
DECLARE_OLC_FUN( mixedit_rlong					);
DECLARE_OLC_FUN( mixedit_type					);
DECLARE_OLC_FUN( mixedit_rwear					);
DECLARE_OLC_FUN( mixedit_value0					);
DECLARE_OLC_FUN( mixedit_value1					);
DECLARE_OLC_FUN( mixedit_value2					);
DECLARE_OLC_FUN( mixedit_value3					);
DECLARE_OLC_FUN( mixedit_value4					);
DECLARE_OLC_FUN( mixedit_ingredients			);
DECLARE_OLC_FUN( mixedit_lock					);
DECLARE_OLC_FUN( mixedit_vessel					);
DECLARE_OLC_FUN( mixedit_failedrname			);
DECLARE_OLC_FUN( mixedit_failedrshort			);
DECLARE_OLC_FUN( mixedit_failedrlong			);

const struct olc_cmd_type mixedit_table[] =
{
//	{	COMMAND			FUNCTION				}
	{   "commands",		show_commands			},
	{	"difficulty",	mixedit_difficulty		},
	{	"ingredients",	mixedit_ingredients		},
	{	"lock",			mixedit_lock			},
	{	"name",			mixedit_name			},
	{	"rlong",		mixedit_rlong			},
	{	"rname",		mixedit_rname			},
	{	"rshort",		mixedit_rshort			},
	{	"rtype",		mixedit_rtype			},
	{	"rwear",		mixedit_rwear			},
	{	"failedrlong",	mixedit_failedrlong		},
	{	"failedrname",	mixedit_failedrname		},
	{	"failedrshort",	mixedit_failedrshort	},
	{	"show",			mixedit_show			},
	{	"short",		mixedit_rshort			},		// just to be safe so we don't start re-shorting players
	{	"type",			mixedit_type			},
	{	"v0",			mixedit_value0			},
	{	"v1",			mixedit_value1			},
	{	"v2",			mixedit_value2			},
	{	"v3",			mixedit_value3			},
	{	"v4",			mixedit_value4			},
	{	"vessel",		mixedit_vessel			},
	{	"vnum",			mixedit_vnum			},
	{	"?",			show_help				},
	{	NULL,			0,						}
};
#endif // MIX_H

