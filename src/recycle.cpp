/**************************************************************************/
// recycle.cpp - memory management systems
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
#include "nanny.h"
#include "track.h"
#include "channels.h"
#include "events.h"

/**************************************************************************/	
// note recycling
NOTE_DATA *note_free;
/**************************************************************************/	
NOTE_DATA *new_note()
{
    NOTE_DATA *note;

    if (note_free == NULL){
		note = (NOTE_DATA *)alloc_perm(sizeof(*note));
    }else{ 
		note = note_free;
		note_free = note_free->next;
    }
	memset(note,0,sizeof(*note));// initialise memory 
    note->text		=str_dup("");
    note->subject	=str_dup("");
    note->to_list	=str_dup("");
	note->cc_list	=str_dup("");
    note->date		=str_dup("");
    note->sender	=str_dup("");
    VALIDATE(note);
    return note;
}
/**************************************************************************/	
void free_note(NOTE_DATA *note)
{
    if (!IS_VALID(note))
	return;

    free_string( note->text    );
    free_string( note->subject );
    free_string( note->to_list );
	free_string( note->cc_list );
    free_string( note->date    );
    free_string( note->sender  );
    INVALIDATE(note);

    note->next = note_free;
    note_free   = note;
}
/**************************************************************************/	 
// recycling ban structures 
BAN_DATA *ban_free;
/**************************************************************************/
BAN_DATA *new_ban(void)
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;
	
    if (ban_free == NULL){
		ban = (BAN_DATA *)alloc_perm(sizeof(*ban));
    }else{
		ban = ban_free;
		ban_free = ban_free->next;
    }
	
    *ban = ban_zero;
    VALIDATE(ban);
    
    ban->intended_people	= &str_empty[0];
	ban->sitemasks			= &str_empty[0];
    ban->ban_date			= current_time;
    ban->reason				= &str_empty[0];
    ban->by					= &str_empty[0];
	ban->expire_date		= 0; // never expire by default
	ban->custom_disconnect_message	=	&str_empty[0];
	ban->allowed_email_masks		=	&str_empty[0];
	ban->disallowed_email_masks		=	&str_empty[0];
	ban->disallowed_email_custom_message=	&str_empty[0];
    return ban;
}
/**************************************************************************/
void free_ban(BAN_DATA *ban)
{
    if (!IS_VALID(ban))
	return;

    free_string(ban->intended_people	);
	free_string(ban->sitemasks			);
    free_string(ban->reason				);
    free_string(ban->by					);
	free_string(ban->custom_disconnect_message	);
	free_string(ban->allowed_email_masks		);
	free_string(ban->disallowed_email_masks		);
	free_string(ban->disallowed_email_custom_message);

    INVALIDATE(ban);

    ban->next = ban_free;
    ban_free = ban;
}
/**************************************************************************/
// connection recycling  
connection_data *connection_allocate()
{
    static connection_data c_zero;
    connection_data *c;

    if(!connection_free){
		c = (connection_data *)alloc_perm(sizeof(*c));

		// set the initial default string values
		c->ident_raw_result=str_dup("");
		c->ident_username=str_dup("");
		c->mxp_version=str_dup("");
		c->mxp_supports=str_dup("");
		c->mxp_options=str_dup("");
		c->terminal_type=str_dup("");

		c->protocol=PROTOCOL_ALL;
		c->contype=CONTYPE_UNSET;
		c->local_ip=str_dup("");
		c->local_port=0;
		c->local_tcp_pair=str_dup("");
		c->remote_ip=str_dup("");
		c->remote_port=0;
		c->remote_tcp_pair=str_dup("");
		c->remote_hostname=str_dup("");
		c->web_request=NULL;		
    }else{
		c = connection_free;
		connection_free = connection_free->next;
    }
	
    *c = c_zero;
    VALIDATE(c);

    c->connected_state   = CON_DETECT_CLIENT_SETTINGS;
	c->connected_state_pulse_counter=0;
    c->showstr_head    = NULL;
    c->showstr_point = NULL;
    c->outsize = 2000;
    c->outbuf  = (char *) alloc_mem( c->outsize );
	c->idle_since = current_time; // initialise idle timer to now
	c->resolved=false;
	c->changed_flag=NULL;
	c->multiple_logins=false;
	c->wrong_password_count=0;
	c->parse_colour=true;
	c->pEdit    = NULL;
	c->pString  = NULL;
	c->editor   = 0;		
	c->colour_mode = CT_AUTODETECT;

	// visual debugging default settings
	c->visual_debugging_enabled=false;
	c->visual_debug_hexoutput=true;
	c->visual_debug_flush_before_prompt=true;
	c->visual_debug_strip_prompt=true;
	c->visual_debug_column_width=25;

	return c;
}
/**************************************************************************/
void connection_deallocate(connection_data *c)
{
    if (!IS_VALID(c)){
		bugf("connection_deallocate(): called to deallocate invalid connetion.");
		return;
	}

	// reset to the defaults
	replace_string(c->ident_raw_result, "");
	replace_string(c->ident_username, "");
	replace_string(c->mxp_version, "");
	replace_string(c->mxp_supports, "");
	replace_string(c->mxp_options, "");
	replace_string(c->terminal_type, "");

	c->protocol=PROTOCOL_ALL;
	c->contype=CONTYPE_UNSET;
	replace_string(c->local_ip,"");
	c->local_port=0;
	replace_string(c->local_tcp_pair,"");
	replace_string(c->remote_ip,"");
	c->remote_port=0;
	replace_string(c->remote_tcp_pair,"");
	replace_string(c->remote_hostname,"");

	delete c->web_request;
	c->web_request=NULL;
    free_mem( c->outbuf, c->outsize );
    INVALIDATE(c);
    c->next = connection_free;
    connection_free = c;
}
/**************************************************************************/
/* stuff for recycling gen_data */
GEN_DATA *gen_data_free;
/**************************************************************************/
GEN_DATA *new_gen_data(void)
{
    static GEN_DATA gen_zero;
    GEN_DATA *gen;

    if (gen_data_free == NULL)
	gen = (GEN_DATA *)alloc_perm(sizeof(*gen));
    else
    {
	gen = gen_data_free;
	gen_data_free = gen_data_free->next;
    }
    *gen = gen_zero;
    VALIDATE(gen);
    return gen;
}
/**************************************************************************/
void free_gen_data(GEN_DATA *gen)
{
	if (!IS_VALID(gen))
		return;

	INVALIDATE(gen);

	gen->next = gen_data_free;
	gen_data_free = gen;
} 
/**************************************************************************/
/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;
/**************************************************************************/
EXTRA_DESCR_DATA *new_extra_descr(void)
{
	EXTRA_DESCR_DATA *ed;

	if (extra_descr_free == NULL)
		ed = (EXTRA_DESCR_DATA *)alloc_perm(sizeof(*ed));
	else
	{
		ed = extra_descr_free;
		extra_descr_free = extra_descr_free->next;
	}

	ed->keyword = &str_empty[0];
	ed->description = &str_empty[0];
	VALIDATE(ed);
	return ed;
}
/**************************************************************************/
void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
	if (!IS_VALID(ed))
		return;

	free_string(ed->keyword);
	free_string(ed->description);
	INVALIDATE(ed);

	ed->next = extra_descr_free;
	extra_descr_free = ed;
}
/**************************************************************************/
/* stuff for recycling affects */
AFFECT_DATA *affect_free;
/**************************************************************************/
AFFECT_DATA *new_affect(void)
{
    static AFFECT_DATA af_zero;
    AFFECT_DATA *af;

    if (affect_free == NULL)
	af = (AFFECT_DATA *)alloc_perm(sizeof(*af));
    else
    {
	af = affect_free;
	affect_free = affect_free->next;
    }

    *af = af_zero;


    VALIDATE(af);
    return af;
}
/**************************************************************************/
void free_affect(AFFECT_DATA *af)
{
    if (!IS_VALID(af))
	return;

    INVALIDATE(af);
    af->next = affect_free;
    affect_free = af;
}
/**************************************************************************/
/* stuff for recycling objects */
OBJ_DATA *obj_free;
/**************************************************************************/
OBJ_DATA *new_obj(void)
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;

    if (obj_free == NULL){
		obj = (OBJ_DATA *)alloc_perm(sizeof(*obj));
    }else{
		obj = obj_free;
		obj_free = obj_free->next;
    }
    //*obj		= obj_zero;
	memcpy(obj, &obj_zero, sizeof(*obj));

    obj->name		=str_dup("");
    obj->description=str_dup("");
    obj->short_descr=str_dup("");
    obj->owner		=str_dup("");
    obj->killer		=str_dup("");
	obj->lastdrop_remote_ip=str_dup("");

	obj->uid	= get_next_uid();
	obj->events=NULL;
	obj->events_count=0;
    VALIDATE(obj);
    return obj;
}
/**************************************************************************/
void free_obj(OBJ_DATA *obj)
{
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ec_next;
	
    if (!IS_VALID(obj))
		return;
	
    for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
		paf_next = paf->next;
		free_affect(paf);
    }
    obj->affected = NULL;
	
    for (ed = obj->extra_descr; ed != NULL; ed = ec_next )
    {
		ec_next = ed->next;
		free_extra_descr(ed);
	}
	obj->extra_descr = NULL;
	
    free_string( obj->name        );
    free_string( obj->description );
    free_string( obj->short_descr );
    free_string( obj->owner );
    free_string( obj->killer );
	free_string( obj->lastdrop_remote_ip);
	if(obj->events){
		bugf("dequeuing events on object %d in free_obj()!!!", obj->vnum());
		obj->mudqueue_dequeue_all(EVENTGROUP_PURGE_EVERY_EVENT_INCLUDING_SYSTEM_EVENTS);
	}
    INVALIDATE(obj);
	
    obj->next   = obj_free;
    obj_free    = obj; 
}

