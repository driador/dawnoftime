/**************************************************************************/
// magic_re.cpp - spells/skills written by Reave 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: magic_re.cpp - spells/skills written by Reave                    *
 *                       (and one by Raphael)                              *
 ***************************************************************************/
#include "include.h"
#include "magic.h"
#include "o_lookup.h"

//  Prototype definitions
DECLARE_DO_FUN( do_look );
void build_fire(  char_data *ch, int chance );
void build_raft(  char_data *ch, int chance );
void build_staff( char_data *ch, int chance );

/***************************************************************************
 *  Prayer - Adds +10 to a random stat modifier                            *
 *  By Reave, November 1998                                                *
 ***************************************************************************/
SPRESULT spell_prayer( int sn, int level, char_data *ch, void *vo, int )
{
	AFFECT_DATA		af;
    char_data *victim = (char_data *) vo;

	if ( is_affected( ch, sn )) 
	{
	    ch->println("You have already gained the favour of your god.");
	    return HALF_MANA;
	}

	int random;
	random = dice(1,13);

	ch->println("Your prayer has been granted.");

	if (random >= 1 && random <= 12)
	{
	    af.where     = WHERE_AFFECTS;
	    af.type      = sn;
	    af.level     = level;
	    af.duration  = level/4;
	    af.modifier  = 10;
	    af.bitvector = 0;
	    if (random == 1)
	    {
			af.location  = APPLY_ST;
			affect_to_char( victim, &af );
			victim->println("You feel stronger.");
		}
		if (random == 2)
		{
			af.location  = APPLY_QU;
			affect_to_char( victim, &af );
			victim->println("You feel quicker.");
		}
		if (random == 3)
		{
			af.location  = APPLY_PR;
			affect_to_char( victim, &af );
			victim->println("You feel more self-confident.");
		}
		if (random == 4)
		{
			af.location  = APPLY_EM;
			affect_to_char( victim, &af );
			victim->println("You feel more one with the universe.");
	    }
	    if (random == 5)
	    {
			af.location  = APPLY_IN;
			affect_to_char( victim, &af );
			victim->println("You feel lucky.");
	    }
	    if (random == 6)
	    {
			af.location  = APPLY_CO;
			affect_to_char( victim, &af );
			victim->println("You feel more durable.");
		}
		if (random == 7)
		{
			af.location  = APPLY_AG;
			affect_to_char( victim, &af );
			victim->println("You feel more agile.");
	    }
	    if (random == 8)
		{
			af.location  = APPLY_SD;
			affect_to_char( victim, &af );
			victim->println("You feel more self-disciplined.");
		}
		if (random == 9)
		{
			af.location  = APPLY_ME;
			affect_to_char( victim, &af );
			victim->println("You feel like your memory has expanded.");
	    }
		if (random == 10)
		{
			af.location  = APPLY_RE;
			affect_to_char( victim, &af );
			victim->println("You feel more intelligent.");
		}
		if (random == 11)
		{
			af.location  = APPLY_HITROLL;
			affect_to_char( victim, &af );
			victim->println("You feel like you can use your weapon better.");
		}
		if (random == 12)
		{
			af.location  = APPLY_DAMROLL;
			affect_to_char( victim, &af );
			victim->println("You feel like you can do more damage with your weapon.");
		}
	}
	else
	{
		af.where     = WHERE_AFFECTS;
		af.type      = sn;
		af.level     = level;
		af.duration  = level/4;
		af.modifier  = -10;
		af.bitvector = 0;
		af.location  = APPLY_SAVES;
		affect_to_char( victim, &af );
		victim->println("You feel a bit more protected from magic.");
	}
	
	act("$n glows very slightly for a moment.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}


/*********************************************************************************
 *  Wildstrike - Wild magic attack spell, pure energy + chance of random affect  *
 *  By Reave, November 1998                                                      *
 *********************************************************************************/

SPRESULT spell_wildstrike( int sn, int level, char_data *ch, void *vo,int  )
{
    char_data *victim = (char_data *) vo;
    AFFECT_DATA af;
    
    int chance;             /* 4% chance caster hits himself */
	chance = dice(1,25);
    if (chance == 1)
	{
		act("$n blasts $mself with a stream of pure energy!",ch,NULL,victim,TO_NOTVICT);
        act("Your stream of pure energy goes wild and hits you!",ch,NULL,victim,TO_CHAR);
        
		int dam;
        dam	= dice(level, 10);

		if ( saves_spell( level, victim,DAMTYPE(sn) )) dam /= 2;

		damage_spell( ch, ch, dam, sn, DAMTYPE(sn),true );
        return FULL_MANA;
	}
	else                  /* 96% chance caster hits intended target */
	{
	act("$n blasts $N with a stream of pure energy!",ch,NULL,victim,TO_NOTVICT);
    act("$n blasts you with a stream of pure energy!",ch,NULL,victim,TO_VICT);
    act("You blast $N with a stream of pure energy!",ch,NULL,victim,TO_CHAR);
	
	int dam;
    dam	= dice(level, 9);

	if ( !saves_spell( level, victim,DAMTYPE(sn) )) {
        int dtoss;
        dtoss = dice(1,28); /* 25% chance of random short-duration 
		                     bad affect in addition to full damage */
	
		switch (dtoss) {
			case 1 :
				if ( !is_affected( ch, gsn_blindness )) {
				    af.where     = WHERE_AFFECTS;
				    af.type      = gsn_blindness;
				    af.level     = level;
				    af.location  = APPLY_HITROLL;
				    af.modifier  = -4;
					af.duration  = level / 10;
				    af.bitvector = AFF_BLIND;
				    affect_to_char( victim, &af );
				    victim->println("You are blinded!");
				    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
				}
			    break;
			case 2 :
				if ( !is_affected( ch, gsn_weaken )) {
					af.where     = WHERE_AFFECTS;
				    af.type      = gsn_weaken;
					af.level     = level;
			        af.duration  = level / 10;
			        af.location  = APPLY_ST;
			        af.modifier  = -1 * (level / 3);
			        af.bitvector = AFF_WEAKEN;
			        affect_to_char( victim, &af );
			        victim->println("You feel your strength slip away.");
			        act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
				}
		        break;
			case 3 :
				if ( !is_affected( ch, gsn_giant_strength )) {
					af.where     = WHERE_AFFECTS;
				    af.type      = gsn_giant_strength;
				    af.level	 = level;
				    af.duration  = level / 10;
				    af.location  = APPLY_ST;
				    af.modifier  = 1 + level/5;
				    af.bitvector = 0;
				    affect_to_char( victim, &af );
			        victim->println("Your muscles surge with heightened power!");
				    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
				}
				break;
			case 4 :
				if ( !is_affected( ch, gsn_slow )) {
					af.where     = WHERE_AFFECTS;
				    af.type      = gsn_slow;
				    af.level     = level;
				    af.duration  = level/2;
				    af.location  = APPLY_QU;
				    af.modifier  = -1 - level/5;
				    af.bitvector = AFF_SLOW;
				    affect_to_char( victim, &af );
				    victim->println("You feel yourself slowing d o w n...");
				    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
				}
			    break;
			case 5 :
				ROOM_INDEX_DATA *pRoomIndex;
				pRoomIndex = get_random_room(victim);
			    if (victim != ch)
     		       victim->println("You have been teleported!");
	 	           act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
		           char_from_room( victim );
		           char_to_room( victim, pRoomIndex );
			       act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
			       do_look( victim, "auto" );
			    break;
		    case 6 :
				if ( !is_affected( ch, gsn_haste )) {
					af.where     = WHERE_AFFECTS;
					af.type      = gsn_haste;
					af.level     = level;
					af.duration  = level / 10;
					af.location  = APPLY_QU;
					af.modifier  = 1 + level/5;
					af.bitvector = AFF_HASTE;
					affect_to_char( victim, &af );
					victim->println("You feel yourself moving more quickly.");
					act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
				}
				break;
		   case 7 :
				if ( !is_affected( ch, gsn_faerie_fire )) {
					af.where     = WHERE_AFFECTS;
					af.type      = gsn_faerie_fire;
					af.level	 = level;
					af.duration  = level / 10;
					af.location  = APPLY_AC;
					af.modifier  = 2 * level;
					af.bitvector = AFF_FAERIE_FIRE;
					affect_to_char( victim, &af );
					victim->println("You are surrounded by a pink outline.");
					act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
				}
				break;
		   default:
		        break;
		}
	}
	else dam /= 2;
    
    damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true );
    return FULL_MANA;
    }
}


/***************************************************************************
 *  Smite - Clerical attack spell, alignment dependent damage              *
 *  By Reave, November 1998                                                *
 ***************************************************************************/
SPRESULT spell_smite(int sn, int level, char_data *ch, void *vo,int )
{
    char_data *victim = (char_data *) vo;
    
	int dam;

    act("$n calls down the wrath of the gods!",ch,NULL,NULL,TO_ROOM);
	ch->println("You call down the wrath of the gods!");

    dam = dice( level, 8 );

	// Should do normal damage unless good vs. evil - if this
	// is the case, will do 20% extra damage
	
	if ( IS_EVIL(ch) && IS_GOOD(victim) )
	{
	    dam = (dam*12)/10;
	    if ( saves_spell( level, victim, DAM_NEGATIVE ) )
	    dam /= 2;
	    damage_spell( ch, victim, dam, sn,DAM_NEGATIVE,true);
        return FULL_MANA;
	}
	if ( IS_GOOD(ch) && IS_EVIL(victim) )
	{
		dam = (dam*12)/10;
	    if ( saves_spell( level, victim, DAM_HOLY ) )
	    dam /= 2;
	    damage_spell( ch, victim, dam, sn,DAM_HOLY,true);
        return FULL_MANA;
	}
    if ( IS_GOOD(ch) && IS_GOOD(victim) )
	{
		dam /= 3;
	    if ( saves_spell( level, victim, DAM_HOLY ) )
	    dam /= 2;
	    damage_spell( ch, victim, dam, sn,DAM_HOLY,true);
        return FULL_MANA;
	}
	if ( IS_EVIL(ch) && IS_EVIL(victim) )
	{
		dam /= 3;
	    if ( saves_spell( level, victim, DAM_HOLY ) )
	    dam /= 2;
	    damage_spell( ch, victim, dam, sn,DAM_HOLY,true);
        return FULL_MANA;
	}
	else
	{
		if ( saves_spell( level, victim, DAM_HOLY ) )
	    dam /= 2;
	    damage_spell( ch, victim, dam, sn,DAM_HOLY,true);
        return FULL_MANA;
	}
	return FULL_MANA;
}


/***************************************************************************
 *  Cantrip - Random minor magical affect                                  *
 *  By Raphael, November 1998                                              *
 ***************************************************************************/
SPRESULT spell_cantrip( int , int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	OBJ_DATA *ouritem;
	char_data *gch;
	AFFECT_DATA af;
	int ct=0;
	int res;

	if(level<20)
		res=dice(1,5);
	else
		res=dice(1,9);

	switch(res)
	{
	case 1:
		if ( level < 15 )
		{
			act("$n causes a small gust of wind to blow.", ch, NULL, NULL, TO_ROOM );
			ch->println("You cause a small gust of wind to blow.");
		}
		else
		{
			act("$n causes a small gust of wind to begin to blow.\nThe wind picks up speed, causing you to struggle to keep your footing!", ch, NULL, NULL, TO_ROOM );
			ch->println("You cause a small gust of wind to blow that quickly\npicks up enough speed to make it difficult to keep your footing!");
		}
		break;
	case 2:
		if ( level < 15 )
		{
			act("You hear a faint tinkling of ethereal music.", ch, NULL, NULL, TO_ROOM );
			ch->println("You cause a faint tinkling of ethereal music to be heard.");
		}
		else
		{
			act("A ghostly bard appears, playing some beautiful music.\nThe bard finishes, and fades from view.", ch, NULL, NULL, TO_ROOM );
			ch->println("You summon a ghostly bard that begins playing some beautiful music.\nThe bard finishes, and fades from view.");
		}
		break;
	case 3:
		if ( level < 15 )
		{
			act("$n causes a small glowing ball to float over $s hand!\nThe ball dances back and forth for a while, and then vanishes.",ch, NULL, NULL, TO_ROOM );
			ch->println("You cause a small glowing ball to float over your hand!\nThe ball dances back and forth for a while, and then vanishes.");
		}
		else
		{
			act("$n causes several glowing balls to float over $s hand!\nThe balls dance back and forth for a while, and then vanish.", ch, NULL, NULL, TO_ROOM );
			ch->println("You cause several glowing balls to float over your hand!\nThe balls dance back and forth for a while, and then vanish.");
		}
		break;
	case 4:
		if ( level < 15 ) 
		{
			act("You hear what sounds like the faint sound of thunder.", ch, NULL, NULL, TO_ROOM );
			ch->println("You cause the faint sound of thunder to be heard.");
		}
		else
		{
			act("$n claps $s hands together, causing a loud crash of thunder!", ch, NULL, NULL, TO_ROOM );
			ch->println("You clap your hands together, causing a loud crash of thunder!");
		}
		break;
	case 5:
		if ( level < 15 )
		{
			act("A faint aura begins to glow around $N,\nbut it quickly fades.", ch, NULL, victim,TO_NOTVICT);
			act("You begin to glow with a faint aura!\nThe aura quickly fades.",ch,NULL,victim,TO_VICT);
			act("You cause a faint aura to begin to glow around $N,\nbut it quickly fades.",ch,NULL,victim,TO_CHAR);
		}
		else
		{
			act("A bright green aura begins to glow around $N!\nThe aura shimmers and waivers for several seconds, and then fades.",ch,NULL,victim,TO_NOTVICT);
			act("A bright green aura begins to glow around you!\nThe aura shimmers and waivers for several seconds, and then fades.",ch,NULL,victim,TO_VICT);
			if ( victim!=ch )
				act("You cause a bright green aura to begin to glow around $N!\nThe aura shimmers and waivers for several seconds, and then fades.",ch,NULL,victim,TO_CHAR);
			else
				act("You cause a bright green aura to begin to glow around yourself!\nThe aura shimmers and waivers for several seconds, and then fades.",ch,NULL,victim,TO_CHAR);
		}
		break;
	case 6:	// make our papaya
		ouritem = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ));
		free_string( ouritem->description );
		free_string( ouritem->name );
		free_string( ouritem->short_descr );
		ouritem->value[0] = level / 2;
		ouritem->value[1] = level;
		ouritem->timer	  = level / 5;
		ouritem->description	= str_dup( "A nicely shaped papaya awaits to be munched on." );
		ouritem->short_descr	= str_dup( "a nice sized papaya" );
		ouritem->name			= str_dup( "papaya" );
		
		obj_to_room( ouritem, ch->in_room );
		act( "$p suddenly appears.", ch, ouritem, NULL, TO_ROOM );
		act( "$p suddenly appears.", ch, ouritem, NULL, TO_CHAR );
		break;
	case 7:
		for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
			if (( gch != ch ) && ( !IS_AFFECTED( gch, AFF_INVISIBLE )) && ( dice( 1,10 ) == 1 ))
			{
				ct++;
				act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
				gch->println("You slowly fade out of existence.");
				
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_invisibility;
				af.level     = level/2;
				af.duration  = 24;
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bitvector = AFF_INVISIBLE;
				affect_to_char( gch, &af );
			}
		}
		if ( ct )
		{
			if( ct > 1 )
				ch->printlnf( "Your spell has caused %d people to disappear!", ct);
			else
				ch->println("Your spell has caused one person to disappear!");
		}
		else
		{
			act("$N appears to fade in and out of existance!",ch,NULL,victim,TO_NOTVICT);
			act("$N appears to fade in and out of existance!",ch,NULL,victim,TO_VICT);
			if( victim != ch )
				act("You cause $N to appear to fade in and out of existance!",ch,NULL,victim,TO_CHAR);
			else
				act("You appear to fade in and out of existance!",ch,NULL,victim,TO_CHAR);
		}
		break;
	case 8:
		ouritem = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ));
		obj_to_room( ouritem, ch->in_room );
		act( "$n causes $p to appear!", ch, ouritem, NULL, TO_ROOM );
		act( "You cause $p to appear!", ch, ouritem, NULL, TO_CHAR );
		break;
	case 9:
		for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
		{
			if (( gch != ch ) && ( !IS_AFFECTED(gch, AFF_FLYING)) && ( dice( 1,10 ) == 1 ))
			{
				ct++;
				act( "$n's feet raise off the ground!", gch, NULL, NULL, TO_ROOM );
				gch->println("You begin to float above the ground!");
				
				af.where     = WHERE_AFFECTS;
				af.type      = gsn_fly;
				af.level     = level;
				af.duration  = level+3;
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bitvector = AFF_FLYING;
				affect_to_char( gch, &af );
			}
		}
		if ( ct )
		{
			if ( ct > 1 )
				ch->printlnf( "Your spell has caused %d people to fly!", ct );
			else
				ch->println("Your spell has caused one person to fly!");
		}
		else
		{
			ch->println("Your spell doesn't seem to have had any effect.");
		}
		break;
	default:
		break;
	}
	return FULL_MANA;
}

