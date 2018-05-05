/**************************************************************************/
// act_obj.cpp - players performing actions relating to objects
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
#include "msp.h" 
#include "lockers.h"
#include "shop.h"

/* command procedures needed */
DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_stand		);
char *get_weapontype(OBJ_DATA *obj); /* prototype - handler.c */
bool trapcheck_get(char_data *ch, OBJ_DATA *obj);
SPRESULT spell_fear_magic( int sn, int level, char_data *ch, void *vo,int ); /* magic.cpp */

// Local functions.
#define CD char_data
#define OD OBJ_DATA
bool	remove_obj	args( (char_data *ch, int iWear, bool fReplace ) );
CD *	find_keeper	args( (char_data *ch ) );
int	get_cost	args( (char_data *keeper, OBJ_DATA *obj, bool fBuy ) );
void 	obj_to_keeper	args( (OBJ_DATA *obj, char_data *ch ) );
OD *	get_obj_keeper	args( (char_data *ch,char_data *keeper,char *argument));

#undef OD
#undef CD

bool can_loot(char_data *, OBJ_DATA *)
{
	return true;	
}

/**************************************************************************/
void get_obj( char_data *ch, OBJ_DATA *obj, OBJ_DATA *container)
{
    // variables for AUTOSPLIT 
    char_data *gch;
    int members;
    char buffer[100];

    if ( !CAN_WEAR(obj, OBJWEAR_TAKE) )
	{
		ch->println("You can't take that.");
		return;
    }

	if ( trapcheck_get( ch, obj ))
		return;

	if ( obj->item_type == ITEM_MONEY)
	{
		// check silver first 
		if ((get_carry_weight( ch ) +
			get_silver_weight(obj->value[0])) > can_carry_w( ch ) )
		{
			ch->println("You cannot carry that much weight.");
			return;
		}
		
		// now lets check if the player can carry that much gold 
		if ((get_carry_weight( ch ) +
			get_gold_weight(obj->value[1])) > can_carry_w( ch ) )
		{
			ch->println("You cannot carry that much weight.");
			return;
		}
	}

    if ( !IS_SWITCHED (ch) && (ch->carry_number + get_obj_number( obj ) > can_carry_n( ch )))
    {
        act( "$d: you can't carry that many items.",
            ch, NULL, obj->name, TO_CHAR );
        return;
    }


    if ( !IS_SWITCHED (ch) && get_carry_weight(ch) + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
		act( "$d: you can't carry that much weight.",
		    ch, NULL, obj->name, TO_CHAR );
		return;
    }

    if (obj->in_room){
		for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room){
			if (gch->on == obj){
				act("$N appears to be using $p.",
					ch,obj,gch,TO_CHAR);
				return;
			}
		}
    }

	// check if the object is contained by an object, which has a container_get_pre trigger
	obj_data *was_contained_inside_object=NULL;
	if(obj->in_obj && IS_VALID(obj->in_obj)){
		was_contained_inside_object=obj->in_obj;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_CONTAINER_GET_PRE))return;
	}

	// check if the object has a get_pre trigger
	if(oprog_execute_if_appropriate(obj, ch, OTRIG_GET_PRE))return;

    if ( container != NULL ){
		if(container->pIndexData->vnum == OBJ_VNUM_PIT
			&&  !CAN_WEAR(container, OBJWEAR_TAKE)
			&&  !IS_OBJ_STAT(obj,OBJEXTRA_HAD_TIMER))
			obj->timer = 0;
		
        act( "You get $p from $P.", ch, obj, container, TO_CHAR );
        act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
		REMOVE_BIT(obj->extra_flags,OBJEXTRA_HAD_TIMER);
		obj_from_obj( obj );
    }else{
        act( "You get $p.", ch, obj, container, TO_CHAR );
        act( "$n gets $p.", ch, obj, container, TO_ROOM );
        obj_from_room( obj );
    }

	if ( obj->item_type == ITEM_MONEY)
    {
		ch->silver += obj->value[0];
		ch->gold += obj->value[1];
        if (IS_SET(ch->act,PLR_AUTOSPLIT))
        { // AUTOSPLIT code 
			members = 0;
			for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
			{
				if (!IS_AFFECTED(gch,AFF_CHARM) && is_same_group( gch, ch ) )
					members++;
			}
			
			if ( members > 1 && (obj->value[0] > 1 || obj->value[1]))
			{
				sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
				do_split(ch,buffer);	
			}
        }

		if(!IS_NPC(ch)){
			if(obj->lastdrop_id!=ch->player_id
				&& !strcmp(obj->lastdrop_remote_ip,ch->remote_ip_copy))
			{
				multilog_alertf(ch, "`Y%s<%d>`x getting `Y%d gold `xand `S%d silver `xfrom somewhere "
					"(prob the ground), `M(room %d)`x, this was put there by someone `B(id=%d)`x other than %s but from the same "
					"ip address as `Y%s!`x", 
					ch->name, 
					ch->level,
					obj->value[1],
					obj->value[0],
					ch->in_room_vnum(),
					(int)obj->lastdrop_id,
					ch->name, 
					ch->name);
			}
		}
		extract_obj( obj );
	}else{
		// multilog abuse checks
		if(!IS_NPC(ch)){
			if(obj->lastdrop_id!=ch->player_id && 
				!strcmp(obj->lastdrop_remote_ip,ch->remote_ip_copy))
			{

				multilog_alertf(ch, 
					"`Y%s<%d>`x getting `Gobject `x'%s'<%d>(%d)`x which was originally put where it is being 'got from' (room %d)"
					"by `Bsomeone (id=%d)`x different than `Y%s`x, but from the same ip address as `Y%s!`x",
					ch->name, 
					ch->level,
					obj->short_descr,
					obj->level,
					obj->pIndexData?obj->pIndexData->vnum:0,
					ch->in_room_vnum(),
					(int)obj->lastdrop_id,
					ch->name, 
					ch->name);
			}
		}	
		obj_to_char( obj, ch );

		// if the object has a get_post trigger, run it
		oprog_execute_if_appropriate(obj, ch, OTRIG_GET_POST);

		// if the object was contained in another, and that object had a container_get_post trigger
		// run that now.
		if(was_contained_inside_object && IS_VALID(was_contained_inside_object)){
			oprog_execute_if_appropriate(was_contained_inside_object, ch, OTRIG_CONTAINER_GET_POST);
		}

    }
    return;
}

/************************************************************************/
// get a random object from the characters inventory
OBJ_DATA *get_random_obj( char_data* mob )
{

    OBJ_DATA *obj_next, *obj, *target=NULL;
    int now = 0, highest = 0;
		  
    for ( obj = mob->carrying; obj != NULL; obj = obj_next)
    {
          obj_next = obj->next_content;

          if ( obj->wear_loc != WEAR_NONE 
              || !can_drop_obj( mob, obj ) )
              continue;

          if ( ( now = number_percent() ) > highest )
          {
              target = obj;
              highest = now;
          }
     }

     return target;
}

/************************************************************************/
void donate(char_data *ch, OBJ_DATA *obj)
{
	if(!CAN_WEAR( obj, OBJWEAR_TAKE) || obj->item_type == ITEM_CORPSE_PC){
		act("You can't donate $p.", ch, obj,NULL, TO_CHAR);
		return;
	}

	int room_vnum=0;
	// confirm the room they are going to donate to exists
	if (obj->level < 11){
		if (obj->item_type == ITEM_WEAPON){
			room_vnum=ROOM_VNUM_NEWBIEWEAPON_DONATE;			
		}else if (obj->item_type == ITEM_ARMOR){
			room_vnum=ROOM_VNUM_NEWBIEARMOR_DONATE;			
		}else{
			room_vnum=ROOM_VNUM_NEWBIEMISC_DONATE;			
		}
	}else{
		if (obj->item_type == ITEM_WEAPON){
			room_vnum=ROOM_VNUM_WEAPON_DONATE;			
		}else if (obj->item_type == ITEM_ARMOR){
			room_vnum=ROOM_VNUM_ARMOR_DONATE;
		}else{
			room_vnum=ROOM_VNUM_MISC_DONATE;
		}
	}
	room_index_data *room=get_room_index(room_vnum);

	if(!room){
		act(FORMATF("Bug: couldn't find room %d to donate object %d ($p) "
			"- please inform the admin.", 
			room_vnum, obj->pIndexData?obj->pIndexData->vnum:-1), ch, obj,NULL, TO_CHAR);
		return;
	}
	
	ch->println("Your donation is greatly appreciated");
	ch->printlnf("%s disappears in a puff of smoke.", obj->short_descr);
	act( "$n donates $p.", ch, obj, NULL, TO_ROOM);

	obj_from_room(obj);	
	room_index_data *oldroom=ch->in_room;
	ch->in_room=room;
	
	SET_BIT(obj->extra2_flags,OBJEXTRA2_NOSELL);

	// find the pit in the room
	OBJ_DATA *pit;
	for( pit= room->contents; pit; pit= pit->next_content )
	{
		if(pit->pIndexData && pit->pIndexData->vnum==OBJ_VNUM_PIT){
			break;
		}
	}

	if(pit){
		act( "$p appears in $P.", ch, obj, pit, TO_ROOM);
		obj->timer=200; // objects donated last around 3 hours irl time
		obj_to_obj(obj,pit);
	}else{
		act( "$p appears in the room by your feet.", ch, obj, NULL, TO_ROOM);
		obj_to_room(obj,room);
	}
	ch->in_room=oldroom;
}
/************************************************************************/
// multiroom donation system - by Balo and Kal
void do_donate( char_data *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg[MIL];
    OBJ_DATA *obj_next;
    bool found;
    sh_int max_don;
	
	one_argument( argument, arg);
	
	if(IS_NULLSTR(arg)){
		ch->println("Donate What?!");
		return;
	}
	
	// handle donating a specific item
	if( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
	{
		obj = get_obj_list( ch, arg, ch->in_room->contents );
		if (!obj){
			ch->printlnf("I don't see any '%s' on the ground here to donate!", arg);
			return;
		}
		donate(ch, obj);
		return;
	}


	// handle donating all 
	{
		found = false;
		max_don = 0; 
		for ( obj = ch->in_room->contents; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			if(!CAN_WEAR(obj, OBJWEAR_TAKE)){
				continue;
			}

			if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
				&&   can_see_obj( ch, obj ) )
			{
				found = true;
				max_don++;
				if (max_don > 100)
				{
					ch->wrapln("That's a lot of stuff!!    Hold up a sec!  "
						"The pipelines in the sky are a little full..  "
						"give it a sec and try again!");
					return;
				}
				donate(ch, obj);
			}
		}
		
		if ( !found ){
			ch->println( "I don't see that here.");
		}
	}
}

/************************************************************************/
void do_get( char_data *ch, char *argument)
{
	char arg1[MIL];
	char arg2[MIL];
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	OBJ_DATA *container;
	bool found;
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	
	if(!str_cmp(arg2,"from")){
		argument = one_argument(argument,arg2);
	}
	
    // Get type. 
    if ( IS_NULLSTR(arg1) )
    {
		ch->println( "Get what?" );
		return;
    }

	// ooc rooms
	if(!IS_IMMORTAL(ch) && IS_SET(ch->in_room->room_flags,ROOM_OOC)){
		ch->println("Not in an OOC room.");
		return;
	}

    if( IS_NULLSTR(arg2) )
	{
		if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
		{
			// 'get obj' 
			obj = get_obj_list( ch, arg1, ch->in_room->contents );
			if ( obj == NULL )
			{
				act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
				return;
			}
			
			get_obj( ch, obj, NULL);    
		}
		else
		{
			// 'get all' or 'get all.obj'
			found = false;
			for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
			{
				obj_next = obj->next_content;
				if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
					&&   can_see_obj( ch, obj ) )
				{
					// allow creation of objects with hidden name
					if(IS_SET( obj->extra_flags, OBJEXTRA_NO_GET_ALL )){
						continue;
					}
											
					found = true;
					get_obj( ch, obj, NULL);    
				}
			}
			
			if ( !found ) 
			{
				if ( arg1[3] == '\0' ){
					ch->println("I see nothing here.");
				}else{
					act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
				}
			}
		}
    }
    else
    {
		// 'get ... container'
		if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
		{
			ch->println("You can't do that.");
			ch->println("syntax: get <object> <containing_object>");
			return;
		}
		
		if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
		{
			act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
			return;
		}

		// allow creation of objects with hidden name
		if(IS_SET( container->extra_flags, OBJEXTRA_NO_GET_ALL )){
			act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
			return;
		}
		
		switch ( container->item_type )
		{
		default:
			{
				ch->printlnf( "%s is not a container.", container->short_descr);
				if(IS_NEWBIE(ch)){
					ch->println( "`Wsyntax:`=C get <what> <container>`x");
				}
				return;
			}

		case ITEM_CAULDRON:
		case ITEM_CONTAINER:
		case ITEM_FLASK:
		case ITEM_MORTAR:
		case ITEM_CORPSE_NPC:
			break;
			
		case ITEM_CORPSE_PC:
			{
				
				if (!can_loot(ch,container))
				{
					ch->println("You can't do that.");
					return;
				}
			}
		}
		
		if ( IS_SET(container->value[1], CONT_CLOSED) )
		{
			act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
			return;
		}

		if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
		{
			// 'get obj container' 
			obj = get_obj_list( ch, arg1, container->contains );
			if ( obj == NULL )
			{
				act( "I see nothing like that in the $T.",
					ch, NULL, arg2, TO_CHAR );
				return;
			}
			get_obj( ch, obj, container);    
		}
		else
		{
			// 'get all container' or 'get all.obj container' 
			found = false;
			for ( obj = container->contains; obj != NULL; obj = obj_next )
			{
				obj_next = obj->next_content;
				if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
					&&   can_see_obj( ch, obj ) )
				{
					found = true;
					if (container->pIndexData->vnum == OBJ_VNUM_PIT
						&&  !IS_IMMORTAL(ch))
					{
						ch->println("Don't be so greedy!");
						return;
					}
					get_obj( ch, obj, container);    
				}
			}
			
			if ( !found )
			{
				if ( arg1[3] == '\0' ){
					act( "I see nothing in the $T.",
						ch, NULL, arg2, TO_CHAR );
				}else{
					act( "I see nothing like that in the $T.",
						ch, NULL, arg2, TO_CHAR );
				}
			}
		}
    }
    return;
}

/************************************************************************/
void do_put( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    obj_data *container;
    obj_data *obj;
    obj_data *obj_next;
	
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
	
    if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
		argument = one_argument(argument,arg2);
	
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
		ch->println("Put what in what?");
		return;
	}

	// ooc rooms
	if(!IS_IMMORTAL(ch) && IS_SET(ch->in_room->room_flags,ROOM_OOC)){
		ch->println("Not in an OOC room.");
		return;
	}
	
    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
		ch->println("You can't do that.");
		return;
    }
	
    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
		act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
		return;
    }
	
	switch ( container->item_type )
	{
	default:
		ch->printlnf( "%s is not a container.", container->short_descr);
		if(IS_NEWBIE(ch)){
			ch->println( "`Whint?:`=C put <what> <container>`x");
		}
		return;
	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
		break;
	}

    if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
		act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
		return;
	}
	
    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
		// 'put obj container' 
		if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
		{
			ch->println("You do not have that item.");
			return;
		}
		
		if ( obj == container )
		{
			ch->println("You can't fold it into itself.");
			return;
		}
		
		if ( !can_drop_obj( ch, obj ) )
		{
			ch->println("You can't let go of it.");
			return;
		}
		
		if (WEIGHT_MULT(obj) != 100)
		{
			ch->println("You have a feeling that would be a bad idea.");
            return;
        }

		if (get_obj_weight(obj) > (container->value[3] * 10))
		{
			ch->printlnf( "%s appears to be too big to ever be put into %s.", 
				obj->short_descr, container->short_descr);
			return;
		}

		if (get_obj_weight( obj ) + get_true_weight( container ) - container->weight 
			> (container->value[0] * 10))
		{
			ch->printlnf( "There doesn't appears to be enough room inside %s to fit %s.`1", 
				container->short_descr, obj->short_descr);
			return;
		}

		if(oprog_execute_if_appropriate(container, ch, OTRIG_CONTAINER_PUTIN_PRE)) return;
		
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_PUT_PRE))return;

		if (container->pIndexData->vnum == OBJ_VNUM_PIT 
			&&  !CAN_WEAR(container,OBJWEAR_TAKE))
		{
			if (obj->timer){
				SET_BIT(obj->extra_flags,OBJEXTRA_HAD_TIMER);
			}else{
				obj->timer = number_range(100,200);
			}
		}
		obj_from_char( obj );
		obj_to_obj( obj, container );
		
		if (IS_SET(container->value[1],CONT_PUT_ON))
		{
			act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
			act("You put $p on $P.",ch,obj,container, TO_CHAR);
		}
		else
		{
			act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
			act( "You put $p in $P.", ch, obj, container, TO_CHAR );
		}
		oprog_execute_if_appropriate(obj, ch, OTRIG_PUT_POST);

		oprog_execute_if_appropriate(container, ch, OTRIG_CONTAINER_PUTIN_POST);

    }
    else
    {
		// PC being ordered to do this
		if ( !IS_NPC(ch) && IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
		{
			if ( ch->master )
				ch->master->println("Not going to happen.");
			return;
		}

		// 'put all container' or 'put all.obj container'
		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;
			
			if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
				&&   can_see_obj( ch, obj )
				&&   WEIGHT_MULT(obj) == 100
				&&   obj->wear_loc == WEAR_NONE
				&&   obj != container
				&&   can_drop_obj( ch, obj )
				&&   get_obj_weight( obj ) + get_true_weight( container )
				<= (container->value[0] * 10) 
				&&   get_obj_weight(obj) <= (container->value[3] * 10))
			{
				if(oprog_execute_if_appropriate(container, ch, OTRIG_CONTAINER_PUTIN_PRE)) continue;

				if(oprog_execute_if_appropriate(obj, ch, OTRIG_PUT_PRE))continue;

				if (container->pIndexData->vnum == OBJ_VNUM_PIT
					&&  !CAN_WEAR(obj, OBJWEAR_TAKE) 
				){
					if (obj->timer){
						SET_BIT(obj->extra_flags,OBJEXTRA_HAD_TIMER);
					}else{
						obj->timer = number_range(100,200);
					}
				}

				obj_from_char( obj );
				obj_to_obj( obj, container );
				
				if (IS_SET(container->value[1],CONT_PUT_ON)){
					act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
					act("You put $p on $P.",ch,obj,container, TO_CHAR);
				}else{
					act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
					act( "You put $p in $P.", ch, obj, container, TO_CHAR );
				}
				oprog_execute_if_appropriate(obj, ch, OTRIG_PUT_POST);

				oprog_execute_if_appropriate(container, ch, OTRIG_CONTAINER_PUTIN_POST);
			}
		}
    }
    return;
}


