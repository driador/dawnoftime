/**************************************************************************/
// mixedit.cpp - Olc mix editor, Kerenos
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
#include "db.h"
#include "olc_ex.h"

mix_data *find_mix( char_data *ch, char *arg );
bool mix_create( char_data *ch, char * );
bool mixedit_values( char_data *ch, char *argument, int value );
bool set_mixvalue( char_data *ch, mix_data *mix, char *argument, int value );
bool set_mix_values( char_data *ch, mix_data *mix, int value_num, char *argument);
void show_obj_values( char_data *ch, OBJ_INDEX_DATA *pObj );
void list_mixes( char_data *ch, char *argument);
int  compare_vn_int( vn_int *, vn_int * );
void mix_bsort( vn_int array[], sh_int num[], int size );
void empty_container( OBJ_DATA *container );

/**************************************************************************/
// semilocalized globals
mix_data	*mix_list;
void save_object_values( FILE *fp, OBJ_INDEX_DATA *pObjIndex );
void load_object_values( FILE *fp, int version, OBJ_INDEX_DATA *pObjIndex);
/**************************************************************************/
bool mixedit_show( char_data *ch, char * )
{
	OBJ_INDEX_DATA *oid = NULL;
	mix_data	*mix;
	int i;
	char buf[MIL];
	
    EDIT_MIX( ch, mix );
	
	ch->titlebar("Mixture Data");
  	ch->printlnf( "`=rName: `x%s", mix->name );
	ch->printlnf( "`=rCreated by: `x%s", mix->creator );
	ch->printlnf( "`=rLocked: `x%s", mix->locked ? "YES" : "NO" );
	ch->printlnf( "`=rType: `x%s", flag_string( mixtype_types, mix->type ));
	ch->printlnf( "`=rVessel: `x[%s]", flag_string( item_types, mix->vessel ));
	ch->printlnf( "`=rDifficulty:     `x%s (%d)",
		modifier_table[mix->difficulty].name,
		modifier_table[mix->difficulty].modifier );
	ch->printlnf( "`=rTemplate VNUM: `x%d", mix->vnum_template );

	ch->println("`=rIngredient   Amount Needed`x");
	for ( i=0; i < 5; i++ )
	{
		if (( oid = get_obj_index( mix->ingredients[i] )) == NULL ){
			if(mix->ingredients[i]==0){
				sprintf(buf, "ignored");
			}else{
				sprintf(buf, "(object not found)");
			}
		}else{
			strcpy(buf, oid->short_descr );
		}
		ch->printlnf( "%d. %-5d               %-2d      (%s)",
		i+1,
		mix->ingredients[i],
		mix->ingredients_num[i],
		buf);
	};
	ch->titlebar("Result Data");

	ch->printlnf( "`=rName:  `x%s", mix->rname );
	ch->printlnf( "`=rShort: `x%s", mix->rshort );
	ch->printlnf( "`=rLong:  `x%s", mix->rlong );
	ch->printlnf( "`=rItem Type:  `x[%s]", flag_string( item_types, mix->ritem_type ));
	ch->printlnf( "`=rWear Flags: `x[%s]", flag_string( wear_flags, mix->rwear ));

	if (( oid = get_obj_index( OBJ_VNUM_MUSHROOM )) == NULL )
	{
		return false;
	}

	OBJ_INDEX_DATA show_objindex;
	show_objindex=*oid;
	show_objindex.item_type	= mix->ritem_type;
	show_objindex.value[0]	= mix->rvalue[0];
	show_objindex.value[1]	= mix->rvalue[1];
	show_objindex.value[2]	= mix->rvalue[2];
	show_objindex.value[3]	= mix->rvalue[3];
	show_objindex.value[4]	= mix->rvalue[4];
	show_obj_values( ch, &show_objindex);
	ch->wrapln("note: the level on resulting potions ignores the spell level set here, and is based on mixer level.");

	ch->titlebar("Failed Result Data Extras");
	ch->printlnf( "`=rName:  `x%s", mix->failedrname );
	ch->printlnf( "`=rShort: `x%s", mix->failedrshort );
	ch->printlnf( "`=rLong:  `x%s", mix->failedrlong );
	ch->wrapln("note: the above failed results will be used for the object description "
		"if the herbalist fails in their attempt to mix something.");

	return false;
}

