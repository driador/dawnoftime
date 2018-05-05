/**************************************************************************/
// magic_ja.cpp - spells/skills written by Jarren
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "magic.h"

// Local functions
DECLARE_DO_FUN( do_look		);


/*helper prototypes*/
void    char_from_room  ( char_data *ch );
void    char_to_room    ( char_data *ch, ROOM_INDEX_DATA *pRoomIndex );
bool	check_cold_shield(int level, char_data *ch, char_data *victim);
bool	check_fire_shield(int level, char_data *ch, char_data *victim);
bool	check_skin( char_data *ch, char_data *victim);
bool	check_strength( int level, char_data *ch, char_data *victim, bool& half );
bool	is_aff_fire_shield( char_data *ch, char_data *victim );
bool	is_aff_cold_shield( char_data *ch, char_data *victim );
void	landchar(char_data *ch);

/**************************************************************************/
SPRESULT spell_blink( int, int level, char_data *ch, void *vo, int )
{
	int door = *(int *)vo; // the direction number (preparsed by cast)
	EXIT_DATA *pexit;
	bool nopass = false;

	// can't blink out of combat
	if ( ch->fighting )
	{
		ch->println( "You can't focus your mind enough to cast the spell." );
		return NO_MANA;
	}

	// figure out the chance of blink going psyco
	int number = 0, steps = 0, spaz = 0, spaz_chance;
	int spaz_mod = ( level/20) + (get_skill(ch, gsn_blink)/20);
	spaz_chance = spaz_mod*100;
	if ( !IS_NPC( ch )) {
		if(ch->pcdata->tired >= 20)
			spaz_chance = spaz_chance/(2*spaz_mod);
		if(ch->pcdata->tired < 20 && ch->pcdata->tired > 16)
			spaz_chance = spaz_chance/(spaz_mod+2);
	}
	spaz = number_range(1, spaz_chance);

	// number of rooms they are moving - how far they are going
	number = number_range(1, level/8);

	// don't let them blink thru nopass doors
	if ( ch->in_room->exit[door] && IS_SET(ch->in_room->exit[door]->exit_info,EX_NOPASS)){ 
		ch->println( "You attempt to blink in that direction but fail." );
		return NO_MANA;
	};

	// start the blink
	ch->println( "You feel yourself fold into another dimension and whisk away." );
	act("$n blinks out of existance.",ch,NULL,NULL,TO_ROOM);
	
	// while they can move in the direction let them continue
	ROOM_INDEX_DATA *was_in_room=NULL;
	ROOM_INDEX_DATA *to_room = ch->in_room;
	for(pexit=ch->in_room->exit[door]; pexit; pexit = to_room->exit[door])
	{
		was_in_room = to_room;
		to_room = pexit->u1.to_room;

		ch->mana -= 15;
		steps++;
		if ( IS_SET(pexit->exit_info,EX_NOPASS))
		{
			ch->println( "You suddenly pop back into your realm and are sent sprawling!" );
			act("$n suddenly pops into existance and clumsily crashes down.",ch,NULL,NULL,TO_ROOM);
			ch->hit -= 5*steps;
			char_from_room(ch);
			char_to_room(ch, was_in_room);
			ch->position = POS_RESTING;
			landchar( ch );
			do_look(ch, "auto");
			return NO_MANA;
		}

		if(!can_see_room(ch,to_room)		
			 ||   IS_SET(to_room->room_flags, ROOM_SAFE)
			 ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
			 ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
			 ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
			 ||   IS_SET(to_room->room_flags, ROOM_NOSCRY)
			 ||	  IS_SET(to_room->room_flags, ROOM_ANTIMAGIC)
			 ||   IS_SET(to_room->area->area_flags, AREA_NOSCRY)
			 ||   IS_SET(to_room->area->area_flags, AREA_NOGATEINTO)	 
			 ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
			 )
		{
			ch->println( "You suddenly pop back into your realm and are sent sprawling!" );
			act("$n suddenly pops into existance and clumsily crashes down.",ch,NULL,NULL,TO_ROOM);
			ch->hit -= 5*steps;
			char_from_room(ch);
			char_to_room(ch, was_in_room);
			ch->position = POS_RESTING;
			landchar( ch );
			do_look(ch, "auto");
			return NO_MANA;
		}

 		if(to_room->clan && ch->clan != to_room->clan)
 		{
 			ch->println( "You suddenly pop back into your realm and are sent sprawling!" );
 			act("$n suddenly pops into existance and clumsily crashes down.",ch,NULL,NULL,TO_ROOM);
 			ch->hit -= 5*steps;
 			char_from_room(ch);
 			char_to_room(ch, was_in_room);
 			ch->position = POS_RESTING;
 			landchar( ch );
 			do_look(ch, "auto");
 			return NO_MANA;
 		}

		if (--number<1) break;

		if ( ch->mana < 15)
		{
			ch->println( "You suddenly pop back into your realm and are sent sprawling!" );
			act("$n suddenly pops into existance and clumsily crashes down.",ch,NULL,NULL,TO_ROOM);
			ch->hit -= 2*steps;
			char_from_room(ch);
			char_to_room(ch, to_room);
			ch->position = POS_RESTING;
			landchar( ch );
			do_look(ch, "auto");
			return NO_MANA;
		}
	}

	if(spaz <= 3)
	{
		ch->wrapln( "You feel like you're spinning out of control. Pain starts to wash over your body as you feel like you are going to ripped apart." );

		WAIT_STATE(ch, 8);
		number = 40 - steps;
		
		for(; pexit; pexit = to_room->exit[door])
		{
			was_in_room = to_room;
			to_room = pexit->u1.to_room;
			steps++;
			number--;
			if (number == 0) break;

			if ( IS_SET(pexit->exit_info,EX_NOPASS))
			{
				nopass = true;
				break;
			}

			if(ch->mana < 15)
				ch->mana = 0;
			else
				ch->mana -= 15;
		}
		
		ch->hit = 3*ch->hit/4;

	}

	char_from_room(ch);
	if ( nopass ){
		char_to_room( ch, was_in_room );
	}else{
		char_to_room(ch, to_room);
	}

	if(number > 0){
		ch->println( "You slam into something blunt and fall onto the floor!" );
		act("$n suddenly pops into existance and clumsily crashes down.",ch,NULL,NULL,TO_ROOM);
		ch->hit -= 2*steps;
		landchar(ch);
		ch->position = POS_RESTING;
	}else{
		ch->println( "You suddenly find yourself in a different place." );
	    act("$n suddenly pops into existance.",ch,NULL,NULL,TO_ROOM);
	}
	do_look(ch, "auto");
	return NO_MANA;
}