/************************************************************************/
void do_drop( char_data *ch, char *argument)
{
    char arg[MIL];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
		ch->println("Drop what?");
		return;
    }

	// ooc rooms
	if(!IS_IMMORTAL(ch) && IS_SET(ch->in_room->room_flags,ROOM_OOC)){
		ch->println("Not in an OOC room.");
		return;
	}

	if ( is_number( arg ) )
    {
		// 'drop NNNN coins'
		int amount, gold = 0, silver = 0;
		
		amount   = atoi(arg);
		argument = one_argument( argument, arg );
		if ( amount <= 0
			|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) &&
			str_cmp( arg, "gold"  ) && str_cmp( arg, "silver") ) )
		{
			ch->println("Sorry, you can't do that.");
			return;
		}
		
		if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin") 
			||   !str_cmp( arg, "silver"))
		{
			if (ch->silver < amount)
			{
				ch->println("You don't have that much silver.");
				return;
			}
			
			ch->silver -= amount;
			silver = amount;
		}
		
		else
		{
			if (ch->gold < amount)
			{
				ch->println("You don't have that much gold.");
				return;
			}
			
			ch->gold -= amount;
			gold = amount;
		}
		
		for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;
			
			switch ( obj->pIndexData->vnum )
			{
            case OBJ_VNUM_SILVER_ONE:
                silver += 1;
                extract_obj(obj);
				break;
				
            case OBJ_VNUM_GOLD_ONE:
                gold += 1;
                extract_obj( obj );
				break;
				
            case OBJ_VNUM_SILVER_SOME:
                silver += obj->value[0];
                extract_obj(obj);
				break;
				
            case OBJ_VNUM_GOLD_SOME:
                gold += obj->value[1];
                extract_obj( obj );
				break;
				
            case OBJ_VNUM_COINS:
                silver += obj->value[0];
                gold += obj->value[1];
                extract_obj(obj);
				break;
			}
		}
		
		obj=create_money( gold, silver );
		// record details for multilogging detection
		if(!IS_NPC(ch)){
			obj->lastdrop_id=ch->player_id;
			replace_string(obj->lastdrop_remote_ip,ch->remote_ip_copy);
		}		
		obj_to_room( obj, ch->in_room );
		
		act( "$n drops some coins.", ch, NULL, NULL, TO_ROOM );
		ch->println( "OK." );
		return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
		// 'drop obj' 
		if ( ( obj = get_obj_carry( ch, arg ) ) == NULL ){
			ch->println( "You do not have that item." );
			return;
		}
		
		if ( !can_drop_obj( ch, obj ) ){
			ch->println( "You can't let go of it." );
			return;
		}
		
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_DROP_PRE))return;

		obj_from_char( obj );

		// record details for multilogging detection
		if(!IS_NPC(ch)){
			obj->lastdrop_id=ch->player_id;
			replace_string(obj->lastdrop_remote_ip, ch->remote_ip_copy);
		}		

		obj_to_room( obj, ch->in_room );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
		if (IS_OBJ_STAT(obj,OBJEXTRA_MELT_DROP))
		{
			act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
			act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
			extract_obj(obj);
		}
		oprog_execute_if_appropriate(obj, ch, OTRIG_DROP_POST);
    }
    else
    {
		// PC being ordered to do this
		if ( !IS_NPC(ch) && IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
		{
			if ( ch->master )
				ch->master->println("Not going to happen.");
			return;
		}
		
		// 'drop all' or 'drop all.obj' 
		found = false;
		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;
			
			if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
				&&   can_see_obj( ch, obj )
				&&   obj->wear_loc == WEAR_NONE
				&&   can_drop_obj( ch, obj ) )
			{				
				found = true;
				
				if(oprog_execute_if_appropriate(obj, ch, OTRIG_DROP_PRE))continue;

				obj_from_char( obj );
				// record details for multilogging detection
				if(!IS_NPC(ch)){
					obj->lastdrop_id=ch->player_id;
					replace_string(obj->lastdrop_remote_ip, ch->remote_ip_copy);
				}		
				obj_to_room( obj, ch->in_room );
				
				act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
				act( "You drop $p.", ch, obj, NULL, TO_CHAR );
				if (IS_OBJ_STAT(obj,OBJEXTRA_MELT_DROP))
				{
					act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
					act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
					extract_obj(obj);
				}
				oprog_execute_if_appropriate(obj, ch, OTRIG_DROP_POST);	
			}
		}
		
		if ( !found )
		{
			if ( arg[3] == '\0' )
				act( "You are not carrying anything.",
				ch, NULL, arg, TO_CHAR );
			else
				act( "You are not carrying any $T.",
				ch, NULL, &arg[4], TO_CHAR );
		}
    }

    return;
}

/************************************************************************/
void do_give_new( char_data *ch, char *argument, int silent)
{
	char arg1 [MIL];
	char arg2 [MIL];
	char buf[MSL];
	char_data *victim;
	OBJ_DATA  *obj;
	
	argument = one_argument( argument, arg1 ); // what
	argument = one_argument( argument, arg2 ); // whom
	
	// display the help if they don't give enough arguments
	if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
	{
		ch->println("");
		if(!IS_NULLSTR(arg1) && IS_NULLSTR(arg2)){
			ch->printlnf("Give '%s' to who?", arg1);
		}else{
			ch->println( "Give what to whom?");
		}
		ch->println("");
		ch->titlebar( "Giving Objects");
		ch->println( "Syntax: give <object/what> [to] <person/creature>");
		ch->println( "e.g. typing 'give sword guard'");
		ch->println( "     - will give a sword to a guard, (assuming you are carrying one).");
		ch->println( "note: the word 'to' is optional in the give command.");
		ch->titlebar( "Giving Currency");
		// 'give NNNN coins [to] victim'
		ch->println( "Syntax: give <amount> gold [to] <person/creature>");
		ch->println( "e.g. typing 'give 15 silver guard'");
		ch->println( "     - will give 15 of your silver coins to a guard.");
		ch->println( "e.g. typing 'give 2 gold guard'");
		ch->println( "     - will give 2 of your gold coins to a guard.");
		ch->println( "note: 1 gold is worth 10 silver.");
		ch->titlebar( "");
		return;
	}

	// ooc rooms
	if(!IS_IMMORTAL(ch) && IS_SET(ch->in_room->room_flags,ROOM_OOC)){
		ch->println("Not in an OOC room, you can only give objects within the main game realm.");
		return;
	}
	
	if ( is_number( arg1 ) )
	{
		// 'give NNNN coins [to] victim'
		int amount;
		bool silver;
		
		amount   = atoi(arg1);
		if ( amount <= 0
			|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) &&
			str_cmp( arg2, "gold"  ) && str_cmp( arg2, "silver")) )
		{
			ch->println("Sorry, you can't do that.");
			return;
		}
		
		silver = str_cmp(arg2,"gold");
		
		argument = one_argument( argument, arg2 );
		if ( IS_NULLSTR(arg2))
		{
			ch->printlnf( "Give %d %s to whom?", amount, silver?"silver":"gold" );
			return;
		}

		// using optional 'to' in give command
		if(!str_cmp("to", arg2)){
			argument = one_argument( argument, arg2 ); 
		}

		// find they person they are giving to
		if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
		{
			ch->printlnf( "You can't seem to find any '%s' here to give %d %s to.", 
				arg2, amount, silver?"silver":"gold");
			return;
		}

		if(victim==ch){
			ch->printlnf("What would be the purpose of giving %d %s to yourself when you already have it?",
					amount, silver?"silver":"gold");
			return;
		}
		
		if (IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER))
		{
			// check that the money changer can see them 
			if(!can_see(victim,ch)){
				ch->println("They can't see you to give you your change.");
				return;
			}

			// check that the money changer isn't subdued
			if (victim->subdued){
				ch->println("They don't appear to be in a state to exchange your money.");
				return;
			}

			// sleeping
			if (!IS_AWAKE(victim)){
				ch->printlnf( "%s is sleeping and can not change your money right now.", PERS(victim, ch));
				return;
			}	   

		}	

		if ( (!silver && ch->gold < amount) || (silver && ch->silver < amount) )
		{
			ch->printlnf( "You haven't got %d %s to give!", amount, silver?"silver":"gold");
			return;
		}

		// make sure victim can carry that much money 
		if (get_carry_weight(victim) +
			  (silver ? get_silver_weight(amount) : get_gold_weight(amount))
               > can_carry_w( victim ) )
		{
			act( "$N can't carry that much weight.",
				ch, NULL, victim, TO_CHAR );
			return;
		}


		// record details for multilogging detection
		if( !IS_NPC(ch) 
			&& !HAS_CONFIG(ch, CONFIG_IGNORE_MULTILOGINS)
			&& !IS_NPC(victim) 
			&& !HAS_CONFIG(victim, CONFIG_IGNORE_MULTILOGINS)
		  )
		{
			if(!strcmp(ch->remote_ip_copy,victim->remote_ip_copy)){
				multilog_alertf(victim, "`C%s<%d>`B(id=%d) `xgiving %s%d %s`x to `Y%s<%d>`x (room %d), both from same IP!", 
					ch->name,
					ch->level,
					(int)ch->player_id,
					silver?"`S":"`Y",
					amount, 
					silver?"silver":"gold", 
					victim->name,
					victim->level,
					ch->in_room_vnum());
			}
		}		

		// exchange the money
		if (silver){
			ch->silver		-= amount;
			victim->silver 	+= amount;
		}else{
			ch->gold		-= amount;
			victim->gold	+= amount;
		}
		
		if (silent){
			sprintf(buf, "SILENTGIVE: %s gives %s %d %s.",  ch->name , victim->name, amount, silver ? "silver" : "gold");
			wiznet(buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
			sprintf(buf,"You SILENTLY give $N %d %s.",amount, silver ? "silver" : "gold");
			act( buf, ch, NULL, victim, TO_CHAR    );
		}else{
			sprintf(buf,"$n gives you %d %s.",amount, silver ? "silver" : "gold");
			act( buf, ch, NULL, victim, TO_VICT    );
			act( "$n gives $N some coins.",  ch, NULL, victim, TO_NOTVICT );
			sprintf(buf,"You give $N %d %s.",amount, silver ? "silver" : "gold");
			act( buf, ch, NULL, victim, TO_CHAR    );
			
			// Bribe trigger
			if ( IS_NPC(victim) && HAS_TRIGGER( victim, MTRIG_BRIBE ) ){
				// record how much gold is being used
				mudprog_bribe_silver_in_use=silver;
				mudprog_bribe_amount_in_use=amount;
				mudprog_bribe_money_from=ch;

				// do the actual trigger
				mp_bribe_trigger( victim, ch, silver ? amount : amount * 100 );
				limit_mobile_wealth(victim);

				mudprog_bribe_amount_in_use=0;
				mudprog_bribe_money_from=NULL;
			}		
		}

		// handle money changers giving money back
		if (IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER))
		{
			int change;
			
			change = (silver ? 95 * amount / 100 / 100 
				: 95 * amount);
			
			
			if (!silver && change > victim->silver){
				victim->silver += change;
			}
			
			if (silver && change > victim->gold){
				victim->gold += change;
			}
			
			if (change < 1 && can_see(victim,ch))
			{
				act(
					"$n tells you 'I'm sorry, you did not give me enough to change.'"
					,victim,NULL,ch,TO_VICT);
				sprintf(buf,"%d %s %s", 
					amount, silver ? "silver" : "gold",ch->name);
				do_give_new(victim,buf, silent);
			}
			else if (can_see(victim,ch))
			{
				sprintf(buf,"%d %s %s", 
					change, silver ? "gold" : "silver",ch->name);
				do_give_new(victim,buf, silent);
				if (silver)
				{
					sprintf(buf,"%d silver %s", 
						(95 * amount / 100 - change * 100),ch->name);
					do_give_new(victim,buf, silent);
				}
				act("$n tells you 'Thank you, come again.'", victim,NULL,ch,TO_VICT);
				limit_mobile_wealth(victim);
			}
		}
		return;
    } // end of code which deals with giving money
	
    
	// === GIVING OBJECTS CODE 
	// give <object> [to] <victim>

	// using optional 'to' in give command
	if(!str_cmp("to", arg2)){
		argument = one_argument( argument, arg2 ); 
	}

	// find the object 
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        ch->printlnf( "You are not carrying any '%s' to give to '%s'.", arg1, arg2 );
        return;
    }
	
    if ( obj->wear_loc != WEAR_NONE )
    {
        ch->printlnf( "You are currently using '%s', you must remove it before giving it to '%s'.", obj->short_descr, arg2 );
        return;
    }
	
	// find the victim to receive the item
    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
		ch->printlnf( "You can't seem to find any '%s' here to give '%s' to.", arg2, obj->short_descr);
		return;
    }
	
	if(victim==ch){
		ch->printlnf("What would be the purpose of giving %s to yourself?", obj->short_descr);
		return;
	}
		
    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
		// if they are a shop keeper without a give trigger which
		// would catch the object, don't accept the item
		if(!mp_would_run_give_trigger(victim, ch, obj )){
			act("$N tells you 'Sorry, you'll have to sell that.'", ch,NULL,victim,TO_CHAR);
			return;
		}
    }
	
    if ( !can_drop_obj( ch, obj ) )
    {
		ch->printlnf( "You can't let go of %s.", obj->short_descr );
		return;
	}
	
    if ( !IS_CONTROLLED(victim) && victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
        act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
        return;
    }
	
    if (!IS_CONTROLLED(victim) && get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
        act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
        return;
    }
	
    if ( !can_see_obj( victim, obj ) )
    {
        act( FORMATF("$N can't seem to see %s.", obj->short_descr), ch, NULL, victim, TO_CHAR );
        return;
    }
	
    obj_from_char( obj );

	// check lastgive details for multilogging detection
	if(!IS_NPC(victim) 
		&& !HAS_CONFIG(ch, CONFIG_IGNORE_MULTILOGINS)
		&& !HAS_CONFIG(victim, CONFIG_IGNORE_MULTILOGINS)){

		// player giving objects to itself via another player or mob
		if( obj->lastdrop_id!=victim->player_id 
			&& !strcmp(obj->lastdrop_remote_ip, victim->remote_ip_copy))
		{
			multilog_alertf(victim, "`M%s<%d>`x giving `Gobject '%s' (%d)`x to `Y%s<%d>`x (room %d), which was recently given to "
				"`M%s`x by `Bsomeone (id=%d)`x other than `Y%s`x from the same ip as `Y%s!`x", 
				ch->name, 
				ch->level,
				obj->short_descr,
				obj->pIndexData?obj->pIndexData->vnum:0,
				victim->name,
				victim->level,
				ch->in_room_vnum(),
				ch->name, 
				(int)obj->lastdrop_id,
				victim->name,
				victim->name);	
		// player giving directly to another of their players
		}else if(!strcmp(ch->remote_ip_copy, victim->remote_ip_copy)){
			multilog_alertf(ch, "`C%s<%d>`B(id=%d) `xgiving `Gobject '%s' (%d) `xto `Y%s<%d>`x (room%d) - object exchange between players from the same IP!",
				ch->name, 
				ch->level,
				(int)ch->player_id,
				obj->short_descr,
				obj->pIndexData?obj->pIndexData->vnum:0,
				victim->name,
				victim->level,
				ch->in_room_vnum());
		}
	}	

	// record host of giver on object if not a mob
	if(!IS_NPC(ch)){
		obj->lastdrop_id=ch->player_id;
		replace_string(obj->lastdrop_remote_ip, ch->remote_ip_copy);
	}

    obj_to_char( obj, victim );
	
    if (silent)
    {
        sprintf(buf, "SILENTGIVE: %s gives %s to %s.", ch->name, obj->short_descr, victim->name);
        wiznet(buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
        act( "You SILENTLY give $p to $N.", ch, obj, victim, TO_CHAR    );
    }
    else
    {
        MOBtrigger = false;
        act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
        act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
        act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
        MOBtrigger = true;
    }
	
    // Give mudprog trigger
    if ( IS_NPC(victim) && HAS_TRIGGER( victim, MTRIG_GIVE ) ){
        mp_give_trigger( victim, ch, obj );
	}
}

// normal give
void do_give( char_data *ch, char *argument )
{
    do_give_new(ch, argument, false);
}

// silent give 
void do_sgive( char_data *ch, char *argument )
{
    do_give_new(ch, argument, true);
}

/************************************************************************/
/*
 *  Take stuff and money from subdued or sleeping characters
 *  Written by Quenralther and Kalahn - Dec 97
 */
void do_grab( char_data *ch, char *argument )
{
	char arg1 [MIL];
	char arg2 [MIL];
	char buf[MSL];
	char_data *victim;
	OBJ_DATA  *obj;
	int random;
	
	if( IS_OOC(ch) )
	{ ch->println("You may not use grab in an ooc area."); return; }
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	
	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		ch->println("Take what from whom?");
		return;
	}

	if ( is_number( arg1 ) )
	{
		// 'give NNNN coins victim' 
		int amount;
		bool silver;
		
		amount	 = atoi(arg1);
		if ( amount <= 0
			|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) &&
			str_cmp( arg2, "gold"  ) && str_cmp( arg2, "silver")) )
		{
			ch->println("Sorry, you can't do that.");
			return;
		}
		
		silver = str_cmp(arg2,"gold");
		
		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			ch->println("Take what from whom?");
			return;
		}
		
		if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		
		if IS_AWAKE(victim) 
		{
			ch->println("You can't grab stuff from someone who's awake.");
			return;
		}	   

		if (IS_IMMORTAL(victim))
		{
			ch->println("You failed.");
			return;
		}

		if (is_safe(ch,victim))
		{
			return;
		}

		if (!IS_IMMORTAL(ch))
		{
			WAIT_STATE( ch, PULSE_VIOLENCE );
		}

		if ((victim->position == POS_SLEEPING 
			&& !IS_IMMORTAL(ch)
			&& (!IS_AFFECTED(victim, AFF_SLEEP)
			|| (!is_same_group(ch, victim) && number_range(1,5)==1)
			|| number_range(1,15)==1)
			)
			|| (victim->position<=POS_SLEEPING && IS_KEEPER(victim) && !IS_IMMORTAL(ch))
		)
        {
            random = dice(1,100);
            if (IS_KEEPER(victim)
                || ( random < 70 + victim->level-ch->level-ch->modifiers[STAT_QU]/5
                + victim->modifiers[STAT_IN]/5) )            
            {
				// remove sleep spell if necessary
				if ( IS_AFFECTED(victim, AFF_SLEEP) ){
					affect_strip( victim, gsn_sleep );
				}
				victim->position = POS_RESTING;
				act("$n tried to take some coins from $N, who wakes up.",ch,NULL,victim,TO_NOTVICT );
				act("You are awakened by $N trying to take some coins from you.",victim,NULL,ch,TO_CHAR);
				act("Your attempts have woken $N!",ch,NULL,victim,TO_CHAR);

				// wiznet thefts messages
				sprintf(buf,"$N <%d> tried to grab %d coins from %s <%d> and woke them up!", 
					ch->level, amount, victim->name, victim->level);
				wiznet(buf,ch,NULL,WIZ_THEFTS,0,0);
				return;				
			}

			random = dice(1,100);
			if ( random < 75-ch->level+victim->level )
			{
				act("$n couldn't manage to pry any coins away from $N.",ch,NULL,victim,TO_NOTVICT );
				act("You couldn't manage to pry any coins away from $N.",ch,NULL,victim,TO_CHAR);

				// wiznet thefts message
				sprintf(buf,"$N tried to grab %d coins from %s <%d> and failed.", 
					amount, victim->name, victim->level);
				wiznet(buf,ch,NULL,WIZ_THEFTS,0,0);
				return;
			}
		} // endif they are in the sleeping position... if they are below that you can 
		  // always take everything
		
		if (!silver && victim->gold < amount)
		{
			amount = victim->gold;
			// wiznet thefts messages
			sprintf(buf,"$N <%d> took using grab %d gold coins from %s <%d>.", 
				ch->level, amount, victim->name, victim->level);
			wiznet(buf,ch,NULL,WIZ_THEFTS,0,0);
		}
		
		if (silver && victim->silver < amount)
		{
			amount = victim->silver;
			// wiznet thefts messages
			sprintf(buf,"$N <%d> took using grab %d silver coins from %s <%d>.", 
				ch->level, amount, victim->name, victim->level);
			wiznet(buf,ch,NULL,WIZ_THEFTS,0,0);
			return;				
		}
		
		if (silver)
		{
			victim->silver	-= amount;
			ch->silver		+= amount;
		}
		else
		{
			victim->gold	-= amount;
			ch->gold		+= amount;
		}
		
		if (victim->position > POS_SLEEPING)
		{
			act( "$n took some coins from you.", ch, NULL, victim, TO_VICT);
		}
		act( "$n takes some coins from $N.",  ch, NULL, victim, TO_NOTVICT );
		sprintf(buf,"You take %d %s from $N.",amount, silver ? "silver" : "gold");
		act( buf, ch, NULL, victim, TO_CHAR    );
		return;
	} // end of coins
	
	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{							   
		ch->println("They aren't here.");
		return;
	}
	
	if (IS_AWAKE(victim))
	{
		ch->println("You can't grab stuff from someone who's awake.");
		return;
	}

	if (IS_IMMORTAL(victim))
	{
		ch->println("You failed.");
		return;
	}
	
	obj = get_obj_carry_for_looker( victim, arg1, ch ); // find the object in victims inventory
	
	if ( !obj) // not in their inventory check if they are wearing it
	{
		if ((obj = get_obj_wear(victim, arg1)) != NULL)
		{
			if ( IS_SET(obj->extra_flags, OBJEXTRA_NOREMOVE ))
			{
				ch->println("You can't wrench it away...it's stuck somehow.");
				return;
			}
		}
	}
	
	if (!obj) // check if nothing was found
	{
		ch->println("You can't find that item on their person.");
		return;
	}
	
#ifdef unix
		if (!IS_IMMORTAL(ch))
		{
			WAIT_STATE( ch, PULSE_VIOLENCE );
		}
