/**************************************************************************/
// class.cpp - class configuring system, Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h" // dawn standard includes
#include "magic.h"   // used for spell_null definition

void do_copyclass(char_data *ch, char * argument);
int class_lookup(const char *name);

char * get_spinfo_data(char_data *ch, int sn);
char *tochar_spellfunction(SPELL_FUN *psp);

/**************************************************************************/
void do_write_skills(char_data *ch, char *);
/**************************************************************************/
/**************************************************************************/
int calc_spell_percent(int clss, int level, int sn);
/**************************************************************************/
// Kal
bool make_clss_spell_table(int clss)
{
	FILE *fp;
    char buf[MSL];
    char buf2[MSL];
    int sp_lev, lev, i, curve_chance;
    int first_spell = FIRST_SPELL;
    int last_spell = LAST_SPELL;

    sprintf (buf, "%s%.8s.csv", CLASSES_DIR, class_table[clss].name);

    logf ("Generating %s class table into %s",
        class_table[clss].name,buf);


	if (!(fp = fopen (buf, "w")))
	{
        bugf ("Could not open file %s in order to generate percentage table for %s.",
            buf, class_table[clss].name);
        return false;
	}

    fprintf (fp, "Spell percentage table for class %s\n", class_table[clss].name);
    fprintf (fp,"\n");

    // header
    fprintf (fp, "\"%s\",%s,%s,%s,\"|\",","Spell", "level","prac rating", "low%");
    for (i = 1; i <= LEVEL_IMMORTAL; i++)
    {
        fprintf (fp,"%d,",i);
    }
    fprintf (fp,"\n");

    for (sp_lev = 0; sp_lev < MAX_LEVEL+10; sp_lev++)
    {
        if (sp_lev == LEVEL_IMMORTAL)
        {
            // header 
            fprintf (fp, "\"%s\",%s,%s,%s,\"|\",","Spell", "level","prac rating", "low%");
            for (i = 1; i <= LEVEL_IMMORTAL; i++)
            {
                fprintf (fp,"%d,",i);
            }
            fprintf (fp,"\n");
        }
        for (i = first_spell; i <= last_spell ; i++)
        {
            if (skill_table[i].skill_level[clss] == sp_lev)
            {
                if (!skill_table[i].name || !skill_table[i].name[0])
                    continue;
        
                fprintf (fp, "\"%s\",%d,%d,%d,\"|\",",
                                skill_table[i].name,
                                skill_table[i].skill_level[clss],
                                skill_table[i].rating[clss],
                                skill_table[i].low_percent_level[clss]
                                );
        
                for (lev = 1; lev <= LEVEL_IMMORTAL && skill_table[i].skill_level[clss]<LEVEL_IMMORTAL; lev++)
                {
                    curve_chance = calc_spell_percent(clss, lev, i);
         
                    sprintf(buf2,"%2d.%.2d,", curve_chance/100, curve_chance%100);
                    if (curve_chance==9999)
                        sprintf(buf2, "*,");               

                    if (skill_table[i].rating[clss]<1)
                        sprintf(buf2, "X,");
                    fprintf (fp,"%s",
                        (skill_table[i].skill_level[clss]<LEVEL_IMMORTAL?buf2:"**imm only**"));
                }
                fprintf (fp,"\n");
            }
        }
    }

	fclose (fp);

    return true;
}


