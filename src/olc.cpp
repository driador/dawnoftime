/**************************************************************************/
// olc.cpp - most of the core olc functions from olc release
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  File: olc.cpp                                                          *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 ***************************************************************************/
#include "include.h" // dawn standard includes

#include "olc.h"
#include "security.h"
#include "interp.h"
#include "immquest.h"
#include "socials.h"
#include "herbedit.h"
#include "socedit.h"
#include "langedit.h"
#include "help.h"

// prototypes
DECLARE_DO_FUN(do_look		);
ROOM_INDEX_DATA *	find_location	args( ( char_data *ch, char *arg ) );

// Local functions.
AREA_DATA *get_area_data	args( ( int vnum ) );
bool interpret_not_used;
/**************************************************************************/
bool run_olc_editor_for_connection(connection_data *d, char *command)
{
	if(!d){
		bugf("run_olc_editor_for_connection(): d==NULL!, command='%s'", command);
		return false;
	}

	// if the command starts with ' then it is a say, so we don't 
	// want to put it thru any olc editors
	if(!IS_NULLSTR(command) && command[0]=='\''){
		return false;
	}

	char_data *ch=d->character;

	assert(ch->desc==d); // there shouldn't be any time this isn't the case

	// interpret() sets interpret_not_used to false... using this method, 
	// we can tell if it was an olc command or a normal interpret command... 
	// if an olc command we may want to log it at the bottom of this function
	interpret_not_used=true; 

    switch ( d->editor )
    {
		case ED_AREA:
			aedit( ch, command);
			break;

		case ED_ROOM:
			redit( ch, command);
			break;

		case ED_OBJECT:
			oedit( ch, command);
			break;

		case ED_MOBILE:
			medit( ch, command);
			break;

		case ED_MPCODE:
    		mpedit( ch, command);
    		break;

		case ED_HELP:
    		hedit( ch, command);
    		break;

		case ED_BAN:
    		banedit( ch, command);
    		break;

		case ED_RACE:
			raceedit( ch, command);
			break;

		case ED_SPELLSKILL:
			sedit( ch, command);
			break;

		case ED_COMMAND:
			comedit( ch, command);
			break;

		case ED_DEITY:
			dedit( ch, command );
			break;

		case ED_QUEST:
			qedit( ch, command );
			break;

		case ED_CLASS:
			classedit( ch, command);
			break;

		case ED_GAME:
			gameedit( ch, command);
			break;

		case ED_SOCIAL:
			socialedit( ch, command);
			break;

		case ED_HERB:
			herbedit( ch, command);
			break;

		case ED_MIX:
			mixedit( ch, command );
			break;

		case ED_CLAN:
			clanedit( ch, command);
			break;

		case ED_SKILLGROUP:
			skillgroupedit( ch, command);
			break;

		case ED_LANGUAGE:
			langedit( ch, command);
			break;


		default:
			if(ch && d->pEdit){
				ch->printlnf("run_olc_editor(): encounted an unprocessed state (%d)!", 
					d->editor);
			}	
			return false;
    }
	// logging for all olc stuff that didnt' get past onto interpret
	if(interpret_not_used && CH(d)){
		char logbuf[MSL];
		// time name/short <descriptor/vnum of mob> Room%vnum
		sprintf(logbuf,"%s<%d> C%d E%d '%s'",
			IS_NPC(CH(d)) ? CH(d)->short_descr : CH(d)->name,
			IS_NPC(CH(d)) ? CH(d)->pIndexData->vnum : d->connected_socket,
			d->connected_state,
			d->editor,
			(char *) command);

        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG))
        {
            append_playerlog( ch, logbuf);
        }

		if(!GAMESETTING2(GAMESET2_DISABLE_VERBOSE_OLC_LOGGING)){
			append_logentry_to_file( ch, OLC_LOGFILE, logbuf);
		}
    }
    return true;
}
/**************************************************************************/
/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor( connection_data *c )
{
	char buf[MSL*2];
	strcpy(buf,substitute_alias( c, c->incomm ));
	return run_olc_editor_for_connection(c, buf);
}
/**************************************************************************/
char *olc_ed_name( char_data *ch )
{
    static char buf[MIL];
	strcpy(buf,olcex_get_editor_name(ch->desc->editor));

	if(HAS_MXP(ch)){
		static char result[MSL];
		sprintf(result, "%s `#`S%s %s ", 
			mxp_create_send(ch,"show", buf), 
			mxp_create_send(ch,"done"),
			mxp_create_send(ch,"commands"));

		if(ch->desc->editor == ED_HELP){
			strcat(result, FORMATF("%s %s %s %s %s ", 
				mxp_create_send(ch,"text"),
				mxp_create_send(ch,"raw showflags","displayraw"),
				mxp_create_send(ch,"wraptext confirm", "wrap"),
				mxp_create_send(ch,"undowrap confirm", "unwrap"),
				mxp_create_send(ch,"editnexthelp", ">>>")
				)
			);
		}
		strcat(result,"`&");

		return result;
	}else{
		return FORMATF("%s ",buf);
	}
}
/**************************************************************************/
char *olc_ed_vnum( char_data *ch )
{
    AREA_DATA		*pArea;
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObj;
    MOB_INDEX_DATA	*pMob;
    MUDPROG_CODE		*pMprog;
    help_data		*pHelp;
    BAN_DATA		*pBan;
    race_data		*pRace;
	DEITY_DATA		*pDeity;
	QUEST_DATA		*pQuest;
	HERB_DATA		*pHerb;

	class_type		*pClass;
	cmd_type		*pCmd;
	skill_type		*pSkill;
	mix_type		*pMix;
	social_type		*pSocial;
	CClanType		*pClan;
	skillgroup_type	*pSG;
	language_data	*pLang;
    static char buf[MIL];
	
    buf[0] = '\0';
    switch ( ch->desc->editor )
    {
    case ED_AREA:
		pArea = (AREA_DATA *)ch->desc->pEdit;
		sprintf( buf, "%d", pArea ? pArea->vnum : 0 );
		break;
    case ED_ROOM:
		pRoom = ch->in_room;
		sprintf( buf, "%d", pRoom ? pRoom->vnum : 0 );
		break;
    case ED_OBJECT:
		pObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
		sprintf( buf, "%d", pObj ? pObj->vnum : 0 );
		break;
    case ED_MOBILE:
		pMob = (MOB_INDEX_DATA *)ch->desc->pEdit;
		sprintf( buf, "%d", pMob ? pMob->vnum : 0 );
		break;
    case ED_MPCODE:
		pMprog = (MUDPROG_CODE *)ch->desc->pEdit;
		sprintf( buf, "%d", pMprog ? pMprog->vnum : 0 );
		break;
		
	case ED_HELP:
		{
			char arg1[MIL];
			
			pHelp= (help_data *)ch->desc->pEdit;
			if (pHelp){
				first_arg( pHelp->keyword, arg1, false);
			}else{
				strcpy(arg1,"???");
			}
			sprintf( buf, "%s", arg1);
			return FORMATF("%s ", mxp_create_send(ch, FORMATF("help %s", arg1), buf));
		}
		break;
		
	case ED_BAN:
		pBan= (BAN_DATA *)ch->desc->pEdit;
		sprintf( buf, "%s", pBan? pBan->sitemasks: "???" );
		break;
		
	case ED_RACE:
		pRace= (race_data *)ch->desc->pEdit;
		sprintf( buf, "%s", pRace ? pRace->name : "???" );
		break;
		
	case ED_SPELLSKILL:
		pSkill= (skill_type*)ch->desc->pEdit;
		sprintf( buf, "%s", pSkill->name);
		break;
	case ED_COMMAND:
		pCmd= (cmd_type*)ch->desc->pEdit;
		sprintf( buf, "%s", pCmd->name);
		break;
	case ED_DEITY:
		pDeity= (DEITY_DATA *)ch->desc->pEdit;
		sprintf( buf, "%s", pDeity ? pDeity->name : "???" );
		break;
	case ED_QUEST:
		pQuest= (QUEST_DATA *)ch->desc->pEdit;
		sprintf( buf, "%s", pQuest ? pQuest->questname : "???" );
		break;
	case ED_CLASS:
		pClass= (class_type *)ch->desc->pEdit;
		sprintf( buf, "%s", pClass? pClass->name : "???" );
		break;

	case ED_SOCIAL:
		pSocial= (social_type *)ch->desc->pEdit;
		sprintf( buf, "%s", pSocial? pSocial->name : "???" );
		break;

	case ED_HERB:
		pHerb= (HERB_DATA *)ch->desc->pEdit;
		sprintf( buf, "%s", pHerb ? pHerb->name : "???" );
		break;

	case ED_MIX:
		pMix= (mix_data *)ch->desc->pEdit;
		sprintf( buf, "%s", pMix ? pMix->name : "???" );
		break;

	case ED_CLAN:
		pClan= (CClanType *)ch->desc->pEdit;
		sprintf( buf, "%s", pClan ? pClan->who_name() : "???" );
		break;
		
	case ED_SKILLGROUP:
		pSG= (skillgroup_type*)ch->desc->pEdit;
		sprintf( buf, "%s", pSG ? pSG->name : "???" );
		break;

	case ED_LANGUAGE:
		pLang= (language_data*)ch->desc->pEdit;
		sprintf( buf, "%s", pLang ? pLang->name : "???" );
		break;



	case ED_GAME:
		if(HAS_MXP(ch)){
			strcpy(buf, "showflags");
		}else{
			buf[0]='\0';
		}
		break;

	default:
		buf[0]='\0';
		break;
    }
	return FORMATF("%s ", mxp_create_send(ch, "showflags", buf));
}
/**************************************************************************/
void display_legal_message(char_data *ch)
{
	if(!codehelp( ch, "builder_legal_notice", 0)){
		ch->titlebar("LEGAL NOTICE: OWNERSHIP OF WORK");
		ch->wrapln("`WBe aware that any work you do here (including but in "
			"no way limited to areas you build in OLC) is the property "
			"of this mud and its administration.`1`1"
			"`xYou can prevent the above message from showing each time you login "
			"an enter building mode by acknowledging that you have read it "
			"by typing `=Cmode read_builder_legal`x.");
		if(IS_ADMIN(ch)){
			ch->wrapln("`1The above text can be customised by creating a "
				"help entry with a keyword of 'code_builder_legal_notice' "
				"[this paragraph is seen by admin only, the legal notice above is seen "
				"by anyone who logs on with an olc security of 1 or higher].");
		}
		ch->titlebar("");
	}
}
/*********************************************************************/
void do_modehelp(char_data *ch, char *)
{
	ch->println( "MODE - switches between playing and building modes." );
	ch->println( "  mode play" );
	ch->println( "  mode build" );
	ch->println( "  mode code" );
	ch->println( "  mode read_builder_legal" );
	return;
}
/**************************************************************************/
void do_mode(char_data *ch, char * argument)
{
	char arg1[MIL];
	
	if(IS_UNSWITCHED_MOB(ch))
        return;

	if(IS_NPC(ch))
	{
		ch->println("You can't change modes while switched, use `Yreturn`x to get back.");
        return;
	}

	if (!HAS_SECURITY(ch,2))
	{
		do_modehelp(ch, argument);
		ch->println("You must have an building security of 2 or higher to change modes");
		return;
	}

	if (IS_NULLSTR(argument))
	{
		do_modehelp(ch, argument);
		return;
	}

	argument = one_argument( argument, arg1 );

    /////////////////////////////////////
	if ( !str_cmp( arg1, "read_builder_legal" ) )
	{
		if(!HAS_SECURITY(ch,1)){
			ch->println("You do not have olc security, you don't access to this command.");
			return;
		}
		if(HAS_CONFIG2(ch, CONFIG2_READ_BUILDER_LEGAL)){
			ch->wrapln("You already have already acknowledged the legal agreement... "
				"displaying to you the message once more.");
			display_legal_message(ch);	
			return;
		}
		SET_CONFIG2(ch, CONFIG2_READ_BUILDER_LEGAL);
		ch->wrapln("The legal reminder regarding ownership of work "
			"will no longer be displayed when you login or enter building mode.");
		return;
	}
    /////////////////////////////////////
	if ( !str_prefix( arg1, "player" ) )
	{
		if (!IS_SET(ch->comm,COMM_BUILDING) && !IS_SET(ch->comm,COMM_CODING))
		{
			ch->println("You are already in playing mode.");
			return;
		}

		if (!IS_IMMORTAL(ch) && !IS_OOC(ch))
		{
			ch->println("You must be in an OOC room to switch back into playing mode.");
			ch->println("use olcgoto to get back to there. (ie olcgoto ooc)");
			return;
		}

		ch->println("You are now in player mode.");
		if(IS_SET(ch->comm,COMM_BUILDING))
			REMOVE_BIT(ch->comm,COMM_BUILDING);
		if(IS_SET(ch->comm,COMM_CODING))
			REMOVE_BIT(ch->comm,COMM_CODING);
		return;
	}

	/////////////////////////////////////
	if ( !str_prefix( arg1, "builder" )  
		|| !str_prefix( arg1, "building" ) 
		|| !str_prefix( arg1, "olc" )  )
	{
		if (IS_SET(ch->comm,COMM_BUILDING))
		{
			ch->println("You are already in building mode!");
		}
		else
		{
			if (!IS_IMMORTAL(ch) && !IS_OOC(ch))
			{
				ch->println("You must be in an OOC room to switch into building mode.");
				return;
			}

			ch->println("You are now in building mode.");
			SET_BIT(ch->comm,COMM_BUILDING);

			if(!HAS_CONFIG2(ch, CONFIG2_READ_BUILDER_LEGAL)){
				display_legal_message(ch);
			}
		}	
		return;
	}

	
	if ( !str_prefix( arg1, "coding" )  
		|| !str_prefix( arg1, "code" ))
	{
		if ( !IS_SET( TRUE_CH(ch)->pcdata->council, COUNCIL_CODE )) 
		{
			ch->println( "You're no k0d3r!" );
			return;
		}
		if ( IS_SET(ch->comm,COMM_CODING))
		{
			ch->println( "You are already in coding mode!" );
		}
		else
		{
			ch->println( "You are now in k0d3r mode." );
			SET_BIT( ch->comm, COMM_CODING );
		}	
		return;
	}
	// invalid option
	ch->println("Invalid mode option.");
	do_modehelp(ch, argument);
}
/*********************************************************************/


