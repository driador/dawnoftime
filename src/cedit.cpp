/**************************************************************************/
// cedit.cpp - olc class editor
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
#include "dynamics.h"
/**************************************************************************/
// prototypes
void do_classlist( char_data *ch, char *);
/**************************************************************************/
//	Entry Point for editing classes
void do_classedit( char_data *ch, char *argument )
{
	int			i;
	char		arg[MSL];

	if ( IS_NPC( ch )){
		ch->println("Players only." );
		return;
	}

	// do security checks
	if (!HAS_SECURITY(ch, 2))
	{
		ch->println( "You must have an olc security 2 or higher to use this command." );
		return;
	}

    if ( !HAS_SECURITY(ch, CLASSEDIT_MINSECURITY))
    {
    	ch->println( "ClassEdit: Insufficient security to modify or create classes." );
    	return;
    }

	if ( !IS_TRUSTED(ch, CLASSEDIT_MINTRUST)) {
		ch->printlnf( "You must have a trust of %d or above "
			"to use this command.", CLASSEDIT_MINTRUST);
		return;
	}

	if (IS_NULLSTR(argument)){
		ch->titlebar("CLASSEDIT: SYNTAX");
		ch->println( "syntax: classedit <class_name>" );
		ch->println("        classedit create <new_class_name>\r\n" );		// extra LF

		ch->println( "<class_name> can selected from one of the following:" );
		do_classlist(ch,"");
		return;
	}
	
	argument = one_argument( argument, arg );
	if ( !str_cmp( arg, "create" )){
		argument = one_argument( argument, arg ); // get the new name
		if(!str_infix(" ", arg)){
			ch->println("The name of a class can't contain any spaces.");
			return;
		}
		if(class_exact_lookup(arg)>=0){
			ch->printlnf("There already exists a class called '%s'.", arg);
			return;
		}
		
		ch->printlnf("Creating new class called '%s'.", arg);
		create_class(arg);
		do_classedit(ch,arg);
		return;
	}

	i = class_lookup(arg);
	if (i<0){
		ch->printlnf( "Couldn't find class '%s'...\r\nSelectable classes:", arg);
		do_classlist(ch,"");
		return;
    }

    ch->desc->pEdit	= &class_table[i];
	ch->desc->editor = ED_CLASS;
	ch->printlnf( "Editing class '%s'", class_table[i].name);
	return;
}
/**************************************************************************/
void do_classlist( char_data *ch, char *)
{
	int i;
	
	ch->titlebar("CLASSES");

	if(GAMESETTING(GAMESET_REMORT_SUPPORTED)){
		for ( i= 0; !IS_NULLSTR(class_table[i].name); i++)
		{
			ch->printf( " %s%-14s [%d]`x", 
				(class_table[i].creation_selectable?"`W":"`S"),
				class_table[i].name,
				class_table[i].remort_number );
			if ( ( i% 4 ) == 3 ){
				ch->print( "\r\n" );
			}
		}
	}else{
		for ( i= 0; !IS_NULLSTR(class_table[i].name); i++)
		{
			ch->printf( " %s%-19s`x", 
				(class_table[i].creation_selectable?"`W":"`S"),			
				class_table[i].name);
			if ( ( i% 4 ) == 3 ){
				ch->print( "\r\n" );
			}
		}
	}
	ch->println( "`x\r\n`SSilver classes can't be selected in creation.`x");
}
/**************************************************************************/
bool classedit_rename(char_data *ch, char *argument)
{
	class_type * pC;
	

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  rename <string>" );
		return false;
    }

    EDIT_CLASS(ch, pC);

    ch->wraplnf("Class name changed from '%s' to '%s'.", pC->name, argument);
	replace_string(pC->name, argument);
	ch->println("NOTES: The skill table and class files should be resaved!");
	ch->println("       It is recommended changing the name on classes that arent currently played.");
	ch->println("       These changes take affect immediately... any players currently logged in as this class will get the new name.");
	ch->println("       Any players not currently logged will have problems connecting as the old name.");


    return true;
}
/**************************************************************************/
bool classedit_show(char_data *ch, char *)
{
	race_data *race;
	class_type * pC;
	
    EDIT_CLASS(ch, pC);
	SET_BIT(ch->dyn,DYN_SHOWFLAGS);

  	ch->printlnf( "`=rClass Name: `x%-10s `=rShortname:  `x%s     `=rSpinfo_letter:`x%s",
		capitalize(pC->name), pC->short_name, pC->spinfo_letter);

	ch->printlnf("`=rHPMinTrain: `x%-5d      `=rHPMaxTrain: `x%d",	pC->hp_min, pC->hp_max);
	ch->printlnf("`=rRecall:     `x%-5d      `=rMorgue:     `x%-5d      `=rRemort:     `x%d ",	
		pC->recall, pC->morgue, pC->remort_number);
	
	if(IS_NULLSTR(pC->remort_to_classes)){
		ch->println( "`=rRemort to classes: `=Rall");
	}else{
		ch->printlnf("`=rRemort to classes:`=R %s`x", pC->remort_to_classes);
	}

	ch->printlnf("`=rPrimeStat 0:`x%s",	capitalize(stat_flags[pC->attr_prime[0]].name));
	ch->printlnf("`=rPrimeStat 1:`x%s",	capitalize(stat_flags[pC->attr_prime[1]].name));

	ch->printlnf("`=rSkill_adept:`x%d",	pC->skill_adept);

	ch->printlnf("`=rTHAC0:`x      %2d  (modifier To Hit Armour Class at level  0)",	pC->thac0_00);
	ch->printlnf("`=rTHAC32:`x     %2d  (modifier To Hit Armour Class at level 32)",	pC->thac0_32);
	ch->println( "`SA THAC modifier is calculated for any level by interpolating the two above values.");

	ch->printlnf("`=rFullManaGain:`x %s  `S(if false, class gets half gains mana each level)`x",	pC->fMana?"True":"False");
	ch->printlnf("`=rDefault_group:`x'%s' `S(group when no customisation is choosen)`x",	
		pC->default_group);

	ch->printlnf( "`=rCreation Selectable:`x %s",pC->creation_selectable?"True":"False");
	ch->wrapln("`SNote: If the base xp for a particular race is below 1000, it wont be "
		"selectable when that race is choosen.`x");

	if(IS_NULLSTR(pC->newbie_prac_location_hint)){
		ch->println( "`=rNewbie_prac_location_hint: `=Rnone");
	}else{
		ch->println( "`=rNewbie_prac_location_hint:`=R");
		ch->print( pC->newbie_prac_location_hint);
		ch->print( "`x");
	}

	if(IS_NULLSTR(pC->newbie_train_location_hint)){
		ch->println( "`=rNewbie_train_location_hint: `=Rnone");
	}else{
		ch->println( "`=rNewbie_train_location_hint:`=R");
		ch->print( pC->newbie_train_location_hint);
		ch->print( "`x");
	}

	if(pC->pendant_vnum){		
		obj_index_data *pendant_obj_index=get_obj_index(pC->pendant_vnum);
		if(pendant_obj_index){
			// check the item type
			if(pendant_obj_index->item_type == ITEM_PENDANT){
				ch->printlnf( "`=rPendant Object Vnum: `x%-5d `=r(`=R%s`=r)",
					pC->pendant_vnum,
					pendant_obj_index->short_descr);
				{	
					ROOM_INDEX_DATA *pr=get_room_index(pendant_obj_index->value[0]);
					if(pr){
						ch->printlnf( "`=r---------Recalls to: `=R%s`=r (room `x%d`=r) [%s%s`=r]`x",
							pr->name, 
							pendant_obj_index->value[0], 
							colour_table[(pr->area->vnum%14)+1].code,
							pr->area->short_name);
					}else{
						ch->printlnf( "    `=rRecall to vnum:  [%d] `=X(UNFOUND)`x",	
							pendant_obj_index->value[0]);
					}
				}
			}else{
				ch->printlnf( "`=rPendant Object Vnum: `=X%-5d `=r(`=R%s`=r) Type: `=X%s!!! - wont be used!`x",
					pC->pendant_vnum,
					pendant_obj_index->short_descr,
					flag_string( item_types, pendant_obj_index->item_type ));

			}
		}else{
			// handle if it is not found
			ch->printlnf( "`=rPendant Object Vnum: `=X%-5d  (UNFOUND!!!)`x",
				pC->pendant_vnum);
		}
	}else{
		ch->println( "`=rPendant Object Vnum: `x0 `=r(unset)`x");

	}

	// show the newbie map vnum
	if(pC->newbie_map_vnum){
		obj_index_data *newbiemap_obj_index=get_obj_index(pC->newbie_map_vnum);
		if(newbiemap_obj_index){
			// check the item type
			if(newbiemap_obj_index->item_type == ITEM_MAP){
				ch->printlnf( "`=rNewbieMap Object Vnum: `x%-5d `=r(`=R%s`=r)",
					pC->newbie_map_vnum,
					newbiemap_obj_index->short_descr);
			}else{
				ch->printlnf( "`=rNewbieMap Object Vnum: `=X%-5d `=r(`=R%s`=r) Type: `=X%s!!! - wont be used!`x",
					pC->newbie_map_vnum,
					newbiemap_obj_index->short_descr,
					flag_string( item_types, newbiemap_obj_index->item_type ));

			}
		}else{
			// handle if it is not found
			ch->printlnf( "`=rNewbieMap Object Vnum: `=X%-5d  (UNFOUND!!!)`x",
				pC->newbie_map_vnum);
		}
	}else{
		ch->println( "`=rNewbieMap Object Vnum: `x0 `=r(unset)`x");
	}

	mxp_display_olc_flags(ch, castnames_types,	pC->class_cast_type,	"casttype",	"Casttype:");

	mxp_display_olc_flags(ch, classflag_flags,	pC->flags,	"flags",	"Selected Flags:");

	int classindex=class_exact_lookup(pC->name);
	assert(classindex>=0);
	ch->println("`=r==`=RClass Base XP ratings for creation selectable races`=r==");
	int count=0;
	for( race=race_list; race; race=race->next)
	{	
		if(race->creation_selectable()){
			ch->printf(" `=r%-17s `x%3d  ", 
				capitalize(race->name),
				race->class_exp[classindex]);
			if(++count%3==0){ ch->println(""); } // column code
		}
	}
	if(count%3!=0){ ch->println(""); } 
	ch->println("`S(Edit these and other races in raceedit)`x");

	ch->printlnf("Use `=C%s`x to edit class poses.", mxp_create_send(ch, "showpose"));
	
	REMOVE_BIT(ch->dyn,DYN_SHOWFLAGS);
    return false;
}
/**************************************************************************/
bool classedit_showpose(char_data *ch, char *)
{
	class_type * pC;
    EDIT_CLASS(ch, pC);

	ch->titlebar("CLASS POSES");
	for(int i=0; i<MAX_LEVEL; i++){
		ch->printlnf("[%3d] %s`x", i, pC->pose_self[i]);
		ch->printlnf("<%3d> %s`x", i, pC->pose_others[i]);
	}
	ch->println("[x] = self, <x> = others");
	ch->println("use 'setpose <number> self text' and 'setpose <number> others text'");
	ch->titlebar("");
	return false;
}
/**************************************************************************/
bool classedit_setpose(char_data *ch, char *argument)
{
	char num[MIL];
	char target[MIL];
	int i;
	class_type * pC;
    EDIT_CLASS(ch, pC);

	argument=one_argument(argument, num);
	argument=one_argument(argument, target);	

	if(IS_NULLSTR(argument)){
		ch->println("Syntax: setpose <number> self <text>");
		ch->println("Syntax: setpose <number> others <text>");
		ch->println("Use - for the text to clear it");
		ch->println("e.g. `=Csetpose 1 self You feel very holy.`x");
		ch->println("e.g. `=Csetpose 1 others $n looks very holy.`x");
		return false;
	}

	if(!is_number(num)){
		ch->printlnf("The pose number must be numeric, '%s' is not.", num);
		return false;
	}
	i=atoi(num);
	if(i<0 || i>MAX_LEVEL-1){
		ch->printlnf("The pose number must be in the range 0 to %d, '%s' is not.", 
			MAX_LEVEL, num);
		return false;
	}

	if(!str_prefix(target, "self")){
		if(*argument=='-'){
			ch->printlnf("Self pose %d cleared, was '%s'",i, pC->pose_self[i]);
			replace_string(pC->pose_self[i],"");
			return true;
		}

		ch->printlnf("Self pose %d changed from '%s' to '%s'",
			i, pC->pose_self[i], argument);
		replace_string(pC->pose_self[i],argument);
		return true;
	}
	
	if(!str_prefix(target, "others")){
		if(*argument=='-'){
			ch->printlnf("Other pose %d cleared, was '%s'",i, pC->pose_others[i]);
			replace_string(pC->pose_others[i],"");
			return true;
		}

		ch->printlnf("Other pose %d changed from '%s' to '%s'",
			i, pC->pose_others[i], argument);
		replace_string(pC->pose_others[i],argument);
		return true;
	}

	ch->printlnf("classedit_setpose(): unrecognised option '%s'", target);
	classedit_setpose(ch,"");
	return false;
}
/**************************************************************************/
bool classedit_flags(char_data *ch, char *argument)
{
	class_type * pC;
    EDIT_CLASS(ch, pC);
	
    int value;
	
    if ( argument[0] != '\0' )
    {
		
		if ( ( value = flag_value( classflag_flags, argument ) ) != NO_FLAG )
		{
			pC->flags^= value;
	
			ch->println( "Class flag toggled." );
			return true;
		}
    }

	show_olc_options(ch, classflag_flags, "flag", "flag", pC->flags);
    return false;
}
/**************************************************************************/
bool classedit_remort( char_data *ch, char *argument )
{
	class_type * pC;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println("The remort value must be numerical.");
			ch->println("Remort 0 = class available at first creation.");
			ch->println("Remort 1 = class available at after first remort creation...");
		}else {
			EDIT_CLASS(ch, pC);
			value = atoi( argument );
			if(value<0 || value>5){
				ch->println("The remort value must be between 0 and 5.");
				return false;
			}
			ch->printlnf("Remort value changed from %d to %d.",
				pC->remort_number, value);
			pC->remort_number= value;
			return true;
		}
	}
    ch->println("Syntax: remort <number>");
	ch->println("Remort 0 = class available at first creation.");
	ch->println("Remort 1 = class available at after first remort creation...");
    return false;
}
/**************************************************************************/
bool classedit_hpmintrain( char_data *ch, char *argument )
{
	class_type * pC;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println("The HPmintrain value must be numerical.");
			ch->wrapln("This is the minimum number of hitpoints a character will get "
				"when training hp.");
		}else {
			EDIT_CLASS(ch, pC);
			value = atoi( argument );
			if(value<0){
				ch->println("The HPmintrain value must be greater than 0.");
				return false;
			}
			ch->printlnf("HPMintrain value changed from %d to %d.",
				pC->hp_min, value);
			pC->hp_min= value;
			if(pC->hp_min>pC->hp_max){
				ch->printlnf("HPMaxtrain value increased from %d to %d to compensate.",
					pC->hp_max, value);
				pC->hp_max=value;
			}
			return true;
		}
	}
    ch->println("Syntax: hpmintrain <number>");
	ch->wrapln("This is the minimum number of hitpoints a character will get "
		"when training hp.");
    return false;
}
/**************************************************************************/
bool classedit_hpmaxtrain( char_data *ch, char *argument )
{
	class_type * pC;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println("The HPmaxtrain value must be numerical.");
			ch->wrapln("This is the maximum number of hitpoints a character will get "
				"when training hp.");
		}else {
			EDIT_CLASS(ch, pC);
			value = atoi( argument );
			if(value<0){
				ch->println("The HPmaxtrain value must be greater than 0.");
				return false;
			}
			ch->printlnf("HPMaxtrain value changed from %d to %d.",
				pC->hp_max, value);
			pC->hp_max= value;
			if(pC->hp_min>pC->hp_max){
				ch->printlnf("HPMintrain value decreased from %d to %d to compensate.",
					pC->hp_min, value);
				pC->hp_min=value;
			}
			return true;
		}
	}
    ch->println("Syntax: hpmaxtrain <number>");
	ch->wrapln("This is the maximum number of hitpoints a character will get "
		"when training hp.");
    return false;
}
/**************************************************************************/
bool classedit_prime0( char_data *ch, char *argument )
{
	int stat=flag_value( stat_flags, argument );
	if (stat== NO_FLAG) {
		ch->printlnf("Invalid statname '%s'", argument);
		return false;
	}

	class_type * pC;
	EDIT_CLASS(ch, pC);
	if(stat==pC->attr_prime[0] || stat==pC->attr_prime[1]){
		ch->printlnf("One of the prime stats is already set to %s.", 
			capitalize(stat_flags[stat].name));
		return false;
	}

	ch->printlnf("Prime stat 0 changed from %s to %s.",
		capitalize(stat_flags[pC->attr_prime[0]].name),
		capitalize(stat_flags[stat].name));

	pC->attr_prime[0]=stat;
	return true;	
}
/**************************************************************************/
bool classedit_prime1( char_data *ch, char *argument )
{
	int stat=flag_value( stat_flags, argument );
	if (stat== NO_FLAG) {
		ch->printlnf("Invalid statname '%s'", argument);
		return false;
	}

	class_type * pC;
	EDIT_CLASS(ch, pC);
	if(stat==pC->attr_prime[0] || stat==pC->attr_prime[1]){
		ch->printlnf("One of the prime stats is already set to %s.", 
			capitalize(stat_flags[stat].name));
		return false;
	}

	ch->printlnf("Prime stat 1 changed from %s to %s.",
		capitalize(stat_flags[pC->attr_prime[1]].name),
		capitalize(stat_flags[stat].name));

	pC->attr_prime[1]=stat;
	return true;	
}
/**************************************************************************/
bool classedit_adept( char_data *ch, char *argument )
{
	class_type * pC;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println("The adept value must be numerical.");
			ch->println("Adept = what a class can prac a skill up to.");
		}else {
			EDIT_CLASS(ch, pC);
			value = atoi( argument );
			if(value<0 || value>100){
				ch->println("The adept value must be between 0 and 100.");
				return false;
			}
			ch->printlnf("Skill adept value changed from %d to %d.",
				pC->skill_adept, value);
			pC->skill_adept= value;
			return true;
		}
	}
    ch->println("Syntax: adept <number>");
	ch->println("Adept = what a class can prac a skill up to.");
    return false;
}
/**************************************************************************/
bool classedit_thac0( char_data *ch, char *argument )
{
	class_type * pC;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println("The THAC0 value must be numerical.");
		}else {
			EDIT_CLASS(ch, pC);
			value = atoi( argument );
			if(value<0 || value>30){
				ch->println("The THAC0 value must be between 0 and 30.");
				ch->println("Lower is better, almost every class has a THAC0 of 20.");
				return false;
			}
			if(value<pC->thac0_32){
				ch->println("THAC0 can not be set lower than THAC32 - reduce THAC32 first.");
				return false;
			}
			ch->printlnf("%s THAC0 value changed from %d to %d.",
				capitalize(pC->name), pC->thac0_00, value);
			pC->thac0_00= value;
			return true;
		}
	}
    ch->println("Syntax: thac0 <number>");
	ch->println("THAC = To Hit Armour Class (it is a modifier)");
	ch->println("The THAC0 value must be between 0 and 30.");
	ch->println("Lower is better, almost every class has a THAC0 of 20.");
    return false;
}
/**************************************************************************/
bool classedit_thac32( char_data *ch, char *argument )
{
	class_type * pC;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println("The THAC32 value must be numerical.");
		}else {
			EDIT_CLASS(ch, pC);
			value = atoi( argument );
			if(value<-30 || value>15){
				ch->println("The THAC32 value must be between -30 and 15.");
				ch->println("Lower is better, examples are around the following:`1"
					"Mage 6, Cleric 2, Thief -4, Warrior -10, Druid 0,  Paladin -9`1"
					"Ranger -8, Barbarian -16, Spellfilcher 5");
				return false;
			}
			if(value>pC->thac0_00){
				ch->println("THAC32 can not be set higher than THAC0 - increase THAC0 first.");
				return false;
			}
			ch->printlnf("%s THAC32 value changed from %d to %d.",
				capitalize(pC->name), pC->thac0_32, value);
			pC->thac0_32= value;
			return true;
		}
	}
    ch->println("Syntax: thac32 <number>");
	ch->println("THAC = To Hit Armour Class (it is a modifier)");
	ch->println("The THAC32 value must be between -30 and 15.");
	ch->println("Lower is better, examples are around the following:`1"
		"Mage 6, Cleric 2, Thief -4, Warrior -10, Druid 0,  Paladin -9`1"
		"Ranger -8, Barbarian -16, Spellfilcher 5");
    return false;
}
/**************************************************************************/
bool classedit_fullmana( char_data *ch, char * )
{
	class_type * pC;
	EDIT_CLASS(ch, pC);

	if(pC->fMana){
		pC->fMana=false;
		ch->println("FullMana setup to false");
	}else{
		pC->fMana=true;
		ch->println("FullMana setup to true");
	}
	return true;

}
/**************************************************************************/
bool classedit_casttype( char_data *ch, char *argument )
{
	class_type * pC;
	EDIT_CLASS(ch, pC);

    if( !IS_NULLSTR(argument))
    {
		int value = flag_value( castnames_types, argument );
		if ( value== NO_FLAG )
		{
			ch->printlnf("Unknown casttype '%s'", argument);
			classedit_casttype(ch,"");
			return false;
		}
		
		if(pC->class_cast_type==value){
			ch->printlnf("%s casttype already set to %s", 
				capitalize(pC->name), flag_string( castnames_types, value));
			classedit_casttype(ch,"");
			return false;
		}

		ch->printlnf("%s casttype changed from %s to %s", 
			capitalize(pC->name), 
			flag_string( castnames_types, pC->class_cast_type),
			flag_string( castnames_types, value));

		pC->class_cast_type=(CLASS_CAST_TYPE)value;
		return true;
	}

	show_olc_options(ch, castnames_types, "casttype", "casttype", pC->class_cast_type);
	return false;
}
/**************************************************************************/
bool classedit_spinfoletter(char_data *ch, char *argument)
{
	class_type * pC;
	

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  spinfoletter <letter>");
		return false;
    }
	if(argument[0]=='`'){
		ch->println("`` is not a valid spinfo letter");
		return false;
	}

    EDIT_CLASS(ch, pC);
	argument[1]='\0';

    ch->wraplnf("Spinfo letter changed from '%s' to '%s'.", pC->spinfo_letter, argument);
	replace_string(pC->spinfo_letter, argument);

    return true;
}
/**************************************************************************/
bool classedit_defaultgroup(char_data *ch, char *argument)
{

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  defaultgroup <defaultgroup name>");
		ch->println( "e.g. defaultgroup mage default");
		return false;
    }
	class_type * pC;
    EDIT_CLASS(ch, pC);

    ch->wraplnf("Defaultgroup changed from '%s' to '%s'.", pC->default_group, argument);
	replace_string(pC->default_group, argument);

    return true;
}
/**************************************************************************/
bool classedit_creation( char_data *ch, char *argument )
{
	class_type * pC;
	
	if ( IS_NULLSTR(argument)) {
		ch->println("== Creation Selectable ==");
		ch->println("Syntax: creation [true|false]");
		return false;
	}

	if (!IS_TRUSTED(ch, CLASSEDIT_CREATION_MINTRUST)){
		ch->printlnf( "You must have a trust of %d to be able to change this!", 
			CLASSEDIT_CREATION_MINTRUST);
		return false;
	}

    EDIT_CLASS(ch, pC);
	if(!str_cmp(argument,"true")){
		if(pC->creation_selectable){
			ch->println( "This class is already selectable in creation.");
			return false;
		}else{
			ch->println( "This class is now selectable in creation.");
			pC->creation_selectable=true;
			return true;
		}
	}else if(!str_cmp(argument,"false")){
		if(pC->creation_selectable){
			ch->println( "This class is now no longer selectable in creation.");
			pC->creation_selectable=false;
			return true;
		}else{
			ch->println( "This class is already unselectable in creation.");
			return false;
		}
	}else{
		ch->println("== Creation Selectable ==");
		ch->println("Syntax: creation [true|false]");
		ch->println("You must type either true or false.");
		return false;
	}

}
/**************************************************************************/
bool classedit_shortname(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  shortname <3letters>");
		return false;
    }
	if(str_len(argument)!=3){
		ch->println("The shortname must be 3 letters long");
		return false;
	}
	if(!str_infix(" ", argument)){
		ch->println("Whitespace is not a valid in a short name");
		return false;
	}
	if(!str_infix("`", argument)){
		ch->println("```` is not a valid in a short name");
		return false;
	}

	class_type * pC;
    EDIT_CLASS(ch, pC);
    ch->wraplnf("shortname changed from '%s' to '%s'.", pC->short_name, argument);
	replace_string(pC->short_name, argument);

    return true;
}


