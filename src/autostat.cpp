/**************************************************************************/
// autostat.cpp - auto stats setting system - Kal, May 2000
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "olc.h"
#include "statset.h"

statset mob_hitpoints;
statset mob_damroll;
statset mob_ac;
statset mob_mana;
/**************************************************************************/
// Save statsets to disk, 
// - just used for reformatting the files nicely at this stage
void save_autostat_files()
{
	mob_hitpoints.save_statset(AUTOSTAT_DIR"mob_hitp.sav");
	mob_damroll.save_statset(AUTOSTAT_DIR"mob_damr.sav");
	mob_mana.save_statset(AUTOSTAT_DIR"mob_mana.sav");
	mob_ac.save_statset(AUTOSTAT_DIR"mob_ac.sav");
}
/**************************************************************************/
void do_save_autostat_files(char_data*ch, char*)
{
	save_autostat_files();
	ch->printf("Stat files saved to *.sav");

}
/**************************************************************************/
void load_autostat_files()
{
	logf("Starting: load_autostat_files()...");
	mob_hitpoints.load_statset(AUTOSTAT_DIR"mob_hitp.txt");
	mob_damroll.load_statset(AUTOSTAT_DIR"mob_damr.txt");
	mob_mana.load_statset(AUTOSTAT_DIR"mob_mana.txt");
	mob_ac.load_statset(AUTOSTAT_DIR"mob_ac.txt");
	logf("Finished: load_autostat_files().");
}
/**************************************************************************/
void do_load_autostat_files(char_data*ch, char*)
{
	load_autostat_files();
	ch->printf("AutoStat files reloaded from *.txt");
}
/**************************************************************************/
// return false for bad input.
// syntax <number> - straight number
// syntax +<number> - base number + amount
// syntax -<number> - base number - amount
bool find_flexivalue(char_data *ch, char*argument, int *result, 
					 int base_value, int min_value, int max_value)
{
	char arg[MIL];
	one_argument(argument, arg);
	int possible_result;

	if(IS_NULLSTR(arg)){
		// no input, just use base value
		*result=base_value;
		return true;
	}

	if(!is_number(arg)){
		ch->println("flexivalue(): Input must be numeric");
		return false;
	}

	possible_result=base_value;

	if(arg[0]=='+'){
		// addition modifier
		possible_result+=atoi(arg+1);
	}else if(arg[0]=='-'){
		// subtraction modifier
		possible_result-=atoi(arg+1);
	}else{
		possible_result=atoi(arg);
	}

	// bounds check the result
	if(possible_result<min_value || possible_result>max_value ){
		ch->printlnf("flexivalue(): The input value must be between %d and %d, %d is not.",
			min_value, max_value, possible_result);
		return false;
	}

	*result=possible_result;
	return true;
}
/**************************************************************************/
// this way we don't need a descriptor on a character
/*
*Thief mobiles should read their hp, ac, and damage at one level lower.
*Mage mobiles read hp and ac at one level lower, and damage three levels lower.
*Cleric mobiles read damage at two levels lower.
*Warrior mobiles read hit points one level higher.
*Armor class vs. magical attacks should be 80-90% of the other AC's.

An average of 100 + 10xlevel for the mana value of mobs. 
This is highly susceptive to change in accordance to the type of mob. 
Mage mobs to have the above recommended values. 
Warrior types to have 50%. 
Barbarian types (hatred to magic type mobs) to have as low as even 10%. 
*/
bool autostat_mindex(char_data* ch, char* argument, mob_index_data *mi)
{
	char buf[MSL];
	int level;

	if(!IS_NULLSTR(argument) && argument[0]=='?'){
		ch->println("medit_autostat -  syntax: autostat <flexivalue_level>");
		ch->printlnf("Flexivalue is the term %s uses for is a value that can be absolute or relative.",
			MUD_NAME);
		ch->println("autostat      - sets the stats for the mobs current level.");
		ch->println("autostat 10   - sets the stats as if the mob was level 10.");
		ch->println("autostat +10  - sets the stats as if the mob was 10 levels higher than its current level.");
		ch->println("autostat -10  - sets the stats as if the mob was 10 levels lower than its current level.");
		return false;
	}
	
	if(!find_flexivalue(ch, argument, &level, mi->level, 0, MAX_LEVEL+STATSET_ABOVE_MAX_LEVEL-1)){
		return false;
	};
	
	if(level==mi->level){
		ch->printlnf("\r\nAuto setting stats for mob[%d] level %d", mi->vnum, level);
	}else{
		ch->printlnf("\r\nAuto setting stats for %slevel %d`x, (mob[%d] level is %d).", 
			level>mi->level?"`G":"`R",
			level, 
			mi->vnum, 
			mi->level);
	}

	// hitpoints
	sprintf(buf,"%dd%d+%d", 
		mi->hit[DICE_ROLLS],
		mi->hit[DICE_SIDES],
		mi->hit[DICE_BONUS]);

	int hlev=level;
	if(IS_SET(mi->act, ACT_WARRIOR)){
		ch->println("Warrior Act Flag - Hitpoints Level +1");
		hlev+=1;
	}
	if(IS_SET(mi->act, ACT_THIEF)){
		ch->println("Thief Act Flag - Hitpoints Level -1");
		hlev-=1;
	}
	if(IS_SET(mi->act, ACT_MAGE)){
		ch->println("Mage Act Flag - Hitpoints Level -1");
		hlev-=1;
	}
	
	if(mob_hitpoints.apply_autostat(hlev, &mi->hit[DICE_ROLLS], 
		&mi->hit[DICE_SIDES], &mi->hit[DICE_BONUS]))
	{
		ch->printlnf("Hitpoints changed from %s to %dd%d+%d",
			buf,
			mi->hit[DICE_ROLLS],
			mi->hit[DICE_SIDES],
			mi->hit[DICE_BONUS]);
	}else{
		ch->printlnf("`RHitpoints unchanged from %s - most likely due to no hitpoints settings for level %d?`x",
			buf, hlev);
	}


	// mana
	sprintf(buf,"%dd%d+%d", 
		mi->mana[DICE_ROLLS],
		mi->mana[DICE_SIDES],
		mi->mana[DICE_BONUS]);


	int mlev=level;
	if(IS_SET(mi->act, ACT_WARRIOR)){
		ch->println("Warrior Act Flag - Mana Level Quartered");
		mlev/=4;
	}
	if(IS_SET(mi->act, ACT_THIEF)){
		ch->println("Thief Act Flag - Mana Level Halved");
		mlev/=2;
	}
	if(IS_SET(mi->act, ACT_MAGE)){
		ch->println("Mage Act Flag - Mana Level Boosted by 10");
		mlev+=10;
	}
	if(mob_mana.apply_autostat(mlev, &mi->mana[DICE_ROLLS], 
		&mi->mana[DICE_SIDES], &mi->mana[DICE_BONUS]))
	{
		ch->printlnf("Mana dice changed from %s to %dd%d+%d",
			buf,
			mi->mana[DICE_ROLLS],
			mi->mana[DICE_SIDES],
			mi->mana[DICE_BONUS]);
	}else{
		ch->printlnf("`RManapoints unchanged from %s - most likely due to no hitpoints settings for level %d?`x",
			buf, mlev);
	}


	// damroll
	int dlev=level;
	if(IS_SET(mi->act, ACT_THIEF)){
		ch->println("Thief Act Flag - Damage Dice Level -1");
		dlev-=1;
	}
	if(IS_SET(mi->act, ACT_MAGE)){
		ch->println("Mage Act Flag - Hitpoints Level -3");
		dlev-=3;
	}
	if(IS_SET(mi->act, ACT_CLERIC)){
		dlev-=1;
		ch->println("Cleric Act Flag - Damage Dice Level -2");
	}
	sprintf(buf,"%dd%d+%d", 
		mi->damage[DICE_ROLLS],
		mi->damage[DICE_SIDES],
		mi->damage[DICE_BONUS]);
	if(mob_damroll.apply_autostat(dlev, &mi->damage[DICE_ROLLS], 
		&mi->damage[DICE_SIDES], &mi->damage[DICE_BONUS]))
	{
		ch->printlnf("Damroll changed from %s to %dd%d+%d",
			buf, 
			mi->damage[DICE_ROLLS],
			mi->damage[DICE_SIDES],
			mi->damage[DICE_BONUS]);
	}else{
		ch->printlnf("`RDamroll unchanged from %s - most likely due to no damroll settings for level %d?`x",
			buf, dlev);
	}

	// ac 
	int alev=level;
	if(IS_SET(mi->act, ACT_THIEF)){
		ch->println("Thief Act Flag - AC Level -1");
		alev-=1;
	}
	if(IS_SET(mi->act, ACT_MAGE)){
		ch->println("Mage Act Flag - AC Level -1");
		alev-=1;
	}
	int ac=500;
	if(mob_ac.apply_autostat(alev, &ac)){
		if(    ac==mi->ac[AC_PIERCE]
			&& ac==mi->ac[AC_BASH]
			&& ac==mi->ac[AC_SLASH]
			&& (ac*0.9)>=mi->ac[AC_EXOTIC]
			&& (ac*0.8)<=mi->ac[AC_EXOTIC])
		{
			ch->printlnf("`gAC unchanged from: %d %d %d %d - in balance for level %d`x",
				mi->ac[AC_PIERCE],
				mi->ac[AC_BASH	],
				mi->ac[AC_SLASH],
				mi->ac[AC_EXOTIC],
				alev);
		}else{
			int new_ac_exotic=ac*number_range(80,90)/100;
			ch->printlnf("AC changed from: %d %d %d %d to %d %d %d %d",
				mi->ac[AC_PIERCE],
				mi->ac[AC_BASH	],
				mi->ac[AC_SLASH],
				mi->ac[AC_EXOTIC],
				ac, ac, ac, new_ac_exotic);
			mi->ac[AC_PIERCE]=mi->ac[AC_BASH]=mi->ac[AC_SLASH]=ac;
			mi->ac[AC_EXOTIC]=new_ac_exotic;
		}
	}else{
		ch->printlnf("`RAC unchanged from: %d %d %d %d - most likely due to no ac settings for level %d?`x",
			mi->ac[AC_PIERCE],
			mi->ac[AC_BASH	],
			mi->ac[AC_SLASH],
			mi->ac[AC_EXOTIC],
			alev);
	}

	if(IS_NULLSTR(argument)){
		ch->println("`xType '`=Cautostat ?`x' for help on some of the features of autostat.");
	}
	return true;
}
/**************************************************************************/
bool medit_autostat(char_data* ch, char* argument)
{
	mob_index_data *mi;

	EDIT_MOB(ch, mi);

	return autostat_mindex(ch, argument, mi);
}
/**************************************************************************/
void do_autobalance(char_data* ch, char* argument)
{
	if (!HAS_SECURITY(ch,9))
	{
		ch->println("You must have a security of 9 to use autobalance.");
		return;
	}

	char lower[MIL];
	char upper[MIL];
	char level[MIL];
	argument=one_argument(argument, lower);
	argument=one_argument(argument, upper);
	argument=one_argument(argument, level);

	if(IS_NULLSTR(level)){
		ch->println("syntax: autobalance <lowervnum> <uppervnum> <level_flexivalue>");
		ch->println("Use '+0' for flexivalue if you want it as the mobs level");
		return;
	}

	if(!is_number(lower)){
		ch->printlnf("Lower vnum must be a numeric value and '%s' is not numeric.", lower);
		return;
	}
	if(!is_number(upper)){
		ch->printlnf("Upper vnum must be a numeric value and '%s' is not numeric.", upper);
		return;
	}
	if(!is_number(level)){
		ch->println("Level_flexivalue must be a numeric expression, digits and +/- are allowed.");
		return;
	}
	
	int l=atoi(lower);
	int u=atoi(upper);

	if(l>u){
		ch->printlnf("Lower vnum (%d) is greater than upper vnum (%d)!", l, u);
		return;
	}

	ch->printlnf("Autobalance on any mobs %d thru %d... using autostat setting of '%s'",
		l, u, level);


	MOB_INDEX_DATA *mi;
	for(int i=l; i<=u; i++){
		mi=get_mob_index( i);
		if(mi){
			autostat_mindex(ch, level, mi);
		}
	}
}
/**************************************************************************/
