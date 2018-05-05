/**************************************************************************/
// comedit.cpp - Code dealing with dynamic editing of commands, Ker & Kal
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
#include "olc_ex.h"
#include "security.h"
#include "interp.h"

// Local Prototypes
void do_write_commandtable(char_data *ch, char *);
int command_exact_lookup( const char *name );
int command_lookup( const char *name );

/**************************************************************************/
GIO_START(cmd_type)
GIO_SHINTH(level,				"Level       ")
GIO_SHWFLAGH(log,				"Log         ", commandlog_types)
GIO_SHINTH(show,				"Show        ")
GIO_WFLAGH(flags,				"Flags       ", commandflag_flags)
GIO_WFLAGH(council,				"Council     ", council_flags)
GIO_WFLAGH(grantgroups,			"GrantGroups ", grantgroup_flags)
GIO_FINISH_NOCLEAR	
/**************************************************************************/
// use a typedef to get 2 different gio profiles for 1 datatype
typedef cmd_type cmd_type_categories; 

GIO_START(cmd_type_categories)
GIO_SHWFLAGH(category,			"Category    ", com_category_types)
GIO_SHWFLAGH(position,			"Position    ", position_types)
GIO_FINISH_NOCLEAR	
/**************************************************************************/
void do_write_commandcategories(char_data *ch, char *)
{
	FILE *fp;
	int i;

	logf("Writing command categories and positions to %s...", COMMANDS_CATEGORIES_FILE );
	fclose( fpReserve );

	unlink(COMMANDS_CATEGORIES_FILE".bak");
	rename(COMMANDS_CATEGORIES_FILE, COMMANDS_CATEGORIES_FILE".bak");

    if ( ( fp = fopen( COMMANDS_CATEGORIES_FILE, "w" ) ) == NULL ){
		bugf("do_write_commandcategories(): fopen '%s' for write - error %d (%s)", 
			COMMANDS_CATEGORIES_FILE, errno, strerror( errno));
		ch->printlnf( "An error occurred! writing to " COMMANDS_CATEGORIES_FILE"." );
    }else{
		// LOOP thru everything in the table, writing it
		for( i= 0; !IS_NULLSTR(cmd_table[i].name); i++ ){
			fprintf( fp, "Name         %s~\n", cmd_table[i].name);
			GIO_SAVE_RECORD(cmd_type_categories, &cmd_table[i], fp, NULL);
		}
		fprintf(fp, "EOF~\n");
		
		fclose( fp );
		ch->printlnf( "Finished writing command categories and positions to %s.",
			COMMANDS_CATEGORIES_FILE);
		logf( "Finished writing command categories and positions to %s.",
			COMMANDS_CATEGORIES_FILE);
    }
	fpReserve = fopen( NULL_FILE, "r" );
}
/**************************************************************************/
void do_write_commandtable(char_data *ch, char *)
{
	FILE *fp;
	int i;

	logf("Writing command table to %s...", COMMANDS_FILE );
	fclose( fpReserve );

	unlink(COMMANDS_FILE".bak");
	rename(COMMANDS_FILE, COMMANDS_FILE".bak");

    if ( ( fp = fopen( COMMANDS_FILE, "w" ) ) == NULL ){
		bugf("do_write_commandtable(): fopen '%s' for write - error %d (%s)", 
			COMMANDS_FILE, errno, strerror( errno));
		ch->printlnf( "An error occurred! writing to " COMMANDS_FILE "." );
    }else{
		// LOOP thru everything in the table, writing it
		for(i=0; !IS_NULLSTR(cmd_table[i].name); i++ )
		{
			fprintf(fp, "Name         %s~\n", cmd_table[i].name);
			GIO_SAVE_RECORD(cmd_type, &cmd_table[i], fp, NULL);
		}
		fprintf(fp, "EOF~\n");
		
		fclose( fp );
		ch->printlnf("Finished writing commandtable to " COMMANDS_FILE ".");
		logf("Finished writing commandtable to " COMMANDS_FILE ".");
    }
	fpReserve=fopen(NULL_FILE, "r" );

	do_write_commandcategories(ch,"");
}

