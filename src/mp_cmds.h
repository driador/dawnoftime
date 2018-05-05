/**************************************************************************/
// mob_cmds.h - Mudprogs commands, code greatly enhanced by Kal
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
 *  Based on MERC 2.2 MUDprograms by N'Atas-ha.                            *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 ***************************************************************************/

struct	mob_cmd_type
{
    const char	* name;
    DO_FUN		* do_fun;
	const char	* descript;
	const char	* syntax;
	const char	* notes;
	const char	* example;
};

/* the command table itself */
extern	const	struct	mob_cmd_type	mob_cmd_table	[];

/*
 * Command functions.
 * Defined in mob_cmds.c
 */
DECLARE_DO_FUN(	do_mpasound		);
DECLARE_DO_FUN(	do_mpgecho		);
DECLARE_DO_FUN(	do_mpzecho		);
DECLARE_DO_FUN(	do_mpkill		);
DECLARE_DO_FUN(	do_mpassist		);
DECLARE_DO_FUN(	do_mpjunk		);
DECLARE_DO_FUN(	do_mpechoaround	);
DECLARE_DO_FUN(	do_mpecho		);
DECLARE_DO_FUN(	do_mpechoat		);
DECLARE_DO_FUN(	do_mpmload		);
DECLARE_DO_FUN(	do_mpoload		);
DECLARE_DO_FUN(	do_mppurge		);
DECLARE_DO_FUN(	do_mpgoto		);
DECLARE_DO_FUN(	do_mpat			);
DECLARE_DO_FUN(	do_mptransfer	);
DECLARE_DO_FUN(	do_mpgtransfer	);
DECLARE_DO_FUN(	do_mpforce		);
DECLARE_DO_FUN(	do_mpgforce		);
DECLARE_DO_FUN(	do_mpvforce		);
DECLARE_DO_FUN(	do_mpcast		);
DECLARE_DO_FUN(	do_mpdamage		);
DECLARE_DO_FUN(	do_mpremember	);
DECLARE_DO_FUN(	do_mpforget		);
DECLARE_DO_FUN(	do_mpdelay		);
DECLARE_DO_FUN(	do_mpcancel		);
DECLARE_DO_FUN(	do_mpcall		);
DECLARE_DO_FUN(	do_mpflee		);
DECLARE_DO_FUN(	do_mpotransfer	);
DECLARE_DO_FUN(	do_mpremove		);
// new to the dawn of time
DECLARE_DO_FUN(	do_mpqset		);
DECLARE_DO_FUN( do_tgive		);	// Ker
DECLARE_DO_FUN( do_tremove		);	// Ker
DECLARE_DO_FUN( do_ttimer		);	// Ker
DECLARE_DO_FUN( do_tjunk		);	// Ker
DECLARE_DO_FUN( do_mpinflict	);	// Cel
DECLARE_DO_FUN( do_mpfollow		);	// Ker
DECLARE_DO_FUN( do_mpsneaky		);	// Ker
DECLARE_DO_FUN( do_mpstun		);	// Ker
DECLARE_DO_FUN( do_mpletpass	);	// Kal
DECLARE_DO_FUN( do_mppreventattack ); // Kal
DECLARE_DO_FUN( do_mpseeall		);	// Kal
DECLARE_DO_FUN( do_mpnoseeall	);	// Kal
DECLARE_DO_FUN( do_mpmsptochar	);  // Kal
DECLARE_DO_FUN( do_mpmsptoroom	);  // Kal
DECLARE_DO_FUN( do_dupeobj		);	// Ker
DECLARE_DO_FUN( do_mppreventmove);  // Kal
DECLARE_DO_FUN( do_mppreventobjectaction);  // Kal, April 04
DECLARE_DO_FUN( do_mpdzecho		);  // Kal
DECLARE_DO_FUN( do_mpqcall		);  // Kal
DECLARE_DO_FUN( do_mpqsay		);  // Kal
DECLARE_DO_FUN( do_mpqemote		);  // Kal
DECLARE_DO_FUN( do_mpqueue		);  // Kal
DECLARE_DO_FUN( do_mpwander		);	// Kal
DECLARE_DO_FUN( do_mpsetclass	);  // Tristan, April 04
DECLARE_DO_FUN( do_mpsetskill	);	// Ker
DECLARE_DO_FUN( do_mpzoecho		);	// Ker
DECLARE_DO_FUN( do_mpaffect		);	// Ker
DECLARE_DO_FUN( do_mpzuecho		);	// Ker
DECLARE_DO_FUN( do_mpdequeueall );	// Kal
DECLARE_DO_FUN( do_mploginprocessed); // Kal
DECLARE_DO_FUN( do_mplogoutprocessed); // Kal
DECLARE_DO_FUN( do_mpinroom		);	// Kal
DECLARE_DO_FUN( do_mpswipe		);	//Ker
DECLARE_DO_FUN( do_mpxpreward	);	// Kal, Aug 2002
DECLARE_DO_FUN( do_mppreventtrain); // Kal, Sept 2002
DECLARE_DO_FUN( do_mppreventprac); // Kal, Sept 2002
DECLARE_DO_FUN( do_mpcustomize);	// Tristan, April 04

DECLARE_DO_FUN( do_mpreturnmoney);	// Kal, Aug 2004
DECLARE_DO_FUN( do_mpsuppresstext); // Kal, April 09

