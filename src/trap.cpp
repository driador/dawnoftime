/**************************************************************************/
// trap.cpp - Kerenos's Trap code
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "trap.h"
#include "olc.h"

// very little commenting, if you don't know what's going on here, you have
// no business looking at this code since it's pretty darned basic :)

int get_sublevels_for_level(int level);
void make_corpse( char_data *ch, char_data *killer );
void trapdamage(char_data *ch, OBJ_DATA *obj);
void do_fearful(char_data *ch );

void do_trapset( char_data *ch, char *argument)
{
	OBJ_DATA *obj;
	bool changed = false;
	char arg1[MIL];
	char arg2[MIL];
	char arg3[MIL];

	if (!HAS_SECURITY(ch,1))
	{
		do_huh( ch, "" );
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	
	if (arg1[0] == '\0' || arg2[0] == '\0'  )
	{
		ch->println("Syntax: trapset <object> <`=Cfield`x> <`yvalue`x>");
		ch->println("`=CField`x: move, get, room, open, damage, charge, modifier");
		ch->println("`yValue`x: Move> north, south, east, west, up, down,");
		ch->println("             northeast, northwest, southeast, southwest, all");
		ch->println("       Damage> help dam-type");
		ch->print("       Modifier> Difficulty of trap ( +/-75  pos is easy, neg is harder )");
		ch->println("       get, open, room> no values");
		return; 
    }
	
	if ( ( obj = get_obj_here(ch, arg1) ) == NULL)
	{
		ch->println("You must be in the same room as the object you are trapping.");
		return;
	}

	if ( !IS_BUILDER( ch, obj->pIndexData->area, BUILDRESTRICT_OBJECTS ) && !IS_IMMORTAL(ch))
	{
		ch->println("trapset: Invalid security for setting traps on object. You are not");
		ch->println("         a valid builder for this area.");
		return;
	}
	
	if (!str_cmp( arg2, "move") )
	{
		int dir = 0;

		if (arg3[0] == '\0')
		{
			ch->println("Syntax: trapset <object> move <direction>");
			return;
		}

		if (!str_cmp( arg3, "all" ))
		{
			dir = 10;
		}
		else
		{
			dir = dir_lookup( arg3 );
		}

		switch ( dir )
		{
		case DIR_NORTH:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_NORTH );
			ch->println("You set a trap for northward movement!");
			break;
		case DIR_EAST:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_EAST );
			ch->println("You set a trap for eastward movement!");
			break;
		case DIR_WEST:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_WEST );
			ch->println("You set a trap for westward movement!");
			break;
		case DIR_SOUTH:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_SOUTH );
			ch->println("You set a trap for southward movement!");
			break;
		case DIR_UP:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_UP );
			ch->println("You set a trap for upward movement!");
			break;
		case DIR_DOWN:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_DOWN );
			ch->println("You set a trap for downward movement!");
			break;
		case DIR_NORTHEAST:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_NORTHEAST );
			ch->println("You set a trap for northeastern movement!");
			break;
		case DIR_NORTHWEST:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_NORTHWEST );
			ch->println("You set a trap for northwestern movement!");
			break;
		case DIR_SOUTHEAST:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_SOUTHEAST );
			ch->println("You set a trap for southeastern movement!");
			break;
		case DIR_SOUTHWEST:
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_SOUTHWEST );
			ch->println("You set a trap for southwest movement!");
			break;
		case 10:	// hack for all directions *blush* well, it's cute... :)
			SET_BIT( obj->trap_trig, TRAP_TRIG_MOVE );
			SET_BIT( obj->trap_trig, TRAP_TRIG_NORTH );
			SET_BIT( obj->trap_trig, TRAP_TRIG_EAST );
			SET_BIT( obj->trap_trig, TRAP_TRIG_SOUTH );
			SET_BIT( obj->trap_trig, TRAP_TRIG_WEST );
			SET_BIT( obj->trap_trig, TRAP_TRIG_UP );
			SET_BIT( obj->trap_trig, TRAP_TRIG_DOWN );
			SET_BIT( obj->trap_trig, TRAP_TRIG_NORTHEAST );
			SET_BIT( obj->trap_trig, TRAP_TRIG_NORTHWEST );
			SET_BIT( obj->trap_trig, TRAP_TRIG_SOUTHEAST );
			SET_BIT( obj->trap_trig, TRAP_TRIG_SOUTHWEST );
			ch->println("You set a trap for ALL movement!!!");
			break;
		case -1:	// invalid direction type
			ch->println("Invalid direction.  Try north, east, south, west, up, down");
			ch->println("  northeast, northwest, southeast, southwest, or all");
			break;
		default:
			ch->println("Something strange happened, note to code about traps and invalid directions.");
			break;
		}
		changed = true;
	}

	if ( !str_cmp( arg2, "get" ))
	{
		SET_BIT( obj->trap_trig, TRAP_TRIG_OBJECT );
		ch->println("The trap will now spring when someone tries to `=Cget`x this object.");
		changed = true;
	}
	
	if ( !str_cmp( arg2, "room" ))
	{
		SET_BIT( obj->trap_trig, TRAP_TRIG_ROOM );
		ch->println("The trap will now affect the whole room, area affect!");
		changed = true;
	}
	
	if ( !str_cmp( arg2, "open" ))
	{
		SET_BIT( obj->trap_trig, TRAP_TRIG_OPEN );
		ch->println("The trap will trigger when the object is opened!");
		changed = true;
	}

	if ( !str_cmp( arg2, "modifier" ))
	{
		int chance;

		if ( arg3[0] == '\0' )
		{
			ch->println("No modifier specified.  Use any number from -75 to 75.");
			ch->println("  The modifier number will be added to the player's remove traps");
			ch->println("  skill, so negative numbers make the trap harder to disarm, and");
			ch->println("  positive ones make the trap easier to remove.");
			return;
		}

		if (( chance = atoi(arg3) ) > 75 || chance < -75 )
		{
			ch->println("Current allowed range is -75 to 75.");
			return;
		}
		obj->trap_modifier = chance;
		ch->printlnf("Trap modifier now set to %d.", chance );
		changed = true;
	}

	if ( !str_cmp( arg2, "damage" ))
	{
		int dtype;

		if ( arg3[0] == '\0' )
		{
			ch->println("No dam-type specified.  Try one of the following.");
			do_help( ch, "dam-type" );
			return;
		}

		dtype = flag_value( damtype_types, arg3 );
	
		if ( dtype ==NO_FLAG )
		{
			ch->printlnf("'%s' is not a recognized damtype.", arg3 );
			do_help( ch, "dam-type" );
			return;
		}

		ch->printlnf("The trap will now do damage of type '%s'.",
			flag_string( damtype_types, dtype ));

		obj->trap_dtype = dtype;
		changed = true;
	}

		
	if ( !str_cmp( arg2, "charge" ))
	{
		int val = 0;

		if (arg3[0] == '\0')
		{
			ch->println("You need to specify how many charges.");
			return;
		}

		if (( val = atoi(arg3) ) > 100 || val < 0)
		{
			ch->println("Current allowed range is 1 to 100.");
			return;
		}

		obj->trap_charge = val;
		ch->printlnf("The trap now has %d charges.", val );
		changed = true;
	}

	if ( !changed )
	{
		do_trapset( ch, "" );	// redisplay the syntax
	}
	else
	{
		SET_BIT( obj->extra2_flags, OBJEXTRA2_TRAP );
		SET_BIT( obj->pIndexData->extra2_flags, OBJEXTRA2_TRAP );
		obj->pIndexData->trap_trig		= obj->trap_trig;
		obj->pIndexData->trap_dtype		= obj->trap_dtype;
		obj->pIndexData->trap_charge	= obj->trap_charge;
		obj->pIndexData->trap_modifier	= obj->trap_modifier;
		// area changed, saved in next maintenace
		SET_BIT( obj->pIndexData->area->olc_flags, OLCAREA_CHANGED );
	}
	return;
}