/**************************************************************************/
void do_read_commandcategories(char_data *ch, char *)
{
	FILE *fp;
	int count=0;

	logf("Reading in command categories and positions from %s...", COMMANDS_CATEGORIES_FILE);
	fclose( fpReserve );

    if ( ( fp = fopen( COMMANDS_CATEGORIES_FILE, "r" ) ) == NULL )
    {
		bugf("do_read_commandcategories(): fopen '%s' for read - error %d (%s)", 
			COMMANDS_CATEGORIES_FILE, errno, strerror( errno));
		ch->printlnf( "An error occurred trying to open %s for reading!", COMMANDS_CATEGORIES_FILE );
    }
    else
    {
		bool morefile=true;
		char *readword;
		char buf[MIL];
		int i;

		while (morefile && !feof(fp)) {
			readword= fread_word(fp);

			if (!str_cmp(readword, "EOF") || !str_cmp(readword, "EOF~")){
				morefile=false;
			}else{
				if(!str_cmp(readword, "name")){
					readword=fread_string(fp); // freed
					sprintf(buf,"%s", readword);
					free_string(readword);
	
					i=command_lookup(buf);
					if(i>=0){
						GIO_LOAD_RECORD(cmd_type_categories, &cmd_table[i], fp);
						count++;
					}else{
						cmd_type t;
						bugf("UNKNOWN COMMAND IN '%s'\n---------- Found '%s' "
							"expecting a known command name", COMMANDS_CATEGORIES_FILE, buf);
						GIO_LOAD_RECORD(cmd_type_categories, &t, fp);
					}
				}else{// unexpected file format
					bugf("Unexpected fileformat in '%s' - found '%s' "
						"expecting 'name'", COMMANDS_CATEGORIES_FILE, readword);
					do_abort();
					return;
				}
			}
		}
		fclose( fp );

		ch->printlnf( "Finished reading command categories from %s. (read in %d)",
			COMMANDS_CATEGORIES_FILE,
			count );
    }
	fpReserve = fopen( NULL_FILE, "r" );

	logf("Finished reading command categories. (read in %d)", count);
	return;
}

