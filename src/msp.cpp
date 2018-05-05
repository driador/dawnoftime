/**************************************************************************/
// msp.cpp - msp support, Kerenos and Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "msp.h"


struct msp_table_type{ // the table using this type is sorted by MSP_TYPE
	char *base_url;			// the base url of filename (including directory)
	char *prefix_filename;	// directory it is stored in
	char *type;				// the category of sound - players can turn off types
	int default_prority;	// default priority of a sound
	int target_extra_prority;// extra priority of a sound when directed at a target
	int default_volume;		// default volume of a sound to all in room
	int target_extra_volume;// extra volume of a sound to some it is targeted at
	
};

msp_table_type msp_table[MSPT_MAX_TYPE];

struct msp_load_table_type
{
	MSP_TYPES type_index;
	const char *directory; // used to create the base_url, prefix_filename and type
	int default_prority;	// default priority of a sound
	int target_extra_prority;// extra priority of a sound when directed at a target
	int default_volume;		// default volume of a sound to all in room
	int target_extra_volume;// extra volume of a sound to some it is targeted at
};

msp_load_table_type msp_load_table_data[]=
{									  // prorities , volumes
	{	MSPT_ACTION,	MSP_ACTION_DIR,		40,	10,	60,	15	},
	{	MSPT_COMBAT,	MSP_COMBAT_DIR,		65,	20,	60,	35	},
	{	MSPT_MUDPROG,	MSP_MUDPROG_DIR,	35,	40,	50,	30	},
	{	MSPT_ROOM,		MSP_ROOM_DIR,		45,	 0,	70,	 0	},
	{	MSPT_SKILL,		MSP_SKILLS_DIR,		60,	10,	50,	35	},
	{	MSPT_SPELL,		MSP_SPELLS_DIR,		60,	10,	50,	35	},
	{	MSPT_WEATHER,	MSP_WEATHER_DIR,	25,	 0,	60,	 0	},
	{	MSPT_MAX_TYPE,	"", 0, 0, 0, 0 }// end of table marker
};


