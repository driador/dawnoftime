/**************************************************************************/
// lockers.cpp - dawn locker system, written Feb 2002 by Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "lockers.h"
#include "ictime.h"

// Locker implementation notes:
//   * the content of all lockers are stored in a single flat file, 
//     depending on how popular the lockers are, it may make sense to 
//     save lockers based on roomvnum... but for now I haven't bothered.

/**************************************************************************/
lockers_object *lockers;
bool lockers_referenced; // if this is true, the lockers will be saved next tick
/**************************************************************************/
lockers_object::lockers_object()
{
	memset(hash, 0, sizeof(hash));
};
/**************************************************************************/
locker_data *lockers_object::get_first(int roomvnum) 
{ 
	return hash[roomvnum%LOCKER_HASH_KEY];
};	
/**************************************************************************/
// return the locker container object
OBJ_DATA *lockers_object::find_locker_object(char_data *ch, bool display_messages, int number)
{
	locker_data *locker;
	locker=find_locker(ch, display_messages, number);
	if(locker){
		locker_data_of_last_found_locker_object=locker;
		return locker->locker_object;
	}
	locker_data_of_last_found_locker_object=NULL;
	return NULL;
}
/**************************************************************************/
locker_data *lockers_object::find_locker_data(char *owner, int roomvnum, int number)
{
	locker_data *locker;
	// scan thru the list of lockers to find one
	for(locker=get_first(roomvnum); locker; locker=locker->next_locker){
		if(locker->roomvnum!=roomvnum){
			continue;
		}

		if(number){
			if(number!=locker->number){
				continue;
			}
		}else{ // no number specified, looking for the owners locker
			if(str_cmp(owner, locker->owner)){
				continue;
			}
		}
		return locker;
	}
	return NULL;
}
/**************************************************************************/
locker_data *lockers_object::find_locker(char_data *ch, bool display_messages, int number)
{
	if(!lockers->room_has_lockers(ch->in_room)){
		if(display_messages){
			ch->println("This room doesn't have any lockers.");
		}
		return NULL;
	}
	if(IS_NPC(ch)){
		ch->println("NPC's can't access lockers.");
		return NULL;
	}

	if(GAMESETTING3(GAMESET3_LOCKERS_DISABLED)){
		ch->println("The locker system is currently disabled.");
		return NULL;
	}
	
	// find a particular locker
	locker_data *locker;
	locker=find_locker_data(ch->name, ch->in_room_vnum(), number);
	if(locker){
		lockers_referenced=true;
		locker->last_accessed=current_time;
		return locker;
	}

	if(display_messages){
		if(number){
			ch->printlnf("Couldn't find rented locker%d in this room.", number);
		}else{
			ch->println("You don't own a locker in this room.");
		}
	}
	return NULL;
}
/**************************************************************************/
int lockers_object::count_used_lockers_in_room(int roomvnum)
{
	int count=0;
	locker_data *locker;

	// scan thru the list of lockers counting the matches
	for(locker=get_first(roomvnum); locker; locker=locker->next_locker){
		if(locker->roomvnum!=roomvnum){
			continue;
		}
		count++;
	}

	return count;
}