/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
// each table and it prototypes are stored in a separate .h file
#define OLC_CPP_INCLUDE_BLOCK
#include "aedit.h"	// area edit table
#include "redit.h"	// room edit table
#include "oedit.h"	// object edit table
#include "medit.h"	// mob edit table
#include "hedit.h"	// help edit table
#include "raceedit.h"// race edit table
#include "sedit.h"
#include "banedit.h"
#include "comedit.h"
#include "deity.h"
#include "qedit.h"
#include "cedit.h" // classedit
#include "gameedit.h" 
#include "socedit.h" 
#include "mixedit.h"
#include "clanedit.h"
#include "grpedit.h"


#undef OLC_CPP_INCLUDE_BLOCK
/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_ aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data( int vnum )
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next )
    {
        if (pArea->vnum == vnum)
            return pArea;
    }

    return 0;
}

/*****************************************************************************
 Name:      area_vnum_range_used
 Purpose:   Returns true if an area exists that is already
            allocated a vnum in the range
 Called by:	do_ aedit(olc.c).
 ****************************************************************************/
bool area_vnum_range_used( int lower_vnum, int upper_vnum)
{
    AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next )
    {
        if( (( lower_vnum>=pArea->min_vnum ) &&
      ( lower_vnum<=pArea->max_vnum )) ||
              (( upper_vnum<=pArea->max_vnum ) &&
      ( upper_vnum>=pArea->min_vnum )) )
            return true;
    }

    return false;
}




/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done( char_data *ch )
{
    ch->desc->pEdit = NULL;
	ch->desc->editor = 0;
    return false;
}
/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/
/* Area Interpreter, called by do_ aedit. */
void aedit( char_data *ch, char *argument )
{
    AREA_DATA *pArea;
    char command[MIL];
    char arg[MIL];
    int  cmd;

    EDIT_AREA(ch, pArea);
    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_AREA ) )
    {
		ch->println("AEdit:  Insufficient security to modify areadata.");
		edit_done( ch );
		return;
    }

    if ( !str_cmp(command, "done") )
    {
		edit_done( ch );
		return;
    }

    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_AREA) )
    {
		interpret( ch, arg );
		return;
    }

    if ( command[0] == '\0' )
    {
		aedit_show( ch, argument );
		return;
    }

	// Search Table and Dispatch Command.
    for ( cmd = 0; aedit_table[cmd].name != NULL; cmd++ )
    {
        if ( !str_prefix( command, aedit_table[cmd].name ) )
        {
            if ( (*aedit_table[cmd].olc_fun) ( ch, argument ) )
            {
		        SET_BIT( pArea->olc_flags, OLCAREA_CHANGED );
            }
			return;
        }
    }

    // Default to Standard Interpreter.
    interpret( ch, arg );
    return;
}


/*****************************************************************/
// Room Interpreter, called by do_redit() 
void redit( char_data *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    char arg[MSL];
    char command[MIL];
    int  cmd;

    EDIT_ROOM(ch, pRoom);
    pArea = pRoom->area;

//  smash_tilde( argument ); // no need to smash tildes as everything is saved thru pack_string()
    strcpy( arg, argument );
    argument = one_argument( argument, command );

	if (!HAS_SECURITY(ch,2))
	{
		ch->println("You must have an olc security 2 or higher to use this command.");
		return;
	}

    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_ROOMS) )
    {
        ch->printlnf("REdit:  Insufficient security to modify room %d.", pRoom->vnum);
		edit_done( ch );
		return;
    }

    if ( !str_cmp(command, "done") )
    {
        edit_done( ch );
        return;
    }

    if ( !IS_BUILDER( ch, pArea,  BUILDRESTRICT_ROOMS) )
    {
        interpret( ch, arg );
        return;
    }

    if ( command[0] == '\0' )
    {
		redit_show( ch, argument );
		return;
    }
  
    /* Search Table and Dispatch Command. */
    for ( cmd = 0; redit_table[cmd].name != NULL; cmd++ )
    {
        if ( !str_prefix( command, redit_table[cmd].name ) )
        {
            if ( (*redit_table[cmd].olc_fun) ( ch, argument ) )
            {
            SET_BIT( pArea->olc_flags, OLCAREA_CHANGED );
            return;
            }
            else
            return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Object Interpreter, called by do_oedit. */
void oedit( char_data *ch, char *argument )
{
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char arg[MSL];
    char command[MIL];
    int  cmd;
	
    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );
	
    EDIT_OBJ(ch, pObj);
    pArea = pObj->area;
	
    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_OBJECTS) )
    {
		ch->println("OEdit: Insufficient security to modify objects in this area.");
		edit_done( ch );
		return;
	}
	
    if ( !str_cmp(command, "done") )
    {
		edit_done( ch );
		return;
    }
	
    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_OBJECTS) )
    {
		interpret( ch, arg );
		return;
    }
	
    if ( IS_NULLSTR(command))
    {
		oedit_show( ch, argument );
		return;
    }
	
    /* Search Table and Dispatch Command. */
    for ( cmd = 0; oedit_table[cmd].name != NULL; cmd++ )
    {
        if ( !str_prefix( command, oedit_table[cmd].name ) )
        {
            if ( (*oedit_table[cmd].olc_fun) ( ch, argument ) )
            {
				SET_BIT( pArea->olc_flags, OLCAREA_CHANGED );
				return;
            }
            else
				return;
        }
    }
	
    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

