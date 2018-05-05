/**************************************************************************/
// letters.cpp - see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 * FILE: letters.cpp - Letter writing system written by Kalahn and Kerenos *
 * ----------------------------------------------------------------------- *
 * The letter system works by having a buffer associated with the player   *
 * in which the player typed in the contents of a letter into a letter     *
 * workspace.  The text contained within the workspace can be scribed to   *
 * an actual parchment once completed.                                     *
 ***************************************************************************/
#include "include.h"

/**************************************************************************/
// core user interface of letter writting system 
// Syntax:  
//			letter show		- shows what the final letter will look like
//			letter edit		- brings up the text of the letter in the string editor
//			letter +		- adds a single line of text to a letter workspace
//			letter -		- deletes a single line of text from a letter workspace
//			letter clear	- clears the letter
//			letter peek     - shows original and language letter (immort)
//			letter scribe	- copys the letter text from the workspace to a parchment
//			letter seal		- puts a seal on a letter
struct	letter_cmd_type
{
    const char * name;
    DO_FUN	*do_fun;
	int flags;
	const char * syntax;
};
/**************************************************************************/
#define CMDLETTER_IMMORTAL_ONLY		(A)	// Imm only command
#define CMDLETTER_TEXT_REQUIRED		(B)	// must have text (prevent scribing empty letters)
#define CMDLETTER_AWAKE				(C)	// must be awake to use this command
/**************************************************************************/
char *letter_generate_scroll( const char *header, const char *text );
DECLARE_DO_FUN(letter_addline);
DECLARE_DO_FUN(letter_delline);
DECLARE_DO_FUN(letter_breakseal);
DECLARE_DO_FUN(letter_clear);
DECLARE_DO_FUN(letter_edit);
DECLARE_DO_FUN(letter_peek);
DECLARE_DO_FUN(letter_scribe);
DECLARE_DO_FUN(letter_seal);
DECLARE_DO_FUN(letter_showscroll);
DECLARE_DO_FUN(letter_showtext);
DECLARE_DO_FUN(letter_tear);
/**************************************************************************/
struct letter_cmd_type letter_cmd_table[]=
{
	{	"+",		letter_addline,	0,"Add a single line of text to the letter."},
	{	"-",		letter_delline,	CMDLETTER_TEXT_REQUIRED, "Remove a single line of text from the letter."},
	{	"breakseal",letter_breakseal,CMDLETTER_AWAKE,"Breaks the seal on a letter - required to read a sealed letter"},
	{	"clear",	letter_clear,	CMDLETTER_TEXT_REQUIRED, "Clears the letter currently in progress."},
	{	"edit",		letter_edit,	0, "Edits the current letter text in the string editor."},
	{	"peek",		letter_peek,	CMDLETTER_IMMORTAL_ONLY, "Peeks at a letter data (immortal only)."},
	{	"scribe",	letter_scribe,	CMDLETTER_TEXT_REQUIRED|CMDLETTER_AWAKE, "Scribes the working letter onto a parchment."},
	{	"seal",		letter_seal,	CMDLETTER_AWAKE,"Seals the letter with wax"},
	{	"showscroll",letter_showscroll,	CMDLETTER_TEXT_REQUIRED, "Shows what the letter will look like once posted (in a scroll picture)."},
	{	"showtext",	letter_showtext,	CMDLETTER_TEXT_REQUIRED, "Shows the text of the letter you are working on."},
	{	"tear",		letter_tear,	0,"Tear up and destroy a letter you are currently holding."},	
	{	"",		NULL, 0, ""}
};
/**************************************************************************/
// letter writing interpreter - Kal
void do_letter( char_data *ch, char *argument )
{
	char arg[MIL];
	int i;

	// PC's only
	if ( IS_NPC( ch ))
    {
		ch->println( "Only players can write letters." );
		return;
    }

	argument=one_argument(argument,arg);
	if(IS_NULLSTR(arg)){ // no args - show syntax
		ch->titlebar("Letter writing");
		ch->println("Syntax:");
		for(i=0;!IS_NULLSTR(letter_cmd_table[i].name); i++){
			if(IS_SET(letter_cmd_table[i].flags, CMDLETTER_IMMORTAL_ONLY) && !IS_IMMORTAL(ch)){
				continue;
			}
			ch->printlnf("  letter %-12s - %s", letter_cmd_table[i].name, letter_cmd_table[i].syntax);
		}
		ch->titlebar("");
		return;
	}

	// find the command
	for(i=0;!IS_NULLSTR(letter_cmd_table[i].name); i++){
		if(IS_SET(letter_cmd_table[i].flags, CMDLETTER_IMMORTAL_ONLY) && !IS_IMMORTAL(ch)){
			continue;
		}
		if(!str_prefix(arg,letter_cmd_table[i].name)){
			// found the command
			break;
		}
	}
	if(IS_NULLSTR(letter_cmd_table[i].name)){
		ch->printlnf("Unrecognised IC letter writting command '%s'", arg);
		do_letter(ch,"");
		return;
	}

	// check if the command can be used in the current situation
	if(		IS_SET(letter_cmd_table[i].flags, CMDLETTER_TEXT_REQUIRED) 
		&&	IS_NULLSTR(ch->pcdata->letter_workspace_text)){
		ch->printlnf("No text currently entered for the letter - no point in using %s at this stage.",
			letter_cmd_table[i].name);
		return;
	}
	if(		IS_SET(letter_cmd_table[i].flags, CMDLETTER_AWAKE) 
		&&	!IS_AWAKE(ch)){
		ch->printlnf("You must be awake to use 'letter %s'", letter_cmd_table[i].name);
		return;
	}

	// run the command
	(*letter_cmd_table[i].do_fun) (ch, argument);
}