/**************************************************************************/
// read in the result item type and values into mixdata node - Kal
void mixdata_read_rvalues(gio_type *, int, void *data, FILE *fp)
{
	mix_data * mix= (mix_data*)data;
	obj_index_data pObjIndex;
	// do the reading
	char *iname=fread_word( fp );
	pObjIndex.item_type= item_lookup(iname);
	if(pObjIndex.item_type==-1){
		bugf("mixdata_read_rvalues(): Unfound item type '%s', mix entry name='%s'", 
			iname, mix->name);
		do_abort();
	};
	load_object_values( fp, 5, &pObjIndex);	

	// transfer the results
	memcpy(mix->rvalue, pObjIndex.value, sizeof(int)*5);
	mix->ritem_type=pObjIndex.item_type;
}
/**************************************************************************/
// write the result item type and values from mixdata node - Kal
void mixdata_write_rvalues(gio_type *gio_table,int ti,void *data, FILE *fp)
{
	mix_data * mix= (mix_data*)data;
	obj_index_data pObjIndex;

	// transfer the info to be saved
	pObjIndex.item_type=mix->ritem_type;
	memcpy(pObjIndex.value, mix->rvalue, sizeof(int)*5);

	// do the writing
	char *iname=item_name(pObjIndex.item_type);
	if(has_space(iname) || IS_NULLSTR(iname)){
		fprintf(fp, "%s '%s' ",
			gio_table[ti].heading, 
			item_name(pObjIndex.item_type));
	}else{
		fprintf(fp, "%s %s ",
			gio_table[ti].heading, 
			item_name(pObjIndex.item_type));
	}
	save_object_values( fp, &pObjIndex);
}

/**************************************************************************/
// create organization GIO lookup table 
GIO_START(mix_data)
GIO_STRH(name,						"Name         ")
GIO_STRH(creator,					"Creator      ")
GIO_BOOLH(locked,					"Locked       ")
GIO_WFLAGH(type,					"Type         ", mixtype_types)
GIO_INTH(vnum_template,				"Template     ")
GIO_SHINTH(difficulty,				"Difficulty   ")
GIO_INTH(vessel,					"Vessel       ")
GIO_INT_ARRAY(ingredients, 5 )
GIO_SHINT_ARRAY(ingredients_num, 5 )
GIO_STRH(rname,						"Result_Name  ")
GIO_STRH(rshort,					"Result_Short ")
GIO_STRH(rlong,						"Result_Long  ")
GIO_STRH(failedrname,				"FailedResult_Name  ")
GIO_STRH(failedrshort,				"FailedResult_Short ")
GIO_STRH(failedrlong,				"FailedResult_Long  ")
GIO_CUSTOM_WRITEH(rvalue,	"Rtype_values ", mixdata_write_rvalues)
GIO_CUSTOM_READH(rvalue,	"Rtype_values ", mixdata_read_rvalues)

GIO_WFLAGH(rwear,					"Result_Wear  ", wear_flags)
GIO_FINISH_STRDUP_EMPTY

/**************************************************************************/
// loads in the mix database
void load_mix_db(void)
{
	logf("===Loading mix database from %s...", MIX_FILE);
	GIOLOAD_LIST(mix_list, mix_data, MIX_FILE); 
	log_string ("load_mix_db(): finished");
}
/**************************************************************************/
// saves the mix database
void save_mix_db( void)
{
	logf("===save_mix_db(): saving mix database to %s", MIX_FILE);
	GIOSAVE_LIST(mix_list, mix_data, MIX_FILE, true);
}
/**************************************************************************/
void do_savemixdb( char_data *ch, char * )
{
	save_mix_db( );
	logf("do_savemixdb(): manual save of mix list completed to %s,\r\n"
		"check logs for any errors.\r\n", MIX_FILE);
	ch->println( "Mix data saved manually." );
}

/**************************************************************************/
//	Entry Point for mixing
void do_mixedit( char_data *ch, char *argument )
{
	mix_data *mix;

	if ( IS_NPC( ch )){
		ch->println("Players only.");
		return;
	}

	if (   !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_REALM)
		&& !IS_ADMIN(ch))
	{
		ch->println( "Only realm council may use this command." );
		return;
	}

	if (IS_NULLSTR(argument)){
		ch->println( " Syntax: mixedit <mix>         (edits an existing mixture)");
		ch->println( " Syntax: mixedit list <type>   (shows all available mixes of a given type)" ); 
		ch->println( " Syntax: mixedit create <name> (creates a new, null mixture)" );
		return;
	}
	
	char arg[MIL];

	argument=one_argument(argument,arg);

	if(!str_cmp(arg,"create")){
		mix_create( ch, "" );
		return;
	}

	if ( !str_cmp( arg, "list" )){
		list_mixes( ch, argument );
		return;
	}


	// find an existing mixture
	if(IS_NULLSTR(arg)){
		ch->println("You must specify the type of mixtures you want to list.");
		do_mixedit(ch,"");
		return;
	}
	
	mix = find_mix(ch, arg);

	if( !mix ){
		ch->printlnf("There is no mixture named '%s'", arg );
		return;
	};

	if ( mix->locked )
	{
		if (!(   is_exact_name(TRUE_CH(ch)->name, mix->creator )
			|| IS_ADMIN(ch)))
		{
			ch->printlnf( "Only %s or an admin may modify this mixture.", mix->creator );
			return;
		}
	}
    ch->desc->pEdit	= (void*)mix;
	ch->desc->editor = ED_MIX;
	ch->printlnf("Editing '%s' mixture.", mix->name);
	return;
}

