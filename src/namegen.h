/**************************************************************************/
// namegen.h - random name generator, Kalahn - Jan 99
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef NAMEGEN_H
#define NAMEGEN_H
 
#define MAX_PARTS 3
struct name_profile{
	char *title;
	char *part[MAX_PARTS];		// the parts making up a name
	short part_count[MAX_PARTS];
	name_profile *next;
};

extern name_profile * name_profiles_list;
char * genname(name_profile * profile);
extern int profile_count;

#endif // NAMEGEN_H

