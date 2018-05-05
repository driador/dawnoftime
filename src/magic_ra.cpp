/**************************************************************************/
// magic_ra.cpp - spells/skills written by Rathern
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: magic_ra.c - spells written by Rathern                           *
 *                                                                         *
 * summon_guardian - conjuration, summoning & wild magic (roleplay spell)  *
 * cause_fear      - phantasum, illusion                                   *
 * wind shield     - air, conjuration                                      *
 * spell_imprint   - for brew and scribe no realms                         *
 * brew $ scribe   - skills for magic some clsses                          *
 * soberness       - time & healing                                        *
 * drunkeness      - body                                                  * 
 ***************************************************************************/
#include "include.h"
#include "magic.h"
#include "msp.h"

/********************************/
/* START OF FUNCTION PROTOTYPES */

DECLARE_DO_FUN(do_bug);
void do_fearful(char_data *);  /* in affects.c */
void do_fearmagic(char_data *);  /* in affects.c */
bool	remove_obj	args( (char_data *ch, int iWear, bool fReplace ) );

/*  END OF FUNCTION PROTOTYPES  */
/********************************/


/****************************************************************************
 *  spell_summon_guardian - by Rathern & Kalahn - September 97              *
 ****************************************************************************/
SPRESULT spell_summon_guardian( int , int , char_data *ch, void *, int )
{
    char_data *mob_guardian;
    MOB_INDEX_DATA *pMobIndex;
    char buf[MSL];

    if ((pMobIndex = get_mob_index(MOB_VNUM_SUMMON_GUARDIAN))) /* make sure mob exists */
    {
        mob_guardian = create_mobile( pMobIndex, 0 ); /*grab mobile SUMMON_GUARDIAN_MOB_VNUM */
    
        mob_guardian->level = ch->level;          /*set mobs level*/
        mob_guardian->max_hit = ch->level*10;     /*set mobs max hit points*/
        mob_guardian->hit = mob_guardian->max_hit* 2/3;   /*set mobs current hit points to a third its total*/
        mob_guardian->timer = ch->level;          /*set how long the mob will last*/
        char_to_room( mob_guardian, ch->in_room); /*send mob to summoner*/
    
        act("As $n completes $s spell, a large stone golem with wings, forms in the room.", ch,NULL,NULL,TO_ROOM);
        ch->println("A shimmering stone golem with wings fades into existence.");
    }
    else // mob with the vnum not found
    {
        sprintf(buf,"BUG: in spell_summon_guardian - missing mob vnum %d!\r\n", MOB_VNUM_SUMMON_GUARDIAN);
        wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); /* put it on the bug wiznet channel */
        log_string( buf ); /* log the bug in the logs */
        do_bug ( ch, buf); /* report the bug into the bug file */
        ch->print( buf );
        return NO_MANA;
    }
	return FULL_MANA;
}    


/****************************************************************************
 *  spell_cause_fear      - by Rathern & Kalahn - September 97              *
 ****************************************************************************/
SPRESULT spell_cause_fear( int sn, int level, char_data *ch, void *vo,int )
{
    char_data *victim = (char_data *) vo;
    AFFECT_DATA af;
	
    if ( saves_spell( level, victim, DAMTYPE(sn))){
		ch->println("The spell fizzles and dies.");
		return FULL_MANA;
    }
	
    if ( IS_AFFECTED(victim, AFF_FEAR) || IS_AFFECTED2(victim, AFF2_FEAR_MAGIC) ){
		ch->println("They are already running for their life.");
		return HALF_MANA;
	}
	
	if (IS_SET(victim->imm_flags,IMM_FEAR)){
		ch->println( "They are unaffected by your fear." );
		return FULL_MANA;
	}
	
    af.where     = WHERE_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    if (ch->level > 25) af.duration +=1;
    if (ch->level > 50) af.duration +=1;
    if (ch->level > 75) af.duration +=1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FEAR;
    affect_to_char( victim, &af );
    victim->println("You panic as you are gripped by an incredible fear.");
    act( "$n screams and runs away.", victim, NULL, NULL, TO_ROOM );
    do_fearful(victim); // in magic.c 
    return FULL_MANA;
}

/****************************************************************************
 *  spell_fear_magic    - by Tibault - June 2000 (copy of spell_cause_fear) *
 ****************************************************************************/
SPRESULT spell_fear_magic( int sn, int level, char_data *ch, void *vo,int )
{
    char_data *victim = (char_data *) vo;
    AFFECT_DATA af;
	
    if ( saves_spell( level, victim, DAMTYPE(sn)))
    {
		ch->println("The spell fizzles and dies.");
		return FULL_MANA;
    }
	
    if (IS_SET(victim->imm_flags,IMM_FEAR))
    {
		ch->println( "They are unaffected by your fear." );
		return FULL_MANA;
    }
	
    if ( IS_AFFECTED2(victim, AFF2_FEAR_MAGIC) || IS_AFFECTED(victim, AFF_FEAR) )
	{
		ch->println("They are already running for their life.");
		return HALF_MANA;
	}
	
    af.where     = WHERE_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    if (ch->level > 25) af.duration +=1;
    if (ch->level > 50) af.duration +=1;
    if (ch->level > 75) af.duration +=1;
	af.location  = APPLY_ST;
	af.modifier  = -1 * (level / 10);
    af.bitvector = AFF2_FEAR_MAGIC;
    affect_to_char( victim, &af );
    victim->println("You panic as you are gripped by an incredible fear.");
    act( "$n screams and runs away.", victim, NULL, NULL, TO_ROOM );
    do_fearmagic(victim); /* in magic.c */
    return FULL_MANA;
}
/****************************************************************************
 *  spell_spell_imprint (brew,scribe) - by Jason Huang edited by Rathern    *
 ****************************************************************************/
