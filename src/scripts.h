/**************************************************************************/
// scripts.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef SCRIPTS_H
#define SCRIPTS_H

typedef struct script_data SCRIPT_DATA;

struct script_data
{
	script_data *next;
	char *script_name;
	char *auth_users;
};

void load_script_db( void );

#endif // SCRIPTS_H
