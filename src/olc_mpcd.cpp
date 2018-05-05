/**************************************************************************/
// olc_mpcd.cpp - olc mudprog editor, see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/* The following code is based on ILAB OLC by Jason Dinkel */
/* Mudprogram code by Lordrom for Nevermore Mud */

#include "include.h" // dawn standard includes

#include "olc.h"

#define MPEDIT( fun )           bool fun(char_data *ch, char*argument)

const struct olc_cmd_type mpedit_table[] =
{
/*	{	command		function	}, */

	{	"commands",	show_commands	},
	{	"create",	mpedit_create	},
	{	"code",		mpedit_code	},
	{	"author",	mpedit_author},
	{	"title",	mpedit_title},
	{	"copy",		mpedit_copy},
	{	"mpcopy",	mpedit_mpcopy},
	{	"delete",	mpedit_delete},
	{	"mpdelete",	mpedit_mpdelete},
	{	"indent",	mpedit_indent},
	{	"autoindent",mpedit_autoindent},
	{	"show",		mpedit_show	},
	{	"?",		show_help	},

	{	NULL,		0		}
};

/**************************************************************************/
void mpedit( char_data *ch, char *argument)
{
    MUDPROG_CODE *pMcode;
    char arg[MIL];
    char command[MIL];
    int cmd;

    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);

    EDIT_MPCODE(ch, pMcode);
    if (ch->pcdata->security < MPEDIT_MINSECURITY)
    {
        ch->println("MPEdit: Insufficient security to modify code");
        edit_done(ch);
        return;
    }

	if( !IS_BUILDER_WHEN_NOT_RESTRICTED( ch, pMcode->area) ){			
		ch->printlnf( "Insufficient security to modify a mudprog in area '%s'.", pMcode->area->name);
		edit_done( ch );
		return;
	}

    if(IS_NULLSTR(command)){
        mpedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

	for (cmd = 0; mpedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix(command, mpedit_table[cmd].name) )
        {
            if ( (*mpedit_table[cmd].olc_fun) (ch, argument) )
            {
				SET_BIT( pMcode->area->olc_flags, OLCAREA_CHANGED );
            }
			return;
        }
    }

    interpret(ch, arg);
    return;
}

/**************************************************************************/
void do_mpedit(char_data *ch, char *argument)
{
    MUDPROG_CODE *pMcode;
    char command[MIL];

	if (!HAS_SECURITY(ch, 2))
	{
		ch->println("You must have an olc security 2 or higher to use this command.");
		return;
	}

    if ( !HAS_SECURITY(ch, MPEDIT_MINSECURITY))
    {
    	ch->println("MPEdit: Insufficient security to modify code.");
    	return;
    }

    argument = one_argument( argument, command);
	int value;
    if(is_number(command) )
    {
		value=atoi(command);
		if (! (pMcode = get_mprog_index( value ) ) )
		{
			ch->println("MPEdit: That vnum does not exist.");
			return;
		}

		if( !IS_BUILDER_WHEN_NOT_RESTRICTED( ch, pMcode->area) ){
			ch->printlnf( "Insufficient security to modify a mudprog in area '%s'.", pMcode->area->name);
			return;
		}

		// officially reserved vnum range
		if(value<500){
			ch->println("Warning: all mobs, rooms and objects below vnum 500 are officially reserved for the dawn codebase.");
			if(!HAS_SECURITY(ch,9)){
				ch->println("As a result of this reservation, only those with security 9 can edit in that vnum range.");
				return;
			}			
		}

		ch->desc->pEdit=(void *)pMcode;
		ch->desc->editor= ED_MPCODE;
		
		return;
    }

    if (!str_cmp(command, "create" ) )
    {
        if (IS_NULLSTR(argument)){
			ch->println("Syntax: `=Cmpedit <mudprog vnum>`x");
			ch->println("Syntax: `=Cmpedit create <new mudprog vnum>`x");
			return;
        }

        if (mpedit_create(ch, argument) ){
                ch->desc->editor = ED_MPCODE;
		}
		return;
    }

	ch->println("Syntax: `=Cmpedit <mudprog vnum>`x");
	ch->println("Syntax: `=Cmpedit create <new mudprog vnum>`x");
	ch->println("MPEdit: Mudprog must be referenced by number.");
	ch->println("Use `=Cmplist`x for a list of all mudprogs in your current area vnum range.");
    return;
}