SPRESULT spell_imprint( int sn, int, char_data *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int       sp_slot, i, mana;
    char      buf[ MSL ];
	
	if (skill_table[sn].spell_fun == spell_null )
	{
        ch->println("That is not a spell.");
        return NO_MANA;
	}
	
    // counting the number of spells contained within	
    for (sp_slot = i = 1; i < 5; i++)
	{
		if (obj->value[i] != -1)
			sp_slot++;
	}
	
    if (sp_slot > 4)
    {
        act ("$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR);
        return NO_MANA;
    }
	
	// scribe/brew costs 4 times the normal mana required to cast the spell
    mana = ((skill_table[sn].min_mana)*4);
	
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
        ch->println("You don't have enough mana.");
        return NO_MANA;
    }
	
    if ( number_percent( ) > get_skill(ch,sn))
    {
        ch->println("You lost your concentration.");
        ch->mana -= mana / 2;
        return NO_MANA;
    }
	
    // executing the imprinting process	
    ch->mana -= mana;
    obj->value[sp_slot] = sn;
	
	
    // Making it successively harder to pack more spells 
	// into potions or scrolls - JH 
	if ( !IS_IMMORTAL(ch) )
	{
		switch( sp_slot )
		{
			
		default:
			bugf( "sp_slot has more than %d spells.", sp_slot );
			return NO_MANA;
			
		case 1:
			if ( number_percent() > 90 )
			{ 
				ch->printlnf( "The magic enchantment has failed --- the %s vanishes.",
					item_type_name(obj) );
				extract_obj( obj );
				check_improve(ch,gsn_scribe, false, 1);
				return FULL_MANA;
			}     
			break;
			
			
		case 2:
			if ( number_percent() > 45 )
			{ 
				ch->printlnf( "The magic enchantment has failed --- the %s vanishes.",
					item_type_name(obj) );        // (obj->item_type) );
				extract_obj( obj );
				check_improve(ch,gsn_scribe, false, 1);
				return FULL_MANA;
			}     
			break;
			
			
		case 3:
			if ( number_percent() > 20 )
			{ 
				ch->printlnf( "The magic enchantment has failed --- the %s vanishes.",
					item_type_name(obj) );
				extract_obj( obj );
				check_improve(ch,gsn_scribe, false, 1);
				return FULL_MANA;
			}     
			break;
		}
	}
	
	
    // labeling the item	
    free_string (obj->short_descr);
    sprintf ( buf, "a %s of ", item_type_name(obj) ); 
    for (i = 1; i <= sp_slot ; i++)
	{
		if (obj->value[i] != -1)
			
		{
			strcat (buf, skill_table[obj->value[i]].name);
			(i != sp_slot ) ? strcat (buf, ", ") : strcat (buf, "") ; 
		}
	}
	
	obj->short_descr = str_dup(buf);
	sprintf( buf, "%s %s '%s'", obj->name, item_type_name(obj), skill_table[sn].name);
	replace_string( obj->name, obj->name );
	ch->printlnf( "You have imbued a new spell to the %s.", item_type_name(obj) );
	check_improve(ch,gsn_scribe, true, 3);
	return FULL_MANA;
}


/****************************************************************************
 *  VOID_DO_BREW        (brew,scribe) - by Jason Huang edited by Rathern    *
 ****************************************************************************/
