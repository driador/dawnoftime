/**************************************************************************/
// debug.cpp - Kal's budget debug system :)
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "interp.h"
#include "olc.h"

/**************************************************************************/
void do_debugroom( char_data *ch, char *argument )
{
    char			buf[MSL];
    char			arg[MIL];
    ROOM_INDEX_DATA *location;
    OBJ_DATA		*obj;
    char_data		*rch;
    int				door;
	AFFECT_DATA		*paf, *paf_last = NULL;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
		ch->println("No such location.");
		ch->println("The syntax is just like 'stat room'.");
		return;
    }

	if (ch->in_room != location
		&& is_room_private_to_char( location, ch ) 
		&& !IS_TRUSTED(ch,IMPLEMENTOR))
	{
		ch->println("That room is private right now.");
		return;
    }

	ch->println("`RDEBUG_ROOM SET TO THE FOLLOWING ROOM:`x");
	DEBUG_ROOM=location;

	ch->printlnf("Name: '%s'\nArea: '%s'  Filename '%s'",
				location->name,
				location->area->name,
				location->area->file_name );

	ch->printlnf("Vnum: %d  Sector: %s  Light: %d  Healing: %d  Mana: %d",
				location->vnum,
				flag_string( sector_types, location->sector_type ),
				location->light,
				location->heal_rate,
				location->mana_rate );

	ch->printf("Room flags: %s.\nDescription:\n%s",
				room_flags_bit_name(location->room_flags),
				location->description );

    if ( location->extra_descr != NULL )
    {
		EXTRA_DESCR_DATA *ed;

		ch->print("Extra description keywords: '");
		for ( ed = location->extra_descr; ed; ed = ed->next )
		{
			ch->printf("%s", ed->keyword );
			if ( ed->next != NULL )
				ch->print(" ");
		}
		ch->println("'.");
	}

	ch->print("Characters:");
	for ( rch = location->people; rch; rch = rch->next_in_room )
	{
		if (can_see(ch,rch))
		{
			ch->print(" ");
			one_argument( rch->name, buf );
			ch->printf("%s", buf );
		}
	}

	ch->print(".\nObjects:   ");
	for ( obj = location->contents; obj; obj = obj->next_content )
	{
		ch->print(" ");
		one_argument( obj->name, buf );
		ch->printf("%s", buf );
	}
	ch->println(".");

	for ( door = 0; door <= 9; door++ )
	{
		EXIT_DATA *pexit;

		if ( ( pexit = location->exit[door] ) != NULL )
		{
			ch->printf("Door: %d.  To: %d.  Key: %d.  Exit flags: %s.  Reset flags: %s\nKeyword: '%s'.  Description: %s",
				door,
				(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
				pexit->key,
				flag_string( exit_flags, pexit->exit_info ),
				flag_string( exit_flags, pexit->rs_flags),
				pexit->keyword,
				pexit->description[0] != '\0'
				? pexit->description : "(none).\r\n" );

		}
	}

	if ( location->affected_by )
	{
		ch->println("`rThe room has the following affects on it.`x");
		for ( paf = location->affected; paf; paf = paf->next )
		{
			if (paf_last && paf->type == paf_last->type){
				continue;
			}else{
				ch->printf("Spell: %-15s", skill_table[paf->type].name );
				if ( paf->duration == -1 )
					ch->print("until the world reforms");
				else
					ch->printf("for %d hours", paf->duration );
			}
			ch->println("");
			paf_last = paf;
		}
	}

	return;
}


