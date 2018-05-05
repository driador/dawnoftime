/**************************************************************************/
// rp.cpp - code dealing with rp and rp support
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "security.h"
#include "interp.h"
#include "olc.h"
#include "ictime.h"

// Locals
void show_rpsheet( char_data *ch, char_data *v);

/**************************************************************************/
void do_metric( char_data *ch, char * )
{
	if ( HAS_CONFIG( ch, CONFIG_METRIC )) {
		ch->println("You will now see your height and weight in Imperial.");
		REMOVE_CONFIG( ch, CONFIG_METRIC );
	} else {
		SET_CONFIG( ch, CONFIG_METRIC );
		ch->println("You will now see your height and weight in Metric.");
	}
}

/**************************************************************************/
void do_rpsupport( char_data *ch, char *argument )
{
	char arg[MIL];
	char_data *victim;
	
    if (!IS_ADMIN(ch)
        && !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADRP))
	{
        ch->println("RP support can only be used by admin or headrp.");
        return;		
	}

    one_argument( argument, arg );
    
    if( IS_NULLSTR(arg)){
        ch->println("Set as/turn off rpsupport on whom?");
        return;
    }
    if (( victim = get_whovis_player_world( ch, arg )) == NULL ){
        ch->printlnf("Can't find any player '%s' in the game.", arg);
        return;
    }

    if ( HAS_CONFIG(victim, CONFIG_RP_SUPPORT ))
    {
        REMOVE_CONFIG(victim, CONFIG_RP_SUPPORT);
        ch->printlnf("RP support turned off for %s.",
            PERS(victim, ch));
	}else{
        SET_CONFIG(victim, CONFIG_RP_SUPPORT);
        ch->printlnf("RP support turned on for %s.",
            PERS(victim, ch));
    }

    save_char_obj( victim );
    return;
}

/**************************************************************************/
void do_rp_obj_load( char_data *ch, char *argument )
{
	OBJ_INDEX_DATA	*pObjIndex;
	OBJ_DATA		*pObj;
	int				wear;

	if ( !IS_IMMORTAL(ch) && !HAS_CONFIG(ch, CONFIG_RP_SUPPORT))
	{
		do_huh(ch,"");
		return;
	}

	if (( pObjIndex = get_obj_index( OBJ_VNUM_RP_ITEM )) == NULL ) {
		ch->println("RP item non-existant, please report this with a note to admin.");
		return;
	}

	pObj = create_object( pObjIndex);
	ch->println("rpobjload: You've created an RP item and it's in your inventory.");

	if ( !IS_NULLSTR( argument ))
	{
		wear = flag_value( wear_flags, argument );
		if ( wear == NO_FLAG )
		{
			ch->printlnf( "'%s' is not a valid wear location. Item reverted to standard take & hold.", argument );
			pObj->wear_flags = OBJWEAR_TAKE|OBJWEAR_HOLD;
		}
		else
		{
			pObj->wear_flags = OBJWEAR_TAKE|wear;
			ch->printlnf( "RP item's wear loc are now: %s.", flag_string( wear_flags, pObj->wear_flags ));
		}
	}
	else
	{
		pObj->wear_flags = OBJWEAR_TAKE|OBJWEAR_HOLD;
	}

	obj_to_char( pObj, ch );
}

