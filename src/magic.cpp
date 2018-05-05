/**************************************************************************/
// magic.cpp - spells etc
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
#include "msp.h"
#include "ictime.h"

// command procedures needed
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_far_scan);
DECLARE_DO_FUN(do_bug);
char *format_obj_to_char (OBJ_DATA *, char_data *, bool Short);/*act_info.c*/
bool check_skin( char_data *ch, char_data *victim);
bool check_strength( int level, char_data *ch, char_data *victim, bool &half );
char *get_weapontype(OBJ_DATA *obj);
bool check_social(char_data *ch,char * command, char * argument, bool global);

// Local functions
void	say_spell	args(( char_data *ch, int sn, CLASS_CAST_TYPE type ) );

// imported functions
bool    remove_obj      args( ( char_data *ch, int iWear, bool fReplace ) );

/**************************************************************************/
// Lookup a skill by name - exact match
int skill_exact_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
		if ( IS_NULLSTR(skill_table[sn].name))
			break;
		if ( !str_cmp( name, skill_table[sn].name ) )
			return sn;
    }

    return -1;
}

/**************************************************************************/
// Lookup a skill by name.
int skill_lookup( const char *name )
{
	int sn;

	if(!GAMESETTING5(GAMESET5_SECOND_SKILL_ALIAS_NOT_REQUIRED)){
		if(!str_cmp(name, "second")){
			name="dual wield";
		}    
	}

	sn=skill_exact_lookup(name);

	if(sn>-1){
		return sn;
	}

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( IS_NULLSTR(skill_table[sn].name))
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

/**************************************************************************/
int find_spell( char_data *ch, const char *name, bool spellonly)
{
    // finds a spell the character can cast if possible
    int sn, found = -1;
	int first=0;
	int last=MAX_SKILL;
	
	if(IS_NPC(ch)){
		return skill_lookup(name);
	}

	if(spellonly){
		first=FIRST_SPELL;
		last=LAST_SPELL+1;
	}

	// exact match first
    for ( sn = first; sn < last; sn++ ){
        if (skill_table[sn].name == NULL){
            break;
		}
        if (!str_cmp(name,skill_table[sn].name)){
            if ( found == -1){
                found = sn;
			}
            if (get_skill(ch,sn) > 0){
                return sn;
			}
        }
    }

	// substring match
    for ( sn = first; sn < last; sn++ ){
        if (skill_table[sn].name == NULL){
            break;
		}
        if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
        &&  !str_prefix(name,skill_table[sn].name)){
            if ( found == -1){
                found = sn;
			}
            if (get_skill(ch,sn) > 0){
                return sn;
			}
        }
    }
    return found;
}

/**************************************************************************/
// Utter mystical words for an sn.
void say_spell( char_data *ch, int sn, CLASS_CAST_TYPE type )
{
    char buf  [MSL];
    char buf2 [MSL];
    char_data *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	nw;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

	buf[0]	= '\0';
	for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
	{
		for ( iSyl = 0; (length = str_len(syl_table[iSyl].old)) != 0; iSyl++ )
		{
			if ( !str_prefix( syl_table[iSyl].old, pName ))
			{
				strcat( buf, syl_table[iSyl].nw );
				break;
			}
		}
		
		if ( length == 0 )
			length = 1;
	}

	switch (type){
		case CCT_MAXCAST:
		case CCT_NONE:
		sprintf( buf2, "$n utters the words - THERE IS A BUG, CCT_NONE IN sayspell(), '%s'.", 
			buf );
		sprintf( buf,  "$n utters the words - THERE IS A BUG, CCT_NONE IN sayspell(), '%s'.", 
			skill_table[sn].name );
		break;

		case CCT_MAGE:
		sprintf( buf2, "$n utters the words, '%s'.", buf );
		sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );
		break;

		case CCT_CLERIC:
		sprintf( buf2, "$n prays the words, '%s'.", buf );
		sprintf( buf,  "$n prays the words, '%s'.", skill_table[sn].name );
		break;
		case CCT_DRUID:
		sprintf( buf2, "$n summons the elements of nature using the words, '%s'.", buf );
		sprintf( buf,  "$n summons the elements of nature using the words, '%s'.", skill_table[sn].name );
		break;
		// Bardic songs won't be encrypted by the syl_table
		case CCT_BARD:
		sprintf( buf2, "$n sings the song, '%s'.", skill_table[sn].name );
		sprintf( buf,  "$n sings the song, '%s'.", skill_table[sn].name );
		break;
	}

	// MSP Spell Sound

	if ( !IS_NULLSTR( skill_table[sn].msp_sound )){
		msp_to_room(MSPT_SPELL, skill_table[sn].msp_sound, 
							0, 
							ch,
							false,
							true);
	}

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
		if ( rch != ch ){
			if ( !is_affected(rch, gsn_deafness)) {

				// imms always get the name of the spell
				if(IS_IMMORTAL(rch)){
					act( buf, ch, NULL, rch, TO_VICT );
					continue;
				}

				if(IS_NPC(ch))
				{
					if(IS_SET(ch->act,ACT_CLERIC)){
						act( class_lookup("cleric")==rch->clss ? buf : buf2, ch, NULL, rch, TO_VICT );
					}else if(IS_SET(ch->act,ACT_MAGE)){
						act( class_lookup("mage")==rch->clss ? buf : buf2, ch, NULL, rch, TO_VICT );
					}else{
						act( buf2, ch, NULL, rch, TO_VICT );
					}
				}else{
					act( ch->clss==rch->clss || IS_IMMORTAL(rch)? buf : buf2, ch, NULL, rch, TO_VICT );
				}
			} else {
				if ( !IS_AFFECTED( rch, AFF_BLIND ) || !is_affected( rch, gsn_blindness )){
					act( "You see $N's lips moving.", rch, NULL, ch, TO_CHAR );
				}
			}
		}
    }
    return;
}



/**************************************************************************/
// Compute a saving throw - Negative apply's make saving throw better.
// a higher save value, the better for the victim
bool saves_spell( int level, char_data *victim, int dam_type )
{
    int save;

    save = 50 + ( victim->level - level) * 5 - victim->saving_throw * 2;
    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/2;

	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_IS_UNSEEN))
	{
		return true;
	}

    switch(check_immune(victim,dam_type))
    {
		case IS_IMMUNE:		return true;
		case IS_RESISTANT:	save *= 2;	break;
		case IS_VULNERABLE:	save /= 2;	break;
    }

    if (!IS_NPC(victim) && class_table[victim->clss].fMana)
	save = 9 * save / 10;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/**************************************************************************/
// RT save for dispels 
bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 15;
      // very hard to dispel permanent effects

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}


/**************************************************************************/
// co-routine for dispel magic and cancellation 
// return true if successful
bool check_dispel( int dis_level, char_data *victim, int sn)
{
    AFFECT_DATA *af, *af_next;
	int type;
	bool result=false;
	SPELL_FUN * spell_fun=NULL;

    if (count_affected_by_base_spell(victim, sn)>0)
    {
		// get the parent spell function if appropriate
		if(sn>=FIRST_SPELL && sn<=LAST_SPELL){
			spell_fun= skill_table[sn].spell_fun;
		}
		if(spell_fun==spell_null){
			spell_fun=NULL;
		}

        for ( af = victim->affected; af; af = af_next )
        {
			af_next=af->next;

			type=af->type;
			if(	(		spell_fun
				  &&	type>=FIRST_SPELL 
				  &&	type<=LAST_SPELL 
				  &&	skill_table[type].spell_fun == spell_fun
				)
				|| (type == sn) )
            {
                if (!saves_dispel(dis_level,af->level,af->duration))
                {
					affect_remove( victim, af);

					if ( !IS_NULLSTR(skill_table[type].msg_off) )
					{
						victim->printlnf( "%s", skill_table[type].msg_off );
					}
					result=true;
			}else{ // weaken the spell slightly
					af->level--;
				}
            }
        }
    }
    return result;
}

/**************************************************************************/
bool check_component( char_data *ch, int sn )
{
    OBJ_DATA *obj;

    if ( IS_NPC( ch )) return true;	// mobs don't need components (yet)

    for ( obj = ch->carrying; obj; obj = obj->next_content )
	{
		if ( obj->item_type == ITEM_COMPONENT && obj->value[1] == sn )
		{
			if ( --obj->value[0] == 0 )
			{
				act ( "$p has been consumed by your magic.", ch, obj, NULL, TO_CHAR );
				extract_obj( obj );
			}
			else if ( obj->value[0] < 0 )
			{
				obj->value[0] = -1;
			}
			return true;
		}
	}

	if ( !obj )
	{
		ch->println( "You lack the proper ingredient for this spell." );
		return false;
    }
    return true;
}