void do_brew ( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int sn;
    int dam;

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED )){
		if(ch->master){
			ch->master->println( "Not going to happen.\r\n");
		}
		return;
	}


    if ( !IS_NPC( ch )                                                  
	&& ch->level < skill_table[gsn_brew].skill_level[ch->clss] )
    {                                          
        ch->println("You do not know how to brew potions.");
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->println("Brew what spell?");
        return;
    }

    // Do we have a vial to brew potions? 
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( obj->item_type == ITEM_POTION && obj->wear_loc == WEAR_HOLD )
            break;
    }

    /* Interesting ... Most scrolls/potions in the mud have no hold
       flag; so, the problem with players running around making scrolls 
       with 3 heals or 3 gas breath from pre-existing scrolls has been 
       severely reduced. Still, I like the idea of 80% success rate for  
       first spell imprint, 25% for 2nd, and 10% for 3rd. I don't like the
       idea of a scroll with 3 ultrablast spells; although, I have limited
       its applicability when I reduced the spell->level to 1/3 and 1/4 of 
       ch->level for scrolls and potions respectively. --- JH */


    /* I will just then make two items, an empty vial and a parchment available
       in midgaard shops with holdable flags and -1 for each of the 3 spell
       slots. Need to update the midgaard.are files --- JH */

    if ( !obj )
    {
        ch->println("You are not holding a vial.");
        return;
    }

    if ( ( sn = skill_lookup(arg) )  < 0)
    {
        ch->println("You don't know any spells by that name.");
        return;
    }

    /* preventing potions of gas breath, acid blast, etc.; doesn't make sense
       when you quaff a gas breath potion, and then the mobs in the room are
       hurt. Those TAR_IGNORE spells are a mixed blessing. - JH */
  
    if ( (skill_table[sn].target != TAR_CHAR_DEFENSIVE) && 
         (skill_table[sn].target != TAR_CHAR_SELF)              ) 
    {
        ch->println("You cannot brew that spell.");
        return;
    }

        act( "$n begins preparing a potion.", ch, obj, NULL, TO_ROOM );
        act( "You begin to brew a potion.", ch, obj, NULL, TO_CHAR );

		if ( !IS_IMMORTAL( ch ))
	        WAIT_STATE( ch, skill_table[gsn_brew].beats ); 

    /* Check the skill percentage, memory and reasoning checks) */

	
    if ( !IS_NPC(ch) 
         && ( number_percent( ) < ch->pcdata->learned[gsn_brew] ||
              number_percent( ) < ((ch->modifiers[STAT_ME]-7)*5 + 
                                   (ch->modifiers[STAT_RE]-7)*3) ))
    {
        act( "$p explodes violently!", ch, obj, NULL, TO_CHAR );
        act( "$p explodes violently!", ch, obj, NULL, TO_ROOM );

		msp_to_room(MSPT_ACTION, MSP_SOUND_EXPLOSION, 
						0,
						ch,
						false,
						true);	

        dam = dice( ch->level, 10 );
        if ( saves_spell( ch->level, ch, DAM_ACID ) )
        dam /= 2;
        damage_spell( ch, ch, dam, gsn_acid_blast,DAM_ACID,true);
        extract_obj( obj );
        return;
    }

    /* took this outside of imprint codes, so I can make do_brew differs from
       do_scribe; basically, setting potion level and spell level --- JH */

    obj->level = ch->level/2;
    obj->value[0] = ch->level/4;
    spell_imprint(sn, ch->level, ch, obj); /* in magic_r.c and interpt.h */ 

	msp_skill_sound(ch, gsn_brew);
}


/****************************************************************************
 *  VOID_DO_SCRIBE      (brew,scribe) - by Jason Huang edited by Rathern    *
 ****************************************************************************/
void do_scribe ( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int sn;
    int dam;

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED )){
		if(ch->master){
			ch->master->println( "Not going to happen.");
		}
		return;
	}

    if ( !IS_NPC( ch )                                                  
	&& ch->level < skill_table[gsn_scribe].skill_level[ch->clss] )
	{
		ch->println("You are not able to scribe in your current condition.");
		return;
	}

	if ( IS_NPC( ch ) || IS_CONTROLLED( ch ) )
	{
		ch->println( "You are too charmed to concentrate on scribing." );
		return;
	}

	argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->println("Scribe what spell?");
        return;
    }

    // Do we have a parchment to scribe spells? 
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( obj->item_type == ITEM_SCROLL && obj->wear_loc == WEAR_HOLD )
            break;
    }

    if ( !obj )
    {
        ch->println("You are not holding a parchment.");
        return;
    }


    if ( ( sn = skill_lookup(arg) )  < 0)
    {
        ch->println("You don't know any spells by that name.");
        return;
    }

    if ( !IS_SPELL(sn))
    {
        ch->println("That isn't a spell.");
        return;
    }

	if ( IS_SET(skill_table[sn].flags, SKFLAGS_NO_SCRIBE) && !IS_IMMORTAL(ch) )
	{
		ch->println("It is impossible for a scroll to capture the essence of this spell.");
		return;
	}
    
    act( "$n begins writing a scroll.", ch, obj, NULL, TO_ROOM );
    act( "You begin to scribe a spell.", ch, obj, NULL, TO_CHAR );

	if ( !IS_IMMORTAL( ch )){
		WAIT_STATE( ch, skill_table[gsn_scribe].beats );
	}

    // Check the skill percentage, fcn(int,wis,skill) 
    if ( !IS_NPC(ch) 
         && ( number_percent( ) > ch->pcdata->learned[gsn_scribe] ||
              number_percent( ) > ((ch->modifiers[STAT_ME]-7)*5 + 
                                   (ch->modifiers[STAT_RE]-7)*3)) 
		 && !IS_IMMORTAL(ch) )
    {
        act( "$p bursts in flames!", ch, obj, NULL, TO_CHAR );
        act( "$p bursts in flames!", ch, obj, NULL, TO_ROOM );
    
        dam = dice(ch->level, 6)+10;
        if ( saves_spell( ch->level, ch, DAM_FIRE) ){
			dam /= 2;
		}
        damage_spell( ch, ch, dam, skill_lookup("fireball"),DAM_FIRE,true);
        extract_obj( obj );
		check_improve(ch,gsn_scribe, false, 1);
        return;
    }

    /* basically, making scrolls more potent than potions; also, scrolls
       are not limited in the choice of spells, i.e. scroll of enchant weapon
       has no analogs in potion forms --- JH */

    obj->level = ch->level*2/3;
    obj->value[0] = ch->level/3;
    spell_imprint(sn, ch->level, ch, obj); 

	msp_skill_sound(ch, gsn_scribe);

}


