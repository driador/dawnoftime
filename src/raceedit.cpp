/**************************************************************************/
// raceedit.cpp - olc based race editing - Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h" // dawn standard includes

#include "olc.h"
#include "security.h"

// prototypes
void do_racelist( char_data *ch, char *);


/**************************************************************************/
typedef unsigned long ptr_val;	// ptr_val is a pointer as a numeric value
								// used to make the code portable

#define GET_OFFSET(datatype, field) (ptr_val)(((char*) &datatype.field)\
		-((char*) &datatype)) 
#define GET_ADDRESS(dataptr, offset) \
	((void *)( (char*)dataptr + offset)) 
race_data temp_race;

#define R_OFFSET(field) GET_OFFSET(temp_race, field)

/**************************************************************************/
race_data *race_create_race(const char *racename);
bool raceedit_create( char_data *ch, char *argument );
bool raceedit_show( char_data *ch, char * );
bool is_stat( const struct flag_type *flag_table );
/**************************************************************************/
void do_raceedit( char_data *ch, char *argument )
{
	int			raceIndex;
	char		arg[MSL];

	if ( IS_NPC( ch )){
		ch->println( "Players only." );
		return;
	}

	// do security checks
	if (!HAS_SECURITY(ch, 2)){
		ch->println( "You must have an olc security 2 or higher to use this command." );
		return;
	}

    if ( !HAS_SECURITY(ch, RACEEDIT_MINSECURITY)){
    	ch->println( "RaceEdit: Insufficient security to modify or create races." );
    	return;
    }

	if ( !IS_TRUSTED(ch, RACEEDIT_MINTRUST)) {
		ch->printlnf( "You must have a trust of %d or above to use this command.",
			RACEEDIT_MINTRUST);
		return;
	}

	if (IS_NULLSTR(argument)){
		ch->olctitlebar("RACEEDIT");
		ch->println("syntax: raceedit <racename>" );
		ch->println("        raceedit create <new_racename>" );
		ch->println("");
		ch->println("<racename> can selected from one of the following:" );
		do_racelist(ch,"edit");
		ch->olctitlebar("");
		return;
	}
	
	argument = one_argument( argument, arg );
	if ( !str_cmp( arg, "create" )) {
		raceedit_create( ch, argument);
		return;
	}

	raceIndex = race_lookup(arg);
	if(raceIndex<0){
		ch->printlnf( "Couldn't find race '%s'...\r\n"
			"Selectable creation selectable races in yellow:", arg);
		do_racelist(ch,"");
		return;
    }
	if(raceIndex==0){
		ch->println( "You can't edit the 'unknown' race.");
		do_racelist(ch,"");
		return;
    }

    ch->desc->pEdit	= race_table[raceIndex];
	ch->desc->editor = ED_RACE;
	ch->printlnf( "Editing race '%s'", race_table[raceIndex]->name);
	raceedit_show( ch, "");
	return;
}
/**************************************************************************/

const char raceedit_tabs[6][20] = 
	{"General",
	 "Race Flags",
	 "Race Saves",
	 "Player only",
	 "XP & Mods",
	 ""};

