/**************************************************************************/
// help.cpp - dawn help system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "help.h"
#include "msp.h"
#include "olc_ex.h"

#include "colour.h"

help_data *help_first;
help_data *help_last;
helpfile_data *helpfile_first;
helpfile_data *helpfile_last;

// prototypes
void colour_convert_helps( helpfile_data *pHelpfile );

/**************************************************************************/
GIO_START(help_data)
GIO_STR(keyword)
GIO_STR(title)
GIO_STR(command_reference)
GIO_SHINT(level)
GIO_WFLAGH(flags, "helpflags ", help_flags)
GIO_WFLAGH(category, "category ", help_category_types)
GIO_STR(parent_help)
GIO_STR(see_also)
GIO_STR(immsee_also)
GIO_STR(spell_name)
GIO_STR(continues)
GIO_STR(text)
GIO_STR(last_editor)
GIO_LONG(last_editdate) 
GIO_INT(assigned_editor)
GIO_FINISH_STRDUP_EMPTY

GIO_START(helpfile_data)
GIO_CHAR(colourcode)
GIO_STR(title)
GIO_STR(editors)
GIO_SHINT(security)

GIO_FINISH_STRDUP_EMPTY

#define HELP_QUICKLOOKUP_HASH (83)
class help_quicklookup_data
{
public:
	help_data *pHelp;
	help_quicklookup_data *next;
	~help_quicklookup_data() {if(next) delete next;};
};

help_quicklookup_data *help_quicklookup_table[HELP_QUICKLOOKUP_HASH];
/**************************************************************************/
void get_widest_line_stats(const char *text, bool ignore_colour_codes, 
						   sh_int *line_number, sh_int *line_width)
{
	// idiot check
	if(!text){
		*line_number=0;
		*line_width=0;
		return;
	}

	const char*p=text;

	sh_int max_width=0;
	sh_int max_line=0;
	sh_int current_line=1;
	sh_int current_width=0;
	for( ;!IS_NULLSTR(p); p++){

		if(*p=='\r'){ 
			continue;
		}

		if(*p=='\n'){
			if(current_width>max_width){
				max_width=current_width;
				max_line=current_line;
			}
			current_line++;
			current_width=0;
			continue;
		}

		// dont count the width of colour codes
		// doesn't take into account if a new line occurs before 
		// a colour code is completed
		if(ignore_colour_codes && *p=='`'){
			if(*(p+1)=='='){
				current_width-=3; // '`' '=' and custom colour code
			}else{
				current_width-=2; // '`' and colour code
			}
		}
		current_width++;
	}

	// end of the text, lets check if the bottom line is the longest
	if(current_width>max_width){
		max_width=current_width;
		max_line=current_line;
	}

	*line_number=max_line;
	*line_width=max_width;
}

/**************************************************************************/
// display the history - debugging code - not enabled by default
void do_helphistory( char_data *ch, char *argument )
{
	//return;
	// make sure we have a pc_data - used to record/display history
	pc_data*p;
	p=ch?TRUE_CH(ch)->pcdata:NULL;
	if(!p){
		ch->println("players only sorry");
		return;
	}

	int i;
	for(i=0; i<MAX_HELP_HISTORY; i++){
		ch->printlnf("%2d] %s", i, p->help_history[i]);

	}
	ch->printlnf("p->help_history_index=%d",p->help_history_index);
	ch->printlnf("p->help_next_count=%d", p->help_next_count);

}
/**************************************************************************/
help_data *help_allocate_new_entry( void )
{
	help_data *pHelp;
	static help_data help_zero;
	pHelp= (help_data *)alloc_perm( sizeof(*pHelp) );
	*pHelp = help_zero; 

	pHelp->title		=str_dup("");
	pHelp->parent_help	=str_dup("");
	pHelp->see_also		=str_dup("");
	pHelp->immsee_also	=str_dup("");
	pHelp->spell_name	=str_dup("");
	pHelp->continues	=str_dup("");
	pHelp->keyword		=str_dup("");
	pHelp->text			=str_dup("");
	pHelp->undo_edittext=str_dup("");
	pHelp->undo_wraptext=str_dup("");
	pHelp->command_reference=str_dup("");
    return pHelp;
}
/**************************************************************************/
helpfile_data *helpfile_allocate_new_entry( void )
{
	helpfile_data *pHelpFD;
	static helpfile_data helpFD_zero;
    pHelpFD=(helpfile_data *)alloc_perm( sizeof(*pHelpFD) );
	*pHelpFD = helpFD_zero; 
	pHelpFD->colourcode='{'; // oldstyle colour codes for now
    return pHelpFD;
}

