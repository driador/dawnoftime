/**************************************************************************/
// olc.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 ***************************************************************************/
#ifndef OLC_H
#define OLC_H

#include "security.h"
#include "o_lookup.h"
#include "olc_ex.h"

/*
 * New typedefs.
 */
#ifndef DECLARE_OLC_FUN
typedef	bool OLC_FUN		args( ( char_data *ch, char *argument ) );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun
#endif

DECLARE_DO_FUN(    do_help    );
DECLARE_SPELL_FUN( spell_null );
void reboot_autosave( char_data *ch);
void display_race_selection( connection_data *d );
bool generic_ed(char_data *ch, char *argument );
EXTRA_DESCR_DATA * dup_extdescr_list(EXTRA_DESCR_DATA * descript);

char * mprog_type_to_name ( int type );
char * getraceFilename(char *racename);
char * getPCraceFilename(char *racename);
bool swapRacesInTable(int first, int second);
void swap_skills( char **p, char **q );

struct olc_help_type
{
    char *command;
    const void *structure;
    char *desc;
};


// Connected states for OLC editor moved into params.h

/*
 * Interpreter Prototypes
 */
void    aedit           args( ( char_data *ch, char *argument ) );
void    redit           args( ( char_data *ch, char *argument ) );
void    medit           args( ( char_data *ch, char *argument ) );
void    oedit           args( ( char_data *ch, char *argument ) );
void	mpedit			args( ( char_data *ch, char *argument ) );
void	hedit			args( ( char_data *ch, char *argument ) );
void	banedit			args( ( char_data *ch, char *argument ) );
void	npcedit			args( ( char_data *ch, char *argument ) );
void	raceedit		args( ( char_data *ch, char *argument ) );
void	sedit			args( ( char_data *ch, char *argument ) );
void	comedit			args( ( char_data *ch, char *argument ) );
void	dedit			args( ( char_data *ch, char *argument ) );
void	qedit			args( ( char_data *ch, char *argument ) );
void	classedit		args( ( char_data *ch, char *argument ) );
void	gameedit		args( ( char_data *ch, char *argument ) );
void	socialedit		args( ( char_data *ch, char *argument ) );
void	herbedit		args( ( char_data *ch, char *argument ) );
void	mixedit			args( ( char_data *ch, char *argument ) );
void	clanedit		args( ( char_data *ch, char *argument ) );
void	skillgroupedit	args( ( char_data *ch, char *argument ) );
void	langedit		args( ( char_data *ch, char *argument ) );


// OLC Constants
#define MAX_MOB	1	// Default maximum number for resetting mobs 

extern const struct olc_cmd_type banedit_table[];
//extern DEITY_DATA *deity_list;
DEITY_DATA *new_deity( void );

/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */


/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r" \
		"     Port a ROM 2.4 v1.00\n\r"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\n\r" \
                "     Modificado para uso en ROM 2.4b4a\n\r"	\
                "     Por Birdie (itoledo@ramses.centic.utem.cl)\n\r"
#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r" \
		"     (Port a ROM 2.4 - Nov 2, 1996)\n\r" \
		"     Version actual : 1.5a - Mar 9, 1997\n\r"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"



/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_type
{
    const char	*name;
    OLC_FUN		*olc_fun;
	const char *hint;
	bool		hidden;
};


/*
 * Utils.
 */
AREA_DATA *get_vnum_area	args ( ( int vnum ) );
AREA_DATA *get_area_data	args ( ( int vnum ) );
void add_reset			args ( ( ROOM_INDEX_DATA *room, 
				         RESET_DATA *pReset, int index ) );



/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type	aedit_table[];
extern const struct olc_cmd_type	redit_table[];
extern const struct olc_cmd_type	oedit_table[];
extern const struct olc_cmd_type	medit_table[];
extern const struct olc_cmd_type	mpedit_table[];
extern const struct olc_cmd_type	hedit_table[];
extern const struct olc_cmd_type	npcedit_table[];
extern const struct olc_cmd_type	raceedit_table[];
extern const struct olc_cmd_type	sedit_table[];
extern const struct olc_cmd_type	comedit_table[];
extern const struct olc_cmd_type	dedit_table[];
extern const struct olc_cmd_type	qedit_table[];
extern const struct olc_cmd_type	classedit_table[];
extern const struct olc_cmd_type	gameedit_table[];
extern const struct olc_cmd_type	socedit_table[];
extern const struct olc_cmd_type	herbedit_table[];
extern const struct olc_cmd_type	mixedit_table[];
extern const struct olc_cmd_type	clanedit_table[];
extern const struct olc_cmd_type	skillgroupedit_table[];
extern const struct olc_cmd_type	langedit_table[];

/*
 * Editor Commands.
 */
DECLARE_DO_FUN( do_aedit        );
DECLARE_DO_FUN( do_redit        );
DECLARE_DO_FUN( do_oedit        );
DECLARE_DO_FUN( do_medit        );
DECLARE_DO_FUN( do_mpedit		);
DECLARE_DO_FUN( do_npcedit		);
DECLARE_DO_FUN( do_raceedit		);
DECLARE_DO_FUN( do_sedit		);
DECLARE_DO_FUN( do_comedit		);
DECLARE_DO_FUN( do_dedit		);
DECLARE_DO_FUN( do_qedit		);
DECLARE_DO_FUN( do_classedit	);
DECLARE_DO_FUN( do_gameedit		);
DECLARE_DO_FUN( do_clanedit		);
//DECLARE_DO_FUN( do_herbedit		);
DECLARE_DO_FUN( do_langedit		);
/*
 * General Functions
 */
