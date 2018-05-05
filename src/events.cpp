/**************************************************************************/
// events.cpp - mud queue system - Kalahn
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
#include "areas.h" // for pack_word()

/**************************************************************************/
static int events_pulse_counter=0;
/**************************************************************************/
int events_queued_count=0;
int events_queued_total=0;
static int id_counter=1; // hack for a unique id
static event_data *eventqueue_hash_table[EVENTQUEUE_HASH_SIZE];
/**************************************************************************/
const struct flag_type event_flags[] =
{
	{  "queued",		EVENTFLAG_QUEUED, 			false	},
	{  "executing",		EVENTFLAG_EXECUTING, 		false	},
	{  NULL,		   0,		0	}
};
/**************************************************************************/
const struct flag_type event_types[] =
{
	{  "undefined",		ET_UNDEFINED, 			false	},
	{  "command",		ET_COMMAND, 			false	},
	{  "print",			ET_PRINT, 				false	},
	{  "echoto",		ET_ECHOTO, 				false	},
	{  "echoaround",	ET_ECHOAROUND, 			false	},
	{  "echoroom",		ET_ECHOROOM, 			false	},
	{  NULL,		   0,		0	}
};

/**************************************************************************/
// convert the eventqueue type into TEXT
const char *eventtype_to_text(eventtype eqt)
{
	return flag_string(event_types, (int)eqt);
}
/**************************************************************************/
// default constructor
event_data::event_data()
{
	id=id_counter++;
	flags=0;
	type=ET_UNDEFINED;
	ent=NULL;
	source_mudprog=0;
	text=str_dup("");
	when=0;
	next_in_queue=NULL;
	prev_in_queue=NULL;
	next_for_entity=NULL;
	prev_for_entity=NULL;

	// record some statistics
	events_queued_count++;
	events_queued_total++;

};
/**************************************************************************/
// default destructor
event_data::~event_data()
{	
	if(next_in_queue){
		bug("Warning: event_data::~event_data() called when next_in_queue!=NULL!!!");
	}
	if(prev_in_queue){
		bug("Warning: event_data::~event_data() called when prev_in_queue!=NULL!!!");
	}
	free_string(text);

	// remove from the events list

	// remove in the forward direction
	if(prev_for_entity){
		// not the first in the list
		prev_for_entity->next_for_entity=next_for_entity;
	}else{
		// first in the list
		ent->events=next_for_entity;
	}

	// remove in the backwards direction
	if(next_for_entity){
		next_for_entity->prev_for_entity=prev_for_entity;
	}
	ent->events_count--;
	ent=NULL;
	next_for_entity=NULL;
	prev_for_entity=NULL;

	// maintain some statistics
	events_queued_count--;
};
/**************************************************************************/
// custom constructor for event_data that handles setting up most things
event_data::event_data(bool automatically_enqueue_event, eventtype t, sh_int event_group, int seconds, entity_data * e, const char *argument, int source_mudprog_number)
{
	id=id_counter++;
	flags=0;
	type=t;
	text=str_dup(argument);
	group=event_group;
	when=seconds*PULSE_PER_SECOND;	
	ent=e;
	source_mudprog=source_mudprog_number;
	next_in_queue=NULL;
	prev_in_queue=NULL;
	smash_tilde(text); // safety code

	// insert at the start of the events list for the entity
	ent->events_count++;
	next_for_entity=e->events;
	if(next_for_entity){
		next_for_entity->prev_for_entity=this;
	}
	ent->events=this;
	prev_for_entity=NULL;

	if(automatically_enqueue_event){
		enqueue();
	}

	// record some statistics
	events_queued_count++;
	events_queued_total++;
};

