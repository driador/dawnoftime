/**************************************************************************/
// langedit.cpp - olc based language editor, Kalahn
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "olc.h"
#include "security.h"

extern language_data *languages;
/**************************************************************************/
bool langedit_show(char_data *ch, char *)
{
	language_data *lang;
	EDIT_LANGUAGE(ch,lang);
	SET_BIT(ch->dyn,DYN_SHOWFLAGS);

	ch->titlebarf("LANGUAGE EDIT%s %s", IS_SET(lang->flags, LANGFLAG_CHANGED)?"*":":", uppercase(lang->name));
  	ch->printlnf("`=rName:   `x%s    `=rSkillName: `x%s `=X%-9s   `=rCommandName: `x%s", 
		capitalize(lang->name), capitalize(lang->skillname), lang->gsn==-1?"UNFOUND!":"",
		capitalize(lang->commandname));	
	int flags=lang->flags;
	REMOVE_BIT(lang->flags, LANGFLAG_CHANGED);
	mxp_display_olc_flags_ex(ch, language_flags, lang->flags, "flags", "Flags:", 77, 8 , 8);	
	lang->flags=flags;
	
	ch->titlebar("What The Language Flags Do");
	ch->wrapln("no_scramble      `S- The language isn't scrambled.`x");
	ch->wrapln("no_holyspeech    `S- Holyspeech doesn't make everyone can understand, even if they don't know the language.`x");
	ch->wrapln("no_order         `S- People can't be ordered to change to start speaking in this language.`x");
	ch->wrapln("reverse          `S- Output is backwards if not understood.`x");
	ch->wrapln("no_language_name `S- The name of the language isn't displayed.`x");
	ch->titlebar("BASE WORDMAP");

	{
		// create the wordlist so we can display it, will be deallocated later
		lang->words_recreate_list();

		wordmapping_data *word;
		int count=0;
		int extrawordcount=0;
		size_t len;
		size_t widestto=0;
		size_t widestfrom=0;
		// loop thru doing all single letter mappings
		for(word=lang->words; word; word=word->next){
			len=str_len(word->to);
			widestto=UMAX(len, widestto);
			len=str_len(word->from);
			widestfrom=UMAX(len, widestfrom);
			if(len==1){
				if(count%6!=0){
					ch->print(" `=t|`x");
				}else{
					ch->print(" ");
				}
				ch->printf(" %s -> %s`x", word->from, str_width(word->to,5,false));
				if(++count%6==0){
					ch->println("");
				}
			}else{
				extrawordcount++;
			}
		}
		if(count%6!=0){
			ch->println("");
		}	
		lang->words_deallocate_list();
		ch->println("  'recreatebase' can be used to automatically create a new base wordmap");
		ch->printlnf("  There are %d extra word%s mapped for this language, viewed with 'words'",	
			extrawordcount, extrawordcount==1?"":"s");
		ch->println("  Wordmappings can be manipulated using 'addword' and 'delword'");		
	}

	ch->titlebar("");
	REMOVE_BIT(ch->dyn,DYN_SHOWFLAGS);
    return false;
}
/**************************************************************************/
void language_recreatebase(language_data *lang)
{
	// create an array of all the letters in the alphabet
	char shuffle[26];
	int i;
	for(i=0; i<26; i++){
		shuffle[i]=i+'a';
	}
	// shuffle the array using a system of swapping
	for(i=0; i<25; i++){
		int pos=number_range(i+1, 25);
		char t=shuffle[i];
		shuffle[i]=shuffle[pos];
		shuffle[pos]=t;
	}

	// load the shuffled array as new words
	char from[2],to[2];
	from[1]='\0';
	to[1]='\0';
	for(i=0; i<26; i++){
		from[0]=i+'a';
		to[0]=shuffle[i];
		lang->add_wordmap_to_tree(from, to);
	}
} 
/**************************************************************************/ 
bool langedit_recreatebase(char_data *ch, char *argument)
{	
	char confirm[MIL];
	argument=one_argument(argument, confirm);

	if(IS_NULLSTR(confirm) || str_cmp(confirm, "confirm")){
		ch->println("syntax: recreatebase confirm");
		return false;
	}

	language_data *lang;
	EDIT_LANGUAGE(ch,lang);
		
	language_recreatebase(lang);

	SET_BIT(lang->flags, LANGFLAG_CHANGED);
	langedit_show(ch, "");
	return true;
}