/**************************************************************************/
// The kludgy global is for spells who want more stuff from command line.
char *target_name;
void check_realm_improve(char_data * ch, int sn, bool success, int multiplier);
/**************************************************************************/
void do_newcast( char_data *ch, char *argument, int cast_level, CLASS_CAST_TYPE type )
{
    char arg1[MIL];
    char arg2[MIL];
    char_data *victim;
    OBJ_DATA *obj;
	EXIT_DATA *pexit;
	int *pdoor;
    void *vo;
    int mana;
    int sn;
    int target;
    int roll;
    int spell_level;
	int mana_result;
	int door;
	AFFECT_DATA *paf; // For spell grouping
	
    // Switched NPC's can cast spells, but others can't.
    if (IS_UNSWITCHED_MOB(ch))
		return;

	if(!IS_ADMIN( ch ) && IS_OOC( ch )){
		ch->println( "You can not use magic in an ooc area." );
		return;
	}
	
	if ( IS_SET( ch->in_room->room_flags, ROOM_ANTIMAGIC )) {
		ch->println( "Your cannot conjure up a spark of magic here." );
		return;
	}
	
	// Check to see if ch is charmed and being ordered to cast
    if ( IS_AFFECTED(ch,AFF_CHARM) && !IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		ch->println( "You must wait for your master to tell you to cast a spell." );
		return;
	}
	
    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );
	
    if ( IS_NULLSTR(arg1))
    {
		switch (type){
		case CCT_MAXCAST:
		case CCT_NONE:
			ch->println( "Cast which what where? - THERE IS A BUG, CCT_NONE IN do_newcast()");
			ch->println( "please report to the admin!");
			break;
			
		case CCT_MAGE:
			ch->println( "Cast which what where?");
			break;
			
		case CCT_CLERIC:
			ch->println( "Commune which what where?");
			break;
			
		case CCT_DRUID:
			ch->println( "Summon which what where?");
			break;
		case CCT_BARD:
			ch->println( "Sing which song?" );
			break;
		}
		return;
    }

	REMOVE_BIT( ch->dyn, DYN_SUCCESS_CAST );		//  Starts out false

    // check that it is a spell, and skill is above 0% 
    if ( ( sn = find_spell( ch,arg1, true ) ) < 0
		|| get_skill(ch,sn) ==0
		|| !IS_SPELL(sn))
    {
        ch->printlnf( "You don't know any spells of the name '%s'.", arg1 );
        return;
    }
	
	// do a level check if they aren't an imm and the level 
	// on the spell is an imm level
	// a skill of 101 in the skill is a budget hack to make 
	// an acception to this
	if (!IS_NPC(ch))
	{
		if (( (skill_table[sn].skill_level[ch->clss] >=LEVEL_IMMORTAL
			   || (skill_table[sn].skill_level[ch->clss]==0))
			&& !IS_IMMORTAL(ch)) && ch->pcdata->learned[sn] !=101)
		{
			ch->wrapln("This spell is not castable by your class... "
				"even though for some reason you have it... It may have been "
				"removed from your class for game balances purposes... talk to an "
				"admin to get a refund for on the pracs you have spent on it.");
			return;
		}
	}
	
    if ( ch->position < skill_table[sn].minimum_position ){
        ch->println( "You can't concentrate enough." );
		if(skill_table[sn].minimum_position==POS_STANDING && ch->position<POS_FIGHTING){
			ch->println( "Perhaps you should try standing first.");
		}
        return;
    }
	
	if ( IS_SET(skill_table[sn].sect_restrict, 1<<ch->in_room->sector_type) )
	{
		ch->printlnf( "You can't seem to cast %s %s.", 
			skill_table[sn].name, 
			sector_desc[ch->in_room->sector_type].name);
		return;
	}
	
    mana = skill_table[sn].min_mana;

	bool improvecheck_mana_focusing_sucess=false;
    if(get_skill(ch,gsn_mana_focusing))
    {
		if( (roll = number_range(1,100)) < get_skill(ch,gsn_mana_focusing))
		{
			mana-=mana*(roll/2)/100;
			improvecheck_mana_focusing_sucess=true;
		}
    }
	
    /*
	* Locate targets.
	*/
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;
	
    switch ( skill_table[sn].target )
    {
    default:
        bugf( "Do_cast: bad target for sn %d.", sn );
        return;
		
    case TAR_IGNORE:
		// Check to see if the spell is component based and if they've got it
		if ( skill_table[sn].component_based )
		{
			if ( !check_component( ch, sn ))
				return;
		}
        break;
		
    case TAR_MOB_OFFENSIVE:
    case TAR_CHAR_OFFENSIVE:
        if ( arg2[0] == '\0' )
        {
            if ( ( victim = ch->fighting ) == NULL )
            {
                ch->println( "Cast the spell on whom?" );
                return;
            }
        }
        else
        {
            if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
            {
                ch->printlnf( "Can't see any '%s' here.", target_name );
				if (ch->desc && ch->desc->repeat>5){
					WAIT_STATE( ch, 3 * PULSE_VIOLENCE );
				}
                return;
            }
        }
		
		// target restriction checks
		if((IS_NPC(victim) && IS_SET(skill_table[sn].flags,SKFLAGS_NO_NPCTARGET))
			||(!IS_NPC(victim) && IS_SET(skill_table[sn].flags,SKFLAGS_NO_PCTARGET)))
		{
            ch->println( "Not on that target." );
            return;
		}
		
		// is safe check
        if ( !IS_NPC(ch) ) 
		{   
            if (is_safe(ch,victim) && victim != ch)
            {
                ch->println( "Not on that target." );
                return;
            }
        }

		if ( !can_initiate_combat( ch, victim, CIT_GENERAL | CIT_CASTING_SPELL )) return;

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
        {
            ch->println( "You can't do that on your own follower." );
            return;
        }
		
        vo = (void *) victim;
        target = TARGET_CHAR;
		
		// Check to see if the spell is component based and if they've got it
		if ( skill_table[sn].component_based )
		{
			if ( !check_component( ch, sn ))
				return;
		}
        break;
		
    case TAR_CHAR_DEFENSIVE:
        if ( arg2[0] == '\0' )
        {
            victim = ch;
        }
        else
        {
            if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
            {
				ch->printlnf( "You can't see any '%s' here.", target_name );
				return;
            }
        }
		
		// Start Grouping check
		for (paf = victim->affected; paf; paf=paf->next)
			for(int i=0; spell_group_flags[i].name; i++)
				if (   IS_SET(skill_table[sn].spellgroup, spell_group_flags[i].bit)
					&& IS_SET(skill_table[paf->type].spellgroup, spell_group_flags[i].bit) )
				{
					if (victim == ch)
						ch->println( "But you're already affected by something similar!");
					else
						ch->printlnf( "But %s is already affected by something similar!", victim->short_descr);
					return;
				}
				// End Grouping check
				
				vo = (void *) victim;
				target = TARGET_CHAR;
				
				// Check to see if the spell is component based and if they've got it
				if ( skill_table[sn].component_based )
				{
					if ( !check_component( ch, sn ))
						return;
				}
				break;
				
    case TAR_CHAR_SELF:
        if ( !IS_NULLSTR(arg2)
			&& !str_cmp(target_name, "self") 
			&& !is_name( target_name, ch->name ) )
        {
            ch->println( "You cannot cast this spell on another." );
            return;
        }
		
		// Start Grouping check
		for (paf = ch->affected; paf; paf=paf->next)
			for(int i=0; spell_group_flags[i].name; i++)
				if (   IS_SET(skill_table[sn].spellgroup, spell_group_flags[i].bit)
					&& IS_SET(skill_table[paf->type].spellgroup, spell_group_flags[i].bit) )
				{
					ch->println( "But you're already affected by something similar!" );
					return;
				}
				// End Grouping check
				
				vo = (void *) ch;
				target = TARGET_CHAR;
				
				// Check to see if the spell is component based and if they've got it
				if ( skill_table[sn].component_based )
				{
					if ( !check_component( ch, sn ))
						return;
				}
				break;
				
    case TAR_OBJ_INV:
        if ( arg2[0] == '\0' )
        {
            ch->println( "What should the spell be cast upon?" );
			return;
        }
		
        if ( ( obj = get_obj_carry( ch, target_name ) ) == NULL )
        {
            ch->printlnf( "You are not carrying any '%s'.", target_name );
            return;
        }
		
        vo = (void *) obj;
        target = TARGET_OBJ;
		
		// Check to see if the spell is component based and if they've got it
		if ( skill_table[sn].component_based )
		{
			if ( !check_component( ch, sn ))
				return;
		}
        break;
		
	case TAR_DIRECTION:
        if (arg2[0] == '\0')
		{
			ch->println( "Cast the spell in what direction?" );
			return;
		}
		
		door = dir_lookup( arg2);
		
		if ( door == -1 )
		{
			ch->printlnf( "'%s' is an invalid direction.", arg2 ); 
			return;
		}
		
		if ( (pexit = ch->in_room->exit[door]) == NULL || pexit->u1.to_room == NULL )
		{
			ch->println( "There is no exit in that direction." );
			return;
		}
		
		pdoor = &door;
		
		vo = (void *) pdoor;
		target = TARGET_CHAR;
		
		// Check to see if the spell is component based and if they've got it
		if ( skill_table[sn].component_based )
		{
			if ( !check_component( ch, sn ))
				return;
		}
		break;
		
    case TAR_OBJ_MOB_OFF:
    case TAR_OBJ_CHAR_OFF:
        if (arg2[0] == '\0')
        {
            if ((victim = ch->fighting) == NULL)
            {
				ch->println( "Cast the spell on whom or what?" );
				return;
            }
			
            target = TARGET_CHAR;
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {
            target = TARGET_CHAR;
        }
		
        if (target == TARGET_CHAR) // check the sanity of the attack        		
		{
			// target restriction checks
			if((IS_NPC(victim) && IS_SET(skill_table[sn].flags,SKFLAGS_NO_NPCTARGET))
				||(!IS_NPC(victim) && IS_SET(skill_table[sn].flags,SKFLAGS_NO_PCTARGET)))
			{
				ch->println( "Not on that target." );
				return;
			}
			
			// is safe check
            if(is_safe_spell(ch,victim,false) && victim != ch)
            {
				ch->println( "Not on that target." );
				return;
            }
			
            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                ch->println( "You can't do that on your own follower." );
                return;
			}
			
            vo = (void *) victim;
        }
        else if ((obj = get_obj_here(ch,target_name)) != NULL)
        {
            vo = (void *) obj;
            target = TARGET_OBJ;
        }
        else
        {
            ch->printlnf( "You don't see any '%s' here.", target_name );
            return;
        }
		// Check to see if the spell is component based and if they've got it
		if ( skill_table[sn].component_based )
		{
			if ( !check_component( ch, sn ))
				return;
		}
        break; 
		
    case TAR_OBJ_CHAR_DEF:
        if (arg2[0] == '\0')
		{
			// Start Grouping check
			for (paf = ch->affected; paf; paf=paf->next)
				for(int i=0; spell_group_flags[i].name; i++)
					if (   IS_SET(skill_table[sn].spellgroup, spell_group_flags[i].bit)
						&& IS_SET(skill_table[paf->type].spellgroup, spell_group_flags[i].bit) )
					{
						ch->println( "But you're already affected by something similar!" );
						return;
					}
					// End Grouping check
					
					vo = (void *) ch;
					target = TARGET_CHAR;
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {
			// Start Grouping check
			for (paf = victim->affected; paf; paf=paf->next)
				for(int i=0; spell_group_flags[i].name; i++)
					if (   IS_SET(skill_table[sn].spellgroup, spell_group_flags[i].bit)
						&& IS_SET(skill_table[paf->type].spellgroup, spell_group_flags[i].bit) )
					{
						ch->printlnf( "But %s is already affected by something similar!", victim->short_descr);
						return;
					}
					//End Grouping check
					
					vo = (void *) victim;
					target = TARGET_CHAR;
        }
        else if ((obj = get_obj_carry(ch,target_name)) != NULL)
        {
            vo = (void *) obj;
            target = TARGET_OBJ;
        }
        else
        {
            ch->printlnf( "You don't see any '%s' here.", target_name );
            return;
        }
		
		// Check to see if the spell is component based and if they've got it
		if ( skill_table[sn].component_based ){
			if ( !check_component( ch, sn )){
				return;
			}
		}
        break;
    }// end of spell type switch
	
    if ( !IS_NPC(ch) && ch->mana < mana ){
        ch->println( "You don't have enough mana." );
        return;
    }
	
    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) ){
        say_spell( ch, sn, type );
	}
	
    WAIT_STATE( ch, skill_table[sn].beats );
	
	// can't spam charming yourself to improve at it
	if (sn==gsn_charm_person && victim==ch)
	{
		if (number_percent( ) > get_skill(ch,sn) )
		{
			ch->mana -= mana / 2;
			ch->println( "You lost your concentration." );
		}
		else
		{
			ch->mana -= mana;
			ch->println( "You like yourself even better!" );
		}
		return;
	}

    if ( number_percent( ) > (get_skill(ch,sn) + ch->modifiers[STAT_SD] - 5))
    {
		ch->mana -= mana / 2;
		ch->println( "You lost your concentration." );
		check_improve(ch,sn,false,1);
		if(get_skill(ch,gsn_mana_focusing)){
			check_improve(ch,gsn_mana_focusing, improvecheck_mana_focusing_sucess, 5);				
		}
    }
    else
    {
		bool improvecheck_sorcery_sucess=false;
        spell_level=ch->level;
		
		// moon casting mods by Kalahn
		// mage 
		if (HAS_CLASSFLAG(ch, CLASSFLAG_CASTING_AFFECTED_BY_MOON)){
			spell_level += weather_info[ch->in_room->sector_type].mage_castmod;
			if (spell_level<1)
				spell_level=1;
			
		}
		// spellfilcher at half
		if (HAS_CLASSFLAG(ch, CLASSFLAG_CASTING_HALFAFFECTED_BY_MOON)){
			spell_level += (weather_info[ch->in_room->sector_type].mage_castmod/2);
			if (spell_level<1)
				spell_level=1;
			
		}
				
		if(get_skill(ch,gsn_sorcery)){
			if( (roll = number_range(1,100)) < get_skill(ch,gsn_sorcery)){
				spell_level+=roll/5;
				improvecheck_sorcery_sucess=true;
			}
		}
		
		if ( IS_SET(skill_table[sn].sect_dampen, 1<<ch->in_room->sector_type) ){
			spell_level -= number_range(1,20);
		}else if ( IS_SET(skill_table[sn].sect_enhance, 1<<ch->in_room->sector_type) ){
			spell_level += number_range(1,10);
		}
		
		// cast_level override if casted from do_castatlevel
		if (cast_level){
			spell_level=cast_level;
		}

		if (!skill_table[sn].spell_fun){
			ch->printlnf( "'%s' is not a spell!", skill_table[sn].name);
			return;
		}

		// spell cast on barbarian turns into a spell_fear_magic instead.
		if( victim 
			&& HAS_CLASSFLAG(victim, CLASSFLAG_MAGIC_ANTIPATHY) 
			&& number_range(1,3)==1 
			&& IS_SET(skill_table[sn].flags, SKFLAGS_MAGICAL_ANTIPATHY) 
			&& !victim->fighting)
		{
			ch->printlnf( "Your spell has set %s into a maddening frenzy!", victim->short_descr);
			sn = gsn_fear_magic;
		}
		
		SET_BIT( ch->dyn, DYN_SUCCESS_CAST );

		if (IS_NPC(ch) || class_table[ch->clss].fMana){
			// clss has spells 
			mana_result = (*skill_table[sn].spell_fun) ( sn, spell_level, ch, vo,target);
        }else{
            mana_result = (*skill_table[sn].spell_fun) (sn, spell_level, ch, vo,target);
		}
		
		switch( mana_result )
		{
		case NO_MANA:
			mana = 0;
			break;
		case HALF_MANA:
			if ( mana > 0 )
				mana /= 2;
			break;
		case DOUBLE_MANA:
			if ( mana > 0 )
				mana *= 2;
			break;
		case ALL_MANA:
			mana = ch->mana;
		default:
			break;
		}
        ch->mana -= mana;

		// handle improvement for mana_focusing and sorcery
		if(mana_result!=NO_MANA){
			// improvement is delayed so we don't improve for these things
			// if no actual mana was required
			if(get_skill(ch,gsn_mana_focusing)){
				check_improve(ch,gsn_mana_focusing, improvecheck_mana_focusing_sucess, 5);				
			}
			if(get_skill(ch,gsn_sorcery)){
				check_improve(ch,gsn_sorcery, improvecheck_sorcery_sucess, 5);
			}

			// give them a chance at improving their realm/sphere etc
			check_realm_improve(ch, sn, true, 2);
		}
		
		if (!IS_NPC(ch)){
			// record when the spell was last used
			//ch->pcdata->last_used[sn]= current_time;
		}
		
        check_improve(ch,sn,true,1);
    }
	
    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
		||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR)
		||   (skill_table[sn].target == TAR_OBJ_MOB_OFF && target == TARGET_CHAR && IS_NPC(victim))
		||   (skill_table[sn].target == TAR_MOB_OFFENSIVE && IS_NPC(victim)))
		&&   ch
		&&   ch->in_room
		&&   victim != ch
		&&   victim->master != ch)
    {
        char_data *vch;
        char_data *vch_next;
		
        for ( vch = ch->in_room->people; vch; vch = vch_next )
        {
            vch_next = vch->next_in_room;
            if ( victim == vch && victim->fighting == NULL )
            { 
				multi_hit( victim, ch, TYPE_UNDEFINED );
				break;
            }
        }
    }
	
	if (skill_table[sn].target == TAR_MOB_OFFENSIVE && !IS_NPC(victim) && ch!=victim)
	{
		ch->pksafe=0;
		if (ch->pknorecall<5)
			ch->pknorecall=5;
	}
	
    return;
}

/**************************************************************************/
void do_summon( char_data *ch, char *argument )
{
	if(!IS_NPC(ch) && (TRUE_CH(ch)->level<LEVEL_IMMORTAL) ){
		if(class_table[ch->clss].class_cast_type==CCT_NONE){
			ch->println( "What would you know about summoning nature?" );
			return;
		}

		if(class_table[ch->clss].class_cast_type!=CCT_DRUID){
			ch->println( "Try cast or commune." );
			return;
		}
	}
	do_newcast( ch, argument, 0, CCT_DRUID);
}
/**************************************************************************/
void do_commune( char_data *ch, char *argument )
{
	if(!IS_NPC(ch) && (TRUE_CH(ch)->level<LEVEL_IMMORTAL) ){
		if(class_table[ch->clss].class_cast_type==CCT_NONE){
			ch->println( "What would you know about praying?" );
			return;
		}

		if(class_table[ch->clss].class_cast_type!=CCT_CLERIC){
			ch->println( "Try cast or summon." );
			return;
		}
	}
	do_newcast( ch, argument, 0, CCT_CLERIC);
}
/**************************************************************************/
void do_sing( char_data *ch, char *argument )
{
	if(!IS_NPC(ch) && (TRUE_CH(ch)->level<LEVEL_IMMORTAL) )
	{
		if ( class_table[ch->clss].class_cast_type != CCT_BARD )
		{	// default to the sing social
			check_social( ch, "sing", argument, false );
			return;
		}
	}
	do_newcast( ch, argument, 0, CCT_BARD);
}
/**************************************************************************/
void do_cast( char_data *ch, char *argument )
{
	if(!IS_NPC(ch) && (TRUE_CH(ch)->level<LEVEL_IMMORTAL) ){		
		if(class_table[ch->clss].class_cast_type==CCT_NONE){
			if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){
				do_newcast( ch, argument, 0, CCT_MAGE);
			}else{
				ch->println( "What would you know about magic?" );
			}
			return;
		}

		if(class_table[ch->clss].class_cast_type!=CCT_MAGE){
			ch->println( "Try commune or summon." );
			return;
		}
	}

	// mobs with the cleric bit don't cast spells, they 'pray' them
	if(IS_NPC(ch) && IS_SET(ch->act,ACT_CLERIC)){
		do_newcast( ch, argument, 0, CCT_CLERIC);
		return;
	}

	do_newcast( ch, argument, 0, CCT_MAGE);
}
/**************************************************************************/
void do_cast_redirect( char_data *ch, char *argument )
{
	if(class_table[ch->clss].class_cast_type==CCT_CLERIC){
		do_commune(ch,argument);
		return;
	}
	if(class_table[ch->clss].class_cast_type==CCT_DRUID){
		do_summon(ch,argument);
		return;
	}
	do_cast(ch,argument);
}

/**************************************************************************/
void do_castatlevel( char_data *ch, char *argument)
{
    char arg[MIL];
	int max_level;

	if (!IS_UNSWITCHED_MOB(ch))
		max_level=ch->level*3;
	else
		max_level=TRUE_CH(ch)->level;

    if (max_level >= MAX_LEVEL*3){
        max_level = MAX_LEVEL*6;
    }

	if ( IS_SET( ch->in_room->room_flags, ROOM_ANTIMAGIC )) {
		ch->println( "You cannot conjure up a spark of magic here." );
		return;
	}
	
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->println( "Castatlevel what?" );
        return;
    }

    if (is_number(arg))
    {
        int new_level = atoi(arg);
        if ((new_level<=max_level) && (new_level>0))
        {
            ch->printlnf( "ATLEVELCAST %d BEGIN.", new_level);
			do_newcast( ch, argument, new_level, CCT_MAGE );
            ch->printlnf( "ATLEVELCAST %d FINISHED.", new_level);
            return;

        }
        else // not high enough trust
        {
            ch->printlnf( "Level must be between 1 and %d.", max_level);
            return;
        }
    }

	ch->println( "Level must be a valid number." );
    return;
}

/**************************************************************************/
// Cast spells at targets using a magical object.
void obj_cast_spell( int sn, int level, char_data *ch, char_data *victim, OBJ_DATA *obj )
{
    void *vo;
    int target = TARGET_NONE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
        bugf( "Obj_cast_spell: bad sn %d.", sn );
        return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bugf( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
        vo = NULL;
        break;

    case TAR_CHAR_OFFENSIVE:
    case TAR_MOB_OFFENSIVE:
        if ( victim == NULL )
            victim = ch->fighting;
        if ( victim == NULL )
        {
            ch->println( "You can't do that." );
            return;
        }
        if (is_safe(ch,victim) && ch != victim)
        {
            ch->println( "Something isn't right..." );
            return;
        }
        vo = (void *) victim;
        target = TARGET_CHAR;
        break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
        if ( victim == NULL ){
            victim = ch;
		}
        vo = (void *) victim;
        target = TARGET_CHAR;
        break;

    case TAR_OBJ_INV:
        if ( obj == NULL )
        {
            ch->println( "You can't do that." );
            return;
        }
        vo = (void *) obj;
        target = TARGET_OBJ;
        break;

    case TAR_OBJ_CHAR_OFF:
        if ( victim == NULL && obj == NULL){
			if (ch->fighting != NULL){
				victim = ch->fighting;
			}else{
				ch->println( "You can't do that." );
				return;
			}
		}

	    if (victim != NULL)
        {
            if (is_safe_spell(ch,victim,false) && ch != victim)
            {
                ch->println( "Something isn't right..." );
                return;
            }   
            vo = (void *) victim;
            target = TARGET_CHAR;
	    }
	    else
	    {
	    	vo = (void *) obj;
	    	target = TARGET_OBJ;
	    }
        break;


    case TAR_OBJ_CHAR_DEF:
        if (victim == NULL && obj == NULL)
        {
             vo = (void *) ch;
            target = TARGET_CHAR;
        }
        else if (victim != NULL)
        {
            vo = (void *) victim;
            target = TARGET_CHAR;
        }
        else
        {
            vo = (void *) obj;
            target = TARGET_OBJ;
        }    
        break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);


    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
        char_data *vch;
        char_data *vch_next;
    
        for ( vch = ch->in_room->people; vch; vch = vch_next )
        {
            vch_next = vch->next_in_room;
            if ( victim == vch && victim->fighting == NULL )
            {
            multi_hit( victim, ch, TYPE_UNDEFINED );
            break;
            }
        }
    }

    return;
}