/**************************************************************************/
void letter_edit( char_data *ch, char *)
{
	ch->println("Editing the text of your letter.");
	string_append(ch, &ch->pcdata->letter_workspace_text);
}    
/**************************************************************************/
void letter_showtext( char_data *ch, char *)
{
	ch->println("Letter Text:");
	ch->wrapln(ch->pcdata->letter_workspace_text);
}    

/**************************************************************************/
void letter_showscroll( char_data *ch, char *)
{
	char header[MIL];
	sprintf(header, "`YThis letter will be written in %s script.", ch->language->name);
	ch->sendpage(letter_generate_scroll(header, ch->pcdata->letter_workspace_text));
}    

/**************************************************************************/
char *getline( char *str, char *buf, int bufsize );
/**************************************************************************/
char *letter_generate_scroll( const char * header, const char *text)
{
	static char buf[HSL];
	char bufline[MSL], scroll_line[MSL];
	char *ftext = str_dup(text);
	ftext=note_format_string_width(ftext, 61, true, true);
	char *string;


	if(IS_NULLSTR(header)){
		// scroll header with no header
		strcpy( buf, "`y    ___________________________________________________________________\r\n"
				"   /                                                               //_\\\\\r\n"
				"  |                                                               |\\\\_//|\r\n"
				"  |                                                               |_\\__/\r\n"
				"  |                                                               |\r\n" );
	}else{
		// scroll header with header 
		sprintf( buf, "`y    ___________________________________________________________________\r\n"
				"   /                                                               //_\\\\\r\n"
				"  |                                                               |\\\\_//|\r\n"
				"  | %s `y|_\\__/\r\n"				
				"  |                                                               |\r\n",
			str_width(header,61,true));
	}

	// the below converts "`1" into a "\n", but not "``1" into a "`\n"
	ftext=string_replace_all(ftext,"``","_KAL'S_H@CK_"); // :)
	ftext=string_replace_all(ftext,"`1","\n");  
	ftext=string_replace_all(ftext,"_KAL'S_H@CK_", "``"); 

	strcat( buf, "`x`#`#"); // default text colour to white to start with

	// loop recursively creating message body text
	string=ftext;
	while ( *string )
	{
		string = getline( string, bufline, MSL);
		sprintf(scroll_line,"`y  |`& %s`#`y|\r\n", str_width(bufline,62,true));
		strcat( buf, scroll_line);
	}
	
	// scroll footer
	strcat(buf,	"`y  |                                                               |\r\n"
				"  |                                                               |\r\n"
				"  |_______________________________________________________________|\r\n"
				" /                                                            //_\\|\r\n" 
				"|                                                             |\\_/|\r\n"
				" \\_____________________________________________________________\\_/`x\r\n" ); 

	free_string(ftext);
	return buf;
}
/**************************************************************************/
void letter_addline(char_data *ch, char *argument)
{
	char **backup=ch->desc->pString; 
	// prob don't even need to use the backup since how are they going to 
	// get here and be in an editor at the same time.
	ch->desc->pString=&ch->pcdata->letter_workspace_text;
	if(TRUE_CH(ch)->pcdata){
		PREFERENCE_TYPE backup_pref=TRUE_CH(ch)->pcdata->preference_dawnftp;
		TRUE_CH(ch)->pcdata->preference_dawnftp=PREF_OFF;
		string_add(ch,argument);	// add our text 
		TRUE_CH(ch)->pcdata->preference_dawnftp=backup_pref;
	}else{
		string_add(ch,argument);	// add our text 
	}
	ch->desc->pString=backup;

	// check the length
	int len=str_len(ch->pcdata->letter_workspace_text)-(MSL*2);
	if(len>0){
		ch->printlnf("`RWarning:`YThe text of your letter is %d character%s too long to be scribed!!!`x",
			len, len==1?"":"s");
	}else{
		len*=-1;
		if(len>MSL){
			if(len<=1){
				ch->wraplnf("Text added to letter, no more room to add any more to the text "
					"before it it scribed to a parchment.");
			}else{
				ch->wraplnf("Text added to letter, up to %d more characters can be added to the letter "
					"before it it scribed to a parchment.",	len);
			}
		}else{
			ch->println("Text added to letter");
		}
	}
}