/**************************************************************************/
// return true if room player is in has lockers
bool lockers_object::room_has_lockers(ROOM_INDEX_DATA * room)
{
	if( !room || !room->lockers 
		|| !room->lockers->initial_rent)
	{
		return false;
	}
	return true;
}
/**************************************************************************/
// when a player starts renting a locker
void lockers_object::start_rent(char_data *ch, int locker_number)
{
	if(!lockers->room_has_lockers(ch->in_room)){
		ch->println("Lockers don't appear to be available in this room.");
		return;
	}
	if(IS_NPC(ch)){
		ch->println("NPC's can't rent lockers.");
		return;
	}

	if(GAMESETTING3(GAMESET3_LOCKERS_DISABLED)){
		ch->println("The locker system is currently disabled.");
		return;
	}

	if(find_locker(ch,false, 0)){
		ch->println("You already own a locker in this room.");
		return;
	}

	if(	ch->in_room->lockers->weight<1
		|| ch->in_room->lockers->capacity<1)
	{
		ch->wrapln("The lockers in this room are incorrectly setup, the locker "
			"weight and capacity must be greater than zero, talk to the admin.");
		return;
	}

	if(count_used_lockers_in_room(ch->in_room_vnum())>= ch->in_room->lockers->quantity){
		ch->println("There are no more lockers available in this room for rent.");
		return;
	}

	if(locker_number>ch->in_room->lockers->quantity){
		ch->printlnf("The locker numbering doesn't go as high as %d in this room.", locker_number);
		return;
	}

	if(locker_number){
		locker_data *l=find_locker_data("", ch->in_room_vnum(), locker_number);
		if(l){
			ch->printlnf("Locker %d is already taken.", locker_number);
			return;
		}
	}

	if(ch->gold<ch->in_room->lockers->initial_rent){
		ch->printlnf("The initial rent for lockers in this room cost %d gold... you only have %ld.",
			ch->in_room->lockers->initial_rent,
			ch->gold);
		return;
	}

	// create a new locker
	locker_data *locker=new locker_data;

	locker->owner=str_dup(ch->name);
	locker->access=str_dup("");
	locker->label=str_dup(ch->short_descr);
	locker->roomvnum=ch->in_room_vnum();
	if(locker_number){
		locker->number=locker_number;
	}else{
		locker->number=get_next_free_locker_number(locker->roomvnum);
	}
	locker->paid_until=current_time+ICTIME_IRLSECS_PER_MONTH;

	// create a locker object for the new locker
	obj_data *o;
	o=create_object(get_obj_index(OBJ_VNUM_DUMMY));	
	replace_string(o->name, FORMATF("locker%d", locker->number));
    replace_string(o->short_descr, FORMATF("locker %d", locker->number));
    replace_string(o->description, FORMATF("locker %d is here", locker->number));
    o->item_type    = ITEM_CONTAINER;
    o->extra_flags  = OBJEXTRA_NOPURGE | OBJEXTRA_BURN_PROOF | OBJEXTRA_NO_DEGRADE | OBJEXTRA_NO_RESTRING;
    o->extra2_flags = OBJEXTRA2_NODECAY | OBJEXTRA2_NOSELL | OBJEXTRA2_NOQUEST | OBJEXTRA_NOLOCATE;
    o->wear_flags	= OBJWEAR_NO_SAC;
    o->weight       = 0;
    o->cost         = 0;
    o->level        = 1;
    o->condition    = 100;
    replace_string(o->material, "metal");
    o->timer        = 0;
    o->absolute_size= 0;
    o->relative_size= 0;
	o->ospec_fun	= NULL;
	o->attune_id	= 0;
	o->attune_flags	= 0;
	o->attune_modifier= 0;
	o->attune_next	= 0;
	o->extra_descr=NULL;
	o->value[0]=ch->in_room->lockers->weight;
	
	// container flags
	if(ch->in_room->lockers->pick_proof==3){
		o->value[1]=CONT_CLOSEABLE | CONT_PICKPROOF | CONT_LOCKER;
	}else if(ch->in_room->lockers->pick_proof==2 && number_range(1,2)==1){
		// 50% chance of getting a pick proof locker if set to random
		o->value[1]=CONT_CLOSEABLE | CONT_PICKPROOF | CONT_LOCKER;
	}else{
		o->value[1]=CONT_CLOSEABLE | CONT_LOCKER; 
	}
	o->value[2]=0; // key vnum
	o->value[3]=ch->in_room->lockers->capacity;
	o->value[4]=0; // weight multiplier

	// link the locker object into the particular room hosting the locker
	o->in_room=ch->in_room;

	locker->locker_object=o;

	// put the new locker in the hash bucket based on room vnum
	locker->next_locker=hash[locker->roomvnum%LOCKER_HASH_KEY];
	hash[locker->roomvnum%LOCKER_HASH_KEY]=locker;

	ch->printlnf("You start renting locker %d for %d gold.", 
		locker->number, ch->in_room->lockers->initial_rent);
	ch->gold-=ch->in_room->lockers->initial_rent;
	ch->println("hint: don't forget to close and lock your locker when you aren't using it.");

	lockers_referenced=true;	
	locker->last_accessed=current_time;
}
/**************************************************************************/
// pay one IC year worth of rent
void lockers_object::pay_rent(char_data *ch, int locker_number)
{
	if(!lockers->room_has_lockers(ch->in_room)){
		ch->println("Lockers don't appear to be available in this room.");
		return;
	}
	if(IS_NPC(ch)){
		ch->println("NPC's can't rent lockers.");
		return;
	}

	if(GAMESETTING3(GAMESET3_LOCKERS_DISABLED)){
		ch->println("The locker system is currently disabled.");
		return;
	}

	locker_data *l=find_locker(ch,true, locker_number);
	if(!l){
		return;
	}

	if(l->paid_until> current_time + (3*ICTIME_IRLSECS_PER_YEAR)){
		ch->println("You can't pay more than 3 IC years in advance.");
		return;
	}

	if(ch->gold<ch->in_room->lockers->ongoing_rent){
		ch->printlnf("The ongoing rent for lockers in this room cost %d gold... you only have %ld.",
			ch->in_room->lockers->ongoing_rent,
			ch->gold);
		return;
	}

	ch->printlnf("You pay %d gold for an additional IC year worth of rent for your locker %d.", 
		ch->in_room->lockers->ongoing_rent, l->number);

	l->paid_until+=ICTIME_IRLSECS_PER_YEAR;
	ch->gold-=ch->in_room->lockers->ongoing_rent;

	ch->printlnf("Your locker rent is now paid until %s (%.24s)", 
		get_shorticdate_from_time(l->paid_until, "", 0),
		ctime(&l->paid_until));

	if(l->locker_object){
		SET_BIT(l->locker_object->value[1],CONT_CLOSEABLE);
	}

	ch->println("note: if your locker is not accessed for two IRL months, it will be automatically purged");

	lockers_referenced=true;	
	l->last_accessed=current_time;
}
/**************************************************************************/
GIO_START(locker_data)
	GIO_INT(roomvnum)
	GIO_SHINT(number)
	GIO_STR(owner)
	GIO_STR(access)
	GIO_STR(label)		
	GIO_LONG(paid_until)
	GIO_LONG(last_accessed)	
