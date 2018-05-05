/**************************************************************************/
// dynamics.cpp - code for dynamic loading of tables etc
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "dynamics.h"
#include "magic.h"

/**************************************************************************/
// Prototypes 
extern dynlookup_type dynlookup_table[];
extern dynspell_type spellpairs_table[];
char *flag_string( const struct flag_type *flag_table, int bits );
char *tochar_spellfunction(SPELL_FUN *psp);
sh_int spellfunctionindex_fromchar(const char *name);
int class_lookup (const char *name);
bool flush_char_outbuffer(char_data *ch);
void do_write_socials( char_data *ch, char * );

/**************************************************************************/
// Returns the index of the class it just created, -1 if couldn't create it
// if a class with the same name already exists, it returns that class
// all classes created using this function have creation selectable set to false
// - Kal March 00
int create_class(char * class_name)
{
	int result;
	result=class_exact_lookup(class_name);
	if(result!=-1){
		bugf("create_class(): class '%s' already exists!", class_name);
		return result;
	}

	// find the end of the table
	int i;
	for(i=0;!IS_NULLSTR(class_table[i].name); i++){
		// do nothing but count thru table
	}

	// check if we are going to overflow the table
	if(i+1>=MAX_CLASS){
		bugf("create_class(): attempting to add class '%s', but there are already %d classes (as defined by MAX_CLASS), this would exceed the maximum - canceling",
			class_name, MAX_CLASS);
		return -1;
	}

	class_table[i+1].name=NULL; // mark the new end of the table
	class_table[i].name=str_dup(class_name);
	class_table[i].short_name=str_dup("???");

	char temp[MIL];
	sprintf(temp,"%s default", class_name);
	class_table[i].default_group=str_dup(temp);

	class_table[i].spinfo_letter=str_dup("-");
	class_table[i].creation_selectable=false;
	class_table[i].hp_min=1;
	class_table[i].hp_max=5;
	class_table[i].skill_adept=75;
	class_table[i].remort_number=0;
	class_table[i].remort_to_classes=str_dup("");

	// add our entry in the classnames_table
	classnames_flags[i].name=str_dup(class_table[i].name);
	classnames_flags[i].bit=1<<i;
	classnames_flags[i].settable=true;
	classnames_flags[i+1].name=NULL; // mark end of table

	// clear out the class specific entries in 
	// the other tables for the new class
	{
		int j;
		// skill_table
		for(j=0;!IS_NULLSTR(skill_table[j].name);j++){						
			skill_table[j].skill_level[i]=0;
			skill_table[j].rating[i]=0;
			skill_table[j].low_percent_level[i]=0;
			skill_table[j].maxprac_percent[i]=0;
			skill_table[j].learn_scale_percent[i]=0;
			skill_table[j].alignrestrict_flags[i]=0;
		}
		if(race_table){
			for(j=0; race_table[j]; j++){						
				race_table[j]->class_exp[i]=1000;
			}
		}
	}


	logf("create_class(): dynamically added class '%s'.", class_name);

	return i;
}
/**************************************************************************/
// Cant be bothered writing word information for the class table :)
GIO_START(class_type)
	GIO_STRH(short_name,	"Who_name         ")
	GIO_SHINT_ARRAY(attr_prime, 2)
	GIO_SHINTH(skill_adept, "skill_adept      ")
	GIO_SHINTH(thac0_00,	"thac0_00         ")
	GIO_SHINTH(thac0_32,	"thac0_32         ")
	GIO_SHINTH(hp_min,		"hp_min           ")
	GIO_SHINTH(hp_max,		"hp_max           ")
	GIO_BOOLH( fMana,		"fMana            ")
	GIO_STRH(spinfo_letter,	"spinfo_letter    ")
	GIO_STRH(default_group,	"default_group    ")
	GIO_BOOLH(creation_selectable,	"creation_selectable ")
	GIO_INTH(class_cast_type,  "class_cast_type  ") // don't need this once file format has been converted
	GIO_WFLAGH(class_cast_type,"cast_type        ", castnames_types)
	GIO_SHINTH(core_clss,	 "core_clss        ")
	GIO_INTH(object_restriction_index, "object_restriction_index ")
	GIO_INTH(objrestrict,	"objrestrict      ")
	GIO_SHINTH(class_id,	"class_id         ")
	GIO_WFLAGH(flags,		"Flags            ", classflag_flags)
	GIO_SHINTH(remort_number,"remort_number    ")
	GIO_STRH(remort_to_classes,	"remort_to_classes ")
	GIO_INT(recall)	
	GIO_INT(morgue)	
	GIO_READ_TO_EOL("base_group") // remove from system
	GIO_STR_ARRAYH(pose_self,	"pose_self        ", MAX_LEVEL)
	GIO_STR_ARRAYH(pose_others,	"pose_others      ", MAX_LEVEL)
	GIO_STR(newbie_prac_location_hint)
	GIO_STR(newbie_train_location_hint)
	GIO_INT(pendant_vnum)
	GIO_INT(newbie_map_vnum)
	
