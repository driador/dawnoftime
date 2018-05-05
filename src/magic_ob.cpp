/**************************************************************************/
// magic_ob.cpp - spells/skills from Oblivion source
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

#include "magic.h"

// Prototypes
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_bug);
bool check_cold_shield( int level, char_data *ch, char_data *victim);
bool check_fire_shield( int level, char_data *ch, char_data *victim);
bool is_aff_fire_shield( char_data *ch, char_data *victim );
bool is_aff_cold_shield( char_data *ch, char_data *victim );
bool check_skin( char_data *ch, char_data *victim);

/**************************************************************************/
// by Airius WWW 
void send_hue_mess(char *clmess, char *clcode, char_data *ch, char_data *victim)
{
	char buf[MSL];
	char_data	*vch;
	char_data   *vch_next;
	
	// to character
	victim->printlnf( "%sA %s hue strikes you!`x", clcode, clmess);
	
	// to room
	sprintf(buf, "%sA %s hue strikes %s!`x", clcode,
		clmess, victim->short_descr);
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
			if ( vch != victim )
			{
				vch->printlnf( "%s", buf );
			}
			continue;
		}
	}
	
}
/**************************************************************************/
// by Airius WWW 
void strike_with_hue( int sn, int level, char_data *ch, char_data *victim)
{
	int dtoss;
	ROOM_INDEX_DATA *pRoomIndex;
	AFFECT_DATA af;
	
	dtoss=dice(1,7);
	switch (dtoss)
	{
	case 1 :
		send_hue_mess("red", "`r", ch, victim);
		damage_spell(ch, victim, saves_spell( level, victim, DAM_LIGHT) ?
			50 : 25, sn, DAM_LIGHT, true);
		break;
	case 2 :
		send_hue_mess("orange", "`R", ch, victim);
		damage_spell(ch, victim, saves_spell( level, victim, DAM_ENERGY) ? 
			100 : 50, sn, DAM_ENERGY, true);
		break;
	case 3 :
		send_hue_mess("yellow", "`Y", ch, victim);
		damage_spell(ch, victim, saves_spell( level, victim, DAM_FIRE) ? 
			150 : 75, sn, DAM_FIRE, true);
		break;
	case 4 :
		send_hue_mess("green", "`g", ch, victim);
		if(!saves_spell(level, victim, DAM_POISON))
		{
			af.where     = WHERE_AFFECTS;
			af.type      = sn;
			af.level     = level;
			af.duration  = level;
			af.location  = APPLY_ST;
			af.modifier  = -4;
			af.bitvector = AFF_POISON;
			affect_join( victim, &af );
			victim->println("You feel very sick.");
			act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
		}
		else
			damage_spell(ch, victim, 20, sn, DAM_POISON, true);
		break;
	case 5 :
		send_hue_mess("blue", "`b", ch, victim);
		if ( victim->in_room == NULL
			|| IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
			|| ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
			|| ( !IS_NPC(ch) && victim->fighting != NULL )
			|| ( victim != ch
			&& ( saves_spell( level - 5, victim,DAM_OTHER))))
		{
			break;
		}
		
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
		send_hue_mess("indigo", "`M", ch, victim);
		if (saves_spell(level,victim,DAM_OTHER)
			||  IS_SET(victim->imm_flags,IMM_MAGIC))
		{
			if (victim != ch)
				ch->println("Nothing seemed to happen.");
			victim->println("You feel momentarily lethargic.");
			break;
		}
		
		if (IS_AFFECTED(victim,AFF_HASTE))
		{
			if (!check_dispel(level,victim, gsn_haste ))
			{
				if (victim!= ch){
					ch->println("Spell failed.");
				}
				return;
			}			
			act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
			return;
		}		
		
		af.where     = WHERE_AFFECTS;
		af.type      = sn;
		af.level     = level;
		af.duration  = level/2;
		af.location  = APPLY_QU;
		af.modifier  = -1 - (level/5);
		af.bitvector = AFF_SLOW;
		affect_join( victim, &af );
		victim->println("You feel yourself slowing d o w n...");
		act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
		break;
	case 7 :
		send_hue_mess("violet", "`m", ch, victim);
		damage_spell(ch, victim, saves_spell( level, victim, DAM_NEGATIVE) ? 200 : 150, sn, DAM_NEGATIVE, true);
		break;
	}
	return;
}
/**************************************************************************/
//  By Wynn
SPRESULT spell_animal_essence( int sn, int level, char_data *ch, void *vo, int )
{
	char_data 	*victim = (char_data *) vo;
	AFFECT_DATA	af;
	
	if ( !IS_SET( victim->affected_by, AFF_FLYING ))
	{
		if ( !IS_SET( ch->in_room->room_flags, ROOM_NOFLY	)){
			af.where		= WHERE_AFFECTS;
			af.type			= sn;
			af.level		= level;
			af.duration		= level/2;
			af.location		= APPLY_NONE;
			af.modifier		= 0;
			af.bitvector	= AFF_FLYING;
			affect_to_char( victim, &af );
			victim->println( "Your feet rise off the ground." );
			act( "$n's feet rise off the ground.", ch, NULL, victim, TO_NOTVICT );
		}
	}
	else
		victim->println("You already can fly.");

	if ( !IS_SET( victim->affected_by, AFF_DETECT_HIDDEN ))
	{
		af.where 	 = WHERE_AFFECTS;
		af.type		 = sn;
		af.level	 = level;
		af.duration  = level/2;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_DETECT_HIDDEN;
		affect_to_char( victim, &af );
		victim->println("Your awareness improves.");
	}
	else
		victim->println("You are as alert as you're ever going to be.");
	
	if ( !IS_SET( victim->affected_by, AFF_DETECT_INVIS ))
	{
		af.where		= WHERE_AFFECTS;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/2;
		af.location		= APPLY_NONE;
		af.modifier		= 0;
		af.bitvector	= AFF_DETECT_INVIS;
		affect_to_char( victim, &af );
		victim->println("Your eyes tingle.");
	}
	else
		victim->println("You can already see the invisible.");

	if ( !is_affected( victim, gsn_augment_hearing ))
	{
		af.where		= WHERE_AFFECTS;
		af.type			= gsn_augment_hearing;
		af.level		= level;
		af.duration		= level/5;
		af.location		= APPLY_NONE;
		af.modifier		= 0;
		af.bitvector	= 0;
		affect_to_char( victim, &af );
		victim->println("Your hearing is greatly augmented.");
	}
	else
		victim->println("Your hearing is already augmented.");

	ch->println("Ok.");
	return FULL_MANA;
}

