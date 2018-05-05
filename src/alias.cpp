/**************************************************************************/
// alias.cpp - command aliases 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with all the licenses *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************
 * >> Original Diku Mud copyright (c)1990, 1991 by Sebastian Hammer,       *
 *    Michael Seifert, Hans Henrik Staerfeldt, Tom Madsen, & Katja Nyboe.  *
 * >> Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael       *
 *    Chastain, Michael Quan, and Mitchell Tse.                            *
 * >> ROM 2.4 is copyright 1993-1995 Russ Taylor and has been brought to   *
 *    you by the ROM consortium: Russ Taylor(rtaylor@pacinfo.com),         *
 *    Gabrielle Taylor(gtaylor@pacinfo.com) & Brian Moore(rom@rom.efn.org) *
 * >> Oblivion 1.2 is copyright 1996 Wes Wagner                            *
 **************************************************************************/

#include "include.h" // dawn standard includes

/***************************************************************************/
// does aliasing and other fun stuff, multibuffered to avoid problems with
// the use of the force command
char * substitute_alias(connection_data *d, char *argument)
{
    char_data *ch;
	static char multi_command_buf[3][MSL*2];
	static int index;
	++index%=3;
	char *command_buf=multi_command_buf[index];
    char buf[MSL*3],prefix[MIL],name[MIL],arg1[MIL],arg2[MIL],arg3[MIL];
    char *point, *alias_sub, *big_arg; //Added *alias_sub to allow for arguments
    int alias;
    bool is_inline = false;

    ch = d->original ? d->original : d->character;

    // check for prefix 
    if (ch->prefix[0] != '\0' && str_prefix("prefix",argument))
    {
		if (str_len(ch->prefix) + str_len(argument) > MIL)
			ch->println("Line too long, prefix not processed.");
		else
		{
			sprintf(prefix,"%s %s",ch->prefix,argument);
			argument = prefix;
		}
    }

    if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||	!str_prefix("alias",argument) || !str_prefix("una",argument) 
    ||  !str_prefix("prefix",argument)) 
    {
		strcpy(command_buf,argument);
		return command_buf;
	}
	
	strcpy(buf,argument);
	
	for (alias = 0; alias < MAX_ALIAS; alias++)	 // go through the aliases 
	{
		if (ch->pcdata->alias[alias] == NULL)
			break;

		if (!str_prefix(ch->pcdata->alias[alias],argument))
		{
			point = one_argument(argument,name);
			big_arg = point;
			point = one_argument(point,arg1);
			point = one_argument(point,arg2);
			point = one_argument(point,arg3);
			alias_sub = ch->pcdata->alias_sub[alias];
			
			if (!strcmp(ch->pcdata->alias[alias],name))
			{
				buf[0] = '\0';

				// Old: strcat(buf,ch->pcdata->alias_sub[alias]);

				// My changes here -- Celrion			
				for(; *alias_sub != '\0'; alias_sub++)
				{
					if (*alias_sub == '%')
					{
						switch (*(alias_sub+1))
						{
						case '1':
							strcat(buf,arg1);
							alias_sub++;
							is_inline = true;
							break;
						case '2':
							strcat(buf,arg2);
							alias_sub++;
							is_inline = true;
							break;
						case '3':
							strcat(buf,arg3);
							alias_sub++;
							is_inline = true;
							break;
						default:
							strncat(buf, alias_sub, 1);
							break;
						}
					}
					else
						strncat(buf, alias_sub, 1);
				}
				if (is_inline == false )
				{
					strcat(buf," ");
					if (buf[str_len(buf)-2]=='_')
					{
						buf[str_len(buf)-2]='\0';
					}
					strcat(buf,big_arg);
				}
			}
			if (str_len(buf) > MIL-1)
			{
				ch->println("Alias substitution too long. Truncated.");
				buf[MIL -1] = '\0';
			}
			break;
		}
    }
	strcpy(command_buf,buf);
	return command_buf;
}

/***************************************************************************/
void do_alia(char_data *ch, char *)
{
	ch->println("I'm sorry, alias must be entered in full.");
	return;
}

/***************************************************************************/
void do_alias(char_data *ch, char *argument)
{
	char_data *rch;
	char arg[MIL],buf[MSL];
	int pos;

	if (ch->desc == NULL)
		rch = ch;
    else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument,arg);
	smash_tilde(arg);
	smash_tilde(argument);

	if (arg[0] == '\0')
	{
		if (rch->pcdata->alias[0] == NULL)
		{
			ch->println("You have no aliases defined.");
			return;
		}
		ch->printlnbw( "Your current aliases are:" );
		
		for (pos = 0; pos < MAX_ALIAS; pos++)
		{
			if (rch->pcdata->alias[pos] == NULL
				||	rch->pcdata->alias_sub[pos] == NULL)
				break;
			sprintf(buf,"    %s:  %s",rch->pcdata->alias[pos],
				rch->pcdata->alias_sub[pos]);
			ch->printlnbw(buf);
		}
		return;
    }

	if(!str_cmp(arg,"\"")){
		ch->println("sorry, you can't realias the double quote (\") character.");
		return;
	}

	if (!str_prefix("una",arg) || !str_cmp("alias",arg))
	{
		ch->println("Sorry, that word is reserved.");
		return;
	}

	if (argument[0] == '\0')
	{
		for (pos = 0; pos < MAX_ALIAS; pos++)
		{
			if (rch->pcdata->alias[pos] == NULL
				||	rch->pcdata->alias_sub[pos] == NULL)
				break;
			if (!str_cmp(arg,rch->pcdata->alias[pos]))
			{
				sprintf(buf,"%s aliases to '%s'.",
					rch->pcdata->alias[pos],
					rch->pcdata->alias_sub[pos]);
				ch->printlnbw(buf);
				return;
			}
		}
		ch->println("That alias is not defined.");
		return;
	}
	
	if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))
	{
		ch->println("That shall not be done!");
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
		if (rch->pcdata->alias[pos] == NULL)
			break;
		if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
		{
			free_string(rch->pcdata->alias_sub[pos]);
			rch->pcdata->alias_sub[pos] = str_dup(argument);
			sprintf(buf,"%s is now re-aliased to '%s'.",arg,argument);
			ch->printlnbw(buf);
			return;
		}
	}

	if (pos >= MAX_ALIAS)
	{
		ch->println("Sorry, you have reached the alias limit.");
		return;
	}
	
	// make a new alias
	rch->pcdata->alias[pos]		= str_dup(arg);
	rch->pcdata->alias_sub[pos]	= str_dup(argument);
	ch->printlnf( "%s is now aliased to '%s'.",	arg, argument);
}

/***************************************************************************/
void do_unalias(char_data *ch, char *argument)
{
	char_data *rch;
	char arg[MIL];
	int pos;
	bool found = false;
    
	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument,arg);
	
	if (IS_NULLSTR(arg))
	{
		ch->println("Unalias what?");
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
		if (rch->pcdata->alias[pos] == NULL)
			break;
		if (found)
		{
			rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
			rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
			rch->pcdata->alias[pos]			= NULL;
			rch->pcdata->alias_sub[pos]		= NULL;
			continue;
		}
		
		if(!strcmp(arg,rch->pcdata->alias[pos]))
		{
			ch->println("Alias removed.");
			free_string(rch->pcdata->alias[pos]);
			free_string(rch->pcdata->alias_sub[pos]);
			rch->pcdata->alias[pos]			= NULL;
			rch->pcdata->alias_sub[pos]		= NULL;
			found = true;
		}
	}
	
	if (!found)
		ch->println("No alias of that name to remove.");
	return;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
