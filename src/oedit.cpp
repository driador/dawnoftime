/**************************************************************************/
// oedit.cpp - olc object editor
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

DECLARE_OLC_FUN( oedit_create );
void do_classlist( char_data *ch, char *);

/**************************************************************************/
extern gameset_value_type gameset_value[];

/**************************************************************************/
void do_material_list( char_data *ch, char *argument)
{
	name_linkedlist_type* materials_list=NULL, *plist;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum,count;
	BUFFER *output;
	char buf[MIL];


	if(IS_NULLSTR(argument)){
		logf("do_material_list(): Sorting materials");
		// add all objects to the linked list, duplicates=false
		for(vnum=0; vnum<MAX_KEY_HASH; vnum ++){
			count=0;
			for ( pObjIndex  = obj_index_hash[vnum];
			  pObjIndex != NULL;
			  pObjIndex  = pObjIndex->next )
			{
				  count++;
				addlist(&materials_list,pObjIndex->material, 0, false, false);
				if(count>20000){
					bugf("Count=%5d, objvnum=%d", count, pObjIndex->vnum);
				}
			}
		}
		logf("do_material_list(): Displaying materials to buffer");
		// go thru displaying all objects
		output = new_buf();  
		count=0;
		for(plist=materials_list;plist; plist=plist->next){
			sprintf(buf, "%3d> %s\n", ++count, plist->name); 
			add_buf( output, buf );
		}
		logf("do_material_list(): Sending buffer to player");
		ch->printlnf( "%d material%s total.",
			count,
			count==1 ? "" : "s" );
		ch->sendpage(buf_string(output));
		free_buf(output);
	}else{
		// do a search for a specified item
		count=0;
		output = new_buf();  
		for(vnum=0; vnum<33000; vnum++){
			pObjIndex =get_obj_index(vnum);
			if(pObjIndex){
				if(is_name(argument,pObjIndex->material)){
					sprintf(buf, "%3d> [%5d] %s   `Smaterial='%s'`x\n", 
						++count, pObjIndex->vnum, 
						pObjIndex->short_descr, pObjIndex->material); 
					add_buf( output, buf );
				}
			}
			if(count>400){
				ch->println( "You can only list up to 400 items at once, be more specific." );
				break;
			}
		}
		// display the list to the character
		ch->printlnf( "Displaying %d material%s total.",
			count,
			count==1 ? "" : "s" );
		ch->sendpage(buf_string(output));
		free_buf(output);
	}
}
/**************************************************************************/
void do_oedit( char_data *ch, char *argument )
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	char arg1[MSL];
	int value;

	// do security checks
	if (!HAS_SECURITY(ch, 2))
	{
		ch->println( "You must have an olc security 2 or higher to use this command." );
		return;
	}

	argument = one_argument( argument, arg1 );

	if ( is_number( arg1 ) )
	{
		value = atoi( arg1 );
		if ( !( pObj = get_obj_index( value )))
		{
			ch->println( "OEdit:  That vnum does not exist." );
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


		if ( !IS_BUILDER( ch, pObj->area, BUILDRESTRICT_OBJECTS) )
		{
			ch->println( "Insufficient security to modify object." );
			return;
		}

		ch->desc->pEdit = (void *)pObj;
		ch->desc->editor = ED_OBJECT;

		ch->wraplnf( "`=rYou are now editing object: '`r%s`=r' vnum: `Y%d`x",
			pObj->short_descr, pObj->vnum);
		ch->println( "`=rType `=Cdone`=r to finish editing." );

		return;
	}
	else
	{
		if ( !str_cmp( arg1, "create" ) )
		{
			value = atoi( argument );
			if ( argument[0] == '\0' || value == 0 )
			{
				ch->println( "Syntax:  oedit create <vnum>" );
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
				ch->println( "OEdit:  That vnum is not assigned an area." );
				return;
			}

			if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_OBJECTS ))
			{
				ch->println( "Insufficient security to modify object." );
				return;
			}

			if ( oedit_create( ch, argument ) )
			{
				SET_BIT( pArea->olc_flags, OLCAREA_CHANGED );
				ch->desc->editor = ED_OBJECT;
				EDIT_OBJ(ch, pObj);

				ch->wraplnf( "`=rcreated object `r%d  `g(stored in %s)`x",
					pObj->vnum, !pObj->area ? "`RNo Area!!!" : pObj->area->file_name );
				ch->println( "Type `=Cdone`x to finish editing." );
			}
			return;
		}
	}

	ch->wrapln( "OEdit:  Type the vnum of the object you want to edit or 'oedit create <vnum>" );
	return;
}

/**************************************************************************/
// by Kal - June 98
AFFECT_DATA * dup_affects_list(AFFECT_DATA* affect)
{
	AFFECT_DATA * pAffect;
	
	if (affect==NULL)
		return (NULL);

	pAffect= new_affect();
	// use recursion to maintain the order of affects
	pAffect->next = dup_affects_list(affect->next);
	
	pAffect->where		= affect->where;
	pAffect->type		= affect->type;
	pAffect->duration	= affect->duration;
	pAffect->location	= affect->location;
	pAffect->modifier	= affect->modifier;
	pAffect->bitvector	= affect->bitvector;

	return (pAffect);
}
/**************************************************************************/
/*
 * Object Editor Functions.
 */
