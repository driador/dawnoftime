/**************************************************************************/
// whofmt.cpp - various formats for the who list
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "channels.h"
/**************************************************************************/
// example whoformat function - about as simple as it gets
char *whoformat_just_names( char_data *ch, char_data *wch, bool two_column)
{
	return wch->name;
}

/**************************************************************************/
char *whoformat_dawn ( char_data *ch, char_data *wch, bool two_column)
{
	static char return_buf[MSL*2];
    char const *clss;
	char bracket_col[10]; // [ colour
    char left_bracket[20]; // L, ! or [ 
    char right_bracket[20]; // ! or ]
    char buf[MSL];
    char buf2[MSL];
    char wizitext[40]; 
	char incogtext[20]; 
	char whoclass[20]; 

	return_buf[0]='\0'; // clear the initial return string

	char name[MIL];
	{
		strcpy(name, wch->name);
		// generate the name displayed
		// put the afk message or title depending on if they are afk
		if(IS_SET(wch->comm, COMM_AFK) && wch->pcdata 
			&& !IS_NULLSTR(wch->pcdata->afk_message))
		{
			strcat(name, FORMATF(" (%s)", wch->pcdata->afk_message));
		}else if(!GAMESETTING3(GAMESET3_WHO_TITLE_DISABLED)){
			// concat the title
			strcat(name, FORMATF(" %s", wch->pcdata->title));
		}
	}

	// Figure out what to print for clss.
	clss="***";
    switch ( wch->level )
    {
		default: clss="***";break;
		case MAX_LEVEL - 0 : clss = "IMP";     break;
		case MAX_LEVEL - 1 : clss = "CRE";     break;
		case MAX_LEVEL - 2 : clss = "SUP";     break;
		case MAX_LEVEL - 3 : clss = "DEI";     break;
		case MAX_LEVEL - 4 : clss = "GUA";     break;
		case MAX_LEVEL - 5 : clss = "IMM";     break;
		case MAX_LEVEL - 6 : clss = "DEM";     break;
		case MAX_LEVEL - 7 : clss = "ANG";     break;
		case MAX_LEVEL - 8 : clss = "AVA";     break;
	}
    
	if(wch->level==ABSOLUTE_MAX_LEVEL && ABSOLUTE_MAX_LEVEL!=MAX_LEVEL){
		if(wch->pcdata && wch->pcdata->who_text && c_str_len(wch->pcdata->who_text)==3){
			clss=wch->pcdata->who_text;
		}else{
			clss="***";
		}
	}

	// alternate 3 tier imm ranking system (IMP, ADM, IMM)
	if(GAMESETTING4(GAMESET4_3TIER_IMMRANKS_IN_WHO)){
		switch ( wch->level )
		{
			case MAX_LEVEL - 0 : clss = "IMP";     break;
			case MAX_LEVEL - 1 : clss = "ADM";     break;
			case MAX_LEVEL - 2 : clss = "ADM";     break;
			case MAX_LEVEL - 3 : clss = "IMM";     break;
			case MAX_LEVEL - 4 : clss = "IMM";     break;
			case MAX_LEVEL - 5 : clss = "IMM";     break;
			case MAX_LEVEL - 6 : clss = "IMM";     break;
			case MAX_LEVEL - 7 : clss = "IMM";     break;
			case MAX_LEVEL - 8 : clss = "IMM";     break;
			default: break; // leave as is
		}
	}

	// push onto morts new/***/clan prefix titles
	if( wch->level<LEVEL_IMMORTAL){
		if ((IS_NEWBIE_SUPPORT(ch) || IS_IMMORTAL(ch))
			&& IS_NEWBIE(wch))
			clss="new";
		else if (two_column && (is_same_clan(ch,wch)||IS_IMMORTAL(ch))){
			sprintf(whoclass,"%s%s",
				wch->clan->color_str(),
				wch->clan->who_cat());
			clss=whoclass;
		}else{
			clss = "***";
		}
	}

	if ( wch->level>=LEVEL_IMMORTAL){
        strcpy( bracket_col, "`=\x83");
    }else if (IS_IRC(wch)){
        strcpy( bracket_col, "`=\x97");
    }else{
        strcpy( bracket_col, "`=\x96"); 
	}

	// incog level 
	if (IS_IMMORTAL(ch) && get_trust(ch)>= wch->incog_level && wch->incog_level > 0){
		if (two_column){
			if (wch->incog_level == LEVEL_IMMORTAL){
				sprintf(incogtext, "(Ic)");
			}else{
				sprintf(incogtext, "(Ic%d)",wch->incog_level );
			}
		}else{
			if (wch->incog_level == LEVEL_IMMORTAL){
				sprintf(incogtext, " (Incog)");
			}else{
				sprintf(incogtext, " (Incog %d)",wch->incog_level );
			}
		}
	}else{
		incogtext[0]= '\0';
	}


	// wizi level
	wizitext[0]='\0';
	if ( IS_IMMORTAL(ch) )
	{
		char iw[30], ow[30], w[30];

		if(wch->iwizi){
			if(wch->iwizi!=LEVEL_IMMORTAL){
				sprintf(iw,"`#%sI%d`&", IS_OOC(wch)?"`=\x88":"`=\x89", wch->iwizi);
			}else{
				sprintf(iw,"`#%sI`&", IS_OOC(wch)?"`=\x88":"`=\x89");
			}
		}else{
			iw[0]='\0';
		}
		if(wch->owizi){
			if(wch->owizi!=LEVEL_IMMORTAL){
				sprintf(ow,"`#%sO%d`&", IS_OOC(wch)?"`=\x90":"`=\x88", wch->owizi);
			}else{
				sprintf(ow,"`#%sO`&", IS_OOC(wch)?"`=\x90":"`=\x88");
			}
		}else{
			ow[0]='\0';
		}

		if(wch->invis_level){
			if (two_column){
				if (wch->invis_level == LEVEL_IMMORTAL){
					sprintf(w, "Wz");
				}else{
					sprintf(w, "Wz%d", wch->invis_level);
				}
			}else{
				if (wch->invis_level == LEVEL_IMMORTAL){
					sprintf(w, "Wizi");
				}else{
					sprintf(w, "Wizi %d", wch->invis_level);
				}
			}
		}else{
			w[0]='\0';
		}

		
		// generate the wizitext
		sprintf(wizitext,"%s%s%s",iw, ow, w);
		if(!IS_NULLSTR(wizitext)){
			sprintf(wizitext,"(%s%s%s)",iw, ow, w);
		}else{
			wizitext[0]='\0';
		}
	}

	
	//  %colour%[%colour
	sprintf(left_bracket,"%s%s%s",
		bracket_col,
		(	IS_SET(wch->act, PLR_LOG) 
			&& !IS_IMMORTAL(wch)
			&& IS_ADMIN(ch)
		)	? 
			("`#`=\x87L`^") // L instead of [
			:(	IS_SET(wch->comm,COMM_NOCHANNELS) 
				&& (IS_IMMORTAL(ch) 
				|| IS_SET(ch->comm, COMM_CANNOCHANNEL)) 
			 )	?"`#`M!`^" :
			(IS_IMMORTAL(ch) && IS_IMMORTAL(wch) 
				&& !IS_SET(wch->comm, COMM_WHOVIS)) ? "<":"[",
			(wch->level>=LEVEL_IMMORTAL)?"`=\x82":"`=\x81"
		);

	sprintf(right_bracket,"%s%s`#%s",
		bracket_col,
		( IS_SET(wch->comm,COMM_NOCHANNELS) // no channeled bracket
		&& (IS_IMMORTAL(ch) 
		|| IS_SET(ch->comm, COMM_CANNOCHANNEL)) )?"`#`=\x86!`^":
			(IS_IMMORTAL(ch) && IS_IMMORTAL(wch) 
				&& !IS_SET(wch->comm, COMM_WHOVIS)) ? ">":"]",
         (IS_IMMORTAL(ch) && IS_IMMORTAL(wch) 
		 && !IS_SET(wch->comm, COMM_WHOVIS)) ? "`#`=\x91" : "");


	if (two_column){
 // Format it up //%[%*%]#  %new%i%w%b%a%o%O%P%i%h%s%Q%C%l%o%q
	sprintf( buf, "%s%s%s%s%s`^%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		left_bracket,
		clss,
		right_bracket,
        ((IS_IMMORTAL(ch)||HAS_IMMTALK_NAME(ch))? (HAS_IMMTALK_NAME(wch)?"`#`=\x92#`&":" ") :" "),        
		wch->name,
        (IS_IMMORTAL(ch) && (wch->level>=LEVEL_IMMORTAL) 
			&& IS_SET(wch->comm, COMM_NEWBIE_SUPPORT)) ? 
			" (Newbie Support!)" : "",
		incogtext,
		wizitext,
        IS_SET(wch->comm, COMM_BUILDING) ? " [Build]" : "",
		IS_SET(wch->comm, COMM_CODING) ? " [Code]" : "",
        IS_SET(wch->comm, COMM_AFK) ? " `#`=\x84[AFK]`^": "",
         (!IS_IMMORTAL(wch) && IS_OOC(wch))? " `#`=^(in OOC)`^" : "",
		 (!IS_IMMORTAL(wch) && IS_OLCAREA(wch->in_room->area))? " `#`=&(OLC AREA)`&" : "",
         (IS_IMMORTAL(ch) && HAS_CONFIG(wch, CONFIG_ACTIVE))? " `#`=\x93<A>`&" : "",		 
		 (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_INVISIBLE))? " (I)" : "",
         (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_HIDE))?      " (H)"  : "",
         (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_SNEAK))?     " (S)" : "",
         (IS_IMMORTAL(ch) && IS_SET(wch->act, PLR_QUESTER))? " `#`=\x95(" "`=\x94" "Q`=\x95)`&" : "",
		 ((IS_IMMORTAL(ch) || HAS_CONFIG(ch, CONFIG_COURTMEMBER)) && HAS_CONFIG(wch, CONFIG_COURTMEMBER))? " `#`=\x95(" "`=\x94" "T`=\x95)`&" : "",
		 ((IS_IMMORTAL(ch) || HAS_CONFIG(ch, CONFIG_BARD_COUNCIL))&& HAS_CONFIG(wch, CONFIG_BARD_COUNCIL))? " `#`=\x95(" "`=\x94" "B`=\x95)`&" : "",
		 (IS_LINKDEAD(wch)?" `#`=\x85[LD]`^":""),
			!HAS_CHANNELOFF(wch, CHANNEL_QUIET)
			&& HAS_CHANNELOFF(wch, CHANNEL_OOC)? " `#`=O(ooc off)`^" : 
					HAS_CHANNELOFF(wch, CHANNEL_CHAT)? " `#`=O(chat off)`^" : "",
		 HAS_CHANNELOFF(wch, CHANNEL_QUIET)? " `#`=\x84(q)`^" : "",
		 (( IS_IMMORTAL(ch) || IS_NEWBIE_SUPPORT(ch)) && HAS_CONFIG(wch, CONFIG_NONEWBIE )) ? "`#`=E(no new)`^" : ""  );
	}else{
 // Format it up //idl%[%*%]# %new%i%w%b%a%o%O%i%h%s%Q%C%B%l%o%q%newbie
	sprintf( buf, "%s%s%s%s%s`^%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		left_bracket,
		clss,
		right_bracket,
		((IS_IMMORTAL(ch)||HAS_IMMTALK_NAME(ch))? (HAS_IMMTALK_NAME(wch)?"`#`=\x92#`&":" ") :" "),
        name,
        (IS_IMMORTAL(ch) && (wch->level>=LEVEL_IMMORTAL) 
			&& IS_SET(wch->comm, COMM_NEWBIE_SUPPORT)) ? 
			" (Newbie Support!)" : "",
		incogtext,
		wizitext,
        IS_SET(wch->comm, COMM_BUILDING) ? " [Building]" : "",
        IS_SET(wch->comm, COMM_CODING) ? " [Coding]" : "",
        IS_SET(wch->comm, COMM_AFK) ? " `#`=\x84[AFK]`^" : "",
         (!IS_IMMORTAL(wch) && IS_OOC(wch))? " `#`=^(in OOC)`^" : "",
		 (!IS_IMMORTAL(wch) && wch->in_room && IS_OLCAREA(wch->in_room->area))? " `#`=&(OLC AREA)`x" : "",
         (IS_IMMORTAL(ch) && HAS_CONFIG(wch, CONFIG_ACTIVE))? " `#`=\x93<A>`&" : "",		 
         (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_INVISIBLE))? " (I)" : "",
         (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_HIDE))?      " (H)"  : "",
         (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_SNEAK))?     " (S)" : "",
         (IS_IMMORTAL(ch) && IS_SET(wch->act, PLR_QUESTER))? " `#`=\x95(" "`=\x94" "Q`=\x95)`&" : "",
		 ((IS_IMMORTAL(ch) || HAS_CONFIG(ch, CONFIG_COURTMEMBER))&& HAS_CONFIG(wch, CONFIG_COURTMEMBER))? " `#`=\x95(" "`=\x94" "T`=\x95)`&" : "",
		 ((IS_IMMORTAL(ch) || HAS_CONFIG(ch, CONFIG_BARD_COUNCIL))&& HAS_CONFIG(wch, CONFIG_BARD_COUNCIL))? " `#`=\x95(" "`=\x94" "B`=\x95)`&" : "",
		 (IS_LINKDEAD(wch)?" `#`=\x85[LD]`^":""),
			!HAS_CHANNELOFF(wch, CHANNEL_QUIET)
			&& HAS_CHANNELOFF(wch, CHANNEL_OOC)? " `#`=O(ooc off)`^" : 
				HAS_CHANNELOFF(wch, CHANNEL_CHAT)? " `#`=O(chat off)`^" : "",
		 HAS_CHANNELOFF(wch, CHANNEL_QUIET)? " `#`=\x84(quiet)`^" : "",
		 (( IS_IMMORTAL(ch) || IS_NEWBIE_SUPPORT(ch)) && HAS_CONFIG(wch, CONFIG_NONEWBIE )) ? "`#`=E(no new)`^" : ""  );
	}

	if (IS_ADMIN(ch) && HAS_MSP(wch)){
		strcat(buf," `#`=\x95(" "`=\x94" "M`=\x95)`^");
	}
	if (IS_ADMIN(ch) && HAS_MXP(wch)){
		strcat(buf," `#`=\x95(" "`=\x94" "X`=\x95)`^");
	}