/**************************************************************************/
void do_trapremove(char_data *ch, char *argument)
{
	OBJ_DATA *obj;
	char arg1[MIL];
	
	argument = one_argument(argument, arg1);

	if (!HAS_SECURITY(ch,1))
	{
		do_huh( ch, "" );
		return;
	}
	
	if (( obj = get_obj_here( ch, arg1 )) == NULL)
	{
		ch->println("That isn't here!");
		return;
	}

	if ( !IS_BUILDER( ch, obj->pIndexData->area, BUILDRESTRICT_OBJECTS ) && !IS_IMMORTAL(ch))
	{
		ch->println("trapremove: Invalid security for setting traps on object. You are not");
		ch->println("            a valid builder for this area.");
		return;
	}

	if ( !IS_SET( obj->extra2_flags, OBJEXTRA2_TRAP ))
	{
		ch->println("That object was not marked as a trap, no change ocurred.");
		return;
	}


	REMOVE_BIT(obj->extra2_flags, OBJEXTRA2_TRAP);
	REMOVE_BIT(obj->pIndexData->extra2_flags, OBJEXTRA2_TRAP);
	SET_BIT( obj->pIndexData->area->olc_flags, OLCAREA_CHANGED );	

	obj->trap_dtype		= 0;
	obj->trap_trig		= 0;
	obj->trap_charge	= 0;
	obj->trap_modifier	= 0;
	obj->pIndexData->trap_trig		= obj->trap_trig;
	obj->pIndexData->trap_dtype		= obj->trap_dtype;
	obj->pIndexData->trap_charge	= obj->trap_charge;
	obj->pIndexData->trap_modifier	= obj->trap_modifier;

	ch->println("Trap removed.");
	return;
}

/**************************************************************************/
void do_trapshow(char_data *ch, char *argument)
{
	char arg1[MIL];
	OBJ_DATA *obj;
	
	argument = one_argument(argument, arg1);

	if (!HAS_SECURITY(ch,1))
	{
		do_huh( ch, "" );
		return;
	}
	
	if (( obj = get_obj_here(ch, arg1) ) == NULL)
	{
		ch->println("That is not here!");
		return;
	}

	if ( !IS_BUILDER( ch, obj->pIndexData->area, BUILDRESTRICT_OBJECTS ) && !IS_IMMORTAL(ch))
	{
		ch->println("trapset: Invalid security for setting traps on object. You are not");
		ch->println("         a valid builder for this area.");
		return;
	}


	if ( !IS_TRAPPED( obj ))
	{
		ch->println("That object in not registered as a trap.");
		return;
	}

	ch->printlnf("`yItem: `x%s   `yVNUM: `x%d",
		obj->short_descr, obj->pIndexData->vnum );

	ch->printlnf("`yDamage Type: `x%s",
		flag_string( damtype_types, obj->trap_dtype ));

	if ( obj->trap_trig == 0 )
	{
		ch->println("`RThe trap has no trigger set!`x");
	}
	else
	{
		if ( IS_SET( obj->trap_trig, TRAP_TRIG_MOVE ))
		{
			ch->print("`yThis trap is triggered when a player moves:`x\r\n  ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_NORTH	 ))
				ch->print("north ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_EAST		 ))
				ch->print("east ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_SOUTH	 ))
				ch->print("south ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_WEST		 ))
				ch->print("west ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_UP		 ))
				ch->print("up ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_DOWN		 ))
				ch->print("down ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_NORTHEAST ))
				ch->print("northeast ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_NORTHWEST ))
				ch->print("northwest ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_SOUTHEAST ))
				ch->print("southeast ");
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_SOUTHWEST ))
				ch->print("southwest ");
			ch->println("");
		}

		if ( IS_SET( obj->trap_trig, TRAP_TRIG_OBJECT ))
			ch->println("`yThe trap is set when a player `=Cgets`y this object.");
		
		if ( IS_SET( obj->trap_trig, TRAP_TRIG_OPEN ))
			ch->println("`yThe trap is set off when object is `=Copened`y.");
		
		if ( IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
			ch->println("`RThe trap affects the whole room!");
	}
	ch->printlnf("`yTrap Charges left: `x%d", obj->trap_charge );
	ch->printlnf("`yTrap Modifier:     `x%d", obj->trap_modifier );
    return;
}

/**************************************************************************/
bool trapcheck_get( char_data *ch, OBJ_DATA *obj )
{
	if ( IS_NPC( ch ))
		return false; 
	
	if ( !IS_TRAPPED( obj ))
		return false;
	
	if ( IS_SET( obj->trap_trig, TRAP_TRIG_OBJECT ) 
		&& obj->trap_charge > 0 )
	{
        trapdamage( ch, obj );
        return true;
	}
    return false;
}

/**************************************************************************/
bool trapcheck_open( char_data *ch, OBJ_DATA *obj )
{
	if ( IS_NPC( ch ))
		return false; 
	
	if ( !IS_TRAPPED( obj ))
		return false;
	
	if ( IS_SET( obj->trap_trig, TRAP_TRIG_OPEN ) 
		&& obj->trap_charge > 0 )
	{
        trapdamage( ch, obj );
        return true;
	}
    return false;
}

/**************************************************************************/
bool trapcheck_move( char_data *ch, int dir )
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	bool found = false;  
		
	if ( IS_NPC( ch ))
		return false;
	
    for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
    {
		obj_next = obj->next_content;
		
		if( IS_TRAPPED( obj )
			&& IS_SET(obj->trap_trig, TRAP_TRIG_MOVE)
			&& obj->trap_charge > 0) {
			found = true;
		} else {
			found = false;
		}
		
		if ( found )
		{
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_NORTH )
				&& dir == DIR_NORTH )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_EAST )
				&& dir == DIR_EAST )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_SOUTH )
				&& dir == DIR_SOUTH )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_WEST )
				&& dir == DIR_WEST )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_UP )
				&& dir == DIR_UP )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_DOWN )
				&& dir == DIR_DOWN )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_NORTHEAST )
				&& dir == DIR_NORTHEAST )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_NORTHWEST )
				&& dir == DIR_NORTHWEST )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_SOUTHEAST )
				&& dir == DIR_SOUTHEAST )
			{
				trapdamage( ch, obj );
				return true;
			}
			if ( IS_SET( obj->trap_trig, TRAP_TRIG_SOUTHWEST )
				&& dir == DIR_SOUTHWEST )
			{
				trapdamage( ch, obj );
				return true;
			}
		}
	}
	return false;
} 

