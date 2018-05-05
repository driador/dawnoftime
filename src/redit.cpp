/**************************************************************************/
// redit.cpp - OLC Based room editor
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

#include "olc.h"
#include "security.h"
#include "lockers.h"

DECLARE_OLC_FUN( redit_create );
/**************************************************************************/
extern gameset_value_type gameset_value[];

/**************************************************************************/
// True if room is private.
bool room_pc_private( ROOM_INDEX_DATA *pRoomIndex )
{
    char_data *rch;
    int count = 0;

    if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
		return true;

    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	{
		if ( !IS_NPC(rch)){
			count++;
		}
	}

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return true;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return true;
    
    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) && count >= 1 )
	return true;

    return false;
}

/**************************************************************************/
// Entry point for editing room_index_data. 
void do_redit( char_data *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom, *pRoom2;
    char arg1[MSL];
    char_data *rch;
	
	if (!HAS_SECURITY(ch,2))
	{
		ch->println( "You must have an olc security 2 or higher to use this command." );
		return;
	}

    argument = one_argument( argument, arg1 );

    pRoom = ch->in_room;

    if ( !str_cmp( arg1, "reset" ) )
    {
		if ( !IS_BUILDER( ch, pRoom->area, BUILDRESTRICT_ROOMS ) )
		{
			ch->println( "Insufficient security to modify the rooms of the area you are in." );
        		return;
		}

		reset_room( pRoom, true );
		ch->printlnf( "Room %d reset.", pRoom->vnum);
		ch->println( "note: you can use the rpurge command to purge the room of mobs/objects." );
		return;
    }
    else if ( !str_cmp( arg1, "create" ) || !str_cmp( arg1, "?" ) )
    {
		if ( argument[0] == '\0' || atoi( argument ) == 0 || !str_cmp( arg1, "?" ))
		{
			ch->println( "Syntax:  redit create [vnum]" );
			ch->println( " typing redit by itself will edit the room you are in." );
			return;
		}

		if ( redit_create( ch, argument ) )
		{
			for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
			{
				if (IS_IMMORTAL(rch))
				{
					act("($n leaves the room via REDIT)",ch,NULL,rch,TO_VICT);              
				}
			}
			char_from_room( ch );
			char_to_room( ch, (ROOM_INDEX_DATA *) ch->desc->pEdit );
			SET_BIT( pRoom->area->olc_flags, OLCAREA_CHANGED );
			pRoom = ch->in_room;
		}
    }
    else
    {
		// support 'redit .' syntax to edit the current room
		if(!IS_NULLSTR(arg1) && arg1[0]=='.' && arg1[1]=='\0'){
			arg1[0]='\0';
		}

		if(!IS_NULLSTR(arg1) && !is_number(arg1)){
			ch->printlnf("'%s' is not a number!", arg1);
			return;
		};

	    pRoom2 = get_room_index(atoi(arg1));  
		// make sure the room exists
		if (!pRoom2 && !IS_NULLSTR(arg1)){
		   ch->printlnf("Room %d does not exist.", atoi(arg1));
		   return;
		}

		// check for privacy - count only pc's for check
		if (pRoom2 && !IS_ADMIN(ch) 
			&& !is_room_owner(ch,pRoom2) && room_pc_private(pRoom2) )
		{
			ch->println( "That room is private right now (mobs ignored in olc private checks)." );
			return;
		}
		
		if ( (pRoom2 != NULL) && IS_BUILDER(ch,pRoom2->area, BUILDRESTRICT_ROOMS) )
	    {
			for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
			{
				if (IS_IMMORTAL(rch))
				{
					act("($n leaves the room via REDIT)",ch,NULL,rch,TO_VICT);              
				}
			}
	       char_from_room( ch );
	       char_to_room( ch, pRoom2 );
			for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
			{
				if (IS_IMMORTAL(rch))
				{
					act("($n enters the room via REDIT)",ch,NULL,rch,TO_VICT);              
				}
			}
	       pRoom = ch->in_room;
	    }else if (atoi(arg1) != 0){
           ch->println( "Insufficient security to edit the room with that vnum." );
	       return;
	    }   
    }

    if ( !IS_BUILDER( ch, pRoom->area, BUILDRESTRICT_ROOMS ) )
    {
		ch->println( "Insufficient security to edit the room you are currently in." );
       	return;
    }

	// officially reserved vnum range
	if(ch->in_room_vnum()<500){
		ch->println("Warning: all mobs, rooms and objects below vnum 500 are officially reserved for the dawn codebase.");
		if(!HAS_SECURITY(ch,9)){
			ch->println("As a result of this reservation, only those with security 9 can edit in that vnum range.");
			return;
		}			
	}

    ch->desc->editor = ED_ROOM;

	ch->wraplnf(	"`=rYou are now editing the room '`r%s`=r' vnum: `Y%d`x", 
				ch->in_room->name,
				ch->in_room->vnum);
	ch->println( "`=rType `=Cdone`=r to finish editing.");
	return;
}
/**************************************************************************/
bool redit_show( char_data *ch, char * )
{
    ROOM_INDEX_DATA	*pRoom;
    char		buf  [MSL];
    char		buf1 [2*MSL];
    OBJ_DATA		*obj;
    char_data		*rch;
    int			door;
    bool		fcnt;
    
    EDIT_ROOM(ch, pRoom);
	

    sprintf(buf, "`=rVnum: `=v%5d  `=rArea[%d]: `=x%-25s ", 
		pRoom->vnum, 
		pRoom->area? pRoom->area->vnum:0,
		pRoom->area? pRoom->area->name:"`=RNo Area!!!`x");
    
	if(IS_SET(ch->dyn,DYN_SHOWFLAGS)){
		ch->println( buf );
	}else{
		ch->print( buf );
	}

	mxp_display_olc_flags(ch, sector_types, pRoom->sector_type, "sector", "Sector Type:");
	
	ch->printlnf("`=rName: `r%s      `=rOwner: `=x%s   `=rClan: `=x%s`x",
			pRoom->name,
			IS_NULLSTR(pRoom->owner)?"`=$(none)":pRoom->owner,
			pRoom->clan ? pRoom->clan->m_pName : "`=$(none)" );
	
    ch->printf("`=rDescription:\r\n  `=R%s`x", pRoom->description );
	
    ch->printlnf("`=rHealth recovery: `=v%3d%%     `=rMana recovery: `=v%d%%",
		pRoom->heal_rate , pRoom->mana_rate );

	mxp_display_olc_flags(ch, room_flags, pRoom->room_flags, "room",	  "Room Flags: ");
	mxp_display_olc_flags(ch, room2_flags, pRoom->room2_flags, "room2",	  "Room2 Flags:");

	// start of characters in room
    fcnt = false;
    strcpy( buf1, FORMATF("`=rCharacters(%d): [`B", pRoom->number_in_room) );
    for ( rch = pRoom->people; rch; rch = rch->next_in_room )
    {
		one_argument( rch->name, buf );
		strcat( buf1, buf );
		strcat( buf1, " " );
		fcnt = true;
    }
    if ( fcnt )
    {
		buf1[str_len(buf1) - 1] = '\0';
        strcat( buf1, "`=r]\r\n" );
    }
    else
	{
        strcat( buf1, "none`=r]\r\n" );
	} // end of characters in room
	
	// start of objects in room
    strcat( buf1, "`=rObjects:    `=r[`g" );
    fcnt = false;
    for ( obj = pRoom->contents; obj; obj = obj->next_content )
    {
		one_argument( obj->name, buf );
		strcat( buf1, buf );
		strcat( buf1, " " );
		fcnt = true;
    }
    if ( fcnt )
    {
		buf1[str_len(buf1) - 1] = '\0';
        strcat( buf1, "`=r]\r\n" );
    }
    else
	{
        strcat( buf1, "none`=r]`x\r\n" );
	}
	// end of objects in room
	
    for ( door = 0; door < MAX_DIR; door++ )
    {
		EXIT_DATA *pexit;
		
		if ( ( pexit = pRoom->exit[door] ) )
		{
			char word[MIL];
			char reset_state[MSL];
			char *state;
			int i, length;
			
			sprintf( buf, "`=r-`Y%-5s `=rto [`Y%5d`=r] Key: [`x%5d`=r] ",
				capitalize(dir_name[door]),
				pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,
				pexit->key );
			strcat( buf1, buf );
			
			/*
			* Format up the exit info.
			* Capitalize all flags that are not part of the reset info.
			*/
			strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
			state = flag_string( exit_flags, pexit->exit_info );
			strcat( buf1, " Exit flags: [`x" );
			for (; ;)
			{
				state = one_argument( state, word );
				
				if ( word[0] == '\0' )
				{
					buf1[str_len(buf1) - 1] = '\0';
					strcat( buf1, "`=r]\r\n" );
					break;
				}
				
				if ( str_infix( word, reset_state ) )
				{
					length = str_len(word);
					for (i = 0; i < length; i++)
						word[i] = UPPER(word[i]);
				}
				strcat( buf1, word );
				strcat( buf1, " " );
			}
			
			if ( pexit->keyword && pexit->keyword[0] != '\0' )
			{
				sprintf( buf, "`=rKwds: [`x%s`=r]\r\n", pexit->keyword );
				strcat( buf1, buf );
			}
			if ( pexit->description && pexit->description[0] != '\0' )
			{
				sprintf( buf, "  `=R%s`x", pexit->description );
				strcat( buf1, buf );
			}
		}
    }
	
    ch->print( buf1 );
	
	ch->printlnf( "`=rMSP:       `x%s", 
		IS_NULLSTR(pRoom->msp_sound)?"`=Rnone`x":pRoom->msp_sound);

	ch->println( "`=rhint: type `=Cusedby`=r to see exits and portals the lead into this room.");

	if(lockers->room_has_lockers(pRoom)){
		ch->println("`=r---============== L O C K E R   D E T A I L S ============================---");
		ch->printlnf("`=rQuantity=`=v%d  `=@(%d in use)      `=rInitial rent=`=v%d `=rgold `=@(includes 1 month rent)\r\n"
			"`=rOngoing rent=`=v%d `=rgold per year  `=rWeight(v0)=`=v%d  `=rCapacity(v3)=`=v%d, `=rPickProof=`=v%d",
			pRoom->lockers->quantity, 
			lockers->count_used_lockers_in_room(pRoom->vnum),
			pRoom->lockers->initial_rent,
			pRoom->lockers->ongoing_rent,			
			pRoom->lockers->weight,
			pRoom->lockers->capacity,
			pRoom->lockers->pick_proof);
	}

	// display any room echos 
	ch->println("`=r---============== R O O M   E C H O E S ==================================---");
	int recount=0;
	for( room_echo_data *re = pRoom->echoes; re; re = re->next )
	{
		if(re==pRoom->echoes){
			ch->println("[##] >=Hr <=Hr   %  -==EchoText==-`x");
		}
		ch->printlnf(  "`=r[%2d] `=v%4s %4s %3d  `=x'%s'`x", recount++, 
			convert24hourto12hour(re->firsthour), convert24hourto12hour(re->lasthour), 
			re->percentage, re->echotext);
		if(has_colour(re->echotext)){ // display black and white if colour in echo
			ch->print("`S");
			ch->printlnfbw(  "[%2d] %4s %4s %3d  '%s'", recount-1, 
				convert24hourto12hour(re->firsthour), convert24hourto12hour(re->lasthour), 
				re->percentage, re->echotext);
			ch->print("`x");
		};
	}
	if(recount==0){
		ch->println("`=Rnone - add using 'addecho'.`x");
	}

	show_char_extended(ch, pRoom->extra_descr, false);
	
    return false;
}

