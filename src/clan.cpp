/**************************************************************************/
// clan.cpp - Clan Class and clan related functions - Tibault & Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "olc.h"
#include "clan.h"

void save_clan_db( void );

CClanType *clan_list=NULL;
/**************************************************************************/    
CClanType::CClanType()
{
	m_pSaveName		= str_dup("");
	m_pName			= str_dup("");
	m_pNoteName		= str_dup("");
	m_pWhoName		= str_dup("");
	m_pDescription	= str_dup("");
	m_pWhoCat		= str_dup("");
	m_pColorStr		= str_dup("");
	m_RecallRoom	= 0;
	m_BankRoom		= 0;

	m_pClanRankTitle[0]	= str_dup( "toprank" );
	m_CanAdd[0]			= true;
	m_CanPromote[0]		= true;
	m_CanRemove[0]		= true;
	m_CanWithdraw[0]	= true;

	for ( int i=1; i< MAX_RANK; i++ )
	{
		m_pClanRankTitle[i]	= str_dup( "" );
		m_CanAdd[i]			= false;
		m_CanPromote[i]		= false;
		m_CanRemove[i]		= false;
		m_CanWithdraw[i]	= false;
	}

	next=NULL;
}

/**************************************************************************/    
CClanType::~CClanType()
{
	free_string(m_pSaveName);
	free_string(m_pName);
	free_string(m_pNoteName);
	free_string(m_pWhoName);	
	free_string(m_pDescription);	
	free_string(m_pWhoCat);
	free_string(m_pColorStr);
	for ( int i=0; i< MAX_RANK; i++ )
	{
		free_string(m_pClanRankTitle[i]);
	}
}

/**************************************************************************/    
void CClanType::printDetails(char_data *ch)
{
	if(IS_ADMIN(ch)){
		ch->printlnf( " `#%s%-40s`& %-15s %6d %8d",
			m_pColorStr, m_pName, m_pNoteName, m_RecallRoom, (int)m_BankFunds);
	}else if(IS_IMMORTAL(ch)){
		ch->printlnf( " `#%s%-40s`& %-15s %6d",
			m_pColorStr, m_pName, m_pNoteName, m_RecallRoom );
	}else{
		ch->printlnf( " `#%s%-40s`& %-15s ???",
			m_pColorStr, m_pName, m_pNoteName);
	}
}

/**************************************************************************/    
void CClanType::printRanks( char_data *ch )
{
	int i;
	int count=0;

	for ( i=0; i< MAX_RANK; i++ ) 
	{
		if ( !IS_NULLSTR(m_pClanRankTitle[i]) ) {
			ch->printlnf( "`sRank %d `s- Title %s%s",
				i,
				m_pColorStr,
				m_pClanRankTitle[i] );
			count++;
		}
	}
	ch->printlnf("There %s a total of %d rank%s in %s",
		count==1?"is":"are", 
		count, 		
		count==1?"":"s", 
		cname());

}

/**************************************************************************/
int CClanType::minRank()
{
	int lIdx;

	for ( lIdx=MAX_RANK-1; lIdx>=0; lIdx-- ) {
		if( !IS_NULLSTR(m_pClanRankTitle[lIdx])){
			return lIdx;
		}
	}

	return 0;
}

