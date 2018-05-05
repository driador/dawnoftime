/**************************************************************************/
// o_lookup.cpp - see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: o_lookup.cpp - olc lookup tables								   *
 *  Written by Kerenos - March 00                                          * 
 ***************************************************************************/

#include "include.h"
#include "o_lookup.h"

void check_obj_balance( char_data *ch );
void check_obj_weapon( char_data *ch, OBJ_INDEX_DATA *pObj );

/**************************************************************************/
// weapon damage dice {1, 10} = 1d10  {2, 8} = 2d8 etc....
static const int weapon_balance_table[100][8][2] =
{
	{ { 1, 5  }},											// 0
	{ { 1, 6  }},
	{ { 1, 7  },{ 2, 3  }},
	{ { 1, 8  }},
	{ { 1, 9  },{ 2, 4  }},
	{ { 1, 10 }},											// 5
	{ { 1, 11 },{ 2, 5  },{ 3, 3  }},
	{ { 1, 12 }},
	{ { 1, 13 },{ 2, 6  }},
	{ { 1, 14 },{ 3, 4  }},
	{ { 1, 15 },{ 2, 7  },{ 4, 3  }},						// 10
	{ { 1, 15 },{ 2, 7  },{ 4, 3  }},
	{ { 2, 8  },{ 3, 5  }},
	{ { 2, 8  },{ 3, 5  }},
	{ { 2, 9  },{ 4, 4  },{ 5, 3  }},
	{ { 3, 6  }},											// 15
	{ { 2, 10 }},
	{ { 2, 10 }},
	{ { 2, 11 },{ 3, 7  },{ 4, 5  },{ 6, 3  }},
	{ { 5, 4  }},
	{ { 2, 12 }},											// 20
	{ { 3, 8  }},
	{ { 2, 13 },{ 4, 6  },{ 7, 3  }},
	{ { 2, 13 },{ 4, 6  },{ 7, 3  }},
	{ { 2, 14 },{ 3, 9  },{ 5, 5  },{ 6, 4  }},
	{ { 2, 14 },{ 3, 9  },{ 5, 5  },{ 6, 4  }},				// 25
	{ { 2, 15 },{ 4, 7  },{ 8, 3  }},
	{ { 3, 10 }},
	{ { 2, 16 }},
	{ { 5, 6  },{ 7, 4  }},
	{ { 2, 17 },{ 3, 11 },{ 4, 8  },{ 6, 5  },{ 9, 3  }},	// 30
	{ { 2, 17 },{ 3, 11 },{ 4, 8  },{ 6, 5  },{ 9, 3  }},
	{ { 2, 18 }},
	{ { 3, 12 }},
	{ { 2, 19 },{ 4, 9  },{ 5, 7  },{ 8, 4  },{10, 3  }},
	{ { 2, 19 },{ 4, 9  },{ 5, 7  },{ 8, 4  },{10, 3  }},	// 35
	{ { 2, 20 },{ 3, 13 },{ 6, 6  },{ 7, 5  }},
	{ { 2, 20 },{ 3, 13 },{ 6, 6  },{ 7, 5  }},
	{ { 2, 21 },{ 4, 10 },{11, 3  }},
	{ { 3, 14 },{ 5, 8  },{ 9, 4  }},
	{ { 2, 22 }},											// 40
	{ { 2, 22 }},
	{ { 2, 23 },{ 3, 15 },{ 4, 11 },{ 6, 7  },{ 8, 5  },{12, 3  }},
	{ { 7, 6  }},
	{ { 2, 24 },{ 5, 9  },{10, 4  }},
	{ { 3, 16 }},											// 45
	{ { 2, 25 },{ 4, 12 },{13, 3  }},
	{ { 2, 25 },{ 4, 12 },{13, 3  }},
	{ { 2, 26 },{ 3, 17 },{ 6, 8  },{ 9, 5  }},
	{ { 5, 10 },{11, 4  }},
	{ { 2, 27 },{ 4, 13 },{ 7, 7  },{ 8, 6  },{14, 3  }},	// 50
	{ { 3, 18 }},
	{ { 2, 28 }},
	{ { 2, 28 }},
	{ { 2, 29 },{ 3, 19 },{ 4, 14 },{ 5, 11 },{ 6, 9  },{10, 5  },{12, 4  },{15, 3  }},
	{ { 2, 29 },{ 3, 19 },{ 4, 14 },{ 5, 11 },{ 6, 9  },{10, 5  },{12, 4  },{15, 3  }},	// 55    YUCK!!!!!
	{ { 2, 30 }},
	{ { 3, 20 },{ 7, 8  },{ 9, 6  }},
	{ { 4, 15 },{ 8, 7  },{16, 3  }},
	{ { 5, 12 },{13, 4  }},
	{ { 3, 21 },{ 6, 10 },{11, 5  }},						// 60
	{ { 3, 21 },{ 6, 10 },{11, 5  }},
	{ { 4, 16 },{17, 3  }},
	{ { 3, 22 }},
	{ { 5, 13 },{ 7, 9  },{10, 6  },{14, 4  }},
	{ { 5, 13 },{ 7, 9  },{10, 6  },{14, 4  }},				// 65
	{ { 3, 23 },{ 4, 17 },{ 6, 11 },{ 8, 8  },{ 9, 7  },{12, 5  },{18, 3  }},
	{ { 3, 23 },{ 4, 17 },{ 6, 11 },{ 8, 8  },{ 9, 7  },{12, 5  },{18, 3  }},
	{ { 3, 24 },{ 5, 14 },{15, 4  }},
	{ { 3, 24 },{ 5, 14 },{15, 4  }},
	{ { 4, 18 },{19, 3  }},									// 70         GETTING BORED!!!!!!!!!!
	{ { 7, 10 },{11, 6  }},
	{ { 3, 25 },{ 6, 12 },{13, 5  }},
	{ { 3, 25 },{ 6, 12 },{13, 5  }},
	{ { 4, 19 },{ 5, 15 },{ 8, 9  },{10, 7  },{16, 4  },{20, 3  }},
	{ { 3, 26 },{ 9, 8  }},									// 75
	{ { 3, 26 },{ 9, 8  }},
	{ { 3, 27 },{ 4, 20 },{ 6, 13 },{ 7, 11 },{12, 6  },{14, 5  },{21, 3  }},
	{ { 3, 27 },{ 4, 20 },{ 6, 13 },{ 7, 11 },{12, 6  },{14, 5  },{21, 3  }},
	{ { 5, 16 },{17, 4  }},
	{ { 5, 16 },{17, 4  }},									// 80
	{ { 3, 28 }},
	{ { 4, 21 },{ 8, 10 },{11, 7  },{22, 3  }},
	{ { 4, 21 },{ 8, 10 },{11, 7  },{22, 3  }},
	{ { 3, 29 },{ 5, 17 },{ 6, 14 },{ 9, 9  },{10, 8  },{15, 5  },{18, 4  }},
	{ { 7, 12 },{13, 6  }},									// 85
	{ { 4, 22 },{23, 3  }},
	{ { 3, 30 }},
	{ { 3, 30 }},
	{ { 5, 18 },{19, 4  }},
	{ { 3, 31 },{ 4, 23 },{ 6, 15 },{ 8, 11 },{12, 7  },{16, 5  },{24, 3  }},	// 90
	{ { 3, 31 },{ 4, 23 },{ 6, 15 },{ 8, 11 },{12, 7  },{16, 5  },{24, 3  }},
	{ { 7, 13 },{14, 6  }},
	{ { 3, 32 },{ 9, 10 },{11, 8  }},
	{ { 4, 24 },{ 5, 19 },{10, 9  },{20, 4  },{25, 3  }},
	{ { 4, 24 },{ 5, 19 },{10, 9  },{20, 4  },{25, 3  }},	// 95
	{ { 3, 33 },{ 6, 16 },{17, 5  }},
	{ { 3, 33 },{ 6, 16 },{17, 5  }},
	{ { 4, 25 },{ 8, 12 },{13, 7  },{26, 3  }},
	{ { 3, 34 },{ 5, 20 },{ 7, 14 },{15, 6  },{21, 4  }},	// 99 + 0 = 100, strange eh?  blah, 3am sucks  4 am sucks more
};