/**************************************************************************/
// find an existing mixture
mix_data *find_mix( char_data *, char *mix )
{
	mix_data *m;

	// first do an exact match
	for ( m = mix_list; m; m = m->next ){
		if( !str_cmp( mix, m->name ))
		{
			return m;
		}
	}

	// do a prefix mix
	for ( m = mix_list; m; m = m->next ){
		if( !str_prefix( mix, m->name ))
		{
			return m;
		}
	}
	return NULL;
};

/**************************************************************************/
void list_mixes( char_data *ch, char *argument )
{
	mix_data	*m;
	int			col = 0;
	int			type = mixtype_lookup( argument );

	if ( type == -1 )
	{
		ch->printlnf( "'%s' is an invalid mix type. Use 'mixedit list <type>', where type is one of: ", argument );
		show_help( ch, "mixtype" );
		ch->println("e.g.: mixedit list herbalism");
		return;
	}
	
	ch->titlebar("Mixes");

	for( m=mix_list; m; m = m->next )
	{
		if ( type == m->type )
		{
			ch->printf( " %-20s", m->name );
			if ( ++col % 4 == 0 )
				ch->print( "\r\n" );
		}
	}
	ch->print("`x\r\n" );
	return;
}

/**************************************************************************/
bool mix_create( char_data *ch, char * )
{
	mix_data	*node;
	static mix_data zero_node;

	node		= new mix_data;
	*node		= zero_node;
	node->next	= mix_list;
	mix_list	= node;

	mix_list->name				= str_dup( "New" );
	mix_list->creator			= str_dup( ch->name );
	mix_list->ritem_type		= ITEM_TRASH;

	ch->desc->pEdit		= (void *)mix_list;
	ch->desc->editor	= ED_MIX;
	ch->println( "Mixture Created." );
	return false;
}

/**************************************************************************/
bool mixedit_name( char_data *ch, char *argument )
{
	mix_data	*mix;

	EDIT_MIX( ch, mix );

	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:   name [name]" );
		return false;
	}

	replace_string( mix->name, argument );

	ch->printlnf( "Mixture is now known as '%s'.", mix->name );
	
    return true;
}

/**************************************************************************/
bool mixedit_lock( char_data *ch, char * )
{
	mix_data	*mix;

	EDIT_MIX( ch, mix );

	if ( mix->locked == false )
	{
		mix->locked = true;
		ch->println( "Mixture locked, only you or an admin may edit it while locked." );
	}
	else
	{
		mix->locked = false;
		ch->println( "Mixture unlocked, anyone may edit this mixture." );
	}

    return true;
}

/**************************************************************************/
bool mixedit_type( char_data *ch, char *argument )
{
	mix_data *mix;
	EDIT_MIX( ch, mix );
	int		 value;

	if ( IS_NULLSTR( argument )){
		ch->println( "Syntax:  type [mixtype]\r\n         ( ? mixtype to list)" );
		return false;
	}

	if (( value = flag_value( mixtype_types, argument )) != NO_FLAG )
	{
		mix->type = value;
		ch->println( "Type set." );
		return true;
	}

	show_olc_options(ch, mixtype_types, "type", "mixtype", mix->type);
	return false;
}

/**************************************************************************/
bool mixedit_difficulty( char_data *ch, char *argument )
{
	mix_data *mix;
	int mod;
	
	EDIT_MIX( ch, mix );
	
	if ( argument[0] == '\0' )
	{
		int i;

		ch->print( "Syntax: difficulty [type]\r\nValid ones are: [`c" );
		for ( i = 0; i < DIFF_MAX;	i++ )
			ch->printf( " %s (%d) ", modifier_table[i].name, modifier_table[i].modifier );
		ch->print( "`x ]\r\n" );
		return false;
	}
	
	mod = difficulty_lookup( argument );
	
	if ( mod < 0 )
	{
		ch->println( "Invalid difficulty type." );
		mixedit_difficulty( ch, "" );
		return false;
	}

	mix->difficulty = mod;
	
	ch->println( "Ok." );
	return true;
}

/**************************************************************************/
bool mixedit_vnum( char_data *ch, char *argument )
{
	mix_data *mix;
	vn_int vnum;
	
	EDIT_MIX( ch, mix );
	
	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:  vnum [number]"  );
		ch->println( "         (Indicates which VNUM this data refers to."  );
		return false;
	}
	
	vnum = atoi( argument );

	if ( vnum < 1 || vnum > game_settings->olc_max_vnum)
	{
		ch->printlnf( "VNUM must be greater than 0 and less than %d", game_settings->olc_max_vnum );
		return false;
	}
	
	mix->vnum_template = vnum;
	
	ch->printlnf( "VNUM set to '%d'.", mix->vnum_template );
	return true;
}

/**************************************************************************/
bool mixedit_rname( char_data *ch, char *argument )
{
	mix_data	*mix;

	EDIT_MIX( ch, mix );

	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:   rname [name] - This is the name of the result" );
		return false;
	}

	replace_string( mix->rname, argument );

	ch->printlnf( "Resulting name is now '%s'.", mix->rname );
	
    return true;
}

