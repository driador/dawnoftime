/**************************************************************************/
// corpse.cpp - saving of corpses across reboots etc
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"

/**************************************************************************/
void fwrite_obj( obj_data *obj, FILE *fp, int iNest, char* heading );
/**************************************************************************/
// saves player corpses to disk
void do_save_corpses(char_data *ch, char *)
{
    obj_data *obj;
	FILE *fp;

	ch->println("Saving objects in rooms with corpses...");
	char write_filename[MIL];
	sprintf(write_filename, "%s.write", CORPSES_FILE);

	// find a free write filename - so if something stuffed up on 
	// the write, we don't go over it.
	if(file_exists(write_filename)){
		for(int i=0; i<20; i++){
			sprintf(write_filename,"%s.write%d", CORPSES_FILE, i);
			if(!file_exists(write_filename)){
				break;
			}
		}
	}
	unlink(write_filename);

	fclose( fpReserve );
    if ( ( fp = fopen( write_filename, "w" ) ) == NULL ){
		bugf("do_save_corpses(): fopen '%s' failed for write - error %d (%s)",
			write_filename, errno, strerror( errno));
	    fpReserve = fopen( NULL_FILE, "r" );
		return;
    }

	ROOM_INDEX_DATA *r;
	// loop thru every room, if there are corpses in a room, we save the room
	for(int i=0; i<MAX_KEY_HASH; i++){
		for( r=room_index_hash[i % MAX_KEY_HASH]; r; r=r->next ){
			
			bool stay_in_room=true; // set to false if we find a corpse in the room
			for( obj= r->contents; obj && stay_in_room; obj=obj->next_content){
				if(obj->item_type==ITEM_CORPSE_PC){
					// we have found a room with a pc corpse, save the complete room
					assert(obj->in_room==r);
					fprintf(fp, "OBJECTS_IN_ROOM %d\n", obj->in_room->vnum);
					fwrite_obj(obj, fp, 0, "O");
					fprintf(fp, "\n");
					stay_in_room=false;
				}
			}
		}
	}

	int bytes_written=fprintf(fp, "EOF\n");
	fclose( fp );
	if(   bytes_written != str_len("EOF\n") ){
		bugf("Incomplete write of %s, write aborted - check diskspace! - error %d (%s)", 
			write_filename, errno, strerror( errno));
	    fpReserve = fopen( NULL_FILE, "r" );
	}else{
		logf("Renaming new %s to %s", write_filename, CORPSES_FILE);
		unlink(CORPSES_FILE);
		rename(write_filename, CORPSES_FILE);
		ch->println("Finished saving objects in rooms with corpses.");	
	}

    fpReserve = fopen( NULL_FILE, "r" );
}

/**************************************************************************/
obj_data * fread_obj    ( FILE *fp, const char *filename );
/**************************************************************************/
void do_load_corpses(char_data *ch, char *)
{
    obj_data *obj;
	FILE *fp;

	ch->println("Loading objects in rooms with pc corpses...");
	logf("Loading objects in rooms with pc corpses...");

	if(!file_exists(CORPSES_FILE)){
		ch->printlnf("No corpses file (%s) exists..",CORPSES_FILE);
		logf("No corpses file (%s) exists..",CORPSES_FILE);
		return;
	}

	fclose( fpReserve );
    if ( ( fp = fopen( CORPSES_FILE, "r" ) ) == NULL ){
		bugf("do_load_corpses(): fopen '%s' failed for read - error %d (%s)",
			CORPSES_FILE, errno, strerror( errno));
	    fpReserve = fopen( NULL_FILE, "r" );
		return;
    }

	int count=0;
	int corcount=0;
	int room_number=0;
	room_index_data *room=NULL;
			
	for( ; ; ){
		char *word= feof( fp ) ? (char*)"EOF" : fread_word( fp );
		
		if(!str_cmp(word,"EOF")){
			break;
		}

		if(!str_cmp(word,"OBJECTS_IN_ROOM")){
			room_number=fread_number(fp);
			room=get_room_index(room_number);
			
			// reset the read in nest
			int iNest;	
			for ( iNest = 0; iNest < MAX_NEST; iNest++ ){
				rgObjNest[iNest] = NULL;
			}
		}else if(!str_cmp(word,"#O")){
			obj=fread_obj(fp, CORPSES_FILE);
			if(obj){
				if(room){
					obj_to_room(obj, room);
					count++;
					if(obj->item_type==ITEM_CORPSE_PC){
						corcount++;
						logf("%d) %s to room %d", corcount, obj->short_descr, room_number);
						ch->printlnf("%d) %s to room %d", corcount, obj->short_descr, room_number);
					}
				}else{
					logf("room %d not found to load object into!", room_number);
				}				
			}
		}else{
			logf("unrecognised word in %s file '%s'", CORPSES_FILE, word);
		}
	}

	fclose(fp);
    fpReserve = fopen( NULL_FILE, "r" );

	logf("finished loading corpses, %d objects loaded (%d pc corpse%s).", 
		count, corcount, corcount==1?"":"s");
	ch->printlnf("finished loading corpses, %d object loaded (%d pc corpse%s).", 
		count, corcount, corcount==1?"":"s");

}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

