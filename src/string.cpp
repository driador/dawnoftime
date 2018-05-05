/**************************************************************************/
// string.cpp - string related functions
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 ***************************************************************************/
#include "include.h" // dawn standard includes
#include "interp.h" 
#include "olc.h" 
#include "nanny.h" 
#include "help.h" 

char *string_linedel( char *, int );
char *string_lineadd( char *, char *, int );
char *number_lines( char_data* ch, char *text, bool use_mxp );

/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( char_data *ch, char **pString )
{
    ch->println("-=======- Entering APPEND Mode -========-");
    ch->println("    Type .h on a new line for help");
    ch->println(" Terminate with a ~ or @ on a blank line.");
    ch->println("-=======================================-");

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }


	// Find dawnftp/mudftp connection if they have one
	bool using_dawnftp=false;
	if(TRUE_CH(ch)->pcdata && TRUE_CH(ch)->pcdata->preference_dawnftp==PREF_AUTOSENSE){
		for (connection_data *m = connection_list; m; m=m->next)  {
			if (m->connected_state == CON_FTP_COMMAND &&
				m->ftp.mode == FTP_PUSH &&
				!str_cmp(m->username, TRUE_CH(ch)->name))
			{
				using_dawnftp=true;
				break;
			}
		}
	}else if (TRUE_CH(ch)->pcdata->preference_dawnftp==PREF_ON){
		using_dawnftp=true; // perm on
	}

	if (using_dawnftp)
	{
        ch->desc->pString = pString;
        if (ftp_push(ch->desc)){ // ftp: PUSH mode 
            ch->println("Editing string via DawnFTP/mudFTP push connection.  Use ~ or @ to abort.");
			ch->desc->ftp.inuse=true;
			return;
		}
		
		{ // try PULL mode, since PUSH mode didnt' work and we have the connection forced on
            ch->printf("Sending DawnFTP/mudFTP request. If your client does not support DawnFTP/mudFTP, abort this\n"
                       "edit (type ~ or @ on a blank line), toggle dawnftp off, and try again.\n"
                       "\ntmp/%lu%c\n", *((unsigned long*) pString), 230);
			return;
        }
    }

	if (str_len(*pString)>MSL-4){
		char buf2[MSL*2];
		strncpy(buf2, *pString, MSL-4);
		buf2[MSL-4]='\0';

		ch->printf("The text you are trying to edit is longer "
			"than %d characters!\r\n", MSL-4);
		bug("string_append: Text too long! - trimmed version will be shown");
		log_string(buf2);
		ch->print(buf2);
		ch->printlnf("\r\n\r\n`RThe text you are trying to edit is longer "
			"than %d characters!`x", MSL-4);
		ch->wrapln("`YSorry, the text will have to be trimmed to a "
			"smaller size before it can be worked on.`x");
		ch->desc->pString = NULL;
	}else{
		ch->print( number_lines(ch, *pString, true));
		ch->print("`x");
		ch->desc->pString = pString;
	}
    return;
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
// search the string orig (expected to be a str_dup()'d string)
// for the text oldstr
// and replace with newstr
// the returned result, is str_dup()'d and orig is freed
char * string_replace( char * orig, char * oldstr, char * newstr )
{
    char xbuf[HSL];
    int i;

    xbuf[0] = '\0';
    strcpy( xbuf, orig );
    if ( strstr( orig, oldstr ) != NULL )
    {
        i = (int)(str_len( orig ) - str_len( strstr( orig, oldstr) ));
        xbuf[i] = '\0';
        strcat( xbuf, newstr );
        strcat( xbuf, &orig[i+str_len( oldstr)] );
        free_string( orig );
    }

    return str_dup( xbuf );
}


/**************************************************************************/
// Kal
// assumes orig is an already str_duped string
char * string_replace_all( char * orig, char * old, char * newstr )
{
	if(!strstr( orig, old ) || !str_cmp(old, newstr)){
		return orig;
	}
	
	while(strstr( orig, old )){
		orig=string_replace(orig,old,newstr);
	}
	return orig;
}
/**************************************************************************/
// getline_by_number returns the line specified by the number, 
// caller needs to free it
char *getline_by_number( char *string, int linenum);
/**************************************************************************/

