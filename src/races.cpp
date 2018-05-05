/**************************************************************************/
// races.cpp - race related code
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"
#include "areas.h"

/**************************************************************************/
void do_langedit(char_data *ch, char *argument);
/**************************************************************************/
char *race_get_races_set_for_n_array(unsigned char n_array[])
{
	static char buf[MSL*2];
	buf[0]='\0';
	buf[1]='\0';
	for(int i=0; race_table[i]; i++){
		if(IS_SETn(n_array, i)){
			strcat(buf, " ");
			strcat(buf, pack_word(race_table[i]->name));
		}
	}
	return &buf[1];
}

/**************************************************************************/
bool race_data::creation_selectable()
{  
	if( IS_ALL_SET(flags, (RACEFLAG_PCRACE|RACEFLAG_CREATION_ACTIVATED))
		&& !IS_SET(flags, RACEFLAG_PCRACE_WITH_NO_CLASSXP_SET)
		&& !IS_SET(flags, RACEFLAG_DYNAMICALLY_GENERATED))
	{
		return true;
	}
	return false;
};

/**************************************************************************/
const struct flag_type race_flags[] =
{
	{ "creation_activated",		RACEFLAG_CREATION_ACTIVATED,true},
	{ "pcrace",					RACEFLAG_PCRACE,			true},
	{ "dynamically_generated",	RACEFLAG_DYNAMICALLY_GENERATED,false}, // a dynamically generated class can't be selected
	{ "pcrace_with_no_classxp_set", RACEFLAG_PCRACE_WITH_NO_CLASSXP_SET,false}, // a dynamically generated class can't be selected

	{ "nightmap",				RACEFLAG_NIGHTMAP,					true},
	{ "regen_slow_in_light",	RACEFLAG_REGEN_SLOW_IN_LIGHT,		true},
	{ "need_twice_as_much_sleep",RACEFLAG_NEED_TWICE_AS_MUCH_SLEEP,	true},	
	{ "always_hidden_from_mortal_raceinfo",RACEFLAG_ALWAYS_HIDDEN_FROM_MORTAL_RACEINFO,true},	
	{ "hidden_from_mortal_raceinfo_when_above_their_remort",RACEFLAG_HIDDEN_FROM_MORTAL_RACEINFO_WHEN_ABOVE_THEIR_REMORT,true},	
	{ "lowcost_launch",			RACEFLAG_LOWCOST_LAUNCH,true},	
	{ "no_customization",		RACEFLAG_NO_CUSTOMIZATION,true},	
	{ NULL,0,0}
};

/**************************************************************************/
void racetype_write_generic_races_set_for_n_array(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	unsigned char *array_n=(unsigned char*)((char *)data + gio_table[tableIndex].index);	
	char *text=race_get_races_set_for_n_array(array_n);
	
	if(IS_NULLSTR(text)){
		return;
	}

	// output the race restricts
	fprintf(fp, "%s %s~\n",gio_table[tableIndex].heading, text);
}
/**************************************************************************/
void racetype_read_generic_races_set_for_n_array(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	unsigned char *array_n=(unsigned char*)((char *)data + gio_table[tableIndex].index);	
	char *all_rnames, *rnames;
	char name[MIL];
	int ri;

	all_rnames=fread_string(fp);
	rnames=all_rnames;

	// table has already been nulled by the gio system
	rnames=one_argument(rnames, name);
	while(!IS_NULLSTR(name)){
		ri=race_exact_lookup(name);

		if (ri<0){
			logf( "racetype_read_generic_races_set_for_n_array: race '%s' not found... "
				"ignoring (header='%s')!", name, gio_table[tableIndex].heading);
		}else{
			SET_BITn(array_n, ri);
		}

		// get the next name
		rnames=one_argument(rnames, name);
	}
}

