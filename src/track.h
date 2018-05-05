/**************************************************************************/
// track.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef TRACK_H
#define TRACK_H

// prototypes
void tracktime_update();
void init_track_table();
void init_room_tracks(); 

enum tracktype {TRACKTYPE_NONE, TRACKTYPE_MOVE, TRACKTYPE_FLY, 
    TRACKTYPE_SNEAK, TRACKTYPE_WALK, TRACKTYPE_BLOODTRAIL, 
	TRACKTYPE_BLOODPOOL, TRACKTYPE_WIZIIMM, TRACKTYPE_PASSWOTRACE, 
	TRACKTYPE_RIDING};

#define MTC MAX_TRACKABLE_CHARACTERS_IN_GAME
/**************************************************************************/
class C_track_table
{
public:
	C_track_table(); // constructor
	
	char * get_pers(int index, char_data *looker);
	char_data * get_char(int index);
	int add_char(char_data *ch);
	void del_char(int index);
	void del_char(char_data *ch);
	char *get_race(int index);
	bool is_npc(int index);
	bool is_active(int index);
	int get_total_tracked_characters();
private:
	int get_race_value(int index);

	// VALUES STORED IN CLASS BELOW HERE
	unsigned short race_oldchar[MTC];// stores the race and MSB is true 
								// if the character pointer is invalid
	char_data *character[MTC];  // if MSB of race_oldchar is 1, then that means
								// character doesnt point to any char_data
								// but is used as a bit array.
	unsigned char bitflags[MTC];
	int total_tracked_characters;
	int next_free_track;
	
};

/**************************************************************************/
class C_track_data{
public:
	C_track_data(); // constructor
	void add_track(char_data *ch, int direction, tracktype type); 
	void show_tracks(char_data *ch);
private:
	tracktype get_tracktype(int index);
	void set_tracktype(int index, tracktype type);
	int get_direction(int index);
	void set_direction(int index, int direction);

	// the reason the array is in here, is because most compilers expand
	// a struct with 2 unsigned shorts and 1 unsigned char to 6bytes
	unsigned short	trackindex[MAX_TRACKS_PER_ROOM];
	unsigned short	time_of_track[MAX_TRACKS_PER_ROOM];
	unsigned char	direction_type[MAX_TRACKS_PER_ROOM];  
	unsigned char	nexttrack;
	// direction_type - first 4 bits are direction, second are type
};
/**************************************************************************/

extern C_track_table *track_table;

#endif // TRACK_H
