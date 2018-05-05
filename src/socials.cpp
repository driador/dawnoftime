/**************************************************************************/
// File: social.cpp - Kal's complete rewrite of the social code 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "socials.h"
#include "channels.h"

/**************************************************************************/
social_type *social_list=NULL;
/**************************************************************************/
void do_mpqueue(char_data *ch, char *argument);
/**************************************************************************/
// default constructor
social_type::social_type()
{
	for(int i=0; i<SOCIAL_ATMAX; i++){
		acts[i]=str_dup("");
	}
	position_flags= (1<<(POS_RESTING )) | (1<<(POS_SITTING )) 
		 | (1<<(POS_KNEELING)) | (1<<(POS_FIGHTING)) | (1<<(POS_STANDING));
	social_flags=	SOC_IN_OOC|SOC_IN_IC;
}
/**************************************************************************/
// default destructor
social_type::~social_type()
{
	for(int i=0; i<SOCIAL_ATMAX; i++){
		free_string(acts[i]);
	}
}
/**************************************************************************/
// return true if ran correctly, i.e. the target was found
bool social_type::process_social_execution(char_data *ch, char *argument, bool global)
{
	char arg[MIL];
	MOBtrigger=true;
	argument=one_argument(argument, arg);

	char_data *target;
	ACTTO_TYPE room_world;
	if(global){
		target=get_whovis_player_world(ch, arg);
		room_world=TO_WORLD;
	}else{
		target=get_char_room(ch, arg);
		room_world=TO_ROOM;
	}

	if(IS_NULLSTR(arg)){
		if(IS_NULLSTR(acts[SOCIAL_ATNOTARGET_MSG2SELF]) 
			&& IS_NULLSTR(acts[SOCIAL_ATNOTARGET_MSG2OTHERS])){
			ch->printlnf("The %s social requires more parameters.",	name);
			ch->printlnf("Type `=Csocshow %s`x to find out more about the social.",	name);
			MOBtrigger=false;
			return false;
		}
		act(acts[SOCIAL_ATNOTARGET_MSG2SELF],	ch, NULL, NULL, TO_CHAR);
		act(acts[SOCIAL_ATNOTARGET_MSG2OTHERS], ch, NULL, NULL, TO_ROOM);
		MOBtrigger=false;
		return false;
	}

	if(target){
		// target is self
		if(target==ch){
			if(IS_NULLSTR(acts[SOCIAL_ATSELF_MSG2SELF]) 
				|| IS_NULLSTR(acts[SOCIAL_ATSELF_MSG2OTHERS])){
				ch->printlnf("The %s social can't be directed at yourself.",	name);
				ch->printlnf("Type `=Csocshow %s`x to find out more about the social.",	name);
				MOBtrigger=false;
				return false;
			}			
			act(acts[SOCIAL_ATSELF_MSG2SELF],	ch, NULL, NULL, TO_CHAR);
			act(acts[SOCIAL_ATSELF_MSG2OTHERS], ch, NULL, NULL, room_world);
			MOBtrigger=false;
			return false;
		}

		if(IS_NULLSTR(acts[SOCIAL_ATTARGET_MSG2SELF]) 
			|| IS_NULLSTR(acts[SOCIAL_ATTARGET_MSG2TARGET])
			|| IS_NULLSTR(acts[SOCIAL_ATTARGET_MSG2OTHERS]))
		{
			ch->printlnf("The %s social can't be directed at others.",	name);
			ch->printlnf("Type `=Csocshow %s`x to find out more about the social.",	name);
			MOBtrigger=false;
			return false;
		}			
		// target is another
		act(acts[SOCIAL_ATTARGET_MSG2SELF],	ch, NULL, target, TO_CHAR);
		act(acts[SOCIAL_ATTARGET_MSG2TARGET], ch, NULL, target, TO_VICT);
		act(acts[SOCIAL_ATTARGET_MSG2OTHERS], ch, NULL, target, TO_NOTVICT);
		MOBtrigger=false;

		// check for a mob response
		if(	!IS_NULLSTR(acts[SOCIAL_ATTARGET_MOBTARGETRESPONSE])
			&&	!IS_NPC(ch) 
			&&	IS_NPC(target) 
			&&	!IS_AFFECTED(target, AFF_CHARM)
			&&	!IS_SET(target->act, ACT_NOAUTOSOCIAL)
			&&	!IS_CONTROLLED(target)
			&&	IS_AWAKE(target)
			&&	can_see(target, ch)
		  )
		{
			// Conditions were right, so target happened to be a mob that executes the response 
			char buf[MSL];
			sprintf(buf,"1 %s", acts[SOCIAL_ATTARGET_MOBTARGETRESPONSE]);
			char *command=str_dup(buf);
			command=string_replace_all(command,"$N", ch->name);
			// queue the response to be done in 1 second	
			do_mpqueue(target, command);
			free_string(command);
		}
		return true;
	}

	// someone specified not found though
	ch->printlnf("You can't find any '%s' to direct the social '%s' towards.",
		arg, name);

	return false;

}
/**************************************************************************/
void social_type::execute_social(char_data *ch, char *argument, bool global)
{
	RECORD_TO_REPLAYROOM=true;
	EXECUTING_SOCIAL=true; // used for players to turn off colour in socials
	process_social_execution(ch, argument, global);
	EXECUTING_SOCIAL=false;
	RECORD_TO_REPLAYROOM=false;
}
/**************************************************************************/
// find a social, based on the character using it
social_type *find_social(char_data * ch, char *social)
{
	social_type *s;

	for(s=social_list; s; s=s->next){
		if(IS_SET(s->social_flags, SOC_IMM_ONLY) && !IS_IMMORTAL(ch)){
			continue;
		}

		if(!str_prefix(social, s->name)){
			return s;
		}
	}
	return NULL;
};
/**************************************************************************/
// return true if a social was found and ran
bool check_social(char_data *ch,char * command, char * argument, bool global)
{
	social_type *soc=find_social(ch, command);

	if(soc){
		if(!global){
			// do position checks etc
			int pos_flag= 1<<ch->position;
			if(!IS_SET(soc->position_flags, pos_flag)){
				ch->printlnf("You can't use the '%s' social in your current position.", soc->name);
				return true;
			}

			// IC / OOC room checks
			if(IS_IC(ch) && !IS_SET(soc->social_flags, SOC_IN_IC)){
				ch->printlnf("You can't use the '%s' social in an IC room.", soc->name);
				return true;
			}
			if(IS_OOC(ch) && !IS_SET(soc->social_flags, SOC_IN_OOC)){
				ch->printlnf("You can't use the '%s' social in an OOC room.", soc->name);
				return true;
			}
		}

		soc->execute_social(ch, argument, global);
		return true; // social found and ran
	}
	return false; // social not found
}
/**************************************************************************/
// assume the name field in soc is all lowercase
void social_add_sorted_to_list(social_type *soc, bool replace) 
{
	if(!social_list){
		soc->next=NULL;
		social_list=soc;
		return;
	}

	social_type *node=social_list;
	social_type *prev=NULL;

	while(node && strcmp(soc->name, node->name)>0){
		prev=node;
		node=node->next;
	}

	if(node && (strcmp(soc->name, node->name)==0)){ // can't have duplicated entries
		if(replace){
			if(prev){ // replace node
				soc->next=node->next;
				delete prev->next;
				prev->next=soc;
			}else{ // replace first in the list
				assert(node==social_list);
				soc->next=social_list->next;
				delete social_list;
				social_list=soc;
			}
		}else{
			delete soc;
		}
		return;
	}
	
	if(prev){
		soc->next=prev->next;
		prev->next=soc;
	}else{ // add to be first in the list
		assert(node==social_list);
		soc->next=social_list;
		social_list=soc;
	}
}
/**************************************************************************/
// import socials from the old socials table
void do_social_import(char_data *ch, char *)
{
	social_type *soc;

	for(int i=0; !IS_NULLSTR(social_table[i].name); i++){
		soc=new social_type;
		soc->name=str_dup(lowercase(social_table[i].name));
		soc->acts[SOCIAL_ATNOTARGET_MSG2SELF		]=safe_str_dup(social_table[i].char_no_arg);
		soc->acts[SOCIAL_ATNOTARGET_MSG2OTHERS		]=safe_str_dup(social_table[i].others_no_arg);
		soc->acts[SOCIAL_ATSELF_MSG2SELF			]=safe_str_dup(social_table[i].char_auto);
		soc->acts[SOCIAL_ATSELF_MSG2OTHERS			]=safe_str_dup(social_table[i].others_auto);
		soc->acts[SOCIAL_ATTARGET_MSG2SELF			]=safe_str_dup(social_table[i].char_found);
		soc->acts[SOCIAL_ATTARGET_MSG2TARGET		]=safe_str_dup(social_table[i].vict_found);
		soc->acts[SOCIAL_ATTARGET_MSG2OTHERS		]=safe_str_dup(social_table[i].others_found);
		
		// add to the linked list		
		social_add_sorted_to_list(soc, false);
	}
	ch->wrapln("Socials imported from old social_table[]... if there were socials under"
		"the new system with the same name, they will still remain.");
	save_socials();
}
/**************************************************************************/
// display the list of socials
void do_socials(char_data *ch, char *)
{
	int col=0;
	char buf[MSL];

    BUFFER *output;
    output = new_buf();	

	for(social_type *soc=social_list; soc; soc=soc->next){
		if(IS_SET(soc->social_flags, SOC_IMM_ONLY) && !IS_IMMORTAL(ch)){
			continue;
		}
		sprintf(buf, "%-12s", soc->name);
		add_buf(output, buf);
		if(++col%6==0){
			sprintf(buf,"\r\n");
			add_buf(output, buf);
		}
	}
	if(col%6!=0){
		sprintf(buf,"\r\n");
		add_buf(output, buf);
	}

	sprintf(buf,"%d social%s total.\r\n", col, col==1?"":"s");
	add_buf(output, buf);

    ch->sendpage(buf_string(output));
    free_buf(output);
}
/**************************************************************************/
const struct flag_type social_flags[] =
{
    {   "ooc",			SOC_IN_OOC,		true    },
    {   "in_ooc",		SOC_IN_OOC,		true    },
    {   "ic",			SOC_IN_IC,		true    },
    {   "in_ic",		SOC_IN_IC,		true    },
    {   "imm_only",		SOC_IMM_ONLY,	true    },	
    {   NULL,			0,					0       }
};
/**************************************************************************/
// GIO STUFF BELOW TO SAVE/LOAD the socials
GIO_START(social_type)
GIO_STR(name)
GIO_WFLAG(social_flags, social_flags)
GIO_WFLAG(position_flags, position_flags)
GIO_STR_ARRAY(acts,SOCIAL_ATMAX)
GIO_FINISH
/**************************************************************************/
// save the list of socials
void save_socials()
{
	logf("===save_socials(): saving socials database to %s", SOCIAL_FILE);
	GIOSAVE_LIST(social_list, social_type, SOCIAL_FILE, true);
}
/**************************************************************************/
// load the list of socials
void load_socials()
{
	logf("===load_socials(): reading in socials database from %s", SOCIAL_FILE);
	GIOLOAD_LIST(social_list, social_type, SOCIAL_FILE);

	// patch up the NULL pointers for str_dup("");
	// One day might even sort the list alphabetically, but not today :)
	social_count=0;
	for(social_type *soc=social_list; soc; soc=soc->next){
		social_count++;
		for(int i=0; i<SOCIAL_ATMAX; i++){
			if(soc->acts[i]==NULL){
				soc->acts[i]=str_dup("");
			}
		}
	}
}

