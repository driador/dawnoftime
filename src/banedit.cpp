/**************************************************************************/
// banedit.cpp
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "ban.h"
#include "olc.h"

extern BAN_DATA *ban_list;
int wild_match(register unsigned char *m, register unsigned char *n);

// prototypes
bool banedit_show (char_data *ch, char *);
void save_bans(void);


extern const struct olc_cmd_type banedit_table[];

/**************************************************************************/
// returns a pointer to the ban number as addressed by 'value'
BAN_DATA *get_ban_index( int value ) 
{
    BAN_DATA *pBan;
    for (pBan = ban_list; pBan != NULL; pBan = pBan->next)
    {
		if (--value<0){
			return pBan;
		}
	}
	return NULL;
}
/**************************************************************************/
void do_banedit( char_data *ch, char *argument )
{
    BAN_DATA *pBan;
    char arg1[MIL];
    int value;

	// must have some pcdata to use the command
	if(!TRUE_CH(ch)->pcdata){
		do_huh(ch,"");
		return;
	}

    if (!IS_TRUSTED(ch,MAX_LEVEL-2) 
		&& !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADLAW))
	{
        ch->println( "Banedit is for law and high level admin only." );
        return;
    }

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( !( pBan= get_ban_index( value ) ) )
        {
            ch->printf("Banedit:  There is no ban numbered '%d'\n", value);
            return;
        }
    
        ch->desc->pEdit = (void *)pBan;
        ch->desc->editor = ED_BAN;
		ch->println( "`=rEditing Ban... type `=Cdone`=r to finish editing." );
		banedit_show(ch,"");
        return;
    }
    ch->print("BanEdit:  Type the number of the ban you want to edit.\n");
    return;
}

/**************************************************************************/
void banedit(char_data *ch, char *argument)
{
//    BAN_DATA *pBan;
    char arg[MSL];
    char command[MIL];
    int  cmd;

/*	if (!HAS_SECURITY(ch,1))
	{
		ch->println( "The banedit command is an olc command, "
			"you don't have olc permissions." );
		edit_done( ch );
		return;
	}
*/
    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

//    BAN_EDIT(ch, pBan);

    if ( !str_cmp(command, "done") )
    {
		edit_done( ch );
		return;
    }

/*	if (!HAS_SECURITY(ch,pHelp->helpfile->security))
	{
		do_modehelp(ch, argument);
		ch->println( "Insufficient security to edit this help entry." );
		edit_done( ch );
		return;
	}
*/
    if ( command[0] == '\0' )
    {
		banedit_show( ch, argument );
		return;
    }

    // Search Table and Dispatch Command.
    for ( cmd = 0; banedit_table[cmd].name != NULL; cmd++ )
    {
        if ( !str_prefix( command, banedit_table[cmd].name ) )
        {
            if ( (*banedit_table[cmd].olc_fun) ( ch, argument ) )
            {
			//	bans_changed=true;
				save_bans();
				return;
            }
            else
            return;
        }
    }

    // Default to Standard Interpreter.
    interpret( ch, arg );
    return;
}