GIO_FINISH_STRDUP_EMPTY
/**************************************************************************/
void lockers_object::lockers_save_db()
{
	if(GAMESETTING3(GAMESET3_LOCKERS_DISABLED)){
		return;
	}

	if(!lockers_referenced){
		return;
	}

	locker_data *locker;

	char write_filename[MSL];

	sprintf(write_filename,"%s.write", LOCKERS_FILE);

	// find a free write filename - so if something stuffed up on 
	// the write, we don't go over it.
	if(file_exists(write_filename)){
		for(int i=0; i<20; i++){
			sprintf(write_filename,"%s.write%d", LOCKERS_FILE, i);
			if(!file_exists(write_filename)){
				break;
			}
		}
	}

	unlink(write_filename);
	logf("lockers_save_db(): saving lockers to %s.", write_filename);

	FILE *fp;
    if ( !( fp = fopen(write_filename, "w" ) ) )
    {
        bugf("lockers_save_db(): fopen '%s' for write - error %d (%s)",
			write_filename, errno, strerror( errno));
		exit_error( 1 , "lockers_save_db", "fopeen for write error");
    }

	// loop thru the list of lockers saving them
	time_t expire_at=current_time-(60*60*24* game_settings->days_lockers_stored_for);
	for(int h=0; h<LOCKER_HASH_KEY; h++){
		for(locker=hash[h]; locker; locker=locker->next_locker){
			if(locker->last_accessed<expire_at)
			{
				logf("Not saving expired locker for %s in room %d.", 
					locker->owner, locker->roomvnum);
				continue;
			}

			// lockers that are six IC months over due in rent (around 14 IRL days)
			if(locker->paid_until+(6*ICTIME_IRLSECS_PER_MONTH)<current_time){
				logf("Not saving unpaid locker for %s in room %d.", 
					locker->owner, locker->roomvnum);
				continue;
			}

			fprintf(fp,"#LOCKER\n");
			{
				char *tstr=locker->access;
				locker->access=ltrim_string(rtrim_string(locker->access));
				GIO_SAVE_RECORD(locker_data, locker, fp, NULL);
				locker->access=tstr;
			}
			if(locker->locker_object){
				fwrite_obj( locker->locker_object, fp, 0, "O");
			}
			fprintf(fp,"\n");
		}
	}
	int bytes_written=fprintf(fp, "EOF~\n");
	fclose( fp );
	if(bytes_written != str_len("EOF~\n") ){
        bugf("lockers_save_db(): fprintf to '%s' incomplete - error %d (%s)",
			write_filename, errno, strerror( errno));

		bugf("Incomplete write of %s, write aborted - check diskspace!", write_filename);
		{
						
			autonote(NOTE_SNOTE, 
				"lockers_save_db()", 
				"Problems saving lockers database!", 
				"code cc: imm", 
				FORMATF("Incomplete write of %s, write aborted - check diskspace!\r\n", write_filename),
				true);
		}
	}else{
		// maintain a backup
		{
			char buf[MIL];
			sprintf(buf, "%s.giobak", LOCKERS_FILE);
			logf("lockers_save_db: Renaming old %s to %s", LOCKERS_FILE, buf);
			unlink(buf);
			rename(LOCKERS_FILE, buf);
		}

		logf("Renaming new %s to %s", write_filename, LOCKERS_FILE);
		unlink(LOCKERS_FILE);
		rename(write_filename, LOCKERS_FILE);
	}
	lockers_referenced=false;
}
/**************************************************************************/
void lockers_object::lockers_load_db()
{
	int zero_numbered_lockers=0;
	// dynamically allocate object memory to itself if required
	if(!this){
		lockers= new lockers_object;
		lockers->lockers_load_db();
		return;
	}

	
	lockers_total_count=0;
	lockers_object_count=0;

	log_string("lockers_load_db(): loading lockers database.");
	lockers_referenced=false;

	locker_data *locker=NULL;

	if(!file_exists(LOCKERS_FILE)){
		logf("lockers_load_db(): lockers database file '%s' not found.", LOCKERS_FILE);
		return;
	}

	FILE *fp;
    if ( !( fp = fopen(LOCKERS_FILE, "r" ) ) )
    {
        bugf("lockers_load_db(): fopen '%s' for read - error %d (%s)",
			LOCKERS_FILE, errno, strerror( errno));
		exit_error( 1 , "lockers_load_db", "fopeen for read error");
    }

	bool morefile=true;
	locker_data *last_locker=NULL;

	// loop thru till we get the end of the file
	while (morefile && !feof(fp)) {
		char *readword= fread_word(fp);

		if (!str_cmp(readword, "EOF") || !str_cmp(readword, "EOF~")){
			morefile=false;
		}else{
			if(!str_cmp(readword, "#LOCKER")){					
				locker=new locker_data;
				// read in the data about the locker
				GIO_LOAD_RECORD(locker_data, locker, fp);

				// update the access text with spaces around the sides
				if(!IS_NULLSTR(locker->access)){
					char *tstr=locker->access;
					locker->access=str_dup(FORMATF(" %s ", ltrim_string(rtrim_string(locker->access))));
					free_string(tstr);
				}

				// confirm the locker number is unique
				if(locker->number){
					if(find_locker_data("", locker->roomvnum, locker->number)){
						bugf("Duplicated locker number %d for room %d, owner=%s, renumbering it.",
							locker->number,
							locker->roomvnum,
							locker->owner);
						locker->number=0; // set it to zero, and we can reallocate it later
						zero_numbered_lockers++;
					}
				}else{
					zero_numbered_lockers++;
				}

				// put the new locker in the hash bucket based on room vnum
				locker->next_locker=hash[locker->roomvnum%LOCKER_HASH_KEY];
				hash[locker->roomvnum%LOCKER_HASH_KEY]=locker;
				last_locker=locker;
				// initialise the locker object to NULL, as locker object will follow
				locker->locker_object=NULL;
				lockers_total_count++;
			}else if(!str_cmp(readword, "#O")){
				OBJ_DATA *o=fread_obj( fp, LOCKERS_FILE);					
				lockers_object_count++;
				if(o){
					if(last_locker->locker_object){
						obj_to_obj(o, last_locker->locker_object);
					}else{						
						last_locker->locker_object=o;
						last_locker->locker_object->in_room=get_room_index(last_locker->roomvnum);				
						// if the locker rental has expired, open it, and make it uncloseable.
						if(last_locker->paid_until<current_time){
							REMOVE_BIT(o->value[1],CONT_CLOSEABLE);
							REMOVE_BIT(o->value[1],CONT_CLOSED);
							REMOVE_BIT(o->value[1],CONT_LOCKED);
						}
					}
				}
			}else {// unexpected file format
				bugf("Unexpected fileformat in '%s' - found '%s' "
					"expecting '#LOCKER or #O'", LOCKERS_FILE, readword);
				write_shutdown_file(NULL);
				do_abort();
				return;
			}
		}
	}
	fclose( fp );

	// loop thru, allocating new locker numbers if required
	if(zero_numbered_lockers){
		logf("%d zero numbered locker%s, allocating new numbers.",
			zero_numbered_lockers,
			zero_numbered_lockers==1?"":"s");

		for(int h=0; h<LOCKER_HASH_KEY; h++){
			for(locker=hash[h]; locker; locker=locker->next_locker){
				if(!locker->number){
					locker->number=get_next_free_locker_number(locker->roomvnum);
					if(locker->locker_object){
						replace_string(locker->locker_object->name, FORMATF("locker%d", locker->number));
						replace_string(locker->locker_object->short_descr, FORMATF("locker %d", locker->number));
						replace_string(locker->locker_object->description, FORMATF("locker %d is here", locker->number));
					}
				}
			}
		}
	}

	logf("Finished reading lockers from " LOCKERS_FILE ". (read in %d lockers, %d objects)",
		lockers_total_count, lockers_object_count);
}
/**************************************************************************/
// return the next available locker number in the room
int lockers_object::get_next_free_locker_number(int roomvnum)
{
	int looking_for;
	for(looking_for=1; ;looking_for++){
		if(!find_locker_data("", roomvnum, looking_for)){
			return looking_for;
		}
	}
	return -1; // shouldn't be able to get here
}