/**************************************************************************/
bool raceedit_show( char_data *ch, char * )
{
	race_data *r;
	int	stat, i;
	int lines=0;

    EDIT_RACE( ch, r);
	SET_BIT(ch->dyn,DYN_SHOWFLAGS); // complete flags always shown 

	ch->olctitlebarf("RACEEDIT: %s", uppercase(r->name));

	// GENERATE the tab at the top
	int tab=ch->pcdata->olc_tab;
	ch->print("`_`=r");
	char top[MIL];
	char bar[MIL];
	char buf[MIL];
	top[0]='\0';
	bar[0]='\0';
	for(i=0; !IS_NULLSTR(raceedit_tabs[i]); i++){
		if(i==0){
			strcat(top," ");// one extra in length due to / in next section
		}else{
			strcat(top,"   ");// one extra in length due to / in next section
			strcat(bar,"__");
		}

		strcpy(buf,FORMATF("%d %s", i+1, raceedit_tabs[i]));
		if(tab==i){
			// top handled above
			strcat(bar,"`=r/`Y");
			strcat(bar,buf);
		}else{
			// top handled above
			strcat(bar,"`=r`_/`=R`_");
			strcat(bar,mxp_create_send(ch, FORMATF("%d", i+1), buf));
		}
		size_t l=str_len(top);
		size_t b=str_len(buf);
		top[l+b]=' ';
		top[l+b+1]='\0';
		memset(&top[l], '_', b);

		if(tab==i){
			strcat(bar,"`=r\\`_");
		}else{
			strcat(bar,"`=r`_\\");
		}
	}
	strcat(bar,"_`x");
	// output the tabs
	ch->println(top);
	ch->println(bar);
	
	if(tab==0){ // general
		bool food=false;
		obj_index_data *o=get_obj_index(r->food_vnum);
		if(o && o->item_type==ITEM_FOOD){
			food=true;
		}
		ch->print_blank_lines(1); lines++;
  		ch->printlnf( "`=rName:          `x%s `=@(can't be changed online)", r->name); lines++;
		ch->printlnf( "`=rShortname:     `x%s `=@(displayed in pinfo etc)", r->short_name); lines++;
		ch->printlnf( "`=rLanguage:      `x%s `=@(given during creation)", r->language->name); lines++;
		ch->printlnf( "`=rFood VNUM:     %s%d  %s", 
			(food?"`=v":"`=X"),	r->food_vnum, food?FORMATF("- %s", o->short_descr)
				:o?"- object found, but not of type food!":"- object not found!"); lines++;
		lines+=mxp_display_olc_flags(ch, size_types, r->size, "size", "Size:");
		ch->printlnf( "`=rLowSize:       `=v%d", r->low_size); lines++;
		ch->printlnf( "`=rHighSize:      `=v%d", r->high_size); lines++;
		lines+=mxp_display_olc_flags(ch, race_flags, r->flags, "flags", "Flags:");

		ch->print_blank_lines(1); lines++;
		ch->printlnf( "`=rSelectable within creation: %s",
			r->creation_selectable()?"yes":"no");lines++;
		ch->println("`=@For a race to be selectable within creation the following must be true:");lines++;
		ch->println("- The 'pcrace' and 'creation_activated' flags must both be set.");lines++;
		ch->println("- The race can not have the dynamically_generated flag set (use nodynamic).");lines++;
		ch->println("- The language must be set to something other than 'unknown'.");lines++;
		ch->println("- At least one class must have an xp modifier set.");lines++;
		ch->println("  (this class should be creation selectable, but this enforced).`x");lines++;
		ch->print_blank_lines(16-lines);
	}else if(tab==1){ // racial flags
		mxp_display_olc_flags(ch, act_flags,		r->act,		"act",		"Act:");
		mxp_display_olc_flags(ch, affect_flags,		r->aff,		"affect",	"Affected by:");
		mxp_display_olc_flags(ch, affect2_flags,	r->aff2,	"affect2",	"Affected by2:");
		mxp_display_olc_flags(ch, affect3_flags,	r->aff3,	"affect3",	"Affected by3:");
		mxp_display_olc_flags(ch, form_flags,		r->form,	"form",		"Form:");
		mxp_display_olc_flags(ch, part_flags,		r->parts,	"parts",	"Parts:");
	}else if(tab==2){ // racial saves
		ch->print_blank_lines(1); 
		mxp_display_olc_flags(ch, imm_flags,		r->imm,		"imm",		"Imm:");
		ch->print_blank_lines(1); 
		mxp_display_olc_flags(ch, res_flags,		r->res,		"res",		"Res:");
		ch->print_blank_lines(1); 
		mxp_display_olc_flags(ch, vuln_flags,		r->vuln,	"vuln",		"Vuln:");
		ch->print_blank_lines(1); 
		mxp_display_olc_flags(ch, off_flags,		r->off,		"off",		"Off:");
		ch->print_blank_lines(1); 
	}else if(tab==3){ // pc only
		room_index_data *recall_room=get_room_index(r->recall_room);
		room_index_data *death_room=get_room_index(r->death_room);
		room_index_data *morgue_room=get_room_index(r->morgue);
		obj_index_data *newbiemap=get_obj_index(r->newbie_map_vnum);

		ch->print_blank_lines(1); lines++;
		ch->printlnf( "   `=rRecall:     [`=v%s`=r]  %s- %s%s",
			mxp_create_tagf(ch, "rmvnum", "%6d", r->recall_room), (recall_room?"`=v":"`=X"),
			recall_room?recall_room->name:"recall room unfound!",
			recall_room?FORMATF(" (%s)", recall_room->area->name):""); lines++;
		ch->printlnf( "   `=rDeath room: [`=v%s`=r]  %s- %s%s",
			mxp_create_tagf(ch, "rmvnum", "%6d", r->death_room), (death_room?"`=v":"`=X"),
			death_room?death_room->name:"death room unfound!",
			death_room?FORMATF(" (%s)", death_room->area->name):""); lines++;
		ch->printlnf( "   `=rMorgue:     [`=v%s`=r]  %s- %s%s",
			mxp_create_tagf(ch, "rmvnum", "%6d", r->morgue), (morgue_room?"`=v":"`=X"),
			morgue_room?morgue_room->name:"morgue room unfound!",
			morgue_room?FORMATF(" (%s)", morgue_room->area->name):""); lines++;
		ch->printlnf( "   `=rNewbieMap:  [`=v%s`=r]  %s- %s%s",
			mxp_create_tagf(ch, "oolcvnum", "%6d", r->newbie_map_vnum), (newbiemap?"`=v":"`=X"),
			newbiemap?newbiemap->name:"newbiemap unfound!",
			newbiemap?FORMATF(" (%s)", newbiemap->area->name):""); lines++;
		
		ch->printlnf( "   `=rStartHp:    [`=v%3d`=r]            Max HP:    [`=v%3d`=r]",
			r->start_hp, r->max_hp );

		ch->printlnf( "   `=rPoints:     [`=v%3d`=r]  `=@(additional creation points)",
			r->points);
		// display the remort value
		ch->print(    "   `=rRemort:     [");
		for(int ai=0; ai<6; ai++){
			if(r->remort_number==ai){
				ch->print(" `=/(");
			}else{
				ch->print("  `=V");
			}
			ch->print(mxp_create_send(ch, FORMATF("sa remort %d", ai), FORMATF("%d",ai)));
			if(r->remort_number==ai){
				ch->print(")");
			}else{
				ch->print(" ");
			}
		}
		ch->println("`=r]");

		ch->print_blank_lines(1); lines++;
		ch->printlnf( "   `=rMinHeight:  [`=v%3d`=r]        MinWeight: [`=v%3d`=r]",
			r->min_height, r->min_weight);
		ch->printlnf( "   `=rMaxHeight:  [`=v%3d`=r]        MaxWeight: [`=v%3d`=r]",
			r->max_height, r->max_weight);

		// pre calculate how many lines the racial skills will take to display
		for( i = 0; i<MAX_RACIAL_SKILLS && r->skills[i]!=-1; i++ ) {
			if(i%2==1){
				lines++; 
			}
		}

		ch->print_blank_lines(8-lines);

		ch->olctitlebar("Racial Skills");
		ch->print("   ");
		for( i = 0; i<MAX_RACIAL_SKILLS && r->skills[i]!=-1; i++ ) {
			ch->printf( "`=rSkill %d: `=V%-34s  ", i+1, skill_table[r->skills[i]].name );
			if(i%2==1){
				ch->println("");
				ch->print("   ");
			}
		}
		if(i>0 || (r->skills[i]!=-1)){
			ch->println("`=v");
			lines++;
		}else{
			ch->println("`=rnone`x");
			lines++;
		}		
		ch->print_blank_lines(1);
	}else if(tab==4){ // XP & Mods
		ch->print_blank_lines(1); lines++;
		int col=0;
		ch->olctitlebar("Class XP ratings for race"); lines++;
		for ( i= 0; !IS_NULLSTR(class_table[i].name); i++)
		{	
			char classname[MIL];
			sprintf(classname, "%-14s",capitalize(class_table[i].name));
			ch->printf("  `=r%s `=w%s`=v%5d`=w%s", 
				mxp_create_send_prompt(ch, FORMATF("classxp '%s' 1000", class_table[i].name), classname),
				r->class_exp[i]==0?" ":
					mxp_create_send(ch, FORMATF("sa classxp '%s' %d", 
											class_table[i].name,
											r->class_exp[i]==1000?0:r->class_exp[i]-100),"<"),
				r->class_exp[i],
				r->class_exp[i]==10000?" ":
					mxp_create_send(ch, FORMATF("sa classxp '%s' %d", 
											class_table[i].name,
											UMAX(900,r->class_exp[i])+100),">")
				);

			if(++col%3==0){
				ch->println("`x"); lines++;
			}else{
				ch->print("  ");
			}
		}
		if(col%3!=0){
			ch->println(""); lines++;
		}	

		ch->print_blank_lines(1); lines++;

		ch->olctitlebar("Racial Modifiers"); lines++;
		int lefttotal=0;
		int righttotal=0;
		int halfmaxstat=MAX_STATS/2;
		for (stat = 0; stat < halfmaxstat; stat++)
		{
			ch->printf( "  `=r%-22s `=w%s`=v%3d`=w%s     ",
				capitalize(stat_flags[stat].name),
				mxp_create_send(ch, FORMATF("sa racemod %s %d", 
						stat_flags[stat].name,
						r->stat_modifier[stat]-5),"<"),
				r->stat_modifier[stat],
				mxp_create_send(ch, FORMATF("sa racemod %s %d", 
						stat_flags[stat].name,
						r->stat_modifier[stat]+5),">")
				);
			lefttotal+=r->stat_modifier[stat];

			ch->printlnf( "             `=r%-22s `=w%s`=v%3d`=w%s",
				capitalize(stat_flags[stat+halfmaxstat].name),
				mxp_create_send(ch, FORMATF("sa racemod %s %d", 
						stat_flags[stat+halfmaxstat].name,
						r->stat_modifier[stat+halfmaxstat]-5),"<"),
				r->stat_modifier[stat+halfmaxstat],
				mxp_create_send(ch, FORMATF("sa racemod %s %d", 
						stat_flags[stat+halfmaxstat].name,
						r->stat_modifier[stat+halfmaxstat]+5),">") 
				); 
			righttotal+=r->stat_modifier[stat+halfmaxstat];
			lines++;			
		}
		ch->printf(   "  %-26s    %3d     ", "`=RLeft Modifier Total:`=r", lefttotal ); 
		ch->printlnf( "              %-26s   %3d`x", "`=RRight Modifier Total:`=r", righttotal);
		lines++;

		ch->print_blank_lines(17-lines);
		ch->printlnf( "   `=@hint: use `=C%s`=@ to compare these values to all selectable races.`x",
			mxp_create_send(ch, "raceinfo")); lines++;
	}
	ch->olctitlebar("");

	REMOVE_BIT(ch->dyn,DYN_SHOWFLAGS);
    return false;
}

