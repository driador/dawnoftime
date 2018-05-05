/**************************************************************************/
// note.cpp - Note system
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
#include "notenet.h"

/**************************************************************************/
bool notes_loaded=false;	// when this is false, append_note() wont 
		// create a new thread but still write to the file.  This is 
		// useful because autonote() is called before the note spools
		// are loaded to send notes during the bootup process.
		// - Kal, Jul 01
/**************************************************************************/
// globals from db.c for load_notes 
extern FILE * fpArea;
extern char   strArea[MIL];

DECLARE_DO_FUN( do_huh    );

/**************************************************************************/
// local procedures 
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time);
void parse_note(char_data *ch, char *argument, int type);
bool hide_note(char_data *ch, NOTE_DATA *pnote);
int	 note_lookup( const char *name );

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *penalty_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;

NOTE_DATA *anote_list;
NOTE_DATA *inote_list;
NOTE_DATA *misc_list;
NOTE_DATA *snote_list;

NOTE_DATA *pknote_list;

/**************************************************************************/
struct note_types
{
    char *name, *names, *lname, *lnames;
};

/**************************************************************************/
// budget hack for lowercase names
const struct note_types note_table [] =
{
    {"Note"     , "Notes"		,"note"     , "notes"		},      //  0
    {"Idea"     , "Ideas"		,"idea"     , "ideas"		},      //  1
    {"Penalty"  , "Penalties"	,"penalty"  , "penalties"	},      //  2
    {"News"     , "News"		,"news"     , "news"		},      //  3
    {"Change"   , "Changes"		,"change"   , "changes"		},      //  4
    {"Anote"    , "Anotes"		,"anote"    , "anotes"		},      //  5
    {"Inote"    , "Inotes"		,"inote"    , "inotes"		},      //  6
    {"Misc"		, "Misc"		,"misc"		, "misc"		},      //  7
    {"Snote"	, "Snotes"		,"snote"	, "snotes"		},      //  8
    {"PKnote"	, "PKnotes"		,"pknote"	, "pknotes"		},      //  8
	{ NULL		, NULL			,NULL		, NULL			}
};

/**************************************************************************/
int note_lookup (const char *name)
{
    int note;

    if ( IS_NULLSTR( name )) {
        log_string( "BUG: note_lookup: was past a NULL string!" );
        return -1;
    }

    for ( note = 0; note_table[note].name != NULL; note++ ) {

		if ( LOWER( name[0] ) == LOWER( note_table[note].name[0] )
			&& !str_prefix( name, note_table[note].name ))
			return note;
    }

    return -1;
}
/**************************************************************************/
extern CClanType *clan_list;
/**************************************************************************/
bool is_note_to_new( char_data *ch, NOTE_DATA *pnote, bool reading_all)
{
	char names_list[MSL];
	sprintf(names_list, "%s %s", pnote->to_list,pnote->cc_list);
	strcpy(names_list,strip_colour(names_list));

    if ( !str_cmp( ch->name, pnote->sender ) )
		return true;

	if ( is_exact_name( ch->name, names_list ) )
		return true;

	// prevent pknote spool from being used by morts to write notes 
	// to each other, (Apart from when a players exact name is used)
	if(pnote->type== NOTE_PKNOTE && !IS_IMMORTAL(ch) && !IS_NOBLE(ch)){
		return false;
	}

    if ( is_exact_name( "all", names_list ) )
		return true;

	if ( IS_IMMORTAL( ch )
		|| class_table[ch->clss].class_cast_type == CCT_BARD
		|| HAS_CONFIG(ch, CONFIG_BARD_COUNCIL ))
	{
		if ( is_exact_name( "bard", names_list ))
			return true;
	}

	if (IS_IMMORTAL(ch)){
		if ( is_exact_name( "council", names_list )){
			return true;
		}
		// next loop thru all the council names as defined in council_flags[]
		// admin can read all those notes
		{
			int index;

			for (index = 0; !IS_NULLSTR(council_flags[index].name); index++)
			{
				if(is_exact_name( council_flags[index].name, names_list )){
					if (IS_ADMIN(ch)
						|| IS_SET( TRUE_CH(ch)->pcdata->council, council_flags[index].bit))
						return true;
				}
			}
		}
	}
 
	if (IS_NOBLE(ch))
	{
		if ( is_name( "noble", names_list ) && 
			!is_exact_name( "noblepkill", names_list) )
			return true;

		if ((!reading_all || !IS_SET(ch->notenet, NOTE_TO_PKILL))
			&& is_exact_name( "pkill", names_list ))
			return true;

		if ((!reading_all || !IS_SET(ch->notenet, NOTE_NOBLEPKILL))
			&& is_name( "noblepkill", names_list ))
			return true;

		if (!str_cmp(pnote->sender, "A noble"))
			return true;	
	}

    if (GAMESETTING2(GAMESET2_AUTONOTE_RENAMES_TO_ADMIN)
		&& IS_ADMIN(ch) 		
		&& is_exact_name( "adminrename", names_list )){	
		if (!reading_all || !IS_SET(ch->notenet, NOTE_ADMIN_RENAME)){
			return true;
		}else{
			return false;
		}
	}

    if ( IS_IMMORTAL(ch))
	{
		if (is_name( "imm", names_list ) ){
			if (is_exact_name( "immpkilldetails", names_list ) 
				|| is_exact_name( "immpkill", names_list )){
				// do nothing - handled lower
			}else{
				return true;
			}
		}
			

		if ((!reading_all || !IS_SET(ch->notenet, NOTE_TO_PKILL))
			&& is_exact_name( "pkill", names_list ))
			return true;

		if ((!reading_all || !IS_SET(ch->notenet, NOTE_IMMPKILL))
			&& is_name( "immpkill", names_list ) )
			return true;	

		if ((!reading_all || !IS_SET(ch->notenet, NOTE_IMMPKILLDETAILS))
			&& is_name( "immpkilldetails", names_list ) )
			return true;

		// court notes
		if ((!reading_all || !IS_SET(ch->notenet, NOTE_COURT))
			&& is_name( "court", names_list ) )
			return true;

	}else{ // non imms getting court notes
		if(HAS_CONFIG(ch, CONFIG_COURTMEMBER)) {
			if ( is_name( "court", names_list ) ){
				return true;
			}
		}
	}

	if (( ch->level==LEVEL_HERO 
				|| IS_ADMIN(ch)) && is_name( "hero", names_list )  )
			return true;

	if ( (IS_IMMORTAL(ch) || IS_QUESTER(ch))
		&& ( is_exact_name( "quester", names_list)))
		return true;

	if ( (IS_IMMORTAL(ch) || !IS_QUESTER(ch))
		&& ( is_exact_name( "nonquester", names_list)))
		return true;

	if ( HAS_SECURITY(ch,1) 
		&& ( is_name( "build", names_list)))
		return true;

	if ( HAS_SECURITY(ch, 5) && is_name( "olc", names_list)) 
		return true;

    if ( IS_ADMIN(ch) && is_name( "admin", names_list ) )
		return true;

    if ( TRUE_CH(ch)->level>=MAX_LEVEL && is_name( "imp", names_list ) )
		return true;

    if ( (IS_NEWBIE_SUPPORT(ch)||IS_IMMORTAL(ch)) && is_name( "nsupport", names_list ) )
		return true;

    if ( (IS_RP_SUPPORT(ch)||IS_IMMORTAL(ch)) && is_name( "rpsupport", names_list ) )
		return true;

	if(GAMESETTING5(GAMESET5_NOTES_TO_RACE_NAMES_SUPPORTED)){
		// racial notes
		if (is_exact_name(race_table[ch->race]->name, names_list)){
 			return true;
		}

		// immortals see all racial notes
		if(IS_IMMORTAL(ch)){
			int race;
			for ( race = 0; race_table[race]; race++)
			{
				if(is_exact_name(race_table[race]->name, names_list)){
					return true;
				}
			}
		}
	}	

	if(GAMESETTING5(GAMESET5_NOTES_TO_CLASS_NAMES_SUPPORTED)){
		// class notes
		if (is_exact_name(race_table[ch->race]->name, names_list)){
			return true;
		}
		
		// immortals see all class notes
		if(IS_IMMORTAL(ch)){
			int classIndex;
			for ( classIndex = 0; !IS_NULLSTR(class_table[classIndex].name); classIndex++)
			{	
				if(is_exact_name(class_table[classIndex].name, names_list)){
					return true;
				}
			}
		}
	}

    if (ch->clan && is_exact_name(ch->clan->notename(),names_list)){
		return true;
	}

	if ( ch->clan || IS_IMMORTAL(ch)) {
		if ((!reading_all || !IS_SET(ch->notenet, NOTE_TO_ALLCLAN))
			&& is_exact_name( "allclan", names_list ))
			return true;
	}

	if (IS_IMMORTAL(ch) && (!reading_all || !IS_SET(ch->notenet, NOTE_CLANNOTES)) )
	{
		CClanType *pClan;

	    for (pClan=clan_list; pClan; pClan=pClan->next)
		{
            if (is_exact_name(pClan->notename(), names_list) )
			{
				return true;
			}
		}
	}

    return false;
}

/**************************************************************************/
bool is_note_to( char_data *ch, NOTE_DATA *pnote)
{	
	return is_note_to_new( ch, pnote, false);
}

/**************************************************************************/
int count_spool(char_data *ch, NOTE_DATA *spool)
{
    int count = 0;
    NOTE_DATA *pnote;

    for (pnote = spool; pnote != NULL; pnote = pnote->next)
	if (!hide_note(ch,pnote))
	    count++;

    return count;
}


/**************************************************************************/
int count_notes(char_data *ch, NOTE_DATA *nlist)
{
    int count = 0;
    NOTE_DATA *pnote;

    for (pnote = nlist; pnote != NULL; pnote = pnote->next)
    {
		if (ch)
		{
			if (is_note_to(ch,pnote))
				count++;
		}
		else
			count++;
    }
    return count;
}

/**************************************************************************/
void show_note_tilde (NOTE_DATA *pnote)
{
    // swaps ­ (ascii 173) for the tilde ~
    show_tilde (pnote->sender);
    show_tilde (pnote->real_sender);
    show_tilde (pnote->date);
    show_tilde (pnote->to_list);
	show_tilde (pnote->cc_list);
    show_tilde (pnote->subject);
    show_tilde (pnote->text);
}


/**************************************************************************/
void hide_note_tilde (NOTE_DATA *pnote)
{
    // swaps the tilde for ­ (ascii 173) 
    hide_tilde (pnote->sender);
    hide_tilde (pnote->real_sender);
    hide_tilde (pnote->date);
    hide_tilde (pnote->to_list);
	hide_tilde (pnote->cc_list);
    hide_tilde (pnote->subject);
    hide_tilde (pnote->text);
}

