/**************************************************************************/
// laston.cpp - Laston command, see below for more
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  LASTON records upto the last LASTON_ARRAY_SIZE times a character       *
 *     was on, and for how long.   Stores the data in a linked list, to    *
 *     data across a reboot, it uses my GIO (generic IO) system            *
 *                                                                         *
 *  check out http://www.dawnoftime.org/ for the latest version of dawn    *
 ***************************************************************************/
char *dawnstat_generate_mud_client_stats();
void dawnstat_update();

#include "include.h" // dawn standard includes
#include "clan.h"
#include "laston.h"

laston_data    *laston_list; //last on list system 
laston_data    *laston_rpers_list; // last on RPS list system
laston_data    *laston_wealth_list; // last on wealth list system

time_t laston_since; // rough estimate of how current the laston data is 
time_t laston_next_save=0;

char *flag_string( const struct flag_type *flag_table, int bits );

#define last_resave resave(ch, argument);laston_next_save

/**************************************************************************/
// write the race name
void lastondata_write_race(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	laston_data * pld;
	pld= (laston_data*) data;
	fprintf(fp, "%s %s~\n",		
			gio_table[tableIndex].heading,	race_table[pld->race]->name);
}
/**************************************************************************/
// read the race name 
void lastondata_read_race(gio_type *, int, void *data, FILE *fp)
{
	laston_data * pld;
	char *pstr;

	pld= (laston_data*) data;

	pstr=fread_string(fp);
	pld->race=race_lookup(pstr);
	if(pld->race<0){
		bugf("Laston found unrecognised race '%s' for '%s'", pstr,pld->name);
		pld->race=0;
	}
	free_string(pstr);
}
/**************************************************************************/
// write the class name
void lastondata_write_class(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	laston_data * pld;
	pld= (laston_data*) data;
	fprintf(fp, "%s %s~\n",		
			gio_table[tableIndex].heading,	class_table[pld->clss].name);
}
/**************************************************************************/
// read the class name 
void lastondata_read_class(gio_type *, int, void *data, FILE *fp)
{
	laston_data * pld;
	char *pstr;

	pld= (laston_data*) data;

	pstr=fread_string(fp);
	pld->clss=class_exact_lookup(pstr);
	if(pld->clss<0){
		bugf("Laston found unrecognised class '%s' for '%s'", pstr,pld->name);
		pld->clss=0;
	}
	free_string(pstr);
}

/**************************************************************************/
// write the clan name
void lastondata_write_clan(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	laston_data * pld;
	pld= (laston_data*) data;

	if(pld->clan){
		fprintf(fp, "%s %s~\n",		
			gio_table[tableIndex].heading,	pld->clan->savename());
	}
}
/**************************************************************************/
// read the clan name 
void lastondata_read_clan(gio_type *, int, void *data, FILE *fp)
{
	laston_data * pld;
	char *pstr;

	pld= (laston_data*) data;
	pstr=fread_string(fp);
	pld->clan=clan_slookup(pstr); // returns NULL if not found
	free_string(pstr);
}
/**************************************************************************/
// write the clanrank name
void lastondata_write_clanrank(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	laston_data * pld;
	pld= (laston_data*) data;

	if(pld->clan>0){
		fprintf(fp, "%s %s~\n",		
			gio_table[tableIndex].heading,	
			pld->clan->clan_rank_title(pld->clanrank));
	}
}
/**************************************************************************/
// read the clanrank name 
void lastondata_read_clanrank(gio_type *, int, void *data, FILE *fp)
{
	laston_data * pld;
	char *pstr;

	pld= (laston_data*) data;
	pstr=fread_string(fp);
	pld->clanrank=pld->clan->rank_lookup(pstr); // returns 0/-1 if not found
	if(pld->clanrank<0){
		bugf("Laston found unrecognised clanrank '%s' for '%s'", pstr,pld->name);
		pld->clanrank=0;
	}
	free_string(pstr);
}
/**************************************************************************/
// create race_type_old GIO lookup table 
GIO_START(laston_data)
GIO_STR(name)
GIO_LONGH(player_id, "id")
GIO_INT(know_index)
GIO_WFLAGH(flags, "lastonflags ", laston_flags)
GIO_WFLAGH(council, "council ", council_flags)
GIO_SHWFLAGH(sex, "sex ", sex_types)
GIO_INT(trust)
GIO_INT(security)
GIO_INT(played)
GIO_INT(logout_room)
GIO_INT(deleted_date)
GIO_CUSTOM_WRITEH(race,	"Race ", lastondata_write_race)
GIO_CUSTOM_READH(race,	"Race ", lastondata_read_race)
GIO_CUSTOM_WRITEH(clss,	"Class ", lastondata_write_class)
GIO_CUSTOM_READH(clss,	"Class ", lastondata_read_class)
GIO_CUSTOM_WRITEH(clan,	"Clan ", lastondata_write_clan)
GIO_CUSTOM_READH(clan,	"Clan ", lastondata_read_clan)
GIO_CUSTOM_WRITEH(clanrank,	"Clanrank ", lastondata_write_clanrank)
GIO_CUSTOM_READH(clanrank,	"Clanrank ", lastondata_read_clanrank)
GIO_INT(rps)
GIO_INT(xp)
GIO_INT(alliance)
GIO_INT(tendency)
GIO_STR(short_descr)
GIO_STR(email)
GIO_STR(webpass)
GIO_STR(mxp_client_version)
GIO_STR(terminal_type)
GIO_LONG(bank)
GIO_LONG(gold)
GIO_LONG(silver)
// arrays
GIO_INT(index)
GIO_STR_ARRAY(host,LASTON_ARRAY_SIZE)
GIO_STR_ARRAY(ip,LASTON_ARRAY_SIZE)
GIO_STR_ARRAY(ident,LASTON_ARRAY_SIZE)
GIO_INT_ARRAY(level,LASTON_ARRAY_SIZE)
GIO_INT_ARRAY(sublevel,LASTON_ARRAY_SIZE)
GIO_LONG_ARRAY(on,LASTON_ARRAY_SIZE)
GIO_LONG_ARRAY(off,LASTON_ARRAY_SIZE)
GIO_SHINT_WITH_DEFAULT(wiznet_type, 0) // saved as a number, to save space
GIO_FINISH

/**************************************************************************/
// prototypes
void laston_save(char_data *ch);
void resort_top_roleplayers();
int get_sublevels_for_level(int level);

