/**************************************************************************/
// clanedit.cpp - olc clan editor, Tibault
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"
#include "olc.h"
#include "clan.h"

extern CClanType *clan_list;

// do da GIO thang
GIO_START( CClanType )
GIO_STRH(  m_pSaveName,			"SaveName     " )
GIO_STRH(  m_pName,				"Name         " )
GIO_STRH(  m_pWhoName,			"WhoName      " )
GIO_STRH(  m_pNoteName,			"NoteName	  " )
GIO_STRH(  m_pWhoCat,			"WhoCat		  " )
GIO_STRH(  m_pDescription,		"Description  " )
GIO_INTH(  m_RecallRoom,		"Recall		  " )
GIO_INTH(  m_BankRoom,			"BankRoom     " )
GIO_LONGH( m_BankFunds,			"BankFunds    " )
GIO_STRH(  m_pColorStr,			"Color		  " )

GIO_STRH(  m_pClanRankTitle[0],	"RankTitle1	  " )
GIO_BOOLH( m_CanAdd[0],			"CanAdd1	  " )
GIO_BOOLH( m_CanPromote[0],		"CanPromote1  " )
GIO_BOOLH( m_CanRemove[0],		"CanRemove1   " )
GIO_BOOLH( m_CanWithdraw[0],	"CanWithdraw1 " )

GIO_STRH(  m_pClanRankTitle[1],	"RankTitle2	  " )
GIO_BOOLH( m_CanAdd[1],			"CanAdd2	  " )
GIO_BOOLH( m_CanPromote[1],		"CanPromote2  " )
GIO_BOOLH( m_CanRemove[1],		"CanRemove2   " )
GIO_BOOLH( m_CanWithdraw[1],	"CanWithdraw2 " )

GIO_STRH(  m_pClanRankTitle[2],	"RankTitle3   " )
GIO_BOOLH( m_CanAdd[2],			"CanAdd3	  " )
GIO_BOOLH( m_CanPromote[2],		"CanPromote3  " )
GIO_BOOLH( m_CanRemove[2],		"CanRemove3   " )
GIO_BOOLH( m_CanWithdraw[2],	"CanWithdraw3 " )

GIO_STRH(  m_pClanRankTitle[3],	"RankTitle4	  " )
GIO_BOOLH( m_CanAdd[3],			"CanAdd4	  " )
GIO_BOOLH( m_CanPromote[3],		"CanPromote4  " )
GIO_BOOLH( m_CanRemove[3],		"CanRemove4   " )
GIO_BOOLH( m_CanWithdraw[3],	"CanWithdraw4 " )

GIO_STRH(  m_pClanRankTitle[4],	"RankTitle5   " )
GIO_BOOLH( m_CanAdd[4],			"CanAdd5	  " )
GIO_BOOLH( m_CanPromote[4],		"CanPromote5  " )
GIO_BOOLH( m_CanRemove[4],		"CanRemove5	  " )
GIO_BOOLH( m_CanWithdraw[4],	"CanWithdraw5 " )

GIO_STRH(  m_pClanRankTitle[5],	"RankTitle6   " )
GIO_BOOLH( m_CanAdd[5],			"CanAdd6	  " )
GIO_BOOLH( m_CanPromote[5],		"CanPromote6  " )
GIO_BOOLH( m_CanRemove[5],		"CanRemove6	  " )
GIO_BOOLH( m_CanWithdraw[5],	"CanWithdraw6 " )

GIO_STRH(  m_pClanRankTitle[6],	"RankTitle7   " )
GIO_BOOLH( m_CanAdd[6],			"CanAdd7	  " )
GIO_BOOLH( m_CanPromote[6],		"CanPromote7  " )
GIO_BOOLH( m_CanRemove[6],		"CanRemove7	  " )
GIO_BOOLH( m_CanWithdraw[6],	"CanWithdraw7 " )

GIO_FINISH_STRDUP_EMPTY

/**************************************************************************/
// load the clans from disk
void load_clan_db( void )
{
	logf("Loading clan database from %s...", CLAN_FILE );
	GIOLOAD_LIST( clan_list, CClanType, CLAN_FILE );
	if(!clan_list){ // display a message if nothing is read in
		log_note("NOTE: Even though no clan entries were loaded, "
			"the mud can boot...  New clan entries can be dynamically created using "
			"'clanedit' within the game.");
	}
	log_string( "load_clan_db(): finished" );
}