#endif
	

if ((victim->position == POS_SLEEPING 
    && !IS_IMMORTAL(ch)
    && (!IS_AFFECTED(victim, AFF_SLEEP)
    || (!is_same_group(ch, victim) && number_range(1,5)==1)
    || number_range(1,15)==1)
    )
    || (victim->position<=POS_SLEEPING && IS_KEEPER(victim) && !IS_IMMORTAL(ch))
)
        {
            random = dice(1,100);
            if (IS_KEEPER(victim)
                ||( random < 90 + victim->level - ch->level
                  - ch->modifiers[STAT_AG]/10+victim->modifiers[STAT_IN]/5 )
               )
            {
/*
	if ( victim->position == POS_SLEEPING
		&& (!IS_AFFECTED(victim, AFF_SLEEP)
			|| (!is_same_group(ch, victim) && number_range(1,5)==1)
			|| number_range(1,15)==1
		   )
		)
	{
		random = dice(1,100);
		if ( random < 90 + victim->level - ch->level
              - ch->modifiers[STAT_AG]/10+victim->modifiers[STAT_IN]/5 )
		{
*/
			victim->position = POS_RESTING;
			act("$n tried to take something from $N, who wakes up.",ch,NULL,victim,TO_NOTVICT );
			act("You are awakened by $N trying to take something from you.",victim,NULL,ch,TO_CHAR);
			act("Your attempts have woken $N!",ch,NULL,victim,TO_CHAR);

			// wiznet thefts messages
            sprintf(buf,"$N <%d> tried to grab $p from %s <%d> and woke them up!", 
				ch->level, victim->name, victim->level);
			wiznet(buf,ch, obj,WIZ_THEFTS,0,0);
			return;				
		}
		random = dice(1,100);
		if ( random < 100-ch->level+victim->level-ch->modifiers[STAT_ST]/5+victim->modifiers[STAT_ST]/5 )
		{
			act("$n couldn't manage to pry anything away from $N.",ch,NULL,victim,TO_NOTVICT );
			act("You couldn't manage to pry anything away from $N.",ch,NULL,victim,TO_CHAR);
	
			// wiznet thefts messages
            sprintf(buf,"$N <%d> tried to grab $p from %s <%d> but failed to get it!", 
				ch->level, victim->name, victim->level);
			wiznet(buf,ch, obj,WIZ_THEFTS,0,0);
			return;
		}
	}

	if (is_safe(ch,victim))
	{
		return;
	}
	
	if ( !can_drop_obj( victim, obj ) )
	{
		ch->println("You can't wrench it away...it's stuck somehow.");

		// wiznet thefts messages
        sprintf(buf,"$N <%d> tried to grab $p from %s <%d> but couldn't wrench it away.", 
			ch->level, victim->name, victim->level);
		wiznet(buf,ch, obj,WIZ_THEFTS,0,0);
		return;
	}

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        ch->println("You have your hands full.");
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
        ch->println("You can't carry that much weight.");
        return;
    }
	


	
	//	object exchanges hands now
	obj_from_char( obj );
	obj_to_char( obj, ch );
	
	act( "$n takes $p from $N.", ch, obj, victim, TO_NOTVICT );
	if ( victim->position > POS_SLEEPING )
	{
		act( "$n takes $p from you.",	ch, obj, victim, TO_VICT	);
	}
	act( "You take $p from $N.", ch, obj, victim, TO_CHAR	 );

	// wiznet thefts messages
    sprintf(buf,"$N <%d> grabbed $p from %s <%d> successfully.", 
		ch->level, victim->name, victim->level);
	wiznet(buf,ch, obj,WIZ_THEFTS,0,0);
	return;
}

/************************************************************************/
/* for poisoning weapons and food/drink */
void do_envenom(char_data *ch, char *argument)
{
	OBJ_DATA *obj;
	AFFECT_DATA af;
	int percent,skill;

	/* find out what */
	if (IS_NULLSTR(argument))
	{
		ch->println("Envenom what item?");
		return;
	}

	obj =  get_obj_list(ch,argument,ch->carrying);

	if (obj== NULL)
	{
		ch->println("You don't have that item.");
		return;
	}

	if ((skill = get_skill(ch,gsn_envenom)) < 1)
	{
		ch->println("Are you crazy? You'd poison yourself!");
		return;
	}

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
		if (number_percent() < skill)  // success!
		{
			act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM);
			act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR);
			if (!obj->value[3])
			{
				obj->value[3] = 1;
				check_improve(ch,gsn_envenom,true,4);
			}
			WAIT_STATE(ch,skill_table[gsn_envenom].beats);
			return;
		}
		act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
		if (!obj->value[3])
		{
			check_improve(ch,gsn_envenom,false,4);
		}
		WAIT_STATE(ch,skill_table[gsn_envenom].beats);
		return;
	}

	if ( obj->item_type == ITEM_WEAPON )
	{
		if (IS_WEAPON_STAT( obj,WEAPON_FLAMING	)
		||  IS_WEAPON_STAT( obj,WEAPON_FROST	)
		||  IS_WEAPON_STAT( obj,WEAPON_VAMPIRIC	)
		||  IS_WEAPON_STAT( obj,WEAPON_SHOCKING	)
		||  IS_WEAPON_STAT( obj,WEAPON_HOLY		))
		{
			act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
			return;
		}

		if (obj->value[3] < 0 
		||  attack_table[obj->value[3]].damage == DAM_BASH)
		{
			ch->println("You can only envenom edged weapons.");
			return;
		}

		if (IS_WEAPON_STAT(obj,WEAPON_POISON))
		{
			act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
			return;
		}

		percent = number_percent();
		if (percent < skill)
		{
			af.where     = WHERE_WEAPON;
			af.type      = gsn_poison;
			af.level     = ch->level;
			af.duration  = ch->level / 4;
			af.location  = APPLY_NONE;
			af.modifier  = 0;
			af.bitvector = WEAPON_POISON;
			affect_to_obj(obj,&af);
 
			act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM);
			act("You coat $p with venom.",ch,obj,NULL,TO_CHAR);
			check_improve(ch,gsn_envenom,true,3);
			WAIT_STATE(ch,skill_table[gsn_envenom].beats);
			return;
		}
		else
		{
			act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
			check_improve(ch,gsn_envenom,false,3);
		    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
			return;
		}
	}
 	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
	return;
}

/**************************************************************************/
void do_fill( char_data *ch, char *argument )
{
    char arg[MIL];
    char buf[MSL];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;
	
    one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println("Fill what?");
		return;
    }
	
    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
		ch->println("You do not have that item.");
		return;
    }
	
    found = false;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
		if ( fountain->item_type == ITEM_FOUNTAIN )
		{
			found = true;
			break;
		}
	}
	
    if ( !found )
    {
		ch->println("There is no fountain here!");
		return;
    }

	// if the fountain is empty you shouldn't be able to fill a drinkcontainer
	if ( fountain->value[1] == 0 )
	{
		act( "$p is empty.", ch, fountain, NULL, TO_CHAR );
		return;
	}

    if ( obj->item_type != ITEM_DRINK_CON )
    {
		ch->println("You can't fill that.");
		return;
    }
	
    if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
    {
		ch->println("There is already another liquid in it.");
		return;
    }
	
    if ( obj->value[1] >= obj->value[0] )
	{
		ch->println("Your container is full.");
		return;
    }

	// decrement the fountain by the fill amount 
	// if it isn't an infinite fountain
	if(fountain->value[1]>=0){
		fountain->value[1] -= (obj->value[0] - obj->value[1]);

		// in case -1 is unlimited etc
		if ( fountain->value[1] < 0 ){
			fountain->value[1] = 0;
		}
	}


    sprintf(buf,"You fill $p with %s from $P.",
		liq_table[fountain->value[2]].liq_name);
    act( buf, ch, obj,fountain, TO_CHAR );
    sprintf(buf,"$n fills $p with %s from $P.",
		liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];

    return;
}

/**************************************************************************/
void do_pour (char_data *ch, char *argument)
{
    char arg[MSL],buf[MSL], outbuf[MSL];
    OBJ_DATA *out, *in;
	char_data *vch = NULL;
	int amount;
	
    argument = one_argument(argument,arg);
    
    if (IS_NULLSTR(arg)|| IS_NULLSTR(argument))
    {
		ch->println("Pour what into what?");
		ch->println("Note: to empty a liquid container type `=Cempty <container_name>`x");
		return;
    }
    
	
    if ((out = get_obj_carry(ch,arg)) == NULL)
    {
		ch->println("You don't have that item." );
		return;
    }
	
	if (out->item_type != ITEM_DRINK_CON)
    {
		ch->println("That's not a drink container." );
		return;
	}
	
	strcpy(outbuf,argument);
	one_argument(outbuf,outbuf);
    if (!str_cmp(outbuf,"out"))
    {
		if (out->value[1] == 0)
		{
			ch->println("It's already empty." );
			return;
		}
		
		out->value[1] = 0;
		out->value[3] = 0;
		sprintf(buf,"You invert $p, spilling %s all over the ground.",
			liq_table[out->value[2]].liq_name);
		act(buf,ch,out,NULL,TO_CHAR);
		
		sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
			liq_table[out->value[2]].liq_name);
		act(buf,ch,out,NULL,TO_ROOM);
		return;
    }
	
	if ((in = get_obj_here(ch,argument)) == NULL)
    {
		vch = get_char_room(ch,argument);
		
		if (vch == NULL)
		{
			ch->println("Pour into what?" );
			return;
		}
		
		in = get_eq_char(vch,WEAR_HOLD);
		
		if (in == NULL)
		{
			ch->println("They aren't holding anything." );
			return;
		}
    }
	
    if (in->item_type != ITEM_DRINK_CON)
	{
		ch->println("You can only pour into other drink containers." );
		return;
    }
    
    if (in == out)
    {
		ch->println("You cannot change the laws of physics!");
		return;
    }
	
    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
		ch->println("They don't hold the same liquid.");
		return;
    }
	
    if (out->value[1] == 0)
    {
		act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
		return;
	}
	
    if (in->value[1] >= in->value[0])
    {
		act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
		return;
    }
	
    amount = UMIN(out->value[1],in->value[0] - in->value[1]);
	
    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];
    
    if (vch == NULL)
    {
		sprintf(buf,"You pour %s from $p into $P.",
			liq_table[out->value[2]].liq_name);
		act(buf,ch,out,in,TO_CHAR);
		sprintf(buf,"$n pours %s from $p into $P.",
			liq_table[out->value[2]].liq_name);
		act(buf,ch,out,in,TO_ROOM);
	}
	else
    {
        sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR);
		sprintf(buf,"$n pours you some %s.",
			liq_table[out->value[2]].liq_name);
		act(buf,ch,NULL,vch,TO_VICT);
        sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT);
		
    }
}

/**************************************************************************/
void do_drink( char_data *ch, char *argument )
{
    char arg[MIL];
	OBJ_DATA *obj;
	int amount;
	int liquid;
	int max_liq;


	for(max_liq=0; !IS_NULLSTR(liq_table[max_liq].liq_name); max_liq++){		
	}
	max_liq--;
	
    one_argument( argument, arg );
	
    if ( arg[0] == '\0' )
    {
		for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
		{
			if ( obj->item_type == ITEM_FOUNTAIN )
				break;
		}
		
		if ( obj == NULL )
		{
			ch->println( "Drink what?" );
			return;
		}
    }
    else
    {
		if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
		{
			ch->println( "You can't find it." );
			return;
		}
    }
	
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
		ch->println( "You fail to reach your mouth.  *Hic*" );
		return;
    }
	
    switch ( obj->item_type )
	{
    default:
		ch->println( "You can't drink from that." );
		return;
		
    case ITEM_FOUNTAIN:
	case ITEM_DRINK_CON:
		if ( obj->value[1]== 0 ){
			ch->println( "It is already empty." );
			return;
		}

		liquid = obj->value[2];
        if ( liquid < 0 || liquid>max_liq)
		{
			bugf( "Do_drink: bad liquid number %d.", liquid );
			autonote(NOTE_SNOTE, "do_drink()", "bad liquid number", "imm",
				FORMATF("Do_drink: bad liquid number %d, on object %d (%s), value reset to 0..", 
					liquid, obj->pIndexData?obj->pIndexData->vnum:0, obj->short_descr ), true);
            liquid = obj->value[2] = 0;
        }
		amount = liq_table[liquid].liq_affect[4];
		if(obj->item_type==ITEM_FOUNTAIN){ // naturally drink more from a fountain
			amount*=3;
		}		
		if(obj->value[1]>=0){
			amount = UMIN(amount, obj->value[1]);
		}
		break;
		
	}
	
    if (!IS_NPC(ch) && !IS_IMMORTAL(ch) 
		&&  ch->pcdata->condition[COND_FULL] > 45)
    {
		ch->println( "You're too full to drink more." );
		return;
	}
	
    act( "$n drinks $T from $p.",
		ch, obj, liq_table[liquid].liq_name, TO_ROOM );
    act( "You drink $T from $p.",
		ch, obj, liq_table[liquid].liq_name, TO_CHAR );
	
    gain_condition( ch, COND_DRUNK,
		amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
    gain_condition( ch, COND_FULL,
		amount * liq_table[liquid].liq_affect[COND_FULL] / 7 );
    gain_condition( ch, COND_THIRST,
		amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 );
	gain_condition(ch, COND_HUNGER,
		amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2 );
	
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
		ch->println("You feel drunk.");
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
		ch->println("You are full.");
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
		ch->println("Your thirst is quenched.");
	
    if ( obj->value[3] != 0 
		&& !HAS_CLASSFLAG(ch, CLASSFLAG_POISON_IMMUNITY)
		)
    {
		// The drink was poisoned!
		AFFECT_DATA af;		
		
		act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
		ch->println("You choke and gag.");
		af.where     = WHERE_AFFECTS;
		af.type      = gsn_poison;
		af.level	 = number_fuzzy(amount); 
		af.duration  = 3 * amount;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_POISON;
		affect_join( ch, &af );
	}
	
	// containers with -1 are infinite, so don't remove the amount.
    if(obj->value[0]>0 && obj->value[1]>0){
        obj->value[1] -= amount;
		// remove the poison if it is emptied
		if(obj->value[1]<=0){
			obj->value[3]=0;
			obj->value[1]=0;
		}
	}
}

/**************************************************************************/
void do_eat( char_data *ch, char *argument )
{
	char arg[MIL];
    OBJ_DATA *obj;
	
	one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{
		ch->println("Eat what?");
		return;
    }
	
    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
		ch->println("You do not have that item.");
		return;
    }
	
    if ( ch->level<LEVEL_IMMORTAL )
    {
        if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
        {
            ch->println("That's not edible.");
            return;
        }
		
        if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 
			&& obj->value[0]!=0 && obj->value[1]!=0 )
        {   
			ch->println("You are too full to eat more.");
            return;
        }
    }
	
    act( "$n eats $p.", ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );
	
    switch ( obj->item_type )
    {
		
    case ITEM_FOOD:
		if ( !IS_NPC(ch) )
		{
			int condition;
			
			condition = ch->pcdata->condition[COND_HUNGER];
			gain_condition( ch, COND_HUNGER, obj->value[0] );
			gain_condition( ch, COND_FULL, obj->value[1]);
			if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0 )
				ch->println("You are no longer hungry.");
			else if ( ch->pcdata->condition[COND_FULL] > 40 )
				ch->println("You are full.");
		}
		
		if ( obj->value[3] != 0 
			&& !HAS_CLASSFLAG(ch, CLASSFLAG_POISON_IMMUNITY))
		{
			// The food was poisoned!- do a saving throw
			if ( saves_spell( number_fuzzy(obj->value[0])*2, ch, DAM_POISON))
			{
				act( "$n turns pale for a moment.", ch, 0, 0, TO_ROOM );
				ch->println("You feel a little sick for a moment but it passes.");
			}else{
				AFFECT_DATA af;
				
				act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
				ch->println("You choke and gag.");
				
				af.where	 = WHERE_AFFECTS;
				af.type      = gsn_poison;
				af.level 	 = number_fuzzy(obj->value[0]);
				af.duration  = 2 * obj->value[0];
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bitvector = AFF_POISON;
				affect_join( ch, &af );
			}
		}
		break;
		
    case ITEM_PILL:

		if ( IS_SET( ch->in_room->room_flags, ROOM_ANTIMAGIC )) {
			ch->println("Nothing seemed to happen.");
			extract_obj( obj );
			return;
		}


        obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
        obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
        obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
        obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );

		 if (ch->fighting){
			WAIT_STATE( ch, 2*PULSE_VIOLENCE );
		 }else{
			WAIT_STATE( ch, 3*PULSE_VIOLENCE/2 );
		 }
        break;
    }
	
    extract_obj( obj );
    return;
}


/**************************************************************************/
// Remove an object, return true if object in wear location has been removed
bool remove_obj( char_data *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return true;

    if ( !fReplace )
	return false;

	if(oprog_execute_if_appropriate(obj, ch, OTRIG_REMOVE_PRE)){
		return false; // mudprog stopped the removal
	}

    if ( !IS_IMMORTAL(ch) && IS_SET(obj->extra_flags, OBJEXTRA_NOREMOVE) )
    {
		act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
		return false;
    }

	if( IS_SET( obj->extra2_flags, OBJEXTRA2_BLESSED_IN_HOLY_HANDS )
	&& HAS_CLASSFLAG(ch, CLASSFLAG_HOLY_CLASS))
	{
		REMOVE_BIT( obj->extra_flags, OBJEXTRA_BLESS );
	}

	if (obj->wear_loc==WEAR_SECONDARY){
		act( "You stop using $p as your offhand weapon.", ch, obj, NULL, TO_CHAR );
	}else if (!IS_SET(obj->extra_flags,OBJEXTRA_LODGED)){ 
		act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
	}

	if ( IS_SET( obj->extra_flags, OBJEXTRA_LODGED ))
	{
		REMOVE_BIT( obj->extra_flags, OBJEXTRA_LODGED );
		if ( iWear == WEAR_LODGED_ARM)
		{
			act( "With a tug you dislodge $p from your arm.", ch, obj, NULL, TO_CHAR );
			act( "With a tug, $n dislodges $p from $s arm.", ch, obj, NULL, TO_ROOM );
			ch->hit -= obj->level / 4;
			obj->wear_loc = -1;
			oprog_execute_if_appropriate(obj, ch, OTRIG_REMOVE_POST);
			return true;
		}
		else if ( iWear == WEAR_LODGED_LEG )
		{
			act( "With a good yank you dislodge $p from your leg.", ch, obj, NULL, TO_CHAR );
			act( "With a good yank, $n dislodges $p from $s leg.", ch, obj, NULL, TO_ROOM );
			ch->hit  -= obj->level / 5;
			ch->move -= obj->level / 2;
			obj->wear_loc = -1;
			oprog_execute_if_appropriate(obj, ch, OTRIG_REMOVE_POST);
			return true;
		}
		else if ( iWear == WEAR_LODGED_RIB )
		{
			act( "With a wrenching pull you dislodge $p from your chest.", ch, obj, NULL, TO_CHAR );
			act( "With a wrenching pull, $n dislodges $p from $s chest.", ch, obj, NULL, TO_ROOM );
			ch->hit  -= obj->level / 2;
			obj->wear_loc = -1;
			oprog_execute_if_appropriate(obj, ch, OTRIG_REMOVE_POST);
			return true;
		}
	}else{
		act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
	}

    unequip_char( ch, obj );
	oprog_execute_if_appropriate(obj, ch, OTRIG_REMOVE_POST);
    return true;
}