/**************************************************************************/
void lockers_object::info(char_data *ch, int locker_number)
{
	ch->titlebar("LOCKERS");
	int lock_in_use=lockers->count_used_lockers_in_room(ch->in_room_vnum());
	ch->printlnf("This room has %d locker%s, of which %d %s in use.",
		ch->in_room->lockers->quantity,
		ch->in_room->lockers->quantity==1?"":"s",
		lock_in_use,
		lock_in_use==1?"is":"are");

	ch->printlnf("It costs %d gold to start renting a locker (1 month rent inclusive).",
		ch->in_room->lockers->initial_rent);
	ch->printlnf("It costs %d gold per year for ongoing rental.",
		ch->in_room->lockers->ongoing_rent);
	ch->printlnf("New lockers can store up to %d lbs total.",
		ch->in_room->lockers->weight);
	ch->printlnf("The largest individual item which will fit in these lockers is %d lbs.",
		ch->in_room->lockers->capacity);

	locker_data *l;
	l=find_locker(ch, false, IS_IMMORTAL(ch)?locker_number:0);
	if(l){
		if(is_exact_name(ch->name, l->owner)){
			ch->titlebarf("YOUR LOCKER (#%d)", l->number);
		}else{
			ch->titlebarf("LOCKER %d", l->number);
		}
		if(l->paid_until<current_time){
			ch->println("YOUR LOCKER RENT HAS LAPSED, THEREFORE YOU CAN'T CLOSE IT!");
			ch->printlnf("Your locker rent expired %s (%.24s)", 
				get_shorticdate_from_time(l->paid_until, "", 0),
				ctime(&l->paid_until));
		}else{
			ch->printlnf("Your locker rent is paid until %s (%.24s)", 
				get_shorticdate_from_time(l->paid_until, "", 0),
				ctime(&l->paid_until));
		}
		if(l->locker_object){
			ch->printlnf("Your locker is currently %s", 
				!IS_SET(l->locker_object->value[1], CONT_CLOSED)?"open.":
					IS_SET(l->locker_object->value[1], CONT_LOCKED)?"locked.":"closed.");
		}
		if(IS_NULLSTR(ltrim_string(rtrim_string(l->access)))){
			ch->println("Your lockers access settings are currently unset.");
		}else{
			ch->println("Your lockers access settings are currently:");
			ch->printlnf("  %s", l->access);
		}
	}
	ch->titlebar("");
}
/**************************************************************************/
void lockers_object::look(char_data *ch)
{
	ch->titlebar("ROOM LOCKERS");

	int lock_in_use=lockers->count_used_lockers_in_room(ch->in_room_vnum());
	ch->printlnf("This room has %d locker%s, of which %d %s in use.",
		ch->in_room->lockers->quantity,
		ch->in_room->lockers->quantity==1?"":"s",
		lock_in_use,
		lock_in_use==1?"is":"are");

	locker_data *locker;

	int i;
	int max=0;
	int roomvnum=ch->in_room_vnum();
	// find the maximum locker number
	for(locker=get_first(ch->in_room_vnum()); locker; locker=locker->next_locker){
		if(locker->roomvnum==roomvnum){
			max=UMAX(max, locker->number);
		}	
	}
	
	for(i=0; i<=max; i++){
		for(locker=get_first(roomvnum); locker; locker=locker->next_locker){
			if(locker->roomvnum!=roomvnum){
				continue;
			}
			if(locker->number!=i){
				continue;
			}

			if(has_access(ch, locker)){ // highlight lockers player has access to
				ch->print("`W");
			}
			if(locker->locker_object){
				ch->printf("[%3d]`x %s %s`x", 
					locker->number,
					IS_NULLSTR(locker->label)?"`S???`x":locker->label,
					!IS_SET(locker->locker_object->value[1],CONT_CLOSEABLE)?"<jammed open - expired lease>":
						!IS_SET(locker->locker_object->value[1], CONT_CLOSED)?"<open>":"<closed>");
			}else{
				ch->printlnf("Error: locker %d doesn't have a storage object!", locker->number);
			}

			if(IS_ADMIN(ch)){
				ch->printf("`S %s%s",
					locker->locker_object && IS_SET(locker->locker_object->value[1], CONT_LOCKED)?"<locked>":"<UNLOCKED>",
					locker->locker_object && IS_SET(locker->locker_object->value[1], CONT_PICKPROOF)?"<pickproof>":"");
				ch->printlnf(" [%s] {%s}`x", locker->owner, locker->access);
			}else{
				ch->println("");
			}
			break;
		}
	}
	if(IS_ADMIN(ch)){
		ch->println("`Sadminnote: [] = owner, {} = access`x");
	}

	ch->titlebar("");
}