/**************************************************************************/
/* stuff for recyling characters */
char_data *char_free;
/**************************************************************************/
char_data *new_char (void)
{
    static char_data ch_zero;
    char_data *ch;
    int i;

    if(char_free == NULL){
		ch = (char_data*)alloc_perm(sizeof(*ch));
    }else{
		ch = char_free;
		char_free = char_free->next;
	}
	memcpy(ch, &ch_zero, sizeof(*ch));

	ch->uid			= get_next_uid();
	ch->running_mudprog_for_object=NULL; 
    VALIDATE(ch);
    ch->name                    = &str_empty[0];
    ch->short_descr             = &str_empty[0];
    ch->long_descr              = &str_empty[0];
    ch->description             = &str_empty[0];
	ch->gprompt                 = &str_empty[0];
    ch->prompt                  = &str_empty[0];
    ch->olcprompt               = &str_empty[0];
    ch->prefix					= &str_empty[0];
	ch->remote_ip_copy			= &str_empty[0];

    ch->logon                   = current_time;
    ch->lines                   = PAGELEN;
	ch->level					=0;
	ch->trust					=0;
    for(i = 0; i < 4; i++){
        ch->armor[i]            = 100;
	}
    ch->position                = POS_STANDING;
    ch->hit                     = 20;
    ch->max_hit                 = 20;
    ch->mana                    = 100;
    ch->max_mana                = 100;
    ch->move                    = 100;
    ch->max_move                = 100;

    ch->subversion				= 2;
    ch->last_ic_room            = NULL;
    ch->mounted_on				= NULL;
    ch->ridden_by				= NULL;
    ch->no_xp                   = false;
    ch->subdued                 = false;
    ch->last_force              = -20;  // hasn't been forced
    ch->controlling             = NULL; // not switched into anyone
	ch->colour_prefix			= COLOURCODE;

	// setup the default colours
	ch->saycolour				= 'x';
	ch->motecolour				= 'S';
	
    ch->tethered=false;
    ch->bucking=false;


	ch->config=0;
	ch->desc=NULL;
//	#define TRUE_CH(ch)(ch->desc ? (ch->desc->original ? ch->desc->original : SAFE_DESC_CHARACTER(ch)):ch) 
//	SET_BIT(TRUE_CH(ch)->config, CONFIG_SHOWMISC);
 
//	SET_BIT(
//	ch->desc ? 
//		(ch->desc->original ? ch->desc->original->config : SAFE_DESC_CHARACTER(ch)->config):ch->config
//
//	, CONFIG_SHOWMISC);

	SET_CONFIG(ch, CONFIG_SHOWMISC);
	SET_CONFIG(ch, CONFIG_AUTORECALL);
	SET_CONFIG(ch, CONFIG_AUTOLANDONREST);
	SET_BIT(ch->comm,COMM_AUTOSELF);	

	for (i = 0; i < MAX_STATS; i ++){
        ch->perm_stats[i] = 1;
        ch->modifiers[i] = 0;
    }
	
	assert(track_table!=NULL);
	ch->track_index = track_table->add_char(ch);

	ch->pload=NULL;
	ch->events=NULL;
	ch->events_count=0;
	ch->previous_room_type=PREVIOUS_ROOM_TYPE_UNKNOWN;

    return ch;
}

