/**************************************************************************/
// races.h - The race class
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef RACES_H
#define RACES_H

/**************************************************************************/
// race flags
#define RACEFLAG_CREATION_ACTIVATED		(B)	// race is activated for creation
#define RACEFLAG_PCRACE					(C)	// pcrace stats have meaning
#define RACEFLAG_DYNAMICALLY_GENERATED	(D)	// pcrace stats have meaning
#define RACEFLAG_PCRACE_WITH_NO_CLASSXP_SET	(E)	

#define RACEFLAG_NIGHTMAP					(G)
#define RACEFLAG_REGEN_SLOW_IN_LIGHT		(H)	
#define RACEFLAG_NEED_TWICE_AS_MUCH_SLEEP	(I)
#define RACEFLAG_ALWAYS_HIDDEN_FROM_MORTAL_RACEINFO (J) // only effects morts
#define RACEFLAG_HIDDEN_FROM_MORTAL_RACEINFO_WHEN_ABOVE_THEIR_REMORT (K) // only effects morts
#define RACEFLAG_LOWCOST_LAUNCH				(L)
#define RACEFLAG_NO_CUSTOMIZATION			(M)


int	race_generate_race_adding_to_race_table(const char *name);
extern const struct flag_type race_flags[];
char *race_get_races_set_for_n_array(unsigned char n_array[]);

/**************************************************************************/
class race_data
{
public: // public member functions
	bool pc_race() { return IS_SET(flags, RACEFLAG_PCRACE)==0?false:true;};
	bool creation_selectable();
	void update_dynamic_flags();

public:
	race_data *next;
	char *	name;				// call name of the race 
	char *	short_name;
	language_data *language;			// default language for the race
	long	flags;				// autosave etc
	long	act;				// act bits for the race 
	long	aff; 				// aff bits for the race
	long	aff2; 				// aff2 bits for the race
	long	aff3; 				// aff3 bits for the race
	long	off; 				// off bits for the race
	long	imm;				// imm bits for the race
	long	res;				// res bits for the race
	long	vuln;				// vuln bits for the race
	long	form;				// default form flag for the race
	long	parts;				// default parts for the race

	sh_int	remort_number;		// which remort this race belongs to
	sh_int	points;					// cost in points of the race 
	sh_int	class_exp[MAX_CLASS];	// exp to play race for a particular class
	sh_int	skills[MAX_RACIAL_SKILLS];	// bonus skills for the race
	sh_int	stat_modifier[MAX_STATS];	// starting stats 
	sh_int	size;					// aff bits for the race 
	sh_int	start_hp;
	int		max_hp;
	sh_int	low_size;
	sh_int	high_size;
	int		recall_room;
	int		death_room;
	vn_int	morgue;
	vn_int	newbie_map_vnum;
	sh_int	min_height;
	sh_int	max_height;
	sh_int	min_weight;
	sh_int	max_weight;
	int		food_vnum;

	// dynamic data below here - not saved, generated each time
	int		inuse;			// number of mobs using that race
	int		areacount;		// number of areas the race is used in
	area_data * lastarea;	// used to get the areacount
	char *	load_language;	// used to load/save languages in text form
	char *  load_skills;	// used to load/save skills in text form
};

#endif // RACES_H
/**************************************************************************/