/**************************************************************************/
void lockers_object::roomlist(char_data *ch, char *nametext)
{
	char name[MIL];
	one_argument(nametext, name);
	ch->titlebar("LOCKERS ROOMLIST");

	bool namesearch=false;
	if(!IS_NULLSTR(name)){
		namesearch=true;
	}

	int minvnum=1;
	int maxvnum=top_vnum_room;

	
	// now list all rooms with lockers in them
	int i;
	locker_data *locker;
	ROOM_INDEX_DATA *r;
	int h=0;
	bool found;
	for(i=minvnum; i<=maxvnum; i++){
		r=get_room_index(i);
		if(r && r->lockers){

			if(namesearch){
				found=false;
				for(locker=get_first(i); locker; locker=locker->next_locker){
					if(locker->roomvnum!=i){
						continue;
					}
					if(!str_cmp(locker->owner, name) || is_exact_name(name, locker->access)){
						found=true;
						break;
					}
				}
				if(!found){
					continue;
				}
			}

			h++;
			ch->printlnf("%3d) [%s%s`x] (%2d/%2d) %s (%s%s`x)",
				h, 
				colour_table[(i%14)+1].code,
				mxp_create_tagf(ch, "rmvnum", "%5d", i),				
				lockers->count_used_lockers_in_room(i),
				r->lockers->quantity,
				r->name, 
				colour_table[(r->area->vnum%14)+1].code,
				r->area->name);
		}		
	}
	if(h==0){
		if(namesearch){
			ch->printlnf("There are no lockers in any rooms which '%s' has access to (note: clan access not checked).", name);
		}else{
			ch->println("There are no rooms in the game with lockers configured.");
		}
	}

	ch->titlebar("");
}

