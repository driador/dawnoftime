/**************************************************************************/
// mining.cpp - mine command, Kal
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
#define MINE_IRON	(0)
#define MINE_SILVER	(1) 
#define MINE_GOLD	(2)

#define	MAX_MINE_RESULTS (5)

char *mine_name[3]=
{
	"ironore ore",
	"silverore ore",
	"goldore ore nugget"
};

struct mine_results_type 
{
	char *short_descr;
	char *name;
	int weight;
};

struct mine_results_type mine_results[3][MAX_MINE_RESULTS]=
{
	{ // iron
		{"a miniscule piece of iron ore", "miniscule piece iron ore"		, 130},
		{"a small lump of iron ore",		"small lump iron ore"				, 210},
		{"an average-sized lump of iron ore", "average-sized lump iron ore"	, 290},
		{"a large lump of iron ore",		"large lump iron ore"				, 370},
		{"a huge lump of iron ore",		"huge lump iron ore"					, 490}
	},
 
	{ // silver
		{"a miniscule piece of silver ore",	"miniscule piece silver ore"		, 120},
		{"a small lump of silver ore",		"small lump silver ore"				, 180},	
		{"an average-sized lump of silver ore","average-sized lump silver ore"	, 245},
		{"a large lump of silver ore",		"large lump silver ore"				, 300},
		{"a huge lump of silver ore",			"huge lump silver ore"			, 400}
	},
 
	{ // gold
		{"a miniscule gold nugget",		"ore miniscule gold nugget"			, 115},
		{"a small gold nugget",			"ore small gold nugget"				, 215},
		{"an average-sized gold nugget","ore average-sized gold nugget"		, 275},
		{"a large gold nugget",			"ore large gold nugget"				, 390},
		{"a huge gold nugget",			"ore huge gold nugget"				, 540}
	}
};

const int mine_min_max_values[3][2]=
{
	{5,		100}, // iron
	{25,	200}, // silver
	{70,	350}  // gold
};


