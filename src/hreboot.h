/**************************************************************************/
// hreboot.cpp - Online hotreboot, 100% by Kalahn
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef HREBOOT_H
#define HREBOOT_H
/**************************************************************************/
typedef unsigned long ptr_val;	// ptr_val is a pointer as a numeric value
								// used to make the code portable
/**************************************************************************/
// extern globals
extern bool hotreboot_in_progress;
extern int	hotreboot_ipc_pipe;

// function prototypes
void hotreboot_process_parent_side_progress();
void hotreboot_reassign_child_pipe(int val);
void hotreboot_init_receive();
void hotreboot_game_environment_transfer();

enum datatypes{
		DT_END,
		DT_INT, 
		DT_LONG,
		DT_SHORT, 
		DT_BOOL, 
		DT_STR,
		DT_CHARDATA, // Sent as ID
		};

enum hotreboot_where{
	HRW_END,
	HRW_GLOBAL_VARIABLE,
	HRW_LAST_LISTENON,
	HRW_LASTCHAR,
	HRW_LASTCHAR_NOTE,
	HRW_LASTPCDATA,
	HRW_LASTCONNECTION,
	HRW_FINDCHARACTER,			// by char id	
	HRW_FINDCHAR_PCDATA,		// by char id	
	HRW_FINDCHAR_DESCRIPTOR		// by char id		
	};

//      HOTREBOOTFLAGs
#define HRF_INIT_SEND				(A)
#define HRF_CONTROL_VAR				(B)
#define HRF_GAME_STAT_VALUE			(C)
#define HRF_SEND_AS_CHARFIELD		(D)
#define HRF_NEXT_PARAMETER_CHARID	(E)
#define HRF_CHECK_NUMERIC			(F)
#define HRF_NOTE_IN_PROGRESS		(G)

struct hotreboot_field_table_type // used to set charvariables etc 
{
	char *name;
	ptr_val	offset;
	datatypes datatype;
	hotreboot_where where;
	int flags;
};
/**************************************************************************/
#endif // HREBOOT_H
/**************************************************************************/

