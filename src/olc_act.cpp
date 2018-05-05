/**************************************************************************/
// olc_act.cpp - olc actions
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  File: olc_act.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
#include "include.h" // dawn standard includes

#include "olc.h"
#include "security.h"



bool show_version( char_data *ch, char * )
{
	ch->printlnf("%s\r\n%s\r\n%s\r\n%s",
		VERSION, AUTHOR, DATE, CREDITS );
    return false;
}    

/**************************************************************************/
void show_char_extended(char_data *ch, EXTRA_DESCR_DATA *ed, bool full)
{
	// Extended descriptions
    if (ed)
    {
		int count=0;

        ch->println("`=r---=======================================================================---");
        ch->println("`=rExtended descriptions keywords:");

        for ( ; ed != NULL; ed = ed->next )
        {
            ch->printlnf("`=r%2d) `Y%-60s `g(%4d bytes)",
				++count, ed->keyword, str_len(ed->description));
			if (full)
			{
					if (str_len(ed->description))
                        ch->printf("    `=R%s", // no \r\n needed since in descript
							ed->description);
					else
                        ch->println("    `RNo TEXT!");
			}
			else
			{
				if (str_len(ed->description)>70)
				{
                    ch->printlnf("    `=R%-70.70s`=r...", ltrim_string(ed->description));
				}
				else
				{
					if (str_len(ed->description))
                        ch->printf("    `=R%s", // no \r\n needed since in descript
							ltrim_string(ed->description));
					else
                        ch->println("    `RNo TEXT!");

				}
			}
        }   
        ch->println("`=r---=======================================================================---");
		ch->print("`x");
    }
	else
	{
        ch->println("`=rThere are no extended descriptions.`x");
	}

}

