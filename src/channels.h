/**************************************************************************/
// channels.h - header for channel related functions
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef CHANNELS_H
#define CHANNELS_H

#define CHANNEL_ADMINTALK	(A)
#define CHANNEL_CODETALK	(B)
#define CHANNEL_FLAME		(C)
#define CHANNEL_GSOCIAL		(D)
#define CHANNEL_GRATS		(E)
#define CHANNEL_HIGHTALK	(F)
#define CHANNEL_IC			(G)
#define CHANNEL_IMMTALK		(H)
#define CHANNEL_INFO		(I)
#define CHANNEL_MYTHOSTALK	(J)
#define CHANNEL_NEWBIETALK	(K)
#define CHANNEL_NOBLETALK	(M)
#define CHANNEL_OOC			(N)
#define CHANNEL_QA			(O)
#define CHANNEL_QUIET		(P)
#define CHANNEL_TELLS		(Q)
#define CHANNEL_CLANTALK	(Q)
#define CHANNEL_CHAT		(R)

#define CHANFLAG_MYSTERY_IMMS			(A) // if set, immortals appear as mystery imms to those who can't see them on who
#define CHANFLAG_ANONYMOUS_TO_MORTS		(B)
#define CHANFLAG_IMM_CHANNEL			(C)
#define CHANFLAG_IMMTALK_ACCESS			(D)
#define CHANFLAG_USE_PERS_IF_SWITCHED	(E)
#define CHANFLAG_ADMIN					(F)
#define CHANFLAG_CODE_COUNCIL			(G)
#define CHANFLAG_MYTHOS_COUNCIL			(H)
#define CHANFLAG_HIGHADMIN				(I)
#define CHANFLAG_NOBLE					(J)
#define CHANFLAG_NEWBIES_AND_SUPPORT	(K)
#define CHANFLAG_CLANMEMBERS_ONLY		(L)
#define CHANFLAG_IGNORE_QUIET			(M)

class channel_data
{
public:
	int *index;
	const char *name;
	const char *colour;
	const char *you_format; // first %s is text, include You at start
	const char *format; // first %s is name, second %s is text
	const char *trailer; // trails the message, channel colour code inserted before this
	const char *logfile;
	int memory_log_lines;
	long channel_flag_value;
	long flags;
	char **memory_log;
	int memory_log_index;

	// member functions
public:
	bool can_see_channel(char_data *ch, char_data *talker);
};

extern class channel_data channels[];

// some prototypes
int channel_exact_lookup(const char *channel_name);
int channel_lookup(const char *channel_name);
char *channel_convert_bitflags_to_text(long bits);
long channel_convert_text_to_bitflags(char *text);
void channels_initialize();

#endif // CHANNELS_h