/****************************************************************************
 *  Starvation by Rathern                                                   *
 ****************************************************************************/
SPRESULT spell_starvation(int sn,int level,char_data *ch, void *vo,int )
{
    char_data *victim = (char_data *) vo;
    
    if (saves_spell(level, victim,DAMTYPE(sn)))
    {
		ch->println("Your spell fizzles and dies.");
		return FULL_MANA;
    } 
	
	if (IS_SET(victim->imm_flags,IMM_HUNGER))
	{
		ch->println( "They are unaffected by starvation." );
		return FULL_MANA;		
	}
	
    if (IS_NPC(victim))
    {
		act("$n looks extremely hungry all of a sudden.",victim,NULL,ch,TO_ROOM);
    }else{
		victim->println("The pains of hunger grip your stomach.");
		act("$n clutches at their stomach, they look in agony.",victim,NULL,NULL,TO_ROOM);
		victim->pcdata->condition[COND_HUNGER] = 0;
	}
    return FULL_MANA;
}


/****************************************************************************
 *  Dehydration by Rathern                                                  *
 ****************************************************************************/
SPRESULT spell_dehydration( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	
	if (saves_spell(level , victim,DAMTYPE(sn)))
	{
		ch->println("Your spell fizzles and dies.");
		return FULL_MANA;
	} 
	
	if (IS_SET(victim->imm_flags,IMM_THIRST))
	{
		ch->println( "They are unaffected by thirst." );
		return FULL_MANA;
		
	}
	
	if (IS_NPC(victim))
	{
		act("$n looks extremely thirsty all of a sudden.",victim,NULL,ch,TO_CHAR);
    }
	else
	{
		victim->println("Your throat is dry and parched, you feel incredibly thirsty.");
		act("$n clutches at their throat, they look in agony.",victim,NULL,NULL,TO_ROOM); 
		victim->pcdata->condition[COND_THIRST] = 0;
	}
    return FULL_MANA;
}


/****************************************************************************
 *  wind_shield by Rathern                                                  *
 ****************************************************************************/
