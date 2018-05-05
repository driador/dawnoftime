/**************************************************************************/
// d2magsys.cpp - dawn magic system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "magic.h"
#include "d2magsys.h"
#include "float.h"

/****************************************************************************/
// Kal - July 99
void check_realm_improve(char_data * ch, int sn, bool success, int multiplier)
{
	char buf[MSL], word[MSL];
	char *argument;
	int i;
	const struct flag_type *table;
	int bits;
	int componentsn;

	for(i=0; i < MAX_REALMS; i++){
		switch (i){
		case 0:
			bits=skill_table[sn].realms;
			table=realm_flags;
			break;
		case 1:
			bits=skill_table[sn].spheres;
			table=sphere_flags;	
			break;
		case 2:
			bits=skill_table[sn].elements;
			table=element_flags;
			break;
		case 3:
			bits=skill_table[sn].compositions;
			table=composition_flags;
			break;
		default:
			ch->bug_printlnf("check_realm_improve(): Unknown i index %d for spell sn=%d!!!", i, sn);
			return;
		}

		// loop thru each type of magic system checking for improvements
		if(bits){
			strcpy(buf, flag_string( table, -1));
			argument=buf;
			for (; ;)
			{
				// using one_argument assumes all the components of a 
				// magic system are single words 
				//   e.g. fire, convocation, earth...
				argument = one_argument( argument, word );

				if ( word[0] == '\0' )
				break;

				// only improve in the realms for the spell in question
				if(IS_SET(bits, flag_value(table,word))){
					componentsn=skill_lookup(word);
					if(componentsn>-1){
						// check improve handles if they below 1% in it
						check_improve(ch,componentsn,success,multiplier);
					}
				}
			}
		}
	}
	return;
}

/****************************************************************************/
// Kal - May 99
void set_char_magic_bits(char_data * ch)
{
	char buf[MSL], word[MSL];
	char *argument=buf;
	
	if(IS_NPC(ch))
		return;

	ch->pcdata->realms=0;
	strcpy(buf, flag_string( realm_flags, -1));
    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
			break;

		if(get_skill(ch,skill_lookup(word))>0){
			ch->pcdata->realms|=flag_value(realm_flags,word);
		}
    }

	ch->pcdata->spheres=0;
	strcpy(buf, flag_string( sphere_flags, -1));
	argument=buf;
    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
			break;

		if(get_skill(ch,skill_lookup(word))>0){
			ch->pcdata->spheres|=flag_value(sphere_flags,word);
		}
    }

	ch->pcdata->elements=0;
	strcpy(buf, flag_string( element_flags, -1));
	argument=buf;
    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
			break;

		if(get_skill(ch,skill_lookup(word))>0){
			ch->pcdata->elements|=flag_value(element_flags,word);
		}
    }

	ch->pcdata->compositions=0;
	strcpy(buf, flag_string( composition_flags, -1));
	argument=buf;
    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
			break;

		if(get_skill(ch,skill_lookup(word))>0){
			ch->pcdata->compositions|=flag_value(composition_flags,word);
		}
    }
}

/****************************************************************************/
// Kal - May 99
bool has_spell_req(char_data * ch, int sn)
{ 
    if(IS_NPC(ch))
        return true;    
  
    // make sure they have ALL the required realms and spheres for the spell 
	switch(class_table[ch->clss].class_cast_type)
	{
		case CCT_MAXCAST:
		case CCT_NONE:
			{
				return false;
			}

		case CCT_MAGE:
			if(IS_ALL_SET(ch->pcdata->realms, skill_table[sn].realms)){
    			return true;
			}else{
	   			return false;
			}

		case CCT_CLERIC:
			if(IS_ALL_SET(ch->pcdata->spheres, skill_table[sn].spheres)){
    			return true;
			}else{
	   			return false;
			}

		case CCT_DRUID:
			if(IS_ALL_SET(ch->pcdata->elements, skill_table[sn].elements)){
    			return true;
			}else{
	   			return false;
			}
		case CCT_BARD:
			if(IS_ALL_SET(ch->pcdata->compositions, skill_table[sn].compositions)){
    			return true;
			}else{
				return false;
			}
	}

    return false;
}


/****************************************************************************/
int average_from_table( char_data * ch, int, 
					   const struct flag_type *flag_table, int bits)
{
	int flag, count=0, total=0;

	
    for (flag = 0; !IS_NULLSTR(flag_table[flag].name); flag++)
    {
		if(IS_SET(flag_table[flag].bit, bits)){
			total+=get_skill(ch, skill_lookup(flag_table[flag].name));
			count++;
		}
	}
	if(count==0){
		return 0;
	}
	return (total/count);
}
/****************************************************************************/
// returns the average skills % for the character in all spell realms
// schools, disciplines and spheres for a given spell
int get_avg(char_data * ch, int sn)
{
	if(IS_NPC(ch))
		return 100;

    // Get the average of all their realms/spheres or elements & seasons 
	switch(class_table[ch->clss].class_cast_type)
	{
		case CCT_MAXCAST:
		case CCT_NONE:{
			return 0;
		}

		case CCT_MAGE:
			return (average_from_table(ch, sn, realm_flags, skill_table[sn].realms)); 
		case CCT_CLERIC:
			return (average_from_table(ch, sn, sphere_flags, skill_table[sn].spheres)); 
		case CCT_DRUID:
			return (average_from_table(ch, sn, element_flags, skill_table[sn].elements)); 
		case CCT_BARD:
			return (average_from_table(ch, sn, composition_flags, skill_table[sn].compositions)); 
	}
		
	return 0;
}

