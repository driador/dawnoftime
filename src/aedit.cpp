/**************************************************************************/
// aedit.cpp - olc area editor
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

void sort_arealists();
/**************************************************************************/
/*
 * Area Editor Functions.
 */
/**************************************************************************/
bool aedit_show(char_data *ch, char *)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    ch->printlnf( " `=rName:        [`=R%3d`=r] `r%s", pArea->vnum, pArea->name );  
    ch->printlnf( " `=rShortName:   `r%s", pArea->short_name );  
    ch->printlnf( " `=rFile:        `x%s", pArea->file_name );
    ch->printlnf( " `=rVnums:       [`x%d`=r-`x%d`=r]", 
		pArea->min_vnum, pArea->max_vnum );   
    ch->printlnf( " `=rMapScale:    [`x%d`=r]", 	pArea->mapscale);
    ch->printlnf( " `=rMapLevel:    [`x%d`=r]", 	pArea->maplevel);
	ch->printlnf( " `=rArea Version [`x%d`=r]",	pArea->version);
    ch->printlnf( " `=rColour code:  `x%s", pArea->colour);

	if (IS_NULLSTR(pArea->lcomment))
	{
        ch->println(" `=rLcomment:    `x<none>");
	}
	else
	{
        ch->printlnf( " `=rLcomment:    `=R%s", pArea->lcomment);
	}

	if (pArea->low_level==-1)
	{
        ch->println(" `=rLrange:      `x<none>");
	}
	else
	{
        ch->printlnf( " `=rLrange:      [`x%2d`=r-`x%2d`=r]", 
			pArea->low_level, pArea->high_level);
	}

	ch->printlnf( " `=rAge:         [`x%d`=r]", pArea->age );
	ch->printlnf( " `=rPlayers:     [`x%d`=r]", pArea->nplayer );
	ch->printlnf( " `=rSecurity:    [`x%d`=r]", pArea->security );
	ch->printlnf( " `=rBuilders:    [`x%s`=r]", IS_NULLSTR(pArea->builders)?"none": trim_string(pArea->builders));
	ch->printlnf( " `=rCredits :    [`x%s`=r]", IS_NULLSTR(pArea->credits)?"none": trim_string(pArea->credits));

	mxp_display_olc_flags(ch, area_flags,	pArea->area_flags,	"areaflags",	" AREAFlags:");
	mxp_display_olc_flags(ch, olc_flags,	pArea->olc_flags,	"olcflags",		" OLCFlags:");

	ch->printlnf( " `=rContinent:   [`x%s`=r]", 
		pArea->continent?pArea->continent->name:"Any" );
	// display builder restrictions 
	ch->println("--Builder restricts--");
	for (int br=0;br<MAX_BUILD_RESTRICTS;br++)
	{
		ch->printlnf( "`=r %-12s`x%s",
			capitalize(flag_string(buildrestrict_types, br)),
			IS_NULLSTR(pArea->build_restricts[br])?"`=Rnone": 
			pArea->build_restricts[br] );
	}

	// Show the area echoes :)
	ch->println("`#`W------============== `CA `YR `GE `MA   `RE `BC `CH `YO `GE `MS `W==============------`^");
	int recount=0;
	for( area_echo_data *ae = pArea->echoes; ae; ae = ae->next )
	{
		if(ae==pArea->echoes){
			ch->println("[##] >=Hr <=Hr   %  -==EchoText==-`x");
		}
		ch->printlnf(  "`=r[%2d] `=v%4s %4s %3d  `=x'%s'`x", recount++, 
			convert24hourto12hour(ae->firsthour), convert24hourto12hour(ae->lasthour), 
			ae->percentage, ae->echotext);
		if(has_colour(ae->echotext)){
			ch->print("`S");
			ch->printlnfbw(  "[%2d] %4s %4s %3d  '%s'", recount-1, 
				convert24hourto12hour(ae->firsthour), convert24hourto12hour(ae->lasthour), 
				ae->percentage, ae->echotext);
			ch->print("`x");
		};
	}
	if(recount==0){
		ch->println("`=Rnone - add using 'addecho'.`x");
	}

    return false;
}


/**************************************************************************/
int count_areas_in_range( int lower, int upper )
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
		if ( ( lower <= pArea->min_vnum && pArea->min_vnum <= upper )
			||   ( lower <= pArea->max_vnum && pArea->max_vnum <= upper ) )
			cnt++;
	}
	return cnt;
}