/**************************************************************************/
// by Airius WWW + Jarren Update
SPRESULT spell_barkskin( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if(check_skin( ch, victim ))
		return HALF_MANA;
	
	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -40;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n's skin turns to bark.", victim, NULL, NULL, TO_ROOM );
	victim->println("Your skin turns to bark.");
	af.where	 = WHERE_VULN;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = VULN_FIRE;
	affect_to_char( victim, &af );
	
	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW + Jarren Update
SPRESULT spell_chill_shield( int, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if ( is_aff_cold_shield( ch, victim ))
		return HALF_MANA;
	
	else if ( check_fire_shield(level, ch, victim))
		return FULL_MANA;

	af.where     = WHERE_AFFECTS;
	af.type      = gsn_chill_shield;
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
	victim->println("You are surrounded by an icy aura.");
	act("$n is surrounded by an icy aura.",victim,NULL,NULL,TO_ROOM);
	if ( ch != victim )
		ch->println("Ok.");
	return FULL_MANA;
}

/**************************************************************************/
// by Airius WWW
SPRESULT spell_cone_cold( int sn, int level, char_data *ch, void *, int  )
{
	char_data	*vch;
	char_data	*vch_next;
	int			dam;
	AFFECT_DATA af;

	dam = dice( level , 3 )+50;

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_ST;
	af.modifier		= -level/3;
	af.bitvector	= 0;

	ch->println("You send forth a cone of cold!");
	act( "$n places out his hands and blasts forth a cone of cold!",ch, NULL, NULL, TO_ROOM );

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
			&& ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch )))
			{
		        damage_spell( ch, vch, saves_spell( level, vch, DAMTYPE(sn)) ? dam /2 : dam, sn, DAMTYPE(sn), true );
				if(!saves_spell(level,vch, DAMTYPE(sn)))
				{
					affect_join( vch, &af );
					vch->printf( "The cold seeps into your bones." );
				}
			}
			continue;
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW
SPRESULT spell_create_buffet( int , int level, char_data *ch, void *,int )
{
	OBJ_DATA *mushroom=NULL;
	int counter=0;
	
	for(counter=0; counter< number_fuzzy(level/5)+1; counter++)
	{
		mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ));
		mushroom->value[0] = number_fuzzy(level) / 2;
		mushroom->value[1] = number_fuzzy(level);
		mushroom->timer = number_fuzzy((number_fuzzy(number_fuzzy((ch->level))+10)));
		obj_to_room( mushroom, ch->in_room );
	}
	if(counter){
		act( FORMATF("$p suddenly appears (x%d).", counter), ch, mushroom, NULL, TO_ROOM );
	}
	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW
SPRESULT spell_drain_blade( int sn, int level, char_data *ch, void *vo, int  )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;
	
	obj=(OBJ_DATA *) vo;
	
	
	if(IS_GOOD(ch))
	{
		ch->println("You are far too nice to use such evil magic.");
		return NO_MANA;
	}
	
	if(!IS_EVIL(ch))
	{
		ch->println("You are not quite wicked enough to do that.");
		return NO_MANA;
	}
	if(obj->item_type != ITEM_WEAPON)
	{
		ch->println("You can only target sharp weapons.");
		return NO_MANA;
	}
	else
	{
		if( obj->value[0] == WEAPON_WHIP
			||  obj->value[0] == WEAPON_EXOTIC )
		{
			act( "Your spell had no effect on $p.", ch, obj, NULL, TO_CHAR );
			return HALF_MANA;
		}
		else
		{
			if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))
			{
				ch->println("That weapon is already quite evil.");
				return HALF_MANA;
			}
			if(IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
			{
				ch->println("That weapon is too holy to be touched by your majiks.");
				return FULL_MANA;
			}
			if(!IS_OBJ_STAT(obj,OBJEXTRA_EVIL))
				SET_BIT(obj->extra_flags, OBJEXTRA_EVIL);
			if(!IS_OBJ_STAT(obj,OBJEXTRA_ANTI_GOOD))
				SET_BIT(obj->extra_flags, OBJEXTRA_ANTI_GOOD);
			if(!IS_OBJ_STAT(obj,OBJEXTRA_ANTI_NEUTRAL))
				SET_BIT(obj->extra_flags, OBJEXTRA_ANTI_NEUTRAL);
			
			af.where    = WHERE_WEAPON;
			af.type     = sn;
			af.level    = level/2;
			af.duration = level/2;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector= WEAPON_VAMPIRIC;
			affect_to_obj(obj, &af);
			
			act("$p becomes dark and evil.",ch,obj,NULL,TO_ALL);
			return FULL_MANA;
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW
SPRESULT spell_empower_blade( int sn, int level, char_data *ch, void *vo, int  )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;
	
	obj=(OBJ_DATA *) vo;
	
	if(obj->item_type != ITEM_WEAPON)
	{
		ch->println("You can only target sharp weapons.");
		return NO_MANA;
	}
	else
	{
		if(    obj->value[0] == WEAPON_WHIP
			|| obj->value[0] == WEAPON_EXOTIC ) {
			act( "Your spell had no effect on $p.", ch, obj, NULL, TO_CHAR );
			return HALF_MANA;
		}
		else
		{
			if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
			{
				ch->println("That weapon is already imbued with power.");
				return HALF_MANA;
			}
			
			af.where    = WHERE_WEAPON;
			af.type     = sn;
			af.level    = level/2;
			af.duration = level;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector= WEAPON_SHOCKING;
			affect_to_obj(obj, &af);
			
			act("$p sparks with electricity.",ch,obj,NULL,TO_ALL);
			return FULL_MANA;
		}
	}
	return FULL_MANA;
}


