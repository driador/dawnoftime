/**************************************************************************/
// offmoot.cpp - Offline mooting, Jarren
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "include.h"
#include "offmoot.h"

/**************************************************************************/
// semilocalized globals
OFFMOOT_DATA	*offmoot_list;
sh_int	OFFMOOT_TABLE_FLAGS;

/**************************************************************************/
// create offmoot GIO lookup table 
GIO_START(OFFMOOT_DATA)
GIO_STRH(name,		"Name   ")
GIO_INTH(amount,	"Amount ")
GIO_FINISH_STRDUP_EMPTY
/**************************************************************************/
// loads in the offmoot database
void load_offmoot_db(void)
{
	logf("===Loading offmoot database from %s...", OFFMOOT_FILE);
	GIOLOAD_LIST(offmoot_list, OFFMOOT_DATA, OFFMOOT_FILE); 	
	log_string ("load_offmoot_db(): finished");
}
/**************************************************************************/
// saves the offmoot database
void save_offmoot_db( void)
{
	logf("===save_offmoot_db(): saving offmoot database to %s", OFFMOOT_FILE);
	GIOSAVE_LIST(offmoot_list, OFFMOOT_DATA, OFFMOOT_FILE, true);
}
/**************************************************************************/
// lists offmoots
void do_listmoot( char_data *ch, char *)
{
	OFFMOOT_DATA *node;
	int count;

	ch->titlebar("-=PENDING MOOTS=-");

	count=0;
	for (node = offmoot_list; node; node= node->next)
	{
  		ch->printlnf("`s%d> `MPlayer: `m%-16s  `YAmount: `y%d",
	  				++count, node->name, node->amount);
	}

	ch->println("\r\n`xAdmin type delmoot <moot_number> do dequeue a pending moot.");
	ch->println("        or addmoot <player_name> <amount> to requeue a pending moot.");
}
/**************************************************************************************/
void do_delmoot( char_data *ch, char *argument )
{
	OFFMOOT_DATA *node;
	OFFMOOT_DATA *prevnode=NULL;
	int target, count=1;
	
	if(offmoot_list==NULL){
		ch->println("There are no pending moots.");
		return;
	}
	
	if(IS_NULLSTR(argument)){
		ch->println("syntax: delmoot <moot_number>");
		return;
	};
	
	if(!is_number(argument)){
		ch->print("Argument to delmoot must be a number.");
		return;
	}
	
	target=atoi(argument);
	
	for (node=offmoot_list;node;node=node->next, ++count){
		if(count==target)
			break;
		prevnode = node;
	}
	
	if(node==NULL){
		ch->printf(0, "Pending moot not found.");
		return;
	}
	
	ch->printf(0, "Moot #%d against %s for %d exp has been dequeued.\r\n",
		target, node->name, node->amount);
	
	free_string(node->name);
	
	if(!prevnode){ // delete the head
		offmoot_list = offmoot_list->next;
	}else{
		prevnode->next=node->next;
	}
	
	delete node;
	save_offmoot_db();
}

/**************************************************************************************/
void do_addmoot(char_data *ch, char *argument)
{
	char arg1[MIL], arg2[MIL];
	int amount;
	
	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);
	
	if(!is_number(arg2)){
		ch->print("syntax: addmoot <player_name> <amount>.");
		return;
	}
	
	amount=atoi(arg2);
	
	queue_moot(arg1, amount);
	
	ch->printf(0, "Moot against %s for %d has been queued.", arg1, amount);
	
}

/**************************************************************************************/
void queue_moot(char * name, int amount){
	
	OFFMOOT_DATA *node = new OFFMOOT_DATA;
	node->name=str_dup(name);
	node->amount = amount;
	node->next=offmoot_list;
	offmoot_list=node;
	save_offmoot_db();
}

/**************************************************************************************/
void check_pending_moot(char_data *ch){
	
	OFFMOOT_DATA *node=offmoot_list, *prevnode=NULL;
	bool matched=false;
	
	while(node){		
		//only award the moot if there is no moot currently in progress
		if(!str_cmp(node->name, ch->name) && moot->moot_type<1){
			
			//setup the moot
			//special note: called_by and moot_victim are intentionally
			//set to be the same as a flag so resolve_moot will know only
			//to reward the player and not mess with noble's diplomacy.
			moot->called_by=TRUE_CH(ch);
			moot->moot_victim=TRUE_CH(ch);
			moot->moot_victim_name=str_dup(node->name);
			moot->moot_type=1;
			moot->scope=node->amount;
			moot->votes_for=1;
					
			//use exsisting code to resolve it
			resolve_moot();
			
			moot->called_by=NULL;
			moot->moot_victim=NULL;
			free_string(moot->moot_victim_name);
			moot->moot_type=0;
			moot->scope=0;
			moot->votes_for=0;
			
			//get rid of the moot once it has been called...
			free_string(node->name);
			
			if(!prevnode){ // delete the head
				offmoot_list = offmoot_list->next;
			}else{
				prevnode->next=node->next;
			}	
			delete node;
			matched=true;
			
			if(!prevnode){
				node=offmoot_list;
			}else{
				node=prevnode;
			}			
			continue;			
		}		
		prevnode=node;
		node=node->next;
	}

	if(matched){
		save_offmoot_db();
	}
	
}
/**************************************************************************************/