/**************************************************************************/
//     Spell functions.
/**************************************************************************/

/**************************************************************************/
SPRESULT spell_acid_blast( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	int dam;

	dam = dice( level, 10 );
	if ( saves_spell( level, victim, DAMTYPE(sn)))
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_armor( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( is_affected( victim, sn ) )
	{
		if (victim == ch)
			ch->println( "You are already armored." );
		else
			act("$N is already armored.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where	 = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 24;
	af.modifier  = -20;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	victim->println( "You feel someone protecting you." );

	if ( ch != victim )
		act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
    return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_bless( int sn, int level, char_data *ch, void *vo, int target)
{
	char_data *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	// deal with the object case first 
	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
		{
			act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
			return HALF_MANA;
		}
		
		if (IS_OBJ_STAT(obj,OBJEXTRA_EVIL))
		{
			AFFECT_DATA *paf;
			
			paf = affect_find(obj->affected,gsn_curse);
			if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
			{
				if (paf != NULL)
					affect_remove_obj(obj,paf);
				act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
				REMOVE_BIT(obj->extra_flags,OBJEXTRA_EVIL);
				return FULL_MANA;
			}
			else
			{
				act("The evil of $p is too powerful for you to overcome.",
					ch,obj,NULL,TO_CHAR);
				return FULL_MANA;
			}
		}
		
		af.where	= WHERE_OBJEXTRA;
		af.type		= sn;
		af.level	= level;
		af.duration	= 6 + level;
		af.location	= APPLY_SAVES;
		af.modifier	= -1;
		af.bitvector= OBJEXTRA_BLESS;
		affect_to_obj(obj,&af);
		
		act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);
		return FULL_MANA;
    }

    // character target 
    victim = (char_data *) vo;

	if(is_affected( victim, gsn_curse )){
		if( check_dispel( level, victim, gsn_curse )){
			act( "Your magic unravels a spell on $N.", ch, NULL, victim, TO_CHAR );
			return FULL_MANA;
        }else{
			ch->println("You failed.");
			return FULL_MANA;
        }
	}

	if(is_affected( victim, sn )){
		if (victim == ch){
			ch->println( "You are already blessed." );
		}else{
			act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 6+level;
	af.location  = APPLY_HITROLL;
	af.modifier  = level / 8;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	af.location  = APPLY_SAVES;
	af.modifier  = 0 - level / 8;
	affect_to_char( victim, &af );
	victim->println( "You feel righteous." );

	if ( ch != victim )
		act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_blindness( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_BLIND))
	{
		ch->printlnf( "%s already appears to be blinded.", PERS(victim, ch));
		return HALF_MANA;
	}

	if (saves_spell(level,victim,DAMTYPE(sn)))
	{
		ch->printlnf( "%s seems unaffected.", PERS(victim, ch) );
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.location  = APPLY_HITROLL;
	af.modifier  = -4;
	af.duration  = 1+level;
	af.bitvector = AFF_BLIND;
	affect_to_char( victim, &af );
	victim->println( "You are blinded!" );
	act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_burning_hands(int sn,int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	static const sh_int dam_each[] = 
	{
		0,
		0,  0,  0,  0,	14,	17, 20, 23, 26, 29,
		29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
		34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
		39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
		44, 44, 45, 45,	46,	46, 47, 47, 48, 48
	};
   
	int dam;

    level	= UMIN(level, (int)sizeof(dam_each)/(int)sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAMTYPE(sn)) )
		dam /= 2;
    damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_call_lightning( int sn, int level,char_data *ch,void *,int )
{
	char_data *vch;
	char_data *vch_next;
	int dam;

	if ( !IS_OUTSIDE(ch) )
	{
		ch->println( "You must be out of doors." );
		return NO_MANA;
	}

	if ( ch->in_room->sector_type == SECT_CAVE )
	{
		ch->println( "You cannot cast this spell here." );
		return NO_MANA;
	}

	if ( weather_info[ch->in_room->sector_type].sky < SKY_RAINING )
	{
		ch->println( "You need bad weather." );
		return HALF_MANA;
	}

	dam = dice(level/2, 8);

	ch->println( "Lightning from the sky strikes your foes!" );
	act( "$n calls upon the lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM );

    for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
        vch_next=vch->next_in_room;
		if ( vch->in_room == NULL )
			continue;
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
				damage_spell( ch, vch, saves_spell( level, vch, DAMTYPE( sn )) 
					? dam / 2 : dam, sn, DAMTYPE(sn), true);
			continue;
		}

		if ( vch->in_room->area == ch->in_room->area
		&&   IS_OUTSIDE(vch)
		&&   vch->in_room->sector_type != SECT_CAVE
		&&   IS_AWAKE(vch) )
		vch->println( "Lightning flashes in the sky." );
	}

	return FULL_MANA;
}


/**************************************************************************/
// RT calm spell stops all fighting in the room

SPRESULT spell_calm( int sn, int level, char_data *ch, void *,int )
{
	char_data *vch;
	int mlevel = 0;
	int count = 0;
	int high_level = 0;    
	int chance;
	AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if (vch->position == POS_FIGHTING)
		{
			count++;
			if (IS_NPC(vch))
				mlevel += vch->level;
			else
				mlevel += vch->level/2;
			high_level = UMAX(high_level,vch->level);
		}
	}

    /* compute chance of stopping combat */
	chance = 4 * level - high_level + 2 * count;

	if ( IS_IMMORTAL( ch )) /* always works */
		mlevel = 0;

	if ( number_range( 0, chance ) >= mlevel )  /* hard to stop large fights */
	{
		for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
		{
			if ( IS_NPC( vch )
			&& ( IS_SET( vch->imm_flags, IMM_MAGIC )
			||   IS_SET( vch->act, ACT_UNDEAD )))
			return FULL_MANA;

			if ( IS_AFFECTED( vch, AFF_CALM )
			||   IS_AFFECTED( vch, AFF_BERSERK )
			||   is_affected( vch, gsn_frenzy ))
			return FULL_MANA;
	    
			vch->println( "A wave of calm passes over you." );

			if (vch->fighting || vch->position == POS_FIGHTING )
				stop_fighting( vch, false );


			af.where = WHERE_AFFECTS;
			af.type = sn;
			af.level = level;
			af.duration = level/4;
			af.location = APPLY_HITROLL;

			if (!IS_NPC(vch))
				af.modifier = -5;
			else
				af.modifier = -2;

			af.bitvector = AFF_CALM;
			affect_to_char(vch,&af);
			af.location = APPLY_DAMROLL;
			affect_to_char(vch,&af);
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_cancellation( int , int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	char_data *roomchar;
	bool found = false;

	level += 2;

	if ((!IS_NPC(ch) && IS_NPC(victim) &&
		!(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
		 (IS_NPC(ch) && !IS_NPC(victim)) )
	{
		ch->println( "You failed, try dispel magic." );
		return HALF_MANA;
	}

	// unlike dispel magic, the victim gets NO save 

	// begin running through the spells 

	if ( IS_SET( ch->in_room->affected_by, ROOMAFF_UTTERDARK )) {
		ch->println( "The darkness around you disappears." );
		act( "The darkness around you disappears.", ch, NULL, NULL, TO_ROOM );
		REMOVE_BIT( ch->in_room->affected_by, ROOMAFF_UTTERDARK );
		found = true;
	}

 	if (check_dispel(level,victim, gsn_regeneration ))
		found = true;
	if (check_dispel(level,victim, gsn_resist_poison ))
		found = true;
	if (check_dispel(level,victim, gsn_poison_immunity ))
		found = true;
	if (check_dispel(level,victim, gsn_prismatic_spray ))
		found = true;
	if (check_dispel(level,victim, gsn_illusions_grandeur ))
		found = true;
	if (check_dispel(level,victim, gsn_protection_fire ))
		found = true;
	if (check_dispel(level,victim, gsn_protection_cold ))
		found = true;
	if (check_dispel(level,victim, gsn_protection_lightning ))
		found = true;
	if (check_dispel(level,victim, gsn_holy_aura ))
		found = true;
	if (check_dispel(level,victim, gsn_true_sight ))
		found = true;
	if (check_dispel(level,victim, gsn_barkskin ))
		found = true;
	if (check_dispel(level,victim, gsn_magic_resistance ))
		found = true;
	if (check_dispel(level,victim, gsn_fire_shield ))
		found = true;
	if (check_dispel(level,victim, gsn_chill_shield ))
		found = true;
	if (check_dispel(level,victim, gsn_animal_essence))
		found = true;
    if (check_dispel(level,victim, gsn_armor ))
		found = true;
	if (check_dispel(level, victim, gsn_bless ))
		found = true;
	if (check_dispel(level,victim, gsn_blindness ))
	{
		found = true;
		act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
	if (check_dispel(level,victim, gsn_calm ))
	{
		found = true;
		act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
	}
    if (check_dispel(level,victim, gsn_change_sex ))
    {
        found = true;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,victim, gsn_charm_person))
	{
		found = true;
		act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
	}
	if (check_dispel(level,victim, gsn_chill_touch ))
	{
		found = true;
		act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
	}
	if (check_dispel(level,victim, gsn_curse ))
		found = true;
	if (check_dispel(level,victim, gsn_detect_evil ))
		found = true;
	if (check_dispel(level,victim, gsn_detect_good ))
		found = true;
    if (check_dispel(level,victim, gsn_detect_hidden ))
        found = true;
	if (check_dispel(level,victim, gsn_detect_invis ))
		found = true;
	if (check_dispel(level,victim, gsn_detect_magic ))
		found = true;
	if (check_dispel(level,victim, gsn_faerie_fire ))
	{
		act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_fly ))
	{
		act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_frenzy ))
	{
		act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
		found = true;
	}
	if (check_dispel(level,victim, gsn_giant_strength ))
	{
		act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_haste ))
	{
		act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_infravision ))
		found = true;
	if (check_dispel(level,victim, gsn_invisibility ))
	{
		act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
    if (check_dispel(level,victim, gsn_mass_invis ))
    {
		act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_pass_door ))
        found = true;
	if (check_dispel(level,victim, gsn_protection_evil ))
        found = true;
	if (check_dispel(level,victim, gsn_protection_good ))
        found = true; 
	if (check_dispel(level,victim, gsn_rage ))
		found = true;
	if (check_dispel(level,victim, gsn_sanctuary ))
	{
		act("The white aura around $n's body vanishes.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_shield ))
	{
		act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_sleep ))
		found = true;
	if (check_dispel(level,victim, gsn_slow ))
	{
		act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_stone_skin ))
	{
		act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_weaken ))
	{
		act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	
	// Cancelation of the vanish skill.
	for ( roomchar = ch->in_room->people; roomchar != NULL; roomchar = roomchar->next_in_room )
	{
		if ( IS_AFFECTED2(roomchar, AFF2_VANISH))
		{
			affect_parentspellfunc_strip( roomchar, gsn_vanish);
			REMOVE_BIT( roomchar->affected_by2, AFF2_VANISH );
			act( "A swirl of dust reveals $n.", roomchar, NULL, NULL, TO_ROOM );
			roomchar->println( "You have been revealed." );
			found = true;
		}
	}

	if (!found) {
        ch->println( "Spell failed." );
		return HALF_MANA;
	}

	ch->println( "Ok." );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_cause_light( int sn, int level, char_data *ch, void *vo,int  )
{
	damage_spell( ch, (char_data *) vo, dice(1, 8) + level / 3, sn, DAMTYPE( sn ),true);
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_cause_critical(int sn,int level,char_data *ch,void *vo,int )
{
	damage_spell( ch, (char_data *) vo, dice(3, 8) + level - 6, sn, DAMTYPE( sn ), true);
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_cause_serious(int sn,int level,char_data *ch,void *vo,int )
{
	damage_spell( ch, (char_data *) vo, dice(2, 8) + level / 2, sn, DAMTYPE( sn ), true);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_chain_lightning(int sn,int level,char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	char_data *tmp_vict,*last_vict,*next_vict;
	bool found;
	int dam;

	// first strike 
	act("A lightning bolt leaps from $n's hand and arcs to $N.", ch,NULL,victim,TO_ROOM);
	act("A lightning bolt leaps from your hand and arcs to $N.", ch,NULL,victim,TO_CHAR);
	act("A lightning bolt leaps from $n's hand and hits you!",   ch,NULL,victim,TO_VICT);  

	dam = dice(level,5);
	if (saves_spell(level,victim, DAMTYPE(sn))){
		dam /= 3;
	}
	damage_spell(ch,victim,dam,sn,DAMTYPE(sn),true);
	last_vict = victim;
    level -= 4;   // decrement damage

    // new targets 
	while (level > 0)
	{
		found = false;
		for (tmp_vict = ch->in_room->people; tmp_vict != NULL; tmp_vict = next_vict) 
		{
			next_vict = tmp_vict->next_in_room;
			if (!is_safe_spell(ch,tmp_vict,true)
				&& tmp_vict != last_vict )
			{
				found = true;
				last_vict = tmp_vict;
				act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
				act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
				dam = dice(level,5);
				if (saves_spell(level,tmp_vict,DAMTYPE(sn))){
					dam /= 3;
				}
				damage_spell(ch,tmp_vict,dam,sn,DAMTYPE(sn),true);
				level -= 4;  // decrement damage
			}
		}   // end target searching loop 
	
		if (!found) // no target found, hit the caster 
		{
			if (ch == NULL)
				return FULL_MANA;

			if (last_vict == ch) // no double hits 
			{
				act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
				act("The bolt grounds out through your body.",ch,NULL,NULL,TO_CHAR);
				return FULL_MANA;
			}

			last_vict = ch;
			act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
			ch->println( "You are struck by your own lightning!" );
			dam = dice(level,6);
			if (saves_spell(level,ch,DAMTYPE(sn))){
				dam /= 3;
			}
			damage_spell(ch,ch,dam,sn,DAMTYPE(sn),true);
			level-= 4;  // decrement damage
			if (ch==NULL || ch->in_room==NULL) {
				return FULL_MANA;
			}
		}
		// now go back and find more targets 
	}
	return FULL_MANA;
}
	  
/**************************************************************************/
SPRESULT spell_change_sex( int sn, int level, char_data *ch, void *vo,int  )
{
    char_data *victim = (char_data *) vo;
    AFFECT_DATA af;

	if ( is_affected( victim, sn ))
	{
		if (victim == ch)
			ch->println( "You've already been changed." );
		else
			act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	if (saves_spell(level , victim,DAMTYPE(sn)))
		return FULL_MANA;

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 2 * level;
	af.location  = APPLY_SEX;

	do { af.modifier  = number_range( 0, 2 ) - victim->sex; }
    while ( af.modifier == 0 );

	af.bitvector = 0;
	affect_to_char( victim, &af );
	victim->println( "You feel different." );
	act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_charm_person( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if (is_safe(ch,victim)) 
	{
		ch->printlnf( "Your spell doesnt appear to have any affect on %s.", PERS(victim, ch));
		return HALF_MANA;
	}

	if ( victim == ch )
	{
		ch->println( "You like yourself even better!" );
		return HALF_MANA;
	}

	if ( IS_AFFECTED(victim, AFF_CHARM)
	||   IS_AFFECTED(ch, AFF_CHARM)
	||   level < victim->level
	||   IS_SET(victim->imm_flags,IMM_CHARM)
	||	 victim->modifiers[STAT_PR]>number_range(0,40)
	||   saves_spell( level, victim, DAMTYPE(sn) ) 
	||   (!IS_NPC(victim) 
			&& !GAMESETTING2(GAMESET2_NOCHARM_HAS_NOAFFECT)
			&& HAS_CONFIG(victim,CONFIG_NOCHARM) 
		 )
		 ){
		ch->println( "You failed." );
		return FULL_MANA;
	}

	if (!IS_NPC(victim) && IS_SET(victim->in_room->room_flags,ROOM_LAW))
	{
		ch->println( "The mayor does not allow charming in the city limits." );
		return HALF_MANA;
	}
  
	if ( victim->master ) {
		stop_follower( victim );
	}
    
	add_follower( victim, ch );
	victim->leader = ch;
	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = number_fuzzy( level / 4 );
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char( victim, &af );

	act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );

	if ( ch != victim ) {
		act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
	}

	if(IS_NPC(victim)) {
		REMOVE_BIT(victim->act, ACT_AGGRESSIVE);
	}
		
	// stop those who are charmed from logging out
	if (!IS_NPC(victim) && ch!=victim)
	{
		ch->pksafe=0;
		ch->pknorecall= UMAX(ch->pknorecall,2);
		ch->pknoquit  = UMAX(ch->pknorecall,7);

		if (!IS_NPC(ch))
		{
			victim->pknorecall= UMAX(victim->pknorecall,4);
			victim->pknoquit=UMAX(victim->pknoquit,10);
		}
	}
    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_chill_touch( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	int dam;

	dam		= dice(level/2, 2);
	if ( !saves_spell( level, victim,DAMTYPE(sn)))
	{
		act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
		af.where     = WHERE_AFFECTS;
		af.type      = sn;
		af.level     = level;
		af.duration  = 6;
		af.location  = APPLY_ST;
		af.modifier  = -5;
		af.bitvector = 0;
		affect_join( victim, &af );
	}
	else
	{
		dam /= 2;
	}

	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_colour_spray( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	int dam			  = dice(level/3, 4)+40;

	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;
	else 
		spell_blindness( gsn_blindness, level/2,ch,(void *) victim,TARGET_CHAR);

	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true );
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_continual_light(int,int ,char_data *ch,void *,int )
{
	OBJ_DATA *light;

	if(!IS_NULLSTR(target_name))  // do a glow on some object 
	{
		light = get_obj_carry(ch,target_name);

		if (light == NULL)
		{
			ch->printlnf( "You don't see any '%s' here.", target_name );
			return NO_MANA;
		}

		if (IS_OBJ_STAT(light,OBJEXTRA_GLOW))
		{
			act("$p is already glowing.",ch,light,NULL,TO_CHAR);
			return NO_MANA;
		}

		SET_BIT(light->extra_flags,OBJEXTRA_GLOW);
		act("$p glows with a white light.",ch,light,NULL,TO_ALL);
		return FULL_MANA;
	}

	light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ));
	obj_to_room( light, ch->in_room );
	act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
	act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_control_weather(int ,int level,char_data *ch,void *,int ) 
{
	if ( !str_cmp( target_name, "worse" ) )
		weather_info[ch->in_room->sector_type].change += dice( level / 3, 4 );
	else if ( !str_cmp( target_name, "better" ) )
		weather_info[ch->in_room->sector_type].change -= dice( level / 3, 4 );
	else {
		ch->println( "Do you want it to get better or worse?" );
		return NO_MANA;
	}

    ch->println( "Ok." );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_create_food2( int , int level, char_data *ch, void *,int )
{
	OBJ_DATA *mushroom;

	mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ));
	mushroom->value[0] = number_fuzzy(level);
	mushroom->value[1] = number_fuzzy(level)*2;
	mushroom->timer = (ch->level*2)+10;
	obj_to_room( mushroom, ch->in_room );
	if(	   ch->in_room->sector_type==SECT_AIR
		|| IS_WATER_SECTOR( ch->in_room->sector_type )){
		act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
		act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
	}else{
		act( "$p suddenly appears to grow from the ground.", ch, mushroom, NULL, TO_ROOM );
		act( "$p suddenly appears to grow from the ground.", ch, mushroom, NULL, TO_CHAR );
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_create_food( int , int level, char_data *ch, void *,int )
{
	OBJ_DATA *fooditem;
	int		  foodvnum;
	char	  buf[MSL];

	foodvnum = race_table[ch->race]->food_vnum;

	if ( foodvnum == 0 )
		foodvnum = OBJ_VNUM_MUSHROOM;

	if ( get_obj_index( foodvnum ) == NULL )
	{
		bugf("Vnum %d not found for spell_create_food!", foodvnum );

		// do an autonote
		sprintf( buf, "Vnum `Y%d`x not found for race `Y%s`x.`1"
			"Please create item at next possible convenience.`1",
			foodvnum, race_table[ch->race]->name);
		autonote(NOTE_SNOTE, "spell_create_food()",	"Create food", "realm", buf, true);

		// default it to a known item so we don't go BOOM
		foodvnum = OBJ_VNUM_MUSHROOM;
	}

	fooditem = create_object( get_obj_index( foodvnum ));
	fooditem->value[0] = number_fuzzy(level);
	fooditem->value[1] = number_fuzzy(level)*2;
	fooditem->timer = (ch->level*2)+10;
	obj_to_room( fooditem, ch->in_room );

	act( "$p suddenly appears.", ch, fooditem, NULL, TO_ROOM );
	act( "$p suddenly appears.", ch, fooditem, NULL, TO_CHAR );

	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_create_rose( int , int , char_data *ch, void *,int  )
{
	OBJ_DATA *rose;
	if (get_obj_index(OBJ_VNUM_ROSE)) {
		rose = create_object(get_obj_index(OBJ_VNUM_ROSE));
		act("$n has created a beautiful red rose.",ch,rose,NULL,TO_ROOM);
		ch->println( "You create a beautiful red rose." );
		obj_to_char(rose,ch);
	}else{
        ch->println( "BUG: No available rose object in spell_create_rose - please report!" );
		return NO_MANA;
	}

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_create_spring(int ,int level,char_data *ch,void *,int )
{
    OBJ_DATA *spring;

	if ( ch->in_room->sector_type == SECT_AIR )
	{
		ch->println( "You cannot create a spring in the middle of the air." );
		return HALF_MANA;
	}

	if ( IS_WATER_SECTOR( ch->in_room->sector_type ))
	{
		ch->println( "You cannot create a spring in water." );
		return HALF_MANA;
	}

	spring = create_object( get_obj_index( OBJ_VNUM_SPRING ));
	spring->timer = number_fuzzy(level);
	obj_to_room( spring, ch->in_room );
	act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
	act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_create_water( int , int level, char_data *ch, void *vo,int )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int water;

	if ( obj->item_type != ITEM_DRINK_CON )
	{
		ch->println( "It is unable to hold water." );
		return NO_MANA;
	}

	if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
	{
		ch->println( "It contains some other liquid." );
		return HALF_MANA;
	}

	water = UMIN(
		level * (weather_info[ch->in_room->sector_type].sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1] );

	if ( water > 0 )
	{
		obj->value[2] = LIQ_WATER;
		obj->value[1] += water;
		if ( !is_name( "water", obj->name ) )
		{
			char buf[MSL];

			sprintf( buf, "%s water", obj->name );
			free_string( obj->name );
			obj->name = str_dup( buf );
		}
		act( "$p is filled.", ch, obj, NULL, TO_CHAR );
	}
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_cure_blindness(int ,int level,char_data *ch,void *vo,int )
{
    char_data *victim = (char_data *) vo;

	if ( !is_affected( victim, gsn_blindness ) )
	{
		if (victim == ch){
			ch->println( "You aren't blind." );
		}else{
			act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}
 
	if (check_dispel(level,victim,gsn_blindness))
	{
		victim->println( "Your vision returns!" );
		act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
	}else{
		ch->println("Spell failed.");
	}

	return FULL_MANA;
}



/**************************************************************************/
SPRESULT spell_cure_critical( int , int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int heal		  = dice(level/4+1, 2);

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );
	victim->println( "You feel better!" );
	if ( ch != victim ){
		ch->println( "Ok." );
	}
	return FULL_MANA;
}

/**************************************************************************/
// RT added to cure plague
SPRESULT spell_cure_disease( int , int level, char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;

	if ( !is_affected( victim, gsn_plague ) ){
		if (victim == ch){
			ch->println( "You aren't ill." );
		}else{
			act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}
    
	if (check_dispel(level,victim,gsn_plague))
	{
		victim->println( "Your sores vanish." );
		act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
	}
	else
		ch->println("Spell failed.");

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_cure_light( int , int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int heal		  = dice(level/12+1, 2);

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );
	victim->println( "You feel better!" );
	if ( ch != victim ){
		ch->println( "Ok." );
	}
    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_cure_poison( int , int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
 
	if ( !is_affected( victim, gsn_poison ) )
	{
		if (victim == ch){
			ch->println( "You aren't poisoned." );
		}else{
			act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}
 
	if (check_dispel(level,victim,gsn_poison))
	{
		victim->println( "A warm feeling runs through your body." );
		act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
	}
	else
		ch->println("Spell failed.");
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_cure_serious( int , int level, char_data *ch, void *vo,int  )
{
    char_data *victim = (char_data *) vo;
    int heal		  = dice(level/8+1, 2);

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );
	victim->println( "You feel better!" );

	if ( ch != victim ){
		ch->println( "Ok." );
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_curse( int sn, int level, char_data *ch, void *vo,int target )
{
	char_data *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

    // deal with the object curses first
	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT(obj,OBJEXTRA_EVIL))
		{
			act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
			return HALF_MANA;
		}

		if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
		{
			AFFECT_DATA *paf;

			paf = affect_find(obj->affected, gsn_bless );
			if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
			{
				if (paf != NULL)
					affect_remove_obj(obj,paf);
				act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);
				REMOVE_BIT(obj->extra_flags,OBJEXTRA_BLESS);
				return FULL_MANA;
			}
			else
			{
				act("The holy aura of $p is too powerful for you to overcome.",ch,obj,NULL,TO_CHAR);
				return FULL_MANA;
			}
		}

		af.where        = WHERE_OBJEXTRA;
		af.type         = sn;
		af.level        = level;
		af.duration     = 2 * level;
		af.location     = APPLY_SAVES;
		af.modifier     = +1;
		af.bitvector    = OBJEXTRA_EVIL;
		affect_to_obj(obj,&af);

		act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);
		return FULL_MANA;
	}

	// character curses 
	victim = (char_data *) vo;

	if( is_affected( victim, gsn_bless )){
		if( check_dispel( level, victim, gsn_bless )){
			act( "Your magic unravels a spell on $N.", ch, NULL, victim, TO_CHAR );
			return FULL_MANA;
		}else{
			ch->println( "You failed." );
			return FULL_MANA;
		}
	}

	if(IS_AFFECTED(victim,AFF_CURSE))
	{
		act("$N already looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}
		
	if(saves_spell(level,victim,DAMTYPE(sn))){
		ch->println( "Your spell doesnt appear to have much affect." );
		return FULL_MANA;
	}
			
	if(HAS_CLASSFLAG(victim, CLASSFLAG_CURSE_IMMUNITY)) // fails if paladin
	{
		ch->printlnf( "Your spell doesnt appear to have any affect on %s.", PERS(victim, ch) );
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level/2 + (number_fuzzy(3)*2);
	af.location  = APPLY_HITROLL;
	af.modifier  = -1 * (level / 8);
	af.bitvector = AFF_CURSE;
	affect_to_char( victim, &af );

	af.location  = APPLY_SAVES;
	af.modifier  = level / 8;
	affect_to_char( victim, &af );

	victim->println( "You feel unclean." );
	if ( ch != victim )
	{
		act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
		// stop those who are slept from logging out
		if (!IS_NPC(ch))
			victim->pknoquit=UMAX(victim->pknoquit,10);
	}

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_detect_evil( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
	{
		if (victim == ch){
			ch->println( "You can already sense evil." );
		}else{
			act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_EVIL;
	affect_to_char( victim, &af );
	victim->println( "Your eyes tingle." );
	if ( ch != victim ){
		ch->println( "Ok." );
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_detect_good( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
 
	if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
	{
		if (victim == ch){
			ch->println( "You can already sense good." );
		}else{
			act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_GOOD;
	affect_to_char( victim, &af );
	victim->println( "Your eyes tingle." );

	if ( ch != victim )
		ch->println( "Ok." );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_detect_hidden(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
	{
		if (victim == ch){
			ch->println( "You are already as alert as you can be." );
		}else{
			act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char( victim, &af );
	victim->println( "Your awareness improves." );
	if ( ch != victim ){
		ch->println( "Ok." );
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_detect_invis( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
	{
		if (victim == ch){
			ch->println( "You can already see invisible." );
		}else{
			act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVIS;
	affect_to_char( victim, &af );
	victim->println( "Your eyes tingle." );
	if ( ch != victim ){
		ch->println( "Ok." );
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_detect_magic( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
	{
		if (victim == ch){
			ch->println( "You can already sense magical auras." );
		}else{
			act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
		}
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_MAGIC;
	affect_to_char( victim, &af );
	victim->println( "Your eyes tingle." );
	if ( ch != victim ){
		ch->println( "Ok." );
	}
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_detect_poison( int , int , char_data *ch, void *vo,int )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
	{
		if ( obj->value[3] != 0 ){
			ch->println( "You smell poisonous fumes." );
		}else{
			ch->println( "It looks delicious." );
		}
	}else{
		ch->println( "It doesn't look poisoned." );
	}

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_dispel_evil( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;
  
	if ( !IS_NPC(ch) && IS_EVIL(ch) )
		victim = ch;
  
	if ( IS_GOOD(victim) )
	{
		act( "$N appears to be protected.", ch, NULL, victim, TO_ROOM );
		return FULL_MANA;
	}

	if ( IS_NEUTRAL(victim) )
	{
		act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
		return FULL_MANA;
	}

	if (victim->hit > (ch->level * 4))
		dam = dice( level, 4 );
	else
		dam = UMAX(victim->hit, dice(level,4));
    
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;

	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	return FULL_MANA;
}

/***************************************************************************/
SPRESULT spell_dispel_good( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	int dam;
 
	if ( !IS_NPC(ch) && IS_GOOD(ch) )
		victim = ch;
 
	if ( IS_EVIL(victim) )
	{
		act( "$N is protected by $S evil.", ch, NULL, victim, TO_ROOM );
		return FULL_MANA;
	}
 
	if ( IS_NEUTRAL(victim) )
	{
		act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
		return FULL_MANA;
	}
 
	if (victim->hit > (ch->level * 4))
		dam = dice( level, 4 );
	else
		dam = UMAX(victim->hit, dice(level,4));
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn) ,true);
	return FULL_MANA;
}

/***************************************************************************/
// modified for enhanced use
SPRESULT spell_dispel_magic( int sn , int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	bool found = false;

	if (saves_spell(level, victim,DAMTYPE(sn)))
	{
		victim->println( "You feel a brief tingling sensation." );
		ch->println( "You failed." );
		return FULL_MANA;
	}

	// begin running through the spells 
	found = check_dispel(level,victim,gsn_regeneration);
	found = check_dispel(level,victim,gsn_resist_poison);
	found = check_dispel(level,victim,gsn_poison_immunity);
	found = check_dispel(level,victim,gsn_prismatic_spray);
	found = check_dispel(level,victim,gsn_illusions_grandeur);
	found = check_dispel(level,victim,gsn_protection_fire);
	found = check_dispel(level,victim,gsn_protection_cold);
	found = check_dispel(level,victim,gsn_protection_acid);
	found = check_dispel(level,victim,gsn_protection_lightning);
	found = check_dispel(level,victim,gsn_holy_aura);
	found = check_dispel(level,victim,gsn_unholy_aura);
	found = check_dispel(level,victim,gsn_true_sight);
	found = check_dispel(level,victim,gsn_barkskin);
	found = check_dispel(level,victim,gsn_magic_resistance);
	found = check_dispel(level,victim,gsn_fire_shield);
	found = check_dispel(level,victim,gsn_chill_shield);
	found = check_dispel(level,victim,gsn_animal_essence);
	found = check_dispel(level,victim,gsn_armor);
	found = check_dispel(level,victim,gsn_bless);

	if (check_dispel(level,victim,gsn_blindness ))
	{
		found = true;
		act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
	}
	if (check_dispel(level,victim,gsn_calm))
	{
		found = true;
		act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
	}
	if (check_dispel(level,victim, gsn_change_sex ))
	{
		found = true;
		act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
	}
	if (check_dispel(level,victim,gsn_charm_person))
	{
		found = true;
		act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
	}
	if (check_dispel(level,victim,gsn_chill_touch))
	{
		found = true;
		act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
	}
	if (check_dispel(level,victim, gsn_curse ))
		found = true;
	if (check_dispel(level,victim,gsn_detect_evil))
		found = true;
	if (check_dispel(level,victim,gsn_detect_scry ))
		found = true;
	if (check_dispel(level,victim,gsn_detect_good))
		found = true;
	if (check_dispel(level,victim,gsn_detect_hidden))
		found = true;
    if (check_dispel(level,victim,gsn_detect_invis))
        found = true;
	if (check_dispel(level,victim,gsn_detect_magic))
		found = true;
	if (check_dispel(level,victim, gsn_faerie_fire ))
	{
		act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_fly ))
	{
		act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_frenzy ))
	{
		act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
		found = true;
	}
	if (check_dispel(level,victim, gsn_giant_strength ))
	{
		act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_haste ))
	{
		act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim,gsn_infravision))
		found = true;
	if (check_dispel(level,victim, gsn_invisibility ))
	{
		act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim,gsn_mass_invis))
	{
		act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim,gsn_pass_door))
		found = true;
	if (check_dispel(level,victim,gsn_protection_evil))
		found = true;
	if (check_dispel(level,victim,gsn_protection_good))
		found = true;
	if (check_dispel(level,victim,gsn_rage))
		found = true;
	if (check_dispel(level,victim, gsn_sanctuary ))
	{
		act("The white aura around $n's body vanishes.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
    if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim, gsn_sanctuary ))
	{
		REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
		act("The white aura around $n's body vanishes.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim,gsn_shield))
	{
		act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim,gsn_sleep))
		found = true;
	if (check_dispel(level,victim, gsn_slow ))
	{
		act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim,gsn_stone_skin))
	{
		act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim, gsn_weaken ))
	{
		act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}
	if (check_dispel(level,victim,gsn_wind_shield))
	{
		act("The wind around $n's body dies down.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

    if (found)
        ch->println( "Ok." );
    else {
        ch->println("Spell failed.");
		return HALF_MANA;
	}
	return FULL_MANA;
}

/***************************************************************************/
SPRESULT spell_earthquake( int sn, int level, char_data *ch, void *,int  )
{
	char_data *vch;
	char_data *vch_next;

	ch->println( "The earth trembles beneath your feet!" );
	act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
        vch_next=vch->next_in_room;
		if ( vch->in_room == NULL )
			continue;
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && !is_safe_spell(ch,vch,true)){
				if (IS_AFFECTED(vch,AFF_FLYING)){
					damage_spell(ch,vch,0,sn,DAMTYPE(sn),true);
				}else{
					damage_spell( ch,vch,level + dice(2, 8), sn, DAMTYPE(sn),true);
				}
			}
			continue;
		}

		if ( vch->in_room->area == ch->in_room->area ){
			vch->println( "The earth trembles and shivers." );
		}
	}
    return FULL_MANA;
}

/***************************************************************************/
SPRESULT spell_enchant_armor( int sn, int level, char_data *ch, void *vo,int )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf; 
	int result, fail;
	int added;
	bool ac_found = false;

    /* checks if enchant can be done */
	if (obj->item_type != ITEM_ARMOR)
	{
		ch->printf( "That isn't an armor.\r\n" );
		return NO_MANA;
	}
	if (obj->wear_loc != -1)
	{
		ch->printf( "The item must be carried to be enchanted.\r\n" );
		return NO_MANA;
	}

	fail = 25;   // base 25% chance of failure 

	// find the current bonuses - affect success 
	for (paf=OBJECT_AFFECTS(obj); paf; paf=paf->next )
	{
		if ( paf->location == APPLY_AC )
		{
			if (paf->duration == -1) // can't add to a perm armour enchant
			{
				ch->printf( "%s starts to pulsate then it dissolves before your eyes!!!\r\n",
					format_obj_to_char(obj, ch, true));
				extract_obj(obj);
				return FULL_MANA;
			}
			ac_found = true;
			fail += 5 * paf->modifier * paf->modifier;
		}
		else
		{
			fail += 20;
		}
	}

	// apply other modifiers 
	fail -= level;
	if (IS_OBJ_STAT(obj,OBJEXTRA_BLESS))
		fail -= 15;
	if (IS_OBJ_STAT(obj,OBJEXTRA_GLOW))
		fail -= 5;

	fail = URANGE(5,fail,85);
	result = number_percent();

	// the moment of truth 
	if (result < (fail / 5))  // item destroyed 
	{
		act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
		act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
		extract_obj(obj);
		return FULL_MANA;
	}

	if (result < (fail / 3)) // item disenchanted
	{
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

	if (result <= (90 - level/5))  // success! 
	{
		act("$p shimmers with a golden aura.",ch,obj,NULL,TO_CHAR);
		act("$p shimmers with a golden aura.",ch,obj,NULL,TO_ROOM);
		SET_BIT(obj->extra_flags, OBJEXTRA_MAGIC);
		added = -1;
	}else{  // exceptional enchant
		act("$p glows a brillant gold!",ch,obj,NULL,TO_CHAR);
		act("$p glows a brillant gold!",ch,obj,NULL,TO_ROOM);
		SET_BIT(obj->extra_flags,OBJEXTRA_MAGIC);
		SET_BIT(obj->extra_flags,OBJEXTRA_GLOW);
		added = -2;
	}
		
	// now add the new armour enchantment 
	if (ac_found){ // if there is already an enchant on it 
		for ( paf = obj->affected; paf != NULL; paf = paf->next)
		{
			if ( paf->location == APPLY_AC)
			{
				paf->type = sn;
				paf->modifier += added;
				if (ch->level>paf->level)
					paf->level = ch->level;
			}
		}
	}else{ // add a new armour affect at the head of the affected list
		paf				= new_affect();
		paf->where		= WHERE_OBJEXTRA;
        paf->type		= sn;
        paf->level		= ch->level;
        paf->duration	= level*20;
        paf->location	= APPLY_AC;
        paf->modifier	= added;
        paf->bitvector	= 0;
        paf->next		= obj->affected;
        obj->affected	= paf;
	}
	return FULL_MANA;
}

/***************************************************************************/
SPRESULT spell_enchant_weapon(int sn,int level, char_data *ch, void *vo,int )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf; 
	int result, fail;
	int added;
	bool hit_found = false, dam_found = false;

	// do standard checks - if spell can be cast 
	if (obj->item_type != ITEM_WEAPON)
	{
		ch->printf( "That isn't a weapon.\r\n" );
		return NO_MANA;
    }
    if (obj->wear_loc != -1)
    {
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
// Drain XP, MANA, HP from victim -  Caster gains HP.
SPRESULT spell_energy_drain( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	int dam;
	int temp_hps;

	if ( saves_spell( level, victim,DAMTYPE(sn)))
	{
		victim->printf( "You feel a momentary chill.\r\n" );
		ch->printf( "Your spell doesnt appear to have much affect.\r\n" );
		return FULL_MANA;
	}

	if ( victim->level <= 2 )
		dam		 = ch->hit + 1;
    else
		dam		 = dice(1, level/4);

	if (damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true))
	{	
		gain_exp( victim, 0 - number_range( level/2, level) );
		victim->mana	/= 2;
		victim->move	/= 2;

		victim->printf( "You feel your life slipping away!\r\n" );
		temp_hps=victim->hit;

		if(victim->hit<-10) 
			victim->hit=-10;

		// give caster some HP back
		if ((temp_hps-victim->hit)>0)
		{
			ch->hit+= (temp_hps-victim->hit);
			ch->printf( "Wow....what a rush!\r\n" );
		}
		else
		{
			ch->printf( "They look drained.\r\n" );
		}
	}

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_fireball( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	int dam;

	dam		= dice(level, 6)+10;
	if ( saves_spell( level, victim, DAMTYPE(sn)))
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_fireproof(int sn, int level, char_data *ch, void *vo,int )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;

	if (IS_OBJ_STAT(obj,OBJEXTRA_BURN_PROOF))
	{
		act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_OBJEXTRA;
	af.type      = sn;
	af.level     = level;
	af.duration  = number_fuzzy(level / 4);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = OBJEXTRA_BURN_PROOF;

	affect_to_obj(obj,&af);

	act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
	act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_flamestrike( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;

	dam = dice(6 + level / 2, 8);
	if ( saves_spell( level, victim,DAMTYPE(sn)))
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_frostball ( int sn, int level, char_data *ch, void *vo, int )
{
	char_data	*victim = (char_data *) vo;
	int			dam;

	dam = dice( level, 9 );
	if ( saves_spell( level, victim, DAMTYPE(sn)))
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn), true );
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_faerie_fire( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) {
		ch->printf( "They are already aglow.\r\n" );
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = level/3 + number_fuzzy(3)*3;
	af.location  = APPLY_AC;
	af.modifier  = 2 * level;
	af.bitvector = AFF_FAERIE_FIRE;
	affect_to_char( victim, &af );
	victim->printf( "You are surrounded by a pink outline.\r\n" );
	act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_faerie_fog( int sn, int level, char_data *ch, void *,int )
{
	char_data *ich;
	long tempflag;
	bool changed= false;

	act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
	ch->printf( "You conjure a cloud of purple smoke.\r\n" );

	for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
	{
		if (INVIS_LEVEL(ich)> 0)
			continue;

		if ( ich == ch || saves_spell( level, ich,DAMTYPE(sn)) )
			continue;

		tempflag=ich->affected_by;

		if (is_affected( ich, gsn_invisibility ))
		{
			affect_strip( ich, gsn_invisibility );
			changed = true;	
		}
		if (is_affected( ich, gsn_mass_invis))
		{
			affect_strip( ich, gsn_mass_invis);
			changed = true;
		}
		if (is_affected( ich, gsn_sneak))
		{
			affect_strip( ich, gsn_sneak);
			changed = true;
		}
		
		if (IS_AFFECTED2( ich, AFF2_VANISH))
		{
			affect_parentspellfunc_strip( ich, gsn_vanish);
			changed = true;
		}

		REMOVE_BIT( ich->affected_by,	AFF_HIDE	);
		REMOVE_BIT( ich->affected_by,	AFF_INVISIBLE	);
		REMOVE_BIT( ich->affected_by,	AFF_SNEAK	);
		REMOVE_BIT( ich->affected_by2,	AFF2_VANISH );

		if (ich->affected_by!=tempflag || changed)
		{
			act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
			ich->printf( "You are revealed!\r\n" );
		}
	}

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_floating_disc( int , int level,char_data *ch,void *,int )
{
	OBJ_DATA *disc, *floating;

	floating = get_eq_char(ch,WEAR_FLOAT);

	if (floating != NULL && IS_OBJ_STAT(floating,OBJEXTRA_NOREMOVE))
	{
		act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
		return NO_MANA;
	}

	disc = create_object(get_obj_index(OBJ_VNUM_DISC));
	disc->value[0]	= ch->level * 10; // 10 lbs per level capacity 
	disc->value[3]	= ch->level * 5; // 5 lbs per level max per item 
	disc->timer		= ch->level * 2 - number_range(0,level / 2); 

	act("$n has created a floating black disc.",ch,NULL,NULL,TO_ROOM);
	ch->printf( "You create a floating disc.\r\n" );
	obj_to_char(disc,ch);
	wear_obj(ch,disc,true, false);

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_fly( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_SET( ch->in_room->room_flags, ROOM_NOFLY )) {
		ch->println( "The powerful winds prevent you from taking to air." );
		return NO_MANA;
	}
	
	if ( IS_AFFECTED(victim, AFF_FLYING) )
	{
		if (victim == ch)
			ch->printf( "You are already airborne.\r\n" );
		else
			act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = level + 3;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_FLYING;
	affect_to_char( victim, &af );
	victim->println( "Your feet rise off the ground." );
	act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );

	return FULL_MANA;
}

/**************************************************************************/
// RT clerical berserking spell 
SPRESULT spell_frenzy(int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
	{
		if (victim == ch)
			ch->printf( "You are already in a frenzy.\r\n" );
		else
			act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	if (is_affected(victim,gsn_calm))
	{
		if (victim == ch)
			ch->printf( "Why don't you just relax for a while?\r\n" );
		else
			act("$N doesn't look like $e wants to fight anymore.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
		(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
		(IS_EVIL(ch) && !IS_EVIL(victim)))
	{
		act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type 	 = sn;
	af.level	 = level;
	af.duration	 = level / 3;
	af.modifier  = level / 6;
	af.bitvector = 0;

	af.location  = APPLY_HITROLL;
	affect_to_char(victim,&af);

	af.location  = APPLY_DAMROLL;
	affect_to_char(victim,&af);

	af.modifier  = 10 * (level / 12);
	af.location  = APPLY_AC;
	affect_to_char(victim,&af);

	victim->printf( "You are filled with holy wrath!\r\n" );
	act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_gate( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	bool gate_pet;

	if (( victim = get_char_icworld( ch, target_name ) ) == NULL
	 ||   victim == ch
	 ||   victim->in_room == NULL
	 ||   !can_see_room(ch,victim->in_room)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	 ||   IS_SET(victim->in_room->room_flags, ROOM_NOSCRY)
	 ||	  IS_SET(victim->in_room->room_flags, ROOM_ANTIMAGIC)
	 ||   IS_SET(victim->in_room->area->area_flags, AREA_NOSCRY)
	 ||   IS_SET(victim->in_room->area->area_flags, AREA_NOGATEINTO)	 
	 ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	 ||   victim->level >= level + 3
	 ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)  // NOT trust
	 ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
	 ||   (IS_NPC(victim) && saves_spell( level, victim,DAMTYPE(sn)))
	 ||   (!IS_NPC(victim) && saves_spell( level, victim, DAMTYPE(sn))))
	 {
		  ch->println( "You failed." );
		  return FULL_MANA;
	 }

	if ( ch->in_room->area->continent != victim->in_room->area->continent )
	{
		ch->println( "The spell cannot span such a great distance." );
		return FULL_MANA;
	}

	if(ch->pet && ch->in_room == ch->pet->in_room){
		gate_pet = true;
	}else{
		gate_pet = false;
	}

	act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
	ch->printf( "You step through a gate and vanish.\r\n" );
	char_from_room(ch);
	char_to_room(ch,victim->in_room);

	act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
	do_look(ch,"auto");
	 /* 
     * If someone is following the char, these triggers get activated
     * for the followers before the char, but it's safer this way...
     */
	if ( IS_NPC( ch ) && HAS_TRIGGER( ch, MTRIG_ENTRY ) ){
		mp_percent_trigger( ch, NULL, NULL, NULL, MTRIG_ENTRY );
	}

	if ( !IS_NPC( ch ) ){
		mp_greet_trigger( ch );
	}

	if (gate_pet)
	{
		act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
		ch->pet->printf( "You step through a gate and vanish.\r\n" );
		char_from_room(ch->pet);
		char_to_room(ch->pet,victim->in_room);
		act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
		do_look(ch->pet,"auto");
	 
		 /* 
		 * If someone is following the char, these triggers get activated
		 * for the followers before the char, but it's safer this way...
		 */
		if (ch->pet)
		{
			if ( IS_NPC( ch->pet) && HAS_TRIGGER( ch->pet, MTRIG_ENTRY ) ){
				mp_percent_trigger( ch->pet, NULL, NULL, NULL, MTRIG_ENTRY );
			}

			if ( !IS_NPC( ch->pet) ){
				mp_greet_trigger( ch->pet);
			}
		}
	}

    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_giant_strength(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	bool half = false;

	if(check_strength(level, ch, victim, half))
		return FULL_MANA;
	if(half)
		return HALF_MANA;

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level;
	af.duration		= level;
	af.location		= APPLY_ST;
	af.modifier		= 1 + level/5;
	af.bitvector	= 0;
	affect_to_char( victim, &af );
	victim->printf( "Your muscles surge with heightened power!\r\n" );
	act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_harm( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;

	dam = UMAX(  20, victim->hit - dice(1,4) );
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam = UMIN( 50, dam / 2 );

	dam = UMIN( 100, dam );

	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	return FULL_MANA;
}

/**************************************************************************/
// RT haste spell
SPRESULT spell_haste( int sn, int level, char_data *ch, void *vo,int)
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
	
	if ( is_affected( victim, sn )
		|| IS_AFFECTED(victim,AFF_HASTE)
		|| IS_SET(victim->off_flags,OFF_FAST))
	{
		if (victim == ch)
			ch->printf( "You can't move any faster!\r\n" );
		else
			act("$N is already moving as fast as $E can.",ch,NULL,victim,TO_CHAR);
        return HALF_MANA;
	}
	
	if (IS_AFFECTED(victim,AFF_SLOW))
	{
		if (!check_dispel(level,victim, gsn_slow ))
		{
			if (victim != ch)
				ch->println("Spell failed.");
			victim->printf( "You feel momentarily faster.\r\n" );
			return FULL_MANA;
		}
		act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;

	if (victim == ch){
		af.duration  = level/2;
	}else{
		af.duration  = level/4;
	}

	af.location  = APPLY_QU;
	af.modifier  = 1 + level/5;
	af.bitvector = AFF_HASTE;
	affect_to_char( victim, &af );
	victim->printf( "You feel yourself moving more quickly.\r\n" );
	act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);

	if ( ch != victim )
		ch->println( "Ok." );

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_heal( int , int , char_data *ch, void *vo,int)
{
	char_data *victim = (char_data *) vo;

	victim->hit = UMIN( victim->hit + game_settings->max_hp_from_heal_spell, victim->max_hit );
	update_pos( victim );
	victim->printf( "A warm feeling fills your body.\r\n" );

	if ( ch != victim )
		ch->println( "Ok." );
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_heat_metal( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	OBJ_DATA *obj_lose, *obj_next;
	int dam = 0;
	bool fail = true;
 
	if (!saves_spell(level + 2,victim,DAMTYPE(sn)) 
	&&  !IS_SET(victim->imm_flags,IMM_FIRE))
	{
		for ( obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next)
		{
			obj_next = obj_lose->next_content;

			if ( number_range(1,2 * level) > obj_lose->level
			&&   !saves_spell(level,victim,DAMTYPE(sn))
			&&   !IS_OBJ_STAT(obj_lose,OBJEXTRA_NONMETAL)
			&&   !IS_OBJ_STAT(obj_lose,OBJEXTRA_BURN_PROOF))
			{
				switch ( obj_lose->item_type )
				{
					case ITEM_ARMOR:
						if (obj_lose->wear_loc != -1) /* remove the item */
						{
							if (can_drop_obj(victim,obj_lose)
							&& (obj_lose->weight / 10) < number_range(1,ch->modifiers[STAT_QU])
							&&  remove_obj( victim, obj_lose->wear_loc, true ))
							{
								act("$n yelps and throws $p to the ground!",victim,obj_lose,NULL,TO_ROOM);
								act("You remove and drop $p before it burns you.",victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 3);
								obj_from_char(obj_lose);
								obj_to_room(obj_lose, victim->in_room);
								fail = false;
							}
							else /* stuck on the body! ouch! */
							{
								act("Your skin is seared by $p!",victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level));
								fail = false;
							}
						}
						else /* drop it if we can */
						{
							if (can_drop_obj(victim,obj_lose))
							{
								act("$n yelps and throws $p to the ground!",victim,obj_lose,NULL,TO_ROOM);
								act("You and drop $p before it burns you.",victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 6);
								obj_from_char(obj_lose);
								obj_to_room(obj_lose, victim->in_room);
								fail = false;
							}
							else /* cannot drop */
							{
								act("Your skin is seared by $p!",victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 2);
								fail = false;
							}
						}
						break;
					case ITEM_WEAPON:
						if (obj_lose->wear_loc != -1) /* try to drop it */
						{
							if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
								continue;

							if (can_drop_obj(victim,obj_lose) 
							&&  remove_obj(victim,obj_lose->wear_loc,true))
							{
								act("$n is burned by $p, and throws it to the ground.",victim,obj_lose,NULL,TO_ROOM);
								victim->printf( "You throw your red-hot weapon to the ground!\r\n" );
								dam += 1;
								obj_from_char(obj_lose);
								obj_to_room(obj_lose,victim->in_room);
								fail = false;
							}
							else /* YOWCH! */
							{
								victim->printf( "Your weapon sears your flesh!\r\n" );
								dam += number_range(1,obj_lose->level);
								fail = false;
							}
						}
						else /* drop it if we can */
						{
							if (can_drop_obj(victim,obj_lose))
							{
								act("$n throws a burning hot $p to the ground!",victim,obj_lose,NULL,TO_ROOM);
								act("You and drop $p before it burns you.",victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 6);
								obj_from_char(obj_lose);
								obj_to_room(obj_lose, victim->in_room);
								fail = false;
							}
							else /* cannot drop */
							{
								act("Your skin is seared by $p!",victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 2);
								fail = false;
							}
						}
						break;
					}
				}
			}
		} 

		if (fail)
	    {
			ch->printf( "Your spell had no effect.\r\n" );
			victim->printf( "You feel momentarily warmer.\r\n" );
		}
		else /* damage! */
		{
		if (saves_spell(level,victim,DAMTYPE(sn)))
			dam = 2 * dam / 3;
		damage_spell(ch,victim,dam,sn,DAMTYPE(sn),true);
    }
	return FULL_MANA;
}

/**************************************************************************/
// RT really nasty high-level attack spell
SPRESULT spell_holy_word(int sn, int level, char_data *ch, void *,int )
{
	char_data *vch;
	char_data *vch_next;
	int dam;
	int bless_num, curse_num, frenzy_num;
   
	bless_num = gsn_bless;
	curse_num = gsn_curse; 
	frenzy_num = gsn_frenzy;

	act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
	ch->printf( "You utter a word of divine power.\r\n" );

	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
	{
		vch_next = vch->next_in_room;

		if ((IS_GOOD(ch) && IS_GOOD(vch))
		||  (IS_EVIL(ch) && IS_EVIL(vch))
		||  (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)))
		{
			vch->printf( "You feel full more powerful.\r\n" );
			spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR);
			spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
		}

		else if ((IS_GOOD(ch) && IS_EVIL(vch))
			 || (IS_EVIL(ch) && IS_GOOD(vch)))
		{
			if (!is_safe_spell(ch,vch,true))
			{
				spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
				vch->printf( "You are struck down!\r\n" );
				dam = dice(level,12);
				damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
			}
		}
		else if (IS_NEUTRAL(vch))
		{
			if (!is_safe_spell(ch,vch,true))
			{
				spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR);
				vch->printf( "You are struck down!\r\n" );
				dam = dice(level,10);
				damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
			}
		}
	}

	ch->printf( "You feel drained.\r\n" );
	ch->move = 0;
	ch->hit /= 2;

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_identify( int, int, char_data *ch, void *vo, int  )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf=NULL;
	
	// Herb check instant crap out
	if ( obj->item_type == ITEM_HERB ) {
		ch->printf( "Your spell fizzles.\r\n" );
		return NO_MANA;
	}
	
	ch->printf( "Object '%s' is type %s,\r\n"
		"extra flags %s.\r\n"
		"extra2 flags %s.\r\n"		
		"Weight is %0.1f lbs, value is %d, level is %d.\r\n",
		obj->name,
		item_type_name( obj ),
		extra_bit_name( obj->extra_flags ),
		extra2_bit_name( obj->extra2_flags ),
		((double)obj->weight) / 10,
		obj->cost,
		obj->level );
	
	switch ( obj->item_type )
	{
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		ch->printf( "Level %d spells of:", obj->value[0] );
		
		if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
		{
			ch->printf( " '%s'", skill_table[obj->value[1]].name );
		}
		
		if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
		{
			ch->printf( " '%s'", skill_table[obj->value[2]].name );
		}
		
		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			ch->printf( " '%s'", skill_table[obj->value[3]].name );
		}
		
		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
		{
			ch->printf( " '%s'", skill_table[obj->value[4]].name );
		}
		ch->printf( ".\r\n" );
		break;
		
	case ITEM_WAND: 
	case ITEM_STAFF:
		ch->printf( "Has %d charges of level %d", obj->value[2], obj->value[0] );
		
		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			ch->printf( " '%s'", skill_table[obj->value[3]].name );
		}
		ch->printf( ".\r\n" );
		break;
		
	case ITEM_DRINK_CON:
		ch->printf( "It holds %s-colored %s.\r\n",
			liq_table[obj->value[2]].liq_color,
			liq_table[obj->value[2]].liq_name);
		break;
		
	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
		ch->printlnf( "Maximum combined weight: %d lbs, Capacity for an individual item: %d lbs.",
			obj->value[0], obj->value[3]);
		ch->printlnf( "Flags: %s.", cont_bit_name(obj->value[1]));
		if (obj->value[4] != 100){
			ch->printf( "Weight multiplier: %d%%\r\n", obj->value[4]);
		}
		break;
		
	case ITEM_WEAPON:
		{
			ch->printf( "Weapon type is %s.\r\n", get_weapontype( obj ));
			ch->printf("Damage is %dd%d (average %d).\r\n",
				obj->value[1],obj->value[2],
				(1 + obj->value[2]) * obj->value[1] / 2);
			if (obj->value[4])  // weapon flags 
			{
				ch->printf("Weapons flags: %s\r\n",weapon_bit_name(obj->value[4]));
			}
		}
		break;
		
	case ITEM_COMPONENT:
		if ( obj->value[0] < 0 )
		{
			ch->printf( "Has unlimited charges for the spell" );
		}
		else
		{
			ch->printf( "Can be used %d more times for the spell", obj->value[0] );
		}
		if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
		{
			ch->printf( " '%s'", skill_table[obj->value[1]].name );
		}
		ch->printf( ".\r\n" );
		break;
		
	case ITEM_ARMOR:
		ch->printf( "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\r\n",
			obj->value[0],	obj->value[1], obj->value[2], obj->value[3] );
		break;
	}

	for ( paf = OBJECT_AFFECTS(obj); paf; paf = paf->next )
	{
        if ( paf->location != APPLY_NONE && paf->modifier != 0 )
        {       
			if(paf->duration > -1){
				ch->printf( "Affects %s by %d for %d hour%s.%s\r\n",
					affect_loc_name( paf->location ), paf->modifier,
					paf->duration, paf->duration==1?"":"s",
					(paf->level>ch->level?"  (above your level)":""));
			}else{
				ch->printf( "Affects %s by %d.%s\r\n",
					affect_loc_name( paf->location ), paf->modifier,
					(paf->level>ch->level?"  (above your level)":""));
			}		
		}else{
			ch->println(to_affect_string( paf, obj->level, false ));
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_infravision( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_INFRARED) )
	{
		if (victim == ch)
			ch->printf( "You can already see in the dark.\r\n" );
		else
			act("$N already has infravision.\r\n",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	act( "$n's eyes glow red.\r\n", ch, NULL, NULL, TO_ROOM );

	af.where	 = WHERE_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 2 * level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INFRARED;
	affect_to_char( victim, &af );
	victim->printf( "Your eyes glow red.\r\n" );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_invisibility( int sn, int level, char_data *ch, void *vo,int target )
{
	char_data *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	/* object invisibility */
	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj,OBJEXTRA_INVIS))
		{
			act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
			return HALF_MANA;
		}

		af.where	= WHERE_OBJEXTRA;
		af.type		= sn;
		af.level	= level;
		af.duration	= level + 12;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= OBJEXTRA_INVIS;
		affect_to_obj(obj,&af);

		act("$p fades out of sight.",ch,obj,NULL,TO_ALL);

		return FULL_MANA;
	}

	/* character invisibility */
	victim = (char_data *) vo;

	if ( IS_AFFECTED(victim, AFF_INVISIBLE )) {
		ch->printf( "They are already invisible.\r\n" );
		return HALF_MANA;
	}

	act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level + 12;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( victim, &af );
	victim->printf( "You fade out of existence.\r\n" );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_know_alignment(int ,int ,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;

	if(IS_ICIMMORTAL(victim)){
		ch->printf( "You really aren't sure about %s.\r\n", PERS(victim, ch));
		return DOUBLE_MANA;
	};
	
    switch (victim->tendency)
	{
		case -3: ch->printf( "%s is very chaotic ",			PERS(victim, ch));	break;
		case -2: ch->printf( "%s is fairly chaotic ",		PERS(victim, ch));	break;
		case -1: ch->printf( "%s has chaotic tendencies ",	PERS(victim, ch));	break;
		case  0: ch->printf( "%s is balanced ",				PERS(victim, ch));	break;
		case  1: ch->printf( "%s has lawful tendencies ",	PERS(victim, ch));	break;
		case  2: ch->printf( "%s is quite lawful ",			PERS(victim, ch));	break;
		case  3: ch->printf( "%s is extremely lawful ",		PERS(victim, ch));	break;
	}
	
	switch (victim->alliance)
	{
		case -3: ch->printf( "and is completely aligned with evil.\r\n");		break;
		case -2: ch->printf( "with many attachments to the dark forces.\r\n");	break;
		case -1: ch->printf( "and some alliance with evil.\r\n");				break;                
		case  0: ch->printf( "and indifferent.\r\n");							break;
		case  1: ch->printf( "with good tendencies.\r\n");						break;                
		case  2: ch->printf( "and has definite attachments to good.\r\n");		break;                
		case  3: ch->printf( "and a champion of the divine.\r\n");				break;               
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_lightning_bolt(int sn,int level,char_data *ch,void *vo,int)
{
	char_data *victim = (char_data *) vo;
	int dam;

	dam		= dice(level,6);
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_locate_object( int , int level, char_data *ch, void *,int )
{
	char buf[MIL];
	BUFFER *buffer;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	bool found;
	int number = 0, max_found;

	found = false;
	number = 0;
	max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

	buffer = new_buf();
 
	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
		if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) 
		||   IS_OBJ_STAT(obj,OBJEXTRA_NOLOCATE) || number_percent() > 2 * level
		||   ch->level < obj->level)
		continue;

		found = true;
		number++;

		for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

		if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
		{
			sprintf( buf, "one is carried by %s\r\n", PERS(in_obj->carried_by, ch) );
		}
		else
		{
			if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
				sprintf( buf, "one is in %s [Room %d]\r\n",in_obj->in_room->name, in_obj->in_room->vnum);
			else
				sprintf( buf, "one is in %s\r\n", in_obj->in_room == NULL
					? "somewhere" : in_obj->in_room->name );
		}

		buf[0] = UPPER(buf[0]);
		add_buf(buffer,buf);

		if (number >= max_found)
			break;
	}

	if ( !found )
		ch->printf( "Nothing like that in heaven or earth.\r\n" );
	else
		ch->sendpage(buf_string(buffer));

	free_buf(buffer);

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_magic_missile( int sn, int level, char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;

	dam		= level/4+2;
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_mass_healing(int , int level, char_data *ch, void *, int )
{
	char_data *gch;
	int heal_num, refresh_num;

	heal_num = gsn_heal;
	refresh_num = gsn_refresh;

	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
		if ((IS_NPC(ch) && IS_NPC(gch))
		|| (!IS_NPC(ch) && !IS_NPC(gch)))
		{
			spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
			spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_mass_invis( int sn, int level, char_data *ch, void *, int )
{
	AFFECT_DATA af;
	char_data *gch;

	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
		if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
			continue;
		act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
		gch->printf( "You slowly fade out of existence.\r\n" );

		af.where     = WHERE_AFFECTS;
		af.type      = sn;
		af.level     = level/2;
		af.duration  = 24;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_INVISIBLE;
		affect_to_char( gch, &af );
	}
	ch->println( "Ok." );

    return FULL_MANA;
}



/**************************************************************************/
SPRESULT spell_null( int, int , char_data *ch, void *, int  )
{
	ch->printf( "That's not a spell!\r\n" );
	return NO_MANA;
}

/**************************************************************************/
SPRESULT spell_pass_door( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
	{
		if (victim == ch)
			ch->printf( "You are already out of phase.\r\n" );
		else
			act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = number_fuzzy( level / 4 );
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_PASS_DOOR;
	affect_to_char( victim, &af );
	act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
	victim->printf( "You turn translucent.\r\n" );
	return FULL_MANA;
}

/**************************************************************************/
// RT plague spell, very nasty 
SPRESULT spell_plague( int sn, int level, char_data *ch, void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if (saves_spell(level,victim,DAMTYPE(sn))
	|| (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
	|| HAS_CLASSFLAG(victim, CLASSFLAG_PLAGUE_IMMUNITY))
	{
		if (ch == victim){
			ch->printf( "You feel momentarily ill, but it passes.\r\n" );
		}else{
			act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
		}
		return FULL_MANA;
	}

	if ( IS_ICIMMORTAL(victim)){
		act( "$n doesn't appear to be even slightly affected by your magic.", victim, NULL, NULL, TO_ROOM );
		return FULL_MANA;
	}

	af.where		= WHERE_AFFECTS;
	af.type			= sn;
	af.level		= level * 3/4;
	af.duration		= level;
	af.location		= APPLY_ST;
	af.modifier		= -10;
	af.bitvector	= AFF_PLAGUE;
	affect_join(victim,&af);
   
	victim->printf( "You scream in agony as plague sores erupt from your skin.\r\n" );
	act("$n screams in agony as plague sores erupt from $s skin.",victim,NULL,NULL,TO_ROOM);

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_poison( int sn, int level, char_data *ch, void *vo, int target )
{
	char_data *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;

		if (obj->item_type == ITEM_FOOD
		||  obj->item_type == ITEM_DRINK_CON)
		{
			obj->value[3] = 1;
			act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
			return FULL_MANA;
		}

		if (obj->item_type == ITEM_WEAPON)
		{
			if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
			||  IS_WEAPON_STAT(obj,WEAPON_FROST)
			||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
			||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
			||  IS_WEAPON_STAT(obj,WEAPON_HOLY))
			{
				act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
				return FULL_MANA;
			}

			if (IS_WEAPON_STAT(obj,WEAPON_POISON))
			{
				act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
				return HALF_MANA;
			}

			af.where		= WHERE_WEAPON;
		    af.type			= sn;
		    af.level		= level / 2;
		    af.duration		= level/8;
			af.location		= APPLY_NONE;
		    af.modifier		= 0;
		    af.bitvector	= WEAPON_POISON;
		    affect_to_obj(obj,&af);

		    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
		    return FULL_MANA;
		}

		act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
		return NO_MANA;
	}

	victim = (char_data *) vo;

	if ( saves_spell( level, victim,DAMTYPE(sn))
	||   HAS_CLASSFLAG(victim, CLASSFLAG_POISON_IMMUNITY))
	{
		act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
		victim->printf( "You feel momentarily ill, but it passes.\r\n" );
		return FULL_MANA;
	}

	if ( IS_ICIMMORTAL(victim)){
		act( "$n doesn't appear to be even slightly affected by your magic.", victim, NULL, NULL, TO_ROOM );
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (level/2);
	af.location  = APPLY_ST;
	af.modifier  = -4;
	af.bitvector = AFF_POISON;
	affect_join( victim, &af );
	victim->printf( "You feel very sick.\r\n" );
	act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_protection_evil(int sn,int level,char_data *ch,void *vo, int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL)
	||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
	{
		if (victim == ch)
			ch->printf( "You are already protected.\r\n" );
		else
			act("$N is already protected.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_SAVES;
	af.modifier  = -1;
	af.bitvector = AFF_PROTECT_EVIL;
	affect_to_char( victim, &af );
	victim->printf( "You feel holy and pure.\r\n" );
	if ( ch != victim )
		act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
	return FULL_MANA;
}
 
/**************************************************************************/
SPRESULT spell_protection_good(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;
 
	if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD)
	||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
	{
		if (victim == ch)
			ch->printf( "You are already protected.\r\n" );
		else
			act("$N is already protected.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_SAVES;
	af.modifier  = -1;
	af.bitvector = AFF_PROTECT_GOOD;
	affect_to_char( victim, &af );
	victim->printf( "You feel aligned with darkness.\r\n" );
	if ( ch != victim )
		act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_ray_of_truth (int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;

	if (IS_EVIL(ch) )
	{
		victim = ch;
		ch->printf( "The energy explodes inside you!\r\n" );
	}
 
	if (victim != ch)
	{
		act("$n raises $s hand, and a blinding ray of light shoots forth!",ch,NULL,NULL,TO_ROOM);
		ch->printf( "You raise your hand and a blinding ray of light shoots forth!\r\n" );
	}

	if (IS_GOOD(victim))
	{
		act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
		victim->printf( "The light seems powerless to affect you.\r\n" );
		return FULL_MANA;
	}

	dam = dice( level, 12 ) / abs(4+victim->alliance);
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;

	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	spell_blindness(gsn_blindness, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_recharge( int , int level, char_data *ch, void *vo,int )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance, percent;

	if (obj->item_type != ITEM_WAND
	&&  obj->item_type != ITEM_STAFF)
	{
		ch->printf( "That item does not carry charges.\r\n" );
		return NO_MANA;
    }

	if (obj->value[3] >= 3 * level / 2)
	{
		ch->printf( "Your skills are not great enough for that.\r\n" );
		return FULL_MANA;
	}

	if (obj->value[1] == 0)
	{
		ch->printf( "That item has already been recharged once.\r\n" );
		return HALF_MANA;
	}

	chance = 40 + 2 * level;
	chance -= obj->value[3]; /* harder to do high-level spells */
	chance -= (obj->value[1] - obj->value[2]) * (obj->value[1] - obj->value[2]);
    chance = UMAX(level/2,chance);

	percent = number_percent();

	if (percent < chance / 2)
	{
		act("$p glows softly.",ch,obj,NULL,TO_CHAR);
		act("$p glows softly.",ch,obj,NULL,TO_ROOM);
		obj->value[2] = UMAX(obj->value[1],obj->value[2]);
		obj->value[1] = 0;
		return FULL_MANA;
	}
	else if (percent <= chance)
	{
		int chargeback,chargemax;

		act("$p glows softly.",ch,obj,NULL,TO_CHAR);
		act("$p glows softly.",ch,obj,NULL,TO_CHAR);

		chargemax = obj->value[1] - obj->value[2];
	
		if (chargemax > 0)
			chargeback = UMAX(1,chargemax * percent / 100);
		else
			chargeback = 0;

		obj->value[2] += chargeback;
		obj->value[1] = 0;
		return FULL_MANA;
	}
	else if (percent <= UMIN(95, 3 * chance / 2))
	{
		ch->printf( "Nothing seemed to happen.\r\n" );
		if (obj->value[1] > 1)
			obj->value[1]--;
		return FULL_MANA;
	}
	else /* whoops! */
	{
		act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
		act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
		extract_obj(obj);
	}

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_refresh( int , int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;

	victim->move = UMIN( victim->move + level, victim->max_move );
	
	if(!IS_NPC(victim) && victim->pcdata->tired!=-1)
	{
		victim->pcdata->tired-=level/25+2;
		if(victim->pcdata->tired<0)
			victim->pcdata->tired=0;
	}
	if (victim->max_move == victim->move)
		victim->printf( "You feel fully refreshed!\r\n" );
	else
		victim->printf(  "You feel less tired.\r\n" );
	if ( ch != victim )
		ch->println( "Ok." );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_remove_curse( int , int level, char_data *ch, void *vo,int target)
{
	char_data *victim;
	OBJ_DATA *obj;
	bool found = false;

	// do object cases first 
	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;
		if(obj==NULL)
			return NO_MANA;

		if (IS_OBJ_STAT(obj,OBJEXTRA_NODROP)
		|| IS_OBJ_STAT(obj,OBJEXTRA_NOREMOVE))
		{
			if (!IS_OBJ_STAT(obj,OBJEXTRA_NOUNCURSE)
			&&  !saves_dispel(level *2,obj->level,0))
			{
				REMOVE_BIT(obj->extra_flags,OBJEXTRA_NODROP);
				REMOVE_BIT(obj->extra_flags,OBJEXTRA_NOREMOVE);
				act("$p glows blue.",ch,obj,NULL,TO_ALL);
				return FULL_MANA;
			}

			act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
			return FULL_MANA;
		}

		act("$p is not cursed.",ch,obj,NULL,TO_CHAR);
		return HALF_MANA;
	}

    /* characters */
	victim = (char_data *) vo;

	if (!is_affected(victim, gsn_curse) || check_dispel(level,victim,gsn_curse))
	{
		victim->printf( "You feel better.\r\n" );
		act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
		for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
		{
			if ((IS_OBJ_STAT(obj,OBJEXTRA_NODROP)
			|| IS_OBJ_STAT(obj,OBJEXTRA_NOREMOVE))
			&&  !IS_OBJ_STAT(obj,OBJEXTRA_NOUNCURSE))
			{   // attempt to remove curse 
				if (!saves_dispel(IS_HEALER(ch)?level*3:level/2,obj->level*3,0))
				{
					found = true;
					REMOVE_BIT(obj->extra_flags,OBJEXTRA_NODROP);
					REMOVE_BIT(obj->extra_flags,OBJEXTRA_NOREMOVE);
					act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
					act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
				}
			}
		}
	}
	else
	{
		ch->printf( "It doesn't appear your magic has had any affect.\r\n" );
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_sanctuary( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_SANCTUARY ))
	{
		if (victim == ch)
			ch->printf( "You are already in sanctuary.\r\n" );
		else
			act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;

	if ( IS_IMMORTAL( ch ))
		af.duration  = level / 6;
	else
		af.duration  = 1 + level / 12;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SANCTUARY;
	affect_to_char( victim, &af );
	act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
	victim->printf( "You are surrounded by a white aura.\r\n" );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_shield( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( is_affected( victim, sn ) )
	{
		if (victim == ch)
			ch->printf( "You are already shielded from harm.\r\n" );
		else
			act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	if ( is_affected( victim, gsn_oak_shield ))
	{
		if (victim == ch)
			ch->printf( "You are already protected by a shielding spell.\r\n" );
		else
			act("$N is already protected by a shielding spell.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 8 + level;
	af.location  = APPLY_AC;
	af.modifier  = -20;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
	victim->printf( "You are surrounded by a force shield.\r\n" );
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_shocking_grasp(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;

	dam		= dice(level/4,2)+20;
	if ( saves_spell( level, victim,DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn),true);
	return FULL_MANA;
}

/**************************************************************************/
void dismount(char_data * rider);
/**************************************************************************/
SPRESULT spell_sleep( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( IS_SET( victim->imm_flags, IMM_SLEEP ) || IS_ICIMMORTAL(victim)){
		act( "$n doesn't appear to be even slightly affected by your magic.", victim, NULL, NULL, TO_ROOM );
		return FULL_MANA;
	}

	if ( IS_AFFECTED(victim, AFF_SLEEP)
	|| ( IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
	|| ( level + 2) < victim->level )
	{
		ch->println( "You failed." );
		return FULL_MANA;
	}

	if(saves_spell( level-4, victim,DAMTYPE(sn)) )
	{
		ch->println( "You failed." );
		// stop cheaters that panic logging out straight away
		if (!IS_NPC(victim) && victim!=ch)
		{
			victim->pknoquit=UMAX(victim->pknoquit,2);
		}
		if (!IS_NPC(ch) && !IS_NPC(victim) && victim!=ch)
		{
			ch->pksafe=0;
			ch->pknorecall= UMAX(ch->pknorecall,2);
			ch->pknoquit  = UMAX(ch->pknorecall,7);
		}
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = number_fuzzy(level);
	if(IS_NPC(victim))
		af.duration  = 4 + (level/2);
	else
		af.duration  = 2 + (level/9);  // duration reduced
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SLEEP;
	affect_join( victim, &af );

	if ( IS_AWAKE(victim))
	{
		if (IS_RIDING(victim)){
			victim->printlnf("You fall off %s and drift off into a deep sleep.", PERS(victim->mounted_on, victim));
			act( "$n falls off $N and lands on the ground, slipping into a deep sleep.", victim, NULL, victim->mounted_on, TO_ROOM );
			victim->mounted_on->tethered=false;
			dismount(victim);
		}else{
			victim->println( "You feel very sleepy ..... zzzzzz." );
			act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
		}
		victim->position = POS_SLEEPING;

		// stop those who are slept from logging out
		if (!IS_NPC(victim) && victim!=ch)
		{
			ch->pksafe=0;
			ch->pknorecall= UMAX(ch->pknorecall,2);
			ch->pknoquit  = UMAX(ch->pknorecall,7);

			if (!IS_NPC(ch)){
				victim->pknoquit=UMAX(victim->pknoquit,10);
			}
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_slow( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if ( is_affected( victim, sn )
	|| IS_AFFECTED(victim,AFF_SLOW))
	{
		if (victim == ch)
			ch->printf( "You can't move any slower!\r\n" );
		else
			act("$N can't get any slower than that.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	if (saves_spell(level,victim,DAMTYPE(sn))
	||  IS_SET(victim->imm_flags,IMM_MAGIC))
	{
		if (victim != ch)
			ch->printf( "Nothing seemed to happen.\r\n" );
		victim->printf( "You feel momentarily lethargic.\r\n" );
		return FULL_MANA;
	}

	if (IS_AFFECTED(victim,AFF_HASTE))
	{
		if (!check_dispel(level,victim, gsn_haste ))
		{
			if (victim != ch)
			{
				ch->println("Spell failed.");
				return FULL_MANA;
			}
		}
		act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level/2;
	af.location  = APPLY_QU;
	af.modifier  = -1 - level/5;
	af.bitvector = AFF_SLOW;
	affect_to_char( victim, &af );
	victim->printf( "You feel yourself slowing d o w n...\r\n" );
	act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_stone_skin( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if(check_skin( ch, victim ))
		return HALF_MANA;

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -40;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
	victim->printf( "Your skin turns to stone.\r\n" );
	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_summon( int sn, int level, char_data *ch, void *,int  )
{
	char_data *victim;

	if ( ( victim = get_char_icworld( ch, target_name ) ) == NULL
	||   victim == ch
	||   victim->in_room == NULL
	||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
	||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	||   IS_SET(victim->in_room->room_flags, ROOM_NOSCRY)
	||   IS_SET(victim->in_room->area->area_flags, AREA_NOSCRY)
	||   IS_SET(ch->in_room->area->area_flags, AREA_NOSUMMONINTO)	
	||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
	||   victim->level >= level + 3
	||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
	||   victim->fighting != NULL
	||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
	||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
	||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
	||   (IS_NPC(victim) && saves_spell( level, victim,DAMTYPE(sn)))
	||	 (IS_SET(victim->form, FORM_MOUNTABLE) // can't summon mounts to inside
	&&	  IS_SET(ch->in_room->room_flags, ROOM_INDOORS))
	||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE)) )
	{
		ch->println( "You failed." );
		return FULL_MANA;
	}

	// no charming in aggy mobs
	if(IS_NPC(victim)){
		REMOVE_BIT(victim->act, ACT_AGGRESSIVE);
	}

	act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
	char_from_room( victim );
	char_to_room( victim, ch->in_room );
	act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
	act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
	do_look( victim, "auto" );

	/* 
	 * If someone is following the char, these triggers get activated
	 * for the followers before the char, but it's safer this way...
	 */
	if ( IS_NPC( victim)
	&& HAS_TRIGGER( victim, MTRIG_ENTRY ) )
		mp_percent_trigger( victim, NULL, NULL, NULL, MTRIG_ENTRY );

	if ( !IS_NPC( victim ))
		mp_greet_trigger( victim );

	return FULL_MANA;
}


/**************************************************************************/
SPRESULT spell_teleport( int sn, int level, char_data *ch, void *vo,int  )
{
	char_data *victim = (char_data *) vo;
	ROOM_INDEX_DATA *pRoomIndex;

	if ( victim->in_room == NULL
	||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	|| ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
	|| ( !IS_NPC(ch) && victim->fighting != NULL )
	|| ( victim != ch
	&& ( saves_spell( level - 5, victim,DAMTYPE(sn)))))
	{
		ch->println( "You failed." );
		return FULL_MANA;
	}

	pRoomIndex = get_random_room(victim);

	if (victim != ch)
		victim->printf( "You have been teleported!\r\n" );

	act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
	char_from_room( victim );
	char_to_room( victim, pRoomIndex );
	act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
	do_look( victim, "auto" );

	/* 
	 * If someone is following the char, these triggers get activated
	 * for the followers before the char, but it's safer this way...
	 */
	if ( IS_NPC( victim)
	&& HAS_TRIGGER( victim, MTRIG_ENTRY ))
		mp_percent_trigger( victim, NULL, NULL, NULL, MTRIG_ENTRY );

	if ( !IS_NPC( victim ))
		mp_greet_trigger( victim );

	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_ventriloquate( int sn, int level, char_data *ch,void *,int )
{
	char buf1[MSL];
	char buf2[MSL];
	char speaker[MIL];
	char_data *vch;

	target_name = one_argument( target_name, speaker );

	sprintf( buf1, "%s says '%s'.", speaker, target_name );
	sprintf( buf2, "Someone makes %s say '%s'.", speaker, target_name );
	buf1[0] = UPPER(buf1[0]);

	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
	{
		if (IS_AWAKE(vch) && !is_affected(vch, gsn_deafness))
		{
			if ( !is_name( speaker, vch->name ))
				vch->printf( "%s\r\n", saves_spell(level,vch,DAMTYPE(sn)) ? buf2 : buf1 );
		}
	}

    return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_weaken( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	AFFECT_DATA af;

	if( is_affected( victim, gsn_giant_strength )){
		if( check_dispel( level, victim, gsn_giant_strength )){
			act( "Your magic unravels a spell on $N.", ch, NULL, victim, TO_CHAR );
			return FULL_MANA;
		}else{
			ch->println( "You failed." );
			return FULL_MANA;
		}
	}

	if ( is_affected( victim, sn ) )
	{
		act("$N already looks tired and weak.",ch,NULL,victim,TO_CHAR);
		return HALF_MANA;
	}

	if (saves_spell( level, victim,DAMTYPE(sn)) )
	{
		ch->printf( "Your spell doesnt appear to have much affect.\r\n" );
		return FULL_MANA;
	}

	af.where     = WHERE_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 2;
	af.location  = APPLY_ST;
	af.modifier  = -1 * (level / 3);
	af.bitvector = AFF_WEAKEN;
	affect_to_char( victim, &af );
	victim->printf( "You feel your strength slip away.\r\n" );
	act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_word_of_recall( int, int, char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	ROOM_INDEX_DATA *location = NULL;

	if(IS_NPC(victim)){
		return NO_MANA;
	}

	bool using_pendant=true;
	location=get_room_index(get_recallvnum(victim, &using_pendant));
	if(!location){
		victim->println( "You are completely lost." );
		return NO_MANA;
	}

	if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL)
	||  IS_SET(victim->in_room->room_flags, ROOM_ANTIMAGIC)
	||  IS_AFFECTED(victim,AFF_CURSE))
    {
		victim->println( "Spell failed." );
		return FULL_MANA;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim,true);
    
	// Stop negative moves characters from defrauding the system
	if (ch->move>0 && IS_IC(ch))
		ch->move /= 2;

	act("$n disappears.",victim,NULL,NULL,TO_ROOM);
	char_from_room(victim);
	char_to_room(victim,location);
	act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
	do_look(victim,"auto");
	return FULL_MANA;
}

/**************************************************************************/
// NPC spells.
SPRESULT spell_acid_breath( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam,hp_dam,dice_dam,hpch;

	act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
	act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

	hpch = UMAX(12,ch->hit);
	hp_dam = number_range(hpch/11 + 1, hpch/6);
	dice_dam = dice(level,16);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
	if (saves_spell(level,victim,DAMTYPE(sn)))
	{
		acid_effect(victim,level/2,dam/4,TARGET_CHAR);
		damage_spell(ch,victim,dam/2,sn,DAMTYPE(sn),true);
	}
	else
	{
		acid_effect(victim,level,dam,TARGET_CHAR);
		damage_spell(ch,victim,dam,sn,DAMTYPE(sn),true);
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_fire_breath( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	char_data *vch, *vch_next;
	int dam,hp_dam,dice_dam;
	int hpch;

	act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
	act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX( 10, ch->hit );
	hp_dam  = number_range( hpch/9+1, hpch/5 );
	dice_dam = dice(level,20);

	dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
	fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,true) 
		||  (IS_NPC(vch) && IS_NPC(ch) 
		&&   (ch->fighting != vch || vch->fighting != ch)))
		continue;

		if (vch == victim) /* full damage */
		{
			if (saves_spell(level,vch,DAMTYPE(sn)))
			{
				fire_effect(vch,level/2,dam/4,TARGET_CHAR);
				damage_spell(ch,vch,dam/2,sn,DAMTYPE(sn),true);
			}
			else
			{
				fire_effect(vch,level,dam,TARGET_CHAR);
				damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
			}
		}
		else /* partial damage */
		{
			if (saves_spell(level - 2,vch,DAMTYPE(sn)))
			{
				fire_effect(vch,level/4,dam/8,TARGET_CHAR);
				damage_spell(ch,vch,dam/4,sn,DAMTYPE(sn),true);
			}
			else
			{
				fire_effect(vch,level/2,dam/4,TARGET_CHAR);
				damage_spell(ch,vch,dam/2,sn,DAMTYPE(sn),true);
			}
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_frost_breath( int sn, int level, char_data *ch, void *vo,int )
{
	char_data *victim = (char_data *) vo;
	char_data *vch, *vch_next;
	int dam,hp_dam,dice_dam, hpch;

	act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a freezing cone of frost over you!",ch,NULL,victim,TO_VICT);
	act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(12,ch->hit);
	hp_dam = number_range(hpch/11 + 1, hpch/6);
	dice_dam = dice(level,16);
	
	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
	cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,true)
		|| (IS_NPC(vch) && IS_NPC(ch) 
		&& (ch->fighting != vch || vch->fighting != ch)))
		continue;

		if (vch == victim) /* full damage */
		{
			if (saves_spell(level,vch,DAMTYPE(sn)))
			{
				cold_effect(vch,level/2,dam/4,TARGET_CHAR);
				damage_spell(ch,vch,dam/2,sn,DAMTYPE(sn),true);
			}
			else
			{
				cold_effect(vch,level,dam,TARGET_CHAR);
				damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
			}
		}
		else
		{
			if (saves_spell(level - 2,vch,DAMTYPE(sn)))
			{
				cold_effect(vch,level/4,dam/8,TARGET_CHAR);
				damage_spell(ch,vch,dam/4,sn,DAMTYPE(sn),true);
			}
			else
			{
				cold_effect(vch,level/2,dam/4,TARGET_CHAR);
				damage_spell(ch,vch,dam/2,sn,DAMTYPE(sn),true);
			}
		}
	}
	return FULL_MANA;
}

   
/**************************************************************************/
SPRESULT spell_gas_breath( int sn, int level, char_data *ch, void *,int  )
{
	char_data *vch;
	char_data *vch_next;
	int dam,hp_dam,dice_dam,hpch;

	act("$n breathes out a cloud of hot poisonous gas!",ch,NULL,NULL,TO_ROOM);
	act("You breath out a cloud of hot poisonous gas.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(16,ch->hit);
	hp_dam = number_range(hpch/15+1,8);
	dice_dam = dice(level,12);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
	poison_effect(ch->in_room,level,dam,TARGET_ROOM);

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,true)
		|| (IS_NPC(ch) && IS_NPC(vch) 
		&& (ch->fighting == vch || vch->fighting == ch)))
		continue;

		if (check_immune(vch,DAMTYPE(sn))==IS_IMMUNE 
			|| HAS_CLASSFLAG(vch, CLASSFLAG_POISON_IMMUNITY))
		{
			vch->wrapln("The heat of the gas burns at your skin."
			"  You feel momentarily ill, but it passes.");
			damage_spell(ch,vch,dam/4,sn,DAMTYPE(sn),true);
		}
		else if (saves_spell(level,vch,DAMTYPE(sn)))
		{
			poison_effect(vch,level/2,dam/4,TARGET_CHAR);
			damage_spell(ch,vch,dam/2,sn,DAMTYPE(sn),true);
		}
		else
		{
			poison_effect(vch,level,dam,TARGET_CHAR);
			damage_spell(ch,vch,dam,sn,DAMTYPE(sn),true);
		}
	}
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_lightning_breath(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam,hp_dam,dice_dam,hpch;

	act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
	act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

	hpch = UMAX(10,ch->hit);
	hp_dam = number_range(hpch/9+1,hpch/5);
	dice_dam = dice(level,20);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

	if (saves_spell(level,victim,DAMTYPE(sn)))
	{
		shock_effect(victim,level/2,dam/4,TARGET_CHAR);
		damage_spell(ch,victim,dam/2,sn,DAMTYPE(sn),true);
	}
	else
	{
		shock_effect(victim,level,dam,TARGET_CHAR);
		damage_spell(ch,victim,dam,sn,DAMTYPE(sn),true);
	}
	return FULL_MANA;
}

/**************************************************************************/
// Spells for mega1.are from Glop/Erkenbrand.
SPRESULT spell_general_purpose(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;
 
	dam = number_range( 25, 100 );
	if ( saves_spell( level, victim, DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn) ,true);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_high_explosive(int sn,int level,char_data *ch,void *vo,int )
{
	char_data *victim = (char_data *) vo;
	int dam;
 
	dam = number_range( 30, 120 );
	if ( saves_spell( level, victim, DAMTYPE(sn)) )
		dam /= 2;
	damage_spell( ch, victim, dam, sn, DAMTYPE(sn) ,true);
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_farsight( int , int , char_data *ch, void *,int )
{
	if (IS_AFFECTED(ch,AFF_BLIND))
	{
		ch->printf( "Maybe it would help if you could see?\r\n" );
		return HALF_MANA;
	}
 
	do_far_scan(ch,target_name);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_portal( int sn, int level, char_data *ch, void *,int )
{
	char_data *victim;
	OBJ_DATA *portal, *stone;

	if ((victim = get_char_world( ch, target_name ) ) == NULL
	||	 victim == ch
    ||	 victim->in_room == NULL
    ||	 !can_see_room(ch,victim->in_room)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	||	 IS_SET(victim->in_room->room_flags, ROOM_ANTIMAGIC)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||	 IS_SET(victim->in_room->area->area_flags, AREA_NOPORTALINTO)
    ||	 IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    
	||	 victim->level >= level + 3
    ||	 (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
	||	 (IS_NPC(victim) && saves_spell( level, victim,DAMTYPE(sn)) )
    ||	 (is_clan(victim) && !is_same_clan(ch,victim)))
    {
        ch->println( "You failed." );
        return FULL_MANA;
    }   

	stone = get_eq_char(ch,WEAR_HOLD);
	
	if (!IS_IMMORTAL(ch) 
	&&  (stone == NULL
	|| stone->item_type != ITEM_WARP_STONE))
	{
		ch->println( "You lack the proper component for this spell." );
		return NO_MANA;
	}

	if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
	{
		act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
		act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
		extract_obj(stone);
	}

	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL));
	portal->timer = 2 + level / 25; 
	portal->value[3] = victim->in_room->vnum;

	// portals aren't opaque - Kal August 98
	SET_BIT(portal->value[2],GATE_OPAQUE);

	obj_to_room(portal,ch->in_room);

	act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
	act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
	return FULL_MANA;
}

/**************************************************************************/
SPRESULT spell_nexus( int sn, int level, char_data *ch, void *, int)
{
	char_data *victim;
	OBJ_DATA *portal, *stone;
	ROOM_INDEX_DATA *from_room;
	ROOM_INDEX_DATA *to_room=NULL;

	from_room = ch->in_room;
 
	if ((victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   (to_room = victim->in_room) == NULL
    ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
    ||	 IS_SET(from_room->room_flags,ROOM_SAFE)
	||   IS_SET(from_room->room_flags,ROOM_NO_RECALL)
    ||   IS_SET(to_room->room_flags, ROOM_SAFE)
	||	 IS_SET(to_room->room_flags, ROOM_ANTIMAGIC)
    ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
	||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
	||   IS_SET(to_room->area->area_flags, AREA_NOPORTALINTO)
	||   victim->level >= level + 3
	||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
	||   (saves_spell( level, victim,DAMTYPE(sn)) ) )
    {
        ch->println( "You failed." );
        return FULL_MANA;
	}   
 
	stone = get_eq_char(ch,WEAR_HOLD);

	if (!IS_IMMORTAL(ch)
    &&  (stone == NULL
	|| stone->item_type != ITEM_WARP_STONE))
	{
		ch->println( "You lack the proper component for this spell." );
		return NO_MANA;
	}
 
	if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
	{
		act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
		act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
		extract_obj(stone);
	}

	// portal one
	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL));
	portal->timer = 1 + level / 10;
	portal->value[3] = to_room->vnum;
 
	obj_to_room(portal,from_room);
 
	act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
	act("$p rises up before you.",ch,portal,NULL,TO_CHAR);

	// no second portal if rooms are the same 
	if (to_room == from_room)
		return HALF_MANA;

    // portal two 
	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL));
	portal->timer = 1 + level/10;
	portal->value[3] = from_room->vnum;

	obj_to_room(portal,to_room);

	if (to_room->people != NULL)
	{
		act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
		act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
	}
	return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_mute( int sn, int level, char_data *ch, void *vo, int)
{
    char_data* victim = (char_data*) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2(victim, AFF2_MUTE) )
    {
		ch->printlnf("%s is already silenced.", PERS(victim, ch));
		return HALF_MANA;
    }

    if ( saves_spell(level,victim,DAMTYPE(sn)) )
    {
		ch->println("You failed.");
		return HALF_MANA;
    }


    af.where     = WHERE_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_MUTE;
    affect_to_char( victim, &af );
    
    act( "You have silenced $N!", ch, NULL, victim, TO_CHAR    );
    act( "$n has silenced you!",  ch, NULL, victim, TO_VICT    );
    act( "$n has silenced $N!",   ch, NULL, victim, TO_NOTVICT );
    return FULL_MANA;
}
/**************************************************************************/
SPRESULT spell_possession( int sn, int level, char_data *ch, void *vo, int)
{
    char_data* victim = (char_data*) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2(victim, AFF2_POSSESSION) )
    {
		if (victim == ch){
			ch->println("You are already inhabited by a spirit.");
			return NO_MANA;
		}else{
			act("$N is already inhabited by a spirit.",ch,NULL,victim,TO_CHAR);
			return HALF_MANA;
		}
    }

    af.where     = WHERE_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_POSSESSION;
    affect_to_char( victim, &af );
    act( "$n looks stronger and mightier as a spirit of a dead warrior inhabits $s body.",
         victim, NULL, NULL, TO_ROOM );
    victim->println("You suddenly gain the insights of a warrior of old.");
    return FULL_MANA;
}
/**************************************************************************/
/**************************************************************************/