/**************************************************************************/
// Airius WWW
SPRESULT spell_flame_blade( int sn, int level, char_data *ch, void *vo, int  )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;
	
	obj=(OBJ_DATA *) vo;
	
	if(obj->item_type != ITEM_WEAPON)
	{
		ch->println("You can only target sharp weapons.");
		return NO_MANA;
	}
	else
	{
		if( obj->value[0] == WEAPON_WHIP
			||  obj->value[0] == WEAPON_EXOTIC ) {
			act( "Your spell had no effect on $p.", ch, obj, NULL, TO_CHAR );
			return HALF_MANA;
		}
		else
		{
			if(IS_WEAPON_STAT(obj,WEAPON_FLAMING))
			{
				ch->println("That weapon is already flaming.");
				return HALF_MANA;
			}
			if(IS_WEAPON_STAT(obj,WEAPON_FROST))
			{
				ch->println("That weapon is too cold to accept the magic.");
				return FULL_MANA;
			}
			
			af.where    = WHERE_WEAPON;
			af.type     = sn;
			af.level    = level/2;
			af.duration = level;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector= WEAPON_FLAMING;
			affect_to_obj(obj, &af);
			
			act("$p gets a fiery aura.",ch,obj,NULL,TO_ALL);
			return FULL_MANA;
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW + Jarren Update
SPRESULT spell_fire_shield( int sn, int level, char_data *ch, void *vo,int  )
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
	victim->println("You are surrounded by a fire shield.");
	act("$n is surrounded by a fire shield.",victim,NULL,NULL,TO_ROOM);
	if ( ch != victim )
		ch->println("Ok.");
	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW
SPRESULT spell_frost_blade( int sn, int level, char_data *ch, void *vo, int  )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;
	
	obj=(OBJ_DATA *) vo;
	
	if(obj->item_type != ITEM_WEAPON)
	{
		ch->println("You can only target sharp weapons.");
		return NO_MANA;
	}
	else
	{
		if(    obj->value[0] == WEAPON_WHIP
			|| obj->value[0] == WEAPON_EXOTIC ) {
			act( "Your spell had no effect on $p.", ch, obj, NULL, TO_CHAR );
			return HALF_MANA;
		}
		else
		{
			if(IS_WEAPON_STAT(obj,WEAPON_FROST))
			{
				ch->println("That weapon is already wickedly cold.");
				return HALF_MANA;
			}
			if(IS_WEAPON_STAT(obj,WEAPON_FLAMING))
			{
				ch->println("That weapon is too hot to accept the magic.");
				return FULL_MANA;
			}
			
			af.where    = WHERE_WEAPON;
			af.type     = sn;
			af.level    = level/2;
			af.duration = level;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector= WEAPON_FROST;
			affect_to_obj(obj, &af);
			
			act("$p grows wickedly cold.",ch,obj,NULL,TO_ALL);
			return FULL_MANA;
		}
	}
	return FULL_MANA;
}


/**************************************************************************/
// by Airius WWW 
SPRESULT spell_holy_aura( int sn, int level, char_data *ch, void *vo, int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if ( !IS_IMMORTAL(ch) && (!IS_GOOD(victim) || !IS_GOOD(ch)))
	{
		if(ch == victim)
		{
			ch->println("You are not holy enough to cast this spell.");
			return NO_MANA;
		}

		ch->println("They are too unrighteous!");
		return HALF_MANA;
	}
	
	if(!is_affected(victim,sn))
	{
		af.where      = WHERE_AFFECTS;
		af.type		  = sn;
		af.level	  = level;
		af.duration	  = level/5;
		af.location	  = APPLY_AC;
		af.bitvector  = 0;
		af.modifier	  = -level;
		affect_to_char( victim, &af);
		af.where	  = WHERE_RESIST;
		af.modifier	  = 0;
		af.location	  = APPLY_NONE;
		af.bitvector  = RES_NEGATIVE;
		affect_to_char( victim, &af);
		victim->println("You are surrounded by a golden aura.");
		act("$n is surrounded with a gold aura.", ch, NULL, victim, TO_NOTVICT);
	}
	else
	{
		if(ch == victim)
			ch->println("You are already protected by divine magic.");
		else
			ch->println("They are already protected.");
		return HALF_MANA;
	}
	
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_ice_storm( int sn, int level, char_data *ch, void *, int  )
{
	char_data	*vch;
	char_data	*vch_next;
	int			dam;
	AFFECT_DATA af;
	
	dam = 150;
	
	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level/10;
	af.location  = APPLY_QU;
	af.modifier  = -1 - (level/5);
	af.bitvector = AFF_SLOW;
	
	ch->println("You conjure up an ice storm!");
	act( "$n raises $s hands and an ice storm appears from nowhere!",ch, NULL, NULL, TO_ROOM );
	
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
				&& ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) ) )
			{
				damage_spell( ch, vch, saves_spell( level, vch, DAMTYPE(sn)) ? dam /2 : dam, sn, DAMTYPE(sn), true );
				if(!saves_spell(level,vch, DAMTYPE(sn)))
				{
					affect_join( vch, &af );
					vch->println("You get covered in ice and have a hard time moving.");
					act("$n is covered in ice.",vch,NULL,NULL,TO_ROOM);
				}
			}
			continue;
		}
	}
	return FULL_MANA;
}