/**************************************************************************/
// return the when value in seconds, if it queued, convert it to seconds
int event_data::get_seconds()
{
	if(IS_SET(flags, EVENTFLAG_QUEUED)){
		return (when-events_pulse_counter)/PULSE_PER_SECOND;
	}else{
		return when/PULSE_PER_SECOND;
	}
}
/**************************************************************************/
// dequeue a node from the event list, while leaving it in the entities
// event queue list - so it can be requeued at a later time
void event_data::dequeue()
{
#ifdef EVENTQUEUE_TRACE_ENABLED
	logf("event_data::dequeue(%d): x->p->n=x->n; x->p=%d, x->n=%d flags=0x%0x", id, prev_in_queue?prev_in_queue->id:-1,next_in_queue?next_in_queue->id:-1, flags);
#endif
	assert(IS_SET(flags, EVENTFLAG_QUEUED)); 

	// first remove ourself from the queued list, 
	// then we will dereference our own pointers

	if(!prev_in_queue){ // if there is no previous pointer, we must be at the head of the list
		// find which hash bucket we are stored
		int hash=when%EVENTQUEUE_HASH_SIZE;
		// make that hash bucket point to our next_in_queue pointer
		eventqueue_hash_table[hash]=next_in_queue;
		if(next_in_queue){
			// mark the next_in_queue pointer's previous as NULL
			next_in_queue->prev_in_queue=NULL;
		}
	}else{
		// we have a previous pointer, therefore just make our previous point past us
		prev_in_queue->next_in_queue=next_in_queue;
		// if there was something after us, link its previous back to back to our previous
		if(next_in_queue){
			next_in_queue->prev_in_queue=prev_in_queue;
		}
	}

	// we are no longer linked in, so clear our pointers
	next_in_queue=NULL;
	prev_in_queue=NULL;

	// change the timer back to how many pulses till execution, and mark it as out of the queue
	when=when-events_pulse_counter;
	REMOVE_BIT(flags, EVENTFLAG_QUEUED); 
}

/**************************************************************************/
// enqueue a node for scheduled execution
void event_data::enqueue()
{
	assert(!IS_SET(flags, EVENTFLAG_QUEUED)); 
	assert(next_in_queue==NULL);
	assert(prev_in_queue==NULL);

	// first update the when value, to indicate the pulse which it will execute on
	// (instead of in how many seconds)
	when=events_pulse_counter+when;
	
	// first find which hash bucket it belongs in	
	int hash=when%EVENTQUEUE_HASH_SIZE;

	if(eventqueue_hash_table[hash]){
		if(eventqueue_hash_table[hash]->when>when){
			// we are to be the first entry in the bucket, since we need to occur earlier
			// than the currently first scheduled event

			// make the current entry the next_in_queue event
			next_in_queue=eventqueue_hash_table[hash];
			// we come previous to that entry
			next_in_queue->prev_in_queue=this;
			// we become the first entry
			eventqueue_hash_table[hash]=this;

		}else{
			// we are to be inserted somewhere after the first entry, 
			// as eventqueue_hash_table[hash]->when < our when
			event_data *preceedingnode=eventqueue_hash_table[hash];

			// keep moving the preceeding node one along the chain, until doing so 
			// would result in a preceeding node that is due to happen after us
			while(preceedingnode->next_in_queue && preceedingnode->next_in_queue->when<=when){
				preceedingnode=preceedingnode->next_in_queue;
			}

			// we need to link in immediately after the preceedingnode

			// insert ourselves in between the preceedingnode and its next_in_queue pointer
			next_in_queue=preceedingnode->next_in_queue;
			preceedingnode->next_in_queue=this;

			// link our previous to the preceedingnode
			prev_in_queue=preceedingnode;
			if(next_in_queue){ // if we aren't at the end of the list, link it back to us
				next_in_queue->prev_in_queue=this;
			}
		}
	}else{
		// we are the only entry to be inserted into the hashbucket
		eventqueue_hash_table[hash]=this;
		// no next_in_queue or prev_in_queue events to configure
	}

	// mark the event as queued
	SET_BIT(flags, EVENTFLAG_QUEUED); 

#ifdef EVENTQUEUE_TRACE_ENABLED
	logf("event_data::enqueue(%d): x->p->n=x->n; x->p=%d, x->n=%d flags=0x%0x", id, prev_in_queue?prev_in_queue->id:-1,next_in_queue?next_in_queue->id:-1, flags);
#endif

}