/**************************************************************************/
bool is_keyword_delimiter( char c)
{
    if ( c == '\'' || c == '"' || c == '%'  || c == '(' || c == '`'){
		return true;
	}
	return false;
}
/**************************************************************************/
// finds what can be a valid keyword from the input... stores it into 
// keyword and then returns the last character inputted into the keyword.
char *help_find_keyword( char *input, char *keyword, char_data *looker)
{
    char cEnd;
	bool matched=false;
	char *keyword_start=keyword;

	// trim all leading whitespace
    while ( is_space(*input) ){
		input++;
	}

	// check if the keyword starts with a multiword delimiter 
	// which will means we need to look for it at the end of the word
    cEnd = ' '; // when we see this character, it marks the end of the keyword
    if ( is_keyword_delimiter(*input) && *input!='`' ){
        if ( *input == '(' ){
			// a keyword is surrounded in brackets
			// e.g. `=_(TWO WORDS)
			// the ) will mark the end of the keyword
            cEnd = ')';
            input++;
        }else {
			cEnd = *input++;
		}
    }

	// loop through until we find the end of the keyword
	// or the end of the input
    while ( *input != '\0' )
    {
		// check for all the terminating conditions (ending the keyword)
        if( (is_space(cEnd) && is_space(*input)) // any white space is considered the same
			|| *input== cEnd // the input is the end marker
			|| *input== '`' // backtick marks the end
			|| *input== ',' // comma marks the end
			|| *input== '\n' // new line marks the end
			|| (  // if the end of keyword delimiter is a space, and an alternative delimiter appears, end the keyword
			    cEnd==' ' && (is_keyword_delimiter(*input) || *input==')') 
			   ) 
		   )
		{
            break;
        }
		*keyword++ = *input++;
		matched=true;
    }
    *keyword = '\0';
	
	if(matched && is_space(cEnd)){
		input--; // the last matching character
	}
	if(*input=='`'){
		input--; // we had a colour code
	}

	// if we have a help keyword ending with a . and there is
	// no matching help entry for it, reverse by one character.
	keyword--;
	if(*keyword=='.' && str_len(keyword_start)>0 
		&& !help_get_by_keyword(keyword_start, looker, true))
	{
		*keyword='\0';
		input--;
	}
    return input;
}
/**************************************************************************/
// returns a helpfile found by its name
helpfile_data *helpfile_get_by_filename( char * argument)
{
    helpfile_data *pHelpFD;
	// get the helpfile name
	if (IS_NULLSTR(argument))
		return NULL;

	if(str_suffix(".txt",argument))
	{
		strcat(argument,".txt");
	}

	for ( pHelpFD = helpfile_first; pHelpFD; pHelpFD = pHelpFD->next )
	{  
		if (!str_cmp( argument, pHelpFD->file_name) )
		{		
			return pHelpFD;
		}
	}
	return NULL;		
}
/**************************************************************************/
void help_init_quicklookup_table()
{
	int i;
	for(i=0; i<HELP_QUICKLOOKUP_HASH; i++){
		if(help_quicklookup_table[i]){
			delete help_quicklookup_table[i];
			help_quicklookup_table[i]=NULL;
		}
	}

	// loop thru adding all the helps
	for(help_data *pHelp=help_last; pHelp; pHelp=pHelp->prev){
		help_quicklookup_data *node;
		char keyword[MIL];
		char *p=pHelp->keyword;
		while(*p){
			p=one_argument(p, keyword); // forces lowercase
			if(keyword[0] && keyword[1] && keyword[2]){ // words 3 characters and longer are indexed
				int hashkey=keyword[0] + keyword[1] + keyword[2];
				hashkey%=HELP_QUICKLOOKUP_HASH;
				node=new help_quicklookup_data;
				node->pHelp=pHelp;
				node->next=help_quicklookup_table[hashkey];
				help_quicklookup_table[hashkey]=node;
			}
		}	
	}
}
/**************************************************************************/
// - Kal
void save_helpfile_NAFF( helpfile_data *pHelpfile )
{
	FILE *fp;
    fclose( fpReserve );

	char name_without_path[MIL];
	if(count_char(pHelpfile->file_name,'/')){
		strcpy(name_without_path, strrchr(pHelpfile->file_name, '/')+1);
	}else{
		strcpy(name_without_path,pHelpfile->file_name);
	}
	char newfilename[MIL];


	sprintf(newfilename, "%s%s.save", BACKUP_HELP_DIR, name_without_path);
    if ( !( fp = fopen( newfilename, "w" ) ) )
    {
        bugf("save_helpfile_NAFF(): fopen '%s' for write - error %d (%s)",
			newfilename, errno, strerror( errno));
		exit_error( 1 , "save_helpfile_NAFF", "write error");
    }

	fprintf(fp, "#HELPFILEDATA\n"); 
	GIO_SAVE_RECORD(helpfile_data, pHelpfile, fp, NULL);

	fprintf(fp, "#HELPENTRIES\n"); 
    for( help_data *pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
		if (pHelp->helpfile==pHelpfile && !IS_SET(pHelp->flags, HELP_REMOVEHELP))
		{
			if(GAMESETTING2(GAMESET2_DONT_SAVE_LASTEDITORS)){
				replace_string(pHelp->last_editor,"");
				pHelp->last_editdate=0;
			}
			GIO_SAVE_RECORD(help_data, pHelp, fp, NULL);
		}
	}
	fprintf(fp, "EOF~\n"); // mark the end of a GIO based help section

	fprintf(fp, "#$\n"); // mark the end of the file for boot_db();
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

	// rename help/*.txt to bak_help/*.txt.old, 
	// then bak_help/*.txt.save to help/*.txt
	{
		char old_filename[MIL];
		sprintf(old_filename, "%s%s.old", BACKUP_HELP_DIR, name_without_path);
		char filename[MIL];
		sprintf(filename, "%s%s", HELP_DIR, pHelpfile->file_name);
#ifdef WIN32
		unlink(old_filename);
#endif
		if(rename(filename,old_filename)!=0){
			bugf("An error occurred renaming '%s' to '%s'!.. exiting to avoid helpfile corruption.", 
				filename, old_filename);
			exit_error( 1 , "save_helpfile_NAFF", "error occurred while renaming old helpfile");
		}
		if(rename(newfilename, filename)!=0){
			bugf("An error occurred renaming '%s' to '%s'!.. exiting to avoid helpfile corruption.", 
				newfilename, filename);
			exit_error( 1 , "save_helpfile_NAFF", "error occurred while renaming new helpfile");
		}		
	}

    return;
}
/**************************************************************************/
extern char	strArea[MIL];
/**************************************************************************/
// - Kal
void load_helpfile_NAFF( FILE *fp)
{
	helpfile_data *pHelpfile=helpfile_allocate_new_entry();
	GIO_LOAD_RECORD(helpfile_data, pHelpfile, fp);
	pHelpfile->file_name = str_dup((char *) &strArea[str_len(HELP_DIR)]);
	pHelpfile->vnum=top_helpfile;

	fread_word(fp); //	#HELPENTRIES
    help_data *pHelp;
	for(;;){
		pHelp=help_allocate_new_entry();
		if(GIO_LOAD_RECORD(help_data, pHelp, fp)>1){
			break; // hit an EOF~
		};

		// link it into the list
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
		pHelp->helpfile=pHelpfile;
		pHelp->helpfile->entries++;

		// patching up the help entries
		if(has_colour(pHelp->keyword)){
			replace_string(pHelp->keyword,rtrim_string(ltrim_string(strip_colour(pHelp->keyword))));
		}
		if(pHelp->level==-1){
			pHelp->level=0;
			SET_BIT(pHelp->flags, HELP_HIDE_KEYWORDS);
		}
		pHelp->undo_edittext=str_dup("");
		pHelp->undo_wraptext=str_dup("");

		get_widest_line_stats(pHelp->text, true, &pHelp->widest_line, &pHelp->widest_line_width);
/*
		// help keyword has single quotes in it... remove and convert 'a b' to a-b
		// and trim any whitespace to the right of the keywords
		if(count_char(pHelp->keyword, '\'')){
			char newkey[MSL]; newkey[0]='\0';
			char *p=pHelp->keyword;
			char pword[MIL];
			while(*p){
				p=first_arg(p, pword, false);
				if(has_whitespace(pword)){
					for(char *pw=pword; *pw; pw++){
						if(is_space(*pw)){
							*pw='-';
						}
					}
				}
				if(!is_exact_name(pword, newkey)){
					strcat(newkey, pword);
					strcat(newkey," ");
				}
			}
			replace_string(pHelp->keyword,rtrim_string(newkey));
		}else{
			replace_string(pHelp->keyword,rtrim_string(pHelp->keyword));
		}
*/
		top_help++;
	}

	// add helpfile to the list of helpfiles
	if (helpfile_first)
	{
		helpfile_last->next=pHelpfile;
		helpfile_last=pHelpfile;
	}
	else
	{
		helpfile_first=pHelpfile;
		helpfile_last=pHelpfile;
	}
	top_helpfile++;

	colour_convert_helps(pHelpfile);

    return;
}


