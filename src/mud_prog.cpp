/**************************************************************************/
// mob_prog.cpp - Mudprogs engine, most enhancements by Kal
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
/***************************************************************************
 *  MUDprograms for ROM 2.4 v0.98g (C) M.Nylander 1996                     *
 *  Based on MERC 2.2 MUDprograms concept by N'Atas-ha.                    *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 ***************************************************************************/
#include "include.h" // dawn standard includes
#include "security.h"
#include "events.h"

extern int flag_lookup( const char *word, const struct flag_type *flag_table );

enum MUDPROG_TRIGGERED_ON{
	MUDPROG_TRIGGERED_ON_MOBILE,
	MUDPROG_TRIGGERED_ON_OBJECT,
	MUDPROG_TRIGGERED_ON_ROOM,
};

/*
 * These defines correspond to the entries in fn_keyword[] table.
 * If you add a new if_check, you must also add a #define here.
 */
#define CHK_RAND		(0)
#define CHK_MOBHERE     (1)
#define CHK_OBJHERE     (2)
#define CHK_MOBEXISTS   (3)
#define CHK_OBJEXISTS   (4)
#define CHK_PEOPLE      (5)
#define CHK_PLAYERS     (6)
#define CHK_MOBS        (7)
#define CHK_CLONES      (8)
#define CHK_ORDER       (9)
#define CHK_HOUR        (10)
#define CHK_ISPC        (11)
#define CHK_ISNPC       (12)
#define CHK_ISGOOD      (13)
#define CHK_ISEVIL      (14)
#define CHK_ISNEUTRAL   (15)
#define CHK_ISIMMORT    (16)
#define CHK_ISCHARM     (17)
#define CHK_ISFOLLOW    (18)
#define CHK_ISACTIVE    (19)
#define CHK_ISDELAY     (20)
#define CHK_ISVISIBLE   (21)
#define CHK_HASTARGET   (22)
#define CHK_ISTARGET    (23)
#define CHK_EXISTS      (24)
#define CHK_AFFECTED    (25)
#define CHK_ACT         (26)
#define CHK_OFF         (27)
#define CHK_IMM         (28)
#define CHK_CARRIES     (29)
#define CHK_WEARS       (30)
#define CHK_HAS         (31)
#define CHK_USES        (32)
#define CHK_NAME        (33)
#define CHK_POS         (34)
#define CHK_CLAN        (35)
#define CHK_RACE        (36)
#define CHK_CLASS       (37)
#define CHK_OBJTYPE     (38)
#define CHK_VNUM        (39)
#define CHK_HPCNT       (40)
#define CHK_ROOM        (41)
#define CHK_SEX         (42)
#define CHK_LEVEL       (43)
#define CHK_ALIGN       (44)
#define CHK_MONEY       (45)
#define CHK_OBJVAL0     (46)
#define CHK_OBJVAL1     (47)
#define CHK_OBJVAL2     (48)
#define CHK_OBJVAL3     (49)
#define CHK_OBJVAL4     (50)
#define CHK_GRPSIZE     (51)
#define CHK_EXACT_NAME  (52)
#define CHK_AREA		(53)
#define CHK_SECTOR		(54)
#define CHK_ROOMFLAG	(55)
#define CHK_ISQUESTER	(56)
#define CHK_HASPKTIMER	(57)
#define CHK_HASTOKEN	(58)
#define CHK_PKKILLS		(59)
#define CHK_PKDEFEATS	(60)
#define CHK_KARNS		(61)
#define CHK_RPS			(62)
#define CHK_DRUNK		(63)
#define CHK_FULL		(64)
#define CHK_THIRST		(65)
#define CHK_HUNGER		(66)
#define CHK_TIRED		(67)
#define CHK_ISCOURT		(68)
#define CHK_ACT2		(69)
#define CHK_VALUE		(70)
#define CHK_CHARHERE    (71)
#define CHK_HASSKILL	(72)
#define CHK_HASTRAINS	(73)
#define CHK_HASPRACS	(74)
#define CHK_HASBONUS	(75)
#define CHK_RACESIZE	(76)
#define CHK_ISEXITOPEN	(77)
#define CHK_ISTHIEF		(78)
#define CHK_ISKILLER	(79)
#define CHK_ISSHEATHED	(80)
#define CHK_ISCONCEALED	(81)
#define CHK_HASEVENTGROUPQUEUED (82)
	
 /*
  * These defines correspond to the entries in fn_evals[] table.
  */
#define EVAL_EQ            0
#define EVAL_GE            1
#define EVAL_LE            2
#define EVAL_GT            3
#define EVAL_LT            4
#define EVAL_NE            5

/**************************************************************************/
/*
 * if-check keywords:
 */
struct	fn_keyword_type
{
    const char * name;
	const char * descript;
	const char * syntax;
	const char * notes;
	const char * example;
};

const	struct	fn_keyword_type		fn_keyword[] =
{
    {"rand",		"random number check", "if rand <number>", 
		"Will be true if a random number between 1 and 100 is less than <number>.",
		"if rand 30"},
    {"mobhere",		"check if a mob is in the room by vnum", "if mobhere <mobvnum>",
					"While technically it could be used like 'if mobhere dog' if charhere is better for that, since charhere supports 'if charhere $q' etc.",
					"if mobhere 3001"},		
    {"objhere",		"check if an object is in the room by vnum/keyword", "if objhere <keyword|vnum>",
					"", 
					"if objhere 3001`1if objhere bottle"},
    {"mobexists", "check if a mob exists somewhere in the game", "if mobexists <mobname>"},
    {"objexists", "check if a object exists somewhere in the game", "if objexists <keyword>"},

    {"people", "use to check for a count of people in the room (mobs + players)", "if people <operation> <number>",
				"'if people > 4' is true when does room contain more than 4 people"},
    {"players", "same as people check, only for players"},
    {"mobs", "same as people check, only for mobs"},
    {"clones", "if clones > 3	- are there > 3 mobs of same vnum here"},
    {"order", "if order == 0	- is mob the first in room"},
    {"hour", "if hour > 11		- is the time > 11 o'clock"},

    {"ispc",		"if ispc $n 		- is $n a pc "},
    {"isnpc",		"if isnpc $n 		- is $n a mobile "},
    {"isgood",		"if isgood $n 	- is $n good "},
    {"isevil",		"if isevil $n 	- is $n evil "},
    {"isneutral",	"if isneutral $n 	- is $n neutral "},
    {"isimmort",	"if isimmort $n	- is $n immortal "},
    {"ischarm",		"if ischarm $n	- is $n charmed "},
    {"isfollow",	"if isfollow $n	- is $n following someone "},
    {"isactive",	"if isactive $n	- is $n's position > SLEEPING ", 
			"useful in mudprogs to wake a mob up if they are asleep`1"
			"              (on triggers that work on asleep mobs)",
			"The following code will wakeup a mob, put this at the top of a prog`1"
			"              (isactive can obviously used on players also)",
			"`1"
			"              if !isactive $i`1"
			"                 mob echoaround $i $I yawns and stretches.`1"
			"                 mq2 wake`1"
			"                 mq5 '>$n Hello there.`1"
			"              else`1"
			"                 '>$n Hello there.`1"
			"              endif"},
    {"isdelay",		"if isdelay $i   - does $i have mudprog pending "},
    {"isvisible",	"if isvisible $n - can mob see $n "},
    {"hastarget",	"if hastarget $i - does $i have a valid target "},
    {"istarget",	"if istarget $n  - is $n mob's target "},
    {"exists",		"if exists $n    - does $n exist somewhere "},

    {"affected",	"if affected $n blind - is $n affected by blind "},
    {"act",			"if act $i sentinel	- is $i flagged sentinel "},
    {"off",			"if off $i berserk	- is $i flagged berserk "},
    {"imm",			"if imm $i fire	- is $i immune to fire "},
    {"carries",		"if carries $n sword	- does $n have a 'sword'`1"
					"if carries $n 1233	- does $n have obj vnum 1233"},
    {"wears",		"if wears $n lantern	- is $n wearing a 'lantern'`1"
					"if wears $n 1233	- is $n wearing obj vnum 1233"},
    {"has",		"if has $n weapon	- does $n have obj of type weapon "},
    {"uses",	"if uses $n armor	- is $n wearing obj of type armor"},
    {"name",	"if name $n puff	- does $n's name contain 'puff', 'if name $o iron' - is the word iron in the objects name."},
    {"pos",		"if pos $n standing	- is $n standing"},
    {"clan",	"if clan $n 'whatever'- does $n belong to clan 'whatever'"},
    {"race",	"if race $n elf	- is $n of 'elf' race"},
    {"class",	"if class $n mage	- is $n's class 'mage'"},
    {"objtype", "if objtype $p scroll	- is $p a scroll"},

    {"vnum",	"if vnum $i == 1233 - virtual number check"},
    {"hpcnt",	"if hpcnt $i > 30	- hit point percent check"},
    {"room",	"if room $i == 1233	- room virtual number"},
    {"sex",		"if sex $i == 0	  - sex check, 0=it, 1=male,2=female"},
    {"level",	"'if level $n < 5' or 'if level $o < 5'	etc - character/object level check"},
    {"align",	"if align $n < -1000	- alignment check"},
    {"money",	"if money $n"},
    {"objval0", "if objval0 $o > 1000 	- object value[] checks 0..4"},
    {"objval1", ""},
    {"objval2", ""},
    {"objval3", ""},
    {"objval4", ""},
    {"grpsize", "if grpsize $n > 6	- group size check"},
    {"exactname", "if exactname $n puff	- is $n's exactname 'puff'"},
	{"area",	"if area $n 'full exact name of area' - check if "
								"$n is in the area which name matches."},
	{"sector",		"if sector $n sectortype  - strprefix checking not exact"},
	{"roomflag",	"if room $n roomflag"},
	{"isquester",	"if quester $n"},
	{"haspktimer",	"if haspktimer $n"},
    {"hastoken",	"if hastoken $n 3087 - is $n carrying 1 or more of token 3087`1"
					"if hastoken $n 3.1233 - is $n carrying 3 or more of token 1233`1"
					"if hastoken $n tokendescr - does $n have a 'tokendescr'"},

	{"pkkills",		"if pkkills $n > 1000 - ch pkkill checks"},
	{"pkdefeats",	"if pkdefeats $n > 1000 - ch pkdefeat checks"},
	{"karns",		"if karns $n > 1000 - ch karn checks"},
	{"rps",			"if rps $n > 1000 - ch rps checks"},

	{"drunk",	"if drunk $n > 10"},
	{"full",	"if full $n > 10"},
	{"thirst",	"if thirst $n > 10"}, 	
	{"hunger",	"if hunger $n > 10"},
	{"tired",	"if tired $n > 10"},	
	{"iscourt", "errr, if is in the court thingie"},
	{"act2",	"act2 flags"},
	{"value", "use this to check room numbers in premove triggers (see mobhelp preventmove)"},
	{"charhere", "if charhere $n"},
	{"hasskill", "if hasskill $n parry > 10"},
	{"hastrains","if hastrains $n > 1 - necessary for mob setskill to ensure enuff trains exist"},
	{"haspracs", "if haspracs $n > 1 - necessary for mob setskill to ensure enuff pracs exist"},
	{"hasbonus", "if hasbonus $n strength > 10 - Strength bonus of $n > 10"},
	{"racesize", "if racesize $n > 1 - checks racial size -- sizes are:`1"
				 "   tiny=0 small=1 medium=2 large=3 huge=4 giant=5"},
	{"isexitopen", "if isexitopen <direction> - returns true if <direction> is an open exit (open door, no door etc)`1"
				 "   <direction> can be a word for the direction or the direction number."},
	{"isthief",	"if isthief $n - true if player is currently flagged as a thief"},
	{"iskiller","if iskiller $n - true if player is currently flagged as a killer"},
	{"issheathed", "if issheathed $n - has weapon sheathed"}, // Ix
	{"isconcealed", "if isconcealed $n - has weapon concealed"}, // Ix
	{"haseventgroupqueued", "if haseventgroupqueued $i 55 - checks if mob has any events queued in event group 55, checking group 0 means any mudprog queued event.`1",
			"if haseventgroupqueued <target> <eventgroupnumber>",
			"",
			"`1    if !haseventgroupqueued $i 55`1"
			"        * cause the mob to smile only if it doesn't already have a smile queued, using eventgroup 55`1"
			"        mqg55,10 smile`1"
			"    endif"}, // Kal, Jan07

    {"", ""},	// Table terminator 
};

