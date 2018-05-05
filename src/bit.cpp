/**************************************************************************/
// bit.cpp - see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  File: bit.cpp                                                          *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include "include.h" // dawn standard includes

#define STNM(flag) flag, #flag

struct flag_stat_type
{
    const struct flag_type *structure;
	const char *name;
    bool stat;
};

/**************************************************************************/
/*****************************************************************************
 Name:		flag_stat_table
 Purpose:	This table catagorizes the tables following the lookup
 		functions below into stats and flags.  Flags can be toggled
 		but stats can only be assigned.  Update this table when a
 		new set of flags is installed.
 ****************************************************************************/
const struct flag_stat_type flag_stat_table[] =
{
//	{	structure, text version				type	}, 
    {   STNM(ac_types),						true    },
    {   STNM(apply_types),					true    },
	{   STNM(area_import_format_types),		true    },
    {   STNM(ban_types),					true    },
    {   STNM(buildrestrict_types),			true    },  
    {   STNM(castnames_types),				true    },
    {   STNM(category_types),				true    },
    {   STNM(colourmode_types),				true    },  
    {   STNM(com_category_types),			true    },
    {   STNM(commandlog_types),				true    },
    {   STNM(damtype_types),				true    },
    {   STNM(direction_types),				true    },
	{   STNM(event_types),					true    },
    {   STNM(help_category_types),			true    },
    {   STNM(immhelp_types),				true    },
    {   STNM(item_types),					true    },
    {   STNM(mixtype_types),				true    },
    {   STNM(position_types),				true    },
    {   STNM(preference_types),				true    },
    {   STNM(sector_types),					true    },
    {   STNM(sex_types),					true    },
    {   STNM(size_types),					true    },
    {   STNM(sktype_types),					true    }, // different types of entries in the skill_table
    {   STNM(target_types),					true    },
    {   STNM(textsearch_types),				true    },
	{   STNM(to_types),						true    },    
    {   STNM(weapon_class_types),			true    },
    {   STNM(wear_location_types),			true    },
    {   STNM(wear_location_strings_types),	true    },
	{   STNM(laston_wizlist_types),			true    },
	// NOTE: ALL TRUE TABLES MUST BE ABOVE HERE

    {   STNM(act_flags),					false   },
    {   STNM(act2_flags),					false   },
    {   STNM(affect_flags),					false   },
    {   STNM(affect2_flags),				false   },
    {   STNM(affect3_flags),				false   },
    {   STNM(align_flags),					false   },
    {   STNM(attune_flags),					false   },
    {   STNM(classnames_flags),				false   },  
    {   STNM(commandflag_flags),			false   },
    {   STNM(container_flags),				false   },
    {   STNM(council_flags),				false   },
    {   STNM(dedit_flags),					false   },
	{   STNM(event_flags),					false	},
    {   STNM(exit_flags),					false   },
    {   STNM(grantgroup_flags),				false   },
	{   STNM(language_flags),				false	},
    {   STNM(objextra_flags),				false   },
    {   STNM(objextra2_flags),				false   },
    {   STNM(form_flags),					false   },
    {   STNM(imm_flags),					false   },
    {   STNM(off_flags),					false   },
    {   STNM(olc_flags),					false   },
    {   STNM(part_flags),					false   },
	{   STNM(res_flags	),          		false   },
    {   STNM(room_flags	),					false   },
	{   STNM(room2_flags),					false   },
    {   STNM(tendency_flags),				false   },
    {   STNM(vuln_flags),					false   },
    {   STNM(weapon_flags),					false   },
    {   STNM(wear_flags),					false   },
	{	0,"",			0	}
};
    


/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns true if the table is a stat table and false if flag.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
bool is_stat( const struct flag_type *flag_table )
{
	int flag;

	for (flag = 0; flag_stat_table[flag].structure && flag_stat_table[flag].stat; flag++)
	{
		if ( flag_stat_table[flag].structure == flag_table
			&& flag_stat_table[flag].stat )
			return true;
	}
	return false;
}

/**************************************************************************/
// Kal - Aug 01
const char *get_flag_table_name( const struct flag_type *flag_table )
{
	int flag;

	for (flag = 0; flag_stat_table[flag].structure; flag++)
	{
		if (flag_stat_table[flag].structure == flag_table){
			return flag_stat_table[flag].name;
		}
	}
	return "unknown";
}
/**************************************************************************/

/*
 * This function is Russ Taylor's creation.  Thanks Russ!
 * All code copyright (C) Russ Taylor, permission to use and/or distribute
 * has NOT been granted.  Use only in this OLC package has been granted.
 */
/*****************************************************************************
 Name:      flag_lookup( flag, table )
 Purpose:	Returns the value of a single, settable flag from the table.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int flag;
 
    // first try an exact match, then do a substring match

	// exact
	for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if ( !str_cmp( name, flag_table[flag].name )
          && flag_table[flag].settable )
            return flag_table[flag].bit;
    }

	// substring
	for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if ( !str_prefix( name, flag_table[flag].name )
          && flag_table[flag].settable )
            return flag_table[flag].bit;
    }
    return NO_FLAG;
}

/*****************************************************************************
 Name:		flag_value( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.

// use wordflag_to_value( const struct flag_type *flag_table, const char *wordtext)
// to convert a text into a value ignoring if the wordtext is settable.
 ****************************************************************************/
