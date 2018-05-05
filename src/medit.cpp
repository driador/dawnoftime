/**************************************************************************/
// medit.cpp - olc mob edit
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
#include "security.h"
#include "shop.h"

DECLARE_OLC_FUN( medit_create );
GAMBLE_FUN *gamble_lookup( const char *name );
char *gamble_name( GAMBLE_FUN *function);
DECLARE_OLC_FUN( medit_autostat	);
/**************************************************************************/
extern gameset_value_type gameset_value[];

/**************************************************************************/
void do_medit( char_data *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    char arg1[MSL];

	if (IS_NPC(ch) || ch->pcdata->security<2)
	{
		ch->println("You must have an olc security 2 or higher to use this command.");
		return;
	}

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( !( pMob = get_mob_index( value ) ))
        {
            ch->println("MEdit:  That vnum does not exist.");
            return;
        }
    
        if ( !IS_BUILDER( ch, pMob->area, BUILDRESTRICT_MOBS ) )
        {
            ch->println("Insufficient security to modify mob.");
            return;
        }

		// officially reserved vnum range
		if(value<500){
			ch->println("Warning: all mobs, rooms and objects below vnum 500 are officially reserved for the dawn codebase.");
			if(!HAS_SECURITY(ch,9)){
				ch->println("As a result of this reservation, only those with security 9 can edit in that vnum range.");
	            return;
			}			
		}
    
        ch->desc->pEdit = (void *)pMob;
        ch->desc->editor = ED_MOBILE;

        // inform the builder they are now in medit mode
		ch->wraplnf("`=rYou are now editing mob: '`r%s`=r' vnum: `Y%d`x", 
				pMob->short_descr, pMob->vnum);
		ch->println("`=rType `=Cdone`=r to finish editing.");   
        return;
    }

    if ( !str_cmp( arg1, "create" ) )
    {
        value = atoi( argument );
        if ( arg1[0] == '\0' || value == 0 )
        {
            ch->println("Syntax:  medit create <vnum>");
            ch->println("    or:  medit <vnum>");
            return;
        }

		// officially reserved vnum range
		if(value<500){
			ch->println("Warning: all mobs, rooms and objects below vnum 500 are officially reserved for the dawn codebase.");
			if(!HAS_SECURITY(ch,9)){
				ch->println("As a result of this reservation, only those with security 9 can edit in that vnum range.");
				return;
			}			
		}


        pArea = get_vnum_area( value );

        if ( !pArea )
        {
            ch->println("MEdit:  That vnum is not assigned an area.");
            return;
        }

        if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_MOBS ) )
        {
            ch->println("Insufficient security to modify mob.");
            return;
        }

        if ( medit_create( ch, argument ) )
        {
            SET_BIT( pArea->olc_flags, OLCAREA_CHANGED );
            ch->desc->editor = ED_MOBILE;
            ch->printlnf("You are now creating a mob of vnum %d.",value);               
        }
        return;
    }
    ch->println("Syntax:  medit create <vnum>");
    ch->println("    or:  medit <vnum>");
}
/**************************************************************************/
bool medit_show(char_data *ch, char *)
{
    MOB_INDEX_DATA *pMob;
    MUDPROG_TRIGGER_LIST *list;

    EDIT_MOB(ch, pMob);

	if(IS_SET(ch->dyn,DYN_SHOWFLAGS)){
		ch->titlebarf("MEDIT SHOWFLAGS %d: Level %d - '%s'", 
			pMob->vnum, pMob->level, pMob->player_name);
	}else{
		ch->titlebarf("MEDIT SHOW %d", pMob->vnum);
		ch->printlnf("`=rVnum: `=v%-5d   `=rLevel: `=v%-3d   `=rArea[%d]: `=x%s", 
			pMob->vnum,  pMob->level,
			pMob->area ? pMob->area->vnum : 0,
			pMob->area ? pMob->area->name : "`=XNo Area!!!`x");

		ch->printlnf("`=rName: `=x%s",  pMob->player_name);

		ch->printf("`=rDescription:\r\n`=x  %s", pMob->description );
		ch->printlnf("`=rShort description:  `=x%s", pMob->short_descr);
		if (has_colour(pMob->short_descr))
		{
			ch->println("`sShort description:  `=x");
			ch->printlnbw(pMob->short_descr);
		}

		ch->println("`=rDefault description: (description used if mob is in default position)`=x");
		ch->printlnf("`=x  %s", pMob->long_descr );

		if (has_colour(pMob->long_descr))
		{
			ch->print( "`sDefault description:  `=x" );
			ch->printlnbw(pMob->long_descr);
		}
	}

	if(IS_SET(ch->dyn,DYN_SHOWFLAGS)){
		mxp_display_olc_flags(ch, position_types,	pMob->default_pos,	"position default",	"Default Position:");
		mxp_display_olc_flags(ch, position_types,	pMob->start_pos,	"position start",	"Start Position:");
		mxp_display_olc_flags(ch, size_types,		pMob->size,			"size",				"Size:");
		mxp_display_olc_flags(ch, sex_types,		pMob->sex,			"sex",				"Sex:");
	}else{
		ch->printf("`=rDefault pos:   `=V%-16s",
			flag_string( position_types, pMob->default_pos ) );
		ch->printlnf("`=rStart pos: `=V%-10s",
			flag_string( position_types, pMob->start_pos ) );

		ch->printf("`=rRace: `=V%-8s ", race_table[pMob->race]->name );
		ch->printf("`=rSize: `=V%-10s",
			flag_string( size_types, pMob->size ) );
		ch->printf("`=rSex: `=R%-9s",
			pMob->sex == SEX_MALE    ? "male"   :
			pMob->sex == SEX_FEMALE  ? "female" : 
			pMob->sex == 3           ? "random" : "neutral" );
		ch->printlnf("`=rMaterial: `=x%s", pMob->material );
	}


	if(!IS_SET(ch->dyn,DYN_SHOWFLAGS)){
		ch->printf("`=rAlliance: `=v%-+2d   ", pMob->alliance);
		ch->printf("`=rTendency: `=v%-+2d    ", pMob->tendency);
		ch->printf("`=rWealth:  `=v%-5ld",	pMob->wealth );
		ch->printf("`=rGroup: `=v%-5d     `=rHelpgroup: `=v%-5d", 
			pMob->group, pMob->helpgroup );
		
		// xp_mod
		if(IS_ADMIN(ch)){
			if (pMob->xp_mod == 100)
			{
				ch->printlnf("`=rxpmod: `=v%-3d",  pMob->xp_mod);
			}else{
				ch->printlnf("`rxpmod: `=X%-3d",  pMob->xp_mod);
			}
		}else{
			if (pMob->xp_mod != 100) // only displayed if unusual to nonadmin
			{
				ch->printlnf("`=rxpmod: `=X%-3d",  pMob->xp_mod);
			}
		}

		ch->printf("`=rHitroll: `=v%-3d   ", pMob->hitroll );

		ch->printf("`=rDamType: `=V%s  ",
    			 attack_table[pMob->dam_type].name );

		ch->printlnf("`=rDamage dice: [`=v%2dd%-3d+%4d`=r]  (avedam %d)",
			 pMob->damage[DICE_NUMBER],
			 pMob->damage[DICE_TYPE],
			 pMob->damage[DICE_BONUS],	 
				(( pMob->damage[DICE_NUMBER] +(pMob->damage[DICE_NUMBER]*pMob->damage[DICE_TYPE]) )/2
				+pMob->damage[DICE_BONUS])
			 );

		ch->println("`=rNote the DamType and Damage Dice are only used when the mob doesn't wield a weapon.`x");

		ch->printf("`=rHitpoints dice: [`=v%2dd%-3d+%4d`=r]       ",
		 pMob->hit[DICE_NUMBER],
		 pMob->hit[DICE_TYPE],
		 pMob->hit[DICE_BONUS] );
		ch->printlnf("`=rMana dice: [`=v%2dd%-3d+%4d`=r]",
			 pMob->mana[DICE_NUMBER],
			 pMob->mana[DICE_TYPE],
			 pMob->mana[DICE_BONUS] );

		ch->printlnf("`=rArmor:         [`=Vpierce: `=v%d  `=Vbash: `=v%d "
			" `=Vslash: `=v%d  `=Vmagic: `=v%d`=r]",
			pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
			pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC] );
	}

	mxp_display_olc_flags(ch, affect_flags,		pMob->affected_by,	"affect",	"Affected By:");
	mxp_display_olc_flags(ch, affect2_flags,	pMob->affected_by2,	"affect2",	"Affected2 By:");
	mxp_display_olc_flags(ch, affect3_flags,	pMob->affected_by3,	"affect3",	"Affected3 By:");
	mxp_display_olc_flags(ch, act_flags,	pMob->act,	"act",	"Action:");
	mxp_display_olc_flags(ch, act2_flags,	pMob->act2,	"act2", "Action2:");
	mxp_display_olc_flags(ch, form_flags,	pMob->form,	"form", "Form:");
	mxp_display_olc_flags(ch, part_flags,	pMob->parts,"part", "Parts:");
   	mxp_display_olc_flags(ch, imm_flags,	pMob->imm_flags,	"imm", "Immunities:");
   	mxp_display_olc_flags(ch, res_flags,	pMob->res_flags,	"res", "Resistances:");
   	mxp_display_olc_flags(ch, vuln_flags,	pMob->vuln_flags,	"vuln","Vulnerablities:");
   	mxp_display_olc_flags(ch, off_flags,	pMob->off_flags,	"off", "Offensive:");

	if(!IS_SET(ch->dyn,DYN_SHOWFLAGS)){
		if ( pMob->spec_fun ){
			ch->printlnf("`=rSpecial function: `=X%s",  spec_name( pMob->spec_fun ) );
		}

		if ( pMob->gamble_fun ){
			ch->printlnf("`=rGambling Game: `=X%s",  gamble_name( pMob->gamble_fun ));
		}

		if ( pMob->pInnData ) {
			cInnData* pInn = pMob->pInnData;

			ch->printlnf(
				"`=rInn data for [%5d]:\r\n"
				"`=r  Markup for purchaser: `=v%d%%\r\n"
				"`=r  Markdown for seller:  `=v%d%%",
					pInn->vnKeeper, pInn->profit_buy, pInn->profit_sell );
			
			ch->printlnf("`=r  Hours: `=v%d `=rto `=v%d.",
				pInn->open_hour, pInn->close_hour );

			ch->println("`=r  Room    VNum    Silver per day");
			ch->println("`=w  ---- ------- -----------------");
			for(int i=0; i<MAX_INN;i++){
				ch->printlnf("`=r     %d  `=v%6d          `=w%8d`x", 
					i+1, pInn->vnRoom[i], (int)pInn->shRate[i]);
			}
		}

		if ( pMob->pShop )	{
			SHOP_DATA *pShop;
			int iTrade;
			pShop = pMob->pShop;
			
			ch->printf(
				"`=rShop data for [%5d]:\r\n"
				"`=r  Markup for purchaser: `=v%d%%\r\n"
				"`=r  Markdown for seller:  `=v%d%%\r\n",
					pShop->keeper, pShop->profit_buy, pShop->profit_sell );
			ch->printlnf("`=r  Hours: `=v%d `=rto `=v%d.",
				pShop->open_hour, pShop->close_hour );

			for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
			{
				if ( pShop->buy_type[iTrade] != 0 )
				{
					if ( iTrade == 0 )
					{
						ch->println("`=r  Number Trades Type");
						ch->println("  ------ -----------");
					}
					ch->printlnf("`=r  [%4d] `=V%s`x",
						iTrade,
						flag_string( item_types, pShop->buy_type[iTrade] ));
				}
			}
		}

		if ( pMob->mob_triggers )
		{
			int cnt;
			
			ch->printlnf("\r\n`=rMUDPrograms for [%5d]:", pMob->vnum);
			ch->printlnf("Mudprogs on this mob %s ignore questers", 
				IS_SET(pMob->act,ACT_MPIGN_QUESTER)?"WILL":"will not");
			ch->printlnf("Mudprogs on this mob %s ignore non questers", 
				IS_SET(pMob->act,ACT_MPIGN_NONQUESTER)?"will":"WILL NOT");	

			if(pMob->mprog_flags==0 && pMob->mprog2_flags==0){
				ch->printlnf("Mob mudprog triggers: none");
			}else{
				ch->wraplnf("Mob mudprog triggers: %s%s%s", 
					pMob->mprog_flags?flag_string(mprog_flags, pMob->mprog_flags):"",
					pMob->mprog_flags?" ":"",
					pMob->mprog2_flags?flag_string(mprog2_flags, pMob->mprog2_flags):"");
			}
			
			for (cnt=0, list=pMob->mob_triggers; list; list=list->next)
			{
				if (cnt ==0)
				{
					ch->println("`=r Number Vnum Trigger Phrase   <Positions>");
					ch->println(" ------ ---- ------- --------------------");
				}		
				ch->printf("`=r[%5d] `=v%s `=V%7s `=x%s ", 
					cnt++,
					mxp_create_tagf(ch, "mprogvnum", "%5d", list->prog->vnum),
					list->trig_type?flag_string(mprog_flags, list->trig_type):flag_string(mprog2_flags, list->trig_type),
					list->trig_phrase);
				if(list->pos_flags){
					ch->printlnf("`=V  <%s>",	flag_string(position_flags,list->pos_flags));
				}else{
					ch->println("`=V  <all possible>");
				}
			}
		}
	}
	ch->print("`x");
    return false;
}
/**************************************************************************/
bool medit_create(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    int iHash;
	int race;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
    ch->println("Syntax:  medit create [vnum]");
	return false;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
        ch->println("MEdit:  That vnum is not assigned an area.");
		return false;
    }

    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_MOBS ) )
    {
        ch->println("MEdit:  Vnum in an area you cannot build in.");
		return false;
    }

    if ( get_mob_index( value ) )
    {
        ch->println("MEdit:  Mobile vnum already exists.");
		return false;
    }

    pMob				= new_mob_index();
    pMob->vnum			= value;
    pMob->area			= pArea;
        
    if ( value > top_vnum_mob )
	{
		top_vnum_mob = value;
	}

    pMob->act			= ACT_IS_NPC | ACT_STAY_AREA | ACT_NO_TAME;
	pMob->xp_mod		= 100; // set to 100% default
    pMob->hit[DICE_BONUS]	 = 1;
    pMob->mana[DICE_BONUS]	 = 1;
    pMob->damage[DICE_BONUS] = 1;

    // set the flags for the race
	if ( ( race = race_lookup( "human") ) != -1 )
    {
		pMob->race = race;
		pMob->act		  |= race_table[race]->act;
		pMob->affected_by |= race_table[race]->aff;
		pMob->off_flags   |= race_table[race]->off;
		pMob->imm_flags   |= race_table[race]->imm;
		pMob->res_flags   |= race_table[race]->res;
		pMob->vuln_flags  |= race_table[race]->vuln;
		pMob->form        |= race_table[race]->form;
		pMob->parts       |= race_table[race]->parts;	
    }

    iHash			= value % MAX_KEY_HASH;
    pMob->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMob;
    ch->desc->pEdit		= (void *)pMob;

    ch->printlnf("Mobile %d Created (stored in %s).", 
			pMob->vnum, pMob->area->name);
    return true;
}

