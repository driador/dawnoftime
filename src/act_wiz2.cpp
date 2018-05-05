/**************************************************************************/
// act_wiz2.cpp - more immortal commands
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
#include "olc.h"
#include "nanny.h"
#include "ictime.h"
#include "channels.h"
#include "support.h" // letgain data etc for do_rename

#define COLUMNS		8

/* command procedures needed */
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_resetlu	);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_forcetick );
DECLARE_DO_FUN(do_quit      );
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_count     );
GAMBLE_FUN *gamble_lookup( const char *name );
void affectprofile_list( char_data *ch );
char *get_weapontype(OBJ_DATA *obj); // prototype - handler.c
void laston_save	(char_data *);			// laston.c 
void dismount     args( ( char_data *) );	// act_move.c
void send_zmudmap_to_char(char_data *ch);
PFILE_TYPE find_pfiletype(const char *name);
void set_char_magic_bits(char_data * ch);
void laston_login(char_data *ch);
void laston_update_char(char_data *ch); // laston.cpp

/*
 * Local functions.
 */
void ostat_show_to_char( char_data *ch, OBJ_DATA *obj);
/**************************************************************************/
void do_slookup( char_data *ch, char *argument )
{
    char arg[MIL];
	int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
		ch->println( "Lookup which skill or spell?" );
		return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
		for ( sn = 0; sn < MAX_SKILL; sn++ )
		{
			if ( skill_table[sn].name == NULL )
				break;
			ch->printf("Sn: %3d  Slot: %3d  Skill/spell: '%s'\r\n",
				sn, skill_table[sn].slot, skill_table[sn].name );
		}
	}
	else
	{
		if ( ( sn = skill_lookup( arg ) ) < 0 )
		{
			ch->println( "No such skill or spell." );
			return;
		}
		ch->printf("Sn: %3d  Slot: %3d  Skill/spell: '%s'\r\n",
			sn, skill_table[sn].slot, skill_table[sn].name );
    }
    return;
}

/**************************************************************************/
/* RT set replaces sset, mset, oset, and rset */
void do_set( char_data *ch, char *argument )
{
    char arg[MIL];

	argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        ch->println( "Syntax:" );
        ch->println( "  set char  <name> <field> <value>" );
        ch->println( "  set mob   <name> <field> <value>" );
        ch->println( "  set obj   <name> <field> <value>" );
        ch->println( "  set room  <room> <field> <value>" );
        ch->println( "  set skill <name> <spell or skill> <value>" );
        ch->println( "  set skillresetLU <name> <spell or skill> <value>" );
        return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
		do_mset(ch,argument);
		return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
		do_sset(ch,argument);
		return;
    }

    if (!str_prefix(arg,"skillresetLU"))
    {
		do_resetlu( ch, argument );
		return;
    }

	if (!str_prefix(arg,"object"))
    {
		do_oset(ch,argument);
		return;
    }

    if (!str_prefix(arg,"room"))
    {
		do_rset(ch,argument);
		return;
    }

    // echo syntax
    do_set(ch,"");
}

/**************************************************************************/
// resets the last used counter on players
void do_resetlu( char_data *ch, char *argument )
{
	char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    char_data *victim;
    int lower, upper;
    int sn;
    bool fAll, fRealms, fLanguages, fSpells;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        ch->println( "Syntax:" );
        ch->println( "  set skillresetLU <name> <spell or skill>" );
        ch->println( "  set skillresetLU <name> all" );  
        ch->println( "  set skillresetLU <name> realms" );
        ch->println( "  set skillresetLU <name> languages" );  
        ch->println( "  set skillresetLU <name> spells" );
        ch->println( "   (use the name of the skill, not the number)" );
        ch->println( " This command resets the last used time on the skill(s)" );
        return;
    }

    
	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println( "Not on NPC's."  );
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    fRealms = !str_cmp( arg2, "realms" );
    fLanguages = !str_cmp( arg2, "languages" );
    fSpells = !str_cmp( arg2, "spells" );

    sn   = 0;
    if ( !(fAll || fRealms || fLanguages || fSpells)
          && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
        ch->println( "No such skill, spell or set skill group exists." );
        ch->println( "Groups are: all, realms (includes spheres etc), languages and spells." );
        return;
    }

    if ( fAll )
    {
        ch->println( "Resetting lastused on all skills, realms, spheres and spells on char." );
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].name != NULL )
            victim->pcdata->last_used[sn] = 0;
        }
    }
    else if ( fRealms )
    {
        ch->println( "Resetting lastused on all realms and spheres on char." );
        lower= skill_lookup( "abjuration" );
        upper= skill_lookup( "time" );
        for ( sn = lower; sn <= upper; sn++ )
        {
            if ( skill_table[sn].name != NULL )
            victim->pcdata->last_used[sn] = 0;
        }
    }
    else if ( fLanguages )
    {
        ch->println( "Resetting lastused on all languages on char." );
        lower= skill_lookup( "human" );
        upper= skill_lookup( "kobold" );
        for ( sn = lower; sn <= upper; sn++ )
        {
            if ( skill_table[sn].name != NULL )
            victim->pcdata->last_used[sn] = 0;
        }
    }
    else if ( fSpells )
    {
        lower= skill_lookup( "acid blast" );
        upper= skill_lookup( "lightning breath" );

        ch->println( "Resetting lastused on all spells on char." );
        for ( sn = lower; sn <= upper; sn++ )
        {
            if ( skill_table[sn].name != NULL )
            victim->pcdata->last_used[sn] = 0;
        }
    }
    {
        victim->pcdata->last_used[sn] = 0;
    }

    ch->println( "Done." );
    return;
}


/**************************************************************************/
extern language_data *languages;
/**************************************************************************/
void do_sset( char_data *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    char_data *victim;
    int value, lower, upper;
    int sn;
    bool fAll, fRealms, fLanguages, fSpells;


    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        ch->println( "Syntax:" );
        ch->println( "  set skill <name> <spell or skill> <value>" );
        ch->println( "  set skill <name> all <value>" );  
        ch->println( "  set skill <name> realms <value>" );
        ch->println( "  set skill <name> languages <value>" );  
        ch->println( "  set skill <name> spells <value>" );
        ch->println( "   (use the name of the skill, not the number)" );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

    if ( IS_NPC(victim) )
    {
        ch->println( "Not on NPC's." );
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    fRealms = !str_cmp( arg2, "realms" );
    fLanguages = !str_cmp( arg2, "languages" );
    fSpells = !str_cmp( arg2, "spells" );

    sn   = 0;
    if ( !(fAll || fRealms || fLanguages || fSpells)
          && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
        ch->println( "No such skill, spell or set skill group exists." );
        ch->println( "Groups are: all, realms (includes spheres etc), languages and spells." );
        return;
    }

	if (!str_cmp("grant", arg3))
	{
		value=101;
	}
	else
	{
		/*
		 * Snarf the value.
		 */
		if ( !is_number( arg3 ) )
		{
			ch->println( "Value must be numeric." );
			return;
		}
		value = atoi( arg3 );
		if ( value < 0 || value > 100 )
		{
			ch->println( "Value range is 0 to 100." );
			ch->println( "Or use the word 'grant' to give the skill/spell for out of class reasons." );
			return;
		}
	}

    if ( fAll )
    {
        ch->println( "Setting all skills, realms, spheres and spells on char." );
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].name != NULL )
            victim->pcdata->learned[sn] = value;
        }
    }
    else if ( fRealms )
    {
        ch->println( "Setting all realms and spheres on char." );
        lower= skill_lookup( "abjuration" );
        upper= skill_lookup( "time" );
        for ( sn = lower; sn <= upper; sn++ )
        {
            if ( skill_table[sn].name != NULL )
            victim->pcdata->learned[sn] = value;
        }
    }
    else if ( fLanguages )
    {
        ch->println( "Setting all languages on char." );
		for(language_data *language=languages; language; language=language->next){			
            if(language->gsn<0 
				|| language->gsn>MAX_SKILL 
				|| IS_NULLSTR(skill_table[language->gsn].name))
			{
				continue;
			}
			if(IS_SET(language->flags, LANGFLAG_SYSTEM_LANGUAGE)){
				ch->printlnf("Skipping system language %s.", 
					skill_table[language->gsn].name);
			}else{
				ch->printlnf("%s set from %d to %d.", 
					skill_table[language->gsn].name,
					victim->pcdata->learned[language->gsn], value);
				victim->pcdata->learned[language->gsn] = value;
			}
		}
    }
    else if ( fSpells )
    {
        lower= FIRST_SPELL;
        upper= LAST_SPELL;

        ch->println( "Setting all spells on char." );
        for ( sn = lower; sn <= upper; sn++ )
        {
            if ( skill_table[sn].name != NULL )
            victim->pcdata->learned[sn] = value;
        }
    }else {
		if(victim->pcdata->learned[sn] == value){
			ch->printlnf("No change in %s skill on %s.", capitalize(skill_table[sn].name), victim->name);
		}else{
			ch->printlnf("%s skill on %s changed from %d to %d.", 
				capitalize(skill_table[sn].name),victim->name, victim->pcdata->learned[sn], value);
	        victim->pcdata->learned[sn] = value;
		}
    }

    ch->println("Done.");
    return;
}


