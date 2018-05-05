/**************************************************************************/
// magic_sb.cpp - spells/skills introduced by stormbringer coders
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

/**************************************************************************/
// From Secret Destroyer MUD by Luke. 
// Added by Meerclar 03-17-2002 for SB:R
SPRESULT spell_vicegrip(int sn, int level, char_data *ch, void *vo, int target)
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if (victim != ch)
	{		
		ch->println("You may only cast this on yourself.");
		return HALF_MANA;
	}

	if (IS_AFFECTED2(victim, sn)){		
		if (victim == ch){
			ch->println("You already have a vicegrip on your weapon.");
		}
		return HALF_MANA;
	}

	af.where	= WHERE_AFFECTS2;
	af.type		= sn;
	af.level	= level;
	af.duration	= 20;
	af.location	= APPLY_AC;
	af.modifier	= -5;
	af.bitvector= AFF2_VICEGRIP;
	
	affect_to_char(victim, &af);
	act("$n's hands grip their weapon with an iron hold.",victim, NULL,NULL, TO_ROOM);
	act("You hold on to your weapon with all your might.",victim, NULL,NULL, TO_CHAR);
	return FULL_MANA;
}
/***************************************************************************/

SPRESULT spell_sb_gate( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	OBJ_DATA *gatec;
    OBJ_DATA *gatev;

//	bool gate_pet;

	if (( victim = get_char_icworld( ch, target_name ) ) == NULL
	 ||   victim == ch
	 ||   victim->in_room == NULL
	 ||   !can_see_room(ch,victim->in_room)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_NOSCRY)
	 ||	  IS_SET(victim->in_room->room_flags, ROOM_ANTIMAGIC)
	 ||   IS_SET(victim->in_room->area->area_flags, AREA_NOSCRY)
	 ||   IS_SET(victim->in_room->area->area_flags, AREA_NOGATEINTO)	 
	 ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	 ||   victim->level >= level + 3
	 ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)  // NOT trust
	 ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
	 ||   (IS_NPC(victim) && saves_spell( level, victim,DAMTYPE(sn)))
	 ||   (!IS_NPC(victim) && saves_spell( level, victim, DAMTYPE(sn))))

	{
		ch->println("A shimmering gateway appears, but seems unstable");
        act("A shimmering gateway swirls into existance.",ch,NULL,NULL,TO_ROOM);

        gatec = create_object( get_obj_index(OBJ_VNUM_PORTAL)); //Needs a new OBJ_VNUM_GATE
        gatec->timer = 10;
        obj_to_room(gatec, ch->in_room);
		SET_BIT(gatec->value[2],GATE_OPAQUE);
		  return FULL_MANA;

	}else{

		if ( ch->in_room->area->continent != victim->in_room->area->continent ){
			ch->println( "The spell cannot span such a great distance." );
			return FULL_MANA;
		}

	    ch->println("A shimmering gateway swirls into existance.");
        act("A shimmering gateway swirls into exastance.", ch,NULL,NULL,TO_ROOM);
        victim->printf("A shimmering gateway swirls into existance.");
        act("A shimmering gateway swirls into existance.",victim,NULL,NULL,TO_ROOM);

        gatec = create_object(get_obj_index(OBJ_VNUM_PORTAL));
        gatev = create_object(get_obj_index(OBJ_VNUM_PORTAL));
		SET_BIT(gatec->value[2],GATE_SHORT_LOOKINTO);
		SET_BIT(gatev->value[2],GATE_SHORT_LOOKINTO);
        gatec->value[3] = victim->in_room->vnum;
        gatev->value[3] = ch->in_room->vnum;
        gatec->timer = 10;
        gatev->timer = 10;
        obj_to_room( gatec, ch->in_room);
        obj_to_room( gatev, victim->in_room);
        return FULL_MANA;
	}

}

/**************************************************************************/
SPRESULT spell_sb_portal( int sn, int level, char_data *ch, void *,int )

