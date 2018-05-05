/**************************************************************************/
// entity.cpp - entity superclass, Kal, Apr05
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"
#include "events.h"
#include "tables.h"
/**************************************************************************/
void eventqueue_purge_events_for(entity_data *extracted_char, sh_int purge_group);
/**************************************************************************/
// dequeue all the queued events involving us
void entity_data::mudqueue_dequeue_all(sh_int purge_group)
{
	if(this && events){
		eventqueue_purge_events_for(this, purge_group);
	}
};  

/**************************************************************************/
const char * entity_data::entity_type_to_text(entity_type et)
{
	static char result[MSL];
	switch(et){
		case ENTITYTYPE_BASE:	return "base";
		case ENTITYTYPE_CH:		return "ch";
		case ENTITYTYPE_OBJ:	return "obj";
		default: {
			sprintf(result, "entity_type_to_text(): undefined entity type (%d)!!!", (int)et);
			return result;
		}
	}
}

/**************************************************************************/
const char * entity_data::get_entitytype_text()
{
	return entity_type_to_text(get_entitytype());
}

/**************************************************************************/
void entity_data::queue_print(sh_int group, int seconds, const char *text)
{
	mpqueue_queue_event_for_entity(ET_PRINT, group, seconds, text, this);
}
/**************************************************************************/
void entity_data::queue_command(sh_int group, int seconds, const char *argument)
{
	mpqueue_queue_event_for_entity(ET_COMMAND, group, seconds, argument, this);
}

/**************************************************************************/
void entity_data::enumerate_events(char_data* send_to)
{
	send_to->printlnf("There are %d event%s associated with entity:", 
		events_count, events_count==1?"":"s");
	if(events_count){
		send_to->println(":InSeconds:`GSource MudProg`x:EventType EventText");
	}
	events->enumerate_events_for_entity_chain(send_to);
}
/**************************************************************************/
void entity_data::suspend_events()
{
	events->suspend_events_for_entity_chain();
}
/**************************************************************************/
void entity_data::unsuspend_events()
{
	events->unsuspend_events_for_entity_chain();
}
/**************************************************************************/
void entity_data::save_events(FILE *fp)
{
	if(!events){
		// no events to save
		return;
	}

	fprintf(fp, "Events 1\n"); // The 1 is the format version
	events->save_events_for_entity_chain(fp);
	fprintf(fp, "-1\n"); // Mark the end of the events

}
/**************************************************************************/
void entity_data::load_events(FILE* fp)
{
	// read to the end of the line
	int events_save_version=fread_number(fp);
	if(events_save_version>1){
		logf("entity_data::load_events(): warning, loading events from version %d, we only recognise up to 1.",
			events_save_version);
	}
	fread_to_eol(fp); // read to the end of the line, incase any other info is put here

	// now read the first column, it can either be a event group number, or an end marker
	int group;
	
	group=fread_number(fp);
	while(group>=0){
		int seconds=fread_number(fp);
		int source_mudprog=fread_number(fp);

		// read in the event flags
		char *_flags=fread_word(fp);
		int flags=wordflag_to_value(event_flags, _flags);
		if(flags==NO_FLAG){
			bugf("entity_data::load_events(): Unfound recognised event flag '%s' when reading in event.",_flags);
			flags=0;
		}

		// read in the event type
		char *_type=fread_word(fp);
		eventtype type=(eventtype)wordflag_to_value(event_types, _type);
		if(type==NO_FLAG){
			bugf("entity_data::load_events(): Unfound recognised event type '%s' when reading in event.",_type);
			type=ET_UNDEFINED;
		}

		char *text=fread_string(fp);
		// create a new event, and automatically enqueue it
		
		new event_data(false, type, group, seconds, this, text, source_mudprog);	
		REMOVE_BIT(flags, EVENTFLAG_QUEUED); // all loaded events start out unqueued
			// as the transition into the game, queues them unless they load into ooc
			// the queuing is done by char_to_room() calling ch->moving_from_ooc_or_load_to_ic();
		SET_BIT(events->flags,flags);
		free_string(text);

		fread_to_eol(fp); // read to the end of the line, incase any other info is put here

		// start reading the next event
		group=fread_number(fp);
	}
	fread_to_eol(fp); // read to the end of the line, incase any other info is put here
}


/**************************************************************************/
/**************************************************************************/

