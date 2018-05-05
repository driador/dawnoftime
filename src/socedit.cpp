/**************************************************************************/
// socedit.cpp - olc based social editor, Kalahn
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "olc.h"
#include "security.h"
#include "socials.h"

/**************************************************************************/
void social_add_sorted_to_list(social_type *soc, bool replace);
/**************************************************************************/
//	Entry Point for editing the socials 
void do_socialedit( char_data *ch, char *argument )
{
	if ( IS_NPC( ch )){
		ch->println("Players only.");
		return;
	}

	// do security checks
	if (!HAS_SECURITY(ch, SOCEDIT_MINSECURITY))
	{
    	ch->printf("You must have an olc security %d or higher to use this command.\r\n",
			SOCEDIT_MINSECURITY);
		return;
	}

	if ( !IS_TRUSTED(ch, SOCEDIT_MINTRUST)) {
		ch->printf("You must have a trust of %d or above "
			"to use this command.\r\n", SOCEDIT_MINTRUST);
		return;
	}

	if (IS_NULLSTR(argument)){
		ch->println("syntax: socedit <social>");
		ch->println("syntax: socedit create <newsocial>");
		ch->println("This command takes you into an olc editor, to edit/create socials");
		return;
	}
	
	char arg[MIL];
	social_type *soc;
	argument=one_argument(argument,arg);

	if(!str_cmp(arg,"create")){
		if(IS_NULLSTR(argument)){
			ch->println("You must specify the name of the social you wish to create.");
			return;
		}

		// check if there is an existing social with matching name
		soc=find_social(ch, arg);
		if(soc){
			ch->printlnf("There is already a social named '%s'", soc->name);
			return;
		};
		soc=new social_type;
		soc->name=str_dup(lowercase(argument));
		SET_BIT(soc->social_flags,SOC_IMM_ONLY);
		social_add_sorted_to_list(soc, false);
		ch->printlnf("Social '%s' created - currently flagged as imm only", argument);
		social_count++;
		do_socialedit(ch,argument);
		return;
	}

	// find an existing social
	soc=find_social(ch, arg);
	if(!soc){
		ch->printlnf("There is no social named '%s'", arg);
		return;
	};

    ch->desc->pEdit	= (void*)soc;
	ch->desc->editor = ED_SOCIAL;
	ch->printlnf("Editing '%s' social.", soc->name);
	return;
}

#define EDIT_SOCIAL(ch, soc) ( soc= (social_type *)ch->desc->pEdit )
/**************************************************************************/
bool socedit_name(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax: name <string>");
		ch->println( "Sets the name of the social.");
		return false;
    }

	social_type *soc;
	EDIT_SOCIAL(ch,soc);

    ch->printf("Social name changed from '%s' to '%s'.\r\n", 
		soc->name, lowercase(argument) );

    replace_string( soc->name, lowercase(argument) );
    return true;
}
/**************************************************************************/
bool socedit_show(char_data *ch, char *)
{
	social_type *soc;
	EDIT_SOCIAL(ch,soc);

	SET_BIT(ch->dyn,DYN_SHOWFLAGS);
	ch->titlebarf("SOCIAL EDIT: %s", uppercase(soc->name));
  	ch->printlnf("`=rName: `x%s", capitalize(soc->name));
	mxp_display_olc_flags(ch, social_flags,	soc->social_flags,		"socialflags",	"Social Flags:");
	mxp_display_olc_flags(ch, position_flags,	soc->position_flags,"positionflags","Allowed Positions:");

	ch->titlebar("ACTS: NO TARGET/PARAMETER");
	ch->printlnf("1. Seen by Self:   %s",	soc->acts[SOCIAL_ATNOTARGET_MSG2SELF]);
	ch->printlnf("2. Seen by Others: %s",	soc->acts[SOCIAL_ATNOTARGET_MSG2OTHERS]);

	ch->titlebar("ACTS: DIRECTED TO SELF");
	ch->printlnf("3. Seen by Self:   %s",	soc->acts[SOCIAL_ATSELF_MSG2SELF]);
	ch->printlnf("4. Seen by Others: %s",	soc->acts[SOCIAL_ATSELF_MSG2OTHERS]);

	ch->titlebar("ACTS: DIRECTED AT A TARGET");
	ch->printlnf("5. Seen by Self:   %s",	soc->acts[SOCIAL_ATTARGET_MSG2SELF]);
	ch->printlnf("6. Seen by Target: %s",	soc->acts[SOCIAL_ATTARGET_MSG2TARGET]);
	ch->printlnf("7. Seen by Others: %s",	soc->acts[SOCIAL_ATTARGET_MSG2OTHERS]);

	ch->titlebar("MOB RESPONSE");
	ch->printlnf("8. Mob response: %s",	soc->acts[SOCIAL_ATTARGET_MOBTARGETRESPONSE]);
	ch->println( "   (What a mob will do 1 second after a social has been directed at it)\r\n"
				 "    $N (players name) is the only valid $ code here");
	
	ch->titlebar("$ VARIABLES");
	ch->println( "    $n = short/name of player           $e = player he/she/it");
	ch->println( "    $m = player him/her/it              $s = player his/her/its");
	ch->println( "    Use uppercase codes for the target. ($N for targets name etc)");

	REMOVE_BIT(ch->dyn,DYN_SHOWFLAGS);
    return false;
}
/**************************************************************************/
// Kal - Feb 01
void do_socshow( char_data *ch, char *argument )
{	
    if ( IS_NULLSTR(argument) ){
		ch->println("Syntax:  socshow <social>");
		return;
    }

	// find an existing social
	social_type *soc=find_social(ch, argument);
	if(!soc){
		ch->printlnf("There is no social named '%s'", argument);
		return;
	};

	void * pTemp = ch->desc->pEdit;
	ch->desc->pEdit = (void *)soc;

	socedit_show( ch, "");
	ch->desc->pEdit = pTemp;
    return; 
}

