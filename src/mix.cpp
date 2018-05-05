/**************************************************************************/
// mix.cpp - mixture code, Kerenos
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "gameset.h"
#include "db.h"

void mix_bsort( vn_int array[], sh_int num[], int size );
void empty_container( OBJ_DATA *container );

#define BADMIX_LVNUM (game_settings->obj_vnum_mix_bad_low)
#define BADMIX_HVNUM (game_settings->obj_vnum_mix_bad_high)
/**************************************************************************/
// semilocalized globals
extern mix_data	*mix_list;

/**************************************************************************/
void do_herbalism( char_data *ch, char *argument)
{
	mix_data	*m;
	OBJ_DATA	*container;
	OBJ_DATA	*obj;
	vn_int		vnum_elements[6], vtemp = 0;
	sh_int		num_elements[6];
	int			total_elements = 0;
	int			i;
	int			chance;
	int			level;
	bool		mFound = false;

	// see if they know how to mix herbs in the first place
	if ( get_skill( ch, gsn_herbalism ) == 0 )
	{
		ch->println( "You don't know the first thing about mixing herbs." );
		return;
	}

	if ( IS_NULLSTR( argument ))
	{
		ch->println( "You must specify a valid cauldron or mortar in which to mix your herbs." );
		return;
	}

	// start off easy, get the container obj
	if (( container = get_obj_carry( ch, argument )) == NULL )
	{
		ch->println( "You are not carrying that." );
		return;
	}

	// determine if it's a valid container
	switch ( container->item_type )
	{
	default:
		{
			ch->println( "You can't mix herbs in that." );
			return;
		}
	case ITEM_FLASK:
		{
			ch->println( "You can't mix herbs in that in this way, try using mixflask instead." );
			return;
		}
	case ITEM_MORTAR:
	case ITEM_CAULDRON:
		break;
	}

	// check for an empty container to crap out at the earliest point
	if ( container->contains == NULL )
	{
		ch->printlnf( "There is nothing in %s.", container->short_descr );
		return;
	}

	// initialize
	for ( i = 0; i < 5; i++ )
	{
		num_elements[i]  = 0;
		vnum_elements[i] = 0;
	}

	// main counting loop to catalogue all the items in the container
	for ( obj = container->contains; obj; obj = obj->next_content )
	{
		vtemp = obj->pIndexData->vnum;

		for ( i=0; i < 5; i++ )
		{
			// increment count of elements if same object found
			if ( vtemp == vnum_elements[i] )
			{
				num_elements[i]++;
				break;
			}
			else if ( vnum_elements[i] == 0 )
			{
				// found a new unique item
				vnum_elements[i] = vtemp;
				num_elements[i]++;
				total_elements++;
				break;
			}
		}
	}

	// finally catalogued all items in the container, time to sort'em
	mix_bsort( vnum_elements, num_elements, 5 );
	
	// time to compare the catalogued contents vs the mixture dbase
	// by now, both are sorted by this point for easier comparison
	for ( m = mix_list; m; m = m->next )
	{
		if ( m->type == MIXTYPE_HERBALISM )
		{
			for ( i = 0; i < 5; i++ )
			{
				mFound = false;
				if (!( vnum_elements[i] == m->ingredients[i] 
					&& num_elements[i]  == m->ingredients_num[i] ))
				{
					mFound = false;
					break;
				}
				mFound = true;
			}
		}
		if ( mFound )
			break;
	}

	// if we didn't find a matching mix, ditch all the ingredients
	if ( !mFound )
	{
		ch->println( "You try to combine the ingredients, but they yield no useful result." );
		empty_container( container );
		return;
	}

	level = UMAX(1, ch->level - (( 5 - modifier_table[m->difficulty].type ) * 3 ));
	if ( !IS_IMMORTAL(ch) && level > LEVEL_HERO ){
		level = LEVEL_HERO;
	}

	// VESSEL CHECK ?

	// skill check to see if the result will happen or not
	chance = (( get_skill( ch, gsn_herbalism ) - m->difficulty ) + ch->modifiers[STAT_IN] );
	if ( chance >= number_percent() )
	{
		OBJ_INDEX_DATA	*obj_template;
		OBJ_DATA		*result;
		
		// success, let's restring
		if (( obj_template = get_obj_index( m->vnum_template )) == NULL )
		{
			ch->printlnf( "Vnum %d is missing for the %s mix, write a note to realm/admin",
				m->vnum_template, m->name );
			return;
		}

		result = create_object( obj_template);

		replace_string( result->name,		 m->rname );
		replace_string( result->short_descr, m->rshort );
		replace_string( result->description, m->rlong  );

		result->level			= ch->level;
		result->item_type		= m->ritem_type;
		result->wear_flags		= m->rwear;
		
		for ( i = 1; i < 5; i++ ){
			result->value[i]	= m->rvalue[i];
		}
		result->value[0]		= level;

		ch->printlnf( "You mix the ingredients within %s and have concocted %s.",
			container->short_descr, m->rshort );
		act( "$n busies $mself and mixes a concoction within $p.", ch, container, NULL, TO_ROOM );
		empty_container( container );
		obj_to_obj( result, container );
		WAIT_STATE( ch, skill_table[gsn_herbalism].beats );
		check_improve( ch, gsn_herbalism, true, 3 );
	}
	// failed, create a bad result
	else
	{
		OBJ_INDEX_DATA	*obj_template;
		OBJ_DATA		*result;
		int value=number_range( BADMIX_LVNUM, BADMIX_HVNUM );

		if (( obj_template = get_obj_index(value)) == NULL )
		{
			ch->wraplnf( "Vnum %d (a random value between game_settings->obj_vnum_mix_bad_low(%d) "
				"and game_settings->obj_vnum_mix_bad_high(%d)) is missing for "
				"the %s mix, write a note to realm/admin including all the "
				"information in this message.",
				value, BADMIX_LVNUM, BADMIX_HVNUM, m->name );
			return;
		}
		result = create_object( obj_template);
		ch->printlnf( "You mix the ingredients within %s and have concocted %s.",
			container->short_descr, m->failedrshort );
		replace_string( result->name,		 m->failedrname );
		replace_string( result->short_descr, m->failedrshort );
		replace_string( result->description, m->failedrlong  );
		result->level= level;

		act( "$n busies $mself and mixes a concoction within $p.", ch, container, NULL, TO_ROOM );
		WAIT_STATE( ch, skill_table[gsn_herbalism].beats );
		check_improve( ch, gsn_herbalism, true, 3 );
		empty_container( container );
		obj_to_obj( result, container );

		if(chance>50){
			ch->println(2, "Perhaps that didn't go quite right.");
		}
	}
};