/**************************************************************************/
// gio prototypes
GIO_CUSTOM_FUNCTION_PROTOTYPE(race_gioWriteClassExp);
GIO_CUSTOM_FUNCTION_PROTOTYPE(race_gioReadClassExp);
// gio structures
// race_data GIO lookup table 
GIO_START(race_data)
GIO_STR(name)
GIO_STR(short_name)
GIO_STRH(load_language, "language")
GIO_SHINT(remort_number)
GIO_WFLAG(flags, race_flags)
GIO_SHWFLAG(size, size_types)
GIO_WFLAG(act, act_flags)
GIO_WFLAG(aff, affect_flags)
GIO_WFLAG(aff2, affect2_flags)
GIO_WFLAG(aff3, affect3_flags)
GIO_WFLAG(off, off_flags)
GIO_WFLAG(imm, imm_flags)
GIO_WFLAG(res, res_flags)
GIO_WFLAG(vuln, vuln_flags)
GIO_WFLAG(form, form_flags)
GIO_WFLAG(parts, part_flags)
GIO_SHINT(points)
GIO_CUSTOM_WRITEH(name, "class_exp", race_gioWriteClassExp)
GIO_CUSTOM_READH(name, "class_exp", race_gioReadClassExp)
GIO_STRH(load_skills, "skills")
GIO_SHINTH(stat_modifier[STAT_ST], "statmod_ST")
GIO_SHINTH(stat_modifier[STAT_QU], "statmod_QU")
GIO_SHINTH(stat_modifier[STAT_PR], "statmod_PR")
GIO_SHINTH(stat_modifier[STAT_EM], "statmod_EM")
GIO_SHINTH(stat_modifier[STAT_IN], "statmod_IN")
GIO_SHINTH(stat_modifier[STAT_CO], "statmod_CO")
GIO_SHINTH(stat_modifier[STAT_AG], "statmod_AG")
GIO_SHINTH(stat_modifier[STAT_SD], "statmod_SD")
GIO_SHINTH(stat_modifier[STAT_ME], "statmod_ME")
GIO_SHINTH(stat_modifier[STAT_RE], "statmod_RE")
GIO_SHINT(start_hp)
GIO_INT(max_hp)
GIO_SHINT(low_size)
GIO_SHINT(high_size)
GIO_INT(recall_room)
GIO_INT(death_room)
GIO_INT(morgue)
GIO_INT(newbie_map_vnum)
GIO_SHINT(min_height)
GIO_SHINT(max_height)
GIO_SHINT(min_weight)
GIO_SHINT(max_weight)
GIO_INT(food_vnum)
GIO_FINISH_STRDUP_EMPTY

/**************************************************************************/
// generates the unknown race - used by the system to prevent crashes
race_data *race_create_race(const char *racename)
{
	race_data *r=new race_data;
	memset(r,0, sizeof(race_data));
	r->name=str_dup(racename);
	r->short_name=str_dup(racename);
	r->language=language_unknown;
	r->load_language=str_dup("");
	r->load_skills=str_dup("");
	r->skills[0]=-1;
	r->remort_number=0;
	r->recall_room=ROOM_VNUM_LIMBO;
	r->death_room=ROOM_VNUM_LIMBO;
	r->flags=RACEFLAG_DYNAMICALLY_GENERATED;
	r->act=0;
	r->aff=0; 				// aff bits for the race
	r->aff2=0; 				// aff2 bits for the race
	r->aff3=0; 				// aff3 bits for the race
	r->off=0; 				// off bits for the race
	r->imm=0;				// imm bits for the race
	r->res=0;				// res bits for the race
	r->vuln=0;				// vuln bits for the race
	r->form=0;				// default form flag for the race
	r->parts=0;				// default parts for the race
	r->points=0;
	r->size=SIZE_MEDIUM;
	r->start_hp		= 10;
	r->max_hp		= 150;
	r->low_size		= 20;
	r->high_size	= 70;	

	r->next=NULL;
	return r;
}
/**************************************************************************/
// create a race and add it to the race_table, returning its index
int race_generate_race_adding_to_race_table(const char *racename)
{
	assertp(race_table); // we assume the race_table has already been allocated

	race_data *newr=race_create_race(racename);

	race_data *r;
	for(r=race_list; r->next; r=r->next){
	}
	r->next=newr;

	// link it into the race_table
	int i;
	for(i=0; race_table[i];i++){
	}
	race_table[i]=newr;	
	return i;
}
/**************************************************************************/
void race_allocate_race_table()
{
	// allocate our race_table, and link its index positions to list positions
	race_table=(race_data **)calloc(MAX_RACE+2, sizeof(race_data*));	
}