/**************************************************************************/
bool lockers_object::has_access(char_data *ch, locker_data *l)
{
	if(!str_cmp(l->owner, ch->name) || is_exact_name(ch->name, l->access)){
		lockers_referenced=true;
		l->last_accessed=current_time;
		return true;
	}
	// check clan access
	if(ch->clan){		
		if(is_exact_name(FORMATF("clan=%s", ch->clan->notename()), l->access)){
			lockers_referenced=true;
			l->last_accessed=current_time;
			return true;
		}
	}
	return false; // they didn't have access
}
/**************************************************************************/
// admin deleting a particular locker
bool lockers_object::has_access(char_data *ch, OBJ_DATA *locker_object)
{
	if(IS_IMMORTAL(ch)){
		return true; // imms always have access to lock/unlock all lockers
	}
	if(!locker_data_of_last_found_locker_object || 
		locker_data_of_last_found_locker_object->locker_object!=locker_object)
	{
		// we don't bother searching for it, if we don't have the last
		// referenced object cached, we forget about it.
		return false;
	}
	if(IS_NPC(ch)){
		return false;
	}
	if(IS_SET(ch->dyn,DYN_IS_BEING_ORDERED)){
		// no access while being ordered
		return false;
	}
	return has_access(ch, locker_data_of_last_found_locker_object);
}
/**************************************************************************/
// remove access from a locker
void lockers_object::remove_access(char_data *ch, char *argument, int locker_number)
{
	locker_data *l;
	// find the locker
	l=find_locker(ch, true, locker_number);
	if(!l){
		return;
	}

	if(!IS_ADMIN(ch) && str_cmp(ch->name, l->owner)){
		ch->printlnf("You can't change the access settings of locker %d.",
			l->number);
		return;
	}

	char arg[MIL];
	one_argument(argument, arg);
	smash_tilde(arg);

	if(has_colour(arg)){
		ch->println("You can't use colour codes here.");
		return;
	}

	if(!is_exact_name(arg, l->access)){
		ch->printlnf("'%s' doesn't have access to locker %d.", arg, l->number);
		ch->printlnf("Access is restricted to: %s", ltrim_string(rtrim_string(l->access)));
		return;
	}

	// remove them from the locker's access list
	l->access=string_replace_all(l->access, FORMATF(" %s ", arg), " ");
	l->access=string_replace_all(l->access, "  ", " ");

	ch->printlnf("'%s' no longer has access to locker %d.", arg, l->number);
	ch->printlnf("Access is now restricted to:%s", l->access);
}
/**************************************************************************/
void lockers_object::changeowner(char_data *ch, char *argument, int locker_number)
{
	locker_data *l;
	// find the locker
	l=find_locker(ch, true, locker_number);
	if(!l){
		return;
	}
	ch->printlnf("locker %d owner changed from '%s' to '%s'",
		l->number, l->owner, argument);
	replace_string(l->owner, argument);

	lockers_referenced=true;	
	l->last_accessed=current_time;
}
/**************************************************************************/
void lockers_object::changelabel(char_data *ch, char *argument, int locker_number)
{
	locker_data *l;
	// find the locker
	l=find_locker(ch, true, locker_number);
	if(!l){
		return;
	}
	ch->printlnf("locker %d label changed from '%s' to '%s'",
		l->number, l->label, argument);
	replace_string(l->label, argument);	

	lockers_referenced=true;	
	l->last_accessed=current_time;
}
/**************************************************************************/
void lockers_object::tempopen(char_data *ch, char *argument, int locker_number)
{
	locker_data *l;
	// find the locker
	l=find_locker(ch, true, locker_number);
	if(!l){
		return;
	}

	if(!l->locker_object){
		ch->printlnf("For some reason, locker %d (%s) doesn't have a locker object!", 
			l->number, l->owner);
		return;
	}
	ch->printlnf("LOCKER %d (%s) TEMPOPEN START:", l->number, l->owner);
	
	// backup the lockers open status
	int locker_flags=l->locker_object->value[1];

	// open the locker the bit way
	REMOVE_BIT(l->locker_object->value[1],CONT_CLOSED);
	REMOVE_BIT(l->locker_object->value[1],CONT_LOCKED);

	// run the command
	interpret(ch, argument);

	// restore the lockers open status
	l->locker_object->value[1]=locker_flags;
	ch->printlnf("LOCKER %d (%s) TEMPOPEN END:", l->number, l->owner);
	
	lockers_referenced=true;	
	l->last_accessed=current_time;
}