/**************************************************************************/
char *help_generate_prev_next_for_char(help_data *pHelp, char_data *ch)
{
	static char result[MSL*2];
	result[0]='\0';

	pc_data *p=TRUE_CH(ch)->pcdata;
	// [PREV][NEXT] links
	if(ch->desc && ch->desc->connected_state==CON_PLAYING){
		if(!IS_SET(pHelp->flags, HELP_HIDE_PREVNEXT) && HAS_MXP(ch)){
			if(!IS_NULLSTR(p->help_history[p->help_history_index])){
				strcat(result, "`=[[");
				strcat(result, mxp_create_send(ch,"helpprev","PREV"));
				strcat(result, "]");
			}else{
				strcat(result, "`=][PREV]");
			}

			if(p->help_next_count>0){
				strcat(result, "`=[[");
				strcat(result, mxp_create_send(ch, "helpnext","NEXT"));
				strcat(result, "]");
			}else{
				strcat(result, "`=][NEXT]");
			}
		}
	}
	return result;
}
/**************************************************************************/
// returns the references as a list of helplinks (only for those which exist)
char *help_generate_references_list_linked(char_data *ch, const char *header, char *references)
{
	static char result[MSL];
	if(IS_NULLSTR(references)){
		return "";
	}

	char *p=references;
	result[0]='\0';
	char pword[MIL];
	while(*p){
		p=first_arg(p, pword, false);
		if(IS_IMMORTAL(ch) || help_get_by_keyword(pword, ch, false)){
			if(has_space(pword)){
				strcat(result, FORMATF(" `=_'%s',", pword));
			}else{
				strcat(result, FORMATF(" `=_%s,", pword));
			}
		}
	}
	if(!IS_NULLSTR(result)){
		// trim the trailing comma
		result[str_len(result)-1]='\0';

		// insert the header, and prepend a new line
		strcpy(result, FORMATF("%s%s\r\n", header, result));
	}
	return result;
}
/**************************************************************************/
// returns the references as a list of helplinks (only for those which exist)
char *help_generate_references_links(char_data *ch, help_data *pHelp)
{
	static char result[MSL*2];
	result[0]='\0';

	strcat(result, help_generate_references_list_linked(ch, "`=|PARENT HELP: ", pHelp->parent_help));
	strcat(result, help_generate_references_list_linked(ch, "`=|>>>SEE ALSO: ", pHelp->see_also));
	strcat(result, help_generate_references_list_linked(ch, "`=|>>>IMMSEE ALSO: ", pHelp->immsee_also));

	return result;
}


/**************************************************************************/
char * get_spinfo_data_requirements(char_data *ch, char *text, 
									char have_colour, char havenot_colour);
