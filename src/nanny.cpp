/**************************************************************************/
// nanny.cpp - Deal with sockets that haven't logged in yet
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

#include "comm.h"
#include "clan.h"
#include "nanny.h"
#include "roles.h"
#include "intro.h"
#include "offmoot.h"
#include "namegen.h"
#include "security.h"
#include "channels.h"
#include "msp.h"
#include "pload.h"

void roll_stats(connection_data *d);
int  count_creation_connections_per_hour(connection_data *d);
bool check_connection(connection_data *d);
void add_connection(connection_data *d);
void nanny_read_motd(connection_data *d, char *);
void nannysup_past_email_check(connection_data *d, const char *);
void nannysup_setprime_stats( char_data *ch );
void mp_login_trigger( char_data *ch);
void nsupport_newbie_alert( char_data *ch, bool created );
int name_confirmed;

#ifdef unix
	const   char    echo_off_str  [] = { IAC, WILL, TELOPT_ECHO, '\0' };
	const   char    echo_on_str   [] = { IAC, WONT, TELOPT_ECHO, '\0' };
	const   char    go_ahead_str  [] = { IAC, GA, '\0' };
#else
	const   char    echo_off_str    [] = { '\0', '\0', '\0' };
	const   char    echo_on_str [] = { '\0' };
	const   char    go_ahead_str    [] = { '\0' };
#endif

// locals
typedef struct creation_counter_data
{
    struct creation_counter_data* next;
	char *ip;
	time_t time;
} CREATION_COUNTER_DATA;

static CREATION_COUNTER_DATA* creation_counter;

DECLARE_DO_FUN( do_unread	);
DECLARE_DO_FUN( do_raceinfo	);
DECLARE_DO_FUN( do_classinfo );
void check_offline_letgain(char_data *ch);
void check_death_update(void);
void display_legal_message(char_data *ch);

/**************************************************************************/
char * creation_titlebar(char *fmt, ...)
{
	char buf [MSL];
	char line[MSL];
	static char returnbuf[MSL];
	int spaces;

	// format all the text into buf
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);


//=============================================================================\r\n", ch);	  
	if(c_str_len(buf)<1 || (c_str_len(buf)==1 && (buf[0]=='-' || buf[0]=='=') ))
	{
		return("-====================================="
			"=======================================-\r\n");
	}

	if(c_str_len(buf)>78)
	{
		sprintf(returnbuf,"%s\r\n",buf);
		return returnbuf;
	}


	spaces= (74-c_str_len(buf))/2;
	for(int j=0;j<spaces; j++)
	{
		line[j]='=';
	}
	line[spaces]='\0';

	sprintf(returnbuf,"-%s`# `=c%s `&%s-\r\n",line, buf, line);
	return returnbuf;

}
/**************************************************************************/
void connected_to_CON_REROLL_STATS(connection_data *d)
{
	roll_stats(d);
	add_connection(d);
	write_to_buffer(d,"If this is your first character here we recommend\r\n",0);
	write_to_buffer(d,"accepting these stats till you know what they all mean.\r\n",0);
	write_to_buffer(d,"(read the newbie doc on the web page for a light explaination)\r\n",0);
	d->connected_state = CON_REROLL_STATS;
}
/**************************************************************************/
// this is run when a player chooses to not customise, 
// if customisation is disabled in the gamesettings
// or they have choosen a class which has customising disabled
void nanny_new_player_not_customizing(connection_data *d)
{
	char_data *ch=d->character;
	group_add(ch,class_table[ch->clss].default_group,true, 1);
	write_to_buffer( d, "\r\n", 2 );
	if(IS_IRCCON(d)){
		do_help( ch, "irc-motd" );
	}else{
		do_help( ch, "motd" );
	}
	ch->hit_return_to_continue();
	d->connected_state = CON_READ_MOTD;
}

