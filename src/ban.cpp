/**************************************************************************/
// ban.cpp - code related to banning players, pretty much a rewrite by Kal
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

#include "include.h" // dawn standard includes
#include "ban.h"

// prototypes
extern	const	struct	flag_type	ban_types[];
char *flag_string( const struct flag_type *flag_table, int bits );
char *getline( char *str, char *buf, int bufsize); // string.cpp
int wild_match(register unsigned char *m, register unsigned char *n);

// globals
BAN_DATA *ban_list;

/**************************************************************************/    
// create ban_type GIO lookup table 
GIO_START(ban_data)
GIO_STR(sitemasks)
GIO_STR(intended_people)
GIO_WFLAG(type, ban_types)
GIO_INT(ban_date)
GIO_STR(reason)
GIO_STR(by)
GIO_INT(expire_date)
GIO_STR(custom_disconnect_message)
GIO_STR(always_allowed_email_masks)
GIO_STR(allowed_email_masks)
GIO_STR(disallowed_email_masks)
GIO_STR(disallowed_email_custom_message)
GIO_SHINT(level)
GIO_BOOL(enabled)
GIO_FINISH_NOCLEAR

/**************************************************************************/
void save_bans(void)
{
    BAN_DATA *pban;
    FILE *fp;
    bool found = false;
	int count=0;
	
    fclose( fpReserve ); 
    if ( ( fp = fopen( BAN_FILE, "w" ) ) == NULL )
    {
		bugf("save_bans(): fopen for write '%s' - error %d (%s)", 
			BAN_FILE, errno, strerror( errno));
		fpReserve = fopen( NULL_FILE, "r" );
		assert(fpReserve!=NULL);
		return;
    }
	
	logf("Saving bans to %s.", BAN_FILE);
    for (pban = ban_list; pban != NULL; pban = pban->next)
    {
		if (pban->permanent)
		{
			found = true;
			fprintf( fp, "BAN%d~\n", count++);
			GIO_SAVE_RECORD(ban_data, pban, fp, NULL);
		}
	}
	fprintf( fp, "EOF~\n" );
	fclose(fp);
	fpReserve = fopen( NULL_FILE, "r" );

	if (!found){
		unlink(BAN_FILE);
	}
}