/**************************************************************************/
/**************************************************************************/
//	GENERIC CLAN FUNCTIONS												  
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
//	do_addclan											  Tibault:Sep 00
//	Function for admin or headclan for setting the clan on players.
//	You can also set the rank with this command. If omitted, the top rank
//	will be set.
/**************************************************************************/
void do_addclan( char_data *ch, char *argument )
{
	char arg1[MIL],arg2[MIL],arg3[MIL];
	char_data *victim;
	int rank = 0;

	CClanType* pClan;

    if (!IS_ADMIN(ch)
        && !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADCLAN))	
    {
		ch->println( "Addclan can only be used by the head of the clan council or admin." );
        return;
    }

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	
	if ( arg1[0] == '\0' || arg2[0] == '\0'  )
	{
		ch->println( "Syntax: addclan <char> <clan name> <clan rank>" );
		ch->println( "Syntax: addclan <char> none (to remove them from a clan)" );
		return;
	}
	if ( ( victim = get_whovis_player_world( ch, arg1 ) ) == NULL )
	{
		ch->println( "They aren't playing." );
		return;
	}
	
	if (!str_prefix(arg2,"none"))
	{
        ch->println( "They are now clanless." );
        victim->println( "You are now a member of no clan!" );
		victim->clan	= NULL;
		victim->clanrank = 0;
        return;
	}
	
	if (!IS_LETGAINED(victim)){
        ch->printlnf( "%s hasn't been letgained yet.", PERS(victim, ch) );
        return;
	}
		
	if ((pClan = clan_lookup(arg2)) == NULL)
	{
        ch->printlnf( "No such clan '%s' exists.", arg2 );
        return;
	}
	
	if (IS_NULLSTR(arg3))
	{
		rank=pClan->minRank();
	}
	else
	{
		if ((rank = pClan->rank_lookup(arg3) ) == -1)
		{
			ch->printlnf( "No such rank '%s' exists in the clan '%s'.", 
				arg3, pClan->cname());
			return;
		}
	}
	
	{
		ch->printlnf( "They are now a %s (%d) of clan %s.",
			pClan->clan_rank_title(rank), rank, pClan->cname() );
		victim->printlnf("You are now a %s of clan %s.",
			pClan->clan_rank_title(rank), pClan->cname() );
	}
	
	victim->clan  = pClan;
	victim->clanrank = rank;
}

/**************************************************************************
	do_clanlist											  Tibault:Sep 00
	Prints the details for every clan in the global variable clan_list.
	If you want to change the way these details are shown, change 
	the CClanType function printdetails().
**************************************************************************/
void do_clanlist(char_data *ch, char *)
{
    CClanType *pClan;

	if(!clan_list){
		ch->println("There are currently no clans, clans can be added using clanedit create.");
		return;
	}

    ch->print( "`#`S-`YClanname`S===================================`YNoteName`S========`YRecall`S");

	if(IS_ADMIN(ch)){
		ch->println("====`YFunds`S-`^");
	}else{
		ch->println("-`^");
	}

    for (pClan = clan_list; pClan; pClan = pClan->next ){
		pClan->printDetails(ch);
	}
}
/**************************************************************************/
// Kal, Feb 02
void clan_show_clanrank_syntax(char_data *ch)
{
	if(IS_IMMORTAL(ch)){
		ch->println("syntax: clanrank <clanname> - to see the ranks of a given clan.");
		ch->println("syntax: clanrank <player> - to see the rank of a given player.");
	}else{
		ch->println("syntax: clanrank <clanmember> - to see the rank of another clan member in the same room.");
	}
}

/**************************************************************************/
// Kal, Feb 02
void clan_show_ranks(char_data *ch, char_data *victim, CClanType *pClan)
{
	if(pClan){
		pClan->printRanks(ch);

		if(victim){
			ch->printlnf( "%s's rank in this clan is: `#%s%s`^",
				PERS(victim, ch),
				pClan->color_str(), 
				pClan->clan_rank_title(victim->clanrank) );   
		}
		
		if ( ch->clan == pClan){
			ch->printlnf( "Your rank in this clan is: `#%s%s`^",
				pClan->color_str(), 
				pClan->clan_rank_title(ch->clanrank) );   
		}
	}

}