SPRESULT spell_wind_shield( int sn, int level, char_data *ch, void *vo, int )
{
    char_data *victim = (char_data *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
		if (victim == ch)
			ch->println("You have that spell on you already.");
		else
			act("$N already has a shield of wind surrounding them.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
    }

    if ( is_affected( victim, gsn_cyclone ) )
    {
		if (victim == ch)
			ch->println("The winds are protecting you as much as they can.");
		else
			act("$N is already protected by the winds.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
    }


    af.where     = WHERE_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/10;
    af.modifier  = ch->level/-2;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->println("You are enveloped within a whirling tomb of wind.");
    act("A roaring wind envelops $n.",victim,NULL,NULL,TO_ROOM);
    return FULL_MANA;
}


/****************************************************************************
 *  Sober by Rathern                                                        *
 ****************************************************************************/
SPRESULT spell_sober( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	
	if (saves_spell(level , victim,DAMTYPE(sn)))
	{
		ch->println("Your spell fizzles and dies.");
		return FULL_MANA;
	} 

	if (IS_NPC(victim))
	{
		act("$n looks extremely sober all of a sudden.",victim,NULL,ch,TO_CHAR);
	}
	else
	{
		victim->println("Your feel suddenly more sober and awake.");
		act("$n looks more sober all of a sudden.",victim,NULL,NULL,TO_ROOM); 
		victim->pcdata->condition[COND_DRUNK] = 0;
	}
    return FULL_MANA;
}


/****************************************************************************
 *  Drunkenness by Rathern                                                  *
 ****************************************************************************/
SPRESULT spell_drunkeness( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;

	if (saves_spell(level , victim,DAMTYPE(sn)))
	{
		ch->println("Your spell fizzles and dies.");
		return FULL_MANA;
	}

	if (IS_NPC(victim))
	{
		act("$n looks extremely drunk all of a sudden.",victim,NULL,ch,TO_CHAR);
	}
	else
	{
		victim->println("Your feel suddenly drunk all of a sudden.");
		act("$n looks drunk all of a sudden.",victim,NULL,NULL,TO_ROOM); 
		victim->pcdata->condition[COND_DRUNK] = 15;
	}
    return FULL_MANA;
}


/****************************************************************************
 *  Permanance by Kalahn & Rathern                                          *
 ****************************************************************************/
SPRESULT spell_permanance(int sn, int level, char_data *ch, void *vo, int )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
	SPRESULT MANA_RESULT=FULL_MANA;
    int result, fail;
    bool hit_found = false, dam_found = false, ac_found = false;

    // do standard checks if it can be done here
//    if ((obj->item_type !=ITEM_WEAPON) && (obj->item_type !=ITEM_ARMOR))
    if ((obj->item_type != ITEM_ARMOR)
	  && (obj->item_type !=ITEM_WEAPON)
      && (obj->item_type != ITEM_LIGHT) 
	  && (obj->item_type != ITEM_TREASURE)
      && (obj->item_type != ITEM_CLOTHING)
	  && (obj->item_type != ITEM_CONTAINER)
	  && (obj->item_type != ITEM_GEM)
      && (obj->item_type != ITEM_JEWELRY))
    {
        ch->println("That isn't a suitable item.");
		return NO_MANA;
	}
	
	if (obj->wear_loc != -1)
	{
		ch->println("The item must be carried to have this spell cast on it.");
		return NO_MANA;
	}

	if (!IS_NPC(ch))
	{
		if (ch->pcdata->tired > 16)
		{
			ch->println( "You are not well-rested enough to cast this "
				"spell just now.  Have a nap first." );
			return NO_MANA;
		}
	}

	fail = 25;	// base 25% chance of failure

	// those who are skilled at permance have a better chance
	if (!IS_NPC(ch))
	{
		fail -= (ch->pcdata->learned[sn]/4);

		if (ch->pcdata->learned[sn]>75)
			fail -= (ch->pcdata->learned[sn]-75)*2;
	}

	// find the current bonuses - affect success
	for (paf=OBJECT_AFFECTS(obj);paf != NULL; paf = paf->next )
	{
		if ( paf->location == APPLY_HITROLL )
		{
			hit_found = true;
			fail += 2 * paf->modifier * paf->modifier;
		}
		else if (paf->location == APPLY_DAMROLL )
		{
			dam_found = true;
			fail += 2 * paf->modifier * paf->modifier;
		}
		else if ( paf->location == APPLY_AC )
		{
			ac_found = true;
			fail += 4 * paf->modifier * paf->modifier;
		}
		else
		{
			fail += 10;
		}
	}
	
	// apply other modifiers
	fail -= 3 * level/2;

	if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
		fail -= 25;
	if (IS_OBJ_STAT(obj,OBJEXTRA_GLOW))
		fail -= 5;

	fail = URANGE(5,fail,95);
	result = number_percent();
	
    if (!IS_NPC(ch))
    {
        MANA_RESULT=ALL_MANA;
        ch->pcdata->tired += 25;
        ch->pcdata->condition[COND_THIRST] = 0;
        ch->pcdata->condition[COND_HUNGER] = 0;
    }

    // the moment of truth
    if (result < (fail / 5))  // item disolved
    {
        act("$p shivers and dissolves before your eyes!",ch,obj,NULL,TO_CHAR);
        act("$p shivers and dissolves before your eyes!",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
        return MANA_RESULT;
    }

    if (result < (fail / 2)) // item disenchanted
	{
		AFFECT_DATA *paf_next;

		act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
		act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);

        // free all affects 
        for (paf = obj->affected; paf; paf = paf_next)
        {
            paf_next = paf->next; 
            free_affect(paf);
        }
        obj->affected = NULL;
        obj->no_affects = true; // flag the object as no_affects so 
								// the olc template's affects arent used.
        // clear all flags 
        obj->extra_flags = 0;
		return MANA_RESULT;
	}

    if ( result <= fail )  // failed, no bad result
    {
		ch->println("Nothing seemed to happen.");
		return MANA_RESULT;
	}

	if (!obj->affected)
	{
		act("You couldn't find any effect on $p to make permanent.", ch, obj, NULL,TO_CHAR);
		return HALF_MANA;
	}

	// now make the enchants perm
	if (ac_found) 
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->location == APPLY_AC && paf->duration!=-1)
            {
                paf->duration = -1;
                act("The protective qualities are now permanent.",ch,obj,NULL,TO_CHAR);
            }
        }
    }
    if (dam_found) // damroll
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if (paf->location == APPLY_DAMROLL && paf->duration!=-1)
            {
                paf->duration = -1;
                act("The damaging affect is now permanent.",ch,obj,NULL,TO_CHAR);
            }
        }
    }
    if (hit_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->location == APPLY_HITROLL && paf->duration!=-1)
            {
                paf->duration = -1;
                act("The hitting affect is now permanent.",ch,obj,NULL,TO_CHAR);
            }
        }
    }
    obj->level+=5; // increase the level of the object
	return MANA_RESULT;
}

/****************************************************************************
 *  Blade Permanance by Kalahn                                              *
 ****************************************************************************/