/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range( int lower, int upper )
{
	if ( count_areas_in_range(lower, upper) > 1 ){
	    return false;
	}
    return true;
}

/**************************************************************************/
// gets and area that matches the vnum range
AREA_DATA *get_vnum_area( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( vnum >= pArea->min_vnum
          && vnum <= pArea->max_vnum )
            return pArea;
    }

    return NULL;
}
/**************************************************************************/
bool aedit_reset(char_data *ch, char *)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    reset_area( pArea );
    ch->println("Area reset.");

    return false;
}

/**************************************************************************/
bool aedit_create(char_data *ch, char *)
{
    AREA_DATA *pArea;

    pArea               =   new_area();
	pArea->area_flags	=	AREA_OLCONLY | AREA_NOTELEPORT | AREA_NOSCRY | AREA_HIDDEN;
    SET_BIT( pArea->olc_flags, OLCAREA_ADDED );
	pArea->colourcode	=	COLOURCODE;
    area_last->next     =   pArea;
    area_last			=   pArea;
    ch->desc->pEdit     =   (void *)pArea;
	ch->desc->editor	=	ED_AREA;

    ch->println( "Area Created." );
    return false;
}

/**************************************************************************/
bool aedit_name(char_data *ch, char *argument)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( IS_NULLSTR(argument) )
    {
		ch->println( "Syntax:   name <name>" );
		return false;
    }

    ch->printlnf( "Area name changed from '%s' to '%s'.", pArea->name, strip_colour(argument) );
    replace_string( pArea->name, strip_colour(argument) );

	sort_arealists(); // resort the vlist and area lists
    return true;
}
/**************************************************************************/
bool aedit_shortname(char_data *ch, char *argument)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( IS_NULLSTR(argument)){
		ch->println( "Syntax:   shortname <name>" );
		ch->println( "Syntax:   shortname clear   - remove the name" );
		ch->println( "(this is the area name that is shown to the right of a room name)" );
		return false;
    }

	if(!str_cmp(argument, "clear")){
		ch->printlnf( "Area shortname cleared (was '%s').", pArea->short_name);
		replace_string( pArea->short_name, "");
    	return true;
	}

	if(str_len(argument)>12){
		ch->wrap( "NOTE: The short name is supposed to be SHORT!  "
			"i.e. one or two words only that can be used to uniquely "
			"identify the area - consider changing.");
	}

    ch->printlnf( "Area shortname changed from '%s' to '%s'.", pArea->short_name, argument );
    replace_string( pArea->short_name, argument );
    return true;
}

/**************************************************************************/
bool aedit_colour(char_data *ch, char *argument)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( IS_NULLSTR(argument) || str_len(argument)>1)
    {
		if (str_len(argument)>1)
		{
            ch->println("The colour code should be only 1 character!\r\n");
		}
        ch->println("Syntax:   colour <code>");
        ch->println("This is the colour of the area on the area list.  The colour code should");
        ch->println("be only the single code character (it doesn't include the colour prefix.");
		return false;
    }

    free_string( pArea->colour);
    pArea->colour = str_dup( argument );

    ch->printlnf( "Colour code set to %s.", pArea->colour);
    return true;
}

/**************************************************************************/
bool aedit_credits(char_data *ch, char *argument)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( IS_NULLSTR(argument) )
    {
		ch->println("Syntax:   credits <credits>");
		ch->printlnf( "notes: if the credits are set, these are shown in the arealist, "
			"in place of the builders names" );
		return false;
    }

	smash_tilde(argument);

	if(IS_NULLSTR(pArea->credits)){
	    ch->printlnf( "Credits set to '%s'.", argument);
	}else{
		ch->printlnf( "Credits changed from '%s' to '%s'.", 
			pArea->credits, argument);
	}
    replace_string( pArea->credits, argument );
    return true;
}

