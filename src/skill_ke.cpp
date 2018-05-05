#/**************************************************************************/
// skill_ke.cpp - skills written by Kerenos
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"
#include "o_lookup.h"
#include "db.h"
#include "ictime.h"

// Prototype declarations
HERB_DATA *find_herb( char_data * ch, char *herb );
void	entangle( char_data *ch, char_data *victim );
void    dismount( char_data *);
void	flourish_totemstaff_guardian( char_data *ch, OBJ_DATA *obj );
void	flourish_totemstaff_spirit(   char_data *ch, OBJ_DATA *obj );
void	forage_food( char_data *ch );
void	forage_herbs( char_data *ch );
int		get_birthmonth( char_data *ch );
int		calculate_season( void );
char *format_obj_to_char_new( OBJ_DATA *obj, char_data *ch, bool fShort );
DECLARE_DO_FUN( do_cast );
char *	const	month_name	[] =
{
    "Winter", "the Winter Storm", "the Frost Blight", 
	"the Return","Blight","the Dragon",
	"Light",  "the Sun", "the Heat", 
	"the Great War", "the Shadows", "the Long Shadows", 
};

/**************************************************************************/
// Kerenos - Sept 98
void do_forage( char_data *ch, char *argument )
{
	if ( IS_NULLSTR( argument ))
	{
		ch->println( "Syntax:  forage [food/herbs]" );
		return;
	}
	if ( !str_prefix( argument, "food" ))
	{
		forage_food( ch );
		return;
	}
	if ( !str_prefix( argument, "herbs" ))
	{
		forage_herbs( ch );
		return;
	}

	ch->println( "Syntax:  forage [food/herbs]" );
	return;	
}

/**************************************************************************/
void forage_food( char_data *ch )
{
	OBJ_INDEX_DATA		*pObjIndex;
	OBJ_DATA			*pObj;
	int					chance;
	
	chance = get_skill( ch, gsn_forage )+10;
	if ( chance < 11){
		ch->println("You don't know how to forage for food.");
		return;
	}

	// make it easier for newbies
	if (ch->level<30)
		chance+=(30 - ch->level); //Arch: scale in a way that makes sense..
	
	
	if ( !(ch->in_room->sector_type == SECT_FOREST
		|| ch->in_room->sector_type == SECT_FIELD
		|| ch->in_room->sector_type == SECT_HILLS
		|| ch->in_room->sector_type == SECT_MOUNTAIN
		|| ch->in_room->sector_type == SECT_WATER_SWIM
		|| ch->in_room->sector_type == SECT_DESERT )) {
		ch->println("You cannot find any foodstuffs around here.");
		return;
	}

	if (chance<number_range(0,105)){
		ch->wrapln("You forage around for something to eat but didnt manage to find "
			"any suitable foodstuffs here.");
#ifdef unix	
        WAIT_STATE( ch, PULSE_PER_SECOND *number_range(3,5));
#endif
		return;
	}

	
	if (( pObjIndex = get_obj_index( OBJ_VNUM_MUSHROOM )) == NULL ) {
		ch->println("Food item non-existant, please report this with a note to admin.");
		return;
	}
	
	pObj = create_object( pObjIndex);
	
	//  Free up old object names and descriptions
	
	free_string( pObj->description );
	free_string( pObj->name );
	free_string( pObj->short_descr );
	
	//  Make food timer 24 hours for perishable foods, no stockpiling allowed :)
	
	pObj->timer = 24;
	pObj->value[0] = 1; //Archiel: Let the items have nutritional content!
	pObj->value[1] = 1;
	
	//  Change food desc according to terrain type
	
	switch ( ch->in_room->sector_type ) {
		
	case SECT_FIELD:
		pObj->description = str_dup( "You see a bunch of eldeberries here." );
		pObj->short_descr = str_dup( "a bunch of eldeberries" );
		pObj->name		  = str_dup( "eldeberries bunch" );
		break;
	case SECT_FOREST:
		pObj->description = str_dup( "You see a handful of hazelnuts." );
		pObj->short_descr = str_dup( "a handful of hazelnuts" );
		pObj->name		  = str_dup( "hazelnuts" );
		break;
	case SECT_HILLS:
		pObj->description = str_dup( "Some wild berries are here." );
		pObj->short_descr = str_dup( "some wild berries" );
		pObj->name		  = str_dup( "berries wild" );
		break;
	case SECT_MOUNTAIN:
		pObj->description = str_dup( "You see a tangy wild root." );
		pObj->short_descr = str_dup( "a tangy wild root" );
		pObj->name		  = str_dup( "root wild tangy" );
		break;
	case SECT_WATER_SWIM:
		pObj->description = str_dup( "You look upon a fresh, dead trout." );
		pObj->short_descr = str_dup( "a fresh, dead trout" );
		pObj->name		  = str_dup( "trout" );
		break;
	case SECT_DESERT:
		pObj->description = str_dup( "You see a pulpy cactus heart here, looks good enough to eat." );
		pObj->short_descr = str_dup( "the heart of a cactus" );
		pObj->name		  = str_dup( "cactus heart pulp" );
		break;
	}
	
	//  Send the message to the world!
#ifdef unix	
	WAIT_STATE( ch, PULSE_PER_SECOND * number_range(6, 10));
#endif
	obj_to_room( pObj, ch->in_room );
	
	ch->printlnf("You found %s to eat.", pObj->short_descr);
	act( "$n seems to be searching for something.", ch, NULL, NULL, TO_ROOM );
	
	check_improve( ch, gsn_forage, true, 2 );
	return;
}

/**************************************************************************/
void forage_herbs( char_data *ch )
{
	OBJ_INDEX_DATA		*pHerb;
	OBJ_DATA			*herb_obj;
	HERB_DATA			*herb;
	int					vnum, chance, num = 0;
	static vn_int		vnum_field[500]; // show me a mud ever with more than 500 herbs
	bool				found = false;
	
	chance = get_skill( ch, gsn_forage );

	if ( chance < 1 ){
		ch->println( "You don't know how to forage." );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_BLIND) ) {
		ch->println( "You can't see a thing." );
		return;
	}

	for( herb = herb_list; herb; herb = herb->next )
	{
		if ( ch->in_room->sector_type != herb->sector )
			continue;

		if ( herb->continent )
		{
			if ( ch->in_room->area->continent != herb->continent ){
				continue;
			}
		}

		if ( herb->area >= 0 )
		{
			if ( ch->in_room->area->vnum != herb->area )
				continue;
		}

		if ( herb->season >=0 )
		{
			if ( calculate_season() != herb->season )
				continue;
		}

		if ( timefield_table[herb->timefield].lowhour != -1 )
		{
			if (   time_info.hour < timefield_table[herb->timefield].lowhour
				|| time_info.hour > timefield_table[herb->timefield].highhour )
				continue;
		}
		
		if (( chance + modifier_table[herb->difficulty].modifier ) < number_percent())
			continue;

		found = true;
		vnum_field[num] = herb->vnum_result;
		num++;
	}

	
	int beats_lag;

	if(skill_table[gsn_forage].beats){
		beats_lag=skill_table[gsn_forage].beats;
	}else{
		beats_lag=number_range(6*PULSE_PER_SECOND, 10*PULSE_PER_SECOND);
	}
	
	ch->println( "You look around for herbs..." );
	WAIT_STATE( ch, beats_lag);	

	if ( !found )
	{
		ch->println(beats_lag/PULSE_PER_SECOND, "You didn't find anything that resembled a useful herb." );
		return;
	}
				
	vnum = vnum_field[number_range( 0, num-1 )];
	
	if (( pHerb = get_obj_index( vnum )) == NULL ) {
		ch->println(beats_lag/PULSE_PER_SECOND, "You didn't find anything that resembled a useful herb." );
		return;
	}
	
	herb_obj = create_object( pHerb);
//??herb_obj->timer = 96;
	obj_to_char( herb_obj, ch );

	ch->printlnf( beats_lag/PULSE_PER_SECOND, "You found %s!", format_obj_to_char_new( herb_obj, ch, true));

	// act is seen before the player finds out they found something - but will do for now :)
	act( "$n has found an herb.", ch, NULL, NULL, TO_ROOM ); 
	check_improve( ch, gsn_forage, true, 2 );
	return;
}

