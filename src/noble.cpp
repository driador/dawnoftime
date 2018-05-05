/**************************************************************************/
// noble.cpp - Noble code, originally from Oblivion, mostly rewritten.
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
#include "offmoot.h"

/**************************************************************************/
void do_noble(char_data *ch, char * argument)
{
    char arg1[MIL];
    char_data *victim;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0')
    {
        ch->println("Syntax: noble <char>");
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        ch->println("They aren't playing.");
        return;
    }

    if (IS_NPC(victim))
    {
        ch->println("You can't noble a NPC!");       
        return;
    }

    if(IS_TRUSTED(victim,LEVEL_IMMORTAL))
		victim->pcdata->diplomacy=50;
    else
        victim->pcdata->diplomacy=10;

    ch->printlnf("%s is now a noble. (diplomacy = %d)",
		victim->name, victim->pcdata->diplomacy);
    victim->println("You are now a noble.");

    victim->pcdata->dip_points+=(victim->pcdata->diplomacy*5);
    save_char_obj(victim);
    return;
}

/**************************************************************************/
void do_moothelp(char_data *ch, char *)
{
	ch->println("Anything after the | is optional arguments.");
	ch->println("moot <character> reward <amount>");
	ch->println("moot <character> induct");
	ch->println("moot <character> outcast");
	return;
}

