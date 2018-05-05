/**************************************************************************/
// hedit.cpp - OLC based help editor, Kal.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h" // dawn standard includes

#include "olc.h"
#include "security.h"
#include "help.h"

/**************************************************************************/
bool hedit_show(char_data *ch, char *)
{
    help_data *pHelp;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The hedit_show command is an olc command, you don't have olc permissions.");
		return false;
	}

    EDIT_HELP(ch, pHelp);

	if(IS_SET(ch->dyn,DYN_SHOWFLAGS)){
		help_display_to_char(pHelp, ch);
		return false;
	}

	SET_BIT(ch->dyn,DYN_SHOWFLAGS);

	ch->println("`s=============================================================================`x");    
	
    ch->printlnf("`=rTitle: `x%s", pHelp->title);
    ch->printlnf("`=rCommand Reference: `x%s", pHelp->command_reference);
    ch->printlnf("`=rKeywords: `x%s", pHelp->keyword);
	mxp_display_olc_flags(ch, help_category_types,	pHelp->category, "category", "Category:");
	ch->printlnf("`=rContinues: `=_%s`x", pHelp->continues);
    ch->printlnf("`=rParent Help: `=_%s`x", pHelp->parent_help);
	
	// see also
    ch->print("`=rSee Also: ");
	if(!IS_NULLSTR(pHelp->see_also)){
		char *p=pHelp->see_also;
		char pword[MIL];
		while(*p){
			p=first_arg(p, pword, false);
			if(has_space(pword)){
				ch->printf(" `=_'%s'", pword);
			}else{
				ch->printf(" `=_%s", pword);
			}
		}
	}
	ch->println("");

	// imm see also
    ch->print("`=rImm See Also: ");
	if(!IS_NULLSTR(pHelp->immsee_also)){
		char *p=pHelp->immsee_also;
		char pword[MIL];
		while(*p){
			p=first_arg(p, pword, false);
			if(has_space(pword)){
				ch->printf(" `=_'%s'", pword);
			}else{
				ch->printf(" `=_%s", pword);
			}
		}
	}

	ch->println("");
	
	if(!IS_NULLSTR(pHelp->spell_name)){
		ch->printlnf("`=rSpell Name: `x%s", pHelp->spell_name);
	}
	
	// display the level
    ch->printf("`=rLevel: `x%d", pHelp->level);
	if(HAS_MXP(ch)){
		ch->printf(" `S%s  %s",
			mxp_create_send(ch, "level 0"),
			mxp_create_send(ch, FORMATF("level %d", LEVEL_IMMORTAL)) );
	}
	ch->println("");

	// display the assigned editor value
    ch->printf("`=rAssigned Editor: `x%d", pHelp->assigned_editor);
	if(HAS_MXP(ch)){
		ch->print(" [");
		for(int ai=0; ai<10; ai++){
			if(pHelp->assigned_editor==ai){
				ch->print("`C");
			}else{
				ch->print("`S");
			}
			ch->print(mxp_create_send(ch, FORMATF("sa assign %d", ai), FORMATF("%d",ai)));
			if(ai!=9){
				ch->print(" ");
			}
		}
		ch->print("`S]");
	}
	ch->println("");
	
	ch->printlnf("`=rHelpfile: `x%s (%d)", 
			pHelp->helpfile? 
				mxp_create_send(ch, FORMATF("hlist %s", pHelp->helpfile->file_name), 
						FORMATF("%s", pHelp->helpfile->file_name))
				: "`RNo Helpfile!!!`x",
			pHelp->helpfile? pHelp->helpfile->vnum:0);
	

	mxp_display_olc_flags(ch, help_flags,	pHelp->flags, "flags", "Flags:");

	ch->printlnf("`=%cWidest Line: %d (%d character%s).", 
		(pHelp->widest_line_width<79?'r':'X'),
		pHelp->widest_line,
		pHelp->widest_line_width,
		pHelp->widest_line_width==1?"":"s");

    ch->printlnf("`=r%s (%d bytes)`=R", mxp_create_send(ch, "text", "Text:"), str_len(pHelp->text));
	if(str_len(pHelp->text)>200){
		ch->printlnf("%.200s`Y...`x", pHelp->text[0]=='.'?pHelp->text+1:pHelp->text);
	}else{
		ch->printlnf("%s`x", pHelp->text[0]=='.'?pHelp->text+1:pHelp->text);
	}

	if(pHelp->next){
		ch->printlnf("`s%s(`=_%s)`x", 
			mxp_create_send(ch, "EditNextHelp", "EditNextHelp"), 
			lowercase(pHelp->next->keyword));
	}

	ch->println("`s=============================================================================`x");
	
	REMOVE_BIT(ch->dyn,DYN_SHOWFLAGS);
	return false;
}
/**************************************************************************/
void do_hedit( char_data *ch, char *argument )
{
    helpfile_data *pHelpFD;
	help_data *pHelp;
    char arg[MIL];
    char *all_but_first_parameter;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The hedit command is an olc command, you don't have olc permissions.");
		return;
	}

	if (!HAS_SECURITY(ch,6))
	{
		ch->println("Security 6 is required to use hedit.");
		return;
	}

	if (IS_NULLSTR(argument)){
        ch->println("syntax: hedit <helpfile_keyword>");
        ch->println("        hedit create <helpfile_to_store_new_entry_in>");
        ch->println("");
        ch->println(" note: <helpfile_to_store_new_entry_in> can be founding using hlist");
		return;
	}

	// check if we have a create option
	all_but_first_parameter= one_argument(argument,arg);
	if ( !str_cmp( arg, "create" ) && IS_TRUSTED(ch,DEMI))
	{
		// get the helpfile name 
		pHelpFD=helpfile_get_by_filename(all_but_first_parameter);
		if (pHelpFD)
		{		
			pHelp = help_allocate_new_entry();
			pHelp->helpfile = pHelpFD;
			pHelp->helpfile->entries++; // increment the entries in the helpfile
			SET_BIT( pHelp->helpfile->flags, HELPFILE_CHANGED);				
			pHelp->level    = 92;
			pHelp->text = str_dup("");
			sprintf(arg,"new_entry_%s", ch->name);
			pHelp->keyword  = str_dup(arg);
			
			// add it into the linked list
			if ( !help_first){
				help_first = pHelp;
				help_first->prev=NULL;
			}
			if ( help_last){
				help_last->next = pHelp;
				pHelp->prev		= help_last;
			}
			help_last       = pHelp;
			pHelp->next     = NULL;
			top_help++;
			help_init_quicklookup_table(); // we need to recreate the lookup hash
			
			// made the help entry put into edit mode
			ch->desc->pEdit = (void *)pHelp;
			ch->desc->editor = ED_HELP;
			return;
		}
		else
		{
			ch->println("You have to specify the helpfile which the entry is to be stored in.");
			ch->println("e.g. `=Chedit create commands`x");
			ch->println("Use `=Chlist`x for a list of all the helpfiles.");
			return;
		}
		return;		
	}// end of create option

	if(IS_TRUSTED(ch, MAX_LEVEL)){ // max level can edit all helps
		int oldtrust=ch->trust;
		ch->trust=MAX_LEVEL+10;

		// try exact match first
		pHelp=help_get_by_keyword(argument, ch, true);
		if(!pHelp){
			pHelp=help_get_by_keyword(argument, ch, false);
		}

		ch->trust=oldtrust;
	}else{
		// try exact match first
		pHelp=help_get_by_keyword(argument, ch, true);
		if(!pHelp){
			pHelp=help_get_by_keyword(argument, ch, false);
		}
	}

	if(pHelp){
		// found the help entry put into edit mode
		ch->desc->pEdit = (void *)pHelp;
		ch->desc->editor = ED_HELP;

		// inform the builder they are now in hedit mode
		ch->wraplnf("`=rYou are now editing help entry: '`r%s`=r' from: `Y%s`x", 
				pHelp->keyword, pHelp->helpfile->file_name);
		ch->println("`=rType `=Cdone`=r to finish editing.");   
		hedit_show(ch,"");
		return;
    }

    ch->printlnf("Sorry, no help on the keyword '%s' was found to edit.", argument);
    return;
}
/**************************************************************************/
bool hedit_level (char_data *ch, char *argument)
{
    help_data *pHelp;

    EDIT_HELP(ch, pHelp);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        ch->println("Syntax:  level [number]");
		return false;
    }

	int value=atoi( argument );
	if(value<0 || value>ABSOLUTE_MAX_LEVEL){
		ch->printlnf("The level of a help entry must be between 0 and %d.", ABSOLUTE_MAX_LEVEL);
		return false;
	}
	if(value==pHelp->level){
		ch->printlnf("This help entry is already level %d.", value);
		return false;
	}

    ch->printlnf( "Help level changed from %d set to %d.\r\n", pHelp->level, value);
    pHelp->level = value;
    return true;
}