/**************************************************************************/
bool raceedit_remort( char_data *ch, char *argument )
{
    race_data *r;
    EDIT_RACE(ch, r);

	if(IS_NULLSTR(argument) || !is_number(argument)){
		ch->println("`=lSyntax: `=Cremort #`x");
		ch->println("This assigns which remort a particular race becomes available at.");
		return false;
	}
	int value=atoi(argument);
	if(value<0 || value>5){
		ch->println("Remort must be in the range 0 to 5");
		return false;
	}

	ch->printlnf("Remort changed from %d to %d.", r->remort_number, value);
	r->remort_number=value;
    return true;
}

/**************************************************************************/
// generic flag toggling function
bool raceedit_flagtoggle(char_data *ch, char *argument, 
	const struct flag_type *ft, char *toggled_name, ptr_val flag_offset, 
	const char *command_name, const char *descript, int tab)
{
    race_data *r;

	EDIT_RACE( ch, r);
	int *flag=(int*)GET_ADDRESS(r, flag_offset);
    int value;
	
    if(!IS_NULLSTR(argument)){
		
		if ( ( value = flag_value( ft, argument ) ) != NO_FLAG )
		{
			if(is_stat(ft)){
				if(*flag==value){
					ch->printlnf("%s is already set to '%s'.", 
						toggled_name, flag_string(ft, value) );
					return false;
				}else{
					ch->printlnf("%s changed from '%s' to '%s'.", 
						toggled_name, flag_string(ft, *flag), flag_string(ft, value) );
					*flag = value;
				}
			}else{
				*flag ^= value;
				ch->printlnf("%s flag toggled.", toggled_name);
			}							
			if(ch->pcdata->olc_tab!=tab){
				olcex_tab(ch, FORMATF("%d",tab+1));
				REMOVE_BIT(ch->dyn, DYN_USING_OLCAFTER);
			}
			return true;
		}
    }

	show_olc_options(ch, ft, command_name, descript, *flag);
    return false;	
}
/**************************************************************************/
bool raceedit_racemod( char_data *ch, char *argument )
{
	race_data *r;
	EDIT_RACE( ch, r );
	char		arg[MIL], arg1[MIL];
	int			value, stat;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg1 );

	if (IS_NULLSTR(arg1)){
		ch->println( "Syntax: racemod <statname> <racial modifier>" );
		return false;
	}

	if (!is_number(arg1)){
		ch->println( "The racial modifier must be a number." );
		return false;	
	}
	

	value = atoi( arg1 );
	if (( stat = flag_value( stat_flags, arg )) == NO_FLAG ) {
		ch->printlnf( "Invalid statname '%s'", arg );
		return false;
	}
	if(value<-99 || value > 99){
		ch->println( "The racial modifier must be in the range -99 to 99.");
		return false;
	}

	ch->printlnf( "Stat '%s' racial modifier changed from %d to %d.", 
		stat_flags[stat].name, r->stat_modifier[stat], value);
	r->stat_modifier[stat] = value;
	return true;
}