/**************************************************************************/
void do_cook( char_data *ch, char *argument )
{
	OBJ_DATA		*obj;
	OBJ_DATA		*fire;
	OBJ_DATA		*cookware;
	ROOM_INDEX_DATA	*pIndexRoom = ch->in_room;
	char			arg[MIL];
	int				chance;
	bool			foundfire = false;
	bool			foundpan  = false;


	chance = get_skill( ch, gsn_cook ) + 30;
	if ( chance < 31){
		ch->println("You don't know the first thing about cooking.");
		return;
	}

	one_argument( argument, arg );

	if IS_NULLSTR( arg ) {
		ch->println("What do you wish to cook?");
		return;
	}

	if (( obj = get_obj_carry( ch, arg )) == NULL ) {
		ch->println("You are not carrying that.");
		return;
	}

	if ( obj->item_type != ITEM_FOOD ) {
		ch->println("You can only cook foodstuffs.");
		return;
	}

	if ( obj->timer == 0 ) {
		act( "$p appears to be cooked already.", ch, obj, NULL, TO_CHAR );
		return;
	}

	for ( fire = pIndexRoom->contents; fire; fire = fire->next_content ) {
		if ( fire->pIndexData->vnum == OBJ_VNUM_FIRE ) {
			foundfire = true;
			break;
		}
    }

	if ( !foundfire ) {
		ch->println("There needs to be a fire for you to cook properly.");
		return;
	}

    for ( cookware = ch->carrying; cookware; cookware = cookware->next_content )
    {
        if ( !str_cmp( cookware->pIndexData->material, "cookware" )
			 && cookware->wear_loc == WEAR_HOLD ) {
			foundpan = true;
            break;
		}
    }

	if ( !foundpan ) {
		ch->println("You need to hold a cooking pot of some kind to cook the food.");
		return;
	}

	if ( chance < number_percent() ) {
		act( "You fail miserably attempting to cook $p, ruining it completely.", ch, obj, NULL, TO_CHAR );
		extract_obj( obj );		
		check_improve( ch, gsn_cook, false, 3 );
		WAIT_STATE( ch, skill_table[gsn_cook].beats/2 );
		return;
	}

	act( "You busy yourself cooking $p, and do it to perfection!", ch, obj, NULL, TO_CHAR );
	act( "$n busies $mself cooking $p.", ch, obj, NULL, TO_ROOM );
	obj->timer = 0;

	if ( obj->pIndexData->vnum == OBJ_VNUM_SLICE ) {
		replace_string( obj->description, "A slice of cooked meat lies here." );
		replace_string( obj->short_descr, "a slice of cooked meat" );
		replace_string( obj->name, "slice meat cooked" );
	}	
	check_improve( ch, gsn_cook, true, 4 );
	WAIT_STATE( ch, skill_table[gsn_cook].beats );
	return;
}
/**************************************************************************/
void do_flip( char_data *ch, char *argument )
{
	int				face = number_range( 0, 1);
	bool			spam = false;
	OBJ_INDEX_DATA	*coinIndex;
	OBJ_DATA		*coin;

	if ( IS_NULLSTR( argument )) {
		ch->println("What do you want to flip, a silver or a gold coin?");
		return;
	}

	if ( ch->desc && ch->desc->repeat>5 )
		spam = true;

	if ( is_name( argument, "silver" ))
	{
		if ( ch->silver < 1 ) {
			ch->println("You have no silver coins to flip.");
			return;
		}

		coinIndex	= get_obj_index( OBJ_VNUM_SILVER_ONE );
		coin		= create_object( coinIndex);
		ch->silver -= 1;
		free_string( coin->description );

		if ( face ) {
			act( "$n flips a coin high into the air and lands showing heads.", ch, NULL, NULL, TO_ROOM );
			ch->println("You flip a coin high into the air.  It lands showing heads.");
			coin->description = str_dup( "You see a silver coin showing heads." );
		} else {
			act( "$n flips a coin high into the air and lands showing tails.", ch, NULL, NULL, TO_ROOM );
			ch->println("You flip a coin high into the air.  It lands showing tails.");
			coin->description = str_dup( "You see a silver coin showing tails." );
		}
		if ( !IS_OOC(ch) ) { obj_to_room( coin, ch->in_room ); }
	}
	else if ( is_name( argument, "gold" ))
	{
		if ( ch->gold < 1 ) {
			ch->println("You have no gold coins to flip.");
			return;
		}

		coinIndex	= get_obj_index( OBJ_VNUM_GOLD_ONE );
		coin		= create_object( coinIndex);
		ch->gold   -= 1;
		free_string( coin->description );

		if ( face ) {
			act( "$n flips a coin high into the air and lands showing heads.", ch, NULL, NULL, TO_ROOM );
			ch->println("You flip a coin high into the air.  It lands showing heads.");
			coin->description = str_dup( "You see a gold coin showing heads." );
		} else {
			act( "$n flips a coin high into the air and lands showing tails.", ch, NULL, NULL, TO_ROOM );
			ch->println("You flip a coin high into the air.  It lands showing tails.");
			coin->description = str_dup( "You see a gold coin showing tails." );
		}
		if ( !IS_OOC(ch)) { obj_to_room( coin, ch->in_room ); }
	}
	else
	{
		ch->println("What do you want to flip, a silver or a gold coin?");
	}
	if ( spam )
		WAIT_STATE( ch, PULSE_PER_SECOND * 10 );	
}
/**************************************************************************/
void do_hobble( char_data *ch, char *argument )
{
    char_data * victim;
    OBJ_DATA  * obj;
    char        arg[MIL];
    int         chance;

	if ((chance = get_skill(ch,gsn_hobble)) == 0)
	{
		do_huh(ch,"");
		return;
	}

    one_argument( argument, arg );

    obj = ( get_eq_char( ch, WEAR_WIELD ));

    if ( !obj ) {
        ch->println("You need to wield a mace to hobble someone.");
        return;
    }
    
	if ( obj ) {
        if ( obj->value[0] != WEAPON_MACE ) {
            ch->println("You are not wielding a mace.");
            return;
        }
    }

	if( IS_RIDING( ch )) {
		ch->println("You cannot reach your opponents legs while mounted.");
		return;
	}
    if ( IS_NULLSTR( arg )) {
        if (( victim = ch->fighting ) == NULL ) {
            ch->println("But you aren't fighting anyone!");
            return;
        }
    }
    else if (( victim = get_char_room( ch, arg )) == NULL ) {
        ch->printlnf("You can't seem to find '%s' here.", arg);
        return;
    }

 	if(ch == victim)
 	{
     	    ch->println("Yeah, right.");
      	   return;
    	}

	if ( is_safe( ch, victim ))
		return;

	if ( !can_initiate_combat( ch, victim, CIT_GENERAL )) return;	
	
    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }


// INSERT WEAPON SPECIALIZATION CHECK HERE

    if ( get_skill( ch,gsn_hobble ) > number_percent() ) {
        act( "You `Bsmash`x $p against $N's legs!", ch, obj, victim, TO_CHAR );
		act( "$n `Bsmashes`x $p against $N's legs!", ch, obj, victim, TO_NOTVICT );
		act( "$n `Bsmashes`x $p on your legs!", ch, obj, victim, TO_VICT );
        if ( damage( ch, victim, number_range( 1, ch->level / 2 ), gsn_hobble, DAM_BASH, true)) {
	        victim->position = POS_RESTING;
		    DAZE_STATE( victim, skill_table[gsn_hobble].beats*2/3 );
			victim->move -= number_range(( ch->level/5 ), ch->level);
		}
        check_improve( ch, gsn_hobble, true, 1 );
        WAIT_STATE( ch, skill_table[gsn_hobble].beats );
	}
    else {
        damage( ch, victim, 0, gsn_hobble, DAM_BASH, false );
        check_improve( ch, gsn_hobble, false, 1 );

		ch->println("You swing wildly and don't manage to connect the blow.");

        act("$n tries to hobble $N but misses completely.", ch, NULL, victim, TO_NOTVICT);
        act("You deftly evade $n's attempt to hobble you.", ch, NULL, victim, TO_VICT );
        WAIT_STATE( ch, skill_table[gsn_hobble].beats );
    }
    return;
}

