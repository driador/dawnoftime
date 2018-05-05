/**************************************************************************/
// chardata.cpp - implementation of char_data member functions 
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

#include "include.h"
#include "events.h"
void flush_cached_write_to_buffer(connection_data *d);
/**************************************************************************/
void mpqueue_queue_print_entity(int seconds, const char *text, entity_data *ent);
extern bool mp_suppress_text_output;
/**************************************************************************/
// just send it to the characters output buffer
void char_data::print(const char *txt)
{
	char *buf=(char *)txt;
	if(!this){
		return;
	}

	if(mp_suppress_text_output && !IS_IMMORTAL(this) && !IS_NPC(this)){
		// text suppression in effect, suppress for non-immortal players
		return;
	}
	
	if(m_pdelay){
		if(mp_suppress_text_output){
			queue_print(EVENTGROUP_SYSTEM_PRINT,m_pdelay, "[SUPPRESSED TEXT]");
		}
		queue_print(EVENTGROUP_SYSTEM_PRINT,m_pdelay, buf);
	}else{		
		process_moblog(this, buf);
		if ( IS_NULLSTR(buf) || !this->desc){
			return;
		}
		if(mp_suppress_text_output){
			write_to_buffer( this->desc, "[SUPPRESSED TEXT]", 17 );
		}
		write_to_buffer( this->desc, buf, str_len(buf) );
	}
}

/**************************************************************************/
// the delay all the print style commands by this amount
int char_data::pdelay()
{
	return m_pdelay;
}; 
/**************************************************************************/
void char_data::set_pdelay(int seconds)
{
	m_pdelay=seconds;
};
/**************************************************************************/
void char_data::printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL, fmt, args);
	va_end(args);
	
	print(temp_HSL_workspace);
}
/**************************************************************************/
void char_data::printfbw(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL, fmt, args);
	va_end(args);
	
	printbw(temp_HSL_workspace);
}
/**************************************************************************/
void char_data::printf(int seconds, const char *fmt, ...)
{
	if(!this){
		return;
	}
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL, fmt, args);
	va_end(args);
	
	m_pdelay=seconds;
	print(temp_HSL_workspace);
	m_pdelay=0;
}
/**************************************************************************/
// send b+w text
void char_data::printbw(const char *buf)
{
	if(!this){
		return;
	}

	if(mp_suppress_text_output && !IS_IMMORTAL(this) && !IS_NPC(this)){
		// text suppression in effect, suppress for non-immortal players
		return;
	}

	if(m_pdelay){
		// no black and white if print delay in use
		if(mp_suppress_text_output){
			queue_print(EVENTGROUP_SYSTEM_PRINT,m_pdelay, "[SUPPRESSED TEXT]");
		}
		queue_print(EVENTGROUP_SYSTEM_PRINT, m_pdelay, buf);
	}else{
		// Send text to a single character with no colour parsing
		process_moblog(this, buf);
		if ( IS_NULLSTR(buf) || !this->desc){
			return;
		}

		// convert < to &lt;, > to &gt; and & to &amp;
		buf=mxp_convert_to_mnemonics(buf);

		// condensing system has to be flushed for raw colour to work correctly
		flush_cached_write_to_buffer(this->desc);
		if(this->desc->parse_colour){
			this->desc->parse_colour=false;
			if(mp_suppress_text_output){
				write_to_buffer( this->desc, "[SUPPRESSED TEXT]", 17 );
			}
			write_to_buffer( this->desc, buf, 0);
			flush_cached_write_to_buffer(this->desc);
			this->desc->parse_colour=true;
		}else{
			if(mp_suppress_text_output){
				write_to_buffer( this->desc, "[SUPPRESSED TEXT]", 17 );
			}
			write_to_buffer( this->desc, buf, 0);
			flush_cached_write_to_buffer(this->desc);
		}
	}
}
/**************************************************************************/
// send b+w text + linefeed
void char_data::printlnbw(const char *buf)
{
	printbw(buf);
	print("\r\n");
}
/**************************************************************************/
// prepend \r\n to what is to be sent
void char_data::println(const char *buf)
{
	print(buf);
	print("\r\n");
}

