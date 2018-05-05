/**************************************************************************/
// duel.cpp - duel combat system implementation - Kal, Dec 99
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "duel.h"

time_t duel_times[6]=
{
	60, //DS_REQUESTED_OF
	60, // DS_WAITING_FOR_REPLY
	15, //DS_ABOUT_TO_START,
	5*60, //DS_BYPASSINGDUEL
	30*60,//DS_DUEL,
	-1 //DS_PROTECTED - remainding time
};

/**************************************************************************/
char *ds_state_name(duel_state st)
{
	switch (st){
	case DS_CHALLENGED_BY: 
		return "challenged by";
	
	case DS_WAITING_FOR_REPLY:
		return "waiting for them to reply";

	case DS_ABOUT_TO_START:
		return "dueling about to activate";

	case DS_DUELING:
		return "dueling";

	case DS_CANT_ATTACK:
		return "can't attack - you can't attack them unless they attack you first";

	case DS_SAFE_FROM_BUT_CAN_ATTACK:
		return "safe - you can attack them, they can't attack you";

	case DS_BYPASSINGDUEL:
		return "bypassing duel";

	default:
		return "BUG - MISSING A STATE IN char *ds_state_name(duel_state st) - PLEASE REPORT!!!";
	}
}
/**************************************************************************/
static char *get_display_pkinfo_time(time_t till)
{
	static char result[MIL];

	int diff=(int)(till-current_time);
	if(diff>15){
		sprintf(result, "%d:%02d", diff/60, diff%60);
	}else{
		sprintf(result, " <15s");
	}
	return result;
}
			  
/**************************************************************************/
// called from in update_handler() on pulse_violence
void duel_update() 
{
	char_data *dch;
    for ( dch = player_list; dch; dch = dch->next_player )
    {
		if(dch->duels){
			dch->duels->update_duels(dch);
		}
	}
}
/**************************************************************************/
// called from in extract_char when someone logs out
void duel_logout(char_data *ch) 
{
	char_data *dch;
    for ( dch = player_list; dch; dch = dch->next_player )
    {
		if(dch->duels){
			dch->duels->logout(dch, ch);
		}
	}
}
/**************************************************************************/
// when someone logs out  - currently players are allowed to logout
//							during a duel, victim is the person logging out
void duel_data::logout(char_data *ch, char_data *victim)
{
	node=find_char(victim);				
	if(node){
		ch->printlnf("`YDuel cancelled with %s because they logged out.`x",
			PERS(victim, ch));
		logf("DUELLOGOUT: %s logged out while active duel with %s (state=%s, time remaining: %s)", 
			victim->name,
			ch->name, 
			ds_state_name(node->state),
			get_display_pkinfo_time(node->till));
		node->state=DS_REMOVE;
		node->till=current_time-1;
		ch->duels->update_duels(ch);
	}
}
/**************************************************************************/
void duel_protect_victim(char_data *victim)
{
	char_data *dch;
	duel_node *node;

    for ( dch = player_list; dch; dch = dch->next_player )
    {
		if(dch->duels){
			node=dch->duels->find_char(victim);
			if(node && node->state==DS_DUELING){
				node->state=DS_CANT_ATTACK;			
				if(victim->duels){
					node=victim->duels->find_char(dch);
				}else{
					node=NULL;
				}
				if(node){
					node->state=DS_SAFE_FROM_BUT_CAN_ATTACK;
				}else{
					bug("duel_protect_victim - failed to do second half!!!");
				}								
			}
		}
	}
	victim->println("Your duels have changed to 'safe' until they expire, you attack or steal.");
};
/**************************************************************************/
void duel_unprotect_victim(char_data *victim)
{
	char_data *dch;
	duel_node *node;
	bool changed=false;

    for ( dch = player_list; dch; dch = dch->next_player )
    {
		if(dch->duels){
			node=dch->duels->find_char(victim);
			if(node && node->state==DS_CANT_ATTACK){
				node->state=DS_DUELING;			
				node=victim->duels->find_char(dch);
				if(node){
					node->state=DS_DUELING;
					changed=true;
				}else{
					bug("duel_unprotect_victim - failed to do second half!!!");
				}
			}
		}
	}
	if(changed){
		victim->println("Your 'safe' duels have all changed to normal.");
	}
};
/**************************************************************************/
void do_acceptduel(char_data *ch, char *argument)
{
	if (IS_OOC(ch)){
		ch->println("You can't perform duel related commands in OOC.");
		return;
	}
	if (ch->fighting){
		ch->println("You can't use this command while fighting.");
		return;
	}

	if(ch->duels){
		ch->duels->accept_duel(ch, argument);
		return;
	}

	ch->println("You have not been challenged to duel recently.");
	return;
};
/**************************************************************************/
void do_declineduel(char_data *ch, char *argument)
{
	if (IS_OOC(ch)){
		ch->println("You can't perform duel related commands in OOC.");
		return;
	}
	if (ch->fighting){
		ch->println("You can't use this command while fighting.");
		return;
	}

	if(ch->duels){
		ch->duels->decline_duel(ch, argument);
		return;
	}

	ch->println("You have not been challenged to duel recently.");
	return;
};