/**************************************************************************/
bool change_exit( char_data *ch, char *argument, int door )
{
    ROOM_INDEX_DATA *pRoom;
	ROOM_INDEX_DATA *pToRoom;
	int rev;
    char command[MIL];
    char arg[MIL];
    int  value;
	
	// record the full argument incase exit flags are being specified
    char fullarg[MIL];
	strcpy(fullarg, argument);
	
	// get the first arg, check if it is a command
    argument = one_argument( argument, command );
    one_argument( argument, arg );
	
	// no arguments - normal move 
    if ( IS_NULLSTR(command))	
    {
		move_char( ch, door, true );                    
		return false;
    }
	
	// help
    if ( command[0] == '?' )
    {
		do_help( ch, "OLC-EXIT" );
		return false;
    }
	
	// editing commands
	EDIT_ROOM(ch, pRoom);
    if ( !str_cmp( command, "remove" ) || !str_cmp( command, "delete" ))
    {
		ROOM_INDEX_DATA *pToRoom;
		sh_int rev;
		
		if ( !pRoom->exit[door] )
		{
            ch->println( "There is no exit in that direction to remove." );
			return false;
		}
		
		/*
		* Remove ToRoom Exit if it leads to current room.
		*/
		rev = rev_dir[door];
		pToRoom = pRoom->exit[door]->u1.to_room;
		
		if ( pToRoom && pToRoom->exit[rev] && pToRoom->exit[rev]->u1.to_room==pRoom)
		{
			free_exit( pToRoom->exit[rev] );
			pToRoom->exit[rev] = NULL;
		}
		
		/*
		* Remove this exit.
		*/
		free_exit( pRoom->exit[door] );
		pRoom->exit[door] = NULL;
		
        ch->println( "Exit removed." );
		return true;
	}
	
    if ( !str_cmp( command, "link" ) )
    {
		EXIT_DATA *pExit;
		
		if ( arg[0] == '\0' || !is_number( arg ) )
		{
            ch->println( "Syntax:  [direction] link [vnum]" );
			return false;
		}
		
		value = atoi( arg );
		
		if ( !get_room_index( value ) )
		{
            ch->println( "REdit:  Cannot link to non-existant room." );
			return false;
		}
		
		if ( !IS_BUILDER( ch, get_room_index( value )->area, BUILDRESTRICT_EXITS ) )
		{
            ch->println( "REdit: Cannot link to that area." );
			return false;
		}
		
		if ( get_room_index( value )->exit[rev_dir[door]] )
		{
            ch->println( "REdit:  Remote side's exit already exists." );
			return false;
		}
		
		if ( !pRoom->exit[door] )
		{
			pRoom->exit[door] = new_exit();
		}
		
		pRoom->exit[door]->u1.to_room = get_room_index( value );

		
		pRoom                   = get_room_index( value );
		door                    = rev_dir[door];
		pExit                   = new_exit();
		pExit->u1.to_room       = ch->in_room;
		pRoom->exit[door]       = pExit;

		// Mark the area the reverse door went on as changed - Kal Feb 01
		if(pRoom->area){ 
			SET_BIT( pRoom->area->olc_flags, OLCAREA_CHANGED );
		}
		
        ch->println( "Two-way link established." );
		return true;
	}

	if (!str_cmp (command, "dig")) 
	{
		if(IS_NULLSTR(arg))
		{
			// find the first unused room vnum in the area
			int newvnum =  ch->in_room->area->min_vnum-1; 
			pToRoom=ch->in_room; // make sure the while loop actually starts
			while(pToRoom){
				newvnum++;
				if (newvnum >  ch->in_room->area->max_vnum){ 
					ch->printlnf("Dig Error: No more free room vnums in area range %d to %d.",
						ch->in_room->area->min_vnum,
						ch->in_room->area->max_vnum); 
					return false; 
				}
				pToRoom = get_room_index(newvnum);
			}

			// create a room with that vnum, and link it in 
			redit_create(ch, FORMATF("%d", newvnum));         
			change_exit(ch, FORMATF("link %d", newvnum), door); 
			ch->printlnf("Created new room %d %s", newvnum, dir_name[door]);
			return true;        
		} 
		else 
		{ 
			if (!is_number( arg ) ){ 
				ch->println( "Syntax: <direction> dig [vnum]" ); 
				return false; 
			} 

			redit_create( ch, arg ); 
			change_exit( ch, FORMATF("link %s", arg ), door); 
			return true; 
		} 
	}
	
    if ( !str_cmp( command, "room" ) )
    {
		if ( arg[0] == '\0' || !is_number( arg ) )
		{
            ch->println( "Syntax:  [direction] room [vnum]" );
			return false;
		}
		
		if ( !pRoom->exit[door] )
		{
			pRoom->exit[door] = new_exit();
		}
		
		value = atoi( arg );
		
		if ( !get_room_index( value ) )
		{
            ch->println( "REdit:  Cannot link to non-existant room." );
			return false;
		}
		
		pRoom->exit[door]->u1.to_room = get_room_index( value );
		
        ch->println( "One-way link established." );
		return true;
	}
	
	/*** KEYS ***/
	if ( !str_cmp( command, "key" )  || !str_cmp( command, "onesidedkey" ))
	{
		OBJ_INDEX_DATA *pObj;
		
		if ( arg[0] == '\0' || !is_number( arg ) )
		{
            ch->println( "Syntax:  <direction> key <vnum>" );
			return false;
		}
		
		if ( !pRoom->exit[door] )
		{
            ch->println( "That exit doesn't exist." );
			return false;
		}
		
		value = atoi( arg );
		
		pObj=get_obj_index( value );
		if ( !pObj )
		{
            ch->println( "REdit:  Item doesn't exist." );
			return false;
		}
		
		ch->printlnf( "`=rUsing object Vnum: `x%-5d   `=rType: "
            "`x%-10s as key (from %s areafile).",
			pObj->vnum, flag_string( item_types, pObj->item_type ), 
			pObj->area ? pObj->area->file_name : "`RNo Area!!!`x");
		
		
		if(!IS_SET(pRoom->exit[door]->rs_flags, EX_ISDOOR)){
			ch->println( "`YAutomatic:`x setting `GDOOR`x flag `GON`x for the exit.");
			change_exit( ch, "door", door );
		}
		if(!IS_SET(pRoom->exit[door]->rs_flags, EX_CLOSED)){
			ch->println( "`YAutomatic:`x setting `GDOOR`x to `GCLOSED`x.");
			change_exit( ch, "closed", door );
		}
		
		if ( pObj->item_type != ITEM_KEY )
		{
            ch->println( "`RWarning:`x That object is not of type key." );
		}
		
		
		// do reverse side
		pToRoom = pRoom->exit[door]->u1.to_room;
		rev = rev_dir[door];
		
		if (pToRoom->exit[rev]){
			if (!str_cmp( command, "key" ))
			{
				if(pToRoom->exit[rev]->u1.to_room==pRoom){
					ch->printf("`GKey vnum on other side automatically set to %d,`x\r\n"
						"`Snote: Use 'onesidedkey' to not set the key on the other side.`x\r\n", value); 
					pToRoom->exit[rev]->key=value;
					// Mark the area the reverse door went on as changed - Kal Feb 01
					if(pToRoom->area){ 
						SET_BIT( pToRoom->area->olc_flags, OLCAREA_CHANGED );
					}
				}else{
					ch->printf("`RWarning:`x The exit going %s from %d (here) to %d doesn't link 2 ways.\r\n"
						"i.e. The exit going %s from %d doesn't go to %d...\r\n"
						"Therefore the key vnum not been set on that side.\r\n",
						dir_name[door], pRoom->vnum, pToRoom->vnum, 
						dir_name[rev], pToRoom->vnum, pRoom->vnum);
				}
			}
			else
			{
				ch->println("`Ronesidedkey used... this side of the door key only affected.`x");
			}
		}else{
			ch->println( "`RWarning:`x That exit is a oneway exit." );
		}
        ch->printlnf( "Exit key changed from %d to %d.", pRoom->exit[door]->key, value);
		pRoom->exit[door]->key = value;
		return true;
	}
	
    if ( !str_cmp( command, "name" ) )
    {
		if ( arg[0] == '\0' )
		{
            ch->println( "Syntax:  <direction> name <string>" );
            ch->println( "         <direction> name none" );
			return false;
		}
		
		if ( !pRoom->exit[door] )
		{
            ch->println( "That exit doesn't exist." );
			return false;
		}
		
		if (str_cmp(arg,"none")){
	        ch->printlnf( "Exit name changed from '%s' to '%s'.",
				pRoom->exit[door]->keyword, arg);
			replace_string(pRoom->exit[door]->keyword, arg );
		}else{
	        ch->printlnf( "Exit name cleared (was '%s').",
				pRoom->exit[door]->keyword);
			replace_string(pRoom->exit[door]->keyword, "" );
		}
		return true;
    }
	
    if ( !str_prefix( command, "description" ) )
    {
		if ( arg[0] == '\0' )
		{
			if ( !pRoom->exit[door] )
			{
				ch->println( "That exit doesn't exist." );
				return false;
			}
			
			if(ch->desc && pRoom->area){ 
				// if the string as changed, this is used to flag the area file saving
				ch->desc->changed_flag=&pRoom->area->olc_flags;
			}
			string_append( ch, &pRoom->exit[door]->description );
			return true;
		}
		
        ch->println( "Syntax:  [direction] desc" );
		return false;
    }
	
	
    // Set the exit flags, needs full argument.
    // ----------------------------------------    
    if ( ( value = flag_value( exit_flags, fullarg) ) != NO_FLAG )
    {
		
		if ( !pRoom->exit[door] )
		{
            ch->println( "Exit doesn't exit." );
			return false;
		}
		
		// This room.
		TOGGLE_BIT(pRoom->exit[door]->rs_flags, value);
		
		// Don't toggle exit_info because it can be changed by players.
		pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;
		
		if(value==EX_ONEWAY){
			if(IS_SET(pRoom->exit[door]->rs_flags, EX_ONEWAY)){
				ch->println( "Oneway exit flag set" );
			}else{
				ch->println( "Oneway exit flag removed" );
			}
		}else{
			if (pRoom->exit[door]->rs_flags && 
				!IS_SET(pRoom->exit[door]->rs_flags , EX_ISDOOR) )
			{
				SET_BIT(pRoom->exit[door]->rs_flags , EX_ISDOOR);
				ch->println( "Door flag added on this side of door." );
				if (IS_SET(value, EX_ISDOOR))
					ch->println( "note: to remove door exit flag, remove all other exit flags first." );
			}
			
			/*
			* Connected room.
			*/
			pToRoom = pRoom->exit[door]->u1.to_room;
			rev = rev_dir[door];
			
			if(pToRoom->exit[rev] && pToRoom->exit[rev]->u1.to_room==pRoom){
				if (IS_SET(pRoom->exit[door]->rs_flags,  value)){
					SET_BIT(pToRoom->exit[rev]->rs_flags,  value);
				}else{
					REMOVE_BIT(pToRoom->exit[rev]->rs_flags,  value);
				}
				pToRoom->exit[rev]->exit_info = pToRoom->exit[rev]->rs_flags;

				if (pToRoom->exit[rev]->rs_flags && 
					!IS_SET(pToRoom->exit[rev]->rs_flags , EX_ISDOOR) )
				{
					SET_BIT(pToRoom->exit[rev]->rs_flags , EX_ISDOOR);
					ch->println( "Door flag added on OTHER side of door." );
					if (IS_SET(value, EX_ISDOOR))
						ch->println( "note: to remove door exit flag, remove all other exit flags first." );
				}
				// Mark the area the reverse door went on as changed - Kal Feb 01
				if(pToRoom->area){ 
					SET_BIT( pToRoom->area->olc_flags, OLCAREA_CHANGED );
				}
				ch->println( "Exit flag toggled this side, set/removed on otherside." );
			}else{
				ch->printf("`RWarning:`x The exit going %s from %d (here) to %d doesn't link 2 ways.\r\n"
					"i.e. The exit going %s from %d doesn't go to %d...\r\n"
					"Therefore the exit flags have not been set on that side.\r\n",
					dir_name[door], pRoom->vnum, pToRoom->vnum, 
					dir_name[rev], pToRoom->vnum, pRoom->vnum);
			}			
		}
		return true;
    }
    ch->wrapln("Unrecognised exit command/exit flags, read help `=_OLC-EXIT for "
		"more help on olc exits and olc exit flags.");
	ch->wraplnf("Exit flags include: %s", flag_string(exit_flags, -1));
    return false;
}