/****************************************************************************
 *  calc_spell_percent - Kalahn - October 97                                *
 *  - returns a number between 1 and 10000                                  *
 *  - This number is a /10000 percent chance that a class at a certain      *
 *     level can get a spell (_based_ of a normal distribution curve)       *
 ****************************************************************************/
int calc_spell_percent(int clss, int level, int sn)
{
    float curve_chance;
    float std_div;
    float z, az;
    int iz, lpercent;

    if (skill_table[sn].low_percent_level[clss] >
            skill_table[sn].skill_level[clss])
    {
        int ti = skill_table[sn].low_percent_level[clss] -
            skill_table[sn].skill_level[clss];

        lpercent = skill_table[sn].skill_level[clss]-ti;
        if (lpercent == 0) lpercent = 1;
    }
    else
        lpercent = skill_table[sn].low_percent_level[clss];

    std_div = ((float)skill_table[sn].skill_level[clss]
        - (float)lpercent)/3;

    if (skill_table[sn].skill_level[clss]==
        lpercent)
    {
        bugf("calc_spell_percent - spell level is same as low_percent for %s - %s!!!",
            skill_table[sn].name, class_table[clss].name);
        std_div=1;        
    }

    if (std_div==0)
    {
        bugf("calc_spell_percent: had to set std_div to 1 (sn=%d, cl=%d, lvl=%d",
            sn, clss, level);       
        std_div=1;
    }

    z = (skill_table[sn].skill_level[clss] -level)/std_div;

    /* get abs value */
    az=z;
    if (az<0)
        az*=-1; 

    /* get integer version for the switch */
    iz= (int)az;

    switch (iz) /* generate our percentage curve */
    {
        case 0:
            curve_chance = (-3000 * az)+ 5000;
            break;
        case 1:
            curve_chance = (-1500 * az)+ 3500;
            break;
        case 2:
            curve_chance = (-400 * az)+ 1300;
            break;
        case 3:
            curve_chance = (-99 * az)+ 397;          
            break;
        default: curve_chance=1;
            break;
    }

    /* always have a chance to learn at least 1/10000 */
    if (curve_chance<1) 
        curve_chance=1;

    /* above the mean level */
    if (z<0)
        curve_chance=10000-curve_chance;

    return ( (int)curve_chance);
}
/****************************************************************************/
bool can_learn_spell(char_data * ch, int sn)
{
    if(IS_NPC(ch))
        return false; 

    // can't learn a spell you already have
    if(ch->pcdata->learned[sn]>0)
        return false;

    // immortal levels only spells 
    if ((skill_table[sn].skill_level[ch->clss]>=LEVEL_IMMORTAL
		 || skill_table[sn].skill_level[ch->clss]<=0)
            && ch->level<LEVEL_IMMORTAL)  
        return false;

    // level zero spells - imm only
    if ((skill_table[sn].skill_level[ch->clss]==0)
            && ch->level<LEVEL_IMMORTAL)  
        return false;

    // can't learn a spell with a zero rating
    if (skill_table[sn].rating[ch->clss]==0)
        return false;

    // can't learn a spell with the level the same as the low_percent_level
    if (skill_table[sn].skill_level[ch->clss]
			== skill_table[sn].low_percent_level[ch->clss])
        return false;


	return true;
}

/****************************************************************************/
bool check_learn_spell(char_data * ch, int sn, int one_out_of)
{
    int chance;
    int curve_chance;
	char wiznet_buf[MSL];

	if (!can_learn_spell(ch, sn))
		return false;

    // must have all realms
    if(!has_spell_req(ch,sn))
        return false;      

    chance=UMAX(get_avg(ch, sn)-10, 1);

    curve_chance = calc_spell_percent(ch->clss, ch->level, sn);
	
	// generate wiznet beta message
	 sprintf(wiznet_buf, "SPELL: %s 1in=%2d, av=%2d%%, curve=%2d.%02d%%, low%%=%2d - %s ",
	    ch->name,		
		one_out_of,
		chance,  // av
		curve_chance/100, curve_chance%100, // curve
        skill_table[sn].low_percent_level[ch->clss],		
		skill_table[sn].name
		);

	wiznet(wiznet_buf,NULL,NULL,WIZ_BETA,0,0);

    /*** do percentage checks ***/
    if (number_range(1,100) > chance)
        return false;

 
    if (number_range(1,10000)>curve_chance)
        return false;

	// debugging stuff
    if (IS_SET(ch->comm, COMM_SPELL_DEBUG))
    {
		logf("Spelldebug on %s [%d]", ch->name, ch->level);
        logf("%-30s av= %3d ", skill_table[sn].name, chance);
        logf("curve= %d.%d%% ", curve_chance/100, curve_chance%100);
        logf("lvl=%2d ", skill_table[sn].skill_level[ch->clss] );
        logf("low%%=%2d\r\n", skill_table[sn].low_percent_level[ch->clss]);
    }
	

    if (number_range(1,one_out_of)!=1)
        return false;

	wiznet("Learnt",NULL,NULL,WIZ_BETA,0,0);
    return true;
}


