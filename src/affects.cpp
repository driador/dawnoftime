/**************************************************************************/
// affects.cpp - see below
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
/***************************************************************************
 *  FILE: affects.cpp - written mainly by Kalahn                           *
 *                                                                         *
 *  affects_update() - called by update_handler in update.c                *
 ***************************************************************************/
#include "include.h" // standard dawn includes

/********************************/
/* START OF FUNCTION PROTOTYPES */
DECLARE_DO_FUN(do_bug);
void rand_to_char(char_data *ch, char *str1, char *str2, char *str3, char *str4, char *str5);
void mount( char_data *, char_data *);
void dismount( char_data *);


/**** local prototypes below ****/
void do_fearful(char_data *);
void do_fearmagic(char_data *);
void do_animal_train(char_data *);

/*  END OF FUNCTION PROTOTYPES  */
/********************************/


/************************************************************************/
/*
 * Control the active affects going on.
 * Called periodically by update_handler.
 */
void affects_update( void )
{
	char_data *ch;

	for ( ch = char_list; ch; ch = ch->next )
	{
		// added stuff fear - Gwynn
		if (IS_AFFECTED(ch, AFF_FEAR) && (number_range(1,3)==1))
		{ 
			do_fearful(ch);
		}          

		if (IS_AFFECTED2(ch, AFF2_FEAR_MAGIC) && (number_range(1,3)==1))
		{ 
			do_fearmagic(ch);
		}          

		// check for missing mounts	
		if (ch->mounted_on)
		{
			if (ch->mounted_on->in_room!=ch->in_room)
			{
				ch->println("Your mount has inexplicably disappeared.");
				dismount(ch);
				bug("BUG affects_update(): Mount in different room than rider.");
			}
		}

		if (ch->bucking){
			do_animal_train(ch);
		}
	}
	return;
}

/************************************************************************/
// for spell_fear_magic - Tibault
void do_fearmagic(char_data *ch)
{
    int attempt;
    ROOM_INDEX_DATA *was_in;
    bool found;
    EXIT_DATA *pexit;
    int door;
	
	if IS_OOC(ch)
		return;
	
	// record the room they start in
    was_in = ch->in_room;
	
    switch(ch->position)
    {
	case POS_SLEEPING:
		rand_to_char(ch,
			"You have nightmares about bolts of lightning striking you.\r\n",
			"You have nightmares about witches dancing around a cauldron.\r\n",
			"You have a nightmare about a dark warlock hurling fireballs.\r\n",
			"You have nightmares about wizards chanting.\r\n",
			"You grunt as you have nightmares about blasts of acid.\r\n" );
		
		ch->println("You SCREAM and wake up, sweat running down your forehead.");
		ch->position = POS_RESTING;
		break;
		
	case POS_RESTING:
	case POS_SITTING:
	case POS_KNEELING:
		ch->println("You stand up, screaming in fright as a blast of flame soars past your head.");
		ch->position = POS_STANDING;
		
	case POS_FIGHTING:
	case POS_STANDING:
		if (ch->mounted_on!=NULL)
		{
			ch->println("You leap from your mount as he transforms into a necromancer.");
			dismount(ch);
		}
		ch->println("As the magical spells fly by you, you scream for your life and run.");
    }
    
    if (ch->position < POS_STANDING)
		return;
	
	rand_to_char(ch,
		"Winds of death *** DEVASTATE *** you.",
		"A wicked witch points her cruel wand towards you.",
		"A blast of acid burns through your skin.",
		"An elementalists improved fireball SLAYS you!.",
		"Shrieking arrows of flame dart in your direction.");
	
	ch->println("\r\nThat really did HURT.\r\nYou flee from combat.");
	
	// do a 'flee' from room 
	for (attempt = 0; attempt < 10; attempt++ )
	{
		found = false;
		
		door = number_door( );
		
		if ( ( pexit = was_in->exit[door] ) == 0
			||   pexit->u1.to_room == NULL
			||   IS_SET(pexit->exit_info, EX_CLOSED)
			||   number_range(0,ch->daze) != 0
			||  (IS_NPC(ch)          
			&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB )))
			continue;
		
		act("$n screams and runs out the room!", ch, NULL, NULL, TO_ROOM );
		move_char( ch, door, false );
		
		// keep going till we get out of the room
		if(ch->in_room != was_in )
			continue;
		
		if (!IS_NPC(ch))
			ch->println("You scream and run away in terror!");
		
		stop_fighting( ch, true ); // incase they were fighting 
		return;
	}
	ch->println("PANIC! You can't seem to escape the horror!");
	return;
}