/**************************************************************************/
bool mixedit_failedrname( char_data *ch, char *argument )
{
	mix_data	*mix;

	EDIT_MIX( ch, mix );

	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:   failedrname [name] - This is the name of a failed result" );
		return false;
	}

	replace_string( mix->failedrname, argument );

	ch->printlnf( "Failed resulting name is now '%s'.", mix->failedrname );
	
    return true;
}

/**************************************************************************/
bool mixedit_rtype( char_data *ch, char *argument )
{
	mix_data	*mix;
	EDIT_MIX( ch, mix );
	int value;

	if( !IS_NULLSTR(argument)){		
		if (( value = flag_value( item_types, argument )) != NO_FLAG )
		{
			mix->ritem_type = value;
			ch->println( "Type set." );

			// Clear the values.
			switch ( mix->ritem_type )
			{
			case ITEM_SCROLL:
			case ITEM_POTION:
			case ITEM_PILL:
				mix->rvalue[0] = -1;
				mix->rvalue[1] = -1;
				mix->rvalue[2] = -1;
				mix->rvalue[3] = -1;
				mix->rvalue[4] = -1;
				break;
			case ITEM_POULTICE:
			case ITEM_WAND:
			case ITEM_STAFF:
				mix->rvalue[0] = 0;
				mix->rvalue[1] = 0;
				mix->rvalue[2] = 0;
				mix->rvalue[3] = -1;
				mix->rvalue[4] = 0;
				break;
			default:
				mix->rvalue[0] = 0;
				mix->rvalue[1] = 0;
				mix->rvalue[2] = 0;
				mix->rvalue[3] = 0;
				mix->rvalue[4] = 0;
				break;
			}
			return true;
		}
	}

	show_olc_options(ch, item_types, "rtype", "resulting object type", mix->ritem_type);
	return false;
}

/**************************************************************************/
bool mixedit_vessel( char_data *ch, char *argument )
{
	mix_data	*mix;
	EDIT_MIX( ch, mix );
	int value;

	if ( !IS_NULLSTR(argument) ){
		if (( value = flag_value( item_types, argument )) != NO_FLAG ){
			mix->vessel = value;
			ch->println( "Type set." );
			return true;
		}
	}

    ch->println( "Syntax:  vessel [item type]" );
	ch->println( "  Note:  there is no sanity checking done here, so it's up to you" );
	ch->println( "         to make sure it's right.  Currently, herbalism requires"  );
	ch->println( "         a `Ycauldron`x or `Ymortar`x item type to work.  More item" );
	ch->println( "         types will be added when new mixing skills are added ( ie, baking" );
	ch->println( "         skill may require an oven or bowl vessel to work [future project])" );
	return false;
}

/**************************************************************************/
bool mixedit_rshort( char_data *ch, char *argument )
{
	mix_data *mix;

	EDIT_MIX( ch, mix );

	// trim the spaces to the right of the short
	while ( !IS_NULLSTR(argument) && is_space(argument[str_len(argument)-1]))
	{
		argument[str_len(argument)-1]='\0';
	}
	
	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:  rshort [string]" );
		return false;
	}

	// make sure first char is lowercase
	argument[0] = LOWER(argument[0]);
	char *ptemp = mix->rshort;
	mix->rshort = str_dup( argument );
	mix->rshort[0] = LOWER( mix->rshort[0] );
	ch->printlnf( "Resulting vnum's short description from '%s' to '%s'.",
		ptemp, mix->rshort );

	free_string( ptemp );
	return true;
}

/**************************************************************************/
bool mixedit_rlong( char_data *ch, char *argument )
{
	mix_data *mix;

	EDIT_MIX( ch, mix );

	// trim the spaces to the right of the short
	while ( !IS_NULLSTR(argument) && is_space(argument[str_len(argument)-1]))
	{
		argument[str_len(argument)-1]='\0';
	}

	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax: rlong [string]" );
		return false;
	}

	char *ptemp = mix->rlong;
	mix->rlong = str_dup( argument );
	mix->rlong[0] = UPPER( mix->rlong[0] );

	ch->printlnf( "Long description from '%s' to '%s'.",	ptemp, mix->rlong );

	free_string( ptemp );

    return true;
}

/**************************************************************************/
bool mixedit_failedrshort( char_data *ch, char *argument )
{
	mix_data *mix;

	EDIT_MIX( ch, mix );

	// trim the spaces to the right of the short
	while ( !IS_NULLSTR(argument) && is_space(argument[str_len(argument)-1]))
	{
		argument[str_len(argument)-1]='\0';
	}
	
	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:  failedrshort [string]" );
		return false;
	}

	// make sure first char is lowercase
	argument[0] = LOWER(argument[0]);
	char *ptemp = mix->failedrshort;
	mix->failedrshort = str_dup( argument );
	mix->failedrshort[0] = LOWER( mix->failedrshort[0] );
	ch->printlnf( "Failed resulting vnum's short description from '%s' to '%s'.",
		ptemp, mix->failedrshort );

	free_string( ptemp );
	return true;
}