DECLARE_OLC_FUN( show_commands	);
DECLARE_OLC_FUN( show_help		);
bool edit_done			args ( ( char_data *ch ) );
DECLARE_OLC_FUN( show_version	);
void show_char_extended(char_data *, EXTRA_DESCR_DATA *, bool);


// Mudprog editor 
DECLARE_OLC_FUN( mpedit_create );
DECLARE_OLC_FUN( mpedit_code   );
DECLARE_OLC_FUN( mpedit_show   );
DECLARE_OLC_FUN( mpedit_title  );
DECLARE_OLC_FUN( mpedit_mobs   );
DECLARE_OLC_FUN( mpedit_author );
DECLARE_OLC_FUN( mpedit_copy);
DECLARE_OLC_FUN( mpedit_mpcopy );
DECLARE_OLC_FUN( mpedit_delete);
DECLARE_OLC_FUN( mpedit_mpdelete);
DECLARE_OLC_FUN( mpedit_indent	);
DECLARE_OLC_FUN( mpedit_autoindent );


// macros moved into merc.h
/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)		( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)		( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)		( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)		( Area = (AREA_DATA *)Ch->desc->pEdit )
#define EDIT_MPCODE(Ch, Code)   ( Code = (MUDPROG_CODE*)Ch->desc->pEdit )
#define EDIT_HELP(Ch, Help)		( Help = (help_data*)Ch->desc->pEdit )
#define EDIT_RACE(Ch, Race)		( Race = (race_data*)Ch->desc->pEdit )
#define EDIT_SPELLSKILL(Ch, spsk) ( spsk= (skill_type*)Ch->desc->pEdit )
#define EDIT_COMMAND(Ch, cmd)	( cmd= (cmd_type*)Ch->desc->pEdit )
#define EDIT_DEITY(Ch, Deity)	( Deity = (DEITY_DATA *)Ch->desc->pEdit )
#define EDIT_BAN(ch, ban)		( ban = (BAN_DATA*)ch->desc->pEdit )
#define EDIT_QUEST(ch, quest)	( quest = (quest_type*)ch->desc->pEdit )
#define EDIT_CLASS(Ch, Class)	( Class= (class_type*)Ch->desc->pEdit )
#define EDIT_HERB(ch, herb)		( herb= ( HERB_DATA *)ch->desc->pEdit )
#define EDIT_MIX(ch, mix)		( mix= (mix_data *)ch->desc->pEdit )
#define EDIT_CLAN(ch, clan)		( clan= (CClanType*)ch->desc->pEdit )
#define EDIT_SKILLGROUP(ch, skgrp)	( skgrp= (skillgroup_type*)ch->desc->pEdit )
#define EDIT_LANGUAGE(ch, lang)	( lang= (language_data*)ch->desc->pEdit )
/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA	*new_reset_data		args ( ( void ) );
void		free_reset_data		args ( ( RESET_DATA *pReset ) );
AREA_DATA	*new_area		args ( ( void ) );
void		free_area		args ( ( AREA_DATA *pArea ) );
EXIT_DATA	*new_exit		args ( ( void ) );
void		free_exit		args ( ( EXIT_DATA *pExit ) );
ED 		*new_extra_descr	args ( ( void ) );
void		free_extra_descr	args ( ( ED *pExtra ) );
ROOM_INDEX_DATA *new_room_index		args ( ( void ) );
void		free_room_index		args ( ( ROOM_INDEX_DATA *pRoom ) );
AFFECT_DATA	*new_affect		args ( ( void ) );
void		free_affect		args ( ( AFFECT_DATA* pAf ) );
SHOP_DATA	*new_shop		args ( ( void ) );
void		free_shop		args ( ( SHOP_DATA *pShop ) );
OBJ_INDEX_DATA	*new_obj_index		args ( ( void ) );
void		free_obj_index		args ( ( OBJ_INDEX_DATA *pObj ) );
MOB_INDEX_DATA	*new_mob_index		args ( ( void ) );
void		free_mob_index		args ( ( MOB_INDEX_DATA *pMob ) );
#undef	ED

void		show_liqlist		args ( ( char_data *ch ) );
void		show_damlist		args ( ( char_data *ch ) );

char *		mprog_type_to_name	args ( ( int type ) );
MUDPROG_TRIGGER_LIST      *new_mprog              args ( ( void ) );
void            free_mprogs             args ( ( MUDPROG_TRIGGER_LIST *mp ) );
MUDPROG_CODE	*new_mpcode		args ( (void) );
void		free_mpcode		args ( ( MUDPROG_CODE *pMcode));

void show_flag_cmds( char_data *ch, const struct flag_type *flag_table );

void show_olc_options(char_data *ch, const struct flag_type *flag_table, 
					  const char *command_name, const char *descript, long flags);
bool run_olc_editor_for_connection(connection_data *c, char *command);

#endif // OLC_H