/**************************************************************************/
// grant access to a locker
void lockers_object::grant_access(char_data *ch, char *argument, int locker_number)
{
	locker_data *l;
	// find the locker
	l=find_locker(ch, true, locker_number);
	if(!l){
		return;
	}

	if(!IS_ADMIN(ch) && str_cmp(ch->name, l->owner)){
		ch->printlnf("You can't change the access settings of locker %d.",
			l->number);
		return;
	}

	char arg[MIL];
	one_argument(argument, arg);
	smash_tilde(arg);

	if(has_colour(arg)){
		ch->println("You can't use colour codes here.");
		return;
	}

	if(is_exact_name(arg, l->access)){
		ch->printlnf("'%s' already has access to locker %d.", arg, l->number);
		ch->printlnf("Access is restricted to:%s", l->access);
		return;
	}

	// add them to the locker's access list
	if(str_len(l->access)> MIL){
		ch->printlnf("Too many names listed with access to locker %d.", l->number);
		ch->printlnf("Access is restricted to:%s", l->access);
		return;
	}

	if(IS_NULLSTR(l->access)){
		replace_string(l->access, FORMATF(" %s ", arg));
	}else{
		char *f=FORMATF("%s%s ", l->access, arg);
		replace_string(l->access, f);
	}
	
	ch->printlnf("access granted to '%s' for locker %d.", arg, l->number);
	ch->printlnf("Access is now restricted to: %s", l->access);
}

/**************************************************************************/
// admin deleting a particular locker
void lockers_object::admin_delete(char_data *ch, int locker_number)
{
	locker_data *l;
	// find the locker
	l=find_locker(ch, true, locker_number);
	if(!l){
		return;
	}

	// we have the locker object, move all objects out of it onto the floor
	ch->printlnf("Locker %d data found...", l->number);
	obj_data *obj=l->locker_object;
	obj_data *obj_next_content;
	if(obj){
		ch->println("Locker container  object exists, moving objects to floor...");
		for( obj=obj->contains; obj; obj = obj_next_content ){
			obj_next_content=obj->next_content;
			obj_from_obj(obj);
			obj_to_room(obj, ch->in_room);
			ch->printlnf("  %s`x", format_obj_to_char(obj, ch, true));
		}
		ch->println("Deallocating locker container object...");
		l->locker_object->in_room=NULL;
		extract_obj(l->locker_object);
		l->locker_object=NULL;
	}	

	ch->printlnf("Deallocating locker %d data...", l->number);
	
	// find the locker data's position in the list
	locker_data *prev=NULL;
	locker_data *node;
	for(node=get_first(l->roomvnum); node; node=node->next_locker){
		if(l!=node){
			prev=node;
			continue;
		}
		// we have found the locker, remove it from the listing
		if(prev){
			prev->next_locker=l->next_locker;
		}else{ // first in the list
			hash[l->roomvnum%LOCKER_HASH_KEY]=l->next_locker;

		}
		// deallocate locker data
		ch->printlnf("Locker %d successfully removed.", l->number);
		delete l;
		return;
	}

	// shouldn't get here
	ch->printlnf("Couldn't find locker %d to remove!?!", l->number);
	return;
}