/**************************************************************************/
// Kal
void show_class_spell_table(char_data *ch, int clss)
{
    BUFFER *buffer;
    char buf[MSL];
    int sp_lev, i;
    int first_spell = FIRST_SPELL;
    int last_spell = LAST_SPELL;

    buffer = new_buf();

    for (sp_lev = 1; sp_lev < MAX_LEVEL+10; sp_lev++)
    {
        for (i = first_spell; i <= last_spell ; i++)
        {
            if (skill_table[i].skill_level[clss] == sp_lev)
            {
                if (!skill_table[i].name || !skill_table[i].name[0])
                    continue;

                sprintf(buf,"%s - %-15.15s lvl=%2d, rating=%2d, low%%level=%2d (sn=%d)\r\n",
                    class_table[clss].short_name,
                    skill_table[i].name,
                    skill_table[i].skill_level[clss],
                    skill_table[i].rating[clss],
                    skill_table[i].low_percent_level[clss],
                    i
                    );
                add_buf(buffer, buf);
       
            }
        }
    }
    for (i = first_spell; i <= last_spell ; i++)
    {
        if (skill_table[i].skill_level[clss] == 0)
        {
            if (!skill_table[i].name || !skill_table[i].name[0])
                continue;

            sprintf(buf,"%s - %-15.15s unattainable (sn=%d)\r\n",
                class_table[clss].short_name,
                skill_table[i].name,
                i
                );
            add_buf(buffer, buf);
   
        }
    }
    ch->sendpage(buf_string(buffer));
    free_buf(buffer);
}
/**************************************************************************/
// Kal
void show_class_skill_table(char_data *ch, int clss)
{
    BUFFER *buffer;
    char buf[MSL];
    int sp_lev, i;

    buffer = new_buf();

    for (sp_lev = 1; sp_lev < MAX_LEVEL+10; sp_lev++)
    {
		for (i = 0; i< MAX_SKILL; i++)
		{
			if (IS_NULLSTR(skill_table[i].name))
				break;

			if (skill_table[i].spell_fun != spell_null)
				continue;

            if (skill_table[i].skill_level[clss] == sp_lev)
            {
                if (!skill_table[i].name || !skill_table[i].name[0])
                    continue;

                sprintf(buf,"%s - %-15.15s lvl=%2d, rating=%2d, low%%level=%2d (sn=%d)\r\n",
                    class_table[clss].short_name,
                    skill_table[i].name,
                    skill_table[i].skill_level[clss],
                    skill_table[i].rating[clss],
                    skill_table[i].low_percent_level[clss],
                    i
                    );
                add_buf(buffer, buf);
       
            }
        }
    }
	for (i = 0; i< MAX_SKILL; i++)
	{
		if (IS_NULLSTR(skill_table[i].name))
			break;

		if (skill_table[i].spell_fun != spell_null)
			continue;

        if (skill_table[i].skill_level[clss] == 0)
        {
            if (!skill_table[i].name || !skill_table[i].name[0])
                continue;

            sprintf(buf,"%s - %-15.15s unattainable (sn=%d)\r\n",
                class_table[clss].short_name,
                skill_table[i].name,
                i
                );
            add_buf(buffer, buf);
  
        }
    }
    ch->sendpage(buf_string(buffer));
    free_buf(buffer);
}

/**************************************************************************/
// Kal
void make_alltables()
{
	int i;

    log_string ("Generating all classes.");
	for (i = 0; !IS_NULLSTR(class_table[i].name); i++)
    {
        if (make_clss_spell_table(i))
            logf("%s percentage table generated and saved to %s%s.csv\r\n", 
				class_table[i].name,CLASSES_DIR, class_table[i].name);
        else
            logf("%s percentage table generated and saved to %s%s.csv\r\n", 
				class_table[i].name,CLASSES_DIR, class_table[i].name);
    }
}