/**************************************************************************/
bool aedit_file(char_data *ch, char *argument)
{
    AREA_DATA *pArea, *pArealist;
    char file[MSL];
    int i, length;

    EDIT_AREA(ch, pArea);

    one_argument( argument, file );	// Forces Lowercase 

    if( IS_NULLSTR(argument)){
		ch->println( "Syntax:  filename <filename>" );
		return false;
    }

    // Simple Syntax Check.
    length = str_len( argument );
    if ( length > 12 )
    {
		ch->println( "No more than twelve letters allowed." );
		return false;
    }
    
    // Allow only letters, numbers and underscore.
    for ( i = 0; i < length; i++ )
    {
		if ( !is_alnum( file[i] ) && file[i]!='_')
		{
            ch->println( "Only letters, the underscore character '_' and numbers are valid in a filename." );
			return false;
		}
    }    

	// check to make sure there isn't already an area with 
	// the same name in the area list
    strcat( file, ".are" );
	if(file_exists(file)){
        ch->printlnf("There is already another file on the server with the filename '%s'.\r\n", file);
        ch->println("- Name unchanged.");
		return false;
	}
    for ( pArealist= area_first; pArealist; pArealist= pArealist->next )
    {
		if (!str_cmp( file, pArealist->file_name)){
            ch->println("There is already another area in the area list with that name.");
            ch->println("- Name unchanged.");
			return false;
		}
	}

	ch->printlnf( "Filename changed from '%s' to '%s'.",  
		pArea->file_name, file);
    replace_string( pArea->file_name, file);
    
    return true;
}

/**************************************************************************/
bool aedit_age(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char age[MSL];

    EDIT_AREA(ch, pArea);

    one_argument( argument, age );

    if ( !is_number( age ) || age[0] == '\0' )
    {
		ch->println("Syntax:  age [#xage]");
		return false;
    }

    pArea->age = atoi( age );

    ch->println("Age set.");
    return true;
}



/**************************************************************************/
bool aedit_security(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char sec[MSL];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, sec );

    if ( !is_number( sec ) || sec[0] == '\0' )
    {
		ch->println("Syntax:  security [#xlevel]");
		return false;
    }

    value = atoi( sec );

    if ( value > ch->pcdata->security || value < 0 )
	{
		if ( ch->pcdata->security != 0 )
		{
			ch->printlnf( "Security is 0-%d.", ch->pcdata->security );
		}
		else
			ch->println("Security is 0 only.");
		return false;
	}

	pArea->security = value;
	
	ch->println("Security set.");
	return true;
}

/**************************************************************************/
// Kalahn - Sept 98
bool aedit_mapscale(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char sec[MSL];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, sec );

    if ( !is_number( sec ) || sec[0] == '\0' )
    {
		ch->println("Syntax:  mapscale [#xlevel]");
		return false;
    }

    value = atoi( sec );

    if ( value > 9 || value < 0 )
    {
	    ch->println("Mapscale can be 0 to 9.");
		return false;
    }

    ch->printlnf( "Mapscale changed from %d to %d.", 
		pArea->mapscale, value);
    pArea->mapscale= value;
    return true;
}

/**************************************************************************/
// Kalahn - Sept 98
bool aedit_maplevel(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char sec[MSL];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, sec );

    if ( !is_number( sec ) || sec[0] == '\0' )
    {
		ch->println("Syntax:  maplevel [#xlevel]");
		return false;
    }

    value = atoi( sec );

    if ( value > ABSOLUTE_MAX_LEVEL || value < 0 )
    {
	    ch->printlnf( "Maplevel can be 0 to %d.", ABSOLUTE_MAX_LEVEL);
		return false;
    }

    ch->printlnf( "Maplevel changed from %d to %d.", 
		pArea->maplevel, value);
    pArea->maplevel= value;
    return true;
}

/**************************************************************************/
bool aedit_builder(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);

	char arg[MIL];
	one_argument(argument, arg);
	smash_tilde(arg);

	if(IS_NULLSTR(arg)){	
		ch->printlnf("Current area builders list:'%s'", trim_string(pArea->builders));
		return false;
	}

	if(has_colour(arg)){
		ch->println("You can't use colour codes here.");
		return false;
	}

	if(is_exact_name(arg, pArea->builders)){
		ch->printlnf("Removing '%s' from area builder list.", arg);

		pArea->builders=string_remove_name(pArea->builders, arg);
		if(IS_NULLSTR(pArea->builders)){
			replace_string(pArea->builders,"none");
		}
		ch->printlnf("Area builder list now: '%s'", pArea->builders);
		return true;
	}

	// add them to the areas builder list
	if(str_len(pArea->builders)> MIL){
		ch->println("Too many names listed in builder list, remove some first.");
		ch->printlnf("Builder list now: '%s'", pArea->builders);
		return false;
	}

	ch->printlnf("Adding '%s' to area builder list.", arg);
	if(IS_NULLSTR(pArea->builders) || !str_cmp(pArea->builders,"none")){
		replace_string(pArea->builders, arg);
	}else{
		char *f=FORMATF("%s %s", pArea->builders, arg);
		replace_string(pArea->builders, f);
	}
	ch->printlnf("Area builder list now: '%s'", pArea->builders);
	return true;
}