/**************************************************************************/
void race_populate_race_table()
{
	int i=0;
	for(race_data *r=race_list; r; r=r->next){
		if(i>=MAX_RACE){
			bugf("race_populate_race_table(): more than MAX_RACE races were read in (%d)", MAX_RACE);
			log_notef("The mud has currently been compiled to read in only %d races... but the races "
				"file '%s' contains more races that that... either increase MAX_RACE in params.h or "
				"remove some races from the races file.",
				MAX_RACE,
				RACES_FILE);
			exit_error( 1 , "race_populate_race_table", "too many races");
		}
		race_table[i++]=r;
	}
}

/**************************************************************************/
void race_convert_skills()
{
	char skillname[MIL];
	int skill, next_skill_index;
	char *arg;
	race_data *r;

	log_string("race_convert_skills(): Converting race skills format");

	for(r=race_list; r; r=r->next){
		arg=r->load_skills;
		next_skill_index=0;
		while(!IS_NULLSTR(arg)){
			arg=one_argument(arg, skillname);
			skill=skill_lookup(skillname);
			if(skill<0){
				bugf("Unknown skill '%s' for race '%s'... continuing bootup (%s).",
					skillname, r->name, r->load_skills);
			}else{
				if(next_skill_index>=MAX_RACIAL_SKILLS){
					bugf("race_convert_skills(): next_skill_index==%d (>=MAX_RACIAL_SKILLS)"
						"for race '%s', load_skills='%s'", next_skill_index,
						r->name, r->load_skills);
					exit_error( 1 , "race_convert_skills", "too many racial skills on race");
				}
				r->skills[next_skill_index]=skill;
				next_skill_index++;				
			}
		}
		replace_string(r->load_skills, ""); // free the memory
		if(next_skill_index<MAX_RACIAL_SKILLS){
			r->skills[next_skill_index]=-1; // mark the end of the skill list
		}
	}
};