#ifdef MCCP_ENABLED
	if (IS_ADMIN(ch) && wch->desc && wch->desc->out_compress){
		strcat(buf," `#`=\x95(" "`=\x94" "C`=\x95)`^");
	}
#endif
	if(IS_ADMIN(ch) && HAS_CONFIG(wch,CONFIG_PRACSYS_TESTER)){
		strcat(buf," `#`=\x095(`=\x094PT`=\x095)`^");
	}
		
    // put on the clan details if required 
    if ( !two_column && wch->clan>0
		&& ( is_same_clan(ch,wch) ||IS_IMMORTAL(ch) )
	   )
    {
		if (IS_IMMORTAL(ch))
		{
			if (c_str_len(buf)>40)
			{
				sprintf( buf2, "%s %s", buf, 
					wch->clan->cwho_name());
			}
			else
			{
				sprintf( buf2, "%s %s", str_width(buf,40,true), 
					wch->clan->cwho_name());
			}
		}
		else
		{
			if (c_str_len(buf)>25)
			{
				sprintf( buf2, "%s %s", buf, wch->clan->cwho_name());					
			}
			else
			{
				sprintf( buf2, "%s %s", str_width(buf,25,true),
					wch->clan->cwho_name());					
			}
		}	
        strcat(return_buf,buf2);
    }
    else
    {
        strcat(return_buf,buf);
    }

	// on the dedicated pkill style muds, show which area each player is in
    if(	GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)		
		&& wch->in_room
		&& wch->level<LEVEL_IMMORTAL 
        && wch->in_room->area 
		&& wch->in_room->area->name)
	{
        sprintf(buf,"`x(%s)", wch->in_room->area->name);
        strcat(return_buf,buf);
    }

	// reset colour back for next line
	strcat(return_buf,"`x");
	return (return_buf);
}
/**************************************************************************/
// storm who format
char *whoformat_storm( char_data *ch, char_data *wch, bool two_column)
{
	static char return_buf[MSL*2];
	char const *clss;
	char align_txt[MIL];
	char immtitle[MIL];
	char title[MIL];
	char level[MIL];
	// -----
	char w0[MIL];
	char w1[MIL];
	char w2[MIL];
	char w3[MIL];
	char w4[MIL];
	char w5[MIL];
	//
	w0[0] = '\0';
	w1[0] = '\0';
	w2[0] = '\0';
	w3[0] = '\0';
	w4[0] = '\0';
	w5[0] = '\0';
	title[0] = '\0';
	immtitle[0] = '\0';
	level[0] = '\0';

	// Ripped right out of whoformat_dawn:
	return_buf[0] = '\0';
	align_txt[0] = '\0';


	// Level, if you're the same clan, or immortal.
	if(IS_IMMORTAL(ch) || is_same_clan(wch,ch)){
		sprintf(level,"`c%s`B%3d`c%s",	
			(IS_IMMORTAL(ch) && IS_IMMORTAL(wch) 
				&& !IS_SET(wch->comm, COMM_WHOVIS)) ? "<":"[",
			wch->level,
			(IS_IMMORTAL(ch) && IS_IMMORTAL(wch) 
				&& !IS_SET(wch->comm, COMM_WHOVIS)) ? ">":"]");
	}else{
			if(ch == wch) 
				sprintf(level,"`G<`B%3d`G>",wch->level);
			else if(IS_NEWBIE(wch) )
				sprintf(level,"`c[`GNeW`c]`x");
			else
				sprintf(level,"`c[   ]`X");
	}
		

	{ //Align stuff
		// get the inner text
		char *t="";
		switch(wch->tendency){
			case 3: case 2:				t="`WLaW"; break;
			case 1: case 0: case -1: 	t="`BBaL"; break;
			case -2: case -3:			t="`RChS"; break;
			default:					t="error"; break;
		}
		// get the inner text
		char oc='x';
		switch(wch->alliance){
			case 3: case 2:				oc='W'; break;
			case 1: case 0: case -1: 	oc='B'; break;
			case -2: case -3:			oc='R'; break;
			default:					t="terr"; break;
		}
		sprintf(align_txt,"`%c`#<%s`^>", oc, t);
	}

	//by graham for immtitle
	char it[MIL];
	
//	if(HAS_CONFIG2(wch,CONFIG2_GUEST)){
	if(false){
		strcpy(it, "`CGUEST`x"); // the default GUEST immtitle 
	}else{
		if(IS_NULLSTR(wch->pcdata->immtitle)){
			strcpy(it, "     "); // the default immtitle for now
		}else{
			strcpy(it, wch->pcdata->immtitle);
		}
	}
	if (wch->level >= LEVEL_IMMORTAL)
	{
		int half;
		int sechalf;
		if (c_str_len(it) == 6){
			sprintf(immtitle, "`C%-6s", it);
		}else{
			half = ((7 - c_str_len(it)) / 2);
			sechalf = (7 - (half + c_str_len(it)));
			sprintf(immtitle, "%*c%s%*c", half,' ',it,sechalf,' ');
		}
	}else if(wch->clan != NULL){
		sprintf(immtitle," %-5s ",wch->clan->cwho_name());
	}else if(wch->level < LEVEL_IMMORTAL){
		sprintf(immtitle," %-5s ",capitalize(race_table[wch->race]->short_name));
	}

	clss="***";	
	// Figure what to Print for Class
	switch(wch->level)
	{
		default:
			if(IS_NEWBIE_SUPPORT(wch) && IS_NEWBIE(ch))
				clss = "`W+++";
			else if(IS_NEWBIE_SUPPORT(ch) && IS_NEWBIE(wch))
				clss = "`G+++";
			else if(IS_IMMORTAL(ch) && IS_NEWBIE(wch))
				clss = "`SNeW";
			else
				clss = class_table[wch->clss].short_name; // temp test
				//clss = "***";
			if(wch == ch)
				clss = class_table[wch->clss].short_name;
			if(IS_IMMORTAL(ch))
				clss = class_table[wch->clss].short_name;
			{
		case MAX_LEVEL + 1: //clss = "***";
			if(wch->pcdata && wch->pcdata->who_text &&
			c_str_len(wch->pcdata->who_text) == 3){
				clss = wch->pcdata->who_text;
				break;
			}else{
				//clss = "***";
				break;
			}
		case MAX_LEVEL - 0: clss = "`SIMP"; break;
		case MAX_LEVEL - 1: clss = "`BCRE"; break;
		case MAX_LEVEL - 2: clss = "`WSUP"; break;
		case MAX_LEVEL - 3: clss = "`WDEI"; break;
		case MAX_LEVEL - 4: clss = "`WGOD"; break;
		case MAX_LEVEL - 5: clss = "`WIMM"; break;
		case MAX_LEVEL - 6: clss = "`WDEM"; break;
		case MAX_LEVEL - 7: clss = "`WANG"; break;
		case MAX_LEVEL - 8: clss = "`WAVA"; break;
		case LEVEL_HERO: clss = "`CHrO"; break;
			}
	}
	
	if(HAS_CHANNELOFF(wch, CHANNEL_QUIET)) strcat(w0,"`GQ");  
	else if IS_SET(wch->comm, COMM_CODING)  strcat(w0,"`GC");  
	else if IS_SET(wch->comm, COMM_BUILDING)  strcat(w0,"`GB");  
	else strcat(w0,"`G-");

	if IS_AFFECTED(wch, AFF_INVISIBLE)  strcat(w1,"`SI");  
	else if IS_AFFECTED(wch, AFF_SNEAK)  strcat(w1,"`SS");  
	else if IS_AFFECTED(wch, AFF_HIDE)  strcat(w1,"`SH");  
	else strcat(w1, "`S-");

	if( IS_KILLER(wch) ) strcat(w2,"`R`FK`x");
	else if( IS_THIEF(wch) ) strcat(w2,"`S`FT`x");
	else if (IS_NEWBIE_SUPPORT(wch) ) strcat(w2,"`SN");
	else if (IS_JAILED(wch) ) strcat(w2,"`CJ");
	else strcat(w2, "`Y-");

	if IS_SET(wch->pcdata->council, COUNCIL_HEADBALANCE)  strcat(w3,"`WB");    // B
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADSUPPORT)  strcat(w3,"`WS");    // S
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADRP)  strcat(w3,"`WP");         // P
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADREALM)  strcat(w3,"`WR");      // R
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADSTORYLINE)  strcat(w3,"`WT");   //T
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADCLAN)  strcat(w3,"`YC");       // N
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADLAW)  strcat(w3,"`WL");        // L
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADMYTHOS)  strcat(w3,"`WY");     // Y
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADCODE)  strcat(w3,"`WC");       // C
	else if IS_SET(wch->pcdata->council, COUNCIL_CODE)  strcat(w3,"`RC");           // C
	else if IS_SET(wch->pcdata->council, COUNCIL_ADMIN)  strcat(w3,"`RA");          // A
	else if IS_SET(wch->pcdata->council, COUNCIL_LAW)  strcat(w3,"`RL");            // L
	else if IS_SET(wch->pcdata->council, COUNCIL_CLAN)  strcat(w3,"`MC");           // N
	else if IS_SET(wch->pcdata->council, COUNCIL_MYTHOS)  strcat(w3,"`RT");			// T
	else if IS_SET(wch->pcdata->council, COUNCIL_REALM)  strcat(w3,"`RR");          // R
	else if IS_SET(wch->pcdata->council, COUNCIL_RP)  strcat(w3,"`RP");             // P
	else if IS_SET(wch->pcdata->council, COUNCIL_SUPPORT)  strcat(w3,"`RS");        // S
	else if IS_SET(wch->pcdata->council, COUNCIL_BALANCE)  strcat(w3,"`RB");        // B
	else strcat(w3, "`R-");

	if (wch->iwizi)  strcat(w4,"`CI");  
	else if (wch->owizi)  strcat(w4,"`CO");  
	else if (wch->invis_level)  strcat(w4,"`CW");  
	else if (wch->incog_level)  strcat(w4,"`RI");  
	else strcat(w4, "`C-");

	if( false){//HAS_CONFIG2(wch,CONFIG2_MUDBALL) ){
//		if(IS_SET(wch->dyn,DYN_MUDBALL_TEAM_RED) && mudball->game_in_progress){
			strcat(w5,"`RM");
//		}else if(IS_SET(wch->dyn,DYN_MUDBALL_TEAM_BLUE) && mudball->game_in_progress){
			strcat(w5,"`BM");
//		}else{
			strcat(w5,"`WM");
//		}
	}else 
	if(IS_SET(wch->act, PLR_QUESTER)){
		strcat(w5,"`BQ");
	}else{
		strcat(w5, "`B-");
	}

	// Title & AFK text
	if IS_SET(wch->comm, COMM_AFK){
		if(IS_NULLSTR(wch->pcdata->afk_message)){
			strcat(title,"`xIs `BAFK`Y!`x");
		}else{
			sprintf(title,"`xIs `BAFK`Y!`x %s ",wch->pcdata->afk_message);
		}
	}else if(!IS_NULLSTR(wch->pcdata->title)){
		strcat(title,wch->pcdata->title);
	}else{
		strcat(title," ");
	}

	sprintf(return_buf,"%s`c[%s`X%s`X%s`c] `c[%s%s%s%s%s%s`c]%s %s `x%s`x",
		level,
		align_txt,
		immtitle,
		clss,
		w0, w1, w2, w3, w4, w5,
		(IS_IMMORTAL(ch) && IS_IMMORTAL(wch) && !IS_SET(wch->comm, COMM_WHOVIS))?"`=\x91":"`x",
		wch->name,
		title);

	// Tada.
	return(return_buf);
}
/**************************************************************************/
//-Whoformat EndlessDreams -Balo-
/**************************************************************************/