/**************************************************************************/
static const int mob_balance_table[100][7] = {
// stored as 1d4+1hp, 90ac, 1d4+0damage
//
//	#hitdie, hitdietype, bonus, armour class, #damdie, damdietype, bonus
	{ 1,4,1,		 90,	1,4,0     },	// 1
	{ 1,4,2,		 80,	1,5,0     },
	{ 1,5,3,		 70,	1,6,0     },
	{ 2,4,1,		 60,	1,5,1     },
	{ 2,5,1,		 50,	1,6,1     },	// 5
	{ 2,5,4,		 40,	1,7,1     },
	{ 2,5,5,		 40,	1,8,1     },
	{ 2,5,8,		 30,	1,7,2     },
	{ 2,5,12,		 20,	1,8,2     },
	{ 2,5,18,		 10,	2,4,2     },	// 10
	{ 2,5,22,		 10,	1,10,2    },
	{ 2,5,45,		  0,	1,10,3    },
	{ 2,5,50,		-10,	2,5,3     },
	{ 2,5,55,		-10,	1,12,3    },
	{ 3,5,60,		-20,    2,6,3     },	// 15
	{ 3,5,67,		-20,    2,6,4     },
	{ 3,5,72,		-30,    3,4,4     },
	{ 3,5,79,		-30,    2,7,4     },
	{ 3,5,82,		-40,    2,7,5     },
	{ 3,5,88,		-40,    2,8,5     },	// 20
	{ 4,5,90,		-50,    4,4,5     },
	{ 4,5,95,		-50,    4,4,6     },
	{ 4,5,100,		-60,    3,6,6     },
	{ 4,5,105,		-60,    2,10,6    },
	{ 4,5,110,		-70,    2,10,7    },	// 25
	{ 4,5,115,		-70,    3,7,7     },
	{ 4,5,120,		-80,    5,4,7     },
	{ 5,5,125,		-80,    2,12,8    },
	{ 5,5,130,		-90,    2,12,8    },
	{ 5,5,140,		-90,    4,6,8     },	// 30
	{ 5,6,150,		-100,   4,6,9     },
	{ 5,6,160,		-100,   6,4,9     },
	{ 5,6,195,		-110,   6,4,10    },
	{ 5,6,205,		-110,   4,7,10    },
	{ 5,6,210,		-110,   4,7,11    },	// 35
	{ 5,6,215,		-120,   3,10,11   },
	{ 5,6,220,		-120,   3,10,12   },
	{ 5,6,225,		-130,   5,6,12    },
	{ 5,6,230,		-130,   5,6,13    },
	{ 5,6,235,		-130,   4,8,13    },	// 40
	{ 5,7,240,		-140,	4,8,14    },
	{ 5,7,242,		-140,	3,12,14   },
	{ 5,7,245,		-150,	3,12,15   },
	{ 5,7,247,		-150,	8,4,15    },
	{ 5,7,250,		-150,   8,4,16    },	// 45
	{ 5,8,255,		-160,   6,6,16    },
	{ 5,8,260,		-170,   6,6,17    },
	{ 5,8,265,		-180,   6,6,18    },
	{ 5,8,270,		-190,   4,10,18   },
	{ 5,9,275,		-200,   5,8,19    },	// 50
	{ 5,9,280,		-210,   5,8,20    },
	{ 5,9,285,		-220,   6,7,20    },
	{ 5,9,290,		-230,   6,7,21    },
	{ 5,9,295,		-240,   7,6,22    },
	{ 5,9,300,		-250,   10,4,23   },	// 55
	{ 6,9,300,		-260,   10,4,24   },
	{ 6,9,305,		-270,   6,8,24    },
	{ 6,9,310,		-280,   5,10,25   },
	{ 6,9,315,		-290,   8,6,26    },
	{ 6,9,320,		-300,   8,6,27    },	// 60
	{ 6,10,320,		-310,   8,6,28    },
	{ 6,10,325,		-320,   8,7,28    },
	{ 6,10,330,		-330,	8,7,29    },
	{ 6,10,335,		-340,	8,7,30    },
	{ 6,10,340,		-350,	7,8,32    },	// 65
	{ 7,10,340,		-360,   7,8,32    },
	{ 7,10,345,		-370,   7,8,33    },
	{ 7,10,350,		-380,   7,8,34    },
	{ 7,10,355,		-390,   8,8,33    },
	{ 7,10,360,		-400,	8,8,34    },	// 70
	{ 7,11,360,		-410,   8,8,35    },
	{ 7,11,365,		-420,   8,8,36    },	
	{ 7,11,370,		-430,   8,9,36    },
	{ 7,11,375,		-440,   8,9,37    },
	{ 7,11,380,		-450,   8,9,38    },	// 75
	{ 8,11,380,		-460,	8,9,39    },
	{ 8,11,385,		-470,   9,9,38    },
	{ 8,11,390,		-480,	9,9,39    },
	{ 8,11,395,		-490,	9,9,40    },
	{ 8,11,400,		-500,   9,9,41    },	// 80
	{ 8,12,400,		-510,   9,10,40   },
	{ 8,12,405,		-520,   9,10,41   },
	{ 8,12,410,		-530,   9,10,42   },
	{ 8,12,415,		-540,   9,10,43   },
	{ 8,12,420,		-550,   10,10,42  },	// 85
	{ 9,12,420,		-560,   10,10,43  },
	{ 9,12,425,		-570,   10,10,44  },
	{ 9,12,430,		-580,   10,10,45  },
	{ 9,12,435,		-590,   10,11,44  },
	{ 9,12,440,		-600,   10,11,45  },	// 90
	{ 9,13,440,		-610,   10,11,46  },
	{ 9,13,445,		-620,   10,11,47  },
	{ 9,13,450,		-630,   10,12,48  },
	{ 9,13,455,		-640,   10,12,49  },
	{ 9,13,460,		-650,   10,12,50  },	// 95
	{ 10,13,460,	-660,   10,12,51  },
	{ 10,13,465,	-670,   10,13,52  },
	{ 10,13,470,	-680,   10,13,53  },
	{ 10,13,475,	-690,   10,13,54  },
	{ 10,14,480,	-700,   10,13,55  }		// 100
};