SPRESULT spell_blade_permanance(int , int level, char_data *ch, void *vo, int)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int result, fail;
    bool vorpal_found = false, sharp_found = false, frost_found= false,
        shocking_found= false, flame_found = false, vampiric_found= false;

    /* do standard checks if it can be done here */
    if ((obj->item_type !=ITEM_WEAPON))
    {
        ch->println("That isn't an suitable item.");
        return NO_MANA;
    }
    if (obj->wear_loc != -1)
    {
        ch->println("The item must be carried to have this spell cast on it.");
        return NO_MANA;
    }

    fail = 25;	// base 25% chance of failure

    // find the current bonuses - affect success 
    for (paf = OBJECT_AFFECTS(obj); paf; paf=paf->next )
    {
        if ( paf->bitvector== WEAPON_VORPAL && paf->duration != -1)
        {
            vorpal_found = true;
            fail += 60;
        }      
        else if ( paf->bitvector== WEAPON_SHARP && paf->duration != -1)
        {
            sharp_found = true;
            fail += 60;
        }      
        else if ( paf->bitvector== WEAPON_FLAMING && paf->duration != -1)
        {
            flame_found = true;
            fail += 60;
        }      
        else if ( paf->bitvector== WEAPON_SHOCKING && paf->duration != -1)
        {
            shocking_found = true;
            fail += 60;
        }      
        else if ( paf->bitvector== WEAPON_FROST && paf->duration != -1)
        {
            frost_found = true;
            fail += 60;
            
        }
        else if ( paf->bitvector== WEAPON_VAMPIRIC && paf->duration != -1)
        {
            vampiric_found = true;
            fail += 80;           
        }
        else
        {
            fail += 25;
        }
    }

    // apply other modifiers 
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS)){
        fail -= 25;
    }
	if (IS_OBJ_STAT(obj,OBJEXTRA_GLOW)){
        fail -= 5;
	}

    fail = URANGE(5,fail,95);
    result = number_percent();

    // the moment of truth 
    if (result < (fail / 5))  // item disolved 
    {
        act("$p shivers and dissolves before your eyes!",ch,obj,NULL,TO_CHAR);
        act("$p shivers and dissolves before your eyes!",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
        return FULL_MANA;
    }

    if (result < (fail / 2)) // item disenchanted 
    {
        AFFECT_DATA *paf_next;
    
        act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
        act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
    
        // free all affects 
        for (paf = obj->affected; paf != NULL; paf = paf_next)
        {
            paf_next = paf->next; 
            free_affect(paf);
        }
        obj->affected = NULL;
        obj->no_affects = true; // flag the object as no_affects so 
								// the olc template's affects arent used.
        // clear all flags 
        obj->extra_flags = 0;
        return FULL_MANA;
    }

    if ( result <= fail )  // failed, no bad result
    {
        ch->println("Nothing seemed to happen.");
        return FULL_MANA;
    }

    if (!obj->affected) // if it doesn't have any local affects, no one has 'enchanted' it
    {
        act("You couldn't find any effect on $p to make permanent.",
            ch, obj, NULL,TO_CHAR);
        return HALF_MANA;
    }

    // now make the enchants perm 
    if (vorpal_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->bitvector== WEAPON_VORPAL && paf->duration != -1)
            {
                paf->duration = -1;
                act("The vorpal affect is now permanent.",ch,obj,NULL,TO_CHAR);
            }
        }
    }
    if (sharp_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->bitvector== WEAPON_SHARP && paf->duration != -1)
            {
                paf->duration = -1;
                act("The sharp affect is now permanent.",ch,obj,NULL,TO_CHAR);
            }
        }
    }
    if (flame_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->bitvector== WEAPON_FLAMING && paf->duration != -1)
            {
                paf->duration = -1;
                act("The fiery aura appears to be permanent now.",ch,obj,NULL,TO_CHAR);
            }
        }
    }
    if (shocking_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->bitvector== WEAPON_SHOCKING && paf->duration != -1)
            {
                paf->duration = -1;
                act("The sparks of electricity appear to be more predictable and stable now.",ch,obj,NULL,TO_CHAR);
            }
        }
    }
    if (frost_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->bitvector== WEAPON_FROST && paf->duration != -1)
            {
                paf->duration = -1;
                act("The frost on the blade is now permanent.",ch,obj,NULL,TO_CHAR);
            }
        }
    }
    if (vampiric_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->bitvector== WEAPON_VAMPIRIC && paf->duration != -1)
            {
                paf->duration = -1;
                act("The vampiric affects are now permanent.",ch,obj,NULL,TO_CHAR);
            }
        }
    }

	return FULL_MANA;
}


/****************************************************************************
 *  steel_breath by Rathern                                                 *
 ****************************************************************************/
SPRESULT spell_steel_breath( int sn, int level, char_data *ch, void *,int )
{
    char_data *vch;
    char_data *vch_next;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes out a rain of deadly steel!",ch,NULL,NULL,TO_ROOM);
    act("You breath out a rain of deadly steel.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(16,ch->hit);
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
		vch_next = vch->next_in_room;
		
		if (is_safe_spell(ch,vch,true)
			||  (IS_NPC(ch) && IS_NPC(vch) 
			&&   (ch->fighting == vch || vch->fighting == ch)))
			continue;
		
		// died from last attack, or got out of room for some other reason
		if(vch->in_room!=ch->in_room){
			continue;
		}
		
		if (saves_spell(level,vch,DAMTYPE(sn)))
		{
			damage_spell(ch,vch,dam/2,sn,DAMTYPE(sn),true);
		}
		else
		{
			damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
		}
    }
	return FULL_MANA;
}


/****************************************************************************
 *  shadow_breath by Rathern                                                *
 ****************************************************************************/
