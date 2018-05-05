/**************************************************************************/
// scan.cpp - Implementation of the scan command
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

char *const distance[6]=
{
	"right here.", "nearby to the %s.", "not far %s.",
	"off in the distance %s.", "off in the far distance %s.", 
	"far far away to the %s."
};

void scan_list( ROOM_INDEX_DATA *scan_room, char_data *ch, sh_int depth, sh_int door);
void scan_char( char_data *victim, char_data *ch, sh_int depth, sh_int door);

/**************************************************************************/
void do_new_scan(char_data *ch, char *argument, int dist, int blocked)
{
	char arg1[MIL];
	ROOM_INDEX_DATA *scan_room;
	EXIT_DATA *pExit;
	sh_int door, depth;
	
    if (dist>5)
        dist = 5;
	
	argument = one_argument(argument, arg1);
	
	if (arg1[0] == '\0')
	{
		act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
		ch->println( "`RLooking around you see:`x" );
		scan_list(ch->in_room, ch, 0, -1);
		
        for (door=0;door<MAX_DIR;door++)
		{
			if ((  pExit = ch->in_room->exit[door]) != NULL 
				&& ( !IS_SET( pExit->exit_info, EX_CLOSED )
				|| ( IS_SET(  pExit->exit_info, EX_CLOSED)
				&& !blocked	)))
				scan_list(pExit->u1.to_room, ch, 1, door);
		}
		return;
	}

	door = dir_lookup( arg1 );
	
    if ( door == -1 )
	{
		ch->println("Which way do you want to scan?");
		return;
	}
	
	act("You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR);
	act("$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM);
	
	scan_room = ch->in_room;
	
	for (depth = 1; depth <= dist; depth++)
	{
        if (scan_room
            && ((pExit = scan_room->exit[door]) != NULL)
            && (   !IS_SET(pExit->exit_info, EX_CLOSED)
			|| (IS_SET(pExit->exit_info, EX_CLOSED) && !blocked)))
        {
			scan_room = pExit->u1.to_room;
			scan_list(pExit->u1.to_room, ch, depth, door);
        }
        else
            continue;
	}
	return;
}

/**************************************************************************/
void do_scan(char_data *ch, char *argument)
{
	if(IS_RIDING(ch)){
	    do_new_scan(ch, argument, 4, true);
	}else{
		do_new_scan(ch, argument, 3, true);
	}
}

/**************************************************************************/
void do_far_scan(char_data *ch, char *argument)
{
    do_new_scan(ch, argument, 5, false);
}

/**************************************************************************/
void scan_list(ROOM_INDEX_DATA *scan_room, char_data *ch, sh_int depth, sh_int door)
{
	char_data *rch;
	
	if (scan_room == NULL)
		return;

	if ( IS_SET( scan_room->room_flags, ROOM_NOSCAN ))
		return;

	if ( door != -1 && scan_room->exit[rev_dir[door]] != NULL
		&& IS_SET(scan_room->exit[rev_dir[door]]->exit_info,EX_CLOSED) ){
		return;
	}

	for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room)
	{
		if (rch == ch)
			continue;
		if (!IS_NPC(rch) && INVIS_LEVEL(rch)> get_trust(ch))
			continue;
		if (can_see(ch, rch))
			scan_char(rch, ch, depth, door);
	}
	return;
}

/**************************************************************************/
void scan_char(char_data *victim, char_data *ch, sh_int depth, sh_int door)
{
	char buf[MIL], buf2[MIL];

	buf[0] = '\0';
	strcat(buf, PERS(victim, ch));
	strcat(buf, ", ");
	sprintf(buf2, distance[depth], dir_name[door]);
	strcat(buf, buf2);
	ch->println(buf);
	return;
}
/**************************************************************************/