// Mobile Interpreter, called by do_medit.
void medit( char_data *ch, char *argument )
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MIL];
    char arg[MSL];
    int  cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_MOB(ch, pMob);
    pArea = pMob->area;

    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_MOBS) )
    {
        ch->println("MEdit: Insufficient security to modify mobs in this area.");
        edit_done( ch );
        return;
    }

    if ( !str_cmp(command, "done") )
    {
        edit_done( ch );
        return;
    }

    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_MOBS ) )
    {
		interpret( ch, arg );
		return;
    }

    if ( command[0] == '\0' )
    {
        medit_show( ch, argument );
        return;
    }

    // Search Table and Dispatch Command. 
    for ( cmd = 0; medit_table[cmd].name != NULL; cmd++ )
    {
        if ( !str_prefix( command, medit_table[cmd].name ) )
        {
            if ( (*medit_table[cmd].olc_fun) ( ch, argument ) )
            {
				SET_BIT( pArea->olc_flags, OLCAREA_CHANGED );
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
void raceedit( char_data *ch, char *argument )
{
	race_data *r;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_RACE( ch, r );
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( get_trust( ch ) < RACEEDIT_MINTRUST ) {
		ch->println("Insufficient trust level to modify races.");
		edit_done( ch );
		return;
	}

	if ( !str_cmp( command, "done" )) {
		edit_done( ch );
		ch->wraplnf("The changes you have made to the races will automatically be saved within "
			"15 minutes (or at the start of a hotreboot).  If you want to feel warm and "
			"fuzzy inside by saving the races manually use saveraces.");
		ch->printlnf("`S[%s] [%s]`x", 
			mxp_create_send(ch, "saveraces"),
			mxp_create_send(ch, "raceedit"));
		return;
	}

	if ( IS_NULLSTR(command) ) {
		raceedit_show( ch, argument );
		return;
	}

//	Search raceedit_table and interpret command
	for ( cmd = 0; !IS_NULLSTR(raceedit_table[cmd].name); cmd++ ) {

		if ( !str_prefix( command, raceedit_table[cmd].name )) {

			if (( *raceedit_table[cmd].olc_fun ) ( ch, argument )) {
				SET_BIT( RACEEDIT_FLAGS, OLC_CHANGED );
				return;
			}
		else return;
		}
	}

//	Default to standard mud interpreter

	interpret( ch, arg );
	return;
}

/**************************************************************************/
// used for editing the classes
DECLARE_OLC_FUN( classedit_show		);
void classedit( char_data *ch, char *argument )
{
	class_type *pC;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_CLASS( ch, pC);
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( get_trust( ch ) < CLASSEDIT_MINTRUST) {
		ch->println("Insufficient trust level to modify classes.");
		edit_done( ch );
		return;
	}

	if ( !str_cmp( command, "done" )) {
		ch->wrapln("The changes you have made to the classes will automatically be saved within "
			"15 minutes (or at the start of a hotreboot).  If you want to feel warm and "
			"fuzzy inside by saving the classes manually use write_classes.");
		ch->printlnf("`S[%s] [%s]`x", 
			mxp_create_send(ch, "write_classes"),
			mxp_create_send(ch, "classedit"));
		edit_done( ch );
		return;
	}

	if ( IS_NULLSTR(command) ) {
		classedit_show( ch, argument );
		return;
	}

//	Search classedit_table and interpret command

	for ( cmd = 0; classedit_table[cmd].name != NULL; cmd++ ) {

		if ( !str_prefix( command, classedit_table[cmd].name )) {

			if (( *classedit_table[cmd].olc_fun ) ( ch, argument )) {
				SET_BIT(CLASS_TABLE_FLAGS,CLASSEDIT_CHANGED);
				return;
			}
		else return;
		}
	}

//	Default to standard mud interpreter
	interpret( ch, arg );
	return;
}
/**************************************************************************/
// used for editing skills/spell information
void sedit( char_data *ch, char *argument )
{
	skill_type *pSpellSkill;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_SPELLSKILL( ch, pSpellSkill);
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( get_trust( ch ) < SEDIT_MINTRUST) {
		ch->println("Insufficient trust level to modify spell or skill parameters.");
		edit_done( ch );
		return;
	}

	if ( !str_cmp( command, "done" )) {
		ch->wrapln("The changes you have made to the skills will automatically be saved within "
			"15 minutes (or at the start of a hotreboot).  If you want to feel warm and "
			"fuzzy inside by saving the skills manually use write_skills.");
		ch->printlnf("`S[%s] [%s]`x", 
			mxp_create_send(ch, "write_skills"),
			mxp_create_send(ch, "sedit"));

		edit_done( ch );
		return;
	}

	if ( IS_NULLSTR(command)) {
		sedit_show( ch, argument );
		return;
	}

//	Search sedit_table and interpret command
	for ( cmd = 0; sedit_table[cmd].name != NULL; cmd++ ) {

		if ( !str_prefix( command, sedit_table[cmd].name )) {
			if (( *sedit_table[cmd].olc_fun ) ( ch, argument )) {
				SET_BIT(SKILL_TABLE_FLAGS,SEDIT_CHANGED);
				return;
			}else{
				return;
			}
		}
	}

//	Default to standard mud interpreter
	interpret( ch, arg );
	return;
}
/**************************************************************************/
// used for editing the game settings
void gameedit( char_data *ch, char *argument )
{
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	// do security checks
	if (!HAS_SECURITY(ch, GAMEEDIT_MINSECURITY))
	{
    	ch->printf("You must have an olc security %d or higher to use this command.\r\n",
			GAMEEDIT_MINSECURITY);
		edit_done( ch );
		return;
	}

	if ( !IS_TRUSTED(ch, GAMEEDIT_MINTRUST)) {
		ch->printlnf("You must have a trust of %d or above "
			"to use this command.", GAMEEDIT_MINTRUST);
		edit_done( ch );
		return;
	}

	if ( !str_cmp( command, "done" )) {
		ch->printlnf("You must manually save your changes using the '`=C%s`x' command.",
			mxp_create_send(ch, "savegameset"));
		edit_done( ch );
		return;
	}

	if ( IS_NULLSTR(command) ) {
		gameedit_show( ch, argument );
		return;
	}

	for ( cmd = 0; gameedit_table[cmd].name != NULL; cmd++ ) {

		if ( !str_prefix( command, gameedit_table[cmd].name )) {

			if (( *gameedit_table[cmd].olc_fun ) ( ch, argument )) {
				//TODO - put in autosave stuff SET_BIT(CLASS_TABLE_FLAGS,CLASSEDIT_CHANGED); 
				return;
			}
		else return;
		}
	}

//	Default to standard mud interpreter
	interpret( ch, arg );
	return;
}
/**************************************************************************/
void save_socials();
// used for editing the game settings
void socialedit( char_data *ch, char *argument )
{
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	// do security checks
	if (!HAS_SECURITY(ch, SOCEDIT_MINSECURITY))
	{
    	ch->printf("You must have an olc security %d or higher to use this command.\r\n",
			SOCEDIT_MINSECURITY);
		edit_done( ch );
		return;
	}

	if ( !IS_TRUSTED(ch, SOCEDIT_MINTRUST)) {
		ch->printf("You must have a trust of %d or above "
			"to use this command.\r\n", SOCEDIT_MINTRUST);
		edit_done( ch );
		return;
	}

	if ( !str_cmp( command, "done" )) {
		save_socials();
		ch->println("Social file automatically saved.");
		edit_done( ch );
		return;
	}

	if ( command[0] == '\0' ) {
		socedit_show( ch, argument );
		return;
	}

	for ( cmd = 0; socedit_table[cmd].name != NULL; cmd++ ) {

		if ( !str_prefix( command, socedit_table[cmd].name )) {

			if (( *socedit_table[cmd].olc_fun ) ( ch, argument )) {
				//TODO - put in autosave stuff SET_BIT(CLASS_TABLE_FLAGS,SOCEDIT_CHANGED); 
				return;
			}
		else return;
		}
	}

//	Default to standard mud interpreter
	interpret( ch, arg );
	return;
}

/**************************************************************************/
void showhelp_aedit( char_data *ch)
{
    ch->println("`C-== aedit initial instructions ==-`x");
    ch->println("To edit an existing area:");
    ch->println("  `=Caedit %area_number%`x (as per alist)");
    ch->println("or `=Caedit .`x (to edit the area you are in)");
    ch->println("To create a new area: (must have security of 9)");
    ch->println(" `=Caedit create %min_vnum% %max_vnum%`x");
    return;       
}

/**************************************************************************/
// Entry point for editing area_data. 
void do_aedit( char_data *ch, char *argument )
{
    AREA_DATA *pArea;
    int value, lvnum, uvnum, swap_vnum;
    char arg1[MIL], arg2[MIL], arg3[MIL];

	if (!HAS_SECURITY(ch,2))
	{
		ch->println("You must have an olc security 2 or higher to use this command.");
		return;
	}

    argument = one_argument(argument,arg1);

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( !( pArea = get_area_data( value ) ) )
        {
            ch->println("aedit: There is no area with that number reference (as per alist).");
            showhelp_aedit(ch);
            return;
        }

        ch->wraplnf("aedit: You are now editing the area '%s'", pArea->name);           
        ch->println("type `=Cdone`x to finish editing.");
        ch->desc->pEdit = (void *)pArea;
        ch->desc->editor = ED_AREA;
        return;
    }
    if ( !str_cmp( arg1, "." ) )
    {
        // set them up to edit the current area
        pArea = ch->in_room->area;

        if (!IS_BUILDER(ch, pArea, BUILDRESTRICT_AREA))
        {
            ch->println("Insufficient security to edit the areadata of the area you are currently in.");
            return;
        }
		ch->wraplnf("`=rYou are now editing the area '`r%s`=r' vnum: `Y%d`x", 
				pArea->name, pArea->vnum);
		ch->println("`=rType `=Cdone`=r to finish editing.");   


        ch->desc->pEdit = (void *)pArea;
        ch->desc->editor = ED_AREA;
        return;
    }

    if ( !str_cmp( arg1, "create" ) )
    {
        if (IS_NPC(ch) || (ch->pcdata->security < 8) )
        {
           ch->println("Insufficient security to create new areas.");
           return;
        }
        argument = one_argument(argument,arg2);
        argument = one_argument(argument,arg3);

        // check we have enough parameters 
        if (IS_NULLSTR(arg3))
        {
            ch->println("Missing both vnum range values for new area");
            showhelp_aedit(ch);
            return;
        }

        // check we have numeric vnums 
        if (!is_number(arg2) || !is_number(arg3) )
        {
            ch->println("Your vnum values must be numbers");
            showhelp_aedit(ch);
            return;
        }

        // get the numbers and swap them if necessary 
        lvnum = atoi(arg2);
        uvnum = atoi(arg3);
        if (lvnum > uvnum)
        {
            swap_vnum = lvnum;
            lvnum = uvnum;
            uvnum = swap_vnum;
        }

        if (area_vnum_range_used(lvnum, uvnum) )
        {
            ch->print("An existing area is already using some of those vnums!");
            return;
        }

        if (lvnum<0 || uvnum> game_settings->olc_max_vnum)
        {
            ch->printlnf("aedit: Upper and lower vnums must be in the range 0->%d (gameedit's max_vnum)", 
				game_settings->olc_max_vnum);
            return;               
        }

		// officially reserved vnum range
		if(lvnum<500){
			ch->println("Warning: all mobs, rooms and objects below vnum 500 are officially reserved for the dawn codebase.");
			ch->println("As a result you can not create any new areas to use this vnum range using aedit create.");
			return;
		}

		aedit_create(ch,"");
		pArea=(area_data*)ch->desc->pEdit;
        pArea->min_vnum     =   lvnum;
        pArea->max_vnum     =   uvnum;

		sort_arealists(); // resort the vlist and area lists

        SET_BIT( pArea->olc_flags, OLCAREA_ADDED );

        ch->printlnf("aedit: You are now editing the area '%s'", pArea->name);
        ch->println("type `=Cdone`x to finish editing.");

		// aedit_create() above has set the following:
        //ch->desc->pEdit = (void *)pArea;
        //ch->desc->editor = ED_AREA;
        return;
    }

    showhelp_aedit(ch);
    return;
}


/**************************************************************************/
void display_resets( char_data *ch )
{
    ROOM_INDEX_DATA	*pRoom;
    RESET_DATA		*pReset;
    MOB_INDEX_DATA	*pMob = NULL;
    char 		buf   [ MSL ];
    char 		final [ MSL ];
    int 		iReset = 0;
	
	EDIT_ROOM(ch, pRoom);
	final[0]  = '\0';
	
	ch->println("Resets: `YM = mobile, `BO = object, `CP = pet, `MS = shopkeeper");
	ch->println(
		" `BNo.  `YLoads    `BDescription          `YLocation      `BVnum   `YMx `BMn `YDescription\r\n"
		"`B==== `Y======== `B=================== `Y============= `B======== `Y==`G=`B== `Y===========");
	
	for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	{
		OBJ_INDEX_DATA  *pObj;
		MOB_INDEX_DATA  *pMobIndex;
		OBJ_INDEX_DATA  *pObjIndex;
		OBJ_INDEX_DATA  *pObjToIndex;
		ROOM_INDEX_DATA *pRoomIndex;
		final[0] = '\0';
		sprintf( final, "`B[%2d] ", ++iReset );
		switch ( pReset->command )
		{
		default:
			sprintf( buf, "Bad reset command: %c.", pReset->command );
			strcat( final, buf );
			break;
			
		case 'M':
			if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
			{
				sprintf( buf, "Load Mobile - Bad Mob %d\r\n", pReset->arg1 );
				strcat( final, buf );
				continue;
			}
			if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
			{
				sprintf( buf, "Load Mobile - Bad Room %d\r\n", pReset->arg3 );
				strcat( final, buf );
				continue;
			}
			
			pMob = pMobIndex;
			
			sprintf( buf, "`YM[%5d] `B%s `Yin room       `BR[%5d] `Y%2d`G-`B%2d `Y%15.15s\r\n",
				pReset->arg1, str_width(pMob->short_descr, 19,false), pReset->arg3,
				pReset->arg2, pReset->arg4, pRoomIndex->name );
			strcat( final, buf );
			
			/*
			* Check for pet shop.
			* -------------------
			*/
			{
				ROOM_INDEX_DATA *pRoomIndexPrev;
				pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
				
				if ( pRoomIndexPrev
					&& IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
					final[5] = 'P';
			}
			break;
			
		case 'O':
			if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
			{
				sprintf( buf, "Load Object - Bad Object %d\r\n", pReset->arg1 );
				strcat( final, buf );
				continue;
			}
			
			pObj = pObjIndex;
			
			if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
			{
                sprintf( buf, "Load Object - Bad Room %d\r\n", pReset->arg3 );
				strcat( final, buf );
				continue;
			}
			sprintf( buf, "O[%5d] %s in room             "
				"R[%5d]       %-15.15s\r\n",
				pReset->arg1, str_width(pObj->short_descr, 13,false),
				pReset->arg3, pRoomIndex->name );
			strcat( final, buf );
			break;
			
		case 'P':
			if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
			{
				sprintf( buf, "Put Object - Bad Object %d\r\n", pReset->arg1 );
				strcat( final, buf );
				continue;
			}
			
			pObj = pObjIndex;
			
			if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
			{
				sprintf( buf, "Put Object - Bad To Object %d\r\n", pReset->arg3 );
				strcat( final, buf );
				continue;
			}
			sprintf( buf,
				"O[%5d] %s inside              O[%5d] %2d-%2d %-15.15s\r\n",
				pReset->arg1,
				str_width(pObj->short_descr,13,false),
				pReset->arg3,
				pReset->arg2,
				pReset->arg4,
				pObjToIndex->short_descr );
			strcat( final, buf );
			break;
			
		case 'G':
			if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
			{
				sprintf( buf, "Give Object - Bad Object %d\r\n", pReset->arg1 );
				strcat( final, buf );
				continue;
			}
			
			pObj = pObjIndex;
			
			if ( !pMob )
			{
				sprintf( buf, "Give Object - No Previous Mobile\r\n" );
				strcat( final, buf );
				break;
			}
			sprintf(buf,"G`r[%5d] `c%s `rput in the inventory of `cS[%5d]\r\n`x",
				pReset->arg1,
				str_width(pObj->short_descr, 25,false),
				pMob->vnum);
			strcat( final, buf );
			break;
			
		case 'E':
			if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
			{
				sprintf( buf, "Equip Object - Bad Object %d\r\n", pReset->arg1 );
				strcat( final, buf );
				continue;
			}
			
			pObj = pObjIndex;
			
			if ( !pMob )
			{
				sprintf( buf, "Equip Object - No Previous Mobile\r\n" );
				strcat( final, buf );
				break;
			}
			sprintf( buf,
				"`yE[%5d] `b%s `y%-23.23s `bM[%5d]\r\n",
				pReset->arg1,
				str_width(pObj->short_descr, 25,false),
				flag_string( wear_location_strings_types, pReset->arg3 ),
				pMob->vnum);
			strcat( final, buf );
			break;
			
		case 'R':
			if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
			{
				sprintf( buf, "Randomize Exits - Bad Room %d\r\n", pReset->arg1 );
				strcat( final, buf );
				continue;
			}
			sprintf( buf, "`RR[%5d] Exits are randomized in %s\r\n", pReset->arg1, pRoomIndex->name );
			strcat( final, buf );
			break;
		}
		
		ch->print(final);
		ch->print("`x");
   }
   return;
}



/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void add_reset( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index )
{
    RESET_DATA *reset;
    int iReset = 0;

    if ( !room->reset_first )
    {
	room->reset_first	= pReset;
	room->reset_last	= pReset;
	pReset->next		= NULL;
	return;
    }

    index--;

    if ( index == 0 )	/* First slot (1) selected. */
    {
		pReset->next = room->reset_first;
		room->reset_first = pReset;
		return;
    }

    /*
     * If negative slot( <= 0 selected) then this will find the last.
     */
    for ( reset = room->reset_first; reset->next; reset = reset->next )
    {
	if ( ++iReset == index )
	    break;
    }

    pReset->next	= reset->next;
    reset->next		= pReset;
    if ( !pReset->next )
	room->reset_last = pReset;
    return;
}

void show_resets_help( char_data *ch)
{
	ch->println("Syntax: RESET <number> OBJ <vnum> <wear_loc>");
	ch->println("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]");
	ch->println("        RESET <number> OBJ <vnum> room");
	ch->println("        RESET <number> MOB <vnum> [max #x area] [max #x room]");
	ch->println("        RESET <number> DELETE");
	ch->println("        RESET <number> RANDOM [#x exits]");
}

#define MARK_CHANGED(room)	SET_BIT( room->area->olc_flags, OLCAREA_CHANGED)
struct wear_position_to_flags_type{
	int position;	// position being reset onto
	int flag;		// flag that must be set on the object
};

wear_position_to_flags_type   wear_pair[]={
	{WEAR_FINGER_L	, OBJWEAR_FINGER},
	{WEAR_FINGER_R	, OBJWEAR_FINGER},
	{WEAR_NECK_1	, OBJWEAR_NECK},
	{WEAR_NECK_2	, OBJWEAR_NECK},
	{WEAR_TORSO		, OBJWEAR_TORSO	},
	{WEAR_HEAD		, OBJWEAR_HEAD	},
	{WEAR_LEGS		, OBJWEAR_LEGS	},
	{WEAR_FEET		, OBJWEAR_FEET	},
	{WEAR_HANDS		, OBJWEAR_HANDS	},
	{WEAR_ARMS		, OBJWEAR_ARMS	},
	{WEAR_SHIELD	, OBJWEAR_SHIELD},
	{WEAR_ABOUT		, OBJWEAR_ABOUT	},
	{WEAR_WAIST		, OBJWEAR_WAIST	},
	{WEAR_WRIST_L	, OBJWEAR_WRIST	},
	{WEAR_WRIST_R	, OBJWEAR_WRIST	},
	{WEAR_WIELD		, OBJWEAR_WIELD},
	{WEAR_HOLD		, OBJWEAR_HOLD},
	{WEAR_FLOAT		, OBJWEAR_FLOAT},
	{WEAR_SECONDARY	, OBJWEAR_WIELD},
	{WEAR_LODGED_ARM, OBJWEAR_LODGED_ARM},
	{WEAR_LODGED_LEG, OBJWEAR_LODGED_LEG},
	{WEAR_LODGED_RIB, OBJWEAR_LODGED_RIB},
	{WEAR_SHEATHED	, OBJWEAR_SHEATHED},
	{WEAR_CONCEALED	, OBJWEAR_CONCEALED},
	{WEAR_EYES		, OBJWEAR_EYES},
	{WEAR_EAR_L     , OBJWEAR_EAR},
	{WEAR_EAR_R     , OBJWEAR_EAR},
	{WEAR_FACE      , OBJWEAR_FACE},
	{WEAR_ANKLE_L   , OBJWEAR_ANKLE},
	{WEAR_ANKLE_R   , OBJWEAR_ANKLE},
	{WEAR_BACK		, OBJWEAR_BACK},
	{MAX_WEAR		, 0},
};

/**************************************************************************/
void do_resets( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    char arg4[MIL];
    char arg5[MIL];
    char arg6[MIL];
    char arg7[MIL];
    RESET_DATA *pReset = NULL;
	ROOM_INDEX_DATA *pRoom = ch->in_room;

	obj_index_data *pObj;

	// do the checks to make sure they can use the command here
	if (!HAS_SECURITY(ch,1))
	{
		ch->println("The resets command is an olc command, you don't have olc permissions.");
		return;
	}
	if(!pRoom){
		ch->println("BUG: Your room pointer is NULL!");
		return;
	}
	if(!pRoom->area){
		ch->println("BUG: The room you are in doesn't have an area associated with it!");
		return;
	}
    if ( !IS_BUILDER( ch, pRoom->area, BUILDRESTRICT_RESETS ) )
    {
		ch->println("Resets: Invalid security for editing resets this area.");
		return;
    }

    // split up the arguments
	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );
    argument = one_argument( argument, arg6 );
    argument = one_argument( argument, arg7 );

    /*
     * Display resets in current room.
     * -------------------------------
     */
    if ( IS_NULLSTR(arg1))
    {
		if ( ch->in_room->reset_first )
		{
			display_resets( ch );
		}
		else
		{
			ch->println("No resets in this room.");
		}
    }
	// display the help
    if (!is_number( arg1 ) )
    {
		show_resets_help( ch);
		return;
	}

    /*
     * Take index number and search for commands.
     * ------------------------------------------
     */
	/*
	* Delete a reset.
	* ---------------
	*/
	if ( !str_cmp( arg2, "delete" ) )
	{
		int insert_loc = atoi( arg1 );
		
		if ( !ch->in_room->reset_first )
		{
			ch->println("No resets in this area.");
			return;
		}
		
		if ( insert_loc-1 <= 0 )
		{
			pReset = pRoom->reset_first;
			pRoom->reset_first = pRoom->reset_first->next;
			if ( !pRoom->reset_first )
				pRoom->reset_last = NULL;
		}
		else
		{
			int iReset = 0;
			RESET_DATA *prev = NULL;
			
			for ( pReset = pRoom->reset_first;
			pReset;
			pReset = pReset->next )
			{
				if ( ++iReset == insert_loc )
					break;
				prev = pReset;
			}
			
			if ( !pReset )
			{
				ch->println("Reset not found.");
				show_resets_help(ch);
				return;
			}
			
			if ( prev )
				prev->next = prev->next->next;
			else
				pRoom->reset_first = pRoom->reset_first->next;
			
			for ( pRoom->reset_last = pRoom->reset_first;
			pRoom->reset_last->next;
			pRoom->reset_last = pRoom->reset_last->next );
		}
		
		free_reset_data( pReset );
		ch->println("Reset deleted.");
		MARK_CHANGED(pRoom); // Flag the area as a changed area for autosave
	}
	else
	
	/*
	* Add a reset.
	* ------------
	*/
	if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
		|| (!str_cmp( arg2, "obj" ) && is_number( arg3 )) )
	{
		/*
		* Check for Mobile reset.
		* -----------------------
		*/
		if ( !str_cmp( arg2, "mob" ) )
		{
			if (get_mob_index( is_number(arg3) ? atoi( arg3 ) : 1 ) == NULL)
			{
				ch->println("Mob doesnt exist.");
				show_resets_help(ch);
				return;
			}
			pReset = new_reset_data();
			pReset->command = 'M';
			pReset->arg1    = atoi( arg3 );
			pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1; /* Max # */
			pReset->arg3    = ch->in_room->vnum;
			pReset->arg4	= is_number( arg5 ) ? atoi( arg5 ) : 1; /* Min # */
		}
		else
		{
		/*
		* Check for Object reset.
		* -----------------------
			*/
			if ( !str_cmp( arg2, "obj" ) )
			{
				pReset = new_reset_data();
				pReset->arg1    = atoi( arg3 );
				/*
				* Inside another object.
				* ----------------------
				*/
				if ( !str_prefix( arg4, "inside" ) )
				{
					OBJ_INDEX_DATA *temp;
					
					temp = get_obj_index(is_number(arg5) ? atoi(arg5) : 1);
					if (!temp)
					{
						ch->printlnf("Couldn't find Object 2 (vnum %d)", 
							is_number(arg5) ? atoi(arg5) : 1) ;
						show_resets_help(ch);
						return;
					}
					
					if ((   temp->item_type != ITEM_CONTAINER  )
						&& (temp->item_type != ITEM_CORPSE_NPC )
						&& (temp->item_type != ITEM_FLASK	   )
						&& (temp->item_type != ITEM_CAULDRON   )
						&& (temp->item_type != ITEM_MORTAR    ))
					{
						ch->printlnf("Object 2 (vnum '%s') isn't a container.", arg5);
						show_resets_help(ch);
						return;
					}
					pReset->command = 'P';
					pReset->arg2    = is_number( arg6 ) ? atoi( arg6 ) : 1;
					pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
					pReset->arg4    = is_number( arg7 ) ? atoi( arg7 ) : 1;
				}
				else
				/*
				* Inside the room.
				* ----------------
				*/
				if ( !str_cmp( arg4, "room" ) )
				{
					if (get_obj_index(atoi(arg3)) == NULL)
					{
						ch->println("Vnum room vnum exists.");
						show_resets_help(ch);
						return;
					}
					pReset->command  = 'O';
					pReset->arg2     = 0;
					pReset->arg2 = -1; //max world - unlimited
					pReset->arg3     = ch->in_room->vnum;
					pReset->arg4     = 0;
				}
				else
				/*
				* Into a Mobile's inventory.
				* --------------------------
				*/
				{
					if ( flag_value( wear_location_types, arg4 ) == NO_FLAG )
					{
						ch->println("Resets: '? wear-loc'");
						show_resets_help(ch);
						return;
					}
					pObj=get_obj_index(atoi(arg3));
					if (pObj== NULL)
					{
						ch->printlnf("Object vnum %d doesn't exist.", atoi(arg3));
						show_resets_help(ch);
						return;
					}
					pReset->arg1 = atoi(arg3);
					pReset->arg3 = flag_value( wear_location_types, arg4 );

					if ( pReset->arg3 == WEAR_NONE )
					{
						pReset->command = 'G';
						//arg1 stays the same
						pReset->arg2 = -1; //max world - unlimited
						pReset->arg3 = 0;
						pReset->arg4 = 0;
					}
					else
					{
						// do warning if the location it is being reset onto isnt 
						// a wear location the object is able to be worn on
						// or if the object isn't takeable.
						if(pReset->arg3==WEAR_LIGHT && pObj->item_type!=ITEM_LIGHT){
							ch->printlnf("`RWARNING:`x object %d '%s' <%d> is being reset into the Light position, but isn't of type light!`x",
								pObj->vnum, pObj->short_descr, pObj->level);
						}else{
							int i;
							// check the position matches the wear flags
							for(i=0; wear_pair[i].position!=MAX_WEAR; i++){
								if(wear_pair[i].position==pReset->arg3){
									if(!IS_SET(pObj->wear_flags, wear_pair[i].flag)){
										ch->printlnf("`RWARNING:`x object %d '%s' <%d> doesn't have the correct wear flags to normally be worn on this position!`x",
											pObj->vnum, pObj->short_descr, pObj->level) ;
									}
								}
							}
						}
						if(!IS_SET(pObj->wear_flags, OBJWEAR_TAKE)){
							ch->printlnf("`RWARNING:`x object %d '%s' <%d> is NOT takeable!`x",
								pObj->vnum, pObj->short_descr, pObj->level) ;
						}
						pReset->command = 'E';
						pReset->arg2 = -1; //max world - unlimited			
					}
				}
			}
		}
		add_reset( ch->in_room, pReset, atoi( arg1 ) );
		MARK_CHANGED(pRoom); // Flag the area as a changed area for autosave
		ch->println("Reset added.");
	}
	else if (!str_cmp( arg2, "random") && is_number(arg3))
	{
		if (atoi(arg3) < 1 || atoi(arg3) > 6)
		{
			ch->println("Invalid argument.");
			show_resets_help(ch);
			return;
		}
		pReset = new_reset_data ();
		pReset->command = 'R';
		pReset->arg1 = ch->in_room->vnum;
		pReset->arg2 = atoi(arg3);
		add_reset( ch->in_room, pReset, atoi( arg1 ) );
		MARK_CHANGED(pRoom); // Flag the area as a changed area for autosave
		ch->println("Random exits reset added.");
	}
	else
	{
		ch->printlnf("'%s' is not a recognised reset command!", arg2);
		show_resets_help( ch);
	}
    return;
}
/**************************************************************************/


/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_alist( char_data *ch, char *argument )
{
    char buf    [ MSL ];
    AREA_DATA *pArea;
	char aname[MIL];
    BUFFER *output;

    if (!HAS_SECURITY(ch,1)){
		ch->println("The alist command is an olc command, you don't have olc permissions.");
		return;
	}
    output = new_buf();
	
	add_buf( output, FORMATF("[%3s] [%-18s]<maplevel>(%-5s-%5s) [%-14s] %3s [%-10s]\r\n",
		"Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders" ));
	
    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
		if (!is_name( argument, pArea->name ) // filter in only required areas
			&& !is_name( argument, pArea->file_name) 
			&& !is_name( argument, pArea->builders) )
			continue;
		sprintf(aname,"%s", str_width(pArea->name,26,false));
		sprintf( buf, "[%3d] %s<%2d>(%-5d-%5d) %-16.16s [%d] [%-10.10s]\r\n",
			pArea->vnum,
			aname,
			pArea->maplevel,
			pArea->min_vnum,
			pArea->max_vnum,
			pArea->file_name,
			pArea->security,
			pArea->builders );
		add_buf( output, buf );
	}
	ch->sendpage(buf_string(output));
    free_buf(output);
}