{
	char_data *victim;
	OBJ_DATA *portal;

	if ((victim = get_char_world( ch, target_name ) ) == NULL
	||	 victim == ch
    ||	 victim->in_room == NULL
    ||	 !can_see_room(ch,victim->in_room)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	||	 IS_SET(victim->in_room->room_flags, ROOM_ANTIMAGIC)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||	 IS_SET(victim->in_room->area->area_flags, AREA_NOPORTALINTO)
    ||	 IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
   	||	 victim->level >= level + 3
    ||	 (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
	||	 (IS_NPC(victim) && saves_spell( level, victim,DAMTYPE(sn)) )
    ||	 (is_clan(victim) && !is_same_clan(ch,victim)))

    {
        ch->printf( "A glowing portal appears, but seems unstable.\r\n" );
		act("A glowing portal appears, but seems unstable.",ch,NULL,NULL,TO_ROOM);
		portal = create_object( get_obj_index ( OBJ_VNUM_PORTAL ));
        portal->timer = 6;
        obj_to_room( portal, ch->in_room);
		SET_BIT(portal->value[2],GATE_OPAQUE);
        return FULL_MANA;

    }else{   
	
	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL));
	portal->timer = 2 + level / 25; 
	portal->value[3] = victim->in_room->vnum;

	// portals aren't opaque - Kal August 98
    //	SET_BIT(portal->value[2],GATE_OPAQUE);  
	// Now we can see into stable portals - Balo - 07-12-01

    SET_BIT(portal->value[2],GATE_SHORT_LOOKINTO);

	obj_to_room(portal,ch->in_room);

	act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
	ch->printf( "A glowing portal appears.\r\n" );
	//act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
	return FULL_MANA;

	}

}

/**************************************************************************/
//Changed from orig blindness func by Meerclar 24Oct2002
SPRESULT spell_sb_blindness( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_BLIND))
	{
		ch->printlnf( "%s already appears to be blinded.", PERS(victim, ch));
		return HALF_MANA;
	}

	if (saves_spell(level,victim,DAMTYPE(sn)))
	{
		ch->printlnf( "%s seems unaffected.", PERS(victim, ch) );
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.location  = APPLY_HITROLL;
	af.modifier  = -40; //Orig value -4
	af.duration  = 1+level;
	af.bitvector = AFF_BLIND;
	affect_to_char( victim, &af );
	victim->println( "You are blinded!" );
	act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}

/**************************************************************************/
// Changed by Meerclar, 24Oct2002 - changes noted inline
// RT calm spell stops all fighting in the room