/**************************************************************************/
void do_unread(char_data *ch, char *)
{
    int count;

    if (IS_NPC(ch))
		return; 

    if ((count = count_spool(ch,news_list)) > 0)
    {
		ch->printlnf( "`sThere %s `x%d`s new `xnews`s article%s waiting.`x",
			count > 1 ? "are" : "is",count, count > 1 ? "s" : "");
	}
	else 
		ch->println( "`sYou have `xno`s unread `xnews`s.`x" );

	if ((count = count_spool(ch,changes_list)) > 0)
	{
		ch->printlnf( "`sThere %s `x%d`s `xchange`s%s waiting to be read.`x",
			count > 1 ? "are" : "is", count, count > 1 ? "s" : "");
	}
	else
		ch->println( "`sYou have `xno`s unread `xchange`ss.`x" );

	if ((count = count_spool(ch,note_list)) > 0)
	{
		ch->printlnf( "`sYou have `x%d`s new `xnote`s%s waiting.`x",
			count, count > 1 ? "s" : "");
	}
	else
		ch->println( "`sYou have `xno`s unread `xnote`ss.`x" );

	if ((count = count_spool(ch,idea_list)) > 0)
	{
		ch->printlnf( "`sYou have `x%d`s unread `xidea`s%s to peruse.`x",
			count, count > 1 ? "s" : "");
	}
	else
		ch->println( "`sYou have `xno`s unread `xidea`ss.`x" );

	if (IS_TRUSTED(ch,ANGEL) && (count = count_spool(ch,penalty_list)) > 0)
	{
		ch->printlnf( "`x%d`x %s been added.`x",
			count, count > 1 ? "`xpenalties`s have" : "`xpenalty`s has");
	}

	if (IS_ADMIN(ch))
	{
		if ( (count = count_spool(ch,anote_list))>0 )
		{
			ch->printlnf( "`x%d`x %s been added.`x",
				count, count > 1 ? "`xanotes`s have" : "`xanote`s has");
		}
		else
			ch->println( "`sYou have `xno`s unread `xanote`ss.`x" );
	}

	if (IS_IMMORTAL(ch))
	{
		if (( count = count_spool(ch,inote_list)) > 0)
		{
			ch->printlnf( "`x%d`x %s been added.`x",
				count, count > 1 ? "`xinotes`s have" : "`xinote`s has");
		}
		else
			ch->println( "`sYou have `xno`s unread `xinote`ss.`x" );
	}

	if (HAS_CONFIG(ch, CONFIG_SHOWMISC) && !HAS_CONFIG(ch, CONFIG_NOMISC))
	{
		if ((count = count_spool(ch,misc_list)) > 0)
		{
			ch->printlnf( "`sYou have `x%d`s new `xmisc`s note%s waiting to be endured.`x",
				count, count > 1 ? "s" : "");
		}
		else
			ch->println( "`sYou have `xno`s unread `xmisc`s notes to endure.`x" );
	}

	if ( IS_IMMORTAL(ch)){	//////// temp!! til we go live
		if ((count = count_spool(ch,snote_list)) > 0)
		{
			ch->printlnf( "`sYou have `x%d`s new `xsnote`s%s waiting to be read.`x",
				count, count > 1 ? "s" : "");
		}
		else
			ch->println( "`sYou have `xno`s unread `xsnote`ss.`x" );
	}

	if (( count = count_spool(ch,pknote_list)) > 0)
	{
		ch->printlnf("`sThere %s %d %s.`x",
			count != 1?"are":"is",
			count, count != 1 ? "`xpknotes`s" : "`xpknote`s");
	}
}

/**************************************************************************/
void do_note(char_data *ch,char *argument)
{
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED )){
		if ( ch->master ){
			ch->master->println( "Not going to happen." );

		}
		return;
	}
    
	parse_note(ch,argument,NOTE_NOTE);
}

/**************************************************************************/
void do_idea(char_data *ch,char *argument)
{
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}

    parse_note(ch,argument,NOTE_IDEA);
}

/**************************************************************************/
void do_penalty(char_data *ch,char *argument)
{
    parse_note(ch,argument,NOTE_PENALTY);
}

/**************************************************************************/
void do_news(char_data *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NEWS);
}

/**************************************************************************/
void do_changes(char_data *ch,char *argument)
{
    parse_note(ch,argument,NOTE_CHANGES);
}

/**************************************************************************/
void do_anote(char_data *ch,char *argument)
{
	if (!IS_ADMIN(ch))
	{
		do_huh(ch,"");
		return;
	}	
    parse_note(ch,argument,NOTE_ANOTE);
}

/**************************************************************************/
void do_inote(char_data *ch,char *argument)
{
    parse_note(ch,argument,NOTE_INOTE);
}

/**************************************************************************/
void do_snote(char_data *ch,char *argument)
{
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println( "Not going to happen." );
		return;
	}
	parse_note(ch,argument,NOTE_SNOTE);
}

/**************************************************************************/
void do_pknote(char_data *ch,char *argument)
{
    parse_note(ch,argument,NOTE_PKNOTE);
}

/**************************************************************************/
void do_misc(char_data *ch,char *argument)
{
	if(HAS_CONFIG(ch, CONFIG_NOMISC)){
		do_huh(ch,"");
		return;
	}

    parse_note(ch,argument,NOTE_MISC);
}

/**************************************************************************/
void save_notes(int type)
{
    FILE *fp;
    char *name;
    NOTE_DATA *pnote;

    switch (type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    pnote = note_list;
	    break;
	case NOTE_IDEA:
	    name = IDEA_FILE;
	    pnote = idea_list;
	    break;
	case NOTE_PENALTY:
	    name = PENALTY_FILE;
	    pnote = penalty_list;
	    break;
	case NOTE_NEWS:
	    name = NEWS_FILE;
	    pnote = news_list;
	    break;
	case NOTE_CHANGES:
	    name = CHANGES_FILE;
	    pnote = changes_list;
	    break;
	case NOTE_ANOTE:
	    name = ANOTE_FILE;
	    pnote = anote_list;
	    break;
	case NOTE_INOTE:
	    name = INOTE_FILE;
	    pnote = inote_list;
	    break;
	case NOTE_MISC:
	    name = MISC_FILE;
	    pnote = misc_list;
	    break;
	case NOTE_SNOTE:
	    name = SNOTE_FILE;
	    pnote = snote_list;
	    break;
	case NOTE_PKNOTE:
	    name = PKNOTE_FILE;
	    pnote = pknote_list;
	    break;
    }

    fclose( fpReserve );
    if ( ( fp = fopen( name, "w" ) ) == NULL ){
        bugf("save_notes(): fopen '%s' for write - error %d (%s)",
			name, errno, strerror( errno));
    }else{
		for ( ; pnote != NULL; pnote = pnote->next )
		{
			hide_note_tilde(pnote);
			fprintf( fp, "Sender  %s~\n", pnote->sender);
			fprintf( fp, "Real    %s~\n", pnote->real_sender);
			fprintf( fp, "Date    %s~\n", pnote->date);
			fprintf( fp, "Stamp   %ld\n", (long) pnote->date_stamp);
			fprintf( fp, "To      %s~\n", pnote->to_list);
			if(!IS_NULLSTR(pnote->cc_list)){
				fprintf( fp, "Cc      %s~\n", pnote->cc_list);
			}
			fprintf( fp, "Subject %s~\n", pnote->subject);
			fprintf( fp, "Text\n%s~\n",   pnote->text);
			show_note_tilde(pnote);
		}
		fclose( fp );
    }
	fpReserve = fopen( NULL_FILE, "r" );
   	return;
}

/**************************************************************************/
void load_notes(void)
{	
	logf("Loading notes...");
	int notes_decay=1;
	if(GAMESETTING5(GAMESET5_KEEP_NOTE_SPOOLS_INDEFINATELY)){
		notes_decay=0;
	}
	
    load_thread(NOTE_FILE,&note_list, NOTE_NOTE, 14*24*60*60*notes_decay);		// 2 WEEKS
    load_thread(IDEA_FILE,&idea_list, NOTE_IDEA, 28*24*60*60*notes_decay);		// 1 MONTH
    load_thread(PENALTY_FILE,&penalty_list, NOTE_PENALTY, 0*notes_decay);		// PERM
    load_thread(NEWS_FILE,&news_list, NOTE_NEWS, 0*notes_decay);				// PERM
    load_thread(CHANGES_FILE,&changes_list,NOTE_CHANGES, 0*notes_decay);		// PERM
    load_thread(ANOTE_FILE,&anote_list,NOTE_ANOTE, 2*28*24*60*60*notes_decay);	// 2 MONTHS
    load_thread(INOTE_FILE,&inote_list,NOTE_INOTE, 2*28*24*60*60*notes_decay);	// 2 MONTHS
    load_thread(MISC_FILE,&misc_list,NOTE_MISC, 14*24*60*60*notes_decay);		// 2 WEEKS
    load_thread(SNOTE_FILE,&snote_list,NOTE_SNOTE, 14*24*60*60*notes_decay);	// 2 WEEKS
    load_thread(PKNOTE_FILE,&pknote_list,NOTE_PKNOTE, 28*24*60*60*notes_decay); // 1 MONTH
	notes_loaded=true;	
}

/**************************************************************************/
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
    FILE *fp;
    NOTE_DATA *pnotelast;
	int count=0;
	char *keyword="";
	
	update_currenttime();

    if ( ( fp = fopen( name, "r" ) ) == NULL )
	return;

	logf("- Reading in %s thread from %s...", note_table[type].lnames, name);
	 
    pnotelast = NULL;
    for ( ; ; )
    {
		NOTE_DATA *pnote;
		char letter;
		
		do
		{
			letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
				if(count){
					logf("- %d %s read into the %s spool.", 
						count,
						count==1?note_table[type].lname:note_table[type].lnames, 
						note_table[type].lname);
				}else{
					logf("- no %s were read into the %s spool.", 
						note_table[type].lname, 
						note_table[type].lnames);
				}
	            return;
            }
        }

        while ( is_space(letter) ); // trim off all the whitespace
        ungetc( letter, fp );

        pnote = new_note();
		
		keyword=fread_word( fp );
        if ( str_cmp( keyword, "sender" ) )
            break;
        pnote->sender   = fread_string( fp );
		
		keyword=fread_word( fp );
		if ( str_cmp( keyword, "real") )
			break;
        pnote->real_sender   = fread_string( fp ); 	
		
		keyword=fread_word( fp );
        if ( str_cmp( keyword, "date" ) )
            break;
        pnote->date     = fread_string( fp );
		
		keyword=fread_word( fp );
        if ( str_cmp( keyword, "stamp" ) )
            break;
        pnote->date_stamp = fread_number(fp);

		
		keyword=fread_word( fp );
        if ( str_cmp( keyword, "to" ) )
            break;
        pnote->to_list  = fread_string( fp );

		keyword=fread_word( fp );
		if ( !str_cmp( keyword, "cc" ) ){
	        pnote->cc_list  = fread_string( fp );
			keyword=fread_word( fp );
		}
        if ( str_cmp( keyword, "subject" ) )
            break;

		pnote->subject  = fread_string( fp );
		if(str_len(pnote->subject)> 600){
			bugf("Note subject length %d, trimming to 600.", str_len(pnote->subject));
			pnote->subject[600]='\0';
		}
		
		keyword=fread_word( fp );
        if ( str_cmp( keyword, "text" ) )
            break;
        pnote->text     = fread_string( fp );
		if(str_len(pnote->text)> MSL*2){
			bugf("Note text length %d, trimming to %d.", str_len(pnote->text), MSL*2);
			pnote->text[MSL*2]='\0';
		}
		
        if (free_time && pnote->date_stamp < current_time - free_time)
        {
			free_note(pnote);
            continue;
        }
		
		show_note_tilde(pnote);
		
		pnote->type = type;
		count++;
		
        if (*list == NULL){
            *list           = pnote;
		}else{
            pnotelast->next     = pnote;
		}
		
        pnotelast       = pnote;
    }
 
    strcpy( strArea, name);
    fpArea = fp;
    bugf("Load_notes: bad keyword '%s'.", keyword);
    exit_error( 1 , "load_notes", "bad keyword");
    return;
}