/**************************************************************************/
void do_aslist( char_data *ch, char *argument )
{
    char buf[ MSL ];
    AREA_DATA *pArea;
	char aname[MIL];
	BUFFER *output;

    if (!HAS_SECURITY(ch,1))
	{
		ch->println("The aslist command is an olc command, you don't have olc permissions.");
		return;
	}

    // setup a buffer for info to be displayed 
    output = new_buf();  

	add_buf( output, "[Num]====== Area Name ======(FirstRoom)[ Continent  ] Short Name\r\n");
    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
		if (!is_name( argument, pArea->name ) // filter in only required areas
			&& !is_name( argument, pArea->file_name) 
			&& !is_name( argument, pArea->continent?pArea->continent->name:"any") 
			&& !is_name( argument, pArea->short_name))
			continue;

		// find the first room in the area
		int first_room=-1;
		for ( int vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		{
			if ( get_room_index( vnum ) ){
				first_room=vnum;
				break;
			}
		}

		sprintf(aname,"%s", str_width(pArea->name,26,true));
		sprintf( buf, "%s[%s] %s`x(%s%s`x)[%s%-16.16s`x] %s%s`x\r\n",
			(IS_NULLSTR(pArea->short_name)?"`W":"`x"),
			mxp_create_send(ch, FORMATF("aedit %d", pArea->vnum), FORMATF("%3d", pArea->vnum)),
			aname,
			first_room!=pArea->min_vnum?"`W":"`x",
			mxp_create_send(ch, FORMATF("goto %d", first_room), FORMATF("%5d", first_room)),
			colour_table[(continent_count(pArea->continent)%14)+1].code,
			pArea->continent?pArea->continent->name:"any",
			colour_table[(pArea->vnum%14)+1].code,
			pArea->short_name );
		add_buf( output, buf );
	}

	ch->sendpage(buf_string(output));
    free_buf(output);

	return;
}
/**************************************************************************/