/**************************************************************************/
// used for one player to challenge another to a duel
void do_duel(char_data *ch, char *argument)
{
	char arg[MIL];
	char_data *victim;

	if(GAMESETTING4(GAMESET4_DUEL_SYSTEM_DISABLED)){
		ch->println("The duel system is disabled.");
		return;
	}

	if (IS_OOC(ch)){
		ch->println("You can't perform duel related commands in OOC.");
		return;
	}
	if (ch->fighting){
		ch->println("You can't use this command while fighting.");
		return;
	}

	if(!IS_LETGAINED(ch)){
		ch->println("You can't use this command unless letgained.");
		return;
	}

    one_argument( argument, arg );
	
    if ( IS_NULLSTR(arg))
    {
		ch->println("Duel whom?");
		return;
    }
	
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
		ch->printlnf("You can't find any '%s' in the current room.", arg);
		return;
    }

	if(victim==ch){
		ch->println("You can't duel yourself sorry.");
		return;
	}

	if(IS_NPC(victim)){
		ch->println("You can duel only other players.");
		return;
	}

	if(!can_see(victim,ch)){
		ch->printlnf("%s can't currently see you, you must go visible to duel them.",
			CPERS(victim, ch));
		return;
	}

	if(IS_PEACEFUL(ch) && IS_ACTIVE(victim) && (victim->pcdata->unsafe_due_to_stealing_till<current_time)){
		ch->printf("%s is an active player that hasnt' stolen anything in recent times, therefore you can not duel them.\r\n"
			"Either `=Cbecomeactive`x yourself or use `=Cbypassduel`x.\r\n",
			CPERS(victim, ch));
		return;
	}


	if(IS_ACTIVE(ch) && IS_ACTIVE(victim)){
		ch->printlnf("%s is not a peaceful player, dueling is not required.",
			CPERS(victim, ch));
		return;
	}


	// allocate memory if required
	if(!ch->duels){
		ch->duels=new duel_data();
	}
	if(!victim->duels){
		victim->duels=new duel_data();
	}

	// checks about previous duel information
	if(ch->duels->is_known(victim)){
		if(ch->duels->has_been_challenged_by(victim)){
			ch->printf("%s has already challenged you to a duel,\r\n"
				"Use `=Cacceptduel`x to accept their duel.\r\n"
				"Use `=Cdeclineduel`x to decline their invitation to duel.\r\n",
				CPERS(victim, ch));
			return;
		}else if(ch->duels->is_waiting_for_reply(victim)){
			ch->printf("You have already challenged %s to a duel,\r\n"
				"You must wait longer to give them a chance to respond before challenging them again.\r\n",
				PERS(victim, ch));
			return;
		}else if(!ch->duels->is_bypassingduel(victim)){ // if not bypassing - don't let them go further
			ch->printlnf("You are already involved in some duel related activities with %s.",
				PERS(victim, ch));
			return;
		}
	}

	// setup duel activities on both characters
	ch->duels->update_challenge(victim, DS_WAITING_FOR_REPLY);
	victim->duels->update_challenge(ch, DS_CHALLENGED_BY);
	
	ch->duel_challenged++;

	// do the room/people echos
    act( "***$n has challenged $N to a duel!",  ch, NULL, victim, TO_NOTVICT );
    act( "`W***You challenge $N to a duel!`x",  ch, NULL, victim, TO_CHAR );
    act( "`W***$n has challenged you to a duel!`x", ch, NULL, victim, TO_VICT );
	victim->printf("You may use either `=Cacceptduel`x or `=Cdeclineduel`x to respond...\r\n"
		"(Of course you can always ignore the challenge if you don't wish to respond)\r\n");
}
/**************************************************************************/
// used for one player to bypass the duel system to attack another
void do_bypassduel(char_data *ch, char *argument)
{
	char arg[MIL];
	char_data *victim;

	if(GAMESETTING4(GAMESET4_DUEL_SYSTEM_DISABLED)){
		ch->println("The duel system is disabled.");
		return;
	}

	if (IS_OOC(ch)){
		ch->println("You can't perform duel related commands in OOC.");
		return;
	}

	if (ch->fighting){
		ch->println("You can't use this command while fighting.");
		return;
	}

	if(!IS_LETGAINED(ch)){
		ch->println("You can't use this command unless letgained.");
		return;
	}

    one_argument( argument, arg );
	
    if ( IS_NULLSTR(arg))
    {
		ch->printf("Bypass the dueling system to attack whom?\r\n"
			"`RWARNING: `WIf you kill them you will lose a karn!\r\n"
			"         Every 5 times you subdue someone using this system, you lose a karn!`x\r\n");
		return;
    }
	
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
		ch->printlnf("You can't find any '%s' in the current room.", arg);
		return;
    }

	if(victim==ch){
		ch->println("You can't bypassduel yourself sorry.");
		return;
	}

	if(IS_NPC(victim)){
		ch->println("You can bypassduel only other players.");
		return;
	}

	if(IS_ACTIVE(ch) && IS_ACTIVE(victim)){
		ch->printlnf("You do not need to bypass the duelling system to attack %s.",			
			PERS(victim, ch));
		return;
	}

	// allocate memory if required
	if(!ch->duels){
		ch->duels=new duel_data();
	}

	// checks about previous duel information
	if(ch->duels->is_known(victim)){
		if(ch->duels->has_been_challenged_by(victim)){
			ch->printlnf("%s has challenged you to a duel, either ignore/decline this challenge first.",
				CPERS(victim, ch));
			return;			
		}else if(ch->duels->is_waiting_for_reply(victim)){
			ch->printf("You have already challenged %s to a duel,\r\n"
				"You must wait longer to give them a chance to respond before resorting to ignoring the dueling system.\r\n",
				PERS(victim, ch));
			return;
		}else if(ch->duels->is_bypassingduel(victim)){
			ch->printlnf("You are already bypassing the duel system with %s,",
				PERS(victim, ch));

			duel_node *node=ch->duels->find_char(victim);
			if(node){ // make it possible to increase the ignoreduel timer 
				if(node->till<current_time + 5*60){
					ch->printlnf("%s bypass duel timer increased to 5 minutes.",
						CPERS(victim, ch));
					node->till=current_time + 5*60;
				}
			}
			return;
		}else{
			ch->printlnf("You are already involved in some duel related activities with %s.",
				PERS(victim, ch));
			return;
		}
	}

	// setup duel activities on both characters
	ch->duels->update_challenge(victim, DS_BYPASSINGDUEL);
	ch->duel_bypass++;

	ch->printf("You are now bypassing the duel system in regards to %s.\r\n"
			"`RWARNING: `WIf you kill them you will lose a karn!\r\n"
			"         Every 5 times you subdue someone using the bypass system, you lose a karn!`x\r\n",
		PERS(victim, ch));
}