/**************************************************************************/
bool banedit_show (char_data *ch, char *){
    ban_data *pBan;

    EDIT_BAN(ch, pBan);

	ch->titlebar("BANEDIT SHOW");
	ch->print("`=rSitemask:    '`=R");
	ch->printbw(pBan->sitemasks);

	if (IS_NULLSTR(pBan->intended_people)){
		ch->print("`=r'\nIntendedPeople: `Rnone\n");
	}else{
		ch->printf("`=r'\nIntendedPeople: `=R%s\n", 
			pBan->intended_people);
	}

	ch->printf("`=rEnabled: `=R%s\n", pBan->enabled?"True":"False");
	ch->printf("`=rPermanent: `=R%s\n", pBan->permanent?"True":"False");
	if(pBan->type==BAN_UNDEFINED){
		ch->printf("`=rType: `R%s!\n", flag_string( ban_types, pBan->type ) );
	}else{
		ch->printf("`=rType: `=R%s\n", flag_string( ban_types, pBan->type ) );
	}
	ch->printf("`=rban created on: `S%s`=rby: `S%s`x\n", (char*)ctime(&pBan->ban_date), pBan->by);
	
	if (IS_NULLSTR(pBan->reason)){
		ch->print("`=rReason: `Rnone\n");
	}else{
		ch->printf("`=rReason: `=R%s", 
			pBan->reason);
	}

	if (IS_NULLSTR(pBan->custom_disconnect_message)){
		ch->print("`=rCustom disconnect message: `Snone\n");
	}else{
		ch->printf("`=rCustom disconnect message: `=R\n%s", 
			pBan->custom_disconnect_message);
	}
		
	ch->printf("`=rExpire date: `=R%s", pBan->expire_date<1?
		"none\n":(char*)ctime(&pBan->expire_date));

	if(pBan->type==BAN_EMAIL_REQ)
	{
		// allowed email masks
		if (IS_NULLSTR(pBan->always_allowed_email_masks)){
			ch->println("`=ralways_allowed_email_masks: `xnone.");
		}else{
			ch->printf("`=ralways_allowed_email_masks: `=R\r\n%s", 
				pBan->always_allowed_email_masks);
		}		

		// allowed email masks
		if (IS_NULLSTR(pBan->allowed_email_masks)){
			ch->println("`=rallowed_email_masks: `RNONE!");
		}else{
			ch->printf("`=rallowed_email_masks: `=R\r\n%s", 
				pBan->allowed_email_masks);
		}		

		// disallowed email masks
		if (IS_NULLSTR(pBan->disallowed_email_masks)){
			ch->println("`=rdisallowed_email_masks: `Rnone");
		}else{
			ch->printf("`=rdisallowed_email_masks: `=R\r\n%s", 
				pBan->disallowed_email_masks);
		}
	
		ch->wrapln( "`=rnote: disallowed_email_masks have a higher priority "
			"than allowed masks, if a particular email addy matches the disallowed email "
			"mask, it will be denied even if it matches the allowed mask also.  "
			"The only exception to this is if the email address is on the "
			"always_allowed_email_masks list for this ban... note the always allowed "
			"only applies to this particular ban... if the host and email match a disallowed "
			"on another ban, it will be declined.`x");

		// disallowed email custom message
		if (IS_NULLSTR(pBan->disallowed_email_custom_message)){
			ch->println("`=rdisallowed_email_custom_message: `Snone - using default");
		}else{
			ch->printf("`=rdisallowed_email_masks_message: `=R\r\n%s", 
				pBan->disallowed_email_custom_message);
		}
	}
	ch->print("`x");
    return false;
};
/**************************************************************************/
bool banedit_type(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(IS_NULLSTR(argument)){
		ch->println( "syntax: type <bantype>" );
		ch->println( " were <bantype> is one of the following" );
		show_help(ch,"ban_types");
		return false;
	}

	int newvalue=flag_value( ban_types, argument );

	if(newvalue==NO_FLAG){
		ch->printf("No such ban type as '%s'\n", argument);
		banedit_type(ch,"");
		return false;
	}

	// check for no change
	if(pBan->type == newvalue){
		ch->printf("No change in ban type, ban type is still '%s'\n", 
			flag_string( ban_types, pBan->type ));
		return false;
	};

	// do the change
	ch->printf("bantype changed from '%s' to '%s'\n",
		flag_string( ban_types, pBan->type),
		flag_string( ban_types, newvalue));
	pBan->type = newvalue;
	return true;
}
/**************************************************************************/
bool banedit_sitemasks(char_data *ch, char *argument)
{
	if(IS_UNSWITCHED_MOB(ch)){
		ch->println( "Players only!" );
		return false;
	}
	
	if(!IS_NULLSTR(argument)){
		ch->println( "syntax: sitemasks <bansitemasks> (string editor used)" );
		ch->wrapln( 
			"This affects all bans:`1"
			"The host address of a player is matched against all site masks..."
			"(1 sitemask per line), read the helps on permban for more details.`1"
			"e.g. `1*.aol.com`1*.aol2.com`1192.168.50.*`1"
			"Would mean the ban applies to machines from the "
			"two domains names and the ip address range");
		return false;
	}
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	string_append( ch, &pBan->sitemasks);
	return true;
}
/**************************************************************************/
bool banedit_intendedpeople(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
		ch->println( "Syntax:  intendedplayer  (string editor used)" );
		ch->println( "uses the string editor to edit the list of intended people." );
		return false;
	}
	string_append( ch, &pBan->intended_people);
	return true;
}
/**************************************************************************/
bool banedit_reason(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
		ch->println( "Syntax:  reason  (string editor used)" );
		ch->println( "uses the string editor to edit/add the reason for the ban." );
		return false;
	}
	string_append( ch, &pBan->reason);
	return true;
}
/**************************************************************************/
bool banedit_disconnect_msg(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
		ch->println( "Syntax:  disconnect_msg  (string editor used)" );
		ch->wrapln( 
			"This message is displayed to someone when they "
			"are disconnected due to the ban.  If there is "
			"no custom disconnect message, the default for "
			"the type of ban is displayed.");
		return false;
	}
	string_append( ch, &pBan->custom_disconnect_message);
	return true;
}
/**************************************************************************/
bool banedit_always_allowed_email_masks(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
		ch->println( "Syntax:  always_allowed (string editor used)" );
		return false;
	}
	string_append( ch, &pBan->always_allowed_email_masks);
	return true;
}
/**************************************************************************/
bool banedit_allowed_email_masks(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
		ch->println( "Syntax:  allowed_email_masks (string editor used)" );
		ch->wrapln( 
			"Comes into affect when the type of ban is 'email'`1"
			"There can be multiple lines of email address formats "
			"that you accept (1 mask per line), read the helps on bans for more details.`1"
			"e.g. `1*@aol.com`1d?g@hotmail.com`1`1would allow someone to say their email "
			"is something at aol.com or a hotmail account with a 3 letter username, "
			"starting with d ending with g.");
		return false;
	}
	string_append( ch, &pBan->allowed_email_masks);
	return true;
}
/**************************************************************************/
bool banedit_disallowed_email_masks(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
		ch->println( "Syntax:  disallowed_email_masks (string editor used)" );
		ch->wrapln(
			"Comes into affect when the type of ban is 'email'`1"
			"There can be multiple lines of email address formats "
			"that you will be declined, regardless even if they pass under an "
			"accept email mask.  Read the helps on bans for more details.`1"
			"e.g. if an accept mask had a line that read '*@aol.com' and we "
			"knew fred@aol.com was bad, we would add fred@aol.com (or could "
			"do *fred*@aol.com) to the list of disallowed email masks.");
		return false;
	}
	string_append( ch, &pBan->disallowed_email_masks);
	return true;
}
/**************************************************************************/
bool banedit_disallowed_msg(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
		ch->println( "Syntax:  disallowed_msg  (string editor used)" );
		ch->wrapln(
			"This message is displayed to someone when they "
			"the email addy the supplied matches one on the "
			"disallowed email masks list.  If there is "
			"no custom disallowed email masks message, "
			"the default is displayed.");
		return false;
	}
	string_append( ch, &pBan->disallowed_email_custom_message);
	return true;
}
/**************************************************************************/
bool banedit_level (char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	int newlevel;
	
	if(IS_NULLSTR(argument) || !is_number( argument ) ){
	    ch->println( "Syntax:  level <number>" );
		return false;
    }

    newlevel= atoi( argument );

	// check the new level
	if(newlevel<LEVEL_IMMORTAL){
	    ch->printf("level must be greater than %d", LEVEL_IMMORTAL-1);
		return false;
	}
	if(newlevel>get_trust(ch)){
	    ch->printf("Level must less than or equal to your trust of %d", 
			get_trust(ch));
		return false;
	}

	// do the change
	ch->printf("ban level changed from %d to %d\n",
		pBan->level,newlevel);

	pBan->level= newlevel;
    return true;
}
/**************************************************************************/
bool banedit_permanent(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
	    ch->println( "Syntax:  permanent" );
	    ch->wrapln( "it simply toggled... If it isn't permanent "
			"it wont be saved, and will be gone next reboot.");
		return false;
    }

	if(pBan->permanent){
		ch->print("Ban set to temporary... wont be saved (gone next reboot).\n");
		pBan->permanent=false;
		return true;
	}

	ch->print("Ban set to permanent.\n");
	pBan->permanent=true;
	return true;
}
/**************************************************************************/
bool banedit_enabled(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	
	if(!IS_NULLSTR(argument)){
	    ch->println( "Syntax:  enabled" );
	    ch->wrapln( "it simply toggled... If it is not enabled"
			"it is still saved to disk, but not applied to players logging in.");
		return false;
    }

	if(pBan->enabled){
		ch->print("Ban disabled\n");
		pBan->enabled=false;
		return true;
	}

	ch->print("Ban enabled.\n");
	pBan->enabled=true;
	return true;
}
/**************************************************************************/
bool banedit_expire_date(char_data *ch, char *argument)
{
    BAN_DATA *pBan;
    EDIT_BAN(ch, pBan);
	int value;
	
	if(IS_NULLSTR(argument) || !is_number( argument)){
	    ch->println( "Syntax: expire 0 (to set for no expire)" );
	    ch->println( "        expire <number of days from today to expire>" );
		return false;
    }

    value = atoi( argument);
	if(value<1){
		if(pBan->expire_date==0){
			ch->println( "Ban is already set to not expire." );
			return false;
		}

		pBan->expire_date=0;
		ch->println( "Ban will no longer expire." );
		return true;
	}
	
	// set the number of days till expirey
	if(value>2000){
		ch->println( "A reasonable number of days is required." );
		return false;
	}

	pBan->expire_date=current_time + (value *60*60*24); // days
	ch->printf("Ban will now expire at %s system time.\n", 
		(char*)ctime(&pBan->expire_date)); 
	return true;
}
/**************************************************************************/
