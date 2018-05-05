/**************************************************************************/
// recycle.h - 
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
#ifndef RECYCLE_H
#define RECYCLE_H

#include "ban.h"

// externs 
extern char str_empty[1];
extern int mobile_count;

// stuff for providing a crash-proof buffer 
#define MAX_BUF		65536
#define MAX_BUF_LIST 	12
#define BASE_BUF 	1024

// valid states 
#define BUFFER_SAFE	0
#define BUFFER_OVERFLOW	1
#define BUFFER_FREED 	2

// note recycling 
#define ND NOTE_DATA
ND	*new_note args( (void) );
void	free_note args( (NOTE_DATA *note) );
#undef ND

// ban data recycling 
#define BD BAN_DATA
BD	*new_ban args( (void) );
void	free_ban args( (BAN_DATA *ban) );
#undef BD

// descriptor recycling 
#define DD connection_data
DD  *connection_allocate args( (void) );
void	connection_deallocate args( (connection_data *d) );
#undef DD

// char gen data recycling 
#define GD GEN_DATA
GD 	*new_gen_data args( (void) );
void	free_gen_data args( (GEN_DATA * gen) );
#undef GD

// extra descr recycling 
#define ED EXTRA_DESCR_DATA
ED	*new_extra_descr args( (void) );
void	free_extra_descr args( (EXTRA_DESCR_DATA *ed) );
#undef ED

// affect recycling 
#define AD AFFECT_DATA
AD	*new_affect args( (void) );
void	free_affect args( (AFFECT_DATA *af) );
#undef AD

// object recycling 
#define OD OBJ_DATA
OD	*new_obj args( (void) );
void	free_obj args( (OBJ_DATA *obj) );
#undef OD

// character recyling 
#define CD char_data
#define PD PC_DATA
CD	*new_char args( (void) );
void	free_char args( (char_data *ch) );
PD	*new_pcdata args( (void) );
void	free_pcdata args( (PC_DATA *pcdata) );
#undef PD
#undef CD


// mob id and memory procedures 
#define MD MEM_DATA
long 	get_pc_id args( (void) );
void	free_mem_data args( ( MEM_DATA *memory) );
MD	*find_memory args( (MEM_DATA *memory, long id) );
#undef MD

// buffer procedures 
BUFFER	*new_buf args( (void) );
void	free_buf args( (BUFFER *buffer) );
bool	add_buf args( (BUFFER *buffer, const char *string) );
void	clear_buf args( (BUFFER *buffer) );
char	*buf_string args( (BUFFER *buffer) );


room_echo_data *new_room_echo( void );
void			free_room_echo( room_echo_data *);

// Area Echoes
area_echo_data *new_area_echo( void );
void			free_area_echo( area_echo_data *);

#endif // RECYCLE_H