/**************************************************************************/
void do_shieldcleave( char_data *ch, char *argument )
{
    char_data * victim;
    OBJ_DATA  * obj;
	OBJ_DATA  * shield;
    char        arg[MIL];
    int         chance;


	if ((chance = get_skill(ch,gsn_shieldcleave)) == 0)
	{
		do_huh(ch,"");
		return;
	}

    one_argument( argument, arg );

    obj = ( get_eq_char( ch, WEAR_WIELD ));

    if ( !obj ) {
        ch->println("You need to wield an axe to cleave a shield.");
        return;
    }
    
	if ( obj ) {
        if ( obj->value[0] != WEAPON_AXE ) {
            ch->println("You are not wielding an axe.");
            return;
        }
    }

    if (( victim = ch->fighting ) == NULL ) {
		ch->println("You aren't fighting anyone.");
		return;
    }

 	if(ch == victim)
 	{
     	    ch->println("Yeah, right.");
      	   return;
    	}

	shield = ( get_eq_char( victim, WEAR_SHIELD ));

	if ( !shield ) {
		ch->println("Your opponent is not wearing a shield.");
		return;
	}

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }

	if ( is_safe( ch, victim ))
		return;

// INSERT WEAPON SPECIALIZATION CHECK HERE

    if ( get_skill( ch,gsn_shieldcleave ) > number_percent() ) {

		chance = 40;
		if IS_WEAPON_STAT( obj, WEAPON_SHARP )		chance += 10;
		if IS_WEAPON_STAT( obj, WEAPON_VORPAL )		chance += 20;
		if IS_WEAPON_STAT( obj, WEAPON_FROST )		chance += 10;
		if IS_WEAPON_STAT( obj, WEAPON_FLAMING )	chance += 5;
		if IS_WEAPON_STAT( obj, WEAPON_SHOCKING )	chance += 5;
		if IS_OBJ_STAT( shield,	OBJEXTRA_BLESS )		chance -= 25;
		if IS_OBJ_STAT( shield,	OBJEXTRA_MAGIC )		chance -= 20;
		if IS_OBJ_STAT( shield,	OBJEXTRA_GLOW )			chance -=  5;
		if IS_OBJ_STAT( shield,	OBJEXTRA_CHAOS )		chance -= number_range(1,25);
		if IS_OBJ_STAT( shield,	OBJEXTRA_HUM )			chance -=  5;
		if IS_OBJ_STAT( shield,	OBJEXTRA_NONMETAL )		chance += 10;
		if IS_OBJ_STAT( shield,	OBJEXTRA_BURN_PROOF )	chance -= 35;

		if ( number_range( 1, 100 ) < chance ) {
			ch->println("Your axe `Bslices`x through the shield like butter.");
	        act("$n `Bslices`x right through $N's shield.", ch, NULL, victim, TO_NOTVICT);
	        act("$n has `Bdestroyed`x your shield.", ch, NULL, victim, TO_VICT);
			extract_obj( shield );	
		} else {
	        act( "$N's shield withstands your blow!", ch, obj, victim, TO_CHAR );
			act( "$n smashes against $N's shield without effect!", ch, obj, victim, TO_NOTVICT );
			act( "Your shield resonates agains $N's swing, but holds firm.", ch, obj, victim, TO_VICT );
		}
		check_improve( ch, gsn_shieldcleave, true, 1 );
        WAIT_STATE( ch, skill_table[gsn_shieldcleave].beats );
	}
    else {
        check_improve( ch, gsn_shieldcleave, false, 1 );

		ch->println("You swing wildly and don't manage to connect the blow.");
        act("$n tries to cleave $N's shield and misses completely.", ch, NULL, victim, TO_NOTVICT);
        victim->println("You manage to avoid your shield being cleaved in two.");
        WAIT_STATE( ch, skill_table[gsn_shieldcleave].beats );
    }
    return;
}
/**************************************************************************/
void do_hurl( char_data *ch, char *argument )
{
	char		arg[MIL];
	char_data	*victim = NULL;
	OBJ_DATA	*dagger = NULL;
	int			dam;
	int			chance;


	chance = get_skill( ch, gsn_hurl );
	if ( chance < 1)
	{
		do_huh(ch,"");
		return;
	}


	dagger = get_eq_char( ch, WEAR_WIELD );


	if ( !dagger )
	{
		ch->println("You have to have your dagger readied in your hand.");
		return;
	}

	if ( dagger->value[0] != WEAPON_DAGGER )
	{
		ch->println("You may only hurl daggers.");
		return;
	}

    if ( !can_drop_obj( ch, dagger ))
    {
		ch->println("You can't let go of that.");
		return;
    }

	one_argument( argument, arg );

	if ( arg[0] == '\0')
	{
		victim = ch->fighting;
		if ( victim == NULL )
		{
			ch->println("But you aren't fighting anyone!");
			return;
		}
	}
	else if (( victim = get_char_room( ch, arg )) == NULL )
	{
		ch->println("They aren't here.");
		return;
	}

 	if(ch == victim)
 	{
     	    ch->println("Yeah, right.");
      	   return;
    	}

	if ( is_safe( ch, victim ))
		return;

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }


	WAIT_STATE( ch, skill_table[gsn_hurl].beats );

	act( "$n hurls $p at $N.",  ch, dagger, victim, TO_NOTVICT );
	act( "You hurl $p at $N.",  ch, dagger, victim, TO_CHAR );
	act( "$n hurls $p at you.", ch, dagger, victim, TO_VICT );

	if (( ch->level + 5 ) < dagger->level
	||  number_percent() >= 10 + get_skill( ch, gsn_hurl ) * 4/5 )
	{
		act( "$p misses $N and lands on the ground.", ch, dagger, victim, TO_CHAR );
		act( "$n hurls $p but it misses and lands on the ground.", ch, dagger, NULL, TO_ROOM );
		obj_from_char( dagger );
		obj_to_room( dagger, ch->in_room );
		check_improve( ch, gsn_hurl, false, 1 );
	    if ( IS_OBJ_STAT( dagger, OBJEXTRA_MELT_DROP ))
	    {
			act( "$p dissolves into smoke.", ch, dagger, NULL, TO_ROOM );
			extract_obj( dagger );
		}
	}
	else
	{
		chance = number_range(1,10);
		switch (chance)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				obj_from_char( dagger );
				dam = dice( dagger->value[1], dagger->value[2] );
				if (damage( ch, victim, dam, gsn_hurl, DAM_PIERCE, true ))
				{

					// after damage() victim can be invalidated (if the damage slayed them)

					if ( !IS_SAME_ROOM(victim,ch) || get_eq_char( victim, WEAR_LODGED_ARM ) != NULL ) {
						obj_to_room( dagger, ch->in_room );
						act( "$p doesn't lodge into your arm.", ch, dagger, victim, TO_VICT );
						act( "$p hurts $N but doesn't successfully lodge.", ch, dagger, victim, TO_CHAR );
					    if ( IS_OBJ_STAT( dagger, OBJEXTRA_MELT_DROP ))
						{
							act( "$p dissolves into smoke.", ch, dagger, NULL, TO_ROOM );
							extract_obj( dagger );
						}
					}
					else
					{
						obj_to_char( dagger, victim );
						SET_BIT( dagger->extra_flags, OBJEXTRA_LODGED );
						dagger->wear_flags = OBJWEAR_TAKE | OBJWEAR_LODGED_ARM;
						wear_obj( victim, dagger, true, false);
						dagger->wear_flags = OBJWEAR_TAKE | OBJWEAR_WIELD;
					}
				}
				else
				{
					obj_to_room( dagger, ch->in_room );
				    if ( IS_OBJ_STAT( dagger, OBJEXTRA_MELT_DROP ))
					{
						act( "$p dissolves into smoke.", ch, dagger, NULL, TO_ROOM );
						extract_obj( dagger );
					}
				}
				check_improve( ch, gsn_hurl, true, 1 );
				break;
			case 6:
			case 7:
			case 8:
				obj_from_char( dagger );
				dam = 3 * ( dice( dagger->value[1], dagger->value[2] )/2);
				if (damage( ch, victim, dam, gsn_hurl, DAM_PIERCE, true ))
				{

					// after damage() victim can be invalidated (if the damage slayed them)

					if ( !IS_SAME_ROOM(victim,ch) || get_eq_char( victim, WEAR_LODGED_LEG ) != NULL ) {
						obj_to_room( dagger, ch->in_room );
						act( "$p doesn't lodge into your leg.", ch, dagger, victim, TO_VICT );
						act( "$p hurts $N but doesn't successfully lodge.", ch, dagger, victim, TO_CHAR );
					    if ( IS_OBJ_STAT( dagger, OBJEXTRA_MELT_DROP ))
						{
							act( "$p dissolves into smoke.", ch, dagger, NULL, TO_ROOM );
							extract_obj( dagger );
						}
					}
					else
					{
						obj_to_char( dagger, victim );
						SET_BIT( dagger->extra_flags, OBJEXTRA_LODGED );
						dagger->wear_flags = OBJWEAR_TAKE | OBJWEAR_LODGED_LEG;
						wear_obj( victim, dagger, true, false);
						dagger->wear_flags = OBJWEAR_TAKE | OBJWEAR_WIELD;
					}
				}
				else
				{
					obj_to_room( dagger, ch->in_room );
				    if ( IS_OBJ_STAT( dagger, OBJEXTRA_MELT_DROP ))
					{
						act( "$p dissolves into smoke.", ch, dagger, NULL, TO_ROOM );
						extract_obj( dagger );
					}
				}
				check_improve( ch, gsn_hurl, true, 1 );
				break;
			case 9:
			case 10:
				obj_from_char( dagger );
				dam = 2 * ( dice( dagger->value[1], dagger->value[2] ));
				if (damage( ch, victim, dam, gsn_hurl, DAM_PIERCE, true ))
				{
					// after damage() victim can be invalidated (if the damage slayed them)

					if ( !IS_SAME_ROOM(victim,ch) || get_eq_char( victim, WEAR_LODGED_RIB ) != NULL ) {
						obj_to_room( dagger, ch->in_room );
						act( "$p doesn't lodge into your rib.", ch, dagger, victim, TO_VICT );
						act( "$p hurts $N but doesn't successfully lodge.", ch, dagger, victim, TO_CHAR );
					    if ( IS_OBJ_STAT( dagger, OBJEXTRA_MELT_DROP ))
						{
							act( "$p dissolves into smoke.", ch, dagger, NULL, TO_ROOM );
							extract_obj( dagger );
						}
					}
					else
					{
						obj_to_char( dagger, victim );
						SET_BIT( dagger->extra_flags, OBJEXTRA_LODGED );
						dagger->wear_flags = OBJWEAR_TAKE | OBJWEAR_LODGED_RIB;
						wear_obj( victim, dagger, true, false);
						dagger->wear_flags = OBJWEAR_TAKE | OBJWEAR_WIELD;
					}
				}
				else
				{
					obj_to_room( dagger, ch->in_room );
				    if ( IS_OBJ_STAT( dagger, OBJEXTRA_MELT_DROP ))
					{
						act( "$p dissolves into smoke.", ch, dagger, NULL, TO_ROOM );
						extract_obj( dagger );
					}
				}
				check_improve( ch, gsn_hurl, true, 1 );
				break;
		}
	}
	return;
}
/**************************************************************************/	
void do_entangle( char_data *ch, char *)
{
    char_data *victim;
    OBJ_DATA *whip, *vObj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill(ch,gsn_entangle)) == 0)
    {
		do_huh(ch,"");
		return;
    }

	whip = get_eq_char( ch, WEAR_WIELD );

	if ( !whip )
	{
		ch->println("You must be wielding a whip to entangle.");
		return;
	}
	if ( whip->value[0] != WEAPON_WHIP )
	{
		ch->println("You must be wielding a whip to entangle.");
		return;
	}

    if (( victim = ch->fighting ) == NULL )
    {
		ch->println("You aren't fighting anyone.");
		return;
    }

    if (( vObj = get_eq_char( victim, WEAR_WIELD ) ) == NULL ) 
	if (( vObj = get_eq_char( victim, WEAR_SECONDARY) ) == NULL )
    {
		ch->println("Your opponent is not wielding a weapon.");
		return;
    }

 	if(ch == victim)
 	{
     	    ch->println("Yeah, right.");
      	   return;
    	}

    // find weapon skills
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    // skill 
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    // quickness/agility  vs. strength 
    chance += (ch->modifiers[STAT_QU] + ch->modifiers[STAT_AG])/2;
    chance -= 2 * victim->modifiers[STAT_ST];

    // level 
    chance += (ch->level - victim->level) * 2;
 
    // and now the attack 
    if (number_percent() < chance)
    {
		WAIT_STATE( ch, skill_table[gsn_entangle].beats );
		entangle( ch, victim );
		check_improve(ch,gsn_entangle,true,1);
    }
    else
    {
		WAIT_STATE(ch,skill_table[gsn_entangle].beats);
		act("You fail to grab their weapon.",ch,NULL,NULL,TO_CHAR);
		act("$n tries to entangle your weapon, but fails.",ch,NULL,victim,TO_VICT);
		act("$n tries to entangle $N's weapon, but fails.",ch,NULL,victim,TO_NOTVICT);
		check_improve(ch,gsn_entangle,false,1);
    }
    return;
}
/**************************************************************************/
void do_dervish( char_data *ch, char * )
{
	char_data	*pChar;
	char_data	*pChar_next = NULL;
	OBJ_DATA	*spear;
	bool		found = false;
	int			chance;
	
    if ( get_skill(ch,gsn_dervish) == 0 )
    {
		do_huh(ch,"");
		return;
    }
 
	spear = get_eq_char( ch, WEAR_WIELD );

	if ( !spear )
	{
		ch->println("You must be wielding a spear to perform this dance.");
		return;
	}
	if ( spear->value[0] != WEAPON_SPEAR )
	{
		ch->println("You must be wielding a spear to perform this dance.");
		return;
	}

	act( "$n holds $p firmly and begins to whirl about like a dervish...", ch, spear, NULL, TO_ROOM );
	act( "You hold $p firmly and begin to whirl like a maddened dervish...",  ch, spear, NULL, TO_CHAR );
   
	for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
	{
		chance = get_skill( ch, gsn_dervish );

		pChar_next = pChar->next_in_room;


		if ( is_safe( ch, pChar ))
			continue;

		if ( IS_AFFECTED( ch, AFF_CHARM )
		&& ch->master == pChar )
			continue;

		if ( IS_IMMORTAL( pChar )
		&& INVIS_LEVEL(pChar)>= LEVEL_IMMORTAL )
			continue;

		if ( is_same_group( ch, pChar ))
		{
			if ( number_percent() < 75 )
				continue;
		}
	
		if (number_percent() < chance)
		{
			act( "With a crazed look, $n turns towards YOU!", ch, NULL, pChar, TO_VICT );
			one_hit( ch, pChar, gsn_dervish, false );
			if ( pChar != ch )
				found = true;
		}
	}
   
	if( !found ){
		ch->println("You feel dizzy, and a tiny bit embarassed.");
		if (( number_percent() < 10 ) && !IS_IMMORTAL( ch ))
		{
			act( "$n drops $p.", ch, spear, NULL, TO_ROOM );
			act( "You drop $p.", ch, spear, NULL, TO_CHAR );
			obj_from_char( spear );
			obj_to_room( spear, ch->in_room );
			check_improve( ch, gsn_dervish, false, 1 );
		}
	}else{
		check_improve( ch, gsn_dervish, true, 1 );

	}

	WAIT_STATE( ch, skill_table[gsn_dervish].beats );
	ch->move = int( ch->move * 0.9 );

	if ( number_percent() < 25 ){
		act( "$n loses $s balance and falls into a heap.", ch, NULL, NULL, TO_ROOM );
		ch->println("You lose your balance and fall into a heap.");
		ch->position = POS_RESTING;
	}
	return;
}
/**************************************************************************/
void do_cutoff( char_data *ch, char * )
{
	if ( get_skill( ch, gsn_cutoff) == 0 ) {
		do_huh(ch,"");
		return;
	}

	if ( ch->position != POS_FIGHTING ) {
		ch->println("You can only use this during combat.");
		return;
	}

	if ( !IS_SET( ch->dyn, DYN_IS_CUTTING_OFF )) {
		ch->println("You will now cut off fleeing or retreating enemies.");
		SET_BIT( ch->dyn, DYN_IS_CUTTING_OFF );
	} else {
		ch->println("You will no longer cut off fleeing or retreating enemies.");
		REMOVE_BIT( ch->dyn, DYN_IS_CUTTING_OFF );
	}

	return;
}
/**************************************************************************/
void do_sheathe( char_data *ch, char * )
{
	OBJ_DATA	*weapon = get_eq_char( ch, WEAR_WIELD );

	if ( get_eq_char( ch, WEAR_SHEATHED ) != NULL )
	{
		ch->println("You already have a weapon sheathed, use draw to remove it.");
		return;
	}

	if ( !weapon )
	{
		ch->println("You must be wielding a weapon in order to sheathe it.");
		return;
	}

	if ( weapon->value[0] == WEAPON_EXOTIC
	||   weapon->value[0] == WEAPON_STAFF
        ||      weapon->value[0] == WEAPON_SPEAR
	||	 weapon->value[0] == WEAPON_POLEARM )
	{
		ch->println("You cannot sheathe that weapon.");
		return;
	}

	if ( IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS)) {
		act ( "$p is too large to be sheathed properly.", ch, weapon, NULL, TO_CHAR );
		return;
	}

	if ( !IS_IMMORTAL( ch) && IS_SET( weapon->extra_flags, OBJEXTRA_NOREMOVE) )
    {
		act( "You can't remove $p.", ch, weapon, NULL, TO_CHAR );
		return;
    }

	unequip_char( ch, weapon );
	weapon->wear_loc = WEAR_SHEATHED;
	act( "With a flourish, you sheathe $p.", ch, weapon, NULL, TO_CHAR );
	act( "With a flourish, $n sheathes $p.", ch, weapon, NULL, TO_ROOM );

	return;
}
/**************************************************************************/
DECLARE_DO_FUN( do_stand );
/**************************************************************************/
void do_draw( char_data *ch, char * )
{
	OBJ_DATA	*weapon = get_eq_char( ch, WEAR_SHEATHED );

	if ( !weapon )
	{
		ch->println("You don't have a weapon sheathed.");
		return;
	}

	if ( get_eq_char( ch, WEAR_WIELD ) != NULL )
	{
		ch->println("You must have a free hand to draw a weapon.");
		return;
	}

	if ( !IS_IMMORTAL(ch) && IS_SET( weapon->extra_flags, OBJEXTRA_NOREMOVE ))
    {
		act( "You can't remove $p.", ch, weapon, NULL, TO_CHAR );
		return;
    }

	if ( ch->position < POS_STANDING )
	{
		do_stand(ch, "");
		if(ch->position < POS_STANDING){
			return;
		}
	}

	equip_char( ch, weapon, WEAR_WIELD );
	act( "You draw $p with unequaled grace.", ch, weapon, NULL, TO_CHAR );
	act( "$n draws $p.", ch, weapon, NULL, TO_ROOM );

	return;
}
/**************************************************************************/
void do_conceal( char_data *ch, char * )
{
	OBJ_DATA	*weapon = get_eq_char( ch, WEAR_WIELD );

	if ( !IS_IMMORTAL( ch ))
	{
		if ( ch->specialization != SPECIALIZE_DAGGER )
		{
			do_huh(ch,"");
			return;
		}
	}

	if ( get_eq_char( ch, WEAR_CONCEALED ) != NULL )
	{
		ch->println("You may only have one weapon concealed.");
		return;
	}

	if ( !weapon )
	{
		ch->println("You must be wielding your dagger in order to conceal it.");
		return;
	}

	if ( weapon->value[0] != WEAPON_DAGGER )
	{
		ch->println("You can only conceal daggers.");
		return;
	}

	if ( !IS_IMMORTAL(ch) && IS_SET( weapon->extra_flags, OBJEXTRA_NOREMOVE) )
    {
		act( "You can't remove $p.", ch, weapon, NULL, TO_CHAR );
		return;
    }

	unequip_char( ch, weapon );
	weapon->wear_loc = WEAR_CONCEALED;
	act( "When no one is looking, you secretly conceal $p.", ch, weapon, NULL, TO_CHAR );

	return;
}
/**************************************************************************/
// CELRION
void do_charge( char_data *ch, char *argument )
{
	char			arg[MIL], dir[MIL];
	char_data		*victim		= NULL;
	char_data		*mount;
	OBJ_DATA		*polearm	= NULL;
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room	= NULL;
	EXIT_DATA *pexit;
	int				door;
	int				chance;
	int				dam;

	chance = get_skill( ch, gsn_charge );
	if ( chance < 1)
	{
		do_huh(ch,"");
		return;
	}

	if ( !IS_RIDING( ch ))
	{
		ch->println("You need to be mounted to charge.");
		return;
	}

	mount = ch->mounted_on;

	polearm = get_eq_char( ch, WEAR_WIELD );

	if ( !polearm )
	{
		ch->println("You need to wield a polearm first.");
		return;
	}

	if ( polearm->value[0] != WEAPON_POLEARM )
	{
		ch->println("You can only charge with polearms.");
		return;
	}

	//Insert Specialization check (somewhere near here at least)

	argument = one_argument ( argument, dir );
	one_argument ( argument, arg );

	if (arg[0] == '\0')
	{
		ch->println("Charge who or what?");
		return;
	}

	door = dir_lookup( dir );
	
    if ( door == -1 )
	{
		ch->printlnf("'%s' is an invalid direction.", dir); 
		ch->println("Syntax: charge <direction> <name>");
		return;
    }

	in_room = ch->in_room;

	if ( ( pexit   = in_room->exit[door] ) == NULL
		||   ( to_room = pexit->u1.to_room   ) == NULL 
		||   !can_see_room(ch,pexit->u1.to_room))
    {
		ch->println("Alas, you cannot go that way.");
		return;
    }

	if (IS_SET(pexit->exit_info, EX_CLOSED)
		&&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
		&&   !IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK))
    {
		
		act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
		return;
    }

	switch (door)
	{
		case 4:
			act( "$n charges upward!", ch, NULL, NULL, TO_ROOM);
			break;
		case 5:
			act( "$n charges down!", ch, NULL, NULL, TO_ROOM);
			break;
		default:
			act( "$n charges to the $T!", ch, NULL, dir_name[door], TO_ROOM);
			break;
	}

	act( "You charge $T!", ch, NULL, dir_name[door], TO_CHAR );
	
	char_from_room(ch);
	char_to_room(ch, to_room);
	char_from_room(mount);
	char_to_room(mount, to_room);

	if ((victim = get_char_room(ch,arg)) == NULL)
	{
		ch->println("They aren't here.");
		act( "$n charges into the room and looks around quizzically.", ch, NULL, NULL, TO_ROOM);
		return;
	}

 	if(ch == victim)
 	{
     	    ch->println("Yeah, right.");
      	   return;
    	}

	if( is_safe (ch, victim) )
		return;

	if ( !can_initiate_combat( ch, victim, CIT_GENERAL )) return;

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }

	//Stole chance mods from bash, I liked 'em.

    chance += ch->modifiers[STAT_ST];
    chance -= victim->modifiers[STAT_QU];
    chance -= GET_AC(victim,AC_PIERCE) /25;

    if (IS_SET(mount->off_flags,OFF_FAST) || IS_AFFECTED(mount,AFF_HASTE))
		chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
		chance -= 30;

    chance += (((ch->level + mount->level)/2) - victim->level);

    if (!IS_NPC(victim) && chance < get_skill(victim,gsn_dodge) )
		chance -= 3 * (get_skill(victim,gsn_dodge) - chance);

	//Move suckdown, also if tired then chance is slashed greatly.

	if( mount->move < mount->max_move/2 ) chance /= 2;
	if( !IS_NPC (ch) && ch->pcdata->tired > 16 ) chance /= 2;
	
	//Actually hitting at this point.

    if (number_percent() < chance )
    {
		act("$n charges into the room and hits you with $p!", ch,polearm, victim,TO_VICT);
		act("You charge into the room and slam $N!",ch,NULL,victim,TO_CHAR);
		act("$n charges into the room and slams right into $N!", ch,NULL,victim,TO_NOTVICT);
		check_improve(ch,gsn_charge,true,1);
		
		DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
		WAIT_STATE(ch,skill_table[gsn_charge].beats);
		if( IS_RIDING (victim))
		{
			if ( number_range(1,3) > 1)
			{
				dismount(victim);
				victim->position = POS_RESTING;
				act("You have been dismounted!", ch, NULL, victim, TO_VICT );
				act("$N has been dismounted!", ch, NULL, victim, TO_NOTVICT );
				act("You have dismounted $N!", ch, NULL, victim, TO_CHAR );
			}
		}

		dam = 3 * ( dice( polearm->value[1], polearm->value[2] ));
		dam	+= 2 * ch->size + chance/10;

		damage( ch, victim, dam, gsn_charge, DAM_PIERCE, true );

		//Stole damage from bash but moreso.
	}
	else
	{
		damage(ch,victim,0,gsn_charge,DAM_BASH,false);
		act("You charge into the room and miss $N completely!",
			ch,NULL,victim,TO_CHAR);
		act("$n charges at $N and completely misses!",
			ch,NULL,victim,TO_NOTVICT);
		act("$n charges right past you, in a vain attempt to hit you!",
			ch,NULL,victim,TO_VICT);
		check_improve(ch,gsn_charge,false,1);
		WAIT_STATE(ch,skill_table[gsn_charge].beats * 3/2); 
    }
	mount->move -= mount->move / 3;
}
/**************************************************************************/
void do_overhead( char_data *ch, char *argument )
{
    char_data * victim;
    OBJ_DATA  * axe;
	OBJ_DATA  * helm;
    char        arg[MIL];
    int         chance, dam;

	if ((chance = get_skill(ch,gsn_shieldcleave)) == 0)
	{
		do_huh(ch,"");
		return;
	}

    one_argument( argument, arg );

    axe = ( get_eq_char( ch, WEAR_WIELD ));

    if ( !axe ) {
        ch->println("You need to wield an axe to do an overhead swing.");
        return;
    }
    
	if ( axe ) {
        if ( axe->value[0] != WEAPON_AXE )
		{
			ch->println("You are not wielding an axe.");
            return;
        }
    }

    if (( victim = ch->fighting ) == NULL ) {
		ch->println("You aren't fighting anyone.");
		return;
    }

 	if(ch == victim)
 	{
     	    ch->println("Yeah, right.");
      	   return;
    	}

	helm = ( get_eq_char( victim, WEAR_HEAD ));
	
    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }

	if ( is_safe( ch, victim ))
		return;