/**************************************************************************/
int get_sublevels_for_level(int level);
/**************************************************************************/
void do_debugmob( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
    char buf[MSL];
    AFFECT_DATA *paf;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->println("Stat whom?");
        return;
    }

    if (( victim = get_char_world( ch, argument ) ) == NULL )
    {
        ch->println("They aren't here.");
        return;
    }

	ch->println("`RDEBUG_MOB SET TO THE FOLLOWING MOBILE:`x");
	DEBUG_MOB=victim;

	ch->println("_______________________________________________________________________________\n|                                                                             |");
	
	if(IS_NPC(victim)){
		ch->printlnf("| `xName:`B %-68s`x|", victim->name);
	}else{
		sprintf(buf, "`xName:`B %s`x   Created: %s",
			victim->name, ctime( (time_t *) & (victim->player_id)));
		buf[str_len(buf)-1]='\0';
		ch->printlnf("| %-82s|", buf);
	}

	if (!IS_NPC(victim) && IS_TRUSTED(ch, ADMIN))
	{
		// for email banning verification
		bool print=false;
		if(!IS_NULLSTR(victim->pcdata->email)){
			ch->printlnf("| Email: `b%-68s`x |", victim->pcdata->email);
			print=true;
		}
		if(!IS_NULLSTR(victim->pcdata->created_from)){
			ch->printf("| Created_from: `b%-38s`x  ", victim->pcdata->created_from);
			print=true;
		}
		if(!IS_NULLSTR(victim->pcdata->unlock_id)){
			if(str_len(victim->pcdata->unlock_id)==6){
				ch->printlnf("Unlock_id: `b%-10s`x |", victim->pcdata->unlock_id);
				print=true;
			}
		}
		if(print){
			ch->println("");
		}
	}

	ch->println("|_____________________________________________________________________________|");
	ch->println("|                                                                             |");


    ch->printlnf("| Vnum: %-5d  Format: %-3s  Room: `m%-5d`x  LastIC: `M%-5d`x  Recall:  %-5d        |",
        IS_NPC(victim) ? victim->pIndexData->vnum : 0,
        IS_NPC(victim) ? "npc" : "pc",
        victim->in_room == NULL ? 0 : victim->in_room->vnum,
		victim->last_ic_room == NULL ? 0 : victim->last_ic_room ->vnum,
		victim->recall_room );


	if (IS_NPC(victim))
    {
		ch->printlnf("| Count: %-3d   Killed: %-3d   XP Mod: %-3d                                      |",
        victim->pIndexData->count,victim->pIndexData->killed, victim->pIndexData->xp_mod);
    }


	ch->printlnf("| Race: `B%-10s  `xGroup: `B%-2d  `xSex: `B%-7s`x                                   |",
		race_table[victim->race]->name,
		IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name );
         
	ch->printf("| Lv: `G%-3d  `xClass: `G%-12s  `xTendency: `g%+d  `xAlliance: `g%+d`x                    |\n| `Y-$$$ `xBank: `y%-6ld  `xGold: `y%-5ld  `xSilver: `s%-5ld`x  Exp: `M%-6d`x",
		victim->level,       
        IS_NPC(victim) ? "mobile" : class_table[victim->clss].name,
		victim->tendency, victim->alliance, victim->bank,
		victim->gold, victim->silver, victim->exp );

	if ( !IS_NPC( victim ))
		ch->printlnf("  HeroXP: %-5d   |", victim->pcdata->heroxp );
	else
		ch->println("                  |");

	if(!IS_NPC(victim) && victim->level>=50){
		ch->printlnf("`g|`G|`x  Sublevel:  %2d/%2d  subprac %d  subtrain %d", 
			victim->pcdata->sublevel, get_sublevels_for_level(victim->level),
			victim->pcdata->sublevel_pracs, victim->pcdata->sublevel_trains);
	}

	ch->printlnf("| Armor: pierce: `b%-6d  `xbash: `b%-6d  `xslash: `b%-6d  `xmagic: `b%-6d`x           |",
		GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
		GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));

    ch->printlnf("| Hit: `c%-3d`x  Dam: `c%-3d`x  Saves: `c%-3d`x  Size: %-7s  Pos: %-8s  Wimpy: %-3d    |",
        GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
        size_table[victim->size].name, position_table[victim->position].name,
        victim->wimpy );

    if (IS_NPC(victim))
    {
		ch->printlnf("| Damage: `C%2dd%-2d  `xMessage: `c%-15s`x                                     |",
			victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
			attack_table[victim->dam_type].noun);
    }else{
		ch->printlnf("| victim->pcdata->objrestrict = %d |",	victim->pcdata->objrestrict);
	}

	ch->println("|_____________________________________________________________________________|\n|                                                                             |");

	ch->printlnf("| Fighting: `C%-54s  `c%s`x |",
		victim->fighting ? victim->fighting->name : "(none)" ,
		victim->no_xp ? "NO_XP_SET" : "         ");