GIO_FINISH_NOCLEAR	
/**************************************************************************/
void skilltype_write_generic_short_class_entry(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	short *array=(short*)((char *)data + gio_table[tableIndex].index);	
	int i;

	// make sure we have something to write before starting into it
	for(i=0; !IS_NULLSTR(class_table[i].name); i++)
	{
		if(array[i]!=0){
			break;
		}
	}
	if(IS_NULLSTR(class_table[i].name)){
		return; // nothing to write
	}

	// put out the header
	fprintf(fp, "%s\n",gio_table[tableIndex].heading);
	// the data for the non 0 classes
	for(i=0; !IS_NULLSTR(class_table[i].name); i++)
	{	
		if(array[i]!=0){
			if(has_space(class_table[i].name)){
				fprintf(fp, "'%-16s' %d\n",		
					class_table[i].name,
					array[i]);
			}else{
				fprintf(fp, "%-16s %d\n",		
					class_table[i].name,
					array[i]);
			}
		}
	}
	// the footer
	fprintf(fp, "/\n");
}
/**************************************************************************/
void skilltype_read_generic_short_class_entry(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	short *array=(short*)((char *)data + gio_table[tableIndex].index);	
	char *className;
	short value;
	int classIndex;
	
	// table has already been nulled by the gio system

	while (true){
		// get the class name and modifier
		className=fread_word(fp);
		if (className[0]=='/' && className[1]=='\0'){
			break;
		}

		// read in the experience modifier		
		value=fread_number(fp);

		classIndex =class_lookup(className);

		if (classIndex<0){
			if (className[0]!='*'){
				bugf( "skilltype_read_generic_short_class_entry: class '%s' not found... "
					"ignoring '%s' field for that class!", className, gio_table[tableIndex].heading);
			}
		}else{
			array[classIndex]=value;
		}
	}
}
/**************************************************************************/
GIO_CUSTOM_FUNCTION_PROTOTYPE( skilltype_write_spellfunction );
GIO_CUSTOM_FUNCTION_PROTOTYPE( skilltype_read_spellfunction );

GIO_CUSTOM_FUNCTION_PROTOTYPE(racetype_write_generic_races_set_for_n_array);
GIO_CUSTOM_FUNCTION_PROTOTYPE(racetype_read_generic_races_set_for_n_array);
/**************************************************************************/
// create skill_type GIO lookup table 
GIO_START(skill_type)
// *** char * name				- is written by the table handling code
//###TODO skill_level[MAX_CLASS]	
//###TODO rating[MAX_CLASS]
#define	STWGSCE skilltype_write_generic_short_class_entry // work smarter not harder :)
#define	STRGSCE skilltype_read_generic_short_class_entry 
GIO_CUSTOM_WRITEH(skill_level,			"===skill_level",			STWGSCE)
GIO_CUSTOM_WRITEH(rating,				"===rating",				STWGSCE)
GIO_CUSTOM_WRITEH(low_percent_level,	"===low_percent_level",		STWGSCE)
GIO_CUSTOM_WRITEH(maxprac_percent,		"===maxprac_percent",		STWGSCE)
GIO_CUSTOM_WRITEH(learn_scale_percent,	"===learn_scale_percent",	STWGSCE)
GIO_CUSTOM_WRITEH(alignrestrict_flags,	"===alignrestrict_flags",	STWGSCE) // storing as a number (doesn't matter really)
GIO_CUSTOM_READH(skill_level,			"===skill_level",			STRGSCE)
GIO_CUSTOM_READH(rating,				"===rating",				STRGSCE)
GIO_CUSTOM_READH(low_percent_level,		"===low_percent_level",		STRGSCE)
GIO_CUSTOM_READH(maxprac_percent,		"===maxprac_percent",		STRGSCE)
GIO_CUSTOM_READH(learn_scale_percent,	"===learn_scale_percent",	STRGSCE)
GIO_CUSTOM_READH(alignrestrict_flags,	"===alignrestrict_flags",	STRGSCE) 

GIO_CUSTOM_WRITEH(spell_fun,	"SpellFunction   ", skilltype_write_spellfunction)
GIO_CUSTOM_READH(spell_fun,		"SpellFunction   ", skilltype_read_spellfunction)
GIO_SHWFLAGH(minimum_position,	"Position        ", position_types)
GIO_SHINTH(min_mana,			"Mana            ")
GIO_SHINTH(beats,				"Beats           ")
GIO_STRH(noun_damage,			"Noun_damage     ")
GIO_STRH(msg_off,				"Wearoff_msg     ")
GIO_STRH(msg_obj,				"Obj_wearoff_msg ")
GIO_CUSTOM_WRITEH(race_restrict_n, "RaceRestriction ", racetype_write_generic_races_set_for_n_array)
GIO_CUSTOM_READH(race_restrict_n,  "RaceRestriction ", racetype_read_generic_races_set_for_n_array)
GIO_WFLAGH(flags,				"Flags           ", skflags_flags)

GIO_WFLAGH(category,			"Category        ", category_types)
GIO_WFLAGH(sktype,				"Type            ", sktype_types)
GIO_SHWFLAGH(damtype,			"Damtype         ", damtype_types)
GIO_WFLAGH(realms,				"Realms          ", realm_flags)
GIO_WFLAGH(spheres,				"Spheres         ", sphere_flags)
GIO_WFLAGH(elements,			"Elements        ", element_flags)
GIO_WFLAGH(spellgroup,			"Spell_Group     ", spell_group_flags)
GIO_WFLAGH(sect_restrict,		"Sector_Restrict ", sectorbit_flags)
GIO_WFLAGH(sect_enhance,		"Sector_Enhance  ", sectorbit_flags)
GIO_WFLAGH(sect_dampen,			"Sector_Dampen   ", sectorbit_flags)
GIO_SHINTH(component_based,		"Component       ")
GIO_STRH(msp_sound,				"MSPSound        ")
GIO_FINISH_NOCLEAR

