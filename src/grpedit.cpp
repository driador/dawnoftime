/**************************************************************************/
// grpedit.cpp - olc based skillgroup editing
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
/**************************************************************************/
// prototypes
const char *pack_word(const char *word); // areas.cpp
/**************************************************************************/
const struct flag_type skillgroup_flags[] =
{
	{ "creation_selectable",		SKILLGROUP_CREATION_SELECTABLE,		true},
	{ "free_for_all",				SKILLGROUP_FREE_FOR_ALL,			true},
	{ NULL,0,0} // table terminator
};

/**************************************************************************/
/**************************************************************************/
// generic IO setup

// custom read/write functions
/**************************************************************************/
// write the class rating for each class listed in the skillgroup table
void skillgroup_WriteClassRatings(gio_type *gio_table, int tableIndex, void *data, FILE *fp)
{
	int classIndex, value;
	skillgroup_type * pG= (skillgroup_type *) data;

	fprintf(fp, "%s\n", gio_table[tableIndex].heading);
	for (classIndex=0; !IS_NULLSTR(class_table[classIndex].name); classIndex++){
		// write only valid values
		if (pG->rating[classIndex]<0 || pG->rating[classIndex]>50){
			value=-1;
		}else{
			value=pG->rating[classIndex];
		}
		if(value!=-1){
			fprintf(fp, "\t%s %d\n", class_table[classIndex].name, value);
		}
	};
	fprintf(fp, "~\n");
}

/**************************************************************************/
// read the class ratings for each class listed in the group table
void skillgroup_ReadClassRatings(gio_type *, int, void *data, FILE *fp)
{
	int classIndex;
	skillgroup_type * pG= (skillgroup_type *) data;

	char className[MIL];
	int rating;

	// initialise all values to -1 (no classes have access to the skill group)
	for (classIndex=0; !IS_NULLSTR(class_table[classIndex].name); classIndex++){
		pG->rating[classIndex]=-1;
	}

	while (true){
		int ret;
		char errbuf[MSL];

		// get the class name and rating
		ret=fscanf(fp, "%s ", className);	
		if(ret!=1){
			sprintf(errbuf, "skillgroup_ReadClassRatings(): Unexpected fscanf result reading className - ret=%d, errno=%d (%s)", ret, errno, strerror(errno));
			bug(errbuf);
			log_string(errbuf);			
		}
	    
		if (className[0]=='~' && className[1]=='\0'){
			break;
		}

		// read in the rating
		ret=fscanf(fp, "%d ", &rating);
		if(ret!=1){
			sprintf(errbuf, "skillgroup_ReadClassRatings(): Unexpected fscanf result reading rating - ret=%d, errno=%d (%s)", ret, errno, strerror(errno));
			bug(errbuf);
			log_string(errbuf);			
		}

		classIndex =class_lookup(className);

		if (classIndex<0){
			bugf( "group_ReadClassRatings: class '%s' not found... "
				"ignoring rating for that class.", className);
		}else{
			pG->rating[classIndex]=rating;
		}
	}
}

/**************************************************************************/
// write all the skills in the skillgroup to disk
void skillgroup_WriteSkills(gio_type *gio_table, int tableIndex, void *data, FILE *fp)
{
	int i;
	skillgroup_type * pG;
	
	pG= (skillgroup_type *) data;

	fprintf(fp, "%s\n", gio_table[tableIndex].heading);

	// write each skill out
	for (i = 0; pG->skills[i]>-1; i++){
		fprintf(fp, "\t%s\n", pack_word(skill_table[pG->skills[i]].name));
	}
	fprintf(fp, "~\n");
}
/**************************************************************************/
// write all the skills in the skillgroup to disk
void skillgroup_WriteSkillsWithValues(gio_type *gio_table, int tableIndex, void *data, FILE *fp)
{
	int i;
	skillgroup_type * pG;
	
	pG= (skillgroup_type *) data;

	fprintf(fp, "%s\n", gio_table[tableIndex].heading);

	// write each skill out
	for (i = 0; pG->skills[i]>-1; i++){
		fprintf(fp, "\t%s %d\n", 
			pack_word(skill_table[pG->skills[i]].name),
			pG->skillsvalue[i]);
	}
	fprintf(fp, "~\n");
}
/**************************************************************************/
// read all the skills from disk into a skillgroup
void skillgroup_ReadSkills(gio_type *, int, void *data, FILE *fp)
{
	skillgroup_type * pG= (skillgroup_type *) data;
	int sn;
	int sindex=0;
	char *readword;

	// initialise all skills to -1 (no skills in the skillgroup)
	memset(pG->skills, -1, sizeof(pG->skills));

	while (true){
		// get the skill name
		readword=fread_word(fp);
		if (readword[0]=='~' && readword[1]=='\0'){
			break;
		}

		sn=skill_exact_lookup(readword);

		if (sn<0){
			bugf( "group_ReadSkills: skill '%s' not found... ignoring skill for skillgroup '%s'.", 
				readword, pG->name);
		}else{
			if(sindex>=MAX_IN_GROUP){
				bugf("group_ReadSkills(): Too many skills (%d) to read into skillgroup '%s' "
					"- increase MAX_IN_GROUP.", sindex, pG->name);
				do_abort();
			}
			pG->skills[sindex]=sn;
			pG->skillsvalue[sindex]=1; // 1% is the default
			sindex++;
		}
	}
}