/**************************************************************************/
void show_obj_values( char_data *ch, OBJ_INDEX_DATA *obj )
{
    switch( obj->item_type )
    {
	default:	// No values. 
	    break;

	case ITEM_LIGHT:
		if ( obj->value[2] == -1)
			ch->println(  "[v2] Light:  Infinite[-1]" );
		else
			ch->printlnf( "[v2] Light:  [%d]", obj->value[2] );
		break;

	case ITEM_INSTRUMENT:
		ch->printlnf(     "[v0] Bardic Spell Mod:      [%d]", obj->value[0] );
		if ( obj->value[1] == -1 )
			ch->println("[v1] Uses before retuning:  Infinite[-1]");
		else
			ch->printlnf( "[v1] Uses before retuning:  [%d]", obj->value[1] );
		break;


	case ITEM_COMPONENT:
		ch->printlnf( "[v0] Charges: [%d]", obj->value[0] );
		ch->printlnf( "[v1] Spell:   %s",
			obj->value[1] != -1 ? skill_table[obj->value[1]].name : "none" );
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		ch->printlnf( "[v0] Level:          [%d]", obj->value[0] );
		ch->printlnf( "[v1] Charges Total:  [%d]", obj->value[1] );
		ch->printlnf( "[v2] Charges Left:   [%d]", obj->value[2] );
		ch->printlnf( "[v3] Spell:          %s",
			obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none"  );
	    break;

	
	case ITEM_POULTICE:
		ch->printlnf( "[v0] Level:               [%d]", obj->value[0] );
		ch->printlnf( "[v1] Applications Total:  [%d]", obj->value[1] );
		ch->printlnf( "[v2] Applications Left:   [%d]", obj->value[2] );
		ch->printlnf( "[v3] Spell:                %s",
			obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none" );
	    break;

	case ITEM_PORTAL:
		ch->printlnf( "[v0] Charges:        [%d]", obj->value[0] );
		ch->printlnf( "[v1] Exit Flags:     %s", flag_string( exit_flags, obj->value[1] ));
		ch->printlnf( "[v2] Portal Flags:   %s", flag_string( portal_flags, obj->value[2] ));
		ch->printlnf( "[v3] Goes to (vnum): [%d]", obj->value[3] );
		if(CAN_WEAR(obj, OBJWEAR_TAKE))
		{
			ch->wrapln( "`#`RNOTE:`& Because this portal is takeable, it wont be able "
				"to be used in no recall rooms/areas\r\n");
		}
		break;

	case ITEM_TOKEN:
		ch->printlnf( "[v0] Token:  [%s]",	flag_string( token_flags, obj->value[0] ));
		break;
	    
	case ITEM_FURNITURE:
		ch->printlnf( "[v0] Max people:      [%d]", obj->value[0] );
		ch->printlnf( "[v1] Max weight:      [%d]", obj->value[1] );
		ch->printlnf( "[v2] Furniture Flags: %s", flag_string( furniture_flags, obj->value[2] ));
		ch->printlnf( "[v3] Heal bonus:      [%d]", obj->value[3] );
		ch->printlnf( "[v4] Mana bonus:      [%d]", obj->value[4] );
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		ch->printlnf( "[v0] Level:  [%d]", obj->value[0] );
		ch->printlnf( "[v1] Spell:  %s", obj->value[1] != -1 ? skill_table[obj->value[1]].name : "none" );
		ch->printlnf( "[v2] Spell:  %s", obj->value[2] != -1 ? skill_table[obj->value[2]].name : "none" );
		ch->printlnf( "[v3] Spell:  %s", obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none" );
		ch->printlnf( "[v4] Spell:  %s", obj->value[4] != -1 ? skill_table[obj->value[4]].name : "none" );
	    break;

	case ITEM_ARMOR:
	    ch->printlnf( "[v0] Ac pierce       [%d]", obj->value[0] );
		ch->printlnf( "[v1] Ac bash         [%d]", obj->value[1] );
		ch->printlnf( "[v2] Ac slash        [%d]", obj->value[2] );
		ch->printlnf( "[v3] Ac exotic       [%d]", obj->value[3] );
		break;

	case ITEM_PARCHMENT:
	    ch->printlnf( "[v0] Unused          [%d]", obj->value[0] );
		ch->printlnf( "[v1] Blank = 0       [%d]", obj->value[1] );
		ch->printlnf( "[v2] Unsealed = 0    [%d]", obj->value[2] );
		ch->printlnf( "[v3] Language        [%s]", 
			language_safe_lookup_by_id(obj->value[3])->name);
		break;

	case ITEM_WEAPON:
		ch->printlnf( "[v0] Weapon class:   %s",
			flag_string( weapon_class_types, obj->value[0] ));
		ch->printlnf( "[v1] Number of dice: [%d]   (avedam %d)",
			obj->value[1], 
			(int) (obj->value[1]+(obj->value[1] * obj->value[2]))/2);
		ch->printlnf( "[v2] Type of dice:   [%d]", obj->value[2] );
		ch->printlnf( "[v3] Attack Type:    %s", attack_table[obj->value[3]].name );
		ch->printlnf( "[v4] Special type:   %s", flag_string( weapon_flags,  obj->value[4] ) );
		ch->printlnf( "Average damage is %d", 
			(int) (obj->value[1]+(obj->value[1] * obj->value[2]))/2);
	    break;

	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
		ch->printlnf( "[v0] Weight:     [%d lbs]", obj->value[0]);
		ch->printlnf( "[v1] Flags:      [%s]", flag_string( container_flags, obj->value[1] ));
		ch->printlnf( "[v2] Key:        [%d] (%s)",
			obj->value[2],
			get_obj_index(obj->value[2]) ? get_obj_index(obj->value[2])->short_descr : "none");
		ch->printlnf( "[v3] Capacity    [%d lbs]", obj->value[3]);
		ch->printlnf( "[v4] Weight Mult [%d]", obj->value[4] );

		ch->wrapln("`YNotes:`x v0 = is the maximum combined weight the container can hold... that is "
			"obtained by adding all objects in the containers true weights together.`1"
			"v3 = is the maximum weight any single object can be to be able to be put inside the container.");			
		break;

	case ITEM_FOUNTAIN:
	case ITEM_DRINK_CON:
		ch->printlnf( "[v0] Liquid Total: [%d] (-1 = infinite)", obj->value[0] );
		ch->printlnf( "[v1] Liquid Left:  [%d] (-1 = infinite)", obj->value[1] );
		ch->printlnf( "[v2] Liquid:       %s", liq_table[obj->value[2]].liq_name );
		ch->printlnf( "[v3] Poisoned:     %s", obj->value[3] != 0 ? "Yes": "No" );
		break;

	case ITEM_FOOD:
		ch->printlnf( "[v0] Food hours: [%d]", obj->value[0] );
		ch->printlnf( "[v1] Full hours: [%d]", obj->value[1] );
		ch->printlnf( "[v3] Poisoned:   %s", obj->value[3] != 0 ? "Yes" : "No" );
		break;

	case ITEM_MONEY:
		ch->printlnf( "[v0] Silver: [%d]", obj->value[0] );
		ch->printlnf( "[v1] Gold:   [%d]", obj->value[1] );
		break;


	case ITEM_SHEATH:
		ch->printlnf( "[v0] Weapon Capacity: [%d]", obj->value[0] );
		break;

    }

	return;
}


/**************************************************************************/
bool set_obj_values( char_data *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument)
{
	int value;
	
    switch( pObj->item_type )
    {
	default:
		break;
		
	case ITEM_LIGHT:
		switch ( value_num )
		{
		default:
			ch->println( "Only valid option is V2, which is the hours of light a light provides (-1 = Infinite)." );
			do_help( ch, "OLC-ITEM-LIGHT" );
			return false;
		case 2:
			ch->println( "HOURS OF LIGHT SET.\r\n" );
			pObj->value[2] = atoi( argument );
			break;
		}
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		switch ( value_num )
		{
		default:
			do_help( ch, "OLC-ITEM-STAFF-WAND" );
			return false;
        case 0:
			value=atoi( argument );
			if(value==pObj->value[0]){
				ch->println( "Spell level unchanged." );
			}else{
				ch->printlnf( "Spell level changed from %d to %d.\r\n", // Extra lf
					pObj->value[0], value);
				pObj->value[0] = value;
			}
            break;
        case 1:
			ch->println( "TOTAL NUMBER OF CHARGES SET.\r\n" );	// Extra lf
            pObj->value[1] = atoi( argument );
            break;
        case 2:
			ch->println( "CURRENT NUMBER OF CHARGES SET.\r\n" );// Extra lf
			pObj->value[2] = atoi( argument );
			break;
		case 3:
			ch->println( "SPELL TYPE SET.\r\n" );				// Extra lf
			pObj->value[3] = skill_lookup( argument );
			break;
		}
		break;

	case ITEM_POULTICE:
		switch ( value_num )
		{
		default:
			do_help( ch, "OLC-ITEM-POULTICE" );
			return false;
		case 0:
			value=atoi( argument );
			if(value==pObj->value[0]){
				ch->println( "Spell level unchanged." );
			}else{
				ch->printlnf( "Spell level changed from %d to %d.\r\n", // Extra lf
					pObj->value[0], value);
				pObj->value[0] = value;
			}
			break;
		case 1:
			ch->println( "TOTAL NUMBER OF APPLICATIONS SET.\r\n" );
			pObj->value[1] = atoi( argument );
			break;
		case 2:
			ch->println( "CURRENT NUMBER OF APPLICATIONS SET.\r\n" );
			pObj->value[2] = atoi( argument );
			break;
		case 3:
			ch->println( "SPELL TYPE SET.\r\n" );
			pObj->value[3] = skill_lookup( argument );
			break;
		}
		break;

	case ITEM_INSTRUMENT:
		switch ( value_num )
		{
		default:
			do_help( ch, "OLC-ITEM-INSTRUMENT" );
			return false;
		case 0:
			ch->println( "BARDIC SONG MOD SET." );
			pObj->value[0] = atoi( argument );
			break;
		case 1:
			ch->println( "# OF SONGS BEFORE RETUNING SET.  (-1 is infinite)" );
			pObj->value[1] = atoi(argument);
			break;
		}
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		switch ( value_num )
	    {
	        default:
		    do_help( ch, "OLC-ITEM-SCROLL-POTION-PILL" );
	            return false;
	        case 0:
				value=atoi( argument );
				if(value==pObj->value[0]){
					ch->println( "Spell level unchanged." );
				}else{
					ch->printlnf( "Spell level changed from %d to %d.\r\n", // Extra lf
						pObj->value[0], value);
					pObj->value[0] = value;
				}
	            break;
	        case 1:
                ch->println( "SPELL TYPE 1 SET.\r\n" );		// Extra lf
	            pObj->value[1] = skill_lookup( argument );
	            break;
	        case 2:
                ch->println( "SPELL TYPE 2 SET.\r\n" );		// Extra lf
	            pObj->value[2] = skill_lookup( argument );
	            break;
	        case 3:
                ch->println( "SPELL TYPE 3 SET.\r\n" );		// Extra lf
	            pObj->value[3] = skill_lookup( argument );
	            break;
	        case 4:
                ch->println( "SPELL TYPE 4 SET.\r\n" );		// Extra lf
	            pObj->value[4] = skill_lookup( argument );
	            break;
 	    }
	    break;


	case ITEM_PARCHMENT:
		switch ( value_num )
		{
		default: return false;
		case 0:  return false;
		case 1:
			ch->println( "Written on field set.  Blank = 0 all else means it can't be written on." );
			pObj->value[1] = atoi( argument );
			break;
		case 2:
			ch->println( "Sealed field set.  Unsealed = 0 all else means it's sealed." );
			pObj->value[2] = atoi( argument );
			break;
		case 3:
			pObj->value[3] = language_safe_lookup(argument)->unique_id;				
			ch->printlnf( "Language set to '%s'.", language_safe_lookup(argument)->name);
			break;
		}
		break;

	case ITEM_ARMOR:
    switch ( value_num )
    {
        default:
			do_help( ch, "OLC-ITEM-ARMOR" );
			return false;
        case 0:
			ch->println( "AC PIERCE SET.\r\n" );		// Extra lf
			pObj->value[0] = atoi( argument );
			break;
		case 1:
			ch->println( "AC BASH SET.\r\n" );			// Extra lf
			pObj->value[1] = atoi( argument );
			break;
		case 2:
			ch->println( "AC SLASH SET.\r\n" );			// Extra lf
			pObj->value[2] = atoi( argument );
			break;
		case 3:
			ch->println( "AC EXOTIC SET.\r\n" );		// Extra lf
			pObj->value[3] = atoi( argument );
			break;
	}
	break;

    case ITEM_WEAPON:
		switch ( value_num )
		{
		default:
			ch->println( "Valid v0 weapon class:");
			show_olc_flags_types_value(ch, weapon_class_types,"v0", pObj->value[0]);
			ch->println( "v1 = number of dice to roll.");
			ch->println( "v2 = type of dice to roll.");
			ch->println( "v3 = weapon attack type.");
			ch->println( "v4 = special weapon type.");			
			show_olc_flags_types_value(ch, weapon_flags,"v4", pObj->value[4]);
			ch->println( "");
			ch->println( "SeeAlso: HELP `=_OLC-ITEM-WEAPON");
			return false;
		case 0:
			{
				if ( ( value = flag_value( weapon_class_types, argument ) ) != NO_FLAG ){
					ch->println( "WEAPON CLASS SET.\r\n" );
					pObj->value[0] = flag_value( weapon_class_types, argument );
				}else{
					ch->println( "Valid v0 weapon classes include:");
					show_olc_flags_types_value(ch, weapon_class_types,"v0", pObj->value[0]);
					return false;
				}
			}
			break;
		case 1:
			ch->println( "NUMBER OF DICE SET.\r\n" );
			pObj->value[1] = atoi( argument );
			ch->printlnf( "Average damage is now %d.",
				(int) (pObj->value[1]+(pObj->value[1] * pObj->value[2]))/2);
			break;
		case 2:
			ch->println( "TYPE OF DICE SET.\r\n" );
			pObj->value[2] = atoi( argument );
			ch->printlnf( "Average damage is now %d.",
				(int) (pObj->value[1]+(pObj->value[1] * pObj->value[2]))/2);
			break;
		case 3:
			{
				int att=attack_lookup_with_error( argument );
				if(att<0){
					ch->printlnf("Attack type '%s' not found, must be one of the following:",
						argument);
				    for ( att= 0; attack_table[att].name != NULL; att++)
					{
						ch->printf("   %-12s", attack_table[att].name);
						if(att%4==3){
							ch->print_blank_lines(1);
						}				
					}
				}else{
					ch->println( "WEAPON ATTACK TYPE SET.\r\n" );
					pObj->value[3] = att;
				}
			}
			break;
		case 4:
			ch->println( "SPECIAL WEAPON TYPE TOGGLED.\r\n" );
			pObj->value[4] ^= (flag_value( weapon_flags, argument ) != NO_FLAG
				? flag_value( weapon_flags, argument ) : 0 );
			break;
	}
	break;

	case ITEM_PORTAL:
		switch ( value_num )
		{
		default:
			ch->printlnf( "Valid v1 portal exit flags are: %s", flag_string( exit_flags, -1));
			ch->printlnf( "Valid v2 portal flags are: %s", flag_string( portal_flags, -1));			
			return false;
		case 0:
			ch->println( "CHARGES SET.\r\n" );
			pObj->value[0] = atoi ( argument );
			break;
		case 1:
			if ( ( value = flag_value( exit_flags, argument ) ) != NO_FLAG )
				TOGGLE_BIT(pObj->value[1], value);
			else
			{
				ch->printlnf( "Valid v1 portal exit flags are: %s",
					flag_string( exit_flags, -1));
				do_help ( ch, "OLC-ITEM-PORTAL" );
				return false;
			}
			ch->println( "PORTAL EXIT FLAG SET.\r\n" );
			break;
		case 2:
			if ( ( value = flag_value( portal_flags, argument ) ) != NO_FLAG )
				TOGGLE_BIT(pObj->value[2], value);
			else
			{
				ch->printlnf( "Valid v2 portal flags are: %s",
					flag_string( portal_flags, -1));
				do_help ( ch, "OLC-ITEM-PORTAL" );
				return false;
			}
			ch->println( "PORTAL EXIT FLAG SET.\r\n" );
			break;
		case 3:
			ch->println( "EXIT VNUM SET.\r\n" );
			pObj->value[3] = atoi ( argument );
			break;
		}
		break;

	case ITEM_PENDANT:
		{	
			switch ( value_num )
			{
			default:
				ch->println( "v0 = Recall to vnum" );
				return false;
			case 0:
				{
					value=atoi( argument );
					ch->printlnf( "Recall Vnum changed from %d to %d.", pObj->value[0], value);
					pObj->value[0] =value; 
					ROOM_INDEX_DATA *pr=get_room_index(value);			
					if(pr){
						ch->printlnf( "[%d] is '%s'", value, pr->name);
					}else{
						ch->printlnf( "`=XRoom %d can't be found`x", value );
						ch->println("note: when a room can't be found, recalling is as though the player isn't wearing a pendant.");
					}
				}
				break;
			}
		}
		break;

	case ITEM_TOKEN:
		switch ( value_num )
		{
		default:
			do_help(ch, "OLC-ITEM-TOKEN" );
			return false;
		case 0:
			ch->println( "Token Flag Toggled.\r\n" );
			pObj->value[0] = ( flag_value( token_flags, argument ) != NO_FLAG
				? flag_value( token_flags, argument ) : 0 );
			break;
		}
		break;

	case ITEM_COMPONENT:
		switch ( value_num )
		{
			default:
				do_help(ch, "OLC-ITEM-COMPONENT" );
				return false;

			case 0:
				ch->println( "Charges set.\r\n" );
				pObj->value[0] = atoi( argument );
				break;
			case 1:
				ch->println( "Spell set.\r\n" );
				pObj->value[1] = skill_lookup( argument );
				break;
		}
		break;

	case ITEM_FURNITURE:
	    switch ( value_num )
	    {
	        default:
	            do_help( ch, "OLC-ITEM-FURNITURE" );
	            return false;
	            
	        case 0:
                ch->println( "NUMBER OF PEOPLE SET.\r\n" );
	            pObj->value[0] = atoi ( argument );
	            break;
	        case 1:
                ch->println( "MAX WEIGHT SET.\r\n" );
	            pObj->value[1] = atoi ( argument );
	            break;
	        case 2:
				ch->printlnf( "Valid v2 Furniture flags are: %s", flag_string( furniture_flags, -1));
                ch->println( "FURNITURE FLAGS TOGGLED.\r\n" );
	            pObj->value[2] ^= (flag_value( furniture_flags, argument ) != NO_FLAG
	            ? flag_value( furniture_flags, argument ) : 0);
	            break;
	        case 3:
                ch->println( "HEAL BONUS SET.\r\n" );
	            pObj->value[3] = atoi ( argument );
	            break;
	        case 4:
                ch->println( "MANA BONUS SET.\r\n" );
	            pObj->value[4] = atoi ( argument );
	            break;
	    }
	    break;

	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
		switch ( value_num )
		{
			int value;
		default:
			do_help( ch, "OLC-ITEM-CONTAINER" );
			ch->printlnf( "Valid v1 flags include: %s", flag_string( container_flags, -1));
			return false;
		case 0:
			ch->wraplnf( "Maximum combined weight the container can hold changed "
				"from %d lbs to %d lbs.", 
				pObj->value[0], atoi( argument ));
			pObj->value[0] = atoi( argument );
			ch->wrapln("`YNotes:`x v0 = is the maximum combined weight the container can hold... that is "
				"obtained by adding all objects in the containers true weights together.`1"
				"v3 = is the maximum weight any single object can be to be able to be put inside the container.");			
			break;
		case 1:
			if ( ( value = flag_value( container_flags, argument ) ) != NO_FLAG ){
				TOGGLE_BIT(pObj->value[1], value);
			}else{
				ch->printlnf( "Valid v1 container flags are: %s", flag_string( container_flags, -1));
				do_help ( ch, "OLC-ITEM-CONTAINER" );
				return false;
			}
			ch->println( "CONTAINER FLAG SET.\r\n" );
			break;
		case 2:
			if ( atoi(argument) != 0 )
			{
				if ( !get_obj_index( atoi( argument ) ) )
				{
					ch->println( "THERE IS NO SUCH ITEM.\r\n" );
					return false;
				}
				if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
				{
					ch->println( "THAT ITEM IS NOT A KEY.\r\n" );
					return false;
				}
			}
			ch->println( "CONTAINER KEY SET.\r\n" );
			pObj->value[2] = atoi( argument );
			break;
		case 3:
			ch->wraplnf( "Maximum weight any single object able to be put "
				"into the container changed from %d lbs to %d lbs.", 
				pObj->value[3], atoi( argument ));
			pObj->value[3] = atoi( argument );
			ch->wrapln("`YNotes:`x v0 = is the maximum combined weight the container can hold... that is "
				"obtained by adding all objects in the containers true weights together.`1"
				"v3 = is the maximum weight any single object can be to be able to be put inside the container.");			
			break;
		case 4:
			ch->println( "WEIGHT MULTIPLIER SET.\r\n" );
			pObj->value[4] = atoi ( argument );
			break;
		}
		break;

	case ITEM_DRINK_CON:
		switch ( value_num )
		{
		default:
			do_help( ch, "OLC-ITEM-DRINK" );
			return false;
		case 0:
			ch->println( "MAXIMUM AMOUT OF LIQUID HOURS SET.\r\n" );
			pObj->value[0] = atoi( argument );
			break;
		case 1:
			ch->println( "CURRENT AMOUNT OF LIQUID HOURS SET.\r\n" );
			pObj->value[1] = atoi( argument );
			break;
		case 2:
			ch->println( "LIQUID TYPE SET.\r\n" );
			pObj->value[2] = ( liq_lookup(argument) != -1 ?
				liq_lookup(argument) : 0 );
			break;
		case 3:
			ch->println( "POISON VALUE TOGGLED.\r\n" );
			pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
			break;
		}
		break;

	case ITEM_FOUNTAIN:
		switch (value_num)
		{
		default:
			do_help( ch, "OLC-ITEM-FOUNTAIN" );
			return false;
		case 0:
			ch->println( "MAXIMUM AMOUT OF LIQUID HOURS SET.\r\n" );
			pObj->value[0] = atoi( argument );
			break;
		case 1:
			ch->println( "CURRENT AMOUNT OF LIQUID HOURS SET.\r\n" );
			pObj->value[1] = atoi( argument );
			break;
		case 2:
			ch->println( "LIQUID TYPE SET.\r\n" );
			pObj->value[2] = ( liq_lookup( argument ) != -1 ?
				liq_lookup( argument ) : 0 );
			break;
 		case 3:
 			ch->println( "POISON VALUE TOGGLED.\r\n" );
 			pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
 			break;
        }
		break;

	case ITEM_FOOD:
		switch ( value_num )
		{
		default:
			do_help( ch, "OLC-ITEM-FOOD" );
			return false;
		case 0:
			ch->println( "HOURS OF FOOD SET.\r\n" );
			pObj->value[0] = atoi( argument );
			break;
		case 1:
			ch->println( "HOURS OF FULL SET.\r\n" );
			pObj->value[1] = atoi( argument );
			break;
		case 3:
			ch->println( "POISON VALUE TOGGLED.\r\n" );
			pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
			break;
		}
		break;

	case ITEM_MONEY:
		{
			int v=atoi(argument);
			switch ( value_num )
			{
			default:
				do_help( ch, "OLC-ITEM-MONEY" );
				return false;
			case 0:
				ch->printlnf( "SILVER AMOUNT CHANGED FROM %d TO %d.", pObj->value[0], v);
				pObj->value[0]=v;
				break;
			case 1:
				ch->printlnf( "GOLD AMOUNT CHANGED FROM %d TO %d.", pObj->value[1], v);
				pObj->value[1]=v;
				break;
			}
		}
		break;

	case ITEM_SHEATH:
		int v = atoi(argument);
		switch( value_num )
		{
		default:
			do_help( ch, "OLC-ITEM-SHEATH" );
			return false;
		case 0:
			ch->printlnf( "WEAPON CAPACITY CHANGED FROM %d TO %d.", pObj->value[0], v);
			pObj->value[0] = v;
			break;
		}
	}

    show_obj_values( ch, pObj );

    return true;
}

/**************************************************************************/
bool oedit_show( char_data *ch, char *)
{
    OBJ_INDEX_DATA *pObj;
	MUDPROG_TRIGGER_LIST *list;
    AFFECT_DATA *paf;
    int cnt;

    EDIT_OBJ(ch, pObj);

  	ch->printlnf( "`=rVnum: `x%-5d   `=rType: `x%-10s `=rLevel: "
        "`x%-3d        `=rArea[%d]: `x%s", 
		pObj->vnum, flag_string( item_types, pObj->item_type ) ,   pObj->level,
		pObj->area ? pObj->area->vnum : 0,
		pObj->area ? pObj->area->file_name : "`RNo Area!!!`x");

	ch->printf(   "`=rCost: `x%-7d ",pObj->cost );
	ch->printf(   "`=rWeight: `x%-3.1f lbs ", ((double)pObj->weight)/10);
	ch->printf(   "`=rCondition: `x%-6d ", pObj->condition );
	ch->printlnf( "`=rMaterial: `x%-14s", pObj->material );

	// sizes
	ch->printf(   "`=rAbsolute size: `x%-6d ", pObj->absolute_size );
	ch->printlnf( "`=rRelative size: `x%-6d", pObj->relative_size );
   
	ch->printlnf( "`=rName: `=x%s", pObj->name);
	ch->printlnf( "`=rShort desc: `=x%s`x", pObj->short_descr);
	if (has_colour(pObj->short_descr))
	{
		ch->print( "`sShort desc: `x" );
		ch->printlnbw(pObj->short_descr);
	}

    ch->printlnf( "`=rLong descr: `=x%s`x", pObj->description);
	if (has_colour(pObj->description))
	{
		ch->print( "`sLong descr: `x" );
		ch->printlnbw(pObj->description);
	}

	// Extended descriptions
	show_char_extended(ch, pObj->extra_descr, false);


	mxp_display_olc_flags(ch, wear_flags, pObj->wear_flags,			"wear",	  "Wear Flags:");
	mxp_display_olc_flags(ch, objextra_flags,  pObj->extra_flags,	"extra",  "Extra Flags:");
	mxp_display_olc_flags(ch, objextra2_flags, pObj->extra2_flags,	"extra2", "Extra2 Flags:");

	if ( IS_SET ( pObj->attune_flags, ATTUNE_NEED_TO_USE ))
	{
		mxp_display_olc_flags(ch, attune_flags, pObj->attune_flags,	"attune", "Attune Flags:");
	}
	if ( pObj->ospec_fun )
	{
		ch->printlnf( "`=rSpecial function: `R%s`x",  ospec_name( pObj->ospec_fun ) );
	}

	if(pObj->item_type!=ITEM_PORTAL){
		if(pObj->class_allowances){
			mxp_display_olc_flags(ch, classnames_flags, pObj->class_allowances,	"classallow", "Class Allowances:");
		}else{
			ch->printlnf( "`=r%s: `xall classes`x", mxp_create_send(ch, "classallow", "Class Allowances"));
		}
	}

    for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
    {
		if ( cnt == 0 )	{
            ch->println( "`=rNumber Modifier Affects" );
			ch->println(    "------ -------- -------`x" );
		}

		if ( paf->where == WHERE_SKILLS )
		{
			ch->printf( "[%4d] %-8d Skill - %s",
				cnt, paf->modifier, skill_table[paf->type].name );
		}
		else if ( paf->where == WHERE_OBJECTSPELL )
		{
			ch->printf( "[%4d] ObjectSpell %s - level=%d, duration=%d",
				cnt, skill_table[paf->type].name, paf->level, paf->duration);
		}else{
			if(paf->location==APPLY_NONE){
				ch->printf( "[%4d]     ", cnt);
			}else{
				ch->printf( "[%4d] %8d %s",
					cnt,
					paf->modifier,
					flag_string( apply_types, paf->location ) );
			}
		}
		// say what the 'none' do
		if (paf->bitvector){
			ch->printlnf( "%s", to_affect_string( paf, pObj->level, false ));
		}else{
			ch->println("");
		}
		cnt++;
    }
 	ch->print( "`x" );

	if(pObj->restrict){
		OBJRESTRICT_LIST_DATA *pr;
		ch->println( "`=r========ClassGroups Object Restrictions========" );
		ch->println( "`=r       ClassGroup       Affect Profile          Prority" );
		ch->println( "--------------------    -----------------------  ---------`x" );

		for ( cnt=0, pr = pObj->restrict; pr; pr = pr->next, cnt++)
		{
	        ch->printlnf( "[%4d] %-20.20s %10s  %3d", 
				cnt,
				pr->classgroup->name,
				pr->affectprofile->name,
				pr->priority);
		}
		ch->println( "note: if multiple classgroup matches, only highest priority is applied.`x" );
    }
 	ch->print( "`x" );

	show_obj_values( ch, pObj );

	if ( pObj->obj_triggers )
	{
		int cnt;
		
		ch->printlnf("\r\n`=rMUDPrograms for object [%5d]:", pObj->vnum);

		if(pObj->oprog_flags==0 && pObj->oprog2_flags==0){
			ch->printlnf("Object mudprog triggers: none");
		}else{
			ch->wraplnf("Object mudprog triggers: %s%s%s", 
				pObj->oprog_flags?flag_string(oprog_flags, pObj->oprog_flags):"",
				pObj->oprog_flags?" ":"",
				pObj->oprog2_flags?flag_string(oprog2_flags, pObj->oprog2_flags):"");
		}

		
		for (cnt=0, list=pObj->obj_triggers; list; list=list->next)
		{
			if (cnt ==0)
			{
				ch->println("`=r Number Vnum Trigger Phrase   <Positions>");
				ch->println(" ------ ---- ------- --------------------");
			}		
			ch->printf("`=r[%5d] `=v%s `=V%7s `=x%s ", 
				cnt++,
				mxp_create_tagf(ch, "mprogvnum", "%5d", list->prog->vnum),
				list->trig_type?flag_string(oprog_flags, list->trig_type):flag_string(oprog2_flags, list->trig_type),
				list->trig_phrase);
			if(list->pos_flags){
				ch->printlnf("`=V  <%s>",	flag_string(position_flags,list->pos_flags));
			}else{
				ch->println("`=V  <all possible>");
			}
		}
	}

    return false;
}

/**************************************************************************/
bool oedit_addrestrict( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
	classgroup_type *cg;
	affectprofile_type *ap;
	OBJRESTRICT_LIST_DATA *pr;
    char classgroup[MIL];
    char affectprofile[MIL];
	char strprority[MIL];
	int priority;

    EDIT_OBJ(ch, pObj);
    argument = one_argument( argument, classgroup);
    argument = one_argument( argument, affectprofile);
	one_argument( argument, strprority);
	
    if ( IS_NULLSTR(classgroup) || IS_NULLSTR(affectprofile))
    {
		if(!codehelp(ch,"oedit_addrestrict", CODEHELP_ALL_BUT_PLAYERS)){
			ch->println("Syntax:  addrestrict <classgrouping> <affectprofile> [priority]");
			ch->println("showaffectprofile lists all affect profiles.");
		}
		return false;
    }

	if (pObj->item_type == ITEM_TRASH){
        ch->println("You can't add classgroup restrictions to objects of type trash.");
		return false;
	}

	if(IS_NULLSTR(strprority)){
		priority=-1;
	}else if(!is_number(strprority)){
        ch->println("The priority must be a number between 0 and 100.");
		return false;
	}else{
		priority=atoi(strprority);
	};

	if(priority>100 || priority<-1){
        ch->println("The priority must be a number between 0 and 100.");
		return false;
	}

	// check the parameters
	cg=classgroup_lookup(classgroup);
	if(!cg)
	{
		int index;
		ch->printlnf( "No such classgroup '%s'", classgroup);
		ch->println( "The classgroup must be one of the following:" );
		for(index=0; !IS_NULLSTR(classgroup_table[index].name);index++){
			ch->printlnf( "%-15s - %s",
				classgroup_table[index].name,
				classgroup_table[index].description);
		}
		return false;
	}

	ap=affectprofile_lookup(affectprofile);
	if(!ap)
	{
		int index;
		ch->printlnf( "No such affectprofile %s'", affectprofile);
		ch->println(  "The affectprofile must be one of the following:" );
		for(index=0; !IS_NULLSTR(affectprofile_table[index].name);index++){
			ch->printlnf( "%-15s - %s", 
				affectprofile_table[index].name,
				affectprofile_table[index].description);
		}
		return false;
	}
	pr=new OBJRESTRICT_LIST_DATA;
	pr->affectprofile=ap;
	pr->classgroup=cg;
	pr->priority=priority;
	// add to the linked list
	pr->next=pObj->restrict;
	pObj->restrict=pr;
	// create bit quick lookup
	SET_BIT(pObj->objrestrict,(1<<pObj->restrict->classgroup->bitindex)); 
    ch->printlnf( "Object Classgroup restriction added:\r\n"
		"   classgroup=%s, affectprofile=%s, priority=%d.",
		pr->classgroup->name, pr->affectprofile->name, pr->priority);
    return true;
}
/**************************************************************************/
bool oedit_delrestrict( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
	OBJRESTRICT_LIST_DATA *pr, *pr_next;
    int  value;
    int  cnt = 0;

    EDIT_OBJ(ch, pObj);

	if ( IS_NULLSTR( argument ))
	{
		ch->println( "Syntax:  delrestrict <#affectprofile>" );
		return false;
	}

    if ( !is_number( argument ))
    {
		ch->println( "#affectprofile must be a number." );
		oedit_delrestrict(ch,"");
		return false;
    }

    value = atoi( argument );
    if ( value < 0 )
    {
		ch->println( "Only non-negative restrict-numbers allowed." );
		return false;
    }

	pr=pObj->restrict;
    if ( !pr )
    {
		ch->println( "OEdit: There a no classgroup restrictions on this object." );
		return false;
    }

    if( value == 0 )	// First case: Remove first affect 
    {
		pr = pObj->restrict;
		pObj->restrict = pr->next;
    }
    else		// Affect to remove is not the first 
    {
		while ( ( pr_next = pr->next ) && ( ++cnt < value ) ){
			pr = pr_next;
		}
		
		if( pr_next )		// See if it's the next affect 
		{
			pr->next = pr_next->next;
		}
		else	// Doesn't exist 
		{
			ch->println( "No such restrict." );
			return false;
		}
    }

    ch->printlnf( "Object Classgroup restriction %d removed:\r\n"
		"   classgroup=%s, affectprofile=%s, priority=%d.",
		value, pr->classgroup->name, pr->affectprofile->name, pr->priority);
	delete pr;

	// reset the bitmask
	pObj->objrestrict=0;
	for(pr=pObj->restrict; pr; pr=pr->next){
		SET_BIT(pObj->objrestrict,(1<<pr->classgroup->bitindex));
	}
    return true;
}
/**************************************************************************/
bool oedit_name ( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) )
    {
		ch->println( "Syntax:  name <string>" );
		return false;
    }

	ch->printlnf( "Object name changed from '%s' to '%s'.", pObj->name, lowercase(argument));
    replace_string( pObj->name, lowercase(argument));
   
    return true;
}
/**************************************************************************/
bool oedit_short( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    // trim the spaces to the right of the short
	argument=rtrim_string(argument);

    if ( IS_NULLSTR(argument) )
    {
        ch->println("Syntax:  short <string>" );
		ch->println("Note: <string> is forced to lowercase unless there are colour codes in it.");
		return false;
    }

	if(has_colour(argument)){
		ch->wraplnf( "Changed object short description from '%s' to '%s' "
			"(lower case not forced due to colour codes).",
			pObj->short_descr, argument);
		replace_string(pObj->short_descr, argument);
	}else{
		ch->printlnf( "Changed object short description from '%s' to '%s'.",
			pObj->short_descr, lowercase(argument));
		replace_string(pObj->short_descr, lowercase(argument));
	}
    return true;
}

/**************************************************************************/
bool oedit_long( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    // trim the spaces to the right of the short
	argument=rtrim_string(argument);

    if ( IS_NULLSTR(argument) )
    {
		ch->println( "Syntax: long <string>" );
		ch->println( "  (this is the description of the object seen in the room when you type look.)" );
		ch->println( "  e.g. A large rock is resting here." );
		return false;
	}

	char *ptemp=pObj->description;
	pObj->description = str_dup( argument );
	pObj->description[0] = UPPER( pObj->description[0] );
	ch->printlnf( "Long description from '%s' to '%s'.",
		ptemp, pObj->description);

    free_string( ptemp );

    return true;
}
/**************************************************************************/
bool oedit_nolong( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);   

	if ( IS_NULLSTR(argument) || str_cmp("confirm", argument))
    {
		ch->println( "Syntax:  nolong confirm" );
		ch->println( "  This removes the long descript, allowing for hidden objects in the room" );
		return false;
	}

	// do security checks
	if (!HAS_SECURITY(ch, OEDIT_NOLONG_MINSECURITY))
	{
		ch->printlnf( "You must have an olc security %d or higher to use this command.",
			OEDIT_NOLONG_MINSECURITY);
		return false;
	}

    replace_string(pObj->description,"");
    ch->println( "Long description cleared." );
    return true;
}