/**************************************************************************/
void char_data::print_blank_lines(int lines)
{
	char buf[MSL];
	buf[0]='\0';
	for(int i=0; i<lines; i++){
		strcat(buf,"\r\n");
	}
	print(buf);
}
/**************************************************************************/
// prepend \r\n to what is to be sent, send to char in number of seconds
void char_data::println(int seconds, const char *buf)
{
	if(!this){
		return;
	}
	
	m_pdelay=seconds;
	// - copy the buffer to be printed, into temp_HSL_workspace, 
	//   then concatenate the new line onto the end before sending for printing
	// - this is done instead of using println(), if println() is used with
	//   the delay, then it results in two queued events - pointless
	strncpy(temp_HSL_workspace,buf, HSL-4);
	strcat(temp_HSL_workspace,"\r\n");
	print(temp_HSL_workspace);
	m_pdelay=0;
}
/**************************************************************************/
void char_data::printlnf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL-4, fmt, args);
	va_end(args);
	
	strcat(temp_HSL_workspace,"\r\n");
	print(temp_HSL_workspace);
}
/**************************************************************************/
void char_data::printlnfbw(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL-4, fmt, args);
	va_end(args);
	
	strcat(temp_HSL_workspace,"\r\n");
	printbw(temp_HSL_workspace);
}
/**************************************************************************/
void char_data::printlnf(int seconds, const char *fmt, ...)
{
	if(!this){
		return;
	}
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL-4, fmt, args);
	va_end(args);
	
	m_pdelay=seconds;
	strcat(temp_HSL_workspace,"\r\n");
	print(temp_HSL_workspace);
	m_pdelay=0;
}
/**************************************************************************/
// wordwrap to the char
void char_data::wrap(const char *buf)
{
	char *wrapped=str_dup(buf);
	wrapped=note_format_string_width(wrapped, 77, true, false);
	print(wrapped);
	free_string(wrapped);
}
/**************************************************************************/
void char_data::wrapf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL, fmt, args);
	va_end(args);
	
	char *wrapped=str_dup(temp_HSL_workspace);
	wrapped=note_format_string_width(wrapped, 77, true, false);
	print(wrapped);
	free_string(wrapped);
}