/**************************************************************************/
void do_surname( char_data *ch, char *argument )
{
	char arg1[MIL];
	char_data	*victim;

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	if(c_str_len(argument)==-1)
		return;

	if ((IS_IMMORTAL(ch) || IS_RP_SUPPORT(ch)))
	{
		argument = one_argument( argument, arg1 );

		if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( argument ))
		{
			ch->println("Syntax: rpsheet surname <player> <name>");
			return;
		}

		if (( victim = get_char_world( ch, arg1 )) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		if ( IS_NPC( victim ))
		{
			ch->println("Players only.");
			return;
		}

		if ( str_len(argument) > 20)
		{
			ch->println("Surname must be 20 characters or less.");
			return;
		}


		if (!str_cmp( argument, "none" ))
		{
	        free_string( victim->pcdata->surname );
			ch->printlnf("%s's surname has been cleared.", victim->name );
			victim->println("Your surname has been cleared.");
			return;
		}

		free_string( victim->pcdata->surname );
        victim->pcdata->surname = str_dup( argument );
		ch->printlnf("%s's surname has been changed to %s.",
			victim->name, victim->pcdata->surname );
		victim->printlnf("Your surname has been changed to %s.",
			victim->pcdata->surname );
		return;
	}

/*	if ( IS_LETGAINED( ch ))
	{
		ch->println("You cannot set your surname once you have been letgained.");
		ch->println(" Send a note to rpsupport if you really need it changed.");
		ch->println(" Please send the note only if you have a really good reason");
		ch->println(" to have it changed, ie marriage, typo, etc...");
		return;
	}
*/
	if ( IS_NULLSTR( argument ))
	{
		ch->println("Syntax: rpsheet surname <name>");
		return;
	}

	if ( str_len(argument) > 20)
	{
		ch->println("Surname must be 20 characters or less.");
		return;
	}

	free_string( ch->pcdata->surname );
	ch->pcdata->surname = str_dup( argument );
	ch->printlnf("Your surname is now %s.\r\nBe aware that you cannot set this after being letgained.",
		ch->pcdata->surname );
	return;
}

/**************************************************************************/
void do_crest( char_data *ch, char *argument )
{
	char arg1[MIL];
	char_data	*victim;

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	if(c_str_len(argument)==-1)
		return;

	if ((IS_IMMORTAL(ch) || IS_RP_SUPPORT(ch)))
	{
		argument = one_argument( argument, arg1 );

		if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( argument ))
		{
			ch->println("Syntax: rpsheet crest <player> <crest>");
			return;
		}

		if (( victim = get_char_world( ch, arg1 )) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		if ( IS_NPC( victim ))
		{
			ch->println("Players only.");
			return;
		}

		if ( str_len(argument) > 40)
		{
			ch->println("Crest must be 40 characters or less.");
			return;
		}


		if (!str_cmp( argument, "none" ))
		{
	        free_string( victim->pcdata->crest );
			ch->printlnf("%s's crest has been cleared.", victim->name );
			victim->println("Your crest has been cleared.");
			return;
		}

		free_string( victim->pcdata->crest );
        victim->pcdata->crest = str_dup( argument );
		ch->printlnf("%s's crest has been changed to %s.",
			victim->name, victim->pcdata->crest );
		victim->printlnf("Your crest has been changed to %s.",
			victim->pcdata->crest );
		return;
	}

/*	if ( IS_LETGAINED( ch ))
	{
		ch->println("You cannot set your crest once you have been letgained.");
		ch->println(" Send a note to rpsupport if you really need it changed.");
		ch->println(" Please send the note only if you have a really good reason");
		ch->println(" to have it changed, ie marriage, typo, etc...");
		return;
	}
*/
	if ( IS_NULLSTR( argument ))
	{
		ch->println("Syntax: rpsheet crest <symbol>");
		return;
	}

	if ( str_len(argument) > 40)
	{
		ch->println("Crest must be 40 characters or less.");
		return;
	}

	free_string( ch->pcdata->crest );
	ch->pcdata->crest = str_dup( argument );
	ch->printlnf("Your crest is now %s.",
		ch->pcdata->crest );
	return;
}


/**************************************************************************/
void do_birthplace( char_data *ch, char *argument )
{
	char arg1[MIL];
	char_data	*victim;

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	if(c_str_len(argument)==-1)
		return;

	if (IS_IMMORTAL(ch))
	{
		argument = one_argument( argument, arg1 );

		if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( argument ))
		{
			ch->println("Syntax: rpsheet birthplace <player> <name>");
			return;
		}

		if (( victim = get_char_world( ch, arg1 )) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		if ( IS_NPC( victim ))
		{
			ch->println("Players only.");
			return;
		}
		
		if ( str_len(argument) > 20)
		{
			ch->println("Birthplace must be 20 characters or less.");
			return;
		}

		free_string( victim->pcdata->birthplace );
        victim->pcdata->birthplace = str_dup( argument );
		ch->printlnf("%s's birthplace is now %s.",
			victim->name, argument );
		victim->printlnf("Your birthplace is now %s.",
			argument );
		return;
	}

/*	if ( IS_LETGAINED( ch ))
	{
		ch->println("You cannot set your birthplace once you have been letgained.");
		ch->println(" Send a note to imm if you really need it changed.  This is");
		ch->println(" something we don't change unless it's an extreme case since");
		ch->println(" where you are born can never really change.");
		return;
	}
*/
	if ( IS_NULLSTR( argument ))
	{
		ch->println("Syntax: rpsheet birthplace <place>");
		return;
	}

	if ( str_len( argument ) > 20 )
	{
		ch->println("Birthplace must be 20 characters or less.");
		return;
	}



	free_string( ch->pcdata->birthplace );
	ch->pcdata->birthplace = str_dup( argument );
	ch->printlnf("Your birthplace is now %s.\r\nBe aware that you cannot set this after being letgained.",
		argument );
	return;
}