/**************************************************************************/
//	The number for the AC_* is the value, based on obj->level / value
//  Higher Values = worse AC... MAKE SURE TO CHECK FOR DIVISION BY ZERO!!!

static const int armour_balance_table[MAX_WEAR][5] = {
//		ITEM_TYPE,		AC_PIERCE, AC_BASH, AC_SLASH, AC_MAGIC
	{	WEAR_LIGHT,			0,		0,			0,		0	},
	{	WEAR_FINGER_L,		10,		10,			10,		10	},
	{	WEAR_FINGER_R,		10,		10,			10,		10	},
	{	WEAR_NECK_1,		6,		6,			6,		8	},
	{	WEAR_NECK_2,		6,		6,			6,		8	},
	{	WEAR_TORSO,			3,		3,			3,		4	},
	{	WEAR_HEAD,			4,		4,			4,		5	},
	{	WEAR_LEGS,			4,		4,			4,		5	},
	{	WEAR_FEET,			5,		5,			5,		6	},
	{	WEAR_HANDS,			5,		5,			5,		6	},
	{	WEAR_ARMS,			4,		4,			4,		5	},
	{	WEAR_SHIELD,		3,		3,			3,		4	},
	{	WEAR_ABOUT,			4,		4,			4,		5	},
	{	WEAR_WAIST,			5,		5,			5,		7	},
	{	WEAR_WRIST_L,		7,		7,			7,		9	},
	{	WEAR_WRIST_R,		7,		7,			7,		9	},
	{	WEAR_WIELD,			0,		0,			0,		0	},
	{	WEAR_HOLD,			0,		0,			0,		0	},
	{	WEAR_FLOAT,			0,		0,			0,		0	},
	{	WEAR_SECONDARY,		0,		0,			0,		0	},
	{	WEAR_LODGED_ARM,	0,		0,			0,		0	},
	{	WEAR_LODGED_LEG,	0,		0,			0,		0	},
	{	WEAR_LODGED_RIB,	0,		0,			0,		0	},
	{	WEAR_SHEATHED,		0,		0,			0,		0	},
	{	WEAR_CONCEALED,		0,		0,			0,		0	},
	{	WEAR_EYES,			0,		0,			0,		0	},
	{	WEAR_EAR_L,			0,		0,			0,		0	},
	{	WEAR_EAR_R,			0,		0,			0,		0	},
	{	WEAR_FACE,			0,		0,			0,		0	},
	{	WEAR_ANKLE_L,		0,		0,			0,		0	},
	{	WEAR_ANKLE_R,		0,		0,			0,		0	},
	{   WEAR_BACK,		    3,		3,			3,		4   }
};