/**************************************************************************/
// read all the skills from disk into a skillgroup
void skillgroup_ReadSkillsWithValues(gio_type *, int, void *data, FILE *fp)
{
	skillgroup_type * pG= (skillgroup_type *) data;
	int sn;
	int sindex=0;
	char *readword;

	// initialise all skills to -1 (no skills in the skillgroup)
	memset(pG->skills, -1, sizeof(pG->skills));

	while (true){
		int value;
		// get the skill name
		readword=fread_word(fp);
		if (readword[0]=='~' && readword[1]=='\0'){
			break;
		}
		value=fread_number(fp);

		sn=skill_exact_lookup(readword);

		if (sn<0){
			bugf( "group_ReadSkills: skill '%s' not found... ignoring skill for skillgroup '%s'.", 
				readword, pG->name);
		}else{
			if(sindex>=MAX_IN_GROUP){
				bugf("group_ReadSkills(): Too many skills (%d) to read into skillgroup '%s' "
					"- increase MAX_IN_GROUP.", sindex, pG->name);
				do_abort();
			}
			pG->skills[sindex]=sn;
			pG->skillsvalue[sindex]=value;
			sindex++;
		}
	}
}

/**************************************************************************/
// GIO structure
GIO_START(skillgroup_type)
	GIO_INT(remort)
	GIO_WFLAGH(flags,		"flags",			skillgroup_flags)
	GIO_CUSTOM_WRITEH(name,	"skillswithvalues",	skillgroup_WriteSkillsWithValues)
	GIO_CUSTOM_READH(name,	"skillswithvalues",	skillgroup_ReadSkillsWithValues)
	GIO_CUSTOM_READH(name,	"skills",			skillgroup_ReadSkills)
	GIO_CUSTOM_WRITEH(name,	"class_ratings",	skillgroup_WriteClassRatings)
	GIO_CUSTOM_READH(name,	"class_ratings",	skillgroup_ReadClassRatings)	
GIO_FINISH_NOCLEAR	

/**************************************************************************/
// Output the skillgroup_table[] to disk 
void do_write_skillgroups(char_data *ch, char *)
{
	FILE *fp;
	int i;

	logf("Writing skillgroup_table[] to " SKILLGROUPS_FILE ".write ...");
	fclose( fpReserve );

    if ( ( fp = fopen( SKILLGROUPS_FILE ".write", "w" ) ) == NULL )
    {
		bugf("do_write_skillgroups(): fopen '%s' for write - error %d (%s)",
			SKILLGROUPS_FILE ".write", errno, strerror( errno));
		bug("An error occurred opening " SKILLGROUPS_FILE ".write for writing!");
		ch->println("An error occurred opening " SKILLGROUPS_FILE ".write for writing!");
		autonote(NOTE_SNOTE, "do_write_skillgroups()", "Problems saving skillgroups table", "code cc: imm", 
			"An error occurred opening " SKILLGROUPS_FILE ".write for writing!\r\n", true);
    }else{
		// LOOP thru everything in the table, writing it
		for ( i = 0; !IS_NULLSTR(skillgroup_table[i].name); i++ )
		{
			fprintf(fp,"######SKILLGROUP    %s~\n", skillgroup_table[i].name);
			GIO_SAVE_RECORD(skillgroup_type, &skillgroup_table[i], fp, NULL);
			fprintf(fp,"\n");
		}

		int bytes_written=fprintf(fp, "EOF~\n");
		fclose( fp );
		if(   bytes_written != str_len("EOF~\n") ){
			bugf("do_write_skillgroups(): fprint to '%s' incomplete - error %d (%s)",
				SKILLGROUPS_FILE ".write", errno, strerror( errno));
			bugf("Incomplete write of " SKILLGROUPS_FILE ".write, write aborted - check diskspace!");
			ch->printf("Incomplete write of " SKILLGROUPS_FILE ".write, write aborted - check diskspace!\r\n");
			autonote(NOTE_SNOTE, "do_write_skillgroups()", "Problems saving skillgroup_table[]", "code cc: imm", 
				"Incomplete write of " SKILLGROUPS_FILE ".write, write aborted - check diskspace!\r\n", true);
		}else{		
			ch->printf("Finished writing skillgroup_table[] to "SKILLGROUPS_FILE ".write\r\n");

			ch->printf("Renaming old " SKILLGROUPS_FILE " to " SKILLGROUPS_FILE ".bak\r\n");
			unlink(SKILLGROUPS_FILE".bak");
			rename(SKILLGROUPS_FILE, SKILLGROUPS_FILE".bak");

			ch->printf("Renaming new " SKILLGROUPS_FILE ".write to " SKILLGROUPS_FILE "\r\n");
			unlink(SKILLGROUPS_FILE);
			rename(SKILLGROUPS_FILE".write", SKILLGROUPS_FILE);
		}
    }
	fpReserve = fopen( NULL_FILE, "r" );
	logf("Finished writing skillgroup_table[].");
}