// INSERT WEAPON SPECIALIZATION CHECK HERE

	dam = dice( axe->value[1], axe->value[2] );

	if ( helm )
	{
	    if ( get_skill( ch, gsn_overhead ) > number_percent() )
		{
			chance = 50;
			if IS_WEAPON_STAT( axe, WEAPON_SHARP )		chance += 10;
			if IS_WEAPON_STAT( axe, WEAPON_VORPAL )		chance += 20;
			if IS_WEAPON_STAT( axe, WEAPON_FROST )		chance += 10;
			if IS_WEAPON_STAT( axe, WEAPON_FLAMING )	chance += 5;
			if IS_WEAPON_STAT( axe, WEAPON_SHOCKING )	chance += 5;
			if IS_OBJ_STAT( helm,	OBJEXTRA_BLESS )		chance -= 25;
			if IS_OBJ_STAT( helm,	OBJEXTRA_MAGIC )		chance -= 20;
			if IS_OBJ_STAT( helm,	OBJEXTRA_GLOW )			chance -=  5;
			if IS_OBJ_STAT( helm,	OBJEXTRA_CHAOS )		chance -= number_range(1,25);
			if IS_OBJ_STAT( helm,	OBJEXTRA_HUM )			chance -=  5;
			if IS_OBJ_STAT( helm,	OBJEXTRA_NONMETAL )		chance += 10;
			if IS_OBJ_STAT( helm,	OBJEXTRA_BURN_PROOF )	chance -= 35;

			if ( number_range( 1, 100 ) < chance )
			{
				helm->condition -= axe->level;

				if ( helm->condition <= 0 )
				{
					act( "Your axe `Bslices`x through $p, destroying it.", ch, helm, NULL, TO_CHAR );
					act( "$n `Bslices`x $p.", ch, helm, NULL, TO_NOTVICT );
					act( "$n has `Bdestroyed`x $p.", ch, helm, victim, TO_VICT );
					extract_obj( helm );
					dam *= 3/2;
				}
				else
				{
					act( "Your axe dents $p worn by your opponent.", ch, helm, NULL, TO_CHAR );
					act( "$n dents your $p.", ch, helm, victim, TO_VICT );
					dam *= 3/4;
				}
			}
			else
			{
				act( "$N's helm remains unscathed!", ch, NULL, victim, TO_CHAR );
				act( "$p resonates agains $N's swing, but holds firm.", ch, axe, victim, TO_VICT );
				dam /= 2;
			}
			check_improve( ch, gsn_overhead, true, 1 );
			WAIT_STATE( ch, skill_table[gsn_overhead].beats );
		}
		else
		{
			check_improve( ch, gsn_overhead, false, 1 );
			act( "$N deftly evades your blow.", ch, NULL, victim, TO_CHAR );
			act( "$n tries to split $N in two but misses.", ch, NULL, victim, TO_NOTVICT);
			act( "$n tries to split you in two but misses.", ch, NULL, victim, TO_VICT );
	        WAIT_STATE( ch, skill_table[gsn_overhead].beats );
	    }
	}
	else
	{
	    if ( get_skill( ch, gsn_overhead ) > number_percent())
		{
			dam *= 3/2;
			check_improve( ch, gsn_overhead, true, 1 );
	        WAIT_STATE( ch, skill_table[gsn_overhead].beats );
		}
		else
		{
			dam *= 0;
			check_improve( ch, gsn_overhead, false, 1 );
	        WAIT_STATE( ch, skill_table[gsn_overhead].beats * 3/2 );
			act( "$N deftly evades your blow.", ch, NULL, victim, TO_CHAR );
			act( "$n tries to split $N in two but misses.", ch, NULL, victim, TO_NOTVICT);
			act( "$n tries to split you in two but misses.", ch, NULL, victim, TO_VICT );
		}
	}
	
	damage( ch, victim, dam, gsn_overhead, DAM_BASH, dam!=0);


}
/**************************************************************************/
void do_boneshatter( char_data *ch, char *argument )
{
    char_data * victim;
    OBJ_DATA  * mace;
    char        arg[MIL];
    int         chance;
	
	if ((chance = get_skill(ch,gsn_boneshatter)) == 0)
	{
		do_huh(ch,"");
		return;
	}
	
	one_argument( argument, arg );

	mace = ( get_eq_char( ch, WEAR_WIELD ));

    if ( !mace ) {
        ch->println("You need to wield a mace to shatter bones.");
        return;
    }
    
	if ( mace ) {
        if ( mace->value[0] != WEAPON_MACE ) {
            ch->println("You are not wielding a mace.");
            return;
        }
    }
	
    if ( IS_NULLSTR( arg )) {
        if (( victim = ch->fighting ) == NULL ) {
            ch->println("But you aren't fighting anyone!");
            return;
        }
    }
    else if (( victim = get_char_room( ch, arg )) == NULL ) {
        ch->printlnf("You can't seem to find '%s' here.", arg);
        return;
    }
	
 	if(ch == victim)
 	{
     	    ch->println("Yeah, right.");
      	   return;
    	}
	
	if ( is_safe( ch, victim ))
		return;
		
    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }
	
		// INSERT WEAPON SPECIALIZATION CHECK HERE
	
    if ( get_skill( ch,gsn_boneshatter ) > ( number_percent() + 20 ))
	{
        act( "You smash $p against $N.", ch, mace, victim, TO_CHAR );
		act( "$n smashes $p against $N!", ch, mace, victim, TO_NOTVICT );
		act( "$n smashes $p against you!", ch, mace, victim, TO_VICT );
        if ( damage( ch, victim, dice( mace->value[1], mace->value[2] ),
			 gsn_boneshatter, DAM_BASH, true ))
		{
			AFFECT_DATA		af;

			if ( !is_affected( victim, gsn_boneshatter )) {

				af.where		= WHERE_AFFECTS;
				af.type			= gsn_boneshatter;
				af.level		= ch->level;
				af.duration		= ch->level/10;
				af.modifier		= -10;
				af.bitvector	= 0;
			    af.location		= APPLY_ST;
			    affect_to_char( victim, &af );
				af.where		= WHERE_AFFECTS;
				af.modifier		= -10;
				af.bitvector	= 0;
				af.location		= APPLY_AG;
				affect_to_char( victim, &af );
				act( "You hear the sickening sound of bones crunching.", ch, NULL, NULL, TO_CHAR );
				act( "PAIN! The sickening sound of your bones crunching overwhelms you.", ch, NULL, victim, TO_VICT );
				act( "$N looks like $e is in a world of pain.", ch, NULL, victim, TO_NOTVICT );
			}
			victim->position = POS_RESTING;
			DAZE_STATE( victim, skill_table[gsn_boneshatter].beats*2/3 );
		}
		check_improve( ch, gsn_boneshatter, true, 1 );
		WAIT_STATE( ch, skill_table[gsn_boneshatter].beats );
	}
    else
	{
        damage( ch, victim, 0, gsn_boneshatter, DAM_BASH, false );
        check_improve( ch, gsn_boneshatter, false, 1 );
		
		ch->println("You swing wildly and don't manage to connect the blow.");
		
        WAIT_STATE( ch, skill_table[gsn_boneshatter].beats );
    }
    return;
}