/**************************************************************************/
bool set_value( char_data *ch, OBJ_INDEX_DATA *pObj, char *argument, int value )
{
    if (IS_NULLSTR(argument)){
		set_obj_values( ch, pObj, -1, "" );
		return false;
    }
    if ( set_obj_values( ch, pObj, value, argument ) ){
		return true;
	}
    return false;
}

/**************************************************************************/


/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The five valueX functions below.
 ****************************************************************************/
bool oedit_values( char_data *ch, char *argument, int value )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( set_value( ch, pObj, argument, value ) ){
        return true;
	}
    return false;
}
/**************************************************************************/
bool oedit_value0( char_data *ch, char *argument )
{
    if ( oedit_values( ch, argument, 0 ) )
        return true;
    return false;
}
/**************************************************************************/
bool oedit_value1( char_data *ch, char *argument )
{
    if ( oedit_values( ch, argument, 1 ) )
        return true;
    return false;
}
/**************************************************************************/
bool oedit_value2( char_data *ch, char *argument )
{
    if ( oedit_values( ch, argument, 2 ) )
        return true;

    return false;
}
/**************************************************************************/
bool oedit_value3( char_data *ch, char *argument )
{
    if ( oedit_values( ch, argument, 3 ) )
        return true;
    return false;
}
/**************************************************************************/
bool oedit_value4( char_data *ch, char *argument )
{
    if ( oedit_values( ch, argument, 4 ) )
        return true;
    return false;
}