/**************************************************************************/
void trapdamage(char_data *ch, OBJ_DATA *obj)
{
	char_data *wch = NULL;
	AFFECT_DATA af;
    int dam = 0;
	
    if (obj->trap_charge <= 0)
		return;
	
	act("You hear a strange noise......", ch, NULL, NULL, TO_ROOM);
	act("Now you've done it...", ch, NULL, NULL, TO_CHAR);
	obj->trap_charge -= 1;
	
	switch( obj->trap_dtype )
	{
		
		//  Damage to the whole room is always one less in severity (multiplier)
		
	case DAM_FIRE:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			act("A fireball shoots out of $p and hits $n!", ch, obj, NULL, TO_ROOM);
			act("A fireball shoots out of $p and hits you!", ch, obj, NULL, TO_CHAR);
			dam = number_range( obj->level * 2, obj->level * 4 ); 
		}
		else
		{
			act("A fireball shoots out of $p and hits everyone in the room!", ch, obj, NULL, TO_ROOM);
			act("A fireball shoots out of $p and hits everyone in the room!", ch, obj, NULL, TO_CHAR);
			dam = number_range( obj->level, obj->level * 3 );
		}
		break;
		
	case DAM_HOLY:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( IS_GOOD( ch ))
			{
				ch->println("You are washed in pure, divine light.");
				return;
			}
			act("A bright beam of holy light shoots out of $p and hits $n!", ch, obj, NULL, TO_ROOM);
			act("A bright beam of holy light shoots out of $p and hits you!", ch, obj, NULL, TO_CHAR);
			dam = obj->level * 3;
		}
		else
		{
			act("A bright beam of holy light shoots out of $p everyone in the room!", ch, obj, NULL, TO_ROOM);
			act("A bright beam of holy light shoots out of $p everyone in the room!", ch, obj, NULL, TO_CHAR);
			dam = obj->level * 3;
		}
		break;
		
		
	case DAM_COLD:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			act("A blast of frost from $p hits $n!", ch, obj, NULL, TO_ROOM);
			act("A blast of frost from $p hits you!", ch, obj, NULL, TO_CHAR);
			dam = number_range( obj->level * 3, obj->level * 5);
		}
		else
		{
			act("A blast of frost from $p fills the room freezing you!", ch, obj, NULL, TO_ROOM);
			act("A blast of frost from $p fills the room freezing you!", ch, obj, NULL, TO_CHAR);
			dam = number_range( obj->level * 2, obj->level * 4 );
		}
		break;
		
	case DAM_ACID:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			act("A blast of acid erupts from $p, burning your skin!", ch, obj, NULL, TO_CHAR);
			act("A blast of acid erupts from $p, burning $n's skin!", ch, obj, NULL, TO_ROOM);
			dam = number_range( obj->level * 2, obj->level * 6 );
		}
		else
		{
			act("A blast of acid erupts from $p, burning your skin!", ch, obj, NULL, TO_ROOM);
			act("A blast of acid erupts from $p, burning your skin!", ch, obj, NULL, TO_CHAR);
			dam = number_range( obj->level * 1, obj->level * 5 );
		}
		break;
		
	case DAM_HARM:
	case DAM_ENERGY:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM) )
		{
			act("A pulse of energy from $p zaps $n!", ch, obj, NULL, TO_ROOM);
			act("A pulse of energy from $p zaps you!", ch, obj, NULL, TO_CHAR);
			dam = number_range( obj->level, obj->level * 3 );
		}
		else
		{
			act("A pulse of energy from $p zaps you!", ch, obj, NULL, TO_ROOM);
			act("A pulse of energy from $p zaps you!", ch, obj, NULL, TO_CHAR);
			dam = number_range( obj->level, obj->level * 2 );
		}
		break;
		
	case DAM_LIGHTNING:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM) )
		{
			act("A bolt of lightning springs from $p and electrifies $n!", ch, obj, NULL, TO_ROOM);
			act("A bolt of lightning springs from $p and electrifies you!", ch, obj, NULL, TO_CHAR);
			dam = number_range( obj->level, obj->level * 3 );
		}
		else
		{
			act("A bolt of lightning springs from $p and electrifies you!", ch, obj, NULL, TO_ROOM );
			act("A bolt of lightning springs from $p and electrifies you!", ch, obj, NULL, TO_CHAR );
			dam = number_range( obj->level, obj->level * 2 );
		}
		break;
		
	case DAM_LIGHT:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( IS_AFFECTED( ch, AFF_BLIND ))
			{
				ch->println("A bright light washes over you but is suddenly gone.");
				return;
			}
			
			if ( saves_spell( obj->level, ch, DAM_LIGHT ))
			{
				ch->println("A bright light washes over you but is suddenly gone.");
				return;
			}
			else
			{	
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_blindness;
				af.level     = obj->level;
				af.location  = APPLY_HITROLL;
				af.modifier  = -4;
				af.duration  = obj->level;
				af.bitvector = AFF_BLIND;
				affect_to_char( ch, &af );
				ch->println("You are blinded!");
				act("$n appears to be blinded.",ch,NULL,NULL,TO_ROOM);
			}
		}
		else
		{
			act( "A bright light washes over the room.", ch, NULL, NULL, TO_CHAR );
			act( "A bright light washes over the room.", ch, NULL, NULL, TO_ROOM );
			
			for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
			{
				if ( IS_AFFECTED( wch, AFF_BLIND ))
				{
					wch->println("A bright light washes over you but is suddenly gone.");
					continue;
				}
				
				if ( saves_spell( obj->level, wch, DAM_LIGHT ))
				{
					wch->println("You manage to close your eyes before the light could blind you.");
					continue;
				}
				else
				{	
					af.where     = WHERE_AFFECTS;
					af.type      = gsn_blindness;
					af.level     = obj->level;
					af.location  = APPLY_HITROLL;
					af.modifier  = -4;
					af.duration  = obj->level;
					af.bitvector = AFF_BLIND;
					affect_to_char( wch, &af );
					wch->println("You are blinded!");
					act( "$n appears to be blinded.", wch, NULL, NULL, TO_ROOM);
				}
			}
		}
		return;
		
	case DAM_POISON:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( IS_AFFECTED( ch, AFF_POISON ))
			{
				ch->println("You feel momentarily ill, but it passes.");
				return;
			}
			
			ch->println("Curses!  You've pricked your finger on a needle.");
			act( "$n pricked $s finger on a little needle.", ch, NULL, NULL, TO_ROOM );
			
			if ( saves_spell( obj->level, ch, DAM_POISON ))
			{
				ch->println("You feel momentarily ill, but it passes.");
				return;
			}
			else
			{	
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_poison;
				af.level     = obj->level;
				af.duration  = (obj->level/2);
				af.location  = APPLY_ST;
				af.modifier  = -4;
				af.bitvector = AFF_POISON;
				affect_join( ch, &af );
				ch->println("You feel very sick.");
				act( "$n looks very ill.", ch, NULL, NULL, TO_ROOM );
				return;
			}
		}
		else
		{
			act( "A noxious cloud seeps out from $p.", ch, obj, NULL, TO_CHAR );
			act( "A noxious cloud seeps out from $p.", ch, obj, NULL, TO_ROOM );
			
			for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
			{
				if ( IS_AFFECTED( wch, AFF_POISON ))
				{
					wch->println("You feel momentarily ill, but it passes.");
					continue;
				}
				
				if ( saves_spell( obj->level, wch, DAM_POISON ))
				{
					wch->println("You feel momentarily ill, but it passes.");
					continue;
				}
				else
				{	
					af.where     = WHERE_AFFECTS;
					af.type      = gsn_poison;
					af.level     = obj->level;
					af.duration  = (obj->level/2);
					af.location  = APPLY_ST;
					af.modifier  = -4;
					af.bitvector = AFF_POISON;
					affect_join( wch, &af );
					wch->println("You feel very sick.");
					act( "$n looks very ill.", wch, NULL, NULL, TO_ROOM );
					continue;
				}
			}
		}
		return;
		
    case DAM_DISEASE:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( IS_AFFECTED( ch, AFF_PLAGUE ))
			{
				ch->println("You feel momentarily ill, but it passes.");
				return;
			}
			
			ch->println("Curses!  You've pricked your finger on a needle.");
			act( "$n pricked $s finger on a little needle.", ch, NULL, NULL, TO_ROOM );
			
			if ( saves_spell( obj->level, ch, DAM_DISEASE ))
			{
				ch->println("You feel momentarily ill, but it passes.");
				return;
			}
			else
			{	
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_plague;
				af.level     = obj->level;
				af.duration  = (obj->level/2);
				af.location  = APPLY_ST;
				af.modifier  = -10;
				af.bitvector = AFF_PLAGUE;
				affect_join( ch, &af );
				
				ch->println("You scream in agony as plague sores erupt from your skin.");
				act( "$n screams in agony as plague sores erupt from $s skin.", ch, NULL, NULL, TO_ROOM );
				return;
			}
		}
		else
		{
			act( "A noxious cloud seeps out from $p.", ch, obj, NULL, TO_CHAR );
			act( "A noxious cloud seeps out from $p.", ch, obj, NULL, TO_ROOM );
			
			for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
			{
				if ( IS_AFFECTED( wch, AFF_PLAGUE ))
				{
					wch->println("You feel momentarily ill, but it passes.");
					continue;
				}
				
				if ( saves_spell( obj->level, wch, DAM_DISEASE ))
				{
					wch->println("You feel momentarily ill, but it passes.");
					continue;
				}
				else
				{	
					af.where     = WHERE_AFFECTS;
					af.type      = gsn_plague;
					af.level     = obj->level;
					af.duration  = (obj->level/2);
					af.location  = APPLY_ST;
					af.modifier  = -10;
					af.bitvector = AFF_PLAGUE;
					affect_join( wch, &af );
					wch->println("You scream in agony as plague sores erupt from your skin.");
					act( "$n screams in agony as plague sores erupt from $s skin.", wch, NULL, NULL, TO_ROOM );
					continue;
				}
			}
		}
		return;
		
	case DAM_DROWNING:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( is_affected( ch, gsn_otterlungs ))
			{
				ch->println("Water fills your lungs, but you are still able to breathe normally.");
				return;
			}
			
			ch->println("You start to cough as water fills your lungs.");
			act( "$n coughs loudly.", ch, NULL, NULL, TO_ROOM );
			
			if ( saves_spell( obj->level, ch, DAM_DROWNING ))
			{
				ch->println("You manage to breathe the sweet, sweet air again.");
				return;
			}
			else
			{	
				ch->println("You gasp for breath, unable to draw in air.");
				act( "$n gasps for breath in a futile attempt to draw in air.\r\n", ch, NULL, NULL, TO_ROOM );
				dam = number_range( obj->level, obj->level * 2 );
			}
		}
		else
		{
			act( "Water fills the room.", ch, obj, NULL, TO_CHAR );
			act( "Water fills the room.", ch, obj, NULL, TO_ROOM );
			
			af.where		= WHERE_AFFECTS;
			af.type			= gsn_drown;
			af.level		= ch->level;
			af.duration		= obj->level/2;
			af.location		= APPLY_NONE;
			af.modifier		= 0;
			af.bitvector	= ROOMAFF_UNDERWATER;
			affect_to_room( ch->in_room, &af );
			return; // no damage per se
		}
		break;
		
	case DAM_CHARM:		// puts players to sleep :)
		
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( IS_AFFECTED( ch, AFF_SLEEP ))
				return;

			if ( saves_spell( obj->level, ch, DAM_CHARM ))
			{
				ch->println("You stifle a yawn.");
				return;
			}
	

			af.where     = WHERE_AFFECTS;
			af.type      = gsn_sleep;
			af.level     = number_fuzzy(obj->level);
			if(IS_NPC(ch))
				af.duration  = 4 + (obj->level/2);
			else
				af.duration  = 2 + (obj->level/9);  // duration reduced
			af.location  = APPLY_NONE;
			af.modifier  = 0;
			af.bitvector = AFF_SLEEP;
			affect_join( ch, &af );
			
			if ( IS_AWAKE( ch ))
			{
				ch->println("You feel very sleepy ..... zzzzzz.");
				act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
				ch->position = POS_SLEEPING;
			}
		}
		else
		{
			for (wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room)
			{			
				if ( IS_AFFECTED( wch, AFF_SLEEP ))
					continue;
				
				if ( saves_spell( obj->level, wch, DAM_CHARM ))
				{
					wch->println("You stifle a yawn.");
					continue;
				}
				
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_sleep;
				af.level     = number_fuzzy(obj->level);
				if(IS_NPC(wch))
					af.duration  = 4 + (obj->level/2);
				else
					af.duration  = 2 + (obj->level/9);  // duration reduced
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bitvector = AFF_SLEEP;
				affect_join( wch, &af );
				
				if ( IS_AWAKE( wch ))
				{ 
					ch->println("You feel very sleepy ..... zzzzzz.");
					act( "$n goes to sleep.", wch, NULL, NULL, TO_ROOM );
					wch->position = POS_SLEEPING;
				}
			}
		}
		return;
		
	case DAM_ILLUSION: // no damage per se
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( IS_AFFECTED( ch, AFF_FEAR ) || IS_AFFECTED2( ch, AFF2_FEAR_MAGIC) )
			{
				ch->println("A bright light flashes then disappears.");
				return;
			}
			
			if ( saves_spell( obj->level, ch, DAM_ILLUSION ))
			{
				ch->println("A bright light flashes then disappears.");
				return;
			}
			else
			{
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_cause_fear;
				af.level     = obj->level;
				af.duration  = 1;
				if ( obj->level > 25) af.duration +=1;
				if ( obj->level > 50) af.duration +=1;
				if ( obj->level > 75) af.duration +=1;
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bitvector = AFF_FEAR;
				affect_to_char( ch, &af );
				wch->println("You panic as you are gripped by an incredible fear.");
				act( "$n screams and runs away.", ch, NULL, NULL, TO_ROOM );
				do_fearful( wch ); /* in magic.c */
			}
		}
		else
		{				
			for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
			{
				if ( IS_AFFECTED( wch, AFF_FEAR ) || IS_AFFECTED2( wch, AFF2_FEAR_MAGIC) )
				{
					wch->println("A bright light flashes then disappears.");
					continue;
				}
				
				if ( saves_spell( obj->level, wch, DAM_ILLUSION ))
				{
					wch->println("A bright light flashes then disappears.");
					continue;
				}
				
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_cause_fear;
				af.level     = obj->level;
				af.duration  = 1;
				if ( obj->level > 25) af.duration +=1;
				if ( obj->level > 50) af.duration +=1;
				if ( obj->level > 75) af.duration +=1;
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bitvector = AFF_FEAR;
				affect_to_char( wch, &af );
				wch->println("You panic as you are gripped by an incredible fear.");
				act( "$n screams and runs away.", wch, NULL, NULL, TO_ROOM );
				do_fearful( wch ); /* in magic.c */
				
			}
		}
		return;
		
	case DAM_MENTAL:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( is_affected( ch, gsn_cause_headache ))
			{
				ch->println("Your head throbs.");
				return;
			}
			
			if ( saves_spell( obj->level, ch, DAM_MENTAL ))
			{
				ch->println("Your head throbs.");
				return;
			}
			else
			{	
				af.where		= WHERE_AFFECTS;
				af.type			= gsn_cause_headache;
				af.level		= obj->level;
				af.duration		= obj->level;
				af.location		= APPLY_SD;
				af.modifier		= - obj->level/5;
				af.bitvector	= 0;
				affect_to_char( ch, &af );
				ch->println("Your head seems to explode with a sudden wave of indescribable pain!");
				act( "$n grimaces in pain.", ch, NULL, NULL, TO_ROOM);
			}
		}
		else
		{
			for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
			{
				if ( is_affected( ch, gsn_cause_headache ))
				{
					wch->println("Your head throbs.");
					continue;
				}
				
				if ( saves_spell( obj->level, wch, DAM_MENTAL ))
				{
					wch->println("Your head throbs.");
					continue;
				}
				
				af.where		= WHERE_AFFECTS;
				af.type			= gsn_cause_headache;
				af.level		= obj->level;
				af.duration		= obj->level;
				af.location		= APPLY_SD;
				af.modifier		= - obj->level/5;
				af.bitvector	= 0;
				affect_to_char( wch, &af );
				wch->println("Your head seems to explode with a sudden wave of indescribable pain!");
				act( "$n grimaces in pain.", wch, NULL, NULL, TO_ROOM);
			}
		}
		return;
		
	case DAM_NEGATIVE:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( IS_EVIL( ch ))
			{
				ch->println("You revel in your wickedness.");
				return;
			}
			if ( is_affected( ch, gsn_wrath ))
			{
				ch->println("You feel the taint of evil on your soul.");
				return;
			}
			
			if ( saves_spell( obj->level, ch, DAM_NEGATIVE ))
			{
				ch->println("You feel as though you want to retch.");
				return;
			}
			else
			{
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_wrath;
				af.level     = obj->level;
				af.duration  = obj->level;
				af.location  = APPLY_HIT;
				af.modifier  = (-obj->level / 4) - 15;
				af.bitvector = 0;

				affect_join( ch, &af );

				ch->println("You feel a force of great ill invade your soul.");
				act( "$n drops to $s knees in horror.", ch, NULL, NULL, TO_ROOM);
			}
		}
		else
		{
			for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
			{

				if ( IS_EVIL( wch ))
				{
					wch->println("You revel in your wickedness.");
					continue;
				}
				
				if ( is_affected( wch, gsn_wrath ))
				{
					wch->println("You feel the taint of evil on your soul.");
					continue;
				}
				
				if ( saves_spell( obj->level, wch, DAM_MENTAL ))
				{
					wch->println("You feel as though you want to retch.");
					continue;
				}
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_wrath;
				af.level     = obj->level;
				af.duration  = obj->level;
				af.location  = APPLY_HIT;
				af.modifier  = (-obj->level / 4) - 15;
				af.bitvector = 0;
				affect_join( wch, &af );
				wch->println("You feel a force of great ill invade your soul.");
				act( "$n drops to $s knees in horror.", wch, NULL, NULL, TO_ROOM);
				
				if ( wch->hit < 0 )
				{
					raw_kill( wch, wch );
				}
				return;
			}
		}
		return;
		
	case DAM_SOUND:
		if ( !IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			if ( is_affected( ch, gsn_deafness ))
			{
				ch->println("Your ears pop.");
				return;
			}
			
			if ( saves_spell( obj->level, ch, DAM_SOUND ))
			{
				ch->println("Your ears pop.");
				return;
			}
			else
			{
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_deafness;
				af.level     = obj->level;
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.duration  = 1+obj->level;
				af.bitvector = 0;
				affect_to_char( wch, &af );
				ch->println("You have been deafened!");
				act( "$n's ears glow for a moment.", ch, NULL, NULL, TO_ROOM );
			}
		}
		else
		{
			for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
			{
				if ( is_affected( ch, gsn_deafness ))
				{
					wch->println("Your ears pop.");
					continue;
				}
				
				if ( saves_spell( obj->level, wch, DAM_SOUND ))
				{
					wch->println("Your ears pop.");
					continue;
				}
				
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_deafness;
				af.level     = obj->level;
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.duration  = 1+obj->level;
				af.bitvector = 0;
				affect_to_char( wch, &af );
				wch->println("You have been deafened!");
				act( "$n's ears glow for a moment.", wch, NULL, NULL, TO_ROOM );
			}
		}
		return;
		
	case DAM_BASH:
		if (!IS_SET( obj->trap_trig, TRAP_TRIG_ROOM ))
		{
			act("$n sets off a trap on $p and is hit by a blunt object!", ch, obj, NULL, TO_ROOM);
			act("You are hit by a blunt object from $p!", ch, obj, NULL, TO_CHAR);
			dam = 3 * obj->level + GET_AC( ch, AC_BASH );
		}
		
		else
		{
			act("$n sets off a trap on $p and you are hit by a flying object!", ch, obj, NULL, TO_ROOM);
			act("You are hit by a blunt object from $p!", ch, obj, NULL, TO_CHAR);
		}
		break;
	case DAM_PIERCE:
		if (!IS_SET( obj->trap_trig, TRAP_TRIG_ROOM) )
		{
			act("$n sets of a trap on $p and is pierced in the chest!", ch, obj, NULL, TO_ROOM);
			act("You set off a trap on $p and are pierced through the chest!", ch, obj, NULL, TO_CHAR);
			dam = 3 * obj->level + GET_AC( ch, AC_PIERCE );
		}
		else
		{
			act("$n sets off a trap on $p and you are hit by a piercing object!", ch, obj, NULL, TO_ROOM); 
			act("You set off a trap on $p and are pierced through the chest!", ch, obj, NULL, TO_CHAR);
		}
		break;
	case DAM_SLASH:
		if (!IS_SET(obj->trap_trig, TRAP_TRIG_ROOM) )
		{
			act("$n just got slashed by a trap on $p.", ch, obj, NULL, TO_ROOM);
			act("You just got slashed by a trap on $p!", ch, obj, NULL, TO_CHAR);
			dam = 3 * obj->level + GET_AC( ch, AC_PIERCE );
		}
		else
		{
			act("$n set off a trap releasing a blade that slashes you!", ch, obj, NULL, TO_ROOM);
			act("You set off a trap releasing blades around the room..", ch, obj, NULL, TO_CHAR);
			act("One of the blades slashes you in the chest!", ch, obj, NULL, TO_CHAR);
		}
		break;
		
	case DAM_NONE:
	case DAM_OTHER:
	default:
		act( "A clicking sound comes from $p but nothing seems to have happened.", ch, obj, NULL, TO_CHAR );
		act( "A clicking sound comes from $p but nothing seems to have happened.", ch, obj, NULL, TO_ROOM );
		return;
		break;
	}
	
	// Moment of truth, dish out the damage
	
	if (!IS_SET(obj->trap_trig, TRAP_TRIG_ROOM))
	{            
		if ( ch->position == POS_DEAD )
			return;
		
		// stop crazy builders
		if ( dam > 1000 )
		{
			bugf( "Damage: %d: more than 1000 points in trap!", dam );
			dam = 1000;
		}
		
		if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
		{
			affect_parentspellfunc_strip( ch, gsn_invisibility );
			affect_parentspellfunc_strip( ch, gsn_mass_invis );
			REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
			act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
		}
		
		// divide 
		
		// Mods and such
		
		if ( IS_AFFECTED( ch, AFF_SANCTUARY ))
			dam /= 2;
		
		if ( saves_spell( obj->level, ch, obj->trap_dtype ))
			dam /= 2;
		
		switch( check_immune( ch, obj->trap_dtype ))
		{
		case(IS_IMMUNE):
			dam = 0;
			break;
		case(IS_RESISTANT):
			dam -= dam/2;
			break;
		case(IS_VULNERABLE):
			dam += dam;
			break;
		}
		
		if ( dam < 0 )
			dam = 0;
		
		ch->hit -= dam;
		
		if (( IS_NPC(ch)
			|| IS_IMMORTAL( ch ))
			&&   ch->hit < 1 )
			ch->hit = 1;
		
		update_pos( ch );
		
		switch( ch->position )
		{
		case POS_MORTAL:
			act( "$n is mortally wounded, and will die soon, if not aided.", ch, NULL, NULL, TO_ROOM );
			ch->println("You are mortally wounded, and will die soon, if not aided.");
			break;
			
		case POS_INCAP:
			act( "$n is incapacitated and will slowly die, if not aided.", ch, NULL, NULL, TO_ROOM );
			ch->println("You are incapacitated and will slowly die, if not aided.");
			break;
			
		case POS_STUNNED:
			act( "$n is stunned, but will probably recover.",ch, NULL, NULL, TO_ROOM);
			ch->println("You are stunned, but will probably recover.");
			break;
			
		case POS_DEAD: 
			act( "$n is DEAD!!", ch, NULL, NULL, TO_ROOM );
			ch->println("You have been `rKILLED!!`x\r\n");
			break;
			
		default:
			if ( dam > ch->max_hit / 4 )
			{
				ch->println("`YThat really did `rHURT!`x");
			}
			if ( ch->hit < ch->max_hit / 4 )
			{
				ch->println("`YYou sure are `rBLEEDING!`x");
			}
			break;
		}
		
		if ( IS_NPC(ch))
		{
			raw_kill(ch,ch);
			return;
		}
		
		if (ch->position == POS_DEAD)
		{
			sprintf( log_buf, "%s [lvl %d] got toasted by a trap at %s [lvl %d] [room %d]",
				ch->name,
				ch->level,
				ch->in_room->name,
				obj->level,
				ch->in_room->vnum );
			
			log_string( log_buf );
			wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);	
			
			if(ch->level<21)
			{
				if ( ch->exp > exp_per_level(ch,ch->pcdata->points)	* ch->level )
				{
					gain_exp( ch, (2 * (exp_per_level(ch,ch->pcdata->points)
						* ch->level - ch->exp)/3) + 50 );
				}
			}
			else
			{
				int xp_amount;
				if(ch->level>50)
				{
					xp_amount=500+(get_sublevels_for_level( ch->level) *150);
				}
				else
				{
					xp_amount=500;
				}
				ch->printlnf("You have lost %d xp.", xp_amount);
				gain_exp( ch, -xp_amount);
				
				if ( IS_HERO( ch ))
					do_heroxp( ch, -xp_amount);
				
				if(ch->exp<exp_per_level(ch,ch->pcdata->points)* ch->level )
				{
					drop_level(ch);
					//	Too mean???			check_perm_damage(ch);
				}
				else
				{
					if(number_range(1,get_sublevels_for_level( ch->level+5))<2)
					{
						//  Too Mean???				check_perm_damage(ch);
					}
					else
					{
						ch->println("Luckily you have not suffered serious injury.");
					}
				}
			}
			raw_kill( ch, ch );
			char_data *trapname = create_mobile( limbo_mob_index_data, 0 );
			replace_string(trapname->short_descr, "a trap");
			make_corpse( ch, trapname );
			extract_char( trapname, true );
		}
	}	
	else
	{
		for (wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room)
		{
			if (wch == NULL)
				break;
			
			if ( obj->trap_dtype   == DAM_BASH
				|| obj->trap_dtype == DAM_PIERCE
				|| obj->trap_dtype == DAM_SLASH)
				dam = (3 * obj->level) + GET_AC(wch,obj->trap_dtype);
			
			if ( wch->position == POS_DEAD )
				continue;
			
			if ( dam > 1000 )
			{
				bugf( "Damage: %d: more than 1000 points!", dam );
				dam = 1000;
			}
			
			if ( IS_AFFECTED(wch, AFF_INVISIBLE) )
			{
				affect_parentspellfunc_strip( wch, gsn_invisibility );
				affect_parentspellfunc_strip( wch, gsn_mass_invis );
				REMOVE_BIT( wch->affected_by, AFF_INVISIBLE );
				act( "$n fades into existence.", wch, NULL, NULL, TO_ROOM );
			}
			
			/*
			* Damage modifiers.
			*/
			if ( IS_AFFECTED( wch, AFF_SANCTUARY ))
				dam /= 2;
			
			if ( saves_spell( obj->level, wch, obj->trap_dtype ))
				dam /= 2;
			
			if ( IS_GOOD( wch ) && obj->trap_dtype == DAM_HOLY )
			{
				wch->println("You are washed in pure, divine light.");
				continue;
			}
			
			switch( check_immune( wch, obj->trap_dtype ))
			{
			case(IS_IMMUNE):
				dam = 0;
				break;
			case(IS_RESISTANT):
				dam -= dam/2;
				break;
			case(IS_VULNERABLE):
				dam += dam;
				break;
			}
			
			if ( dam < 0 )
				dam = 0;
			
			wch->hit -= dam;
			if (( IS_NPC(wch)
				|| IS_IMMORTAL( wch ))
				&& wch->hit < 1 )
				wch->hit = 1;
			
			update_pos( wch );
			
			switch( wch->position )
			{
			case POS_MORTAL:
				act( "$n is mortally wounded, and will die soon, if not aided.", wch, NULL, NULL, TO_ROOM );
				wch->println("You are mortally wounded, and will die soon, if not aided.");
				break;
				
			case POS_INCAP:
				act( "$n is incapacitated and will slowly die, if not aided.", wch, NULL, NULL, TO_ROOM );
				wch->println("You are incapacitated and will slowly die, if not aided.");
				break;
				
			case POS_STUNNED:
				act( "$n is stunned, but will probably recover.",wch, NULL, NULL, TO_ROOM);
				wch->println("You are stunned, but will probably recover.");
				break;
				
			case POS_DEAD:
				act( "$n is DEAD!!", wch, 0, 0, TO_ROOM );
				wch->println("You have been `rKILLED!!`x\r\n");
				break;
				
			default:
				if ( dam > wch->max_hit / 4 )
				{
					wch->println("`YThat really did `rHURT!`x");
				}
				if ( wch->hit < wch->max_hit / 4 )
				{
					wch->println("`YYou sure are `rBLEEDING!`x");
				}
				break;
			}
			
			if ( IS_NPC(wch))
			{
				raw_kill(wch,wch);
				continue;
			}
			
			if (wch->position == POS_DEAD)
			{
				sprintf( log_buf, "%s [lvl %d] got toasted by a trap at %s [lvl %d] [room %d]",
					wch->name,
					wch->level,
					wch->in_room->name,
					obj->level,
					wch->in_room->vnum );
				
				log_string( log_buf );
				wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);	
				
				if(wch->level<21)
				{
					if ( wch->exp > exp_per_level(wch,wch->pcdata->points)	* wch->level )
					{
						gain_exp( wch, (2 * (exp_per_level(wch,wch->pcdata->points)
							* wch->level - wch->exp)/3) + 50 );
					}
				}
				else
				{
					int xp_amount;
					if(wch->level>50)
					{
						xp_amount=500+(get_sublevels_for_level( wch->level) *150);
					}
					else
					{
						xp_amount=500;
					}
					wch->printlnf("You have lost %d xp.", xp_amount);
					gain_exp( wch, -xp_amount);
					
					if ( IS_HERO( wch ))
						do_heroxp( wch, -xp_amount);
					
					if(wch->exp<exp_per_level(wch,wch->pcdata->points)* wch->level )
					{
						drop_level(wch);
						//	Too mean???			check_perm_damage(wch);
					}
					else
					{
						if(number_range(1,get_sublevels_for_level( wch->level+5))<2)
						{
							//  Too Mean???				check_perm_damage(victim);
						}
						else
						{
							wch->println("Luckily you have not suffered serious injury.");
						}
					}
				}
				raw_kill( wch, wch );
				char_data *trapname = create_mobile( limbo_mob_index_data, 0 );
				replace_string(trapname->short_descr, "a trap");
				make_corpse( wch, trapname );
				extract_char( trapname, true );
			}
		}
	}
	return;
}