/**************************************************************************/
// read in races from disk
void load_races()
{
	race_data *r, *prev;
    log_string("===Reading in races...");
    GIOLOAD_LIST(race_list, race_data, RACES_FILE);

	// remove any 'unknown' races loaded from the races file
	prev=NULL;
	for(r=race_list; r; r=r->next){
		if(IS_NULLSTR(r->name) || !str_cmp(r->name, "unknown")){
			log_notef("Discarding NULL/'unknown' race read in from %s... the "
				"unknown race must be dynamically generated by the code.",
				RACES_FILE);

			if(prev){
				prev->next=r->next;
			}else{
				race_list=r->next;
			}
			continue;
		}
		prev=r;
	}

	// if race file not found, try to import from old format
	if(!race_list){
		log_note("load_races(): no races loaded - WE NEED TO ATTEMPT TO IMPORT IN HERE!!!");
	}


	// dynamically create a single unknown race and put it at the start
	// of the races list.
	r=race_create_race("unknown");
	r->next=race_list;
	race_list=r;

	race_allocate_race_table();
	race_populate_race_table();

	// patch up the languages, skills etc  + do a count while here
	int count=0;
	bool creation_selectable_race_found=false;
	for(r=race_list; r; r=r->next){
		if(r->start_hp<1){
			r->start_hp=1;
		}
		if(r->creation_selectable()){
			creation_selectable_race_found=true;
		}
		if(IS_NULLSTR(r->load_language)){
			r->language=language_unknown;
		}else{
			r->language=language_lookup(r->load_language);
		}
		if(!r->language){
			logf("unfound language '%s' for race '%s' - creating language.", 
				r->load_language, r->name);
			do_langedit(NULL,FORMATF("create %s",r->load_language));

			r->language=language_lookup(r->load_language);
			if(!r->language){
				logf("Dynamic language creation failed, something is wrong - aborting bootup process.");
				do_abort();
			}
		}
		count++;

		r->update_dynamic_flags();
	}

	// if there are no creation selectable races, make the unknown race 
	// creation selectable so someone can create using it (and therefore 
	// create more races)
	if(!creation_selectable_race_found){
		log_note("load_races()  Note: no creation selectable races were loaded "
			"so the auto generated 'unknown' race has been configured as creation "
			"selectable - enabling you to create a character, then edit the races "
			"using the raceedit command.");
		SET_BIT(race_list->flags, RACEFLAG_PCRACE|RACEFLAG_CREATION_ACTIVATED);
	}

    logf("===Race loading completed - %d race%s in total.", 
		count, count==1?"":"s");
}
/**************************************************************************/
// save race to disk
void save_races()
{
	race_data *r;

	// prepare fields for saving
	for(r=race_list; r; r=r->next){
		 // prepare the load_language value
		replace_string(r->load_language, r->language->name);

		{ // prepare the load_skill value
			char skills_text[MSL];	
			skills_text[0]='\0';
			skills_text[1]='\0';
			for( int i = 0; i<MAX_RACIAL_SKILLS && r->skills[i]!=-1; i++ ) {
				strcat(skills_text, " ");
				strcat(skills_text, pack_word(skill_table[r->skills[i]].name));
			};
			replace_string(r->load_skills, &skills_text[1]);  // skip the leading space
		}
	}
	
    log_string ("===Saving races...");
	assert(!str_cmp(race_list->name, "unknown")); // first race should always be the 'unknown' race
	// save race_list->next, so we don't save the unknown race
    GIOSAVE_LIST(race_list->next, race_data, RACES_FILE, true);
	log_string ("===Races saved.");
}
/**************************************************************************/
// write the exp to play race for a particular class
void race_gioWriteClassExp(gio_type *gio_table, int tableIndex, void *data, FILE *fp){
	
	race_data * r;
	int classIndex;
	
	r= (race_data*) data;

	fprintf(fp, "%s\n", gio_table[tableIndex].heading);
	for (classIndex=0; !IS_NULLSTR(class_table[classIndex].name); classIndex++){
		// only write the classes with valid xp per level values
		if (r->class_exp[classIndex]==0 || (
			r->class_exp[classIndex]>=1000 
			&& r->class_exp[classIndex]<=20000)){
			fprintf(fp, "\t%s %d\n", pack_word(class_table[classIndex].name), 
				r->class_exp[classIndex]);
		}else{
			fprintf(fp, "\t*%s_has_an_invalid_xp_modifer %d\n", class_table[classIndex].name, 
				r->class_exp[classIndex]);
		}
	};
	fprintf(fp, "~\n");
}

/**************************************************************************/
int create_class(char * class_name);
/**************************************************************************/
// race the exp to play race for a particular class
void race_gioReadClassExp(gio_type *, int, void *data, FILE *fp)
{
	race_data* r;
	char className[MIL];
	int xpMod;
	int classIndex;

	r= (race_data*) data;

	// initialise all values to 0
	for (classIndex=0; class_table[classIndex].name; classIndex++){
		r->class_exp[classIndex]=0;
	}

	while (true){
		// get the class name and modifier
		//fscanf(fp, "%s ", className);
		strcpy(className,fread_word(fp));
		if (className[0]=='~' && className[1]=='\0'){
			break;
		}

		// read in the experience modifier
		xpMod=fread_number(fp);
//		fscanf(fp, "%d", &xpMod);

		classIndex =class_lookup(className);

		if (classIndex<0){
			if (className[0]!='*'){
				// add a new class if necessary
				classIndex=create_class(className);
				if(classIndex<0){
					bugf( "race_gioReadClassExp: class '%s' not found... problems dynamically adding... aborting!", className);
					do_abort();
				}else{
					bugf( "race_gioReadClassExp: class '%s' not found...\n"
						  "********* Dynamically added %s - creation selectable = false.", 
						  className, className);
					r->class_exp[classIndex]=xpMod;
				}
			}
		}else{
			r->class_exp[classIndex]=xpMod;
		}
	}
}

