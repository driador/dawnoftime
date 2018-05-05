/**************************************************************************/
// laston.h - Laston command
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef LASTON_H
#define LASTON_H

#ifndef MUD_NAME
#define MUD_NAME "The Dawn of Time"
#endif

#ifndef ADMIN
#define ADMIN (MAX_LEVEL - 3) // admin group for notes and stuff 
#endif

#define LASTON_SAVE_DATA true

// save laston data every 15 mins - recorded in seconds 
#define LASTON_SAVE_INTERVAL 60*15

// array_size - the number of times laston records 
#ifndef LASTON_ARRAY_SIZE
#define LASTON_ARRAY_SIZE 7
#define LASZ LASTON_ARRAY_SIZE
#endif 

//    Last on data - stores data when people were last on
typedef struct laston_data
{
    struct laston_data * next;
    struct laston_data * prev;
    bool    in_list;        // true when they are in the linked list
    char*   name;           // name of player
    long    player_id;      // players id
    int     index;          // which is the newest time (mod LASZ)
    char*   host[LASZ];     // records which host machine they came in from
    char*   ip[LASZ];		// records which ip they connected from
    char*   ident[LASZ];    // records their ident they had a logon
    int     level[LASZ];    // level they were when they logged off
    int		sublevel[LASZ];	// sublevel they were when they logged off
    time_t  on[LASZ];       // last LAS times logon time 
    time_t  off[LASZ];      // last LAS times logoff time
    time_t  deleted_date;	// date they deleted, if non zero
    int     race;          
    int     clss;          
    int     trust;          // trust level they had when they logged off
    CClanType *clan;           // players clan
    int     clanrank;       // players rank in the clan
    int     security;       // olc security level
    sh_int	sex;          
    long    flags;          // series of flags relating to player
    long    council;        // series of flags relating to which council a player is on
    int     rps;			// RPS score
    int     xp;				
    int     alliance;		
    int     tendency;		
	long	bank;
	long	gold;
	long	silver;
	long	wealth; // not saved... basically bank *100 + gold *100 + silver... hopefully we don't get any overflow
	char*	short_descr;
    int     played;			// seconds played
	char*	email;
	char*	webpage;
	int		logout_room, login_room;
	int		know_index;
	char*	webpass;		// used for webpage password
	char*	mxp_client_version;
	char*	terminal_type;
	sh_int	wiznet_type;	// saved as an int so we don't have to save the default

	// top-rpers system
	struct laston_data * next_rper;
	// top-wealth system
	struct laston_data * next_wealth;
} LASTON_DATA;

#define LASTON_ONLINE       (A)  // marks player as online
#define LASTON_ON_AT_REBOOT (B)  // online at reboot
#define LASTON_CAN_ADVANCE  (C)  // people that are letgained
#define LASTON_NOT_SHOWN    (D)  // not shown on the laston list
#define LASTON_LOGGED		(E)  // players that are logged
#define LASTON_IRC          (F)  // player last connected on IRC port last time
#define LASTON_FIRSTIRC     (G)  // player first time IRC
#define LASTON_HASUSEDIRC   (H)  // player has used the IRC gateway

#define LASTON_NOBLE		(I)  // is a noble
#define LASTON_PERM_PKILLED	(J)  // was perm pkilled
#define LASTON_DELETED		(K)  // in deleted directory
#define LASTON_LETGAINED	(L)  // is letgained
#define LASTON_USING_AUTOMAP	(M) 
#define LASTON_USING_MSP		(O) 
#define LASTON_ACTIVE			(P) 
#define LASTON_QUESTER			(Q) 
#define LASTON_NSUPPORT			(R) 
#define LASTON_ALLOWIMMTALK		(S) 
#define LASTON_NOMAXKARN		(T) 
#define LASTON_MCCP				(U) 

#define LASTONWIZLISTTYPE_HIDDEN	(0)
#define LASTONWIZLISTTYPE_ACTIVE	(1) 
#define LASTONWIZLISTTYPE_GUEST		(2) 
#define LASTONWIZLISTTYPE_RETIRED	(3) 


/***************************************************************************
 *  laston saving details and structures                                   *
 ***************************************************************************/
#define LASTON_FILE			DATA_DIR "laston.txt"
#define LASTON_SAVEFILE		DATA_DIR "laston.sav"
#define LASTON_BACKFILE		DATA_DIR "laston.bak"

char *laston_generate_mud_client_stats();

#endif // LASTON_H