SPRESULT spell_shadow_breath( int sn, int level, char_data *ch, void *,int )
{
    char_data *vch;
    char_data *vch_next;
    int dam, hp_dam, dice_dam, hpch, dam2;
    int temp_hps;

    act("$n breathes out a cloud of blackness!",ch,NULL,NULL,TO_ROOM);
    act("You breath out a cloud of blackness.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(16,ch->hit);
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell(ch,vch,true)
            ||  (IS_NPC(ch) && IS_NPC(vch) 
            &&   (ch->fighting == vch || vch->fighting == ch)))
            continue;

        if(ch==vch)
        {
            ch->println("You cannot target yourself!");
            return NO_MANA;
        }

        /* vampiric touch */
        dam2=dice(level, 7)/abs(4+ch->alliance);
    
        if(saves_spell(level, vch, DAMTYPE(sn)))
        {
            dam2 /= 2;
        }

        temp_hps=vch->hit; // backup the current hitpoints
        if (damage_spell(ch, vch, dam2, sn, DAMTYPE(sn), true)) // do the damage
		{
			if(vch->hit<-10)
			{
				vch->hit=-10;
			}
    
			// gives caster victims hps
			if(vch!=NULL)
			{
			   ch->hit+= temp_hps-vch->hit;
			}   

		   // energy drain   
			if ( saves_spell( level, vch,DAMTYPE(sn)) )
			{
				vch->println("You feel a momentary chill.");
				return FULL_MANA;
			}
       
			if ( vch->level <= 2 )
			{
				dam2     = ch->hit + 1;
			}
			else
			{
				gain_exp( vch, 0 - number_range( level/2, 3 * level / 2 ) );
				vch->mana    /= 2;
				vch->move    /= 2;
				dam2      = dice(1, level/4);
    
				vch->println("You feel your life slipping away!!!");
				ch->println("Wow....what a rush!");
			}

			// If they died from an attack up above don't kill them again
			if (vch->pksafe==30) 
				return FULL_MANA;

			// died from last attack, or got out of room for some other reason
			if(vch->in_room!=ch->in_room){
				continue;
			}

			temp_hps=vch->hit;

			damage_spell( ch, vch, dam, sn, DAMTYPE(sn) ,true);

			if(vch->hit<-10) vch->hit=-10;
			{
				ch->hit+=temp_hps-vch->hit;
			}
			   
			// If they died from an attack up above don't kill them again
			if (vch->pksafe==30) 
				return FULL_MANA;

			// died from last attack, or got out of room for some other reason
			if(vch->in_room!=ch->in_room){
				continue;
			}
			if (saves_spell(level,vch,DAMTYPE(sn)))
			{
				damage_spell(ch,vch,dam/2,sn,DAMTYPE(sn),true);
			}
			else
			{
				damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
			}
		}
    }
	return FULL_MANA;
}


/****************************************************************************
 *  element_ring by Kalahn & Rathern                                        *
 ****************************************************************************/
SPRESULT spell_element_ring( int , int level, char_data *ch, void *vo, int target)
{
    // negative effects of ring
    ch->println("You call on the power of the ring!");

    // spells stored in ring
    spell_animal_essence(skill_lookup("animal essence"), level, ch, ch, target);
    spell_earthquake(gsn_earthquake, level, ch, NULL, TARGET_NONE);
    spell_frostball( gsn_frostball, level, ch, vo, target);
    spell_downdraft( gsn_downdraft, level, ch, vo, target);
    spell_fireball(  gsn_fireball, level, ch, vo, target);

    if(!IS_NPC(ch) && ch->level<LEVEL_IMMORTAL)
	{
		ch->println("You feel tired.");
		act("$n looks drained and tired.",ch,NULL,NULL,TO_ROOM);
		ch->pcdata->tired += 35;
	}
	return FULL_MANA;
}

/****************************************************************************
 *  spell_summon_vyr - by Rathern - September 97                            *
 ****************************************************************************/

/* stay as i have set it.  Multiple copies of the mobs shouldnt be able to be created, at 
 the moment they can.

 If you ae feeling really wounderful, the statue can only be used
 once every 24 mud hours, if you can somehow do that then you can remove the negative 
 effects of using the statue, those effects are just an increase in the tired counter.

 i remarked the negative effects part cause it was crashing the spell,
 can you also have a look at that :) 
*/
SPRESULT spell_summon_vyr( int , int , char_data *ch, void *, int )
{
    char_data *mob_vyr, *pMobCheck;
    MOB_INDEX_DATA *pMobIndex;
	int vtimer;
    char buf[MSL];


    if(number_range(0,1))
	{	// good
		pMobIndex = get_mob_index(MOB_VNUM_VYR_GOOD);

		// ensure good mob exists
		if (!pMobIndex) 
		{
			sprintf(buf,"BUG: in spell_summon_vyr - missing host mob!!! <vnum %d>\r\n",
				MOB_VNUM_VYR_GOOD);
			wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel
			log_string( buf ); // log the bug in the logs 
			do_bug ( ch, buf); // report the bug into the bug file
			ch->print( buf );
			return NO_MANA;
		}

		vtimer = ch->level/3;	// set how long the good mob will last
	}
	else // bad
	{
		pMobIndex = get_mob_index(MOB_VNUM_VYR_BAD);

		// ensure bad mob exists
		if (!pMobIndex) 
		{
			sprintf(buf,"BUG: in spell_summon_vyr - missing host mob!!! <vnum %d>\r\n",
				MOB_VNUM_VYR_BAD);
			wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel
			log_string( buf ); // log the bug in the logs 
			do_bug ( ch, buf); // report the bug into the bug file
			ch->print( buf );
			return NO_MANA;
		}

		vtimer = ch->level/10; // set how long the bad mob will last
	}

	// check for another vyr already in room
	pMobCheck = get_char_room( ch, pMobIndex->player_name);
	if ( pMobCheck ) 
	{
		ch->println("Nothing happens.");
		act("$n looks drained and tired.",ch,NULL,NULL,TO_ROOM);
        if (!IS_NPC(ch))
		{
    		ch->pcdata->tired += 20;
        }
		return FULL_MANA;
	}
    
	mob_vyr = create_mobile( pMobIndex, 0 ); // create Vyr mobile 
    mob_vyr->level = UMAX(ch->level-10,1);// set mobs level
    mob_vyr->max_hit = ch->level*10;      // set mobs max hit points
    mob_vyr->hit = mob_vyr->max_hit*2/3;  // set mobs current hit points to a third its total
    mob_vyr->timer = vtimer;			  // set how long the mob will last
    char_to_room( mob_vyr, ch->in_room);  // send mob to summoner

   // negative effects of statue
	ch->println("You call on the power of the statue!.");
	ch->println("You feel tired.");
	act("$n looks drained and tired.",ch,NULL,NULL,TO_ROOM);
    if (!IS_NPC(ch)){
    	ch->pcdata->tired += 30;
    }

    act("As $n brandishes the statuette a form appears in the room!", ch,NULL,NULL,TO_ROOM);
    ch->println("A shimmering form appears in the room.");  

	return FULL_MANA;
}    

