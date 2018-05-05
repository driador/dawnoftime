/**************************************************************************/
// track.cpp - Incomplete track system written by Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "track.h"

void resort_tracks();

int rooms_with_tracks=0;
unsigned short tracktime=1500;
C_track_table *track_table=NULL;

#define	MTPR	MAX_TRACKS_PER_ROOM
/**************************************************************************/
void init_track_table()
{
	log_string("init_track_table():: Initialising track_table...");
	track_table=new C_track_table;

	if(MAX_RACE>32000){
		bugf( __FILE__":init_track_table() - MAX_RACE is set to %d, which is higher than 32000...\n"
			"Track wont function correctly in this environment without modifications.  Aborting startup.", MAX_RACE);
		exit_error( 1 , "init_track_table", "MAX_RACE too high");
	}
	log_string("init_track_table():: track_table initialised.");
}
/**************************************************************************/
// constructor 
C_track_table::C_track_table()
{
	// NULL out all the tracks to start with 
	for(int i=0;i<MTC; i++){
		character[i]=NULL;
		race_oldchar[i]=0;
	}
	total_tracked_characters=0;
	next_free_track=0;
}
/**************************************************************************/
// returns true if the character field is pointing to a valid char_data ch
// - this will only be true if that ch is in the game currently
bool C_track_table::is_active(int index)
{		
	if(IS_SET(race_oldchar[index], 0x80)){
		return false;
	}else{
		if(character[index]==NULL){
			return false;
		}
		return true;
	}
}
/**************************************************************************/
int C_track_table::add_char(char_data *ch)
{	
	int trackindex=next_free_track;
	character[trackindex]=ch; 
	// MSB (most significant bit) of race_oldchar is only set to 1 after
	// the character has been freed, then the character pointer no longer
	// points to something of char_data but becomes a flags field recording
	// info about what the character was e.g. npc/pc, immortal etc.
	race_oldchar[trackindex]=ch->race & 0x7F; // ensure MSB is off for now

	total_tracked_characters++;

	// find where our next free track index is 
	int count=0;
	do{	++next_free_track%=MTC;
		count++;
		if(count>MTC){
			bugf( __FILE__":C_track_table::add_char() - MTC is set to %d which is less than", MTC);
			bug("the number of players + mobs in the game!  Track can't run unless MTC is increased");
			bug("(MTC is short for MAX_TRACKABLE_CHARACTERS_IN_GAME which is set in params.h)");
			bug("Increase it by say 2500, do a clean recompile then restart the mud.)");
			do_abort();
		}
	}while(is_active(next_free_track));

	return (trackindex);
}
/**************************************************************************/
void C_track_table::del_char(int index)
{
	assert(index>=0);
	assert(index<MTC);

	if(is_active(index)){
		bitflags[index]=0x8; // mark the character field
							 // as unused
		// record what we want to record from the character
		// before it is freed
		if(IS_NPC(character[index])){
			bitflags[index]|=0x01; // first bit records is_npc status
		}else{
			bitflags[index]=0x00;
			if(IS_IMMORTAL(character[index])){
				bitflags[index]|=0x02; // second bit records imm status
			}
		}
		total_tracked_characters--;
	}else{
		bugf(__FILE__":C_track_table::del_char(int) - index %d was previously "
			"deleted!", index);
	}
}
/**************************************************************************/
void C_track_table::del_char(char_data *ch)
{
	assert(ch->track_index>=0);
	assert(ch->track_index<MTC);
	
	if(character[ch->track_index]!=ch){
		bugf(__FILE__":C_track_table::del_char(CD*) - ch != character[ch->track_index]");
	}else{
		del_char(ch->track_index);
	}
}
/**************************************************************************/
int C_track_table::get_race_value(int index)
{
	assert(index>=0);
	assert(index<MTC);

	if(is_active(index)){ // temp hack till cleaned up
		race_oldchar[index]=character[index]->race & 0x7F; // ensure MSB is off for now
	}
	return (int)(race_oldchar[index] & 0x7f); // result less the MSB
}
/**************************************************************************/
char *C_track_table::get_race(int index)
{
	assert(index>=0);
	assert(index<MTC);

	return (race_table[get_race_value(index)]->name); 
}
/**************************************************************************/
int C_track_table::get_total_tracked_characters()
{
	return total_tracked_characters;	
}