/**************************************************************************/
//  Used to return mob's HP dice (3d5+88)
//
//  result!!
int mob_balance_lookup( int level, int field )
{
	if ( field < 0 || field >= MOB_BALANCE_MAX )
		return 0;

	if ( level > 100 ) level = 100;
	if ( level < 0  ) level = 0;
	return ( mob_balance_table[level][field]);
}

/**************************************************************************/
// returns proper value of weapon_balance_table
int weapon_balance_lookup( int level, int pos, int die )
{
	// first off, return 1d4 for anything below level -2
	if ( level < -2 )
	{
		return ( die = 0 ? 1 : 4 );
	}

	// normalize values and bounds checking, just to be safe!!
	if ( pos   < 0	)		pos		= 0;
	if ( pos   > 7	)		pos		= 0;
	if ( level < 0	)		level	= 0;
	if ( level > 99	)		level	= 99;
	if ( die   < 0	)		die		= 0;
	if ( die   > 1	)		die		= 1;

	// all safe now, return a number
	return ( weapon_balance_table[level][pos][die]);
}

/**************************************************************************/
// Main balance command

void do_checkbalance( char_data *ch, char *argument )
{
	if ( !str_cmp( argument, "obj" ))
	{
		check_obj_balance( ch );
		return;
	}

	ch->println( "Syntax: checkbalance [obj]  more to follow." );
	return;
}

