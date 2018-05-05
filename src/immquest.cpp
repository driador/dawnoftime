/**************************************************************************/
// immquest.cpp - Quest database, Jarren.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
//current status..	delquest dosent work..look into removing elements of linked lists
//					need to write funcs for changing name, immname, status, resource,etc

#include "include.h"
#include "olc.h"
#include "immquest.h"

QUEST_DATA *quest_lookup( const char *name );
DECLARE_OLC_FUN( qedit_create );

/**************************************************************************/
// semilocalized globals
quest_type	*quest_list;
sh_int	QUEST_TABLE_FLAGS;

/**************************************************************************/
// create immquest GIO lookup table 
GIO_START(QUEST_DATA)
GIO_STRH(questname,				"QuestName	")
GIO_STRH(immnames,				"Name		")
GIO_INTH(created_date,			"Cre_date	")
GIO_INTH(modified_date,			"Mod_date	")
GIO_STRH(status,				"Status		")
GIO_STRH(resource,				"Resources	")
GIO_WFLAGH(immhelp,				"IMMhelp	" , immhelp_types)
GIO_STRH(synopsis,				"Synopsis	")
GIO_FINISH
/**************************************************************************/
// loads in the quest database
void load_quest_db(void)
{
	logf("===Loading quest database from %s...", QUEST_FILE);
	GIOLOAD_LIST(quest_list, QUEST_DATA, QUEST_FILE); 
	log_string ("load_quest_db(): finished");
}
/**************************************************************************/
// saves the quest database
void save_quest_db( void)
{
	logf("===save_quest_db(): saving quest database to %s", QUEST_FILE);
	GIOSAVE_LIST(quest_list, QUEST_DATA, QUEST_FILE, true);
}
/**************************************************************************/
void do_savequestdb( char_data *ch, char * )
{
	save_quest_db( );
	ch->printlnf("do_savequestdb(): manual save of quest completed to %s,\r\n"
		"check logs for any errors.", QUEST_FILE);	
		
}
/**************************************************************************/
// lists letgains
void do_listquest( char_data *ch, char *argument )
{
	QUEST_DATA *node;
	int count;

	if(!IS_NULLSTR(argument))
	{
		ch->titlebar("QUEST DETAILS");

		node = quest_lookup( argument );

		if(node){
			qedit_showquestinfo( ch, node);
		}else{
			ch->printlnf("Couldn't find the quest '%s' in the quest database.", argument);
		}
		return;
	}
	
	ch->titlebar("LIST OF QUESTS");

	ch->titlebar("Closed Quests");
	count=0;
	for (node = quest_list; node; node= node->next)
	{
		if(node->immhelp == IMMHELP_CLOSED)
		{
			ch->printlnf("`s%d> `MQuest Name: `m%-37s  `GStatus: `g%s",
				++count, node->questname,
				IS_NULLSTR(node->status)?"(none)":node->status);
			ch->printlnf("         `CBy: `c%-40s `BLast Modified: `b%.10s",
				node->immnames, ctime( &node->modified_date));
		}
	}

	ch->titlebar("Possible Help Quests");
	count=0;
	for (node = quest_list; node; node= node->next)
	{
		if(node->immhelp == IMMHELP_POSSIBLE)
		{
			ch->printlnf("`s%d> `MQuest Name: `m%-37s  `GStatus: `g%s",
				++count, node->questname,
				IS_NULLSTR(node->status)?"(none)":node->status);
			ch->printlnf("         `CBy: `c%-40s `BLast Modified: `b%.10s",
				node->immnames, ctime( &node->modified_date));
		}
	}

	ch->titlebar("Free Help Quests");
	count=0;
	for (node = quest_list; node; node= node->next)
	{
		if(node->immhelp == IMMHELP_FREE)
		{
			ch->printlnf("`s%d> `MQuest Name: `m%-37s  `GStatus: `g%s",
				++count, node->questname,
				IS_NULLSTR(node->status)?"(none)":node->status);
			ch->printlnf("         `CBy: `c%-40s `BLast Modified: `b%.10s",
				node->immnames, ctime( &node->modified_date));
		}
	}

	ch->titlebar("Other");
	count=0;
	for (node = quest_list; node; node= node->next)
	{
		if(node->immhelp == IMMHELP_UNDEFINED)
		{
			ch->printlnf("`s%d> `MQuest Name: `m%-37s  `GStatus: `g%s",
				++count, node->questname,
				IS_NULLSTR(node->status)?"(none)":node->status);
			ch->printlnf("         `CBy: `c%-40s `BLast Modified: `b%.10s",
				node->immnames, ctime( &node->modified_date));
		}
	}
	ch->println("\r\n`xType listquest <questname> for more info on a quest.");
}
/**************************************************************************************/
void do_delquest( char_data *ch, char *argument )
{
	QUEST_DATA *node;
	QUEST_DATA *prevnode=NULL;

	if(IS_NULLSTR(argument))
	{
		ch->println("syntax: delquest <quest_name>");
		return;
	};


	// try exact match first
	for (node = quest_list; node; node = node->next )
	{
		if(!str_cmp(node->questname,argument))
			break;
		prevnode = node;
	}

	if(!node)
	{
		// check for a prefix match if
		for (node = quest_list, prevnode = NULL; node; node= node->next)
		{		
			if ( !str_prefix( argument, node->questname ) )
				break;
			prevnode = node;
		}
		// ensure we have found something
		if(!node)
		{
			ch->printlnf("Couldn't find the quest '%s' in the quest database.", argument);
			return;
		}
	}

	if(!IS_RESPONSIBLE(ch,node))
	{
		ch->println("You must be one of the Immortals responsible for a quest to delete it.");
		return;
	}

	ch->printlnf("Quest: '%s' has been deleted.", node->questname);

	free_string(node->questname);
	free_string(node->immnames);
	free_string(node->status);
	free_string(node->resource);
	free_string(node->synopsis);
	
	if(!prevnode) // delete the head
		quest_list = quest_list->next;
	else
		prevnode->next=node->next;

	SET_BIT(QUEST_TABLE_FLAGS,QEDIT_CHANGED);
}
/**************************************************************************************/
void do_qedit( char_data *ch, char *argument )
{
	QUEST_DATA *pQuest = NULL;
	char arg[MIL];

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	if ( !IS_TRUSTED(ch, LEVEL_IMMORTAL))
	{
		do_huh(ch,"");
		return;
	}

	if (IS_NULLSTR(argument))
	{
		ch->println("Syntax:  qedit create <quest_name>");
		ch->println("    or:  qedit <quest_name>");
		return;
	}


	argument = one_argument( argument, arg );

	pQuest = quest_lookup( arg );

	if( pQuest )
	{
		if(!IS_RESPONSIBLE(ch,pQuest))
		{
			ch->println("You must be one of the Immortals responsible for a quest to edit it.");
			return;
		}

		ch->desc->pEdit	= pQuest;
		argument = str_dup(pQuest->questname);
	}
    else if ( !str_cmp( arg, "create" ) )
    {
        if ( argument[0] == '\0' )
		{
			ch->println("Syntax:  qedit create <quest_name>");
			ch->println("    or:  qedit <quest_name>");
			return;
		}

		if(!qedit_create( ch, argument )){
			return;
		}
	}
	else
	{
		ch->println("Syntax:  qedit create <quest_name>");
		ch->println("    or:  qedit <quest_name>");
		return;
	}

	ch->desc->editor = ED_QUEST;
	ch->printlnf("Editing quest details of '%s'", argument);
	ch->println("`=rType `=Cdone`=r to finish editing."); 
}
/**************************************************************************************/
QUEST_DATA *quest_lookup( const char *name )
{
	QUEST_DATA *node;

	//check for exact match in name
	for (node = quest_list; node && str_cmp(node->questname,name); node= node->next)
	{
	}

	if(node)
		return node;

	//check for partial match
	for (node = quest_list; node && str_cmp(node->questname,name); node= node->next)
    {
		if (!str_prefix( name, node->questname )){
			return node;
		}
	}
	return NULL;
}
/**************************************************************************************/
// Kal port of SB code, Dec 2001
void do_qpoints( char_data *ch, char *argument )
{
	char arg[MIL];
	char arg2[MIL];
	char_data *victim;
	int modifier;
	
	argument = one_argument( argument, arg );
	strcpy( arg2, argument );
	
    if( IS_NULLSTR(arg)){
		ch->titlebar("QUEST POINTS");
        ch->println("Syntax: qpoints <name>             - show how many points <name> has.");
        ch->println("Syntax: qpoints <name> <modifier>  - add <modifier> to <names> qpoints.");
        ch->println("  <modifier> may be positive or negative.");
		ch->titlebar("");
        return;
    }
	
	victim = get_whovis_player_world(ch, arg );
    if(!victim){
		ch->printlnf( "There is '%s' in the game.", arg);
		return;
    }
	
    if(IS_NULLSTR(arg2)){
        ch->printlnf("%s currently has %d quest points.", victim->name,
			victim->pcdata->qpoints);
        return;
    }
	
    if(!is_number( arg2 ) )
    {
        ch->printlnf( "The modifier must be numeric - '%s' is not.", arg2);
        return;
    }

    modifier= atoi( arg2 );

	ch->printlnf("Quest points on %s changed from %d to %d",
		PERS(victim, ch), 
		victim->pcdata->qpoints,
		victim->pcdata->qpoints+modifier);

    victim->pcdata->qpoints += modifier;
	
    if(victim->pcdata->qpoints < 0){
		victim->pcdata->qpoints = 0;
		ch->printlnf("Questpoints on %s reset to 0 - can't have negative qpoints.",
			PERS(victim, ch));
	}
    return;
}
/**************************************************************************************/
/**************************************************************************************/

