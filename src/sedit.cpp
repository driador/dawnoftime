/**************************************************************************/
// sedit.cpp - Olc editor code dealing with dynamic editing of skill 
//             and spell parameters, Kal
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
#include "d2magsys.h"
#include "dynamics.h"

extern dynspell_type spellpairs_table[];
char *tochar_spellfunction(SPELL_FUN *psp);
sh_int spellfunctionindex_fromchar(const char *name);

/**************************************************************************/
char *category_indexlookup(int index){
	int flag;

	if(category_types[index].bit==index){
		return (category_types[index].name);
	}

	for (flag = 0; category_types[flag].name; flag++)
	{
		if(category_types[flag].bit==index){
			return (category_types[flag].name);
		}
	}

	bugf("category_indexlookup(): Invalid index %d!", index);
	return ("");
	
};
/**************************************************************************/
// returns -1 if the category can't be found
int category_lookup(char *name){
	int flag;

	// first attempt exact match 
	for ( flag = 0; category_types[flag].name!= NULL; flag++)
	{
		if (LOWER(name[0]) == LOWER(category_types[flag].name[0])
			&&  !str_cmp( name, category_types[flag].name))
            return category_types[flag].bit;
	}
	
	// now attempt a prefix match
	for ( flag = 0; category_types[flag].name!= NULL; flag++)
	{
		if (LOWER(name[0]) == LOWER(category_types[flag].name[0])
			&&  !str_prefix( name, category_types[flag].name))
			return category_types[flag].bit;
	}
	
	bugf("category_lookup(): Invalid category name '%s'!", name);
	return -1;
};
/**************************************************************************/
void do_spellskilllist( char_data *ch, char *){
	int i, j, col=0;
	int maxcat, mincat;
	char buf[MIL];
    BUFFER *output;

	if ( IS_NPC( ch )){
		ch->println( "Players only." );
		return;
	}

	// find the maximum category
	maxcat=0;
	mincat=200;
	for(i=0; i<MAX_SKILL; i++){
		if (IS_NULLSTR(skill_table[i].name)){
			continue;
		}
		maxcat=UMAX(maxcat, skill_table[i].category);
		mincat=UMIN(mincat, skill_table[i].category);
	}
	
	// output 
    output = new_buf(); 
	sprintf( buf,"`?%s`x", format_titlebar("CATEGORIES"));
	add_buf(output,buf);
	for (j=mincat;j<=maxcat;j++){
		add_buf( output,"\r\n");
		col=0;
		sprintf( buf,"`?%s`x", format_titlebar(category_indexlookup(j)));
		add_buf(output,buf);
		for(i=0; !IS_NULLSTR(skill_table[i].name); i++){
			if (skill_table[i].category!=j){
				continue;
			}

			if (!str_cmp("reserved",skill_table[i].name)){
				continue;
			}


			strcpy(buf, str_width(skill_table[i].name, 25,false));
			add_buf( output, buf);

			if (++col%3==0){
				add_buf( output,"\r\n");
			}
		}
	}
	add_buf( output,"\r\n");
	ch->sendpage(buf_string(output));
    free_buf(output);
}
/**************************************************************************/
//	Entry Point for editing a spells realms, spheres and category
//  or skills categories
void do_sedit( char_data *ch, char *argument )
{
	int		sn;
	char	arg[MIL];

	if ( IS_NPC( ch )){
			ch->println( "Players only." );
		return;
	}

	// do security checks
	if (!HAS_SECURITY(ch, 2))
	{
		ch->println( "You must have an olc security 2 or higher to use this command." );
		return;
	}

    if ( !HAS_SECURITY(ch, SEDIT_MINSECURITY))
    {
    	ch->println( "sEdit: Insufficient security to modify spell realms, spheres or categories nor skill categories." );
    	return;
    }

	if ( !IS_TRUSTED(ch, SEDIT_MINTRUST)) {
		ch->printlnf( "You must have a trust of %d or above to use this command.",
			SEDIT_MINTRUST);
		return;
	}

	if (IS_NULLSTR(argument)){
		ch->titlebar("SEDIT: SYNTAX");
		ch->println( "syntax: sedit <spell/skill_name>" );
		ch->println( "<spell/skill_name> can selected from one of the following:" );
		ch->println( "syntax: createspell 'new spell name'" );
		do_spellskilllist(ch,"");
		ch->titlebar("SEDIT: SYNTAX");
		ch->println( "syntax: sedit <spell/skill_name>" );
		ch->println( "<spell/skill_name> can selected from one of the above." );
		ch->println( "syntax: createspell 'new spell name'" );
		return;
	}
	
	// using ' codes are optional
	if(!str_infix("'",argument)){
		argument = one_argument( argument, arg );
	}else{
		strcpy(arg, argument);
	}

    if ( ( sn = skill_lookup( arg ) ) < 0)
    {
        ch->printlnf( "There is no spell/skill with the name '%s'.", arg);
        return;
    }

    ch->desc->pEdit	= &skill_table[sn];
	ch->desc->editor = ED_SPELLSKILL;
	ch->printlnf( "Editing spell/skill properties of '%s'",
		skill_table[sn].name);
	return;
}
/**************************************************************************/
bool sedit_show(char_data *ch, char *)
{
	int sn;
	bool spell;
	skill_type * pS;
	
    EDIT_SPELLSKILL(ch, pS);

	sn=skill_lookup(pS->name);
	
	spell=IS_SPELL(sn);

	if (spell){
  		ch->printlnf( "`=rSpell Name: `x%s", pS->name);

		// realms
		if (pS->realms==0){
			ch->println( "`=rRealms:`=R (none)");
		}else{		
			ch->printlnf( "`=rRealms:`=R%s`x",
				flag_string( realm_flags, pS->realms) );
		}
		// spheres
		if (pS->spheres==0){
			ch->println( "`=rSpheres:`=R (none)");
		}else{		
			ch->printlnf( "`=rSpheres:`=R%s`x",
				flag_string( sphere_flags, pS->spheres) );
		}
		// elements/seasons
		if (pS->elements==0){
			ch->println( "`=rElements/Seasons:`=R (none)");
		}else{		
			ch->printlnf( "`=rElements/Seasons:`=R%s`x",
				flag_string( element_flags, pS->elements) );
		}
		// elements/seasons
		if (pS->compositions==0){
			ch->println( "`=rCompositions:`=R (none)");
		}else{		
			ch->printlnf( "`=rCompositions:`=R%s`x",
				flag_string( composition_flags, pS->compositions ));
		}

		// Sector Restrictions
		if (pS->sect_restrict==0){
			ch->println( "`=rSector Restrictions:`=R (none)");
		}else{
			ch->printlnf( "`=rSector Restrictions:`=R%s`x",
				flag_string( sectorbit_flags, pS->sect_restrict) );
		}
		// Sector Enhancements
		if (pS->sect_enhance==0){
			ch->println( "`=rSector Enhancements:`=R (none)");
		}else{
			ch->printlnf( "`=rSector Enhancements:`=R%s`x",
				flag_string( sectorbit_flags, pS->sect_enhance) );
		}
		// Sector Dampening
		if (pS->sect_dampen==0){
			ch->println( "`=rSector Dampening:`=R (none)");
		}else{
			ch->printlnf( "`=rSector Dampening:`=R%s`x",
				flag_string( sectorbit_flags, pS->sect_dampen) );
		}
		// Spell groupings
		if (pS->spellgroup==0){
			ch->println( "`=rSpell Groups:`=R (none)");
		}else{
			ch->printlnf( "`=rSpell Groups:`=R%s`x",
				flag_string( spell_group_flags, pS->spellgroup) );
		}

		ch->printlnf( "`=rSpell `YWearoff`=r Message: `x%s", 
			IS_NULLSTR(pS->msg_off)?"`=Rnone`x":pS->msg_off);
		ch->printlnf( "`=rSpell `YObject`=r Wearoff Message: `x%s", 
			IS_NULLSTR(pS->msg_obj)?"`=Rnone`x":pS->msg_obj);
		ch->printlnf( "`=rSpell Function: `x%s", tochar_spellfunction(pS->spell_fun));
		if(pS->spell_fun){
			ch->printlnf( "`=r>>spell function flags: `S%s`x", 
				flag_string( dynspell_flags, spellpairs_table[pS->spell_function_index].flags) );
			ch->printlnf( "`=r>>target type: `S%s`x", 
				flag_string( target_types, pS->target) );
		}
	}else{
  		ch->printlnf( "`=rSkill Name: `x%s", pS->name);
	}

	SET_BIT(ch->dyn,DYN_SHOWFLAGS); // complete flags always shown 
	mxp_display_olc_flags(ch, category_types,	pS->category,	"category", "Category:");
	mxp_display_olc_flags(ch, damtype_types,	pS->damtype,	"damtype", "DamType:");
	mxp_display_olc_flags(ch, skflags_flags,	pS->flags,	"flags", "Flags:");
	mxp_display_olc_flags(ch, position_types,	pS->minimum_position,	"position",	"Position:");
	REMOVE_BIT(ch->dyn,DYN_SHOWFLAGS); 

	ch->printlnf( "`=rMana: `x%d", pS->min_mana );
	ch->printlnf( "`=rBeats: `x%d (%d second%s) - there %s %d beat%s per second.", 
		pS->beats,  
		(pS->beats/PULSE_PER_SECOND), 
		(pS->beats/PULSE_PER_SECOND==1?"":"s"),
		PULSE_PER_SECOND==1?"is":"are",
		PULSE_PER_SECOND,
		PULSE_PER_SECOND==1?"":"s");
	ch->printlnf( "`=rNoun Damage: `x%s", 
		pS->noun_damage?pS->noun_damage:"`=Rnone`x");
	if(IS_SET(pS->flags, SKFLAGS_USE_RACE_RESTRICTIONS)){
		ch->printlnf( "`=rRace restriction: `x%s `=r can get it.`x", 
			race_get_races_set_for_n_array(pS->race_restrict_n) );
	}else{
		ch->println( "`=rRace restriction: `xall races`=r can get it.`x"); 
	}
	ch->printlnf( "`SType: `x%s`S(not settable to/from spell)`x", 
		flag_string( sktype_types, pS->sktype) );
	ch->printlnf( "`=rComponent: `x%s", pS->component_based ? "Yes" : "No" );
	ch->printlnf( "`=rMSP:       `x%s", 
			IS_NULLSTR(pS->msp_sound)?"`=Rnone`x":pS->msp_sound);

	if(IS_SET(pS->flags, SKFLAGS_NEW_IMPROVE_SYSTEM))
	{
		ch->println( "`=rClass `YMax`=res:          Prac:  Learn:  `gLvl,Rating,Low%%(non settable in sedit - use class)`x");
		for(int i=0; !IS_NULLSTR(class_table[i].name);i++){
//			if(pS->maxprac_percent[i]==0 && pS->maxlearn_percent[i]==0){
//				continue;
//			}
			ch->printlnf( "  `S%-18s  `=R%3d%%    %3d%%`g     %3d,%3d,%3d%%`x", 
				capitalize(class_table[i].name),
				pS->maxprac_percent[i],
				pS->learn_scale_percent[i]==0?100:pS->learn_scale_percent[i],
				pS->skill_level[i],
				pS->rating[i],
				pS->low_percent_level[i]);
		}
	}
   
	return false;
}
/**************************************************************************/
//     uses sedit_show to display info to the character
void do_sshow( char_data *ch, char *argument )
{
    int sn;
	void * pTemp;

	if (!HAS_SECURITY(ch,1))
	{
		ch->println( "The show command is an olc command, you don't have olc permissions." );
		return;
	}

    if ( IS_NULLSTR(argument) )
    {
		ch->println(  "Syntax:  sshow <skill/spell/realm/sphere/whatever>" );
		ch->println(  "  sshow is short for sedit show" );
		return;
    }


    if ( ( sn = skill_lookup( argument ) ) < 0)
    {
        ch->printlnf( "There is no spell/skill with the name '%s'.", argument );
        return;
    }

	pTemp = ch->desc->pEdit;
    ch->desc->pEdit	= &skill_table[sn];
    sedit_show( ch, argument );
    ch->desc->pEdit = pTemp;
    return; 
}
/**************************************************************************/
bool sedit_category(char_data *ch, char *argument)
{
	skill_type * pS;
	EDIT_SPELLSKILL(ch, pS);
	return olc_generic_flag_toggle(ch, argument, "category", "category", category_types, &pS->category);
}