/*****************************************************************************
 Name:      do_vlist
 Purpose:	Normal command to list areas and display area information.
            - sorted by vnum
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_vlist( char_data *ch, char *argument )
{
    char buf[MSL];
    AREA_DATA *pArea;
	char prenum[3], postnum[3];
	char aname[MIL];
	BUFFER *output;

	strcpy(prenum,"`x");

	if (!HAS_SECURITY(ch,1))
	{
		ch->println("The vlist command is an olc command, you don't have olc permissions." );
		return;
	}

    output = new_buf();

    add_buf(output, FORMATF("[%3s] [%-27s] (%-5s-%5s) [%-14s] %3s [%-10s] [%-12s]\r\n",
       "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders", "Continent" ));

    for ( pArea = area_vnumsort_first; pArea; pArea = pArea->vnumsort_next )
    {
        if (pArea->vnumsort_next){
            if (pArea->max_vnum >= pArea->vnumsort_next->min_vnum){
                add_buf(output, "`R");
            }  
			
			if (pArea->max_vnum+1 == pArea->vnumsort_next->min_vnum){
				strcpy(postnum,"`G");
			}else{
				strcpy(postnum,"`x");
			}
            
		}else{
			strcpy(postnum,"`x");
		}

		if (!is_name( argument, pArea->name ) // filter in only required areas
			&& !is_name( argument, pArea->file_name) 
			&& !is_name( argument, pArea->builders)
			&& !is_name( argument, pArea->continent?pArea->continent->name:"any"))
			continue;

		strcpy(aname, str_width(pArea->name,29,true));
		sprintf( buf, "[%3d] %s (`#%s%-5d`^%s-%5d`^) %-16.16s [%d] [%-10.10s] [%-16.16s]`x\r\n",
			pArea->vnum,
			aname,
			prenum,
			pArea->min_vnum,
			postnum,
			pArea->max_vnum,
			pArea->file_name,
			pArea->security,
			pArea->builders,
			pArea->continent?pArea->continent->name:"any" );
		add_buf(output, buf );
		strcpy(prenum, postnum);
    }

    ch->sendpage(buf_string(output));
    free_buf(output);
	return;
}


/*****************************************************************************
 Name:      do_olcgoto
 Purpose:	Used as a goto command, can only goto olc and ooc rooms.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_olcgoto( char_data *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    char_data *rch;
    int count = 0;

	if(IS_NPC(ch))
	{
		ch->println("You can't use OLCGOTO while while switched, use `=Creturn`x to get back.");
        return;
	}

	if (!HAS_SECURITY(ch,2))
	{
		ch->println("You must have an olc building security of 2 or higher to use this command.");
		return;
	}

	if (!IS_SET(ch->comm,COMM_BUILDING))
	{
		ch->println("You must be in building mode to use olcgoto.");
		ch->println("In an ooc room type `=Cmode build`x.");
		return;
	}
	
    if ( argument[0] == '\0' )
    {
        ch->println("Syntax: olcgoto <vnum>");
        ch->println("        olcgoto ooc");
        return;
    }


	if (!str_cmp(argument, "ooc") )
	{
		location = get_room_index(30000);

		// take them out of olc mode
		ch->desc->pEdit = NULL;
		ch->desc->editor = 0;
	}
	else
	{
		if (( location = find_location( ch, argument ) ) == NULL )
		{
			ch->println("No such location.");
			return;
		}

		if (ch->in_room==location)
		{
			ch->println("You are already in that room.");
			return;
		}

		if (!IS_SET(location->area->area_flags, AREA_OLCONLY) 
			&& !IS_SET(location->room_flags, ROOM_OOC) )
		{
			ch->println("You can only olcgoto olc rooms you have security to or the main ooc room.");
			return;
		}

		if (!IS_BUILDER( ch, location->area, BUILDRESTRICT_ROOMS))
		{
			ch->println("You can only olcgoto olc rooms you have security to or the main ooc room.");
			return;
		}

		if (is_room_private_to_char(location, ch) &&  (count > 1 || get_trust(ch) < MAX_LEVEL))
		{
			ch->println("That room is private right now.");
			return;
		}

	}

    if ( ch->fighting)
        stop_fighting( ch, true );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= INVIS_LEVEL(ch))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
        {
            act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            if (rch->level >= MAX_LEVEL)
            {
                act("($n OLCGOTO POOFOUT)",ch,NULL,rch,TO_VICT);              
            }
        }
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    char_from_room( ch );
    char_to_room( ch, location );
    if (ch->mounted_on)
    {  
        char_from_room( ch->mounted_on );
        char_to_room( ch->mounted_on, location );
        do_look(ch->mounted_on, "auto");
    }
    
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= INVIS_LEVEL(ch))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
            {
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
                if (rch->level >= MAX_LEVEL)
                {
                    act("($n OLCGOTO POOFIN)",ch,NULL,rch,TO_VICT);              
                }
            }
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_look( ch, "auto" );
    return;
}


/*****************************************************************************
 Name:      do_newresets
 Purpose:	Replacement resets comand - new layout
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_newresets( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    char arg4[MIL];
    char arg5[MIL];
    char arg6[MIL];
    char arg7[MIL];
    RESET_DATA *pReset = NULL;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );
    argument = one_argument( argument, arg6 );
    argument = one_argument( argument, arg7 );

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_RESETS ) )
    {
	ch->println("Resets: Invalid security for editing this area.");
	return;
    }

    /*
     * Display resets in current room.
     * -------------------------------
     */
    if ( IS_NULLSTR(arg1))
    {
		if ( ch->in_room->reset_first )
		{
			display_resets( ch );
		}
		else
		{
			ch->println("No resets in this room.");
		}
    }


    /*
     * Take index number and search for commands.
     * ------------------------------------------
     */
    if ( is_number( arg1 ) )
    {
	ROOM_INDEX_DATA *pRoom = ch->in_room;

	/*
	 * Delete a reset.
	 * ---------------
	 */
	if ( !str_cmp( arg2, "delete" ) )
	{
	    int insert_loc = atoi( arg1 );

	    if ( !ch->in_room->reset_first )
	    {
			ch->println("No resets in this area.");
			return;
	    }

	    if ( insert_loc-1 <= 0 )
	    {
		pReset = pRoom->reset_first;
		pRoom->reset_first = pRoom->reset_first->next;
		if ( !pRoom->reset_first )
		    pRoom->reset_last = NULL;
	    }
	    else
	    {
		int iReset = 0;
		RESET_DATA *prev = NULL;

		for ( pReset = pRoom->reset_first;
		  pReset;
		  pReset = pReset->next )
		{
		    if ( ++iReset == insert_loc )
			break;
		    prev = pReset;
		}

		if ( !pReset )
		{
		    ch->println("Reset not found.");
		    return;
		}

		if ( prev )
		    prev->next = prev->next->next;
		else
		    pRoom->reset_first = pRoom->reset_first->next;

		for ( pRoom->reset_last = pRoom->reset_first;
		  pRoom->reset_last->next;
		  pRoom->reset_last = pRoom->reset_last->next );
	    }

	    free_reset_data( pReset );
	    ch->println("Reset deleted.");
	}
	else

	/*
	 * Add a reset.
	 * ------------
	 */
	if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
	  || (!str_cmp( arg2, "obj" ) && is_number( arg3 )) )
	{
	    /*
	     * Check for Mobile reset.
	     * -----------------------
	     */
	    if ( !str_cmp( arg2, "mob" ) )
	    {
		if (get_mob_index( is_number(arg3) ? atoi( arg3 ) : 1 ) == NULL)
		  {
		    ch->println("Mob no existe.");
		    return;
		  }
		pReset = new_reset_data();
		pReset->command = 'M';
		pReset->arg1    = atoi( arg3 );
		pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1; /* Max # */
		pReset->arg3    = ch->in_room->vnum;
		pReset->arg4	= is_number( arg5 ) ? atoi( arg5 ) : 1; /* Min # */
	    }
	    else
	    /*
	     * Check for Object reset.
	     * -----------------------
	     */
	    if ( !str_cmp( arg2, "obj" ) )
	    {
		pReset = new_reset_data();
		pReset->arg1    = atoi( arg3 );
		/*
		 * Inside another object.
		 * ----------------------
		 */
		if ( !str_prefix( arg4, "inside" ) )
		{
		    OBJ_INDEX_DATA *temp;

		    temp = get_obj_index(is_number(arg5) ? atoi(arg5) : 1);
			if  ((  temp->item_type != ITEM_CONTAINER  )
				&& (temp->item_type != ITEM_CORPSE_NPC )
				&& (temp->item_type != ITEM_FLASK	   )
				&& (temp->item_type != ITEM_CAULDRON   )
				&& (temp->item_type != ITEM_MORTAR	  ))
			{
				ch->println( "Obj val 2 isn't a valid container type." );
				return;
			}
		    pReset->command = 'P';
		    pReset->arg2    = is_number( arg6 ) ? atoi( arg6 ) : 1;
		    pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
		    pReset->arg4    = is_number( arg7 ) ? atoi( arg7 ) : 1;
		}
		else
		/*
		 * Inside the room.
		 * ----------------
		 */
		if ( !str_cmp( arg4, "room" ) )
		{
		    if (get_obj_index(atoi(arg3)) == NULL)
			{
				ch->println( "Vnum doesn't exist." );
				return;
			}
		    pReset->command  = 'O';
		    pReset->arg2     = 0;
		    pReset->arg3     = ch->in_room->vnum;
		    pReset->arg4     = 0;
		}
		else
		/*
		 * Into a Mobile's inventory.
		 * --------------------------
		 */
		{
		    if ( flag_value( wear_location_types, arg4 ) == NO_FLAG )
		    {
				ch->println("Resets: '? wear-loc'");
				return;
		    }
		    if (get_obj_index(atoi(arg3)) == NULL)
		      {
                 ch->printlnf("Object vnum %d doesn't exist.", atoi(arg3));
		         return;
		      }
		    pReset->arg1 = atoi(arg3);
		    pReset->arg3 = flag_value( wear_location_types, arg4 );
		    if ( pReset->arg3 == WEAR_NONE )
				pReset->command = 'G';
		    else
				pReset->command = 'E';
		}
	    }
	    add_reset( ch->in_room, pReset, atoi( arg1 ) );
        SET_BIT( ch->in_room->area->olc_flags, OLCAREA_CHANGED );
	    ch->println("Reset added.");
	}
	else
	if (!str_cmp( arg2, "random") && is_number(arg3))
	{
		if (atoi(arg3) < 1 || atoi(arg3) > 6)
			{
				ch->println("Invalid argument.");
				return;
			}
		pReset = new_reset_data ();
		pReset->command = 'R';
		pReset->arg1 = ch->in_room->vnum;
		pReset->arg2 = atoi(arg3);
		add_reset( ch->in_room, pReset, atoi( arg1 ) );
        SET_BIT( ch->in_room->area->olc_flags, OLCAREA_CHANGED );
		ch->println("Random exits reset added.");
	}
	else
	{
	ch->println("Syntax: RESET <number> OBJ <vnum> <wear_loc>");
	ch->println("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]");
	ch->println("        RESET <number> OBJ <vnum> room");
	ch->println("        RESET <number> MOB <vnum> [max #x area] [max #x room]");
	ch->println("        RESET <number> DELETE");
	ch->println("        RESET <number> RANDOM [#x exits]");
	}
    }

    return;
}

