/**************************************************************************/
// flags.cpp - implementation of do_flag() - needs a rewrite
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

int flag_lookup args( ( const char *name, const struct flag_type *flag_table) );

/**************************************************************************/
void do_flag(char_data *ch, char *argument)
{
    char arg1[MIL],arg2[MIL],arg3[MIL];
    char word[MIL];
    char_data *victim;
    long *flag, old = 0, nw = 0, marked = 0, pos;
    char type;
    const struct flag_type *flag_table;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
    argument = one_argument(argument,arg3);

    type = argument[0];

    if (type == '=' || type == '-' || type == '+')
        argument = one_argument(argument,word);

    if (arg1[0] == '\0')
    {
	ch->println("Syntax:");
	ch->println("  flag mob  <name> <field> <flags>");
	ch->println("  flag char <name> <field> <flags>");
	ch->println("  flag obj  <name> <field> <flags>");
	ch->println("  flag room <room> <field> <flags>");
	ch->println("  mob  flags: act,aff,off,imm,res,vuln,form,part");
	ch->println("  char flags: plr,comm,aff,imm,res,vuln,thief,killer");
	ch->println("  obj  flags: extra,wear,weap,cont,gate,exit");
	ch->println("  room flags: room");
	ch->println("  +: add flag, -: remove flag, = set equal to");
	ch->println("  otherwise flag toggles the flags listed.");
	return;
    }

    if (arg2[0] == '\0')
    {
	ch->println("What do you wish to set flags on?");
	return;
    }

    if (arg3[0] == '\0')
    {
	ch->println("You need to specify a flag to set.");
	return;
    }

    if (argument[0] == '\0')
    {
	ch->println("Which flags do you wish to change?");
	return;
    }

    if (!str_prefix(arg1,"mob") || !str_prefix(arg1,"char"))
    {
	victim = get_char_world(ch,arg2);
	if (victim == NULL)
	{
	    ch->println("You can't find them.");
	    return;
	}

        /* select a flag to set */
	if (!str_prefix(arg3,"act"))
	{
	    if (!IS_NPC(victim))
	    {
		ch->println("Use plr for PCs.");
		return;
	    }

	    flag = &victim->act;
	    flag_table = act_flags;
	}

	else if (!str_prefix(arg3,"plr"))
	{
	    if (IS_NPC(victim))
	    {
		ch->println("Use act for NPCs.");
		return;
	    }

	    flag = &victim->act;
	    flag_table = plr_flags;
	}

 	else if (!str_prefix(arg3,"aff"))
	{
	    flag = &victim->affected_by;
	    flag_table = affect_flags;
	}

  	else if (!str_prefix(arg3,"immunity"))
	{
	    flag = &victim->imm_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"resist"))
	{
	    flag = &victim->res_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"vuln"))
	{
	    flag = &victim->vuln_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"form"))
	{
	    if (!IS_NPC(victim))
	    {
	 	ch->println("Form can't be set on PCs.");
		return;
	    }

	    flag = &victim->form;
	    flag_table = form_flags;
	}

	else if (!str_prefix(arg3,"parts"))
	{
	    if (!IS_NPC(victim))
	    {
		ch->println("Parts can't be set on PCs.");
		return;
	    }

	    flag = &victim->parts;
	    flag_table = part_flags;
	}

	else if (!str_prefix(arg3,"comm"))
	{
	    if (IS_NPC(victim))
	    {
		ch->println("Comm can't be set on NPCs.");
		return;
	    }

	    flag = &victim->comm;
	    flag_table = comm_flags;
	}

	else if (!str_prefix(arg3,"thief"))
	{
	    if(IS_NPC(victim)){
			ch->println("Not on NPCs.");
			return;
		}
		if(!IS_THIEF(victim)){
			ch->printf("%s is not currently marked as a thief.", PERS(victim, ch));
			return;
		}
		ch->printf("Thief status removed on %s.", PERS(victim, ch));
		victim->pcdata->thief_until=0;
		return;
	}

	else if (!str_prefix(arg3,"killer"))
	{
	    if(IS_NPC(victim)){
			ch->println("Not on NPCs.");
			return;
		}
		if(!IS_KILLER(victim)){
			ch->printf("%s is not currently marked as a killer.", PERS(victim, ch));
			return;
		}
		ch->printf("Killer status removed on %s.", PERS(victim, ch));
		victim->pcdata->killer_until=0;
		return;
	}
	else 
	{
	    ch->println("That's not an acceptable flag.");
	    return;
	}

	old = *flag;
	victim->zone = NULL;

	if (type != '=')
	    nw = old;

        /* mark the words */
        for (; ;)
        {
	    argument = one_argument(argument,word);

	    if (word[0] == '\0')
		break;

	    pos = flag_lookup(word,flag_table);
	    if (pos == 0)
	    {
		ch->println("That flag doesn't exist!");
		return;
	    }
	    else
		SET_BIT(marked,pos);
	}

	for (pos = 0; flag_table[pos].name != NULL; pos++)
	{
	    if (!flag_table[pos].settable && IS_SET(old,flag_table[pos].bit))
	    {
		SET_BIT(nw,flag_table[pos].bit);
		continue;
	    }

	    if (IS_SET(marked,flag_table[pos].bit))
	    {
		switch(type)
		{
		    case '=':
		    case '+':
			SET_BIT(nw,flag_table[pos].bit);
			break;
		    case '-':
			REMOVE_BIT(nw,flag_table[pos].bit);
			break;
		    default:
			if (IS_SET(nw,flag_table[pos].bit))
			    REMOVE_BIT(nw,flag_table[pos].bit);
			else
			    SET_BIT(nw,flag_table[pos].bit);
		}
	    }
	}
	*flag = nw;
	return;
    }
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
   