/**************************************************************************/
void bring_moot_forward(void)
{
    connection_data *d;

    for ( d = connection_list; d; d = d->next )
    {
        if ( d->connected_state == CON_PLAYING ){
             if(CH(d)->pcdata->diplomacy){
                 CH(d)->printf("`YMoot brought forward because those "
					 "that were quick enough or cared enough have voted.`x\r\n");
             }
   
        }
    }
    moot->pulse=1; // bring moot to update now 

}
/**************************************************************************/
void do_autovote(char_data *ch, char * argument)
{
    char arg[MIL];

	if (!IS_NOBLE(ch))
	{
		ch->println("Autovote is a noble only command... You are not a noble sorry.");
		return;
	}

	one_argument( argument, arg );
	if(IS_NULLSTR(arg) || !is_number(arg)){
		if(!codehelp(ch, "do_autovote_invalid_argument", false)){
			ch->println("syntax: autovote <value_of_moot_you_will_automatically_vote_on_upto>");
			ch->wrapln("if a moot is below this value the moot "
				"will not wait for you for longer than 15 seconds vote before "
				"attempting to resolve the voting on the moot`1"
				"Note: This only applies to positive moots of type reward.");
		}
		return;
	}

	int val=atoi(arg);

	if(val<0 || val>1000){
		ch->println("Autovote value must be between 0 and 1000.\r\n");
		return;
	}

	ch->wraplnf("The mooting system will no longer wait longer "
		"than 15 seconds for you to vote on postive moots of type reward "
		"that are worth less than %dxp in value.", val);
	TRUE_CH(ch)->pcdata->autovote=val;

};
/**************************************************************************/
void do_moot(char_data *ch, char * argument)
{
	char arg1[MIL],arg2[MIL],arg3[MIL];
	char_data *victim;
    connection_data *d;
	int cost=0;
	int scope=0;
	int type=0;
    int nobles_online;

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master ){
			ch->master->println( "Not going to happen.");
		}
		return;
	}

	if (!IS_NOBLE(ch))
	{
		ch->println("You are not a noble.");
        return;
	}

	if(moot->moot_type!=0)
	{
		ch->println("A moot is already in progress.");
        return;
	}

	argument = one_argument( argument, arg1 );
	if(IS_NULLSTR(arg1))
	{
		do_moothelp(ch,"auto");
        return;
	}

	if ( ( victim = get_whovis_player_world( ch, arg1 ) ) == NULL )
	{
		ch->println("They aren't here.");
		return;
	}

	if(TRUE_CH(ch)==victim)
	{
        ch->println("You cannot call moots on your own behalf.");
        return;
	}

	if(IS_NPC(victim))
	{ 
		ch->println("Only on PC's.");
		return;
	}

	argument = one_argument( argument, arg2 );


    if ( !str_prefix( arg2, "reward" ) )
	{
		argument = one_argument( argument, arg3 );
		if(is_number(arg3))
        {
            scope=atoi(arg3);
        }
		else
		{
			do_moothelp(ch,"auto");
			return;
		}
		
		if(scope<0 && TRUE_CH(ch)->pcdata->dip_points<12){
			ch->println("You must have a diplomacy higher than 12 to start -ve moots.");
			return;
		}

		cost=cost+abs(scope)/10;
		type=1;
	}

    if ( !str_prefix( arg2, "induct" ) )
	{
		if(victim->pcdata->diplomacy)
		{
		 ch->println("They are already a noble.");
		 return;
		}	
		cost=2000;
		type=2;
	}
    if ( !str_prefix( arg2, "outcast" ) )
	{
		if(!IS_NOBLE(victim))
        {
			ch->println("They are not a noble.");
			return;
        }
               
		cost=4000;
		type=3;
	}

    if(!type)
	{
		ch->println("That is not a valid moot type.");
		return;
	}

	if(TRUE_CH(ch)->pcdata->dip_points < cost)
	{
		ch->println("You have not enough diplomacy points to call a moot.");
		return;
	}

	switch(type)
	{
	default: break;
	case 1: victim->printlnf("A moot has been called on your behalf for %+d.", scope);	break;
	case 2: victim->println("The nobles are attempting to recruit you!!!");				break;
	case 3: victim->println("Your fellow nobles are trying to outcast you!");			break;
	case 4: victim->println("A moot has been called to allow you to progress.");		break;
	}

	TRUE_CH(ch)->pcdata->dip_points-=cost;

	moot->called_by=TRUE_CH(ch);
	moot->moot_victim=TRUE_CH(victim);
	if(victim){
		replace_string(moot->moot_victim_name,victim->name);
	}else{
		replace_string(moot->moot_victim_name,"(unknown - bug?!?)");
	}
	moot->moot_type=type;
	moot->scope=scope;
	moot->votes_for=0;
	moot->votes_against=0;
	moot->pulse=PULSE_MOOT;

	broadcast_moot();

    // fast forward moot if only 1 noble online
    nobles_online=0;
    for ( d = connection_list; d; d = d->next )
	{
        if ( d->connected_state == CON_PLAYING )
        {
             if(IS_NOBLE(CH(d)))
             {
                nobles_online++;
    
             }
    
        }
    }
    if (nobles_online==1 && type==1 && scope>0)
    {
        moot->pulse=1; // bring moot to update now 

        bring_moot_forward();
    }
	return;
}
/**************************************************************************/
//This can be changed to do_moot and the above function can be deleted after
//specific regulations regarding mortal nobles and offling mooting can be made
void do_offmoot(char_data *ch, char * argument)
{
	char arg1[MIL],arg2[MIL], arg3[MIL];
	char_data *victim;
    connection_data *d;
	int cost=0;
	int scope=0;
	int type=0;
    int nobles_online;
	
	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED )){
		if ( ch->master ){
			ch->master->println( "Not going to happen.");
		}
		return;
	}
	
	if (!IS_NOBLE(ch)){
		ch->println("You are not a noble.");
        return;
	}
	
	if(moot->moot_type!=0){
		ch->println("A moot is already in progress.");
        return;
	}
	
	argument = one_argument( argument, arg1 );
	if(IS_NULLSTR(arg1)){
		do_moothelp(ch,"auto");
        return;
	}
	
	victim = get_whovis_player_world( ch, arg1 );
	
	if(TRUE_CH(ch)==victim){
        ch->println("You cannot call moots on your own behalf.");
        return;
	}
	
	if(victim!=NULL && IS_NPC(victim)){ 
		ch->println("Only on PC's.");
		return;
	}
	
	if(victim==NULL && find_pfiletype(arg1)==PFILE_NONE){
		ch->printlnf("Player %s does not exist in Dawn.", arg1);
		return;
	}
	
	argument = one_argument( argument, arg2 );
	
    if ( !str_prefix( arg2, "reward" ) )
	{
		
		argument = one_argument( argument, arg3 );
		if(is_number(arg3))
        {
            scope=atoi(arg3);
        }
		else
		{
			do_moothelp(ch,"auto");
			return;
		}
		
		if(scope<0 && TRUE_CH(ch)->pcdata->dip_points<12){
			ch->println("You must have a diplomacy higher than 12 to start -ve moots.");
			return;
		}
		
		cost=cost+abs(scope)/10;
		type=1;
	}
	
    if ( !str_prefix( arg2, "induct" ) )
	{
		if(victim==NULL){
			ch->println("This type of moot can't be done when the player is offline.");
			replace_string(moot->moot_victim_name,"");
			return;
		}
		if(victim->pcdata->diplomacy)
		{
			ch->println("They are already a noble.");
			return;
		}	
		cost=2000;
		type=2;
	}
    if ( !str_prefix( arg2, "outcast" ) )
	{
		if(victim==NULL){
			ch->println("This type of moot can't be done when the player is offline.");
			replace_string(moot->moot_victim_name,"");
			return;
		}
		if(!IS_NOBLE(victim))
        {
			ch->println("They are not a noble.");
			return;
        }
		
		cost=4000;
		type=3;
	}
	
    if(!type)
	{
		ch->println("That is not a valid moot type.");
		return;
	}
	
	if(TRUE_CH(ch)->pcdata->dip_points < cost)
	{
		ch->println("You have not enough diplomacy points to call a moot.");
		return;
	}
	
	
	switch(type)
	{
	default: break;
	case 1:
		if(victim!=NULL)
			victim->printlnf("A moot has been called on your behalf for %+d.", scope);
		break;
	case 2:
		victim->println("The nobles are attempting to recruit you!!!");
		break;
	case 3: victim->println("Your fellow nobles are trying to outcast you!");
		break;
	case 4: victim->println("A moot has been called to allow you to progress.");
		break;
	}
	
	TRUE_CH(ch)->pcdata->dip_points-=cost;
	
	moot->called_by=TRUE_CH(ch);
	if(victim!=NULL){
		moot->moot_victim=TRUE_CH(victim);
	}
	
	replace_string(moot->moot_victim_name,arg1);
	moot->moot_type=type;
	moot->scope=scope;
	moot->votes_for=0;
	moot->votes_against=0;
	moot->pulse=PULSE_MOOT;
	
	broadcast_moot();
	
    // fast forward moot if only 1 noble online
    nobles_online=0;
    for ( d = connection_list; d; d = d->next )
	{
        if ( d->connected_state == CON_PLAYING )
        {
			if(IS_NOBLE(CH(d)))
			{
                nobles_online++;
				
			}
			
        }
    }
    if (nobles_online==1 && type==1 && scope>0)
    {
        moot->pulse=1; // bring moot to update now 
		
        bring_moot_forward();
    }
	return;
}
/**************************************************************************/
void broadcast_moot(void)
{
	connection_data *d;
	char mt_type[MSL];

	switch (moot->moot_type)
	{
	default: sprintf(mt_type,"none"); break;
	case 1: sprintf(mt_type,"reward"); break;
	case 2: sprintf(mt_type,"induction"); break;
	case 3: sprintf(mt_type,"outcasting"); break;
	case 4: sprintf(mt_type,"progression"); break ;
	}
	
	for ( d = connection_list; d != NULL; d = d->next )
	{
		if ( d->connected_state == CON_PLAYING && IS_NOBLE(CH(d)))
		{
			d->character->printf(
				"`YMoot in progress of type %s called against %s "
				"of value:%d will be resolved in %d seconds.`x\r\n",
				mt_type,
				capitalize(moot->moot_victim_name),
				moot->scope,
				moot->pulse/PULSE_PER_SECOND);
			if(CH(d)->pcdata->votes_cast==0 && moot->pulse!=PULSE_MOOT){
				if( (CH(d)->idle<=5) 
					&& !IS_SET(CH(d)->comm,COMM_AFK) 
					&& CH(d)->pcdata->votes_cast==0
					&& moot->scope>CH(d)->pcdata->autovote)
				{
					d->character->printf("         `Y# # #   V O T E   P L E A S E !   # # #\r\n");
				}
			}
		}
	}

	return;
}

