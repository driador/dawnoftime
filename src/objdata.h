/**************************************************************************/
// objdata.h - header for obj_data class
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

#ifndef obj_data_H
#define obj_data_H

#include "entity.h"

class obj_data : public entity_data
{
public: // override virtual functions in entity_data as appropriate
	entity_type get_entitytype() {return ENTITYTYPE_OBJ;};	
	vn_int vnum();
	void moving_from_ic_to_ooc(); 
	void moving_from_ooc_or_load_to_ic(); 

public:
	int					uid; // unique id
	OBJ_DATA			*next;
	OBJ_DATA			*next_content;
	OBJ_DATA			*contains;
	OBJ_DATA			*in_obj;
	OBJ_DATA			*on;
	char_data			*carried_by;
	EXTRA_DESCR_DATA	*extra_descr;
	AFFECT_DATA			*affected;
	OBJ_INDEX_DATA		*pIndexData;
	ROOM_INDEX_DATA		*in_room;
    OSPEC_FUN			*ospec_fun;
	bool				no_affects; // replacement for enchanted - means more
	bool				chaos;	
	bool				restrung; // true if the object is restrung puts R<vnum> in pfile
	char				*owner;
	char				*short_descr;
	sh_int				item_type;
	int					extra_flags;
	int					extra2_flags;
	int					wear_flags;
	long				lastdrop_id;		// used to detect multilogging
	char *				lastdrop_remote_ip;	// used to detect multilogging
	sh_int				wear_loc;
	sh_int				weight;
	int 				cost;
	sh_int				level;
	sh_int				condition;
	char				*material;
	int					timer;
	int			 		absolute_size;
	sh_int				relative_size;
	int			 		value[5];
	char				*killer;			// used to store killer's short on corpses
	long				dynamic;			// dyn bits, never saved
	long				trap_trig;			// trap trigger type....
    sh_int				trap_dtype;			// damage type trap will inflict
    sh_int				trap_charge;		// amount of charges the trap has, should be kept low
	sh_int				trap_modifier;		// difficulty rating of trap in % to be added to remove trap skill
	long				attune_id;
	long				attune_flags;
	sh_int				attune_modifier;
	time_t				attune_next;
	long				running_otrigger; // set when a particular otrigger is being run on the object
};

#endif