/**************************************************************************/
// Kal, Feb 02
void do_clanranks(char_data *ch, char *argument)
{	
	char_data *victim;

	// non clanned players can not use the command
	if(!IS_IMMORTAL(ch) && !ch->clan){
		ch->println("You are clanless, you have no use for this command.");
		return;
	}

	// if no argument is specified, show a player/immortal 
	// the clan they are in
	if(IS_NULLSTR(argument)){		
		clan_show_ranks(ch, NULL, ch->clan);
		clan_show_clanrank_syntax(ch);
		return;
	}

	// attempt to find a specified player
	victim = get_whovis_player_world(ch, argument);
	if(!victim){
		// character not found
		if(IS_IMMORTAL(ch)){ // imms can also specify the clan name
			// picking a clan
			CClanType* pClan= clan_lookup(argument);
			if(!pClan){
				ch->printlnf( "No such clan or player '%s' is currently in the game.", argument );
				clan_show_clanrank_syntax(ch);
				return;
			}

			// clan found, show the ranks
			clan_show_ranks(ch, NULL, pClan);
			return;
		}

		ch->printlnf( "No such clanmember '%s' is currently in the game.", argument );	
		return;
	}

	// victim was found, do some checks
	if(IS_IMMORTAL(ch)){
		if(!victim->clan){
			ch->printlnf("%s is not in a clan.", PERS(victim, ch));
			clan_show_clanrank_syntax(ch);
			return;
		}
	}else{
		if(!victim->clan || victim->clan!=ch->clan){
			ch->printlnf("%s is not in the same clan as you.", PERS(victim, ch));
			clan_show_clanrank_syntax(ch);
			return;
		}

		if(ch->in_room!=victim->in_room){
			ch->printlnf("%s is not in the same room as you.", PERS(victim, ch));
			clan_show_clanrank_syntax(ch);
			return;
		}
	}

	
	// victim found, show the ranks
	clan_show_ranks(ch, victim, victim->clan);
}

/**************************************************************************
	do_add												Tibault:Sep 00
	Function for mortals to add members to their clan.
**************************************************************************/
void do_add( char_data *ch, char *argument )
{
    char arg1[MIL];
	char_data *victim;

	if ( IS_OOC(ch) ) {
		ch->println( "Add is an IC command, you can't use it in OOC rooms." );
		return;
    }

    argument = one_argument( argument, arg1 );
    if( ch->clan  == NULL ) {
        ch->println( "You are not in a clan yourself!" );
        return;
    }

    if( ch->clan->m_CanAdd[ch->clanrank] == false ) {
        ch->println( "You don't have the authority to do that." );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
		ch->println( "They aren't playing." );
		return;
    }

    if ( victim->in_room != ch->in_room ) {
        ch->println( "They aren't here." );
        return;
    }

    if ( !IS_LETGAINED(victim) ) {
        ch->printlnf( "%s hasn't been letgained yet.", PERS(victim, ch) );
        return;
    }

    if( victim->clan  ) {
        ch->printlnf( "%s is already in a clan.", PERS(victim, ch) );
        return;
    }

    if( victim->seeks != ch->clan ) {
        ch->wraplnf("They are not seeking to join your clan."
			"They have been told you invited them to join, and to seek your clan if interested"
			"- try again shortly.");

        victim->wraplnf("%s invited you to become a member of a "
            "clan which goes by the name '%s'.  You weren't seeking that "
            "clan, type 'seek %s' if you want to join, and when they invite "
            "you again... you will automatically join.",
            PERS(ch, victim),
            ch->clan->name(),
            ch->clan->notename());
        return;
    }

    victim->clan 		= ch->clan ;
    victim->clanrank	= ch->clan->minRank();
	victim->seeks		= NULL;

    ch->printlnf( "They are now a %s of your clan.",
		victim->clan->clan_rank_title(victim->clanrank));
	victim->printlnf( "You are now a %s of clan %s.",
		victim->clan->clan_rank_title(victim->clanrank),
		victim->clan->name());

    return;
}

/**************************************************************************
	do_demote											  Tibault:Sep 00
	Function for mortals to demote members to their clan.
**************************************************************************/
void do_demote( char_data *ch, char *argument )
{
    char arg1[MIL];
	char_data *victim;

	if ( IS_OOC(ch) ) {
		ch->println( "Demote is an IC command, you can't use it in OOC rooms." );
		return;
    }

    if( ch->clan  == NULL ) {
        ch->println( "You are not in a clan yourself!" );
        return;
    }

    if( ch->clan->m_CanPromote[ch->clanrank] == false ) {
        ch->println( "You don't have the authority to do that." );
        return;
    }

	argument = one_argument( argument, arg1 );
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
		ch->println( "They aren't playing." );
		return;
    }

    if ( victim->in_room != ch->in_room ) {
        ch->println( "They aren't here." );
        return;
    }

    if( victim->clan  != ch->clan  ) {
        ch->println( "They are not in your clan." );
        return;
    }

    if( victim->clanrank <= ch->clanrank  ) {
        ch->println( "They are too high in rank for your authority." );
		ch->println( "You Must ask an Immortal to remove another leader." );
        return;
    }

    if( victim->clanrank >= victim->clan->minRank() ) {
		ch->println( "They are already of the lowest rank." );
		return;
	}

    victim->clanrank++;

    ch->printlnf( "They are now a %s of your clan.",
		 ch->clan->clan_rank_title(victim->clanrank) );

	victim->printlnf( "You are now a %s of clan %s.",
         victim->clan->clan_rank_title(victim->clanrank),
         victim->clan->name());

    return;
}

