/**************************************************************************/
// magic_da.cpp - spells/skills written for Dawn by anyone 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with all the licenses *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/ 
#include "include.h"
#include "magic.h"

DECLARE_DO_FUN(do_bug);
/**************************************************************************/
// By Thaddeus
void do_lay_on_hands(char_data * ch, char * argument)
{
	int heal;
	int skill;
	char arg[MIL];
	char_data *victim;
    
	one_argument( argument, arg );
	
	if ( IS_OOC( ch )) {
		ch->printf( "Not in an OOC room.\r\n" );
		return;
	}
	
    if ( arg[0] == '\0' )
    {
		ch->printf( "Lay your hands on whom?\r\n" );
		return;
    }
	
	
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		ch->printf( "You don't see anyone like that here.\r\n" );
		return;
	}
	
	if ( !IS_IMMORTAL( ch ))
		WAIT_STATE( ch, skill_table[gsn_lay_on_hands].beats );
	
	// don't spam message
	if (ch->desc && ch->desc->repeat>8)
	{
		ch->printf( "The lay on hands skill has no effect when used on yourself.\r\n" );
		ch->printf( "(learn to read the help files - read help lay)\r\n" );
		return;
	}
	
	if ( (victim == ch) )
	{
		act("$n lays $s hands on $mself.", ch, NULL,NULL, TO_ROOM);
		act("You lay your hands on yourself.", ch, NULL, NULL, TO_CHAR); 
		return;
	}
	
	act("$n lays $s hands on $N.", ch, NULL, victim, TO_NOTVICT);
	act("You lay your hands on $N.", ch, NULL, victim, TO_CHAR); 
	act("$n lays $s hands on you.", ch, NULL, victim, TO_VICT); 

    if (!(skill=get_skill(ch,gsn_lay_on_hands)))
        return;   
    if (IS_NPC(ch))   
        return;
    if (ch->pcdata->lays<=0)
        return;
    ch->pcdata->lays--;
	
    
    heal = UMAX(number_range(1,(ch->level/10)+1),
		number_range(1,(ch->level/10)+1));
	
    heal=(heal * skill)/100;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    
    update_pos( victim );
    victim->printf( "You feel better!\r\n" );
	ch->printf( "Your hands glow softly as a sense of divine power travels through you.\r\n" );
	
	check_improve( ch, gsn_lay_on_hands, true, 1 );
	
    return; 
}
/**************************************************************************/
// By Kirion and Kalahn
SPRESULT spell_mindspeak( int, int, char_data *ch, void *, int )
{
    static int mindspeakmon=0;
	char arg[MIL],buf[MSL],buf2[MSL];
	char tellname[MIL];
	char *argument;
    char_data *victim;
    
	// unswitched mobs can't send mindspeaks
    if (IS_UNSWITCHED_MOB(ch))
    {
		return NO_MANA;
    }

    target_name = one_argument( target_name, arg );
	argument = target_name;


    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->printf( "Mindspeak whom what?\r\n" );
        return NO_MANA;
    }

    // remove a , from the name field if required
	if (arg[str_len(arg)-1]==','){
		arg[str_len(arg)-1]=0;
	}

    // cannot send mindspeaks to mobs or to players which are not logged or are inside of ooc rooms
    if (( victim = get_whovis_player_world( ch, arg ))==NULL
		|| IS_NPC(victim)
		|| IS_SET(victim->in_room->room_flags, ROOM_OOC)) 
    {
        ch->printf( "Their mind is unreachable.\r\n" );
        return NO_MANA;
    }

	//cannot mindspeak to yourself
	if (victim==ch)
	{
		ch->printf( "You are always mindspeaking to yourself!\r\n");
		return NO_MANA;
	}

	//cannot mindspeak while with a pkill-norecall, pkill-ool or pkill-noquit timer
	if((ch->pknorecall>0) || (ch->pknoquit>0) || (ch->pkool>2000))
	{
		ch->printf( "Your mind is too troubled for you to focus on any one idea.\r\n" );
		return HALF_MANA;
	}

	// Mindspeaking to asleep people, or people affected with cause fear
    if ( (!IS_AWAKE(victim)) || (IS_AFFECTED(victim, AFF_FEAR)) || 
		 (IS_AFFECTED2(victim, AFF2_FEAR_MAGIC)) )
    {
        act( "Your thoughts could not remain in $S mind for long.", ch, 0, victim, TO_CHAR );
        return HALF_MANA;
    }

	// If caster is invisible, target receives it as a someone, instead of his desc
	strcpy(tellname,(can_see_who(TRUE_CH(victim), TRUE_CH(ch))? TRUE_CH(ch)->name:"someone"));

	// Mortals cannot mindspeak if AFK
    if (IS_SET(ch->comm,COMM_AFK) && !IS_IMMORTAL(ch))
	{
		ch->printf( "You can't mindspeak while AFK!\r\n" );
		return NO_MANA;
	}

	// cannot mindspeak to an AFK victim, but immortals get it nevertheless
    if (IS_SET(victim->comm,COMM_AFK))
    {
        act("$E is AFK, and thus, unable to receive mindspeaks",ch,NULL,victim,TO_CHAR);

		if IS_IMMORTAL(victim)
		{
			victim->printf( "%s just tried to mindspeak you '%s' "
				"(you are marked as afk)`x\r\n",
			PERS(ch, victim), argument);
			return NO_MANA;
		}

        return NO_MANA;
    }

    // wiznet mindspeak - needed to monitor possible abuses
    if (!IS_NPC(ch))
	{
		if (!IS_OOC(ch)) 
		{
			mindspeakmon++;
			if (mindspeakmon>=10)
			{
				sprintf (buf2, "Wiznet mindspeak: %s mindspeaked `s'%s'`x to %s", 
					ch->name, argument, victim->name);	
				wiznet(buf2,ch,NULL,WIZ_QUESTING,0,get_trust(ch));
				mindspeakmon=0;
			}
			ch->pcdata->did_ic=true;
		}
	}

	//Check to see if the next char is a '!' or a ',' , in which case, does a Mental emote
	if ((argument[0]=='!') || (argument[0]==','))
	{
		argument++;
		if (argument[0]==' ') argument++;
	    if(!IS_NPC(ch)) ch->pcdata->did_ooc=true;
	    ch->printf( "`bWithin the mind of %s, your image %s`x\r\n",	
			PERS(victim, ch), argument);
	    victim->printf( "`bWithin your mind. the image of %s %s`x\r\n",
			PERS(ch, victim), argument);
	 
		return FULL_MANA;
	}

	// Languages still influence mindspeaks, you think in a specific language, so
	// mindspeaks cannot be used for universal communication.

	translate_language(ch->language, false, ch, victim, argument, buf);

	//At last! The Mindspeak.
    if(!IS_NPC(ch)){
		ch->pcdata->did_ooc=true;
	}

    ch->printf( "`BYou speak your thoughts into the mind of %s: "
		"'%s`B'`x\r\n", PERS(victim, ch), argument);
    victim->printf( "`BYou hear the voice of %s within your mind: "	
		"'%s`B'`x\r\n", PERS(ch, victim), buf);
		
 	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_naturespeak( int, int, char_data *ch, void *, int ) //Kirion with slight tweaks by Ker
{
	static int naturespeakmon=0;
	char arg[MIL],buf[MSL],buf2[MSL];
	char tellname[MIL];
	char *argument;
	char_data *victim, *to;
    
	// unswitched mobs can't naturespeak
    if (IS_UNSWITCHED_MOB(ch))
    {
		ch->printf( "Unswitched mobs cannot naturespeak." );
		return NO_MANA;
    }
	
    target_name = one_argument( target_name, arg );
	argument = target_name;
	
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->printf( "Deliver which message, and to whom?\r\n" );
        return NO_MANA;
    }
	
    // remove a , from the name field if required
	if (arg[str_len(arg)-1]==',') arg[str_len(arg)-1]=0;
	
    // cannot naturespeak to mobs or to players which are not logged or are inside of ooc rooms
    if (( victim = get_whovis_player_world( ch, arg ))==NULL
		|| IS_NPC(victim)
		|| IS_SET(victim->in_room->room_flags, ROOM_OOC)) 
    {
        ch->printf( "They are nowhere to be found.\r\n" );
        return NO_MANA;
    }
	
	//cannot mindcast to yourself
	if (victim==ch)
	{
		ch->printf( "You think of a message and mentally say it to yourself.\r\n");
		return NO_MANA;
	}
	
	//cannot naturespeak while with a pkill-norecall, pkill-ool or pkill-noquit timer
	if((ch->pknorecall>0) || (ch->pknoquit>0) || (ch->pkool>0))
	{
		ch->printf( "You can't seem to focus enough to naturespeak.\r\n" );
		return HALF_MANA;
	}
	
	// Naturespeaking to asleep people, or people affected with cause fear
    if ( (!IS_AWAKE(victim)) || (IS_AFFECTED(victim, AFF_FEAR)) 
		|| (IS_AFFECTED2(victim, AFF2_FEAR_MAGIC)) )
    {
        act( "$S mind is drifting aimessly, you can't lock onto it.", ch, 0, victim, TO_CHAR );
        return HALF_MANA;
    }
	
	// If caster is invisible, target receives it as a someone, instead of his desc
	strcpy(tellname,(can_see_who(TRUE_CH(victim), TRUE_CH(ch)) ? TRUE_CH(ch)->name:"someone"));
	
	// Mortals cannot naturespeak if AFK
    if (IS_SET(ch->comm,COMM_AFK))
		if (!IS_IMMORTAL(ch))
		{
			ch->printf( "You can't naturespeak while AFK!\r\n" );
			return NO_MANA;
		}
		
		// cannot naturespeak to an AFK victim, but immortals get it nevertheless
		if (IS_SET(victim->comm,COMM_AFK))
		{
			act("$E is AFK, and thus, unable to receive a naturespeak.",ch,NULL,victim,TO_CHAR);
			
			if IS_IMMORTAL(victim)
			{
				victim->printf( "%s just tried to naturespeak to you '%s' (you are marked as afk)`x\r\n",
					PERS(ch, victim), argument);
				return NO_MANA;
			}
			
			return NO_MANA;
		}
		
		// wiznet NATURESPEAK - needed to monitor possible abuses
		if (!IS_NPC(ch))
		{
			if (!IS_OOC(ch)) 
			{
				naturespeakmon++;
				if (naturespeakmon>=10)
				{
					sprintf (buf2, "Wiznet naturespeaks: %s	naturespoke `s'%s'`x to %s", ch->name, argument, victim->name);
					wiznet(buf2,ch,NULL,WIZ_QUESTING,0,get_trust(ch));
					naturespeakmon=0;
					buf2[0]='\0';
				}
				ch->pcdata->did_ic=true;
			}	
		}
		
		// Caster feedback
		switch ( victim->in_room->sector_type )
		{
			case SECT_UNDERWATER:
			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
			case SECT_SWAMP:
				ch->println( "The spirits of water, the undines, have answered your call.");
				break;
			case SECT_CAVE:
			case SECT_MOUNTAIN:
			case SECT_CITY:
			case SECT_HILLS:
				ch->println( "The spirits of earth, the grendels, have answered your call.");
				break;
			case SECT_DESERT:
				ch->println( "The spirits of fire, the salamanders, have answered your call.");
				break;
			case SECT_AIR:
				ch->println( "The spirits of air, the sylphs, have answered your call.");
				break;
			case SECT_FOREST:
			case SECT_FIELD:
				ch->println( "The spirits of nature, the dryads, have answered your call.");
				break;
			default:
				ch->println( "No spirit could reach the target.");
				return NO_MANA;
				break;
		}
		
		buf2[0]='\0';
		
		//At last! The Naturespeak.
		if(!IS_NPC(ch)) ch->pcdata->did_ooc=true;
		
		switch ( victim->in_room->sector_type )
		{
			case SECT_FOREST:
				strcat(buf2, "`gA nearby tree takes the shape of");
				break;
			case SECT_HILLS:
				strcat(buf2, "`sA group of stones and grasses rise in the shape of");
				break;
			case SECT_FIELD:
				strcat(buf2, "`sA nearby bush shifts into the form of");
				break;
			case SECT_MOUNTAIN:
				strcat(buf2, "`sA large boulder shifts into the form of");
				break;
			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
				strcat(buf2, "`bA column of water rises in the form of");
				break;
			case SECT_SWAMP:
				strcat(buf2, "`mA pool of mud grows into the shape of");
				break;
			case SECT_AIR:
				strcat(buf2, "`cA small cloud forms, resembling");
				break;
			case SECT_DESERT:
				strcat(buf2, "`yA small dune shapes into the form of");
				break;
			case SECT_CAVE:
				strcat(buf2, "`rA nearby stone morphs into the form of");
				break;
			case SECT_UNDERWATER:
				strcat(buf2, "`BBubbles form in the water on the shape of");
				break;
			case SECT_CITY:
				strcat(buf2, "`wStones from a nearby wall morphs into the shape of");
				break;
			default:
				strcat(buf2, "`wVapour from nowhere appears in the shape of");
				break;
		}

		for( to = victim->in_room->people; to ; to = to->next_in_room )
		{
			//Check to see if the next char is a '!' or a ',' , in which case, does a nature emote
			if ((argument[0]=='!') || (argument[0]==','))
			{
				argument++;
				if (argument[0]==' ') argument++;
				if(!IS_NPC(ch)) ch->pcdata->did_ooc=true;
				to->printf( "%s %s and %s`x\r\n", buf2, PERS (ch, to), argument); 
			}
			else
			{
				// Languages still influence naturespeaks
				translate_language(ch->language, false, ch, victim, argument, buf);
				to->printf( "%s %s and says to %s:'%s'`x\r\n", buf2, PERS (ch, to),
					YOU_PERS(victim, to), buf); 
			}
		}
		
	return FULL_MANA;
}