/**************************************************************************/
bool oedit_weight( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

	if ( IS_NULLSTR(argument) || !is_number( argument ) )
	{
		ch->println( "Syntax:  weight <number>" );
		ch->println( "Where the number is in 10th of a pound.");
		ch->println( "e.g. 8=0.8lbs, 20=2lbs.");
		return false;
	}

	int value=atoi( argument);
	
	if(value<0){
		ch->println("oedit_weight(): Weight must be 0 or greater.");
		oedit_weight(ch,"");
		return false;
	}

	ch->printlnf( "Weight changed from %0.1f lbs to %0.1f lbs.", 
		((double)pObj->weight/10), ((double)value)/10);
	pObj->weight = value;
	return true;
}

/**************************************************************************/
bool oedit_cost( char_data *ch, char *argument )
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if ( IS_NULLSTR(argument) || !is_number( argument ) )
	{
		ch->println( "Syntax:  cost <number>" );
		return false;
	}

	int value=atoi( argument);
	
	if(value<0){
		ch->println("oedit_cost(): Cost must be 0 or greater.");
		oedit_cost(ch,"");
		return false;
	}

	ch->printlnf( "Cost changed from %d to %d.", pObj->cost, value);
	pObj->cost = value;
    return true;
}


/**************************************************************************/
bool oedit_create( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
		ch->println( "Syntax:  oedit create <vnum>" );
		return false;
	}

	pArea = get_vnum_area( value );

	if ( !pArea )
	{
		ch->println( "OEdit: That vnum is not assigned an area." );
		return false;
    }

    if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_OBJECTS) )
    {
		ch->println( "OEdit: Vnum in an area you cannot create objects in." );
		return false;
    }

    if ( get_obj_index( value ) )
    {
		ch->println( "OEdit:  Object vnum already exists." );
		return false;
	}

	pObj					= new_obj_index();
	pObj->vnum				= value;
	pObj->area				= pArea;
	pObj->relative_size		= 50;		// main size so all can wear it
        
	if ( value > top_vnum_obj ){
		top_vnum_obj		= value;
	}

	iHash					= value % MAX_KEY_HASH;
    pObj->next				= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;
    ch->desc->pEdit			= (void *)pObj;

	{ // add vnums to new object default descriptions
		char buf[MIL];
		sprintf(buf, "no name %d", value);  
		replace_string(pObj->name, buf);
		sprintf(buf, "(no short description %d)", value);  
		replace_string(pObj->short_descr, buf);
		sprintf(buf, "(no description %d)", value);  
		replace_string(pObj->description, buf);
	}

    ch->printlnf( "Object %d Created.", value );
    return true;
}

