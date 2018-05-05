/**************************************************************************/
// clan.h - Clan Class and clan related functions - Tibault & Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef CCLAN_H
#define CCLAN_H

class CClanType
{
public:
	CClanType*	next;	

	int			minRank();
	void		printDetails( char_data* ch );
	void		printRanks( char_data* ch );
				CClanType();
	virtual		~CClanType();

	const char *name(); 
	const char *cname(); 
	const char *savename();  // name used to save to disk, never changes
	const char *color_str(); 
	const char *notename(); 
	const char *who_name(); 
	const char *cwho_name();
	const char *who_cat(); 
	const char *clan_rank_title(int rank);
	int rank_lookup(const char *ranktitle);
	int recall_room();

	char*		m_pDescription;
	char*		m_pWhoCat;
	bool		m_CanAdd[MAX_RANK];
	bool		m_CanPromote[MAX_RANK];
	bool		m_CanRemove[MAX_RANK];
	bool		m_CanWithdraw[MAX_RANK];
	
	char*		m_pSaveName; // name used to save to disk, never changes
	char*		m_pName;
	char*		m_pWhoName;
	char*		m_pNoteName;
	char*		m_pClanRankTitle[MAX_RANK];
	char*		m_pColorStr;
	int			m_RecallRoom;
	int			m_BankRoom;
	long		m_BankFunds;
};

CClanType* clan_lookup( const char *name );
CClanType* clan_nlookup( const char *name );
CClanType* clan_slookup( const char *savename );

void load_clan_db( void );
#define resave do_transfer
void do_transfer(char_data*, char*);
void clan_bank( char_data *ch, char *task, char *amount);

#endif // CCLAN_H