/**************************************************************************/
// timediff() - written By Kalahn - June 97
char *timediff(time_t t1, time_t t2)
{
	static char timebuf[3][MSL];
    static int index; // used to rotate around 3 buffers, so timediff can 
                      // be used up to 3 times in one context without
                      // overwriting a previous buffer 
    ++index%=3;				// rotate the index
    timebuf[index][0]='\0'; // clear the previous string 

    // calculate the difference in time, and break it down 
    long dsec, dmin, dhour, dday;
    long dweek, dmonth, dyear;
    dsec = abs((int)(t1-t2)); // difference between times in seconds 

	dday  = dsec/(60*60*24);
	dsec  %= 60*60*24;

	dyear = dday/365;
	dday  %= 365;

	dmonth= dday/(365/12);
	dday -= dmonth*(365/12);

	dweek = dday/7;
	dday  %= 7;

	dhour = dsec/(60*60);
	dsec  %= 60*60;

    dmin  = dsec/60;
    dsec  = dsec%60;

	// format the result up
	char working[MSL];
    if (dyear!=0)
    {
        sprintf (working,"%ld year%s, ", dyear, ((dyear!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dmonth!=0)
    {                                 
        sprintf (working,"%ld month%s, ", dmonth, ((dmonth!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dweek!=0)
    {
        sprintf (working,"%ld week%s, ", dweek, ((dweek!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dday!=0)
    {
        sprintf (working,"%ld day%s, ", dday, ((dday!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dhour!=0)
    {
        sprintf (working,"%ld hour%s, ", dhour, ((dhour!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dmin!=0)
    {
        sprintf (working,"%ld minute%s, ", dmin, ((dmin!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dsec!=0)
    {
        sprintf (working,"%ld second%s", dsec, ((dsec!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    // remove a space and comma that could be at the end of the line 
    char *trim = &timebuf[index][str_len(timebuf[index])-1];
    if (*trim==' ' || *trim==',')
    {
        *trim = '\0';
        trim--;
        if (*trim==' ' || *trim==',')
            *trim = '\0';
    }

    return(timebuf[index]);
}
/**************************************************************************/
// condensed_timediff() - written By Kalahn - Aug 97
char *condensed_timediff(time_t t1, time_t t2)
{
	static char timebuf[3][MSL];
    static int index; // used to rotate around 3 buffers, so timediff can 
                      // be used up to 3 times in one context without
                      // overwriting a previous buffer 
    ++index%=3;				// rotate the index
    timebuf[index][0]='\0'; // clear the previous string 

    // calculate the difference in time, and break it down 
    long dsec, dmin, dhour, dday;
    long dweek, dmonth, dyear;
    dsec = abs((int)(t1-t2)); // difference between times in seconds 

	dday  = dsec/(60*60*24);
	dsec  %= 60*60*24;

	dyear = dday/365;
	dday  %= 365;

	dmonth= dday/(365/12);
	dday -= dmonth*(365/12);

	dweek = dday/7;
	dday  %= 7;

	dhour = dsec/(60*60);
	dsec  %= 60*60;

    dmin  = dsec/60;
    dsec  = dsec%60;

	// format the result up
	char working[MSL];
    if (dyear!=0)
    {
        sprintf(working,"%ld year%s,", dyear, ((dyear!=1)?"s":""));
        strcat(timebuf[index], working);
    }

    if (dmonth!=0)
    {
        sprintf (working,"%ld month%s,", dmonth, ((dmonth!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dweek!=0)
    {
        sprintf (working,"%ld week%s, ", dweek, ((dweek!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dday!=0)
    {
        sprintf (working,"%ldd,", dday);
        strcat (timebuf[index], working);
    }

    sprintf (working,"%2ld:%.2ld:%.2ld", dhour, dmin, dsec);
    strcat (timebuf[index], working);

    // remove a space and comma that could be at the end of the line 
    char *trim = &timebuf[index][str_len(timebuf[index])-1];
    if (*trim==' ' || *trim==',')
    {
        *trim = '\0';
        trim--;
        if (*trim==' ' || *trim==','){
            *trim = '\0';
		}
    }
    return(timebuf[index]);
}

/**************************************************************************/
// short_timediff() - written By Kalahn - Aug 97
char *short_timediff(time_t t1, time_t t2)
{
	static char timebuf[3][MSL];
    static int index; // used to rotate around 3 buffers, so timediff can 
                      // be used up to 3 times in one context without
                      // overwriting a previous buffer 
    ++index%=3;				// rotate the index
    timebuf[index][0]='\0'; // clear the previous string 

    // calculate the difference in time, and break it down 
    long dsec, dmin, dhour, dday;
    long dweek, dmonth, dyear;
    dsec = abs((int)(t1-t2)); // difference between times in seconds 

	dday  = dsec/(60*60*24);
	dsec  %= 60*60*24;

	dyear = dday/365;
	dday  %= 365;

	dmonth= dday/(365/12);
	dday -= dmonth*(365/12);

	dweek = dday/7;
	dday  %= 7;

	dhour = dsec/(60*60);
	dsec  %= 60*60;

    dmin  = dsec/60;
    dsec  = dsec%60;

	// format the result up
	char working[MSL];
    if (dyear!=0)
    {
        sprintf(working,"%ld year%s,", dyear, ((dyear!=1)?"s":""));
        strcat(timebuf[index], working);
    }

    if (dmonth!=0)
    {
        sprintf (working,"%ld month%s,", dmonth, ((dmonth!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dweek!=0)
    {
        sprintf (working,"%ld week%s, ", dweek, ((dweek!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (dday!=0)
    {
        sprintf (working,"%ld day%s, ", dday, ((dday!=1)?"s":""));
        strcat (timebuf[index], working);
    }

    if (str_len(timebuf[index])!=0) // put in a summary of the hrs,mins&secs
    {
        sprintf (working,"%2ld:%.2ld:%.2ld", dhour, dmin, dsec);
        strcat (timebuf[index], working);
    }
    else
    {
        if (dhour!=0)
        {
            sprintf (working,"%ld hr%s, ", dhour, ((dhour!=1)?"s":""));
            strcat (timebuf[index], working);
        }

        if (dmin!=0)
        {
            sprintf (working,"%ld min%s, ", dmin, ((dmin!=1)?"s":""));
            strcat (timebuf[index], working);
        }
    
        if (dsec!=0)
        {
            sprintf (working,"%ld sec%s", dsec, ((dsec!=1)?"s":""));
            strcat (timebuf[index], working);
        }
    }

    // remove a space and comma that could be at the end of the line 
    char *trim = &timebuf[index][str_len(timebuf[index])-1];
    if (*trim==' ' || *trim==',')
    {
        *trim = '\0';
        trim--;
        if (*trim==' ' || *trim==','){
            *trim = '\0';
		}
    }
    return(timebuf[index]);
}

/**************************************************************************/
// laston_remove_list - written by Kalahn - Sept 1997
// - removes someone from the laston list
// - NOTE: it doesn't deallocate the memory for the node
void laston_remove_list(laston_data *node)
{
    // check to make sure they are marked as being in the list
    if (!node->in_list)
        return;

    if (node== laston_list) // we are the head of the list 
    {
        laston_list = laston_list->next;// move the head 
        if (laston_list)                // if not empty list now
            laston_list->prev= NULL;    // remove node from prev link 
        node->in_list = false;          // mark node as not in list 
        return;
    }

    if (node->next) // we are in the middle of the list
    {
        node->prev->next = node->next;  // remove node from next link
        node->next->prev = node->prev;  // remove node from prev link
        node->in_list = false;          // mark node as not in list

    }
    else // we must be at the tail of the list
    {
        node->prev->next = NULL;        // remove node from next link
        node->in_list = false;          // mark node as not in list
    }

}


/**************************************************************************/
// laston_add_list - written by Kalahn - Sept 1997
// - adds someone to the laston list above the given pointer
void laston_add_list(laston_data *newnode, laston_data *where)
{
    if (!newnode) // newnode can't be null
        return;

    // check to make sure we aren't trying to add above itself
    if (newnode == where)
        return;

    // remove from the list if we are already in there 
    if (newnode->in_list)
        laston_remove_list(newnode);

    // *** check if we are starting a new list or adding to the head ***
    if (where == laston_list) // add to the head of the new list 
    {
        newnode->next= laston_list; // put in next node in list
        if (newnode->next) // link back - if list isn't NULL
            newnode->next->prev= newnode; 
        newnode->prev=NULL;         // head of list - no prev pointer 
        laston_list=newnode;        // make newnode the head of laston_list
        newnode->in_list = true;    // mark node as in list
        return;
    }

    if (!where) // you can't add newnode to a NULL pointer
        return;


    // end of checks - now add the newnode before where

    // code above tells us that 'where' is a not the head node
    newnode->next = where;
    newnode->prev = where->prev;
    where->prev   = newnode;
    newnode->prev->next = newnode; 
    newnode->in_list = true;        // mark node as now in list
}

/**************************************************************************/
// laston_update_node - written by Kalahn
// - updates a node record with the players info    
// - marks them as online                           
// - does NOT affect the position in the laston list
void laston_update_node(laston_data *node, char_data *ch, bool pload_based_update)
{
	assert(!IS_NPC(ch));

	// load their council from laston
	ch->pcdata->council= node->council;	

	if(!pload_based_update){
		// it is a logged in player, not something like do_laston_update_from_disk()
		node->level[node->index] = ch->level;
		node->sublevel[node->index] = ch->pcdata->sublevel;	
		if(ch->desc){
			replace_string(node->host[node->index], ch->desc->remote_hostname);	
			replace_string(node->ip[node->index], ch->desc->remote_ip);	
		}
		// ident
		if(ch->desc && !IS_NULLSTR(ch->desc->ident_username)){
			replace_string(node->ident[node->index], ch->desc->ident_username);
		}else{
			replace_string(node->ident[node->index],"");
		}

		if(ch->in_room){
			node->logout_room=ch->in_room->vnum;
		}else{
			node->logout_room=0;
		}
	}

    // name
    replace_string(node->name,ch->name);

    // other details
    node->race = ch->race;
    node->clss = ch->clss;
    node->trust = ch->trust;
    node->clan = ch->clan;
    node->clanrank = ch->clanrank;
    node->security = ch->pcdata->security; // olc security
	node->sex  = ch->pcdata->true_sex;
    node->rps = ch->pcdata->rp_points;
    node->xp = ch->exp;
	node->alliance = ch->alliance;
	node->tendency = ch->tendency;
	node->bank = ch->bank;
	node->gold = ch->gold;
	node->silver = ch->silver;
	node->wealth=node->bank*100 + node->gold*100 + node->silver;
	node->know_index= ch->know_index;
	

	node->played=ch->played;
    //===+--- FLAGS ---+===
	// CURRENTLY ONLINE
	if(!pload_based_update){
		SET_BIT(node->flags, LASTON_ONLINE); 

		// IRC PLAYER
		if (IS_IRC(ch)){
			SET_BIT(node->flags, LASTON_IRC);
			SET_BIT(node->flags, LASTON_HASUSEDIRC);	
		}else{
			REMOVE_BIT(node->flags, LASTON_IRC);
		}
	}

	// LOGGED PLAYER
	if (IS_SET(ch->act, PLR_LOG)){
		SET_BIT(node->flags, LASTON_LOGGED);
	}else{
		REMOVE_BIT(node->flags, LASTON_LOGGED);
	}

	// NOBLES
	if (IS_NOBLE(ch)){
		SET_BIT(node->flags, LASTON_NOBLE);
	}else{
		REMOVE_BIT(node->flags, LASTON_NOBLE);	
	}

	// LETGAINED
    if ( IS_LETGAINED(ch) ){
		SET_BIT(node->flags, LASTON_LETGAINED);
	}else{
		REMOVE_BIT(node->flags, LASTON_LETGAINED);	
	}

	// Automap	
    if ( USING_AUTOMAP(ch) ){
		SET_BIT(node->flags, LASTON_USING_AUTOMAP);
	}else{
		REMOVE_BIT(node->flags, LASTON_USING_AUTOMAP);	
	}

	// MSP
	if ( HAS_MSP( ch )){
		SET_BIT(node->flags, LASTON_USING_MSP);
	}else{
		REMOVE_BIT(node->flags, LASTON_USING_MSP);	
	}

	// ACTIVE
	if ( IS_ACTIVE(ch)){
		SET_BIT(node->flags, LASTON_ACTIVE);
	}else{
		REMOVE_BIT(node->flags, LASTON_ACTIVE);	
	}

	// Quester		
	if ( IS_SET(ch->act, PLR_QUESTER)){
		SET_BIT(node->flags, LASTON_QUESTER);
	}else{
		REMOVE_BIT(node->flags, LASTON_QUESTER);	
	}

	// Nsupport
	if ( IS_SET(ch->comm, COMM_NEWBIE_SUPPORT)){
		SET_BIT(node->flags, LASTON_NSUPPORT);
	}else{
		REMOVE_BIT(node->flags, LASTON_NSUPPORT);	
	}
	
	// Allowimmtalk
	if ( ch->pcdata->immtalk_name){
		SET_BIT(node->flags, LASTON_ALLOWIMMTALK);
	}else{
		REMOVE_BIT(node->flags, LASTON_ALLOWIMMTALK);	
	}
	
	// NO MAX KARN
	if ( IS_SET(ch->act, PLR_NOREDUCING_MAXKARN)){
		SET_BIT(node->flags, LASTON_NOMAXKARN);
	}else{
		REMOVE_BIT(node->flags, LASTON_NOMAXKARN);	
	}
	
	if(ch->pcdata){
		replace_string(node->email,ch->pcdata->email);
		replace_string(node->webpass, ch->pcdata->webpass);
	}

    replace_string(node->short_descr,ch->short_descr);

	if(!pload_based_update && ch->desc){
		// mccp 
#ifdef MCCP_ENABLED
		if(ch->desc->out_compress){
			SET_BIT(node->flags, LASTON_MCCP);
		}else{
			REMOVE_BIT(node->flags, LASTON_MCCP);
		}
#else
		REMOVE_BIT(node->flags, LASTON_MCCP);
#endif

		// mxp version
		if(!IS_NULLSTR(ch->desc->mxp_version)){
			replace_string(node->mxp_client_version,ch->desc->mxp_version);
		}else{
			replace_string(node->mxp_client_version,"");
		}
		// terminal type
		if(!IS_NULLSTR(ch->desc->terminal_type)){
			replace_string(node->terminal_type,ch->desc->terminal_type);
		}else{
			replace_string(node->terminal_type,"");
		}
	}

	if(!pload_based_update){
		// save laston data if it hasn't been saved recently
		if (LASTON_SAVE_DATA && laston_next_save < current_time){
			laston_save(NULL);
		}else{    // record system just started - set next save time
			laston_next_save = current_time + LASTON_SAVE_INTERVAL;
		}
	}

}

/**************************************************************************/
// returns either a pointer to the node or NULL
laston_data *find_node_from_id(int id)
{
	laston_data *node;
    
	// find them in the list
    for (node= laston_list; node; node=node->next)
    {
        if (node->player_id==id)
			return node;
    }
	return NULL;
}

/**************************************************************************/
void laston_update_char(char_data *ch)
{
    laston_data *node;
    char buf[MSL];

	if(!ch){
		bug("laston_update_char(): ch==NULL!!!");
		return;
	}

	// ploaded players are not recorded in laston
	if(ch->pload){
		return; 
	}

	node=find_node_from_id(ch->player_id);

    if (node){
        laston_update_node(node, ch, false); // update all the entries in the node 
    }else{
        sprintf(buf,"laston_update_char was passed a ch not in the laston_list (%s)\r\n"
            , ch->name);
        wiznet(buf,NULL,NULL,WIZ_BUGS,0,ADMIN); // put it on the bug channel 
        log_string( buf );
    }
}

/**************************************************************************/
// laston_login - written by Kalahn - July 1997 
// - records time when someone logs on
// - called from comm.c just before do_unread(ch,"");
void laston_login(char_data *ch) 
{
    laston_data *node=NULL;
    int ti; 

    // check if they are in the list 
    for (node= laston_list; node; node=node->next){
        if(node->player_id==ch->player_id){ // we have found our player already in list
			break;
		}
    }

    if (!node){ // we didn't find them - new addition to the laston database
        node = (laston_data *)malloc(sizeof(laston_data));
		memset( node, 0, sizeof(laston_data) );
        node->in_list = false;
        node->player_id=ch->player_id;
        node->index = 0;
        node->flags = 0;
		node->council=0;	
        node->name=str_dup("(unknown)");
        node->short_descr=str_dup("");
        node->email= str_dup("");
        node->webpass=str_dup("");
		node->mxp_client_version=str_dup("");
		node->terminal_type=str_dup("");

		node->login_room=0;
        for (ti=0; ti<LASZ;ti++) // initialise on and off values 
        {
            node->on[ti]=0;
            node->off[ti]=0;
            node->level[ti]=0;
            node->sublevel[ti]=0;
            node->host[ti]=str_dup("");
			node->ident[ti]=str_dup("");
        }

		// set if they are connecting from IRC first time
		if (IS_IRC(ch)){
			SET_BIT(node->flags, LASTON_FIRSTIRC);
		}
    }
    else // we found them in the list 
    {
        ++node->index%=LASZ; //get next index mod LASZ
        // remove from their current place in the list 
        laston_remove_list(node);
    }
	node->deleted_date=0;
    node->on[node->index] = current_time; // record the logon time 

    laston_add_list(node, laston_list); // add them to the head of the list 

    laston_update_node(node,ch, false); // update all the entries in the node 
	
	// load their council from laston
	ch->pcdata->council= node->council;	
}	

/**************************************************************************/
//  laston_logout - written by Kalahn - July 1997       
//  - records logout time in the laston database
//  - called from handler.c in extract_char()
void laston_logout(char_data *ch) 
{
    laston_data *node, *last_online=NULL, *found_node=NULL;

    if (IS_NPC(ch))
        return;

    // find them in the list 
    for (node= laston_list; node; node=node->next)
    {
        if (IS_SET(node->flags, LASTON_ONLINE)) // record the laston online
            last_online = node;

		// skip to next record if we don't have a match
		if (node->player_id!=ch->player_id){
			continue;
		}
			
		// we have found our player in the list      

        found_node = node;
        node->off[node->index] = current_time; // record when they log off
        if (node->off[node->index]<=node->on[node->index])
            node->off[node->index]++;          // incase system clock is reset
        node->level[node->index] = ch->level;  // record logoff level

        node->trust = ch->trust;
        node->clan = ch->clan;
        node->clanrank = ch->clanrank;
        node->security = ch->pcdata->security; // olc security 
        REMOVE_BIT(node->flags, LASTON_ONLINE); // no longer online			
		node->council = ch->pcdata->council;	// record their council

		// record their perm_pkilled status
		if (ch->pcdata->karns<0){
			SET_BIT(node->flags, LASTON_PERM_PKILLED); // permkilled
		}else{
			REMOVE_BIT(node->flags, LASTON_PERM_PKILLED); // not permkilled
		}
    }

    // move them to the first offline place in the list if required 
    if ( last_online && found_node && (last_online!=found_node) )
    {
        laston_remove_list(found_node);
        if (last_online->next)
            laston_add_list(found_node, last_online->next); 
        else
        {   // shuffle around to get them at the bottom 
            laston_add_list(found_node, last_online);
            laston_remove_list(last_online);
            laston_add_list(last_online, found_node);
        }
    }; 

    // save laston data if it hasn't been saved recently 
    if (laston_next_save < current_time){
        laston_save(NULL);
	}
}

/**************************************************************************/
//  laston_delete - written by Kalahn - July 1997       
//  - removes someone from the laston database
void laston_id_delete(long id)
{  
    laston_data *node=NULL;

    int ti;
    // find them in the list
    for (node= laston_list; node; node=node->next )
    {   
        if (node->player_id==id) break;
    }

    if (node) // They are in the list
    {
		// create the deleters log
		{
			char buf[MIL], buf2[MIL];
			int tempindex, newindex;

			sprintf ( buf, "DELETED CHARACTER REMOVAL: %s %s %s %s", 
				node->name,
                (node->clss > -1?class_table[node->clss].short_name:""),
                (node->race > -1?race_table[node->race]->name:""),
                 shortdate(NULL)
				 );
			append_datetimestring_to_file( LASTON_DELETE_LOGFILE, buf);
			
			// display clan info
			if (node->clan>0){
		        sprintf(buf,"`xclan: %s `xrank: %s (%d)`x",
					node->clan->cwho_name(),
					node->clan->clan_rank_title(node->clanrank),
					node->clanrank);              
				append_datetimestring_to_file( LASTON_DELETE_LOGFILE, buf);
			}

			sprintf ( buf, "Noble: %s  Created: %s", 
				(IS_SET(node->flags, LASTON_NOBLE)?"`#`RYES`^":"no"),
				ctime( (time_t *) &node->player_id));
			append_datetimestring_to_file( LASTON_DELETE_LOGFILE, buf);

			sprintf ( buf, "Email: %s\r\n", node->email);
			append_datetimestring_to_file( LASTON_DELETE_LOGFILE, buf);

            // characters times and levels 
            for (tempindex=node->index+LASZ; tempindex > node->index; tempindex--)
            {
                newindex = tempindex % LASZ;
                if (node->on[newindex]!=0) // display only valid records 
                {
                    strcpy(buf2, ctime((time_t *) &node->on[newindex]));
                    buf2[20] = '\0'; //remove year 

                    if (node->off[newindex]<=node->on[newindex])
                    {
                        sprintf ( buf, "on: %s\nonline for %s (lvl %d) [%s]",
							buf2,
                            short_timediff(node->on[newindex] , current_time),
                            node->level[newindex],                         
                            node->host[newindex]
							);
						append_datetimestring_to_file( LASTON_DELETE_LOGFILE, buf);
                    }
                    else
                    {
                        sprintf ( buf, "on: %s %s\nwas on for %s (left level %d) [%s]",
							buf2,
                            short_timediff(node->on[newindex], node->off[newindex]),
                            short_timediff(node->on[newindex], current_time),
                            node->level[newindex],                         
                            node->host[newindex]
							);
						append_datetimestring_to_file( LASTON_DELETE_LOGFILE, buf);
                    }
                } // display valid data
            }  // end of loop thru a characters LASZ times 
        }

        // remove them from the list
        laston_remove_list(node);

        // deallocate memory for hosts
        for (ti=0; ti<LASZ;ti++){
			if(!IS_NULLSTR(node->host[ti])){
				free_string(node->host[ti]); 
			}
		}

        // deallocate memory
        free(node);
    }

	// resort the laston toproleplayers list
	resort_top_roleplayers();

	// resort the laston topwealth list
	resort_top_wealth();
}

/**************************************************************************/
void laston_delete(char_data * ch)
{
    laston_id_delete(ch->player_id);
    laston_save(ch);
}
/**************************************************************************/
// Instead of deleting players, record the date they deleted on... then
// laston can drop them out of the database depending on their level.
// gamesettings record how long to keep deleted players in laston.
void laston_player_deleting(char_data * ch)
{
    // find them in the list
    laston_data *node=NULL;
    for (node= laston_list; node; node=node->next )
    {   
        if (node->player_id==ch->player_id) break;
    }

	if(!node){
		bugf("laston_player_deleting(): Couldn't find player '%s' in the laston "
			"database to mark as deleted.", ch->name);
		return;
	}
	node->deleted_date=current_time;
}


/**************************************************************************/
// laston_mortal - written by Kalahn - Sept 1997
// - displays the basic laston data for a mortal.
void laston_mortal( char_data *ch, char *argument )
{
    extern time_t boot_time;
    BUFFER *output;
    char buf[MSL];
    char arg[MIL];
    sh_int newindex=0, ntotal, displaycounter;
    bool buffer_overflow=false;
    
    laston_data *node;

    // Morts can't get a full listing
    // - so if they didn't specify a parameter,  send them back
    if(GAMESETTING3(GAMESET3_MORTLASTON_REQUIRES_PART_OF_NAME) && IS_NULLSTR(argument)){
        ch->println("You must type part of a characters name");
        return; // exit with doing anything 
    }

    argument = one_argument( argument, arg );

    // Display a message telling roughly how current the data is
    ch->printlnf("%s has been up for %s",
             MUD_NAME, timediff(boot_time, current_time));
    ch->printlnf("The current time is %sLaston records date back %s.",
             (char *) ctime( &current_time ),
             timediff(laston_since, current_time));
    ch->println("Showing upto the ten most recent matches");
	if(GAMESETTING3(GAMESET3_MORTLASTON_REQUIRES_FULL_IMM_NAME)){
		ch->println("Note: Immortals do not show up on this list unless you spell their full name.");
	}

    // setup a buffer for info to be displayed 
    output = new_buf();  

    // reset counters 
    ntotal=0; // node total counter 
    displaycounter=0; // counter of number about to displayed 

    // loop thru all character records in linked list
    for (node= laston_list; node; node=node->next)
    {
        newindex= node->index;

        if (displaycounter>10) // display only last 10 characters
            continue;

        if (!str_prefix(arg, node->name))
        {
            if (ch->player_id==node->player_id){ // check if lastoning themselves
                sprintf ( buf, "%-12s (you) have been online for %s.\r\n",
                       node->name, timediff(node->on[newindex] , current_time));
            }else{
				// morts can't see deleted players in laston
				if(node->deleted_date){
					continue;
				}

				// those that have abused laston can't see others within 2 days
				if (IS_SET(ch->comm,COMM_REDUCED_LASTON) && 
					(IS_SET(node->flags, LASTON_ONLINE) ||
                    (node->off[newindex]> current_time-3600*48)))
				{
                    sprintf ( buf, "%-12s is currently online or has been within the last 2 days.\r\n",
                                    node->name);
				}
				// those who have CONFIG2_ACCURATE_LASTON_TIMES set
				else if(HAS_CONFIG2(ch, CONFIG2_ACCURATE_LASTON_TIMES)){
					if ( IS_SET(node->flags, LASTON_ONLINE) ){
						sprintf ( buf, "%-12s is currently online.\r\n", node->name);			
					}else{                   
						sprintf ( buf, "%-12s laston %s ago\r\n",
							node->name, timediff(node->off[newindex] , current_time));
					}
				}
				// handle viewing of immortal characters
				else if(node->level[node->index]>=LEVEL_IMMORTAL) 
				{
					// gotta do exact name match for an imm
					if(GAMESETTING3(GAMESET3_MORTLASTON_REQUIRES_FULL_IMM_NAME) 
						&& str_cmp(arg, node->name)){
						continue;
					}
					if(GAMESETTING3(GAMESET3_MORTLASTON_REDUCED_LASTDAY_ON_IMMS)
						&& (node->off[newindex]>current_time-3600*24
						|| IS_SET(node->flags, LASTON_ONLINE)) )
					{
						sprintf ( buf, "%-12s is currently online or has been within the last day.\r\n",
                                    node->name);
					}					
					// immortal is currently online
					else if ( IS_SET(node->flags, LASTON_ONLINE) ){
						sprintf ( buf, "%-12s is currently online.\r\n", node->name);			
					}
					// they aren't online
					else{
						sprintf ( buf, "%-12s laston %s ago\r\n", node->name, 
							timediff(node->off[newindex] , current_time));
					}				
				}
				// if we have a 2 hour reduced view of mortals 
				// and they are on or have been within the last 2 hours
				// show the reduced info
                else if ( GAMESETTING3(GAMESET3_MORTLASTON_REDUCED_TO2HOURS_ON_MORTS)
					&& ( node->off[newindex]> current_time-7200 
						 || IS_SET(node->flags, LASTON_ONLINE) ) 
					)
                {
                    sprintf ( buf, "%-12s is currently online or has been within the last 2 hours.\r\n",
                                    node->name);
                }
				// they are currently online
                else if ( IS_SET(node->flags, LASTON_ONLINE) ){
                    sprintf ( buf, "%-12s is currently online.\r\n", node->name);			
				}
				// they aren't online
                else
                {                   
                    sprintf ( buf, "%-12s laston %s ago\r\n",
                        node->name, timediff(node->off[newindex] , current_time));
                }
            }

            if (!add_buf( output, buf)){
                buffer_overflow=true;
            }else{
                displaycounter++;
			}
        }

    }
     // tell mortal that they couldn't find anyone with that name 
    if (displaycounter==0) ch->println("No one found with that name.");

    if (buffer_overflow){
        ch->println("Unable to display all laston data, due to the amount requested.");
	}

	ch->sendpage(buf_string(output));
    free_buf(output);
    return;
}
/**************************************************************************/
void do_councillist( char_data *ch, char *argument )
{
    laston_data *node;
    BUFFER *output; 
	char buf[MSL];
	int count=0;

	char arg[MIL];
	argument = one_argument( argument, arg );

	output = new_buf();  

	sprintf( buf,"`?`#`^%s`x", format_titlebar("COUNCIL LIST`^"));
	add_buf( output, buf);

	for (node= laston_list; node; node=node->next)
	{
		if (node->level[node->index]<LEVEL_IMMORTAL
			  && node->trust<LEVEL_IMMORTAL)
			continue;	

		if (node->level[node->index]>=ADMIN
			  || node->trust>=ADMIN)
			continue;	

		sprintf( buf, "`%c %-13s `G-`Y %s`x\r\n", 
			node->council?'W':'x',
			node->name,
			node->council?flag_string( council_flags, node->council):"`cnone");

		if (!is_name( arg, buf)){
			continue;
		}
    
		add_buf( output, buf );
		count++;
	}
	sprintf( buf, "%d immortal record%s displayed.\r\n", count, count==1?"":"s");
	add_buf( output, buf );

	ch->sendpage(buf_string(output));
	free_buf(output);
}
/**************************************************************************/
void do_lastoncouncil( char_data *ch, char *argument )
{
    laston_data *node;
    BUFFER *output; 
	char buf[MSL];
	int count=0;

	if(IS_NULLSTR(argument)){
		output = new_buf();  
		add_buf( output, "name.  [`Mlevel, `Btrust, `Rsecurity`x]- councils\r\n");
    
		for (node= laston_list; node; node=node->next)
		{
			if((node->level[node->index]<LEVEL_IMMORTAL
				  && node->trust<LEVEL_IMMORTAL)
				  || node->deleted_date){
				continue;	
			}

			sprintf( buf, "`%c%-13s `S[`M%3d`S,`%c%3d`S,`R%d`S]`G- `Y%s`x\r\n", 
				node->council?'W':'x',
				node->name,
				node->level[node->index],	
				node->level[node->index]<node->trust?'C':'B',
				node->trust,
				node->security,
				node->council?flag_string( council_flags, node->council):"`cnone");
        
			add_buf( output, buf );
			count++;
		}
		sprintf( buf, "%d immortal record%s displayed.\r\n", count, count==1?"":"s");
		add_buf( output, buf );

		ch->sendpage(buf_string(output));
		free_buf(output);
	}else{ // modify a councils settings on a player
		int council;
		char name[MIL];

		// split and check the parameters
		argument=one_argument(argument, name);
		council = flag_lookup(argument,council_flags);
					
		if ( council == NO_FLAG)
		{
			ch->printlnf("'%s' is not a valid council.",argument);
			ch->println("syntax: lastoncouncil                     - shows the council info for imms");
			ch->println("syntax: lastoncouncil <immname> <council> - toggles the council flag for immname");
			ch->printlnf(" Valid councils are: %s", flag_string( council_flags, -1));
			return;
		}

		// find the player
		for (node= laston_list; node; node=node->next)
		{	
			// immortals only
			if (node->level[node->index]<LEVEL_IMMORTAL
				  && node->trust<LEVEL_IMMORTAL)
				continue;	

			if(!str_cmp(name,node->name)){
				TOGGLE_BIT( node->council, council);
				ch->printlnf("Council toggled on %s.", node->name );
				ch->printlnf("now are: %s", flag_string( council_flags, node->council ));
				
				// update the player's council if they are online
				{
					char_data *wch;
					for ( wch = player_list; wch; wch = wch->next_player )
					{
						// must have an exactly matching name
						if (str_cmp(name, wch->name))
							continue;

						// corruption check
						if(wch->player_id!=node->player_id){
							ch->println("do_lastoncouncil(): BUG!!! victim->player_id!=node->player_id");
							bugf("do_lastoncouncil(): BUG!!! victim->player_id!=node->player_id, name=%s\r\n", 
								wch->name);
						}else{
							wch->pcdata->council=node->council;
							wch->println("`?YOUR COUNCILS HAVE BEEN UPDATED!!!`x");
							wch->println("`?YOUR COUNCILS HAVE BEEN UPDATED!!!`x");
							wch->println("`?YOUR COUNCILS HAVE BEEN UPDATED!!!`x");
							wch->printlnf("They are now: %s", flag_string( council_flags, wch->pcdata->council ));
						}
					}
				}
				return;
			}
		}

		ch->printlnf("Couldn't find any immortal character called '%s'", name);
		ch->println("syntax: lastoncouncil                     - shows the council info for imms");
		ch->println("syntax: lastoncouncil <immname> <council> - toggles the council flag for immname");
		ch->printlnf(" Valid councils are: %s", flag_string( council_flags, -1));
	}
}
/**************************************************************************/
int wild_match(register unsigned char *m, register unsigned char *n);
/**************************************************************************/
// return false on a match
bool do_host_filter(laston_data *node, char* host_filter_substring)
{
    int i;

    for (i=0; i<LASZ;i++)
    {
		if(IS_NULLSTR(node->host[i]))
			continue;

		// check the hostname
		if (wild_match((unsigned char*)host_filter_substring, (unsigned char*)node->host[i])
			|| !str_infix(host_filter_substring,node->host[i]))
		{
            return false;
		}

		// check the ip
		if (wild_match((unsigned char*)host_filter_substring, (unsigned char*)node->ip[i])
			|| !str_infix(host_filter_substring,node->ip[i]))
		{
            return false;
		}
    }
    return true;
}

/****************************************************************************
 *  do_laston_level_one_pwipe - written by Kalahn - Sept 1997   *
 *   - wipes all level 1 pfiles older than 1 week old                      *
 ***************************************************************************/
void do_laston_level_one_pwipe(char_data *ch)
{
    laston_data *node;
	laston_data *node_next;
    char buf[MSL];
    int ni;
	int count=0;
	
    for (node= laston_list; node; node=node_next)
    {
		node_next=node->next;
        ni= node->index;
        if (node->level[ni]==1 && !node->deleted_date && ((node->off[ni]+(60*60* 24 *7)) < current_time ))
        {
            count++;
            ch->printlnf("%3d) LASTON - Deleting old newbie file %s ", 
				count, node->name);

			// find the pfile and remove it
			PFILE_TYPE pt=find_pfiletype(node->name);
			if(pt==PFILE_NONE){
				ch->printlnf("ERROR: a pfile for %s wasnt found!", node->name);
				logf("do_laston_level_one_pwipe(): ERROR - a pfile for %s wasnt found!", node->name);
				continue;
			}
			
			if(pt==PFILE_MULTIPLE){
				ch->printlnf("ERROR: Multiple pfiles for %s were found!", node->name);
				logf("do_laston_level_one_pwipe(): ERROR - Multiple pfiles for %s were found!", node->name);
				continue;
			}

			strcpy( buf, pfilename(node->name, pt));

			logf("do_laston_level_one_pwipe():[%3d] removing level 1 pfile %s from %s", 
				count, node->name, buf);
            unlink(buf);
            laston_id_delete(node->player_id); // remove them from the laston list 
        }
    } // next node in list 
}
/***************************************************************************/
void show_immortal_laston_help( char_data *ch)
{
    ch->printf("LASTON: Immortal options:\r\n"
		" -?                  - display this help\r\n"
		" -a                  - all laston records\r\n"
		" -b                  - brief format\r\n"
		" -c                  - display class in short listing\r\n"
		" -clan=<clan_name>   - for members of a specific clan\r\n"
		" -clan=all           - show all players in a clan\r\n"
		" -class=<class_name> - for players of a specific class\r\n"
		" -d                  - show deleted players.\r\n");


	ch->println(" -l=<id>             - lower ID");
	ch->println(" -m=<id>             - max ID");

	if(IS_TRUSTED(ch,ADMIN-1)){
		ch->println(" -e                  - display players email address, if we know it.");
		ch->println(" -e=email_filter     - display all players that match the email filter.");
		ch->println(" -ip                 - show ip address as well as hostname");
	}
	ch->println(
		" -f                  - full logins info on players (all recorded logins)\r\n"
		" -i                  - immortal/trusted characters only\r\n"
		" -irc                - player has used irc\r\n"
		" -n                  - noble characters only\r\n"
		" -o                  - olc builder\r\n"
		" -r                  -  race"
		" -race=<race_name>   - for players of a specific race\r\n"
		" -t                  -  times, instead of time differences\r\n"
		" -w                  - wide format\r\n"
		" -x                  -  number entries\r\n"
		"====-       restrict query by level      -====\r\n"
		"    <upperlevel      - less than upperlevel\r\n"
		"    >lowerlevel      - greater than lowerlevel\r\n"
		"    (e.g. laston >10 <20)");

	if (IS_TRUSTED(ch,GUARDIAN)){
        ch->println(" -h=<host_mask_filter>  (search on their login host).");
	}
    if (IS_TRUSTED(ch,IMPLEMENTOR)){
        ch->println(" -z!!!               - wipe all level 1 pfiles older than one week old.");
	}
    return;
}


/****************************************************************************
 *  laston_immortal - written by Kalahn - Sept 1997   *
 *   - process immortal laston request                                      *
 ***************************************************************************/
void laston_immortal( char_data *ch, char *argument )
{
    extern time_t boot_time;
    BUFFER *output;
    laston_data *node= NULL;
    char buf[MSL];
    char working_arg[MIL];
    char buf2[MSL];
    char buf3[MIL];
    char host_filter_substring[MIL];
    char email_filter_substring[MIL];
    char *parse=NULL;
    sh_int newindex=0, ntotal, displaycounter, linenum;
    int tempindex;
    bool buffer_overflow=false;
	int count;

    int greater_than_level=0;
    int less_than_level=MAX_LEVEL+2;
    char name_prefix[MIL];

    bool laston_summary=true;
    bool laston_summary_specified=false;
    
    // default parameters
    bool laston_show_first_x=true;
    int  laston_show_first_x_amount=ch->lines-4; /* default to one page */

	bool laston_email=false;
	bool laston_email_filter= false;

    bool laston_imm_only= false;
    bool laston_host_filter= false;
	bool laston_show_deleted=false;

	bool laston_wide=false; // show the seconds online, wide time, and ip
	bool laston_show_ip=false; // show the ip address

    bool laston_show_race= false;
    bool laston_show_class= false;
	bool laston_show_times=false;
    bool laston_clan_filter= false;
    bool laston_number_entries= false;
    bool laston_show_allclans_only= false;
	bool laston_olc_builder= false;
	bool laston_irc_only=false;
    bool laston_no_args;
	short laston_class_filter=-1;
	short laston_race_filter=-1;

    bool laston_noble_only= false;
    CClanType *clan_filter=NULL;

	// filter on creation id's
	time_t laston_lower_id_value=0;
    bool laston_lower_id= false;
	time_t laston_max_id_value=0;
    bool laston_max_id= false;

	long total_bank;
	long total_gold;
	long total_silver;

    if (laston_show_first_x_amount<1){
        laston_show_first_x_amount=25;
	}

    name_prefix[0]='\0';

    if (IS_NULLSTR(argument)){
        laston_no_args = true;
    }else{
        laston_no_args = false;
	}

    // parse all parameters 
    for (;;)
    {
        argument = one_argument( argument, working_arg );

        if (working_arg[0] == 0)
            break;

        switch (working_arg[0])
        {
            case '>':
                parse = &working_arg[0];
                parse++;
                if (is_number(parse))
                {
                    greater_than_level = atoi(parse);
                }
                else
                    ch->printlnf("Invalid parameter '%s' (should be numeric)", working_arg);
                break;
    
            case '<':
                parse = &working_arg[0];
                parse++;
                if (is_number(parse))
                {
                    less_than_level = atoi(parse);
                }
                else
                    ch->printlnf("Invalid parameter '%s' (should be numeric)", working_arg);
                break;
    
            case '-':
                parse = &working_arg[0];
                parse++;
                switch (parse[0])
                {
                    case '?':
                        show_immortal_laston_help(ch);
                        return;
                    break;

                    case '!':
                        GIOSAVE_LIST(laston_list, laston_data, LASTON_BACKFILE, true);
                        ch->printlnf("laston data saved to %s", LASTON_BACKFILE);
                        logf("laston data manually saved by %s to %s\r\n",
                            ch->name, LASTON_BACKFILE);
                        return;
                        break;


                    case 'a': laston_show_first_x= false;
                    break;

                    case 'b': laston_summary= true; laston_summary_specified=true;
                    break;                       

                    case 'c':
                            if (parse[1]=='\0') // show classes in brief search
                            {
                                laston_show_class=true;
                                break;
                            }else if (!str_prefix("clan=", parse)){
                            	// support -clan= clan searches
	                            if (laston_clan_filter)
								{
									ch->println("You can't filter 2 clans at the same time.");
								}
								parse+=str_len("clan=");	                            
								if (!str_cmp(parse, "all"))
								{
									laston_show_allclans_only= true;
								}else{ // do a specific clan filter
									clan_filter= clan_nlookup(parse);
									if(!clan_filter){
										ch->printlnf("Laston -clan= filter error: No such clan '%s' exists.",parse);
										return;
									}
								}
								laston_clan_filter= true;
                            }else if (!str_prefix("class=", parse)){
								// support -class= class searches
	                            if (laston_class_filter>-1)
								{
									ch->println("You can't filter 2 classes at the same time.");
								}
								parse+=str_len("class=");

								laston_class_filter=class_lookup(parse);
								if(laston_class_filter<0){
									ch->printlnf("Laston -class= filter error: No such class '%s' exists.",parse);
									return;
								}
                            }else{
                                ch->printlnf("LASTON SYNTAX ERROR: Unrecognised option '-%s'",parse);
								ch->printf("To do a clan search use  `=C-clan=`S<clanname>`x for a specfic clan\r\n"
										   "                     or  `=C-clan=all`x for all clans.\r\n");
								ch->println("To do a class search use `=C-class=`S<classname>`x for a specfic class");
                                return;
							}
                    break;

                    case 'd': laston_show_deleted=true;
                    break;

                    case 'e': 
                        if (IS_TRUSTED(ch,ADMIN-1))
                        {                      
                            parse++;
                            if (parse[0]!='='){
								if(parse[0]!='\0'){								
									ch->println("LASTON SYNTAX ERROR: incorrect email filter syntax!  "
										"(Should be like -e=someone@somewhere.com or -e=*@*aol.net, or just -e to show all chars with emails)");
									return;
								}else{
									laston_email= true;
								}
                            }else{ // an email search
								if (laston_email_filter)
								{
									ch->println("You can't filter 2 email addresses at the same time.");
									return;
								}

								parse++;
								strcpy(email_filter_substring,parse);
								laston_email_filter= true;
								laston_email= true;
							}
                        }				
                    break;                       

                    case 'f': laston_summary= false; laston_summary_specified=true;
                    break;                       

                    case 'h': 
                        if (IS_TRUSTED(ch,GUARDIAN))
                        {                      
                            laston_show_first_x= false;
                            parse++;
                            if (parse[0]!='=')
                            {
                                ch->println("LASTON SYNTAX ERROR: incorrect host filter syntax!  "
									"(Should be like -h=ipx.com or -h=*cust??.*col*.uu.net)");
                                return;
                            }
                            if (laston_host_filter)
                            {
                                ch->println("You can't filter 2 hosts at the same time.");
                            }
                            parse++;
                            strcpy(host_filter_substring,parse);
                            laston_host_filter= true;
                        }
                    break;

                    case 'i': 
						if (parse[1]=='\0') // show classes in brief search
                        {
							laston_imm_only= true; laston_show_first_x= false;
                            break;
                        }else if (!str_cmp("irc", parse)){
							laston_irc_only=true;
							break;
						}else if (!str_cmp("ip", parse)){
							laston_show_ip= true;
							break;
						}else{
                            ch->printlnf("LASTON SYNTAX ERROR: Unrecognised option '-%s'",parse);
							ch->wrapln("To do search for irc players use `=C-irc`x, "
								"use `=C-ip`x to show ip addresses and `=C-i`x to "
								"show only imms.");
                            return;
						}
                    break;

                    case 'l': 
                        parse++;
                        if (parse[0]!='=')
                        {
                            ch->println("LASTON SYNTAX ERROR: incorrect lower ID filter syntax!  "
								"(Should be like -l=957420642)");
							ch->println("The ID relates to when they created - seconds past 1Jan1970 - see the memory command)");
                            return;
                        }
                        if (laston_lower_id)
                        {
                            ch->println("You can't have two lower filters");
							return;
                        }
                        parse++;
						if(!is_number(parse)){
							ch->println("LASTON SYNTAX ERROR: incorrect lower ID filter number!  "
								"(Should be like -l=957420642)\r\n"
								"The ID relates to when they created - seconds past 1Jan1970 - see the memory command)");
                            return;
						}
						laston_lower_id_value=(time_t)atoi(parse);
                        laston_lower_id= true;
                    break;

                    case 'm': 
                        parse++;
                        if (parse[0]!='=')
                        {
                            ch->println("LASTON SYNTAX ERROR: incorrect max ID filter syntax!  "
								"(Should be like -m=987420642)\r\n"
								"The ID relates to when they created - seconds past 1Jan1970 - see the memory command)");
                            return;
                        }
                        if (laston_max_id)
                        {
                            ch->println("You can't have two max filters");
							return;
                        }
                        parse++;
						if(!is_number(parse)){
							ch->println("LASTON SYNTAX ERROR: incorrect max ID filter number!  "
								"(Should be like -l=987420642)\r\n"
								"The ID relates to when they created - seconds past 1Jan1970 - see the memory command)");
                            return;
						}
						laston_max_id_value=(time_t)atoi(parse);
                        laston_max_id= true;
                    break;

                    case 'n': laston_noble_only= true;
                    break;

					case 'o': laston_summary= true; laston_olc_builder= true;
                    break;                       

                    case 'r': 
                        if (parse[1]=='\0') // show races in brief search
                        {
                            laston_show_race=true;
                            break;
                        }else if (!str_prefix("race=", parse)){
							// support -race= race searches
	                        if (laston_race_filter>-1)
							{
								ch->println("You can't filter 2 races at the same time.");
							}
							parse+=str_len("race=");

							laston_race_filter=pcrace_lookup(parse);
							if(laston_race_filter<0){
								ch->printlnf("Laston -race= filter error: No such pc race '%s' exists.",parse);
								return;
							}
                        }else{
                            ch->printlnf("LASTON SYNTAX ERROR: Unrecognised option '-%s'",parse);
							ch->println("To do a race search use `=C-race=`S<racename>`x for a specfic race");
                            return;
						}
                    break;

                    case 't': laston_show_times=true;
                    break;

					case 'w': 
						laston_wide=true;
						laston_show_ip=true;
                    break;

                    case 'x': laston_number_entries= true; 						
                    break;

                    case 'z': 
                        if (IS_TRUSTED(ch,IMPLEMENTOR))
                        {                      
                            if (parse[1]=='!' && parse[2]=='!' && parse[3]=='!')
                            {
                                ch->println("LASTON LEVEL 1 - 1 week old pfile wipe!!!");
                                do_laston_level_one_pwipe(ch);
                            }
                            return;
                        }
                    break;                       
                }
                break;
        
                case '?':
                    show_immortal_laston_help(ch);
                    return;
                break;

            default:
                strcpy (name_prefix, working_arg);
                laston_show_first_x= false;
                break;
        }
		if(IS_SET(ch->dyn,DYN_IMMLASTON)){
			if(working_arg[0]=='`'){
				last_resave=laston_next_save;
				return;
			}
		}
    }

    // if there is no name prefix default to brief 
    if (!laston_summary_specified)
    {
        if (IS_NULLSTR(name_prefix)){
            laston_summary= true;			
        }else{
            laston_summary= false;
			laston_show_class=true;
			laston_show_race=true;
		}
    }

    // Display a message telling how current the data is
    ch->printlnf("%s has been up for %s",
             MUD_NAME, timediff(boot_time, current_time));
    ch->printlnf("The current time is %sLaston records date back %s.",
             (char *) ctime( &current_time ),
             timediff(laston_since, current_time));

    // setup a buffer for info to be displayed 
    output = new_buf();  
    displaycounter=0; // counter of amount of matches displayed 
	linenum= 0;
	count=0;
	char clanname_buffer[MIL];

    if (laston_no_args){
        ch->printlnf("No filter parameters specified, showing the last %d logins.",
            laston_show_first_x_amount);
    }else{
        sprintf(buf,"Laston records being filtered with the following settings:\r\n");
        add_buf( output, buf );

        if (laston_show_first_x)
        {
            sprintf(buf,"- Show the most recent %d matching characters.\r\n",
                    laston_show_first_x_amount);
            add_buf( output, buf );
			linenum++;
        }else{
            sprintf(buf,"- Search thru and display all the matching laston records.\r\n");
            add_buf( output, buf );
			linenum++;
        }

        if (name_prefix[0] != 0)
        {
            sprintf(buf,"- Where the characters name starts with '%s'.\r\n",
                name_prefix);
            add_buf( output, buf );
			linenum++;
        }
		
        if (greater_than_level)
        {
            sprintf(buf,"- Characters with level greater than %d.\r\n", greater_than_level);
            add_buf( output, buf );
			linenum++;
        }

        if (less_than_level<MAX_LEVEL+2)
        {
            sprintf(buf,"- Characters with level less than %d.\r\n", less_than_level);
            add_buf( output, buf );
			linenum++;
        }

		if(laston_class_filter>-1){
            add_buf( output, "- Where characters class is ");
            add_buf( output, class_table[laston_class_filter].name);
			add_buf( output, "\r\n");
			linenum++;
			laston_show_class=true;
		}

		if(laston_lower_id){
            sprintf(buf,"- ID is greater or equal to %d.\r\n", (int)laston_lower_id_value);
            add_buf( output, buf );
			linenum++;
		}
		if(laston_max_id){
            sprintf(buf,"- ID is less than or equal to %d.\r\n", (int)laston_max_id_value);
            add_buf( output, buf );
			linenum++;
		}

		if(laston_race_filter>-1){
            add_buf( output, "- Where characters race is ");
            add_buf( output, race_table[laston_race_filter]->name);
			add_buf( output, "\r\n");
			linenum++;
			laston_show_race=true;
		}		

		if (laston_olc_builder)
        {           
            add_buf( output, "- Where the characters has OLC access.\r\n");
			linenum++;
        }

		if (laston_noble_only)
        {           
            add_buf( output, "- Where the characters is a noble\r\n");
			linenum++;
        }

        if (laston_imm_only)
        {
            sprintf(buf,"- Immortal characters only.\r\n");
            add_buf( output, buf );
			linenum++;
        }

		
		if (laston_wide)
		{
            sprintf(buf,"- Wide format.\r\n");
            add_buf( output, buf );
			linenum++;
		}

		if (laston_show_ip)
		{
            sprintf(buf,"- Show IP address in addition to hostname.\r\n");
            add_buf( output, buf );
			linenum++;
		}

        if (laston_irc_only)
        {
            sprintf(buf,"- Where character has logged on via the irc port at least once.\r\n");
            add_buf( output, buf );
			linenum++;
        }
		
        if(laston_number_entries){
            add_buf( output, "- Numbering each character entry.\r\n");
			linenum++;
        }

		if(laston_show_deleted){
            add_buf( output, "- Showing deleted players.\r\n");
			linenum++;
		}

        if(laston_host_filter){
            sprintf(buf,"- Where the players login host or ip matches or contains the mask '%s'.\r\n",
                host_filter_substring);
            add_buf( output, buf );
			linenum++;
        }

        if (laston_email_filter){
            sprintf(buf,"- Where the players email address matches or contains the mask '%s'.\r\n",
                email_filter_substring);
            add_buf( output, buf );
			linenum++;
        }

        if ( laston_clan_filter)
        {
            if (laston_show_allclans_only)
            {
                sprintf(buf,"- Where the character is a member of a clan.\r\n");
                add_buf( output, buf );
				linenum++;
            }
            else
            {
                sprintf(buf,"- Where the character is a member of the clan %s`x.\r\n",
					clan_filter->cname());
                    add_buf( output, buf );
				linenum++;
            }
        }
    }
    sprintf(buf,"\r\n");
    add_buf( output, buf );

    // reset counters 
    ntotal=0; // node total counter 
	total_bank=0;
	total_gold=0;
	total_silver=0;
    clanname_buffer[0]='\0';

    // loop thru all laston records in linked list
    for (node= laston_list; node; node=node->next)
    {
        ntotal++;

		if (output->state == BUFFER_OVERFLOW)
		{
			buffer_overflow=true;
		}

        if (laston_show_first_x && (laston_show_first_x_amount<linenum))
            continue;

        if (node->level[node->index]<=greater_than_level)
            continue;
		
		if (laston_olc_builder && node->security==0)
            continue;
		
        if (node->level[node->index]>=less_than_level)
            continue;

		if(laston_class_filter>-1 && laston_class_filter!=node->clss)
			continue;

		if(laston_race_filter>-1 && laston_race_filter!=node->race)
			continue;		

		if(laston_lower_id && node->player_id<laston_lower_id_value){
			continue;
		}
		if(laston_max_id && node->player_id>laston_max_id_value){
			continue;
		}

		if(laston_irc_only 
			&& !IS_SET(node->flags, LASTON_IRC)
			&& !IS_SET(node->flags, LASTON_HASUSEDIRC)){
			continue;
		}

        if (laston_imm_only
              && node->level[node->index]<LEVEL_IMMORTAL
              && node->trust<LEVEL_IMMORTAL)
            continue;

		if (laston_noble_only 
			&& !IS_SET(node->flags, LASTON_NOBLE))
			continue;

        if ( laston_host_filter){
            if (do_host_filter(node,host_filter_substring)){
                continue;
			}
		}

		if(node->deleted_date && !laston_show_deleted){
			continue;
		}

	    if ( laston_email_filter ){
			if(IS_NULLSTR(node->email)){
				continue;
			}
            if (!wild_match((unsigned char*)email_filter_substring, (unsigned char*)node->email)
				&& str_infix(email_filter_substring, node->email)){
                continue;
			}
		}


        if ( laston_clan_filter)
        {
            if (laston_show_allclans_only)
            {
                if (!node->clan)
                    continue;
            }
            else
            {
                if (node->clan!=clan_filter)
                    continue;
            }
        }

		// email only check
		if ( laston_email && IS_NULLSTR(node->email)){
			continue;
		}

		// the name match check 
		if (name_prefix[0] != 0 && str_prefix(name_prefix, node->name)){
            continue;
		}
	
        if (laston_show_allclans_only){
            sprintf(clanname_buffer,"%s[%d]`x ",
                node->clan->cwho_name(),
				node->clanrank);
        }

		// get the colour of the entries name
		char name_colour[3];
		if(node->deleted_date){
			strcpy(name_colour, "`S");
		}else if (IS_SET(node->flags, LASTON_IRC)){
			strcpy(name_colour, "`M");
		}else if (IS_SET(node->flags, LASTON_HASUSEDIRC)){
			strcpy(name_colour, "`g");
		}else{
			strcpy(name_colour, "`x");
		}

		// tally up the total bank, gold, silver for those shown
		total_bank+=node->bank;
		total_gold+=node->gold;
		total_silver+=node->silver;

        if (laston_summary){ // show summary list
            newindex= node->index;

            if (IS_SET(node->flags, LASTON_ONLINE))
            {                  
                sprintf(buf, "%s%-12s %s %s%s[%2d.%d] %sonline for %s.%s`Y%s`x\r\n",
					name_colour,
                    node->name,
                    (laston_show_class && (node->clss>-1)?class_table[node->clss].short_name:""),
                    (laston_show_race && (node->race>-1)?race_table[node->race]->short_name:""),
					(IS_TRUSTED(ch,ANGEL) && node->level[newindex]>=100?"":" "),
                    (IS_TRUSTED(ch,ANGEL)?node->level[newindex]:0),
                    (IS_TRUSTED(ch,ANGEL)?node->sublevel[newindex]:0),
                    clanname_buffer,
					laston_wide?short_timediff(node->on[newindex] , current_time)
								:condensed_timediff(node->on[newindex] , current_time),
                    (IS_TRUSTED(ch,GUARDIAN)
							?(!IS_NULLSTR(node->ip[newindex]) && laston_show_ip
								?FORMATF(" (%s)[%s] ", node->host[newindex], node->ip[newindex])
								:FORMATF(" (%s) ", node->host[newindex]))
							:""),
                    (IS_TRUSTED(ch,ADMIN-1) && laston_email && !IS_NULLSTR(node->email))?node->email:""
					);
            }else{ // single line listing - player not currently online
                sprintf(buf, "%s%-12s %s %s%s[%2d.%d] %s%s %s ago%s`Y%s`x\r\n",
					name_colour,
                    node->name,
                    (laston_show_class && (node->clss>-1)?class_table[node->clss].short_name:""),
                    (laston_show_race && (node->race>-1)?race_table[node->race]->short_name:""),
					(IS_TRUSTED(ch,ANGEL) && node->level[newindex]>=100?"":" "),
                    (IS_TRUSTED(ch,ANGEL)?node->level[newindex]:0),
                    (IS_TRUSTED(ch,ANGEL)?node->sublevel[newindex]:0),
                    clanname_buffer,
					IS_SET(node->flags, LASTON_PERM_PKILLED)?"`#`Rpermkilled`&":(node->deleted_date?"deleted":"on"),
					laston_wide?short_timediff(node->off[newindex] , current_time)
								:condensed_timediff(node->off[newindex] , current_time),
                    (IS_TRUSTED(ch,GUARDIAN)
							?(!IS_NULLSTR(node->ip[newindex]) && laston_show_ip
								?FORMATF(" (%s)[%s] ", node->host[newindex], node->ip[newindex])
								:FORMATF(" (%s) ", node->host[newindex]))
							:""),
					(IS_TRUSTED(ch,ADMIN-1) && laston_email && !IS_NULLSTR(node->email))?node->email:""
                    );
			}

			if (laston_olc_builder)
			{
                sprintf(buf3,"OLC=%d ", node->security);
				if (!add_buf( output, buf3 )){
					buffer_overflow=true;
				}else{
					linenum++;
				}
                
			}
			
            // prepend the laston entry with the line counter
			if (laston_number_entries && !buffer_overflow) 
            {
                sprintf(buf3,"`x%3d ", displaycounter+1);
				if (!add_buf( output, buf3 )){
					buffer_overflow=true;
				}
            }
			// add the info to display to the page buffer
            if (!add_buf( output, buf )){
                buffer_overflow=true;
			}else{
				displaycounter++;
				linenum++;
			}
        }
        else // show full listing
        {
			char trust_buf[MIL];
			sprintf(trust_buf,"Trust: %d, Sec: %d",node->trust, node->security);
            sprintf ( buf, "CHARACTER: %s%s`x %s %s %s%s- %s%s %s\r\n", 
				name_colour, 
				node->name,
                (laston_show_class && (node->clss>-1)?class_table[node->clss].short_name:""),
                (laston_show_race && (node->race>-1)?race_table[node->race]->short_name:""),
				capitalize(flag_string(sex_types, node->sex)),
                clanname_buffer,
                (IS_SET(node->flags, LASTON_ONLINE)?"":
							(node->deleted_date?"`Sdeleted ":"laston ")),
                (IS_SET(node->flags, LASTON_ONLINE)?
                   "online":timediff(node->off[node->index] , current_time)),
				   IS_ADMIN(ch)?trust_buf:""
            );

			// tack on the deleted date
			if(node->deleted_date){
				char delbuf[MIL];
				sprintf(delbuf, "Deleted: %s", ctime(&node->deleted_date));
				strcat(buf, delbuf);				
			}

            // prepend the laston entry with the line counter
            if (laston_number_entries && !buffer_overflow) 
            {
                sprintf(buf3,"%3d ", displaycounter+1);
				if (!add_buf( output, buf3 )){
					buffer_overflow=true;
				}
            }

			if (IS_SET(node->flags, LASTON_PERM_PKILLED))
			{
				if (!add_buf( output, "`#`RPERMKILLED`& ")){
					buffer_overflow=true;
				}else{
					linenum++;
				}
                
			}	

			// add the info to display to the page buffer
            if (!add_buf( output, buf ))
                buffer_overflow=true;
            else{
				displaycounter++;
				linenum++;
			}

			// display clan info
			if (node->clan>0){
		        sprintf(buf,"`xclan: %s `xrank: %s (%d)`x\r\n",
				node->clan->cwho_name(),
				node->clan->clan_rank_title(node->clanrank),
				node->clanrank);              
				if (!add_buf( output, buf )){
					buffer_overflow=true;
				}else{
					linenum++;
				}
			}

			sprintf ( buf, "Noble: %s  Letgained: %s  KnowIndex=%d, ID=%d - Created: %s ", 
				(IS_SET(node->flags, LASTON_NOBLE)?"`#`RYES`^":"no"),
				(IS_SET(node->flags, LASTON_LETGAINED)?"`#`BYES`^":"no"),
				(int)node->know_index, (int)node->player_id, ctime( (time_t *) &node->player_id));
			if (!add_buf( output, buf ))
                buffer_overflow=true;
            else{
				linenum++;
			}

			if(IS_ADMIN(ch) && !IS_NULLSTR(node->email)){
				sprintf ( buf, "Email: %s\r\n", node->email);
				if (!add_buf( output, buf ))
					buffer_overflow=true;
				else{
					linenum++;
				}
			}

			double played_percent=0.0;
			time_t last_on_time;
			if(IS_SET(node->flags, LASTON_ONLINE)){
				last_on_time=current_time;
			}else{
				last_on_time=node->off[node->index];
			}			
			if( (last_on_time-node->player_id)!=0 ){// prevent divide by zero's
				played_percent= node->played/(double)(last_on_time-node->player_id);
				played_percent*=100;
			}

			sprintf ( buf, "Played:%d.%02d(%0.03f%%) LogoutRoom:%d Short: %s`x\r\n", 
				(int) node->played/ 3600, 
				((int) (node->played /36)%100),
				played_percent,
				node->logout_room,
				node->short_descr);				
			if (!add_buf( output, buf )){
                buffer_overflow=true;
            }else{
				linenum++;
			}

			// alliance/tendency, bank, gold, silver
			if(IS_TRUSTED(ch,ANGEL)){
				sprintf ( buf, "Alliance: %d  Tendency: %d  Bank: %ld  Gold: %ld  Silver: %ld\r\n", 
					node->alliance, node->tendency, node->bank, node->gold, node->silver);
				if (!add_buf( output, buf ))
					buffer_overflow=true;
				else{
					linenum++;
				}
			}

            // characters times and levels 
            for (tempindex=node->index+LASZ; tempindex > node->index; tempindex--)
            {
				char identbuf[MIL];
				newindex = tempindex % LASZ;
                if (node->on[newindex]==0){// display only valid records 					
					continue;
				}
                strcpy(buf2, ctime((time_t *) &node->on[newindex]));

				// see if we need to display the year instead of the actual hours
				if(node->on[newindex]+ 60*60*24*365 < current_time){
					// it has been longer than 1 year
					if(laston_wide){
						// wide format, show pretty much the whole date						
						buf2[24] = '\0'; //trim off the new line after the year
					}else{
						buf2[24] = '\0'; //trim off the new line after the year
						buf2[11] = '\0'; //remove hh:mm:ss yyyy
						// then add the year back in
						strcpy(&buf2[11], &buf2[19]); // copy year portion
					}
				}else{
					if(laston_wide){
						buf2[20] = '\0'; //remove year 
					}else{
						buf2[16] = '\0'; //remove year and seconds 
					}
				}
                sprintf ( buf, " on: %s", buf2);
                add_buf( output, buf );

				if(!IS_NULLSTR(node->ident[newindex])){
					sprintf(identbuf, "(ident=%s)",node->ident[newindex]);						
				}else{
					identbuf[0]='\0';

				}

                if (node->off[newindex]<=node->on[newindex])
                {
					sprintf ( buf, "(lvl %d.%d/%d) online for %s %s%s\r\n",
						(IS_TRUSTED(ch,ANGEL)?node->level[newindex]:0),
						(IS_TRUSTED(ch,ANGEL)?node->sublevel[newindex]:0),
						(IS_TRUSTED(ch,ANGEL)?get_sublevels_for_level(node->level[newindex]):0),
						short_timediff(node->on[newindex] , current_time),
	                    (IS_TRUSTED(ch,GUARDIAN)
							?(!IS_NULLSTR(node->ip[newindex]) && laston_show_ip
								?FORMATF(" (%s)[%s] ", node->host[newindex], node->ip[newindex])
								:FORMATF(" (%s) ", node->host[newindex]))
							:""),
						identbuf
						);
                }
                else
                {
					if(laston_show_times){
						sprintf ( buf, "[%d.%d] off %-20.20s %s%s\r\n",
							(IS_TRUSTED(ch,ANGEL)?node->level[newindex]:0),
							(IS_TRUSTED(ch,ANGEL)?node->sublevel[newindex]:0),
							ctime((time_t *) &node->off[newindex]),
							(IS_TRUSTED(ch,GUARDIAN)
								?(!IS_NULLSTR(node->ip[newindex]) && laston_show_ip
									?FORMATF(" (%s)[%s] ", node->host[newindex], node->ip[newindex])
									:FORMATF(" (%s) ", node->host[newindex]))
								:""),
							identbuf
							);
					}else{
						sprintf ( buf, "[%d.%d] on %s %s%s\r\n",
							(IS_TRUSTED(ch,ANGEL)?node->level[newindex]:0),
							(IS_TRUSTED(ch,ANGEL)?node->sublevel[newindex]:0),
							short_timediff(node->on[newindex] , node->off[newindex]),
							(IS_TRUSTED(ch,GUARDIAN)
								?(!IS_NULLSTR(node->ip[newindex]) && laston_show_ip
									?FORMATF(" (%s)[%s] ", node->host[newindex], node->ip[newindex])
									:FORMATF(" (%s) ", node->host[newindex]))
								:""),
							identbuf
							);
					}
                }
                if (!add_buf( output, buf ))
                {
                    buffer_overflow=true;
                };
            }  // end of loop thru a characters LASZ times 
            if (!add_buf( output, "`x\r\n"))
            {
                buffer_overflow=true;
            };
        }
    } // next node in list

    ch->printlnf("%d character%s total, displaying %d",
        ntotal, (ntotal==1?"":"s"), displaycounter);

	long total_wealth=total_bank*100 + total_gold*100 + total_silver;
	ch->printlnf("Total wealth for %d character%s displayed: %ld.%ld (bank=%ld, gold=%ld, silver=%ld)",
        displaycounter, (displaycounter==1?"":"s"),
		total_wealth/100, total_wealth%100, total_bank, total_gold, total_silver);

    if(name_prefix[0] != '\0')
    {
        sprintf( buf,"\r\n%d character%s total, displayed %d\r\n",
            ntotal, (ntotal==1?"":"s"), displaycounter);
        if (!add_buf( output, buf ))
            buffer_overflow= true;
    }

    if(buffer_overflow){
        ch->println("Unable to display all laston data, due to the amount requested.");
	}

    ch->sendpage(buf_string(output));
    free_buf(output);
    return;
}

/****************************************************************************
 *  do_laston_remove - written by Kalahn - Jan 1998                         *
 *   - removes the last occurance of a name in the last database            *
 *   - doesn't delete the pfile though										*
 ***************************************************************************/
void do_lastonremove( char_data *ch, char *argument )
{
    char pname[MIL];
    laston_data *node;
	int count;

    if (IS_NULLSTR(argument))
	{
		ch->println("Syntax: lastonremove <playername>");
		ch->println("  note: if there are duplicates, it will remove the tail entry.");
	}
    
    argument = one_argument( argument, pname );

    // loop thru all laston records in linked list
	count=0;
	// count the number of matching occurances
    for (node= laston_list; node; node=node->next){
		if(!str_cmp(pname, node->name)){
			count++;
		}
	};   

	if(count>0){
		int removenum=0;
		for (node= laston_list; node; node=node->next){
			if(!str_cmp(pname, node->name)){
				if(++removenum==count){
					if (node)
					{
						ch->printlnf("Removed %s from laston database, there were %d total found.", 
							node->name, count);
						laston_id_delete(node->player_id); // remove them from the laston list
					};
					break;
				};
			}
		};   
	}else{
		ch->printlnf("No name matching '%s' was found to remove.", pname);
	}
    return;
}

/****************************************************************************
 *  do_laston - written by Kalahn - July 1997            *
 *   - selects how to process the laston request (immortal vs mortal)       *
 ***************************************************************************/
void do_laston( char_data *ch, char *argument )
{
    // command not valid for npcs
    if ( IS_NPC(ch) ){
		ch->println("Players only");
        return;
	}

	if(HAS_CONFIG2(ch, CONFIG2_HIGHIMMORTAL_LASTON_ACCESS)){
		chImmortal->level=ch->level;
		ch->level=ABSOLUTE_MAX_LEVEL;
	}

	if(IS_SET(ch->dyn,DYN_IMMLASTON)){
		chImmortal->level=ch->level;
		ch->level=ABSOLUTE_MAX_LEVEL;
	}

	if(!GAMESETTING2(GAMESET2_USE_LASTON_IMM_VALUE)){
		if (IS_IMMORTAL(ch)){
			laston_immortal(ch, argument);
		}else{
			laston_mortal(ch, argument);
		}
	}else{
		if (ch->level>=game_settings->laston_level_for_immortal_version){
			laston_immortal(ch, argument);
		}else{
			laston_mortal(ch, argument);
		}
	}

	if(IS_SET(ch->dyn,DYN_IMMLASTON)){
		ch->level=chImmortal->level;
	}
}

/***************************************************************************
 *  laston_save - written by Kalahn - July 1997							   *
 *   - saves all players                                                   *
 *   - called by laston_login and laston_logout if we haven't saved        * 
 *     for a the LASTON_SAVE_INTERVAL (in seconds)                         *            
 *   - also called from in act_wiz.c by do_reboot and do_save,             *
 *     maintence_saves() in update.c									   *
 ***************************************************************************/
void laston_save(char_data *) 
{
    laston_data *node;

    if (LASTON_SAVE_DATA){
        // update times on characters 
        for (node= laston_list; node; node=node->next){
            if (IS_SET(node->flags, LASTON_ONLINE)){
                node->off[node->index] = current_time; // record when the data is uptil 
			}
        }

        logf("===Saving in laston records to %s.", LASTON_FILE);
		GIOSAVE_LIST(laston_list, laston_data, LASTON_FILE, true);
        logf("Laston save completed");

    }else{
        logf("Laston save disabled - records not saved.");
	}
    // record next autosave time 
    laston_next_save = current_time + LASTON_SAVE_INTERVAL;
}
/**************************************************************************/
void update_alarm();
/**************************************************************************/
// loop thru all characters listed in laston and update details in their 
// node from what is seen on disk
void do_laston_update_from_disk(char_data *ch, char *argument )
{
	if(ch && IS_NPC(ch)){
		do_huh(ch,"");
        return;
	}

	if (str_cmp("confirm", argument)){
        ch->println( "`xType `=Claston_update_from_disk confirm`x if you want to have the mud pload" );
        ch->println( "EVERY single player listed in laston and update their laston record." );
		ch->println( "NOTE: THIS WILL LAG THE MUD FOR A FEW SECONDS + DEPENDING ON THE NUMBER OF PLAYERS!" );
		return;
	}

	// we have a green light
	int iCount=0;
	log_string("do_laston_update_from_disk(): ploading pfiles, updating laston nodes...");
	for(laston_data *node=laston_list; node; node=node->next){
		char_data *lch=pload_load_character(NULL, node->name, true);
		if(lch){
			laston_update_node(node, lch, true);
			iCount++;
			pload_unload_character(NULL, lch);
		}
		// update the alarm so a long update doesn't cause the mud to kill itself
		update_alarm(); 
	}
	logf("do_laston_update_from_disk(): finished updating laston %d records%s, resaving laston...", iCount, iCount==1?"":"s");
	// resave the laston database
	laston_save(NULL);
	ch->printlnf("%d records%s updated, laston database resaved.", iCount, iCount==1?"":"s");

	log_string("do_laston_update_from_disk(): completed.");
}

/***************************************************************************
 *  laston_load - written by Kalahn - July 1997
 *   - reads in all laston records
 ***************************************************************************/
void laston_load() // loads the laston data
{
    laston_data *node, *prev;
    laston_list = NULL;
	int count=0;
	bool bWealthSeen=false;// if the entire laston database has 0 wealth for everyone, we will pload all pfiles and generate it

    log_string ("===Reading in laston records.");
    GIOLOAD_LIST(laston_list, laston_data, LASTON_FILE);

    // now mark as offline and link the previous node pointers
	laston_since = current_time;
    prev=NULL;
    for(node=laston_list; node; node=node->next)
    {
        REMOVE_BIT(node->flags, LASTON_ONLINE); // nolonger online
        node->in_list = true;  // mark as a member of the list
        node->prev= prev;
        prev = node;
		count++;

		// figure out when the laston data relates back to 
		if(node->on[node->index]>100 && node->on[node->index]<laston_since){
			laston_since = node->on[node->index];  // a rough estimate
		}
    }


    logf("%d laston record%s %s been read.", 
		count, count==1?"":"s", count==1?"has":"have");


	// now remove player records of those long since deleted
	laston_data *next_node;
    for(node=laston_list; node; node=next_node){
		next_node=node->next;

		if(!node->deleted_date){
			continue;
		}

		int level=node->level[node->index];
		int days_to_keep=0;
		if(level<5){
			days_to_keep=game_settings->laston_remove_deleted_players_0_4;
		}else if(level<35){
			days_to_keep=game_settings->laston_remove_deleted_players_5_34;
		}else if(level<65){
			days_to_keep=game_settings->laston_remove_deleted_players_35_64;
		}else if(level<92){
			days_to_keep=game_settings->laston_remove_deleted_players_65_91;
		}else{
			days_to_keep=game_settings->laston_remove_deleted_players_92;
		}
		if(	(node->deleted_date + (days_to_keep* 60*60*24))<current_time){
			logf("Removing old laston record of deleted player '%s'<%d>", 
				node->name, level);
			laston_id_delete(node->player_id);
		}
	}

	// check for wealth, 
	for(node=laston_list; node; node=node->next){
		node->wealth=node->bank*100 + node->gold*100 + node->silver;
		if(node->wealth){
			bWealthSeen=true;
		}
	}
	// haven't seen any wealth... read in all players wealth, Kal - Mar 09
	if(!bWealthSeen){
		log_note("laston_load() - didn't see any wealth information for any of the laston records - required for the topwealth command..."
			"performing a 'laston_update_from_disk confirm' to populate the laston database with wealth information from every pfile.");
		do_laston_update_from_disk(NULL, "confirm"); 
		log_string("laston_load(), laston update completed.");
	}
	
    return;
}
/**************************************************************************/


/***************************************************************************
 *  lastonreload - reloads the laston list from disk                       *
 *   - allows manual editing of laston file.							   *	
 *   - calls laston_load												   *
 ***************************************************************************/
void do_lastonreload(char_data *ch, char *)
{
	ch->println("Reloading laston records.");
	laston_load();
	ch->println("Laston records have been reloaded.");  
}

/**************************************************************************/
void add_top_roleplayers(laston_data *pnode)
{
	laston_data *node, *prev;

	if (!laston_rpers_list)
	{
		laston_rpers_list=pnode;
		return;		
	}

	// go down the list till we find the place they are less than
	prev=NULL;
    for (node= laston_rpers_list; node; node=node->next_rper)
    {
		if (node->rps < pnode->rps){
			if (!prev)
			{
				pnode->next_rper=laston_rpers_list;
				laston_rpers_list=pnode;
				return;
			}

			pnode->next_rper = node;
			prev->next_rper  =pnode;
			return;
		}
		prev=node;
	}
	pnode->next_rper = node;
	prev->next_rper  =pnode;

}

/**************************************************************************/
void resort_top_roleplayers()
{
	laston_data *node;

	log_string("resorting top roleplayers list...");
	// first wipe the old toprp list
	laston_rpers_list=NULL;
    for (node= laston_list; node; node=node->next)
    {
		node->next_rper=NULL;		
	}

	// now readd to the list if their rps is over 10000
    for (node= laston_list; node; node=node->next)
    {
		if (node->rps>=10000  // nonimms on in the last 8 weeks
			&& (node->level[node->index]<LEVEL_IMMORTAL)
			&& node->off[node->index]> current_time-(60*60*24*7*8))
			add_top_roleplayers(node);	
	}
	log_string("toprp resort complete.");

}

/**************************************************************************/
void do_toprp(char_data *ch, char *)
{
	laston_data *node;
    BUFFER *output;
    char buf[MSL];
	int col=0;
	int line=3;
	int lines_to_show=ch->lines;
	int count=1;

	if (lines_to_show==0)
		lines_to_show=9999;

    output= new_buf();

	sprintf( buf,"`?%s`x\r\n", format_titlebar("HIGHEST RPS SCORES"));
    add_buf(output,buf);

    for (node= laston_rpers_list; node; node=node->next_rper)
    {
//		sprintf(buf, "   %-13s%6d    ", node->name, node->rps);
		sprintf(buf, "`W%3d `w%-13s%6d    ", count, node->name, node->rps);
	    add_buf(output,buf);
		
		if (++col%3==0)
		{
		    add_buf(output,"\r\n");
			if (++line>lines_to_show)
				break;
		}
		count++;
	}

	if (col%3!=0)
		add_buf(output,"\r\n");

	ch->sendpage(buf_string(output));
	free_buf(output);
}

/**************************************************************************/
void add_top_wealth(laston_data *pnode)
{
	laston_data *node, *prev;

	if (!laston_wealth_list){
		laston_wealth_list=pnode;
		return;		
	}

	// go down the list till we find the place they are less than
	prev=NULL;
	for (node= laston_wealth_list; node; node=node->next_wealth)
    {
		if (node->wealth < pnode->wealth){
			if (!prev){
				pnode->next_wealth=laston_wealth_list;
				laston_wealth_list=pnode;
				return;
			}

			pnode->next_wealth = node;
			prev->next_wealth =pnode;
			return;
		}
		prev=node;
	}
	pnode->next_wealth= node;
	prev->next_wealth=pnode;

}
/**************************************************************************/
void resort_top_wealth()
{
	laston_data *node;

	log_string("resorting top wealth list...");
	// first wipe the old wealth list
	laston_wealth_list=NULL;
    for (node= laston_list; node; node=node->next){
		node->next_wealth=NULL;		
	}

	// now readd to the list if their rps is over 10000
    for (node= laston_list; node; node=node->next){
		add_top_wealth(node);
	}
	log_string("wealth resort complete.");

}
/**************************************************************************/
void do_topwealth(char_data *ch, char *argument)
{
	laston_data *node;
    BUFFER *output;
    char buf[MSL];
	int col=0;
	int line=3;
	int lines_to_show=0; //ch->lines;
	int count=1;

	if (lines_to_show==0)
		lines_to_show=9999;

    output= new_buf();

	sprintf( buf,"`?%s`x", format_titlebar("TOP WEALTH"));
    add_buf(output,buf);

	for (node= laston_wealth_list; node; node=node->next_wealth)
    {		
		if(node->wealth==1000 && node->level[node->index]==1){
			// don't show level 1 players with the default amount of gold
			// unless a filter was specified
			if(IS_NULLSTR(argument)){
				continue;
			}
		}

		// filter in only required areas
		if(!IS_NULLSTR(argument) && !is_name( argument, node->name)) {
			continue;
		}
		// get the colour of the entries name
		char name_colour[3];
		if(node->deleted_date){
			strcpy(name_colour, "`S");
		}else if (IS_SET(node->flags, LASTON_IRC)){
			strcpy(name_colour, "`M");
		}else if (IS_SET(node->flags, LASTON_HASUSEDIRC)){
			strcpy(name_colour, "`g");
		}else{
			strcpy(name_colour, "`x");
		}

		char level_text[20];
		if(IS_TRUSTED(ch,ANGEL)){
			sprintf(level_text,"%s[%2d.%d]",
				node->level[node->index]>=100?"":" ",
				node->level[node->index],
				node->sublevel[node->index]);
		}else{
			strcpy(level_text,"");
		}
		sprintf(buf, "`W%4d`x%s%s%-14s`Y%8ld.`S%02ld     `x",
			count,
			level_text,
			name_colour,
			node->name,			
			node->wealth/100, node->wealth%100);
	    add_buf(output,buf);
		
		if (++col%2==0)
		{
		    add_buf(output,"\r\n");
			if (++line>lines_to_show){
				break;
			}
		}
		count++;
	}

	if (col%2!=0){
		add_buf(output,"\r\n");
	}
	add_buf(output,"Note: use `=Cclanlist`x to see clan bank balances\r\n");
	if(IS_NULLSTR(argument)){		
		add_buf(output,"Note2: you can filter the topwealth output by specifying a name prefix\r\n");
	}

	ch->sendpage(buf_string(output));
	free_buf(output);
}

/**************************************************************************/
int count_active_players(void)
{
	laston_data *node;
	int active=0;

    for (node= laston_list; node; node=node->next)
    {
		if (node->level[node->index]<6)
			continue;
		if (node->level[node->index]>LEVEL_IMMORTAL)
			continue;
		if (node->off[node->index]+ (60*60*24*7)<current_time)
			continue;
		if(node->deleted_date)
			continue;
		
		if(!race_table[node->race]->creation_selectable())
			continue;
		active++;
	}

	return active;
}

/**************************************************************************/
// laston accuracy reduced for those that abuse laston info
// Written by Kalahn at the request of Rathern
void do_reducelaston( char_data *ch, char *argument )
{
    char arg[MIL], buf[MSL];
	char_data *victim;
 
	// can only be used by imms 
	if (  !IS_IMMORTAL(ch) )
	{
		do_huh(ch,"");
		return;
	}
	
	one_argument( argument, arg );
 
    if ( arg[0] == '\0' ){
        ch->println("Reduce laston accuracy on whom?");
        return;
    }
 
    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println("They aren't here.");
        return;
    }
 
    // can't reduce laston on high level imms than yourself
	if ( get_trust( victim ) >= LEVEL_IMMORTAL 
		&& get_trust( victim ) >= get_trust( ch ) )
    {
        ch->println("You failed.");
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_REDUCED_LASTON) )
    {
        REMOVE_BIT(victim->comm, COMM_REDUCED_LASTON);
		logf("reducelaston removed on %s by %s", victim->name, ch->name);
        victim->println("The gods have restored your ability to get detailed info from laston.");
        ch->println("Reduced laston removed.");
		sprintf(buf,"$N regives full laston details to %s",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_REDUCED_LASTON);
		logf("reduced laston set on %s by %s", victim->name, ch->name);
        victim->println("The gods have revoked your ability to get detailed info on imms from laston.");
		ch->printf("%s can no longer tell with laston if an imm has been on within the last 2 days.", victim->name);

		sprintf(buf,"$N reduces %s's ability to get laston login times more accurate that 2 days regarding imms.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}
/**************************************************************************/
// mud client statistics code, Kal, Feb 2002
char *laston_generate_mud_client_stats()
{
	char *client_version="-";
	char buf[MSL];
	char num[MSL];
	int val;
	bool buffer_full;
	name_linkedlist_type *plist, *plist_next; // used for going thru the list etc
	name_linkedlist_type *list=NULL; // all players
	name_linkedlist_type *alist=NULL; // active players
	name_linkedlist_type *clist=NULL; // condensed list
	name_linkedlist_type *slist=NULL; // sort list
	laston_data *node;
	int total=0;
	int atotal=0;
	int ctotal=0;
	static char result[32000];
	char all_buf[16000];
	char active_buf[8000];
	char condensed_buf[8000];

    
	// loop thru the list getting all mxp client versions
    for (node= laston_list; node; node=node->next)
    {
		if(!IS_NULLSTR(node->mxp_client_version)){
			client_version=node->mxp_client_version;
		}else if(!IS_NULLSTR(node->terminal_type)){
			client_version=FORMATF(" termtype=%s", node->terminal_type);
		}else{
			client_version=" unknown";
		}

		total++;
		addlist(&list, client_version, 0, false, false);
		if (node->level[node->index]<6)
			continue;
		if (node->level[node->index]>LEVEL_IMMORTAL)
			continue;
		if (node->off[node->index]+ (60*60*24*7)<current_time)
			continue;
		if(node->deleted_date)
			continue;
		
		if(!race_table[node->race]->creation_selectable())
			continue;

		addlist(&alist, client_version, 0, false, false);		

		// condensed list
		sprintf(buf, "%d #%s%s", str_len(client_version), client_version, node->host[node->index]);
		client_version=buf;
		addlist(&clist, client_version, 0, false, false);		
		
		atotal++;
    }

	// sort the all player total list by quantity of clients, highest first
	for(plist=list;plist; plist=plist_next){
		plist_next=plist->next;
		addlist(&slist, FORMATF("%5d=%s", 
			plist->count,
			plist->name), 0, true, true);		
		free_string(plist->name);
		delete plist;
	}

	sprintf(all_buf,"-=all player total:%d\r\n", total);
	buffer_full=false;
	for(plist=slist;plist; plist=plist_next){
		plist_next=plist->next;
		if(!buffer_full){
			if(str_len(all_buf)>15350){
				buffer_full=true;
				strcat(all_buf, "   -1= buffer too full to display remaining text");
			}else{
				// we assume plist->name is less than 600 bytes in length
				// because the code limits the mxp version string to 512 bytes
				strcat(all_buf,plist->name);
				strcat(all_buf,"\n\r");
			}
		}
		free_string(plist->name);
		delete plist;
	}
	slist=NULL;

	// sort the active player list by quantity of clients, highest first
	for(plist=alist;plist; plist=plist_next){
		plist_next=plist->next;
		addlist(&slist, FORMATF("%5d=%s", 
			plist->count,
			plist->name), 0, true, true);		
		free_string(plist->name);
		delete plist;
	}

	sprintf(active_buf,"-=active player total:%d\r\n", atotal);
	buffer_full=false;
	for(plist=slist;plist; plist=plist_next){
		plist_next=plist->next;
		if(!buffer_full){
			if(str_len(active_buf)>7350){
				buffer_full=true;
				strcat(active_buf, "   -1= buffer too full to display remaining text");
			}else{
				// we assume plist->name is less than 600 bytes in length
				// because the code limits the mxp version string to 512 bytes
				strcat(active_buf,plist->name);
				strcat(active_buf,"\n\r");
			}
		}
		free_string(plist->name);
		delete plist;
	}	
	slist=NULL;

	// loop thru the results, generating a condensed version of the list
	list=NULL;
	ctotal=0;
	for(plist=clist;plist; plist=plist_next){
		plist_next=plist->next;
		client_version=one_argument(plist->name, num);
		val=atoi(num);
		
		strcpy(buf,client_version+1);
		buf[val]='\0';

		addlist(&list, buf, 0, false, false);
		ctotal++;
		free_string(plist->name);
		delete plist;
	}

	// sort the condensed list by quantity of clients, highest first
	for(plist=list;plist; plist=plist_next){
		plist_next=plist->next;
		addlist(&slist, FORMATF("%5d=%s", 
			plist->count,
			plist->name), 0, true, true);		
		free_string(plist->name);
		delete plist;
	}

	sprintf(condensed_buf,"-=condensed player total:%d\r\n", ctotal);
	buffer_full=false;
	for(plist=slist;plist; plist=plist_next){
		plist_next=plist->next;
		if(!buffer_full){
			if(str_len(condensed_buf)>7350){
				buffer_full=true;
				strcat(condensed_buf, "   -1= buffer too full to display remaining text");
			}else{
				// we assume plist->name is less than 600 bytes in length
				// because the code limits the mxp version string to 512 bytes
				strcat(condensed_buf,plist->name);
				strcat(condensed_buf,"\n\r");
			}
		}
		free_string(plist->name);
		delete plist;
	}	
	slist=NULL;

	strcpy(result,condensed_buf);
	strcat(result, "\r\n");
	strcat(result,active_buf);
	strcat(result, "\r\n");
	strcat(result,all_buf);
	strcat(result, "=-\r\n");
	return result;
}

/****************************************************************************
 *  do_lastonstats - written by Kalahn - Sept 1998       *
 ***************************************************************************/
void do_lastonstats( char_data *ch, char *argument)
{
	int i, class_count[MAX_CLASS];
	int race_count[MAX_RACE];

	int bit_table[32];

	int total;
	int colcount=0;
	int alignment_matrix[8][8];
	memset(alignment_matrix, 0, sizeof(alignment_matrix));
    
    laston_data *node;

    /*
     * Display a message telling how current the data is
     */
    ch->printlnf("%s has been up for %s",
             MUD_NAME, timediff(boot_time, current_time));
    ch->printlnf("The current time is %sLaston records date back %s.",
             (char *) ctime( &current_time ),
             timediff(laston_since, current_time));

    // reset counters 
	total=0;
	for (i=0; i<MAX_CLASS; i++){
		class_count[i]=0;
	}
	for (i=0; i<MAX_RACE; i++){
		race_count[i]=0;
	}
	for (i=0; i<32; i++){
		bit_table[i]=0;
	}

	int gender[3];
	for (i=0; i<3; i++){
		gender[i]=0;
	}

    // loop thru all character records in linked list
    for (node= laston_list; node; node=node->next)
    {
		if (node->level[node->index]<6)
			continue;
		if (node->level[node->index]>LEVEL_IMMORTAL)
			continue;
		if (node->off[node->index]+ (60*60*24*7)<current_time) // active only players
			continue;
		if(node->deleted_date)
			continue;
		if(!race_table[node->race]->creation_selectable())
			continue;
     
		class_count[node->clss]++;
		race_count[node->race]++;
		total++;

		for (int bit_index=0; bit_index<32; bit_index++)
		{
			int bit_value = 1<<bit_index;

			if (!IS_SET(bit_value, node->flags))
				continue;
			bit_table[bit_index]++;
		}

		gender[URANGE(0, node->sex,3)]++;

		// record alignment matrix details 
		alignment_matrix[node->alliance+3][node->tendency+3]++;
		alignment_matrix[7][node->tendency+3]++;
		alignment_matrix[node->alliance+3][7]++;
    }

	if (total<1){
		ch->printlnf("There are no active players. (>5 <%d)", 
			LEVEL_IMMORTAL);
		return;
	}

	ch->print("`?`#"); // get the random colour
	if(!str_cmp(argument,"class")|| IS_NULLSTR(argument))
	{
		colcount=0;
		ch->titlebar("LASTON CLASS STATS");
		for (i=0; !IS_NULLSTR(class_table[i].name) && i<MAX_CLASS; i++)
		{
			if (class_table[i].creation_selectable==false)
				continue;
			ch->printf("`x  %-14s %3d (%5.1f%%)", 
				capitalize(class_table[i].name), class_count[i], 
				((float)(class_count[i]*100)/(float)total));
			if(++colcount%2==0){
				ch->println("`x");
			}else{
				ch->print("                 ");
			};
		}
		if(colcount%2==1){
			ch->println("`x");
		};
	}


	if(!str_cmp(argument,"race") || IS_NULLSTR(argument))
	{
		colcount=0;
		ch->titlebar("LASTON RACE STATS");
		for (i=1; race_table[i]; i++)	
		{
			if(!race_table[i]->creation_selectable())
				continue;
			ch->printf("`x  %-14s %3d (%5.1f%%)", 
				capitalize(race_table[i]->name), race_count[i],
				((float)(race_count[i]*100)/(float)total));
			if(++colcount%2==0){
				ch->println("`x");
			}else{
				ch->print("                 ");
			};
		}
		if(colcount%2==1){
			ch->println("`x");
		};
	}

	if(IS_NULLSTR(argument) && IS_ADMIN(ch))
	{
		colcount=0;
		ch->titlebar("LASTON FLAG STATS");
		for (int bit_index=0; bit_index<32; bit_index++)
		{
			if (bit_table[bit_index]==0)
				continue;

			int bit_value = 1<<bit_index;

			ch->printf("`x  %-14s %3d (%5.1f%%)", 
				capitalize(flag_string(laston_flags, bit_value)), bit_table[bit_index],
				((float)(bit_table[bit_index]*100)/(float)total));
			if(++colcount%2==0){
				ch->println("`x");
			}else{
				ch->print("                 ");
			};
		}
		if(++colcount%2==0){
			ch->println("`x");
		};


		ch->titlebar("LASTON GENDER STATS");
		colcount=0;
		for(i=0; i<3; i++)
		{
			ch->printf("`x  %-14s %3d (%5.1f%%)", 
				capitalize(flag_string(sex_types, i)), gender[i],
				((float)(gender[i]*100)/(float)total));
			if(++colcount%2==0){
				ch->println("`x");
			}else{
				ch->print("                 ");
			};
		}
		if(++colcount%2==0){
			ch->println("`x");
		};
	}

	// Alignment Matrix
	if(!str_cmp(argument,"align") || IS_NULLSTR(argument))
	{
		char buf[MIL], grid[MSL];
		colcount=0;
		ch->titlebar("LASTON ALIGNMENT MATRIX");

		int ali, ten;
		ch->printf("                 `^TENDENCY: `YLawful <----> Neutral <----> Chaotic\r\n");
		ch->printf("                         `S+===`Y3");	
		for(ten=5; ten>=3; ten--){
			ch->printf( "`S====`Y%-d", ten-3);	
		}
		for(ten=2; ten>=0; ten--){
			ch->printf( "`S===`Y%d", ten-3);	
		}
		ch->println("`S==`gTotal`S==");

		grid[0]='\0';
		for(ali=6; ali>=0; ali--){
			strcat(grid, "   ");
			switch (ali){
				case 6: strcat(grid, "`^   ALLIANCE:`B Good ");break;
				case 5: strcat(grid, "`B                  ");break;
				case 4: strcat(grid, "`B                  ");break;
				case 3: strcat(grid, "`B          Neutral ");break;
				case 2: strcat(grid, "`B                  ");break;
				case 1: strcat(grid, "`B                  ");break;
				case 0: strcat(grid, "`B             Evil ");break;
				default:break;
			}
			sprintf(buf," %2d `S|`x", ali-3);
			strcat(grid, buf);
			for(ten=6; ten>=0; ten--){
				sprintf(buf,"%4d ", alignment_matrix[ali][ten]);
				strcat(grid, buf);
			}
			sprintf(buf," `g%4d\r\n", alignment_matrix[ali][7]);
			strcat(grid, buf);
		}

		// bottom totals
		strcat(grid, "   `g               Totals `S|`g");
		for(ten=6; ten>=0; ten--){
			sprintf(buf,"%4d ", alignment_matrix[7][ten]);
			strcat(grid, buf);
		}
		ch->print(grid);
		ch->printlnf(" `G%4d`x\r\n", total);
	}
	
	if(++colcount%2==0){
		ch->println("`x");
	};

	ch->printlnf( "NOTE: active players (%d) are "
		"only in these stats.", total);

}
/**************************************************************************/
void do_classstats( char_data *ch, char *)
{
	do_lastonstats(ch,"class");
}
/**************************************************************************/
void do_racestats( char_data *ch, char *)
{
	do_lastonstats(ch,"race");
}
/**************************************************************************/
void do_alignstats( char_data *ch, char *)
{
	do_lastonstats(ch,"align");
}
/**************************************************************************/
void do_mudclientstats( char_data *ch, char *)
{
	ch->titlebar("MUD CLIENT STATISTICS");
	ch->println(laston_generate_mud_client_stats());
	ch->titlebar("");
}
/**************************************************************************/
// return -1 if immortal not setup for weblevel
// return 0 if failed access
// return the level/trust of the immortal if password and username correct
int laston_web_level(char *username, char *password)
{
    laston_data *node;
	// find the username 
    for (node= laston_list; node; node=node->next){
		if(!str_cmp(username, node->name)){
			// check if it is an imm
			if(node->level[node->index]>=LEVEL_IMMORTAL 
				|| node->trust>LEVEL_IMMORTAL)
			{
				if(IS_NULLSTR(node->webpass)){
					return -1;
				}

				char *crypted_pass=dot_crypt(password, node->webpass);
				if(!strcmp(crypted_pass, node->webpass)){
					return UMAX(node->level[node->index], node->trust);
				}else{
					return 0;
				}
			}	
		}
	};   
	return 0;
}
/**************************************************************************/
/**************************************************************************/
