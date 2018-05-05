/**************************************************************************/
// deity.cpp - Kereno's uncompleted deity code
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

// protos
void dedit_read_race(gio_type *, int, void *data, FILE *fp);
void dedit_write_race(gio_type *gio_table, int tableIndex, void *data, FILE *fp);
void dedit_read_rival(gio_type *, int, void *data, FILE *fp);
void dedit_write_rival(gio_type *gio_table, int tableIndex, void *data, FILE *fp);

GIO_CUSTOM_FUNCTION_PROTOTYPE(racetype_write_generic_races_set_for_n_array);
GIO_CUSTOM_FUNCTION_PROTOTYPE(racetype_read_generic_races_set_for_n_array);

// do da GIO thang
GIO_START( DEITY_DATA )
GIO_STRH(  name,			"Name         " )
GIO_STRH(  description,		"Description  " )
GIO_SHINTH(shrinevnum,		"Shrine       " )
GIO_SHINTH(symbol[0],		"Symbol1	  " )
GIO_SHINTH(symbol[1],		"Symbol2	  " )
GIO_SHINTH(symbol[2],		"Symbol3      " )
GIO_SHINTH(symbol[3],		"Symbol4	  " )
GIO_SHINTH(tendency,		"Tendency     " )
GIO_SHINTH(alliance,		"Alliance     " )
GIO_SHINTH(followers,		"Followers    " )
GIO_SHINTH(max_followers,	"MaxFollow    " )
GIO_WFLAGH(alignflags,		"Alignflags	  ", align_flags )
GIO_WFLAGH(tendflags,		"Tendflags	  ", tendency_flags )
GIO_CUSTOM_WRITEH(race_allow_n, "RaceAllowances ", racetype_write_generic_races_set_for_n_array)
GIO_CUSTOM_READH(race_allow_n,  "RaceAllowances ", racetype_read_generic_races_set_for_n_array)
GIO_WFLAGH(sex,				"Sex          ", sex_types )
GIO_CUSTOM_WRITEH(rival,	"Rival		  ", dedit_write_rival )
GIO_CUSTOM_READH( rival,	"Rival		  ", dedit_read_rival )
GIO_CUSTOM_WRITEH(race,		"Race         ", dedit_write_race )
GIO_CUSTOM_READH( race,		"Race         ", dedit_read_race )
GIO_FINISH

deity_type *deity_list;

/**************************************************************************/
// load em up
void load_deity_db( void )
{
	DEITY_DATA* pD = NULL;
	DEITY_DATA* pRival = NULL;

	logf("===Loading deity database from %s...", DEITY_FILE );
	GIOLOAD_LIST( deity_list, DEITY_DATA, DEITY_FILE );

	// Replace temporary rivals with proper ones from the deity_list;
	for ( pD=deity_list; pD; pD=pD->next )
	{
		if ( pD->rival && pD->rival->alignflags == -1 
					   && pD->rival->tendflags  == -1 )
		{
			if ( (pRival = deity_lookup(pD->rival->name) ) == NULL )
			{
				bugf("Dedit found unrecognised rival deity '%s' for '%s'", pD->rival->name, pD->name);
				free(pD->rival);
				pD->rival = NULL;
			} else {
				free(pD->rival);
				pD->rival = pRival;
			}
		}
	}

	log_string( "load_deity_db(): finished" );
}

/**************************************************************************/
// save the deity list
void save_deity_db( void )
{
	logf( "save_deity_db(): saving deity database to %s...", DEITY_FILE );
	GIOSAVE_LIST( deity_list, DEITY_DATA, DEITY_FILE, true );
}

/**************************************************************************/
// do func so it can be used as a command
void do_savedeities( char_data *ch, char * )
{
	save_deity_db();
	ch->println("Deities saved...");
	logf( "do_savedeities(): manual save of deities..." );
}

