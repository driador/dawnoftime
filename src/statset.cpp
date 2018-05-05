/**************************************************************************/
// statset.cpp - stats set handling class, 
//               used as backend of autostats system - Kal, May 2000
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

/* File format:
-1 = eof marker
-2 = comment to end of line

// file format
level X Y Z
// level = X d Y + Z

level X -1 0 
// = fixed at X for level
 
*/
#include "include.h"
#include "statset.h"
/**************************************************************************/
// read in a statset from a file
void statset::load_statset(const char *filename)
{
	// open our file for input
	FILE *fp=fopen( filename,"r");
	if(!fp){
		valid=false;
		bugf("statset::load_statset(): Couldn't open file '%s' to read in statset from!",
			filename);
		return;
	}

	logf("statset::load_statset(): Loading in statset from %s...", filename);
	// initialise the levels as empty
	memset(&lev[0], 0, sizeof(diceset_type)* (MAX_LEVEL+STATSET_ABOVE_MAX_LEVEL));

	int level=0;
	while(!feof(fp)){
		level=fread_number(fp);

		if(level==-1){// end of file marker
			break;
		}

		if(level==-2){// comment to end of line
			fread_to_eol(fp); // allow expansion later on/comments
			continue;
		}


		if(level<0 || level>MAX_LEVEL+STATSET_ABOVE_MAX_LEVEL){
			bugf("statset::load_statset(): value %d read in from file "
				"- not within level valid range... aborting!", level);
			do_abort();
		}

		lev[level].dice[DICE_ROLLS]=fread_number(fp);
		lev[level].dice[DICE_SIDES]=fread_number(fp);
		lev[level].dice[DICE_BONUS]=fread_number(fp);
		fread_to_eol(fp); // allow expansion later on/comments
	}
	if(feof(fp) && level!=-1){
		logf("statset::load_statset(): load incomplete, aborting!");
		do_abort();
	}

	valid=true;

	logf("statset::load_statset(): Load complete.");
	fclose(fp);
}
/**************************************************************************/
// save statset to a file
void statset::save_statset(const char *filename)
{
	if(!valid){
		bugf("statset::save_statset('%s'): current stat set is NULL!", filename);
		do_abort(); // temp for now?
		return;
	}

	logf("statset::save_statset(): Saving in statset to %s...", filename);

	// open our file for output
	FILE *fp=fopen( filename,"w");
	if(!fp){
		bugf("statset::save_statset(): Couldn't open file '%s' to save statset to!",
			filename);
		return;
	}

	// loop thru saving the lot
	fprintf(fp,"-2 Level  Dice_rolls  Dice_sides  Dice_bonus\n"); // finish our file off
	for(int level=0; level<MAX_LEVEL+STATSET_ABOVE_MAX_LEVEL; level++){
		if(lev[level].dice[DICE_ROLLS] 
			|| lev[level].dice[DICE_SIDES]==-1
			|| lev[level].dice[DICE_BONUS])
		{
			fprintf(fp,"%d %d %d %d\n", 
				level,
				lev[level].dice[DICE_ROLLS],
				lev[level].dice[DICE_SIDES],
				lev[level].dice[DICE_BONUS]);
		}
	}
	fprintf(fp,"-1\n"); // finish our file off

	fclose(fp);

	logf("statset::save_statset(): Saving complete.");

}
/**************************************************************************/
// return true if an error with level or invalid
bool statset::bounds_error(int level, bool fixed)
{
	if(!valid){
		bug("statset::bounds_error(): invalid node!");
		return true;
	}

	if(level<0 || level>=MAX_LEVEL+STATSET_ABOVE_MAX_LEVEL){
		bugf("statset::bounds_error(): level %d out of range!", level);
		return true;
	}

	if(lev[level].dice[DICE_SIDES]==-1){ // fixed value
		if(!fixed){
			bugf("statset::bounds_error(): fixed value requested, "
				"not fixed entry type (level %d)", level);
			return true;
		}
	}else{
		if(fixed){
			bugf("statset::bounds_error(): dice value requested, "
				"not dice entry type (level %d)", level);
			return true;
		}
	}

	if(lev[level].dice[DICE_ROLLS]==0 && lev[level].dice[DICE_SIDES]==0){
		bugf("statset::bounds_error(): no statset value for level %d.", level);
		return true;
	}

	return false;
};
/**************************************************************************/
// return false if error
bool statset::apply_autostat(int level, int *dice_rolls, int *dice_sides, int *dice_bonus)
{
	if(bounds_error(level, false)){return false;}

	*dice_rolls=lev[level].dice[DICE_ROLLS];
	*dice_sides=lev[level].dice[DICE_SIDES];
	*dice_bonus=lev[level].dice[DICE_BONUS];
	return true;
};
/**************************************************************************/
bool statset::apply_autostat(int level, int *fixed_value)
{
	if(bounds_error(level, true)){return false;}

	*fixed_value=lev[level].dice[DICE_ROLLS];
	return true;
};

/**************************************************************************/
bool statset::apply_autostat(int level, short *dice_rolls, short *dice_sides, short *dice_bonus)
{
	if(bounds_error(level, false)){return false;}

	*dice_rolls=(short)lev[level].dice[DICE_ROLLS];
	*dice_sides=(short)lev[level].dice[DICE_SIDES];
	*dice_bonus=(short)lev[level].dice[DICE_BONUS];
	return true;
};
/**************************************************************************/
bool statset::apply_autostat(int level, short *fixed_value)
{
	if(bounds_error(level, true)){return false;}

	*fixed_value=(short)lev[level].dice[DICE_ROLLS];
	return true;
};
/**************************************************************************/
/**************************************************************************/