/**************************************************************************/
// direction commands section to deal with exits
bool redit_north( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_NORTH ) )
	return true;

    return false;
}
bool redit_south( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_SOUTH ) )
	return true;

    return false;
}
bool redit_east( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_EAST ) )
	return true;

    return false;
}
bool redit_west( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_WEST ) )
	return true;

    return false;
}
bool redit_up( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_UP ) )
	return true;

    return false;
}
bool redit_down( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_DOWN ) )
        return true;

    return false;
}
bool redit_northeast( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_NORTHEAST ) )
        return true;

    return false;
}
bool redit_southeast( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_SOUTHEAST ) )
        return true;

    return false;
}
bool redit_southwest( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_SOUTHWEST ) )
        return true;

    return false;
}
bool redit_northwest( char_data *ch, char * argument)
{
    if ( change_exit( ch, argument, DIR_NORTHWEST ) )
        return true;

    return false;
}

/**************************************************************************/
bool redit_ed( char_data *ch, char * argument)
{
	return( generic_ed (ch, argument) );
}

/**************************************************************************/
bool redit_create( char_data *ch, char * argument)
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;
    
    EDIT_ROOM(ch, pRoom);
	
    value = atoi( argument );
	
    if ( argument[0] == '\0' || value <= 0 )
    {
        ch->println( "Syntax:  create [vnum > 0]" );
		return false;
    }
	
    pArea = get_vnum_area( value );
    if ( !pArea )
    {
        ch->println( "REdit:  That vnum is not assigned an area." );
		return false;
    }
	
    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_ROOMS))
    {
        ch->println( "REdit: Vnum in an area you cannot build in." );
		return false;
    }
	
    if ( get_room_index( value ) )
    {
        ch->println( "REdit:  Room vnum already exists." );
		return false;
    }
	
	// officially reserved vnum range
	if(value<500){
		ch->println("Warning: all mobs, rooms and objects below vnum 500 are officially reserved for the dawn codebase.");
		if(!HAS_SECURITY(ch,9)){
			ch->println("As a result of this reservation, only those with security 9 can edit in that vnum range.");
			return false;
		}			
	}

    pRoom					= new_room_index();
    pRoom->area				= pArea;
    pRoom->vnum				= value;
	
    if ( value > top_vnum_room )
        top_vnum_room = value;
	
    iHash					= value % MAX_KEY_HASH;
    pRoom->next				= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoom;
    ch->desc->pEdit			= (void *)pRoom;

	ch->printlnf( "Room %d created.", pRoom->vnum);
    return true;
}

/**************************************************************************/
bool redit_name( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  name <string>" );
		return false;
    }

	ch->printlnf( "Room name changed from '%s' to '%s'.", pRoom->name, argument);
    replace_string( pRoom->name, argument);
    return true;
}

/**************************************************************************/
bool redit_desc( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( IS_NULLSTR(argument) )
    {
		if(ch->desc && pRoom->area){ 
			// if the string as changed, this is used to flag the area file saving
			ch->desc->changed_flag=&pRoom->area->olc_flags;
		}
		string_append( ch, &pRoom->description );
		return true;
    }

    ch->println( "Syntax:  desc" );
    return false;
}

/**************************************************************************/
bool redit_heal( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);

    if(is_number(argument)){
		ch->printlnf( "Healing HP regen rate changed from %d to %d.", 
			pRoom->heal_rate, atoi( argument ));
		pRoom->heal_rate= atoi ( argument );
		return true;
	}
    
    ch->println( "Syntax : heal <#xnumber>" );
    return false;
}       

/**************************************************************************/
bool redit_mana( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if(is_number(argument)){
		ch->printlnf( "Mana regen rate changed from %d to %d.", 
			pRoom->mana_rate, atoi( argument ));
		pRoom->mana_rate = atoi ( argument );
		return true;
	}

    ch->println( "Syntax : mana <#xnumber>" );
    return false;
}       

/**************************************************************************/
void do_clanlist(char_data *ch, char *);
/**************************************************************************/
bool redit_clan( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);


	if(IS_NULLSTR(argument)){
		ch->println("Syntax: clan <clanname>");
		ch->println("Syntax: clan none");
		do_clanlist(ch,"");
		return false;
	}

    if(!str_cmp(argument, "none")){
		ch->printlnf("Clan cleared - was '%s`x'", pRoom->clan?pRoom->clan->cname():"none already");
		pRoom->clan=NULL;
		return true;
    }

	CClanType* newclan=clan_lookup(argument);

	if(!newclan){
		ch->printlnf("There is no clan called '%s'", argument);
		return false;
	}
	    
	if(!pRoom->clan){
		ch->printlnf("Room clan set to '%s'", newclan->cname());
	}else{
		if(newclan==pRoom->clan){
			ch->printlnf("This room is already owned by the '%s`x' clan.", 
				newclan->cname());
			return false;
		}
		ch->printlnf("Room clan changed from '%s`x' to '%s`x'", 
			pRoom->clan->cname(),
			newclan->cname());
	}
    pRoom->clan = newclan;
    
    return true;
}
      
