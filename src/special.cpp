/**************************************************************************/
// special.cpp - mob special functions
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

void do_unlock( char_data *ch, char *argument );
void do_lock(   char_data *ch, char *argument );
void do_order(  char_data *ch, char *argument );
void do_pkill( char_data *ch, char *argument );

void say_spell( char_data *ch, int sn, CLASS_CAST_TYPE type );

// these really only relate to rom based area files
#define MOB_VNUM_PATROLMAN         2106 
#define GROUP_VNUM_TROLLS          2100
#define GROUP_VNUM_OGRES           2101

/* direction command procedures needed */
DECLARE_DO_FUN(do_north     );
DECLARE_DO_FUN(do_south     );
DECLARE_DO_FUN(do_west      );
DECLARE_DO_FUN(do_east      );

/* other command procedures needed */
DECLARE_DO_FUN(do_pbackstab  );
DECLARE_DO_FUN(do_close     );
DECLARE_DO_FUN(do_flee      );
DECLARE_DO_FUN(do_kill      );
DECLARE_DO_FUN(do_open      );
DECLARE_DO_FUN(do_say       );
DECLARE_DO_FUN(do_sleep     );
DECLARE_DO_FUN(do_wake      );
DECLARE_DO_FUN(do_yell      );


// The following special functions are available for MOBS
DECLARE_SPEC_FUN(	spec_breath_any			);
DECLARE_SPEC_FUN(	spec_breath_acid		);
DECLARE_SPEC_FUN(	spec_breath_fire		);
DECLARE_SPEC_FUN(	spec_breath_frost		);
DECLARE_SPEC_FUN(	spec_breath_gas			);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_breath_steam		);
DECLARE_SPEC_FUN(	spec_cast_adept			);
DECLARE_SPEC_FUN(	spec_cast_cleric		);
DECLARE_SPEC_FUN(	spec_cast_druid			);
DECLARE_SPEC_FUN(	spec_cast_mage			);
DECLARE_SPEC_FUN(	spec_cast_undead		);
DECLARE_SPEC_FUN(	spec_fido				);
DECLARE_SPEC_FUN(	spec_guard				);
DECLARE_SPEC_FUN(	spec_janitor			);
DECLARE_SPEC_FUN(	spec_mayor				);
DECLARE_SPEC_FUN(	spec_poison				);
DECLARE_SPEC_FUN(	spec_thief				);
DECLARE_SPEC_FUN(	spec_nasty				);
DECLARE_SPEC_FUN(	spec_gold_dragon        );
DECLARE_SPEC_FUN(	spec_red_drac           );
DECLARE_SPEC_FUN(	spec_fearful            );
DECLARE_SPEC_FUN(	spec_lava_monster       ); 	
DECLARE_SPEC_FUN(	spec_assassin       	); 
DECLARE_SPEC_FUN(	spec_beggar				); 
DECLARE_SPEC_FUN(	spec_summoned_guardian	); 
DECLARE_SPEC_FUN(	spec_elf_muncher		); 
DECLARE_SPEC_FUN(	spec_shadow_dragon		);
DECLARE_SPEC_FUN(	spec_steel_dragon		);
DECLARE_SPEC_FUN(	spec_cast_mean			);
DECLARE_SPEC_FUN(	spec_cast_meaner		);
DECLARE_SPEC_FUN(	spec_charmer			);
DECLARE_SPEC_FUN(   spec_master_thief       );
DECLARE_SPEC_FUN(	spec_questmaster		);
DECLARE_SPEC_FUN(	spec_troll_member		);
DECLARE_SPEC_FUN(	spec_ogre_member		);
DECLARE_SPEC_FUN(	spec_clan_transport     );

// The following special functions are available for OBJS
DECLARE_OSPEC_FUN(	spec_obj_test			);

// Special Functions Table -- MOBS
const	struct	spec_type	spec_table	[ ] =
{
    // Special function commands.

    { "spec_breath_any",		spec_breath_any			},
    { "spec_breath_acid",		spec_breath_acid		},
    { "spec_breath_fire",		spec_breath_fire		},
    { "spec_breath_frost",		spec_breath_frost		},
    { "spec_breath_gas",		spec_breath_gas			},
    { "spec_breath_lightning",	spec_breath_lightning	},
    { "spec_breath_steam",		spec_breath_steam		},	
    { "spec_cast_adept",		spec_cast_adept			},
    { "spec_cast_cleric",		spec_cast_cleric		},
    { "spec_cast_druid",		spec_cast_druid			},
    { "spec_cast_mage",			spec_cast_mage			},
    { "spec_cast_undead",		spec_cast_undead		},
    { "spec_fido",				spec_fido				},
    { "spec_guard",				spec_guard				},
    { "spec_janitor",			spec_janitor			},
    { "spec_mayor",				spec_mayor			    },
    { "spec_poison",			spec_poison				},
    { "spec_thief",				spec_thief			    },
    { "spec_nasty",				spec_nasty				},
    { "spec_gold_dragon",		spec_gold_dragon		},
    { "spec_red_drac",			spec_red_drac			}, 
    { "spec_fearful",			spec_fearful			},
    { "spec_lava_monster",		spec_lava_monster		},  
    { "spec_assassin",			spec_assassin			},      
    { "spec_beggar",			spec_beggar				},
    { "spec_summoned_guardian", spec_summoned_guardian	},      
    { "spec_elf_muncher",		spec_elf_muncher		},      
    { "spec_shadow_dragon",		spec_shadow_dragon		},      
    { "spec_steel_dragon",		spec_steel_dragon		},
    { "spec_charmer",           spec_charmer            },
    { "spec_master_thief",      spec_master_thief       },
    { "spec_cast_mean",			spec_cast_mean			},
    { "spec_cast_meaner",		spec_cast_meaner		},
    { "spec_questmaster",		spec_questmaster		},
    { "spec_troll_member",		spec_troll_member		},
    { "spec_ogre_member",		spec_ogre_member		},
    { "spec_clan_transport",	spec_clan_transport		},	
	{ "",						0						}
};

// Special Functions Table -- OBJS
const   struct  ospec_type    ospec_table[] =
{
    { "spec_obj_test",			spec_obj_test			},
	{	NULL,					NULL					}
};

/*****************************************************************************
 Name:		ospec_lookup
 Purpose:	Given a name, return the appropriate object spec fun.
 ****************************************************************************/
OSPEC_FUN *ospec_lookup( const char *name )
{
	int i;
	for ( i = 0; ospec_table[i].ospec_name != NULL; i++)
	{
		if (LOWER(name[0]) == LOWER(ospec_table[i].ospec_name[0])
			&&  !str_prefix( name,ospec_table[i].ospec_name))
			return ospec_table[i].ospec_fun;
	}
	return 0;
}


/*****************************************************************************
 Name:		spec_lookup
 Purpose:	Given a name, return the appropriate mobile spec fun.
 Called by:	do_mset(act_wiz.c) load_specials,reset_area(db.c)
 ****************************************************************************/