/**************************************************************************/
/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( char_data *ch, OBJ_DATA *obj, bool fReplace, bool hold )
{
	obj_data *existing_primary_position; // used for object wearing preference system 
	// --- object wearing preference system works as follows:
	// if 'ch' is wearing 2 items in a particular wear location 
	// (e.g. WEAR_FINGER_L, WEAR_FINGER_R), and try to wear an object 
	// into that wear location, the code will favour replacing the item 
	// which doesn't have the same vnum as the object about to be put on.
	// The effect of this, is you can type 'wear golden', 'wear golden'...
	// and you would replace both of your rings with golden rings...
	// instead of the second 'wear golden' replacing what you just
	// wore using the first 'wear golden'.
	// - Kal, Sept 03


	if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_PRE))return;

	// do check if objects level is LEVEL_IMMORTAL or higher, and it is a
	// mortal trying to wear the object, remove it from them.
	if (( obj->level>LEVEL_HERO)&&(ch->level < LEVEL_IMMORTAL))
	{
		char buf[MSL];
		
		sprintf( buf, "`x%s (%s) lvl %d`1tried to wear `1%s [%d] lvl %d`1It was removed since it was out of level.",
			ch->name,		ch->short_descr,	ch->level, 
			obj->short_descr,	obj->pIndexData->vnum, obj->level);
		autonote(NOTE_SNOTE, "p_anote()","`Yoverlevel object!`x", "imm", buf, true);
		
		ch->printlnf( "You try to use %s and it crumbles to dust.", obj->short_descr );
		
		act( "$n tries to use $p, but it crumbles to dust.",
			ch, obj, NULL, TO_ROOM );
		
		extract_obj(obj);
		return;
	}
	
	if ( IS_SET( obj->attune_flags, ATTUNE_NEED_TO_USE ))
	{
		if ( ch->player_id != obj->attune_id )
		{
			act( "You cannot use $p until you have attuned yourself to it.", ch, obj, NULL, TO_CHAR );
			return;
		}
	}
	
    if(IS_SET(obj->extra_flags, OBJEXTRA_MAGIC)
		&& HAS_CLASSFLAG(ch, CLASSFLAG_MAGIC_ANTIPATHY))
    {
		act("You have great antipathy towards $p.",ch,obj,NULL,TO_CHAR);
		return;
    }
	
	if ( !IS_ADMIN( ch ))  // Immortal requested not to have this but I wanted it :)
	{
		if(!IS_NPC(ch) && obj->pIndexData->relative_size<
			race_table[ch->race]->low_size &&
			obj->item_type != ITEM_LIGHT &&
			!CAN_WEAR( obj, OBJWEAR_HOLD)  &&
			!CAN_WEAR( obj, OBJWEAR_FLOAT) )
		{
			act("$p is too small for you.",ch,obj,NULL,TO_CHAR);
			return;
		}
		
		
		if(!IS_NPC(ch) && obj->pIndexData->relative_size>
			race_table[ch->race]->high_size &&
			obj->item_type != ITEM_LIGHT &&
			!CAN_WEAR( obj, OBJWEAR_FLOAT) )
		{
			act("$p is too large for you.", ch, obj, NULL, TO_CHAR);
			return;
		}
	}
	
	if ( !IS_IMMORTAL( ch ))
	{
		if(DISALLOWED_OBJECT_FOR_CHAR(obj, ch))
		{
			act("$p cannot be used by your class.",ch,obj,NULL,TO_CHAR); 
			return;
		} 
	}
	
	if ( obj->item_type == ITEM_LIGHT )
	{
		if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;

		act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
		act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_LIGHT );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}

	if( GAMESETTINGMF1( GAMESETMF1_PENDANTS_ENABLED ) ){
		if ( CAN_WEAR( obj, OBJWEAR_PENDANT ) )
		{
			if ( !remove_obj( ch, WEAR_PENDANT, fReplace ) )
				return;
			
			if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
			
			act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
			act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
			equip_char( ch, obj, WEAR_PENDANT );
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_FINGER ) )
	{
		// remove something from the fingers to make way for the item about to go on if necessary
		existing_primary_position=get_eq_char( ch, WEAR_FINGER_L );
		if ( existing_primary_position && get_eq_char( ch, WEAR_FINGER_R ))
		{
			if(existing_primary_position->pIndexData  
				&& obj->pIndexData
				&& existing_primary_position->pIndexData->vnum==obj->pIndexData->vnum)
			{
				// favour replacing the secondary position item over doing an exact swap
				if(   !remove_obj( ch, WEAR_FINGER_R, fReplace )
					&&!remove_obj( ch, WEAR_FINGER_L, fReplace ) ){
					return;
				}
			}else{
				if(   !remove_obj( ch, WEAR_FINGER_L, fReplace )
					&&!remove_obj( ch, WEAR_FINGER_R, fReplace ) ){
					return;
				}
			}
		}

		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;

		if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
		{
			act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
			act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
			equip_char( ch, obj, WEAR_FINGER_L );
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
		{
			act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
			act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
			equip_char( ch, obj, WEAR_FINGER_R );
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		bug( "Wear_obj: no free finger.");
		ch->println("You already wear two rings.");
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_NECK ) )
	{
		// remove something from the neck to make way for the item about to go on if necessary
		existing_primary_position=get_eq_char( ch, WEAR_NECK_1 );
		if ( existing_primary_position && get_eq_char( ch, WEAR_NECK_2 ))
		{
			if(existing_primary_position->pIndexData  
				&& obj->pIndexData
				&& existing_primary_position->pIndexData->vnum==obj->pIndexData->vnum)
			{
				// favour replacing the secondary position item over doing an exact swap
				if(   !remove_obj( ch, WEAR_NECK_2, fReplace )
					&&!remove_obj( ch, WEAR_NECK_1, fReplace ) ){
					return;
				}
			}else{
				if(   !remove_obj( ch, WEAR_NECK_1, fReplace )
					&&!remove_obj( ch, WEAR_NECK_2, fReplace ) ){
					return;
				}
			}
		}

		// put the item onto the free wear position
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;

		if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
		{
			act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
			act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
			equip_char( ch, obj, WEAR_NECK_1 );
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
		{
			act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
			act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
			equip_char( ch, obj, WEAR_NECK_2 );
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		bug("Wear_obj: no free neck.");
		ch->println("You already wear two neck items.");
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_TORSO ) )
	{
		if ( !remove_obj( ch, WEAR_TORSO, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;

		act( "$n wears $p on $s torso.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your torso.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_TORSO );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_HEAD ) )
	{
		if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;

		act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_HEAD );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_LEGS ) )
	{
		if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_LEGS );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	//////////////
	if (CAN_WEAR(obj, OBJWEAR_EYES))
	{
		if (!remove_obj (ch, WEAR_EYES, fReplace)){
			return;
		}
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p on $s eyes.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your eyes.", ch, obj, NULL, TO_CHAR );
		equip_char (ch, obj, WEAR_EYES);
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	
	if (CAN_WEAR(obj, OBJWEAR_EAR))
	{
		// remove something from the ears to make way for the item about to go on if necessary
		existing_primary_position=get_eq_char( ch, WEAR_EAR_L );
		if ( existing_primary_position && get_eq_char( ch, WEAR_EAR_R ))
		{
			if(existing_primary_position->pIndexData  
				&& obj->pIndexData
				&& existing_primary_position->pIndexData->vnum==obj->pIndexData->vnum)
			{
				// favour replacing the secondary position item over doing an exact swap
				if(   !remove_obj( ch, WEAR_EAR_R, fReplace )
					&&!remove_obj( ch, WEAR_EAR_L, fReplace ) ){
					return;
				}
			}else{
				if(   !remove_obj( ch, WEAR_EAR_L, fReplace )
					&&!remove_obj( ch, WEAR_EAR_R, fReplace ) ){
					return;
				}
			}
		}

		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		if (get_eq_char (ch, WEAR_EAR_L) == NULL)
		{
			act( "$n wears $p on $s left ear.",   ch, obj, NULL, TO_ROOM );
			act( "You wear $p on your left ear.", ch, obj, NULL, TO_CHAR );
			equip_char (ch, obj, WEAR_EAR_L);
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		if (get_eq_char (ch, WEAR_EAR_R) == NULL)
		{
			act( "$n wears $p on $s right ear.",   ch, obj, NULL, TO_ROOM );
			act( "You wear $p on your right ear.", ch, obj, NULL, TO_CHAR );
			equip_char (ch, obj, WEAR_EAR_R);
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		bug("Wear_obj: no free ear.");
		ch->println("You already wear two ear rings.");
		return;
	}
    
	if (CAN_WEAR(obj, OBJWEAR_FACE))
	{
		if (!remove_obj (ch, WEAR_FACE, fReplace))
			return;		
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p on $s face.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your face.", ch, obj, NULL, TO_CHAR );
		equip_char (ch, obj, WEAR_FACE);
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}

	if ( CAN_WEAR( obj, OBJWEAR_BACK ) )
	{
		if ( !remove_obj( ch, WEAR_BACK, fReplace ) )
		return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p on $s back.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your back.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_BACK );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if (CAN_WEAR(obj, OBJWEAR_ANKLE))
	{
		// remove something from the ankles to make way for the item about to go on if necessary
		existing_primary_position=get_eq_char( ch, WEAR_ANKLE_L );
		if ( existing_primary_position && get_eq_char( ch, WEAR_ANKLE_R ))
		{
			if(existing_primary_position->pIndexData  
				&& obj->pIndexData
				&& existing_primary_position->pIndexData->vnum==obj->pIndexData->vnum)
			{
				// favour replacing the secondary position item over doing an exact swap
				if(   !remove_obj( ch, WEAR_ANKLE_R, fReplace )
					&&!remove_obj( ch, WEAR_ANKLE_L, fReplace ) ){
					return;
				}
			}else{
				if(   !remove_obj( ch, WEAR_ANKLE_L, fReplace )
					&&!remove_obj( ch, WEAR_ANKLE_R, fReplace ) ){
					return;
				}
			}
		}	
		
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		if (get_eq_char (ch, WEAR_ANKLE_L) == NULL)
		{
			act( "$n wears $p on $s left ankle.",   ch, obj, NULL, TO_ROOM );
			act( "You wear $p on your left ankle.", ch, obj, NULL, TO_CHAR );
			equip_char (ch, obj, WEAR_ANKLE_L);
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		if (get_eq_char (ch, WEAR_ANKLE_R) == NULL)
		{
			act( "$n wears $p on $s right ankle.",   ch, obj, NULL, TO_ROOM );
			act( "You wear $p on your right ankle.", ch, obj, NULL, TO_CHAR );
			equip_char (ch, obj, WEAR_ANKLE_R);
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		bug("Wear_obj: no free ankle.");
		ch->println("You already wear items on both your ankles.");
		return;
	}
	//////////////
	
	if ( CAN_WEAR( obj, OBJWEAR_FEET ) )
	{
		if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_FEET );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_HANDS ) )
	{
		if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_HANDS );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_ARMS ) )
	{
		if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_ARMS );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_ABOUT ) )
	{
		if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p about $s torso.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p about your torso.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_ABOUT );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_WAIST ) )
	{
		if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
		act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_WAIST );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_WRIST ) )
	{
		// remove something from the wrists to make way for the item about to go on if necessary
		existing_primary_position=get_eq_char( ch, WEAR_WRIST_L );
		if ( existing_primary_position && get_eq_char( ch, WEAR_WRIST_R ))
		{
			if(existing_primary_position->pIndexData  
				&& obj->pIndexData
				&& existing_primary_position->pIndexData->vnum==obj->pIndexData->vnum)
			{
				// favour replacing the secondary position item over doing an exact swap
				if(   !remove_obj( ch, WEAR_WRIST_R, fReplace )
					&&!remove_obj( ch, WEAR_WRIST_L, fReplace ) ){
					return;
				}
			}else{
				if(   !remove_obj( ch, WEAR_WRIST_L, fReplace )
					&&!remove_obj( ch, WEAR_WRIST_R, fReplace ) ){
					return;
				}
			}
		}	
		
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
		{
			act( "$n wears $p around $s left wrist.",
				ch, obj, NULL, TO_ROOM );
			act( "You wear $p around your left wrist.",
				ch, obj, NULL, TO_CHAR );
			equip_char( ch, obj, WEAR_WRIST_L );
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
		{
			act( "$n wears $p around $s right wrist.",
				ch, obj, NULL, TO_ROOM );
			act( "You wear $p around your right wrist.",
				ch, obj, NULL, TO_CHAR );
			equip_char( ch, obj, WEAR_WRIST_R );
			oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
			return;
		}
		
		bug("Wear_obj: no free wrist.");
		ch->println("You already wear two wrist items.");
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_SHIELD ) )
	{
		OBJ_DATA *weapon;
		
		if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
			return;
		
		weapon = get_eq_char(ch,WEAR_WIELD);
		if (weapon != NULL && ch->size < SIZE_LARGE
			&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS))
		{
			ch->println("Your hands are tied up with your two-handed weapon!");
			return;
		}
		
		if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
		{
			if (hold){		
				if ( !remove_obj( ch, WEAR_SECONDARY, true) ){
					return;
				}
			}else{
				ch->println("You cannot use a shield while using 2 weapons.");
				ch->println("(Try using hold instead of wear to automatically remove your offhand weapon.)");
				return;
			}
		}						
		
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
		act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_SHIELD );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_WIELD ) )
	{
		int sn,skill;
		
		if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
			return;
		
		if ( !IS_NPC(ch) 
			&& get_obj_weight(obj) > ((ch->modifiers[STAT_ST] + 20) * 10 ))  
			
		{
			ch->println("It is too heavy for you to wield.");
			return;
		}
		
		if (!IS_NPC(ch) && ch->size < SIZE_LARGE
			&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
			&&  ((get_eq_char(ch,WEAR_SHIELD) != NULL)
			||  (get_eq_char(ch,WEAR_SECONDARY) != NULL)))
		{
			ch->println("You need two hands free for that weapon.");
			return;
		}
		
		if ( IS_SET( obj->extra2_flags, OBJEXTRA2_NOPRIMARY )) {
			act( "$p is designed to be wielded in your offhand.", ch, obj, NULL, TO_CHAR );
			return;
		}
		
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
 		act( "You wield $p.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_WIELD );
		
		if( IS_SET( obj->extra2_flags, OBJEXTRA2_BLESSED_IN_HOLY_HANDS )
			&& HAS_CLASSFLAG(ch, CLASSFLAG_HOLY_CLASS)
			&& !(IS_OBJ_STAT(obj,OBJEXTRA_BLESS)) )
		{
			SET_BIT(obj->extra_flags, OBJEXTRA_BLESS);
		}
		
		sn = get_weapon_sn(ch);
		
		if (sn == gsn_hand_to_hand)
			return;
		
		skill = get_weapon_skill(ch,sn);

		if (skill >= 100
			|| ( get_skill( ch, gsn_blade_mastery )
				&& sn == gsn_sword )
			|| ( get_skill( ch, gsn_arms_mastery ) )
			|| ( get_skill( ch, gsn_dagger_mastery )
				&& sn == gsn_dagger ) 
				)
			act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
		else if (skill > 85
			|| ch->clss == class_lookup("citizen"))
			act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
		else if (skill > 70)
			act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
		else if (skill > 50)
			act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
		else if (skill > 25)
			act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
		else if (skill > 1)
			act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
		else
			act("You don't even know which end is up on $p.",
			ch,obj,NULL,TO_CHAR);
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_HOLD ) )
	{
		if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
		{
			if (hold){		
				if ( !remove_obj( ch, WEAR_SECONDARY, true) ){
					return;
				}
			}else{
				ch->println("You cannot hold an item while using 2 weapons.");
				ch->println("(Try using hold instead of wear to automatically remove your offhand weapon.)");
				return;
			}
		}
		
		if ( !remove_obj( ch, WEAR_HOLD, fReplace ) ){
			return;
		}
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act( "$n holds $p in $s hand.",   ch, obj, NULL, TO_ROOM );
		act( "You hold $p in your hand.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, WEAR_HOLD );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR(obj,OBJWEAR_FLOAT) )
	{
		if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM);
		act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR);
		equip_char(ch,obj,WEAR_FLOAT);
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_LODGED_ARM ) )
	{
		if ( !remove_obj( ch, WEAR_LODGED_ARM, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		equip_char( ch, obj, WEAR_LODGED_ARM );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_LODGED_LEG ) )
	{
		if ( !remove_obj( ch, WEAR_LODGED_LEG, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		equip_char( ch, obj, WEAR_LODGED_LEG );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( CAN_WEAR( obj, OBJWEAR_LODGED_RIB ) )
	{
		if ( !remove_obj( ch, WEAR_LODGED_RIB, fReplace ) )
			return;
		if(oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_MID))return;
		equip_char( ch, obj, WEAR_LODGED_RIB );
		oprog_execute_if_appropriate(obj, ch, OTRIG_WEAR_POST);
		return;
	}
	
	if ( fReplace ){
		ch->println("You can't wear, wield, or hold that.");
	}
	
	return;
}

/**************************************************************************/
void do_wear( char_data *ch, char *argument )
{
	char arg[MIL];
	OBJ_DATA *obj;
	
	one_argument( argument, arg );
	
	if ( IS_NULLSTR(arg) )
	{
		ch->println( "Wear, wield what?" );
		return;
	}
	
	if ( !str_cmp( arg, "all" ) )
	{
		OBJ_DATA *obj_next;
		
		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;
			if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) ){
				wear_obj ( ch, obj, false, false );
			}
		}
		return;
	}

	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL ){
		ch->println( "You do not have that item." );
		return;
	}		
	wear_obj( ch, obj, true, false );
	
    return;
}

/**************************************************************************/
void do_hold( char_data *ch, char *argument )
{
	char arg[MIL];
	OBJ_DATA *obj;
	
	one_argument( argument, arg );
	
	if ( IS_NULLSTR(arg)){
		ch->println( "Hold what?" );
		return;
	}

	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL ){
		ch->println( "You do not have that item." );
		return;
	}		

	wear_obj( ch, obj, true, true );
	
    return;
}


/**************************************************************************/
void do_remove( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
	
    one_argument( argument, arg );
	
    if ( arg[0] == '\0' )
    {
		ch->println("Remove what?");
		return;
    }
	
    if (!str_cmp(arg, "all"))
    {
		// PC being ordered to do this
		if ( !IS_NPC(ch) && IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
		{
			if ( ch->master )
				ch->println("Not going to happen.");
			return;
		}

        bool found =false;
        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        {
            if ( obj->wear_loc != WEAR_NONE
				&& obj->wear_loc != WEAR_PENDANT // "remove all" doesn't affect pendants
				&&   can_see_obj( ch, obj ) )
            {
				if (!found)
					found = true;
				if ( obj->wear_loc == WEAR_SHEATHED
				||	 obj->wear_loc == WEAR_CONCEALED )
				{
					obj->wear_loc = -1;
				    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
					act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
				}
				else
				{
					remove_obj( ch, obj->wear_loc, true );
				}
				WAIT_STATE(ch, 1);
				if (ch->fighting)
					WAIT_STATE(ch, 5);
            }
        }
        if (!found)
        {
            ch->println("You are not wearing anything.");
        }
        return;
    }
	
    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
	{
		ch->println("You do not have that item.");
		return;
    }

	if ( obj->wear_loc == WEAR_SHEATHED
	||	 obj->wear_loc == WEAR_CONCEALED )
	{
		obj->wear_loc = -1;
	    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
		act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
	}else{
		remove_obj( ch, obj->wear_loc, true );
	}

	return;
}

/**************************************************************************/
void do_quaff( char_data *ch, char *argument )
{
    char arg[MIL];
	OBJ_DATA *obj;
	
    one_argument( argument, arg );
	
	if ( IS_OOC( ch )) {
		ch->println("Not in an OOC room.");
		return;
	}
	
    if ( arg[0] == '\0' )
    {
		ch->println("Quaff what?");
		return;
    }
	
    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
		ch->println("You do not have that potion.");
		return;
    }
	
    if ( obj->item_type != ITEM_POTION )
    {
		ch->println("You can quaff only potions.");
		return;
	}
	
	if ( IS_SET( ch->in_room->room_flags, ROOM_ANTIMAGIC ))
	{
		act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
		act( "You quaff $p but nothing seems to happen.", ch, obj, NULL ,TO_CHAR );
		extract_obj( obj );
		return;
	}

	// Added so that barbarians have a good chance of 'fearing magic'. - Tib
	if(IS_SET(obj->extra_flags, OBJEXTRA_MAGIC)
		&& HAS_CLASSFLAG(ch, CLASSFLAG_MAGIC_ANTIPATHY) && number_range(1,3)==1)
	{
		if ( IS_AFFECTED( ch, AFF_FEAR ) || IS_AFFECTED2( ch, AFF2_FEAR_MAGIC) )
		{
			ch->println("The potion shatters on the ground, falling from your shaking hands.");
			extract_obj( obj );
			return;
		}

		act( "$n quaffs $p and screams in terror!", ch, obj, NULL, TO_ROOM );
		act( "You quaff $p and scream as you realize the potion was magic.", 
			ch, obj, NULL ,TO_CHAR );
		msp_to_room(MSPT_ACTION, MSP_SOUND_QUAFF, 0, ch, false, true);
		spell_fear_magic( gsn_fear_magic, UMIN(250, (obj->level*5)), ch, ch, 0);
		extract_obj( obj );
		return;
	}

	if ((ch->level < obj->level)&&(obj->level>10))
	{
		ch->println("This liquid is too powerful for you to drink.");
		return;
	}
	
	act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
	act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );


	msp_to_room(MSPT_ACTION, MSP_SOUND_QUAFF, 
					0,
					ch,
					false,
					true);	
	
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
	
	if (ch->fighting){
		WAIT_STATE( ch, 2*PULSE_VIOLENCE );
	}else{
		WAIT_STATE( ch, 3*PULSE_VIOLENCE/2 );
	}
	
	
	extract_obj( obj );
	return;
}

