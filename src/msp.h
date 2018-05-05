/**************************************************************************/
// msp.h - msp support header
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef MSP_H
#define MSP_H

enum MSP_TYPES {MSPT_ACTION, MSPT_COMBAT, MSPT_MUDPROG, MSPT_ROOM,  
				MSPT_SKILL, MSPT_SPELL, MSPT_WEATHER, MSPT_MAX_TYPE};

void msp_to_room(MSP_TYPES type, const char *filename, 
					int volume, // 1 ->100, if 0 default for type vol used 
					char_data *target,
					bool target_only_in_room,
					bool surrounding_rooms);

void msp_skill_sound(char_data *target, int sn);
void msp_update_char(char_data *ch);

// *** MSP SOUNDS
// combat related
#define MSP_SOUND_TRIP			"trip.wav"
#define MSP_SOUND_BACKSTAB		"backstab.wav"
// action related 
#define MSP_SOUND_QUAFF			"quaff.wav"
#define MSP_SOUND_EXPLOSION		"explosion.wav"
#define MSP_SOUND_FLEE			"flee.wav"

#define MSP_SOUND_CLOSE_DOOR	"close_door.wav"
#define MSP_SOUND_OPEN_DOOR		"open_door.wav"
#define MSP_SOUND_UNLOCK		"unlock.wav"
#define MSP_SOUND_LOCK			"lock.wav"
#define MSP_SOUND_HUH			"huh.wav"
#define MSP_SOUND_NOHELP		"nohelp.wav"
#define MSP_SOUND_SOUNDON		"soundon.wav"
#define MSP_SOUND_RESTORE		"restore.wav"
#define MSP_SOUND_SHUTDOWN		"shutdown.wav"

// weather related
#define MSP_SOUND_THUNDER		"thunder.wav"

#endif // MSP_H