/**************************************************************************/
void do_search( char_data *ch, char *argument )
{
	OBJ_DATA *obj;
	char arg[MIL];
	int chance;

	one_argument(argument, arg);

	if ( IS_NPC(ch))
		return;

	if ( IS_NULLSTR( arg ))
	{
		ch->println("What do you want to search?");
		return;
	}

	if (( obj = get_obj_here( ch, arg )) == NULL)
	{
		ch->println("What do you wish to search?");
		return;
	}

	if ( !IS_TRAPPED( obj ))
	{
		act( "$p doesn't appear to be trapped.", ch, obj, NULL, TO_CHAR );
		return;
	}

	// if players wanna spam this command, they'll get punished for it
	if ( ch->desc && ch->desc->repeat>5 )
	{
		act( "Upon inspecting $p closer, you've managed to trigger a well-hidden trap.", ch, obj, NULL, TO_CHAR );
		trapdamage( ch, obj );
		return;
	}

	chance = ( get_skill( ch,gsn_search ) + obj->trap_modifier ) - number_percent();

	if ( chance > 25 ) 	{
		act( "$p is rigged with a trap.", ch, obj, NULL, TO_CHAR );
		if ( obj->trap_modifier <= 65 )
		{
			obj->trap_modifier += 10;
		}
	} else if ( chance > -20 ) {
		act( "You're not entirely certain, but $p could be trapped.", ch, obj, NULL, TO_CHAR );
	} else  {
		act( "$p doesn't appear to be trapped.", ch, obj, NULL, TO_CHAR );
		if ( obj->trap_modifier >= -65 )
		{
			obj->trap_modifier -= 10;
		}
	}

	if ( chance < -50 )
	{
		act( "\r\nUpon inspecting $p closer, you've managed to trigger a well-hidden trap.", ch, obj, NULL, TO_CHAR );
		act( "$n inadvertently triggered a hidden trap on $p.", ch, obj, NULL, TO_ROOM );
		check_improve( ch, gsn_search, false, 10 );
		trapdamage( ch, obj );
	}
	else
	{
		check_improve( ch, gsn_search, true, 10 );
	}
	return;
}