/**************************************************************************/
// Global Social Channel
bool social_global_broadcast( char_data *ch, char *command, char *argument )
{
	bool found = false;
	char arg[MIL];
	char buf[MIL];
	social_type *soc;
	char_data *victim;
	char_data *vic;
	connection_data *d;

	for (soc = social_list;soc;soc=soc->next)
	{
		if(IS_SET(soc->social_flags, SOC_IMM_ONLY) && !IS_IMMORTAL(ch) ){
			continue;
		}
		if(!str_prefix(command,soc->name)){
			found = true;
			break;
		}
	}

	if(!found){
 		return false;
	}

	one_argument( argument, arg );
	victim = NULL;

	if((victim = get_char_world( ch, arg )) != NULL){
		if(IS_NPC(victim)){
			victim = NULL;
		}
	}

	// Found the social - Display to Char
	// Send Msg to Char - No Target	
	if ( arg[0] == '\0' ){
		sprintf( buf, "`g[S] %s`x", soc->acts[SOCIAL_ATNOTARGET_MSG2SELF]);
		act_new( buf,  ch, NULL, victim, TO_CHAR,POS_SLEEPING );
	}
 	// Does the victim exist?
	else if ( victim == NULL ){
		strcpy( buf, "`g[S] They aren't here.`x" );
		act_new( buf,  ch, NULL, victim, TO_CHAR,POS_SLEEPING );
		return true;
 	// Send Msg to Char - Victim is Char
 	}else if ( victim == ch ){
 		strcpy( buf, "`g[S] " );
 		strcat( buf, soc->acts[SOCIAL_ATSELF_MSG2SELF] );
 		strcat( buf, "`x");
 		act_new( buf,  ch, NULL, victim, TO_CHAR,POS_SLEEPING );
 	// Send Msg to Char - Victim is Exists and isn't Char
 	}else{
 		strcpy( buf, "`g[S] " );
		strcat( buf, soc->acts[SOCIAL_ATTARGET_MSG2SELF] );
 		strcat( buf, "`x");
 		act_new( buf,  ch, NULL, victim, TO_CHAR,POS_SLEEPING );
 	}
 
 	// Found the social, display to everyone else
 	for ( d = connection_list; d != NULL; d = d->next )
 	{
	 	vic = d->original ? d->original : d->character;
 		if ( d->connected_state == CON_PLAYING &&
 			d->character != ch &&
			!HAS_CHANNELOFF(vic, CHANNEL_QUIET)
			&& !IS_SET(ch->comm, COMM_GLOBAL_SOCIAL_OFF)) 
		{
 			if ( arg[0] == '\0' ){
 				strcpy( buf, "`g[S] " );
 				strcat( buf,soc->acts[SOCIAL_ATNOTARGET_MSG2OTHERS]);
 				strcat( buf, "`x");
 				act_new( buf,  ch, NULL, vic, TO_VICT,POS_SLEEPING);
 			}else if (victim == NULL){
 			// Don't do anything
 			}else if ( victim == ch ){
 				strcpy( buf, "`g[S] " );
 				strcat( buf, soc->acts[SOCIAL_ATSELF_MSG2OTHERS]);
 				strcat( buf, "`x");
 				act_new( buf,  ch, vic, victim, TO_WORLD,POS_SLEEPING);
			}else if ( vic == victim ){
 				strcpy( buf, "`g[S] " );
 				strcat( buf, soc->acts[SOCIAL_ATTARGET_MSG2TARGET]);
 				strcat( buf, "`x");
 				act_new( buf,  ch, NULL, victim,TO_VICT,POS_SLEEPING );
 			}else{
 				strcpy( buf, "`g[S] " );
 				strcat( buf, soc->acts[SOCIAL_ATTARGET_MSG2OTHERS]);
 				strcat( buf, "`x");
 				act_new( buf,  ch, vic, victim, TO_WORLD,POS_SLEEPING);
 			}
	 	}
 	} // For
  	return true;
}