/**************************************************************************/
// by Airius  WWW 
SPRESULT spell_illusions_grandeur( int sn, int level, char_data *ch, void *vo, int )
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
// by Airius 
SPRESULT spell_improved_phantasm( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	int dam;
	
	dam = dice(level, 9);
	if(saves_spell( level, victim, DAMTYPE(sn)))
		dam = dam/3;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn), true);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_induce_sleep( int, int, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	
	if(ch==victim)
	{
		ch->println("You put yourself to sleep.");
		ch->is_trying_sleep=true;
		ch->position=POS_SLEEPING;
		act( "$n goes quietly to sleep.", ch, NULL, NULL, TO_ROOM );
	}
	
	if(IS_NPC(victim))
	{
		ch->println("You cannot put to sleep unwilling creatures.");
		return FULL_MANA;
	}
	
	if(victim->is_trying_sleep)
	{
		victim->println("You drift off into dreamland.");
		victim->position=POS_SLEEPING;
		act( "$n goes quietly to sleep.", victim, NULL, NULL, TO_ROOM );
		return FULL_MANA;
	}
	
	ch->println("You cannot put to sleep others so unwilling.");
	
	return FULL_MANA;
}

/**************************************************************************/
// by Airius
SPRESULT spell_magic_resistance( int sn, int level, char_data *, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if(!is_affected(victim,sn))
	{
		af.where		= WHERE_RESIST;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/10+1;
		af.location 	= APPLY_NONE;
		af.modifier 	= 0;
		af.bitvector	= RES_MAGIC;
		affect_to_char(victim, &af);
		victim->println("You are protected from magic.");
	}
	else
	{
		victim->println("You are already protected from magic.");
		return HALF_MANA;
	}
	return FULL_MANA;
}

/**************************************************************************/
// by Airius
SPRESULT spell_phantasmal_force( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	int dam;
	
	dam = dice(level, 4);
	if(saves_spell( level, victim, DAMTYPE(sn)))
		dam = 0;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn), true);
	return FULL_MANA;
}

/**************************************************************************/
// by Airius WWW 
SPRESULT spell_prismatic_spray( int sn, int level, char_data *ch, void *, int )
{
	char_data	*vch;
	char_data   *vch_next;
	int number_hits;
	int i;
	
	ch->println("You put out your hands and send forth a dazzling pristmatic spray!");
	act( "$n raises $s hands and sends out a dazzling prismatic spray!",ch, NULL, NULL, TO_ROOM );
	
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
			if ( vch != ch && ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) ) )
			{
				if(dice(1,8)==8)
					number_hits=2;	else number_hits=1;
				if(dice(1,8)==8)
					number_hits+=2; else number_hits+=1;
				for(i=1; i<=number_hits; i+=1)
				{
					strike_with_hue(sn,level,ch,vch);
				}
			}
			continue;
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
// by Airius 
SPRESULT spell_protection_fire( int sn, int level, char_data *, void *vo, int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if(!is_affected(victim,sn))
	{
		af.where		= WHERE_RESIST;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/2;
		af.location 	= APPLY_NONE;
		af.modifier 	= 0;
		af.bitvector	= RES_FIRE;
		affect_to_char(victim, &af);
		victim->println("You are protected from flames.");
	}
	else
	{
		victim->println("You are already protected from fire.");
		return HALF_MANA;
	}
	return FULL_MANA;
}

/**************************************************************************/
// by Airius
SPRESULT spell_protection_cold( int sn, int level, char_data *, void *vo, int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if(!is_affected(victim,sn))
	{
		af.where		= WHERE_RESIST;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/2;
		af.location 	= APPLY_NONE;
		af.modifier 	= 0;
		af.bitvector	= RES_COLD;
		affect_to_char(victim, &af);
		victim->println("You are protected from cold.");
	}
	else
	{
		victim->println("You are already protected from cold.");
		return HALF_MANA;
	}
	return FULL_MANA;
}

/**************************************************************************/
// by Airius 
SPRESULT spell_protection_lightning( int sn, int level, char_data *, void *vo, int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if(!is_affected(victim,sn))
	{
		af.where		= WHERE_RESIST;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/2;
		af.location 	= APPLY_NONE;
		af.modifier 	= 0;
		af.bitvector	= RES_LIGHTNING;
		affect_to_char(victim, &af);
		victim->println("You are protected from lightning.");
	}
	else
	{
		victim->println("You are already protected from lightning.");
		return HALF_MANA;
	}
	return FULL_MANA;
}

/**************************************************************************/
// by Airius 
SPRESULT spell_poison_immunity( int sn, int level, char_data *, void *vo, int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if(!is_affected(victim,sn))
	{
		af.where		= WHERE_IMMUNE;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/4+10;
		af.location 	= APPLY_NONE;
		af.modifier 	= 0;
		af.bitvector	= IMM_POISON;
		affect_to_char(victim, &af);
		victim->println("You are immune to all forms of poison.");
	}
	else
	{
		victim->println("You are already immune to poison.");
		return HALF_MANA;
	}
	return FULL_MANA;
}