/***************************************************************************
 *  Build - woodcraft ability, build fires and various objects             *
 *  By Reave & Kerenos December 1998                                       *
 ***************************************************************************/
void do_build( char_data *ch, char *argument )
{
	int		chance;    	
	char	arg[MIL];

	argument = one_argument(argument,arg);

	chance = get_skill(ch,gsn_build) + 10;
	if ( chance < 11 ) {
		ch->println("You don't know how to build anything useful.");
		return;
    }

	if ( arg[0] == '\0' ) {
		ch->println("Build what?");
		ch->println("Syntax is 'build <object>'");
		ch->println("Only 'fire', 'raft' and 'staff' are available at this time.");
		return;
	}

	if (!str_cmp(arg,"fire"))
	{
		build_fire( ch, chance );
	}
	else if ( !str_cmp(arg,"raft")) // Kerenos
	{
		build_raft( ch, chance );
	}
	else if ( !str_cmp(arg,"staff")) // Kerenos
	{
		build_staff( ch, chance );
	}
	else
	{
		ch->println("You can't build that.");
		return;
	}
}

/***************************************************************************/
SPRESULT spell_augment_hearing( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;


	if( is_affected( victim, gsn_deafness )){
		if( check_dispel( level, victim, gsn_deafness )){
			act( "Your magic unravels a spell on $N.", ch, NULL, victim, TO_CHAR );
			return FULL_MANA;
		}else{
			ch->println("You failed.");
			return FULL_MANA;
		}
	}

	if ( is_affected(victim, sn)) {
		if (victim == ch){
			ch->println("Your hearing is already augmented.");
		}else{
			act("$N already has augmented hearing.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}

	af.where	 = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = level/2;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	victim->println("Your hearing is greatly augmented.");

	if ( ch != victim )
		act("$N's hearing is greatly augmented.",ch,NULL,victim,TO_CHAR);
    return FULL_MANA;
}

/***************************************************************************/
SPRESULT spell_aura_of_temperance( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA af;

	if ( !IS_IMMORTAL( ch )) {
		if ( IS_EVIL( ch ) || IS_GOOD( ch )) 
		{
			ch->println("You must walk within the path of balance to use this spell.");
			return NO_MANA;
		}
	}

	if ( is_affected( ch, sn )) {
			ch->println("You are already affected by this spell.");
		return HALF_MANA;
	}

	af.where	 = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 1 + level/20;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( ch, &af );

	ch->println("You are protected from aggressive creatures.");

    return FULL_MANA;
}
/***************************************************************************/
void build_fire( char_data *ch, int chance )
{
	if ( !IS_OUTSIDE( ch )
	||	  IS_WATER_SECTOR( ch->in_room->sector_type )
	||	  ch->in_room->sector_type == SECT_AIR )
	{
		ch->println("You can't build a fire here.");
		return;
	}

	if ( chance < number_range( 0, 105 ))
	{
		ch->println("You try very hard, but can't start a fire.");
		WAIT_STATE( ch, skill_table[gsn_build].beats );
		check_improve( ch, gsn_build, false, 1 );
		return;
	}
		
	OBJ_DATA *fire;

	if ( get_obj_index(OBJ_VNUM_FIRE) == NULL )
	{
		bugf("Vnum %d not found for do_build!", OBJ_VNUM_FIRE);
		ch->printlnf( "Vnum %d not found for do_build!, please report to the admin.", OBJ_VNUM_FIRE );
		return;
	}

	fire = create_object( get_obj_index( OBJ_VNUM_FIRE ));
	fire->timer = number_fuzzy(4);
	obj_to_room( fire, ch->in_room );
	act("$n strikes flint and steel a few times and starts a small fire.",ch,NULL,NULL,TO_ROOM);
	ch->println("After trying for a bit, the fire catches and grows bright.");
	check_improve( ch,gsn_build,true,2);
	WAIT_STATE( ch, skill_table[gsn_build].beats );
	return;
}

/***************************************************************************/
void build_raft( char_data *ch, int chance )
{
	OBJ_DATA	*axe = get_eq_char( ch, WEAR_WIELD );
	OBJ_DATA	*raft;

	if ( ch->in_room->sector_type != SECT_FOREST )
	{
		ch->println("You must be in a forest to gather the necessary materials.");
		return;
	}
	
	if ( !axe )
	{
		ch->println("You must wield an axe to build a raft.");
		return;
	}
	
	if ( axe->value[0] != WEAPON_AXE )
	{
		ch->println("You must wield an axe to build a raft.");
		return;
	}
	
	if ( chance < number_range( 0, 105 ))
	{
		ch->println("You work hard at building something that looks `rnothing`x like a raft.");
		WAIT_STATE( ch, skill_table[gsn_build].beats );
		check_improve( ch, gsn_build, false, 1 );
		return;
	}

	if ( get_obj_index(OBJ_VNUM_RAFT) == NULL )
	{
		bugf("Vnum %d not found for do_build!", OBJ_VNUM_RAFT);
		ch->printlnf( "Vnum %d not found for do_build!, please report to the admin.", OBJ_VNUM_RAFT );
		return;
	}

	raft = create_object( get_obj_index( OBJ_VNUM_RAFT ));
	raft->timer = 96;
	obj_to_room( raft, ch->in_room );
	act("$n chops some logs and busies $mself building a raft.",ch,NULL,NULL,TO_ROOM);
	ch->println("You build a raft that you feel can keep you afloat.");
	check_improve( ch,gsn_build,true,2);
	WAIT_STATE( ch, skill_table[gsn_build].beats * 3/2 );
	return;
}	

/***************************************************************************/
void build_staff( char_data *ch, int chance )
{
	OBJ_DATA	*staff;
	int			vlevel, random, numdie, dietype;

	if ( ch->in_room->sector_type != SECT_FOREST )
	{
		ch->println( "You must be in a forest to gather the necessary materials." );
		return;
	}
	
	if ( chance < number_range( 0, 105 ))
	{
		ch->println( "You try to fashion a staff but it doesn't seem very sturdy." );
		ch->println( "You toss out the useless piece of wood in frustration." );
		WAIT_STATE( ch, skill_table[gsn_build].beats );
		check_improve( ch, gsn_build, false, 1 );
		return;
	}

	if ( get_obj_index(OBJ_VNUM_STAFF) == NULL )
	{
		bugf("Vnum %d not found for do_build!", OBJ_VNUM_STAFF);
		ch->printlnf( "Vnum %d not found for do_build!, please report to the admin.", OBJ_VNUM_STAFF );
		return;
	}

	staff = create_object( get_obj_index( OBJ_VNUM_STAFF ));

	vlevel	   = UMIN( ch->level, 10 );
	// first try to get a random damage based on level
	random  = number_range( 0, 7 );
	numdie  = weapon_balance_lookup( vlevel, random, 0 );
	dietype = weapon_balance_lookup( vlevel, random, 1 );	

	// damage lookup failed, use polarmost damage class,
	if ( numdie == 0 ||  dietype == 0 )
	{
		numdie  = weapon_balance_lookup( vlevel, 0, 0 );
		dietype = weapon_balance_lookup( vlevel, 0, 1 );
	}

	staff->value[1]	= numdie;
	staff->value[2]	= dietype;
	staff->level	= vlevel;

	obj_to_room( staff, ch->in_room );
	act("$n fashions a usable staff.",ch,NULL,NULL,TO_ROOM);
	ch->println( "You fashion a staff out of a nice sturdy branch." );
	check_improve( ch,gsn_build,true,2);
	WAIT_STATE( ch, skill_table[gsn_build].beats * 3/2 );
	return;
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

