/**************************************************************************/
// statset.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef STATSET_H
#define STATSET_H

#ifdef DICE_BONUS 
#undef DICE_BONUS 
#endif
#define DICE_ROLLS (0)
#define DICE_SIDES (1)
#define DICE_BONUS (2)

#define STATSET_ABOVE_MAX_LEVEL 50

struct diceset_type{
	int dice[3];
};

class statset{
	public:
		void load_statset(const char *filename);
		void save_statset(const char *filename);

		// return false if error
		bool apply_autostat(int level, int *dice_rolls, int *dice_sides, int *dice_bonus);
		bool apply_autostat(int level, int *fixed_value);
		bool apply_autostat(int level, short *dice_rolls, short *dice_sides, short *dice_bonus);
		bool apply_autostat(int level, short *fixed_value);

	private:
		bool bounds_error(int level, bool fixed);
		diceset_type lev[MAX_LEVEL+STATSET_ABOVE_MAX_LEVEL];
		bool valid;
};

#endif // STATSET_H
/**************************************************************************/