/**************************************************************************/
bool oedit_ed( char_data *ch, char *argument ) 
{
	return( generic_ed (ch, argument) );
}

/**************************************************************************/
bool oedit_extra( char_data *ch, char *argument )
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olc_generic_flag_toggle(ch, argument, "extra", "extra", objextra_flags, &pObj->extra_flags);
}
/**************************************************************************/
bool oedit_extra2( char_data *ch, char *argument )
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olc_generic_flag_toggle(ch, argument, "extra2", "extra2", objextra2_flags, &pObj->extra2_flags);
}

/**************************************************************************/
bool oedit_wear( char_data *ch, char *argument )
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olc_generic_flag_toggle(ch, argument, "wear", "wear", wear_flags, &pObj->wear_flags);
}

/**************************************************************************/
bool oedit_type( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
    int value;

    if ( !IS_NULLSTR(argument))
    {

		if ( ( value = flag_value( item_types, argument ) ) != NO_FLAG )
		{
			if(pObj->item_type == value){
				ch->printlnf( "Object is already type '%s'",  
					flag_string( item_types, pObj->item_type ));
			
				return false;
			}
	        ch->printlnf( "Type changed from '%s' to '%s'.",  
				flag_string( item_types, pObj->item_type ),
				flag_string( item_types, value ));
		    pObj->item_type = value;

		    // Reset the default values.
			REMOVE_BIT(pObj->wear_flags, OBJWEAR_PENDANT);
			switch (pObj->item_type)
			{
			case ITEM_SCROLL:
			case ITEM_POTION:
			case ITEM_PILL:
				pObj->value[0] = -1; // set them to none
				pObj->value[1] = -1;
				pObj->value[2] = -1;
				pObj->value[3] = -1;
				pObj->value[4] = -1;
				break;
			case ITEM_WAND:
			case ITEM_STAFF:
				pObj->value[0] = 0;
				pObj->value[1] = 0;
				pObj->value[2] = 0;
				pObj->value[3] = -1;
				pObj->value[4] = 0;
				break;
			case ITEM_PENDANT:
				pObj->value[0] = 0;
				pObj->value[1] = 0;
				pObj->value[2] = 0;
				pObj->value[3] = 0;
				pObj->value[4] = 0;
				SET_BIT(pObj->wear_flags, OBJWEAR_TAKE);
				SET_BIT(pObj->wear_flags, OBJWEAR_PENDANT);
				break;

			default:
				pObj->value[0] = 0;
				pObj->value[1] = 0;
				pObj->value[2] = 0;
				pObj->value[3] = 0;
				pObj->value[4] = 0;
				break;
			}

			return true;
		}
	}

	show_olc_options(ch, item_types, "type", "type", pObj->item_type);
	return false;
}