/**************************************************************************/
// Kal
static void display_classrating_info(char_data *ch, int clss_no, int sn)
{
	if(skill_table[sn].skill_level[clss_no]!=0 
		&& skill_table[sn].skill_level[clss_no]<LEVEL_IMMORTAL)
	{
		ch->printlnf("%s - %s lvl=%2d, rating=%2d, low%%level=%2d, maxprac=%2d, maxlearn=%2d",
		class_table[clss_no].short_name,
		skill_table[sn].name,
		skill_table[sn].skill_level[clss_no],
		skill_table[sn].rating[clss_no],
		skill_table[sn].low_percent_level[clss_no],
		skill_table[sn].maxprac_percent[clss_no],
		skill_table[sn].learn_scale_percent[clss_no]);
	}else{
		ch->printlnf("%s - %s (unattainable) - level=%d",
			class_table[clss_no].short_name, 
			skill_table[sn].name,
			skill_table[sn].skill_level[clss_no]);
	}
}
/****************************************************************************/
// Kal
void do_cinfo(char_data *ch, char * argument)
{
	int sn;
	
	if(IS_NULLSTR(argument)){
		ch->println("Syntax: cinfo <part_of_a_spellname>");
		return;
	}
	
	ch->titlebar("CINFO");
	ch->println("SINFO: Required realms, spheres, elements & seasons(see help spinfo-key)\r\n"
		"50%% level on spells with colour codes:\r\n"
		"`b1`x-`c8`x-`g14`x-`m20`x-`r26`x-`y32"
		"`x-`S38`x-`w44`x-`W50`x-`Y56`x-`R62`x-`M68`x-`G74"
		"`x-`C80`x-`B86+`x\r\n"		
		"____________________CLASS____________________________________________");
	
	for(sn=0; sn < MAX_SKILL; sn++ )
	{
		if ( skill_table[sn].name == NULL )
			break;
		if(!str_cmp("reserved", skill_table[sn].name))
			continue;
		if(is_name(argument, skill_table[sn].name))
		{			
			int tlevel, clss_no;
			ch->print( get_spinfo_data(ch, sn));
			if(IS_SPELL(sn)){
				ch->printlnf("`=rSpell Function: `x%s", 
					tochar_spellfunction(skill_table[sn].spell_fun));
			}
			for (tlevel=1; tlevel<LEVEL_IMMORTAL; tlevel++)
			{
				for (clss_no = 0; class_table[clss_no].name; clss_no++)
				{
					if (skill_table[sn].skill_level[clss_no]==tlevel 
						&& skill_table[sn].rating[clss_no]!=0)
					{
						display_classrating_info(ch, clss_no,sn);
					}
				}     
			}
			for (clss_no = 0; class_table[clss_no].name; clss_no++)
			{
				if (skill_table[sn].skill_level[clss_no]==0
					&& skill_table[sn].rating[clss_no]!=0)
				{
					display_classrating_info(ch, clss_no,sn);
				}
			}     
		}
	}
	return;
}
/**************************************************************************/
// Kal
void do_class (char_data *ch ,char * argument)
{
    char clss_name[MIL], skill_name[MIL],
        level_buf[MIL], rating_buf[MIL], low_percent_level_buf[MIL];
    int sn, clss_no, level, rating, low_percent_level, tlevel;
    bool setting_for_all= false;

	if (IS_NPC(ch))
	{
		do_huh(ch,"");
        return;
	}

	if (IS_NULLSTR(argument))
	{
        ch->println( "Syntax is: CLASS <class> <skill> [<level> <rating> <low_percent_level>]." );
        ch->println( "If you just type CLASS <class> <skill> - it will show you the current settings." );
        ch->println( "(use 0 for level, rating and low_percent_level for unchanged." );
        ch->println( "   or CLASS <class> showspells" );
        ch->println( "   or CLASS <class> showskills" );
        ch->println( " * or CLASS <class> maketable" );
        ch->println( " * or CLASS makealltables" );
        ch->println( " * or CLASS saveall" );
        ch->println( " * or CLASS copy <from> <to>" );
        ch->println( " * `YNote: `WSetting the level to the same value as the low_percent_level\r\n"
			"   means the spell can no longer be taught.`x" );
        ch->println( " * = restricted commands." );
		return;
	}
	argument = one_argument (argument, clss_name);
	argument = one_argument (argument, skill_name);


    if (IS_TRUSTED(ch,MAX_LEVEL-1)
        || IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADBALANCE)
		|| IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADREALM))
	{// RESTRICTED COMMANDS BELOW
		if (!str_cmp(clss_name,"makealltables"))
		{
			ch->printlnf("Class percentage tables generated and saved in %s", CLASSES_DIR);
			ch->println("(Check the system logs if there were any problems)");
			make_alltables();
			return;
		}

		if (!str_cmp(clss_name,"copy")){
			char buf[MIL];
			sprintf(buf,"'%s' %s", skill_name, argument);
			do_copyclass(ch, buf);
			return;
		}

		if (!str_cmp(clss_name,"saveall"))
		{
			ch->println("Not required.");
			return;
		}
	}// RESTRICTED COMMANDS ABOVE

    if (!str_cmp(clss_name,"all"))
    {
        setting_for_all = true;
        clss_no = 0; // a default clss, to avoid stupid errors 
    }
    else
    {
        for (clss_no = 0; class_table[clss_no].name; clss_no++){
            if (!str_cmp(clss_name, class_table[clss_no].short_name) ||
                !str_prefix(clss_name,class_table[clss_no].name))
                break;
		}
    
        if (!class_table[clss_no].name)
        {
            ch->printlnf("No class named '%s' exists. Use the 3-letter WHO names (Mag, Cle, Spf etc.)", clss_name);
            return;
        }
    }

    if (!str_cmp(skill_name,"maketable"))
    {
        if (setting_for_all)
        {
            ch->printlnf("Class percentage tables generated and saved in %s", CLASSES_DIR);
            ch->println("(Check the system logs if there were any problems)");
            make_alltables();
            return;
        }
        else
        {
            if (make_clss_spell_table(clss_no))
                ch->printlnf("Class percentage table generated and saved to %s%s.csv", 
					CLASSES_DIR, class_table[clss_no].name);
            else
                ch->printlnf("Class percentage table generated and saved to %s%s.csv", 
					CLASSES_DIR, class_table[clss_no].name);
        }
        return;
    }

    if (!str_cmp(skill_name,"showspells"))
    {
        if (setting_for_all)
        {
            ch->wrapln("You can't show all spells of all the class "
				"tables at the same time.");
        }
        else
        {
            show_class_spell_table(ch, clss_no);
        }
        return;
    }

    if (!str_cmp(skill_name,"showskills"))
    {
        if (setting_for_all){
            ch->wrapln("You can't show all skills of "
				"all the class tables at the same time.");
        }else{
            show_class_skill_table(ch, clss_no);
        }
        return;
    }

    if ( (sn = skill_lookup (skill_name)) == -1)
	{
		ch->printlnf("There is no such spell/skill as '%s'.", skill_name);
		return;
	}


    if (setting_for_all)
    {
        for (tlevel=1; tlevel<MAX_LEVEL; tlevel++)
		{
			for (clss_no = 0; class_table[clss_no].name; clss_no++)
			{
				if (skill_table[sn].skill_level[clss_no]==tlevel)
				{
					display_classrating_info(ch, clss_no,sn);
				}
			}     
		}
		for (clss_no = 0; class_table[clss_no].name; clss_no++)
		{
			if (skill_table[sn].skill_level[clss_no]==0)
			{
				display_classrating_info(ch, clss_no,sn);
			}
		}     
    }
    else
    {
		display_classrating_info(ch, clss_no,sn);
    }    


	if (IS_NULLSTR(argument))
    {
        return;
    }

	if (!IS_ADMIN(ch)
        && !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADBALANCE)
		&& !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_HEADREALM))
	{
		ch->println("You can't modify anything with your level of access.");
        return;
	}

    argument = one_argument (argument, level_buf);
    argument = one_argument (argument, rating_buf);
	if (IS_NULLSTR(argument))
    {
        return;
    }
    argument = one_argument (argument, low_percent_level_buf);


    level = atoi (level_buf);
    rating = atoi (rating_buf);
    low_percent_level = atoi (low_percent_level_buf);

    if (!is_number(level_buf) || level < 0 || level > LEVEL_IMMORTAL)
	{
		ch->printlnf("Level range is from 0 to %d.", LEVEL_IMMORTAL);
		return;
	}

    if (!is_number(rating_buf) || rating < 0 || rating > 30)
	{
        ch->println("Rating range is from 0 to 30.");
		return;
	}

    if (!is_number(low_percent_level_buf)
      || low_percent_level < -1
      || (level && (low_percent_level > level))
      || (!level && (low_percent_level > skill_table[sn].skill_level[clss_no])))
	{
        ch->println("low_percent_level must be between -1 and the level of the spell (inclusive).");
        ch->println("-1 will set the low percent to 0, 0 will leave the lowpercent unchanged.");
		return;
	}

    if (setting_for_all)
    {
        for (clss_no = 0; class_table[clss_no].name; clss_no++)
        {
            if (level)
                skill_table[sn].skill_level[clss_no]       = level;
            if (rating)
                skill_table[sn].rating[clss_no]            = rating;
            if (low_percent_level!=0){
				if(low_percent_level==-1){
					low_percent_level=0;
				}
                skill_table[sn].low_percent_level[clss_no] =low_percent_level;
			}
        
            ch->println( "Has been changed to:" );
			display_classrating_info(ch,clss_no,sn);
        }
    }
    else
    {
        if (level)
            skill_table[sn].skill_level[clss_no]       = level;
        if (rating)
            skill_table[sn].rating[clss_no]            = rating;

        if (low_percent_level!=0){
			if(low_percent_level==-1){
				low_percent_level=0;
			}
            skill_table[sn].low_percent_level[clss_no] =low_percent_level;
		}
    
        ch->println( "Has been changed to:" );
		display_classrating_info(ch,clss_no,sn);
    }
	do_write_skills(NULL,"");
}