/**************************************************************************/
void classedit_copyability(int sn, int target_class, int source_class)
{
	skill_table[sn].skill_level[target_class]			=skill_table[sn].skill_level[source_class];
	skill_table[sn].rating[target_class]				=skill_table[sn].rating[source_class];
	skill_table[sn].low_percent_level[target_class]		=skill_table[sn].low_percent_level[source_class];
	skill_table[sn].maxprac_percent[target_class]		=skill_table[sn].maxprac_percent[source_class];
	skill_table[sn].learn_scale_percent[target_class]	=skill_table[sn].learn_scale_percent[source_class];
}
/**************************************************************************/
bool classedit_copy(char_data *ch, char *argument)
{
    char what[MIL];
    char parent_class[MIL];
	int pci; // parent class index
	int oi; // own index
    bool fAll, fSkills, fRealms, fLanguages, fSpells;

    argument = one_argument( argument, what);
    argument = one_argument( argument, parent_class);

    if(IS_NULLSTR(parent_class))
    {
		ch->println( "Classedit copy commands - used to copy ability settings from another class." );
		ch->println( "This can only be used on classes that arent creation selectable.");
		ch->println( "Syntax:" );
        ch->println( "  copy all <from_class>");
        ch->println( "  copy skills <from_class>");
        ch->println( "  copy realms <from_class>    (includes spheres, seasons & elements)");
        ch->println( "  copy languages <from_class>");
        ch->println( "  copy spells <from_class>");
        return false;
    }

	class_type * pC;
    EDIT_CLASS(ch, pC);

	if(pC->creation_selectable){		
		ch->printlnf( "%s is creation selectable - can't use the copy system on this class.", capitalize(pC->name));
		classedit_copy(ch,"");
        return false;

	}


    fAll =			!str_cmp( what, "all" );
    fSkills =		!str_cmp( what, "skills" );
    fRealms =		!str_cmp( what, "realms" );
    fLanguages =	!str_cmp( what, "languages" );
    fSpells =		!str_cmp( what, "spells" );

    int sn;
    if ( !(fAll || fSkills || fRealms || fLanguages || fSpells))
    {
        ch->printlnf( "Invalid selection '%s'.", what);
		classedit_copy(ch,"");
        return false;
    }

	pci=class_lookup(parent_class);
	if(pci<0){
        ch->printlnf( "Class '%s' not found to copy from.", parent_class);
		classedit_copy(ch,"");
        return false;
	}

	oi=class_lookup(pC->name);
	if( pci==oi){
		ch->println( "Can not copy abilities from the class you are currently editing.");
		classedit_copy(ch,"");
        return false;
	};

    if ( fAll )
    {
        ch->printlnf("Copying all ability settings from the parent class %s.", class_table[pci].name);
        for ( sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++ )
        {
			classedit_copyability(sn, oi, pci);
        }
    }else if ( fSkills ){
		ch->printlnf("Copying all skill related ability settings from the parent class %s.", class_table[pci].name);
        for ( sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++ )
        {
			if(skill_table[sn].sktype==SKTYPE_SKILL){
				classedit_copyability(sn, oi, pci);
			}
        }
    }else if ( fRealms ){
		ch->printlnf("Copying all realm, sphere, element and season related ability settings from the parent class %s.", class_table[pci].name);
        for ( sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++ )
        {
			if(IS_REALM(sn)){
				classedit_copyability(sn, oi, pci);
			}
        }
    }
    else if ( fLanguages )
    {
		ch->printlnf("Copying all language related ability settings from the parent class %s.", class_table[pci].name);
        for ( sn = gsn_human; sn <= gsn_mremish; sn++ )
        {
			classedit_copyability(sn, oi, pci);
		}
    }
    else if ( fSpells )
    {
		ch->printlnf("Copying all spell ability settings from the parent class %s.", class_table[pci].name);
        for ( sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++ )
        {
			if(IS_SPELL(sn)){
				classedit_copyability(sn, oi, pci);
			}
        }
    }else {
		bug("classedit_copy(): - shouldn't have got here!!!");
		do_abort();
    }

    ch->println( "Done." );
    return true;
}
/**************************************************************************/
/**************************************************************************/
bool classedit_recall( char_data *ch, char *argument )
{
	class_type * pC;
	int			value;

    if(IS_NULLSTR(argument)) {
	    ch->println("Syntax: recall <room number>");
		ch->println("Use a room number of 0 to clear the class based recall.");
		return false;
	}

	if(!is_number(argument)){
		ch->println("The recall value must be room vnum.");
		classedit_recall(ch, "");
		return false;
	}else{
		EDIT_CLASS(ch, pC);
		value = atoi( argument );
		
		if(value==0){ // option to clear the recall
			ch->printlnf("Class recall value cleared, was %d.",
				pC->recall);
			pC->recall= value;
			return true;
		}

		if(!get_room_index(value)){
			ch->printlnf("Couldn't find any room %d to set the class recall to.", value);
			return false;
		}
		ch->printlnf("Class recall value changed from %d to %d.",
			pC->recall, value);
		pC->recall= value;
		return true;
	}
}
/**************************************************************************/
bool classedit_morgue( char_data *ch, char *argument )
{
	class_type * pC;
	int			value;

    if(IS_NULLSTR(argument)) {
	    ch->println("Syntax: morgue <room number>");
		ch->println("Use a room number of 0 to clear the class based morgue.");
		return false;
	}

	if(!is_number(argument)){
		ch->println("The morgue value must be room vnum.");
		classedit_morgue(ch, "");
		return false;
	}else{
		EDIT_CLASS(ch, pC);
		value = atoi( argument );
		
		if(value==0){ // option to clear the recall
			ch->printlnf("Class morgue value cleared, was %d.",
				pC->morgue);
			pC->morgue= value;
			return true;
		}

		if(!get_room_index(value)){
			ch->printlnf("Couldn't find any room %d to set the class morgue to.", value);
			return false;
		}
		ch->printlnf("Class morgue value changed from %d to %d.",
			pC->morgue, value);
		pC->morgue= value;
		return true;
	}
}

