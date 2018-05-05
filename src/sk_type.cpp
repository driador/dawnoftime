/**************************************************************************/
// sk_type.cpp - start of implementation of skill_type member functions 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "sk_type.h"
/**************************************************************************/
int skill_type::get_maxprac(char_data *ch)
{
	if(HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER)){
		if(maxprac_percent[ch->clss]){
			return URANGE(0,maxprac_percent[ch->clss], 100);
		}
		return(50); // default maxprac for now
	}else{
		return URANGE(0,class_table[ch->clss].skill_adept, 100);
	}
};
/**************************************************************************/
int skill_type::get_learnscale(char_data *ch)
{
	if(HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER)){
		if(learn_scale_percent[ch->clss]){
			return URANGE(0,learn_scale_percent[ch->clss], 100);
		}
		return(65); // default maxlearn for now
	}else{
		return(100); // old system user 
	}
};
/**************************************************************************/