/**************************************************************************/
void do_read_commandtable(char_data *ch, char *)
{
	FILE *fp;
	int count=0;

	logf("Reading in command table from %s...", COMMANDS_FILE );
	fclose( fpReserve );

    if ( ( fp = fopen( COMMANDS_FILE, "r" ) ) == NULL )
    {
		bugf("do_read_commandtable(): fopen '%s' for read - error %d (%s)", 
			COMMANDS_FILE, errno, strerror( errno));
		ch->printlnf( "An error occurred trying to open %s for reading!", COMMANDS_FILE );
    }else{
		bool morefile=true;
		char *readword;
		char buf[MIL];
		sh_int i;

		while (morefile && !feof(fp)) {
			readword= fread_word(fp);

			if (!str_cmp(readword, "EOF") || !str_cmp(readword, "EOF~")){
				morefile=false;
			}else{
				if(!str_cmp(readword, "name")){
					readword=fread_string(fp); // freed
					sprintf(buf,"%s", readword);
					free_string(readword);
	
					i=command_lookup(buf);
					if(i>=0){
						GIO_LOAD_RECORD(cmd_type, &cmd_table[i], fp);
						count++;
					}else{
						cmd_type t;
						bugf("UNKNOWN COMMAND IN '%s'\n---------- Found '%s' "
							"expecting a known command name", COMMANDS_FILE, buf);
						GIO_LOAD_RECORD(cmd_type, &t, fp);
					}
				}else{// unexpected file format
					bugf("Unexpected fileformat in '%s' - found '%s' "
						"expecting 'name'", COMMANDS_FILE, readword);
					do_abort();
					return;
				}
			}
		}
		fclose( fp );

		ch->printlnf( "Finished reading commandtable from %s. (read in %d)",
			COMMANDS_FILE,
			count );
    }
	fpReserve = fopen( NULL_FILE, "r" );

	logf("Finished reading commandtable. (read in %d)", count);
	
	do_read_commandcategories(ch,"");	
}
/**************************************************************************/
int command_lookup( const char *name )
{
    int i;

	i=command_exact_lookup(name);

	if(i>-1){
		return i;
	}

    for ( i = 0; !IS_NULLSTR(cmd_table[i].name); i++ )
    {
		if ( LOWER(name[0]) == LOWER(cmd_table[i].name[0])
		&&   !str_prefix( name, cmd_table[i].name ) )
			return i;
    }
    return -1;
}
/**************************************************************************/
int command_exact_lookup( const char *name )
{
    int i;

    for ( i = 0; !IS_NULLSTR(cmd_table[i].name); i++ )
    {
		if ( LOWER(name[0]) == LOWER(cmd_table[i].name[0])
		&&   !str_cmp( name, cmd_table[i].name ) )
			return i;
    }
    return -1;
}
/**************************************************************************/
void do_comedit( char_data *ch, char *argument )
{
	int		i;
	char	arg[MIL];

	if ( IS_NPC( ch )){
		ch->println("Players only.");
		return;
	}

	// do security checks
	if (!HAS_SECURITY(ch, 2))
	{
		ch->println("You must have an olc security 2 or higher to use this command.");
		return;
	}

    if ( !HAS_SECURITY(ch, COMEDIT_MINSECURITY))
    {
    	ch->println("comEdit: Insufficient security to modify commands.");
    	return;
    }

	if ( !IS_TRUSTED(ch, COMEDIT_MINTRUST)) {
		ch->printlnf( "You must have a trust of %d or above to use this command.", COMEDIT_MINTRUST);
		return;
	}

	if (IS_NULLSTR(argument)){
		ch->titlebar("COMEDIT: SYNTAX");
		ch->println("syntax: comedit <command>");
		return;
	}
	
	// using ' codes are optional
	if(!str_infix("'",argument)){
		argument = one_argument( argument, arg );
	}else{
		strcpy(arg, argument);
	}

	int display=0;
    if ( ( i = command_exact_lookup( arg ) ) < 0)
    {
        ch->printlnf( "There is no command with the exact name '%s'.", arg );
		ch->println( "Are you looking for one of the following:");

		for ( i = 0; !IS_NULLSTR(cmd_table[i].name); i++ )
		{
			if (!str_prefix( arg, cmd_table[i].name ) ){		
				ch->printf("`x  [%3d]%-25s`x", 
					cmd_table[i].level,
					cmd_table[i].name);
				if(++display%3==0){
					ch->println("");
				}
			}		
		}
		if(display==0){
			ch->println( "`Sno prefix matches either.`x");
		}else{
			if(display%3!=0){
				ch->println("");
			}
		}
        return;
    }

    ch->desc->pEdit	= &cmd_table[i];
	ch->desc->editor = ED_COMMAND;

	ch->printlnf( "`WEditing command properties of '%s' (level %d)`x", 
		cmd_table[i].name, cmd_table[i].level);

	if(cmd_table[i].name[1]=='\0'){
		ch->println("`sLonger commands starting with the same letter include:");
		display=0;
		for ( int j = 0; !IS_NULLSTR(cmd_table[j].name); j++ )
		{
			if (*cmd_table[i].name==*cmd_table[j].name ){	
				ch->printf("`s  [%3d]%-25s`x", 
					cmd_table[j].level,
					cmd_table[j].name);
				if(++display%3==0){
					ch->println("");
				}
			}
		}
		if(display%3!=0){
			ch->println("");
		}
		ch->printlnf( "`WEditing command properties of '%s' (level %d)`x", 
			cmd_table[i].name, cmd_table[i].level);
	}

	return;
}
/**************************************************************************/
bool comedit_level(char_data *ch, char *argument)
{
	cmd_type * pC;

    if ( IS_NULLSTR(argument))
    {
		ch->println(" Syntax:  level [#]");
		ch->println("`SThis is the minimum level/trust you must be to use the command.");
		return false;
	}
	
	if(!is_number(argument)){
		ch->println("The level value must be numeric");
		comedit_level(ch,""); // redisplay the help
		return false;
	}

	EDIT_COMMAND(ch, pC);
	
    pC->level= atoi(argument);
    ch->printlnf( "Level set to '%d'.", pC->level );
	return true;
}
/**************************************************************************/
bool comedit_show(char_data *ch, char *)
{
	cmd_type * pC;
	
    EDIT_COMMAND(ch, pC);

	ch->printlnf( "`=rCommand Name: `x%s", pC->name);
	ch->printlnf( "`=rLevel       : `x%d", pC->level);
	ch->printlnf( "`=rVisible     : `x%s",	pC->show ? "Yes" : "No" );
	mxp_display_olc_flags(ch, position_types,	pC->position,	"position",	"Position:");
	mxp_display_olc_flags(ch, commandlog_types,	pC->log,	"log",	"Log Type:");
	mxp_display_olc_flags(ch, council_flags,	pC->council,	"council",	"Council(s):");
	mxp_display_olc_flags(ch, commandflag_flags,	pC->flags,	"flag",	"Flag(s):");
 	mxp_display_olc_flags(ch, grantgroup_flags,	pC->grantgroups,	"grantgroups",	"Grantgroup(s):");
	ch->wrapln("Note: if the ic flag is set, the command can not be "
		"used in OOC (and vice versa for the ooc flag).");
	ch->wraplnf("Note2: if no grantgroups are set, only levels will be used.  "
		"level %d can access all commands regardless of grant status.", MAX_LEVEL);
	mxp_display_olc_flags(ch, com_category_types,	pC->category,	"category",	"Category:");
    return false;
}
/**************************************************************************/
bool comedit_position(char_data *ch, char *argument)
{
	cmd_type * pC;
	int value;

    if ( IS_NULLSTR(argument))
    {
		ch->println(" Syntax:  position <position>");
		ch->wraplnf( " Valid positions include: %s", flagtable_names(position_types));
		return false;
	}

	value = flag_value( position_types, argument);
	if(value == NO_FLAG ){
		ch->printlnf( "Invalid position '%s'.", argument);
		comedit_position(ch,""); // redisplay the help
		return false;
	}

	EDIT_COMMAND(ch, pC);
	
    pC->position = value;
    ch->printlnf( "Position set to '%s'.", position_table[value].name);
	return true;
}
/**************************************************************************/
bool comedit_visible(char_data *ch, char *)
{
	cmd_type * pC;

	EDIT_COMMAND(ch, pC);
	
	if ( pC->show == 0 ) {
		pC->show = 1;
		ch->printlnf( "'%s' will now be visible in the commands list/wizhelp.", pC->name);
	}else{
		pC->show = 0;
		ch->printlnf( "'%s' will `Yno longer`x be visible in the commands list/wizhelp.", pC->name);
	}
	return true;
}
/**************************************************************************/
bool comedit_inviscmds( char_data *ch, char *)
{
    int i, j;

	j = 1;

    for ( i = 0; !IS_NULLSTR(cmd_table[i].name); i++ )
    {
		if ( !cmd_table[i].show )
		{
			ch->printf( "%20s", cmd_table[i].name );
			if ( j == 4 )
			{
				ch->printf( "\r\n" );
				j = 0;
			}
			j++;
		}
    }
	ch->printf( "\r\n" );

    return true;
}
/**************************************************************************/
bool comedit_logtype(char_data *ch, char *argument)
{
	cmd_type * pC;
	int value;

    if ( IS_NULLSTR(argument))
    {
		ch->println(" Syntax: log <logtype>");		
		ch->wraplnf("Valid logtypes include: %s", flagtable_names(commandlog_types));
		return false;
	}

	value = flag_value( commandlog_types, argument);
	if(value == NO_FLAG ){
		ch->printlnf( "Invalid logtype '%s'.", argument);
		comedit_logtype(ch,""); // redisplay the help
		return false;
	}

	EDIT_COMMAND(ch, pC);
	
    pC->log = value;
    ch->printlnf( "Log set to '%s'.", argument );
	return true;
}
/**************************************************************************/
bool comedit_council(char_data *ch, char *argument)
{
	cmd_type * pC;
	int value;

    if ( IS_NULLSTR(argument))
    {
		ch->println(" Syntax:  council <councilname>");
		ch->wraplnf( " Valid councils include: %s.", 
			flagtable_names(council_flags));
		return false;
	}

	value = flag_value( council_flags, argument);
	if(value == NO_FLAG ){
		ch->printlnf( "Invalid council '%s'.", argument);
		comedit_council(ch,""); // redisplay the help
		return false;
	}

	EDIT_COMMAND(ch, pC);
	
    TOGGLE_BIT(pC->council,value);
    ch->printlnf( "Council flag '%s' toggled.",
		flag_string( council_flags, value));
		
	ch->printlnf( "Councils for '%s' are now: %s", pC->name,
		flag_string( council_flags, pC->council ));
	return true;
}
/**************************************************************************/
bool comedit_flags(char_data *ch, char *argument)
{
	cmd_type * pC;
	int value;

    if ( IS_NULLSTR(argument))
    {
		ch->println(" Syntax:  flag [type]");
		ch->wraplnf( "Valid flags include: %s", flagtable_names(commandflag_flags));
		return false;
	}

	value = flag_value( commandflag_flags, argument);
	if(value == NO_FLAG ){
		ch->printlnf( "Invalid flag '%s'.", argument);
		comedit_flags(ch,""); // redisplay the help
		return false;
	}

	EDIT_COMMAND(ch, pC);
	
    pC->flags^=value;
    ch->printlnf( "'%s' flag toggled, now flags are '%s'.",
		capitalize(argument),flag_string( commandflag_flags, pC->flags ));
	return true;
}

