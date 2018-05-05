/**************************************************************************/
// areas.h - area.cpp header - Kalahn
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef AREAS_H
#define AREAS_H

// areas.cpp
void load_rooms_NAFF( FILE *fp, int version );
void load_objects_NAFF( FILE *fp, int version);
void load_mobiles_NAFF( FILE *fp, int version );
void apply_area_vnum_offset(int *old_vnum);
APPLOC translate_old_apply_number(int);
int reverse_translate_old_apply_number(APPLOC newvalue);
const char *pack_word(const char *word);
char *pack_string(const char *str); // not multibuffered
char *unpack_string(char *str);
void save_shops_NAFF( FILE *fp, AREA_DATA *pArea );
void load_shops_NAFF( FILE *fp );
void save_mudprogs_NAFF( FILE *fp, AREA_DATA *pArea );
void load_mudprogs_NAFF( FILE *fp );
AFFECT_DATA *fread_affect(FILE *fp);
void fwrite_affect(AFFECT_DATA *paf, FILE *fp);
void fwrite_affect_recursive(AFFECT_DATA *paf, FILE *fp);

// area import system - makes it easy to read in multiple mud area file formats
void areaimport_mobile_affects_stock( FILE *fp, int version, MOB_INDEX_DATA *pMobIndex);
void areaimport_mobile_affects_format2( FILE *fp, int version, MOB_INDEX_DATA *pMobIndex);
void areaimport_object_translate_flags_stock( FILE *fp, int version, OBJ_INDEX_DATA *pMobIndex);
void areaimport_object_translate_flags_format2( FILE *fp, int version, OBJ_INDEX_DATA *pMobIndex);
void areaimport_object_translate_flags_format3( FILE *fp, int version, OBJ_INDEX_DATA *pMobIndex);

void areaimport_room_flags_stock( FILE *fp, int version, ROOM_INDEX_DATA *pRoomIndex);
void areaimport_room_flags_format2( FILE *fp, int version, ROOM_INDEX_DATA *pMobIndex);

int areaimport_translate_wear_locations_stock( int old_wear_location, int area_version);
int areaimport_translate_wear_locations_format2( int old_wear_location, int area_version);
int areaimport_translate_wear_locations_format3( int old_wear_location, int area_version);

void areaimport_convert_credits(AREA_DATA *pArea);

void attach_resets();

void save_area_roominvitelist( AREA_DATA *pArea );
void load_area_roominvitelist( AREA_DATA *pArea );

#endif // AREAS_H