/**************************************************************************/
void resolve_moot(void)
{
	bool passed=false;
	char buf[MSL];
	char_data *ch;
	connection_data *d;


  bug("resolving");

	if (moot->moot_type>0)
	{

		if(moot->moot_type==1)
		{
		  if(moot->votes_for>moot->votes_against || (moot->votes_for==0 && moot->votes_against==0))
      {
		    passed=true;
        if(moot->moot_victim==NULL){
          //save moot for later...
          queue_moot(moot->moot_victim_name, moot->scope);
        }else{
          if(IS_NPC(moot->moot_victim)) return;
		        gain_exp(moot->moot_victim, moot->scope);

			    // HEROPXP SYSTEM
			    if ( IS_HERO( moot->moot_victim ))
			      do_heroxp( moot->moot_victim, moot->scope );

          moot->moot_victim->pcdata->rp_points+=moot->scope;

          //IF the caller is the same as the victim then it was an
          //offline moot and we can end after the award
          if(!strcmp(moot->called_by->name, moot->moot_victim->name)){
	          for ( d = connection_list; d != NULL; d = d->next )
	          {
		          if ( d->connected_state == CON_PLAYING && IS_NOBLE(CH(d)))
		          {
                d->character->printf("`#`Y%s has been mooted %d upon logging in.\r\n`^",
                  moot->moot_victim->name, moot->scope);
              }
            }
            moot->moot_victim->printf("`#`YA moot was passed on your behalf for %d exp.\r\n`^",
              moot->scope);
            return;
          }
        }
      }
			sprintf(buf, "`YA moot involving %s for %d xp just %s.`x\r\n",
			  moot->moot_victim_name,
			  moot->scope,
			  passed ? "passed" : "failed"
			  );
		}
		else if(moot->moot_type==2)
		{
		  if(100*moot->votes_for/((moot->votes_for+moot->votes_against)+1)>70)
		  {
			  passed=true;
			  moot->moot_victim->pcdata->diplomacy=10;
			  moot->moot_victim->pcdata->dip_points=50;
			  moot->called_by->pcdata->diplomacy+=2;
		  }
		  // do an autonote
		  sprintf(buf, "`BA moot called by %s to induct %s as a new noble has %s.`x\r\n",
			  moot->called_by->name,
				moot->moot_victim->name,
			  passed ? "succeeded" : "failed");
			autonote(NOTE_SNOTE, "resolve_moot()", "Noble induction", "admin", buf, true);

			sprintf(buf, "A moot to induct a new noble has %s.\r\n",
			 passed ? "succeeded" : "failed");
		}
		else if(moot->moot_type==3)
		{
		  if(100*moot->votes_for/((moot->votes_for+moot->votes_against)+1)>80 &&
					  moot->votes_for>5000)
		  {
			  passed=true;
			  moot->moot_victim->pcdata->diplomacy=0;
			  moot->called_by->pcdata->diplomacy+=5;
		  }
		  else
		  {
			  moot->called_by->pcdata->diplomacy=
				  moot->called_by->pcdata->diplomacy/2-1;
		  }
		  sprintf(buf, "A moot to outcast a noble has %s.\r\n",
			  passed ? "succeeded" : "failed");
		  if(moot->called_by->pcdata->diplomacy<0)
			  moot->called_by->pcdata->diplomacy=0;
		}
		else if(moot->moot_type==4)
		{
		  if(100*moot->votes_for/((moot->votes_for+moot->votes_against)+1)>49)
		  {
			  passed=true;
			  SET_BIT(moot->moot_victim->act, PLR_CAN_ADVANCE);
			  save_char_obj( moot->moot_victim );
		  }
		  sprintf(buf, "`GA moot to permit someone to gain has %s.`x\r\n", 
			  passed ? "succeeded" : "failed");
		}

		moot->moot_victim->print(buf);

		for ( d = connection_list; d; d = d->next )
		{
		  if ( d->connected_state == CON_PLAYING && IS_NOBLE(CH(d)))
		  {
			  ch=CH(d);
			  if (ch!= moot->moot_victim){
				  d->character->print(buf);
			  }
				  
			  if( (TRUE_CH(ch)->pcdata->in_favor==1 && passed) ||
				  (TRUE_CH(ch)->pcdata->in_favor==-1 && !passed))
			  {
				  if(number_range(1,100)<(TRUE_CH(ch)->pcdata->votes_cast*50/((
					  moot->votes_for+moot->votes_against)+1))
					  &&number_range(1,20)<TRUE_CH(ch)->pcdata->votes_cast)
					  TRUE_CH(ch)->pcdata->diplomacy++;
				  if(TRUE_CH(ch)->pcdata->diplomacy>50)
					  TRUE_CH(ch)->pcdata->diplomacy=50;
			  }
			  else
			  {
				  if(number_range(1,100)<(TRUE_CH(ch)->pcdata->votes_cast*100/((
					  moot->votes_for+moot->votes_against)+1)))
					  TRUE_CH(ch)->pcdata->diplomacy--;
			  }
			  TRUE_CH(ch)->pcdata->in_favor=0;
			  TRUE_CH(ch)->pcdata->votes_cast=0;
      }
    }
  }


	moot->called_by=NULL;
	moot->moot_victim=NULL;
	replace_string(moot->moot_victim_name,"");
	moot->moot_type=0;
	moot->scope=0;
	moot->votes_for=0;
	moot->votes_against=0;
    moot->number_of_votes=0;
	return;
}