/**************************************************************************
	do_outcast											  Tibault:Sep 00
	Function for mortals to outcast members to their clan.
**************************************************************************/
void do_outcast( char_data *ch, char *argument )
{
    char arg1[MIL];
	char_data *victim;

	if ( IS_OOC(ch) ) {
		ch->println( "Outcast is an IC command, you can't use it in OOC rooms." );
		return;
    }

    if( ch->clan  == NULL ) {
        ch->println( "You are not in a clan yourself!" );
        return;
    }

    if( ch->clan->m_CanRemove[ch->clanrank] == false ) {
        ch->println( "You don't have the authority to do that." );
        return;
    }

    argument = one_argument( argument, arg1 );
    if ( (victim=get_char_world(ch, arg1)) == NULL ) {
		ch->println( "They aren't playing." );
		return;
	}

    if ( victim->in_room != ch->in_room ) {
        ch->println( "They aren't here." );
        return;
    }

    if( victim->clan  != ch->clan ){
        ch->println( "They are not in your clan." );
        return;
    }

    if(ch->clanrank >= victim->clanrank ) {
        ch->println( "You don't have the authority to do that." );
        return;
    }

    ch->printlnf( "They are no longer a %s of your clan.",
		victim->clan->clan_rank_title(victim->clanrank) );
 	victim->printlnf( "You are no longer a %s of clan %s.",
		victim->clan->clan_rank_title(victim->clanrank),
		victim->clan->name());
 	victim->println("You have been outcast!");

    victim->clan  = NULL;
    victim->clanrank = 0;

    return;
}

/**************************************************************************
	do_promote											  Tibault:Sep 00
	Function for mortals to outcast members to their clan.
**************************************************************************/
void do_promote( char_data *ch, char *argument )
{
    char arg1[MIL];
	char_data *victim;

	if ( IS_OOC(ch) ) {
		ch->println( "Promote is an IC command, you can't use it in OOC rooms." );
		return;
    }

    if( ch->clan  == NULL ) {
        ch->println("You are not in a clan yourself!" );
        return;
    }

    if( ch->clan->m_CanPromote[ch->clanrank] == false ) {
        ch->println( "You don't have the authority to do that." );
        return;
    }

    argument = one_argument( argument, arg1 );
    if ( (victim = get_char_world(ch, arg1)) == NULL ) {
		  ch->println( "They aren't playing." );
		  return;
    }

    if ( victim->in_room != ch->in_room ) {
        ch->println( "They aren't here." );
        return;
    }

    if( victim->clan  != ch->clan  ) {
        ch->println( "They are not in your clan." );
        return;
    }

    if( victim->clanrank<=ch->clanrank ){
        ch->println( "They are already too high in rank for your authority." );
        return;
    }

	if(victim->clanrank==0){
        ch->println( "They are already the highest rank in your clan." );
        return;
	}

    victim->clanrank--;

    ch->printlnf( "They are now a %s of your clan.",
		ch->clan->clan_rank_title(victim->clanrank) );
	victim->printlnf( "Congratulations, you are now a %s of clan %s.",
		ch->clan->clan_rank_title(victim->clanrank),
		ch->clan->name());

    return;
}