/**************************************************************************/
// Handles the objects, then goes to specified function per specific item_type
// this had to get modularized like this, otherwise it'd get too huge and
// cumbersome
void check_obj_balance( char_data *ch )
{
	int i;
	OBJ_INDEX_DATA *pObj;

	for ( i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++)
	{
		if (( pObj = get_obj_index( i )) == NULL )
			continue;

		switch ( pObj->item_type )
		{
		case ( ITEM_WEAPON ):
			check_obj_weapon( ch, pObj );
			break;
		default:
			break;
		}
	}
}

/**************************************************************************/
void check_obj_weapon( char_data *ch, OBJ_INDEX_DATA *pObj )
{
	bool balanced = false;			// always assume the worst
	int  vlevel   = pObj->level;	// virtual level in balance table
	int  i;
    AFFECT_DATA	  *paf;

	/*	start adjusting vlevel according to the balance sheet
	
		dagger				-8
		exotic				-5		-20 mana		+1
		whip				-3		+10 mana		-1
		flail				-2		-4  hp			+1
		sword/mace			 0		+2  hp			-1
		axe/2h staff		+3		-2  hit/dam		+1
		2h mace/2h sword	+6		+2  hit/dam		-1
		polearm				+8		+1  attribute	-1
		2h axes				+9		-1  attribute	+1

		sharp				-4		vorpal			-4
		vampiric			-3		flaming			-3
		freezing			-3		empowered		-3
		poison				-3		holy			-3

	*/

	if ( pObj->level > LEVEL_HERO ) {
		ch->printlnf( "`c%-5d`x..............`YSKIPPED`x Immortal Level", pObj->vnum );
		return;
	}

	// do weapon types first (easiest) :)
	switch (pObj->value[0])
	{
	case (WEAPON_DAGGER):		vlevel -=	8;		break;
	case (WEAPON_EXOTIC):		vlevel -=	5;		break;
	case (WEAPON_WHIP):			vlevel -=	3;		break;
	case (WEAPON_FLAIL):		vlevel -=	2;		break;
	case (WEAPON_STAFF):
		if ( !IS_WEAPON_STAT( pObj, WEAPON_TWO_HANDS ))
								vlevel -=	10;
		break;
	case (WEAPON_POLEARM):		vlevel +=	8;		break;
	case (WEAPON_SICKLE):
		if ( IS_WEAPON_STAT( pObj, WEAPON_TWO_HANDS ))	{
								vlevel +=	3;
		} else {
								vlevel -=	3;
		}
		break;
	case (WEAPON_SWORD):
        case (WEAPON_SPEAR):
	case (WEAPON_MACE):
		if ( IS_WEAPON_STAT( pObj, WEAPON_TWO_HANDS ))
								vlevel +=	6;
		break;
	case (WEAPON_AXE):
		if ( IS_WEAPON_STAT( pObj, WEAPON_TWO_HANDS ))	{
								vlevel +=	9;
		} else {
								vlevel +=	3;
		}
		break;
	default:
		break;
	}

	if ( IS_WEAPON_STAT( pObj, WEAPON_FLAMING	))		vlevel -= 3;
	if ( IS_WEAPON_STAT( pObj, WEAPON_FROST		))		vlevel -= 3;
	if ( IS_WEAPON_STAT( pObj, WEAPON_VAMPIRIC	))		vlevel -= 3;
	if ( IS_WEAPON_STAT( pObj, WEAPON_SHARP		))		vlevel -= 4;
	if ( IS_WEAPON_STAT( pObj, WEAPON_VORPAL	))		vlevel -= 4;
	if ( IS_WEAPON_STAT( pObj, WEAPON_SHOCKING	))		vlevel -= 3;
	if ( IS_WEAPON_STAT( pObj, WEAPON_POISON	))		vlevel -= 3;
	if ( IS_WEAPON_STAT( pObj, WEAPON_HOLY		))		vlevel -= 3;
//	if ( IS_WEAPON_STAT( pObj, WEAPON_LIVING	))		vlevel -= 0;    // VOT?


	for ( paf = pObj->affected; paf; paf = paf->next )
	{
		switch (paf->location)
		{
		case (APPLY_DAMROLL):
		case (APPLY_HITROLL):
														vlevel += paf->modifier * -(1/2);
			break;
		case (APPLY_MANA):
			if ( paf->modifier > 0 )					vlevel += paf->modifier * -(1/10);
			if ( paf->modifier < 0 )					vlevel += paf->modifier * -(1/20);
			break;
		case (APPLY_HIT):
			if ( paf->modifier > 0 )					vlevel += paf->modifier * -(1/2);
			if ( paf->modifier < 0 )					vlevel += paf->modifier * -(1/4);
			break;
		case (APPLY_AC):
			if ( paf->modifier > 0 )					vlevel += paf->modifier * -(2/10);
			if ( paf->modifier < 0 )					vlevel += paf->modifier * -(1/10);
			break;
		case (APPLY_SAVES):
			if ( paf->modifier > 0 )					vlevel += paf->modifier * -(4/10);
			if ( paf->modifier < 0 )					vlevel += paf->modifier * -(2/10);
			break;
		case (APPLY_ST):
		case (APPLY_QU):
		case (APPLY_PR):
		case (APPLY_EM):
		case (APPLY_IN):
		case (APPLY_CO):
		case (APPLY_AG):
		case (APPLY_SD):
		case (APPLY_ME):
		case (APPLY_RE):
														vlevel -= paf->modifier;
			break;
		default:
			break;
		}
	}

	for ( i=0; i < 8; i++ )		// 8 elements in weapon_balance_table
	{
		if ( weapon_balance_lookup( vlevel, i, 0 ) >= pObj->value[1] 
		&&   weapon_balance_lookup( vlevel, i, 1 ) >= pObj->value[2] )
		{
			balanced = true;
			break;
		}
	}

	if ( balanced ) {
		ch->printlnf( "`c%-5d`x..............`GPASSED`x", pObj->vnum );
	} else {
		ch->printlnf( "`c%-5d`x..............`RFAILED`x  Level: %-3d  Dam Level Suggested: %-3d",
			pObj->vnum, pObj->level, vlevel );
	}
	return;
	
}

/**************************************************************************/
/*
'but you can't use negative mods to just boost up the damage level.'
'you can only use negative mods to counter positive ones.'
'For example adding +20 saves to a weapon and nothing else would 
 have no affect on the damage roll.'
'Because if the weapon is chaos-laced the +20 saves would disappear, 
 leaving a really really nice weapon.'
Equip  Normal  1 Alliance  2 Alliance
 level           flag or    flag and
               magic flag  magic flag
0-10      0         0         1
11-20     1         1         2
21-30     2         3         4
31-40     3         4         5
41-50     4         5         6
51-60     5         6         7
61-70     6         7         8
71-80     7         8         9
81-90     8         9         10
91        9         10        11
Table 2:  Max modifier based on items lev and alliance restrictions
 
One Modifier Equals
+1 attribute
+2 hitpoints
+20 mana
+2 hitroll
+2 damage
-2 save
-5 AC
+40 movement
bless flag
*/
/**************************************************************************/
/**************************************************************************/