SPEC_FUN *spec_lookup( const char *name )	// OLC
{
	int cmd;
	char buf[MSL], bufmatch[MSL];
	
	strcpy(buf, name);	
	buf[5]='\0';
	if (!str_cmp( buf, "spec_"))
	{
		strcpy(bufmatch, name);
	}
	else
	{
		sprintf(bufmatch,"spec_%s", name);
	}
	
	for ( cmd = 0; !IS_NULLSTR(spec_table[cmd].spec_name); cmd++ )
		if ( !str_prefix( bufmatch, spec_table[cmd].spec_name ) )
			return spec_table[cmd].spec_fun;
		
	return NULL;
}

/*****************************************************************************
 Name:		spec_string
 Purpose:	Given a function, return the appropriate name.
 Called by:	<???>
 ****************************************************************************/
char *spec_string( SPEC_FUN *fun )	/* OLC */
{
	int cmd;
	
	for ( cmd = 0; spec_table[cmd].spec_fun; cmd++ )
		if ( fun == spec_table[cmd].spec_fun )
			return spec_table[cmd].spec_name;
		
		return 0;
}

/***************************************************************************/
/*modified for OLC */
char *spec_name( SPEC_FUN *function)
{
	char *i;
	i=spec_string( function );
	return i;
}

/***************************************************************************/
// returns the name of the object special function
char *ospec_name( OSPEC_FUN *function)
{
	int i;

	for (i = 0; ospec_table[i].ospec_fun != NULL; i++)
	{
		if (function == ospec_table[i].ospec_fun )
			return ospec_table[i].ospec_name;
	}
	return NULL;
}

/***************************************************************************
 !!!!!!!!!!!!!!!!!!!!!!!MOB SPECIAL FUNCTIONS BELOW HERE!!!!!!!!!!!!!!!!!!!!
 ***************************************************************************/
bool spec_fearful( char_data *ch )
{
	if(ch->position==POS_FIGHTING)
	{
		do_yell(ch,"HELP! I am being attacked!!! HELP!!!");
		do_flee(ch,"");
		return true;
	}
	return false ;
}

/***************************************************************************/ 
bool spec_red_drac( char_data *ch )
{
    char_data *victim;
    char_data *v_next;
    char *spell;
    int sn;
	
    if(ch->position!=POS_FIGHTING)
    {
		if(ch->hit<ch->max_hit)
		{
			sn = gsn_heal;
			(*skill_table[sn].spell_fun) (sn, ch->level, ch, ch, TARGET_CHAR);
		}
		if(number_range(1,100)>99)
		{
			switch(number_range(0,3))
			{
			case 0: spell = "haste"; break;
			case 1: spell = "fire shield"; break;
			case 2: spell = "protection good"; break;
			case 3: spell = "sanctuary"; break; 
			default: spell = "protection good"; break;
			}
			if( ( sn = skill_lookup(spell) ) < 0 ) return false;
			(*skill_table[sn].spell_fun) ( sn, ch->level, ch, ch,TARGET_CHAR);
			return true;
			
		}	  
		return false;
    }
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch ) 
            break;
    }
    
    if ( victim == NULL )
        return false;
	
    for ( ; ; )
    {
        switch ( number_range(0, 15) )
        {
        case  0: spell = "blindness";			break;
        case  1: spell = "chain lightning";		break;
        case  2: spell = "acid blast";			break;
        case  3: spell = "chain lightning";		break;
        case  4: spell = "frostball";			break;
        case  5: spell = "curse";				break;
        case  6: spell = "poison";				break;
        case  7: spell = "heat metal";			break;
        case  8: spell = "teleport";			break;
        case  9:
        case 10: spell = "wrath";				break;
        case 11: spell = "prismatic spray";		break;
        default: spell = "dispel magic";		break;
        }
		break ;
    }
    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return false;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return true;
}

/***************************************************************************/	
bool spec_nasty( char_data *ch )
{
    char_data *victim, *v_next;
	long gold;
	
    if (!IS_AWAKE(ch)) {
		return false;
    }

    if (ch->position != POS_FIGHTING ) {
		for ( victim = ch->in_room->people; victim != NULL; victim = v_next)
		{
			v_next = victim->next_in_room;
			if (!IS_NPC(victim)
				&& (victim->level > ch->level)
				&& (victim->level < ch->level + 10))
			{
				do_pbackstab(ch,victim->name);
				if (ch->position != POS_FIGHTING){
					do_pkill(ch,victim->name);
				}
				// should steal some coins right away? :) 
				return true;
			}
		}
		return false;    //  No one to attack 
    }
	
	// okay, we must be fighting.... steal some coins and flee 
    if ( (victim = ch->fighting) == NULL)
        return false;   /* let's be paranoid.... */
	
    switch ( number_range(0, 3) )
    {
	case 0:  act( "$n rips apart your coin purse, spilling your gold!",
				 ch, NULL, victim, TO_VICT);
		act( "You slash apart $N's coin purse and gather his gold.",
			ch, NULL, victim, TO_CHAR);
		act( "$N's coin purse is ripped apart!",
			ch, NULL, victim, TO_NOTVICT);
		gold = victim->gold / 10;  /* steal 10% of his gold */
		victim->gold -= gold;
		ch->gold     += gold;
		return true;
		
		  case 1:  do_flee( ch, "");
			  return true;
			  
		  default: return false;
    }
}

// Core procedure for dragons.

/***************************************************************************/
bool dragon( char_data *ch, char *spell_name )
{
    char_data *victim;
    char_data *v_next;
    int sn;
	
    if ( ch->position != POS_FIGHTING )
		return false;
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next = victim->next_in_room;
		if ( victim->fighting == ch && number_range(0, 7 ) == 0 )
			break;
	}
	
    if ( victim == NULL )
		return false;
	
    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
		return false;
	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TARGET_CHAR);
    return true;
}




// Special procedures for mobiles.

/***************************************************************************/
bool spec_breath_any( char_data *ch )
{
	if ( ch->position != POS_FIGHTING )
		return false;

	switch ( number_range(0, 7 ) )
	{
		case 0: return spec_breath_fire		( ch );
		case 1:
		case 2: return spec_breath_lightning( ch );
		case 3: return spec_breath_gas		( ch );
		case 4: return spec_breath_acid		( ch );
		case 5:
		case 6:
		case 7: return spec_breath_frost	( ch );
    }

    return false;
}

/***************************************************************************/
bool spec_breath_acid( char_data *ch )
{
    return dragon( ch, "acid breath" );
}

/***************************************************************************/
bool spec_breath_fire( char_data *ch )
{
    return dragon( ch, "fire breath" );
}

/***************************************************************************/
bool spec_breath_frost( char_data *ch )
{
    return dragon( ch, "frost breath" );
}
/***************************************************************************/
bool spec_breath_steam( char_data* ch )
{
    if ( ch->wait > 0 )
        return false;

    if ( is_affected(ch, gsn_neck_thrust))
		return false;
    return dragon( ch, "steam breath" );
}