/**************************************************************************/
//	Entry Point for editing the languages
void do_langedit( char_data *ch, char *argument )
{
	int count=0;
	language_data *language;

	if(ch){
		if ( IS_NPC( ch )){
			ch->println("Players only.");
			return;
		}

		// do security checks
		if (!HAS_SECURITY(ch, 2)){
			ch->println( "You must have an olc security 2 or higher to use this command." );
			return;
		}

		// do security checks
		if (!HAS_SECURITY(ch, LANGEDIT_MINSECURITY))
		{
    		ch->printlnf("You must have an olc security %d or higher to use this command.",
				LANGEDIT_MINSECURITY);
			return;
		}

		if ( !IS_TRUSTED(ch, LANGEDIT_MINTRUST)) {
			ch->printlnf("You must have a trust of %d or above "
				"to use this command.", LANGEDIT_MINTRUST);
			return;
		}
	}

	if (IS_NULLSTR(argument)){		
		ch->titlebar("LANGEDIT");
		ch->println("syntax: langedit <language>");
		ch->println("syntax: langedit create <newlanguage>");
		ch->println("syntax: langedit list    - list the languages in a table");
		ch->println("This command takes you into an olc editor, to edit/create languages");
		ch->println("`x");
		ch->println("The following uneditable system languages exist:");
		for(language=languages; language; language=language->next){
			if(IS_SET(language->flags, LANGFLAG_SYSTEM_LANGUAGE)){
				ch->printf("    `S%-24s", language->name);
				if(++count%3==0){
					ch->print_blank_lines(1);
				}
			}
		}
		
		if(count%3!=0){
			ch->print_blank_lines(1);
		}
		count=0;
		ch->println("`x");
		ch->println("The following existing languages can be edited:");
		for(language=languages; language; language=language->next){
			if(!IS_SET(language->flags, LANGFLAG_SYSTEM_LANGUAGE)){
				ch->printf("    `=C%s", mxp_create_send(ch,FORMATF("langedit %s", language->name), FORMATF("%-24s", language->name)));
				if(++count%3==0){
					ch->print_blank_lines(1);
				}
			}
		}
		if(count%3!=0){
			ch->print_blank_lines(1);
		}
		ch->titlebar("");
		return;
	}
	
	char arg[MIL];
	argument=one_argument(argument,arg);

	if(!str_cmp(arg,"create")){
		argument=one_argument(argument,arg);
		if(IS_NULLSTR(arg)){
			ch->println("You must specify the name of the language you wish to create.");
			return;
		}

		// Allow only letters, numbers and underscore.
		int length=str_len(arg);
		for(int i=0; i<length; i++){
			if(!is_alnum(arg[i]) && arg[i]!='_'){
				ch->println( "Only letters, the underscore character '_' and numbers are valid in a language name." );
				return;
			}
		}

		// check if there is an existing language with matching name
		language=language_exact_lookup(arg);
		if(language){
			ch->printlnf("There is already a language named '%s'", language->name);
			return;
		}
		language=new language_data;
		language->name=str_dup(lowercase(arg));
		language->flags=0;
		if(ch){
			language->skillname=str_dup("");
		}else{
			language->skillname=str_dup(language->name);
		}
		language->gsn=skill_exact_lookup(language->skillname);
		language->commandname=str_dup("-");
		language->words=NULL;
		language->initialise_tree();

		// add new language to the front of the list
		language->next=languages;		
		languages=language;
		language_recreatebase(language);

		if(ch){
			ch->printlnf("Language '%s' created", arg);
			ch->desc->pEdit	= (void*)language;
			ch->desc->editor = ED_LANGUAGE;
			langedit_show(ch, "");		
		}
		return;
	}

	if(!str_cmp(arg,"list")){
		for(language=languages; language; language=language->next){
			if(IS_SET(language->flags, LANGFLAG_SYSTEM_LANGUAGE)){
  				ch->printlnf("`=$Name: `x%-18s `=$SkillName: `x%-18s `=$CommandName: `x%s", 
					capitalize(language->name), capitalize(language->skillname), 
					capitalize(language->commandname));				
			}else{
  				ch->printlnf("`=rName: `x%-18s `=rSkillName: `x%-18s `=rCommandName: `x%s", 
					mxp_create_send(ch,FORMATF("langedit %s", language->name), FORMATF("%-18s", capitalize(language->name))),
					capitalize(language->skillname), 
					capitalize(language->commandname));	
			}
		}
		return;
	}

	// find an existing social
	language=language_lookup(arg);
	if(!language){
		ch->printlnf("There is no language named '%s'", arg);
		do_langedit(ch, "");
		return;
	}

	// ensure it isn't a system language
	if(IS_SET(language->flags, LANGFLAG_SYSTEM_LANGUAGE)){
		ch->printlnf("'%s' is a system language, therefore can not be edited.", language->name);
		return;
	}

    ch->desc->pEdit	= (void*)language;
	ch->desc->editor = ED_LANGUAGE;
	ch->printlnf("Editing '%s' language.", language->name);
	langedit_show(ch, "");
	return;
}