/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void save_bans(void);

void string_add( char_data *ch, char *argument )
{
    char buf[HSL];

    smash_tilde( argument );

    if(ch->desc && ch->desc->ftp.inuse && str_cmp(argument, "@"))
	{
        ch->println("Type @  to manually abort FTP mode.\r\n"
			"If DawnFTP/mudFTP is not supported by your client, abort this edit and turn dawnftp off.");
        return;
	}

    if ( *argument == '.' )
    {
        char arg1 [MIL];
        char arg2 [MIL];
        char arg3 [MIL];
        char tmparg3 [MIL];
        char text [MIL];

        argument = one_argument( argument, arg1 );
		strcpy(text, argument);
        argument = first_arg( argument, arg2, false );
		strcpy( tmparg3, argument );
        argument = first_arg( argument, arg3, false );

        if ( !str_cmp( arg1, ".a" ) )
        {
			do_answer(ch, text);
            return;
        }
        if ( !str_cmp( arg1, ".o" ) )
        {
			do_ooc(ch, text);
            return;
        }
		if ( !str_cmp( arg1, ".q" ) )
        {
			do_question(ch, text);
            return;
        }
		if ( !str_cmp( arg1, ".:" ))
        {
			interpret(ch, text);
            return;
        }
/*        if ( !str_cmp( arg1, ".:" )  )
        {
			do_immtalk(ch, text);
            return;
        }
*/
        if ( !str_cmp( arg1, ".c" ) )
        {
            ch->println("String cleared.");
			free_string(*ch->desc->pString);
			*ch->desc->pString = str_dup( "" );
            return;
        }

		if ( !str_cmp( arg1, ".i" ) )
		{
			if(ch->desc && ch->desc->editor == ED_MPCODE){
				ch->println("CODE INDENTED...");
				char *indented=indent_mobprog_code(ch,*ch->desc->pString, atoi(arg2));
				free_string(*ch->desc->pString);
				*ch->desc->pString=str_dup(indented);
				string_add(ch, ".s");
			}else{
				ch->println("You have to be editing mobprog code in order to use .i to indent code.");
			}
			return;
		}

// .spell <word> - spellcheck a single word
// .spell        - spellcheck the whole string

#ifdef unix
		if ( !str_cmp( arg1, ".spell" ) )
		{
			if (!IS_NULLSTR(arg2)){
				do_ispell(ch, arg2);
			}else{
				ispell_string(ch);
			}
			return;
		}
#endif

        if ( !str_cmp( arg1, ".s" ) )
        {
			// decide if to put line numbers on the string
			if (str_infix("`1",*ch->desc->pString)){
	            ch->println("String so far:");
				ch->print( number_lines(ch, *ch->desc->pString, true));
				ch->print("`x");
			}else{
	            ch->println("String so far: (has ``1 codes in it, use .b to see line numbering)");
				ch->print( *ch->desc->pString);
			}
            return;
        }

        if ( !str_cmp( arg1, ".b" ) )
        {
			ch->println( "String so far:" );
			ch->printbw( number_lines(ch, *ch->desc->pString, false));
			ch->print( "`x" );
            return;
        }

        if ( !str_cmp( arg1, ".r" ) )
        {
            if ( arg2[0] == '\0' )
            {
                ch->println("usage:  .r \"old string\" \"new string\"");
                return;
            }

			if (!str_infix(arg2 , *ch->desc->pString)){
				// check the lengths
				if ( str_len( *ch->desc->pString) + str_len( arg3 ) - str_len( arg2 )>= ( MSL - 50 ) )
				{
					ch->println("You can't replace that much, the resulting string would be too long.");
					return;
				}
				*ch->desc->pString =
					string_replace( *ch->desc->pString, arg2, arg3 );
                sprintf( buf, "'%s' replaced with '%s'.", arg2, arg3 );
				ch->printlnbw( buf );
			}else{
                sprintf( buf, "Couldn't find '%s' in string.", arg2);
				ch->printlnbw( buf );
			}
            return;
        }

        if ( IS_IMMORTAL(ch) && !str_cmp( arg1, ".z" ) )
        {
			int count=0;

			// hide all ``1 codes from the replace of `1 codes
			while(!str_infix("``1", *ch->desc->pString)){
				*ch->desc->pString =
					string_replace( *ch->desc->pString, "``1", "!@#$%%^%$#@!");
			}		
			while(!str_infix("`1", *ch->desc->pString)){
				*ch->desc->pString =
                    string_replace( *ch->desc->pString, "`1", "`+\n");
				count++;
			}
			// return all original ``1 codes 
            while(!str_infix("!@#$%%^%$#@!", *ch->desc->pString)){
				*ch->desc->pString =
					string_replace( *ch->desc->pString, "!@#$%%^%$#@!", "``1");
			}

			if(count==0){
                sprintf( buf, "Couldn't find any `1 code in string to mass replace.");
				ch->printlnbw( buf );
			}else{
                ch->printlnf("Replaced %d ``1 symbols with ``+\\n",count);
			}
            return;
        }


        if ( !str_cmp( arg1, ".n" ) || !str_cmp( arg1, ".f" ) )
        {
            *ch->desc->pString = note_format_string( *ch->desc->pString );
            ch->println("String noteformatted.");
            return;
        }
        
        if ( !str_cmp( arg1, ".w" ) )
        {
            *ch->desc->pString = format_string( *ch->desc->pString );
            ch->println("String formatted.");
            return;
        }

        if (!str_cmp(arg1,".-") || !str_cmp(arg1,".d"))
        {
            size_t len;
            bool found = false;  
    
            if (ch->desc->pString == NULL || *ch->desc->pString[0] == '\0')
            {
                ch->println("No lines left to remove.");
                return;
            }
    
            strcpy(buf,*ch->desc->pString);
    
            for (len = str_len(buf); len > 0; len--)
            {
                if (buf[len] == '\n')
                {
                    if (!found)  // back it up 
                    {
                        if (len > 0)
                            len--;
                        found = true;
                    }
                    else // found the second one 
                    {
                        buf[len + 1] = '\0';
                        free_string(*ch->desc->pString);
                        *ch->desc->pString = str_dup(buf);
                        ch->println("Bottom line removed.");
                        return;
                    }
                }
            }
            buf[0] = '\0';
            free_string(*ch->desc->pString);
            *ch->desc->pString = str_dup(buf);
            ch->println("Bottom line removed.");
            return;
        }

	if ( !str_cmp( arg1, ".ld" ) )
	{
		*ch->desc->pString = string_linedel( *ch->desc->pString, atoi(arg2) );
        ch->printlnf("Line %d deleted.", atoi(arg2) );
		return;
	}

	if ( !str_cmp( arg1, ".li" ) )
	{
		*ch->desc->pString = string_lineadd( *ch->desc->pString, tmparg3, atoi(arg2) );
        ch->printlnf("Line '%s' inserted above line %d.", tmparg3, atoi(arg2));
		return;
	}

	// line swap - Kal - Nov 99
	if ( !str_cmp( arg1, ".ls" ) )
	{
		char *first, *second;
		int line1=atoi(arg2);
		int line2=atoi(arg3);

		// validate the numbers
		if(line1<1 || line2<1){
			ch->printf("Line Swap (.ls) requires 2 numbers, both greater than 0.\r\n"
				"(e.g. '.ls 2 5' would swap lines 2 and 5.)\r\n");
			return;
		}
		first=getline_by_number( *ch->desc->pString, line1);
		if(IS_NULLSTR(first)){
			ch->printlnf("Line %d not found!", line2);		
			free_string(first);
			return;
		}
		second=getline_by_number( *ch->desc->pString, line2);
		if(IS_NULLSTR(second)){
			ch->printlnf("Line %d not found!", line2);		
			free_string(first);
			free_string(second);
			return;
		}

		*ch->desc->pString = string_linedel( *ch->desc->pString, line1 );
		*ch->desc->pString = string_lineadd( *ch->desc->pString, second, line1 );
		*ch->desc->pString = string_linedel( *ch->desc->pString, line2 );
		*ch->desc->pString = string_lineadd( *ch->desc->pString, first, line2 );
        ch->printlnf("Line %d swapped with line %d.", line1, line2);
		free_string(first);
		free_string(second);
		return;
	}


	if ( !str_cmp( arg1, ".lr" ) )
	{
		*ch->desc->pString = string_linedel( *ch->desc->pString, atoi(arg2) );
		*ch->desc->pString = string_lineadd( *ch->desc->pString, tmparg3, atoi(arg2) );
        ch->printlnf("Line %d replaced with '%s'.", atoi(arg2), tmparg3);
		return;
	}


        if ( !str_cmp( arg1, ".h" ) )
        {
            ch->println("`xSedit help (commands on blank line):   ");
            ch->println(".r 'old' 'new'      - replace a substring ");
            ch->println("                      (requires '', \"\") ");
			if ( IS_IMMORTAL(ch)){
				ch->println(".z                  - convert ``1 format to ``+ format");
			}
			ch->println(".h                  - get help (this info)");
            ch->println(".s                  - show string so far  ");
            ch->println(".b                  - show string bare with line numbers");
			ch->println("                      and with colour codes");
            ch->println(".n                  - noteformat string (wordwrap but");
			ch->println("                      don't put spaces after fullstops)");
			ch->println(".f                  - noteformat string");
            ch->println(".w                  - wordwrap string (changing spacing)");
            ch->println(".c                  - clear string so far ");
			ch->println(".i                  - code indent mobprog code ");
#ifdef unix
			ch->println(".spell              - spell check string so far ");
			ch->println(".spell <word>       - spell check word");
#endif
            ch->println(".d                  - delete the bottom line");
            ch->println(".-                  - delete the bottom line");
            ch->println(".ld <num>           - delete the line numbered <num>");
            ch->println(".li <num> <str>     - insert <str> before line <num>");
            ch->println(".lr <num> <str>     - replace line <num> with <str>");
            ch->println(".ls <num1> <num2>   - swap lines <num1> and <num2>");
            ch->println("@                   - end string          ");
            ch->println("-External commands-");
            ch->println(".a <text to reply>  - answer a question");
            ch->println(".o <text to ooc>    - ask a question");
            ch->println(".q <text to ask>    - ask a question");
			if (IS_IMMORTAL(ch))
			{
                ch->println(".: or : process external command");           
			}
            
            return;
        }         

        ch->println("SEdit:  Invalid dot command.");
        return;
    }

    if ( *argument == '~' || *argument == '@' )
    {
		// all changed flags system, enables some olc editor to mark
		// something as changed when they come out of the editor
		if(ch->desc->changed_flag){
			SET_BIT(*ch->desc->changed_flag,A); 
			ch->desc->changed_flag=NULL;
		}
		if ( ch->desc->editor == ED_BAN) // banedit 
		{
			save_bans();
		}else if ( ch->desc->editor == ED_HELP ){ // hedit
			help_data *pHelp;
			EDIT_HELP(ch, pHelp);
			get_widest_line_stats(pHelp->text, true, &pHelp->widest_line, &pHelp->widest_line_width);
			if(pHelp->widest_line_width>78){
				ch->printlnf("`RWARNING: Line text is %d characters wide.`x", pHelp->widest_line_width);
			}
		}else if ( ch->desc->editor == ED_MPCODE ){ // mob progs
			MOB_INDEX_DATA *mob;
			int hash;
			MUDPROG_TRIGGER_LIST *mpl;
			MUDPROG_CODE *mpc;

			EDIT_MPCODE(ch, mpc);

			if ( mpc != NULL )
			{
				AREA_DATA *pArea;

				pArea=get_vnum_area( mpc->vnum);
				if (pArea)
				{
					SET_BIT( pArea->olc_flags, OLCAREA_CHANGED );
				}

				// display which mobs was affected by the updating of code 
				// - not really necessary but message is cool :)
				for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
				{
					for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
					{
						for ( mpl = mob->mob_triggers; mpl; mpl = mpl->next )
						{
							if ( mpl->prog == mpc)
							{
                                ch->printlnf("Updated mob %d %s(%s).", 
									mob->vnum, mob->short_descr, mob->player_name);
								continue;
							}
						}
					}
				}

				// autoindent if required
				if(ch->pcdata && ch->pcdata->mpedit_autoindent>=0){
					mpedit_indent(ch, "auto");					
				}
			}
		}

        ch->desc->pString = NULL;
		ch->desc->ftp.inuse=false;
        return;
    }

	strcpy( buf, *ch->desc->pString );

    /*
     * Truncate strings to MSL.
     * --------------------------------------
     */
    if ( str_len( buf ) + str_len( argument ) >= ( MSL - 4 ) )
    {
        ch->println("String too long, last line skipped.");

	/* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

    /*
     * Ensure no tilde's inside string.
     * --------------------------------
     */
    smash_tilde( argument );

    strcat( buf, argument );
    strcat( buf, "\r\n" );
    free_string( *ch->desc->pString );
    *ch->desc->pString = str_dup( buf );
    return;
}



/* Second half of format_string was rewritten by kalahn May 98, 
 * Now wordwraps correct with ` colorcode sequences.
 */
/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_string( char *oldstring /*, bool fSpace */)
{
    char xbuf[HSL];
    char xbuf2[HSL];
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
		else if (*rdesc==' ')
		{
			if (xbuf[i-1] != ' ')
			{
				xbuf[i]=' ';
				i++;
			}
		}
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
		else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
			if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
				(xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
			{
				xbuf[i-2]=*rdesc;
				if (*(rdesc+1) != '\"')
				{
					xbuf[i-1]=' ';
					xbuf[i]=' ';
					i++;
				}
				else
				{
					xbuf[i-1]='\"';
					xbuf[i]=' ';
					xbuf[i+1]=' ';
					i+=2;
					rdesc++;
				}
			}
			else
			{
				xbuf[i]=*rdesc;
				if (*(rdesc+1) != '\"')
				{
					xbuf[i+1]=' ';
					xbuf[i+2]=' ';
					i += 3;
				}
				else
				{
					xbuf[i+1]='\"';
					xbuf[i+2]=' ';
					xbuf[i+3]=' ';
					i += 4;
					rdesc++;
				}
			}
			cap = true;
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
	rdesc=xbuf2;
	xbuf[i]=0;
	strcpy(xbuf2,xbuf);
	
	xbuf[0]=0;
	
	
	/*
	*  Code above here removes all the lines and caps the correct words.
	*  below here puts in lines
	*/
	// code below here was written by Kalahn May 98
	// next wordwrap the string so no line is no greater 
	// than 77 visible characters
	
	// rdesc is a pointer to the start of the string
	// get the formatted version of rdesc into xbuf
	{
		int i=0; // index
		int vischars=0; // visible characters on the current line
		int last_space=0;
		char *point;
		int width=77;
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
                    *point++='\r';
                    *point++='\n';
					i=last_space;
				}
				else // line to long
				{
					logf("noteformatstring: line too long. '%s'", rdesc);
					*point++='-';
                    *point++='\r';
                    *point++='\n';
				}
				
				// setup for next time thru loop
				vischars=0;
				rdesc+=i;
				i=0;
				last_space=-1;
			}
		}
        *point++='\r';
        *point++='\n';
		*point='\0';
	}
	
	free_string(oldstring);
	return(str_dup(xbuf));
	
}