/**************************************************************************/
void append_note(NOTE_DATA *pnote)
{
    FILE *fp;
    char *name;
    NOTE_DATA **list;
    NOTE_DATA *last;

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    name = IDEA_FILE;
	    list = &idea_list;
	    break;
	case NOTE_PENALTY:
	    name = PENALTY_FILE;
	    list = &penalty_list;
	    break;
	case NOTE_NEWS:
	     name = NEWS_FILE;
	     list = &news_list;
	     break;
	case NOTE_CHANGES:
	     name = CHANGES_FILE;
	     list = &changes_list;
	     break;
	case NOTE_ANOTE:
	     name = ANOTE_FILE;
	     list = &anote_list;
	     break;
	case NOTE_INOTE:
	     name = INOTE_FILE;
	     list = &inote_list;
	     break;
	case NOTE_MISC:
	     name = MISC_FILE;
	     list = &misc_list;
	     break;
	case NOTE_SNOTE:
	     name = SNOTE_FILE;
	     list = &snote_list;
	     break;
	case NOTE_PKNOTE:
	     name = PKNOTE_FILE;
	     list = &pknote_list;
	     break;
    }

    if (*list == NULL){
		if(notes_loaded){ // if notes aren't loaded, don't create a new spool
			*list = pnote;
		}
    }else{
		for ( last = *list; last->next != NULL; last = last->next);{
			last->next = pnote;
		}
    }

    fclose(fpReserve);
    if ( ( fp = fopen(name, "a" ) ) == NULL ){
        bugf("append_note(): fopen '%s' for append - error %d (%s)",
			name, errno, strerror( errno));
    }else{
		// swaps the tilde for ­ (ascii 173)
        hide_note_tilde(pnote);

        fprintf( fp, "Sender  %s~\n", pnote->sender);
        fprintf( fp, "Real    %s~\n", pnote->real_sender);
        fprintf( fp, "Date    %s~\n", pnote->date);
        fprintf( fp, "Stamp   %ld\n", (long) pnote->date_stamp);
        fprintf( fp, "To      %s~\n", pnote->to_list);
		if(!IS_NULLSTR(pnote->cc_list)){
			fprintf( fp, "Cc      %s~\n", pnote->cc_list);
		}
        fprintf( fp, "Subject %s~\n", pnote->subject);
        fprintf( fp, "Text\n%s~\n", pnote->text);
        fclose( fp );

        show_note_tilde(pnote);
    }
    fpReserve= fopen( NULL_FILE, "r" );

}

/**************************************************************************/
void note_attach( char_data *ch, int type )
{
    NOTE_DATA *pnote;

    if ( ch->pnote != NULL )
	return;

    pnote = new_note();

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->name );
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
	pnote->cc_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    pnote->type		= type;
    ch->pnote		= pnote;

    return;
}


/**************************************************************************/
void note_remove( char_data *ch, NOTE_DATA *pnote, bool deleting)
{
    NOTE_DATA *prev;
    NOTE_DATA **list;
	char *lower;

    if(!deleting && str_cmp( ch->name, pnote->real_sender ))
    {
		// this section of code is run when 'note remove' by a recipient (and not the real sender)
				
		// remove the recipients name from the to and/or the cc list
		if(is_exact_name(ch->name, pnote->to_list)){
			lower=lowercase(pnote->to_list);
			replace_string(pnote->to_list, lower);
			pnote->to_list=string_replace_all(pnote->to_list, lowercase(ch->name), "");
			pnote->to_list=string_replace_all(pnote->to_list, "  ", " ");
		}

		if(is_exact_name(ch->name, pnote->cc_list)){
			lower=lowercase(pnote->cc_list);
			replace_string(pnote->cc_list, lower);
			pnote->cc_list=string_replace_all(pnote->cc_list, lowercase(ch->name), "");
			pnote->cc_list=string_replace_all(pnote->cc_list, "  ", " ");
		}

		// if recipients remain, then resave the note, otherwise we will delete the note
		if(!IS_NULLSTR(ltrim_string(pnote->to_list)) || !IS_NULLSTR(ltrim_string(pnote->cc_list))){
			// the note still needs to be kept, so resave the spool
			save_notes(pnote->type);
			return;
		}
	}

    // deleting the entire note 
    switch(pnote->type)
    {
		default:
			return;
		case NOTE_NOTE:
			list = &note_list;
			break;
		case NOTE_IDEA:
			list = &idea_list;
			break;
		case NOTE_PENALTY:
			list = &penalty_list;
			break;
		case NOTE_NEWS:
			list = &news_list;
			break;
		case NOTE_CHANGES:
			list = &changes_list;
			break;
		case NOTE_ANOTE:
			list = &anote_list;
			break;
		case NOTE_INOTE:
			list = &inote_list;
			break;
		case NOTE_MISC:
			list = &misc_list;
			break;
		case NOTE_SNOTE:
			list = &snote_list;
			break;
		case NOTE_PKNOTE:
			list = &pknote_list;
			break;
	}

    // Remove note from linked list.
    if ( pnote == *list )
    {
		*list = pnote->next;
    }
    else
    {
		for ( prev = *list; prev != NULL; prev = prev->next )
		{
			if ( prev->next == pnote )
			break;
		}

		if ( prev == NULL )
		{
		    bug("Note_remove: pnote not found.");
		    return;
		}

		prev->next = pnote->next;
    }

    save_notes(pnote->type);
    free_note(pnote);
    return;
}

/**************************************************************************/
bool new_hide_note(char_data *ch, NOTE_DATA *pnote, bool reading_all){
    time_t last_read;

    if (IS_NPC(ch))
	return true;

    switch (pnote->type)
    {
	default:
		return true;
	case NOTE_NOTE:
		last_read = ch->pcdata->last_note;
		break;
	case NOTE_IDEA:
		last_read = ch->pcdata->last_idea;
		break;
	case NOTE_PENALTY:
		last_read = ch->pcdata->last_penalty;
		break;
	case NOTE_NEWS:
		last_read = ch->pcdata->last_news;
		break;
	case NOTE_CHANGES:
		last_read = ch->pcdata->last_changes;
		break;
	case NOTE_ANOTE:
		last_read = ch->pcdata->last_anote;
		break;
	case NOTE_INOTE:
		last_read = ch->pcdata->last_inote;
		break;
	case NOTE_MISC:
		last_read = ch->pcdata->last_misc;
		break;
	case NOTE_SNOTE:
		last_read = ch->pcdata->last_snote;
		break;
	case NOTE_PKNOTE:
		last_read = ch->pcdata->last_pknote;
		break;
    }
    
    if (pnote->date_stamp <= last_read)
	return true;

    if (!str_cmp(ch->name,pnote->sender))
	return true;

	if (!is_note_to_new(ch,pnote, reading_all))
		return true;

    return false;
}

/**************************************************************************/
bool hide_note (char_data *ch, NOTE_DATA *pnote)
{
	return new_hide_note(ch, pnote, false);
}

/**************************************************************************/
void update_read(char_data *ch, NOTE_DATA *pnote)
{
	time_t stamp;
	
	if (IS_NPC(ch) || !pnote)
		return;

	stamp = pnote->date_stamp;

	switch (pnote->type)
	{
	default:
		return;
	case NOTE_NOTE:
		ch->pcdata->last_note		= UMAX(ch->pcdata->last_note,stamp);
		break;
	case NOTE_IDEA:
		ch->pcdata->last_idea		= UMAX(ch->pcdata->last_idea,stamp);
		break;
	case NOTE_PENALTY:
		ch->pcdata->last_penalty	= UMAX(ch->pcdata->last_penalty,stamp);
		break;
	case NOTE_NEWS:
		ch->pcdata->last_news		= UMAX(ch->pcdata->last_news,stamp);
		break;
	case NOTE_CHANGES:
		ch->pcdata->last_changes	= UMAX(ch->pcdata->last_changes,stamp);
		break;
	case NOTE_ANOTE:
		ch->pcdata->last_anote		= UMAX(ch->pcdata->last_anote,stamp);
		break;
	case NOTE_INOTE:
		ch->pcdata->last_inote		= UMAX(ch->pcdata->last_inote,stamp);
		break;
	case NOTE_MISC:
		ch->pcdata->last_misc		= UMAX(ch->pcdata->last_misc,stamp);
		break;
	case NOTE_SNOTE:
		ch->pcdata->last_snote		= UMAX(ch->pcdata->last_snote,stamp);
		break;
	case NOTE_PKNOTE:
		ch->pcdata->last_pknote		= UMAX(ch->pcdata->last_pknote,stamp);
		break;
	}
}

/**************************************************************************/
void mark_lastread(char_data *ch, NOTE_DATA *pnote)
{
	time_t stamp;

	if (IS_NPC(ch))
		return;

	stamp = pnote->date_stamp;

	switch (pnote->type)
	{
	default:
		return;
	case NOTE_NOTE:
		ch->pcdata->last_note = stamp;
		break;
	case NOTE_IDEA:
		ch->pcdata->last_idea = stamp;
		break;
	case NOTE_PENALTY:
		ch->pcdata->last_penalty = stamp;
		break;
	case NOTE_NEWS:
		ch->pcdata->last_news = stamp;
		break;
	case NOTE_CHANGES:
		ch->pcdata->last_changes = stamp;
		break;
	case NOTE_ANOTE:
		ch->pcdata->last_anote= stamp;
		break;
	case NOTE_INOTE:
		ch->pcdata->last_inote= stamp;
		break;
	case NOTE_MISC:
		ch->pcdata->last_misc= stamp;
		break;
	case NOTE_SNOTE:
		ch->pcdata->last_snote= stamp;
		break;
	case NOTE_PKNOTE:
		ch->pcdata->last_pknote= stamp;
		break;
	}
}

