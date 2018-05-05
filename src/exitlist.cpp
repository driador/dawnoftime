/**************************************************************************/
// exitlist.cpp - see below for additional copyright info etc
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
// The original version of this file originated from Erwin S. Andreasen '96

#include "include.h"
DECLARE_DO_FUN(	do_help );

/* To have VLIST show more than vnum 0 - 9900, change the number below: */ 

//#define MAX_SHOW_VNUM   333 
#define MAX_SHOW_VNUM   ((game_settings->olc_max_vnum)/100)

#define NUL '\0'


extern ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH]; /* db.c */

/* opposite directions */
const sh_int opposite_dir [10] = 
	{ DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP, 
	 DIR_SOUTHWEST, DIR_NORTHWEST, DIR_NORTHEAST, DIR_SOUTHEAST};



typedef enum {exit_from, exit_to, exit_both} exit_status;

/**************************************************************************/
char * room_door_codes(ROOM_INDEX_DATA* room, ROOM_INDEX_DATA* to, char * col)
{
	static char buf[MSL];

	int door;
	char doors[MSL];
	char hdoors[MSL]; // hidden doors
	bool hfound;
	EXIT_DATA *pexit;


	// do the exits for room
	doors[0] = '\0';
	for (door = 0; door < MAX_DIR; door++)
	{
		if ((pexit = room->exit[door]) != NULL
		&&  pexit->u1.to_room != NULL
		&&  !IS_SET(pexit->exit_info,EX_CLOSED))
		{
			if (pexit->u1.to_room==to)
			{
				strcat(doors, col);
				strcat(doors,dir_shortname[door]);
			}
			else
			{
				strcat(doors,CC"B");
				strcat(doors,dir_shortname[door]);
			}
		}
	}
	// hidden exits 
	hfound = false;
	hdoors[0] = '\0';
	strcat(hdoors,"`S(");

	for (door = 0; door < MAX_DIR; door++)
	{
		if ((pexit = room->exit[door]) != NULL
		&&  pexit->u1.to_room != NULL
		&&  IS_SET(pexit->exit_info,EX_CLOSED))
		{
			hfound = true;
			if (pexit->u1.to_room==to)
			{
				strcat(hdoors,col);
				strcat(hdoors,dir_shortname[door]);
			}
			else
			{
				strcat(hdoors,CC"B");
				strcat(hdoors,dir_shortname[door]);
			}
		}
	}

	// clean up
	strcat(hdoors,"`S)");
	if (hfound){
		strcat(doors,hdoors);
	}
	strcat(doors,CC"x");

	strcpy(buf, doors);

	return buf;
}

/**************************************************************************/
/* depending on status print > or < or <> between the 2 rooms */
void room_pair (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex, char *buffer)
{
	char *sExit;
	char lbuf[MSL], rbuf[MSL];
	char col[4];

	col[0]='\0';
	switch (ex)
	{
		default:
			sExit = "??"; break; // invalid usage 
		case exit_from:
			sExit = ">>"; 
			sprintf(col,"`R");
			break;
		case exit_to:
			sExit = "<<"; 
			sprintf(col,"`Y");
			break;
		case exit_both:
			sExit = "==";
			sprintf(col,"`G");
			break;

	}

	{
		char dcodes[MSL];
		strcpy(dcodes, room_door_codes(left, right, col));
	
		sprintf(lbuf, CC"S[`r%s `B%s`S:%s%5d", 
				str_width(capitalize( left->name), 
				(25-c_str_len(dcodes)), true), 
				dcodes, col, left->vnum);

		strcpy(dcodes, room_door_codes(right, left, col));
		sprintf(rbuf, "%5d`S:`r%s `B%s`S]", right->vnum, 
				str_width(capitalize( right->name ), 
				(25-c_str_len(dcodes)), true), 
				dcodes);
	
		sprintf (buffer, "`x %-8.8s%s%s%s`x\r\n",
			  area_name(left->area),
			  lbuf,
			  sExit,
			  rbuf);
	}

}