/**************************************************************************/
extern	struct	group_type	group_table	[MAX_GROUP_TABLE+1];
/**************************************************************************/
// convert group_table[] into skillgroup_table[]
void convert_group_table()
{
	int i, gn;
	int class_total_ratings[MAX_CLASS];

	memset(class_total_ratings, 0, sizeof(class_total_ratings));
	memset(skillgroup_table, 0, sizeof(skillgroup_table));

    for( gn = 0; !IS_NULLSTR(group_table[gn].oldname); gn++ )
    {
		memset(skillgroup_table[gn].skills, -1, sizeof(skillgroup_table[gn].skills));
		memset(skillgroup_table[gn].rating, -1, sizeof(skillgroup_table[gn].rating));
		// copy the name across
		skillgroup_table[gn].name=str_dup(group_table[gn].oldname);

		for ( i = 0; !IS_NULLSTR(group_table[gn].oldspells[i]); i++){
			skillgroup_table[gn].skills[i]=skill_lookup(group_table[gn].oldspells[i]);
		}

		// loop thru, transferring all class ratings, if all are 0, then set the free_for_all flag
		bool all_free=true;
		for(i=0; !IS_NULLSTR(class_table[i].name); i++){
			skillgroup_table[gn].rating[i]=group_table[gn].oldrating[i];
			if(skillgroup_table[gn].rating[i]!=0){
				all_free=false;
			}
			// record the combined total ratings for each class
			class_total_ratings[i]+=skillgroup_table[gn].rating[i]; 
		}
		if(all_free){
			SET_BIT(skillgroup_table[gn].flags, SKILLGROUP_FREE_FOR_ALL);
		}
		SET_BIT(skillgroup_table[gn].flags, SKILLGROUP_CREATION_SELECTABLE);
	}

	// for all classes with a combined total rating 0 (i.e. everything free)
	// set to -1 except on those which are free_for_all
	for(i=0; !IS_NULLSTR(class_table[i].name); i++){
		if(class_total_ratings[i]==0){
			for( gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++ )
			{
				if(!IS_SET(skillgroup_table[gn].flags, SKILLGROUP_FREE_FOR_ALL)){
					skillgroup_table[gn].rating[i]=-1;
				}
			}
		}

	}
	
}