/**************************************************************************/
void do_recite( char_data *ch, char *argument )
{
	char arg1[MIL];
	char arg2[MIL];
	char_data *victim;
	OBJ_DATA *scroll;
	OBJ_DATA *obj;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		ch->println( "You do not have that scroll." );
		return;
	}

	if(IS_AFFECTED( ch, AFF_BLIND ) ){
		ch->println( "You can't recite scrolls while blind." );
		return;
	}

	if ( scroll->item_type != ITEM_SCROLL )
	{
		ch->println( "You can recite only scrolls." );
		return;
	}

	if (( ch->level < scroll->level)&&(scroll->level>10))
	{
		ch->println( "This scroll is too complex for you to comprehend." );
		return;
	}

	obj = NULL;
	if ( arg2[0] == '\0' )
	{
		victim = ch;
	}
	else
	{
		if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
			&& ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
		{
			ch->println("You can't find it.");
			return;
		}
	}

	act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
	act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

	if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
	{
		ch->println("You mispronounce a syllable.");
		check_improve(ch,gsn_scrolls,false,2);
	}
	else
	{
		obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
		obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
		obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
		obj_cast_spell( scroll->value[4], scroll->value[0], ch, victim, obj );
		check_improve(ch,gsn_scrolls,true,2);
	}

	extract_obj( scroll );
	return;
}


/**************************************************************************/
void do_brandish( char_data *ch, char *argument )
{
    OBJ_DATA  *staff;
    int sn;

    // new stuff here 
    char arg2[MIL];
    char_data *victim;
    OBJ_DATA *obj;
    void *vo;
    int target;

    strcpy(arg2, argument);

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
        ch->println("You hold nothing in your hands to brandish.");
        return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
        ch->println("You can brandish only with a staff.");
        return;
    }

	if ( IS_SET( ch->in_room->room_flags, ROOM_ANTIMAGIC )) {
		act( "Your $p doesn't seem to have any power.", ch, staff, NULL, TO_CHAR );
		return;
	}

    if (( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
        bugf( "do_brandish(): bad sn %d.", sn );
        return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if (staff->value[2] < -1)
    {
        staff->value[2] = -1;
    }

    if (( staff->value[2] > 0 ) || (staff->value[2] = -1))
    {
        act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
        act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );

        if ( ch->level < staff->level
        ||  (get_skill(ch,gsn_staves)==0)
        ||   number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
        {
            act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
            act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
            check_improve(ch,gsn_staves,false,2);
        }    
        else // cast the spell 
        {   
            // Locate targets.
            victim  = NULL;
            obj     = NULL;
            vo      = NULL;
            target  = TARGET_NONE;
              
            switch ( skill_table[sn].target )
            {
            default:
                bugf( "do_cast(): bad target for sn %d.", sn );
                return;
        
            case TAR_IGNORE:
                break;
        
            case TAR_MOB_OFFENSIVE:
            case TAR_CHAR_OFFENSIVE:
                if ( arg2[0] == '\0' )
                {
                    if ( ( victim = ch->fighting ) == NULL )
                    {
                        ch->println("Brandish the object while aiming it towards whom?");
                        return;
                    }
                }
                else
                {
                    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
                    {
                        ch->println("They aren't here.");
                        return;
                    }
                }
           
                if ( !IS_NPC(ch) )
                {
            
                    if (is_safe(ch,victim) && victim != ch)
                    {
                        ch->println("Not on that target.");
                        return;
                    }
                }
            
                if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
                {
                    ch->println("You can't do that on your own follower.");
                    return;
                }
            
                vo = (void *) victim;
                target = TARGET_CHAR;
                break;
            
            case TAR_CHAR_DEFENSIVE:
                if ( arg2[0] == '\0' )
                {
                    victim = ch;
                }
                else
                {
                    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
                    {
                    ch->println("They aren't here.");
                    return;
                    }
                }
            
                vo = (void *) victim;
                target = TARGET_CHAR;
                break;
            
            case TAR_CHAR_SELF:
                if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) )
                {
                    ch->println("You can't brandish this object while aiming it at another.");
                    return;
                }
            
                vo = (void *) ch;
                target = TARGET_CHAR;
                break;
            
            case TAR_OBJ_INV:
                if ( arg2[0] == '\0' )
                {
                    ch->println("You must brandish the object towards another object?");
                     return;
                }
            
                if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
                {
                    ch->println("You are not carrying that.");
                    return;
                }
            
                vo = (void *) obj;
                target = TARGET_OBJ;
                break;
            
            case TAR_OBJ_MOB_OFF:
            case TAR_OBJ_CHAR_OFF:
                if (arg2[0] == '\0')
				{
					if ((victim = ch->fighting) == NULL)
                    {
						ch->println("Brandish the object on whom or what?");
						return;
                    }
            
                    target = TARGET_CHAR;
                }
                else if ((victim = get_char_room(ch,arg2)) != NULL)
                {
                    target = TARGET_CHAR;
                }
            
                if (target == TARGET_CHAR) // check the sanity of the attack 
                {
                    if(is_safe_spell(ch,victim,false) && victim != ch)
                    {
						ch->println("Not on that target.");
						return;
					}
            
                        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
                        {
                            ch->println("You can't do that on your own follower.");
							return;
						}
						vo = (void *) victim;
				}
				else if ((obj = get_obj_here(ch,arg2)) != NULL)
				{
					vo = (void *) obj;
					target = TARGET_OBJ;
				}
				else
				{
					ch->println("You don't see that here.");
					return;
				}
				break;
			
			case TAR_OBJ_CHAR_DEF:
				if (arg2[0] == '\0')
				{
					vo = (void *) ch;
					target = TARGET_CHAR;
				}
				else if ((victim = get_char_room(ch,arg2)) != NULL)
				{
					vo = (void *) victim;
					target = TARGET_CHAR;
				}
				else if ((obj = get_obj_carry(ch,arg2)) != NULL)
				{
					vo = (void *) obj;
					target = TARGET_OBJ;
				}
				else
				{
					ch->println("You don't see that here.");
					return;
				}
				break;
			} // end of spell type switch
			
			if (IS_NPC(ch) || class_table[ch->clss].fMana)
				// clss has spells
				(*skill_table[sn].spell_fun) ( sn, staff->value[0], ch, vo,target);
			else
				(*skill_table[sn].spell_fun) (sn, staff->value[0], ch, vo,target);
			check_improve(ch,gsn_staves,true,2);
		}
	}

    // get rid of the staff if required 
    if ( staff->value[2] == -1)
	{
		return;
	}
	staff->value[2]--;

	if (staff->value[2] < 1)
	{
		act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
		act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
		extract_obj( staff );
	}

	return;
}

/**************************************************************************/
void do_apply( char_data *ch, char *argument )
{
	char arg[MIL];
	char_data *victim;
	OBJ_DATA *poultice;

	if ( IS_OOC( ch )) {
		ch->println( "Not in an OOC room." );
		return;
	}

	if ( get_skill( ch, gsn_apply ) == 0 )
	{
		ch->println( "You are not educated in the herbal arts." );
		return;
	}

	one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "Apply what to whom?" );
		return;
	}

	if (( poultice = get_eq_char( ch, WEAR_HOLD )) == NULL )
 	{
		ch->println( "You hold nothing in your hand." );
		return;
	}

	if ( poultice->item_type != ITEM_POULTICE )
	{
		ch->println( "You are not holding a suitable medicinal component." );
		return;
	}

	if (( victim = get_char_room ( ch, arg ) ) == NULL )
	{
		ch->println( "They are not here." );
		return;
	}

	WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

	if ( poultice->value[2] != 0 )
	{
		act( "$n applies $p to $N.", ch, poultice, victim, TO_ROOM );
		act( "You apply $p to $N.", ch, poultice, victim, TO_CHAR );
	}

 	if ( ch->level < poultice->level 
	||  number_percent() >= 20 + get_skill( ch, gsn_apply ) * 4/5 )
	{
		act( "Your efforts to apply $p didn't produce any noticeable effects.",
			ch, poultice, NULL, TO_CHAR );
		act( "$n's efforts to apply $p had no noticeable effects.",
			ch, poultice, NULL, TO_ROOM );
		check_improve( ch, gsn_apply, false, 2 );
	}
	else
	{
		obj_cast_spell( poultice->value[3], poultice->value[0], ch, victim, NULL );
		check_improve( ch, gsn_apply, true, 2 );
	}

    if ( --poultice->value[2] == 0 )
    {
		act( "$n has used up $p.", ch, poultice, NULL, TO_ROOM );
		act( "You have used up $p.", ch, poultice, NULL, TO_CHAR );
		extract_obj( poultice );
	}

	if ( poultice->value[2] < 0 )
		poultice->value[2] = -1;		// set it back to infinite

    return;
}

   
/**************************************************************************/
void duel_unprotect_victim(char_data *victim);
/**************************************************************************/
void do_steal( char_data *ch, char *argument )
{
    char buf  [MSL];
    char arg1 [MIL];
    char arg2 [MIL];
    char_data *victim;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        ch->println( "Steal what from whom?" );
        return;
    }

	// ooc rooms
	if(!IS_IMMORTAL(ch) && IS_SET(ch->in_room->room_flags,ROOM_OOC)){
		ch->println("Not in an OOC room.");
		return;
	}

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

	if(!IS_NPC(victim) && victim->level<10 && ch->level>(victim->level+10)
		&& !IS_NPC(ch) && !IS_IMMORTAL(ch))
	{
		ch->println( "A faint wall of light appears to surround them preventing you from robbing them." );
		return;
	}

	if (!IS_IMMORTAL(ch) && !IS_NPC(ch) && !IS_NPC(victim) && GAMESETTING4(GAMESET4_PREVENT_STEALING_FROM_PLAYERS)){
		if(GAMESETTING4(GAMESET4_OOC_PREVENTION_MESSAGES)){
			ch->println( "Stealing from other players is not permitted on this mud." );
		}else{
			ch->println( "A faint wall of light appears to surround them preventing you from robbing them." );
		}
		return;
	}

    if ( victim == ch ){
        ch->println( "That's pointless." );
        return;
    }

	if(GAMESETTING5(GAMESET5_NO_STEALING_FROM_FIGHTING_CHARACTERS) && victim->fighting){
		ch->println( "Not while they are fighting!" );
		return;
	}

    if (is_safe(ch,victim)){
        return;
	}

    if ( IS_NPC(victim) && victim->position == POS_FIGHTING)
    {
        ch->println( "You'd better not -- you might get hit." );
        return;
    }

	// optional stealing checks which affect stealing between players
	if (!IS_IMMORTAL(ch) && !IS_NPC(victim)){
		if( GAMESETTING4(GAMESET4_MUST_BE_IN_CLAN_TO_STEAL_FROM_PLAYERS) ){
			ch->println( "You are not in a clan, stealing from players is restricted to clan members." );
			return;
		}
		
		if( GAMESETTING4(GAMESET4_NO_STEALING_FROM_NON_CLANNED) && !victim->clan ){
			ch->println( "They are not in a clan, stealing from players is restricted to clan members." );
			return;
		}

		// optional checks relating to letgain status
		if(GAMESETTING_LETGAINING_IS_REQUIRED){

			// is the potential thief letgained
			if ( GAMESETTING4(GAMESET4_MUST_BE_LETGAINED_TO_STEAL_FROM_PLAYERS)
				&& !IS_LETGAINED(ch))
			{
				ch->println("You can't steal from non letgained players.");
				return;
			}

			// is the intended victim letgained
			if ( GAMESETTING4(GAMESET4_NO_STEALING_FROM_NON_LETGAINED)
				&& !IS_LETGAINED(victim))
			{
				ch->println("You can't steal from non letgained players.");
				return;
			}

			if(GAMESETTING5(GAMESET5_MUST_BE_ACTIVE_TO_BE_INVOLVED_IN_STEALING)){
   				if(!HAS_CONFIG(victim, CONFIG_ACTIVE))
		 		{
 					ch->println( "You can't steal from non active players." );
 					return;
		 		}
  		 		if(!HAS_CONFIG(ch, CONFIG_ACTIVE))
		 		{
 					ch->println( "You must be active in order to steal from players." );
		 			return;
				}
			}
  
 			if(HAS_CONFIG2(ch,CONFIG2_NOPKILL))
		 	{
 				ch->println( "A faint wall of light appears to surround them preventing you from robbing them." );
		 		return;
 			}
 
		 	if(HAS_CONFIG2(victim,CONFIG2_NOPKILL))
 			{
		 		ch->println( "A faint wall of light appears to surround them preventing you from robbing them." );
 				return;
		 	}
 
		}
		// optional: stealing only permitted within ten levels of each other
		if (GAMESETTING4(GAMESET4_NO_STEALING_OUTSIDE_10_LEVELS)){	
			if ( (ch->level - victim->level) > 10  || (ch->level - victim->level) < -10 ) {
				ch->println("You may not steal from a character of more than 10 levels difference to your own." );
				return;
			}
		}
	}

	// silent imms don't get lagged
	if (!IS_SILENT(ch) || !IS_ICIMMORTAL(ch)){
		WAIT_STATE( ch, skill_table[gsn_steal].beats );
	}

	// for 20 minutes you are unsafe if you steal
	if(!IS_NPC(ch) && !IS_NPC(victim)){
		ch->pcdata->unsafe_due_to_stealing_till=current_time+ 20*60; 

		if(ch->duels){
			duel_unprotect_victim(ch);
		}
	}

	if (IS_ICIMMORTAL(victim)){
		ch->println( "You failed." );
		victim->wraplnf( "NOTICE: %s attempted to steal '%s' from you and failed because you are considered an IC immortal.",
			PERS(ch, victim), arg1);
		return;
	}

    percent  = number_percent();
    if (get_skill(ch,gsn_steal) >= 1){
        percent  += ( IS_AWAKE(victim) ? 10 : -50 );
	}
	if(victim->pcdata && victim->pcdata->is_trying_aware){
		percent += (get_skill(victim, gsn_awareness))/6;
	}

    if ( (IS_KEEPER(victim))
        ||(!IS_NPC(ch) && (percent+2*victim->level-2*ch->level > get_skill(ch,gsn_steal)))
		||(!IS_NPC(ch) && get_skill(ch,gsn_steal)==0)  
		|| (ch->level<=LEVEL_HERO && victim->level>LEVEL_HERO))
    {
        /*
         * Failure.
         */
        ch->println( "Oops." );

		// make them go visible
		affect_strip ( ch, gsn_invisibility					);
		affect_strip ( ch, gsn_mass_invis					);
		affect_strip ( ch, gsn_sneak						);
		REMOVE_BIT   ( ch->affected_by, AFF_HIDE			);
		REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE		);
		REMOVE_BIT   ( ch->affected_by, AFF_SNEAK			);
		
		if (!IS_NPC(victim))
		{
			ch->pksafe=0;
			ch->pknorecall= (UMAX(ch->pknorecall,2));
			ch->pknoquit=(UMAX(ch->pknoquit,10));
		}

        if (IS_AWAKE(victim)){
            act( "$n tried to steal from YOU!", ch, NULL, victim, TO_VICT );
			if( victim->pcdata && victim->pcdata->is_trying_aware )
				check_improve(victim,gsn_awareness,true,1);
		}else{
			if( victim->pcdata && victim->pcdata->is_trying_aware
			 && get_skill(victim, gsn_awareness) > number_percent() )
			{
				victim->println( "You feel the presence of another person lurking near you." );
				victim->println( "You feel someone try to take something from you and instantly pop awake!" );
				victim->position=POS_RESTING;
				do_stand(victim,"");
				check_improve(victim,gsn_awareness,true,2);
			}else{
				victim->println( "You are partially awakened by someone in the room." );
				victim->println( "You roll over and drift back to sleep." );
			}
		}

        act( "$n tried to steal from $N.",  ch, NULL, victim, TO_NOTVICT );

        if ( !IS_NPC(ch) )
        {
            if ( IS_NPC(victim) )
            {
                check_improve(ch,gsn_steal,false,2);
                multi_hit( victim, ch, TYPE_UNDEFINED );
            }
            else
            {
				sprintf(buf,"$N tried to steal from %s <%d>.", 
					victim->name, victim->level);
				wiznet(buf,ch,NULL,WIZ_THEFTS,0,0);
				if(GAMESETTING3(GAMESET3_THIEF_SYSTEM_ENABLED)){
					if(!IS_NPC(ch)){
						if(!IS_THIEF(ch)){
							ch->println( "*** You are now tagged as a THIEF!! ***");
						}
						ch->pcdata->thief_until=current_time+ (60*game_settings->thief_system_tagged_duration);
						save_char_obj( ch );
					}
				}
            }
        }  
        return;
    }
	else 


	if (!IS_NPC(victim))
	{
		// transfer those to the void that are attacked 
		// while linkdead and don't have a pkill timer
		if ( victim->pknorecall ==0 
			&& IS_LINKDEAD(victim)
			&& victim->was_in_room == NULL 
			&& victim->in_room != NULL )
		{
			victim->was_in_room = victim->in_room;
			if ( victim->fighting != NULL )
				stop_fighting( victim, true );
			act( "$n disappears into the void.", victim, NULL, NULL, TO_ROOM );
			victim->println( "You disappear into the void." );
			if (victim->level > 1)
				save_char_obj( victim);
			char_from_room( victim);
			char_to_room( victim, get_room_index( ROOM_VNUM_LIMBO ) );
			return;
		}

		ch->pksafe=0;
		ch->pknoquit=(UMAX(ch->pknoquit,10));
	}

    if (	!str_cmp( arg1, "coin"  )
        ||	!str_cmp( arg1, "coins" )
        ||	!str_cmp( arg1, "gold"  )
        ||	!str_cmp( arg1, "silver"))
    {
        int gold, silver;
        gold = victim->gold * number_range(1, ch->level) / MAX_LEVEL;
        silver = victim->silver * number_range(1,ch->level) / MAX_LEVEL;

        if (gold > victim->gold) gold = victim->gold;
        if (silver > victim->silver) silver = victim->silver;
              
        if ( gold <= 0 && silver <= 0 )
        {
             ch->println( "You couldn't get any coins." );
             return;
        }
        ch->gold        += gold;
        ch->silver      += silver;
        victim->silver  -= silver;
        victim->gold    -= gold;
        if (silver <= 0)
             ch->printlnf( "Bingo!  You got %d gold coins.", gold );
        else if (gold <= 0)
             ch->printlnf( "Bingo!  You got %d silver coins.", silver);
        else
             ch->printlnf( "Bingo!  You got %d silver and %d gold coins.", silver, gold );
    
        check_improve(ch,gsn_steal,true,2);

		if (!IS_NPC(victim))
		{
			sprintf(buf,"$N successfully stole %d silver and %d "
				"gold coins from %s <%d>.", silver, gold, 
				victim->name, victim->level);
			wiznet(buf,ch,NULL,WIZ_THEFTS,0,0);
		}
        return;
    }

    if ( ( obj = get_obj_carry_for_looker( victim, arg1, ch ) ) == NULL )
    {
        ch->println( "You can't find it." );
        return;
    }

    if ( !can_drop_obj( ch, obj )
        ||   IS_SET(obj->extra_flags, OBJEXTRA_INVENTORY) )
    {
        ch->println( "You can't pry it away." );
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        ch->println( "You have your hands full." );
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
		ch->println( "You can't carry that much weight." );
		return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    check_improve(ch,gsn_steal,true,2);
    ch->println( "Got it!" );
	
	if (!IS_NPC(victim))
	{
		sprintf(buf,"$N successfully stole %s [%d] from %s <%d>.",
			obj->short_descr, obj->pIndexData->vnum, victim->name, 
			victim->level);
		wiznet(buf,ch,NULL,WIZ_THEFTS,0,0);
	}
    return;
}
/**************************************************************************/
/*
 * Shopping commands.
 */