/**************************************************************************/
const char *fn_evals[] =
{
    "==", ">=", "<=", ">", "<", "!=", "\n"
};
/**************************************************************************/
// a command to display all the mudprog if checks
void do_ifhelp( char_data *ch, char *argument )
{
    int cmd;
	// do the checks to make sure they can use the command here
	if (!HAS_SECURITY(ch,1)){
		ch->println("The ifhelp command is an olc mudprog programming related command, you don't have olc permissions.");
		return;
	}

    if ( !HAS_SECURITY(ch, MPEDIT_MINSECURITY))
    {
    	ch->printlnf("ifhelp: Insufficient olc security to use this command %d required.", 
			MPEDIT_MINSECURITY);
    	return;
    }
	
	// Return a valid keyword from a keyword table
	if(IS_NULLSTR(argument)){
		// command list syntax
		for ( cmd = 0; !IS_NULLSTR(fn_keyword[cmd].name); cmd++ )
		{
			ch->printlnf("%s%-11s `S- `B%s",
				(IS_NULLSTR(fn_keyword[cmd].notes)?"`W":"`Y"),
				capitalize(fn_keyword[cmd].name),
				IS_NULLSTR(fn_keyword[cmd].descript)?"`Sno descripton":fn_keyword[cmd].descript);
		}
		ch->println("`xTo see more info on a given command, type `=Cifhelp <command>`x");
		ch->println("`xNote: you can put an ! before any keyword to negate its operation (e.g. if !class $n mage)");
		ch->println("`YYellow commands have example code/notes`x");

	}else{
		// specific list syntax
		for ( cmd = 0; !IS_NULLSTR(fn_keyword[cmd].name); cmd++ )
		{
			// filter in only required commands
			if(str_infix(argument,fn_keyword[cmd].name))
				continue;
			ch->printlnf("`W%-11s `S- `B%s\r\n"
				"      `Ssyntax: `Y%s\r\n"
				"      `Snotes:  `g%s\r\n"
				"      `Sexample:`x%s\r\n", 
				capitalize(fn_keyword[cmd].name),
				IS_NULLSTR(fn_keyword[cmd].descript)?"`Sno descripton":fn_keyword[cmd].descript,
				IS_NULLSTR(fn_keyword[cmd].syntax)?"`Sno syntax":fn_keyword[cmd].syntax,
				IS_NULLSTR(fn_keyword[cmd].notes)?"`Sno notes":fn_keyword[cmd].notes,
				IS_NULLSTR(fn_keyword[cmd].example)?"`Sno example":fn_keyword[cmd].example);
		}
	}
}

/**************************************************************************/
// Return a valid keyword from a keyword table
int keyword_lookup( const char **table, char *keyword )
{
    register int i;
    for( i = 0; table[i][0] != '\n'; i++ )
        if( !str_cmp( table[i], keyword ))
            return( i );
	return -1;
}

/**************************************************************************/
// Return a valid keyword from a keyword table
int fn_keyword_lookup( char *keyword )
{
    register int i;
    for( i = 0; !IS_NULLSTR(fn_keyword[i].name); i++ ){
        if( !str_cmp( fn_keyword[i].name, keyword )){
            return( i );
		}
	}
	return -1;
}
/**************************************************************************/
// Perform numeric evaluation.
// Called by cmd_eval()
int num_eval( int lval, int oper, int rval )
{
    switch( oper )
    {
	case EVAL_EQ:
		return ( lval == rval );
	case EVAL_GE:
		return ( lval >= rval );
	case EVAL_LE:
		return ( lval <= rval );
	case EVAL_NE:
		return ( lval != rval );
	case EVAL_GT:
		return ( lval > rval );
	case EVAL_LT:
		return ( lval < rval );
	default:
		bug("num_eval: invalid oper");
		return 0;
    }
}

/**************************************************************************/
/*
 * ---------------------------------------------------------------------
 * UTILITY FUNCTIONS USED BY CMD_EVAL()
 * ----------------------------------------------------------------------
 */

/*
 * Get a random PC in the room (for $r parameter)
 */
char_data *get_random_char( char_data *mob )
{
    char_data *vch, *victim = NULL;
    int now = 0, highest = 0;
    for( vch = mob->in_room->people; vch; vch = vch->next_in_room )
    {
        if ( mob != vch 
			&&   !IS_NPC( vch ) 
			&&   can_see( mob, vch )
			&&   ( now = number_percent() ) > highest )
        {
            victim = vch;
            highest = now;
        }
    }
    return victim;
}
/**************************************************************************/
/* 
 * How many other players / mobs are there in the room
 * iFlag: 0: all, 1: players, 2: mobiles 3: mobs w/ same vnum 4: same group
 */
int count_people_room( char_data *mob, int iFlag )
{
    char_data *vch;
    int count;
    for ( count = 0, vch = mob->in_room->people; vch; vch = vch->next_in_room )
		if ( mob != vch 
			&&   (iFlag == 0
			|| (iFlag == 1 && !IS_NPC( vch )) 
			|| (iFlag == 2 && IS_NPC( vch ))
			|| (iFlag == 3 && IS_NPC( mob ) && mob->vnum() == vch->vnum() )
			|| (iFlag == 4 && is_same_group( mob, vch )) )
			&& can_see( mob, vch ) ) 
			count++;
		return ( count );
}

/**************************************************************************/
/*
 * Get the order of a mob in the room. Useful when several mobs in
 * a room have the same trigger and you want only the first of them
 * to act 
 */
int get_order( char_data *ch )
{
    char_data *vch;
    int i;
	
    if ( !IS_NPC(ch) )
		return 0;
    for ( i = 0, vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
		if ( vch == ch )
			return i;
		if ( IS_NPC(vch) 
			&&   vch->vnum() == ch->vnum() )
			i++;
    }
    return 0;
}

/**************************************************************************/
/*
 * Check if ch has a given item or item type
 * vnum: item vnum or -1
 * item_type: item type or -1
 * fWear: true: item must be worn, false: don't care
 */
bool has_item( char_data *ch, vn_int vnum, int item_type, bool fWear )
{
    OBJ_DATA *obj;
    for ( obj = ch->carrying; obj; obj = obj->next_content ){	
		if ( ( vnum < 0 || obj->pIndexData->vnum == vnum )
			&&   ( item_type < 0 || obj->pIndexData->item_type == item_type )
			&&   ( !fWear || obj->wear_loc != WEAR_NONE ) )
		{
			// can't find the token with this sytsem
			if(obj->item_type == ITEM_TOKEN){
				mpbugf( "trying to match a token using a syntax other than 'if hastoken'.");
				return false;
			}
			return true;
		}
	}
	return false;
}

/**************************************************************************/
// Check if ch has a given token - Kal August 99
//  
// Supports the hastoken trigger... can do things like
// if hastoken $* 3.3005 to see if the player has 3 or more of token 3005
// or also supports using 3.textnameoftoken
bool has_token( char_data *ch, char *argument)
{
	char arg[MIL];
	OBJ_DATA *obj;
	int number;
	int count;
	int tokenvnum;

	number = number_argument( argument, arg );

	count  = 0;

	tokenvnum=0;
	if(is_number(arg)){ 
		tokenvnum=atoi(arg);
	}

	if(tokenvnum){ // token is being referenced by number
		for ( obj = ch->carrying; obj; obj = obj->next_content ) {
			if ( obj->item_type == ITEM_TOKEN
			&&	obj->pIndexData 
			&& 	obj->pIndexData->vnum==tokenvnum) {
				if ( ++count == number )
					return true;
			}
		}
	}else{ // token is being referenced by textname
		for ( obj = ch->carrying; obj; obj = obj->next_content ) {
			if ( obj->item_type == ITEM_TOKEN
			&&	 is_name( arg, obj->name )) {
				if ( ++count == number )
					return true;
			}
		}
	}
	return false;
}
/**************************************************************************/
// Check if ch has a given event group queued - Kal Jan 2007
//  
bool has_eventgroupqueued( char_data *ch, char *argument)
{
	char first_arg[MIL];
	sh_int group;

	one_argument(argument, first_arg);

	if(IS_NULLSTR(first_arg) || !is_number(first_arg)){
		mpbugf("has_eventgroupqueued(): incorrect syntax, must specify a number between 0 and %d for the eventgroup - '%s' is not.", 
			EVENTGROUP_SYSTEM-1,
			first_arg);
		return false;
	}
	group=(sh_int)atoi(first_arg);

	if(group<0 || group>=EVENTGROUP_SYSTEM){
		mpbugf("has_eventgroupqueued(): incorrect syntax, must specify a number between 0 and %d for the eventgroup - '%s' is not.", 
			EVENTGROUP_SYSTEM-1,
			first_arg);
		return false;
	}

	event_data *e=ch->running_mudprog_for_object?ch->running_mudprog_for_object->events:ch->events;

	while(e){
		if(e->group==group || (group==0 && e->group<EVENTGROUP_SYSTEM)){
			return true;
		}
		e=e->get_next_for_entity();
	}
	return false;
}

/**************************************************************************/
/*
 * Check if there's a mob with given vnum in the room
 */
bool get_mob_vnum_room( char_data *ch, int vnum )
{
    char_data *mob;
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
		if ( IS_NPC( mob ) && mob->vnum() == vnum )
			return true;
	return false;
}

/**************************************************************************/
/*
 * Check if there's an object with given vnum in the room
 */
bool get_obj_vnum_room( char_data *ch, int vnum )
{
    OBJ_DATA *obj;
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
		if ( obj->pIndexData->vnum == vnum )
			return true;
	return false;
}

/**************************************************************************/
/* ---------------------------------------------------------------------
 * CMD_EVAL
 * This monster evaluates an if/or/and statement
 * There are five kinds of statement:
 * 1) keyword  and value (no $-code)			if random 30
 * 2) keyword, comparison and value				if people > 2
 * 3) keyword  and actor						if isnpc $n
 * 4) keyword, actor and value					if carries $n sword
 * 5) keyword, actor, comparison and value		if level $n >= 10
 *----------------------------------------------------------------------
 */