/**************************************************************************/
bool raceedit_size(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		size_types, "Racial size",R_OFFSET(size), 
		"size", "size", 0);
}
/**************************************************************************/
bool raceedit_flags(char_data *ch, char *argument)
{
	bool result=raceedit_flagtoggle(ch, argument, 
		race_flags, "Racial flags",R_OFFSET(flags), 
		"flags", "flags", 0);

	race_data *r;
	EDIT_RACE(ch,r);
	r->update_dynamic_flags();

	return result;
}
/**************************************************************************/
bool raceedit_act(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		act_flags, "Act",R_OFFSET(act), 
		"act", "act", 1);
}

/**************************************************************************/
bool raceedit_aff(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		affect_flags, "Racial affects",R_OFFSET(aff), 
		"aff", "affect", 1);
}
/**************************************************************************/
bool raceedit_aff2(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		affect2_flags, "Racial affects 2",R_OFFSET(aff2), 
		"aff2", "affect 2", 1);
}
/**************************************************************************/
bool raceedit_aff3(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		affect3_flags, "Racial affects 3",R_OFFSET(aff3), 
		"aff3", "affect 3", 1);
}
/**************************************************************************/
bool raceedit_form(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		form_flags, "Racial Form",R_OFFSET(form), 
		"form", "form", 1);
}
/**************************************************************************/
bool raceedit_parts(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		part_flags, "Racial part",R_OFFSET(parts), 
		"part", "part", 1);
}
/**************************************************************************/
bool raceedit_imm(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		imm_flags, "Racial immunitity", R_OFFSET(imm), 
		"imm", "imm", 2);
}
/**************************************************************************/
bool raceedit_res(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		res_flags, "Racial resistance", R_OFFSET(res), 
		"imm", "imm", 2);
}
/**************************************************************************/
bool raceedit_vuln(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		vuln_flags, "Racial vulnerability", R_OFFSET(vuln), 
		"imm", "imm", 2);
}
/**************************************************************************/
bool raceedit_off(char_data *ch, char *argument)
{
	return raceedit_flagtoggle(ch, argument, 
		off_flags, "Racial offensive", R_OFFSET(off), 
		"off", "off", 2);
}
/**************************************************************************/
bool raceedit_tab1(char_data *ch, char *)
{
	return olcex_tab(ch, "1");
}
/**************************************************************************/
bool raceedit_tab2(char_data *ch, char *)
{
	return olcex_tab(ch, "2");
}
/**************************************************************************/
bool raceedit_tab3(char_data *ch, char *)
{
	return olcex_tab(ch, "3");
}
/**************************************************************************/
bool raceedit_tab4(char_data *ch, char *)
{
	return olcex_tab(ch, "4");
}
/**************************************************************************/
bool raceedit_tab5(char_data *ch, char *)
{
	return olcex_tab(ch, "5");
}
/**************************************************************************/
bool raceedit_skills( char_data *ch, char *argument )
{
	race_data *r;
	char		arg[MIL], arg1[MIL];
	int		i, number, sn;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg1 );

	if ( !IS_NULLSTR( arg1 )) {
	
		EDIT_RACE( ch, r );

		if( !str_cmp( arg, "add" )) {

			sn = skill_lookup( arg1 );
			if ( sn == -1 ) {
				ch->printlnf( "The skill '%s' does not exist.", arg1);
				return false;
			}
		
			for( i = 0; i<MAX_RACIAL_SKILLS && r->skills[i]!=-1; i++ );// Find the empty entry in array
			if ( i >=MAX_RACIAL_SKILLS || r->skills[i]!=-1) {   //  You may want to change this to 10??  that's how many there is room for
				ch->wrapln( "Maximum skills reached, either remove some, or increase MAX_RACIAL_SKILLS in params.h and recompile." );
				return false;
			}
			r->skills[i] = sn;
			ch->printlnf( "Skill '%s' added.", skill_table[sn].name);
			// mark the end of the racial skills
			if(i+1<MAX_RACIAL_SKILLS){
				r->skills[i+1]=-1;
			}
			return true;
		}

		if( !str_cmp( arg, "remove" )) {
			int last_skill;
			// to reflect the raceedit_show, making the first skill #1 instead of #0
			number = atoi( arg1 );
			
			for( last_skill= 0; last_skill<MAX_RACIAL_SKILLS && r->skills[last_skill]!=-1; last_skill++ );// Find the empty entry in array

			if ( number > last_skill || number < 1 || r->skills[number-1]==-1) {
				ch->printlnf( "There is no skill entry # %d.", number );
				return false;
			}
			
			ch->printlnf( "Skill %d '%s' removed.", number, skill_table[r->skills[number-1]].name ); 
			// find the last skill 

			last_skill--; // last_skill less 1 is the skill we can swap with

			// swap the last one into the position of the deleted skill
			r->skills[number-1]=r->skills[last_skill];
			r->skills[last_skill]=-1;
			return true;
		}
	}

	ch->println( "Syntax: skill add <skill>" );
	ch->println( "Syntax: skill remove <skill number>" );
	return false;
}