/**************************************************************************/
// mixflask only version of herbalism - Kal, Sept 02
void do_mixflask( char_data *ch, char *argument)
{
	mix_data	*m;
	OBJ_DATA	*container;
	OBJ_DATA	*obj;
	vn_int		vnum_elements[6], vtemp = 0;
	sh_int		num_elements[6];
	int			total_elements = 0;
	int			i;
	int			chance;
	int			level;
	bool		mFound = false;

	if ( IS_NULLSTR( argument )){
		ch->println( "Syntax: mixflask <flask>   - to mix herbs in a flask.");
		ch->println( "This command requires the herbalism skill to use." );
		return;
	}

	// see if they know how to mix herbs in the first place
	if ( get_skill( ch, gsn_herbalism ) == 0 )
	{
		ch->println( "You don't know the first thing about mixing herbs." );
		return;
	}

	// start off easy, get the container obj
	if (( container = get_obj_carry( ch, argument )) == NULL )
	{
		ch->println( "You are not carrying that." );
		return;
	}

	// determine if it's a valid container
	switch ( container->item_type )
	{
	default:
		{
			ch->println( "You can't mix herbs in that." );
			return;
		}

	case ITEM_MORTAR:
	case ITEM_CAULDRON:
		{
			ch->println( "You can't use mixflask to mix herbs in that, try using herbalism!" );
			return;
		}

	case ITEM_FLASK:
		break;
	}

	// check for an empty container to crap out at the earliest point
	if ( container->contains == NULL ){
		ch->printlnf( "There is nothing in %s.", container->short_descr );
		return;
	}

	// initialize
	for ( i = 0; i < 5; i++ )
	{
		num_elements[i]  = 0;
		vnum_elements[i] = 0;
	}

	// main counting loop to catalogue all the items in the container
	for ( obj = container->contains; obj; obj = obj->next_content )
	{
		vtemp = obj->pIndexData->vnum;

		for ( i=0; i < 5; i++ )
		{
			// increment count of elements if same object found
			if ( vtemp == vnum_elements[i] )
			{
				num_elements[i]++;
				break;
			}
			else if ( vnum_elements[i] == 0 )
			{
				// found a new unique item
				vnum_elements[i] = vtemp;
				num_elements[i]++;
				total_elements++;
				break;
			}
		}
	}

	// finally catalogued all items in the container, time to sort'em
	mix_bsort( vnum_elements, num_elements, 5 );
	
	// time to compare the catalogued contents vs the mixture dbase
	// by now, both are sorted by this point for easier comparison
	for ( m = mix_list; m; m = m->next )
	{
		if ( m->type == MIXTYPE_HERBALISM )
		{
			for ( i = 0; i < 5; i++ )
			{
				mFound = false;
				if (!( vnum_elements[i] == m->ingredients[i] 
					&& num_elements[i]  == m->ingredients_num[i] ))
				{
					mFound = false;
					break;
				}
				mFound = true;
			}
		}
		if ( mFound )
			break;
	}

	// if we didn't find a matching mix, ditch all the ingredients
	if ( !mFound )
	{
		ch->println( "You try to combine the ingredients, but they yield no useful result." );
		empty_container( container );
		return;
	}

	level = UMAX(1, ch->level - (( 5 - modifier_table[m->difficulty].type ) * 3 ));
	if ( !IS_IMMORTAL(ch) && level > LEVEL_HERO ){
		level = LEVEL_HERO;
	}

	// VESSEL CHECK ?

	// skill check to see if the result will happen or not
	chance = (( get_skill( ch, gsn_herbalism ) - m->difficulty ) + ch->modifiers[STAT_IN] );
	if ( chance >= number_percent() )
	{
		OBJ_INDEX_DATA	*obj_template;
		OBJ_DATA		*result;
		
		// success, let's restring
		if (( obj_template = get_obj_index( m->vnum_template )) == NULL )
		{
			ch->printlnf( "Object vnum %d is missing for the %s mix (template object), "
				"write a note to realm/admin",
				m->vnum_template, m->name );
			return;
		}

		result = create_object( obj_template);

		replace_string( result->name,		 m->rname );
		replace_string( result->short_descr, m->rshort );
		replace_string( result->description, m->rlong  );

		result->level			= ch->level;
		result->item_type		= m->ritem_type;
		result->wear_flags		= m->rwear;
		
		for ( i = 1; i < 5; i++ ){
			result->value[i]	= m->rvalue[i];
		}
		result->value[0]		= level;

		ch->printlnf( "You mix the ingredients within %s and have concocted %s.",
			container->short_descr, m->rshort );
		act( "$n busies $mself and mixes a concoction within $p.", ch, container, NULL, TO_ROOM );
		extract_obj(container);		
		obj_to_char( result, ch);
		WAIT_STATE( ch, skill_table[gsn_herbalism].beats );
		check_improve( ch, gsn_herbalism, true, 3 );
	}
	// failed, create a bad result
	else
	{
		OBJ_INDEX_DATA	*obj_template;
		OBJ_DATA		*result;

		int badvnum=number_range( BADMIX_LVNUM, BADMIX_HVNUM );
		if (( obj_template = get_obj_index(badvnum)) == NULL )
		{
			ch->printlnf( "Vnum %d (between BADMIX_LVNUM(%d) and BADMIX_HVNUM(%d)) is missing for the %s mix, write a note to realm/admin",
				badvnum, BADMIX_LVNUM, BADMIX_HVNUM, m->name );
			return;
		}
		result = create_object( obj_template);
		ch->printlnf( "You mix the ingredients within %s and have concocted %s.",
			container->short_descr, m->rshort );

		if(!IS_NULLSTR(m->failedrname)){
			replace_string( result->name, m->failedrname );
		}else{
			replace_string( result->name, m->rname );
		}

		if(!IS_NULLSTR(m->failedrshort)){
			replace_string( result->short_descr, m->failedrshort );
		}else{
			replace_string( result->short_descr, m->rshort );
		}

		if(!IS_NULLSTR(m->failedrlong)){
			replace_string( result->description, m->failedrlong );
		}else{
			replace_string( result->description, m->rlong  );
		}
		
		result->level= level;

		act( "$n busies $mself and mixes a concoction within $p.", ch, container, NULL, TO_ROOM );
		WAIT_STATE( ch, skill_table[gsn_herbalism].beats );
		check_improve( ch, gsn_herbalism, true, 3 );
		extract_obj( container );
		obj_to_char( result, ch);
	}
};


/**************************************************************************/
void empty_container( OBJ_DATA *container )
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( container->contains == NULL )
		return;

	for ( obj = container->contains; obj; obj = obj_next )
	{
		obj_next = obj->next_content;
		extract_obj( obj );
	}
}
/**************************************************************************/