/**************************************************************************/
void do_globalsocial( char_data *ch, char *argument )
{
	char arg[MIL];
	char *arg2;
	connection_data *d;
	bool emote=false;

	if(IS_NULLSTR(argument))
	{
		ch->titlebar("GLOBAL SOCIAL");		
		ch->println("`=lsyntax:`x gs <normal use of a social>");
		ch->println("`=lsyntax:`x gs emote <normal use of standard emotes>");
		ch->println("`=lsyntax:`x gs ,<normal use of standard emotes>");
		ch->println("`=lsyntax:`x gs <social text>");
		ch->println("`=lsyntax:`x gs off  - turn off global socials");
		ch->println("e.g. `=Cgs smile kal`x");
		ch->titlebar("");
		return;
	}
	// social sent, turn Social on if it isn't already 
	if(HAS_CHANNELOFF(ch, CHANNEL_QUIET))
	{
		ch->println("You must turn off quiet mode first.");
		return;
	}
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
		ch->println("The gods have revoked your channel privilidges.");
		return;
    }

	if(ch->in_room && IS_SET(ch->in_room->room_flags,ROOM_NOCHANNELS)){
		if(IS_IMMORTAL(ch)){
			ch->println( "`Snote: mortals can't use any channels while in this room.`x" );
		}else{
			ch->println( "You can't use any channels while in this room." );
			return;
		}
	}

	if(!str_cmp(argument,"off")){
		SET_BIT(ch->comm, COMM_GLOBAL_SOCIAL_OFF);
		ch->println("Global Socials turned off.");
		return;
	}

	// left trim the string
	argument=ltrim_string(argument);

	// search for a , indicating it is an emote
	if(argument[0]==',' && !is_space(argument[1])){
		strcpy(arg, ",");
		argument++;
		arg2=argument;
	}else{
		arg2 = one_argument( argument, arg );
	}

	if(IS_SET(ch->comm, COMM_GLOBAL_SOCIAL_OFF)){
		REMOVE_BIT(ch->comm, COMM_GLOBAL_SOCIAL_OFF);
	}

    if(social_global_broadcast(ch,arg,arg2)){
		return;
	}

	if(!IS_NULLSTR(arg2) && (!str_cmp(arg, "emote") || !str_cmp(arg, ","))){
		emote=true;
		ch->printlnf( "`g[E] %s %s`x", ch->name, arg2);
	}else{
		ch->printlnf( "`gYou socialize '%s`g'`x", argument);
	}
	
	for ( d = connection_list; d; d = d->next )
	{
		char_data *victim;
		victim = d->original ? d->original : d->character;
		if (   d->connected_state == CON_PLAYING
			&& d->character != ch
			&& !HAS_CHANNELOFF(ch, CHANNEL_QUIET)
			&& !IS_SET(ch->comm, COMM_GLOBAL_SOCIAL_OFF))
		{
			if(emote){
				act_new("`g[E] $n $t`x", ch, arg2, d->character,TO_VICT,POS_SLEEPING);
			}else{
				act_new("`g$n socializes '$t`g'`x", ch, argument,d->character,TO_VICT,POS_SLEEPING);
			}
		}
	}
}
 