/**************************************************************************/
void do_haircolour( char_data *ch, char *argument )
{
	char arg1[MIL];
	char_data	*victim;

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	if(c_str_len(argument)==-1)
		return;

	if (IS_IMMORTAL(ch))
	{
		argument = one_argument( argument, arg1 );

		if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( argument ))
		{
			ch->println("Syntax: rpsheet hair <player> <colour>");
			return;
		}

		if (( victim = get_char_world( ch, arg1 )) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		if ( IS_NPC( victim ))
		{
			ch->println("Players only.");
			return;
		}

		if ( str_len(argument) > 12)
		{
			ch->println("Colour must be 12 characters or less.");
			return;
		}

		free_string( victim->pcdata->haircolour );
        victim->pcdata->haircolour = str_dup( argument );
		ch->printlnf("%s's hair colour has been changed to %s.",
			victim->name, argument );
		victim->printlnf("Your hair colour has been changed to %s.",
			argument );
		return;
	}

/*	if ( IS_LETGAINED( ch ))
	{
		ch->println("You cannot set your hair colour once you have been letgained.");
		ch->println(" Send a note to rp and support if you really need it changed.");
		return;
	}
*/
	if ( IS_NULLSTR( argument ))
	{
		ch->println("Syntax: rpsheet hair <colour>");
		return;
	}

	if ( str_len(argument) > 12)
	{
		ch->println("Colour must be 12 characters or less.");
		return;
	}

	free_string( ch->pcdata->haircolour );
	ch->pcdata->haircolour = str_dup( argument );
	ch->printlnf("Your hair colour is now %s.\r\nBe aware that you cannot set this after being letgained.",
		argument );
	return;
}

/**************************************************************************/
void do_eyecolour( char_data *ch, char *argument )
{
	char arg1[MIL];
	char_data	*victim;

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	if(c_str_len(argument)==-1)
		return;

	if (IS_IMMORTAL(ch))
	{
		argument = one_argument( argument, arg1 );

		if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( argument ))
		{
			ch->println("Syntax: rpsheet eyes <player> <colour>");
			return;
		}

		if (( victim = get_char_world( ch, arg1 )) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		if ( IS_NPC( victim ))
		{
			ch->println("Players only.");
			return;
		}

		if ( str_len(argument) > 12)
		{
			ch->println("Colour must be 12 characters or less.");
			return;
		}

		free_string( victim->pcdata->eyecolour );
        victim->pcdata->eyecolour = str_dup( argument );
		ch->printlnf("%s's eye colour has been changed to %s.",
			victim->name, argument );
		victim->printlnf("Your eye colour has been changed to %s.",
			argument );
		return;
	}

/*	if ( IS_LETGAINED( ch ))
	{
		ch->println("You cannot set your eye colour once you have been letgained.");
		ch->println(" Send a note to rp and support if you really need it changed.");
		ch->println(" This is unlikely to be changed unless there is a spectacular");
		ch->println(" and cataclysmic character rp event.");
		return;
	}
*/
	if ( IS_NULLSTR( argument ))
	{
		ch->println("Syntax: rpsheet eyes <colour>");
		return;
	}

	if ( str_len(argument) > 12)
	{
		ch->println("Colour must be 12 characters or less.");
		return;
	}

	free_string( ch->pcdata->eyecolour );
	ch->pcdata->eyecolour = str_dup( argument );
	ch->printlnf("Your eye colour is now %s.\r\nBe aware that you cannot set this after being letgained.",
		argument );
	return;
}

