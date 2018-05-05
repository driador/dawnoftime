/**************************************************************************/
// skill_tr.cpp - skills & spells written by Tristan
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"

/**************************************************************************/

SPRESULT spell_mana_regeneration( int sn, int level, char_data *ch, void *vo, int )
{
	char_data		*vch = ( char_data * ) vo;
	affect_data		af;

	if( is_affected(vch, sn) )
	{
		ch->println( "Your spell fizzles and dies." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS2;
	af.type			= sn;
	af.level		= level;
	af.duration		= level / 12;
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= AFF2_MANA_REGEN;

	affect_to_char( ch, &af );

	ch->println( "Warmth spreads throughout your body." );

	return FULL_MANA;
}

/**************************************************************************/
// For Blessed Grounds and Cursed Grounds, Tristan
SPRESULT spell_blessed_grounds( int sn, int level, char_data *ch, void *, int )
{
	affect_data		af;

	switch( ch->in_room->sector_type )
	{
	case SECT_WATER_NOSWIM:
	case SECT_WATER_SWIM:
	case SECT_SWAMP:
	case SECT_AIR:
	case SECT_UNDERWATER:
	case SECT_LAVA:
		{
			ch->println( "You failed." );
			return NO_MANA;
		}
	};

	if( IS_SET( ch->in_room->affected_by, ROOMAFF_BLESSED_GROUNDS ) )
	{
		if( ch->alliance >= 0 )
		{
			ch->println( "These grounds have already been blessed." );
			return HALF_MANA;
		}
		else
		{
			REMOVE_BIT( ch->in_room->affected_by, ROOMAFF_BLESSED_GROUNDS );
			ch->println( "The warmth in the ground dissipates." );
			return FULL_MANA;
		}
	}

	if( IS_SET( ch->in_room->affected_by, ROOMAFF_CURSED_GROUNDS ) )
	{
		if( ch->alliance < 0 )
		{
			ch->println( "These grounds have already been cursed." );
			return HALF_MANA;
		}
		else
		{
			REMOVE_BIT( ch->in_room->affected_by, ROOMAFF_CURSED_GROUNDS );
			ch->println( "The cold in the ground dissipates." );
			return FULL_MANA;
		}
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level / 20;
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	
	if( ch->alliance < 0 )
	{
		af.bitvector = ROOMAFF_CURSED_GROUNDS;
		ch->println( "An icy chill spreads rapidly through the ground beneath your feet." );
		act( "$n touches the ground and it quickly becomes cold.", ch, NULL, NULL, TO_ROOM );
	}
	else
	{
		af.bitvector = ROOMAFF_BLESSED_GROUNDS;
		ch->println( "Warmth spreads rapidly through the ground beneath your feet." );
		act( "$n touches the ground and it becomes rather warm.", ch, NULL, NULL, TO_ROOM );
	}

	affect_to_room( ch->in_room, &af );

	return FULL_MANA;
}

/**************************************************************************/

