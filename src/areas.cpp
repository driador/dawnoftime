/**************************************************************************/
// areas.cpp - reading/writing of NAFF area files (New Area File Format)
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
//
// The concept of NAFF is based the following principles:
// * Never use a hard coded number in the area file.
// * Always use word representation of flags in areafiles.
// * As a general rule each line should stand alone and have a meaningful 
//   header, If multiple lines worth of information are needed to be stored 
//   but not necessarily stored in every situation (e.g. room exit flags),
//   then reference something that was previously read in.
// * Put every string thru pack_string when writing, and unpack_string when
//   loading. (These functions add a leading . if necessary + other things).
// * Put everything that will be read in with fread_word() thru pack_word
//   when writing (this adds the ' 's if necessary).
// NOTE: pack_string() is not multibuffered!!! 
//   (This means do NOT call it more than once within a single fprintf()!)
// 
#include "include.h"
#include "areas.h"
#include "db.h"
#include "olc.h"
#include "lockers.h"
#include "shop.h"

// mob_cmds.cpp
char *mprog_type_to_name( int type );

// gamble.cpp
char *gamble_name( GAMBLE_FUN *function);
GAMBLE_FUN *gamble_lookup( const char *name );

// db.cpp

/**************************************************************************/
// local prototypes
void fread_mob_NAFF( MOB_INDEX_DATA *pMob, FILE *fp );
/**************************************************************************/
// will change the vnum if it is only inside the untranslated vnum range 
// of the area - needed to find the new reset values of mobs etc
void apply_area_vnum_offset(int *old_vnum)
{
	if(!area_last){
		bug("apply_area_offset called when area_last==NULL!");
	}

	// do the translation if necessary
	if(	area_last->vnum_offset 
		&& *old_vnum <= area_last->max_vnum 
		&& *old_vnum >= area_last->min_vnum)
	{
		*old_vnum+=area_last->vnum_offset;
	}	

}

/**************************************************************************/
// Prepare a word to be written to file by putting it in 's if necessary
// Also convert ''s to `'s
// Multibuffered :)
const char *pack_word(const char *word)
{
	static int i;
    static char buf[5][512];
	++i%=5;
    buf[i][0] = '\0';

	if(IS_NULLSTR(word)){
		return "''";
	}
	int j=1;	
	bool quote=false;
	for(const char *p=word; *p; p++)
	{
		if(is_space(*p)){
			quote=true;
			buf[i][j]=*p;
		}else if(*p=='\''){
			buf[i][j]='`';
		}else{
			buf[i][j]=*p;
		}
		j++;
	}
	buf[i][j]='\0';

	if(quote){
		buf[i][0]='\'';
		buf[i][j]='\'';
		buf[i][j+1]='\0';
		return buf[i];
	}

	return &buf[i][1];
}
/**************************************************************************/
// Prepare a string to be written to file - Kal Jan 2001
// - Removing all \r's
// - Converting ~'s into {-
// - Prefixing the string with a . if the string begins with a . or space
char *pack_string(const char *str)
{
	if(IS_NULLSTR(str)){
		return "";
	}

	const char *p=str;
	char *result=&temp_HSL_workspace[1];

	// process the input string
	while(*p){
		if(*p=='\r'){ // ignore \r's
			p++; 
		}else if(*p=='~'){ // convert ~ into `-			
			*result++='`'; 
			*result++='-';
			p++;
		}else{
			*result++=*p++;
		}
	}
	*result='\0';

	if(is_space(temp_HSL_workspace[1]) || temp_HSL_workspace[1]=='.'){
		temp_HSL_workspace[0]='.';
		return temp_HSL_workspace;
	}
	return &temp_HSL_workspace[1];
}
/**************************************************************************/
// Remove a leading . from a string if it has one - Kal Jan 2001
char *unpack_string(char *str)
{
	if(IS_NULLSTR(str)){
		return str_dup("");
	}

	if(*str=='.'){
		char *result=str_dup(str+1);
		free_string(str);
		return result;
	}
	return str;
}

/**************************************************************************/
void fwrite_mudprog(MUDPROG_TRIG_ON trig_on, MUDPROG_TRIGGER_LIST *pMprog, FILE *fp)
{	
	const flag_type *t1=mprog_flags, *t2=mprog2_flags;

	switch (trig_on){
		case MUDPROG_TRIG_ON_MOBILE:
			t1=mprog_flags;
			t2=mprog2_flags;
			break;
		case MUDPROG_TRIG_ON_OBJECT:
			t1=oprog_flags;
			t2=oprog2_flags;
			break;
		case MUDPROG_TRIG_ON_ROOM:
			t1=rprog_flags;
			t2=rprog2_flags;
			break;
	}

	// the 0 is for version number, so it will be easy to bump the version
	// and change the format if required
	fprintf(fp, "MudProg 0 %s %s %d %s~%s~\n",
		pack_word(flag_string(t1, pMprog->trig_type)),
		pack_word(flag_string(t2, pMprog->trig2_type)),
		pMprog->prog->vnum,
		pack_string(pMprog->trig_phrase),
		flag_string(position_flags, pMprog->pos_flags));	
}
/**************************************************************************/
// mudprog saving recursive loop in order to save the mudprogs in 
// reverse order as loaded - Kal Jan 01
void fwrite_mudprog_recursive(MUDPROG_TRIG_ON trig_on, MUDPROG_TRIGGER_LIST *pMprog, FILE *fp)
{
	if(!pMprog){ // no more? mudprogs to write
		return;
	}
	fwrite_mudprog_recursive(trig_on, pMprog->next, fp);
	fwrite_mudprog(trig_on, pMprog, fp);

}

/**************************************************************************/
// returns either a mudprog trigger entry or NULL
MUDPROG_TRIGGER_LIST *fread_mudprog(MUDPROG_TRIG_ON trig_on, FILE *fp)
{
	MUDPROG_TRIGGER_LIST *pMprog;

	char *word=fread_word( fp );
	
	if(str_cmp(word, "0")){
		bugf("fread_mudprog(), version 0 expected, '%s' read in!", word);
		word=fread_string(fp); free_string(word);		
		word=fread_string(fp); free_string(word);
		return NULL;
	}

	// allocate some memory for the new 
	pMprog=new_mprog();

	const flag_type *t1=mprog_flags, *t2=mprog2_flags;
	switch (trig_on){
		case MUDPROG_TRIG_ON_MOBILE:
			t1=mprog_flags;
			t2=mprog2_flags;
			break;
		case MUDPROG_TRIG_ON_OBJECT:
			t1=oprog_flags;
			t2=oprog2_flags;
			break;
		case MUDPROG_TRIG_ON_ROOM:
			t1=rprog_flags;
			t2=rprog2_flags;
			break;
	}

	word=fread_word(fp);
	pMprog->trig_type=wordflag_to_value(t1, word);
	word=fread_word(fp);
	pMprog->trig2_type=wordflag_to_value(t2, word);
	int mpvnum=fread_number( fp );
	apply_area_vnum_offset( &mpvnum); // this system only works within a particular area
	assert(mpvnum!=0);
	pMprog->temp_mpvnum= mpvnum; // hack for loading - fixed up in fix_mudprogs

	pMprog->trig_phrase = unpack_string(fread_string( fp ));

	word=fread_string(fp);
	pMprog->pos_flags=wordflag_to_value(position_flags, word);

	return pMprog;
}
/**************************************************************************/
void fwrite_roomecho(room_echo_data *pRe, FILE *fp)
{
	fprintf(fp, "RoomEcho %2d %2d %3d %s~\n", pRe->firsthour, pRe->lasthour,
			pRe->percentage,pack_string(pRe->echotext));
}
/**************************************************************************/
void fwrite_roomecho_recursive(room_echo_data *pRe, FILE *fp)
{
	if(!pRe){ // no more? descripts to write
		return;
	}
	fwrite_roomecho_recursive(pRe->next, fp);
	fwrite_roomecho(pRe, fp);
}
/**************************************************************************/
void fwrite_extradesc(EXTRA_DESCR_DATA *pEd, FILE *fp)
{
	fprintf( fp, "ExtraDesc %s~\n", pack_string(pEd->keyword));
	fprintf( fp, "%s~\n",pack_string(pEd->description)); 
}
/**************************************************************************/
// extra description saving recursive loop in order to save the descripts 
// in reverse order as loaded - Kal Jan 01
void fwrite_extradesc_recursive(EXTRA_DESCR_DATA *pEd, FILE *fp)
{
	if(!pEd){ // no more? descripts to write
		return;
	}
	fwrite_extradesc_recursive(pEd->next, fp);
	fwrite_extradesc(pEd, fp);

}