bool cmd_eval( int vnum, char *line, int check,
	char_data *mob, char_data *ch, 
	const void *arg1, const void *arg2, char_data *rch )
{
	char_data *lval_char = mob;
	char_data *vch  = (char_data *) arg2;
	OBJ_DATA  *obj1 = (OBJ_DATA  *) arg1;
	OBJ_DATA  *obj2 = (OBJ_DATA  *) arg2;
	OBJ_DATA  *lval_obj = NULL;

	char *original, buf[MIL], code='\0';
	int lval = 0, oper = 0, rval = -1;
	
	original = line;
	line = one_argument( line, buf );
	if ( buf[0] == '\0' || mob == NULL )
		return false;
	
	/*
	 * If this mobile has no target, let's assume our victim is the one
	 */
	if ( mob->mprog_target == NULL )
		mob->mprog_target = ch;

	// had to put this up here since the hasskill check doesn't comply with
	// any of the 5 cmd_eval types, and making a sixth one would mean a
	// rewrite of #5, and by that time, all the arguments and such have been
	// really destroyed beyond recognition, needing the use of original, and
	// since this is a unique situation, a little mini-hack was the simplest
	// solution, and we all know very well....  K.I.S.S. :)
	//
	// keyword, actor, >keyword2<, comp, value -- if hasskill $n parry > 10
	// <MINIHACK>
	if ( check == CHK_HASSKILL )
	{
		char actor[MIL];
		char buf2[MIL];

		strcpy( actor, buf );

		if ( IS_NULLSTR( line ))
		{
			mpbugf("Cmd_eval(): prog %d  if hasskill (null)", vnum );
			return false;
		}
		
		if ( actor[0] != '$' || buf[1] == '\0' )
		{
			mpbugf( "Cmd_eval: prog %d syntax error in actor '%s'", vnum, original );
			return false;
		}
		else
			code = buf[1];
		switch( code )
		{
		case 'i':	lval_char = mob;										 break;
		case 'n':	lval_char = ch;											 break;
		case 't':	lval_char = vch;										 break;
		case 'r':	lval_char = rch == NULL ? get_random_char( mob ) : rch ; break;
		case 'q':	lval_char = mob->mprog_target;							 break;
		default:
			mpbugf( "Cmd_eval: prog %d syntax error in actor '%s'", vnum, original );
			return false;
		}

		line = one_argument( line, buf );		// buf has skillname
		line = one_argument( line, buf2 );		// buf2 has operator
		
		if (( oper = keyword_lookup( fn_evals, buf2 )) < 0 )
		{
			mpbugf( "Cmd_eval: prog %d incorrect operator value '%s'", vnum, original );
			return false;
		}

		rval = atoi( line );

		if ( lval_char != NULL && !IS_NPC( lval_char )) {
			lval = get_skill( lval_char, skill_lookup(buf));
			logf( "lval %d rval %d", lval,rval );
		}
		else
		{
			mpbugf("Cmd_eval(): prog %d is buggy '%s'", vnum, original );
			return false;
		}

		if(num_eval( lval, oper, rval )){
			return true;
		}
		return false;
	}
	if ( check == CHK_HASBONUS )
	{
		char actor[MIL];
		char buf2[MIL];

		strcpy( actor, buf );

		if ( IS_NULLSTR( line ))
		{
			mpbugf("Cmd_eval(): prog %d  if hasbonus (null)", vnum );
			return false;
		}
		
		if ( actor[0] != '$' || buf[1] == '\0' )
		{
			mpbugf( "Cmd_eval: prog %d syntax error in actor '%s'", vnum, original );
			return false;
		}
		else
			code = buf[1];
		switch( code )
		{
		case 'i':	lval_char = mob;										 break;
		case 'n':	lval_char = ch;											 break;
		case 't':	lval_char = vch;										 break;
		case 'r':	lval_char = rch == NULL ? get_random_char( mob ) : rch ; break;
		case 'q':	lval_char = mob->mprog_target;							 break;
		default:
			mpbugf( "Cmd_eval: prog %d syntax error in actor '%s'", vnum, original );
			return false;
		}

		line = one_argument( line, buf );		// buf has bonus
		line = one_argument( line, buf2 );		// buf2 has operator
		
		if (( oper = keyword_lookup( fn_evals, buf2 )) < 0 )
		{
			mpbugf( "Cmd_eval: prog %d incorrect operator value '%s'", vnum, original );
			return false;
		}

		rval = atoi( line );

		if ( lval_char != NULL && !IS_NPC( lval_char )) {
//			lval = get_skill( lval_char, skill_lookup(buf));
			lval = lval_char->modifiers[stat_lookup(buf)];

			logf( "lval %d rval %d", lval,rval );
		}
		else
		{
			mpbugf("Cmd_eval(): prog %d is buggy '%s'", vnum, original );
			return false;
		}

		if( num_eval( lval, oper, rval )){
			return true;
		}
		return false;
	}
	// </MINIHACK>         ps - ker->geekfactor += 1; for using html-like comments :)
	switch ( check )
	{
	/*
	 * Case 1: keyword and value
	 */
	case CHK_RAND:
		{
			int numval;
			
			if(is_number(buf))
			{
				numval=atoi( buf );
			}
			else
			{
				mpbugf( "Cmd_eval: prog %d syntax with rand command - '%s ' is not a number!",
					vnum, buf);
				return false;
			}
			
			int numpercent=number_percent();
//			logf("prog=%d, numval=%d, numpercent=%d, returning %s", 
//				vnum, numval, numpercent,	numval>numpercent?"true":"false");	
			return( numval>numpercent);
		}
	case CHK_MOBHERE:
		if ( is_number( buf ))
			return( get_mob_vnum_room( mob, (vn_int)atoi(buf) ) );
		else{
			if(buf[0] == '$'){
				mpbugf( "Cmd_eval - mobhere if check: Can't process variable arguments (parameters with $ in them, use 'if charhere $n' etc.");
				return false;
			}
			return( (bool) (get_char_room( mob, buf) != NULL) );
		}
	case CHK_OBJHERE:
		if ( is_number( buf ) ){
			return( get_obj_vnum_room(mob, atoi(buf)) );
		}else{
			return( (bool) (get_obj_here( mob, buf) != NULL) );
		}
	case CHK_MOBEXISTS:
		return( (bool) (get_char_world( mob, buf) != NULL) );
	case CHK_OBJEXISTS:
		return( (bool) (get_obj_world( mob, buf) != NULL) );

	case CHK_ISEXITOPEN:
		{
			int processed_dir;
			if(IS_NULLSTR(buf)){
				mpbugf("Cmd_eval(): prog %d  if isexitopen (null)", vnum );
				return false;
			}
			// calculate the direction
			if(is_number(buf)){
				processed_dir=atoi( buf);
			}else{
				processed_dir=dir_lookup(buf);					
			}
			if(processed_dir<0 || processed_dir>=MAX_DIR){
				mpbugf("cmd_eval(): Unrecognised direction '%s' in is isexitopen check - program %d.",
					buf, vnum);
				return false;
			}
			if(mob->in_room 
				&& mob->in_room->exit[processed_dir] 
				&& mob->in_room->exit[processed_dir]->u1.to_room
				&& can_see_room(mob, mob->in_room->exit[processed_dir]->u1.to_room) 
				&& !IS_SET(mob->in_room->exit[processed_dir]->exit_info, EX_CLOSED)){
				return true;		
			}
			return false;
		}
		break;

	case CHK_ISTHIEF:
		return( lval_char != NULL && !IS_NPC(lval_char) && IS_THIEF(lval_char) );

	case CHK_ISKILLER:
		return( lval_char != NULL && !IS_NPC(lval_char) && IS_KILLER(lval_char) );
			

	/*
	 * Case 2 begins here: We sneakily use rval to indicate need
	 * 		       for numeric eval...
	 */
	case CHK_PEOPLE:
		rval = count_people_room( mob, 0 ); break;
	case CHK_PLAYERS:
		rval = count_people_room( mob, 1 ); break;
	case CHK_MOBS:
		rval = count_people_room( mob, 2 ); break;
	case CHK_CLONES:
		rval = count_people_room( mob, 3 ); break;
	case CHK_ORDER:
		rval = get_order( mob ); break;
	case CHK_HOUR:
		rval = time_info.hour; break;
	case CHK_VALUE:
		{
			if(is_number(buf)){
				rval = atoi(buf); 
			}else{
				mpbugf( "Cmd_eval - 'value' if check: non numeric input on line '%s'",
					original );
				return false;
			}
		    line = one_argument( line, buf );
		}
		break;
	default:;
    }
	
    /*
	 * Case 2 continued: evaluate expression
	 */
	if ( rval >= 0 )
	{
		if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
		{
			mpbugf( "Cmd_eval: prog %d syntax error(2) '%s'",
				vnum, original );
			return false;
		}
		one_argument( line, buf );
		lval = rval;
		rval = atoi( buf );
		if( num_eval( lval, oper, rval )){
			return true;
		}
		return false;

	}
	
	/*
	 * Case 3,4,5: Grab actors from $* codes
	 */
	if ( buf[0] != '$' || buf[1] == '\0' )
    {
		mpbugf( "Cmd_eval: prog %d syntax error(3) '%s'",
			vnum, original );
        return false;
    }
    else
        code = buf[1];
    switch( code )
    {
	case 'i':
		lval_char = mob; break;
	case 'n':
		lval_char = ch; break;
	case 't':
		lval_char = vch; break;
	case 'r':
		lval_char = rch == NULL ? get_random_char( mob ) : rch ; break;
	case 'o':
		lval_obj = obj1; break;
	case 'p':
		lval_obj = obj2; break;
	case 'q':
		lval_char = mob->mprog_target; break;
	default:
		mpbugf( "Cmd_eval: prog %d syntax error(4) '%s'",
			vnum, original );
		return false;
	}
    /*
	 * From now on, we need an actor, so if none was found, bail out
	 */
    if ( lval_char == NULL && lval_obj == NULL )
		return false;
	
	/*
	 * Case 3: Keyword, comparison and value
	 */
    switch( check )
    {
	case CHK_ISPC:
		return( lval_char != NULL && !IS_NPC( lval_char ) );
	case CHK_ISNPC:
		return( lval_char != NULL && IS_NPC( lval_char ) );
	case CHK_ISGOOD:
		return( lval_char != NULL && IS_GOOD( lval_char ) );
	case CHK_ISEVIL:
		return( lval_char != NULL && IS_EVIL( lval_char ) );
	case CHK_ISNEUTRAL:
		return( lval_char != NULL && IS_NEUTRAL( lval_char ) );
	case CHK_ISIMMORT:
		return( lval_char != NULL && IS_IMMORTAL( lval_char ) );
		
	case CHK_ISCOURT:
		// using IS_SET to check for the court flag since when switched it 
		// shouldn't get the court status of the controling player, but the mob
		return( lval_char != NULL && !IS_NPC(lval_char) && IS_COURT(lval_char) );
		
	case CHK_ISQUESTER:
		return( lval_char != NULL && !IS_NPC(lval_char) && IS_SET(lval_char->act, PLR_QUESTER) );
		
	case CHK_HASPKTIMER:
		return( lval_char != NULL && !IS_NPC(lval_char) 
			&& (UMAX(lval_char->pknoquit,lval_char->pknorecall)>0) );		
		
	case CHK_ISCHARM: /* A relic from MERC 2.2 MUDprograms */
		return( lval_char != NULL && IS_AFFECTED( lval_char, AFF_CHARM ) );
	case CHK_ISFOLLOW:
		return( lval_char != NULL && lval_char->master != NULL 
			&& lval_char->master->in_room == lval_char->in_room );
	case CHK_ISACTIVE:
		return( lval_char != NULL && lval_char->position > POS_SLEEPING );
	case CHK_ISDELAY:
		return( lval_char != NULL && lval_char->mprog_delay > 0 );

	case CHK_CHARHERE:
		return( lval_char != NULL && lval_char->in_room == mob->in_room);

	case CHK_ISVISIBLE:
		switch( code )
		{
		default :
		case 'i':
		case 'n':
		case 't':
		case 'r':
		case 'q':
			return( lval_char != NULL && can_see( mob, lval_char ) );
		case 'o':
		case 'p':
			return( lval_obj != NULL && can_see_obj( mob, lval_obj ) );
		}
		case CHK_HASTARGET:
			return( lval_char != NULL && lval_char->mprog_target != NULL
				&&  lval_char->in_room == lval_char->mprog_target->in_room );
		case CHK_ISTARGET:
			return( lval_char != NULL && mob->mprog_target == lval_char );

		case CHK_ISSHEATHED: 
			{
				if(lval_char == NULL){
					return false;
				}
				if ( get_eq_char( lval_char, WEAR_SHEATHED ) != NULL ){
					return true;
				}
				return false;
			}

		case CHK_ISCONCEALED: 
			{
				if(lval_char == NULL){
					return false;
				}
				if ( get_eq_char( lval_char, WEAR_CONCEALED ) != NULL ){
					return true;
				}
				return false;
			}

		default:;
	}
	
	/* 
	 * Case 4: Keyword, actor and value
	 */
	line = one_argument( line, buf );
	switch( check )
	{
	case CHK_AFFECTED:
		{
			if(!lval_char){
				mpbugf("affected check, lval==NULL prob target of check wasn't found.");
				return false;
			}
			if(NO_FLAG!=flag_lookup(buf, affect_flags))
			{
				if(IS_AFFECTED( lval_char, flag_lookup(buf, affect_flags))){
					return true;
				}			
			}
			int sn=skill_lookup( buf);
			if(sn<0){
				mpbugf("affected check for '%s', '%s' wasn't found.", buf, buf);
				return false;
			}
			if(is_affected( lval_char, sn)){
				return true;
			}
			return false;
			//			return( lval_char != NULL 
			//			&&  IS_SET(lval_char->affected_by, flag_lookup(buf, affect_flags)) );
		}
	case CHK_ACT:
		return( lval_char != NULL 
			&&  IS_SET(lval_char->act, flag_lookup(buf, act_flags)) );
	case CHK_ACT2:
		return( lval_char != NULL 
			&&  IS_SET(lval_char->act2, flag_lookup(buf, act2_flags)) );
	case CHK_IMM:
		return( lval_char != NULL 
			&&  IS_SET(lval_char->imm_flags, flag_lookup(buf, imm_flags)) );
	case CHK_OFF:
		return( lval_char != NULL 
			&&  IS_SET(lval_char->off_flags, flag_lookup(buf, off_flags)) );
	case CHK_CARRIES:
		if ( is_number( buf ) )
			return( lval_char != NULL && has_item( lval_char, (vn_int)atoi(buf), -1, false ) );
		else
			return( lval_char != NULL && (get_obj_carry( lval_char, buf ) != NULL) );
		
	case CHK_HASTOKEN:
		return( lval_char != NULL && has_token( lval_char, buf) );

	case CHK_HASEVENTGROUPQUEUED:
		return( lval_char != NULL && has_eventgroupqueued( lval_char, buf) );
		
	case CHK_WEARS:
		if ( is_number( buf ) )
			return( lval_char != NULL && has_item( lval_char, (vn_int)atoi(buf), -1, true ) );
		else
			return( lval_char != NULL && (get_obj_wear( lval_char, buf ) != NULL) );
	case CHK_HAS:
		return( lval_char != NULL && has_item( lval_char, -1, (short) item_lookup(buf), false ) );
	case CHK_USES:
		return( lval_char != NULL && has_item( lval_char, -1, (short) item_lookup(buf), true ));
	case CHK_NAME:
		switch( code )
		{
		default :
		case 'i':
		case 'n':
		case 't':
		case 'r':
		case 'q':
			return( lval_char != NULL && is_name( buf, lval_char->name ) );
		case 'o':
		case 'p':
			return( lval_obj != NULL && is_name( buf, lval_obj->name ) );
		}
		case CHK_EXACT_NAME:
            switch( code )
            {
			default :
			case 'i':
			case 'n':
			case 't':
			case 'r':
			case 'q':
				return( lval_char != NULL && is_exact_name( buf, lval_char->name ) );
			case 'o':
			case 'p':
				return( lval_obj != NULL && is_exact_name( buf, lval_obj->name ) );
			}
			
			case CHK_POS:
				return( lval_char != NULL && lval_char->position == position_lookup( buf ) );
			case CHK_CLAN:
				return( lval_char != NULL && lval_char->clan == clan_nlookup( buf ) );
			case CHK_RACE:
				{
					if (race_lookup( buf )==-1 )
						return( lval_char != NULL && lval_char->race == 0 );
					else
						return( lval_char != NULL && lval_char->race == race_lookup( buf ) );
				}
			case CHK_CLASS:
				return( lval_char != NULL && lval_char->clss == class_lookup( buf ) );
			case CHK_OBJTYPE:
				return( lval_obj != NULL && lval_obj->item_type == item_lookup( buf ) );
			case CHK_AREA:
				//				return( lval_obj != NULL && lval_char->in_room 
				return( lval_char->in_room 		
					&& lval_char->in_room->area==area_lookup( buf ));
			case CHK_SECTOR:
				return( lval_char != NULL && lval_char->in_room->sector_type == sector_lookup( buf ));
			case CHK_ROOMFLAG:
				return( lval_char != NULL
					&& IS_SET( lval_char->in_room->room_flags, flag_lookup( buf, room_flags )));
			default:;
    }
	
    /*
	 * Case 5: Keyword, actor, comparison and value
	 */
    if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
    {
		mpbugf( "Cmd_eval: prog %d syntax error(5): '%s'", vnum, original );
		return false;
    }
    one_argument( line, buf );
    rval = atoi( buf );
	
    switch( check )
    {
	case CHK_VNUM:
		switch( code )
		{
		default :
		case 'i':
		case 'n':
		case 't':
		case 'r':
		case 'q':
			if( lval_char != NULL && IS_NPC( lval_char ) )
				lval = lval_char->vnum();
			break;
		case 'o':
		case 'p':
			if ( lval_obj != NULL )
				lval = lval_obj->pIndexData->vnum;
		}
		break;
		
	case CHK_HPCNT:
		if ( lval_char != NULL ) lval = (lval_char->hit * 100)/(UMAX(1,lval_char->max_hit)); break;
	case CHK_ROOM:
		if ( lval_char != NULL && lval_char->in_room != NULL )
			lval = lval_char->in_room->vnum; break;
	case CHK_SEX:
		if ( lval_char != NULL ) lval = lval_char->sex; break;
	case CHK_LEVEL:
		switch( code )
		{
		default :
		case 'i':
		case 'n':
		case 't':
		case 'r':
		case 'q':
			if ( lval_char != NULL ) lval = lval_char->level; 
			break;
		case 'o':
		case 'p':
			if ( lval_obj != NULL ) lval = lval_obj->level;
		}
		break;

	case CHK_ALIGN:
		if ( lval_char != NULL ) lval = lval_char->alliance; break;
	case CHK_MONEY:  /* Money is converted to silver... */
		if ( lval_char != NULL ) 
			lval = lval_char->gold + (lval_char->silver * 100); break;
	case CHK_OBJVAL0:
		if ( lval_obj != NULL ) lval = lval_obj->value[0]; break;
	case CHK_OBJVAL1:
		if ( lval_obj != NULL ) lval = lval_obj->value[1]; break;
	case CHK_OBJVAL2: 
		if ( lval_obj != NULL ) lval = lval_obj->value[2]; break;
	case CHK_OBJVAL3:
		if ( lval_obj != NULL ) lval = lval_obj->value[3]; break;
	case CHK_OBJVAL4:
		if ( lval_obj != NULL ) lval = lval_obj->value[4]; break;
	case CHK_GRPSIZE:
		if( lval_char != NULL ) lval = count_people_room( lval_char, 4 ); break;
	case CHK_PKKILLS:
		if ( lval_char != NULL ) lval = lval_char->pkkills; break;
	case CHK_PKDEFEATS:
		if ( lval_char != NULL ) lval = lval_char->pkdefeats; break;
	case CHK_KARNS:
		if ( lval_char != NULL ) 
			lval = IS_NPC(lval_char)?0:lval_char->pcdata->karns; break;
	case CHK_RPS:
		if ( lval_char != NULL ) 
			lval = IS_NPC(lval_char)?0:lval_char->pcdata->rp_points; break;
	case CHK_DRUNK:
		if ( lval_char != NULL ) 
			lval = IS_NPC(lval_char)?0:lval_char->pcdata->condition[COND_DRUNK];
		break;
	case CHK_FULL:
		if ( lval_char != NULL ) 
			lval = IS_NPC(lval_char)?0:lval_char->pcdata->condition[COND_FULL];
		break;
	case CHK_THIRST:
		if ( lval_char != NULL )
			lval = IS_NPC(lval_char)?0:lval_char->pcdata->condition[COND_THIRST];
		break;
	case CHK_HUNGER:
		if ( lval_char != NULL )
			lval = IS_NPC(lval_char)?0:lval_char->pcdata->condition[COND_HUNGER];
		break;
	case CHK_TIRED:
		if ( lval_char != NULL )
			lval = IS_NPC(lval_char)?0:lval_char->pcdata->tired;
		break;
	case CHK_HASTRAINS:
		if ( lval_char != NULL )
			lval = lval_char->train;
		break;
	case CHK_HASPRACS:
		if ( lval_char != NULL )
			lval = lval_char->practice;
		break;
	case CHK_RACESIZE:
		if ( lval_char != NULL )
			lval = IS_NPC(lval_char)?0:race_table[lval_char->race]->size;
		break;
	default:
		return false;
	}
	if( num_eval( lval, oper, rval )){
		return true;
	}
	return false;

}
/**************************************************************************/
/*
 * ------------------------------------------------------------------------
 * EXPAND_ARG
 * This is a hack of act() in comm.c. I've added some safety guards,
 * so that missing or invalid $-codes do not crash the server
 * ------------------------------------------------------------------------
 */

 void expand_arg(
	 char *buf, const char *format, char_data *mob, char_data *ch, 
	 const void *arg1, const void *arg2, char_data *rch )
 {
	 const char *someone = "someone";
	 const char *something = "something";
	 const char *someones = "someone's";
	 
	 char fname[MIL];
	 char_data *vch = (char_data *) arg2;
	 OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
	 OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
	 const char *str;
	 const char *i;
	 char *point;
	 
	 /*
     * Discard null and zero-length messages.
     */
	 if ( format == NULL || format[0] == '\0' )
		 return;
	 
	 point   = buf;
	 str     = format;
	 while ( *str != '\0' )
	 {
		 if ( *str != '$' )
		 {
			 *point++ = *str++;
			 continue;
		 }
		 ++str;
		 
		 switch ( *str )
		 {
		 default:  bugf( "Expand_arg: bad code %d (%c).", *str, *str);
			 i = " <@@@> ";
			 break;
		 case 'i':
			 one_argument( mob->name, fname );
			 i = fname;
			 break;
		 case 'I': 
			 i = mob->short_descr;
			 break;
		 case 'n': 
			 i = someone;
			 if ( ch != NULL && can_see( mob, ch ) )
			 {
            	    one_argument( ch->name, fname );
					i = capitalize(fname);
			 }
			 break;
		 case 'N': 
			 i = (ch != NULL && can_see( mob, ch ) )
				 ? (ch->short_descr): someone;
			 break;
			 
			//	i = (ch != NULL && can_see( mob, ch ) )
			//	? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
			//	: someone;
			//	break;
			 
		 case 't': 
			 i = someone;
			 if ( vch != NULL && can_see( mob, vch ) )
			 {
				 one_argument( vch->name, fname );
				 i = capitalize(fname);
			 }
			 break;
		 case 'T': 
			 i = (vch != NULL && can_see( mob, vch ))
				 ? ( IS_NPC( vch ) ? vch->short_descr : vch->name )
				 : someone;
			 break;
		 case 'r': 
			 if ( rch == NULL ) 
				 rch = get_random_char( mob );
			 i = someone;
			 if( rch != NULL && can_see( mob, rch ) )
			 {
				 one_argument( rch->name, fname );
				 i = capitalize(fname);
			 }
			 break;
		 case 'R': 
			 if ( rch == NULL ) 
				 rch = get_random_char( mob );
			 i  = ( rch != NULL && can_see( mob, rch ) )
				 ?rch->short_descr:someone;
			 break;
		 case 'q':
			 i = someone;
			 if ( mob->mprog_target != NULL && can_see( mob, mob->mprog_target ) )
			 {
				 one_argument( mob->mprog_target->name, fname );
				 i = capitalize( fname );
			 }
			 break;
		 case 'Q':
			 i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
				 ? ( IS_NPC( mob->mprog_target ) ? mob->mprog_target->short_descr : mob->mprog_target->name )
				 : someone;
			 break;
		 case 'j':
			 i = he_she  [URANGE(0, mob->sex, 2)];
			 break;
		 case 'e': 
			 i = (ch != NULL && can_see( mob, ch ))
				 ? he_she  [URANGE(0, ch->sex, 2)]        
				 : someone;
			 break;
		 case 'E': 
			 i = (vch != NULL && can_see( mob, vch ))
				 ? he_she  [URANGE(0, vch->sex, 2)]        
				 : someone;
			 break;
		 case 'J': 
			 i = (rch != NULL && can_see( mob, rch ))
				 ? he_she  [URANGE(0, rch->sex, 2)]        
				 : someone;
			 break;
		 case 'X':
			 i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target))
				 ? he_she  [URANGE(0, mob->mprog_target->sex, 2)]
				 : someone;
			 break;
		 case 'k':
			 i = him_her [URANGE(0, mob->sex, 2)];
			 break;
		 case 'm': 
			 i = (ch != NULL && can_see( mob, ch ))
				 ? him_her [URANGE(0, ch  ->sex, 2)]
				 : someone;
			 break;
		 case 'M': 
			 i = (vch != NULL && can_see( mob, vch ))
				 ? him_her [URANGE(0, vch ->sex, 2)]        
				 : someone;
			 break;
		 case 'K':
			 if ( rch == NULL ) 
				 rch = get_random_char( mob );
			 i = (rch != NULL && can_see( mob, rch ))
				 ? him_her [URANGE(0, rch ->sex, 2)]
				 : someone;
			 break;
		 case 'Y': 
			 i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
				 ? him_her [URANGE(0, mob->mprog_target->sex, 2)]        
				 : someone;
			 break;
		 case 'l':
			 i = his_her [URANGE(0, mob ->sex, 2)];
			 break;
		 case 's': 
			 i = (ch != NULL && can_see( mob, ch ))
				 ? his_her [URANGE(0, ch ->sex, 2)]
				 : someones;
			 break;
		 case 'S': 
			 i = (vch != NULL && can_see( mob, vch ))
				 ? his_her [URANGE(0, vch ->sex, 2)]
				 : someones;
			 break;
		 case 'L': 
			 if ( rch == NULL ) 
				 rch = get_random_char( mob );
			 i = ( rch != NULL && can_see( mob, rch ) )
				 ? his_her [URANGE(0, rch ->sex, 2)]
				 : someones;
			 break;
		 case 'Z': 
			 i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
				 ? his_her [URANGE(0, mob->mprog_target->sex, 2)]
				 : someones;
			 break;
		 case 'o':
			 i = something;
			 if ( obj1 != NULL && can_see_obj( mob, obj1 ) )
			 {
            	    one_argument( obj1->name, fname );
                    i = fname;
			 }
			 break;
		 case 'O':
			 i = (obj1 != NULL && can_see_obj( mob, obj1 ))
				 ? obj1->short_descr
				 : something;
			 break;
		 case 'p':
			 i = something;
			 if ( obj2 != NULL && can_see_obj( mob, obj2 ) )
			 {
            	    one_argument( obj2->name, fname );
					i = fname;
			 }
			 break;
		 case 'P':
			 i = (obj2 != NULL && can_see_obj( mob, obj2 ))
				? obj2->short_descr
				: something;
			 break;

		 case 'u': // unique ID of the mob/object which the trigger is running on
			 i = FORMATF("#%d", mob->uid);
			 break;

		 case 'U': // unique ID of the character who caused the trigger
			 if ( ch ){
				i = FORMATF("#%d", ch->uid);
			 }else{
				 i = "#0"; // invalid unique id
			 }
			 break;

		 case 'V': // saved room
			 {
				 static char room_vnum_text[10];
				 sprintf(room_vnum_text,"%d", mob->mprog_remember_room_vnum);
				 i = room_vnum_text;
			 }
			 break;
        }
		
        ++str;
        while ( ( *point = *i ) != '\0' )
            ++point, ++i;
		
    }
    *point = '\0';
	
    return;
}    
/**************************************************************************/
char * mprog_type_to_name ( int type );
/**************************************************************************/
// used for mptrace command
int mptrace_current;
int mptrace_calledby[MAX_MPTRACE];
vn_int mptrace_pvnum[MAX_MPTRACE];
vn_int mptrace_mvnum[MAX_MPTRACE];
vn_int mptrace_rvnum[MAX_MPTRACE];
sh_int mptrace_triggered_on[MAX_MPTRACE];
unsigned char mptrace_calllevel[MAX_MPTRACE];
time_t mptrace_time[MAX_MPTRACE];
bool mptrace_aborted[MAX_MPTRACE];
/**************************************************************************/
void do_mptrace(char_data *ch, char *argument )
{
	char arg1[MIL];
	BUFFER *output;
	int lines_to_show;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The mptrace command is an olc command, you don't have olc permissions.");
		return;
	}

	if (!HAS_SECURITY(ch,7))
	{
		ch->println("You must have an OLC security of 7 or higher to use mptrace.");
		return;
	}

    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		if(ch->lines){
			lines_to_show=ch->lines-2;
		}else{
			lines_to_show=PAGELEN-2; // default page length 
		}
		lines_to_show%=MAX_MPTRACE;
	}
	else if(is_number( arg1 ))
	{
	    lines_to_show= atoi(arg1);	
		if (lines_to_show<1)
		{
			ch->println("Lines to show, increased to 1");
			lines_to_show=1;
		}
		else if(lines_to_show>MAX_MPTRACE)
		{
			ch->printlnf("Lines to show, decreased to the number logged in the trace - %d",
				MAX_MPTRACE);
			lines_to_show=MAX_MPTRACE;
		}		
	}
	else
	{
		ch->printf("`RThe only parameter for mptrace must be a numeric value\r\n"
			"for the number of lines of the trace you wish to see.`x\r\n");
		return;
	}
	ch->printlnf("Number of trace lines to show=%d", lines_to_show);

	output = new_buf();

	add_buf(output,"`xMPTrace `G#mudprog`x, `Ymobvnum`x/`robjvnum`x, `Binroom`x\r\n");
	
	// generate our lines of info
	int line=mptrace_current+MAX_MPTRACE-lines_to_show;
	int count;
	int i;
	int lc=0;
	char buf[MIL];
	for( count=0; count<lines_to_show; count++)
	{
		++line%=MAX_MPTRACE;

		if(mptrace_time[line]==0)
			continue;

		// put the line count and time per line
	    char * tbuf = ctime( &mptrace_time[line] );
		tbuf[str_len(tbuf)-6] = '\0';
		sprintf(buf,"[%3d]%s  ",++lc, (char *)&tbuf[11]);
		add_buf( output, buf);

		// indent to represent nested calling
		for(i=0; i<mptrace_calllevel[line]; i++){
			add_buf( output, "        ");
		}

		sprintf(buf,"`G%5d`x,`%c%5d`x,`B%5d`x, %s\r\n", 
			mptrace_pvnum[line],
			(mptrace_triggered_on[line]==MUDPROG_TRIGGERED_ON_OBJECT?'r':'Y'),			
			mptrace_mvnum[line],
			mptrace_rvnum[line],
			(mptrace_triggered_on[line]==MUDPROG_TRIGGERED_ON_OBJECT
				?flag_string(oprog_flags, mptrace_calledby[line])
				:flag_string(mprog_flags, mptrace_calledby[line]))
			);
		add_buf( output, buf);
	}
    ch->sendpage(buf_string(output));
    free_buf(output);
}