/**************************************************************************/
DEITY_DATA *deity_lookup( char *name )
{
    DEITY_DATA *pDeity;

	// first try an exact match
	for(pDeity=deity_list;pDeity;pDeity=pDeity->next){
		if(!str_cmp(name,pDeity->name)){
			return pDeity;
		}
	}

	// now try a prefix match
	for(pDeity=deity_list;pDeity;pDeity=pDeity->next){
		if(!str_prefix(name,pDeity->name)){
			return pDeity;
		}
	}

	// not found
    return NULL;
}

/**************************************************************************/
// OLC section
bool deity_create( char_data *ch, char *newName )
{
	DEITY_DATA	*node;
	static DEITY_DATA zero_node;

	node	   = new DEITY_DATA;
	*node	   = zero_node;
	node->next = deity_list;
	deity_list = node;

	deity_list->name			= str_dup( newName );
	deity_list->description		= str_dup( "" );
	deity_list->rival			= NULL;
	deity_list->symbol[0]		= 0;
	deity_list->symbol[1]		= 0;
	deity_list->symbol[2]		= 0;
	deity_list->symbol[3]		= 0;
	deity_list->max_followers	= 1;
	deity_list->race			= race_lookup( "human" );
	deity_list->sex				= sex_lookup( "male" );

	ch->desc->pEdit		= (void *)deity_list;
	ch->desc->editor	= ED_DEITY;
	ch->println("Deity Created.");
	return false;
}

/**************************************************************************/
void do_dedit( char_data *ch, char *argument )
{
	DEITY_DATA	*pDeity;
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
		ch->titlebar("DEDIT: SYNTAX");
		ch->println("syntax: dedit <deity>");
		ch->println("        dedit `=Ccreate`x to make a new deity.");
		ch->println("        dedit `=Clist`x lists existing deities.");
		return;
	}
	
	argument = one_argument( argument, arg );

	if ( !str_cmp( arg, "list" ))
	{
		DEITY_DATA	*pD;
		int			col = 0;

		ch->titlebar("Existing Deities");

		for( pD=deity_list;pD;pD=pD->next)
		{
            ch->printf(" %-40s", pD->name );
			if ( ++col % 2 == 0 )
				ch->println("");
		}
		ch->println("`x");
		return;
	}

	if ( !str_cmp( arg, "create" ))
	{
		one_argument( argument, arg );
		deity_create( ch, arg );
		return;
	}

	pDeity = deity_lookup( arg );
	
    if ( !pDeity )
    {
        ch->printlnf("There is no deity named '`Y%s`x'.", arg);
        return;
    }

	ch->desc->pEdit = (void *)pDeity;
	ch->desc->editor = ED_DEITY;
	ch->printlnf("Editing properties '%s'", pDeity->name );
	return;
}
/**************************************************************************/
bool dedit_name(char_data *ch, char *argument)
{
    DEITY_DATA		*pDeity;
	
    EDIT_DEITY( ch, pDeity );
	
    if ( argument[0] == '\0' )
    {
		ch->println("Syntax:   name [name]");
		return false;
    }
	
    free_string( pDeity->name );
    pDeity->name = str_dup( argument );
	
    ch->println("Deity name set.");
	
    return true;
}
/**************************************************************************/
bool dedit_show(char_data *ch, char *)
{
	DEITY_DATA *pD;
	
    EDIT_DEITY(ch, pD);

	if(IS_NULLSTR( pD->name )) {ch->println("NULL DEITY?!?");return false;}

	ch->printlnf("`=rName:  `x%-20s   `=r", pD->name );
	ch->wraplnf("`=rDescription: `x%s",	pD->description );
	ch->printlnf("`=rRace: `x%s     `=rSex:   `x%s",
		race_table[pD->race]->name,
		pD->sex == SEX_MALE ? "male" :
		pD->sex == SEX_FEMALE ? "female" : 
		pD->sex == 3 ? "random" : "neutral" );

	ch->printlnf("`=rRival Deity: `x%-20s    `=r",
		pD->rival == NULL ? "none" : pD->rival->name );
	ch->printlnf("`=rAlliance:    `x%-5d   `=rTendency:    `x%-5d",
		pD->alliance, pD->tendency );
	ch->printlnf("`=rShrine Vnum: `x%-5d",
		pD->shrinevnum);
	ch->printlnf("`=rSymbol1 Vnum: `x%-5d   `=rSymbol2 Vnum: `x%-5d",
		pD->symbol[0], pD->symbol[1] );
	ch->printlnf("`=rSymbol3 Vnum: `x%-5d   `=rSymbol4 Vnum: `x%-5d",
		pD->symbol[2], pD->symbol[3] );
	ch->printlnf("`=rAlignment Restrictions: [`x%s`=r]", 
		flag_string(align_flags, pD->alignflags) );
	ch->printlnf("`=rTendency Restrictions : [`x%s`=r]",
		flag_string(tendency_flags, pD->tendflags) );
	ch->printlnf("`=rRaces allowed: [`x%s`=r]", 
		race_get_races_set_for_n_array(pD->race_allow_n));

	return false;
}