/**************************************************************************/
// by Wynn
SPRESULT spell_poison_rain( int sn, int level, char_data *ch, void *, int  )
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
	af.modifier  = -4;
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
// by Wynn
SPRESULT spell_rage ( int sn, int level, char_data *ch, void *vo, int )
{
	char_data	*victim = (char_data *)	vo;
	AFFECT_DATA	af;
	
	if ( is_affected( victim, sn ))
	{
		if (victim == ch)
			ch->println("You are already enraged.");
		else
			act("$N is already enraged.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}
	
	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level	 	= level;
	af.duration		= level / 4;
	af.location		= APPLY_DAMROLL;
	af.modifier		= level / 5;
	af.bitvector	= 0;
	affect_to_char( victim, &af );
	
	af.location = APPLY_AC;
	af.modifier = level*3;
	affect_to_char( victim, &af );
	
	af.location = APPLY_HIT;
	af.modifier = level/2;
	affect_to_char( victim, &af );
	victim->hit = UMIN( victim->hit + level*2, victim->max_hit );
	update_pos( victim );
	
	if (ch != victim ){
		act("$N appears to be filled with rage.",ch,NULL,victim,TO_CHAR);
	}
	victim->println("You feel enraged!");
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_regeneration( int sn, int level, char_data *ch, void *vo, int )
{
	char_data 	*victim = (char_data *) vo;
	AFFECT_DATA	af;

	if (IS_AFFECTED( victim, AFF_REGENERATION ))
	{
		if(ch==victim)
			ch->println("You are already healing at an accelerated rate.");
		else
			ch->println("They are already regenerating.");
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level	 	= level;
	af.duration		= level / 4;
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= AFF_REGENERATION;
	affect_to_char( victim, &af );

	if(victim==ch)
		ch->println("You feel more vibrant!!!");
	else
	{
		victim->println("You feel more vibrant!!!");
		ch->println("Ok.");
	}

	return FULL_MANA;
}

/**************************************************************************/
// contributed by Wynn, Modified by Airius
SPRESULT spell_rejuvinate( int, int level, char_data *ch, void *vo, int  )
{
	char_data 	*victim = (char_data *) vo;

	victim->hit = victim->max_hit;
	update_pos( victim );
	victim->println("You feel rejuvinated...");
	if ( ch != victim )
		ch->println("Ok.");
	if ( is_affected( victim, gsn_curse ) )
	{
		affect_strip( victim, gsn_curse );
		victim->println("A warm feeling fills your body.");
		act( "$N looks better.", ch, NULL, victim, TO_NOTVICT );
	}

	if ( is_affected( victim, gsn_poison ) )
	{
		affect_strip( victim, gsn_poison );
		victim->println("You feel better.");
	}
	// boost movement points
	victim->move = UMIN( victim->move + level/2, victim->max_move );
	// restore tiredness
	if(!IS_NPC(victim) && victim->pcdata->tired!=-1)
	{
		victim->pcdata->tired-=level/10+2;
		if(victim->pcdata->tired<0)
			victim->pcdata->tired=0;
		victim->println("You feel less tired.");
	}
	return FULL_MANA;
}

/**************************************************************************/
// by Airius
SPRESULT spell_resist_poison( int sn, int level, char_data *, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if(!is_affected(victim,sn))
	{
		af.where		= WHERE_RESIST;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/4+10;
		af.location 	= APPLY_NONE;
		af.modifier 	= 0;
		af.bitvector	= RES_POISON;
		affect_to_char(victim, &af);
		victim->println("You are protected from all forms of poison.");
	}
	else
	{
		victim->println("You are already protected from poison.");
		return HALF_MANA;
	}

	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW
SPRESULT spell_rune_edge( int sn, int level, char_data *ch, void *vo, int )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	obj=(OBJ_DATA *) vo;

	if(obj->item_type != ITEM_WEAPON)
	{
		ch->println("You can only target sharp weapons.");
		return NO_MANA;
	}
	else
	{
		if(obj->value[0]!=WEAPON_SWORD && obj->value[0]!=WEAPON_DAGGER
			&& obj->value[0]!=WEAPON_STAFF && obj->value[0]!=WEAPON_AXE
			&& obj->value[0]!=WEAPON_POLEARM && obj->value[0]!=WEAPON_EXOTIC)
			return HALF_MANA;
		else
		{
			if(IS_WEAPON_STAT(obj,WEAPON_SHARP)||IS_WEAPON_STAT(obj,WEAPON_VORPAL))
			{
				ch->println("That weapon is already quite sharp.");
				return HALF_MANA;
			}
			
			af.where    = WHERE_WEAPON;
			af.type     = sn;
			af.level    = level/2;
			af.duration = level;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector= WEAPON_SHARP;
			affect_to_obj(obj, &af);
			act("$p looks newly honed.",ch,obj,NULL,TO_ALL);
			return FULL_MANA;
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW
SPRESULT spell_runic_blade( int sn, int level, char_data *ch, void *vo, int )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	obj=(OBJ_DATA *) vo;

	if(obj->item_type != ITEM_WEAPON)
	{
		ch->println("You can only target sharp weapons.");
		return NO_MANA;
	}
	else
	{
		if(    obj->value[0] == WEAPON_WHIP
			|| obj->value[0] == WEAPON_EXOTIC ) {
			act( "Your spell had no effect on $p.", ch, obj, NULL, TO_CHAR );
			return HALF_MANA;
		}
		else
		{
		  if(IS_WEAPON_STAT(obj,WEAPON_VORPAL) || IS_WEAPON_STAT(obj,WEAPON_SHARP))
		  {
			ch->println("That weapon is already magically sharp.");
			return HALF_MANA;
		  }

		  af.where    = WHERE_WEAPON;
		  af.type     = sn;
		  af.level    = level/2;
		  af.duration = level;
		  af.location = APPLY_NONE;
		  af.modifier = 0;
		  af.bitvector= WEAPON_VORPAL;
		  affect_to_obj(obj, &af);

		  act("$p gleams with magical strength.",ch,obj,NULL,TO_ALL);
		  return FULL_MANA;
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW
SPRESULT spell_true_sight( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if( !is_affected( victim, sn ))
	{
		af.where		= WHERE_IMMUNE;
		af.type			= sn;
		af.level		= level;
		af.duration		= level/2;
		af.location 	= APPLY_NONE;
		af.modifier		= 0;
		af.bitvector	= IMM_ILLUSION;
		affect_to_char(victim, &af);
		victim->println("You see more clearly now.");

		if ( !IS_SET( victim->affected_by, AFF_DETECT_HIDDEN ))
		{
			af.where		= WHERE_AFFECTS;
			af.type			= sn;
			af.level		= level;
			af.duration		= level/2;
			af.location		= APPLY_NONE;
			af.modifier		= 0;
			af.bitvector	= AFF_DETECT_HIDDEN;
			affect_to_char( victim, &af );
			victim->println("Your awareness improves.");
		}
		else
			victim->println("You can already detect hidden.");

		if ( !IS_SET( victim->affected_by, AFF_DETECT_INVIS ))
		{
			af.where		= WHERE_AFFECTS;
			af.type			= sn;
			af.level		= level;
			af.duration		= level/2;
			af.location		= APPLY_NONE;
			af.modifier		= 0;
			af.bitvector	= AFF_DETECT_INVIS;
			affect_to_char( victim, &af );
			victim->println("Your eyes tingle.");
		}
		else
			victim->println("You can already see the invisible.");
	}
	else
	{
		if(victim!=ch)
			ch->println("They can already see clearly.");
		else
			ch->println("You can already see clearly.");
		return HALF_MANA;
	}

	ch->println("Ok.");

	return FULL_MANA;
}

/**************************************************************************/
// Airius WWW
SPRESULT spell_vampiric_touch( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	int dam;
	int temp_hps;
	
	if(ch==victim)
	{
		ch->println("You cannot target yourself!");
		return NO_MANA;
	}

	if ( IS_ICIMMORTAL(victim)){
		act( "$n doesn't appear to be even slightly affected by your magic.", victim, NULL, NULL, TO_ROOM );
		return FULL_MANA;
	}
	
	dam=dice(level, 7)/abs(4+ch->alliance);
	
	if(saves_spell(level, victim, DAMTYPE(sn))){
		dam /= 2;
	}
	temp_hps=victim->hit;
    damage_spell(ch, victim, dam, sn, DAMTYPE(sn), true);
	if(victim->hit<-10) victim->hit=-10;
	
	//gives caster victims hps
	if(victim){	// at most you can get 40 hp per cast
		ch->hit+=UMIN(number_range(25,40), temp_hps-victim->hit);
	}
	return FULL_MANA;
}

/**************************************************************************/
// by Airius 
SPRESULT spell_wizard_eye( int , int level, char_data *ch, void *vo, int  )
{
	char_data	*victim = (char_data *) vo;
	char		buf[MSL];
	int			chance, random, i;
	bool		saved = false;
    char_data	*pMob;

	if (( victim = get_char_world( ch, target_name )) == NULL
	 ||   victim == ch
	 ||   victim->in_room == NULL
	 ||	  IS_OOC(victim)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_ANTIMAGIC )
	 ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_NOSCRY)
	 ||   IS_SET(victim->in_room->area->area_flags, AREA_NOSCRY)
	 ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL )) { // NOT trust
		ch->println("You failed.");
		return FULL_MANA;
	}

	if(	IS_SET(victim->imm_flags, IMM_SCRY)	 
		|| (IS_SET(victim->res_flags, RES_SCRY) && number_range(1,3)!=1) )
	{
		ch->println("You failed.");
		return FULL_MANA;
	}

	if ( is_affected( victim, gsn_detect_scry )) {
		chance = 50 - (( ch->level - victim->level ) * 3 );
		random = number_range( 1, 100);
		if ( chance >= random )
			victim->println("You feel someone watching you.");
		if (( chance /= 5 ) >= random )
			act( "$n is observing you.", ch, NULL, victim, TO_VICT );
	}

	if ( victim->mirage )
	{
		for ( i=0; i < victim->level; i += 20 )
		{
			if ( saves_spell( level, victim, DAM_ILLUSION)) saved = true;
		}
	}

	if (saved)
	{
		char	buf2[MSL];

		sprintf( buf, "%d look", victim->mirage );

		pMob = create_mobile( limbo_mob_index_data, 0 );

		sprintf(buf2, "%s is standing here.", victim->short_descr );
		replace_string(pMob->long_descr, buf2 );
		pMob->level = 1;
		pMob->max_hit = 1;
		pMob->hit = 1;
		pMob->timer = 1;
	    char_to_room( pMob, get_room_index( victim->mirage ));
		do_at( ch, buf );
		extract_char( pMob, true );
	}
	else
	{
		sprintf(buf, "'%s' look", target_name);
		do_at(ch, buf);
	}

	return FULL_MANA;
}

/**************************************************************************/
// By Bonhomme
SPRESULT spell_chaos_lace( int sn, int level, char_data *ch, void *vo, int )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    int result, fail;

    if ( !CAN_WEAR( obj, OBJWEAR_TORSO) &&
         !CAN_WEAR( obj, OBJWEAR_HEAD) &&
         !CAN_WEAR( obj, OBJWEAR_LEGS) &&
         !CAN_WEAR( obj, OBJWEAR_WIELD) )
      {
        ch->println("Chaos lace works only on torso armor, leggings, helmets, and weapons.");
        return NO_MANA;
      }

// Some objects are impossible to lace

	if ( IS_SET( obj->extra2_flags, OBJEXTRA2_NOCHAOS ))
	{
		act("$p weirds out completely!",ch,obj,NULL,TO_CHAR);
		act("$p weirds out completely!",ch,obj,NULL,TO_ROOM);
		extract_obj(obj);
		return FULL_MANA;
	}

    if (obj->wear_loc != -1) {
        ch->println("The item must be carried to be enchanted.");
        return NO_MANA;
    }
// Only one weird enchant per item 

    if (obj->chaos || IS_SET(obj->extra_flags,OBJEXTRA_CHAOS)) {
        ch->println("That item is already as wierd as it can get!");
        return HALF_MANA;
    }

// Items with attitudes don't like weirdness, and resist it 

    if ( (IS_OBJ_STAT(obj,OBJEXTRA_ANTI_EVIL)) ||
         (IS_OBJ_STAT(obj,OBJEXTRA_ANTI_GOOD)) ||
         (IS_OBJ_STAT(obj,OBJEXTRA_ANTI_NEUTRAL)) ) {
       fail = 50;
       fail += (level - obj->level);
       result = number_percent();
       if (result < fail) {
            act("$p weirds out completely!",ch,obj,NULL,TO_CHAR);
            act("$p weirds out completely!",ch,obj,NULL,TO_ROOM);
            extract_obj(obj);
            return FULL_MANA;
       }
       REMOVE_BIT(obj->extra_flags,OBJEXTRA_ANTI_GOOD);
       REMOVE_BIT(obj->extra_flags,OBJEXTRA_ANTI_EVIL);
       REMOVE_BIT(obj->extra_flags,OBJEXTRA_ANTI_NEUTRAL);
    }

// weirdness ruins all flags

    obj->no_affects=true;
    for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
       paf_next = paf->next;
       free_affect(paf);
    }
    obj->affected = NULL;
    obj->extra_flags = 0;

    obj->chaos = true;

    act("$p suddenly looks rather weird.",ch,obj,NULL,TO_CHAR);
    act("$p suddenly looks rather weird.",ch,obj,NULL,TO_ROOM);
    SET_BIT(obj->extra_flags,OBJEXTRA_CHAOS);
	SET_BIT( obj->extra_flags, OBJEXTRA_MAGIC );
	obj->timer=UMIN(obj->timer, number_range(600, 15000));

	// Add affects, often bad, depending on what type item this is 
    paf = new_affect();
    paf->where       = WHERE_OBJEXTRA;
    paf->type        = sn;
    paf->level       = ch->level;
    paf->duration    = -1;
    paf->bitvector   = 0;

    if ( CAN_WEAR(obj, OBJWEAR_TORSO) ) {
       if (number_percent() <= 60) {
          paf->location = APPLY_HIT;
          paf->modifier = (((number_percent() % 6) - 3) * 10);
          paf->next     = obj->affected;
          obj->affected = paf;
       }
       else {
          paf->location = APPLY_ST;
          paf->modifier = ((number_percent() % 6) - 3) * 2 ;
          paf->next     = obj->affected;
          obj->affected = paf;
       }
    }
    if ( CAN_WEAR(obj, OBJWEAR_HEAD) ) {
       if (number_percent() <= 60) {
          paf->location = APPLY_MANA;
          paf->modifier = (((number_percent() % 6) - 3) * 10);
          paf->next     = obj->affected;
          obj->affected = paf;
       }
       else if (number_percent() <= 50) {
          paf->location = APPLY_IN;
          paf->modifier = ((number_percent() % 6) - 3) * 2;
          paf->next     = obj->affected;
          obj->affected = paf;
       }
       else {
          paf->location = APPLY_ME;
          paf->modifier = ((number_percent() % 6) - 3) * 2;
          paf->next     = obj->affected;
          obj->affected = paf;
       }
    }
    if ( CAN_WEAR(obj, OBJWEAR_LEGS) ) {
       if (number_percent() <= 60) {
          paf->location = APPLY_MOVE;
          paf->modifier = (((number_percent() % 6) - 3) * 10);
          paf->next     = obj->affected;
          obj->affected = paf;
       }
       else {
          paf->location = APPLY_CO;
          paf->modifier = ((number_percent() % 6) - 3) * 2;
          paf->next     = obj->affected;
          obj->affected = paf;
       }
    }

// weird weapons are weird, and have three different 10% chances;
// they can easily wind up with no affect, though still weird 

    if ( CAN_WEAR(obj, OBJWEAR_WIELD) ) {
       if (number_percent() <= 10) {
          paf->location = APPLY_HIT;
          paf->modifier = (((number_percent() % 6) - 3) * 10);
          paf->next     = obj->affected;
          obj->affected = paf;
       }
       else if (number_percent() <= 10) {
          paf->location = APPLY_ST;
          paf->modifier = ((number_percent() % 6) - 3) * 2;
          paf->next     = obj->affected;
          obj->affected = paf;
       }
       else if (number_percent() <= 10) {
          paf->location = APPLY_QU;
          paf->modifier = ((number_percent() % 6) - 3) * 2;
          paf->next     = obj->affected;
          obj->affected = paf;
       }
    }

    return FULL_MANA;
}

/**************************************************************************/
// By Bonhomme
SPRESULT spell_mithril_glaze( int sn, int level, char_data *ch, void *vo, int )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    OBJ_DATA *mit, *mit_next;
    AFFECT_DATA *paf;
	
	
    if(!CAN_WEAR( obj, OBJWEAR_ABOUT) ){
		ch->println( "You can only apply a mithril glaze to objects which are worn about the body." );
		return NO_MANA;
	}

	if (obj->wear_loc != -1){
		ch->println( "The item must be carried to be glazed." );
        return NO_MANA;
    }
	/*
	* Have to put (at least some) mithril items in manually until
	* someone gets around to fixing obj->pIndexData->material ;)
	*/
    for ( mit = ch->carrying; mit != NULL; mit = mit_next)
    {
        mit_next = mit->next_content;
        if	((mit->wear_loc == -1) &&
			( !strcmp( mit->pIndexData->material, "mithril" )))
		{
			break;
		}
    }
    if (mit == NULL)
	{
		ch->println( "You must have a mithril item to be consumed by this spell." );
        return NO_MANA;
    }
    if (ch->pcdata->tired > 16)
	{
		ch->println( "You are not well-rested enough to cast this spell just now. Have a nap first." );
		return NO_MANA;
    }
	
    act("$p effervesces into a mithril cloud...",ch,mit,NULL,TO_CHAR);
    act("$p effervesces into a mithril cloud...",ch,mit,NULL,TO_ROOM);
    extract_obj(mit);
    if ( number_percent() <= 40 ) {
        act("Which settles on $p and sets it aflame!",ch,obj,NULL,TO_CHAR);
        act("Which settles on $p and sets it aflame!",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
    }else{
        ch->mana = 0;
        ch->pcdata->tired += 60;
        ch->pcdata->condition[COND_THIRST] = 0;
        ch->pcdata->condition[COND_HUNGER] = 0;
        act("Which slowly condenses at your command upon $p.",ch,obj,NULL,TO_CHAR);
        act("Which slowly condenses upon $p.",ch,obj,NULL,TO_ROOM);
    }

	// success, get the template affects on to the object if their is no local affects
	affects_from_template_to_obj(obj); 

    for ( paf = obj->affected; paf; paf = paf->next ) {
        if ( !str_cmp( skill_table[paf->type].name, "mithril glaze" ) ) {
            paf->modifier+= dice(level / 3, 3);
            paf->level = UMIN(LEVEL_HERO, obj->level + 5); 
            obj->level = UMIN(LEVEL_HERO, obj->level + 5);
            if ( ch->mana == 0 ){
				return ALL_MANA;
			}else{
				return FULL_MANA;
			}
        }
    }
    paf = new_affect();
    paf->where       = WHERE_OBJEXTRA;
    paf->type        = sn;
    paf->level       = UMIN(LEVEL_HERO, obj->level + 5);
    paf->duration    = -1;
    paf->bitvector   = 0;
    paf->location    = APPLY_MANA;
    paf->modifier    = dice(level / 3, 3);
    paf->next        = obj->affected;
    obj->level       = UMIN(LEVEL_HERO, obj->level + 5);
    obj->affected    = paf;
	SET_BIT( obj->extra_flags, OBJEXTRA_MAGIC );
	if ( ch->mana == 0 ){
		return ALL_MANA;
	}else{
		return FULL_MANA;
	}
}

/**************************************************************************/
// By Bonhomme 
SPRESULT spell_extension( int sn, int level, char_data *ch, void *vo, int )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf, *afpaf;

    if (obj->wear_loc != -1)
	{
		ch->println("The item must be carried to be extended.");
        return NO_MANA;
    }
    for ( afpaf = paf = obj->affected; paf != NULL; paf = paf->next ) {
        if ( paf->duration < 0 )
                continue;
        if ( afpaf->duration < 0 ) {
                afpaf = paf;
                continue;
        }
        if ( afpaf->duration > paf->duration )
                afpaf = paf;
    }
    if ( ( afpaf != NULL ) && ( afpaf->duration > 0 ) ) {
        afpaf->duration += dice(((ch->pcdata->learned[sn] * level)/1000), 10);
        if ( afpaf->duration > 9999 )
            afpaf->duration = 9999;
        ch->printlnf( "The %s affect seems steadier.", skill_table[afpaf->type].name );
    }
    else
	{
		act("You couldn't find any effect on $p to extend.",ch,obj,NULL,TO_CHAR);
		return HALF_MANA;
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_wrath( int sn, int level, char_data *ch, void *vo, int )
{
	AFFECT_DATA af;
	char_data *victim;

	victim=(char_data *) vo;

	if ( !IS_IMMORTAL(ch))
	{
		if(!IS_EVIL(ch))
		{
			ch->println("You must be evil to wield such magic.");
			return NO_MANA;
		}

		if(IS_EVIL(victim))
		{
			ch->println("You may not cast that against those who are evilly aligned.");
			return HALF_MANA;
		}
	}

	if(!saves_spell( level, victim, DAMTYPE(sn)) )
	{
	    af.where     = WHERE_AFFECTS;
	    af.type      = sn;
	    af.level     = level;
	    af.duration  = level/5;
	    af.location  = APPLY_HIT;
	    af.modifier  = (-level / 4) - 15;
	    af.bitvector = 0;
	    affect_join( victim, &af );
	    victim->println("You feel a force of great ill invade your soul.");
	    act("$n drops to $s knees in horror.",victim,NULL,NULL,TO_ROOM);
	    return FULL_MANA;
	}
	else
	{          
		act("$n shivers against a great force, but it passes.",victim,NULL,NULL,TO_ROOM);
		victim->println("You feel a great ill, but it passes.");
		return FULL_MANA;
	}
	return FULL_MANA;
}
/**************************************************************************/