/**************************************************************************/
bool aedit_vnum(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char lower[MSL];
    char upper[MSL];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || lower[0] == '\0'
    || !is_number( upper ) || upper[0] == '\0' )
    {
		ch->println("Syntax:  vnum [#xlower] [#xupper]");
		return false;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
		ch->println("AEdit:  Upper must be larger then lower.");
		return false;
    }
    
    if ( !check_range( atoi( lower ), atoi( upper ) ) )
    {
		ch->println("AEdit:  Range must include only this area.");
		return false;
    }

    if ( get_vnum_area( ilower ) && get_vnum_area( ilower ) != pArea )
    {
		ch->println("AEdit:  Lower vnum already assigned.");
		return false;
    }

    pArea->min_vnum = ilower;
    ch->println("Lower vnum set.");

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
		ch->println("AEdit:  Upper vnum already assigned.");
		return true;	// The lower value has been set.
    }

    pArea->max_vnum = iupper;
    ch->println("Upper vnum set.");

	sort_arealists(); // resort the vlist and area lists

    return true;
}

/**************************************************************************/
bool aedit_lvnum(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char lower[MSL];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, lower );

    if ( !is_number( lower ) || lower[0] == '\0' )
    {
		ch->println("Syntax:  min_vnum [#xlower]");
		return false;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->max_vnum ) )
    {
		ch->println("AEdit:  Value must be less than the max_vnum.");
		return false;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
		ch->println("AEdit:  Range must include only this area.");
		return false;
    }

    if ( get_vnum_area( ilower ) && get_vnum_area( ilower ) != pArea )
    {
		ch->println("AEdit:  Lower vnum already assigned.");
		return false;
    }

    ch->printlnf( "Lower vnum changed from %d to %d.", pArea->min_vnum, ilower);
    pArea->min_vnum = ilower;
	sort_arealists(); // resort the vlist and area lists
    return true;
}

/**************************************************************************/
bool aedit_uvnum(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char upper[MSL];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, upper );

    if ( !is_number( upper ) || upper[0] == '\0' )
    {
		ch->println("Syntax:  max_vnum <#xupper>");
		return false;
    }

    if ( ( ilower = pArea->min_vnum ) > ( iupper = atoi( upper ) ) )
    {
		ch->println( "AEdit:  Upper must be larger then lower." );
		return false;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
		ch->println( "AEdit:  Range must include only this area." );
		return false;
    }

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
		ch->println( "AEdit:  Upper vnum already assigned.");
		return false;
    }

    ch->printlnf( "Upper vnum changed from %d to %d.", pArea->max_vnum, iupper);
    pArea->max_vnum = iupper;
	sort_arealists(); // resort the vlist and area lists
    return true;
}

/**************************************************************************/
bool aedit_lrange(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char lower[MSL];
    char upper[MSL];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || lower[0] == '\0'
    || !is_number( upper ) || upper[0] == '\0' )
    {
        ch->println( "Syntax:  lrange <lowerlevel> <upperlevel>" );
        ch->println( "Use this command to set the recommended level ranges on the arealist." );
        ch->println( "To wipe a lrange type:  lrange -1 -1" );
        ch->println( "Notes: if a lcomment exists, this is displayed." );
        ch->println( "  if that doesn't exist... and a lrange does the lrange is displayed." );
        ch->println( "  Otherwise ?????? is displayed." );
		return false;
    }
 
    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
        ch->println( "AEdit:  Upper must be larger then lower." );
		return false;
    }

	if (ilower==-1 && iupper==-1)
	{
	    pArea->low_level = -1;
		pArea->high_level = -1;
        ch->println( "Numeric level range has been cleared on area.");
		sort_arealists(); // resort the vlist and area lists  
		return true;	
	}

    
	if (ilower<0)
	{
        ch->println( "AEdit: lowerlevel less than 0 - set to 0." );
		ilower = 0;
	}

	if (iupper<0)
	{
        ch->println( "AEdit: upperlevel less than 0 - set to 0." );
		iupper =0;
	}


	if (ilower>iupper)
	{
        ch->println( "AEdit: lowerlevel higher than upper - set to 1." );
		ilower = 1;
	}

	if (iupper>MAX_LEVEL)
	{
        ch->println( "AEdit: upperlevel higher than maxlevel - set to max_level." );
		iupper =MAX_LEVEL;
	}

    pArea->low_level = ilower;
    ch->printlnf( "Lower level set to %d.", pArea->low_level);
    pArea->high_level = iupper;
    ch->printlnf( "Upper level set to %d.", pArea->high_level);

	sort_arealists(); // resort the vlist and area lists  
    return true;
}