/**************************************************************************/
// Output the class_table to disk 
void do_write_classes(char_data *ch, char *)
{
	FILE *fp;
	int i;

	logf("Writing class table to " CLASSES_LIST ".write ...");
	fclose( fpReserve );

    if ( ( fp = fopen( CLASSES_LIST ".write", "w" ) ) == NULL )
    {
		bugf("do_write_classes(): fopen '%s' for write - error %d (%s)",
			CLASSES_LIST ".write", errno, strerror( errno));
		ch->printf("An error occurred opening " CLASSES_LIST ".write for writing!\r\n");
		autonote(NOTE_SNOTE, "do_write_classes()", "Problems saving class table", "code cc: imm", 
			"An error occurred opening " CLASSES_LIST ".write for writing!\r\n", true);
    }else{
		// LOOP thru everything in the table, writing it
		for ( i = 0; !IS_NULLSTR(class_table[i].name); i++ )
		{
			fprintf(fp,"######NAME        %s~\n", class_table[i].name);
			GIO_SAVE_RECORD(class_type, &class_table[i], fp, NULL);
			fprintf(fp,"\n");
		}

		int bytes_written=fprintf(fp, "EOF~\n");
		fclose( fp );
		if(   bytes_written != str_len("EOF~\n") ){
			bugf("do_write_classes(): fprintf to '%s' - error %d (%s)",
				CLASSES_LIST ".write", errno, strerror( errno));
			bugf("Incomplete write of " CLASSES_LIST ".write, write aborted - check diskspace!");
			ch->printf("Incomplete write of " CLASSES_LIST ".write, write aborted - check diskspace!\r\n");
			autonote(NOTE_SNOTE, "do_write_classes()", "Problems saving class table", "code cc: imm", 
				"Incomplete write of " CLASSES_LIST ".write, write aborted - check diskspace!\r\n", true);
		}else{		
			ch->printf("Finished writing class table to "CLASSES_LIST ".write\r\n");

			ch->printf("Renaming old " CLASSES_LIST " to " CLASSES_LIST ".bak\r\n");
			unlink(CLASSES_LIST".bak");
			rename(CLASSES_LIST, CLASSES_LIST".bak");

			ch->printf("Renaming new " CLASSES_LIST ".write to " CLASSES_LIST "\r\n");
			unlink(CLASSES_LIST);
			rename(CLASSES_LIST".write", CLASSES_LIST);
		}
    }
	fpReserve = fopen( NULL_FILE, "r" );
	logf("Finished writing class table.");
}
/**************************************************************************/
// read in the class_table from disk
void do_read_classes(char_data *ch, char *)
{
	FILE *fp;
	int count=0;
	int i;

	// because system doesn't support reassigning class numbers, only allow 
	// this to run at bootup and when one imm is on
	if(runlevel!=RUNLEVEL_BOOTING){
//		if(player_list && !player_list->next_player){ // only one in the game
//			ch->println("Allowing class read in due to a single person being logged in.");
//		}else{
			ch->println("do_read_classes() currently doesn't support being called a game time... boot only!");
			ch->println("Doing so in its current state might stuff up everyone's class,");
			ch->println("and object classrestrictions in area files... DON'T DO IT!");
			return;
//		}
	}

	logf("Reading in class table from " CLASSES_LIST "...");
	fclose( fpReserve );

    if ( ( fp = fopen( CLASSES_LIST, "r" ) ) == NULL )
    {	// couldn't find the file to read in
		bugf("do_read_classes(): fopen '%s' for read - error %d (%s)",
			CLASSES_LIST, errno, strerror( errno));
		bugf("An error occurred trying to open " CLASSES_LIST " for reading!");
		ch->printf("An error occurred trying to open " CLASSES_LIST " for reading!\r\n");
		if(file_exists(CLASSES_LIST )){
			bugf("File " CLASSES_LIST "found, but it wasn't opened for some reason?!?");
			write_shutdown_file(NULL);
			do_abort();
		}else{
			if(runlevel==RUNLEVEL_BOOTING){
				log_notef("File " CLASSES_LIST " not found... the mud can't boot without this file!`1"
				"`1"
				"This file contains the basic information about all classes, if the mud "
				"booted without this file then there would be no classes, and therefore "
				"none would be able to login.`1"
				"`1"
				"If you are in the process of setting up this mud, it is most likely that you "
				"either haven't downloaded the dawn support files (which include the required system "
				"files, helps and optional area set), or haven't correctly installed the support files.  "
				"Download and install them now, then if you are still having "
				"problems which you can't resolve, ask for assistance on the dawn forums at "
				"http://forums.dawnoftime.org/`1"
				"`1"
				"If this error has just started to occur on a mud which was previously "
				"working, the information of the previous class table might be "
				"contained in " CLASSES_LIST ".bak or "
				"secondly " CLASSES_LIST ".write... (the bak file is probably the better "
				"of the two if you have both) if you have either of these files copy them to "
				CLASSES_LIST" then restart the mud.`1");
				write_shutdown_file(NULL);
				exit_error( 1 , "do_read_classes", "classes list not found");
			}else{
				ch->printf("Couldn't find " CLASSES_LIST " to read in! - read the inote for more detail!\r\n");
				autonote(NOTE_SNOTE, "do_read_classes()", "Problems finding class table file", "code cc: imm", 
					"Couldn't find " CLASSES_LIST " to read into the class table!`1"
					"The mud will not be able to automatically start up if this file "
					"is not there next time the mud starts... the recommended fix to "
					"this condition (`RAssuming there is NO corruption to the class "
					"information in the currently running mud`x) is to make sure there "
					"is enough free diskspace to store this file then get the mud to "
					"recreate this file using the write_classes command.`1`1"
					"It is important that everyone does not do this, it should be done "
					"only once, and it is best if it is not done if the mud has "
					"already rebooted since the date of this note... if you don't "
					"understand what this note is explaining, it is best you leave it.",					
					true);
			}
		}
    }else{ // CLASSES_LIST was found, read in
		bool morefile=true;
		char *readword;
		
		if(runlevel==RUNLEVEL_BOOTING){
			// *** Mark the existing table as empty - since we are loading everything in
			// - This could cause class corruption if done when people are online...
			//   that is why it can be only done if one is online or during booting.
			class_table[0].name=NULL; 
		}

		for ( i=0; i<MAX_CLASS; i++)
		{ 
			class_table[i].already_loaded=false;
		}

		// loop thru till we get the end of the table
		while (morefile && !feof(fp)) {
			int load_index;
			readword= fread_word(fp);

			if (!str_cmp(readword, "EOF") || !str_cmp(readword, "EOF~")){
				morefile=false;
			}else{
				if(!str_cmp(readword, "######NAME")){
					// get the name of the class
					readword=fread_string(fp); // freed

					// check if the class hasn't already been loaded - no duplicates
					load_index=class_exact_lookup(readword);
					
					if(load_index==-1){ // if not found, allocate a space for a new class
						// find the end of the class table
						for ( load_index=0; !IS_NULLSTR(class_table[load_index].name); load_index++)
						{ // do nothing till we find the end
						}
						//load_index now points to the end of the list
						if(load_index>=MAX_CLASS){
							bugf("Too many classes (%d) trying to load class '%s' from %s, increase MAX_CLASS - aborting",
									i, readword, CLASSES_LIST);
							ch->printlnf("Too many classes (%d) trying to load class '%s' from %s, increase MAX_CLASS - aborting",
									i, readword, CLASSES_LIST);
							flush_char_outbuffer(ch);
							write_shutdown_file(NULL);
							exit_error( 1 , "do_read_classes", "too many classes");
						}
						class_table[load_index+1].name=NULL; // mark the new end of the table
						class_table[load_index].name=str_dup(readword);

					}
					// prevent duplicate classes in the file
					if(class_table[load_index].already_loaded){
						bugf("Duplicate class '%s' in %s, aborting",
								readword, CLASSES_LIST);
						ch->printlnf("Duplicate class '%s' in %s, aborting",
								readword, CLASSES_LIST);
						flush_char_outbuffer(ch);
						write_shutdown_file(NULL);
						exit_error( 1 , "do_read_classes", "duplicate class");
					}

					// initialise the pose data
					for(i=0; i<MAX_LEVEL; i++){
						class_table[load_index].pose_self[i]=str_dup("");
						class_table[load_index].pose_others[i]=str_dup("");
					}
					class_table[load_index].remort_to_classes=str_dup("");
					class_table[load_index].newbie_prac_location_hint=str_dup("");
					class_table[load_index].newbie_train_location_hint=str_dup("");

					// read in the data
					GIO_LOAD_RECORD(class_type, &class_table[load_index], fp);
					class_table[load_index].already_loaded=true;
				
					count++;
					free_string(readword);
				}else{// unexpected file format
					bugf("Unexpected fileformat in '%s' - found '%s' "
						"expecting '######NAME'", CLASSES_LIST , readword);
					ch->printf( "Unexpected fileformat in '%s' - found '%s' "
						"expecting '######NAME'\r\n", CLASSES_LIST , readword);
					flush_char_outbuffer(ch);
					write_shutdown_file(NULL);
					do_abort();
					return;
				}
			}
		}
		fclose( fp );

		ch->printlnf("Finished reading class_table from " CLASSES_LIST  ". (read in %d)", count);
		logf("Finished reading class_table from " CLASSES_LIST ". (read in %d)", count);

		for ( i=0; !IS_NULLSTR(class_table[i].name); i++){
			if(!class_table[i].already_loaded){
				ch->printlnf("Class '`R%s`x' wasn't found in file to load in!, after next reboot it wont be there.",
					class_table[i].name);
				bugf("Class '%s' wasn't found in file to load in.",
					class_table[i].name);
			}
		}

    }
	fpReserve = fopen( NULL_FILE, "r" );
	
	// create a new the classnames_types
	{
		int i;
		logf("Creating a new class lookup table in classnames_types[]");

		// first attempt exact match 
		for (i=0; !IS_NULLSTR(class_table[i].name); i++)
		{	
			classnames_flags[i].name=str_dup(class_table[i].name);
			classnames_flags[i].bit=1<<i;
			classnames_flags[i].settable=true;
		}
		classnames_flags[i].name=NULL; // mark end of table
	}

	return;
}
/**************************************************************************/
char *tochar_spellfunction(SPELL_FUN *psp)
{
	sh_int sn;

    for ( sn = 0; !IS_NULLSTR(spellpairs_table[sn].name); sn++ )
    {
		if(spellpairs_table[sn].psp==psp){
			return spellpairs_table[sn].name;
		}
	}
	bugf("tochar_spellfunction: spell function not found, returning "
		"\"spell_null\" sn=%d",sn);
	return "spell_null";
}
/**************************************************************************/
sh_int spellfunctionindex_fromchar(const char *name)
{
	sh_int sn;

    for ( sn = 0; !IS_NULLSTR(spellpairs_table[sn].name); sn++ )
    {
		if(!str_cmp(spellpairs_table[sn].name,name)){
			return sn;
		}
	}
	return -1;
}
/**************************************************************************/
// write the SpellFunction section
void skilltype_write_spellfunction(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	skill_type * psk;

	psk= (skill_type*) data;

	if(	psk->spell_fun!= NULL 
		&& psk->spell_fun!=spell_null)
	{
		fprintf(fp, "%s %s~\n",		
			gio_table[tableIndex].heading,
			tochar_spellfunction(psk->spell_fun));
	}
}
/**************************************************************************/
// read the spell function in
void skilltype_read_spellfunction(gio_type *, int, void *data, FILE *fp)
{
	skill_type * psk;
	char *pstr;

	psk= (skill_type*) data;

	pstr=fread_string(fp);
	psk->spell_function_index=spellfunctionindex_fromchar(pstr);
	if(psk->spell_function_index>-1){
		psk->target=spellpairs_table[psk->spell_function_index].target;
		psk->spell_fun= spellpairs_table[psk->spell_function_index].psp;
	}else{
		psk->spell_fun=spell_null;
		psk->target=TAR_IGNORE;
	}
	
	free_string(pstr);
}
/**************************************************************************/
// writes to the fp a table for cross referencing spell names and
// their spell function, this assumes that the function is named based
// of the actual spell name in the table
void write_dynlookup_table(FILE *fp)
{
    sh_int sn;
	char buf[MIL];
	char type[MIL];
	fprintf(fp,"dynlookup_type dynlookup_table[]=" BRACKET_OPEN "\n");

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
		if ( skill_table[sn].name == NULL )
		    break;
		sprintf(buf,"\"%s\",",skill_table[sn].name);

		// generate the type
		if(IS_SPELL(sn)){
			sprintf(type,"DYNTYPE_SPELL");
		}else if(IS_REALM(sn)){
			sprintf(type,"DYNTYPE_REALM");
		}else if(IS_SKILL(sn)){
			sprintf(type,"DYNTYPE_SKILL");
		}else{
			sprintf(type,"DYNTYPE_UNDEFINED");
		}

		fprintf(fp, "\t` %-25s %s, &gsn_%s },\n", 
			buf, type, underscore_word(skill_table[sn].name)
			);		
    }
	fprintf(fp,"\t` NULL, DYNTYPE_UNDEFINED, NULL}\n");
	fprintf(fp,"};\n\n");
}
/**************************************************************************/
void write_spellpairs_table(FILE *fp)
{
    sh_int sn;
	fprintf(fp,"dynspell_type spellpairs_table[]="BRACKET_OPEN"\n");
	fprintf(fp, "\t` \"spell_null\", spell_null },\n");

    for ( sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++ )
    {
		if(skill_table[sn].spell_fun== NULL 
			|| skill_table[sn].spell_fun==spell_null)
			continue;

		fprintf(fp, "\t` \"spell_%s\", spell_%s, false},\n", 
			underscore_word(skill_table[sn].name),
			underscore_word(skill_table[sn].name));
    }
	fprintf(fp,"\t` \"\", NULL}\n");
	fprintf(fp,"};\n");
}
/**************************************************************************/
// this function is no longer used, but included because someone one day
// may find it useful - Kal, Nov 03
void do_write_dynamic_include(char_data *ch, char *)
{
	FILE *fp;
	fclose( fpReserve );
#ifndef WIN32
    if ( ( fp = fopen( DYNAMIC_INCLUDE, "w" ) ) == NULL )
    {
		bugf("do_write_dynamic_include(): fopen '%s' for write - error %d (%s)",
			DYNAMIC_INCLUDE, errno, strerror( errno));
		ch->printf("An error occurred! writing to " DYNAMIC_INCLUDE "\r\n");
    }
#else
	if ( ( fp = fopen( SRC_DIR DYNAMIC_INCLUDE, "w" ) ) == NULL )
    {
		bugf("do_write_dynamic_include(): fopen '%s' for write - error %d (%s)",
			SRC_DIR DYNAMIC_INCLUDE, errno, strerror( errno));
		ch->printf("An error occurred! writing to " SRC_DIR DYNAMIC_INCLUDE "\r\n");
    }
#endif
    else
    {
		fprintf(fp,
		"// Dyntable.cpp - The Dawn of time dynamically written cpp file.\n"
		"// used for dynamic table lookups.\n"
		"#include \"include.h\"\n"
		"#include \"dynamics.h\"\n"
		"#include \"magic.h\"\n\n");
		write_dynlookup_table(fp);
		write_spellpairs_table(fp);
	
		fclose( fp );
		ch->println("Done.");
    }
	fpReserve = fopen( NULL_FILE, "r" );

	return;
}
/**************************************************************************/
// initialises the skill table with the info in dynlookup_table stored in
// dyntable.cpp
void init_skilltable(void)
{
	static skill_type skill_zero;
	sh_int sn;
	int count=0;
	skill_zero.spell_fun=spell_null;
	
	logf("init_skilltable: initialising table...");
	// add all our entries
	for ( sn = 0; !IS_NULLSTR(dynlookup_table[sn].name); sn++ )
    {
		skill_table[sn]=skill_zero;
		skill_table[sn].name=str_dup(dynlookup_table[sn].name);
		skill_table[sn].pgsn=dynlookup_table[sn].pgsn;
		if ( skill_table[sn].pgsn != NULL )
			*skill_table[sn].pgsn = sn;
		count++;
    }

	// mark the end of the table
	skill_table[sn].name=&str_empty[0];
	logf("init_skilltable: %d entr%s added.", count, count==1?"y":"ies");
}
/**************************************************************************/
// a basic linked list function which sorts insertion by name alphabetically
void addlist(name_linkedlist_type **list,char *name, int tag, bool duplicates, bool reversed)
{
	//handle the easy cases first
	name_linkedlist_type *plist,*newnode, *prev;
	int val=0;

	if(IS_NULLSTR(name))
		return;

	newnode=new name_linkedlist_type;
	newnode->name=str_dup(name);
	newnode->tag=tag;
	newnode->count=1;
//	logf("added %s", name);
	plist=*list;
	prev=NULL;
	if(plist){// insert sorted into the list	

		// loop thru until
		while(plist)
		{
			val=strcmp(plist->name,name);
			if(reversed){
				if(val<0) // later
					break;
			}else{
				if(val>=0) // earlier
					break;
			}
			prev=plist;
			plist=plist->next;
		}

		// add it into the list if not a duplicate
		if(duplicates || val)
		{
			if(prev){
				newnode->next=prev->next;
				prev->next=newnode;
			}else{
				newnode->next=*list;
				*list=newnode;
			}
		}else{ 
			plist->count++; // increase the count of number of duplicates
			// deallocate the memory
			free_string(newnode->name);
			delete newnode;
		}
	}else{// new list, just add us at the top
		newnode->next=NULL;
		*list=newnode;
	}

};
/**************************************************************************/
// loads in the skill table using the following steps
// - Reading in the skill file, making linked lists of the 
//   different skills/spells/realms etc while loading, sorting them 
//   alphabetically while loading in.  These linked lists only hold the 
//   name of the entry, if this is successful it continues.
// - Clears the skill_table
// - These names are then copied into the skill table and the first_spell
//   and last_spell short ints are assigned.  Also in the copy it leaves
//   space after the bottom spell, and the first skill allowing the addition
//   of a few spells without rebooting. (after the next reboot there 
//   will be more space)
// - The gsn pointers are then copied, and their values are set. 
// - The skill text file is then reread, this time copying in the information
//   from the text file into the appropriate places
void do_loadskilltable(char_data *ch, char *)
{
	FILE *fp;
	int count, index;
	skill_type skill_prescan;
	int sn;

	// declare and setup our reading in lists.
	name_linkedlist_type *list[10], *plist;
	for(count=0;count<10; count++){
		list[count]=NULL;
	}
	count=0;

	static skill_type skill_zero;
	skill_zero.spell_fun=spell_null;

	// read in the skill text file making sorted linked lists.
//#define SKTYPE_UNDEFINED	0
//#define SKTYPE_SPELL		1
//#define SKTYPE_SKILL		2
//#define SKTYPE_OTHER		3

	logf("Doing prescan of " SKILLS_FILE "... for skills, spells etc");

	// open the file for input
	fclose( fpReserve );
    if ( ( fp = fopen( SKILLS_FILE, "r" ) ) == NULL )
    {
		bugf("do_loadskilltable(): fopen '%s' for read - error %d (%s)",
			SKILLS_FILE, errno, strerror( errno));
		ch->printf("An error occurred trying to open " SKILLS_FILE" for reading!\n");
		write_shutdown_file(NULL);
		do_abort();
    }
    
	{
		bool morefile=true;
		char *readword;
		char buf[MIL];

		while (morefile && !feof(fp)) {
			readword= fread_word(fp);

			if (!str_cmp(readword, "EOF") || !str_cmp(readword, "EOF~")){
				morefile=false;
			}else{
				if(!str_cmp(readword, "######NAME")){
					readword=fread_string(fp); // freed
					sprintf(buf,"%s", readword);
					// load the skill entry for the prescan
					GIO_LOAD_RECORD(skill_type, &skill_prescan, fp);

					if(str_cmp(readword, "reserved"))
					{
						count++;
						// add to list, tag of skill_prescan.type, allow duplicates = true
						addlist(&list[skill_prescan.sktype],readword, 0, true, false);
// different types
//	{"undefined",		SKTYPE_UNDEFINED,		true},
//	{"spell",			SKTYPE_SPELL,			false},
//	{"skill",			SKTYPE_SKILL,			true},
//	{"other",			SKTYPE_OTHER,			true},
//	{"realm",			SKTYPE_REALM,			true},
//	{"sphere",			SKTYPE_SPHERE,			true},
//	{"elementseason",	SKTYPE_ELEMENTSEASON,	true},
	
					}
					free_string(readword);
				}else{// unexpected file format
					bugf("Unexpected fileformat in '%s' - found '%s' "
						"expecting '######NAME'", SKILLS_FILE, readword);
					write_shutdown_file(NULL);
					do_abort();
					return;
				}
			}
		}
		
		logf("Prescan found %d skill%s.", count, count==1?"":"s");		
		if(count+MAX_RESERVED_SPELL_SPACE>=MAX_SKILL){
			logf("Prescan +%d found more skills than MAX_SKILL, increase MAX_SKILL above %d\n", 
				MAX_RESERVED_SPELL_SPACE, MAX_SKILL);
			logf("The +%d is due to space allocation to allow adding of spells while online.\n",
				MAX_RESERVED_SPELL_SPACE);
			write_shutdown_file(NULL);
			exit_error( 1 , "do_loadskilltable", "prescan found more skills than allocated storage space.");
		}
		
		fclose( fp );

		// manually add all the skills/spells not found in the prescan
		// that values exist in the gsn table
		// - this means that no manual editing is required to add the skill
		for ( sn = 0; !IS_NULLSTR(dynlookup_table[sn].name); sn++ )
		{
			if(dynlookup_table[sn].dyn_sktype==SKTYPE_SKILL){
				// add it to the skill list without duplicates
				addlist(&list[SKTYPE_SKILL], dynlookup_table[sn].name, SKTYPE_SKILL, false, false); 
			}
			if(dynlookup_table[sn].dyn_sktype==SKTYPE_SPELL){
				// add it to the spell list without duplicates
				addlist(&list[SKTYPE_SPELL], dynlookup_table[sn].name, SKTYPE_SPELL, false, false); 
			}
		}

		{
			static skill_type skill_zero;
			skill_zero.spell_fun=spell_null;
			index=0;

			for(count=0;count<10; count++){
				if(list[count]==NULL)
					continue;
//				logf("list #%d",count);	
				
				if(count==SKTYPE_SPELL)
				{
					FIRST_SPELL=index;
				}
				for(plist=list[count];plist; plist=plist->next){
//					logf("--->>%d <- %s", index, plist->name);
					// check for duplicate already.
					skill_table[index+1].name=NULL;
					if(skill_exact_lookup(plist->name)!=-1){
						bugf("DUPLICATE ENTRY '%s' - last entry used.", plist->name);
					}else{
						skill_table[index]=skill_zero; // clear entry fully
						skill_table[index].name=plist->name; // no need to str_dup it
															 // because addlist str_dup's

						// set the default type in order to work with loading the gsn
						// table entries, and getting the correct type when they 
						// haven't manually been added to the skills.txt file
						if(plist->tag){
							logf("Detected '%s' not in prescan, but in dyntable - added.",
								plist->name);
							skill_table[index].sktype=plist->tag; 							

							// bind the spell function if possible
							if(plist->tag==SKTYPE_SPELL){
								char possible_spellname[MIL];
								// get the lowercase with spaces converted to underscores version
								sprintf(possible_spellname,"spell_%s", underscore_word(plist->name));

								skill_table[index].spell_function_index=spellfunctionindex_fromchar(possible_spellname);
								if(skill_table[index].spell_function_index>-1){
									logf("SPELL '%s' - binding spell func %s",
										plist->name, possible_spellname);						

									skill_table[index].target=spellpairs_table[skill_table[index].spell_function_index].target;
									skill_table[index].spell_fun= spellpairs_table[skill_table[index].spell_function_index].psp;
								}else{
									logf("SPELL '%s' couldn't find %s, binding spell_null instead!",
										plist->name, possible_spellname);						
									skill_table[index].spell_fun=spell_null;
									skill_table[index].target=TAR_IGNORE;
								}
							}
						}
						
						index++;
					}
				}
				// add extra space for adding new spells
				if(count==SKTYPE_SPELL)
				{
					int space;
					LAST_SPELL=index;
					for(space=0;space<MAX_RESERVED_SPELL_SPACE; space++){
						skill_table[index].name=str_dup("reserved");
						index++;
					}
				}
			}
			logf("Marked end of skill table at index %d.", index);
			skill_table[index].name=NULL; // mark it end of the table
			TOP_SKILL=index; 
		}
    }
	fpReserve = fopen( NULL_FILE, "r" );

	logf("Finished prescan of skill file, assigning gsn's");

	// copy the gsn pointers and assign them 
	{ 
		sh_int index;
		count=0;

		for ( sn = 0; !IS_NULLSTR(dynlookup_table[sn].name); sn++ )
		{
			index=skill_lookup(dynlookup_table[sn].name);

			if(index>=0){
				skill_table[index].pgsn=dynlookup_table[sn].pgsn;
				if (skill_table[index].pgsn)
				{
					*skill_table[index].pgsn = index;
					count++;
				}
			}else{
				logf("Ignoring dyntable's '%s' gsn since wasn't found in prescan.",
					dynlookup_table[sn].name);
			}
		}
		logf("%d gsn%s assigned, loading in all data from skill file now.",
			count, count==1?"":"s");
	}

	// load in the whole skill table now

	// open the file for input
	fclose( fpReserve );
    if ( ( fp = fopen( SKILLS_FILE, "r" ) ) == NULL )
    {
		bugf("do_loadskilltable(): fopen '%s' for read - error %d (%s)",
			SKILLS_FILE, errno, strerror( errno));
		ch->printf("An error occurred trying to open " SKILLS_FILE" for reading!\n");
		write_shutdown_file(NULL);
		do_abort();
    }

    {
		bool morefile=true;
		char *readword;
		char buf[MIL];
		sh_int sn;

		count=0;
		
		while (morefile && !feof(fp)) {
			readword= fread_word(fp);
			
			if (!str_cmp(readword, "EOF") || !str_cmp(readword, "EOF~")){
				morefile=false;
			}else{
				if(!str_cmp(readword, "######NAME")){
					readword=fread_string(fp); // freed					
					if(str_cmp(readword, "reserved"))
					{
						sprintf(buf,"%s", readword);
						// lookup the skill and read it in.
						sn=skill_lookup(buf);
						if(sn>=0){
							GIO_LOAD_RECORD(skill_type, &skill_table[sn], fp);
							count++;
							// count the spell functions in use
							if(skill_table[sn].spell_function_index>=0){
								spellpairs_table[skill_table[sn].spell_function_index].count++;
							}

							// this isn't the most efficient way, but I can't be bothered 
							// writing something more efficient
							if(!IS_NULLSTR(race_get_races_set_for_n_array(skill_table[sn].race_restrict_n))){
								SET_BIT(skill_table[sn].flags, SKFLAGS_USE_RACE_RESTRICTIONS);
							}

						}else{
							// if we can't find the entry in the skill table then
							// the skill file has been modified in the 2 second 
							// period or something it corrupting info, therefore abort
							bugf("do_loadskilltable: Something very screwy is going on!\n"
								"a NEW entry ('%s') have appeared in the skill file '%s' since the prescan!n"
								"Aborting!", buf,SKILLS_FILE);
							write_shutdown_file(NULL);
							do_abort();
							return;
						}
					}else{
						// ignore the reserved record
						GIO_LOAD_RECORD(skill_type, &skill_prescan, fp);
					}
					free_string(readword);
				}else{// unexpected file format
					bugf("Unexpected fileformat in '%s' - found '%s' "
						"expecting '######NAME'", SKILLS_FILE, readword);
					write_shutdown_file(NULL);
					exit_error( 1 , "do_loadskilltable", "unexpected file format");
					return;
				}
			}
		}
		logf("Finished reading skill file. (read in %d total)", count);
	}
	fclose( fp );

	fpReserve = fopen( NULL_FILE, "r" );
	return;
}
/**************************************************************************/
// Output the skill_table to disk
void do_write_skills(char_data *ch, char *)
{
	FILE *fp;
	sh_int sn;

	logf("Writing skill table to " SKILLS_FILE ".write ...");
	fclose( fpReserve );

    if ( ( fp = fopen( SKILLS_FILE".write", "w" ) ) == NULL )
    {
		bugf("do_write_skills(): fopen '%s' for write - error %d (%s)",
			SKILLS_FILE".write", errno, strerror( errno));
		ch->printf("An error occurred opening " SKILLS_FILE".write for writing!\r\n");
		autonote(NOTE_SNOTE, "do_write_skills()", "Problems saving skill table", "code cc: imm", 
			"An error occurred opening " SKILLS_FILE".write for writing!\r\n", true);
    }else{
		// LOOP thru everything in the table, writing it
		for ( sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++ )
		{
			if(!str_cmp("reserved", skill_table[sn].name)){
				continue;
			}
			fprintf(fp,"######NAME       %s~\n", skill_table[sn].name);
			GIO_SAVE_RECORD(skill_type, &skill_table[sn], fp, NULL);
			fprintf(fp,"\n");
		}
		fprintf(fp, "EOF~\n");

		int bytes_written=fprintf(fp, "EOF~\n");
		fclose( fp );
		if(   bytes_written != str_len("EOF~\n") ){
			bugf("do_write_skills(): fprintf to '%s' incomplete - error %d (%s)",
				SKILLS_FILE".write", errno, strerror( errno));
			bugf("Incomplete write of " SKILLS_FILE".write, write aborted - check diskspace!");
			ch->printf("Incomplete write of " SKILLS_FILE".write, write aborted - check diskspace!\r\n");
			autonote(NOTE_SNOTE, "do_write_skills()", "Problems saving skill table", "code cc: imm", 
				"Incomplete write of " SKILLS_FILE ".write, write aborted - check diskspace!\r\n", true);
		}else{		
			ch->printf("Finished writing skill table to "SKILLS_FILE ".write\r\n");
			logf("Finished writing skill table to "SKILLS_FILE ".write");

			ch->printf("Renaming old " SKILLS_FILE " to " SKILLS_FILE ".bak\r\n");
			logf("Renaming old " SKILLS_FILE " to " SKILLS_FILE ".bak");
			unlink(SKILLS_FILE ".bak");
			rename(SKILLS_FILE , SKILLS_FILE ".bak");

			ch->printf("Renaming new " SKILLS_FILE ".write to " SKILLS_FILE "\r\n");
			logf("Renaming new " SKILLS_FILE ".write to " SKILLS_FILE);
			unlink(SKILLS_FILE );
			rename(SKILLS_FILE ".write", SKILLS_FILE );
			logf("Finished writing skill table.");
		}
    }
	fpReserve = fopen( NULL_FILE, "r" );

}
/**************************************************************************/