/**************************************************************************/
// Notify people of new notes 
void do_note_notify(int minlevel, int maxlevel)
{
	connection_data *d2, *d2_next;
	char_data *ch;

	// Tell all characters that have a new note by looping thru all them
    for( d2=connection_list; d2!=NULL; d2=d2_next )
	{
		d2_next = d2->next;
		
		ch = d2->original ? d2->original : d2->character;
		if ((( d2->connected_state == CON_PLAYING)
			&& !IS_NPC(ch)
			&& ch->level >= minlevel && ch->level <= maxlevel ))
		{
			int count;
			
			if ((count = count_spool(ch,news_list)) > 0)
			{
				ch->printlnf( "There %s %d new news article%s waiting.",
					count > 1 ? "are" : "is",count, count > 1 ? "s" : "");
			}

			if ((count = count_spool(ch,changes_list)) > 0)
			{
				ch->printlnf( "There %s %d change%s waiting to be read.",
					count > 1 ? "are" : "is", count, count > 1 ? "s" : "");
			}

			if ((count = count_spool(ch,note_list)) > 0)
			{
				ch->printlnf( "You have %d new note%s waiting.",
					count, count > 1 ? "s" : "");
			}

			if ((count = count_spool(ch,idea_list)) > 0)
			{
				ch->printlnf( "You have %d unread idea%s to peruse.",
					count, count > 1 ? "s" : "");
			}

			if (IS_TRUSTED(ch,ANGEL) && (count = count_spool(ch,penalty_list)) > 0)
			{
				ch->printlnf( "%d %s been added.",
					count, count > 1 ? "penalties have" : "penalty has");
			}

			if (   IS_ADMIN(ch) 
				&& (count = count_spool(ch,anote_list)) > 0)
			{
				ch->printlnf( "`x%d`x %s been added.`x",
					count, count > 1 ? "`xanotes`s have" : "`xanote`s has");
			}

			if (IS_TRUSTED(ch,LEVEL_IMMORTAL)
				&& (count = count_spool(ch,inote_list)) > 0)
			{
				ch->printlnf( "`x%d`x %s been added.`x",
					count, count > 1 ? "`xinotes`s have" : "`xinote`s has" );
			}

			if (HAS_CONFIG(ch, CONFIG_SHOWMISC) && !HAS_CONFIG(ch, CONFIG_NOMISC))
			{
				if ((count = count_spool(ch,misc_list)) > 0)
				{
					ch->printlnf( "You have %d unread misc note%s to endure.",
						count, count > 1 ? "s" : "");
				}
			}

			if (( count = count_spool(ch,snote_list)) > 0 )
			{
				ch->printlnf( "`sYou have `x%d`s new `xsnote`s%s waiting to be read (system notes).`x",
					count, count > 1 ? "s" : "");
			}
        }
    }
}
/**************************************************************************/
char *get_notetype(int index)
{
	switch(index)
	{
	default:
		return "UNKNOWN NOTE TYPE - UPDATE get_notetype()";
	case NOTE_NOTE:
		return "note";
	case NOTE_IDEA:
		return "idea";
	case NOTE_PENALTY:
		return "penatly";
	case NOTE_NEWS:
		return "news";
	case NOTE_CHANGES:
		return "change";
	case NOTE_ANOTE:
		return "anote";
	case NOTE_INOTE:
		return "inote";
	case NOTE_MISC:
		return "misc";
	case NOTE_SNOTE:
		return "snote";
	case NOTE_PKNOTE:
		return "pknote";
	}
}
/**************************************************************************/
void note_type_to_char(char_data *ch)
{
	if (ch->pnote)
	{
		ch->printlnf( "`Y You are working on a %s.", get_notetype(ch->pnote->type));
	}
}
/**************************************************************************/
void display_note_to_char( NOTE_DATA *pnote, int vnum, char_data *ch)
{
    char buf[MSL];
    char displaybuf[MSL*5];
	char realbuf[MSL];

	if ((   str_cmp(pnote->real_sender, "autonote") 
		&& str_cmp(pnote->real_sender, pnote->sender) 
		&& IS_IMMORTAL(ch))
		||
		(!str_cmp(pnote->sender, "A noble") 
			&& IS_NOBLE(ch) && IS_IMMORTAL(ch))
		)
	{
		sprintf(realbuf, "`BReal sender:`x %s\r\n", pnote->real_sender);
	}
	else
	{
		realbuf[0]='\0';
	}
	
	sprintf(displaybuf,
		"======================================="
		"======================================\n");
	sprintf( buf, 
				"`x[%3d] %s`x: %s`x`1"
				"%s%s`x`1"
				"To: %s`x`1",
						vnum-1,
						pnote->sender,
						pnote->subject,
						realbuf,
						pnote->date,
						pnote->to_list);
	strcat(displaybuf,buf);

	if(!IS_NULLSTR(pnote->cc_list)){
		sprintf( buf, 
				"Cc: %s`x`1", pnote->cc_list);
		strcat(displaybuf,buf);
	}
	
	strcat(displaybuf,"---------------------------------------"
		"--------------------------------------\n");

	if(!pnote->text){
		strcat(displaybuf,"`RERROR: for some reason there is no text in this note body!!!`x\n");
	}else if(str_len(pnote->text)< ((MSL*5) - str_len(displaybuf) - 100))
	// ensure their is room in the buffer		
	{
		strcat(displaybuf, pnote->text);
	}else{
		strcat(displaybuf,"NOTE TO LARGE TO DISPLAY!!!\n");
	}
	strcat(displaybuf,
		"`x=======================================" 
		"======================================\n");
	ch->sendpage(displaybuf);
}

/**************************************************************************/
// check if they can post note, if they can mark their last post time
// returns true if they can... system doesn't apply to those 
// level NOTE_POST_RESTRICTIONS_BELOW_LEVEL and over
bool can_post_note( char_data *ch )
{
	int i;
	int count;

	ch=TRUE_CH(ch);
	if(IS_NPC(ch)){
		return true;
	}
	
	if(IS_TRUSTED(ch, NOTE_POST_RESTRICTIONS_BELOW_LEVEL)){
		return true;
	}

	// count how many times they have posted in the last 60 seconds.
	for(i=0, count=0; i<MAX_NOTE_POST_TIME_INDEX; i++){
		if(ch->pcdata->note_post_time[i]>current_time-60){
			count++;
		}
	}
	if(count>=2){
		ch->wraplnf("`RNOTE NOT POSTED!!! `S- `WAs a player below level %d, you can only post/forward up to two notes in a "
			"60 second period to prevent abuse of the note system.", 
			NOTE_POST_RESTRICTIONS_BELOW_LEVEL);
		return false;
	}

	// count how many times they have posted in the last 10 minutes seconds.
	for(i=0, count=0; i<MAX_NOTE_POST_TIME_INDEX; i++){
		if(ch->pcdata->note_post_time[i]>current_time- (60 * 10)){
			count++;
		}
	}
	if(count>=5){
		ch->wraplnf("`RNOTE NOT POSTED!!! `S- `WAs a player below level %d, you can only post/forward up to five notes in a "
			"10 minute period to prevent abuse of the note system.", 
			NOTE_POST_RESTRICTIONS_BELOW_LEVEL);
		return false;
	}

	// find the oldest time slot and replace it
	time_t oldest_time=current_time;
	for(i=0, count=0; i<MAX_NOTE_POST_TIME_INDEX; i++){
		if(ch->pcdata->note_post_time[i]<oldest_time){
			count=i;
			oldest_time=ch->pcdata->note_post_time[i];
		}
	}
	ch->pcdata->note_post_time[count]=current_time;
	return true;
}
/**************************************************************************/
char *note_return_header()
{
	return "`x=======Date====To=============`WFrom`x=========Subject======\r\n";
}
/**************************************************************************/
char *note_format_noteentry_for_char(int vnum, NOTE_DATA *pnote, char_data *ch)
{
	char dateword[26];
    // get just month and day from the whole date string 
    char *datewordptr = pnote->date;
    datewordptr+=4; // skip day 
    strcpy(dateword,datewordptr);
    dateword[7]='\0';

	static char buf[MSL];
	bool alread_read_note=hide_note(ch,pnote);
	const char *mxp_vnum;
	char *spool_name=note_table[pnote->type].name;
	
	if(alread_read_note){
		mxp_vnum=mxp_create_send(ch,
			FORMATF("%s read %d|%s marklastread %d\" hint=\"read|mark as last read", 
				spool_name, vnum, spool_name, vnum)
			,FORMATF("%3d", vnum)); // text see on screen underlined			
	}else{
		mxp_vnum=mxp_create_send(ch,
			FORMATF("%s read %d|%s peek %d\" hint=\"read|peek without marking as read", 
				spool_name, vnum, spool_name, vnum)
			,FORMATF("%3d", vnum)); // text see on screen underlined
	}

	char noteto[MSL];
	strcpy(noteto, pnote->to_list);
	if(!IS_NULLSTR(pnote->cc_list)){
		strcat(noteto, " `Scc:`x ");
		strcat(noteto, pnote->cc_list);
	}

	sprintf( buf, "`x[%s%s] %6s %s`W %s`x %s`x\r\n",
		mxp_vnum,
        alread_read_note? " " : "N", 
		dateword,
		str_width(noteto, 17, false),
		str_width(pnote->sender, 12, false),
		pnote->subject );

	return buf;

}
/**************************************************************************/
void parse_note( char_data *ch, char *argument, int type )
{
    BUFFER *output, *buffer;
    char buf[MSL];
	char realbuf[MSL];
    char arg[MIL];
    NOTE_DATA *pnote, *prevnote;
    NOTE_DATA **list;
    char *list_name;
    
    int vnum, linenum, head;
    int anum;

    if ( IS_NPC(ch) )
        return;

    switch(type)
    {
        default:
            return;
        case NOTE_NOTE:
            list = &note_list;
            list_name = "notes";
            break;
        case NOTE_IDEA:
            list = &idea_list;
            list_name = "ideas";
            break;
        case NOTE_PENALTY:
            list = &penalty_list;
            list_name = "penalties";
            break;
        case NOTE_NEWS:
            list = &news_list;
            list_name = "news";
            break;
        case NOTE_CHANGES:
            list = &changes_list;
            list_name = "changes";
            break;
        case NOTE_ANOTE:
            list = &anote_list;
            list_name = "anotes";
            break;
        case NOTE_INOTE:
            list = &inote_list;
            list_name = "inotes";
            break;
        case NOTE_MISC:
            list = &misc_list;
            list_name = "misc notes";
            break;
        case NOTE_SNOTE:
            list = &snote_list;
            list_name = "snotes";
            break;
        case NOTE_PKNOTE:
            list = &pknote_list;
            list_name = "pknotes";
            break;
    }

    argument = one_argument( argument, arg );

    /*   smash_tilde( argument );
     *  - removed - added hide_tilde & show_tilde
     *    it swaps '~' with ascii 173 '­'
     */

	if ( arg[0] == '\0' || !str_prefix( arg, "read" ) )
    {
        bool fAll;

        if ( !str_cmp( argument, "all" ) )
        {
            fAll = true;
            anum = 0;
        }

        else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
        // read next unread note 
        {
            vnum = 0;
			prevnote=NULL;
            for ( pnote = *list; pnote != NULL; pnote = pnote->next)
            {
                if (!new_hide_note(ch,pnote, true))
                {
					display_note_to_char( pnote, vnum+1, ch);
					update_read(ch,pnote);
                    return;
                }
				else
				{
					if (is_note_to(ch,pnote))
					{
						vnum++;
					}
				}
				prevnote=pnote;
            }

			update_read(ch,prevnote);
            if (vnum>0)
			{
				sprintf(buf,"You have no unread %s.\r\n"
					"(last read/written was %d)\r\n", list_name,vnum-1);
				do_unread(ch,"");
			}
			else
			{
                sprintf(buf,"There are no %s.\r\n" , list_name);
			}
            ch->print(buf);
            ch->println("Reminder of a few note related commands:");
            ch->println("  unread, note list, note tail.");
            return;
        }

        else if ( is_number( argument ) )
        {
            fAll = false;
            anum = atoi( argument );
        }
        else
        {
            ch->println( "Read which number?" );
            return;
        }

        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
			if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
            {
				display_note_to_char( pnote, vnum, ch);
                update_read(ch,pnote);
                return;
            }
		}
        ch->printlnf("There aren't that many %s.",list_name);
        return;
    }

