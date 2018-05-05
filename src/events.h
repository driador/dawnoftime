/**************************************************************************/
// events.h - the queuing definitions - Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef __events_H
#define __events_H

#define EVENTGROUP_PURGE_EVERY_EVENT_INCLUDING_SYSTEM_EVENTS -1
#define EVENTGROUP_PURGE_ALL	0
#define EVENTGROUP_DEFAULT		1
#define EVENTGROUP_SYSTEM		32000
#define EVENTGROUP_SYSTEM_PRINT	32001

/**************************************************************************/
#define EVENTQUEUE_HASH_SIZE (6151)
//#define EVENTQUEUE_TRACE_ENABLED 1

/**************************************************************************/
#define EVENTFLAG_QUEUED	(A)
#define EVENTFLAG_EXECUTING	(B)
/**************************************************************************/
enum eventtype {ET_UNDEFINED, ET_COMMAND, 
	ET_PRINT, ET_ECHOTO, ET_ECHOAROUND, ET_ECHOROOM};

/**************************************************************************/
class event_data{
public:
	int id; // a unique id number for debugging
	eventtype type; // the type of queue - ET_UNDEFINED, ET_COMMAND, ET_PRINT, ET_ECHOAT, ET_ECHOAROUND, ET_ECHOROOM
	int flags;
	char *text; // what to say, print, command, do etc	
	sh_int group;
	
public: // public member functions
	event_data();// default constructor
	event_data(bool automatically_enqueue_event, eventtype t, sh_int event_group, int seconds, entity_data * ent, const char *argument, int source_mudprog_number);
	~event_data();// deconstructor

	void dequeue(); // remove it from the queue
	void enqueue(); // add it to the queue

	void suspend_events_for_entity_chain();
	void unsuspend_events_for_entity_chain();

	event_data *get_next_in_queue() {return next_in_queue;};
	event_data *get_next_for_entity() {return next_for_entity;};
	int get_seconds(); // seconds till execute (assuming the event was currently queued)
	char *get_debug_details();

	entity_data* ent;// the entity we are dealing with
	int when; // number of pulses till event should happen if suspended, or pulse number for it to happen on
				// this depends on if the EVENTFLAG_QUEUED is set

	int source_mudprog;

	void enumerate_events_for_entity_chain(char_data *send_to);

	void save_events_for_entity_chain(FILE *fp);
private:	
	// a doubly linked list of events which are currently queued for execution
	event_data *next_in_queue; // next in hash bucket, if NULL, bottom of the chain
	event_data *prev_in_queue; // previous in hash bucket, if NULL, top of the bucket

	// a doubly linked list of events relating to a specific entity	
	event_data *next_for_entity; // next in hash bucket, if NULL, bottom of the chain
	event_data *prev_for_entity; // previous in hash bucket, if NULL, top of the bucket

};

/**************************************************************************/
void mpqueue_queue_event_for_entity(eventtype t, sh_int group, int seconds, const char *argument, entity_data *ent);

/**************************************************************************/


#endif // __events_H