/**************************************************************************/
bool redit_wrap( char_data *ch, char * )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    pRoom->description = format_string( pRoom->description );

    ch->println( "Room description wordwrapped." );
    return true;
}

/**************************************************************************/
bool redit_mreset( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA	*pRoom;
    MOB_INDEX_DATA	*pMobIndex;
    char_data		*newmob;
    char			arg [ MIL ];
    char			arg2 [ MIL ];
    RESET_DATA		*pReset;

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
		ch->println( "Syntax:  mreset <vnum> <max #x> <mix #x>" );
		return false;
    }

    if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
		ch->println( "REdit: No mobile has that vnum." );
		return false;
    }

    if ( pMobIndex->area != pRoom->area )
    {
		ch->println( "REdit: No such mobile in this area." );
		return false;
    }

    /*
     * Create the mobile reset.
     */
    pReset              = new_reset_data();
    pReset->command		= 'M';
    pReset->arg1		= pMobIndex->vnum;
    pReset->arg2		= is_number( arg2 ) ? atoi( arg2 ) : MAX_MOB;
    pReset->arg3		= pRoom->vnum;
    pReset->arg4		= is_number( argument ) ? atoi (argument) : 1;
    add_reset( pRoom, pReset, 0/* Last slot*/ );

    /*
     * Create the mobile.
     */
    newmob = create_mobile( pMobIndex, 0 );
    char_to_room( newmob, pRoom );

    ch->printf("%s (%d) has been loaded and added to resets.\r\n"
		"There will be a maximum of %d loaded to this room.\r\n",
		capitalize( pMobIndex->short_descr ),
		pMobIndex->vnum,
		pReset->arg2 );
    act( "$n has created $N!", ch, NULL, newmob, TO_ROOM );
    return true;
}

/**************************************************************************/
struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};
/**************************************************************************/
const struct wear_type wear_table[] =
{
    {	WEAR_NONE,			OBJWEAR_TAKE		},
    {	WEAR_LIGHT,			ITEM_LIGHT			},
    {	WEAR_FINGER_L,		OBJWEAR_FINGER		},
    {	WEAR_FINGER_R,		OBJWEAR_FINGER		},
    {	WEAR_NECK_1,		OBJWEAR_NECK		},
    {	WEAR_NECK_2,		OBJWEAR_NECK		},
    {	WEAR_TORSO,			OBJWEAR_TORSO		},
    {	WEAR_HEAD,			OBJWEAR_HEAD		},
    {	WEAR_LEGS,			OBJWEAR_LEGS		},
    {	WEAR_FEET,			OBJWEAR_FEET		},
    {	WEAR_HANDS,			OBJWEAR_HANDS		},
    {	WEAR_ARMS,			OBJWEAR_ARMS		},
    {	WEAR_SHIELD,		OBJWEAR_SHIELD		},
    {	WEAR_ABOUT,			OBJWEAR_ABOUT		},
    {	WEAR_WAIST,			OBJWEAR_WAIST		},
    {	WEAR_WRIST_L,		OBJWEAR_WRIST		},
    {	WEAR_WRIST_R,		OBJWEAR_WRIST		},
    {	WEAR_WIELD,			OBJWEAR_WIELD		},
    {	WEAR_HOLD,			OBJWEAR_HOLD		},
	{	WEAR_LODGED_ARM,	OBJWEAR_LODGED_ARM	},
	{	WEAR_LODGED_LEG,	OBJWEAR_LODGED_LEG	},
	{	WEAR_LODGED_RIB,	OBJWEAR_LODGED_RIB	},
	{	WEAR_SHEATHED,		OBJWEAR_SHEATHED	},
	{	WEAR_CONCEALED,		OBJWEAR_CONCEALED	},
	{	WEAR_EYES,			OBJWEAR_EYES		},
	{	WEAR_EAR_L,			OBJWEAR_EAR			}, 
	{	WEAR_EAR_R,			OBJWEAR_EAR			}, 
	{	WEAR_FACE,			OBJWEAR_FACE		},
	{   WEAR_ANKLE_L,		OBJWEAR_ANKLE		}, 
	{   WEAR_ANKLE_R,		OBJWEAR_ANKLE		}, 
 	{	WEAR_BACK,			OBJWEAR_BACK		},
    {	NO_FLAG,			NO_FLAG				}
};




/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }
 
    return 0;
}
/**************************************************************************/
bool redit_oreset( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*newobj;
    OBJ_DATA		*to_obj;
    char_data		*to_mob;
    char			vnum[MIL];
    char			name[MIL];
    int				olevel = 0;
    RESET_DATA		*pReset;
	
    EDIT_ROOM(ch, pRoom);
	
    argument = one_argument( argument, vnum);
    argument = one_argument( argument, name);
	
    if ( IS_NULLSTR(vnum) || !is_number( vnum ) )
    {
		ch->println( "Syntax:  oreset <vnum>                      = reset object into room" );
		ch->println( "Syntax:  oreset <vnum> <objname>            = reset object into object" );
		ch->println( "Syntax:  oreset <vnum> <mobname> <wearloc>  = reset object onto mob" );
		return false;
    }
	
    if ( !( pObjIndex = get_obj_index( atoi( vnum ) ) ) )
    {
		ch->printlnf( "redit_oreset(): No object has vnum '%s'.", vnum);
		return false;
    }
	
    if ( pObjIndex->area != pRoom->area )
    {
		ch->printlnf( "redit_oreset(): Object vnum '%s' doesn't belong to this area.", vnum );
		return false;
    }
	    
    if(IS_NULLSTR(name)){ // no name, load into room.
		pReset			= new_reset_data();
		pReset->command	= 'O';
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= 0;
		pReset->arg3	= pRoom->vnum;
		pReset->arg4	= 0;
		add_reset( pRoom, pReset, 0/* Last slot*/ );
		
		newobj = create_object( pObjIndex);
		obj_to_room( newobj, pRoom );
		
		ch->printlnf("%s (%d) has been loaded and added to resets.",
			capitalize( pObjIndex->short_descr ),
			pObjIndex->vnum );

		act( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
		return true;
    }
	
	
	// name, but no wear location - therefore object inside object
	if(IS_NULLSTR(argument)){
		to_obj = get_obj_list( ch, name, pRoom->contents );

		if(!to_obj){
			ch->printlnf("redit_oreset(): Couldn't find any object '%s' currently in the room.", name);
			return false;
		}
		pReset		= new_reset_data();
		pReset->command	= 'P';
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= 0;
		pReset->arg3	= to_obj->pIndexData->vnum;
		pReset->arg4	= 1;
		add_reset( pRoom, pReset, 0);
		
		newobj = create_object( pObjIndex);
		obj_to_obj( newobj, to_obj );
		newobj->cost = 0;
		
		ch->printf("%s (%d) has been loaded into "
			"%s (%d) and added to resets.\r\n",
			capitalize( newobj->short_descr ),
			newobj->pIndexData->vnum,
			to_obj->short_descr,
			to_obj->pIndexData->vnum );

		ch->printlnf("%s (%d) has been loaded and added to resets.",
			capitalize( pObjIndex->short_descr ),
			pObjIndex->vnum );

		act( "$n has created $p and put it inside another object!", ch, newobj, NULL, TO_ROOM );
		return true;
    }
	
	// because the object hasnt been loaded into the room or another object 
	// the reset must be loading the object onto a mob
	// format of the command oreset <vnum> <mobname> <wearlocation>
	to_mob = get_char_room( ch, name);
	if(!to_mob){
		ch->printlnf("redit_oreset(): Couldn't find any mobile '%s' currently in the room to load the object onto.", name);
		return false;
	}

	int	wear_loc= flag_value( wear_location_types, argument );
	if(wear_loc==NO_FLAG){
		ch->printlnf("redit_oreset(): Invalid wear_loc '%s'", argument);
		ch->println("Valid wear locations include:");
		show_olc_flags_types(ch, wear_location_types);
		ch->printlnf("%s (%d) has wear flags: [%s]",
			capitalize( pObjIndex->short_descr ),
			pObjIndex->vnum,
			flag_string( wear_flags, pObjIndex->wear_flags ) );
		return false;
	}
		
	// Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
	if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wear_loc) ) )
	{
		ch->printlnf("redit_oreset(): %s (%d) has wear flags: [%s]",
			capitalize( pObjIndex->short_descr ),
			pObjIndex->vnum,
			flag_string( wear_flags, pObjIndex->wear_flags ) );
		return false;
	}
		
	// Can't load into same position.
	if ( get_eq_char( to_mob, wear_loc ) )
	{
		ch->printlnf( "redit_oreset():  The '%s' wear location is already in use.", argument);
		return false;
	}
		
	{ // create the object onto mobile reset
		pReset			= new_reset_data();
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= wear_loc;
		if ( pReset->arg2 == WEAR_NONE ){
			pReset->command = 'G';
		}else{
			pReset->command = 'E';
		}
		pReset->arg3	= wear_loc;
		
		add_reset( pRoom, pReset, 0);
		
		olevel  = URANGE( 0, to_mob->level - 2, LEVEL_HERO );

		if ( to_mob->pIndexData->pShop )	// Shop-keeper? 
		{
			switch ( pObjIndex->item_type )
			{
			default:			olevel = 0;							break;
			case ITEM_PILL:		olevel = number_range(  0, 10 );	break;
			case ITEM_POTION:	olevel = number_range(  0, 10 );	break;
			case ITEM_SCROLL:	olevel = number_range(  5, 15 );	break;
			case ITEM_WAND:		olevel = number_range( 10, 20 );	break;
			case ITEM_STAFF:	olevel = number_range( 15, 25 );	break;
			case ITEM_ARMOR:	olevel = number_range(  5, 15 );	break;
			case ITEM_WEAPON:
				if ( pReset->command == 'G' ){
					olevel = number_range( 5, 15 );
				}else{
					olevel = number_fuzzy( olevel );
				}
				break;
			}
			
			if ( pReset->arg2 == WEAR_NONE ){
				ch->println("Automatically adding the extra 'inventory' flag to object due to the 'none' wear location onto a shopkeeper.");
				SET_BIT( pObjIndex->extra_flags, OBJEXTRA_INVENTORY );
			}
		}
        newobj = create_object( pObjIndex);
		obj_to_char( newobj, to_mob );
		
		if ( pReset->command == 'E' ){
			equip_char( to_mob, newobj, pReset->arg3 );
		}
		
	}

	ch->printlnf("%s (%d) has been loaded %s of %s (%d) and added to resets.",
		capitalize( pObjIndex->short_descr ),
		pObjIndex->vnum,
		flag_string( wear_location_strings_types, pReset->arg3 ),
		to_mob->short_descr,
		to_mob->pIndexData->vnum );
	
    act( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
    return true;
}
/**************************************************************************/
// duplicate the rooms data from another 
// written by Kalahn - June 98
bool redit_copy(char_data *ch, char *argument)
{
	ROOM_INDEX_DATA *pRoom;
	ROOM_INDEX_DATA *pSrc; // source room
	char arg1[MIL];
	int value;

    argument = one_argument( argument, arg1 );

    if ( !is_number( arg1 ) )
    {
        ch->println( "Syntax: rcopy <source room vnum>" );
        ch->println( "  - copies the source room over the room you are currently editing!" );
        ch->println( "    Apart from exits, room specials and resets at this stage." );
        ch->println( "    (warning copies over everything else!)" );
        return false;
	}

    value = atoi( arg1 );
    if ( !( pSrc = get_room_index(  value ) ) )
    {
        ch->println( "REdit_copy:  The source vnum does not exist." );
        return false;
    }
    
    if ( !IS_BUILDER( ch, pSrc->area, BUILDRESTRICT_ROOMS ) && !IS_IMMORTAL(ch))
    {
        ch->println( "Insufficient security to copy rooms from the area that room is stored in." );
        return false;
    }

    

	// needs to deal with ROOM_SPEC_DATA 
	// - whole spec data system needs to be supported by olc
	// - doesn't copy resets
//	ROOM_SPEC_DATA   *     spec_data;
//	RESET_DATA *   reset_first;
//	RESET_DATA *   reset_last;

	EDIT_ROOM(ch, pRoom);

	// copy the object details
	pRoom->name			= str_dup(pSrc->name);
	pRoom->description	= str_dup(pSrc->description);
	pRoom->owner		= str_dup(pSrc->owner);

	pRoom->room_flags	= pSrc->room_flags;
	pRoom->sector_type	= pSrc->sector_type;
	pRoom->heal_rate	= pSrc->heal_rate;
	pRoom->mana_rate	= pSrc->mana_rate;
	pRoom->clan			= pSrc->clan;

	// COPY THE EXTENDED DESCRIPTIONS 
	pRoom->extra_descr	= dup_extdescr_list(pSrc->extra_descr);

	ch->wraplnf( "`=rCopied room '%s'[%d] to vnum %d", 
				pSrc->name, pSrc->vnum, pRoom->vnum);
    return true;
}
/**************************************************************************/
bool redit_owner( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if(IS_NULLSTR(argument)){
		ch->println( "Syntax:  owner [owner]" );
		ch->println( "         owner none" );
		return false;
    }

    if (!str_cmp(argument, "none")){
		ch->printlnf("Owner cleared - was '%s'", pRoom->owner);
    	replace_string(pRoom->owner, "");
    }else{
		ch->printlnf("Owner changed from '%s' to '%s'", 
			pRoom->owner, argument);
		replace_string(pRoom->owner, argument );
	}
    return true;
}
/**************************************************************************/
/**************************************************************************/
bool redit_rlist( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA		*pArea;
    char		buf  [ MSL   ];
    BUFFER		*buf1;
    char		arg  [ MIL    ];
    bool found;
    int vnum;
    int  col = 0;

	if (!HAS_SECURITY(ch,1))
	{
		ch->println("The olist command is an olc command, you don't have olc permissions." );
		return false;
	}

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_ROOMS ) && IS_IMMORTAL(ch))
    {
        ch->println( "rlist: Invalid security for listing rooms in this area." );
		return false;
    }

    one_argument( argument, arg );

    pArea = ch->in_room->area;
    buf1=new_buf();