/***************************************************************************/
bool spec_breath_gas( char_data *ch )
{
    int sn;
	
    if ( ch->position != POS_FIGHTING )
		return false;
	
    if (( sn = gsn_gas_breath) < 0 )
		return false;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, NULL,TARGET_CHAR);
    return true;
}

/***************************************************************************/
bool spec_gold_dragon( char_data *ch )
{
	
	if(ch->in_room==NULL)
		return false;
	
	if( (ch->in_room->vnum==15024) && (ch->in_room->exit[3]!=NULL) 
		&& (!IS_SET(ch->in_room->exit[3]->exit_info,EX_CLOSED)))
	{
		if(!IS_AWAKE(ch))
		{ do_wake(ch,""); }
		else
		{ do_west(ch,""); }
		return true;
	}
	
	if( (ch->in_room->vnum==15025) && (ch->position!=POS_FIGHTING) )
	{
		do_east(ch, "");
		do_sleep(ch, "");
		return true;
	}
	
	if(ch->position==POS_FIGHTING)
	{	
		switch ( number_range(0, 3 ) )
		{
        case 0: return dragon(ch, "prismatic spray"); 
		case 1: return dragon(ch, "fire breath");
		case 2: return dragon(ch, "gas breath");
		case 3: return dragon(ch, "acid breath");
		} 	
		
	}	
	
	return false;	
}
/***************************************************************************/
bool spec_breath_lightning( char_data *ch )
{
    return dragon( ch, "lightning breath" );
}

/***************************************************************************/
// This is the 'healer' system
bool spec_cast_adept( char_data *ch )
{
    char_data *victim;
    char_data *v_next;
	int sn;

    if ( !IS_AWAKE(ch) )
	return false;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next = victim->next_in_room;
		if ( victim != ch && can_see( ch, victim ) && number_range(0, 1) == 0 
			&& !IS_NPC(victim) && victim->level < 21)
			break;
	}

	if ( victim == NULL )
		return false;

	switch ( number_range(0,12) )
	{
	case 0:
		sn=gsn_armor;
		if ( !is_affected( victim, sn ))
		{
			act( "$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM );
			spell_armor( sn, ch->level,ch,victim,TARGET_CHAR);
		}
		return true;

	case 1:
		sn=gsn_bless;
		if ( !is_affected( victim, sn ))
		{
			act( "$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM );
			spell_bless( sn, ch->level,ch,victim,TARGET_CHAR);
		}
		return true;

	case 2:
		sn=gsn_cure_blindness;
		if ( IS_AFFECTED(victim, AFF_BLIND))
		{
			act("$n utters the words 'judicandus noselacri'.",ch,NULL,NULL,TO_ROOM);
			spell_cure_blindness( sn, ch->level, ch, victim,TARGET_CHAR);
		}
		return true;

	case 3:
		sn=gsn_cure_light;
		if ( victim->hit != victim->max_hit )
		{
			act("$n utters the words 'judicandus dies'.", ch,NULL, NULL, TO_ROOM );
			spell_cure_light( sn, ch->level, ch, victim,TARGET_CHAR);
		}
		return true;

	case 4:
		sn=gsn_cure_poison;
		if ( is_affected( victim, gsn_poison ))
		{
			act( "$n utters the words 'judicandus sausabru'.",ch,NULL,NULL,TO_ROOM);
			spell_cure_poison( sn, ch->level, ch, victim,TARGET_CHAR);
		}
		return true;

	case 5:
		sn=gsn_refresh;
		if ((victim->max_move != victim->move))
		{
			act("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM );
			spell_refresh( sn,ch->level,ch,victim,TARGET_CHAR);
		}
		return true;

	case 6:
		sn=gsn_cure_disease;
		if ( is_affected( victim, gsn_plague ))
		{
			act("$n utters the words 'judicandus eugzagz'.",ch,NULL,NULL,TO_ROOM);
			spell_cure_disease( sn,	ch->level,ch,victim,TARGET_CHAR);
		}
		return true;
	}
    return false;
}

/***************************************************************************/
bool spec_cast_cleric( char_data *ch )
{
	char_data *victim;
	char_data *v_next;
	char *spell;
	int sn;

    if ( ch->position != POS_FIGHTING 
		|| ch->wait>0 
		|| IS_AFFECTED(ch,AFF2_MUTE)
		|| IS_AFFECTED( ch, AFF_BLIND ) // because they can't see the target, most are target spells
		|| IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) ){
		return false;
	}
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim->fighting == ch && number_range(0, 3) == 0 )
			break;
    }

	if ( victim == NULL )
		return false;

    for ( ; ; )
    {
		int min_level;

		switch ( number_range(0, 15) )
		{
			case  0: min_level =  0; spell = "blindness";      break;
			case  1: min_level =  3; spell = "cause serious";  break;
			case  2: min_level =  7; spell = "earthquake";     break;
			case  3: min_level =  9; spell = "cause critical"; break;
			case  4: min_level = 10; spell = "dispel evil";    break;
			case  5: min_level = 12; spell = "curse";          break;
			case  6: min_level = 12; spell = "change sex";     break;
			case  7: min_level = 13; spell = "flamestrike";    break;
			case  8: min_level = 14; spell = "cause headache"; break; //Y: mobs cause headache too!!
			case  9:
			case 10: min_level = 15; spell = "harm";           break;
			case 11: min_level = 15; spell = "plague";	   break;
			default: min_level = 16; spell = "dispel magic";   break;
		}

		if ( ch->level >= min_level )
		    break;
	}

	if ( ( sn = skill_lookup( spell ) ) < 0 )
		return false;

	say_spell(ch, sn, CCT_CLERIC);
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
	return true;
}

/***************************************************************************/
bool spec_cast_druid( char_data *ch )
{
	char_data *victim;
    char_data *v_next;
    char *spell;
	int sn;
	
    if ( ch->position != POS_FIGHTING 
		|| ch->wait>0 
		|| IS_AFFECTED(ch,AFF2_MUTE)
		|| IS_AFFECTED( ch, AFF_BLIND ) // because they can't see the target, most are target spells
		|| IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) ){
		return false;
	}
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim->fighting == ch && number_range(0, 3) == 0 )
			break;
    }
	
    if ( victim == NULL )
		return false;
	
    for ( ; ; )
    {
		int min_level;
		
		switch ( number_range(0, 15 ) )
		{
		case  0: min_level =  0; spell = "faerie fire";      break;
		case  1: min_level =  3; spell = "withering";        break;
		case  2: min_level =  6; spell = "vitality drain";   break;
		case  3: min_level =  9; spell = "closed eyes";      break;
		case  4: min_level = 12; spell = "snake bite";       break;
		case  5: min_level = 15; spell = "remove spells";    break;
		case  6: min_level = 18; spell = "merciless hunger"; break;
		case  7: min_level = 21; spell = "merciless thirst"; break;
		case  8: min_level = 20; spell = "black death";      break;
		case  9: min_level = 50; spell = "flamestorm";       break;
		default: min_level = 18; spell = "destroy living";   break;
		}
		
		if ( ch->level >= min_level )
			break;
    }
	
    if ( ( sn = skill_lookup( spell ) ) < 0 )
		return false;
	say_spell(ch, sn, CCT_DRUID);
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return true;
}