/**************************************************************************/
bool sedit_realm(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( "sedit_realm(): You can't set the realms for anything other than a spell.");
			return false;
		}
	}

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( realm_flags, argument ) ) != NO_FLAG )
		{
			pS->realms^= value;
			ch->println( "Realm toggled." );
			return true;
		}
		ch->printlnf( "Unrecognized realm '%s'", argument);
	}

    ch->println( "Syntax: realm [flag]" );
	ch->wraplnf( "Realms available: [`=R%s`x]",
				flag_string( realm_flags, -1) );
	
	return false;
}
/**************************************************************************/
bool sedit_sphere(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( "sedit_sphere(): You can't set the spheres for anything other than a spell." );
			return false;
		}
	}

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( sphere_flags, argument ) ) != NO_FLAG )
		{
			pS->spheres^= value;
			ch->println( "Sphere toggled." );
			return true;
		}
		ch->printlnf( "Unrecognized sphere '%s'", argument);
	}

    ch->println( "Syntax: sphere [flag]" );
	ch->wraplnf( "Spheres available: [`=R%s`x]",
				flag_string( sphere_flags, -1) );
    
	return false;
}
/**************************************************************************/
bool sedit_element(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( "sedit_element(): You can't set the elements/seasons for anything other than a spell.");
			return false;
		}
	}

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( element_flags, argument ) ) != NO_FLAG ){
			pS->elements^= value;
			ch->println( "Element toggled." );
			return true;
		}
		ch->printlnf( "Unrecognized element/season '%s'", argument);
	}

    ch->println( "Syntax: element [flag]" );
	ch->wraplnf( "Elements/seasons available: [`=R%s`x]",
				flag_string( element_flags, -1) );
    return false;
}
/**************************************************************************/
bool sedit_composition(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( "sedit_composition(): You can't set the compositions for anything other than a spell." );
			return false;
		}
	}

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( composition_flags, argument ) ) != NO_FLAG )
		{
			pS->compositions ^= value;
			ch->println( "Composition toggled." );
			return true;
		}
		ch->printlnf( "Unrecognized composition '%s'", argument);
	}

    ch->println( "Syntax: composition [flag]" );
	ch->wraplnf( "Compositions available: [`=R%s`x]",
				flag_string( composition_flags, -1) );
    
	return false;
}