/**************************************************************************/
bool generic_ed(char_data *ch, char *argument )
{
    char command[MIL];
    char keyword[MIL];
	ROOM_INDEX_DATA *pRoom = NULL;
    OBJ_INDEX_DATA *pObj = NULL;
    EXTRA_DESCR_DATA *ed = NULL;

    switch ( ch->desc->editor )
    {
		case ED_ROOM:
			EDIT_ROOM( ch, pRoom );
			break;
		
		case ED_OBJECT:
			EDIT_OBJ(ch, pObj);
			break;

		default:
			bugf("generic_ed: default (%d)", ch->desc->editor);
			return false;
    }

    argument = one_argument( argument, command );
    argument = one_argument( argument, keyword );

    if ( IS_NULLSTR(command))
    {
        ch->println("Syntax:  ed add [keyword]");
        ch->println("         ed edit [keyword]");
        ch->println("         ed delete [keyword]");
        ch->println("         ed format [keyword]");
        ch->println("         ed rekey [keyword] [keywords]");
        ch->println("         ed show");
		return false;
    }

	///////////////////////////////
    if ( !str_cmp( command, "add" ) )
    {
		if ( keyword[0] == '\0' )
		{
            ch->println("Syntax:  ed add [keyword]");
			return false;
		}

		strcat(keyword, " ");
		strcat(keyword, argument);

		ed					= new_extra_descr();
		ed->keyword			= str_dup( keyword );
		ed->description		= str_dup( "" );
		switch ( ch->desc->editor )
		{
			case ED_ROOM:
				ed->next = pRoom->extra_descr;
				pRoom->extra_descr	= ed;
				break;	
			case ED_OBJECT:
				ed->next = pObj->extra_descr;
				pObj->extra_descr	= ed;
				break;
		}
		string_append( ch, &ed->description );

		return true;
    }


	///////////////////////////////
    if ( !str_cmp( command, "edit" ) )
    {
		if ( keyword[0] == '\0' )
		{
            ch->println("Syntax:  ed edit [keyword]");
			return false;
		}

		switch ( ch->desc->editor )
		{
			case ED_ROOM:
				for ( ed = pRoom->extra_descr; ed; ed = ed->next )
				{
					if ( is_name( keyword, ed->keyword ) )
					break;
				}
				break;	
			case ED_OBJECT:
				for ( ed = pObj->extra_descr; ed; ed = ed->next )
				{
					if ( is_name( keyword, ed->keyword ) )
					break;
				}
				break;
		}

		if ( !ed )
		{
            ch->printlnf("generic_ed:  Extra description keyword '%s' not found.", keyword);
			return false;
		}

		string_append( ch, &ed->description );

		return true;
    }

	///////////////////////////////
    if ( !str_cmp( command, "delete" ) )
    {
		EXTRA_DESCR_DATA *ped = NULL;

		if ( keyword[0] == '\0' )
		{
            ch->println("Syntax:  ed delete [keyword]");
			return false;
		}

		switch ( ch->desc->editor )
		{
			case ED_ROOM:
				for ( ed = pRoom->extra_descr; ed; ed = ed->next )
				{
					if ( is_name( keyword, ed->keyword ) )
					break;
					ped = ed;
				}
				break;	
			case ED_OBJECT:
				for ( ed = pObj->extra_descr; ed; ed = ed->next )
				{
					if ( is_name( keyword, ed->keyword ) )
					break;
					ped = ed;
				}
				break;
		}


		if ( !ed )
		{
            ch->println("generic_ed:  Extra description keyword not found.");
			return false;
		}

		if ( !ped )
			switch ( ch->desc->editor )
			{
				case ED_ROOM:
					pRoom->extra_descr = ed->next;
					break;	
				case ED_OBJECT:
					pObj->extra_descr = ed->next;
					break;
			}
		else
			ped->next = ed->next;

		free_extra_descr( ed );

        ch->println("Extra description deleted.");
		return true;
    }

	///////////////////////////////
    if ( !str_cmp( command, "format" ) )
    {
		if ( keyword[0] == '\0' )
		{
            ch->println("Syntax:  ed format [keyword]");
			return false;
		}

		switch ( ch->desc->editor )
		{
			case ED_ROOM:
				for ( ed = pRoom->extra_descr; ed; ed = ed->next )
				{
					if ( is_name( keyword, ed->keyword ) )
					break;
				}
				break;	
			case ED_OBJECT:
				for ( ed = pObj->extra_descr; ed; ed = ed->next )
				{
					if ( is_name( keyword, ed->keyword ) )
					break;
				}
				break;
		}


		if ( !ed )
		{
            ch->println("generic_ed:  Extra description keyword not found.");
			return false;
		}

		ed->description = format_string( ed->description );

        ch->println("Extra description formatted.");
		return true;
    }
	
	///////////////////////////////
    if ( !str_cmp( command, "rekey" ) )
    {
		if ( keyword[0] == '\0' )
		{
            ch->println("Syntax:  ed rekey [keyword] [keywords]");
            ch->println(" notes:  use only 1 keyword for [keyword]");
            ch->println("         use all the keywords for [keywords]");
			return false;
		}

		switch ( ch->desc->editor )
		{
			case ED_ROOM:
				for ( ed = pRoom->extra_descr; ed; ed = ed->next )
				{
					if ( is_name( keyword, ed->keyword ) )
					break;
				}
				break;	
			case ED_OBJECT:
				for ( ed = pObj->extra_descr; ed; ed = ed->next )
				{
					if ( is_name( keyword, ed->keyword ) )
					break;
				}
				break;
		}

		if ( !ed )
		{
            ch->printlnf("generic_ed:  Extra description keyword '%s' not found.", keyword);
			return false;
		}

		if (IS_NULLSTR(argument)){
            ch->printlnf("generic_ed:  Need to specify which keyword you want to rekey '%s' to also.", ed->keyword);
			return false;
		}

		ch->wraplnf("Extra description with keywords '%s' "
            "has had its keywords replaced with '%s'.", ed->keyword, argument);
		replace_string(ed->keyword, argument);
		return true;
    }

	///////////////////////////////
    if ( !str_cmp( command, "show" ) )
    {
		switch ( ch->desc->editor )
		{
			case ED_ROOM:
				show_char_extended(ch, pRoom->extra_descr, true);
				break;	
			case ED_OBJECT:
				show_char_extended(ch, pObj->extra_descr, true);
				break;
		}
	    return false;
	}

    generic_ed( ch, "" );
    return false;
}
/**************************************************************************/
/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
/**************************************************************************/
const struct olc_help_type help_table[] =
{
    {   "olcarea",			olc_flags,			"OLCArea attributes."			},
    {   "area",				area_flags,			"Area attributes."				},
    {	"room",				room_flags,			"Room attributes."				},
    {	"sector",			sector_types,		"Sector types, terrain."		},
    {	"exit",				exit_flags,			"Exit types."					},
    {	"type",				item_types,			"Types of objects."				},
    {	"extra",			objextra_flags,		"Object attributes."			},
	{	"extra2",			objextra2_flags,	"Second set of EXTRA flags."	},
    {	"wear",				wear_flags,			"Where to wear object."			},
    {	"spec",				spec_table,			"Available special programs."	},
    {	"sex",				sex_types,			"Sexes."						},
    {	"act",				act_flags,			"Mobile attributes."			},
	{	"act2",				act2_flags,			"Second set of ACT flags."		},
    {	"affect",			affect_flags,		"Mobile affects."				},
	{	"affect2",			affect2_flags,		"Second set of AFF flags."		},
	{	"affect3",			affect3_flags,		"Third set of AFF flags."		},
    {	"wear-loc",			wear_location_types,		"Where mobile wears object."	},
    {	"spells",			skill_table,		"Names of current spells."		},
    {	"container",		container_flags,	"Container status."				},

    {	"armor",			ac_types,			"Ac for different attacks."		},
    {   "apply",			apply_types,		"Apply flags"					},
    {	"form",				form_flags,			"Mobile body form."				},
    {	"part",				part_flags,			"Mobile body parts."			},
    {	"imm",				imm_flags,			"Mobile immunity."				},
    {	"res",				res_flags,			"Mobile resistance."			},
    {	"vuln",				vuln_flags,			"Mobile vulnerability."			},
    {	"off",				off_flags,			"Mobile offensive behaviour."	},
    {	"size",				size_types,			"Mobile size."					},
    {   "position",			position_types,		"Mobile positions."				},
    {   "wclass",			weapon_class_types,		"Weapon class."					}, 
    {   "wtype",			weapon_flags,		"Special weapon type."			},
    {	"portal",			portal_flags,		"Portal types."					},
    {	"furniture",		furniture_flags,	"Furniture types."				},
    {   "liquid",			liq_table,			"Liquid types."					},
    {	"apptype",			to_types,		"Apply types."					},
    {	"weapon",			attack_table,		"Weapon types."					},
	{	"ospec",			ospec_table,		"Object special programs."		},
    {	"mprog",			mprog_flags,		"MudProgram flags."				},
    {	"ban_types",		ban_types,			"Ban types."					},
	{	"council",			council_flags,		"Council flags."				},
	{	"cmdflags",			commandflag_flags,	"Command flags."				},
	{	"attune",			attune_flags,		"Attune Flags."					},
	{	"mixtypes",			mixtype_types,		"Mix Type Flags."				},
	{	"alignflags",		align_flags,		"Alignment flags."				},
	{	"tendflags",		tendency_flags,		"Tendency flags."				},
    {	NULL,		NULL,		 NULL				 }
};



