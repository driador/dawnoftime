/**************************************************************************/
// magic_qu.cpp - spells/skills written by Quenrelthaer
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: magic_q.c - spells written by Quenrelthaer                       *
 *                                                                         *
 * cutting_wind      - air, evocation & wild magic (area attack)           *
 * decay             - evocation & necromancy                              *
 * downdraft         - air & evocation                                     *
 * fling             - air & alteration                                    *
 * shaping_the_chaos - conjuration, summoning & wild magic                 *
 * spirit_walk       - divination, summoning                               *
 * disorientation    - illusion
 *                                                                         *
 ***************************************************************************/
#include "include.h"
#include "magic.h"

/********************************/
/* START OF FUNCTION PROTOTYPES */

DECLARE_DO_FUN(do_bug);
DECLARE_DO_FUN(do_look);

/*  END OF FUNCTION PROTOTYPES  */
/********************************/


/***************************************************************************
 *  spell_cutting_wind - by Quenrelthaer August 97 - modified by kalahn    *
 ***************************************************************************/
SPRESULT spell_cutting_wind(int sn,int level,char_data *ch,void *,int )
{
	char_data	*vch;
    char_data   *vch_next;
    int dam, pdam, random;

    random = dice(1,level/3)+2;
    dam = dice((level/2)+1, random);

    for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
        vch_next=vch->next_in_room;
        pdam = dam;

        if (vch == ch)
            continue;

		if ( is_safe_spell(ch,vch,true))
			continue;

        if ( saves_spell( level, vch, DAMTYPE(sn)) )
        {
            pdam /= 2;
        }
        damage_spell( ch, vch, pdam, sn, DAMTYPE(sn),true);
	}

	return FULL_MANA;
}

/****************************************************************************
 *  spell_decay - by Quenrelthaer August 97,                                *
 * - modified by Kalahn Nov 97, made it not destroy items inside containers *
 ****************************************************************************/
