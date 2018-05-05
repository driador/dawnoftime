/**************************************************************************/
// db.h - 
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

extern bool	fBootDb;
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern int top_mob_index;
extern int top_obj_index;
extern int top_affect;
extern int top_ed; 
extern AREA_DATA *area_first;
extern AFFECT_DATA *affect_free;
extern DEITY_DATA *deity_first;
extern HERB_DATA *herb_list;
extern mix_data *mix_list;

// db.c 
extern void assign_area_vnum( int vnum );

// db2.c 
extern int	social_count;

// macro for flag swapping 
#define GET_UNSET(flag1,flag2)	(~(flag1)&((flag1)|(flag2)))

// Magic number for memory allocation 
#define MAGIC_NUM 52571214

// obdb.cpp
void load_object_values( FILE *fp, int /*version*/, OBJ_INDEX_DATA *pObjIndex);

// olc_save.cpp
void save_object_values( FILE *fp, OBJ_INDEX_DATA *pObjIndex ); 

