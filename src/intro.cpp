/**************************************************************************/
// intro.cpp - Introduction system - Kal May 2000
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/**************************************************************************/
// Unfinished like most of dawn *grin*, currently uses 2 bit flags to cache
// who knows who... if you have more than 512 people logged in at once make
// sure to increase MAX_CACHED_LOOKUPS ;)
// A future project could be to allow custom names... a bit cache value of 
// 01 could be used to mark these... currently:
// 00 = uncached, 01 = unused, 10 = unknown name, 11 = known name.
/**************************************************************************/
#include "include.h"
#include "intro.h"

#define MAX_KNOW_SET 10000
intro_data *intro_data_database[MAX_KNOW_SET];


#define MAX_CACHED_LOOKUPS (512)// after 512 logins, we reinitialise 
								// the complete intro_cache
#define BYTES_TO_STORE_CACHE (MAX_CACHED_LOOKUPS%4==0? MAX_CACHED_LOOKUPS/4:(MAX_CACHED_LOOKUPS/4)+1)
/**************************************************************************/
// intro debugging log
void ilogf(char *fmt, ...)
{
	if(!GAMESETTING5(GAMESET5_VERBOSE_INTRODUCTION_LOGGING)){
		return; // introduction logging disabled by default
	}
    char buf[MSL*2];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL*2, fmt, args);
	va_end(args);
	append_datetimestring_to_file( INTRO_DEBUG_FILE, buf);
	log_string(buf);
}
/**************************************************************************/
unsigned short find_empty_know_node()
{
	for(unsigned short i=10; i<MAX_KNOW_SET; i++){
		if(!intro_data_database[i]){
			return i;
		}
	}
	bugf("unsigned short find_empty_know_node(), couldn't find an empty node!\n"
		"System needs improvement for dynamic increasement here!");
	do_abort();

	return 0;
}
/**************************************************************************/
// returns the know index of a player, based on its player id
unsigned short find_know_index_from_char_id(time_t id)
{
	for(unsigned short i=10; i<MAX_KNOW_SET; i++){
		if(intro_data_database[i]){
			if(intro_data_database[i]->owner_id==id){
				return i;
			}	
		}
	}
	return 0;
}
/**************************************************************************/
void intro_data::allocate_intro_cache()
{
	static int last_allocated_cache_index=0;
	
	last_allocated_cache_index++;

	if(last_allocated_cache_index>MAX_CACHED_LOOKUPS-1){
		// we need to reinitialise the complete cache
		ilogf("intro_data::allocate_cache_details() - No space, reinitialising the complete cache");
		logf("intro_data::allocate_cache_details() - No space, reinitialising the complete cache");

		// restart the index
		last_allocated_cache_index=0;

		// loop thru deallocating the complete cache
		for(unsigned short i=10; i<MAX_KNOW_SET; i++){
			if(intro_data_database[i]){
				if(intro_data_database[i]->intro_cache){
					free(intro_data_database[i]->intro_cache);
					intro_data_database[i]->intro_cache=NULL;
				}
				intro_cache_index=0;
			}
		}

		// now loop thru all the players, reallocating them new cache
	    for( char_data *ch = player_list; ch; ch = ch->next_player ) {
			ch->know->allocate_intro_cache();
		}
	}else{
		intro_cache=(unsigned char *)malloc(BYTES_TO_STORE_CACHE);
		intro_cache_index=last_allocated_cache_index;
		ilogf("Allocated intro_cache_index %d to know_index %d", 
			intro_cache_index, my_know_index);
	}
}
/**************************************************************************/
void intro_data::assign_owner(char_data *ch)
{
	ilogf("intro_data::assign_owner() assigning know index %d to %s(%d)",
		my_know_index, ch->name, ch->know_index);
	assert(ch->player_id==owner_id);
	assert(ch->know_index==my_know_index);
	owner=ch; 
	last_logged_in=current_time; 	
	replace_string(owner_name,owner->name);
	ch->know=this; 
	if(!intro_cache){ // allocate memory for the cached lookups
		allocate_intro_cache();
	}
	logf("intro_data::assign_owner() assigned know index %d to %s(%d)[%d]",
		my_know_index, ch->name, ch->know_index, intro_cache_index);
	ilogf("intro_data::assign_owner() assigned know index %d to %s(%d)[%d]",
		my_know_index, ch->name, ch->know_index, intro_cache_index);
}
/**************************************************************************/
// link who a player knows to their pointer
void attach_know(char_data* player)
{
	if(IS_NPC(player)){
		player->know=NULL;
		return;
	}

	if(player->know_index!=0 && intro_data_database[player->know_index]==NULL){
		bugf("attach_know(): intro_data_database[player->know_index]==NULL, for player=%s, knowindex=%d, playerid=%d, inplace id=none, allocating a new know id.",
			player->name, player->know_index, (int)player->player_id);
		player->know_index=0;
	}

	// 'player->know_id' is treated as what their id might possibly 
	// this way if it doesn't match due to database corruption, we can reassign
	if(player->know_index==0){ // new char
		player->know_index=find_know_index_from_char_id(player->player_id);
		if(player->know_index==0){
			player->know_index=find_empty_know_node();		
			assert(player->know_index<MAX_KNOW_SET);
			assert(intro_data_database[player->know_index]==NULL);
			intro_data_database[player->know_index]=new intro_data;
			intro_data_database[player->know_index]->owner_id=player->player_id;
			intro_data_database[player->know_index]->my_know_index=player->know_index;
		}
	}

	assert(player->know_index<MAX_KNOW_SET);
	
	if(intro_data_database[player->know_index]->owner_id!=player->player_id){
		ilogf("intro_data_database[player->know_index]->owner_id!=player->player_id, player=%s, knowindex=%d, inplace id=%d,",
			player->name, player->know_index, intro_data_database[player->know_index]->owner_id);
		bugf("intro_data_database[player->know_index]->owner_id!=player->player_id, player=%s, knowindex=%d, inplace id=%d",
			player->name, player->know_index, (int)intro_data_database[player->know_index]->owner_id);
		// time to get allocated a new index - rerun the function
		player->know_index=0;
		attach_know(player);
		return;
	}

	assert(player->player_id==intro_data_database[player->know_index]->owner_id);
	assert(player->know_index==intro_data_database[player->know_index]->my_know_index);

	// set player->know to point the correct location etc
	intro_data_database[player->know_index]->assign_owner(player);
};
/**************************************************************************/
intro_node * intro_node::find_know_node_by_index(unsigned short lookup_know_index)
{
	if(!this){
		return NULL;
	}
	if(know_index==lookup_know_index){
		return this;
	}
	intro_node *node=next;
	for(; node; node=node->next){
		if(node->know_index==lookup_know_index){
			return node;
		}
	};
	return NULL;
};
/**************************************************************************/
// return false if they arent new
bool intro_node::add_person_by_index(unsigned short new_person_know_index, intro_data *know)
{
	if(!know){
		ilogf("intro_node::add_person_by_index(unsigned short new_person_know_index, intro_data *know), know==NULL!");
		make_corefile();
		return false;
	}

	if(know->people && know->people->find_know_node_by_index(new_person_know_index)){
		return false; // already known
	}
	// add a new person
	know->load_person(new_person_know_index, current_time);

	return true;
}
/**************************************************************************/
// load/add a relationship about new_person_id knowing 'this'
bool intro_data::load_person(unsigned short new_person_id, time_t last_seen)
{
	// add a new person
	intro_node *node= new intro_node;
	node->know_index=new_person_id;
	node->last_seen=last_seen;
	node->next=people;
	people=node;
	return true;
}