/**************************************************************************/
void do_bury( char_data *ch, char *argument )
{
	OBJ_DATA	*obj;
	OBJ_DATA	*contents;
	OBJ_DATA	*shovel;
	bool		fshovel = false;
	char		arg[MIL];

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->println("Bury what?");
		return;
	}

    for ( shovel = ch->carrying; shovel; shovel = shovel->next_content )
    {
        if ( !str_cmp( shovel->pIndexData->material, "shovel" )
			 && ( shovel->wear_loc == WEAR_HOLD
			 ||   shovel->wear_loc == WEAR_WIELD )) {
			fshovel = true;
            break;
		}
    }

	if ( !fshovel ) {
        ch->println("You must be holding a shovel to dig with.");
        return;
	}	

	obj = get_obj_list( ch, arg, ch->in_room->contents );
	
	if ( obj ==  NULL )
	{
		act( "I see not $T here.", ch, NULL, arg, TO_CHAR );
		return;
	}

	if ( !IS_OUTSIDE( ch )
		|| ch->in_room->sector_type == SECT_CITY
		|| ch->in_room->sector_type == SECT_UNDERWATER
		|| ch->in_room->sector_type == SECT_WATER_SWIM
		|| ch->in_room->sector_type == SECT_WATER_NOSWIM
		|| ch->in_room->sector_type == SECT_AIR ) {

		ch->println("You can't bury anything here.");
		return;
	}

	if (   obj->item_type == ITEM_FURNITURE			// prevent stupid item burials
		|| obj->item_type == ITEM_PORTAL
		|| obj->item_type == ITEM_TOKEN
		|| obj->item_type == ITEM_FOUNTAIN
		|| obj->item_type == ITEM_BOAT )
	{
		ch->println("You can't bury that.");
		return;
	}

	if ( !IS_NPC( ch ))
	{
		if ( !IS_IMMORTAL( ch ))
		{
			++ch->pcdata->tired;
			WAIT_STATE( ch, 36 );
		}

		if (   obj->item_type == ITEM_CORPSE_PC
			|| obj->item_type == ITEM_CORPSE_NPC )
		{
			act( "You dig a grave and place $p within.",  ch, obj, NULL, TO_CHAR );
			act( "$n digs a grave and places $p within.", ch, obj, NULL, TO_ROOM );
			++ch->pcdata->tired;					// graves are tiresome :)
		}
		else
		{
			act( "You dig a hole and place $p within.",  ch, obj, NULL, TO_CHAR );
			act( "$n digs a hole and places $p within.", ch, obj, NULL, TO_ROOM );
		}
	}

	// Set corpse and all it's potential items as buried

	SET_BIT( obj->extra2_flags, OBJEXTRA2_BURIED );

	// Only set PC_CORPSE items as buried since NPC_CORPSE items poof when the corpse does
	// Container contents don't need to be flagged as buried

	if ( obj->item_type == ITEM_CORPSE_PC )
	{
		for ( contents = obj->contains; contents; contents = contents->next_content )
		{
			SET_BIT( contents->extra2_flags, OBJEXTRA2_BURIED );
		}
	}
}