/***************************************************************************/
bool spec_cast_judge( char_data *ch )
{
	 char_data *victim;
    char_data *v_next;
    char *spell;
    int sn;
 
    if ( ch->position != POS_FIGHTING 
		|| ch->wait>0 
		|| IS_AFFECTED(ch,AFF2_MUTE)
		|| IS_AFFECTED( ch, AFF_BLIND ) // because they can't see the target, most are target spells
		|| IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) ){
		return false;
	}
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_range(0, 3) == 0 )
            break;
    }
 
    if ( victim == NULL )
        return false;
 
    spell = "high explosive";
    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return false;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return true;
}

/***************************************************************************/
bool spec_cast_mage( char_data *ch )
{
    char_data *victim;
    char_data *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING 
		|| ch->wait>0 
		|| IS_AFFECTED(ch,AFF2_MUTE)
		|| IS_AFFECTED( ch, AFF_BLIND ) // because they can't see the target, most are target spells
		|| IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) ){
		return false;
	}

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_range(0, 3) == 0 )
	    break;
    }

    if ( victim == NULL )
	return false;

    for ( ;; )
	{
	int min_level;

	switch ( number_range(0, 15) )
	{
	    case  0: min_level =  0; spell = "burning hands";  break;
		case  1: min_level =  3; spell = "chill touch";    break;
		case  2: min_level =  7; spell = "weaken";         break;
		case  3: min_level =  8; spell = "teleport";       break;
		case  4: min_level = 11; spell = "colour spray";   break;
		case  5: min_level = 12; spell = "change sex";     break;
		case  6: min_level = 13; spell = "energy drain";   break;
		case  7: 
		case  8:
		case  9: min_level = 15; spell = "fireball";       break;
		case 10: min_level = 20; spell = "decay";          break;
		case 11: min_level = 30; spell = "cause fear";     break;
		case 12: min_level = 50; spell = "chaotic poison"; break;
		default: min_level = 20; spell = "acid blast";     break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 ){
		return false;
	}


	say_spell(ch, sn, CCT_MAGE);
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return true;
}

/***************************************************************************/
bool spec_cast_undead( char_data *ch )
{
    char_data *victim;
    char_data *v_next;
    char *spell;
    int sn;
	
    if ( ch->position != POS_FIGHTING 
		|| ch->wait>0 
		|| IS_AFFECTED(ch,AFF2_MUTE)
		|| IS_AFFECTED( ch, AFF_BLIND ) // because they can't see the target, most are target spells
		|| IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) ){
		return false;
	}
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next = victim->next_in_room;
		if ( victim->fighting == ch && number_range(0, 3) == 0 )
			break;
    }
	
	if ( victim == NULL )
		return false;
	
    for ( ;; )
    {
		int min_level;
		
		switch ( number_range(0, 9) )
		{
		case  0: min_level =  0; spell = "curse";			break;
		case  1: min_level =  3; spell = "weaken";			break;
		case  2: min_level =  6; spell = "chill touch";		break;
		case  3: min_level =  9; spell = "blindness";		break;
		case  4: min_level = 12; spell = "poison";			break;
		case  5: min_level = 15; spell = "energy drain";	break;
		case  6: min_level = 18; spell = "harm";			break;
		case  7: min_level = 21; spell = "teleport";		break;
		case  8: min_level = 20; spell = "plague";			break;
		default: min_level = 18; spell = "harm";			break;
		}
		
		if ( ch->level >= min_level )
			break;
    }
	
    if ( ( sn = skill_lookup( spell ) ) < 0 )
		return false;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return true;
}

/***************************************************************************/
bool spec_fido( char_data *ch )
{
    OBJ_DATA *corpse;
	OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
	
    if ( !IS_AWAKE(ch) )
		return false;
	
    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
		c_next = corpse->next_content;
		if ( corpse->item_type != ITEM_CORPSE_NPC )
			continue;
		
		act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
		for ( obj = corpse->contains; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			obj_from_obj( obj );
			obj_to_room( obj, ch->in_room );
		}
		extract_obj( corpse );
		return true;
    }
	
	return false;
}

/***************************************************************************/
bool spec_guard( char_data *)
{
    return false;
}

/***************************************************************************/
bool spec_janitor( char_data *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;
	
    if ( !IS_AWAKE(ch) )
		return false;
	
    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
        trash_next = trash->next_content;
        if ( !IS_SET( trash->wear_flags, OBJWEAR_TAKE )
            || (trash->item_type == ITEM_CORPSE_PC)
            || !can_loot(ch,trash))
            continue;
		
        if ( trash->item_type == ITEM_DRINK_CON
			||   trash->item_type == ITEM_TRASH
			||   trash->cost < 10 )
        {
            act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
            obj_from_room( trash );
            obj_to_char( trash, ch );
            return true;
        }
    }
	
    return false;
}

/***************************************************************************/
bool spec_mayor( char_data *ch )
{
	static const char open_path[] =
		"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
	
    static const char close_path[] =
		"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";
	
    static const char *path;
    static int pos;
    static bool move;
	
    if ( !move )
    {
		if ( time_info.hour ==  6 )
		{
			path = open_path;
			move = true;
			pos  = 0;
		}
		
		if ( time_info.hour == 20 )
		{
			path = close_path;
			move = true;
			pos  = 0;
		}
    }
	
    if ( ch->fighting != NULL )
		return spec_cast_mage( ch );
    if ( !move || ch->position < POS_SLEEPING )
		return false;
	
    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
		move_char( ch, path[pos] - '0', false );
		break;
		
    case 'W':
		ch->position = POS_STANDING;
		act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
		break;
		
    case 'S':
		ch->position = POS_SLEEPING;
		act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
		break;
		
    case 'a':
		act( "$n says 'Hello Whore!'", ch, NULL, NULL, TO_ROOM );
		break;
		
    case 'b':
		act( "$n says 'These Humans can not clean up after themselves.'",
			ch, NULL, NULL, TO_ROOM );
		break;
		
    case 'c':
		act( "$n says 'Vandals!  I shall see them Hung!'",
			ch, NULL, NULL, TO_ROOM );
		break;
		
    case 'd':
		act( "$n says 'Out of my way!'", ch, NULL, NULL, TO_ROOM );
		break;
		
    case 'e':
		act( "$n says 'I hereby declare this wretched city open!'",
			ch, NULL, NULL, TO_ROOM );
		break;
		
    case 'E':
		act( "$n says 'I hereby declare this wretched city closed!'",
			ch, NULL, NULL, TO_ROOM );
		break;
		
    case 'O':
		do_unlock( ch, "gate" ); 
		do_open( ch, "gate" );
		break;
		
    case 'C':
		do_close( ch, "gate" );
		do_lock( ch, "gate" ); 
		break;
		
    case '.' :
		move = false;
		break;
    }
	
    pos++;
    return false;
}