/**************************************************************************/
bool oedit_material( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  material <string>" );
		return false;
    }

	replace_string( pObj->material, lowercase(argument));

    ch->printlnf( "Material set to '%s'.", pObj->material );
    return true;
}

/**************************************************************************/
bool oedit_level( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *paf;
	int value;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
		ch->println( "Syntax:  level <number>" );
		return false;
    }

	value=atoi( argument );
    if( pObj->level == value){
		ch->println("Level unchanged.");
	};

    ch->printlnf( "Object level changed from %d to %d.", 
		pObj->level , value);

    for ( paf = pObj->affected; paf; paf = paf->next )
	{
		paf->level=value;
	}
	pObj->level = value;
    return true;
}



/**************************************************************************/
bool oedit_condition( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( !IS_NULLSTR(argument)
    && ( value = atoi (argument ) ) >= 1
    && ( value <= 100 ) )
    {
		EDIT_OBJ( ch, pObj );

		ch->printlnf( "Condition changed from %d to %d.", 
			pObj->condition, value);
		pObj->condition = value;
		return true;
    }

    ch->println("Syntax:  condition <number>");
	ch->println("Where number can range from 1 (ruined) to 100 (perfect).");
    return false;
}

/**************************************************************************/
bool oedit_classallowances( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
    int value;

    if ( IS_NULLSTR(argument))
    {
		show_olc_options(ch, classnames_flags, "classallow", "classallowance", pObj->class_allowances);
		ch->println( "note: If no class allowances are set, then all classes can use the object." );
		ch->println( "If class allowances are set, then only the classes on the list can use the object." );
		return false;
	}

	if ( ( value = flag_value( classnames_flags, argument ) ) != NO_FLAG )
	{
		TOGGLE_BIT(pObj->class_allowances, value);
		
		ch->println( "Classallowances flag toggled." );
		return true;
	}

	ch->printlnf("Unrecognised classname '%s'", argument);
    return false;
}

/**************************************************************************/
bool oedit_asize( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
	int value;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        ch->println( "Syntax:  asize <number>" );
        ch->println( "(note at this stage absolute size has no effect)" );
		return false;
    }

	value = atoi( argument );
    pObj->absolute_size = value;

	ch->printlnf( "Absolute size set to %d.", value );
    ch->println( "(note at this stage absolute size has no effect)" );
    return true;
}

/**************************************************************************/
bool oedit_rsize( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
	int value, race;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) || !is_number( argument ) ){
		ch->println( "Syntax:  rsize <number>" );
		ch->wrapln( "Every object has a relative size value, if this value "
			"is smaller than a races lowsize, the object will be too small for that "
			"race to wear (and the reverse for a races high size).  "
			"The default convention for an object that is wearable by "
			"all races is to set the relative size to 50.");
		return false;
    }

	value = atoi( argument );
	if (value>100 || value<10)
	{
	    ch->println( "Try a value closer to 50." );
		return false;
	}

	{
		char race_wear_list[MSL];
		char race_nowear_list[MSL];
		
		race_wear_list[0]='\0';
		race_nowear_list[0]='\0';
		for ( race = 1; race_table[race]; race++ )
		{
			if (value>=race_table[race]->low_size
				&& value<=race_table[race]->high_size)
			{
				strcat(race_wear_list,race_table[race]->name);
				strcat(race_wear_list,"\r\n");
			}else{
				strcat(race_nowear_list,race_table[race]->name);
				if(value<race_table[race]->low_size){
					strcat(race_nowear_list," - object is too small.\r\n");
				}else{
					strcat(race_nowear_list," - object is too big.\r\n");
				}				
			}
		}

		pObj->relative_size = value;
		ch->printlnf( "Relative size set to %d.", value);

		if (IS_NULLSTR(race_wear_list))
		{
			ch->println( "Warning, no player races can wear it with that relative size value." );
		}	
		else
		{
			ch->printlnf( "`YPlayer races that `Gcan`Y wear it:`x%s", 
				race_wear_list );
			ch->printlnf( "`YPlayer races that `Rcan't`Y wear it:`x%s",
				race_nowear_list );
		}
	}
    return true;
}
/**************************************************************************/
// duplicate the object data from another 
// written by Kalahn - June 98
bool oedit_copy(char_data *ch, char *argument)
{
	OBJ_INDEX_DATA *pObj;
	OBJ_INDEX_DATA *pSrc; // source object
	char arg1[MIL];
	int value;

    argument = one_argument( argument, arg1 );

    if ( !is_number( arg1 ) )
    {
        ch->println( "Syntax: ocopy <source object vnum>" );
        ch->println( "  - copies the source object over the object you are currently editing!" );
        ch->println( "    (warning copies over everything!)" );
        return false;
	}

    value = atoi( arg1 );
    if ( !( pSrc = get_obj_index( value ) ) )
    {
        ch->println( "OEdit_copy:  The source vnum does not exist." );
        return false;
    }
    
    if ( !IS_BUILDER( ch, pSrc->area, BUILDRESTRICT_OBJECTS) && !IS_IMMORTAL(ch) )
    {
        ch->println( "Insufficient security to copy from the area that object\r\n"
            "is stored in and your aren't an immortal." );
        return false;
    }
    
	EDIT_OBJ(ch, pObj);

	// copy the object details
	pObj->name			= str_dup(pSrc->name);
	pObj->short_descr	= str_dup(pSrc->short_descr);
	pObj->description	= str_dup(pSrc->description);
	pObj->material		= str_dup(pSrc->material);

	pObj->item_type		= pSrc->item_type;

	pObj->extra_flags	= pSrc->extra_flags;
        pObj->extra2_flags      = pSrc->extra2_flags;
	pObj->wear_flags	= pSrc->wear_flags;
		
	pObj->level			= pSrc->level;
	pObj->condition		= pSrc->condition;
	pObj->count			= pSrc->count;
	pObj->weight		= pSrc->weight;
	pObj->cost			= pSrc->cost;
	pObj->value[0]		= pSrc->value[0];
	pObj->value[1]		= pSrc->value[1];
	pObj->value[2]		= pSrc->value[2];
	pObj->value[3]		= pSrc->value[3];
	pObj->value[4]		= pSrc->value[4];
	pObj->value[4]		= pSrc->value[4];
	pObj->absolute_size	= pSrc->absolute_size;
	pObj->class_allowances = pSrc->class_allowances;


	// NOTE: we aren't deallocating the existing affects and 
	//	descriptions on the objects... we should, but I can't be
	//	bothered writing the code to do so right now

	// COPY AFFECTS 
	pObj->affected =dup_affects_list(pSrc->affected);

	// COPY THE EXTENDED DESCRIPTIONS 
	pObj->extra_descr	= dup_extdescr_list(pSrc->extra_descr);

	// copy mudprogs list
	pObj->obj_triggers	= dup_mudprog_list(pSrc->obj_triggers);
	pObj->oprog_flags 	= pSrc->oprog_flags;
	pObj->oprog2_flags 	= pSrc->oprog2_flags;


//	sh_int         vnum;
//	sh_int         reset_num;
//	AREA_DATA *    area;           

	ch->wraplnf( "`=rCopied object '%s'[%d] to vnum %d", 
				pSrc->short_descr, pSrc->vnum, pObj->vnum);
    return true;
}
/**************************************************************************/
bool oedit_attune( char_data *ch, char *argument )
{
    int value;
    OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if( !IS_IMMORTAL( ch )) {
		ch->println( "oedit_attune(): Currently only for immortal testing." );
		return false;
	}

    if( !IS_NULLSTR(argument))
    {
		
		if ( ( value = flag_value( attune_flags, argument ) ) != NO_FLAG )
		{
			TOGGLE_BIT(pObj->attune_flags, value);
			ch->println( "Attune flag toggled." );
			return true;
		}
    }
	
	show_olc_options(ch, attune_flags, "attune", "attune", pObj->attune_flags);
    return false;
}
/**************************************************************************/
bool oedit_addskill( char_data *ch, char *argument )
{
    int value;
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ(ch, pObj);
    AFFECT_DATA *pAf;
    char sn[MSL];
    char mod[MSL];
	int modifier;

	if( !IS_IMMORTAL( ch )) {
		ch->println( "oedit_addskill(): Currently only for immortal testing." );
		return false;
	}


    argument = one_argument( argument, sn );
    one_argument( argument, mod );

    if ( IS_NULLSTR(sn) || IS_NULLSTR(mod) ){
		ch->println( "Syntax:  addskill <skillname> <modifier>" );
		return false;
    }

	if(!is_number( mod ) ){
		ch->println( "Modifier must be a number." );
		oedit_addskill(ch,"");
		return false;
	}

	modifier=atoi(mod);
	if (  modifier< 0 || modifier> 50 )
	{
		ch->println("Value must range from 0 to 50.");
		return false;
	}

	if (pObj->item_type == ITEM_TRASH){
        ch->println( "You can't add a skill modifiers to objects of type trash." );
		return false;
	}

	if (pObj->item_type == ITEM_RP){
        ch->println( "You can't add a skill modifiers to RP objects." );
		return false;
	}

    if (( value = skill_lookup( sn )) == -1 )
    {
        ch->println( "That's not a valid skill." );
		return false;
    }

    pAf             =   new_affect();
    pAf->location   =   APPLY_NONE;
    pAf->modifier   =   modifier;
    pAf->where	    =   WHERE_SKILLS;
    pAf->type       =   value;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    ch->printlnf( "Skill modifier '%s' by %d added.", 
		skill_table[value].name, modifier);
    return true;
}

