/**************************************************************************/
// mount.cpp - mob mounting system written by Thaddeus
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h" //default dawn include files

/**************************************************************************/
void mount(char_data * rider, char_data * ridee)
{
    if (is_affected(rider, gsn_sneak))
    {
        rider->println( "You cannot sneak while riding." );
        affect_strip(rider, gsn_sneak);
    }
    if (is_affected(rider, gsn_hide))
    {
        rider->println("You cannot hide while riding." );
        affect_strip(rider, gsn_hide);
    }
   
    ridee->tethered=false; 
    if (!(ridee->bucking))
       SET_BIT(ridee->act, ACT_SENTINEL); 
    
    if (ridee->position==POS_SLEEPING)
    {
        act("$N wakes up.", rider, NULL, ridee, TO_ROOM);
    }
    
    if (ridee->position<POS_STANDING)
    {
        act("$N stands up.", rider, NULL, ridee, TO_ROOM);
    }
    ridee->position = POS_STANDING;

    if (!IS_NPC(rider))
    {
        if (get_skill(rider, gsn_riding)<75)
        {
            check_improve(rider, gsn_riding, true, 1);
        }
    }
   
    ridee->ridden_by=rider;
    rider->mounted_on=ridee;
}

/**************************************************************************/
void tame_a_little(char_data * ch, char_data * victim)
{
    int beats = (101 - ch->level + victim->level)*2;
    int skill_level = get_skill(ch, gsn_domesticate);
    if (skill_level==0)
    {
		ch->println( "You don't know how to tame." );
		return;
    }

    
    act("You try to tame $N.", ch, NULL, victim, TO_CHAR);
    switch(number_range(1,3))
    {
		case 1:
			act("$n tries to tame $N.", ch, NULL, victim, TO_ROOM);
			break;
       
		case 2:
			act("$n trains $N a little.", ch, NULL, victim, TO_ROOM);
			break;

		case 3:
			act("$n works a bit with $N.", ch, NULL, victim, TO_ROOM);
			break;
    }
    
    if (victim->wildness>80)
       {
       act("$N still seems quite wild.", ch, NULL, victim, TO_CHAR);
       act("$N still seems quite wild.", ch, NULL, victim, TO_ROOM);
       }
    else if (victim->wildness>60)
       {
       act("$N is not so wary of you.", ch, NULL, victim, TO_CHAR);
       act("$N doesn't seem as afraid of $n.", ch, NULL, victim, TO_ROOM);
       }
    else if (victim->wildness>40)
       {
       act("$N seems to be getting used to the idea.", ch, NULL, victim, TO_CHAR);
       act("$N seems to be getting used to the idea.", ch, NULL, victim, TO_ROOM);
       }
    else if (victim->wildness>20)
       {
       act("$N will allow you to pat it.", ch, NULL, victim, TO_CHAR);
       act("$N will allow $n to pat it.", ch, NULL, victim, TO_ROOM);
       }
    else
       {
       act("$N will eat out of your hand.", ch, NULL, victim, TO_CHAR);
       act("$N will eat out of $n's hand.", ch, NULL, victim, TO_ROOM);
       }

    
    if (skill_level < 10)
        beats = beats * 2;
    else if (skill_level < 20)
        beats = (beats * 7)/ 4;
    else if (skill_level < 30)
        beats = (beats * 3)/ 2;
    else if (skill_level < 40)
        beats = (beats * 5) / 4;
    else if (skill_level > 60)
        beats = (beats * 5) / 6;
    else if (skill_level > 70)
        beats = (beats * 4) / 5;
    else if (skill_level > 80)
        beats = (beats * 3) / 4;
    else if (skill_level > 90)
        beats = (beats * 2) / 3;
    else if (skill_level == 100)
        beats = (beats * 1) / 2;
    
    skill_level = 1+(ch->level - victim->level)/8;
	skill_level +=ch->modifiers[STAT_PR]/3;
    
    if (IS_SET(victim->act, ACT_AGGRESSIVE))
       skill_level /=2;
    if (IS_SET(victim->form, FORM_SENTIENT))
       skill_level/=3;
    
	if (skill_level<1 
		&& number_range(1,get_skill(ch, gsn_domesticate))<60)
    {
       ch->println( "Your efforts seem to be futile." );
       return;
    }
    
    victim->wildness-=skill_level;   
    
    check_improve(ch, gsn_domesticate, true, 3);
    

    if (IS_SET(victim->form, FORM_MOUNTABLE))
    {
       beats/=2;
       victim->wildness--;
    }
     

    if (victim->wildness<=0)
    {   
        victim->wildness=0;   
        victim->will/=2;
        // make a pet out of it 
        SET_BIT(victim->act, ACT_PET);
        SET_BIT(victim->affected_by, AFF_CHARM);
		REMOVE_BIT(victim->act, ACT_AGGRESSIVE);

        victim->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
        add_follower( victim, ch );
        victim->leader = ch;
        ch->pet = victim;
        act( "You have finished taming $N!", ch, NULL, victim, TO_CHAR);
        act( "$n has tamed $N as a pet!", ch, NULL, victim, TO_ROOM );
    }
    else
    {
        WAIT_STATE(ch, beats); 
    }
    
    return;
}