SPRESULT spell_decay(int sn,int level,char_data *ch,void *vo,int target)
{
	OBJ_DATA *decay, *decay_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
	char_data *victim=(char_data*)vo;
	int dam, random;

	if (target == TARGET_OBJ)
	{
		decay = get_obj_carry(ch,target_name);
		if (decay == NULL)
		{
			ch->println("You do not see that here.");
			return NO_MANA;
		}
		
		if ( IS_SET( decay->extra2_flags, OBJEXTRA2_NODECAY ))
		{
			ch->println("Nothing seemed to happen.");
			return HALF_MANA;
		}

		decay->timer = (30/level)+1;
		act( "$p slowly starts to break apart.", ch, decay, NULL, TO_ROOM );
		act( "$p slowly starts to break apart as the decay sets in.", ch, decay, NULL, TO_CHAR );
		return FULL_MANA;
	}
	dam = dice(level, 8);
	if ( saves_spell( level, victim, DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	if (dam<level*4)
	{
		random = dice(2,10)-1;
		for ( decay = victim->carrying; decay != NULL; decay = decay_next)
		{
			decay_next = decay->next_content;
			if (decay->wear_loc == random)
				break;
		}
		if (decay == NULL)
			return FULL_MANA;

		if ( IS_SET( decay->extra2_flags, OBJEXTRA2_NODECAY ))
		{
			ch->println("Nothing seemed to happen.");
			return HALF_MANA;
		}

		act("$p suddenly falls apart, decaying into dust just before it hits the ground!",ch,decay,NULL,TO_ROOM);
		act("The magic sets upon $p, grinding it to dust.",ch,decay,NULL,TO_CHAR);

		switch ( decay->item_type )
        {
			default:
				extract_obj(decay);
				break;

			case ITEM_CAULDRON:
			case ITEM_CONTAINER:
			case ITEM_FLASK:
			case ITEM_MORTAR:
			case ITEM_CORPSE_NPC:
			case ITEM_CORPSE_PC:
				for ( obj = decay->contains; obj != NULL; obj = obj_next )
				{
					obj_next = obj->next_content;
					obj_from_obj(obj);

					if (number_range(1,10)==1)
					{
						obj_to_char( obj, ch );
						act("You manage to catch $p as before it hits the ground.", ch, obj, NULL, TO_CHAR);
					}
					else
					{
						obj_to_room(obj, ch->in_room);
						act("$p falls to the ground.", ch, obj, NULL, TO_ROOM);
						act("The $p falls to your feet.", ch, obj, NULL, TO_CHAR);
					}
				}
		}
		obj_from_char(decay);
		extract_obj(decay);
	}
	return FULL_MANA;
}

/****************************************************************************
 *  spell_disorientation - by Quenrelthaer - August 97                      *
 ****************************************************************************/
SPRESULT spell_disorientation(int ,int ,char_data *ch,void *vo,int )
{
	char_data *victim=(char_data*)vo;
	ROOM_INDEX_DATA *pRoomIndex;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	pRoomIndex = get_random_room(victim);

	victim->println("You have been teleported!");
	act("$n looks very confused all of a sudden.", victim, ch, NULL,TO_ROOM);

	if (victim != ch)
		act("$n looks very confused all of a sudden.", victim, ch, NULL,TO_CHAR);

	original = victim->in_room;
	on = victim->on;
	char_from_room( victim );
	char_to_room( victim, pRoomIndex );
	do_look(victim,"auto");
	char_from_room( victim );
	char_to_room( victim, original );
	victim->on = on;
	return FULL_MANA;
}

/****************************************************************************
 *  spell_downdraft - by Quenrelthaer August 97                             *
 ****************************************************************************/
SPRESULT spell_downdraft(int sn,int level,char_data *ch,void *vo,int )
{
	OBJ_DATA *obj, *obj_next;
	char_data *victim=(char_data*)vo;
	int dam, random;

	dam = dice(level,3)+4;
	if ( saves_spell( level, victim, DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);

	if (dam>level*2)
	{
		random = dice(2,10)-1;
		for ( obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			if (obj->wear_loc == random)
				break;
		}
		if (obj == NULL)
			return FULL_MANA;
		act("The force of the wind blows $p to the ground.",ch,obj,NULL,TO_ROOM);
		act("The force of the wind blows $p to the ground.",ch,obj,NULL,TO_CHAR);
		obj_from_char(obj);
		obj_to_room(obj,ch->in_room);
	}
	return FULL_MANA;
}

/****************************************************************************
 *  spell_fling - by Quenrelthaer August 97, updated by Kalahn Sept 97      *
 ****************************************************************************/
SPRESULT spell_fling(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim=(char_data*)vo;
	OBJ_DATA *obj, *obj_next;
	int dam, max;
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;

		if (obj->wear_loc != WEAR_NONE){
			continue;
		}

		// can't fling what you can't see/tokens
		if( !can_see_obj(ch, obj) || obj->item_type == ITEM_TOKEN){
			continue;
		}

		// can't fling nodrop items to get rid of them
		if ( !can_drop_obj( ch, obj ) ){
			act("You can't seem to fling $p, you decide to try something else!",ch,obj,NULL,TO_CHAR);			
			continue;
		}

		// we have something potentially flingable
		break; 
	}
	if (obj == NULL)
	{
		ch->println("You must be carrying something to fling.");
		return NO_MANA;
	}
	max = ((obj->weight)/10)+1;
	if (max>level+5)
	{
		act("$p is too heavy for you to fling.",ch,obj,NULL,TO_CHAR);
		return HALF_MANA;
	}
	act("Your magic flings $p into the air!",ch,obj,NULL,TO_CHAR);
	dam = dice(max,4);
	if ( saves_spell( level, victim, DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	act("$p flies up into the air, then plummets downward, striking heavily.",ch,obj,NULL,TO_ROOM);
	act("$p hits the ground with a thud.",ch,obj,NULL,TO_ROOM);
	act("$p hits the ground with a thud.",ch,obj,NULL,TO_CHAR);

	obj_from_char( obj );
	obj_to_room(obj,ch->in_room);
	return FULL_MANA;
}

/****************************************************************************
 *  spell_shaping_the_chaos - by Quenrelthaer & Kalahn August 97            *
 ****************************************************************************/
SPRESULT spell_shaping_the_chaos(int ,int level,char_data *ch,void *,int )
{
	OBJ_INDEX_DATA *pObjIndex;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_DATA *obj_chaos;
	char_data *mob_chaos;
	char buf[MSL];

	int random, max;

	ch->println("You reach into the chaos with your mind and attempt\r\nto shape it into something...");
	
	int value =0; // default type is an object

	// in rooms that are 'law' objects only summoned
	if(ch->in_room && !IS_SET(ch->in_room->room_flags,ROOM_LAW)){
		value =number_range(0,1);
	}

	switch (value)
	{
		case 0: // CREATE CHAOS OBJECT 
			max = dice(1,level)+5;
			if (max > SPELL_SHAPING_THE_CHAOS_NUMBER_OF_OBJECTS)
				max = SPELL_SHAPING_THE_CHAOS_NUMBER_OF_OBJECTS;
			random = dice(1,max);
            
			if ((pObjIndex = // check for object vnum 
				get_obj_index(SPELL_SHAPING_THE_CHAOS_FIRST_OBJECT_VNUM+random)))
            {
				obj_chaos= create_object(pObjIndex);
				obj_to_room(obj_chaos,ch->in_room);
				obj_chaos->timer = (ch->level/4)+1;
				act("Suddenly, $p appears in midair and falls to the ground.",ch,obj_chaos,NULL,TO_ROOM);
                act("The chaos forms itself into $p, which appears in midair\r\n"
                   "and falls to the ground.",ch,obj_chaos,NULL,TO_CHAR);
            }
            else /* object with the vnum not found */
            {
                sprintf(buf,"BUG: in spell_shaping_the_chaos - missing object vnum %d!\r\n", 49+random);
                wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); /* put it on the bug wiznet channel */
                log_string( buf ); /* log the bug in the logs */
                do_bug ( ch, buf); /* report the bug into the bug file  */
                ch->printf( "%s", buf ); /* tell the char what happened */
                return NO_MANA;
            }
            break;

        case 1: /* CREATE CHAOS MOB */
            max = dice(1,level)+5;
            if (max > SPELL_SHAPING_THE_CHAOS_NUMBER_OF_MOBS)
                max = SPELL_SHAPING_THE_CHAOS_NUMBER_OF_MOBS;
            random = dice(1,max);

            if ((pMobIndex = // make sure mob exists 
                    get_mob_index(SPELL_SHAPING_THE_CHAOS_FIRST_MOB_VNUM+random))) 
            {
                mob_chaos = create_mobile(pMobIndex,0); // create the mobile 
                mob_chaos->level = ch->level;
                mob_chaos->no_xp = true; // can't get xp for killing it
                char_to_room( mob_chaos ,ch->in_room);
                act("Suddenly, $N appears in midair and falls to the ground, landing solidly.",ch,NULL,mob_chaos,TO_ROOM);
                act("The chaos takes the form of $N, which appears in midair\nand falls to the ground, landing on its feet.",ch,NULL,mob_chaos,TO_CHAR);
            }
            else // mob with the vnum not found 
            {
                sprintf(buf,"BUG: in spell_shaping_the_chaos - missing mob vnum %d!\r\n", 49+random);
                wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel 
                log_string( buf ); // log the bug in the logs 
                do_bug ( ch, buf); // report the bug into the bug file 
                ch->printf( "%s", buf );
                return NO_MANA;
            }
            break;                           
	}
	return FULL_MANA;
}

/****************************************************************************
 *  spell_spirit_walk - by Quenrelthaer - August 97                         *
 ****************************************************************************/
SPRESULT spell_spirit_walk(int sn,int level,char_data *ch,void *vo,int )
{
    char buf[MSL];
	char_data *victim=(char_data*)vo;
	char_data *spirit;
	MOB_INDEX_DATA *pMobIndex;

	if(IS_SWITCHED(ch)){
		ch->println("You can't control two forms at once.");
		return NO_MANA;
	}

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAMTYPE(sn))))
    {
		ch->println("You could not hold your spirit there.");
		return FULL_MANA;
    }
    pMobIndex = get_mob_index(SPELL_SPIRIT_WALK_SPIRIT_MOB);

    if (!pMobIndex) // mob with the vnum not found 
    {
        sprintf(buf,"BUG: in spell_spirit_walk - missing host mob!!! <vnum %d>\r\n",
            SPELL_SPIRIT_WALK_SPIRIT_MOB);
        wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel
        log_string( buf ); // log the bug in the logs 
        do_bug ( ch, buf); // report the bug into the bug file
        ch->printf( "%s", buf );
        return NO_MANA;
    }

	spirit = create_mobile(pMobIndex, 0);
	char_to_room(spirit, victim->in_room);
    ch->desc->character = spirit;
    ch->desc->original  = ch;
    spirit->desc        = ch->desc;
    ch->desc            = NULL;
    ch->controlling     = spirit;   // so switched ppl aren't marked as idle/linkdead

    // change communications to match
    if (ch->prompt != NULL)
        spirit->prompt = str_dup(ch->prompt);

    spirit->comm = ch->comm;
    spirit->lines = ch->lines;
    spirit->timer = ch->level/40+1;   //set how long the spirit will last
	spirit->println("Your spirit pulls away from your body...");
	spirit->println("You find yourself somewhere else...");
	do_look(spirit,"auto");
    return FULL_MANA;
}

/***************************************************************************/