/**************************************************************************/
bool langedit_rename(char_data *ch, char *argument)
{
	char arg[MIL];
	argument=one_argument(argument,arg);

    if ( IS_NULLSTR(arg))
    {
		ch->println( "Syntax: rename <string>");
		ch->println( "Rename the language.");
		return false;
    }

	// Allow only letters, numbers and underscore.
	int length=str_len(arg);
	for(int i=0; i<length; i++){
		if(!is_alnum(arg[i]) && arg[i]!='_'){
			ch->println( "Only letters, the underscore character '_' and numbers are valid in a language name." );
			return false;
		}
	}

	language_data *lang;

	// check if there is an existing language with matching name
	lang=language_exact_lookup(arg);
	if(lang){
		ch->printlnf("There is already a language named '%s'", lang->name);
		return false;
	}

	EDIT_LANGUAGE(ch,lang);
    ch->printlnf("Language name changed from '%s' to '%s'.", 
		lang->name, lowercase(arg) );

    replace_string( lang->name, lowercase(arg) );
    return true;
}
/**************************************************************************/
bool langedit_skillname(char_data *ch, char *argument)
{
	char arg[MIL];
	argument=one_argument(argument,arg);

    if ( IS_NULLSTR(arg))
    {
		ch->println( "Syntax: skillname <string>");
		ch->println( "Set/change the skillname of the language.");
		return false;
    }

	language_data *lang;
	EDIT_LANGUAGE(ch,lang);

	// find the skill first
	int sn=skill_exact_lookup(arg);
	if(sn<0){
		ch->printlnf("There is no skill called '%s'", arg);
		return false;
	}
	if(lang->gsn==sn){
		ch->printlnf("Language skillname is already set to %s", lang->skillname);
	}

    ch->printlnf("Language skillname changed from '%s' to '%s'.", 
		lang->skillname, lowercase(arg) );

    replace_string( lang->skillname, lowercase(arg) );
	lang->gsn=sn;
    return true;
}
/**************************************************************************/
bool langedit_commandname(char_data *ch, char *argument)
{
	char arg[MIL];
	argument=one_argument(argument,arg);

    if ( IS_NULLSTR(arg))
    {
		ch->println( "Syntax: commandname <string>");
		ch->println( "Syntax: commandname -         (to clear the command name)");
		ch->println( "Set/change the commandname of the language.");
		ch->println( "Note: if this is blank, the language can't be accessed as a command.");
		return false;
    }

	language_data *lang;
	EDIT_LANGUAGE(ch,lang);

	if(!str_cmp(arg, "-")){
		ch->println("Commandname cleared.");
		replace_string(lang->commandname,"-");
		return true;
	}

    ch->printlnf("Language commandname changed from '%s' to '%s'.", 
		lang->commandname, lowercase(arg) );

    replace_string( lang->commandname, lowercase(arg) );
    return true;
}
/**************************************************************************/
bool langedit_flags(char_data *ch, char *argument)
{
	language_data *lang;
	EDIT_LANGUAGE(ch,lang);
	return olc_generic_flag_toggle(ch, argument,
				"flags", "flags", language_flags, &lang->flags);
}
/**************************************************************************/
// this is inefficient, but the code will very rarely be used
bool langedit_words(char_data *ch, char *argument)
{
	language_data *lang;
	EDIT_LANGUAGE(ch,lang);

	char buf[MSL];
	BUFFER *output;
	output	= new_buf();
	
	// create the wordlist so we can display it, will be deallocated later
	lang->words_recreate_list();

	wordmapping_data *word;
	int count=0;
	int len;
	int widestto=0;
	int widestfrom=0;
	// loop thru doing all single letter mappings
	add_buf( output, format_titlebar(FORMATF("%s WORD MAPPINGS", uppercase(lang->name))));
	for(word=lang->words; word; word=word->next){
		len=str_len(word->to);
		widestto=UMAX(len, widestto);
		len=str_len(word->from);
		widestfrom=UMAX(len, widestfrom);
	}

	count=0;
	for(word=lang->words; word; word=word->next){
		if(!IS_NULLSTR(argument)){ // support infix word matching
			if(str_infix(argument, word->from) 
				&& str_infix(argument, word->to)){
				continue;
			}
		}

		if(str_len(word->from)!=1){
			if(count%3!=0){
				add_buf( output, " `=t|`x");
			}else{
				add_buf( output, " ");
			}
			sprintf(buf, 
				FORMATF(" %%-%ds -> %%-%ds", widestfrom, widestto)				
				,word->from, word->to);
			if(++count%3==0){
				strcat(buf, "\r\n");
			}
			add_buf( output, buf);
		}
	}
	if(count){
		if(count%3!=0){
			add_buf( output, "\r\n");
		}
	}
	add_buf( output, "Add/change word mappings using the addword command\r\n");
	add_buf( output, "You can delete words with delword and infix search this list.\r\n");
	add_buf( output, format_titlebar("="));
	
	ch->sendpage(buf_string(output));
	free_buf(output);

	lang->words_deallocate_list();
    return false;	
}
/**************************************************************************/
bool langedit_addword(char_data *ch, char *argument)
{
	char from[MIL];
	char to[MIL];
	argument=one_argument(argument, from);
	if(str_len(from)==1 && is_alnum(*from) && !str_cmp(argument,"''")){
		// special use case - setting a single character to blank
		to[0]='\0'; // null out the word we are setting to
	}else{
		// typical use case - process the to parameter and length requirements as normal
		argument=one_argument(argument, to);
		if(IS_NULLSTR(to)){
			ch->println("syntax: addword <word_from> <word_to>");
			return false;
		}
	}

	if(str_len(from)==1){
		if(!is_alnum(*from)){
			ch->println("You can't add single character 'words' other than letters and numbers");
			return false;
		}
	}

	language_data *lang;
	EDIT_LANGUAGE(ch,lang);
	languagetree_data *tail;
	// look for an existing word first
	int length=lang->find(from, &tail);

	if(length==str_len(from)){
		if(!str_cmp(to, tail->stored_word)){
			ch->printlnf("No changed required, '%s' is already mapped to '%s'.", 
				from, to);		
			return false;
		}
		ch->printlnf("Modified mapping '%s'->'%s' to '%s'->'%s'", 
			from, tail->stored_word, from, to);		
		lang->add_wordmap_to_tree(from, to);
	}else{
		ch->printlnf("Added new word mapping to '%s'->'%s'", from, to);
		lang->add_wordmap_to_tree(from, to);
	}
	return true;
}
/**************************************************************************/
bool langedit_delword(char_data *ch, char *argument)
{	
	char delword[MIL];
	argument=one_argument(argument, delword);

	if(IS_NULLSTR(delword)){
		ch->println("syntax: delword <word_to_delete>");
		return false;
	}

	if(str_len(delword)==1 && is_alpha(*delword) ){
		ch->wrapln("You can't delete single character 'words' that are letters, "
			"only change them with addword.  To blank a single character "
			"wordmapping use \"addword <word> ''\"");
		return false;
	}

	language_data *lang;
	EDIT_LANGUAGE(ch,lang);
	languagetree_data *tail;
	// first check the delword exists, before trying to remove it
	int length=lang->find(delword, &tail);

	if(length!=str_len(delword)){
		ch->printlnf("Couldn't find any word '%s' to remove.", delword);
		return false;
	}

	// create the wordlist so we can pull it out then reinitialise the tree
	lang->words_recreate_list();

	wordmapping_data *word, *prev;
	prev=NULL;
	bool found=false;
	for(word=lang->words; !found && word; word=word->next){
		if(!str_cmp(delword, word->from)){
			if(prev){
				prev->next=word->next;
			}else{
				// we are the head of the list
				lang->words=word->next;
			}
			// deallocate the memory used by the deleted node
			free_string(word->from);
			free_string(word->to);
			word->next=NULL;
			delete word;

			found=true;
			// we don't have to worry about words_last here, as we call words_deallocate_list() below
		}
		prev=word;
	}

	// it has to be found, because lang->find() matched the word
	assert(found);

	// reinitialise the tree from the wordmap list which has 
	// just had the word we are deleting removed from it
	lang->reinitialise_tree();

	// return the memory
	lang->words_deallocate_list();

	ch->printlnf("Word mapping for '%s' has been removed.", delword);
	return true;
}
/**************************************************************************/
/**************************************************************************/