/////////////////////////////////////////////////
	// note peek
    if ( !str_prefix( arg, "peek" ) )
    {
        bool fAll;

        if ( !str_cmp( argument, "all" ) )
        {
            fAll = true;
            anum = 0;
        }

        else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
        // peek next unread note
        {
            vnum = 0;
            for ( pnote = *list; pnote != NULL; pnote = pnote->next)
            {
                if (!hide_note(ch,pnote))
                {
					display_note_to_char( pnote, vnum, ch);
	                return;
                }else{ 
					if (is_note_to(ch,pnote)){
						vnum++;
					}
				}
            }
            if (vnum>0){
				ch->printlnf("You have no unread %s."
				"(last read/written was %d)\r\n", list_name,vnum-1);
            }else{
				ch->printlnf("There are no %s.", list_name);
			}
			
            ch->println("Reminder of a few note related commands:");
            ch->println("  unread, note list, note tail.");
            return;
        }

        else if ( is_number( argument ) )
        {
            fAll = false;
            anum = atoi( argument );
        }
        else
        {
            ch->println("Peek at which number?");
            return;
        }

        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
            {
				display_note_to_char( pnote, vnum, ch);
                return;
            }
        }

        ch->printlnf("There aren't that many %s.",list_name);
        return;
    }

	/////////////////////////////////////////////
	// note forward
    if ( !str_prefix( arg, "forward" ) )
    {
		char argnum[MIL], newsub[MSL], newtext[MSL];

		smash_tilde( argument );

		if ( HAS_CONFIG( ch, CONFIG_NOTE_ONLY_TO_IMM ))
		{
			ch->println("You cannot forward notes while note restricted.");
			return;
		}

		argument = one_argument( argument, argnum );

        if (IS_NULLSTR(argument))
		{
			ch->println("Syntax: note forward <note number> list of receipients");
			return;			
		}

        if ( !is_number( argnum ) )
        {
            ch->println("Forward which note number?");
			return;
        }
		
		anum = atoi( argnum );
        vnum = 0;

        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && ( vnum++ == anum) )
            {
				sprintf(newsub, "`GFWD:`x %s", pnote->subject);

				sprintf(newtext, "`G%s `Gwrote to `G%s `Gon %s\r\n", pnote->sender, pnote->to_list, pnote->date);
				if(!IS_NULLSTR(pnote->cc_list)){					
					strcat(newtext, FORMATF("`GCC=%s\r\n", pnote->cc_list));
				}				
				strcat(newtext, FORMATF("-+== Forwarded %s follows ==+-`x\r\n", list_name));
				strcat(newtext, pnote->text);

				// trim end to prevent growing length indefinately
				newsub[MSL-20]='\0';
				newtext[MSL-20]='\0';

				// forwarded note 
				autonote(type, ch->name, newsub, argument, newtext, false);
				ch->printlnf("Forwarded number %d, re: %s", anum, pnote->subject);
                return;
            }
        }

        ch->printlnf("There aren't that many %s.",list_name);

        return;
    }

//////////////////////////////////////////////
    if ( !str_prefix( arg, "copyto" ))
	{

		char argnum[MIL], arg2[MIL], newsub[MSL], newtext[MSL];
		int  newtype;

		smash_tilde( argument );

		argument = one_argument( argument, argnum );
		argument = one_argument( argument, arg2 );
		
		if ( !IS_IMMORTAL( ch ))
		{
			ch->println("You can't do that.");
			return;
		}

		if ( IS_NULLSTR( argnum ) || IS_NULLSTR( arg2 ))
		{
			ch->println("Syntax:    copyto <note number> <new note board>"); 
			return;
		}

        if ( !is_number( argnum ))
		{
            ch->println("Copy which note number?");
			return;
        }

		if (( newtype = note_lookup( arg2 )) == -1 )
		{
			ch->println("No such message board.");
			return;
		}

		if ( !IS_IMMORTAL( ch ))
		{
			if ( newtype != NOTE_NOTE || newtype != NOTE_IDEA ) {
				ch->println("You may only exchange messages to the `=Cnote`x and `=Cidea`x boards.");
				return;
			}
		}
		
		anum = atoi( argnum );
        vnum = 0;

        for ( pnote = *list; pnote != NULL; pnote = pnote->next ) {

            if ( is_note_to( ch, pnote ) && ( vnum++ == anum )) {

				sprintf(newtext, "`G%s `Goriginally wrote this in the `Y%s `Gboard.`x\r\n",
					pnote->sender, note_table[pnote->type].name );
				sprintf( newsub, "`GCOPIED:`x %s", pnote->subject );
				strcat( newtext, pnote->text );

				// trim end to prevent growing length indefinately
				newsub[MSL-20]='\0';
				newtext[MSL-20]='\0';

		// now we copy it using the autonote system :)

				autonote( newtype, ch->name, newsub, pnote->to_list, newtext, false);
				ch->printlnf("Copied number %d, Subj: %s To: %s",
					anum, pnote->subject, note_table[newtype].name );
                return;
            }
        }

        ch->printlnf("There aren't that many %s.",list_name);
        return;
    }


//////////////////////////////////////////////
    if ( !str_prefix( arg, "head" ) )
    {
        output = new_buf();
        vnum = 0;
        linenum =0;
        head =0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum<150)
            {
                if (ch->lines && (linenum%ch->lines==0)) // print header once every page 
                {
                    add_buf( output, note_return_header());
                    linenum++;
                    head++;
                    if (head%2==0) linenum++;
                }

                add_buf( output, note_format_noteentry_for_char(vnum, pnote, ch));
                vnum++;
                linenum++;
            }
        }
		if (vnum==150)
		{
            add_buf( output, "`xYou have more than 150 notes,\r\n"
				"use `=Cnote list`x or `=Cnote tail`x to see the rest.\r\n");
		}
        ch->sendpage(buf_string(output));
        free_buf(output);
        return;
    }

/************************************************************************/
// search all notes and list all with matching text
    if (!str_prefix( arg, "search" ) || !str_prefix( arg, "wordsearch" ) )
    {
		int count=0;
       // int ncount = count_notes(ch, *list);
        output = new_buf();
        vnum = 0;
        linenum =0;
        head =0;

		bool fWord;
		
		if(!str_prefix( arg, "wordsearch" )){
			fWord=true;
		}else{
			fWord=false;
		}
		
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) )
            {

                // search for note
				if (linenum<150
					&& (
							(!fWord &&
								(
								!str_infix( argument,pnote->to_list)
								|| !str_infix( argument,pnote->cc_list)
								|| !str_infix( argument,pnote->sender)
								|| (IS_IMMORTAL(ch) && !str_infix( argument,pnote->real_sender))
								|| !str_infix( argument,pnote->subject)
								|| !str_infix( argument,pnote->date)
								|| !str_infix( argument,pnote->text)
								)
							)
						||
							(fWord&&
								(
								is_exact_name( argument,pnote->to_list)
								|| is_exact_name( argument,pnote->cc_list)
								|| is_exact_name( argument,pnote->sender)
								|| (IS_IMMORTAL(ch) && is_exact_name( argument,pnote->real_sender))
								|| is_exact_name( argument,pnote->subject)
								|| is_exact_name( argument,pnote->date)
								|| is_exact_name( argument,pnote->text)
								)
							)
						)
					)
                { 

                    if (ch->lines && (linenum%ch->lines==0)) /* print header once every page */
                    {
                        sprintf( buf, "`x=======Date====To=============`WFrom`x=========Subject======\r\n");
                        add_buf( output, buf );
                        linenum++;
                        head++;
                        if (head%2==0) linenum++;
                    }
    
                    add_buf( output, note_format_noteentry_for_char(vnum, pnote, ch));
                    linenum++;
					count++;
                }
                vnum++;
            }
        }

		if (linenum>=150)
		{
            add_buf( output, "`xMore than 150 notes, match your search "
				"text... be more specfic.\r\n");
		}
		else
		{
			sprintf( buf, "Found %d %s that contain '%s'\r\n", 
				count, count==1?note_table[type].lname:note_table[type].lnames,  
				argument);
            add_buf( output, buf );
		}

        ch->sendpage(buf_string(output));
        free_buf(output);
        return;
    }
/************************************************************************/
    if ( !str_prefix( arg, "list" ) ) // list the last 150 notes or 
									  // starting from number specfied
    {
        int ncount = count_notes(ch, *list)-150;
        output = new_buf();
        vnum = 0;
        linenum =0;
        head =0;

        if ( is_number( argument ) )
        {
            ncount = atoi( argument );
        }
		ncount= UMAX(0,	ncount);
		
		if (ncount>0)
		{
			ch->titlebarf("Listing from number %d up", ncount);
		}

        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) )
            {
                if (vnum>= ncount && vnum<ncount+150)
                {
                    if (ch->lines && (linenum%ch->lines==0)) /* print header once every page */
                    {
                        sprintf( buf, "`x=======Date====To=============`WFrom`x=========Subject======\r\n");
                        add_buf( output, buf );
                        linenum++;
                        head++;
                        if (head%2==0) linenum++;
                    }
    
    
                    add_buf( output, note_format_noteentry_for_char(vnum, pnote, ch));
                    linenum++;
                }
                vnum++;
            }
        }

		if (vnum>ncount+150)
		{
			char tbuf[MSL];
            sprintf( tbuf, "`xNote list continues to note %d,\r\n"
				"use `=Cnote list <number>`x to see notes starting "
				"from <number>\r\n", count_notes(ch, *list)-1);
			add_buf( output, tbuf);
		}

        ch->sendpage(buf_string(output));
        free_buf(output);
        return;

    }