/**************************************************************************/
void load_bans(void)
{
    FILE *fp;
    BAN_DATA *ban_last;
	char *input=&str_empty[0];
	int count=0;
 
    if ( ( fp = fopen( BAN_FILE, "r" ) ) == NULL )
        return;
	logf("Loading bans from %s...", BAN_FILE);
 
    ban_last = NULL;
    for ( ; ; )
    {
        BAN_DATA *pban;
        if ( feof(fp) )
        {
            fclose( fp );
			logf("%d ban%s loaded.", count, count==1?"":"s");
            return;
        }

	    // check for end of file system
		free_string(input);			// recycle memory
		input = fread_string(fp);
		if (!str_cmp(input, "EOF")){
            fclose( fp );
			logf("%d ban%s loaded.", count, count==1?"":"s");
			return;
		}
		
        pban = new_ban();
		GIO_LOAD_RECORD(ban_data, pban, fp);
		if(!IS_NULLSTR(pban->sitemasks)){
			pban->permanent=true;
			count++;

			// check expire date - disable ban if appropriate
			if(pban->enabled 
				&& pban->expire_date>0 
				&& pban->expire_date<current_time)
			{
				logf("BAN: %s set on: %.24s (lvl=%d ban, banflags=%s)\n"
					"was disabled due to expiring %.24s.\n",
					fix_string(pban->sitemasks), 
					((char*)ctime(&pban->ban_date)),
					pban->level, 
					flag_string( ban_types, pban->type),
					((char*)ctime(&pban->expire_date))); 
				pban->enabled=false;
				pban->expire_date=0;
			};
			
			// add to linked list
			if (ban_list == NULL){
				ban_list = pban;
			}else{
				ban_last->next = pban;
			}
			ban_last = pban;
		}
    }
}
/**************************************************************************/
// returns true if 'matchto' can be matched by any of the multiple_masks
bool match_multiline_masks( char *multiple_masks, char *matchto)
{
	char tmpb[MSL*4];
	char *t;
	char *banmask;
	
	if(IS_NULLSTR(multiple_masks)){
		return false;
	}

	while ( *multiple_masks)
	{
		multiple_masks = getline( multiple_masks, tmpb, MSL*4);

		// find we the ban mask starts 
		banmask=tmpb;
		while(is_space(*banmask)){
			banmask++;
		}

		// terminate the ban mask at the first whitespace character
		for (t=banmask; *t; t++){
			if (is_space(*t)){
				*t='\0';
				break;
			}
		}

		if(IS_NULLSTR(banmask))
			continue;

		if (wild_match((unsigned char*)banmask, (unsigned char*)matchto)){
			return true;
		}
	}
	return false;
}
/**************************************************************************/
BAN_DATA *check_ban_for_site(char *site,int type)
{
    BAN_DATA *pban;
    char host[MSL];
	int i;

	// get hostname and make sure it is lowercase
    strcpy(host,site);
	for ( i = 0; host[i] != '\0'; i++ ){
		host[i] = LOWER(host[i]);
	}

    for ( pban = ban_list; pban; pban = pban->next )     
	{
		// check ban is enabled
		if(!pban->enabled)
			continue;

		// check ban type
		if(pban->type!=type)
			continue;

		// check expire date - disable ban if appropriate
		if(pban->enabled 
			&& pban->expire_date>0 
			&& pban->expire_date<current_time)
		{
			logf("BAN: %s set on: %.24s (lvl=%d ban, banflags=%s)\n"
				"was disabled due to expiring %.24s.\n",
				pban->sitemasks, 
				((char*)ctime(&pban->ban_date)),
				pban->level, 
				flag_string( ban_types, pban->type),
				((char*)ctime(&pban->expire_date))); 
			pban->enabled=false;
			pban->expire_date=0;
		};

		// do wildcard mask pattern matching
		if (match_multiline_masks( pban->sitemasks, host)){
			return pban;
		}
    }

    return NULL;
}
/**************************************************************************/
BAN_DATA *check_ban(connection_data *c,int type)
{
	BAN_DATA *result;
	result=check_ban_for_site(c->remote_hostname, type);
	if(!result){
		result=check_ban_for_site(c->remote_ip, type);
	}
	return result;
}
/**************************************************************************/
bool email_descriptor_unlock_id(connection_data *d)
{
#ifdef unix
	char unlockname[MIL];
	sprintf(unlockname, "emailunlockfile_%d_%d.tmp", getpid(), (int)current_time);
	char buf[MSL];
	unlink(unlockname);
	sprintf(buf,
"Hello %s,\n"
"This email is an automated email message from %s \n"
"mud... This email is only sent\n"
"after someone creates a character on the game and requests for an unlock\n"
"code to be sent.  The ip address of the person who requested the email is\n"
"%s\n"
"\n"
"  Unlock details:\n"
"    Character name: %s\n"
"    Unlock code: %s\n"
"\n", CH(d)->pcdata->email, MUD_NAME,
	d->remote_ip, CH(d)->name, CH(d)->pcdata->unlock_id);
	append_string_to_file( unlockname, buf, true);
	sprintf(buf,"mail %s -s \"%s dawn unlock code\" < %s ; sleep 10; rm %s &", 
		CH(d)->pcdata->email, CH(d)->name, unlockname, unlockname);
	log_string(buf);
	int ret=system(buf);
	if(ret<0){
		char errbuf[MSL];
		sprintf(errbuf, "Within the ban code, email_descriptor_unlock_id(%d) system call failed with code %d, errno=%d (%s) - providing unlock code directly.  System parameter='%s'", 
			d->connected_socket, ret, errno, strerror(errno), buf);
		log_string(errbuf);
		wiznet(errbuf,NULL,NULL,WIZ_SECURE,0,0);
		log_string(log_buf);
		CH(d)->wraplnf("Emailing the unlock code failed for some reason, your unlock code is %s", CH(d)->pcdata->unlock_id);
	}
#else

	CH(d)->wraplnf("Unlocking codes automatically "
	"emailed is currently supported only by servers that run unix... your unlock "
	"code is %s", CH(d)->pcdata->unlock_id);
#endif
	return true;
}
/**************************************************************************/
// returns NULL if the email is okay, and a text message if the email isn't
char *is_invalid_email(char *email)
{
	static char result[MSL];
	unsigned char *newemail= (unsigned char*)email;
	char *bad_characters=INVALID_CHARACTERS_FROM_USERS_FOR_SYSTEM_CALLS;

	if (has_whitespace(email)){
		sprintf(result,"The email address you enter can not have any whitespace "
			"characters in it!`1reenter:`1");
		return result;
	}
	if (!wild_match((unsigned char*)"*?@?*", newemail)){
		sprintf(result,"The email address must have a @ in the middle somewhere...`1"
			"e.g. bob@aol.com`1reenter:`1");
		return result;
	}
	if (!wild_match((unsigned char*)"*?@?*.?*", newemail)){
		sprintf(result,"The email address must have a @ in the middle "
			"somewhere and at least one . in the domain name to the right of "
			"the @ symbol..`1"
			"e.g. bob@aol.com`1reenter:`1");
		return result;
	}
	if (wild_match((unsigned char*)"*@*@*", newemail)){
		sprintf(result,"The email address must not have more than "
			"one @ symbol in it."
			"`1e.g. bob@aol.com is valid... smith@bob@aol.com is invalid."
			"`1reenter:`1");
		return result;
	}

	logf("check_email_ban(): Doing Badcharacter checks.");
	{ // check for the bad characters
		int i;
		char buf[MSL];
		for(i=0; bad_characters[i];i++){
			sprintf(result,"*%c*", bad_characters[i]);
			if (wild_match((unsigned char*)buf, newemail)){
				sprintf(result,"%c is not an allowed character to have in an email address.`1"
					"reenter:`1", bad_characters[i]);				
				return result;
			}
		}
	}

	return NULL;
}