/**************************************************************************/
// read in the skillgroup_table from disk
void do_read_skillgroups(char_data *ch, char *)
{
	FILE *fp;
	int count=0;

	// because system doesn't support reassigning group numbers, only allow 
	// this to run at bootup and when one imm is on
	if(runlevel!=RUNLEVEL_BOOTING){
		ch->println("do_read_skillgroups() currently doesn't support being called a game time... boot only!");
		ch->println("Doing so in its current state might stuff up everyone's skill ");
		ch->println("groups... BASICALLY DONT DO IT!");
		return;
	}

	logf("Reading in skillgroup_table[] from " SKILLGROUPS_FILE "...");
	fclose( fpReserve );

    if ( ( fp = fopen( SKILLGROUPS_FILE, "r" ) ) == NULL )
    {	// couldn't find the file to read in
		bugf("do_read_skillgroups(): fopen '%s' for read - error %d (%s)",
			SKILLGROUPS_FILE , errno, strerror( errno));
		bugf("An error occurred trying to open " SKILLGROUPS_FILE " for reading!");
		ch->printf("An error occurred trying to open " SKILLGROUPS_FILE " for reading!\r\n");
		if(file_exists(SKILLGROUPS_FILE )){
			bugf("File " SKILLGROUPS_FILE "found, but it wasn't opened for some reason?!?");
			write_shutdown_file(NULL);
			do_abort();
		}else{
			ch->println("Converting old skillgroup_table[] and creating a new " SKILLGROUPS_FILE " file.");
			logf("Converting old skillgroup_table[] and creating a new " SKILLGROUPS_FILE " file.");
			convert_group_table();
			fpReserve = fopen( NULL_FILE, "r" );
			do_write_skillgroups(NULL,"");
		}
		return;
    }
	
	// SKILLGROUPS_FILE was found, and successfully opened for input
	bool morefile=true;
	char *readword;
	
	if(runlevel==RUNLEVEL_BOOTING){
		// *** Mark the existing table as empty - since we are loading everything in
		// - This could cause skillgroup corruption if done when people are online...
		//   that is why it can be only done if one is online or during booting.
		memset(skillgroup_table, 0, sizeof(skillgroup_table));
	}

	// loop thru till we get the end of the table
	while (morefile && !feof(fp)) {
		int load_index;
		readword= fread_word(fp);

		if (!str_cmp(readword, "EOF") || !str_cmp(readword, "EOF~")){
			morefile=false;
		}else{
			if(str_cmp(readword, "######SKILLGROUP")){
				// unexpected file format
				bugf("Unexpected fileformat in '%s' - found '%s' "
					"expecting '######SKILLGROUP'", SKILLGROUPS_FILE , readword);
				ch->printf( "Unexpected fileformat in '%s' - found '%s' "
					"expecting '######SKILLGROUP'\r\n", SKILLGROUPS_FILE , readword);
				flush_char_outbuffer(ch);
				write_shutdown_file(NULL);
				do_abort();
				return;
			}

			// get the name of the skillgroup
			{
				readword=fread_string(fp); // freed

				// check if the skillgroup hasn't already been loaded - no duplicates
				load_index=skillgroup_exact_lookup(readword);

				if(load_index>-1){ // we have already loaded a skill group with that name!?!
					bugf("do_read_skillgroups(): skillgroup '%s' has already been loaded!", readword);
					do_abort();
				}

				// we have a new skill group to read in... allocate space for it

				for ( load_index=0; !IS_NULLSTR(skillgroup_table[load_index].name); load_index++){
					// do nothing till we find the end
				}
				//load_index now points to the end of the list
				if(load_index>=MAX_SKILLGROUP){
					bugf("Too many skillgroups (%d) trying to load skillgroup '%s' from %s, increase MAX_SKILLGROUP - aborting",
						load_index, readword, SKILLGROUPS_FILE);
					write_shutdown_file(NULL);
					exit_error( 1 , "do_read_skillgroups", "Too many skill groups");
				}

				// fill in the defaults for our entry
				memset(skillgroup_table[load_index].skills, -1, sizeof(skillgroup_table[0].skills));
				memset(skillgroup_table[load_index].rating, -1, sizeof(skillgroup_table[0].rating));
				skillgroup_table[load_index].name=str_dup(readword);

				// mark the new end of the table
				skillgroup_table[load_index+1].name=NULL; 

				// read in the data
				GIO_LOAD_RECORD(skillgroup_type, &skillgroup_table[load_index], fp);
				count++;
				free_string(readword);
			}
		}
	}
	fclose( fp );

	ch->printf("Finished reading skillgroup_table from " SKILLGROUPS_FILE  ". (read in %d)\r\n", count);
	logf("Finished reading skillgroup_table from " SKILLGROUPS_FILE ". (read in %d)", count);

	fpReserve = fopen( NULL_FILE, "r" );
	return;
}

