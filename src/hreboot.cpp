/**************************************************************************/
// hreboot.cpp - Online hotreboot, 100% by Kalahn 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/**************************************************************************/
// - Win32 isn't supported, technically it looks possible but impractical
/**************************************************************************/
#include "network.h"
#ifdef unix
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <signal.h>
#endif
#ifdef WIN32
	#include <process.h>
#endif

#define __SEE_NETIO_INTERNAL_STRUCTURES__ // we get some extra structs from netio.h
#include "include.h"
#include "hreboot.h"
#include "nanny.h"
#include "laston.h"
#include "intro.h"
#include "msp.h"
#include "comm.h"
#include "netio.h"

/**************************************************************************/
// globals
bool hotreboot_in_progress=false;
int	hotreboot_ipc_pipe=-1;
char_data		*last_loaded_char=NULL;
connection_data *last_loaded_connection=NULL;
void laston_save(char_data *);
bool hotreboot_GEL_listen_on(char *argument);
void hotreboot_send_listen_on_list(listen_on_type *listen_on_entry);
void netio_binded_listen_on_output();
/**************************************************************************/
extern bool listening_on_ipv4_addresses;
extern bool listening_on_ipv6_addresses;
/**************************************************************************/
#define GET_OFFSET(datatype, field) (ptr_val)(((char*) &datatype.field)\
		-((char*) &datatype)) 

#define GET_ADDRESS(dataptr, offset) \
	((void *)( (char*)dataptr + offset)) 
		

connection_data temp_connection;
char_data		temp_char;
pc_data			temp_pcdata;
NOTE_DATA		temp_note;

char *hotreboot_get_send_format(void *struct_head, char* in_between, int i);
/**************************************************************************/
void hotreboot_debug(char *fmt, ...)
{
#ifdef DEBUG_HOTREBOOT_TO_LOG
	// support variable arguments
    char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 2048, fmt, args);
	va_end(args);

	logf("HOTREBOOT DEBUGMESSAGE: %s", buf);
#else
	fmt=NULL; // do something to remove compile warnings
#endif 
	return;
}
/**************************************************************************/
// prototypes
void laston_update_char args((char_data *));
void set_char_magic_bits(char_data * ch);
void reboot_autosave( char_data *ch);
void do_pinfo(char_data *ch, char *argument );
void do_cwhere(char_data *ch, char *argument );
void do_look(char_data *ch, char *argument );
/**************************************************************************/
#define HOTREBOOT_PARENT_PIPE 0
#define HOTREBOOT_CHILD_PIPE  1