/**************************************************************************/
void do_disarm_trap( char_data *ch, char *argument )
{
	OBJ_DATA *obj;
	char arg[MIL];
	int chance;

	one_argument(argument, arg);

	if ( IS_NPC(ch))
		return;

	if (( obj = get_obj_here( ch, arg )) == NULL)
	{
		ch->println("What do you wish to disarm?");
		return;
	}

	if ( !IS_TRAPPED( obj ))
	{
		act( "Ok.", ch, NULL, NULL, TO_CHAR );
		return;
	}

	chance = (( get_skill( ch,gsn_search ) + obj->trap_modifier ) - number_percent()) - 25;

	if ( chance > 25 ) 	{		// good success, trap is gone
		act( "Ok.", ch, NULL, NULL, TO_CHAR );
		check_improve( ch, gsn_search, true, 10 );
		REMOVE_BIT( obj->extra2_flags, OBJEXTRA2_TRAP );
	} else if ( chance > 0 ) {	// medium success, charges lowered, could still be trapped though
		act( "Ok.", ch, NULL, NULL, TO_CHAR );
		obj->trap_charge -= number_range( 1, 3 );
		check_improve( ch, gsn_search, true, 5 );
	} else  {					// failure,  BOOOOM!!!
		act( "Curses!  You've managed to trigger $p.", ch, obj, NULL, TO_CHAR );
		check_improve( ch, gsn_search, false, 10 );
		trapdamage( ch, obj );
	}
	return;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