/**************************************************************************/
void dismount(char_data * rider)
{
    if (rider->mounted_on==NULL)
    {
        bug("Error in dismount, rider isn't riding anything!");
        return;
    }
    rider->mounted_on->ridden_by=NULL;
    rider->mounted_on=NULL;
}

/**************************************************************************/
// get on
void do_ride( char_data *ch, char *argument )  
{
    char arg[MIL];
    char_data *victim;
    
    one_argument( argument, arg );

    if( IS_NULLSTR(arg) ){
		if(	ch->pet 
			&& ch->pet->in_room==ch->in_room 
			&& IS_SET(ch->pet->form, FORM_MOUNTABLE)
			&& ch->pet->size>race_table[ch->race]->size)
		{
			victim=ch->pet;
		}else{
			ch->println( "syntax: ride <animal>" );
			ch->println( "or just: ride    (if you have a rideable pet in the room)" );
			ch->println( "(for this message to be displayed, there is no rideable pet of yours in the room)." );
			return;
		}
	}else{
		if ( ( victim = get_char_room( ch, arg ) ) == NULL )
		{
			ch->println( "You don't see that here." );
			return;
		}
	}
   
    if (!(IS_NPC(victim)))
    {
        ch->println( "You cannot mount other players!" );
        return;
    }

    if (ch->mounted_on)
    {
        ch->println( "You are already mounted on something else!" );
        return;
    }

    if (victim->ridden_by)
    {
        ch->println( "That is already mounted by someone else!" );
        return;
    }
   
    if (!IS_SET(ch->form, FORM_BIPED))
    {
        ch->println( "Your body is not built for that." );
        return;
    }
   
    if (!IS_SET(victim->form, FORM_MOUNTABLE))
    {
        ch->println( "It is not that kind of creature." );
        return;
    }
   
    if ( IS_AFFECTED(ch, AFF_FEAR) || IS_AFFECTED2(ch, AFF2_FEAR_MAGIC) )
    {
        ch->println( "You are too afraid of it." );
        return;
    }

	if ( !IS_NPC( ch ))
	{
		if (victim->size <= race_table[TRUE_CH(ch)->race]->size)
		{
			ch->println( "It is too small for you to mount." );
			return;
		}
	}
   
    if (victim->position<POS_SLEEPING)
    { 
		act("$N doesn't look well enough for that.", ch, NULL, victim, TO_CHAR);
		return;
    }

    if (victim->position==POS_SLEEPING)
    { 
		act("$N appears to be sleeping.", ch, NULL, victim, TO_CHAR);
		return;
    }

    
    if ((!IS_SET(victim->act, ACT_DOCILE))
       &&(!IS_NPC(ch))
       &&(victim->master != ch))
    {
        if (get_skill(ch, gsn_animal_training)==0)
        {
            act("$N will not allow you to mount it!",ch, NULL, victim, TO_CHAR);
            act("$n tries to mount $N but is soundly rebuffed.",ch, NULL, victim, TO_ROOM);
            return;
        }
        else
        {
            act("You mount $N in an attempt to break them.",ch, NULL, victim, TO_CHAR);
            act("$n mounts $N in an attempt to break them.",ch, NULL, victim, TO_ROOM);
            victim->bucking=true;
            mount(ch, victim);
            return;
        }
    }
       
    if((!IS_SET(victim->act, ACT_DOCILE))
       &&(IS_NPC(ch))
       &&(victim->master != ch))   
    {
        act("$N will not allow you to mount it!",ch, NULL, victim, TO_CHAR);
        act("$n tries to mount $N but is soundly rebuffed.",ch, NULL, victim, TO_ROOM);
        return;
    }

    if(IS_SET(race_table[ch->race]->aff, AFF_FLYING))
    {
        act("$n flies up and alights on $N.",ch, NULL, victim, TO_ROOM);
        act("You fly up and alight on $N.",ch, NULL, victim, TO_CHAR);
        mount(ch, victim);
        return;
    }
   
   
    if(!IS_NPC(ch))
    {
        if (get_skill(ch, gsn_riding)==0)   
        {
            if (number_range(1,3)!=1)
            {
                act("$n tries to mount $N, but fails.",ch, NULL, victim, TO_ROOM);
                act("You try to mount $N, but fail.",ch, NULL, victim, TO_CHAR);
                return;
            }
            else 
            {
                act("$n is barely able to mount $N.",ch, NULL, victim, TO_ROOM);
                act("You barely manage to pull yourself onto $N.",ch, NULL, victim, TO_CHAR);
                mount(ch, victim);
                return;
            }
        }
    }
   
   
    act("$n mounts $N.",ch, NULL, victim, TO_ROOM);
    act("You mount $N.",ch, NULL, victim, TO_CHAR);

    if (IS_NEWBIE(ch))
    {
        ch->println( "Type dismount or tether to climb off when you want to." );
    }
    mount(ch,victim);
    return;
}
/**************************************************************************/
void do_untether( char_data *ch, char *argument)
{
    char_data *victim;
    char arg[MIL];
    
    one_argument( argument, arg );

	if(IS_NULLSTR(arg))
	{
		ch->println( "Untether what?" );
		return;
	}

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        ch->printlnf("You don't see any '%s' here.", arg);
		return;
    }
  
	if(victim->tethered!=true){
        ch->printlnf("%s is not tethered.", PERS(victim, ch));
        return;
	}

    if (!(IS_NPC(victim)))
    {
        ch->printlnf("%s doesn't appear to want to be untethered.", PERS(victim, ch));
        return;
    }

 
    if ( IS_AFFECTED(ch, AFF_FEAR) || IS_AFFECTED2(ch, AFF2_FEAR_MAGIC) )
    {
        ch->println( "You are too afraid of it." );
        return;
    }
      
    if ((!IS_SET(victim->act, ACT_DOCILE))
       &&(!IS_NPC(ch))
       &&(victim->master != ch))
    {
        if (get_skill(ch, gsn_animal_training)==0)
        {
            act("$N will not allow you to untether it!",ch, NULL, victim, TO_CHAR);
            act("$n tries to untether $N but is soundly rebuffed.",ch, NULL, victim, TO_ROOM);
            return;
        }
    }
       
    if ((!IS_SET(victim->act, ACT_DOCILE))
       &&(IS_NPC(ch))
       &&(victim->master != ch))   
    {
        act("$N will not allow you to untether it!",ch, NULL, victim, TO_CHAR);
        act("$n tries to untether $N but is soundly rebuffed.",ch, NULL, victim, TO_ROOM);
        return;
    }

    act("$n untethers $N.",ch, NULL, victim, TO_ROOM);
    act("You untether $N.",ch, NULL, victim, TO_CHAR);
   
    victim->tethered=false;
}
/**************************************************************************/
void tether_not_riding( char_data *ch, char *arg)
{
    char_data *victim;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        ch->printlnf("You don't see any '%s' here.", arg);
		return;
    }

	assert(!ch->mounted_on);

	if (victim->ridden_by)
    {
        ch->printlnf("%s is being ridden by %s.", 
			PERS(victim, ch), PERS(victim->ridden_by, ch));
        return;
    }

	if(victim->tethered==true){
        ch->printlnf("Someone else is riding %s.", PERS(victim, ch));
        return;
	}

    if (!(IS_NPC(victim)))
    {
        ch->printlnf("%s doesn't appear to want to be tethered.", PERS(victim, ch));
        return;
    }

    if (!IS_SET(victim->form, FORM_MOUNTABLE))
    {
        ch->println( "It is not that kind of creature." );
        return;
    }
   
    if ( IS_AFFECTED(ch, AFF_FEAR) || IS_AFFECTED2(ch, AFF2_FEAR_MAGIC) )
    {
        ch->println( "You are too afraid of it." );
        return;
    }
      
    if ((!IS_SET(victim->act, ACT_DOCILE))
       &&(!IS_NPC(ch))
       &&(victim->master != ch))
    {
        if (get_skill(ch, gsn_animal_training)==0)
        {
            act("$N will not allow you to tether it!",ch, NULL, victim, TO_CHAR);
            act("$n tries to tether $N but is soundly rebuffed.",ch, NULL, victim, TO_ROOM);
            return;
        }
    }
       
    if ((!IS_SET(victim->act, ACT_DOCILE))
       &&(IS_NPC(ch))
       &&(victim->master != ch))   
    {
        act("$N will not allow you to tether it!",ch, NULL, victim, TO_CHAR);
        act("$n tries to tether $N but is soundly rebuffed.",ch, NULL, victim, TO_ROOM);
        return;
    }

    act("$n tethers $N.",ch, NULL, victim, TO_ROOM);
    act("You tether $N.",ch, NULL, victim, TO_CHAR);
   
    victim->tethered=true;
}