/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds( char_data *ch, const struct flag_type *flag_table )
{
	char buf  [MSL];
	char buf1 [MSL];
	int  flag;
	int  col;

	buf1[0] = '\0';
	col = 0;
	for (flag = 0; flag_table[flag].name != NULL; flag++)
	{
		if ( flag_table[flag].settable )
		{
			sprintf( buf, "%-19.18s", flag_table[flag].name );
			strcat( buf1, buf );
			if ( ++col % 4 == 0 )
				strcat( buf1, "\r\n" );
		}
	}

	if ( col % 4 != 0 ){
		strcat( buf1, "\r\n" );
	}
	ch->print(buf1);
	return;
}


/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds( char_data *ch, int tar )
{
	char buf  [ MSL ];
	char buf1 [ MSL*2 ];
	int  sn;
	int  col;

	buf1[0] = '\0';
	col = 0;
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if ( !skill_table[sn].name )
			break;
		if ( !str_cmp( skill_table[sn].name, "reserved" )
			|| skill_table[sn].spell_fun == spell_null )
			continue;
		if ( tar == -1 || skill_table[sn].target == tar )
		{
			sprintf( buf, "%-19.18s", skill_table[sn].name );
			strcat( buf1, buf );
			if ( ++col % 4 == 0 )
				strcat( buf1, "\r\n" );
		}
	}

	if ( col % 4 != 0 ){
		strcat( buf1, "\r\n" );
	}
	ch->print(buf1);
	return;
}