/***************************************************************************/
bool spec_poison( char_data *ch )
{
    char_data *victim;
	
	victim = ch->fighting;
	
    if ( ch->position != POS_FIGHTING
		||   !victim
		||   number_percent( ) > 2 * ch->level )
		return false;
	
    act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
    act( "$n bites you!", ch, NULL, victim, TO_VICT    );
    spell_poison( gsn_poison, ch->level, ch, victim,TARGET_CHAR);
    return true;
}

/***************************************************************************/
bool spec_thief( char_data *ch )
{
    char_data *victim;
    char_data *v_next;
    long gold,silver;
	
    if ( ch->position != POS_STANDING )
		return false;
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next = victim->next_in_room;
		
		if ( IS_NPC(victim)
			||   victim->level >= LEVEL_IMMORTAL
			||   number_range(0, 31 ) != 0 
			||   !can_see(ch,victim))
			continue;
		
		if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
		{
			act( "You discover $n's hands in your wallet!",
				ch, NULL, victim, TO_VICT );
			act( "$N discovers $n's hands in $S wallet!",
				ch, NULL, victim, TO_NOTVICT );
			return true;
		}
		else
		{
			gold = victim->gold * UMIN(number_range(1,20),ch->level / 2) / 100;
			gold = UMIN(gold, ch->level * ch->level * 10 );
			ch->gold     += gold;
			victim->gold -= gold;
			silver = victim->silver * UMIN(number_range(1,20),ch->level/2)/100;
			silver = UMIN(silver,ch->level*ch->level * 25);
			ch->silver	+= silver;
			victim->silver -= silver;
			return true;
		}
    }
	
    return false;
}

/*************************************************************************
 *                                                                       *
 *  Lava monster comes out of his lair after few ticks of someone        *
 *  being on his bridge - Kalahn & Raine - Apr 97                        *
 *                                                                       *
 ************************************************************************/
