/**************************************************************************/
// banedit.h - see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: banedit.h - olc.cpp include file... adds the banedit command     *
 *                    table and prototypes... banedit.cpp holds all the    *
 *                    commands.											   *
 ***************************************************************************/
#ifndef BANEDIT_H
#define BANEDIT_H
// Banedit - November 1998 - Kal
DECLARE_OLC_FUN( banedit_show		);
DECLARE_OLC_FUN( banedit_sitemasks	);		
DECLARE_OLC_FUN( banedit_type			);	
DECLARE_OLC_FUN( banedit_intendedpeople		);
DECLARE_OLC_FUN( banedit_reason				);
DECLARE_OLC_FUN( banedit_expire_date		);
DECLARE_OLC_FUN( banedit_disconnect_msg		);
DECLARE_OLC_FUN( banedit_always_allowed_email_masks);
DECLARE_OLC_FUN( banedit_allowed_email_masks);
DECLARE_OLC_FUN( banedit_disallowed_email_masks);
DECLARE_OLC_FUN( banedit_disallowed_msg		);
DECLARE_OLC_FUN( banedit_level				);
DECLARE_OLC_FUN( banedit_permanent			);
DECLARE_OLC_FUN( banedit_enabled			);
  
// Banedit olc command table - Kal
const struct olc_cmd_type banedit_table[] =
{
    {   "commands",					show_commands	},
    {   "show",						banedit_show			},
    {   "sitemasks",				banedit_sitemasks		},
    {   "type",						banedit_type			},
	{   "intendedpeople",			banedit_intendedpeople	},
    {   "reason",					banedit_reason			},
    {   "expire_date",				banedit_expire_date		},
    {   "disconnect_msg",			banedit_disconnect_msg	},
    {   "always_allowed",			banedit_always_allowed_email_masks},
    {   "allowed_email_masks",		banedit_allowed_email_masks},
	{   "disallowed_email_masks",	banedit_disallowed_email_masks},
	{   "disallowed_msg",			banedit_disallowed_msg	},
    {   "level",					banedit_level			},
    {   "permanent",				banedit_permanent		},
    {   "enabled",					banedit_enabled		},
    {	NULL,		0,		}
};
/**************************************************************************/
#endif // BANEDIT_H