/**************************************************************************
	do_seek												  Tibault:Sep 00
	Function for mortals to seek the clan they wish to join.
**************************************************************************/
void do_seek( char_data *ch, char *argument )
{
    char arg1[MIL];
    CClanType* pClan;

    argument = one_argument(argument, arg1);

    if ( !str_cmp(arg1,"none") ) {
		ch->seeks = NULL;
        ch->println( "You are seeking no clan." );
        return;
    }

    if ( ch->clan ) {
        ch->println( "You are already in a clan." );
        return;
    }

    if ( (pClan = clan_lookup(arg1)) == NULL ) {
		ch->println( "No such clan exists." );
        return;
    }

    ch->seeks = pClan;

	ch->printlnf( "You are seeking to join %s.",
		pClan->name() );
	ch->println("(type 'seek none' to stop seeking a clan.)");

    return;
}
/**************************************************************************/
const char * CClanType::name()
{
	if(!this){
		return "";		
	}
	return m_pName;
}
/**************************************************************************/
const char * CClanType::cname()
{
	if(!this){
		return "";		
	}
	static char result[MIL];
	sprintf(result, "`#%s%s`&",	m_pColorStr, m_pName);

	return result;
}
/**************************************************************************/
// name used to save to disk, never changes
const char * CClanType::savename()
{
	if(!this){
		return "";		
	}
	return m_pSaveName;
}
/**************************************************************************/
const char * CClanType::notename()
{
	if(!this){
		return "";		
	}
	return m_pNoteName;
}
/**************************************************************************/
const char * CClanType::color_str()
{
	if(!this){
		return "";		
	}
	return m_pColorStr;
}
/**************************************************************************/
const char * CClanType::who_name()
{
	if(!this){
		return "";		
	}
	return m_pWhoName;
}
/**************************************************************************/
const char * CClanType::cwho_name()
{
	if(!this){
		return "";		
	}
	static char result[MIL];
	sprintf(result, "`#%s%s`&",	m_pColorStr, m_pWhoName);

	return result;
}
/**************************************************************************/
const char * CClanType::who_cat()
{
	if(!this){
		return "";		
	}
	return m_pWhoCat;
}
/**************************************************************************/
const char * CClanType::clan_rank_title(int rank)
{
	return m_pClanRankTitle[rank];
}
/**************************************************************************/
int CClanType::recall_room()
{
	return m_RecallRoom;
}