/**************************************************************************/
bool mpedit_create(char_data *ch, char *argument)
{
    MUDPROG_CODE *pMcode;
    int value;
	AREA_DATA * tarea=NULL;

    value = atoi(argument);
    if (argument[0] == '\0' || value == 0)
    {
        ch->println("Syntax: mpedit create [vnum]");
        return false;
    }

    if (!IS_NPC(ch) && ch->pcdata->security < MPEDIT_MINSECURITY)
    {
        ch->println("MPEdit: Insufficient security to create MudProgs.");
        return false;
    }

    if (get_mprog_index(value) )
    {
        ch->println("MPEdit: Code vnum already exists.");
        return false;
    }

	// officially reserved vnum range
	if(value<500){
		ch->println("Warning: all mobs, rooms and objects below vnum 500 are officially reserved for the dawn codebase.");
		if(!HAS_SECURITY(ch,9)){
			ch->println("As a result of this reservation, only those with security 9 can edit in that vnum range.");
			return false;
		}			
	}

	tarea = get_vnum_area( value );
    if (!tarea)
    {
    	ch->println("MPEdit: Vnum not assigned for current area?");
    	return false;
    }

	if( !IS_BUILDER_WHEN_NOT_RESTRICTED( ch, tarea) ){
		ch->printlnf( "Insufficient security to create a mudprog in area '%s'.", tarea->name);
		return false;
	}


    pMcode						= new_mpcode();
    pMcode->vnum				= value;
    pMcode->next				= mudprog_list;
	pMcode->area				= tarea;

	free_string( pMcode->author);
    pMcode->author= str_dup( ch->name);

    mudprog_list                    = pMcode;
    ch->desc->pEdit               = (void *)pMcode;

    ch->println("MudProgram Code Created.");
    return true;
}

/**************************************************************************/
bool mpedit_show(char_data *ch, char*)
{
    MUDPROG_CODE *pMcode;

    EDIT_MPCODE(ch,pMcode);

	ch->println("`s=============================================================================`x");	  
    ch->printlnf("`=rVnum: `x%5d     `=rArea[%d]: `x%s",
           pMcode->vnum, 
		   (pMcode->area ? pMcode->area->vnum : 0),
		   pMcode->area ? pMcode->area->file_name : "`RNo Area!!!`x");

    ch->printlnf("`=rTitle: `r%s  `=rAuthor(s): `r%s",
   		   (IS_NULLSTR(pMcode->title)?"`Sunknown":pMcode->title),
		   (IS_NULLSTR(pMcode->author)?"`Sunknown":pMcode->author));

	if (HAS_SECURITY(ch, 9) && pMcode->last_editdate>0)
	{
		char buf2[MSL];
		strcpy(buf2, (char *)ctime(&pMcode->last_editdate));
		buf2[str_len(buf2)-1]='\0';
		ch->printf("`=rLast editor: `g%s `=ron `g", pMcode->last_editor);
		ch->printlnf("`g%s `S(%s ago)`x", 
			buf2, timediff(pMcode->last_editdate, current_time));
	}

	ch->printlnf("`=rCode:`=R\r\n%s`x", pMcode->code);

	ch->println("`s=============================================================================`x");	  

	if(!IS_NULLSTR(pMcode->disabled_text)){
		if(pMcode->disabled){
			ch->println("`RThis prog is currently disabled for the following reason:`x");
			ch->println("`S(note: To enable the program, simply edit its code).`x");
		}else{
			ch->println("`SThis prog `YWAS`S automatically disabled recently for the following reason:`x");
			ch->println("`S(note: the program is no longer disabled).`x");
		}
		ch->print(pMcode->disabled_text);
		ch->println("`s=============================================================================`x");	  
	}
	// show mobs it is used on
	bool found=false;
	{		
		MOB_INDEX_DATA *mob;
		int hash;
		MUDPROG_TRIGGER_LIST *mpl;

		for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
		{
			for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
			{
				for ( mpl = mob->mob_triggers; mpl; mpl = mpl->next )
				{
					if ( mpl->prog == pMcode )
					{
						if (!found)
						{
							ch->printlnf("`=rMobiles that use mudprog %d follow:`x", pMcode->vnum);
							found=true;
						}
						ch->printlnf("%s %s (%s) - %s (%.10s).", 
							mxp_create_tagf(ch, "molcvnum", "%d", mob->vnum),
							mob->short_descr, mob->player_name, 
							mprog_type_to_name(mpl->trig_type), 
							mpl->trig_phrase);
					}
				}
			}
		}

		if (!found)
		{
			ch->println("Currently no mobiles `Wdirectly`x use this program.");
			ch->println("Note that the code could still be accessed via the 'mp call' command...");
			ch->printlnf("e.g. mp force inventor mp call %d $n", pMcode->vnum);
		}
	}

	// show objects it is used on
	found=false;
	{
		bool found=false;
		OBJ_INDEX_DATA *obj;
		int hash;
		MUDPROG_TRIGGER_LIST *mpl;

		for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
		{
			for ( obj = obj_index_hash[hash]; obj; obj = obj->next )
			{
				for ( mpl = obj->obj_triggers; mpl; mpl = mpl->next )
				{
					if ( mpl->prog == pMcode )
					{
						if (!found)
						{
							ch->printlnf("`=rObjects that use mudprog %d follow:`x", pMcode->vnum);
							found=true;
						}
						ch->printlnf("%s %s - %s (%.10s).", 
							mxp_create_tagf(ch, "oolcvnum", "%d", obj->vnum),
							obj->short_descr,
							flag_string(oprog_flags, mpl->trig_type), 
							mpl->trig_phrase);
					}
				}
			}
		}

		if (!found)
		{
			ch->println("Currently no objects directly use this program.");
		}
	}

	ch->println("`s=============================================================================`x");	  
    return false;
}

