/**************************************************************************/
// immquest.h - Quest database, Jarren.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef IMMQUEST_H
#define IMMQUEST_H

/**************************************************************************/
// data structures first
typedef struct	quest_type				QUEST_DATA;

struct quest_type
{
    struct	quest_type * next;
	char *  questname;			// name of quest
    char *	immnames;			// name of responsible imm(s)
	time_t	created_date;		// date quest was created
	time_t	modified_date;		// date quest was modified
	char *	status;				// current state of quest
	char *	resource;			// helpfile resources
	int		immhelp;			// help flags
	char *	synopsis;			// description/synopsis
};
/**************************************************************************/
//prototypes
void load_quest_db( void );
void save_quest_db( void );
QUEST_DATA *quest_lookup( const char *name );
void qedit_showquestinfo( char_data *ch, QUEST_DATA *pQ);

//immquest immhelp types
#define IMMHELP_FREE		1
#define IMMHELP_POSSIBLE	2
#define IMMHELP_CLOSED		3
#define IMMHELP_UNDEFINED	4

// semilocalized globals
extern quest_type *quest_list;
extern sh_int	QUEST_TABLE_FLAGS;

#define IS_RESPONSIBLE(ch, Quest)  \
	( is_exact_name(TRUE_CH(ch)->name, Quest->immnames )  \
	|| is_exact_name("any", Quest->immnames )	\
	|| IS_ADMIN(ch))

#endif // IMMQUEST_H