/**************************************************************************/
char_data *find_keeper( char_data *ch )
{
    char_data *keeper;
    SHOP_DATA *pShop;
	
	pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
		if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
			break;
	}
	
    if ( pShop == NULL )
	{
		ch->println("You can't do that here.");
		return NULL;
    }
	
    // Shop hours.
    if ( time_info.hour < pShop->open_hour )
    {
		do_say( keeper, "Sorry, I am closed. Come back later." );
		return NULL;
    }
    
	if ( time_info.hour > pShop->close_hour )
    {
		do_say( keeper, "Sorry, I am closed. Come back tomorrow." );
		return NULL;
    }
	
    // Invisible or hidden people.
	if ( !can_see( keeper, ch )  && INVIS_LEVEL(ch)<1)
    {
		do_say( keeper, "I don't trade with folks I can't see." );
		return NULL;
	}
	
    return keeper;
}

/**************************************************************************/
// insert an object at the right spot for the keeper 
void obj_to_keeper( OBJ_DATA *obj, char_data *ch )
{
    OBJ_DATA *t_obj, *t_obj_next;
	
	// see if any duplicates are found 
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
		t_obj_next = t_obj->next_content;
		
		if (obj->pIndexData == t_obj->pIndexData
			&&  !str_cmp(obj->short_descr,t_obj->short_descr))
		{
			// if this is an unlimited item, destroy the new one 
			if (IS_OBJ_STAT(t_obj,OBJEXTRA_INVENTORY))
			{
				extract_obj(obj);
				return;
			}
			obj->cost = t_obj->cost; // keep it standard 
			break;
		}
    }
	
    if (t_obj == NULL)
	{
		obj->next_content = ch->carrying;
		ch->carrying = obj;
    }
    else
    {
		obj->next_content = t_obj->next_content;
		t_obj->next_content = obj;
	}
	
    obj->carried_by      = ch;
	obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

/**************************************************************************/
// get an object from a shopkeeper's list 
OBJ_DATA *get_obj_keeper( char_data *ch, char_data *keeper, char *argument )
{
	char arg[MIL];
	char newarg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

	// convert 'buy 5' to 'buy 5.'
	if(is_number(argument)){
		sprintf(newarg,"%s.",argument);
		argument=newarg;
	};
	
    number = number_argument( argument, arg );
    count  = 0;
	int uid=get_uid(arg);
	if(uid){
		number=1;
	}

	for ( obj = keeper->carrying; obj; obj = obj->next_content )
    {
		if (obj->wear_loc == WEAR_NONE
			&&  can_see_obj( keeper, obj )
			&&  can_see_obj(ch,obj)			
			&&  (  (uid && uid==obj->uid)
				 || is_name( arg, obj->name ) 
				 )
			)
        {
            if ( ++count == number )
                return obj;
			
			// skip other objects of the same name
			while (obj->next_content != NULL
				&& obj->pIndexData == obj->next_content->pIndexData
				&& !str_cmp(obj->short_descr,obj->next_content->short_descr))
				obj = obj->next_content;
		}
    }
	
    return NULL;
}

/**************************************************************************/
// figure out how much a shop keeper will pay/sell an item for.
int get_cost( char_data *keeper, OBJ_DATA *obj, bool fBuy )
{
	SHOP_DATA *pShop;
    int cost;
	
    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
		return 0;

	if(IS_SET(obj->extra2_flags,OBJEXTRA2_NOSELL)){
		// non sellable objects
		return 0; 
	}
	
	if ( fBuy )
    {
		cost = obj->cost * pShop->profit_buy  / 100;
    }
	else
    {
		OBJ_DATA *obj2;
		int itype;
		
		cost = 0;
		for ( itype = 0; itype < MAX_TRADE; itype++ )
		{
			if ( obj->item_type == pShop->buy_type[itype] )
			{
				cost = obj->cost * pShop->profit_sell / 100;
				break;
			}
		}
		
		if (!IS_OBJ_STAT(obj,OBJEXTRA_SELL_EXTRACT)){
			for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
			{
				if ( obj->pIndexData == obj2->pIndexData
					&&   !str_cmp(obj->short_descr,obj2->short_descr) ){
					if (IS_OBJ_STAT(obj2,OBJEXTRA_INVENTORY)){
						cost /= 2;
					}else{
						cost = cost * 3 / 4;
					}
				}
			}
		}
    }
	
    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND ) {
		if (obj->value[1] == 0){
			cost /= 4;
		}else{
			cost = cost * obj->value[2] / obj->value[1];
		}
    }
	
    return cost;
}
/**************************************************************************/
void buy_in_petshop( char_data *ch, char *argument )
{
	int cost,roll;
	char arg[MIL];
	char buf[MSL];
	char_data *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;
	
	if ( IS_NPC(ch) )
		return;
	
	argument = one_argument(argument,arg);
	
	// petshop storage room is the room which vnum 
	// is one greater than the current room
	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	
	if ( pRoomIndexNext == NULL )
	{
		bugf( "do_buy(): bad pet shop at vnum %d.", ch->in_room->vnum );
		ch->println("Sorry, you can't buy that here.");
		return;
	}
	
	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_pet_room( ch, arg );
	ch->in_room = in_room;
	
	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
		ch->println("Sorry, you can't buy that here.");
		return;
	}
	
	if ( ch->pet != NULL )
	{
		ch->println("You already own a pet.");
		return;
	}
	
	cost = 10 * pet->level * pet->level;
	
	if IS_CONTROLLED(ch) cost =0;
	
	if ( ch->level < pet->level )
	{
		ch->println("You're not powerful enough to master this pet.");
		return;
	}
	// haggle 
	roll = number_percent();
	if (roll < get_skill(ch,gsn_haggle))
	{
		cost -= cost / 2 * roll / 100;
		if ( (ch->silver + 100 * ch->gold) < cost)
		{				
			ch->printlnf( "You haggle the price down to %d coin%s, but "
				"still can't afford it.", cost, cost==1?"":"s");
			
			if ( (ch->silver + 100 * ch->gold)>0 
				&& (ch->silver + 100 * ch->gold) < (cost* 96/100) )
			{				
				cost=(ch->silver + 100 * ch->gold);
				ch->printlnf( "You explain this to the pet shopkeeper and manage "
					"haggle the price down to the money that you do have!");
			}else{ // didn't get it
				WAIT_STATE(ch, 20); // 5 seconds lag to reduce potential abuse
				return;
			}
		}else{
			ch->printlnf( "You haggle the price down to %d coin%s.",
				cost, cost==1?"":"s");
			check_improve(ch,gsn_haggle,true,4);
		}		
	}else{
		if ( (ch->silver + 100 * ch->gold) < cost )
		{
			ch->println("You can't afford it.");
			return;
		}
	}
	
	deduct_cost(ch,cost);
	pet			= create_mobile( pet->pIndexData, 0 );
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
	
	argument = one_argument( argument, arg );
	if(!IS_NULLSTR(arg)){
		sprintf( buf, "%s %s", pet->name, arg );
		free_string( pet->name );
		pet->name = str_dup( buf );
	}
	
	sprintf( buf, "%sA neck tag says 'I belong to %s'.\r\n",
		pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );
	
	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	ch->println("Enjoy your pet.");
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
}

/**************************************************************************/
void do_lockers(char_data *ch, char *argument);
/**************************************************************************/
void do_buy( char_data *ch, char *argument )
{
    char buf[MSL];
	int cost,roll;
	
    if(IS_NULLSTR(argument)){
		ch->println( "Buy what?" );
		if(IS_NEWBIE(ch)){
			ch->println( "Use the 'list' command to see what is available while near a shopkeeper." );
		}
		return;
    }
	
	smash_tilde(argument);
	
	// pet shops
	if( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)){
		buy_in_petshop(ch, argument);
		return;
    }

	// locker code
	if(!str_prefix("locker", argument)){
		do_lockers(ch, FORMATF("%s startrent", argument+6));
		return;
	}

    {
		char_data *keeper;
		OBJ_DATA *obj,*t_obj;
		char arg[MIL];
		int number, count = 1;
		
		if ( ( keeper = find_keeper( ch ) ) == NULL )
			return;
	
		number = mult_argument(argument,arg);
		obj  = get_obj_keeper( ch,keeper, arg );
		cost = get_cost( keeper, obj, true );
				
		if(number<1){
			return;
		}
		if (number>100) 
		{ 
			ch->println("You may only purchase in maximum increments of 100."); 
			return; 
		}
		
		if ( cost <= 0 || !can_see_obj( ch, obj ) )
		{
			act( "$n tells you 'I don't sell that -- try 'list''.",
				keeper, NULL, ch, TO_VICT );
			return;
		}
		
		if (!IS_OBJ_STAT(obj,OBJEXTRA_INVENTORY))
		{
			for (t_obj = obj->next_content;
			count < number && t_obj != NULL;
			t_obj = t_obj->next_content)
			{
				if (t_obj->pIndexData == obj->pIndexData
					&&  !str_cmp(t_obj->short_descr,obj->short_descr))
					count++;
				else
					break;
			}
			
			if (count < number)
			{
				act("$n tells you 'I don't have that many in stock.",
					keeper,NULL,ch,TO_VICT);
				//ch->reply = keeper;
				return;
			}
		}
		
		if (!IS_CONTROLLED(ch))
		{
				
			if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
			{
				ch->println("You can't carry that many items.");
				return;
			}
			
			if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
			{
				ch->println("You can't carry that much weight.");
				return;
			}
			
			// haggle 
			roll = number_percent();
			if (!IS_OBJ_STAT(obj,OBJEXTRA_SELL_EXTRACT) 
				&& roll < get_skill(ch,gsn_haggle))
			{
				cost -= obj->cost / 2 * roll / 100;
				
				// do can afford checks
				if ( (ch->silver + ch->gold * 100) < cost * number )
				{
					act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
					if (number > 1){
						act("$n tells you 'I wont sell you that many so cheap.",
						keeper,obj,ch,TO_VICT);
					}else{
						act( "$n tells you 'I wont sell $p that cheap sorry'.",
						keeper, obj, ch, TO_VICT );
					}
					WAIT_STATE(ch, 20); // 5 seconds lag to reduce potential abuse
					return;
				}else{
					act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
					check_improve(ch,gsn_haggle,true,4);
				}
			}

			// do can afford checks
			if ( (ch->silver + ch->gold * 100) < cost * number )
			{
				if (number > 1)
					act("$n tells you 'You can't afford to buy that many.",
					keeper,obj,ch,TO_VICT);
				else
					act( "$n tells you 'You can't afford to buy $p'.",
					keeper, obj, ch, TO_VICT );
				return;
			}
		}
		
		if (number > 1)
		{
			sprintf(buf,"$n buys $p[%d].",number);
			act(buf,ch,obj,NULL,TO_ROOM);
			sprintf(buf,"You buy $p[%d] for %d silver.",number,cost * number);
			act(buf,ch,obj,NULL,TO_CHAR);
		}
		else
		{
			act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
			sprintf(buf,"You buy $p for %d silver.",cost);
			act( buf, ch, obj, NULL, TO_CHAR );
		}
		
		if (!IS_CONTROLLED (ch))
			deduct_cost(ch,cost * number);
		
		keeper->gold += cost * number/100;
		keeper->silver += cost * number - (cost * number/100) * 100;

		// limit a shopkeepers wealth if more than 10 minutes after a reboot
		if(boot_time+360<current_time){ 
			limit_mobile_wealth(keeper);
		}
		
		for (count = 0; count < number; count++)
		{
			if ( IS_SET( obj->extra_flags, OBJEXTRA_INVENTORY ) ){
				t_obj = create_object( obj->pIndexData);
			}else{
				t_obj = obj;
				obj = obj->next_content;
				obj_from_char( t_obj );
			}
			
			if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,OBJEXTRA_HAD_TIMER))
				t_obj->timer = 0;
			
			REMOVE_BIT(t_obj->extra_flags,OBJEXTRA_HAD_TIMER);
			
			REMOVE_BIT(t_obj->extra_flags,OBJEXTRA_INVENTORY);
			
			obj_to_char( t_obj, ch );
			if (cost < t_obj->cost)
				t_obj->cost = cost;
		}
    }
}