/**************************************************************************/
SPRESULT spell_low_area_attack( int sn, int level, char_data *ch, void *,int  )
{
	char_data *vch;
	char_data *vch_next;

	int dam = level;

    for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
        vch_next=vch->next_in_room;
		if ( vch->in_room == NULL )
			continue;
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && !is_safe_spell(ch,vch,true))
			damage_spell( ch, vch, saves_spell( level, vch, DAMTYPE( sn )) 
					? dam / 2 : dam, sn, DAMTYPE(sn),true);
			continue;
		}
	}
    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_mid_area_attack( int sn, int level, char_data *ch, void *,int  )
{
	char_data *vch;
	char_data *vch_next;

	int dam = dice( 3, 5);

    for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
        vch_next=vch->next_in_room;
		if ( vch->in_room == NULL )
			continue;
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && !is_safe_spell(ch,vch,true))
			damage_spell( ch, vch, saves_spell( level, vch, DAMTYPE( sn )) 
					? dam / 2 : dam, sn, DAMTYPE(sn),true);
			continue;
		}
	}
    return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_high_area_attack( int sn, int level, char_data *ch, void *,int  )
{
	char_data *vch;
	char_data *vch_next;
    
	int dam = dice( 5, 8);

    for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
        vch_next=vch->next_in_room;
		if ( vch->in_room == NULL )
			continue;
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && !is_safe_spell(ch,vch,true))
			damage_spell( ch, vch, saves_spell( level, vch, DAMTYPE( sn )) 
					? dam / 2 : dam, sn, DAMTYPE(sn),true);
			continue;
		}
	}
    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_rocky_skin( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if(check_skin( ch, victim ))
		return HALF_MANA;
	
	int aff_mod = number_range(30, 40);
	aff_mod += level/10;

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -aff_mod;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n's skin becomes rock hard!", victim, NULL, NULL, TO_ROOM );
	victim->println( "Your skin becomes rock hard!" );
	af.where	 = WHERE_VULN;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = VULN_ACID;
	affect_to_char( victim, &af );
	
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_tough_skin( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if(check_skin( ch, victim ))
		return HALF_MANA;

	int aff_mod = number_range(20, 40);

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -aff_mod;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n's skin looks much tougher now.", victim, NULL, NULL, TO_ROOM );
	victim->println( "You feel you skin become leathery and tough." );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_iron_skin( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if(check_skin( ch, victim ))
		return HALF_MANA;

	int aff_mod = number_range(30, 40);
	aff_mod += level/6;
	
	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier	 = -aff_mod;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n's skin becomes hard as iron!", victim, NULL, NULL, TO_ROOM );
	victim->println( "Your skin becomes hard as iron!" );
	af.where	 = WHERE_VULN;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = VULN_LIGHTNING;
	affect_to_char( victim, &af );
	
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_magical_vestment( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	int aff_mod = number_range(25, 40);
	aff_mod += level/8;

	if ( is_affected(victim, sn ) )
	{
		if (victim == ch){
			ch->println( "You are already protected by a cloak of magic." );
		}else{
			ch->println( "They are already protected by a cloak of magic." );
		}
		return HALF_MANA;
	}

	if ( is_affected( ch, gsn_blessed_garments ) )
	{
		if (victim == ch){
			ch->println( "You are already protected by blessed clothing." );
		}else{
			ch->println( "They are already protected by a cloak of magic." );
		}
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -aff_mod;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n is surrounded by a cloak of magic.", victim, NULL, NULL, TO_ROOM );
	victim->println( "You are surrounded by a cloak of magic." );
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_blessed_garments( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	int aff_mod = number_range(20, 40);

	if ( is_affected( victim, sn ))
	{
		if (victim == ch){
			ch->println( "You are already clothed with righteousness." );
		}else{
			ch->println( "They are already protected by a cloak of magic." );
		}
		return HALF_MANA;
	}

	if ( is_affected( ch, gsn_magical_vestment ) )
	{
		if (victim == ch){
			ch->println( "You are already protected by magical clothing." );
		}else{
			ch->println( "They are already protected by a cloak of magic." );
		}
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -aff_mod;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	victim->println( "You feel as though you are clothed with righteousness." );
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_cyclone( int sn, int level, char_data *ch, void *vo, int )
{
    char_data *victim = (char_data *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
		if (victim == ch)
			ch->println( "You are already protected by the power of a cyclone." );
		else
			act("$N is already protected by the power of the cyclone.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
    }

    if ( is_affected( victim, gsn_wind_shield ) )
    {
		if (victim == ch)
			ch->println( "The winds are protecting you as much as they can." );
		else
			act("$N is already protected by the winds.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
    }

    af.where     = WHERE_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->level/10;
    af.modifier  = (level+6)/-2;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->println( "You are enveloped within the eye of a cyclone." );
    act("A roaring twister envelops $n.",victim,NULL,NULL,TO_ROOM);
    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_shelter( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( is_affected( victim, sn ) )
	{
		if (victim == ch)
			ch->println( "You are already sheltered from harm." );
		else
			act("$N is already sheltered.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 8 + level;
	af.location  = APPLY_AC;
	af.modifier  = -20;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n is sheltered from harm.", victim, NULL, NULL, TO_ROOM );
	victim->println( "You are sheltered from harm." );
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_oak_shield( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( is_affected( victim, sn ) )
	{
		if (victim == ch)
			ch->println( "You already have a shield of oak." );
		else
			act("$N already has a shield of oak.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	if ( is_affected( victim, gsn_shield ))
	{
		if (victim == ch)
			ch->println( "You already have a shield spell on you." );
		else
			act("$N is already protected by a shield spell.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}


	af.where	 = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 24;
	af.modifier  = -25;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	victim->println( "A shield of oak forms around you." );

	if ( ch != victim )
		act("$N is protected by your oak shield.",ch,NULL,victim,TO_CHAR);
    return HALF_MANA;
}
/**************************************************************************/
SPRESULT spell_ogre_strength(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	bool half = false;

	if(check_strength(level, ch, victim, half))
		return FULL_MANA;
	if(half)
		return HALF_MANA;

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_ST;
	af.modifier		= 1 + level/6;
	af.bitvector	= 0;
	affect_to_char( victim, &af );
	victim->println( "Your muscles surge with heightened power!" );
	act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_dragon_strength(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	bool half = false;

	if(check_strength(level, ch, victim, half))
		return FULL_MANA;
	if(half)
		return HALF_MANA;

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_ST;
	af.modifier		= 1 + level/4;
	af.bitvector	= 0;
	affect_to_char( victim, &af );
	victim->println( "Your muscles surge with heightened power!" );
	act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_fiery_armour( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( is_aff_fire_shield( ch, victim ))
		return HALF_MANA;

	else if ( check_cold_shield(level, ch, victim))
		return FULL_MANA;
		
	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	if (victim == ch)
		af.duration  = level/5;
	else
		af.duration  = level/10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	af.where	  = WHERE_RESIST;
	af.bitvector = RES_COLD;
	affect_to_char( victim, &af );
	af.where	  = WHERE_VULN;
	af.bitvector = VULN_FIRE;
	affect_to_char( victim, &af );
	victim->println( "You become armoured with fire." );
	act("$n becomes armoured with fire.",victim,NULL,NULL,TO_ROOM);
	if ( ch != victim )
		ch->println( "Ok." );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_wall_of_fire( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if ( is_aff_fire_shield( ch, victim ))
		return HALF_MANA;
	
	else if ( check_cold_shield(level, ch, victim))
		return FULL_MANA;
	
	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	if (victim == ch)
		af.duration  = level/5;
	else
		af.duration  = level/10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	af.where	  = WHERE_RESIST;
	af.bitvector = RES_COLD;
	affect_to_char( victim, &af );
	af.where	  = WHERE_VULN;
	af.bitvector = VULN_FIRE;
	victim->println( "You are surrounded by a wall of fire." );
	act("$n is surrounded by a wall of fire.",victim,NULL,NULL,TO_ROOM);
	if ( ch != victim )
		ch->println( "Ok." );
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_icy_armour( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if ( is_aff_cold_shield( ch, victim ))
		return HALF_MANA;
	
	else if ( check_fire_shield(level, ch, victim))
		return FULL_MANA;
	
	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	if (victim == ch)
		af.duration  = level/5;
	else
		af.duration  = level/10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	af.where	  = WHERE_RESIST;
	af.bitvector = RES_FIRE;
	affect_to_char( victim, &af );
	af.where	  = WHERE_VULN;
	af.bitvector = VULN_COLD;
	affect_to_char( victim, &af );
	victim->println( "You become armoured with ice." );
	act("$n becomes armoured with ice.",victim,NULL,NULL,TO_ROOM);
	if ( ch != victim )
		ch->println( "Ok." );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_wall_of_ice( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if ( is_aff_cold_shield( ch, victim ))
		return HALF_MANA;
	
	else if ( check_fire_shield(level, ch, victim))
		return FULL_MANA;
	
	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	if (victim == ch)
		af.duration  = level/5;
	else
		af.duration  = level/10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	af.where	  = WHERE_RESIST;
	af.bitvector = RES_FIRE;
	affect_to_char( victim, &af );
	af.where	  = WHERE_VULN;
	af.bitvector = VULN_COLD;
	victim->println( "You are surrounded by a wall of ice." );
	act("$n is surrounded by a wall of ice.",victim,NULL,NULL,TO_ROOM);
	if ( ch != victim )
		ch->println( "Ok." );
	return FULL_MANA;
}
/**************************************************************************/
// helper function for fire/cold shield type spells
bool is_aff_fire_shield( char_data *ch, char_data *victim )
{

	if ( is_affected( victim, gsn_fire_shield ))
	{
		if (victim == ch)
			ch->println( "You already have a fire shield." );
		else
			act("$N already has a shield of fire.",ch,NULL,victim,TO_CHAR);
		return true;
	}

	if ( is_affected( victim, gsn_wall_of_fire ))
	{
		if (victim == ch)
			ch->println( "You are already surrounded by fire." );
		else
			act("$N is already surrounded by fire.",ch,NULL,victim,TO_CHAR);
		return true;
	}
	if ( is_affected( victim, gsn_fiery_armour ))
	{
		if (victim == ch)
			ch->println( "You already have armour of fire." );
		else
			act("$N already has armour of fire.",ch,NULL,victim,TO_CHAR);
		return true;
	}
	else
		return false;
}

/**************************************************************************/
// helper function for fire/cold shield type spells
bool is_aff_cold_shield( char_data *ch, char_data *victim )
{

	if ( is_affected( victim, gsn_chill_shield ))
	{
		if (victim == ch)
			ch->println( "You already have a chill shield." );
		else
			act("$N already has a shield of icy magic.",ch,NULL,victim,TO_CHAR);
		return true;
	}

	if ( is_affected( victim, gsn_wall_of_ice ))
	{
		if (victim == ch)
			ch->println( "You're already surrounded by ice." );
		else
			act("$N is already surrounded by ice.",ch,NULL,victim,TO_CHAR);
		return true;
	}
	if ( is_affected( victim, gsn_icy_armour ))
	{
		if (victim == ch)
			ch->println( "You already have armour of ice." );
		else
			act("$N already has armour of ice.",ch,NULL,victim,TO_CHAR);
		return true;
	}
	else
		return false;
}

/**************************************************************************/
// helper function for fire/cold shield type spells
bool check_cold_shield(int level, char_data *ch, char_data *victim)
{
	if (is_affected(victim, gsn_icy_armour ))
	{
		if (check_dispel(level,victim, gsn_icy_armour ))
		{
			act( "$n's armour of ice melts away.", victim, NULL, NULL, TO_ROOM );
			return true;
		}
		ch->println( "Spell failed." );
		return true;
	}
	else if (is_affected(victim, gsn_chill_shield ))
	{
		if (check_dispel(level,victim, gsn_chill_shield ))
		{
			act( "$n's chill shield is destroyed.", victim, NULL, NULL, TO_ROOM );
			return true;
		}
		ch->println( "Spell failed." );
		return true;
	}
	else if (is_affected(victim, gsn_wall_of_ice ))
	{
		if (check_dispel(level,victim, gsn_wall_of_ice ))
		{
			act( "$n's wall of ice melts away.", victim, NULL, NULL, TO_ROOM );
			return true;
		}
		ch->println( "Spell failed." );
		return true;
	}
	else
		return false;
}

/**************************************************************************/
// helper function for fire/cold shield type spells
bool check_fire_shield(int level, char_data *ch, char_data *victim)
{
	if (is_affected(victim, gsn_fire_shield ))
	{
		if (check_dispel(level,victim, gsn_fire_shield ))
		{
			act( "$n's fire shield is quelled.", victim, NULL, NULL, TO_ROOM );
			return true;
		}
		ch->println( "Spell failed." );
		return true;
	}
	else if (is_affected(victim, gsn_fiery_armour ))
	{
		if (check_dispel(level,victim, gsn_fiery_armour ))
		{
			act( "$n's armour of fire is quelled.", victim, NULL, NULL, TO_ROOM );
			return true;
		}
		ch->println( "Spell failed." );
		return true;
	}
	else if (is_affected(victim, gsn_wall_of_fire ))
	{
		if (check_dispel(level,victim, gsn_wall_of_fire ))
		{
			act( "$n's wall of fire is destroyed.", victim, NULL, NULL, TO_ROOM );
			return true;
		}
		ch->println( "Spell failed." );
		return true;
	}
	else
		return false;
}

/**************************************************************************/
// helper func for skin spells
bool check_skin( char_data *ch, char_data *victim)
{
	if( is_affected( victim, gsn_iron_skin )
	 || is_affected( victim, gsn_barkskin )
	 || is_affected( victim, gsn_rocky_skin )
	 || is_affected( victim, gsn_tough_skin )
	 || is_affected( victim, gsn_stone_skin ))
	{
		if (victim == ch)
			ch->println( "Your skin has already been altered." );
		else
			act("$N's skin has already been altered.",ch,NULL,victim,TO_CHAR);
		return true;
	}
	else
		return false;
}

/**************************************************************************/
// helper func for strength spells
bool check_strength( int level, char_data *ch, char_data *victim, bool& half )
{
	if( is_affected( victim, gsn_weaken ))
	{
		if( check_dispel( level, victim, gsn_weaken ))
		{
			act( "Your magic unravels a spell on $N.", ch, NULL, victim, TO_CHAR );
			return true;
		}
		else
		{
			ch->println( "You failed." );
			return true;
		}
	}


	if ( is_affected( victim, gsn_giant_strength )
	  || is_affected( victim, gsn_ogre_strength )
	  || is_affected( victim, gsn_dragon_strength ))
	{
		if (victim == ch)
			ch->println( "Your strength has already been heightened." );
		else
			act("$N's strength has already been heightened.",ch,NULL,victim,TO_CHAR);
		half = true;
		return false;
	}
	return false;
}

/**************************************************************************/
SPRESULT spell_create_bouquet( int , int , char_data *ch, void *,int  )
{
	OBJ_DATA *rose;

	if (get_obj_index(OBJ_VNUM_ROSE)){
		rose = create_object(get_obj_index(OBJ_VNUM_ROSE));

		replace_string(rose->description, 
			"A bouquet of red roses sparkling with natures bloom lies here." );
		replace_string(rose->short_descr, 
			"a bouquet of beautiful red roses" );
		replace_string(rose->name, "bouquet red roses" );
		replace_string(rose->material, "flowers" );

		rose->wear_flags  = OBJWEAR_TAKE + OBJWEAR_HOLD;
		rose->item_type	  = ITEM_TRASH;
		rose->extra_flags = OBJEXTRA_GLOW + OBJEXTRA_BLESS + OBJEXTRA_NO_DEGRADE;

		act("$n has created a bouquet of beautiful red roses!",ch,rose,NULL,TO_ROOM);
		ch->println( "You create a bouquet of beautiful red roses!" );
		obj_to_char(rose,ch);
	}else{
        ch->println( "BUG: No available object in spell_create_bouquet - please report to code!" );
		return NO_MANA;
	}
	return FULL_MANA;
}
/**************************************************************************/