/**************************************************************************/
void letter_delline(char_data *ch, char *)
{
	char **backup=ch->desc->pString; 
	ch->desc->pString=&ch->pcdata->letter_workspace_text;
	string_add(ch,".-");	// get rid of the bottom line
	ch->desc->pString=backup;

	// check the length
	int len=str_len(ch->pcdata->letter_workspace_text)-(MSL*2);
	if(len>0){
		ch->printlnf("`RWarning:`YThe text of your letter is still too long to be scribed!!! (by %d character%s)`x",
			len, len==1?"":"s");
	}else{
		len*=-1;
		if(len>MSL){
			if(len<=1){
				ch->wraplnf("Bottom line removed - This letter can now be scribed to a parchment.");				
			}else{
				ch->wraplnf("Bottom line removed - up to %d more characters can fit before scribing.", len);				
			}
		}else{
			ch->println("Text removed from letter");
		}
	}
}

/**************************************************************************/
void letter_clear(char_data *ch, char *)
{
	replace_string(ch->pcdata->letter_workspace_text,"");
	ch->println("Letter in progress cleared");
}
/**************************************************************************/
// prefix an extended to the extended description given - Kal April 00
EXTRA_DESCR_DATA *ed_prefix(EXTRA_DESCR_DATA *ed, char *keyword, char *description)
{
	EXTRA_DESCR_DATA * new_ed	= new_extra_descr();
	new_ed->keyword				= str_dup( keyword );
	new_ed->description			= str_dup( description );
	new_ed->next				= ed;
	return new_ed;
}