/**************************************************************************/
// wear object as a secondary weapon 
void do_second (char_data *ch, char *argument)
{
	OBJ_DATA *obj;

	if(!GAMESETTING2(GAMESET2_NO_SECOND_SKILL_REQUIRED)){
		if ( !IS_NPC(ch) && ch->get_skill(gsn_second)<1){
			ch->println("You don't have a clue how to hold a weapon in your off-hand correctly.");
			return;
		}
	}
	
	if (IS_NULLSTR(argument)){
		ch->println("Wear which weapon in your off-hand?");
		return;
	}
	
	obj = get_obj_carry (ch, argument); // find the obj withing ch's inventory 
	
	if (obj == NULL)
	{
		ch->println("You have no such thing in your backpack.");
		return;
	}
	
	if (( obj->level>LEVEL_HERO)&&(ch->level < LEVEL_IMMORTAL))
	{
		char buf[MSL];
		
		sprintf( buf, "`x%s (%s) lvl %d`1tried to wear `1"
			"%s [%d] lvl %d`1"
			"It was removed since it was out of level.", 
			ch->name,		ch->short_descr,	ch->level, 
			obj->short_descr,	obj->pIndexData->vnum, obj->level);
		
		autonote(NOTE_SNOTE, "p_anote()","`Yoverlevel object!`x", "imm", buf, true);
		
		ch->printlnf( "You try to use %s and it crumbles to dust.", obj->short_descr );
		
		act( "$n tries to use $p, but it crumbles to dust.",
			ch, obj, NULL, TO_ROOM );
		
		extract_obj(obj);
		return;
	}
	
	if(obj->item_type != ITEM_WEAPON)
	{
		ch->println("That is not a weapon.");
		return; }
	
	if(!CAN_WEAR( obj, OBJWEAR_WIELD ))
	{
        ch->println("You can not wield that.");
        return;
	}

	if ( IS_SET( obj->extra2_flags, OBJEXTRA2_NOSECONDARY ))
	{
		act( "You cannot wield $p in your offhand.", ch, obj, NULL, TO_CHAR );
		return;
	}

	if ( !IS_ADMIN( ch )) // Immortal requested not to have this but I wanted it :)
	{
		if(!IS_NPC(ch) && obj->pIndexData->relative_size<
			race_table[ch->race]->low_size)
		{
	        act("$p is too small for you.",ch,obj,NULL,TO_CHAR);
	        return;
		}
	
		if(!IS_NPC(ch) && obj->pIndexData->relative_size>
			race_table[ch->race]->high_size)
		{
	        act("$p is too large for you.",ch,obj,NULL,TO_CHAR);
	        return;
		}
	}

	if ( !IS_IMMORTAL( ch ))
	{
		if(DISALLOWED_OBJECT_FOR_CHAR(obj, ch))
		{
			act("$p cannot be used by your class.",ch,obj,NULL,TO_CHAR);
			return;
		}
	}
	
    if(IS_SET(obj->extra_flags, OBJEXTRA_MAGIC)
		&& HAS_CLASSFLAG(ch, CLASSFLAG_MAGIC_ANTIPATHY))
	{
		act("You have great antipathy towards $p.",ch,obj,NULL,TO_CHAR);
		return;
	}

	
	// check that the character is using a first weapon at all 
	if (get_eq_char (ch, WEAR_WIELD) == NULL) 
	{
		ch->println("You need to wield a primary weapon, before using a secondary one!");
		return;
	}
	
	
	// check for str - secondary weapons have to be lighter 
	if ( get_obj_weight( obj ) > ( (ch->modifiers[STAT_ST] + 15)*5) )
	{
		ch->println("This weapon is too heavy to be used as a secondary weapon by you.");
		return;
	}
	
	if (IS_WEAPON_STAT(get_eq_char(ch,WEAR_WIELD),WEAPON_TWO_HANDS)
		&& ch->size < SIZE_LARGE)
	{
		ch->println("You cannot wear a weapon in your off hand if using a two handed weapon.");
		return;
	}
	
	// check if the char is using a shield or a held weapon 
	if ( get_eq_char (ch,WEAR_SHIELD) != NULL) 
	{
		if ((!remove_obj(ch, WEAR_SHIELD, true))){
	//		ch->println("You cannot use a secondary weapon while using a shield or holding an item.");
			return;
		}
	}

	if ( get_eq_char(ch,WEAR_HOLD) != NULL)
	{		
		if ((!remove_obj(ch, WEAR_HOLD, true))){
	// ch->println("You cannot use a secondary weapon while using a shield or holding an item.");
			return;
		}
	}


	// at last - the char uses the weapon 
	if (!remove_obj(ch, WEAR_SECONDARY, true)) // remove the current weapon if any 
		return;                                // remove obj tells about any no_remove 
	
	if (IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
	{
		ch->println("You can't use a two handed weapon in your off hand.");
		return;
	}
	
	act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
	act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
	equip_char ( ch, obj, WEAR_SECONDARY);
	return;
}

/**************************************************************************/
char_data* find_innkeeper(char_data *ch);
/**************************************************************************/
//format_obj_to_char( OBJ_DATA *obj, char_data *ch, bool fShort )
void do_list( char_data *ch, char *argument )
{
    char buf[MSL];
	BUFFER *output;

	if(str_len(argument)>3 && !str_prefix(argument, "lockers")){
		do_lockers(ch, "list");
		return;
	}

    if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    { // pet shop 
        ROOM_INDEX_DATA *pRoomIndexNext;
        char_data *pet;
        bool found;

        pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

        if (pRoomIndexNext == NULL )
        {
            bugf( "do_list(): bad pet shop at vnum %d.", ch->in_room->vnum );
            ch->println("You can't do that here.");
            return;
        }

		output = new_buf();
        found = false;
        for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
        {

            if ( IS_NPC(pet) && IS_SET(pet->act, ACT_PET) )
            {
                if ( !found )
                {
                    found = true;
					add_buf(output,"Pets for sale:\r\n");
                }
                sprintf( buf, "[%2d] %8d - %s\r\n",

                    pet->level,
                    10 * pet->level * pet->level,
                    pet->short_descr );

				// holyvnum
				if (!IS_UNSWITCHED_MOB(ch) && IS_SET(TRUE_CH(ch)->act, PLR_HOLYVNUM))
				{
					char buf2[MSL];
					if (IS_NPC(pet) && pet->pIndexData)
					{
						if (pet->pIndexData->mob_triggers)
						{
							sprintf(buf2," `#`r*%d,%d*`^\r\n", pet->pIndexData->vnum, pet->level);
						}
						else
						{
							sprintf(buf2," `#`B[%d,%d]`^\r\n", pet->pIndexData->vnum, pet->level);
						}
						buf[str_len(buf)-2]= '\0'; // chop off the \r\n
						strcat(buf,buf2);
					}
				}
				add_buf(output,buf);
            }
        }

        if ( !found ){
			add_buf(output,"Sorry, we're out of pets right now.\r\n");
		}
		ch->sendpage(buf_string(output));
	    free_buf(output);
	    return;
		// end of petshop code
	} else if ( IS_SET(ch->in_room->room_flags, ROOM_INN) ){
	// Innkeeper code.
		ROOM_INDEX_DATA* pRoom	= NULL;
		char_data* pInnKeeper	= NULL;
		cInnData* pInn			= NULL;
		char pListItem[MSL];
		vn_int roomVnum			= 0;
		int roomRate			= 0;
		sh_int roomIdx			= 0;
		bool foundItem			= false;

		output = new_buf();

		// Find the first mob in the room that is an innkeeper.
		if ( !(pInnKeeper=find_innkeeper(ch))  ) {
			return;
		}

		// Just a shortcut pointer to make the rest of the code more readable.
		pInn = pInnKeeper->pIndexData->pInnData;

		// Add valid rooms to the output buffer.
		for(roomIdx=0; roomIdx<MAX_INN; roomIdx++ ) {
			roomVnum = pInn->vnRoom[roomIdx]; 
			roomRate = pInn->shRate[roomIdx];

			if ( roomVnum > 0 ) {
				if ( (pRoom=get_room_index(roomVnum)) != NULL ) {
					if ( !foundItem ) {
						foundItem = true;
						add_buf(output, "INN - ROOMS FOR RENT`1");
						add_buf(output, "[Number  Price/Hour] Description`1");
					}

					sprintf(pListItem, " %6d    %8d  `#%-32s`^\r\n", roomIdx+1, roomRate, pRoom->name);
					add_buf(output, pListItem);
				}
			}
		}

		if ( !foundItem ) {
			add_buf(output, "There are no rooms to rent here.`1");
		}

		ch->sendpage(buf_string(output));
	    free_buf(output);

		return;
    }else{// end of inn start of normal shop
		char_data *keeper;
        OBJ_DATA *obj;
        int count, can_wear;
		int foundcount;
        char arg[MIL];
		char buf2[MSL];
		int cost=0, linecount;

		// listing by level matching system
		bool doingFiltering=false;
		bool levelBasedFiltering=false;
		int max_obj_level=MAX_LEVEL;
		int min_obj_level=0;

        if (!(keeper = find_keeper(ch)))
            return;

        // preparse any parameters
		argument=one_argument(argument,arg);
		if (!IS_NULLSTR(arg)){
			doingFiltering=true;
			// figure out if it is level based
			if(is_number(arg)){
				levelBasedFiltering=true;

				max_obj_level=atoi(arg);
				// second level - level range match
				argument=one_argument(argument,arg);

				if(!IS_NULLSTR(arg)){
					if(!is_number(arg)){
						ch->println("You can't search by level and substrings at the same time sorry.");
						return;
					}

					min_obj_level=atoi(arg);
					
					// swap the levels if necessary
					if (min_obj_level>max_obj_level){
						foundcount=max_obj_level;
						max_obj_level=min_obj_level;
						min_obj_level=foundcount;
					}

				}				
			}else{
				// substring match
			}
		}// end of parsing format and show results
      
		output = new_buf();

		foundcount=0;
		linecount=0;
        for ( obj = keeper->carrying; obj && linecount<200;obj = obj->next_content )
        {
            if ( obj->wear_loc == WEAR_NONE
               &&   can_see_obj( ch, obj )
               &&   ( cost = get_cost( keeper, obj, true ) ) > 0 )
            {
				// do the filtering
				if (doingFiltering){
					if(levelBasedFiltering){
						if (obj->level<min_obj_level){
							continue;
						}
						if (obj->level>max_obj_level){
							continue;
						}
					}else{ // substring match filter
						if (!is_name(arg,obj->name)){
							continue;
						}
					}
				}

                if ( foundcount==0 ){
					add_buf(output,"  [Lv  Price Qty] Item                      (type?)\r\n");
                }
				foundcount++;

				// colour code wearable items vs unwearable
				if(	(
						IS_SET(obj->extra_flags, OBJEXTRA_MAGIC)
						&& HAS_CLASSFLAG(ch, CLASSFLAG_MAGIC_ANTIPATHY)				
					)
					||(	   !IS_NPC(ch) 
						&& obj->pIndexData->relative_size 
							< race_table[ch->race]->low_size 
						&& obj->item_type != ITEM_LIGHT 
						&& !CAN_WEAR( obj, OBJWEAR_HOLD)  
						&& !CAN_WEAR( obj, OBJWEAR_FLOAT) 
					  )
					||(	   !IS_NPC(ch) 
						&& obj->pIndexData->relative_size>race_table[ch->race]->high_size 
						&& obj->item_type != ITEM_LIGHT 
						&& !CAN_WEAR( obj, OBJWEAR_FLOAT) 
					  )
				  )
				{
					can_wear=0;
				}else{
					can_wear=1;
				}


				if(!IS_NPC(ch) && can_wear)
				{
					can_wear = DISALLOWED_OBJECT_FOR_CHAR(obj, ch)?0:1;
				}
				
				if(can_wear==1){
					add_buf(output,"`x");
				}else{
					add_buf(output,"`s");
				}
				//// end of colour coding


				// create an mxp short descript
				char short_descr[MSL];
				if(obj->item_type == ITEM_WEAPON){
					strcpy(short_descr,				
						mxp_create_tag(ch, FORMATF("buy-uid %d", obj->uid), FORMATF("%-25s", obj->short_descr)));
				}else{
					strcpy(short_descr,
						mxp_create_tag(ch, FORMATF("buy-uid %d", obj->uid), obj->short_descr));
				}

                if (IS_OBJ_STAT(obj,OBJEXTRA_INVENTORY))
                    if (obj->item_type == ITEM_WEAPON)
                    {
                        sprintf(buf,"[%2d %6d -- ] %s (%s)\r\n",
                        obj->level,cost,short_descr, get_weapontype(obj));
                    }else{
                        sprintf(buf,"[%2d %6d -- ] %s\r\n",
                        obj->level,cost,short_descr);
                    }
                else
                {
                    count = 1;           
                    while (obj->next_content != NULL 
                        && obj->pIndexData == obj->next_content->pIndexData
                        && !str_cmp(obj->short_descr,
                            obj->next_content->short_descr))
                    {
                        obj = obj->next_content;
                        count++;
                    }
                    if (obj->item_type == ITEM_WEAPON)
                    {   
                        sprintf(buf,"[%2d %6d %2d ] %s (%s)\r\n",
                            obj->level,cost,count,short_descr, get_weapontype(obj));
                    }
                    else
                    {
                        sprintf(buf,"[%2d %6d %2d ] %s\r\n",
                            obj->level,cost,count,short_descr);
                    }
                }
				//holyvnum
				if (!IS_UNSWITCHED_MOB(ch) && IS_SET(TRUE_CH(ch)->act, PLR_HOLYVNUM))
				{

					if (obj->pIndexData)
					{
						sprintf(buf2," `#`s[%d,%d]`^\r\n", obj->pIndexData->vnum, obj->level);
						buf[str_len(buf)-2]= '\0'; // chop off the \r\n
						strcat(buf,buf2);
					}
				}

				// number each item
				strcpy(buf2, mxp_create_tag(ch, FORMATF("buy-uid %d", obj->uid), FORMATF("%2d", linecount+1)));
				add_buf(output,buf2);

				// add the the buffer the item details
				add_buf(output,buf);
				linecount++;
            }
        }

        if ( linecount==0 )
		{
			if (IS_NULLSTR(arg))
				ch->println("You can't buy anything here.");
			else
				ch->println("No matching items found.");
		}

		if (linecount>200){
			add_buf(output,"`xCan only list 200 lines on the list at one time.\r\n");
			add_buf(output,"filter the list by level range to access the \r\n"
				"information regarding the complete list.\r\n");
		}else if ( IS_NEWBIE(ch) && linecount>20 )
		{
			add_buf(output,"`xYou can actually filter the list, for example if you wanted to \r\n");
			add_buf(output,"purchase an item that was green, type `=Clist green`x.  If the item was\r\n");
			add_buf(output,"displayed as the 3rd on the list then to purchase it type `=Cbuy 3.green`x\r\n");
		}

        ch->sendpage(buf_string(output));
	    free_buf(output);
        return;
    }
}

/************************************************************************/
void sell_object(char_data *ch, char_data *keeper, OBJ_DATA *obj)
{
    char buf[MSL];
    int cost,roll;
    
	if ( !can_drop_obj( ch, obj ) )
    {
		ch->printlnf( "You can't let go of %s.", obj->short_descr);
		return;
    }

	if  (( obj->item_type == ITEM_CAULDRON 
		|| obj->item_type == ITEM_CONTAINER
		|| obj->item_type == ITEM_FLASK
		|| obj->item_type == ITEM_MORTAR )
		&& obj->contains)
    {
		ch->printlnf( "Try emptying everything out of %s before selling it.", obj->short_descr);
		return;
    }

	if (!can_see_obj(keeper,obj))
	{
		act("$n doesn't appear to be able to see $p.",keeper,obj,ch,TO_VICT);
		return;
	}

    if ( ( cost = get_cost( keeper, obj, false ) ) <= 0 )
    {
		act( "`s$n looks uninterested in $p.`x", keeper, obj, ch, TO_VICT );
		return;
    }
    if ( cost > (keeper-> silver + 100 * keeper->gold) )
	{
		act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
			keeper,obj,ch,TO_VICT);
		return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );

    // haggle 
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,OBJEXTRA_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
    {
        ch->println("You haggle with the shopkeeper.");
        cost += obj->cost / 2 * roll / 100;
		cost = UMIN(cost,95 * get_cost(keeper,obj,true) / 100);
		cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
        check_improve(ch,gsn_haggle,true,4);
    }
    
	if (((int)cost/100)>0)
	{
		sprintf( buf, "You sell $p for %d silver and %d gold piece%s.",
			cost - (cost/100) * 100, cost/100, cost == 1 ? "" : "s" );
	}
	else
	{
		sprintf( buf, "You sell $p for %d silver piece%s.",
			cost - (cost/100) * 100, cost == 1 ? "" : "s" );
	}

	act( buf, ch, obj, NULL, TO_CHAR );
	ch->gold     += cost/100;
	ch->silver 	 += cost - (cost/100) * 100;

    deduct_cost(keeper,cost);
    if ( keeper->gold < 0 )
		keeper->gold = 0;
    if ( keeper->silver< 0)
		keeper->silver = 0;

    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,OBJEXTRA_SELL_EXTRACT))
    {
		extract_obj( obj );
    }
    else
    {
		obj_from_char( obj );
		if (obj->timer)
			SET_BIT(obj->extra_flags,OBJEXTRA_HAD_TIMER);
		else
			obj->timer = number_range(50,100);
		obj_to_keeper( obj, keeper );
    }

}

/**************************************************************************/
void do_sell( char_data *ch, char *argument )
{
    char arg[MIL], allarg[MIL];
    char_data *keeper;
    OBJ_DATA *obj, *obj_next;
	int count=0;

    one_argument( argument, arg );

	if (!str_cmp("all", arg )){
		strcpy(arg,"all.");
	}

    if ( !str_prefix( "all.", arg ) )
    {
		strcpy(allarg, arg+4);
	}

    if ( arg[0] == '\0' )
    {
		ch->println("Sell what?");
		return;
    }

	if ( ( keeper = find_keeper( ch ) ) == NULL )
		return;

    if ( str_prefix( "all.", arg) )
    {
		if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
		{
			act( "$n tells you 'You don't have that item'",keeper, NULL, ch, TO_VICT );
			return;
		}
	}
	else
	{
		if ( ( obj = get_obj_carry( ch, allarg ) ) == NULL )
		{
			ch->println( "No matching items found to sell." );
			return;
		}
	}

	if (!can_see(keeper,ch))
	{
		act("$n doesn't appear to be able to see you.",keeper,NULL,ch,TO_VICT);
		return;
	}

/*
	Slortar 24/03/02
	This check is handled in sell_object and does not exit the function when argument is 'all'
	if (!can_see_obj(keeper,obj))
	{
		act("$n doesn't appear to be able to see $p.",keeper,obj,ch,TO_VICT);
		return;
	}
*/

/************/
    if ( str_prefix( "all.", arg ) )
    {
		// 'sell obj' 
		if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
		{
			ch->println( "You do not have that item." );
			return;
		}
		sell_object(ch, keeper, obj);
	}
	else // 'sell all.obj' 
	{
		// PC being ordered to do this
		if ( !IS_NPC(ch) && IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
		{
			if ( ch->master ){
				ch->master->println( "Not going to happen." );
			}
			return;
		}

		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;

			if (is_name( allarg, obj->name)
			&&   can_see_obj( ch, obj )
			&&   obj->wear_loc == WEAR_NONE)
			{
				if(IS_NULLSTR(allarg) && get_cost( keeper, obj, false )<=0 ){ 
					// true if was a 'sell all' and keeper not interested in type of object
					continue;
				}
				if (count++>25)
				{
					ch->println( "You can only sell up to 25 items at a time." );
					return;
				}
				sell_object(ch, keeper, obj);
				
				// lag for selling multiple items
				WAIT_STATE(ch, 1);
				if (ch->fighting)
					WAIT_STATE(ch, 5);

			}
		}

		if(IS_NULLSTR(allarg) && count==0){ 
			act( "`s$n doesn't look interested in anything you have.`x", keeper, obj, ch, TO_VICT );
		}else if(count==25){
			act( "`sYou have nothing else that $n is interested in buying.`x", keeper, obj, ch, TO_VICT );
		}
	}
    return;
}

/**************************************************************************/
void do_value( char_data *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL];
    char_data *keeper;
    OBJ_DATA *obj;
    int cost;

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		ch->println( "Value what?" );
		return;
	}

	if ( ( keeper = find_keeper( ch ) ) == NULL )
		return;

	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
		act( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
		//ch->reply = keeper;
		return;
	}

	if (!can_see_obj(keeper,obj))
	{
		act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
		return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
		ch->println( "You can't let go of it." );
		return;
	}

	if ( ( cost = get_cost( keeper, obj, false ) ) <= 0 )
	{
		act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
		return;
	}

	sprintf( buf, "$n tells you 'I'll give you %d silver and %d gold coins for $p'.",
		cost - (cost/100) * 100, cost/100 );
	act( buf, keeper, obj, ch, TO_VICT );
	//ch->reply = keeper;

	return;
}

/**************************************************************************/
// by Rathern 
void do_slice( char_data *ch, char *argument )
{
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *slice;
	int lag;
	bool found;
	char buf[MSL];
	found = false;
	
	
	if ( !IS_NPC(ch) && 
		(ch->level < skill_table[gsn_slice].skill_level[ch->clss]
		|| ch->pcdata->learned[gsn_slice] < 1 ))   // skill is not known
	{
		ch->println("You are not learned in slicing meat from objects.");
		return;
	}
	
	if ( IS_NULLSTR(argument) )
	{ 
		ch->println("From what do you wish to slice meat?");
		return;
	}
	
	
	if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
		||   ( obj->value[3] != 1 && obj->value[3] != 2 && obj->value[3] != 3
		&& obj->value[3] != 11) )
	{
		ch->println("You need to wield a sharp weapon.");
		return;
	}
	
	if ( (corpse = get_obj_here( ch, argument )) == NULL)
	{  
		ch->println("You can't find that here.");
		return;
	}
	
	if (corpse->item_type != ITEM_CORPSE_NPC 
		&& corpse->item_type != ITEM_CORPSE_PC)
	{
		ch->println("That is not a suitable source of meat.");
		return;
	}
		
	// fiddle with the corpse after its been sliced to much 
	if (corpse->timer < 2)
	{
		sprintf( buf,"A sliced up and rotting %s is lying here.",
			corpse->short_descr);
		free_string(corpse->description);
		corpse->description=str_dup(buf);
		ch->println("That meat is to old and used now.");
		return;
	}
	
	
	if ( get_obj_index(OBJ_VNUM_SLICE) == NULL )
	{
		bugf("Vnum %d not found for do_slice!", OBJ_VNUM_SLICE);
		ch->printlnf( "Vnum %d not found for do_slice!, please report to the admin.", OBJ_VNUM_SLICE);
		return;
	}
	
	if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) && number_percent() >
		ch->pcdata->learned[gsn_slice] )
		
	{
		ch->println("You fail to slice the meat properly.");
		check_improve(ch,gsn_slice,false,1);
		
		// Just in case they die :> 
				
		if ( number_percent() + ch->modifiers[STAT_QU] - 13 < 10)
		{
			act("You cut yourself, ouch that hurt!.",ch,NULL,NULL,TO_CHAR);
			damage( ch, ch, obj->level / 2, gsn_slice, DAM_SLASH, false );
		}		
		return;
	}
	
    // make and restring the slice of meat 
    buf[0]='\0';
    strcat(buf,"a slice of raw meat from ");
    strcat(buf,corpse->short_descr);
    strcat(buf," is lying here.");
    slice = create_object( get_obj_index(OBJ_VNUM_SLICE));
	free_string(slice->description);
    slice->description=str_dup(buf);
    
    buf[0]='\0';
    strcat(buf,"a slice of raw meat from ");
    strcat(buf,corpse->short_descr);
	free_string(slice->short_descr);
    slice->short_descr=str_dup(buf);
	slice->timer = 24;				// make the meat rot in 1 day	
	
    act("$n cuts a slice of meat from $p.", ch, corpse, NULL, TO_ROOM);
    act("You cut a slice of meat from $p.", ch, corpse, NULL, TO_CHAR);
    
	
    // give the sliced meat to the character 
    obj_to_char(slice, ch);
	
    // make it so only 5 slices can be cut from any one corpse 	
    corpse->value[2]+=10;
    if (corpse->value[2] > 40)
    {
        corpse->timer = 1;
    }
	
    // delay depends on skill 
    lag = skill_table[gsn_slice].beats;
    if (!IS_NPC(ch))
    {
        lag -= (ch->pcdata->learned[gsn_slice] * 30 /100 );
    }
	// reduce the lag for first slicing
	if (corpse->value[2]==10){
		lag/=3;
	}else if (corpse->value[2]==20){
		lag/=2;
	}
	
    check_improve(ch,gsn_slice,true,1);
	
    WAIT_STATE(ch,lag);
    return;
}

/**************************************************************************/
// Kalahn, May 98 - based off do_pour()
void do_empty(char_data *ch, char *argument)
{
    char arg[MSL], buf[MSL];
    OBJ_DATA *out;

    argument = one_argument(argument,arg);
    
    if (IS_NULLSTR(arg))
    {
		ch->println("Empty what drink container?");
		return;
    }
	
	if ((out = get_obj_carry(ch,arg)) == NULL)
    {
		ch->println("You don't have that item.");
		return;
    }
	
	if (out->item_type != ITEM_DRINK_CON)
    {
		ch->println("That's not a drink container.");
		return;
	}
	
	if (out->value[1] == 0)
	{
		ch->println("It's already empty.");
		return;
	}
	
	out->value[1] = 0;
	out->value[3] = 0;
	sprintf(buf,"You invert $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_CHAR);
	
	sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_ROOM);
	return;
}

/**************************************************************************/
// By Kerenos & Kal - July 98
void do_throw( char_data *ch, char *argument )
{
    char arg1[MIL], arg2[MIL];
    OBJ_DATA		*obj;
    ROOM_INDEX_DATA	*in_room, *to_room;
    EXIT_DATA		*pexit;
    int			door;
	
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
	
    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2))
    {
		ch->println("Throw what in which direction?");
		ch->println("syntax: throw <object> <direction>");
		return;
    }

	if (IS_OOC(ch)){
		ch->println("You can't throw objects in OOC rooms.");
		return;
	}
	
    if (( obj = get_obj_carry( ch, arg1)) == NULL )
    {
		ch->println("You are not carrying that item.");
		return;
    }
	
    if ( !can_drop_obj( ch, obj ))
    {
		ch->println("You can't let go of that.");
		return;
    }

	door = dir_lookup( arg2 );
	
    if ( door == -1 )
	{
		ch->printlnf( "'%s' is an invalid direction.", arg2 );
		ch->println("Syntax: throw <object> <direction>");
		return;
    }

	in_room = ch->in_room;
	to_room = NULL;
	
	if ( (( pexit   = in_room->exit[door] ) == NULL
	||  ( to_room = pexit->u1.to_room   ) == NULL
	||  !can_see_room( ch, pexit->u1.to_room )) 
	&&  strcmp( arg2, "down" ))
	{
		ch->println("You cannot throw in that direction.");
		return;
    }

	if ( to_room == NULL )  {
	    act( "You throw $p to the floor.", ch, obj, NULL, TO_CHAR );
	    act( "$n throws $p to the floor.", ch, obj, NULL, TO_ROOM );

	    obj_from_char( obj );
		obj_to_room( obj, in_room );
	}
	else
	{
		if (IS_SET(to_room->room_flags, ROOM_OOC)){
			ch->println("You can't throw objects into OOC rooms.");
			return;
		}
	
	// Allow those with holywalk to throw thru closed doors 
	    if ( IS_SET( pexit->exit_info, EX_CLOSED )
			&& !IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK) )
	    {
			ch->println("Try opening the door first.");
			return;
	    }

	    act( "You throw $p $T.", ch, obj, dir_name[door], TO_CHAR );
	    act( "$n throws $p $T.", ch, obj, dir_name[door], TO_ROOM );
	    obj_from_char( obj );
	    obj_to_room( obj, to_room );
	
	    ch->in_room = to_room;
	    act( "With a resounding thud, $p lands in the room.", ch, obj, NULL, TO_ROOM );
	    ch->in_room = in_room;
	}

    if ( IS_OBJ_STAT( obj, OBJEXTRA_MELT_DROP ))
    {
		act( "$p dissolves into smoke.", ch, obj, NULL, TO_ROOM );
		extract_obj( obj );
    }