/**************************************************************************/
// get off and tether creature
void do_tether( char_data *ch, char *argument)
{
    char arg[MIL];
    
    one_argument( argument, arg );

	// support tethering a mob that isn't being ridden
    if ( !ch->mounted_on){
		if(IS_NULLSTR(arg))
	    {
			ch->println( "Tether what?" );
			return;
		}else{
			tether_not_riding(ch, arg);
			return;
		}
	}

    act("$n dismounts and tethers $N.",ch, NULL, ch->mounted_on, TO_ROOM);
    act("You dismount and tether $N.",ch, NULL, ch->mounted_on, TO_CHAR);
   
    ch->mounted_on->tethered=true;

    dismount(ch);
   
    if (get_skill(ch, gsn_riding))
    {
        check_improve(ch, gsn_riding, true, 1);
    }
    
    return;  
}

/**************************************************************************/
// dismount creature
void do_dismount( char_data *ch, char *)
{
    if (!ch->mounted_on)
    {
        ch->println( "You have to be on a creature to dismount it." );  
        return;
    }
    act("$n dismounts $N.",ch, NULL, ch->mounted_on, TO_ROOM);
    act("You dismount $N.",ch, NULL, ch->mounted_on, TO_CHAR);
   
	ch->mounted_on->tethered=false;
    dismount(ch);
   
    if (get_skill(ch, gsn_riding))
    {
        check_improve(ch, gsn_riding, true, 1);
    }
    
    return;
   
}


