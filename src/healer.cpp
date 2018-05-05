/**************************************************************************/
// healer.h - The heal code
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
void do_heal(char_data *ch, char *argument)
{
    char_data *mob;
    char arg[MIL];
    char target[MIL];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	
	char_data *victim=ch; // default 'victim' of heal
	
	// exception in here so players can buy heals while badly hurt
	// but can't while sleeping
	if (ch->position == POS_SLEEPING)
	{
		ch->println("You can't do that while sleeping.");
        return;
	}
	
    // check for healer
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
	
    if ( mob == NULL )
    {
        ch->println("You can't do that here.");
        return;
    }
	
	if ( ch->fighting ) {
		if ( ch->fighting == mob ){
			ch->println("The healer doesn't seem to want to help you at the moment, you wonder why?");
		}else{
			ch->println("You seem to be too busy to exchange gold for services rendered.");
		}
		return;
	}
	
    argument=one_argument(argument,arg);
	one_argument(argument,target);
	
	if(!IS_NULLSTR(target)){
		victim= get_char_room( ch,target);
		if(!victim){
			ch->printlnf("'%s' doesn't appear to be here.", target);
			return;
		}
	}
	
    if (arg[0] == '\0')
    {
        // display price list 
		act("$N says 'I offer the following spells:'",ch,NULL,mob,TO_CHAR);
		ch->println("  light:    cure light wounds     1 gold");
		ch->println("  serious:  cure serious wounds   2 gold");
		ch->println("  critic:   cure critical wounds  5 gold");
		ch->println("  refresh:  restore movement      5 gold");
		ch->println("  disease:  cure disease          8 gold");
		ch->println("  heal:     healing spell        10 gold");
		ch->println("  blind:    cure blindness       10 gold");
		ch->println("  poison:   cure poison          10 gold");
		ch->println("  mana:     restore mana         10 gold");
		ch->println("  headache: cure headache        15 gold");
		ch->println("  uncurse:  remove curse         20 gold");
		ch->println("  cancel:   cancellation         30 gold");
		ch->println("  chaos:    chaotic poison       35 gold");
		ch->println("");
		ch->println("  Type heal <type> to be healed.");
		ch->println("  You can pay the healer to heal someone else using:");
		ch->println("  heal <type> <person>.");
		return;
    }
	
    if (!str_prefix(arg,"light"))
    {
        spell = spell_cure_light;
		sn    = skill_lookup("cure light");
		words = "judicandus dies";
		cost  = 100;
	}
	
	else if (!str_prefix(arg,"serious"))
	{
		spell = spell_cure_serious;
		sn    = skill_lookup("cure serious");
		words = "judicandus gzfuajg";
		cost  = 200;
	}
	
	else if (!str_prefix(arg,"critical"))
	{
		spell = spell_cure_critical;
		sn    = skill_lookup("cure critical");
		words = "judicandus qfuhuqar";
		cost  = 500;
	}
	
	else if (!str_prefix(arg,"heal"))
	{
		spell = spell_heal;
		sn = skill_lookup("heal");
		words = "pzar";
		cost  = 1000;
    }
	
    else if (!str_prefix(arg,"blindness"))
    {
		spell = spell_cure_blindness;
		sn    = skill_lookup("cure blindness");
		words = "judicandus noselacri";		
		cost  = 1000;
	}
	
	else if (!str_prefix(arg,"disease"))
	{
		spell = spell_cure_disease;
		sn    = skill_lookup("cure disease");
		words = "judicandus eugzagz";
		cost = 800;
	}
	
	else if (!str_prefix(arg,"poison"))
	{
		spell = spell_cure_poison;
		sn    = skill_lookup("cure poison");
		words = "judicandus sausabru";
		cost  = 1000;
	}
	
	else if (!str_prefix(arg,"cancel"))
	{
		spell = spell_cancellation;
		sn    = gsn_cancellation;
		words = "judicandus katesism";
		cost  = 3000;
	}

	else if (!str_prefix(arg,"chaos"))
	{
		spell = spell_cure_chaotic_poison;
		sn    = gsn_chaotic_poison;
		words = "judicandus erdanata";
		cost  = 3500;
	}
	
	else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
    {
		spell = spell_remove_curse; 
		sn    = skill_lookup("remove curse");
		words = "candussido judifgz";
		cost  = 2000;
	}
	
	else if (!str_prefix(arg,"mana") || !str_prefix(arg,"energize"))
	{
		spell = NULL;
		sn = -1;
		words = "energizer";
		cost = 1000;
	}

	else if (!str_prefix(arg,"headache"))
	{
		spell = spell_cure_headache;
		sn = skill_lookup("cure headache");
		words = "treifher";
		cost = 1500;
	}

	else if (!str_prefix(arg,"refresh") || !str_prefix(arg,"moves"))
	{
		spell =  spell_refresh;
		sn    = skill_lookup("refresh");
		words = "candusima";
		cost  = 500;
	}
	
	else
	{
		act("$N says 'Type 'heal' for a list of spells.'",
			ch,NULL,mob,TO_CHAR);
		return;
    }
	
	if (cost > (ch->gold * 100 + ch->silver))
	{
		act("$N says 'You do not have enough gold for my services.'",
			ch,NULL,mob,TO_CHAR);
		return;
	}
	
	WAIT_STATE(ch,PULSE_VIOLENCE);
	
	deduct_cost(ch,cost);
	mob->gold += cost;
	
	limit_mobile_wealth(mob);

	if(victim!=ch){ // do a nod to show who payed for the heal
		act( "$N nods at you.", ch, NULL, mob, TO_CHAR );
		act( "$N nods at $n.", ch, NULL, mob, TO_ROOM );
	}
	act("$n utters the words '$T'.",mob,NULL,words,TO_ROOM);
	
	if (spell == NULL)  // restore mana trap...kinda hackish 
	{
		victim->mana += dice(4,8) + mob->level / 2;
		victim->mana = UMIN(victim->mana,victim->max_mana);
		victim->println("A warm glow passes through you.");
		return;
	}
	
	if (sn == -1)
		return;
	
	spell(sn,mob->level,mob,victim,TARGET_CHAR);
}
/**************************************************************************/