#ifdef unix
	if (!IS_IMMORTAL(ch))
	{
		WAIT_STATE( ch, PULSE_VIOLENCE );
	}
#endif
}

/**************************************************************************/
// mods by Kerenos 
void do_zap( char_data *ch, char *argument )
{
    OBJ_DATA  *wand;
    int sn;
	
    // new stuff here 
    char arg2[MIL];
    char_data *victim;
    OBJ_DATA *obj;
    void *vo;
    int target;
	
    strcpy(arg2, argument);
	
    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
        ch->println("You are holding nothing in your hands.");
        return;
    }
	
    if ( wand->item_type != ITEM_WAND )
    {
        ch->println("You can only zap with a wand.");
        return;
    }
	
	if ( IS_SET( ch->in_room->room_flags, ROOM_ANTIMAGIC )) {
		act( "$p doesn't seem to have any power.", ch, wand, NULL, TO_CHAR );
		return;
	}
	
    if (( sn = wand->value[3] ) < 0
		||   sn >= MAX_SKILL
		||   skill_table[sn].spell_fun == 0 )
    {
        bugf( "do_zap(): bad sn %d.", sn );
        return;
    }
	
    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
	
    if (wand->value[2] < -1){
		wand->value[2] = -1;
    }
	
    if (( wand->value[2] > 0 ) || (wand->value[2] = -1))
    {
        act( "$n holds out $p.", ch, wand, NULL, TO_ROOM );
        act( "You hold out $p.",  ch, wand, NULL, TO_CHAR );
		
        if ( ch->level < wand->level
			||  (get_skill(ch,gsn_wands)==0)
			||   number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5)
        {
            act ("Your efforts with $p produce only smoke and sparks.",ch,wand,NULL,TO_CHAR);
            act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
            check_improve(ch,gsn_wands,false,2);
        }    
        else // cast the spell 
        {   
            // Locate targets.
            victim  = NULL;
            obj     = NULL;
            vo      = NULL;
            target  = TARGET_NONE;
			
            switch ( skill_table[sn].target )
            {
            default:
                bugf( "do_zap(): bad target for sn %d.", sn );
                return;
				
            case TAR_IGNORE:
                break;
				
            case TAR_MOB_OFFENSIVE:
            case TAR_CHAR_OFFENSIVE:
                if ( arg2[0] == '\0' )
                {
                    if ( ( victim = ch->fighting ) == NULL )
                    {
                        ch->println("Zap whom?");
                        return;
                    }
                }
                else
                {
                    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
                    {
                        ch->println("They aren't here.");
                        return;
                    }
                }
				
                if ( !IS_NPC(ch) )
                {
					
                    if (is_safe(ch,victim) && victim != ch)
                    {
                        ch->println("Not on that target.");
                        return;
                    }
                }
				
                if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
                {
                    ch->println("You can't do that on your own follower.");
                    return;
                }
				
                vo = (void *) victim;
                target = TARGET_CHAR;
                break;
				
            case TAR_CHAR_DEFENSIVE:
                if ( arg2[0] == '\0' )
                {
                    victim = ch;
                }
                else
                {
                    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
                    {
						ch->println("They aren't here.");
						return;
                    }
                }
				
                vo = (void *) victim;
                target = TARGET_CHAR;
                break;
				
            case TAR_CHAR_SELF:
                if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) )
                {
                    ch->println("You can't zap yourself while aiming it at another.");
                    return;
                }
				
                vo = (void *) ch;
                target = TARGET_CHAR;
                break;
				
            case TAR_OBJ_INV:
                if ( arg2[0] == '\0' )
                {
                    ch->println("You can't find that.");
					return;
                }
				
                if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
                {
                    ch->println("You are not carrying that.");
                    return;
                }
				
                vo = (void *) obj;
                target = TARGET_OBJ;
                break;
				
            case TAR_OBJ_MOB_OFF:
            case TAR_OBJ_CHAR_OFF:
                if (arg2[0] == '\0')
                {
                    if ((victim = ch->fighting) == NULL)
                    {
                        ch->println("Zap whom or what?");
                        return;
                    }
					
                    target = TARGET_CHAR;
                }
                else if ((victim = get_char_room(ch,arg2)) != NULL)
                {
                    target = TARGET_CHAR;
                }
				
                if (target == TARGET_CHAR) // check the sanity of the attack 
                {
                    if(is_safe_spell(ch,victim,false) && victim != ch)
                    {
						ch->println("Not on that target.");
						return;
                    }
					
					if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
					{
						ch->println("You can't do that on your own follower.");
						return;
					}
					
					
                    vo = (void *) victim;
                }
                else if ((obj = get_obj_here(ch,arg2)) != NULL)
                {
                    vo = (void *) obj;
                    target = TARGET_OBJ;
                }
                else
                {
                    ch->println("You don't see that here.");
                    return;
                }
                break; 
				
            case TAR_OBJ_CHAR_DEF:
				if (arg2[0] == '\0')
				{
					vo = (void *) ch;
					target = TARGET_CHAR;                                                 
				}
				else if ((victim = get_char_room(ch,arg2)) != NULL)
				{
					vo = (void *) victim;
					target = TARGET_CHAR;
                }
                else if ((obj = get_obj_carry(ch,arg2)) != NULL)
                {
                    vo = (void *) obj;
                    target = TARGET_OBJ;
                }
                else
                {
                    ch->println("You don't see that here.");
                    return;
                }
                break;
            }// end of spell type switch
			
            if (IS_NPC(ch) || class_table[ch->clss].fMana)
				// clss has spells
                (*skill_table[sn].spell_fun) ( sn, wand->value[0], ch, vo,target);
            else
                (*skill_table[sn].spell_fun) (sn, wand->value[0], ch, vo,target);
            check_improve(ch,gsn_wands,true,2);
			
        }
    }
	
    // get rid of the staff if required 
    if ( wand->value[2] == -1)
    {
        return;
    }
    wand->value[2]--;
	
    if (wand->value[2] < 1)
    {
        act( "$p blazes bright and is gone.", ch, wand, NULL, TO_ROOM );
        act( "$p blazes bright and is gone.", ch, wand, NULL, TO_CHAR );
        extract_obj( wand );
    }
	
    return;
}

/************************************************************************/
void do_place( char_data *ch, char *argument)
{
	char arg1 [MIL];
	char arg2 [MIL];
	char_data *victim;
	OBJ_DATA  *obj;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	
	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		ch->println("Place what on whom?");
		return;
	}
	
    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        ch->println("You do not have that item.");
        return;
    }
	
    if ( obj->wear_loc != WEAR_NONE )
    {
        ch->println("You must remove it first.");
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
		ch->println("You can't let go of it.");
		return;
	}

	int	chance = get_skill(ch, gsn_place);
	if (!str_cmp( arg2, "ground"))
	{
		char_data *vch, *vch_next;
		int lowerchance;

		for ( vch = char_list; vch != NULL; vch = vch_next )
		{
			vch_next = vch->next;
			lowerchance=0;

			if ( vch->in_room == NULL )
				continue;
			if ( vch->in_room == ch->in_room )
			{
				if(vch->pcdata && vch->pcdata->is_trying_aware)
					lowerchance = (get_skill(vch, gsn_awareness))/6;
				
				lowerchance += (vch->level - ch->level)/3;

				if(IS_IMMORTAL(vch) || (number_percent()+lowerchance > chance && str_cmp(ch->name, vch->name))){
					vch->printlnf( "You notice %s drop %s", ch->short_descr, obj->short_descr);
					check_improve(vch,gsn_awareness,true,1);
				}
			}
		}

		// silently drop it
		{
			int old_invis_level;
			SET_BIT(ch->dyn,DYN_SILENTLY);
			// back up the wizi level
			old_invis_level= ch->invis_level;
			// hide it from mortals
			ch->invis_level= UMAX(ch->invis_level, LEVEL_IMMORTAL);
			// drop the object
			do_drop(ch, arg1);
			// restore the wizi level
			ch->invis_level=old_invis_level;
		}

		WAIT_STATE( ch, skill_table[gsn_place].beats );
//shouldn't check improve if all they are doing is dropping it
		return;
	}

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        ch->println("They aren't here.");
        return;
    }

	if (!IS_NPC(victim) && victim->level<10 && ch->level>(victim->level+10)
		&& !IS_NPC(ch) && !IS_IMMORTAL(ch))
	{
		ch->println("A faint wall of light appears to surround them preventing you from placing that on them.");
		return;
	}

    if ( victim == ch )
    {
        ch->println("That's pointless.");
        return;
    }
	
    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
        act("$N tells you 'Sorry, you'll have to sell that.'",
			ch,NULL,victim,TO_CHAR);
        return;
    }
	
    if ( !IS_CONTROLLED(victim) && victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
        act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
        return;
    }
	
    if (!IS_CONTROLLED(victim) && get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
        act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
        return;
    }
	
	if(victim->pcdata && victim->pcdata->is_trying_aware)
		chance -= (get_skill(victim, gsn_awareness))/6;
	if(!IS_AWAKE(victim))
		chance += 50;

	if(number_percent() < chance+((ch->level - victim->level)/3))
	{
		obj_from_char( obj );
		obj_to_char( obj, victim );
		ch->printlnf( "You successfully place %s on %s.", obj->short_descr, victim->short_descr );
		check_improve(ch, gsn_place, true, 1);
		WAIT_STATE( ch, skill_table[gsn_place].beats );
		return;
	}
	
	act( "$n tries to sneak $p to $N.", ch, obj, victim, TO_NOTVICT );
	act( "You try to sneak $p to $N but get caught!", ch, obj, victim, TO_CHAR    );

	if (IS_AWAKE(victim)){
		act( "$n tries to sneak $p into your inventory!",   ch, obj, victim, TO_VICT    );
		if(victim->pcdata && victim->pcdata->is_trying_aware )
			check_improve(victim,gsn_awareness,true,1);
	}
    else{
		if( victim->pcdata && victim->pcdata->is_trying_aware
		 && get_skill(victim, gsn_awareness) > number_percent() )
		{
			victim->printlnf( "You feel the presence of another person lurking near you.\r\n"
				"You feel someone messing with your things and instantly pop awake!" );
			victim->position=POS_RESTING;
			do_stand(victim,"");
			check_improve(victim,gsn_awareness,true,2);
		}else{
			victim->printlnf( "You are partially awakened by someone in the room,\r\n"
				          "you roll over and drift back to sleep." );
		}
	}
	
	affect_strip ( ch, gsn_invisibility					);
	affect_strip ( ch, gsn_mass_invis					);
	affect_strip ( ch, gsn_sneak						);
	REMOVE_BIT   ( ch->affected_by, AFF_HIDE			);
	REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE		);
	REMOVE_BIT   ( ch->affected_by, AFF_SNEAK			);
	
	if (!IS_NPC(victim))
	{
		ch->pksafe=0;
		ch->pknorecall= (UMAX(ch->pknorecall,2));
		ch->pknoquit=(UMAX(ch->pknoquit,10));
	}

	check_improve(ch, gsn_place, false, 1);
	WAIT_STATE( ch, skill_table[gsn_place].beats*3/2 );

    return;
}
/************************************************************************/
void do_attune( char_data *ch, char *argument)
{
	OBJ_DATA *obj;
	int modifier = 0;
	int percent  = number_percent();
	int failure  = 0;
	int seconds;

    if ( IS_NULLSTR( argument ))
    {
		ch->println( "Attune to which item?" );
		return;
    }
	
	if ( IS_NPC( ch ))
	{
		ch->println( "Players only!" );
		return;
	}

    obj =  get_obj_list(ch,argument,ch->carrying);
	
    if (obj== NULL)
    {
        ch->println( "You don't have that item." );
        return;
    }
	
	if ( !IS_SET( obj->attune_flags, ATTUNE_NEED_TO_USE ))
	{
			ch->println( "You don't need to attune yourself to that." );
			return;
	}

	if ( obj->attune_id == ch->player_id )
	{
		act( "You have already attuned yourself to $p.", ch, obj, NULL, TO_CHAR );
		return;
	}

	if ( IS_SET( obj->attune_flags, ATTUNE_PREVIOUS ))
	{
		if ( IS_SET( obj->attune_flags, ATTUNE_ONCE_ONLY ))
		{
			act( "$p cannot be reattuned, it only obeys one master.", ch, obj, NULL, TO_CHAR );
			return;
		}
	}

	if ( IS_SET( obj->attune_flags, ATTUNE_EQUAL_LEVEL ))
	{
		if (obj->level > ch->level)
		{
			act( "You lack the power to attune yourself to $p.", ch, obj, NULL, TO_CHAR );
			return;
		}
	}

	if ( current_time < obj->attune_next )
	{
		act( "You cannot attune to $p yet.", ch, obj, NULL, TO_CHAR );
		return;
	}

	// ATTUNE mods

	modifier = ( ch->level - obj->level );

	if ( modifier > 0 ) modifier *= 3;		// +3% per positive level diff
	else modifier *=5;						// -5% per negative level diff

	modifier += 50;							// 50% mean chance
											// EM+EM+PR/3 bonus
	modifier += (( ch->modifiers[STAT_EM] +  ch->modifiers[STAT_EM] +ch->modifiers[STAT_PR] )/3);
											// LORE bonus
	modifier += ((( get_skill( ch, gsn_lore) - 50 ) / 5) +
		(get_skill(ch, gsn_lore) > 0) ? 5 : 0);  // +5% if lore > 50... range is -10 to +15%

	if ( IS_SET( obj->attune_flags, ATTUNE_TRIVIAL ))
		modifier += 25;
	if ( IS_SET( obj->attune_flags, ATTUNE_EASY ))
		modifier += 10;
	if ( IS_SET( obj->attune_flags, ATTUNE_HARD ))
		modifier -= 10;
	if ( IS_SET( obj->attune_flags, ATTUNE_INFURIATING ))
		modifier -= 25;
	if ( IS_SET( obj->attune_flags, ATTUNE_PREVIOUS ))
		obj->attune_modifier -= 10;
	
	modifier += obj->attune_modifier;

	failure = percent - modifier;

	// flag it as previously attempt at attunement for next time :)
	SET_BIT( obj->attune_flags, ATTUNE_PREVIOUS );

	// they failed, let's set the next attune time (in ticks)
	if ( failure > 0 )
	{
		if ( IS_SET( obj->attune_flags, ATTUNE_VANISH ))
		{
			act( "$p glows brightly for an instant, then crumbles to dust.", ch, obj, NULL, TO_CHAR );
			act( "A flash of light emanates from $n and is gone in an instant.", ch, NULL, NULL, TO_ROOM );
			extract_obj(obj);
			return;
		}

		seconds = obj->level * 3600;		// 1 hour per object level base
		
		if ( failure > 30 )
		{
			obj->attune_modifier -= 25;
			seconds *= 2;
		}
		else if ( failure > 10 )
		{
			obj->attune_modifier -= 10;
			seconds *= 3/2;
		}
		else if ( failure > 0 )
		{
			obj->attune_modifier += 10;
			seconds /= 2;
		}
		if ( IS_SET( obj->attune_flags, ATTUNE_TRIVIAL ))
			seconds /= 4;
		if ( IS_SET( obj->attune_flags, ATTUNE_EASY ))
			seconds /= 2;
		if ( IS_SET( obj->attune_flags, ATTUNE_HARD ))
			seconds *= 2;
		if ( IS_SET( obj->attune_flags, ATTUNE_INFURIATING ))
			seconds *= 3;

		obj->attune_next = current_time + seconds;

		act( "You try to exert your will upon $p but were unable to attune yourself to it.", ch, obj, NULL, TO_CHAR );
	
		if ( !is_affected( ch, gsn_cause_headache ))
		{
			AFFECT_DATA af;

			ch->println( "Your head seems to explode with a sudden wave of indescribable pain!" );
			af.where		= WHERE_MODIFIER;
			af.type			= gsn_cause_headache;
			af.level		= obj->level;
			af.duration		= obj->level;
			af.location		= APPLY_SD;
			af.modifier		= - obj->level/5;
			af.bitvector	= 0;
			affect_to_char( ch, &af );
		}
		act( "$p pulsates softly and falls to the ground.", ch, obj, NULL, TO_CHAR );
		act( "$n holds $s head in pain and drops $p.", ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		return;
	}
	else
	{
		obj->attune_id = ch->player_id;
		obj->attune_next = 0;
		act( "A bright flash envelops $p as you concentrate on it.", ch, obj, NULL, TO_CHAR );
		act( "You have become one with $p.", ch, obj, NULL, TO_CHAR );
		act( "$n is bathed in a flash of light as $e holds $p.", ch, obj, NULL, TO_ROOM );
	}

	return;
}
/**************************************************************************/
void letter_read( char_data *ch, OBJ_DATA *letter );
/**************************************************************************/
// Show an object to another player - Kal Mar 01
void do_show( char_data *ch, char *argument)
{
	char arg[MIL];
	char *pdesc;
	char objname[MIL];
	int number,count;
	OBJ_DATA *obj;

	argument=one_argument(argument, arg);
	number = number_argument(arg,objname);
	count = 0;

	if(IS_NULLSTR(argument)){
		ch->println("Show what object to whom?");
		ch->println("hint: if you want to display your affects at the bottom of your score use the `=Cshowaffects`x command");
		return;
	}

	char_data *victim=get_char_room(ch, argument);
	if(!victim){
		ch->printlnf("Could not find any '%s' in the room to show '%s' to.",
			argument, objname);
		return;
	}

	if(!IS_AWAKE(victim)){
		ch->printlnf("%s doesnt appear to be awake.", capitalize(PERS(victim, ch)));
		return;
	}

	if(!can_see(victim, ch)){
		ch->printlnf("%s doesn't appear to be able to see you.", capitalize(PERS(victim, ch)));
		return;
	}

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) )
        {  // player can see object
			
			// parchments take highest priority
            if ( obj->item_type == ITEM_PARCHMENT && is_name( objname, obj->name ) )
            {
                if (++count == number)
                {
					if(can_see_obj( victim, obj )){
						ch->printlnf("You show %s to %s.", obj->short_descr, PERS(victim, ch));
						victim->printlnf("`W%s shows you %s, it reads:`x",	capitalize(PERS(ch, victim)), obj->short_descr);
						letter_read( victim, obj );
					}else{
						ch->printlnf("It doesn't appear as though %s can see %s.",
							PERS(victim, ch), obj->short_descr);
					}
                    return;
                }
            }
            // use only one of the sets of extended descriptions 
            if (obj->extra_descr)
            { // unique objects extended descriptions 
                pdesc = get_extra_descr( objname, obj->extra_descr );
                if ( pdesc != NULL )
                {
                    if (++count == number)
                    {
						if(can_see_obj( victim, obj )){
							ch->printlnf("You show %s to %s.", obj->short_descr, PERS(victim, ch));
							victim->printlnf("`W%s shows you %s:`x", capitalize(PERS(ch, victim)), obj->short_descr);
	                        victim->sendpage(pdesc);
						}else{
							ch->printlnf("It doesn't appear as though %s can see %s.",
								PERS(victim, ch), obj->short_descr);
						}
                        return;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else // vnums extended descriptions
            {        
                pdesc = get_extra_descr( objname, obj->pIndexData->extra_descr );
                if ( pdesc != NULL )
                {
                    if (++count == number)
                    { 
						if(can_see_obj( victim, obj )){
							ch->printlnf("You show %s to %s.", obj->short_descr, PERS(victim, ch));
							victim->printlnf("`W%s shows you %s:`x", capitalize(PERS(ch, victim)), obj->short_descr);
	                        victim->sendpage(pdesc);
						}else{
							ch->printlnf("It doesn't appear as though %s can see %s.",
								PERS(victim, ch), obj->short_descr);
						}
                        return;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
			
            if ( is_name( objname, obj->name ) )
            {
                if (++count == number)
                {
					if(can_see_obj( victim, obj )){
						ch->printlnf("You show %s to %s.", obj->short_descr, PERS(victim, ch));
						victim->printlnf("`W%s shows you %s:`x", capitalize(PERS(ch, victim)), obj->short_descr);
	                    victim->printlnf( "%s", obj->description );
					}else{
						ch->printlnf("It doesn't appear as though %s can see %s.",
							PERS(victim, ch), obj->short_descr);
					}
                    return;
                }
            }
        }
		
    }
	ch->printlnf("You possess no '%s' object to show %s.", arg, PERS(victim, ch));
}
/************************************************************************/
/************************************************************************/
/************************************************************************/
/************************************************************************/