/**************************************************************************/
DECLARE_DO_FUN( do_mpdzecho		);  // Kal
void mpbuggy_prog(MUDPROG_TRIGGER_LIST *program, char * fmt, ...); 
/**************************************************************************/
/*
 * ------------------------------------------------------------------------
 *  PROGRAM_FLOW
 *  This is the program driver. It parses the mob program code lines
 *  and passes "executable" commands to interpret()
 *  Lines beginning with 'mob' are passed to mob_interpret() to handle
 *  special mob commands (in mob_cmds.c)
 *-------------------------------------------------------------------------
 */
#define MAX_NESTED_LEVEL 12 /* Maximum nested if-else-endif's (stack size) */
#define BEGIN_BLOCK       0 /* Flag: Begin of if-else-endif block */
#define IN_BLOCK         -1 /* Flag: Executable statements */
#define END_BLOCK        -2 /* Flag: End of if-else-endif block */
extern bool mp_object_action_prevented;
extern bool mp_suppress_text_output;
/**************************************************************************/
void program_flow( MUDPROG_TRIGGER_LIST *program,
	char_data *mob, char_data *ch, const void *arg1, const void *arg2 )
{
	char *source=program->prog->code;
	static bool init_mptrace=true;
	mp_object_action_prevented=false;
	mp_suppress_text_output=false;

	REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);
	REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);

	if(init_mptrace){
		memset(&mptrace_time[0], 0, sizeof(time_t));
		init_mptrace=false;
	}

	if(!program){
		bug("program_flow(): program==NULL!!!");
		mpbugf("program_flow(): program==NULL!!!");
		return;
	}
    vn_int pvnum=program->prog->vnum;

    char_data *rch = NULL;
    char *code, *line;
    char buf[MSL];
    char control[MIL], data[MSL];
	
	
	// moved to be a global
	//    static int call_level; /* Keep track of nested "mpcall"s */
	
	static time_t last_wiznet;
	static int count_since_wiznet;
	
    int level, check;
	bool eval;
    int state[MAX_NESTED_LEVEL], /* Block state (BEGIN,IN,END) */
		cond[MAX_NESTED_LEVEL];  /* Boolean value based on the last if-check */
	
	// moved to be globals
	// callstack system written by Kalahn - april 98

	// if the mob isn't considered valid, don't run progs on it.
	if(!mob || !IS_VALID(mob)){
		logf("Mob prog %d running on mob vnum %d - which is no longer valid!", pvnum, mob->vnum());
		if(mob){
			REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);
		}
		return;
	}
	
    vn_int mvnum;
	if(mob->running_mudprog_for_object){
		mvnum= mob->running_mudprog_for_object->pIndexData->vnum;
	}else{
		mvnum= mob->vnum();
	}
	
	callstack_pvnum[call_level]=pvnum;
	callstack_mvnum[call_level]=mvnum;
	callstack_rvnum[call_level]=mob->in_room->vnum;
	callstack_aborted[call_level]= true; // assumed abort till completed
	callstack_line[call_level]=0;

	++mptrace_current%=MAX_MPTRACE;
	mptrace_calledby[mptrace_current]=program->trig_type;

	if(mob->running_mudprog_for_object){
		mptrace_triggered_on[mptrace_current]=MUDPROG_TRIGGERED_ON_OBJECT;
	}else{
		mptrace_triggered_on[mptrace_current]=MUDPROG_TRIGGERED_ON_MOBILE;	
	}
	mptrace_pvnum[mptrace_current]=pvnum;
	mptrace_mvnum[mptrace_current]=mvnum;
	mptrace_rvnum[mptrace_current]=mob->in_room_vnum();	
	mptrace_time[mptrace_current]=current_time;
	mptrace_calllevel[mptrace_current]=call_level;


	// support CONFIG2_MP_TRIGGER_IN_ROOM
	for(char_data *pl=player_list; pl; pl=pl->next_player){
		if(HAS_CONFIG2(pl, CONFIG2_MP_TRIGGER_IN_ROOM)
			&& HAS_SECURITY(pl, MPEDIT_MINSECURITY)
			&& pl->in_room == mob->in_room
			&& IS_BUILDER_WHEN_NOT_RESTRICTED(pl, mob->in_room->area))
		{			
			pl->printlnf("MPTriggerInRoom:`GProg:%d`x, %s trigger on %s\r\n", 
				pvnum, 
				(mob->running_mudprog_for_object
					?flag_string(oprog_flags, program->trig_type)
					:flag_string(mprog_flags, program->trig_type)),
				PERS(mob, pl));			
		}

	}

    if( ++call_level > MAX_CALL_LEVEL )
    {
		char buf[MSL];
		int i;
		
		//bug( "MUDprogs: MAX_CALL_LEVEL exceeded, vnum %d", mob->vnum() );
		sprintf(buf, "MUDprogs BUG: MAX_CALL_LEVEL exceeded, running prog %d on mob %d", 
			pvnum, mvnum );
		log_string(buf);
		mpbug(buf);
		
		// display the call stack
		log_string("Mudprog callstack:\n");
		for (i=0; i<MAX_CALL_LEVEL;i++)
		{
			sprintf(buf, "[%2d] MUDprog %d on mob %d (in room %d)", i,
				callstack_pvnum[i], callstack_mvnum[i], callstack_rvnum[i]);
			log_string(buf);
		}
		
		// don't log the bug on wiznet more than once every 5 minutes
		if (last_wiznet<current_time-300)
		{
			mpbug(buf);
			// put it on the bug wiznet channel
			wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); 
			sprintf(buf, "WIZNET_BUGS: In the last 5 mins the bug has happened %d time%s.\r\n", 
				count_since_wiznet, (count_since_wiznet==1?"":"s") );
			wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); 
			
			wiznet("Mudprog callstack:\r\n",NULL,NULL,WIZ_BUGS,0,AVATAR); // put it on the bug wiznet channel
			
			// display the call stack
			for (i=0; i<MAX_CALL_LEVEL;i++)
			{
				sprintf(buf, "[%2d] MUDprog %d on mob %d (in room %d)", i,
					callstack_pvnum[i], callstack_mvnum[i], callstack_rvnum[i]);
				wiznet(buf,NULL,NULL,WIZ_BUGS,0,AVATAR); 
			}
			wiznet("Use `=Cmpinfo`x to see the info on mudprogs at anytime.\r\n"
				"and `=Cmpreset`x to reset the mudprogs (normally once the offending prog is fixed).\r\n",
				NULL,NULL,WIZ_BUGS,0,AVATAR); 
			last_wiznet = current_time;
			count_since_wiznet=0;
		}else{
			count_since_wiznet++;
		}
		REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);
		return;
    }

	// check if a prog is disabled
	if(program->prog->disabled)
	{
		char bufmsg[MSL];
		if(ch){
			sprintf(bufmsg,"Prog %d triggered (%s) on me by '%s', not run cause prog is disabled.",
				program->prog->vnum, mprog_type_to_name(program->trig_type), ch->name);
		}else{
			sprintf(bufmsg,"Prog %d triggered (%s), not run cause prog is disabled.",
				program->prog->vnum, mprog_type_to_name(program->trig_type));
		}
		do_mpdzecho(mob, bufmsg);
		call_level--;
		callstack_aborted[call_level]= false; 
		REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);
		return;
	}
	
    /*
	* Reset "stack"
	*/
    for ( level = 0; level < MAX_NESTED_LEVEL; level++ )
    {
		state[level] = IN_BLOCK;
        cond[level]  = true;
    }
    level = 0;
	
    code = source;
	SET_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
    /*
	* Parse the MUDprog code
	*/
    while ( *code && IS_VALID(mob))
    {
		bool first_arg = true;
		char *b = buf, *c = control, *d = data;
		/*
		* Get a command line. We sneakily get both the control word
		* (if/and/or) and the rest of the line in one pass.
		*/
		while( is_space( *code ) && *code ) {
			if(*code == '\n'){
				callstack_line[call_level-1]++;
			}
			code++;
		}

		while ( *code )
		{
			if ( *code == '\r' || *code == '\n'){
				break; // end of line - don't count the lines here... count them above
			}
			else if ( is_space(*code) )
			{
				if ( first_arg ){
					first_arg = false;
				}else{
					*d++ = *code;
				}
			}
			else
			{
				if ( first_arg ){
					*c++ = *code;
				}else{
					*d++ = *code;
				}
			}
			*b++ = *code++;
		}
		*b = *c = *d = '\0';
		
		if ( buf[0] == '\0' ){ // end of the source code
			break;
		}
		if ( buf[0] == '*' ){ // Comment - skip to end of line
			continue;
		}
		
        line = data;
		// Match control words
		if ( !str_cmp( control, "if" ) )
		{
			if ( state[level] == BEGIN_BLOCK )
			{
				mpbuggy_prog(program, "Mudprog: misplaced if statement, mob %d prog %d",
					mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
			}
			state[level] = BEGIN_BLOCK;
            if ( ++level >= MAX_NESTED_LEVEL )
            {
				mpbuggy_prog(program, "Mudprog: Max nested level exceeded, mob %d prog %d",
					mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
			}
			if ( level && cond[level-1] == false ) 
			{
				cond[level] = false;
				continue;
			}
			line = one_argument( line, control );

			bool negate=false;	
			char *pControl=control;
			if(*pControl=='!'){
				negate=true;
				pControl++; // skip over the !
			}

			if ( ( check = fn_keyword_lookup( pControl ) ) >= 0 )
			{
				if(negate){
					cond[level] = !cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
				}else{
					cond[level] = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
				}
			}
			else
			{
				mpbuggy_prog(program, "Mudprog: invalid if_check (if) '%s', mob %d prog %d", pControl, mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);		
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
			}
			state[level] = END_BLOCK;
		}
		else if ( !str_cmp( control, "or" ) )
		{
			if ( !level || state[level-1] != BEGIN_BLOCK )
			{
				mpbuggy_prog(program, "Mudprog: or without if, mob %d prog %d", mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);		
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
			}
			if ( level && cond[level-1] == false ){
				continue;	// if the parent nest is false, no point in processing the or
			}

			line = one_argument( line, control );
			bool negate=false;	
			char *pControl=control;
			if(*pControl=='!'){
				negate=true;
				pControl++; // skip over the !
			}

			if ( ( check = fn_keyword_lookup( pControl ) ) >= 0 )
			{
				eval = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
				if(negate){
					eval=!eval;
				}
			}
			else
            {
				mpbuggy_prog(program, "Mudprog: invalid if_check (or), mob %d prog %d", mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);		
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
            }
            cond[level] = ((bool)eval == true) ? true : cond[level];
		}
		else if ( !str_cmp( control, "and" ) )
		{
			if ( !level || state[level-1] != BEGIN_BLOCK )
			{
				mpbuggy_prog(program, "Mudprog: and without if, mob %d prog %d", mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);		
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
			}
			if ( level && cond[level-1] == false ) {
				continue; // if the parent nest is false, no point in processing the or
			}

			line = one_argument( line, control );

			bool negate=false;	
			char *pControl=control;
			if(*pControl=='!'){
				negate=true;
				pControl++; // skip over the !
			}

			if ( ( check = fn_keyword_lookup( pControl ) ) >= 0 )
			{
				eval = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
				if(negate){
					eval=!eval;
				}
			}
			else
			{
				mpbuggy_prog(program, "Mudprog: invalid if_check (and), mob %d prog %d", mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);		
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
			}
			if(!(cond[level] && eval)){
				cond[level]=false;
			}
		}
		else if ( !str_cmp( control, "endif" ) )
		{
			if ( !level || state[level-1] != BEGIN_BLOCK )
			{
				mpbuggy_prog(program, "Mudprog: endif without if, mob %d prog %d",	mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);		
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
			}
			cond[level] = true;
			state[level] = IN_BLOCK;
            state[--level] = END_BLOCK;
        }
		else if ( !str_cmp( control, "else" ) )
		{
			if ( !level || state[level-1] != BEGIN_BLOCK )
			{
				mpbuggy_prog(program, "Mudprog: else without if, mob %d prog %d", mvnum, pvnum );
				REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);		
				REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
				return;
			}
			if ( level && cond[level-1] == false ) continue;
            state[level] = IN_BLOCK;
			if(cond[level]){
				cond[level]=false; 
			}else{
				cond[level]=true; 
			}
        }
		else if ( cond[level] && ( !str_cmp( control, "break" ) || !str_cmp( control, "end" ) ) )
		{
			call_level--;
			callstack_aborted[call_level]= false; 
			REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);		
			REMOVE_BIT(mob->dyn,DYN_RUNNING_MUDPROG_CMD);
            return;
		}
		else if ( (!level || cond[level]) && buf[0] != '\0' )
		{
			state[level] = IN_BLOCK;
            expand_arg( data, buf, mob, ch, arg1, arg2, rch );
			if ( !str_cmp( control, "mp" ) || !str_cmp( control, "mob" ) ){
				// Found a mudprog restricted command, pass it to mob interpreter
				line = one_argument( data, control );
				mp_interpret( mob, line );
			}else{
				//Found a normal mud command, pass it to interpreter
				interpret( mob, data );
			}
		}
    }
    call_level--;
	callstack_aborted[call_level]= false; 
	REMOVE_BIT(mob->dyn,DYN_MOB_SEE_ALL);
}
/**************************************************************************/
// global place to disable a trigger running due to something about a 
// player, something about the trigger or something about the mudprog
// e.g. quester only triggers etc
// true means run the program
bool run_trigger_by_on( MUDPROG_TRIGGER_LIST *prg, char_data *mob, char_data *ch, int mtrig)
{
	// doesn't bother with prg yet - may do one day

	if(!ch) {
		return true;
	}
	assert(IS_NPC(mob));

	// do position checks 
	if(prg && prg->pos_flags){
		if(!IS_SET(prg->pos_flags, (1<<mob->position))){
			return false;
		}
	}

	// check if the mudprog is quester/nonquester only
	if(!IS_NPC(ch))
	{
		if(IS_SET(mob->act, ACT_MPIGN_QUESTER) && IS_QUESTER(ch))
			return false;

		if(IS_SET(mob->act, ACT_MPIGN_NONQUESTER) && !IS_QUESTER(ch))
			return false;
	}

	// check if the prog is recursing
	if(IS_RUNNING_TRIGGER(mob, mtrig) && !IS_SET(mob->act2, ACT2_MUDPROG_RECURSION_ALLOWED)){
		// don't run, as it is already running - prevent recursion to running a program with the same trigger on the same mob
		return false;
	}
	// mark the trigger type as being run for this mob
	SET_RUNNING_TRIGGER(mob,mtrig);
	return true;
}