/**************************************************************************/
bool medit_spec(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
        ch->println("Syntax:  spec [special function]");
        ch->println(" To clear a mob special function: spec none\r\n");
        ch->println("`=rSelect a special from below:`=R");
		show_help (ch, "spec");
		ch->print("`x");
		return false;
    }


    if ( !str_cmp( argument, "none" ) )
    {
        pMob->spec_fun = NULL;

        ch->println("Spec removed.");
        return true;
    }

    if ( spec_lookup( argument ) )
    {
		pMob->spec_fun = spec_lookup( argument );
        ch->printlnf("Spec set to %s.", spec_name( pMob->spec_fun ));
		return true;
    }

    ch->println("MEdit: No matching special function.");
    ch->print("`=rSelect one from below:\r\n`=R");
	show_help (ch, "spec");
	ch->print("`x");
		
    return false;
}

/**************************************************************************/
bool medit_damtype (char_data *ch, char *argument)
{
	MOB_INDEX_DATA *pMob;
	
	EDIT_MOB(ch, pMob);
	if ( argument[0] == '\0' )
    {
		ch->println("Syntax:  damtype [damage message]");
		ch->println("To see a list of damage messages type '? weapon'.");
		return false;
	}
	pMob->dam_type = attack_lookup(argument);
	ch->println("Damage type set.");
	return true;
}