/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions for MOBS
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds( char_data *ch )
{
	char buf[MSL];
	char buf1[MSL];
	int  spec;
	int  col;

	buf1[0] = '\0';
	col = 0;
	for (spec = 0; spec_table[spec].spec_fun != NULL; spec++)
	{
		sprintf( buf, "%-19.18s", &spec_table[spec].spec_name[5] );
		strcat( buf1, buf );
		if ( ++col % 4 == 0 )
			strcat( buf1, "\r\n" );
	}
	if ( col % 4 != 0 )
		strcat( buf1, "\r\n" );
	ch->print( buf1);
	return;
}

/*****************************************************************************
 Name:		show_ospec_cmds
 Purpose:	Displays settable special functions for OBJECTS
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_ospec_cmds( char_data *ch )
{
	char buf[MSL];
	char buf1[MSL];
	int  spec;
	int  col;

	buf1[0] = '\0';
	col = 0;
	for ( spec = 0; ospec_table[spec].ospec_fun != NULL; spec++ )
	{
		sprintf( buf, "%-19.18s", &ospec_table[spec].ospec_name[5] );
		strcat( buf1, buf );
		if ( ++col % 4 == 0 )
			strcat( buf1, "\r\n" );
	}
	if ( col % 4 != 0 )
		strcat( buf1, "\r\n" );
	ch->print( buf1);
	return;
}


/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help( char_data *ch, char *argument )
{
    char arg[MIL];
    char spell[MIL];
    int cnt;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    /*
     * Display syntax.
     */
    if ( arg[0] == '\0' )
    {
    ch->println("Syntax:  ? [command]\r\n");
    ch->println("[command]  [description]");
	for (cnt = 0; help_table[cnt].command != NULL; cnt++)
	{
        ch->printlnf("%-10.10s -%s",
			capitalize( help_table[cnt].command ),
			help_table[cnt].desc );
	}
	return false;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
    for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
		if (  arg[0] == help_table[cnt].command[0]
			&& !str_prefix( arg, help_table[cnt].command ) )
		{
			if ( help_table[cnt].structure == spec_table )
			{
				show_spec_cmds( ch );
				return false;
			}
			else if ( help_table[cnt].structure == ospec_table )
			{
				show_ospec_cmds( ch );
				return false;
			}

			else if ( help_table[cnt].structure == liq_table )
			{
				show_liqlist( ch );
				return false;
			}
			else if ( help_table[cnt].structure == attack_table )
			{
				show_damlist( ch );
				return false;
			}
			else if ( help_table[cnt].structure == skill_table )
			{
				if ( spell[0] == '\0' )
				{
					ch->println( "Syntax:  ? spells [ignore/attack/defend/self/object/all]");
					return false;
				}

				if ( !str_prefix( spell, "all" ) )
					show_skill_cmds( ch, -1 );
				else if ( !str_prefix( spell, "ignore" ) )
					show_skill_cmds( ch, TAR_IGNORE );
				else if ( !str_prefix( spell, "attack" ) )
					show_skill_cmds( ch, TAR_CHAR_OFFENSIVE );
				else if ( !str_prefix( spell, "defend" ) )
					show_skill_cmds( ch, TAR_CHAR_DEFENSIVE );
				else if ( !str_prefix( spell, "self" ) )
					show_skill_cmds( ch, TAR_CHAR_SELF );
				else if ( !str_prefix( spell, "object" ) )
					show_skill_cmds( ch, TAR_OBJ_INV );
				else
					ch->println("Syntax:  ? spell [ignore/attack/defend/self/object/all]");

				return false;
			}
			else
			{
				show_flag_cmds( ch, (const struct flag_type *)help_table[cnt].structure );
				return false;
			}
		}
    }
	show_help( ch, "" );
    return false;
}
/**************************************************************************/
// by Kal - June 98
EXTRA_DESCR_DATA * dup_extdescr_list(EXTRA_DESCR_DATA * descript)
{
	EXTRA_DESCR_DATA *ed;

	if (descript==NULL)
		return (NULL);

	ed = new_extra_descr();
	// use recursion to maintain the order of descriptions
	ed->next = dup_extdescr_list(descript->next);
    ed->keyword		= str_dup(descript->keyword);
	ed->description = str_dup(descript->description);

	return (ed);
}
/**************************************************************************/
// Tristan - May 04
void do_apurge( char_data *ch, char *argument )
{
		AREA_DATA			*pArea = ch->in_room->area;
		ROOM_INDEX_DATA		*location;
		char_data			*victim, *vnext;
		OBJ_DATA			*obj, *obj_next;
		char				buf[MSL];
		bool				purge_all = false;

		if( !HAS_SECURITY( ch, 1 ) )
		{
			do_huh( ch, "" );
			return;
		}

		if( IS_NULLSTR( argument )
			|| (str_cmp( "confirm", argument ) && str_cmp( "all confirm", argument ) ) )
		{
			ch->println( "`#`CSyntax:`^ apurge [all] confirm" );
			return;
		}

		if( !str_cmp( "all confirm", argument ) )
			purge_all = true;

		for( int vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		{
			if(( location = get_room_index( vnum )))
			{
				for( victim = location->people; victim != NULL; victim = vnext )
				{
					vnext = victim->next_in_room;
					if( IS_NPC( victim )
						&& ( !IS_SET( victim->act, ACT_NOPURGE ) || purge_all )
						&& victim != ch )

						extract_char( victim, true );

					else if( IS_NPC( victim ) && IS_SET( victim->act, ACT_NOPURGE ) )
					{
						ch->printlnf( "Mob '%s' <`#`G%i`^> in room `#`Y%i`^ NOT purged!",
							victim->short_descr,
							victim->pIndexData->vnum,
							victim->in_room->vnum );
					}
				}

				for( obj = location->contents; obj != NULL; obj = obj_next )
				{
					obj_next = obj->next_content;

					if( !IS_OBJ_STAT( obj, OBJEXTRA_NOPURGE ) || purge_all )
						extract_obj( obj );

					else
					{
						ch->printlnf( "Obj '%s' <`#`R%i`^> in room `#`Y%i`^ NOT purged!",
							obj->short_descr,
							obj->pIndexData->vnum,
							obj->in_room->vnum );
					}
				}
			}
		}

		if( !IS_SET( pArea->area_flags, AREA_OLCONLY ) )
		{
			sprintf( buf, "%s purged area '%s' [%d-%d] (Not flagged OLC Only!)",
				ch->name,
				pArea->name,
				pArea->min_vnum,
				pArea->max_vnum );
			wiznet( buf, NULL, NULL, WIZ_SECURE, 0, AVATAR );
		}

		ch->printlnf( "Area '%s' [%i-%i] purged. Remember to use `#`Yresetarea`^.",
			pArea->name,
			pArea->min_vnum,
			pArea->max_vnum );

		return;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/