/****************************************************************************/
void on_level_learn(char_data * ch)
{
    int i;
	double lchance;

    if(IS_NPC(ch))
        return;
  
    set_char_magic_bits(ch); 

	if(ch->pcdata->realms==0 
		&& ch->pcdata->spheres==0
		&& ch->pcdata->elements==0
		&& ch->pcdata->compositions==0){
		return;
	}

    for(i=FIRST_SPELL; i<=LAST_SPELL; i++)
    {
		if(!IS_SET(skill_table[i].flags, SKFLAGS_LEVEL_SPGAIN))
			continue;

		// can't learn a spell you already have
		if(ch->pcdata->learned[i]>0){
			continue;
		}

		lchance = (skill_table[i].skill_level[ch->clss]+2)/3.0;
	
        // levels 1 thru 10 get a better chance of learning a spell
        if (ch->level<11){
            lchance/=2.0;
		};		

		// if the mortal is above the level of the spell, make it up to 50% easier
		if(GAMESETTINGMF1(GAMESETMF1_SPELL_ONLEVEL_LEARN_EASIER_IF_ABOVE_SPELL_LEVEL)){
			if(skill_table[i].skill_level[ch->clss]>0 && ch->level>skill_table[i].skill_level[ch->clss]){
				// we need to reduce the lchance by up to 50, depending on if we are up to double the level of the spell
				double level_ratio=ch->level/skill_table[i].skill_level[ch->clss];
				if(level_ratio>2.0){
					level_ratio=2.0;
				}
				// level_ratio should be between 1.0 and 2.0 by this point
				lchance/=level_ratio;			
			}
		}

		if(lchance<1.0){
			lchance=1.0;
		}

        if(check_learn_spell(ch,i,(int)lchance))
        {
            ch->pcdata->learned[i]=1;
            ch->printlnf("You have mastered the spell `Y%s`x.", skill_table[i].name);
            check_realm_improve(ch,i,true,25);  
        }
    }  
    return;
}

