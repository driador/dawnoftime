/**************************************************************************/
// shop.cpp - shop functionality written by Slortar
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************/

#include "shop.h"

/**************************************************************************/
/*
 * The default constructor. Don't forget to initliase new member variables
 * here.
 * \return Nothing.
 * \param None.
 */
cShopData::cShopData()
{
	close_hour		= 23;
	open_hour		= 0;
	profit_buy		= 100;
	profit_sell		= 100;
	vnKeeper		= 0;
}

/**************************************************************************/
/*
 * The default destructor.
 * \return Nothing.
 * \param None.
 */
cShopData::~cShopData()
{

}

/**************************************************************************/
/*
 * The default constructor. Don't forget to initliase new member variables
 * here.
 * \return Nothing.
 * \param None.
 */
cInnData::cInnData()
{
	pNextInn		= NULL;
	for(int i=0; i<MAX_INN; i++){
		vnRoom[i]=0;
		shRate[i]=0;
	}
}

/**************************************************************************/
/*
 * The default destructor.
 * \return Nothing.
 * \param None.
 */
cInnData::~cInnData()
{

}

/**************************************************************************/
/* Returns the first NPC in the same room as *ch that is an innkeeper. If there
 * is an innkeeper in the room, but he's unavailable for action. Closing hours,
 * unable to see *ch, then NULL is returned.
 * \return The pointer to the char_data structure of a NPC innkeeper.
 * \param ch Pointer to the char_data structure of a player.
 */
char_data* find_innkeeper(char_data *ch)
{
	cInnData* pInn		= NULL;
	char_data* pKeeper	= NULL;
	
	for ( pKeeper=ch->in_room->people; pKeeper; pKeeper=pKeeper->next_in_room ) {
		if ( IS_NPC(pKeeper) && (pInn=pKeeper->pIndexData->pInnData) != NULL ) {
			break;
		}
	}

	// No innkeeper in the room.
	if ( pInn == NULL ) {
		ch->println("You can't do that here.");
		return NULL;
    }

    // Shop hours.
    if ( time_info.hour < pInn->open_hour )
    {
		do_say( pKeeper, "Sorry, I am closed. Come back later." );
		return NULL;
    }
	if ( time_info.hour > pInn->close_hour )
    {
		do_say( pKeeper, "Sorry, I am closed. Come back tomorrow." );
		return NULL;
    }

    // Invisible or hidden people.
	if ( !can_see( pKeeper, ch )  && INVIS_LEVEL(ch)<1)
    {
		do_say( pKeeper, "I don't rent to folks I can't see." );
		return NULL;
	}
	
    return pKeeper;
}

/**************************************************************************/
/*
 * Allows players to rent a room. The function makes the necesary checks on 
 * the room and the player etc. Then the function parses the argument which
 * should represent the number of hours the player wishes to rent the room.
 * Renting a room will set the player's first point of recall to this room,
 * for a certain amount of hours.
 * \return Nothing.
 * \param ch The pointer to the player's char_data structure.
 * \param aArg The string containing the input the player supplied.
 */
