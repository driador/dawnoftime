/**************************************************************************/
// pushdrag.cpp - push and drag functionality - by Meerclar
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************/
#include "include.h"

// This code needs a little more work, the following should be implemented:
// * racial size checks for mobs/players
// * to awake players/mobs if dragged - even an attempt
// * if a player doesn't have passdoor they should never go thru a door
// * if ch has passdoor and dragging, they should go thru door even if the
//   person they arent dragging doesn't.
// * if pushing someone with pass door, there should be a chance you pass
//   thru them... other times if you don't have passdoor they should pushable
//   thru a closed exit but you don't follow.
// * trigger mudprog exit triggers
// * trigger mudprog entry triggers
// * player pushing echoes shohuld be like object echoes.
/**************************************************************************/
void push_drag( char_data *ch, char *argument, char *verb )
{
	char arg1[MSL];
	char arg2[MSL];
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	char_data *victim;
	EXIT_DATA *pexit;
	OBJ_DATA *obj=NULL;
	int door;
	bool moving_object=false;
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	
	if(IS_NULLSTR(arg1) || IS_NULLSTR(arg2)){
		ch->printlnf("syntax: %s <name> <direction>", lowercase(verb));
		ch->printlnf("This command can be used to %s players/mobs/objects into other rooms.", 
			capitalize(verb));
		return;
	}

	victim = get_char_room(ch,arg1);
	if(!victim){
		obj = get_obj_list( ch, arg1, ch->in_room->contents );
		if(!obj){
			ch->printlnf("There is no '%s' in the room which you can see to %s.",
				arg1, lowercase(verb));
			push_drag(ch, "", verb);
			return;
		}
		moving_object=true;
	}

	// check they have the movement points required
	if(ch->move<10){
		ch->printlnf("You are too tired to %s any%s around!", verb,
			moving_object?"thing":"body");
		return;
	}

	// lookup the direction
	door=dir_lookup(arg2);
	if(door<0){		
		ch->printlnf("'%s' is not a recognised direction.", arg2);
		push_drag(ch, "", verb);
		return;
	}
	
	// check the direction is appropriate	
	in_room=ch->in_room;
	pexit=in_room->exit[door];
	if(!pexit || !pexit->u1.to_room || !can_see_room(ch,pexit->u1.to_room)){
		ch->printlnf("There is no exit to the %s.", dir_name[door]);
		return;
	}
	to_room = pexit->u1.to_room;

	
	if(moving_object){
		if(IS_SET(pexit->exit_info, EX_CLOSED|EX_NOPASS)){
			act( "You cannot $t it through the $d.", ch, verb, pexit->keyword, TO_CHAR );
			act( "$n decides to $t $P around!",  ch, verb, obj, TO_ROOM );
			return;
		}
				
		act( "You attempt to $T $p out of the room.",ch,  obj, verb, TO_CHAR );
		act( "$n is attempting to $T $p out of the room.",ch,   obj, verb, TO_ROOM );
		
		if ( obj->weight >  (2 * can_carry_w(ch)) ){
	           act( "$p is too heavy to $T.",ch,  obj, verb, TO_CHAR);
			   act( "$n attempts to $T $p, but it is too heavy.",ch,  obj, verb, TO_ROOM);
			   return;
		}

		// do some failure checks which relate to morts only
		if(!IS_IMMORTAL(ch)){
			if( IS_SET(in_room->room_flags, ROOM_SAFE|ROOM_PRIVATE|ROOM_SOLITARY)
				|| IS_SET(to_room->room_flags, ROOM_SAFE|ROOM_PRIVATE|ROOM_SOLITARY))
			{
				act( "$p is not moving.",ch,  obj, verb, TO_CHAR );
				act( "$n doesn't appear to have any success.",ch,   obj, verb, TO_ROOM );
				return;
			}			
		}

		// success pushing an object
		ch->move -= 10;
		ch->println( "You succeed!" );
		act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
		if (!str_cmp( verb, "drag" )){
			act( "$n drags $p $T!", ch, obj, dir_name[door], TO_ROOM );
			char_from_room( ch );
			char_to_room( ch, pexit->u1.to_room );
			obj_from_room( obj );
			obj_to_room( obj, to_room );
			act( "$n drags $p into the room.", ch, obj, dir_name[door], TO_ROOM );
		}else{			
			act( "$p is pushed %T of the room by $n!", ch, obj, dir_name[door], TO_ROOM );
			char_from_room( ch );
			char_to_room( ch, to_room);
			obj_from_room( obj );
			obj_to_room( obj, to_room);
			act( "$p is pushed into the room by $n!", ch, obj, dir_name[door], TO_ROOM );
		}
		return; // end of pushing/draging an object
	}
	
	// pushing a mob/player, first lets do some idiot checks :)
	if ( ch == victim )	{		
		act( "You $t yourself about the room and look very silly.", ch, verb, NULL, TO_CHAR );
		act( "$n decides to be silly and $t $mself about the room.", ch, verb, NULL, TO_ROOM );
		return;
	}

	act( "You attempt to $t $N out of the room.", ch, verb, victim, TO_CHAR );
	act( "$n is attempting to $t you out of the room!", ch, verb, victim, TO_VICT );
	act( "$n is attempting to $t $N out of the room.", ch, verb, victim, TO_NOTVICT );

	if(!IS_IMMORTAL(ch)){
		// cant push around mobs which provide services
		if(IS_NPC(victim)
			&& (IS_SET(victim->act,ACT_TRAIN | ACT_PRACTICE | ACT_IS_HEALER)
			|| IS_SET(victim->imm_flags,IMM_SUMMON)
			|| IS_KEEPER(victim)))
		{
			act( "$N is not moving.",ch, victim, NULL, TO_CHAR );
			act( "$n doesn't appear to have any success.",ch, NULL, NULL, TO_ROOM );
			return;
		}

		if( IS_SET(in_room->room_flags, ROOM_SAFE|ROOM_PRIVATE|ROOM_SOLITARY)
			|| IS_SET(to_room->room_flags, ROOM_SAFE|ROOM_PRIVATE|ROOM_SOLITARY))
		{
			act( "$N is not moving.",ch, victim, NULL, TO_CHAR );
			act( "$n doesn't appear to have any success.",ch, NULL, NULL, TO_ROOM );
			return;
		}			

		if((!str_cmp( verb, "push" ) && victim->position != POS_STANDING)
		||  is_safe(ch,victim)
		||  (number_percent() <90)
		||   (victim->max_hit > (ch->max_hit + (ch->perm_stats[STAT_ST])*20)) )
		{
			act( "$N is not moving.",ch, victim, NULL, TO_CHAR );
			act( "$n doesn't appear to have any success.",ch, NULL, NULL, TO_ROOM );
			return;
		}
	}
	

	ch->move -= 10;
	ch->println( "You succeed!" );
	act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
	if (!str_cmp( verb, "drag" )){		
		move_char( ch, door, true);
		act( "$n is dragged $T!", victim, NULL, dir_name[door], TO_ROOM );
		act( "You are dragged $T!", victim, NULL, dir_name[door], TO_CHAR );
		char_from_room( victim );
		char_to_room( victim, to_room);
		act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
	}else if (!str_cmp( verb, "push" )){		
		act( "$n `Wflies`x $T!", victim, NULL, dir_name[door], TO_ROOM );
		act( "You `Wfly`x $T!", victim, NULL, dir_name[door], TO_CHAR );
		char_from_room( victim );
		char_to_room( victim, pexit->u1.to_room );
		act( "$n `Wflies`x into the room!", victim, NULL, NULL, TO_ROOM );
	}
}
/**************************************************************************/               
void do_push( char_data *ch, char *argument )
{
	push_drag( ch, argument, "push" );
}

/**************************************************************************/
void do_drag( char_data *ch, char *argument )
{
	push_drag( ch, argument, "drag" );
}

/**************************************************************************/
/**************************************************************************/