/**************************************************************************/
bool hedit_text(char_data *ch, char *argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

	REMOVE_BIT(pHelp->flags,HELP_WORDWRAPPED);
    if ( IS_NULLSTR(argument))
    {
		if (str_len(pHelp->text)>MSL-20)
		{
            ch->wrapln("To long to be edited with olc... "
				"split help entry manually first. "
				"(or type either `=Ctext clear`x to clear the whole entry or "
				"`=Ctext trim`x to chop off the tail end of the help)");
			return false;
		}
		else
		{		
			replace_string(pHelp->undo_edittext, pHelp->text);
			string_append(ch, &pHelp->text);
			// setup the changed flags to ensure the help entry is
			// autosaved even if they are in the description editor
			// during an autosave.
			if(ch->desc && pHelp->helpfile){
				ch->desc->changed_flag=&pHelp->helpfile->flags;
			}
			replace_string(pHelp->last_editor, ch->name);
			pHelp->last_editdate = current_time;
			return true;
		}
    }

	if(!str_cmp(argument,"clear")){
		free_string(pHelp->text);
		pHelp->text= str_dup( "" );
	    ch->println("Help text cleared!");
	}else if(!str_cmp(argument,"trim")){
		if (str_len(pHelp->text)<=(MSL-20)){
			ch->wrapln("The 'text trim' command can only be used when the "
				"text is too long it can't be edited!");
		}else{
			char t=pHelp->text[MSL-30];
			pHelp->text[MSL-30]='\0';
			char *trimmedtext=str_dup(pHelp->text);
			pHelp->text[MSL-30]=t;
			free_string(pHelp->text);
			pHelp->text= trimmedtext;
			ch->println("Help text trimmed!");
		}
	}else{
	    ch->println(" Syntax: text       - to edit the text in the description editor.");
	    ch->println(" Syntax: text clear - to delete all the text.");
	    ch->println(" Syntax: text trim  - to trim the text so it can be edited.");
	}
	get_widest_line_stats(pHelp->text, true, &pHelp->widest_line, &pHelp->widest_line_width);
    return false;
}