/**************************************************************************/
void char_data::bug_print(const char *txt)
{
	bug(txt);
	print("BUG: ");
	print(txt);
	print("\r\n===please report above bug to the admin==");
}
/**************************************************************************/
// send a bug entry to system log + prepend \r\n to what is sent to user
void char_data::bug_println(const char *buf)
{
	bug_print(buf);
	print("\r\n");
}
/**************************************************************************/
void char_data::bug_printlnf(const char *fmt, ...)
{
	if(!this){
		return;
	}
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL, fmt, args);
	va_end(args);
	
	bug_println(temp_HSL_workspace);
}
/**************************************************************************/
// prepend \r\n to what is to be sent
void char_data::wrapln(const char *buf)
{
	char *wrapped=str_dup(buf);
	wrapped=note_format_string_width(wrapped, 77, true, true);
	print(wrapped);
	free_string(wrapped);
}
/**************************************************************************/
void char_data::wraplnf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp_HSL_workspace, HSL, fmt, args);
	va_end(args);
	
	char *wrapped=str_dup(temp_HSL_workspace);
	wrapped=note_format_string_width(wrapped, 77, true, true);
	print(wrapped);
	free_string(wrapped);
}
/**************************************************************************/
int char_data::get_skill_level(int sn)
{
    if (sn == -1 || sn>MAX_SKILL){ 
		bugf("int char_data::get_skill_level(int sn), asking for sn=%d", sn);
		return MAX_LEVEL;
	}
	return skill_table[sn].skill_level[clss];
}
/**************************************************************************/
// returns the percentage of skill they have in it
int char_data::get_skill(int sn)
{
	// shorthand for level based skills 
    if (sn == -1){ 
		return URANGE(0,(level * 5 / 2),100);
	}

	// bounds checking
    if(sn < -1 || sn > MAX_SKILL){
        bugf("char_data::get_skill(): Bad sn %d",sn);
        return 0;
	}

	if(IS_NPC(this)){
		return npc_skill_level(sn);
	}
	return pc_skill_level(sn);
}
/**************************************************************************/
int char_data::npc_skill_level(int sn)
{
	if(IS_CONTROLLED(this)){
		return 100;
	}

	int skill=0;
    if (IS_SPELL(sn)){
        skill= 40 + 2 * level;
	}else{
		if(	sn==gsn_sneak||
			sn==gsn_hide)
		{
				skill= level * 2 + 20;
		}else if (sn==gsn_dodge){
			if(IS_SET(off_flags,OFF_DODGE)){
				skill = level * 2;
			}
		}else if(sn== gsn_parry){
			if(IS_SET(off_flags,OFF_PARRY)){
				skill = level * 2;
			}
		}else if(sn== gsn_fade){
			if(IS_SET(off_flags,OFF_FADE)){ 
				skill = level * 2;
			} 
		}else if(sn== gsn_shield_block){
			skill=10 + 2 * level;
		}else if(sn== gsn_second_attack){
			if(IS_SET(act,ACT_WARRIOR | ACT_THIEF)){
				skill = 10 + 3 * level;
			}
		}else if(sn== gsn_third_attack){
			if(IS_SET(act,ACT_WARRIOR)){
				skill = 4 * level - 40;
			}
		}else if(sn== gsn_quad_attack){
			if(IS_SET(act,ACT_WARRIOR)){
				skill = 5 * level - 60; 
			}
		}else if(sn== gsn_hand_to_hand){
			skill = 40 + 2 * level;
		}else if(sn== gsn_trip){
			if(IS_SET(off_flags,OFF_TRIP)){
				skill = 10 + 3 * level;
			}
		}else if(sn== gsn_bash){
			if(IS_SET(off_flags,OFF_BASH)){
				skill = 10 + 3 * level;
			}
		}else if(sn== gsn_disarm){
			if(IS_SET( off_flags,OFF_DISARM) && 
				IS_SET(act,ACT_WARRIOR | ACT_THIEF))
			{
				skill = 20 + 3 * level;
			}
		}else if(sn== gsn_berserk){
			if(IS_SET( off_flags,OFF_BERSERK)){
				skill = 3 * level;
			}
		}else if(sn== gsn_kick){
			skill = 10 + 3 * level;
		}else if(sn== gsn_gore){
			if(IS_SET( off_flags, OFF_GORE )){
				skill = 10 + 3 * level;
			}
		}else if(sn== gsn_backstab){
			if(IS_SET(act,ACT_THIEF)){
				skill = 20 + 2 * level;
			}
		}else if(sn== gsn_rescue || sn== gsn_recall){
			skill = 40 + level; 
		}else if(sn== gsn_sword
			|| sn== gsn_spear
			|| sn== gsn_dagger
			|| sn== gsn_staff
			|| sn== gsn_mace			
			|| sn== gsn_axe
			|| sn== gsn_flail
			|| sn== gsn_whip
			|| sn== gsn_polearm)
		{
			skill = 40 + 5 * level / 2;
		}else if (IS_SET(act2, ACT2_ALLSKILLS)){
			skill = 40 + 2 * level;
		}else{			
			skill = 0;
		}
	}

	// dazed mobs
    if (daze > 0)
    {
		if (IS_SPELL(sn)){
			skill = 2 * skill / 3;
		}else{
			skill /= 2;
		}
    }
    return URANGE(0,skill,100);
}
/**************************************************************************/
int char_data::pc_skill_level(int sn)
{
	int skill;
    // have level checks if it isnt a spell 
    if (!IS_SPELL(sn))
    {
        if (level < skill_table[sn].skill_level[clss]
			|| skill_table[sn].skill_level[clss]==0)
            skill = 0;
        else
            skill = pcdata->learned[sn];
    }
    else
	{
        skill = pcdata->learned[sn];
	}

	// grant working on skills and spells
	if (pcdata->learned[sn]==101){
		skill = 100;
	}

	// dazed players
    if (daze > 0)
    {
		if (IS_SPELL(sn)){
			skill = 2 * skill / 3;
		}else{
			skill /= 2;
		}
    }
	// drunk players
	if(pcdata->condition[COND_DRUNK]>10){
		skill = 9 * skill / 10;
	}

	// scale down if using the new system
	if(HAS_CONFIG(this,CONFIG_PRACSYS_TESTER)){
		skill = skill * skill_table[sn].get_learnscale(this) / 100;
	}
    return URANGE(0,skill,100);
}; 
/**************************************************************************/
// return the percentage they have out of the max for the class
int char_data::get_display_skill(int sn)
{
	if(IS_NPC(this)){
		return this->get_skill(sn);
	}

	// bounds checking
    if(sn < 0 || sn > MAX_SKILL){
        bugf("char_data::get_display_skill(): Bad sn %d",sn);
        return 0;
	}
	return (pcdata->learned[sn]);
}; 