/**************************************************************************/
void connected_to_CON_GET_ALLIANCE(connection_data *d)
{
	CH(d)->printf("`1`=j%s", creation_titlebar("="));
	
	CH(d)->printf("%s", creation_titlebar("ALIGNMENT SELECTION"));
	CH(d)->printf("%s`x", creation_titlebar("="));
	CH(d)->printf("`1`=j%s`x", creation_titlebar("ALLIANCE"));
		write_to_buffer(d,
"You must now choose your alignment, alignment is very important as it \r\n"
"affects how you are expected to roleplay your character, and acting in a \r\n"
"manner out-of-alignment can invite penalties from the gods and nobles of\r\n"
"the realm.  Only pick an extreme alignment if you can and will live up\r\n"
"to it, otherwise you might do yourself great harm.\r\n\r\n",0);

	if(GAMESETTING(GAMESET_MAX_ALIGN_RANGE22)){
        write_to_buffer(d,
"The first half of you alignment is your ALLIANCE towards good and evil,\r\n"
"this value can be from -2 to 2 (including 0 being neutral) 2 being good\r\n"
"and -2 being evil. \r\n`WWhat value do you want for your alliance?`x ",0);
	}else{
        write_to_buffer(d,
"The first half of you alignment is your ALLIANCE towards good and evil,\r\n"
"this value can be from -3 to 3 (including 0 being neutral) 3 being extreme\r\n"
"good and -3 being extreme evil. \r\n`WWhat value do you want for your alliance?`x ",0);
	}

	if(IS_IRCCON(d)){
		write_to_buffer( d, "\r\n", 0 );
	}
	d->connected_state = CON_GET_ALLIANCE;
}
/**************************************************************************/
void connected_to_CON_GET_NEW_CLASS(connection_data *d)
{
	int iClass;
	char buf[MSL];

	CH(d)->printf("`=j%s`x", creation_titlebar("CLASS SELECTION"));
	strcpy( buf, "[" );
	for( iClass = 0; !IS_NULLSTR(class_table[iClass].name); iClass++ )
	{
		if(!class_table[iClass].creation_selectable
			|| class_table[iClass].remort_number > d->creation_remort_number)
			continue;
	
		if(race_table[CH(d)->race]->class_exp[iClass]<1000)
			continue;

		// perform remort_to_classes restrictions
		if(CH(d)->remort>0 
			&& !IS_NULLSTR(class_table[CH(d)->clss].remort_to_classes)
			&& !is_exact_name(class_table[iClass].name, class_table[CH(d)->clss].remort_to_classes)){
			continue;
		}

		if(iClass > 0 ){
			strcat( buf, " " );
		}
		strcat( buf, class_table[iClass].name );
		if(!GAMESETTING(GAMESET_CLASS_CREATION_NO_STAR)){
			if(IS_SET(class_table[iClass].flags, CLASSFLAG_MAGIC_ANTIPATHY)){
				strcat( buf, "`Y*`x" );
			}
		}
	}
	strcat( buf, "]");

	CH(d)->println("Select a class:");
	CH(d)->wrapln(buf);
	if(!GAMESETTING(GAMESET_CLASS_CREATION_NO_STAR)){
		CH(d)->println("note: classes with a `Y*`x can not use magical items.");
	}

	if(!GAMESETTING5(GAMESET5_CLASSINFO_DISABLED_IN_CREATION)){
		CH(d)->wraplnf(
		"Note: You can use `=Cclassinfo`x to obtain information about "
		"the prime attributes of classes, and base xp values for your "
		"choosen race (%s).", lowercase(race_table[CH(d)->race]->name));
	}
	CH(d)->println("--> " );
	if(IS_IRCCON(d)){
		CH(d)->println("");
	}

	d->connected_state = CON_GET_NEW_CLASS;
}
/**************************************************************************/
void nanny_char_version_updates(char_data *ch)
{
	char tempbuf[MSL];

	if(ch->version<8)
	{
        ch->practice+=3;
        ch->train+=2;
		int langsn=race_table[ch->race]->language->gsn;
		if(langsn>0){
			ch->pcdata->learned[langsn]=100;         
		}
        ch->language=race_table[ch->race]->language;
	}
   
    if(ch->version==0)
	{
        ch->practice=9;
        ch->train=5;
    }

    // reset players default short description 
    if(ch->short_descr[0]=='\0')
    {
        sprintf(tempbuf,"a %s %s",
            (ch->sex==0 ? "sexless" : ch->sex==1 ? "male" : "female"),
            race_table[ch->race]->name);
        ch->short_descr= str_dup(tempbuf);
    }

    if(ch->version<9){
		ch->printf("There is a problem with your character, version number is less than 9!\r\n"
			"Talk to the admin about it.\r\n");
    }
    // end of stat rolling 

}
/**************************************************************************/
void nanny_get_email(connection_data *c, const char *argument) 
{
	int result;
	char logbuf[MSL];

	if(IS_NULLSTR(argument)){
		write_to_buffer(c,"An email address is required sorry.\r\n",0);
		write_to_buffer(c,"Please type in your email address now:\r\n",0);
		return;
	}

	CH(c)->printf("`=j%s`x", creation_titlebar("="));	
	logf("Checking for email ban with check_email_ban(%d, %s)", 
		c->connected_socket, argument);
	result=check_email_ban(c, (char*)argument);
	logf("check_email_ban returned = %d", result);

	switch(result){		
	default:
		bugf("handle_get_email(): unknown result of %d from check_email_ban()!",result);
		do_abort(); // get a coredump and debug it
		break;
	case 0: // 0 accepted email, email an id code to them
		{
			char unlockbuf[20];
			sprintf(unlockbuf,"%X", number_range(0x100000,0xFFFFFF));
			// Log the email addy and details to file
			sprintf(logbuf,"%-13s from '%s' accepted email '%s', unlock = %s",
				CH(c)->name, c->remote_hostname, argument, unlockbuf);
			append_datetimestring_to_file( EMAILADDRESSES_FILE, logbuf);

			replace_string(CH(c)->pcdata->unlock_id, unlockbuf);
			email_descriptor_unlock_id(c);
			CH(c)->wraplnf("An email has been sent to '%s' "
				"with an unlock id", CH(c)->pcdata->email);
		}
		nannysup_past_email_check(c,argument); // finished handling the email checks
		break;
	case 1: // 1 email rejected, they need another attempt.
		// do nothing 
		sprintf(logbuf,"%-13s from '%s' rejected email '%s'",
			CH(c)->name, c->remote_hostname, argument);
		append_datetimestring_to_file( EMAILADDRESSES_FILE, logbuf);
		break;
	case 2: // 2 if they should be disconnected
		connection_close(c);
		break;	
	}	
}
/**************************************************************************/
// run when waiting for the IP addy to resolve on a newbie
void nanny_resolve_ip(connection_data *c, const char *argument) 
{
	if(c->resolved){
		write_to_buffer( c,"\r\n",0);
		nannysup_email_check(c,argument);	
	}else{ 
		if(c->outtop==0){
			c->write(".", 1);
			if(c->connected_state_pulse_counter%30==0){
				c->write("\r\n",2);
			}
		}

		if(++c->connected_state_pulse_counter>PULSE_PER_SECOND*60){
			// skip the whole process.
			c->resolved=true;
			write_to_buffer( c,"\r\nResolving timed out.",0);
#ifdef WIN32
			bugf("Resolver timed out for %s - if this happens all the "
				"time disable resolving in game settings or turn "
				"on local resolving. (note: the resolver hasnt' been "
				"fully developed for win9x)", c->remote_hostname);
#else
			bugf("Resolver timed out for %s", c->remote_hostname);
#endif
		}
	}
}
/**************************************************************************/
// called when someone has an id code they have to key in
void nanny_enter_unlock_id(connection_data *d, const char *argument)
{
	char buf[MSL];
	
	if(IS_NULLSTR(argument)){
		CH(d)->printf("Enter the unlock code for '%s' or type `=Cquit`x:`1",
			CH(d)->name);
		return;
	}

	if(!str_cmp("quit",argument)){
		CH(d)->printf("Disconnecting you now, come back when you have the code.`1");
		connection_close( d );
		return;
	}

	if(!str_cmp("resend",argument)){
		email_descriptor_unlock_id(d);
		CH(d)->wraplnf("An email has been sent to '%s' "
				"with an unlock id", CH(d)->pcdata->email);
		CH(d)->println("Download the email and type in the code.");
		return;
	}

	if(!str_cmp("change",argument)){
		replace_string(CH(d)->pcdata->email,"");
		CH(d)->printf("`=j%s`x", creation_titlebar("CHANGE EMAIL ADDRESS"));
				CH(d)->wrapln(
		"   You need to key in a new email address for your character.  Then the mud "
		"will email you an unlock key, with which you can unlock your character a single time "
		"during the logon process.");
				CH(d)->wrapln(
		"  The email address 9 times of out 10 will have to be a valid email account "
		"as part of either the isps domain or an email account on the server you "
		"are connecting through... e.g. if you are connecting from "
		"`W207-112-146.ipt.aol.com`x it is most likely that the email "
		"address you enter must be something like `Wusername@aol.com`x"
		"`1  The email address you type in will not be publically available... "
		"only being able to be seen by those who administer the mud.`1"
		"Please type in your email address now:");		
		d->connected_state = CON_RECHECK_EMAIL;
		return;
	}


	if(!str_cmp("delete",argument)){
		char lockedpfilename[MSL];
		strcpy(buf, pfilename( CH(d)->name, CH(d)->pcdata->pfiletype));
		sprintf(lockedpfilename, "%s%s", LOCKED_PFILES_DIR, pfile_filename(CH(d)->name)); 
		if(rename(buf,lockedpfilename)!=0){
			char errbuf[MSL];
			sprintf(errbuf,"nanny_enter_unlock_id(): error moving %s to %s as delete, error %d (%s) desc=%d", 
				buf, 
				lockedpfilename,
				errno, 
				strerror(errno),
				d->connected_socket);
			log_string(errbuf);
			bug(errbuf);
			autonote(NOTE_SNOTE, "nanny_enter_unlock_id()", "error moving pfile to the locked pfiles directory!", "imm", errbuf, true);
		}

		CH(d)->printf("%s has been deleted, disconnecting you now.`1", CH(d)->name);
		connection_close( d );
		return;
	}
	
	if(!str_cmp(CH(d)->pcdata->unlock_id,argument)){
		// correct code :)
		sprintf(buf, "%s - unlocked by %s", 
			CH(d)->pcdata->unlock_id, d->remote_hostname);
		#define validatedhost ch->act&W?"..":

		// move the pfile if required
		{
			PFILE_TYPE pt=get_pfiletype(CH(d));
			replace_string(CH(d)->pcdata->unlock_id, buf);
			PFILE_TYPE newpt=get_pfiletype(CH(d));
			if(pt!=newpt){
				rename(pfilename(CH(d)->name,pt),
					pfilename(CH(d)->name,newpt)); // move the file
			}
			CH(d)->pcdata->pfiletype=newpt;
			save_char_obj(CH(d)); // resave pfile
		}
		CH(d)->wraplnf(
"`1%s has been unlocked and saved!!! well done, `1"
"[press Y to continue]`1 :)",CH(d)->name);
        d->connected_state = CON_READ_MOTD;
		return;
	}else{ // incorrect code
		CH(d)->wraplnf(
"`1`1The code '%s' is incorrect...`1"
"This character '%s' can only be accessed with the unlock code that was "
"automatically emailed to '%s'.`1`1If you have that unlock code enter it now, "
"otherwise there are a few options - type:`1"
"`=Cquit`x to be disconnected`1"
"`=Cdelete`x to remove %s`1"	
"`=Cresend`x to have the unlock resent to '%s'`1"
"`=Cchange`x to change your email address.",
argument, CH(d)->name, CH(d)->pcdata->email, CH(d)->name, CH(d)->pcdata->email); 
		return;
	}

}
/**************************************************************************/
void nannysup_email_check(connection_data *d, const char *argument) 
{
	if(!d->resolved && resolver_running){
		write_to_buffer( d,"Resolving ip address, this can take up to 60 seconds... please wait.", 0);
		d->connected_state = CON_RESOLVE_IP;
		d->connected_state_pulse_counter=0;
		return;	
	}

	replace_string(CH(d)->pcdata->created_from, 
					FORMATF("%s(%s)", d->remote_ip,d->remote_hostname));

	if(check_ban(d,BAN_EMAIL_REQ)){
	CH(d)->printf("`=j%s`x", creation_titlebar("EMAIL VERFICATION REQUIRED"));
		CH(d)->wrapln(
"   It appears we have had problems at some stage with people that connect "
"from the isp or server you are connecting to us from.  In the past the "
"only way to deal with these types of problem players was to ban all "
"connections from the ISP or server involved.  This was obviously "
"not a long term solution to the problem so we have developed a system "
"that allows you to enter your email address in, and the mud will email "
"you an unlock key, with which you can unlock the character you are "
"creating either at the end of the creation process, or you will be "
"prompted for the unlock key when you next logon.");
		CH(d)->wrapln(
"  The email address 9 times of out 10 will have to be a valid email account "
"as part of either the isps domain or an email account on the server you "
"are connecting through... e.g. if you are connecting from "
"`W207-112-146.ipt.aol.com`x it is most likely that the email "
"address you enter must be something like `Wusername@aol.com`x"
"`1  The email address you type in will not be publically available... "
"only being able to be seen by those who administer the mud.`1"
"Please type in your email address now:");
		d->connected_state = CON_GET_EMAIL;
	}else{
		nannysup_past_email_check(d,argument);
	}

}
/**************************************************************************/
void nanny_recheck_email(connection_data *d, const char *argument) 
{
	int result;
	char logbuf[MSL];

	logf("Start handle_recheck_email");
	if(IS_NULLSTR(argument)){
		write_to_buffer(d,"An email address is required sorry.\r\n",0);
		write_to_buffer(d,"Please type in your email address now:\r\n",0);
		return;
	}

	CH(d)->printf("`=j%s`x", creation_titlebar("="));	
	logf("Going into check_recheck_email argument='%s'", argument);
	result=check_email_ban(d, (char*)argument);
	logf("Result = %d", result);

	switch(result){		
	default:
		bugf("handle_get_email(): unknown result of %d from check_email_ban()!",result);
		do_abort(); // get a coredump and debug it
		break;
	case 0: // 0 accepted email, and id code emailed to them.
		{
			char unlockbuf[20];
			sprintf(unlockbuf,"%X", number_range(0x100000,0xFFFFFF));

			// Log the email addy and details to file
			sprintf(logbuf,"%-13s from '%s' accepted email '%s', unlock = %s",
				CH(d)->name, d->remote_hostname, argument, unlockbuf);
			append_datetimestring_to_file( EMAILADDRESSES_FILE, logbuf);
			// move the pfile if required
			{
				save_char_obj(CH(d));
				PFILE_TYPE oldpt=get_pfiletype(CH(d));
				replace_string(CH(d)->pcdata->unlock_id, unlockbuf);
				email_descriptor_unlock_id(d);
				PFILE_TYPE pt=get_pfiletype(CH(d));
				if(oldpt!=pt){
					rename(pfilename(CH(d)->name,oldpt),
						pfilename(CH(d)->name,pt)); // move the file
					CH(d)->pcdata->pfiletype=pt;
				}
				save_char_obj(CH(d)); // update the lock key in the pfile
			}
			CH(d)->wraplnf("An email has been sent to '%s' "
				"with an unlock id.", CH(d)->pcdata->email);
		}
		nanny_read_motd(d,"");
		break;
	case 1: // 1 email rejected, they need another attempt.
		// do nothing 
		sprintf(logbuf,"%-13s from '%s' rejected email '%s'",
			CH(d)->name, d->remote_hostname, argument);
		append_datetimestring_to_file( EMAILADDRESSES_FILE, logbuf);
		break;
	case 2: // 2 if they should be disconnected
		connection_close(d);
		break;	
	}
	logf("End of handle_get_email()");
}
/**************************************************************************/
// find the vnum players start in
int get_startvnum(char_data *ch)
{
	if(IS_IRC(ch) && get_room_index(ROOM_VNUM_STARTIRC)){
		return ROOM_VNUM_STARTIRC;
	}

	if(get_room_index(ROOM_VNUM_STARTTELNET)){
		return ROOM_VNUM_STARTTELNET;
	}

	return ROOM_VNUM_OOC;
};
/**************************************************************************/
void do_save_gamesettings(char_data *ch, char *);
void do_outfit( char_data *ch, char *);
/**************************************************************************/
void nanny_read_motd(connection_data *d, char *)
{
    char_data *ch = d->character;
	char buf[MSL];
	char automaticbuf[MSL];
	BAN_DATA *pban;
	int sn;
	
	if(ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
	{
		write_to_buffer( d, "Warning! Null password!\r\n",0 );
		write_to_buffer( d, "Please report old password with bug.\r\n",0);
		write_to_buffer( d,
			"Type 'password null <new password>' to fix.\r\n",0);
	}
	
	reset_char(ch);
	
	if(!IS_NULLSTR(ch->pcdata->unlock_id) && 
		str_len(ch->pcdata->unlock_id)==6){
		CH(d)->printf("`1`=j%s`x", creation_titlebar("UNLOCK CODE REQUIRED"));
		if(d->newbie_creating){
			save_char_obj(ch);
			ch->wraplnf(
				"An unlock code for %s was automatically emailed to %s earlier during "
				"the character creation process.  This code is now "
				"required for the one time unlocking of %s. "
				"Your characters file has been saved, so if the email has not arrived yet "
				"you can type `=Cquit`x now to disconnect, and then login later after you "
				"have the code for unlocking.`1`1Enter your unlock code or type `=Cquit`x:",
				ch->name, ch->pcdata->email, ch->name);
		}else{
			ch->wraplnf(
				"This character '%s' can only be accessed with the unlock code that was "
				"automatically emailed to '%s'.`1`1If you have that unlock code enter it now, "
				"otherwise there are a few options, type `=Cquit`x to be disconnected, "
				"`=Cdelete`x to remove %s, `=Cresend`x to have the unlock resent to '%s'"
				"`=Cchange`x to change your email address.",
				ch->name, ch->pcdata->email, ch->name, ch->pcdata->email); 
		}
		d->connected_state = CON_ENTER_UNLOCK_ID;
		return;
	}

	pban=check_ban(d,BAN_EMAIL_REQ);
	if(pban){
		logf("quick check '%s'", ch->pcdata->email);
		if(IS_NULLSTR(ch->pcdata->email)){
			CH(d)->printf("`=j%s`x", creation_titlebar("EMAIL VERFICATION REQUIRED"));
				CH(d)->wrapln(
		"   Since you created your character it appears we have had problems with "
		"with players that that connect from the isp or server you are connecting to us from.  "
		"In the past the only way to deal with these types of problem players was to ban all "
		"connections from the ISP or server involved.  This was obviously "
		"not a long term solution to the problem so we have developed a system "
		"that allows you to enter your email address in, and the mud will email "
		"you an unlock key, with which you can unlock your character a single time "
		"during the logon process.");
				CH(d)->wrapln(
		"  The email address 9 times of out 10 will have to be a valid email account "
		"as part of either the isps domain or an email account on the server you "
		"are connecting through... e.g. if you are connecting from "
		"`W207-112-146.ipt.aol.com`x it is most likely that the email "
		"address you enter must be something like `Wusername@aol.com`x"
		"`1  The email address you type in will not be publically available... "
		"only being able to be seen by those who administer the mud.`1"
		"Please type in your email address now:");
			d->connected_state = CON_RECHECK_EMAIL;
			return;
		}else{  // they have already typed their email at some stage earlier
				// check that is it still valid
			int result=check_email_ban(d, ch->pcdata->email);

			switch(result){
			default:
				bugf("nanny_read_motd(): unknown result of %d from check_email_ban()!",result);
				do_abort(); // get a coredump and debug it
				break;
			case 0: // 0 still accepted email, do nothing
				break;

			case 1: // 1 email rejected since they last logged on
				CH(d)->printf("`=j%s`x", creation_titlebar("EMAIL VERFICATION REQUIRED"));
					CH(d)->wraplnf(
			"   It appears we have had problems with with players that that connect from "
			"the isp or server you are connecting to us from.  "
			"Your email address that you typed in is no longer accepted because the restrictions "
			"on email addresses from your ISP has been increased.  As a result you will need "
			"to enter a different email address than '%s'", ch->pcdata->email);
					CH(d)->wrapln(
			"  The email address 9 times of out 10 will have to be a valid email account "
			"as part of either the isps domain or an email account on the server you "
			"are connecting through... e.g. if you are connecting from "
			"`W207-112-146.ipt.aol.com`x it is most likely that the email "
			"address you enter must be something like `Wusername@aol.com`x"
			"`1  The email address you type in will not be publically available... "
			"only being able to be seen by those who administer the mud.`1"
			"Please type in your email address now:");
				d->connected_state = CON_RECHECK_EMAIL;
				return;
			case 2: // 2 if they should be disconnected - currently not used by check_email_ban()
				connection_close(d);
				return;
			}
		}
	}

	// do multilogging checks
	replace_string(ch->remote_ip_copy,d->remote_ip);	
	if(!HAS_CONFIG(ch,CONFIG_IGNORE_MULTILOGINS)){
		for(char_data* pch=player_list; pch; pch=pch->next)
		{
			if(!HAS_CONFIG(pch,CONFIG_IGNORE_MULTILOGINS)){
				if(!strcmp(ch->remote_ip_copy, pch->remote_ip_copy)){
					d->multiple_logins=true;
					if(TRUE_CH(pch)->desc){
						TRUE_CH(pch)->desc->multiple_logins=true;
					}
				};
			}
		};
		if(d->multiple_logins){
			// notify wiznet
			multilog_alertf(ch, "`YPossible multilog by `W%s `Y(%s)", 
				ch->name, ch->remote_ip_copy);
		}
	}

	
	// add them to the player_list
	ch->next_player = player_list;
    player_list = ch;
	
	// add them to the character list
	ch->next	= char_list;
	char_list	= ch;
	
	// safety check they have at least 0 karns
	if(ch->pcdata->karns<0){
		check_death_update();
		return;
	}

	ch->wraplnf("`1Welcome to %s.  Enjoy your stay and try to leave smiling.", MUD_NAME);

	if(d->newbie_creating){
		// tell the imms
		char newplayerbuf[MSL];
		sprintf(newplayerbuf, "Newbie %s completed creation in %s, reroll=%d", 
			ch->name, short_timediff(ch->player_id, current_time),ch->pcdata->reroll_counter);
		log_string(newplayerbuf);
		wiznet(newplayerbuf, ch,NULL,WIZ_NEWBIE,0,0);	
		// tell everyone else
		info_broadcast(ch, "Welcome %s, the newest traveler to this realm!", ch->name);
		d->newbie_creating=false;
	}
	
	ch->beginning_remort=0;
    d->connected_state = CON_PLAYING;

	// newbie starting out
	if(ch->level == 0 )
	{
		// setting recall_vnum to the race recall
		int start_room;
		
		ch->level	= 1;
		ch->exp	= exp_per_level(ch,ch->pcdata->points);
			ch->hit = ch->max_hit;
		ch->mana    = ch->max_mana;
		ch->move    = ch->max_move;

		// allocate a new players starting amounts - Daos, Oct03
		ch->train = game_settings->newbie_start_train; // default 5
		ch->practice = game_settings->newbie_start_practice; // default 10
		ch->silver = game_settings->newbie_start_silver; // default 100
		ch->gold = game_settings->newbie_start_gold; // default 0

		// mark most of the notes and ideas etc as read
		ch->pcdata->last_note   = current_time-21600;  // 6 hours
		ch->pcdata->last_idea   = current_time-21600;
		ch->pcdata->last_penalty= current_time;
		ch->pcdata->last_news   = current_time-172800; // 2 days
		ch->pcdata->last_changes= current_time-172800;
		
		if(get_obj_index(OBJ_VNUM_WORLD_MAP)) // load world map if it exists
            obj_to_char(create_object(get_obj_index(OBJ_VNUM_WORLD_MAP)),ch);

		if(get_obj_index(OBJ_VNUM_NEWBIE_GUIDE )) // load newbie guide if it exists
            obj_to_char(create_object(get_obj_index(OBJ_VNUM_NEWBIE_GUIDE)),ch);

		// find a pendant for the character based on their class
		if( GAMESETTINGMF1( GAMESETMF1_PENDANTS_ENABLED ) ){
			int class_pendant_vnum=class_table[ch->clss].pendant_vnum;
			if(class_pendant_vnum){
				obj_index_data *pendant_obj_index=get_obj_index(class_pendant_vnum);
			
				if(pendant_obj_index){
					if(pendant_obj_index->item_type==ITEM_PENDANT){
						obj_data *obj = create_object(pendant_obj_index);
						obj->cost = 0;
						obj_to_char( obj, ch );
						equip_char( ch, obj, WEAR_PENDANT );
						ch->println("You have mysteriously been equipped with a pendant for recalling near you guild.");	
					}else{
						bugf("nanny_read_motd(): recall pendant %d for class %s, is not of type pendant... ignoring.",
							pendant_obj_index->vnum,
							class_table[ch->clss].name);
					}
				}
			}
		}


		if(!GAMESETTING2(GAMESET2_DONT_AUTOOUTFIT_ON_NEWBIE_LOGIN)){
			do_outfit(ch,"");
		}

/*		// not supported at this stage
		if(GAMESETTING2(GAMESET2_NEWBIES_GET_ALL_MAPS)){
			if(get_obj_index(OBJ_VNUM_EVIL_CITY_MAP)){ // load main evil city map if it exists 
				obj_to_char(create_object(get_obj_index(OBJ_VNUM_EVIL_CITY_MAP)),ch);
			}
			if(get_obj_index(OBJ_VNUM_GOOD_CITY_MAP)){ // load main good city map if it exists 
				obj_to_char(create_object(get_obj_index(OBJ_VNUM_GOOD_CITY_MAP)),ch);
			}
		}else{
			if(race_table[ch->race]->recall_room==ROOM_VNUM_EVIL_RECALL)
			{		// evil city 
				if(get_obj_index(OBJ_VNUM_EVIL_CITY_MAP)){ // load main evil city map if it exists 
					obj_to_char(create_object(get_obj_index(OBJ_VNUM_EVIL_CITY_MAP)),ch);
				}
			}
			else	// good city 
			{
				if(get_obj_index(OBJ_VNUM_GOOD_CITY_MAP)){ // load main good city map if it exists 
					obj_to_char(create_object(get_obj_index(OBJ_VNUM_GOOD_CITY_MAP)),ch);
				}
			}
		}
		*/
		// give them their races newbie map
		if(race_table[ch->race]->newbie_map_vnum){		
			if(get_obj_index(race_table[ch->race]->newbie_map_vnum)){ 
				obj_to_char(create_object(get_obj_index(race_table[ch->race]->newbie_map_vnum)),ch);
			}
		}

		// give them their class's map if set
		if(class_table[ch->clss].newbie_map_vnum){
			obj_index_data *class_map_index=get_obj_index(class_table[ch->clss].newbie_map_vnum);
			if(class_map_index && class_map_index->item_type==ITEM_MAP){
				obj_to_char(create_object(class_map_index),ch);
			}
		}
	
		// sending newbies to different starting locations based on connection
        start_room = get_startvnum(ch);	
        if(get_room_index(start_room)!=NULL){
			char_to_room( ch, get_room_index(start_room));
        }else{
            ch->printf( "BUG: the starting room for you race "
				"doesn't exist (room number %d).\r\n", start_room);
            bugf("BUG: the starting room for you race "
				"doesn't exist (room number %d).\r\n", start_room);
			if(get_room_index(ROOM_VNUM_LIMBO)==NULL){
	            bugf("BUG: get_room_index(ROOM_VNUM_LIMBO)==NULL"
					"ROOM_VNUM_LIMBO = %d!  Saving game settings, change it in there.\r\n", ROOM_VNUM_LIMBO);
				do_save_gamesettings(NULL, "");
				exit_error( 1 , "nanny_read_motd", "missing limbo!");
			}
            char_to_room( ch,  get_room_index(ROOM_VNUM_LIMBO));            
        }		
		ch->println("\r\n");
		do_help(ch,"");
		ch->println("\r\n"); 
		
    }else if(ch->in_room){
		char_to_room( ch, ch->in_room );
	}else{
		ROOM_INDEX_DATA *target_room= get_room_index(race_table[ch->race]->recall_room);

		if(!target_room){
			bugf("nanny_read_motd(): Couldn't find room %d to put player in "
				"(as per race_table[ch->race]->recall_room)... putting them in limbo instead.",
				race_table[ch->race]->recall_room);
			target_room= get_room_index( ROOM_VNUM_LIMBO );
		}
        char_to_room(ch, target_room);
    }

	if(!GAMESETTING2(GAMESET2_NO_SECOND_SKILL_REQUIRED)){
		if(get_eq_char(ch, WEAR_SECONDARY) && !IS_NPC(ch) && ch->get_skill(gsn_second)<1){
			logf("removing second weapon from %s - doesn't have the skill to use it", ch->name);
			unequip_char( ch, get_eq_char(ch, WEAR_SECONDARY));
		}
	}
	
	// automatically turn on their IC object restrictions
	SET_CONFIG(ch, CONFIG_OBJRESTRICT); 

	if(IS_SET(ch->act,PLR_AUTOMAP)){
		SET_CONFIG(ch,CONFIG_AUTOMAP);
		REMOVE_BIT(ch->act,PLR_AUTOMAP);
	}

	// conversion to comm field
	if(IS_SET(ch->act, PLR_SPECIFY_SELF)){
		SET_BIT(ch->comm,COMM_AUTOSELF);
		REMOVE_BIT(ch->act, PLR_SPECIFY_SELF);
	}
    /*********************************************
	* turn on all their channels automatically *
	*********************************************/
	automaticbuf[0]='\0';
    //  quiet channels - turn off 
    if(HAS_CHANNELOFF(ch, CHANNEL_QUIET))
    {
        sprintf(buf,"Quiet mode has been automatically removed.\r\n");
		strcat(automaticbuf,buf);		
		REMOVE_CHANNELOFF(ch, CHANNEL_QUIET);
    }
	
    //  OOC channel turn on/off depending on nochannel status
	if(IS_SET(ch->comm, COMM_NOCHANNELS))
	{
        sprintf(buf, "`COOC channel has been automatically turned OFF.\r\n");
		strcat(automaticbuf,buf);		
        SET_CHANNELOFF(ch, CHANNEL_OOC);
	}
	else{
		if(HAS_CHANNELOFF(ch, CHANNEL_OOC))
		{
			sprintf(buf, "`COOC channel has been automatically turned ON.\r\n");
			strcat(automaticbuf,buf);		
			REMOVE_CHANNELOFF(ch, CHANNEL_OOC);
		}
		if(HAS_CHANNELOFF(ch, CHANNEL_CHAT))
		{
			sprintf(buf, "`CCHAT channel has been automatically turned ON.\r\n");
			strcat(automaticbuf,buf);		
			REMOVE_CHANNELOFF(ch, CHANNEL_CHAT);
		}
	}
	
    //  Q/A channel turn on 
    if(HAS_CHANNELOFF(ch, CHANNEL_QA))
    {
		sprintf(buf, "`gQ/A channel has been automatically turned ON.\r\n");
		strcat(automaticbuf,buf);		
        REMOVE_CHANNELOFF(ch, CHANNEL_QA);
    }

    //  Newbie channel turn on 
    if(HAS_CONFIG( TRUE_CH(ch), CONFIG_NONEWBIE ))
    {
		sprintf(buf, "`SNewbie channel has been automatically turned ON.\r\n");
		strcat(automaticbuf,buf);		
        REMOVE_CONFIG( TRUE_CH(ch), CONFIG_NONEWBIE );
    }
	
    //  AFK turn off 
    if(IS_SET(ch->comm,COMM_AFK))
    {
        sprintf(buf, "`MAFK mode has been automatically turned OFF.\r\n");
		strcat(automaticbuf,buf);		
        REMOVE_BIT(ch->comm,COMM_AFK);
    }

	if(ch->level>=LEVEL_IMMORTAL && IS_SET(ch->comm, COMM_NEWBIE_SUPPORT)){
		ch->println("Removed your newbie support status as immortals always helps newbies ;)");
        REMOVE_BIT(ch->comm, COMM_NEWBIE_SUPPORT);
	}
	
    // don't allow levels or trust greater than ABSOLUTE_MAX_LEVEL
	if(ch->level>ABSOLUTE_MAX_LEVEL)
	{
		ch->level= LEVEL_IMMORTAL;
	}
	if(ch->trust>ABSOLUTE_MAX_LEVEL)
	{
		ch->trust= LEVEL_IMMORTAL;
	}
	
    // turn on imm things
    if(ch->level >= LEVEL_IMMORTAL )
    {
		// turn wizi on if desired
		if(HAS_CONFIG2(ch, CONFIG2_AUTOWIZILOGIN) && !INVIS_LEVEL(ch))
		{
			sprintf(buf, "`YYou have been made wizi %d automatically.\r\n",
				LEVEL_IMMORTAL );
			strcat(automaticbuf,buf);		
			ch->invis_level=LEVEL_IMMORTAL;
		}
		
		// turn whoinvis on if desired
		if(HAS_CONFIG2(ch, CONFIG2_AUTOWHOINVISLOGIN)
			&& IS_SET(TRUE_CH(ch)->comm, COMM_WHOVIS))
		{			
			REMOVE_BIT(TRUE_CH(ch)->comm, COMM_WHOVIS);
			sprintf( buf,"`rYou have been made whoinvis automatically.\r\n");
			strcat(automaticbuf,buf);		
		}
		
        //  IMMTALK turn on 
        if(HAS_CHANNELOFF(ch, CHANNEL_IMMTALK)){
            sprintf(buf,"`=IImmortal channel has been automatically turned ON.\r\n");
			strcat(automaticbuf,buf);		
			REMOVE_CHANNELOFF(ch, CHANNEL_IMMTALK);
        }
		
		{
			for(int ti=0; ti<4; ti++){
				if(!IS_SET(ch->wiznet[ti],WIZ_ON) && IS_SET(ch->wiznet[ti],WIZ_AUTOON) )
				{
					switch(ti){
						case 0: sprintf(buf, "Immwiznet autoactivated.\r\n"); break;
						case 1: sprintf(buf, "Wiznet autoactivated.\r\n"); break;
						case 2: sprintf(buf, "Wiznet2 autoactivated.\r\n"); break;
						case 3: sprintf(buf, "Wiznet3 autoactivated.\r\n"); break;
					}
					strcat(automaticbuf,buf);		
					SET_BIT(ch->wiznet[ti],WIZ_ON);
				}
			}
		}
		
    }
    strcat(automaticbuf,"`x");
   /******************************************************
	* end of turning on all their channels automatically *
	******************************************************/

	// tell them about ircwhite and irc colour modes if appropriate
	if(IS_NEWBIE(ch) && IS_IRC(ch)){
		ch->println("NOTE: You can change between black and white backgrounds by typing:");
		ch->println("   `=Ccolour mode ircwhite`x and `=Ccolour mode irc`x.");
	}

	attach_know(ch); // setups ch->know stuff
	assertp(ch->know);

	if(!GAMESETTING2(GAMESET2_DONT_DISPLAY_WHO_4_LOGIN)){
		// give autowho to everyone 
		// - doesn't show imms automatically though for morts
	    do_who( ch, "-noimm4morts" );
	}

	// update their msp settings
	msp_update_char(ch);

	ch->print(automaticbuf);

	check_offline_letgain(ch);

    // Reset lastused on all everything for imms
	if(ch->level>=LEVEL_IMMORTAL){
		for( sn = 0; sn < MAX_SKILL; sn++ ){
			if(!IS_NULLSTR(skill_table[sn].name)){
				ch->pcdata->last_used[sn] = 0;
			}
		}
	}
	
	if(ch->pet){
		if(ch->pet->in_room){
			char_to_room(ch->pet,ch->pet->in_room); // restore to room they left from
		}else{
			char_to_room(ch->pet,ch->in_room); // restore to room the master
		}
		act("$n has entered the realm.",ch->pet,NULL,NULL,TO_ROOM);		
	}

	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD))
	{
		char_data *victim=ch;
		int newlevel=UMAX(number_range(LEVEL_HERO-10,LEVEL_HERO-1),ch->level);    
		ch->pcdata->p9999kills=ch->pcdata->p9999defeats=0;
		
		
		if(ch->level<10){
			ch->pksafe=10;
		}
		if(!IS_LETGAINED(ch)){
			SET_BIT(ch->act,PLR_CAN_ADVANCE);
			ch->println("`GYOU HAVE BEEN AUTOMATICALLY LETGAINED!!!.`x\r\n");
		};
        if(ch->level<LEVEL_IMMORTAL ){
			int iLevel;
            int temp_prac;
			
			ch->pknoquit=4;
			ch->pksafe=3;
			
            ch->printf("`BAUTOMATICALLY SETTING YOU TO LEVEL %d!!!`x", newlevel);
			
            if(victim->level<newlevel){
                temp_prac = victim->practice;
                victim->level    = 1;
                victim->exp      = exp_per_level(victim,victim->pcdata->points);
                victim->max_hit  = 10;
                victim->max_mana = 100;
                victim->max_move = 100;
                victim->practice = 0;
                victim->hit      = victim->max_hit;
                victim->mana     = victim->max_mana;
                victim->move     = victim->max_move;
                advance_level( victim );
                victim->practice = temp_prac;
            }
            for( iLevel = ch->level ; iLevel < newlevel; iLevel++ )
			{
				ch->level += 1;
				advance_level( ch);
			}
			ch->exp   = exp_per_level(ch,ch->pcdata->points)
				* UMAX( 1, ch->level );
		}
		
        {
            ch->println("`GSETTING ALL MORTAL SKILLS TO 101%.`x\r\n");
            for( sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++ )
            {
				if(IS_SET(skill_table[sn].flags,SKFLAGS_NO_GAIN)){
					continue;
				}
				if(IS_SPELL(sn) 
					&& !IS_SET(skill_table[sn].flags, SKFLAGS_LEVEL_SPGAIN)
					&& !IS_SET(skill_table[sn].flags, SKFLAGS_STUDY_SPGAIN))
				{
					 continue;
				}

				// check if no class can get it
				bool a_class_can_get=false;
				{
					int cl;
					
					for(cl=0;  !IS_NULLSTR(class_table[cl].name); cl++){
						if(class_table[cl].creation_selectable){
							if(skill_table[sn].rating[cl] > 0
								&&  skill_table[sn].skill_level[cl]< LEVEL_IMMORTAL
								&&  skill_table[sn].skill_level[cl]>0)
							{
								a_class_can_get=true;
							}

						}
					}
						
				}
				if(!a_class_can_get){
					continue;
				}
				victim->pcdata->learned[sn] = 101;
            }
        }
		reset_char(victim);
		
		victim->max_hit+=number_range(40,200);
		victim->max_mana+=number_range(40,200);
		victim->max_move+=number_range(40,200);
		victim->gold+=number_range(40,400);
		
		// login restore	
		victim->subdued = false;
		if(!IS_NPC(victim) && victim->pcdata->tired!=-1)
		{
			victim->pcdata->tired=0;
		}
		
		affect_parentspellfunc_strip(victim,gsn_plague);
		affect_parentspellfunc_strip(victim,gsn_poison);
		affect_parentspellfunc_strip(victim,gsn_blindness);
		affect_parentspellfunc_strip(victim,gsn_sleep);
		affect_parentspellfunc_strip(victim,gsn_curse);
		affect_parentspellfunc_strip(victim,gsn_cause_fear);
		affect_parentspellfunc_strip(victim,gsn_fear_magic);
		
		victim->hit 	= victim->max_hit;
		victim->mana	= victim->max_mana;
		victim->move	= victim->max_move;
		victim->pcdata->tired=0;
		update_pos( victim);
		ch->println(  "`MYOU HAVE BEEN AUTOMATICALLY BOOSTED AND RESTORED!!!.`x");
		ch->printlnf( "`YYOU ARE PK SAFE FOR %d MINUTES!!!.`x",ch->pksafe);
		
		// announce their arrival
		if(!IS_IMMORTAL(ch)){
            pkill_broadcast("%s has entered the realm of death! [Pk=0.Pd=0,L=%d]",
				ch->name, ch->level);
		}		
	}

	// automatically letgain remorted players
	if(ch->remort>0 && !IS_LETGAINED(ch)){
		SET_BIT(ch->act,PLR_CAN_ADVANCE);
		ch->println("`GYOU HAVE BEEN AUTOMATICALLY LETGAINED!!!.`x\r\n");
	}

	ch->mxp_send_init();
		
	do_look( ch, "brief" );
	
	act( "$n has entered the realm.", ch, NULL, NULL, TO_ROOM );
	info_broadcast(ch, "%s has entered the realm.", ch->name);
	
    if(IS_IRCCON(d)){
		sprintf(buf, "`B%s has joined the game via IRC. [%s] (lvl %d, remort %d)`x",
			ch->name, ch->desc?ch->desc->remote_hostname:"no_descriptor", 
			ch->level, ch->remort);
	}else{
		sprintf(buf, "%s has joined the game via telnet. [%s] (lvl %d, remort %d)", 
			ch->name, ch->desc?ch->desc->remote_hostname:"no_descriptor", 
			ch->level, ch->remort);
	}
    wiznet(buf,ch,NULL, WIZ_LOGINS,0,UMIN(get_trust(ch), MAX_LEVEL));

	if(ch->level <= 10 && !HAS_CONFIG( ch, CONFIG_NONEWBIE )){
		nsupport_newbie_alert( ch, true );
	}
	
    laston_login(ch); // record the time the character logged on
	
	do_unread(ch,"");

	// display a legal notice about olc creation
	if(HAS_SECURITY(ch, 1) && !HAS_CONFIG2(ch, CONFIG2_READ_BUILDER_LEGAL)){
		display_legal_message(ch);
	}

	// tell MAX_LEVEL to set up their security.
	if(IS_TRUSTED(ch, MAX_LEVEL) && GET_SECURITY(ch)<1){
		ch->titlebar("INCREASE YOUR OLC SECURITY?");
		ch->wraplnf("`xYour olc security is currently set to 0, you need a security "
			"of %d to edit the game settings (by typing `=Cgameedit confirm`x).  "
			"You can increase your olc security to 9 (the maximum) by typing "
			"`=Cset char self security 9`x", GAMEEDIT_MINSECURITY);
		ch->wraplnf("`xHint: to list all the olc editors available, type `=Ccommand edit`x "
			"for the ones commonly used by builders and `=Cwiz edit`x for the "
			"immortal/administrative editors.");
		ch->titlebar("`S[The above message disappears once you have a security higher than 0]");
	}

	// tell users to upgrade their mud client if they have one with known issues
	if(!GAMESETTING2(GAMESET2_NO_MSG_ABOUT_OLD_MUDCLIENTS)){
		if(ch->desc && IS_SET(ch->desc->flags, CONNECTFLAG_MXP_SECURE_PREFIX_EACH_LINE) ){
			ch->titlebar("THERE ARE KNOWN MXP ISSUES WITH YOUR CURRENT MUD CLIENT");
			ch->wrapln("`xThis mud makes heavy use of MXP... there are known "
				"bugs in your particular mud client (MXP and MCCP).  It is "
				"recommended that you update your mud client if an update "
				"is available, or try another "
				"client - such as `WMuClient`x (available from "
				"`Bhttp://www.muclient.com`x)`1`1Alternatively if you notice "
				"unexpected output on your screen while using this client you may "
				"disable MXP and MCCP (view menu -> preferences -> "
				"general -> mxp) then restart the client.");
			ch->titlebar("");
		}
	}

	// message to tell mushclient users to turn off the forced link colour
	if(ch->desc 
		&& !IS_NULLSTR(ch->desc->mxp_options) 
		&& strstr(ch->desc->mxp_options, "use_custom_link_colour=1")){

		ch->titlebar("MXP RECOMMENDATION");
		ch->println("Turn 'use custom link colour' mxp option off in your mud client!");
		ch->wrapln("`xThis mud makes heavy use of colours in MXP links (especially in olc)... "
			"currently your mud client is configured to ignore the muds "
			"colour for links and has the 'use custom link colour' mxp option on..."
			" It is strongly recommended that you turn this option off for a better "
			"mudding experience.  (Try Ctrl+Alt+U to do this).");
		ch->titlebar("");
	}	

	// mudprog login trigger
	mp_login_trigger( ch);

	// offline moot kickin
	check_pending_moot(ch);

	// record their entry into the game into their plog if they have one
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG)){
        append_playerlog( ch, 
			FORMATF("%s has entered the game. (level %d), room=%d, lasticroom=%d", 
					ch->name, ch->level,
					ch->in_room?ch->in_room->vnum:0,
					ch->last_ic_room?ch->last_ic_room->vnum:0));
    }

}