/**************************************************************************/
bool hedit_addkey(char_data *ch, char *argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

	char keyword[MSL*2];

	if ( IS_NULLSTR(argument))
	{
        ch->println("Syntax:  addkey <keyword>");
		return false;
	}

	// invalid character checks
	if(!str_infix("`", argument)){
		ch->println("You can't have the ```` colour code in a help reference.");
		return false;
	}
	if(!str_infix(",", argument)){
		ch->println("You can't have the , character in a help reference.");
		return false;
	}

	sprintf(keyword,"%s %s", pHelp->keyword,argument);

	if (str_len(keyword)>MSL-20)
	{
        ch->println("The keyword would be too long if you added that.");
		return false;
	}

	// convert to uppercase
	for (int i=0; keyword[i]; i++)
	{
		keyword[i]=UPPER(keyword[i]);
	}

	free_string(pHelp->keyword);
	pHelp->keyword=str_dup(keyword);

    ch->printlnf("Keywords are now %s.", pHelp->keyword);

	help_init_quicklookup_table();// we need to recreate the lookup hashtable
    return true;
}

/**************************************************************************/
bool hedit_rekey(char_data *ch, char *argument)
{
	char keyword[MSL*2];

    help_data *pHelp;

    EDIT_HELP(ch, pHelp);

	if ( IS_NULLSTR(argument))
	{
        ch->println("Syntax: rekey <keywords>");
        ch->println("  (replace all the current keywords with the new ones.)");
		return false;
	}

	// invalid character checks
	if(!str_infix("`", argument)){
		ch->println("You can't have the ```` colour code in a help reference.");
		return false;
	}
	if(!str_infix(",", argument)){
		ch->println("You can't have the , character in a help reference.");
		return false;
	}

	strcpy(keyword, argument);

	// convert to uppercase
	for (int i=0; keyword[i]; i++)
	{
		keyword[i]=UPPER(keyword[i]);
	}

    ch->printlnf("Keywords changed from %s to %s.", 
		pHelp->keyword, keyword);
	free_string(pHelp->keyword);
	pHelp->keyword=str_dup(keyword);

	help_init_quicklookup_table();// we need to recreate the lookup hashtable
    return true;
}

