/**************************************************************************/
// tables.h - 
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
#ifndef TABLES_H
#define TABLES_H

#include "clan.h"

extern	struct classgroup_type classgroup_table[31];
extern	struct affectprofile_type affectprofile_table[50];

extern			struct	flag_type		classnames_flags[MAX_CLASS+1];

extern	const	struct	flag_type		ac_types[];
extern	const	struct	flag_type		act_flags[];
extern	const	struct	flag_type		act2_flags[];
extern	const	struct	flag_type		affect_flags[];
extern	const	struct	flag_type		affect2_flags[];
extern	const	struct	flag_type		affect3_flags[];
extern	const	struct	flag_type		align_flags[];
extern	const	struct	flag_type		apply_types[];
extern	const	struct	flag_type		area_import_format_types[];
extern	const	struct	flag_type		to_types[];
extern	const	struct	flag_type		area_flags[];
extern	const	struct	flag_type		attune_flags[];
extern	const	struct	flag_type		ban_types[];
extern	const	struct	flag_type		buildrestrict_types[];
extern	const	struct	flag_type		castcommand_types[];
extern	const	struct	flag_type		castnames_types[];
extern	const	struct	flag_type		category_types[];
extern	const	struct	flag_type		classflag_flags[];
extern	const	struct	flag_type		colourmode_types[];
extern	const	struct	flag_type		com_category_types[];
extern	const	struct	flag_type		comm_flags[];
extern	const	struct	flag_type		commandflag_flags[];
extern	const	struct	flag_type		commandlog_types[];
extern	const	struct	flag_type		composition_flags[];
extern	const	struct	flag_type		config_flags[]; 
extern	const	struct	flag_type		config2_flags[]; 
extern	const	struct	flag_type		container_flags[];
extern	const	struct	flag_type		council_flags[];
extern	const	struct	flag_type		damtype_types[];
extern	const	struct	flag_type		dedit_flags[];
extern	const	struct	flag_type		direction_types[];
extern	const	struct	flag_type		dynspell_flags[];
extern	const	struct	flag_type		element_flags[];
extern	const	struct	flag_type		exit_flags[];
extern	const	struct	flag_type		event_types[]; // stored in events.cpp
extern	const	struct	flag_type		event_flags[]; // stored in events.cpp
extern	const	struct	flag_type		form_flags[];
extern	const	struct	flag_type		furniture_flags[];
extern	const	struct	flag_type		grantgroup_flags[];
extern	const	struct	flag_type		help_flags[];
extern			struct	flag_type		help_category_types[MAX_HELP_CATEGORIES];
extern	const	struct	flag_type		imm_flags[];
extern	const	struct	flag_type		immhelp_types[];
extern	const	struct	flag_type		language_flags[];
extern	const	struct	flag_type		laston_flags[];
extern	const	struct	flag_type		laston_wizlist_types[];
extern	const	struct	flag_type		letgain_db_flags[];
extern	const	struct	flag_type		liquid_flags[];
extern	const	struct	flag_type		mixtype_types[];
extern	const	struct	flag_type		mixtype_type[];
extern	const	struct	flag_type		mprog_flags[];
extern	const	struct	flag_type		mprog2_flags[];
extern	const	struct	flag_type		notenet_flags[]; // to save notenet info
extern	const	struct	flag_type		objextra_flags[];
extern	const	struct	flag_type		objextra2_flags[];
extern	const	struct	flag_type		objspell_flags[];
extern	const	struct	flag_type		off_flags[];
extern	const	struct	flag_type		olc_flags[];
extern	const	struct	flag_type		oprog_flags[];
extern	const	struct	flag_type		oprog2_flags[];
extern	const	struct	flag_type		part_flags[];
extern	const	struct	flag_type		pconfig_flags[]; 
extern	const	struct	flag_type		plr_flags[];
extern	const	struct	flag_type		portal_flags[];
extern	const	struct	flag_type		position_flags[];
extern	const	struct	flag_type		position_types[];
extern	const	struct	flag_type		preference_types[];
extern	const	struct	flag_type		realm_flags[];
extern	const	struct	flag_type		res_flags[];
extern	const	struct	flag_type		room_flags[];
extern	const	struct	flag_type		room2_flags[];
extern	const	struct	flag_type		rprog_flags[];
extern	const	struct	flag_type		rprog2_flags[];
extern	const	struct	flag_type		sector_desc[];
extern	const	struct	flag_type		sector_types[];
extern	const	struct	flag_type		sectorbit_flags[];
extern	const	struct	flag_type		sex_types[];
extern	const	struct	flag_type		size_types[];
extern	const	struct	flag_type		skflags_flags[];
extern	const	struct	flag_type		sktype_types[];
extern	const	struct	flag_type		spell_group_flags[];
extern	const	struct	flag_type		sphere_flags[];
extern	const	struct	flag_type		stat_flags[];
extern	const	struct	flag_type		target_types[];
extern	const	struct	flag_type		tendency_flags[];
extern	const	struct	flag_type		textsearch_types[];
extern	const	struct	flag_type		token_flags[];
extern	const	struct	flag_type		traptype_flags[];
extern	const	struct	flag_type		item_types[];
extern	const	struct	flag_type		vuln_flags[];
extern	const	struct	flag_type		weapon_class_types[];
extern	const	struct	flag_type		weapon_flags[];
extern	const	struct	flag_type		wear_flags[];
extern	const	struct	flag_type		wear_location_types[];
extern	const	struct	flag_type		wear_location_strings_types[];
extern	const	struct	flag_type		wiznet_flags[];

extern	const	struct	gamble_type 	gamble_table[];
extern	const	struct	position_type	position_table[];
extern	const	struct	sector_type 	sect_table[];
extern	const	struct	sex_type		sex_table[];
extern	const	struct	size_type		size_table[];

#endif // TABLES_H