/**************************************************************************/
bool aedit_lcomment(char_data *ch, char *argument)
{
    AREA_DATA *pArea;
    char comment[MSL];

    EDIT_AREA(ch, pArea);

	strcpy(comment, argument);

    if ( comment[0] == '\0' )
    {
        ch->println("lcomment is the comment that is displayed in");
        ch->println("the level position on the arealist.");
        ch->println("Syntax:  lcomment [comment]");
        ch->println("Syntax:  lcomment clear  - removes any comment");
        ch->println("Notes: if a lcomment exists, this is displayed in the arealist.");
        ch->println("  if that doesn't exist... and a lrange does the lrange is displayed.");
        ch->println("  Otherwise ?????? is displayed.");
		return false;
    }


	if (!str_cmp(comment, "clear")) // clear the comment
	{
		free_string( pArea->lcomment );
        ch->println("Level comment has been cleared.");
		pArea->lcomment = '\0';
		return true;
	}

    ch->printlnf( "Level comment has been set to '%s'", comment);
	replace_string( pArea->lcomment, comment);
	return true;
}
/**************************************************************************/
bool aedit_olcflags(char_data *ch, char *argument)
{
	AREA_DATA *pArea;
	int value;


	if (!HAS_SECURITY(ch,8))
	{
		ch->println("You must have an olc security of 8 or higher to edit these flags.");
		return false;
	}

    EDIT_AREA(ch, pArea);

	if ( ( value = flag_value( olc_flags, argument) ) != NO_FLAG )
	{
		TOGGLE_BIT(pArea->olc_flags, value);

		ch->println("OLCAREA flag toggled.");
		SET_BIT( pArea->olc_flags, OLCAREA_CHANGED );
		return true;
	}
	

	ch->printlnf( "Unrecognised olcarea flag '%s'.", argument);
	ch->wraplnf("olcflags available: [`=R%s`x]", 
				flag_string( olc_flags, -1) );
	return false;
}

