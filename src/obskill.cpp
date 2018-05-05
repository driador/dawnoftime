/**************************************************************************/
// obskill.cpp - skills from Oblivion
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

/**************************************************************************/
void do_channel(char_data *ch, char *argument)
{
	char arg1 [MIL];
	char arg2 [MIL];
	char_data *victim;
	int roll, roll2;
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( IS_OOC(ch) || IS_SET( ch->in_room->room_flags, ROOM_ANTIMAGIC )) {
		ch->println("You cannot focus your magic for some reason.");
		return;
	}	
	
	if (get_skill(ch,gsn_channel) < 1)
	{
		ch->println("You have not the power in your soul.");
		return;
	}
	
	if ( arg1[0] == '\0' )
	{
		ch->println("Channel how much?");
		return;
	}
	
	if (is_number(arg1) )
	{
		int amount;
		
		amount = atoi(arg1);
		
		if (amount<1)
		{
			ch->println("Yeah right!");
			return;
		}
		
		if(amount>ch->hit)
		{
			ch->println("You can not channel more power then you have.");
			return;	
		}
		
		if(arg2[0] == '\0' ) 
		{
			ch->hit-=amount;
			update_pos(ch);
			
			roll=number_percent();
			if(roll<get_skill(ch,gsn_channel))
			{
				ch->mana+=amount;
				ch->println("You feel physically exhausted.");
				if(number_percent()<amount-10){
					check_improve(ch,gsn_channel,true,14);
				}
				return;
			}
			else
			{
				ch->println("You feel no gain from your rigorous attempts at power.");
				if(number_percent()<amount-10){
					check_improve(ch,gsn_channel,false,14);
				}
				return;
			}
		}
		else
		{
			roll=number_percent();
			roll2=number_percent();			
			if( (victim = get_char_icworld(ch, arg2)) ==NULL )
			{
				ch->println("You send your power to nobody.");
				return;
			}
			
			if(amount>ch->mana)
			{
				ch->println("You have not enough mana.");
				return;
			}
			
			if(roll<get_skill(ch,gsn_channel))
			{
				if(get_skill(victim,gsn_channel)<1)
				{
					ch->println("Your target had not the skill to receive your power.");
					ch->mana-=amount;
					return;
				}
				if(roll2<get_skill(victim,gsn_channel))
				{
					ch->mana-=amount;
					victim->mana+= amount*roll2/90;
					ch->printlnf("You send your power to %s.", victim->sex==0 ? "it" : victim->sex==1 ? "him" : "her");
					victim->println("You receive magical power from an outer source.");
					if(number_percent()<amount-10){
						check_improve(ch,gsn_channel,true,14);
					}
					if(number_percent()<amount-10){
						check_improve(victim,gsn_channel,true,14);
					}
					return;
				}
				else
				{
					ch->mana-=amount;
					ch->println("Your power is received by the inept who can not harnass it.");
					if(number_percent()<amount-10){
						check_improve(ch,gsn_channel,false,14);
					}
					return;
				}
			}
			else
			{
				ch->println("You failed to send any power.");
				if(number_percent()<amount-10){
					check_improve(ch,gsn_channel,true,14);
				}
				return;
			}
		}
	}
	else
	{
        ch->println("You must specify how much to channel in hit points or mana.");
        return;
	}
	return;
}

/**************************************************************************/
void do_awareness(char_data *ch, char *)
{
	
    if(IS_NPC(ch)) 
		return;
	
	if (get_skill(ch,gsn_awareness) < 1)
	{
		ch->println("Your sense are not that acute.");
		return;
	}
	
	if(ch->pcdata->is_trying_aware)
	{
		ch->pcdata->is_trying_aware=false;
		ch->println("You let your guard down.");
	}
	else
	{
		ch->pcdata->is_trying_aware=true;
		ch->println("You make yourself more paranoid.");
	}
	
	return;
}
/**************************************************************************/