/**************************************************************************/
// Kal, Dec 2001 - rewrite of code from storm
void do_pose(char_data *ch, char *)
{
	int i;
	int cl;
	
	if ( IS_NPC(ch) )
	{
		if( IS_SET(ch->act,ACT_CLERIC) )
			cl = CLASS_CLERIC;
		else if( IS_SET(ch->act,ACT_MAGE) )
			cl = CLASS_MAGE;
		else if( IS_SET(ch->act,ACT_THIEF) )
			cl = CLASS_THIEF;
		else
			cl = CLASS_WARRIOR; // NPCs pose as warriors if no other act flags are set
	} else {
		cl= ch->clss;
	}

	int p=number_range(0, UMIN(ch->level, MAX_LEVEL));
	for(i=p; i>=0; i--){
		if(!IS_NULLSTR(class_table[cl].pose_self[i])
			&& !IS_NULLSTR(class_table[cl].pose_others[i]))
		{
			act( class_table[cl].pose_self[i], ch, NULL, NULL, TO_CHAR );
			act( class_table[cl].pose_others[i], ch, NULL, NULL, TO_ROOM );
			return;
		}
	}
	ch->printlnf("Sorry, there are no poses configured for the %s class.", 
		class_table[cl].name);
}
/**************************************************************************/
/**************************************************************************/