/**************************************************************************/
int com_category_lookup(char *name);
/**************************************************************************/
bool comedit_category( char_data *ch, char *argument)
{
	cmd_type * pC;
	int newcat;
	char arg[MIL];
	
    EDIT_COMMAND( ch, pC );

	argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg))
    {
		ch->println("Syntax: category <category name>");
		do_com_categorylist(ch,"");
		return false;
    }

	newcat=com_category_lookup(arg);

	if (newcat == pC->category )
	{
		ch->println("No change in category.");
		return false;
	}

	if (newcat>=0)
	{
		ch->printlnf( "Category of '%s' changed from '%s' to '%s'.",
			pC->name, 
			com_category_indexlookup( pC->category),
			com_category_indexlookup( newcat));
		pC->category=newcat;
		return true;
	}

	ch->printlnf( "Couldn't find '%s' try one of the following:", arg);
	do_com_categorylist(ch,"");
    return false;
}

/**************************************************************************/
void do_com_categorylist( char_data *ch, char * )
{
	int flag;

	ch->titlebar("CATEGORIES LIST");
	for (flag = 0; com_category_types[flag].name; flag++)
	{
		ch->printlnf( "%s", com_category_types[flag].name);
	}
};
/**************************************************************************/
bool comedit_grantgroups(char_data *ch, char *argument)
{
	cmd_type * pC;
	EDIT_COMMAND(ch, pC);
    int value;
	
    if(!IS_NULLSTR(argument)){	
		if ( ( value = flag_value( grantgroup_flags, argument ) ) != NO_FLAG )
		{
			pC->grantgroups^= value;
			ch->println("grantgroups toggled.");
			return true;
		}
    }

	show_olc_options(ch, grantgroup_flags, "grantgroups", "grantgroups", pC->grantgroups);
	ch->println("PLEASE NOTE: The grant system is incomplete.");
    return false;	
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

