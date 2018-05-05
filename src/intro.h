/**************************************************************************/
// intro.h - Introduction system - Kal May 2000
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef INTRO_H
#define INTRO_H

/**************************************************************************/
class intro_node{
public:
	unsigned short know_index;    // 10->65000 - unique number for a player's know info
	time_t last_seen;  
	intro_node *next;

	intro_node *find_know_node_by_index(unsigned short lookup_know_index); // find a node in the *next list
	bool add_person_by_index(unsigned short new_person_know_index, intro_data *know);
	bool valid;
};
/**************************************************************************/
class intro_data{
public:
	intro_data(){owner=NULL; people=NULL; intro_cache=NULL; 
				intro_cache_index=0;owner_id=0; my_know_index=0; save_in_db=true; owner_name=str_dup(""); suppress_old_unsaved_log=false;}; // constructor
	~intro_data() { replace_string(owner_name,"")};
	char *owner_name; // a copy of the owners name
	void assign_owner(char_data *ch);
	void allocate_intro_cache(); 
	bool knows(char_data *person);// check if they know the person
	bool introduced_to(char_data *person); // when you are introduced to someone
	void forgetting(char_data *person); // when you choose to forget someone
	bool load_person(unsigned short new_person_id, time_t last_seen);
	bool load_id(time_t player_id);

	unsigned short my_know_index;	// the known index for the player
	intro_node *people;
	time_t owner_id;		// the game id of the player
	time_t last_logged_in;	// players that don't login after a set period of time are automatically purged.

	bool save_in_db;
	bool suppress_old_unsaved_log;
private:
	unsigned char *intro_cache;
	unsigned short intro_cache_index; // if not 0, then we are caching them
	char_data *owner; // owner of this node
};
/**************************************************************************/
void attach_know(char_data* player);
void save_intro_database();
void intro_player_delete(char_data *player);

#endif // INTRO_H

