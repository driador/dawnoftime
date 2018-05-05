/**************************************************************************/
// magic_ce.cpp - spells/skills written by Celrion
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
DECLARE_DO_FUN( do_affects		);

/**************************************************************************/
SPRESULT spell_chaotic_poison( int sn, int level, char_data *, void *vo, int  )
{
    char_data *victim = ( char_data * ) vo;
    AFFECT_DATA af;

	if ( is_affected( victim, sn ))
	{
		act("$n is already very ill.",victim,NULL,NULL,TO_ROOM);
		return HALF_MANA;
	}

    if ( saves_spell( level, victim, DAMTYPE(sn)) 
		|| HAS_CLASSFLAG(victim, CLASSFLAG_POISON_IMMUNITY))
    {
		act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
		victim->printf( "You feel momentarily ill, but it passes.\r\n" );
		return FULL_MANA;
    }

    af.where     = WHERE_AFFECTS;
    af.type      = sn;
    af.level     = number_range( victim->level-5, victim->level+5 );
    af.duration  = number_range( victim->level-5, victim->level+5 );
    af.location  = APPLY_ST;
    af.modifier  = number_range( -10, -2 );
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );

    if ( af.level > 60 )
        victim->printf( "You feel extremely sick.\r\n" );
    else if ( af.level > 40 )
        victim->printf( "You feel incredibly sick.\r\n" );
    else if ( af.level > 20 )
        victim->printf( "You feel very sick.\r\n" );
    else
        victim->printf( "You feel somewhat sick.\r\n" );

    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_protection_acid( int sn, int level, char_data *, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if(!is_affected(victim,sn))
	{
		af.where = WHERE_RESIST;
		af.type = sn;
		af.level = level;
		af.duration = level/4+10;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector   = RES_ACID;
		affect_to_char(victim, &af);
		victim->printf( "You are protected from all forms of acid.\r\n" );
	}
	else
	{
		victim->printf( "You are already protected from acid.\r\n" );
		return HALF_MANA;
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_holy_beam( int sn, int level, char_data *ch, void *vo, int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	int dam;

	act("$n summons forth a divine beam of light!",ch,NULL,NULL,TO_ROOM);
	ch->printf( "You summon forth a divine light.\r\n" );

	if ( saves_spell( level, victim, DAMTYPE(sn)))
	{
		ch->printf( "They seem unaffected.\r\n" );
		return FULL_MANA;
	}

	dam = dice(level, 6)+10;

	if ( IS_AFFECTED( victim, AFF_BLIND ) )
	{
		ch->printf( "They already appear to be blinded.\r\n" );
		dam /= 2;
	}
	else
	{
		af.where     = WHERE_AFFECTS;
		af.type      = gsn_blindness;
		af.level     = level;
		af.location  = APPLY_HITROLL;
		af.modifier  = -4;
		af.duration  = level+5;
		af.bitvector = AFF_BLIND;
		affect_to_char( victim, &af );
		victim->printf( "You are blinded by a beam of light!\r\n" );
		act("$n appears to be badly blinded by a beam of light.",victim,NULL,NULL,TO_ROOM);
	}

	victim->printf( "You feel your retina being seared.\r\n" );
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true );

	return FULL_MANA;
}

/**************************************************************************/
void do_diagnose( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
    char_data *patient; /* <grin> */
    int chance;

    one_argument(argument,arg);

    if ( ( chance = get_skill ( ch, gsn_diagnose ) ) == 0
       || (!IS_NPC(ch) && 
       ch->level < skill_table[gsn_diagnose].skill_level[ch->clss]))
    {
        ch->printf( "You couldn't diagnose a cold, much less anything else.\r\n" );
        return;
    }

    if (arg[0] == '\0')
    {
        ch->printf( "Diagnose who?\r\n" );
        return;
    }

    if ( ( patient = get_char_room ( ch, arg ) ) == NULL )
    {
        ch->printf( "They aren't here.\r\n" );
        return;
    }

    if ( patient == ch )
    {
        do_affects ( ch, "" );
        return;
    }

    if ( patient->position >= POS_STANDING )
    {
        ch->printf( "Have your patient sit or rest first.\r\n" );
        return;
    }

    if ( patient->mounted_on )
    {
        ch->printf( "You can't very well diagnose someone who is mounted.\r\n" );
        return;
    }

    chance += ( ch->modifiers[STAT_EM] + ch->modifiers[STAT_RE] ) / 2;
    chance += ( ch->level - patient->level );

    act("$n looks over $N thoroughly.",ch,NULL,patient,TO_NOTVICT);
    act("$n looks you over thoroughly.",ch,NULL,patient,TO_VICT);

    if ( number_percent() < chance )
    {
        act("You look over $N and come to the following conclusions:",
			ch,NULL,patient,TO_CHAR);

		if ( !IS_NPC( patient ))
		{
	        sprintf(buf, " They are");
	        if ( patient->pcdata->tired > 16 ) strcat( buf, " tired," );
		        ( patient->pcdata->condition[COND_HUNGER] == 0 ) ?
			      strcat( buf, " hungry," )	: strcat( buf, " full," );
				( patient->pcdata->condition[COND_DRUNK] == 0 ) ?
				  strcat( buf, " sober" )		: strcat( buf, " drunk" );
				( patient->pcdata->condition[COND_THIRST] == 0 ) ?
				  strcat( buf, ", thirsty." )	: strcat( buf, "." );
			strcat( buf, "\r\n" );
			ch->printf( "%s\r\n", buf );
		}

		if ( is_affected( patient, gsn_blindness ))
				ch->printf( " They appear to be blind.\r\n" );
		if ( is_affected( patient, gsn_curse ))
				ch->printf( " They are cursed.\r\n" );
		if ( is_affected( patient, gsn_plague ))
				ch->printf( " They look to have the plague.\r\n" );
		if ( is_affected( patient, gsn_poison ))
				ch->printf( " They appear poisoned.\r\n" );
		if ( is_affected( patient, gsn_change_sex ))
				ch->printf( " Their gender isn't what it used to be.\r\n" );
		if ( is_affected( patient, gsn_weaken ))
				ch->printf( " They appear incredibly weak.\r\n" );
		if ( is_affected( patient, gsn_haste ))
				ch->printf( " They appear to be moving incredibly fast.\r\n" );
		if ( is_affected( patient, gsn_slow ))
				ch->printf( " They appear to be stuck in slow motion.\r\n" );
		if ( is_affected( patient, gsn_deafness) )
				ch->printf( " They appear to be deaf.\r\n" );
		if ( is_affected( patient, gsn_cause_headache) )
				ch->printf( " They appear to have a headache.\r\n" );
		check_improve(ch,gsn_diagnose,true,6);
	}
	else
	{
		act("You can't seem to diagnose $N properly.", ch, NULL, patient, TO_CHAR);
		check_improve(ch,gsn_diagnose,false,6);
	}

	WAIT_STATE(ch,skill_table[gsn_diagnose].beats);
	return;
}
/**************************************************************************/
SPRESULT spell_resist_weapons( int sn, int level, char_data *ch, void *,int )
{
	AFFECT_DATA af;
	
	if( is_affected(ch, sn) )
	{
		ch->printf( "You're already protected against weapons!\r\n" );
		return HALF_MANA;
	}

		af.where = WHERE_RESIST;
		af.type = sn;
		af.level = level;
		af.duration = level/4;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector   = RES_WEAPON;
		affect_to_char(ch, &af);

	ch->printf( "You feel more resistant to weapons.\r\n" );
	act( "$n's skin ripples slightly.", ch, NULL, NULL, TO_ROOM );

	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_strength_of_the_land( int sn, int level, char_data *ch, void *, int )
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
		af.modifier		= level;
		af.bitvector	= 0;
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
SPRESULT spell_deafness( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if( is_affected( victim, gsn_augment_hearing )){
		if( check_dispel( level, victim, gsn_augment_hearing )){
			act( "Your magic unravels a spell on $N.", ch, NULL, victim, TO_CHAR );
			act( "Your hearing returns.", victim, NULL, NULL, TO_CHAR );
			return FULL_MANA;
		}else{
			ch->printf( "You failed.\r\n" );
			return FULL_MANA;
		}
	}

	if ( is_affected(victim, sn) )
	{
		ch->printf( "They have already been deafened.\r\n" );
		return HALF_MANA;
	}

	if (saves_spell(level,victim,DAMTYPE(sn)))
	{
		ch->printf( "They seem unaffected.\r\n" );
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.duration  = 1+level;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	victim->printf( "You have been deafened!\r\n" );
	act("$n's ears glow for a moment.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_cure_deafness(int ,int level,char_data *ch,void *vo,int )
{
    char_data *victim = (char_data *) vo;

	if ( !is_affected( victim, gsn_deafness ) )
	{
		if (victim == ch)
			ch->printf( "You aren't deaf.\r\n" );
		else
			act("$N doesn't appear to be deafened.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}
 
	if (check_dispel(level,victim,gsn_deafness))
	{
		victim->printf( "Your hearing returns!\r\n" );
		act("$n is no longer deafened.",victim,NULL,NULL,TO_ROOM);
	}
	else
		ch->printf( "Spell failed.\r\n" );

	return FULL_MANA;
}
/**************************************************************************/
void do_sharpen( char_data *ch, char *)
{
	OBJ_DATA *weapon = ( get_eq_char( ch, WEAR_WIELD ));
	OBJ_DATA *stone  = ( get_eq_char( ch, WEAR_HOLD  ));
	OBJ_INDEX_DATA *scrap;
	int chance, degrade;
	
	if ((chance = get_skill(ch,gsn_sharpen)) == 0)
	{
		do_huh(ch,"");
		return;
	}
	
	if ( !weapon )
	{
		ch->printf( "You need to be wielding something to sharpen.\r\n" );
		return;
	}
	
	if ( !stone || str_cmp( stone->pIndexData->material, "whetstone" ) )
	{
		ch->printf( "You need to be holding a sharpening stone.\r\n" );
		return;
	}
	
	if ( (weapon->value[0] != WEAPON_DAGGER
		&& weapon->value[0] != WEAPON_SWORD
		&& weapon->value[0] != WEAPON_AXE) 
		|| IS_WEAPON_STAT(weapon,WEAPON_VORPAL))
	{
		act("You cannot sharpen $p.", ch, weapon, NULL, TO_CHAR);
		return;
	}
	
	//Chance changes
	chance += ch->modifiers[STAT_SD];
	chance += ch->modifiers[STAT_ST] / 5;
	
	//Condition change
	degrade = chance > 100 ? number_range(100, chance) : number_range(chance, 100);
	degrade = degrade > 100 ? degrade - 100 : 100 - degrade;
	weapon->condition -= degrade;
	
	act("You begin to sharpen $p.", ch, weapon, NULL, TO_CHAR);
	act("$n begins to sharpen $p.", ch, weapon, NULL, TO_ROOM);
	
	if ( weapon->condition <= 0 || chance < number_range(25, 75) )
	{
		act("You wind up completely ruining $p!", ch, weapon, NULL, TO_CHAR);
		act("$n turns $p into a pile of scrap.", ch, weapon, NULL, TO_ROOM);
		obj_from_char( weapon );
		
		if (( scrap = get_obj_index( OBJ_VNUM_MUSHROOM )) == NULL ) 
		{
			ch->printf( "Non-existant item for sharpen, please report this with a note to admin.\r\n" );
			bug("Non-existant item for sharpen.");
			return;
		}
		
		weapon = create_object( scrap);
		free_string( weapon->description );
		free_string( weapon->name );
		free_string( weapon->short_descr );
		
		weapon->description = str_dup( "A pile of scrap metal lies here." );
		weapon->short_descr = str_dup( "a pile of scrap metal" );
		weapon->name = str_dup( "scrap metal" );
		weapon->item_type = ITEM_TRASH;
		
		obj_to_char( weapon, ch );
		
		check_improve( ch, gsn_sharpen, false, 1); 
		WAIT_STATE(ch, skill_table[gsn_sharpen].beats * 3/2);
		return;
	}
	
	act("You complete your work on $p.", ch, weapon, NULL, TO_CHAR);
	act("$n finishes sharpening $p.", ch, weapon, NULL, TO_ROOM);
	
	if (!IS_WEAPON_STAT( weapon, WEAPON_SHARP )) 
		SET_BIT(weapon->value[4], WEAPON_SHARP);
	
	check_improve( ch, gsn_sharpen, true, 1);
	WAIT_STATE(ch, skill_table[gsn_sharpen].beats);
	return;
}
/**************************************************************************/