/**************************************************************************/
// save the clan list
void save_clan_db( void )
{
	logf( "save_clan_db(): saving clan database to %s...", CLAN_FILE);
	GIOSAVE_LIST( clan_list, CClanType, CLAN_FILE, true );
}

/**************************************************************************/
// do func so it can be used as a command
void do_saveclans( char_data *ch, char * )
{
	save_clan_db();
	ch->println( "clans saved..." );
	logf( "do_saveclans(): manual save of clans..." );
}
/**************************************************************************/
// do func so it can be used as a command
void do_loadclans( char_data *ch, char * )
{
	load_clan_db();
	ch->println( "clans loaded..." );
}

/**************************************************************************/
// OLC section
bool clanedit_create( char_data *ch, char *newName )
{
	CClanType*			node;
	static CClanType	zero_node;

	node		= new CClanType;
	*node		= zero_node;
	node->next	= clan_list;
	clan_list	= node;

	replace_string(clan_list->m_pSaveName, newName ); // set it once, doesn't change
	replace_string(clan_list->m_pName, newName );	
	replace_string(clan_list->m_pNoteName, newName );
	replace_string(clan_list->m_pWhoName, newName );
	replace_string(clan_list->m_pWhoCat, "" );
	replace_string(clan_list->m_pDescription, "" );
	replace_string(clan_list->m_pColorStr, "`W" );
	clan_list->m_RecallRoom			= ROOM_VNUM_LIMBO;
	clan_list->m_BankRoom			= 0;
	clan_list->m_BankFunds			= 0;	// No starting money for em! :)
	
	ch->desc->pEdit		= (void *)clan_list;
	ch->desc->editor	= ED_CLAN;
	ch->printlnf("Clan '%s' Created.", newName);
	return false;
}

/**************************************************************************/
void do_clanedit( char_data *ch, char *argument )
{
	CClanType	*pClan;
	char		arg[MIL];

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	// do security checks
	if ( !HAS_SECURITY( ch, 8 ))
	{
		ch->println("You must have an olc security 8 or higher to use this command.");
		return;
	}


	if (IS_NULLSTR(argument)){
		ch->olctitlebar("CLANEDIT: SYNTAX");
		ch->println("syntax: clanedit <clan>.");
		ch->println("        clanedit `=Ccreate`x to make a new clan.");
		ch->println("        clanedit `=Clist`x lists existing clans.");
		for( pClan=clan_list;pClan;pClan=pClan->next){
			pClan->printDetails(ch);	
		}
		ch->println("`x");
		return;
	}
	
	argument = one_argument( argument, arg );

	if ( !str_cmp( arg, "list" ))
	{
		CClanType	*pC;

		ch->titlebar("Existing Clans");

		for( pC=clan_list;pC;pC=pC->next)
			pC->printDetails(ch);
			
		ch->println("`x");
		return;
	}

	if ( !str_cmp( arg, "create" ))
	{
		
		if(!str_infix(" ", argument) && str_infix("'", argument)){
			ch->wrapln("If you are going to create a clan with a "
				"space in its name, you must put single quotes "
				"around the complete clan name.");
			return;
		}
		one_argument( argument, arg );
		if(!str_infix("`", arg)){
			ch->println("A new clan name must not contain any colour characters.");
			return;
		}
		if(IS_NULLSTR(arg) || str_len(arg)<2){
			ch->println("A new clan name must be over 2 characters long.");
			ch->println("syntax: clanedit create <new clan name>");
			return;
		}
		if(clan_lookup(arg)){
			ch->printlnf("There already exists a clan with an exact or similar name to '%s', you must use another name sorry.", arg);
			return;
		}
		clanedit_create( ch, arg );
		return;
	}

	pClan = clan_lookup( arg );
	
    if ( !pClan )
    {
        ch->printlnf("There is no clan named '`Y%s`x'.", arg);
        return;
    }

	ch->desc->pEdit = (void *)pClan;
	ch->desc->editor = ED_CLAN;
	ch->printlnf("Editing properties '%s'.", pClan->m_pName );
	return;
}