/**************************************************************************/
bool aedit_areaflags(char_data *ch, char *argument)
{
	AREA_DATA *pArea;
	int value;

	if (!HAS_SECURITY(ch,8))
	{
		ch->println("You must have a security of 8 or higher to change area flags.");
		return false;
	}

    EDIT_AREA(ch, pArea);

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( area_flags, argument ) ) != NO_FLAG )
		{
			pArea->area_flags ^= value;
			ch->println("Areaflag toggled.");
			return true;
		}
		ch->printlnf( "Unrecognised areaflag '%s'", argument);
	}

    ch->println("Syntax: areaflag [flag]");
	ch->wraplnf("areaflags available: [`=R%s`x]", 
				flag_string( area_flags, -1) );
	
	return false;
}
/**************************************************************************/
bool aedit_lock(char_data *ch, char *)
{
	AREA_DATA *pArea;

	if (!HAS_SECURITY(ch,9))
	{
		ch->println("You must have a security of 9 to lock or unlock an area.");
		return false;
	}

    EDIT_AREA(ch, pArea);

	if(IS_SET(pArea->area_flags,AREA_LOCKED)){
		REMOVE_BIT(pArea->area_flags,AREA_LOCKED);
		ch->println("`RAREA UNLOCKED!`x");
		logf("aedit_lock(): %s unlocked %s", ch->name, pArea->name);
	}else{
		SET_BIT(pArea->area_flags,AREA_LOCKED);
		ch->println("`RAREA LOCKED!`x");
		logf("aedit_lock(): %s locked %s", ch->name, pArea->name);
	}
	return true;
}
/**************************************************************************/
bool aedit_use_buildrestricts(char_data *ch, char *)
{
	AREA_DATA *pArea;

	if (!HAS_SECURITY(ch,9))
	{
		ch->println("You must have a security of 9 to toggle the use of build restricts.");
		return false;
	}

    EDIT_AREA(ch, pArea);

	if(IS_SET(pArea->area_flags, AREA_USE_BUILDRESTRICTS	)){
		REMOVE_BIT(pArea->area_flags,AREA_USE_BUILDRESTRICTS);
		ch->println("`RAREA BUILDRESTRICTS DISABLED!`x");
		logf("aedit_lock(): %s disabled build restricts on %s", ch->name, pArea->name);
	}else{
		SET_BIT(pArea->area_flags,AREA_USE_BUILDRESTRICTS);
		ch->println("`RAREA BUILDRESTRICTS ENABLED!`x");
		logf("aedit_lock(): %s enabled build restricts on %s", ch->name, pArea->name);
	}
	return true;
}
/**************************************************************************/
// Kal - Aug 99
bool aedit_buildrestricts(char_data *ch, char *argument)
{
	AREA_DATA *pArea;

	if (!HAS_SECURITY(ch,9))
	{
		ch->println("You must have a security of 9 to edit the build restricts of an area.");
		return false;
	}

    EDIT_AREA(ch, pArea);
	char type[MIL], name[MIL];
	
	argument=one_argument(argument, type);
	argument=one_argument(argument, name);

	if(IS_NULLSTR(name)){
		ch->println("Syntax: buildrestricts <build restrict type> <name to toggle>.");
		return false;
	}

	int index;
	if (( index = flag_value( buildrestrict_types, type)) == NO_FLAG ) {
		ch->printlnf( "Invalid build restrict type '%s'", type);
		return false;
	}

	char *restrict_type=flag_string(buildrestrict_types, index);

	char *arg=name;

	if(is_exact_name(arg, pArea->build_restricts[index])){
		ch->printlnf("Removing '%s' from area %s builder restrict.", arg, restrict_type);
		// remove them from the areas builder restrict
		pArea->build_restricts[index]=string_remove_name(pArea->build_restricts[index], arg);
		if(IS_NULLSTR(pArea->builders)){
			replace_string(pArea->build_restricts[index],"none");
		}
		ch->printlnf("Area %s builder restrict now: '%s'", 
			restrict_type, 
			pArea->build_restricts[index]);
		return true;
	}

	// add them to the areas builder restrict
	if(str_len(pArea->build_restricts[index])> MIL){
		ch->printlnf("Too many names listed in %s builder restrict list, remove some first.", restrict_type);
		ch->printlnf("%s builder restrict list now: '%s'", 
			restrict_type, 
			pArea->build_restricts[index]);
		return false;
	}

	ch->printlnf("Adding '%s' to area %s builder restrict list.", arg, restrict_type);
	if(IS_NULLSTR(pArea->build_restricts[index])){
		replace_string(pArea->build_restricts[index], FORMATF(" %s ", arg));
	}else{
		char *f=FORMATF("%s %s", pArea->build_restricts[index], arg);
		replace_string(pArea->build_restricts[index], f);
	}
	ch->printlnf("Area %s builder restrict list now: '%s'", 
		restrict_type,
		pArea->build_restricts[index]);
	return true;
}
/**************************************************************************/
bool aedit_continent( char_data *ch, char *argument )
{
	AREA_DATA *pArea;
	continent_type *cont;

	if ( !HAS_SECURITY( ch, 7 ))
	{
		ch->println( "You must have a security of 7 to set the continent name." );
		return false;
	}

    EDIT_AREA(ch, pArea);

	if( IS_NULLSTR( argument )){
		ch->println( "Syntax: continent any");
		ch->println( "Syntax: continent <name>");		
		continents_show(ch);
		return false;
	}

	if( !str_cmp(argument, "any") ){
		ch->printlnf( "Area set to any (was in '%s').", 			
			pArea->continent?pArea->continent->name:"any");
		pArea->continent = NULL;
		return true;
	}

	// look up continent
	cont = continent_lookup( argument );
	if(!cont ){		
		ch->printlnf( "Invalid Continent '%s'.", argument );
		continents_show( ch );
		return false;
	}

	ch->printlnf( "Area now resides in %s (was in '%s').", 
		cont->name,
		pArea->continent?pArea->continent->name:"any");
	pArea->continent = cont;
	return true;

}