/**************************************************************************/
bool raceedit_language( char_data *ch, char *argument )
{
	race_data *r;
	language_data *newlanguage;

	if ( IS_NULLSTR(argument)) {
		ch->println( "Syntax: language [language]" );
		return false;
	}

	EDIT_RACE( ch, r );

	newlanguage=language_lookup( argument );
	if ( !newlanguage ) {
		ch->printlnf( "There is no '%s' language.", argument );
		return false;
	}
	if( r->language==newlanguage){
		ch->printlnf( "%s's language is already set to %s.", r->name, newlanguage->name);
		return false;
	}

	ch->printlnf( "Language changed from '%s' to '%s'.", 
		r->language->name, newlanguage->name);
	r->language = newlanguage;
	return true;
}

/**************************************************************************/
bool raceedit_classxp( char_data *ch, char *argument )
{
	race_data *r;
	EDIT_RACE( ch, r );
	char		arg[MIL], arg1[MIL];
	int			value, classindex;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg1 );

	if (IS_NULLSTR(arg1)){
		ch->println( "Syntax: classxp <classname> <xpmodifier>" );
		ch->wrapln( "note: xpmodifiers must be either in the range 1000 to 10000 or "
			"0 (in the situation where this race can't play that specified class)." );
		return false;
	}

	if (!is_number(arg1)){
		ch->println( "The xpmodifiers must be a numbers." );
		return false;	
	}
	value=atoi(arg1);
	
	classindex=class_lookup(arg);
	if (classindex<0) {
		ch->printlnf( "No such class '%s'", arg);
		return false;
	}

	if(value<1000 && value !=0){
		ch->println( "Base Class XP must be 1000 or greater (unless it is set to 0)!" );
		return false;
	}
	if(value>10000){
		ch->println( "Base Class XP must be less than 10000!");
		return false;
	}
	if(r->class_exp[classindex]==value){
		ch->printlnf( "Base %s XP is already set to %d!",
			class_table[classindex].name, value);
		return false;
	}

	ch->printlnf( "Class '%s' xpmodifier changed from %d to %d.", 
		class_table[classindex].name, r->class_exp[classindex], value);
	r->class_exp[classindex] = value;

	r->update_dynamic_flags();

	return true;
}