/****************************************************************************/
// returns a symbol system for which clsses can get a spell 
// written by Kalahn - Sep 97 
char *spell_class_code(int sn)
{
    int cla, cvalue;
    static char clss_code[MSL];

    clss_code[0]= '\0'; // clear the current return buffer 

    for (cla=0; !IS_NULLSTR(class_table[cla].name); cla++)
    {
        if (class_table[cla].spinfo_letter[0]!='-')
        {
            if (skill_table[sn].skill_level[cla]>0
				&& skill_table[sn].skill_level[cla]<=LEVEL_HERO )
            {
                // get colour code for the spell
                cvalue = ((skill_table[sn].skill_level[cla]-2)/6)+1;
                switch (cvalue)
                {
                    case 0: case 1:
                        strcat(clss_code, "`b");
                        break;
                    case 2:
                        strcat(clss_code, "`c");
                        break;
                    case 3:
                        strcat(clss_code, "`g");
                        break;
                    case 4:
                        strcat(clss_code, "`m");
                        break;
                    case 5:
                        strcat(clss_code, "`r");
                        break;
                    case 6:
                        strcat(clss_code, "`y");
                        break;
                    case 7:
                        strcat(clss_code, "`S");
                        break;
                    case 8:
                        strcat(clss_code, "`w");
                        break;
                    case 9:
                        strcat(clss_code, "`W");
                        break;
                    case 10:
                        strcat(clss_code, "`Y");
                        break;
                    case 11:
                        strcat(clss_code, "`R");
                        break;
                    case 12:
                        strcat(clss_code, "`M");
                        break;
                    case 13:
                        strcat(clss_code, "`G");
                        break;
                    case 14:
                        strcat(clss_code, "`C");
                        break;
                    case 15:
                        strcat(clss_code, "`B");
                        break;
                }
		        // get clss letter for spell
	            strcat(clss_code, class_table[cla].spinfo_letter);
            }
            else
            {
                strcat(clss_code, " ");
            }
        }

    }

    if (skill_table[sn].target==TAR_MOB_OFFENSIVE
        || skill_table[sn].target==TAR_OBJ_MOB_OFF)
        strcat(clss_code, "`x!");
    else
        strcat(clss_code, " `x");

    return (clss_code);
}
/****************************************************************************/
// Kal - Jan 2001
char * get_spinfo_data_requirements(char_data *ch, char *text, 
									char have_colour, char havenot_colour)
{
	static char result[MIL];
	char word[MIL], buf[MIL], workspace[MIL];
	workspace[0]='\0';
	workspace[1]='\0';
	int sum=0;
	int count=0;
	bool possible=true;

	while(!IS_NULLSTR(text)){
		text=one_argument(text,word);
		int sn=skill_lookup(word);
		if(sn>=0 && ch->pcdata && ch->pcdata->learned[sn]>0){
			sprintf(buf, " `%c%s", have_colour, word);
		}else{
			sprintf(buf, " `%c%s", havenot_colour, word);
			possible=false;
		}
		strcat(workspace, buf);
		{ // add on the players percentages
			if(sn>=0 && ch->pcdata && ch->pcdata->learned[sn]>0){
				sprintf(buf, "(%d%%)", ch->pcdata->learned[sn]);
				strcat(workspace, buf);
				sum+=ch->pcdata->learned[sn];
				count++;
			}
		}

	}
	if(possible && count>0){
		sprintf(result,"`%c[%5s%%]%s", have_colour, FORMATF("%3.1f",(double)sum/(double)count), &workspace[1]);
	}else{
		strcpy(result,&workspace[1]);
	}
	return result;
}
/****************************************************************************/
char * get_spinfo_data(char_data *ch, int sn)
{
	static char buf[MSL];

	char spell_colour;
	if(sn>=0 && ch->pcdata && ch->pcdata->learned[sn]>0){
		spell_colour='y';
	}else{
		spell_colour='Y';
	}
	
	sprintf(buf, "`%c%-20s%7s :%s%s%s:", 
		spell_colour,
		skill_table[sn].name, spell_class_code(sn),
		IS_SET(skill_table[sn].flags,SKFLAGS_TEACH_SPGAIN)?"T":" ",
		IS_SET(skill_table[sn].flags,SKFLAGS_LEVEL_SPGAIN)?"L":" ",
		IS_SET(skill_table[sn].flags,SKFLAGS_STUDY_SPGAIN)?"S":" ");

	if(skill_table[sn].realms){
		strcat(buf, get_spinfo_data_requirements( ch, flag_string( realm_flags, skill_table[sn].realms), 'r', 'R'));
	}
	if(skill_table[sn].spheres){
		strcat(buf, get_spinfo_data_requirements( ch, flag_string( sphere_flags, skill_table[sn].spheres), 'g', 'G'));
	}
	if(skill_table[sn].elements){
		strcat(buf, get_spinfo_data_requirements( ch, flag_string( element_flags, skill_table[sn].elements), 'b', 'B'));
	}
	if(skill_table[sn].compositions){
		strcat(buf, get_spinfo_data_requirements( ch, flag_string( composition_flags, skill_table[sn].compositions), 'c', 'C'));
	}
	strcat(buf,"`x\r\n");

	return buf;
}
/****************************************************************************/
void do_spinfo(char_data *ch, char * argument)
{
	int j;
	int type=0, bitfield=0, count=0;
	
	if(IS_NULLSTR(argument))
	{
		if(GAMESETTING(GAMESET_BARDDONE) || IS_IMMORTAL(ch))
		{
			ch->wrapln(  "Spell Info (SPINFO) tells you what spells a class "
			"can get and what realms, spheres, compositions, elements & seasons are required to get that.\r\n" );
			ch->println( "  Usage: spinfo `S<`Rrealm`S|`Gsphere`S|`Belement/season`S|`Ccomposition`S|`Yall`S>`x" );
			ch->println( "`S==`xRealms - mage based requirements:" );
			ch->println( "  `rabjuration alteration charm conjuration enchantment evocation" );
			ch->println( "  foretelling illusion necromancy phantasm summoning wild essence`x" );
			ch->println( "`S==`xSpheres - cleric based requirements:" );
			ch->println( "  `Gbody combat convocation creation death divination" );
			ch->println( "  `Gelemental healing mind nature protection time weather`x" );
			ch->println( "`S==`xElements/Seasons - druid based requirements:" );
			ch->println( "  `Bair animal earth fire land moon plant sun water" );
			ch->println( "  `Bautumn spring summer winter`x" );
			ch->println( "`S==`xCompositions - bard based requirements:" );
			ch->println( "  `Cbeguiling ceremonial epic esoteric`x" );
			ch->println( "  `Cethereal holistic requiem`x" );
			return;
		}
		else
		{
			ch->wrapln(  "Spell Info (SPINFO) tells you what spells a class "
			"can get and what realms, spheres, elements & seasons are required to get that.\r\n" );
			ch->println( "  Usage: spinfo `S<`Rrealm`S|`Gsphere`S|`Belement/season`S|`Yall`S>`x" );
			ch->println( "`S==`xRealms - mage based requirements:" );
			ch->println( "  `rabjuration alteration charm conjuration enchantment evocation" );
			ch->println( "  foretelling illusion necromancy phantasm summoning wild essence`x" );
			ch->println( "`S==`xSpheres - cleric based requirements:" );
			ch->println( "  `Gbody combat convocation creation death divination" );
			ch->println( "  `Gelemental healing mind nature protection time weather`x" );
			ch->println( "`S==`xElements/Seasons - druid based requirements:" );
			ch->println( "  `Bair animal earth fire land moon plant sun water" );
			ch->println( "  `Bautumn spring summer winter`x" );
			return;
		}
	}

	if(!str_cmp(argument,"all"))
	{
		type=5;
	}
	else
	{
		if (( bitfield= flag_value( realm_flags, argument )) != NO_FLAG )
		{
			type=1;
		}
		else if (( bitfield= flag_value( sphere_flags, argument )) != NO_FLAG )
		{
			type=2;
		}
		else if (( bitfield= flag_value( element_flags, argument )) != NO_FLAG )
		{
			type=3;
		}
		else if (( bitfield= flag_value( composition_flags, argument )) != NO_FLAG )
		{
			if(GAMESETTING(GAMESET_BARDDONE) || IS_IMMORTAL(ch))
			{ type=4; }
			else
			{ type=5; }
		}
		else
		{
			if ( GAMESETTING(GAMESET_BARDDONE) || IS_IMMORTAL(ch))
			{ ch->println( "No such realm, sphere, composition, or element/season." ); }
			else
			{ ch->println( "No such realm, sphere, or element/season." ); }
			do_spinfo(ch,"");
			return;
		}
	}
		ch->printlnf(
		"Spell: Required realms and Spheres of Influence (see help spinfo-key)\r\n"
		"50%% level on spells with colour codes:\r\n"
		"`b1`x-`c8`x-`g14`x-`m20`x-`r26`x-`y32"
		"`x-`S38`x-`w44`x-`W50`x-`Y56`x-`R62`x-`M68`x-`G74`x-`C80`x-`B86+`x\r\n"
		"____________________CLASS____________________________________________" );

	for(j=FIRST_SPELL; j<=LAST_SPELL; j++)
	{
		switch(type)
		{
		case 1:// realm
			if(!IS_SET(skill_table[j].realms, bitfield))
				continue;
			break;
		case 2:// sphere
			if(!IS_SET(skill_table[j].spheres, bitfield))
				continue;
			break;
		case 3:// element
			if(!IS_SET(skill_table[j].elements, bitfield))
				continue;
			break;
		case 4:// composition
			if(!IS_SET(skill_table[j].compositions, bitfield))
				continue;
			break;
		case 5:// all
			break;
		}

		if(!IS_SET(skill_table[j].flags,
			SKFLAGS_TEACH_SPGAIN|SKFLAGS_LEVEL_SPGAIN|SKFLAGS_STUDY_SPGAIN))
		{
			continue;
		}

		if(IS_IMMORTAL(ch))
		{
			if( !IS_NULLSTR(skill_table[j].msp_sound ))
			{
				ch->printf("%3d] %s", ++count, get_spinfo_data(ch, j));
			}
			else
			{
				ch->printf("%3d) %s", ++count, get_spinfo_data(ch, j));
			}
		}
		else
		{
			ch->print(get_spinfo_data(ch, j));
		}
	}
	
	if(IS_IMMORTAL(ch))
	{
		ch->println( "===BELOW HERE UNSEEN SPELLS BY MORTS===");
		for(j=FIRST_SPELL; j<=LAST_SPELL; j++)
		{
			switch(type)
			{
			case 1:// realm
				if(!IS_SET(skill_table[j].realms, bitfield))
					continue;
				break;
			case 2:// sphere
				if(!IS_SET(skill_table[j].spheres, bitfield))
					continue;
				break;
			case 3:// element
				if(!IS_SET(skill_table[j].elements, bitfield))
					continue;
				break;
			case 4:// composition
				if(!IS_SET(skill_table[j].compositions, bitfield))
					continue;
				break;
			case 5:// all
				break;
			}

			if(IS_SET(skill_table[j].flags,
				SKFLAGS_TEACH_SPGAIN|SKFLAGS_LEVEL_SPGAIN|SKFLAGS_STUDY_SPGAIN))
			{
				continue;
			}

			if(IS_IMMORTAL(ch))
			{
				ch->printf( "%3d) %s", ++count, get_spinfo_data(ch, j));
			}
			else
			{
				ch->print(get_spinfo_data(ch, j));
			}
		}
	}

	ch->println( "The TLS Column is ways to gain the spell: T = Teach, L = Level, S = Study." );

	ch->println( "____________________CLASS____________________________________________\r\n"
				 "50%% level on spells with colour codes:\r\n"
				 "`b1`x-`c8`x-`g14`x-`m20`x-`r26`x-`y32"
				 "`x-`S38`x-`w44`x-`W50`x-`Y56`x-`R62`x-`M68`x-`G74`x-`C80`x-`B86+`x");
	return;
}

