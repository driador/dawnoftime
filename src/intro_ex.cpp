/**************************************************************************/
// intro_ex.cpp - Introduction system Extra Functions - Kal May 2000
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "intro.h"
/**************************************************************************/

/**************************************************************************/
void do_introduce(char_data *ch, char * argument)
{
	char_data *victim;
	if(IS_NPC(ch)){
		ch->println("players only");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println("introduce <player>");
		ch->println("introduce all");
		return;
	}

	if(!str_cmp(argument, "all")){
		for(victim=ch->in_room->people; victim; victim=victim->next_in_room){
			if(IS_NPC(victim) || victim==ch || !can_see(ch, victim) || !IS_AWAKE(victim)){
				continue;
			}

			ch->printlnf("You introduce yourself to %s", PERS(victim, ch));
			act("$N introduces $Mself to you as $t",victim, ch->name, ch, TO_CHAR);
			victim->know->introduced_to(ch);
		}
		return;
	}

	victim= get_char_room(ch, argument);
	if(!victim){
		ch->printlnf("Couldn't find any '%s' in the room to introduce yourself to", argument);
		return;
	}

	if(IS_NPC(victim)){
		ch->println("Introduce to players only");
		return;
	}
	if(victim==ch){
		ch->println("Cant introduce yourself to yourself.");
		return;
	}

	if ( !IS_AWAKE(victim))
	{
		ch->println( "Wait till they wake up before you make your introductions." );
		return;
	}

	if(!can_see(victim, ch)){
		ch->printlnf("You attempt to introduce yourself to %s, but it doesn't appear that they can see you.", 
			PERS(victim, ch));
		return;
	}

	ch->printlnf("You introduce yourself to %s", PERS(victim, ch));
	act("$N introduces $Mself to you as $t",victim, ch->name, ch, TO_CHAR);
	victim->know->introduced_to(ch);
};
/**************************************************************************/
void do_askname(char_data *ch, char * argument)
{
	char_data *victim;
	if(IS_NPC(ch)){
		ch->println("players only");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println("  `BSyntax:`x askname <player>");
		ch->println("");
		ch->wrapln("Use askname to request a player to introduce themselves to you... they will "
			"automatically introduce themselves to you if they haven't turned off autoanswer.");
		return;
	}

	victim= get_char_room(ch, argument);
	if(!victim){
		ch->printlnf("Couldn't find any '%s' in the room to ask their name of.", argument);
		return;
	}

	if(IS_NPC(victim)){
		ch->printlnf("%s doesn't look very interested in you.", PERS(victim,ch));
		return;
	}
	if(victim==ch){
		ch->println("You already know your own name.");
		return;
	}
	if(ch->know && ch->know->knows(victim)){
		ch->printlnf("You already know %s.", victim->name);
		return;
	}

	ch->printlnf("You ask %s what their name is.", victim->short_descr);
	victim->printlnf("`Y%s asks you what your name is.`x", PERS(ch, victim));
	if(!HAS_CONFIG(victim, CONFIG_NOAUTOANSWER)){
		do_introduce(victim, ch->name);
	}
};
/**************************************************************************/
// not finished yet
void do_forget(char_data *ch, char * argument)
{
	if(IS_NPC(ch)){
		ch->println("players only");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->println("forget <player>");
		ch->println("- removes their name beside their short after they have introduced themselves");
		return;
	}

	char_data *victim= get_char_room(ch, argument);
	if(!victim){
		ch->printlnf("Couldn't find any '%s' in the room to forget.", argument);
		return;
	}

	if(ch->know && ch->know->knows(victim)){
		// make them forget the name
		ch->know->forgetting(victim);
		ch->printlnf("You choose to not recall the name of %s.", PERS(victim, ch));
	}else{
		ch->printlnf("You don't know the name of %s.", PERS(victim, ch));
	}

};
/**************************************************************************/
// Short description function - by Kalahn - Multibuffered :)
// supports looker being NULL (for operating system stuff).
char * PERS( char_data *ch, char_data *looker)
{
	static int i;
    static char buf[5][MSL];
	char buf2[MSL];

	if (!ch){
		return "";
	}

	// create an mxp tag for the mob/mob running on behalf of an object
	char *mxp_tag_for_entity;
	if(ch->running_mudprog_for_object){
		mxp_tag_for_entity=mxp_tag_for_object(looker, ch->running_mudprog_for_object);
	}else{
		mxp_tag_for_entity=mxp_tag_for_mob(looker, ch);
	}

	char short_descr[MSL];
	strcpy(short_descr, mxp_create_tag(looker, mxp_tag_for_entity, ch->short_descr));

	char cname[MSL];
	strcpy(cname, mxp_create_tag(looker, mxp_tag_for_entity, capitalize(TRUE_CH(ch)->name)));
	char name[MSL];
	strcpy(name, mxp_create_tag(looker, mxp_tag_for_entity, TRUE_CH(ch)->name));
	
	// rotate buffers
	++i%=5;
	char *result=buf[i];

    if ( !looker || can_see(looker, ch) || IS_SET(looker->act, PLR_HOLYNAME))
    {
        // MOBS
        if ( IS_NPC(ch)){
            if (    !looker 
				  || IS_SET(looker->act, PLR_HOLYNAME)
                  || IS_CONTROLLED (looker)
                  || IS_SWITCHED (looker) )               
            {
                if (IS_CONTROLLED(ch)){
                    sprintf( result, "%s `#`Y[%s]`&", short_descr, name);
                }else{
                    sprintf( result, "%s", short_descr);
				}
            }else{
                strcpy(result,short_descr);
			}
        // PLAYERS
        }else{
			if(GAMESETTING(GAMESET_NOSHORT_DESCRIPTS)){
				strcpy( result, name);
			}else{
				if (	!looker || 
				  		(
							(IS_SET(TRUE_CH(looker)->act, PLR_HOLYNAME)
								|| GAMESETTING(GAMESET_HOLYNAME_FOR_ALL)
								|| IS_CONTROLLED(looker)
								|| IS_SWITCHED(looker) 
							)
							&& str_cmp(ch->short_descr, ch->name) 
						)
				   )
				{
					if(looker && HAS_CONFIG2(looker,CONFIG2_NAME_ONLY_FOR_KNOWN)){
						strcpy( result, cname);
					}else{
						if(looker && HAS_CONFIG(looker, CONFIG_NAMES_BEFORE_SHORT)){
							sprintf( result, "(%s) %s", capitalize(ch->name), short_descr);
						}else{
							sprintf( result, "%s (%s)", short_descr, capitalize(ch->name) );
						}
					}
				}else{
					// nonimmortal player 
					if (IS_OOC(ch) && IS_OOC(looker)){
						strcpy( result, name);
					}else{
						// check for intro system
						if(!HAS_CONFIG(looker, CONFIG_NONAMES) 
							&& looker->know 
							&& looker->know->knows(ch))
						{
							if(looker && HAS_CONFIG2(looker,CONFIG2_NAME_ONLY_FOR_KNOWN)){
								strcpy( result, cname);
							}else{
								if(HAS_CONFIG(looker, CONFIG_NAMES_BEFORE_SHORT)){
									sprintf( result, "(%s) %s", capitalize(ch->name), short_descr);
								}else{
									sprintf( result, "%s (%s)", short_descr, capitalize(ch->name));
								}
							}
						}else{
							strcpy( result, short_descr);
						}
					}
				}
			}
		}
    }else{
		strcpy(result,"someone");
	}

	// extra things to append to what someone sees
	if (!looker || (!IS_UNSWITCHED_MOB(looker) &&
		HAS_DESC(looker) && IS_SET(TRUE_CH(looker)->act, PLR_HOLYVNUM)))
	{
		if (ch->pIndexData) // mobs
		{
			if(ch->running_mudprog_for_object){ 
				// objectprog mob
				sprintf(buf2," `#`=H#OBJ%s%d,%d#`&", 
					ch->running_mudprog_for_object->events?"Q":"",
					ch->running_mudprog_for_object->pIndexData->vnum, 
					ch->level);
			}else if (ch->pIndexData->mob_triggers){
				sprintf(buf2," `#`=H*%s%d,%d*`&", 
					ch->events?"Q":"",
					ch->pIndexData->vnum, 
					ch->level);
			}else{
				sprintf(buf2," `#`=h[%s%d,%d]`&", 
					ch->events?"Q":"",
					ch->pIndexData->vnum, 
					ch->level);
			}
			strcat(result,buf2);
		}
		else
		{
			// <level> for players
			sprintf(buf2," `#`=L<%d>`&", ch->level);
			strcat(result,buf2);
		}
		
	}

	// check for validity
	if(!IS_VALID(ch)){
		strcat(result,"<<<<--INVALID CH!!!");
	}

	// IMMS AND MOBS USING THE SILENTLY COMMAND
	if (IS_SILENT(ch)
		&& (!looker || IS_SET(TRUE_CH(looker)->act, PLR_HOLYLIGHT)) )
	{
		sprintf(buf2," `#`Y[SILENTLY]`&");
		strcat(result,buf2);
	}

	if ((IS_SET( TRUE_CH(ch)->dyn, DYN_USING_AT))
		&& (!looker && IS_SET(TRUE_CH(looker)->act, PLR_HOLYLIGHT)))
	{
		sprintf(buf2," `#`Y[AT]`&");
		strcat(result,buf2);
	}

	// strip off the colour from the details if the looker is NULL
	if (!looker){
		char *nocolour=strip_colour(result);
		strcpy(result, nocolour);
	}

    return (result);
}
/**************************************************************************/
/**************************************************************************/