/**************************************************************************/
void finished_running_prog_for_trigger( char_data *mob, char_data *ch, int mtrig)
{
	// clear the trigger type as being run for this mobile
	REMOVE_RUNNING_TRIGGER(mob,mtrig);
}


/**************************************************************************/
/* 
 * ---------------------------------------------------------------------
 * Trigger handlers. These are called from various parts of the code
 * when an event is triggered.
 * ---------------------------------------------------------------------
 */

/*
 * A general purpose string trigger. Matches argument to a string trigger
 * phrase.
 */
void mp_act_trigger( 
	char *argument, char_data *mob, char_data *ch, 
	const void *arg1, const void *arg2, int type )
{
	MUDPROG_TRIGGER_LIST *prg;
	
	// check for flags on mob and player that might prevent prog being run
	if (!run_trigger_by_on( NULL, mob, ch, type)) return;

	for ( prg = mob->pIndexData->mob_triggers; prg != NULL; prg = prg->next )
	{
		if ( prg->trig_type == type 
		&& (!str_infix(prg->trig_phrase, argument) 
		|| !str_cmp(prg->trig_phrase, "*always_trigger*")))
		{
			program_flow( prg, mob, ch, arg1, arg2 );
			break;
		}
	}
	// flag the trigger as finished running, for the recursion prevention code
	finished_running_prog_for_trigger(mob, ch, type);

	return;
}
/**************************************************************************/
bool mp_cmd_trigger( 
	char *argument, char_data *mob, char_data *ch, 
	const void *arg1, const void *arg2, int type )
{
	// check for flags on mob and player that might prevent prog being run
	if (!run_trigger_by_on( NULL, mob, ch, type)) return false;

	MUDPROG_TRIGGER_LIST *prg;
	bool cmdfound = false;

	for ( prg = mob->pIndexData->mob_triggers; prg != NULL; prg = prg->next )
	{
		if ( prg->trig_type == type 
		&& !str_cmp(prg->trig_phrase, argument))
		{
			program_flow( prg, mob, ch, arg1, arg2 );
			cmdfound = true;
			break;
		}
	}
	// flag the trigger as finished running, for the recursion prevention code
	finished_running_prog_for_trigger(mob, ch, type);

	return cmdfound;
}
/**************************************************************************/
/*
 * A general purpose percentage trigger. Checks if a random percentage
 * number is less than trigger phrase
 */
