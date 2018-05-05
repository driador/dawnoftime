/**************************************************************************/
// skills.cpp - gain, train, prac etc
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

#include "magic.h"
#include "d2magsys.h"
#include "math.h"
#include "nanny.h"

// command procedures needed
DECLARE_DO_FUN(do_skillgroups);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_spinfo	);
DECLARE_DO_FUN(do_cspinfo	);
DECLARE_DO_FUN(do_cskinfo	);

/**************************************************************************/
// used to get new skills
void do_gain(char_data *ch, char *argument)
{
    char arg[MIL];
    char_data *trainer;
    int gn = 0, sn = 0;
	
    if (IS_NPC(ch))
		return;
	

	one_argument(argument,arg);

	// gain list no longer requires you to be at the healers
	// - Kal, Feb 06
	if (!IS_NULLSTR(arg) && !str_prefix(arg,"list"))
	{
		int col;
		
		col = 0;
		

		ch->printlnf("%-28s %-5s    %-28s %-5s",
			"skillgroup","cost","skillgroup","cost");
				
		// skillgroups at the top
		for (gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++)
		{
			if (!ch->pcdata->skillgroup_known[gn]
				&&  IS_SKILLGROUP_SELECTABLE_FOR_CHAR(gn, ch))
			{
				ch->printf("%s%-28s %-5d    ",
					skillgroup_table[gn].rating[ch->clss]>ch->train?"`S":"`x",
					skillgroup_table[gn].name, 
					skillgroup_table[gn].rating[ch->clss]);
				if (++col % 2 == 0){
					ch->print_blank_lines(1);
				}
			}
		}
		if (col % 2 != 0)
		{
			ch->println("");
		}
		// skillgroups up above
		
		ch->println("`x");
		
		// skills below
		col = 0;   
		ch->print("`?`#`Yskill `^=============="
			"`Ycost`^==`Ylevel`^=======");
		ch->println("`Yskill`^==============`Ycost" 
			"`^==`Ylevel`x");
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (IS_NULLSTR(skill_table[sn].name)){
				break;
			}

			if(IS_SET(skill_table[sn].flags,SKFLAGS_NO_GAIN)){
				continue;
			}
			
			if( IS_SET(skill_table[sn].flags,SKFLAGS_USE_RACE_RESTRICTIONS)
				&& !IS_SETn(skill_table[sn].race_restrict_n, ch->race))
			{
				continue;
			}
			
			if (!ch->pcdata->learned[sn]
				&&  skill_table[sn].rating[ch->clss] > 0
				&&  skill_table[sn].skill_level[ch->clss]< LEVEL_IMMORTAL
				&&  skill_table[sn].skill_level[ch->clss]>0
				&&  skill_table[sn].spell_fun == spell_null)
			{
				ch->printf("%s%-18s %3d  %4d         ",
					skill_table[sn].rating[ch->clss]>ch->train?"`S":"`x",
					skill_table[sn].name,
					skill_table[sn].rating[ch->clss],
					skill_table[sn].skill_level[ch->clss]);
				if (++col % 2 == 0)
					ch->print_blank_lines(1);
			}
		}
		
		if (col % 2 != 0){
			ch->println("`x");
		}else{
			ch->print("`x");
		}

		ch->println("`^-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-`x");	
		// display number of trains and pracs, then comments about converting between
		ch->printlnf("You have %d train%s, and %d practice%s.",
			ch->train,
			ch->train==1?"":"s",
			ch->practice,
			ch->practice==1?"":"s");
		ch->println("`Snotes: It is possible to convert 10 pracs into 1 train with 'gain convert'.");
		ch->println("You can also convert 1 train into 10 pracs with 'gain revert' after lvl9.`x");
		ch->println("`^-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-`x");	
		
		return;
		// skills above
	}// gain list


    // find a trainer
    for ( trainer = ch->in_room->people; trainer; trainer = trainer->next_in_room)
	{
		if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
			break;
	}

	if (trainer == NULL || !can_see(ch,trainer))
	{
		ch->println( "You can't do that here." );
		return;
	}
	

	if(IS_NULLSTR(arg))
	{
		do_say(trainer,"Pardon me?");
		return;
	}
	
	
	if ( ch->position < POS_STANDING )
	{
		ch->println( "You need to stand first." );
		return;
		
	}
	
	
	if (!str_prefix(arg,"convert"))
	{
		if (ch->practice < 10)
		{
			act("$N tells you 'You are not yet ready.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		act("$N helps you apply your practice to training",
			ch,NULL,trainer,TO_CHAR);
		ch->practice -= 10;
		ch->train++;
		return;
	}

	if (!str_prefix(arg,"revert"))
	{
		if (ch->train < 1)
		{
			act("$N tells you 'You are not yet ready.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}

		if(ch->level<10)
		{
			ch->println("You can't revert trains before level 10.");
			return;
		}
		
		act("$N helps you apply your training to practice",
			ch,NULL,trainer,TO_CHAR);
		ch->practice+=10;
		ch->train--;
		return;
	}
	
	if (!str_prefix(arg,"points"))
	{
		if (ch->train < 2)
		{
			act("$N tells you 'You are not yet ready.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		if (ch->pcdata->points <= 40)
		{
			act("$N tells you 'There would be no point in that.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		act("$N trains you, and you feel more at ease with your skills.",
			ch,NULL,trainer,TO_CHAR);
		
		// record how much xp they have beyond their current level
		int beyond=UMAX(0, ch->exp - (exp_per_level(ch,ch->pcdata->points) * ch->level));
		// reduce their creation points
		ch->train -= 2;
		ch->pcdata->points -= 1;
		ch->exp = exp_per_level(ch,ch->pcdata->points) * ch->level;
		// give them back the xp they already had on that given level
		if(beyond){
			gain_exp(ch, beyond);
		}
		return;
	}
	
	/* else add a group/skill */
	
	gn = skillgroup_lookup(argument);
	if (gn > 0)
	{
		if (ch->pcdata->skillgroup_known[gn])
		{
			act("$N tells you 'You already know that skillgroup!'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		if (skillgroup_table[gn].rating[ch->clss] <= 0)
		{
			act("$N tells you 'That skillgroup is beyond your powers.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		if (ch->train < skillgroup_table[gn].rating[ch->clss])
		{
			act("$N tells you 'You are not yet ready for that skillgroup.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}

		if (!IS_SKILLGROUP_SELECTABLE_FOR_CHAR(gn, ch)){
			ch->printlnf("%s tells you 'You aren't ready for the skillgroup '%s' at this point in time.'",
				PERS(trainer, ch), skillgroup_table[gn].name);
			return;
		}
		
		// add the group 
		gn_add(ch,gn);
		act("$N trains you in the art of $t",
			ch,skillgroup_table[gn].name,trainer,TO_CHAR);
		ch->train -= skillgroup_table[gn].rating[ch->clss];
		set_char_magic_bits(ch); // fix up their magic bits
		return;
	}
	
	sn = skill_lookup(argument);
	if (sn > -1)
	{
		if (skill_table[sn].spell_fun && skill_table[sn].spell_fun != spell_null)
		{
			act("$N tells you 'You must learn the full group.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		if( IS_SET(skill_table[sn].flags,SKFLAGS_USE_RACE_RESTRICTIONS)
			&& !IS_SETn(skill_table[sn].race_restrict_n, ch->race))
		{
			act("$N tells you 'Your race can't learn that skill.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}

		if(IS_SET(skill_table[sn].flags,SKFLAGS_NO_GAIN)){
			act("$N tells you 'That skill can't be gained sorry.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		if (ch->pcdata->learned[sn])
		{
			act("$N tells you 'You already know that skill!'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		if (skill_table[sn].rating[ch->clss] <= 0
			|| skill_table[sn].skill_level[ch->clss] >= LEVEL_IMMORTAL
			|| skill_table[sn].skill_level[ch->clss]<=0)
		{
			act("$N tells you 'That skill is beyond your powers.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		if (ch->train < skill_table[sn].rating[ch->clss]*1)
		{
			act("$N tells you 'You are not yet ready for that skill.'",
				ch,NULL,trainer,TO_CHAR);
			return;
		}
		
		// add the skill 
		ch->pcdata->learned[sn] = 1;
		act("$N trains you in the art of $t",
			ch,skill_table[sn].name,trainer,TO_CHAR);
		ch->train -= skill_table[sn].rating[ch->clss]*1;
		set_char_magic_bits(ch); // fix up their magic bits
		return;
	}
	
	act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);
}

/**************************************************************************/
void do_skills(char_data *ch, char *argument)
{
	char *skill_list[MAX_LEVEL+2];
    unsigned char skill_columns[MAX_LEVEL+2];
	char linebuf[MSL];
    int sn,lev;
    char arg[MIL];
    char_data *victim;
    char buf[MSL];
    BUFFER *output;
	int top_level=0;
    bool found = false;
	
	if (IS_UNSWITCHED_MOB(ch)){
		return;
	}
	
    one_argument( argument, arg );
	
    if (IS_IMMORTAL(ch) && !IS_NULLSTR(arg))
    {
		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{		
			ch->println( "They aren't here." );
			return;
		}
    }
	else
	{
		victim=ch;
	}
	
    if (IS_NPC(victim))
	{
		ch->println( "Mobs don't have skill percentages as such." );
		return;
	}
	
	
    output = new_buf();
	{
		if(ch->desc && ch->desc->connected_state!=CON_PLAYING){
			if(ch==victim){
				sprintf(buf,"`=j%s", creation_titlebar("SKILLS"));
			}else{
				sprintf(buf,"`=j%s", creation_titlebar("SKILLS FOR %s", uppercase(victim->name)));
			}
		}else{
			if(ch==victim){
				sprintf(buf,"`=t%s", format_titlebar("SKILLS"));
			}else{
				sprintf(buf,"`=t%s", format_titlebarf("SKILLS FOR %s", uppercase(victim->name)));
			}
		}

		buf[str_len(buf)-2]= '\0';
		strcat(buf,"`x");
	}
	add_buf( output, buf);

	if (ch!=victim)
	{
		sprintf(buf,"\r\nSkills for %s (Level: %d, Race: %s, Class: %s)",
			victim->name, 
			victim->level, 
			race_table[victim->race]->name, 
			class_table[victim->clss].name);
		add_buf( output, buf);
	}
	
    // initialize data
    for (lev = 0; lev < MAX_LEVEL+2; lev++){
        skill_columns[lev] = 0;
        skill_list[lev]= str_dup("");
    }
	
	if (IS_IMMORTAL(ch)){
		top_level= ABSOLUTE_MAX_LEVEL;
	}else{
		top_level= LEVEL_HERO;
	}
	
	bool specially_granted_skills=false;
    for (sn = 0; !IS_NULLSTR(skill_table[sn].name) ; sn++)
    {
		if (skill_table[sn].spell_fun == spell_null 
			&&	(
					(victim->pcdata->learned[sn]==101)
				  || 
					(skill_table[sn].skill_level[victim->clss]<=top_level
					&& skill_table[sn].skill_level[victim->clss]>0
					&& victim->pcdata->learned[sn] > 0)
				)
			)
		{
			if(victim->pcdata->learned[sn]==101 
				&& skill_table[sn].skill_level[victim->clss]>LEVEL_HERO)
			{
				specially_granted_skills=true;
			}
			
			found = true;
			lev = skill_table[sn].skill_level[victim->clss];
			if(IS_SET(skill_table[sn].flags,SKFLAGS_NEW_IMPROVE_SYSTEM)){
				if (victim->level < lev){
					sprintf(buf,"%-18s  n/a                  ", capitalize(skill_table[sn].name));
				}else{
					sprintf(buf,"%-18s %3d%%(%3d%%,%3d%%)       ",capitalize(skill_table[sn].name),
						victim->pcdata->learned[sn], 
						skill_table[sn].learn_scale_percent[victim->clss],
						(skill_table[sn].learn_scale_percent[victim->clss]==0?
								100:skill_table[sn].learn_scale_percent[ch->clss]));
				}
			}else{
				if (victim->level < lev && victim->pcdata->learned[sn]!=101){
					if(ch->level<lev){
						sprintf(buf,"%-18s  n/a                  ", capitalize(skill_table[sn].name));
					}else{
						sprintf(buf,"%-18s  n/a(%3d%%)            ", 
							capitalize(skill_table[sn].name),
							victim->pcdata->learned[sn]);
					}
				}else{
					sprintf(buf,"%-18s %3d%%                  ",capitalize(skill_table[sn].name),
						victim->pcdata->learned[sn]);
				}
			}
			
			if (IS_NULLSTR(skill_list[lev])){
				sprintf(linebuf,"\r\n Level %2d: %s",lev,buf);
				replace_string(skill_list[lev], linebuf);				
			}else{ // append
				if ( ++skill_columns[lev] % 2 == 0){
					sprintf(linebuf,"%s\r\n           ",skill_list[lev]);
					replace_string(skill_list[lev], linebuf);
				}else{
					// trim off the extra spaces
					strcpy(linebuf, skill_list[lev]);
					linebuf[str_len(linebuf)-6]='\0';
					replace_string(skill_list[lev], linebuf);
				}
				sprintf(linebuf,"%s%s",skill_list[lev],buf);
				replace_string(skill_list[lev], linebuf);
			}
		}
    }

	if (IS_IMMORTAL(ch) || specially_granted_skills){
		top_level= ABSOLUTE_MAX_LEVEL;
	}else{
		top_level= LEVEL_HERO;
	}
	
	
    // return results
    if (!found){
		if (ch==victim){
			ch->println( "You know no skills." );
		}else{
			ch->println( "They know no skills." );
		}
    }else{
		for (lev = 1; lev < top_level; lev++){
			if(lev==LEVEL_IMMORTAL){
				if(IS_IMMORTAL(ch)){
					sprintf(buf, "\r\n\r\n%s", format_titlebar("SKILLS DISABLED FOR CHARACTER UNLESS GRANTED BELOW HERE"));
				}else{
					sprintf(buf, "\r\n\r\n%s", format_titlebar("SPECIALLY GRANTED SKILLS"));
				}
				buf[str_len(buf)-2]= '\0';
				strcat(buf,"`x");				
				add_buf( output, buf);
			}
			if (!IS_NULLSTR(skill_list[lev])){
				add_buf( output, skill_list[lev] );
			}
		}
		add_buf( output, "\r\n");
		ch->sendpage(buf_string(output));
		free_buf(output);
	}

	// deallocate all memory used
	for (lev = 0; lev < MAX_LEVEL+2; lev++){
        free_string(skill_list[lev]);
    }
	return;
}

/****************************************************************************/
// Kal - Dec 00
void do_cskinfo(char_data *ch, char * argument)
{
	bool found = false;
	char buf[MSL], clss_name[MSL];
	int sn, count, lev;
	BUFFER *output;

	int clss_no;

	if (IS_NULLSTR(argument))
	{
		ch->wrapln( "CSKINFO - Class Skill Info - at what levels "
			"a particular class gets a skill and for how many trains.");
		ch->println( "Syntax: CSKINFO <class>" );
		return;
	}

	argument = one_argument (argument, clss_name);

	for (clss_no = 0; !IS_NULLSTR(class_table[clss_no].name); clss_no++)
	{
		if (!str_cmp(clss_name, class_table[clss_no].short_name) ||
            !str_prefix(clss_name,class_table[clss_no].name))
		{
			found = true;
			break;
		}
	}

	if ( !found )
	{
		ch->printlnf( "No class named '%s' exists. Use the 3-letter WHO names (Mag, Spf etc.)", clss_name);
		do_cskinfo(ch, "");
		return;
	}

	// setup a buffer for info to be displayed
	output	= new_buf();
	count	= 0;

	sprintf(buf,"Class Skill Info - all skills for `B%ss`x\r\n", class_table[clss_no].name);
	add_buf( output, buf);
	add_buf( output, "==Skill==========Level [Trains]==========");
	add_buf( output, "==Skill==========Level [Trains]==\r\n");

	for (lev=1; lev<=LEVEL_HERO; lev++)
	{      
		for (sn = 0; !IS_NULLSTR(skill_table[sn].name) ; sn++)
		{
			if ( skill_table[sn].skill_level[clss_no]==lev
				&& skill_table[sn].spell_fun == spell_null )
			{
				count++;
				sprintf(buf, "  %-20.20s %3d [%2d]", capitalize(skill_table[sn].name), 
					lev, skill_table[sn].rating[clss_no]);
				add_buf( output, buf);
				if(count%2==0){
					add_buf( output, "\r\n"); 
				}else{
					add_buf( output, "          "); 
				}
			}
		}
	}	
	sprintf(buf, "%s  %d skill%s total.", 
		count%2==1?"\r\n":"",
		count, 
		count==1?"":"s");
	add_buf( output, buf); 

	ch->sendpage(buf_string(output));
	free_buf(output);
	return;
}
/**************************************************************************/
// shows skills, groups and costs (only if not bought) 
void list_group_costs(char_data *ch)
{
    if (IS_NPC(ch))
        return;
	
    int gn,sn,col;
	
    col = 0;
    ch->print(" `g===`MSkillgroup`g==================`xCP`g====");
    ch->println("====`MSkillgroup`g==================`xCP`g====");

    for (gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++)
    {
        if (!ch->gen_data->skillgroup_chosen[gn] 
			&&  !ch->pcdata->skillgroup_known[gn]
			&&  IS_SKILLGROUP_SELECTABLE_FOR_CHAR(gn, ch))
        {
			ch->printf("   `S[`M%-28.28s`x%2d`S]`x   ",
				skillgroup_table[gn].name, 
				skillgroup_table[gn].rating[ch->clss]);
            if (++col % 2 == 0){
				ch->println("");
			}
        }
    }
    if ( col % 2 != 0 ){
        ch->println( "" );
	}
	ch->println( "" );
	
    col = 0;
	
    ch->print(" `g=`YSkill`g===========`xCP`g`BLevel`g=");
    ch->print("`g=`YSkill`g===========`xCP`g`BLevel`g=");
    ch->println("`g=`YSkill`g===========`xCP`g`BLevel`g=");
	
    for (sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++)
	{
		
		// race restrictions
		// race restrictions
		if( IS_SET(skill_table[sn].flags,SKFLAGS_USE_RACE_RESTRICTIONS)
				&& !IS_SETn(skill_table[sn].race_restrict_n, ch->race))
		{
			continue;
		}
		
		// no_gain
		if(IS_SET(skill_table[sn].flags,SKFLAGS_NO_GAIN))
		{
			continue;
		}

        if (!ch->gen_data->skill_chosen[sn] 
			&&  ch->pcdata->learned[sn] == 0
			&&  skill_table[sn].spell_fun == spell_null
			&&  skill_table[sn].rating[ch->clss] > 0
			&&  skill_table[sn].skill_level[ch->clss] < LEVEL_IMMORTAL
			&&  skill_table[sn].skill_level[ch->clss]>0)
        {
            ch->printf(" `S[`Y%-16.16s`x%2d`B<%2d>`S]`x",
				skill_table[sn].name,  
                skill_table[sn].rating[ch->clss],
                skill_table[sn].skill_level[ch->clss]);
            if (++col % 3 == 0)
                ch->print( "\r\n" );
        }
    }
    if ( col % 3 != 0 )
        ch->print( "\r\n" );

    ch->println( "`x\r\n" );
	
    ch->printlnf( "Creation points (CP): %3d    Experience per level (XP): %d",
		ch->gen_data->points_chosen, exp_per_level(ch,ch->gen_data->points_chosen));
	
    return;
	
}
/**************************************************************************/
void list_group_chosen(char_data *ch)
{
    int gn,sn,col;
	
    col = 0;
    ch->print(" `g===`MSkillgroup`g==================`xCP`g====");
    ch->println("====`MSkillgroup`g==================`xCP`g====");

    for (gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++)
    {
        if (ch->gen_data->skillgroup_chosen[gn])
        {
			ch->printf("   `S[`M%-28.28s`x%2d`S]`x   ",
				skillgroup_table[gn].name, 
				skillgroup_table[gn].rating[ch->clss]);
            if (++col % 2 == 0){
				ch->println("");
			}
        }
    }
    if ( col % 2 != 0 ){
        ch->println( "" );
	}
	ch->println( "" );

	
	col=0;
    ch->print(" `g=`YSkill`g===========`xCP`g`BLevel`g=");
    ch->print("`g=`YSkill`g===========`xCP`g`BLevel`g=");
    ch->println("`g=`YSkill`g===========`xCP`g`BLevel`g=");
	
    for (sn = 0; !IS_NULLSTR(skill_table[sn].name); sn++)
    {
        if (ch->gen_data->skill_chosen[sn] 
			&&  skill_table[sn].rating[ch->clss] > 0)
        {
            ch->printf(" `S[`Y%-16.16s`x%2d`B<%2d>`S]`x",
				skill_table[sn].name,  
                skill_table[sn].rating[ch->clss],
                skill_table[sn].skill_level[ch->clss]);
            if (++col % 3 == 0)
                ch->print( "\r\n" );
        }
    }
    if ( col % 3 != 0 )
        ch->print( "" );

    ch->println( "`x\r\n" );
	
    ch->printlnf( "Creation points (CP): %3d    Experience per level (XP): %d",
		ch->gen_data->points_chosen, exp_per_level(ch,ch->gen_data->points_chosen));
    return;
}
/**************************************************************************/
int exp_per_level(char_data *ch, int points)
{
    int expl,inc;
	
    if (IS_NPC(ch)){
		return 1000;
	}
	
    expl = 1000;
    inc = 500;
	
	
	if(race_table[ch->race]->class_exp[ch->clss]<1000){ // safety check
		ch->println("BUG: exp_per_level for your race class combination is below 1000... please report this to the immortals!");
		return 5000; 
	}
	
    if (points < 40){
        return race_table[ch->race]->class_exp[ch->clss];
	}
	
    // processing 
    points -= 40;
	
    while (points > 9)
    {
		expl += inc;
		points -= 10;
		if (points > 9)
		{
			expl += inc;
			inc *= 2;
			points -= 10;
		}
    }
    expl += points * inc / 10;  
	
    return UMAX(1000, expl * race_table[ch->race]->class_exp[ch->clss]/1000);
}
/**************************************************************************/
// this procedure handles the input parsing for the skill generator
bool parse_gen_groups(char_data *ch,char *argument)
{
    char arg[MIL];
    int gn,sn,i;
	
    if (IS_NULLSTR(argument)){
		return false;
	}
	
    argument = one_argument(argument,arg);
	
    if (!str_prefix(arg,"help"))
    {
		if (argument[0] == '\0')
		{
			do_help(ch,"group help");
			return true;
		}
		
        do_help(ch,argument);
		return true;
    }
	
    if (!str_prefix(arg,"add"))
    {
		if (IS_NULLSTR(argument))
		{
			ch->println( "You must provide a skill name." );
			return true;
		}
		
		// can't take all the skills to over load the size on an int
		if (ch->pcdata->points>150)
		{
			ch->println( "There is a limit of 150 creation points..." );
			ch->println( "you must remove other skills/groups before adding any more." );
			return true;
		}
		
		gn = skillgroup_lookup(argument);
		if (gn != -1)
		{
			if (ch->gen_data->skillgroup_chosen[gn]
				||  ch->pcdata->skillgroup_known[gn])
			{
				ch->printlnf("You already the skillgroup '%s'.",  
					skillgroup_table[gn].name);
				return true;
			}
			
			if (  !IS_SKILLGROUP_SELECTABLE_FOR_CHAR(gn, ch))
			{
				ch->printlnf( "The skillgroup '%s' is not available to you.", 
					skillgroup_table[gn].name );
				return true;
			}

			// can't take all the skills to over load the size on an int
			if (ch->pcdata->points + skillgroup_table[gn].rating[ch->clss]>150)
			{
				ch->println( "There is a limit of 150 creation points..." );
				ch->println( "you must remove other skills/groups before adding this group." );
				return true;
			}
			
			ch->printlnf( "%s skillgroup added.", skillgroup_table[gn].name);
			
			ch->gen_data->skillgroup_chosen[gn] = true;
			ch->gen_data->points_chosen += skillgroup_table[gn].rating[ch->clss];
			gn_add(ch,gn);
			ch->pcdata->points += skillgroup_table[gn].rating[ch->clss];
			return true;
		}
		
		sn = skill_lookup(argument);
		if (sn != -1)
		{
			if (ch->gen_data->skill_chosen[sn]
				||  ch->pcdata->learned[sn] > 0)
			{
				ch->println( "You already know that skill!" );
				return true;
			}
			
			if ( !IS_SKILL_VALID_FOR_CLASS(sn, ch->clss)
				||  IS_SPELL(sn))
			{
				ch->println( "That skill is not available." );
				return true;
			}
			
			// race restrictions
			if( IS_SET(skill_table[sn].flags,SKFLAGS_USE_RACE_RESTRICTIONS)
				&& !IS_SETn(skill_table[sn].race_restrict_n, ch->race))
			{
				ch->println( "That skill is not available to your race." );
				return true;
			}

			if(IS_SET(skill_table[sn].flags,SKFLAGS_NO_GAIN)){
				ch->println( "That skill is not available." );
				return true;
			}

			// can't take all the skills to over load the size on an int
			if (ch->pcdata->points + skillgroup_table[gn].rating[ch->clss]>150)
			{
				ch->println( "There is a limit of 150 creation points..." );
				ch->println( "you must remove other skills/groups before adding this skill." );
				return true;
			}
			
			ch->printlnf( "%s skill added.", skill_table[sn].name);
			ch->gen_data->skill_chosen[sn] = true;
			ch->gen_data->points_chosen += skill_table[sn].rating[ch->clss];
			ch->pcdata->learned[sn] = 1;
			ch->pcdata->points += skill_table[sn].rating[ch->clss];
			return true;
		}
		
		ch->println( "No skills or groups by that name..." );
		return true;
    }
	
    if (!strcmp(arg,"drop"))
    {
		if (argument[0] == '\0')
		{
			ch->println( "You must provide a skill to drop." );
			return true;
		}
		
		gn = skillgroup_lookup(argument);
		if (gn != -1 && ch->gen_data->skillgroup_chosen[gn])
		{
			ch->println( "Skillgroup dropped." );
			ch->gen_data->skillgroup_chosen[gn] = false;
			ch->gen_data->points_chosen -= skillgroup_table[gn].rating[ch->clss];
			gn_remove(ch,gn);
			// readd all the groups to make sure we don't lose any skills
			for (i = 0; !IS_NULLSTR(skillgroup_table[i].name); i++)
			{
				if (ch->gen_data->skillgroup_chosen[gn]){
					gn_add(ch,gn);
				}
			}
			ch->pcdata->points -= skillgroup_table[gn].rating[ch->clss];
			return true;
		}
		
		sn = skill_lookup(argument);
		if (sn != -1 && ch->gen_data->skill_chosen[sn])
		{
			ch->println( "Skill dropped." );
			ch->gen_data->skill_chosen[sn] = false;
			ch->gen_data->points_chosen -= skill_table[sn].rating[ch->clss];
			ch->pcdata->learned[sn] = 0;
			ch->pcdata->points -= skill_table[sn].rating[ch->clss];
			return true;
		}
		
		ch->println( "You haven't bought any such skill or group." );
		return true;
    }
	
    if (!str_prefix(arg,"premise") || !str_prefix(arg,"explain"))
    {
		do_help(ch,"premise");
		return true;
    }
	
    if (!str_prefix(arg,"list"))
    {
		ch->titlebar("LIST OF SKILLS AND SKILLSGROUPS STILL AVAILABLE");
        list_group_costs(ch);
        return true;
    }
	
    if (!str_prefix(arg,"learned"))
    {
		ch->titlebar("LEARNED SKILLS AND SKILLGROUPS");
        list_group_chosen(ch);
        ch->println( "You already have the following skills:" );
        ch->println( "You have these skills as a mixture, of the default skills you get" );
        ch->println( "with your race and class, plus the others you have added." );
		do_skills(ch,"");
        return true;
    }
	
    if (!str_prefix(arg,"skillgroups") || !str_prefix(arg,"info"))
    {
		do_skillgroups(ch,argument);
		return true;
    }
	
    if (!str_prefix(arg,"spinfo"))
    {
		do_spinfo(ch,argument);
        return true; 
    }	
	
    if (!str_prefix(arg,"cspinfo"))
    {
		do_cspinfo(ch,argument);
        return true; 
    }

    if (!str_prefix(arg,"cskinfo"))
    {
		do_cskinfo(ch,argument);
        return true; 
    }

    return false;
}
/**************************************************************************/
// shows all skillgroups, or the sub-members of a skillgroup
void do_skillgroups(char_data *ch, char *argument)
{
    int gn,sn,col;
	
    if (IS_NPC(ch))
		return;
	
    col = 0;

    if (argument[0] == '\0')
    {   
		ch->titlebar("SKILLGROUPS YOU HAVE");
		for (gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++)
		{
			if (ch->pcdata->skillgroup_known[gn])
			{
				ch->printf( "%-25s ",skillgroup_table[gn].name);

				if (++col % 3 == 0)
					ch->print( "\r\n" );
			}
        }
        if ( col % 3 != 0 )
            ch->print( "\r\n" );
		ch->titlebar("");
		ch->println("To list all skillgroups type `=Cskillgroups all`x.");
		ch->println("To show the skills in a particular skillgroup type `=Cskillgroups <skgrp name>`x.");
		ch->titlebar("");
		return;
	}
	
	if (!str_cmp(argument,"all"))    // show all skillgroups
	{
		ch->titlebar("ALL SKILLGROUPS");
	    for (gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++)
	    {
			if(!IS_SET(skillgroup_table[gn].flags, SKILLGROUP_CREATION_SELECTABLE)){
				continue;
			}
			if(skillgroup_table[gn].remort){
				ch->printf( "`S%d`x%-24s ", skillgroup_table[gn].remort, skillgroup_table[gn].name );
			}else{
				ch->printf( "%-25s ", skillgroup_table[gn].name );
			}
			if (++col % 3 == 0)
				ch->print( "\r\n" );
        }
        if ( col % 3 != 0 ){
            ch->print( "\r\n" );
		}
		ch->titlebar("");
		return;
	}
	
	
	// show the sub-members of a group 
	gn = skillgroup_lookup(argument);
	if (gn == -1)
	{
		ch->printlnf( "No skillgroup of the name '%s' exist.", argument );
		ch->println( "Type 'skillgroups all' for a full listing." );
		return;
	}
	
	ch->titlebarf("SKILLS WITHIN THE SKILLGROUP '%s'", uppercase(skillgroup_table[gn].name));
	for (sn = 0; skillgroup_table[gn].skills[sn]>-1; sn++)
	{
		ch->printf( "%-25s ", skill_table[skillgroup_table[gn].skills[sn]].name);

		if (++col % 3 == 0)
			ch->print( "\r\n" );
	}
    if ( col % 3 != 0 )
        ch->print( "\r\n" );
	ch->titlebar("");
}

/**************************************************************************/
// new check improve system - Kal
// trial code, not really completed yet.
void new_check_improve( char_data *ch, int sn, bool success, int multiplier)
{
	// standard situations where you can't improve
    if (IS_NPC(ch) || sn==-1 || IS_OOC(ch) || multiplier<1) {
		logf("1start %d\r\n",multiplier);
		return;
	}

	// can't improve by spamming
#ifndef WIN32
	if (ch->desc && ch->desc->repeat>5){
		return;
	}
#endif

	// store the perthousand improve table
	static short improve_chance_lookup_table[102]; // stored in range 1 ->10000
	static bool initialise_table=true;
	if(initialise_table){ // calculate the improvement chances once, first time
		int i;
		double f;
		for(i=0; i<=100; i++){
//			f=sqrt((100.0-i)/100);
			f=(100.0-i)/100;
			f=pow(100.0,f);
			f*=100; // put in 1->10000 range - increased precision
			improve_chance_lookup_table[i]=(short)f;
			//logf("improve_chance_lookup_table[%d]=%d", i, improve_chance_lookup_table[i]);
		}
		improve_chance_lookup_table[101]=1;
		initialise_table=false;
	}

	int maxlearn=skill_table[sn].learn_scale_percent[ch->clss];
	if(maxlearn<1){
		maxlearn=65;
	}
	
    if ( (!IS_SPELL(sn) && ch->level < skill_table[sn].skill_level[ch->clss])
		||  skill_table[sn].skill_level[ch->clss]==0
		||  skill_table[sn].rating[ch->clss] == 0
		||  ch->pcdata->learned[sn] <2 // 2% required - must prac at least once to improve
		||  ch->pcdata->learned[sn] >=maxlearn)
	{
		return;  // skill is not known, or past the point of improvement
	}

	if (number_range(1,4)!=1){ // make things harder
		new_check_improve( ch, sn, success, multiplier-1);
		return;
	}
	
    // check to see if the character has a chance to learn 
    int chance= 8 * ch->modifiers[STAT_ME] + 2*ch->modifiers[STAT_IN];
    chance /= (		multiplier
		*	skill_table[sn].rating[ch->clss] 
		*	4);
    chance += ch->level*3/4;
	
    if (number_range(1,1000) > chance){
		// try again
		new_check_improve( ch, sn, success, multiplier-1);
		return;
	}
	
    // now that the character has a CHANCE to learn, see if they really have

	chance=improve_chance_lookup_table[ch->pcdata->learned[sn]];
	int max_percent=UMAX(maxlearn,
						skill_table[sn].maxprac_percent[ch->clss]);
	if(max_percent>0 && max_percent<100){
		chance=chance* max_percent/100;
		if(chance<1){
			chance=1;
		}
	}
	
	if (!success){
		chance/=2;
		chance = URANGE(1,chance,2500);
	}
	if (number_range(1,10000)> chance){
		// try again
		new_check_improve( ch, sn, success, multiplier-1);
		return;
	}

	if (success){
		ch->printlnf("*You have become better at `Y%s`x!",
			skill_table[sn].name);
		ch->pcdata->learned[sn]++;
    }else{
		ch->printlnf("*You learn from your mistakes, and your `Y%s`x skill improves.",
				skill_table[sn].name);
		ch->pcdata->learned[sn] += number_range(1,3);
		ch->pcdata->learned[sn] = (unsigned char)UMIN(ch->pcdata->learned[sn],skill_table[sn].learn_scale_percent[ch->clss]);
    }
	ch->printlnf("*Your `Y%s`x is now at %d%%!!",
			skill_table[sn].name, ch->pcdata->learned[sn]);

	// calc how far they can prac it
	int maxprac=skill_table[sn].maxprac_percent[ch->clss]?
					skill_table[sn].maxprac_percent[ch->clss]:
					50;
	
	if(!(ch->level>70 && ch->pcdata->learned[sn]<maxprac)){
		gain_exp(ch,2 * skill_table[sn].rating[ch->clss]);
	}
}
/**************************************************************************/
// checks for skill improvement 
void check_improve( char_data *ch, int sn, bool success, int multiplier )
{
	if(IS_SET(skill_table[sn].flags,SKFLAGS_NEW_IMPROVE_SYSTEM)
		|| HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER))
	{
		new_check_improve(ch, sn, success, multiplier);
		return;
	}

    if (IS_NPC(ch))
		return;

	if (sn==-1){
		return;
	}

	// can't improve in ooc rooms
    if (IS_OOC(ch)) 
		return;
	
	// can't improve by spamming
	if (ch->desc && ch->desc->repeat>5)
		return;
	
    if (  (!IS_SPELL(sn) && ch->level < skill_table[sn].skill_level[ch->clss])
		|| skill_table[sn].skill_level[ch->clss]==0
		||  skill_table[sn].rating[ch->clss] == 0
		||  ch->pcdata->learned[sn] <1
		||  ch->pcdata->learned[sn] >99)
		return;  // skill is not known

    // check to see if the character has a chance to learn 
    int chance= 8 * ch->modifiers[STAT_ME] + 2*ch->modifiers[STAT_IN];
    chance /= (		multiplier
		*	skill_table[sn].rating[ch->clss] 
		*	4);
    chance += ch->level;
	
    if (number_range(1,1000) > chance)
		return;
	
    // now that the character has a CHANCE to learn, see if they really have
	if (success){
		chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
		if (number_percent() < chance)
		{
			ch->printlnf("You have become better at `Y%s`x!", skill_table[sn].name);
			ch->pcdata->learned[sn]++;
			if(!(ch->level>75 && ch->pcdata->learned[sn]<75)){
				gain_exp(ch,2 * skill_table[sn].rating[ch->clss]);
			}
		}
    }else{
		chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
		if (number_percent() < chance)
		{
			ch->printf(
				"You learn from your mistakes, and your `Y%s`x skill improves.\r\n",
				skill_table[sn].name);
			ch->pcdata->learned[sn] += number_range(1,3);
			ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
			if(!(ch->level>70 && ch->pcdata->learned[sn]<75)){
				gain_exp(ch,2 * skill_table[sn].rating[ch->clss]);
			}
		}
    }
}
/**************************************************************************/
// returns a skillgroup index number given the exact name
int skillgroup_exact_lookup( const char *name )
{
    for( int gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++ )
    {
        if ( !str_cmp( name, skillgroup_table[gn].name ) ){
            return gn;
		}
    }
	return -1;
}
/**************************************************************************/
// returns a skillgroup index number given the name
int skillgroup_lookup( const char *name )
{
    int gn;

	// perform exact match first
	gn=skillgroup_exact_lookup(name);
	if(gn>-1){
		return gn;
	}
	
	// perform prefix match
    for( gn = 0; !IS_NULLSTR(skillgroup_table[gn].name); gn++ )
    {
        if ( LOWER(name[0]) == LOWER(skillgroup_table[gn].name[0])
			&& !str_prefix( name, skillgroup_table[gn].name ) )
            return gn;
    }
	
    return -1;
}
/**************************************************************************/
// recursively adds a group given its number -- uses group_add
void gn_add( char_data *ch, int gn)
{
    int i;
	if(IS_NPC(ch)){
		return;
	}

	if(!IS_SKILLGROUP_SELECTABLE_FOR_CHAR(gn, ch)){
		return;
	}
    
    ch->pcdata->skillgroup_known[gn] = true;

	for (i = 0; skillgroup_table[gn].skills[i]>-1; i++)
	{
		if(IS_SKILL_VALID_FOR_CLASS(skillgroup_table[gn].skills[i], ch->clss)){
			group_add(ch,skill_table[skillgroup_table[gn].skills[i]].name,false, skillgroup_table[gn].skillsvalue[i]);
		}
    }
}
/**************************************************************************/
// recursively removes a group given its number -- uses group_remove 
void gn_remove( char_data *ch, int gn)
{
    int i;
	
    ch->pcdata->skillgroup_known[gn] = false;
	
	for (i = 0; skillgroup_table[gn].skills[i]>-1; i++)
	{
		if(IS_SKILL_VALID_FOR_CLASS(skillgroup_table[gn].skills[i], ch->clss)){
			group_remove(ch,skill_table[skillgroup_table[gn].skills[i]].name);
		}
    }
}
/**************************************************************************/	
// use for processing a skill or group for addition 
void group_add( char_data *ch, const char *name, bool deduct, int percent)
{
    int sn,gn;
	
    if (IS_NPC(ch)){ // NPCs do not have skills
		return;
	}
	
	if(percent<0 || percent>101){ // safety check
		return;
	}

    sn = skill_exact_lookup(name);
	
    if (sn != -1)
    {
		if (ch->pcdata->learned[sn] < percent){
			ch->pcdata->learned[sn] = percent;
			if (deduct){
				ch->pcdata->points += skill_table[sn].rating[ch->clss]; 
			}
		}
		return;
    }
	
	// if there wasn't a skill by the name, check for a group
	gn = skillgroup_lookup(name);
	
    if (gn != -1)
    {
		if (ch->pcdata->skillgroup_known[gn] == false)  
		{
			ch->pcdata->skillgroup_known[gn] = true;
			if (deduct)
				ch->pcdata->points += skillgroup_table[gn].rating[ch->clss];
		}
		gn_add(ch,gn); // make sure all skills in the group are known
    }
}
/**************************************************************************/
// used for processing a skill or group for deletion -- no points back!
void group_remove(char_data *ch, const char *name)
{
    int sn, gn;
    
	sn = skill_lookup(name);
	
    if (sn != -1)
    {
		ch->pcdata->learned[sn] = 0;
		return;
    }
	
    // now check skillgroups
	
    gn = skillgroup_lookup(name);
	
    if (gn != -1 && ch->pcdata->skillgroup_known[gn] == true)
    {
		ch->pcdata->skillgroup_known[gn] = false;
		gn_remove(ch,gn);  // be sure to call gn_add on all remaining skillgroups
    }
}

/**************************************************************************/
// Jarren - June 1999
void do_vault( char_data *ch, char *argument )
{
	char arg[MIL];
	char_data *victim;
	int chance, accuracy;
	
	one_argument(argument,arg);
	
	if ( (chance = get_skill(ch,gsn_vault)) == 0
		||      (!IS_NPC(ch)
		&&       ch->level < skill_table[gsn_vault].skill_level[ch->clss]))
	{
		if (!IS_CONTROLLED(ch))
		{
			ch->println( "Vault? What's that?" );
			return;
		}
	}
	
	if (arg[0] == '\0')
	{
		victim = ch->fighting;
		if (!victim){
			ch->println( "But you aren't fighting anyone!" );
			return;
		}
	}else if ((victim = get_char_room(ch,arg)) == NULL){
		ch->println( "They aren't here." );
		return;
	}
	
	if (victim->position < POS_FIGHTING)
	{
		act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
		return;
	}
	
	if (victim == ch){
		ch->println( "You try to pole vault onto yourself but miss horribly." );
		return;
	}
	
	if (is_safe(ch,victim))
		return;

	if ( !can_initiate_combat( ch, victim, 1 )) return;	

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim){
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }
	
    // modifiers
	
    // size and weight
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;
	
    if (ch->size < victim->size){
		chance += (ch->size - victim->size) * 15;
    }else{
		chance += (ch->size - victim->size) * 10; 
	}
		
    // stats
    chance += ch->modifiers[STAT_ST];
    chance -= victim->modifiers[STAT_QU];
    chance -= GET_AC(victim,AC_BASH) /25;
    
	// speed
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE)){
		chance += 10;
    }
	if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE)){
		chance -= 30;
	}
	
    // level
    chance += (ch->level - victim->level);
	
    if (IS_CONTROLLED(ch)){
        chance*=3;
    };
	
    if (!IS_NPC(victim) && chance < get_skill(victim,gsn_dodge) )
		   chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
	
    // now the attack
    if (number_percent() < chance )
    {

		accuracy = 100 - get_skill(victim,gsn_vault);
		accuracy -= ch->modifiers[STAT_PR]/10;
		accuracy -= ch->modifiers[STAT_AG]/10;
		accuracy += victim->modifiers[STAT_QU]/9;
		if(ch->pcdata){
			accuracy += ch->pcdata->tired/2;
		}

		if(accuracy < number_range(1,8))
		{
			act("$n pole vaults at you delivering a powerfull blow to your head!",ch,NULL,NULL,TO_VICT);
			act("You pole vault at $n delivering a powerfull blow to $m head!",victim,NULL,NULL,TO_CHAR);
			act("$n pole vaults at $N delivering a powerfull blow to $M head.",ch,NULL,victim,TO_NOTVICT);
			accuracy = 2;
		}else{
			act("$n pole vauls at you delivering a crushing blow to your abdomen!",ch,NULL,NULL,TO_VICT);
			act("You pole vault at $n delivering a crushing blow to $m abdomen!",victim,NULL,NULL,TO_CHAR);
			act("$n pole vaults at $N delivering a crushing blow to $M abdomen.",ch,NULL,victim,TO_NOTVICT);
			accuracy = 1;
		}
		
		check_improve(ch,gsn_vault,true,1);
		
		DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
		WAIT_STATE(ch,skill_table[gsn_vault].beats);
		victim->position = POS_RESTING;
		damage(ch,victim,number_range(2,2 + accuracy * ch->size + chance/10),
			gsn_vault,	DAM_BASH,false);
		
    }else{
		damage(ch,victim,0,gsn_vault,DAM_BASH,false);
		act("You try to pole vault at $n but miss and fly right past $m!", victim,NULL,NULL,TO_CHAR);
		act("$n tries to pole vault at $N but flies right past $M.", ch,NULL,victim,TO_NOTVICT);
		act("You evade $n's pole vault, causing $m to fly right past you.", ch,NULL,NULL,TO_VICT);
		check_improve(ch,gsn_vault,false,1);
		WAIT_STATE(ch,skill_table[gsn_vault].beats * 3/2); 
    }
}
/**************************************************************************/