/**************************************************************************/
bool socedit_socialflags(char_data *ch, char *argument)
{
	social_type *soc;
	EDIT_SOCIAL(ch,soc);
	return olc_generic_flag_toggle(ch, argument,
				"socialflags", "socialflags", social_flags, &soc->social_flags);
}
/**************************************************************************/
bool socedit_positionflags(char_data *ch, char *argument)
{
	social_type *soc;
	EDIT_SOCIAL(ch,soc);
	return olc_generic_flag_toggle(ch, argument,
				"positionflags", "positionflags", position_flags, &soc->position_flags);
}
/**************************************************************************/
bool socedit_act(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax: act <number> <string>");
		ch->println( "Sets the act string for a particular act number (as per socialedit show).");
		return false;
    }
	char numbertext[MIL];
	argument=first_arg(argument,numbertext, false);

	int number=atoi(numbertext);
	if(number<1 || number>SOCIAL_ATMAX){
		ch->printlnf("'%s' is not a number between 1 and %d",
			numbertext, SOCIAL_ATMAX);
		socedit_act(ch,"");
		return false;
	}

	social_type *soc;
	EDIT_SOCIAL(ch,soc);

    ch->printf("Social act %d changed from '%s' to '%s'.\r\n", 
		number, soc->acts[number-1], argument );

    replace_string( soc->acts[number-1], argument );
    return true;
}
/**************************************************************************/
// Daos - Oct 03
bool socedit_sdelete( char_data *ch, char *argument )
{
	social_type *pSoc, *soc;
	
	if(str_cmp("confirm", argument)){
		ch->println("Type `=Csdelete confirm`x to remove this social.");
		return false;
	}
	
	EDIT_SOCIAL( ch, pSoc);

	// check if anyone else is editing the social
	for(connection_data *c=connection_list; c; c=c->next){
		if(c!=ch->desc && c->pEdit==(void *)pSoc){
			ch->println("Someone else is currently editing it, so it can't currently be deleted.");
			return false;
		}
	}

	
	if ( social_list == pSoc ){
		ch->printlnf("'%s' social deleted.", pSoc->name);
		logf("'%s' social deleted by %s.", pSoc->name, PERS(ch, NULL));
		social_list = pSoc->next;
		free(pSoc);
		edit_done ( ch );
		social_count--;
		save_socials();
		return true;
	}
	
	for ( soc = social_list; soc; soc = soc->next )
	{
		if ( soc->next == pSoc )
		{
			ch->printlnf("'%s' social deleted.", pSoc->name);
			logf("'%s' social deleted by %s.", pSoc->name, PERS(ch, NULL));
			soc->next = pSoc->next;
			free(pSoc);
			edit_done( ch );
			social_count--;
			save_socials();
			return true;
		}
	}
	ch->printlnf("For some strange reason the '%s' social couldn't be found in the list of socials to delete.", pSoc->name);
	bugf("'%s' social unfound in list for delete by %s.", pSoc->name, PERS(ch, NULL));

	return false;
}
/**************************************************************************/
/**************************************************************************/