/**************************************************************************/
// scribe a working letter onto a parchment
void letter_scribe(char_data *ch, char *)
{

	if(IS_OOC(ch)){
		ch->println("You can't scribe letters while in OOC!`x");
		return;
	}

	// length check
	if(str_len(ch->pcdata->letter_workspace_text)>MSL*2){
		ch->println("`RYou have too much information in your letter to fit on a single parchment.`x");
	}

	obj_data *parchment;	

	// check to see if they are holding a parchment type item
	parchment = get_eq_char( ch, WEAR_HOLD );

	if ( !parchment || parchment->item_type != ITEM_PARCHMENT)
	{
		ch->println( "You need to be holding some parchment to scribe a letter." );
		return;
	}

	// if parchment->value[1]  (v1) is not 0, then the parchement isn't blank
	if ( parchment->value[1] != 0 )
    {
		act( "$p has already been written on.", ch, parchment, NULL, TO_CHAR );
		return;
    }

	if ( get_skill( TRUE_CH(ch), ch->language->gsn)<50)
	{
		ch->printlnf( "Your command of the %s language is insufficient.",
			ch->language->name );
		return;
	}

	// BECAUSE DO_LOOK() REDIRECTS LOOKING AT EXTENDED DESCRIPTIONS TO LETTER_READ
	// WE CAN ENCODE ALL OUR INFO ON THE EXTENDED DESCRIPTIONS OF THE LETTER.
	
	char lbuf[HSL];
	parchment->extra_descr=NULL;

	// _SCRIBED_BY
	sprintf(lbuf, "%s (%s) %d", ch->short_descr, ch->name, ch->vnum());
	parchment->extra_descr=ed_prefix(parchment->extra_descr, "_SCRIBED_BY", lbuf);

	// _SCRIBED_TIME
	sprintf(lbuf, "%s", (char *) ctime( &current_time ));
	parchment->extra_descr=ed_prefix(parchment->extra_descr, "_SCRIBED_TIME", lbuf);

	// _ORIGINAL_TEXT
	parchment->extra_descr=ed_prefix(parchment->extra_descr, "_ORIGINAL_TEXT", 
												ch->pcdata->letter_workspace_text);

	// _LANGUAGE_NAME
	parchment->extra_descr=ed_prefix(parchment->extra_descr, "_LANGUAGE_NAME", ch->language->name);

	// _LANGUAGE_TEXT
	translate_language( ch->language, false, ch, ch, ch->pcdata->letter_workspace_text, lbuf );
	parchment->extra_descr=ed_prefix(parchment->extra_descr, "_LANGUAGE_TEXT", lbuf);

	// clear the writters buffer
	replace_string(ch->pcdata->letter_workspace_text,"");

	// adjust the flags
	// Flag the letter as being written  [v1 = 0 is a blank parchment]
	parchment->value[1] = 1;
	// Make sure it's not flagged sealed [v2 = 0 is an unsealed parchment ]
	parchment->value[2] = 0;

	// v3 is the language the note is written in
	parchment->value[3] = ch->language->unique_id;

	// Language the note is written in	 [v4 = language skill] 
	parchment->value[4] = ch->get_skill(ch->language->gsn);

	// restring shorts and longs
	replace_string(parchment->description, "A hand-written letter is here.");
	replace_string(parchment->short_descr, "a hand-written letter");
	replace_string(parchment->name, "hand-written letter");
 
	ch->println( "You scribe your letter to your parchment." );
	act( "$n pens a letter.", ch, NULL, NULL, TO_ROOM );
	WAIT_STATE( ch, PULSE_PER_SECOND * number_range( 2, 3 ));
	return; 
}

/**************************************************************************/
void letter_seal(char_data *ch, char *argument)
{
	obj_data *letter;

	letter = get_obj_list( ch, argument, ch->carrying );

	if ( !letter )
	{
		ch->println( "What do you want to seal?" );
		return;
	}

	if ( letter->item_type != ITEM_PARCHMENT )
	{
		ch->println( "You can't seal that, it must be a written letter." );
		return;
    }

	// v2 > 0 == already sealed
	if (letter->value[2] != 0)
	{
		ch->println( "It is already sealed." );
		return;
	}

	letter->value[2] = 1;
	
	// restring it again
	if(IS_NULLSTR( ch->pcdata->crest )){
		replace_string(letter->description, "A letter is here, sealed with a blob of red wax.");
		replace_string(letter->short_descr,	"a letter sealed with a blob of red wax");
		replace_string(letter->name, "letter sealed blob red wax");
		act( "$n seals a letter with a red blob of wax.", ch, NULL, NULL, TO_ROOM );
		ch->println( "You seal the letter with a blob of red wax." );
	}else{
		char buf[MIL];
		sprintf(buf, "A letter is here, sealed with a crest depicting %s.", ch->pcdata->crest);
		replace_string(letter->description, buf);
		sprintf(buf,"a letter sealed with a crest depicting %s", ch->pcdata->crest);
		replace_string(letter->short_descr, buf);
		sprintf(buf, "letter sealed crest depicting %s", ch->pcdata->crest);
		replace_string(letter->name, buf);

		sprintf(buf,"$n seals a letter with a crest depicting %s", ch->pcdata->crest);
		act( buf, ch, NULL, NULL, TO_ROOM );
		ch->printlnf( "You seal the letter with a crest depicting %s.", ch->pcdata->crest);
	}
	WAIT_STATE( ch, PULSE_PER_SECOND * number_range( 3, 5 ));
}