/**************************************************************************/
//	Entry Point for editing the skill groups
void do_skillgroupedit( char_data *ch, char *argument )
{
	char arg[MIL];
	if(!str_infix("create", argument)){		
		argument=one_argument(argument, arg);
	}else{
		strcpy(arg, argument);
	}

	if ( IS_NPC( ch )){
		ch->println("Players only.");
		return;
	}

	// do security checks
	if (!HAS_SECURITY(ch, GROUPEDIT_MINSECURITY)){
    	ch->printlnf("You must have an olc security %d or higher to use this command.",
			GROUPEDIT_MINSECURITY);
		return;
	}

	if (!IS_TRUSTED(ch, GROUPEDIT_MINTRUST)){
		ch->printlnf("You must have a trust of %d or above to use this command.", 
			GROUPEDIT_MINTRUST);
		return;
	}

	if (IS_NULLSTR(arg)){
		ch->println("Syntax: `=Cskgrpedit create <newskillgroup>`x");
		ch->println("    or: `=Cskgrpedit <skillgroup>`x");
		ch->println("Where <skillgroup> is one of the following:");
		
		ch->println(" [remort number]<number of skills in group> group name");
		// groups up below
		int col=0;
		for (int gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++)
		{
			// count the total skills in a group
			int skcnt =0;			
			for ( int skcnti= 0; skillgroup_table[gn].skills[skcnti]>-1; skcnti++) {
				skcnt++;
			}

			// display the group
			ch->printf("`%c  [%d]%2d %-33s `x", 
				IS_SET(skillgroup_table[gn].flags, SKILLGROUP_CREATION_SELECTABLE)?
					(IS_SET(skillgroup_table[gn].flags, SKILLGROUP_FREE_FOR_ALL)?'G':'x')
					:IS_SET(skillgroup_table[gn].flags, SKILLGROUP_FREE_FOR_ALL)?'g':'S',
				skillgroup_table[gn].remort,skcnt,
				skillgroup_table[gn].name);
			if (++col % 2 == 0){
				ch->println("");
			}
		}
		if(col % 2 != 0){
			ch->println("");
		}
		ch->println("NumberPrefix = remort, Colour coding flag guide: CS=creation selectable, FFA = free for all");
		ch->println("`Gbright green= CS,FFA  `gdark green = FFA   `xwhite=CS  `Ssilver=both flags unset.`x");
		ch->println("CS=creation selectable, FFA = free for all (everyone can get it for free).");
		return;
	}

	if ( !str_cmp( arg, "create" ))
	{
		// allocate space for a new skillgroup
		int gn;

		for ( gn=0; !IS_NULLSTR(skillgroup_table[gn].name); gn++){
			// do nothing till we find the end
		}
		//load_index now points to the end of the list
		if(gn>=MAX_SKILLGROUP){
			ch->printlnf("There are too many skillgroups (%d), increase MAX_SKILLGROUP if you want to add more",
				gn);
			return;
		}

		// fill in the defaults for our entry
		memset(skillgroup_table[gn].skills, -1, sizeof(skillgroup_table[0].skills));
		memset(skillgroup_table[gn].rating, -1, sizeof(skillgroup_table[0].rating));
		skillgroup_table[gn].name=str_dup(argument);

		// mark the new end of the table
		skillgroup_table[gn+1].name=NULL; 

		// update them on what is happening
		ch->wraplnf( "`xCreated skillgroup '%s'",argument);
		ch->println( "Type `=Cdone`x to finish editing." );
		ch->desc->pEdit	= (void*)&skillgroup_table[gn];
		ch->desc->editor = ED_SKILLGROUP;
		return;
	}

	int skillgroupindex=skillgroup_lookup(arg);
	if(skillgroupindex<0){
		ch->printlnf("Could not find a skillgroup '%s' to edit.", arg);
		return;
	}
    ch->desc->pEdit	= (void*)&skillgroup_table[skillgroupindex];
	ch->desc->editor = ED_SKILLGROUP;
	ch->printlnf("Editing skillgroup '%s' with skillgroup editor.", 
		skillgroup_table[skillgroupindex].name);
	return;
}
/**************************************************************************/
// returns a negative amount if 
int skillgroupedit_calc_individual_cost(skillgroup_type * pG, int classindex)
{
	int total=0;	
	bool unattainable_skill=false;
    for ( int i = 0; pG->skills[i]>-1; i++)
    {
		if(IS_SKILL_VALID_FOR_CLASS(pG->skills[i], classindex)){
			total+=skill_table[pG->skills[i]].rating[classindex];
		}else{
			unattainable_skill=true;
		}
    }
	if(unattainable_skill){
		return (total+1)*-1;
	}else{
		return total;
	}
}
/**************************************************************************/
bool skillgroupedit_show(char_data *ch, char *argument)
{
	int i, count, showclass, classcost;
	char skillcolour;
	bool unused_skills=false;

	skillgroup_type * pG;
	EDIT_SKILLGROUP(ch, pG);
	SET_BIT(ch->dyn,DYN_SHOWFLAGS);

	ch->titlebarf("SKILLGROUPEDIT: %s", uppercase(pG->name));
  	ch->printlnf( "`=rSkillgroup Name: `x%-10s       `=rRemort: `x%d ", 
		capitalize(pG->name), pG->remort);
	mxp_display_olc_flags(ch, skillgroup_flags, pG->flags,	"flags", "Flags:");

	// display the skills in the group, first count them then display
	count =0;
    for ( i = 0; pG->skills[i]>-1; i++) {
		count++;
	}

	showclass=class_lookup(argument);
	if(showclass<0){
		skillcolour='x';
		ch->titlebarf("SKILLS IN GROUP - %d TOTAL", count);
	}else{
		ch->titlebarf("SKILLS IN GROUP - %d TOTAL (CLASS FILTER=%s)", 
			count, uppercase(class_table[showclass].name));
	}
    for ( i = 0; pG->skills[i]>-1; i++)
    {
		if(showclass>-1 && !IS_SKILL_VALID_FOR_CLASS(pG->skills[i], showclass)){
				skillcolour='R';
		}else{
				skillcolour='x';
		}

		if(i%3==2){
			ch->printlnf("   `%c%s`x", skillcolour, 
				FORMATF("%s [%d%%]", skill_table[pG->skills[i]].name, pG->skillsvalue[i]));
		}else{
			ch->printf("   `%c%-27s`x", skillcolour, 
				FORMATF("%s [%d%%]", skill_table[pG->skills[i]].name, pG->skillsvalue[i]));
		}
    }
	if(i%3!=0){
		ch->println("");
	}

	// display the cost in trains for each class
	ch->titlebar("COST (TRAINS)");
	for(i=0; !IS_NULLSTR(class_table[i].name); i++){
		// the free for all flag overwrites the cost
		if(IS_SET(pG->flags, SKILLGROUP_FREE_FOR_ALL)){
			classcost=0;
		}else{
			classcost=pG->rating[i];
		}

		if(classcost<0){
			ch->printf(" %-23s `S------`x", class_table[i].name);
		}else{
			count=skillgroupedit_calc_individual_cost(pG, i);

			if(count<0){
				ch->printf(" %-23s `%c%2d`x(`R%2d`x)", 
					class_table[i].name, 
					(classcost<1?'G':'x'),
					pG->rating[i], 
					(count*-1)-1);
				unused_skills=true;
			}else{
				ch->printf(" %-23s `%c%2d`x(%2d)", 
					class_table[i].name, 
					(classcost<1?'G':'x'),
					pG->rating[i], 
					skillgroupedit_calc_individual_cost(pG, i));
			}
		}
		if(i%2==0){
			ch->print("              ");
		}else{
			ch->println("");
		}
	}
	ch->println("");
	if(i%2!=0){
		ch->println("");
	}
	ch->wrapln("The number before the brackets after a class name, is how much the group "
		"costs for that class to gain/add.  The number in brackets is the total of how "
		"much each individual skill costs for that class.");
	if(IS_SET(pG->flags, SKILLGROUP_FREE_FOR_ALL)){
		ch->println("`S   note: because 'free_for_all' is set, all classes can get this for 0 trains.");
		ch->println("         and will automatically get it during creation (hence green on all)`x.");
	}else{
		ch->print_blank_lines(2);
	}
	if(unused_skills && showclass==-1){
		  //        "-===========================================================================-"
		ch->println("`S   Classes with a red total (`R##`S) don't get access to all the skills listed");
		ch->println("   above... use 'show <classname>' to see what skills they can/can't get.");
	}else{
		ch->print_blank_lines(2);
	}
	ch->titlebar("");

	REMOVE_BIT(ch->dyn,DYN_SHOWFLAGS);
    return false;
}
/**************************************************************************/
// add a skill to the currently edited group
bool skillgroupedit_addskill(char_data *ch, char *argument)
{
	char arg[MIL];
	int sn, i;

	skillgroup_type * pG;
	EDIT_SKILLGROUP(ch, pG);

	// check for arguments -> show syntax if none
	if (IS_NULLSTR(argument)){
		ch->println("groupedit_addskill(): syntax: `=Caddskill <skillname>`x");
		ch->println("note: use `=Cremoveskill`x to remove skills from a skillgroup.");
		return false;
	}

	// find the skill we are talking about
	if (!str_prefix("'",argument)){  // ' symbol optional
		argument = one_argument( argument, arg );
	}else{
		strcpy(arg, argument);
	}

	sn=skill_lookup(arg);

	if(sn<0){
		ch->printlnf("groupedit_addskill(): Couldn't find the skill '%s' to add.", arg);
		return false;
	}

	// we have the gsn of a skill in sn... check it is not a spell
	if(IS_SPELL(sn)){
		ch->printlnf("groupedit_addskill(): '%s' is a spell, you can't add spells to skillgroups.", 
			skill_table[sn].name);
		return false;
	}

	// check if the skill is already allocated
	for (i = 0; pG->skills[i]>-1; i++){
		if(pG->skills[i]==sn){
			ch->printlnf("groupedit_addskill(): '%s' is already a skill in '%s'... use removeskill to remove it.", 
				skill_table[sn].name, pG->name);
			return false;
		}
	}

	// find the next free slot to store it
	for (i = 0; pG->skills[i]>-1; i++){
	}
	if(i>=MAX_IN_GROUP){
		ch->wraplnf("groupedit_addskill(): There is a maximum of %d skills in a skillgroup... "
			"no room for any more skills... increase MAX_IN_GROUP in the code if more per "
			"group is required.",
			MAX_IN_GROUP);
		return false;
	}

	// add the skill to the group
	pG->skills[i+1]=-1; // safety code
	pG->skills[i]=sn;
	pG->skillsvalue[i]=1; // default to 1%

	ch->printlnf("groupedit_addskill(): '%s' added to skillgroup '%s'.", 
			skill_table[sn].name, pG->name);
	return true;
}
/**************************************************************************/
// change the skillvalue on a skill
bool skillgroupedit_skillvalue(char_data *ch, char *argument)
{
	char arg[MIL];
	char valarg[MIL];
	int sn, i, value;

	skillgroup_type * pG;
	EDIT_SKILLGROUP(ch, pG);

	// check for arguments -> show syntax if none
	if (IS_NULLSTR(argument)){
		ch->println("syntax: `=Cskillvalue <skillname> <value>`x");
		ch->println("note: value is the percentage they will get the skill at.");
		ch->println("note: use `=Cremoveskill`x to remove skills from a skillgroup.");
		return false;
	}

	// find the skill we are talking about
	argument = one_argument( argument, arg );
	argument = one_argument( argument, valarg);
	
	sn=skill_lookup(arg);

	if(sn<0){
		ch->printlnf("No such skill '%s' exists.", arg);
		return false;
	}

	// we have the gsn of a skill in sn... check it is not a spell
	if(IS_SPELL(sn)){
		ch->printlnf("'%s' is a spell, you can't add spells to skillgroups.", 
			skill_table[sn].name);
		return false;
	}

	value=atoi(valarg);
	if(value<0 || value>101 || !is_number(valarg)){
		ch->printlnf("A sensible value is required for the percentage"
			" - '%s' isn't valid here.", valarg);
		return false;
	}

	// check if the skill is already allocated
	for (i = 0; pG->skills[i]>-1; i++){
		if(pG->skills[i]==sn){	
			ch->printlnf("Skill value of '%s' changed from %d to %d.", 
				skill_table[sn].name, pG->skillsvalue[i], value);
			pG->skillsvalue[i]=value;
			return true;
		}
	}

	ch->printlnf("Couldn't find any '%s' to set a value on.", skill_table[sn].name);
	return false;
}
/**************************************************************************/
// remove a skill from the currently edited group
bool skillgroupedit_removeskill(char_data *ch, char *argument)
{
	char arg[MIL];
	int sn, i, last;

	skillgroup_type * pG;
	EDIT_SKILLGROUP(ch, pG);

	// check for arguments -> show syntax if none
	if (IS_NULLSTR(argument)){
		ch->println("groupedit_removeskill(): syntax: `=Cremoveskill <skillname>`x");
		ch->println("note: use `=Caddskill`x to add skills to a skillgroup.");
		return false;
	}

	// find the skill we are talking about
	if (!str_prefix("'",argument)){  // ' symbol optional
		argument = one_argument( argument, arg );
	}else{
		strcpy(arg, argument);
	}

	sn=skill_lookup(arg);

	if(sn<0){
		ch->printlnf("groupedit_removeskill(): Couldn't find the skill '%s' to remove.", arg);
		return false;
	}

	// find the skill 
	for (i = 0; pG->skills[i]>-1; i++){
		if(pG->skills[i]==sn){
			break;
		}
	}
	if(i>=MAX_IN_GROUP){
		ch->wraplnf("groupedit_removeskill(): the skill '%s' was not found in the skillgroup '%s' to remove.", 
			skill_table[sn].name, pG->name);
		return false;
	}

	// find the last skill
	for (last = 0; pG->skills[last]>-1; last++){
	}
	last--;

	// remove the skill
	if(last==i){
		pG->skills[i]=-1;
	}else{
		pG->skills[i]=pG->skills[last];
		pG->skills[last]=-1;
	}

	ch->wraplnf("groupedit_removeskill(): '%s' removed from skillgroup '%s'.", 
			skill_table[sn].name, pG->name);
	return true;
}