/************************************************************************/
// for spell_cause_fear and dragonfear - Rathern 
void do_fearful(char_data *ch)
{
    int attempt;
    ROOM_INDEX_DATA *was_in;
    bool found;
    EXIT_DATA *pexit;
    int door;
	
	if IS_OOC(ch)
		return;
	
	// record the room they start in
    was_in = ch->in_room;
	
    switch(ch->position)
    {
	case POS_SLEEPING:
		rand_to_char(ch,
			"You have nightmares about vampires sucking your blood.\r\n",
			"You have nightmares about spiders crawling over your bed.\r\n",
			"You have a nightmare about a scorpion under your pillow.\r\n",
			"You have nightmares about monsters under your bed.\r\n",
			"You clench your stomach as you have nightmares about the fabled Rott Worm.\r\n" );
		
		ch->println("You SCREAM and wake up, sweat running down your forehead.");
		ch->position = POS_RESTING;
		break;
		
	case POS_RESTING:
	case POS_SITTING:
	case POS_KNEELING:
		ch->println("You stand up, screaming in fright as the monsters loom around you.");
		ch->position = POS_STANDING;
		
	case POS_FIGHTING:
	case POS_STANDING:
		if (ch->mounted_on!=NULL)
		{
			ch->println("You leap from the fiend you are sitting on in mortal terror.");
			dismount(ch);
		}
		ch->println("As the monsters close in around you, you scream for your life and run.");
    }
    
    if (ch->position < POS_STANDING)
		return;
	
	rand_to_char(ch,
		"The goblin's slash *** DEMOLISHES *** you.",
		"The beast's attack *** DEVASTATES *** you.",
		"The bee's sting poisons you.",
		"The Worms DEVASTATE! you as they crawl from your mouth.",
		"Allethrin SLAYS you!.");
	
	ch->println("\r\nThat really did HURT.\r\nYou flee from combat.");
	
	// do a 'flee' from room 
	for (attempt = 0; attempt < 10; attempt++ )
	{
		found = false;
		
		door = number_door( );
		
		if ( ( pexit = was_in->exit[door] ) == 0
			||   pexit->u1.to_room == NULL
			||   IS_SET(pexit->exit_info, EX_CLOSED)
			||   number_range(0,ch->daze) != 0
			||  (IS_NPC(ch)          
			&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB )))
			continue;
		
		act("$n screams and runs out the room!", ch, NULL, NULL, TO_ROOM );
		move_char( ch, door, false );
		
		// keep going till we get out of the room
		if(ch->in_room != was_in )
			continue;
		
		if (!IS_NPC(ch))
			ch->println("You scream and run away in terror!");
		
		stop_fighting( ch, true ); // incase they were fighting 
		return;
	}
	ch->println("PANIC! You can't seem to escape the horror!");
	return;
}

/************************************************************************/
void do_animal_train(char_data *ch)  
{
	int chance=number_range(1,100);
	char_data * rider = ch->ridden_by;
   
	if (IS_SET(ch->act, ACT_PET)) 
		chance+=5;

	if (!rider)
	{
		bug("Error in do_animal_train, riderless ch");
		ch->bucking=false;
		return;
	}
   
	if (IS_NPC(rider))
	{
		bug("Error in do_animal_train, NPC rider");
		dismount(ch->ridden_by);
		ch->bucking=false;
		return;
	}
   
	// check to see if rider is bucked
	if ((chance<10)
		|| (chance>=get_skill(rider,gsn_animal_training))
		|| (chance>=get_skill(rider,gsn_riding)))
		ch->bucking = false;

	if (!ch->bucking)
	{
		dismount(rider);
		act("$N bucks you off!", rider, NULL, ch, TO_CHAR); 
		act("$n is bucked off of $N", rider, NULL, ch, TO_ROOM);
		check_improve(rider, gsn_animal_training, false, 3);
		rider->position = POS_RESTING;
		if (number_range(1,3)==1)
		{
			rider->println("This is really wearing you out.");
			rider->pcdata->tired+=number_fuzzy(3);
		}
		
		// see if mount gets really pissed off
		if (ch->level+5 > rider->level)
			multi_hit(ch, rider, TYPE_UNDEFINED);
	}
	else
	{
		act("$N tries to buck you off, but you hold on!", rider, NULL, ch, TO_CHAR); 
		act("$N tries to buck off $n, but $e holds on!", rider, NULL, ch, TO_ROOM);
		if (number_range(1,3)==1)
		{
			rider->println("This is really wearing you out.");
			rider->pcdata->tired++;
		}
		chance = 1;
		if (rider->level < ch->level)
			chance = 0;
		else 
			chance += (rider->level - ch->level) / 5;
		if (IS_SET(ch->act, ACT_PET))
			chance+=2;
		
		ch->will -= chance;

		if (ch->will <= 0)
		{
			act("$N stops bucking, you have broken $M!", rider, NULL, ch, TO_CHAR); 
			act("$N stops bucking, $n has broken $M!", rider, NULL, ch, TO_ROOM);
			ch->will = 0;
			SET_BIT(ch->act, ACT_DOCILE);
			ch->bucking = false;
		}
		check_improve(rider, gsn_animal_training, true, 3); 
		check_improve(rider, gsn_riding, true, 1);
		return;
	}
	return;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
