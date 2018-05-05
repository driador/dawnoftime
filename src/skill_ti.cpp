/**************************************************************************/
// skill_ti.cpp - skills written by Tibault
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"

void do_collectwater( char_data *ch);
void do_collect( char_data *ch, char *argument);
void do_pounce( char_data *ch, char *argument);
void landchar( char_data *ch); // act_move.cpp
void do_worship(char_data *ch, char *argument);

/**************************************************************************/
// - Tibault Jun 2000
void do_collect( char_data *ch, char *argument)
{
	char	arg[MIL];
	argument = one_argument(argument,arg);

	if ( !str_cmp(arg, "water" ) )
	{
		do_collectwater(ch);
		return;
	}
	else
	{
		ch->println("Collect what?");
		ch->println("Syntax is 'collect <object>'");
		ch->println("Only 'water' is available at this time.");
		return;
	}
}

/**************************************************************************/
// - Tibault Jun 2000, class flag tweaks by Kalahn
void do_collectwater( char_data *ch)
{
	OBJ_DATA *obj;
	int liquid = 0;
	bool foundFountain = false;

	// can't collect water while sleeping - Daos
	if (!IS_AWAKE(ch)){
		ch->println("Collecting water in your sleep?  Very impressive!");
		return;
	}

	// Check for a fountain with untainted water.
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
		if ( obj->item_type == ITEM_FOUNTAIN )
		{
			if ( ( liquid = obj->value[2] ) < 0 )
			{
				bugf( "Do_collectwater: bad liquid number %d.", liquid );
				liquid = obj->value[2] = 0;
			}

			if ( strstr(liq_table[liquid].liq_name, "untainted water") )
				break;
			else 
				// Found a fountain but not with correct liquid type.
				foundFountain = true;
		}
	}	
	if ( obj == NULL )
	{
		if ( foundFountain )
		ch->printlnf( "This %s will only dirty your dousing flask.", 
			liq_table[liquid].liq_name);
		else
			ch->println("Collect water from what?");
		return;
	}
    if ( ( liquid = obj->value[2] )  < 0 )
	{
		bugf( "Do_collectwater: bad liquid number %d.", liquid );
           liquid = obj->value[2] = 0;
    }
	if ( obj->value[1] <= 0 )
	{
		ch->println("It is already empty.");
		return;
	}

	// Check for empty dousing flask.
	for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
		if ( strstr(obj->pIndexData->material, "dousingflask") )
			break;
    }

	if ( obj == NULL )
	{
		ch->println("You do not seem to have an empty dousing flask!");
		return;
	}

	int objvnum=OBJ_VNUM_CRYSTAL_FLASK;
	if(!HAS_CLASSFLAG(ch, CLASSFLAG_CAN_COLLECT_WATER) && number_range(1,10)!=1){
		objvnum=OBJ_VNUM_BADCRYSTAL_FLASK; // classes that can't do it usually stuff up
	}

	if ( get_obj_index(objvnum) == NULL )
	{
		bugf("Vnum %d not found for do_collectwater!", objvnum);
		ch->printlnf( "Vnum %d not found for do_collect!, please report to the admin.", 
			objvnum);
		return;
	}

	extract_obj(obj);
	obj = create_object( get_obj_index( objvnum ));
	obj_to_char( obj, ch );
	act("$n expertly busies $mself collecting water in a dousing flask.",ch,NULL,NULL,TO_ROOM);
	if(HAS_CLASSFLAG(ch, CLASSFLAG_CAN_COLLECT_WATER)){
		ch->println("You collect water into your dousing flask.");
		WAIT_STATE( ch, 16 );
	}else{
		ch->println("You carefully collect water into your dousing flask.");
		WAIT_STATE( ch, 36 );
	}
	return;
}