/**************************************************************************/
void free_char (char_data *ch)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    if(!IS_VALID(ch)){
		return;
	}

	// remove them from the track table sooner than later
	// so the table can record any info about
	// the char before it is freed.
	track_table->del_char(ch);

	// if the character has set an alarm during the time they were logged in 
	// loop through all the rooms in the entire game, any room that has an alarm set to this character, clear it
	if(IS_SET(ch->dyn, DYN_USER_OF_ALARM_SPELL)){
		ROOM_INDEX_DATA *pAlarmRoom;
		for ( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
		{
			for ( pAlarmRoom = room_index_hash[iHash];pAlarmRoom;pAlarmRoom= pAlarmRoom->next )
			{
				if(pAlarmRoom->alarm == ch){
					REMOVE_BIT( ch->in_room->affected_by, ROOMAFF_ALARM );
					pAlarmRoom->alarm =NULL;
				}
			}
		}
	}

    if(IS_NPC(ch)){
		mobile_count--;
	}

    for (obj = ch->carrying; obj != NULL; obj = obj_next){
		obj_next = obj->next_content;
		extract_obj(obj);
    }

    for (paf = ch->affected; paf != NULL; paf = paf_next){
		paf_next = paf->next;
		affect_remove(ch,paf);
    }

    free_string(ch->name);
    free_string(ch->short_descr);
    free_string(ch->long_descr);
    free_string(ch->description);
    free_string(ch->prompt);
    free_string(ch->prefix);
    free_note(ch->pnote);
   	free_pcdata(ch->pcdata);

	if(ch->events){
		bugf("dequeuing events on character %d in free_char()!!!", ch->vnum());
		ch->mudqueue_dequeue_all(EVENTGROUP_PURGE_EVERY_EVENT_INCLUDING_SYSTEM_EVENTS);
	}

    ch->next = char_free;
    char_free  = ch;	

    INVALIDATE(ch);
	
    return;
}
/**************************************************************************/
PC_DATA *pcdata_free;
/**************************************************************************/
PC_DATA *new_pcdata(void)
{
    int alias;
	int i;

    static PC_DATA pcdata_zero;
    PC_DATA *pcdata;

    if (pcdata_free == NULL)
        pcdata = (PC_DATA *)alloc_perm(sizeof(*pcdata));
    else
    {
        pcdata = pcdata_free;
        pcdata_free = pcdata_free->next;
    }

    *pcdata = pcdata_zero;

    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
        pcdata->alias[alias] = NULL;
        pcdata->alias_sub[alias] = NULL;
    }

    pcdata->buffer = new_buf();

    pcdata->karns = 3;              // default karns
    pcdata->next_karn_countdown = (pcdata->karns*700)+200; 
   
	pcdata->emote_index=-1;
	pcdata->say_index=-1;
	pcdata->last_logout_site=&str_empty[0];
	pcdata->last_logout_time=0;
    pcdata->fadein=&str_empty[0];
    pcdata->fadeout=&str_empty[0];
    pcdata->battlelag=&str_empty[0];

	// for email banning verification
	pcdata->email	=str_dup("");
	pcdata->created_from=str_dup("");
	pcdata->unlock_id=str_dup("");

	pcdata->sublevel=0;
	pcdata->sublevel_trains=0;
	pcdata->sublevel_pracs=0;
	pcdata->birthdate=0;
	pcdata->who_text=&str_empty[0];
	pcdata->title=&str_empty[0];
	pcdata->immtitle=&str_empty[0];
    pcdata->immtalk_name=&str_empty[0];
	pcdata->imm_role=&str_empty[0];
	pcdata->history=&str_empty[0];

	// players default to a security of 1 on olc
	if(GAMESETTING5(GAMESET5_DEDICATED_OLC_BUILDING_MUD)){
		pcdata->security= URANGE(0,(game_settings->default_newbie_security_on_olc_port),9);
	}else{
		pcdata->security=0;
	}

	{	// setup the replaytell buffers	
		for(i=0; i<MAX_REPLAYTELL; i++){
			pcdata->replaytell_text[i]=&str_empty[0];
		}
		pcdata->next_replaytell=0;
	}

	{	// setup the replayroom buffers
		for(i=0; i<MAX_REPLAYROOM; i++){
			pcdata->replayroom_text[i]=&str_empty[0];
		}
		pcdata->next_replayroom=0;
	}

	{	// setup the replaychannels buffers
		for(i=0; i<MAX_REPLAYCHANNELS; i++){
			pcdata->replaychannels_text[i]=&str_empty[0];
		}
		pcdata->next_replaychannels=0;
	}

	for (int index=0; index<RPS_AUDIT_SIZE; index++){
		pcdata->emotecheck[index]=str_dup("");
		pcdata->saycheck[index]=str_dup("");
	}

	pcdata->letter_workspace_text=str_dup("");

	pcdata->preference_msp=PREF_AUTOSENSE;
	pcdata->preference_mxp=PREF_AUTOSENSE;
	pcdata->preference_dawnftp=PREF_AUTOSENSE;
	pcdata->preference_colour_in_socials=PREF_AUTOSENSE; // use the mudwide default

	for(alias=0; alias<MAX_HELP_HISTORY; alias++){
		pcdata->help_history[alias]=str_dup("");
	}

	pcdata->afk_message=str_dup("");
	pcdata->channeloff=CHANNEL_FLAME;
	pcdata->autoafkafter=0;
	pcdata->hero_level_count=0;

    VALIDATE(pcdata);
    return pcdata;
}
/**************************************************************************/	
void free_pcdata(PC_DATA *pcdata)
{
    int alias;
	int index;
	int i;

    if (!IS_VALID(pcdata))
	return;
	
	for (index=0; index<RPS_AUDIT_SIZE; index++){
		free_string(pcdata->emotecheck[index]);
		free_string(pcdata->saycheck[index]);
	}

    free_string(pcdata->pwd);
    free_string(pcdata->bamfin);
    free_string(pcdata->bamfout);
    free_buf(pcdata->buffer);
    
    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
        free_string(pcdata->alias[alias]);
        free_string(pcdata->alias_sub[alias]);
    }
	free_string(pcdata->last_logout_site);
    free_string(pcdata->fadein);
    free_string(pcdata->fadeout);

	// for email banning verification
	free_string(pcdata->email);
	free_string(pcdata->created_from);
	free_string(pcdata->unlock_id);
	free_string(pcdata->who_text);
	free_string(pcdata->title);
	free_string(pcdata->immtitle);

	// free the replaytell stuff
	for(i=0; i<MAX_REPLAYTELL; i++){
		free_string(pcdata->replaytell_text[i]);
	}

	// free the replayroom buffers
	for(i=0; i<MAX_REPLAYROOM; i++){
		free_string(pcdata->replayroom_text[i]);
	}

	// free the replaychannels buffers
	for(i=0; i<MAX_REPLAYCHANNELS; i++){
		free_string(pcdata->replaychannels_text[i]);
	}
	
	// free the help history stuff
	for(i=0; i<MAX_HELP_HISTORY; i++){
		free_string(pcdata->help_history[i]);
	}
	
    INVALIDATE(pcdata);
    pcdata->next = pcdata_free;
    pcdata_free = pcdata;

    return;
}
/**************************************************************************/	
/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
    long val;

    val = ((long)current_time <= last_pc_id) ? last_pc_id + 1 : (long)current_time;
    last_pc_id = val;
    return val;
}

