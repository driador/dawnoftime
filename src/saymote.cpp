/**************************************************************************/
// saymote.cpp - saymote system written by Kalahn, Idea from Kirion.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/*
The player types:

saymote >kalahn [with a bow] greetings [and, then, smiling] how have you been?

and Kalahn would see it:

Kirion says to you with a bow: 'greetings' and, then, smiling: 'how have you been?'

Translating the said parts according to the language of the
speaker/listener.

Also supports [[ and ]] to allow a player to actually put '[' and ']' codes 
in their says
*/
#include "include.h" // dawn standard includes

/**************************************************************************/
// Kal - June 1999
void saymote( language_data *language, char_data *ch, char *argument, int sayflags)
{
	int i;

	if(IS_SET(sayflags, SAYFLAG_ONLY_REMEMBERED_CHAR) 
		&& (!IS_NPC(ch)|| !ch->mprog_target))
	{
		ch->printlnf("rsay '%s' not processed because you arent remembering anyone.", argument);
		return;
	}

	bool say_with_emotes=true;
	if(IS_SET(sayflags, SAYFLAG_NO_SAYMOTE)){
		say_with_emotes=false;
	}
	bool convert_colour=false;
	if(IS_SET(sayflags, SAYFLAG_CONVERT_COLOUR)){
		convert_colour=true;
	}
	
    static int rpmon=0;
	const int MAXBUFCOUNT=8;

	char	tochar_sayask_text[30], 
			toall_sayask_text[30], 
			tovictim_sayask_text[30];
	char splitbuf[MAXBUFCOUNT][(MIL*2)+3];	// (MIL*2)+3 to prevent $$$$... 
											// causing buffer overrun
	tovictim_sayask_text[0]='\0';
	char_data *victim= NULL; // if victim is true we have a sayto target

	// clear our buffers
	for(i=0; i<MAXBUFCOUNT; i++){
		splitbuf[i][0]='\0';
	};

	// *** idiot checks
    if ( IS_NULLSTR(argument) )
    {
        ch->println("Say what?");
        return;
    }
	// don't allow messages that could be defrauding
	if(check_defrauding_argument(ch, argument)){
		return;
	}
	// prevent potential buffer overruns
    if (count_char(argument,'$')>10) 
    {
        ch->println("You can't have more than 10 $'s in a saymote.");
        return;
    }
    if (count_char(argument,'[')>10) 
    {
        ch->println("You can't have more than 10 ['s in a say/saymote.");
        return;
    }
    if (count_char(argument,']')>10) 
    {
        ch->println("You can't have more than 10 ]'s in a say/saymote.");
        return;
    }

	if ( !IS_NPC(ch) 
		&& !IS_SET(language->flags, LANGFLAG_NO_SKILL_REQUIRED) 
		&& language->gsn>=0 
		&& get_skill(TRUE_CH(ch), language->gsn)<1 )
	{
		ch->printlnf("You have no knowledge of any %s words.", language->name);            
		return;
	}

	// wiznet RPMONITOR
    if (!IS_NPC(ch) && !IS_OOC(ch)) 
    {
        rpmon++;
        if (rpmon>=20)
        {
			int rplevel;
			char buf2[MSL];
			rplevel = (IS_TRUSTED(ch,LEVEL_IMMORTAL)?MAX_LEVEL:get_trust(ch));

            sprintf (buf2, "Wiznet rpmonitor: %s says '%s'`x", 
				ch->name, argument);
            wiznet(buf2,ch,NULL,WIZ_RPMONITOR,0,rplevel);
            if (rpmon>=23) rpmon=0;
        }
    }

	if(!IS_NPC(ch))
    {
		ch->pcdata->did_ic=true;
        if(is_room_private_to_char(ch->in_room, ch))
		{
            ch->pcdata->did_ooc=true;
            ch->pcdata->did_ic=false;
        }
	}

    // trim off any leading blanks
	argument = ltrim_string(argument);

	// anti autorps abuse system
	if(!IS_NPC(ch) && IS_IC(ch))
	{
		++ch->pcdata->say_index%=RPS_AUDIT_SIZE;
		free_string(ch->pcdata->saycheck[ch->pcdata->say_index]);
		ch->pcdata->saycheck[ch->pcdata->say_index]=str_dup(argument);
		ch->pcdata->saytimes[ch->pcdata->say_index]=current_time;
	}

	// Check for the nospeak flag, Cel Nov 99
	if(IS_SET(ch->in_room->room_flags, ROOM_NOSPEAK)
	&& !HAS_CONFIG(ch, CONFIG_HOLYSPEECH)) {
		ch->println("You feel the air escape your lungs, and yet... no sound.");
		return;
	}

	// *** Find if we have a sayto victim
	if (argument[0]=='>')
	{
		char to_arg[MIL];
		argument++; 
	    argument = one_argument( argument, to_arg);
	    victim = get_char_room( ch, to_arg);
		if (!victim)
		{
			ch->printlnf("You can't find '%s' to 'say to'",to_arg);
			return;
		}

		if (victim==ch)
		{
			ch->println("You can't direct your says at yourself.");
			return;
		}

		if (victim->position ==POS_SLEEPING)
		{
			ch->printlnf("%s is asleep.",icapitalize(PERS(victim, ch)));
			return;
		}

		if ( IS_NULLSTR(argument) )
		{
			ch->printlnf( "Say what to %s?", PERS(victim,ch));
			return;
		}
	}

	// parse the text
	int splitbuf_index=0;// even index = say, odd = emote
	bool in_emote=false; // true when we get a '[' but haven't yet seen a ']'
	char *pstr;
	char *writestr=&splitbuf[0][0]; // where the next character will go
	bool trim_space=true; // used to trim the leading space off something

	for(pstr=argument; !IS_NULLSTR(pstr); pstr++){
		// trim of a single space from the start of each type
		if(trim_space && *pstr==' '){	
			continue;
		}
		trim_space=false;

		// $ are converted to $$ and then converted back to $ by act_new()
		if(*pstr=='$'){
			// double up the character into the appropriate buffer
			*writestr++='$';
			*writestr++='$';
			continue;
		};

		// ` is converted to `` if convert colour is enabled
		if(convert_colour && *pstr=='`'){
			// double up the character into the appropriate buffer
			*writestr++='`';
			*writestr++='`';
			continue;
		};

		// convert [[ into [
		if(say_with_emotes){
			if(*pstr=='[' && *(pstr+1)=='[' ){
				*writestr++='[';
				pstr++;
				continue;
			}

			// convert ]] into ]
			if(*pstr==']' && *(pstr+1)==']' ){
				*writestr++=']';
				pstr++;
				continue;
			}
		}
		
		// check for state changing characters
		if(say_with_emotes && *pstr=='['){ // start emote
			if(in_emote){
				ch->printf("You can't have a [ character in a say after a [.\r\n"
					"  (to close the emote use a ]).\r\n"
					"   See `=Chelp saymote`x.\r\n");
				ch->wrapln("hint: you can also disable the automated use of saymote "
					"for the say command by typing `=Cautosaymote`x");
				return;
			};

			splitbuf_index++;
			if(splitbuf_index>=MAXBUFCOUNT){
				ch->printlnf("There is a limit of %d alternating says + emotes in a saymote.",
					MAXBUFCOUNT);
				return;
			};
			
			in_emote=true;
			*writestr='\0';	// terminate the buffer we were working on
			//trim off last character in a say if it is a space
			if(is_space(*(writestr-1))){
				*(writestr-1)='\0'; // terminate the buffer trimming the last single
									// space if necessary from the say
			}
			
			writestr=&splitbuf[splitbuf_index][0];
			continue; // just switched into emote mode, go onto next character
		}
		
		if(say_with_emotes && *pstr==']'){ // close emote 
			if(!in_emote){ 
				ch->println("You can't have a ] character before a matching [.\r\n"
					"  (to start an emote inside a say use a [).\r\n"
					"   See `=Chelp saymote`x.\r\n"
					"   To say an actual [ character, use [[.");
				ch->wrapln("hint: you can also disable the automated use of saymote "
					"for the say command by typing `=Cautosaymote`x");
				return;
			};

			splitbuf_index++;
			if(splitbuf_index>=MAXBUFCOUNT){
				ch->printlnf("There is a limit of %d alternating says + emotes in a saymote.",
					MAXBUFCOUNT);
				return;
			};
	
			in_emote=false;
			trim_space=true;// enable inital space trimming for next say
			*writestr='\0'; // terminate the buffer we were working on
			writestr=&splitbuf[splitbuf_index][0];
			continue; // just switched into say mode, go onto next character
		}

		// copy the character into the appropriate buffer
		*writestr++=*pstr;
	}
	// check that there isn't an open emote
	if(in_emote){
		ch->printf("You can't leave an open emote in a saymote.\r\n"
			"   See `=Chelp saymote`x.\r\n");
		ch->wrapln("hint: you can also disable the automated use of saymote "
			"for the say command by typing `=Cautosaymote`x");
		return;
	}
	*writestr='\0'; // terminate the buffer we were working on
	
	// *** decide if is relating to asking/saying and sayto
	bool question=false;
	bool exclaim=false;
	for(i=0; i<=splitbuf_index; i+=2){
		if(IS_NULLSTR(splitbuf[i])){
			continue;
		}
		char *trimmedtext=rtrim_string(splitbuf[i]);

		if(trimmedtext[str_len(trimmedtext)-1]=='?'){
			question=true;
		}else if(trimmedtext[str_len(trimmedtext)-1]=='!'){
			exclaim=true;
		}
		break;
	}
	
	if (question){ // it is a question therefore asking
		if(victim){						
			strcpy(tochar_sayask_text,"ask $N");
			strcpy(toall_sayask_text,"asks $N");
			strcpy(tovictim_sayask_text,"`#`Wasks you`&");
		}else{
			strcpy(tochar_sayask_text,"ask");
			strcpy(toall_sayask_text,"asks");
		}
	}
	else if (exclaim){
		if(victim){						
			strcpy(tochar_sayask_text,"exclaim to $N");
			strcpy(toall_sayask_text,"exclaims to $N");
			strcpy(tovictim_sayask_text,"`#`Wexclaims to you`&");
		}else{
			strcpy(tochar_sayask_text,"exclaim");
			strcpy(toall_sayask_text,"exclaims");
		}
	}
	else
	{ // saying
		if(victim){						
			strcpy(tochar_sayask_text,"say to $N");
			strcpy(toall_sayask_text,"says to $N");
			strcpy(tovictim_sayask_text,"`#`Wsays to you`&");
		}else{
			strcpy(tochar_sayask_text,"say");
			strcpy(toall_sayask_text,"says");
		}
	}

	char format_buffer[MSL*2]; // *2 because the translator can expand text
	int format_index;
	char buf[MSL*2];
	char wizitext[20];
	bool langshow;
	if(GAMESETTING3(GAMESET3_LANGUAGE_NAME_NOT_IN_SAYS) 
		|| language == language_unknown){
		langshow=false;
	}else{
		langshow=true;
	}

	int max_invislevel=INVIS_LEVEL(ch);
	
	if(victim){
		max_invislevel=UMAX(INVIS_LEVEL(victim),max_invislevel);
	}
	if(max_invislevel){
		sprintf(wizitext,"[Wizi %d] ", max_invislevel);
	}else{
		wizitext[0]='\0';
	}
	bool blind=false, deaf=false;

    // translate the language to all/intended people? in the room
    for( char_data *to= ch->in_room->people; to ; to = to->next_in_room )
    {
		if(IS_SET(sayflags, SAYFLAG_ONLY_REMEMBERED_CHAR) && to!=ch->mprog_target){
			continue;
		}

        if (
            (!IS_NPC(to) && to->desc == NULL) // linkdead or switched out of
             || ch==to						  // self	
             || (IS_UNSWITCHED_MOB(to) && !IS_SET(to->act,ACT_MOBLOG))  // non switched mob 
																		// without a moblog
			 || !IS_AWAKE(to) ){    // asleep
			continue;
		}

		// players can't see saymotes by wizi characters, or said to 
		// wizi characters
		if (!IS_TRUSTED(to, max_invislevel)){
			continue;
		}

		// set the deaf and blind for quick lookup
		if(IS_IC(to)){
			deaf=(is_affected(to, gsn_deafness ));

			blind=(IS_AFFECTED( to, AFF_BLIND ) || is_affected( to, gsn_blindness ));

			// someone that is deaf and blind gets nothing sent to them if IC
			if(deaf && blind)
				continue;
		}

		sprintf(format_buffer,"%s%s%s %s",
			(IS_IMMORTAL(to)?wizitext:""),			// [Wizi %d]
			IS_OOC(ch)?"`B(OOC) `=\xc3":"`=\xc3",			// (OOC)
			PERS(ch, to),							// ch->name/ch->short_descr
			to==victim?tovictim_sayask_text:toall_sayask_text); // say/ask

		// if there are more parts process them
		bool show_lang_to_char=langshow;
		for(format_index=0; format_index<=splitbuf_index; format_index++){
			if(IS_NULLSTR(splitbuf[format_index]))
				continue;
			if(format_index%2==0){ // says are even
				if(deaf){
					sprintf(buf," `S<%s's lips move>`x", PERS(ch,to));
					strcat(format_buffer,buf);
				}else{
					sprintf(&format_buffer[str_len(format_buffer)]," `x'`%c", ch->saycolour);					
					translate_language(language, show_lang_to_char, ch, to, splitbuf[format_index], buf);
					strcat(format_buffer, buf);
					strcat(format_buffer,"`x'");// reset colour
				}
				show_lang_to_char=false;
			}else{ //emotes are odd
				if(!blind){
					sprintf(&format_buffer[str_len(format_buffer)]," `%c", ch->motecolour);
					strcat(format_buffer,splitbuf[format_index]);
					strcat(format_buffer,"`x");// reset colour
				}
			}
		}		
		RECORD_TO_REPLAYROOM=true;
		act( format_buffer, to, NULL, victim, TO_CHAR );
		RECORD_TO_REPLAYROOM=false;
	} // end of translation loop

	
	{	// format what is sent to the actual speaker
		// because this goes thru act() we don't need to put the [Wizi %d] on
		if(IS_OOC(ch)){
			if(IS_SET(language->flags,LANGFLAG_REVERSE_TEXT)){
				sprintf(format_buffer,"`B(OOC) `=\xc3You %s [reversing language]", tochar_sayask_text);
			}else{
				sprintf(format_buffer,"`B(OOC) `=\xc3You %s", tochar_sayask_text);
			}
		}else{
			if(GAMESETTING3(GAMESET3_LANGUAGE_NAME_NOT_IN_SAYS) 
				|| language == language_unknown)
			{
				sprintf(format_buffer,"`x%s`=\xc3You %s", 
					(IS_IMMORTAL(ch)?wizitext:""), tochar_sayask_text);					
			}else{
				sprintf(format_buffer,"`x%s`=\xc3You %s in %s",
					(IS_IMMORTAL(ch)?wizitext:""), tochar_sayask_text, language->name);
			}
		}

		// if there are more parts process them
		for(format_index=0; format_index<=splitbuf_index; format_index++){
			if(IS_NULLSTR(splitbuf[format_index]))
				continue;
			if(format_index%2==0){ // says are even
				sprintf(&format_buffer[str_len(format_buffer)]," `=\xc3'`%c", ch->saycolour);
				strcat(format_buffer,splitbuf[format_index]);
				strcat(format_buffer,"`=\xc3'`x");// reset colour
			}else{ //emotes are odd
				sprintf(&format_buffer[str_len(format_buffer)]," `%c", ch->motecolour);
				strcat(format_buffer,splitbuf[format_index]);
				strcat(format_buffer,"`=\xc3`x");// reset colour
			}
		}		
		RECORD_TO_REPLAYROOM=true;
		act( format_buffer, ch, NULL, victim, TO_CHAR );
		RECORD_TO_REPLAYROOM=false;
	}

	int number_in_room=ch->in_room->number_in_room;
	// note: number_in_room is used to prevent a mudprog on two mobs in 
	// a room creating an endless loop by removing themselves from
	// the room and putting themselves back in the room - Kal, June 01

	// handle mudprog speech and act triggers
    if ( !IS_NPC(ch) && !IS_SET(sayflags, SAYFLAG_ONLY_REMEMBERED_CHAR) )
    {
		char_data *mob_next;
        char_data *mob= ch->in_room->people; 
        for ( ; mob && --number_in_room>=0; mob = mob_next )
        {
            mob_next = mob->next_in_room;
			if(!IS_NPC(mob)){
				continue;
			}
            if (HAS_TRIGGER( mob, MTRIG_SPEECH ) 
				|| HAS_TRIGGER( mob, MTRIG_ACT ) 
				|| (HAS_TRIGGER( mob, MTRIG_SAYTO) && victim==mob))
			{
				for(format_index=0; format_index<=splitbuf_index; format_index++){
					if(IS_NULLSTR(splitbuf[format_index]))
						continue;
					if(format_index%2==0){ // says are even
						if(HAS_TRIGGER( mob, MTRIG_SAYTO))
						{
							mp_act_trigger( splitbuf[format_index], mob, 
								ch, NULL, NULL, MTRIG_SAYTO );
						}
						if(HAS_TRIGGER( mob, MTRIG_SPEECH ))
						{
							mp_act_trigger( splitbuf[format_index], mob, 
								ch, NULL, NULL, MTRIG_SPEECH );
						}
					}else{ //emotes are odd
						if(HAS_TRIGGER( mob, MTRIG_ACT )){
							mp_act_trigger( splitbuf[format_index], mob, 
								ch, NULL, NULL, MTRIG_ACT );
						}
					}
				}		
			}
        }
    }
}
/**************************************************************************/
