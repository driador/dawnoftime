/**************************************************************************/
// notenet.cpp - Immortal notification of notes
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
#include "include.h"
#include "interp.h"
#include "notenet.h"
 
// notenet table and prototype for future flag setting 
const   struct wiznet_type  notenet_table[]={
	{   "noblepkill",			NOTE_NOBLEPKILL,		IM },
	{   "immpkill",				NOTE_IMMPKILL,			IM },
	{   "immpkilldetails",		NOTE_IMMPKILLDETAILS,	IM },
	{   "clannotes",			NOTE_CLANNOTES,			IM },
    {   "pkill",                NOTE_TO_PKILL,          IM },
	{	"allclan",				NOTE_TO_ALLCLAN,		IM },
	{	"admin_rename",			NOTE_ADMIN_RENAME,		ADMIN },
	{	"court",				NOTE_COURT,				IM},
	{   NULL,           0,              0  }
};

/**************************************************************************/
// returns a flag for notenet
long notenet_lookup (const char *name)
{
    int flag;

	// exact match first
    for (flag = 0; notenet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(notenet_table[flag].name[0])
	&& !str_cmp(name,notenet_table[flag].name))
	    return flag;
    }

	// substring match
    for (flag = 0; notenet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(notenet_table[flag].name[0])
	&& !str_prefix(name,notenet_table[flag].name))
	    return flag;
    }

    return -1;
}
/**************************************************************************/
void do_notenet( char_data *ch, char *argument )
{
    int flag, col=1;
    char buf[MSL], buf2[MSL];

    if ( IS_NULLSTR(argument)) // display status 
    {
        buf[0] = '\0';
        strcat(buf,"   ---==== Current Notenet Filtering Out Settings ====---\r\n");

        for (flag = 0; notenet_table[flag].name != NULL; flag++)
        {
            if (notenet_table[flag].level <= get_trust(ch))
            {
                sprintf(buf2,"     %-15s %s",notenet_table[flag].name,
                (IS_SET(ch->notenet,notenet_table[flag].flag)?"on   ":"off  "));
                strcat(buf,buf2);
                col++;
                if (col%2==1)
                    strcat(buf,"\r\n");

            }
        }
		ch->println(buf);
        return;
     }

    flag = notenet_lookup(argument);

    if (flag == -1 || get_trust(ch) < notenet_table[flag].level)
    {
		ch->println("No such option.");
		return;
    }
   
    if (IS_SET(ch->notenet,notenet_table[flag].flag))
    {
		ch->printlnf("You will no longer have %s filtered out when reading the default note.",
				notenet_table[flag].name);		
		REMOVE_BIT(ch->notenet,notenet_table[flag].flag);
    	return;
    }
    else
    {		
    	ch->printlnf("You will now have %s filtered out when reading the default note.", 
			notenet_table[flag].name);
    	SET_BIT(ch->notenet,notenet_table[flag].flag);
	return;
    }

}

/**************************************************************************/