char *whoformat_endless( char_data *ch, char_data *wch, bool two_column)
{
	static char return_buf[MSL*2];
	char title[MIL];
	// -----
	char w0[MIL];
	char w1[MIL];
	char w2[MIL];
	char w3[MIL];
	char w4[MIL];
	//
	w0[0] = '\0';
	w1[0] = '\0';
	w2[0] = '\0';
	w3[0] = '\0';
	w4[0] = '\0';
	title[0] = '\0';

	return_buf[0] = '\0';

	
	if(HAS_CHANNELOFF(wch, CHANNEL_QUIET)) strcat(w0,"`GQ");  
	else if IS_SET(wch->comm, COMM_CODING)  strcat(w0,"`GC");  
	else if IS_SET(wch->comm, COMM_BUILDING)  strcat(w0,"`GB");  
	else strcat(w0,"`G-");

	if IS_AFFECTED(wch, AFF_INVISIBLE)  strcat(w1,"`SI");  
	else if IS_AFFECTED(wch, AFF_SNEAK)  strcat(w1,"`SS");  
	else if IS_AFFECTED(wch, AFF_HIDE)  strcat(w1,"`SH");  
	else strcat(w1, "`S-");

	if IS_SET(wch->pcdata->council, COUNCIL_HEADBALANCE)  strcat(w2,"`WB");    // B
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADSUPPORT)  strcat(w2,"`WS");    // S
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADRP)  strcat(w2,"`WP");         // P
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADREALM)  strcat(w2,"`WR");      // R
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADSTORYLINE)  strcat(w2,"`WT");   //T
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADCLAN)  strcat(w2,"`WN");       // N
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADLAW)  strcat(w2,"`WL");        // L
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADMYTHOS)  strcat(w2,"`WY");     // Y
	else if IS_SET(wch->pcdata->council, COUNCIL_HEADCODE)  strcat(w2,"`WC");       // C
	else if IS_SET(wch->pcdata->council, COUNCIL_CODE)  strcat(w2,"`RC");           // C
	else if IS_SET(wch->pcdata->council, COUNCIL_ADMIN)  strcat(w2,"`RA");          // A
	else if IS_SET(wch->pcdata->council, COUNCIL_LAW)  strcat(w2,"`RL");            // L
	else if IS_SET(wch->pcdata->council, COUNCIL_CLAN)  strcat(w2,"`RC");           // N
	else if IS_SET(wch->pcdata->council, COUNCIL_MYTHOS)  strcat(w2,"`RT");			// T
	else if IS_SET(wch->pcdata->council, COUNCIL_REALM)  strcat(w2,"`RR");          // R
	else if IS_SET(wch->pcdata->council, COUNCIL_RP)  strcat(w2,"`RP");             // P
	else if IS_SET(wch->pcdata->council, COUNCIL_SUPPORT)  strcat(w2,"`RS");        // S
	else if IS_SET(wch->pcdata->council, COUNCIL_BALANCE)  strcat(w2,"`RB");        // B
	else if IS_NEWBIE_SUPPORT(wch) strcat(w2, "`SN");	//N
	else strcat(w2, "`R-");

	if (wch->iwizi)  strcat(w3,"`CI");  
	else if (wch->owizi)  strcat(w3,"`CO");  
	else if (wch->invis_level)  strcat(w3,"`CW");  
	else if (wch->incog_level)  strcat(w3,"`RI");  
	else strcat(w3, "`C-");


	if(IS_SET(wch->act, PLR_QUESTER)){
		strcat(w4,"`BQ");
	}else{
		strcat(w4, "`B-");
	}

	// Title & AFK text
	if IS_SET(wch->comm, COMM_AFK){
		if(IS_NULLSTR(wch->pcdata->afk_message)){
			strcat(title,"`xIs `BAFK`Y!`x");
		}else{
			sprintf(title,"`xIs `BAFK`Y!`x %s ",wch->pcdata->afk_message);
		}
	}else if(!IS_NULLSTR(wch->pcdata->title)){
		strcat(title,wch->pcdata->title);
	}else{
		strcat(title," ");
	}

	sprintf(return_buf," `c[%s%s%s%s%s`c]%s %s `x%s`x",

		w0, w1, w2, w3, w4,
		(IS_IMMORTAL(ch) && IS_IMMORTAL(wch) && !IS_SET(wch->comm, COMM_WHOVIS))?"`=\x91":"`x",
		wch->name,
		title);


	return(return_buf);
}
/**************************************************************************/
//-Whoformat Whispers of Times Lost -Ixliam-
/**************************************************************************/
char *whoformat_wotl ( char_data *ch, char_data *wch, bool two_column)
{
	 static char return_buf[MSL*2];
    char const *clss;
	 char bracket_col[10]; // [ colour
    char left_bracket[20]; // L, ! or [ 
    char right_bracket[20]; // ! or ]
    char buf[MSL];
    char buf2[MSL];
    char wizitext[40]; 
	 char incogtext[20];
	 char whoclass[20];

	return_buf[0]='\0'; // clear the initial return string

	char name[MIL];
	{
//	   	if(!IS_NULLSTR( wch->pcdata->pretitle)) {
//			strcpy(name, wch->pcdata->pretitle);
//			strcat(name, FORMATF(" %s", wch->name));
//		}
//		else 
		strcpy(name, wch->name);
		// generate the name displayed
		// put the afk message or title depending on if they are afk
		if(IS_SET(wch->comm, COMM_AFK) && wch->pcdata 
			&& !IS_NULLSTR(wch->pcdata->afk_message))
		{
			strcat(name, FORMATF(" (%s)", wch->pcdata->afk_message));
		}else if(!GAMESETTING3(GAMESET3_WHO_TITLE_DISABLED)){
			// concat the title
			strcat(name, FORMATF(" %s", wch->pcdata->title));
		}
	}

	// Figure out what to print for clss.
	clss="***";
    switch ( wch->level )
    {
		default: clss="***";break;
		case MAX_LEVEL - 0 : clss = "IMP";     break;
		case MAX_LEVEL - 1 : clss = "CRE";     break;
		case MAX_LEVEL - 2 : clss = "SUP";     break;
		case MAX_LEVEL - 3 : clss = "DEI";     break;
		case MAX_LEVEL - 4 : clss = "GUA";     break;
		case MAX_LEVEL - 5 : clss = "IMM";     break;
		case MAX_LEVEL - 6 : clss = "DEM";     break;
		case MAX_LEVEL - 7 : clss = "ANG";     break;
		case MAX_LEVEL - 8 : clss = "AVA";     break;
	}
    
	if(wch->level==ABSOLUTE_MAX_LEVEL && ABSOLUTE_MAX_LEVEL!=MAX_LEVEL){
		if(wch->pcdata && wch->pcdata->who_text && c_str_len(wch->pcdata->who_text)==3){
			clss=wch->pcdata->who_text;
		}else{
			clss="***";
		}
	}

	// alternate 3 tier imm ranking system (IMP, ADM, IMM)
	if(GAMESETTING4(GAMESET4_3TIER_IMMRANKS_IN_WHO)){
		switch ( wch->level )
		{
			case MAX_LEVEL - 0 : clss = "IMP";     break;
			case MAX_LEVEL - 1 : clss = "ADM";     break;
			case MAX_LEVEL - 2 : clss = "ADM";     break;
			case MAX_LEVEL - 3 : clss = "IMM";     break;
			case MAX_LEVEL - 4 : clss = "IMM";     break;
			case MAX_LEVEL - 5 : clss = "IMM";     break;
			case MAX_LEVEL - 6 : clss = "IMM";     break;
			case MAX_LEVEL - 7 : clss = "IMM";     break;
			case MAX_LEVEL - 8 : clss = "IMM";     break;
			default: break; // leave as is
		}
	}

	// push onto morts new/***/clan prefix titles
	if( wch->level<LEVEL_IMMORTAL)
	{
//		if (IS_VAMPIRE(wch) &&
//			(IS_VAMPIRE (ch) || IS_IMMORTAL(ch)
//			|| wch == ch))
//			clss="`#`RVam`^";
//		else
//		if (IS_WEREWOLF(wch) && (IS_WEREWOLF (ch) || IS_IMMORTAL(ch) || wch == ch))
//			clss="`#`CWlf`^";
//		else
		if ((IS_NEWBIE_SUPPORT(ch) || IS_IMMORTAL(ch)) && IS_NEWBIE(wch))
			clss="`#`Gnew`^";
		else
		if (IS_NEWBIE_SUPPORT(wch) && IS_NEWBIE(ch))
			clss="`#`C+++`^";
		else
		if (IS_NEWBIE_SUPPORT(wch) && IS_IMMORTAL(ch))
		{
			clss = class_table[wch->clss].short_name;
			sprintf(whoclass,"`#`G%s`^", clss);
			clss=whoclass;
		}
		else
		if (IS_NEWBIE_SUPPORT(wch) && is_same_clan(ch,wch))
		{
			clss = class_table[wch->clss].short_name;
			sprintf(whoclass,"`#`G%s`^", clss);
			clss=whoclass;
		}
		else
		if (IS_NEWBIE_SUPPORT(wch) && wch==ch)
		{
			clss = class_table[wch->clss].short_name;
			sprintf(whoclass,"`#`S%s`^", clss);
			clss=whoclass;
		}
		else
		if (is_same_clan(ch,wch) || IS_IMMORTAL(ch))
		{
			clss = class_table[wch->clss].short_name;
			sprintf(whoclass,"`#`S%s`^", clss);
			clss=whoclass;
		}
		else
		if (wch==ch)
		{
			clss = class_table[wch->clss].short_name;
			sprintf(whoclass,"`#`S%s`^", clss);
			clss=whoclass;
		}
		else
		if (IS_NEWBIE_SUPPORT(wch) && IS_NEWBIE_SUPPORT(ch))
			clss = "`#`G+++`^";
		else clss = "***";
	}

	if ( wch->level>=LEVEL_IMMORTAL)
	{
        strcpy( bracket_col, "`=\x83");
   }
	else if (IS_IRC(wch))
	{
        strcpy( bracket_col, "`=\x97");
   }
	else
	{
        strcpy( bracket_col, "`=\x96"); 
	}

	// incog level 
	if (IS_IMMORTAL(ch) && get_trust(ch)>= wch->incog_level && wch->incog_level > 0)
	{
		if (wch->incog_level == LEVEL_IMMORTAL)
		{
				sprintf(incogtext, " (Incog)");
		}
		else
		{
			sprintf(incogtext, " (Incog %d)",wch->incog_level );
		}
	}
	else
	{
		incogtext[0]= '\0';
	}

	// wizi level
	wizitext[0]='\0';
	if ( IS_IMMORTAL(ch) )
	{
		char iw[30], ow[30], w[30];

		if(wch->iwizi){
			if(wch->iwizi!=LEVEL_IMMORTAL){
				sprintf(iw,"`#%sI%d`&", IS_OOC(wch)?"`=\x88":"`=\x89", wch->iwizi);
			}else{
				sprintf(iw,"`#%sI`&", IS_OOC(wch)?"`=\x88":"`=\x89");
			}
		}else{
			iw[0]='\0';
		}
		if(wch->owizi){
			if(wch->owizi!=LEVEL_IMMORTAL){
				sprintf(ow,"`#%sO%d`&", IS_OOC(wch)?"`=\x90":"`=\x88", wch->owizi);
			}else{
				sprintf(ow,"`#%sO`&", IS_OOC(wch)?"`=\x90":"`=\x88");
			}
		}else{
			ow[0]='\0';
		}

		if(wch->invis_level)
		{
				if (wch->invis_level == LEVEL_IMMORTAL)
				{
					sprintf(w, "Wizi");
				}
				else
				{
					sprintf(w, "Wizi %d", wch->invis_level);
				}
		}
		else
			{
				w[0]='\0';
			}
                                                      		
		// generate the wizitext
		sprintf(wizitext,"%s%s%s",iw, ow, w);
		if(!IS_NULLSTR(wizitext)){
			sprintf(wizitext,"(%s%s%s)",iw, ow, w);
		}else{
			wizitext[0]='\0';
		}
	}
	
	//  %colour%[%colour
	sprintf(left_bracket,"%s%s%s",
		bracket_col,
		(	IS_SET(wch->act, PLR_LOG) 
			&& !IS_IMMORTAL(wch)
			&& IS_ADMIN(ch)
		)	? 
			("`#`=\x87L`^") // L instead of [
			:(	IS_SET(wch->comm,COMM_NOCHANNELS) 
				&& (IS_IMMORTAL(ch) 
				|| IS_SET(ch->comm, COMM_CANNOCHANNEL)) 
			 )	?"`#`M!`^" :
			(IS_IMMORTAL(ch) && IS_IMMORTAL(wch) 
				&& !IS_SET(wch->comm, COMM_WHOVIS)) ? "<":"[",
			(wch->level>=LEVEL_IMMORTAL)?"`=\x82":"`=\x81"
		);

	sprintf(right_bracket,"%s%s`#%s",
		bracket_col,
		( IS_SET(wch->comm,COMM_NOCHANNELS) // no channeled bracket
		&& (IS_IMMORTAL(ch) 
		|| IS_SET(ch->comm, COMM_CANNOCHANNEL)) )?"`#`=\x86!`^":
			(IS_IMMORTAL(ch) && IS_IMMORTAL(wch) 
				&& !IS_SET(wch->comm, COMM_WHOVIS)) ? ">":"]",
         (IS_IMMORTAL(ch) && IS_IMMORTAL(wch) 
		 && !IS_SET(wch->comm, COMM_WHOVIS)) ? "`#`=\x91" : "");

 // Format it up //idl%[%*%]# %new%i%w%b%a%o%O%i%h%s%Q%C%B%l%o%q%newbie
	sprintf( buf, "%s%s%s%s%s`^%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		left_bracket,
		clss,
		right_bracket,
		((IS_IMMORTAL(ch)||HAS_IMMTALK_NAME(ch))? (HAS_IMMTALK_NAME(wch)?"`#`=\x92#`&":" ") :" "),
        name,
        (IS_IMMORTAL(ch) && (wch->level>=LEVEL_IMMORTAL) && IS_SET(wch->comm, COMM_NEWBIE_SUPPORT)) ? 
			" (Newbie Support!)" : "",
		incogtext,
		wizitext,
        IS_SET(wch->comm, COMM_BUILDING) ? " [Building]" : "",
        IS_SET(wch->comm, COMM_CODING) ? " [Coding]" : "",
        IS_SET(wch->comm, COMM_AFK) ? " `#`=\x84[AFK]`^" : "",
         (!IS_IMMORTAL(wch) && IS_OOC(wch))? " `#`=^(in OOC)`^" : "",
		 (!IS_IMMORTAL(wch) && wch->in_room && IS_OLCAREA(wch->in_room->area))? " `#`=&(OLC AREA)`x" : "",
         (IS_IMMORTAL(ch) && HAS_CONFIG(wch, CONFIG_ACTIVE))? " `#`=\x93<A>`&" : "",		 
         (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_INVISIBLE))? " (I)" : "",
         (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_HIDE))?      " (H)"  : "",
         (IS_IMMORTAL(ch) && IS_AFFECTED(wch, AFF_SNEAK))?     " (S)" : "",
         (IS_IMMORTAL(ch) && IS_SET(wch->act, PLR_QUESTER))? " `#`=\x95(" "`=\x94" "Q`=\x95)`&" : "",
		 ((IS_IMMORTAL(ch) || HAS_CONFIG(ch, CONFIG_COURTMEMBER))&& HAS_CONFIG(wch, CONFIG_COURTMEMBER))? " `#`=\x95(" "`=\x94" "T`=\x95)`&" : "",
		 ((IS_IMMORTAL(ch) || HAS_CONFIG(ch, CONFIG_BARD_COUNCIL))&& HAS_CONFIG(wch, CONFIG_BARD_COUNCIL))? " `#`=\x95(" "`=\x94" "B`=\x95)`&" : "",
		 (IS_LINKDEAD(wch)?" `#`=\x85[LD]`^":""),
			!HAS_CHANNELOFF(wch, CHANNEL_QUIET)
			&& HAS_CHANNELOFF(wch, CHANNEL_OOC)? " `#`=O(ooc off)`^" : 
				HAS_CHANNELOFF(wch, CHANNEL_CHAT)? " `#`=O(chat off)`^" : "",
		 HAS_CHANNELOFF(wch, CHANNEL_QUIET)? " `#`=\x84(quiet)`^" : "",
		 (( IS_IMMORTAL(ch) || IS_NEWBIE_SUPPORT(ch)) && HAS_CONFIG(wch, CONFIG_NONEWBIE )) ? "`#`=E(no new)`^" : ""  );

	
    // put on the clan details if required 
    if (wch->clan>0)
   {
		if( (is_name( "Coven", wch->clan->notename()) && ( is_same_clan(wch, ch) || IS_IMMORTAL(ch))) 
		    || (is_name( "Coven", ch->clan->notename()) && wch == ch ) ) 
		{

			if (c_str_len(buf)>40)
			{
				sprintf( buf2, "%s `#`W[`RT`rhe `RC`roven`W]`^", buf );
			}
			else
			{
				sprintf( buf2, "%s `#`W[`RT`rhe `RC`roven`W]`^", str_width(buf,40,true));
			}
		}else 

		if( (is_name( "MoonShadow", wch->clan->notename()) && ( is_same_clan(wch, ch) || IS_IMMORTAL(ch))) 
		    || (is_name( "MoonShadow", ch->clan->notename()) && wch == ch ) ) 
		{

			if (c_str_len(buf)>40)
			{
				sprintf( buf2, "%s `#`W[`CM`Soon`CS`Shadow`W]`^", buf );
			}
			else
			{
				sprintf( buf2, "%s `#`W[`CM`Soon`CS`Shadow`W]`^", str_width(buf,40,true));
			}
		}else 
			if (c_str_len(buf)>40)
			{
				sprintf( buf2, "%s %s", buf, 
					wch->clan->cwho_name());
			}
			else
			{
				sprintf( buf2, "%s %s", str_width(buf,40,true), 
					wch->clan->cwho_name());
			}

        strcat(return_buf,buf2);
    }
    else
    {
        strcat(return_buf,buf);
    }

	// reset colour back for next line
	strcat(return_buf,"`x");
	return (return_buf);
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