/**************************************************************************/
bool mixedit_failedrlong( char_data *ch, char *argument )
{
	mix_data *mix;

	EDIT_MIX( ch, mix );

	// trim the spaces to the right of the short
	while ( !IS_NULLSTR(argument) && is_space(argument[str_len(argument)-1]))
	{
		argument[str_len(argument)-1]='\0';
	}

	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax: failedrlong [string]" );
		return false;
	}

	char *ptemp = mix->failedrlong;
	mix->failedrlong = str_dup( argument );
	mix->failedrlong[0] = UPPER( mix->failedrlong[0] );

	ch->printlnf( "Failed long description from '%s' to '%s'.",	ptemp, mix->failedrlong );

	free_string( ptemp );

    return true;
}

/**************************************************************************/
bool mixedit_rwear( char_data *ch, char *argument )
{
	mix_data	*mix;
	EDIT_MIX( ch, mix );
	int value;

	if ( !IS_NULLSTR( argument )){
		if (( value = flag_value( wear_flags, argument ) ) != NO_FLAG )
		{
			TOGGLE_BIT( mix->rwear, value);
			ch->println( "Wear flag toggled." );
			return true;
		}
	}

	show_olc_options(ch, wear_flags, "rwear", "resulting object wear", mix->rwear);
	return false;
}

/**************************************************************************/
bool set_mixvalue( char_data *ch, mix_data *mix, char *argument, int value )
{
	if ( IS_NULLSTR( argument ))
	{
		set_mix_values( ch, mix, -1, "" );
		return false;
	}

	if ( set_mix_values( ch, mix, value, argument ) )
		return true;
	return false;
}