/**************************************************************************/
char *help_generate_help_entry_for_char(help_data *pHelp, char_data *ch)
{
	if(!pHelp){
		bugf("help_generate_help_entry_for_char() Empty help to display!?!?");
		do_abort();
		return "";
	}
	bool use_prevnext_bar=false;
		
	if(!ch || !TRUE_CH(ch)->pcdata){
		return "help_generate_help_entry_for_char(): players only sorry";
	}

	// the maximum size a help can get is roughly MAX_HELP_SIZE 
	// + MIL for each extra format option (header, title, see also, parent, continues)
	static char result[MAX_HELP_SIZE + 6*MIL];

	result[0]='\0';

	char prefix[MIL];
	prefix[0]='\0';
	// display the category and edit option	
	if(pHelp->category){
		char *cat=flag_string(help_category_types, pHelp->category);
		strcat(prefix, FORMATF("`S[`x%s`s]", mxp_create_send(ch, FORMATF("helpcat %s", cat), cat)));
		if(HAS_SECURITY(ch, 9) && HAS_MXP(ch)){
			strcat(prefix, mxp_create_send(ch, FORMATF("hedit %s", pHelp->keyword), "edit"));
		}
	}else{
		if(HAS_SECURITY(ch, 9) && HAS_MXP(ch)){						
			strcat(prefix,"`S");
			strcat(prefix, mxp_create_send(ch, FORMATF("hedit %s", pHelp->keyword), "edit"));
		}
	}
	// display the header
	if(!GAMESETTING3(GAMESET3_HELP_HEADER_FOOTER_BAR_DISABLED) 
		&& !IS_NULLSTR(game_settings->help_header_bar) 
		&& !IS_SET(pHelp->flags, HELP_HIDE_HEADER_FOOTER))
	{
		strcat(result, "`=\xad"); // default colour for the bar
		strcat(result, game_settings->help_header_bar);
		strcat(result, "`x\r\n");
	}
	// display the keywords if appropriate unless they are hidden
    if( IS_SET(pHelp->flags, HELP_HIDE_KEYWORDS) )
    {
		if (IS_IMMORTAL(ch) && ch->desc->connected_state==CON_PLAYING){
			strcat(result, prefix);
			strcat(result,FORMATF("`g[%2d - %s]`S%s`x\r\n",
				pHelp->level, 
				pHelp->helpfile?
					mxp_create_send(ch, FORMATF("hlist %s", pHelp->helpfile->file_name),
						pHelp->helpfile->file_name)
					:"`#`Runknown!!!`^",

				pHelp->keyword));
		}
	}else{
		if (IS_IMMORTAL(ch)){
			strcat(result, prefix);
			strcat(result, FORMATF("`G[%2d - %s]`=J%s`x\r\n",
				pHelp->level, 
				pHelp->helpfile?
					mxp_create_send(ch, FORMATF("hlist %s", pHelp->helpfile->file_name),
					pHelp->helpfile->file_name)
					:"`#`Runknown!!!`^",
				pHelp->keyword));
		}else{
			strcat(result,prefix);
			strcat(result, FORMATF("`=J%s`x\r\n", pHelp->keyword));
		}
    }


	// display the title 'centered'
	if(!IS_NULLSTR(pHelp->title)){
		int spaces=(70-c_str_len(pHelp->title))/2;
		strcat(result, FORMATF("%*c`#`=u%s`&\r\n\r\n",spaces,' ',pHelp->title));
	}

	// display the command reference - left align
	if(!IS_NULLSTR(pHelp->command_reference)){
		strcat(result, FORMATF("`#`=uCommand Reference: %s`&\r\n\r\n",pHelp->command_reference));
	}

	// display redirection comment
	if(IS_SET(pHelp->flags, HELP_REDIRECTION_ENTRY)){
		strcat(result, "`=JRedirection help file:\r\n\r\n");
	}

    // check if help entry isn't too long
    if (str_len(pHelp->text)>MAX_HELP_SIZE)
    {
	    char logbuf[MSL]; // logging of extra long helps
        sprintf(logbuf, "Help entry found '%s' - but it is too long to be "
            "displayed... please inform an admin so it can be fixed.\r\n",
            pHelp->keyword);
        strcat(result, logbuf);

        sprintf(logbuf, "longhelp: %s found help entry too long '%s'!",
            ch->name, pHelp->keyword);
        append_datetime_ch_to_file( ch, NO_HELP_FILE, logbuf);
        strcat (logbuf, "\r\n");
        wiznet(logbuf,ch,NULL,WIZ_NOHELP,0,get_trust(ch));
        return result;
    }

	// if this help has a spellname insert the casting syntax
	int sn=-1;
	if(!IS_NULLSTR(pHelp->spell_name)){
		sn=spell_exact_lookup(pHelp->spell_name);
		int cla;
		int ct;
		char *target;
		if(sn>=0){
			// get the target info
			switch ( skill_table[sn].target )
			{
			default:
			case TAR_CHAR_SELF:
				target="";
				break;

			case TAR_IGNORE:
				target="<target>";
				break;

			case TAR_MOB_OFFENSIVE:
			case TAR_CHAR_OFFENSIVE:
				target="<target>  `S(offensive)";
				break;

			case TAR_CHAR_DEFENSIVE:
				target="<target>";
				break;	
										
			case TAR_OBJ_INV:
				target="<object in inventory>";
				break;
				
			case TAR_DIRECTION:
				target="<direction>";
				break;
				
			case TAR_OBJ_MOB_OFF:
			case TAR_OBJ_CHAR_OFF:
				target="<target|object>  `S(offensive)";
				break; 
				
			case TAR_OBJ_CHAR_DEF:
				target="<target|object>";
				break;
			}

			// loop thru all the cast types, checking if we have a 
			// class which can cast the spell
			for(ct=CCT_NONE+1; !IS_NULLSTR(castnames_types[ct].name); ct++){
				// find a class which uses this cast type
				// that has access to the spell
				for(cla=0; !IS_NULLSTR(class_table[cla].name); cla++){
					if(class_table[cla].class_cast_type==ct
						&& skill_table[sn].skill_level[cla]>0
						&& skill_table[sn].skill_level[cla]<=LEVEL_HERO)
					{
						if(has_space(skill_table[sn].name)){
							strcat(result, FORMATF("`=lSyntax: `=?%s '%s' %s\r\n",							
								castcommand_types[ct].name,
								skill_table[sn].name,
								target));
						}else{
							strcat(result, FORMATF("`=lSyntax: `=?%s %s %s\r\n",							
								castcommand_types[ct].name,
								skill_table[sn].name,
								target));
						}
						break;  // skip to next cast type
					}
				}
			}
			strcat(result, "\r\n");
		}
	}

	strcat(result, "`=?"); // help text defaults to CC_HELP_DEFAULT :)

	// display the text body of the help
    // Strip leading '.' to allow initial blanks.
    strcat(result, pHelp->text[0] == '.'?pHelp->text+1:pHelp->text);


	// if it is a spell, display the realms/spheres etc
	if(sn>=0){
		// check if we need to add the newline
		int lines=0;
		{
			bool in_colour_code=false;
			char *pstr;
			pstr=&pHelp->text[UMAX(str_len(pHelp->text)-50, 0)]; // jump back up to 50 characters from end
			for(; *pstr; pstr++){
				switch(*pstr){
				case '\n':
					lines++;
					break;
				
				 // ignore these
				case '\r':
				case ' ': 
					break;

				case COLOURCODE:
					in_colour_code=true;
					break;

				default:
					{
						if(in_colour_code){
							if(*pstr!='='){
								in_colour_code=false;
							}
						}else{
							lines=0;
						}
					}
					break;
				}
			}
		}

		if(lines<2){
			strcat(result, "\r\n");
		}
		char *txt;

		// realms
		txt=get_spinfo_data_requirements( ch, flag_string( realm_flags, skill_table[sn].realms), 'r', 'R');
		if(strcmp(strip_colour(txt), "none")){
			strcat(result, FORMATF("`xRealms: %s\r\n", txt));
		}

		// spheres
		txt=get_spinfo_data_requirements( ch, flag_string( sphere_flags, skill_table[sn].spheres), 'g', 'G');
		if(strcmp(strip_colour(txt), "none")){
			strcat(result, FORMATF("`xSpheres: %s\r\n", txt));
		}

		// elements
		txt=get_spinfo_data_requirements( ch, flag_string( element_flags, skill_table[sn].elements), 'b', 'B');
		if(strcmp(strip_colour(txt), "none")){
			strcat(result, FORMATF("`xElements & Seasons: %s\r\n", txt));
		}

		// compositions
		txt=get_spinfo_data_requirements( ch, flag_string( composition_flags, skill_table[sn].compositions), 'c', 'C');
		if(strcmp(strip_colour(txt), "none")){
			strcat(result, FORMATF("`xCompositions: %s\r\n", txt));
		}

		strcat(result, FORMATF("`xMana: %d   DamType: %s\r\n", 
			skill_table[sn].min_mana, flag_string(damtype_types, skill_table[sn].damtype)));		
	}

	if(IS_SET(pHelp->flags, HELP_DISPLAY_MXP_DOUBLE)){
		strcat(result, "============================== MXP VERSION ==============================\r\n");
	    strcat(result, mxp_tagify(pHelp->text[0] == '.'?pHelp->text+1:pHelp->text));
	}
	
	if(!IS_NULLSTR(pHelp->continues)){
		strcat(result, FORMATF("\r\n`=|>>>This help continues in `=_%s\r\n",
			pHelp->continues));
	}
	
	// put [PREV][NEXT] above the see also stuff if appropriate
	if(GAMESETTING4(GAMESET4_HELP_PREV_NEXT_SEPARATE_FROM_FOOTER)
		&& GAMESETTING4(GAMESET4_HELP_PREV_NEXT_ABOVE_SEE_ALSO))
	{
		strcat(result, help_generate_prev_next_for_char(pHelp, ch));
		strcat(result, "\r\n");
	}

	// put in the see also related references
	strcat(result, help_generate_references_links(ch, pHelp));

	if(GAMESETTING5(GAMESET5_MXP_EDIT_AT_BOTTOM_OF_HELPS)){
		if(HAS_SECURITY(ch, 9) && HAS_MXP(ch)){
			strcat(result,"`S");
			strcat(result, mxp_create_send(ch, FORMATF("hedit %s", pHelp->keyword), "edit"));
			strcat(result,"\r\n");
		}
	}

	// put [PREV][NEXT] below the see also stuff if appropriate
	if(GAMESETTING4(GAMESET4_HELP_PREV_NEXT_SEPARATE_FROM_FOOTER)
		&& GAMESETTING4(GAMESET4_HELP_PREV_NEXT_ABOVE_SEE_ALSO))
	{
		strcat(result, help_generate_prev_next_for_char(pHelp, ch));
		strcat(result, "\r\n");
	}

	// put [PREV][NEXT] as the start of the footer bar if appropriate
	if(!GAMESETTING4(GAMESET4_HELP_PREV_NEXT_SEPARATE_FROM_FOOTER)){
		char *pn=help_generate_prev_next_for_char(pHelp, ch);
		if(!IS_NULLSTR(pn)){
			strcat(result, pn);
			use_prevnext_bar=true;
		}
	}

	// display the footer 
	if(!GAMESETTING3(GAMESET3_HELP_HEADER_FOOTER_BAR_DISABLED) 
		&& !IS_NULLSTR(game_settings->help_footer_bar) 
		&& !IS_NULLSTR(game_settings->help_prevnext_footer_bar) 
		&& !IS_SET(pHelp->flags, HELP_HIDE_HEADER_FOOTER))
	{
		strcat(result, "`=\xad"); // default colour for the bar
		if(use_prevnext_bar){
			strcat(result, game_settings->help_prevnext_footer_bar);
		}else{
			strcat(result, game_settings->help_footer_bar);
		}
		strcat(result, "\r\n");
	}else{
		// if we didn't display a footer, but displayed a [PREV][NEXT]
		// to form the start of the bar, we need to put a new line
		if(!GAMESETTING4(GAMESET4_HELP_PREV_NEXT_SEPARATE_FROM_FOOTER)){
			strcat(result, "\r\n");
		}
	}

	strcat(result,"`x");

	return result;
}
/**************************************************************************/
void help_display_to_char(help_data *pHelp, char_data *ch)
{
	ch->sendpage(help_generate_help_entry_for_char(pHelp, ch));
}
/**************************************************************************/
bool help_valid_for_char(help_data *pHelp, char_data *ch)
{
	if(!ch){
		if(pHelp->level>LEVEL_IMMORTAL 
			|| IS_SET( pHelp->flags, HELP_NSUPPORT )
			|| IS_SET( pHelp->flags, HELP_NOBLE )
			|| IS_SET( pHelp->flags, HELP_RPSUPPORT )
			)
		{
			return false;
		}
		return true;
	}

	if ( pHelp->level > get_trust( ch ) ){
		return false;
	}

	if ( IS_IMMORTAL( ch )){
		return true;
	}

	if ( IS_SET( pHelp->flags, HELP_NSUPPORT )&& !IS_NEWBIE_SUPPORT(ch)){
		return false;
	}

	if ( IS_SET( pHelp->flags, HELP_NOBLE ) &&   !IS_NOBLE(ch)){
		return false;
	}

	if ( IS_SET( pHelp->flags, HELP_RPSUPPORT )&&!HAS_CONFIG( ch, CONFIG_RP_SUPPORT )){
		return false;
	}
	
	if ( IS_SET( pHelp->flags, HELP_BUILDER ) && GET_SECURITY(ch)==0){
		return false;
	}
	return true;
}
/**************************************************************************/
// return a help based on the keyword for a particular character
help_data *help_get_by_keyword(char * keyword, char_data *ch, bool exact_match)
{
	static bool space_to_dash=false;
	char buf[MSL];
	int count=0;
    int number=1;
	help_data *pHelp;
    char key[MIL];

	if(space_to_dash){
		strncpy(buf,keyword, MSL-1);
		buf[MSL-1]='\0';
		keyword=buf;
 
		bool found=false;
		// if this is set, we convert all spaces in the keyword to dashes
		for(char *sp=keyword;*sp; sp++){
			if(*sp==' '){
				*sp='-';
				found=true;
			}
		}
		// if we didn't find any spaces, then the previous search didn't have
		// any spaces, therefore no point in searching again
		if(!found){
			return NULL;
		}

	}

	// support x.keyword syntax
    number = number_argument( keyword, key);

	// use the hash table quick search system for words longer 3 characters+
	if(str_len(key)>2 && !is_space(key[1]) && !is_space(key[2])){ 
		int hashkey=LOWER(key[0]) + LOWER(key[1]) + LOWER(key[2]);
		if(key[0]=='\''){
			hashkey-='\'';
			hashkey+=LOWER(key[3]);			
		}
		hashkey%=HELP_QUICKLOOKUP_HASH;
		if(key[0]!='\'' || (key[3] && !is_space(key[3]))){ 
			// because of the ' at the start it needs to be 4 characters
			help_quicklookup_data *node=help_quicklookup_table[hashkey];

			while(node){
				if( (exact_match && is_exact_name(key, node->pHelp->keyword))
					|| (!exact_match && is_name(key, node->pHelp->keyword)) )
				{
					if(help_valid_for_char(node->pHelp, ch) && ++count == number){
						return node->pHelp;
					}
				}
				node=node->next;
			}

			// if the search which has just failed to match didn't use 
			// space_to_dash, retry using it.
			if(!space_to_dash){ 
				space_to_dash=true;
				help_data *result=help_get_by_keyword(keyword, ch, exact_match);
				space_to_dash=false;
				return result;
			}
			return NULL;
		}
	}

	// for the 3 character or less words
	for(pHelp=help_first; pHelp; pHelp=pHelp->next){
		if(is_name(key, pHelp->keyword)){
			if(help_valid_for_char(pHelp, ch) && ++count == number){
				return pHelp;
			}
		}
	}

	// if the search which has just failed to match didn't use 
	// space_to_dash, retry using it.
	if(!space_to_dash){ 
		space_to_dash=true;
		help_data *result=help_get_by_keyword(keyword, ch, exact_match);
		space_to_dash=false;
		return result;
	}

	return NULL;
}
/**************************************************************************/
// display a previously viewed help entry
void do_helpprev( char_data *ch, char *argument )
{
    help_data *pHelp;

	// make sure we have a pc_data - used to record/display history
	pc_data*p;
	p=ch?TRUE_CH(ch)->pcdata:NULL;
	if(!p){
		ch->println("players only sorry");
		return;
	}

	// get the previous keyword
	p->help_history_index= (p->help_history_index - 1 + MAX_HELP_HISTORY) % MAX_HELP_HISTORY;
	char *keywords=p->help_history[p->help_history_index];
	if(IS_NULLSTR(keywords)){
		++p->help_history_index%=MAX_HELP_HISTORY;
		ch->println("There are no more previous helps");
		return;
	}
	
	// search for it
	pHelp=help_get_by_keyword(keywords, ch, true);
	if(!pHelp){
		ch->printlnf("No help entry '%s' appears to exist anymore.",keywords);
		return;
	}

	// display it
	p->help_next_count++;
	p->help_history_index= (p->help_history_index - 1 + MAX_HELP_HISTORY) % MAX_HELP_HISTORY;
	help_display_to_char(pHelp, ch);
	++p->help_history_index%=MAX_HELP_HISTORY;
}
/**************************************************************************/
// display the next help in the help history after using help prev
void do_helpnext( char_data *ch, char *argument )
{
    help_data *pHelp;

	// make sure we have a pc_data - used to record/display history
	pc_data*p;
	p=ch?TRUE_CH(ch)->pcdata:NULL;
	if(!p){
		ch->println("players only sorry");
		return;
	}

	// get the next keyword
	++p->help_history_index%=MAX_HELP_HISTORY;
	p->help_next_count--;
	char *keywords=p->help_history[p->help_history_index];
	if(IS_NULLSTR(keywords)){
		++p->help_history_index%=MAX_HELP_HISTORY;
		ch->println("There are no more previous helps");
		return;
	}
	
	// search for it
	pHelp=help_get_by_keyword(keywords, ch, true);
	if(!pHelp){
		ch->printlnf("No help entry '%s' appears to exist anymore.",keywords);
		return;
	}

	// display it	
	p->help_history_index= (p->help_history_index - 1 + MAX_HELP_HISTORY) % MAX_HELP_HISTORY;
	help_display_to_char(pHelp, ch);
	++p->help_history_index%=MAX_HELP_HISTORY;
}