/**************************************************************************/
// do_rlist: shows all the rooms and vnums in the current area
void display_rlist(char_data *ch, char *argument, bool balance_version)
{	
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA		*pArea;
    char		buf  [MSL];
    char		buf2  [MSL];
    BUFFER		*buf1;
    BUFFER		*output;
    char		arg[MIL];
    bool found;
    int vnum;
    int  col = 0;
	int num_of_rooms =0;

	if (!HAS_SECURITY(ch,1))
	{
		ch->println("The rlist command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_ROOMS ) && !IS_IMMORTAL(ch))
    {
		ch->println("rlist: Invalid security for viewing rooms in this area.");
		return;
    }

    one_argument( argument, arg );

    pArea = ch->in_room->area;
    buf1=new_buf();
    found   = false;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
		if ( ( pRoomIndex = get_room_index( vnum ) ) )
		{
			int door;
			char doors[30];
			char hdoors[30]; // hidden doors
			bool hfound;
			EXIT_DATA *pexit;
			char vnumtext[30];
			
			// support infix searching on room name and vnum
			sprintf(vnumtext, "%d", pRoomIndex->vnum);
			if(str_infix( arg, pRoomIndex->name) && str_infix( arg, vnumtext) )	{
				continue;
			};
			
			found = true;


			// do the exits for each room
			doors[0] = '\0';
			for (door = 0; door < MAX_DIR; door++)
			{
				if ((pexit = pRoomIndex->exit[door]) != NULL
				&&  pexit ->u1.to_room != NULL
				&&  !IS_SET(pexit->exit_info,EX_CLOSED))
				{
					found = true;
					strcat(doors,dir_shortname[door]);
				}
			}
		    // hidden exits 
			hfound = false;
			hdoors[0] = '(';
			hdoors[1] = '\0';
			for (door = 0; door < MAX_DIR; door++)
			{
				if ((pexit = pRoomIndex->exit[door]) != NULL
				&&  pexit ->u1.to_room != NULL
				&&  IS_SET(pexit->exit_info,EX_CLOSED))
				{
					hfound = true;
					strcat(hdoors,dir_shortname[door]);
				}
			}
			strcat(hdoors,")");
			if (hfound) strcat(doors,hdoors);

			if(balance_version)
			{
				char flagcheck[150];
				flagcheck[0]='\0';

				if(IS_NULLSTR(pRoomIndex->description)){
					strcat(flagcheck,"  `YNO ROOM DESCRIPTION!!! ");
				}
				if(IS_SET(pRoomIndex->room_flags,ROOM_LIGHT)){
					strcat(flagcheck," `WLight");
				};
				if(IS_SET(pRoomIndex->room_flags,ROOM_SAFE)){
					strcat(flagcheck," `YSafe");
				};
				if(IS_SET(pRoomIndex->room_flags,ROOM_NO_RECALL)){
					strcat(flagcheck," `GNoRecall");
				};
				if(IS_SET(pRoomIndex->room_flags,ROOM_NOSCRY)){
					strcat(flagcheck," `CNoScry");
				};
				if(IS_SET(pRoomIndex->room_flags,ROOM_ANTIMAGIC)){
					strcat(flagcheck," `MAntiMagic");
				};
				
				if(IS_SET(pRoomIndex->room_flags,ROOM_PET_SHOP)){
					strcat(flagcheck," `cPetShop");
				};				
				if(IS_SET(pRoomIndex->room_flags,ROOM_SOLITARY)){
					strcat(flagcheck," `RSolitary");
				};
				if(IS_SET(pRoomIndex->room_flags,ROOM_PRIVATE)){
					strcat(flagcheck," `rPrivate");
				};
				if(IS_SET(pRoomIndex->room_flags,ROOM_NO_MOB)){
					strcat(flagcheck," `mNoMob");
				};				
				if(IS_SET(pRoomIndex->room_flags,ROOM_NOSPEAK)){
					strcat(flagcheck, " `RNoSpeak");
				};				
				if(IS_SET(pRoomIndex->room_flags,ROOM_NOCHANNELS)){
					strcat(flagcheck, " `RNoChannels");
				};				

				sprintf( buf, "`S [`%c%s%s%c `r%s%s `B%s`S]",
					(ch->in_room->vnum == vnum?'Y':'x'), 
					mxp_create_send(ch,FORMATF("redit %d", vnum), FORMATF("%5d",vnum) ),
					(IS_SET(pRoomIndex->room_flags,ROOM_DARK)?"`S":"`W"),
					(IS_SET(pRoomIndex->room_flags,ROOM_INDOORS)?'I':':'),
					str_width(capitalize( pRoomIndex->name ), 
						(65-(str_len(doors)+c_str_len(flagcheck))),false),
						flagcheck, doors );
				add_buf( buf1, buf );
				add_buf( buf1, "`x\r\n" );
			}else{
				sprintf( buf, "`S[`%c%s%s%c `r%s `B%s`S]",
					(ch->in_room->vnum == vnum?'Y':'x'), 
					mxp_create_send(ch, FORMATF("redit %d", vnum), FORMATF("%5d",vnum) ),
					(IS_SET(pRoomIndex->room_flags,ROOM_DARK)?"`S":"`W"),
					(IS_SET(pRoomIndex->room_flags,ROOM_INDOORS)?'I':':'),
					str_width(capitalize( pRoomIndex->name ), 
						(28-str_len(doors)),true),	doors );
				if ( ++col % 2 == 0 ){
					add_buf( buf1, buf );
					add_buf( buf1, "`x\r\n" );
				}else{ // first on a line, include mxp prefix
					add_buf( buf1, buf );
					add_buf( buf1, " " );
				}
			}
			num_of_rooms++;
		}
    }

    if ( !found )
    {
		ch->println("Rooms not found in this area.");
		return;
    }

    if ( col % 2 != 0 ){
		add_buf( buf1, "`x\r\n" );
	}else{
		add_buf( buf1, "`x" );
	}

	sprintf(buf, "RLIST: %d room%s found", num_of_rooms, (num_of_rooms==1?"":"s"));
	sprintf( buf2,"`S%s`x", format_titlebar(buf));

	output = new_buf();
	add_buf(output,buf2);
	add_buf(output,buf_string(buf1));
	free_buf(buf1);

    ch->sendpage(buf_string(output));
    free_buf(output);
    return;
}
/**************************************************************************/
void do_rlist( char_data *ch, char *argument )
{
	display_rlist(ch, argument, false);
}
/**************************************************************************/
void do_brlist( char_data *ch, char *argument )
{
	display_rlist(ch, argument, true);
}
/**************************************************************************/
//  do_mlist: shows all the mobs and vnums in the current area 
void do_mlist( char_data *ch, char *argument )
{
	static int		colour;
    MOB_INDEX_DATA	*pMobIndex;
    AREA_DATA		*pArea;
    char			buf[MSL];
    BUFFER			*buf1;
    char			arg[MIL];
    bool			fAll, found, cTest;
    int				vnum, stars;
    int				col = 0;
	char			vnum_text[MIL];

	if (!HAS_SECURITY(ch,1))
	{
		ch->println("The mlist command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_MOBS ) && !IS_IMMORTAL(ch))
    {
		ch->println("mlist: Invalid security for viewing mobs in this area.");
		return;
    }

    one_argument( argument, arg );
	if ( !str_cmp(arg, "?") )
    {
		ch->println("Syntax:  mlist <all/name>");
		return;
    }

    buf1  = new_buf();
    pArea = ch->in_room->area;
    fAll  = (!str_cmp( arg, "all" ) || IS_NULLSTR(arg));
	cTest = false;   
	if (!str_cmp( arg, "ctest" ))
	{
		fAll = true;
		cTest = true;
		colour++;
	}
    found   = false;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
		pMobIndex = get_mob_index( vnum );
		if(!pMobIndex){
			continue;
		}

		strcpy(vnum_text, mxp_create_send(ch,FORMATF("medit %d", pMobIndex->vnum), FORMATF("%5d", pMobIndex->vnum)));

		if ( fAll || is_name( arg, pMobIndex->player_name ) )
		{
			if (cTest)
			{
				if (has_colour(pMobIndex->short_descr) 
					||has_colour(pMobIndex->long_descr) 
					||has_colour(pMobIndex->description))
				{
					found = true;		
					sprintf( buf, CC"x"CC"%s[%s] %s *",
						colour%2==0?"R":"G",
						vnum_text, 
						pMobIndex->short_descr);
					stars = 70-c_str_len(pMobIndex->short_descr);
					while (stars>0)
					{
						strcat(buf,"*");
						stars--;
					}
					strcat(buf,"`x\r\n");
					add_buf( buf1, buf );

					sprintf( buf, CC"x"CC"%sDefault %s *",
						colour%2==0?"B":"Y", pMobIndex->long_descr);
					stars = 70-c_str_len(pMobIndex->long_descr);
					while (stars>0)
					{
						strcat(buf,"*");
						stars--;
					}
					strcat(buf,"`x\r\n");
				}
				else
				{
					continue;
				}
			}
			else
			{
				found = true;
				sprintf( buf, CC"x[%s] %s ", 
					vnum_text, str_width(capitalize( pMobIndex->short_descr ),29,true) );
				if ( ++col % 2 == 0 ){
					strcat( buf, "`x\r\n");
				}
			}	
			add_buf( buf1, buf );
		}
    }

	if (cTest && !found)
    {
		ch->println("There are no mobiles in the area you are in that have colour");
		ch->println("in either their short or default description.");
		free_buf(buf1);
		return;
    }

    if ( !found )
    {
		ch->println("Syntax:  mlist <all/name>");
		ch->println("Mobile(s) not found in this area.");
		ch->println("To test the colour codes type `=Cmlist ctest`x");
		free_buf(buf1);
		return;
    }

    if ( col % 2 != 0 )
		add_buf( buf1, "\r\n" );

    ch->sendpage(buf_string(buf1));
    free_buf(buf1);
    return;
}
/**************************************************************************/
//  do_bmlist: balance mob list directions - Kal
void do_bmlist( char_data *ch, char *)
{
	ch->println("Use `=Cbmvlist`x for balance mob list sorted by vnum.");
	ch->println("Use `=Cbmllist`x for balance mob list sorted by level.");
}
/**************************************************************************/
//  do_bmvlist: balance mob list (by vnum) - Kal
void do_bmvlist( char_data *ch, char *argument )
{
    MOB_INDEX_DATA	*pMobIndex;
    AREA_DATA		*pArea;
    char			buf[MSL];
    BUFFER			*buf1;
    char			arg[MIL];
    bool			fAll, found;
    int				vnum;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The bmlist command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_MOBS ) && !IS_IMMORTAL(ch))
    {
		ch->println("bmlist: Invalid security for viewing mobs in this area.");
		return;
    }

    one_argument( argument, arg );
	if ( !str_cmp(arg, "?") )
    {
		ch->println("Syntax:  bmlist <all/name>");
		return;
    }

    buf1  = new_buf();
    pArea = ch->in_room->area;
    fAll  = (!str_cmp( arg, "all" ) || IS_NULLSTR(arg));

    found   = false;

	add_buf( buf1,
"*[mVNUM] Short Description             Level [ DI DH DV SA PD FL RG CH SU SL]\r\n");
//2345] 12345678901234567890123456789 <123> [%3s %3s %3s %3s %3s %3s %3s]");
    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
		if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
		{
			if ( fAll || is_name( arg, pMobIndex->player_name ) )
			{
				found = true;
				sprintf( buf, "%s%s[%5d] %s %s<%3d> [ %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s]`x\r\n", 
					(pMobIndex->mob_triggers?"`Y*":" "),
					IS_SET(pMobIndex->act, ACT_AGGRESSIVE)?"`R":"`x",
					pMobIndex->vnum, 
					str_width(capitalize( pMobIndex->short_descr ),29,false),
					IS_SET(pMobIndex->act, ACT_SENTINEL)?"`B":"`x",
					pMobIndex->level,
					IS_SET(pMobIndex->affected_by,AFF_DETECT_INVIS)?"x":"",
					IS_SET(pMobIndex->affected_by,AFF_DETECT_HIDDEN)?"x":"",
					IS_SET(pMobIndex->affected_by,AFF_DARK_VISION)?"x":"",
					IS_SET(pMobIndex->affected_by,AFF_SANCTUARY)?"x":"",
					IS_SET(pMobIndex->affected_by,AFF_PASS_DOOR)?"x":"",
					IS_SET(pMobIndex->affected_by,AFF_FLYING)?"x":"",
					IS_SET(pMobIndex->affected_by,AFF_REGENERATION)?"x":"",
					IS_SET(pMobIndex->imm_flags,IMM_CHARM)?"x":"",
					IS_SET(pMobIndex->imm_flags,IMM_SUMMON)?"x":"",
					IS_SET(pMobIndex->imm_flags,IMM_SLEEP)?"x":"");
				add_buf( buf1, buf );
			}
		}
    }

    if ( !found )
    {
		ch->println("Syntax:  bmlist <all/name>");
		ch->println("Mobile(s) not found in this area.");
		free_buf(buf1);
		return;
    }
	add_buf( buf1,"  DI=detect invis, DH=detect hidden, DV=dark vision, SA=sanc, PD=passdoor,\r\n");
	add_buf( buf1,"  FL=Fly, RG=Regen, CH=Imm charm, SU=Imm Summon, SL=Imm Sleep,\r\n");
	add_buf( buf1,"  `Y*`x=Mprog, `RRED`x=Aggie, `BBlue`x=Sentiel (no wandering around)\r\n");

    ch->sendpage(buf_string(buf1));
    free_buf(buf1);
    return;
}
/**************************************************************************/
//  do_bmllist: balance mob list (by level) - Kal
void do_bmllist( char_data *ch, char *argument )
{
    MOB_INDEX_DATA	*pMobIndex;
    AREA_DATA		*pArea;
    char			buf[MSL];
    BUFFER			*buf1;
    char			arg[MIL];
    bool			fAll, found;
    int				vnum;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The bmlist command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_MOBS ) && !IS_IMMORTAL(ch))
    {
		ch->println("bmlist: Invalid security for viewing mobs in this area.");
		return;
    }

    one_argument( argument, arg );
	if ( !str_cmp(arg, "?") )
    {
		ch->println("Syntax:  bmlist <all/name>");
		return;
    }

    buf1  = new_buf();
    pArea = ch->in_room->area;
    fAll  = (!str_cmp( arg, "all" ) || IS_NULLSTR(arg));

	// go thru and find the highest and lowest levels.
	int lowestlevel=MAX_LEVEL*10;
	int highestlevel=-10;
    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
		if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
		{
			if ( fAll || is_name( arg, pMobIndex->player_name ) )
			{
				lowestlevel=UMIN(lowestlevel, pMobIndex->level);
				highestlevel=UMAX(highestlevel, pMobIndex->level);
			}
		}
    }



    found   = false;

	add_buf( buf1,
"*[mVNUM] Short Description             Level [ DI DH DV SA PD FL RG CH SU SL]\r\n");
//2345] 12345678901234567890123456789 <123> [%3s %3s %3s %3s %3s %3s %3s]");
	for(int lvl=lowestlevel; lvl<=highestlevel; lvl++){
		for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		{
			if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
			{
				if ( pMobIndex->level==lvl && (fAll || is_name( arg, pMobIndex->player_name )) )
				{
					found = true;
					sprintf( buf, "%s%s[%5d] %s %s<%3d> [ %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s]`x\r\n", 
						(pMobIndex->mob_triggers?"`Y*":" "),
						IS_SET(pMobIndex->act, ACT_AGGRESSIVE)?"`R":"`x",
						pMobIndex->vnum, 
						str_width(capitalize( pMobIndex->short_descr ),29,false),
						IS_SET(pMobIndex->act, ACT_SENTINEL)?"`B":"`x",
						pMobIndex->level,
						IS_SET(pMobIndex->affected_by,AFF_DETECT_INVIS)?"x":"",
						IS_SET(pMobIndex->affected_by,AFF_DETECT_HIDDEN)?"x":"",
						IS_SET(pMobIndex->affected_by,AFF_DARK_VISION)?"x":"",
						IS_SET(pMobIndex->affected_by,AFF_SANCTUARY)?"x":"",
						IS_SET(pMobIndex->affected_by,AFF_PASS_DOOR)?"x":"",
						IS_SET(pMobIndex->affected_by,AFF_FLYING)?"x":"",
						IS_SET(pMobIndex->affected_by,AFF_REGENERATION)?"x":"",
						IS_SET(pMobIndex->imm_flags,IMM_CHARM)?"x":"",
						IS_SET(pMobIndex->imm_flags,IMM_SUMMON)?"x":"",
						IS_SET(pMobIndex->imm_flags,IMM_SLEEP)?"x":"");
					add_buf( buf1, buf );
				}
			}
		}
	}

    if ( !found )
    {
		ch->println("Syntax:  bmlist <all/name>");
		ch->println("Mobile(s) not found in this area.");
		free_buf(buf1);
		return;
    }
	add_buf( buf1,"  DI=detect invis, DH=detect hidden, DV=dark vision, SA=sanc, PD=passdoor,\r\n");
	add_buf( buf1,"  FL=Fly, RG=Regen, CH=Imm charm, SU=Imm Summon, SL=Imm Sleep,\r\n");
	add_buf( buf1,"  `Y*`x=Mprog, `RRED`x=Aggie, `BBlue`x=Sentiel (no wandering around)\r\n");

    ch->sendpage(buf_string(buf1));
    free_buf(buf1);
    return;
}
/**************************************************************************/
// do_olist: shows all the objects and vnums in the current area
void do_olist( char_data *ch, char *argument )
{
	static int		colour;
    OBJ_INDEX_DATA	*pObjIndex;
    AREA_DATA		*pArea;
    char			buf[MSL];
    BUFFER			*buf1;
    char			arg[MIL];
    bool			fAll, found, cTest;
    int				vnum, stars;
    int				col = 0;
	char			vnum_text[MIL];

	if (!HAS_SECURITY(ch,1)){
		ch->println("The olist command is an olc command, you don't have olc permissions.");
		return;
	}

	if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_OBJECTS ) && !IS_IMMORTAL(ch))
	{
		ch->println("olist: Invalid security for viewing objects in this area.");
		return;
	}

	one_argument( argument, arg );

	if ( !str_cmp(arg, "?") )
	{
		ch->println("Syntax:  olist <ctest/all/name/item_type>");
		ch->println("note: Objects with () around their vnum don't have the take wear flag set.");
		return;
	}

	pArea = ch->in_room->area;
	buf1  = new_buf();
    fAll  = (!str_cmp( arg, "all" ) || IS_NULLSTR(arg));	

	cTest = false;   
	if (!str_cmp( arg, "ctest" ))
	{
		fAll = true;
		cTest = true;
		colour++;
	}
    found   = false;
	int item_type_arg=flag_value( item_types, arg );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
		pObjIndex = get_obj_index( vnum );
		if(!pObjIndex){
			continue;
		}		

		strcpy(vnum_text, mxp_create_send(ch,FORMATF("oedit %d", pObjIndex->vnum), FORMATF("%5d", pObjIndex->vnum)));
		if ( fAll || is_name( arg, pObjIndex->name )
		|| item_type_arg== pObjIndex->item_type )
		{
			if (cTest)
			{
				if (has_colour(pObjIndex->short_descr) 
					||has_colour(pObjIndex->description))
				{
					found = true;
					sprintf( buf, CC"x"CC"%s%s%s%s %s *",
						colour%2==0?"R":"G",
						pObjIndex->obj_triggers?"*":"[",
						vnum_text, 
						pObjIndex->obj_triggers?"*":"]",
						pObjIndex->short_descr);
					stars = 70-c_str_len(pObjIndex->short_descr);
					while (stars>0)
					{
						strcat(buf,"*");
						stars--;
					}
					strcat(buf,"\r\n");
					add_buf( buf1, buf );

					sprintf( buf, CC"x"CC"%s%sLong-%s %s *",
						colour%2==0?"B":"Y",
						pObjIndex->obj_triggers?"*":"[",
						pObjIndex->obj_triggers?"*":"]",
						pObjIndex->description);
					stars = 70-c_str_len(pObjIndex->description);
					while (stars>0)
					{
						strcat(buf,"*");
						stars--;
					}
				}
				else
				{
					continue;
				}
			}else{
				found = true;
				sprintf( buf, CC"x%s%s%s %s ", 
					(IS_SET(pObjIndex->wear_flags, OBJWEAR_TAKE)?
						(pObjIndex->obj_triggers?"*":"["):(pObjIndex->obj_triggers?"*":"(")),
					vnum_text, 
					(IS_SET(pObjIndex->wear_flags, OBJWEAR_TAKE)?
						(pObjIndex->obj_triggers?"*":"]"):")"),
					str_width(capitalize(pObjIndex->short_descr),29,true) );
			}
			add_buf( buf1, buf );
			if ( ++col % 2 == 0 || cTest){
				add_buf( buf1, "\r\n" );
			}
		}	
    }

	if (cTest && !found)
    {
		ch->println("There are no objects in the area you are in that have colour");
		ch->println("in either their short or long description.");
		return;
    }

    if ( !found )
    {
		ch->println("`xSyntax:  olist <all/name/item_type>");
		ch->println("To test the colour codes type `=Colist ctest`x");
		ch->println("Object(s) not found in this area.");

		if(item_type_arg!=NO_FLAG){
			ch->printlnf("Search included looking for objects of type %s",
				flag_string(item_types, item_type_arg));
		}
		return;
    }

    if ( col % 2 != 0 ){
		add_buf( buf1, "\r\n" );
	}
	add_buf( buf1, "[] around an object vnum represents a takeable object and () is untakeable, *=oprog.\r\n" );

	if(item_type_arg!=NO_FLAG){
		sprintf(buf, "Search included looking for objects of type %s\r\n",
			flag_string(item_types, item_type_arg));
		add_buf( buf1, buf);
	}

	if (cTest)
		ch->println("COLOUR TEST: The * on the right side should be the same colour as the vnums.");
    ch->sendpage(buf_string(buf1));

    free_buf(buf1);
    return;
}
/**************************************************************************/
//  do_bolist: balance object list directions - Kal
void do_bolist( char_data *ch, char *)
{
	ch->println("Use `=Cbovlist`x for balance object list sorted by vnum.");
	ch->println("Use `=Cbollist`x for balance object list sorted by level.");
}
/**************************************************************************/
// do_bovlist: shows all the objects and vnums in the current area 
//             sorted by vnum
void do_bovlist( char_data *ch, char *argument )
{
    obj_index_data	*pObjIndex;
    AREA_DATA		*pArea;
    char		buf  [MSL];
    BUFFER		*buf1;
    char		arg  [MIL];
    bool fAll, found;
    int vnum;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The bovlist command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_OBJECTS ) && !IS_IMMORTAL(ch))
    {
		ch->println("bovlist: Invalid security for listing objects in this area.");
		return;
    }

    one_argument( argument, arg );

	if ( !str_cmp(arg, "?") )
    {
		ch->println("Syntax:  bovlist <all/name/item_type>");
		return;
    }

    pArea = ch->in_room->area;
    buf1=new_buf();
    fAll    = (!str_cmp( arg, "all" ) || IS_NULLSTR(arg));

    found   = false;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
		if ( ( pObjIndex = get_obj_index( vnum ) ) )
		{
			if ( fAll || is_name( arg, pObjIndex->name )
			|| flag_value( item_types, arg ) == pObjIndex->item_type )
			{
				if (!found){
					add_buf( buf1, "[ Vnum] Short              <Lvl> [v0,v1,v2,v3,v4] - type\r\n");
				}
				found = true;
				sprintf( buf, CC"%c%s%5d%s %s <%3d> [%3d,%3d,%3d,%3d,%3d] - %s`x\r\n", 
					pObjIndex->affected?'R':'x',
					(IS_SET(pObjIndex->wear_flags, OBJWEAR_TAKE)?"[":"<"),
					pObjIndex->vnum, 
					(IS_SET(pObjIndex->wear_flags, OBJWEAR_TAKE)?"]":">"),
					str_width(capitalize(pObjIndex->short_descr),29,false),
					pObjIndex->level, 
					pObjIndex->value[0],
					pObjIndex->value[1],
					pObjIndex->value[2],
					pObjIndex->value[3],
					pObjIndex->value[4],
					flag_string( item_types, pObjIndex->item_type ) 
					);
				add_buf( buf1, buf );

			}
		}
    }

    if ( !found )
    {
		ch->println("`xSyntax:  bovlist <all/name/item_type>");
		ch->println("To test the colour codes type `=Cbovlist ctest`x");
		ch->println("Object(s) not found in this area.");
		return;
    }
	add_buf( buf1, "`RRed`x objects have additional affects on them.\r\n");

    ch->sendpage(buf_string(buf1));
    free_buf(buf1);
    return;
}
/**************************************************************************/
// do_bollist: shows all the objects and vnums in the current area 
//             sorted by level
void do_bollist( char_data *ch, char *argument )
{
    obj_index_data	*pObjIndex;
    AREA_DATA		*pArea;
    char		buf  [MSL];
    BUFFER		*buf1;
    char		arg  [MIL];
    bool fAll, found;
    int vnum;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The bollist command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( !IS_BUILDER( ch, ch->in_room->area, BUILDRESTRICT_OBJECTS ) && !IS_IMMORTAL(ch))
    {
		ch->println("bollist: Invalid security for listing objects in this area.");
		return;
    }

    one_argument( argument, arg );

	if ( !str_cmp(arg, "?") )
    {
		ch->println("Syntax:  bollist <all/name/item_type>");
		return;
    }

    pArea = ch->in_room->area;
    buf1=new_buf();
    fAll    = (!str_cmp( arg, "all" ) || IS_NULLSTR(arg));

	// go thru and find the highest and lowest levels.
	int lowestlevel=MAX_LEVEL*10;
	int highestlevel=-10;
    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
		if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
		{
			if(fAll || is_name( arg, pObjIndex->name )
				|| flag_value( item_types, arg ) == pObjIndex->item_type )
			{
				lowestlevel=UMIN(lowestlevel, pObjIndex->level);
				highestlevel=UMAX(highestlevel, pObjIndex->level);
			}
		}
    }

    found   = false;
	for(int lvl=lowestlevel; lvl<=highestlevel; lvl++){
		for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		{
			if ( ( pObjIndex = get_obj_index( vnum ) ) )
			{
				if ( pObjIndex->level==lvl && (fAll || is_name( arg, pObjIndex->name )
				|| flag_value( item_types, arg ) == pObjIndex->item_type ))
				{
					if (!found){
						add_buf( buf1, "[ Vnum] Short              <Lvl> [v0,v1,v2,v3,v4] - type\r\n");
					}
					found = true;
					sprintf( buf, CC"%c%s%5d%s %s <%3d> [%3d,%3d,%3d,%3d,%3d] - %s`x\r\n", 
						pObjIndex->affected?'R':'x',
						(IS_SET(pObjIndex->wear_flags, OBJWEAR_TAKE)?"[":"<"),
						pObjIndex->vnum, 
						(IS_SET(pObjIndex->wear_flags, OBJWEAR_TAKE)?"]":">"),
						str_width(capitalize(pObjIndex->short_descr),29,false),
						pObjIndex->level, 
						pObjIndex->value[0],
						pObjIndex->value[1],
						pObjIndex->value[2],
						pObjIndex->value[3],
						pObjIndex->value[4],
						flag_string( item_types, pObjIndex->item_type ) 
						);
					add_buf( buf1, buf );

				}
			}
		}
	}

    if ( !found )
    {
		ch->println("`xSyntax:  bollist <all/name/item_type>");
		ch->println("To test the colour codes type `=Cbollist ctest`x");
		ch->println("Object(s) not found in this area.");
		return;
    }
	
	add_buf( buf1, "`RRed`x objects have additional affects on them.\r\n");

    ch->sendpage(buf_string(buf1));
    free_buf(buf1);
    return;
}