/**************************************************************************/
static void event_purge_node(event_data *node)
{
#ifdef EVENTQUEUE_TRACE_ENABLED
	logf("event_purge_node(%d) g=%d sec=%d][%s,%d] %-8s='%s' (%s)", 
		node->id,
		node->group,
		node->get_seconds(),
		node->ent->get_entitytype_text(),
		node->ent->vnum(),
		eventtype_to_text(node->type),
		node->text,
		node->ent->name
		);
#endif

	// if the node is currently queued, extract it
	if(IS_SET(node->flags, EVENTFLAG_QUEUED)){
		node->dequeue();
	}

	// delete the node now
	delete node;

}

/**************************************************************************/
// return some details for rebugging
char *event_data::get_debug_details()
{
	static char result[MSL];
	char *tbuf;
	time_t t=current_time+get_seconds();
	tbuf=ctime(&t);
	tbuf[str_len(tbuf)-6] = '\0';
	sprintf(result, "###event id=%d g=%d %s p=%d n=%d t='%s'", id, group, tbuf+4, prev_in_queue?prev_in_queue->id:-1, next_in_queue?next_in_queue->id:-1, text);

	return result;
}

/**************************************************************************/
void eventqueue_purge_events_for(entity_data *extracted_entity, sh_int purge_group)
{
	logf("eventqueue_purge_events_for('%s' %d)", extracted_entity->name, extracted_entity->vnum());

	event_data *node, *node_next;
	for(node=extracted_entity->events; node; node=node_next){
		node_next=node->get_next_for_entity();
		if(IS_SET(node->flags, EVENTFLAG_EXECUTING)){
			// we have found a particular note that is executing
			// don't delete it... as eventqueue_execute() will delete it
			// once it has finished its execution
			continue;
		}

		if(purge_group!=EVENTGROUP_PURGE_EVERY_EVENT_INCLUDING_SYSTEM_EVENTS){
			if(purge_group!=EVENTGROUP_PURGE_ALL && node->group!=purge_group && node->group<EVENTGROUP_SYSTEM){
				continue;
			}
		}


#ifdef EVENTQUEUE_TRACE_ENABLED
		log_string(node->get_debug_details());
#endif 				
		// purge the node (remove from queue, delete it and decrement the count)
		event_purge_node(node);
	}
}