/**************************************************************************/
bool mpedit_code(char_data *ch, char*argument)
{
    MUDPROG_CODE *pMcode;
    EDIT_MPCODE(ch, pMcode);

    if (argument[0] =='\0')
    {
		if(pMcode->disabled){ // easiest place to put reenabling, but not the best.
			ch->println("Prog reenabled."); 
			pMcode->disabled=false;
		}
		string_append(ch, &pMcode->code);
		free_string(pMcode->last_editor);
		pMcode->last_editor = str_dup(ch->name);
		pMcode->last_editdate = current_time;
		return true;
    }

    ch->println(" Syntax: code");
    return false;
}


/**************************************************************************/
bool mpedit_title(char_data *ch, char*argument)
{
    MUDPROG_CODE *mprg;
    EDIT_MPCODE(ch, mprg);

	smash_tilde(argument);

    if (argument[0] =='\0')
    {
		ch->println("Syntax:  title [string]");
		return false;
    }
    
	free_string( mprg->title);
    mprg->title= str_dup( argument );

    ch->println("Title set.");
    return true;
}


/**************************************************************************/
bool mpedit_author(char_data *ch, char*argument)
{
    MUDPROG_CODE *mprg;
    EDIT_MPCODE(ch, mprg);

	smash_tilde(argument);

    if (argument[0] =='\0')
    {
		ch->println("Syntax:  author [string]");
		return false;
    }
    
	free_string( mprg->author);
    mprg->author= str_dup( argument );

    ch->println("Authors set.");
    return true;
}