/****************************************************************************/
void do_cspinfo(char_data *ch, char * argument)
{
	bool found = false;
	char buf[MSL], clss_name[MSL];
	int j, count, lev;
	BUFFER *output;

	int clss_no;

	if (!argument[0])
	{
		ch->println( "CSPINFO - Class SPINFO - shows all spells a class can get." );
		ch->println( "Syntax: CSPINFO <class>" );
		return;
	}

	argument = one_argument (argument, clss_name);

	for (clss_no = 0; class_table[clss_no].name; clss_no++)
	{
		if ( class_table[clss_no].creation_selectable == false )
			continue;
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
		ch->println(  "CSPINFO - Class SPINFO - shows all spells a class can get." );
		ch->println(  "Syntax: CSPINFO <class>" );
		return;
	}

	// setup a buffer for info to be displayed
	output	= new_buf();
	count	= 0;

	sprintf(buf,"Class Spell Info - all spells for `B%ss`x\r\n", class_table[clss_no].name);
	add_buf( output, buf);

	sprintf(buf,
		"Spell: Required realms, spheres, elements & seasons (see help spinfo-key)\r\n"
		"50%% level on spells with colour codes:\r\n"
		"`b1`x-`c8`x-`g14`x-`m20`x-`r26`x-`y32"
		"`x-`S38`x-`w44`x-`W50`x-`Y56`x-`R62`x-`M68`x-`G74`x-`C80`x-`B86+`x\r\n"
		"____________________CLASS____________________________________________\r\n");
    add_buf( output, buf);

	for (lev=1; lev<=LEVEL_HERO; lev++)
	{      
		for(j=FIRST_SPELL; j<=LAST_SPELL; j++)
		{
			if(!IS_SET(skill_table[j].flags,
				SKFLAGS_TEACH_SPGAIN|SKFLAGS_LEVEL_SPGAIN|SKFLAGS_STUDY_SPGAIN))
			{
				continue;
			}
			if (skill_table[j].skill_level[clss_no]== lev)
			{
				count++;
				if ( !IS_NULLSTR( skill_table[j].msp_sound ))
				{
					sprintf(buf, "%3d] %s", count, get_spinfo_data(ch, j));
				}
				else
				{
					sprintf(buf, "%3d) %s", count, get_spinfo_data(ch, j));
				}
				add_buf( output, buf); 
			}
		}
	}

	add_buf( output,
		"50% level on spells with colour codes:\r\n"
		"`b1`x-`c8`x-`g14`x-`m20`x-`r26`x-`y32"
		"`x-`S38`x-`w44`x-`W50`x-`Y56`x-`R62`x-`M68`x-`G74`x-`C80`x-`B86+`x\r\n"
		"____________________CLASS____________________________________________\r\n");
	sprintf(buf,"The TLS Column is ways to gain the spell: T = Teach, L = Level, S = Study.\r\n");
	add_buf( output,buf);
	ch->sendpage(buf_string(output));
	free_buf(output);
	return;
}