/**************************************************************************/
void do_dig( char_data *ch,	char * )
{
	OBJ_DATA	*obj;
	OBJ_DATA	*pcCorpse;
	OBJ_DATA	*shovel;
	bool		found = false;
	bool		fshovel = false;

    for ( shovel = ch->carrying; shovel; shovel = shovel->next_content )
    {
        if ( !str_cmp( shovel->pIndexData->material, "shovel" )
			 && ( shovel->wear_loc == WEAR_HOLD
			 ||   shovel->wear_loc == WEAR_WIELD )) {
			fshovel = true;
            break;
		}
    }

	if ( !fshovel )
	{
        ch->println("You must be holding a shovel to dig with.");
        return;
	}	

	if ( !IS_OUTSIDE( ch )
		|| ch->in_room->sector_type == SECT_CITY
		|| ch->in_room->sector_type == SECT_UNDERWATER
		|| ch->in_room->sector_type == SECT_WATER_SWIM
		|| ch->in_room->sector_type == SECT_WATER_NOSWIM
		|| ch->in_room->sector_type == SECT_AIR )
	{

		ch->println("You can't dig here.");
		return;
	}


	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
		if ( IS_SET( obj->extra2_flags, OBJEXTRA2_BURIED ))
		{
			REMOVE_BIT( obj->extra2_flags, OBJEXTRA2_BURIED );
			found = true;
			if ( obj->item_type == ITEM_CORPSE_PC )
			{
				for ( pcCorpse = obj->contains; pcCorpse; pcCorpse = pcCorpse->next_content )
				{
					REMOVE_BIT( pcCorpse->extra2_flags, OBJEXTRA2_BURIED );
				}
			}
		}
	}

	if ( !IS_NPC( ch ))	{
		if ( !IS_IMMORTAL( ch )) {
			++ch->pcdata->tired;
			WAIT_STATE( ch, 36 );
		}
		act( "You begin to dig.", ch, NULL, NULL, TO_CHAR );
		act( "$n begins to dig.", ch, NULL, NULL, TO_ROOM );
	}

	if ( found ) {
		act( "You have unearthed something.", ch, NULL, NULL, TO_CHAR );
		act( "$n has unearthed something.", ch, NULL, NULL, TO_ROOM );
	}
}