/**************************************************************************/
bool medit_alliance (char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    int value;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
    ch->println("Syntax:  alliance [number]");
	return false;
    }

    value = atoi( argument );

    if (value>3 || value<-3)
    {
        ch->println("Alliance must be in the range -3 thru 3.");
        return false;
    }
    pMob->alliance = value;
    ch->printlnf("Alliance set to %d.", pMob->alliance);
    return true;
}

/**************************************************************************/
bool medit_tendency(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    int value;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        ch->println("Syntax:  tendency [number]");
        return false;
    }

    value = atoi( argument );

    if (value>3 || value<-3)
    {
        ch->println("Tendency must be in the range -3 thru 3.");
        return false;
    }
    pMob->tendency = value;
    ch->printlnf("Tendency set to %d.", pMob->tendency);
    return true;
}

/**************************************************************************/
bool medit_level (char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	int value;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
		ch->println("Syntax:  level [number]");
		return false;
    }

	value=atoi( argument );
    if( pMob->level == value){
		ch->println("Mobile level unchanged.");
	};
    ch->printlnf("Mobile level changed from %d to %d.", 
		pMob->level, value);
	pMob->level = value;

	medit_autostat(ch,""); // this will eventually be optional
    return true;
}


/**************************************************************************/
bool medit_xpmod(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	int old;

    EDIT_MOB(ch, pMob);
    
	if (!HAS_SECURITY(ch,MEDIT_XPMOD_MINSECURITY))
	{
        ch->printf("You require a minimum security of %d to set xp modifiers on mobs.\n", 
			MEDIT_XPMOD_MINSECURITY);
		return false;
	}

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        ch->println("Syntax:  xpmod [number]");
		return false;
    }

	old=pMob->xp_mod;

	if (old == atoi(argument))
	{
        ch->printlnf("xpmod was already %d.", old);
		return false;
	}

    pMob->xp_mod = atoi( argument );

    ch->printlnf("xpmod changed from %d to %d.", 
		old, pMob->xp_mod);
    return true;
}

/**************************************************************************/
bool medit_desc(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
		if(ch->desc && pMob->area){ 
			// if the string as changed, this is used to flag the area file saving
			ch->desc->changed_flag=&pMob->area->olc_flags;
		}
		string_append( ch, &pMob->description );
		return true;
    }

    ch->println("Syntax:  desc    - line edit");
    return false;
}

/**************************************************************************/
bool medit_wrap( char_data *ch, char * )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

	pMob->description= format_string( pMob->description );
    
	ch->println("Mob description wordwrapped.");
    return true;
}
/**************************************************************************/
bool medit_long(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) ) {
		ch->println("Syntax:  long [string]");
		return false;
    }

    free_string( pMob->long_descr );
    pMob->long_descr = str_dup( argument );
    pMob->long_descr[0] = UPPER( pMob->long_descr[0]  );

    ch->println("Default description set.");
    return true;
}


/**************************************************************************/
bool medit_short(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);
    
    // trim the spaces to the right of the short
    while ( !IS_NULLSTR(argument) && is_space(argument[str_len(argument)-1])){
        argument[str_len(argument)-1]='\0';
    }

    if ( IS_NULLSTR(argument) ){
        ch->println("Syntax:  short <string>");
		ch->println("Note: <string> is forced to lowercase unless there are colour codes in it.");
		return false;
    }

	if(has_colour(argument)){
		ch->wraplnf( "Changed mobile short description from '%s' to '%s' "
			"(lower case not forced due to colour codes).",
			pMob->short_descr, argument);
		replace_string(pMob->short_descr, argument);
	}else{
		ch->printlnf( "Changed mobile short description from '%s' to '%s'.",
			pMob->short_descr, lowercase(argument));
		replace_string(pMob->short_descr, lowercase(argument));
	}
    return true;
}

/**************************************************************************/
bool medit_name(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) )
    {
		ch->println("Syntax:  name <string>");
		return false;
    }

	ch->printlnf( "Mobile name changed from '%s' to '%s'.", pMob->player_name, lowercase(argument));
    replace_string( pMob->player_name, lowercase(argument));

    return true;
}

/**************************************************************************/
bool medit_inn(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    char command[MIL];
    char arg1[MIL];

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg1 );

    EDIT_MOB(ch, pMob);

    if ( command[0] == '\0' )
    {
		ch->println("Syntax:  inn hours [#xopening] [#xclosing]");
		ch->println("         inn profit [#xbuying%%] [#xselling%%]");
		ch->println("         inn room  [#xroom number] [#xvnum]");
		ch->println("         inn rate  [#xroom number] [#xsilver/day]");
		ch->println("         inn assign");
		ch->println("         inn remove");
		return false;
    }

	if ( !str_cmp(command, "room") ) {
		if ( arg1[0] == '\0' || argument[0] == '\0' || !is_number(arg1) || !is_number(argument) ) {
			ch->println("Syntax:  inn room [#xroom number] [#xvnum]");
			return false;
		}

		if ( !pMob->pInnData ) {
			ch->println("MEdit: You should create an inn first (inn assign).");
			return false;
		}

		const int roomnumber = atoi(arg1);
		if(roomnumber<1 || roomnumber>MAX_INN){
			ch->printlnf("The room number must be between 1 and %d", MAX_INN);
			return false;
		}

		const int roomvnum= atoi(argument);
		if ( get_room_index(roomvnum) == NULL && roomvnum!=0) {
			ch->printlnf("Room %d does not exist.", roomvnum);
			return false;
		}

		if(roomvnum){ // setting/changing
			ch->printlnf("Inn %d room vnum changed from %d to %d.", 
				roomnumber,
				pMob->pInnData->vnRoom[roomnumber-1],
				roomvnum);

		}else{ // clearing
			ch->printlnf("Inn %d room vnum cleared (was %d).", 
				roomnumber,
				pMob->pInnData->vnRoom[roomnumber-1]);

		}
		pMob->pInnData->vnRoom[roomnumber-1] = roomvnum;
		return true;
	}

	if ( !str_cmp(command, "rate") ) {
		if ( arg1[0] == '\0' || argument[0] == '\0' || !is_number(arg1) || !is_number(argument) ) {
			ch->println("Syntax:  inn rate  [#xroom number] [#xsilver/day]");
			return false;
		}

		if ( !pMob->pInnData ) {
			ch->println("MEdit: You should create an inn first (inn assign).");
			return false;
		}

		const int roomnumber= atoi(arg1);
		if(roomnumber<1 || roomnumber>MAX_INN){
			ch->printlnf("The room number must be between 1 and %d", MAX_INN);
			return false;
		}

		const int roomrate	= atoi(argument);
		if ( roomrate<0 ) {
			ch->println("The amount of silver per day has to be a positive number.");
			return false;
		}
		
		ch->printlnf("Inn %d room rate changed from %d to %d silver.", 
			roomnumber,
			pMob->pInnData->shRate[roomnumber-1],
			roomrate);
		
		pMob->pInnData->shRate[roomnumber-1] = roomrate;
		return true;
	}

    if ( !str_cmp( command, "hours" ) )
	{
		if ( arg1[0] == '\0' || !is_number( arg1 )
			|| argument[0] == '\0' || !is_number( argument ) )
		{
			ch->println("Syntax:  inn hours [#xopening] [#xclosing]");
			return false;
		}
		if ( !pMob->pInnData )
		{
			ch->println("MEdit: You should create an inn first (inn assign).");
			return false;
		}
		
		pMob->pInnData->open_hour = atoi( arg1 );
		pMob->pInnData->close_hour = atoi( argument );
		ch->println("Inn hours set.");
		return true;
	}

    if ( !str_cmp( command, "profit" ) )
	{
		if ( arg1[0] == '\0' || !is_number( arg1 )
			|| argument[0] == '\0' || !is_number( argument ) )
		{
			ch->println("Syntax:  shop profit [#xbuying%] [#xselling%]");
			return false;
		}
		if ( !pMob->pInnData )
		{
			ch->println("MEdit: You should create an inn first (inn assign).");
			return false;
		}

		pMob->pInnData->profit_buy     = atoi( arg1 );
		pMob->pInnData->profit_sell    = atoi( argument );
		ch->println("Inn profit set.");
		return true;
    }

	if ( !str_prefix(command, "assign") ) {
		if ( pMob->pInnData ) {
			ch->println("Mob already has an inn assigned to it.");
			return false;
		}

		pMob->pInnData = new cInnData;
		top_inn++;
		if ( !pFirstInn ) {
			pFirstInn=pMob->pInnData;
		}
		if ( pLastInn ) {
			pLastInn->pNextInn = pMob->pInnData;
		}

		pLastInn			= pMob->pInnData;
		pLastInn->pNextInn	= NULL;

		pMob->pInnData->vnKeeper = pMob->vnum;

		ch->println("New inn assigned to mobile.");
		return true;
	}

	if ( !str_prefix(command, "remove") ) {
		cInnData* pInn;

		if ( (pInn=pMob->pInnData) == NULL ) {
			ch->println("This mobile does not have any inn data to remove.");
			return false;
		}

		pMob->pInnData = NULL;
		
		if ( pInn == pFirstInn ) {
			if ( (pFirstInn=pInn->pNextInn) == NULL ) {
				pLastInn=NULL;
			}
		} else {
			cInnData* pInnRef;

			for ( pInnRef=pFirstInn; pInnRef; pInnRef=pInnRef->pNextInn) {
				if ( pInnRef->pNextInn == pInn ) {
					if ( !pInn->pNextInn ) {
						pLastInn			= pInnRef;
						pLastInn->pNextInn	= NULL;
					} else {
						pInnRef->pNextInn = pInn->pNextInn;
					}
				}
			}
		}

		delete pInn;

		ch->println("Mobile is no longer an innkeeper.");
		return true;
	}

    medit_inn( ch, "" );
    return false;
}