/**************************************************************************/
// syntax: mpdequeueall 
void do_mpdequeueall(char_data *ch, char *argument)
{
	char first_arg[MIL];
	sh_int purge_group;
	// find the event group they are wanting to purge
	one_argument(argument,first_arg);

	// validate arguments
	if(IS_NULLSTR(first_arg)){ // dequeue all groups
		purge_group=EVENTGROUP_PURGE_ALL;
	}else{
		if(!is_number(first_arg)){
			mpbugf("do_mpdequeueall: optional argument must specify a event group number between 0 and 31999.");
			mpbugf("incorrect argument is '%s', purging all progs", first_arg);
			purge_group=EVENTGROUP_PURGE_ALL;
		}else{
			purge_group=atoi(first_arg);
			if(purge_group<0 || purge_group>=EVENTGROUP_SYSTEM){
				mpbugf("do_mpdequeueall: optional argument must specify a event group number between 0 and 31999.");
				mpbugf("incorrect argument is '%s', purging all progs", first_arg);
				purge_group=EVENTGROUP_PURGE_ALL;
			}
		}
	}

	if(ch->running_mudprog_for_object){
		// technically an object prog mob can call this on behalf of an 
		// object.  If so, we need to dequeue on the object
		eventqueue_purge_events_for(ch->running_mudprog_for_object, purge_group);
	}else{
		eventqueue_purge_events_for(ch, purge_group);
	}
}
/**************************************************************************/
char_data *objectmudprog_generate_mobified_object(obj_data *obj, char_data * ch, int otrig);
void objectmudprog_degenerate_mobified_object(char_data *objectmudprog_mob);
void eventqueue_execute(event_data *node);
/**************************************************************************/
// execute a single queue node in the hash table queue
void eventqueue_purge_then_execute_next(event_data *node)
{	
	assertp(node);
	event_data *next_to_execute=node->get_next_in_queue();
	// purge the node (remove from queue, delete it and decrement the count)
	event_purge_node(node);

	if(next_to_execute){
		// run deeper nodes of the same time
		eventqueue_execute(next_to_execute);		
	}

}
/**************************************************************************/
// execute a single queue node in the hash table queue
void eventqueue_execute(event_data *node)
{	
	assertp(node);

	if(node->when>events_pulse_counter){ // check if the queued event is ready to run
		return;
	}

	if(!node->ent){
		// NULL node->ch... this can occur when a prog purges itself
#ifdef EVENTQUEUE_TRACE_ENABLED
		logf("########eventqueue_execute(): node->ent==NULL, '%s', queue system deleting", node->text);
#endif
		// finished with this queue entry, purge it, then run the next_in_queue entry if appropriate
		eventqueue_purge_then_execute_next(node);
		return;
	}

	char_data *entity_ch=NULL;
	obj_data *entity_obj=NULL;
	switch(node->ent->get_entitytype()){
		case ENTITYTYPE_CH:
			entity_ch=(char_data*)node->ent;
			break;
		case ENTITYTYPE_OBJ:
			{
				entity_obj=(obj_data*)node->ent;
				char_data *objectmudprog_mob=NULL;
				// generate a mob to run the queued event on behalf of the object (create + transfer what object contains to inventory)
				objectmudprog_mob=objectmudprog_generate_mobified_object(entity_obj, NULL, 0);

				if(!objectmudprog_mob){
					// failed to create an objectmudprog_mob for some reason
					// so we can't go any further
					bugf("########eventqueue_execute(): failed to create an objectmudprog_mob for some reason, '%s', queue system purging queued event", node->text);

					// finished with this queue entry, purge it, then run the next_in_queue entry if appropriate
					eventqueue_purge_then_execute_next(node);
					return;
				}

				entity_ch=objectmudprog_mob;
			}
			break;

		default:
			logf("eventqueue_execute(): Entity type '%s' not supported, prog type='%s',text='%s'", 
				node->ent->get_entitytype_text(),
				eventtype_to_text(node->type),
				node->text				
				);
			do_abort();
			break;
	}


	// flag that we are currently executing the node so if the entity if 
	// removed from the game by other means (eg. "mq5 mob purge self")
	// the node isn't deleted, then we try to delete it at the bottom
	// of this function.
	SET_BIT(node->flags, EVENTFLAG_EXECUTING);

	switch(node->type){
		case ET_COMMAND:
				interpret(entity_ch, node->text);
			break;

		case ET_PRINT:
			entity_ch->set_pdelay(0); // incase someone stuffed up and we would get an endless loop
			entity_ch->print(node->text);
			break;

		case ET_ECHOTO:
			act(node->text, entity_ch, NULL, NULL, TO_CHAR);
			break;

		case ET_ECHOAROUND:
			act(node->text, entity_ch, NULL, NULL, TO_ROOM);
			break;

		case ET_ECHOROOM:
			act(node->text, entity_ch, NULL, NULL, TO_CHAR);
			act(node->text, entity_ch, NULL, NULL, TO_ROOM);
			break;
			
		default:
			bugf("eventqueue_execute(event_data *node): Unsupported queued event type (%d)!", node->type);
			break;
	}
	// NULL node->ch

	// if we were running a queue event for an object, demobify the object_mob
	if(entity_obj){
		// absorb mob back into the object (transfer objects back + purge)
		objectmudprog_degenerate_mobified_object(entity_ch);
	}
	
	// finished with this queue entry, purge it, then run the next_in_queue entry if appropriate
	eventqueue_purge_then_execute_next(node);
}
/**************************************************************************/
// called every pulse, by update_handler
void process_events_queue()
{	
	events_pulse_counter++;

	// find what we are running this pulse - if anything
	int hash=events_pulse_counter%EVENTQUEUE_HASH_SIZE;
	
	if(!eventqueue_hash_table[hash] || eventqueue_hash_table[hash]->when>events_pulse_counter){
		return; // nothing to run this time
	}

	// run it, and any others which follow and should be run
	eventqueue_execute(eventqueue_hash_table[hash]);

	return;
}
/**************************************************************************/
// syntax: mp queue [g#,]<seconds> command to run 
void do_mpqueue(char_data *ch, char *argument)
{
	char group_arg[MIL];
	char first_arg[MIL];
	char *pSeconds;
	char *pComma;
	sh_int group;
	
	char fullarg[MIL];
	strcpy(fullarg, argument);
	
	argument=one_argument(argument,first_arg);

	// validate arguments
	if(IS_NULLSTR(argument)){ // not enough arguments
		mpbugf("do_mpqueue: not enough arguments, must have at least seconds and a command to queue.");
		mpbugf("syntax: mob queue [g#,]<seconds> command to queue");
		mpbugf("where the # in g# is the group number in the range 0 to 32000.");
		return;
	}

	// the pick off the optional group number from the front of the seconds argument
	// or g#,#  (where g#= the group number, and # = the seconds)
	pComma=strstr(first_arg, ",");
	if(pComma && LOWER(first_arg[0])=='g'){
		*pComma='\0';
		strcpy(group_arg, &first_arg[1]); // skip over the leading g
		if(!is_number(group_arg)){
			mpbugf("do_mpqueue: invalid group argument, look at 'mudhelp queue' for syntax.");
			mpbugf("buggy argument in: '%s'", fullarg);
			return;
		}
		group=atoi(group_arg);
		if(group<1 || group>=32000){
			mpbugf("do_mpqueue: The group if specified must be greater than 0 and less than 32000, command not queued.");
			mpbugf("buggy argument in: '%s'", fullarg);
			return;
		}
		pSeconds=pComma+1; // skip to what immediately follows the comma
	}else{
		// optional group not present
		pSeconds=first_arg;
		group=EVENTGROUP_DEFAULT; // default the group to 1
	}

	if(!is_number(pSeconds)){
		mpbugf("do_mpqueue: [g#,]<seconds> The seconds value ('%s') must be numeric.",  pSeconds);
		mpbugf("syntax: mob queue [g#,]<seconds> command to queue");
		mpbugf("buggy argument in: '%s'", fullarg);
		return;
	}
	
	// START PROCESSING AND VALIDATING ARGUMENTS
	// Number of seconds in which prog will be run
	int seconds=atoi(pSeconds);
	if(seconds<1 || seconds>2160000){
		mpbugf("do_mpqueue: seconds parameter (%s) must be between 1 and 2160000 (25 days).", pSeconds);
		mpbugf("syntax: mob queue [g#,]<seconds> command to queue");
		mpbugf("buggy argument in: '%s'", fullarg);
		return;
	}

	// actually queue the event
	ch->queue_command(group, seconds, argument);
	
}