/**************************************************************************/
void race_data::update_dynamic_flags()
{ 
	// pcrace_with_no_classxp_set flag
	if(!IS_SET(flags, RACEFLAG_PCRACE)){
		REMOVE_BIT(flags, RACEFLAG_PCRACE_WITH_NO_CLASSXP_SET);
	}else{
		int count=0;
		for ( int i=0; !IS_NULLSTR(class_table[i].name); i++){ 
			if(class_exp[i]){ 
				count++; 
			}	
		} 
		if(count){
			REMOVE_BIT(flags, RACEFLAG_PCRACE_WITH_NO_CLASSXP_SET);
		}else{
			SET_BIT(flags, RACEFLAG_PCRACE_WITH_NO_CLASSXP_SET);
		}
	}
};
/**************************************************************************/
void display_race_selection(connection_data *d);
/**************************************************************************/
// Kal - Aug 01
void do_raceinfo( char_data *ch, char *argument )
{
	char titlebar[MIL];
	int stat, race, col_index=1;
	bool customization_disabled=false;
	char buf[MIL], linebuf[MSL];
	char col;
	int total;		
	connection_data *d=ch->desc;
	if(!d){
		ch->println("You have to be connected to use this command.");
		return;
	}

	// display races
	ch->print_blank_lines(1);
	if(IS_IMMORTAL(ch)){
		ch->println("`xThe game currently has the following player races (`Y*`x=not creation selectable):");
	}else{
		ch->println("`xThe game currently has the following player selectable races:");
	}

	// create and display a titlebar
	sprintf(titlebar, "%s====`srace name`m====`sst`m==`squ`m==`spr`m==`sem`m==`sin`m==`sco`m==`sag"
		"`m==`ssd`m==`sme`m==`sre`m=`stotal`m==`scp`m===`shp`m==`x",
		(GAMESETTING(GAMESET_REMORT_SUPPORTED)?"`sremort`m":"`m====="));	
	ch->println(titlebar);

	for( race = 1; race_table[race]; race++ )
	{
		if(!race_table[race]->pc_race())
			continue;

		if(!IS_IMMORTAL(ch)){
			if(!race_table[race]->creation_selectable()){
				continue;
			}
			if(IS_SET(race_table[race]->flags, RACEFLAG_ALWAYS_HIDDEN_FROM_MORTAL_RACEINFO)){
				continue;
			}

			if(GAMESETTING(GAMESET_REMORT_SUPPORTED)
				&& ch->desc 
				&& ch->desc->connected_state == CON_PLAYING
				&& IS_SET(race_table[race]->flags, RACEFLAG_HIDDEN_FROM_MORTAL_RACEINFO_WHEN_ABOVE_THEIR_REMORT)
				&& race_table[race]->remort_number > ch->remort)
			{
				continue;
			}
			
		}
		
		if(ch->desc && ch->desc->connected_state != CON_PLAYING){
			if(race_table[race]->remort_number > ch->desc->creation_remort_number)
				continue;
		}

		++col_index%=2;

		col= col_index?'M':'x';
		
		linebuf[0]='\0';

		if(GAMESETTING(GAMESET_REMORT_SUPPORTED)){
			sprintf(buf,"`s%d",race_table[race]->remort_number);			
			strcat(linebuf,buf);
		}
		sprintf(buf,"`%c%21s`%c=%s ", col, FORMATF("`=_%s",race_table[race]->name), 
			col, race_table[race]->creation_selectable()?" ":"`Y*");
		strcat(linebuf,buf);

		total=0;
		for(stat = 0; stat < MAX_STATS; stat++)
		{
			sprintf(buf, "`%c%+3d`%c=",
				race_table[race]->stat_modifier[stat]<0?
					'R':race_table[race]->stat_modifier[stat]==0?'g':'B',
				race_table[race]->stat_modifier[stat],
				col);
			strcat(linebuf,buf);
			total+=race_table[race]->stat_modifier[stat];
		}		

		// display the total value as well
		sprintf(buf, "`%c%+4d`%c=", total<0?'R':total==0?'g':'B',total,col);
		strcat(linebuf,buf);

		sprintf(buf," %3d %4d", 
			race_table[race]->points,
			race_table[race]->max_hp); 
		strcat(linebuf,buf);

		if(!GAMESETTING5(GAMESET5_CREATION_DISABLE_CUSTOMIZATION) 
			&& IS_SET(race_table[race]->flags, RACEFLAG_NO_CUSTOMIZATION))
		{
			strcat(linebuf,"`Y-`x");	
			customization_disabled=true;
		}
		strcat(linebuf,"\r\n");
		ch->print(linebuf);
	}	
	// repeat the titlebar
	ch->println(titlebar);
	if(customization_disabled ){
		ch->println("`Y-`x = this race has no advanced customization available.");
	}
	ch->print_blank_lines(1);
}
/**************************************************************************/
void do_saveraces( char_data *ch, char * )
{
	save_races();
    log_string("save completed");
    ch->println("Races manually saved.");
}