/**************************************************************************/
void do_help( char_data *ch, char *argument )
{
    help_data *pHelp;

	pc_data*p;
	p=ch?TRUE_CH(ch)->pcdata:NULL;

	if(!p){
		ch->println("Players only sorry");
		return;
	}

    if(IS_NULLSTR(argument)){
        argument = "summary";
    }

	// try exact match first
	pHelp=help_get_by_keyword(argument, ch, true);
	if(!pHelp){
		// try substring match as fall back
		pHelp=help_get_by_keyword(argument, ch, false);
	}
	if(pHelp){
		if(p->help_next_count){
			p->help_history_index= (p->help_history_index + p->help_next_count)%MAX_HELP_HISTORY;
			p->help_next_count=0;
		}
		help_display_to_char(pHelp, ch);
		// record the last seen help entry - unless it is exactly like the current
		if(str_cmp(p->help_history[p->help_history_index], pHelp->keyword)){
			++p->help_history_index%=MAX_HELP_HISTORY;
			replace_string(p->help_history[p->help_history_index], pHelp->keyword);
			// wipe the next to prevent reverse looping
			replace_string(p->help_history[(p->help_history_index+1)%MAX_HELP_HISTORY],""); 
		}
		return;
	}
	msp_to_room(MSPT_ACTION, MSP_SOUND_NOHELP, 0, ch, true, false );
    ch->printlnf( "Sorry, no help on the keyword '%s' was found.", argument);
	ch->printlnf( "Try using 'helplist' with the first few letters of\r\n"
		     "what you are looking for." );
    // log missed help entry into NO_HELP_FILE 

    append_datetime_ch_to_file( ch, NO_HELP_FILE, argument);

    char logbuf[MSL];
    sprintf(logbuf, "no_help: %s<%d> found no help for '%s'\n", ch->name, ch->level, argument);
    wiznet(logbuf,ch,NULL,WIZ_NOHELP,0,get_trust(ch));
    return;
}
/**************************************************************************/
// Count all the help entries in a particular category... Kal - Apr 01
int help_count_in_category( int category)
{
    help_data *pHelp;
	int total=0;
    for ( pHelp = help_first; pHelp; pHelp = pHelp->next ){
		if(pHelp->category==category){
			total++;
		}
	}
	return total;
}
/**************************************************************************/
// Kal - Apr 01
void do_helpcat( char_data *ch, char *argument )
{
	int i;
	int max_undefined=UMAX(ch->lines, 20);
	int max_per_category=250;

	if(IS_NULLSTR(argument)){
		// display all the categories
		ch->titlebar("HELP CATEGORIES");
		for(i=0; !IS_NULLSTR(help_category_types[i].name); ){
			ch->printf(FORMATF("   %%s%%-%ds", 22-str_len(help_category_types[i].name)),
				mxp_create_send(ch,FORMATF("helpcat %s", help_category_types[i].name), 
					help_category_types[i].name),				
				FORMATF("(%d)",help_count_in_category(i)));
			if(++i%3==0){
				ch->println("");
			}
		}
		if(i%3!=0){
			ch->println("");
		}
		ch->titlebar("");
		return;
	}

	int index=flag_value(help_category_types, argument);
	if(index==NO_FLAG){
		ch->printlnf("Couldn't find any '%s' help category", argument);
		return;
	}

	// display the content of one category
    help_data *pHelp;
	ch->titlebarf("HELP CATEGORIES: %s", flag_string(help_category_types, index));
	i=0;
    for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
    {
		if(pHelp->category!=index){
			continue;
		}
		if(!help_valid_for_char(pHelp, ch)){
			continue;
		};
		i++;
		if(i>max_per_category){
			continue;
		}
		if(i>max_undefined && index==0){
			continue;
		}
		ch->printf("  %s", IS_NULLSTR(pHelp->title)?"":pHelp->flags?"+":"*");
		if(str_len(pHelp->keyword)>70){
			ch->printlnf("`=_\"%-72.72s\"`x", FORMATF("%s",pHelp->keyword));
		}else{
			ch->printlnf("`=_%-72.72s`x", FORMATF("\"%s\"",pHelp->keyword));
		}

	}
	ch->printf("  `B%s`x ", mxp_create_send(ch, "helpcat", "Show all categories"));
	if(i>max_undefined && index==0){
		ch->printlnf("  `S%d undefined entries, displaying first %d.`x", i, max_undefined);
	}else if(i>max_per_category){
		ch->printlnf("  `S%d entries, displaying first %d.`x", i, max_per_category);
	}else{
		ch->printlnf("  `S%d entries displayed.`x", i);
	}
	ch->titlebarf("HELP CATEGORIES: %s", flag_string(help_category_types, index));
}
/**************************************************************************/
void do_helplist( char_data *ch, char *argument )
{
    help_data *pHelp;
    char argall[MIL],argone[MIL];
    char logbuf[MSL]; // no_help logging stuff 
    int count=0;

    if ( IS_NULLSTR(argument))
    {
		do_help(ch,"HELPLIST");
        return;
    }

    // this parts handles help a b so that it returns help 'a b' 
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
        argument = one_argument(argument,argone);
        if (argall[0] != '\0')
            strcat(argall," ");
        strcat(argall,argone);
    }


    for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
    {
        if ( is_name( argall, pHelp->keyword ))
        {
			if(!help_valid_for_char(pHelp, ch)){
				continue;
			};

            count++;
            if (IS_IMMORTAL(ch)){
                ch->printlnf( "%s%3d)`x %s`x (length =%d bytes) <%s> [%d]%s", 
					IS_SET(pHelp->flags,HELP_REMOVEHELP)?"`R***":"", count,
                    mxp_create_send(ch, FORMATF("help %s", pHelp->keyword), pHelp->keyword),
					(int) str_len(pHelp->text), pHelp->helpfile->file_name, pHelp->level,
					IS_SET(pHelp->flags,HELP_REMOVEHELP)?" `RFLAGGED FOR REMOVAL`x":"");
            }else{
                ch->printlnf( "%3d) %s`x", count, 
					mxp_create_send(ch, FORMATF("help %s", pHelp->keyword), pHelp->keyword));
			}
        }
    }

    if (count) // found 
    {
        ch->println( "To access one of these help entries type help <number>.keyword" );
		ch->println( "e.g. 'help 2.who'" );
    }
    else // not found 
    {
        ch->println( "No help entries found with that contain that keyword." );
    
        // log missed help entry into NO_HELP_FILE 
    
        append_datetime_ch_to_file( ch, NO_HELP_FILE, argall);
		msp_to_room(MSPT_ACTION, MSP_SOUND_NOHELP, 0, ch, true, false );    
        sprintf(logbuf, "nohlist: %s found no helplist for '%s'\n", ch->name, argall);
        wiznet(logbuf,ch,NULL,WIZ_NOHELP,0,get_trust(ch));
    }

    return;
}