/**************************************************************************/
vn_int char_data::vnum()
{
	if(!this){
		return -2; // NULL pointer
	}
	if(pIndexData){
		return pIndexData->vnum; // mob vnum
	}
	return 0; // zero means it is a player
};
/**************************************************************************/
vn_int char_data::in_room_vnum()
{
	if(!this || !in_room){
		return 0; // zero means they arent in a room for some reason
	}
	return in_room->vnum; // room vnum
};
/**************************************************************************/
void char_data::sendpage(const char *buf)
{
	char *txt=(char *)buf;
	process_moblog(this, txt); // moblog flag

    if ( IS_NULLSTR(txt) || !this || !desc){
		return;
	}

    if (lines == 0 ) // pager turned off
    {
		print(txt);
		return;
    }
	
	if (desc->showstr_head &&
       (str_len(txt)+str_len(desc->showstr_head)+1) < 32000)
    {
		char *temp=(char*)alloc_mem(str_len(txt) + str_len(desc->showstr_head) + 1);
		strcpy(temp, desc->showstr_head);
		strcat(temp, txt);
		desc->showstr_point = temp + 
			(desc->showstr_point - desc->showstr_head);
		free_mem(desc->showstr_head, str_len(desc->showstr_head) + 1);
		desc->showstr_head=temp;
    }else{
		if (desc->showstr_head){
			free_mem(desc->showstr_head, str_len(desc->showstr_head)+1);
		}
		desc->showstr_head = (char*)alloc_mem(str_len(txt) + 1);
		strcpy(desc->showstr_head,txt);
		desc->showstr_point = desc->showstr_head;
		show_string(desc,"");
    }
}
/**************************************************************************/
void char_data::titlebar(const char *txt)
{
	char *line=	"=================================================================="
				"==================================================================";
	int clen=c_str_len(txt); // length of string without colour codes

	// display a blank bar?
	if(clen<1)
	{
		println("`#`=t-===========================================================================-`&");
		return;       
	}

	// to much text to display within a bar?
	if(clen>78)
	{
		println(txt);
		return;
	}

	char buf[MIL];
	int spaces= (74-clen)/2;

	if(clen%2==0){ // even
		sprintf(buf, "`=t-%%.%ds`#`#`#`# `=T%%s `&`&`&`&%%.%ds-`x", spaces, spaces-1);
	}else{ // odd
		sprintf(buf, "`=t-%%.%ds`#`#`#`# `=T%%s `&`&`&`&%%.%ds-`x", spaces, spaces);
	}

	// show the titlebar
	printlnf( buf, line, txt, line);
}
/**************************************************************************/
void char_data::titlebarf(const char *fmt, ...)
{
	char buf[MIL];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MIL, fmt, args);
	va_end(args);
	
	titlebar(buf);
}
/**************************************************************************/
void char_data::olctitlebar(const char *txt)
{
	char *line=	"=================================================================="
				"==================================================================";
	int clen=c_str_len(txt); // length of string without colour codes

	// display a blank bar?
	if(clen<1)
	{
		println("`#`=r-===========================================================================-`&");
		return;       
	}

	// to much text to display within a bar?
	if(clen>78)
	{
		println(txt);
		return;
	}

	char buf[MIL];
	int spaces= (74-clen)/2;

	if(clen%2==0){ // even
		sprintf(buf, "`=r-%%.%ds`#`#`#`# `=R%%s `&`&`&`&%%.%ds-`x", spaces, spaces-1);
	}else{ // odd
		sprintf(buf, "`=r-%%.%ds`#`#`#`# `=R%%s `&`&`&`&%%.%ds-`x", spaces, spaces);
	}

	// show the titlebar
	printlnf( buf, line, txt, line);
}
/**************************************************************************/
void char_data::olctitlebarf(const char *fmt, ...)
{
	char buf[MIL];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MIL, fmt, args);
	va_end(args);
	
	olctitlebar(buf);
}
/**************************************************************************/
extern const char *mxp_start; // defined in connect.h
/**************************************************************************/
void char_data::mxp_send_init()
{
	// turning off or already off
	if(!MXP_DETECTED_OR_ON(this)){
		if(this){
			if(pcdata){
				pcdata->mxp_enabled=false;
			}
			if(desc){
				desc->mxp_enabled=false;
			}
			print(MXP_LOCKED_MODE);
		}
		return;
	}

	// turning on 
	// send the mxp init stuff only once per hotreboot
	if(!pcdata->mxp_enabled){ 
		pcdata->mxp_enabled=true;
		if(desc){
			desc->mxp_enabled=true;
		}

		// this puts the client into secure mode
		mxp_define_elements_to_char(this);
	}
}
/**************************************************************************/
// send them the MXP enabled [Hit Return to continue] bar
void char_data::hit_return_to_continue()
{
	print("`#`=\xaa");
	print(mxp_create_send(this, "continue", "[Hit Return to continue]"));					
	println("`^");
}
/**************************************************************************/
// record says and emotes etc for display with the replayroom command
void char_data::record_replayroom_event(const char *txt)
{
	char_data *ch=TRUE_CH(this);

	if(!ch || !ch->pcdata){
		// only record for players
		return; 
	}

	char recordbuf[MSL];
	if(ch->pcdata->replayroom_lastevent_roomvnum!=in_room_vnum()){
		// insert a blank line with a - for new rooms
		sprintf(recordbuf, "-\r\n%s> %s", shorttime(NULL), txt);
		ch->pcdata->replayroom_lastevent_roomvnum=in_room_vnum();
	}else{
		sprintf(recordbuf, "%s> %s", shorttime(NULL), txt);
	}

	replace_string(	// record it in their replayroom buffer
		ch->pcdata->replayroom_text[ch->pcdata->next_replayroom], 
		recordbuf);
	++ch->pcdata->next_replayroom%=MAX_REPLAYROOM;

}
/**************************************************************************/
// suspend events for self, inventory, any pet and its inventory
void char_data::moving_from_ic_to_ooc() 
{
	// order of logic taking from pfile saving
	suspend_events();
	if(carrying){
		carrying->moving_from_ic_to_ooc();
	}

	if(pet){
		pet->moving_from_ic_to_ooc();
		// it is not necesary to handle saving the pets inventory here
		// because it will be handled by pet->moving_from_ic_to_ooc();
	}

}
/**************************************************************************/
// unsuspend events for self, and anything we contain/next in carried list
void char_data::moving_from_ooc_or_load_to_ic() 
{
	// order of logic taking from pfile saving
	unsuspend_events();
	if(carrying){
		carrying->moving_from_ooc_or_load_to_ic();
	}

	if(pet){
		pet->moving_from_ooc_or_load_to_ic();
		// it is not necesary to handle saving the pets inventory here
		// because it will be handled by pet->moving_from_ooc_or_load_to_ic();
	}

}
/**************************************************************************/
/**************************************************************************/