/**************************************************************************/
bool clanedit_show(char_data *ch, char *)
{
	CClanType	*pC;
	int			lIdx = 0;
	
    EDIT_CLAN(ch, pC);

	if (IS_NULLSTR( pC->m_pName )) {
		ch->println("clanedit_show(): NULL");
		return false;
	}

	ch->printf(  "`=rName:      `x%s`=r\r\n`=rClanColour:`s", pC->cname());
	ch->printlnbw(pC->color_str());
	ch->printlnf(  "`SSaveName:  `x%s`S(can't be changed - saved on disk with this name)`x",pC->m_pSaveName );
	ch->printlnf(  "`=rNoteName:  `x%-20s   `=r", pC->m_pNoteName );
	ch->printlnf("`=rWho name:  %-20s   `=r",	pC->m_pWhoName );
	ch->printlnf("`=rWho short: [`x%-3s`=r]   `=r", pC->m_pWhoCat );
	ch->printlnf("`=rRecall vnum : `x%-5d`=r",pC->m_RecallRoom );
	ch->printlnf("`=rBank room : `x%-5d`=r",pC->m_BankRoom );
	ch->println("`=rDescription:");
	ch->wraplnf( "`x%s`=R", pC->m_pDescription );

	// Ranks
	ch->println("`b/---------------------------------------------------------\\`=r");
	ch->println("`b|   `=rTitle                `b|`=rAdd    `b|`=rPromote`b|`=rOutcast`b|`=rWithdraw`b|`=r");
	ch->println("`b|---------------------------------------------------------|`=r");
	
	for ( lIdx = 0; lIdx < MAX_RANK; lIdx++ )
	{
		ch->printlnf("`b|`x%d. %-20s `b|`x%-1s      `b|`x%-1s      `b|`x%-1s      `b|`x%-1s       `b|`=r",
			lIdx+1,
			pC->m_pClanRankTitle[lIdx], 
			pC->m_CanAdd[lIdx] ? "`GY`=r" : "`RN`=r",
			pC->m_CanPromote[lIdx] ? "`GY`=r" : "`RN`=r",
			pC->m_CanRemove[lIdx] ? "`GY`=r" : "`RN`=r",
			pC->m_CanWithdraw[lIdx] ? "`GY`=r" : "`RN`=r");
	}
	ch->println("`b\\---------------------------------------------------------/`=r");
	ch->println("(`xRank 1 is the highest rank, promote includes demote.`=r)");

	return false;
}

/**************************************************************************/
bool clanedit_name(char_data *ch, char *argument)
{
    CClanType		*pClan;
	
    EDIT_CLAN( ch, pClan );
	
    if ( argument[0] == '\0' )
    {
		ch->println("Syntax:   name [name]");
		return false;
    }
	
    free_string( pClan->m_pName );
    pClan->m_pName = str_dup( argument );
	
    ch->println("Clan name set.");
	
    return true;
}

/**************************************************************************/
bool clanedit_whoname(char_data *ch, char *argument)
{
    CClanType		*pClan;
	
    EDIT_CLAN( ch, pClan );
	
    if ( argument[0] == '\0' )
    {
		ch->println( "Syntax:   whoshort [name]." );
		return false;
    }
	
    free_string( pClan->m_pWhoName );
    pClan->m_pWhoName = str_dup( argument );
	
    ch->println("Clan who name set.");
	
    return true;
}

/**************************************************************************/
bool clanedit_whocat(char_data *ch, char *argument)
{
    CClanType		*pClan;
	
    EDIT_CLAN( ch, pClan );
	
    if ( argument[0] == '\0' || str_len(argument) != 3 )
    {
		ch->println( "Syntax:   whoshort [name]. Name must be 3 characters long." );
		return false;
    }
	
    free_string( pClan->m_pWhoCat );
    pClan->m_pWhoCat = str_dup( argument );
	
    ch->println("Clan who short set.");
	
    return true;
}

/**************************************************************************/
bool clanedit_notename(char_data *ch, char *argument)
{
    CClanType		*pClan;
	
    EDIT_CLAN( ch, pClan );
	
    if(IS_NULLSTR(argument)){
		ch->println("Syntax:   notename [name]");
		return false;
    }

	if(has_whitespace(argument)){
		ch->println("The clan notename can not have whitespace in it... it should be a single word");
		return false;
	}
	
	ch->printlnf( "Clan notename changed from '%s' to '%s'.",
		pClan->m_pNoteName, argument);
    replace_string( pClan->m_pNoteName, argument  );
    return true;
}

/**************************************************************************/
bool clanedit_description(char_data *ch, char *argument)
{
	CClanType *pC;
	EDIT_CLAN( ch, pC );
	
	if(!IS_NULLSTR(argument)){
		ch->println("Syntax:  description (string editor used)");
		ch->println("uses the string editor to edit/add the clan description.");
		return false;
	}
	string_append( ch, &pC->m_pDescription);
	return true;
}