void do_rent( char_data *ch, char *aArg)
{
	char_data* pInnKeeper			= NULL;
	cInnData* pInn					= NULL;
	ROOM_INDEX_DATA* pRoom			= NULL;
	const double uiTicksPerGameHour	= 11.4;
	unsigned int uiNumHours			= 0;
	vn_int roomVnum					= 0;
	long roomRate					= 0;
	int roomCost					= 0;
	int roomHaggle					= 0;
	sh_int roomIdx					= 0;
	char arg1[MIL];				
	char arg2[MIL];

	aArg = one_argument(aArg, arg1);
	aArg = one_argument(aArg, arg2);

	// Check the room is an inn.
	if ( (ch->in_room != NULL) && (!IS_SET(ch->in_room->room_flags, ROOM_INN)) ) {
		ch->println("Perhaps try renting a room at an inn.");
		if(IS_IMMORTAL(ch) || HAS_SECURITY(ch,1)){
			ch->wrapln("[imm/builder only message] A room must have the INN flag set.");
		}
		return;
	}

	// Find the innkeeper.
	if ( (pInnKeeper=find_innkeeper(ch)) == NULL ) {
		ch->println("The innkeeper seems to have run off.");
		if(IS_IMMORTAL(ch) || HAS_SECURITY(ch,1)){
			ch->wrapln("[imm/builder only message] This room is marked as an inn, but a mob with an inn assigned "
				"was not found (or outside the hours for their inn).");
		}
		return;
	}

	// Check arguments.
	if ( !is_number(arg1) || !is_number(arg2) ) {
		ch->println("Syntax: rent <room number> <number of hours>");
		return;
	}

	roomIdx			= (sh_int)atoi(arg1);
	if(roomIdx<1 || roomIdx>MAX_INN){
		ch->println("Syntax: rent <room number> <number of hours>");
		ch->printlnf("The room number must be between 1 and %d", MAX_INN);		
		return;
	}

	uiNumHours		= (sh_int)atoi(arg2);
	pInn			= pInnKeeper->pIndexData->pInnData;

	// Get the relevant room details.
	roomVnum = pInn->vnRoom[roomIdx-1]; 
	roomRate = pInn->shRate[roomIdx-1]; 

	// Check the room is a valid room.
	if ( (pRoom=get_room_index(roomVnum))==NULL ) {
		ch->println("That room is not available.");
		return;
	}

	// Calculate the cost and check the cost will be positive.
	if ( (roomCost=(roomRate*uiNumHours)) <= 0 ) {
		ch->println("You can't rent a room for that amount of hours.");
		return;
	}

	// haggle ( code taken from do_buy the petshop bit, made a few alterations. )
	roomHaggle = number_percent();
	if (roomHaggle < get_skill(ch,gsn_haggle)) {
		// Haggle succeeded.

		roomCost -= roomCost / 2 * roomHaggle / 100;
		if ( (ch->silver + 100 * ch->gold) < roomCost)
		{				
			ch->printlnf( "You haggle the price down to %d coin%s, but "
				"still can't afford it.", roomCost, roomCost==1?"":"s");
			
			if ( (ch->silver + 100 * ch->gold) > (roomCost * 96/100) )
			{				
				roomCost=(ch->silver + 100 * ch->gold);
				ch->printlnf( "You explain this to the innkeeper and manage to "
					"haggle the price down to the money that you do have!");
			} else { // didn't get it
				WAIT_STATE(ch, 20); // 5 seconds lag to reduce potential abuse
				return;
			}
		} else {
			ch->printlnf( "You haggle the price down to %d coin%s.",
				roomCost, roomCost==1?"":"s");
			check_improve(ch, gsn_haggle, true, 4);
		}		
	}else{
		// Haggle failed.
		if ( (ch->silver + 100 * ch->gold) < roomCost ) {
			ch->println("You can't afford it.");
			check_improve(ch, gsn_haggle, false, 2);

			return;
		} else {
			ch->printlnf( "You successfully rented a room for %d coin%s.",
				roomCost, roomCost==1?"":"s");
		}
	}

	deduct_cost(ch, roomCost);

	// If you're renting the same room, then just add the extra days to the end of your 
	// current period. Rounded downwards.
	if ( ch->recall_inn_room == roomVnum && ch->expire_recall_inn>0 ) {
		uiNumHours+=(ch->expire_recall_inn / (int)uiTicksPerGameHour);
	} else {
		ch->recall_inn_room = roomVnum;
	}
	
	ch->expire_recall_inn=(int)(uiNumHours*uiTicksPerGameHour);

	ch->printlnf("You will recall to `#%s`^ for the following %d hour%s.", pRoom->name, uiNumHours, uiNumHours==1 ? "" : "s");
	ch->println("Type `#`=Crecall reset`^ to cancel this and recall to your default location.");

    return;
}


/**************************************************************************/
/**************************************************************************/