/**************************************************************************/
void nanny_get_name(connection_data *d, char *argument)
{
    char_data *ch;
    bool pfile_loaded_okay;
	char buf[MSL];

	if(IS_NULLSTR(argument)){
		write_to_buffer( d, 
			"You can't have a blank name, type 'new' to create a new character,\r\n"
			"'quit' to disconnect, or type your characters name - try again.\r\n", 0);
		write_to_buffer(d, LOGIN_PROMPT, 0);
		return;
	}

	if(!str_cmp(argument,"quit")){
	    connection_close( d );
		return;
	}

	if(!str_cmp(argument,"dawnftp") || !str_cmp(argument,"mudftp")){
		logf("DawnFTP connection %d starting.", d->connected_socket);
		// 00 code indicates unsupported
		// 01 code indicates continue
		write_to_buffer(d, "\n:DAWNFTP:01 v1:transferring to mudftp authentication state.\n", 0);
		d->connected_state=CON_FTP_AUTH;
		return;
	}
	
	// check for invalid characters
	if(strstr(".", argument) || strstr("\\", argument) 
		|| strstr("/", argument)){
		write_to_buffer( d, "Invalid characters in name, "
			"try again.\r\nName:", 0);
		return;
	}

    argument=capitalize(argument);
	
	pfile_loaded_okay= load_char_obj( d, argument );
	ch   = d->character;
	d->newbie_creating = false;

	// do checks on what was read in
	// check for multiple pfiles found
	if(ch->pcdata->pfiletype==PFILE_MULTIPLE){
		ch->wraplnf("For some reason multiple copies of pfiles "
			"for '%s' found... to avoid loading the incorrect copy, your pfile "
			"can not be loaded.  Create a new character and then write a note to "
			"admin once in the game.", ch->name);
		ch->print("Name:");
		return;
	}

	// check for levels/trust/security etc
	if(pfile_loaded_okay)
	{
		int maxsecurity=0;
		int maxtrust=0;
		int maxlevel=LEVEL_HERO;
		
		switch(ch->pcdata->pfiletype){
		case PFILE_LOCKED:
			break;
		case PFILE_NORMAL:
			break;
		case PFILE_BUILDER:
			maxsecurity=9;
			break;
		case PFILE_TRUSTED:
			maxsecurity=9;
			maxtrust=MAX_LEVEL;
			break;
		case PFILE_IMMORTAL:
			maxsecurity=9;
			maxtrust=ABSOLUTE_MAX_LEVEL;
			maxlevel=ABSOLUTE_MAX_LEVEL;
			break;
		default:
			break;
		}

		if(ch->pcdata->security>maxsecurity){
			if(maxsecurity==9){
				maxsecurity=8;
			}
			ch->wraplnf("`RNOTICE: Your player file was loaded from a directory that "
				"limits olc security.  Your olc security was read in being higher than allowed, "
				"therefore your security has been changed from %d to %d!`x",
				ch->pcdata->security, maxsecurity);
			ch->pcdata->security=maxsecurity;
		}
		if(ch->trust>maxtrust){
			if(maxtrust==ABSOLUTE_MAX_LEVEL){
				maxtrust=MAX_LEVEL;
			}
			ch->wraplnf("`RNOTICE: Your player file was loaded from a directory that "
				"limits trust level.  Your trust was read in being higher than allowed, "
				"therefore your trust has been changed from %d to %d!`x",
				ch->trust, maxtrust);
			ch->trust=maxtrust;
		}
		if(ch->level>maxlevel){
			if(maxlevel==ABSOLUTE_MAX_LEVEL){
				maxlevel=MAX_LEVEL;
			}
			ch->wraplnf("`RNOTICE: Your player file was loaded from a directory that "
				"limits your level to that of a mortal.  Your level was read in as being higher "
				"than allowed, therefore your level has been changed from %d to %d!`x",
				ch->level, maxlevel);
			ch->level=maxlevel;
		}
	}


	if(IS_SET(ch->act, PLR_DENY)){
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->remote_hostname );
	    log_string( log_buf );
		wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(ch));
        d->write("You are denied access!\r\n", 0 );
		if(d){
			if(IS_IRCCON(d)){
				sprintf( log_buf, "DENIED PLAYER DROPPED (nanny)\r\n"
				"was %s@%s (connected via IRC). (socket = %d)",
				ch->name, d->remote_hostname, d->connected_socket );
			}else{
				sprintf( log_buf, "DENIED PLAYER DROPPED (nanny)\r\n"
				"was %s@%s (socket = %d)",
				ch->name, d->remote_hostname, d->connected_socket);
			}
			wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(ch));
			log_string(log_buf);
		}

	    connection_close(d);
	    return;
	}

	ch->mxp_send_init();

    if(check_connection_ban(d)){
		return;
	}

	if(check_reconnect( d, argument, false )){
	    pfile_loaded_okay= true;
	}else{ 
		if(GAMESETTING(GAMESET_PLAYER_LOCKED)){
			write_to_buffer( d, "\r\nThe game is playerlocked, only the administration can currently connect.\r\n", 0 );
			if(!IS_IMMORTAL(ch) && !ch->host_validated && !d->ident_confirmed){
				connection_close( d );
				return;
			}			
			write_to_buffer( d, "\r\n\r\nIF THE ADMIN ARE WANTING PLAYERS TO LOG INTO THE GAME, THEY NEED TO UNPLAYERLOCK THE GAME!\r\n\r\n", 0 );
	    }


#ifdef DAWN_STATIC_BETA_RELEASE
		// tell them about the code expiry
		{
			time_t expiry_date=DAWN_STATIC_BETA_RELEASE_EXPIRY_DATE;
			
			if(expiry_date<current_time){
				write_to_buffer( d, "\r\nThis version of the dawn code is a beta release which expired on:\r\n", 0);
				write_to_buffer( d, ctime(&expiry_date), 0);
				write_to_buffer( d, "Only immortals are able to connect until the version is replaced.\r\n", 0);
				write_to_buffer( d, "A new version should be available from www.dawnoftime.org.\r\n", 0);
				if(!IS_IMMORTAL(ch) && !ch->host_validated && !d->ident_confirmed){
					connection_close( d );
					return;
				}			
			}
		}
#endif 

	}

	if(pfile_loaded_okay){  
		if(ch->pcdata && ch->pcdata->colourmode){
			d->colour_mode=ch->pcdata->colourmode;
		}else{
			d->colour_mode=CT_NOCOLOUR;
		}

		if(!str_cmp( "-", ch->pcdata->pwd ))
        {
			write_to_buffer( d, "Your password has been wiped...\r\n", 0 );
            write_to_buffer( d, "Please set your password using the password command, then relogin.\r\n", 0 );
			write_to_buffer( d, "Syntax: password - <new password>.\r\n", 0 );
            d->connected_state = CON_READ_MOTD;      
        }
		else
		{
			// Old player 
			if(HAS_MXPDESC(d)){
				write_to_buffer( d, mxp_tagify("`WCharacter Password:`x <PASSWORD>"), 0 );
			}else{
				write_to_buffer( d, "`WCharacter Password:`x ", 0 );
			}
			if(IS_IRCCON(d)){
				write_to_buffer( d, "\r\n", 0 );
			}else{
				write_to_buffer( d, echo_off_str, 0 );
			}
			d->connected_state = CON_GET_OLD_PASSWORD;
		}
	    return;
	}
	else
	{
		if(!str_cmp("new", argument) 
			|| !str_cmp( "newMFExperiementalSystem", argument)
			){
			if(!d->ident_confirmed){
			if(GAMESETTING(GAMESET_PLAYER_LOCKED)){
				write_to_buffer( d, "Sorry, the game is currently playerlocked, no new players can be created at this point in time.\r\n", 0 );
				connection_close( d );
				return;
			}
			if(GAMESETTING(GAMESET_NEWBIE_LOCKED)){
				write_to_buffer( d, "Sorry, the game is currently newbielocked, no new players can be created at this point in time.\r\n", 0 );
				write_to_buffer( d, "Lets start over...\r\n", 0);
				write_to_buffer( d, LOGIN_PROMPT, 0);
				if(IS_IRCCON(d)){
					write_to_buffer(d,"\r\n", 2);
				}
				return;
			}
	        logf("Socket %d (%s) is considering making a new character", 
				d->connected_socket, d->remote_hostname);
			}
			write_to_buffer( d, "Do you really want to create a new character? (Y or N)", 0);
			if(IS_IRCCON(d)){
				write_to_buffer( d, "\r\n", 0);
			}

			// Determine how creation proceeds
			d->newbie_creating = true;
			if( GAMESETTINGMF1( GAMESETMF1_ENABLE_MF_EXPERIMENTAL_CREATION_SYSTEM ) 
				|| !str_cmp("newMFExperiementalSystem",argument))
			{
				d->newbie_creating = CREATING_MF_EXPERIMENTAL_SYSTEM;
			}else{
				d->newbie_creating = CREATING_DAWN_SYSTEM;
			}

			d->connected_state = CON_CONFIRM_CREATING_NEW;
			return;
		}

		// pfile not found for playername
	    sprintf( buf, 
			"I don't recognise the name '%s', if you wish to create a \r\n"
			"character called '%s' type 'new' (without the quotes).\r\n", 
			argument, argument);
		write_to_buffer( d, buf, 0);

		write_to_buffer( d, LOGIN_PROMPT, 0);
		if(IS_IRCCON(d)){
			write_to_buffer(d,"\r\n", 2);
		}
		// recycle the memory we just allocated
		free_char(d->character);
		d->character = NULL;
	    return;
	}
}
/**************************************************************************/
void nanny_get_automap(connection_data *d, char *argument)
{
    switch( *argument )
	{
	default:
		write_to_buffer(d,"Please type Yes or No ",0);
		return;

	case 'y' : case 'Y':
		SET_BIT(d->character->act,PLR_AUTOMAP);
		d->character->println("Automap activated!\r\n");
	    break;

	case 'n' : case 'N':
		REMOVE_BIT(d->character->act,PLR_AUTOMAP);
		d->character->println("Automap not activated.\r\n");
		break;
	}

	// start class selection
	connected_to_CON_REROLL_STATS(d);
	return;
}
/**************************************************************************/
void begin_remort( char_data *oc);
/**************************************************************************/
void nannysup_process_correct_connect_password(connection_data *d)
{
	char_data *ch = d->character;

	// remort patch in here
	if(ch->beginning_remort){
		begin_remort(ch);
		return;
	}

    if(IS_IRCCON(d)){
		sprintf( log_buf, "%s@%s has connected via IRC. (socket=%d, lvl=%d, remort=%d)", 
			ch->name, d->remote_hostname, ch->desc->connected_socket, ch->level, ch->remort);
	}else{
		sprintf( log_buf, "%s@%s has connected. (socket=%d, lvl=%d, remort=%d)", 
			ch->name, d->remote_hostname, ch->desc->connected_socket, ch->level, ch->remort);
	}
    log_string( log_buf );

	wiznet(log_buf,ch,NULL,WIZ_SITES,0, UMIN(get_trust(ch), MAX_LEVEL));

	nanny_char_version_updates(ch);
		
    if(ch->version>5 && ch->version!=0)
    {
        if(IS_IMMORTAL(ch) )
        {
			do_help( ch, "imotd" );
			ch->hit_return_to_continue();			
			d->connected_state = CON_READ_IMOTD;
        }
        else
        {
			if(IS_IRCCON(d)){
				do_help( ch, "irc-motd" );
			}else{
				do_help( ch, "motd" );
			}
			ch->hit_return_to_continue();
            d->connected_state = CON_READ_MOTD;
        }
	}
	else
	{
		if(ch->version==0)
        {
			int langsn=race_table[ch->race]->language->gsn;
			if(langsn>0){
				ch->pcdata->learned[langsn]=100;         
			}
			ch->language=race_table[ch->race]->language;
			ch->gen_data = new_gen_data();
			ch->gen_data->points_chosen = ch->pcdata->points;
			do_help(ch,"group-header");
			list_group_costs(ch);
			write_to_buffer(d,"You already have the following skills:\r\n",0);
			do_skills(ch,"");
			do_help(ch,"menu-choice");
			d->connected_state = CON_GEN_GROUPS;
			return;
		}
		ch->version=6;
		connected_to_CON_GET_ALLIANCE(d);
		return;
	}

}
/**************************************************************************/
void nanny_get_old_password(connection_data *d, char *argument)
{
    char_data *ch = d->character;
	write_to_buffer( d, "\r\n", 2 );

	if(ch->pcdata->overwrite_pwd){
		write_to_buffer( d, "Your password has been wiped...\r\n", 0 );
        write_to_buffer( d, "Please set your password using the password command, then relogin.\r\n", 0 );
		write_to_buffer( d, "Syntax: password - <new password>.\r\n", 0 );
	}else{
		if(!is_valid_password(argument, ch->pcdata->pwd, d))
		{
			logf( "Socket %d (%s) got the password wrong for %s!", 
				ch->desc->connected_socket, ch->desc->remote_hostname, ch->name);

			write_to_buffer( d, "Wrong password", 0 );

			if(++d->wrong_password_count<3){
				write_to_buffer( d, " - try again.\r\n", 0 );			
				write_to_buffer( d, "`WCharacter Password:`x ", 0 );
				if(IS_IRCCON(d)){
					write_to_buffer( d, "\r\n", 0 );
				}else{
					write_to_buffer( d, echo_off_str, 0 );
				}
				return;
			}else{
				write_to_buffer( d, ".\r\n", 0 );
			}

			// notify of wrong password - Daos Aug 04
			autonote(NOTE_ANOTE, "p_anote()", "Invalid Password", "admin law headlaw",
				FORMATF("Invalid Password:  Socket %d (%s) got the password wrong 3 times for %s!",
					ch->desc->connected_socket, ch->desc->remote_hostname, ch->name),true);
			connection_close( d );
			return;
		}
	}
    
    logf("Socket %d (%s) got the password correct for %s",
            ch->desc->connected_socket, ch->desc->remote_hostname, ch->name);

	// automatically extract a ploaded player if they login while ploaded
	{
		char_data *t=pload_find_player_by_name(ch->name);
		if(t){
			logf("automatically extracting the ploaded player '%s' due to login.", ch->name);
			pload_extract(NULL, t);
		}
	}

	if(ch->version==0)
	{
        logf("Socket %d - char version = 0", ch->desc->connected_socket);
        write_to_buffer( d, "Please remember to set your password when you finish creating.\r\n", 0 );
    }
	
	if(!IS_IRCCON(d)){
		write_to_buffer( d, echo_on_str, 0 );
	}
	write_to_buffer( d, "\r\n", 0 );

	// display the codebase advert
	if(!GAMESETTING2(GAMESET2_DONT_DISPLAY_CODEBASE_4_LOGIN)){
		ch->println("       `GThis mud is based on the Dawn of Time codebase`x");
	}

	// display last login info
	if(!IS_NULLSTR(ch->pcdata->last_logout_site)){
		char lastbuf[MSL];
		sprintf(lastbuf, "   You last logged in from %s\r\n"
			             "   and logged out at %s",
						ch->pcdata->last_logout_site,
						(char *) ctime( &ch->pcdata->last_logout_time));
		lastbuf[str_len(lastbuf)-1]='\0';
		strcat(lastbuf, FORMATF("\r\n   which was %s ago.\r\n\r\n",
			short_timediff(ch->pcdata->last_logout_time, current_time)));
        write_to_buffer( d, lastbuf, 0 );
	}

	if(check_playing(d,ch->name))
		 return;

	if(check_reconnect( d, ch->name, true ) )
		 return;

	if(ch->host_validated){
		ch->print(validatedhost"...");
	}else{
		// process a connection password - if there is one
		if(!IS_NULLSTR(game_settings->password_player_connect) 
			&& str_cmp(game_settings->password_player_connect, "-")
			&& !IS_IMMORTAL(ch)){
			ch->printf("`=j%s`x", creation_titlebar("CONNECT PASSWORD REQUIRED"));
			ch->wrapln("There is currently a player connect password set which you must "
				"enter in now before you can connect to the game`1Please enter the connection password:");
			d->connected_state=CON_GET_CONNECT_PASSWORD;
			return;
		}
	}
	nannysup_process_correct_connect_password(d);
}