/**************************************************************************/
// save one room in New Area File Format
// - Kal, based on what Kerenos started
void save_rooms_NAFF( FILE *fp, AREA_DATA *pArea )
{
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pExit;
	int i;
	int exit;

	fprintf( fp, "#ROOMS\n" );
	
    for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ ){
		pRoomIndex=get_room_index(i);
		if(pRoomIndex)
		{
			fprintf( fp, "#%d\n",			pRoomIndex->vnum );
			fprintf( fp, "Name %s~\n",		pack_string(pRoomIndex->name));
			fprintf( fp, "Desc %s~\n",		pack_string(pRoomIndex->description));

			fwrite_wordflag( room_flags,	pRoomIndex->room_flags,		"RoomFlags ", fp);
			fwrite_wordflag( room2_flags,	pRoomIndex->room2_flags,	"Room2Flags ", fp);
			fwrite_wordflag( sector_types,	pRoomIndex->sector_type,	"Sector ", fp);
			if ( pRoomIndex->mana_rate != 100 ){
				fprintf( fp, "Mana %d\n", pRoomIndex->mana_rate );
			}
			if ( pRoomIndex->heal_rate != 100 ){
				fprintf( fp, "Heal %d\n", pRoomIndex->heal_rate );
			}

			if(pRoomIndex->lockers){
				locker_room_data *rl=pRoomIndex->lockers;
				if ( rl->quantity){
					fprintf( fp, "LockerQuant %d\n", rl->quantity);
				}
				if(rl->initial_rent){
					fprintf( fp, "LockerInitRent %d\n", rl->initial_rent);					
				}				
				if ( rl->ongoing_rent){
					fprintf( fp, "LockerOngoRent %d\n", rl->ongoing_rent);
				}
				if ( rl->weight){
					fprintf( fp, "LockerWeight %d\n", rl->weight);
				}
				if ( rl->capacity){
					fprintf( fp, "LockerCapacity %d\n", rl->capacity);
				}
				if ( rl->pick_proof){
					fprintf( fp, "LockerPickProof %d\n", rl->pick_proof);
				}				
			}

			if(!IS_NULLSTR(pRoomIndex->msp_sound)){
				fprintf( fp, "MSP %s~\n", pack_string(pRoomIndex->msp_sound));
			}
			if(pRoomIndex->clan){
				fprintf( fp, "Clan %s\n",	pack_word(pRoomIndex->clan->savename()));
			}
			if(!IS_NULLSTR(pRoomIndex->owner)){
				fprintf( fp, "Owner %s~\n", pack_string(pRoomIndex->owner));
			}				

			// save any room echos 
			fwrite_roomecho_recursive(pRoomIndex->echoes, fp);
			// save any extra descriptions
			fwrite_extradesc_recursive(pRoomIndex->extra_descr, fp);

			// Exits leading out of the room
			for( exit= 0; exit<MAX_DIR; exit++)
			{
				if ((  pExit = pRoomIndex->exit[exit] )
					&& pExit->u1.to_room )
				{
					fprintf( fp, "Exit %s %d\n", pack_word(flag_string(direction_types, exit)), 
						pExit->u1.to_room->vnum);
					fwrite_wordflag(exit_flags,	pExit->rs_flags,	"EFlags ", fp);
					if(pExit->key){
						fprintf( fp, "EKeyvnum %d\n", pExit->key);
					}
					if(!IS_NULLSTR(pExit->keyword)){
						fprintf( fp, "EKeywords %s~\n", pack_string(pExit->keyword));
					}
					if(!IS_NULLSTR(pExit->description)){
						fprintf( fp, "EDesc %s~\n", pack_string(pExit->description));
					}
                }
            }
			fprintf( fp, "End\n\n\n" );
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
    return;
}

/**************************************************************************/
// - Kal, based on what Kerenos started
void load_rooms_NAFF( FILE *fp, int version)
{
	ROOM_INDEX_DATA *pRoomIndex;
	char *word=NULL;
	char *previousword;
	bool fMatch, done = false;
	
	if ( area_last == NULL )
	{
		bug("Load_rooms_NAFF(): no #AREA seen yet.");
		exit_error( 1 , "load_rooms_NAFF", "no #AREA seen yet");
	}
	
	for ( ; ; )
	{
		EXIT_DATA *last_exit;
		vn_int vnum;
		char letter;
		int door;
		int iHash;
		
		letter				= fread_letter( fp );
		if ( letter != '#' )
		{
			bug("Load_rooms_NAFF: # not found.");
			exit_error( 1 , "load_rooms_NAFF", "# not found");
		}
		
		vnum = fread_number( fp );
		if ( vnum == 0 )
			break;
		vnum+= area_last->vnum_offset; 
				
		fBootDb = false;
		ROOM_INDEX_DATA *dup_room_index=get_room_index( vnum );
		if ( dup_room_index)
		{
			bugf( "Load_rooms_NAFF: vnum %d duplicated with room from areafile '%s'.", 
				vnum, dup_room_index->area?dup_room_index->area->file_name:"unknown");
			exit_error( 1 , "load_rooms_NAFF", "duplicate vnum");
		}
		fBootDb = true;
		
		last_vnum = vnum; // backup the last vnum for gdb use
		last_exit=NULL;

		pRoomIndex				= new_room_index();
		pRoomIndex->area		= area_last;
		pRoomIndex->vnum		= vnum;
		
		// check vnum fits in area vnum range 
		if ( vnum < pRoomIndex->area->min_vnum + pRoomIndex->area->vnum_offset)
		{
			bugf("Room with Vnum %d is less than area %s <%s> vnum %d!",
				vnum,
				pRoomIndex->area->name,
				pRoomIndex->area->file_name,
				pRoomIndex->area->min_vnum );
		}
		if ( vnum > pRoomIndex->area->max_vnum + pRoomIndex->area->vnum_offset)
		{
			bugf("Room with Vnum %d is greater than area %s <%s> vnum %d!",
				vnum,
				pRoomIndex->area->name,
				pRoomIndex->area->file_name,
				pRoomIndex->area->max_vnum );
		}
		

		for ( ; ; )
		{
			previousword = word;
			word   = feof( fp ) ? (char*)"End" : fread_word( fp );
			fMatch = false;
			
			switch ( UPPER(word[0]) )
			{
			case '*':
				fMatch = true;
				fread_to_eol( fp );
				break;

			case 'C':
				KEY( "Clan",		pRoomIndex->clan,				clan_slookup(fread_word( fp )));
				break;

			case 'D':
				KEY( "Desc",		pRoomIndex->description,		unpack_string(fread_string( fp )));
				break;

			case 'E':
				if ( !str_cmp( word, "ExtraDesc" ))
				{
					EXTRA_DESCR_DATA *ed=new_extra_descr();
					ed->keyword		= unpack_string(fread_string( fp ));
					ed->description	= unpack_string(fread_string( fp ));
					ed->next		= pRoomIndex->extra_descr;
					pRoomIndex->extra_descr	= ed;
					top_ed++;
					fMatch = true;
					break;
				}

				if ( !str_cmp( word, "Exit" ))
				{
					EXIT_DATA *pexit;
					door = wordflag_to_value( direction_types, fread_word( fp ));
					if ( door < 0 || door>=MAX_DIR )
					{
						bugf("load_rooms_NAFF: room vnum %d '%s' has bad door number.", 
							vnum, pRoomIndex->name);
						exit_error( 1 , "load_rooms_NAFF", "room with bad doors");
					}
					
					pexit						= new_exit();
					pexit->u1.vnum				= fread_number( fp );
					apply_area_vnum_offset(&pexit->u1.vnum);
					last_exit=pexit;
					pRoomIndex->exit[door]		= pexit;
					top_exit++;			
					fMatch = true;
					break;
				}

				KEY( "EFlags",		last_exit->rs_flags,		fread_wordflag( exit_flags, fp ));
				KEY( "EKeyvnum",	last_exit->key,				fread_number(fp) );
				KEY( "EKeywords",	last_exit->keyword,			unpack_string(fread_string(fp)));
				KEY( "EDesc",		last_exit->description,		unpack_string(fread_string(fp)));

				if ( !str_cmp( word, "End" ))
				{
					done = true;
					fMatch = true;
					break;
				}
				break;

			case 'H':
				KEY( "Heal",		pRoomIndex->heal_rate,	fread_number( fp ));
				break;

			case 'L':
				{
					if(!str_prefix("Locker", word)){
						// allocate memory for lockers on a just in time basis
						if(!pRoomIndex->lockers){
							pRoomIndex->lockers=new locker_room_data;
							pRoomIndex->lockers->quantity=0;
							pRoomIndex->lockers->initial_rent=0;
							pRoomIndex->lockers->ongoing_rent=0;
							pRoomIndex->lockers->weight=0;
							pRoomIndex->lockers->capacity=0;
							pRoomIndex->lockers->pick_proof=0;
						}
					}

					KEY( "LockerQuant",		pRoomIndex->lockers->quantity,		fread_number( fp ));
					KEY( "LockerInitRent",	pRoomIndex->lockers->initial_rent,	fread_number( fp ));
					KEY( "LockerOngoRent",	pRoomIndex->lockers->ongoing_rent,	fread_number( fp ));
					KEY( "LockerWeight",	pRoomIndex->lockers->weight,		fread_number( fp ));
					KEY( "LockerCapacity",	pRoomIndex->lockers->capacity,		fread_number( fp ));
					KEY( "LockerPickProof",	pRoomIndex->lockers->pick_proof,	fread_number( fp ));
				}
				break;

			case 'M':
				KEY( "Mana",		pRoomIndex->mana_rate,	fread_number( fp ));
				KEY( "MSP",			pRoomIndex->msp_sound,	unpack_string(fread_string( fp )));
				break;

			case 'N':
				KEY( "Name",		pRoomIndex->name,		unpack_string(fread_string( fp )));
				break;

			case 'O':
				KEY( "Owner",		pRoomIndex->owner,		unpack_string(fread_string( fp )));
				break;
			
			case 'R':
				KEY( "RoomFlags",	pRoomIndex->room_flags,		fread_wordflag( room_flags, fp ));
				KEY( "Room2Flags",	pRoomIndex->room2_flags,	fread_wordflag( room2_flags, fp ));
				if(!str_cmp( word, "RoomEcho"))
				{
					room_echo_data *re=new_room_echo();
					re->firsthour		=fread_number(fp);
					re->lasthour	=fread_number(fp);
					re->percentage	=fread_number(fp);
					re->echotext	=unpack_string(fread_string(fp));
					re->next		=pRoomIndex->echoes;
					pRoomIndex->echoes= re;
					fMatch = true;
					break;
				}
				break;

			case 'S':
				KEY( "Sector",		pRoomIndex->sector_type,	fread_wordflag( sector_types, fp ));
				break;
			}
			
			if ( !fMatch )
			{
				bugf( "load_rooms_NAFF: no match for '%s', word before that = '%s'.",
					word, previousword);
				fread_to_eol( fp );
			}

			if ( done )
			{
				done = false;
				break;
			}
		}

	
		iHash					= vnum % MAX_KEY_HASH;
		pRoomIndex->next		= room_index_hash[iHash];
		room_index_hash[iHash]	= pRoomIndex;
		top_vnum_room			= top_vnum_room < vnum ? vnum : top_vnum_room;
		assign_area_vnum( vnum );
	}
	return;
}

/**************************************************************************/
// save one object in New Area File Format
// - Kal, based on what Kerenos started
void save_object_NAFF( FILE *fp, OBJ_INDEX_DATA *pObjIndex )
{
    fprintf( fp, "#%d\n",			pObjIndex->vnum );
    fprintf( fp, "Name %s~\n",		pack_string(pObjIndex->name));
    fprintf( fp, "Short %s~\n",		pack_string(pObjIndex->short_descr));
    fprintf( fp, "Desc %s~\n",		pack_string(pObjIndex->description));
    fprintf( fp, "Level %d\n",		pObjIndex->level );
	// ItemType must be saved early, so loading knows how to handle values
	fwrite_wordflag( item_types,		pObjIndex->item_type,	"ItemType ", fp); 

	if ( IS_TRAPPED( pObjIndex ))
	{
		fprintf( fp, "Trap %ld %d %d %d\n",
							(long)pObjIndex->trap_trig,
							pObjIndex->trap_dtype,
							pObjIndex->trap_charge,
							pObjIndex->trap_modifier );
	}

    fprintf( fp, "Cost %d\n", pObjIndex->cost );
	if(pObjIndex->condition!=100){
		fprintf( fp, "Condition %d\n", URANGE(1,pObjIndex->condition, 100));
	}
    fprintf( fp, "Asize %d\n", pObjIndex->absolute_size );
	fprintf( fp, "Rsize %d\n", pObjIndex->relative_size );
	fprintf( fp, "Values " );	save_object_values( fp, pObjIndex );
    fprintf( fp, "Weight %d\n", pObjIndex->weight );
	if(IS_NULLSTR(pObjIndex->material)){
		fprintf( fp, "Material unknown~\n");
	}else{
		fprintf( fp, "Material %s~\n", pack_string(pObjIndex->material)); 
	}
	fwrite_wordflag( objextra_flags,	pObjIndex->extra_flags,	"Extra    ", fp);	
	fwrite_wordflag( objextra2_flags,	pObjIndex->extra2_flags,"Extra2   ", fp);	
	fwrite_wordflag( wear_flags,		pObjIndex->wear_flags,	"Wear     ", fp);	
	fwrite_wordflag(classnames_flags,	pObjIndex->class_allowances,"ClassAllowances ", fp );
	fwrite_wordflag(attune_flags,		pObjIndex->attune_flags,"AttuneFlags ", fp );	

	// save all the affects on an object
	fwrite_affect_recursive(pObjIndex->affected, fp);

	// save classgroup object restrictions - system not really finished
	{
		OBJRESTRICT_LIST_DATA *pr;
		for( pr = pObjIndex->restrict; pr; pr = pr->next )
		{
			fprintf( fp, "Restrict %d %s %s\n",
				pr->priority,
				pack_word(pr->classgroup->name),
				pack_word(pr->affectprofile->name));
		}
	}

	// extra descriptions
	fwrite_extradesc_recursive(pObjIndex->extra_descr, fp);

	fwrite_mudprog_recursive(MUDPROG_TRIG_ON_OBJECT, pObjIndex->obj_triggers, fp);

	fprintf(fp, "End\n\n");
    return;
}

/**************************************************************************/
// - Kal, based on what Kerenos started
void load_objects_NAFF( FILE *fp, int version)
{
	OBJ_INDEX_DATA *pObjIndex;
	char *word=NULL;
	char *previousword;
	bool fMatch, done = false;
	
	if ( !area_last )   // OLC
	{
		bug("Load_objects: no #AREA seen yet.");
		exit_error( 1 , "load_objects_NAFF", "no #AREA seen yet");
	}
	
	for ( ; ; )
	{
		vn_int vnum;
		char letter;
		int iHash;
		
        letter = fread_letter( fp );
        if ( letter != '#' )
        {
            bug("Load_objects_NAFF: # not found.");
			exit_error( 1 , "load_objects_NAFF", "# not found");
        }
		
        vnum = fread_number( fp );
		if ( vnum == 0 )
            break;
		vnum+= area_last->vnum_offset; 
		
        fBootDb = false;
        if ( get_obj_index( vnum ) != NULL )
        {
			char *aname="an unknown area.";
			if( get_obj_index( vnum )->area 
				&& get_obj_index( vnum )->area->file_name){
				aname=get_obj_index( vnum )->area->file_name;
			}
            bugf( "Load_objects_NAFF: vnum %d duplicated.", vnum );
			logf("with %s (%s) from %s",
				get_obj_index( vnum )->short_descr,
				get_obj_index( vnum )->name,
				aname);
			exit_error( 1 , "load_objects_NAFF", "duplicate vnum");
        }
        fBootDb = true;
		
        last_vnum = vnum; // backup the last vnum for gdb use
		
        pObjIndex = (OBJ_INDEX_DATA *)alloc_perm( sizeof(*pObjIndex) );
        pObjIndex->vnum = vnum;
        pObjIndex->area = area_last;

        // check object vnum fits in areas vnum range 
        if ( vnum < pObjIndex->area->min_vnum + pObjIndex->area->vnum_offset)
        {
            bugf("Object with Vnum %d is less than area %s <%s> vnum %d!",
                vnum,
                pObjIndex->area->name,
                pObjIndex->area->file_name,
                pObjIndex->area->min_vnum
                );
        }
        if ( vnum > pObjIndex->area->max_vnum + pObjIndex->area->vnum_offset)
        {
            bugf("Object with Vnum %d is greater than area %s <%s> vnum %d!",
                vnum,
                pObjIndex->area->name,
                pObjIndex->area->file_name,
                pObjIndex->area->max_vnum
                );
        }
		
		pObjIndex->trap_trig	= 0;
		pObjIndex->trap_dtype	= 0;
		pObjIndex->trap_charge	= 0;
		pObjIndex->trap_modifier= 0;
		pObjIndex->condition	= 100;
        pObjIndex->reset_num	= 0;

		for ( ; ; )
		{
			previousword = word;
			word   = feof( fp ) ? (char*)"End" : fread_word( fp );
			fMatch = false;
			
			switch ( UPPER(word[0]) )
			{
			case '*':
				fMatch = true;
				fread_to_eol( fp );
				break;

			case 'A':
				KEY( "Asize",			pObjIndex->absolute_size,	fread_number( fp ));
				KEY( "AttuneFlags",		pObjIndex->attune_flags,	fread_wordflag( attune_flags, fp ));
				
				if ( !str_cmp( word, "Affect" ))
				{
					AFFECT_DATA *paf=fread_affect(fp);
					paf->next		= pObjIndex->affected;
					pObjIndex->affected = paf;
					top_affect++;
					fMatch = true;
					break;
				}
				break;

			case 'C':
				KEY( "Cost",			pObjIndex->cost,			fread_number( fp ));
				KEY( "Condition",		pObjIndex->condition,		fread_number( fp ));


				if ( !str_cmp( word, "ClassAllowances" ))
				{
//					KEY( "ClassAllowances",	pObjIndex->class_allowances,fread_wordflag( classnames_flags, fp ));
					char *pszClassAllowances=fread_string(fp);
					fread_to_eol(fp);
					if(GAMESETTING5(GAMESET5_TRANSLATE_ROGUE_CLASS_TO_THIEF)){
						char *pszClassAllowancesLower=str_dup(lowercase(pszClassAllowances));
						free_string(pszClassAllowances);
						pszClassAllowances=string_replace(pszClassAllowancesLower, "rogue", "thief");
					}
					pObjIndex->class_allowances=wordflag_to_value(classnames_flags,pszClassAllowances);
					replace_string(pszClassAllowances,"");
					fMatch = true;
				}
				break;

			case 'D':
				KEY( "Desc",			pObjIndex->description,		unpack_string(fread_string( fp )));
				break;

			case 'E':
				KEY( "Extra",		pObjIndex->extra_flags,			fread_wordflag( objextra_flags, fp ));
				KEY( "Extra2",		pObjIndex->extra2_flags,		fread_wordflag( objextra2_flags, fp ));

				if ( !str_cmp( word, "ExtraDesc" ))
				{
					EXTRA_DESCR_DATA *ed=new_extra_descr();
					ed->keyword		= unpack_string(fread_string( fp ));
					ed->description	= unpack_string(fread_string( fp ));
					ed->next		= pObjIndex->extra_descr;
					pObjIndex->extra_descr	= ed;
					top_ed++;
					fMatch = true;
					if (str_len(ed->description)>MSL-4 || str_len(ed->keyword)>MSL-4 )
					{
						bugf("load_objects_NAFF: Extended description in object "
							"%d is %d characters!!! (more than %d)", 
							(int)last_vnum, str_len(ed->description), MSL-4);
						exit_error( 1 , "load_objects_NAFF", "extended description too long.");
					}
					break;
				}

				if ( !str_cmp( word, "End" ))
				{
					if(pObjIndex->item_type==0){
						bugf("undefined item type for object %d... can not load area file!",
							pObjIndex->vnum);
						exit_error( 1 , "load_objects_NAFF", "undefined item type");
					}
					done = true;
					fMatch = true;
					break;
				}
				break;

			case 'I':
				KEY( "ItemType",		pObjIndex->item_type,		fread_wordflag( item_types, fp ));
				break;

			case 'L':
				KEY( "Level",			pObjIndex->level,			fread_number( fp ));
				break;

			case 'M':
				KEY( "Material",		pObjIndex->material,		unpack_string(fread_string(fp)));

				if ( !str_cmp( word, "MudProg" ))
				{
					MUDPROG_TRIGGER_LIST *pMudProgTrigger=fread_mudprog(MUDPROG_TRIG_ON_OBJECT, fp);
					if(pMudProgTrigger){
						pMudProgTrigger->next	= pObjIndex->obj_triggers;
						pObjIndex->obj_triggers	= pMudProgTrigger;
					}
					fMatch = true;
					break;
				}
				break;

			case 'N':
				KEY( "Name",			pObjIndex->name,			unpack_string(fread_string(fp)));
				break;

			case 'R':
				KEY( "Rsize",			pObjIndex->relative_size,	fread_number( fp ));

				if ( !str_cmp( word, "Restrict" ))
				{
					OBJRESTRICT_LIST_DATA *pr;
					classgroup_type * cg;
					affectprofile_type *ap;

					int priority			= fread_number( fp );
					char * classgroup		= fread_word ( fp );
					char * affectprofile	= fread_word ( fp );

					cg=classgroup_lookup(classgroup);
					ap=affectprofile_lookup(affectprofile);
					if(!cg || !ap){
						if(!cg){
							bugf("Unknown classgroup '%s' for object vnum %d restriction, IGNORING!", 
								classgroup, vnum);
						}
						if(!ap){
							bugf("Unknown affectprofile '%s' for object vnum %d restriction, IGNORING!", 
								affectprofile, vnum);
						}
					}else{
						pr=new OBJRESTRICT_LIST_DATA;
						pr->affectprofile=ap;
						pr->classgroup=cg;
						pr->priority=priority;
						// can't have positive affect modifiers 
						if(ap->wear_amount>0){
							bugf("positive ap->wear_amount for %s, inverted.", ap->name);
							ap->wear_amount=0-ap->wear_amount;
						}		
						// Add it to the restricts list
						pr->next			= pObjIndex->restrict;
						pObjIndex->restrict	= pr;
						SET_BIT(pObjIndex->objrestrict,(1<<cg->bitindex)); // create bit quick lookup
					}
					fMatch = true;
					break;
				}
				break;

			case 'S':
				KEY( "Short",			pObjIndex->short_descr,		unpack_string(fread_string( fp )));
				break;

			case 'T':
				if ( !str_cmp( word, "Trap" ))
				{
					pObjIndex->trap_trig	= fread_number( fp );
					pObjIndex->trap_dtype	= fread_number( fp );
					pObjIndex->trap_charge	= fread_number( fp );
					pObjIndex->trap_modifier= fread_number( fp );
					fMatch = true;
					break;
				}
				break;

			case 'V':
				if ( !str_cmp( word, "Values" )){
					load_object_values( fp, version, pObjIndex );
					fMatch = true;
					break;
				}
				break;

			case 'W':
				KEY( "Wear",			pObjIndex->wear_flags,		fread_wordflag( wear_flags, fp ));
				KEY( "Weight",			pObjIndex->weight,			fread_number( fp ));
				break;
					
			}

			if ( !fMatch )
			{
				bugf( "load_objects_NAFF: no match for '%s', word before that = '%s'.",
					word, previousword);
				fread_to_eol( fp );
			}
			if ( done )
			{
				done = false;
				break;
			}
        }
		
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
	}
	
    return;
}

/**************************************************************************/
// save one mobile in New Area File Format
// - Kal, based on what Kerenos started
void save_mobile_NAFF( FILE *fp, MOB_INDEX_DATA *pMobIndex )
{
    fprintf( fp, "#%d\n",				pMobIndex->vnum);
    fprintf( fp, "Name %s~\n",			pack_string( pMobIndex->player_name));
    fprintf( fp, "ShortD %s~\n",		pack_string( pMobIndex->short_descr));
    fprintf( fp, "LongD %s~\n",			pack_string( pMobIndex->long_descr ));
    fprintf( fp, "Desc %s~\n",			pack_string( pMobIndex->description));
    fprintf( fp, "Race %s~\n",			pack_string(race_table[pMobIndex->race]->name));

    fprintf( fp, "Align %d %d\n",		pMobIndex->tendency, pMobIndex->alliance );
	if(pMobIndex->xp_mod!=100){
		fprintf( fp, "XPMod %d\n",		pMobIndex->xp_mod );
	}
    fprintf( fp, "Level %d\n",			pMobIndex->level );
    fprintf( fp, "Hitroll %d\n",		pMobIndex->hitroll );
    fprintf( fp, "HitDice %dd%d+%d\n",	pMobIndex->hit[DICE_NUMBER],
										pMobIndex->hit[DICE_TYPE],
										pMobIndex->hit[DICE_BONUS] );
    fprintf( fp, "ManaDice %dd%d+%d\n",	pMobIndex->mana[DICE_NUMBER], 
										pMobIndex->mana[DICE_TYPE], 
										pMobIndex->mana[DICE_BONUS] );
    fprintf( fp, "DamDice %dd%d+%d\n",	pMobIndex->damage[DICE_NUMBER], 
										pMobIndex->damage[DICE_TYPE], 
										pMobIndex->damage[DICE_BONUS] );
    fprintf( fp, "DamType %s\n",		pack_word(attack_table[pMobIndex->dam_type].name));
    fprintf( fp, "AC %d %d %d %d\n",	pMobIndex->ac[AC_PIERCE], 
										pMobIndex->ac[AC_BASH], 
										pMobIndex->ac[AC_SLASH], 
										pMobIndex->ac[AC_EXOTIC]);
	fprintf( fp, "Wealth %ld\n",		pMobIndex->wealth );

	fwrite_wordflag( act_flags,			pMobIndex->act,			"Act    ", fp);
	fwrite_wordflag( act2_flags,		pMobIndex->act2,		"Act2   ", fp);
	fwrite_wordflag( affect_flags,		pMobIndex->affected_by, "AffBy  ", fp);
	fwrite_wordflag( affect2_flags,		pMobIndex->affected_by2,"AffBy2 ", fp);
	fwrite_wordflag( affect3_flags,		pMobIndex->affected_by3,"AffBy3 ", fp);
	fwrite_wordflag( off_flags,			pMobIndex->off_flags,	"Off    ", fp);
	fwrite_wordflag( imm_flags,			pMobIndex->imm_flags,   "Imm    ", fp);
	fwrite_wordflag( res_flags,			pMobIndex->res_flags,   "Res    ", fp);
	fwrite_wordflag( vuln_flags,		pMobIndex->vuln_flags,  "Vuln   ", fp);
	fwrite_wordflag( form_flags,		pMobIndex->form,		"Form   ", fp);
	fwrite_wordflag( part_flags,		pMobIndex->parts,		"Part   ", fp);
	fwrite_wordflag( position_types,	pMobIndex->start_pos,	"StartP ", fp);
	fwrite_wordflag( position_types,	pMobIndex->default_pos,	"DefPos ", fp);
	fwrite_wordflag( size_types,		pMobIndex->size,		"Size   ", fp);
	fwrite_wordflag( sex_types,			pMobIndex->sex,			"Sex    ", fp);

	if(pMobIndex->gamble_fun){
		fprintf(fp,"Gamble %s\n", pack_word(gamble_name(pMobIndex->gamble_fun)));
	}

	if(pMobIndex->pInnData){
		fprintf(fp,"InnBuy   %d\n", pMobIndex->pInnData->profit_buy);
		fprintf(fp,"InnSell  %d\n", pMobIndex->pInnData->profit_sell);
		fprintf(fp,"InnOpen  %d\n", pMobIndex->pInnData->open_hour);
		fprintf(fp,"InnClose %d\n", pMobIndex->pInnData->close_hour);
		for(int i=0;i<MAX_INN; i++){
			if(pMobIndex->pInnData->vnRoom[i]){
				fprintf(fp,"InnRoom %d %d\n", pMobIndex->pInnData->vnRoom[i], pMobIndex->pInnData->shRate[i]);
			}
		}
	}

	if(IS_NULLSTR(pMobIndex->material)){
		fprintf( fp, "Material unknown\n");
	}else{
		fprintf( fp, "Material %s\n" , pack_word(pMobIndex->material)); 
	}

    if (pMobIndex->group){
     	fprintf( fp, "Group %d\n", pMobIndex->group);
	}
    if (pMobIndex->helpgroup){
     	fprintf( fp, "Helpgroup %d\n", pMobIndex->helpgroup);
	}

	fwrite_mudprog_recursive(MUDPROG_TRIG_ON_MOBILE, pMobIndex->mob_triggers, fp);
    
    fprintf(fp, "END\n\n");

    return;
}

/**************************************************************************/
// - Kal, based on what Kerenos started
void load_mobiles_NAFF( FILE *fp, int)
{
	MOB_INDEX_DATA *pMobIndex;
	
	if ( !area_last )	// OLC 
	{
		bug("Load_mobiles: no #AREA seen yet.");
		exit_error( 1 , "load_mobiles_NAFF", "no #AREA seen yet");
	}
	
	for ( ; ; )
	{
		vn_int vnum;
		char letter;
		int iHash;
		
		letter = fread_letter( fp );
		if ( letter != '#' )
		{
			bug("load_mobiles_NAFF(): # not found.");
			exit_error( 1 , "load_mobiles_NAFF", "# not found");
		}
		
		vnum = fread_number( fp );
		if ( vnum == 0 )// end of mobiles section
			break;

		// apply our vnum offset
		vnum+= area_last->vnum_offset; 
		
		fBootDb = false; // Must be outside the get_mob_index() check as with fBootDb 
						 // set to false get_mob_index() doesn't log the lookup
		if ( get_mob_index( vnum ) != NULL )
		{
			char *aname="an unknown area.";
			if( get_mob_index( vnum )->area 
				&& get_mob_index( vnum )->area->file_name){
				aname=get_mob_index( vnum )->area->file_name;
			}
			bugf( "load_mobiles_NAFF(): vnum %d duplicated.", vnum );
			logf("with %s (%s) from %s",
				get_mob_index( vnum )->short_descr,
				get_mob_index( vnum )->player_name,
				aname);
			exit_error( 1 , "load_mobiles_NAFF", "duplicate vnum");
		}
		fBootDb = true;
		
		last_vnum = vnum; // backup the last vnum for gdb use 
		
		pMobIndex= (MOB_INDEX_DATA *)alloc_perm( sizeof(*pMobIndex) );
		pMobIndex->vnum	= vnum;
		pMobIndex->area	= area_last;

		// check mob vnum fits in areas vnum range 
		if ( vnum < pMobIndex->area->min_vnum + pMobIndex->area->vnum_offset)
		{
			bugf("Mob with Vnum %d is less than area %s <%s> vnum %d!",
				vnum,
				pMobIndex->area->name,
				pMobIndex->area->file_name,
				pMobIndex->area->min_vnum
				);
		}
		if ( vnum > pMobIndex->area->max_vnum + pMobIndex->area->vnum_offset)
		{
			bugf("Mob with Vnum %d is greater than area %s <%s> vnum %d!",
				vnum,
				pMobIndex->area->name,
				pMobIndex->area->file_name,
				pMobIndex->area->max_vnum
				);
		}
		
		pMobIndex->pShop	= NULL;
		pMobIndex->xp_mod	= 100; // default to normal xp

		fread_mob_NAFF( pMobIndex, fp );
		
		iHash					= vnum % MAX_KEY_HASH;
		pMobIndex->next 		= mob_index_hash[iHash];
		mob_index_hash[iHash]	= pMobIndex;
		top_mob_index++;
		kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;

		// linked list of all pMobIndex records
		pMobIndex->listnext=pMobIndexlist;
		pMobIndexlist=pMobIndex;
	}
	return;
}

/**************************************************************************/
// - Kal, based on what Kerenos started
void fread_mob_NAFF( MOB_INDEX_DATA *pMob, FILE *fp )
{
	char *word=NULL;
	char *previousword;
	bool fMatch;
	
	for ( ; ; )
	{
		previousword=word;
		word   = feof( fp ) ? (char*)"End" : fread_word( fp );
		fMatch = false;
		
		switch ( UPPER(word[0]) )
		{
		case '*':
			fMatch = true;
			fread_to_eol( fp );
			break;
			
		case 'A':
			KEY( "Act",		pMob->act,			fread_wordflag( act_flags, fp ));
			KEY( "Act2",	pMob->act2,			fread_wordflag( act2_flags, fp ));
			KEY( "AffBy",	pMob->affected_by,	fread_wordflag( affect_flags, fp ));
			KEY( "AffBy2",	pMob->affected_by2,	fread_wordflag( affect2_flags, fp ));
			KEY( "AffBy3",	pMob->affected_by3,	fread_wordflag( affect3_flags, fp ));

			
			if (!str_cmp( word, "Align"))
			{
				pMob->tendency=fread_number( fp );
				pMob->alliance=fread_number( fp );
				fMatch=true;
				break;
			}

			if (!str_cmp(word,"AC"))
			{
				for(int i = 0; i < 4; i++){
					pMob->ac[i] = fread_number(fp);
				}
				fMatch = true;
				break;
			}
			break;
			
		case 'D':
			KEY( "Desc",		pMob->description,		unpack_string(fread_string( fp )));
			KEY( "DamType",		pMob->dam_type,			attack_lookup(fread_word(fp)));
			KEY( "DefPos",		pMob->default_pos,		fread_wordflag( position_types, fp ));

			if ( !str_cmp( word, "DamDice" ))				// 5 d 10 + 50
			{
				pMob->damage[DICE_NUMBER]	= fread_number( fp );	// 5
											  fread_letter( fp );	// d
				pMob->damage[DICE_TYPE]		= fread_number( fp );	// 10
											  fread_letter( fp );	// +
				pMob->damage[DICE_BONUS] 	= fread_number( fp );	// 50
				fMatch = true;
				break;
			}

			break;

		case 'E':
			if ( !str_cmp( word, "End" ) )
			{
				// Last spot in which to default mob settings and such

				// count all the usage stats while loading
				race_table[pMob->race]->inuse++;
				total_npcracescount++;
				if (race_table[pMob->race]->lastarea!=area_last){
					race_table[pMob->race]->areacount++;
					total_npcareacount++;
				}
				race_table[pMob->race]->lastarea=area_last;

				// Capitalize
				pMob->long_descr[0]			= UPPER(pMob->long_descr[0]);
				pMob->description[0]		= UPPER(pMob->description[0]);
				pMob->short_descr[0]		= LOWER(pMob->short_descr[0]);

				SET_BIT(pMob->act, ACT_IS_NPC); // all mobs are NPC's :)
				return;
			}
			break;

        case 'F':
			KEY( "Form",		pMob->form,	fread_wordflag( form_flags, fp ));
            break;
        
		case 'G':
			KEY( "Group",		pMob->group,			fread_number( fp ));
			KEY( "Gamble",		pMob->gamble_fun,		gamble_lookup(fread_word(fp)));			
			break;
			
		case 'H':
			KEY( "HelpGroup",	pMob->helpgroup,		fread_number( fp ))
			KEY( "Hitroll",		pMob->hitroll,			fread_number( fp ));

			if ( !str_cmp( word, "HitDice" ))				// 5 d 10 + 50
			{									
				pMob->hit[DICE_NUMBER] 	= fread_number( fp );	// 5
										  fread_letter( fp );	// d (discarded)
				pMob->hit[DICE_TYPE]	= fread_number( fp );	// 10
										  fread_letter( fp );	// + (discarded)
				pMob->hit[DICE_BONUS]	= fread_number( fp );	// 50

				if(pMob->hit[DICE_NUMBER]<0 || pMob->hit[DICE_TYPE]<0 || pMob->hit[DICE_BONUS]<0){
					char *aname="an unknown area.";
					if(pMob->area && !IS_NULLSTR(pMob->area->file_name)){
						aname=pMob->area->file_name;
					}
					bugf("Mob %d has an invalid hitdice of %dd%d+%d... manually edit %s and fix the area.",
						pMob->vnum, pMob->hit[DICE_NUMBER], pMob->hit[DICE_TYPE], 
						pMob->hit[DICE_BONUS], aname);
					exit_error( 1 , "fread_mob_NAFF", "invalid hitdice");
				}
				fMatch = true;
				break;
			}
			break;
			
		case 'I':
			KEY( "Imm",		pMob->imm_flags,	fread_wordflag( imm_flags, fp ));

			{ // handle reading in the inns
				// if a mob has something that starts with the word Inn,
				// confirm we have allocated memory for the inn
				// if we haven't then we do it now
				if(!str_prefix("inn", word)){
					if(!pMob->pInnData){
						pMob->pInnData= new cInnData;
						top_inn++;
						if ( !pFirstInn ) {
							pFirstInn=pMob->pInnData;
						}
						if ( pLastInn ) {
							pLastInn->pNextInn = pMob->pInnData;
						}
						pLastInn			= pMob->pInnData;
						pLastInn->pNextInn	= NULL;
						pMob->pInnData->vnKeeper = pMob->vnum;
					}					
				}

				KEY( "InnBuy",	pMob->pInnData->profit_buy,	fread_number(fp ));
				KEY( "InnSell",	pMob->pInnData->profit_sell,fread_number(fp ));
				KEY( "InnOpen",	pMob->pInnData->open_hour,	fread_number(fp ));
				KEY( "InnClose",pMob->pInnData->close_hour,	fread_number(fp ));

				if(!str_cmp(word, "InnRoom")){
					int i;
					// first read the data
					int roomvnum=fread_number(fp);
					int roomrate=fread_number(fp);

					// now add them as the first unused inn
					for(i=0; i<MAX_INN; i++){
						if(!pMob->pInnData->vnRoom[i]){
							pMob->pInnData->vnRoom[i]=roomvnum;
							pMob->pInnData->shRate[i]=roomrate;
							break;
						}
					}
					if(i==MAX_INN){
						bugf("No room to store InnRoom %d %d for mob %d, discarding.",
							roomvnum, roomrate, pMob->vnum);
					}
					fMatch = true;
				}
			}
			break;
			
		case 'L':
			KEY( "Level",		pMob->level,			fread_number( fp ));
			KEY( "LongD",		pMob->long_descr,		
				trim_trailing_carriage_return_line_feed(unpack_string(fread_string( fp ))));
			break;
			
		case 'M':
			KEY( "Material",		pMob->material,		str_dup(fread_word( fp )));

			if ( !str_cmp( word, "ManaDice" ))				// 5 d 10 + 50
			{
				pMob->mana[DICE_NUMBER]	= fread_number( fp );	// 5
										  fread_letter( fp );	// d
				pMob->mana[DICE_TYPE]	= fread_number( fp );	// 10
										  fread_letter( fp );	// +
				pMob->mana[DICE_BONUS] 	= fread_number( fp );	// 50
				fMatch = true;
				break;
			}

			if ( !str_cmp( word, "MudProg" ))
			{
				MUDPROG_TRIGGER_LIST *pMprog=fread_mudprog(MUDPROG_TRIG_ON_MOBILE, fp);
				if(pMprog){
					pMprog->next	= pMob->mob_triggers;
					pMob->mob_triggers	= pMprog;
				}
				fMatch = true;
				break;
			}

			
			if ( !str_cmp( word, "MProg" ))
			{
				MUDPROG_TRIGGER_LIST *pMprog;
				char *wod;
				int trigger = 0;
				bool positions=false;
				
				pMprog	   = (MUDPROG_TRIGGER_LIST *)alloc_perm(sizeof(*pMprog));
				wod		   = fread_word( fp );

				// support position flags
				if(wod[0]=='='){ 
					positions=true;
					wod= fread_word( fp );
				}

				if ( !(trigger = flag_lookup( wod, mprog_flags )) )
				{
					bugf("MUDprogs: Invalid trigger '%s' on mob %d.", wod, pMob->vnum);
					exit_error( 1 , "fread_mob_NAFF", "invalid trigger");
				}
				pMprog->trig_type	= trigger;
			
				// get the mudprog number, with vnum translation support
				int mpvnum=fread_number( fp );
				apply_area_vnum_offset( &mpvnum); // this system only works within a particular area
				assert(mpvnum!=0);
				pMprog->temp_mpvnum= mpvnum; // hack for loading - fixed up in fix_mudprogs
				pMprog->trig_phrase = unpack_string(fread_string( fp ));
				if(positions){
					pMprog->pos_flags=fread_wordflag(position_flags, fp);
				}else{
					pMprog->pos_flags=0;
				}
				pMprog->next	= pMob->mob_triggers;
				pMob->mob_triggers	= pMprog;
				fMatch = true;
				break;
			}

			break;
			
		case 'N':
			KEY( "Name",		pMob->player_name,		unpack_string(fread_string( fp )));
			break;
			
		case 'O':
			KEY( "Off",		pMob->off_flags,	fread_wordflag( off_flags, fp ));
			break;
			
		case 'P':
			KEY( "Part",	pMob->parts,		fread_wordflag( part_flags, fp ));
			break;
			
		case 'R':
			KEY( "Res",		pMob->res_flags,	fread_wordflag( res_flags, fp ));

			if ( !str_cmp( word, "Race" ) )
			{
				char *racename=unpack_string(fread_string( fp ));
				pMob->race = race_lookup(racename);
				if(pMob->race == -1)
				{
					logf("Mob %d has an unrecognised race '%s', "
						"dynamically creating race.", pMob->vnum, racename);
					pMob->race= race_generate_race_adding_to_race_table(racename);			
				}
				free_string(racename);
				fMatch = true;
				break;
			}
			
			break;
			
		case 'S':
			KEY( "ShortD",		pMob->short_descr,			unpack_string(fread_string( fp )));
			KEY( "StartP",		pMob->start_pos,	fread_wordflag( position_types, fp ));
			KEY( "Size",		pMob->size,			fread_wordflag( size_types, fp ));
			KEY( "Sex",			pMob->sex,			fread_wordflag( sex_types, fp ));
			break;
			
		case 'V':
			KEY( "Vuln",		pMob->vuln_flags,	fread_wordflag( vuln_flags, fp ));
			break;
			
		case 'W':
			if ( !str_cmp( word, "Wealth" ))
			{
				pMob->wealth = fread_number( fp );
				
				if ( pMob->wealth > pMob->level*MAX_MOB_WEALTH_MULTIPLIER){
					bugf("Mob %d read in with wealth greater than Mob->level*%d (%d), reduced wealth to max", 
						pMob->vnum,  pMob->level*MAX_MOB_WEALTH_MULTIPLIER, (int)pMob->wealth);
					pMob->wealth = pMob->level*MAX_MOB_WEALTH_MULTIPLIER;
				}
				if( pMob->wealth < 0 ){
					pMob->wealth = 0;
				}
				fMatch = true;
				break;
			}
			break;
			
		case 'X':
			if ( !str_cmp( word, "XPMod" ))
			{
				pMob->xp_mod				= fread_number( fp );
				if( pMob->xp_mod < 0 ){		
					bugf("Mob %d read in with xpmod of %d, increased it to 0", pMob->vnum, pMob->xp_mod);
					pMob->xp_mod = 0;
				}
				if( pMob->xp_mod > 200 ){
					bugf("Mob %d read in with xpmod of %d, reduced it to 200", pMob->vnum, pMob->xp_mod);
					pMob->xp_mod = 200;
				}
				fMatch = true;
				break;
			}
			break;
		}
		
		if ( !fMatch )
		{
			bugf( "Fread_char: no match for '%s', word before that = '%s'.", 
				word, previousword);
			fread_to_eol( fp );
		}
    }
}
/**************************************************************************/
#define OLDAPPLY_NONE			0
#define OLDAPPLY_ST				1
#define OLDAPPLY_QU				2
#define OLDAPPLY_PR				3
#define OLDAPPLY_EM				4
#define OLDAPPLY_IN				5
#define OLDAPPLY_SEX			6
#define OLDAPPLY_CLASS			7
#define OLDAPPLY_LEVEL			8
#define OLDAPPLY_AGE			9
#define OLDAPPLY_HEIGHT			10
#define OLDAPPLY_WEIGHT			11
#define OLDAPPLY_MANA			12
#define OLDAPPLY_HIT			13
#define OLDAPPLY_MOVE			14
#define OLDAPPLY_GOLD			15
#define OLDAPPLY_EXP			16
#define OLDAPPLY_AC				17
#define OLDAPPLY_HITROLL		18
#define OLDAPPLY_DAMROLL		19
#define OLDAPPLY_SAVES			20
#define OLDAPPLY_SAVING_PARA	20
#define OLDAPPLY_SAVING_ROD		21
#define OLDAPPLY_SAVING_PETRI	22
#define OLDAPPLY_SAVING_BREATH	23
#define OLDAPPLY_SAVING_SPELL	24
#define OLDAPPLY_SPELL_AFFECT	25 // set to saves, unused in theory though.
#define OLDAPPLY_CO				26
#define OLDAPPLY_AG				27
#define OLDAPPLY_SD				28
#define OLDAPPLY_ME				29
#define OLDAPPLY_RE				30 
#define OLDAPPLY_COPY_TO_CHAR	31

struct old_apply_translate_table_type { int oldval; APPLOC apploc_value; };

old_apply_translate_table_type old_apply_translate_table[]=
{
	{OLDAPPLY_NONE, APPLY_NONE},
	{OLDAPPLY_ST, APPLY_ST},
	{OLDAPPLY_QU, APPLY_QU},
	{OLDAPPLY_PR, APPLY_PR},
	{OLDAPPLY_EM, APPLY_EM},
	{OLDAPPLY_IN, APPLY_IN},
	{OLDAPPLY_SEX, APPLY_SEX},
	{OLDAPPLY_CLASS, APPLY_CLASS},
	{OLDAPPLY_LEVEL, APPLY_LEVEL},
	{OLDAPPLY_AGE, APPLY_AGE},
	{OLDAPPLY_HEIGHT, APPLY_HEIGHT},
	{OLDAPPLY_WEIGHT, APPLY_WEIGHT},
	{OLDAPPLY_MANA, APPLY_MANA},
	{OLDAPPLY_HIT, APPLY_HIT},
	{OLDAPPLY_MOVE, APPLY_MOVE},
	{OLDAPPLY_GOLD, APPLY_GOLD},
	{OLDAPPLY_EXP, APPLY_EXP},
	{OLDAPPLY_AC, APPLY_AC},
	{OLDAPPLY_HITROLL, APPLY_HITROLL},
	{OLDAPPLY_DAMROLL, APPLY_DAMROLL},
	{OLDAPPLY_SAVES, APPLY_SAVES},
	{OLDAPPLY_SAVING_PARA, APPLY_SAVES},
	{OLDAPPLY_SAVING_ROD, APPLY_SAVES},
	{OLDAPPLY_SAVING_PETRI, APPLY_SAVES},
	{OLDAPPLY_SAVING_BREATH, APPLY_SAVES},
	{OLDAPPLY_SAVING_SPELL, APPLY_SAVES},
	{OLDAPPLY_SPELL_AFFECT, APPLY_SAVES},  // mapped to a saves...
	{OLDAPPLY_CO, APPLY_CO},
	{OLDAPPLY_AG, APPLY_AG},
	{OLDAPPLY_SD, APPLY_SD},
	{OLDAPPLY_ME, APPLY_ME},
	{OLDAPPLY_RE, APPLY_RE},
	{OLDAPPLY_COPY_TO_CHAR, APPLY_NONE},
	{-1, APPLY_NONE} // mark end of table with -1 oldloc
};

/**************************************************************************/
// Convert numeric values stored in files (old file formats) to enumerated
// values - Kal
APPLOC translate_old_apply_number(int oldvalue)
{
	for(int i=0; old_apply_translate_table[i].oldval!=-1; i++){
		if(oldvalue==old_apply_translate_table[i].oldval){
			APPLOC val=old_apply_translate_table[i].apploc_value;
	//		logf("Translating apply number %2d to %s (%d)", 
	//			oldvalue, flag_string(apply_types,val), val);			
			return val;
		}
	}
	bugf("translate_old_apply_number(): Unsupported old APPLY_VALUE '%d'... "
		"manually remove where it came from?", oldvalue);

	do_abort();
	return APPLY_NONE; // no compiler area
}
/**************************************************************************/
// reverse the affect of translate_old_apply_number() - Kal
int reverse_translate_old_apply_number(APPLOC newvalue)
{
	for(int i=0; old_apply_translate_table[i].oldval!=-1; i++){
		if(newvalue==old_apply_translate_table[i].apploc_value){
			return old_apply_translate_table[i].oldval;
		}
	}
	bug("reverse_translate_old_apply_number(): Couldn't find number to reverse to!");
	do_abort();
	return 0;
}

/**************************************************************************/
// write the buy types - Kal
void shopdata_write_buy_types(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	shop_data * psd;
	psd= (shop_data*) data;
	for(int i=0; i<MAX_TRADE; i++){
		if(psd->buy_type[i]){
			char heading[MIL];
			strcpy(heading, gio_table[tableIndex].heading);
			fwrite_wordflag(item_types, psd->buy_type[i], heading, fp);
		}
	}
}
/**************************************************************************/
// read the buy types - Kal
void shopdata_read_buy_types(gio_type *, int, void *data, FILE *fp)
{
	shop_data * psd;
	psd= (shop_data*) data;
	int bt=fread_wordflag(item_types,fp);

	// find the next free buy type slot to put it in.
	for(int i=0; i<MAX_TRADE; i++){
		if(!psd->buy_type[i]){
			psd->buy_type[i]=bt;
			return;
		}
	}
	bugf("shopdata_read_buy_types(): Too many buy types for reading in "
		"shop that is applied to mob %d", psd->keeper);
	exit_error( 1 , "shopdata_read_buy_types", "Too many buy types");
}

/**************************************************************************/
GIO_START(shop_data)
GIO_CUSTOM_WRITEH(buy_type,	"buy_type ", shopdata_write_buy_types)
GIO_CUSTOM_READH(buy_type,	"buy_type ", shopdata_read_buy_types)
GIO_SHINT(profit_buy)
GIO_SHINT(profit_sell)
GIO_SHINT(open_hour)
GIO_SHINT(close_hour)
GIO_FINISH


/**************************************************************************/
// Save shops in NAFF format - Kal
void save_shops_NAFF( FILE *fp, AREA_DATA *pArea )
{
    SHOP_DATA *pShopIndex;
    MOB_INDEX_DATA *pMobIndex;
    int iHash;
    
    fprintf( fp, "#SHOPS\n" );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop )
            {
                pShopIndex = pMobIndex->pShop;
				fprintf(fp, "#%d\n", pShopIndex->keeper);
				GIO_SAVE_RECORD(shop_data, pShopIndex, fp, NULL);
            }
        }
    }

    fprintf( fp, "#0\n\n\n" );
    return;
}
/**************************************************************************/
// Kal
void load_shops_NAFF( FILE *fp )
{
	SHOP_DATA *pShop;

	for ( ; ; )
	{
		char letter = fread_letter( fp );
		if ( letter != '#' )
		{
			bugf("load_shops_NAFF: # not found, letter='%c'.", letter);
			exit_error( 1 , "load_shops_NAFF", "# not found");
		}

		int mobnum=fread_number ( fp );
		if ( mobnum == 0 ){
			 return;
		}
		apply_area_vnum_offset( &mobnum);

		// read in the shop keeper data using GIO
		pShop= (SHOP_DATA *)alloc_perm( sizeof(*pShop) );
		GIO_LOAD_RECORD(shop_data, pShop, fp);

		pShop->keeper= mobnum;

		MOB_INDEX_DATA *pMobIndex= get_mob_index( pShop->keeper );
		if(!pMobIndex){
			bugf("load_shops: Can't find shop keeper with Vnum %d!", pShop->keeper);						
		}else{	
			pMobIndex->pShop = pShop;
			if ( shop_first == NULL ){
				shop_first = pShop;
			}
			if ( shop_last  != NULL ){
				shop_last->next = pShop;
			}
			shop_last       = pShop;
			pShop->next     = NULL;
			top_shop++;
		}

    }

    return;
}
/**************************************************************************/
GIO_START(mudprog_code)
GIO_STR(title)
GIO_STR(author) 
GIO_STR(code)
GIO_BOOL(disabled)
GIO_STR(disabled_text)
GIO_STR(last_editor)
GIO_LONG(last_editdate) 
GIO_FINISH_STRDUP_EMPTY
/**************************************************************************/
// Kal
void save_mudprogs_NAFF( FILE *fp, AREA_DATA *pArea )
{
	MUDPROG_CODE *pMprog;
	int i;
	
	fprintf(fp, "#MUDPROGS\n");
	
	for( i = pArea->max_vnum; i>=pArea->min_vnum; i-- )
	{
		if ( (pMprog = get_mprog_index(i) ) != NULL)
		{
			if(GAMESETTING2(GAMESET2_DONT_SAVE_MUDPROG_AUTHORS)){
				replace_string(pMprog->author,"");
			}
			if(GAMESETTING2(GAMESET2_DONT_SAVE_LASTEDITORS)){
				replace_string(pMprog->last_editor,"");
				pMprog->last_editdate=0;
			}
			fprintf(fp, "#%d\n", i);
			GIO_SAVE_RECORD(mudprog_code, pMprog, fp, NULL);
		}
	}

	fprintf(fp,"#0\n\n\n");
	return;
}
/**************************************************************************/
// Kal
void load_mudprogs_NAFF( FILE *fp )
{
	for ( ; ; )
	{
		char letter = fread_letter( fp );
		if ( letter != '#' )
		{
			bugf("load_mudprogs_NAFF: # not found, letter='%c'.", letter);
			exit_error( 1 , "load_mudprogs_NAFF", "# not found");
		}

		int vnum=fread_number ( fp );
		if ( vnum== 0 ){
			 return;
		}
		apply_area_vnum_offset( &vnum);

		fBootDb = false;
		if ( get_mprog_index( vnum ) != NULL )
		{
			bugf( "Load_mudprogs: vnum %d duplicated.", vnum );
			exit_error( 1 , "load_mudprogs_NAFF", "duplicate vnum");
		}
		fBootDb = true;

		MUDPROG_CODE *pMprog=(MUDPROG_CODE*)alloc_perm( sizeof(*pMprog) );
		// read in the mudprog code using GIO
		GIO_LOAD_RECORD(mudprog_code, pMprog, fp);
		mudprog_count++;
		pMprog->vnum = vnum;
		pMprog->area = area_last;

		// add our prog to the list of progs
		if ( !mudprog_list ){
			mudprog_list = pMprog;
		}else{
			pMprog->next = mudprog_list;
			mudprog_list  = pMprog;
		}
    }
    return;
}
/**************************************************************************/
// generic affect saving recursive loop in order to save the affects in 
// reverse order as loaded - Kal Jan 01
void fwrite_affect_recursive(AFFECT_DATA *paf, FILE *fp)
{
	if(!paf){ // we got the short end of the stick
		return;
	}
	fwrite_affect_recursive(paf->next, fp);
	fwrite_affect(paf, fp);

}
/**************************************************************************/
// generic affect saving - Kal Jan 01
void fwrite_affect(AFFECT_DATA *paf, FILE *fp)
{
	char skillname_text[MIL];
	const flag_type *bv_flags=affect_get_bitvector_table_for_where(paf->where);

	if(paf->type>=0 && paf->type<MAX_SKILL){
		strcpy(skillname_text,skill_table[paf->type].name);
	}else{
		sprintf(skillname_text,"%d", paf->type);
	}

	if(!bv_flags && paf->bitvector){
		// prevent future developers using the bitvectors without
		// adding a wordflag table.
		bugf("Affect bitvectors for %d don't have a matching table and are non NULL!",
			paf->where);
		logf( "Affect %s %s %d %d %s %d %s <<<<<< the last value needs to be non 0!",
			pack_word(flag_string(to_types, paf->where)),
			pack_word(skillname_text),
			paf->level,
			paf->duration,
			pack_word(flag_string(apply_types, paf->location)),
			paf->modifier,
			pack_word(bv_flags?flag_string(bv_flags, paf->bitvector):"0"));
		do_abort(); 
	}

	// generic affect saving
	fprintf( fp, "Affect %s %s %d %d %s %d %s\n",
		pack_word(flag_string(to_types, paf->where)),
		pack_word(skillname_text),
		paf->level,
		paf->duration,
		pack_word(flag_string(apply_types, paf->location)),
		paf->modifier,
		pack_word(bv_flags && paf->bitvector?flag_string(bv_flags, paf->bitvector):"0"));

}
/**************************************************************************/
// generic affect loading - Kal Jan 01
AFFECT_DATA *fread_affect(FILE *fp)
{
	AFFECT_DATA *paf;
	paf=(AFFECT_DATA *)alloc_perm( sizeof(*paf) );
	char *where_word=fread_word(fp);
	paf->where=wordflag_to_value(to_types, where_word);
	
	const flag_type *bv_flags=affect_get_bitvector_table_for_where(paf->where);
	
	char *skillname	= fread_word(fp);
	if(is_number(skillname)){
		paf->type=atoi(skillname);
	}else{
		paf->type = skill_lookup(skillname);
		if(paf->type<0)
		{
			bugf("fread_affect(): Unfound non numeric skillname '%s' when reading in affect.", skillname);
			paf->type=-1;
			
			{	// autonote it and move on.
				char body[MSL];
				sprintf(body, "fread_affect(): Unfound unrecognised skillname '%s' "
					"when reading in affect.  This may or maynot be on a pfile... "
					"You can get more details in the mud main log by searching for"
					"log entries around this date and time.", skillname);
				autonote(NOTE_SNOTE, "fread_affect()",
					"Unfound non numeric skillname.", 
					"admin code", body, true);
			}
		}
	}

	paf->level		= fread_number(fp);
	paf->duration	= fread_number(fp);

	char *location_word=fread_word(fp);
	paf->location=(APPLOC)wordflag_to_value(apply_types, location_word);
	if(paf->location==NO_FLAG){
		bugf("fread_affect(): Unfound recognised affect location '%s' when reading in affect.",
			location_word);
		exit_error( 1 , "fread_affect", "unfound recognised affect location");
	}
	paf->modifier	= fread_number( fp );

	char *bitvector_word=fread_word(fp);
	if(bv_flags && !(bitvector_word[0]=='0' && bitvector_word[1]=='\0') ){
		paf->bitvector=wordflag_to_value(bv_flags, bitvector_word);
		if(paf->bitvector==NO_FLAG){
			bugf("fread_affect(): Unfound recognised affect bitvector '%s' when reading in affect.",
				location_word);
			exit_error( 1 , "fread_affect", "unfound recognised affect bitvector");
		}
	}else{
		assert(!str_cmp(bitvector_word,"0")); // safety check
		paf->bitvector=0;
	}

	// convert to WHERE_MODIFIER if appropriate
	if(paf->where == WHERE_OBJEXTRA && paf->bitvector==0 && paf->location!=APPLY_NONE){
		paf->where=WHERE_MODIFIER;
	};
	return paf;
}
/**************************************************************************/
// Read in a mobiles affects and possible affects two, from an area file
// - Kal, July 01
void areaimport_mobile_affects_stock( FILE *fp, int version, MOB_INDEX_DATA *pMobIndex)
{
	if(version<3){
		pMobIndex->affected_by	= fread_flag( fp )  | race_table[pMobIndex->race]->aff;
		pMobIndex->affected_by2	= race_table[pMobIndex->race]->aff2;
		pMobIndex->affected_by3	= race_table[pMobIndex->race]->aff3;
		return;
	}

	if ( version > 3 ){
		pMobIndex->act2			= fread_flag( fp );
	}
	if ( version > 4 ){
		fread_to_eol(fp); 
	}
	pMobIndex->affected_by		= fread_flag( fp )| race_table[pMobIndex->race]->aff;

	if ( version > 3 ){
		pMobIndex->affected_by2	= fread_flag( fp )| race_table[pMobIndex->race]->aff2;

		// not reading in any aff3 flags, since file format most likely doesn't have it - Kal, May09
		//pMobIndex->affected_by3	= fread_flag( fp )| race_table[pMobIndex->race]->aff3; 
		pMobIndex->affected_by3	= race_table[pMobIndex->race]->aff3;
	}
	if ( version > 4  || AREA_IMPORT_FLAG(AREAIMPORTFLAG_READ_TO_EOL_ON_ACT_AFF_LINE)){
		fread_to_eol(fp); 
	}

}
/**************************************************************************/
void areaimport_mobile_affects_format2( FILE *fp, int version, MOB_INDEX_DATA *pMobIndex)
{
	// in this format, mobile affects are stored in the format
	// 16bitnumber 16bitnumber 16bitnumber .... -1

	// to translate to dawn the first two 16 bits must be concatenated, 
	// put thru the rom affect flag to dawn affect flag converter
	// then the remainder discarded or translated

	// any remaining numbers need to be translated/ignored

	// read the first two numbers
	int num=fread_number(fp);
	int num2=fread_number(fp);
	// merge them
	num+=num2<<16;

	pMobIndex->affected_by=num;
	// remove any flags we don't support 
	// - can be changed to a translation using the example code here:
	// *** example translation code *** 
	//if(IS_SET(pMobIndex->affected_by, ee)){
	//	REMOVE_BIT(pMobIndex->affected_by, ee);
	//  SET_BIT(pMobIndex->affected_by2, AFF2_MUTE);
	//}
	REMOVE_BIT(pMobIndex->affected_by, L);
	REMOVE_BIT(pMobIndex->affected_by, ee);

	// read the next 16 bits
	num=fread_number(fp);

	pMobIndex->affected_by2=0;
	// translate the affect bits over to dawn positions
	REMOVE_BIT(pMobIndex->affected_by, L);
	if(IS_SET(pMobIndex->affected_by, ee)){ // AFF_FIRESHIELD (old bit 31)
		REMOVE_BIT(pMobIndex->affected_by, ee);
		SET_BIT(pMobIndex->affected_by2, AFF2_FIRE_SHIELD);
	}
	if(IS_SET(pMobIndex->affected_by, 1<<31)){  // AFF_ICESHIELD (old bit 32)
		REMOVE_BIT(pMobIndex->affected_by, 1<<31);
		SET_BIT(pMobIndex->affected_by2, AFF2_ICE_SHIELD);
	}
	// bits higher than 33 have moved into num, starting from bit position A
	if(IS_SET(num, A)){// AFF_SHOCKSHIELD (old bit 33) -> A
		SET_BIT(pMobIndex->affected_by2, AFF2_SHOCK_SHIELD);
	}
	if(IS_SET(num, B)){// AFF_HALLUCINATE (old bit 34) -> B
		SET_BIT(pMobIndex->affected_by2, AFF2_HALLUCINATE);
	}

	// keep reading until we get a -1 (we aren't interested in any more of the numbers)
	while (num!=-1){
		num=fread_number(fp);
	}
}
/**************************************************************************/
void areaimport_object_translate_flags_stock( FILE *fp, int version, OBJ_INDEX_DATA *pObjIndex)
{
	if(version<4){
		// TRANSLATE mud WEAR FORMAT TO DAWN FORMAT
		// stock rom defines up to Q
		REMOVE_BIT(pObjIndex->wear_flags, R | S | T | U | V | W | X | Y | Z | aa | bb | cc | dd | ee);
/*		
		// fix this up on a per mud basis !?! 
		// - wear flags
		if(IS_SET(pObjIndex->wear_flags, R)){
			REMOVE_BIT(pObjIndex->wear_flags, R);
			SET_BIT(pObjIndex->wear_flags, OBJWEAR_EYES);
		}
		if(IS_SET(pObjIndex->wear_flags, S)){
			REMOVE_BIT(pObjIndex->wear_flags, S);
			SET_BIT(pObjIndex->wear_flags, OBJWEAR_EAR);
		}
		if(IS_SET(pObjIndex->wear_flags, T)){
			REMOVE_BIT(pObjIndex->wear_flags, T);
			SET_BIT(pObjIndex->wear_flags, OBJWEAR_FACE);
		}
		if(IS_SET(pObjIndex->wear_flags, U)){
			REMOVE_BIT(pObjIndex->wear_flags, U);
			SET_BIT(pObjIndex->wear_flags, OBJWEAR_ANKLE);
		}
*/
		// extra flags
		// stock rom defines up to Z less R
		REMOVE_BIT(pObjIndex->extra_flags, R | aa | bb | cc | dd | ee);
/*		if(IS_SET(pObjIndex->extra_flags, bb)){
			REMOVE_BIT(pObjIndex->extra_flags, bb);
			SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_QUEST);
		}
		if(IS_SET(pObjIndex->extra_flags, cc)){
			REMOVE_BIT(pObjIndex->extra_flags, cc);
			SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_HOLY);
		}
		if(IS_SET(pObjIndex->extra_flags, dd)){
			REMOVE_BIT(pObjIndex->extra_flags, dd);
			SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_REMORT);
		}
		if(IS_SET(pObjIndex->extra_flags, ee)){
			REMOVE_BIT(pObjIndex->extra_flags, ee);
			SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_VAMPIRE_BANE);
		}
*/
	}
}
/**************************************************************************/
void areaimport_object_translate_flags_format2( FILE *fp, int version, OBJ_INDEX_DATA *pObjIndex)
{
	int wear_flags=pObjIndex->wear_flags;
	pObjIndex->wear_flags=0;
	// TRANSLATE mud WEAR FORMAT TO DAWN FORMAT
	// - wear flags
	if(IS_SET(wear_flags, A)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_TAKE);
	}

	if(IS_SET(wear_flags, B)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_FLOAT);
	}

	if(IS_SET(wear_flags, C)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_HEAD);
	}

	if(IS_SET(wear_flags, D)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_FACE);
	}
		
	if(IS_SET(wear_flags, E)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_EAR);
	}
		
	if(IS_SET(wear_flags, F)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_NECK);
	}

	if(IS_SET(wear_flags, G)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_ABOUT);
	}
		
	if(IS_SET(wear_flags, H)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_TORSO);
	}
		
	if(IS_SET(wear_flags, I)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_BACK);
	}
		
	if(IS_SET(wear_flags, J)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_ARMS);
	}
		
	if(IS_SET(wear_flags, K)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_WRIST);
	}
		
	if(IS_SET(wear_flags, L)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_HANDS);
	}
		
	if(IS_SET(wear_flags, M)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_FINGER);
	}
		
	if(IS_SET(wear_flags, N)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_WIELD);
	}
		
	if(IS_SET(wear_flags, O)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_HOLD);
	}
		
	if(IS_SET(wear_flags, P)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_SHIELD);
	}
		
	if(IS_SET(wear_flags, Q)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_WAIST);
	}
		
	if(IS_SET(wear_flags, R)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_LEGS);
	}
		
	if(IS_SET(wear_flags, S)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_ANKLE);
	}
		
	if(IS_SET(wear_flags, T)){
		SET_BIT(pObjIndex->wear_flags, OBJWEAR_FEET);
	}

	// extra flags
	if(IS_SET(pObjIndex->extra_flags, bb)){
		REMOVE_BIT(pObjIndex->extra_flags, bb);
		SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_QUEST);
	}
	if(IS_SET(pObjIndex->extra_flags, cc)){
		REMOVE_BIT(pObjIndex->extra_flags, cc);
		SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_HOLY);
	}
	if(IS_SET(pObjIndex->extra_flags, dd)){
		REMOVE_BIT(pObjIndex->extra_flags, dd);
		SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_REMORT);
	}
	if(IS_SET(pObjIndex->extra_flags, ee)){
		REMOVE_BIT(pObjIndex->extra_flags, ee);
		SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_VAMPIRE_BANE);
	}
}
/**************************************************************************/
void areaimport_object_translate_flags_format3( FILE *fp, int version, OBJ_INDEX_DATA *pObjIndex)
{
	if(version<4){
		// TRANSLATE format3 WEAR FORMAT TO DAWN FORMAT
		// format3 defines bits up to T, R,S & T are different
		// - wear flags
		if(IS_SET(pObjIndex->wear_flags, R)){
			REMOVE_BIT(pObjIndex->wear_flags, R);
			SET_BIT(pObjIndex->wear_flags, OBJWEAR_EAR);
		}
		if(IS_SET(pObjIndex->wear_flags, S)){
			REMOVE_BIT(pObjIndex->wear_flags, S);
			SET_BIT(pObjIndex->wear_flags, OBJWEAR_FACE);
		}
		if(IS_SET(pObjIndex->wear_flags, T)){
			REMOVE_BIT(pObjIndex->wear_flags, T);
			SET_BIT(pObjIndex->wear_flags, OBJWEAR_BACK);
		}
		// remove any other bits we don't know about
		REMOVE_BIT(pObjIndex->wear_flags, U | V | W | X | Y | Z | aa | bb | cc | dd | ee);

		// extra flags
		// format3 defines up to bb less R (aa and bb are different)
		REMOVE_BIT(pObjIndex->extra_flags, R | cc | dd | ee);
		if(IS_SET(pObjIndex->extra_flags, aa)){
			REMOVE_BIT(pObjIndex->extra_flags, aa);
			SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_NOSELL);
		}
		if(IS_SET(pObjIndex->extra_flags, bb)){
			REMOVE_BIT(pObjIndex->extra_flags, cc);
			SET_BIT(pObjIndex->extra2_flags, OBJEXTRA2_NOQUEST);
		}
	}
}
/**************************************************************************/
void areaimport_room_flags_stock( FILE *fp, int version, ROOM_INDEX_DATA *pRoomIndex)
{
	if(version<4){	// remove any room flags that dawn didn't use before version 4
		REMOVE_BIT(pRoomIndex->room_flags, U|V|X|Y|Z); 
	}
	if(version<11){
		// flag was allocated to class restriction, since removed
		REMOVE_BIT(pRoomIndex->room_flags, V); 
	}
}
/**************************************************************************/
void areaimport_room_flags_format2( FILE *fp, int version, ROOM_INDEX_DATA *pRoomIndex)
{
	if(IS_SET(pRoomIndex->room_flags, U)){ // BANK
		REMOVE_BIT(pRoomIndex->room_flags, U); 
		SET_BIT(pRoomIndex->room_flags, ROOM_BANK); 
	}
	if(IS_SET(pRoomIndex->room_flags, V)){ // ARENA
		REMOVE_BIT(pRoomIndex->room_flags, V); 
		SET_BIT(pRoomIndex->room_flags, ROOM_ARENA); 
	}	
	if(IS_SET(pRoomIndex->room_flags, Y)){ // NO_MAGIC
		REMOVE_BIT(pRoomIndex->room_flags, Y); 
		SET_BIT(pRoomIndex->room_flags, ROOM_ANTIMAGIC); 
	}

	if(IS_SET(pRoomIndex->room_flags, W)){ // NO_SUMMON
		REMOVE_BIT(pRoomIndex->room_flags, W); 
		SET_BIT(pRoomIndex->room_flags, ROOM_NO_SUMMON); 
	}

	if(IS_SET(pRoomIndex->room_flags, X)){ // NO_PORTAL
		REMOVE_BIT(pRoomIndex->room_flags, X); 
		SET_BIT(pRoomIndex->room_flags, ROOM_NO_PORTAL); 
	}

}