/**************************************************************************/
// - Tibault Jun 2000
void do_pounce(char_data *ch, char *argument)
{
	char arg[MIL];
	char_data *victim;
	int chance;
	int beats;
	
	one_argument(argument,arg);
	
	if (( chance = get_skill(ch,gsn_pounce)) == 0
	||  ( IS_NPC(ch) )
	||  ( !IS_NPC(ch) && ch->level < skill_table[gsn_pounce].skill_level[ch->clss]))
	{
		if (!IS_CONTROLLED(ch))
		{
			ch->println("You do not know how to pounce!");
			return;
		}
	}
	
	if (arg[0] == '\0')
	{
		victim = ch->fighting;
		if (victim == NULL)
		{
			ch->println("But you aren't fighting anyone!");
			return;
		}
	}
	
	else if ((victim = get_char_room(ch,arg)) == NULL)
	{
		ch->println("They aren't here.");
		return;
	}
	
	if (victim == ch)
	{
		ch->println("You leap up, realizing the futility of pouncing on yourself.");
		return;
	}

	if ( IS_AFFECTED(ch, AFF_FLYING) )
	{
		ch->println("You cannot pounce from your mid-air position.");
		return;
	}

	if ( !IS_AFFECTED(victim, AFF_FLYING) )
	{
		ch->println("There is no need to exhaust yourself. They are not flying.");
		return;
	}
	
	if (is_safe(ch,victim))
		return;
	
	if ( !can_initiate_combat( ch, victim, CIT_GENERAL )) return;
    
	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }
	
    // Modifiers
	beats = skill_table[gsn_pounce].beats;
    if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 15;
    else
		chance += (ch->size - victim->size) * 10; 
	
    // Stats
    chance += ch->modifiers[STAT_AG];
	chance += ch->modifiers[STAT_QU] / 2;
    chance -= victim->modifiers[STAT_QU];
    chance -= victim->modifiers[STAT_AG];
	chance -= victim->modifiers[STAT_IN] / 3;
    
	// Speed
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
		chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
		chance -= 30;

	// Affects
	if ( IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
		chance +=20;
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) && !IS_AFFECTED(victim, AFF_DETECT_INVIS) )
		chance += 15;
	if (   IS_AFFECTED(victim, AFF_BLIND) 
		|| IS_AFFECTED(victim, AFF_POISON) 
		|| IS_AFFECTED(victim, AFF_PLAGUE) )
		chance += 10;
	if ( is_affected(victim, AFF_FAERIE_FIRE) )
		chance += 10;
	if ( is_affected(victim, AFF_SLOW) || IS_AFFECTED(victim, AFF_WEAKEN) )
		chance += 10;
	if ( is_affected(victim, AFF_BLIND) )
		chance -= 15;
	if ( is_affected(victim, AFF_SLOW) )
		chance -= 10;
	if ( is_affected(victim, AFF_POISON) || IS_AFFECTED(victim, AFF_PLAGUE) )
		chance -= 10;

	if ( ch->position == POS_FIGHTING )
		chance -= 30;
	else
		beats /= 2;
	
    // Level
    chance += (ch->level - victim->level);
		
    if (!IS_NPC(victim) && chance < get_skill(victim,gsn_dodge) )
		chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
	
    /* now the attack */
    if (number_percent() < chance )
    {	
		act("$n pounces onto you, sending you tumbling to the ground!",
			ch,NULL,victim,TO_VICT);
		act("You pounce onto $N, and send $M tumbling to the ground!",ch,NULL,victim,TO_CHAR);
		act("$n pounces onto $N, sending them tumbling to the ground.",
			ch,NULL,victim,TO_NOTVICT);
		check_improve(ch,gsn_pounce,true,1);
		
		landchar(victim);
		damage(ch,victim,0,gsn_pounce,DAM_BASH,false);
		WAIT_STATE(ch,beats);
    }
	else
	{
		damage(ch,victim,0,gsn_bash,DAM_BASH,false);
		act("You completely miss and fall flat on your face!",
			ch,NULL,victim,TO_CHAR);
		act("$n leaps into the air, only to fall flat on $s face a few moments later.",
			ch,NULL,victim,TO_NOTVICT);
		act("You evade $n's pounce, causing $m to fall flat on $s face.",
			ch,NULL,victim,TO_VICT);
		check_improve(ch,gsn_pounce,false,1);
		ch->position = POS_RESTING;
		damage(ch,victim,0,gsn_pounce,DAM_BASH,false);
		WAIT_STATE(ch,beats * 2); 
    }
}

/**************************************************************************/
// - Tibault Jun 2000
void do_worship(char_data *ch, char *argument)
{
	char arg[MIL];
	DEITY_DATA *pDeity;

	argument = one_argument(argument, arg);

	// Check input and classflags.
	if ( arg == '\0' )
	{
		ch->println( "`#`WSyntax:`^ worship <deity>" );
		return;
	}
	if ( IS_NPC(ch) || !HAS_CLASSFLAG( ch, CLASSFLAG_DEITIES ) )
	{
		ch->println( "The deity you have chosen to worship ignores you." );
		ch->println( "Perhaps you should have chosen a holier path to walk" );
		return;
	}
	if ( (pDeity = deity_lookup(arg)) == NULL ) 
	{
		ch->printlnf( "`#`Y%s`^ is not a valid deity.", arg);
		return;
	}

	// Check deity allowances
	if ( (ch->deity != NULL) && (ch->deity->name != NULL) ) {
		ch->printlnf( "You already worship the deity %s.", ch->deity->name );
		return;
	}
	if ( !IS_SETn( pDeity->race_allow_n, ch->race) ) {
		ch->printlnf( "%s looks down upon your race.", pDeity->name);
		return;
	}
	// Alignment checks.
	if ( ch->alliance > 1 && !IS_SET( pDeity->alignflags, ALIGN_GOOD) ) {
		ch->printlnf( "%s has no need for your rightiousness.", pDeity->name);
		return;
	} else if ( ch->alliance < -1 && !IS_SET( pDeity->alignflags, ALIGN_EVIL) ) {
		ch->printlnf( "%s senses the evil within you and ignores your prayers.", 
			pDeity->name);
		return;
	} else if (!IS_SET( pDeity->alignflags, ALIGN_NEUTRAL) ) {
		ch->printlnf( "%s cares little for your neutral path.", pDeity->name);
		return;
	}
	// Tendency checks.
	if ( IS_TEND_LAWFUL(ch) && !IS_SET( pDeity->tendflags, TENDFLAG_LAWFUL) ) {
		ch->printlnf( "%s does not share your lawful attitute, and seeks followers elsewhere",
			pDeity->name);
		return;
	} else if ( IS_TEND_CHAOTIC(ch) && !IS_SET( pDeity->tendflags, TENDFLAG_CHAOTIC) ) {
		ch->printlnf( "%s detects the chaos within your heart, and ignores your prayers.",
			pDeity->name);
		return;
	} else if ( !IS_SET( pDeity->tendflags, TENDFLAG_NEUTRAL) ) {
		ch->printlnf( "%s frowns upon your inability to choose between order and chaos.",
			pDeity->name);
		return;
	}
	// Check whether the player choose this himself and whether he typed confirm.
	if ( IS_CONTROLLED(ch) )
	{
		ch->println( "The gods sense this worship is not completely voluntary." );
		return;
	}

	ch->printlnf( "%s accepts you as a follower.", pDeity->name);
	ch->printlnf( "You now worship `#`W%s`^.", pDeity->name);
	ch->deity = pDeity;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