bool spec_lava_monster( char_data *ch )
{
	static ROOM_INDEX_DATA *bridge[5];
	static long last_tick = 0;
	static int wake_countdown = 3 , /* countdown before mob awakens */
		mob_state = 0; /* state mob is in - 0 = sleeping in lair */
	int room_index;
	bool player_found;
	
	if(ch->in_room==NULL)
		return false;
	
	if (!bridge[0] ) /* load pointers to all the rooms required */
	{      
        bridge[0] = get_room_index(29646); /* lava monsters lair         */
        bridge[1] = get_room_index(29627); /* just outside his lair      */
        bridge[2] = get_room_index(29626); /* on northern end of bridge  */
        bridge[3] = get_room_index(29625); /* on southern end of bridge  */
        bridge[4] = get_room_index(29624); /* just south of bridge       */
	}

	for(int checkrooms=0; checkrooms<5; checkrooms++){
		if(!bridge[checkrooms]){
			return false;
		}
	}
	
	/* Every tick check for people around or on the lava bridge
	- this is used to delay the lava monster from coming out straight away
    */
	if (tick_counter > last_tick)
		
	{ /* a tick has past */
        last_tick = tick_counter;
		
        /* check all along bridge for a players */
        player_found = false;
        for (room_index=1; room_index<=4; room_index++)
        {
			if (bridge[room_index]->people != NULL)
			{
				wake_countdown--;
				player_found = true;
			}
        }
        
        /*  check to reset counter if no one on bridge*/
        if (player_found == false) wake_countdown=3;
		
		
        if ((wake_countdown<=0) && (mob_state==0)) mob_state = 1; /* awaken mob if it isn't already */
		
	}
	
	switch (mob_state)
    {             
		
	case 0: /* mob is asleep - check for ppl in mobs lair - instant wakeup  */
		
		if ((bridge[0]->people != ch) || (ch->next_in_room != NULL))
		{
			wake_countdown = 0;
			do_wake(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			do_yell(ch,"Who is this that disturbs my slumber?");
			mob_state = 2;
		}
		break;
		
	case 1: /* awaken mob */
		do_wake(ch,"");
		act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
		do_yell(ch,"Fool's are they that disturb my slumber");
		mob_state = 2;
		break;
		
	case 2: /* mob awake in lair */
		if (ch->position!=POS_FIGHTING)
		{
			do_west(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			mob_state = 3;
			
		}
		break;
		
	case 3: /* mob just outside of lair */
		if (ch->position!=POS_FIGHTING)
		{
			do_south(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			mob_state = 4;
			
		}
		break;
		
	case 4: /* mob on northern end of bridge */
		if (ch->position!=POS_FIGHTING)
		{
			do_south(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			mob_state = 5;
			
		}
		break;
		
	case 5: /* mob on southern end of bridge */
		if (ch->position!=POS_FIGHTING)
		{
			do_south(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			mob_state = 6;
			
		}
		break;
		
	case 6: /* mob just south of bridge - turn around and go back up */
		if (ch->position!=POS_FIGHTING)
		{
			do_north(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			mob_state = 7;
			
		}
		break;
		
		
	case 7: /* mob on southern end of bridge - returning */
		if (ch->position!=POS_FIGHTING)
		{
			do_north(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			mob_state = 8;
			
		}
		break;
		
	case 8: /* mob on northern end of bridge - returning */
		if (ch->position!=POS_FIGHTING)
		{
			do_north(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			mob_state = 9;
			
		}
		break;
		
	case 9: /*  mob just north of bridge, west of lair - returning */
		if (ch->position!=POS_FIGHTING)
		{
			do_east(ch,"");
			act("$n rises up menacingly above YOU!, flame ripples along its form.", ch, NULL, NULL, TO_ROOM );
			mob_state = 10; 
			
		}
		break;
		
		
	case 10: /* in lair - going to sleep */
		if (ch->position!=POS_FIGHTING)
		{
			do_sleep(ch,"");
			mob_state = 0; 
			wake_countdown=3; /* reset mob */
			act("$n reclines slowly back into the lava.", ch, NULL, NULL, TO_ROOM );
		}
		break;
		
	default:
		if (ch->position!=POS_FIGHTING)
			return dragon(ch, "fire breath");
		
   }
   return false;	
}

/***************************************************************************/
bool spec_assassin( char_data *ch )
{
    char buf[MSL];
    char_data *victim;
    char_data *v_next;
    int rnd_say;
	
    if ( ch->fighting != NULL )
		return false;
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next= victim->next_in_room;
		if (victim->clss != class_lookup("thief"))  
			break;
    }
	
    if ( victim == NULL || victim == ch || IS_IMMORTAL(victim) )
        return false;
    if ( victim->level > ch->level + 7 || IS_NPC(victim))
        return false;
	
    rnd_say = number_range (1, 10);
	
    switch (rnd_say)
    {
	case 6:
		sprintf( buf, "Time to die...");
		break;
	case 7:
		sprintf( buf, "Death is the true end..."); 
		break;
	case 8:
		sprintf( buf, "Welcome to your fate...");
		break;
	case 9:
		sprintf( buf, "Its a good day to die...");
		break;
	case 10:
		sprintf( buf, "Ever dance the dance..."); 
		break;
	default:
		sprintf( buf, "Die!!!");
		break;
    }
    do_say( ch, buf );
	
    multi_hit( ch, victim, gsn_backstab );
    multi_hit( ch, victim, gsn_backstab );
    return true;
}

/*************************************************************************
 *  spec_beggar - Created by Raine & edited by Kalahn - Apr 97           *
 ************************************************************************/
bool spec_beggar( char_data *ch )
{
    char buf[MSL];
    int rnd_say;
    static long last_tick = 0;

    if ( ch->fighting != NULL )
                return false;

    if (tick_counter > last_tick)
    {
        last_tick = tick_counter;
        rnd_say = number_range (1, 10);                
        switch (rnd_say)
        {
             case 6:
                act("$n points at the shimmering doorways.", ch, NULL, NULL, TO_ROOM );
                sprintf( buf, "Blue red green pink, i can never remember which one");
                break;
             case 7:
                act("$n chuckles insanely.", ch, NULL, NULL, TO_ROOM );
                sprintf( buf, "Dance the dance they said, and o how i danced");
                break;
             case 8:
                act("$n mulls over some idea.", ch, NULL, NULL, TO_ROOM );
                sprintf( buf, "Tis funny i could never tell blue or pink apart");
                break;
             case 9:
                act("$n rants and raves.", ch, NULL, NULL, TO_ROOM );
                sprintf( buf, "Aye pinks the one, take the pink road");
                break;
             case 10:
                act("$n smiles coyly.", ch, NULL, NULL, TO_ROOM );
                sprintf( buf, "Beware your next step, it may be your last, so go prepared"); 
                break;
             default:
                act("$n shudders as he rants and raves.", ch, NULL, NULL, TO_ROOM );
                sprintf( buf, "Tis the beast, its the one, walk quick or it will have some fun");                
                break;
        }
        do_say( ch, buf );
    }
    return true;
}

/************************************************************************
 *  spec_summoned_guardian - Created by Raine & Kalahn   - Sept97       *
 ************************************************************************/
bool spec_summoned_guardian( char_data *ch )
{
    char_data *victim;

    /* timer for guardian, timer is approx
       number of ticks guardian exists for */
    if (number_range(1,(int)PULSE_TICK/PULSE_MOBILE) == 1)
        ch->timer--;

    if ( ch->timer<1 )
    {
       act("$n shimmers and fades out of existance.", ch, NULL, NULL, TO_ROOM );
       extract_char( ch, true );
       return false;
    }


    if ( ch->fighting != NULL )
        return false;

    
    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
    {
         // don't activate on a mage, itself or an immortal
         if (victim->clss == class_lookup("mage") || victim == ch || IS_IMMORTAL(victim))
            continue;

         break; // someone is in the room to attack
    }

    if (victim == NULL)
        return false;

    /* don't attack newbies */
    if ( victim->level < 11 )
    {
        do_yell(ch,"Master, someone disturbs you!");
        return false;
    }

    /* don't attack those under 15 levels of the mob */
    if ( victim->level+15 < ch->level)
    {
        do_yell(ch,"Master, someone disturbs you!");
        return false;
    }

    do_say(ch, "The master commands it");
    do_yell(ch,"Master, someone disturbs you!");
    act("$n charges toward you on wings of stone.", ch, NULL, NULL, TO_ROOM );
    do_kill(ch, victim->name);
	
    return true;
}

/************************************************************************
 *  spec_elf_muncher - Created by Raine                  - Sept97       *
 ************************************************************************/
bool spec_elf_muncher( char_data *ch )
{
    char_data *victim;

    if ( ch->fighting != NULL )
    return false;

    
    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
    {
         /* don't activate on imm or self or NPC*/
         if (victim == ch || IS_IMMORTAL(victim) || IS_NPC(victim))
            continue;

         break; /* someone is in the room to attack */
    }

    if (victim == NULL)
        return false;

    // only activate on elves 
    if (victim->race != race_lookup("elf"))
        return false;

    /* don't attack to low a level */
    if ( victim->level < 29 )
       {
          do_yell(ch,"Filthy Elf!  Id kill you if you were not so pathetic");
          return false;
       }
    
    /* yell and smash an elf */
    do_say(ch, "Time to die Elf!");
    do_yell(ch,"Call out the guard, elf kiblits all around!");
    act("$n charges toward you, blood red eyes a flame.", ch, NULL, NULL, TO_ROOM );
    do_kill(ch, victim->name);
    return true;
}

/***************************************************************************/
bool spec_shadow_dragon( char_data *ch )
{
    char_data *victim;
    char_data *v_next;
    char *spell;
    int sn;
	
    if ( ch->position != POS_FIGHTING )
		return false;
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next = victim->next_in_room;
		if ( victim->fighting == ch && number_range(0, 3) == 0 )
			break;
    }
	
    if ( victim == NULL )
		return false;
	
    for ( ;; )
	{
		int min_level;
		
		switch ( number_range(0, 15) )
		{
		case  0: min_level =  0; spell = "shadow breath";  break;
		case  1: min_level =  3; spell = "vampiric touch"; break;
		case  2: min_level =  7; spell = "energy drain";   break;
		case  3: min_level =  8; spell = "curse";          break;
		case  4: min_level =  0; spell = "shadow breath";  break;
		case  5: min_level =  0; spell = "shadow breath";  break;
		case  6: min_level =  0; spell = "shadow breath";  break;
		case  7: min_level =  0; spell = "shadow breath";  break;
		case  8: min_level =  0; spell = "shadow breath";  break;
		case  9: min_level =  0; spell = "shadow breath";  break;
		case 10: min_level = 20; spell = "wrath";          break;
		default: min_level = 20; spell = "shadow breath";  break;
		}
		
		if ( ch->level >= min_level )
			break;
    }
	
    if ( ( sn = skill_lookup( spell ) ) < 0 )
		return false;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return true;
}

/***************************************************************************/
bool spec_steel_dragon( char_data *ch )
{
    char_data *victim;
    char_data *v_next;
    char *spell;
    int sn;
	
    if ( ch->position != POS_FIGHTING )
		return false;
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next = victim->next_in_room;
		if ( victim->fighting == ch && number_range(0, 3) == 0 )
			break;
    }
	
    if ( victim == NULL )
		return false;
	
    for ( ;; )
	{
		int min_level;
		
		switch ( number_range(0, 15) )
		{
		case  0: min_level =  0; spell = "steel breath";  break;
		case  1: min_level =  3; spell = "steel breath";  break;
		case  2: min_level =  7; spell = "steel breath";  break;
		case  3: min_level =  8; spell = "heat metal";    break;
		case  4: min_level =  3; spell = "steel breath";  break;
		case  5: min_level =  3; spell = "steel breath";  break;
		case  6: min_level =  3; spell = "steel breath";  break;
		case  7: min_level =  3; spell = "steel breath";  break;
		case  8: min_level =  3; spell = "steel breath";  break;
		case  9: min_level =  3; spell = "steel breath";  break;
		case 10: min_level = 20; spell = "heat metal";    break;
		default: min_level = 20; spell = "steel breath";  break;
		}
		
		if ( ch->level >= min_level )
			break;
    }
	
    if ( ( sn = skill_lookup( spell ) ) < 0 )
		return false;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return true;
}

/***************************************************************************/
bool spec_cast_mean( char_data* ch )
{
	char_data* victim;
	char_data* v_next;
	char *spell;
	int sn;
	
    if ( ( ch->position != POS_FIGHTING ) ||
		( ch->wait > 0 ) ||
		( IS_AFFECTED( ch, AFF_BLIND ) ) ||
		( IS_AFFECTED( ch, AFF2_MUTE ) ) ||
		( IS_AFFECTED2( ch, AFF2_TAUNT ) ) ||
		( IS_AFFECTED2( ch, AFF2_ROAR ) ) ||
		( IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) ) )
		return false;
	
	for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim->fighting == ch && number_range(0, 3) == 0 )
			break;
	}
	
	if ( victim == NULL )
		return false;
	
	for ( ;; )
	{
		int min_level;
		
		if ( IS_AFFECTED( ch, AFF_BLIND ) )
		{
			victim = ch;
			min_level = 0;
			spell = "cure blindness";
			break;
		}
		else if ( IS_AFFECTED( ch, AFF2_MUTE ) || 
			IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) )
		{
			victim = ch;
			min_level = 0;
			spell = "dispel silence";
			break;
		}
		else switch ( number_range(0, 15) )
		{
			case  0: min_level =  0; spell = "blindness";      break;
			case  1: min_level =  3; spell = "mental implosion";    break;
			case  2: min_level =  7; spell = "natures fury";         break;
			case  3: min_level =  8; spell = "dispel magic";       break;
			case  4: min_level = 11; spell = "ice storm";   break;
			case  5: min_level = 12; spell = "faerie fire";     break;
			case  6: min_level = 13; spell = "heat metal";   break;
			case  7:
			case  8: min_level = 14; spell = "mute";           break;
			case  9: min_level = 15; spell = "frostball";       break;
			case 10: min_level = 20; spell = "spirit lance";         break;
			default: min_level = 20; spell = "improved phantasm";     break;
		}
		
		if ( ch->level >= min_level )
			break;
	}
	
	if ( ( sn = skill_lookup( spell ) ) < 0 )
		return false;
	say_spell(ch, sn, CCT_MAGE);
	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
	return true;
}
/***************************************************************************/
bool spec_cast_meaner( char_data* ch )
{
	char_data* victim;
	char_data* v_next;
	char *spell;
	int sn;
	
    if ( ( ch->position != POS_FIGHTING ) ||
		( ch->wait > 0 ) ||
		( IS_AFFECTED( ch, AFF_BLIND ) ) ||
		( IS_AFFECTED( ch, AFF2_MUTE ) ) ||
		( IS_AFFECTED2( ch, AFF2_TAUNT ) ) ||
		( IS_AFFECTED2( ch, AFF2_ROAR ) ) ||
		( IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) ) )
		return false;
	
	for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim->fighting == ch && number_range(0, 3) == 0 )
			break;
	}
	
	if ( victim == NULL )
		return false;
	
	for ( ;; )
	{
		int min_level;
		
		if ( IS_AFFECTED( ch, AFF_BLIND ) )
		{
			victim = ch;
			min_level = 0;
			spell = "cure blindness";
			break;
		}
		else if ( IS_AFFECTED( ch, AFF2_MUTE ) || 
			IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ) )
		{
			victim = ch;
			min_level = 0;
			spell = "dispel silence";
			break;
		}
		else switch ( number_range(0, 15) )
		{
			case  0: min_level =  0; spell = "meteor swarm";      break;
			case  1: min_level =  3; spell = "mental implosion";    break;
			case  2: min_level =  7; spell = "gas breath";         break;
			case  3: min_level =  8; spell = "dispel magic";       break;
			case  4: min_level = 11; spell = "elemental storm";   break;
			case  5: min_level = 12; spell = "holy fire";     break;
			case  6: min_level = 13; spell = "heat metal";   break;
			case  7: min_level = 13; spell = "fire breath"; break;
			case  8: min_level = 14; spell = "lightning breath";           break;
			case  9: min_level = 15; spell = "steam breath";       break;
			case 10: min_level = 20; spell = "spirit lance";         break;
			default: min_level = 20; spell = "mind thrust";     break;
		}
		
		if ( ch->level >= min_level )
			break;
	}
	
	if ( ( sn = skill_lookup( spell ) ) < 0 )
		return false;
	say_spell(ch, sn, CCT_MAGE);
	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
	return true;
}
/***************************************************************************/
bool spec_master_thief( char_data* ch )
{
    char_data* victim;
    char_data* v_next;
    OBJ_DATA *obj;
    int percent, skill;
	
    if ( ( ch->position < POS_STANDING ) ||
		( ch->wait > 0 ) ||
		( IS_AFFECTED( ch, AFF_BLIND ) ) ||
		( IS_AFFECTED2( ch, AFF2_TAUNT ) ) ||
		( IS_AFFECTED2( ch, AFF2_ROAR ) ) )
		return false;
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next = victim->next_in_room;
		
        if ( IS_NPC(victim) ) /* Only PC's are targets */
            return false;
		
        if ( is_safe( ch, victim ) ) /* Only viable targets */
            return false;
		
        if ( victim->level >= LEVEL_IMMORTAL /* Don't mess with IMMs */
			||   number_range(0, 31) != 0 
			||   !can_see(ch,victim) )
			continue;
		
        skill = 40 + 2 * ch->level;
		percent  = number_percent();
		
        /* Some sanity checking below, NPC's should NOT be targets */
        if ( !IS_NPC( victim ) && get_skill(victim, gsn_awareness) >= 1 )
        {
            percent += victim->pcdata->learned[gsn_awareness]/2;
            if ( !IS_AWAKE( victim ) )
                do_wake(victim, "");
            check_improve(victim, gsn_awareness, true, 14);
        }
		
        percent  += (!IS_AWAKE(victim) ? -50 : !can_see(victim, ch) ? -25 : 10);
		
        if ( percent+victim->level-ch->level > skill )
        {
		/*
		* Failure.
			*/
            act( "$n tried to steal from you!\r\n", ch, NULL, victim, TO_VICT    );
            act( "$n tried to steal from $N!\r\n",  ch, NULL, victim, TO_NOTVICT );
            return false;
        }
        else
        {
            switch ( number_range(1, 2) )
            {
				int gold, silver;
				
			case 1:
				
				gold = victim->gold * number_range(1, ch->level) / 160;
				silver = victim->silver * number_range(1, ch->level) / 160;
				if ( gold <= 0 && silver <= 0 )
					return false;
				
				ch->gold       += gold;
				ch->silver     += silver;
				victim->silver -= silver;
				victim->gold   -= gold;
				return true;
				
			case 2:
				
				if ( ( obj = get_random_obj( victim ) ) == NULL
					|| !can_see_obj( ch, obj ) )
					return false;
				
				obj_from_char( obj );
				obj_to_char( obj, ch );
				return true;
			}
		}
	}
	return false;
}
/***************************************************************************/
bool spec_charmer( char_data* ch )
{
    char_data* victim;
    char_data* v_next;
    char buf[MSL];

    if ( ch->position != POS_FIGHTING )
    {
       switch ( number_range(0, 7) ) 
	 {
       case 0:
	  sprintf( buf, "all sing %s", ch->name );
          do_order( ch, buf ); // a chance to get free here
          break;
       case 1:
          break;
       case 2:
          break;
       case 3:
          do_order( ch, "all remove dagger" );
	    sprintf( buf, "all give dagger %s", ch->name );
          do_order( ch, buf );
          break;
       case 4:
          do_order( ch, "all remove sword" );
	    sprintf( buf, "all give sword %s", ch->name );
          do_order( ch, buf );
          break;
       case 5:
          do_order( ch, "all remove mace" );
	    sprintf( buf, "all give mace %s", ch->name );
          do_order( ch, buf );
          break;
       case 6:
          do_order( ch, "all drop all" );
          break;
       case 7:
	    sprintf( buf, "all cast 'cure light' %s", ch->name );
          do_order( ch, buf );
          break;
       };

       return true;
       }

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_range(0, 3) == 0 )
            break;
    }

    if ( victim == NULL )
        return false;

    if (IS_AFFECTED2(ch, AFF2_MUTE) || IS_SET( ch->in_room->affected_by, ROOMAFF_CONE_OF_SILENCE ))
	return false;

    act( "$n begins playing a new, beautiful song.", ch, NULL, NULL,TO_ROOM );
    spell_charm_person(gsn_charm_person, ch->level, ch, victim, TARGET_CHAR );
    if (IS_AFFECTED(victim, AFF_CHARM))
       stop_fighting( victim, true );

    return true;
}
/***************************************************************************/
bool spec_troll_member( char_data* ch)
{
    char_data* vch;
    char_data* victim = 0;
    int count = 0;
    char *message;

    if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == NULL 
    ||  IS_AFFECTED(ch,AFF_CHARM) || ch->fighting != NULL)
	return false;

    // find an ogre to beat up 
    for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room)
    {
	if (!IS_NPC(vch) || ch == vch)
	    continue;

	if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
	    return false;

	if (vch->pIndexData->group == GROUP_VNUM_OGRES
	&&  ch->level > vch->level - 2 && !is_safe(ch,vch))
	{
	    if (number_range(0,count) == 0)
		victim = vch;

	    count++;
	}
    }

    if (victim == NULL)
	return false;

	// say something, then raise hell 
    switch (number_range(0,6))
    {
	default:  message = NULL; 	break;
	case 0:	message = "$n yells 'I've been looking for you, punk!'";
		break;
	case 1: message = "With a scream of rage, $n attacks $N.";
		break;
	case 2: message = 
		"$n says 'What's slimy Ogre trash like you doing around here?'";
		break;
	case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
		break;
	case 4: message = "$n says 'There's no cops to save you this time!'";
		break;	
	case 5: message = "$n says 'Time to join your brother, spud.'";
		break;
	case 6: message = "$n says 'Let's rock.'";
		break;
    }

    if (message != NULL)
    	act(message,ch,NULL,victim,TO_ALL);
    multi_hit( ch, victim, TYPE_UNDEFINED );
	 return true;
}