/**************************************************************************/
bool skillgroupedit_flags(char_data *ch, char *argument)
{
    int value;
	skillgroup_type * pG;
	EDIT_SKILLGROUP(ch, pG);
	
    if(!IS_NULLSTR(argument)){
		if ( ( value = flag_value( skillgroup_flags, argument ) ) != NO_FLAG )
		{
			pG->flags ^= value;
			
			ch->println("SkillGroup flag toggled.");
			return true;
		}
    }

	show_olc_options(ch, skillgroup_flags, "flags", "skillgroup", pG->flags );
    return false;	
}

/**************************************************************************/
// adjust the remort level a skillgroup requires
bool skillgroupedit_remort(char_data *ch, char *argument)
{
	skillgroup_type * pG;
	EDIT_SKILLGROUP(ch, pG);
	int			value;

    if( IS_NULLSTR(argument) || !is_number(argument)) {
		ch->println("Syntax: remort <number>");
		ch->println("Remort 0 = skillgroup available at first creation.");
		ch->println("Remort 1 = skillgroup available at after first remort creation...");
		return false;
	}

	value = atoi( argument );
	if(value<0 || value>5){
		ch->println("The remort value must be between 0 and 5.");
		return false;
	}
	ch->printlnf("Remort value changed from %d to %d.",
		pG->remort, value);
	pG->remort= value;
	return true;
}

