/**************************************************************************/
// hedit.h - OLC based help editor, Kal.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef HEDIT_H
#define HEDIT_H

//prototypes
DECLARE_OLC_FUN( hedit_show			);
DECLARE_OLC_FUN( hedit_level		);
DECLARE_OLC_FUN( hedit_text			);
DECLARE_OLC_FUN( hedit_addkey		);
DECLARE_OLC_FUN( hedit_rekey		);
DECLARE_OLC_FUN( hedit_file			);
DECLARE_OLC_FUN( hedit_flags		);
DECLARE_OLC_FUN( hedit_removehelp	);
DECLARE_OLC_FUN( hedit_category		);
DECLARE_OLC_FUN( hedit_parenthelp	);
DECLARE_OLC_FUN( hedit_seealso		);
DECLARE_OLC_FUN( hedit_immseealso	);
DECLARE_OLC_FUN( hedit_title		);
DECLARE_OLC_FUN( hedit_continues	);
DECLARE_OLC_FUN( hedit_wraptext		);
DECLARE_OLC_FUN( hedit_undowrap		);
DECLARE_OLC_FUN( hedit_undotext		);
DECLARE_OLC_FUN( hedit_assigneditor	);
DECLARE_OLC_FUN( hedit_commandref	);
DECLARE_OLC_FUN( hedit_spellname	);
DECLARE_OLC_FUN( hedit_convertlinecodes);
DECLARE_OLC_FUN( hedit_editnexthelp);

// table  
const struct olc_cmd_type hedit_table[] =
{
//  {   command			function		},
    {   "addkey",		hedit_addkey	}, 
    {   "assigneditor",	hedit_assigneditor}, 
    {   "rekey",		hedit_rekey		}, 
    {   "category",		hedit_category	}, 
    {   "continues",	hedit_continues	}, 
	{   "commandref",	hedit_commandref},
	{   "convertlinecodes",	hedit_convertlinecodes},	
	{   "editnexthelp",	hedit_editnexthelp},
    {   "file",			hedit_file		},
	{	"flags",		hedit_flags		},
    {   "text",			hedit_text		},
	{   "title",		hedit_title		},
    {   "level",		hedit_level		},
    {   "parenthelp",	hedit_parenthelp},	 
    {   "show",			hedit_show      }, 
    {	"showflags",	olcex_showflags	},
    {	"sa",			olcex_showafter	},
    {	"sfa",			olcex_showafter	},
	{	"sca",			olcex_showcommandafter},	
    {   "seealso",		hedit_seealso	},
    {   "spellname",	hedit_spellname },
	{   "immseealso",	hedit_immseealso},
    {   "wraptext",		hedit_wraptext	}, 
	{   "undowrap",		hedit_undowrap	}, 
	{   "undotext",		hedit_undotext	}, 
    {   "commands",		show_commands	},
    {   "?",			show_help		},
    {   "removehelp",	hedit_removehelp},
    {	NULL,			0,				}
};
/**************************************************************************/
#endif // HEDIT_H
