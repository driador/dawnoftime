/**************************************************************************/
// sk_type.h - header file for skill_type class
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef SK_TYPE_H
#define SK_TYPE_H

class skill_type
{
public:
	// member functions
	int get_maxprac(char_data *ch);
	int get_learnscale(char_data *ch);

public:
    char * name;					// Name of skill/spell
    sh_int skill_level[MAX_CLASS];	// Level needed by class
    sh_int rating[MAX_CLASS];		// How hard it is to learn
    SPELL_FUN *    spell_fun;		// Spell pointer (for spells)
    sh_int target;					// Legal targets
	sh_int minimum_position;	// Position for caster / user
    sh_int * pgsn;			// Pointer to associated gsn
    sh_int slot;			// Slot for #OBJECT loading
    sh_int min_mana;		// Minimum mana used
    sh_int beats;			// Lag time after use (in pulses (sec/4))
    char * noun_damage;		// Damage message
    char * msg_off;			// Wear off message
    char * msg_obj;			// Wear off message for objects

	// all dynamic	
    unsigned char race_restrict_n[(MAX_RACE+7)/8];
	long flags;
	sh_int low_percent_level[MAX_CLASS]; // 1% level

	// all dynamic data below here
	sh_int	maxprac_percent[MAX_CLASS];
    sh_int	learn_scale_percent[MAX_CLASS];  // percent that 100% is for the class
	sh_int	alignrestrict_flags[MAX_CLASS];
	int	category;	// category of spell/skill
	int sktype;

	sh_int damtype;
	sh_int spell_function_index;

	int realms;				// Mages
    int spheres;			// Clerics
    int elements;			// Druids
	int compositions;		// Bards

	int spellgroup;
	int sect_restrict;
	int sect_enhance;
	int sect_dampen;

	bool component_based;	// if spell needs a component or not
	char *msp_sound;
};

#endif