/**************************************************************************/
hotreboot_field_table_type hotreboot_field_table[]=
{
#ifndef WIN32
	// sent initially	
	{ "resolver_running",	(ptr_val)&resolver_running,		DT_INT,		HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},
	{ "resolver_stdinout",	(ptr_val)&resolver_stdinout,	DT_INT,		HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},
	{ "resolver_stderr",	(ptr_val)&resolver_stderr,		DT_INT,		HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},
	{ "resolver_version",	(ptr_val)&resolver_version,		DT_INT,		HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},
	{ "mainport",			(ptr_val)&mainport,				DT_INT,		HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},

	{ "max_on",				(ptr_val)&max_on,				DT_INT,		HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},
	{ "maxon_time",			(ptr_val)&maxon_time,			DT_LONG,	HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},
	{ "true_count",			(ptr_val)&true_count,			DT_INT,		HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},
	{ "lastreboot_time",	(ptr_val)&lastreboot_time,		DT_LONG,	HRW_GLOBAL_VARIABLE, HRF_INIT_SEND| HRF_CHECK_NUMERIC},

	// sent on a per character basis
	{ "logon",		GET_OFFSET(temp_char,	logon),			DT_LONG,	HRW_LASTCHAR	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},	
	{ "idle",		GET_OFFSET(temp_char,	idle),			DT_SHORT,	HRW_LASTCHAR	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},	
	{ "remote_ip_copy",	GET_OFFSET(temp_char,	remote_ip_copy),DT_STR,	HRW_LASTCHAR	,HRF_SEND_AS_CHARFIELD },	

	// support transferring character notes thru the hotreboot
	{ "note_type",			GET_OFFSET(temp_note,	type),			DT_SHORT,	HRW_LASTCHAR_NOTE,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC },
	{ "note_sender",		GET_OFFSET(temp_note,	sender),		DT_STR,		HRW_LASTCHAR_NOTE,HRF_SEND_AS_CHARFIELD },	
	{ "note_real_sender",	GET_OFFSET(temp_note,	real_sender),	DT_STR,		HRW_LASTCHAR_NOTE,HRF_SEND_AS_CHARFIELD },	
	{ "note_date",			GET_OFFSET(temp_note,	date),			DT_STR,		HRW_LASTCHAR_NOTE,HRF_SEND_AS_CHARFIELD },	
	{ "note_to_list",		GET_OFFSET(temp_note,	to_list),		DT_STR,		HRW_LASTCHAR_NOTE,HRF_SEND_AS_CHARFIELD },	
	{ "note_subject",		GET_OFFSET(temp_note,	subject),		DT_STR,		HRW_LASTCHAR_NOTE,HRF_SEND_AS_CHARFIELD },	
	{ "note_text",			GET_OFFSET(temp_note,	text),			DT_STR,		HRW_LASTCHAR_NOTE,HRF_SEND_AS_CHARFIELD },	
	{ "note_data_stamp",	GET_OFFSET(temp_note,	date_stamp),	DT_LONG,	HRW_LASTCHAR_NOTE,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},	

	// connection settings on the last connection
	{ "ident_raw_result",GET_OFFSET(temp_connection,	ident_raw_result),	DT_STR,		HRW_LASTCONNECTION,HRF_SEND_AS_CHARFIELD	},
	{ "ident_username",GET_OFFSET(temp_connection,ident_username),DT_STR,		HRW_LASTCONNECTION,HRF_SEND_AS_CHARFIELD	},
	{ "colour_mode",GET_OFFSET(temp_connection,	colour_mode),	DT_INT,		HRW_LASTCONNECTION,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "multilog",	GET_OFFSET(temp_connection,multiple_logins),	DT_BOOL,	HRW_LASTCONNECTION,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "idle_since",	GET_OFFSET(temp_connection,	idle_since),	DT_LONG,	HRW_LASTCONNECTION,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	

//	{ "connected_socket",	GET_OFFSET(temp_connection,	connected_socket),	DT_INT,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
//	{ "protocol",			GET_OFFSET(temp_connection,	protocol),			DT_INT,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
//	{ "contype",			GET_OFFSET(temp_connection,	contype),			DT_INT,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "local_ip",			GET_OFFSET(temp_connection,	local_ip),			DT_STR,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD },
	{ "local_port",			GET_OFFSET(temp_connection,	local_port),		DT_INT,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "local_tcp_pair",		GET_OFFSET(temp_connection,	local_tcp_pair),	DT_STR,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD },
	{ "remote_ip",			GET_OFFSET(temp_connection,	remote_ip),			DT_STR,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD },
	{ "remote_port",		GET_OFFSET(temp_connection,	remote_port),		DT_INT,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "remote_tcp_pair",	GET_OFFSET(temp_connection,	remote_tcp_pair),	DT_STR,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD },
	{ "remote_hostname",	GET_OFFSET(temp_connection,	remote_hostname),	DT_STR,		HRW_LASTCONNECTION,	HRF_SEND_AS_CHARFIELD },
	{ "resolved",	GET_OFFSET(temp_connection,	resolved),		DT_BOOL,	HRW_LASTCONNECTION	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},

	{ "mccp_version",GET_OFFSET(temp_connection,	mccp_version),	DT_SHORT,	HRW_LASTCONNECTION	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "bytes_sent",	GET_OFFSET(temp_connection,	bytes_sent),	DT_INT,		HRW_LASTCONNECTION	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "flags",		GET_OFFSET(temp_connection,	flags),			DT_INT,		HRW_LASTCONNECTION	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "bsbc",GET_OFFSET(temp_connection,bytes_sent_before_compression),	DT_INT,		HRW_LASTCONNECTION	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "bsac",GET_OFFSET(temp_connection,bytes_sent_after_compression),	DT_INT,		HRW_LASTCONNECTION	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
	{ "bsac",GET_OFFSET(temp_connection,bytes_sent_after_compression),	DT_INT,		HRW_LASTCONNECTION	,HRF_SEND_AS_CHARFIELD | HRF_CHECK_NUMERIC},
#endif // not WIN32

	{ "", 0, DT_END, HRW_END, 0}
};
/**************************************************************************/
void hotreboot_check_tables()
{
	bool error_found=false;

	for(int i=0; !IS_NULLSTR(hotreboot_field_table[i].name); i++){
		switch(hotreboot_field_table[i].datatype){
			case DT_INT:
			case DT_LONG:
			case DT_SHORT: 
			case DT_BOOL:
				if(!IS_SET(hotreboot_field_table[i].flags,HRF_CHECK_NUMERIC)){			
					bugf("Hotreboot field %s needs HRF_CHECK_NUMERIC flag set!",
						hotreboot_field_table[i].name);
					error_found=true;
				}
				break;
			default:
				break;
		}	
	}
	if(error_found){
		bug("Wont boot with problems in hotreboot_field_table[], fix them then recompile.");
		exit_error( 1 , "hotreboot_check_tables", "problems in hotreboot_field_table[]");
	}
};
/**************************************************************************/
void direct_to_char(char_data*ch, char*message)
{
	if(ch->desc)
	{
       ch->desc->write(message,0);
	}
};
/**************************************************************************/
void hotreboot_reassign_child_pipe(int val)
{
	logf("hotreboot: child assigning hotreboot_ipc_pipe to %d.", val);
	hotreboot_ipc_pipe=val;
};
/**************************************************************************/
// return true if sucessful
// each sent transmission is prefixed with the length of the transmission
bool hotreboot_send_command(char *fmt, ...)
{
	// support variable arguments
    char buf[HSL-100];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, HSL-100, fmt, args);
	va_end(args);

	char sendbuf[HSL];
	int sent;
	ptr_val len=(ptr_val)str_len(buf);

	strcpy(sendbuf+sizeof(ptr_val),buf);
	strncpy(sendbuf, (char*)&len, sizeof(len)); 

	sent=send(hotreboot_ipc_pipe, sendbuf, sizeof(len) + len ,0);

	if((unsigned int)sent!=sizeof(ptr_val) + len){
		bugf("hotreboot_send_command(): sent=%d which is not sizeof("
			"len)+len (%d)!!!", sent, (int)(sizeof(len)+len));
		return false;
	}

	hotreboot_debug("hotreboot_send_command(%d): send '%s'", hotreboot_ipc_pipe, buf);
	return true;
};
/**************************************************************************/
// return the string or NULL if nothing left to be read?
// each sent transmission is prefixed with the length of the transmission
// therefore we pick off that first, then return 
char *hotreboot_receive_command(int *received)
{
	static char receivebuf[HSL];
	ptr_val len;
	
	// support a NULL argument
	int unwanted_received;
	if(received==NULL){
		received=&unwanted_received;
	}
	hotreboot_debug("hotreboot_receive_command(): hotreboot_ipc_pipe=%d",hotreboot_ipc_pipe);

	memset(receivebuf,0,HSL);
	// get the length value
	*received=recv(hotreboot_ipc_pipe, (char*)(&len), sizeof(len),0);
	if(*received!=sizeof(ptr_val)){
		bugf("hotreboot_receive_command(): *received=%d which is not "
			"sizeof(len)(%d)!!!",	*received, (int)sizeof(len));
		return NULL;
	}

	hotreboot_debug("hotreboot_receive_command(): incoming message length=%d, received=%d", 
		len, *received);

	// get the message content
	*received=recv(hotreboot_ipc_pipe, receivebuf, len ,0);
	if((ptr_val)(*received)!=len){
		bugf("hotreboot_receive_command(): *received=%d which is not "
			"len(%d)!!!", *received, (int)len);
		return NULL;
	}
	
	hotreboot_debug("hotreboot_receive_command(%d): returning '%s'",
		hotreboot_ipc_pipe,receivebuf);
	return receivebuf;
};
/**************************************************************************/
// sends thru the parent pipe all the controls etc
void hotreboot_init_send()
{	
	for(int i=0; !IS_NULLSTR(hotreboot_field_table[i].name); i++){
		// only send the fields that are marked for initial send
		if(IS_SET(hotreboot_field_table[i].flags, HRF_INIT_SEND)){
			hotreboot_send_command("field %s", 				
				hotreboot_get_send_format(0,"", i));
		}
	}; 
	hotreboot_send_listen_on_list(listen_on_first);
	hotreboot_send_command("end_init_send"); 
}
/**************************************************************************/
// find the index of an id in the hotreboot_int_table[]
// return -1 if unfound
int hotreboot_lookup_field_table_index(char *fieldname)
{
	for(int i=0; !IS_NULLSTR(hotreboot_field_table[i].name); i++){
		if (!str_cmp( hotreboot_field_table[i].name, fieldname))
			return i;
	}	
	return -1;
}
/**************************************************************************/
// GEL = Game Environment Load
bool hotreboot_GEL_character(char *argument)
{
	char_data *ch;
	char charname[MSL], socketvalue[10], mccpvalue[10];
	int gel_socket;
	int gel_mccp_version;
	char sz_protocol[MIL];
	PROTOCOL_TYPE protocol;
	char sz_contype[MSL];	
	CONNECTION_TYPE contype;

	// get the charname and descriptor
	argument=one_argument(argument, charname);
	argument=one_argument(argument, socketvalue);
	argument=one_argument(argument, mccpvalue);
	argument=one_argument(argument, sz_protocol);
	argument=one_argument(argument, sz_contype);
		
	// check validity of descriptor value
	if(!is_number(socketvalue)){
		bugf("hotreboot_GEL_character(): socketvalue is not an integer '%s' for '%s'",
			socketvalue, charname);
		return false;
	}
	gel_socket=atoi(socketvalue);

	if(!is_number(mccpvalue)){
		bugf("hotreboot_GEL_character(): mccpvalue is not an integer '%s' for '%s'",
			mccpvalue, charname);
		return false;
	}
	gel_mccp_version=atoi(mccpvalue);

	// check that the protocol is either ipv6 or ipv4
	if(!str_cmp(sz_protocol, "ipv6")){
		protocol=PROTOCOL_IPV6;
	}else if(!str_cmp(sz_protocol, "ipv4")){
		protocol=PROTOCOL_IPV4;
	}else{
		bugf("hotreboot_GEL_character(): protocol '%s' is not a recognised value for socket %d",
			sz_protocol, gel_socket);
		return false;
	}
	// check the contype
	contype=loparse_connection_type_lookup(sz_contype);
	if(contype==CONTYPE_UNSET){
		bugf("hotreboot_GEL_character(): '%s' is not a recognised connection type for socket %d",
			sz_contype, gel_socket);
		return false;
	}

	// load the player and attach them to the connection list etc
	connection_data *c=connection_allocate();
	c->connected_socket=gel_socket;
	c->mccp_version=gel_mccp_version;
	c->protocol=protocol;
	c->contype=contype;

#ifdef MCCP_ENABLED 
	// initialise mccp from our end
	if(c->mccp_version){
		c->continue_compression();
	}
#endif


	c->write("as though you are being pulled ",0);
	c->next=connection_list;
	c->connected_state = CON_DETECT_CLIENT_SETTINGS;
	connection_list=c;


	if(!load_char_obj( c, charname )){
		c->write("out of the game.\r\n"
			"For some reason your pfile has disappeared in the hotreboot... sorry, try logging on again.\r\n",0);
		connection_close(c);
	};

	ch=c->character;
	ch->next_player=player_list;
	player_list=ch;

	ch->next=char_list;
	char_list=ch;

	direct_to_char(ch, "from one place to another\r\n");

direct_to_char(ch,"\r\nNext before your eyes, or is it in your mind?"
"\r\nYou have the sensation like colours and feelings washing over your body...\r\n"
"\r\nThen mountains and hills begin to rise up around you as if they were alive!\r\n"
"\r\nYou just go blank, and then...");

	// put them back into the room and their pet
	char_to_room (ch, ch->in_room);
	if (ch->pet)
	{
		if (ch->pet->in_room){
			char_to_room(ch->pet,ch->pet->in_room); // restore to room they left from
		}else{
			char_to_room(ch->pet,ch->in_room); // restore to room the master
		}
	}

	c->connected_state = CON_PLAYING;
	last_loaded_char=ch;
	last_loaded_connection=c;
	return true;
}
/**************************************************************************/
void hotreboot_assign_received_field(void *struct_head, int i, char*argument)
{
#ifndef WIN32
	// first find 
	char buf[MSL];
	ptr_val value=0;
	ptr_val offset=hotreboot_field_table[i].offset;

	if(IS_SET(hotreboot_field_table[i].flags,HRF_CHECK_NUMERIC)){
		if(!is_number(argument)){
			bugf("hotreboot_assign_received_field(): non numeric value '%s' when it should be, field name='%s'!",
				argument, hotreboot_field_table[i].name);
			return;
		}
		value=atoi(argument);
		hotreboot_debug("hotreboot_assign_received_field(): value '%s' converted to %d!",
				argument, value);
	}

	switch(hotreboot_field_table[i].datatype){
	case DT_END: buf[0]='\0'; break;
	case DT_INT:
		*(int*)GET_ADDRESS(struct_head, offset)=value;
		break;
	case DT_LONG:
		*(long*)GET_ADDRESS(struct_head, offset)=value;
		break;
	case DT_SHORT: 
		*(short*)GET_ADDRESS(struct_head, offset)=(short)value;
		break;
	case DT_BOOL:
		if(value){
			(*(bool*)GET_ADDRESS(struct_head, offset))=true;
		}else{
			(*(bool*)GET_ADDRESS(struct_head, offset))=false;
		}
		break;
	case DT_STR:
		{
			(*(ptr_val*)((char *)struct_head + (offset)))=(ptr_val)str_dup(argument);
		}
		break;
	case DT_CHARDATA:
/*		sprintf(buf, "%s %s %ld", 
			hotreboot_field_table[i].name, 
			in_between,
			((char_data*)GET_ADDRESS(struct_head, offset))->id);
*/
		break;
	default:buf[0]='\0'; break;
	}
#endif // ifndef WIN32
	return;
}
/**************************************************************************/
// GEL = Game Environment Load
bool hotreboot_GEL_field(char *argument)
{
	char fieldname[MSL];
	void *data;
	int i;

	hotreboot_debug("hotreboot_GEL_field: '%s'", argument);
	// get the fieldname and descriptor
	argument=one_argument(argument, fieldname);

	// find the index of the field
	i=hotreboot_lookup_field_table_index(fieldname);
	if(i<0){
		bugf("hotreboot_GEL_field: Unrecognised field header '%s' (%s %s)",
			fieldname, fieldname, argument);
		return false;
	}

	switch(hotreboot_field_table[i].where){
		case HRW_END: data=NULL; break;
		case HRW_GLOBAL_VARIABLE: data=0; break;
		case HRW_LASTCHAR: data=(void*)last_loaded_char; break;
		case HRW_LASTCHAR_NOTE: // if we have a char, return their note								
			if(last_loaded_char){ 
				if(!last_loaded_char->pnote){
					last_loaded_char->pnote=new_note(); // allocate memory for a note if required
				}
				data=last_loaded_char->pnote;
			}else{
				data=NULL; // no one to attach the note to
			}
			break;
		case HRW_LASTPCDATA: data=last_loaded_char? (void*)last_loaded_char->pcdata:NULL; break;
		case HRW_LASTCONNECTION: data=last_loaded_connection; break;

		default:
		case HRW_FINDCHARACTER:			// by char id	
		case HRW_FINDCHAR_PCDATA:		// by char id	
		case HRW_FINDCHAR_DESCRIPTOR:		// by char id	
			data=NULL;
			bugf("hotreboot_GEL_field(): '%s' not implemented (%s %s)",
				fieldname, fieldname, argument);
			break;
	}

	if(!data && hotreboot_field_table[i].where!=HRW_GLOBAL_VARIABLE){
		return false;
	}

	hotreboot_assign_received_field(data, i, argument);
	return true;
}
/**************************************************************************/
void mp_login_trigger(char_data *ch);
/**************************************************************************/
void hotreboot_finalise()
{
	char_data *ch;
	for ( ch = player_list; ch; ch = ch->next_player )
	{
		attach_know(ch); // setups ch->know stuff
		assertp(ch->know);
	}

	for ( ch = player_list; ch; ch = ch->next_player )
	{
		// reset their magic bits 
		set_char_magic_bits(ch);

		// update laston
		laston_update_char(ch);

		// TODO: go thru sending all the extra info about players 
		//		 like who is following who etc

	    if ( IS_AWAKE(ch))
	    {
			ch->printf("\r\nWas that a dream?  You feel a little disoriented,\r\n"
				"\r\nlooking around, everything looks newer... like it was just created\r\n"
				"\r\nbut still everything looks the same... you look around again...\r\n");
			do_look (ch, "auto");
		}else{
			ch->println("\r\nYour dream fades away quickly as it surfaced...");
		}

		ch->mxp_send_init(); // detects if they have mxp
		msp_update_char(ch);
		mp_login_trigger(ch);
	}

};

/**************************************************************************/
extern char current_logfile_name[MSL];
/**************************************************************************/
// run by client - core loop that receives all info about the game
// environment on the parent side
void hotreboot_game_environment_transfer()
{
#ifndef unix
	return;
#endif
	logf("hotreboot_game_environment_transfer(): starting transfer of game environment");
	hotreboot_send_command("begin_environment_transfer");

	char incoming_command[MSL];

	char command[MSL];
	char *input;
	char *argument;
	int receive_count;

	for(;;){
		// get the next input line and the command type from the front
		argument=incoming_command;
		input=hotreboot_receive_command(&receive_count);
		// check for failed hotreboot
		if(receive_count<1){
			// parent process connection failed - we drop off hotreboot canceled
			exit_clean( 0, "hotreboot_game_environment_transfer", "parent process connection failed");
		};
		strcpy(incoming_command,input);

		argument=one_argument(argument, command);

		// GEL = Game Environment Load
		if(!str_cmp(command, "character")){
			hotreboot_GEL_character(argument);
		}else if(!str_cmp(command, "field")){
			hotreboot_GEL_field(argument);
		}else if(!str_cmp(command, "environment_transfer_completed")){
			hotreboot_send_command("new_log_filename=%s", current_logfile_name);
			hotreboot_send_command("new_process_id=%d", getpid());
			hotreboot_send_command("transfer_accepted"); 
			hotreboot_finalise();
			closesocket(hotreboot_ipc_pipe);
			logf("hotreboot_game_environment_transfer(): transfer completed.");
			hotreboot_ipc_pipe=-1;
			hotreboot_in_progress=false;
			return;
		}else{
			bugf("hotreboot_process_parent_side_progress(): Unknown command '%s' received in '%s'!!!",
				command, incoming_command);
		}
	}
	logf("hotreboot_game_environment_transfer(): hotreboot completed.");

};
/**************************************************************************/
// gets all the controls from the parent pipe 
void hotreboot_init_receive()
{
	char incoming_command[MSL];

	char command[MSL];
	char *argument;

	for(;;){
		// get the next input line and the command type from the front
		argument=incoming_command;
		strcpy(incoming_command,hotreboot_receive_command(NULL));
		argument=one_argument(argument, command);
		hotreboot_debug("hotreboot_init_receive(): command = %s", command);

		if(!str_cmp(command, "end_init_send")){
			hotreboot_debug("hotreboot_init_receive(): end_init_send received");
			logf("hotreboot_init_receive() completed.");
			break;
		}else if(!str_cmp(command, "field")){
			hotreboot_GEL_field(argument);
		}else if(!str_cmp(command, "listenon")){
			hotreboot_GEL_listen_on(argument);
		}else{
			bugf("hotreboot_init_receive(): Unknown command '%s' received in '%s'!!!",
				command, incoming_command);
			exit_error( 1 , "hotreboot_init_receive", "unknown command received");
		}
		
	}
	hotreboot_debug("hotreboot_init_receive(): end");
}
/**************************************************************************/
// returns true if successful
bool cleanup_after_failed_hotreboot(char_data* ch)
{
#ifdef WIN32
	ch->println("cleanup_after_failed_hotreboot() not yet implemented!");
	bug("cleanup_after_failed_hotreboot() not yet implemented!");
#else
	ch->print("");
#endif
	hotreboot_in_progress=false;
	return true;
}
/**************************************************************************/
// returns true if successful
bool prepare_for_hotreboot(char_data* ch)
{
	hotreboot_in_progress=true;

#ifdef WIN32
	// go thru duplicating all the socket descriptors
/*	// hotreboot in win32 was never completed, so this code is now commented out
	{
		connection_data *d;
		HANDLE ProcessHandle=GetCurrentProcess();
		int success;
		
		// need to duplicate the socket handles in order to make them
		// inherited when create_process is called
		for(d=connection_list; d; d=d->next)
		{
			success=DuplicateHandle(
						ProcessHandle, 
						(HANDLE)d->oldsocket,
						ProcessHandle, 
						(HANDLE *)&d->duplicated_descriptor, 
						0,
						true, 
						DUPLICATE_SAME_ACCESS);		

			if(!success){
				ch->printlnf("WIN32 Hotreboot failed during preparation stage while duplicating socket %d.",
					d->oldsocket);
				ch->printlnf("DuplicateHandle() error code = %d", GetLastError());
				bugf("WIN32 Hotreboot failed during preparation stage while duplicating socket %d.\r\n",
					d->oldsocket);
				bugf("DuplicateHandle() error code = %d\r\n", GetLastError());
				cleanup_after_failed_hotreboot(ch);
				return false;
			}
		};
	}
*/
#else
	ch->print("");
#endif

	return true;
};
/**************************************************************************/
void hotreboot_disconnect_non_CON_PLAYING_characters()
{	
	for (connection_data *d = connection_list; d; d=c_next)
	{
		c_next = d->next; 
		
		if (!d->character || d->connected_state != CON_PLAYING) // drop those logging on 
		{
            d->write("\r\nSorry, we are hotrebooting, come back in 30 seconds.\r\n", 0);
			connection_close(d); 
		}
	}
}

/**************************************************************************/
void do_save_corpses(char_data *ch, char *);
extern int main_argc;
extern char **main_argv;
extern bool commandlineoption_log_console;
extern FILE *current_logfile_descriptor;
/**************************************************************************/
// Written by Kalahn - October 99
// - currently only unix supported
void do_hotreboot(char_data* ch, char * argument)
{
#ifdef WIN32
	ch->println("Only supported with a unix server at this stage sorry.");
	return;
#else

	char exe_filename[MSL];

	if (IS_CONTROLLED(ch))
	{
        ch->println("You can't start a hotreboot while switched!");
		return; 
	}
	
	if (IS_NULLSTR(argument) || str_cmp("confirm", argument))
    {
		// do checks
		do_pinfo(ch, "");
		ch->println("PC corpses are automatically saved across hotreboots, unless "
			"someone dies after the hotreboot has begun.");
        ch->println("`xType `=Chotreboot confirm`x to do the hotreboot.");
		return;
	}

	// idiot checks
	if(hotreboot_in_progress){
		ch->println("There is already a hotreboot in progress!");
		return;
	}

	// do all the auto saves
	reboot_autosave( ch);

	if(!file_existsf("%s",EXE_FILE)){		
#ifdef __CYGWIN__
		if(!file_exists("%s.exe",EXE_FILE)){
			ch->printlnf("Couldn't find a new binary file called '%s' nor '%s.exe' to hotreboot over to.",
				EXE_FILE, EXE_FILE);
			return;
		}else{
			sprintf(exe_filename, "%s.exe", EXE_FILE);
		}
#else
		ch->printlnf("Couldn't find a new binary file called '%s' to hotreboot over to.",
			EXE_FILE);
		return;
#endif
	}else{
		strcpy(exe_filename, EXE_FILE);
	}

	logf("Starting hotreboot initiated by %s", PERS(ch, NULL));
	dawnlog_write_index(FORMATF("Starting hotreboot initiated by %s", PERS(ch, NULL)));

	// do necessary preparation work - set hotreboot_in_progress to true
	if(!prepare_for_hotreboot(ch)){
		logf("Hotreboot canceled");
		return; // hotreboot canceled
	};

	// connections outside of CON_PLAYING have to be dropped 
	// before we fork, otherwise we will leak their file descriptor
	hotreboot_disconnect_non_CON_PLAYING_characters();

	pid_t fork_result;
	// create our bi-directional pipe
	int hotreboot_pipes[2];

	int spr=socketpair(AF_UNIX, SOCK_STREAM, 0, hotreboot_pipes);
	if(spr!=0){
		bugf("do_hotreboot(): socketpair returned %d, errno=%d (%s)",
			spr, errno, strerror(errno));
	}else{
		hotreboot_debug("socketpair for ipc created successfully");
	}

	signal(SIGCHLD, SIG_IGN);

	fork_result=fork();

	// check if something went wrong
	if(fork_result<0){ 
		ch->println("Hotreboot failed initial fork() call - hotreboot cancelled!");
		bug("Hotreboot failed initial fork() call - hotreboot cancelled!");		
		cleanup_after_failed_hotreboot(ch);
		return;
	}

	// do what we have to do for each side of the pipe
	// starting with closing the ends of pipe we arent using
	if(fork_result){ // parent
		close(hotreboot_pipes[HOTREBOOT_CHILD_PIPE]);
		hotreboot_ipc_pipe=hotreboot_pipes[HOTREBOOT_PARENT_PIPE];
		logf("Starting hotreboot, parent hotreboot_ipc_pipe = %d",hotreboot_ipc_pipe);
		hotreboot_init_send(); // send thru all the connection controls etc
		// nothing more to do with the parent at this stage, rest 
		// handled by hotreboot_process_parent_side_progress()		
	}else{ // child
		ispell_done(); // close off ispell
		// close the parent's connection to the communication pipe
		close(hotreboot_pipes[HOTREBOOT_PARENT_PIPE]);
		
		// close all the reserve files
		if(fpReserve)			fclose(fpReserve);
		if(fpAppend2FilReserve)	fclose(fpAppend2FilReserve);
		if(fpReserveFileExists)	fclose(fpReserveFileExists);

		// close the logging file descriptor
		if(current_logfile_descriptor) fclose(current_logfile_descriptor);

		char child_pipe[10];
		sprintf(child_pipe,"%d", hotreboot_pipes[HOTREBOOT_CHILD_PIPE]);

		{
			// start up the new mud process
			execl( exe_filename, 
				EXE_FILE,
				FORMATF("%d", mainport),
				commandlineoption_log_console?"-lc":"-q",
				FORMATF("--hotreboot=%d", hotreboot_pipes[HOTREBOOT_CHILD_PIPE]), 
				(void*)0);
		}
	}
	ch->println("Hotreboot begun!");

	// loop thru, everyone in building mode gets notified
	for (connection_data *d=connection_list; d; d=d->next){
		if(d->editor){
			CH(d)->wrapln("`R===`YA background hotreboot has just begun, anything OLC related you do "
				"after this point in time wont appear in the rebooted mud!`R===`x");
		}else if (IS_IMMORTAL(CH(d))){
			CH(d)->wrapln("`R===`YA background hotreboot has just begun.`x");
		}
	}
#endif
}
/**************************************************************************/
void alarm_update();
/**************************************************************************/
void hotreboot_wait_for_shutdown_confirm()
{
#ifdef unix
	alarm_update();
#endif
	logf("Waiting for 'transfer_accepted' from child mud process.");

	char incoming_command[MSL];

	char command[MSL];
	char *input;
	char *argument;
	int receive_count;

	for(;;){
		// get the next input line and the command type from the front
		argument=incoming_command;
		input=hotreboot_receive_command(&receive_count);
		// check for failed hotreboot
		if(receive_count<1){
			for ( char_data *wch = player_list; wch; wch = wch->next_player )
			{
				if(IS_IMMORTAL(wch)){	
					wch->println("Hotreboot failed!");
				}
			}
			// child process closed down for some reason - hotreboot canceled
			hotreboot_ipc_pipe=-1;
			hotreboot_in_progress=false;
			return;
		};
		strcpy(incoming_command,input);

		argument=one_argument(argument, command);

		if(!str_prefix("new_log_filename=", command)){
			logf("received notification child process is logging to %s",
				&command[str_len("new_log_filename=")]);
			log_notef("note: on machines which have the tail command installed, "
				"you can follow this log using the command:  tail -n 5000 -f %s", 
				&command[str_len("new_log_filename=")]);
			dawnlog_write_index(FORMATF("hotreboot child mud is logging to %s",
				&command[str_len("new_log_filename=")]));
		}else if(!str_prefix("new_process_id=", command)){
			logf("received notification child process id is %s",
				&command[str_len("new_process_id=")]);
			dawnlog_write_index(FORMATF("hotreboot child mud has a process id of %s",
				&command[str_len("new_process_id=")]));
		}else if(!str_cmp(command, "transfer_accepted")){			
			logf("'transfer_accepted' received by parent - parent closing down.");
			dawnlog_write_index("hotreboot transfer accepted, parent shutting down.");
			hotreboot_send_command("hotreboot_completed");
			exit_clean(0, "hotreboot_wait_for_shutdown_confirmation", "transfer accepted");
		}else{
			bugf("hotreboot_process_parent_side_progress(): Unknown command '%s' received in '%s'!!!",
				command, incoming_command);
		}
	}
}
/**************************************************************************/
char *hotreboot_get_send_format(void *struct_head, char* in_between, int i)
{
	static char buf[MSL];
	buf[0]='\0';
#ifndef WIN32
	ptr_val offset=hotreboot_field_table[i].offset;

	switch(hotreboot_field_table[i].datatype){
	case DT_END: buf[0]='\0'; break;
	case DT_INT: 
		sprintf(buf, "%s %s %d", 
			hotreboot_field_table[i].name, 
			in_between,
			*(int*)GET_ADDRESS(struct_head, offset));
		break;
	case DT_LONG:
		sprintf(buf, "%s %s %ld", 
			hotreboot_field_table[i].name, 
			in_between,
			*(long*)GET_ADDRESS(struct_head, offset));
		break;
	case DT_SHORT: 
		sprintf(buf, "%s %s %d", 
			hotreboot_field_table[i].name, 
			in_between,
			*(short*)GET_ADDRESS(struct_head, offset));
		break;
	case DT_BOOL:
		sprintf(buf, "%s %s %d", 
			hotreboot_field_table[i].name, 
			in_between,
			(*(bool*)GET_ADDRESS(struct_head, offset))==true?1:0);
		break;
	case DT_STR:
		sprintf(buf, "%s %s %s", 
			hotreboot_field_table[i].name, 
			in_between,
			// ugly type casting here, I am sure it could be 
			// improved but I have never bothered - Kal, Oct 99
			((char *)(*( (ptr_val *) ((char *)struct_head + (offset) ) ))));
		break;
	case DT_CHARDATA:
		sprintf(buf, "%s %s %ld", 
			hotreboot_field_table[i].name, 
			in_between,
			((char_data*)GET_ADDRESS(struct_head, offset))->player_id);
		break;
	default:buf[0]='\0'; break;
	}
#endif // ifndef WIN32
	return buf;
}
/**************************************************************************/
void hotreboot_send_charfields(char_data *ch)
{
	void *data;
	for(int i=0; !IS_NULLSTR(hotreboot_field_table[i].name); i++){
		switch(hotreboot_field_table[i].where){
			case HRW_LASTCHAR:		data=(void*)ch; break;
			case HRW_LASTCHAR_NOTE: data=(void*)ch->pnote; break;
			case HRW_LASTPCDATA:	data=(void*)ch->pcdata; break;
			case HRW_LASTCONNECTION:data=(void*)ch->desc; break;
			default: data=NULL; break;
		}
		if(!data){
			continue;
		}
		if(IS_SET(hotreboot_field_table[i].flags, HRF_SEND_AS_CHARFIELD)){
			hotreboot_send_command("field %s", 				
				hotreboot_get_send_format(data,"", i));
		}
	}; 
	return;
};

/**************************************************************************/
// sends a player over the connection
void hotreboot_send_characters(char_data *ch)
{
	// use recursion to maintain the order of the players
	if(ch->next_player){
		hotreboot_send_characters(ch->next_player);
	}

	save_char_obj(ch);
	if(ch->desc)
	{
		direct_to_char(ch,"\r\nTime seems to stop for a moment...\r\n");

		direct_to_char(ch,"\r\n\r\nYou feel ");

		hotreboot_send_command("character '%s' %d %d %s '%s'", 
			ch->name, 
			ch->desc->connected_socket,
			ch->desc->mccp_version,
			ch->desc->protocol== PROTOCOL_IPV6?"ipv6":"ipv4",
			CONTYPE_table[ch->desc->contype].name); 
		hotreboot_send_charfields(ch);
	}

}
/**************************************************************************/
bool ftp_reconnect(char *name);
/**************************************************************************/
void hotreboot_transfer_dawnftp_clients_in_pushstate()
{
	for (connection_data *d = connection_list; d; d=c_next)
	{
		if(d->connected_state == CON_FTP_COMMAND && d->ftp.mode == FTP_PUSH){
			ftp_reconnect(d->username);
		}
	}
}

/**************************************************************************/
void hotreboot_send_environment()
{
#ifdef DEBUG_SHOW_HOTREBOOT_INFO
	for ( char_data *wch = player_list; wch; wch = wch->next_player )
	{
		if(IS_IMMORTAL(wch)){	
			wch->println("Hotreboot environment transfer requested from child process.");
		}
	}
#endif 

	hotreboot_transfer_dawnftp_clients_in_pushstate();

	hotreboot_disconnect_non_CON_PLAYING_characters();

	if(player_list){
		hotreboot_send_characters(player_list);
	}


	hotreboot_send_command("environment_transfer_completed");
	// hotreboot_wait_for_shutdown_confirm() function should really
	// interact with the alarm system, so the alarm can cancel
	// the blocking here if it triggers, instead of aborting 
	// the parent mud 
	hotreboot_wait_for_shutdown_confirm();

	// if we get to here, the child failed to accept the hotreboot
	hotreboot_ipc_pipe=-1;
	hotreboot_in_progress=false;
}
/**************************************************************************/
// processes parent side progress of a hotreboot
void hotreboot_process_parent_side_progress()
{
	if(!hotreboot_in_progress){
		bug("process_hotreboot_parent_side_progress() called and there is no hotreboot running!");
		return;
	}

	char incoming_command[MSL];

	char command[MSL];
	char *input;
	char *argument;
	int receive_count;

	for(;;){
		// get the next input line and the command type from the front
		argument=incoming_command;
		input=hotreboot_receive_command(&receive_count);
		// check for failed hotreboot
		if(receive_count<1){
			for ( char_data *wch = player_list; wch; wch = wch->next_player )
			{
				if(IS_IMMORTAL(wch)){	
					wch->println("Hotreboot failed!");
					wch->println("`R=================`YHotreboot failed!");
					wch->println("Hotreboot failed!");
					wch->println("`R=================`YHotreboot failed!");
					wch->println("Hotreboot failed!");
					wch->println("`R=================`YHotreboot failed!");
					wch->println("Hotreboot failed!");
				}
			}
			// child process closed down for some reason - hotreboot canceled
			hotreboot_ipc_pipe=-1;
			hotreboot_in_progress=false;
			dawnlog_write_index("hotreboot failed!");
			return;
		};
		strcpy(incoming_command,input);

		argument=one_argument(argument, command);

		if(!str_cmp(command, "begin_environment_transfer")){
			hotreboot_send_environment();
		}else{
			bugf("hotreboot_process_parent_side_progress(): Unknown command '%s' received in '%s'!!!",
				command, incoming_command);
		}
	}
}
/**************************************************************************/
// sends listen_on configuration
void hotreboot_send_listen_on_entry(listen_on_type *listen_on_entry)
{
	if(listen_on_entry->listening){
		// ensure the protocol is either ipv6 or ipv4
		if(listen_on_entry->protocol!= PROTOCOL_IPV6 && 
			listen_on_entry->protocol!= PROTOCOL_IPV4){
			bugf("hotreboot_send_listen_on_entry(): listen_on_entry->protocol==%d",
				(int)listen_on_entry->protocol);
			do_abort();
		}
		hotreboot_send_command("listenon %d %s '%s' "
			"'%s' '%s' %d", // 'psz_bound_pair', 'psz_bind_address', bind_port
			listen_on_entry->listening_socket,
			listen_on_entry->protocol== PROTOCOL_IPV6?"ipv6":"ipv4",
			CONTYPE_table[listen_on_entry->contype].name,
			listen_on_entry->psz_bound_pair,
			listen_on_entry->psz_bind_address,
			listen_on_entry->bind_port);
	}
	if(listen_on_entry->next){
		hotreboot_send_listen_on_entry(listen_on_entry->next);
	}
}
/**************************************************************************/
// sends listen_on configuration
void hotreboot_send_listen_on_list(listen_on_type *listen_on_entry)
{
	hotreboot_send_listen_on_entry(listen_on_first);
}

/**************************************************************************/
extern bool listen_on_source_text_set;
/**************************************************************************/
bool hotreboot_GEL_listen_on(char *argument)
{
	listen_on_type *node;
	char sz_listening_socket[MSL];
	char sz_protocol[MIL];
	PROTOCOL_TYPE protocol;
	char sz_contype[MSL];	
	CONNECTION_TYPE contype;
	char sz_bound_pair[MSL];	
	char sz_bind_address[MSL];	
	char sz_bind_port[MSL];	

	// get the socket number, address and port
	argument=one_argument(argument, sz_listening_socket);
	argument=one_argument(argument, sz_protocol);
	argument=one_argument(argument, sz_contype);
	argument=one_argument(argument, sz_bound_pair);
	argument=one_argument(argument, sz_bind_address);
	argument=one_argument(argument, sz_bind_port);

	// check validity of bind_socket value
	if(!is_number(sz_listening_socket)){
		bugf("hotreboot_GEL_listenon(): listening_socket is not an integer '%s'",
			sz_listening_socket);
		return false;
	}
	// check that the protocol is either ipv6 or ipv4
	if(!str_cmp(sz_protocol, "ipv6")){
		protocol=PROTOCOL_IPV6;
	}else if(!str_cmp(sz_protocol, "ipv4")){
		protocol=PROTOCOL_IPV4;
	}else{
		bugf("hotreboot_GEL_listenon(): protocol '%s' is not a recognised value for socket %s",
			sz_protocol, sz_listening_socket);
		return false;
	}
	// check the contype
	contype=loparse_connection_type_lookup(sz_contype);
	if(contype==CONTYPE_UNSET){
		bugf("hotreboot_GEL_listenon(): '%s' is not a recognised connection type for socket %s",
			sz_contype, sz_listening_socket);
		return false;
	}
	
	// allocate the listen_on entry node
	node=new listen_on_type;
	memset(node, 0, sizeof(listen_on_type));
	// the values received from the parent mud
	node->listening_socket=atoi(sz_listening_socket);
	node->protocol=protocol;
	node->contype=contype;
	node->listening=true;

	if(node->protocol==PROTOCOL_IPV4){
		listening_on_ipv4_addresses=true;
	}else if(node->protocol==PROTOCOL_IPV6){
		listening_on_ipv6_addresses=true;
	}

	node->psz_bound_pair=str_dup(sz_bound_pair);
	node->psz_bind_address=str_dup(sz_bind_address);
	node->bind_port=atoi(sz_bind_port);

	node->listening_exception_count=0; // reset this, as it is not important to transfer it

	// unused fields which aren't used once the mud has 
	// binded its ports during the initial bootup
	// - node->parsed_port_offset=
	// - node->parsed_port=

	node->next=NULL;

	// append the new node to the linked list of listen_on bindings
	if(listen_on_last){
		listen_on_last->next=node;
		listen_on_last=node;
	}else{
		listen_on_first=node;
		listen_on_last=node;
	}

	// prevent netio overriding what we have received
	listen_on_source_text_set=true;

	netio_binded_listen_on_output();
	return true;
}
/**************************************************************************/
// check if the resolver connection has input
void hotreboot_poll_and_process()
{
	static int errcount=0;
	static struct timeval null_time;
	static fd_set hotreboot_ipc_in;
	static fd_set hotreboot_ipc_exception;

	FD_ZERO( &hotreboot_ipc_in);	
	FD_ZERO( &hotreboot_ipc_exception);

	FD_SET( (dawn_socket)hotreboot_ipc_pipe, &hotreboot_ipc_in);
	FD_SET( (dawn_socket)hotreboot_ipc_pipe, &hotreboot_ipc_exception);

	if( select( hotreboot_ipc_pipe+1, &hotreboot_ipc_in, NULL, &hotreboot_ipc_exception, &null_time ) < 0 ){
		socket_error( FORMATF("hotreboot_poll_and_process: select()."));
		do_abort();
		exit_error( 1 , "hotreboot_poll_and_process", "select error");
	}


	if( FD_ISSET( hotreboot_ipc_pipe, &hotreboot_ipc_exception) )
	{
		errcount++;
		logf("Exception raised when reading from hotreboot ipc pipe.");
		if(errcount>50){
			logf("Too many errors encounted, closing hotreboot ipc pipe.");
			hotreboot_ipc_pipe=-1;
			hotreboot_in_progress=false;
		}
		for ( char_data *wch = player_list; wch; wch = wch->next_player )
		{
			if(IS_IMMORTAL(wch)){	
				wch->println("Hotreboot failed - exception on hotreboot_ipc_pipe!");
			}
		}
		return;
	}

	if( FD_ISSET( hotreboot_ipc_pipe, &hotreboot_ipc_in) )
	{		
		hotreboot_process_parent_side_progress();
	}

}
/**************************************************************************/
/**************************************************************************/