////////////////////////////////////////////////////////////
    if ( !str_prefix( arg, "tail" ) )
    {
		int numtoshow=20;
        int ncount = count_notes(ch, *list);
        output = new_buf();
        vnum = 0;
        linenum =0;
        head =0;

        // allow for "note tail %number_to_tail"
		if (!IS_NULLSTR(argument) && is_number( argument ) )
        {
            numtoshow = atoi( argument );
			if (numtoshow<10 || numtoshow>150)
			{
				ch->println("The number to tail must be between 10 and 150.");
				return;
			}
        }

        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) )
            {
                if (vnum>= ncount-numtoshow)
                {
                    if (ch->lines && (linenum%ch->lines==0)) /* print header once every page */
                    {
                        sprintf( buf, "`x=======Date====To=============`WFrom`x=========Subject======\r\n");
                        add_buf( output, buf );
                        linenum++;
                        head++;
                        if (head%2==0) linenum++;
                    }
    
                    add_buf( output, note_format_noteentry_for_char(vnum, pnote, ch));
                    linenum++;
                }
                vnum++;
            }
        }
        ch->sendpage(buf_string(output));
        free_buf(output);
        return;
    }

/////////////////////////////////////////////////////////////////////
	if (IS_TRUSTED(ch,MAX_LEVEL))
	{
		if ( !str_prefix( arg, "tailall" ))
		{
			int numtoshow=20;
			int ncount = count_notes(NULL, *list);
			output = new_buf();
			vnum = 0;
			linenum =0;
			head =0;

			// allow for "note tail %number_to_tail"
			if (!IS_NULLSTR(argument) && is_number( argument ) )
			{
				numtoshow = atoi( argument );
				if (numtoshow<10 || numtoshow>200)
				{
					ch->println("The number to tail must be between 10 and 200.");
					return;
				}
			}

			for ( pnote = *list; pnote != NULL; pnote = pnote->next )
			{
			  //  if ( is_note_to( ch, pnote ) )
				{
					if (vnum>= ncount-numtoshow)
					{
						if (ch->lines && (linenum%ch->lines==0)) /* print header once every page */
						{
							sprintf( buf, "`x=======Date====To=============`WFrom`x=========Subject======\r\n");
							add_buf( output, buf );
							linenum++;
							head++;
							if (head%2==0) linenum++;
						}
    
	                    add_buf( output, note_format_noteentry_for_char(vnum, pnote, ch));
						linenum++;
					}
					vnum++;
				}
			}
			ch->sendpage(buf_string(output));
			free_buf(output);
			return;
		}

		if ( !str_prefix( arg, "readall" ) )
		{
			bool fAll;

			if ( !str_cmp( argument, "all" ) )
			{
				fAll = true;
				anum = 0;
			}
			else if ( is_number( argument ) )
			{
				fAll = false;
				anum = atoi( argument );
			}
			else
			{
				ch->println("Read which number?");
				return;
			}

			vnum = 0;
			for ( pnote = *list; pnote != NULL; pnote = pnote->next )
			{
				 if ( ( vnum++ == anum || fAll ) )
				{
					display_note_to_char( pnote, vnum-1, ch);
					update_read(ch,pnote);
					return;
				}
			}

			ch->printlnf("There aren't that many %s.",list_name);
			return;
		}
	}

////////////////////////////////////////////////////////////////
    if ( !str_prefix( arg, "remove" ) )
    {
        if ( !is_number( argument ) )
        {
            ch->println("Note remove which number?");
            return;
        }

		anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                ch->printlnf("Ok, removed note titled '%s' written by '%s'.", 
					pnote->subject, pnote->sender);
                note_remove( ch, pnote, false ); // 'note remove' used
                return;
            }
        }

        ch->printlnf("There aren't that many %s.",list_name);
        return;
    }

////////////////////////////////////////////////////////////////
    if ( !str_prefix( arg, "delete" ) && IS_IMMORTAL(ch) )
	{
		if ( !is_number( argument ))
		{
			ch->println( "Note delete which number?" );
			return;
		}

		anum = atoi( argument );
		vnum = 0;
		
		for ( pnote = *list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && vnum++ == anum )
			{
				// create a copy to admin before deleting it if nonadmin deleting it
				if(!IS_ADMIN(ch)){
					char newsub[MSL], newtext[MSL];
					sprintf(newsub, "`GDEL BY %s:`x %s", TRUE_CH(ch)->name, pnote->subject);

					sprintf(newtext, "`G%s `Gwrote to `G%s `Gon %s\r\n", pnote->sender, pnote->to_list, pnote->date);
					if(!IS_NULLSTR(pnote->cc_list)){						
						strcat(newtext, FORMATF(newtext, "`GCC=%s\r\n", pnote->cc_list));
					}
					strcat(newtext, FORMATF("-+== Deleted %s follows ==+-`x\r\n", list_name));
					strcat(newtext, pnote->text);

					// deleted note
					autonote(type, "admin", newsub, "admin", newtext, false);
				}
				// delete the note
				ch->printlnf( "Ok, deleted note titled '%s' written by '%s'.",
					pnote->subject, pnote->sender);
				note_remove( ch, pnote,true ); // 'note delete' used
				return;
			}
		}

		ch->printlnf( "There aren't that many %s.", list_name );
		return;
	}

////////////////////////////////////////////////////////////////
	if (!str_prefix(arg,"marklastread"))
	{
		bool fAll;
		
		if ( is_number( argument ))
		{
			fAll = false;
			anum = atoi( argument );
		}
		else
		{
			ch->println( "marklastread which number?" );
			return;
		}
		
		vnum = 0;

		for ( pnote = *list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
			{
				if ((   str_cmp(pnote->real_sender, "autonote") 
					&& str_cmp(pnote->real_sender, pnote->sender) 
					&& IS_IMMORTAL(ch))
					|| (!str_cmp(pnote->sender, "A noble")
					&& IS_NOBLE(ch)))
				{
					sprintf(realbuf, "`BReal sender:`x %s\r\n", pnote->real_sender);
				}
				else
				{
					realbuf[0]='\0';
				}
				ch->printlnf( "`RMARKLASTREAD ON NOTE:`x\r\n[%3d] %s`x: "
					"%s`x\r\n%s%s`x\r\nTo: %s`x",
					vnum - 1,
					pnote->sender,
					pnote->subject,
					realbuf,
					pnote->date,
					pnote->to_list );

				mark_lastread(ch,pnote);
				return;
			}
		}
		
		ch->printlnf( "There aren't that many %s.", list_name );
		return;
	}

////////////////////////////////////////////////////////////////
	if (!str_prefix(arg,"catchup"))
	{
		switch(type)
		{
		case NOTE_NOTE:
			ch->pcdata->last_note = current_time;
			ch->println( "All notes have been marked as read." );
			break;
		case NOTE_IDEA:
			ch->pcdata->last_idea = current_time;
			ch->println( "All ideas have been marked as read." );
			break;
		case NOTE_PENALTY:
			ch->pcdata->last_penalty = current_time;
			ch->println( "All penalties have been marked as read." );
			break;
		case NOTE_NEWS:
			ch->pcdata->last_news = current_time;
			ch->println( "All the news has been marked as read." );
			break;
		case NOTE_CHANGES:
			ch->pcdata->last_changes = current_time;
			ch->println( "All changes have been marked as read." );
			break;
		case NOTE_ANOTE:
			ch->pcdata->last_anote = current_time;
			ch->println( "All anotes have been marked as read." );
			break;
		case NOTE_INOTE:
			ch->pcdata->last_inote = current_time;
			ch->println( "All inotes have been marked as read." );
			break;
		case NOTE_MISC:
			ch->pcdata->last_misc = current_time;
			ch->println( "All misc notes have been marked as read." );
			break;
		case NOTE_SNOTE:
			ch->pcdata->last_snote = current_time;
			ch->println( "All snotes have been marked as read." );
			break;
		case NOTE_PKNOTE:
			ch->pcdata->last_pknote = current_time;
			ch->println( "All pknotes have been marked as read." );
			break;
		}
		return;
	}

////////////////////////////////////////////////////////////////
// below this point only certain people can edit notes
    if  ((   type == NOTE_NEWS && !IS_TRUSTED(ch,ANGEL))
		|| ( type == NOTE_CHANGES && !IS_TRUSTED(ch,COUNCIL)))
	{
		ch->printlnf( "You aren't high enough level to write %s.", list_name );
		return;
	}

	// snotes
	if (  type == NOTE_SNOTE && !IS_ADMIN(ch) )
	{
		ch->println( "Snotes are system notes - only written by the system/game itself." );
		return;
	}


////////////////////////////////////////////////////////////////
	if ( !str_cmp( arg, "fromnoble" ) && IS_NOBLE(ch) && (list!=&misc_list))
	{
		if(!ch->pnote)
		{
			ch->println( "You must start a note before marking it fromnoble." );
			return;
		}

		if (ch->pnote->type != type)
		{
			ch->println( "You might want to use the note type you started with." );
			note_type_to_char(ch);
			return;
		}

        ch->pnote->sender = str_dup("A noble");
        ch->wraplnf( "Ok, your current %s will appear as though it was from 'A noble'.",
			note_table[ch->pnote->type].name);
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_cmp( arg, "from" ) && IS_IMMORTAL(ch) )
	{
		if(ch->pnote==NULL)
		{
			ch->println( "You must start a note before setting the 'from' address." );
			return;
		}

		if (ch->pnote->type != type)
		{
			ch->println( "You might want to use the note type you started with." );
			note_type_to_char(ch);
			return;
		}
		ch->pnote->sender = str_dup(argument);
		ch->wraplnf( "Ok, your current %s will appear as though it was from '%s'.",
			note_table[ch->pnote->type].name, argument);
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_cmp( arg, "+" ))
	{
		char tstr_buf[5000];
		buffer = new_buf();
		note_attach( ch,type );
		if (ch->pnote->type != type)
		{
			ch->wraplnf( "You are working on a %s (not a %s).",
				note_table[ch->pnote->type].lname, note_table[type].lname);
			ch->printlnf( "You can clear partially written %s by typing `=C%s clear`x.", 
				note_table[ch->pnote->type].lname, note_table[ch->pnote->type].lname);
			return;
		}
		
		if (str_len(ch->pnote->text)+str_len(argument) >= 4096)
		{
			ch->printlnf( "%s too long.",
				note_table[ch->pnote->type].name);
			return;
		}

		// convert the use of the old new line code {} to `1
		if(!HAS_CONFIG2(ch, CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING)){
			if( strstr(argument, "{}") ){
				char *tstr=str_dup(argument);
				tstr=string_replace_all(tstr, "{}", "`1");
				strcpy(tstr_buf, tstr);
				free_string(tstr);
				argument=tstr_buf;
				ch->wrapln("Note: Your use of the old style code for new line in a note {} "
					"has been automatically converted to the new style ``1.  If you want to "
					"disable this automatic conversion use the "
					"unlisted 'detect_oldstyle_note_writing' command.");
			}
		}


		add_buf(buffer,ch->pnote->text);
		add_buf(buffer,argument);
		add_buf(buffer,"\r\n");
		free_string( ch->pnote->text );
		ch->pnote->text = str_dup( buf_string(buffer) );
		free_buf(buffer);
		ch->print( "Ok." );
		if (IS_SET(ch->act,PLR_AUTOREFORMAT))
			ch->print( " (Autoreformat is On, use `=C``1`x for a newline)" );
		else
			ch->print( " (Autoreformat Off)" );
		ch->println( "" );
		return;
	}