/**************************************************************************/
bool skillgroupedit_rename(char_data *ch, char *argument)
{
	skillgroup_type * pG;
	EDIT_SKILLGROUP(ch, pG);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  rename <string>" );
		return false;
    }

	if(IS_SET(pG->flags, SKILLGROUP_CREATION_SELECTABLE)){
		ch->println("You cant rename a skillgroup that is creation selectable.");
		return false;
	}

    ch->wraplnf("Skillgroup name changed from '%s' to '%s'.", pG->name, argument);
	replace_string(pG->name, argument);
	return true;
}
/**************************************************************************/
// adjust the cost a particular class pays for the skillgroup
bool skillgroupedit_cost(char_data *ch, char *argument)
{
	if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  cost <class> <amount>" );
		ch->println( "Syntax:  cost <class> free" );
		ch->println( "Syntax:  cost <class> na   (not available)" );
		return false;
    }

	// check we have the class
	char classname[MIL];
	
	argument=one_argument(argument, classname);

	int class_index=class_lookup(classname);

	if(class_index<0){
		ch->printlnf("There is no class '%s'", classname);
		return false;
	}

	skillgroup_type * pG;
	EDIT_SKILLGROUP(ch, pG);

	if(!str_cmp(argument,"free")){
		pG->rating[class_index]=0;
		ch->printlnf("Skillgroup '%s' is now free for the %s class.",
			pG->name, class_table[class_index].name);
		return false;
	}
	if(!str_cmp(argument,"na")){
		pG->rating[class_index]=-1;
		ch->printlnf("Skillgroup '%s' is now unavailable for the %s class (assuming the free_for_all flag is unset).",
			pG->name, class_table[class_index].name);
		return false;
	}

	if(!is_number(argument)){
		ch->printlnf("Second argument must be 'free', 'na' or a numeric value, '%s' is none of these.",
			argument);
		skillgroupedit_cost(ch,"");
		return false;
	}

	int value=atoi(argument);
	if(value<1 || value >50){
		ch->println("The cost value range is 1 to 50.");
		return false;
	}

	if(pG->rating[class_index]>=0){
		ch->printlnf("Skillgroup '%s' cost changed from %d to %d for the %s class.",
			pG->name, pG->rating[class_index], value, class_table[class_index].name);
	}else{
		ch->printlnf("Skillgroup '%s' cost changed from being unavailable to %d for the %s class.",
			pG->name, value, class_table[class_index].name);
	}
	pG->rating[class_index]=value;

	value=skillgroupedit_calc_individual_cost(pG, class_index);
	if(value<0){
		value=(value+1)*-1;
		ch->printlnf("The individual cost of all the skills in the skillgroup for '%s' is %d.",
			class_table[class_index].name, value);
		ch->println("Note: this total only applies to the skills which the class can get");		
	}else{
		ch->printlnf("The individual cost of all the skills in the skillgroup for '%s' is %d.",
			class_table[class_index].name, value);
	}

	if(value<pG->rating[class_index]){
		ch->println("`Rnote: it costs more to purchase the skillgroup than the skills individually!`x");
	}
	return true;
}
/**************************************************************************/