/**************************************************************************/
void do_shadow( char_data *ch, char *argument )
{
    char		arg[MIL];
    char_data	*victim;
	int			chance = get_skill( ch, gsn_shadow );

	one_argument( argument, arg );

	if ( chance < 1 )
	{
		ch->println("Your attempts at shadowing someone would be useless.");
		return;
	}

	if ( arg[0] == '\0' )
	{
		ch->println("Who do you want to shadow?");
		return;
	}

	if (( victim = get_char_room( ch, arg )) == NULL )
    {
		ch->println("They aren't here.");
		return;
    }

	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
		ch->println("You're too busy following someone else.");
		return;
	}

	if ( victim == ch )
	{
		if ( ch->master == NULL )
		{
			ch->println("You stalk yourself like a fool.");
			return;
		}
		SET_BIT( ch->dyn, DYN_SILENTLY );
		stop_follower(ch);
		REMOVE_BIT( ch->dyn, DYN_SILENTLY );
		return;
	}

	if (IS_IMMORTAL(victim))
	{
		ch->println("Ok.");
		return;
    }

	if ( ch->master != NULL )
	{
		SET_BIT( ch->dyn, DYN_SILENTLY );
		stop_follower( ch );
		REMOVE_BIT( ch->dyn, DYN_SILENTLY );
	}

	SET_BIT( ch->dyn, DYN_SILENTLY );
	add_follower( ch, victim );
	REMOVE_BIT( ch->dyn, DYN_SILENTLY );

	ch->println("Ok.");

    if ( chance > number_percent() )
	{
		check_improve( ch, gsn_shadow, true, 2 );
	}
	else
	{
		check_improve( ch, gsn_shadow, false, 4 );
		victim->println("You have a feeling someone is following you.");
	}

	return;
}