/***************************************************************************/
bool spec_ogre_member( char_data* ch)
{
    char_data* vch;
    char_data* victim = 0;
    int count = 0;
    char *message;
 
    if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == NULL
    ||  IS_AFFECTED(ch,AFF_CHARM) || ch->fighting != NULL)
        return false;

    // find an troll to beat up 
    for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room)
	 {
        if (!IS_NPC(vch) || ch == vch)
            continue;
 
        if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
            return false;
 
        if (vch->pIndexData->group == GROUP_VNUM_TROLLS
        &&  ch->level > vch->level - 2 && !is_safe(ch,vch))
		  {
            if (number_range(0,count) == 0)
                victim = vch;
 
            count++;
        }
    }
 
    if (victim == NULL)
        return false;
 
    // say something, then raise hell
    switch (number_range(0,6))
    {
	default: message = NULL;	break;
		  case 0: message = "$n yells 'I've been looking for you, punk!'";
                break;
        case 1: message = "With a scream of rage, $n attacks $N.'";
                break;
        case 2: message =
                "$n says 'What's Troll filth like you doing around here?'";
                break;
        case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
                break;
		  case 4: message = "$n says 'There's no cops to save you this time!'";
                break;
        case 5: message = "$n says 'Time to join your brother, spud.'";
                break;
        case 6: message = "$n says 'Let's rock.'";
                break;
    }
 
    if (message != NULL)
    	act(message,ch,NULL,victim,TO_ALL);
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return true;
}
/***************************************************************************/
bool spec_questmaster (char_data* ch)
{
    if (ch->fighting != NULL) return spec_cast_mage( ch );
    return false;
}
/***************************************************************************/
bool spec_clan_transport( char_data* ch )
{
    char_data* victim;
    char_data* v_next;
	
    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
		return false;
	
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
		v_next = victim->next_in_room;
		
        if ( victim == NULL )
			return false;
        if ( IS_NPC(victim) )
            return false;
		
		act("$n eyes glow brightly for a moment.",ch,NULL,victim,TO_ALL);
		
	    ROOM_INDEX_DATA *location;

		int recall_vnum=get_recallvnum(ch, NULL);

		if ( ( location = get_room_index( recall_vnum ) ) == NULL )
		{
			ch->println("You are completely lost.");
			
			if ( ( location = get_room_index( ROOM_VNUM_OOC ) ) == NULL)
			{
				ch->printf("BUG: Cant find the main ooc room (vnum = %d)\r\n"
					"Please report this to an admin.\r\n", ROOM_VNUM_OOC);
				return false;
			}
			else
			{
				if (IS_SET(location->room_flags, ROOM_OOC))
				{
					ch->println("Taking you to the main OOC room since your normal recall doesnt exist.");
				}
				else
				{
					ch->printf("BUG: Taking you to the main ooc room (vnum = %d)\r\n"
						"This room SHOULD be an OOC room - please report this bug to an admin.\r\n", ROOM_VNUM_OOC);
				}
			}	
		}

        char_from_room(victim);
        char_to_room(victim, location);
		do_look( victim, "auto" );
        return true;
    }
    return false;
}
/***************************************************************************
 !!!!!!!!!!!!!!!!!!!!!!!OBJ SPECIAL FUNCTIONS BELOW HERE!!!!!!!!!!!!!!!!!!!!
 ***************************************************************************/
bool spec_obj_test( OBJ_DATA *, char_data *)
{
    act("SAYS HELLO!!!!",NULL,NULL,NULL,TO_ROOM);
    act("SAYS HELLO!!!!",NULL,NULL,NULL,TO_CHAR);
    return true;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