/**************************************************************************/	
MEM_DATA *mem_data_free;
/**************************************************************************/	
// procedures and constants needed for buffering
BUFFER *buf_free;

/**************************************************************************/	
void free_mem_data(MEM_DATA *memory)
{
    if (!IS_VALID(memory))
	return;

    memory->next = mem_data_free;
    mem_data_free = memory;
    INVALIDATE(memory);
}
/**************************************************************************/	
/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
    16,32,64,128,256,1024,2048,4096,8192,16384, 32768, 60000
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
    int i;

    for (i = 0; i < MAX_BUF_LIST; i++)
	{
		if (buf_size[i] >= val)
		{
			return buf_size[i];
		}
	}
    
    return -1;
}
/**************************************************************************/	
BUFFER *new_buf()
{
    BUFFER *buffer;

    if (buf_free == NULL) 
		buffer = (BUFFER *)alloc_perm(sizeof(*buffer));
    else
    {
		buffer = buf_free;
		buf_free = buf_free->next;
    }

    buffer->next	= NULL;
    buffer->state	= BUFFER_SAFE;
    buffer->size	= get_size(BASE_BUF);

    buffer->string	= (char *)alloc_mem(buffer->size);
    buffer->string[0]	= '\0';
    VALIDATE(buffer);

    return buffer;
}