/**************************************************************************/
void do_mset( char_data *ch, char *argument )
{
	char arg1 [MIL];
	char arg2 [MIL];
	char arg3 [MIL];
	char_data *victim;
	int value;
	
	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
	
	if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
		ch->println( "Syntax:" );
		ch->println( "  set char <name> <field> <value>" ); 
		ch->println( "  Field being one of:" );
		ch->println( "    sex class level temple" );
		ch->println( "    race group gold silver maxhp hp maxmana mana maxmove move prac" );
 		ch->println( "    train thirst hunger drunk full tired bleeding security" );   
		ch->println( "    alliance tendency rps wildness will damroll" );
		ch->println( "    mkills mdefeats recall age worshiptime title immtitle" );
		ch->println( "    autoafkafter");
		if (IS_TRUSTED(ch,MAX_LEVEL-1))
		{
			ch->println( "    dip RRXP RRPS newimmortal council sublevel subprac subtrain nopkill" );
			ch->println( "    immrole accurate_laston_times highimmortal_laston_access" );
		}
		if (IS_TRUSTED(ch,MAX_LEVEL))
		{
			ch->println( "    xp cp dip votes cannochannel" );
		}
		ch->println( "   PKILL RELATED: pkkills pkdefeats pksafe pkool pknorecall karns kcountdown" );
		ch->println( "      (to let someone quit, set pknorecall and pknoquit to 0)" );
		
        ch->println( "   ATTRIBUTES:" );
        ch->println( "    st  qu  pr  em  in  co  ag  sd  me  re" );
        ch->println( "    pst pqu ppr pem pin pco pag psd pme pre" );
		return;
	}
	
	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	if(!IS_ADMIN(ch) && get_trust(ch) <= get_trust(victim) && victim!=ch) 
	{ 
		ch->println( "Maybe that wasn't a good idea..." ); 
		victim->printlnf( "%s just tried to set '%s %s' on you.", 
			PERS(ch, victim), arg2, argument); 
		return; 
	}
	
	// clear zones for mobs
	victim->zone = NULL;
	
	
	// Snarf the value (which need not be numeric).
	value = is_number( arg3 ) ? atoi( arg3 ) : -1;
	
	// Do the setting
	if ( !str_prefix( arg2, "pkkills" ) || !str_prefix( arg2, "pkills" ) )
    {
        if ( value < 0  )
        {
            ch->println( "*bonk self* - You can't have -ve pkills!" );
            return;
        }
		ch->printf("Pkkills changed from %d to %d on %s(%s)\r\n",
			victim->pkkills, value, victim->short_descr, victim->name);
        victim->pkkills = value;
        return;
    }
	
	if ( !str_prefix( arg2, "pkdefeats" ) )
    {
        if ( value < 0  )
        {
            ch->println( "*bonk self* - You can't have -ve pkdefeats!" );
            return;
        }
		ch->printf("Pkdefeats changed from %d to %d on %s(%s)\r\n",
			victim->pkdefeats, value, victim->short_descr, victim->name);
        victim->pkdefeats = value;
        return;
    }
	
	
    if ( !str_prefix( arg2, "pksafe" ) )
    {
        if ( value < 0 )
        {
            ch->println( "*bonk self* - You can't safe for -ve hours!" );
            return;
        }
        if ( value > 100 )
        {
            ch->println( "Try a lower value." );
            return;
        }
		ch->printlnf("Pksafe change from %d to %d on %s(%s)",
			victim->pksafe, value, victim->short_descr, victim->name);
        victim->pksafe = value;
        return;
    }
		
    if ( !str_prefix( arg2, "pkool" ) )
    {
        if ( value < 0 )
        {
            ch->println( "*bonk self* - You have -ve hours!" );
            return;
        }
        if ( value > 15000 )
        {
            ch->println( "That is getting a bit high don't you think?" );
            return;
        }
		ch->printlnf("Pkools changed from %d to %d on %s(%s)",
			victim->pkool, value, victim->short_descr, victim->name);
        victim->pkool = value;
        return;
    }

    if ( !str_prefix( arg2, "pknorecall" ) )
    {
        if ( value < 0 )
        {
            ch->println( "*bonk self* - You have -ve hours!" );
            return;
        }
        if ( value > 50 )
        {
            ch->println( "That is getting a bit high don't you think? (try 50)" );
            return;
        }
		ch->printlnf("Pknorecall changed from %d to %d on %s(%s)",
			victim->pknorecall,	value, victim->short_descr, victim->name);
        victim->pknorecall = value;
        return;
    }
	
     if ( !str_prefix( arg2, "bleeding" ) )
     {
 		if ( IS_NPC(victim) )
 		{
 			ch->println( "Not on NPC's." );
 			return;
 		}
 		
 		if ( value < -1 || value > 100 )
 		{
 			ch->println( "Bleeding range is -1 to 100." );
 			return;
 		}
 		
 		ch->printf("Bleeding changed from %d to %d on %s(%s)\r\n",
 			victim->bleeding, value, victim->short_descr, victim->name);
 		victim->bleeding = value;
 		return;
     }

	if ( !str_prefix( arg2, "pknoquit" ) )
    {
        if ( value < 0 )
        {
            ch->println( "*bonk self* - You have -ve hours!" );
            return;
        }
        if ( value > 50 )
        {
            ch->println( "That is getting a bit high don't you think? (try 50)" );
            return;
        }
		ch->printlnf("Pknoquit changed from %d to %d on %s(%s)",
			victim->pknoquit, value, victim->short_descr, victim->name);
        victim->pknoquit = value;
        return;
    }
	
	if(!IS_NPC(victim)){
		if ( !str_prefix( arg2, "immtitle" ) ){
			if(get_trust(victim) >= get_trust(ch)){
				ch->println("I am sure they can manage their own immtitle.");
				return;
			}
			ch->printlnfbw("Immtitle changed from '%s' to '%s' on %s.",
				victim->pcdata->immtitle,
				victim->name,
				arg3);
			replace_string(victim->pcdata->immtitle, arg3);	
			return;
		}
		
		if ( !str_prefix( arg2, "title" ) ){
			if(get_trust(victim) >= get_trust(ch)){
				ch->println("I am sure they can manage their own title.");
				return;
			}
			ch->printlnfbw("Title changed from '%s' to '%s' on %s.",
				victim->pcdata->title,
				victim->name,
				arg3);
			replace_string(victim->pcdata->title, arg3);	
			return;
		}
	}
	
    if ( !str_prefix( arg2, "karns" ) )
    {
        if (IS_NPC(victim))
        {                  
            ch->println( "You can't set this on NPCs!" );
            return;
        }
        if ( value < 0 || value > 5 )
        {
            ch->println( "*bonk self* - Karns must be in the range of 0 to 5." );
            return;
        }
		ch->printf("Karns changed from %d to %d on %s(%s)\r\n",
			victim->pcdata->karns, value, victim->short_descr, victim->name);
        victim->pcdata->karns= value;
        return;
    }
	
    if ( !str_prefix( arg2, "kcountdown" ) )
    {
        if (IS_NPC(victim))
        {                  
            ch->println( "You can't set this on NPCs!" );
            return;
        }
        if ( value < 1 )
        {
            ch->println( "*bonk self* - You can't be safe for less than one hour." );
            return;
        }
        if ( value > 20000 )
        {
            ch->println( "Try a lower value." );
            return;
        }
		ch->printf("Karn countdown changed from %d to %d on %s(%s)\r\n",
			victim->pcdata->next_karn_countdown, value, victim->short_descr, victim->name);
        victim->pcdata->next_karn_countdown = value;
        return;
    }

	// Addition for worshipTimer
	if ( !str_prefix( arg2, "worshiptime" ))
	{
		if (IS_NPC(victim))
		{
			ch->println("You can't set this on NPCs!" );
			return;
		}
		if (value < 0 || value > 1000000)
		{
			ch->println( "Valid Range for `#`=Cworshiptime`^ is (`W0`^ - `W1000000`^)" );
			return;
		}
		ch->printlnf( "Worship Time changed from `#`Y%d`^ to `Y%d`^ on %s(`W%s`^).",
			(int)(current_time - victim->pcdata->worship_time) , value, victim->short_descr, victim->name);
		victim->pcdata->worship_time = (current_time - value);
		return;
	}

	if ( !str_prefix( arg2, "damroll" ))
	{
		if (!IS_NPC(victim))
		{                  
			ch->println( "You can't set this on players!" );
			return;
		}
		
		if ( value < -1000 || value > 5000 )
		{
			ch->println( "Between -1000 and 5000 please!" );
			return;
		}
		
		ch->printlnf("Damroll changed from %d to %d on %s(%s)",
			victim->damroll, value, victim->short_descr, victim->name);
		
		victim->damroll=value;
		ch->println( "DO NOT FORGET TO PURGE THIS MOB IF YOU DON'T INTEND FOR IT TO BE LIKE THIS PERMANENTLY!!!" );
		
	};
    
	if ( !str_prefix( arg2, "st" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_ST] = value;
        reset_char(victim);
		ch->printlnf("Perm ST set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "qu" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_QU] = value;
        reset_char(victim);
		ch->printlnf("Perm QU set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "pr" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_PR] = value;
        reset_char(victim);
		ch->printlnf("Perm PR set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "em" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_EM] = value;
        reset_char(victim);
		ch->printlnf("Perm EM set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "in" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_IN] = value;
        reset_char(victim);
		ch->printlnf("Perm IN set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "co" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_CO] = value;
        reset_char(victim);
		ch->printlnf("Perm CO set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "ag" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_AG] = value;
        reset_char(victim);
		ch->printlnf("Perm AG set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "sd" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_SD] = value;
        reset_char(victim);
		ch->printlnf("Perm SD set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "me" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_ME] = value;
        reset_char(victim);
		ch->printlnf("Perm ME set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "re" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
        victim->perm_stats[STAT_RE] = value;
        reset_char(victim);
		ch->printlnf("Perm RE set to %d on %s(%s)",
			value, victim->short_descr, victim->name);
        return;
	}
	
	if ( !str_prefix( arg2, "pst" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential ST changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_ST], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_ST] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "pqu" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential QU changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_QU], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_QU] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "ppr" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential PR changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_PR], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_PR] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "pem" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential EM changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_EM], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_EM] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "pin" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential IN changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_IN], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_IN] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "pco" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential CO changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_CO], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_CO] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "pag" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential AG changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_AG], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_AG] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "psd" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential SD changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_SD], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_SD] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "pme" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential ME changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_ME], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_ME] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "pre" ) )
	{
        if ( value < 1 || value > 101 )
        {
			ch->println( "Stat range is 1 to 101." );
			return;
        }
		ch->printlnf("Potential RE changed from %d to %d on %s(%s)",
			victim->potential_stats[STAT_RE], value, victim->short_descr, victim->name);
        victim->potential_stats[STAT_RE] = value;
        reset_char(victim);
        return;
	}
	
	if ( !str_prefix( arg2, "sex" ) )
	{
		if ( value < 0 || value > 2 )
		{
			ch->println( "Sex range is 0 to 2." );
			ch->println( "0 = It, 1 = Male, 2 = Female." );
			return;
		}
		ch->printlnf("Sex changed from %d to %d on %s.",
			victim->sex, value, PERS(victim, ch));
		ch->println( "0 = It, 1 = Male, 2 = Female." );
		victim->sex = value;
		if (!IS_NPC(victim)){
			victim->pcdata->true_sex = value;
		}
		return;
	}
	
	if ( !str_prefix( arg2, "mkills" ))
	{
		if ( IS_NPC( victim )) {
			ch->println("Not on NPC's." );
			return;
		}
		ch->printlnf("MKills changed from %d to %d on %s(%s)",
			victim->pcdata->mkills, value,
			victim->short_descr, victim->name );
		victim->pcdata->mkills = value;
		return;
	}
	if ( !str_prefix( arg2, "mdefeats" ))
	{
		if ( IS_NPC( victim )) {
			ch->println("Not on NPC's." );
			return;
		}
		ch->printlnf("MDefeats changed from %d to %d on %s(%s)",
			victim->pcdata->mdefeats, value,
			victim->short_descr, victim->name );
		victim->pcdata->mdefeats = value;
		return;
	}


	if ( !str_prefix( arg2, "autoafkafter" ) )
	{
		if(IS_NPC(victim)){
			ch->println("autoafkafter can be set on players only.");
			return;
		}

		ch->printlnf("autoafkafter changed on %s from %d to %d",
			PERS(victim, ch),
			victim->pcdata->autoafkafter,
			value);		
		victim->pcdata->autoafkafter=value;
		return;
	}

    if ( !str_prefix( arg2, "age" ))
    {
        if ( value < 0 )
        {
            ch->println( "Age must be 1 or greater (if 0 it clears their age)." );
            return;
        }
        if ( value > 200 )
        {
            ch->println( "Age must be less than 200." );
            return;
        }
        if (IS_NPC(victim))
        {                  
            ch->println( "You can't set age on NPCs!" );
            return;
        }
		if(value==0){
			victim->pcdata->birthdate=0; 
			ch->printlnf("Age cleared on %s", PERS(victim,ch));
		}else{
			victim->pcdata->birthdate=current_time-(value *ICTIME_IRLSECS_PER_YEAR);
			ch->printlnf("Age set on %s to %d.", PERS(victim,ch), value);
		}
        return;
    }

    if ( !str_prefix( arg2, "recall" ))
    {
        if ( value < 0 )
        {
            ch->println( "Recall room must be 0 or greater." );
            return;
        }

		if ( value != 0 )
		{
			if ( !get_room_index( value ))
			{
		        ch->println( "That room doesn't exist." );
				return;
	        }
		}
		ch->printlnf("Recall room changed from %d to %d on %s(%s)",
			victim->recall_room, value, victim->short_descr, victim->name);
        victim->recall_room = value;
        return;
    }
	

	if ( !str_prefix( arg2, "alliance" ) )
	{
		if ( value < -3 || value > 3 )
		{
			ch->println( "Alliance range is -3 to 3." );
			return;
		}
		ch->printlnf("Alliance changed from %d to %d on %s(%s)",
			victim->alliance , value, victim->short_descr, victim->name);
		victim->alliance = value;
		return;
	}
	
	if ( !str_prefix( arg2, "tendency" ) )
	{
		if ( value < -3 || value > 3 )
		{
			ch->println( "Tendency range is -3 to 3." );
			return;
		}
		ch->printlnf("Tendency changed from %d to %d on %s(%s)",
			victim->tendency , value, victim->short_descr, victim->name);
		victim->tendency = value;
		return;
	}
	
	if ( !str_prefix( arg2, "class" ) )
	{
		int clss;
		
		if (IS_NPC(victim))
		{
			ch->println( "Mobiles have no class. :)" );
			return;
		}
		
		clss = class_lookup(arg3);
		if ( clss == -1 )
		{
			char buf[MSL];
			
			strcpy( buf, "Possible classes are: " );
			
			for ( clss = 0; class_table[clss].name; clss++ )
			{
				if ( clss > 0 )
					strcat( buf, " " );
				strcat( buf, class_table[clss].name );
			}
			
			ch->println(buf);
			return;
		}
		
		ch->printlnf("Class changed from %s to %s on %s(%s)",
			class_table[victim->clss].name, class_table[clss].name, victim->short_descr, victim->name);
		victim->clss = clss;
		reset_char(victim);
		return;
	}
	
	if ( !str_prefix( arg2, "level" ) )
	{
		if ( !IS_NPC(victim) )
		{
			ch->println( "Not on PC's." );
			return;
		}
		
		if ( value < 0 || value > 100 )
		{
			ch->println( "Level range for mobs is 0 to 100." );
			return;
		}
		ch->printlnf("Level changed from %d to %d on %s(%s)",
			victim->level, value, victim->short_descr, victim->name);
		victim->level = value;
		return;
    }
	
	if ( !str_prefix( arg2, "wildness" ) )
	{
		if ( !IS_NPC(victim) )
		{
			ch->println( "Not on players." );
			return;
		}
		
		if ( value < 0 || value > 600 )
		{
			ch->println( "Level range for wildness on mobs is 0 to 600." );
			return;
		}
		ch->printlnf("Wildness changed from %d to %d on %s(%s)",
			victim->wildness , value, victim->short_descr, victim->name);
		victim->wildness = value;
		return;
    }
	if ( !str_prefix( arg2, "will" ) )
	{
		if ( !IS_NPC(victim) )
		{
			ch->println( "Not on players." );
			return;
		}
		
		if ( value < 0 || value > 600 )
		{
			ch->println( "Level range for will on mobs is 0 to 600." );
			return;
		}
		ch->printlnf("Will changed from %d to %d on %s(%s)",
			victim->will , value, victim->short_descr, victim->name);
		victim->will = value;
		return;
    }
	
	if ( !str_prefix( arg2, "gold" ) )
    {
		ch->printlnf("`YGold`x changed from %ld to %d on %s(%s)",
			victim->gold, value, victim->short_descr, victim->name);
		victim->gold = value;
		return;
    }
	
    if ( !str_prefix(arg2, "silver" ) )
    {
		ch->printlnf("`SSilver`x changed from %ld to %d on %s(%s)",
			victim->silver, value, victim->short_descr, victim->name);
		victim->silver = value;
		return;
    }

	if ( !str_prefix( arg2, "bank" ) )
    {
		ch->printlnf("`GBank`x changed from %ld to %d on %s(%s)",
			victim->bank, value, victim->short_descr, victim->name);
		victim->bank = value;
		return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
		if ( value < -10 || value > 30000 )
		{
			ch->println( "Hp range is -10 to 30,000 hit points." );
			return;
		}
		ch->printlnf("Hitpoints changed from %d to %d on %s(%s)",
			victim->hit, value, victim->short_descr, victim->name);
		victim->hit = value;
		return;
	}

    if ( !str_prefix( arg2, "maxhp" ) )
    {
		if ( value < -10 || value > 30000 )
		{
			ch->println( "Maxhp range is -10 to 30,000 hit points." );
			return;
		}
		ch->printlnf("Maxhitpoints changed from %d to %d on %s(%s)",
			victim->max_hit, value, victim->short_descr, victim->name);
		victim->max_hit = value;
        if (!IS_NPC(victim)){
			victim->pcdata->perm_hit = value;
		}
		return;
	}
	
	if ( !str_prefix( arg2, "mana" ) )
	{
		if ( value < 0 || value > 30000 )
		{
			ch->println( "Mana range is 0 to 30,000 mana points." );
			return;
		}
		ch->printlnf("Mana changed from %d to %d on %s(%s)",
			victim->mana , value, victim->short_descr, victim->name);
		victim->mana = value;
		return;
	}

	if ( !str_prefix( arg2, "maxmana" ) )
	{
		if ( value < 0 || value > 30000 )
		{
			ch->println( "MaxMana range is 0 to 30,000 mana points." );
			return;
		}
		ch->printlnf("MaxMana changed from %d to %d on %s(%s)",
			victim->max_mana , value, victim->short_descr, victim->name);
		victim->max_mana = value;
		if (!IS_NPC(victim))
			victim->pcdata->perm_mana = value;
		return;
	}

	if ( !str_prefix( arg2, "move" ) )
	{
		if ( value < 0 || value > 30000 )
		{
			ch->println( "Move range is 0 to 30,000 move points." );
			return;
		}
		ch->printlnf("Movement points changed from %d to %d on %s(%s) (max on players)",
			victim->move , value, victim->short_descr, victim->name);
		victim->move = value;
		return;
	}
	
	if ( !str_prefix( arg2, "maxmove" ) )
	{
		if ( value < 0 || value > 30000 )
		{
			ch->println( "MaxMove range is 0 to 30,000 move points." );
			return;
		}
		ch->printlnf("MaxMovement points changed from %d to %d on %s(%s) (max on players)",
			victim->max_move , value, victim->short_descr, victim->name);
		victim->max_move = value;
		if (!IS_NPC(victim))
			victim->pcdata->perm_move = value;
		return;
	}
	
    if ( !str_prefix( arg2, "practice" ) )
    {
		if ( value < -20 || value > 250 )
		{
			ch->println( "Practice range is -20 to 250 sessions." );
			return;
		}
		ch->printlnf("Practices changed from %d to %d on %s(%s)",
			victim->practice , value, victim->short_descr, victim->name);
		victim->practice = value;
		return;
    }
	
	if ( !str_prefix( arg2, "train" ))
	{
		if (value < -20 || value > 100 )
		{
			ch->println( "Training session range is -20 to 100 sessions." );
			return;
		}
		ch->printlnf("Trains changed from %d to %d on %s(%s)",
			victim->train, value, victim->short_descr, victim->name);
		victim->train = value;
		return;
	}
	
	if ( !str_prefix( arg2, "temple" ))
	{
		if (get_room_index(value)==NULL)
		{
			ch->println( "That room does not exist." );
			return;
		}
		ch->printlnf("Temple changed from %d to %d on %s(%s)",
			victim->temple , value, victim->short_descr, victim->name);
		victim->temple = value;
		return;
	}
	
    if ( !str_prefix( arg2, "thirst" ) )
    {
		if ( IS_NPC(victim) )
		{
			ch->println( "Not on NPC's." );
			return;
		}
		
		if ( value < -1 || value > 100 )
		{
			ch->println( "Thirst range is -1 to 100." );
			return;
		}
		
		ch->printlnf("Thirst changed from %d to %d on %s(%s)",
			victim->pcdata->condition[COND_THIRST], value, victim->short_descr, victim->name);
		victim->pcdata->condition[COND_THIRST] = value;
		return;
    }
	
    if ( !str_prefix( arg2, "drunk" ) )
    {
		if ( IS_NPC(victim) )
		{
			ch->println( "Not on NPC's." );
			return;
		}
		
		if ( value < -1 || value > 100 )
		{
			ch->println( "Drunk range is -1 to 100." );
			return;
		}
		
		ch->printlnf("Drunk changed from %d to %d on %s(%s)",
			victim->pcdata->condition[COND_DRUNK], value, victim->short_descr, victim->name);
		victim->pcdata->condition[COND_DRUNK] = value;
		return;
    }
	
    if ( !str_prefix( arg2, "full" ) )
    {
		if ( IS_NPC(victim) )
		{
			ch->println( "Not on NPC's." );
			return;
		}
		
		if ( value < -1 || value > 100 )
		{
			ch->println( "Full range is -1 to 100." );
			return;
		}
		
		ch->printlnf("Full changed from %d to %d on %s(%s)",
			victim->pcdata->condition[COND_FULL], value, victim->short_descr, victim->name);
		victim->pcdata->condition[COND_FULL] = value;
		return;
    }
	
	if ( !str_prefix( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
            ch->println( "Not on NPC's." );
            return;
        }
		
        if ( value < -1 || value > 100 )
        {
            ch->println( "Full range is -1 to 100." );
            return;
        }
		
		ch->printlnf("Hunger changed from %d to %d on %s(%s)",
			victim->pcdata->condition[COND_HUNGER], value, victim->short_descr, victim->name);
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }
	
	if ( !str_prefix( arg2, "tired" ) )
    {
        if ( IS_NPC(victim) )
        {
            ch->println( "Not on NPC's." );
            return;
        }
		
        if ( value < -1 || value > 80 )
        {
            ch->println( "Full range is -1 to 80." );
            ch->println( "-1 means never get tired, 0 = just woken up, 16 = you feel tired." );
            ch->println( "30-40 will collapse, 80 pretty much means instant sleep the next tick." );
            return;
        }
		ch->printlnf("Tired changed from %d to %d on %s(%s)",
			victim->pcdata->tired, value, victim->short_descr, victim->name);
        victim->pcdata->tired = value;
        return;
    }
	
    if ( !str_prefix( arg2, "rps" ) )
    {
        if ( IS_NPC(victim) )
        {
            ch->println( "Not on NPC's." );
            return;
        }
		
        if ( value < -10000 || value > 200000 )
        {
            ch->println( "Full range is -10000 to 200000." );
            return;
        }
		
		ch->printlnf("RPS changed from %ld to %d on %s(%s)",
			victim->pcdata->rp_points, value, victim->short_descr, victim->name);
        victim->pcdata->rp_points = value;
        return;
    }
	
	if (!str_prefix( arg2, "race" ) )
	{
		int race;
		
		race = race_lookup(arg3);
		
		if ( race == -1)
		{
			ch->printlnf( "'%s' is not a valid race.", arg3 );
			return;
		}
		
		if(!IS_NPC(victim) && !race_table[race]->pc_race())
		{
			ch->printlnf( "%s is not a valid player race.", race_table[race]->name );
			return;
		}
		
		ch->printlnf("Race changed from %s to %s on %s(%s)",
			race_table[victim->race]->name, 
			race_table[race]->name, victim->short_descr, victim->name);	
		victim->race = race;
		reset_char(victim); // recalc the changes
		return;
	}
	
	//  Council
	if ( !str_prefix( arg2, "council" ))
	{
		ch->println( "Use lastoncouncil" );
		return;
	}
	
	if ( !str_prefix( arg2, "security" ) )	// OLC security
	{
		if ( IS_NPC( victim ) )
		{
			ch->println( "Not on NPC's." );
			return;
		}
		
		if (value<0 || value>9)
		{
			ch->println( "The olc security must be between 0 and 9." );
			return;
		}
		
		
		if ( IS_TRUSTED(ch, MAX_LEVEL) || (value <= ch->pcdata->security && value >= 0) ){
			ch->printlnf("OLC security changed from %d to %d on %s(%s)",
				victim->pcdata->security, value, victim->short_descr, victim->name);
			victim->pcdata->security = value;
			REMOVE_CONFIG2(victim, CONFIG2_READ_BUILDER_LEGAL);
			// move the pfile if required
			{
				PFILE_TYPE pt=get_pfiletype(victim);
				if(victim->pcdata->pfiletype!=pt && pt!=PFILE_NONE){
					rename(pfilename(victim->name,victim->pcdata->pfiletype),
						pfilename(victim->name,pt)); // move the file
				}
				victim->pcdata->pfiletype=pt;
			}
			laston_update_char(victim);
		}else{
			if ( ch->pcdata->security != 0 ){
				ch->printlnf("Valid olc security is 0-%d.", ch->pcdata->security );
			}else{
				ch->println( "Valid olc security is 0 only." );
			}
			return;
		}
		return;
	}
	
	
	if (!str_prefix(arg2,"group"))
	{
		if (!IS_NPC(victim))
		{
			ch->println( "Only on NPCs." );
			return;
		}
		victim->group = value;
		return;
    }
	
	
	// stuff set on players only by max_level-1 and higher
	if(IS_TRUSTED(ch,MAX_LEVEL-1) && !IS_NPC(victim))
	{
		if ( !str_cmp( arg2, "dip" ) && IS_NOBLE(ch))
		{
			if ( value < 0 || value > 50 )
			{
				ch->println( "Diplomacy value must be between 0 and 50." );
				return;
			}
			
			victim->pcdata->diplomacy= value;
			ch->printlnf("Diplomacy set to %d on %s.", value, victim->name);
			return;
		}
		
		
		// sublevel related stuff
		if ( !str_cmp( arg2, "sublevel" ) )
		{
			if ( value < 0 || value > 20 )
			{
				ch->println( "Sublevel must be between 0 and 20" );
				return;
			}
			
			ch->printlnf("Sublevel changed from %d to %d on %s.", 
				victim->pcdata->sublevel, value, victim->name);
			victim->pcdata->sublevel= value;
			return;
		}
		if ( !str_cmp( arg2, "subprac" ) )
		{
			if ( value < 0 || value > 20 )
			{
				ch->println( "Subprac must be between 0 and 20" );
				return;
			}
			
			ch->printlnf("Subprac changed from %d to %d on %s.",
				victim->pcdata->sublevel_pracs, value, victim->name);
			victim->pcdata->sublevel_pracs= value;
			return;
		}
		if ( !str_cmp( arg2, "subtrain" ) )
		{
			if ( value < 0 || value > 20 )
			{
				ch->println( "Subtrain must be between 0 and 5" );
				return;
			}
			
			ch->printlnf("Subtrain changed from %d to %d on %s.", 
				victim->pcdata->sublevel_trains, value, victim->name);
			victim->pcdata->sublevel_trains= value;
			return;
		}
		
		// reduced systems to subtly slow players down
		if ( !str_cmp( arg2, "rrps" ) )
		{
			if ( value < 0 || value > 500 )
			{
				ch->println( "Reduced RPS percentage must be between 0 and 500, 0 = normal RPS." );
				return;
			}
			
			ch->printlnf("Reduced RPS percentage changed from %d to %d on %s.", 
				victim->pcdata->reduce_rps_percent, value, victim->name);
			victim->pcdata->reduce_rps_percent= value;
			return;
		}
		
		if ( !str_cmp( arg2, "rxp" ) )
		{
			if ( value < 0 || value > 500 )
			{
				ch->println( "Reduced XP percentage must be between 0 and 500, 0 = normal RPS." );
				return;
			}
			
			ch->printlnf("Reduced XP percentage changed from %d to %d on %s.", 
				victim->pcdata->reduce_xp_percent, value, victim->name);
			victim->pcdata->reduce_xp_percent= value;
			return;
		}

		if ( !str_cmp( arg2, "nopkill" ) )
		{
			if(IS_NPC(victim)){
				ch->println("nopkill can be set on players only.");
				return;
			}
			if(HAS_CONFIG2(victim, CONFIG2_NOPKILL)){
				REMOVE_CONFIG2(victim, CONFIG2_NOPKILL);
				ch->printlnf("nopkill status removed on %s", PERS(victim, ch));
			}else{
				SET_CONFIG2(victim, CONFIG2_NOPKILL);
				ch->printlnf("nopkill status set on %s", PERS(victim, ch));
			}
			return;
		}

		if ( !str_cmp( arg2, "accurate_laston_times" ) )
		{
			if(IS_NPC(victim)){
				ch->println("accurate_laston_times can be set on players only.");
				return;
			}
			if(HAS_CONFIG2(victim, CONFIG2_ACCURATE_LASTON_TIMES)){
				REMOVE_CONFIG2(victim, CONFIG2_ACCURATE_LASTON_TIMES);
				ch->printlnf("accurate_laston_times removed on %s by %s", PERS(victim, ch), PERS(ch, ch));
		        append_datetimestring_to_file( SECURE_FILE, 
					FORMATF("accurate_laston_times removed on %s by %s", PERS(victim, NULL), PERS(ch, NULL)));
			}else{
				SET_CONFIG2(victim, CONFIG2_ACCURATE_LASTON_TIMES);
				ch->printlnf("accurate_laston_times set on %s by %s", PERS(victim, ch), PERS(ch, ch));
		        append_datetimestring_to_file( SECURE_FILE, 
					FORMATF("accurate_laston_times set on %s by %s", PERS(victim, NULL), PERS(ch, NULL)));
			}
			return;
		}

		if ( !str_cmp( arg2, "highimmortal_laston_access" ) )
		{
			if(IS_NPC(victim)){
				ch->println("highimmortal_laston_access can be set on players only.");
				return;
			}
			if(HAS_CONFIG2(victim, CONFIG2_HIGHIMMORTAL_LASTON_ACCESS)){
				REMOVE_CONFIG2(victim, CONFIG2_HIGHIMMORTAL_LASTON_ACCESS);
				ch->printlnf("highimmortal_laston_access removed on %s by %s", PERS(victim, ch), PERS(ch, ch));
		        append_datetimestring_to_file( SECURE_FILE, 
					FORMATF("highimmortal_laston_access removed on %s by %s", PERS(victim, NULL), PERS(ch, NULL)));
			}else{
				SET_CONFIG2(victim, CONFIG2_HIGHIMMORTAL_LASTON_ACCESS);
				ch->printlnf("highimmortal_laston_access set on %s by %s", PERS(victim, ch), PERS(ch, ch));
		        append_datetimestring_to_file( SECURE_FILE, 
					FORMATF("highimmortal_laston_access set on %s by %s", PERS(victim, NULL), PERS(ch, NULL)));
			}
			return;
		}



		
		if ( !str_cmp( arg2, "newimmortal" ) )
		{
			int t;
			char buf[MIL];
				
			if(!IS_IMMORTAL(victim) || IS_NPC(victim)){
				ch->println("on imms only.");
				return;
			}
			
			for (t=0; t<MAX_STATS; t++){
				victim->potential_stats[t] = 100;
				victim->perm_stats[t] = 100;
			}
			victim->pcdata->security = UMAX(6,victim->pcdata->security);
			ch->printlnf("Security is now at %d.", victim->pcdata->security);
			sprintf(buf, "%s all grant", TRUE_CH(victim)->name);
			do_sset( ch, buf);
			
			// turn on their log
			{
				SET_BIT(victim->act, PLR_LOG);
				sprintf( buf, "Player log turned ON by %s", ch->name);
				append_playerlog( victim, buf);
				ch->printlnf("LOG set on %s.", victim->name);
			}
			save_char_obj( victim );
			
			reset_char(victim);
			ch->printlnf("Immortal character setup completed on %s.",  victim->name);
			return;
		}

		if ( !str_cmp( arg2, "immrole" ) )
		{
		
			if(!IS_IMMORTAL(victim) || IS_NPC(victim)){
				ch->println("roles can be set on imms only.");
				return;
			}
			if(c_str_len(arg3)>12){
				ch->println("Warning, only the first 12 visible characters in the immrole are shown in score.");					
			}

			if(IS_NULLSTR(victim->pcdata->imm_role)){
				ch->printlnf("Immrole on %s set to '%s`x'", 
					PERS(victim, ch), arg3);
			}else{
				ch->printlnf("Immrole on %s changed from '%s`x' to '%s`x'", 
					PERS(victim, ch), victim->pcdata->imm_role, arg3);
			}
			replace_string(victim->pcdata->imm_role, arg3);
			save_char_obj( victim );

			ch->printlnf("Your immrole has been set to '%s`x' by %s", 
				victim->pcdata->imm_role, PERS(ch, victim));
			return;
		}
		
		
	}
	
	// stuff set on players only by max_level or law
    if (!IS_NPC(victim) && (IS_TRUSTED(ch,MAX_LEVEL)
        || IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADLAW)))
    {
		
		if ( !str_cmp( arg2, "xp" ) )
		{
			if ( value < 1 )
			{
				ch->println( "Experience value must be greater than zero." );
				return;
			}		
			ch->printlnf("Experience points changed from %d to %d on %s.", 
				victim->exp, value, victim->name);
			victim->exp= value; 
			return;
		}
	}
	
	
	// stuff set on players only by max_level
    if (!IS_NPC(victim) && IS_TRUSTED(ch,MAX_LEVEL))
    {
		if ( !str_cmp( arg2, "cp" ) )
		{
			if ( value < 1 || value > 200 )
			{
				ch->println( "Full creation points range is 1 to 200." );
				return;
			}	
			victim->pcdata->points = value; 
			ch->printlnf("Creation points set to %d on %s.", 
				value, victim->name);
			return;
		}
		
		
		if ( !str_cmp( arg2, "votes" ) )
		{
			if ( IS_NPC(victim) )
			{
				ch->println( "Not on NPC's." );
				return;
			}
			
			if ( value < 0 || value > 50000 )
			{
				ch->println( "Votes must be between 0 and 50000." );
				return;
			}
			
			victim->pcdata->dip_points= value;
			ch->printlnf("Votes set to %d on %s.", value, victim->name);
			return;
		}
		
		if ( !str_cmp( arg2, "cannochannel" ) )
		{
			if (IS_SET(victim->comm, COMM_CANNOCHANNEL))
			{
				ch->printlnf("CANNOCHANNEL Ability removed from %s",victim->name);
				REMOVE_BIT(victim->comm,COMM_CANNOCHANNEL);
			}
			else
			{
				SET_BIT(victim->comm,COMM_CANNOCHANNEL);
				ch->printlnf("CANNOCHANNEL granted to %s",victim->name);
			}
			save_char_obj( victim );
			return;
		}
		
	}
	
	
    /*
	* Generate usage message.
	*/
    do_mset( ch, "" );
    return;
}