/**************************************************************************/
// Kalahn - August 98
void do_npcinfo( char_data *ch, char *)
{
	int raceIndex, raceCount=0;

	ch->titlebar("NPC INFO");

	for ( raceIndex= 0; race_table[raceIndex]; raceIndex++ )
	{
		raceCount++;
		ch->printf(" `x%-15s `G%4d`g%5.1f%%`M%4d`m%5.1f%%  `x", 
			race_table[raceIndex]->name,
			race_table[raceIndex]->inuse,
			((float)(race_table[raceIndex]->inuse*100)/(float)total_npcracescount),
			race_table[raceIndex]->areacount,
			((float)(race_table[raceIndex]->areacount*100)/(float)top_area));
		if (raceCount%2==0)
		{
			ch->println("");
		}
    }

	if (raceCount%2==1)
	{
		ch->println("");
	}

	ch->printf("\r\n  `RTOTALS: `xTotal number of NPC races: %d/%d\r\n"
		"  Total number of mobs: `G%-5d`x       Total number of areas: `M%3d`x\r\n"
		"  Total number of different races used in different areas: `Y%3d`x\r\n",
		raceCount, MAX_RACE, total_npcracescount, top_area, total_npcareacount);


};
/**************************************************************************/
void do_racelist( char_data *ch, char *arg)
{
	bool editing=false;
	if(!str_cmp("edit",arg)){
		editing=true;
	}
	int count=-1;
	int ri;
	char name[MIL];
	// loop thru showing all the creation selectable races
	for( ri= 1; race_table[ri]; ri++ ){
		if(!race_table[ri]->creation_selectable()){
			continue;
		}
		sprintf(name, "%-16s",race_table[ri]->name);
		if(GAMESETTING(GAMESET_REMORT_SUPPORTED)){
			ch->printf(" `S[%d]`x`Y%s", 
				race_table[ri]->remort_number,
				editing?mxp_create_send(ch,FORMATF("raceedit %s", race_table[ri]->name), name):name);
		}else{
			ch->printf("    `Y%s", editing?mxp_create_send(ch,FORMATF("raceedit %s", race_table[ri]->name), name):name);
		}
		if(++count%3==2){
			ch->print_blank_lines(1);
		}
	}
	// loop thru showing all the non creation selectable races
	for(ri= 1; race_table[ri]; ri++ ){
		if(race_table[ri]->creation_selectable()){
			continue;
		}
		sprintf(name, "   %-16s",race_table[ri]->name);
		ch->printf(" `x%s", editing?mxp_create_send(ch,FORMATF("raceedit %s", race_table[ri]->name), name):name);
		if(++count%3==2){
			ch->print_blank_lines(1);
		}
	}
	if(count!=2){
		ch->print_blank_lines(1);
	}
	ch->println("Yellow races are creation selectable.`x");
}
/**************************************************************************/