/**************************************************************************/
void wideshow_flag_cmds( char_data *ch, const struct flag_type *flag_table )
{
	char buf  [ MSL ];
	char buf1 [ MSL ];
	int  flag;
	int  col;

	buf1[0] = '\0';
	col = 0;
	for (flag = 0; flag_table[flag].name != NULL; flag++)
	{
		if ( flag_table[flag].settable )
		{
			sprintf( buf, "%-39.38s", flag_table[flag].name );
			strcat( buf1, buf );
			if ( ++col % 2 == 0 )
				strcat( buf1, "\r\n" );
		}
    }

	if ( col % 2 != 0 )
		strcat( buf1, "\r\n" );

	ch->print( buf1 );
	return;
}
/**************************************************************************/
bool sedit_flags(char_data *ch, char *argument)
{
	skill_type * pS;
	EDIT_SPELLSKILL(ch, pS);
	return olc_generic_flag_toggle(ch, argument, "flags", "flags", skflags_flags, &pS->flags);
}
/**************************************************************************/
bool sedit_noundamage( char_data *ch, char *argument )
{
	skill_type * pS;

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  noundamage [string]");
		return false;
	}
	EDIT_SPELLSKILL(ch, pS);
	
    free_string( pS->noun_damage);
    pS->noun_damage = str_dup( argument );
    ch->printlnf( "Noun damage set to '%s'", pS->noun_damage);
    return true;
}
/**************************************************************************/
bool sedit_wearoff( char_data *ch, char *argument )
{
	skill_type * pS;

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  `=Cwearoff [string]`x");
		ch->println( "    or:  `=Cwearoff -`x for no wear off message");
		ch->println( "e.g `=Cwearoff You feel less righteous.`x");
		return false;
	}
	EDIT_SPELLSKILL(ch, pS);
	
    free_string( pS->msg_off);
	if(strcmp(argument,"-")){
		pS->msg_off = str_dup( argument );
	    ch->printlnf( "Wearoff message set to '%s'", pS->msg_off);
	}else{
		pS->msg_off = str_dup("");
	    ch->println("Wearoff message cleared.");
	}
    return true;
}
/**************************************************************************/
bool sedit_objectwearoff( char_data *ch, char *argument )
{
	skill_type * pS;

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  `=Cobjectwearoff [string]`x");
		ch->println( "    or:  `=Cobjectwearoff -`x for no wear off message");
		ch->println( "e.g `=Cobjectwearoff $p's holy aura fades.`x");
		return false;
	}
	EDIT_SPELLSKILL(ch, pS);

    free_string( pS->msg_obj);
	
	if(strcmp(argument,"-")){
		pS->msg_obj = str_dup( argument );
	    ch->printlnf( "Object wearoff message set to '%s'", pS->msg_obj);
	}else{
		pS->msg_obj = str_dup("");
	    ch->println( "Object wearoff message cleared.");
	}
    return true;
}
/**************************************************************************/
bool sedit_position(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;

    if ( IS_NULLSTR(argument))
    {
		ch->println( " Syntax:  position [position]" );
		ch->println( "   `SThis position is the minimum position the caster\r\n"
					  "   must be in for casting spells.`x" );
		ch->println( " Type '? position' for a list of positions." );
		ch->println( " examples: position sit" );
		ch->println( "           position rest\r\n" );	// extra lf
		ch->println( " Selectable positions can be one of the following:" );
		show_help(ch, "position");
		return false;
	}

	value = flag_value( position_types, argument);
	if(value == NO_FLAG ){
		ch->printlnf( "Invalid position '%s'", argument);
		sedit_position(ch,""); // redisplay the help
		return false;
	}

	EDIT_SPELLSKILL(ch, pS);
	
    pS->minimum_position = value;
    ch->printlnf( "Minimum casting position set to '%s'", 
		position_table[value].name);
	return true;
}
/**************************************************************************/
bool sedit_mana(char_data *ch, char *argument)
{
	skill_type * pS;

    if ( IS_NULLSTR(argument))
    {
		ch->println( " Syntax:  mana [amount_of_mana]" );
		ch->println( "`SThis mana amount is how much is used up from "
			"the default successful cast." );
		return false;
	}

	
	if(!is_number(argument)){
		ch->println( "The mana value must be numeric.");
		sedit_mana(ch,""); // redisplay the help
		return false;
	}

	EDIT_SPELLSKILL(ch, pS);
	
    pS->min_mana= atoi(argument);
    ch->printlnf( "Mana set to '%d'.", pS->min_mana);
	return true;
}
/**************************************************************************/
bool sedit_beats(char_data *ch, char *argument)
{
	skill_type * pS;

    if ( IS_NULLSTR(argument))
    {
		ch->println( " Syntax:  beats [beats_worth_of_lag]" );
		ch->printlnf( "`SThis beats amount is how much lag the player gets when using the skill\r\n"
					  "or spell... There are %d beats per second.", PULSE_PER_SECOND);
		return false;
	}
	
	if(!is_number(argument)){
		ch->println( "The beats value must be numeric." );
		sedit_beats(ch,""); // redisplay the help
		return false;
	}

	EDIT_SPELLSKILL(ch, pS);
	
	pS->beats= atoi(argument);
	ch->printlnf(  "Beats set to '%d'", pS->beats);
	return true;
}
/**************************************************************************/
bool sedit_damtype(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "syntax: damtype <damagetype>" );
		ch->println( "        from one of the following." );
		wideshow_flag_cmds( ch, (const struct flag_type *)damtype_types);
		return false;
    }

	if(pS->spell_function_index<0){
		ch->println( "Note: not much point in setting the damtype since their is no spell function." );
	}else if(!IS_SET(spellpairs_table[pS->spell_function_index].flags,SPFUNC_DAMTYPE)){
		ch->println( "`RWARNING: `xThe spell function for this spell hasnt been marked as a\r\n"
			"spell function that supports a dynamic damtype." );
	}

	value = flag_value( damtype_types, argument);
	if(value==NO_FLAG){
		ch->printlnf(  "'%s' is not a recognised damtype.", argument);
		sedit_damtype(ch,"");
		return false;	
	}

	if (value==pS->damtype){
		ch->printlnf( "No change in damtype since it was already set to %s.",
			flag_string( damtype_types, pS->damtype));
		return false;
	}

	if (value>=0){
		ch->printlnf( "Damtype of '%s' changed from '%s' to '%s'.",
			pS->name, 
			flag_string( damtype_types, pS->damtype),
			flag_string( damtype_types, value));
		pS->damtype=value;
		return true;
	}
    return false;
}
/**************************************************************************/
bool sedit_spellfunc(char_data *ch, char *argument)
{
	skill_type * pS;
	sh_int spellfunc_index;


    EDIT_SPELLSKILL(ch, pS);
	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( " sedit_spellfunc(): You can't set the spell function for anything other than a spell." );
			return false;
		}
	}

    if ( IS_NULLSTR(argument))
    {
		ch->println( " Syntax:  spellfunc <name of a spell function>" );
		ch->println( " note: use funclist for all list of functions." );
		ch->println( "	      or funclist <spell function name> for a list of\r\n"
					 "spells using a certain function." );
		return false;
	}

	spellfunc_index=spellfunctionindex_fromchar(argument);
	if(spellfunc_index<0){
		ch->printlnf( "Couldn't find a spell function '%s'... try another.",
			argument);
		sedit_spellfunc(ch,"");
		return false;
	}

	if(pS->spell_function_index==spellfunc_index){
		ch->printlnf( "The spell function for '%s' is already set to %s.",
			pS->name, tochar_spellfunction(pS->spell_fun));
		return false;
	}

    ch->printlnf( "Spell function for '%s' changed from %s to %s.", 
		pS->name, spellpairs_table[pS->spell_function_index].name,
		spellpairs_table[spellfunc_index].name);

	spellpairs_table[pS->spell_function_index].count--;
	pS->spell_fun=spellpairs_table[spellfunc_index].psp;
	pS->target=spellpairs_table[spellfunc_index].target;
	pS->spell_function_index=spellfunc_index;
	spellpairs_table[pS->spell_function_index].count++;
	return true;
}
/**************************************************************************/
bool sedit_funclist(char_data *ch, char *argument)
{
	sh_int sn, spellfunc_index;
	int count, mincount, maxcount, col;
	char buf[MSL];

    BUFFER *output;

    if ( IS_NULLSTR(argument)){
		mincount= 1;
		maxcount=-1;
		// prescan the table to get maxcount and mincount values
		for ( sn = 0; !IS_NULLSTR(spellpairs_table[sn].name); sn++ )
		{
			mincount=spellpairs_table[sn].count<mincount?spellpairs_table[sn].count:mincount;
			maxcount=spellpairs_table[sn].count>maxcount?spellpairs_table[sn].count:maxcount;
		}
		output = new_buf();
		
		// display all spells
		col=0;
		for (count=mincount; count<=maxcount; count++){	
			for ( sn = 0; !IS_NULLSTR(spellpairs_table[sn].name); sn++ )
			{
				if(spellpairs_table[sn].count==count)
				{
					sprintf(buf,"    %-27.27s %3d ", 
						spellpairs_table[sn].name, spellpairs_table[sn].count);
					++col%=2;
					add_buf( output, buf);
					if(col==0){
						add_buf( output, "\r\n");
					}
				}
			}
		}
		if(col==1){
			add_buf( output, "\r\n");
		}
		
		add_buf( output, "Note: you can use funclist to show which spells use a spell function,\r\n"
			"type funclist <spell function name> to use.\r\n");
		// send the buffer 
		ch->sendpage(buf_string(output));
		free_buf(output);
	}else{
		count=0;
		// do a lookup on the spellfunction and show what uses it
		spellfunc_index=spellfunctionindex_fromchar(argument);
		if(spellfunc_index<0){
			ch->println( "syntax: funclist <spell function name>  - to list all spells which use it." );
			ch->println( "syntax: funclist     - to list all spell functions and their usage counts.\r\n" );	// extra lf
			ch->printlnf( "Couldn't find a spell function '%s'... try another.", argument);
			return false;
		}

		output = new_buf();
		sprintf(buf,"Spell function '%s' is used by:\r\n", spellpairs_table[spellfunc_index].name);
		add_buf( output, buf);
		for ( sn = FIRST_SPELL; sn <= LAST_SPELL; sn++ )
		{
			if(skill_table[sn].spell_function_index==spellfunc_index){
				count++;
				sprintf(buf,"  %s\r\n", skill_table[sn].name);
				add_buf( output, buf);
			}
		}

		if(count==0){
			ch->printlnf( "Spell function '%s' is currently not used by any spells.", 
				spellpairs_table[spellfunc_index].name);
		}else{
			sprintf(buf,"%d entr%s total.\r\n", count, count==1?"y":"ies");
			add_buf( output, buf);
			// send the buffer 
			ch->sendpage(buf_string(output));
		}
		free_buf(output);
	}
	return false;
}
/**************************************************************************/
// creates new spells based off existing ones
void do_createspell(char_data *ch, char *argument)
{
	sh_int sn;
	char arg[MIL];

	if ( !HAS_SECURITY(ch, SEDIT_MINSECURITY)){
    	ch->println( "sEdit: Insufficient security to create spells.");
    	return;
    }

    if ( IS_NULLSTR(argument)){
		ch->println( " Syntax:  createspell <name of new spell>" );
		return;
	}

	// using ' codes are optional
	if(!str_infix("'",argument)){
		argument = one_argument( argument, arg );
	}else{
		strcpy(arg, argument);
	}

	if(!str_infix("'",arg)){
		ch->println( "You can't have a single quote (') symbol as part of a skill/spell name.");
		return;
	}

	// check the length of the argument - must be longer than 3 characters
	// and have no colour codes in it
	if(c_str_len(arg)==-1){
		ch->println( "The name of the spell can NOT contain ``1 characters." );
		return;
	}
	if(c_str_len(arg)<=3){
		ch->println( "The name of the spell must be longer than 3 characters." );
		return;
	}
	// check for invalid spellnames
	if(is_name(arg,"reserved")){
		ch->println( "Invalid spell name." );
		return;
	}

	// next find where we are adding the new spell 
	sn=skill_lookup("reserved");

	// check that there was a reserved space to add our new spell on
	if(sn==-1){
		ch->wrapln( "There is no more space to add another spell right now, "
			"(up to 10 can be added between reboots), try again after a hotreboot." );
		return;
	}

	// check their isn't already a spell/skill called that
	if(skill_exact_lookup(arg)>=0){
		ch->printlnf( "There already exists some skill/spell/realm/sphere/... with a name of '%s'.", 
			arg);
		return;
	}


	// create the spell entry
	{
		static skill_type skill_zero;
		skill_zero.spell_fun=spell_null;
		
		free_string(skill_table[sn].name);
		skill_table[sn]=skill_zero;
		skill_table[sn].name= str_dup(lowercase(arg));
		skill_table[sn].sktype=SKTYPE_SPELL;
		LAST_SPELL++;
	}

	ch->printlnf( "Spell '%s' created!", arg);
	{
		int clss_no;
        for (clss_no = 0; class_table[clss_no].name; clss_no++)
        {
			skill_table[sn].skill_level[clss_no]= LEVEL_IMMORTAL;
			skill_table[sn].rating[clss_no]= 1;
            skill_table[sn].low_percent_level[clss_no] =1;
        }
	}
	do_sedit(ch, arg);

}
/**************************************************************************/
bool sedit_rename( char_data *ch, char *argument )
{
	skill_type * pS;
	char arg[MIL];

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  rename [string]" );
		ch->println( " sets the name of the skill/spell to a new name." );
		ch->println( " You must have the RENAMEABLE flag set... to prevent accidental renaming." );
		return false;
	}
	EDIT_SPELLSKILL(ch, pS);

	if(!IS_SET(pS->flags, SKFLAGS_RENAMABLE)){
	    ch->println( "Renameable flag not set, set that first!" );
		sedit_rename(ch,"");
		return false;
	}

	// using ' codes are optional
	if(!str_infix("'",argument)){
		argument = one_argument( argument, arg );
	}else{
		strcpy(arg, argument);
	}

	if(!str_infix("'",arg)){
		ch->println( "You can't have a single quote (') symbol as part of a skill/spell name.");
		return false;
	}
	
    ch->printlnf( "Name of skill/spell changed from '%s' to '%s'.", 
		pS->name, lowercase(arg));
    replace_string(pS->name,lowercase(arg));
    return true;
}
/**************************************************************************/
bool sedit_component(char_data *ch, char *)
{
	skill_type *pS;

	EDIT_SPELLSKILL(ch, pS);
	
	if ( pS->component_based == false ) {
		pS->component_based = true;
		ch->println( "Spell is now marked as needing a component to be cast." );
	} else {
		pS->component_based = false;
		ch->println( "Spell is now marked as `YNOT`x needing a component to be cast." );
	}
	return true;
}
/**************************************************************************/
bool sedit_mspsound(char_data *ch, char *argument)
{
	skill_type * pS;
	
    EDIT_SPELLSKILL(ch, pS);

	if ( IS_NULLSTR( argument )) {
		ch->println( "Syntax:  `=Cmspsound [string]`x");
		ch->println( "         `=Cmspsound -`x clears the value (include the -)\r\n"); // extra lf
		ch->println( " Associates the .wav [string] to appropriate skill/spell." );
		ch->println( " Will only accept existing filenames residing in the msp dir." );
		return false;
	}

	free_string( pS->msp_sound );

	if(strcmp(argument,"-"))
	{
		int sn=skill_lookup(pS->name);		
		char full_filename[MSL];
		if(IS_SPELL(sn)){
			sprintf(full_filename,MSP_DIR MSP_SPELLS_DIR "%s",
				argument);
		}else{
			sprintf(full_filename,MSP_DIR MSP_SKILLS_DIR "%s",
				argument);		
		}

		if ( file_exists( full_filename))
		{
			replace_string(pS->msp_sound, argument);
		    ch->printlnf( "MSP Sound set successfully to %s.", full_filename);
			return true;
		} else {
			ch->printlnf( "MSP file '%s' not found.", full_filename);
			return false;
		}
	}else{
		replace_string(pS->msp_sound, "");
	    ch->println( "MSP Sound cleared.");
	}
    return true;
}
/**************************************************************************/
bool sedit_spell_group(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( "sedit_spellgroup(): You can't set groups "
				"for anything other than a spell." );
			return false;
		}
	}

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( spell_group_flags, argument ) ) != NO_FLAG )
		{
			pS->spellgroup^= value;
			ch->println( "Group toggled." );
			return true;
		}
		ch->printlnf( "Unrecognized group '%s'.", argument);
	}

    ch->println( "Syntax: spellgroup [flag]" );
	ch->wraplnf( "Groups available: [`=R%s`x]",
				flag_string( spell_group_flags, -1) );
	
	return false;
}
/**************************************************************************/
bool sedit_sect_restrict(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( "sedit_sect_restrict(): You can't set restrictions "
				"for anything other than a spell.");
			return false;
		}
	}

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( sectorbit_flags, argument ) ) != NO_FLAG )
		{
			pS->sect_restrict^= value;
			ch->println( "Restriction toggled." );
			return true;
		}
		ch->printlnf( "Unrecognized sector '%s'", argument);
	}

    ch->println( "Syntax: restrict [sector]" );
	ch->wraplnf( "Sectors available: [`=R%s`x]",
				flag_string( sectorbit_flags, -1) );
	
	return false;
}
/**************************************************************************/
bool sedit_sect_enhance(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( "sedit_sect_enhance(): You can't set enhancements "
				"for anything other than a spell.");
			return false;
		}
	}

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( sectorbit_flags, argument ) ) != NO_FLAG )
		{
			pS->sect_enhance^= value;
			ch->println( "Enhancement toggled." );
			return true;
		}
		ch->printlnf( "Unrecognized sector '%s'", argument);
	}

    ch->println( "Syntax: enhance [sector]" );
	ch->wraplnf( "Sectors available: [`=R%s`x]",
				flag_string( sectorbit_flags, -1) );
	
	return false;
}
/**************************************************************************/
bool sedit_sect_dampen(char_data *ch, char *argument)
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

	// make sure it is a spell
	{
		int sncheck;
		sncheck=skill_lookup(pS->name);
		if(!IS_SPELL(sncheck)){
			ch->wrapln( "sedit_sect_dampen(): You can't set dampening "
				"for anything other than a spell." );
			return false;
		}
	}

    if ( !IS_NULLSTR(argument))
    {	
		if ( ( value = flag_value( sectorbit_flags, argument ) ) != NO_FLAG )
		{
			pS->sect_dampen^= value;
			ch->println( "Dampening toggled." );
			return true;
		}
		ch->printlnf( "Unrecognized sector '%s'", argument);
	}

    ch->println( "Syntax: dampen [sector]" );
	ch->wraplnf( "Sectors available: [`=R%s`x]",
				flag_string( sectorbit_flags, -1) );
	
	return false;
}
/**************************************************************************/
bool sedit_max(char_data *ch, char *argument)
{
	skill_type * pS;

    if ( IS_NULLSTR(argument))
    {
		ch->println( " Syntax:  max <class> <prac> <learn>");
		ch->wrapln( "`SThis command sets the maximum ability percentage a particular class can attain in "
					"the skill/spell by both practicing and with use.  This can only be set on skills that "
					"use the new improvement system.");
		return false;
	}

	EDIT_SPELLSKILL(ch, pS);

	if(!IS_SET(pS->flags, SKFLAGS_NEW_IMPROVE_SYSTEM))
	{
		ch->wrapln( "This command can only be used on skills that have the new_improve_system flag set.");
		sedit_max(ch,"");
		return false;
	}

	char class_name[MIL];
	argument=one_argument( argument, class_name);
	int class_index=class_lookup(class_name);
	
	if(class_index<0){
		ch->printlnf( "No such class as '%s'",class_name);
		sedit_max(ch,"");
		return false;
	}

	char prac[MIL];
	char learn[MIL];
	argument=one_argument( argument, prac);
	argument=one_argument( argument, learn);
	// check the prac argument
	if(!is_number(prac)){
		ch->println( "The second argument of sedit_max is the practice amount, must be a number." );
		sedit_max(ch,"");
		return false;
	}
	int prac_val=atoi(prac);
	if(prac_val<0 || prac_val>100){
		ch->println( "The second argument of sedit_max is the practice amount, must be a number between 0 and 100." );
		sedit_max(ch,"");
		return false;
	}

	// check the learn argument
	if(!is_number(learn)){
		ch->println( "The third argument of sedit_max is the learn amount, must be a number." );
		sedit_max(ch,"");
		return false;
	}
	int learn_val=atoi(learn);
	if(learn_val<0 || learn_val>100){
		ch->println( "The second argument of sedit_max is the learn amount, must be a number between 0 and 100." );
		sedit_max(ch,"");
		return false;
	}

	
    
    ch->printlnf( "%s maxprac set to %d (was %d), learn scale set to %d (was %d).", 
		class_table[class_index].name, 
		prac_val,
		pS->learn_scale_percent[class_index],
		learn_val,
		pS->learn_scale_percent[class_index]);

	pS->maxprac_percent[class_index]=prac_val;
	pS->learn_scale_percent[class_index]=learn_val;

	return true;
}