/**************************************************************************/
void do_height( char_data *ch, char *argument )
{
	char		arg1[MIL];
	char_data	*victim;
	int			value;

	argument = one_argument( argument, arg1 );

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	if (IS_IMMORTAL(ch))
	{
		if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( argument ))
		{
			ch->println("Syntax: rpsheet height <player> <inches>");
			ch->println("  Imms can go beyond the regular racial limitations,");
			ch->println("  but make sure it is realistic, we don't want to go,");
			ch->println("  back and change it once it's set.");
			ch->println("  `=Chelp RACIAL-WEIGHT/HEIGHT`x");
			return;
		}

		if (( victim = get_char_world( ch, arg1 )) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		if ( IS_NPC( victim ))
		{
			ch->println("Players only.");
			return;
		}

		value = atoi(argument);

        victim->pcdata->height = value;
		ch->printlnf("%s's height been changed to %d inches.",
			victim->name, value );
		victim->printlnf("Your height has been changed to %d inches.",
			value );
		return;
	}

/*	if ( IS_LETGAINED( ch ))
	{
		ch->println("You cannot set your height once you have been letgained. Send");
		ch->println(" a note to rp and support if you really need it changed.");
		ch->println(" Your reason must have a strong IC basis.");
		return;
	}
*/
	if ( IS_NULLSTR( arg1 ))
	{
		ch->println("Syntax: rpsheet height <inches>");
		return;
	}

	value = atoi( arg1 );
	
	if ( value < race_table[ch->race]->min_height
	||   value > race_table[ch->race]->max_height )
	{
		ch->printlnf("Invalid height value.  Choose a number between %d and %d.",
			race_table[ch->race]->min_height,
			race_table[ch->race]->max_height);
		ch->println(" If you feel your character should be outside of these values,");
		ch->println(" send a note to support and to rp explaining your reasons.");
		return;
	}

	ch->pcdata->height = value;
	ch->printlnf("Your height is now %d inches.\r\nBe aware that you cannot set this after being letgained.",
		value );
	return;
}

/**************************************************************************/
void do_weight( char_data *ch, char *argument )
{
	char		arg1[MIL];
	char_data	*victim;
	int			value;

	argument = one_argument( argument, arg1 );

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}


	if (IS_IMMORTAL(ch))
	{
		if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( argument ))
		{
			ch->println("Syntax: rpsheet weight <player> <pounds>");
			ch->println("  Imms can go beyond the regular racial limitations,");
			ch->println("  but make sure it is realistic, we don't want to go,");
			ch->println("  back and change it once it's set.");
			ch->println("  `=Chelp RACIAL-WEIGHT/HEIGHT`x");
			return;
		}

		if (( victim = get_char_world( ch, arg1 )) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		if ( IS_NPC( victim ))
		{
			ch->println("Players only.");
			return;
		}

		value = atoi(argument);

        victim->pcdata->weight = value;
		ch->printlnf("%s's weight been changed to %d pounds.",
			victim->name, value );
		victim->printlnf("Your weight has been changed to %d pounds.",
			value );
		return;
	}

/*	if ( IS_LETGAINED( ch ))
	{
		ch->println("You cannot set your weight once you have been letgained. Send");
		ch->println(" a note to rp and support if you really need it changed.");
		ch->println(" Your reason must have a strong IC basis.");
		return;
	}
*/
	if ( IS_NULLSTR( arg1 ))
	{
		ch->println("Syntax: rpsheet weight <pounds>");
		return;
	}

	value = atoi( arg1 );
	
	if ( value < race_table[ch->race]->min_weight
	||   value > race_table[ch->race]->max_weight )
	{
		ch->printlnf("Invalid weight value.  Choose a number between %d and %d.",
			race_table[ch->race]->min_weight,
			race_table[ch->race]->max_weight);
		ch->println(" If you feel your character should be outside of these values,");
		ch->println(" send a note to support and to rp explaining your reasons.");
		return;
	}

	ch->pcdata->weight = value;
	ch->printlnf("Your weight is now %d pounds.\r\nBe aware that you cannot set this after being letgained.",
		value );
	return;
}