/**************************************************************************/
// Return true if ch knows the target
bool intro_data::knows(char_data *person)
{
	if(IS_NPC(person) || !person->know){
		return false;
	}
	if(!people){
		return false;
	}

	int cache_index=intro_data_database[person->know->my_know_index]->intro_cache_index;
	assert(cache_index); // have to have a cache index.. if they dont, how did they get here?

	// check if we have it cached
	if(IS_SET(intro_cache[cache_index/4], 1<<((cache_index%4)*2))){
		// bit caching is fast :)
		if(IS_SET(intro_cache[cache_index/4], 1<<(((cache_index%4)*2)+1) ) ){
			ilogf("Intro cache hit true for %d looking at %d (%d)", my_know_index, person->know->my_know_index, cache_index);
			return true;
		}

		ilogf("Intro cache hit false for %d looking at %d (%d)", my_know_index, person->know->my_know_index, cache_index);
		return false;
	}
	
	// support cached lookups later?
	intro_node *node= people->find_know_node_by_index(person->know->my_know_index);

	// cache the result
	SET_BIT(intro_cache[cache_index/4], 1<<((cache_index%4)*2)); // we are caching

	ilogf("Recording cache update for %d looking at %d (%d)", my_know_index, person->know->my_know_index, cache_index);

	if(node){
		node->last_seen=current_time; // record so we dont forget ever seeing them
		ilogf("Intro cache miss for %d looking at %d (%d)", my_know_index, person->know->my_know_index, cache_index);

		SET_BIT(intro_cache[cache_index/4], 1<<(((cache_index%4)*2)+1) ); // they do know them (cache it)
		return true;
	}
	REMOVE_BIT(intro_cache[cache_index/4], 1<<(((cache_index%4)*2)+1) ); // they do NOT know them (cache it)
	return false;
};
/**************************************************************************/
// when you are introduced to someone
bool intro_data::introduced_to(char_data *person)
{
	if(!this){
		ilogf("intro_data::introduced_to('%s'), this==NULL!", person?person->name:"null person also");
		return false;
	}
	if(owner->know!=this){
		ilogf("intro_data::introduced_to('%s'), this==NULL!", person?person->name:"null person also");
		return false;
	}
	assert(owner->know==this);

	// update the caching system
	int cache_index=intro_data_database[person->know->my_know_index]->intro_cache_index;
	SET_BIT(intro_cache[cache_index/4], 1<<(((cache_index%4)*2)) ); // they now know them
	SET_BIT(intro_cache[cache_index/4], 1<<(((cache_index%4)*2)+1) ); // they now know them
	ilogf("intro_data::introduced_to(): (introduction) recording cache update for %d knowing %d (%d)", 
		my_know_index, person->know->my_know_index, cache_index);

	return(people->add_person_by_index(person->know->my_know_index, this));
}; 
/**************************************************************************/
// when you choose to forget someone
void intro_data::forgetting(char_data *person)
{
	if(!this){
		ilogf("intro_data::forgetting('%s'), this==NULL!", person?person->name:"null person also");
		return;
	}
	if(owner->know!=this){
		ilogf("intro_data::forgetting('%s'), this==NULL!", person?person->name:"null person also");
		return;
	}
	assert(owner->know==this);

	if(!people){
		return; // if we dont know anyone
	}

	// update the caching system
	int cache_index=intro_data_database[person->know->my_know_index]->intro_cache_index;
	REMOVE_BIT(intro_cache[cache_index/4], 1<<(((cache_index%4)*2)) ); // they now know them
	REMOVE_BIT(intro_cache[cache_index/4], 1<<(((cache_index%4)*2)+1) ); // they now know them
	ilogf("intro_data::forgetting(): (forget) cache cleared for %d not knowing %d (%d)", 
		my_know_index, person->know->my_know_index, cache_index);

	{ // remove them
		ilogf("Removing");
		int mki=person->know->my_know_index;
		intro_node *prev=people;
		if(people->know_index==mki){
			people=people->next;
			delete prev;
			ilogf("delete prev; - removed");
			return;
		}else{
			for(intro_node *delnode=people->next; delnode; delnode=delnode->next){
				if(delnode->know_index==mki){
					prev->next=delnode->next;
					delete delnode;
					ilogf("delete delnode; - removed");
					return;
				}
				prev=delnode;
			}
		}
		ilogf("not removed?!?");
	}
}; 
/**************************************************************************/
// Save the database of 'who knows who' to disk.
// saving is done in 2 stages:
// * the first stage saves the data linking a player id to their 'know index'
// * stage two saves which know indexs know others.
// This means we can discard old know data for deleted players
// when reading in the database.
void save_intro_database()
{
	unsigned short i;
	intro_node *node;
	char filename[MIL];
	sprintf(filename, "%s.write", INTRODB_FILE);

	logf("Saving introduction database to %s", filename);
	FILE *fp=fopen( filename,"w");
	if(!fp){
		bugf("save_intro_database(): Couldn't open file '%s' to save introduction database to!",
			filename);
		return;
	}

	// stage 1
	for(i=10; i<MAX_KNOW_SET; i++){
		if(intro_data_database[i] && intro_data_database[i]->save_in_db){
			// owner player ID is compulsory in the database, since this is 
			// what everything is linked back against in attach_know
			// if an index to player id mapping not saved, the
			// data relating to that node will be dropped reboot

			if(intro_data_database[i]->last_logged_in< current_time- (60*60*24*500)){
				// if you haven't logged on for 500 days you are dropped
				// from the database
				if(!intro_data_database[i]->suppress_old_unsaved_log){
					logf("save_intro_database(): skipping saving old record - id=%d, name=%s, last seen=%-24.24s  (%s)",
						(int)intro_data_database[i]->owner_id,
						intro_data_database[i]->owner_name,
						ctime((time_t *)&intro_data_database[i]->last_logged_in),
						short_timediff(intro_data_database[i]->last_logged_in, current_time));					
					intro_data_database[i]->suppress_old_unsaved_log=true;
				}
				continue;
			}

			fprintf(fp,"-5 %d %d %d '%s' '%-24.24s'\n", 
				i, 
				(int)intro_data_database[i]->owner_id,
				(int)intro_data_database[i]->last_logged_in,
				intro_data_database[i]->owner_name,
				ctime((time_t *)&intro_data_database[i]->last_logged_in)
				); 
		}
	}

	// stage 2 - save which indexes know which indexes, and last seen
	for(i=10; i<MAX_KNOW_SET; i++){
		if(intro_data_database[i] && intro_data_database[i]->save_in_db){
			for(node=intro_data_database[i]->people; node; node=node->next){
				if(intro_data_database[node->know_index]){
					fprintf(fp,"%d %d %d\n", i, node->know_index, (int)node->last_seen); 
				}
			}
		}
	}
	int bytes_written=fprintf(fp,"-1\n"); 
	fclose( fp );

	if(   bytes_written != str_len("-1\n") ){
        bugf("save_intro_database(): fprintf to '%s' incomplete - error %d (%s)",
			filename, errno, strerror( errno));
		bugf("Incomplete write of %s, write aborted - check diskspace!", filename);
		autonote(NOTE_SNOTE, "save_intro_database()", 
			"Problems saving class table", "code cc: imm", 
			"Incomplete write of "  INTRODB_FILE ".write, write aborted - check diskspace!\r\n", true);
	}else{		
		logf("Renaming old " INTRODB_FILE " to " INTRODB_FILE ".bak");
		unlink(INTRODB_FILE".bak");
		rename(INTRODB_FILE, INTRODB_FILE".bak");

		logf("Renaming new %s to " INTRODB_FILE,  filename);
		unlink(INTRODB_FILE);
		rename(filename, INTRODB_FILE);
	}
	logf("Finished saving introduction database to %s", INTRODB_FILE);

}
/**************************************************************************/
// called when a character is deleting... we flag their know info so it 
// isn't saved to disk
void intro_player_delete(char_data *player)
{
	if(!player){
		return;
	}

	// bounds check
	assert(player->know_index>0);
	assert(player->know_index<MAX_KNOW_SET);
	
	if(intro_data_database[player->know_index]->owner_id!=player->player_id){
		ilogf("intro_player_delete(): intro_data_database[player->know_index]->owner_id!=player->player_id, player=%s, knowindex=%d",
			player->name, player->know_index);
		bugf("intro_player_delete(): intro_data_database[player->know_index]->owner_id!=player->player_id, player=%s, knowindex=%d",
			player->name, player->know_index);
		// looks like we have some corruption
		do_abort();
		return;
	}

	// do a few double checks
	assert(player->player_id==intro_data_database[player->know_index]->owner_id);
	assert(player->know_index==intro_data_database[player->know_index]->my_know_index);

	// set player->know to point the correct location etc
	intro_data_database[player->know_index]->save_in_db=false;
	ilogf("intro_player_delete(): player %s(%d)[%d] flagged for intro database removal.",
		player->name, player->player_id, player->know_index);
	logf("intro_player_delete(): player %s(%d)[%d] flagged for intro database removal.",
		player->name, (int)player->player_id, player->know_index);
		
}
/**************************************************************************/
void load_intro_database()
{
	logf("Load introduction database from %s", INTRODB_FILE);
	FILE *fp=fopen( INTRODB_FILE,"r");
	memset(&intro_data_database[0], 0, sizeof(intro_data*)* (MAX_KNOW_SET));
	if(!fp){
		bugf("load_intro_database(): Couldn't open file '%s' to load introduction database from!",
			INTRODB_FILE);
		log_note("NOTE: The introduction database stores who knows who, if this is a new mud "
			"then it is expected that this file is missing until a mortal uses the "
			"introduction system.  If this is an existing mud which is making use of the "
			"introduction system, then player introductions will have to be recreated.  "
			"It is safe to delete this file at any stage if required.");
		return;
	}

	int know_index=0;
	while(!feof(fp)){
		know_index=fread_number(fp);

		if(know_index==-1){// end of file marker
			break;
		}

		if(know_index==-2){// comment to end of line
			fread_to_eol(fp); // allow expansion later on/comments
			continue;
		}

		if(know_index==-3 || know_index==-4 || know_index==-5){// know index to owner id cross reference data (save stage 1)
			int nlast_logged_in;
			bool read_last_login=true;
			bool read_name_and_date=true;
			switch(know_index){
				case -3:
					read_last_login=false;
				case -4:
					read_name_and_date=false;
				case -5:
					break;
				default:
					assert(!"load_intro_database(): case condition not handled - we shouldn't ever get here!");
					break;
			}
			know_index=fread_number(fp);

			if(know_index<10 || know_index>MAX_KNOW_SET){
				bugf("load_intro_database(): -3/-4 know_index %d invalid!"
					"- not within level valid range... exiting!", know_index);
				exit_error( 1 , "load_intro_database", "-3 know_index invalid");
			}

			int nid=fread_number(fp);
			if(!intro_data_database[know_index]){
				intro_data_database[know_index]=new intro_data;	
			}
			intro_data_database[know_index]->owner_id=nid;
			intro_data_database[know_index]->my_know_index=know_index;

			// get the last logged in details
			if(read_last_login){
				nlast_logged_in=fread_number(fp);
			}else{ // set a default value
				nlast_logged_in=(int)current_time;
			}
			intro_data_database[know_index]->last_logged_in=nlast_logged_in;

			// get the name & consume the date text
			if(read_name_and_date){				
				intro_data_database[know_index]->owner_name= str_dup(fread_word(fp));
				fread_word(fp); // we don't need the date, it is there for troubleshooting
			}		

			fread_to_eol(fp); // allow expansion later on/comments
			continue;
		}

		// below here is assumed to be a result of stage 2 saving
		if(know_index<10 || know_index>MAX_KNOW_SET){
			bugf("load_intro_database(): know_index %d invalid!"
				"- not within level valid range... exiting!", know_index);
			exit_error( 1 , "load_intro_database", "know_index invalid");
		}
		int nindex=fread_number(fp);
		int nlastseen=fread_number(fp);
		fread_to_eol(fp); // allow expansion later on/comments

		// only if both people involved still exist, will it 
		// accept the know info
		if(intro_data_database[know_index] && intro_data_database[nindex]){ 
			intro_data_database[know_index]->load_person(nindex, nlastseen);
		}
	}
	if(feof(fp) && know_index!=-1){
		logf("load_intro_database(): load incomplete, exiting!");
		exit_error( 1 , "load_intro_database", "load incomplete");
	}

	logf("load_intro_database(): Load complete.");
	fclose(fp);
}
/**************************************************************************/
/**************************************************************************/