/*    buf1[0] = '\0'; */
    found   = false;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoomIndex = get_room_index( vnum ) ) )
	{
		found = true;
		sprintf( buf, CC"x[%5d] %-17.16s",
		    vnum, capitalize( pRoomIndex->name ) );
		add_buf( buf1, buf );
		if ( ++col % 3 == 0 )
            add_buf( buf1, "\r\n" );
	}
    }

    if ( !found )
    {
        ch->println( "Room(s) not found in this area." );
		return false;
    }

    if ( col % 3 != 0 )
    add_buf( buf1, "\r\n" );

    ch->sendpage(buf_string(buf1));
    free_buf(buf1);
    return false;
}

/**************************************************************************/
DECLARE_DO_FUN( do_mlist	);
/**************************************************************************/
bool redit_mlist( char_data *ch, char * argument)
{
	do_mlist(ch, argument);
    return false;
}

/**************************************************************************/
DECLARE_DO_FUN( do_olist	);
/**************************************************************************/
bool redit_olist( char_data *ch, char * argument)
{
	do_olist(ch, argument);
    return false;
}

/**************************************************************************/
bool redit_room( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olc_generic_flag_toggle(ch, argument, "room", "room", room_flags, &pRoom->room_flags);
}
/**************************************************************************/
bool redit_room2( char_data *ch, char * argument)
{
    ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olc_generic_flag_toggle(ch, argument, "room2", "room2", room2_flags, &pRoom->room2_flags);
}

/**************************************************************************/
bool redit_sector( char_data *ch, char * argument)
{
	int  value;
    ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);

	if(IS_NULLSTR(argument)){
		show_olc_options(ch, sector_types, "sector", "sector", pRoom->sector_type);
		return false;
	}

    if ( ( value = flag_value( sector_types, argument) ) != NO_FLAG )
    {
        pRoom->sector_type  = value;
        ch->println( "Sector type set." );
        return true;
    }
	return false;
}
/**************************************************************************/
bool redit_msp_roomsound(char_data *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoom;
	
	EDIT_ROOM(ch, pRoom);

	if ( IS_NULLSTR( argument )) {
		ch->println("Syntax:  `=Cmspsound [string]`x");
		ch->println("         `=Cmspsound -`x clears the value\r\n"); 
		ch->println(" Associates the .wav or .mid [string] to appropriate skill/spell.");
		ch->println(" Will only accept existing filenames residing in the msp dir.");
		return false;
	}

	free_string( pRoom->msp_sound );

	if(strcmp(argument,"-"))
	{
		if ( file_existsf( MSP_DIR"%s", argument ))
		{
			pRoom->msp_sound = str_dup( argument );
		    ch->println("MSP Sound set successfully.");
			return true;
		} else {
			ch->println("MSP file not found.");
			return false;
		}
	}else{
		pRoom->msp_sound = NULL;
	    ch->println("MSP Sound cleared.");
	}
    return true;
}
/**************************************************************************/
void do_aroomlist(char_data *ch, char *argument )
{	
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA		*pArea;
	char			arg0[MIL];
	char			arg1[MIL];
	int				vlowrange = 0;
	int				vhighrange = 0;
	int				numrooms = 0;
    int				vnum;
	bool			args = false;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The aroomlist command is an olc command, you don't have olc permissions.");
		return;
	}

	argument = one_argument( argument, arg0 );
    argument = one_argument( argument, arg1 );

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_ROOMS ) && !IS_IMMORTAL(ch))
    {
		ch->println("aroomlist: Invalid security for viewing rooms in this area.");
		return;
    }

    pArea = ch->in_room->area;

	// Argument and dummy checks

	if ( !IS_NULLSTR( arg0 ) && !IS_NULLSTR( arg1 ))
	{
		vlowrange  = atoi( arg0 );
		vhighrange = atoi( arg1 );

		args = true;

		if ( vlowrange < pArea->min_vnum || vlowrange > pArea->max_vnum ) {
			ch->printlnf("aroomlist: Vnum value %d isn't within this area.", vlowrange );
			return;
		} else if ( vhighrange < pArea->min_vnum || vhighrange > pArea->max_vnum ) {
			ch->printlnf("aroomlist: Vnum value %d isn't within this area.", vhighrange );
			return;
		} else if ( vlowrange > vhighrange ) {
			ch->println("aroomlist: Vnum 1 must be less than Vnum 2.");
			return;
		}
	}

	if ( vlowrange  < 1 ) vlowrange  = pArea->min_vnum;
	if ( vhighrange < 1 )
		vhighrange = (UMIN( pArea->max_vnum, ( vlowrange + 100 ))-1);

	ch->printf("`x"//
		"             L N I     S   N       N   N   N A N N N\r\n"
		"           D I O N P S L   O     H E   O   O N O O O\r\n"
		"           A G M D R A T P R I I E W L W O S T S F S\r\n"
		"           R H O R I F R E C M M R B A H O C I P L C\r\n"
		" VNUMS     K T B S V E Y T L P M O I W R C R M K Y N  SECTOR    NAME\r\n" );

    for ( vnum = vlowrange; vnum <= vhighrange; vnum++ )
    {
		if ( ( pRoomIndex = get_room_index( vnum ) ) )
		{
			ch->printf("  %-5d    ", pRoomIndex->vnum);

			ch->printlnf("%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %-7s  %-10s",
				IS_SET( pRoomIndex->room_flags, ROOM_DARK				) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_LIGHT				) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_NO_MOB				) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_INDOORS			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_SAFE				) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_PET_SHOP			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_IMP_ONLY			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_GODS_ONLY			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_HEROES_ONLY		) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_NEWBIES_ONLY		) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_LAW 				) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_NOWHERE			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_OOC				) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_NOSCRY				) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_ANTIMAGIC			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_NOSPEAK			) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_NOFLY				) ? "x" : " " ,
				IS_SET( pRoomIndex->room_flags, ROOM_NOSCAN				) ? "x" : " " ,
				flag_string( sector_types, pRoomIndex->sector_type ),
				!IS_NULLSTR( pRoomIndex->name) ? pRoomIndex->name : "(UNNAMED)");
			numrooms++;
		}
	}
	ch->printlnf("\r\nTotal rooms: %d", numrooms );
	if ( !args && numrooms == 100 )
		ch->println("`ROnly the first 100 vnums were shown.\r\nTry using aroomlist <vnum1> <vnum2> to see a better range...");

    return;
}