/**************************************************************************/
void lockers_object::postletter(char_data *ch, int locker_number)
{
	obj_data *letter;

	letter= get_eq_char(ch,WEAR_HOLD);
    if( !letter || letter->item_type != ITEM_PARCHMENT )
    {
        ch->println( "You must be holding the letter you want to post in your hands to post it." );
        return;
    }

	locker_data *l;
	// find the locker
	l=find_locker(ch, true, locker_number);
	if(!l){
		return;
	}

	if(!l->locker_object){
		ch->printlnf("For some reason locker %d doesn't have a locker object, "
			"please report this to the admin.",
			l->number);
		return;
	}
	// note: by design there is no limit on the number of letters 
	// you can put in a locker.  This way a room can be setup as postboxes,
	// where the weight and capacity of the lockers are 1... not fitting any
	// decent sized object.
	obj_from_char(letter);
	obj_to_obj(letter, l->locker_object);

	act("You post a letter into $p.", ch, l->locker_object, NULL, TO_CHAR );
	act("$n posts a letter into $p.",ch,l->locker_object,NULL,TO_ROOM);
}
/**************************************************************************/
void locker_notes(char_data *ch)
{
	ch->titlebar("LOCKER NOTES");
	ch->println("syntax: buy locker           - purchase the first available locker in the room.");
	ch->println("syntax: buy locker5          - attempt to purchase locker number 5 in the room.");
	ch->println("syntax: look lockers         - list all lockers in the room.");
	ch->println("syntax: look in locker       - look inside your locker.");
	ch->println("syntax: look in locker2      - look inside locker2.");
	ch->println("syntax: lock loc             - close and lock your locker.");
	ch->println("syntax: lock locker23        - close and lock locker 23 (if you have lock access).");
	ch->println("syntax: open loc             - unlock and open your locker.");
	ch->println("syntax: open locker8         - unlock (if you have lock access) and open locker 8.");
	ch->println("syntax: put bread in locker5 - put the bread in locker 5 (assuming it is open).");
	ch->titlebar("");
}
/**************************************************************************/
void do_lockers(char_data *ch, char *argument)
{	
	char arg[MIL];
	int locker_number=0;

	if(GAMESETTING3(GAMESET3_LOCKERS_DISABLED)){
		ch->println("The locker system is currently disabled.");
		return;
	}

	smash_tilde(argument);

	// no ordering to access lockers
	if(IS_SET(ch->dyn,DYN_IS_BEING_ORDERED)){
		if(ch->master){
			ch->master->println( "Not going to happen.");
		}
		return;
	}
	

	if(IS_NULLSTR(argument)){
		ch->titlebar("LOCKER COMMANDS");
		ch->println("syntax: locker notes - display notes on using lockers");
		ch->println("syntax: locker # postletter - post the letter you are holding into locker #");
		ch->println("syntax: locker info - show info on lockers in the room, and your rent status");
		ch->println("syntax: locker payrent - pay an IC years worth of rent");
		ch->println("");
		ch->println("== access commands for those renting a locker:");
		ch->println("syntax: locker grantaccess <name> - give <name> access to locker");
		ch->println("syntax: locker grantaccess clan=<clan_notename>");
		ch->println("              - give all members in clan <clan_notename> access to locker");
		ch->println("syntax: locker removeaccess <name> - remove access to locker from <name>");
		ch->println("");
		if(IS_ADMIN(ch)){
			ch->println("== admin only locker commands:");
			ch->println("syntax: locker roomlist [name] - list rooms with lockers in them [for name]"); 
			ch->println("syntax: locker changeowner - changes the owner of a given locker");
			ch->println("syntax: locker changelabel <label text> - change a label on locker");//
			ch->println("syntax: locker # delete - delete a locker, any contents are moved to ground");
			ch->println("syntax: locker # tempopen <command> - flags a locker open for <command>");
			ch->println("                                 e.g. locker 5 tempopen look in locker5");
			ch->println("");
		}
		ch->println("note: # = locker number, if you omit this number your locker will be used");
		ch->titlebar("");
		return;
	}

	argument=one_argument(argument, arg);
	// support the optional locker number
	if(is_number(arg)){
		locker_number=atoi(arg);
		if(locker_number<1){
			ch->println("The locker numbering starts at 1.");
			return;
		}
		// shift arguments left one
		argument=one_argument(argument, arg);
	}


	// admin room list command, can be used in any room
	if(IS_ADMIN(ch)){
		if(!str_prefix(arg, "roomlist")){
			lockers->roomlist(ch, argument);
			return;
		}
	}

	if(!str_prefix(arg, "notes")){
		locker_notes(ch);
		return;
	}

	// all commands below here must be used in a room with lockers
	if(!lockers->room_has_lockers(ch->in_room)){
		ch->println("This room doesn't have any lockers.");
		return;
	}
	if(IS_NPC(ch)){
		ch->println("NPC's can't access lockers.");
		return;
	}

	if(!str_prefix(arg, "info")){
		lockers->info(ch, locker_number);
		return;
	}

	if(!str_prefix(arg, "look")){
		lockers->look(ch);
		return;
	}

	if(!str_cmp(arg, "grantaccess")){
		if(IS_NULLSTR(argument)){
			ch->println("You need to specify a name after the grantaccess command.");
			return;
		}
		lockers->grant_access(ch, argument, locker_number);
		return;
	}	
	if(!str_cmp(arg, "removeaccess")){
		if(IS_NULLSTR(argument)){
			ch->println("You need to specify a name after the removeaccess command.");
			return;
		}
		lockers->remove_access(ch, argument, locker_number);
		return;
	}		

	if(!str_cmp(arg, "postletter")){
		if(locker_number==0){
			ch->println("specify the locker number you want to post into... e.g. 'locker 5 postletter'");
			return;
		}
		lockers->postletter(ch, locker_number);
		return;
	}	

	if(!str_cmp(arg, "startrent")){
		lockers->start_rent(ch, locker_number);
		return;
	}	

	if(!str_cmp(arg, "payrent")){
		if(!IS_IMMORTAL(ch) && locker_number!=0){
			ch->println("You can not pay the rent on someone elses locker... just type 'locker payrent' to pay your rent.");
			return;
		}
		lockers->pay_rent(ch, locker_number);
		return;
	}	


	// admin only commands
	if(IS_ADMIN(ch)){
		if(!str_cmp(arg, "delete")){
			lockers->admin_delete(ch, locker_number);
			return;
		}
		if(!str_cmp(arg, "changeowner")){
			lockers->changeowner(ch, argument, locker_number);
			return;
		}

		if(!str_cmp(arg, "changelabel")){
			lockers->changelabel(ch, argument, locker_number);
			return;
		}

		if(!str_cmp(arg, "tempopen")){
			lockers->tempopen(ch, argument, locker_number);
			return;
		}
	}


	ch->printlnf("Unrecognised lockers option '%s'", arg);
	do_lockers(ch,"");

}
/**************************************************************************/
/**************************************************************************/