/**************************************************************************/
// Kal
void do_copyclass(char_data *ch, char * argument)
{
	char fromClassBuf[MIL], toClassBuf[MIL];
	int fromClassIndex, toClassIndex;
	int i;

	argument = one_argument (argument, fromClassBuf);
	fromClassIndex=class_lookup(fromClassBuf);
	if(fromClassIndex<0){
        ch->printlnf("Couldn't find the class '%s' to copy from!", fromClassBuf);
		return;
	}

	argument = one_argument (argument, toClassBuf);	
	toClassIndex=class_lookup(toClassBuf);
	if(toClassIndex<0){
        ch->printlnf("Couldn't find the class '%s' to copy from!", toClassBuf);
		return;
	}

	if(class_table[toClassIndex].creation_selectable){
		ch->wraplnf("The '%s' class is creation selectable... you can only copy over a class that is not creation selectable.",
			class_table[toClassIndex].name);
		return;
	}
	

	for (i = 0; i< MAX_SKILL; i++){
        skill_table[i].skill_level[toClassIndex]
			=skill_table[i].skill_level[fromClassIndex];
        skill_table[i].rating[toClassIndex]
			=skill_table[i].rating[fromClassIndex];
        skill_table[i].low_percent_level[toClassIndex]
			= skill_table[i].low_percent_level[fromClassIndex];
    }
    ch->printlnf("Skill & Spell copy from '%s' to '%s' completed... use write_skills to save the copy.", fromClassBuf, toClassBuf);

}
/**************************************************************************/
/**************************************************************************/