/**************************************************************************/
void letter_breakseal(char_data *ch, char *argument)
{
	obj_data *letter;
	
	letter = get_obj_list(ch,argument,ch->carrying);
	
	if ( !letter )
	{
		ch->println( "What do you want to break the seal on?" );
		return;
	}
	
	if ( letter->item_type != ITEM_PARCHMENT )
	{
		ch->println( "You can't break the seal on that." );
		return;
	}
	
	if ( letter->value[2] == 0 )
	{
		ch->println( "The seal on that has already been broken." );
		return;
	}
	
	// [v2 = 0 is an unsealed parchment ]
	letter->value[2] = 0;

	if(str_infix(letter->description, "crest")){
		// first case when red blob of wax
		replace_string(letter->description, "A letter is here, with a broken blob of red wax.");
		replace_string(letter->short_descr,	"a letter with a broken blob of red wax");
		replace_string(letter->name, "letter broken blob red wax");
	}else{
		letter->description=string_replace(letter->description,	
			"sealed with a crest", "with a broken crest");
		letter->short_descr=string_replace(letter->short_descr, 
			"sealed with a crest depicting","with a broken crest depicting");
		letter->name=string_replace(letter->name, "sealed","broken");
	}
	ch->println( "You break the seal on the letter." );
	act("$n breaks the seal on a letter.", ch, NULL, NULL, TO_ROOM );
}

/**************************************************************************/
void letter_tear( char_data *ch, char *argument )
{
	obj_data *letter;

	letter = get_obj_list(ch,argument,ch->carrying);

	if ( !letter )
	{
		ch->println( "What do you want to tear apart?" );
		return;
	}

	if ( letter->item_type != ITEM_PARCHMENT )
	{
		ch->println( "You can only tear up hand written letters." );
		return;
	}

	act( "You tear $p into a thousand pieces.", ch, letter, NULL, TO_CHAR );
	act( "$n tears $p into a thousand pieces.", ch, letter, NULL, TO_ROOM );
	extract_obj( letter );
	return;
}

/**************************************************************************/
void letter_read( char_data *ch, obj_data *letter )
{
	char	lbuf[MSL];

	if (( letter->value[2] != 0))
	{
		ch->println( "The letter is sealed." );
		return;
	}

	if ( letter->value[1] == 0 )
	{
		ch->println( "It is blank." );
		return;
	}

	char *pdesc=get_extra_descr("_LANGUAGE_TEXT", letter->extra_descr);
	if(!pdesc){
		ch->println( "There is nothing written there." );
		return;
	};

	translate_language(language_safe_lookup_by_id(letter->value[3]), false, ch, 
						ch, pdesc, lbuf);

	char header[MIL];
		
	sprintf(header, "`SYou unroll the letter and read in %s script.", 
		language_safe_lookup_by_id(letter->value[3])->name);
	
	ch->sendpage(letter_generate_scroll(header, lbuf));

	act("$n reads $p.",ch, letter, NULL, TO_ROOM);
	WAIT_STATE( ch, PULSE_PER_SECOND * number_range( 3, 5 ));
}
/**************************************************************************/
void letter_peek( char_data *ch, char *argument )
{
	obj_data *letter;

	letter = get_obj_list(ch,argument,ch->carrying);

	if ( !letter )
	{
		ch->println( "What do you want to peek at?" );
		return;
	}

	if ( letter->item_type != ITEM_PARCHMENT )
	{
		ch->println( "You can only peek at hand written letters." );
		return;
	}

	if ( !letter->extra_descr || IS_NULLSTR(letter->extra_descr->description ))
	{
		ch->println( "There is nothing written there." );
		return;
	}

	char	lbuf[HSL];
	char	fbuf[MSL];

	lbuf[0]='\0';
	char *pdesc;

	// SCRIBED BY 
	pdesc=get_extra_descr("_SCRIBED_BY", letter->extra_descr);
	if(pdesc){
		sprintf(fbuf,"SCRIBED BY: %s\r\n", pdesc);
		strcat(lbuf,fbuf);
	};

	pdesc=get_extra_descr("_SCRIBED_TIME", letter->extra_descr);
	if(pdesc){
		sprintf(fbuf,"SCRIBED TIME: %s\r\n", pdesc);
		strcat(lbuf,fbuf);
	};

	pdesc=get_extra_descr("_ORIGINAL_TEXT", letter->extra_descr);
	if(pdesc){
		sprintf(fbuf,"Original Text:\r\n%s\r\n", pdesc);
		strcat(lbuf,fbuf);
	};

	pdesc=get_extra_descr("_LANGUAGE_TEXT", letter->extra_descr);
	if(pdesc){
		sprintf(fbuf,"Language Text (%s):\r\n%s\r\n", 
			language_safe_lookup_by_id(letter->value[3])->name,
			pdesc);
		strcat(lbuf,fbuf);
	};

	if(IS_NULLSTR(lbuf)){
		ch->println("This letter is empty.");
	}else{
		ch->sendpage(lbuf);
	}
}
/**************************************************************************/
