/**************************************************************************/
// offmoot.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef OFFMOOT_H
#define OFFMOOT_H

/**************************************************************************/
// data structures first
typedef struct	offmoot_type				OFFMOOT_DATA;

struct offmoot_type
{
  struct	offmoot_type * next;
	char *  name;			// name of player to receive
  int     amount;  // amount of moot
};
/**************************************************************************/
//prototypes
void load_offmoot_db( void );
void save_offmoot_db( void );
void queue_moot(char * name, int amount);
void check_pending_moot(char_data *ch);


// semilocalized globals
extern OFFMOOT_DATA *offmoot_list;
extern sh_int	OFFMOOT_TABLE_FLAGS;


#endif // OFFMOOT_H