/**************************************************************************/
// Daos - Dec. 2001
bool aedit_addecho( char_data *ch, char * argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println( "`#`WSyntax:  `Caddecho <firsthour> <secondhour> <percentage> echotext`^" );
		ch->println( "For the echotext to be displayed to all in the area the following must be meet.");
		ch->println( "IC time hour is inbetween <firsthour> and <secondhour> (inclusive)." );
		ch->println( "A random number between 1 and 100 must be less than or equal to <percentage>" );
		ch->println( "e.g. addecho 19 22 10 the wind blows silently.");
		ch->println( "     (will be displayed from 7pm to 10pm 10% of the time on the tick)" );
		ch->println( "e.g. addecho 7 5 3 the mist grows thick. ");
		ch->println( "     (will be displayed from 7am thru to 5am the next day 3% of the time on the tick)" );
		return false;
    }
    AREA_DATA *pArea;
    EDIT_AREA(ch, pArea);

	char low[MIL];
	char high[MIL];
	char percentage[MIL];
	argument=one_argument(argument, low);
	argument=one_argument(argument, high);
	argument=one_argument(argument, percentage);

	if(IS_NULLSTR(argument)){
		aedit_addecho(ch,"");
		return false;
	}
	if(!is_number(low) || !is_number(high) || !is_number(percentage)){
		ch->println("<firsthour> <lasthour> and <percentage> must all be numbers");
		aedit_addecho(ch,"");
		return false;
	}
	int ilow=atoi(low);
	int ihigh=atoi(high);
	int ipercentage=atoi(percentage);

	if(ilow<0 || ilow>23 || ihigh<0 || ihigh>23){
		ch->println("<firsthour> and <lasthour> must be between 0 and 23 (0=midnight, 23= 11pm)");
		aedit_addecho(ch,"");
		return false;
	}

	if(ipercentage<1 ||ipercentage>100){
		ch->println("<percentage> must be between 1 and 100.");
		aedit_addecho(ch,"");
		return false;
	}

	area_echo_data *ae=new_area_echo();
	replace_string(ae->echotext, argument);
	ae->firsthour=ilow;
	ae->lasthour=ihigh;
	ae->percentage=ipercentage;
	
	ae->next=pArea->echoes;
	pArea->echoes=ae;

    ch->printlnf( "AreaEcho %d(%s) %d(%s) %d%% '%s' added.", 
		ae->firsthour, convert24hourto12hour(ae->firsthour), 
		ae->lasthour, convert24hourto12hour(ae->lasthour),
		ae->percentage, ae->echotext);
	ch->println( "Use `=Cdelecho`x to remove area echoes");
    return true;
}

/**************************************************************************/
// Daos - Dec. 2001
bool aedit_delecho(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println( "`#`WSyntax:  `Cdelecho <echo#>`^" );
		ch->println( "Removes an area echo listed in olc's aedit show."); 
		ch->println( "<echo#> is the number of the echo in the list." );
		return false;
    }

    AREA_DATA *pArea;
    EDIT_AREA(ch, pArea);
	char num[MIL];

    one_argument( argument, num);

    if (IS_NULLSTR(num) || !is_number( num))
    {
		ch->println("The single parameter must be a number");
		aedit_delecho(ch, "");					
		return false;
    }	
    
	int value = atoi(num);
	
    if(value < 0){
        ch->println("Only positive area echo numbers are valid .");
        return false;
    }
	
    if( !pArea->echoes){
        ch->println("This area doesn't have any echos to remove.");
        return false;
    }
	
    area_echo_data *list;
    area_echo_data *list_next;
	int cnt = 0;

	list = pArea->echoes;
    if ( value == 0 ){
        pArea->echoes= list->next;
        free_area_echo( list );
    }else{
        while ( (list_next = list->next) && (++cnt < value ) )
			list = list_next;		
        if(list_next)
        {
			list->next = list_next->next;
			free_area_echo(list_next);
        }
        else
        {
			ch->printlnf("No area echo number %d.", value);
			return false;
        }
    }

    ch->printlnf("Area echo %d removed", value);
    return true;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
