/**************************************************************************/
// magic_ti.cpp - spells/skills written by Tibault
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "magic.h"
#include "o_lookup.h"

/**************************************************************************/
SPRESULT spell_higher_learning( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA		af;
    char_data		*victim;
	connection_data *d;
	int				alignStray;

	if ( is_affected( ch, sn )) 
	{
	    ch->println("You already feel inspired.");
	    return HALF_MANA;
	}

	for (d = connection_list; d != NULL; d = d->next)
	{
		victim = d->character;
		if (victim && !IS_OOC(victim) && !IS_NPC(victim) && !is_affected( victim, sn) )
		{
			alignStray = abs(ch->alliance - victim->alliance);
			
			af.where		= WHERE_AFFECTS;
			af.type			= sn;
			af.location		= APPLY_RE;
			af.level		= level;
			if ( ch == victim ) {
				af.duration	= level/3;
				af.modifier	= level/20;
			} else {
				af.duration	= level/6;
				af.modifier	= level/8 - (level*alignStray/30);
			}
			af.bitvector	= 0;
			affect_to_char( victim, &af );

			if ( ch == victim ) {
				af.where		= WHERE_AFFECTS;
				af.type			= sn;
				af.location		= APPLY_SD;
				af.level		= level;
				af.duration		= 5;
				af.modifier		= -level/5;
				af.bitvector	= 0;
				affect_to_char( victim, &af );
				ch->println("You pray to your deity for inspiration throughout the realm!");
				ch->println("You feel a little light-headed.");
			} else {
				victim->println("You feel inspired by divine intervention.");
			}
		}
	}

	ch->pcdata->tired += 25;
    ch->pcdata->condition[COND_THIRST] = 0;
    ch->pcdata->condition[COND_HUNGER] = 0;

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_detect_treeform( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED2(victim, AFF2_DETECT_TREEFORM) )
	{
		if (victim == ch)
			ch->printf( "You area already one with nature.\r\n" );
		else
			act("$N is already in touch with nature.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS2;
	af.type      = sn;
	af.level     = level;
	af.duration  = level/2;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF2_DETECT_TREEFORM;
	affect_to_char( victim, &af );

	victim->printf( "You feel more in touch with nature.\r\n" );
	act("$n is surrounded with a `#`ggreen`^ aura.", ch, NULL, victim, TO_NOTVICT);

	if ( ch != victim )
		ch->printf( "Ok.\r\n" );
	return FULL_MANA;
}

/**************************************************************************/
/**************************************************************************/