/****************************************************************************/
void do_sinfo(char_data *ch, char * argument)
{
	int j;
	
	if(IS_NULLSTR(argument))
	{
		ch->println( "Syntax: sinfo <part_of_a_spellname>" );
		return;
	}

	ch->titlebar("SPELL INFO");
	ch->printlnf(
		"SINFO: Required realms, spheres, elements & seasons(see help spinfo-key)\r\n"
		"50%% level on spells with colour codes:\r\n"
		"`b1`x-`c8`x-`g14`x-`m20`x-`r26`x-`y32"
		"`x-`S38`x-`w44`x-`W50`x-`Y56`x-`R62`x-`M68`x-`G74"
		"`x-`C80`x-`B86+`x\r\n"
		"____________________CLASS____________________________________________");


	for(j=FIRST_SPELL; j<=LAST_SPELL; j++)
	{
		if(!IS_SET(skill_table[j].flags,
			SKFLAGS_TEACH_SPGAIN|SKFLAGS_LEVEL_SPGAIN|SKFLAGS_STUDY_SPGAIN))
		{
			continue;
		}	
		
		if(is_name(argument, skill_table[j].name))
		{			
			ch->print(get_spinfo_data(ch, j));
		}
	}
	ch->println( "The TLS Column is ways to gain the spell: T = Teach, L = Level, S = Study." );
	return;
}

/****************************************************************************/
#define MAX_SPELL_TARGET_NAMES (5)
char *spell_target_names[MAX_SPELL_TARGET_NAMES]={
	"Area Affect or Informational Spells",
	"Offensive Spells",
	"Defensive Spells",
	"Use On Self Only Spells",
	"Object Spells",
};

int spell_target_names_count[MAX_SPELL_TARGET_NAMES];
/****************************************************************************/
int spell_get_target_index(int sn)
{
	int index=0;
	switch(skill_table[sn].target){
		case TAR_IGNORE:			index=0; break; // Area Affect or Informational Spells(0)
		case TAR_CHAR_OFFENSIVE:	index=1; break; // Offensive Spells (1)
		case TAR_CHAR_DEFENSIVE:	index=2; break; // Defensive Spells (2)
		case TAR_CHAR_SELF:			index=MAX_SPELL_TARGET_NAMES-2; break; // Self only spells
		case TAR_OBJ_INV:			index=MAX_SPELL_TARGET_NAMES-1; break; // Object Spells 
		case TAR_OBJ_CHAR_DEF:		index=2; break; 
		case TAR_OBJ_CHAR_OFF:		index=1; break;
		case TAR_MOB_OFFENSIVE:		index=1; break;
		case TAR_OBJ_MOB_OFF:		index=1; break;
		case TAR_DIRECTION:			index=0; break; 
		default:	index=0; break;
	}
	return index;
}