/**************************************************************************/
/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
bool checkexits (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer)
{
	char buf[MSL];
	int i;
	EXIT_DATA *exit;
	ROOM_INDEX_DATA *to_room;
	bool found=false;
	
	strcpy (buffer, "");

	for (i = 0; i < MAX_DIR; i++)
	{
		exit = room->exit[i];
		if (!exit){
			continue;
		}
		to_room = exit->u1.to_room;
		
		if (to_room){ // there is something on the other side
			if ( (room->area == pArea) && (to_room->area != pArea) )
			{ // an exit from our area to another area
			  // check first if it is a two-way exit
			
				if ( to_room->exit[opposite_dir[i]] &&
					to_room->exit[opposite_dir[i]]->u1.to_room == room )
					room_pair (to_room,room,exit_both,buf); // <>
				else
					room_pair (to_room,room,exit_to,buf); // >
				
				strcat (buffer, buf);
				found = true;
			}			
			else if ( (room->area != pArea) && (exit->u1.to_room->area == pArea) )
			{ // an exit from another area to our area

				if  (!
			    	 (to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->u1.to_room == room )
					)
				// two-way exits are handled in the other if
				{						
					room_pair (room,to_room,exit_from,buf);
					strcat (buffer, buf);
					found = true;
				}
				
			} // if room->area 
		}

			
	} // for

	return found;
}

/**************************************************************************/
/* for now, no arguments, just list the current area */
void do_exitlist (char_data *ch, char * )
{
	AREA_DATA* pArea;
	ROOM_INDEX_DATA* room;
	int i;
	char buf[MSL], buf2[MSL], buf3[MSL];
	int num_of_rooms=0;
	int num_portal=0;

    BUFFER *output;
    BUFFER *portalbuf;
    BUFFER *final;
    output= new_buf();
	portalbuf= new_buf();
    final= new_buf();
    
	pArea = ch->in_room->area; // this is the area we want info on
	for (i = 0; i < MAX_KEY_HASH; i++) // room index hash table
	{
		for (room = room_index_hash[i]; room != NULL; room = room->next)
		// run through all the rooms on the MUD		
		{
			if (checkexits (room, pArea, buf))
			{
				num_of_rooms++;
				add_buf(output,buf);
			}
		}
	}

	sprintf(buf2, "EXITS LIST: %d room%s found", num_of_rooms, (num_of_rooms==1?"":"s"));
	sprintf( buf3,"`S%s`x", format_titlebar(buf2));
		add_buf(final,buf3);
		add_buf(final,str_dup( buf_string(output)));

	{// do the portals leading in

    OBJ_INDEX_DATA *pObj;
	int vnum;
	int nMatch=0;
    for ( vnum = 0; vnum<game_settings->olc_max_vnum; vnum++ )
    {
		if ( ( pObj= get_obj_index( vnum ) ) != NULL )
		{
		    nMatch++;	
			if ( pObj->item_type ==ITEM_PORTAL )
			{
				ROOM_INDEX_DATA *pRoom;
				pRoom= get_room_index( pObj->value[3]);
				if (pRoom)
				{
					if (pRoom->area == pArea)
					{
						sprintf(buf, "`x %-8.8s`S[`r%s(%s)`x %5d `#`R>>>>`^ %5d`S]`x\r\n", 
							pObj->area ? area_name(pObj->area) : "`#`RNo Area!`^",
							str_width(capitalize(pObj->short_descr),25, true), 
							str_width(pObj->name,22,true), 						
							pObj->vnum, pObj->value[3]);
						add_buf(portalbuf,buf);
						num_portal++;						
					}
				}
			
			}
		}
    }
	}

	if (num_portal>0)
	{
		sprintf(buf2, "%2d portal%s found that lead into %s", 
			num_portal, (num_portal==1?"":"s"),
			area_name(pArea));
		sprintf( buf3,"\r\n`S%s`x", format_titlebar(buf2));
		add_buf(final,buf3);
		add_buf(final,str_dup( buf_string(portalbuf)));
	}

	num_portal=0;
	free_buf(portalbuf);
	portalbuf=new_buf();
	{// do the portals leading out
    OBJ_INDEX_DATA *pObj;
	int vnum;
	int nMatch=0;
    for ( vnum = 0; vnum<game_settings->olc_max_vnum; vnum++ )
    {
		if ( ( pObj= get_obj_index( vnum ) ) != NULL )
		{
		    nMatch++;	
			if ( pObj->item_type ==ITEM_PORTAL )
			{
				if (pObj->area == pArea)
				{
					sprintf(buf, "`x %-8.8s`S[`r%s(%s)`x %5d `#`R>>>>`^ %5d`S]`x\r\n", 
						pObj->area ? area_name(pObj->area) : "`#`RNo Area!`^",
						str_width(capitalize(pObj->short_descr),25,true), 
						str_width(pObj->name,22,true), 						
						pObj->vnum,pObj->value[3]);
					add_buf(portalbuf,buf);
					num_portal++;						
				}
			
			}
		}
    }
	}

	if (num_portal>0)
	{
		sprintf(buf2, "%2d portal%s found that lead out of %s", 
			num_portal, (num_portal==1?"":"s"),
			area_name(pArea));
		sprintf( buf3,"\r\n`S%s`x", format_titlebar(buf2));
		add_buf(final,buf3);
		add_buf(final,str_dup( buf_string(portalbuf)));
	}

	ch->sendpage(str_dup( buf_string(final)));
	free_buf(output);
	free_buf(portalbuf);
	free_buf(final);
} 