/**************************************************************************/
bool hedit_file(char_data *ch, char *argument)
{
    helpfile_data *pHelpFD;
    help_data *pHelp;

    EDIT_HELP(ch, pHelp);

	if ( IS_NULLSTR(argument))
	{
        ch->println("Syntax:  file <keyword>");
        ch->println("  (changes the file the help entry is stored in.)");
		return false;
	}

	pHelpFD=helpfile_get_by_filename(argument);

	if (pHelpFD)
	{
		if (pHelpFD==pHelp->helpfile)
		{
            ch->printlnf("Help entry already stored in %s",
				pHelp->helpfile->file_name);
			return false;
		}
		// change the help entry
		pHelp->helpfile->entries--; // decrement the entries in the old file
		SET_BIT( pHelp->helpfile->flags, HELPFILE_CHANGED);
		pHelp->helpfile = pHelpFD;
		pHelp->helpfile->entries++; // increment the entries in the newfile
		SET_BIT( pHelp->helpfile->flags, HELPFILE_CHANGED);
        ch->printlnf("Help entry now stored in %s",pHelp->helpfile->file_name);
		return false;
	}
	else
	{
        ch->println("`#Couldn't find the name of the new helpfile to store the");
        ch->println("help entry in.  Use `=Chlist`^ to find a name");
		return false;
	}

    return true;
}
/**************************************************************************/
bool hedit_flags(char_data *ch, char *argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);
	int		  value;


    if ( IS_NULLSTR(argument))
    {
		show_olc_options(ch, help_flags, "flags", "help", pHelp->flags);
		return false;
    }

	if (( value = flag_value( help_flags, argument )) != NO_FLAG )
	{
	    TOGGLE_BIT( pHelp->flags, value);
        ch->println("Help flag toggled.");
	    return true;
	}

    return false;
}
/**************************************************************************/
bool hedit_removehelp(char_data *ch, char *argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( IS_NULLSTR(argument))
    {
        ch->println( "Syntax:  removehelp confirm");
		ch->wrapln(  "This flags the helpfile for removal, next time the help is "
			"autosaved, the help entry will not be saved in the file - effectively "
			"removed after the next reboot.  Helplist shows entries marked for removal.");
		ch->println( "Syntax:  removehelp restore");
		ch->println(" restore can be used to mark the helpfile to be saved next autosave.");
		return false;
    }


	if ( !str_cmp("restore", argument)) {
		if(IS_SET(pHelp->flags, HELP_REMOVEHELP)){
			REMOVE_BIT(pHelp->flags, HELP_REMOVEHELP);
			SET_BIT( pHelp->helpfile->flags, HELPFILE_CHANGED);
			do_hsave(NULL,"");
			ch->println( "Help entry will no longer be removed.");				
			return true;
		}
		ch->println( "This help entry has not been flagged for removal.");
		return false;
	}

	if ( !str_cmp("confirm", argument)) {
		char buf[MSL];
		SET_BIT(pHelp->flags, HELP_REMOVEHELP);
		ch->println( "This help entry has been flagged for removal.");
		ch->println( "next hotreboot this help will no longer exist, use 'removehelp restore' to undo this effect.");
		sprintf(buf, "%s flagged help entry '%s' for removal, level %d, flags '%s'\ntext:", 
			TRUE_CH(ch)->name, pHelp->keyword, pHelp->level, flag_string( help_flags, pHelp->flags));
		log_string(buf);		
		append_string_to_file( REMOVEDHELP_FILE, buf, true);
		append_string_to_file( REMOVEDHELP_FILE, pHelp->text, true);
		return true;
	}
	hedit_removehelp(ch,"");
    return false;
}
/**************************************************************************/
bool hedit_category( char_data *ch, char * argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);
	
	int value;

	if(IS_NULLSTR(argument)){
		show_olc_options(ch, help_category_types, "category", "help category", pHelp->category);
		return false;
	}

    if ( ( value = flag_value( help_category_types, argument) ) != NO_FLAG )
    {
        pHelp->category= value;
        ch->printlnf( "Help category set to %s.", 
			flag_string(help_category_types, pHelp->category) );
        return true;
    }
	return false;
}
/**************************************************************************/
bool hedit_parenthelp( char_data *ch, char * argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  parenthelp (string)" );
		ch->println( "Syntax:  parenthelp -  (to clear)" );
		return false;
    }

	if(!str_infix("`", argument)){
		ch->println("You can't have the ```` colour code in a help reference.");
		return false;
	}

	if(!str_cmp(argument,"-")){
		ch->printlnf("Parent help cleared (was '%s')", pHelp->parent_help);
		replace_string( pHelp->parent_help, "");
		return true;
	}

	argument=uppercase(rtrim_string(argument));
	if(!help_get_by_keyword(argument, NULL, false)){
		ch->printlnf("`RWARNING:`x couldn't find any help entry to match '%s'", argument);
	}

	ch->printlnf( "Parent help changed from '%s' to '%s'.", pHelp->parent_help, argument);
    replace_string( pHelp->parent_help, argument);
	return true;
}
/**************************************************************************/
bool hedit_seealso( char_data *ch, char * argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  seealso (string)" );
		ch->println( "Syntax:  seealso -  (to clear)" );
		return false;
    }

	if(!str_infix("`", argument)){
		ch->println("You can't have the ```` colour code in a help reference.");
		return false;
	}
	if(!str_infix(",", argument)){
		ch->println("All comma characters are automatically stripped from the seealso words.");
	}
	
	if(!str_cmp(argument,"-")){
		ch->printlnf("SeeAlso cleared (was '%s')", pHelp->see_also);
		replace_string( pHelp->see_also, "");
		return true;
	}

	argument=uppercase(rtrim_string(argument));
	ch->printlnf( "See_also changed from '%s' to '%s'.", pHelp->see_also, argument);
    replace_string( pHelp->see_also, argument);
	pHelp->see_also=string_replace_all(pHelp->see_also, ",", "");
    return true;
}
/**************************************************************************/
bool hedit_immseealso( char_data *ch, char * argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  immseealso (string)" );
		ch->println( "Syntax:  immseealso -  (to clear)" );
		return false;
    }

	// invalid character checks
	if(!str_infix("`", argument)){
		ch->println("You can't have the ```` colour code in a help reference.");
		return false;
	}
	if(!str_infix(",", argument)){
		ch->println("All comma characters are automatically stripped from the immseealso words.");
	}

	if(!str_cmp(argument,"-")){
		ch->printlnf("ImmSeeAlso cleared (was '%s')", pHelp->immsee_also);
		replace_string( pHelp->immsee_also, "");
		return true;
	}

	argument=uppercase(rtrim_string(argument));
	ch->printlnf( "ImmSeeAlso changed from '%s' to '%s'.", pHelp->immsee_also, argument);
    replace_string( pHelp->immsee_also, argument);
    pHelp->immsee_also=string_replace_all(pHelp->immsee_also, ",", "");
    return true;
}
/**************************************************************************/
bool hedit_spellname( char_data *ch, char * argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);
	int sn;

    if ( IS_NULLSTR(argument)){
		ch->println( "Syntax:  spellname (string)" );
		ch->println( "Syntax:  spellname -  (to clear)" );
		return false;
    }

	sn=spell_lookup(argument);
	if(sn<0){
		ch->printlnf("There is no spell called '%s'", argument);
		return false;
	}

	ch->printlnf("Spell name changed from '%s' to '%s'", 
		pHelp->spell_name,
		skill_table[sn].name);
	replace_string(pHelp->spell_name, skill_table[sn].name);

    return true;
}

