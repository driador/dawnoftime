/**************************************************************************/
// lookup.cpp - most of the lookup commands
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with all the licenses *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************
 * >> Original Diku Mud copyright (c)1990, 1991 by Sebastian Hammer,       *
 *    Michael Seifert, Hans Henrik Staerfeldt, Tom Madsen, & Katja Nyboe.  *
 * >> Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael       *
 *    Chastain, Michael Quan, and Mitchell Tse.                            *
 * >> ROM 2.4 is copyright 1993-1995 Russ Taylor and has been brought to   *
 *    you by the ROM consortium: Russ Taylor(rtaylor@pacinfo.com),         *
 *    Gabrielle Taylor(gtaylor@pacinfo.com) & Brian Moore(rom@rom.efn.org) *
 * >> Oblivion 1.2 is copyright 1996 Wes Wagner                            *
 **************************************************************************/

#include "include.h" // dawn standard includes
#include "ictime.h"

/**************************************************************************/
int position_lookup (const char *name)
{
   int pos;

   for (pos = 0; position_table[pos].name != NULL; pos++)
   {
    if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
        && !str_prefix(name,position_table[pos].name))
	    return pos;
   }
   
   return -1;
}

/**************************************************************************/
int sex_lookup (const char *name)
{
	int sex;
	
	for (sex = 0; sex_table[sex].name != NULL; sex++)
	{
		if (LOWER(name[0]) == LOWER(sex_table[sex].name[0])
			&&  !str_prefix(name,sex_table[sex].name))
			return sex;
	}
	
	return -1;
}

/**************************************************************************/
int size_lookup (const char *name)
{
   int size;
 
   for ( size = 0; size_table[size].name != NULL; size++)
   {
        if (LOWER(name[0]) == LOWER(size_table[size].name[0])
        &&  !str_prefix( name,size_table[size].name))
            return size;
   }
 
   return -1;
}
/**************************************************************************/
// returns race number - doing exact matching
int race_exact_lookup(const char *name)
{
	int race;
	// do an exact lookup on a race name
	for ( race = 0; race_table[race]; race++)
	{
		if(!str_cmp( name, race_table[race]->name))
			return race;
	}
	return -1;
}

/**************************************************************************/
// returns race number 
int race_lookup(const char *name)
{
	int race;
	// first attempt exact match 
	race=race_exact_lookup(name);
	if(race>=0){
		return race;
	}
	
	// now attempt a prefix match
	for ( race = 0; race_table[race]; race++)
	{
		if (LOWER(name[0]) == LOWER(race_table[race]->name[0])
			&&  !str_prefix( name,race_table[race]->name))
			return race;
	}
	
	return -1;
} 
/**************************************************************************/
// returns race number 
int pcrace_lookup(const char *name)
{
	int race;
	
	// first attempt exact match 
	for ( race = 0; race_table[race]; race++)
	{
		if(!race_table[race]->pc_race())
			continue;
		if (LOWER(name[0]) == LOWER(race_table[race]->name[0])
			&&  !str_cmp( name,race_table[race]->name))
			return race;
	}
	
	// now attempt a prefix match
	for ( race = 0; race_table[race]; race++)
	{
		if(!race_table[race]->pc_race()){
			continue;
		}
		if(!str_prefix( name,race_table[race]->name)){
			return race;
		}
	}
	
	return -1;
} 
/**************************************************************************/
// returns class index number using an exact match
int class_exact_lookup(const char *name)
{
	int classIndex;
	
	if (IS_NULLSTR(name)){
		return -1;
	}
	
	// automatically convert the name on read in
	if(GAMESETTING5(GAMESET5_TRANSLATE_ROGUE_CLASS_TO_THIEF) && !str_cmp(name, "rogue")){
		name="thief";
	}	
	
	// first attempt exact match 
	for ( classIndex = 0; !IS_NULLSTR(class_table[classIndex].name); classIndex++){	
		if(!str_cmp( name,class_table[classIndex].name)){
			return classIndex;
		}
	}
	return -1;
} 
/**************************************************************************/
// returns class index number
int class_lookup(const char *name)
{
   int classIndex;

   if (IS_NULLSTR(name)){
	   return -1;
   }

   // first attempt exact match 
   classIndex=class_exact_lookup(name);   
   if(classIndex>-1){
		return classIndex;
   }

   // now attempt a prefix match
   for ( classIndex = 0; !IS_NULLSTR(class_table[classIndex].name); classIndex++){	
	   if(!str_prefix( name,class_table[classIndex].name)){
			return classIndex;
	   }
   }

   return -1;
} 
/**************************************************************************/
int liq_lookup (const char *name)
{
    int liq;

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
	&& !str_prefix(name,liq_table[liq].liq_name))
	    return liq;
    }

    return -1;
}