/****************************************************************************/
void do_spells(char_data *ch, char * argument)
{
	bool found = false;
	int i;
	char buf[MSL];
	char col[MSL];
	BUFFER *output;
	BUFFER *spell_target_buffers[MAX_SPELL_TARGET_NAMES];
	int t; // used to count MAX_SPELL_TARGET_NAMES
	char arg[MIL];
	char_data *victim;  
	int count;

	if (IS_UNSWITCHED_MOB(ch)){
		ch->println( "Unswitched mobs can't use this." );
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
		ch->println( "Mobs don't have spells as such." );
		return;
	}

    output = new_buf();
	for(t=0; t<MAX_SPELL_TARGET_NAMES; t++){
		spell_target_buffers[t]=new_buf();
		spell_target_names_count[t]=0;
	}

	add_buf(output,"`?`#"); // get the random colour for title bars

	if (ch!=victim)
	{
		sprintf(buf,"Spells for %s (Level: %d, Race: %s, Class: %s)\r\n",
			victim->name, 
			victim->level, 
			race_table[victim->race]->name, 
			class_table[victim->clss].name);
	}
	else
	{
		sprintf(buf,"`^%s`x", format_titlebar("SPELLS"));
	}
    add_buf( output, buf );
	add_buf( output, percent_colour_codebar());
	count=0;

	for(i=FIRST_SPELL; i<=LAST_SPELL; i++)
	{
		if(victim->pcdata->learned[i])
		{
			found=true;
			count++;
			strcpy(col, percent_colour_code(victim->pcdata->learned[i]));
			sprintf(buf,"   %s%-22.22s %3d `x(%3d%%)   ", col,
				capitalize(skill_table[i].name), skill_table[i].min_mana, victim->pcdata->learned[i]);

			// put it into the correct list
			t=spell_get_target_index(i);
			if(++spell_target_names_count[t]%2==0){
				add_buf( spell_target_buffers[t], rtrim_string(buf) );
				add_buf( spell_target_buffers[t], "\r\n");
			}else{
				add_buf( spell_target_buffers[t], buf );
			}
		}
	}

	if (!found)
    {
		if ( ch == victim ){
			ch->println( "You have no spells." );
		}else{
			ch->println( "They have no spells." );
		}
	}
	else
	{
		// construct the spell groupings into the main buffer
		for(t=0; t<MAX_SPELL_TARGET_NAMES; t++){
			add_buf( output, FORMATF("`^%s`x", format_titlebar(spell_target_names[t])));
			add_buf( output, buf_string(spell_target_buffers[t]));
			if(spell_target_names_count[t]%2!=0){
				add_buf( output, "\r\n" );
			}
		}

		add_buf( output, FORMATF("`^%s`x", format_titlebarf("%d SPELL%s TOTAL", count, count==1?"":"S")));
		ch->sendpage(buf_string(output));
	}

	// recycle the memory
	free_buf(output);
	for(t=0; t<MAX_SPELL_TARGET_NAMES; t++){
		free_buf(spell_target_buffers[t]);
	}

	return;
}
 
/****************************************************************************/
void do_study(char_data * ch, char * argument)
{
	char arg1[MIL];
	OBJ_DATA *scroll;
	int i;

	if(IS_NPC(ch))
	{
		ch->println( "Only players can study." );
		return;
	}

	set_char_magic_bits(ch);

	argument = one_argument( argument, arg1 );

	if (( scroll = get_obj_carry( ch, arg1 )) == NULL )
	{
		ch->println( "You do not have that scroll." );
		return;
	}

	if ( scroll->item_type != ITEM_SCROLL )
	{
		ch->println( "You can study only scrolls." );
		return;
	}

	for(i=1; i<=4; i++) // loop thru each spell
	{
		if (scroll->value[i]<1)
			continue;

		if (TRUE_CH(ch)->pcdata->learned[scroll->value[i]]>0)
		{
			ch->printlnf( "You already know %s.",
				skill_table[scroll->value[i]].name);
			continue;
		}

		if (!IS_SET(skill_table[scroll->value[i]].flags, SKFLAGS_STUDY_SPGAIN))
		{
			ch->printlnf( "You don't appear to be able to study %s.",
				skill_table[scroll->value[i]].name);
			return;
		}

		if (!has_spell_req(ch,scroll->value[i]))
		{
			ch->printlnf( "You do not have the requirements to learn %s.",
				skill_table[scroll->value[i]].name);
			continue;
		}

		if(check_learn_spell(ch,scroll->value[i],3))
		{
			TRUE_CH(ch)->pcdata->learned[scroll->value[i]]=1;
			ch->printlnf( "Your studious nature has paid off.\r\n"
				"You have learned %s!",	skill_table[scroll->value[i]].name);
			check_realm_improve(ch,scroll->value[i], true, 25);
		}
		else
		{
			ch->printlnf( "You have failed to learn %s.",
				skill_table[scroll->value[i]].name);
			check_realm_improve(ch,scroll->value[i], false, 15);
		}
	}

	extract_obj(scroll);
	WAIT_STATE(ch, 150);
	return;
}