/**************************************************************************/
bool dedit_shrinevnum( char_data *ch, char *argument )
{
	DEITY_DATA	*pD;
		
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: shrinevnum [vnum]");
		ch->println("        The shrinevnum denotes the room vnum where a player");
		ch->println("        is able to devote him/herself to the deity.  Should");
		ch->println("        be a remote place, so that the players have to try");
		ch->println("        and find it.");
		return false;
    }

	if ( !is_number( argument ))
	{
		ch->println("The vnum value must be numeric.");
		dedit_shrinevnum( ch, "" );		// redisplay the help
		return false;
	}

	if ( get_room_index( atoi( argument )) == '\0' )
	{
		ch->println("That room vnum doesn't exist.");
		return false;
	}

    EDIT_DEITY( ch, pD );

	pD->shrinevnum = atoi( argument );
	ch->printf("The shrine vnum is now set to '%d'.", pD->shrinevnum );
    return true;
}

bool dedit_rival( char_data *ch, char *argument )
{
	char arg[MIL];
	DEITY_DATA *pDeity;
	DEITY_DATA *pD;
	argument = one_argument(argument, arg);

	if ( IS_NULLSTR(arg))
	{
		ch->println( "Syntax: rival [deity name]." );
		return false;
	}

	EDIT_DEITY( ch, pD );

	if ( ( strstr(arg, "none") != '\0') )
	{
		pD->rival = NULL;
		ch->println( "Rival deity is now set to 'none'") ;
		return true;
	}

	if ( !(pDeity = deity_lookup( arg )) )
	{
		ch->printlnf( "`#`Y%s`^ is not a valid deity.", arg );
		return false;
	}


	if ( pDeity == pD )
	{
		ch->println("Rival deity cannot be the deity himself.");
		return false;
	}

	pD->rival = pDeity;
	ch->printlnf( "Rival deity is now set to '`#`Y%s`^'.", pD->rival->name );
	return true;
}

/**************************************************************************/
bool dedit_symbol( char_data *ch, char *argument )
{
	char arg[MIL];
	DEITY_DATA *pD;
	argument = one_argument(argument,arg);
		
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: symbol [number 1-4] [vnum]");
		ch->println("        The symbol denotes the symbol object that will be");
		ch->println("        used for when the player has stacked up enough favour");
		ch->println("        points.  They will then be able to summon the symbol to");
		ch->println("        show that they are loyal followers.");
		return false;
    }

	if ( !is_number( argument ) || !is_number(arg) )
	{
		ch->println("Both symbol number and  vnum value must be numeric.");
		dedit_symbol( ch, "" );		// redisplay the help
		return false;
	}

	if ( atoi(arg) < 1 || atoi(arg) > 4 )
	{
		ch->println( "The valid range for symbol numbers is between `#`W1`^ and `W4`^." );
		return false;
	}

	if ( get_obj_index( atoi( argument )) == '\0' )
	{
		ch->println("That object doesn't exist, create it first then set this value.");
		return false;
	}

    EDIT_DEITY( ch, pD );

	pD->symbol[atoi(arg)-1] = atoi( argument );
	ch->printlnf ( "The symbol%d vnum is now set as '%d'.", atoi(arg), atoi(argument) );
    return true;
}
/**************************************************************************/
bool dedit_alliance (char_data *ch, char *argument)
{
    DEITY_DATA *pD;
    int value;
	
    EDIT_DEITY( ch, pD );
	
    if ( argument[0] == '\0' || !is_number( argument ) )
    {
		ch->println("Syntax:  alliance [number]");
		return false;
    }
	
    value = atoi( argument );
	
    if (value>3 || value<-3)
    {
        ch->println("Alliance must be in the range -3 thru 3.");
        return false;
    }
    pD->alliance = value;
    ch->printlnf("Alliance set to %d.", pD->alliance);
    return true;
}

