/**************************************************************************/
// effects.cpp - effect spells
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

/***************************************************************************/
void acid_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) // nail objects on the floor 
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		
		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			acid_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
    }
	
    if (target == TARGET_CHAR)  // do the effect on a victim 
    {
		char_data *victim = (char_data *) vo;
		OBJ_DATA *obj, *obj_next;
		
		// let's toast some gear 
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			acid_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
    }
	
    if (target == TARGET_OBJ) // toast an object 
    {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		OBJ_DATA *t_obj,*n_obj;
		int chance;
		char *msg;
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_BURN_PROOF)
			||  IS_OBJ_STAT(obj,OBJEXTRA_NOPURGE)
			||  number_range(0,4) == 0)
			return;
		
		chance = level / 4 + dam / 10;
		
		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) / 2 + 50;
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
			chance -= 5;
		
		chance -= obj->level * 2;
		
		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_CAULDRON:
		case ITEM_CONTAINER:
		case ITEM_FLASK:
		case ITEM_MORTAR:
		case ITEM_CORPSE_PC:
		case ITEM_CORPSE_NPC:
			msg = "$p fumes and dissolves.";
			break;
		case ITEM_ARMOR:
			msg = "$p is pitted and etched.";
			break;
		case ITEM_CLOTHING:
			msg = "$p is corroded into scrap.";
			break;
		case ITEM_STAFF:
		case ITEM_WAND:
			chance -= 10;
			msg = "$p corrodes and breaks.";
			break;
		case ITEM_SCROLL:
			chance += 10;
			msg = "$p is burned into waste.";
			break;
		case ITEM_COMPONENT:
			chance -= 10;
			msg = "$p melts before your very eyes.";
			break;
		}
		
		chance = URANGE(5,chance,95);
		
		if (number_percent() > chance)
			return;
		
		if (obj->carried_by != NULL)
			act(msg,obj->carried_by,obj,NULL,TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg,obj->in_room->people,obj,NULL,TO_ALL);
		
		if (obj->item_type == ITEM_ARMOR)  /* etch it */
		{
			AFFECT_DATA *paf;
			bool af_found = false;
			int i;
			
			affects_from_template_to_obj(obj); // make sure the object has local affects			
			for ( paf = obj->affected; paf; paf = paf->next)
            {
                if ( paf->location == APPLY_AC)
                {
                    af_found = true;
                    paf->type = -1;
                    paf->modifier += 1;
                    paf->level = UMAX(paf->level,level);
					// break; // Kal - do all ac modifiers? can have more than one?
				}
            }			

            if (!af_found) // needs a new affect
            {
				paf = new_affect();
				
                paf->type       = -1;
                paf->level      = level;
                paf->duration   = -1;
                paf->location   = APPLY_AC;
                paf->modifier   =  1;
                paf->bitvector  = 0;
                paf->next       = obj->affected;
                obj->affected   = paf;
            }
			
            if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
				return;
		}
		
		// get rid of the object 
		if (obj->contains)  // dump contents 
		{
			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
			{
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);
				if (obj->in_room != NULL)
					obj_to_room(t_obj,obj->in_room);
				else if (obj->carried_by != NULL)
					obj_to_room(t_obj,obj->carried_by->in_room);
				else
				{
					extract_obj(t_obj);
					continue;
				}
				acid_effect(t_obj,level/2,dam/2,TARGET_OBJ);
			}
		}
		extract_obj(obj);
		return;
    }
}