/**************************************************************************/
void nanny_break_connect(connection_data *d, const char *argument)
{
	connection_data *d_old, *c_next;
	char name[MIL];
	
	strcpy(name, CH(d)?CH(d)->name:"???");
	
	switch( *argument )
	{
	case 'y' : case 'Y':
        logf("Socket %d (%s) to break connect (%s)", 
			d->connected_socket, d->remote_hostname, name);
		
        for( d_old = connection_list; d_old != NULL; d_old = c_next )
		{
			c_next = d_old->next;
			if(d_old == d || d_old->character == NULL)
				continue;
			
			if(str_cmp(name,d_old->original ?
				d_old->original->name : d_old->character->name))
				continue;
			
			connection_close(d_old);
		}
		if(check_reconnect(d,name,true)){
			return;
		}
		
        logf("Socket %d (%s) reconnect failed for %s", 
			d->connected_socket, d->remote_hostname, name);
		write_to_buffer(d,"Reconnect attempt failed.\r\nName: ",0);
		if(IS_IRCCON(d)){
			write_to_buffer( d, "\r\n", 0 );
		}
        if(d->character != NULL ){
            free_char( d->character );
            d->character = NULL;
        }
		d->connected_state = CON_GET_NAME;
		break;
		
	case 'n' : case 'N':
        sprintf( log_buf, "Socket %d (%s) decided not to force another (%s) off.", 
			d->connected_socket, d->remote_hostname, name);
        log_string(log_buf);
		
		write_to_buffer(d,"Name: ",0);
        if(d->character){
			free_char( d->character );
			d->character = NULL;
        }
		d->connected_state = CON_GET_NAME;
		break;
		
	default:
		write_to_buffer(d,"Please type Yes or No ",0);
		if(IS_IRCCON(d)){
			write_to_buffer( d, "\r\n", 0 );
		}
		
		break;
	}
}
/**************************************************************************/
// send the name selection prompt to the new player
void nannysup_name_select_prompt(connection_data *d)
{
	char_data *ch=d->character;
	ch->printlnf("Name selector commands: <1-%d>, list, help, requirements\r\n"
		"Enter your characters name or name selector command: ", profile_count);

	if(IS_IRCCON(d)){
		ch->println("");
	}
}
/**************************************************************************/
void nannysup_begin_name_select(connection_data *d)
{
	char_data *ch=d->character;

	ch->printf("`=j%s`x", creation_titlebar("NAME REQUIREMENTS"));
	ch->lines += 15;
	if(!codehelp(CH(d), "creation_name_requirements", false)){
	ch->wrapln(		
		"The first stage in creating a character is to come up with a name "
		"for it. There are many things to consider when naming your character, "
		"here are some requirement your character's name must meet:"
		"`1"
		"`1- Be creative. Do not give your character the name of another character"
		"`1  you like in a fantasy book."
		"`1- Do not give your char a descriptive name such as CoolGuy, SexyJane,"
		"`1  WeaponMaster or GreatFighter."
		"`1- Don't put titles in your name such as SirCrosash or LadyElbriana"
		"`1- Do not put nouns, adjectives or adverbs. If a word has a meaning,"
		"`1  keep it off your name. Examples: Slasher, TheShadow, RedDrac, Bloody."
		"`1- Swear words and names from earthly religions are not allowed."
		"`1- The administration reserve the right to change your name for any"
		"`1  reason what so ever, and will most likely do so if could be in"
		"`1  anyway disrespectful, or in violation of any of the above points."
		"`1"
		"`1All that said, we do have a random name generator integrated into "
		"the name selection process.  For instructions on using the name generator "
		"type 'help' now (without the quotes).`1");
	}
	ch->lines -= 15;
	nannysup_name_select_prompt(d);
	d->connected_state=CON_NAME_SELECT;
}
/**************************************************************************/
void nanny_name_select(connection_data *d, char *argument)
{
	char_data *ch=d->character;
	name_profile *nl;
	name_confirmed=0;

	// show help
	if(!str_cmp("help", argument)){
		ch->printf("`=j%s`x", creation_titlebar("NAME SELECTOR HELP"));
		ch->wrapln(
			"The name selector gives you the option to type in a name you would "
			"like to give the character you are creating or can be used to suggest "
			"possible names you might like to consider."
			"`1The names the selector generates are randomized sequences of letters "
			"based on various naming profiles... these naming profiles give names of "
			"a certain 'style'."
			"`1"
			"`1Name selector commands:"
			"`1  '`=Chelp`x' shows this help screen."
			"`1  '`=Crequirements`x' shows you the requirements your name must meet."
			"`1  '`=Clist`x' will list the name profiles available."
			"`1"
			"`1The list command displays the title of a name profile and its number, "
			"to see names generated with that particular profile simply type the "
			"corresponding number and press enter.  Because the system generates "
			"random names from a profile, you can use a profile numerous times "
			"with different names of similar style each time.");
			
		nannysup_name_select_prompt(d);
		return;
	}

	if(IS_NULLSTR(argument) || !str_prefix(argument, "requirements")){
		nannysup_begin_name_select(d);
		return;
	}
	if(d && d->ident_confirmed){
		name_confirmed=1;
	}

	// list profiles
	if(!str_cmp("list", argument)){
		int count=0;
		ch->printf("`=j%s`x", creation_titlebar("NAME GENERATOR PROFILES"));
		for(nl=name_profiles_list; nl; nl=nl->next){	
			ch->printlnf("%2d> %s", ++count, nl->title);
		}
		ch->wrapln("`SNOTE: randomly generated names in some rare "
			"situations will not meet the naming requirements.`x");
		nannysup_name_select_prompt(d);
		return;
	}

	// use a profile
	if(is_number(argument)){
		int count=0;
		int profile_num=atoi(argument);
		if(profile_num<1 || profile_num>profile_count){
			ch->printlnf("Invalid profile number %d.", profile_num);
			return;
		}
		for(nl=name_profiles_list; nl; nl=nl->next){	
			if(++count>=profile_num){
				break;
			};
		}
		int i;

		ch->printf("`=j%s`x", creation_titlebar("NAME GENERATOR - %s",uppercase(nl->title)));
		for(i=0; i<60; i++){
			ch->printf("  %-18s", capitalize(genname(nl)));
			if(i%4==3){
				ch->println("");
			}
		}
		ch->println("");
		ch->wrapln("`=cNOTE: randomly generated names in some rare situations "
			"will not meet the naming requirements.`x");

		nannysup_name_select_prompt(d);
		return;
	}

	// *** A player has suggested a name - check it out

	// check for invalid characters in name
	if(strstr(".", argument) || strstr("\\", argument) 
		|| strstr(" ", argument) || strstr("/", argument)){
		ch->wraplnf("Invalid letters name '%s' try another name (no spaces, fullstops "
			"or slashes in your please, try another single word name.", argument);
		nannysup_name_select_prompt(d);
		return;
	}
    argument=capitalize(argument);

	ch->wraplnf("Checking if the name '%s' is not currently used by another player "
		"or monster, please wait...", argument);

	// check for existing player with same name
	PFILE_TYPE pt=find_pfiletype(argument);
	ch->pcdata->pfiletype=pt;
	if(pt!=PFILE_NONE){
		ch->printlnf("The name '%s' is already used by another player, try another name.", argument);
		nannysup_name_select_prompt(d);
		return;
	}	

	// check if we can reject the name for other reasons
    if(!check_parse_name( argument ) )
    {
        ch->printlnf("Sorry the name '%s' is unable to be used, try another.", argument);
		nannysup_name_select_prompt(d);
        return;
    }

	{ // don't allow duplicate names in creation
		connection_data *dc;
		for( dc = connection_list; dc; dc = dc->next )
		{
			if(d!=dc && dc->character && !str_cmp(dc->character->name, argument) )
			{
				logf("Socket %d (%s) had name %s rejected because another is creating with it", 
					d->connected_socket, d->remote_hostname, CH(d)->name);
				ch->printlnf("Sorry someone else is currently creating using the name '%s', try another.", argument);
				nannysup_name_select_prompt(d);
				return;
			}
		}
	}

	ch->printf("`=j%s`x", creation_titlebar("CONFIRM INTENT TO CREATE CHARACTER %s", uppercase(argument)));
	replace_string(ch->name, argument);
	ch->wraplnf("The name '%s' appears to be available for you to use (assuming it mets "
		"the naming requirements listed earlier)."
		"`1Do you want to start creating a new character called '%s'?", argument, argument);
	d->connected_state=CON_CONFIRM_NEW_NAME;
}
/**************************************************************************/
void nanny_confirm_new_name(connection_data *d, const char *argument)
{
	char_data *ch=d->character;
	switch( *argument )
	{
	case 'y': case 'Y':
		if(count_creation_connections_per_hour(d)>2){
			ch->wrapln("WARNING: We limit how many times you can create a character per hour!!!"
				"`1You can only create a few more times this hour!");
		}

		ch->printf("`=j%s`x", creation_titlebar("PICK A PASSWORD FOR %s", uppercase(ch->name)));
		ch->printlnf("Welcome %s!  It is time for you to now pick a password.", ch->name);
		ch->printf("`WPlease enter a new password for %s:`x %s",
				ch->name, IS_IRCCON(d)?"\r\n":echo_off_str );
			d->connected_state = CON_GET_NEW_PASSWORD;
		break;

	case 'n': case 'N':
        logf("Socket %d (%s) decided not to make a new player.", 
			d->connected_socket, d->remote_hostname);

	    write_to_buffer( d, "Ok, lets start from the very start again...\r\n", 0 );
		write_to_buffer( d, LOGIN_PROMPT, 0);
		if(IS_IRCCON(d)){
			write_to_buffer( d, "\r\n", 0 );
		}

		free_char(d->character);
		d->character = NULL;
		d->connected_state = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No ", 0 );
		if(IS_IRCCON(d)){
 			write_to_buffer( d, "\r\n", 0 );
		}
	    break;
	}
}

