/**************************************************************************/
// ictime.cpp - IC Time system - Kalahn & Kerenos 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "ictime.h"

/**************************************************************************/
void init_weather();
/***************************************************************************/
const struct season_type	season_table	[WEATHER_SEASON_MAX] =
{
	{ WEATHER_SEASON_WINTER,	"winter"	},
	{ WEATHER_SEASON_SPRING,	"spring"	},
	{ WEATHER_SEASON_SUMMER,	"summer"	},
	{ WEATHER_SEASON_AUTUMN,	"autumn"	}
};
/**************************************************************************/
void set_ictime()
{
	time_info.minute = (int)current_time/ICTIME_IRLSECS_PER_MINUTE;
	time_info.minute%= ICTIME_MINUTES_PER_HOUR;
	
	time_info.hour = (int)current_time/ICTIME_IRLSECS_PER_HOUR;
	time_info.hour%= ICTIME_HOURS_PER_DAY;

	time_info.day = (int)current_time/ICTIME_IRLSECS_PER_DAY;
	time_info.day%= ICTIME_DAYS_PER_MONTH;

	time_info.month	= (int)current_time/ICTIME_IRLSECS_PER_MONTH;
	time_info.month%= ICTIME_MONTHS_PER_YEAR;

	time_info.year = (int)current_time/ICTIME_IRLSECS_PER_YEAR;
	time_info.year+=ICTIME_YEAR_OFFSET; // calibrate year for dawns ic time
	char buf[MIL];
	sprintf(buf,"%s",(char*)ctime((time_t*)&current_time));
	buf[str_len(buf)-1]='\0';
	logf("set_ictime(): current_time=%s (%d)", buf, (int)current_time);
	logf("- IC results: min=%d, hour=%d, day=%d, month=%d, year=%d",
		time_info.minute,
		time_info.hour,
		time_info.day,	
		time_info.month,
		time_info.year);

/*	{ // log it to confirm it is working correctly
		char tbuf[MSL];
		sprintf(tbuf, "%s (%d) -> %2d:%02d  %2d/%02d/%4d",
			buf, 
			(int)current_time, 
			time_info.hour,
			time_info.minute,
			time_info.day,	
			time_info.month,
			time_info.year);
		append_string_to_file( "ictime.log", tbuf);
	}
*/
}
/**************************************************************************/
void do_setage(char_data *ch, char *argument)
{
    if (IS_NPC(ch)){
		ch->println("Players only.");
		return;
	}

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println("Not going to happen.");
		return;
	}



	if(ch->pcdata->birthdate && !GAMESETTING(GAMESET_ALWAYS_ALLOW_SETAGE)){
		ch->println("Your age as already been set.");
		return;
	}

	char		arg[MIL], arg1[MIL];
	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg1 );

	if ( IS_NULLSTR(arg1) || str_cmp("confirm", arg1)) {
		if(!codehelp(ch, "do_setage_noargument", false)){
			ch->println("You can set your age once and once only using the setage command.");
			ch->println("  `=Csetage <age> confirm`x." );
			ch->println("`RBE WARNED: `xYou age can only be set once and it can not be changed once set.");
		};
		return;
	}
	
	int value=atoi(arg);

    if ( value < 4 )
    {
        ch->println("Age must be 4 or greater.");
        return;
    }
    if ( value > 1000 )
    {
        ch->println("Age must be less than 1000.");
        return;
    }
	ch->pcdata->birthdate= current_time+ number_range(-ICTIME_IRLSECS_PER_YEAR,ICTIME_IRLSECS_PER_YEAR);
	ch->pcdata->birthyear_modifier=-value;
	ch->pcdata->birthyear_modifier++;
	ch->pcdata->birthdate-=ICTIME_IRLSECS_PER_YEAR;
	ch->println("Age set - see score to see your new age.");
}
/**************************************************************************/
char *get_ictimediff(time_t t1, time_t t2, int icyears_added_to_result)
{
    static char timebuf[3][MSL];
    static int index; 
    int dsec, icyear, icmonth, icweek, icday;

    ++index%=3;

    // initialise working string
    timebuf[index][0]=0;

    dsec   = abs((int)(t1-t2)); // number of IRL seconds between the 2 times
	icyear = dsec/ICTIME_IRLSECS_PER_YEAR;
	icyear+= icyears_added_to_result;
	dsec  %= ICTIME_IRLSECS_PER_YEAR;
    icmonth= dsec/ICTIME_IRLSECS_PER_MONTH;
	dsec  %= ICTIME_IRLSECS_PER_MONTH;
    icweek = dsec/ICTIME_IRLSECS_PER_WEEK;
	dsec  %= ICTIME_IRLSECS_PER_WEEK;
	icday  = dsec/ICTIME_IRLSECS_PER_DAY;	

    sprintf(timebuf[index],"%d year%s, %d month%s, %d week%s, %d day%s",
		icyear, 
		(icyear!=1?"s":""),
		icmonth, 
		(icmonth!=1?"s":""),
		icweek, 
		(icweek!=1?"s":""),
		icday, 
		(icday!=1?"s":""));

    return(timebuf[index]);
}
/**************************************************************************/
char *get_shorticdate_from_time(time_t tt, char * format, int icyears_added_to_result)
{
	static char buf[MIL];

	time_info_data tm;
	tm.day = (int)tt/ICTIME_IRLSECS_PER_DAY;
	tm.day%= ICTIME_DAYS_PER_MONTH;
	tm.month	= (int)tt/ICTIME_IRLSECS_PER_MONTH;
	tm.month%= ICTIME_MONTHS_PER_YEAR;
	tm.year = (int)tt/ICTIME_IRLSECS_PER_YEAR;
	tm.year+=ICTIME_YEAR_OFFSET+icyears_added_to_result; // calibrate year for dawns ic time
	
	// budget system to allow for customizing the format
	if(count_char(format, '%')!=3 || count_char(format, 'd')!=3){
		sprintf(buf, "%d/%d/%d", tm.day+1, tm.month+1, tm.year);
	}else{
		sprintf(buf, format, tm.day+1, tm.month+1, tm.year);
	}

	return buf;
}
/**************************************************************************/
int get_birthmonth( char_data *ch )
{
	int month = -1;
	
	if ( ch->pcdata->birthdate )
	{
		month = (int)ch->pcdata->birthdate/ICTIME_IRLSECS_PER_MONTH;
		month%= ICTIME_MONTHS_PER_YEAR;
	}

	return month;
}
/**************************************************************************/
void set_weather()
{
	init_weather();

	int sect;
	for ( sect = 0; sect < SECT_MAX; sect++ )
	{
		if ( time_info.hour <  5 ) weather_info[sect].sunlight = SUN_DARK;
		else if ( time_info.hour <  6 ) weather_info[sect].sunlight = SUN_RISE;
		else if ( time_info.hour < 19 ) weather_info[sect].sunlight = SUN_LIGHT;
		else if ( time_info.hour < 20 ) weather_info[sect].sunlight = SUN_SET;
		else                            weather_info[sect].sunlight = SUN_DARK;
		
		weather_info[sect].change     = 0;
		weather_info[sect].mmhg       = 960;
		if ( time_info.month >= 7 && time_info.month <=12 )
			weather_info[sect].mmhg += number_range( 1, 50 );
		else
			weather_info[sect].mmhg += number_range( 20, 80 );
		
		if ( weather_info[sect].mmhg <=  980 )		weather_info[sect].sky = SKY_LIGHTNING;
		else if ( weather_info[sect].mmhg <= 1000 ) weather_info[sect].sky = SKY_RAINING;
		else if ( weather_info[sect].mmhg <= 1020 ) weather_info[sect].sky = SKY_CLOUDY;
		else										weather_info[sect].sky = SKY_CLOUDLESS;
	}
}
/**************************************************************************/