/***************************************************************************/
void cold_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) // nail objects on the floor 
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;
		
        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }
	
    if (target == TARGET_CHAR) // whack a character 
    {
		char_data *victim = (char_data *) vo;
		OBJ_DATA *obj, *obj_next;
		
		// chill touch effect 
		if (!saves_spell(level/4 + dam / 20, victim, DAM_COLD))
		{
			AFFECT_DATA af;
			
            act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
			act("A chill sinks deep into your bones.",victim,NULL,NULL,TO_CHAR);
            af.where     = WHERE_AFFECTS;
            af.type      = gsn_chill_touch;
            af.level     = level;
            af.duration  = 6;
            af.location  = APPLY_ST;
            af.modifier  = -1;
            af.bitvector = 0;
            affect_join( victim, &af );
		}
		
		// hunger! (warmth sucked out)
		if (!IS_NPC(victim))
			gain_condition(victim,COND_HUNGER,dam/20);
		
		// let's toast some gear 
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			cold_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}
	
	if (target == TARGET_OBJ) // toast an object 
	{
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_BURN_PROOF)
			||  IS_OBJ_STAT(obj,OBJEXTRA_NOPURGE)
			||  number_range(0,4) == 0)
			return;
		
		chance = level / 4 + dam / 10;
		
		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) / 2 + 50;
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
			chance -= 5;
		
		chance -= obj->level * 2;
		
		switch(obj->item_type)
		{
		default:
			return;
		case ITEM_POTION:
			msg = "$p freezes and shatters!";
			chance += 25;
			break;
		case ITEM_DRINK_CON:
			msg = "$p freezes and shatters!";
			chance += 5;
			break;
		}
		
		chance = URANGE(5,chance,95);
		
		if (number_percent() > chance)
			return;
		
		if (obj->carried_by != NULL)
			act(msg,obj->carried_by,obj,NULL,TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg,obj->in_room->people,obj,NULL,TO_ALL);
		
		extract_obj(obj);
		return;
    }
}


/***************************************************************************/
void fire_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		
		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			fire_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
    }
	
    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
		char_data *victim = (char_data *) vo;
		OBJ_DATA *obj, *obj_next;
		
		/* chance of blindness */
		if (!IS_AFFECTED(victim,AFF_BLIND)
			&&  !saves_spell(level / 4 + dam / 20, victim,DAM_FIRE))
		{
            AFFECT_DATA af;
            act("$n is blinded by smoke!",victim,NULL,NULL,TO_ROOM);
            act("Your eyes tear up from smoke...you can't see a thing!",
				victim,NULL,NULL,TO_CHAR);
			
            af.where        = WHERE_AFFECTS;
            af.type         = gsn_fire_breath;
            af.level        = level;
			//          af.duration     = number_range(0,level/10);
			af.duration		= 1;
            af.location     = APPLY_HITROLL;
            af.modifier     = -4;
            af.bitvector    = AFF_BLIND;
			
            affect_to_char(victim,&af);
		}
		
		/* getting thirsty */
		if (!IS_NPC(victim))
			gain_condition(victim,COND_THIRST,dam/20);
		
		/* let's toast some gear! */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			
			fire_effect(obj,level,dam,TARGET_OBJ);
        }
		return;
    }
	
    if (target == TARGET_OBJ)  /* toast an object */
    {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		OBJ_DATA *t_obj,*n_obj;
		int chance;
		char *msg;
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_BURN_PROOF)
			||  IS_OBJ_STAT(obj,OBJEXTRA_NOPURGE)
			||  number_range(0,4) == 0)
            return;
		
        chance = level / 4 + dam / 10;
		
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
		
        if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
            chance -= 5;
        chance -= obj->level * 2;
		
        switch ( obj->item_type )
        {
        default:             
			return;
        case ITEM_CONTAINER:
            msg = "$p ignites and burns!";
            break;
        case ITEM_POTION:
		case ITEM_CAULDRON:
            chance += 25;
            msg = "$p bubbles and boils!";
            break;
        case ITEM_SCROLL:
            chance += 50;
            msg = "$p crackles and burns!";
            break;
        case ITEM_STAFF:
            chance += 10;
            msg = "$p smokes and chars!";
            break;
        case ITEM_WAND:
            msg = "$p sparks and sputters!";
            break;
        case ITEM_FOOD:
            msg = "$p blackens and crisps!";
            break;
        case ITEM_PILL:
            msg = "$p melts and drips!";
            break;
		case ITEM_COMPONENT:
			chance -= 10;
			msg = "$p is consumed by fire.";
			break;
			
        }
		
        chance = URANGE(5,chance,95);
		
        if (number_percent() > chance)
            return;
		
		if (obj->carried_by != NULL)
            act( msg, obj->carried_by, obj, NULL, TO_ALL );
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg,obj->in_room->people,obj,NULL,TO_ALL);
		
        if (obj->contains)
        {
            /* dump the contents */
			
            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);
				if (obj->in_room != NULL)
                    obj_to_room(t_obj,obj->in_room);
				else if (obj->carried_by != NULL)
					obj_to_room(t_obj,obj->carried_by->in_room);
				else
				{
					extract_obj(t_obj);
					continue;
				}
				fire_effect(t_obj,level/2,dam/2,TARGET_OBJ);
            }
        }
		
        extract_obj( obj );
		return;
    }
}

