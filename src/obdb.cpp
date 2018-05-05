/**************************************************************************/
// obdb.cpp - reads in area files etc
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

#include "areas.h"
#include "db.h"
#include "clan.h"
#include "olc.h"

extern int flag_lookup args((const char *name, const struct flag_type *flag_table));

void apply_area_vnum_offset(int *old_vnum);
/**************************************************************************/
// Snarf a room section.
void load_rooms( FILE *fp, int version )
{
	ROOM_INDEX_DATA *pRoomIndex;
	
	if ( area_last == NULL )
	{
		bug("Load_rooms: no #AREA seen yet.");
		exit_error( 1 , "load_rooms", "no #AREA seen yet");
	}
	
	for ( ; ; )
	{
		vn_int vnum;
		char letter;
		int door;
		int iHash;
		
		letter				= fread_letter( fp );
		if ( letter != '#' )
		{
			bug("Load_rooms: # not found.");
			exit_error( 1 , "load_rooms", "# not found");
		}
		
		vnum = fread_number( fp );
		if ( vnum == 0 )
			break;
		vnum+= area_last->vnum_offset; 
		
		// check for a duplicate room number
		fBootDb = false;
		ROOM_INDEX_DATA *dup_room_index=get_room_index( vnum );
		if ( dup_room_index)
		{
			bugf( "Load_rooms: vnum %d duplicated with room from areafile '%s'.", 
				vnum, dup_room_index->area?dup_room_index->area->file_name:"unknown");
			exit_error( 1 , "load_rooms", "Duplicate vnum");
		}
		fBootDb = true;
		
		last_vnum = vnum; // backup the last vnum for gdb use
		
		pRoomIndex				= new_room_index();
		pRoomIndex->area		= area_last;
		pRoomIndex->vnum		= vnum;
		pRoomIndex->name		= fread_string( fp );
		pRoomIndex->description	= fread_string( fp );
		
		if (str_len(pRoomIndex->description)>MSL-4)
		{
			bugf("load_rooms: description of room %d is %d characters!!! (more than %d)",
				(int)last_vnum, str_len(pRoomIndex->description), MSL-4);
			exit_error( 1 , "load_rooms", "too many characters in room description");
		}
		
		fread_number( fp ); // just a 0 - padding, not really used
		pRoomIndex->room_flags		= fread_flag( fp );

		if(AREA_IMPORT_FORMAT(AIF_FORMAT3)){
			fread_number( fp ); // read in a room affects flag - not used on dawn
		}

		pRoomIndex->sector_type		= fread_number( fp );

		if(AREA_IMPORT_FORMAT(AIF_FORMAT2)){
			areaimport_room_flags_format2(fp, version, pRoomIndex);
		}else{
			// AIF_STOCK - the default
			areaimport_room_flags_stock(fp, version, pRoomIndex);
		}
		
		pRoomIndex->light		= 0;
		for ( door = 0; door < MAX_DIR; door++ ){
			pRoomIndex->exit[door] = NULL;
		}
		
		// defaults
		pRoomIndex->heal_rate = 100;
		pRoomIndex->mana_rate = 100;
		
		// check vnum fits in area vnum range 
		if ( vnum < pRoomIndex->area->min_vnum + pRoomIndex->area->vnum_offset)
		{
			bugf("Room with Vnum %d is less than area %s <%s> vnum %d!",
				vnum,
				pRoomIndex->area->name,
				pRoomIndex->area->file_name,
				pRoomIndex->area->min_vnum
				);
		}
		if ( vnum > pRoomIndex->area->max_vnum + pRoomIndex->area->vnum_offset)
		{
			bugf("Room with Vnum %d is greater than area %s <%s> vnum %d!",
				vnum,
				pRoomIndex->area->name,
				pRoomIndex->area->file_name,
				pRoomIndex->area->max_vnum
				);
		}
		
		for ( ; ; )
		{
			letter = fread_letter( fp );
			
			if ( letter == 'S' )
				break;
			
			if ( letter == 'H'){ // healing room 
				pRoomIndex->heal_rate = fread_number(fp);
			
			}else if ( letter == 'M'){ // mana room 
				pRoomIndex->mana_rate = fread_number(fp);
			
			}else if ( letter == 'C'){ // clan 
				if (pRoomIndex->clan)
				{
					bug("Load_rooms: duplicate clan fields.");
					exit_error( 1 , "load_rooms", "duplicate clan fields");
				}
				pRoomIndex->clan = clan_slookup(fread_string(fp));
			}else if ( AREA_IMPORT_FORMAT(AIF_FORMAT3) &&
					   (letter=='c' || letter=='s' || letter=='R') )
			{	// AREA_IMPORT_FORMAT3 only stuff
				if(letter == 'c' ){
					// format3 class restriction system
					fread_string(fp);
				}else if(letter == 's' ){
					// format3 sex based restriction system 
					fread_number(fp);
				}else if(letter == 'R' ){
					// format3 race based restriction system 
					fread_string(fp);
				}
			}else if ( UPPER(letter) == 'X' ){ 
				// MSP Room sound
				pRoomIndex->msp_sound	= fread_string( fp );
			}else if (UPPER(letter) == 'D'){
				EXIT_DATA *pexit;
				
				door = fread_number( fp );
				if ( door < 0 || door >=MAX_DIR )
				{
					bugf( "load_rooms: vnum %d has bad door number.", vnum );
					exit_error( 1 , "load_rooms", "bad door numbers");
				}
				
				pexit				= (EXIT_DATA *)alloc_perm( sizeof(*pexit) );
				pexit->description	= fread_string( fp );
				pexit->keyword		= fread_string( fp );
				if(version==1){
					int locks= fread_number( fp );
					switch ( locks )
					{
					case 1: pexit->rs_flags = EX_ISDOOR;                break;
					case 2: pexit->rs_flags = EX_ISDOOR | EX_PICKPROOF; break;
					case 3: pexit->rs_flags = EX_ISDOOR | EX_NOPASS;    break;
					case 4: pexit->rs_flags = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
						break;
					}
				}else{
					pexit->rs_flags		= fread_flag( fp );
				}
				pexit->key			= fread_number( fp );
				pexit->u1.vnum		= fread_number( fp );
				apply_area_vnum_offset(&pexit->u1.vnum);
				
				pexit->exit_info=pexit->rs_flags;
				pRoomIndex->exit[door]	= pexit;
				top_exit++;
			}
			else if ( UPPER(letter) == 'E' )
			{
				EXTRA_DESCR_DATA *ed;
				
				ed			= (EXTRA_DESCR_DATA *)alloc_perm( sizeof(*ed) );
				ed->keyword		= fread_string( fp );
				ed->description		= fread_string( fp );
				ed->next		= pRoomIndex->extra_descr;
				pRoomIndex->extra_descr	= ed;
				top_ed++;
			}
			
			else if (UPPER(letter) == 'O')
			{
				if (pRoomIndex->owner[0] != '\0')
				{
					bug("Load_rooms: duplicate owner.");
					exit_error( 1 , "load_rooms", "duplicate owner");
				}
				
				pRoomIndex->owner = fread_string(fp);
			}
			
			else if ( letter == 'F' )
            {
                room_echo_data *re= new_room_echo();
                re->firsthour	= fread_number( fp );
                re->lasthour	= fread_number( fp );
                int lowroll		= fread_number( fp );
				int highroll	= fread_number( fp );
				re->percentage	= UMAX(highroll,lowroll)-UMIN(highroll,lowroll);
                re->echotext	= fread_string( fp );
                re->next		= pRoomIndex->echoes;
                pRoomIndex->echoes= re;
            }
			else
			{
				bugf( "Load_rooms: vnum %d has flag '%c' which is not 'DESOF'.", vnum, letter );
				exit_error( 1 , "load_rooms", "invalid flag");
			}
		}
		
		iHash			= vnum % MAX_KEY_HASH;
		pRoomIndex->next	= room_index_hash[iHash];
		room_index_hash[iHash]	= pRoomIndex;
		top_room++;
		top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;
		assign_area_vnum( vnum );		
    }	
	return;
}