/**************************************************************************/
void moot_check_bring_forward()
{
    connection_data *d;
    // fast forward moots only if positive of type reward and more than
	// 15 seconds have past since the moot started
	int nobles_still_to_vote=0;
	if(moot->moot_type==1 && moot->scope>0 && moot->pulse<(PULSE_MOOT-15))
	{
		for ( d = connection_list; d; d = d->next )
		{
			if ( d->connected_state == CON_PLAYING )
			{
				if( CH(d)->pcdata->diplomacy 
					&& (CH(d)->idle<=5) 
					&& !IS_SET(CH(d)->comm,COMM_AFK) 
					&& CH(d)->pcdata->votes_cast==0)
				{
					if(moot->scope>CH(d)->pcdata->autovote)
					{
						nobles_still_to_vote++;							
					}
				}
			}
		}
		if (nobles_still_to_vote==0)
		{
			bring_moot_forward();
		}
	}
}
/**************************************************************************/
void do_vote(char_data *ch, char * argument)
{
	char arg1[MIL],arg2[MIL];
	int votes;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master ){
			ch->master->println( "Not going to happen.");
		}
		return;
	}

	if (!IS_NOBLE(ch))
	{
		ch->println("Voting is only done by nobles... You are not a noble sorry.");
		return;
	}


	if(moot->moot_type==0)
	{
		ch->println("There is no moot in progress.");
		return;
	}

	if(arg1[0]=='\0' || !is_number(arg2) ||
        !(!str_prefix( arg1, "favor" ) 
		|| !str_prefix( arg1, "favour" )
		|| !str_prefix( arg1, "oppose" )))
	{
        ch->println("Syntax: vote favour <votes>");
		ch->println("Syntax: vote oppose <votes>");
		return;
	}

	votes=atoi(arg2);
	if(votes> TRUE_CH(ch)->pcdata->dip_points)
	{
		ch->println("You don't have that many votes.");
		return;
	}
	if(votes<=0)
	{
		ch->println("Postive votes only.");
		return;
	}
	if(TRUE_CH(ch)->pcdata->votes_cast)
	{
		ch->println("You have already voted.");
		return;
	}
	
    if(!str_prefix( arg1, "favor" ) || !str_prefix( arg1, "favour" ))
	{
		TRUE_CH(ch)->pcdata->in_favor=1;
		TRUE_CH(ch)->pcdata->dip_points-=votes;
		TRUE_CH(ch)->pcdata->votes_cast=votes;
		moot->votes_for+=votes;
	}
	else
	{
		TRUE_CH(ch)->pcdata->in_favor=-1;
		TRUE_CH(ch)->pcdata->dip_points-=votes;
		TRUE_CH(ch)->pcdata->votes_cast=votes;
		moot->votes_against+=votes;
	}

	ch->println("Your votes have been cast");

    moot->number_of_votes++;

	moot_check_bring_forward();

    return;
}
/**************************************************************************/