bool mp_percent_trigger( 
	char_data *mob, char_data *ch, 
	const void *arg1, const void *arg2, int type )
{
	// check for flags on mob and player that might prevent prog being run
	if (!run_trigger_by_on( NULL, mob, ch, type)) return false;

    MUDPROG_TRIGGER_LIST *prg;

    for ( prg = mob->pIndexData->mob_triggers; prg != NULL; prg = prg->next )
	{
		if ( prg->trig_type == type 
			&&   number_percent() < atoi( prg->trig_phrase ) )
		{
			program_flow( prg, mob, ch, arg1, arg2 );
			// flag the trigger as finished running, for the recursion prevention code
			finished_running_prog_for_trigger(mob, ch, type);
			return true;
		}
	}
	// flag the trigger as finished running, for the recursion prevention code
	finished_running_prog_for_trigger(mob, ch, type);

    return false;
}
/**************************************************************************/
void mp_bribe_trigger( char_data *mob, char_data *ch, int amount )
{
	// check for flags on mob and player that might prevent prog being run
	if (!run_trigger_by_on( NULL, mob, ch, MTRIG_BRIBE)) return;
  
	MUDPROG_TRIGGER_LIST *prg;
	MUDPROG_TRIGGER_LIST *highprog=NULL;
	int highest=-1;

	// first find the most expensive bribe trigger that matches 
	// which we gave the money for - if there are multiple progs
    for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next )
    {
		if ( prg->trig_type == MTRIG_BRIBE
			&& amount >= atoi( prg->trig_phrase ))
		{
			if(highest< atoi( prg->trig_phrase)){
				highest=atoi( prg->trig_phrase);
				highprog=prg;
			}
		}
	}

	// run the highest prog if one was found
	if(highprog){		
		program_flow( highprog, mob, ch, NULL, NULL );
	}

	// flag the trigger as finished running, for the recursion prevention code
	finished_running_prog_for_trigger(mob, ch, MTRIG_BRIBE);

    return;
}
/**************************************************************************/
void mp_hour_trigger( char_data *ch )
{
	MUDPROG_TRIGGER_LIST	*prg;
	int			hour;

	for ( prg = ch->pIndexData->mob_triggers; prg; prg = prg->next )
	{
		hour = time_info.hour;
		if ( prg->trig_type == MTRIG_HOUR
		&& ( hour %= 24 ) == atoi( prg->trig_phrase ))
		{
			program_flow( prg, ch, NULL, NULL, NULL );
			return;
		}
	}
	return;
}
/**************************************************************************/
bool mp_exit_trigger_no_letpass=true;
/**************************************************************************/
// NOTE - There is no recursion prevention for MTRIG_EXIT or MTRIG_EXALL
//        as it could break mudprogs running with followers
bool mp_exit_trigger( char_data *ch, int dir )
{
    char_data *mob;
    MUDPROG_TRIGGER_LIST   *prg;
	int processed_dir=0;

	// exit triggers don't work on wizi LEVEL_IMMORTAL+ imms
	if (INVIS_LEVEL(ch)>=LEVEL_IMMORTAL 
		&& IS_IMMORTAL(ch)){
		return false;
	}

	mp_exit_trigger_no_letpass=true; // use mob letpass to set this to false
	mob = ch->in_room->people;
	int number_in_room=ch->in_room->number_in_room;
	// note: number_in_room is used to prevent a mudprog on two mobs in 
	// a room creating an endless loop by removing themselves from
	// the room and putting themselves back in the room - Kal, June 01
    for ( ; mob && --number_in_room>=0; mob = mob->next_in_room )
    {    
		if ( IS_NPC( mob )
			&&   ( HAS_TRIGGER(mob, MTRIG_EXIT) || HAS_TRIGGER(mob, MTRIG_EXALL) ) )
		{
			// check for flags on mob and player that might prevent prog being run
			// because the mtrig parameter is 0, recursion prevention is not affected
			if (!run_trigger_by_on( NULL, mob, ch, 0)) continue; 

			for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next )
			{
			/*
			* Exit trigger works only if the mobile is not busy
			* (fighting etc.). If you want to be sure all players
			* are caught, use ExAll trigger
				*/
				// check for an all direction trigger
				if ( prg->trig_type == MTRIG_EXIT || prg->trig_type == MTRIG_EXALL)
				{ 
					if(is_exact_name("all", prg->trig_phrase)){
						if ( prg->trig_type == MTRIG_EXIT
							&&  mob->position == mob->pIndexData->default_pos
							&&  can_see( mob, ch ) )
						{
							program_flow( prg, mob, ch, NULL, NULL );
							return mp_exit_trigger_no_letpass;
						}else{
							if ( prg->trig_type == MTRIG_EXALL)
							{
								program_flow( prg, mob, ch, NULL, NULL );
								return mp_exit_trigger_no_letpass;
							}
						}
					}
				}

				// figure out the direction
				if ( prg->trig_type == MTRIG_EXIT || prg->trig_type == MTRIG_EXALL)
				{
					if(is_number(prg->trig_phrase)){
						processed_dir=atoi( prg->trig_phrase );
					}else{
						processed_dir=dir_lookup(prg->trig_phrase);					
					}

					if(processed_dir<0 || processed_dir>=MAX_DIR){
						mpbugf("exit_trigger: direction phrase '%s' isn't a valid direction, must be direction number or text version (n,s,ne, southwest,... etc)",
							prg->trig_phrase);
						continue;
					}
				}

				if ( prg->trig_type == MTRIG_EXIT
					&&  dir == processed_dir
					&&  mob->position == mob->pIndexData->default_pos
					&&  can_see( mob, ch ) )
				{
					program_flow( prg, mob, ch, NULL, NULL );
					return mp_exit_trigger_no_letpass;
				}else{
					if ( prg->trig_type == MTRIG_EXALL
						&&   dir == processed_dir )
					{
						program_flow( prg, mob, ch, NULL, NULL );
						return mp_exit_trigger_no_letpass;
					}
				}
			}
		}
    }
    return false;
}
/**************************************************************************/
// returns true if the trigger would normally be run
bool mp_would_run_give_trigger( char_data *mob, char_data *ch, OBJ_DATA *obj )
{
	// check for flags on mob and player that might prevent prog being run
	// because the mtrig parameter is 0, recursion prevention is not affected
	if (!run_trigger_by_on( NULL, mob, ch, 0)) return false;
	
    char buf[MIL], *p;
    MUDPROG_TRIGGER_LIST  *prg;
    for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next ){
		if ( prg->trig_type == MTRIG_GIVE ){
			p = prg->trig_phrase;
			if ( is_number( p ) ){
				// Vnum argument
				if ( obj->pIndexData->vnum == atoi(p) ){
					return true;
				}
			}else{
				// Object name argument, e.g. 'sword'
				while( *p ){
					p = one_argument( p, buf );
					
					if ( is_name( buf, obj->name )
						||   !str_cmp( "all", buf ) )
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}
/**************************************************************************/
void mp_give_trigger( char_data *mob, char_data *ch, OBJ_DATA *obj )
{
	// check for flags on mob and player that might prevent prog being run
	if (!run_trigger_by_on( NULL, mob, ch, MTRIG_GIVE)) return;
	
    char        buf[MIL], *p;
    MUDPROG_TRIGGER_LIST  *prg;
	
    for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next ){
		if ( prg->trig_type == MTRIG_GIVE )
		{
			p = prg->trig_phrase;
			/*
			* Vnum argument
			*/
			if ( is_number( p ) )
			{
				if ( obj->pIndexData->vnum == atoi(p) )
				{
					program_flow(prg, mob, ch, (void *) obj, NULL);
					// flag the trigger as finished running, for the recursion prevention code
					finished_running_prog_for_trigger(mob, ch, MTRIG_GIVE);
					return;
				}
			}
			/*
			* Object name argument, e.g. 'sword'
			*/
			else
			{
				while( *p )
				{
					p = one_argument( p, buf );
					
					if ( is_name( buf, obj->name )
						||   !str_cmp( "all", buf ) )
					{
						program_flow(prg, mob, ch, (void *) obj, NULL);
						// flag the trigger as finished running, for the recursion prevention code
						finished_running_prog_for_trigger(mob, ch, MTRIG_GIVE);
						return;
					}
				}
			}
		}
	}
	// flag the trigger as finished running, for the recursion prevention code
	finished_running_prog_for_trigger(mob, ch, MTRIG_GIVE);	
}
/**************************************************************************/
void mp_greet_trigger( char_data *ch )
{
    char_data *mob;

	// greet triggers don't work on wizi LEVEL_IMMORTAL+ imms
	if (INVIS_LEVEL(ch)>=LEVEL_IMMORTAL 
		&& IS_IMMORTAL(ch)){
		return;
	}

	mob = ch->in_room->people;
	int number_in_room=ch->in_room->number_in_room;
	// note: number_in_room is used to prevent a mudprog on two mobs in 
	// a room creating an endless loop by removing themselves from
	// the room and putting themselves back in the room - Kal, June 01
    for ( ; mob && --number_in_room>=0; mob = mob->next_in_room )
    {   		
		if ( IS_NPC( mob ))
		{
			// check for flags on mob and player that might prevent prog being run
			if (!run_trigger_by_on( NULL, mob, ch, MTRIG_GREET)) continue;

			// Greet trigger works only if the mobile is in their 
			// default position and can see the character
			// If you want to catch all players, use a GrAll trigger
			if ( HAS_TRIGGER( mob,MTRIG_GREET )
				&&   mob->position == mob->pIndexData->default_pos
				&&   can_see( mob, ch ) 
				&&   !IS_AFFECTED(ch, AFF_SNEAK) )
			{
				mp_percent_trigger( mob, ch, NULL, NULL, MTRIG_GREET );
			}

			// GrAll triggers will greet all players except
			// wizi LEVEL_IMMORTAL+ immortals
			if ( HAS_TRIGGER( mob, MTRIG_GRALL )){
				mp_percent_trigger( mob, ch, NULL, NULL, MTRIG_GRALL );
			}

			// flag the trigger as finished running, for the recursion prevention code
			finished_running_prog_for_trigger(mob, ch, MTRIG_GREET);
		}
    }
    return;
}
/**************************************************************************/
void mp_hprct_trigger( char_data *mob, char_data *ch )
{
	// check for flags on mob and player that might prevent prog being run
	if (!run_trigger_by_on( NULL, mob, ch, MTRIG_HPCNT)) return;

    MUDPROG_TRIGGER_LIST *prg;

    for ( prg = mob->pIndexData->mob_triggers; prg != NULL; prg = prg->next )
	if ( ( prg->trig_type == MTRIG_HPCNT )
	&& ( (100 * mob->hit / mob->max_hit) < atoi( prg->trig_phrase ) ) )
	{
	    program_flow( prg, mob, ch, NULL, NULL );
	    break;
	}
	// flag the trigger as finished running, for the recursion prevention code
	finished_running_prog_for_trigger(mob, ch, MTRIG_HPCNT);
}
/**************************************************************************/
void do_mpreset( char_data *ch, char *)
{
	int i;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The mpreset command is an olc command, you don't have olc permissions.");
		return;
	}

	if (!HAS_SECURITY(ch,7))
	{
		ch->println("You must have an OLC security of 7 or higher to use mpreset.");
		return;
	}


	ch->println("Mudprogs reseted.  (mudprog callstack depth to 0)");
	for (i=0; i<MAX_CALL_LEVEL;i++)
	{
		callstack_pvnum[i]=0;
		callstack_mvnum[i]=0;
		callstack_rvnum[i]=0;
		callstack_aborted[i]=false;
	}
	call_level=0;

	mpbugf( "%s reset the mudprog callstack", TRUE_CH(ch)->name);	
    return;
}
/**************************************************************************/
void do_mpinfo( char_data *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char arg1 [MIL];
	int i;

    BUFFER *output;

	if (!HAS_SECURITY(ch,1)){
		ch->println("The mpinfo command is an olc command, you don't have olc permissions.");
		return;
	}

	if (!HAS_SECURITY(ch,7))
	{
		ch->println("You must have an OLC security of 7 or higher to use mpinfo.");
		return;
	}


    output= new_buf();

	sprintf( buf,"`?`#%s`x", format_titlebar("MUDPROGS INFO"));
	add_buf(output,buf);
	
	sprintf(buf,"    Displaying mudprog call abort history - calllevel currently is %d\r\n", call_level);
	add_buf(output,buf);
	
	// display the call stack
	for (i=0; i<MAX_CALL_LEVEL;i++)
	{
		if (callstack_aborted[i])
		{
			sprintf(buf, "      [%2d] MUDprog %d on mob %d (in room %d) `RABORTED!!! (BUGGY?)`X\r\n", i,
				callstack_pvnum[i], callstack_mvnum[i], callstack_rvnum[i]);
			add_buf(output,buf);
		}
		else
		{
			sprintf(buf, "      [%2d] MUDprog %d on mob %d (in room %d)\r\n", i,
				callstack_pvnum[i], callstack_mvnum[i], callstack_rvnum[i]);
			add_buf(output,buf);
		}
	}

	add_buf(output,"`xMPBUG `G#mudprog`x, `Ymobvnum`x, `Binroom`x,`rline`x,`Ccall_level`x:   `WNOTE: see also `=CMPTRACE`x \r\n");

	{ // loop thru finding all disabled progs
		MUDPROG_CODE *prg;
		char msgbuf[MSL];

		for( prg = mudprog_list; prg; prg = prg->next )
		{
			if ( prg->disabled){
				sprintf(msgbuf,"`SMudprog %d is disabled with the following text:`x.\r\n",
					prg->vnum);
				add_buf(output,msgbuf);
				add_buf(output,prg->disabled_text);
				add_buf(output,"`x");			
			}			
		}
	}    


	// now display the specified number of lines of the log file.
    argument = one_argument( argument, arg1 );
	if (IS_NULLSTR(arg1))
	{
		sprintf(buf, "tail " MPBUG_FILE " -n 10");
		add_buf(output, "\r\n    You can select the number of loglines, type mpinfo <number of lines>");
	
	}
	else if (is_number ( arg1 ))
	{
	    int value = atoi(arg1);	
		if (value<1 || value>20000)
		{
			add_buf(output,"\r\n`RNumber of lines to tail must be between 1 and 20000.`x\r\n");
			sprintf(buf, "tail " MPBUG_FILE " -n 10");
		}
		else
		{
			sprintf(buf, "tail " MPBUG_FILE " -n %d", value);
		}	
	}
	else
	{
		add_buf(output, "\r\n`RThe only parameter for this command must be a "
			"numeric value\r\nfor the number of lines of the error "
			"log you wish to see.`x\r\n");
		sprintf(buf, "tail " MPBUG_FILE " -n 10");
	}

	sprintf( buf2,"\r\n`^%s`x", format_titlebarf("Piping:`x %s", buf));
	add_buf(output,buf2);

	add_buf(output,get_piperesult(buf));

	ch->sendpage(buf_string(output));
	free_buf(output);

    return;
}
/**************************************************************************/
bool mp_prekill_trigger_result;
/**************************************************************************/
// return true if our triggers stops them
bool mp_prekill_trigger( char_data *mob, char_data *attacker)
{
	// check for flags on mob and player that might prevent prog being run
	if (!run_trigger_by_on( NULL, mob, attacker, MTRIG_PREKILL)) return false;

	mp_prekill_trigger_result=false;

    MUDPROG_TRIGGER_LIST  *prg;

    for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next )
    {
		if ( prg->trig_type == MTRIG_PREKILL 
			&&   number_percent() < atoi( prg->trig_phrase ) )
        {
			program_flow( prg, mob, attacker, NULL, NULL);
			// flag the trigger as finished running, for the recursion prevention code
			finished_running_prog_for_trigger(mob, attacker, MTRIG_PREKILL);
			return mp_prekill_trigger_result;
		}
    }
	// flag the trigger as finished running, for the recursion prevention code
	finished_running_prog_for_trigger(mob, attacker, MTRIG_PREKILL);
    return false;
}
/**************************************************************************/
bool mp_premove_trigger_allowmove=true;
int active_premove_trigger_count=0; // don't allow more than 3
/**************************************************************************/
// return true if they can move - default
bool mp_premove_trigger( char_data *mob, int vnum, int dir)
{
	if(active_premove_trigger_count>3){
		mpbugf("premove_trigger: active_premove_trigger_count is %d - "
			"greater than 3, trigger ignored.", active_premove_trigger_count);
		return true;
	}
	active_premove_trigger_count++;
	mp_premove_trigger_allowmove=true; // default to allowing movement

	// record the vnum of where the mob is moving to
	mob->mprog_remember_room_vnum=vnum;
	
	int processed_dir;
	MUDPROG_TRIGGER_LIST  *prg;

	for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next )
	{
		// check for an all direction trigger
		if ( prg->trig_type == MTRIG_PREMOVE)
		{ 
			if(is_exact_name("all", prg->trig_phrase)){
				program_flow( prg, mob, mob, NULL, NULL );
				active_premove_trigger_count--;
				return mp_premove_trigger_allowmove;
			}

			// calculate the direction
			if(is_number(prg->trig_phrase)){
				processed_dir=atoi( prg->trig_phrase );
			}else{
				processed_dir=dir_lookup(prg->trig_phrase);					
			}
			if(processed_dir<0 || processed_dir>=MAX_DIR){
				mpbugf("premove_trigger: direction phrase '%s' isn't a "
					"valid direction, must be direction number or "
					"text version (n,s,ne, southwest,... etc)",
					prg->trig_phrase);
				continue;
			}
			if(processed_dir==dir){
				program_flow( prg, mob, mob, NULL, NULL );
				active_premove_trigger_count--;
				return mp_premove_trigger_allowmove;
			}
		}
	}
	active_premove_trigger_count--;
    return true;
}
/**************************************************************************/
bool mp_login_trigger_processed=false;
/**************************************************************************/
// mp_login_trigger runs two triggers, loginroom, loginarea.
// if a mob called 'mob loginprocessed' then no more triggers on mobs 
// for this login will be run.
// loginroom triggers are run before loginarea
// ch = character logging in, ran just after the player has been placed in the room
void mp_login_trigger( char_data *ch)
{
	mp_login_trigger_processed=false; // default to no mob processed the login

	if(IS_NPC(ch)) return; // don't trigger on pet logins

	assertp(ch->in_room);
	assertp(ch->in_room->area);

	//loop thru all mobs in the room first - TRIG_LOGINROOM
	MUDPROG_TRIGGER_LIST  *prg;
	char_data *mob, *mob_next_in_room;
	for(mob=ch->in_room->people; mob; mob=mob_next_in_room){
		mob_next_in_room=mob->next_in_room;
		
		// ** reasons for the prog not to run etc
		if(!IS_NPC(mob)) continue;
		// check for flags on mob and player that might prevent prog being run 
		if (!run_trigger_by_on( NULL, mob, ch, MTRIG_LOGINROOM)) continue;

		for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next )
		{
			if ( prg->trig_type != MTRIG_LOGINROOM 
				|| number_percent()>atoi( prg->trig_phrase)){
				continue;
			}

			program_flow( prg, mob, ch, NULL, NULL);

			if(mp_login_trigger_processed){
				// flag the trigger as finished running, for the recursion prevention code
				finished_running_prog_for_trigger(mob, ch, MTRIG_LOGINROOM);
				return; // a mudprog marked the login as processed
			}
		}
		// flag the trigger as finished running, for the recursion prevention code
		finished_running_prog_for_trigger(mob, ch, MTRIG_LOGINROOM);
	}

	//loop thru all mobs in the area - MTRIG_LOGINAREA
	char_data *mob_next;
	for(mob=char_list; mob; mob=mob_next){
		mob_next=mob->next;
		
		// ** reasons for the prog not to run etc
		if(!IS_NPC(mob)) continue;
		if(!mob->in_room || mob->in_room->area!=ch->in_room->area) continue;
		// check for flags on mob and player that might prevent prog being run 
		if (!run_trigger_by_on( NULL, mob, ch, MTRIG_LOGINAREA)) continue;

		for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next )
		{
			if ( prg->trig_type != MTRIG_LOGINAREA
				|| number_percent()>atoi( prg->trig_phrase)){
				continue;
			}

			program_flow( prg, mob, ch, NULL, NULL);

			if(mp_login_trigger_processed){
				// flag the trigger as finished running, for the recursion prevention code
				finished_running_prog_for_trigger(mob, ch, MTRIG_LOGINAREA);
				return; // a mudprog marked the login as processed
			}
		}
		// flag the trigger as finished running, for the recursion prevention code
		finished_running_prog_for_trigger(mob, ch, MTRIG_LOGINAREA);
	}
}
/**************************************************************************/
bool mp_logout_trigger_processed=false;
/**************************************************************************/
// mp_logout_trigger runs two triggers, logoutroom, logoutarea.
// if a mob called 'mob logoutprocessed' then no more triggers on mobs 
// for this logout will be run.
// logoutroom triggers are run before logoutarea
// ch = character logging out, is run just before any of the logout checks - like pknoquit etc
void mp_logout_trigger( char_data *ch)
{
	mp_logout_trigger_processed=false; // default to no mob processed the logout

	if(IS_NPC(ch)) return; // don't trigger on pet logout

	assertp(ch->in_room);
	assertp(ch->in_room->area);

	//loop thru all mobs in the room first - TRIG_LOGOUTROOM
	MUDPROG_TRIGGER_LIST  *prg;
	char_data *mob, *mob_next_in_room;
	for(mob=ch->in_room->people; mob; mob=mob_next_in_room){
		mob_next_in_room=mob->next_in_room;
		
		// ** reasons for the prog not to run etc
		if(!IS_NPC(mob)) continue;
		// check for flags on mob and player that might prevent prog being run 
		if (!run_trigger_by_on( NULL, mob, ch, MTRIG_LOGOUTROOM)) continue;

		for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next )
		{
			if ( prg->trig_type != MTRIG_LOGOUTROOM 
				|| number_percent()>atoi( prg->trig_phrase)){
				continue;
			}

			program_flow( prg, mob, ch, NULL, NULL);

			if(mp_logout_trigger_processed){
				// flag the trigger as finished running, for the recursion prevention code
				finished_running_prog_for_trigger(mob, ch, MTRIG_LOGOUTROOM);
				return; // a mudprog marked the logout as processed
			}
		}
		// flag the trigger as finished running, for the recursion prevention code
		finished_running_prog_for_trigger(mob, ch, MTRIG_LOGOUTROOM);
	}

	//loop thru all mobs in the area - MTRIG_LOGOUTAREA
	char_data *mob_next;
	for(mob=char_list; mob; mob=mob_next){
		mob_next=mob->next;
		
		// ** reasons for the prog not to run etc
		if(!IS_NPC(mob)) continue;
		if(!mob->in_room || mob->in_room->area!=ch->in_room->area) continue;
		// check for flags on mob and player that might prevent prog being run 
		if (!run_trigger_by_on( NULL, mob, ch, MTRIG_LOGOUTAREA)) continue;

		for ( prg = mob->pIndexData->mob_triggers; prg; prg = prg->next )
		{
			if ( prg->trig_type != MTRIG_LOGOUTAREA
				|| number_percent()>atoi( prg->trig_phrase)){
				continue;
			}

			program_flow( prg, mob, ch, NULL, NULL);

			if(mp_logout_trigger_processed){
				// flag the trigger as finished running, for the recursion prevention code
				finished_running_prog_for_trigger(mob, ch, MTRIG_LOGOUTAREA);
				return; // a mudprog marked the logout as processed
			}
		}
		// flag the trigger as finished running, for the recursion prevention code
		finished_running_prog_for_trigger(mob, ch, MTRIG_LOGOUTAREA);
	}
}