/**************************************************************************/
void load_object_values( FILE *fp, int version, OBJ_INDEX_DATA *pObjIndex)
{
	assertp(fp);

    switch(pObjIndex->item_type)
    {
    case ITEM_WEAPON:
        pObjIndex->value[0]         = weapontype(fread_word(fp));
        pObjIndex->value[1]         = fread_number(fp);
        pObjIndex->value[2]         = fread_number(fp);
        pObjIndex->value[3]         = attack_lookup(fread_word(fp));
        pObjIndex->value[4]         = fread_flag(fp);
        break;
	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
        pObjIndex->value[0]         = fread_number(fp);
        pObjIndex->value[1]         = fread_flag(fp);
        pObjIndex->value[2]         = fread_number(fp);
        pObjIndex->value[3]         = fread_number(fp);
        pObjIndex->value[4]         = fread_number(fp);
        break;
    case ITEM_DRINK_CON:
    case ITEM_FOUNTAIN:
        pObjIndex->value[0]         = fread_number(fp);
        pObjIndex->value[1]         = fread_number(fp);
        pObjIndex->value[2]         = liq_lookup(fread_word(fp));
		if(version<2){
			pObjIndex->value[3]		= fread_number(fp);
		}else{
			pObjIndex->value[3]     = fread_flag(fp);
		}
        pObjIndex->value[4]         = fread_number(fp);
        break;
	case ITEM_PARCHMENT:
        pObjIndex->value[0]         = fread_number(fp);
        pObjIndex->value[1]         = fread_number(fp);
        pObjIndex->value[2]         = fread_number(fp);
        pObjIndex->value[3]         = language_safe_lookup(fread_word( fp ))->unique_id;
        pObjIndex->value[4]         = fread_number(fp);
        break;

	case ITEM_INSTRUMENT:
		pObjIndex->value[0]			= fread_number(fp);
		pObjIndex->value[1]			= fread_number(fp);
		pObjIndex->value[2]			= fread_number(fp);
		pObjIndex->value[3]			= fread_number(fp);
		pObjIndex->value[4]			= fread_number(fp);
		break;

    case ITEM_WAND:
    case ITEM_STAFF:
	case ITEM_POULTICE:
        pObjIndex->value[0]         = fread_number(fp);
        pObjIndex->value[1]         = fread_number(fp);
        pObjIndex->value[2]         = fread_number(fp);
        pObjIndex->value[3]         = skill_lookup(fread_word(fp));
        pObjIndex->value[4]         = fread_number(fp);
        break;
    case ITEM_POTION:
    case ITEM_PILL:
    case ITEM_SCROLL:
        pObjIndex->value[0]         = fread_number(fp);
        pObjIndex->value[1]         = skill_lookup(fread_word(fp));
        pObjIndex->value[2]         = skill_lookup(fread_word(fp));
        pObjIndex->value[3]         = skill_lookup(fread_word(fp));
        pObjIndex->value[4]         = skill_lookup(fread_word(fp));
		{
			int snr=skill_lookup("reserved");
			if(snr!=-1){ // convert reserved into -1
				for(int i=1; i<5; i++){
					if(pObjIndex->value[i]==snr){
						pObjIndex->value[i]=-1;
					}
				}
			}
		}
        break;

	case ITEM_COMPONENT:
		pObjIndex->value[0]		= fread_number(fp);
		pObjIndex->value[1]		= skill_lookup(fread_word(fp));
		pObjIndex->value[2]		= fread_number(fp);
		pObjIndex->value[3]		= fread_number(fp);
		pObjIndex->value[4]		= fread_number(fp);
		break;

	case ITEM_SHEATH:
		pObjIndex->value[0]		= fread_number(fp);
		pObjIndex->value[1]		= fread_number(fp);
		pObjIndex->value[2]		= fread_number(fp);
		pObjIndex->value[3]		= fread_number(fp);
		pObjIndex->value[4]		= fread_number(fp);
		break;

    default:
        pObjIndex->value[0]             = fread_flag( fp );
        pObjIndex->value[1]             = fread_flag( fp );
        pObjIndex->value[2]             = fread_flag( fp );
        pObjIndex->value[3]             = fread_flag( fp );
        pObjIndex->value[4]             = fread_flag( fp );
        break;
    }
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