/**************************************************************************/
bool set_mix_values( char_data *ch, mix_data *mix, int value_num, char *argument)
{
	OBJ_INDEX_DATA	*oid;
	int value;
	char combuf[MIL];

	switch( mix->ritem_type )
	{
	default:
		break;
	case ITEM_LIGHT:
		switch ( value_num )
		{
		default:
			ch->println( "Only valid option is V2, which is the hours of light a light provides." );
			do_help( ch, "OLC-ITEM-LIGHT" );
			return false;
		case 2:
			ch->println( "HOURS OF LIGHT SET." );
			mix->rvalue[2] = atoi( argument );
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
			if(value==mix->rvalue[0]){
				ch->println( "Spell level unchanged.\r\n" );
			}else{
				ch->printlnf( "Spell level changed from %d to %d.\r\n",
					mix->rvalue[0], value);
				mix->rvalue[0] = value;
			}
			break;
		case 1:
			ch->println( "TOTAL NUMBER OF CHARGES SET." );
			mix->rvalue[1] = atoi( argument );
			break;
		case 2:
			ch->println( "CURRENT NUMBER OF CHARGES SET." );
			mix->rvalue[2] = atoi( argument );
			break;
		case 3:
			sprintf(combuf, "v%d", value_num);
			olc_generic_skill_assignment_to_int(ch, &mix->rvalue[value_num],
				argument,true, true, combuf, "resulting object (type %s) spell", 
											item_name(mix->ritem_type));
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
			if(value==mix->rvalue[0]){
				ch->println( "Spell level unchanged.\r\n" );
			}else{
				ch->printlnf( "Spell level changed from %d to %d.\r\n",
					mix->rvalue[0], value);
				mix->rvalue[0] = value;
			}
			break;
		case 1:
			ch->println( "TOTAL NUMBER OF APPLICATIONS SET." );
			mix->rvalue[1] = atoi( argument );
			break;
		case 2:
			ch->println( "CURRENT NUMBER OF APPLICATIONS SET." );
			mix->rvalue[2] = atoi( argument );
			break;
		case 3:
			sprintf(combuf, "v%d", value_num);
			olc_generic_skill_assignment_to_int(ch, &mix->rvalue[value_num],
				argument,true, true, combuf, "resulting object (type %s) spell", 
											item_name(mix->ritem_type));
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
			if(value==mix->rvalue[0]){
				ch->println( "Spell level unchanged." );
			}else{
				ch->printlnf( "Spell level changed from %d to %d.\r\n",
					mix->rvalue[0], value);
				mix->rvalue[0] = value;
			}
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			sprintf(combuf, "v%d", value_num);
			olc_generic_skill_assignment_to_int(ch, &mix->rvalue[value_num],
				argument,true, true, combuf, "resulting object (type %s) spell%d", 
											item_name(mix->ritem_type), value_num);
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
			mix->rvalue[0] = atoi( argument );
			break;
		case 1:
			ch->println( "# OF SONGS BEFORE RETUNING SET.  (-1 is infinite)" );
			mix->rvalue[1] = atoi(argument);
			break;
		}
		break;
	case ITEM_PARCHMENT:
		switch ( value_num )
		{
		default:
			return false;
		case 0:
			return false;
		case 1:
			ch->println( "Written on field set.  Blank = 0 all else means it can't be written on." );
			mix->rvalue[1] = atoi( argument );
			break;
		case 2:
			ch->println( "Sealed field set.  Unsealed = 0 all else means it's sealed." );
			mix->rvalue[2] = atoi( argument );
			break;
		case 3:			
			mix->rvalue[3] = language_safe_lookup(argument)->unique_id;				
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
			ch->println( "AC PIERCE SET." );
			mix->rvalue[0] = atoi( argument );
			break;
		case 1:
			ch->println( "AC BASH SET." );
			mix->rvalue[1] = atoi( argument );
			break;
		case 2:
			ch->println( "AC SLASH SET." );
			mix->rvalue[2] = atoi( argument );
			break;
		case 3:
			ch->println( "AC EXOTIC SET." );
			mix->rvalue[3] = atoi( argument );
			break;
		}
	    break;

	case ITEM_WEAPON:
		switch ( value_num )
		{
		default:
			do_help( ch, "OLC-ITEM-WEAPON" );
			return false;
        case 0:
			ch->println( "WEAPON CLASS SET." );
			mix->rvalue[0] = flag_value( weapon_class_types, argument );
			break;
		case 1:
			ch->println( "NUMBER OF DICE SET." );
			mix->rvalue[1] = atoi( argument );
			ch->printlnf( "Average damage is now %d", 
				(int) (mix->rvalue[1]+(mix->rvalue[1] * mix->rvalue[2]))/2);
			break;
		case 2:
			ch->println( "TYPE OF DICE SET." );
			mix->rvalue[2] = atoi( argument );
			ch->printlnf( "Average damage is now %d",
					(int) (mix->rvalue[1]+(mix->rvalue[1] * mix->rvalue[2]))/2);
			break;
        case 3:
			ch->println( "WEAPON TYPE SET." );
			mix->rvalue[3] = attack_lookup( argument );
			break;
		case 4:
			ch->println( "SPECIAL WEAPON TYPE TOGGLED." );
			mix->rvalue[4] ^= (flag_value( weapon_flags, argument ) != NO_FLAG
				? flag_value( weapon_flags, argument ) : 0 );
			break;
		}
		break;
		
	case ITEM_PORTAL:
		switch ( value_num )
		{
		default:
			do_help(ch, "OLC-ITEM-PORTAL" );
			return false;
		case 0:
			ch->println( "CHARGES SET." );
			mix->rvalue[0] = atoi ( argument );
			break;
		case 1:
			if (( value = flag_value( exit_flags, argument )) != NO_FLAG )
				TOGGLE_BIT( mix->rvalue[1], value);
			else
			{
				ch->printlnf( "Valid v1 portal exit flags are: %s", 
					flag_string( exit_flags, -1));
				do_help ( ch, "OLC-ITEM-PORTAL" );
				return false;
			}
			ch->println( "PORTAL EXIT FLAG SET." );
			break;
		case 2:
			if ( ( value = flag_value( portal_flags, argument ) )  != NO_FLAG )
				TOGGLE_BIT( mix->rvalue[2], value);
			else
			{
				ch->printlnf( "Valid v2 portal flags are: %s",
					flag_string( portal_flags, -1));
				do_help ( ch, "OLC-ITEM-PORTAL" );
				return false;
			}
			ch->println( "PORTAL EXIT FLAG SET." );
			break;
		case 3:
			ch->println( "EXIT VNUM SET." );
			mix->rvalue[3] = atoi ( argument );
			break;
		}
		break;

	case ITEM_TOKEN:
		switch ( value_num )
		{
		default:
			do_help(ch, "OLC-ITEM-TOKEN" );
			return false;
		case 0:
			ch->println( "Token Flag Toggled." );
			mix->rvalue[0] = ( flag_value( token_flags, argument ) != NO_FLAG
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
			ch->println( "Charges set." );
			mix->rvalue[0] = atoi( argument );
			break;
		case 1:
			olc_generic_skill_assignment_to_int(ch, &mix->rvalue[value_num],
				argument,true, true, "v1", "resulting object (%s) spell %d", 
											item_name(mix->ritem_type), value_num);
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
			ch->println( "NUMBER OF PEOPLE SET." );
			mix->rvalue[0] = atoi ( argument );
			break;
		case 1:
			ch->println( "MAX WEIGHT SET." );
			mix->rvalue[1] = atoi ( argument );
			break;
		case 2:
			ch->printlnf( "Valid v2 Furniture flags are: %s",
				flag_string( furniture_flags, -1));
			ch->println( "FURNITURE FLAGS TOGGLED." );
			mix->rvalue[2] ^= (flag_value( furniture_flags, argument ) != NO_FLAG
				? flag_value( furniture_flags, argument ) : 0);
			break;
		case 3:
			ch->println( "HEAL BONUS SET." );
			mix->rvalue[3] = atoi ( argument );
			break;
		case 4:
			ch->println( "MANA BONUS SET." );
			mix->rvalue[4] = atoi ( argument );
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
			return false;
		case 0:
			ch->wraplnf( "Maximum combined weight the resulting container can "
				"hold changed from %d lbs to %d lbs.", 
				mix->rvalue[0], atoi( argument ));
			mix->rvalue[0] = atoi( argument );
			break;
		case 1:
			if ( ( value = flag_value( container_flags, argument ) ) != NO_FLAG )
				TOGGLE_BIT( mix->rvalue[1], value);
			else
			{
				ch->printlnf( "Valid v1 container flags are: %s",
					flag_string( container_flags, -1));
				do_help ( ch, "OLC-ITEM-CONTAINER" );
				return false;
			}
			ch->println( "CONTAINER FLAG SET." );
			break;
		case 2:
			if ( atoi(argument) != 0 )
			{
				if ( !get_obj_index( atoi( argument )))
				{
					ch->println( "THERE IS NO SUCH ITEM." );
					return false;
				}

				if ( get_obj_index( atoi( argument ))->item_type != ITEM_KEY )
				{
					ch->println( "THAT ITEM IS NOT A KEY." );
					return false;
				}
			}
			ch->println( "CONTAINER KEY SET." );
			mix->rvalue[2] = atoi( argument );
			break;
		case 3:
			ch->wraplnf( "Maximum weight any single object able to be put into the "
				"resulting container changed from %d lbs to %d lbs.", 
				mix->rvalue[3], atoi( argument ));
			mix->rvalue[3] = atoi( argument );
			break;

		case 4:
			ch->println( "WEIGHT MULTIPLIER SET." );
			mix->rvalue[4] = atoi ( argument );
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
			ch->println( "MAXIMUM AMOUT OF LIQUID HOURS SET." );
			mix->rvalue[0] = atoi( argument );
			break;
		case 1:
			ch->println( "CURRENT AMOUNT OF LIQUID HOURS SET." );
			mix->rvalue[1] = atoi( argument );
			break;
		case 2:
			ch->println( "LIQUID TYPE SET." );
			mix->rvalue[2] = ( liq_lookup(argument) != -1 ? liq_lookup(argument) : 0 );
			break;
		case 3:
			ch->println( "POISON VALUE TOGGLED." );
			mix->rvalue[3] = ( mix->rvalue[3] == 0 ) ? 1 : 0;
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
			ch->println( "MAXIMUM AMOUT OF LIQUID HOURS SET." );
			mix->rvalue[0] = atoi( argument );
			break;
		case 1:
			ch->println( "CURRENT AMOUNT OF LIQUID HOURS SET." );
			mix->rvalue[1] = atoi( argument );
			break;
		case 2:
			ch->println( "LIQUID TYPE SET." );
			mix->rvalue[2] = ( liq_lookup( argument ) != -1 ? liq_lookup( argument ) : 0 );
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
			ch->println( "HOURS OF FOOD SET." );
			mix->rvalue[0] = atoi( argument );
			break;
		case 1:
			ch->println( "HOURS OF FULL SET." );
			mix->rvalue[1] = atoi( argument );
			break;
		case 3:
			ch->println( "POISON VALUE TOGGLED." );
			mix->rvalue[3] = ( mix->rvalue[3] == 0 ) ? 1 : 0;
			break;
		}
		break;

	case ITEM_MONEY:
		switch ( value_num )
		{
		default:
			do_help( ch, "OLC-ITEM-MONEY" );
			return false;
		case 0:
			ch->println( "GOLD AMOUNT SET." );
			mix->rvalue[0] = atoi( argument );
			break;
		case 1:
			ch->println( "SILVER AMOUNT SET." );
			mix->rvalue[1] = atoi( argument );
			break;
		}
		break;
	}

	if (( oid = get_obj_index( OBJ_VNUM_DUMMY )) == NULL )
	{
		return true;
	}

	oid->item_type	= mix->ritem_type;
	oid->value[0]	= mix->rvalue[0];
	oid->value[1]	= mix->rvalue[1];
	oid->value[2]	= mix->rvalue[2];
	oid->value[3]	= mix->rvalue[3];
	oid->value[4]	= mix->rvalue[4];
	show_obj_values( ch, oid );

	// OBJ_VNUM_DUMMY shouldn't be left anything other than VNUM_DUMMY
	oid->item_type	= ITEM_TRASH; 

    return true;
}


/**************************************************************************/
bool mixedit_values( char_data *ch, char *argument, int value )
{
	mix_data *mix;

    EDIT_MIX( ch, mix );

	if ( set_mixvalue( ch, mix, argument, value ))
		return true;
	return false;
}


/**************************************************************************/
bool mixedit_value0( char_data *ch, char *argument )
{
	if ( mixedit_values( ch, argument, 0 ))
        return true;
    return false;
}

/**************************************************************************/
bool mixedit_value1( char_data *ch, char *argument )
{
	if ( mixedit_values( ch, argument, 1 ))
		return true;
	return false;
}

/**************************************************************************/
bool mixedit_value2( char_data *ch, char *argument )
{
	if ( mixedit_values( ch, argument, 2 ))
		return true;
	return false;
}

/**************************************************************************/
bool mixedit_value3( char_data *ch, char *argument )
{
	if ( mixedit_values( ch, argument, 3 ))
		return true;
	return false;
}

/**************************************************************************/
bool mixedit_value4( char_data *ch, char *argument )
{
	if ( mixedit_values( ch, argument, 4 ))
		return true;
	return false;
}

/**************************************************************************/
bool mixedit_ingredients( char_data *ch, char *argument )
{
	mix_data	*mix;
	char		parg[MIL];
	int			pos;
	char		varg[MIL];
	vn_int		vnum;
	char		qarg[MIL];
	vn_int		qty;
	int i;
	
	EDIT_MIX( ch, mix );
	
	if ( IS_NULLSTR( argument ))
	{
		ch->println( "Syntax:  ingredient <#> <vnum> <qty>"  );
		ch->println( "         (# = a number between 1 and 5)");
		ch->println( "         (<vnum> = object vnum)");
		ch->println( "         (<qty> = required quantity of object)");
		ch->println( "Syntax:  ingredient <#> -    (to clear an ingredient line)");
		return false;
	}

	// check the position
	argument = one_argument( argument, parg );
	pos = atoi( parg );
	if (  pos < 1 || pos  > 5 )
	{
		ch->println( "# must be between 1 and 5." );
		return false;
	}

	// check the vnum
	argument = one_argument( argument, varg );	
	if(varg[0]=='-' && varg[1]=='\0'){
		// clearing entry
		ch->printlnf( "Position %d cleared (was %d, qty %d).", 	
			pos,
			mix->ingredients[pos-1],
			mix->ingredients_num[pos-1]);
		mix->ingredients[pos-1] = 0;
		mix->ingredients_num[pos-1] = 0;		
		mix_bsort( mix->ingredients, mix->ingredients_num, 5 );
		ch->println( "Required ingredient combination resorted." );
		mixedit_show(ch,"");
		return true;
	}
	if(!is_number(varg)){
		ch->printlnf( "Vnum must be numeric or -, '%s' is neither of these.", varg );
		mixedit_ingredients(ch,"");
		return false;
	}
	vnum = atoi( varg);
	if ( vnum < 1 ){
		ch->printlnf( "Ingredient vnum for position %d must be greater than zero", pos);
		mixedit_ingredients(ch,"");
		return false;
	}
	
	// check the quantity
	argument = one_argument( argument, qarg );
	qty= atoi( qarg );
	if (  qty < 1 || qty > 25 )	{
		ch->printlnf( "The quantity of ingredient %d for position %d must be between 1 and 25.", 
			vnum, pos);
		return false;
	}

	// check the ingredient value isn't already use in another position 
	for(i=0; i<5; i++){
		if(i==pos-1){
			continue;
		}
		
		if(mix->ingredients[i]==vnum){
			ch->printlnf("There is already an ingredient with of a vnum of %d in position %d.",
				vnum, i+1);
			return false;
		}
	}

	// announce the change
	ch->printlnf( "Ingredient in position %d changed (was vnum %d, qty %d).", 	
		pos, mix->ingredients[pos-1], mix->ingredients_num[pos-1]);

	mix->ingredients[pos-1] = vnum;
	mix->ingredients_num[pos-1] = qty;

	// resort
	mix_bsort( mix->ingredients, mix->ingredients_num, 5 );
	ch->println( "Required ingredient combination resorted." );

	// display updated sorting
	mixedit_show(ch,"");

	return true;
}


/**************************************************************************/
// quick and dirty bubble sort routine for mix vnums<>num_ingredients
// had to kind of cheat since I was trying to sort the vnums of the
// container and assign the number of each ingredient needed to the
// correct vnum, couldn't use the qsort since the two arrays aren't
// really dependent on each other once sorted
void mix_bsort( vn_int array[], sh_int num[], int size )
{
	int temp, i, j;

	for ( i = 0; i < size; i++ ){
		for ( j = 0; j < size; j++ ){
			if ( array[i] < array[j] ){
				temp = array[i];
				array[i] = array[j];
				array[j] = temp;
				// here comes the hack, all dressed in white
				temp = num[i];
				num[i] = num[j];
				num[j] = temp;
			}
		}
	}
}

/**************************************************************************/
//     uses mixedit_show to display info
void do_mixshow( char_data *ch, char *argument )
{
	mix_data *m;
	void * pTemp = NULL;

	if (!HAS_SECURITY(ch,1)){
		ch->println( "The mixshow command is an olc command, you don't have olc permissions." );
		return;
	}

	if ( IS_NULLSTR( argument ))
	{
		ch->println( "Syntax:  mixshow <mixname>" );
		return;
	}

	m = find_mix( ch, argument );

	if( !m )
	{
		ch->printlnf("There is no mixture named '%s'", argument );
		return;
	};

	pTemp = ch->desc->pEdit;
	ch->desc->pEdit = (void *)m;

    mixedit_show( ch, "" );
	ch->desc->pEdit = pTemp;
    return; 
}

/**************************************************************************/
/**************************************************************************/