/***************************************************************************
 *  Cause Headache - Adds -20 to SD                                        *
 *  By Ylerin, February 1999                                               *
 ***************************************************************************/

SPRESULT spell_cause_headache(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( is_affected( victim, sn ))
	{
		if (victim == ch)
			ch->printf( "Your head already aches tremendously! You surely don't want to increase it.\r\n" );
		else
			act("$N already suffers from a terrible headache. Have a bit of pity on $m.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_SD;
	af.modifier		= - level/5;
	af.bitvector	= 0;
	affect_to_char( victim, &af );
	victim->printf( "Your head seems to explode with a sudden wave of indescribable pain!\r\n" );
	act("$n grimaces in pain.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}

/***************************************************************************
 *  Cure Headache - Cures headache :p                                      *
 *  By Ylerin, Auguts 1999                                                 *
 ***************************************************************************/

SPRESULT spell_cure_headache( int , int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
 
	if ( !is_affected( victim, gsn_cause_headache ) )
	{
		if (victim == ch)
			ch->printf( "You don't have a headache.\r\n" );
		else
			act("$N appears to be well.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}
 
	if (check_dispel(level,victim,gsn_cause_headache))
	{
		victim->printf( "Your headache is gone, what a relief!\r\n" );
		act("$n looks much relieved.",victim,NULL,NULL,TO_ROOM);
	}
	else
		ch->printf( "Your spell failed.\r\n" );
	return FULL_MANA;
}

/**************************************************************************/
// Kal
SPRESULT spell_restore_mana( int, int level, char_data *ch, void *vo,int)
{
	char_data *victim = (char_data *) vo;

	victim->mana = UMIN( victim->mana + UMIN(number_range(level, level*3),150), victim->max_mana );
	victim->println( "A warm glow passes through you.");

	if ( ch != victim ){
		ch->println( "Ok.");
	}
	return FULL_MANA;
}

/**************************************************************************/
// balo
SPRESULT spell_remove_alignment( int sn, int level, char_data *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int result, fail;
    AFFECT_DATA *paf;

    if (obj->wear_loc != -1)
    {
        ch->println("The item must be carried to remove its alignment.");
        return NO_MANA;
    }

    if ((!IS_OBJ_STAT(obj,OBJEXTRA_ANTI_GOOD))
       && (!IS_OBJ_STAT(obj,OBJEXTRA_ANTI_EVIL))
       && (!IS_OBJ_STAT(obj,OBJEXTRA_ANTI_NEUTRAL)))
    {
        ch->println("The item has no alignment.");
        return HALF_MANA;
    }

    fail = 25;  // base 25% chance of failure 

    // find the bonuses (Its harder to remove align from a powerful object) 

    if (!obj->affected) // enchanted dosn`t seem to exist ? (!obj->enchanted)
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            fail += 20;
        }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        fail += 20;
    }


    // apply other modifiers 
    fail += obj->level;
    fail -= 3 * level / 2;

    // harder to remove align from objects inherently blessed or evil 
    if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS)){
        fail += 5;
	}
    if (IS_OBJ_STAT(obj,OBJEXTRA_EVIL)){
        fail += 5;
	}

    fail = URANGE(5,fail,90);

    result = number_percent();

    // the moment of truth 
    if (result < (fail / 5))  // item destroyed 
    {
        act("$p shivers and shudders... then implodes!",ch,obj,NULL,TO_CHAR);
        act("$p shivers and shudders... then implodes!",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
        return FULL_MANA;
    }

    if ( result <= fail )  // failed, no bad result 
    {
        ch->printf("Nothing seemed to happen.\n\r");
        return FULL_MANA;
    }

    // Success!  Remove the item's alignment 
    REMOVE_BIT(obj->extra_flags,OBJEXTRA_ANTI_EVIL);
    REMOVE_BIT(obj->extra_flags,OBJEXTRA_ANTI_GOOD);
    REMOVE_BIT(obj->extra_flags,OBJEXTRA_ANTI_NEUTRAL);
    act("You remove $p's alignment!",ch,obj,NULL,TO_CHAR);
    act("$n removes $p's alignment!",ch,obj,NULL,TO_ROOM);
    return FULL_MANA;
}

/***************************************************************************/
SPRESULT spell_enchant_item(int sn,int level, char_data *ch, void *vo,int )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf; 
	int result, fail;
	int added;
	bool hit_found = false, dam_found = false;

	// do standard checks - if spell can be cast 
    if ((obj->item_type != ITEM_LIGHT)		&& (obj->item_type != ITEM_TREASURE)
      && (obj->item_type != ITEM_CLOTHING)	&& (obj->item_type != ITEM_CONTAINER)
      && (obj->item_type != ITEM_GEM)		&& (obj->item_type != ITEM_JEWELRY))
	{
		ch->println( "That isn't enchantable." );
		return NO_MANA;
    }

    if(obj->wear_loc!=-1){
        ch->printf( "The item must be carried to be enchanted.\r\n" );
        return NO_MANA;
    }

    fail = 25;	// base 25% chance of failure 

    // find the current bonuses - affect success 
	for (paf=OBJECT_AFFECTS(obj); paf; paf=paf->next )
    {
        if ( paf->location == APPLY_HITROLL ){
            hit_found = true;
            fail += 2 * paf->modifier * paf->modifier;
            if (paf->duration == -1) // can't add to a perm weapon enchant 
            {
                ch->printf( "%s starts to pulsate then it dissolves before your eyes!!!",
                    capitalize(format_obj_to_char(obj, ch, true)));
                extract_obj(obj);
                return FULL_MANA;
            }
        }else if (paf->location == APPLY_DAMROLL ){
            dam_found = true;
            fail += 2 * paf->modifier * paf->modifier;
            if (paf->duration == -1) // can't add to a perm weapon enchant 
            {
                ch->printf( "%s starts to pulsate then it dissolves before your eyes!!!\r\n",
                    format_obj_to_char(obj, ch, true));
                extract_obj(obj);
                return FULL_MANA;
            }
        }else{
            fail += 20;
        }
    }

    // apply other modifiers 
    fail -= 3 * level/2;
    if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS)){
        fail -= 15;
    }if (IS_OBJ_STAT(obj,OBJEXTRA_GLOW)){
        fail -= 5;
	}

    fail = URANGE(5,fail,95);
    result = number_percent();

    // the moment of truth 
    if (result < (fail / 5)){  // item destroyed 
        act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
        act("$p shivers violently and explodes!",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
        return FULL_MANA;
    }

    if (result < (fail / 2)){ // item disenchanted 
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
		return FULL_MANA;
	}

    if ( result <= fail )  // failed, no bad result
    {
        ch->printf( "Nothing seemed to happen.\r\n" );
        return FULL_MANA;
    }

    // **** ALL SUCCESSFUL ENCHANTS BELOW HERE ****
	// now setup all the enchants:
	// - if it enchants, weren't stored on the object, copy the enchants
    //   from the object vnums enchants.   
	affects_from_template_to_obj(obj); 

    if (result <= (100 - level/5)){  // success! 
        act("$p glows blue.",ch,obj,NULL,TO_CHAR);
        act("$p glows blue.",ch,obj,NULL,TO_ROOM);
        SET_BIT(obj->extra_flags, OBJEXTRA_MAGIC);
        added = 1;
    }else{  // exceptional enchant     
        act("$p glows a brillant blue!",ch,obj,NULL,TO_CHAR);
        act("$p glows a brillant blue!",ch,obj,NULL,TO_ROOM);
        SET_BIT(obj->extra_flags,OBJEXTRA_MAGIC);
        SET_BIT(obj->extra_flags,OBJEXTRA_GLOW);
        added = 2;
    }
		
    // now add the enchantments 
    if (dam_found){
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->location == APPLY_DAMROLL)
            {
                paf->type = sn;
                paf->modifier += added;
                if (ch->level>paf->level) // up the level if caster is higher
                    paf->level = ch->level;
                if (paf->modifier > 4 && (number_range(1,100)<95))
                    SET_BIT(obj->extra_flags,OBJEXTRA_HUM);
            }
        }
    }else{ // add a new damroll affect 
        paf = new_affect();
        paf->where      = WHERE_OBJEXTRA;
        paf->type       = sn;
        paf->level      = ch->level;
        paf->duration   = level * 20;
        paf->location   = APPLY_DAMROLL;
        paf->modifier   = added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
    }

    if (hit_found){
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if ( paf->location == APPLY_HITROLL)
            {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level)/2;
                if (paf->modifier > 4){
					SET_BIT(obj->extra_flags,OBJEXTRA_HUM);
				}
            }
        }
    }else{ // add a new affect 
        paf = new_affect();
        paf->type       = sn;
        paf->level      = ch->level;
        paf->duration   = level * 20;
        paf->location   = APPLY_HITROLL;
        paf->modifier   = added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
    }
	return FULL_MANA;
}
/**************************************************************************/
/**************************************************************************/

