/**************************************************************************/
// ictime.h - IC Time system - Kalahn & Kerenos 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef ICTIME_H
#define ICTIME_H

// Protoypes
void set_ictime();
void set_weather();
char *get_shorticdate_from_time(time_t tt, char * format, int icyears_added_to_result);
char *get_ictimediff(time_t t1, time_t t2, int icyears_added_to_result);

// SETTINGS
// ICTIME stuff
#define ICTIME_YEAR_OFFSET	(game_settings->icyear_offset)// calibrate year to game settings

#define ICTIME_MONTHS_PER_YEAR		12
#define ICTIME_WEEKS_PER_MONTH		3
#define ICTIME_DAYS_PER_WEEK		7
#define ICTIME_DAYS_PER_MONTH		(ICTIME_DAYS_PER_WEEK  * ICTIME_WEEKS_PER_MONTH)
#define ICTIME_HOURS_PER_DAY		24
#define ICTIME_MINUTES_PER_HOUR		60
#define ICTIME_DAYS_PER_YEAR		(ICTIME_DAYS_PER_MONTH * ICTIME_MONTHS_PER_YEAR)

#define ICTIME_IRLSECS_PER_MINUTE	6
#define ICTIME_IRLSECS_PER_HOUR	(ICTIME_IRLSECS_PER_MINUTE * ICTIME_MINUTES_PER_HOUR)
#define ICTIME_IRLSECS_PER_DAY	  (ICTIME_IRLSECS_PER_HOUR * ICTIME_HOURS_PER_DAY)
#define ICTIME_IRLSECS_PER_WEEK	   (ICTIME_IRLSECS_PER_DAY * ICTIME_DAYS_PER_WEEK)
#define ICTIME_IRLSECS_PER_MONTH  (ICTIME_IRLSECS_PER_WEEK * ICTIME_WEEKS_PER_MONTH)
#define ICTIME_IRLSECS_PER_YEAR	 (ICTIME_IRLSECS_PER_MONTH * ICTIME_MONTHS_PER_YEAR)

// Weather stuff.
#define HOUR_SUNRISE				((ICTIME_HOURS_PER_DAY / 4)-1)		//  5 o'clock
#define HOUR_DAY_BEGIN				(HOUR_SUNRISE + 1)					//  6 o'clock
#define HOUR_NOON					(ICTIME_HOURS_PER_DAY / 2)			// 12 o'clock
#define HOUR_SUNSET					(((ICTIME_HOURS_PER_DAY / 4) * 3)+1)// 19 o'clock
#define HOUR_NIGHT_BEGIN			(HOUR_SUNSET + 1)					// 20 o'clock
#define HOUR_MIDNIGHT				ICTIME_HOURS_PER_DAY				// 24 o'clock

#define WEATHER_SEASON_WINTER		0
#define WEATHER_SEASON_SPRING		1
#define WEATHER_SEASON_SUMMER		2
#define WEATHER_SEASON_AUTUMN		3
#define WEATHER_SEASON_MAX			4

#define TOTAL_WEATHER_CHANCE		1000
#define SKY_CLEAR_RANGE				(TOTAL_WEATHER_CHANCE / WEATHER_SEASON_MAX)
#define SKY_CLOUDY_RANGE			(TOTAL_WEATHER_CHANCE / WEATHER_SEASON_MAX)
#define SKY_RAINY_RANGE				(TOTAL_WEATHER_CHANCE / WEATHER_SEASON_MAX)
#define SKY_LIGHTNING_RANGE			(TOTAL_WEATHER_CHANCE / WEATHER_SEASON_MAX)

#define INFLUENCE_BASE				((TOTAL_WEATHER_CHANCE / WEATHER_SEASON_MAX) / 4)
#define INFLUENCE_P4				(INFLUENCE_BASE * 4)
#define INFLUENCE_P3				(INFLUENCE_BASE * 3)
#define INFLUENCE_P2				(INFLUENCE_BASE * 2)
#define INFLUENCE_P1				(INFLUENCE_BASE * 1)
#define INFLUENCE_NONE				0
#define INFLUENCE_M1				(INFLUENCE_BASE * -1)
#define INFLUENCE_M2				(INFLUENCE_BASE * -2)
#define INFLUENCE_M3				(INFLUENCE_BASE * -3)
#define INFLUENCE_M4				(INFLUENCE_BASE * -4)

#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3
#define	SKY_MAX						4

extern	const	struct	season_type		season_table	[WEATHER_SEASON_MAX];

#endif