/**************************************************************************/
void duel_data::display_pkinfo(char_data *ch)
{
	ch->titlebar("DUEL INFO");

	ch->println(" Time - Who(status)");
	for(node=first; node; node=node->next){
		if(node->state!=DS_REMOVE){
			ch->printlnf("%5s - %s(%s)",
				get_display_pkinfo_time(node->till),
				CPERS(node->victim,ch),
				ds_state_name(node->state));
		}
	}	
}
/**************************************************************************/
void duel_data::accept_duel(char_data *ch, char *argument)
{
	char arg[MIL];
	char_data *victim;
    one_argument( argument, arg );

	// check there is only 1 who is currently challenging them
	if(IS_NULLSTR(arg)){
		duel_node *last=NULL;
		int count=0;
		for(node=first; node; node=node->next){
			if(node->state==DS_CHALLENGED_BY){
				count++;
				last=node;
			}
		}
		if(count==0){
			ch->println("There is currently no one challenging you to a duel.");
			return;
		}
		if(count>1){
			ch->printf( "There is more than one person currently challenging you to a duel,\r\n"
				"please type `=Cacceptduel <name>`x to be more specific.\r\n"
				"(checkout pkinfo to see who is challenging you).\r\n");
			return;
		}
		
		{ // single person has challenged - accept them
			node=last;
			victim = get_char_room( ch, node->victim->name);
			if(victim==node->victim){
				act( "***$n has accepted $N's challenge to duel!",  ch, NULL, victim, TO_NOTVICT );
				act( "`W***You accept $N's duel!`x",  ch, NULL, victim, TO_CHAR );
				act( "`W***$n accepts your challenge to duel!`x", ch, NULL, victim, TO_VICT );	
				ch->println("`YPK activities may commence in approximately 15 seconds, and last up to 30 minutes!`x");
				victim->println("`YPK activities may commence in approximately 15 seconds, and last up to 30 minutes!`x");
				
				// update the ch node
				node->state=DS_ABOUT_TO_START;
				node->till=current_time + duel_times[DS_ABOUT_TO_START];				
				// update the victim node
				duel_node *temp=victim->duels->find_char(ch);				
				assert(temp!=NULL);
				temp->state=DS_ABOUT_TO_START;
				temp->till=current_time + duel_times[DS_ABOUT_TO_START];				
			}
		}
	}else{
		victim = get_char_room( ch, arg);
		
		if(!victim){
			ch->printlnf("You can't seem to find any '%s' here.", arg);
			return;
		}
		if(IS_NPC(victim)){ // idiot check
			ch->printlnf("%s is not a player, you can duel only other players.",
				CPERS(victim, ch));
			return;
		}

		node=find_char(victim);				
		if(node){
			if(node->state==DS_CHALLENGED_BY){

				act( "***$n has accepted $N's challenge to duel!",  ch, NULL, victim, TO_NOTVICT );
				act( "`W***You accept $N's duel!`x",  ch, NULL, victim, TO_CHAR );
				act( "`W***$n accepts your challenge to duel!`x", ch, NULL, victim, TO_VICT );	
				ch->println("`YPK activities may commence in approximately 15 seconds, and last up to 30 minutes!`x");
				victim->println("`YPK activities may commence in approximately 15 seconds, and last up to 30 minutes!`x");
				
				// update the ch node
				node->state=DS_ABOUT_TO_START;
				node->till=current_time + duel_times[DS_ABOUT_TO_START];				
				// update the victim node
				duel_node *temp=victim->duels->find_char(ch);				
				assert(temp!=NULL);
				temp->state=DS_ABOUT_TO_START;
				temp->till=current_time + duel_times[DS_ABOUT_TO_START];
				ch->duel_accept++;
			}else{
				ch->printlnf("There is no currently duel CHALLENGE from %s involving you.",
					PERS(victim, ch));
				return;

			}
		}

		ch->printlnf("%s hasn't challenged you to a duel.",
			CPERS(victim, ch));
	}	
} 
/**************************************************************************/
void duel_data::decline_duel(char_data *ch, char *argument)
{
	char arg[MIL];
	char_data *victim;
    one_argument( argument, arg );

	// check there is only 1 who is currently challenging them
	if(IS_NULLSTR(arg)){
		duel_node *last=NULL;
		int count=0;
		for(node=first; node; node=node->next){
			if(node->state==DS_CHALLENGED_BY){
				count++;
				last=node;
			}
		}
		if(count==0){
			ch->println("There is currently no one challenging you to a duel to decline.");
			return;
		}
		if(count>1){
			ch->printf( "There is more than one person currently challenging you to a duel,\r\n"
				"please type `=Cdeclineduel <name>`x to be more specific.\r\n"
				"(checkout pkinfo to see who is challenging you).\r\n");
			return;
		}
		
		{ // single person has challenged - decline them
			node=last;
			victim = get_char_room( ch, node->victim->name);

			if(!victim){
				ch->printlnf("%s doesn't appear to be in the room to decline their duel.",
					CPERS(node->victim, ch));
				return;
			}
			if(victim==node->victim){
				act( "***$n has declined $N's challenge to duel!",  ch, NULL, victim, TO_NOTVICT );
				act( "`W***You decline $N's duel!`x",  ch, NULL, victim, TO_CHAR );
				act( "`W***$n declines your challenge to duel!`x", ch, NULL, victim, TO_VICT );	
				ch->duel_decline++;
				
				node->state=DS_REMOVE;
				node->till=current_time-1;				
				// update the victim node
				if(victim->duels){
					duel_node *temp=victim->duels->find_char(ch);
					if(temp){
						temp->state=DS_REMOVE;
						temp->till=current_time-1;
					}
				}
			}
		}
	}else{
		victim = get_char_room( ch, arg);
		
		if(!victim){
			ch->printlnf("You can't seem to find any '%s' here.", arg);
			return;
		}
		if(IS_NPC(victim)){ // idiot check
			ch->printlnf("%s is not a player, you can duel only other players.",
				CPERS(victim, ch));
			return;
		}

		node=find_char(victim);				
		if(node){
			if(node->state==DS_CHALLENGED_BY){

				act( "***$n has declined $N's challenge to duel!",  ch, NULL, victim, TO_NOTVICT );
				act( "`W***You decline $N's duel!`x",  ch, NULL, victim, TO_CHAR );
				act( "`W***$n declines your challenge to duel!`x", ch, NULL, victim, TO_VICT );			
				ch->duel_decline++;
				
				node->state=DS_REMOVE;
				node->till=current_time-1;				
				// update the victim node
				duel_node *temp=victim->duels->find_char(ch);				
				assert(temp!=NULL);
				temp->state=DS_REMOVE;
				temp->till=current_time-1;
			}else{
				ch->printlnf("There is no currently duel CHALLENGE from %s involving you.",
					PERS(victim, ch));
				return;

			}
		}

		ch->printlnf("%s hasn't challenged you to a duel.",
			CPERS(victim, ch));
	}	
} 
/**************************************************************************/
// destructor - deallocate all memory used 
duel_data::~duel_data()
{
	node=first;
	while(node){
		first=first->next;
		delete node;
		node=first;	
	}
}
/**************************************************************************/
// constructor
duel_data::duel_data()
{
	first=NULL;
}