/**************************************************************************/
void nanny_confirm_creating_new(connection_data *d, const char *argument)
{
	char_data *ch=d->character;
	switch( *argument )
	{
	case 'y': case 'Y':
//		d->newbie_creating= true;
		// check for newbie bans
		if(check_connection_ban(d))
			break;
		if(check_connection(d))
			break;

		// mark them as using the new colour code
		if(ch->pcdata){
			ch->pcdata->colour_code=COLOURCODE;
		}

		// process a connection password - if there is one
		if(!IS_NULLSTR(game_settings->password_player_connect) 
			&& str_cmp(game_settings->password_player_connect, "-")
			&& !IS_IMMORTAL(ch) && !ch->host_validated && !d->ident_confirmed){
			ch->printf("`=j%s`x", creation_titlebar("CONNECT PASSWORD REQUIRED"));
			ch->wrapln("There is currently a player connect password set which you must "
				"entered to play. Before you can create a new character you need to enter "
				"the connect password.`1Please enter the connection password:");
			d->connected_state=CON_GET_CONNECT_PASS2CREATE;
			return;
		}else if(!IS_NULLSTR(game_settings->password_creation) 
			&& str_cmp(game_settings->password_creation, "-")
			&& !ch->host_validated && !d->ident_confirmed){
			ch->printf("`=j%s`x", creation_titlebar("CREATION PASSWORD REQUIRED"));
			ch->wrapln("There is currently a creation password set which you must "
				"enter in now before creating a new character`1Please enter the creation password:");
			d->connected_state=CON_GET_CREATION_PASSWORD;
		}else{
			nannysup_begin_name_select(d);
		}
		break;

	case 'n': case 'N':
		d->newbie_creating= false;
        logf("Socket %d (%s) decided not to make a new player.", 
			d->connected_socket, d->remote_hostname);
        
	    write_to_buffer( d, "Ok, lets try again...\r\n", 0 );
		write_to_buffer( d, LOGIN_PROMPT, 0);
		if(IS_IRCCON(d)){
			write_to_buffer( d, "\r\n", 0 );
		}

		free_char( d->character );
		d->character = NULL;
		d->connected_state = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No ", 0 );
		if(IS_IRCCON(d)){
 			write_to_buffer( d, "\r\n", 0 );
		}
	    break;
	}
}
/**************************************************************************/
void nanny_get_connect_password_before_creating(connection_data *d, const char *argument)
{
	char_data *ch=d->character;

	assertp(game_settings->password_player_connect);

	// correct creation password
	if(!str_cmp(game_settings->password_player_connect, argument)){
		if(!IS_NULLSTR(game_settings->password_creation) 
			&& str_cmp(game_settings->password_creation, "-")
			&& !ch->host_validated && !d->ident_confirmed){
			ch->printf("`=j%s`x", creation_titlebar("CREATION PASSWORD REQUIRED"));
			ch->wrapln("There is also currently a creation password set which you must "
				"enter in now before creating a new character`1Please enter the creation password:");
			d->connected_state=CON_GET_CREATION_PASSWORD;
		}else{
			nannysup_begin_name_select(d);
		}
		return;
	}

	// wrong creation password
	ch->wraplnf("Sorry, '%s' is not the connect password... you can't create a new "
		"character at this point in time.", argument);
	write_to_buffer( d, LOGIN_PROMPT, 0);
	if(IS_IRCCON(d)){
		write_to_buffer( d, "\r\n", 0 );
	}

	free_char( ch );
	d->character = NULL;
	d->connected_state = CON_GET_NAME;

}
/**************************************************************************/
void nanny_get_creation_password(connection_data *d, const char *argument)
{
	char_data *ch=d->character;

	assertp(game_settings->password_creation);

	// correct creation password
	if(!str_cmp(game_settings->password_creation, argument)){
		ch->println("great, now lets get on with creating a character.");
		nannysup_begin_name_select(d);
		return;
	}

	// wrong creation password
	ch->wraplnf("Sorry, '%s' is not the creation password... you can't create a new "
		"character at this point in time.", argument);
	write_to_buffer( d, LOGIN_PROMPT, 0);
	if(IS_IRCCON(d)){
		write_to_buffer( d, "\r\n", 0 );
	}

	free_char( ch );
	d->character = NULL;
	d->connected_state = CON_GET_NAME;

}
/**************************************************************************/
void nanny_get_connect_password(connection_data *d, const char *argument)
{
	char_data *ch=d->character;

	assertp(game_settings->password_player_connect);

	// correct connect password
	if(!str_cmp(game_settings->password_player_connect, argument)){
		ch->println("great, now lets get on with creating a character.");
		nannysup_process_correct_connect_password(d);
		return;
	}

	// wrong creation password
	ch->wraplnf("Sorry, '%s' is not the connect password... you can't "
		"play this character at this point in time.", argument);
	write_to_buffer( d, LOGIN_PROMPT, 0);
	if(IS_IRCCON(d)){
		write_to_buffer( d, "\r\n", 0 );
	}

	free_char( ch );
	d->character = NULL;
	d->connected_state = CON_GET_NAME;

}
/**************************************************************************/
void nanny_get_new_password(connection_data *d, const char *argument)
{
	char_data *ch=d->character;
	write_to_buffer( d, "\r\n", 2 );

	if(str_len(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\r\nPassword: ",
		0 );
	    return;
	}

	char *p;
    char *pwdnew = dot_crypt( argument, CH(d)->name );
	for( p = pwdnew; *p != '\0'; p++ )
	{
	    if(*p == '~' )
	    {
			write_to_buffer( d,"New password not acceptable, try again.\r\nPassword: ",0 );
			if(IS_IRCCON(d)){
				write_to_buffer( d, "\r\n", 0 );
			}
			return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	ch->printf("`=j%s`x", creation_titlebar("RETYPE PASSWORD"));
    write_to_buffer( d, "Please retype password: ", 0 );
	if(IS_IRCCON(d)){
		write_to_buffer( d, "\r\n", 0 );
	}

	d->connected_state = CON_CONFIRM_NEW_PASSWORD;
}
/**************************************************************************/
void nanny_confirm_new_password(connection_data *d, const char *argument)
{
	char_data *ch=d->character;

#if defined(unix)
	write_to_buffer( d, "\r\n", 2 );
#endif

	if(strcmp( dot_crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\r\nEnter a new password: ",0 );
		if(IS_IRCCON(d)){
			write_to_buffer( d, "\r\n", 0 );
		}

	    d->connected_state = CON_GET_NEW_PASSWORD;
	    return;
	}

	ch->printf("`=j%s`x", creation_titlebar("DO YOU WANT COLOUR"));
	if(!IS_IRCCON(d)){
		write_to_buffer( d, echo_on_str, 0 );
		write_to_buffer( d, "Do you want to play with ansi colour?\r\n", 0 );
		write_to_buffer( d, "(you can toggle at anytime once within the game by typing colour)\r\n", 0 );
		d->connected_state = CON_GET_COLOUR;
	}else{ // automatically turn on colour for IRC connections - default them to irc white
		d->colour_mode=CT_IRCWHITE;
		ch->pcdata->colourmode=CT_IRCWHITE;
		nannysup_email_check(d,argument);
	}
}
/**************************************************************************/
void nanny_get_colour(connection_data *d, const char *argument)
{
	char_data *ch=d->character;
	switch( *argument )
	{
	default:
		write_to_buffer(d,"Please type Yes or No ",0);
		if(IS_IRCCON(d)){
			write_to_buffer( d, "\r\n", 0 );
		}
		return;
	    break;

	case 'y' : case 'Y':
		ch->pcdata->colourmode=CT_ANSI;
		ch->println("`?C`?o`?l`?o`?u`?r`? `?i`?s `?n`?o`?w `?O`?N`?!`x");
	    break;

	case 'n' : case 'N':
		d->colour_mode=CT_NOCOLOUR;
		ch->pcdata->colourmode=CT_NOCOLOUR;
	    write_to_buffer(d,"You now have colour disabled.\r\n",0);
		break;
	}
	nannysup_email_check(d,argument);
}
/**************************************************************************/
void nanny_get_new_race(connection_data *d, char *argument)
{
	char arg[MIL];
	char_data *ch=d->character;

	one_argument(argument,arg);

	if(!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
		if(argument[0] == '\0'){
			do_help(ch,"race help");
		}else{
			do_help(ch,argument);
		}
        write_to_buffer(d, "What race do you want to play?`1",0);
	    return;
	}

	if(!GAMESETTING5(GAMESET5_RACEINFO_DISABLED_IN_CREATION) && !strcmp(arg,"raceinfo"))
	{
		CH(d)->wrapln("`xSee \"help `=_attributes`x\" and "
			"\"help `=_'creation points'`x\" for an explaination of "
			"what the following numbers relate to.");
		do_raceinfo(ch, "");
		ch->print("`WEnter the name of the race would you like to play: `x");
		if(IS_IRCCON(d)){
			ch->print_blank_lines(1);
		}
	    return;
	}

	int	race = race_lookup(argument);

	if(race == -1 || !race_table[race]->creation_selectable()
		||  (race_table[race]->remort_number > d->creation_remort_number))
	{
		if(IS_NULLSTR(argument)){
			CH(d)->printf("You have to type something!`1");
		}else{
			CH(d)->printf("'%s' is not a valid race.`1", argument);
		}
		if(!codehelp(CH(d), "racial_option", false)){
			CH(d)->printf("Please select your race from one of the following:`1 ");
			int count=0;
			for( race = 0; race_table[race]; race++ )
			{
				// creation selectable pc races only
				if(!race_table[race]->creation_selectable()
					|| (race_table[race]->remort_number > d->creation_remort_number)){
					continue;
				}
				CH(d)->printf(" `S[`Y%12.12s`S]", race_table[race]->name);
				if(++count%5==0){
					CH(d)->printf("`x\r\n ");
				}
			}
			CH(d)->printf("`1`xType in the name of the race you wish to play now:`1");
		}
	    return;
	}

	CH(d)->printf("Race '%s' selected.`1`1", race_table[race]->name);
	ch->race = race;

	ch->affected_by = ch->affected_by|race_table[race]->aff;
	REMOVE_BIT(ch->affected_by, AFF_FLYING); // new players don't start off flying
	affect_fly_update(ch); // set/remove DYN_MAGICAL_FLYING as required
	ch->affected_by2= ch->affected_by2|race_table[race]->aff2;
	ch->affected_by3= ch->affected_by3|race_table[race]->aff3;
	ch->imm_flags	= ch->imm_flags|race_table[race]->imm;
	ch->res_flags	= ch->res_flags|race_table[race]->res;
	ch->vuln_flags	= ch->vuln_flags|race_table[race]->vuln;
	ch->form	= race_table[race]->form;
	ch->parts	= race_table[race]->parts;

	// add racial skills 
	for(int i = 0; i < MAX_RACIAL_SKILLS; i++){
		if(race_table[race]->skills[i]==-1){
			break;
		}
		group_add(ch,skill_table[race_table[race]->skills[i]].name,false, 1); // all racial skills are 1%
	}
	// add cost 
	ch->pcdata->points = race_table[race]->points;
	ch->size = race_table[race]->size;

	CH(d)->printf("`=j%s`x", creation_titlebar("GENDER SELECTION"));
    write_to_buffer( d, "What gender is your character (M/F)? ", 0 );
	if(IS_IRCCON(d)){
		write_to_buffer( d, "\r\n", 0 );
	}
	d->connected_state = CON_GET_NEW_SEX;
}
/**************************************************************************/
void nanny_get_new_sex(connection_data *d, const char *argument)
{
	char_data *ch=d->character;

	switch( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\r\nWhat IS your sex? ", 0 );
		if(IS_IRCCON(d)){
			write_to_buffer( d, "\r\n", 0 );
		}
	    return;
	}

	if(GAMESETTING5(GAMESET5_CREATION_ASK_IF_WANT_AUTOMAP_ON)){
		CH(d)->printf("`=j%s`x", creation_titlebar("AUTOMAP OPTION"));
		ch->lines += 15;
		if(!codehelp(CH(d), "automap_option", CODEHELP_ALL_BUT_PLAYERS)){
			ch->printf("Do you want the automap option enabled?\r\n");
		}
		ch->lines -= 15;
		d->connected_state=CON_GET_AUTOMAP;
	}else{
		// automap defaults to on automatically, 
		// unless game is configured to ask in the gamesettings
		SET_BIT(d->character->act,PLR_AUTOMAP);
		// start class selection
		connected_to_CON_REROLL_STATS(d);
	}
}
/**************************************************************************/
void nanny_get_new_class(connection_data *c, char *argument)
{
	char arg[MIL];
    char tempbuf[MIL];
	char_data *ch=c->character;
	
	one_argument(argument,arg);
	if(!strcmp(arg,"help")){
		argument = one_argument(argument,arg);
		if(IS_NULLSTR(argument)){
			do_help(ch,"classes");
		}else{
			do_help(ch,argument);
		}
		ch->println("What class do you wish to be? ");
		return;		
	}

	if(!GAMESETTING5(GAMESET5_CLASSINFO_DISABLED_IN_CREATION) && !strcmp(arg,"classinfo"))
	{
		argument = one_argument(argument,arg);
		ch->print_blank_lines(1);
		ch->wrapln("`xSee \"help `=_basexp`x\", \"help `=_prime`x\" and "
			"\"help `=_attributes`x\" for an explaination of "
			"what the following values relate to.");
		if(!GAMESETTING5(GAMESET5_RACEINFO_DISABLED_IN_CREATION)){
			ch->println("`=Craceinfo`x is also available at this stage in creation for reference purposes.");
		}
		if(IS_NULLSTR(argument)){
			do_classinfo(ch, race_table[ch->race]->name);
			if(GAMESETTING5(GAMESET5_RACEINFO_DISABLED_IN_CREATION)){
				ch->wrapln("You can also look at the base xp amounts for other races "
					"(for comparision purposes), by typing 'classinfo <racename>'.");
			}else{
				ch->wrapln("You can also look at the base xp amounts for other races "
					"(for comparision purposes), by typing 'classinfo <racename>'... "
					"use 'raceinfo' to get a list of races.");
			}
		}else{
			do_classinfo(ch, argument);
		}
		ch->print("`WEnter the name of the class would you like to play: `x");
		ch->print_blank_lines(IS_IRC(ch)?1:0);
		return;
	}
	if(!GAMESETTING5(GAMESET5_RACEINFO_DISABLED_IN_CREATION) && !strcmp(arg,"raceinfo"))
	{
		ch->wrapln("`xSee \"help `=_attributes`x\" and "
			"\"help `=_'creation points'`x\" for an explaination of "
			"what the following values relate to.");
		do_raceinfo(ch, "");
		ch->print("`WEnter the name of the CLASS would you like to play: `x");
		ch->print_blank_lines(IS_IRC(ch)?1:0);
	    return;
	}
	
	int iClass= class_lookup(argument);	
	if(iClass == -1 
		|| !class_table[iClass].creation_selectable
		|| (class_table[iClass].remort_number > c->creation_remort_number))
	{
		ch->printlnf("'%s' is not a recognised class.", argument);
		ch->print("What IS your class? ");
		ch->print_blank_lines(IS_IRC(ch)?1:0);
		return;
	}
	if(race_table[ch->race]->class_exp[iClass]<1000){
		ch->printlnf("The %s class is not available for your race sorry.",  
			class_table[iClass].name);
		ch->print("What IS your class? ");
		ch->print_blank_lines(IS_IRC(ch)?1:0);
		return;
	}

	// perform remort_to_classes restrictions
	if(ch->remort>0 
		&& !IS_NULLSTR(class_table[ch->clss].remort_to_classes)
		&& !is_exact_name(class_table[iClass].name, class_table[ch->clss].remort_to_classes)){
		ch->printlnf("The %s class is not available to remort to from your previous class (%s) sorry.",  
			class_table[iClass].name, class_table[ch->clss].name);
		ch->print("What IS your class? ");
		ch->print_blank_lines(IS_IRC(ch)?1:0);
		return;
		
	}

	ch->clss = iClass;
	
	ch->pcdata->perm_hit=race_table[ch->race]->start_hp;
	ch->max_hit=ch->pcdata->perm_hit;
	ch->hit=ch->max_hit;
	
	int langsn=race_table[ch->race]->language->gsn;
	if(langsn>0){
		ch->pcdata->learned[langsn]=100;         
	}
	ch->language=race_table[ch->race]->language;
	
	// set players default short description
	if(ch->short_descr[0]=='\0')
	{
		sprintf(tempbuf,"a %s %s",
			(ch->sex==0  ? "sexless" : ch->sex==1 ? "male" : "female"),
			race_table[ch->race]->name);
		ch->short_descr= str_dup(tempbuf);
	}

	// turn on players auto defaults 
	SET_BIT(ch->act,PLR_AUTOREFORMAT);
	SET_BIT(ch->act,PLR_AUTOLOOT);
	SET_BIT(ch->act,PLR_AUTOASSIST); 
	SET_BIT(ch->act,PLR_AUTOGOLD); 
	SET_BIT(ch->act,PLR_AUTOEXIT);
	SET_BIT(ch->act,PLR_AUTOSPLIT);
	SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
	SET_CONFIG(ch, CONFIG_AUTOLANDONREST);
	if(!GAMESETTING5(GAMESET5_AUTODAMAGE_DEFAULTS_OFF)){
		SET_CONFIG2(ch,CONFIG2_AUTODAMAGE);
	}
	
	// log them if we autolog all new players
	if(GAMESETTING4(GAMESET4_AUTOLOG_ALL_NEW_PLAYERS)){
		SET_BIT(ch->act,PLR_LOG);
        append_playerlog( ch, "Player log turned ON by GAMESET4_AUTOLOG_ALL_NEW_PLAYERS.");
	}

	ch->println("Applying primary stats for your class to your character..." );
	nannysup_setprime_stats(ch);
	
	if(IS_IRC(ch)){
		sprintf(log_buf,"%s@%s new IRC player!!! (socket = %d)", 
			ch->name, c->remote_hostname, c->connected_socket);
	}else{
	sprintf(log_buf,"%s@%s new player!!! (socket = %d)", 
		ch->name, c->remote_hostname, c->connected_socket);
	}
	log_string(log_buf);
	wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	wiznet("Newbie alert!  $N sighted, they are currently part way through creation",ch,NULL,WIZ_NEWBIE,0,0);
	
	// alert newbie support of an incoming newbie
	nsupport_newbie_alert( ch, false );
	
	connected_to_CON_GET_ALLIANCE(c);
}
/**************************************************************************/
void nanny_reroll_stats(connection_data *d, char *argument)
{
	char_data *ch=d->character;
	char arg[MIL];
	one_argument(argument,arg);

	switch( *arg)
	{
	case 'n' : case 'N':
		// don't let people reroll more than 100 times
		if(ch->pcdata->reroll_counter>100)
		{
			d->write("\r\n"
				"\r\nFind something better to do with your time!\r\n\r\n",0);
				logf("%s %s (%d) dropped due to rerolling over 100 times.", 			
					CH(d)->name, d->remote_hostname, d->connected_socket );
			connection_close(d);
			return;
		}

		// reroll and show the stats
	    write_to_buffer(d,"Rerolling please wait.\r\n",0);
		roll_stats(d);
		if(!ch->pcdata->reroll_counter)
		{
			write_to_buffer(d,"Note: Attributes are rolled with a bias in favour doing less rerolls.\r\n",0);
			write_to_buffer(d,"The attributes rolling system has been changed since dawn1.1 and will NOT give\r\n"
							  "high attributes like it used to, also note that modifiers now start at 60 not 70!\r\n",0);
		}
		ch->pcdata->reroll_counter++;	
	    break;

	// player keeps stats
	case 'y' : case 'Y':
		// mydrian fields experimental creation system
		if( d->newbie_creating == CREATING_MF_EXPERIMENTAL_SYSTEM )
		{
			char tempbuf[MIL];
			ch->clss = class_lookup( "citizen" );
	
			ch->pcdata->perm_hit=race_table[ch->race]->start_hp;
			ch->max_hit=ch->pcdata->perm_hit;
			ch->hit=ch->max_hit;
	
			int langsn=race_table[ch->race]->language->gsn;
			if(langsn>0){
				ch->pcdata->learned[langsn]=100;         
			}
			ch->language=race_table[ch->race]->language;
	
			// set players default short description
			if(ch->short_descr[0]=='\0')
			{
				sprintf(tempbuf,"a %s %s",
					(ch->sex==0  ? "sexless" : ch->sex==1 ? "male" : "female"),
					race_table[ch->race]->name);
				ch->short_descr= str_dup(tempbuf);
			}

			// turn on players auto defaults 
			SET_BIT(ch->act,PLR_AUTOREFORMAT);
			SET_BIT(ch->act,PLR_AUTOLOOT);
			SET_BIT(ch->act,PLR_AUTOASSIST); 
			SET_BIT(ch->act,PLR_AUTOGOLD); 
			SET_BIT(ch->act,PLR_AUTOEXIT);
			SET_BIT(ch->act,PLR_AUTOSPLIT);
			SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
			SET_CONFIG(ch, CONFIG_AUTOLANDONREST);
			if(!GAMESETTING5(GAMESET5_AUTODAMAGE_DEFAULTS_OFF)){
				SET_CONFIG2(ch,CONFIG2_AUTODAMAGE);
			}
	
			// log them if we autolog all new players
			if(GAMESETTING4(GAMESET4_AUTOLOG_ALL_NEW_PLAYERS)){
				SET_BIT(ch->act,PLR_LOG);
			    append_playerlog( ch, "Player log turned ON by GAMESET4_AUTOLOG_ALL_NEW_PLAYERS.");
			}

//			ch->println("Applying primary stats for your class to your character..." );
//			nannysup_setprime_stats(ch);
	
			if(IS_IRC(ch)){
				sprintf(log_buf,"%s@%s new IRC player!!! (socket = %d)", 
					ch->name, d->remote_hostname, d->connected_socket);
			}else{
			sprintf(log_buf,"%s@%s new player!!! (socket = %d)", 
				ch->name, d->remote_hostname, d->connected_socket);
			}
			log_string(log_buf);
			wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
			wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
	
			// alert newbie support of an incoming newbie
			nsupport_newbie_alert( ch, false );
		
			connected_to_CON_GET_ALLIANCE(d);
			break;
		}
		
		connected_to_CON_GET_NEW_CLASS(d);
		break;

	default:
		{
			
			if(!strcmp(arg,"help"))
			{
				argument = one_argument(argument,arg);
				if(IS_NULLSTR(arg)){
					do_help(ch,"attributes");
				}else{
					do_help(ch,argument);
				}
				ch->println("Are you happy with the attributes above the help entry? (Y, N): ");
				return;
			}


			write_to_buffer(d,"Please type Y, N or HELP? ",0);
			if(IS_IRCCON(d)){
				write_to_buffer( d, "\r\n", 0 );
			}
		}
	    break;
	}
}
/**************************************************************************/
void nanny_get_alliance(connection_data *d, const char *argument)
{
	char_data *ch=d->character;
	if(is_number(argument))
	{
		if(GAMESETTING(GAMESET_MAX_ALIGN_RANGE22)){
			if(atoi(argument)<-2 || atoi(argument)>2)
			{
				write_to_buffer(d,"Value between -2 and 2 please:",0);
				return;
			}
		}else{
			if(atoi(argument)<-3 || atoi(argument)>3)
			{
				write_to_buffer(d,"Value between -3 and 3 please:",0);
				return;
			}
		}
		
		ch->alliance=atoi(argument);
		{
	CH(d)->printf("`1`1`=j%s`x", creation_titlebar("TENDENCY"));
			write_to_buffer(d,
	"The second half of you alignment is your TENDENCY towards law and chaos,\r\n",0);

			if(GAMESETTING(GAMESET_MAX_ALIGN_RANGE22)){
				write_to_buffer(d,
	"this value can be from -2 to 2(including 0 being neutral) 2 being lawful,\r\n"
	"lawful and -2 being chaotic.  \r\nWhat is your choice? ",0);
			}else{
				write_to_buffer(d,
	"this value can be from -3 to 3(including 0 being neutral) 3 being extremely,\r\n"
	"lawful and -3 being extremely chaotic.  \r\nWhat is your choice? ",0);
			}

			if(IS_IRCCON(d)){
				write_to_buffer( d, "\r\n", 0 );
			}
			d->connected_state = CON_GET_TENDENCY;
		}
	}else{
		if(GAMESETTING(GAMESET_MAX_ALIGN_RANGE22)){
			write_to_buffer(d,"That is not a number, try again(-2 through 2):",0);
		}else{
			write_to_buffer(d,"That is not a number, try again(-3 through 3):",0);
		}
		return;
	}
}
/**************************************************************************/
void nanny_get_tendency(connection_data *d, const char *argument)
{
	char_data *ch=d->character;

	if(is_number(argument))
	{
		if(GAMESETTING(GAMESET_MAX_ALIGN_RANGE22)){
			if(atoi(argument)<-2 || atoi(argument)>2)
			{
				write_to_buffer(d,"Value between -2 and 2 please:",0);
				return;
			}
		}else{
			if(atoi(argument)<-3 || atoi(argument)>3)
			{
				write_to_buffer(d,"Value between -3 and 3 please:",0);
				return;
			}
		}

		{
			ch->tendency=atoi(argument);

			if(d->ident_confirmed){
				CH(d)->host_validated=1;
			}
			
			if(ch->version<6)
			{
				ch->version=6;
				write_to_buffer(d,"\r\n",0);
				
				// loop thru all the groups giving new character all the ones 
				// his/her class can get for free and the free for all's
				for ( int lgn = 0; !IS_NULLSTR(skillgroup_table[lgn].name); lgn++ )
				{
					if(  skillgroup_table[lgn].rating[ch->clss]==0
						|| IS_SET(skillgroup_table[lgn].flags, SKILLGROUP_FREE_FOR_ALL))
					{
						gn_add(ch, lgn);
					}
				}

				// give them some defaults
				ch->pcdata->learned[gsn_recall] = 50;

				if(GAMESETTING5(GAMESET5_CREATION_DISABLE_CUSTOMIZATION)
					|| HAS_CLASSFLAG(ch, CLASSFLAG_NO_CUSTOMIZATION)
					|| IS_SET(race_table[ch->race]->flags, RACEFLAG_NO_CUSTOMIZATION))
				{
					nanny_new_player_not_customizing(d);
					return;
				}

			
				CH(d)->printf("`1`=j%s`x", creation_titlebar("CUSTOMIZATION"));
				write_to_buffer(d,"Do you wish to customize this character?\r\n",0);
				write_to_buffer(d,
					"Customization takes time, but allows a wider range of skills and abilities.\r\n",0);
				write_to_buffer(d,"(we recommend choosing no, unless you have played here before as\r\n",0);
				write_to_buffer(d,"you can always gain skills at a later stage.)\r\n",0);
				write_to_buffer(d,"Customize (Y/N)? ",0);
				d->connected_state = CON_DEFAULT_CHOICE;
				return;						
			}
			else
			{
				if(IS_IMMORTAL(ch) )
				{
					do_help( ch, "imotd" );
					ch->hit_return_to_continue();
					d->connected_state = CON_READ_IMOTD;
				}
				else
				{
					if(IS_IRCCON(d)){
						do_help( ch, "irc-motd" );
					}else{
						do_help( ch, "motd" );
					}
					ch->hit_return_to_continue();
					d->connected_state = CON_READ_MOTD;
				}
			}
			
		}
	}
	else
	{
		if(GAMESETTING(GAMESET_MAX_ALIGN_RANGE22)){
			write_to_buffer(d,"That is not a number, try again(-2 through 2):",0);
		}else{
			write_to_buffer(d,"That is not a number, try again(-3 through 3):",0);
		}
		return;
	}
}
/**************************************************************************/
void nanny_default_choice(connection_data *d, const char *argument)
{
	char_data *ch=d->character;
	write_to_buffer(d,"\r\n",2);
	switch( argument[0] )
	{
	case 'y': case 'Y': 
		{
			int langsn=race_table[ch->race]->language->gsn;
			if(langsn>0){
				ch->pcdata->learned[langsn]=100;         
			}
			ch->language=race_table[ch->race]->language;
			ch->gen_data = new_gen_data();
			ch->gen_data->points_chosen = ch->pcdata->points;
			do_help(ch,"group-header");
			list_group_costs(ch);
			write_to_buffer(d,"You already have the following skills:\r\n",0);
			do_skills(ch,"");
			do_help(ch,"menu-choice");
			d->connected_state = CON_GEN_GROUPS;
		}
		break;
	case 'n': case 'N':
		{
			nanny_new_player_not_customizing(d); // take some defaults, then d->connected_state -> CON_READ_MOTD 
		}
		break;
	default:
		write_to_buffer( d, "Please answer (Y/N)? ", 0 );
		return;
	}
}
/**************************************************************************/
void nanny_gen_groups(connection_data *d, char *argument)
{
	char_data *ch=d->character;
    ch->println("");
    if(!str_cmp(argument,"done") || !str_cmp(argument,"doneconfirm"))
    {
		if(ch->pcdata->points == race_table[ch->race]->points
			&& !str_cmp(argument, "doneconfirm"))
		{
			group_add(ch,class_table[ch->clss].default_group,true, 1);
			ch->printf("Creation points: %d\r\n",ch->pcdata->points);
			ch->printf("Experience per level: %d\r\n",
			exp_per_level(ch,ch->pcdata->points));
     
			if(ch->pcdata->points < 40)
				ch->train = (40 - ch->pcdata->points + 1) / 2;

		}

		else if(ch->pcdata->points == race_table[ch->race]->points)
 	    {
 	        ch->wrapln("You didn't pick anything.  To cancel customization, type `#`Ydoneconfirm`^ with no skills added.\r\n");
 			return;
 	    }

		else if(str_cmp(argument,"doneconfirm")){
  			if(ch->pcdata->points < 40)
			{
				ch->wrapln("You haven't taken at least 40 points of skills and groups... "
					"You can add more skills without affecting the amount of experience required to level.  "
					"If you wish to continue without adding anymore skills/groups, type `=Cdoneconfirm`x");					
				return;
			}
		}

		ch->printf("Creation points: %d\r\n",ch->pcdata->points);

		ch->printf("Experience per level: %d\r\n",
			exp_per_level(ch,ch->pcdata->points));
     
        if(ch->pcdata->points < 40)
            ch->train = (40 - ch->pcdata->points + 1) / 2;

        free_gen_data(ch->gen_data);
        ch->gen_data = NULL;
        write_to_buffer( d, "\r\n", 2 );

		if( ch->level > 1 )
		{
			ch->desc->connected_state = CON_PLAYING;
			return;
		}

        if(IS_IRCCON(d)){
			do_help( ch, "irc-motd" );
		}else{
			do_help( ch, "motd" );
		}
		ch->hit_return_to_continue();
        d->connected_state = CON_READ_MOTD;
        return;
    }

    if(!parse_gen_groups(ch,argument)){
        ch->println("That isn't a valid selection, (type help if you are stuck):\r\n");
    }
    ch->println("\r\n");
    do_help(ch,"menu-choice");
}

/**************************************************************************/
void do_mobloglist( char_data *ch, char *argument);
/**************************************************************************/
void nanny_read_imotd(connection_data *d, const char *)
{
	char_data *ch=d->character;
    write_to_buffer(d,"\r\n",2);
	if(IS_IRCCON(d)){
		do_help( ch, "irc-motd" );
	}else{
		do_help( ch, "motd" );
	}
	ch->hit_return_to_continue();

	if(IS_ADMIN(ch)){
		// check the resolver version
		if(resolver_running && resolver_version<1400){
			ch->titlebar("UPDATE THE DNS RESOLVER");
			ch->wrapln("The dns resolver program (resolver) is an old version and "
				"needs to be updated.  On a linux/bsd/unix platform this can be done by "
				"deleting the existing resolver program and recompile the code.  "
				"On the Win32 platform you can "
				"download the latest version of the resolver.exe file from"
				" `1http://www.dawnoftime.org.`1This message is only displayed to the admin.");
			ch->titlebar("");
		}

		// tell admin about the moblogs on 
		do_mobloglist(ch, "auto");

		// tell admin about missing CORPSEs
		if(!get_obj_index(OBJ_VNUM_CORPSE_NPC)){ 
			ch->titlebar("MISSING DAWN SYSTEM OBJECT - NPC CORPSE TEMPLATE");
			ch->printlnf("Missing object template vnum %d - for creating NPC Corpses!!!", OBJ_VNUM_CORPSE_NPC);
			ch->wrapln("Without this file, the mud will have problems when a mobile is killed!  "
				"It is strongly recommend that you include the latest dawn.are area file from "
				"the dawn codebase system files.");
			ch->titlebar("");            
		}
		if(!get_obj_index(OBJ_VNUM_CORPSE_PC)){ 
			ch->titlebar("MISSING DAWN SYSTEM OBJECT - PC CORPSE TEMPLATE");
			ch->printlnf("Missing object template vnum %d - for creating PC Corpses!!!", OBJ_VNUM_CORPSE_NPC);
			ch->wrapln("Without this file, the mud will have problems when a player is killed!  "
				"It is strongly recommend that you include the latest dawn.are area file from "
				"the dawn codebase system files.");
			ch->titlebar("");            
		}

		// tell admin about the missing Objectprogs mob
		if(!get_mob_index(MOB_VNUM_TO_RUN_OBJECT_MUDPROGS)){ 
			ch->titlebar("MISSING DAWN SYSTEM MOBILE - MOB TO RUN OBJECT MUDPROGS");
			ch->printlnf("Missing mobile template vnum %d - for running object progs!!!", MOB_VNUM_TO_RUN_OBJECT_MUDPROGS);
			ch->wrapln("Without this file, object progs won't work correctly!  "
				"It is strongly recommend that you include the latest dawn.are "
				"area file from the dawn codebase system files.");
			ch->titlebar("");            
		}

		
#ifdef DAWN_STATIC_BETA_RELEASE
		// tell them about the code expiry
		{
			time_t expiry_date=DAWN_STATIC_BETA_RELEASE_EXPIRY_DATE;
			
			if(current_time<expiry_date){
				ch->titlebar("DAWN BETA RELEASE");
				ch->wraplnf("This version of the dawn code is a beta release which "
					"will expire on %s.  After this date only immortals will be "
					"able to connect", ctime(&expiry_date));
			}else{
				ch->titlebar("DAWN BETA RELEASE EXPIRED");
				ch->wraplnf("This version of the dawn code is a beta release which "
					"expired on %s!  Only immortals are able to connect.  Look at "
					"downloading a newer release from www.dawnoftime.org.", 
					ctime(&expiry_date));
			}
		}
		ch->titlebar("");
#endif 
	}

    d->connected_state = CON_READ_MOTD;
}
/**************************************************************************/
void visual_debug_flush( connection_data *d);
/**************************************************************************/
// Deal with sockets that haven't logged in yet.
void nanny( connection_data *d, char *argument )
{    
//	logf("Nanny: d->connected_state %d, '%s' %d %d .", d->connected_state, 
//		argument, *argument, *(argument+1) ); // debug testing code

	if(d->connected_state!= CON_FTP_DATA && d->connected_state!= CON_DETECT_CLIENT_SETTINGS){
		while( is_space(*argument) ){
			argument++;
		}
		logf("Start nanny state %2d (sock=%d)", d->connected_state, d->connected_socket);
	}

	if(d->visual_debugging_enabled && d->visual_debug_flush_before_prompt){
		visual_debug_flush( d );
	}

    switch( d->connected_state )
    {
		default:
			bugf("Nanny: bad d->connected_state %d (sock=%d).", d->connected_state, d->connected_socket);
			connection_close( d );
			break;

		// a new connection begins in state CON_DETECT_CLIENT_SETTINGS
		case CON_DETECT_CLIENT_SETTINGS: nanny_detect_client_settings(d, argument); break;
		case CON_GET_NAME:				nanny_get_name(d, argument); break; 				
		case CON_CONFIRM_CREATING_NEW:	nanny_confirm_creating_new(d, argument); break;
		case CON_GET_CONNECT_PASS2CREATE: nanny_get_connect_password_before_creating(d, argument); break;
		case CON_GET_CREATION_PASSWORD:	nanny_get_creation_password(d, argument); break;
		case CON_GET_CONNECT_PASSWORD:	nanny_get_connect_password(d, argument); break;			
		case CON_NAME_SELECT:			nanny_name_select(d, argument); break; 
		case CON_CONFIRM_NEW_NAME:		nanny_confirm_new_name(d, argument); break; 
		case CON_RESOLVE_IP:			nanny_resolve_ip(d, argument); break;
		case CON_GET_EMAIL:				nanny_get_email(d, argument); break;
		case CON_RECHECK_EMAIL:			nanny_recheck_email(d, argument); break;		
		case CON_ENTER_UNLOCK_ID:		nanny_enter_unlock_id(d, argument); break;
		case CON_GET_AUTOMAP:			nanny_get_automap(d, argument); break;
		case CON_GET_OLD_PASSWORD:		nanny_get_old_password(d, argument); break;
		case CON_BREAK_CONNECT:			nanny_break_connect(d, argument); break;
		case CON_GET_NEW_PASSWORD:		nanny_get_new_password(d, argument); break;
		case CON_CONFIRM_NEW_PASSWORD:	nanny_confirm_new_password(d, argument); break;
		case CON_GET_COLOUR:			nanny_get_colour(d, argument); break;
		case CON_GET_NEW_RACE:			nanny_get_new_race(d, argument); break;
		case CON_GET_NEW_SEX:			nanny_get_new_sex(d, argument); break;
		case CON_REROLL_STATS:			nanny_reroll_stats(d, argument); break;
		case CON_GET_NEW_CLASS:			nanny_get_new_class(d, argument); break;
		case CON_GET_ALLIANCE:			nanny_get_alliance(d, argument); break;
		case CON_GET_TENDENCY:			nanny_get_tendency(d, argument); break;
		case CON_DEFAULT_CHOICE:		nanny_default_choice(d, argument); break;
		case CON_GEN_GROUPS:			nanny_gen_groups(d, argument); break;
		case CON_READ_IMOTD:			nanny_read_imotd(d, argument); break;
		case CON_READ_MOTD:				nanny_read_motd(d, argument ); break;
		case CON_FTP_AUTH:				handle_ftp_auth(d,argument); break;
		case CON_FTP_COMMAND:			handle_ftp_command(d,argument); break;
		case CON_FTP_DATA:				handle_ftp_data(d,argument); break;
    }
}

/**************************************************************************/
// Parse a name for acceptability.
bool check_parse_name( char *name )
{
	// Reserved words.
    if(is_name( name, "auto someone something the you new self system dawnftp mudftp" ))
		return false;

	// some potentially offensive/inappropriate names in hex :)
	if(is_name_infix( name, 
		"\x66\x75\x63\x6b \x70\x68\x75\x63\x6b \x66\x75\x6b \x70\x68\x75\x6b "
		"\x73\x68\x69\x74 \x64\x61\x6d\x6e \x63\x6f\x63\x6b \x63\x75\x6e\x74 "
		"\x62\x6c\x6f\x77\x6a\x6f\x62 \x61\x73\x73\x68\x6f\x6c\x65 "
		"\x70\x75\x73\x73\x79 \x6a\x65\x73\x75\x73 \x67\x6f\x64 "
		"\x61\x6e\x67\x65\x6c \x73\x61\x74\x61\x6e "
		"\x6c\x75\x63\x69\x66\x69\x65\x72 \x64\x65\x6d\x6f\x6e "
		"\x6f\x73\x61\x6d\x61 \x73\x61\x64\x61\x61\x6d"))
	{
		return false;
	}

	// just plain unoriginal/possibly intending to offend  
	if(is_name( name, 
		"merlin gandolf gandalf hero takhisis paladine majere morgion glarth "
		"trispringer chemosh zivilyn alexander belrak death angel bozo stupid "
		"beavis butthead mohammed mohammad buffy willow zander doom slayer rules rulez "
		"bauhaus lunitari solinari nuitari devil yugi anime penis assmunch "
		"kiri-jolith mishakal habbakuk branchala gilean sirrion reorx "
		"chislev shinare sargonnas zeboim hiddukel trinity morpheus "
		"blitz blytz budda buddah garlo blah blood buto cypher neo sliver "
		"alaric deity genocide freedalis gremlic hack hate king knight "
		"baron lord count prince princess darth vader lestat night zero "
		"victor viktor vashiva dashiva tinkle tanis sanctus rhuid rude"))
        return false;

    // reference names
	if(is_name( "none", name))	// none - used by olc
        return false;
    if(is_name( "self", name))	// self - used to reference self
        return false;
    if(is_name( "all",  name))	// everyone in the game
        return false;
    if(is_name( "system", name))
        return false;

	// council related names
	if(is_exact_name( "council", name ))
		return false;

	// next loop thru all the council names as defined in council_flags[]
	// admin can read all those notes
	{
		int index;

		for(index = 0; !IS_NULLSTR(council_flags[index].name); index++)
		{
			if(is_exact_name( council_flags[index].name, name )){
				return false;
			}
		}
	}

	// next few are for allowing notes to certain groups 
    if(is_name( "imm", name))				// immortals
        return false;

    if(is_name( "hero", name))			// heros
        return false;

    if(is_name( "heros", name))			// heros
        return false;

    if(is_name( "nsupport", name))		// newbie support
        return false;

    if(is_name( "head", name))			// head of departments
        return false;

    if(is_name( "noble", name))			// nobles 
        return false;
	
    if(is_name( "admin", name))			// administrators 
        return false;

	if(is_name( "imp", name))				// implementors
		return false;

	if(is_name( "rpsupport", name))		// role playing support
		return false;

    if(is_name( "olc", name))				// olc note group security >4
        return false;

    if(is_name( "build", name))			// olc note group security >0
        return false;

    if(is_name( "nsupport", name))		// newbie support
        return false;

	if(is_name( "bard", name ))			// bards, for notes to bard
		return false;

	if(is_name( "karn", name))			// karns 
        return false;

	// can't be named after clans
	if(clan_nlookup(name)>0)
		return false;

	// can't be named after a race
	if(race_exact_lookup(name)>=0){
		return false;
	}

	// can't be named after a class
	if(class_exact_lookup(name)>=0){
		return false;
	}

    // Length restrictions.    
    if(str_len(name) <= 2 )
		return false;

    if(str_len(name)>12)
		return false;

	if(name_confirmed)
		return true;	

    //  player specific 
	if(is_name( "kal", name))				// kalahn
        return false;
	if(is_name_infix( name, "alahn elahn alehn elehn nhala"))	// kalahn
        return false;

	if(is_name( "yle", name))			// ylerin
        return false;

	if(is_name( "quox", name))		    // quoxatyl
        return false;

    if(is_name( "reav", name))			// reave
        return false;

	if(is_name( "imi", name))			// imidazole
        return false;

	 // Alphanumerics only.
     // Lock out IllIll twits.
	{
		char *pc;
		bool fIll,adjcaps = false,cleancaps = false;
		int total_caps = 0;
		
		fIll = true;
		for( pc = name; *pc != '\0'; pc++ )
		{
			if(!is_alpha(*pc) && *pc!='-')
				return false;
			
			if(is_upper(*pc)) // ugly anti-caps hack 
			{
				if(adjcaps)
					cleancaps = true;
				total_caps++;
				adjcaps = true;
			}
			else
				adjcaps = false;
			
			if(LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
				fIll = false;
		}
		
		if(fIll )
			return false;
		
		if(cleancaps || (total_caps > (int) (str_len(name) / 2) && str_len(name) < 3))
			return false;
    }

    // Prevent players from naming themselves after mobs.
    {
		extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
		MOB_INDEX_DATA *pMobIndex;
		int iHash;

		for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
		{
			for( pMobIndex  = mob_index_hash[iHash];
			  pMobIndex != NULL;
			  pMobIndex  = pMobIndex->next )
			{
			if(is_name( name, pMobIndex->player_name ) )
				return false;
			}
		}
    }
   
	{
     // Prevent players from naming themselves after pkilled players
		FILE* file;
		char buf[MSL];

		sprintf(buf, "%s%s", DEAD_DIR, name);
		fclose(fpReserve);		 // close the reserve file 
		file = fopen (buf, "r"); // attempt to to open pkilled pfile 
		if(file)
		{
			fclose(file);
    		fpReserve = fopen( NULL_FILE, "r" ); 
			return false;		
		}
   		fpReserve = fopen( NULL_FILE, "r" );  // reopen the reserve file
	}

    // Prevent players from naming themselves after deleted players
	if(file_existsf("%s%s", DELETE_DIR, pfile_filename(name))){
		return false;
	}

    // Prevent players from naming themselves after dead players
	if(file_existsf("%s%s", DEAD_DIR, pfile_filename(name))){
		return false;
	}

	// Prevent players from naming themselves after builders and credits 
	// on the arealist
	if(GAMESETTING5(GAMESET5_PREVENT_PLAYERS_NAMING_AFTER_AREALIST_NAMES)){
		for ( AREA_DATA *pArea = area_levelsort_first; pArea; pArea = pArea->levelsort_next)
		{
			if(IS_NULLSTR(pArea->credits)){
				if(is_exact_name( name, pArea->builders)){
					return false;
				}
			}else{
				if(is_exact_name( name, pArea->credits)){
					return false;
				}
			}
		}
	}
    return true;
}


/**************************************************************************/
// extremely ugly routine... needs a complete rewrite 
// if someone can be bothered.
void roll_stats(connection_data *d)
{
	char_data *ch, *v;
    int i;
    int total;
	int bias;
    char buf[MSL], sendbuf[MSL];
	char * widthbuf="          ";
	bias = 0;
	sendbuf[0]='\0';
    ch = d->character;

	total=0;

	v=ch;

	// generate the stats
	{
		attributes_set rm_stats_set;
		gen_rolemaster_stats(&rm_stats_set, ch->pcdata->reroll_counter);

		for(i=0; i<MAX_STATS; i++)
		{
			ch->perm_stats[i]= rm_stats_set.perm[i];
			ch->potential_stats[i]=rm_stats_set.potential[i];
		}

	}

	sprintf(buf,"  `xStrength (ST):`g%3d`x(`B%3d`x)   Constitution   (CO):`g%3d`x(`B%3d`x)\r\n",
		v->perm_stats[STAT_ST], v->potential_stats[STAT_ST],
		v->perm_stats[STAT_CO], v->potential_stats[STAT_CO]);
	strcat(sendbuf, widthbuf);
	strcat(sendbuf, buf);

	sprintf(buf,"  `xQuickness(QU):`g%3d`x(`B%3d`x)   Agility        (AG):`g%3d`x(`B%3d`x)\r\n",
		v->perm_stats[STAT_QU], v->potential_stats[STAT_QU], 
		v->perm_stats[STAT_AG], v->potential_stats[STAT_AG]); 
	strcat(sendbuf, widthbuf);
	strcat(sendbuf, buf);

	sprintf(buf,"  `xPresence (PR):`g%3d`x(`B%3d`x)   Self-Discipline(SD):`g%3d`x(`B%3d`x)\r\n",
		v->perm_stats[STAT_PR], v->potential_stats[STAT_PR], 
		v->perm_stats[STAT_SD], v->potential_stats[STAT_SD]); 
	strcat(sendbuf, widthbuf);
	strcat(sendbuf, buf);

	sprintf(buf,"  `xEmpathy  (EM):`g%3d`x(`B%3d`x)   Memory         (ME):`g%3d`x(`B%3d`x)\r\n",
		v->perm_stats[STAT_EM], v->potential_stats[STAT_EM], 
		v->perm_stats[STAT_ME], v->potential_stats[STAT_ME]);
	strcat(sendbuf, widthbuf);
	strcat(sendbuf, buf);

	sprintf(buf,"  `xIntuition(IN):`g%3d`x(`B%3d`x)   Reasoning      (RE):`g%3d`x(`B%3d`x)\r\n",
		v->perm_stats[STAT_IN], v->potential_stats[STAT_IN], 
		v->perm_stats[STAT_RE], v->potential_stats[STAT_RE]);
	strcat(sendbuf, widthbuf);
	strcat(sendbuf, buf);

	sprintf(buf,"  `ggreen is starting value, `B(blue) is potential with training`x.\r\n");
	strcat(sendbuf, widthbuf);
	strcat(sendbuf, buf);


	if(GAMESETTING(GAMESET_SHOW_STAT_AVERAGES_IN_CREATION)){
		int avpot=0, avperm=0; 

		for(i=0; i<MAX_STATS; i++)
		{
			avperm+= ch->perm_stats[i];
			avpot+= ch->potential_stats[i];
		}
		sprintf(buf,"  `gaverage start %3d`B (average potential %3d)`x\r\n", avperm/MAX_STATS, avpot/MAX_STATS);
		strcat(sendbuf, widthbuf);
		strcat(sendbuf, buf);
	}
	strcat(sendbuf, "Are you happy with these attributes?\r\n");
	
	ch->printf("`=j%s`x", creation_titlebar("CHARACTER ATTRIBUTES SELECTION"));
	ch->print(sendbuf);
}
/**************************************************************************/
// returns true if the person was disconnected
int count_creation_connections_per_hour(connection_data *d){
	CREATION_COUNTER_DATA* tc;
	int count=0;
	// check them to the head of the linked list
	for(tc = creation_counter; tc; tc=tc->next)
	{
		if( !str_cmp(tc->ip, d->remote_ip)
			&& (tc->time> current_time-60*60))
		{
			count++;
		}
	}

	if(!GAMESETTING5(GAMESET5_RESTRICTED_CREATIONS_PER_HOUR) && count<40){
		return 0;
	}

	return count;
}

/**************************************************************************/
// returns true if the person was disconnected
bool check_connection(connection_data *d){

	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){
        return false;
    }

	if(count_creation_connections_per_hour(d)>5){
		d->write(note_format_string(str_dup(
			"`1`1Due to abuse or potential abuse of the character creation system, "
			"we limit the number of characters that can be created within "
			"an hour or so from a single site.  Try creating again an hour or so.`1`1")), 0);

		logf("%s %s (%d) dropped - creating too many times.",
			CH(d)->name, d->remote_hostname, d->connected_socket );

		{ 
			char log_buf[MSL];

			sprintf( log_buf, "%s %s (%d) dropped - creating to many times.\r\n",
				CH(d)->name, d->remote_hostname, d->connected_socket );
			wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(CH(d)) );
		}

		connection_close(d);
		return true;
	}
	return false;
}
/**************************************************************************/
void add_connection(connection_data *d)
{
	CREATION_COUNTER_DATA* tc;

	tc = new CREATION_COUNTER_DATA;

	tc->ip	= str_dup(d->remote_ip);
	tc->time = current_time;

	// add them to the head of the linked list
	tc->next=creation_counter;
	creation_counter=tc;
}
/**************************************************************************/
// go on to selecting the race
void nannysup_past_email_check(connection_data *d, const char *) 
{
	int race;
	// display races
	CH(d)->printf("`=j%s`x", creation_titlebar("RACE SELECTION"));
	CH(d)->wrapln(
		"It it now time to select a race... You can read online help on any "
		"race by simply typing `=Chelp `W<racename>`x where `W<racename>`x is the "
		"name of the race you are interested in (e.g. `=Chelp `=_human`x).  "
		"If you are unsure about which race to play, we recommend human as "
		"they are good all round race, and start with the most commonly spoken "
		"language.`1");

	if(!GAMESETTING5(GAMESET5_RACEINFO_DISABLED_IN_CREATION)){
		CH(d)->wraplnf(
		"Note: You can type `=Craceinfo`x for a list of races, their "
		"racial attribute modifiers, creation point cost and "
		"maximum hitpoints.`1");
	}

	if(!codehelp(CH(d), "racial_option", false)){
		CH(d)->printf("Please select your race from one of the following:`1 ");
		int count=0;
		for( race = 0; race_table[race]; race++ )
		{
			// creation selectable pc races only
			if(!race_table[race]->creation_selectable()
				|| (race_table[race]->remort_number > d->creation_remort_number)){
				continue;
			}
			CH(d)->printf(" `S[`Y%12.12s`S]", race_table[race]->name);
			if(++count%5==0){
				CH(d)->printf("`x\r\n ");
			}
		}
		CH(d)->printf("`1`xType in the name of the race you wish to play now:`1");
	}
	
	d->connected_state = CON_GET_NEW_RACE;
}
/**************************************************************************/
void nannysup_setprime_stats( char_data *ch )
{
	bool	changed = false;

	// if character's prime stats are below 80 they get set to 80.
	// if character's prime stats are 90+ they get +5 to their prime to a max of 101
	int old;
	// Do first primary stat
	if(ch->potential_stats[class_table[ch->clss].attr_prime[0]] < 80 )
	{
		old=ch->potential_stats[class_table[ch->clss].attr_prime[0]];
		ch->potential_stats[class_table[ch->clss].attr_prime[0]] = 80;
		ch->printlnf("Your %s has increased from %d to 80!",
			stat_flags[class_table[ch->clss].attr_prime[0]].name,
			old);
		changed = true;
	}
	else if(ch->potential_stats[class_table[ch->clss].attr_prime[0]] < 90 )
	{
		old=ch->potential_stats[class_table[ch->clss].attr_prime[0]];
		ch->potential_stats[class_table[ch->clss].attr_prime[0]] 
			+= (UMIN(ch->potential_stats[class_table[ch->clss].attr_prime[0]]-78, 10)/2);
		ch->printlnf("Your %s has increased from %d to %d!",
			stat_flags[class_table[ch->clss].attr_prime[0]].name,
			old,
			ch->potential_stats[class_table[ch->clss].attr_prime[0]]);
		changed = true;
	}
	else // if(ch->potential_stats[class_table[ch->clss].attr_prime[0]] => 90 )
	{
		old=ch->potential_stats[class_table[ch->clss].attr_prime[0]];
		ch->potential_stats[class_table[ch->clss].attr_prime[0]] += 5;
		if(ch->potential_stats[class_table[ch->clss].attr_prime[0]] > 101)
		{
			ch->potential_stats[class_table[ch->clss].attr_prime[0]] = 101;
		}
		ch->printlnf("Your %s has increased from %d to %d!",
			stat_flags[class_table[ch->clss].attr_prime[0]].name,
			old,
			ch->potential_stats[class_table[ch->clss].attr_prime[0]]);
		changed = true;
	}

	// Do second primary stat
	if(ch->potential_stats[class_table[ch->clss].attr_prime[1]] < 80 )
	{
		old=ch->potential_stats[class_table[ch->clss].attr_prime[1]];
		ch->potential_stats[class_table[ch->clss].attr_prime[1]] = 80;
		ch->printlnf("Your %s has increased from %d to 80!",
			stat_flags[class_table[ch->clss].attr_prime[1]].name,
			old);
		changed = true;
	}
	else if(ch->potential_stats[class_table[ch->clss].attr_prime[1]] < 90 )
	{
		old=ch->potential_stats[class_table[ch->clss].attr_prime[1]];
		ch->potential_stats[class_table[ch->clss].attr_prime[1]] 
			+= (UMIN(ch->potential_stats[class_table[ch->clss].attr_prime[1]]-78, 10)/2);
		ch->printlnf("Your %s has increased from %d to %d!",
			stat_flags[class_table[ch->clss].attr_prime[1]].name,
			old,
			ch->potential_stats[class_table[ch->clss].attr_prime[1]]);
		changed = true;
	}
	else // if(ch->potential_stats[class_table[ch->clss].attr_prime[1]] => 90 )
	{
		old=ch->potential_stats[class_table[ch->clss].attr_prime[1]];
		ch->potential_stats[class_table[ch->clss].attr_prime[1]] += 5;
		if(ch->potential_stats[class_table[ch->clss].attr_prime[1]] > 101)
		{
			ch->potential_stats[class_table[ch->clss].attr_prime[1]] = 101;
		}
		ch->printlnf("Your %s has increased from %d to %d!",
			stat_flags[class_table[ch->clss].attr_prime[1]].name,
			old,
			ch->potential_stats[class_table[ch->clss].attr_prime[1]]);
		changed = true;
	}

	if(!changed){
		ch->println("Your stats have remained the same.");
	}
	return;
}
/**************************************************************************/
void do_clear_createcount( char_data *ch, char *argument )
{
	CREATION_COUNTER_DATA *tc, *next_tc; 

	if(str_cmp("confirm", argument)) {
	    ch->println("Syntax: clear_createcount confirm");			
	    ch->println("notes:  It clears the counter of who created in the last hour.");			
		return;
	}    
	for(tc = creation_counter; tc; tc = next_tc){
		next_tc = tc->next;
		delete tc;
	}
	creation_counter=NULL;
	ch->println("Creation counter cleared.");
}
/**************************************************************************/
void nsupport_newbie_alert( char_data *ch, bool created )
{
	connection_data *d;

	for( d = connection_list; d != NULL; d = d->next )
	{
		if(  d->connected_state == CON_PLAYING
			&& CH(d)
			&& d->character != ch
			&& IS_NEWBIE_SUPPORT( d->character ))
		{
			// an existing newbie logging on,
			// if they have newbie channel off, then no warning is given
			if(created && !HAS_CONFIG( ch, CONFIG_NONEWBIE ))
			{
				d->character->printlnf( "`sNewbie '%s' has just logged on.`x", ch->name );
			}
			if(!created )
			{
				d->character->printlnf( "`sNew player '%s' is creating, please make them feel welcome when they arrive.", ch->name );
			}
        }
	}
}

/**************************************************************************/
void greet_new_connection(connection_data *d);
/**************************************************************************/
// give time for the telnet option negotiation to work
void nanny_detect_client_settings(connection_data *c, char *argument) 
{
	bool t=c->fcommand;
	c->fcommand=true;

#ifdef SHOW_CLIENT_DETECTION
	if(c->outtop==0){
		write_to_buffer( c, ".", 1);
		if(c->connected_state_pulse_counter%42==0){
			write_to_buffer( c, "\r\n", 2);
		}
	}
#endif

#ifdef SHOW_CLIENT_DETECTION
	if(!IS_NULLSTR(argument)){
		write_to_buffer( c, "^", 1);
	}
#endif
	if(!IS_NULLSTR(argument) || ++c->connected_state_pulse_counter>PULSE_PER_SECOND*4){
#ifdef SHOW_CLIENT_DETECTION
		{
	//		write_to_buffer( c,".. detection completed.\033\133\062\113\r\n", 0); 		
		// the above esc code sequence was used to delete the detect line
			write_to_buffer( c,".. detection completed.\r\n", 0); 		
		}
#endif
		c->fcommand=t;
		greet_new_connection(c);

		// jump straight into username input if detection was aborted
		// for any keystroke other than enter
		c->connected_state=CON_GET_NAME;
		if(!IS_NULLSTR(argument)){
			if(*argument!=' ' && *(argument+1)!='\0'){
				nanny(c, argument);
			}
		}
		return;
	}
	c->fcommand=t;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