/**************************************************************************/
// Kal, Sept 02
void do_mine(char_data *ch, char*argument)
{
	if(IS_NULLSTR(argument)){
		ch->println("Syntax: mine ore  - to attempt to mine the room for ore.");
		ch->println("Note: In order to have any success, you must have the mining skill and a pickaxe.");		
		return;
	}

	if( str_prefix(argument, "ore")){
		ch->println("Only ore can be mined.");
		return;
	}

	if(IS_NPC(ch) || !ch->in_room){
		ch->println("players only/must be in a room sorry.");
		return;
	}

	if( IS_AFFECTED(ch,AFF_BLIND)){
		ch->println("How would you propose to do that while being blind?");
		return;
	}

	// check if they have the skill to mine
	int skill=get_skill(ch, gsn_mining);
	if(skill<1){
		ch->println("What would you know about mining ore?");
		return;
	}

	// check they are holding/wielding a pick axe
	obj_data *pickaxe;
	bool fpickaxe=false;
    for ( pickaxe = ch->carrying; pickaxe; pickaxe = pickaxe->next_content )
    {
        if ( !str_cmp( pickaxe->pIndexData->material, "pickaxe" )
			 && ( pickaxe->wear_loc == WEAR_HOLD
			 ||   pickaxe->wear_loc == WEAR_WIELD )) {
			fpickaxe = true;
            break;
		}
    }
	if(!fpickaxe){
		ch->println("You need to be holding a pickaxe in order to mine.");
		return;
	}


	// check they are in a mining room
	// if not in a mining room, they waste a little time and get a little lagged
	if(!IS_SET(ch->in_room->room2_flags, ROOM2_MINE)){
		WAIT_STATE(ch, 3*PULSE_PER_SECOND); // lag them for 3 seconds
		ch->println(3,"You attempt to mine with no success, perhaps you should try mining in an actual mine?!?");
		return;
	}

	// we are in a mine, lets see what they can find

	// apply the lag here, so we don't have to do it for every result
	int lag=10*PULSE_PER_SECOND;// a default amount of lag, if one isn't specified
	if(skill_table[gsn_mining].beats){ 
		lag=skill_table[gsn_mining].beats; 
	}
	if(IS_IMMORTAL(ch)){// imms get 1 second lag regardless
		lag=PULSE_PER_SECOND; 
	}
	WAIT_STATE(ch, lag); 
	lag/=PULSE_PER_SECOND; // get lag into seconds

	act("You start to mine with $p...",ch,pickaxe, NULL,TO_CHAR);
	act("$n starts to mine with $p.",ch,pickaxe, NULL,TO_ROOM);

	// if mine has been used in the last 15 minutes, they get nothing
	if(ch->in_room->last_mined_in_room+600>current_time){
		ch->println(lag,"You attempt to mine for a while with no success, this location appears to have already been mined recently.");
		return;
	}

	// decide on the type of ore they get, before skill mods on these values
	//nothing	11%	11
	//iron		44%	55
	//silver	33%	88
	//gold		12%	100
	// skill adds up to 10% in the value

	int type=number_range(-10,90) + URANGE(0, skill/10, 10); // random between 1 and 100 

	if(type<11){ // nothing - they failed, don't mark the room as mined
		ch->println(lag,"You mine for quite some time, but fail to find any form of ore.");
		return;
	}

	// mark the room as mined, since they are guaranteed success beyond here

	if(type<55){ // iron
		type=MINE_IRON;	
	}else if(type<88){ // silver
		type=MINE_SILVER;
	}else{ // gold
		type=MINE_GOLD;
	}

	// now figure out the value of the ore
	// skill and time since last mining increase the lower bound
	
	int lower_bound=5 // up to the last 100 minutes, adds up to 30% below
		+ URANGE(0, (((int)(current_time-ch->in_room->last_mined_in_room))/200), 30)
		+ URANGE(0, skill/5, 20); // skill adds up to 20%
	int value=number_range(1, number_range(lower_bound, 100));
	// value is now number between 1 and 100

	// scale this to a value ((max-min)*value/100)+min
	int objvalue=
		(	(mine_min_max_values[type][1]-mine_min_max_values[type][0]
			) // (max-min)
			* value/100
		) 
		+mine_min_max_values[type][0]; // +min

	// globally scale the results by the game defined setting
	objvalue*=game_settings->global_scale_mining_value_scaling_percentage/100;

	int index=URANGE(0,(value/(100/MAX_MINE_RESULTS)),MAX_MINE_RESULTS-1);

	// create the object
	obj_index_data *ore_template=get_obj_index(OBJ_VNUM_ORE);
	if(!ore_template){
		ch->wraplnf("Unfortunately the ore object template is missing "
			"from the realm (object vnum %d), "
			"so you can't be awarded any ore "
			"- please report this bug to the admin.",
			OBJ_VNUM_ORE);
		return;
	}
	obj_data *ore=create_object(ore_template);

	// string the object	
	replace_string(ore->name, mine_results[type][index].name);
	replace_string(ore->short_descr, mine_results[type][index].short_descr);
	replace_string(ore->description, capitalize(FORMATF("%s is here.", ore->short_descr)));
	ore->cost=objvalue*2;

	// make it weigh something
	ore->weight=mine_results[type][index].weight;

	SET_BIT(ore->wear_flags, OBJWEAR_TAKE|OBJWEAR_HOLD);

	ch->printlnf(lag, "You have unearthed %s.", ore->short_descr);
	act("$n has unearthed $p.", ch, ore, NULL, TO_ROOM);

	// record the last time the room was mined
	ch->in_room->last_mined_in_room=current_time;

    if( ch->carry_number + get_obj_number( ore ) > can_carry_n( ch )){
		ch->println(lag, "You are carrying too many items to pick up your unearthed ore.");
		obj_to_room(ore, ch->in_room);
		return;
    }

    if ( !IS_SWITCHED (ch) && get_carry_weight(ch) + get_obj_weight( ore ) > can_carry_w( ch ) )
    {
		ch->println(lag, "You are already carrying too much weight to carry this ore.");
		obj_to_room(ore, ch->in_room);		
		return;
    }

	ch->printlnf(lag, "You pick up %s.", ore->short_descr);
	act("$n picks up $p.", ch, ore, NULL, TO_ROOM);
	obj_to_char(ore, ch);
}

/**************************************************************************/
/**************************************************************************/