/****************************************************************************/
void do_teach(char_data *ch, char * argument)
{
	char arg1[MIL];
	char arg2[MIL];
	int sn;
	char_data *victim;
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	
	if(arg1[0]=='\0' || arg2[0]=='\0')
	{
		ch->println( "Syntax: teach <character> <spell>" );
		return; 
	}
	
	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		ch->printlnf( "'%s' isn't here.", arg1 );
		return; 
	}
	
	if(IS_SET(victim->act, PLR_NOTEACH))
	{
		ch->println( "They ignore you as you attempt to teach them." );
		victim->printlnf( "%s attempts to teach you a spell.\r\n"
			"You simply ignore them.", PERS(ch, victim));
		return;
	}

	if( ( sn = skill_lookup(arg2) ) == -1  || !IS_SPELL(sn))
	{
		ch->printlnf( "'%s' is not a spell.", arg2 );
		return;
	} 

	if(IS_NPC(victim)){
		ch->println( "You can't teach NPCs spells." );
		return;
	}

	if(!IS_IMMORTAL(ch)){
		if(IS_NPC(ch)){
			ch->println( "Not with NPCs." );
			return;
		}
		
		if(ch->pcdata->learned[sn]<1)
		{
			ch->printlnf( "You do not know the spell '%s'.", skill_table[sn].name);
			return;
		}
	}

	if(!IS_SET(skill_table[sn].flags, SKFLAGS_TEACH_SPGAIN))
	{
		ch->printlnf( "'%s' is not a teachable spell.", skill_table[sn].name);
		return;
	}

	if(!IS_IMMORTAL(ch) && !GAMESETTING3(GAMESET3_CAN_TEACH_SPELLS_WHILE_UNLEARNED)){
		// game option to require players to be praced in a spell before they can teach it.
		int adept=skill_table[sn].maxprac_percent[ch->clss];
		if(adept==0){
			if(HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER)){
				adept=50;
			}else{
				adept=class_table[ch->clss].skill_adept;
			}
		}
		if ( ch->pcdata->learned[sn]<adept ){
			ch->printlnf( "You are not learned in '%s'.", skill_table[sn].name);
			return;
		}
	}


	if(IS_SET(skill_table[sn].flags, SKFLAGS_NO_INTERCLASS_TEACH) 
		&& ch->clss!=victim->clss)
	{
		ch->printlnf( "'%s' can only be taught to those of the same class as you.", 
			skill_table[sn].name);
		if ( !IS_IMMORTAL( ch ))
			WAIT_STATE(ch, 30);
		return;
	}

	//can't teach/learn if you have a headache
	if (is_affected( ch, gsn_cause_headache ))
	{
		ch->println( "Your head is exploding with a headache. You can barely concentrate " );
		ch->println( "on your name, much less on teaching a spell." );
		victim->println( "Your teacher seems to be suffering from a terrible headache, " );
		victim->println( "and was unable to teach you anything." );
		return;
	}

	if(is_affected( victim, gsn_cause_headache )){
		ch->println( "Your student seems unable to concentrate on anything right now." );
		victim->println(  "Your headache is killing you, you could not grasp any " );
		victim->printlnf( "meaning from %s's words.", ch->short_descr);
		return;
	}

	if(!has_spell_req(victim,sn))
	{
		ch->println( "After you started the lesson, you realized student isn't familiar with all " );
		ch->println( "the spheres and realms that spell involves.  You decided any more teaching " );
		ch->println( "would be too unpredictable." );

		victim->wrapln( "Your teacher realized once the lesson started, that you did not know "
			"all the spheres and realms that spell involves, and continuing the "
			"lesson would have been too unpredictable." );
		
		if ( !IS_IMMORTAL( ch )){
	        WAIT_STATE(ch, 30);
		}
		if ( !IS_IMMORTAL( victim )){
	        WAIT_STATE(victim, 35);
		}
        return;
	}

    // can't learn a spell with the level the same as the low_percent_level
    if (skill_table[sn].skill_level[ch->clss]
			== skill_table[sn].low_percent_level[ch->clss])
	{
		ch->wrapln( "After you started the lesson, you realized your student is unable to grasp the"
			"concepts to learn this spell, and it is very apparent they never will." );

		victim->wrapln( "Your teacher realized once the lesson started, that you are unable"
			"to grasp the concepts to learn this spell, and it is very apparent that"
			"you never will understand the concepts well enough to learn the spell." );
		if ( !IS_IMMORTAL( ch )){
	        WAIT_STATE(ch, 15);
		}
		if ( !IS_IMMORTAL( victim )){
	        WAIT_STATE(victim, 20);
		}
        return;
	}

	if(victim->position>POS_RESTING)
	{
		ch->println( "Your student must be in a relaxed state." );
		return;
	}
	
	if(victim->position<POS_RESTING)
	{
		ch->println( "Your student must be awake." );
		return;
	}

	if(victim->pcdata->learned[sn]>0)
	{
		ch->printlnf( "They already know '%s'.", skill_table[sn].name);
		victim->printlnf( "Your teacher realized once the lesson started you already knew '%s'.",
			skill_table[sn].name);
		return;
	}

	if(check_learn_spell(victim,sn,2))
	{
		victim->pcdata->learned[sn]=1;
		victim->printlnf( "You have learned %s from %s.",
            skill_table[sn].name, PERS(ch, victim));
		ch->println( "You have taught your student well." );
		check_realm_improve(ch,sn,true, 25);
		check_realm_improve(victim,sn,true, 15);
	}else{ 
		ch->printlnf( "You have failed to teach %s %s.", 
			PERS(victim, ch), skill_table[sn].name);
		victim->printlnf( "You have failed to learn %s from %s.",
			skill_table[sn].name, PERS(ch, victim));

		// you don't improve if they can't learn the spell
		if(can_learn_spell(victim, sn))
		{
			check_realm_improve(ch,sn,false,10);
			check_realm_improve(victim,sn,false, 15);
		}else{
			int newpercentage=UMAX(1, ch->pcdata->learned[sn]-number_range(1,number_range(2,20)));
			int loss=(ch->pcdata->learned[sn]-newpercentage);
			loss=loss * 2 * skill_table[sn].rating[ch->clss];

			if(loss){
				ch->printlnf( "In fact you got so confused you have gotten worse at %s.", 
					skill_table[sn].name);
				gain_exp(ch, -loss);
				ch->printlnf( "You have lost %d xp as a result.", loss );
				ch->pcdata->learned[sn]= newpercentage;
			}else{ // wont actually lose the spell
				ch->printlnf( "Lucky you didn't get confused and forget %s all together.", 
					skill_table[sn].name);
			}
		}
	}

	if ( !IS_IMMORTAL( ch )){	
		WAIT_STATE(ch, 225);
	}
	if ( !IS_IMMORTAL( victim )){
		WAIT_STATE(victim, 225);
	}
	
	return; 
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