/***************************************************************************/
void poison_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;
		
        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }
	
    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        char_data *victim = (char_data *) vo;
        OBJ_DATA *obj, *obj_next;
		
		/* chance of poisoning */
        if (!saves_spell(level / 4 + dam / 20,victim,DAM_POISON) 
			&& HAS_CLASSFLAG(victim, CLASSFLAG_POISON_IMMUNITY))
        {
			AFFECT_DATA af;
			
            victim->println("You feel poison coursing through your veins.");
            act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
			
            af.where     = WHERE_AFFECTS;
            af.type      = gsn_poison;
            af.level     = level;
            af.duration  = level / 2;
            af.location  = APPLY_ST;
            af.modifier  = -1;
            af.bitvector = AFF_POISON;
            affect_join( victim, &af );
        }
		
		/* equipment */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			poison_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
    }
	
    if (target == TARGET_OBJ)  /* do some poisoning */
    {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_BURN_PROOF)
			||  IS_OBJ_STAT(obj,OBJEXTRA_BLESS)
			||  number_range(0,4) == 0)
			return;
		
		chance = level / 4 + dam / 10;
		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) / 2 + 50;
		
		chance -= obj->level * 2;
		
		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_FOOD:
			break;
		case ITEM_DRINK_CON:
			if (obj->value[0] == obj->value[1])
				return;
			break;
		}
		
		chance = URANGE(5,chance,95);
		
		if (number_percent() > chance)
			return;
		
		obj->value[3] = 1;
		return;
    }
}

/***************************************************************************/
void shock_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		
		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			shock_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
    }
	
    if (target == TARGET_CHAR)
    {
		char_data *victim = (char_data *) vo;
		OBJ_DATA *obj, *obj_next;
		
		// daze and confused?
		if (!saves_spell(level/4 + dam/20,victim,DAM_LIGHTNING))
		{
			victim->println("Your muscles stop responding.");
			DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
		}
		
		// toast some gear
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			shock_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
    }
	
    if (target == TARGET_OBJ)
    {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_BURN_PROOF)
			||  IS_OBJ_STAT(obj,OBJEXTRA_NOPURGE)
			||  number_range(0,4) == 0)
			return;
		
		chance = level / 4 + dam / 10;
		
		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) /2 + 50;
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
			chance -= 5;
		
		chance -= obj->level * 2;
		
		switch(obj->item_type)
		{
		default:
			return;
		case ITEM_WAND:
		case ITEM_STAFF:
			chance += 10;
			msg = "$p overloads and explodes!";
			break;
		case ITEM_JEWELRY:
			chance -= 10;
			msg = "$p is fused into a worthless lump.";
		}
		
		chance = URANGE(5,chance,95);
		
		if (number_percent() > chance)
			return;
		
		if (obj->carried_by != NULL)
			act(msg,obj->carried_by,obj,NULL,TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg,obj->in_room->people,obj,NULL,TO_ALL);
		
		extract_obj(obj);
		return;
    }
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
