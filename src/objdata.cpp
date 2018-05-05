/**************************************************************************/
// objdata.cpp 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"

/**************************************************************************/
// return the object vnum
vn_int obj_data::vnum()
{
	if(!this){
		return -2; // NULL pointer
	}
	if(pIndexData){
		return pIndexData->vnum; // object vnum
	}
	return 0; // shouldn't ever get zero for objects
}

/**************************************************************************/
// suspend events for self, and anything we contain/next in carried list
void obj_data::moving_from_ic_to_ooc() 
{
	// order is exactly the same as saving the players inventory
	if(next_content){
		next_content->moving_from_ic_to_ooc();
	}

	suspend_events();

	if(contains){
		contains->moving_from_ic_to_ooc();
	}
}
/**************************************************************************/
// unsuspend events for self, and anything we contain/next in carried list
void obj_data::moving_from_ooc_or_load_to_ic() 
{
	// order is exactly the same as saving the players inventory
	if(next_content){
		next_content->moving_from_ooc_or_load_to_ic();
	}

	unsuspend_events();

	if(contains){
		contains->moving_from_ooc_or_load_to_ic();
	}
}
/**************************************************************************/
/**************************************************************************/

