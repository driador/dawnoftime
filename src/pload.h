/**************************************************************************/
// pload.h - dawn player loading/unloading system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef PLOAD_H
#define PLOAD_H

void pload_extract(char_data *ch, char_data *victim);
char_data *pload_find_player_by_name(char *name);

struct pload_data
{
	bool dont_save; // if true, the pfile wont be saved
	char_data *loaded_by; // the imm that loaded the player file	
};

#endif // PLOAD_H