/**************************************************************************/
void do_setrooms(char_data *ch, char *argument )
{	
    ROOM_INDEX_DATA	*pRoom = NULL;
    AREA_DATA		*pArea;
	bool			adding = false, sector = false;
	bool			heal = false, mana = false;
	char			arg0[MIL];	// 0 cause I added it last and didn't want to change things :)
	char			arg1[MIL];
	char			arg2[MIL];
	char			arg3[MIL];
	int				value;
	int				vnum;
    int				lowvnum;
	int				highvnum;

	argument = one_argument( argument, arg0 );
    argument = one_argument( argument, arg1 );

	// if arg1=='.' replace it with the vnum of the current room,
	// and set arg2 to the same vnum
	if(!str_cmp(arg1,".")){
		sprintf(arg1, "%d", ch->in_room->vnum);
		strcpy(arg2,arg1);
	}else{
		argument = one_argument( argument, arg2 );
	}
    argument = one_argument( argument, arg3 );
	
	if (!HAS_SECURITY(ch,1)){
		ch->println( "The setrooms command is an olc command, you don't have olc permissions." );
		return;
	}
	
    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_ROOMS ) && !IS_IMMORTAL(ch))
    {
		ch->println( "setrooms: Invalid security for setting rooms in this area." );
		return;
    }
	
	if (   IS_NULLSTR( arg0 )
		|| IS_NULLSTR( arg1 )
		|| IS_NULLSTR( arg2 )
		|| IS_NULLSTR( arg3 ))
	{
		ch->println( "Syntax: setrooms add <vnum1> <vnum2> <roomflag>" );
		ch->println( "Syntax: setrooms add2 <vnum1> <vnum2> <roomflag2>" );
		
		ch->println( "Syntax: setrooms remove <vnum1> <vnum2> <roomflag>" );
		ch->println( "Syntax: setrooms remove2 <vnum1> <vnum2> <roomflag2>" );

		ch->println( "Syntax: setrooms sector <vnum1> <vnum2> <value>" );
		ch->println( "Syntax: setrooms heal <vnum1> <vnum2> <value>" );
		ch->println( "Syntax: setrooms mana <vnum1> <vnum2> <value>" );
		ch->wrapln( "Note: '<vnum1> <vnum2>' can be replaced with a single . to only do the changes to the current room." );

		ch->println( "e.g. setrooms add . <roomflag>" );	

		return;
	}
	
    pArea		= ch->in_room->area;
	lowvnum		= atoi( arg1 );
	highvnum	= atoi( arg2 );

	// determine if we are adding or removing


	bool flag2=false;
	if ( !str_cmp( arg0, "add" ) || !str_cmp( arg0, "add2" )) {
		if(!str_cmp( arg0, "add2" )){
			flag2=true;
		}
		adding= true;
	} else if ( !str_cmp( arg0, "remove" ) || !str_cmp( arg0, "remove2" )) {
		if(!str_cmp( arg0, "remove2" )){
			flag2=true;
		}
		adding= false;
	} else if ( !str_cmp( arg0, "sector" )) {
		sector = true; 
	} else if ( !str_cmp( arg0, "heal" )) {
		heal = true;
	} else if ( !str_cmp( arg0, "mana" )) {
		mana = true;
	} else {
		ch->println( "Syntax: setrooms <`=Cadd`x/`=Cremove`x/`=Csector`x/`=Cheal`x/`=Cmana`x> <vnum1> <vnum2> <value>" );
		ch->println( "        use add or remove for roomflags." );
		return;
	}

	// Filter out bad vnum1 and vnum2 ranges
	if ( lowvnum > highvnum )
	{
		ch->println("`RError:`x vnum1 must be lower than or equal to vnum2.");
		return;
	}
	
	if ( lowvnum < pArea->min_vnum )
	{
		ch->println("`RError:`x vnum1 is lower than the lowest vnum of this area.");
		ch->printlnf("   vnum1 must be at least %d, '%s' is not.", pArea->min_vnum, arg1 );
		return;
	}
	
	if ( highvnum > pArea->max_vnum )
	{
		ch->println("`RError:`x vnum2 is higher than the highest vnum of this area.");
		ch->printlnf("   vnum2 must be at lower than or equal to %d, '%s' is not.", pArea->max_vnum, arg1);
		return;
	}

	// see if arg0 is sector, if it is value must be a valid sector type
	if ( sector ) {
		if (( value = sector_lookup( arg3 )) < 0 ) {
			ch->printlnf( "`RError:`x %s is not a valid sector type.", arg3 );
			return;
		}
	} else if ( mana || heal ) {
		value = atoi( arg3 );
	} else {
		value = flag_value( flag2?room2_flags:room_flags, arg3 );
		// Check if arg3 is a valid room flag
		if ( value == NO_FLAG )
		{
			ch->printlnf("`RError:`x %s in not a valid room%s flag.", arg3, flag2?"2":"" );
			return;
		}
	}
		
	for ( vnum = lowvnum; vnum <= highvnum; vnum++ )
	{
		pRoom = get_room_index( vnum );

		if(!pRoom){
			continue;
		}
	
		if ( sector ) {
			pRoom->sector_type = value;
			SET_BIT( pRoom->area->olc_flags, OLCAREA_CHANGED );
			continue;
		}

		if ( heal ) {
			pRoom->heal_rate = value;
			SET_BIT( pRoom->area->olc_flags, OLCAREA_CHANGED );
			continue;
		}

		if ( mana ) {
			pRoom->mana_rate = value;
			SET_BIT( pRoom->area->olc_flags, OLCAREA_CHANGED );
			continue;
		}

		if ( adding ){
			SET_BIT( flag2?pRoom->room2_flags:pRoom->room_flags, value );
			SET_BIT( pRoom->area->olc_flags, OLCAREA_CHANGED );
		}else{
			REMOVE_BIT( flag2?pRoom->room2_flags:pRoom->room_flags, value );
			SET_BIT( pRoom->area->olc_flags, OLCAREA_CHANGED );
		}
	}
	
	ch->println( "Done." );
	return;
}
/**************************************************************************/
// Kal
bool redit_addecho( char_data *ch, char * argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println( "Syntax:  addecho <firsthour> <secondhour> <percentage> echotext" );
		ch->println( "For the echotext to be displayed to all in the room the following must be met.");
		ch->println( "IC time hour is inbetween <firsthour> and <secondhour> (inclusive)." );
		ch->println( "A random number between 1 and 100 must be less than or equal to <percentage>" );
		ch->println( "e.g. addecho 19 22 10 a owl hoots in the distance.");
		ch->println( "     (will be displayed from 7pm to 10pm 10% of the time on the tick)" );
		ch->println( "e.g. addecho 7 5 3 a bird sings. ");
		ch->println( "     (will be displayed from 7am thru to 5am the next day 3% of the time on the tick)" );
		return false;
    }
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch, pRoom);

	char low[MIL];
	char high[MIL];
	char percentage[MIL];
	argument=one_argument(argument, low);
	argument=one_argument(argument, high);
	argument=one_argument(argument, percentage);

	if(IS_NULLSTR(argument)){
		redit_addecho(ch,"");
		return false;
	}
	if(!is_number(low) || !is_number(high) || !is_number(percentage)){
		ch->println("<firsthour> <lasthour> and <percentage> must all be numbers");
		redit_addecho(ch,"");
		return false;
	}
	int ilow=atoi(low);
	int ihigh=atoi(high);
	int ipercentage=atoi(percentage);

	if(ilow<0 || ilow>23 || ihigh<0 || ihigh>23){
		ch->println("<firsthour> and <lasthour> must be between 0 and 23 (0=midnight, 23= 11pm)");
		redit_addecho(ch,"");
		return false;
	}

	if(ipercentage<1 ||ipercentage>100){
		ch->println("<percentage> must be between 1 and 100.");
		redit_addecho(ch,"");
		return false;
	}

	// allocate our new echo
	room_echo_data *re=new_room_echo();
	replace_string(re->echotext, argument);
	re->firsthour=ilow;
	re->lasthour=ihigh;
	re->percentage=ipercentage;
	
	// add to the list of echos
	re->next=pRoom->echoes;
	pRoom->echoes=re;

    ch->printlnf( "RoomEcho %d(%s) %d(%s) %d%% '%s' added.", 
		re->firsthour, convert24hourto12hour(re->firsthour), 
		re->lasthour, convert24hourto12hour(re->lasthour),
		re->percentage, re->echotext);
	ch->println( "Use `=Cdelecho`x to remove room echoes");
    return true;
}
/**************************************************************************/
// Kal
bool redit_delecho(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println( "Syntax:  delecho <echo#>" );
		ch->println( "Removes a room echo listed in olc's redit show."); 
		ch->println( "<echo#> is the number of the echo in the list." );
		return false;
    }

    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch, pRoom);
	char num[MIL];

    one_argument( argument, num);

    if (IS_NULLSTR(num) || !is_number( num))
    {
		ch->println("The single parameter must be a number");
		redit_delecho(ch, "");					
		return false;
    }	
    
	int value = atoi(num);
	
    if(value < 0){
        ch->println("Only positive room echo numbers are valid .");
        return false;
    }
	
    if( !pRoom->echoes){
        ch->println("This room doesn't have any echos to remove.");
        return false;
    }
	
	// remove the room echo
    room_echo_data *list;
    room_echo_data *list_next;
	int cnt = 0;

	list = pRoom->echoes;
    if ( value == 0 ){
        pRoom->echoes= list->next;
        free_room_echo( list );
    }else{
        while ( (list_next = list->next) && (++cnt < value ) )
			list = list_next;		
        if(list_next)
        {
			list->next = list_next->next;
			free_room_echo(list_next);
        }
        else
        {
			ch->printlnf("No room echo number %d.", value);
			return false;
        }
    }

    ch->printlnf("Room echo %d removed", value);
    return true;
}
/**************************************************************************/
// show which rooms/objects link to this room - Kal
bool redit_usedby( char_data *ch, char * )
{
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch, pRoom);
	int i;

	ch->titlebarf("ROOMS WITH EXITS LEADING INTO ROOM %d", pRoom->vnum);	
	for(i=0; i<MAX_KEY_HASH; i++){
		for(ROOM_INDEX_DATA *r= room_index_hash[i]; r; r= r->next )
		{
			for ( int door = 0; door < MAX_DIR; door++ )
			{
				if ( r->exit[door] && r->exit[door]->u1.to_room==pRoom)
				{
					ch->printlnf("`x  Room %5d leads %s into %d (from area '%s%s`x')", 
						r->vnum,
						dir_name[door],
						pRoom->vnum,
						colour_table[(r->area->vnum%14)+1].code,
						r->area->name);
				}
			}
		}
	}
	
	ch->titlebarf("PORTALS LEADING INTO ROOM %d", pRoom->vnum);	
		 
	for(i=0; i<MAX_KEY_HASH; i++){
		for(OBJ_INDEX_DATA *o= obj_index_hash[i]; o; o= o->next )
		{
			if(o->item_type==ITEM_PORTAL && o->value[3]==pRoom->vnum){
				ch->printlnf("  Object %5d is a portal which leads to %d (from area '%s%s`x')", 
					o->vnum,
					pRoom->vnum,
					get_vnum_area(o->vnum)?colour_table[(get_vnum_area(o->vnum)->vnum%14)+1].code:"",
					get_vnum_area(o->vnum)?get_vnum_area(o->vnum)->name:"unknown");
			}
		}
	}
	ch->titlebar("");
	return false;
}
/**************************************************************************/
bool redit_delete( char_data *ch, char *)
{
	ch->println("If you want to delete a room, use the 'rdelete' command");
	return false;
}

