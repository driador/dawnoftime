/**************************************************************************/
// gamble.cpp - Unfinished gambling system by Kerenos
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
#include "include.h"


// local prototypes

GAMBLE_FUN *gamble_lookup( const char *name );
DECLARE_GAMBLE_FUN( gamble_seven );

/**************************************************************************/
GAMBLE_FUN *gamble_lookup( const char *name )
{
	int game;
	
	for ( game = 0; !IS_NULLSTR( gamble_table[game].name); game++ ){
		if ( !str_prefix( name, gamble_table[game].name )){
			return gamble_table[game].gamble_fun;
		}
	}
	bugf("gamble_lookup(): Unfound gambling game '%s'", name);
		
	return NULL;
}

/*****************************************************************************/
// Name:		gamble_string
// Purpose:	Given a function, return the appropriate name.
// Called by:	gamble_name
char *gamble_string( GAMBLE_FUN *fun )
{
	int cmd;
	
	for ( cmd = 0; gamble_table[cmd].gamble_fun; cmd++ ){
		if ( fun == gamble_table[cmd].gamble_fun ){
			return gamble_table[cmd].name;
		}
	}

	bugf("gamble_string(): Unfound gambling function!");
	return "unknown";
}

/***************************************************************************/
char *gamble_name( GAMBLE_FUN *function)
{
	return gamble_string( function );
}

/**************************************************************************/
void do_gamble( char_data *ch, char *argument )
{
	char		original[MIL], arg[MIL];
	char_data	*dealer;

	if ( !IS_NULLSTR( argument ))
	{
		sprintf( original, "%s", argument );
		argument = one_argument( argument, arg );
	}
	else
	{
		ch->println("Type help gamble to see how it works.");
		return;
	}

	for( dealer = ch->in_room->people; dealer; dealer = dealer->next_in_room )
	{
        if ( IS_NPC( dealer )
		&& ( dealer->gamble_fun != 0 )
		&& dealer != ch )
		break;
	}

    if ( !dealer )
    {
		ch->println("There is no dealer present.");
        return;
    }

    if ( gamble_lookup( arg ) > 0 ){
	    dealer->gamble_fun( ch, dealer, argument );
	}else{
		ch->printlnf("No such game '%s'.", argument);
	}
	
    return;
}
/**************************************************************************/
void gamble_seven( char_data *ch, char_data *dealer, char *argument )
{
    char msg    [ MSL ];
    char wager  [ MIL ];
    char choice [ MIL ];
    char buf	[ MSL ];
	int  ichoice;
    int  amount;
    int	 limit = 100;
	int  die1;
    int  die2;
    int  total;

    argument = one_argument( argument, wager );
    one_argument( argument, choice );

    if ( wager[0] == '\0' || !is_number( wager ) )
    {
        ch->println("How many gold coins would you like to wager?");
        return;
    }

    amount = atoi( wager );

	if ( amount <= 0 )
	{
		ch->println("The wager must be a positive amount greater than zero.");
		return;
	}

    if ( amount > ch->gold )
    {
        ch->println("You don't have enough gold!");
        return;
    }

    if ( amount > limit )
    {
		ch->printlnf("`W%s says to you 'Sorry, the house limit is %d.`x'",
            dealer->short_descr, limit );
        return;
    }

		 if ( !str_cmp( choice, "under" ) ) ichoice = 1;
    else if ( !str_cmp( choice, "over"  ) ) ichoice = 2;
    else if ( !str_cmp( choice, "seven" ) ) ichoice = 3;
    else if ( !str_cmp( choice, "7"     ) ) ichoice = 3;
    else
    {
        ch->println("What do you wish to bet: Under, Over, or Seven?");
        return;
    }

	ch->printlnf("You place %d gold coins on the table, and wager '%s'.", amount, choice );

    act( "$n places a bet with you.", ch, NULL, dealer, TO_VICT );
	act( "$n places some coins on the table.", ch, NULL, NULL, TO_ROOM );

    ch->gold -= amount;

    die1 = number_range( 1, 6 );
    die2 = number_range( 1, 6 );
    total = die1 + die2;

	sprintf( buf, "%d and %d", die1, die2);
    sprintf( msg, "%s rolls the dice: they come up %d, and %d", dealer->short_descr, die1, die2 );
	act( "$n rolls the dice: they come up $t.", dealer, buf, ch, TO_NOTVICT );

    if( total == 7 )
    {
        strcat( msg, "." );
        act( msg, dealer, NULL, ch, TO_VICT );

        if ( ichoice == 3 )
        {
            amount *= 5;
            ch->printlnf("It's a SEVEN!  You win %d gold coins!", amount );
			act( "$n rejoices as $e wins some gold!!", ch, NULL, NULL, TO_ROOM );
            ch->gold += amount;
        }
        else
		{
            ch->println("It's a SEVEN!  You lose!");
			act( "$n mutters something as $e loses some coins!!", ch, NULL, NULL, TO_ROOM );
		}
		return;
    }

    ch->printlnf("%s, totalling %d.", msg, total );

    if ((( total < 7 ) && ( ichoice == 1 ))
        || (( total > 7 ) && ( ichoice == 2 )))
    {
        amount *= 2;
        ch->printlnf("You win %d gold coins!", amount );
		act( "$n cheers as $e wins a some coins!!", ch, NULL, NULL, TO_ROOM );
        ch->gold += amount;
    }
    else
	{
        ch->println("Sorry, better luck next time!");
		act( "$n loses $s wager!!", ch, NULL, NULL, TO_ROOM );
	}
	WAIT_STATE( ch, PULSE_VIOLENCE / 2 );
    return;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