/**************************************************************************/
bool raceedit_create( char_data *ch, char *argument )
{
	int ri;
	race_data *r;

	if (!IS_TRUSTED(ch, RACEEDIT_CREATE_MINTRUST)){
		ch->printlnf( "You must have a trust of %d to be able to create races", 
			RACEEDIT_CREATE_MINTRUST);
		return false;
	}

	if (IS_NULLSTR(argument)){
		ch->println( "You must type in name of the race you wish to create." );
		edit_done(ch);
		return false;
	}

	// can't have spaces, ', ~, { in a race name
	if(has_space(argument) 
		|| count_char(argument, '\'')>0 
		|| count_char(argument, '~')>0
		|| count_char(argument, '`')>0){
		ch->println("You can't have spaces, ', ~ or `` in a racename.");
		return false;
	}

	// check for naming collisions
	ri=race_exact_lookup(argument);
	if(ri>-1){
	    ch->printlnf( "There is already a race called '%s' try a different name.", 
			race_table[ri]->name);
		return false;
	};

	// find the last race in the races list... so we can confirm there is space
	race_data *last_race;
	int count=0;
	for(last_race=race_list; last_race->next; last_race=last_race->next){
		count++;
	}
	if(count>=MAX_RACE){
		log_notef("The mud has currently been compiled to operate with %d races... There are already "
			"that many races in the game... to add more you need to the increase MAX_RACE value in "
			"params.h and perform a clean recompile of the mud ('make clean' followed by 'make').",
			MAX_RACE);
		return false;
	}

	if(str_len(argument)>18){
		ch->println("`=XYOU ARE CREATING A RACE WITH A NAME MORE THAN 18 CHARACTERS LONG.");
		ch->println("THE FORMATTING OF SOME COMMANDS WILL BE MISALIGNED!`x");
	}

	// check we haven't reached the maximum number of races
	ch->printlnf( "`YCreating new race '%s'`x", argument );
	// add the race value
	r=race_create_race(argument);

	// put them in the races table
	last_race->next=r;
	race_table[count]=r;

	// edit the race
	do_raceedit( ch, argument);
	return true;
}

/**************************************************************************/
bool raceedit_highsize( char_data *ch, char *argument )
{
	race_data *r;
	EDIT_RACE( ch, r );
	int	value;

    if( IS_NULLSTR(argument) || !is_number(argument)) {
		ch->println( "Syntax: highsize <number>" );
		ch->wrapln( "Every piece of equipment has a relative size value, if this value "
			"is larger than the races highsize, the object will be too large for this "
			"race to wear.  The default convention for an object that is wearable by "
			"all races is to set the relative size to 50... therefore the value 50 should "
			"be within the range of your low/high sizes for this race.");
		return false;
	}
	value = atoi( argument );

	if (value>100 || value<10){
	    ch->println( "Try a value closer to 50 (valid range 10 to 100)." );
		return false;
	}

	if(value==r->high_size){
		ch->printlnf( "This races highsize is already set to %d!", value);
		return false;
	}

	ch->printlnf( "Race highsize changed from %d to %d.", r->high_size, value);
	r->high_size = value;
	return true;
}