/**************************************************************************/
bool medit_shop(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    char command[MIL];
    char arg1[MIL];

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg1 );

    EDIT_MOB(ch, pMob);

    if ( command[0] == '\0' )
    {
		ch->println("Syntax:  shop hours [#xopening] [#xclosing]");
		ch->println("         shop profit [#xbuying%%] [#xselling%%]");
		ch->println("         shop type [#x0-4] [item type]");
		ch->println("         shop assign");
		ch->println("         shop remove");
		return false;
    }


    if ( !str_cmp( command, "hours" ) )
	{
		if ( arg1[0] == '\0' || !is_number( arg1 )
			|| argument[0] == '\0' || !is_number( argument ) )
		{
			ch->println("Syntax:  shop hours [#xopening] [#xclosing]");
			return false;
		}
		if ( !pMob->pShop )
		{
			ch->println("MEdit: You should create a shop first (shop assign).");
			return false;
		}
		
		pMob->pShop->open_hour = atoi( arg1 );
		pMob->pShop->close_hour = atoi( argument );
		ch->println("Shop hours set.");
		return true;
	}

    if ( !str_cmp( command, "profit" ) )
	{
		if ( arg1[0] == '\0' || !is_number( arg1 )
			|| argument[0] == '\0' || !is_number( argument ) )
		{
			ch->println("Syntax:  shop profit [#xbuying%] [#xselling%]");
			return false;
		}
		if ( !pMob->pShop )
		{
			ch->println("MEdit: You should create a shop first (shop assign).");
			return false;
		}

		pMob->pShop->profit_buy     = atoi( arg1 );
		pMob->pShop->profit_sell    = atoi( argument );
		ch->println("Shop profit set.");
		return true;
    }


    if ( !str_cmp( command, "type" ) )
    {
		int value;

		if ( arg1[0] == '\0' || !is_number( arg1 )
			|| argument[0] == '\0' )
		{
			ch->println("Syntax:  shop type [#x0-4] [item type]");
			return false;
		}
		
		if ( atoi( arg1 ) >= MAX_TRADE )
		{
			ch->printlnf("MEdit:  May sell %d items max.", MAX_TRADE );
			return false;
		}

		if ( !pMob->pShop )
		{
			ch->println("MEdit: You should create a shop first (shop assign).");
			return false;
		}

		if ( ( value = flag_value( item_types, argument ) ) == NO_FLAG )
		{
			ch->println("MEdit:  That type of item is not known.");
			return false;
		}

		pMob->pShop->buy_type[atoi( arg1 )] = value;
		ch->println("Shop type set.");
		return true;
    }

    // shop assign && shop delete by Phoenix 
    if ( !str_prefix(command, "assign") )
    {
		if ( pMob->pShop ){
            ch->println("Mob already has a shop assigned to it.");
			return false;
		}
		
		// allocate shop memory and add it to the list
		pMob->pShop		= new_shop();
		if ( !shop_first ){
			shop_first	= pMob->pShop;
		}
		if ( shop_last ){
			shop_last->next	= pMob->pShop;
		}
		shop_last		= pMob->pShop;
		shop_last->next = NULL;
		
		// update the shop keeper vnum
		pMob->pShop->keeper	= pMob->vnum;
		
		ch->println("New shop assigned to mobile.");
		return true;
    }

    if ( !str_prefix(command, "remove") )
    {
		SHOP_DATA *pShop;

		if(!pMob->pShop){
			ch->println("This mobile does not have any shop data to remove.");
			return false;
		}
		
		pShop		= pMob->pShop;
		pMob->pShop	= NULL;
		
		if ( pShop == shop_first )
		{
			if ( !pShop->next ){
				shop_first = NULL;
				shop_last = NULL;
			}else{
				shop_first = pShop->next;
			}
		}else{
			SHOP_DATA *ipShop;
			
			for ( ipShop = shop_first; ipShop; ipShop = ipShop->next )
			{
				if ( ipShop->next == pShop ){
					if ( !pShop->next ){
						shop_last = ipShop;
						shop_last->next = NULL;
					}else{
						ipShop->next = pShop->next;
					}
				}
			}
		}
		
		free_shop(pShop);
		
		ch->println("Mobile is no longer a shopkeeper.");
		return true;
    }

    medit_shop( ch, "" );
    return false;
}


/**************************************************************************/
bool medit_sex(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	long s=pMob->sex;
	bool result=olc_generic_flag_toggle(ch, argument, 
		"sex", "sex", sex_types, &s);
	pMob->sex=(short)s;
	return result;
}
/**************************************************************************/
bool medit_act(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	bool result=olc_generic_flag_toggle(ch, argument,
								"act", "act", act_flags, &pMob->act);
	SET_BIT( pMob->act, ACT_IS_NPC );
    return result;	
}
/**************************************************************************/
bool medit_act2(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, "act2", "act2", act2_flags, &pMob->act2);
}
/**************************************************************************/
bool medit_affect(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"affect", "affect", affect_flags, &pMob->affected_by);
}
/**************************************************************************/
bool medit_affect2(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"affect2", "affect2", affect2_flags, &pMob->affected_by2);
}
/**************************************************************************/
bool medit_affect3(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"affect3", "affect3", affect3_flags, &pMob->affected_by3);
}

/**************************************************************************/
bool medit_ac(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
    char arg[MIL];
    int pierce, bash, slash, exotic;

    do   // So that I can use break and send the syntax in one place 
    {
		if ( argument[0] == '\0' )  break;
		
		argument = one_argument( argument, arg );
		
		if ( !is_number( arg ) )  break;
		pierce = atoi( arg );
		argument = one_argument( argument, arg );
		
		if ( arg[0] != '\0' )
		{
			if ( !is_number( arg ) )  break;
			bash = atoi( arg );
			argument = one_argument( argument, arg );
		}
		else
			bash = pMob->ac[AC_BASH];
		
		if ( arg[0] != '\0' )
		{
			if ( !is_number( arg ) )  break;
			slash = atoi( arg );
			argument = one_argument( argument, arg );
		}
		else
			slash = pMob->ac[AC_SLASH];
		
		if ( arg[0] != '\0' )
		{
			if ( !is_number( arg ) )  break;
			exotic = atoi( arg );
		}
		else
			exotic = pMob->ac[AC_EXOTIC];
		
		pMob->ac[AC_PIERCE] = pierce;
		pMob->ac[AC_BASH]   = bash;
		pMob->ac[AC_SLASH]  = slash;
		pMob->ac[AC_EXOTIC] = exotic;
		
		ch->println("Armour set.");
		return true;
    } while ( false );    // Just do it once.. 

    ch->println("Syntax:  armour [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]");
    ch->println("help MOB_AC  gives a list of reasonable ac-values.");
    return false;
}

