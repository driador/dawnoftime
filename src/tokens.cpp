/**************************************************************************/
// tokens.cpp - token system written by Kalahn & Kerenos
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
 
#include "include.h"

void ostat_show_to_char( char_data *ch, OBJ_DATA *obj);
/**************************************************************************/
void do_tgive( char_data *ch, char *argument )
{
	char arg1[MIL];
	char arg2[MIL];
	char_data *victim;
	OBJ_DATA  *token;
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	
	if ( IS_NULLSTR( arg1) || IS_NULLSTR( arg2 )) {
		ch->println( "Give token to whom?" );
		ch->println( " (tgive <tokenumber> <person>)" );
		if(IS_NPC(ch)){
			mpbugf("tgive: 2 arguments required - arg1=token, arg2=to who to give it to"); 
		}
		return;
	}

    if (( token = get_obj_token( ch, arg1 )) == NULL ) {
        ch->println( "You do not have that token." );
		if(IS_NPC(ch)){
			mpbugf("tgive: arg1 '%s' token couldn't be found on mob!", arg1); 
		}
        return;
    }
	
    if ( token->item_type != ITEM_TOKEN ) {
        ch->println( "That is not a token." );
		if(IS_NPC(ch)){
			mpbugf("tgive: arg1 '%s' object wasn't a token!", arg1); 
		}
        return;
    }
	
    if (( victim = get_char_room( ch, arg2 )) == NULL ) {
        ch->println( "They aren't here." );
		if(IS_NPC(ch)){
			mpbugf("tgive: arg2 '%s' couldn't be found in the room!", arg2); 
		}
        return;
    }

	if(ch==victim){
        ch->println( "No point in giving the tokens to yourself." );
		if(IS_NPC(ch)){
			mpbugf("tgive: trying to give tokens to itself arg2 '%s'.", arg2); 
		}
        return;
	}

    obj_from_char( token );
    obj_to_char( token, victim );
	
    act( "You SILENTLY give '$p' to $N.", ch, token, victim, TO_CHAR );
}