/**************************************************************************/
// Kal - Sept 2000
int CClanType::rank_lookup(const char *ranktitle)
{
	int rnk;
	if(this==NULL){ // clanless
		return 0;		
	}

	for (rnk = 0; rnk < MAX_RANK; rnk++)
	{
		if (!str_prefix(ranktitle,m_pClanRankTitle[rnk])){
			return rnk;
		}
	}
    return -1;
}
/**************************************************************************/
CClanType* clan_lookup( const char *name )
{
    CClanType *pClan;

	if(IS_NULLSTR(name)){
		return NULL;
	}

	// first try an exact match
	for(pClan=clan_list;pClan;pClan=pClan->next){
		if(!str_cmp(name,pClan->name() )){
			return pClan;
		}
	}

	// now try a prefix match
	for(pClan=clan_list;pClan;pClan=pClan->next){
		if(!str_prefix(name,pClan->name())){
			return pClan;
		}
	}

	// not found
    return NULL;
}
/**************************************************************************/
// lookup a clan by their note name - Kalahn oct97
CClanType* clan_nlookup( const char *name )
{
    CClanType *pClan;

	// first try an exact match
	for(pClan=clan_list;pClan;pClan=pClan->next){
		if(!str_cmp(name,pClan->notename())){
			return pClan;
		}
	}

	// now try a prefix match
	for(pClan=clan_list;pClan;pClan=pClan->next){
		if(!str_prefix(name,pClan->notename())){
			return pClan;
		}
	}

	// not found
    return NULL;
}
/**************************************************************************/
// lookup a clan by their save name - Kalahn Sept2000
CClanType* clan_slookup( const char *savename )
{
    CClanType *pClan;

	// only do exact matching
	for(pClan=clan_list;pClan;pClan=pClan->next){
		if(!str_cmp(savename,pClan->savename())){
			return pClan;
		}
	}

	// not found
    return NULL;
}
/**************************************************************************/    
void clan_bank( char_data *ch, char *task, char *amount)
{
	int number;
	if(IS_NPC(ch)){
		ch->println("clan banking by players only sorry.");
		return;
	}
	
	if(!str_prefix(task,"balance") )
	{
		ch->printlnf( "Your clan has %ld gold in its coffers.", ch->clan->m_BankFunds );

		// the clanbank transaction trail
		append_datetimestring_to_file( CLANBANKING_FILE, 
			FORMATF("%s[%d] got a clanbank balance of %s - balance is %d`x",
			TRUE_CH(ch)->name,
			ch->clanrank,
			ch->clan->who_name(),
			(int)ch->clan->m_BankFunds));

		return;
	}     
	
	if(!is_number(amount))
	{
		ch->printlnf( "clanbank: The second argument must be a number, '%s' is not.", amount );
		return;
	}

	number=atoi(amount);
	if(number<=0)
	{
		ch->println( "clanbank: The second argument must be a number greater than 0." );
		return;
	}
	if(number>1250000){
		ch->println( "clanbank: Sorry, we don't deal with such large amounts!" );
		return;
	}
	
	if(!str_prefix(task,"deposit"))
	{
		if( number+5>ch->gold )
		{
			
			if( number>ch->gold ){
				ch->println( "You do not have that much money." );
			}else{
				ch->wrapln( "There is a 5 gold piece fee per deposit.  Banking all that money will not leave you with enough to pay our deposit fee." );
			}
			return;
		}
		ch->gold -= number + 5;
		ch->clan->m_BankFunds += number;
		ch->printlnf( "Deposit made.\r\nYour clan now has %ld gold pieces in its coffers.",
			ch->clan->m_BankFunds );

		logf("[clanbank] %s deposited %d gold into clan bank account %s - balance now %d",
			ch->name,
			number,
			ch->clan->who_name(),
			(int)ch->clan->m_BankFunds);

		// the clanbank transaction trail
		append_datetimestring_to_file( CLANBANKING_FILE, 
			FORMATF("%s[%d] deposited %d gold into clan bank account %s - balance now %d`x",
			TRUE_CH(ch)->name,
			ch->clanrank,
			number,
			ch->clan->who_name(),
			(int)ch->clan->m_BankFunds));

		// resave the clan database
		save_clan_db();		
		save_char_obj(ch); // resave them
		WAIT_STATE(ch, PULSE_PER_SECOND*3);
		return;
	}
	
    if(!str_prefix(task,"withdraw"))
	{
		if ( !ch->clan->m_CanWithdraw[ch->clanrank])
		{
			// the clanbank transaction trail
			append_datetimestring_to_file( CLANBANKING_FILE, 
				FORMATF("%s[%d] attempted to withdraw %d gold from clan bank account %s but didn't have authority - balance is %d`x",
				TRUE_CH(ch)->name,
				ch->clanrank,
				number,
				ch->clan->who_name(),
				(int)ch->clan->m_BankFunds));
			ch->println( "You do not have the authority to withdraw money from your clan." );
			return;
		}
		if( number < 6 )
		{
			ch->println("Due to the surcharge of 5 gold you can not withdraw that small of an amount.");
			return;
		}
		
		if( number > ch->clan->m_BankFunds-5 )
		{
			ch->println("You do not have that much in the bank.");
			return;
		}
		ch->gold += number;
		ch->clan->m_BankFunds -= number + 5;
		ch->printlnf( "Withdrawal made.  There was a 5 gold piece surcharge.\r\nYour clan now has %ld gold pieces in its coffers.",
			ch->clan->m_BankFunds );
		
		logf("[clanbank] %s withdrew %d gold from clan bank account %s - balance now %d",
			ch->name,
			number,
			ch->clan->who_name(),
			(int)ch->clan->m_BankFunds);

		// the clanbank transaction trail
		append_datetimestring_to_file( CLANBANKING_FILE, 
			FORMATF("%s[%d] withdrew %d gold from clan bank account %s - balance now %d`x",
			TRUE_CH(ch)->name,
			ch->clanrank,
			number,
			ch->clan->who_name(),
			(int)ch->clan->m_BankFunds));

		// resave the clan database
		save_clan_db();
		save_char_obj(ch); // resave them
		WAIT_STATE(ch, PULSE_PER_SECOND*3);
		return;
	}
	
	ch->printlnf( "'%s' is not a valid clanbank transaction.", task );
	
	return;
}
/**************************************************************************/    
/**************************************************************************/    
/**************************************************************************/    
/**************************************************************************/    