/**************************************************************************/
// *** create a mobile which is used to run the object prog
// *** put it in the room of the object
// *** transfer the objects contained by the object into its inventory
// return the newly created mob
// return NULL if there was a problem creating the mob 
// return NULL if there was a problem finding somewhere to put the mob
//   obj must exist, ch can be NULL, otrig can be 0
char_data *objectmudprog_generate_mobified_object(obj_data *obj, char_data * ch, int otrig)
{
	char_data *objectmudprog_mob;
	
	// first find the mob vnum allocated
	MOB_INDEX_DATA *pMobIndex=get_mob_index(MOB_VNUM_TO_RUN_OBJECT_MUDPROGS);
	if(!pMobIndex){
		ch->bug_printlnf("objectmudprog_generate_mobified_object(): "
			"Couldn't find mob index %d to create a mob to run object prog %d (trigger type %d)... ",
			MOB_VNUM_TO_RUN_OBJECT_MUDPROGS,					
			obj->pIndexData->vnum, otrig);
		return NULL;
	}

	// create the objectmudprog_mob
	objectmudprog_mob= new_char();
	objectmudprog_mob->pIndexData	= pMobIndex;
	objectmudprog_mob->name			= str_dup( obj->name);    
	objectmudprog_mob->short_descr	= str_dup( obj->short_descr);    
	objectmudprog_mob->long_descr	= str_dup( pMobIndex->long_descr);     
	objectmudprog_mob->description	= str_dup( obj->description );
	objectmudprog_mob->level		= obj->level;
	objectmudprog_mob->player_id	= obj->pIndexData->vnum;
	objectmudprog_mob->uid			= obj->uid;
	objectmudprog_mob->language		= language_native;
	objectmudprog_mob->running_mudprog_for_object=obj;
	SET_BIT(objectmudprog_mob->dyn,DYN_MOB_SEE_ALL);

	// put the objectmudprog_mob into a room, 
	// if we can't we report an error and abort the program
	if(obj->in_room){
		char_to_room(objectmudprog_mob, obj->in_room);
	}else if( obj->carried_by && obj->carried_by->in_room){
		char_to_room(objectmudprog_mob, obj->carried_by->in_room);
	}else if( ch && ch->in_room){
		char_to_room(objectmudprog_mob, ch->in_room);
	}else{
		bugf("objectmudprog_generate_mobified_object(): Couldn't find room to put "
			"objectmudprog_mob, trigger type %d on object %d", 
			otrig, obj->pIndexData->vnum);
		ch->printlnf("BUG: Couldn't find room to put objectmudprog_mob, "
			"trigger type %d on object %d, please report this to the admin.", 
			otrig, obj->pIndexData->vnum);
		free_char(objectmudprog_mob);
		return NULL;
	}

	// transfer what the object contains to the objectmudprog_mob	
	{	
		assert(objectmudprog_mob->carrying==NULL);

		objectmudprog_mob->carrying=obj->contains;
		obj->contains=NULL;

		// update every object being carried, telling it that it is now
		// carried_by the objectmudprog_mob instead of inside the object
		for(obj_data *o=objectmudprog_mob->carrying; o; o=o->next_content){
			o->in_obj		= NULL;
			o->carried_by	= objectmudprog_mob;
		}
	}

	return objectmudprog_mob;
}