/**************************************************************************/
bool medit_form(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"form", "form", form_flags, &pMob->form);
}

/**************************************************************************/
bool medit_part(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"part", "part", part_flags, &pMob->parts);
}

/**************************************************************************/
bool medit_imm(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"imm", "immunity", imm_flags, &pMob->imm_flags);
}

/**************************************************************************/
bool medit_res(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"res", "resistance", res_flags, &pMob->res_flags);
}

/**************************************************************************/
bool medit_vuln(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"vuln", "vulnerability", vuln_flags, &pMob->vuln_flags);
}

/**************************************************************************/
bool medit_material(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	    ch->println("Syntax:  material <string>");
		return false;
    }

    ch->printlnf("Mob %d material changed from '%s' to '%s'.", 
		pMob->vnum, pMob->material, lowercase(argument));
    replace_string( pMob->material, lowercase(argument));

    return true;
}

/**************************************************************************/
bool medit_off(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	return olc_generic_flag_toggle(ch, argument, 
		"off", "offensive", off_flags, &pMob->off_flags);
}

/**************************************************************************/
bool medit_size(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
	long s=pMob->size;
	bool result=olc_generic_flag_toggle(ch, argument, 
		"size", "size", size_types, &s);
	pMob->size=(short)s;
	return result;
}