////////////////////////////////////////////////////////////////
	if (!str_cmp(arg,"-"))
	{
		int len;
		bool found = false;
		note_attach(ch,type);
		if (ch->pnote->type != type)
		{
			ch->wraplnf( "You are working on a %s (not a %s).",
				note_table[ch->pnote->type].name, note_table[type].name);
			return;
		}
		if (ch->pnote->text == NULL || ch->pnote->text[0] == '\0')
		{
			ch->println( "No lines left to remove." );
			return;
		}
		strcpy(buf,ch->pnote->text);
		for (len = str_len(buf); len > 0; len--)
		{
			if (buf[len] == '\r')
			{
				if (!found)  /* back it up */
				{
					if (len > 0)
						len--;
					found = true;
				}
				else /* found the second one */
				{
					buf[len + 1] = '\0';
					free_string(ch->pnote->text);
					ch->pnote->text = str_dup(buf);
					return;
				}
			}
		}
		buf[0] = '\0';
		free_string(ch->pnote->text);
		ch->pnote->text = str_dup(buf);
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "edit" ))
	{
		if ( ch->pnote == NULL )
		{
			ch->println( "You have no note in progress." );
			return;
		}

		if (ch->pnote->type != type)
		{
			ch->println( "You aren't working on that kind of note." );
			note_type_to_char(ch);
			return;
		}
		ch->println( "Editing the text on your note:" );
		string_append(ch, &ch->pnote->text);
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "subject" ))
	{
		note_attach( ch,type );
		if (ch->pnote->type != type)
		{
			ch->println( "You already have a different note in progress." );
			note_type_to_char(ch);
			ch->printlnf( "You can clear partially written %s by typing `=C%s clear`x.", 
				note_table[ch->pnote->type].lname, note_table[ch->pnote->type].lname);
			return;
		}
		if (str_len(argument)>125)
		{
			ch->println( "`RDO NOT PUT THE MESSAGE BODY OF YOUR NOTE ON THE SUBJECT LINE!!! READ THE HELP FILES ON THE NOTES!!!`x" );
		}
		if (str_len(argument)>500)
		{
			ch->println( "`RYOU CAN NOT HAVE A SUBJECT LINE LONGER THAN 500 CHARACTERS!!!`x" );
			return;
		}
		free_string( ch->pnote->subject );
		ch->pnote->subject = str_dup( argument );
		ch->println( "Ok." );
		
		if (IS_NOBLE(ch))
		{
			ch->println( "Hint: use 'note fromnoble' to write an anoymous noble related note." );
		}
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "to" ))
	{
		note_attach( ch,type );
		if (ch->pnote->type != type)
		{
			ch->println( "You already have a different note in progress." );
			note_type_to_char(ch);
			ch->printlnf( "You can clear partially written %s by typing `=C%s clear`x.", 
				note_table[ch->pnote->type].lname, note_table[ch->pnote->type].lname);
			return;
		}

		if(IS_IMMORTAL(ch) 
			&& ch->pnote->type==NOTE_NOTE 
			&& !str_cmp("imm", ltrim_string(rtrim_string(argument)) )  ){
            ch->wrapln( "`xImmortals writing notes to the imms only should send them as inotes.`1"
				"use '`=Cinote to imm`x', '`=Cinote + ...`x' instead." );
			return;
		}

		if (HAS_CONFIG(ch, CONFIG_NOTE_ONLY_TO_IMM)){
			int council;
			council = flag_lookup(argument,council_flags);
			if ( council == NO_FLAG)
			{
				ch->println( "`RNote to field set to go to only the immortals.`x" );
				ch->println( "note: You can address notes specifically to a\r\n"
					"council name or headcouncil name (e.g. headlaw)`x" );
				replace_string(ch->pnote->to_list, "imm");
			}else{
				replace_string(ch->pnote->to_list, argument);
			}
		}else{
			if(!str_infix("@", argument)){
				ch->println("The @ symbol isn't valid in the to address of a note!");
				return;
			}
			replace_string(ch->pnote->to_list, argument );
		}		

		if (str_len(ch->pnote->to_list)>50)
		{
			ch->println( "`RDO NOT PUT THE MESSAGE BODY OF YOUR NOTE ON THE TO LINE!!! READ THE HELP FILES ON THE NOTES!!!`x" );
		}
		ch->println( "Ok.  (The ``note to' system has been extended, type ``help notes' for details)" );
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "cc" ))
	{
		note_attach( ch,type );
		if (ch->pnote->type != type)
		{
			ch->println( "You already have a different note in progress." );
			note_type_to_char(ch);
			ch->printlnf( "You can clear partially written %s by typing `=C%s clear`x.", 
				note_table[ch->pnote->type].lname, note_table[ch->pnote->type].lname);
			return;
		}

		if (HAS_CONFIG(ch, CONFIG_NOTE_ONLY_TO_IMM)){
			int council;
			council = flag_lookup(argument,council_flags);
			if ( council == NO_FLAG)
			{
				ch->println( "`RNote to field set to go to only the immortals.`x" );
				ch->println( "note: You can address notes specifically to a\r\n"
					"council name or headcouncil name (e.g. headlaw)`x" );
				replace_string(ch->pnote->cc_list, "imm");
			}else{
				replace_string(ch->pnote->cc_list, argument);
			}
		}else{
			if(!str_infix("@", argument)){
				ch->println("The @ symbol isn't valid in the to address of a note!");
				return;
			}
			replace_string(ch->pnote->cc_list, argument );
		}		

		if (str_len(ch->pnote->cc_list)>50)
		{
			ch->println( "`RDO NOT PUT THE MESSAGE BODY OF YOUR NOTE ON THE CC LINE!!! READ THE HELP FILES ON THE NOTES!!!`x" );
		}
		ch->printlnf( "Ok, note is cc'd to '%s'", ch->pnote->cc_list);
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "clear" ))
	{
		if ( ch->pnote != NULL )
		{
			free_note(ch->pnote);
			ch->pnote = NULL;
		}
		
		ch->println( "Cleared." );
		return;
	}

////////////////////////////////////////////////////////////////
	if( str_len(arg)>4
		&&  !str_prefix( arg, "showraw" ))
	{
		if(!ch->pnote){
			ch->println("You have no note in progress.");
			return;
		}

		if(ch->pnote->type != type){
			ch->println("You aren't working on that kind of note.");
			note_type_to_char(ch);
			return;
		}

		ch->println("=== NOTE SHOWRAW, read `=Chelp note-showraw`x for other options ===");
		ch->printlnf("`x%s`x: %s`x`1`xTo: %s`x`1Cc: %s`x",
			ch->pnote->sender,
			ch->pnote->subject,
			ch->pnote->to_list,
			ch->pnote->cc_list);
		ch->printbw(ch->pnote->text);
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "show" ))
	{
		if ( ch->pnote == NULL )
		{
			ch->println( "You have no note in progress." );
			return;
		}
		if (ch->pnote->type != type)
		{
			ch->println( "You aren't working on that kind of note." );
			note_type_to_char(ch);
			return;
		}

		ch->println( "=== NOTE SHOW, read `=Chelp `=_note-show`x for other options ===" );
		ch->printlnf( "`x%s`x: %s`x`1`xTo: %s`x`1`xCc: %s`x",
			ch->pnote->sender,
			ch->pnote->subject,
			ch->pnote->to_list,
			ch->pnote->cc_list );

		if (IS_SET(ch->act,PLR_AUTOREFORMAT))
		{
			char *tempdup= note_format_string(str_dup(ch->pnote->text));
			ch->sendpage(tempdup);
			free_string(tempdup);
		}
		else
		{
			ch->sendpage(ch->pnote->text);
		}

		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "reformat" ))
	{
		if ( ch->pnote == NULL )
		{
			ch->println( "You have no note in progress." );
			return;
		}

		if (ch->pnote->type != type)
		{
			ch->println( "You aren't working on that kind of note." );
			note_type_to_char(ch);
			return;
		}

		ch->pnote->text = note_format_string (ch->pnote->text);
		ch->println( "Ok." );
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "reedit" ) && IS_TRUSTED(ch,CREATOR))
	{
		if ( !is_number( argument ))
		{
			ch->println( "Note reedit which number?" );
			return;
		}

		anum = atoi( argument );
		vnum = 0;

		for ( pnote = *list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && vnum++ == anum )
			{
				ch->println( "When you finish editing the note, type note resave to save it to disk." );
				string_append(ch, &pnote->text);
				return;
			}
		}

		ch->printlnf( "There aren't that many %s.", list_name );
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "resave" ) && IS_TRUSTED(ch,CREATOR))
	{
		ch->printlnf( "Resaving the %s file.", list_name);
		save_notes(type);
		return;
	}

////////////////////////////////////////////////////////////////
	if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send"))
	{
		char *strtime;
		if ( ch->pnote == NULL )
		{
			ch->println( "`YYou have no note in progress.`x" );
			return;
		}

		if (ch->pnote->type != type)
		{
			ch->println( "`YYou aren't working on that kind of note.`x" );
			note_type_to_char(ch);
			ch->printlnf( "`R%s NOT POSTED!`x",
				note_table[ch->pnote->type].name);
			return;
		}

		if (!str_cmp(ch->pnote->to_list,""))
		{
			ch->println( "`YYou need to provide a recipient (name, all, admin or immortal)." );
			ch->printlnf( "`R%s NOT POSTED!`x\r\n",
				note_table[ch->pnote->type].name );
			return;
		}

		if (!str_cmp(ch->pnote->subject,""))
		{
			ch->println( "`YYou need to provide a subject." );
			ch->printlnf( "`R%s NOT POSTED!`x",
				note_table[ch->pnote->type].name);
			return;
		}

		// check if they can post note, if they can mark their last post time
		if (!can_post_note(ch)){
			return;
		}

		// automatically reformat the note if required
		if (IS_SET(ch->act,PLR_AUTOREFORMAT))
		{
			ch->pnote->text = note_format_string (ch->pnote->text);
		}

		ch->pnote->next                 = NULL;
		strtime                         = ctime( &current_time );
		strtime[str_len(strtime)-1]      = '\0';
		ch->pnote->date                 = str_dup( strtime );
		ch->pnote->date_stamp           = current_time;
		ch->pnote->real_sender          = str_dup(ch->name);
		append_note(ch->pnote);

		if (IS_SET(ch->act,PLR_AUTOREFORMAT))
			ch->printlnf( "`G%s REFORMATTED AND POSTED.`x",
			note_table[ch->pnote->type].name);
		else
			ch->printlnf( "%s POSTED.`x",
			note_table[ch->pnote->type].name);
		ch->pnote = NULL;

		// notify imms & heros of new mail 
		do_note_notify(game_settings->minimum_note_notify_level,ABSOLUTE_MAX_LEVEL);
		if (note_notify_counter<0){
			note_notify_counter = 4;
		}
		return;
	}
	
	ch->println( "You can't do that." );
	return;
}