/**************************************************************************/
// Kalahn - October 97
void do_oextended( char_data *ch, char *argument )
{
    EXTRA_DESCR_DATA *ed, *ed_prev;
    OBJ_DATA *obj;

    char obj_arg [MIL];
    char mode    [MIL];
    char keyword [MIL];
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];

	if (IS_UNSWITCHED_MOB(ch)){
		ch->println( "Players only." );
		return;
	}	

	if ((  !IS_HERO(ch) 
		&& !IS_IMMORTAL(ch)
		&& !IS_RP_SUPPORT(ch)))
	{
		do_huh( ch, "" );
		return;
	}

    smash_tilde( argument );
    argument = one_argument( argument, obj_arg  );
    argument = one_argument( argument, mode     );
    argument = one_argument( argument, keyword  );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    /****************** HELP ******************/
    /* show the help if something isn't right */
    if ( obj_arg[0] == '\0' || mode[0] == '\0')
    {
        ch->println( "Syntax:" );
        ch->println( "  OEXTENDED <object> <mode> <keyword> [...]" );
        ch->println( "  mode: add clear copy edit remove rename tag" );
        ch->println( "    add       - adds a new keyword with empty description." );
        ch->println( "    appendkey - appends keywords to existing keywords" );
        ch->println( "    clear     - clears ALL extended description on an object!" );
        ch->println( "    copy      - copies an extended description from another object." );
        ch->println( "    edit      - puts you into the description editor for keyword." );
        ch->println( "    remove    - removes an extended description." );
        ch->println( "    rename    - renames keyword to new keyword." );
        ch->println( "  A keyword can be more than one word - use 'x y' syntax." );
        ch->println( "  (for more help on any of these type 'OEXTENDED <object> <mode>'");
        return;       
    }


    // find object we are working on
    if ( (obj = get_obj_here(ch, obj_arg)) == NULL )
    {
        ch->wraplnf("Object '%s' not found in your inventory or the room.", arg1);
        return;
    }

	if ( IS_OBJ_STAT( obj, OBJEXTRA_NO_RESTRING )) {
		ch->println( "You cannot set an extended description on that object." );
		return;
	}

	{ // log the usage of the oextended command to 2 files, a global, and a player
		char logbuf[MSL];
		sprintf (logbuf,"oextnd: %5d: %-13s - '%s'", obj->pIndexData->vnum,
			TRUE_CH(ch)->name, argument);
		// general log
		append_datetimestring_to_file(RESTRING_LOGFILE, logbuf);
		
		// personal playerlog
		{
			char buffilename[MSL];
			if (!IS_UNSWITCHED_MOB(ch))
			{
					sprintf(buffilename, RESTRING_LOGS_DIR"%.8s.rst", 
						TRUE_CH(ch)->name);
					append_logentry_to_file( ch, buffilename, logbuf);
			}
		}
	}


    /****************** ADD ******************/
    if (!str_prefix(mode,"add"))
    {
        // display help if required
        if ( keyword[0] == '\0')
        {
            ch->println( "Syntax:" );
            ch->println( "  OEXTENDED <object> ADD <keyword>" );
            ch->println( "  Add an empty keyword to an object.\r\n" );	// extre LF
			ch->println( "e.g     oextended book add page3" );
            ch->println( "or even oextended book add 'page3 three'" );
        }

        // add the new extended description to the object
        ed = new_extra_descr();
        ed->keyword         = str_dup( keyword  );
        ed->description     = str_dup( " " );
        ed->next            = obj->extra_descr;
        obj->extra_descr    = ed;

		// mark the object as restrung
		obj->restrung=true;

        ch->wraplnf("Added the keyword '%s' to '%s'.",
            keyword,
            obj->name);
        return;
    }

    /****************** COPY ******************/
    if (!str_prefix(mode,"copy"))
    {          
        OBJ_DATA *objsrc;
        char *pdesc;

        // display help if required
        if ( keyword[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' )
        {
            ch->println( "Syntax:" );
            ch->println( "  OEXTENDED <object> COPY <newkeyword> <objectsrc> <srckeyword>" );
            ch->println( "    Copies an extended description from another object" );
            ch->println( "    and names it after the new keyword.\r\n" );	// extra LF
            ch->println( "e.g oextended book copy page4 scroll writing" );
            ch->println( " This will copy from a scroll the extended description" );
            ch->println( " called writing and renaming the keyword to page4." );
        }

        // find source object of the extended
        if ( (objsrc = get_obj_here(ch, arg1)) == NULL )
        {
            ch->wraplnf("Source of Keyword Object '%s' wasn't found in your "
                "inventory or the room.", arg1);
            return;
        }

        // find the keyword on the source object - use either the 
        //       custom or vnum keyword - search for custom first
       
        pdesc = get_extra_descr( arg2, objsrc->extra_descr );
        if ( !IS_NULLSTR(pdesc))
        { // extended unique objects extended descriptions
            ch->println( "Found matching custom extended on source object - copied." );
        }
        else // vnums extended descriptions
        {        
            pdesc = get_extra_descr( arg2, objsrc->pIndexData->extra_descr );
            if ( !IS_NULLSTR(pdesc))
            {
                ch->println( "Found matching vnum extended on source object - copied." );
            }
            else
            {
                ch->wraplnf("Couldn't find any matching keywords on '%s'.", obj->name);
                return;
            }
        }

        // copy the extended description to the target object
        ed = new_extra_descr();
        ed->keyword         = str_dup( keyword );
        ed->description     = str_dup( pdesc );
        ed->next            = obj->extra_descr;
        obj->extra_descr    = ed;

		// mark the object as restrung
		obj->restrung=true;
        return;   
    }

    /****************** CLEAR ******************/
    if (!str_prefix(mode,"clear"))
    {          
        // display help if required
        if ( keyword[0] == '\0' )
        {
            ch->println( "Syntax:" );
            ch->println( "  OEXTENDED <object> CLEAR ALL");
            ch->println( "    Clears all customised extended description from an object.\r\n" );	// extra LF
			ch->println( "note: object then reverts back to the original vnum extendeds." );
        }

        // custom extended must already exist on object to be removed
        if (!obj->extra_descr)
        {
            ch->wraplnf("There are no custom extended descriptions "
                "on '%s'.", obj->name);
            ch->println( "Reminder: you can only clear when there are extendeds to clear." );
            return;
        }

        if (str_cmp("all", keyword))
        {
            ch->println( "You must type ALL as the keyword to confirm you want to clear all extendeds." );
            return;
        }
 
        // custom extended exists on this object
        for (ed=obj->extra_descr; ed != NULL; ed = ed->next )
        {
            obj->extra_descr =ed->next;
            free_extra_descr(ed); // free the memory 
        }

		// mark the object as restrung
		obj->restrung=true;
        ch->wraplnf("All custom extendeds have been cleared on '%s'.", obj->name);
        return;
    }


    /****************** EDIT ******************/
    if (!str_prefix(mode,"edit"))
    {          
        // display help if required
        if ( keyword[0] == '\0' )
        {
            ch->println( "Syntax:" );
            ch->println( "  OEXTENDED <object> EDIT <keyword>" );
            ch->println( "    Edits the text section of an extended description.\r\n" );	// extra LF
			ch->println( "e.g oextended book edit page4" );
            ch->println( " This will bring up the description editor with the contents" );
            ch->println( " Of the keyword to page4 on the object book." );
            ch->println( "Note: you can only edit an existing custom extended, use the" );
            ch->println( " copy command if you want to base your extended off another.");
        }

        // custom extended must already exist on object to be edited
        if (!obj->extra_descr)
        {
            ch->wraplnf("There are no custom extended descriptions on '%s'.", obj->name);
            ch->println( "Reminder: you can only edit an existing custom extended, use the" );
            ch->println( "copy command first if you want to base your extended off another." );
            return;
        }

        // custom extended exists on this object
        for (ed=obj->extra_descr ; ed != NULL; ed = ed->next )
        {
            if ( is_name( (char *) keyword, ed->keyword ) )
            {
                // put them into the editor
                ch->wraplnf("Keyword Match Found - Editing extended keyword "
					"'%s' on '%s'.",ed->keyword, obj->name);
                string_append(ch, &ed->description);

				// mark the object as restrung
				obj->restrung=true;
                return;
            }
        }
        ch->wraplnf("Couldn't find any matching keywords on the "
           "objects custom extendeds (object '%s').", obj->name);
        return;
    }

    /****************** REMOVE ******************/
    if (!str_prefix(mode,"remove"))
    {          
        // display help if required 
        if ( keyword[0] == '\0' )
        {
            ch->println( "Syntax:" );
            ch->println( "  OEXTENDED <object> REMOVE <keyword>" );
            ch->println( "    Removes an extended description from an object.\r\n" );	// extra LF
			ch->println( "e.g oextended book remove page4" );
            ch->println( " This will remove the extended description" );
            ch->println( " that is referenced by the keyword page4 from." );
            ch->println( " The book object." );
            ch->println( "Notes: use rename to remove part of a keyword." );
            ch->println( " You can only remove a custom extendeds from an object." );
            ch->println( " To get rid of a vnum extended - add a custom extended." );
            ch->wrapln("(because vnum extendeds aren't used when you have at "
                "least one extended description.");
        }

        // custom extended must already exist on object to be removed
        if (!obj->extra_descr)
        {
            ch->wraplnf("There are no custom extended descriptions on '%s'.", obj->name);
            ch->println( "Reminder: you can only remove an existing custom extended." );
            return;
        }

        // custom extended exists on this object
        ed_prev = NULL;
        for (ed=obj->extra_descr; ed != NULL; ed = ed->next )
        {
            if ( is_name( (char *) keyword, ed->keyword ) )
            {
                // the extended is found 
                ch->wraplnf("Keyword '%s' found on '%s' - removed.",
                    ed->keyword, obj->name);

                // remove the extended
                if (ed_prev){ // we aren't the head in the list
                    ed_prev->next = ed->next;
                }else{
                    obj->extra_descr =ed->next;
                }
                free_extra_descr(ed); // free the memory
                return;
            }
            ed_prev = ed;
        }

        ch->wraplnf("Couldn't find any matching keywords on the "
           "custom extendeds on '%s' to remove.", obj->name);
        return;
    }

    /****************** RENAME ******************/
    if (!str_prefix(mode,"rename"))
    {
        // trim the spaces to the right of the newkeyword
        while ( !IS_NULLSTR(arg1) && is_space(arg1[str_len(argument)-1]))
        {
            arg1[str_len(arg1)-1]='\0';
        }
    
        // check we have a new keyword left 
        if(arg1[0] == '\0' )
        {
            ch->println( "You must put in a replacement keyword." );
            ch->println( "use 'x y' format for multiple keys." );
            return;
        }
        
        // display help if required
        if ( keyword[0] == '\0' )
        {
            ch->println( "Syntax:" );
            ch->println( "  OEXTENDED <object> RENAME <oldkeyword> <newkeyword>" );
            ch->println( "    Renames the custom oldkeyword on an extended description" );
            ch->println( "    to the newkeyword.\r\n" );	// extra LF
            ch->println( "e.g oextended book rename page4 'page3 three'" );
            ch->println( " This will rename the extended descriptions called page4" );
            ch->println( " to the keyword 'page3 three'." );
            ch->println( " For someone to look at the keyword they must type one of the words in it." );
            ch->println( "Notes: You can only rename keywords on a custom extended description." );
        }

        // custom extended must already exist on object to be renamed
        if (!obj->extra_descr)
        {
            ch->wraplnf("There are no custom extended descriptions on '%s'.", obj->name);
            ch->println( "Reminder: you can only rename on custom extendeds." );
            return;
        }

        // custom extended exists on this object
        for (ed=obj->extra_descr; ed != NULL; ed = ed->next )
        {
            if ( is_name( (char *) keyword, ed->keyword ) )
            {
                // the extended is found
                free_string(ed->keyword);
                ed->keyword = str_dup( arg1 );
                return;
            }
        }

        ch->wraplnf("Couldn't find any matching keywords on the "
           "custom extendeds on '%s' to rename.", obj->name);
        return;
    }

    /****************** ADDKEY ******************/
    if (!str_prefix(mode,"appendkey"))
    {
        // trim the spaces to the right of the tag to add
        while ( !IS_NULLSTR(arg1) && is_space(arg1[str_len(argument)-1]))
        {
            arg1[str_len(arg1)-1]='\0';
        }
    
        // check we have a tag left
        if(arg1[0] == '\0' )
        {
            ch->println( "You must put in a keyword/keywords you wish to append." );
            return;
        }
        
        // display help if required
        if ( keyword[0] == '\0' )
        {
            ch->println( "Syntax:" );
            ch->println( "  OEXTENDED <object> APPENDKEY <currentkeyword> <keyword to add>" );
            ch->println( "    appends another keyword to the current keywords of an extended description\r\n" );		// extra LF
			ch->println( "e.g oextended book appendkey page3 three" );
            ch->println( " This will add an keyword of 'three' to the current keyword page3" );
        }

        // custom extended must already exist on object to be renamed
        if (!obj->extra_descr)
        {
            ch->wraplnf("There are no custom extended descriptions on '%s'.", obj->name);
            ch->println( "Reminder: you can only append keywords to custom extendeds." );
            return;
        }

        // custom extended exists on this object
        for (ed=obj->extra_descr; ed != NULL; ed = ed->next )
        {
            if ( is_name( (char *) keyword, ed->keyword ) )
            {
                char tbuf[MSL];
                        
                // the extended is found - add new keyword.
                sprintf (tbuf,"%s %s", ed->keyword, arg1);
                free_string(ed->keyword);
                ed->keyword = str_dup( tbuf );
                ch->wraplnf("Keyword is now '%s' on '%s'" ,tbuf, obj->name);
                ed->keyword = str_dup( tbuf );
                return;
            }
        }

        ch->wraplnf("Couldn't find any matching keywords on the "
           "custom extendeds on '%s' to append your keyword to.", obj->name);
        return;
    }

    ch->println( "Incorrect Mode... type 'oextended' for help." );
    return;
}

/**************************************************************************/
void do_string( char_data *ch, char *argument )
{
    char type [MIL];
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    char logged[MIL];
    char_data *victim;
    OBJ_DATA *obj;

	if (IS_UNSWITCHED_MOB(ch)){
		ch->println( "Players only." );
		return;
	}	

	if ((  !IS_HERO(ch) 
		&& !IS_IMMORTAL(ch)
		&& !IS_RP_SUPPORT(ch)))
	{
		do_huh( ch, "" );
		return;
	}

	strcpy(logged, argument );
    smash_tilde( argument );
    argument = one_argument( argument, type );
	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
		ch->println( "Syntax:" );
        if (IS_IMMORTAL(ch))
		{
			ch->println( "  STRING OBJ <name> <field> <string>" );
		}
		else
		{
			ch->println( "  STRING OBJ <name_of_object> <field> <string>" );
			ch->println( "  (hold the object you wish to string)" );
		}
		ch->println( "    fields: name short long" );
		ch->println( "  (use oextended for extended descriptions)\r\n" );	// extra LF

		if (!IS_IMMORTAL(ch))
			return;
		ch->println( "  STRING CHAR <name> <field> <string>" );
		ch->println( "    fields: name long desc spec\r\n" );	// extra LF
		ch->println( "  Use the SHORT command to set short descriptions" );	
		ch->println( "  Use CHARDESC for doing the long description on mobs.\r\n" ); //extra LF
		ch->println( "  See HELP STRING for examples on how to string mobs for quests." );
		return;
    }

    if (!str_prefix(type,"object"))
    {
    	// string an obj
    	
        if (IS_IMMORTAL(ch))
		{
			if ( (obj = get_obj_here(ch, arg1)) == NULL )
    		{
				ch->println(  "Nothing like that in heaven or earth." );
				return;
    		}
		}else
		{
			if ( (obj = get_obj_wear( ch, arg1)) == NULL )
    		{
				ch->println(  "You must be wearing or holding the object you wish to restring." );
				return;
    		}
		}

		if ( IS_OBJ_STAT( obj, OBJEXTRA_NO_RESTRING )) {
			ch->println(  "You cannot restring that object." );
			return;
		}

		{ // log the usage of the string command to 2 files, a global, and a player
			char logbuf[MSL];
			sprintf (logbuf,"string: %5d: %-13s - '%s'", obj->pIndexData->vnum,
				TRUE_CH(ch)->name, logged);
			// general log
			append_datetimestring_to_file(RESTRING_LOGFILE, logbuf);
			
			// personal playerlog
			{
				char buffilename[MSL];
				if (!IS_UNSWITCHED_MOB(ch))
				{
						sprintf(buffilename, RESTRING_LOGS_DIR"%.8s.rst", TRUE_CH(ch)->name);
						append_logentry_to_file( ch, buffilename, logbuf);
				}
			}
		}

    	
        if ( !str_prefix( arg2, "name" ) )
    	{
			ch->wraplnf("Name has been changed on %s [%d] from '%s' to '%s'",
				obj->short_descr, obj->pIndexData->vnum, obj->name, arg3);
            free_string( obj->name );
            obj->name = str_dup( arg3 );
			// mark the object as restrung
			obj->restrung=true;
            return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
			ch->wraplnf("Short description has been changed on %s [%d] from '%s' to '%s'",
				obj->name, obj->pIndexData->vnum, obj->short_descr, arg3);
            replace_string( obj->short_descr, arg3);
			// mark the object as restrung
			obj->restrung=true;
            return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
			ch->wraplnf("Long description has been changed on %s [%d] from '%s' to '%s'",
				obj->short_descr, obj->pIndexData->vnum, obj->description, arg3);
            replace_string( obj->description, arg3);
			// mark the object as restrung
			obj->restrung=true;
            return;
    	}

		if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
		{
			EXTRA_DESCR_DATA *ed;
			
			argument = one_argument( argument, arg3 );
			if ( IS_NULLSTR(argument) )
			{
				ch->println(  "Syntax: string obj <object> ed <keyword> <string>" );
				return;
			}
			
			strcat(argument,"\r\n");
			
			// add the extended description to the object
			ed = new_extra_descr();
			ed->keyword         = str_dup( arg3     );
			ed->description     = str_dup( argument );
			ed->next            = obj->extra_descr;
			obj->extra_descr    = ed;
			return;
		}
    }


    // echo bad use message for those that are heros
	if (!IS_IMMORTAL(ch)){
	    do_string(ch,"");
	}

    
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {    	
		if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
			ch->println( "They aren't here." );
			return;
		}

	// clear zone for mobs 
	victim->zone = NULL;

	// string something

     	if ( !str_prefix( arg2, "name" ) )
    	{
			if ( !IS_NPC(victim) ){
				ch->println( "You can only change the name of mobs using the string command." );
				return;
			}
			replace_string( victim->name, arg3 );
			return;
		}
    	
    	if ( !str_prefix( arg2, "description" ) ){
			ch->wraplnf("Description changed from '%s' to '%s' on %s",
				victim->description, arg3, PERS(victim, ch));
    	    replace_string(victim->description, arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) ){
			ch->wraplnf("Long/default description changed from '%s' to '%s' on %s",
				victim->long_descr, arg3, PERS(victim, ch));
            replace_string( victim->long_descr, arg3 );
            return;
    	}

    	if ( !str_prefix( arg2, "spec" ) ){
			if ( !IS_NPC(victim) ){
				ch->println( "Not on PC's." );
				return;
			}
			
			SPEC_FUN *newspec=spec_lookup( arg3 );
			if ( !newspec ){
				ch->printlnf( "No such spec function '%s'." , arg3 );
				return;
			}

			victim->spec_fun = newspec;
			ch->printlnf("Spec function changed to '%s' on %s.",
				arg3,PERS(victim, ch) );
			return;
		}
	}
    
    // echo bad use message
    do_string(ch,"");
}