/**************************************************************************/
void do_mshow( char_data *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    int value;
	void * pTemp = NULL;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The mshow command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( argument[0] == '\0' )
    {
		ch->println("Syntax:  mshow <vnum>");
		return;
    }

    if ( !is_number( argument ) )
    {
		ch->println("mshow: Numeric vnum is required.");
		return;
    }

	value = atoi( argument );
	if ( !( pMob = get_mob_index( value ) ))
	{
		ch->println("mshow:  That mobile does not exist.");
		return;
	}

	pTemp = ch->desc->pEdit;
	ch->desc->pEdit = (void *)pMob;

	// Fix for security breach in which a builder can view any item if
	// they are in their own area
	if ( !IS_BUILDER( ch, pMob->area, BUILDRESTRICT_MOBS ) && !IS_IMMORTAL(ch))
	{
		ch->println("mshow: Invalid security for viewing mobs in that area.");
	    ch->desc->pEdit = pTemp;		
		return;
	}

	medit_show( ch, argument );
	ch->desc->pEdit = pTemp;
    return; 
}
/**************************************************************************/
void do_mpshow( char_data *ch, char *argument )
{
    MUDPROG_CODE *pMcode;
    int value;
	void * pTemp = NULL;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The mpshow command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( argument[0] == '\0' )
    {
		ch->println("Syntax:  mpshow <vnum>");
		return;
    }

    if ( !is_number( argument ) )
    {
		ch->println("mpshow: Numeric vnum is required.");
		return;
    }

	value = atoi( argument );
	
	if (! (pMcode = get_mprog_index( value ) ) )
	{
		ch->println("mpshow: There is no mob prog with that vnum.");
		return;
	}
	pTemp = ch->desc->pEdit;
	ch->desc->pEdit=(void *)pMcode;

	// Fix for security breach in which a builder can view any mprog
	// with a security of 7+  now supports BUILDRESTRICT flags as well
	if ( !IS_BUILDER( ch, pMcode->area, BUILDRESTRICT_MUDPROGS )
	&& !IS_IMMORTAL(  ch ))
//	&& !HAS_SECURITY( ch, 7))
	{
		ch->println("mpshow: Invalid security for viewing mudprogs in that area.");
	    ch->desc->pEdit = pTemp;		
		return;
	}

    mpedit_show( ch, argument );
    ch->desc->pEdit = pTemp;
    return; 
}

/**************************************************************************/
// uses oedit_show to display info to the character
void do_oshow( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    int value;
	void * pTemp = NULL;
	char arg[MIL];

	if (!HAS_SECURITY(ch,1)){
		ch->println("The oshow command is an olc command, you don't have olc permissions.");
		return;
	}

	argument = one_argument( argument, arg );

    if(IS_NULLSTR(arg)){
		ch->println("Syntax:  oshow <vnum>");
		return;
    }

    if ( !is_number( arg) ){
       ch->println("oshow: Numeric vnum is required.");
       return;
    }

	value = atoi( arg);
	if ( !( pObj = get_obj_index( value ) ))
	{
		ch->printlnf("oshow: object %d does not exist.", value);
		return;
	}
	pTemp = ch->desc->pEdit;
	ch->desc->pEdit = (void *)pObj;

	// Fix for security breach in which a builder can view any item if
	// they are in their own area
	if ( !IS_BUILDER( ch, pObj->area, BUILDRESTRICT_OBJECTS ) && !IS_IMMORTAL(ch))
	{
		ch->println("oshow: Invalid security for viewing objects in that area.");
	    ch->desc->pEdit = pTemp;		
		return;
	}

    oedit_show( ch, argument );
    ch->desc->pEdit = pTemp;
    return; 
}
/**************************************************************************/
void do_rshow( char_data *ch, char *argument )
{
    ROOM_INDEX_DATA	*pRoom=ch->in_room;
	ROOM_INDEX_DATA	*pOriginalRoom=ch->in_room;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The rshow command is an olc command, you don't have olc permissions.");
		return;
	}

    if ( !IS_NULLSTR(argument))
    {
		if ( !is_number( argument ) )
		{
			ch->println("rshow: a Numeric vnum is required if you want a specific room.");
			return;
		}
		else
		{   
			pRoom = get_room_index(atoi( argument ));  
			if ( !pRoom)
			{
				ch->println("rshow: That room does not exist.");
				return;
			}
		}
	}

	// Fix for security breach in which a builder can view any room if
	// they are in their own area
	if ( !IS_BUILDER( ch, pRoom->area, BUILDRESTRICT_ROOMS ) && !IS_IMMORTAL(ch))
	{
		ch->println("rshow: Invalid security for viewing rooms in that area.");
		return;
	}

	void * pTemp = ch->desc->pEdit;
	ch->in_room=pRoom;
	redit_show(ch, "");
	ch->in_room=pOriginalRoom;
    ch->desc->pEdit = pTemp;
    return;
}

/**************************************************************************/
void help_update_greetings();
/**************************************************************************/
void hedit(char_data *ch, char *argument)
{
    help_data *pHelp;
    char arg[MSL];
    char command[MIL];
    int  cmd;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The hedit command is an olc command, you don't have olc permissions.");
		edit_done( ch );
		return;
	}

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_HELP(ch, pHelp);

    if ( !str_cmp(command, "done") )
    {
		if(HAS_MXP(ch)){
			ch->wraplnf("`#`S[%s] [%s] [%s] [%s]",
				mxp_create_send(ch,"hsave ."),
				mxp_create_send(ch,"helpcat"),
				mxp_create_send(ch,FORMATF("helpcat %s", flag_string(help_category_types, pHelp->category))),
				mxp_create_send(ch,FORMATF("hlist %s", pHelp->helpfile->file_name)));
			ch->printlnf("[%s]`&", mxp_create_send(ch,FORMATF("help %s",pHelp->keyword)));
		}
		edit_done( ch );
		return;
    }

	if (!HAS_SECURITY(ch,pHelp->helpfile->security))
	{
		do_modehelp(ch, argument);
		ch->println("Insufficient security to edit this help entry.");
		edit_done( ch );
		return;
	}

    if ( command[0] == '\0' )
    {
		hedit_show( ch, argument );
		return;
    }

    // Search Table and Dispatch Command. 
    for ( cmd = 0; hedit_table[cmd].name != NULL; cmd++ )
    {
        if ( !str_prefix( command, hedit_table[cmd].name ) )
        {
            if ( (*hedit_table[cmd].olc_fun) ( ch, argument ) ){
				SET_BIT( pHelp->helpfile->flags, HELPFILE_CHANGED);
            }
            return;
        }
    }

    // Default to Standard Interpreter.
    interpret( ch, arg );
    return;
}

/**************************************************************************/
void comedit( char_data *ch, char *argument )
{
	cmd_type	*pCmd;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_COMMAND( ch, pCmd);
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( get_trust( ch ) < COMEDIT_MINTRUST) {
		ch->println("Insufficient trust level to modify command parameters.");
		edit_done( ch );
		return;
	}

	if ( !str_cmp( command, "done" )) {
		ch->wraplnf("Ok - commands will autosave within 15 minutes, "
			"or you can force a save of the command table using `=C%s`x", 
			mxp_create_send(ch,"write_comtable"));
		edit_done( ch );
		return;
	}

	if ( IS_NULLSTR(command)) {
		comedit_show( ch, argument );
		return;
	}

//	Search sedit_table and interpret command
	for ( cmd = 0; comedit_table[cmd].name != NULL; cmd++ ) {

		if ( !str_prefix( command, comedit_table[cmd].name )) {
			if (( *comedit_table[cmd].olc_fun ) ( ch, argument )) {
				SET_BIT(COM_TABLE_FLAGS,COMEDIT_CHANGED);
				return;
			}else{
				return;
			}
		}
	}

//	Default to standard mud interpreter
	interpret( ch, arg );
	return;
}
/**************************************************************************/
void dedit( char_data *ch, char *argument )
{
    DEITY_DATA	*pDeity;
    char		command[MIL];
    char		arg[MIL];
    int			cmd;
	
    EDIT_DEITY( ch, pDeity );
    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );
	
    if ( !str_cmp(command, "done") )
    {
		edit_done( ch );
		return;
    }
	
    if ( command[0] == '\0' )
    {
		dedit_show( ch, argument );
		return;
    }
	
    // Search Table and Dispatch Command.
    for ( cmd = 0; dedit_table[cmd].name != NULL; cmd++ )
    {
        if ( !str_prefix( command, dedit_table[cmd].name ))
        {
            if (( *dedit_table[cmd].olc_fun) ( ch, argument ))
            {
				SET_BIT(DEITY_FLAGS,DEDIT_CHANGED);
				return;
            }
            else
				return;
        }
    }
	
    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}