/**************************************************************************/
void do_tremove( char_data *ch, char *argument )
{
	char arg1[MIL];
	char arg2[MIL];
//	char buf[MSL];
	char_data *victim;
	OBJ_DATA  *token;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( IS_NULLSTR( arg1) || IS_NULLSTR( arg2 )) {
		ch->println( "Take token from whom?" );
		ch->println( " (tremove <tokennumber> <person>)" );
		if(IS_NPC(ch)){
			mpbugf("tremove: 2 arguments required - arg1=token number, arg2=to who to remove it from"); 
		}
		return;
	}

	if (( victim = get_char_room( ch, arg2 )) == NULL ) {
        ch->println( "They aren't here." );
		if(IS_NPC(ch)){
			mpbugf("tremove: arg2 '%s' couldn't be found in the room.", arg2); 
		}
		return;
	}

	if (( token = get_obj_token( victim, arg1 )) == NULL ) {
		ch->println( "They do not have that token." );
		if(IS_NPC(ch)){
			mpbugf("tremove: arg2 '%s' didn't have token arg1 '%s'.", arg2, arg1); 
		}
		return;
	}

	if ( token->item_type != ITEM_TOKEN ) {
		ch->println( "That is not a token." );
		if(IS_NPC(ch)){
			mpbugf("tremove: arg2 '%s' had arg1 '%s', but it wasn't a token.", arg2, arg1); 
		}
		return;
	}

	obj_from_char( token );
	obj_to_char( token, ch );

	act( "You SILENTLY take $p to $N.", ch, token, victim, TO_CHAR );
}
/**************************************************************************/
void do_twhere(char_data *ch, char *argument)
{
    char buf[MIL];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;

	bool display_stat = false;    
	int display_number=0;
    OBJ_DATA *display_obj= NULL;

	int number = 0, max_found, vnum;
	
    found = false;
    number = 0;
    max_found = 200;

    if ( !IS_NULLSTR(argument))
    {
		if ( !is_number( argument ) ){
			ch->println( "Syntax:  twhere <item number in list>" );
			ch->println( " twhere with no parameters to see list of all token objects in the game." );
			return;
		}


		display_stat =true;
		display_number = atoi( argument );
		if (display_number <0){
			ch->println( "Value must be greater than 0." );
			return;
		}
	}

    buffer = new_buf();
	
	sprintf( buf,"`?%s`x", format_titlebar("TOKEN OBJECTS WHERE"));
	add_buf(buffer,buf);

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || ( obj->item_type != ITEM_TOKEN )
			||   ch->level < obj->level)
            continue;

        found = true;
        number++;

		if (display_stat){
			if (number==display_number){
				display_obj=obj;
			}
			continue;
		}

        for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj );
		
		if (obj->pIndexData) {
			vnum=obj->pIndexData->vnum;
		}
		else{
			vnum=-1;
		}

		if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
			&&   in_obj->carried_by->in_room != NULL)
		{
			sprintf( buf, "`G%3d)`x %s [%d] is \r\n    carried by %s [Room %d]`x\r\n",
				number, obj->short_descr, vnum,	PERS(in_obj->carried_by, ch),
				in_obj->carried_by->in_room->vnum );
		}
		else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
		{
			sprintf( buf, "`Y%3d)`x %s [%d] is \r\n    in %s [Room %d]`x\r\n",
				number, obj->short_descr, vnum, 
				in_obj->in_room->name, in_obj->in_room->vnum);
		}
		else
			sprintf( buf, "`R%3d)`x %s [%d] is somewhere`x\r\n",
				number, obj->short_descr, vnum);
		
        buf[0] = UPPER(buf[0]);
		
		if (!add_buf(buffer,buf))
		{
			ch->println( "Too many objects... buffer overflow." );
            break;
		} 	

        if (number >= max_found)
		{
			add_buf(buffer,"Not all token objects are listed - due to list limit.\r\n");
            break;
		}
    }
	
    if ( !found )
        ch->println( "No token objects are currently in the game." );
    else{
        ch->sendpage(buf_string(buffer));

		// rwhere searching code 
		if (display_stat){
			if (display_obj){
				ostat_show_to_char(ch, display_obj);
			}else{
				ch->printf("Didn't find token object %d in "
					"the twhere list.\r\n", display_number);
			}
		}
	}	
    free_buf(buffer);
}
/**************************************************************************/
void do_ttimer( char_data *ch, char *argument )
{
	char		arg1[MIL];
	char		arg2[MIL];
	char		arg3[MIL];
	int			value;
	char_data	*victim;
	obj_data	*token;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if ( IS_NULLSTR( arg1) || IS_NULLSTR( arg2 ) || IS_NULLSTR( arg3 )) {
		ch->println( "Syntax: ttimer <tokenname> playername ticks" );
		if(IS_NPC(ch)){
			mpbugf("ttimer: 3 arguments required - arg1=token, arg2=playername, arg3=number of ticks"); 
		}
		return;
	}

	if (( victim = get_char_world( ch, arg2 )) == NULL ) {
        ch->printlnf("'%s' is not logged on.", arg2);
		if(IS_NPC(ch)){
			mpbugf("ttimer: player/mob arg2 '%s' doesn't appear to be logged into the game.", arg2); 
		}
		return;
	}

	if (( token = get_obj_token( victim, arg1 )) == NULL ) {
		ch->printlnf("They do not have token '%s'.", arg1);
		if(IS_NPC(ch)){
			mpbugf("ttimer: player/mob arg2 '%s' doesn't have token arg2 '%s'.", arg2, arg1); 
		}
		return;
	}

	if ( token->item_type != ITEM_TOKEN ) {
		ch->printlnf("'%s' is not a token.", arg1);
		if(IS_NPC(ch)){
			mpbugf("ttimer: item arg1 '%s' carried by player/mob arg2 '%s' isn't a token.", arg1, arg2); 
		}
		return;
	}

	value = atoi( arg3 );

	if ( !is_number( arg3 )) {
		ch->println( "Ticks field must be a numerical value." );
		ch->println( "Syntax: ttimer player token ticks" );
		if(IS_NPC(ch)){
			mpbugf("ttimer: value arg3 '%s' is the number of ticks, therefore must be numeric.", arg3); 
		}
		return;
	}

	token->timer = value;
	ch->printf( "Done, timer set to %d on token %d.\r\n", value, 
		token->pIndexData?0:token->pIndexData->vnum);
}
/**************************************************************************/
void do_tjunk( char_data *ch, char *argument )
{
	char arg1[MIL];
	char arg2[MIL];
	char_data *victim;
	OBJ_DATA  *token;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( IS_NULLSTR( arg1) || IS_NULLSTR( arg2 )) {
		ch->println( "Junk token from whom?" );
		ch->println( " (tjunk <tokennumber> <person>)" );
		ch->println( " (tjunk all.<tokennumber> <person>)" );
		if(IS_NPC(ch)){
			mpbugf("tjunk: 2 arguments required - arg1=token number, arg2=to who to remove it from, use 'self' for mob on itself"); 
			mpbugf( " (tjunk <tokennumber> <person>)");
			mpbugf( " (tjunk all.<tokennumber> <person>)");
		}
		return;
	}

	if (( victim = get_char_room( ch, arg2 )) == NULL ) {
        ch->println( "They aren't here." );
		if(IS_NPC(ch)){
			mpbugf("tjunk: arg2 '%s' couldn't be found in the room.", arg2); 
		}
		return;
	}

	if(str_prefix( "all.", arg1 ) )
	{
		if (( token = get_obj_token( victim, arg1 )) == NULL ) {
			ch->printlnf("'%s' does not have token '%s'.", PERS(victim, ch), arg1);
			if(IS_NPC(ch)){
				mpbugf("tjunk: arg2 '%s' didn't have token arg1 '%s'.", arg2, arg1); 
			}
			return;
		}
	}else{
		char *pallof=&arg1[4];
		int count=0;

		do{
			token = get_obj_token( victim, pallof);
			if(token){
				if ( token->item_type != ITEM_TOKEN ) {
					ch->println( "That is not a token." );
					if(IS_NPC(ch)){
						mpbugf("tjunk: arg2 '%s' had arg1 '%s', but it wasn't a token.", arg2, arg1); 
					}
					continue;
				}
				count++;
				obj_from_char( token );
				extract_obj( token );
			}
		}while(token);

		if(count==0){
			ch->printlnf("'%s' does not have any tokens '%s'.", PERS(victim, ch), pallof);
			return;
		}else{
			ch->printlnf("You SILENTLY junked %d '%s' from %s.", 
				count, arg1, PERS(victim, ch));
			return;
		}
	}

	if ( token->item_type != ITEM_TOKEN ) {
		ch->println( "That is not a token." );
		if(IS_NPC(ch)){
			mpbugf("tjunk: arg2 '%s' had arg1 '%s', but it wasn't a token.", arg2, arg1); 
		}
		return;
	}
	act( "You SILENTLY junk $p from $N.", ch, token, victim, TO_CHAR );

	obj_from_char( token );
	extract_obj( token );

}
/**************************************************************************/
/**************************************************************************/