/**************************************************************************/
// redit room delete code - Kal, Jan 2001
bool redit_rdelete( char_data *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
	ROOM_INDEX_DATA *pDeleteRoom;
    EDIT_ROOM(ch, pRoom);
	int no_delete_count=0;
	int door;
	int i;

	if(IS_NULLSTR(argument)){
		ch->titlebar("RDELETE SYNTAX");
		ch->println("rdelete confirm  - delete the current room");
		ch->println("rdelete <number> confirm - delete room vnum <number>");
		ch->println("Any room that you delete must meet the following conditions:");
		ch->println("* Must be in the same area that you are current editing.");
		ch->println("* Must have no rooms leading into it from outside the current area.");
		ch->println("* Must not have any portals leading into the room.");
		ch->println("* Must not have any lockers.");
		ch->println("* No players are in the room you are deleting (except you).");
		ch->println("* Room must not be a pet shop container room (previous room having pet_shop flag).");
		ch->println("* None of the game setting vnums make use of the room.");		
		ch->println("If you are in the room when you delete it... you will be moved to another room in the area.");
        ch->wrapln("NOTE: It is strongly recommended that you check no mudprogs transfer into the room "
			"you are wanting to remove... the easiest method to do this is 'textsearch mudprog <roomvnum>'.");
		return false;
	}

	// support specifying the room by number
	char arg1[MIL];
	argument=one_argument(argument, arg1);
	if(is_number(arg1)){
		pDeleteRoom=get_room_index(atoi(arg1));
		if(!pDeleteRoom){
			ch->printlnf("redit_rdelete(): There is no room number %s to delete.", arg1);
			return false;
		}
		if(pDeleteRoom->area!=pRoom->area){
			ch->printlnf("redit_rdelete(): Room %s (%s) is not in the same area as the room you are currently editing.", 
				arg1, pDeleteRoom->name);
			return false;
		}				
		argument=one_argument(argument, arg1); // put the word 'confirm' into arg1
	}else{
		pDeleteRoom=pRoom; // deleting the room we are currently in
	}

	// security check
	if ( !IS_BUILDER( ch, pDeleteRoom->area, BUILDRESTRICT_ROOMS) ){
		ch->printlnf( "Insufficient security to delete room %d.", pDeleteRoom->vnum );
		return false;
	}

	// confirm they are using 'confirm'
	if(str_cmp(arg1, "confirm")){
		ch->println("You must confirm your intention to delete a room.");
		redit_rdelete(ch,"");
		return false;
	}

	if(!IS_NULLSTR(ltrim_string(argument))){
		ch->println("Incorrect syntax - too many arguments, or arguments in wrong order.");
		redit_rdelete(ch, "");
		return false;
	}

	// We have the room they are wanting to delete and they have 
	// confirmed they want to delete it, check they are allowed.

//		ch->println("* Must have no rooms leading into it from outside the current area.");
	for(i=0; i<MAX_KEY_HASH; i++){
		for(ROOM_INDEX_DATA *r= room_index_hash[i]; r; r= r->next )
		{
			for ( door = 0; door < MAX_DIR; door++ )
			{
				if ( r->exit[door] && r->exit[door]->u1.to_room==pRoom && r->area!=pRoom->area)
				{
					ch->printlnf("`x  Room %5d leads %s into %d (from area '%s%s`x')", 
						r->vnum,
						dir_name[door],
						pRoom->vnum,
						colour_table[(r->area->vnum%14)+1].code,
						r->area->name);
					no_delete_count++;
				}
			}
		}
	}

//		ch->println("* Must not have any portals leading into the room.");
	for(i=0; i<MAX_KEY_HASH; i++){
		for(OBJ_INDEX_DATA *o= obj_index_hash[i]; o; o= o->next )
		{
			if(o->item_type==ITEM_PORTAL && o->value[3]==pRoom->vnum){
				ch->printlnf("  Object %5d is a portal which leads to %d (from area '%s%s`x')", 
					o->vnum,
					pRoom->vnum,
					get_vnum_area(o->vnum)?colour_table[(get_vnum_area(o->vnum)->vnum%14)+1].code:"",
					get_vnum_area(o->vnum)?get_vnum_area(o->vnum)->name:"unknown");
				no_delete_count++;
			}
		}
	}

//	ch->println("* No players are in the room you are deleting.");
	for(char_data *p=pDeleteRoom->people; p; p=p->next_in_room){
		if(!IS_NPC(p) && p!=ch){
			ch->printlnf("%s is in room %d", 
				PERS(p, ch),
				pDeleteRoom->vnum);
			no_delete_count++;
		}
	}

//	ch->println("* Must not have any lockers.");
	if(pDeleteRoom->lockers){
		ch->printlnf("room %d has some lockers assigned.", pDeleteRoom->vnum);
		no_delete_count++;
	}


//	ch->println("* Room must not be a pet shop container room (previous room having pet_shop flag).");
	{
		int petshop_vnum=pDeleteRoom->vnum-1;
		if(get_room_index(petshop_vnum) 
			&& IS_SET(get_room_index(petshop_vnum)->room_flags, ROOM_PET_SHOP)){
			ch->printlnf("Previous room (%d - %s) is a pet shop!", 
				petshop_vnum, get_room_index(petshop_vnum)->name);
			no_delete_count++;
		}
	}


//	ch->println("* None of the game setting vnums make use of the room.");
	for(i=0; !IS_NULLSTR(gameset_value[i].name); i++){
		if(gameset_value[i].category!=GSVC_ROOM){
			continue;
		}
		// get our numeric value
		int value=GSINT(gameset_value[i].offset);

		if(value==pDeleteRoom->vnum){
			ch->printlnf("Game setting value '%s (%s)' makes use of room %d.", 
				gameset_value[i].name, gameset_value[i].description, value);
			no_delete_count++;
		}

	}

	if(no_delete_count){
		ch->printlnf("The reason%s listed above prevent%s room %d from being deleted.",
			(no_delete_count==1?"":"s"), 
			(no_delete_count==1?"s":""), 
			pDeleteRoom->vnum);
		return false;
	}

	// *** delete the room
	
	// first move them out of the room if necessary
	if(pRoom==pDeleteRoom){
		// - look for an exit within the area
		for ( door = 0; door < MAX_DIR; door++ )
		{
			if ( pRoom->exit[door] 
				&& pRoom->exit[door]->u1.to_room!=pRoom 
				&& pRoom->exit[door]->u1.to_room->area==pRoom->area)
			{
				ch->printlnf("Moving you %s to room %d", 
					dir_name[door], 
					pRoom->exit[door]->u1.to_room->vnum);
				char_from_room(ch);
				char_to_room(ch, pRoom->exit[door]->u1.to_room);
				EDIT_ROOM(ch, pRoom);
				break;
			}
		}
		// find any room in the area to move them to
		if(pRoom==pDeleteRoom){
			for(i=pRoom->area->min_vnum; i<=pRoom->area->max_vnum; i++){
				if(pRoom->vnum!=i && get_room_index(i)){
					ch->printlnf("Moving you to room %d (in same area)", i);
					char_from_room(ch);
					char_to_room(ch, get_room_index(i));
					EDIT_ROOM(ch, pRoom);
					break;
				}
			}
		}
		// move them to limbo if all else fails
		if(pRoom==pDeleteRoom){
			ch->printlnf("Moving you to room %d (LIMBO) - couldn't find anywhere else to move you to!", 
				ROOM_VNUM_LIMBO);
			char_from_room(ch);
			char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
			EDIT_ROOM(ch, pRoom);
			edit_done(ch);
		}
	}


	// player definately out of pDeleteRoom, deallocate all its contents.
	{
		char_data *vnext, *victim;
		OBJ_DATA  *obj, *obj_next;
		if(pDeleteRoom->people){
			ch->println("Purging room mobiles...");
		}
		for ( victim = pDeleteRoom->people; victim; victim = vnext )
		{
			vnext = victim->next_in_room;
			extract_char( victim, true );
		}
		if(pDeleteRoom->contents){
			ch->println("Purging objects in room...");
		}
		for ( obj = pDeleteRoom->contents; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			extract_obj( obj );
		}
	}

	// remove all the resets
	if(pDeleteRoom->reset_first)
	{
		ch->println("Deallocating room resets...");
		reset_data *rnext, *reset;
		for(reset=pDeleteRoom->reset_first; reset; reset=rnext){
			rnext=reset->next;
			free_reset_data( reset);
		}
		pDeleteRoom->reset_first=NULL;
	}

	// remove all the exits leading out of the room
	{	
		bool no_exits_removed=true;
		for ( door = 0; door < MAX_DIR; door++ ){
			if(!pDeleteRoom->exit[door]){
				continue;
			}
			if(no_exits_removed){
				ch->println("Removing exits leading out of the room...");
				no_exits_removed=false;
			}
			// remove exit leading out
			free_exit( pDeleteRoom->exit[door] );
			pDeleteRoom->exit[door]=NULL;
		}
	}

	// remove all the exits leading into the room 
	{
		bool no_exits_removed=true;
		for(i=0; i<MAX_KEY_HASH; i++){
			for(ROOM_INDEX_DATA *r= room_index_hash[i]; r; r= r->next )
			{
				for ( door = 0; door < MAX_DIR; door++ )
				{
					if ( r->exit[door] && r->exit[door]->u1.to_room==pDeleteRoom)
					{
						if(no_exits_removed){
							ch->println("Removing exits leading into room...");
							no_exits_removed=false;
						}
						free_exit( r->exit[door] );
						r->exit[door]=NULL;
					}
				}
			}
		}
	}

	// remove all the room echoes
	if(pDeleteRoom->echoes)
	{
		ch->println("Removing room echoes...");
		room_echo_data *echo, *echo_next;
		for(echo= pDeleteRoom->echoes; echo; echo=echo_next){
			echo_next=echo->next;
			free_room_echo( echo );
		}
		pDeleteRoom->echoes=NULL;
	}
	
	// remove extended descriptions
	if(pDeleteRoom->extra_descr)
	{
		ch->println("Removing room extra descriptions...");
	    EXTRA_DESCR_DATA *ed, *ec_next;
		for ( ed = pRoom->extra_descr; ed; ed = ec_next ){
			ec_next=ed->next;
			free_extra_descr( ed );
		}
		pRoom->extra_descr=NULL;
	}
	ch->println("Deleting room...");
	int v=pDeleteRoom->vnum;

	// remove room from hash table
	{
		i=pDeleteRoom->vnum% MAX_KEY_HASH;
		// check if we are the first entry in the hash table
		if(pDeleteRoom== room_index_hash[i]){
			room_index_hash[i]=room_index_hash[i]->next;
		}else{
			ROOM_INDEX_DATA *prev=room_index_hash[i];
			if(!prev){
				bugf("redit_rdelete(): Trying to free room vnum %d, but not found in room_index_hash[%d]!", 
					pDeleteRoom->vnum, i);
			}else{
				for( ; prev->next; prev=prev->next )
				{
					if(prev->next==pDeleteRoom){
						prev->next=pDeleteRoom->next; // remove the room from the link
						break;
					}
				}
			}
		}
	}
	free_room_index(pDeleteRoom);
	top_room--;

	ch->printlnf("Room %d Deleted.",v);
	
	return true;
}
/**************************************************************************/
bool redit_setlockers( char_data *ch, char * argument)
{
	int value;
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);

    if(IS_NULLSTR(argument)){
		ch->println("Syntax : setlockers <quantity> <intitial rent> <ongoing rent> <weight> <capacity> <pickproof>" );
		ch->println("<quantity> = the number of lockers in the room.");
		ch->println("<initial rent> = the amount of gold to be allocated a locker, and pay for the first IC months rent of it.");
		ch->println("<ongoing rent> = the amount of gold to pay for 1 IC years rent of the locker.");
		ch->println("<weight> = this is the v0 value of the locker container object.");
		ch->println("<capacity> = this is the v3 value of the locker container object.");
		ch->println("<pickproof> = 1=very hard to pick, 2=random chance it will be on purchase, 3=pickproof.");
		ch->println("Syntax : setlockers clear    - remove all room lockers." );
		ch->println("Note: only admin can set lockers in a room.");
		return false;
	}

	if(!IS_ADMIN(ch)){
		ch->println("Only admin can edit the locker configuration of a room.");
		return false;
	}

	// check if they are trying to clear the lockers
	int inuse=lockers->count_used_lockers_in_room(pRoom->vnum);
	if(!str_cmp(argument, "clear")){
		if(inuse){
			ch->printlnf("There %s currently %d locker%s in use.  Manually remove these first with the 'locker delete' command.",
				inuse==1?"is":"are",
				inuse,
				inuse==1?"":"s");											
			return false;
		}

		if(!pRoom->lockers){
			ch->println("There are no lockers setup in this room.");
			return false;
		}

		ch->println("Removing locker allocation, previous locker details:");
		ch->printlnf("  quantity=%d, initial rent=%d, ongoing rent=%d, "
			"weight(v0)=%d, capacity(v3)=%d, pickproof=%d",
			pRoom->lockers->quantity,
			pRoom->lockers->initial_rent,
			pRoom->lockers->ongoing_rent,			
			pRoom->lockers->weight,
			pRoom->lockers->capacity,
			pRoom->lockers->pick_proof);
		delete pRoom->lockers;
		pRoom->lockers=NULL;
		return true;
	}

    // split up the five arguments
	int i;
    char args[6][MIL];
	for(i=0; i<6; i++){
		argument = one_argument( argument, args[i]);

		if(IS_NULLSTR(args[i])){
			ch->println("All six arguments must be completed.");
			redit_setlockers(ch,"");
			return false;
		}

		if(!is_number(args[i])){
			ch->printlnf("All six arguments must be numeric, argument %d (%s) is not.", i+1, args[i]);
			redit_setlockers(ch,"");
			return false;	
		}

		value=atoi(args[i]);
		if(value<1){
			ch->printlnf("All six arguments must be 1 or greater, argument %d is not.", i+1);
			return false;
		}
	}


	int quantity		=atoi(args[0]);
	int intitial_rent	=atoi(args[1]);
	int ongoing_rent	=atoi(args[2]);
	int weight			=atoi(args[3]);
	int capacity		=atoi(args[4]);
	int pickproof		=atoi(args[5]);


	if(capacity>weight){
		ch->printlnf("It makes no sense for the capacity (%d) to be greater than weight (%d).",
			capacity,
			weight);
		return false;
	}

	if(pickproof<1 || pickproof>3){
		ch->printlnf("Pickproof setting must be in the range 1 to 3 not (%d).",
			pickproof);
		return false;
	}

	// we know all arguments are above 0, so if there are currently no lockers, we are adding some
	if(!pRoom->lockers){
		pRoom->lockers=new locker_room_data;
		pRoom->lockers->quantity=0;
		pRoom->lockers->initial_rent=0;
		pRoom->lockers->ongoing_rent=0;
		pRoom->lockers->weight=0;
		pRoom->lockers->capacity=0;
		pRoom->lockers->pick_proof=0;
	}
	
	if(quantity<inuse){
		ch->printlnf("There %s currently %d locker%s in use.  You can not set the quantity to less than this.",
			value==1?"is":"are",
			value,
			value==1?"":"s");											
		return false;
	}



	ch->println("Changing locker allocation, previous locker details:");
	ch->printlnf("Quantity=%d, initial rent=%d, ongoing rent=%d, "
		"weight(v0)=%d, capacity(v3)=%d, pickproof=%d",
		pRoom->lockers->quantity,
		pRoom->lockers->initial_rent,
		pRoom->lockers->ongoing_rent,			
		pRoom->lockers->weight,
		pRoom->lockers->capacity,
		pRoom->lockers->pick_proof);

	pRoom->lockers->quantity=quantity;
	pRoom->lockers->initial_rent=intitial_rent;
	pRoom->lockers->ongoing_rent=ongoing_rent;
	pRoom->lockers->weight=weight;
	pRoom->lockers->capacity=capacity;
	pRoom->lockers->pick_proof=pickproof;

	ch->println("Updated locker details:");
	ch->printlnf("quantity=%d, initial rent=%d, ongoing rent=%d, "
		"weight(v0)=%d, capacity(v3)=%d, pickproof=%d",
		pRoom->lockers->quantity,
		pRoom->lockers->initial_rent,
		pRoom->lockers->ongoing_rent,			
		pRoom->lockers->weight,
		pRoom->lockers->capacity,
		pRoom->lockers->pick_proof);

	ch->println("Note: the weight, capacity and pickproof settings of any existing lockers are unaffected.");
    return true;
}       

/**************************************************************************/
/**************************************************************************/