/* show a list of all used VNUMS */
int count_areas_in_range( int lower, int upper );
AREA_DATA *get_vnum_area( int vnum );

#define COLUMNS 		3   /* number of columns */
#define MAX_ROW 		((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */

/**************************************************************************/
void do_vnummap (char_data *ch, char *)
{
	int i,j,vnum;	
	static char *buffer; // permanently allocated buffer
	if(!buffer){	// should be plenty 
		buffer= new char[MAX_ROW*200];
		assertp(buffer);
	}

	char buf2[100];
	AREA_DATA *pArea;	
	
	BUFFER *output= new_buf();
	
	for (i = 0; i < MAX_ROW; i++)
	{
		strcpy (buffer, ""); // clear the buffer for this row 
		
		for (j = 0; j < COLUMNS; j++) // for each column 
		{
			vnum = ((j*MAX_ROW) + i); // find a vnum which should be there 
			if (vnum < MAX_SHOW_VNUM)
			{
				if(count_areas_in_range(vnum * 100, vnum * 100 + 49)){
					pArea=get_vnum_area(vnum * 100);
					sprintf (buf2, "`x%3d0%s%-8.8s ", vnum, 
						(pArea? (IS_SET(pArea->area_flags, AREA_OLCONLY)?"`G":"`W"): "`Y"), // olc colour: plain: can't find area
							 pArea? area_name(pArea) : "?" ); 
				}else{
					sprintf (buf2, "`x%3d0`B%-8.8s ", vnum, "-" ); 
				}
				strcat (buffer,buf2);				
				if(count_areas_in_range(vnum * 100+50, vnum * 100 + 99)){
					pArea=get_vnum_area(vnum * 100 + 50);
					sprintf (buf2, "`x%3d5%s%-8.8s ", vnum, 
						(pArea? (IS_SET(pArea->area_flags, AREA_OLCONLY)?"`G":"`W"): "`Y"), // olc colour: plain: can't find area
							 pArea? area_name(pArea) : "?" ); 
				}else{
					sprintf (buf2, "`x%3d5`B%-8.8s ", vnum, "-" ); 
				}
				strcat (buffer,buf2);				
			} 
		} // for columns 
		add_buf( output, buffer);
		add_buf( output, "`x\r\n");
	} // for rows 

    ch->sendpage(buf_string(output));
    free_buf(output);
}

/**************************************************************************/
/*
 * do_rename renames a player to another name.
 * PCs only. Previous file is deleted, if it exists.
 * Char is then saved to new file.
 * New name is checked against std. checks, existing offline players and
 * online players. 
 * .gz files are checked for too, just in case.
 */


/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containing
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/   
/**************************************************************************/
const char * name_expand (char_data *ch)
{
	int count = 1;
	char_data *rch;
	char name[MIL]; 
	// HOPEFULLY no mob has a name longer than THAT

	static char outbuf[MIL];	
	
	if (!IS_NPC(ch))
		return ch->name;
		
	one_argument (ch->name, name); /* copy the first word into name */
	
	if (!name[0]) /* weird mob .. no keywords */
	{
		strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
		return outbuf;
	}
		
	for (rch = ch->in_room->people; rch && (rch != ch);rch = rch->next_in_room)
		if (is_name (name, rch->name))
			count++;
			

	sprintf (outbuf, "%d.%s", count, name);
	return outbuf;
}