//	ch->printlnf("| Memory: {r%-67s{x |",
//		victim->mobmemory ? victim->mobmemory->name : "(none)" );

	if ( !IS_NPC(victim) )
    {
		ch->printf(
           "| Thirst: `c%-2d  `xHunger: `c%-2d  `xFull: `c%-2d  `xDrunk: `c%-2d  `xTired: `c%-2d`x                      |\r\n",
            victim->pcdata->condition[COND_THIRST],
            victim->pcdata->condition[COND_HUNGER],
            victim->pcdata->condition[COND_FULL],
            victim->pcdata->condition[COND_DRUNK],
            victim->pcdata->tired );

        if (victim->clan){
	        ch->printlnf("| `xClan: %-10s `xRank: %s%-15s (%d)`x                                  |",
				victim->clan->cwho_name(),
				victim->clan->color_str(),
				victim->clan->clan_rank_title(victim->clanrank),
				victim->clanrank);              
		}
    }

    if (!IS_NPC(victim))
    {
		ch->printf("| Played: %d(%0.03f%%)  LastLevel: %-4d ",
            (int) (GET_SECONDS_PLAYED(victim)/ 3600), 
			GET_SECONDS_PLAYED(victim)* 100/ (double)(current_time-victim->player_id),
			victim->pcdata->last_level);
    }

    // display the timer on pcs and only on mobs if they have one 
    if (!IS_NPC(victim)){
		ch->printlnf("Timer: %-2d                         |", victim->timer);
    }else if (victim->timer>0){
        ch->printlnf("Timer: %-2d  Idle: %-2d               |", victim->timer, victim->idle);
	}

    if (IS_NPC(victim) && victim->off_flags)
        ch->printlnf("| Offense: %-66s |",off_bit_name(victim->off_flags));

    if (victim->imm_flags)
		ch->printlnf("| Immune: %-67s |",imm_bit_name(victim->imm_flags));
 
    if (victim->res_flags)
		ch->printlnf("| Resist: %-67s |", imm_bit_name(victim->res_flags));

    if (victim->vuln_flags)
		ch->printlnf("| Vulnerable: %-63s |", imm_bit_name(victim->vuln_flags));

    if (!IS_NULLSTR(victim->prompt))
	{
        sprintf(buf,"| Prompt: %-67s |",victim->prompt);
        ch->printlnbw(buf);	
    }

    if (!IS_NULLSTR(victim->olcprompt))
    {
        sprintf(buf,"| OLCPrompt: %-68s ",victim->olcprompt);
        ch->printlnbw(buf);
    }

	if (victim->comm)
		ch->printlnf("| Comm: %-69s ",comm_bit_name(victim->comm));

	if (victim->config){
		ch->printlnf("| Config: %-67s ", 
			flag_string( config_flags, victim->config));
	}
    
	ch->printlnf("| Act: %-70s ", act_bit_name(victim->act));
    
    if (victim->affected_by)
    {
        ch->printlnf("| Affected by: `c%-63s`x ", 
            affect_bit_name(victim->affected_by));
    }

	ch->printlnf("| Parts: %-68s \n| Form: %-69s |",
        part_bit_name(victim->parts), form_bit_name(victim->form));

    ch->printlnf("| Master: %-15s  Leader: %-15s  Pet: %-15s      |",
        victim->master      ? victim->master->name   : "(none)",
        victim->leader      ? victim->leader->name   : "(none)",
        victim->pet         ? victim->pet->name      : "(none)");
    
	ch->printlnf("| Mounted on: %-15s  Ridden by: %-15s                     |",
        victim->mounted_on      ? victim->mounted_on->name   : "(none)",
        victim->ridden_by       ? victim->ridden_by->name    : "(none)");
    
    if (victim->last_force > -20)
	{
		    ch->printlnf("| Wildness: %-3d  Will: %-3d  Last_force: %-10ld (%-4ld tick%s ago)           |",
				victim->wildness, victim->will,
				victim->last_force, tick_counter -victim->last_force,
				(tick_counter -victim->last_force<=1)?" ":"s");
    }
    else
    {
			ch->printlnf("| Wildness: %-3d  Will: %-3d  Last_force: never                                 |",
				victim->wildness, victim->will);
    }

	ch->println("|_____________________________________________________________________________|\n|                                                                             |");

    ch->printlnf("| Pkills: `R%-2d`x  PkDefeats: `r%-2d`x                                                   |",
             victim->pkkills, victim->pkdefeats);

	ch->printlnf("| Pksafe: `C%-2d`x  PkOOL: `C%-4d`x  PkNorecall: `C%-2d`x  Pknoquit: `C%-2d`x                       |",
			 victim->pksafe, victim->pkool, victim->pknorecall, victim->pknoquit);

    if(!IS_NPC(victim))         
    {
        ch->printlnf("| MKills: `B%-4d`x  MDefeats: `B%-3d`x                              |",
			victim->pcdata->mkills, victim->pcdata->mdefeats);

		if(victim->pcdata->council)
			ch->printlnf("Councils: %s", flag_string( council_flags, victim->pcdata->council ));

        ch->printlnf("| Realm Bits: %s", flag_string( realm_flags, victim->pcdata->realms));
        ch->printlnf("| Sphere Bits: %s", flag_string( sphere_flags, victim->pcdata->spheres));
        ch->printlnf("| Element&Season Bits: %s", flag_string( element_flags, victim->pcdata->elements));			

        ch->printf("| Karns: `Y%d`x    NextKarn: `y%-5d`x  Sec: `c%d`x  Subdued: `c%-2d`x  ",
			victim->pcdata->karns,
			victim->pcdata->next_karn_countdown,
			victim->pcdata->security,
			victim->subdued_timer);
		
		if (IS_ADMIN(ch))
		{
			char buf2[MIL];
			
			ch->printf("Trust: %-3d  ",
				(IS_TRUSTED(victim,get_trust(ch))? get_trust(ch):get_trust(victim)));

			if (IS_SET(victim->act, PLR_LOG))
				sprintf(buf, "`#`RLOGGED`&");
			else
				sprintf(buf, "      ");

			// show xp penality
			if (victim->pcdata->xp_penalty>0)
			{
				sprintf( buf2, "`#`RXP PEN: %-4d `^",victim->pcdata->xp_penalty);
			}else{
				sprintf(buf2, "         ");
			}
			strcat(buf, buf2);

			ch->print(buf);
		}else ch->print("                       ");
		
		ch->println(" |");


		ch->printlnf("| Creation points: %-4d  XPPerLvl: `m%-4d`x  XPTillNextlvl: `m%-5d`x  RPS: `m%-6ld`x    |",
            victim->pcdata->points,
            exp_per_level(victim, victim->pcdata->points),
            (victim->level+1)*exp_per_level(victim, victim->pcdata->points)-(victim->exp),
            victim->pcdata->rp_points );
    }           

	ch->println("|_____________________________________________________________________________|\n|                                                                             |");

    if IS_NPC(victim)
	{
        ch->printlnf("| Short description: %-56s |\r\n| Long  description: %s",
        victim->short_descr,
        IS_NULLSTR(victim->long_descr)? "(none)" : victim->long_descr );
	}else{
        ch->printlnf("| Short description: %-56s |",
        victim->short_descr);
	}


	// ADMIN CAN SEE NOTES BEING WRITTEN
	if (IS_ADMIN(ch) && ch->level > victim->level)
	{	
		if (victim->pnote)
        {
			ch->println("|_____________________________________________________________________________|\n|                                                                             |");

			sprintf(buf,"`#`?%s is writing the following %s:`&", 
				victim->name, get_notetype(victim->pnote->type));
			ch->printlnf("| %-82s|", buf);
            ch->printlnf("| %-15s: %-58s |\n| To: %-71s |",
				victim->pnote->sender,
				victim->pnote->subject,
				victim->pnote->to_list);
			if (IS_SET(victim->act,PLR_AUTOREFORMAT)){
				char *tempdup= note_format_string(str_dup(victim->pnote->text));
				ch->print(tempdup);
				free_string(tempdup);
			}else{
				ch->print(victim->pnote->text);
			}
			ch->print("`x");
		}

	}

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
        ch->printlnf("Mobile has special procedure `G%-20s`x                             |",
			spec_name(victim->spec_fun));
    }

	// where definitions 
    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