/**************************************************************************/
void herbedit( char_data *ch, char *argument )
{
	HERB_DATA	*herb;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_HERB( ch, herb );
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done"))
	{
		edit_done( ch );
		ch->wraplnf("The changes you have made to the herbs will automatically be saved within "
			"15 minutes (or at the start of a hotreboot).  If you want to feel warm and "
			"fuzzy inside by saving the herbs manually use saveherbs.");
		ch->printlnf("`S[%s] [%s]`x", 
			mxp_create_send(ch, "saveherbs"),
			mxp_create_send(ch, "herbedit"));
		return;
	}

	if ( command[0] == '\0' )
	{
		herbedit_show( ch, argument );
		return;
	}

	// Search Table and Dispatch Command.
    for ( cmd = 0; herbedit_table[cmd].name != NULL; cmd++ )
	{
		if ( !str_prefix( command, herbedit_table[cmd].name ))
		{
			if (( *herbedit_table[cmd].olc_fun) ( ch, argument ))
			{
				SET_BIT( HERB_FLAGS, DEDIT_CHANGED );
			}
			return;
		}
	}

	// Default to Standard Interpreter
	interpret( ch, arg );
	return;
}


/**************************************************************************/
void mixedit( char_data *ch, char *argument )
{
	mix_data	*mix;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_MIX( ch, mix );
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done"))
	{
		edit_done( ch );
		ch->wraplnf("The changes you have made to the mixes will automatically be saved within "
			"15 minutes (or at the start of a hotreboot).  If you want to feel warm and "
			"fuzzy inside by saving the mixes manually use savemix.");
		ch->printlnf("`S[%s] [%s]`x", 
			mxp_create_send(ch, "savemix"),
			mxp_create_send(ch, "mixedit"));
		return;
	}

	if ( command[0] == '\0' )
	{
		mixedit_show( ch, argument );
		return;
	}

	// Search Table and Dispatch Command.
    for ( cmd = 0; mixedit_table[cmd].name != NULL; cmd++ )
	{
		if ( !str_prefix( command, mixedit_table[cmd].name ))
		{
			if (( *mixedit_table[cmd].olc_fun) ( ch, argument ))
			{
				SET_BIT( MIX_FLAGS, DEDIT_CHANGED );		// again, using the value of dedit cause I'm too lazy :)
				return;
			}
			else
				return;
		}
	}

	// Default to Standard Interpreter
	interpret( ch, arg );
	return;
}

/**************************************************************************/
// Kalahn - Sept 2000... (rehash) - should really be made generic *shrug*
void clanedit( char_data *ch, char *argument )
{
	CClanType *pClan;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_CLAN( ch, pClan);
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done"))
	{
		edit_done( ch );
		ch->wrapln("The changes you have made to the clans will automatically be saved within "
			"15 minutes (or at the start of a hotreboot).  If you want to feel warm and "
			"fuzzy inside by saving the clans manually use saveclans.");
		ch->printlnf("`S[%s] [%s]`x", 
			mxp_create_send(ch, "saveclans"),
			mxp_create_send(ch, "clanedit"));
		return;
	}

	if ( command[0] == '\0' )
	{
		clanedit_show( ch, argument );
		return;
	}

	// Search Table and Dispatch Command.
    for ( cmd = 0; clanedit_table[cmd].name != NULL; cmd++ )
	{
		if ( !str_prefix( command, clanedit_table[cmd].name ))
		{
			if (( *clanedit_table[cmd].olc_fun) ( ch, argument ))
			{
				SET_BIT( CLAN_FLAGS, DEDIT_CHANGED );
			}
			return;
		}
	}

	// Default to Standard Interpreter
	interpret( ch, arg );
	return;
}

/**************************************************************************/
void skillgroupedit( char_data *ch, char *argument )
{
	skillgroup_type *pGroup;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_SKILLGROUP( ch, pGroup);
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done"))
	{
		ch->wrapln("The changes you have made to the skillgroups will automatically be saved within "
			"15 minutes (or at the start of a hotreboot).  If you want to feel warm and "
			"fuzzy inside by saving the skillgroups manually use write_skillgroups.");
		ch->printlnf("`S[%s] [%s]`x", 
			mxp_create_send(ch, "write_skillgroups"),
			mxp_create_send(ch, "skgrpedit"));
		edit_done( ch );
		return;
	}

	if ( IS_NULLSTR(command) )
	{
		skillgroupedit_show( ch, argument );
		return;
	}

	// Search Table and Dispatch Command.
    for ( cmd = 0; skillgroupedit_table[cmd].name != NULL; cmd++ )
	{
		if ( !str_prefix( command, skillgroupedit_table[cmd].name ))
		{
			if (( *skillgroupedit_table[cmd].olc_fun) ( ch, argument ))
			{
				SET_BIT( SKILLGROUPEDIT_FLAGS, DEDIT_CHANGED );
			}
			return;
		}
	}

	// Default to Standard Interpreter
	interpret( ch, arg );
	return;
}


/**************************************************************************/
// Kalahn - April 2003 (rehash yet again) - should one day become generic
void langedit( char_data *ch, char *argument )
{
	language_data *pLang;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_LANGUAGE( ch, pLang);
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done"))
	{
		edit_done( ch );
		ch->wraplnf("The changes you have made to the languages will automatically be saved within "
			"15 minutes (or at the start of a hotreboot).  If you want to feel warm and "
			"fuzzy inside by saving the languages manually use write_languages.");
		ch->printlnf("`S[%s] [%s]`x", 
			mxp_create_send(ch, "write_languages"),
			mxp_create_send(ch, "langedit"));
		return;

		return;
	}

	if ( command[0] == '\0' )
	{
		langedit_show( ch, argument );
		return;
	}

	// Search Table and Dispatch Command.
    for ( cmd = 0; langedit_table[cmd].name != NULL; cmd++ )
	{
		if ( !str_prefix( command, langedit_table[cmd].name ))
		{
			if (( *langedit_table[cmd].olc_fun) ( ch, argument )){
				LANGUAGE_NEEDS_SAVING=true;
				SET_BIT( pLang->flags, LANGFLAG_CHANGED);
			}
			return;
		}
	}

	// Default to Standard Interpreter
	interpret( ch, arg );
	return;
}


/**************************************************************************/
void qedit( char_data *ch, char *argument )
{
	quest_type *pQuest;
	char		command[MIL];
	char		arg[MIL];
	int			cmd;

	EDIT_QUEST( ch, pQuest );
	smash_tilde( argument );
	strcpy( arg, argument );
	argument = one_argument( argument, command );


	if ( !str_cmp( command, "done" ))
	{
		pQuest->modified_date=current_time;
		edit_done( ch );
		return;
	}

	if ( IS_NULLSTR(command))
	{
		qedit_show( ch, argument );
		return;
	}

//	Search qedit_table and interpret command
	for ( cmd = 0; qedit_table[cmd].name != NULL; cmd++ )
	{
		if ( !str_prefix( command, qedit_table[cmd].name ))
		{
			if (( *qedit_table[cmd].olc_fun ) ( ch, argument ))
			{
				SET_BIT(QUEST_TABLE_FLAGS,QEDIT_CHANGED);
				return;
			}
			else
				return;
		}
	}

//	Default to standard mud interpreter
	interpret( ch, arg );
	return;
}
/**************************************************************************/
void do_ashow( char_data *ch, char *argument )
{
	AREA_DATA *pArea;

	void * pTemp = NULL;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The ashow command is an olc command, you don't have olc permissions.");
		return;
	}

	// find the area they want to look at
    if ( IS_NULLSTR(argument))
    {
		pArea=ch->in_room->area;
    }else{
		if ( !is_number( argument ) )
		{
			ch->println("ashow: Numeric vnum is required.");
			return;
		}

		pArea = get_area_data(atoi(argument)) ;
        if ( !pArea )
        {
            ch->println("ashow: There is no area with that number reference (as per alist).");
            return;
        }
	}

    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_AREA) && !IS_IMMORTAL(ch))
    {
		ch->printlnf("do_ashow: Invalid security for viewing the area '%s'", pArea->name);
		return;
    }

	pTemp = ch->desc->pEdit;
	ch->desc->pEdit = (void *)pArea;
 
    aedit_show( ch, "");
    ch->desc->pEdit = pTemp;
    return; 
}
/**************************************************************************/
void do_autoreset( char_data *ch, char *argument )
{
    RESET_DATA *pReset = NULL;
	ROOM_INDEX_DATA *pRoom = ch->in_room;
	char_data  *pMob, *pMob_next;
	OBJ_DATA   *pObj, *pCont;
	int	resetcount=0;

	// do the checks to make sure they can use the command here
	if (!HAS_SECURITY(ch,1)){
		ch->println("The autoreset command is an olc command, you don't have olc permissions." );
		return;
	}

	if(str_cmp(argument,"confirm")){
		ch->println("syntax: autoreset confirm");
		ch->println("Autoreset clears all the resets in a given room, and takes a 'snapshot'");
		ch->println("of the current room configuration - writing it as a new set of resets.");
		ch->println("To use it while building, load a room up with the objects and mobs you ");
		ch->println("want, forcing the mobs to wear/hold objects etc, then autoreset confirm");
		return;
	}

	if(!pRoom){
		ch->println( "BUG: Your room pointer is NULL!");
		return;
	}
	if(!pRoom->area)
	{
		ch->println( "BUG: The room you are in doesn't have an area associated with it!");
		return;
	}
    if ( !IS_BUILDER( ch, pRoom->area, BUILDRESTRICT_RESETS ) )
    {
		ch->println( "Resets: Invalid security for setting resets in this area." );
		return;
    }

//  No matter the outcome, we'll flag the area as being changed...

	MARK_CHANGED(pRoom);

//	First off, delete any resets in this room, wipe em all!

	if ( !pRoom->reset_first )
	{
		ch->println( "There were no resets in this room." );
	}
	else
	{
		// Are we leaking yet? Are we?
		pRoom->reset_first = NULL;
		pRoom->reset_last  = NULL;
		ch->println( "All existing resets were annihilated." );
	}

//  Let them know what's going on...

	ch->println( "Searching for MOBS to be reset in this room...." );

	for ( pMob = pRoom->people; pMob; pMob = pMob_next )
	{

		pMob_next = pMob->next_in_room;
		
		if (!IS_NPC(pMob))
			continue;

		++resetcount;

		pReset = new_reset_data();
		pReset->command = 'M';
		pReset->arg1	= pMob->pIndexData->vnum;
		pReset->arg2	= 1;						// Max in world
		pReset->arg3	= pRoom->vnum;
		pReset->arg4	= 1;						// Min #

		ch->printlnf( ">MOB:   `Y%s`x reset into room.", pMob->short_descr );
		add_reset( pRoom, pReset, resetcount );

		// Now loop thru the mob's eq list and reset that too

		for ( pObj = pMob->carrying; pObj; pObj = pObj->next_content )
		{
			++resetcount;

			pReset = new_reset_data();
			
			pReset->arg1 = pObj->pIndexData->vnum;
			pReset->arg2 = -1;						// Max in world -- unlimited
			pReset->arg4 = 0;
			
			// WEAR_NONE goes to INV all else goes to EQ
			if ( pObj->wear_loc == WEAR_NONE )
			{
				pReset->command = 'G';
				ch->printlnf( ">>OBJ:  `y%s`x reset in INV of %s.", pObj->short_descr, pMob->short_descr );
				pReset->arg3 = 0;					// WEAR_NONE
			}
			else
			{
				pReset->command = 'E';
				pReset->arg3 = pObj->wear_loc;// - OBJWEAR_TAKE;     // ???

				ch->printlnf( ">>OBJ:  `y%s`x reset in EQ of %s.", pObj->short_descr, pMob->short_descr );
			}

			add_reset( pRoom, pReset, resetcount );

			// Container/Corpse(NPC)
			if (( pObj->item_type == ITEM_CONTAINER  )
			||	( pObj->item_type == ITEM_CORPSE_NPC )
			||  ( pObj->item_type == ITEM_CAULDRON   )
			||  ( pObj->item_type == ITEM_FLASK		)
			||  ( pObj->item_type == ITEM_MORTAR    ))
			{
				for ( pCont = pObj->contains; pCont; pCont = pCont->next_content )
				{
					++resetcount;

					pReset = new_reset_data();
					pReset->command = 'P';
					pReset->arg1	= pCont->pIndexData->vnum;
					pReset->arg2    = -1;
					pReset->arg3    = pObj->pIndexData->vnum;
					pReset->arg4    = 1;
					
					ch->printlnf( ">>>OBJ: `s%s`x reset inside %s.", pCont->short_descr, pObj->short_descr );
					add_reset( pRoom, pReset, resetcount );
				}
			}
		}
	}
	
//  All mobs and their eq cycled!  Now Room objects etc

	ch->println( "Searching for OBJS to be reset in this room...." );

	for ( pObj = pRoom->contents; pObj; pObj = pObj->next_content )
	{
		++resetcount;

		pReset = new_reset_data();
		pReset->command = 'O';
		pReset->arg1	= pObj->pIndexData->vnum;
		pReset->arg2	= -1;			// Max world - unlimited
		pReset->arg3	= pRoom->vnum;
		pReset->arg4	= 0;

		ch->printlnf( ">OBJ:  `B%s`x reset into the room.", pObj->short_descr );
		add_reset( pRoom, pReset, resetcount );

		// Container/Corpse(NPC)
		if (( pObj->item_type == ITEM_CONTAINER  )
		||	( pObj->item_type == ITEM_CORPSE_NPC )
		||	( pObj->item_type == ITEM_FLASK		 )
		||  ( pObj->item_type == ITEM_MORTAR     )
		||  ( pObj->item_type == ITEM_CAULDRON  ))
		{
			for ( pCont = pObj->contains; pCont; pCont = pCont->next_content )
			{
				++resetcount;

				pReset = new_reset_data();
				pReset->command = 'P';
				pReset->arg1	= pCont->pIndexData->vnum;
				pReset->arg2    = -1;
				pReset->arg3    = pObj->pIndexData->vnum;
				pReset->arg4    = 1;

				ch->printlnf( ">>OBJ: `b%s`x reset inside %s.", pCont->short_descr, pObj->short_descr );
				add_reset( pRoom, pReset, resetcount );
			}
		}
	}
	return;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
