/**************************************************************************/
// roles.h - character stat roller, Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef STATS_SYSTEM_DEBUG
//#define STATS_SYSTEM_DEBUG
#endif

// rolemaster stat table structure
struct role_type{
    int low;	// low value on table
	int high;   // high value or equal to.
    int table[10];
};

struct attributes_set{
	int perm[10];		// permanent stats
	int potential[10];  // potential stats
};

// prototypes
void gen_rolemaster_stats(attributes_set *rolled_stats, int bias_against);