/**************************************************************************/
// ignore them time has expired
duel_node* duel_data::find_char(char_data *victim)
{
	for(node=first; node; node=node->next){
		if(node->victim==victim){
			return node;
		}
	}
	return NULL;
};
/**************************************************************************/
// updates the duels on the current character
void duel_data::update_duels(char_data *ch)
{
	assert(ch->duels==this);
	assert(!IS_NPC(ch));

	duel_node *prev;
	duel_node *node_next;
	prev=NULL;
	bool remove=false; // if true, the node is removed

	for(node=first; node; node=node_next){
		node_next=node->next;
		if(node->till<current_time){ // a timer has expired
			switch(node->state){
			case DS_CHALLENGED_BY:
				ch->printlnf("`W***You have taken to long to respond to the duel of %s, duel challenge considered ignored.`x",
					PERS(node->victim, ch));
				logf("DUELIGNORE: %s ignored the duel of %s", ch->name, node->victim->name);
				ch->duel_ignore++;
				remove=true;
				break;

			case DS_WAITING_FOR_REPLY:
				ch->printlnf("`W***%s hasn't responded to your challenge to a duel, it appears they have ignored you.`x",
					CPERS(node->victim, ch));
				remove=true;
				break;

			case DS_ABOUT_TO_START:
				node->state=DS_DUELING; // go active, but don't tell them
				node->till=current_time + duel_times[DS_DUELING];
				break;

			case DS_DUELING:
			case DS_SAFE_FROM_BUT_CAN_ATTACK:
			case DS_CANT_ATTACK:
				ch->printlnf("`W***Your duel with %s is over.`x",
					PERS(node->victim, ch));
				remove=true;
				break;
			
			case DS_BYPASSINGDUEL:
				if(ch->fighting){
					ch->printlnf("`R***Your bypassing of the duel system with %s timer has been extended due to combat.`x",
						PERS(node->victim, ch));
					node->till=current_time + 60;
				}else{
					ch->printlnf("`R***Your bypassing of the duel system with %s is over.`x",
						PERS(node->victim, ch));
					remove=true;
				}
				break;

			case DS_REMOVE: // removes a node from the list for any particular reason
				remove=true;
				break;
			
			default:
				bug("Unknown state in duel_data::update_duels(char_data *ch)!!!");
				do_abort();
			}
		}else if (node->state==DS_CHALLENGED_BY 
			&& (IS_ACTIVE(ch) || (ch->pcdata->unsafe_due_to_stealing_till>current_time)))
		{
			// PK players autoaccept if victim is in the same room
			char_data *victim = get_char_room( ch, node->victim->name);
			if(!victim){
				return;
			}
			if(victim==node->victim){
				duel_node *temp;
				act( "***$n has accepted $N's challenge to duel!",  ch, NULL, victim, TO_NOTVICT );
				act( "`W***You accept (automatically) $N's duel!`x",  ch, NULL, victim, TO_CHAR );
				act( "`W***$n accepts your challenge to duel!`x", ch, NULL, victim, TO_VICT );	
				ch->println("`YPK activities may commence in approximately 15 seconds, and last up to 30 minutes!`x");
				victim->println("`YPK activities may commence in approximately 15 seconds, and last up to 30 minutes!`x");
				
				// update the ch node
				node->state=DS_ABOUT_TO_START;
				node->till=current_time + duel_times[DS_ABOUT_TO_START];
				// update the victim node
				temp=victim->duels->find_char(ch);				
				assert(temp!=NULL);
				temp->state=DS_ABOUT_TO_START;
				temp->till=current_time + duel_times[DS_ABOUT_TO_START];				
			}
		}

		// remove the node if necessary
		if(remove){
			if(prev){
				prev->next=node->next;
			}else{
				first=node->next;
			}
			delete node;
		}else{
			prev=node;
		}
	}
	// delete their duels pointer if it holds nothing
	if(!first){
		delete ch->duels;
		ch->duels=NULL;
	}
}; 
/**************************************************************************/
void duel_data::update_challenge(char_data *victim, duel_state to_state)
{
	node=find_char(victim);
		
	if(!node){
		node=new duel_node;
		node->flags=0;
		node->victim=victim;
		// prepend to list of duels because it is new
		node->next=first;
		first=node;
	}
	node->state=to_state;
	node->till=current_time + duel_times[to_state];
}
/**************************************************************************/
// return true if they are previously known 
bool duel_data::is_known(char_data *victim)
{
	node=find_char(victim);
	if(node){
		return true;
	}
	return false;
}
/**************************************************************************/
bool duel_data::has_been_challenged_by(char_data *victim)
{
	node=find_char(victim);
	if(node){
		return (node->state==DS_CHALLENGED_BY);
	}
	return false;
}; 
/**************************************************************************/
bool duel_data::is_waiting_for_reply(char_data *victim)
{
	node=find_char(victim);
	if(node){
		return (node->state==DS_WAITING_FOR_REPLY);
	}
	return false;
}; 
/**************************************************************************/
bool duel_data::is_about_to_start(char_data *victim)
{
	node=find_char(victim);
	if(node){
		return (node->state==DS_ABOUT_TO_START);
	}
	return false;
}; 
/**************************************************************************/
bool duel_data::is_bypassingduel(char_data *victim)
{
	node=find_char(victim);
	if(node){
		return (node->state==DS_BYPASSINGDUEL);
	}
	return false;
}; 
/**************************************************************************/
bool duel_data::is_dueling(char_data* ch, char_data *victim){
	node=find_char(victim);

	if(node){

		// check if someone is losing their 'safeness'
		if(node->state==DS_SAFE_FROM_BUT_CAN_ATTACK){			
			if(victim->duels){
				duel_node *temp=victim->duels->find_char(ch);
				if(temp){
					temp->state=DS_DUELING;
					node->state=DS_DUELING;
				}
			}
		}
		return (node->state==DS_DUELING);
	}
	return false;
}; 
/**************************************************************************/
