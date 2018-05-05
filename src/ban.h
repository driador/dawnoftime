/**************************************************************************/
// ban.h - code related to banning players, pretty much a rewrite by Kal
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

#ifndef BAN_H
#define BAN_H

// types of bans
#define BAN_UNDEFINED	0
#define BAN_ALL			1	// strongest
#define BAN_PERMIT		2	// permit required
#define BAN_EMAIL_REQ	3	// email system
#define BAN_NEWBIE		4	// newbies
#define BAN_LOGNEWBIE	5	// log a newbie when the create

typedef struct  ban_data	BAN_DATA;
BAN_DATA *check_ban(connection_data *c,int type);
BAN_DATA *check_ban_for_site(char *site,int type);
bool email_descriptor_unlock_id(connection_data *d);
int check_email_ban(connection_data *d, char *email_addy);

struct  ban_data
{
	// saved data
	char *      intended_people;
    int			type;
	char *      sitemasks;
	time_t		ban_date;
	char *      reason; // multiple line string
	char *      by;
	time_t		expire_date; // < 1 = noexpire
	char *		custom_disconnect_message;
	char *      always_allowed_email_masks;		// multiline... 
					// email addy must match one of the masks
	char *      allowed_email_masks;		// multiline... 
					// email addy must match one of the masks
	char *      disallowed_email_masks; // multiline... 
					// email addy if matches one of these rejected
	char *      disallowed_email_custom_message;
	sh_int      level;

	// non saved data should be at the bottom for GIO to not stuff 
	// up with word alignments
    ban_data *  next;
    bool        valid;
	bool        permanent; // saved if true
	bool        enabled;
};

#endif // BAN_H