/**************************************************************************/
bool oedit_addspell( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;

	char name[MIL];
	char level[MIL];
	char duration[MIL];
	char use_spell_for_all_levels[MIL];

    argument = one_argument( argument, name);
    argument = one_argument( argument, level);
    argument = one_argument( argument, duration);
	argument = one_argument( argument, use_spell_for_all_levels);
	
    if ( IS_NULLSTR(name))
    {
		ch->println("Syntax: addspell 'spellname' [level] [duration] [use_spell_for_all_levels]");
		ch->println("Notes: If level is 0, object level is used (default).");
		ch->println("       If duration is 0, spell duration is used.");
		ch->println("       If duration is -1, spell is permanent while object is worn (default).");
		ch->println("       If use_spell_for_all_levels is 1, then the spell will take effect ");
		ch->println("       even if the spell/object is over the players level.");
		return false;
    }

	// check the spell name is valid
	int sn=skill_lookup(name);
	if(sn<FIRST_SPELL || sn>LAST_SPELL){
		ch->printf("oedit_addspell: Couldn't find the spell '%s' to add to object.\r\n",
			name);
		oedit_addspell(ch,"");
		return false;
	}

	// get/check the numeric values
	int ilevel=0;
	int iduration=-1;
	if(!IS_NULLSTR(level)){
		if(is_number(level)){
			ilevel=atoi(level);
		}else{
			ch->println("oedit_addspell: level must be numeric if specified.");
			oedit_addspell(ch,"");
			return false;
		}

		if(!IS_NULLSTR(duration)){
			if(is_number(duration)){
				iduration=atoi(duration);
			}else{
				ch->println("oedit_addspell: duration must be numeric if specified.");
				oedit_addspell(ch,"");
				return false;
			}
		}

	}

    EDIT_OBJ(ch, pObj);
	if (pObj->item_type == ITEM_TRASH)
	{
        ch->println( "You can't add spells to objects of type trash." );
		return false;
	}

	if (pObj->item_type == ITEM_RP)
	{
        ch->println( "You can't add spells to objects of RP items." );
		return false;
	}

    pAf             =   new_affect();
    pAf->location   =   APPLY_NONE;
    pAf->modifier   =   0;
    pAf->where	    =   WHERE_OBJECTSPELL;
    pAf->type       =   sn;
    pAf->duration   =   iduration;
	if(!str_cmp(use_spell_for_all_levels,"1")){
		pAf->bitvector  =   OBJSPELL_IGNORE_LEVEL;
	}else{
		pAf->bitvector  =   0;
	}
    pAf->level      =	ilevel;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

	ch->printlnf("ObjectSpell '%s' added.", skill_table[sn].name);
    return true;
}

/**************************************************************************/
bool oedit_delaffect( char_data *ch, char *argument )
{
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	AFFECT_DATA *pAf_next;
	char affect[MSL];
	int  value;
	int  cnt = 0;

	EDIT_OBJ(ch, pObj);

	one_argument( argument, affect );

	if ( !is_number( affect ) || affect[0] == '\0' )
	{
		ch->println( "Syntax:  delaffect <#xaffect>" );
		ch->println( "Syntax:  delflag <#xaffect>" );
		ch->println( "Syntax:  delmodifier <#xaffect>" );
		return false;
	}

	value = atoi( affect );

	if ( value < 0 )
	{
		ch->println( "Only non-negative affect-numbers allowed." );
		return false;
	}

	if ( !( pAf = pObj->affected ) )
	{
		ch->println( "OEdit:  Non-existant affect." );
		return false;
	}

	if( value == 0 )	// First case: Remove first affect 
	{
		pAf = pObj->affected;
		pObj->affected = pAf->next;
		free_affect( pAf );
	}
	else		// Affect to remove is not the first 
	{
		while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
			pAf = pAf_next;

		if( pAf_next )		// See if it's the next affect 
		{
			pAf->next = pAf_next->next;
			free_affect( pAf_next );
		}
		else                                 // Doesn't exist 
		{
			ch->println( "No such affect." );
			return false;
		}
	}

	ch->println( "Affect removed." );
	return true;
}

