/**************************************************************************/
// olc_ex.cpp - Extended commands for olc - Dawn only code in here :)
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "olc_ex.h"
#include "olc.h"
#include "help.h"

/**************************************************************************/
// Lookup a spell by name - exact match - Kal
int spell_exact_lookup( const char *name )
{
    int sn;

    for( sn = FIRST_SPELL; sn < LAST_SPELL; sn++ )
    {
		if( IS_NULLSTR(skill_table[sn].name))
			break;
		if( !str_cmp( name, skill_table[sn].name ) )
			return sn;
    }

    return -1;
}

/**************************************************************************/
// Lookup a spell by name - Kal
int spell_lookup( const char *name )
{
    int sn;

	sn=spell_exact_lookup(name);

	if(sn>-1){
		return sn;
	}
    for( sn = FIRST_SPELL; sn < LAST_SPELL; sn++ )
    {
	if( IS_NULLSTR(skill_table[sn].name))
	    break;
	if( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}
/**************************************************************************/
// Returns the name of a skill without crashing if sn is out of bounds - Kal
char * safe_skill_name(int sn)
{
	if(sn==-1){
		return "unassigned";
	}
	if(sn<-1 || sn>MAX_SKILL-1){
		return "unknown!";
	}
	return skill_table[sn].name;
}

/**************************************************************************/
// return true if changed - Kal
bool olc_generic_skill_assignment_to_int(char_data *ch, int *field, const char *argument, 
	bool spell_only, bool extra_lf, 
	const char *command_name, const char *description_of_field_sprintf, ...)
{
	// format the description_of_field_sprintf into description
    char description[MSL]; 
	va_list args;
	va_start(args, description_of_field_sprintf);
	vsnprintf(description, MSL-10, description_of_field_sprintf, args);
	va_end(args);

	if(IS_NULLSTR(argument)){
		ch->printlnf("`Bsyntax:`x %s <%s>  - to change/assign %s to %s", 
			command_name, spell_only?"spell":"skill/spell",
			spell_only?"spell":"skill/spell", description);
		ch->printlnf("`Bsyntax:`x %s clear - to clear %s", 
			command_name, description);
		ch->printlnf("`x%s is currently assigned as: %s`x", 
			capitalize(description), safe_skill_name(*field));
		if(extra_lf) ch->println("");
		return false;
	}

	if(!str_cmp(argument,"clear")){
		if(*field==-1){
			ch->printlnf("`Y%s is already unassigned!`x", capitalize(description));
			if(extra_lf) ch->println("");
			return false;
		}

		ch->printlnf("`G%s cleared, was originally set to %s.`x", 
			capitalize(description), safe_skill_name(*field));
		*field=-1;
		if(extra_lf) ch->println("");
		return true;
	}

	int value;
	if(spell_only){
		value=spell_lookup( argument );
	}else{
		value=skill_lookup( argument );
	}

	if(value==-1){
		ch->printlnf("`RNo such %s '`x%s`R' found!`x", spell_only?"spell":"skill/spell", argument);
		olc_generic_skill_assignment_to_int(ch, field, "", spell_only, extra_lf, 
				command_name, description);
		return false;
	}
	
	if(*field==value){
		ch->printlnf("`Y%s is already assigned to `x%s!", 
			capitalize(description), safe_skill_name(value));
		if(extra_lf) ch->println("");
		return false;
	}
	
	ch->printlnf( "`G%s changed from `x%s `Gto `x%s.",
		capitalize(description), safe_skill_name(*field), safe_skill_name(value));
	*field= value;
	if(extra_lf) ch->println("");
	return true;
}
/**************************************************************************/
// turns the list of all flags in a table
char *flagtable_names( const struct flag_type *flag_table)
{
	static int i;
    static char buf[5][512];
    int flag;
	// rotate buffers
	++i= i%5;
    buf[i][0] = '\0';

    for(flag = 0; flag_table[flag].name != NULL; flag++){
		strcat( buf[i], " " );
		strcat( buf[i], flag_table[flag].name );
	}
    return buf[i]+1;
}
/**************************************************************************/
bool is_stat( const struct flag_type *flag_table );
/**************************************************************************/
// Kal - Jan 2001
void show_olc_flags_types(char_data *ch, const struct flag_type *flag_table)
{
	int count=0;

    for(int flag = 0; !IS_NULLSTR(flag_table[flag].name); flag++)
    {
		if(flag_table[flag].settable){
			count++;
			ch->printf("`=R   %-34s`x%s", flag_table[flag].name,
				count%2==0?"\r\n":"");
		}
	}
	ch->println("");		
}					  
/**************************************************************************/
// Kal - Jan 2001
void show_olc_flags_types_value(char_data *ch, const struct flag_type *flag_table, 
											const char *command_name, long value)
{
	int count=0;
	bool type_table=is_stat(flag_table);

	char c;
    for(int flag = 0; !IS_NULLSTR(flag_table[flag].name); flag++)
    {
		if(flag_table[flag].settable){
			count++;
			int l=34-str_len(flag_table[flag].name);
			bool match=false;
			if(type_table){
				if(value==flag_table[flag].bit){
					match=true;
				}
			}else{
				if(IS_SET(value,flag_table[flag].bit)){
					match=true;
				}
			}
			if(flag_table[flag].settable){
				c=match?'\'':'$';
			}else{
				if(match){
					c='R';
				}else{
					// we don't show these flags
					continue;
				}
			}			
			if(type_table && match){
				c='/'; // selected option code
			}

			const char *suffix;
			if(count%2==0){
				suffix="\r\n";
			}else{
				suffix="";
			}
	                        // "{=R    %s%20s{x\r\n"
			ch->printf(FORMATF("`=%c   %%s%%%ds`x%s", c, l, suffix), 
				mxp_create_send(ch, FORMATF("sca %s %s %s", command_name, 
					command_name, flag_table[flag].name), 
				flag_table[flag].name), "");
		}
	}
	ch->println("");
}					  
/**************************************************************************/
// Kal - Dec 2000
void show_olc_options(char_data *ch, const struct flag_type *flag_table, 
			const char *command_name, const char *descript, long flags)
{
	if(is_stat(flag_table)){
		ch->printlnf("Syntax: %s <%s type>", command_name, descript);
		ch->printlnf("Valid %s types include (pick one):", descript);
	}else{
		ch->printlnf("Syntax: %s <%s flag(s)>", command_name, descript);
		ch->printlnf("Valid %s flags include:", descript);		
	};
	show_olc_flags_types_value(ch, flag_table, command_name, flags);
}
/**************************************************************************/
int mxp_display_olc_flags_ex(char_data *ch, const struct flag_type *flag_table, 
		long value, char *command, char *heading, 
		int max_width /*77*/,int first_indent /*16*/, int indent /*5*/)
{
	int width;
	int found_count=0;
	bool type_table=is_stat(flag_table);
	char *flag_start="";
	int lines=0;

	char openbracket='[';
	char closebracket=']';
	if(type_table){
		openbracket='(';
		closebracket=')';
	}

	char buf[MSL];
	int l=first_indent-1-str_len(heading);
	l=UMAX(l,0);
	sprintf(buf, FORMATF("`=r%%s%%%ds%c",l, openbracket),
		mxp_create_send(ch,command, heading), "");
	width=first_indent;

	char flagbuf[MSL];
	if(!IS_SET(ch->dyn,DYN_SHOWFLAGS)){
		strcat(buf,"`=R");
		flag_start=&buf[str_len(buf)-1]; // record where the flags start so we can
										// strip the mxp later
		flagbuf[0]='\0';
	};

	bool match;
    for(int flag = 0; !IS_NULLSTR(flag_table[flag].name); flag++)
    {
		match=false;
		if(type_table){
			if(value==flag_table[flag].bit){
				match=true;
			}
		}else{
			if(IS_SET(value,flag_table[flag].bit)){
				match=true;
			}
		}

		if(IS_SET(ch->dyn,DYN_SHOWFLAGS)){
			if(flag_table[flag].settable){
				if(match){
					strcpy(flagbuf,"`='");
				}else{
					strcpy(flagbuf,"`=$");
				}
			}else{
				if(match){
					strcpy(flagbuf,"`=\x98");
				}else{
					// we don't show these flags
					continue;
				}
			}			
			if(type_table && match){
				strcpy(flagbuf,"`=/"); // selected option code
			}
		}else{
			if(!match){ // if they arent using 'showflags', we don't show unset flags
				continue;
			}
		}

		found_count++;
		width+= str_len(flag_table[flag].name)+1;
		if(width>max_width){
			strcat(buf,"\r\n");
			lines++;
			strcat(buf,FORMATF(FORMATF("%%%ds", indent), ""));
			width=indent + str_len(flag_table[flag].name) +1;
		}

		strcat(buf, flagbuf);
		if(flag_table[flag].settable){
			strcat(buf, mxp_create_send(ch,
							 FORMATF("sfa %s %s", command, flag_table[flag].name)
							,flag_table[flag].name));
		}else{
			strcat(buf, flag_table[flag].name);
		}
		strcat(buf," ");
	}
	if(found_count==0){
		strcat(buf,"none ");
	}
	buf[str_len(buf)-1]='\0';
	strcat(buf,FORMATF("`=r%c", closebracket));

	if(IS_SET(ch->dyn,DYN_SHOWFLAGS)){
		ch->println(buf);
	}else{
		*flag_start='\0';
		ch->printlnf("%sR%s", buf, flag_start+1); 
		lines++;
	}
	return lines;
}
/**************************************************************************/
int mxp_display_olc_flags(char_data *ch, const struct flag_type *flag_table, 
						   long value, char *command, char *heading)
{
	return mxp_display_olc_flags_ex(ch, flag_table, value, command, heading, 
		77 /*max_width*/, 16 /*first_indent*/, 5 /*indent*/);
}

/**************************************************************************/
bool olcex_showflags(char_data *ch, char *)
{
	SET_BIT(ch->dyn,DYN_SHOWFLAGS);
	run_olc_editor_for_connection(ch->desc, "show");
	REMOVE_BIT(ch->dyn,DYN_SHOWFLAGS);
	return false;
}
/**************************************************************************/
// does a show after putting argument thru medit()
bool olcex_showafter(char_data *ch, char *argument)
{
	if(IS_NULLSTR(argument)){
		ch->println("ShowAfter: Shows olc info after the argument is run as a command.");
		return false;
	}
	if(IS_SET(ch->dyn, DYN_USING_OLCAFTER)){
		ch->println("No run after loops!");
		return false;
	}
	SET_BIT(ch->dyn, DYN_USING_OLCAFTER);
	run_olc_editor_for_connection(ch->desc, argument);
	REMOVE_BIT(ch->dyn, DYN_USING_OLCAFTER);
	run_olc_editor_for_connection(ch->desc, "show");
	return false;
}
/**************************************************************************/
// does a command (first arg) after putting argument thru medit()
bool olcex_showcommandafter(char_data *ch, char *argument)
{
	char arg[MIL];
	argument=one_argument(argument, arg);
	if(IS_NULLSTR(argument)){
		ch->println("ShowCommandAfter: shows a command after all but the first arg is run as a command.");
		return false;
	}
	if(IS_SET(ch->dyn, DYN_USING_OLCAFTER)){
		ch->println("No run after loops!");
		return false;
	}
	SET_BIT(ch->dyn, DYN_USING_OLCAFTER);
	run_olc_editor_for_connection(ch->desc, argument);
	REMOVE_BIT(ch->dyn, DYN_USING_OLCAFTER);
	run_olc_editor_for_connection(ch->desc, arg);
	return false;
}
/**************************************************************************/
// does a show after putting argument thru medit()
bool olcex_showflagsafter(char_data *ch, char *argument)
{
	if(IS_NULLSTR(argument)){
		ch->println("ShowFlagsAfter: ShowsFlags after the argument is run as a command.");
		return false;
	}
	if(IS_SET(ch->dyn, DYN_USING_OLCAFTER)){
		ch->println("No run after loops!");
		return false;
	}
	SET_BIT(ch->dyn, DYN_USING_OLCAFTER);
	run_olc_editor_for_connection(ch->desc, argument);
	REMOVE_BIT(ch->dyn, DYN_USING_OLCAFTER);
	olcex_showflags(ch,"");
	return false;
}
/**************************************************************************/
// Kal - Apr 01
char *olcex_get_editor_name( int edit_mode)
{
    switch (edit_mode){
		case ED_AREA:		return "AEdit";
		case ED_ROOM:		return "REdit";
		case ED_OBJECT:		return "OEdit";
		case ED_MOBILE:		return "MEdit";
		case ED_MPCODE:		return "MPEdit";
		case ED_HELP:		return "HEdit";
		case ED_BAN:		return "BanEdit";
		case ED_RACE:		return "RaceEdit";
		case ED_CLASS:		return "ClassEdit";
		case ED_SPELLSKILL:	return "Spell/SkillEdit";
		case ED_COMMAND:	return "comEdit";
		case ED_DEITY:		return "dEdit";
		case ED_QUEST:		return "qEdit";
		case ED_GAME:		return "GameEdit";
		case ED_SOCIAL:		return "SocialEdit";
		case ED_HERB:		return "HerbEdit";
		case ED_MIX:		return "MixEdit";
		case ED_CLAN:		return "ClanEdit";
		case ED_SKILLGROUP:	return "SkillGroupEdit";
		case ED_LANGUAGE:	return "LangEdit";
		default:			return "";
	}
}
/**************************************************************************/
// Kal - Apr 01
const olc_cmd_type *olcex_get_editor_command_table( int edit_mode)
{
    switch (edit_mode){
		case ED_AREA:		return aedit_table;
		case ED_ROOM:		return redit_table;
		case ED_OBJECT:		return oedit_table;
		case ED_MOBILE:		return medit_table;
		case ED_MPCODE:		return mpedit_table;
		case ED_HELP:		return hedit_table;
		case ED_BAN:		return banedit_table;
		case ED_RACE:		return raceedit_table;
		case ED_CLASS:		return classedit_table;
		case ED_SPELLSKILL:	return sedit_table;
		case ED_COMMAND:	return comedit_table;
		case ED_DEITY:		return dedit_table;
		case ED_QUEST:		return qedit_table;
		case ED_GAME:		return gameedit_table;
		case ED_SOCIAL:		return socedit_table;
		case ED_HERB:		return herbedit_table;
		case ED_MIX:		return mixedit_table;
		case ED_CLAN:		return clanedit_table;
		case ED_SKILLGROUP:	return skillgroupedit_table;
		case ED_LANGUAGE:	return langedit_table;
		default:			return NULL;
	}
}
/**************************************************************************/
// Show the commands in an olc editor... display mxp help links for
// all entries - Kal, Apr 01
void show_olc_cmds( char_data *ch, const struct olc_cmd_type *olc_table )
{
	if(!olc_table){
		ch->println("show_olc_cmds() given NULL olc_table");
		ch->println("- this function is typically called by show_commands()");
		return;
	}

	int i;
	int count=0;
	for(i=0; !IS_NULLSTR(olc_table[i].name); i++){
		if(olc_table[i].hidden){
			continue;
		}
		if(HAS_MXP(ch) && ch->desc){
			// automatically make the entries helplinks if a help exists
			// if not run the command
			char keyword[MIL];
			bool found=false;
			sprintf(keyword,"OLC%s-%s", 
				uppercase(olcex_get_editor_name(ch->desc->editor)),
				uppercase(olc_table[i].name));

			// search for our generated keyword
			if(help_get_by_keyword(keyword, ch, false)){
				found=true;
			}
			// if we have _ in the keyword and we didn't find a help try 
			// converting the _ to a - and see if the help then exists
			if(!found && count_char(keyword, '_')){
				char *p=keyword;
				while(*p){
					if(*p=='_'){
						*p='-';
					}
					p++;
				}
				// search for modified keyword
				if(help_get_by_keyword(keyword, ch, false)){
					found=true;
				}
			}

			if(found){ // generate the mxp help link
				ch->printf("`=; %s `x", mxp_create_send(ch,
						FORMATF("help %s|%s\" hint=\"help %s|%s"
							,keyword, olc_table[i].name, keyword, olc_table[i].name)
						,FORMATF("%-18.18s", olc_table[i].name)) // text see on screen underlined						
					);
			}else{ // generate MXP command links
				ch->printf("`=: %s `x", mxp_create_send(ch,
								FORMATF("%s", olc_table[i].name)
								,FORMATF("%-18.18s", olc_table[i].name))
						 );
			}
		}else{
			ch->printf(" %-18.18s ", olc_table[i].name);
		}

		if(++count%4==0){
			ch->println("");
		}
	}
	if(count%4!=0){
		ch->println("");
	}
}
/**************************************************************************/
// Kal - Apr 01
bool show_commands( char_data *ch, char *)
{
	char *line=	"========================================================"
				"========================================================";

	char *editorname=uppercase(olcex_get_editor_name(ch->desc->editor));

	if(IS_NULLSTR(editorname)){
		editorname="[Unknown editor mode name - update olcex_get_editor_name()]";
	}

	ch->titlebarf("%s COMMANDS", editorname);
	show_olc_cmds( ch,	olcex_get_editor_command_table(ch->desc->editor));

	char mhelp[MIL];
	sprintf(mhelp,"MASTER HELP: `=_OLC-%s. ", uppercase(olcex_get_editor_name(ch->desc->editor)));
	int len=str_len(mhelp);
	 
	ch->printlnf("`=t%s[`=T %s`=t]=-`x", 
		(FORMATF(FORMATF("-%%.%ds", 74-len), line)),// the line
		mhelp);
    return false;
}
/**************************************************************************/
bool olcex_tab( char_data *ch, char *argument)
{
	if(IS_NPC(ch)){
		ch->println("Players only.");
		return false;
	}
	if(IS_NULLSTR(argument)){
		ch->println("syntax: tab <tab number>");
		return false;
	}

	int value =atoi(argument);
	if(value>0){
//		ch->printlnf("olc tab changed from %d to %d",ch->pcdata->olc_tab+1, value);
		ch->pcdata->olc_tab=value-1;
	}else{
		ch->println("The tab must be 1 or higher");
	}

	olcex_showflags(ch,"");
	return false;
}
/**************************************************************************/
void continents_show( char_data *ch )
{
	continent_type *c;

    ch->println( "Current Continental Land Masses" );
	ch->println( "===============================" );

	int col=0;
	for( c=continent_list; c; c=c->next){
		ch->printf(" %-40s`x", c->name );
		if ( ++col % 2 == 0 ){
			ch->print_blank_lines(1);
		}
	}
	if ( ++col % 2 == 0 ){
		ch->print_blank_lines(1);
	}
	return;
}
/**************************************************************************/
void do_ownerlist( char_data *ch, char *argument )
{
	char buf[MSL],sbuf[MIL];
	int count=0;
    BUFFER *output;
	bool all_owners=false;
	bool search=false;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The ownerlist command is an olc command, you don't have olc permissions.");
		return;
	}

    output= new_buf();

	argument =one_argument( argument, sbuf );

	if (!IS_NULLSTR(sbuf))
	{
		if (!str_cmp("all", sbuf))
		{
			all_owners=true;
		}
		else
		{
			search=true;
		}
	}


	// now list all rooms with lockers in them
	int minvnum=1;
	int maxvnum=top_vnum_room;


	// if we arent searching, nor listing all areas reduce the 
	// searching range to the current area
	if(!search  && !all_owners){
		minvnum=ch->in_room->area->min_vnum;
		maxvnum=ch->in_room->area->max_vnum;
	}

	int i;
	ROOM_INDEX_DATA *r;
	for(i=minvnum; i<=maxvnum; i++){
		r=get_room_index(i);

		if(!r || IS_NULLSTR(r->owner)){
			continue;
		}

		if(search // filter if specifed
			&& !is_name( sbuf, r->owner ) 
			&& !is_name( sbuf, r->invite_list)){
			continue;
		}

		sprintf( buf, "`x%3d`S[`=R%5d`S-%s%-8.8s`S] `g%s:`w%s`x\r\n", 
			++count, r->vnum, 
			colour_table[(r->area->vnum%14)+1].code,
			r->area->short_name,
			r->owner,
			r->invite_list);
	    add_buf(output,buf);
    }

	if(!count){
		add_buf(output,"No rooms with owners/invites found.\r\n");
	}

	if(!search  && !all_owners){
		add_buf(output,"you can also search this list, or use 'all' to see all matching rooms.\r\n");		
	}
 
	if(GAMESETTING4(GAMESET4_ROOM_INVITES_DISABLED)){
		add_buf(output,"Note: the room invites system is disabled in gameedit.\r\n");
	}

	ch->sendpage( buf_string(output));
	free_buf(output);
}
/**************************************************************************/
// Kal, Sept 02 - Count the number of bits set to 1 a long
int count_bits(long value)
{
	int count=0;
	for(int i=0; i<(int)sizeof(long)*8; i++){
		if(value&1<<i){
			count++;
		}
	}
	return count;
}
/**************************************************************************/
// Kal, Sept 02 - Finally wrote a generic flag toggling/type setting  handler :)
bool olc_generic_flag_toggle(char_data *ch, char *argument,
	const char *command_name, const char *descript, 
	const struct flag_type *flag_table, long *value)
{
    if(!IS_NULLSTR(argument)){
		long flags_to_toggle=flag_value( flag_table, argument);

		if(flags_to_toggle!=NO_FLAG){
			if(is_stat(flag_table)){
				// types for setting
				if(*value==flags_to_toggle){
					ch->printlnf("The %s type is already set to %s.", 
						descript,
						flag_string(flag_table, flags_to_toggle));
					return false;
				}else{
					ch->printlnf("%s type changed from %s to %s", 
						capitalize(descript),
						flag_string(flag_table, *value),
						flag_string(flag_table, flags_to_toggle));
					*value=flags_to_toggle;
					return true;
				}
			}else{ 
				// flags for toggling
				*value^= flags_to_toggle; // do the toggle
				// see what was toggled on 
				long on=flags_to_toggle & (*value); 
				if(on){
					ch->printlnf("%d %s flag%s toggled on: %s", 
						count_bits(on),
						descript,
						count_bits(on)==1?"":"s",
						flag_string(flag_table,on));
				}
				// see what was toggled off 
				long off=flags_to_toggle & (~(*value));
				if(off){
					ch->printlnf("%d %s flag%s toggled off: %s", 
						count_bits(off),
						descript,
						count_bits(off)==1?"":"s",
						flag_string(flag_table,off));
				}
			}
		}else{
			ch->printlnf("`=X'%s' wasn't recognised as a valid %s flag.`x", argument, descript);
			show_olc_options(ch, flag_table, command_name, descript, *value);
		}
		return true;
    }
	show_olc_options(ch, flag_table, command_name, descript, *value);
	return false;
}
/**************************************************************************/
// Kal, April 03 - support 'int *', as well as 'long *' above
bool olc_generic_flag_toggle(char_data *ch, char *argument,
	const char *command_name, const char *descript, 
	const struct flag_type *flag_table, int *value)
{
	long lng=(long)*value;
	bool result=olc_generic_flag_toggle(ch, argument, command_name, descript, flag_table, &lng);
	*value=(int)lng;
	return result;
}

/**************************************************************************/
// returns a pointer to the character after the \n
// it is assumed that there are no \r's in the input
// - Kal, April 04
const char *retrieve_line( const char *entire_text, char *line)
{
	while(*entire_text){
		if(*entire_text=='\n'){
			entire_text++;
			break;
		}else{
			*line++=*entire_text++;
		}
	}
	*line='\0';
	return entire_text;
}

/**************************************************************************/
// - Kal, April 2004
char *indent_mobprog_code(char_data *ch, char *code, int custom_indent_amount)
{
	char code_less_carriage_return[MSL*2];
	static char result[MSL*2];
	char line[MSL*2];

	// pre processing loop thru creating a copy of the code without carriage returns
	char *out=code_less_carriage_return;
	for(char *in=code; *in; in++){
		if(*in!='\r'){
			*out++=*in;
		}
	}
	*out='\0';
	
	char *l;
	char first_word[MIL];
	result[0]='\0';
	int this_indent=0;
	int new_indent=0;
	int indent_amount;
	
	if(custom_indent_amount){
		indent_amount=URANGE(0, custom_indent_amount, 8);
	}else{
		indent_amount=4;
	}
	const char *p=code_less_carriage_return;
	while(*p){
		// retrieve a line
		p=retrieve_line(p, line);
		l=ltrim_string(line);

		// process the first word on the line, making an indenting decision
		one_argument(l, first_word);
		if(!str_cmp( first_word, "if" )){
			new_indent+=indent_amount;
		}else if(!str_cmp( first_word, "and" )){
			this_indent-=indent_amount;
		}else if(!str_cmp( first_word, "or" )){
			this_indent-=indent_amount;
		}else if(!str_cmp( first_word, "else" )){
			this_indent-=indent_amount;
		}else if(!str_cmp( first_word, "endif" )){
			this_indent-=indent_amount;
			new_indent-=indent_amount;			
		}
		if(this_indent<0){
			ch->println("`RWARNING:`x It seems you have more endif's than if statements!");
			this_indent=0;
			if(new_indent<0){
				new_indent=0;
			}
		}
		if(new_indent<0){
			ch->println("`RWARNING:`x It seems you have an else statement before a matching if statements!");
			new_indent=0;
		}
		strcat(result, FORMATF(FORMATF("%%%ds%%s\r\n",this_indent), "", l));
		this_indent=new_indent;
	}

	if(new_indent!=0){
		ch->println("`RWARNING:`x It appears your program doesn't correctly close off its if statements!");
	}

	return result;
}
/**************************************************************************/
// by Kal - June 98
MUDPROG_TRIGGER_LIST * dup_mudprog_list(MUDPROG_TRIGGER_LIST * mplist)
{	
	MUDPROG_TRIGGER_LIST * pMplist;
	
	if (mplist==NULL)
		return(NULL);

	pMplist				= new_mprog();
	// use recursion to maintain the order of the list
	pMplist->next = dup_mudprog_list(mplist->next);

	pMplist->trig_phrase= str_dup(mplist->trig_phrase);
	pMplist->trig_type	= mplist->trig_type;
	pMplist->prog		= mplist->prog;
	
	return (pMplist);
}

/**************************************************************************/
/**************************************************************************/