/**************************************************************************/
void do_racelist( char_data *ch, char *arg);
/**************************************************************************/
bool sedit_racerestrict( char_data *ch, char *argument )
{
	skill_type * pS;
	int value;
	
    EDIT_SPELLSKILL(ch, pS);

    if ( !IS_NULLSTR(argument))
    {		
		value = race_lookup(argument);
		if (value>=0)
		{
			TOGGLE_BITn(pS->race_restrict_n, value);
			ch->printlnf( "Race '%s' toggled.", race_table[value]->name );
			if(!IS_NULLSTR(race_get_races_set_for_n_array(pS->race_restrict_n))){
				SET_BIT(pS->flags, SKFLAGS_USE_RACE_RESTRICTIONS);
			}else{
				REMOVE_BIT(pS->flags, SKFLAGS_USE_RACE_RESTRICTIONS);
			}

			return true;
		}
		ch->printlnf( "Unknown race '%s'.", argument);
	}

    ch->println( "Syntax: racerestrict [race]" );
	do_racelist(ch, "");
	return false;
}

/**************************************************************************/
bool sedit_type(char_data *ch, char *argument)
{
	skill_type * pS;
	int oldvalue;
	
    EDIT_SPELLSKILL(ch, pS);

	if(pS->sktype==SKTYPE_SPELL){
		ch->printlnf("%s is already set as a spell, and therefore its type can't be changed.",
			pS->name);			
		return false;
	}

	oldvalue=pS->sktype;
	bool result=olc_generic_flag_toggle(ch, argument, "type", "type", sktype_types, &pS->sktype);

	// just incase it was accidentally set
	if(pS->sktype==SKTYPE_SPELL){
		ch->printlnf("%s can't be set to a spell, setting it back to its original value!",pS->name);			
		pS->sktype=oldvalue;
		return false;
	}

    return result;	
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