/**************************************************************************/
bool oedit_addmodifier( char_data *ch, char *argument )
{
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char location[MSL];
    char amount[MSL];

    EDIT_OBJ(ch, pObj);
    argument = one_argument( argument, location );
    one_argument( argument, amount );

    if ( IS_NULLSTR(location) || IS_NULLSTR(amount))
    {
		ch->println( "Syntax:  addmodifer <location> <amount>" );
		ch->println( "Where <location> is one of:");
		show_olc_flags_types(ch, apply_types);
		ch->println( "(<amount> is a number)");

		if (pObj->item_type == ITEM_TRASH){
			ch->println( "You can't add modifiers to objects of type trash." );
		}
		if (pObj->item_type == ITEM_RP){
			ch->println( "You can't add modifiers to objects of RP items." );
		}
		return false;
    }

	if(!is_number( amount ) ){
		ch->println( "<amount> must be a number." );
		oedit_addmodifier(ch,"");
		return false;
	}

	if (pObj->item_type == ITEM_TRASH){
        ch->println( "You can't add modifiers to objects of type trash." );
		return false;
	}

	if (pObj->item_type == ITEM_RP){
        ch->println( "You can't add modifiers to objects of RP items." );
		return false;
	}

    if ( ( value = flag_value( apply_types, location ) ) == NO_FLAG )
    {
		ch->println("Valid modifiers to apply include:");
		show_olc_flags_types(ch, apply_types);		
		return false;
    }

    pAf             =   new_affect();
    pAf->location   =   (APPLOC)value;
    pAf->modifier   =   atoi( amount);
    pAf->where	    =   WHERE_MODIFIER;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    ch->printlnf( "Modifing affect '%s' added to object... (use delmodifier to remove modifying affects).", 
		to_affect_string(pAf, pObj->level, false) );
    return true;
}
/**************************************************************************/
// - Kal
bool oedit_addflag( char_data *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char where[MSL];
    char flag[MSL];
	int wherevalue, flagvalue;

    EDIT_OBJ(ch, pObj);
	
    argument = one_argument( argument, where );
    argument = one_argument( argument, flag);

	if(IS_NULLSTR(where)){
		ch->println( "Syntax: addflag <where> <flag>" ); // flag sets the bitvector
		ch->println( "e.g. addflag affects swim" );
    	
		ch->println( "Valid <where> places to apply to include:" );
		show_olc_flags_types(ch, to_types);        
		ch->println( "The <flag>'s available depend on <where> the flag is being applied." );

		if (pObj->item_type == ITEM_TRASH){
			ch->println( "You can't add applies to objects of type trash." );
		}
		if (pObj->item_type == ITEM_RP){
			ch->println( "You can't add applies to RP objects." );
		}
		return false;
	}

	if (pObj->item_type == ITEM_TRASH){
        ch->println( "You can't add applies to objects of type trash." );
		return false;
	}

	if (pObj->item_type == ITEM_RP){
        ch->println( "You can't add applies to RP objects." );
		return false;
	}

    if ( (wherevalue= flag_value( to_types, where)) == NO_FLAG ){
    	ch->println( "Invalid <where> location to apply a flag to... valid places <where> you can apply a flag include:" );
		show_olc_flags_types(ch, to_types);
    	return false;
    }
	const flag_type *bv_flags=affect_get_bitvector_table_for_where(wherevalue);

	if ( !bv_flags ){
		ch->wraplnf( "oedit_addflag(): coding bug - please report: For some reason "
			"affect_get_bitvector_table_for_where() returned NULL for a wherevalue of %d, "
			"The most likely cause of this is a new character affect location has been "
			"added but not added to affect_get_bitvector_table_for_where() or an affect "
			"of another type has been added to to_types[], and should be flagged as false. "
			"Until this is fixed, you cant add flags with a <where> of '%s'"
			, wherevalue, where);
		return false;
	}

    if ( IS_NULLSTR(flag) ){
		oedit_addflag(ch,"");
        ch->printlnf( "Valid flags when applying '%s' include:", where);
		show_olc_flags_types(ch, bv_flags);		
		return false;
    }

	flagvalue=flag_value( bv_flags, flag);
    if ( flagvalue== NO_FLAG )
    {
		ch->printlnf( "Invalid flag type '%s'.", flag);
        ch->printlnf( "Valid flags when applying '%s' include:", where);
		show_olc_flags_types(ch, bv_flags);		
		return false;
	}

    pAf             =   new_affect();
    pAf->location   =   APPLY_NONE;
    pAf->modifier   =   0;
	pAf->where      =   wherevalue;
    pAf->type	    =	-1;
    pAf->duration   =   -1;
    pAf->bitvector  =   flagvalue;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    ch->printlnf( "Flag '%s' added... (use delflag to remove flagged affects)", 
		to_affect_string(pAf, pObj->level, false));
    return true;
}
/**************************************************************************/
bool oedit_delete( char_data *ch, char *)
{
	ch->println("If you want to delete an object, use the 'odelete' command");
	return false;
}
/**************************************************************************/
// Kal, Feb 2001
bool oedit_odelete(char_data *ch, char *argument)
{
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ(ch, pObj);

    if (IS_NULLSTR(argument))
    {
		ch->titlebar("ODELETE SYNTAX");
        ch->println("Syntax: odelete confirm - delete the current object");
        ch->println("Syntax: odelete <number> confirm - delete object vnum <number>");
		ch->println("Any object that you delete must meet the following conditions:");
		ch->println("* Must not be used by any reset in any room.");
		ch->println("* Must not be currently loaded in the game.");
		ch->println("* Gameedit can't be making use of the object.");
		ch->println("* You must have sufficient security to edit that object.");
		ch->println("* No one else can currently be editing the object.");
        ch->wrapln("NOTE: It is strongly recommended that no mudprogs attempt to load the object "
			"you are considering deleting... the easiest method to do this is 'textsearch mudprog <objvnum>'.");
		return false;
    }

	// support specifying the object by vnum
	char arg1[MIL];
	OBJ_INDEX_DATA *pDeleteObj;
	argument=one_argument(argument, arg1);
	if(is_number(arg1)){
		pDeleteObj=get_obj_index(atoi(arg1));
		if(!pDeleteObj){
			ch->printlnf("oedit_odelete(): There is no object number %s to delete.", arg1);
			return false;
		}
		argument=one_argument(argument, arg1); // put the word 'confirm' into arg1
	}else{
		pDeleteObj=pObj; // deleting the object we are currently editing
	}

	// security check
	if ( !IS_BUILDER( ch, pDeleteObj->area, BUILDRESTRICT_OBJECTS ) )
    {
        ch->printlnf("OEdit: Insufficient security to delete object %d.", pDeleteObj->vnum);
        return false;
    }

	// confirm they are using 'confirm'
	if(str_cmp(arg1, "confirm")){
		ch->println("You must confirm your intention to delete an object.");
		oedit_odelete(ch,"");
		return false;
	}

	if(!IS_NULLSTR(ltrim_string(argument))){
		ch->println("Incorrect syntax - too many arguments, or arguments in wrong order.");
		oedit_odelete(ch, "");
		return false;
	}

	int v=pDeleteObj->vnum;
	// We have the object they are wanting to delete and they have 
	// confirmed they want to delete it, check if it isn't in use
	{
		int in_use=0;
		// objects in game
		for ( obj_data *inuse_obj= object_list; inuse_obj; inuse_obj= inuse_obj->next ){
			if(inuse_obj->pIndexData==pDeleteObj){
				ch->printlnf("`=rOne or more objects based on object template %d are currently in the game... use owhere <vnum> to find them.`x", v);
				in_use++;
				break;
			}
		}

		// resets using this object
		for( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
		{
			for( ROOM_INDEX_DATA *pRoom= room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
			{
				for ( RESET_DATA *pReset = pRoom->reset_first; pReset; pReset = pReset->next )
				{
					if( pReset->arg1==v  && 
						(pReset->command=='P' 
						|| pReset->command=='O' 
						|| pReset->command=='G' 
						|| pReset->command=='E' 
						) )
					{
						ch->printlnf("object %d reset in room %d (%s).", 
							v, pRoom->vnum, pRoom->name);
						in_use++;
					}
				}
			}
		}

		for(int i=0; !IS_NULLSTR(gameset_value[i].name); i++){
			if(gameset_value[i].category!=GSVC_OBJECT){
				continue;
			}
			// get our numeric value
			int value=GSINT(gameset_value[i].offset);

			if(value==v){
				ch->printlnf("Game setting value '%s (%s)' makes use of object %d.", 
					gameset_value[i].name, gameset_value[i].description, value);
				in_use++;
			}

		}

		// someone else currently editing the mob
		for(connection_data *c=connection_list; c; c=c->next){
			if(c!=ch->desc && c->pEdit==(void *)pDeleteObj){
				ch->println("Someone else is currently editing it, so it can't currently be deleted.");
				in_use++;
			}
		}

		if(in_use){
			ch->println("You can't delete this object, it is currently in use.");
			oedit_odelete(ch, "");
			return false;
		}
	}

	if(pObj==pDeleteObj){
		edit_done(ch);
	}
	ch->printlnf("Deleting object %d.", v);
	
	// remove object from hash table
	{
		int i=v% MAX_KEY_HASH;
		// check if we are the first entry in the hash table
		if(pDeleteObj== obj_index_hash[i]){
			obj_index_hash[i]=obj_index_hash[i]->next;
		}else{
			OBJ_INDEX_DATA *prev=obj_index_hash[i];
			if(!prev){
				bugf("oedit_odelete(): Trying to free object vnum %d, but not found in obj_index_hash[%d]!", 
					v, i);
			}else{
				for( ; prev->next; prev=prev->next )
				{
					if(prev->next==pDeleteObj){
						prev->next=pDeleteObj->next; // remove the object from the link
						break;
					}
				}
			}
		}
	}
	free_obj_index(pDeleteObj);
	top_obj_index--;

	ch->printlnf("Object %d Deleted.",v);
	
	return true;
}
/**************************************************************************/
bool oedit_addmprog(char_data *ch, char *argument)
{
	int value;
    OBJ_INDEX_DATA *pObj;
	MUDPROG_TRIGGER_LIST *list;
	MUDPROG_CODE *prog;
	char trigger[MSL];
	char phrase[MSL];
	char num[MSL];
	vn_int mpnum;
	
    EDIT_OBJ(ch, pObj);
	argument=one_argument(argument, num);
	argument=one_argument(argument, trigger);
	strcpy(phrase, trim_string(argument));
		
	if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' ){
        ch->println("`xSyntax:   addmprog <vnum> <trigger> <phrase>");
		ch->println("Where <trigger> is one of:");
		ch->wraplnf("[`=R%s`x]", flag_string( oprog_flags, -1));
		ch->println("Try setting the phrase to 100, if you want it to trigger 100% of the time.");
        return false;
	}
	
	mpnum=atoi(num);

	if ( ( prog =get_mprog_index (mpnum ) ) == NULL)
	{
        ch->printlnf("No such MUDProgram %d.", mpnum);
        return false;
	}

    if(pObj->area!= get_vnum_area( mpnum )){
		if(HAS_SECURITY(ch,9)){
			ch->println("`RWARNING: That mudprog number belongs to a different area!`x");
		}else{
			ch->println("`RThat mudprog number belongs to a different area!`x");
			ch->println("`ROnly security 9 can link in mudprogs from different areas.`x");
			return false;
		}
	};
	
	if ( (value = flag_value (oprog_flags, trigger) ) == NO_FLAG)
	{
		ch->printlnf("Valid mudprog trigger flags are: %s", 
			flag_string( oprog_flags, -1));
        return false;
	}
	
	list                  = new_mprog();
	list->trig_type       = value;
	list->trig_phrase     = str_dup(phrase);
	list->prog            = prog;
	list->next            = pObj->obj_triggers;
	pObj->obj_triggers          = list;
	
	// recalc the trigger bits
	SET_BIT(pObj->oprog_flags, value);
    ch->printlnf("Mudprog %d '%s' added", mpnum, prog->title);
	ch->printlnf("Objects '%s' triggering is enabled.", flag_string(oprog_flags, list->trig_type));
    ch->printlnf("NOTE: This change affects all objects based on the "
		"object template %d instantly.", pObj->vnum);

	return true;
}
/**************************************************************************/
bool oedit_delmprog(char_data *ch, char *argument)
{
    OBJ_INDEX_DATA *pObj;
	
    MUDPROG_TRIGGER_LIST *list;
    MUDPROG_TRIGGER_LIST *list_next;
    char mprog[MIL];
    int value;
    int cnt = 0;
	long trigtype=0;
	
    EDIT_OBJ(ch, pObj);
	
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
	
    if ( !(list= pObj->obj_triggers) )
    {
        ch->printlnf("OEdit: There are no mudprogs on object template %d.", pObj->vnum);
        return false;
    }
	
    if ( value == 0 )
    {
        list = pObj->obj_triggers;
		trigtype=list->trig_type;
        pObj->obj_triggers = list->next;
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
			ch->printlnf("No such mprog %d.", value);
			return false;
        }
    }

	// recalc the trigger bits
	pObj->oprog_flags=0;
	pObj->oprog2_flags=0;
	for(list=pObj->obj_triggers; list; list=list->next){
		SET_BIT(pObj->oprog_flags, list->trig_type);
		SET_BIT(pObj->oprog2_flags, list->trig2_type);
	}

    ch->printlnf("Mprog %d removed", value);
    ch->printf("NOTE: This change affects all objects based on the "
		"object template %d instantly.\r\n", pObj->vnum);
    return true;
}

/**************************************************************************/
/**************************************************************************/

