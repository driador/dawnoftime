/**************************************************************************/
// jail.cpp - jail code
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"

/**************************************************************************/
//** Jail a player **
void do_jail( char_data *ch, char *argument )
{
    char arg1[MIL];
    char_data *victim;
	
    argument = one_argument( argument, arg1 );
	
    if(IS_NULLSTR(arg1)){
		ch->println("syntax: jail <playername>");
		ch->printlnf("The jail command transfers players to room %d.", ROOM_VNUM_JAIL);
		ch->println("Players can not delete in this room, nor talk on public channels.");
        return;
    }
	
	victim = get_whovis_player_world( ch, arg1 );
    if(!victim){
        ch->printlnf("You can't seem to find '%s' to jail.", arg1);
        return;
    }
	
    if (IS_NPC(victim)){
		ch->println("You can't jail mobiles!");
		return; 
    }
	
    if ( victim->level >= ch->level ){
        ch->println( "I don't think they'd like that too much.");
        return;
    }
	
    if(victim->in_room_vnum()==ROOM_VNUM_JAIL){
		ch->println("They are already in jail.");
		return;
    }
	if(!get_room_index( ROOM_VNUM_JAIL )){
		ch->println("There is no jail room set, this is configured in gameedit.");
		return;
	}

	char_from_room( victim );
	char_to_room( victim, get_room_index( ROOM_VNUM_JAIL ));
	info_broadcast(victim ,"%s has been jailed!",victim->name);
	victim->println("`WYou `Chave `Rbeen `Bjailed `Yfor `Mbreaking `Cthe `ylaw!`x");
	act( "You have placed $N in jail.", ch, NULL, victim, TO_CHAR );		
	ch->printlnf("The jail room is %d... the room should have the norecall and ooc flags set.", ROOM_VNUM_JAIL);
	ch->println("The code prevents players from deleting in this room.");
}

/**************************************************************************/
//** Release a Jailed Player **
void do_release( char_data *ch, char *argument )
{
    char arg1 [MIL];
    char_data *victim;
	ROOM_INDEX_DATA *location;
	
    argument = one_argument( argument, arg1 );
	
    if(IS_NULLSTR(arg1)){
        ch->println( "Release whom from jail?" );
        return;
    }
	
	victim = get_whovis_player_world( ch, arg1 );
    if(!victim){
        ch->printlnf("You can't seem to find %s to release.", arg1 );
        return;
    }
	
    if (IS_NPC(victim)){
		ch->println("You can't release mobiles!");
		return; 
    }
	
    if (( victim->level >= ch->level ))
    {
        ch->println( "You cannot release players that are a higher "
			"than or equal to level that you are.");
        return;
    }
	
    if(victim->in_room_vnum()!=ROOM_VNUM_JAIL)
    {
		ch->println("They aren't in jail.");
		return;
    }
	
    {
		char_from_room( victim );
		
		if ( class_table[victim->clss].recall)
		{
			location = get_room_index( class_table[victim->clss].recall ); // Class
		} else  { // Class recall room non-existant, check race room
			location = get_room_index( race_table[victim->race]->recall_room); // Race
		}
		
		char_to_room(victim, location);
		
		info_broadcast(victim ,"%s has been released from jail!",victim->name);
		victim->println("`BYou `Ghave `Ybeen `Rreleased `Mfrom `Cjail!`x");
		act( "You have released $N from jail.", ch, NULL, victim, TO_CHAR );
		
    }
	
}
/**************************************************************************/
/**************************************************************************/

