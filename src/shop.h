/**************************************************************************/
// shop.h - shop functionality written by Slortar
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************
 *																		   *
 * Handles the headers for any classes, functions or anything else to do   *
 * with shops. Try to add more of the shop functionality here at a later   *
 * date. So that it can be encapsulated in one place.                      *
 *																		   *
 * cShopData is a baseclass with all the properties that every type of     *
 * shop should have.													   *
 * cInnData is is derived from cShopData and adds the functionality of	   *
 * inns to that of the basic shops.										   *
 *																		   *
 ***************************************************************************/

/**************************************************************************/
/*
 * TODO: Alter the way rooms are saved. Would be nicer to have a small 
 * vector of rooms instead of the default 4 rooms and rates.
 * 
 * Add a collection class for shops, which is loaded and saved to the
 * area files.
 */

#ifndef SHOP_H
#define SHOP_H

#include "include.h"

/**************************************************************************/
class cShopData  
{
public:
	cShopData();
	virtual ~cShopData();

	vn_int		vnKeeper;				// vnum of the keeper of the shop.
	sh_int		profit_buy;				// percentage of price to charge when buying.
	sh_int		profit_sell;			// profit of price to charge when selling.
	sh_int		open_hour;				// hour the shop opens.
	sh_int		close_hour;				// hour the shop closes.
};

/**************************************************************************/
class cInnData : public cShopData  
{
public:
	cInnData();
	virtual ~cInnData();

	cInnData*	pNextInn;				// Pointer to the next shop in the list.
	int			vnRoom[MAX_INN];		// vnum of room.
	int			shRate[MAX_INN];		// rate of room.
};

/**************************************************************************/

#endif // SHOP_H