/**************************************************************************/
bool clanedit_colour(char_data *ch, char *argument)
{
    CClanType *pClan;

    EDIT_CLAN(ch, pClan);

    if ( IS_NULLSTR(argument) || c_str_len(argument)>0)
    {
		if (c_str_len(argument)>0)
		{
            ch->println("The colour code should no contain any visible characters!\r\n");
		}
        ch->println("Syntax:   colour <code>");
        ch->wrapln( "This is the colour code prefixed to the clan name and "
			"whoname in some situations, it should be non visible colour codes" );        
		return false;
    }

    ch->printlnfbw( "Clan colour code changed from '%s' to '%s'",
		pClan->m_pColorStr, argument);
    replace_string( pClan->m_pColorStr, argument );
    return true;
}

/**************************************************************************/
bool clanedit_ranktitle( char_data *ch, char *argument )
{
	char arg[MIL];
	CClanType *pC;
	argument = one_argument(argument,arg);
		
    if ( IS_NULLSTR(argument))
    {
		ch->printlnf("Syntax: ranktitle [number 1-%d] [title]",
			MAX_RANK);
		return false;
    }

	if ( IS_NULLSTR(argument) || !is_number(arg) )
	{
		ch->println("First argument must be a rank number, the second the title.");
		clanedit_ranktitle( ch, "" );		// redisplay the help
		return false;
	}

	if ( atoi(arg) < 1 || atoi(arg) > MAX_RANK)
	{
		ch->printlnf( "The valid range for ranks is between `#`W1`^ and `W%d`^.",
			MAX_RANK);
		return false;
	}

    EDIT_CLAN( ch, pC );

    free_string( pC->m_pClanRankTitle[atoi(arg)-1] );
	pC->m_pClanRankTitle[atoi(arg)-1] = str_dup(argument);

	ch->printlnf ( "Rank %d is now known as %s.", atoi(arg), argument );
    return true;
}

/**************************************************************************/
bool clanedit_canadd( char_data *ch, char *argument )
{
	char arg[MIL];
	CClanType *pC;
	argument = one_argument(argument,arg);
		
    if ( IS_NULLSTR(argument))
    {
		ch->printlnf("Syntax: canadd [number 1-%d] [Y/N]", MAX_RANK );
		return false;
    }

	if ( IS_NULLSTR(argument) 
		|| !is_number(arg) 
		|| str_len(argument) > 1 
		|| ( (LOWER(*argument)!='y') 
		&&    LOWER(*argument)!='n') )
	{
		ch->println("First argument must be a rank number, the second either Y or N.");
		clanedit_canadd( ch, "" );		// redisplay the help
		return false;
	}

	if ( atoi(arg) < 1 || atoi(arg) > MAX_RANK )
	{
		ch->printlnf( "The valid range for ranks is between `#`W1`^ and `W%d`^.",
			MAX_RANK);
		return false;
	}

    EDIT_CLAN( ch, pC );

	pC->m_CanAdd[atoi(arg)-1] = (LOWER(*argument)=='y');

	ch->printlnf ( "Rank %d's ability to add other members is set to %s.", atoi(arg), argument );
    return true;
}

/**************************************************************************/
bool clanedit_canpromote( char_data *ch, char *argument )
{
	char arg[MIL];
	CClanType *pC;
	argument = one_argument(argument,arg);
		
    if ( IS_NULLSTR(argument))
    {
		ch->printlnf("Syntax: canadd [number 1-%d] [Y/N]", MAX_RANK);
		return false;
    }

	if ( IS_NULLSTR(argument) 
		|| !is_number(arg) 
		|| str_len(argument) > 1 
		|| ( (LOWER(*argument)!='y') 
		&&    LOWER(*argument)!='n') )
	{
		ch->println("First argument must be a rank number, the second either Y or N.");
		clanedit_canadd( ch, "" );		// redisplay the help
		return false;
	}

	if ( atoi(arg) < 1 || atoi(arg) > MAX_RANK)
	{
		ch->printlnf( "The valid range for ranks is between `#`W1`^ and `W%d`^.",
			MAX_RANK);
		return false;
	}

    EDIT_CLAN( ch, pC );

	pC->m_CanPromote[atoi(arg)-1] = (LOWER(*argument)=='y');

	ch->printlnf ( "Rank %d's ability to promote other members is set to %s.", atoi(arg), argument );
    return true;}