/**************************************************************************/
void do_oset( char_data *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    OBJ_DATA *obj;
    int value;
	
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );
	
    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
		ch->println( "Syntax:" );
		ch->println( "  set obj <object> <field> <value>" );
		ch->println( "  Field being one of:" );
		ch->println( "    value0 value1 value2 value3 value4 (v1-v4)" );
		ch->println( "    extra wear level weight cost timer" );
		ch->println( "  note: flag values can be set using ABC...XYZabcde" );
		return;
	}
	
	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
	{
		ch->println( "Nothing like that in heaven or earth." );
		return;
	}
	
    // Snarf the value (which need not be numeric).
	if(is_number(arg3) || has_space(arg3)){
		value = atoi( arg3 );
	}else{
		value = flags_read(arg3 );
	}
	
    // Set something.
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
		ch->printlnf("object value 0 changed from %d to %d", obj->value[0], value);
		obj->value[0] = value;
		return;
	}
	
    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
		ch->printlnf("object value 1 changed from %d to %d", obj->value[1], value);
		obj->value[1] = value;
		return;
    }
	
    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
		ch->printlnf("object value 2 changed from %d to %d", obj->value[2], value);
		obj->value[2] = value;
		return;
    }
	
    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
		ch->printlnf("object value 3 changed from %d to %d", obj->value[3], value);
		obj->value[3] = value;
		return;
    }
	
    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
		ch->printlnf("object value 4 changed from %d to %d", obj->value[4], value);
		obj->value[4] = value;
		return;
	}
	
    if ( !str_prefix( arg2, "extra" ) )
    {
		ch->printlnf("object extra flags changed from %d to %d", obj->extra_flags , value);
		obj->extra_flags = value;
		return;
    }
	
    if ( !str_prefix( arg2, "wear" ) )
    {
		ch->printlnf("object wear flags changed from %d to %d", obj->wear_flags , value);
		obj->wear_flags = value;
		return;
    }
	
    if ( !str_prefix( arg2, "level" ) )
    {
		ch->printlnf("object level changed from %d to %d", obj->level, value);
		obj->level = value;
		return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
		ch->printlnf("object weight changed from %d to %d", obj->weight, value);
		obj->weight = value;
		return;
	}
	
    if ( !str_prefix( arg2, "cost" ) )
    {
		ch->printlnf("object cost changed from %d to %d", obj->cost, value);
		obj->cost = value;
		return;
    }
	
    if ( !str_prefix( arg2, "timer" ) )
    {
		ch->printlnf("object timer changed from %d to %d", obj->timer, value);
		obj->timer = value;
		return;
    }
	
    // Generate usage message.
    do_oset( ch, "" );
    return;
}


/**************************************************************************/
void do_rset( char_data *ch, char *argument )
{
	char arg1 [MIL];
	char arg2 [MIL];
	char arg3 [MIL];
	ROOM_INDEX_DATA *location;
	int value;

	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
	
	if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
		ch->println( "Syntax:" );
		ch->println( "  set room <location> <field> <value>" );
		ch->println( "  Field being one of:" );
		ch->println( "    flags sector" );
		return;
	}

	if ( ( location = find_location( ch, arg1 ) ) == NULL )
	{
		ch->println( "No such location." );
		return;
	}

    if (ch->in_room != location
		&& is_room_private_to_char( location, ch ) 
		&& !IS_TRUSTED(ch,IMPLEMENTOR))
    {
		ch->println( "That room is private right now." );
		return;
	}

    /*
     * Snarf the value.
     */
	if ( !is_number( arg3 ) )
	{
		ch->println( "Value must be numeric." );
		return;
	}
	value = atoi( arg3 );
    
	/*
     * Set something.
     */
	if ( !str_prefix( arg2, "flags" ) )
	{
		location->room_flags	= value;
		return;
	}

	if ( !str_prefix( arg2, "sector" ) )
	{
		location->sector_type	= value;
		return;
	}

    /*
     * Generate usage message.
     */
	do_rset( ch, "" );
	return;
}