/**************************************************************************/
void do_traits( char_data *ch, char *argument )
{
	char		arg1[MIL];
	char_data	*victim;
	int			value;

	if(c_str_len(argument)==-1)
		return;

	argument = one_argument( argument, arg1 );

	if ( IS_NPC( ch ))
	{
		ch->println("Players only.");
		return;
	}

	if ((IS_IMMORTAL(ch) || IS_RP_SUPPORT(ch)))
	{
		char		arg2[MIL];

		if ( IS_NULLSTR( arg1 ) || IS_NULLSTR( argument ))
		{
			ch->println("Syntax: rpsheet trait <player> <1-9> <trait>");
			return;
		}

		argument = one_argument( argument, arg2 );
		value = atoi( arg2 );

		if (( victim = get_char_world( ch, arg1 )) == NULL )
		{
			ch->println("They aren't here.");
			return;
		}
		if ( IS_NPC( victim ))
		{
			ch->println("Players only.");
			return;
		}

		if ( value < 1 || value > 9 )
		{
			ch->println("Syntax: `=Crpsheet trait <player> <1-9> <trait>`x");
			return;
		}

		if ( str_len(argument) > 60 )
		{
			ch->println("Please keep the trait under 60 characters.");
			return;
		}

		free_string( victim->pcdata->trait[value-1] );
        victim->pcdata->trait[value-1] = str_dup( argument );
		ch->printlnf("%s's trait #%d is `y%s`x.",
			victim->name, value, argument );
		victim->printlnf("Your trait #%d is now `y%s`x.",
			value, argument );
		return;
	}

/*	if ( IS_LETGAINED( ch ))
	{
		ch->println("You cannot set your traits once you have been letgained.");
		ch->println(" Send a note to imm and rpsupport if you really need it changed.");
		ch->println(" Please include some good RP reasons to reflect this change.");
		return;
	}
*/

	value = atoi( arg1 );

	if ( value < 1 || value > 9 )
	{
		ch->println("Syntax: `=Crpsheet trait <1-9> <trait>`x");
		return;
	}


	if ( str_len( argument ) > 60 )
	{
		ch->print("Please keep your trait description under 60 characters.r\n");
		return;
	}


	free_string( ch->pcdata->trait[value-1] );
	ch->pcdata->trait[value-1] = str_dup( argument );
	ch->printlnf("Your trait #%d now `y%s`x.\r\nBe aware that you cannot change this after being letgained.",
		value, argument );
	return;
}

