/**************************************************************************/
// dynamics.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef DYNAMICS_H
#define DYNAMICS_H

/**************************************************************************/
int create_class(char * class_name);
void do_write_classes(char_data *ch, char *);
/**************************************************************************/
struct dynlookup_type{
	char *name;
	sh_int dyn_sktype;
	sh_int * pgsn;
};
/**************************************************************************/
struct dynspell_type{
	int flags;
	char *name;
	SPELL_FUN *psp;
	sh_int target;
	sh_int count;
};
/**************************************************************************/

#endif // DYNAMICS_H