/**************************************************************************/
// Written by Stimpy, ported to rom2.4 by Silverhand with extras
// Searching written by Kal
void do_sockets( char_data *ch, char *argument )
{
    char_data       *vch;
    connection_data *c;
    char            buf  [ MSL ];
    char            buf2 [ MSL ];
    char            showname [ MSL ];
    char            rcounter[ MSL ];
    int             count;
    char *          st;
    char            s[100];

	char arg[MSL];
	argument = one_argument( argument, arg );
	
    count= 0;
    buf[0]= '\0';
    buf2[0]= '\0';
	
    strcat( buf2, "\r\n`=\x9b[`=\x9cNum `=\x9d""Connected_State `=\x9fLogin@"
		"   `=\xa0Idle`=\x9b] `=\xa1Player Name  `=\xa2Host `=\xa3(Sys time: " );
    strftime( s, 100, "%I:%M%p", localtime( &current_time ) );
    strcat( buf2,s);
    strcat( buf2,")\r\n");
	
    strcat( buf2,
		"`=\xa4--------------------------------------------------------------------------`c\r\n");  
    for ( c = connection_list; c; c = c->next )
    {
        char addr[MSL];

		// can't see high level imms hidden from lower imms
		if (CH(c) && !can_see_who( ch, CH(c)) ){
			continue;
		}

		// do matching if they specified an argument
		if(!IS_NULLSTR(arg)){

			// if host name matches we don't do any other matching
			if(str_infix(arg,c->remote_hostname))
			{		
				if (CH(c)){ // character connection (match on character name)
					if (!is_name( arg, CH(c)->name)){
							continue;
					}
				}else{ // at title screen (ignore) or mudftp (match on username)
					if(!IS_NULLSTR(c->username)){
						if(!is_name( arg, c->username)){
							continue;
						}
					}

				}
			}
		}
        
        count++;
		// display the hostname and ident if known
		if(IS_NULLSTR(c->ident_username)){
			if(IS_NULLSTR(c->ident_raw_result)){
				sprintf(addr, "`=\xa2%s%s%s", 
					c->protocol==PROTOCOL_IPV6?"[":"",
					c->remote_hostname,
					c->protocol==PROTOCOL_IPV6?"]":"");
			}else{
				sprintf( addr, "`=\xa6#`s@`=\xa2%s%s%s", 
					c->protocol==PROTOCOL_IPV6?"[":"",
					c->remote_hostname,
					c->protocol==PROTOCOL_IPV6?"]":""); 
			}
        }else{
            sprintf( addr, "`#`=\xa5%s`s@`=\xa2%s%s%s", 
				c->ident_username, 
				c->protocol==PROTOCOL_IPV6?"[":"",
				c->remote_hostname,
				c->protocol==PROTOCOL_IPV6?"]":""); 
		}

		sprintf(showname,"%s",( c->original ) ? c->original->name
			: ( c->character )  ? c->character->name
			: "(unknown)");
		
		// NB: You may need to edit the CON_ values
		switch( c->connected_state )
		{
		case CON_PLAYING:				st = "    PLAYING    ";		break;
		case CON_DETECT_CLIENT_SETTINGS:st = "detect settings";		break;
		case CON_GET_NAME:				st = "   Get Name    ";		break;
		case CON_CONFIRM_CREATING_NEW:  st = "conf create new";		break;
		case CON_GET_CREATION_PASSWORD:	st = "get create pass";		break;
		case CON_GET_CONNECT_PASSWORD:	st = "get connectpass";		break;			
		case CON_GET_CONNECT_PASS2CREATE:st= "get connectpas2";		break;						
		case CON_NAME_SELECT:			st = " name selector ";		break;
		case CON_GET_OLD_PASSWORD:		st = "Get Old Passwd ";		break;
		case CON_GET_NEW_PASSWORD:		st = "Get New Passwd ";		break;
		case CON_CONFIRM_NEW_PASSWORD:	st = "Confirm Passwd ";		break;
		case CON_CONFIRM_NEW_NAME:		st = "Confirm newName";		break;		
		case CON_GET_NEW_RACE:			st = "  Get New Race ";		break;
		case CON_GET_NEW_SEX:			st = "  Get New Sex  ";		break;
		case CON_GET_NEW_CLASS:			st = " Get New Class ";		break;
		case CON_GET_ALIGNMENT:   		st = " Get New Align ";		break;
		case CON_DEFAULT_CHOICE:		st = " Choosing Cust ";		break;
		case CON_GEN_GROUPS:			st = " Customization ";		break;
		case CON_PICK_WEAPON:			st = " Picking Weapon";		break;
		case CON_READ_IMOTD:			st = " Reading IMOTD "; 	break;
		case CON_BREAK_CONNECT:			st = "   LINKDEAD    ";		break;
		case CON_READ_MOTD:				st = "  Reading MOTD ";		break;
		case CON_GET_ALLIANCE:			st = " Get Alliance  ";		break;
		case CON_GET_TENDENCY:			st = " Get Tendency  ";		break;
		case CON_HOTREBOOT_RECOVER:		st = " HOTREBOOTING  ";		break;
		case CON_FTP_COMMAND:			st = " FTPCOMMAND    "; 
			sprintf(showname,"-%s-",c->username);					break;
		case CON_FTP_DATA:				st = " FTPDATA       ";
			sprintf(showname,"-%s-",c->username);					break;
		case CON_FTP_AUTH:				st = " FTPCONNECTING ";
			sprintf(showname,"-%s-",c->username);					break;
		case CON_WEB_REQUEST:			st = "  WEB REQUEST  ";		break;
		case CON_RESOLVE_IP:			st = " RESOLVING IP  ";		break;
		case CON_GET_EMAIL:				st = " GET EMAIL ADDR";		break;
		case CON_ENTER_UNLOCK_ID:		st = "ENTERING UNLOCK";		break;
		case CON_GET_AUTOMAP:			st = "  GET AUTOMAP  ";		break;			
		case CON_RECHECK_EMAIL:			st = " RECHECK EMAIL ";		break;			
		case CON_REROLL_STATS: 
			sprintf(rcounter, " Rerolling #%-3d", 
				c->character->pcdata->reroll_counter);
			st = &rcounter[0];										break;
		case CON_GET_COLOUR:			st = "  Get colour   ";		break;
		default:						st = "   !UNKNOWN!   ";		break;
		}
		
		// Format "login" value...
		
		if (c->character){
			vch = c->original ? c->original : c->character;
			strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );
		}else{
			s[0]='\0';
		}
		int idletime=(int)(current_time-c->idle_since);
		
		sprintf( buf, "`=\x9b[`=\x9c%3d %s%s `=\x9f%7s `=\xa0%4d%s`=\x9b] `=\xa1%-12s %s%s`=\xa2%c%d\r\n", 
			c->connected_socket,
			(c->connected_state==CON_PLAYING?"`=\x9d":"`=\x9e"),
			st,
			s,
			(idletime<600?idletime:idletime/60),
			(idletime<600?"s":"m"),
			showname,
			c->multiple_logins && IS_ADMIN(ch)?"`=\xa7":"`=\xa2",
			addr, 
			(c->multiple_logins && IS_ADMIN(ch)?'*':
				(CH(c) && IS_SET(CH(c)->config,CONFIG_IGNORE_MULTILOGINS))?'+':':'),
			c->remote_port );
		
		strcat( buf2, buf );

		// flush the buffer - avoid crash with too many connections
		ch->print( buf2 );
		buf2[0]='\0';
    }
	
	if(!IS_NULLSTR(arg)){
		sprintf( buf, "\r\n`x%d matching connection%s\r\n", count, count == 1 ? "" : "s" );
	}else{
		sprintf( buf, "\r\n`x%d connection%s\r\n", count, count == 1 ? "" : "s" );
	}
    strcat( buf2, buf );
    ch->print( buf2 );
    return;
}
/**************************************************************************/
// forced_interpret allows force to be used in olc and also handles 
// alias parsing etc
void forced_interpret( char_data *ch, char *argument)
{
	// handle forced mobs
	if(IS_UNSWITCHED_MOB(ch)){
		interpret(ch, argument);
		return;
	}
	
	if (ch->desc){
		if ( ch->desc->showstr_point ){ // pager
			show_string( ch->desc, argument );
		}else if ( ch->desc->pString ){ // string editor
			string_add( ch, argument );
		}else if(ch->desc->editor){ // in olc editor
			char hold_incomm[MIL];
			strcpy(hold_incomm, ch->desc->incomm);
			strcpy(ch->desc->incomm, argument);
			if(!run_olc_editor( ch->desc )){
				interpret(ch, substitute_alias(ch->desc, argument));
			}
			strcpy(ch->desc->incomm, hold_incomm);
		}else{ // standard force
			interpret(ch, substitute_alias(ch->desc, argument));
		}
	}else{
		// must be a linkless player
		interpret(ch, argument);
	}
}
/**************************************************************************/
void do_force( char_data *ch, char *argument )
{
    char arg[MIL];
    char arg2[MIL];

    argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) ){
		ch->println( "syntax: force <player|mob> <command for them to execute>" );
		ch->println( "syntax: force all <command for all to execute>" );
		ch->println( "syntax: force players <command for all players to execute>" );
		ch->println( "syntax: force immortals <command for all immortals below your level to execute>" );
		return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") ){
        ch->println( "That will NOT be done." );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
	{
		char_data *vch;
		char_data *vch_next_player;
		
		for ( vch = player_list; vch; vch = vch_next_player )
		{
			vch_next_player = vch->next_player;
			if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
			{
				if (can_see_who(vch, TRUE_CH(ch))){
					vch->printlnf("%s forces you to '%s'.", TRUE_CH(ch)->name, argument);
				}else{
					vch->printlnf("a mystery imm forces you to '%s'.", argument);
				}
				forced_interpret( vch, argument );
			}
		}
	}
    else if (!str_cmp(arg,"players"))
    {
        char_data *vch;
        char_data *vch_next_player;
 
        for ( vch = player_list; vch; vch = vch_next_player )
        {
			vch_next_player = vch->next_player;
			
			if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
				&&	vch->level < LEVEL_HERO)
			{
				if (can_see_who(vch, TRUE_CH(ch))){
					vch->printlnf("%s forces you to '%s'.", TRUE_CH(ch)->name, argument);
				}else{
					vch->printlnf("a mystery imm forces you to '%s'.", argument);
				}
                forced_interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"immortals"))
    {
        char_data *vch;
        char_data *vch_next_player;
 
        for ( vch = player_list; vch; vch = vch_next_player )
        {
            vch_next_player = vch->next_player;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
            &&   vch->level >= LEVEL_HERO)
            {
				if (can_see_who(vch, TRUE_CH(ch))){
					vch->printlnf("%s forces you to '%s'.", TRUE_CH(ch)->name, argument);
				}else{
					vch->printlnf("a mystery imm forces you to '%s'.", argument);
				}

                forced_interpret( vch, argument );
            }
        }
    }
    else
    {
		char_data *victim;
		
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			ch->println( "They aren't here." );
			return;
		}
		
		if ( victim == ch )
		{
			ch->println( "Aye aye, right away!" );
			return;
		}

		if( IS_NPC(victim) && !str_prefix(arg2,"mob")){
			ch->println( "You can't force mobs to use the mob command." );
			return;
		}
		
		if (ch->in_room != victim->in_room
			&& is_room_private_to_char( victim->in_room, ch ) 
			&& !IS_TRUSTED(ch,IMPLEMENTOR))
		{
            ch->println("That character is in a private room." );
            return;
        }
		
		if ( get_trust( victim ) >= get_trust( ch ) )
		{
			ch->println( "Do it yourself!" );
			return;
		}
		
		// questing wiznet
		if (IS_NPC(victim))
		{
			char qbuf[MSL];
			sprintf (qbuf, "`mQUEST> %s forces mob '%s' to '%s'`x",
				ch->name, victim->name, argument);
			wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
		}
		
		victim->last_force = tick_counter;
		
		if (can_see_who(victim, TRUE_CH(ch))){
			victim->printlnf("%s forces you to '%s'.", TRUE_CH(ch)->name, argument);
		}else{
			victim->printlnf("a mystery imm forces you to '%s'.", argument);
		}

		forced_interpret( victim, argument );
    }

	if (IS_SILENT(ch)){
		ch->println( "Ok - force done silently." );
	}else{
		ch->println( "Ok - force done." );
	}
    return;
}
/**************************************************************************/
// used for forcing mobs
void do_mforce( char_data *ch, char *argument )
{
    char arg[MIL];
    char arg2[MIL];

    argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) ){
		ch->println( "syntax: force <mob> <command for mob to execute>" );
		return;
    }

    one_argument(argument,arg2);
  
    {
		char_data *victim;
		
		if ( ( victim = get_char_room( ch, arg ) ) == NULL )
		{
			ch->println( "They aren't here." );
			return;
		}
		
		if ( victim == ch )
		{
			ch->println( "Aye aye, right away!" );
			return;
		}
		
		if( !IS_NPC(victim)){
			ch->println( "You can only force mobs to do commands with the mforce command." );
			return;
		}

		if( !str_prefix(arg2,"mob")){
			ch->println( "You can't force mobs to use the mob command." );
			return;
		}
		
		// questing wiznet
		if (IS_NPC(victim))
		{
			char qbuf[MSL];
			sprintf (qbuf, "`mQUEST> %s forces mob '%s' to '%s'`x",
				ch->name, victim->name, argument);
			wiznet(qbuf,ch,NULL,WIZ_QUESTING,0,LEVEL_IMMORTAL);
		}
		
		victim->last_force = tick_counter;
		
		if (can_see_who(victim, TRUE_CH(ch))){
			victim->printlnf("%s forces you to '%s'.", TRUE_CH(ch)->name, argument);
		}else{
			victim->printlnf("a mystery imm forces you to '%s'.", argument);
		}

		forced_interpret( victim, argument );
    }

	if (IS_SILENT(ch)){
		ch->println( "Ok - force done silently." );
	}else{
		ch->println( "Ok - force done." );
	}
    return;
}

/**************************************************************************/
// used for forcing newbies
void do_nforce( char_data *ch, char *argument )
{
    char arg[MIL];
    char arg2[MIL];

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) ){
		ch->println( "syntax: force <newbie> <command for newbie to execute>" );
		return;
    }

    one_argument(argument,arg2);
  
    {
		char_data *victim;
		
		if ( ( victim = get_char_room( ch, arg ) ) == NULL )
		{
			ch->println( "They aren't here." );
			return;
		}
		
		if ( victim == ch )
		{
			ch->println( "Aye aye, right away!" );
			return;
		}
		
		if( IS_NPC(victim) || !IS_NEWBIE(victim)){
			ch->println( "You can only force newbie players to do commands with the nforce command." );
			return;
		}

		victim->last_force = tick_counter;
		
		if (can_see_who(victim, TRUE_CH(ch))){
			victim->printlnf("%s forces you to '%s'.", TRUE_CH(ch)->name, argument);
		}else{
			victim->printlnf("a mystery imm forces you to '%s'.", argument);
		}

		forced_interpret( victim, argument );
    }

	if (IS_SILENT(ch)){
		ch->println( "Ok - force done silently." );
	}else{
		ch->println( "Ok - force done." );
	}
    return;
}

/**************************************************************************/
// Another piece of Kal's magic testing code :)
void do_forcetick( char_data *ch, char *argument)
{
    extern int pulse_point;
    pulse_point=1;  
    ch->println( "TICK FORCED!" );

	ch->println("Note, forcetick doesn't cause hour triggered mudprogs to fire, use setichour to test these.");
}

/**************************************************************************/
// New routines by Dionysos. Defaulting to LEVEL_IMMORTAL Kalahn
// Tweaked for FadeIN/OUT by Kerenos
void do_wizinvis( char_data *ch, char *argument )
{
    int				level;
    char			arg[MSL];
    char_data *rch;

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg)){ // take the default path 
        if ( ch->invis_level){
            ch->invis_level= 0;
			if ( !ch->pcdata || IS_NULLSTR( ch->pcdata->fadein ) ){
				act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
			}else{
				act( "$t", ch, ch->pcdata->fadein, NULL, TO_ROOM );
				for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
				{
					if (IS_IMMORTAL(rch)){
						act("($n fadein)",ch,NULL,rch,TO_VICT);              
					}
				}
			}			
            ch->println("You slowly fade back into existence.");
        }else{
            if ( !ch->pcdata || IS_NULLSTR( ch->pcdata->fadeout ) ){
	            act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
			}else{
				act( "$t", ch, ch->pcdata->fadeout, NULL, TO_ROOM );
				for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
				{
					if (IS_IMMORTAL(rch))
					{
						act("($n fadeout)",ch,NULL,rch,TO_VICT);              
					}
				}
			}

            if (get_trust(ch) > LEVEL_IMMORTAL)
                ch->invis_level= LEVEL_IMMORTAL;
            else
                ch->invis_level= get_trust(ch);


            if (ch->invis_level== LEVEL_IMMORTAL){
                ch->println("You are now invisible to mortal beings.");					
            }else{
                ch->println( "You slowly vanish into thin air." );
			}
        }
    }else{  // do the level thing
        level = atoi(arg);
        if (level < 2 || level > get_trust(ch)){
            ch->println( "Invis level must be between 2 and your level." );
            return;
        }else{
            ch->invis_level= level;
			if ( !IS_SWITCHED(ch)) {
				if ( IS_NULLSTR( ch->pcdata->fadeout )) {
		            act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
				}else{
					act( "$t", ch, ch->pcdata->fadeout, NULL, TO_ROOM );
					for (rch = ch->in_room->people; rch; rch = rch->next_in_room){
						if (IS_IMMORTAL(rch)){
							act("($n fadeout)",ch,NULL,rch,TO_VICT);              
						}
					}
				}
			}else{
				act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
			}
         
			if (ch->invis_level== LEVEL_IMMORTAL){
				ch->println( "You are now invisible to mortal beings." );
            }else{
                ch->println( "You slowly vanish into thin air." );
			}
        }
    }

	ch->printlnf("True invisiblity level is now %d",	INVIS_LEVEL(ch));
    return;
}

/**************************************************************************/
void do_iwizi( char_data *ch, char *argument )
{
    int				level;
    char_data *rch;

    if ( IS_NULLSTR(argument)) 
    // take the default path 

        if ( ch->iwizi)
        {	// iwizi being turned off
            ch->iwizi= 0;
			// show message to room if they just became room visible
			if(!INVIS_LEVEL(ch) && !IS_OOC(ch)){
				if ( !ch->pcdata || IS_NULLSTR( ch->pcdata->fadein ) )
					act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
				else
				{
					act( "$t", ch, ch->pcdata->fadein, NULL, TO_ROOM );
					for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					{
						if (IS_IMMORTAL(rch))
						{
							act("($n iwizi fadein)",ch,NULL,rch,TO_VICT);              
						}
					}
				}			
			}
            ch->printlnf("Iwizi set to 0.");
        } 
        else // iwizi being turned on to LEVEL_IMMORTAL
        {
			// show message to room if they just became room visible
			if(!INVIS_LEVEL(ch) && !IS_OOC(ch)){
				if ( !ch->pcdata || IS_NULLSTR( ch->pcdata->fadeout ) ){
					act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
				}else{
					act( "$t", ch, ch->pcdata->fadeout, NULL, TO_ROOM );
					for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					{
						if (IS_IMMORTAL(rch))
						{
							act("($n iwizi fadeout)",ch,NULL,rch,TO_VICT);              
						}
					}
				}
			}

            if (get_trust(ch) > LEVEL_IMMORTAL)
                ch->iwizi= LEVEL_IMMORTAL;
            else
                ch->iwizi= get_trust(ch);

			ch->printlnf( "Iwizi set to %d.", ch->iwizi);
        }
    else
    // do the level thing
    {
        level = atoi(argument);
        if (level < 2 || level > get_trust(ch))
        {
            ch->println( "Iwizi level must be between 2 and your level." );
            return;
        }
        else
        {
            ch->iwizi = level;
			if(!IS_OOC(ch))
			{
				if ( !IS_SWITCHED(ch)) {
					if ( IS_NULLSTR( ch->pcdata->fadeout )) {
						act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
					} else {
						act( "$t", ch, ch->pcdata->fadeout, NULL, TO_ROOM );
						for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
						{
							if (IS_IMMORTAL(rch))
							{
								act("($n iwizi fadeout)",ch,NULL,rch,TO_VICT);              
							}
						}
					}
				}else{
					act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
				}
			}         
			ch->printlnf("Iwizi set to %d.", ch->iwizi);
        }
    }
    return;
}
/**************************************************************************/
void do_olcwizi( char_data *ch, char *argument )
{
    int	level;
    char_data *rch;

    if ( IS_NULLSTR(argument)){
    // take the default path 
        if ( ch->olcwizi){	// olcwizi being turned off
            ch->olcwizi= 0;
			// show message to room if they just became room visible
			if(!INVIS_LEVEL(ch) && IS_OLC(ch)){
				if ( !ch->pcdata || IS_NULLSTR( ch->pcdata->fadein ) )
					act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
				else
				{
					act( "$t", ch, ch->pcdata->fadein, NULL, TO_ROOM );
					for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					{
						if (IS_IMMORTAL(rch))
						{
							act("($n olcwizi fadein)",ch,NULL,rch,TO_VICT);              
						}
					}
				}			
			}
            ch->printlnf("Olcwizi set to 0.");
		}else{ // olcwizi being turned on to LEVEL_IMMORTAL
			// show message to room if they just became room visible
			if(!INVIS_LEVEL(ch) && IS_OLC(ch)){
				if ( !ch->pcdata || IS_NULLSTR( ch->pcdata->fadeout ) ){
					act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
				}else{
					act( "$t", ch, ch->pcdata->fadeout, NULL, TO_ROOM );
					for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					{
						if (IS_IMMORTAL(rch))
						{
							act("($n olcwizi fadeout)",ch,NULL,rch,TO_VICT);              
						}
					}
				}
			}

            if (get_trust(ch) > LEVEL_IMMORTAL)
                ch->olcwizi= LEVEL_IMMORTAL;
            else
                ch->olcwizi= get_trust(ch);

			ch->printlnf( "Olcwizi set to %d.", ch->olcwizi);
        }
    }else{ // do the level thing
        level = atoi(argument);
        if (level < 2 || level > get_trust(ch))
        {
            ch->println( "Olcwizi level must be between 2 and your level." );
            return;
        }else{
            ch->olcwizi = level;
			if(IS_OLC(ch))
			{
				if ( !IS_SWITCHED(ch)) {
					if ( IS_NULLSTR( ch->pcdata->fadeout )) {
						act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
					} else {
						act( "$t", ch, ch->pcdata->fadeout, NULL, TO_ROOM );
						for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
						{
							if (IS_IMMORTAL(rch))
							{
								act("($n olcwizi fadeout)",ch,NULL,rch,TO_VICT);              
							}
						}
					}
				}else{
					act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
				}
			}         
			ch->printlnf("Olcwizi set to %d.", ch->olcwizi);
        }
    }
    return;
}
/**************************************************************************/
void do_owizi( char_data *ch, char *argument )
{
    int				level;
    char_data *rch;

    if ( IS_NULLSTR(argument)) 
    // take the default path 

        if ( ch->owizi)
        {	// owizi being turned off
            ch->owizi= 0;
			// show message to room if they just became room visible
			if(!INVIS_LEVEL(ch) && IS_OOC(ch)){
				if ( !ch->pcdata || IS_NULLSTR( ch->pcdata->fadein ) )
					act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
				else
				{
					act( "$t", ch, ch->pcdata->fadein, NULL, TO_ROOM );
					for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					{
						if (IS_IMMORTAL(rch))
						{
							act("($n owizi fadein)",ch,NULL,rch,TO_VICT);              
						}
					}
				}			
			}
            ch->printlnf("Owizi set to 0.");
        } 
        else // owizi being turned on to LEVEL_IMMORTAL
        {
			// show message to room if they just became room visible
			if(!INVIS_LEVEL(ch) && IS_OOC(ch)){
				if ( !ch->pcdata || IS_NULLSTR( ch->pcdata->fadeout ) ){
					act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
				}else{
					act( "$t", ch, ch->pcdata->fadeout, NULL, TO_ROOM );
					for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					{
						if (IS_IMMORTAL(rch))
						{
							act("($n owizi fadeout)",ch,NULL,rch,TO_VICT);              
						}
					}
				}
			}

            if (get_trust(ch) > LEVEL_IMMORTAL)
                ch->owizi= LEVEL_IMMORTAL;
            else
                ch->owizi= get_trust(ch);

            ch->printlnf("Owizi set to %d.", ch->owizi);
        }
    else
    // do the level thing
    {
        level = atoi(argument);
        if (level < 2 || level > get_trust(ch))
        {
            ch->println( "Owizi level must be between 2 and your level." );
            return;
        }
        else
        {
            ch->owizi = level;
			if(!IS_OOC(ch))
			{
				if ( !IS_SWITCHED(ch)) {
					if ( IS_NULLSTR( ch->pcdata->fadeout )) {
						act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
					} else {
						act( "$t", ch, ch->pcdata->fadeout, NULL, TO_ROOM );
						for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
						{
							if (IS_IMMORTAL(rch))
							{
								act("($n owizi fadeout)",ch,NULL,rch,TO_VICT);              
							}
						}
					}
				}else{
					act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
				}
			}         
			ch->printlnf("Owizi set to %d.", ch->owizi);
        }
    }
    return;
}
/**************************************************************************/
void do_incognito( char_data *ch, char *argument )
{
    int level;
    char arg[MSL];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
        if ( ch->incog_level)
        {
            ch->incog_level = 0;
            act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
            ch->println( "You are no longer cloaked." );
        }
        else
        {
            if (get_trust(ch) > LEVEL_IMMORTAL)
                ch->incog_level = LEVEL_IMMORTAL;
            else
                ch->incog_level = get_trust(ch);
            act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );

            if (ch->incog_level == LEVEL_IMMORTAL) 
                ch->println( "You cloak your presence from mere mortals." );
            else
                ch->println( "You cloak your presence." );
        }
    else
    /* do the level thing */
    {
        level = atoi(arg);
        if (level < 2 || level > get_trust(ch))
        {
            ch->println( "Incog level must be between 2 and your level." );
            return;
        }
        else
        {
            ch->reply = NULL;
            ch->incog_level = level;
            act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
            if (ch->incog_level == LEVEL_IMMORTAL) 
                ch->println( "You cloak your presence from mere mortals." );
            else
                ch->println( "You cloak your presence." );
        }
    }
    return;
}