/* Second half of format_string was rewritten by Kalahn May 98, 
 * Now wordwraps correct with ` colorcode sequences.
 */
/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap written by Surreality.
 */
/*****************************************************************************
 Name:      note_format_string
 Purpose:	Special string formating and word-wrapping.
 Called by: 
 ****************************************************************************/
char *note_format_string_width( char *oldstring, int width, bool returns, bool trailing_newline)
{
	char xbuf[MSL*16];
	char xbuf2[MSL*16];
	char *rdesc;
	int i=0;
	bool cap=true;
	
	xbuf[0]=xbuf2[0]=0;
	
	i=0;
	
	for (rdesc = oldstring; *rdesc; rdesc++)
	{
        if (*rdesc=='\r')
		{
			if (xbuf[i-1] != ' ')
			{
				if(!(i>1 && xbuf[i-1]== '+' && xbuf[i-2]== '`')){
					xbuf[i]=' ';
					i++;
				}
			}
		}
        else if (*rdesc=='\n') ;
        /*    else iff (*rdesc==' ')
		{
		if (xbuf[i-1] != ' ')
		{
		xbuf[i]=' ';
		i++;
		}
		}
		*/
		else if (*rdesc==')')
		{
			if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
				(xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
			{
				xbuf[i-2]=*rdesc;
				xbuf[i-1]=' ';
				xbuf[i]=' ';
				i++;
			}
			else
				
			{
				xbuf[i]=*rdesc;
				i++;
			}
		}
		else
		{
			xbuf[i]=*rdesc;
			if ( cap )
			{
				cap = false;
				xbuf[i] = UPPER( xbuf[i] );
			}
			i++;
		}
	}
	xbuf[i]=0;
	strcpy(xbuf2,xbuf);
	
	rdesc=xbuf2;
	
	xbuf[0]='\0';
	
	/*
	*  Code above here removes all the line feeds and caps the correct words.
	*  below here puts in lines
	*/
		
	
	// code below here was written by Kalahn May 98
	// next wordwrap the string so no line is no greater 
	// than 'width' visible characters
	
	// rdesc is a pointer to the start of the string
	// get the formatted version of rdesc into xbuf
	{
		int i=0;
		int vischars=0; // visible characters on the current line
		int last_space=0; // where the last space was encounted
		char *point;
		bool more_string= true;
		
		point=xbuf; // target buffer
		bool inside_tag=false;
		
		while(more_string)
		{
			while (vischars<width && *(rdesc+i)!='\0')
			{
				*point= *(rdesc+i); // copy the character
				
				{
					// this section of code enables note_format_string_width()
					// to wrap strings with embeded MXP tags correctly 
					// - Kal, Jan 04
					if (*(point)==MXP_BEGIN_TAG){
						inside_tag=true;					
					}
					if (*(point)==MXP_END_TAG){
						inside_tag=false;
						vischars--;
					}
					if(inside_tag){
						// when inside a tag, just keep copying till we reach the end
						i++;
						*(++point)='\0';
						continue;
					}
				}

				vischars++;
				
				// record the last space location.
				if (*(point)==' ')
				{
					last_space=i;
				}
								
				// calculate the effects of colour codes
				if (*(point)=='`')
				{
					i++;
					point++;
					*point= *(rdesc+i); // copy the next character
					switch (*point)
					{
						default: // most codes count as none
							vischars--;
							break;
				
						case '1': // newline system reset the counters
							vischars=0;
							rdesc+=i;
							i=-1;
							point--;
							last_space=-1;
							break;

						case '+': // inline paragraph system `+
							point++;
							if(returns){
								*point++='\r';
							}
							*point= '\n';
							vischars=0;
							rdesc+=i+1;
							i=-1;
							last_space=-1;
							break;
				  
						case 'N': // N - counts as the length of the name
							vischars+=str_len(game_settings->gamename)-1;
							break;

						case '-': // creates ~ counts as one
						case '`': // creates ` counts as one
							break;
					}
					i++;
					point++;
					continue;
				}
				i++;
				*(++point)='\0';
			}

			// all tags should be terminated, otherwise someone has
			// a bug somewhere... if so close the tag anyway
			if(inside_tag){
				bugf("note_format_string_width(): mxp/html tag not closed!");
				*(point)=MXP_END_TAG;
				point++;
			}

			// end of string or adding a new line
			if (vischars<width) 
			{
				*point='\0';
				more_string=false;
			}
			else
			{
				if (last_space>-1)
				{
					last_space++;
					point-= (i-last_space);
					if(returns){
		                *point++='\r';
					}
                    *point++='\n';
					i=last_space;
				}
				else // line to long
				{
					logf("noteformatstring: line too long. '%s'", rdesc);
					*point++='-';
					if(returns){
	                    *point++='\r';
					}
                    *point++='\n';
				}
				
				// setup for next time thru loop
				vischars=0;
				rdesc+=i;
				i=0;
				last_space=-1;
			}
		}
		if(returns && trailing_newline){
	        *point++='\r';
		}
		if(trailing_newline){
			*point++='\n';
		}
		*point='\0';
	}

	free_string(oldstring);
	return(str_dup(xbuf));
}

/**************************************************************************/
char *note_format_string( char *oldstring)
{
	return note_format_string_width(oldstring, 77, true, true);
}

/**************************************************************************/
// AUTONOTE
// automated note creation
////////////////////////////////////////////////////////////////////
void autonote(int type, const char *sender, const char *subject, const char *to, const char *text, bool reformat)
{
    NOTE_DATA *pnote;
    char *strtime;
	time_t note_time;
	static time_t last_note_time;

    pnote = new_note();
    pnote->next		= NULL;

    pnote->type		= type;

	// sender
    pnote->sender	= str_dup( sender );
	pnote->real_sender	= str_dup( sender );

	note_time = current_time;
	if (last_note_time>=note_time){
		note_time= last_note_time+1;
	}
	last_note_time= note_time;

	// date
	strtime 						= ctime( &note_time  );
	strtime[str_len(strtime)-1]		= '\0';
	pnote->date 				= str_dup( strtime );


	pnote->date_stamp			= note_time;
    pnote->to_list	= str_dup( to );

	// safety length checks
	if(str_len(subject)>600){
		char trimmed_subject[600];
		strncpy(trimmed_subject, subject, 599);
		trimmed_subject[599]='\0';
	    pnote->subject	= str_dup( trimmed_subject );
	}else{
	    pnote->subject	= str_dup( subject );
	}
	if(str_len(text)>MSL*2){
		char trimmed_text[MSL*2];
		strncpy(trimmed_text, text, MSL*2-1);
		trimmed_text[MSL*2-1]='\0';
	    pnote->text		= str_dup( trimmed_text);
	}else{
	    pnote->text		= str_dup( text );
	}
    

	// automatically reformat the note text
	if (reformat)
		pnote->text = note_format_string (pnote->text);

	// quick hackish piece of code to allow autonotes to be posted at all times... 
	FILE *holder=fpReserve;
    fpReserve = fopen( NULL_FILE, "r" );
	if(fpReserve==NULL){ // problems - lack of available file descriptors
		logf("Autonote couldn't get a temporary file descriptor");
		fpReserve=holder;
		append_note(pnote);
	}else{
		append_note(pnote);
		fclose(fpReserve);
		fpReserve=holder;
	}

	// notify imms & heros of new mail 
	do_note_notify(game_settings->minimum_note_notify_level,ABSOLUTE_MAX_LEVEL); 

	if (note_notify_counter<0){
		note_notify_counter = 4;
	}

	return;
}

/**************************************************************************/
void do_showmisc(char_data *ch, char *)
{
    if (HAS_CONFIG(ch, CONFIG_SHOWMISC))
    {
		ch->println("Details of unread misc notes will no longer be displayed in `=Cunread`x.");
		REMOVE_CONFIG(ch, CONFIG_SHOWMISC);
    }
    else
    {
		ch->println("Details of unread misc notes will now be displayed in `=Cunread`x.");
		ch->println("`RWARNING: The misc note column can contain a lot of spam!");
		ch->println("Do NOT read miscs if you value your time!`x");
		SET_CONFIG(ch, CONFIG_SHOWMISC);
    }
}
/**************************************************************************/
void do_nomisc(char_data *ch, char *)
{
    if (HAS_CONFIG(ch, CONFIG_NOMISC))
    {
		ch->println("Misc note reading enabled.");
		REMOVE_CONFIG(ch, CONFIG_NOMISC);
    }
    else
    {
		ch->println("Misc note reading disabled.");
		SET_CONFIG(ch, CONFIG_NOMISC);
    }
}

/**************************************************************************/
char *colour_convert_code_format(char *text);
#define notestr_replace_colour_code(str) \
	do{   char *t; \
		if((str)!=NULL && (str)[0]!='\0'){ \
			t=colour_convert_code_format(str); \
			replace_string(str, t); \
		} \
	}while(0)
/**************************************************************************/
void colour_convert_note_spool(NOTE_DATA *list, int type)
{
	for( ;list; list=list->next){
	    notestr_replace_colour_code(list->sender);
		notestr_replace_colour_code(list->real_sender);
		notestr_replace_colour_code(list->to_list);
		notestr_replace_colour_code(list->cc_list);
		notestr_replace_colour_code(list->subject);
		notestr_replace_colour_code(list->text);
	}
	save_notes(type);
}

/**************************************************************************/
void note_convert_colour_codes()
{
	// convert the notes
    colour_convert_note_spool(note_list, NOTE_NOTE);
    colour_convert_note_spool(idea_list, NOTE_IDEA);
    colour_convert_note_spool(penalty_list, NOTE_PENALTY);
    colour_convert_note_spool(news_list, NOTE_NEWS);
    colour_convert_note_spool(changes_list,NOTE_CHANGES);
    colour_convert_note_spool(anote_list,NOTE_ANOTE);
    colour_convert_note_spool(inote_list,NOTE_INOTE);
    colour_convert_note_spool(misc_list,NOTE_MISC);
    colour_convert_note_spool(snote_list,NOTE_SNOTE);
    colour_convert_note_spool(pknote_list,NOTE_PKNOTE);
}
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/