/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is false and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char *first_arg( char *argument, char *arg_first, bool force_lowercase)
{
    char cEnd;

    while ( is_space(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
        if( (is_space(cEnd) && is_space(*argument)) || *argument== cEnd ){
            argument++;
            break;
        }
		if ( force_lowercase) *arg_first = LOWER(*argument);
		else *arg_first = *argument;
		arg_first++;
		argument++;
    }
    *arg_first = '\0';

    while ( is_space(*argument) ){
		argument++;
	}

    return argument;
}


/**************************************************************************/
/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper( char * argument )
{
    char *s;

    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            *s = UPPER(*s);
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return argument;
}

/**************************************************************************/
char *string_linedel( char *string, int line )
{
	char *strtmp = string;
    char buf[HSL];
	int cnt = 1, tmp = 0;

	buf[0] = '\0';

	for ( ; *strtmp != '\0'; strtmp++ )
	{
		if ( cnt != line )
			buf[tmp++] = *strtmp;

		if ( *strtmp == '\n' )
		{
			if ( *(strtmp + 1) == '\r' )
			{
				if ( cnt != line )
					buf[tmp++] = *(++strtmp);
				else
					++strtmp;
			}

			cnt++;
		}
	}

	buf[tmp] = '\0';

	free_string(string);
	return str_dup(buf);
}

/**************************************************************************/
char *string_lineadd( char *string, char *newstr, int line )
{
	char *strtmp = string;
	int cnt = 1, tmp = 0;
	bool done = false;
    char buf[HSL];

	buf[0] = '\0';

	for ( ; *strtmp != '\0' || (!done && cnt == line); strtmp++ )
	{
		if ( cnt == line && !done )
		{
			strcat( buf, newstr );
			strcat( buf, "\r\n" );
			tmp += str_len(newstr) + 2;
			cnt++;
			done = true;
		}

		buf[tmp++] = *strtmp;

		if ( done && *strtmp == '\0' )
			break;

		if ( *strtmp == '\n' )
		{
			if ( *(strtmp + 1) == '\r' )
				buf[tmp++] = *(++strtmp);

			cnt++;
		}

		buf[tmp] = '\0';
	}

	free_string(string);
	return str_dup(buf);
}

/**************************************************************************/
// copy up till a \n from str into buf
// return the position of str after the line has been put into buf
char *getline( char *str, char *buf, int bufsize )
{
	int tmp = 0;
	bool found = false;
	bufsize--; 

	while ( *str && tmp<bufsize){
		if ( *str == '\n' )
		{
			found = true;
			break;
		}
		if(*(str)=='\r'){// don't want \r codes returned at all
			str++;
		}else{
			buf[tmp++] = *(str++);
		}
	}

	// buffer bounds check
	if(tmp>=bufsize){
		boundsbug("getline(): bufsize=%d - insufficient buffer space allocated.", bufsize);
	}

	if ( found )
	{
		// skip over line codes
		if ( *(str + 1) == '\r' ){
			str += 2;
		}else{
			str += 1;
		}
	} 

	buf[tmp] = '\0';

	return str;
}

/**************************************************************************/
// return the lines in a string numbered on the right hand side
// (.s in the string editor)
char *number_lines( char_data *ch, char *string, bool use_mxp )
{
	int cnt = 1;
	static char buf[MSL*4];
	char buf2[MSL*4], tmpb[MSL*4];
	char *string_start=string; 

	buf[0] = '\0';

	while ( *string )
	{
		string = getline( string, tmpb, MSL*4 );

		if(HAS_MXP(ch) && use_mxp){
			sprintf( buf2, "%s. %s\n", 
				mxp_create_send(ch,
					FORMATF("@|.ld %d\" hint=\"exit editor|delete line %d", cnt, cnt)
					,FORMATF("%2d", cnt)) // text seen on screen underlined
			, tmpb );
			cnt++;
		}else{
			sprintf( buf2, "%2d. %s\n", cnt++, tmpb );
		}

		// bounds check
		if(str_len(buf) + str_len(buf2)> MSL*4){
			boundsbug("number_lines(): insufficient buffer space allocated "
				"to display string '%200.200s'.", string_start);
		}
		strcat( buf, buf2 );
	}

	return buf;
}

/**************************************************************************/
// returns the line specified by the number, caller needs to free it
// Kal - Nov 99
char *getline_by_number( char *string, int linenum)
{
    char buf[HSL];

	int cnt = 0;
	if(linenum<1){
		return str_dup("");
	}

	buf[0] = '\0';

	while ( !IS_NULLSTR(string) )
	{
		string = getline( string, buf, HSL);
		if(++cnt==linenum){
			// return our string without \r and ~ codes (removed by fix_string)
			// note: getline doesn't returns up to the first \n 
			return str_dup(fix_string(buf)); 
			break;
		}
	}

	return str_dup("");
}
/**************************************************************************/
// add whitespace around the sides
// remove the name
// remove any double white spaces, and trim the sides
char *string_remove_name(char *str, char *name)
{
	name=lowercase(name);
	// pad with whitespace .. we only work in lowercase
	char *padded=str_dup(lowercase(FORMATF(" %s ", str)));
	free_string(str);

	padded=string_replace_all(padded, FORMATF(" %s ", name), " "); // remove the name
	padded=string_replace_all(padded, "  ", " "); // remove double spaces
	str=str_dup(trim_string(padded)); // trim the sides
	free_string(padded);
	return str;
}
/**************************************************************************/
/**************************************************************************/


