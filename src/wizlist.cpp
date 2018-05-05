/**************************************************************************/
// wizlist.cpp - wizlist related commands
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "laston.h"

extern laston_data *laston_list;
void do_help(char_data *ch, char *);
/**************************************************************************/
char *	const	wiz_titles	[] =
{
	"`S  Founder   ",
    "`SImplementor ",
    "   `mCreators ",
    "`BSupremacies ",
    " `rImmortals  ",
    " `rImmortals  ",
    " `rImmortals  ",
    " `rImmortals  ",
    " `rImmortals  ",
    " `rImmortals  "
};

/**************************************************************************/
// Balo & Kal, Feb 02
void display_dynwizlist_to_char(char_data *ch)
{
	laston_data *node;
    char buf[MSL];
	char buf2[MSL];
	char divider[MSL];
    BUFFER *buffer;
    int level;
    int i;
    int amt;
    bool found;
	bool donethis;
	int wlevel = 0;
	

	// setup for formatting the dynamic wizlist
	buffer = new_buf();
	sprintf(divider,"`y    |`W%33c-=-`y%33c|\r\n",' ',' ');

	// add our header to the top
	add_buf(buffer,
		"    `r                        /|                 |\\ \r\n"
		"                           / | ___.--'''--.___ | \\\r\n"
		"      ...--=.._           /  ''`Y___`r'\\_   _/'`Y___`r''  \\        _..=--... \r\n"
		"     '   .-=_)/==._     _ \\ .('  `?`#o`r'-.\\ /.-'`^o`r  '). / _  _.==\\(_=-.   ''-._ \r\n"
		"       _/.-  /         / \\/\\_ '---'_-=V=-_'---' _/\\/ \\      \\  -.\\_      - \r\n"
		"      /_/  ./          \\/\\_-_''v-/'o') ('o'\\-v''_-_/\\/       \\.  \\_\\      ' \r\n"
		"     //    |          _-=___==/(__         __)\\==___=-_       |    \\\\\\ \r\n"
		"      ))    \\          _/ _ \\ X___---===---___X / _ \\_       /    (( ))\\ \r\n"
		"     \\ \\_   |         /_\\/_\\ (( `Y\\| `` `` ' ' |/`r )) /_\\/_\\      |   _/ /   \\ \r\n"
		"      \\  \\   \\       ''  / _ /\\\\ `YV`R  /'V')`Y  V`r //\\ _ \\  ''    /   /  /     \\ \r\n"
		"       \\_ \\  |           \\/ \\\\ \\\\^ `R \\ )/ `r  ^//|// \\/        |  / _/       \\ \r\n"
		"         \\ \\  \\               '/\\\\^ `R )/`r   ^// |``           /  / / \r\n"      
		"          \\ \\_ |             '| (\\\\^ `RV `r  ^//  |``          | _/ / \r\n"       
		"           \\  \\\\             '/(''\\``-___-'/) /``           //  / \r\n"        
		"            \\_ \\\\ /|        '|('''-\\_   _/)  |``          |\\ _/ \r\n"         
		"              \\_/''''-.     '/('''---==='')  |``      .-'''''\\_ \r\n"        
		"             _/   _    \\   '|('''-----''')  /``      /     _   \\_ \r\n"   
		"            // / /\\\\')  \\  '|('''-----''')  |``     /   ('//\\ \\ \\X \r\n"    
		"`y  /\\==---`r/ \\ \\ )`y--=====----=====------====-----=====-----`r( / / \\`y--====---\\ \r\n"
		"`y |/\\      `r\\ (\\_)`Y\\`=?                                         `Y/`r(_/) /         `y\\ \r\n"  
		"`y \\_/|     `r\\_)`Y\\                                            `Y/`r(_/            `y|  \r\n");
    
    
	sprintf(buf2,"`b---`B======`c[ `WThe Immortals of The Realm `c]`B======`b--- ");
	
	sprintf(buf,"`y    |%69s| \r\n", " ");
    add_buf(buffer,buf);
	
	i= (69 - c_str_len(buf2))/2;
	add_buf(buffer,FORMATF("`y    |%*c%s%*c`y|\r\n", i,' ', buf2, i,' '));

	
	sprintf(buf,"`y    |%69s| \r\n", " ");
    add_buf(buffer,buf);
	
	donethis = false;
	
	for (level = ABSOLUTE_MAX_LEVEL; level > LEVEL_IMMORTAL; level--){
		found = false;
		amt = 0;
		
		for (node = laston_list; node; node=node->next){
			wlevel = node->level[node->index];
			
			if(wlevel == level
				&& !node->deleted_date
				&& node->wiznet_type==LASTONWIZLISTTYPE_ACTIVE
				) 
			{
				amt++;
				found = true;
			}
			
		}			
		
		if (!found){
			if (level == LEVEL_IMMORTAL)
			{
				sprintf(buf,"`y    |%69s|\r\n", " ");
				add_buf(buffer,buf);
			}
			continue;
		}
		
		if (!IS_NULLSTR(wiz_titles[ABSOLUTE_MAX_LEVEL-level])){
			if (level == ABSOLUTE_MAX_LEVEL && ABSOLUTE_MAX_LEVEL!=MAX_LEVEL){
				sprintf(buf,"`y    |`R%37s `B[%d]`y%28s|\r\n",
					wiz_titles[0], level, " ");
				add_buf(buffer,buf);
			} 
			if (level == MAX_LEVEL){
				sprintf(buf,"`y    |`R%37s `B[%d]`y%28s|\r\n",
					wiz_titles[1], level, " ");
				add_buf(buffer,buf);
			} 
			
			if ( level == CREATOR ){
				sprintf(buf,"`y    |`R%36s `B  [%d]`y%28s|\r\n",
					wiz_titles[2], level, " ");
				add_buf(buffer,buf);
			}
			if ( level == SUPREME ){
				sprintf(buf,"`y    |`R%36s `B  [%d]`y%28s|\r\n",
					wiz_titles[3], level, " ");
				add_buf(buffer,buf);
			}			
			
			
			if (( level < SUPREME ) && ( donethis == false ))
			{
				sprintf(buf,"`y    |`R%34s `B   [%s]`y%26s|\r\n",
					wiz_titles[4], "92-97", " ");
				add_buf(buffer,buf);
				
				
				add_buf(buffer,divider);
				donethis = true;
			}
			
			if ( ! donethis ){
				add_buf(buffer,divider);
			}
		}
		
		int lngth = 0;
		for (node= laston_list; node; node=node->next){
			if((node->level[node->index] == level )
				&& ! node->deleted_date
				&& node->wiznet_type==LASTONWIZLISTTYPE_ACTIVE
				)
				
			{
				if (lngth == 0){
					if (amt > 2){
						sprintf(buf, "`y    |`%s%12s%-17s ",
							level >= DEMI ? "c" : "c", " ",
							node->name );
						add_buf(buffer, buf);
						lngth = 1;
					}else if (amt > 1){						
						sprintf(buf, "`y    |`%s%21s%-17s ",
							level >= DEMI ? "c" : "c", " ",
							node->name);
						add_buf(buffer, buf);
						lngth = 1;						
					}else{
						sprintf(buf, "`y    |`%s%30s%-39s`y|\r\n",
							level >= DEMI ? "c" : "c", " ",
							node->name);
						add_buf(buffer, buf);
						lngth = 0;
					}
				}else if (lngth == 1){
					if (amt > 2){
						sprintf(buf, "%-17s ",
							node->name);
						add_buf(buffer, buf);
						lngth = 2;
					}else{
						sprintf(buf, "%-30s`y|\r\n",
							node->name);
						add_buf(buffer, buf);
						lngth = 0;
					}
				}else{
					sprintf(buf, "%-21s`y|\r\n",
						node->name);
					add_buf(buffer, buf);
					lngth = 0;
					//amt = 0;
					amt -= 3;
				}
			}
		}
	}			
		
    add_buf(buffer, FORMATF("`y  /\\|%69c|\r\n", ' '));    
    add_buf(buffer, FORMATF(" |\\/|%69c/\r\n", ' '));
    add_buf(buffer, "  \\/__________________________________"
						"___________________________________/`x\r\n");  
	
	ch->sendpage( buf_string(buffer));
	free_buf(buffer);
	return;
}
/**************************************************************************/
void do_wizlist(char_data *ch, char *)
{
	int amount=0;
    if(ch->lines){
		amount=25;
	}
	ch->lines += amount;
	if(GAMESETTING3(GAMESET3_USE_DYNAMIC_WIZLIST)){
		display_dynwizlist_to_char(ch);
	}else{
		if(!codehelp(ch,"wizlist", 0)){
			display_dynwizlist_to_char(ch);
			if(IS_ADMIN(ch)){
				ch->println("`S[admin only note: help entry code_wizlist unfound, using dynamic wizlist, to]`x");
				ch->println("`S[                 remove this message, turn on the dynamic wizlist in the   ]`x");
				ch->println("`S[                 game settings flags3, or setup the code_wizlist help entry]`x");
			}
		};
	}
	ch->lines -= amount;
}
/**************************************************************************/
void do_wizlistedit( char_data *ch, char *argument )
{
    laston_data *node;
	int count=0;

	if(IS_NULLSTR(argument)){
		ch->titlebar("Wizlist Edit - Current Settings");
		ch->println(" Name.  [`Mlevel, `Btrust, `Rsecurity`x]- wizlist flags");
    		for (node= laston_list; node; node=node->next)
		{
			if((node->level[node->index]<LEVEL_IMMORTAL
				  && node->trust<LEVEL_IMMORTAL)
				  || node->deleted_date){
				continue;	
			}

			ch->printf(" `%c%-13s `S[`M%3d`S,`%c%3d`S,`R%d`S]`G- `Y%-13s`x", 
				node->wiznet_type?'W':'x',
				node->name,
				node->level[node->index],	
				node->level[node->index]<node->trust?'C':'B',
				node->trust,
				node->security,
				flag_string(laston_wizlist_types, node->wiznet_type));			
			if(++count%2==0){
				ch->print_blank_lines(1);
			}
		}
		if(count%2==1){
			ch->print_blank_lines(1);
		}
		ch->printlnf(" %d immortal record%s displayed.\r\n", count, count==1?"":"s");

		ch->titlebar("WizlistEdit Syntax");
		ch->println(" Syntax: wizlistedit <name> <wizlist_type>");
		ch->print(" Wizlist types include:");
		for(count=0; !IS_NULLSTR(laston_wizlist_types[count].name); count++){
			ch->printf("  %s  ", laston_wizlist_types[count].name);
		}
		ch->print_blank_lines(1);
		ch->titlebar("");
		return;
	}
	
	{ // modify the wizlist settings on a player
		char name[MIL];
		
		// split and check the parameters
		argument=one_argument(argument, name);

		int wizlist_type=flag_lookup(argument,laston_wizlist_types);
			
		if ( wizlist_type== NO_FLAG)
		{
			do_wizlistedit(ch,"");
			ch->printlnf("'%s' is not a valid wizlist type.",argument);
			return;
		}

		// find the player
		for (node= laston_list; node; node=node->next)
		{	
			// immortals only
			if (node->level[node->index]<LEVEL_IMMORTAL
				  && node->trust<LEVEL_IMMORTAL)
				continue;	

			if(!str_cmp(name,node->name)){
				node->wiznet_type=wizlist_type;
				ch->printlnf("Wizlist type set to %s for %s.", 						
						flag_string( laston_wizlist_types, node->wiznet_type),
						node->name);			
				return;
			}
		}

		ch->printlnf("Couldn't find any immortal character called '%s'", name);
		ch->println(" Syntax: wizlistedit <name> <wizlist_type>");
		ch->print(" Wizlist types include:");
		for(count=0; !IS_NULLSTR(laston_wizlist_types[count].name); count++){
			ch->printf("  %s  ", laston_wizlist_types[count].name);
		}
		ch->print_blank_lines(1);
	}
}

/**************************************************************************/
/**************************************************************************/