/**************************************************************************/
bool dedit_tendency(char_data *ch, char *argument)
{
    DEITY_DATA *pD;
    int value;
	
    EDIT_DEITY( ch, pD );
	
    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        ch->println("Syntax:  tendency [number]");
        return false;
    }
	
    value = atoi( argument );
	
    if (value>3 || value<-3)
    {
        ch->println("Tendency must be in the range -3 thru 3.");
        return false;
    }
    pD->tendency = value;
    ch->printlnf("Tendency set to %d.", pD->tendency);
    return true;
}
/**************************************************************************/
bool dedit_sex(char_data *ch, char *argument)
{
    DEITY_DATA *pD;
	EDIT_DEITY( ch, pD );
    int value;
	
    if ( argument[0] != '\0' )
    {
		
		if (( value = flag_value( sex_types, argument ) ) != NO_FLAG )
		{
			pD->sex = value;
			ch->println("Sex set.");
			return true;
		}
    }
	
	show_olc_options(ch, sex_types, "sex", "sex", pD->sex);
    return false;
}
/**************************************************************************/
bool dedit_race(char_data *ch, char *argument)
{
    DEITY_DATA *pD;
    int race;
	
    if ( argument[0] != '\0'
		&& ( race = race_lookup( argument )) >= 0 )
    {
		EDIT_DEITY( ch, pD );
		
		pD->race = race;
        ch->printlnf("Race set to '%s'.", race_table[race]->name);
		return true;
    }
	
    if ( argument[0] == '?' )
    {
		ch->print("Available races are:");
		
		for ( race = 1; race_table[race]; race++ )
		{
			if (( race % 3 ) == 0 )
				ch->println("");
			ch->printf(" %-15s", race_table[race]->name );
		}
		
		ch->println("");
		return false;
    }
	
    ch->printf("Syntax:  race [race]\r\n"
				 "Type 'race ?' for a list of races.\r\n" );
    return false;
}

/**************************************************************************/
bool dedit_maxfollowers(char_data *ch, char *argument)
{
    DEITY_DATA *pD;
    EDIT_DEITY( ch, pD );
    int value;
	
    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        ch->println("Syntax:  maxfollowers [number]");
        return false;
    }
	
    value = atoi( argument );
	
    if (value>1000 || value<0)
    {
        ch->println("Value must lie between 1 and 1000.");
        return false;
    }
    pD->max_followers = value;
    ch->printlnf("Max Followers set to %d.", pD->max_followers );
    return true;
}