/**************************************************************************/	
void free_buf(BUFFER *buffer)
{
    if (!IS_VALID(buffer))
	return;

    free_mem(buffer->string,buffer->size);
    buffer->string = NULL;
    buffer->size   = 0;
    buffer->state  = BUFFER_FREED;
    INVALIDATE(buffer);

    buffer->next  = buf_free;
    buf_free      = buffer;
}
/**************************************************************************/	
bool add_buf(BUFFER *buffer, const char *string)
{
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
	return false;

    len = str_len(buffer->string) + str_len(string) + 1;

    while (len >= buffer->size) /* increase the buffer size */
    {
		buffer->size 	= get_size(buffer->size + 1);
		{
			if (buffer->size == -1) /* overflow */
			{
				buffer->size = oldsize;
				buffer->state = BUFFER_OVERFLOW;
				bugf("buffer overflow past size %d",buffer->size);
				return false;
			}
  		}
    }

    if (buffer->size != oldsize)
    {
	buffer->string	= (char *)alloc_mem(buffer->size);

	strcpy(buffer->string,oldstr);
	free_mem(oldstr,oldsize);
    }

    strcat(buffer->string,string);
    return true;
}
/**************************************************************************/	

void clear_buf(BUFFER *buffer)
{
    buffer->string[0] = '\0';
    buffer->state     = BUFFER_SAFE;
}
/**************************************************************************/	

char *buf_string(BUFFER *buffer)
{
    return buffer->string;
}

/**************************************************************************/	
// stuff for recycling mudprograms 
MUDPROG_TRIGGER_LIST *mprog_free;
/**************************************************************************/	
MUDPROG_TRIGGER_LIST *new_mprog(void)
{
	static MUDPROG_TRIGGER_LIST mp_zero;
	MUDPROG_TRIGGER_LIST *mp;
	
	if (mprog_free == NULL)
		mp = (MUDPROG_TRIGGER_LIST *)alloc_perm(sizeof(*mp));
	else
	{
		mp = mprog_free;
		mprog_free=mprog_free->next;
	}
	
	*mp = mp_zero;
	mp->trig_type	= 0;
	mp->trig2_type	= 0;
	mp->prog		= NULL;
	return mp;
}
/**************************************************************************/	
void free_mprogs(MUDPROG_TRIGGER_LIST *mp)
{
	if(!mp){
		return;
	}

	if(mp->next){
		free_mprogs(mp->next);
	}
	mp->next = mprog_free;
	mprog_free = mp;
}
/**************************************************************************/	