SPRESULT spell_sb_calm( int sn, int level, char_data *ch, void *,int )
{
	char_data *vch;
	int mlevel = 0;
	int count = 0;
	int high_level = 0;    
	int chance;
	AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if (vch->position == POS_FIGHTING)
		{
			count++;
			if (IS_NPC(vch))
				mlevel += vch->level;
			else
				mlevel += vch->level/2;
			high_level = UMAX(high_level,vch->level);
		}
	}

    /* compute chance of stopping combat */
	chance = 4 * level - high_level + 2 * count;

	if ( IS_IMMORTAL( ch )) /* always works */
		mlevel = 0;

	if ( number_range( 0, chance ) >= mlevel )  /* hard to stop large fights */
	{
		for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
		{
			if ( IS_NPC( vch )
			&& ( IS_SET( vch->imm_flags, IMM_MAGIC )
			||   IS_SET( vch->act, ACT_UNDEAD )))
			return FULL_MANA;

			if ( IS_AFFECTED( vch, AFF_CALM )
			||   IS_AFFECTED( vch, AFF_BERSERK )
			||   is_affected( vch, gsn_frenzy ))
			return FULL_MANA;
	    
			vch->println( "A wave of calm passes over you." );

			if (vch->fighting || vch->position == POS_FIGHTING )
				stop_fighting( vch, false );


			af.where = WHERE_AFFECTS;
			af.type = sn;
			af.level = level;
			af.duration = level/4;
			af.location = APPLY_HITROLL;

			if (!IS_NPC(vch))
				af.modifier = -50; //Changed from orig -5 value
			else
				af.modifier = -20; //Changed from orig -2 value

			af.bitvector = AFF_CALM;
			affect_to_char(vch,&af);
			af.location = APPLY_DAMROLL;
			affect_to_char(vch,&af);
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
// Changed to useful healing levels by Meerclar 24Oct2002
SPRESULT spell_sb_cure_critical( int , int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int heal		  = dice(3,8)+level-6;

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );
	victim->println( "You feel better!" );
	if ( ch != victim ){
		ch->println( "Ok." );
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_sb_cure_light( int , int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int heal		  = dice(1, 8)+level/3;

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );
	victim->println( "You feel better!" );
	if ( ch != victim ){
		ch->println( "Ok." );
	}
    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_sb_cure_serious( int , int level, char_data *ch, void *vo,int  )
{
    char_data *victim = (char_data *) vo;
    int heal		  = dice(2, 8)+level/2;

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );
	victim->println( "You feel better!" );

	if ( ch != victim ){
		ch->println( "Ok." );
	}
	return FULL_MANA;
}

/**************************************************************************/
// RT really nasty high-level attack spell
// movement and hp costs to caster reduced to make cleric types more attractive
SPRESULT spell_sb_holy_word(int sn, int level, char_data *ch, void *,int )
{
	char_data *vch;
	char_data *vch_next;
	int dam;
	int bless_num, curse_num, frenzy_num;
   
	bless_num = gsn_bless;
	curse_num = gsn_curse; 
	frenzy_num = gsn_frenzy;

	act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
	ch->printf( "You utter a word of divine power.\r\n" );

	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
	{
		vch_next = vch->next_in_room;

		if ((IS_GOOD(ch) && IS_GOOD(vch))
		||  (IS_EVIL(ch) && IS_EVIL(vch))
		||  (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)))
		{
			vch->printf( "You feel full more powerful.\r\n" );
			spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR);
			spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
		}

		else if ((IS_GOOD(ch) && IS_EVIL(vch))
			 || (IS_EVIL(ch) && IS_GOOD(vch)))
		{
			if (!is_safe_spell(ch,vch,true))
			{
				spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
				vch->printf( "You are struck down!\r\n" );
				dam = dice(level,12);
				damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
			}
		}
		else if (IS_NEUTRAL(vch))
		{
			if (!is_safe_spell(ch,vch,true))
			{
				spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR);
				vch->printf( "You are struck down!\r\n" );
				dam = dice(level,10);
				damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
			}
		}
	}

	ch->printf( "You feel drained.\r\n" );
	ch->move /= 2;
	ch->hit *= 3/4;

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_sb_protection_evil(int sn,int level,char_data *ch,void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL)
	||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
	{
		if (victim == ch)
			ch->printf( "You are already protected.\r\n" );
		else
			act("$N is already protected.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_SAVES;
	af.modifier  = -1 * level/8;
	af.bitvector = AFF_PROTECT_EVIL;
	affect_to_char( victim, &af );
	victim->printf( "You feel holy and pure.\r\n" );
	if ( ch != victim )
		act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
	return FULL_MANA;
}
 
/**************************************************************************/
SPRESULT spell_sb_protection_good(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
 
	if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD)
	||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
	{
		if (victim == ch)
			ch->printf( "You are already protected.\r\n" );
		else
			act("$N is already protected.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_SAVES;
	af.modifier  = -1 * level/8;
	af.bitvector = AFF_PROTECT_GOOD;
	affect_to_char( victim, &af );
	victim->printf( "You feel aligned with darkness.\r\n" );
	if ( ch != victim )
		act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_sb_word_of_recall( int, int, char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	ROOM_INDEX_DATA *location = NULL;

	if(IS_NPC(victim)){
		return NO_MANA;
	}

	// class recall spot first, then race
    // recall_vnum = pc_race_table[ch->race].recall_room; 
	if ( class_table[victim->clss].recall)
	{
		location = get_room_index( class_table[victim->clss].recall ); // Class
	}
	else  // Class recall room non-existant, check race room
	{
		location = get_room_index( race_table[victim->race]->recall_room); // Race
	}
	

	if ( location == NULL )
	{
		victim->println( "You are completely lost." );
		return NO_MANA;
	}

	if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL)
	||  IS_SET(victim->in_room->room_flags, ROOM_ANTIMAGIC)
	||  IS_AFFECTED(victim,AFF_CURSE))
    {
		victim->println( "Spell failed." );
		return FULL_MANA;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim,true);
    
	// Stop negative moves characters from defrauding the system
	if (ch->move>0 && IS_IC(ch))
		ch->move /= 3*4;

	act("$n disappears.",victim,NULL,NULL,TO_ROOM);
	char_from_room(victim);
	char_to_room(victim,location);
	act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
	do_look(victim,"auto");
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_sb_strength_of_the_land( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA af;

	if ( is_affected( ch, gsn_illusions_grandeur )) {
		ch->printf( "Spell failed.\r\n" );
		return HALF_MANA;
	}

	//Switch to check sectors
	switch ( ch->in_room->sector_type ) {
	case SECT_INSIDE:
	case SECT_CITY:
		ch->printf( "You're too far from Nature to summon the strength.\r\n" );
		return HALF_MANA;
		break;
	case SECT_AIR:
		ch->printf( "You try to summon the land's strength, but are too high in the air.\r\n" );
		return HALF_MANA;
		break;
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
		ch->printf( "You try to summon the land's strength, but are too far from land itself.\r\n" );
		return HALF_MANA;
		break;
	case SECT_UNDERWATER:
		ch->printf( "You try to summon the land's strength, but are too deep underwater to do so.\r\n" );
		return HALF_MANA;
		break;
// Always happens if not in the above sectors :)
//	default:	//Should never happen.
//		return HALF_MANA;
//		break;
	}

	if ( !is_affected(ch,sn) )
	{
		af.where		= WHERE_AFFECTS;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/8;
		af.location		= APPLY_HIT;
		af.modifier		= level*2;
		af.bitvector	= 0;
		affect_to_char( ch, &af );
		
		af.location   = APPLY_AC;
		af.modifier   = level/8;
		af.bitvector  = 0;
		affect_to_char( ch, &af );
		
		af.location   = APPLY_SAVES;
		af.modifier   = level/8;
		af.bitvector  = 0;
		affect_to_char( ch, &af );

		ch->printf( "The power of the land enriches you!\r\n" );
		act( "$n glows bright green for a moment.\r\n", ch, NULL, NULL, TO_ROOM );
	}
	else
	{
		ch->printf( "You are already enriched with the power of the land.\r\n" );
		return HALF_MANA;
	}
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_sb_magic_missile( int sn, int level, char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;
  int num_missile;
	
//start missile strikes
  num_missile = level/10 + 1;
	dam		= dice(level/4,5);
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	num_missile -= 1;
	
	while (num_missile > 0){
		dam		= dice(level/4,5);
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	num_missile -= 1;
  }
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_sb_illusions_grandeur( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( is_affected( victim, gsn_strength_of_the_land )) {
		ch->println("Spell failed.");
		return HALF_MANA;
	}
	
	if(!is_affected(victim,sn))
	{
		af.where		= WHERE_AFFECTS;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/10;
		af.location		= APPLY_HIT;
		af.modifier		= level;
		af.bitvector	= 0;
		affect_to_char( victim, &af );
		
		af.location   = APPLY_AC;
		af.modifier   = level/8;
		af.bitvector  = 0;
		affect_to_char( ch, &af );
		
		af.location   = APPLY_SAVES;
		af.modifier   = level/8;
		af.bitvector  = 0;
		affect_to_char( ch, &af );

		victim->println("You appear mightier than you are!");
		act( "$n grows in stature and appears quite mighty now!", victim, NULL, NULL, TO_ROOM );
	}
	else
	{
		victim->println("You are having delusions of grandeur now.");
		return HALF_MANA;
	}
	return FULL_MANA;
}

/**************************************************************************/

SPRESULT spell_sb_poison_rain( int sn, int level, char_data *ch, void *, int  )
{
	char_data	*vch;
	char_data       *vch_next;
	int			 dam;
	AFFECT_DATA af;
	
	if ( !IS_OUTSIDE( ch ) )
	{
		ch->println("You must be out of doors.");
		return NO_MANA;
	}
	
	if ( ch->in_room->sector_type == SECT_CAVE )
	{
		ch->println("You can't make it rain here.");
		return NO_MANA;
	}

	dam = dice( level , 8 );
	
	af.where	 = WHERE_AFFECTS;
	af.type		 = gsn_poison;
	af.level	 = level;
	af.duration  = level;
	af.location  = APPLY_ST;
	af.modifier  = -20;
	af.bitvector = AFF_POISON;
	
	ch->println("A burning rain falls on your enemy's heads!");
	act( "$n brings forth a rain of poison!", ch, NULL, NULL, TO_ROOM );
	
    for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
        vch_next=vch->next_in_room;
		// don't affect unseen mobs
		if ( IS_NPC(vch) && IS_SET(vch->act, ACT_IS_UNSEEN)){
			continue;
		}

		if (vch->in_room == NULL)
			continue;
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch
				&& ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
				&& !HAS_CLASSFLAG(vch, CLASSFLAG_POISON_IMMUNITY))
			{
				damage_spell( ch, vch, saves_spell( level, vch, DAMTYPE(sn) ) ? dam /2 : dam, sn, DAMTYPE(sn), true );
				if(!saves_spell(level,vch, DAMTYPE(sn)))
					affect_join( vch, &af );
			}
			continue;
		}
	}
	
	return FULL_MANA;
}

/**************************************************************************/