/**************************************************************************/
bool dedit_tendflags( char_data *ch, char *argument )
{
	DEITY_DATA *pD;
	int value;

	if ( IS_NULLSTR( argument ))
	{
		ch->println("Syntax:  tendflag [flag].");
		ch->println("  Type '? tendflag' for a list.");
		return false;
	}

	value = flag_value( tendency_flags, argument );

	if ( value == NO_FLAG ) {
		ch->printlnf( "Invalid flag '%s'", argument );
		dedit_tendflags( ch, "" ); // redisplay the help
		return false;
	}

	EDIT_DEITY ( ch, pD );

	TOGGLE_BIT( pD->tendflags, value );
	ch->println( "Flag value toggled." );
	return true;
}
/**************************************************************************/
bool dedit_alignflags( char_data *ch, char *argument )
{
	DEITY_DATA *pD;
	int value;

	if ( IS_NULLSTR( argument ))
	{
		ch->println("Syntax:  alignflag [flag].");
		ch->println("  Type '? alignflag' for a list.");
		return false;
	}

	if ( (value = flag_value(align_flags, argument) ) == NO_FLAG )
	{
		ch->printlnf( "Invalid flag '%s'", argument );
		dedit_alignflags( ch, "" ); // redisplay the help
		return false;
	}

	EDIT_DEITY ( ch, pD );

	TOGGLE_BIT( pD->alignflags, value );
	ch->println( "Flag value toggled." );
	return true;
}

/**************************************************************************/
bool dedit_raceallow( char_data *ch, char *argument )
{
	DEITY_DATA *pD;
	EDIT_DEITY( ch, pD );
	int value;

	if ( IS_NULLSTR( argument )){
		ch->println("Syntax:  raceallow [race].");
		return false;
	}

	value = race_lookup(argument);
	if (value>=0)
	{
		TOGGLE_BITn(pD->race_allow_n, value);
		ch->printlnf( "Race '%s' toggled.", race_table[value]->name );
		return true;
	}

	ch->printlnf("Invalid race '%s'", argument );
	dedit_raceallow( ch, "" );	// redisplay the help
	return false;
}

/**************************************************************************/
void dedit_write_race(gio_type *gio_table, int tableIndex, void *data, FILE *fp)
{
	DEITY_DATA *pD;
	pD = (DEITY_DATA * ) data;
	fprintf(fp, "%s %s~\n",		
			gio_table[tableIndex].heading,	race_table[pD->race]->name);
}
/**************************************************************************/
void dedit_read_race(gio_type *, int, void *data, FILE *fp)
{
	DEITY_DATA *pD;
	char *pstr;

	pD = (DEITY_DATA * ) data;

	pstr=fread_string(fp);
	pD->race=race_lookup(pstr);
	if(pD->race<0){
		bugf("Dedit found unrecognised race '%s' for '%s'", pstr, pD->name);
		pD->race=0;
	}
	free_string(pstr);
}

/**************************************************************************/
void dedit_write_rival(gio_type *gio_table, int tableIndex, void *data, FILE *fp)
{
	DEITY_DATA *pD;

	pD = (DEITY_DATA * ) data;
	if ( !pD->rival )
		fprintf(fp, "%s none~\n", gio_table[tableIndex].heading);
	else
		fprintf(fp,"%s %s~\n", gio_table[tableIndex].heading , pD->rival->name);
}

/**************************************************************************/
void dedit_read_rival(gio_type *, int, void *data, FILE *fp)
{
	DEITY_DATA *pD, *pRival;
	char *pStr;

	pD = (DEITY_DATA *) data;

	pStr = fread_string(fp);
	if ( strstr(pStr, "none") != '\0' ) {
		pD->rival = NULL;
		return;
	}

	if ( (pRival = deity_lookup(pStr)) == NULL )
	{
		pRival = new DEITY_DATA;
		pRival->name = pStr;
		pRival->alignflags = -1;
		pRival->tendflags = -1;
		pD->rival = pRival;
	} else {
		pD->rival = pRival;
	}
	free_string(pStr);
}

/**************************************************************************/
bool dedit_description(char_data *ch, char *argument)
{
	DEITY_DATA *pD;
    EDIT_DEITY( ch, pD );
	
	if(!IS_NULLSTR(argument)){
		ch->println("Syntax:  description (string editor used)");
		ch->println("uses the string editor to edit/add the deity description.");
		return false;
	}
	string_append( ch, &pD->description);
	return true;
}
/**************************************************************************/
