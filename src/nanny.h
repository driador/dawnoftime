/**************************************************************************/
// nanny.h - see below
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
/***************************************************************************
 *  FILE: nanny.h  - all creation related defines etc                      *
 *  USED BY: nanny.cpp, comm.cpp, act_wiz.cpp                              *
 ***************************************************************************/
#ifndef NANNY_H
#define NANNY_H

/*
* Connected state for a channel.
*/
//#define CON_PLAYING                 0
// CON_PLAYING is defined in dawn.h because it is referenced by many 
// code modules.
#define CON_DETECT_CLIENT_SETTINGS	35
#define CON_GET_NAME                1
#define CON_GET_OLD_PASSWORD        2
#define CON_CONFIRM_CREATING_NEW	3

#define CON_NAME_SELECT				30
#define CON_GET_CREATION_PASSWORD   31
#define CON_CONFIRM_NEW_NAME		32
#define CON_GET_CONNECT_PASSWORD	33
#define CON_GET_CONNECT_PASS2CREATE	34

#define CON_GET_NEW_PASSWORD        4
#define CON_CONFIRM_NEW_PASSWORD    5
#define CON_GET_NEW_RACE            6
#define CON_GET_NEW_SEX             7
#define CON_GET_NEW_CLASS           8
#define CON_GET_ALIGNMENT           9
#define CON_DEFAULT_CHOICE          10
#define CON_GEN_GROUPS              11
#define CON_PICK_WEAPON             12
#define CON_READ_IMOTD              13
#define CON_READ_MOTD               14
#define CON_BREAK_CONNECT           15
#define CON_GET_ALLIANCE            16
#define CON_GET_TENDENCY            17
#define CON_HOTREBOOT_RECOVER       18
#define CON_REROLL_STATS			19
#define CON_GET_COLOUR				20

// mudftp states
#define CON_FTP_COMMAND				21
#define CON_FTP_DATA				22
#define CON_FTP_AUTH				23
 
// newbie confirming email addresses states
#define CON_RESOLVE_IP				24
#define CON_GET_EMAIL				25
#define	CON_ENTER_UNLOCK_ID			26
#define CON_GET_AUTOMAP				27
#define CON_RECHECK_EMAIL			28

#define CON_WEB_REQUEST				29

void nanny_get_email(connection_data *d, const char *argument);
void nanny_resolve_ip(connection_data *d, const char *argument);
void nannysup_email_check(connection_data *d, const char *argument);
void nanny_enter_unlock_id(connection_data *d, const char *argument);
void nanny_break_connect(connection_data *d, const char *argument);
void nanny_detect_client_settings(connection_data *d, char *argument);

#endif // NANNY_H