/**************************************************************************/
void extract_char_from_char_list(char_data *ch);
/**************************************************************************/
void do_rpsheet( char_data *ch, char *argument )
{
	char arg[MIL];
	char arg1[MIL];
	char buf[MIL];
	char_data *victim;

	if(c_str_len(argument)==-1){
		ch->println("no newlines colour codes can be used with the rpsheet command.");
		return;
	}

	argument = one_argument( argument, arg1 );
	sprintf( buf, "%s", argument );
	argument = one_argument( argument, arg );

	if(!IS_NULLSTR(arg1)){
		if(!str_prefix( arg1, "view" )) 		
		{ 
			// immortal version of rpsheet - can view offline players
			if(IS_IMMORTAL(ch))
			{
				if(IS_NULLSTR(arg)){
					show_rpsheet(ch, ch);
					return;
				}

				victim=get_whovis_player_world( ch, arg );
				if(victim){ 
					// redirect them to the online player
					show_rpsheet(ch, victim);
					return;
				}

				connection_data d;
				memset(&d, 0, sizeof(d));
				d.make_connected_socket_invalid();

				// attempt to load the pfile
				bool pfile_loaded_okay= load_char_obj( &d, arg);

				if(!pfile_loaded_okay || !d.character){
					ch->printlnf("Couldn't load pfile '%s'.", arg);
					return;
				}

				// pfile loaded, clean up the pointer relationship
				victim=d.character;
				victim->desc=NULL;
				if(victim->pet){
					// remove pets from the char_list 
					extract_char_from_char_list(victim->pet); 
				}

				if(str_cmp(ch->name, victim->name)){
					ch->println("Player is offline, displaying offline rpsheet:");
				}

				// display the rpsheet
				show_rpsheet(ch, victim);

				// deallocate the loaded character data
				if(victim->pet){
					free_char( victim->pet );
				}
				free_char( victim);
				return;
			}

			// rp support can view any online players
			if(IS_RP_SUPPORT(ch) && !IS_NULLSTR(arg))
			{
				if (( victim = get_char_world( ch, arg )) == NULL )
				{
					ch->println("They aren't here.");
					return;
				}
				if ( IS_NPC( victim ))
				{
					ch->println("Players only!");
					return;
				}
			}else{
				victim=ch;
			}
   			show_rpsheet(ch, victim);
			return;
		}

		if(!str_prefix( arg1, "crest" ))
		{
			do_crest( ch, buf );
			return;
		}

		if (!str_prefix( arg1, "surname" ))
		{
			do_surname( ch, buf );
			return;
		}

		if (!str_prefix( arg1, "hair" ))
		{
			do_haircolour( ch, buf );
			return;
		}

		if (!str_prefix( arg1, "eyes" ))
		{
			do_eyecolour( ch, buf );
			return;
		}

		if (!str_prefix( arg1, "height" ))
		{
			do_height( ch, buf );
			return;
		}

		if (!str_prefix( arg1, "weight" ))
		{
			do_weight( ch, buf );
			return;
		}

		if (!str_prefix( arg1, "birthplace" ))
		{
			do_birthplace( ch, buf );
			return;
		}

		if (!str_prefix( arg1, "trait" ))
		{
			do_traits( ch, buf );
			return;
		}
	}

	if (IS_IMMORTAL(ch))
	{
		ch->println("`YRPSHEET COMMANDS\r\n");
		ch->println("  `Bview        `x<player>   (note: player can be offline)");
		ch->println("  `Beyes        `x<player> <colour>");
		ch->println("  `Bhair        `x<player> <colour>");
		ch->println("  `Bheight      `x<player> <inches>");
		ch->println("  `Bweight      `x<player> <pounds>");
		ch->println("  `Bbirthplace  `x<player> <location>");
		ch->println("  `Bsurname     `x<player> <name>");
		ch->println("  `Btrait       `x<player> <1-9> <trait>");
		ch->println("  `Bcrest       `x<player> <symbol>");
		return;
	}

	if ( IS_RP_SUPPORT( ch ))
	{
		ch->println("`YRPSHEET COMMANDS\r\n");
		ch->println("  `Bview    `x<player>");
		ch->println("  `Bsurname `x<player>");
		ch->println("  `Bcrest   `x<player> <symbol>");
		ch->println("  `Btrait   `x<player> <1-9> <trait>");
		return;
	}

	ch->println("`YRPSHEET COMMANDS\r\n");
	ch->println("  `Bview       `x- displays your rpsheet");
	ch->println("  `Beyes       `x- sets the colour of your eyes");
	ch->println("  `Bhair       `x- sets the colour of your hair");
	ch->println("  `Bheight     `x- sets your height in inches");
	ch->println("  `Bweight     `x- sets your weight in pounds");
	ch->println("  `Bbirthplace `x- sets the place of your birth");
	ch->println("  `Bsurname    `x- sets your last name");
	ch->println("  `Btrait      `x- sets your physical and personality traits");
	ch->println("  `Bcrest      `x- sets your character's family symbol");
	ch->println("  `B           `x- once crest is set, it cannot be changed! get it right");
	ch->println("  `B           `x- send a note to rpsupport and/or imm to get it changed");
}
/**************************************************************************/
void show_rpsheet( char_data *ch, char_data *v)
{
	int dsec, icyear, i;
	int feet, inches;
    double cm, kg;

	dsec	= abs((int)(v->pcdata->birthdate - current_time)); // number of IRL seconds between the 2 times
	icyear	= (dsec/ICTIME_IRLSECS_PER_YEAR)+v->pcdata->birthyear_modifier;

	if ( !v->pcdata->height )
		v->pcdata->height = race_table[v->race]->min_height;
	
	if ( !v->pcdata->weight )
		v->pcdata->weight = race_table[v->race]->min_weight;


	inches	= v->pcdata->height % 12;
	feet	= v->pcdata->height / 12;
	cm		= v->pcdata->height * 2.54;
	kg		= v->pcdata->weight * .454;

	ch->println("`b _____________________________________________________________________________");
	ch->println("`b|\\`B___________________________________________________________________________`b/|");
	ch->println("`b|`B|                                                                           `B|`b|");
	ch->printlnf("`b|`B|`x  Name: `W%-60s       `B|`b|",capitalize(v->name));

	ch->printlnf("`b|`B|`x  Surname: `W%-20s          `xBirthplace: `r%-20s  `B|`b|",
		TRUE_CH(v)->pcdata->surname ? TRUE_CH(v)->pcdata->surname : "(none)" ,
		TRUE_CH(v)->pcdata->birthplace ? TRUE_CH(v)->pcdata->birthplace : "(unset)" );
	if(v->pcdata->birthdate){
		ch->printlnf("`b|`B|`x  Born:  `y%s `x  Age:  `y%-44s  `B|`b|",
			str_width( get_shorticdate_from_time(v->pcdata->birthdate, 
						"`#%d`S/`^%d`S/`&%d", v->pcdata->birthyear_modifier),11,false),
			get_ictimediff(v->pcdata->birthdate, current_time, -v->pcdata->birthyear_modifier));
		
	}else{
		ch->printlnf("`b|`B|`x  Born:  `y%s `x  Age:  `y%-44s  `B|`b|",
			str_width(get_shorticdate_from_time(
				v->player_id - (17*ICTIME_IRLSECS_PER_YEAR), 
					"`#%d`S/`^%d`S/`&%d", 0),11,false),
		get_ictimediff(v->player_id-(17*ICTIME_IRLSECS_PER_YEAR), 
			current_time, 0));
	}	
	ch->println("`b|`B|                                                                           `B|`b|");

	if ( HAS_CONFIG( ch, CONFIG_METRIC ))
	{
		ch->printlnf("`b|`B|`x  Hair: `W%-12s`x   Eyes: `W%-12s`xHeight: `W%d cm    `xWeight: `W%-4dkg  `B|`b|",
			TRUE_CH(v)->pcdata->haircolour ? capitalize(TRUE_CH(v)->pcdata->haircolour ) : "(unset)",
			TRUE_CH(v)->pcdata->eyecolour ? capitalize(TRUE_CH(v)->pcdata->eyecolour ) : "(unset)",
			(int)cm,
			(int)kg );
	}
	else
	{
		ch->printlnf("`b|`B|`x  Hair: `W%-12s`x   Eyes: `W%-12s`xHeight: `W%d'%d\"    `xWeight: `W%-4d lbs  `B|`b|",
			TRUE_CH(v)->pcdata->haircolour ? capitalize(TRUE_CH(v)->pcdata->haircolour ) : "(unset)",
			TRUE_CH(v)->pcdata->eyecolour ? capitalize(TRUE_CH(v)->pcdata->eyecolour ) : "(unset)",
			feet,
			inches,
			v->pcdata->weight );
	}
	ch->printlnf("`b|`B|`x  Family Crest: `W%-40s                   `B|`b|",
		TRUE_CH(v)->pcdata->crest ? capitalize(TRUE_CH(v)->pcdata->crest) : "(none)" );
	ch->println("`b|`B|___________________________________________________________________________`B|`b|");
	ch->println("`b|`B|                                                                           `B|`b|");	
	ch->println("`b|`B| `CPersonality Traits                                                        `B|`b|");
	ch->println("`b|`B|                                                                           `B|`b|");	
	for ( i=0; i < 6; i++ )
	{
		ch->printlnf("`b|`B| `C%d. `W%-60s           `B|`b|",
		i+1,
		v->pcdata->trait[i] ? capitalize( v->pcdata->trait[i] ) : "" );
	}
	ch->println("`b|`B|                                                                           `B|`b|");
	ch->println("`b|`B|___________________________________________________________________________`B|`b|");
	ch->println("`b|`B|                                                                           `B|`b|");
	ch->println("`b|`B| `CPhysical Traits                                                           `B|`b|");
	ch->println("`b|`B|                                                                           `B|`b|");
	for ( i=6; i < 9; i++ )
	{
		ch->printlnf("`b|`B| `C%d. `W%-60s           `B|`b|",
		i+1,
		v->pcdata->trait[i] ? capitalize( v->pcdata->trait[i] ) : "" );
	}
	ch->println("`b|`B|___________________________________________________________________________`B|`b|");
	ch->println("`b|/___________________________________________________________________________`b\\|");
	return;


}