/**************************************************************************/
bool raceedit_lowsize( char_data *ch, char *argument )
{
	race_data *r;
	EDIT_RACE( ch, r );
	int	value;

    if( IS_NULLSTR(argument) || !is_number(argument)) {
		ch->println( "Syntax: lowsize <number>" );
		ch->wrapln( "Every piece of equipment has a relative size value, if this value "
			"is smaller than the races lowsize, the object will be too small for this "
			"race to wear.  The default convention for an object that is wearable by "
			"all races is to set the relative size to 50... therefore the value 50 should "
			"be within the range of your low/high sizes for this race.");
		return false;
	}
	value = atoi( argument );

	if (value>100 || value<10){
	    ch->println( "Try a value closer to 50 (valid range 10 to 100)." );
		return false;
	}

	if(value==r->low_size){
		ch->printlnf( "This races lowsize is already set to %d!", value);
		return false;
	}

	ch->printlnf( "Race lowsize changed from %d to %d.", r->low_size, value);
	r->low_size = value;
	return true;
}

/**************************************************************************/
bool raceedit_starthp( char_data *ch, char *argument )
{
	race_data *r;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println( "The hp value must be numerical." );
		}else {
			EDIT_RACE( ch, r );
			value = atoi( argument );
			if(value<1){
				ch->println("You can't set the start hp to below zero.");
				return false;
			}
			ch->printlnf( "Starting hitpoints changed from %d to %d.", 
				r->start_hp , value);
			r->start_hp = value;
			return true;
		}
	}
    ch->println( "Syntax: starthp <hp>" );
    return false;
}

/**************************************************************************/
bool raceedit_maxhp( char_data *ch, char *argument )
{
	race_data *r;
	int			value;

    if ( !IS_NULLSTR(argument)) {

		EDIT_RACE( ch, r );
		value = atoi( argument );
		if(value<1){
			ch->println("You can't set the max hp to below zero.");
			return false;
		}
		if(value<r->start_hp){
			ch->println("The Max HP cant be less than the Start HP.");
			return false;
		}
		ch->printlnf( "Maximum Hit changed from %d to %d.", 
			r->max_hp, value);
		r->max_hp = value;
		return true;
    }

    ch->println( "Syntax: maxhp <hp>" );
    return false;
}

/**************************************************************************/
bool raceedit_points( char_data *ch, char *argument )
{
	race_data *r;
	int	value;

    if ( IS_NULLSTR(argument)) {
		ch->println("Syntax: points <number>" );
		ch->wrapln("This is the additional number of creation points "
			"incurred by choosing this race.");
		return false;
	}

	EDIT_RACE( ch, r );
	value = atoi( argument );
	if(value<0 || value>100){
		ch->println("The points must be in the range 0 to 100.");
		return false;
	}

	if(value==r->points){
		ch->printlnf("The points are already set to %d", value);
		return false;
	}

	ch->printlnf( "Point cost changed from %d to %d.", r->points, value);
	r->points = value;
	return true;
}

/**************************************************************************/
bool raceedit_recall( char_data *ch, char *argument )
{
	race_data *r;
	int			value;

    if ( !IS_NULLSTR(argument)) {

		EDIT_RACE( ch, r );
		value = atoi( argument );
		if ( get_room_index( value ) == '\0' ) {
			ch->println( "That room vnum doesn't exist." );
			return false;
		}
		r->recall_room = value;
	    ch->println( "Recall Room Set." );
	    return true;
    }

    ch->println( "Syntax: recall [room number]" );
    return false;
}
/**************************************************************************/
bool raceedit_morgue( char_data *ch, char *argument )
{
	race_data *r;
	int	value;

    if ( !IS_NULLSTR(argument)) {

		EDIT_RACE( ch, r );
		value = atoi( argument );
		if ( get_room_index( value ) == '\0' ) {
			ch->printlnf( "Room vnum %d doesn't exist.", value );
			return false;
		}
		r->morgue= value;
	    ch->println( "Morgue Set." );
	    return true;
    }

    ch->println( "Syntax: morgue <room number>" );
    return false;
}
/**************************************************************************/
bool raceedit_newbiemap( char_data *ch, char *argument )
{
	race_data *r;
	int	value;

    if ( !IS_NULLSTR(argument)) {

		EDIT_RACE( ch, r );
		value = atoi( argument );
		if(!get_obj_index( value )){
			ch->printlnf( "Newbiemap vnum %d doesn't exist.", value );
			return false;
		}
		r->newbie_map_vnum= value;
	    ch->println( "Newbiemap Set." );
	    return true;
    }

    ch->println( "Syntax: newbiemap <object number>" );
    return false;
}