/**************************************************************************/
int attack_lookup(const char *name)
{
    int att;

    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
	&&  !str_prefix(name,attack_table[att].name))
	    return att;
    }

    return 0;
}
/**************************************************************************/
int attack_lookup_with_error(const char *name)
{
    int att;

    for ( att = 0; attack_table[att].name != NULL; att++)
    {
		if(!str_prefix(name,attack_table[att].name)){
			return att;
		}
    }

    return -1;
}
/**************************************************************************/
// returns a flag for wiznet 
int wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
	&& !str_prefix(name,wiznet_table[flag].name))
	    return flag;
    }

    return -1;
}
/**************************************************************************/
/**************************************************************************/
// returns a pointer to the area data of the area which name matches 
// exactly the name supplied.  returns NULL if no area was found.
AREA_DATA * area_lookup( char *name)
{
    AREA_DATA *pArea;

	for ( pArea = area_first; pArea; pArea = pArea->next)
    {
		if (!str_cmp( name, pArea->name))
			return pArea;			
	}
    return NULL;
}
/**************************************************************************/
int sector_lookup( const char *name)
{
    int sect;

    for ( sect = 0; sect < SECT_MAX; sect++)
    {
		if ( LOWER( name[0] ) == LOWER( sect_table[sect].name[0])
		&&	!str_prefix( name, sect_table[sect].name ))
	    return sect;
    }
	return -1;
}
/**************************************************************************/
int dir_lookup( char *dir )
{
	if      ( !str_cmp( dir, "n" )	|| !str_cmp( dir, "north" ))		return 0;
	else if ( !str_cmp( dir, "e" )	|| !str_cmp( dir, "east" ))			return 1;
	else if ( !str_cmp( dir, "s" )	|| !str_cmp( dir, "south" ))		return 2;
	else if ( !str_cmp( dir, "w" )	|| !str_cmp( dir, "west" ))			return 3;
	else if ( !str_cmp( dir, "u" )	|| !str_cmp( dir, "up" ))			return 4;
	else if ( !str_cmp( dir, "d" )	|| !str_cmp( dir, "down" ))			return 5;
	else if ( !str_cmp( dir, "ne" ) || !str_cmp( dir, "northeast" ))	return 6;
	else if ( !str_cmp( dir, "se" ) || !str_cmp( dir, "southeast" ))	return 7;
	else if ( !str_cmp( dir, "sw" ) || !str_cmp( dir, "southwest" ))	return 8;
	else if ( !str_cmp( dir, "nw" ) || !str_cmp( dir, "northwest" ))	return 9;

	return -1;
}
/**************************************************************************/
int stat_lookup( char *stat )
{
	int mod;

	for ( mod = 0; stat_flags[mod].name != NULL; mod++ )
	{
		if (LOWER( stat[0]) == LOWER( stat_flags[mod].name[0])
			&& !str_prefix( stat, stat_flags[mod].name))
	    return mod;
    }

    return -1;
}
/**************************************************************************/
continent_type *continent_exact_lookup(const char *name)
{
	continent_type*c;

	for ( c=continent_list; c; c=c->next){
		if( !str_cmp( name, c->name)){
			return c;
		}
	}
	
	return NULL;
}
/**************************************************************************/
continent_type *continent_lookup(const char *name)
{
	continent_type*c;

	// perform an exact lookup of continents first
	c=continent_exact_lookup(name);
	if(c!=NULL){
		return c;
	}
	for ( c=continent_list; c; c=c->next){
		if( !str_prefix( name, c->name)){
			return c;
		}
	}
	
	return NULL;
}
/**************************************************************************/
int continent_count(continent_type *cont)
{
	if(cont){	
		int count=0;
		continent_type*c;
		for ( c=continent_list; c; c=c->next){
			count++;
			if(c==cont){
				return count;
			}
		}
	}
	
	return 0;
}

/**************************************************************************/
int season_lookup( const char *name )
{
	int seas;

	for ( seas = 0; seas < WEATHER_SEASON_MAX; seas++ )
	{
		if ( LOWER( name[0] ) == LOWER( season_table[seas].name[0] )
			&& !str_prefix( name, season_table[seas].name ))
			return seas;
	}
	
	return -1;
}

/**************************************************************************/
int time_lookup( const char *name )
{
	int time;

	for ( time = 0; time < TIME_MAX; time++ )
	{
		if ( LOWER( name[0] ) == LOWER( timefield_table[time].name[0] )
			&& !str_prefix( name, timefield_table[time].name ))
			return time;
	}
	
	return -1;
}

/**************************************************************************/
int difficulty_lookup( const char *name )
{
	int mod;

	for ( mod = 0; mod < DIFF_MAX; mod++ )
	{
		if ( LOWER( name[0] ) == LOWER( modifier_table[mod].name[0] )
			&& !str_prefix( name, modifier_table[mod].name ))
			return mod;
	}
	
	return -1;
}

/**************************************************************************/
int mixtype_lookup (const char *name)
{
   int mix;
 
   for ( mix = 0; mixtype_types[mix].name != NULL; mix++ )
   {
        if (LOWER(name[0]) == LOWER( mixtype_types[mix].name[0])
        &&  !str_prefix( name, mixtype_types[mix].name))
            return mix;
   }
 
   return -1;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