/**************************************************************************/
// ** This function explains to the user what happened, and what they **
// ** need to do... the returned values tell the calling function how **
// ** to handle the connection. **
// 
// 0 accepted email, and id code emailed to them.
// 1 email rejected, they need another attempt.
// 2 if they should be disconnected - currently not used
int check_email_ban(connection_data *c, char *email_addy)
{
    BAN_DATA *pban;
    char host[MSL];
	char *message;
    unsigned char newemail[MSL];
	int i;
	bool foundallowedemail=false;
	bool found_a_ban_with_an_invalid_email=false;

	// get hostname and make sure it is lowercase
    strcpy(host,c->remote_hostname);
	logf("Starting check_email_ban(): host='%s', email addy='%s'", 
		host, email_addy);

	for ( i = 0; host[i] != '\0'; i++ ){
		host[i] = LOWER(host[i]);
	}
	// get the potential email and make sure it is lowercase
    strcpy((char*)newemail,email_addy);
	for ( i = 0; newemail[i] != '\0'; i++ ){
		newemail[i] = LOWER(newemail[i]);
	}

	// validate the email addy is formatted okay
	message=is_invalid_email((char*)&newemail[0]);
	if(message){
		// email wasnt okay, display the message why not
		CH(c)->wrapln(message);
		return 1;
	}

	logf("check_email_ban(): Doing ban checks.");
    for ( pban = ban_list; pban; pban = pban->next )     
	{
		// check ban is enabled
		if(!pban->enabled)
			continue;

		// check ban type
		if(pban->type!=BAN_EMAIL_REQ)
			continue;
		
		// do wildcard mask pattern matching
		if (match_multiline_masks( pban->sitemasks, host)){
			// matched... now check all email addies, start with always_allowed
			logf("Found match with pban->sitemasks='%s', checking always allowed.", pban->sitemasks);
			if(match_multiline_masks(pban->always_allowed_email_masks, 
				(char *)newemail)){
				foundallowedemail=true;
			}else{
				// disallowed emails
				logf("checking disallowed.");
    			if(match_multiline_masks(pban->disallowed_email_masks, 
							(char *)newemail)){
					if(IS_NULLSTR(pban->disallowed_email_custom_message)){
						logf("check_email_ban(): disallowed email address "
							"'%s' from '%s'!",	newemail, CH(c)->name);
						CH(c)->wraplnf("The email address you supplied "
							"'%s' is marked as a disallowed email address..."
						"`1try another or disconnect:", newemail);
						return 1;
					}else{
						CH(c)->println(pban->disallowed_email_custom_message);
					}
					return 1;
				};
				logf("Checking allowed.");
    			if(match_multiline_masks(pban->allowed_email_masks,
							(char *)newemail)){
					foundallowedemail=true;
				}else{
					found_a_ban_with_an_invalid_email=true;
				}
			}
		}
    }

	logf("Found allowed = %s, found_a_ban_with_an_invalid_email=%s, %s - %s.", 
		foundallowedemail==true?"true":"false",
		found_a_ban_with_an_invalid_email==true?"true":"false",
		host,
		(char*)newemail
		);

	if(foundallowedemail && !found_a_ban_with_an_invalid_email){
		if(str_cmp(CH(c)->pcdata->email, (char*)newemail)){
			replace_string(CH(c)->pcdata->email, (char*)newemail);
		};
		return 0;
	}else{
		if(c->newbie_creating > 0){
			CH(c)->wraplnf("The email address you supplied "
				"'%s' was not found to be on the list of potentially "
				"accepted email address (e.g. *@aol.com in the case of aol.com)"
				"`1If don't have a valid email account, then you are unable to "
				"create a character sorry."
			"`1try another or disconnect:", newemail);
		}else{
			CH(c)->wraplnf("The email address you supplied "
				"'%s' was not found to be on the list of potentially "
				"accepted email address (e.g. *@aol.com in the case of aol.com)"
				"`1try another or disconnect:", newemail);
		}
		return 1;
	}
}
/**************************************************************************/
void ban_site(char_data *ch, char *argument, bool fPerm)
{
    char buf[MSL];
    char arg1[MIL], arg2[MIL];
    char *sitemask;
    BUFFER *buffer;
    BAN_DATA *pban, *prev;
    int type=0;
	int count=0;

	smash_tilde(argument);

	// must have some pcdata to use the command
	if(!TRUE_CH(ch)->pcdata){
		do_huh(ch,"");
		return;
	}

	// permbans - the adding of bans by a select group
    if (fPerm
		&& !IS_TRUSTED(ch,MAX_LEVEL-2)
		&& !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADLAW))
    {
		do_huh(ch,"");
        return;
    }

	if(!fPerm){
		if (ban_list == NULL)
		{
			ch->println( "No sites banned at this time." );
			return;
		}
		buffer = new_buf();
		
        add_buf(buffer,"`=t-================================== `=TBANS `=t===================================-\r\n");
						   
		count=-1;
        for (pban = ban_list;pban != NULL;pban = pban->next)
        {
			count++;
			if(!IS_NULLSTR(argument) &&  
				str_infix(argument,pban->sitemasks))
				continue;
			
			sprintf(buf,"%s%d)%-2.2s%-3.3s%-2.2s=============level: %3d, duration: %s, type: %s\r\n`c",
				(pban->enabled?"`x":"`S"),
				count, 
				((char*)ctime(&pban->ban_date))+8, // quick hack
				((char*)ctime(&pban->ban_date))+4,
				((char*)ctime(&pban->ban_date))+22,
				pban->level, 
				pban->permanent?"perm":"temp",
				flag_string( ban_types, pban->type)); 
			add_buf(buffer,buf);
			add_buf(buffer,pban->sitemasks);

			int l=str_len(pban->sitemasks);
			if(pban->sitemasks[l-1]!='\r' && pban->sitemasks[l-1]!='\n'){
				add_buf(buffer,"\r\n");
			}
			add_buf(buffer,"`=t-===========================================================================-`x\r\n");
        }
        ch->sendpage(buf_string(buffer));
		free_buf(buffer);
        return;
	}

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' )
    {
		if (ban_list == NULL)
		{
			ch->println( "No sites banned at this time." );
			return;
		}
		buffer = new_buf();
		
        add_buf(buffer,"==- Banned sites                     date  lvl status type\r\n");
        for (pban = ban_list;pban != NULL;pban = pban->next)
        {
			sprintf(buf,"%s%2d) %-40s %-2.2s%-3.3s%-2.2s %3d  %s  %s\n",
				(pban->enabled?"`x":"`S"),
				count++, 
				pban->sitemasks, 
				((char*)ctime(&pban->ban_date))+8, // quick hack
				((char*)ctime(&pban->ban_date))+4,
				((char*)ctime(&pban->ban_date))+22,
				pban->level, 
				pban->permanent?"perm":"temp",
				flag_string( ban_types, pban->type)); 
			add_buf(buffer,buf);
        }
        ch->sendpage(buf_string(buffer));
		free_buf(buffer);
        return;
    }

    // find out what type of ban 
    if (arg2[0] == '\0' || !str_prefix(arg2,"all"))
		type = BAN_ALL;
    else if (!str_prefix(arg2,"newbies"))
		type = BAN_NEWBIE;
    else if (!str_prefix(arg2,"email"))
		type = BAN_EMAIL_REQ;
    else if (!str_prefix(arg2,"permit"))
		type = BAN_PERMIT;
    else
    {
		ch->println( "Acceptable ban types are all, newbies, email and permit." ); 
		return;
    }

    sitemask = arg1;


    if (str_len(sitemask) == 0)
    {
		ch->println( "You have to ban SOMETHING." );
		return;
    }


	// check for a banning of themselves
	if(IS_UNSWITCHED_MOB(ch)){
		ch->println( "Players only!" );
		return;
	}
	if(!TRUE_CH(ch)->desc){
		ch->println( "You can't be doing this while linkdead." );
		return;
	}
	if(wild_match((unsigned char*)sitemask, (unsigned char*)TRUE_CH(ch)->desc->remote_hostname)){
		ch->printlnf( "Cant use a siteban mask ('%s') that bans yourself!", sitemask);
		return;
	};

    prev = NULL;
    for ( pban = ban_list; pban != NULL; prev = pban, pban = pban->next )
    {
        if (!str_cmp(sitemask,pban->sitemasks))
        {
			if (pban->level > get_trust(ch))
			{
				ch->println( "That ban was set by a higher power." );
				return;
			}
			else
			{
				if (prev == NULL)
					ban_list = pban->next;
				else
					prev->next = pban->next;
				free_ban(pban);
			}
        }
    }

    pban = new_ban();
    
	pban->by	= str_dup(TRUE_CH(ch)->name);
	pban->sitemasks=str_dup(sitemask);
	if(type){
		pban->type=type;
		pban->enabled=true;
	}

	// a default level (97) unless atlevel is used
    pban->level = IS_SET(ch->dyn,DYN_USING_ATLEVEL)?get_trust(ch):MAX_LEVEL-3;

    // set ban type 
    //pban->ban_flags = type;

    if (fPerm){
		pban->permanent=true;
	}

    // add to the linked list of bans
	pban->next  = ban_list;
    ban_list    = pban;

	// save the ban file
    save_bans();

    ch->printlnf("'%s' has been banned.",pban->sitemasks);
    return;
}