/**************************************************************************/
bool hedit_title( char_data *ch, char * argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  title (string)" );
		ch->println( "Syntax:  title -  (to clear)" );
		return false;
    }

	argument=rtrim_string(argument);

	if(!str_cmp(argument,"-")){
		ch->printlnf("Help Title cleared (was '%s')", pHelp->title);
		replace_string( pHelp->title, "");
		return true;
	}

	ch->printlnf( "Help Title changed from '%s' to '%s'.", pHelp->title, argument);
    replace_string( pHelp->title, argument);
    return true;
}
/**************************************************************************/
bool hedit_commandref( char_data *ch, char * argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  commandref (string)" );
		ch->println( "Syntax:  commandref -  (to clear)" );
		ch->println( "");
		ch->println( "type `=Ccommands`x to see the hedit commands");
		return false;
    }

	argument=ltrim_string(rtrim_string(lowercase(argument)));

	if(!str_cmp(argument,"-")){
		ch->printlnf("Help command reference cleared (was '%s')", pHelp->command_reference);
		replace_string( pHelp->command_reference, "");
		return true;
	}

	ch->printlnf( "Help command reference changed from '%s' to '%s'.", pHelp->command_reference, argument);
    replace_string( pHelp->command_reference, argument);
    return true;
}
/**************************************************************************/
bool hedit_continues( char_data *ch, char * argument)
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( IS_NULLSTR(argument))
    {
		ch->println( "Syntax:  continues (string)" );
		ch->println( "Syntax:  continues  -  (to clear)" );
		ch->println( "This is used to link to a help file that continues with another.");
		return false;
    }

	if(!str_infix("`", argument)){
		ch->println("You can't have the ```` colour code in a help reference.");
		return false;
	}

	if(!str_cmp(argument,"-")){
		ch->printlnf("Continues help cleared (was '%s')", pHelp->continues);
		replace_string( pHelp->continues, "");
		return true;
	}

	argument=uppercase(rtrim_string(argument));
	if(!help_get_by_keyword(argument, NULL, false)){
		ch->printlnf("`RWARNING:`x couldn't find any help entry to match '%s'", argument);
	}

	ch->printlnf( "Continues help changed from '%s' to '%s'.", pHelp->continues, argument);
    replace_string( pHelp->continues, argument);
    return true;
}
/**************************************************************************/
bool hedit_wraptext( char_data *ch, char *argument )
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

	if(IS_NULLSTR(argument) || str_cmp(argument,"confirm")){
		ch->println("`=lSyntax: `=Cwraptext confirm`x");
		ch->wrapln("This will wordwrap the text of the help entry, if you don't like it you "
			"can use `=Cundowrap`x to revert a backup made just before wrapping.");
		return false;
	}

	replace_string(pHelp->undo_wraptext, pHelp->text);
	pHelp->text= note_format_string( pHelp->text);
	get_widest_line_stats(pHelp->text, true, &pHelp->widest_line, &pHelp->widest_line_width);
	SET_BIT(pHelp->flags,HELP_WORDWRAPPED);
   
	ch->println("Help text wordwrapped, you can use undowrap to revert to previous text layout.");
    return true;
}
/**************************************************************************/
bool hedit_undowrap( char_data *ch, char *argument )
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

	if(IS_NULLSTR(argument) || str_cmp(argument,"confirm")){
		ch->println("`=lSyntax: `=Cundowrap confirm`x");
		ch->wrapln("This reverts the text to a backup made just before last wordwrapping.");
		return false;
	}

	if(IS_NULLSTR(pHelp->undo_wraptext)){
		ch->println("hedit_undowrap(): sorry there is no unwrap text backup available.");
		return false;
	}

	replace_string(pHelp->text, pHelp->undo_wraptext);
	get_widest_line_stats(pHelp->text, true, &pHelp->widest_line, &pHelp->widest_line_width);
	REMOVE_BIT(pHelp->flags,HELP_WORDWRAPPED);

	ch->println("Text reverted to just before last wraptext.");
    return true;
}
/**************************************************************************/
bool hedit_undotext( char_data *ch, char *argument )
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

	if(IS_NULLSTR(argument) || str_cmp(argument,"confirm")){
		ch->println("`=lSyntax: `=Cundotext confirm`x");
		ch->wrapln("This reverts the text to a backup made just before "
			"the last time you entered the text editor.");
		return false;
	}

	if(IS_NULLSTR(pHelp->undo_edittext)){
		ch->println("hedit_undotext(): sorry there is no text backup available.");
		return false;
	}

	replace_string(pHelp->text, pHelp->undo_edittext);
	get_widest_line_stats(pHelp->text, true, &pHelp->widest_line, &pHelp->widest_line_width);
	REMOVE_BIT(pHelp->flags,HELP_WORDWRAPPED);

	ch->println("Text reverted to just before last text edit.");
    return true;
}
/**************************************************************************/
bool hedit_assigneditor( char_data *ch, char *argument )
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

	if(IS_NULLSTR(argument) || !is_number(argument)){
		ch->println("`=lSyntax: `=Cassigneditor #`x");
		ch->wrapln("This assigns an editor to the help entry which can "
			"then be searched for later.");
		return false;
	}
	int value=atoi(argument);

	ch->printlnf("Editor changed from %d to %d.", pHelp->assigned_editor, value);
	pHelp->assigned_editor=value;
    return true;
}
/**************************************************************************/
bool hedit_convertlinecodes( char_data *ch, char *argument )
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( IS_NULLSTR(argument))
    {
        ch->println( "Syntax:  convertlinecodes confirm");
		ch->println( "This command converts ``1 line codes into ``+.");
		ch->wrapln("``+ is a line code, which is only converted to a new line "
			"by the string wrapping code.  It is silently ignored by the "
			"colour system.  The reason for using ``+ within helps is that "
			"it can be difficult using dawnftp to work with help entries in external "
			"editors that do not treat ``1 as a newline.");
		return false;
    }

	if ( str_cmp("confirm", argument)) {
		hedit_convertlinecodes(ch, "");
		return false;
	}
	int count=0;

	// hide all ``1 codes from the replace of `1 codes
	while(!str_infix("``1", pHelp->text)){
		pHelp->text= string_replace( pHelp->text, "``1", "!@#$%%^%$#@!");
	}		
	while(!str_infix("`1", pHelp->text)){
		pHelp->text=string_replace( pHelp->text, "`1", "`+\n");
		count++;
	}
	// return all original ``1 codes 
    while(!str_infix("!@#$%%^%$#@!", pHelp->text)){
		pHelp->text=string_replace( pHelp->text, "!@#$%%^%$#@!", "``1");
	}

	if(count==0){
		ch->println( "Couldn't find any ``1 code in string to replace.");
	}else{
        ch->printlnf("Replaced %d ``1 symbols with ``+\\n",count);
	}
    return true;
}
/**************************************************************************/
bool hedit_editnexthelp( char_data *ch, char *argument )
{
    help_data *pHelp;
    EDIT_HELP(ch, pHelp);

	if(!pHelp->next){
		ch->printlnf("There is no help file after 'help `=_%s'.", pHelp->keyword);
		return false;
	}

	do_help(ch, pHelp->next->keyword);

	ch->printlnf("Moving editing from 'help `=_%s' to 'help `=_%s'.", 
		pHelp->keyword,
		pHelp->next->keyword);

	pHelp=pHelp->next;
	ch->desc->pEdit = (void *)pHelp;

	hedit_show(ch,"");

	return false;	
}
/**************************************************************************/
/**************************************************************************/