/**************************************************************************/
bool medit_hitdice(char_data *ch, char *argument)
{
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;
    EDIT_MOB( ch, pMob );

    if ( IS_NULLSTR(argument)){
		ch->println("Syntax:  hitdice <number> d <type> + <bonus>");
		return false;
    }

    num = cp = argument;

    while ( is_digit( *cp ) ) ++cp;
    while ( *cp != '\0' && !is_digit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( is_digit( *cp ) ) ++cp;
    while ( *cp != '\0' && !is_digit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( is_digit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
		medit_hitdice(ch,"");
		return false;
    }

    pMob->hit[DICE_NUMBER] = atoi( num   );
    pMob->hit[DICE_TYPE]   = atoi( type  );
    pMob->hit[DICE_BONUS]  = atoi( bonus );

    ch->println("Hitdice set.");
    return true;
}

/**************************************************************************/
bool medit_manadice(char_data *ch, char *argument)
{    
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;
    EDIT_MOB( ch, pMob );

    if ( IS_NULLSTR(argument)){
		ch->println("Syntax:  manadice <number> d <type> + <bonus>");
		return false;
    }

    num = cp = argument;

    while ( is_digit( *cp ) ) ++cp;
    while ( *cp != '\0' && !is_digit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( is_digit( *cp ) ) ++cp;
    while ( *cp != '\0' && !is_digit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( is_digit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
		medit_manadice(ch,"");
		return false;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
		medit_manadice(ch,"");
		return false;
    }

    pMob->mana[DICE_NUMBER] = atoi( num   );
    pMob->mana[DICE_TYPE]   = atoi( type  );
    pMob->mana[DICE_BONUS]  = atoi( bonus );

    ch->println("Manadice set.");
    return true;
}

/**************************************************************************/
bool medit_damdice(char_data *ch, char *argument)
{
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;
    EDIT_MOB( ch, pMob );

    if ( IS_NULLSTR(argument)){
		ch->println("Syntax:  damdice <number> d <type> + <bonus>");
		return false;
    }
	
    num = cp = argument;

    while ( is_digit( *cp ) ) ++cp;
    while ( *cp != '\0' && !is_digit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( is_digit( *cp ) ) ++cp;
    while ( *cp != '\0' && !is_digit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( is_digit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
		medit_damdice(ch,"");
		return false;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
		medit_damdice(ch,"");
		return false;
    }

    pMob->damage[DICE_NUMBER] = atoi( num   );
    pMob->damage[DICE_TYPE]   = atoi( type  );
    pMob->damage[DICE_BONUS]  = atoi( bonus );

    ch->println("Damdice set.");
    return true;
}


/**************************************************************************/
bool medit_race(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
    int race;

    if ( argument[0] != '\0'
    && ( race = race_lookup( argument ) ) >= 0 )
    {

		pMob->race = race;
		pMob->act	  |= race_table[race]->act;
		pMob->affected_by = race_table[race]->aff;
		pMob->off_flags   = race_table[race]->off;
		pMob->imm_flags   = race_table[race]->imm;
		pMob->res_flags   = race_table[race]->res;
		pMob->vuln_flags  = race_table[race]->vuln;
		pMob->form        = race_table[race]->form;
		pMob->parts       = race_table[race]->parts;

        ch->printlnf("Race set to '%s'.", race_table[race]->name );
		ch->println("Note: The following fields have been set to their racial defaults:");
		ch->println("      Affected by Flags, Offensive Flags, Immunitity Flags,");
		ch->println("      Resistance Flags, Vulnerability Flags, Form and Parts");
		return true;
    }

    if ( argument[0] == '?' )
    {
		ch->print("Available races are:");

		for ( race = 1; race_table[race]; race++ )
		{
			if ( ( race % 3 ) == 0 )
				ch->println("");
			ch->printf(" %-15s", race_table[race]->name );
		}
		ch->println("");
		return false;
    }

    ch->printf( "Syntax:  race [race]\r\n"
        "Type 'race ?' for a list of races.\r\n" );
    return false;
}

/**************************************************************************/
bool medit_position(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
	EDIT_MOB( ch, pMob );
    char arg[MIL];
    int value;

    argument = one_argument( argument, arg );

    switch ( arg[0] )
    {
    default:
		break;
		
    case 'S':
    case 's':
		if ( str_prefix( arg, "start" ) )
			break;
		
		if ( ( value = flag_value( position_types, argument ) ) == NO_FLAG )
			break;
		
		
		pMob->start_pos = value;
		ch->println("Start position set.");
		return true;
		
    case 'D':
    case 'd':
		if ( str_prefix( arg, "default" ) )
			break;
		
		if ( ( value = flag_value( position_types, argument ) ) == NO_FLAG )
			break;
		
		
		pMob->default_pos = value;
		ch->println("Default position set.");
		return true;
    }

    ch->println(" Syntax:  position [start/default] [position]");
    ch->println(" Type '? position' for a list of positions.");
    ch->println(" examples: position start sit");
    ch->println("           position default rest");
    ch->println("");
    ch->println(" Selectable positions can be one of the following:");
	show_help(ch, "position");

    return false;
}

/**************************************************************************/
bool medit_wealth(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    EDIT_MOB(ch, pMob);
	int value;


    if ( argument[0] == '\0' || !is_number( argument ) )
    {
		ch->println("Syntax:  wealth [number]");
		return false;
    }

	value= atoi( argument );

	if(value<0 ){
		ch->println("Wealth must be zero or greater!");
		return false;
	}else if(value > pMob->level*MAX_MOB_WEALTH_MULTIPLIER){ 
		ch->printlnf("Wealth must be less than %d times the mobs level (%d).",
			MAX_MOB_WEALTH_MULTIPLIER, pMob->level*MAX_MOB_WEALTH_MULTIPLIER);
		return false;
	}
    
	ch->printlnf("Wealth changed from %d to %d", (int)pMob->wealth, value);
	pMob->wealth=value;

    return true;
}

/**************************************************************************/
bool medit_hitroll(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
		ch->println("Syntax:  hitroll [number]");
		return false;
    }

    pMob->hitroll = atoi( argument );

    ch->println("Hitroll set.");
    return true;
}

/**************************************************************************/
void show_liqlist(char_data *ch)
{
    int liq;
    BUFFER *buffer;
    char buf[MSL];
    
    buffer = new_buf();
    
    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if ( (liq % 21) == 0 )
        add_buf(buffer,"Name                 Color          Proof Full Thirst Food Ssize\r\n");

    sprintf(buf, "%-20s %-14s %5d %4d %6d %4d %5d\r\n",
		liq_table[liq].liq_name,liq_table[liq].liq_color,
		liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
		liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
		liq_table[liq].liq_affect[4] );
	add_buf(buffer,buf);
    }

    ch->sendpage(buf_string(buffer));
    free_buf(buffer);

	return;
}

/**************************************************************************/
void show_damlist(char_data *ch)
{
    int att;
    BUFFER *buffer;
    char buf[MSL];
    
    buffer = new_buf();
    
    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if ( (att % 21) == 0 )
        add_buf(buffer,"Name                 Noun\r\n");

    sprintf(buf, "%-20s %-20s\r\n",
		attack_table[att].name,attack_table[att].noun );
	add_buf(buffer,buf);
    }

    ch->sendpage(buf_string(buffer));
    free_buf(buffer);

	return;
}

/**************************************************************************/
bool medit_group(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMTemp;
    char arg[MSL];
    char buf[MSL];
    int temp;
    BUFFER *buffer;
    bool found = false;
    
    EDIT_MOB(ch, pMob);
    
    if ( argument[0] == '\0' )
    {
        ch->println("Syntax: group <number>");
        ch->println("        group show <number>");
		ch->println(" (Mobs with a particular helpgroup number will assist other mobs with that group number (excluding 0))");
		ch->wrapln("An example of how to use group and helpgroup... set the group number "
			"of a citizen to 5, set the helpgroup number of a guard to 5.  "
			"the guard will autoassist the citizen (unless charmed), "
			"but the citizen wont autoassist the guard.");
    	return false;
    }
    
    if (is_number(argument))
    {
		pMob->group = atoi(argument);
        ch->printf( "Group set to %d.\r\n", pMob->group);
		return true;
    }
    
    argument = one_argument( argument, arg );
    
    if ( !strcmp( arg, "show" ) && is_number( argument ) )
    {
		if (atoi(argument) == 0)
		{
			ch->println("Are you crazy?");
			return false;
		}
		
		buffer = new_buf ();
		
		for (temp = 0; temp < game_settings->olc_max_vnum; temp++)
		{
			pMTemp = get_mob_index(temp);
			if ( pMTemp && ( pMTemp->group == atoi(argument) ) )
			{
				found = true;
                sprintf( buf, "[%5d] %s\r\n", pMTemp->vnum, pMTemp->player_name );
				add_buf( buffer, buf );
			}
		}
		
		if (found){
			ch->sendpage(buf_string(buffer));
		}else{
			ch->println("No mobs in that group.");
		}
		
		free_buf( buffer );
        return false;
    }
    
    return false;
}
/**************************************************************************/
bool medit_helpgroup(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMTemp;
    char arg[MSL];
    char buf[MSL];
    int temp;
    BUFFER *buffer;
    bool found = false;
    
    EDIT_MOB(ch, pMob);
    
    if ( argument[0] == '\0' )
    {
        ch->println("Syntax: helpgroup [number]");
        ch->println("        helpgroup show [number]");
        ch->println(" (Mobs with a particular helpgroup number will assist other mobs with that group number (excluding 0))");
		ch->wrapln("An example of how to use group and helpgroup... set the group number "
			"of a citizen to 5, set the helpgroup number of a guard to 5.  "
			"the guard will autoassist the citizen (unless charmed), "
			"but the citizen wont autoassist the guard.");
    	return false;
    }
    
    if (is_number(argument))
    {
		pMob->helpgroup = atoi(argument);
        ch->printf( "Helpgroup set to %d.\r\n", pMob->helpgroup);
		return true;
    }
    
    argument = one_argument( argument, arg );
    
    if ( !strcmp( arg, "show" ) && is_number( argument ) )
    {
		if (atoi(argument) == 0)
		{
			ch->println("Are you crazy?");
			return false;
		}
		
		buffer = new_buf ();
		
		for (temp = 0; temp < game_settings->olc_max_vnum; temp++)
		{
			pMTemp = get_mob_index(temp);
			if ( pMTemp && ( pMTemp->helpgroup== atoi(argument) ) )
			{
				found = true;
                sprintf( buf, "[%5d] %s\r\n", pMTemp->vnum, pMTemp->player_name );
				add_buf( buffer, buf );
			}
		}
		
		if (found){
			ch->sendpage(buf_string(buffer));
		}else{
			ch->println("No mobs in that helpgroup.");
		}
		
		free_buf( buffer );
        return false;
    }
    
    return false;
}
/**************************************************************************/
bool medit_addmprog(char_data *ch, char *argument)
{
	int value;
	MOB_INDEX_DATA *pMob;
	MUDPROG_TRIGGER_LIST *list;
	MUDPROG_CODE *prog;
	char trigger[MSL];
	char phrase[MSL];
	char num[MSL];
	vn_int mpnum;
	
	EDIT_MOB(ch, pMob);
	argument=one_argument(argument, num);
	argument=one_argument(argument, trigger);
	strcpy(phrase, trim_string(argument));
		
	if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' ){
        ch->println("`xSyntax:   addmprog <vnum> <trigger> <phrase>");
		ch->println("Where <trigger> is one of:");
		ch->wraplnf("[`=R%s`x]", flag_string( mprog_flags, -1));
        return false;
	}
	
	mpnum=atoi(num);

	if ( ( prog =get_mprog_index (mpnum ) ) == NULL)
	{
        ch->printlnf("No such MUDProgram %d.", mpnum);
        return false;
	}

    if(pMob->area!= get_vnum_area( mpnum )){
		if(HAS_SECURITY(ch,9)){
			ch->println("`RWARNING: That mudprog number belongs to a different area!`x");
		}else{
			ch->println("`RThat mudprog number belongs to a different area!`x");
			ch->println("`ROnly security 9 can link in mudprogs from different areas.`x");
			return false;
		}
	};
	
	if ( (value = flag_value (mprog_flags, trigger) ) == NO_FLAG)
	{
		ch->printlnf("Valid mudprog trigger flags are: %s", 
			flag_string( mprog_flags, -1));
        return false;
	}
	
	list                  = new_mprog();
	list->trig_type       = value;
	list->trig_phrase     = str_dup(phrase);
	list->prog            = prog;
	list->next            = pMob->mob_triggers;
	pMob->mob_triggers          = list;
	
	// recalc the trigger bits
	SET_BIT(pMob->mprog_flags, value);
    ch->printlnf("Mprog %d '%s' added", mpnum, prog->title);
	ch->printlnf("Mob '%s' triggering is enabled.", mprog_type_to_name(value));
    ch->printf("NOTE: This change affects all mobs based on the "
		"mob template %d instantly.\r\n", pMob->vnum);

	return true;
}
/**************************************************************************/
// loop thru a whole list of progs, calculating all the trigger bits
// Kal - Nov 99
long calc_mprog_flags(MUDPROG_TRIGGER_LIST *mprogs)
{
    MUDPROG_TRIGGER_LIST *list;
	long result=0;

    for (list= mprogs; list; list=list->next){
		SET_BIT(result,list->trig_type);
	}
	return result;
}
/**************************************************************************/
bool medit_delmprog(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    MUDPROG_TRIGGER_LIST *list;
    MUDPROG_TRIGGER_LIST *list_next;
    char mprog[MIL];
    int value;
    int cnt = 0;
	long trigtype=0;
	
    EDIT_MOB(ch, pMob);
	
    one_argument( argument, mprog );
    if (!is_number( mprog ) || mprog[0] == '\0' )
    {
		ch->println("Syntax:  delmprog <#mprog>");
		return false;
    }
	
    value = atoi ( mprog );
	
    if ( value < 0 )
    {
        ch->println("Only non-negative mprog-numbers allowed.");
        return false;
    }
	
    if ( !(list= pMob->mob_triggers) )
    {
        ch->printlnf("MEdit: There are no mudprogs on mob template %d.", pMob->vnum);
        return false;
    }
	
    if ( value == 0 )
    {
        list = pMob->mob_triggers;
		trigtype=list->trig_type;
        pMob->mob_triggers = list->next;
		list->next=NULL;
        free_mprogs( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
			list = list_next;
		
        if ( list_next )
        {
			list->next = list_next->next;
			trigtype=list_next->trig_type;
			list_next->next=NULL;
			free_mprogs(list_next);
        }
        else
        {
			ch->println("No such mprog.");
			return false;
        }
    }

	// recalc the trigger bits
	// recalc the trigger bits
	pMob->mprog_flags=0;
	pMob->mprog2_flags=0;
	for(list=pMob->mob_triggers; list; list=list->next){
		SET_BIT(pMob->mprog_flags, list->trig_type);
		SET_BIT(pMob->mprog2_flags, list->trig2_type);
	}

	ch->printlnf("Mprog %d removed", value);
    ch->printf("NOTE: This change affects all mobs based on the "
		"mob template %d instantly.\r\n", pMob->vnum);
    return true;
}


/**************************************************************************/
bool medit_posmprog(char_data *ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    MUDPROG_TRIGGER_LIST *list;
    char mprog[MIL];
    int value;
    int cnt = 0;
	
    EDIT_MOB(ch, pMob);
	
    one_argument( argument, mprog );
    if (!is_number( mprog ) || IS_NULLSTR(argument))
    {
		ch->println("Syntax:  posmprog <#mprog> <position flag to toggle>");
		return false;
    }
	
    value = atoi ( mprog );
	
    if ( value < 0 )
    {
        ch->println("Only non-negative mprog-numbers allowed.");
        return false;
    }
	
    if ( !(list= pMob->mob_triggers) )
    {
        ch->printlnf("MEdit: There are no mudprogs on mob template %d.", pMob->vnum);
        return false;
    }
	
	cnt=0;
	for(list = pMob->mob_triggers; list; list=list->next){
		cnt++;
		if(cnt>value){
			break;
		}
	}

	if(!list){
		ch->println("No such mprog.");
		return false;
    }
		
	if ( ( value = flag_value( position_flags, argument ) ) != NO_FLAG )
	{
		list->pos_flags ^= value;	
		ch->println("Mprog position flag toggled.");
		return true;
	}
	
    ch->println("Syntax: posmprog <number> <flag>");
    return false;
}


/**************************************************************************/
// duplicate a single shop entry, but set the keeper vnum to zero
// - Kal June 98, updated June 02
static SHOP_DATA * dup_shop(SHOP_DATA * shop)
{	
	SHOP_DATA * pShop;
	
	if (shop==NULL)
		return(NULL);

	pShop	= new_shop();
	pShop->next = NULL;
	pShop->keeper = 0;

	pShop->profit_buy = shop->profit_buy;
	pShop->profit_sell = shop->profit_sell;
	pShop->open_hour = shop->open_hour;
	pShop->close_hour = shop->close_hour;
	for (int i=0; i<MAX_TRADE; i++){
		pShop->buy_type[i]= shop->buy_type[i];
	}

	return (pShop);
}
/**************************************************************************/
// duplicate the mob data from another 
// written by Kalahn - June 98
bool medit_copy(char_data *ch, char *argument)
{
	MOB_INDEX_DATA *pMob;
	MOB_INDEX_DATA *pSrc; // source mob
	char arg1[MIL];
	int value;

    argument = one_argument( argument, arg1 );

    if ( !is_number( arg1 ) )
    {
        ch->println("Syntax: mcopy <source mob vnum>");
        ch->println("  - copies the source mob over the mob you are currently editing!");
        ch->println("    (warning copies over everything!)");
        return false;
	}

    value = atoi( arg1 );
    if ( !( pSrc = get_mob_index( value ) ) )
    {
        ch->println("MEdit_copy:  The source vnum does not exist.");
        return false;
    }

    if ( !IS_BUILDER( ch, pSrc->area, BUILDRESTRICT_MOBS) && !IS_IMMORTAL(ch) )
    {
        ch->println( "Insufficient security to copy from the area that mob");
        ch->println("is stored in and your arent an immortal.");
        return false;
    }

    
    EDIT_MOB(ch, pMob);

	// copy the mob details
	pMob->player_name	= str_dup(pSrc->player_name);
	pMob->short_descr	= str_dup(pSrc->short_descr);
	pMob->long_descr	= str_dup(pSrc->long_descr);
	pMob->description	= str_dup(pSrc->description);

	pMob->group			= pSrc->group;
	pMob->helpgroup		= pSrc->helpgroup;
	pMob->act			= pSrc->act;
	pMob->affected_by	= pSrc->affected_by;

	pMob->alliance		= pSrc->alliance;
	pMob->tendency		= pSrc->tendency;
	pMob->level			= pSrc->level;
	
	pMob->hitroll		= pSrc->hitroll;
	pMob->hit[0]		= pSrc->hit[0];
	pMob->hit[1]		= pSrc->hit[1];
	pMob->hit[2]		= pSrc->hit[2];
	pMob->mana[0]		= pSrc->mana[0];
	pMob->mana[1]		= pSrc->mana[1];
	pMob->mana[2]		= pSrc->mana[2];
	pMob->damage[0]		= pSrc->damage[0];
	pMob->damage[1]		= pSrc->damage[1];
	pMob->damage[2]		= pSrc->damage[2];
	pMob->ac[0]			= pSrc->ac[0];
	pMob->ac[1]			= pSrc->ac[1];
	pMob->ac[2]			= pSrc->ac[2];
	pMob->ac[3]			= pSrc->ac[3];
	pMob->dam_type		= pSrc->dam_type;

	pMob->off_flags		= pSrc->off_flags;
	pMob->imm_flags		= pSrc->imm_flags;
	pMob->res_flags		= pSrc->res_flags;
	pMob->vuln_flags	= pSrc->vuln_flags;

	pMob->start_pos		= pSrc->start_pos;
	pMob->default_pos	= pSrc->default_pos;

	pMob->sex			= pSrc->sex;
	pMob->race			= pSrc->race;

	pMob->wealth		= pSrc->wealth;
	pMob->form			= pSrc->form;
	pMob->size			= pSrc->size;
	pMob->parts			= pSrc->parts;	
	pMob->xp_mod		= pSrc->xp_mod;

	pMob->material		= str_dup(pSrc->material);

	pMob->spec_fun		= pSrc->spec_fun;


	// NOTE: we aren't deallocating the existing mudprog lists and 
	//	shops... we should, but I can't be bothered writing the code 
	// to do so right now.

	// copy mudprogs list
	pMob->mob_triggers		= dup_mudprog_list(pSrc->mob_triggers);
	pMob->mprog_flags		= pSrc->mprog_flags;
	pMob->mprog2_flags		= pSrc->mprog2_flags;

	// copy the shopkeeper details
	pMob->pShop			= dup_shop(pSrc->pShop);

	// assign it to the shop list and the keeper value
	if ( !shop_first ){
		shop_first	= pMob->pShop;
	}
	if ( shop_last ){
		shop_last->next	= pMob->pShop;
	}

	if(pMob->pShop){
		shop_last		= pMob->pShop;
		shop_last->next = NULL;
		
		pMob->pShop->keeper = pMob->vnum;
	}

	ch->wraplnf("`=rCopied mob '%s'[%d] to vnum %d", 
				pSrc->short_descr, pSrc->vnum, pMob->vnum);
    return true;
}
/**************************************************************************/
// do_mxpmodlist: shows all the mobs and vnums in all areas between an 
//            xpmod range
void do_mxpmodlist( char_data *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL], arg2[MIL];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    bool found;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The mxpmodlist command is an olc related command, you don't have olc permissions.");
		return;
	}

    argument=one_argument( argument, arg );
    one_argument( argument, arg2 );
	if ( IS_NULLSTR(arg2) ){
		ch->println("Syntax:  mxpmodlist <lower> <upper>");
		return;
    }

	int lower=atoi(arg);
	int upper=atoi(arg2);
		
	// swap the mod values if necessary
	if (lower>upper){
		vnum=upper;
		upper=lower;
		lower=vnum;
	}

	int count=0;
    found	= false;
    BUFFER	*buf1=new_buf();

    for ( vnum = 0; vnum<game_settings->olc_max_vnum; vnum++ )
    {
		if(count>200) {
			ch->println("More than 200, displaying 200.");			
			break;
		}
		if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
		{
			if ( pMobIndex->xp_mod>lower && pMobIndex->xp_mod<upper)
			{
				count++;
				found = true;
				sprintf( buf, "<%5d>[xpmod=%3d][%2d] %s\r\n",
					pMobIndex->vnum, pMobIndex->xp_mod,
					pMobIndex->level, pMobIndex->short_descr );
				add_buf( buf1, buf );				
			}
		}
    }

    if ( !found )
		ch->println("No mobiles found in that range.");

	ch->printlnf("`xMobs in xpmod range %d->%d, %d found.",	lower, upper, count);

    ch->sendpage(buf_string(buf1));
    free_buf(buf1);
    return;
}
/**************************************************************************/
// do_mobloglist: lists all mobs with the moblog flag on
void do_mobloglist( char_data *ch, char *argument)
{
    char buf[MSL];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    bool found;
	
	if (!HAS_SECURITY(ch,1)){
		if(str_cmp(argument, "auto")){
			ch->println("The mobloglist command is an olc related command, you don't have olc permissions.");
		}
		return;
	}

	int count=0;
    found	= false;
    BUFFER	*buf1=new_buf();

    for ( vnum = 0; vnum<game_settings->olc_max_vnum; vnum++ )
    {
		if(count>200) {
			ch->println("More than 200, displaying 200.");			
			break;
		}
		if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
		{
			if(IS_SET(pMobIndex->act,ACT_MOBLOG)){
				count++;
				found = true;
				sprintf( buf, "[%d] <%5d,%3d> %s\r\n",
					count,
					pMobIndex->vnum, 										
					pMobIndex->level,
					pMobIndex->short_descr);
				add_buf( buf1, buf );
			}
		}
    }

    if ( !found){
		if(str_cmp(argument, "auto")){
			ch->println("No mobiles found with the moblog on.");
		}
	    free_buf(buf1);
		return;
	}

	ch->titlebar("MOBLOGLIST OUTPUT - MOBILES WITH MOBLOG ENABLED IN GAME");
	ch->println("`xMobiles with the moblog turned on.");

	if(!str_cmp(argument, "auto")){
		add_buf( buf1, 
			"\r\n"
			"The moblog system is used to debug mob interaction when mudprogs are in use.\r\n"
			"Leaving the 'act moblog' flag on mobs which aren't being debugged wastes \r\n"
			"resources and can create a very large logfile (use the checkmoblog \r\n"
			"command to view this log file).  Turn off the 'act moblog' flag on any mobs \r\n"
			"above which are not being actively debugged.\r\n"
	   "`#`=t-===========================================================================-`\r\n");
	}

    ch->sendpage(buf_string(buf1));
    free_buf(buf1);
	if(!str_cmp(argument, "auto")){
		ch->hit_return_to_continue();
	}
    return;
}

/**************************************************************************/
bool medit_gamble( char_data *ch, char *argument )
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if ( argument[0] == '\0' )
	{
		ch->println("Syntax:  gamble [game]");
		ch->println(" To clear a mob gambling function: gamble none\r\n");
		ch->print("`=rSelect a game from below:\r\n`=R");
		show_help (ch, "gamble_games");
		ch->print("`x");
		return false;
	}

	if ( !str_cmp( argument, "none" ))
	{
		pMob->gamble_fun = NULL;
		ch->println("Game removed.");
		return true;
	}

	if ( gamble_lookup( argument ))
	{
		pMob->gamble_fun = gamble_lookup( argument );
		ch->printlnf("Gambling game set to %s.", gamble_name( pMob->gamble_fun ));
		return true;
	}

	ch->println("MEdit: No matching gambling function.");
    ch->print("`=rSelect one from below:\r\n`=R");
	show_help (ch, "gamble_games");
	ch->print("`x");

    return false;
}