/**************************************************************************/
// Quick hack, Kal Jan 2007... not very clean
bool classedit_addremortclass(char_data *ch, char *argument)
{
	class_type * pC;
	char keyword[MSL*2];

    if(IS_NULLSTR(argument)) {
        ch->println("Syntax:  addremortclass <classname>");
		return false;
	}

	EDIT_CLASS(ch, pC);

	// invalid character checks
	if(!str_infix("`", argument)){
		ch->println("You can't have the ```` colour code in a class name.");
		return false;
	}
	if(!str_infix(",", argument)){
		ch->println("You can't have the , character in a class name.");
		return false;
	}

	strcpy(keyword, pC->remort_to_classes);
	strcat(keyword, FORMATF(" %s", argument));

	if (str_len(keyword)>MSL-20)
	{
        ch->println("The list of remort to classes would be too long if you added that.");
		return false;
	}

	// convert to lowercase
	for (int i=0; keyword[i]; i++)
	{
		keyword[i]=LOWER(keyword[i]);
	}

	replace_string(pC->remort_to_classes, keyword);

    ch->printlnf("Remort to classes are now %s.", pC->remort_to_classes);
    return true;
}

/**************************************************************************/
bool classedit_setremortclasses(char_data *ch, char *argument)
{
	class_type * pC;
	char keyword[MIL];

	if ( IS_NULLSTR(argument))
	{
        ch->println("Syntax: setremortclasses <class list>");
        ch->println("  (replace all the current 'remort to classes' with the new list.)");
        ch->println("Syntax: setremortclasses -");
        ch->println("  To allow this class to lead to all classes (the default)");
		return false;
	}


	EDIT_CLASS(ch, pC);

	// invalid character checks
	if(!str_infix("`", argument)){
		ch->println("You can't have the ```` colour code in a class name.");
		return false;
	}
	if(!str_infix(",", argument)){
		ch->println("You can't have the , character in a class name.");
		return false;
	}

	if(!str_cmp(argument, "-")){
		ch->printlnf("Class configured so it can remort to any class.");
		strcpy(keyword, "");
	}else{
		strcpy(keyword, argument);
		// convert to lowercase
		for (int i=0; keyword[i]; i++)
		{
			keyword[i]=LOWER(keyword[i]);
		}

		ch->printlnf("Remort to classes changed from %s to %s.", 
			pC->remort_to_classes, keyword);
	}

	replace_string(pC->remort_to_classes, keyword);

    return true;
}