/**************************************************************************/
bool raceedit_death( char_data *ch, char *argument )
{
	race_data *r;
	int			value;

    if ( !IS_NULLSTR(argument)) {

		EDIT_RACE( ch, r );
		value = atoi( argument );
		if ( get_room_index( value ) == '\0' ) {
			ch->println( "That room vnum doesn't exist." );
			return false;
		}
		if(r->death_room == value){
			ch->printlnf( "%s death room is already set at %d.", 
				r->name, r->death_room);
			return false;
		}
	    ch->printlnf( "%s death room changed from %d to %d.", 
			r->name, r->death_room, value);
		r->death_room = value;
	    return true;
    }

    ch->println( "Syntax: death [room number]" );
    return false;
}

/**************************************************************************/
bool raceedit_minheight( char_data *ch, char *argument )
{
	race_data *r;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println( "The height value must be numerical." );
		}else {
			EDIT_RACE( ch, r );
			value = atoi( argument );
			ch->printlnf( "Minimum Height changed from %d to %d.", 
				r->min_height, value);
			r->min_height = value;
			return true;
		}
	}
    ch->println( "Syntax: minheight <inches>" );
    return false;
}

/**************************************************************************/
bool raceedit_maxheight( char_data *ch, char *argument )
{
	race_data *r;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println( "The height value must be numerical." );
		}else {
			EDIT_RACE( ch, r );
			value = atoi( argument );
			ch->printlnf( "Maximum Height changed from %d to %d.", 
				r->max_height, value);
			r->max_height = value;
			return true;
		}
	}
    ch->println( "Syntax: maxheight <inches>" );
    return false;
}

/**************************************************************************/
bool raceedit_minweight( char_data *ch, char *argument )
{
	race_data *r;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println( "The weight value must be numerical." );
		}else {
			EDIT_RACE( ch, r );
			value = atoi( argument );
			ch->printlnf( "Minimum weight changed from %d to %d.", 
				r->min_weight, value);
			r->min_weight = value;
			return true;
		}
	}
    ch->println( "Syntax: minweight <pounds>" );
    return false;
}

/**************************************************************************/
bool raceedit_maxweight( char_data *ch, char *argument )
{
	race_data *r;
	int			value;

    if ( !IS_NULLSTR(argument)) {
		if (!is_number(argument)){
			ch->println( "The weight value must be numerical." );
		}else {
			EDIT_RACE( ch, r );
			value = atoi( argument );
			ch->printlnf( "Maximum weight changed from %d to %d.", 
				r->max_weight, value);
			r->max_weight = value;
			return true;
		}
	}
    ch->println( "Syntax: maxweight <pounds>" );
    return false;
}

/**************************************************************************/
bool raceedit_foodvnum( char_data *ch, char *argument )
{
	race_data *r;
	obj_index_data *o;
	int value;

    if ( IS_NULLSTR(argument) || !is_number(argument)) {
		ch->println( "Syntax: foodvnum <vnum>" );
		return false;
    }

	EDIT_RACE( ch, r );
	
	value = atoi( argument );
	o=get_obj_index( value );
	if(!o){
		ch->printlnf( "`RThere is no object with a vnum of %d!", value );
		return false;
	}
	if(o->item_type!=ITEM_FOOD){
		ch->printlnf( "`RThe object must be of type food... '%s' is not.", o->short_descr );
		return false;
	}

	ch->printlnf("Food vnum changed from %d to %d (%s)",
		value, r->food_vnum, o->short_descr);		
	r->food_vnum = value;
	return true;
}

/**************************************************************************/
bool raceedit_nodynamic( char_data *ch, char *argument )
{
	race_data *r;
	EDIT_RACE( ch, r );

	if(!IS_SET(r->flags, RACEFLAG_DYNAMICALLY_GENERATED)){
		ch->println("The dynamically_generated race flag is not set.");
		return false;
	}

	REMOVE_BIT(r->flags, RACEFLAG_DYNAMICALLY_GENERATED);
	ch->println("The dynamically_generated race flag has been removed.");

	return true;
}
/**************************************************************************/
bool raceedit_shortname(char_data *ch, char *argument)
{
	race_data *r;
	EDIT_RACE( ch, r );

    if(IS_NULLSTR(argument)){
		ch->println("Syntax:  shortname <string>");
		return false;
    }

	ch->printlnf( "Race shortname changed from '%s' to '%s'.", r->short_name, argument);
    replace_string(r->short_name, argument);

    return true;
}
/**************************************************************************/