/**************************************************************************/
bool medit_delete( char_data *ch, char *)
{
	ch->println("If you want to delete a mob, use the 'mdelete' command");
	return false;
}
/**************************************************************************/
// Kal, Feb 2001
bool medit_mdelete(char_data *ch, char *argument)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);

    if (IS_NULLSTR(argument))
    {
		ch->titlebar("MDELETE SYNTAX");
        ch->println("Syntax: mdelete confirm - delete the current mob");
        ch->println("Syntax: mdelete <number> confirm - delete mob vnum <number>");
		ch->println("Any mob that you delete must meet the following conditions:");
		ch->println("* Must not be used by any reset in any room.");
		ch->println("* Must not be currently loaded in the game.");
		ch->println("* Gameedit can't be making use of the mobile.");
		ch->println("* You must have sufficient security to edit that mob.");
		ch->println("* No one else can currently be editing the mobile.");
        ch->wrapln("NOTE: It is strongly recommended that no mudprogs attempt to load the mob "
			"you are considering deleting... the easiest method to do this is 'textsearch mudprog <mobvnum>'.");
		return false;
    }

	// support specifying the mob by vnum
	char arg1[MIL];
	MOB_INDEX_DATA *pDeleteMob;
	argument=one_argument(argument, arg1);
	if(is_number(arg1)){
		pDeleteMob=get_mob_index(atoi(arg1));
		if(!pDeleteMob){
			ch->printlnf("medit_mdelete(): There is no mob number %s to delete.", arg1);
			return false;
		}
		argument=one_argument(argument, arg1); // put the word 'confirm' into arg1
	}else{
		pDeleteMob=pMob; // deleting the mob we are currently editing
	}

	// security check
	if ( !IS_BUILDER( ch, pDeleteMob->area, BUILDRESTRICT_MOBS ) )
    {
        ch->printlnf("MEdit: Insufficient security to delete mobile %d.", pDeleteMob->vnum);
        return false;
    }

	// confirm they are using 'confirm'
	if(str_cmp(arg1, "confirm")){
		ch->println("You must confirm your intention to delete a mob.");
		medit_mdelete(ch,"");
		return false;
	}

	if(!IS_NULLSTR(ltrim_string(argument))){
		ch->println("Incorrect syntax - too many arguments, or arguments in wrong order.");
		medit_mdelete(ch, "");
		return false;
	}

	int v=pDeleteMob->vnum;
	// We have the mob they are wanting to delete and they have 
	// confirmed they want to delete it, check if it isn't in use
	{
		// mobs in game
		int in_use=0;
		for(char_data *inuse_mob=char_list; inuse_mob; inuse_mob=inuse_mob->next){
			if(inuse_mob->pIndexData==pDeleteMob){
				ch->printlnf("`=rOne or more mobs based on mob template %d are currently in the game... use mwhere <vnum> to find them.`x", v);
				in_use++;
				break;
			}
		}

		// resets using this mob
		for( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
		{
			for( ROOM_INDEX_DATA *pRoom= room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
			{
				for ( RESET_DATA *pReset = pRoom->reset_first; pReset; pReset = pReset->next )
				{
					if(pReset->command=='M' && pReset->arg1==v){
						ch->printlnf("mob %d reset in room %d (%s).", 
							v, pRoom->vnum, pRoom->name);
						in_use++;
					}
				}
			}
		}

		for(int i=0; !IS_NULLSTR(gameset_value[i].name); i++){
			if(gameset_value[i].category!=GSVC_MOB){
				continue;
			}
			// get our numeric value
			int value=GSINT(gameset_value[i].offset);

			if(value==v){
				ch->printlnf("Game setting value '%s (%s)' makes use of mobile %d.", 
					gameset_value[i].name, gameset_value[i].description, value);
				in_use++;
			}
		}

		// someone else currently editing the mob
		for(connection_data *c=connection_list; c; c=c->next){
			if(c!=ch->desc && c->pEdit==(void *)pDeleteMob){
				ch->println("Someone else is currently editing it, so it can't currently be deleted.");
				in_use++;
			}
		}

		if(in_use){
			ch->println("You can't delete this mob, it is currently in use.");
			medit_mdelete(ch, "");
			return false;
		}
	}

	if(pMob==pDeleteMob){
		edit_done(ch);
	}
	ch->printlnf("Deleting mob %d.", v);
	
	// remove mob from hash table
	{
		int i=v% MAX_KEY_HASH;
		// check if we are the first entry in the hash table
		if(pDeleteMob== mob_index_hash[i]){
			mob_index_hash[i]=mob_index_hash[i]->next;
		}else{
			MOB_INDEX_DATA *prev=mob_index_hash[i];
			if(!prev){
				bugf("medit_mdelete(): Trying to free mob vnum %d, but not found in mob_index_hash[%d]!", 
					v, i);
			}else{
				for( ; prev->next; prev=prev->next )
				{
					if(prev->next==pDeleteMob){
						prev->next=pDeleteMob->next; // remove the mob from the link
						break;
					}
				}
			}
		}
	}
	free_mob_index(pDeleteMob);
	top_mob_index--;

	ch->printlnf("Mob %d Deleted.",v);
	
	return true;
}
/**************************************************************************/
bool medit_help( char_data *ch, char *argument )
{
	if(IS_NULLSTR(argument)){
		ch->println("Type `=Chelpgroup`x to set a mobs helpgroup");
		ch->println("or `=Chelp <keyword>`x to browse the normal helps.");
		return false;
	}
	do_help(ch, argument);
	return false;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