/**************************************************************************/
void do_holylight( char_data *ch, char *)
{
    if ( IS_UNSWITCHED_MOB(ch) )
	return;

    if ( IS_SET(TRUE_CH(ch)->act, PLR_HOLYLIGHT) )
    {
        REMOVE_BIT(TRUE_CH(ch)->act, PLR_HOLYLIGHT);
        ch->println( "Holy light mode off." );
    }
    else
    {
        SET_BIT(TRUE_CH(ch)->act, PLR_HOLYLIGHT);
        ch->println( "Holy light mode on." );
    }
    return;
}

/**************************************************************************/
void do_holyspeech( char_data *ch, char *)
{
    if(IS_UNSWITCHED_MOB(ch)){
	    return;
	}

    if ( HAS_HOLYSPEECH(ch) )
    {
		REMOVE_BIT(ch->config, CONFIG_HOLYSPEECH);
        ch->println( "Holy speech mode off." );
    }
    else
    {
		SET_BIT(ch->config, CONFIG_HOLYSPEECH);
        ch->println( "Holy speech mode on." );
    }

    return;
}

/**************************************************************************/
void do_holyname( char_data *ch, char *)
{
    if(IS_UNSWITCHED_MOB(ch)) 
    return;

    if ( IS_SET(TRUE_CH(ch)->act, PLR_HOLYNAME) )
    {
        REMOVE_BIT(TRUE_CH(ch)->act, PLR_HOLYNAME);
        ch->println( "Holy name mode off." );
    }
    else
    {
        SET_BIT(TRUE_CH(ch)->act, PLR_HOLYNAME);
        ch->println( "Holy name mode on." );
    }
    return;
}

/**************************************************************************/
void do_holywalk( char_data *ch, char *)
{
    if(IS_UNSWITCHED_MOB(ch)) 
    return;

    if ( IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK) )
    {
        REMOVE_BIT(TRUE_CH(ch)->act, PLR_HOLYWALK);
        ch->println( "Holy walk off." );
    }
    else
    {
        SET_BIT(TRUE_CH(ch)->act, PLR_HOLYWALK);
        ch->println( "Holy walk on." );
    }
    return;
}

/**************************************************************************/
void do_holyvnum( char_data *ch, char *)
{
    if(IS_UNSWITCHED_MOB(ch)) 
    return;

    if ( IS_SET(TRUE_CH(ch)->act, PLR_HOLYVNUM) )
    {
        REMOVE_BIT(TRUE_CH(ch)->act, PLR_HOLYVNUM);
        ch->println( "Holyvnum off." );
    }
    else
    {
        SET_BIT(TRUE_CH(ch)->act, PLR_HOLYVNUM);
        ch->println( "Holyvnum on." );
    }
    return;
}

/**************************************************************************/
void do_seevnum( char_data *ch, char *argument )
{
    if(IS_UNSWITCHED_MOB(ch)) 
	{
		ch->println( "Immortal players only." );	
		return;
	}

	if (IS_NULLSTR(argument))
	{
		ch->println( "`xType a command as the parameter..." );
		ch->println( "e.g. `=Cseevnum inv`x" );	
	}

    if ( !IS_SET(TRUE_CH(ch)->act, PLR_HOLYVNUM) )
    {
        SET_BIT(TRUE_CH(ch)->act, PLR_HOLYVNUM);
        ch->println("SEEVNUM BEGINS:");
        interpret( ch, argument );
        ch->println("SEEVNUM FINISHED.");
        REMOVE_BIT(TRUE_CH(ch)->act, PLR_HOLYVNUM);
    }
    else
    {
        ch->println("SEEVNUM BEGINS");
        interpret( ch, argument );
        ch->println("SEEVNUM FINISHED");
    }
    return;
}
/**************************************************************************/
// Kal - Jan 00 :)
void do_inroom( char_data *ch, char *argument )
{
	if(IS_NULLSTR(argument)){
		ch->print("Syntax: inroom <command>\r\n"
			"Notes: inroom removes you ability see objects and mobs outside the room\r\n"
			"       you are currently in, this is useful to specifically find an object\r\n"
			"       with stat, or set, and stat into a mob in the current room etc.\r\n");
		return;
	}

	SET_BIT(ch->dyn,DYN_IN_ROOM_ONLY);
	ch->println("INROOM BEGINS");
	// do the command
	interpret( ch, argument );
	ch->println("INROOM ENDS");
	REMOVE_BIT(ch->dyn,DYN_IN_ROOM_ONLY);

}
/**************************************************************************/
void do_silently( char_data *ch, char *argument )
{
	int old_invis_level;
	int whovis;
	
	whovis = IS_SET(TRUE_CH(ch)->comm, COMM_WHOVIS);
    REMOVE_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);

	SET_BIT(ch->dyn,DYN_SILENTLY);
	ch->println("SILENTLY BEGINS");
	
	// back up the wizi level
	old_invis_level= ch->invis_level;
	// hide them to mortals
	ch->invis_level= UMAX(ch->invis_level, LEVEL_IMMORTAL);
	// do the command
	interpret( ch, argument );

	// restore their wizi
	ch->invis_level=old_invis_level;

	ch->println("SILENTLY FINISHED");
	REMOVE_BIT(ch->dyn,DYN_SILENTLY);

	if (whovis){
		SET_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);
	}

    return;
}

/**************************************************************************/
void do_whovis( char_data *ch, char *)
{
    if(IS_UNSWITCHED_MOB(ch)) 
    return;

    if ( IS_SET(TRUE_CH(ch)->comm, COMM_WHOVIS) )
    {
		ch->wrapln( "You are already visible on the wholist... use whoinvis to disappear.");
    }
    else
    {
		SET_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);
		ch->println( "You are now visible on the who list to everyone no matter what." );
    }
    return;
}
/**************************************************************************/
void do_whoinvis( char_data *ch, char *)
{
    if(IS_UNSWITCHED_MOB(ch)) 
    return;

    if ( IS_SET(TRUE_CH(ch)->comm, COMM_WHOVIS) )
    {
		REMOVE_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);
		ch->wrapln( "You will not appear on the wholist to any mortal, and only seen on it by immortals greater than the higher of your wizi and incog levels.");
		ch->println("Note: Immortals being visible on the wholist generally make a more newbie friendly mud.");
    }
    else
    {
		ch->println( "Your presence on the wholist is already cloaked, use whovis to decloak." );
    }
    return;
}

/**************************************************************************/
/* prefix command: it will put the string typed on each line typed */
void do_prefi (char_data *ch, char *)
{
    ch->println( "You cannot abbreviate the prefix command." );
    return;
}

/**************************************************************************/
void do_prefix (char_data *ch, char *argument)
{
    if (IS_NULLSTR(argument))
    {
		if (IS_NULLSTR(ch->prefix))
		{
			ch->println( "You have no prefix to clear." );
			return;
		}

		ch->println( "Prefix removed." );	
		replace_string(ch->prefix, "");
		return;
    }

	if (IS_NULLSTR(ch->prefix)){
		ch->printlnf("Prefix changed to %s.",argument);
	}else{
		ch->printlnf("Prefix set to %s.\r\n",argument);
	}
	ch->println("Type 'prefix' by itself to get out of prefix mode");
	replace_string(ch->prefix,argument);
}

/**************************************************************************/
void do_pinfo( char_data *ch, char *argument )
{
	char buf[MSL];
	char pcolour[MSL];
	char bracket_col[MIL];
	BUFFER *output;
	int nMatch;
    char_data *wch;
	char status[2]; 
	char reduced_buf[MIL];
	bool generic_short;
	// status used to display different status codes 
	// - s when sleeping
	// - * when fighting
	// - + when in pkill fight
	// - ~ when in description editor
	// - @ when switched
	// - # when writing a note

	if ( is_number( argument  ) )
	{
		ch->println( "Levels are not supported by pinfo." );
		return;
	}

    nMatch = 0;
    output = new_buf();

    for ( wch = player_list; wch; wch = wch->next_player )
    {
		if (!can_see_who(ch,wch))
			continue;

		// filter on name/short
		if (!is_name( argument, wch->name ) 
			&& !is_name( argument, wch->short_descr)
			&& !is_name( argument, race_table[wch->race]->name)
			&& !is_name( argument, class_table[wch->clss].name)
			) 
			continue;

        nMatch++;
		
		status[0]='\0';
		if (!IS_AWAKE(wch))
            sprintf(status,"s");
		if (wch->pnote)
            sprintf(status,"#");
		if (wch->controlling)
            sprintf(status,"@"); 
		else if (IS_IN_EDITMODE(wch))
            sprintf(status,"~");
				
		if (wch->fighting)
		{
			if (!IS_NPC(wch->fighting) && status[0]!= '@')
				sprintf(status,"+");
			else
				sprintf(status,"*");
		}
							
		// setup the colour codes
		sprintf(pcolour,"`%s",
			(IS_LETGAINED(wch) && wch->pcdata->diplomacy==0) ? "g":
				(wch->pcdata->diplomacy? "R" : "x")); 	
		sprintf(bracket_col,"`%s", HAS_CHANNELOFF(wch,CHANNEL_OOC)?"c":
			IS_IRC(wch)?"B": IS_IMMORTAL(wch)?"G":"x");

		// reduced RPS or XP system
		reduced_buf[0]='\0';
		if (IS_ADMIN(ch) && (wch->pcdata->reduce_xp_percent 
			|| wch->pcdata->reduce_rps_percent))
		{
			sprintf( reduced_buf, " `#`Y(RXP %d RRPS %d)`^", 
				wch->pcdata->reduce_xp_percent, wch->pcdata->reduce_rps_percent);
		}

		// get generic short description colour
		{
			char tempbuf[MIL];
			sprintf(tempbuf,"a %s %s",
				(wch->sex==0 ? "sexless" : wch->sex==1 ? "male" : "female"),
				race_table[wch->race]->name);

			if(str_cmp(wch->short_descr, tempbuf)){
				generic_short=false;
			}else{
				generic_short=true;
			}
		}


		// Format it up
		sprintf( buf, "%s[%s%1s%1s%3d %+2d/%+2d %-6.6s %-3.3s %3d %3d %3d%3d %2d%s] %s%s%s%s-`x%s %s%s`x\r\n",
			bracket_col,
			pcolour, (wch->pcdata->true_sex==1) ? "M" : "F", status, wch->level,
			wch->alliance, wch->tendency,
			!IS_NULLSTR(race_table[wch->race]->short_name)?race_table[wch->race]->short_name:"",  
			class_table[wch->clss].short_name,
			(int) wch->hit*100/(wch->max_hit==0?1:wch->max_hit),
			(int) wch->mana*100/(wch->max_mana==0?1:wch->max_mana),
			(int) wch->move*100/(wch->max_move==0?1:wch->max_move),
			wch->pkkills,
			wch->pcdata->karns,
			bracket_col,
			(IS_NULLSTR(wch->description)?"`B":"`x"), // those with no long have a blue name
			wch->name,
			!IS_NULLSTR(wch->pcdata->immtalk_name)?"`#`G#`&":"",
			wch->pcdata?(IS_NULLSTR(wch->pcdata->charnotes)?"":"`Y"):"`x",
			reduced_buf,
			(generic_short?"`B":"`x"),  // those with a generic short have a blue short
			wch->short_descr);
		
		add_buf(output,buf);
	}
	
	ch->println("`x"
		" S  L   A  T   R     C   H   M   M   P  K\r\n"
		" E  V   L  E   C     L   P   N   V   K  R\r\n"
		" X  L   I  N   E     A   %   %   %   S  N" );
	ch->sendpage(buf_string(output));
	free_buf(output);
	return;
}

/**************************************************************************/
/**************************************************************************/
/*
 * do_rename renames a player to another name.
 * PCs only. Previous file is deleted, if it exists.
 * Char is then saved to new file.
 * New name is checked against std. checks, existing offline players and
 * online players. 
 * .gz files are not checked for 
 */
bool check_parse_name (char* name);		// comm.cpp

/**************************************************************************/
void do_rename (char_data* ch, char* argument)
{
	char old_name[MIL], 
	     new_name[MIL],
		 buf[MSL],
		 renamer[MSL];
	char *previous_name;

	char_data* victim;
	
	argument = one_argument(argument, old_name); // find new/old name 
	one_argument(argument, new_name);
	
	// Trivial checks 
	if (IS_NULLSTR(old_name))
	{
		ch->println( "Rename who?" );
		return;
	}
	
	victim = get_char_world (ch, old_name);
	
	if (!victim)
	{
		ch->println( "There is no such a person online." );
		return;
	}
	
	if (IS_NPC(victim))
	{   
		ch->println("You cannot use Rename on NPCs." );
		return;
	}

	// allow rename self new_name,but otherwise only lower level 
	if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) )
	{
		ch->println("You failed." );
		return;
	}
	
	if (!victim->desc || (victim->desc->connected_state != CON_PLAYING) )
	{
		ch->println( "This player has lost his link or is inside a pager or the like." );
		return;
	}

	if (!new_name[0])
	{
		ch->println( "Rename to what new name?" );
		return;
	}


	// First, check if there is a player named that off-line 
	{
		PFILE_TYPE result=find_pfiletype(new_name);
		if(result !=PFILE_NONE){
			ch->printlnf("A player with a name of '%s' already exists! (pt=%d)", 
				new_name, (int)result);
			return;
		}		
	}

	{ // don't allow duplicates with names in creation/level 1 unsaved player
		connection_data *dc;
		for ( dc = connection_list; dc; dc = dc->next )
		{
			if ( CH(dc) && !str_cmp(new_name, CH(dc)->name) )
			{
				ch->println( "Someone else is currently creating using that name." );
				return;
			}
		}
	}

	if(count_char(new_name, '`')>0){
		ch->println("You can't put the `` character into a name.");
		return;
	}

	if (!check_parse_name(new_name))
	{
		ch->println( "The new name is illegal, letting you rename anyway." );
	}
		
	strcpy(renamer, PERS(ch, NULL));
	
	// Rename the character and save them to a new file
	previous_name=victim->name;
	victim->name = str_dup(capitalize(new_name));
	
	// create the new file
	save_char_obj(victim);
	
	// unlink the old file 
	unlink(pfilename(previous_name, victim->pcdata->pfiletype));
	free_string(previous_name);

	// all finished
    ch->printlnf("You have renamed %s to %s.", old_name , victim->name);

	// update laston
	laston_update_char(victim);
	laston_save(NULL);// resave laston data
	
	logf("%s renamed %s to %s", renamer, old_name, victim->name);
	sprintf(buf,"%s renamed %s to %s\r\n", renamer, old_name, victim->name);
	wiznet(buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));

	victim->position = POS_STANDING; // I am laaazy 

    victim->wraplnf("You have been renamed to '%s' by %s.",
		victim->name, can_see_who(victim, ch)?ch->name:"a mystery imm");

	// do an autonote?
	if(GAMESETTING2(GAMESET2_AUTONOTE_RENAMES_TO_ADMIN)){
		autonote(NOTE_SNOTE, "p_anote()","player rename", "adminrename", buf, true);
	}

	// update the offline letgain database if required
		// find the node
	{
		letgain_data *node=find_letgain(old_name);
		if (!node){
			return;
		}
		replace_string(node->name,victim->name);
		ch->println("Name in offline letgain database updated.");
		save_letgain_db();
	}		
} 
/**************************************************************************/
// lets you make a mob die quickly for questing purposes
// also can be used to speed hotreboots - using qdie all
void do_qdie( char_data *ch, char *argument )
{
    char arg[MIL];
	char_data *victim;

	argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg))
    {
        ch->println( "syntax: qdie %playername  (sets the HP to 1 on the mob playername is fighting)" );
        ch->println( "    or: qdie %mobname (sets the HP to 1 on the mob if it is fighting someone)" );
        ch->println( "    or: qdie all (sets the HP to 1 on the fights)" );
        return;
    }

	if ( !str_cmp( arg, "all" ) )
    {
		char_data *vch;
		char_data *vch_next;

		if (!IS_ADMIN(ch))
		{
			 ch->println( "Not at your level!" );
			 return;
		}

		for ( vch = player_list; vch != NULL; vch = vch_next )
		{
			if (vch->next == NULL) 
			{
				return; 
			}
		
			vch_next = vch->next_player;

			victim= vch->fighting;

			if (victim && IS_NPC(victim) && victim->hit>1)
			{
				ch->printlnf( "Current hitpoints on %s lvl: %d have been set to 1 from %d (was fighting %s lvl: %d).", 
					victim->short_descr, victim->level,	victim->hit,
					vch->name, vch->level);
				victim->hit=1;

				update_pos( victim);
			}
		}
		return;
	}
    

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

    if (!IS_NPC(victim ))
	{
		if (!victim->fighting)
		{
			ch->println( "That player isn't fighting anyone." );
			return;
		}
		else
			victim= victim->fighting;
	}

	if (!IS_NPC(victim))
	{
		ch->println( "That player isn't a mob!" );
		return;
	}

	if (!victim->fighting)
	{
		ch->println( "That mob isn't fighting anyone!" );
		return;
    }

	if (victim->hit>1)
		victim->hit=1;
    update_pos( victim);

	ch->printlnf("Current hitpoints on %s have been set to 1.", victim->short_descr);
}
/**************************************************************************/
void do_showplayerlist( char_data *ch, char *)
{
	char_data *vch;
	char_data *vch_next;
	int count=0;

	for ( vch = player_list; vch != NULL; vch = vch_next )
	{
		if (vch->next == NULL) 
		{
			return; 
		}
	
		vch_next = vch->next_player;

		count++;
		ch->printlnf("%2d - %s ", count, vch->name);
	}
	return;
}
/**************************************************************************/
/**************************************************************************/
void do_overwritepasswd( char_data *ch, char *argument )
{
    char arg[MIL];
    connection_data *c;
	
    if(IS_NPC(ch)) 	{
		ch->println( "Only immortal players can use the overwrite password command." );
		return;
	}

    one_argument( argument, arg );
    if ( IS_NULLSTR(arg) ) {
		ch->titlebar("OVERWRITE PASSWORD");
		ch->println("`x-- How to use this command:");
		ch->println("1. Tell a player to connect to the mud, and type in their player name.");
		ch->wrapln("2. When it prompts them for a password, use the sockets command to find "
			"out their socket number... then type `=Coverwritepassword <socketnum>`x "
			"for their connection.");
		ch->wrapln("3. Tell them to just press enter when asked for the password.  The "
			"mud will allow them to login to the game.  They can change their password "
			"using the password command in the form `=Cpasswd - <newpassword>`x.");
		ch->println("Note1: This command can't be used on immortal characters.");
		ch->wrapln("Note2: You can also use the command on players which have "
			"already logged in, so they could use - in place of their old password "
			"with the password command.");
		ch->titlebar("");
		return;
    }
	
    if (is_number(arg))
    {
		int socknum;
		
		socknum = atoi(arg);
		for ( c = connection_list; c != NULL; c = c->next )
		{
            if ( c->connected_socket == socknum)
            {
				if (CH(c))
				{
					if (IS_NPC(CH(c))){
						ch->println( "Not on switched players." );
					}else if (IS_IMMORTAL(CH(c))){
						ch->println( "Not on IMMORTALS!" );
					}else{
						CH(c)->pcdata->overwrite_pwd = true;
						ch->printlnf("Tell %s to type `=Cpasswd - <newpassword>`x"
							" to update their password.", CH(c)->name);
						logf( "%s overwrote the password on %s!", ch->name, CH(c)->name);
					}
					return;
				}else{
					ch->printlnf( "Connection %d hasn't provided their name yet.", socknum);
					return;
				}
            }
		}
    }

	// show the help
	do_overwritepasswd(ch,"");
	ch->println( "You must use the socket number of a player which is "
		"available using the `=Csockets`x command." );
	return;
}
/**************************************************************************/
// displays a table showing all the info on all PC races
void do_rinfo( char_data *ch, char *)
{
	char buf[MSL];
    BUFFER *output;
	race_data *race;

    output= new_buf();
	sprintf( buf,"`?%s`x", format_titlebar("RINFO")); add_buf(output,buf);
	sprintf( buf,"`^===================== `Ysize`^ ========== `YHP`^ ========== `Ydefault`^ ======= `Yrecall`^ =\r\n"); add_buf(output,buf);
	sprintf( buf,"`^===== `Yrace`^ ======== `Ylow`^==`Yhigh`^ ==`Y start`^==`Ymax `^========  `Y lang `^=======   `Yvnum `^=\r\n"); add_buf(output,buf);
	for ( race = race_list->next; race; race=race->next )
	{
		if(!race->pc_race()){
			continue;
		}
		sprintf( buf,"%s%-18s %3d  %3d      %3d  %4d  %14s          %5d\n`x", 
			race->creation_selectable()?" `x":"`Y*`x",
			race->name,
			race->low_size,
			race->high_size,
			race->start_hp,
			race->max_hp,
			race->language->name,
			race->recall_room);			
		add_buf(output,buf);
	}
	sprintf( buf,"`?%s`x", format_titlebar("`xSee also the raceinfo command, `Y*`x=not creation selectable")); add_buf(output,buf);

	ch->sendpage(buf_string(output));
	free_buf(output);

}
/**************************************************************************/
// by Kalahn - June 98 - basically owhere tweaked
void do_rwhere(char_data *ch, char *argument)
{
    char buf[MIL];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;

	bool display_stat = false;    
	int display_number=0;
    OBJ_DATA *display_obj= NULL;

	int number = 0, max_found, vnum;
	
    found = false;
    number = 0;
    max_found = 200;

    if ( !IS_NULLSTR(argument))
    {
		if ( !is_number( argument ) ){
			ch->println( "Syntax:  rwhere <item number in list>" );
			ch->println( " rwhere with no parameters to see list of all restrung objects in the game." );
			return;
		}


		display_stat =true;
		display_number = atoi( argument );
		if (display_number <0){
			ch->println( "Value must be greater than 0." );
			return;
		}
	}

    buffer = new_buf();
	
	sprintf( buf,"`?%s`x", format_titlebar("RESTRUNG OBJECTS WHERE"));
	add_buf(buffer,buf);

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !obj->restrung ||   ch->level < obj->level)
            continue;

		// check to see if the item shouldn't be flaged as restrung