/**************************************************************************/
char * get_msptype_name(const char *word)
{
	static char rbuf[3][MIL];
	static int index;
	char *pStr;

	++index%=3; // rotate return buffer

	sprintf(rbuf[index], "%s", word);

	pStr=&rbuf[index][0];

	// convert spaces into _ characters
	// and upper to lowercase
	do{
		if (*pStr==' '){
			*pStr='_';
		}else if (*pStr=='/'){
			*pStr='_';
		}else{
			*pStr=tolower(*pStr);
		}
	}while (*(++pStr));

	// trim off any tailing _
	if(*(pStr-1)=='_'){
		*(pStr-1)='\0';
	}	
	return rbuf[index];
}
/**************************************************************************/
// change any '\' into a '/'   
void backslash_to_slash(char *word)
{
	char *pStr=word;
	do{
		if (*pStr=='\\'){
			*pStr='/';
		}
	}while (*(++pStr));
}
/**************************************************************************/
// Kal - Dec 99
void msp_load_table()
{
	#define data_table  msp_load_table_data[i]
	#define tab_element msp_table[msp_load_table_data[i].type_index]
	int i;
	for (i=0; !IS_NULLSTR(data_table.directory); i++){
		tab_element.base_url		= str_dup(MSP_URL);
		tab_element.prefix_filename	= str_dup(data_table.directory);
		tab_element.type			= str_dup(get_msptype_name(data_table.directory));
		// the prorities and volumes
		tab_element.default_prority		= data_table.default_prority;
		tab_element.target_extra_prority= data_table.target_extra_prority;
		tab_element.default_volume		= data_table.default_volume;
		tab_element.target_extra_volume	= data_table.target_extra_volume;
	}
	#undef tab_element
	#undef data_table
	logf("msp_load_table(): msp_table loaded with msp type info.");
}
/**************************************************************************/
float get_yellreduction(short sector);
/**************************************************************************/
// Sends an msp sound to everyone in the room - Kal, Dec 99
// NOTES:
// * if the volume is zero, the default volume is used for the sound type
// * repeats = number of times to play the sound
// * If surrounding_rooms is true, you can hear the sound in the 
//   surrounding rooms (go figure)
// * If the extension of the sound filename is .mid then !!MUSIC is used
void msp_to_room(MSP_TYPES type, const char *filename, 
					int volume, // 1 ->100, if 0 default for type vol used 
					char_data *target,
					bool target_only_in_room,
					bool surrounding_rooms)
{
	if(IS_NULLSTR(filename)){
		return;
	}

	char msp_trigger_format_buf[MSL];
	char_data *to;
	// handle the loading of the msp_table on its first use
	static bool msp_table_needs_loading=true;
	if(msp_table_needs_loading){
		msp_load_table();
		msp_table_needs_loading=false;
	}

	// get the filename of what we are sending, check the file is on the server
	char on_disk_filename[MSL];
	strncpy(on_disk_filename, filename, MSL-1);
	on_disk_filename[MSL-1]='\0';

	char send_filename[MSL];
	// support system if last character in the on_disk_filename before the extension is a digit 
	// the sound will be randomized, with filename0.wav->filename#.wav where # is
	// the value of that digit
	int len=str_len(on_disk_filename);
	if(	len>5 
		&& is_digit(on_disk_filename[len-5])
		&& !str_suffix(".wav", on_disk_filename) 
		)
	{
		// get a random sound file - if not found drop back to '*0.wav'
		on_disk_filename[len-5]=(char)number_range('0', on_disk_filename[len-5]);

		if(GAMESETTING(GAMESET_MSP_CHECK_FILEEXIST)){
			if(!file_existsf(MSP_DIR "%s%s", msp_table[type].prefix_filename, on_disk_filename)){
				on_disk_filename[len-5]=(char)'0';
			}
		}
	}
	sprintf(send_filename, "%s%s", msp_table[type].prefix_filename, on_disk_filename);
	if(GAMESETTING(GAMESET_MSP_CHECK_FILEEXIST)){
		if(!file_existsf(MSP_DIR "%s", send_filename)){
			bugf("msp_to_room(): couldn't find sound to send - send_filename = '%s'",
				send_filename);
			return;
		};
	}

	// check our target and its room is valid 
	if(target==NULL || target->in_room==NULL){
		bugf("msp_to_room(): target==NULL || target->in_room==NULL! - send_filename = 'msp "DIR_SYM"%s'",
			send_filename);
		return;
	}

	// get the default volume
	if(volume==0){
		volume=msp_table[type].default_volume;
	}
	if(volume>100){
		volume=100;
	}

	//construct the unchanging parts of the msp trigger
	if(!str_suffix( ".mid", send_filename)){
		sprintf(msp_trigger_format_buf,
			"!!MUSIC(%s V=%%d L=1 T=%s", send_filename, msp_table[type].type);
	}else{
		sprintf(msp_trigger_format_buf, "!!SOUND(%s V=%%d L=1 P=%%d T=%s",
				send_filename, msp_table[type].type);
	}

	// if the url is sufficient length, has been changed from the default, we use it.
	if(!IS_NULLSTR(msp_table[type].base_url) 
		&& str_len(msp_table[type].base_url)>5
		&& strcmp(DEFAULT_MSP_URL,msp_table[type].base_url))
	{
		strcat(msp_trigger_format_buf, FORMATF(" U=%s%s", msp_table[type].base_url, send_filename));
	}
	strcat(msp_trigger_format_buf,")");

	// convert any backslashes to slashes
	backslash_to_slash(msp_trigger_format_buf);

	// send to the target
	if(CAN_HEAR_MSP(target))
	{
		flush_char_outbuffer(target);
		target->printf(msp_trigger_format_buf,
			((volume+msp_table[type].target_extra_volume)>100?
				100:(volume+msp_table[type].target_extra_volume)),
			(msp_table[type].default_prority+msp_table[type].target_extra_prority));
	}

	if(!target_only_in_room){
		// send to all in room except target
		for(to = target->in_room->people; to; to=to->next_in_room ){
			if (to!=target && CAN_HEAR_MSP( to )){
				flush_char_outbuffer(to);
				to->printf(msp_trigger_format_buf,
					volume,
					msp_table[type].default_prority);
			}
		}
	}

	if(!surrounding_rooms){
		return;
	}
	
	// do the sending to all surrounding rooms
    ROOM_INDEX_DATA *from_room=target->in_room;		
    for ( int door = 0; door < MAX_DIR; door++ )
    {
		EXIT_DATA       *pexit;
		int main_volume=volume;
		
		if ( ( pexit = from_room->exit[door] ) != NULL
			&&   pexit->u1.to_room != NULL
			&&   pexit->u1.to_room != from_room)
		{
			// get the new volume - same volume reduction as yelling 
			// - reduction should technically be logarithmic, but life goes on :)
			volume=(int)((float)main_volume * get_yellreduction(pexit->u1.to_room->sector_type));
			if(IS_SET(from_room->exit[door]->exit_info,EX_CLOSED)){
				volume=main_volume/2; // closed doors make things quieter
			}

			// send to all in room except target
			for(to = pexit->u1.to_room->people; to; to=to->next_in_room ){
				if (CAN_HEAR_MSP( to )){
					flush_char_outbuffer(to);
					to->printf(msp_trigger_format_buf,
						volume,
						msp_table[type].default_prority/2);
				}
			}
		}
    }    
}
/**************************************************************************/
// plays a skills sound if it has one set in sedit
void msp_skill_sound(char_data *target, int sn)
{	
	if(!IS_NULLSTR(skill_table[sn].msp_sound)){
		msp_to_room(MSPT_SKILL, skill_table[sn].msp_sound, 
				0,target,	false, true);
	}
}
/**************************************************************************/
void do_msp( char_data *ch, char *argument )
{
	pc_data *pcdata=TRUE_CH(ch)->pcdata; // the characters pcdata we want to work on
	if(!pcdata){
		ch->println("Players can only use this command");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->titlebar("MUD SOUND PROTOCOL OPTIONS");
		ch->println("syntax: `=Cmsp off`x  - msp is permanately off.");
		ch->wrapln("syntax: `=Cmsp auto`x - mud will attempt to automatically "
					"detect if you have mud client that supports msp, if one is detected "
					"the mud will send you msp sound triggers");
		ch->wrapln("syntax: `=Cmsp on`x  - msp is permanately on, mud will send you sound "
			"triggers even if your mud client doesn't support msp.");
		ch->printlnf("Your msp preference is currently set to %s", 
			preference_word(pcdata->preference_msp));
		if(pcdata->preference_msp==PREF_AUTOSENSE){
			ch->printlnf("Your connections msp support has %sbeen automatically detected.",
				(ch->desc && IS_SET(ch->desc->flags, CONNECTFLAG_MSP_DETECTED))?"":"not ");
		}
		ch->titlebar("");
		return;
	}

	PREFERENCE_TYPE pt;
	if(!str_prefix(argument, "off")){
		pt=PREF_OFF;
	}else if(!str_prefix(argument, "autosense")){
		pt=PREF_AUTOSENSE;
	}else if(!str_prefix(argument, "on")){
		pt=PREF_ON;
	}else{
		ch->printlnf("Unsupported msp option '%s'", argument);
		do_msp(ch,"");
		return;
	}
	if(pcdata->preference_msp==pt){
		ch->printlnf("Your msp preference is already set to %s", preference_word(pt));
		return;
	}

	ch->printlnf("msp preference changed from %s to %s", 
		preference_word(pcdata->preference_msp),
		preference_word(pt));
	pcdata->preference_msp=pt;

	msp_update_char(ch);

}
/**************************************************************************/
void msp_update_char(char_data*ch)
{
	if(!ch){
		return;
	}
	pc_data *pcdata=TRUE_CH(ch)->pcdata;
	connection_data *d=TRUE_CH(ch)->desc;
	if(!pcdata || !d){
		return;
	}

	switch(pcdata->preference_msp){
		case PREF_OFF:	
			pcdata->msp_enabled=false;
			break;
		case PREF_AUTOSENSE: 
			if(IS_SET(d->flags,CONNECTFLAG_ANSWERED_MSP)
				&& IS_SET(d->flags,CONNECTFLAG_MSP_DETECTED)){
				pcdata->msp_enabled=true;
			}else{
				pcdata->msp_enabled=false;
			}
			break;
		case PREF_ON: 
			pcdata->msp_enabled=true;
			break;
	}
}
/**************************************************************************/


