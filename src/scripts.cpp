/**************************************************************************/
// scripts.cpp - src file for scripting commands written by Celrion
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "scripts.h"

SCRIPT_DATA *script_list;
/**************************************************************************/
//Create script_data GIO lookup table
GIO_START(SCRIPT_DATA)
GIO_STRH(script_name,	"Script_Name		")
GIO_STRH(auth_users,	"Authorized_Users	")
GIO_FINISH
/**************************************************************************/
//Loads up the script database
void load_script_db( void )
{
	logf("===Loading script database from %s...", SCRIPTS_FILE);	
	if(file_exists(SCRIPTS_FILE)){
		GIOLOAD_LIST(script_list, SCRIPT_DATA, SCRIPTS_FILE); 
	}else{
		logf("Scripts database file not present - "
			"this is normal if no scripts have been defined.");
	}
	log_string ("load_script_db(): finished");
}

/**************************************************************************/
//Save the script database
void save_script_db( void )
{
	logf("save_script_db(): saving script database to %s", SCRIPTS_FILE);
	GIOSAVE_LIST(script_list, SCRIPT_DATA, SCRIPTS_FILE, true);
}
/**************************************************************************/
//Returns pointer to script_data node
script_data *find_script_node( char *name )
{
	script_data *node;

	for (node = script_list; node; node = node->next)
	{
		if ( !strcmp ( name, node->script_name ) )
			return node;
	}

	return NULL;
}
/**************************************************************************/
// Creates a node of type script_data
script_data *create_script_node( char_data *ch, char *scriptname, char *name )
{
	script_data *node;
	static script_data zero_node;

	node = new script_data;
	*node = zero_node;

	// add the new node to the head of the script_list
	node->next = script_list;
	script_list = node;

	smash_tilde(scriptname);

	node->script_name=str_dup(scriptname);

	if( name )
	{
		node->auth_users=str_dup(name);
		ch->printlnf("A new script named %s has been added, with %s as an authorized user.",
			node->script_name, node->auth_users);
	}
	else
	{
		node->auth_users=str_dup("None");
		ch->printlnf("A new script named '%s' has been added.", node->script_name);
	}

	save_script_db();
	return node;
}
/**************************************************************************/
// if it returns NULL the scriptname is valid
// otherwise it returns a text message stating why it isn't valid
char *is_valid_scriptname(char *scriptname)
{
	static char result[MSL];
	char *bad_characters=INVALID_CHARACTERS_FROM_USERS_FOR_SYSTEM_CALLS;

	if (has_whitespace(scriptname)){
		sprintf(result,"The script filename can not have any whitespace "
			"characters in it!`1try again:`1");
		return result;
	}
	{ // check for the bad characters
		int i;
		for(i=0; bad_characters[i];i++){
			if(count_char(scriptname, bad_characters[i])>0){
				sprintf(result,"%c is not an allowed character allowed in a script name -"
					"try again.`1", bad_characters[i]);				
				return result;
			}
		}
	}
	return NULL;
}
/**************************************************************************/
void do_addscript( char_data *ch, char *argument )
{
	script_data *node;
	char scriptname[MIL], filename[MIL], immname[MIL], buf[MIL];

	argument = one_argument( argument, scriptname );
	one_argument( argument, immname );

	if (IS_NULLSTR(scriptname))
	{
		ch->println("syntax: Addscript <script name> [immortal name]");
		return;
	}

	// ensure the scriptname wont stuff things up
	{
		char *sresult=is_valid_scriptname(scriptname);
		if(sresult){
			ch->print(sresult);
			return;
		}
	}

	node = find_script_node(scriptname);
	sprintf(filename, "%s%s", SCRIPTS_DIR, scriptname);

	if (IS_NULLSTR(immname))
	{
		if(node)
		{
			ch->printlnf("The script %s already exists!", node->script_name);
			return;
		}
		else
		{
			if ( !file_exists(filename) )
			{
				ch->printlnf("There is no script named '%s' in the scripts directory.", scriptname);
				{
					char bufcommand[MSL],buf2[MSL];
					BUFFER *output;

					output= new_buf();
					#ifdef unix
						sprintf( bufcommand,"ls " SCRIPTS_DIR " -al -t");
					#else
						sprintf( bufcommand,"dir " SCRIPTS_DIR );
					#endif
						sprintf( buf2,"\r\n`?%s`x", 
							format_titlebarf("SCRIPTS DIR - Piping:`x %s", bufcommand));
						add_buf(output,buf2);
						add_buf(output,get_piperesult(bufcommand));

					sprintf( buf2,"`^%s`x", format_titlebar("-"));
					add_buf(output,buf2);
					ch->sendpage(buf_string(output));
					free_buf(output);
				}
				return;
			}

			create_script_node( ch, scriptname, NULL );
			return;
		}
	}
	else
	{
		immname[0] = UPPER( immname[0] );

		if (!node)
		{
			if ( !file_exists(filename) )
			{
				ch->printlnf("There is no script named '%s' in the scripts directory.", scriptname);
				{
					char bufcommand[MSL],buf2[MSL];
					BUFFER *output;

					output= new_buf();
					#ifdef unix
						sprintf( bufcommand,"ls " SCRIPTS_DIR " -al -t");
					#else
						sprintf( bufcommand,"dir " SCRIPTS_DIR );
					#endif
						sprintf( buf2,"\r\n`?%s`x", 
							format_titlebarf("SCRIPTS DIR - Piping:`x %s", bufcommand));
						add_buf(output,buf2);
						add_buf(output,get_piperesult(bufcommand));

					sprintf( buf2,"`^%s`x", format_titlebar("-"));
					add_buf(output,buf2);
					ch->sendpage(buf_string(output));
					free_buf(output);
				}
				return;
			}

			create_script_node( ch, scriptname, immname );
			return;
		}
		
		if ( is_exact_name( immname, node->auth_users ) )
		{
			ch->printlnf("%s is already an authorized user of this script.", immname);
			return;
		}

		buf[0] = '\0';

		if ( strstr( node->auth_users, "None" ) != '\0' )
		{
			node->auth_users = string_replace( node->auth_users, "None", "" );
			node->auth_users = ltrim_string(rtrim_string( node->auth_users ));
		}

		if ( node->auth_users[0] != '\0' )
		{
			strcat( buf, node->auth_users );
			strcat( buf, " " );
		}
		strcat( buf, immname );
		free_string( node->auth_users );
		node->auth_users = string_proper( str_dup( buf ) );

		ch->printlnf("Authorized User added to script. Current authorized users:\r\n%s", 
			node->auth_users );
		save_script_db();
		return;
	}
}
/**************************************************************************/
void do_delscript( char_data *ch, char *argument )
{
	char scriptname[MIL], immname[MIL];
	script_data *node = NULL, *previous = NULL, *current = NULL;

	if( IS_NULLSTR( argument ) )
	{
		ch->println("Syntax: delscript <script_name> [authorized_users]");
		ch->println("With an authorized users name, it will just remove the user,");
		ch->println("Without a user listed at all, it will remove the script entirely.");
		return;
	}

	argument = one_argument( argument, scriptname );
	one_argument( argument, immname );

	node = find_script_node(scriptname);
	
	if( !node )
	{
		ch->printlnf("There is no script named '%s' in the script database.", scriptname );
		return;
	}

	if( IS_NULLSTR( immname ) )
	{
		ch->printlnf("%s has been removed from the scripts database.", node->script_name );

		if( node == script_list ) // Stripping from head of list
			script_list = script_list->next;
		else
		{
			previous = script_list;
			for( current = script_list->next; current; current = current->next)
				if ( node == current )
					break;
				previous = current;
		}

		if( current )
			previous->next = current->next;
	} 
	else 
	{
		immname[0] = UPPER( immname[0] );

		if ( !is_exact_name( immname, node->auth_users ) )
		{
			ch->printlnf("%s is not an authorized user of this script.", immname );
			return;
		}
		
		if ( strstr( node->auth_users, immname ) != '\0' ){
			node->auth_users = string_replace( node->auth_users, immname, "\0" );
			node->auth_users = ltrim_string(rtrim_string( node->auth_users ));

			if ( node->auth_users[0] == '\0' )
			{
				free_string( node->auth_users );
				node->auth_users = str_dup( "None" );
			}

			ch->printlnf("%s has been removed from authorized users.", immname );
			ch->printlnf("Current authorized users:\r\n%s", node->auth_users);
		}else{
			ch->println("bug");
		}
	}
	save_script_db();
	return;
}
/**************************************************************************/
void do_listscripts( char_data *ch, char *argument )
{
	script_data *node;
	int count = 1;
	char scriptname[MIL];

	if( IS_NULLSTR( argument ) )
	{
		ch->titlebar("Scripts - All");

		for( node = script_list; node; node = node->next, count++)
		{
			ch->printlnf("%-2d -  `#`CScript name:`^ %s", count, node->script_name );
			ch->printlnf("      `#`CUsers:`^       %s", node->auth_users );
		}
	}else{
		one_argument( argument, scriptname );
		node = find_script_node( scriptname );

		if( node )
		{
			ch->printlnf("`#`CScript name:`^ %s", node->script_name);
			ch->printlnf("`#`CUsers:`^       %s", node->auth_users);
		}
		else
			ch->printlnf("There was no script named '%s' found.", scriptname );

	}
	return;
}
/**************************************************************************/
void do_runscript( char_data *ch, char *argument )
{
	script_data *node;
    char filename[MIL], scriptname[MIL];

	one_argument( argument, scriptname );

	if( !str_cmp( TRUE_CH(ch)->name, "none")){
		ch->println("You can't run scripts if your name is 'None'!");
		return;
	}

	if( IS_NULLSTR( argument ) )
	{
		ch->printf("syntax: runscript <scriptname>\r\n"
			"You may run any of the following scripts:\r\n");

		for( node = script_list; node; node = node->next)
		{
			if ( !is_exact_name( TRUE_CH(ch)->name, node->auth_users ) ){
				continue;
			}

			ch->printlnf("`#`CScript name:`^ %s", node->script_name );
			if(IS_ADMIN(ch)){
				ch->printlnf("      `#`CUsers:`^       %s", node->auth_users );
			}
		}
		return;
	}

	node = find_script_node(scriptname);

	if ( !node )
	{
		ch->printlnf("There is no script named '%s' in the database.", scriptname );
		return;
	}

	
	if ( !is_exact_name( TRUE_CH(ch)->name, node->auth_users ) )
	{
		ch->println("You're not authorized to run that script.");
		return;
	}

	sprintf(filename, "%s%s", SCRIPTS_DIR, scriptname);
#ifdef unix
	strcat(filename, " &");
#endif
	int ret=system(filename);
	if(ret<0){
		logf("do_runscript() returned code=%d (error), errno=%d (%s) running script '%s' - used by %s, filename='%s'", ret, errno, strerror(errno), 
			scriptname, TRUE_CH(ch)->name, filename);
		ch->printlnf("Script %s returned error code=%d (error), errno=%d (%s)", scriptname, ret, errno, strerror(errno));
		return;
	}

	ch->printlnf("Script %s successfully run!", scriptname);	
}

/**************************************************************************/