/**************************************************************************/
void do_rptell( char_data *ch, char *argument )
{
	char arg[MIL], buf[MSL];
    char_data *nch, *victim, *nvictim;

    // unswitched mobs can't send rptells
    if (IS_UNSWITCHED_MOB(ch))
    {
		return;
    }

	if (( !IS_IMMORTAL(ch) || !IS_RP_SUPPORT(ch)))
	{
		do_huh(ch,"");
		return;
	}

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}

    argument = one_argument( argument, arg );

	if ( arg[0] == '\0' || argument[0] == '\0' )
	{
		ch->println("RP tell whom what?");
		return;
	}

	if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL
		||  IS_NPC(victim) ) 
	{
		ch->println( "They aren't here." );
		return;
	}
	//Put in these lines from do_tell in because it made some sense. <shrug>
	if (victim->controlling && victim->controlling->desc)
	{
		victim=victim->controlling;
	}
	
	if(ch==victim)
	{
		ch->println( "No point in sending an rptell to yourself." );
		return;
	};

	//Also from do_tell, looked good to put.
	if ( victim->desc == NULL)
	{
		act("$N seems to have misplaced $S link...try again later.", ch,NULL,victim,TO_CHAR);
		sprintf(buf,"An RP support member tells you '%s'\r\n`x", argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
	}

	if (IS_SET(victim->comm,COMM_AFK))
	{
		if (IS_NPC(victim))
		{
			act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
			return;
		}

		act("$E is AFK, but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR);
		
		sprintf(buf,"An RP support member tells you '%s'\r\n", argument );
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
    
		victim->printlnf( "An RP support member just tried to tell you '%s' (you are marked as afk)`x", argument);
		return;
	}

	if(!IS_NPC(ch))
		ch->pcdata->did_ooc=true;

	//Send the tell to the player.
	victim->printlnf("`y%s tells you '%s'"
		"(you can use rpreply to talk back if you need to)`x\r\n", 
		IS_IMMORTAL(victim)?PERS(ch,victim):"An RP support member", argument );

	//Start the broadcast of the rptell to all rpsupport
	for ( nch = player_list; nch; nch = nch->next_player )
	{
		if (TRUE_CH(ch)==nch)
		{
			ch->printlnf( "`MYou tell %s '%s`M'", victim->name, argument);
			continue;
		}

		// don't see both the rp broadcast and the personal message
		if(	nch==victim)
		{
			continue;
		};

		nvictim=nch;
		// rptalk going thru switch
		if (nvictim->controlling && nvictim->controlling->desc)
			nvictim=nvictim->controlling;

		if (IS_RP_SUPPORT(nch))
		{
			if (IS_IMMORTAL(nch))
			{
				nvictim->printlnf( "`M<%s rptells %s>: '%s`M'`x", 
					TRUE_CH(ch)->name, victim->name, argument);
			}else{
				nvictim->printlnf( "`M<An RP support member tells %s>: '%s`M'`x", victim->name, argument);
			}
		}
	}
	return;
}
/**************************************************************************/
void do_rpreply( char_data *ch, char *argument )
{
	char_data *nch, *victim;

    // unswitched mobs can't send nreplies
    if (IS_UNSWITCHED_MOB(ch))
    {
		return;
    }

    if ( IS_NULLSTR(argument) )
    {
        ch->println("RPreply what?");
        return;
    }

	//Start the broadcast of the rpreply to all rpsupport
    for ( nch = player_list; nch; nch = nch->next_player )
    {
		if (TRUE_CH(ch)==nch)
		{
			ch->printlnf( "`MYou rpreply '%s`M'", argument);
			continue;
		}

		victim=nch;

		// rpreply going thru switch
		if (victim->controlling && victim->controlling->desc)
			victim=victim->controlling;

		if ( IS_RP_SUPPORT(nch))
		{
			victim->printlnf( "`M<%s rpreplies> '%s`M'`x",
					TRUE_CH(ch)->name, argument);
		}
	}
	return;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