/**************************************************************************/
// purge a mobified object, absorbing what is carries back into the 
// original object 
void objectmudprog_degenerate_mobified_object(char_data *objectmudprog_mob)
{
	if(objectmudprog_mob && IS_VALID(objectmudprog_mob)){
		obj_data *obj=objectmudprog_mob->running_mudprog_for_object;

		// transfer any contents back to the object if it still exists
		if(obj && IS_VALID(obj)){
			obj->contains=objectmudprog_mob->carrying;
			objectmudprog_mob->carrying=NULL;

			for(obj_data *o=obj->contains; o; o=o->next_content){
				o->in_obj		= obj;
				o->carried_by	= NULL;
			}
		}

		char_from_room(objectmudprog_mob);
		free_char(objectmudprog_mob);
	}
}
/**************************************************************************/
// return true if a program was run
bool object_program_flow(MUDPROG_TRIGGER_LIST *prg, obj_data *obj, char_data *ch, obj_data *obj2)
{
	char_data *objectmudprog_mob=NULL;

	// generate a mob to run the prog on behalf of the object (create + transfer what object contains to inventory)
	objectmudprog_mob=objectmudprog_generate_mobified_object(obj, ch, prg->trig_type);
	if(!objectmudprog_mob){
		return false;
	}

	mp_object_action_prevented=false;
	mp_suppress_text_output=false;
	program_flow(prg, objectmudprog_mob, ch, (void *) obj2, NULL);

	// absorb mob back into the object (transfer objects back + purge)
	objectmudprog_degenerate_mobified_object(objectmudprog_mob);

	return true;
}
/**************************************************************************/
bool mp_object_action_prevented;
bool mp_suppress_text_output; // used to suppress the output of text 
								// to players for the duration of the command
/**************************************************************************/
// return true if a program was run, 
// AND the program requested the action is cancelled (for pre triggers)
bool oprog_execute_if_appropriate(obj_data *obj, char_data * ch, int otrig)
{
	if(!IS_VALID(obj)){
		return false;
	}
	if(!HAS_OTRIGGER(obj, otrig)){
		return false;
	}

	if(IS_RUNNING_OTRIGGER(obj, otrig) && !IS_SET( obj->extra2_flags, OBJEXTRA2_MUDPROG_RECURSION_ALLOWED)){		
		// don't recurse to running a program with the same trigger on the same object
		return mp_object_action_prevented;
	}
	// mark the trigger type as being run for this object
	SET_RUNNING_OTRIGGER(obj,otrig);

	// find which program or programs should be triggered
	mp_object_action_prevented=false;
	mp_suppress_text_output=false;
	bool keep_looping=true;
    for( MUDPROG_TRIGGER_LIST *prg = obj->pIndexData->obj_triggers; prg && keep_looping; prg = prg->next ){
		if ( prg->trig_type == otrig )
		{
			char *p = prg->trig_phrase;
			char buf[MIL];

			switch (otrig){
				case OTRIG_GET_PRE:
				case OTRIG_GET_POST:
				case OTRIG_DROP_PRE:
				case OTRIG_DROP_POST:
				case OTRIG_LOOKAT_PRE:
				case OTRIG_LOOKAT_POST:
				case OTRIG_LOOKIN_PRE:
				case OTRIG_LOOKIN_POST:
				case OTRIG_PUT_PRE:
				case OTRIG_PUT_POST:
				case OTRIG_WEAR_PRE:
				case OTRIG_WEAR_MID:
				case OTRIG_WEAR_POST:
				case OTRIG_CONTAINER_GET_PRE:	
				case OTRIG_CONTAINER_GET_POST:
				case OTRIG_CONTAINER_PUTIN_PRE:
				case OTRIG_CONTAINER_PUTIN_POST:
				case OTRIG_REMOVE_PRE:
				case OTRIG_REMOVE_POST:
				{
					// argument if numeric is a percentage
					// if nonnumeric it is considered an exact name
					// Vnum argument
					if( is_number( p ) ){
						if ( atoi(p)>=number_percent() )
						{
							object_program_flow(prg, obj, ch, obj);
							keep_looping=false; // terminate the execution of the loop
						}
					}else{
						// exact player name list argument, e.g. 'bob fred'
						while( *p ){
							p = one_argument( p, buf );							
							if ( is_exact_name( buf, ch->name)){
								object_program_flow(prg, obj, ch, obj);
								keep_looping=false; // terminate the execution of the loop
								break;
							}
						}
					}
				}
				break;
			
			default:
				bugf("oprog_execute_if_appropriate(): Unrecognised trigger type %d on object %d", 
					otrig, obj->pIndexData->vnum);
				ch->printlnf("BUG: oprog_execute_if_appropriate(): Unrecognised trigger "
					"type %d on object %d... please report to the admin.", 
					otrig, obj->pIndexData->vnum);
				break;
			}
		}
	}

	// clear the trigger type as being run for this object
	REMOVE_RUNNING_OTRIGGER(obj,otrig);

	return mp_object_action_prevented;
}

/**************************************************************************/
/**************************************************************************/