/**************************************************************************/
/*
	// RESETS VERSION 1 AND 2 FORMATS
	M 0 <mnum>			<global mob limit>	<rnum>			<room mob limit>
	O 0 <onum>			<global limit>		<rnum>			0
	P 0 <contents_onum> <global limit>	<container_onum>	<container_limit>
(v2 the P RESET MUST directly follow the container onum)
    G 0 <onum>			<global limit>		0				0
	E 0 <onum>			<global limit>		<wear_location>	0 
(v1 wear_loc= numeric, v2 wear_loc is a wordflag from wear_location_types)
	D 0 <rnum>			<direction>			<door flags>	0 
(v2 D is not valid in a version 2 - currently silently ignored)
	R 0 <rnum>			<direction>			0				0
    S

NOTES:

  // mob to room reset
		M loads mobile mnum into room rnum, up to the max world_limit
          of mobs.

  // object related resets
	//to room
        O puts object onum into room rnum.
	//to object
        P puts contents_onum into container_onum.
		  Technically the container_onum, doesn't have to be an object
		  and can be nested - i.e. objects within other objects, within
		  other objects - can be an object carried by a mob also?
	//to mob inventory
        G puts onum into a mob's inventory. This MUST follow the M of
          the mob you wish to give the item to!
	//to mob equip
        E equips mob with onum on wear_location. This MUST follow the
          M of the mob you wish to give the item to!

  // exit related resets
		D sets the door facing "direction" in rnum to door_state
        R randomizes rnums exits. Put total number of exits into
          total_exits. (This reset is not documented, probably because
		  all right-thinking people realize this is a major annoyance)

  // reset list terminator S
        S Denotes the end of the #RESETS section
*/
/**************************************************************************/
extern RESET_DATA  *complete_resets_list;
void new_reset( ROOM_INDEX_DATA *pR, RESET_DATA *pReset );

