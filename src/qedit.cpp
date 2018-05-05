/**************************************************************************/
// qedit.cpp - olc quest editing functions, Jarren
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h" // dawn standard includes

#include "olc.h"
#include "security.h"
#include "immquest.h"

DECLARE_OLC_FUN( qedit_create );

/**************************************************************************/
void qedit_showquestinfo( char_data *ch, QUEST_DATA *pQ)
{	
	ch->printlnf("`=RQuestName: `=r%s`x",
		IS_NULLSTR(pQ->questname)?"`x(none)":pQ->questname);
	ch->printf("`=RDate Created: `=r%s", ctime( &pQ->created_date));
	ch->printf("`=RDate Last Modified: `=r%s", ctime( &pQ->modified_date));
	ch->printlnf("`=RImms Responsible: `=r%s",
		IS_NULLSTR(pQ->immnames)?"`x(none)":pQ->immnames);
	ch->printlnf("`=RCurrent Status: `=r%s",
		IS_NULLSTR(pQ->status)?"`x(none)":pQ->status);
	ch->printlnf("`=RResources: `=r%s",
		IS_NULLSTR(pQ->resource)?"`x(none)":pQ->resource);
	ch->printlnf("`=RImm Help?:`=r%s`x",
		pQ->immhelp==IMMHELP_UNDEFINED?"`x(undefined)":flag_string( immhelp_types, pQ->immhelp));
	ch->printlnf("`=RSynopsis: `x\r\n%s",
		IS_NULLSTR(pQ->synopsis)?"`x(none)":pQ->synopsis);
}
/**************************************************************************/
bool qedit_show( char_data *ch, char *)
{
	QUEST_DATA *pQ;
	EDIT_QUEST(ch, pQ);

	// display the quest to the imm
	qedit_showquestinfo( ch, pQ);
	return false;
}
/**************************************************************************/
bool qedit_create( char_data *ch, char *argument )
{
	QUEST_DATA *pQuest;
	static QUEST_DATA zero_node;

    if ( argument[0] == '\0')
    {
		ch->println("Syntax:  qedit create [quest_name]");
		return false;
    }

	pQuest = quest_lookup( argument );
	if(pQuest && !str_cmp(pQuest->questname, argument))
	{

		ch->printlnf("A quest with the name '%s' already exsists.", argument);
		return false;
	}

	pQuest=new QUEST_DATA;
	*pQuest=zero_node;
	pQuest->next		= quest_list;
	quest_list			= pQuest;

	pQuest->questname	= str_dup(argument);
	pQuest->immnames	= str_dup(TRUE_CH(ch)->name);
	pQuest->created_date=current_time;
	pQuest->modified_date=current_time;
 	pQuest->status		= str_dup("");
	pQuest->resource	= str_dup("");
	pQuest->immhelp		= IMMHELP_UNDEFINED;
	pQuest->synopsis	= str_dup("");

    ch->desc->pEdit	= pQuest;

	SET_BIT(QUEST_TABLE_FLAGS,QEDIT_CHANGED);

    ch->printlnf("Quest '%s' Created.", pQuest->questname);
    return true;
}
/**************************************************************************/
bool qedit_questname( char_data *ch, char *argument )
{
	QUEST_DATA *pQ;

    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax:  questname [string]");
		return false;
	}
	EDIT_QUEST(ch, pQ);
	
    free_string( pQ->questname);
    pQ->questname = str_dup( argument );
    ch->printlnf("Quest now called '%s'", pQ->questname);
    return true;
}
/**************************************************************************/
bool qedit_responsible(char_data *ch, char *argument)
{
    QUEST_DATA *pQ;
    char name[MSL];
    char buf[MSL];

    EDIT_QUEST(ch, pQ);

    one_argument( argument, name );

    if ( name[0] == '\0' )
    {
		ch->println("Syntax:  responsible [%name]  -toggles responsible imms");
		ch->println("Syntax:  responsible Any      -allows everyone");
		return false;
    }

    name[0] = UPPER( name[0] );

	if(IS_NULLSTR(pQ->immnames)){
		bug("qedit_responsible(): for some reason pQ->immnames was zero length or NULL!");
		pQ->immnames=str_dup("None");
	}

    if ( strstr( pQ->immnames, name ) != '\0' )
    {
		pQ->immnames = string_replace( pQ->immnames, name, "" );
		pQ->immnames = str_dup(ltrim_string(rtrim_string(pQ->immnames )));

		if ( pQ->immnames[0] == '\0' )
		{
		    free_string( pQ->immnames );
			pQ->immnames = str_dup( "None" );
		}
		ch->printlnf("Responsible Imm '%s' removed.", name );
		return true;
    }
    else
    {
		buf[0] = '\0';
		if ( strstr( pQ->immnames, "None" ) != '\0' )
		{
			pQ->immnames = string_replace( pQ->immnames, "None", "" );
			pQ->immnames = str_dup(ltrim_string(rtrim_string(pQ->immnames )));
		}

		if (pQ->immnames[0] != '\0' )
		{
		    strcat( buf, pQ->immnames );
			strcat( buf, " " );
		}
		strcat( buf, name );
		free_string( pQ->immnames );
		pQ->immnames = string_proper( str_dup( buf ) );

		ch->printlnf("Imm added to list - current Imms now:\r\n%s", 
			pQ->immnames);
		return true;
    }

    return false;
}
/**************************************************************************/
bool qedit_status( char_data *ch, char *argument )
{
	QUEST_DATA *pQ;

    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax:  status [string]");
		return false;
	}
	EDIT_QUEST(ch, pQ);
	
    free_string( pQ->status);
    pQ->status = str_dup( argument );
    ch->printlnf("Quest status now: '%s'", pQ->status);
    return true;
}
/**************************************************************************/
bool qedit_resource(char_data *ch, char *argument)
{
    QUEST_DATA *pQ;
    char name[MSL];
    char buf[MSL];

    EDIT_QUEST(ch, pQ);

    one_argument( argument, name );

    if ( name[0] == '\0' )
    {
	    ch->println("Syntax:  resource [%helpfile]  -toggles resource file");
		return false;
    }

    name[0] = UPPER( name[0] );
	
	if(IS_NULLSTR(pQ->resource)){
		bug("qedit_resource(): for some reason pQ->resource was zero length or NULL!");
		pQ->resource=str_dup("None");
	}

    if ( strstr( pQ->resource, name ) != '\0' )
    {
		pQ->resource = string_replace( pQ->resource, name, "" );
		pQ->resource = ltrim_string(rtrim_string(( pQ->resource )));

		if ( pQ->resource[0] == '\0' )
		{
			free_string( pQ->resource );
			pQ->resource = str_dup( "None" );
		}
		ch->printlnf("Resource file '%s' removed.", name );
		return true;
    }
    else
    {
		if ( strstr( pQ->resource, "None" ) != '\0' )
		{
			pQ->resource = string_replace( pQ->resource, "None", "" );
			pQ->resource = ltrim_string(rtrim_string( pQ->resource ));
		}

		buf[0] = '\0';
		if (pQ->resource[0] != '\0' )
		{
			strcat( buf, pQ->resource );
			strcat( buf, " " );
		}
		strcat( buf, name );
		free_string( pQ->resource );
		pQ->resource = string_proper( str_dup( buf ) );

		ch->printlnf("Resource entry added to list - current resources now:\r\n%s", 
			pQ->resource);
		return true;
    }

    return false;
}
/**************************************************************************/
bool qedit_help(char_data *ch, char *argument)
{
	QUEST_DATA * pQ;
	int value;
	
    EDIT_QUEST(ch, pQ);

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( immhelp_types, argument ) ) != NO_FLAG )
		{
			ch->printlnf("Immhelp type changed from %s to %s.", 
				pQ->immhelp==IMMHELP_UNDEFINED?"`x(undefined)":flag_string( immhelp_types, pQ->immhelp),
				value==IMMHELP_UNDEFINED?"`x(undefined)":flag_string( immhelp_types, value));
			pQ->immhelp = value;
			return true;
		}
		ch->printlnf("Unrecognized type '%s'", argument);
	}

    ch->println("Syntax: help [type]");
	ch->print("Help types available: [ ");

	for(int i=0; immhelp_types[i].name != NULL; i++)
		ch->printf("`=R%s ", immhelp_types[i].name);

	ch->println("`x]");
	
	return false;
}
/**************************************************************************/
bool qedit_synopsis( char_data *ch, char * argument)
{
    QUEST_DATA *pQ;

    EDIT_QUEST(ch, pQ);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pQ->synopsis );
	return true;
    }

    ch->println("Syntax:  synopsis");
    return false;
}
/**************************************************************************/
