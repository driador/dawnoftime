/**************************************************************************/
// locker.h - locker header
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef LOCKERS_H
#define LOCKERS_H

// notes on locker implementation at the head of lockers.cpp

#define LOCKER_HASH_KEY (47)
/**************************************************************************/
struct locker_room_data
{
	sh_int	quantity;		// quantity in room
	sh_int	initial_rent;	// initial IC monthly rent and setup charge
	sh_int	ongoing_rent;	// ongoing IC yearly rent
	sh_int	weight;			// oedit's v0
	sh_int	capacity;		// oedit's v3
	sh_int	pick_proof;		// if a locker is pickproof
};

/**************************************************************************/
struct locker_data // information about a single locker in a room
{
	int roomvnum;		// room the locker is stored in
	sh_int number;		// locker number within the room
	char *owner;		// name of owner of locker
	char *access;		// names of those with access	
	char *label;		// label on exterior of locker
	time_t paid_until;	// the rent is paid until this time

	time_t last_accessed;

	OBJ_DATA *locker_object;

	locker_data * next_locker;
};
/**************************************************************************/
class lockers_object // this locker is treated as a normal object
{
public:
	bool room_has_lockers(ROOM_INDEX_DATA * room); // return true if room player is in has lockers
	OBJ_DATA *find_locker_object(char_data *ch, bool display_messages, int number); // return the requested locker object
	locker_data *find_locker_data(char *owner, int roomvnum, int number); // return the requested locker data 
	locker_data *find_locker(char_data *ch, bool display_messages, int number); // return the requested locker data
	void admin_delete(char_data *ch, int locker_number); // admin deleting a particular locker
	lockers_object(); // constructor

	bool has_access(char_data *ch, locker_data *l);
	bool has_access(char_data *ch, OBJ_DATA *locker_object);
	
	void remove_access(char_data *ch, char *argument, int locker_number);
	void grant_access(char_data *ch, char *argument, int locker_number);

	void postletter(char_data *ch, int locker_number);

	void start_rent(char_data *ch, int locker_number);
	void pay_rent(char_data *ch, int locker_number);
	void lockers_load_db();
	void lockers_save_db();

	void changeowner(char_data *ch, char *argument, int locker_number);
	void changelabel(char_data *ch, char *argument, int locker_number);
	void tempopen(char_data *ch, char *argument, int locker_number);

	void look(char_data *ch);
	void roomlist(char_data *ch, char *name);

	locker_data *get_first(int roomvnum);	
		// get_first() returns a starting place to 
		// start searching for rooms of a given locker vnum
		// (adding this now, will make it easier to use hash tables later)

	int count_used_lockers_in_room(int roomvnum);
	int get_next_free_locker_number(int roomvnum); // return the next available locker number in the room
	void info(char_data *ch, int locker_number);

	OBJ_DATA *locker_contains;
	locker_data * next_locker;
	locker_data * locker_data_of_last_found_locker_object;

private:
	locker_data *hash[LOCKER_HASH_KEY];
};

/**************************************************************************/
extern lockers_object *lockers;
/**************************************************************************/
/**************************************************************************/
#endif // LOCKERS_H