/**************************************************************************/
void do_for (char_data *ch, char *argument)
{
	char range[MIL];
	char buf[MSL];
	bool fGods = false, fMortals = false, fMobs = false, fEverywhere = false, found;
	ROOM_INDEX_DATA *room, *old_room;
	char_data *p, *p_next;
	int i;
	
	argument = one_argument (argument, range);
	
	if (!range[0] || !argument[0]) /* invalid usage? */
	{
		do_help (ch, "for");
		return;
	}
	
	if (!str_prefix("quit", argument))
	{
		ch->println("Are you trying to crash the MUD or something?");
		return;
	}
	
	
	if (!str_cmp (range, "all"))
	{
		fMortals = true;
		fGods = true;
	}
	else if (!str_cmp (range, "gods"))
		fGods = true;
	else if (!str_cmp (range, "mortals"))
		fMortals = true;
	else if (!str_cmp (range, "mobs"))
		fMobs = true;
	else if (!str_cmp (range, "everywhere"))
		fEverywhere = true;
	else
		do_help (ch, "for"); /* show syntax */

	/* do not allow # to make it easier */		
	if (fEverywhere && strchr (argument, '#'))
	{
		ch->println("Cannot use FOR EVERYWHERE with the # thingie.");
		return;
	}
		
	if (strchr (argument, '#')) /* replace # ? */
	{ 
		for (p = char_list; p ; p = p_next)
		{
			p_next = p->next; /* In case someone DOES try to AT MOBS SLAY # */
			found = false;
			
			if (!(p->in_room) || is_room_private_to_char(p->in_room, ch) || (p == ch))
				continue;
	
			if (IS_NPC(p) && fMobs)
				found = true;
			else if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
				found = true;
			else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
				found = true;

			/* It looks ugly to me.. but it works :) */				
			if (found) /* p is 'appropriate' */
			{
				char *pSource = argument; /* head of buffer to be parsed */
				char *pDest = buf; /* parse into this */
				
				while (*pSource)
				{
					if (*pSource == '#') /* Replace # with name of target */
					{
						const char *namebuf = name_expand (p);
						
						if (namebuf) /* in case there is no mob name ?? */
							while (*namebuf) /* copy name over */
								*(pDest++) = *(namebuf++);

						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				} /* while */
				*pDest = '\0'; /* Terminate */
				
				/* Execute */
				old_room = ch->in_room;
				char_from_room (ch);
				char_to_room (ch,p->in_room);
				interpret (ch, buf);
				char_from_room (ch);
				char_to_room (ch,old_room);
				
			} /* if found */
		} /* for every char */
	}
	else /* just for every room with the appropriate people in it */
	{
		for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
			for (room = room_index_hash[i] ; room ; room = room->next)
			{
				found = false;
				
				/* Anyone in here at all? */
				if (fEverywhere) /* Everywhere executes always */
					found = true;
				else if (!room->people) /* Skip it if room is empty */
					continue;
					
					
				/* Check if there is anyone here of the required type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				for (p = room->people; p && !found; p = p->next_in_room)
				{

					if (p == ch) /* do not execute on oneself */
						continue;
						
					if (IS_NPC(p) && fMobs)
						found = true;
					else if (!IS_NPC(p) && (p->level >= LEVEL_IMMORTAL) && fGods)
						found = true;
					else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)
						found = true;
				} /* for everyone inside the room */
						
				if (found && !is_room_private_to_char(room, ch)) /* Any of the required type here AND room not private? */
				{					
					/* This may be ineffective. Consider moving character out of old_room
					   once at beginning of command then moving back at the end.
					   This however, is more safe?
					*/
				
					old_room = ch->in_room;
					char_from_room (ch);
					char_to_room (ch, room);
					interpret (ch, argument);
					char_from_room (ch);
					char_to_room (ch, old_room);
				} /* if found */
			} /* for every room in a bucket */
	} /* if strchr */
} /* do_for */
/**************************************************************************/