/**************************************************************************/
bool C_track_table::is_npc(int index)
{
	assert(index>=0);
	assert(index<MTC);

	if(is_active(index)){
		return (IS_NPC(character[index]));
	}else{ 
		// check bit 0 of the character field
		return(IS_SET(bitflags[index],0x80)!=false);
	}
}
/**************************************************************************/
char_data * C_track_table::get_char(int index)
{
	assert(index>=0);
	assert(index<MTC);

	if(is_active(index)){
		return character[index];
	}else{
		return NULL;
	}
};
/**************************************************************************/
char * C_track_table::get_pers(int index, char_data *looker)
{
	assert(index>=0);
	assert(index<MTC);

	if(is_active(index)){
		return PERS(character[index], looker);
	}else{		
		return "(logged out or dead)";
	}
};
/**************************************************************************/


/**************************************************************************/
void tracktime_update()
{
	tracktime++;
	if(tracktime==65535){
		// this happens once every 4 days or so if PULSE_MINUTE is
		// once every 6 seconds.
		resort_tracks();
		tracktime=1500; // start as if there are 2.5 hours worth of tracks 
	}
}
/**************************************************************************/
void resort_tracks()
{
	bug("resort_tracks() called - not yet implemented");
};
/**************************************************************************/
void init_room_tracks()
{
	ROOM_INDEX_DATA *pRI;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
		for ( pRI= room_index_hash[iHash]; pRI; pRI=pRI->next )
		{			
			pRI->tracks=NULL; // add tracks as they are needed
		}
	}
}
/**************************************************************************/
// tracks for room constructor
C_track_data::C_track_data()
{
	memset(trackindex, 0, sizeof(trackindex));
	memset(time_of_track, 0, sizeof(time_of_track));
	memset(direction_type, 0, sizeof(direction_type));
	nexttrack=0;
}
/**************************************************************************/
tracktype C_track_data::get_tracktype(int index)
{
	return (tracktype)(direction_type[index]>>4);
};
/**************************************************************************/
void C_track_data::set_tracktype(int index, tracktype type)
{
	direction_type[index]= (direction_type[index] & 0x0F) + (type << 4);
};
/**************************************************************************/
int C_track_data::get_direction(int index)
{
	return (direction_type[index]&0x0F);
};
/**************************************************************************/
void C_track_data::set_direction(int index, int direction)
{
	direction_type[index]= (direction_type[index] & 0xF0) + (direction& 0x0F);
};
/**************************************************************************/
void C_track_data::add_track(char_data *ch, int direction, tracktype type)
{	
	assertp(ch->in_room); 
	if(this==NULL){
		// allocate memory for tracks when they are needed, not before
		ch->in_room->tracks=new C_track_data();	
		rooms_with_tracks++;
//		logf("allocating track memory for room %d", ch->in_room->vnum);		
		ch->in_room->tracks->add_track(ch, direction, type);
		return;
	}
	// ensure ch->in_room is the room we (this) belong to
	assert(ch->in_room->tracks==this); 

	// record the details of the track
	time_of_track[nexttrack]=tracktime;
	trackindex[nexttrack]=ch->track_index;

	// record the type and direction
	// type is converted if the general move into a more specific type
	if(type==TRACKTYPE_MOVE){
		if( (INVIS_LEVEL(ch)>=LEVEL_IMMORTAL) 
			|| IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK))
		{
			type=TRACKTYPE_WIZIIMM;
		}else if(IS_AFFECTED(ch, AFF_FLYING)){
			type=TRACKTYPE_FLY;
		}else if(ch->mounted_on){
			type=TRACKTYPE_RIDING;
		}else if(IS_AFFECTED2(ch, AFF2_PASSWOTRACE)){
			if ( IS_OUTSIDE(ch)){
				type=TRACKTYPE_PASSWOTRACE;
			}else{
				type=TRACKTYPE_WALK;
			}
		}else if(IS_AFFECTED(ch, AFF_SNEAK)){
			type=TRACKTYPE_SNEAK;
		}else{
			type=TRACKTYPE_WALK;
		}
	}
	set_direction(nexttrack, direction);
	set_tracktype(nexttrack, type);

	++nexttrack%=MAX_TRACKS_PER_ROOM;
}
/**************************************************************************/
char *tracktype_name(tracktype type)
{
	switch(type){
	default: return "unknown type!!! - report the bug";
	case(TRACKTYPE_NONE			):	return "none";
	case(TRACKTYPE_MOVE			):	return "move";
	case(TRACKTYPE_FLY			):	return "fly";
	case(TRACKTYPE_SNEAK		):	return "sneak";
	case(TRACKTYPE_WALK			):	return "walk";
	case(TRACKTYPE_BLOODTRAIL	):	return "bloodtrail";
	case(TRACKTYPE_BLOODPOOL	):	return "bloodpool";
	case(TRACKTYPE_PASSWOTRACE	):	return "pass without trace";
	case(TRACKTYPE_WIZIIMM		):	return "wiziimm/holywalk";
	}
}
/**************************************************************************/
char *tracktype_age(int time)
{
	// time is between 0 and 600

	int sw; //=URANGE(6, time, 600)/50;

	if(time<2) sw=0;		// 12 seconds IRL	(2mins IC)
	else if(time<5) sw=1;   // 30 seconds IRL	(5mins IC)
	else if(time<10) sw=2;  // 1 minute IRL		(10minsIC)
	else if(time<20) sw=3;  // 2 minutes IRL	 (10		
	else if(time<40)  sw=4; // 4 minutes IRL	 (40mins IC)
	else if(time<70) sw=5;  // 7 minutes IRL	 (1.1hoursIC)
	else if(time<100) sw=6;  // 10 minutes IRL	 (1.6hoursIC)
	else if(time<150) sw=7;  // 15 minutes IRL   (2.5hoursIC)
	else if(time<225) sw=8;  // 22.5 minutes IRL (3,75hours IC)
	else if(time<300) sw=9;	 // 30 minutes IRL (5hours IC)
	else if(time<400) sw=10; // 40 minutes IRL (6.6hours IC)
	else if(time<500) sw=11; // 50 minutes IRL (8.3hours IC)
	else sw=12;

	switch(sw){
		default: return "";
		case(0): return "extremely fresh";
		case(1): return "very fresh";
		case(2): return "fresh";
		case(3): return "rather fresh";
		case(4): return "moderately fresh";
		case(5): return "fairly recent";
		case(6): return "recent";
		case(7): return "moderately recent";
		case(8): return "old";
		case(9): return "very old";
		case(10): return "extremely old";
		case(11): return "faintly visible";					  
		case(12): return "barely visible";
	}
}
/**************************************************************************/
void C_track_data::show_tracks(char_data *ch)
{
	assertp(ch->in_room); 
	int seen=0;
	int sect=ch->in_room->sector_type;

	int sk;
	int main_sn;
	switch(sect){
		case(SECT_INSIDE):
		case(SECT_CITY):
			sk=get_skill(ch, gsn_citytrack) + get_skill(ch, gsn_fieldtrack)/5;
			main_sn=gsn_citytrack;
			break;
		default:
			sk=get_skill(ch, gsn_fieldtrack) + get_skill(ch, gsn_citytrack)/8;
			main_sn=gsn_fieldtrack;
			break;
	}
	sk++; // everyone gets it basically
	if(sk<1){
		ch->println("What would you know about tracking in this terrain?");
		return;
	}
	if(this==NULL){
		if(IS_ICIMMORTAL(ch)){
			ch->println("No tracks in the room yet.");
		}else{
			ch->println("You failed to see any tracks here.");
			WAIT_STATE( ch, skill_table[main_sn].beats );	
		}
		return;
	}
	// ensure ch->in_room is the room we (this) belong to
	assert(ch->in_room->tracks==this); 

	if(!IS_IMMORTAL(ch)){
		if(IS_WATER_SECTOR(sect)){
			ch->println("You can't see tracks in the water.");
			return;
		}
		if(sect==SECT_AIR){
			ch->println("Tracks arent left in the air.");
			return;
		}
	}
	WAIT_STATE( ch, skill_table[main_sn].beats );	
	
	// loop thru displaying the track info in newest to oldest order
	for (int tempindex=nexttrack+MTPR-1; tempindex>=nexttrack; tempindex--)
	{
		int i= tempindex%MTPR;
		if(!time_of_track[i]){
			continue;
		}
		if(IS_ICIMMORTAL(ch)){
			ch->printlnf("%2d> age=%5d, %s tracks of %s to the %s.", 
				i,
				time_of_track[i],
				tracktype_name( (tracktype)(direction_type[i]>>4) ),
				track_table->get_pers(trackindex[i],ch),
				dir_name[(direction_type[i]&0x0F)]);
			seen++;
		}else{
			int timediff;
			if(tracktime<time_of_track[i]){
				// approx hack for now to support looping
				timediff= tracktime+ 65535 - 1500 -time_of_track[i];
			}else{
				timediff=tracktime-time_of_track[i];
			}		
			
			if(timediff< (8*number_range(1,sk*2))) // allow a range of 8->1600
			{
				tracktype type=get_tracktype(i);
				char *race=lowercase(track_table->get_race(trackindex[i]));
				if(!race){
					bugf( __FILE__":C_track_data::show_tracks() - get_race() return NULL\n");
					continue;
				}
				char *racea_an;
				if(*race=='a' || *race=='e' || *race=='i' || *race=='o' || *race=='u'){
					racea_an="an";
				}else{
					racea_an="a";
				}

				int dir=get_direction(i);
				switch(type){
					default: 
						ch->printlnf("unknown track type %d for index %d!!! - please report the bug to code", 
								 (int)type, i);
						break;
					case(TRACKTYPE_NONE			):	break;
					
					case(TRACKTYPE_MOVE			):	
					case(TRACKTYPE_SNEAK		):	
					case(TRACKTYPE_WALK			):
						if(ch->track_index==trackindex[i]){
							ch->printlnf("`SSome %s tracks of %s %s (possibly your own) lead %s%s.`x", 
								tracktype_age(timediff),
								racea_an,
								race,
								(dir==DIR_UP || dir==DIR_DOWN)?"":"to the ",
								dir_name[dir]);
						}else{
							ch->printlnf("Some %s tracks of %s %s lead %s%s.", 
								tracktype_age(timediff),
								racea_an,
								race,
								(dir==DIR_UP || dir==DIR_DOWN)?"":"to the ",
								dir_name[dir]);
						}
						seen++;
						break;

					case(TRACKTYPE_FLY			):	break;
					case(TRACKTYPE_BLOODTRAIL	):	break;
					case(TRACKTYPE_BLOODPOOL	):	break;
					case(TRACKTYPE_WIZIIMM		):	break;
					case(TRACKTYPE_PASSWOTRACE	):	break;
					case(TRACKTYPE_RIDING		):	break;
				}
			}
		}	
	}
	if(seen==0){
		check_improve(ch,main_sn, false, 10);
		ch->println("You failed to see any tracks here.");
	}else{
		check_improve(ch,main_sn, true, 10);
	}

}
/**************************************************************************/
void do_tracks( char_data *ch, char *)
{
	ch->in_room->tracks->show_tracks(ch);
}
/**************************************************************************/
void do_autotrack(char_data *ch, char *)
{
    if (HAS_CONFIG(ch, CONFIG_AUTOTRACK))
    {
		ch->println("Autotrack disabled.");
		REMOVE_CONFIG(ch, CONFIG_AUTOTRACK);
    }
    else
    {
		ch->wraplnf("Autotrack enabled, you will automatically "
			"look for tracks when you move and are not following others (assuming "
			"you have any tracking skill greater than 1%% and you arent speedwalking).");
		SET_CONFIG(ch, CONFIG_AUTOTRACK);
    }
}
/**************************************************************************/
/**************************************************************************/