/**************************************************************************/
bool classedit_newbie_prac_location_hint(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println( "Syntax:  newbie_prac_location_hint edit");
		ch->println( " - To go into the string editor to edit the newbie prac location hint");

		ch->println( "Syntax:  newbie_prac_location_hint wordwrap");
		ch->println( " - To wordwrap the newbie prac location hint");
		return false;
    }
	class_type * pC;
    EDIT_CLASS(ch, pC);

	if(!str_cmp(argument, "edit")){
		string_append( ch, &pC->newbie_prac_location_hint);
		return true;
	}

	if(!str_cmp(argument, "wordwrap")){
		pC->newbie_prac_location_hint=note_format_string(pC->newbie_prac_location_hint);
		ch->wraplnf("newbie_prac_location_hint wordwrapped.");
		return true;
	}

	classedit_newbie_prac_location_hint(ch,"");
    return false;
}
/**************************************************************************/
bool classedit_newbie_train_location_hint(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println( "Syntax:  newbie_train_location_hint edit");
		ch->println( " - To go into the string editor to edit the newbie train location hint");

		ch->println( "Syntax:  newbie_train_location_hint wordwrap");
		ch->println( " - To wordwrap the newbie train location hint");
		return false;
    }
	class_type * pC;
    EDIT_CLASS(ch, pC);

	if(!str_cmp(argument, "edit")){
		string_append( ch, &pC->newbie_train_location_hint);
		return true;
	}

	if(!str_cmp(argument, "wordwrap")){
		pC->newbie_prac_location_hint=note_format_string(pC->newbie_train_location_hint);
		ch->wraplnf("newbie_train_location_hint wordwrapped.");
		return true;
	}

	classedit_newbie_train_location_hint(ch,"");
    return false;
}
/**************************************************************************/
bool classedit_pendant_vnum(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println( "Syntax:  pendant_vnum <pendant_object_vnum>");
		ch->println( "Syntax:  pendant_vnum 0   (to clear)");
		return false;
    }
	class_type * pC;
    EDIT_CLASS(ch, pC);

	int value=atoi(argument);

	// support clearing the pendant vnum
	if(value==0){
		ch->printlnf("Pendant vnum cleared, was %d", 
			pC->pendant_vnum);
		pC->pendant_vnum=0;
		return true;
	}

	obj_index_data *pO=get_obj_index(value);
	if(!pO){
		ch->printlnf("Couldn't find any object %d, to set as the pendant vnum.", value);
		return false;
	}

	if(pO->item_type!=ITEM_PENDANT){
		ch->printlnf("Object %d (%s), isn't of type pendant.", value, pO->short_descr);
		return false;

	}

	ch->printlnf("Pendant vnum changed from %d to %d (%s)", 
		pC->pendant_vnum, value, pO->short_descr);
	pC->pendant_vnum=value;

    return true;
}

/**************************************************************************/
bool classedit_newbiemap( char_data *ch, char *argument )
{
    if ( IS_NULLSTR(argument)){
		ch->println( "Syntax:  newbiemap <map_object_vnum>");
		ch->println( "Syntax:  newbiemap 0   (to clear)");
		return false;
    }
	class_type * pC;
    EDIT_CLASS(ch, pC);

	int value=atoi(argument);

	// support clearing the map vnum
	if(value==0){
		ch->printlnf("Newbiemap vnum cleared, was %d", 
			pC->newbie_map_vnum);
		pC->newbie_map_vnum=0;
		return true;
	}

	obj_index_data *pO=get_obj_index(value);
	if(!pO){
		ch->printlnf("Couldn't find any object %d, to set as the newbiemap vnum.", value);
		return false;
	}

	if(pO->item_type!=ITEM_MAP){
		ch->printlnf("Object %d (%s), isn't of type map.", value, pO->short_descr);
		return false;

	}

	ch->printlnf("Newbiemap vnum changed from %d to %d (%s)", 
		pC->newbie_map_vnum, value, pO->short_descr);
	pC->newbie_map_vnum=value;

    return true;
}

/**************************************************************************/


/**************************************************************************/
/**************************************************************************/