/**************************************************************************/
// attaches all the resets to the rooms
// - Kal, July 01
void attach_resets()
{	
	logf("Attaching resets to rooms...");
	char bad_resets[MSL*4];
	char buf[MIL];
	bad_resets[0]='\0';

	ROOM_INDEX_DATA *pRoom;
	MOB_INDEX_DATA *pMob;
	OBJ_INDEX_DATA *pObj;

	ROOM_INDEX_DATA *pRoomOfLastMob=NULL; // room which the last mob was reset into
	ROOM_INDEX_DATA *pRoomOfLastObj=NULL; // room which the last obj was reset into

	RESET_DATA  *r=complete_resets_list;
	RESET_DATA  *r_next;

	pRoomOfLastMob=NULL;

	for ( ; r; r=r_next)
	{
		r_next=r->next;

		// Validate parameters.
		// We're calling the index functions for the side effect.
		switch ( r->command) // checks moved into fix_resets
		{
		default:
			// should never get here
			bugf( "attach_resets(): bad command '%c'.", r->command);
			do_abort();
			break;
		
		case 'M': // M 0 <mnum> <global mob limit> <rnum> <room mob limit>
			{
				// check the mob exists
				pMob=get_mob_index( r->arg1);
				if(!pMob){
					sprintf(buf, "attach_resets(): unfound mob %d for reset M 0 %d %d %d %d (removed)",
						r->arg1, r->arg1, r->arg2, r->arg3, r->arg4);
					bug(buf);
					strcat(bad_resets, buf);
					strcat(bad_resets, "`1");					

					pRoomOfLastMob=NULL;
					free_reset_data(r);
					continue;
				}

				// check the room exists
				pRoom = get_room_index ( r->arg3 );
				if(!pRoom){
					sprintf(buf, "attach_resets(): unfound room %d for reset M 0 %d %d %d %d (removed)",
						r->arg3, r->arg1, r->arg2, r->arg3, r->arg4);
					bug(buf);
					strcat(bad_resets, buf);
					strcat(bad_resets, "`1");					

					pRoomOfLastMob=NULL;
					free_reset_data(r);
					continue;
				}

				// put the M reset into the room
				new_reset( pRoom, r );
				pRoomOfLastMob=pRoom;
				continue;
			}
			break; 
			
		case 'O': // O 0 <onum> <global limit> <rnum>
			{ 
				// check the object exists
				pObj=get_obj_index( r->arg1);
				if(!pObj){
					sprintf(buf, "attach_resets(): unfound object %d for reset O 0 %d %d %d (removed)",
						r->arg1, r->arg1, r->arg2, r->arg3);
					bug(buf);
					strcat(bad_resets, buf);
					strcat(bad_resets, "`1");

					pRoomOfLastObj=NULL;
					free_reset_data(r);
					continue;
				}

				// check the room exists
				pRoom = get_room_index ( r->arg3 );
				if(!pRoom){
					sprintf(buf, "attach_resets(): unfound room %d for reset O 0 %d %d %d %d (removed)",
						r->arg3, r->arg1, r->arg2, r->arg3, r->arg4);
					bug(buf);
					strcat(bad_resets, buf);
					strcat(bad_resets, "`1");

					pRoomOfLastObj=NULL;
					free_reset_data(r);
					continue;
				}

				// put the O reset into the room
				new_reset( pRoom, r );
				pRoomOfLastObj=pRoom;
				continue;
			}
			break; // case O
			
		case 'P':	// P 0 <contents_onum> <global limit> <container_onum> <max_in_container>
			{	// NOTE: Because the code in load_resets() moves the P resets to appear 
				// directly after the object they load into, we know that pRoomOfLastObj 
				// relates to the room which the correct <container_onum> is being loaded into.
				if( !pRoomOfLastObj ){
					sprintf(buf, "attach_resets(): unfound room resetting container object %d for reset P 0 %d %d %d %d (removed)",
						r->arg3, r->arg1, r->arg2, r->arg3, r->arg4);
					bug(buf);
					strcat(bad_resets, buf);
					strcat(bad_resets, "`1");
					free_reset_data(r);
					continue;
				}

				new_reset( pRoomOfLastObj, r );
			}
			break; // case P
			
		case 'G':
		case 'E':
			{
				if(!pRoomOfLastMob){
					sprintf(buf, "attach_resets(): unfound room resetting mob for reset %c 0 %d %d %d %d (removed)",
						r->command, r->arg1, r->arg2, r->arg3, r->arg4);
					bug(buf);
					strcat(bad_resets, buf);
					strcat(bad_resets, "`1");
					free_reset_data(r);
				pRoomOfLastObj=NULL;
					continue;
				}
				new_reset( pRoomOfLastMob, r );
				pRoomOfLastObj=pRoomOfLastMob;
			}
			break; // case G and E
			
		case 'D':  // D's are converted then removed by load_resets()
			{
				bugf("attach_resets(): A 'D' reset was encounted!!!... these should be converted/removed by load_resets().");
				do_abort();
			}
			break;
			
		case 'R':
			{
				ROOM_INDEX_DATA *pRoomIndex;
				if ( r->arg2 < 0 || r->arg2 >=MAX_DIR) // Last Door.
				{
					bugf( "attach_resets: 'R': bad exit %d.", r->arg2 );
					exit_error( 1 , "attach_resets", "bad exit");
				}
				
				if ( ( pRoomIndex = get_room_index( r->arg1 ) ) ){
					new_reset( pRoomIndex, r );
				}
			}
			break;
		}
    }
	logf("All resets have been attached.");
	return;
}
/**************************************************************************/
int areaimport_translate_wear_locations_stock( int old_wear_location, int area_version)
{
	if(area_version>3){ // dawn versions of the wear locations don't need converting
		return old_wear_location;
	}

	// old, new
	int wear_translate_table[][2]={					
		{ 0,	WEAR_LIGHT},		
		{ 1,	WEAR_FINGER_L},
		{ 2,	WEAR_FINGER_R},
		{ 3,	WEAR_NECK_1},
		{ 4,	WEAR_NECK_2},
		{ 5,	WEAR_TORSO},
		{ 6,	WEAR_HEAD},
		{ 7,	WEAR_LEGS},
		{ 8,	WEAR_FEET},
		{ 9,	WEAR_HANDS},
		{ 10,	WEAR_ARMS},
		{ 11,	WEAR_SHIELD},
		{ 12,	WEAR_ABOUT},
		{ 13,	WEAR_WAIST},
		{ 14,	WEAR_WRIST_L},
		{ 15,	WEAR_WRIST_R},
		{ 16,	WEAR_WIELD},
		{ 17,	WEAR_HOLD},
		{ 18,	WEAR_FLOAT},
/*
		// stock rom only goes this far
		{ 19,	WEAR_SECONDARY},
		{ 20,	WEAR_LODGED_ARM},
		{ 21,	WEAR_LODGED_LEG},
		{ 22,	WEAR_LODGED_RIB},
		{ 23,	WEAR_SHEATHED},
		{ 24,	WEAR_CONCEALED},
		{ 25,	WEAR_EYES},
		{ 26,	WEAR_EAR_L},
		{ 27,	WEAR_EAR_R},
		{ 28,	WEAR_FACE},
		{ 29,	WEAR_ANKLE_L},
		{ 30,	WEAR_ANKLE_R},
		{ 31,	WEAR_BACK},
*/
		{ -1, -1} // -1 terminate the list		
	};


	int new_value;
	for(int i=0; wear_translate_table[i][0]!=-1; i++){
		if(old_wear_location==wear_translate_table[i][0]){
			new_value=wear_translate_table[i][1];
			if(new_value==WEAR_NONE){
				logf("areaimport_translate_wear_locations_stock(): converting wear location '%d', "
					"to WEAR_NONE.", old_wear_location);
			}
			return new_value;
		}
	}
	logf("areaimport_translate_wear_locations_stock(): unrecognised wear location '%d', "
		"converting it to WEAR_NONE.", old_wear_location);
	return WEAR_NONE; // translating it to WEAR_NONE
}
/**************************************************************************/
int areaimport_translate_wear_locations_format2( int old_wear_location, int area_version)
{
	if(area_version>3){ // dawn versions of the wear locations don't need converting
		return old_wear_location;
	}
	// old, new
	int wear_translate_table[][2]={					
		{ 0,	WEAR_LIGHT},		
		{ 15,	WEAR_FINGER_L},
		{ 16,	WEAR_FINGER_R},
		{ 6,	WEAR_NECK_1},
		{ 7,	WEAR_NECK_2},
		{ 9,	WEAR_TORSO},
		{ 2,	WEAR_HEAD},
		{ 22,	WEAR_LEGS},
		{ 24,	WEAR_FEET},
		{ 14,	WEAR_HANDS},
//		{ ,	WEAR_ARMS},
		{ 20,	WEAR_SHIELD},
		{ 8,	WEAR_ABOUT},
		{ 21,	WEAR_WAIST},
		{ 12,	WEAR_WRIST_L},
		{ 13,	WEAR_WRIST_R},
		{ 17,	WEAR_WIELD},
		{ 19,	WEAR_HOLD},
		{ 1,	WEAR_FLOAT},
		{ 18,	WEAR_SECONDARY},
		{ 11,	WEAR_LODGED_ARM},
//		{ ,	WEAR_LODGED_LEG},
//		{ ,	WEAR_LODGED_RIB},
//		{ ,	WEAR_SHEATHED},
//		{ ,	WEAR_CONCEALED},
//		{ ,	WEAR_EYES},
		{ 4,	WEAR_EAR_L},
		{ 5,	WEAR_EAR_R},
		{ 3,	WEAR_FACE},
		{ 23,	WEAR_ANKLE_L},
		{ 23,	WEAR_ANKLE_R},
		{ 10,	WEAR_BACK},

		{ -1, -1} // -1 terminate the list		
	};

	int new_value;
	for(int i=0; wear_translate_table[i][0]!=-1; i++){
		if(old_wear_location==wear_translate_table[i][0]){
			new_value=wear_translate_table[i][1];
			if(new_value==WEAR_NONE){
				logf("areaimport_translate_wear_locations_stock(): converting wear location '%d', "
					"to WEAR_NONE.", old_wear_location);
			}
			return new_value;
		}
	}
	logf("areaimport_translate_wear_locations_stock(): unrecognised wear location '%d', "
		"converting it to WEAR_NONE.", old_wear_location);
	return WEAR_NONE; // translating it to WEAR_NONE
}
/**************************************************************************/
int areaimport_translate_wear_locations_format3( int old_wear_location, int area_version)
{
	if(area_version>3){ // dawn versions of the wear locations don't need converting
		return old_wear_location;
	}

	// old, new
	int wear_translate_table[][2]={					
		{ 0,	WEAR_LIGHT},		
		{ 1,	WEAR_FINGER_L},
		{ 2,	WEAR_FINGER_R},
		{ 3,	WEAR_NECK_1},
		{ 4,	WEAR_NECK_2},
		{ 5,	WEAR_TORSO},
		{ 6,	WEAR_HEAD},
		{ 7,	WEAR_LEGS},
		{ 8,	WEAR_FEET},
		{ 9,	WEAR_HANDS},
		{ 10,	WEAR_ARMS},
		{ 11,	WEAR_SHIELD},
		{ 12,	WEAR_ABOUT},
		{ 13,	WEAR_WAIST},
		{ 14,	WEAR_WRIST_L},
		{ 15,	WEAR_WRIST_R},
		{ 16,	WEAR_WIELD},
		{ 17,	WEAR_HOLD},
		{ 18,	WEAR_FLOAT},

		// additional positions which rom doesn't have in format3
		{ 19,	WEAR_SECONDARY},
		{ 20,	WEAR_FACE},
		{ 21,	WEAR_EAR_L},
		{ 22,	WEAR_EAR_R},
		{ 23,	WEAR_BACK},

		{ -1, -1} // -1 terminate the list		
	};


	int new_value;
	for(int i=0; wear_translate_table[i][0]!=-1; i++){
		if(old_wear_location==wear_translate_table[i][0]){
			new_value=wear_translate_table[i][1];
			if(new_value==WEAR_NONE){
				logf("areaimport_translate_wear_locations_stock(): converting wear location '%d', "
					"to WEAR_NONE.", old_wear_location);
			}
			return new_value;
		}
	}
	logf("areaimport_translate_wear_locations_stock(): unrecognised wear location '%d', "
		"converting it to WEAR_NONE.", old_wear_location);
	return WEAR_NONE; // translating it to WEAR_NONE
}
/**************************************************************************/
// break up the area credits into dawns format
void areaimport_convert_credits(AREA_DATA *pArea)
{
	char nbuf[MSL];
	char nword[MSL];
	char *name;
	strcpy(nbuf,pArea->credits);
	name=nbuf;

	// area must be below version 4 and credits must 
	// have { and } in them to be considered convertible
	if(	pArea->version>3 
		|| count_char(nbuf, '{')<1 
		|| count_char(nbuf, '}')<1 ){
		return;
	}

	// ignore the leading {'s
	while (*name=='{'){
		name++;
	}
	
	// the first number - lower level
	name=one_argument(name,nword);
	if(is_number(nword)){ 
		pArea->low_level=atoi(nword);

		// the second number - upper level
		name=one_argument(name,nword);
		// chop off the trailing '}'
		if(nword[str_len(nword)-1]=='}'){
			nword[str_len(nword)-1]='\0';
		}
		if(is_number(nword)){ // first number
			pArea->high_level=atoi(nword);

			// next word becomes the credits
			name=one_argument(name,nword);
			pArea->credits=str_dup(nword);
		}else{
			pArea->low_level=-1;

		}
	}else{  // prob an { all } section
		char lcomment[MSL];
		lcomment[0]='\0';

		// copy all the text up till the }
		while(!IS_NULLSTR(nword) && nword[0]!='}'){
			strcat(lcomment, " ");
			strcat(lcomment, nword);
			name=one_argument(name,nword);
		}
		replace_string(pArea->lcomment, &lcomment[1]);

		// next word becomes the credits
		name=one_argument(name,nword);
		replace_string(pArea->credits, nword);
	}
}
/**************************************************************************/
void save_area_roominvitelist( AREA_DATA *pArea )
{
    FILE *fp;
	ROOM_INDEX_DATA *room;
    fclose( fpReserve );
	char newfilename[MIL];

	sprintf(newfilename, "%s%s.ril.save", BACKUP_AREA_RIL_DIR, pArea->file_name);
	// trim out the '.are' bit
	{
		char *are=strstr(newfilename, ".are.ril.save");
		if(are && are[13]==0){
			strcpy(are, ".ril.save");
		}
	}

	logf("save_area_roominvitelist(): saving ril to %s...", newfilename);
    if ( !( fp = fopen( newfilename, "w" ) ) )
    {
        bugf("save_area_roominvitelist(): fopen for write '%s' - error %d (%s)", 
			newfilename, errno, strerror( errno));
		exit_error( 1 , "save_area_roominvitelist", "fopen for write error");
    }

	int count=0;
	fprintf( fp, "#DAWNAREA_RoomInviteList\n" );
    for( int i = pArea->min_vnum; i <= pArea->max_vnum; i++ ){
		room=get_room_index(i);
		if(room && !IS_NULLSTR(room->owner) && !IS_NULLSTR(room->invite_list)){
			char *txt=ltrim_string(rtrim_string(room->invite_list));
			if(!IS_NULLSTR(txt)){
				count++;
				fprintf(fp, "RIL %d %s~\n", room->vnum, txt); // Room Invite List
			}
		}
	}
    fprintf( fp, "#$\n" );

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

	// rename area/*.are.ril to bak_area/*.are.old.ril, 
	// then bak_area/*.are.ril.save to area/invitelist/*.are.ril
	{
		char old_areafilename[MIL];
		sprintf(old_areafilename, "%s%s.ril.old", BACKUP_AREA_RIL_DIR, pArea->file_name);
		char areafilename[MIL];
		sprintf(areafilename, "%s%s.ril", AREA_RIL_DIR, pArea->file_name);
		// trim out the '.are' bit
		{
			char *are=strstr(areafilename, ".are.ril");
			if(are && are[8]==0){
				strcpy(are, ".ril");
			}
		}
#ifdef WIN32
		unlink(old_areafilename);
#endif
		if(file_exists(areafilename)){
			if(rename(areafilename,old_areafilename)!=0){
				bugf("Error %d occurred renaming '%s' to '%s'!.. exiting to avoid area file corruption.", 
					errno, areafilename, old_areafilename);
				exit_error( 1 , "save_area_roominvitelist", "rename error - file exists");
			}
		}
		if(count>0){
			if(rename(newfilename, areafilename)!=0){
				bugf("Error %d occurred renaming '%s' to '%s'!.. exiting to avoid area file corruption.", 
					errno, newfilename, areafilename);
				exit_error( 1 , "save_area_roominvitelist", "rename failed");
			}		
		}else{
			logf("No room invite list text for area, removing %s file.", areafilename);
			unlink(areafilename);
		}
	}
	logf("save_area_roominvitelist(): save complete.");
	
}
/**************************************************************************/
// these are used so bug() will report the correct filename and line number.
extern FILE	*fpArea; 
extern char	strArea[MIL];
/**************************************************************************/
void load_area_roominvitelist( AREA_DATA *pArea )
{
	ROOM_INDEX_DATA *room;

	sprintf(strArea, "%s%s.ril", AREA_RIL_DIR, pArea->file_name);

	// trim out the '.are' bit
	char *are=strstr(strArea, ".are.ril");
	if(are && are[8]==0){
		strcpy(are, ".ril");
	}
	
	if(!file_exists(strArea)){
		return;
	}
	
	if ( ( fpArea= fopen( strArea, "r" ) ) == NULL )
	{
		bugf("load_area_roominvitelist(): fopen for read '%s' - error %d (%s)", 
			strArea, errno, strerror( errno));
		exit_error( 1 , "load_area_roominvitelist", "fopen for read failed");
	}

	logf(" Reading in RIL  %s",strArea);

	char *word=fread_word(fpArea);
	if(str_cmp(word, "#DAWNAREA_RoomInviteList")){
		logf("load_area_roominvitelist(): reading in file %s, "
			"unexpected start of file '%s'- ignoring list.", strArea, word);
		fclose(fpArea);
		fpArea=NULL;
		return;
	}

	word=fread_word(fpArea); // read the 'RIL' keyword or eof marker '#$'
	while(str_cmp(word, "#$")){

		// the RIL keyword has just been read		
		if(str_cmp(word, "RIL")){ 
			bugf("load_area_roominvitelist(): reading in file %s, "
				"expecting 'RIL' keyword but found '%s' - ignoring rest of file.", strArea, word);
			fclose(fpArea);
			fpArea=NULL;
			return;
		}

		// read the room vnum
		word=fread_word(fpArea);
		if(!is_number(word)){
			bugf("load_area_roominvitelist(): reading in file %s, "
				"expecting room vnum directly after 'RIL' keyword but "
				"found '%s' - ignoring rest of file.", strArea, word);
			fclose(fpArea);
			fpArea=NULL;
			return;
		}
		int roomvnum=atoi(word);

		// apply the vnum offet to the room vnum
		apply_area_vnum_offset(&roomvnum);

		// read the invite list string
		word=fread_string(fpArea);

		// find the room
		room=get_room_index(roomvnum);

		if(room){
			replace_string(room->invite_list, FORMATF(" %s ",word));
		}else{
			logf("ignoring room invite list '%s' for room %d - unfound room.", word, roomvnum);

		}		
		free_string(word);
		word=fread_word(fpArea); // read the next 'RIL' keyword or eof marker '#$'
	}

	fclose(fpArea);
	fpArea=NULL;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
