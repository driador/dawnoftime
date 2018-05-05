/**************************************************************************/
// magic_ke.cpp - spells/skills written by Kerenos
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "areas.h"
#include "magic.h"
#include "o_lookup.h"
#include "ictime.h"


/********************************/
/* START OF FUNCTION PROTOTYPES */
bool check_social( char_data *ch, char *arg, char *arg1, bool global );
/*  END OF FUNCTION PROTOTYPES  */
/********************************/


/**************************************************************************/
// Kerenos - Sept 98
SPRESULT spell_otterlungs( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA		af;

// You may want to omit this check
// Would allow character to update his otterlungs

	if ( is_affected( ch, sn )) {
		ch->println( "You already are affected by otterlungs." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= ch->level;
	af.duration		= level;
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= AFF_OTTERLUNGS;
					
	affect_to_char( ch, &af );

	ch->println("You can now travel underwater without fear of drowning.");

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_benedict( int sn, int level, char_data *ch, void *vo, int )
{
	char_data		*victim;
	AFFECT_DATA		af;
	int			chance, stat;

	victim = ( char_data * ) vo;

	if ( victim == ch ) {
		ch->println( "You may only benedict others, it is a selfless supplication." );
		return NO_MANA;
	}

	if ( !IS_IMMORTAL( ch )) {
		if ( victim->alliance!= URANGE(ch->alliance -1, victim->alliance, ch->alliance+1)) { 
			act( "You cannot grant a benediction upon $N.", ch, NULL, victim, TO_CHAR );
			return HALF_MANA;
		}	// Only works on PC's with similar alignment, (good neutral evil)
	}		// lawful and chaotic makes no difference	

	if ( is_affected( victim, sn )) {
		act( "The grace of the gods already touch $N.", ch, NULL, victim, TO_CHAR );
		return HALF_MANA;
	}

	if (( chance = number_range( 0, 1 ))){
		stat = number_range( 1, 5 );	// ST QU PR EM IN
	}else{
		stat = number_range( 26, 30);	// CO AG SD ME RE
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= 24;
	af.location		= translate_old_apply_number(stat);	// determined above 
	af.modifier		= 10;				// level dependant or always +10?  Balance issue
	af.bitvector	= 0;				// will be a fairly low level spell so I think
										// leaving it at 10 will be ok.
	affect_to_char( victim, &af );

	victim->println( "The gods smile kindly upon you." );
	act( "The gods smile kindly upon $N.", ch, NULL, victim, TO_CHAR );

	return FULL_MANA;
}

/***********************************************************************/
SPRESULT spell_maledict( int sn, int level, char_data *ch, void *vo, int )
{
	char_data		*victim;
	AFFECT_DATA		af;
	int				chance, stat;

	victim = ( char_data * ) vo;

	if ( victim == ch ) {
		ch->println( "Your god will not forsake you." );
		return NO_MANA;
	}

	if ( !IS_IMMORTAL( ch )) {
		if ( victim->alliance== URANGE(ch->alliance -1, victim->alliance, ch->alliance+1)) { 
			act( "You cannot maledict $N.", ch, NULL, victim, TO_CHAR );
			return HALF_MANA;
		}	// Only works on PC's of unequal alignment, (good neutral evil)
	}		// lawful and chaotic makes no difference

	if ( is_affected( victim, sn )) {
		act( "The cannot maledict $N any further.", ch, NULL, victim, TO_CHAR );
		return HALF_MANA;
	}

	if (( chance = number_range( 0, 1 ))){
		stat = number_range( 1, 5 );	// ST QU PR EM IN
	}else{
		stat = number_range( 26, 30);	// CO AG SD ME RE
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= 24;
	af.location		= translate_old_apply_number(stat);	// determined above 
	af.modifier		= -10;				// level dependant or always -10?  Balance issue
	af.bitvector	= 0;				// will be a fairly low level spell so I think
										// leaving it at -10 will be ok.
	affect_to_char( victim, &af );

	victim->println( "The gods look upon you with distate." );

	act( "The gods look upon $N with distate.", ch, NULL, victim, TO_CHAR );

	return FULL_MANA;
}

/****************************************************************************/
SPRESULT spell_earthwalk( int sn, int level, char_data *ch, void *vo,int )
{
	char_data	*victim		= ( char_data * ) vo;
	char		*buf;

	if (( victim = get_char_icworld( ch, target_name )) == NULL
	 ||   victim == ch
	 ||   victim->in_room == NULL
	 ||   !can_see_room(ch,victim->in_room )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_SAFE )
	 ||	  IS_SET( victim->in_room->room_flags, ROOM_INDOORS )
	 ||	  IS_SET( victim->in_room->room_flags, ROOM_ANTIMAGIC )
	 ||	  IS_SET( victim->in_room->room_flags, ROOM_PET_SHOP )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_NOSCRY )
	 ||   IS_SET( victim->in_room->area->area_flags, AREA_NOSCRY )
	 ||   IS_SET( victim->in_room->area->area_flags, AREA_NOGATEINTO)
	 ||   IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
	 ||   victim->level >= level + 3
	 ||	  victim->in_room->sector_type == SECT_INSIDE 
 	 ||	  victim->in_room->sector_type == SECT_CITY 
	 ||	  victim->in_room->sector_type == SECT_WATER_SWIM 
	 ||	  victim->in_room->sector_type == SECT_WATER_NOSWIM 
	 ||	  victim->in_room->sector_type == SECT_AIR 
	 ||	  victim->in_room->sector_type == SECT_UNDERWATER 
	 ||   ( !IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL )		// NOT trust
	 ||   ( IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON ))
	 ||   ( IS_NPC(victim) && saves_spell( level, victim,DAMTYPE(sn)))
	 ||   ( !IS_NPC(victim) && saves_spell( level, victim, DAMTYPE(sn)))) {
		  ch->println( "You failed." );
		return FULL_MANA;
	}

	switch ( ch->in_room->sector_type ) {
		case SECT_FIELD:
			act("The grass below $n opens up and swallows $m up.",ch,NULL,NULL,TO_ROOM);
			ch->println( "The grass below you opens wide and swallows you." );
			break;
		case SECT_FOREST:
			act("$n melts into a tree and vanishes.",ch,NULL,NULL,TO_ROOM);
			ch->println( "You step into a tree and vanish." );
			break;
		case SECT_HILLS:
		case SECT_MOUNTAIN:
			act("$n suddenly sinks into the rocky soil and is gone.",ch,NULL,NULL,TO_ROOM);
			ch->println( "Suddenly the rocky soil splits open and you sink down." );
			break;
		case SECT_SWAMP:
			act("The swampy ground under $n bubbles, and $e sinks and is gone.",ch,NULL,NULL,TO_ROOM);
			ch->println( "The ground under your feet swallows you up." );
			break;
		case SECT_DESERT:
			act("A sandy whirlpool is created under $n and suddenly $e disappears.",ch,NULL,NULL,TO_ROOM);
			ch->println( "You flush yourself into the sand and vanish." );
			break;
		case SECT_CAVE:
			act("$n is sucked into the ground and is longer before you.",ch,NULL,NULL,TO_ROOM);
			ch->println( "The ground sucks you deep into the bowels of the earth." );
			break;
		default:
			ch->println( "You could not attune yourself enough to travel." );
			return HALF_MANA;
			break;
	}
	
	char_from_room(ch);
	char_to_room(ch,victim->in_room);

	switch ( ch->in_room->sector_type ) {
		case SECT_FIELD:
			buf = str_dup("rises from the grass");
			break;
		case SECT_FOREST:
			buf = str_dup("steps out of a tree");
			break;
		case SECT_HILLS:
		case SECT_MOUNTAIN:
			buf = str_dup("rises up from the rocky soil");
			break;
		case SECT_SWAMP:
			buf = str_dup("rises up from the swampy ground");
			break;
		case SECT_DESERT:
			buf = str_dup("materializes from the sand");
			break;
		case SECT_CAVE:
			buf = str_dup("appears from the ground");
			break;
		default:
			buf = str_dup("appears from nowhere");		// This should never occur
			break;
	}
			
	act("$n $t.",ch,buf,NULL,TO_ROOM);
	do_look(ch,"auto");

    if ( IS_NPC( ch ) && HAS_TRIGGER( ch, MTRIG_ENTRY ))
		mp_percent_trigger( ch, NULL, NULL, NULL, MTRIG_ENTRY );

    if ( !IS_NPC( ch ))
		mp_greet_trigger( ch );

    return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_detect_scry( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA		af;

	if ( is_affected( ch, sn )) {
		ch->println( "You can already detect scrying attempts." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= 24;
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= 0;
						
	affect_to_char( ch, &af );

	ch->printf( "You are now able to detect scrying attempts." );

	return FULL_MANA;
}

/*****************************************************************************/
SPRESULT spell_sunfire( int sn, int level, char_data *ch, void *vo, int )
{
	char_data		*victim = (char_data *) vo;
	int				dam, dam2;

	dam2 = number_range( 6, 12);
	dam  = dice( level, dam2 );

	if ( !IS_OUTSIDE( ch )
		|| ( time_info.hour < 6 || time_info.hour > 18 )) {
		ch->println( "This spell requires the light of the sun to work." );
		return NO_MANA;
	}

	if ( ch->in_room->sector_type == SECT_CAVE )
	{
		ch->println( "The sun does not shine here." );
		return NO_MANA;
	}

	if ( saves_spell( level, victim, DAMTYPE(sn)))
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn), true );
	return FULL_MANA;
}

/********************************************************************************/
SPRESULT spell_cure_chaotic_poison(int ,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;

	if ( !is_affected( victim, gsn_chaotic_poison ))
	{
		if (victim == ch)
			ch->println( "You aren't poisoned." );
		else
			act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}
 
	if (check_dispel(level,victim,gsn_chaotic_poison ))
	{
		victim->println( "The poison no longer courses through your veins!" );
		act("$n is no longer poisoned.",victim,NULL,NULL,TO_ROOM);
	}
	else
		ch->println( "Spell failed." );

	return FULL_MANA;
}

/****************************************************************************/
SPRESULT spell_shadowflight( int , int level, char_data *ch, void *vo,int )
{
	char_data	*victim		= ( char_data * ) vo;

	if ( time_info.hour >= 6 && time_info.hour <= 18 ) {
		ch->println( "You may only use shadowflight under the cloak of night." );
		return HALF_MANA;
	}

	if (( victim = get_char_icworld( ch, target_name )) == NULL
	 ||   victim == ch
	 ||   victim->in_room == NULL
	 ||   !can_see_room(ch,victim->in_room )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_SAFE )
	 ||	  IS_SET( victim->in_room->room_flags, ROOM_INDOORS )
	 ||	  IS_SET( victim->in_room->room_flags, ROOM_ANTIMAGIC )
	 ||	  IS_SET( victim->in_room->room_flags, ROOM_PET_SHOP )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
	 ||   IS_SET( victim->in_room->room_flags, ROOM_NOSCRY )
	 ||   IS_SET( victim->in_room->area->area_flags, AREA_NOSCRY )
	 ||   IS_SET( victim->in_room->area->area_flags, AREA_NOGATEINTO )	 
	 ||   IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
	 ||   victim->level >= level + 5
	 ||   ( !IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL )		// NOT trust
	 ||   ( IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON ))) {
		  ch->println( "You failed." );
		return FULL_MANA;
	}

	if ( ch->in_room->area->continent != victim->in_room->area->continent )
	{
		ch->println( "The spell cannot span such a great distance." );
		return FULL_MANA;
	}


	act("$n steps into the night and vanishes.",ch,NULL,NULL,TO_ROOM);
	ch->println( "You step into the night and vanish." );
	char_from_room(ch);
	char_to_room(ch,victim->in_room);

	act("The shadows suddenly deepen.",ch,NULL,NULL,TO_ROOM);
	do_look(ch,"auto");

    if ( IS_NPC( ch ) && HAS_TRIGGER( ch, MTRIG_ENTRY ))
		mp_percent_trigger( ch, NULL, NULL, NULL, MTRIG_ENTRY );

    if ( !IS_NPC( ch ))
		mp_greet_trigger( ch );

    return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_mirage(int,int,char_data *ch,void *,int )
{
	ch->mirage = ch->in_room->vnum;
	ch->mirage_hours = UMAX( 1, number_range( ch->level/4, ch->level/2));
	ch->println( "You have attuned your image to this location." );
	return FULL_MANA;
}
/*******************************************************************************/
SPRESULT spell_commune_with_dead(int, int, char_data *ch,void *vo,int target)
{
	OBJ_DATA *corpse;

	if (target == TARGET_OBJ)
	{
		corpse = (OBJ_DATA *) vo;

		if (corpse->item_type != ITEM_CORPSE_NPC 
			&& corpse->item_type != ITEM_CORPSE_PC)
		{
			ch->println( "You can only commune through corpses." );
			return HALF_MANA;
		}
		act( "A ghostly form rises from $p.", ch, corpse, NULL, TO_CHAR );
		if(corpse->killer){
			ch->wraplnf("A mental image forms in your mind, you see the outline of %s.",
				corpse->killer );
		}else{
			ch->wrapln( "A mental image starts to form in your mind, "
				"then vanishes before you can focus on it.");
		}

		return FULL_MANA;
	}

	ch->println( "You can only commune through corpses." );
	return HALF_MANA;
}
/*******************************************************************************/
SPRESULT spell_utterdark( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA		af;

	if ( IS_SET( ch->in_room->affected_by, ROOMAFF_UTTERDARK )) {
		ch->println( "This room is already as dark as it will get." );
		return HALF_MANA;
	}

	if (IS_SET(ch->in_room->room_flags,ROOM_LAW))
	{
		ch->println( "For a moment the room appears dark, then the darkness dissipates as quickly as it came." );
		act( "For a moment the room appears dark, then the darkness dissipates as quickly as it came.", ch, NULL, NULL, TO_ROOM );
		return HALF_MANA;
	}


	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= ch->level;
	af.duration		= level/20;
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= ROOMAFF_UTTERDARK;
						
	affect_to_room( ch->in_room, &af );

	ch->println( "Suddenly, everything around you becomes very dark." );
	act( "Suddenly, everything around you becomes very dark.", ch, NULL, NULL, TO_ROOM );
	return FULL_MANA;
}
/*******************************************************************************/
SPRESULT spell_alarm( int sn, int, char_data *ch, void *, int )
{
	AFFECT_DATA		af;

	if(!ch->in_room){
		bug("spell_alarm(): ch->in_room==NULL.");
		return NO_MANA;
	}

	// This should never happen, since entering a room with alarm
	// should trip the alarm off
	if ( IS_SET( ch->in_room->affected_by, ROOMAFF_ALARM )) {
		ch->println( "This room is already alarmed." );
		return NO_MANA;
	}

	if ( is_affected( ch, sn )) {
		ch->println( "You cannot place another alarm quite yet." );
		return NO_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= ch->level;
	af.duration		= -1;			// Permanent
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= ROOMAFF_ALARM;
						
	affect_to_room( ch->in_room, &af );
	ch->in_room->alarm = ch;
	SET_BIT(ch->dyn, DYN_USER_OF_ALARM_SPELL);
	ch->println( "The alarm has been set." );	

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= ch->level;
	af.duration		= 24;			
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= 0;
	affect_to_char( ch, &af );

	return FULL_MANA;
}
/*******************************************************************************/
SPRESULT spell_divine_light( int sn, int level, char_data *ch, void *, int )
{
	OBJ_DATA	*dlight;
	AFFECT_DATA	af;

	if ( is_affected( ch, sn )) {
		ch->println( "You cannot summon the mists quite yet." );
		return NO_MANA;
	}

	if ( get_obj_index( OBJ_VNUM_DIVINE_LIGHT ) == NULL )
	{
		bugf("Vnum %d not found for Divine Light!", OBJ_VNUM_DIVINE_LIGHT );
		ch->printlnf( "Vnum %d not found for Divine Light, please report to the admin.", OBJ_VNUM_DIVINE_LIGHT );
		return NO_MANA;
	}

	dlight = create_object( get_obj_index( OBJ_VNUM_DIVINE_LIGHT ));
	dlight->timer = 25;
	dlight->value[3] = (100 + ((level / 10) * 5));	// Heal bonus of 5% per 10 levels
	dlight->value[4] = (100 + ((level / 20) * 5));	// Mana bonus of 5% per 20 levels
	obj_to_room( dlight, ch->in_room );

	act( "$n calls up to the heavens and a shimmering globe of light descends,", ch, NULL, NULL, TO_ROOM );
	act( "diffusing into an ethereal, glowing mist that hovers on the floor.", ch, NULL, NULL, TO_ROOM );
	act( "You call up to the heavens and a shimmering globe of light descends,", ch, NULL, NULL, TO_CHAR );
	act( "diffusing into an ethereal, glowing mist that hovers on the floor.", ch, NULL, NULL, TO_CHAR );

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= ch->level;
	af.duration		= 24;			
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= 0;
	affect_to_char( ch, &af );

	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_immolation( int sn, int level, char_data *ch, void *, int )
{
	char_data *vch;
	AFFECT_DATA af;

	if ( is_affected( ch, gsn_immolation )) {
		ch->println( "You failed." );
		return NO_MANA;
	}

	ch->println( "You are engulfed within the flames of wizardry." );
	act( "$n is engulfed in fire.", ch, NULL, NULL, TO_ROOM );

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 2;
	af.location  = APPLY_HITROLL;
	af.modifier  = get_skill(ch, gsn_sorcery ) / 5;
	af.bitvector = 0;
	affect_to_char( ch, &af );

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 2;
	af.location  = APPLY_DAMROLL;
	af.modifier  = get_skill(ch, gsn_sorcery ) / 5;
	af.bitvector = 0;
	affect_to_char( ch, &af );

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 2;
	af.location  = APPLY_AC;
	af.modifier  = 0 - get_skill(ch, gsn_sorcery ) / 2;
	af.bitvector = 0;
	affect_to_char( ch, &af );


	ch->println( "Tongues of flame from your body fly forth." );
	act( "Fire from $n reach out threatening to consume you.", ch, NULL, NULL, TO_ROOM );

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
	{
		if ( vch->in_room == NULL )
			continue;
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && !is_safe_spell(ch,vch,true)){
				if (IS_AFFECTED(vch,AFF_FLYING)){
					damage_spell(ch,vch,0,sn,DAMTYPE(sn),true);
				}else{
					damage_spell( ch,vch, dice(level, 10 ) + 10, sn, DAMTYPE(sn),true);
				}
			}
			continue;
		}
	}
	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_treeform( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA af;

	if ( ch->in_room->sector_type != SECT_FOREST )
	{
		ch->println( "You can only cast this spell in a forest." );
		return NO_MANA;
	}

	if ( IS_SET( ch->affected_by2, AFF2_TREEFORM ))
	{
		ch->println( "You have already assumed the shape of a tree." );
		return NO_MANA;
	}

	if ( !is_affected( ch, gsn_barkskin ))
	{
		ch->println( "You must make your skin as that of a tree first." );
		return NO_MANA;
	}

	af.where     = WHERE_AFFECTS2;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF2_TREEFORM;
	affect_to_char( ch, &af );

	ch->println( "Your body transforms into the shape of a tree." );
	return FULL_MANA;

}

/*******************************************************************************/
SPRESULT spell_pass_without_trace( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA af;

	if (IS_AFFECTED2(ch, AFF2_PASSWOTRACE))
	{
		ch->println("You are not leaving any footprints already.");
		return NO_MANA;
	}

	af.where	= WHERE_AFFECTS2;
	af.type		= sn;
	af.level	= level;
	af.duration = 24;
	af.location	= APPLY_NONE;
	af.modifier = 0;
	af.bitvector= AFF2_PASSWOTRACE;
	affect_to_char( ch, &af );

	ch->println( "You will not leave footprints outdoors." );
	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_pine_needles( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA af;

	if ( ch->in_room->sector_type != SECT_FOREST )
	{
		ch->println( "You can only cast this spell in a forest." );
		return NO_MANA;
	}

	if ( is_affected( ch, sn ))
	{
		ch->println( "Pine needles already swirl about you." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= 24;
	af.modifier		= -(level / 5 + 1);	// -2 to -20
	af.location		= APPLY_AC;
	af.bitvector	= 0;
	affect_to_char( ch, &af );

	ch->println( "A swirl of pine needles surrounds you." );
	act( "A swirl of pine needles surrounds $n.", ch, NULL, NULL, TO_ROOM );
	return FULL_MANA;
}

/*******************************************************************************/
void do_needlepoint( char_data *ch, char *argument )
{
	int				ac;		// to store the -ac value to be ++ed
	AFFECT_DATA		paf;
	char_data		*victim = NULL;
	char			arg1[MIL];
	char			arg2[MIL];

	if ( !is_affected( ch, gsn_pine_needles )){
		ch->println("You have to be affected by pine needles to use this spell.");
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( IS_NULLSTR( arg1 ))
	{
		victim = ch->fighting;
		if (victim == NULL)
		{
			ch->println( "But you aren't fighting anyone!" );
			return;
		}
	} 
	else if (( victim = get_char_room( ch, arg1 )) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	if ( !can_initiate_combat( ch, victim, CIT_GENERAL )) return;

	// point to the right affect in ch->affected
	paf = *affect_find( ch->affected, gsn_pine_needles );

	// store the ac from the pine needles into int ac
	ac = paf.modifier;

	if ( ch->desc && ch->desc->repeat>4 )
	{
		ac += 5;				// spamming point will weaken the needles by 5 :)
		ch->wrapln( "The constant stream of needles issuing forth causes some of the other needles to drop harmlessly to the ground." );
	}

	if ( ac >= 0 )
	{
		ch->println( "You have depleted all your pine needles." );
		affect_strip( ch, gsn_pine_needles );
		return;
	}

	// increment ac, lessening their reserve by one (or all if 10 or more and all is used)
	if ( !str_cmp( arg2, "all" ))
	{
		if ( ac > -10 )
		{
			ch->println( "You don't have enough needles swirling about you to do this." );
			return;
		}
		else
		{ // all of them shoot (acid blast damage)
			act( "You shoot a barrage of pine needles at $N.", ch, NULL, victim, TO_CHAR );
			act( "$n let's loose a barrage of needles at $N.", ch, NULL, victim, TO_ROOM );
			if ( ac <= -20 ) {
				damage_spell( ch, victim, ( dice( ch->level, 10 )) * 2, gsn_pine_needles, DAM_PIERCE, true );
				ch->mana -= 75;
			}
			else
			{
				damage_spell( ch, victim, ( dice( ch->level, 10 )) , gsn_pine_needles, DAM_PIERCE, true );
				ch->mana -= 30;
			}
			ch->println( "You have depleted all your pine needles." );
			affect_strip( ch, gsn_pine_needles );
			WAIT_STATE(ch, skill_table[gsn_pine_needles].beats * 3 / 2);
			return;
		}
	}
	
	act( "One of your pine needles flies towards $N.", ch, NULL, victim, TO_CHAR );
	act( "A needle from $n flies straight towards you.", ch, NULL, victim, TO_VICT );
	act( "A needle from $n flies towards $N.", ch, NULL, victim, TO_NOTVICT );
	damage_spell( ch, victim, ch->level / 4 + 2, gsn_pine_needles, DAM_PIERCE, true );
	ch->mana -= 5;
	WAIT_STATE(ch, skill_table[gsn_pine_needles].beats );

	// increment and update AC in character's thingie :)
	ac++;

	affect_strip( ch, gsn_pine_needles );

	paf.where		= WHERE_AFFECTS;
	paf.type		= gsn_pine_needles;
	paf.level		= ch->level;
	paf.duration	= 24;
	paf.modifier	= ac;
	paf.location	= APPLY_AC;
	paf.bitvector	= 0;
	affect_to_char( ch, &paf );

	return;
}

/*******************************************************************************/
SPRESULT spell_spirit_hammer(int sn, int, char_data *ch, void *, int )
{
	AFFECT_DATA af;
	OBJ_INDEX_DATA *pObj;
	OBJ_DATA *obj;
	OBJ_DATA *cObj;
	int numdie, dietype, random, vlevel;

	if ( is_affected( ch, gsn_spirit_hammer ))
	{
		ch->println("You cannot summon another hammer quite yet.");
		return NO_MANA;
	}

	if (( pObj = get_obj_index( OBJ_VNUM_SPIRIT_HAMMER )) == NULL )
	{
		ch->printf( "Vnum %d not found for Spirit Hammer, please report to the admin.", OBJ_VNUM_SPIRIT_HAMMER );
		return NO_MANA;
	}

	obj = create_object( pObj);
	obj->level = ch->level;

	vlevel	   = obj->level + number_range( 0, (( 100 - ch->level ) / 10 ));
	// first try to get a random damage based on level
	random  = number_range( 0, 7 );
	numdie  = weapon_balance_lookup( vlevel, random, 0 );
	dietype = weapon_balance_lookup( vlevel, random, 1 );	

	// damage lookup failed, use polarmost damage class,
	if ( numdie == 0 ||  dietype == 0 )
	{
		numdie  = weapon_balance_lookup( vlevel, 0, 0 );
		dietype = weapon_balance_lookup( vlevel, 0, 1 );
	}


	obj->attune_flags = ATTUNE_NEED_TO_USE|ATTUNE_HARD|ATTUNE_INFURIATING|ATTUNE_PREVIOUS;
	obj->attune_id    = ch->player_id;
	obj->timer        = 48;
	obj->value[1]	  = numdie;
	obj->value[2]	  = dietype;
	obj->wear_loc	  = WEAR_WIELD;

	if ( IS_EVIL( ch )){
		SET_BIT( obj->extra_flags, OBJEXTRA_ANTI_GOOD );
	}

	if ( IS_GOOD( ch )){
		SET_BIT( obj->extra_flags, OBJEXTRA_ANTI_EVIL );
	}

	af.where	= WHERE_AFFECTS;
	af.type		= sn;
	af.level	= ch->level;
	af.duration = 48;
	af.location	= APPLY_NONE;
	af.modifier = 0;
	af.bitvector= 0;
	affect_to_char( ch, &af );

	ch->println( "You call to the heavens and your cry is heard." );
	ch->println( "A mighty hammer appears in your hands." );
	act( "A fervent look passes across the face of $n.", ch, NULL, NULL, TO_ROOM );
	act( "A hammer suddenly appears in $s hands.", ch, NULL, NULL, TO_ROOM );

	// see if the have a weapon, this will allow removal of cursed items too
	cObj = get_eq_char( ch, WEAR_WIELD );
	if ( cObj ){
		unequip_char( ch, cObj );
	}
	
	obj_to_char( obj, ch );

	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_druidstaff( int, int level, char_data *ch, void *, int )
{
    AFFECT_DATA *af;
	OBJ_INDEX_DATA *pObj;
	OBJ_DATA *obj;
	OBJ_DATA *cObj;
	int numdie, dietype, random, vlevel;

	if (( pObj = get_obj_index( OBJ_VNUM_DRUIDSTAFF )) == NULL )
	{
		ch->printlnf( "Vnum %d not found for Druidstaff, please report to the admin.", OBJ_VNUM_DRUIDSTAFF );
		return NO_MANA;
	}

	// see if the have a built staff in their wield slot
	cObj = get_eq_char( ch, WEAR_WIELD );
	if ( cObj )
	{
		if ( cObj->pIndexData->vnum != OBJ_VNUM_STAFF )
		{
			ch->println( "You must cast this spell on a built staff that you are holding." );
			return NO_MANA;
		}
	}
	else
	{
		ch->println( "You must cast this spell on a built staff that you are holding." );
		return NO_MANA;
	}
	
	obj = create_object( pObj);
	vlevel = UMIN( level, 20 );
	obj->level = vlevel;

	// first try to get a random damage based on level
	random  = number_range( 0, 7 );
	numdie  = weapon_balance_lookup( vlevel, random, 0 );
	dietype = weapon_balance_lookup( vlevel, random, 1 );	

	// damage lookup failed, use polarmost damage class,
	if ( numdie == 0 ||  dietype == 0 )
	{
		numdie  = weapon_balance_lookup( vlevel, 0, 0 );
		dietype = weapon_balance_lookup( vlevel, 0, 1 );
	}


	obj->attune_flags	= ATTUNE_NEED_TO_USE|ATTUNE_PREVIOUS|ATTUNE_ONCE_ONLY;
	obj->attune_id		= ch->player_id;
	obj->attune_next	= current_time + ICTIME_IRLSECS_PER_DAY;	// 8640 seconds or 144 Minutes :)
	obj->value[1]		= numdie;
	obj->value[2]		= dietype;
	obj->wear_loc		= WEAR_WIELD;			// Set wear loc to the character's inventory

	af					= new_affect();
	af->location		= APPLY_MANA;
	af->modifier		= UMIN( ch->level / 3, 10 );
	af->where			= WHERE_OBJEXTRA;
	af->type			= -1;
	af->duration		= -1;
	af->bitvector		= 0;
	af->level			= vlevel;
	af->next			= obj->affected;
	obj->affected		= af;
	
	act( "A faint green glow passes over $p.", ch, cObj, NULL, TO_CHAR );
	act( "A faint green glow passes over $p held by $n.", ch, cObj, NULL, TO_ROOM );
	extract_obj( cObj );		// Nuke the built staff
	obj_to_char( obj, ch );		// give them the new one

	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_totemstaff( int, int level, char_data *ch, void *, int )
{
	OBJ_INDEX_DATA *pObj;
	OBJ_DATA *obj;
	OBJ_DATA *cObj;
	int numdie, dietype, random, vlevel;

	if (( pObj = get_obj_index( OBJ_VNUM_TOTEMSTAFF )) == NULL )
	{
		ch->printlnf( "Vnum %d not found for Druidstaff, please report to the admin.", OBJ_VNUM_TOTEMSTAFF );
		return NO_MANA;
	}

	// see if the have a druidstaff in their wield slot
	cObj = get_eq_char( ch, WEAR_WIELD );
	if ( cObj )
	{
		if ( cObj->pIndexData->vnum != OBJ_VNUM_DRUIDSTAFF )
		{
			ch->println( "You must cast this spell on a druidstaff that you are holding." );
			return NO_MANA;
		}
	}
	else
	{
		ch->println( "You must cast this spell on a druidstaff that you are holding." );
		return NO_MANA;
	}

	if ( cObj->attune_id != ch->player_id )
	{
		ch->println( "That is not your staff." );
		return NO_MANA;
	}

	if ( current_time < cObj->attune_next && !IS_IMMORTAL(ch))
	{
		ch->println( "You cannot convert the druidstaff yet." );
		return NO_MANA;
	}
	
	obj = create_object( pObj);
	vlevel = UMIN( level, 30 );
	obj->level = vlevel;

	// first try to get a random damage based on level
	random  = number_range( 0, 7 );
	numdie  = weapon_balance_lookup( vlevel, random, 0 );
	dietype = weapon_balance_lookup( vlevel, random, 1 );	

	// damage lookup failed, use polarmost damage class,
	if ( numdie == 0 ||  dietype == 0 )
	{
		numdie  = weapon_balance_lookup( vlevel, 0, 0 );
		dietype = weapon_balance_lookup( vlevel, 0, 1 );
	}

	obj->attune_flags	= ATTUNE_NEED_TO_USE|ATTUNE_PREVIOUS|ATTUNE_ONCE_ONLY;
	obj->attune_id		= ch->player_id;
	obj->attune_next	= current_time + ICTIME_IRLSECS_PER_DAY * 2;
	obj->value[1]		= numdie;
	obj->value[2]		= dietype;
	obj->wear_loc		= WEAR_WIELD;			// Set wear loc to the character's inventory

	act( "A mild green glow passes over $p.", ch, cObj, NULL, TO_CHAR );
	act( "A mild green glow passes over $p held by $n.", ch, cObj, NULL, TO_ROOM );
	extract_obj( cObj );		// Nuke the druid staff
	obj_to_char( obj, ch );		// give them the new one

	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_mnemonics(int sn,int level,char_data *ch,void *,int )
{
    AFFECT_DATA af;

	if ( is_affected( ch, sn ))
	{
		ch->println( "You cannot cram any more information into your head." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_ME;
	af.modifier		= +5;
	af.bitvector	= 0;
	affect_to_char( ch, &af );
	ch->println( "You seem to be able to recall events more clearly." );
	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_logic(int sn,int level,char_data *ch,void *,int )
{
    AFFECT_DATA af;

	if ( is_affected( ch, sn ))
	{
		ch->println( "You are not able to process information any quicker." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_RE;
	af.modifier		= +5;
	af.bitvector	= 0;
	affect_to_char( ch, &af );
	ch->println( "You seem to be able to reason more effectively." );
	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_clarity(int sn,int level,char_data *ch,void *,int )
{
    AFFECT_DATA af;

	if ( is_affected( ch, sn ))
	{
		ch->println( "You see things as clearly as you are able." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_IN;
	af.modifier		= +5;
	af.bitvector	= 0;
	affect_to_char( ch, &af );
	ch->println( "You gain insight into the unknown." );
	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_persuasion(int sn,int level,char_data *ch,void *,int )
{
    AFFECT_DATA af;

	if ( is_affected( ch, sn ))
	{
		ch->println( "You are as persuasive as can be." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_PR;
	af.modifier		= +5;
	af.bitvector	= 0;
	affect_to_char( ch, &af );
	ch->println( "You feel more persuasive." );
	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_lucidity(int sn,int level,char_data *ch,void *,int )
{
    AFFECT_DATA af;

	if ( is_affected( ch, sn ))
	{
		ch->println( "You are already thinking clearly." );
		return HALF_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_IN;
	af.modifier		= +5;
	af.bitvector	= 0;
	affect_to_char( ch, &af );
	ch->println( "You feel more lucid." );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_night_eyes( int sn, int level, char_data *, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if(!is_affected(victim,sn))
	{
		af.where = WHERE_RESIST;
		af.type = sn;
		af.level = level;
		af.duration = level/4+10;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector   = RES_LIGHT;
		affect_to_char(victim, &af);

		if ( !IS_AFFECTED(victim, AFF_INFRARED ))
		{
			af.where	 = WHERE_AFFECTS;
			af.type      = sn;
			af.level	 = level;
			af.duration  = level/4+10;
			af.location  = APPLY_NONE;
			af.modifier  = 0;
			af.bitvector = AFF_INFRARED;
			affect_to_char( victim, &af );
		}
		victim->println( "Your pupils contract." );
	}
	else
	{
		victim->println( "Your pupils are as small as they will get." );
		return HALF_MANA;
	}
	return FULL_MANA;
}

/*******************************************************************************/
SPRESULT spell_shrink(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
    AFFECT_DATA af;

	if ( victim->size <= SIZE_SMALL )		// small or tiny
	{
		if ( victim == ch )
		{
			ch->println( "You cannot shrink yourself any smaller." );
			return NO_MANA;
		}
		else
		{
			ch->println( "They cannot be shrunk down any more than they already are." );
			return NO_MANA;
		}
	}

	if ( is_affected( victim, sn ))
	{
		if ( victim == ch )
		{
			ch->println( "You are already in a shrunken state." );
			return HALF_MANA;
		}
		else
		{
			ch->println( "They are already in a shrunken state." );
			return HALF_MANA;
		}
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level/4;
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= 0;
	affect_to_char( ch, &af );

	victim->println( "You feel yourself shrinking." );

	if ( ch != victim )
		act("$N shrinks to a fraction of $s former size.", ch, NULL, victim, TO_CHAR );
	return FULL_MANA;
}


/*******************************************************************************/
SPRESULT spell_night_of_the_leonids( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA	af;
	char_data *rch;
	int dam;

	if (  ch->in_room->sector_type == SECT_INSIDE
		|| IS_SET( ch->in_room->room_flags, ROOM_INDOORS ))
	{
		ch->println( "You can only call upon the Leonids in the out of doors." );
		return NO_MANA;
	}

	if (   time_info.hour >= HOUR_SUNRISE
		&& time_info.hour <= HOUR_SUNSET )
	{
		ch->println( "This song is only effective at night." );
		return NO_MANA;
	}
	
	ch->println( "Meteors rain from the sky." );
	act( "Meteors rain from the sky.", ch, NULL, NULL, TO_ROOM );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
	{
		if ( rch->in_room == NULL )
			continue;

		if ( is_affected( rch, gsn_canticle_of_warding ))
			continue;

		if ( is_safe_spell(ch, rch, true))
			continue;

		// damage is level/30 * 20 with half dam done if victim saves vs spell
		dam = ( level / 30 * 20 ) / ( saves_spell( level, rch, DAMTYPE(sn)) ? 2 : 1 );

		damage_spell( ch, rch, dam, sn, DAMTYPE(sn), true );
	}

	if ( IS_SET( ch->in_room->affected_by, ROOMAFF_UTTERDARK ))
		REMOVE_BIT( ch->in_room->affected_by, ROOMAFF_UTTERDARK );

	// check to see if the room is already lit up, no stacked effect
	if ( IS_SET( ch->in_room->affected_by, ROOMAFF_LEONIDS ))
	{
		return FULL_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= 1;
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector	= ROOMAFF_LEONIDS;
						
	affect_to_room( ch->in_room, &af );

	ch->println( "The room is washed in an ethereal glow." );
	act( "The room is washed in an ethereal glow.", ch, NULL, NULL, TO_ROOM );
	return FULL_MANA;
}
/*******************************************************************************/
SPRESULT spell_canticle_of_warding( int, int, char_data *, void *, int )
{
	return NO_MANA;
}
/*******************************************************************************/
/*******************************************************************************/