/*		if (obj->pIndexData){
			if (!str_cmp(obj->short_descr,obj->pIndexData->short_descr)
				&& !str_cmp(obj->name,obj->pIndexData->name)
				&& !str_cmp(obj->description,obj->pIndexData->description)
				&& !obj->extra_descr)
			{
				obj->restrung= false;
				continue;
			}
		}
*/

        found = true;
        number++;

		if (display_stat){
			if (number==display_number){
				display_obj=obj;
			}
			continue;
		}

		
        for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
            ;
		
		if (obj->pIndexData){
			vnum=obj->pIndexData->vnum;
		}else{
			vnum=-1;
		}

		if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
			&&   in_obj->carried_by->in_room != NULL)
		{
			sprintf( buf, "`G%3d)`x %s [%d] is \n    carried by %s [Room %d]`x\r\n",
				number, obj->short_descr, vnum,	PERS(in_obj->carried_by, ch),
				in_obj->carried_by->in_room->vnum );
		}
		else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
		{
			sprintf( buf, "`Y%3d)`x %s [%d] is \n    in %s [Room %d]`x\r\n",
				number, obj->short_descr, vnum, 
				in_obj->in_room->name, in_obj->in_room->vnum);
		}
		else
			sprintf( buf, "`R%3d)`x %s [%d] is somewhere`x\r\n",
				number, obj->short_descr, vnum);
		
        buf[0] = UPPER(buf[0]);
		
		if (!add_buf(buffer,buf))
		{
			ch->println( "Too many objects... buffer overflow." );
            break;
		} 	

        if (number >= max_found)
		{
			add_buf(buffer,"Not all restrung objects are listed - due to list limit.\r\n");
            break;
		}
    }
	
    if(!found){
        ch->println("No restrung objects are currently in the game.");
    }else{
        ch->sendpage(buf_string(buffer));

		// rwhere searching code 
		if (display_stat){
			if (display_obj){
				ostat_show_to_char(ch, display_obj);
			}else{
				ch->printlnf("Didn't find restring object %d in "
					"the rwhere list.", display_number);
			}
		}
	}	
    free_buf(buffer);
}
/**************************************************************************/
// by Kalahn - June 98
void do_breaktell(char_data *ch, char *argument)
{
    char arg[MIL];
	char_data *wch;
	bool found;

	if (IS_NPC(ch)){
		ch->println( "Players only." );
		return;
	}

	if (IS_SET(ch->comm, COMM_WHOVIS)){
		ch->println( "No point unless you are whoinvis first." );
		return;
	}

	argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg))
    {
        ch->println( "syntax: breaktell <playername>" );
        ch->println( "    or: breaktell all" );
        return;
    }

	found=false;
	if ( !str_cmp( arg, "all" ) )
    {
		for ( wch = player_list; wch; wch = wch->next_player )
		{
			if (can_see_who(wch,ch))
				continue;

			if (TRUE_CH(wch)->retell == ch)	{
				ch->printf("retell from %s disconnected.\r\n", TRUE_CH(wch)->name);
				TRUE_CH(wch)->retell =NULL;
				found=true;
			}

			if (TRUE_CH(wch)->reply == ch)	{
				ch->printf("reply from %s disconnected.\r\n", TRUE_CH(wch)->name);
				TRUE_CH(wch)->reply =NULL;
				found=true;
			}		
		}

		if (!found){
	        ch->println( "No one was found who can see you on the wholist,\r\n"
				"that had some form of tell lock on you." );
		}
		return;
	}
    

    if ( ( wch= get_char_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

	if (can_see_who(wch,ch)){       // was (ch,wch)
		ch->printlnf( "%s can see you on the wholist.", TRUE_CH(wch)->name );
		return;
	}

	if (TRUE_CH(wch)->retell == ch)	{
		ch->printlnf( "Retell from %s disconnected.", TRUE_CH(wch)->name);
		TRUE_CH(wch)->retell =NULL;
		found=true;
	}

	if (TRUE_CH(wch)->reply == ch)	{
		ch->printlnf( "Reply from %s disconnected.", TRUE_CH(wch)->name);
		TRUE_CH(wch)->reply =NULL;
		found=true;
	}
	
	if (!found){
		ch->println( "They aren't sending you retells nor replying to you." );
	}

}

/**************************************************************************/
// if a player is noprayed, all their prayers are automatically ignored
// on wiznet prayers... but it appears no different to the imm
// Kalahn August 98
void do_nopray( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
	char_data *victim;
 
	// can only be used by imms 
	if ( !IS_IMMORTAL(ch) )
	{
		do_huh(ch,"");
		return;
	}
	
	one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	{
		ch->println( "Nopray whom?" );
		return;
	}
 
    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }
 
    // can't nopray higher level imms than yourself
	if ( get_trust( victim ) >= LEVEL_IMMORTAL 
		&& get_trust( victim ) >= get_trust( ch ) )
    {
        ch->println( "You failed." );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOPRAY) )
    {
        REMOVE_BIT(victim->comm, COMM_NOPRAY);
		logf("nopray removed on %s by %s", victim->name, ch->name);
        ch->printf("nopray removed on %s - prays from them will now be heard on wiznet.\r\n", victim->name);
		sprintf(buf,"$N reconnects the prayers of %s to wiznet prayers",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOPRAY);

		logf("nopray set on %s by %s", victim->name, ch->name);
        ch->printf("nopray set on %s - prays from them will NOT be heard on wiznet :)\r\n", victim->name);
		sprintf(buf,"$N disconnects the prayers of %s to wiznet prayers",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    return;
}
/**************************************************************************/
void do_noterestrict( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
	char_data *victim;
 
    if (!IS_ADMIN(ch)
        && !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADLAW))	
    {
        ch->println( "Noterestrict can only be used by the head of the law council or admin." );
        return;
    }
	one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	{
		ch->wrapln( "Toggle restricted note writing on whom?  (When on they can only write to imm)." );
        return;
    }
 
    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }
 
    // can't reduce laston on high level imms than yourself
	if(victim!=ch){
		if ( get_trust( victim ) >= LEVEL_IMMORTAL 
			&& get_trust( victim ) >= get_trust( ch ) )
		{
			ch->println( "You failed." );
			return;
		}
	}
 
    if ( HAS_CONFIG(victim, CONFIG_NOTE_ONLY_TO_IMM))
    {
        REMOVE_CONFIG(victim, CONFIG_NOTE_ONLY_TO_IMM);
		logf("noterestriction removed on %s by %s", victim->name, ch->name);
		ch->printf("noterestriction removed on %s by %s\r\n", victim->name, ch->name);
		sprintf(buf,"$N turned off note restriction on %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_CONFIG(victim, CONFIG_NOTE_ONLY_TO_IMM);
		logf("noterestriction enabled on %s by %s", victim->name, ch->name);
		ch->printf("noterestriction ebabled on %s by %s\r\n", victim->name, ch->name);
		sprintf(buf,"$N turned on note restriction on %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    save_char_obj( victim );
    return;
}
/**************************************************************************/
void do_holylist(char_data *ch, char *)
{
	ch->println(  "  action     status" );
	ch->println(  "---------------------" );

	ch->printlnf( "holylight      %s",
		IS_SET( TRUE_CH(ch)->act, PLR_HOLYLIGHT ) ? "ON" : "OFF" );
	ch->printlnf( "holyname       %s",
		IS_SET( TRUE_CH(ch)->act, PLR_HOLYNAME ) ? "ON" : "OFF" );
	ch->printlnf( "holyspeech     %s   (character dependant)",
		HAS_HOLYSPEECH(ch) ? "ON" : "OFF" );
	ch->printlnf( "holywalk       %s",
		IS_SET( TRUE_CH(ch)->act, PLR_HOLYWALK ) ? "ON" : "OFF" );
	ch->printlnf( "holyvnum       %s",
		IS_SET( TRUE_CH(ch)->act, PLR_HOLYVNUM ) ? "ON" : "OFF" );

	if(TRUE_CH(ch)->incog_level)
	{
		ch->printlnf( "incognito      ON at %d",	TRUE_CH(ch)->incog_level);
	}
	else
	{
		ch->println( "incognito      OFF" );
	}

	ch->printlnf( "wizi            %d", ch->invis_level);

	if(TRUE_CH(ch)->iwizi)
	{
		ch->printlnf( "`RIC `xwizi        ON at %d", TRUE_CH(ch)->iwizi);
	}
	else
	{
		ch->println( "`RIC `xwizi        OFF" );
	}

	if(TRUE_CH(ch)->olcwizi)
	{
		ch->printlnf( "`YOLC `xwizi       ON at %d", TRUE_CH(ch)->olcwizi);
	}
	else
	{
		ch->println( "`YOLC `xwizi       OFF" );
	}

	if(TRUE_CH(ch)->owizi)
	{
		ch->printlnf( "`BOOC `xwizi       ON at %d",  TRUE_CH(ch)->owizi);
	}
	else
	{
		ch->println( "`BOOC `xwizi       OFF");
	}
}
/**************************************************************************/
// Kerenos November 98
void do_fadein( char_data *ch, char *argument )
{
    if ( !IS_NPC( ch )) {
		smash_tilde( argument );

		if ( argument[0] == '\0' ) {
		    ch->printf("Your fadein is %s\r\n",ch->pcdata->fadein );
			return;
		}

		free_string( ch->pcdata->fadein );
		ch->pcdata->fadein = str_dup( argument );

        ch->printf("Your fadein is now %s\r\n",ch->pcdata->fadein );
    }
    return;
}

/**************************************************************************/
// Kerenos November 98
void do_fadeout( char_data *ch, char *argument )
{
    if ( !IS_NPC( ch )) {
        smash_tilde( argument );
 
        if ( argument[0] == '\0' ) {
            ch->printf("Your fadeout is %s\r\n", ch->pcdata->fadeout );
            return;
        }
 
        free_string( ch->pcdata->fadeout );
        ch->pcdata->fadeout = str_dup( argument );
 
        ch->printf("Your fadeout is now %s\r\n",ch->pcdata->fadeout );
    }
    return;
}
/**************************************************************************/

void do_strip_affects( char_data *ch, char *argument )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	int sn;
	char_data *victim;
	char arg1[MIL];
	char arg2[MIL];

	argument = one_argument( argument, arg1);
	argument = one_argument( argument, arg2);

	if ( IS_NULLSTR( arg1 )) {
		ch->println( "Syntax: stripaff <target> <all|specific spell>" );
		return;
	}

	if (( victim = get_char_world( ch, arg1 )) == NULL ) {
		ch->printlnf( "No '%s' found.", arg1 );
		return;
	}

	if (( !str_cmp( arg2, "all" )) || ( IS_NULLSTR ( arg2 ))) {
        ch->println( "All affects cleared." );

		if ( str_cmp( victim->name, ch->name ))
			victim->println( "Your magical spells have been stripped away!" );

		for (paf = victim->affected; paf != NULL; paf = paf_next) {
			paf_next = paf->next;
			affect_remove(victim, paf);
		}

        victim->affected = NULL;
        return;
	}

	sn = skill_lookup(arg2);

	if ( is_affected( victim, sn )) {
		for (paf = victim->affected; paf !=NULL; paf=paf_next) {
			paf_next = paf->next;
			if ( paf->type == sn)
				affect_remove(victim, paf);
		}
		ch->println( "Affect removed." );

		if ( str_cmp( victim->name, ch->name ))
			victim->println("Some of your dwoemers have been stripped away!" );

		return;
	}
}

/**************************************************************************/
void do_setalliance( char_data *ch, char *argument )
{
	char arg1 [MIL];
	char arg2 [MIL];
	char_data *victim;
	int value;

	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		ch->println( "Syntax: setalliance <name> <value>" );
		ch->println( "  Value being a number from -3 to 3" );
		return;
	}

	if (( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	// clear zones for mobs
	victim->zone = NULL;

	/*
	 * Snarf the value (which need not be numeric).
	 */
	value = is_number( arg2 ) ? atoi( arg2 ) : 99;

	if ( value < -3 || value > 3 ) {
		ch->println( "Alliance range is -3 to 3." );
		return;
	}
	ch->printlnf( "Alliance changed from %d to %d on %s(%s)",
		victim->alliance , value, victim->short_descr, victim->name);
		victim->alliance = value;
	return;
}

/**************************************************************************/
void do_settendency( char_data *ch, char *argument )
{
	char arg1 [MIL];
	char arg2 [MIL];
	char_data *victim;
	int value;

	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		ch->println( "Syntax: settendency <name> <value>" );
		ch->println( "  Value being a number from -3 to 3" );
		return;
	}

	if (( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	// clear zones for mobs
	victim->zone = NULL;

	 /*
	  * Snarf the value (which need not be numeric).
	  */
	value = is_number( arg2 ) ? atoi( arg2 ) : 99;

	if ( value < -3 || value > 3 ) {
		ch->println( "Tendency range is -3 to 3." );
		return;
	}
	ch->printlnf( "Tendency changed from %d to %d on %s(%s)",
		victim->tendency, value, victim->short_descr, victim->name);
		victim->tendency = value;
	return;
}

/**************************************************************************/
void do_dream( char_data *ch, char *argument )
{	
	char buf[MSL];
	
	if( IS_AWAKE(ch)){
        ch->println("You day dream a bit...\r\nTry dreaming while you are asleep.");
        return;
	}
	
	if( IS_NULLSTR(argument)){
        ch->println("What would you like to dream?");
        return;
	}
	
	ch->printlnf( "You dream: `s%s`x", argument );
	
	if (!IS_SET(ch->comm, COMM_NOPRAY)){
        sprintf(buf,"`w%s dreams `w'`s%s`w'`x\r\n", 
			ch->name, argument);
        wiznet(buf,NULL,NULL,WIZ_PRAYERS_DREAMS,0,AVATAR); 
	}
	return;
}

/**************************************************************************/
void flush_cached_write_to_buffer(connection_data *d);
/**************************************************************************/
void do_rawcolour( char_data *ch, char *argument )
{
    if ( IS_NULLSTR( argument ))
    {
        ch->println( "Rawcolour what?" );
        return;
    }			
	
	if(!ch->desc){
        ch->println( "You must have a valid connection to use raw colour." );
        return;
	}

    ch->println("RAWCOLOUR BEGIN.");
	// condensing system has to be flushed for raw colour to work correctly
	flush_cached_write_to_buffer(ch->desc);
	ch->desc->parse_colour=false;

	forced_interpret( ch, argument );
	if(ch->desc){
		flush_cached_write_to_buffer(ch->desc);
		ch->desc->parse_colour=true;

		ch->println("RAWCOLOUR END.");
	}
}
/**************************************************************************/
void do_permrawcolour( char_data *ch, char *argument )
{  
	if ( str_cmp("confirm", argument)) {
		ch->printf("Turns on raw colour permantantly (till use the raw command to turn it off):\r\n");
		ch->printf("type 'permraw confirm' to activate.\r\n");
		return;
	}
	if(!ch->desc){
		ch->println("You must have an active connection to do this.");
		return;
	}
    ch->println("RAWCOLOUR TURNED ON, USE ANY COMMAND WITH RAW TO TURN IT OFF OR RELOG.");
	flush_cached_write_to_buffer(ch->desc);
	ch->desc->parse_colour=false;
}
/**************************************************************************/
void do_showaffectprofile( char_data *ch, char *argument )
{
	int		index;
	bool	found = false;

	if ( IS_NULLSTR(argument))
	{
		if(!codehelp(ch,"showaffectprofile", false))
		{
			ch->println("Syntax:  showaffectprofile <profile>");
			ch->println("The affectprofile must be one of the following:");
			affectprofile_list( ch );
			return;
		}
	}

	if ( !is_number( argument ))
	{
		for ( index=0; !IS_NULLSTR(affectprofile_table[index].name);index++)
		{
			if ( !str_prefix( argument, affectprofile_table[index].name ))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			ch->printf("The affectprofile must be one of the following:\r\n");
			affectprofile_list( ch );
			return;
		}
	}
	else
	{
		index = atoi(argument);
		if ( index < 0 || IS_NULLSTR(affectprofile_table[index].name))
		{
			ch->printf("The affectprofile must be one of the following:\r\n");
			affectprofile_list( ch );
			return;
		}
	}

	format_titlebar( "Affect Profile");

	ch->printf("`=rName:  `x%s\r\n",	affectprofile_table[index].name );
	ch->printf("`=rDesc:  `x%s\r\n",	affectprofile_table[index].description );
	ch->printf("`=rFlags: `x%d\r\n",	affectprofile_table[index].flags );
	ch->printf("`=rWear Msg: `x%s\r\n",affectprofile_table[index].wear_message );
	ch->printf("`=rWear Loc: `x%-15s  `=rWear Amount: `x%d\r\n",
		flag_string( apply_types, affectprofile_table[index].wear_location),
		affectprofile_table[index].wear_amount );
	ch->printf("`=rForced Drop Msg: `x%s\r\n", affectprofile_table[index].forced_drop_message );
	ch->printf("`=rRemove Message:  `x%s\r\n", affectprofile_table[index].remove_message );
	ch->printf("\r\n  == Movement Stats ==\r\n" );
	ch->printf("  `=rChance: `x%-3d  `=rLocation: `x%-3d  `=rAmount: `x%-3d\r\n",
		affectprofile_table[index].move_chance,
		affectprofile_table[index].move_location,
		affectprofile_table[index].move_amount );
	ch->printf("  `=rMessage: `x%s\r\n", affectprofile_table[index].move_message );
	ch->printf("\r\n  == Tick Stats ==\r\n" );
	ch->printf("  `=rChance: `x%-3d  `=rLocation: `x%-3d  `=rAmount: `x%-3d\r\n",
		affectprofile_table[index].tick_chance,
		affectprofile_table[index].tick_location,
		affectprofile_table[index].tick_amount );
	ch->printf("  `=rMessage: `x%s\r\n", affectprofile_table[index].tick_message );

	ch->println("NOTE: The affect profile system has not been finished");
	return;
}
/**************************************************************************/
void affectprofile_list( char_data *ch )
{
	int index;

	for(index=0; !IS_NULLSTR(affectprofile_table[index].name);index++)
	{
		ch->printf("%-2d %-25s - %s\r\n",
			index,	affectprofile_table[index].name,
			affectprofile_table[index].description);
	}
	return;
}
/**************************************************************************/
void do_freevnum(char_data *ch, char *argument)
{
	int i,j;
	char arg[MIL];

    if (!HAS_SECURITY(ch,1)){
		ch->println("The freevnum command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_AREA) && !IS_IMMORTAL(ch)){
		ch->println("freevnum: Invalid security for viewing information about this area.");
		return;
    }

	one_argument( argument, arg );
	
	if ( IS_NULLSTR( arg )){
		ch->println("Syntax:  freevnum <obj/mob/room/mudprog/mp>");		
		return;
    }

	j=1;

	if (!str_cmp(arg,"obj"))
    {
		ch->titlebar("FREE OBJ VNUMS");
		ch->titlebar(ch->in_room->area->name);

		for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
			if (get_obj_index(i) == NULL) {
				ch->printf("%8d ",i);
				if (j == COLUMNS) {
					ch->println("");
					j=0;
				}
				j++;
			}
		}
		ch->println("`x");
		return;
    }

	if (!str_cmp(arg,"mob"))
    {
		ch->titlebar("FREE MOBILE VNUMS");
		ch->titlebar(ch->in_room->area->name);

		for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
			if (get_mob_index(i) == NULL) {
				ch->printf("%8d ",i);
				if (j == COLUMNS) {
					ch->println("");
					j=0;
				}
				j++;
			}
		}
		ch->println("`x");
		return;
    }

	if (!str_cmp(arg,"room"))
    {
		ch->titlebar("FREE ROOM VNUMS");
		ch->titlebar(ch->in_room->area->name);

		for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
			if (get_room_index(i) == NULL) {
				ch->printf("%8d ",i);
				if (j == COLUMNS) {
					ch->println("");
					j=0;
				}
				j++;
			}
		}
		ch->println("`x");
		return;
    }

	if (!str_cmp(arg,"mudprog") || !str_cmp(arg,"mp"))
    {
		ch->titlebar("FREE MUDPROG VNUMS");
		ch->titlebar(ch->in_room->area->name);

		for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
			if (get_mprog_index( i) == NULL) {
				ch->printf("%8d ",i);
				if (j == COLUMNS) {
					ch->println("");
					j=0;
				}
				j++;
			}
		}
		ch->println("`x");
		return;
    }

	// no matched command... rerun with no argument to show syntax
	do_freevnum(ch, "");
}
/**************************************************************************/
void do_removequester( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
	char_data *victim;
 
	// can only be used by imms 
	if (  !IS_IMMORTAL(ch) )
	{
		do_huh(ch,"");
		return;
	}
	
	one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	{
        ch->println( "Removequester status from whom?" );
        return;
    }
 
    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

 
    if ( IS_SET(victim->act, PLR_QUESTER) )
    {
        REMOVE_BIT(victim->act, PLR_QUESTER);
		logf("quester flag removed on %s by %s", victim->name, ch->name);
        ch->printf("quester flag removed on %s.\r\n", victim->name);
        victim->printf("Your quester flag has been removed.\r\n");
		sprintf(buf,"$N removes the quester flag of %s",victim->name);
		wiznet(buf,ch,NULL,WIZ_QUESTING,WIZ_SECURE,0);
    }else{
		ch->printf("%s didn't have the quester flag set to start with.\r\n", 
			PERS(victim, ch));
	}
    return;
}