/**************************************************************************/
// - Kal April 98
SPRESULT spell_summon_justice( int sn, int , char_data *ch, void *, int )
{
	OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *pObj;
    char buf[MSL];
	int tempwizi;

	// can't be cast by mobs
	if (IS_NPC(ch))
	{
		ch->println("Players only sorry.");
		return NO_MANA;
	}

	// check the spell hasn't been cast within the laston week
	if ((current_time - (7*24*60*60)) < ch->pcdata->last_used[sn])
	{
		ch->println("Nothing happens.");

		sprintf (buf, "%s cast summon justice, was declined last used %s ago.\r\n",
			ch->name, timediff(current_time, ch->pcdata->last_used[sn]) );
        wiznet(buf,NULL,NULL,WIZ_SECURE,0,AVATAR); // put it on the secure channel 
		log_string( buf ); // log the bug in the logs 
		return NO_MANA;
	}

	// check object exists 
	pObjIndex = get_obj_index(OBJ_VNUM_SUMMON_JUSTICE);

    if (!pObjIndex) // object with the vnum not found
    {
        sprintf(buf,"BUG: in spell_summon_justice - missing object vnum %d!\r\n", OBJ_VNUM_SUMMON_JUSTICE);
        wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel
        log_string( buf ); // log the bug in the logs 
        do_bug ( ch, buf); // report the bug into the bug file
        ch->print( buf ); // tell the char what happened
        return NO_MANA;
    }

	// record the last time the spell was used
	ch->pcdata->last_used[sn] = current_time; 

	//  make the object
    pObj= create_object(pObjIndex);
    obj_to_char(pObj, ch);

	pObj->timer = 30+(ch->level/5);

	// tell the room what happened    
	act("A bright light surrounds $n\r\n"
	"Leaving $p in $s hand!",ch,pObj,NULL,TO_ROOM);
    ch->println("A flash of lightening fills the room,");
	act("Then from nowhere a $p appears!",ch,pObj,NULL,TO_CHAR);

	// put the object in characters hand (use wizi to hide some of it)
	tempwizi = INVIS_LEVEL(ch);
	INVIS_LEVEL(ch)= LEVEL_IMMORTAL;
	remove_obj( ch, WEAR_WIELD, true ); // remove their current object
	wear_obj( ch, pObj, true, false );	// wield the justice object
	INVIS_LEVEL(ch)= tempwizi;

	return FULL_MANA;
}

/**************************************************************************/
// Rathern April 98
SPRESULT spell_thorny_feet( int sn, int level, char_data *ch, void *vo, int )
{
    char_data *victim = (char_data *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch){
          ch->println("You have that spell on you already.");
		}else{
          act("$N appears to already be afflicted with thorns.", ch,NULL,victim,TO_CHAR);
		}
        return HALF_MANA;
    }

    if (saves_spell(level,victim,DAMTYPE(sn)))
    {
        ch->println("They seem unaffected.");
        return HALF_MANA;
    }   

    af.where     = WHERE_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/20 + 1;
    af.modifier  = -5000;
    af.location  = APPLY_MOVE;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->println("Your legs and feet are covered with thorns, you can't MOVE!");
    act("$n suddenly appears to be covered with thorns.",victim,NULL,NULL,TO_ROOM);
    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_unholy_aura( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( !IS_IMMORTAL(ch) && (!IS_EVIL(victim) || !IS_EVIL(ch)))
	{
		if(ch == victim)
		{
			ch->println("You are not unholy enough to cast this spell.");
			return NO_MANA;
		}	
		ch->println("They are too righteous!");
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
		af.bitvector  = RES_HOLY;
		affect_to_char( victim, &af);
		victim->println("You are surrounded by a red aura.");
		act("$n is surrounded with a red aura.", ch, NULL, victim, TO_NOTVICT);
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
