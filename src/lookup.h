/**************************************************************************/
// lookup.h - prototypes for the majority of lookup commands 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with all the licenses *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************
 * >> Original Diku Mud copyright (c)1990, 1991 by Sebastian Hammer,       *
 *    Michael Seifert, Hans Henrik Staerfeldt, Tom Madsen, & Katja Nyboe.  *
 * >> Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael       *
 *    Chastain, Michael Quan, and Mitchell Tse.                            *
 * >> ROM 2.4 is copyright 1993-1995 Russ Taylor and has been brought to   *
 *    you by the ROM consortium: Russ Taylor(rtaylor@pacinfo.com),         *
 *    Gabrielle Taylor(gtaylor@pacinfo.com) & Brian Moore(rom@rom.efn.org) *
 * >> Oblivion 1.2 is copyright 1996 Wes Wagner                            *
 **************************************************************************/

#ifndef LOOKUP_H
#define LOOKUP_H

int	position_lookup(	const char *name );
int sex_lookup(			const char *name );
int size_lookup(		const char *name );
int race_exact_lookup(	const char *name );
int race_lookup (		const char *name );
int class_exact_lookup (const char *name );
int class_lookup (		const char *name );
int liq_lookup (		const char *name );
int attack_lookup(		const char *name );
int attack_lookup_with_error(const char *name);
int wiznet_lookup(		const char *name );
int sector_lookup(		const char *name );
int season_lookup(		const char *name );
int time_lookup(		const char *name );
int difficulty_lookup(	const char *name );
int mixtype_lookup(		const char *name );
int stat_lookup(		char *stat );
AREA_DATA *area_lookup(	char *name);

continent_type *continent_exact_lookup(const char *name);
continent_type *continent_lookup(	const char *name );


int continent_count(continent_type *cont);

#endif // LOOKUP_H