/**************************************************************************/
void do_setdeity( char_data *ch, char *argument ) // Tibault 11/07/2000
{
	char		victimName[MIL];
	char		deityName[MIL];

	char_data*	victim;
	DEITY_DATA* pDeity;

	argument = one_argument( argument, victimName);
	argument = one_argument( argument, deityName);

	if ( victimName[0] == '\0' || deityName[0] == '\0' )
	{
		ch->println( "Syntax: `#`=Csetdeity `W<char> <deity name/none>`^." );
		ch->println( "Look at `#`=Cdedit list`^ for all the existing deities." );
		return;
	}

	if (( victim = get_char_world( ch, victimName ) ) == NULL )
	{
		ch->println( "They aren't here." );
		return;
	}

	if (IS_NPC(victim))
	{
		ch->println( "You cannot `#`=Csetdeity`^ on NPC's.");
		return;
	}

	if (!HAS_CLASSFLAG( victim, CLASSFLAG_DEITIES ))
	{
		ch->println( "You can only `#`=Csetdeity`^ on classes with the `Ydeities`^ flag." );
		ch->println( "This flag is set in `#`=Cclassedit`^." );
		return;
	}

	if ( strstr(deityName, "none") != '\0' )
	{
		if ( victim->deity && victim->deity->name )
		{
			ch->printlnf( "`#`Y%s`^ no longer worships `W%s`^.", victim->name, victim->deity->name );
			victim->printlnf( "%s no longer cares for you!", victim->deity->name) ;
			victim->pcdata->worship_time = current_time;
			victim->deity = NULL;
			return;
		}
		else
		{
			ch->printlnf( "`#`Y%s`^ already doesn't worship any deity.", victim->name );
			return;
		}
	}

	if ( (pDeity = deity_lookup(deityName)) == NULL) 
	{
		ch->printlnf( "`#`Y%s`^ is not a valid deity.", deityName);
		ch->println( "Please look at `#`=Cdedit list`^ for existing deities." );
		return;
	}
	
	ch->printlnf("`#`Y%s`^ worships `W%s`^ now.", victim->name, pDeity->name );
	victim->deity = pDeity;
	victim->pcdata->worship_time = current_time;
	victim->printlnf( "You now worship %s!", pDeity->name );
	return;
}

/**************************************************************************/
void do_setrace( char_data *ch, char *argument )
{
	char	arg [MIL];
	int		race;

	smash_tilde( argument );
	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->printf("Syntax: setrace <race> - Note: This is a self set only command\r\n" );
		return;
	}

	race = race_lookup(arg);
		
	if ( race == -1)
	{
		ch->printlnf("'%s' is not a valid race.", arg);
		return;
	}
		
	if ( !race_table[race]->pc_race() )
	{
		ch->printlnf("%s is not a valid player race.", race_table[race]->name );
		return;
	}
		
	ch->printlnf("Your race has changed from %s to %s.",
		race_table[ch->race]->name, 
		race_table[race]->name );
		ch->race = race;
		reset_char(ch); // recalc the changes
		return;
}
/**************************************************************************/
// add a single person to the iwhere buffer
void add_iwhere_person(BUFFER *buffer, char_data *v, int *count, int *group)
{
    char buf[MSL];

	if(IS_NPC(v)){
		sprintf(buf,"%s%2d) `x[%s%5d`x] %-12s `Y(in the body of %s`S%2d`Y)`x is in %s (%s%s`x)\r\n",			
			colour_table[((*group)%14)+1].code, *group, 
			colour_table[(v->in_room->vnum%14)+1].code,
			v->in_room->vnum, 
			TRUE_CH(v)->name,v->short_descr, INVIS_LEVEL(v), 
			v->in_room->name, 
			colour_table[(v->in_room->area->vnum%14)+1].code,
			v->in_room->area->name);
	}else{
		sprintf(buf,"%s%2d) `x[%s%5d`x] %-12s`S%2d`x is in %s (%s%s`x)\r\n",
			colour_table[((*group)%14)+1].code,*group, 
			colour_table[(v->in_room->vnum%14)+1].code,
			v->in_room->vnum, v->name,
			INVIS_LEVEL(v), 
			v->in_room->name, 
			colour_table[(v->in_room->area->vnum%14)+1].code,
			v->in_room->area->name);
	}
	++(*count);
	SET_BIT(v->dyn, DYN_TEMP_FLAG);
	add_buf(buffer,buf);

}
/**************************************************************************/
// adds a whole room entry to the iwhere buffer - Kal - Nov 99
void add_iwhere_room_list(char_data *looker, BUFFER *buffer, char_data *v, int *count, int *group)
{
	if(!v->in_room){
		bugf("add_iwhere_room_list: v->in_room==NULL, v->name==%s\r\n", v->name);
		return;
	}

	if(!can_see_who(looker, v)){
		return;
	}

	(*group)++;
	// add the person to the list
	add_iwhere_person(buffer, v, count, group);

	// get all those in the room
	for( char_data *rch = v->in_room->people; rch; rch = rch->next_in_room )
    {
		if( IS_SET(rch->dyn, DYN_TEMP_FLAG) 
			|| IS_UNSWITCHED_MOB(rch)
			|| !can_see_who(looker, rch)){
			continue;
		};
		add_iwhere_person(buffer, rch, count, group);
	}
}
/**************************************************************************/
// immortal where - Kal - Nov 99
// shows where imms are, and any morts in the room, including switches
void do_iwhere( char_data *ch, char *)
{
	BUFFER *buffer;
    char_data *vch;
	int count=0;
	int group=0;
	char buf[MSL];
	
	buffer = new_buf();
	add_buf(buffer,"`=t`#"); // get the set colour
	sprintf( buf, "`^%s`x", format_titlebar("IMMORTAL WHERE`^"));
	add_buf(buffer,buf);

	// turn off everyone's DYN_TEMP_FLAG 
	for ( vch = char_list; vch; vch = vch->next)
	{
		REMOVE_BIT(vch->dyn, DYN_TEMP_FLAG);
		if(vch->controlling){
			REMOVE_BIT(vch->controlling->dyn, DYN_TEMP_FLAG);
		}
	}

	for ( vch = char_list; vch; vch = vch->next)
	{
		// don't directly list morts or those already list
		if(IS_SET(vch->dyn, DYN_TEMP_FLAG) || !IS_IMMORTAL(vch)){
			continue;
		};

		add_iwhere_room_list(ch, buffer, vch, &count, &group);

		// switched imm in a room that hasn't been processed yet
		if(vch->controlling && !IS_SET(vch->controlling->dyn, DYN_TEMP_FLAG)){
			add_iwhere_room_list(ch, buffer, vch->controlling, &count, &group);
		}
	}

	sprintf( buf, "`^%s`x", format_titlebarf("%d TOTAL`^", count));
	add_buf(buffer,buf);

	ch->sendpage(buf_string(buffer));
	free_buf(buffer);
    return;
}
/**************************************************************************/
void do_pracsystester( char_data *ch, char *argument )
{
    char arg[MIL];
	char_data *victim;
 
	one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{
        ch->println("Set whom as a prac/improve system tester?");			
		ch->println("Don't put this on players unless you know what you are doing.");
        return;
    }
 
    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }
 
    if ( HAS_CONFIG(victim,CONFIG_PRACSYS_TESTER))
    {
        REMOVE_CONFIG(victim,CONFIG_PRACSYS_TESTER);
		ch->printf("%s is no longer a prac system tester.\r\n", victim->name);
    }
    else
    {
        SET_CONFIG(victim,CONFIG_PRACSYS_TESTER);
		ch->printf("%s is no a prac system tester.\r\n", victim->name);
    }
    save_char_obj( victim );
    return;
}
/**************************************************************************/
void do_forcetime( char_data *ch, char * )
{
	time_info.minute = 59;
	ch->println("Minutes set to 59.");
	ch->printlnf( "WEATHER - Change %d  Mage Castmod %d MMHG %d Moon Bright %d Sky %d Sunlight %d",
		weather_info[ch->in_room->sector_type].change,
		weather_info[ch->in_room->sector_type].mage_castmod,
		weather_info[ch->in_room->sector_type].mmhg,
		weather_info[ch->in_room->sector_type].moon_getting_brighter,
		weather_info[ch->in_room->sector_type].sky,
		weather_info[ch->in_room->sector_type].sunlight );
}
/**************************************************************************/
void do_webpass( char_data *ch, char *argument )
{
	if(IS_NPC(ch)){
		ch->println("Players only sorry!");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println( "This command sets your password for the webpage." );
		ch->println( "Syntax: webpass <password>");
		return;
	}

	if(str_len(argument )>=25){
		ch->println("Password must be less than 25 characters.");
		return;
	}

	char *new_pass=dot_crypt(argument, ch->name);

	if(count_char(new_pass, '~')!=0){
		ch->println("The new password is not suitable, try another.");
		return;
	}
	
	ch->println("Web password set.");
	replace_string(ch->pcdata->webpass, new_pass);
	laston_update_char(ch);
	save_char_obj(ch);
}
/**************************************************************************/
// Take an object off a player, regardless of if it is no remove etc.
// - Balo/Kal, Sept 01
void do_immget(char_data *ch,char *argument)
{
    char_data *victim;
    obj_data *obj; 
    char arg1[MIL];
    char buf[MSL];
    buf[0] = '\0';
    bool found = false;
	
    argument = one_argument(argument,arg1);

	// unswitched mobs can't use this command
	if(IS_UNSWITCHED_MOB(ch)){
		ch->println( "Players or switched players only." );
		return;
	}
    
    if(IS_NULLSTR(arg1)){
		ch->println("Syntax:");
		ch->println("immget <char|mob> <item>");
		ch->wrapln("Takes an object off a player/mob, if they are actually using the item."
			"  The first time the command is used, it will unequip them, the second time "
			"it will actually take the item.");
		return; 
	}
    
    if((victim = get_char_room(ch,arg1)) == NULL){ 
		ch->printlnf("There is no mob/player called '%s' here.", arg1);
		return; 
	}
    
    if(!IS_NPC(victim) && IS_TRUSTED(victim, get_trust(ch))){
		ch->println("They are too high level for you to do that.");
		return; 
	}
    
    for(obj = victim->carrying; obj; obj = obj->next_content ){
		if ( is_name( argument, obj->name )
			&&   can_see_obj( ch, obj ))
		{
			found = true;
			break;
		}
    }
    
    if(!found){ 
		ch->printlnf("%s doesn't have any '%s' item.", PERS(victim, ch), argument );
		return; 
	}

    if( obj->wear_loc != WEAR_NONE && obj->wear_loc != WEAR_SHEATHED ){
		ch->printlnf("%s is currently using %s", PERS(victim, ch), obj->short_descr);
		ch->wraplnf("Unequiping it from them into their inventory... "
			"if you want to take it off them, use immget once more.");
		unequip_char(victim, obj);
	    victim->printlnf("%s has just unequipped %s from you, into your inventory.",ch->name,obj->short_descr);		
		return;
	}
    obj_from_char( obj );  
    obj_to_char( obj, ch );
    ch->printlnf("You have taken %s from %s.", obj->short_descr, PERS(victim, ch));
    victim->printlnf("%s has taken %s from you.",ch->name,obj->short_descr);
}
/**************************************************************************/
// turn on/off the prompt switch prefix
void do_switchprefix( char_data *ch, char *argument )
{
	if(!IS_SWITCHED(ch)){
		ch->wrapln("This command is used to turn on/off the prefix to "
			"your prompt that is displayed while you are switched.  It has "
			"no effect while you aren't switched.");
		return;
	}
	
	if(IS_SET(TRUE_CH(ch)->dyn, DYN_NO_PROMPT_SWITCHED_PREFIX)){
		REMOVE_BIT(TRUE_CH(ch)->dyn, DYN_NO_PROMPT_SWITCHED_PREFIX);
		ch->println("The switch prefix on your prompt is now displayed.");
	}else{
		SET_BIT(TRUE_CH(ch)->dyn, DYN_NO_PROMPT_SWITCHED_PREFIX);
		ch->println("The switch prefix on your prompt is no longer displayed.");
	}
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