/**************************************************************************/
bool clanedit_canremove( char_data *ch, char *argument )
{
	char arg[MIL];
	CClanType *pC;
	argument = one_argument(argument,arg);
		
    if ( IS_NULLSTR(argument))
    {
		ch->printlnf("Syntax: canadd [number 1-%d] [Y/N]", MAX_RANK);
		return false;
    }

	if ( IS_NULLSTR(argument) 
		|| !is_number(arg) 
		|| str_len(argument) > 1 
		|| ( (LOWER(*argument)!='y') 
		&&    LOWER(*argument)!='n') )
	{
		ch->println("First argument must be a rank number, the second either Y or N.");
		clanedit_canadd( ch, "" );		// redisplay the help
		return false;
	}

	if ( atoi(arg) < 1 || atoi(arg) > MAX_RANK)
	{
		ch->printlnf( "The valid range for ranks is between `#`W1`^ and `W%d`^.",
			MAX_RANK);
		return false;
	}

    EDIT_CLAN( ch, pC );

	pC->m_CanRemove[atoi(arg)-1] = (LOWER(*argument)=='y');

	ch->printlnf ( "Rank %d's ability to outcast other members is set to %s.", 
		atoi(arg), argument );
    return true;
}
/**************************************************************************/
bool clanedit_recallroom( char_data *ch, char *argument )
{
	CClanType *pClan;
		
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: recallroom [vnum]");
		return false;
    }

	if ( !is_number( argument ))
	{
		ch->println("The vnum value must be numeric.");
		clanedit_recallroom( ch, "" );		// redisplay the help
		return false;
	}

	if ( get_room_index( atoi( argument )) == '\0' )
	{
		ch->println("That room vnum doesn't exist.");
		return false;
	}

    EDIT_CLAN( ch, pClan );

	pClan->m_RecallRoom = atoi( argument );

	ch->printlnf("The recall room has been set to '%d'.", pClan->m_RecallRoom );
    return true;
}

/**************************************************************************/
bool clanedit_delete( char_data *ch, char *argument )
{
	CClanType *pClan;
	CClanType *pC;

	if(str_cmp("confirm", argument)){
		ch->println("type `=Cdelete confirm`x to remove this clan.");
		return false;
	}

	EDIT_CLAN( ch, pClan);

	if ( clan_list == pClan )
	{
		clan_list = pClan->next;
		free(pClan);
		edit_done ( ch );
		return true;
	}

	for ( pC = clan_list; pC; pC = pC->next )
	{
		if ( pC->next == pClan )
		{
			pC->next = pClan->next;
			free(pClan);
			edit_done( ch );
			return true;
		}
	}

	return false;
}

/**************************************************************************/
bool clanedit_withdraw( char_data *ch, char *argument )
{
	char arg[MIL];
	CClanType *pC;
	argument = one_argument(argument,arg);
	EDIT_CLAN( ch, pC );

    if ( IS_NULLSTR(argument))
    {
		ch->printlnf("Syntax: canwithdraw [number 1-%d] [Y/N]", MAX_RANK );
		return false;
    }

	if ( IS_NULLSTR(argument) 
		|| !is_number(arg) 
		|| str_len(argument) > 1 
		|| ( (LOWER(*argument)!='y') 
		&&    LOWER(*argument)!='n') )
	{
		ch->println("First argument must be a rank number, the second either Y or N.");
		clanedit_withdraw( ch, "" );		// redisplay the help
		return false;
	}

	if ( atoi(arg) < 1 || atoi(arg) > MAX_RANK )
	{
		ch->printlnf( "The valid range for ranks is between `#`W1`^ and `W%d`^.",
			MAX_RANK);
		return false;
	}

    pC->m_CanWithdraw[atoi(arg)-1] = (LOWER(*argument)=='y');

	ch->printlnf( "Rank %d's ability to withdraw funds is set to %s.", atoi(arg), argument );
    return true;
}
/**************************************************************************/
bool clanedit_bankroom( char_data *ch, char *argument )
{
	CClanType *pClan;
		
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: bankroom [vnum]");
		return false;
    }

	if ( !is_number( argument ))
	{
		ch->println("The vnum value must be numeric.");
		clanedit_bankroom( ch, "" );		// redisplay the help
		return false;
	}

	if ( get_room_index( atoi( argument )) == '\0' )
	{
		ch->println("That room vnum doesn't exist.");
		return false;
	}

    EDIT_CLAN( ch, pClan );

	pClan->m_BankRoom = atoi( argument );

	ch->printlnf("The bank room has been set to '%d'.", pClan->m_BankRoom );
    return true;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