/**************************************************************************/
void do_mplist( char_data *ch, char *argument )
{
	char buf[MSL],sbuf[MSL];
	MUDPROG_CODE *prg;
	int count=0;
    BUFFER *output;
	bool all_progs=false;
	bool search=false;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The mplist command is an olc command, you don't have olc permissions.");
		return;
	}

    output= new_buf();

	argument =one_argument( argument, sbuf );

	if (!IS_NULLSTR(sbuf))
	{
		if (!str_cmp("all", sbuf))
		{
			all_progs=true;
		}
		else
		{
			search=true;
		}
	}

    for( prg = mudprog_list; prg; prg = prg->next )
    {
		if (!search  && !all_progs && prg->area!=ch->in_room->area)
			continue;

		if (search // filter if specifed
			&& !is_name( sbuf, prg->area->file_name ) 
			&& !is_name( sbuf, prg->title) 
			&& !is_name( sbuf, prg->author))
			continue;

		sprintf( buf, "`x%3d`S[`=R%5d`S-`y%-8.8s`g %s%-12.12s`S] `r%s`x\r\n", 
			++count, prg->vnum, area_name(prg->area),
			IS_NULLSTR(prg->author)?"`S":"`g",
			IS_NULLSTR(prg->author)?"unknown":prg->author,
			IS_NULLSTR(prg->title)?"`Sunknown":prg->title);
	    add_buf(output,buf);
    }

	ch->sendpage(buf_string(output));
	free_buf(output);
}
/**************************************************************************/
bool mpedit_delete( char_data *ch, char *)
{
	ch->println("If you want to delete a mudprog, use the 'mpdelete' command");
	return false;
}
/**************************************************************************/
// Kal, Jan 2001
bool mpedit_mpdelete(char_data *ch, char *argument)
{
    MUDPROG_CODE *pMcode;
    EDIT_MPCODE(ch, pMcode);

    if (IS_NULLSTR(argument))
    {
		ch->titlebar("MPDELETE SYNTAX");
        ch->println("Syntax: mpdelete confirm - delete the current mudprog");
        ch->println("Syntax: mpdelete <number> confirm - delete mudprog vnum <number>");
		ch->println("Any mudprog that you delete must meet the following conditions:");
		ch->println("* Must not be used by any mobile.");
		ch->println("* Must be in a vnum range that you have security to edit.");
        ch->wrapln("NOTE: It is strongly recommended that no other mudprogs call the mudprog "
			"you are considering deleting... the easiest method to do this is 'textsearch mudprog <mudprogvnum>'.");
		return false;
    }

    if (IS_NPC(ch) || ch->pcdata->security < MPEDIT_MINSECURITY)
    {
        ch->println("MPEdit: Insufficient security to delete MudProgs.");
        return false;
    }

	// support specifying the mudprog by number
	char arg1[MIL];
	MUDPROG_CODE *pDeleteMcode;	
	argument=one_argument(argument, arg1);
	if(is_number(arg1)){
		pDeleteMcode=get_mprog_index(atoi(arg1));
		if(!pDeleteMcode){
			ch->printlnf("mpedit_mpdelete(): There is no mudprog number %s to delete.", arg1);
			return false;
		}
		if(pDeleteMcode->area!=pMcode->area){
			ch->printlnf("mpedit_mpdelete(): Mudprog %s (%s) is not in the same area as the mudprog you are currently editing is.", 
				arg1, pDeleteMcode->title);
			return false;
		}				
		argument=one_argument(argument, arg1); // put the word 'confirm' into arg1
	}else{
		pDeleteMcode=pMcode; // deleting the mudprog we are currently editing
	}

	if ( !IS_BUILDER( ch, pDeleteMcode->area, BUILDRESTRICT_MUDPROGS) ){
		ch->printlnf( "Insufficient security to delete mudprog %d.", pDeleteMcode->vnum );
		return false;
	}

	// confirm they are using 'confirm'
	if(str_cmp(arg1, "confirm")){
		ch->println("You must confirm your intention to delete a mudprog.");
		mpedit_mpdelete(ch,"");
		return false;
	}

	if(!IS_NULLSTR(ltrim_string(argument))){
		ch->println("Incorrect syntax - too many arguments, or arguments in wrong order.");
		mpedit_mpdelete(ch, "");
		return false;
	}

	// We have the mudprog they are wanting to delete and they have 
	// confirmed they want to delete it, check if it isn't in use
	{
		int in_use=0;
		MOB_INDEX_DATA *mob;
		int hash;
		MUDPROG_TRIGGER_LIST *mpl;

		for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
		{
			for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
			{
				for ( mpl = mob->mob_triggers; mpl; mpl = mpl->next )
				{
					if ( mpl->prog == pDeleteMcode)
					{
						if(in_use==0){
							ch->printlnf("`=rMobiles that use mudprog %d follow:`x", pMcode->vnum);
						}
						ch->printlnf("%d %s (%s) - %s (%.10s).", mob->vnum, 
							mob->short_descr, mob->player_name, 
							mprog_type_to_name(mpl->trig_type), 
							mpl->trig_phrase);
						in_use++;
					}
				}
			}
		}

		if(in_use){
			ch->println("You can't delete a mudprog that is currently in use.");
			mpedit_mpdelete(ch, "");
			return false;
		}
	}

	if(pMcode==pDeleteMcode){
		edit_done(ch);
	}
	ch->printlnf("Deleting mudprog %d.", pDeleteMcode->vnum);
	// remove the mudprog from the list of progs
	{
		MUDPROG_CODE *prg, *prev=NULL;
		for( prg = mudprog_list; prg; prg = prg->next )
		{
			if(prg==pDeleteMcode){
				break;
			}
			prev=prg;
		}
		if(pDeleteMcode!=prg){
			bugf("mpedit_mpdelete(): Couldn't find mudprog %d in mudprog_list!!!",
				pDeleteMcode->vnum);
			do_abort();
		}
		if(prev){ // anywhere in list but first
			prev->next=pDeleteMcode->next;
		}else{ // first in list
			mudprog_list=mudprog_list->next;
		}
	}    
	free_mpcode(pDeleteMcode);
	mudprog_count--;

	ch->println("Mudprog deleted.");
    return true;
}