/**************************************************************************/
void do_tame( char_data *ch, char *argument )  
{
    char arg[MIL];
    char_data *victim;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' ){
        ch->println( "Tame what?" );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL ){
        ch->println( "You don't see that here." );
        return;
    }
    
    if (IS_NPC(ch)){
        ch->println( "Only characters can tame." );
        return;
    }
    
    if (!IS_NPC(victim)){
       ch->println( "Don't you wish!" );
       return;
    }
    
    if (ch->pet){
        act("$N would be too jealous of your other pet.", ch, NULL, victim, TO_CHAR); 
        return;
    }
    
    if (victim->ridden_by){
        act("$N seems to belong to someone else.", ch, NULL, victim, TO_CHAR); 
        return;
    }
    
    if (IS_SET(victim->act, ACT_PET)){ 
		ch->println( "It is already quite tame." );
        return;
    }
    
    if (IS_SET(victim->act, ACT_NO_TAME)){                 
        ch->println( "That creature will never make a good pet." );
        return;
    }
    
    if (ch->level < victim->level){
       act("$N would probably tame YOU if you tried that!", ch, NULL, victim, TO_CHAR);
       return;
    }  

    if (victim->position<POS_SLEEPING){ 
		act("$N doesn't look well enough for that.", ch, NULL, victim, TO_CHAR);
		return;
    }

    if (victim->position==POS_SLEEPING){ 
		act("$N appears to be sleeping.", ch, NULL, victim, TO_CHAR);
		return;
    }

	if( victim->fighting ){
		act("$N didn't seem to pay much attention to you.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if( ch->fighting ){
		ch->println("Not while you are fighting.");
		return;
	}

	// doubletame on mountable mobs
	if (IS_SET(victim->form, FORM_MOUNTABLE)){
        tame_a_little(ch, victim);
    }
   
    tame_a_little(ch, victim);
    
    return;
}
/**************************************************************************/