//		ch->println("|                                                                             |");
        switch (paf->where)
        {
        case WHERE_WEAPON:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d hr%s (weaponbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                weapon_bit_name( paf->bitvector ),
                paf->level);
            break;

        case WHERE_VULN:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d hr%s (vulnbits %s), lvl %d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                imm_bit_name( paf->bitvector ),
                paf->level);
            break;

        case WHERE_RESIST:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d hr%s (resistbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                imm_bit_name( paf->bitvector ),
                paf->level);
            break;

        case WHERE_IMMUNE:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d hr%s (immunebits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                imm_bit_name( paf->bitvector ),
                paf->level);
            break;

        case WHERE_OBJEXTRA:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d hr%s (objbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                extra_bit_name(paf->bitvector),
                paf->level);
            break;

        case WHERE_AFFECTS:
        default:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d hr%s (affbits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                affect_bit_name( paf->bitvector ),
                paf->level);
            break;
        case WHERE_AFFECTS2:
            sprintf( buf,
                "`WSp:`x '%s' mods %s by %d for %d hr%s (aff2bits %s), lvl%d.",
                skill_table[(int) paf->type].name,
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
				paf->duration==1?"":"s",
                affect2_bit_name( paf->bitvector ),
                paf->level);
            break;
		case WHERE_AFFECTS3:
			sprintf( buf,
				"`WSp:`x '%s' mods %s by %d for %d hr%s (aff3bits %s), lvl%d.",
				skill_table[(int) paf->type].name,
				affect_loc_name( paf->location ),
				paf->modifier,
				paf->duration,
				paf->duration==1?"":"s",
				affect3_bit_name( paf->bitvector ),
				paf->level);
			 break; 
        }
		ch->printlnf("| %-80s|", buf);
    }


	//dawn2?
	// show last time skills were used
	{
		int sn;
		if (!IS_NPC(victim))
		{
			for ( sn = 0; sn < MAX_SKILL; sn++ )
			{
				if ( skill_table[sn].name != NULL && victim->pcdata->last_used[sn] > 0 )
				{
					ch->println("|_____________________________________________________________________________|\n|                                                                             |");

					ch->printlnf("| `YSpell LastUsed:`x '%-25s' at %-15s                    |",
						skill_table[sn].name, 
						(char *) ctime( &victim->pcdata->last_used[sn]) );
				}
			}
		}
	}

	ch->println("|_____________________________________________________________________________|");

    return;
}
/**************************************************************************/
void ostat_show_to_char( char_data *ch, OBJ_DATA *obj);
/**************************************************************************/
void do_debugobject( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	ch->println("Stat what?");
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
		ch->println("Nothing like that in hell, earth, or heaven.");
		return;
    }

	ch->println("`RDEBUG_OBJECT SET TO THE FOLLOWING OBJECT:`x");
	DEBUG_OBJECT=obj;

	ostat_show_to_char(ch, obj);
    return;
}
/**************************************************************************/
void make_corefile();
/**************************************************************************/
void do_makecorefile( char_data *ch, char *)
{
	make_corefile();
	ch->println("`RCorefile created!`x");
}
/**************************************************************************/