/**************************************************************************/
void do_cannibalize( char_data *ch, char * )
{
	int		chance	= get_skill( ch, gsn_cannibalize );
	int		gain	= UMIN( 50, ch->level * 5 );

	if ( chance < 1 )
	{
		do_huh( ch, "" );
		return;
	}

	if (ch->fighting != NULL)
	{
		ch->println("You cannot do this while fighting.");
        return;
    }

	gain += UMAX( -25, chance - 75 );

	if ( (ch->hit) <= gain )
	{
		ch->println("You would surely die if you tried this now.");
		return;
	}

	ch->println("You draw magical power from your life force.");
	act( "$n is enveloped in a cold blue aura.", ch, NULL, NULL, TO_ROOM );
	ch->hit  -= gain;
	ch->mana += gain;

	WAIT_STATE( ch, 18 );
	check_improve(	ch, gsn_cannibalize, true, 1 );
	return;	
}

/**************************************************************************/
void do_fork( char_data *ch, char *argument )
{
	int	mana, roll;

	REMOVE_BIT( ch->dyn, DYN_SUCCESS_CAST );

	do_cast(ch,argument);
	roll = number_percent();

	if ( IS_SET( ch->dyn, DYN_SUCCESS_CAST ) && roll < get_skill(ch,gsn_fork) / 4 )
	{
		ch->println("You successfully forked the spell!");
		mana = ch->mana;
		do_cast( ch, argument );
		ch->mana = mana - (roll / 6);
		check_improve(ch,gsn_fork,true,1);
	}

	return;
}

/**************************************************************************/
void do_flourish( char_data *ch, char *argument )
{
	OBJ_DATA * obj = get_eq_char( ch, WEAR_WIELD );

	if ( !obj ){
		ch->println( "You are not wielding a druidic staff." );
		return;
	}
	
	if(obj->pIndexData->vnum==OBJ_VNUM_DRUIDSTAFF){
		ch->println( "Your druidstaff has no additional powers." );
	}else if (obj->pIndexData->vnum==OBJ_VNUM_TOTEMSTAFF){
		if ( !str_cmp( argument, "totemguardian" )){
			flourish_totemstaff_guardian( ch, obj );
		}
		if ( !str_cmp( argument, "totemspirit" )){
			flourish_totemstaff_spirit( ch, obj );
		}
	}
	return;
}

/**************************************************************************/
void flourish_totemstaff_guardian( char_data *ch, OBJ_DATA *obj )
{
	MOB_INDEX_DATA *pMob;
	char_data *totem;
	int vnum;
	int month = get_birthmonth( ch );
	int foo, i;		// reusable piece of junk generic var

	if ( month < 0 )
	{
		ch->println( "You have not chosen your age yet.  Type help setage to see how this is done." );
		return;
	}

	vnum = MOB_VNUM_TOTEM_BEAR + month;	// Bear = 60 and is used as an offset

	if (( pMob = get_mob_index( vnum )) == NULL )
	{
		ch->printf( "Vnum %d not found for your totem mob, please report to the admin.", vnum );
		return;
	}
	
	// Make the mob
	totem = create_mobile( pMob, UMIN( ch->level, 99));
	totem->level		= UMIN( ch->level, 99);

	// determine it's hitpoints
	foo  = dice( mob_balance_lookup( totem->level, 0 ),
				 mob_balance_lookup( totem->level, 1 ));

	totem->max_hit		= foo + mob_balance_lookup( totem->level, 2 );
	totem->hit			= totem->max_hit;

	// determine it's AC
	foo = mob_balance_lookup( totem->level, MOB_BALANCE_AC );

	for ( i = 0; i < 4; i++ )
		totem->armor[i] = foo;

	// determine it's damage
	totem->damage[DICE_NUMBER]	= mob_balance_lookup( totem->level, MOB_BALANCE_NUM_DAM_DIE );
	totem->damage[DICE_TYPE]	= mob_balance_lookup( totem->level, MOB_BALANCE_DAM_DIE_TYPE );
	totem->damage[DICE_BONUS]	= mob_balance_lookup( totem->level, MOB_BALANCE_DAM_BONUS );

	// set it to follow the druid
	add_follower( totem, ch );
	totem->leader = ch;

	ch->println( "You flourish your druidstaff which pulsates with a greenish glow and vanishes from your hands." );
	act( "$n flourishes $s druidstaff which pulsates in a greenish glow.", ch, obj, NULL, TO_ROOM );
    char_to_room( totem, ch->in_room);  // send mob to summoner
	act( "$n has suddenly arrived.", totem, NULL, NULL, TO_ROOM );
	act("$N looks at you with adoring eyes.", ch, NULL, totem, TO_CHAR );
	SET_BIT( totem->affected_by, AFF_CHARM );
	extract_obj( obj );					// nuke the staff

	return;
}
/**************************************************************************/
void flourish_totemstaff_spirit( char_data *ch, OBJ_DATA *)
{ 
	ch->println("flourish_totemstaff_spirit(): not yet implemented.");
}
/**************************************************************************/
void do_herblore( char_data *ch, char *argument )
{
	char arg[MIL];
	OBJ_DATA *herb;
	HERB_DATA *h;
	int chance = get_skill( ch, gsn_herblore);
	int skillroll;

	if ( IS_OOC( ch ))
	{
		ch->println( "Not in an OOC room." );
		return;
	}

	if ( chance == 0 )
	{
		ch->println( "You know nothing about herbs." );
		return;
	}

	one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "Learn about which herb?" );
		return;
	}

	herb = get_obj_list( ch, argument, ch->carrying );

	if ( herb == NULL )
	{
		ch->println( "You don't have that herb." );
		return;
	}

	if ( herb->item_type != ITEM_HERB )
	{
		ch->println( "That is not an herb." );
		return;
	}

	h = find_herb( ch, arg );

	if ( !h )
	{
		ch->println( "You couldn't learn anything from that particular herb." );
		return;
	}

	skillroll = number_range( 1, 100 );

	if ( skillroll > ( chance - h->difficulty ))
	{
		ch->println( "You couldn't learn anything useful." );
		check_improve( ch, gsn_herblore, false, 2 );
		return;
	}

	switch (number_range( 1, 5 ))
	{
	case (1):
		ch->printlnf( "%s only grows %s.",
			h->name,
			sector_desc[h->sector].name );
		break;
	case (2):
		if ( !h->continent)	{
			ch->printlnf( "%s is prolific, and can be found in any continent of %s.",
				h->name, capitalize(game_settings->realm_name));
		}else{
			ch->printlnf( "%s is indigenous to %s.",
				h->name,
				h->continent->name );
		}
		break;
	case (3):
		if ( h->month == -1 )
		{
			ch->printlnf( "%s is a perennial plant.",
				h->name );
		}
		else
		{
			ch->printlnf( "%s only grows in the month of %s.",
				h->name,
				month_name[h->month] );
		}
		break;
	case (4):
		if ( h->season == -1 )
		{
			ch->printlnf( "%s is a perennial plant.",
				h->name );
		}
		else
		{
			ch->printlnf( "%s only grows during the season of %s.",
				h->name,
				season_table[h->season].name );
		}
		break;
	case (5):
		if ( timefield_table[h->timefield].lowhour == -1 )
		{
			ch->printlnf( "%s may be harvested successfully at any time of the day or night.",
				h->name );
		}
		else
		{
			ch->printlnf( "%s can only be successfully harvested %s.",
				h->name,
				timefield_table[h->timefield].name );
		}
		break;
	}

	act( "$n studies $p.", ch, herb, NULL, TO_ROOM );

	WAIT_STATE( ch, skill_table[gsn_herblore].beats );

	check_improve( ch, gsn_herblore, true, 5 );

    return;
}



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
