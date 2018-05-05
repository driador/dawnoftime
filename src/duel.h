/**************************************************************************/
// duel.h - duel combat system header - Kal, Dec 99
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef DUEL_H
#define DUEL_H

#include "include.h"

enum duel_state{
	DS_CHALLENGED_BY,// person being asked to duel
	DS_WAITING_FOR_REPLY,	// person asking
	DS_ABOUT_TO_START,
	DS_BYPASSINGDUEL,	// used to allow people to attack without needing to duel first
	DS_DUELING,
	DS_CANT_ATTACK,	
	DS_SAFE_FROM_BUT_CAN_ATTACK,
	DS_REMOVE  // easy way to handle all removal from the linked list
};

// duel flags, not in dawn.h so adding a new flag doesn't require a 
// complete recompile
#define DUEL_TODEATH		(A)
#define DUEL_NO_LOGOUT		(B)

struct duel_node
{
	char_data	* victim;	// the other person involved in the dueling process
	long		flags;		// duel flags
	time_t		till;		// when the duel will last till
	duel_state	state;
	duel_node	*next;
};

void duel_protect_victim(char_data *victim); // sets everyone's duel on this character to protected

class duel_data
{
public:
	// state questions
	bool is_known(char_data *victim); // true if they are known - find_char() knows about them
	bool has_been_challenged_by(char_data *victim); 
	bool is_waiting_for_reply(char_data *victim); 
	bool is_about_to_start(char_data *victim); 
	bool is_active(char_data *victim); 
	bool is_dueling(char_data* ch, char_data *victim); 
	bool is_bypassingduel(char_data *victim); 

	void logout(char_data *ch, char_data *victim);  // when someone logs out

	void accept_duel(char_data *ch, char *argument); 
	void decline_duel(char_data *ch, char *argument); 
	void display_pkinfo(char_data *ch); 
										
	duel_data();	// constructor
	~duel_data();	// destructor

	void update_duels(char_data *ch); // updates the duels on the current character
	void update_challenge(char_data *victim, duel_state to_state); // adds a new node if required
	duel_node	*find_char(char_data *victim); // if the char is in the list of nodes, it is returned
private:
	duel_node	*first; 
	duel_node	*node; // workhorse node - used for all the loops etc
};

#endif
/**************************************************************************/