/**************************************************************************/
void do_ban(char_data *ch, char *argument)
{
    ban_site(ch,argument,false);
}

/**************************************************************************/
void do_permban(char_data *ch, char *argument)
{
    ban_site(ch,argument,true);
}

/**************************************************************************/
void do_allow( char_data *ch, char *argument )                        
{
    char arg[MIL];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->println( "Remove which site from the ban list?" );
        return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
		if ( !str_cmp( arg, curr->sitemasks) )
		{
			if (curr->level > get_trust(ch)){
				ch->println( "You are not powerful enough to lift that ban." );
				return;
			}
			if ( prev == NULL )
				ban_list   = ban_list->next;
			else
				prev->next = curr->next;
			
			free_ban(curr);
			ch->printlnf("Ban on %s lifted.",arg);
			save_bans();
			return;
		}
	}

    ch->println( "Site is not banned." );
    return;
}


/**************************************************************************/
// returns true if the connection was disconnected
bool check_connection_ban(connection_data *c)
{
	BAN_DATA *pBan;
	char *msg;
	// check unconditional bans
	if ( (pBan=check_ban(c,BAN_ALL)) )
    {
		if(IS_NULLSTR(pBan->custom_disconnect_message)){
			c->write_colour("`1`1`WYour site has been `Rbanned`W from this mud!!!`1",0);
		}else{
			msg=note_format_string(str_dup(pBan->custom_disconnect_message));
			c->write_colour(msg, 0);
			free_string(msg);
		}
		logf("%s %s (%d) dropped due to complete siteban on their site.", 			
			CH(c)?CH(c)->name:"(unknown name)", c->remote_hostname, c->connected_socket);
		c->outtop=0;// empty output buffer
		connection_close(c);
		return true;
    }

	if ( c->newbie_creating && (pBan=check_ban(c,BAN_LOGNEWBIE)) )
    {				
		logf("Autolog turned on for newbie '%s' from '%s'", 
			c->character->name, c->remote_hostname);
		SET_BIT(c->character->act,PLR_LOG);
	}

	if ( c->newbie_creating && (pBan=check_ban(c,BAN_NEWBIE)) )
    {
		if(IS_NULLSTR(pBan->custom_disconnect_message)){
			msg=note_format_string(str_dup(
				"`1`1Due to abuse or potential abuse of this mud, we have "
				"disabled newbie creation from your site.  This will be in "
				"place to maintain some control of who creates from your "
				"site... If you are new here, "
				"you are welcome to play, we can lift this for you to create "
				"or maybe even permantly :) If you would like to talk with us "
				"regarding this matter, email the administation of "
				"this mud.`1`1"));
		}else{
			msg=note_format_string(str_dup(pBan->custom_disconnect_message));
		}
		c->write_colour(msg, 0);
		free_string(msg);

		connection_close(c);
		logf("%s %s (%d) dropped due to being a newbie creating and from newbie banned site.", 
			CH(c)?CH(c)->name:"(unknown name)", c->remote_hostname, c->connected_socket);
		return true;
    }


	// check bans with permits - only applied to those who have logged in
	if ( CH(c) && (pBan=check_ban(c,BAN_PERMIT)) )
	{
		if (!IS_SET(CH(c)->act,PLR_PERMIT))
		{
			if(IS_NULLSTR(pBan->custom_disconnect_message)){
				msg=note_format_string(str_dup(
					"`1`1Due to abuse or potential abuse of this mud, we have "
					"disabled access from your site.  If you are new here, "
					"you are welcome to play, we can lift this siteban for "
					"individual players... If you would like to talk with us "
					"regarding this matter, email the administation of "
					"this mud.`1`1"));
			}else{
				msg=note_format_string(str_dup(pBan->custom_disconnect_message));
			}
			c->write_colour(msg, 0);				
			free_string(msg);

			if (IS_IRCCON(c)){
				sprintf( log_buf, "NONPERMITED PLAYER DROPPED DUE TO BAN\r\n"
				"was %s@%s (connected via IRC). (socket = %d)",
				CH(c)?CH(c)->name:"(unknown name)", c->remote_hostname, c->connected_socket);
			}else{
				sprintf( log_buf, "NONPERMITED PLAYER DROPPED DUE TO BAN\r\n"
				"was %s@%s (socket = %d)",
				CH(c)?CH(c)->name:"(unknown name)", c->remote_hostname, c->connected_socket);
			}
			wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(CH(c)) );
			log_string(log_buf);

			connection_close(c);
			return true;
		}
		else
		{
			if (IS_IRCCON(c)){
				sprintf( log_buf, "PERMITED PLAYER FROM BANNED SITE ACCEPTED\r\n"
				"(%s@%s connected via IRC). (socket = %d)", 
				CH(c)?CH(c)->name:"(unknown name)", c->remote_hostname, c->connected_socket);
			}else{
				sprintf( log_buf, "PERMITED PLAYER FROM BANNED SITE ACCEPTED\r\n"
				"(%s@%s connected via IRC). (socket = %d)",
				CH(c)?CH(c)->name:"(unknown name)", c->remote_hostname, c->connected_socket);
			}
			wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(CH(c)));
			log_string(log_buf);
		}
	}

	return false;
}
/**************************************************************************/
bool banedit_show (char_data *ch, char *);
int wild_match(register unsigned char *m, register unsigned char *n);
/**************************************************************************/
void display_bans(char_data *ch, char *site,int type)
{
    BAN_DATA *pban;
    char host[MSL];
	int i;

	// get hostname and make sure it is lowercase
    strcpy(host,site);
	for ( i = 0; host[i] != '\0'; i++ ){
		host[i] = LOWER(host[i]);
	}

	i=-1;
    for ( pban = ban_list; pban; pban = pban->next )     
	{
		i++;
		// check ban is enabled
		if(!pban->enabled)
			continue;

		// check ban type
		if(pban->type!=type)
			continue;
		
		// do wildcard mask pattern matching
		if (match_multiline_masks( pban->sitemasks, host)){
			ch->desc->pEdit = (void *)pban;
			ch->printlnf("Ban #%d", i);
			banedit_show( ch, "");
		}
    }

}
/**************************************************************************/
void do_checkban( char_data *ch, char * argument)
{
	void * pTemp = NULL;
	BAN_DATA *pBan;

	if(IS_NULLSTR(argument)){
		ch->println( "syntax: checkban <ispaddy of player>");
		return;
	}

	if(str_len(argument)<4){
		ch->println( "syntax: checkban <ispaddy of player>");
		ch->println( "Must be longer than 4 characters");
		return;
	}
	

	pTemp = ch->desc->pEdit;

	ch->printlnf( "==- CHECK BAN ON: '%s'",argument);
	ch->printlnf( "- Newbies: %s",
		(pBan=check_ban_for_site(argument,BAN_NEWBIE))?"`Rdenied`x":"`Gallowed`x");
	if(pBan){
		display_bans(ch, argument,BAN_NEWBIE);
	}
	ch->printlnf( "- Permit/Letgained check: %s", 
		(pBan=check_ban_for_site(argument,BAN_PERMIT))?"`Rdenied`x":"`Gallowed`x");
	if(pBan){
		display_bans(ch, argument,BAN_PERMIT);
	}
	ch->printlnf( "- Email check: %s", 
		(pBan=check_ban_for_site(argument,BAN_EMAIL_REQ))?"`Rdenied`x":"`Gallowed`x");
	if(pBan){
		display_bans(ch, argument,BAN_EMAIL_REQ);
	}
	ch->printlnf( "- All: %s", 
		(pBan=check_ban_for_site(argument,BAN_ALL))?"`Rdenied`x":"`Gallowed`x");
	if(pBan){
		display_bans(ch, argument,BAN_ALL);
	}

    ch->desc->pEdit = pTemp;

}

/**************************************************************************/
void do_showban( char_data *ch, char * argument)
{
	void * pTemp = NULL;
	BAN_DATA *pBan;
	int count;

	if(IS_NULLSTR(argument)){
		ch->println( "syntax: showban <number of ban in the ban list>");
		return;
	}

	count=atoi(argument);
    for ( pBan = ban_list; pBan; pBan = pBan->next ){
		if(--count<0)
			break;
	}

	if(!pBan){
		ch->println( "There aren't that many bans.");
		return;
	}
	
	pTemp = ch->desc->pEdit;

	ch->desc->pEdit = (void *)pBan;
	banedit_show( ch, "");
    ch->desc->pEdit = pTemp;
}
/**************************************************************************/