int flag_value( const struct flag_type *flag_table, char *argument)
{
    char word[MIL];
    int  bit;
    int  marked = 0;
    bool found = false;

	if(IS_NULLSTR(argument)){
		return NO_FLAG;
	}

    if ( is_stat( flag_table ) )
    {
		one_argument( argument, word );

		if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG ){
			return bit;
		}else{
			return NO_FLAG;
		}
    }

    // Accept multiple flags.
    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
	    break;

        if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG )
        {
            SET_BIT( marked, bit );
            found = true;
        }
    }

    if ( found ){
		return marked;
    }else{
		return NO_FLAG;
	}
}



/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
char *flag_string( const struct flag_type *flag_table, int bits )
{
	static int i;
    static char buf[5][MSL];
	char tbuf[MIL];
    int flag;
	bool bit_found=false;

	// rotate buffers
	++i%=5;

    buf[i][0] = '\0';

	// 2 different modes... if their is a stat table to be written 
	// a different mode is used from checking each bit is written
	if (is_stat( flag_table )){
	    for (flag = 0; flag_table[flag].name && !bit_found; flag++)
	    {
			if ( flag_table[flag].bit == bits ){
				strcat( buf[i], " " );
				strcat( buf[i], flag_table[flag].name );
				bit_found=true;
			}
		}
		// report that we have a missing value in a table
		if(!bit_found && bits!=-1)
		{
			sprintf(tbuf," `#`RVALUE%dMISSING!!!`&",bits);
			strcat( buf[i], tbuf);
		}
	}else{
		// write bits the long way so we can record if a bit isn't written due 
		// to no matching bit in the flag table.
		// Also write only one bit word for a single bit allowing multiple 
		// words for a bit value in a table (colour vs color) and single words in 
		// stored files.
		for (int bit_index=0; bit_index<32; bit_index++)
		{
			bit_found=false;
			int bit_value = 1<<bit_index;

			if (!IS_SET(bit_value, bits))
				continue;

			for (flag = 0; flag_table[flag].name && !bit_found; flag++)
			{
				if ( IS_SET(bit_value, flag_table[flag].bit) )
				{
					strcat( buf[i], " ");
					strcat( buf[i], flag_table[flag].name );
					bit_found= true;
				}
			}
			// report that we have a missing bit in a table
			if(!bit_found && bits!=-1)
			{
				sprintf(tbuf," `#`RBIT#%d(%c)MISSING!!!`&",
					bit_index,(bit_index <= 'Z' - 'A'?'A' + bit_index :
						'a' + bit_index - ( 'Z' - 'A' + 1 )));
				strcat( buf[i], tbuf);
			}
		}
	} // non stat table (multiple bit table)

	// return the result
    return (buf[i][0] != '\0') ? buf[i]+1 : (char*)"none";
}
/*****************************************************************************
 Name:      flagging_info( table, flags/stat )
 Purpose:   tells a char what flags they just toggled
 Called by: olc.c
 ****************************************************************************/
/**************************************************************************/
// perform only exact case insensitive matching
// returns a pointer to the table entry or NULL if unfound
// Kal - Feb 99
classgroup_type * classgroup_lookup(const char* name)
{
	int index;
	for(index=0; !IS_NULLSTR(classgroup_table[index].name);index++){
		if(!str_cmp(classgroup_table[index].name, name)){
			return &classgroup_table[index];
		}
	}
	return NULL;
}
/**************************************************************************/
affectprofile_type * affectprofile_lookup(const char *name)
{
	int index;
	for(index=0; !IS_NULLSTR(affectprofile_table[index].name);index++){
		if(!str_cmp(affectprofile_table[index].name, name)){
			return &affectprofile_table[index];
		}
	}
	return NULL;
}
/**************************************************************************/
// encode the bits into A->Z a->e form
char *flags_print(int flag)
{
	int count, pos = 0;
	static char buf[52];

	for (count = 0; count < 32;  count++)
	{
		if (IS_SET(flag,1<<count))
		{
			if (count < 26)
				buf[pos] = 'A' + count;
			else
				buf[pos] = 'a' + (count - 26);
			pos++;
		}
	}

	if (pos == 0)
	{
		buf[pos] = '0';
		pos++;
	}
	
	buf[pos] = '\0';
	return buf;
}
/**************************************************************************/
// convert bits from the form A->Z a->e into a number value
int flags_read(char *flagtext)
{
	int number;
	char c;
	char *p=flagtext;
	bool negative = false;
    
	do{
		c=*p++;
	}
	while ( is_space(c));
    
	if (c == '-')
	{
		negative = true;
		c = *p++;
	}

	number = 0;
    
	if (!is_digit(c))
	{
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
		{
			number += flag_convert(c);
			c = *p++;
		}
	}else{
		while (is_digit(c))
		{
			number = number * 10 + c - '0';
			c = *p++;
		}
	}

	if(negative){
		number*=-1;
	}
	return number;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