/**************************************************************************/
// Dump the contents of the eventqueue
void do_dumpeventqueue(char_data *ch, char *argument)
{
	// scan the table for the oldest time
	time_t latest_time=0;
	time_t oldest_time=current_time;
	time_t look_for_time;
	int i;
	bool displayed=false;
	event_data *node;

	for(i=0; i<EVENTQUEUE_HASH_SIZE; i++){
		node=eventqueue_hash_table[i];
		while(node){
			if(latest_time<node->when){
				latest_time=node->when;
			}
			if(oldest_time>node->when){
				oldest_time=node->when;
			}

			if(!IS_NULLSTR(argument)){
				ch->printlnf("%d %s%d,%s%d %4d`x]`G%5d`x[%3s,%d] %-8s='%s' (%s)", 
					i,
					colour_table[node->group%14+1].code,
					node->group,
					colour_table[((node->get_seconds())%14)+1].code,
					node->get_seconds(),					
					node->id,
					node->source_mudprog,
					node->ent->get_entitytype_text(),
					node->ent->vnum(),
					eventtype_to_text(node->type),
					node->text,
					node->ent->name
					);
			}

			node=node->get_next_in_queue();
		}
	}

	if(!IS_NULLSTR(argument)){
		return;
	}

	for(look_for_time=oldest_time; look_for_time<=latest_time; look_for_time++){
		int hash=(int)look_for_time%EVENTQUEUE_HASH_SIZE;
		node=eventqueue_hash_table[hash];
		while(node){
			if(node->when==look_for_time){
				if(!displayed){
					ch->titlebar("Events Queued");
					ch->println("grp,secs] src [vnum] type='action'");
					displayed=true;
				}
				
				ch->printlnf("%s%d,%s%4d`x]%s%5d`x[%3s,%s%d`x] %-8s='%s'`x", 
					colour_table[node->group%14+1].code,
					node->group,
					colour_table[node->get_seconds()%14+1].code,
					node->get_seconds(),
					colour_table[node->source_mudprog%14+1].code,
					node->source_mudprog,
					node->ent->get_entitytype_text(),
					colour_table[node->ent->vnum()%14+1].code,
					node->ent->vnum(),
					eventtype_to_text(node->type),
					node->text
					);

			}
			node=node->get_next_in_queue();
		}
	}
	if(!displayed){
		ch->println("There are no outstanding queued events, there may however be suspended events.");
		if(IS_NULLSTR(argument)){
			ch->println("When events are displayed, if you supply any parameter an alternative format will show.");

		}
	}

}
/**************************************************************************/
void mpqueue_queue_event_for_entity(eventtype t, sh_int group, int seconds, const char *argument, entity_data *ent)
{
	if(!IS_VALID(ent)){
		mpbugf("A 'invalid' (probably deleted) entity (type=%s) is trying to queue '%s'.", 
			ent->get_entitytype_text(), argument);
		return;
	}

	// redirect queuing for mobified objects directly to the object the were generated from
	if(ent->running_mudprog_for_object){
		ent=ent->running_mudprog_for_object;
	}

	int source_mudprog=0;
	// determine the mudprog which is performing the queuing
	if(callstack_aborted[call_level-1]){
		// it is possible the command is queued manually, 
		// therefore, no source mudprog
		source_mudprog=callstack_pvnum[call_level-1];
	}

	// create a new event, and automatically enqueue it
	new event_data(true, t, group, seconds, ent, argument, source_mudprog);
	
}
/**************************************************************************/
void event_data::enumerate_events_for_entity_chain(char_data *send_to)
{
	if(this){
		next_for_entity->enumerate_events_for_entity_chain(send_to);
		send_to->printlnf("%s%d`x:%2d:`G%5d`x%s %s %s", 
			colour_table[group%14+1].code,
			(int)group,
			get_seconds(), 
			source_mudprog, 
			IS_SET(flags, EVENTFLAG_QUEUED)?":":"S",
			eventtype_to_text(type), text);
	}
}
/**************************************************************************/
void event_data::suspend_events_for_entity_chain()
{
	if(this){
		if(!IS_SET(flags, EVENTFLAG_EXECUTING)){
			dequeue();
		}
		next_for_entity->suspend_events_for_entity_chain();
	}
}
/**************************************************************************/
void event_data::unsuspend_events_for_entity_chain()
{
	if(this){
		if(!IS_SET(flags, EVENTFLAG_EXECUTING)){
			enqueue();
		}
		next_for_entity->unsuspend_events_for_entity_chain();
	}
}
/**************************************************************************/
void event_data::save_events_for_entity_chain(FILE *fp)
{
	if(this){
		// use recursion to write out the events backwards,
		// so when they load they will be in the original order
		next_for_entity->save_events_for_entity_chain(fp);

		// the use of pack_word(flag_string(...)...) is just like fwrite_affect()

		if(IS_SET(flags, EVENTFLAG_EXECUTING)){
			// flags shouldn't be executing as we are trying to save them
			bugf("save_events_for_entity_chain(): id=%d, %d,%d][%s,%d] %-8s='%s'", 
					id,
					group,
					get_seconds(),
					ent?ent->get_entitytype_text():"ent=NULL",
					ent->vnum(),
					eventtype_to_text(type),
					text);
			return;
		}
		// format of entry is:
		// event group number (0=default), seconds, source_mudprog, flags, type, text
		fprintf(fp, "%d %d %d %s %s %s~\n", 
			group,
			get_seconds(),
			source_mudprog,
			pack_word(flag_string(event_flags, flags)),
			pack_word(flag_string(event_types, type)),
			fix_string(text));
		
	}
}

/**************************************************************************/
/**************************************************************************/
