/**************************************************************************/
// entity.h - the superclass for mobs and objects
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef __entity_H
#define __entity_H

enum entity_type {ENTITYTYPE_BASE, ENTITYTYPE_CH, ENTITYTYPE_OBJ};
class event_data;

class entity_data
{
public:
	// parameters common to all entities
	char *name;
	char *description;
	bool valid;

public: // virtualised functions
	virtual ~entity_data() {}; // a virtual destructor is required based on C++ specs
	virtual entity_type get_entitytype() {return ENTITYTYPE_BASE;};
	virtual vn_int vnum(){return 0;}; // a safe way to get the vnum from a mob/player
	const char *get_entitytype_text();
	OBJ_DATA *running_mudprog_for_object; // should only be set by char_data entities

public:
// member functions
	void mudqueue_attached(time_t when);
	void mudqueue_dequeue_all(sh_int purge_group);  // dequeue all the queued events involving us

// queuing stuff
	void queue_print(sh_int group, int seconds, const char *text);
	void queue_command(sh_int group, int seconds, const char *argument);

//	vn_int vnum(); // a safe way to get the vnum from a mob/player
//	vn_int in_room_vnum(); // a safe way to get the vnum of a room

public:
//	int uid; // unique id

protected:

public:		
	virtual void moving_from_ic_to_ooc() {return;}; 
	virtual void moving_from_ooc_or_load_to_ic() {return;}; 
	void save_events(FILE *fp); // serialize the events of an entity, including writing a "EventsStart 1" header and a "-1" end of events marker
	void load_events(FILE *fp);
	void suspend_events(); 
	void unsuspend_events();
	void enumerate_events(char_data* send_to); // return a list of the events
	int events_count; // just a basic count for now - includes all suspended
	event_data *events;
private:
	const char * entity_type_to_text(entity_type et); // convert entity_type into a char* representation

};

#endif // __entity_H