/**************************************************************************/
// - Kal, April 04
bool mpedit_indent(char_data *ch, char*argument)
{
    MUDPROG_CODE *pMcode;
    EDIT_MPCODE(ch, pMcode);	

	char *old_code=pMcode->code;
	if(!str_cmp(argument, "auto") && ch->pcdata){
		pMcode->code=str_dup(indent_mobprog_code(ch, pMcode->code, ch->pcdata->mpedit_autoindent));
	}else{
		pMcode->code=str_dup(indent_mobprog_code(ch, pMcode->code, atoi(argument)));
	}
	free_string(old_code);

	ch->titlebar("INDENTED CODE");
	ch->println( pMcode->code);
	ch->titlebar("");
    return false;
}
/**************************************************************************/
bool mpedit_autoindent(char_data *ch, char*argument)
{
    if(IS_NULLSTR(argument) || !is_number(argument))
    {
		ch->titlebar("AUTOINDENT");
        ch->println("Syntax: autoindent -1       - turn off autoindent of mobprog code");
        ch->println("Syntax: autoindent  0       - Use the mudwide default autoindent amount");
        ch->println("Syntax: autoindent <number> - Specify an autoindent amount");
		ch->wrapln("This function either turns off the automatic indentation of mobprog code "
			"or specifies the number of spaces to indent each block of mobprog code.");
        ch->wrapln("NOTE: <number> must be in the range 1 to 8.");
		return false;
    }
    if (IS_NPC(ch))
    {
        ch->println("MPEditAutoIndent: players only.");
        return false;
    }

	int value=URANGE(-1, atoi(argument), 8);

	if(ch->pcdata->mpedit_autoindent==value){
		ch->printlnf("Your autoindent is already set to %d", value);
	}else{
		if(value==-1){
			ch->println("Autoindent turned off.");
		}else{
			ch->printlnf("Autoindent changed from %d to %d.", ch->pcdata->mpedit_autoindent, value);
		}
		ch->pcdata->mpedit_autoindent=value;
	}
	
	return true;
}

/**************************************************************************/
// Duplicate the mudprog from another 
// Kal - Nov 05
bool mpedit_mpcopy(char_data *ch, char *argument)
{
    MUDPROG_CODE *pMcode;
	MUDPROG_CODE *pSrc; // source mudprog
	char arg1[MIL];
	int value;

    argument = one_argument( argument, arg1 );

    if ( !is_number( arg1 ) )
    {
        ch->println("Syntax: mpcopy <source mudprog vnum>");
        ch->println("  - copies the source mudprog over the mudprog you are currently editing!");
        ch->println("    (warning copies over everything!)");
        return false;
	}

    value = atoi( arg1 );
	if (! (pSrc = get_mprog_index( value ) ) )
	{
		ch->printlnf("mpcopy: There is no mudprog numbered %d.", value);
		return false;
	}

	// perform some basic security checks
	if ( !IS_BUILDER( ch, pSrc->area, BUILDRESTRICT_MUDPROGS )
	&& !IS_IMMORTAL(  ch ))
	{
		ch->printlnf("mpshow: Insufficient security for accessing/viewing mudprogs the area which prog %d belongs (%s).",
			pSrc->vnum, pSrc->area->name);
		return false;
	}

	if( !IS_BUILDER_WHEN_NOT_RESTRICTED( ch, pSrc->area) ){
		ch->printlnf( "Insufficient security to access a mudprog in area '%s'.", pSrc->area->name);
		return false;
	}

    if ( !IS_BUILDER( ch, pSrc->area, BUILDRESTRICT_MOBS) && !IS_IMMORTAL(ch) )
    {
        ch->println( "Insufficient security to copy from the area that mob");
        ch->println("is stored in and your arent an immortal.");
        return false;
    }


    EDIT_MPCODE(ch, pMcode);

	replace_string(pMcode->author, pSrc->author);
	replace_string(pMcode->code, pSrc->code);
	replace_string(pMcode->title, pSrc->title);	

	ch->wraplnf("`=rCopied mudprog '%s'[%d] to vnum %d", 
				pSrc->title, pSrc->vnum, pMcode->vnum);
    return true;
}
/**************************************************************************/
bool mpedit_copy( char_data *ch, char *)
{
	ch->println("If you want to copy a mudprog, use the 'mpcopy' command");
	return false;
}

/**************************************************************************/
/**************************************************************************/