/**************************************************************************/
void do_testhelps( char_data *ch, char * )
{
    help_data *pHelp;
    bool found= false;
    char logbuf[MSL]; /* no_help logging stuff */

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
        if (str_len(pHelp->text)>8000)
        {
            sprintf(logbuf, "[%2d] %s - HELP ENTRY TOO LONG!!! (length =%ld bytes!) <%s>\r\n", 
                pHelp->level, pHelp->keyword, (long) str_len(pHelp->text), pHelp->helpfile->file_name);
            ch->printf( "%s", logbuf );

            /* log to long help entry into NO_HELP_FILE */       
            append_datetime_ch_to_file( ch, NO_HELP_FILE, logbuf);
            found = true;
        }
    }

    if (found)
        ch->println("Any entry longer than 10000 characters can't be viewed.");
	else
        ch->println("All help entries are less than 8000 characters.");

    return;
}

/**************************************************************************/
// Kal
void do_hlist( char_data *ch, char *argument )
{
    helpfile_data *pHelpFD;
    help_data *pHelp;

    if (!HAS_SECURITY(ch,1))
	{
		ch->println("The hlist command is an olc command, you dont have olc permissions.");
		return;
	}

	pHelpFD=helpfile_get_by_filename(argument);
	if (pHelpFD)
	{
		int count =0;
		ch->printf("`?`#-===[`YLVL`^]=<`Y LEN `^>(`YWL`^)=== "
			   "`YHelp entries contained in %s `^================-`x\r\n",
			   pHelpFD->file_name);
//		ch->printf("{`%s{x", 
//			format_titlebar("Help entries contained in %s", pHelpFD->file_name));
		for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
		{   
			if ( pHelp->helpfile==pHelpFD)
			{
				count++;
                if (IS_IMMORTAL(ch))
				{
					ch->printlnf("%3d)[%3d]%s<%5d>(%s%2d`x) (%s)`=_%-56.56s`x",
						count,
						pHelp->level, 
						mxp_create_send(ch,FORMATF("hedit %s", pHelp->keyword), "*"),
						(int) str_len(pHelp->text), 
						(pHelp->widest_line_width<79?(IS_SET(pHelp->flags, HELP_WORDWRAPPED)?"`S":""):"`R"),
						pHelp->widest_line_width,
						mxp_create_send(ch,FORMATF("helpcat %s", flag_string(help_category_types,pHelp->category)),
							FORMATF("%-15.15s", flag_string(help_category_types,pHelp->category))),
						FORMATF("\"%s\"",pHelp->keyword)
						);
				}else{
					ch->printlnf("%3d) %s`x", count, pHelp->keyword);           
				}
			}
		}
		if(HAS_MXP(ch)){
			ch->println(mxp_create_send(ch, "hlist", "`S[list of hlist files]`x"));
		}
	}
	else
	{
		ch->titlebar("HLIST FILES");
		int c=0;
		for ( pHelpFD = helpfile_first; pHelpFD; pHelpFD = pHelpFD->next )
		{  
			ch->printf(" [%2d] %-20s count=%s",
				pHelpFD->vnum,
				pHelpFD->file_name,
				mxp_create_send(ch, FORMATF("hlist %s", pHelpFD->file_name), 
									FORMATF("%3d", pHelpFD->entries))
				);
			if(++c%2==0){
				ch->println("");
			}else{
				ch->print("    ");
			}
		}
		if(c%2!=0){
			ch->println("");
		}
		ch->println("To see all the help entries in one of these files type `=Chlist <filename>`x.");
	}
    return;
}
/**************************************************************************/
// Version of help that does only exact matching, keywords are prefixed with 
// code_ and function returns true if it finds an exact matching help entry,
// false if it doesnt find one - Kal Feb 99
bool codehelp( char_data *ch, char *keyword, int report_unfound_codehelp_flags)
{
	help_data *pHelp;
	char lookfor[MIL];
	char logbuf[MSL]; 
	
	sprintf(lookfor,"code_%s", keyword);

	pHelp=help_get_by_keyword(lookfor, ch, true);
	if(pHelp){
		help_display_to_char(pHelp, ch);
		return true;
	}

	// codehelp entry not found... pick up the pieces
	sprintf(logbuf, "no_codehelp: %s found no help for '%s'", ch->name, lookfor);

	if(IS_SET(report_unfound_codehelp_flags, CODEHELP_LOG)){
		logf("%s", logbuf);
	}
	if(IS_SET(report_unfound_codehelp_flags, CODEHELP_NO_HELPFILE)){
		append_datetime_ch_to_file( ch, NO_HELP_FILE, logbuf);
	}
	if(		(IS_SET(report_unfound_codehelp_flags, CODEHELP_IMM) && IS_IMMORTAL(ch))
		||  (IS_SET(report_unfound_codehelp_flags, CODEHELP_ADMIN) && IS_ADMIN(ch))
		||  IS_SET(report_unfound_codehelp_flags, CODEHELP_EVERYONE))
	{
		ch->printlnf("Error: codehelp() - no help found for '%s'", lookfor);
		ch->println("Please report this to the admin.");
	}
	if(IS_SET(report_unfound_codehelp_flags,CODEHELP_WIZNET)){
		wiznet(logbuf,ch,NULL,WIZ_NOHELP,0,get_trust(ch));
	}
	return false;
}
/**************************************************************************/
/**************************************************************************/

