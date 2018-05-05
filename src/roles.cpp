/**************************************************************************/
// roles.cpp - character stat roller, Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: roles.cpp - stat roling system - based heavily of the            *
 *                    rolemaster stat system.                              *
 ***************************************************************************/
#include "include.h"
#include "roles.h"

const struct role_type role_table[39] = {
//  low,high, 25, 39, 59, 74, 84, 89, 94, 97, 99,100
	{-2, -2, {25, 39, 59, 74, 84, 89, 94, 97, 99,100}},
    {1 , 10, {25, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, 
    {11, 20, {30, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, 
    {21, 30, {35, 39, -1, -1, -1, -1, -1, -1, -1, -1}}, 
    {31, 35, {38, 42, 59, -1, -1, -1, -1, -1, -1, -1}}, 
    {36, 40, {40, 45, 62, -1, -1, -1, -1, -1, -1, -1}}, 
    {41, 45, {42, 47, 64, -1, -1, -1, -1, -1, -1, -1}}, 
    {46, 49, {44, 49, 66, -1, -1, -1, -1, -1, -1, -1}}, 
    {50, 51, {46, 51, 68, -1, -1, -1, -1, -1, -1, -1}}, 
    {52, 53, {48, 53, 70, -1, -1, -1, -1, -1, -1, -1}}, 
    {54, 55, {50, 55, 71, -1, -1, -1, -1, -1, -1, -1}}, 
    {56, 57, {52, 57, 72, 74, 84, -1, -1, -1, -1, -1}}, 
    {58, 59, {54, 59, 73, 75, 85, -1, -1, -1, -1, -1}}, 
    {60, 61, {56, 61, 74, 76, 86, -1, -1, -1, -1, -1}}, 
    {62, 63, {58, 63, 75, 77, 87, -1, -1, -1, -1, -1}}, 
    {64, 65, {60, 65, 76, 78, 88, -1, -1, -1, -1, -1}}, 
    {66, 67, {62, 67, 77, 79, 88, 89, -1, -1, -1, -1}}, 
    {68, 69, {64, 69, 78, 80, 89, 89, -1, -1, -1, -1}}, 
    {70, 71, {66, 71, 79, 81, 89, 90, -1, -1, -1, -1}}, 
    {72, 73, {68, 73, 80, 82, 90, 90, -1, -1, -1, -1}}, 
    {74, 75, {70, 75, 81, 83, 90, 91, -1, -1, -1, -1}}, 
    {76, 77, {72, 77, 82, 84, 91, 91, -1, -1, -1, -1}}, 
    {78, 79, {74, 79, 83, 85, 91, 92, -1, -1, -1, -1}}, 
    {80, 81, {76, 81, 84, 86, 92, 92, -1, -1, -1, -1}}, 
    {82, 83, {78, 83, 85, 87, 92, 93, -1, -1, -1, -1}}, 
    {84, 85, {80, 85, 86, 88, 93, 93, 94, -1, -1, -1}}, 
    {86, 87, {82, 86, 87, 89, 93, 94, 94, -1, -1, -1}}, 
    {88, 89, {84, 87, 88, 90, 94, 94, 95, -1, -1, -1}}, 
    {90, 90, {86, 88, 89, 91, 94, 95, 95, 97, -1, -1}}, 
    {91, 91, {88, 89, 90, 92, 95, 95, 96, 97, -1, -1}}, 
    {92, 92, {90, 90, 91, 93, 95, 96, 96, 97, -1, -1}}, 
    {93, 93, {91, 91, 92, 94, 96, 96, 97, 98, -1, -1}}, 
    {94, 94, {92, 92, 93, 95, 96, 97, 97, 98, 99, -1}}, 
    {95, 95, {93, 93, 94, 96, 97, 97, 98, 98, 99, -1}}, 
    {96, 96, {94, 94, 95, 97, 97, 98, 98, 99, 99, -1}}, 
    {97, 97, {95, 95, 96, 97, 98, 98, 99, 99, 99, -1}}, 
    {98, 98, {96, 96, 97, 98, 98, 99, 99, 99,100, -1}}, 
    {99, 99, {97, 97, 98, 98, 99, 99,100,100,100, -1}}, 
    {100,100,{98, 98, 99, 99, 99,100,100,100,100,101}}
};

/**************************************************************************/
int role_table_lookup(int first_role){
	int second_roll;
	int vert_index, horizontal_index;

	// find where first_role fits horizontally in the table 
	for(horizontal_index=0;horizontal_index<10;horizontal_index++){
		if (first_role<= role_table[0].table[horizontal_index]){
			break;
		}
	}
	
	second_roll = number_range(1,100);
	// find where the second roll fits vertically in the table
	for(vert_index=1;vert_index<39;vert_index++){
		if (second_roll<= role_table[vert_index].high){
			break;
		}
	}

#ifdef STATS_SYSTEM_DEBUG
	logf("rtl> f=%2d, s=%2d, r=%2d",first_role, second_roll,
		UMAX(role_table[vert_index].table[horizontal_index],first_role));
#endif
	return (UMAX(role_table[vert_index].table[horizontal_index], first_role));
}


/**************************************************************************/
// modifies the rolled_stats pointer to a rolled set of stats
// using the rolemaster stat generation system
void gen_rolemaster_stats(attributes_set *rolled_stats, int bias_against){
	int rolling_stat, roll_out_of, total, loop, i;
	int highstats;

#ifdef STATS_SYSTEM_DEBUG
	logf("gen_rolemaster_stats() test - bias = %d ", 
		bias_against);
#endif

	loop=0;
    do{
		loop++;
		for (rolling_stat=0; rolling_stat<10; rolling_stat++){
			// generate the first part of the stat
			do{
				if(GAMESETTING(GAMESET_ROLE_ROLEMASTER_STATS)){
					roll_out_of=100;
				}else{					
					roll_out_of=number_range(UMIN(game_settings->roll_min_total/5, 100),
											UMIN(game_settings->roll_max_total/10, 100));
					roll_out_of=number_range(URANGE(10,game_settings->roll_min_total/7, 70),	
												roll_out_of);
				}

				if(!GAMESETTING(GAMESET_NO_BIAS_AGAINST_REROLLING)){
					if(bias_against>5){
						roll_out_of-=(bias_against*3);
						roll_out_of=UMAX(roll_out_of,60);
					}
				}

				rolled_stats->perm[rolling_stat]=number_range(1,roll_out_of);			
			}while (rolled_stats->perm[rolling_stat]<20);

			rolled_stats->potential[rolling_stat]=
				role_table_lookup(rolled_stats->perm[rolling_stat]);	
		}
        total=0; 
		highstats=0;
        for(i=0; i<MAX_STATS; i++)
        {
            total+=rolled_stats->potential[i];
            if(rolled_stats->potential[i]>80)
            {
                total+=10;
            }
            if(rolled_stats->potential[i]>90)
            {
				highstats++;
                total+=5;
            }
        }
		total+=highstats*highstats*4;

		if(GAMESETTINGMF1(GAMESETMF1_ENSURE_HIGH_STRENGTH_ROLLS)){
			if(rolled_stats->potential[STAT_ST]<80){
				// force a reroll because strength wasn't high enough
				total=-1;
			}
		}

    }while( (   total<game_settings->roll_min_total
		     || total> game_settings->roll_max_total)
			&& loop<100);


	if (loop>99)
	{
		logf("gen_rolemaster_stats(): Reroll looped %d times! bias_against = %d", 
			loop, bias_against);

		char buf[MSL];
		sprintf(buf,"Rolling char - warning, if we get lots of these, "
			"there might be a bug in stat rolling - unverified stats were allowed thru!");
		autonote(NOTE_SNOTE, "p_anote()","Reroll looped =100 times!", 
			"admin", buf, true);
	}


}
/**************************************************************************/
