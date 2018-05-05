/**************************************************************************/
// act_info.cpp - primarily code showing players information
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
#include "magic.h"
#include "olc.h"
#include "duel.h"
#include "math.h"
#include "msp.h"
#include "ictime.h"
#include "lockers.h"
#include "nanny.h"

/* command procedures needed */
DECLARE_DO_FUN( do_exits    );
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_map); // Kal
DECLARE_DO_FUN( do_tracks); // Kal

int get_birthmonth( char_data *ch );
char *get_weapontype(OBJ_DATA *obj);

void where_char    args((char_data *victim, char_data *ch,
                            sh_int depth, sh_int door));

const char *const where_distance[2] = {
    "in the same room.",
    "directly to the %s."
};
void letter_read( char_data *ch, OBJ_DATA *letter );

DECLARE_DO_FUN( do_count        );

const char * const where_name [] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on torso>     ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              ",
    "<floating nearby>   ",
    "<secondary weapon>  ",
	"<lodged in arm>     ",
	"<lodged in leg>     ",
	"<lodged in rib>     ",
	"<sheathed>          ",
	"<concealed>         ",
    "<worn on eyes>      ",
    "<worn on ear>       ",
    "<worn on ear>       ",
    "<worn on face>      ",
    "<worn on ankle>     ",
    "<worn on ankle>     ",
	"<worn on back>      ",
	"<worn as a pendant> "
};


/*
 * Local functions.
 */
void	show_list_to_char	args( ( OBJ_DATA *list, char_data *ch, char *filter,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( char_data *victim, char_data *ch ) );
void	show_char_to_char_1	args( ( char_data *victim, char_data *ch ) );
void	show_char_to_char	args( ( char_data *list, char_data *ch ) );
bool	check_blind		args( ( char_data *ch ) );

/**************************************************************************/
char *get_canwear_colour( OBJ_DATA *obj, char_data *ch)
{
    int can_wear=0; // if not 0, it can't be worn

	
		
    if(  (  IS_SET(obj->extra_flags, OBJEXTRA_MAGIC)
		    && HAS_CLASSFLAG(ch, CLASSFLAG_MAGIC_ANTIPATHY)
		  )
      ||(!IS_NPC(ch) && obj->pIndexData->relative_size<
                           race_table[ch->race]->low_size &&
            obj->item_type != ITEM_LIGHT &&
            !CAN_WEAR( obj, OBJWEAR_HOLD)  &&
            !CAN_WEAR( obj, OBJWEAR_FLOAT) )
      ||(!IS_NPC(ch) && obj->pIndexData->relative_size>
                           race_table[ch->race]->high_size &&
            obj->item_type != ITEM_LIGHT &&
            !CAN_WEAR( obj, OBJWEAR_FLOAT) )
      )
    {
        can_wear=1;
    }

    if(!IS_NPC(ch) && !can_wear)
    {
		can_wear = DISALLOWED_OBJECT_FOR_CHAR(obj, ch)?1:0;
    }

	// restrictions for objects
    if(!IS_NPC(ch) && !can_wear)
    {
		if(obj->pIndexData && ch->pcdata
			&& HAS_CONFIG(ch,CONFIG_OBJRESTRICT) 
			&& ((ch->pcdata->objrestrict& obj->pIndexData->objrestrict)>0))
		{
			can_wear=2;
		}

	}

	switch(can_wear){
	case 0:
		return "`=d";
	case 1:
		return "`=k";
	case 2:
		return "`=K";
	}

	bugf("get_canwear_colour(): unknown value %d", can_wear);
	return "`x";
}
/**************************************************************************/
char *format_obj_to_char_new( OBJ_DATA *obj, char_data *ch, bool fShort )
{
    static char buf[MSL];
    buf[0] = '\0';
	char name[MSL];
    name[0] = '\0';


	// get the objects name into 'name'
    if ( fShort ){
		if ( !IS_NULLSTR(obj->short_descr) ){
			strcpy( name, obj->short_descr );
		}
    }else{
		if ( !IS_NULLSTR(obj->description) ){
			strcpy( name, obj->description );
		}
    }

    if ( ( fShort && IS_NULLSTR(obj->short_descr) )
		|| (!fShort && IS_NULLSTR(obj->description) ) )
	{
		if (HAS_HOLYLIGHT(ch))
		{
			char buf2[MSL];

			sprintf(buf2," `#`m(no short|long%s%d)", 
				obj->pIndexData?" ":"", obj->pIndexData?obj->pIndexData->vnum:0);
			strcat(buf,buf2);
			if (obj->pIndexData && 
				!IS_UNSWITCHED_MOB(ch) && IS_SET(TRUE_CH(ch)->act, PLR_HOLYVNUM))
			{
				if(obj->pIndexData->obj_triggers){
					sprintf(buf2," `#`G*%s%d,%d*", 
						obj->events?"Q":"",
						obj->pIndexData->vnum, 
						obj->level);
				}else{
					sprintf(buf2," `#`g[%s%d,%d]", 
						obj->events?"Q":"",
						obj->pIndexData->vnum, 
						obj->level);
				}
				strcat(buf,buf2);
			}
			strcat(buf,"`&");			
		}
		return buf;
	}

	
	
	
    if ( IS_OBJ_STAT(obj, OBJEXTRA_INVIS)   )		strcat( buf, game_settings->aura_invis);
	if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
			&& IS_OBJ_STAT(obj, OBJEXTRA_EVIL) )	strcat( buf, game_settings->aura_evil);
    if ( IS_AFFECTED(ch, AFF_DETECT_GOOD)
		 &&  IS_OBJ_STAT(obj,OBJEXTRA_BLESS))		strcat( buf, game_settings->aura_good);
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, OBJEXTRA_MAGIC))		strcat( buf, game_settings->aura_magical);
    if ( IS_OBJ_STAT(obj, OBJEXTRA_GLOW)    )		strcat( buf, game_settings->aura_glowing);
    if ( IS_OBJ_STAT(obj, OBJEXTRA_CHAOS)	)		strcat( buf, game_settings->aura_chaos);	
	if ( IS_OBJ_STAT(obj, OBJEXTRA_HUM)		)		strcat( buf, game_settings->aura_hum);
	if ( IS_SET( obj->extra2_flags, OBJEXTRA2_BURIED))	strcat( buf, game_settings->aura_buried);
//	if ( IS_TRAPPED( obj )
//		&& IS_AFFECTED2( ch, AFF2_DET_TRAPS ))	strcat( buf, "{#{r(Trapped){& "		);

	if ( IS_WEAPON_STAT(obj, WEAPON_HOLY)     
		&&  IS_OBJ_STAT(obj,OBJEXTRA_ANTI_EVIL))   strcat(buf,game_settings->aura_holy);

	if(CAN_WEAR(obj, OBJWEAR_WIELD))
	{
		if ( IS_WEAPON_STAT(obj, WEAPON_HOLY) &&  IS_OBJ_STAT(obj,OBJEXTRA_ANTI_GOOD)){
			strcat(buf,game_settings->aura_unholy);
		}
		if ( IS_WEAPON_STAT(obj, WEAPON_FLAMING)){
			strcat(buf,game_settings->aura_flaming);
		}
		if ( IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)){
			strcat(buf,game_settings->aura_vampric);
		}
		if ( IS_WEAPON_STAT(obj, WEAPON_SHOCKING)){
			strcat(buf,game_settings->aura_shocking);
		}
		if ( IS_WEAPON_STAT(obj, WEAPON_FROST)){
			strcat(buf,game_settings->aura_frost);
		}
    }

	// mxp the object
	strcat(buf, mxp_create_tag(ch, mxp_tag_for_object(ch, obj), name));
	
	if (!IS_UNSWITCHED_MOB(ch) && IS_SET(TRUE_CH(ch)->act, PLR_HOLYVNUM)){
		char buf2[MSL];

		if (obj->pIndexData)
		{
			if(obj->pIndexData->obj_triggers){
				sprintf(buf2," `#`G*%s%d,%d*`&", 
					obj->events?"Q":"",
					obj->pIndexData->vnum, 
					obj->level);
			}else{
				sprintf(buf2," `#`g[%s%d,%d]`&", 
					obj->events?"Q":"",
					obj->pIndexData->vnum, 
					obj->level);
			}
			strcat(buf,buf2);
		}
	}
    return buf;
}

/**************************************************************************/
char *format_obj_to_char( OBJ_DATA *obj, char_data *ch, bool fShort )
{
    static char buf[MSL];
	sprintf(buf,"`#%s%s`&", 
		get_canwear_colour( obj, ch),
		format_obj_to_char_new( obj, ch, fShort ));
    return buf;
}
/**************************************************************************/
/* Show a list to a character.
 * Can coalesce duplicated items.
 * filter support added by Kalahn Feb 01.
 */
void show_list_to_char( OBJ_DATA *list, char_data *ch, char *filter, bool fShort, bool fShowNothing )
{
    char buf[MSL];
    BUFFER *output;
    char **prgpstrShow;
    char **prgpstrShowMXP=NULL;
    char **prgpstrColour;
    int *prgnShow;
    char *pstrShow;
	OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
	bool fCombine;

    if ( ch->desc == NULL ){
		return;
	}

	// pre count the number of items we will be displaying
    count = 0;
    for ( obj = list; obj; obj = obj->next_content ){
		count++;
	}
	if(count<1){
		return; // don't do anything if nothing to display
	}

	// record if they have MXP, if they do, initially work with non MXP versions
	// of the object list so we can compact easily without MXP UID's confusing
	// the compact system.
	bool player_has_mxp;
	if(HAS_MXP(ch)){
		TRUE_CH(ch)->pcdata->mxp_enabled=false;
		player_has_mxp=true;
	}else{
		player_has_mxp=false;
	}
	
	// Alloc space for output lines.
	output = new_buf();
	
	// allocate memory to handle the sorting of this list etc
	prgnShow      = (int *)alloc_mem( count * sizeof(int)    );
	prgpstrShow	  = (char **)alloc_mem( count * sizeof(char *) );
	prgpstrColour = (char **)alloc_mem( count * sizeof(char *) );
	if(player_has_mxp){
		prgpstrShowMXP =(char **)alloc_mem( count * sizeof(char *) );
	}
	
	nShow	= 0;
	
	// Format the list of objects.
	bool matched;
	for ( obj = list; obj != NULL; obj = obj->next_content )
	{ 
		// filter support for inventory list etc
		if(!IS_NULLSTR(filter)){
			matched=false;
			if(is_name( filter, obj->name)) {
				matched=true;
			}else if(is_name( filter, obj->short_descr )) {
				matched=true;
			}else if(is_name( filter, obj->description )) {
				matched=true;
			}else if(flag_value( item_types, filter) == obj->item_type){
				matched=true;
			}
			
			if(!matched){
				continue;
			}
		}
		
		if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj )) 
		{
			pstrShow = format_obj_to_char_new( obj, ch, fShort );
			fCombine = false;
			
			if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
			{
				//  Look for duplicates, case sensitive.
				//  Matches tend to be near end so run loop backwords.
				for ( iShow = nShow - 1; iShow >= 0; iShow-- )
				{
					if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
					{
						prgnShow[iShow]++;
						fCombine = true;
						break;
					}
				}
			}
			
			// Couldn't combine, or didn't want to.
			if ( !fCombine )
			{
				prgpstrShow [nShow] = str_dup( pstrShow );
				prgnShow    [nShow] = 1;
				
				// record the colour code of this object 
				prgpstrColour[nShow]=str_dup(get_canwear_colour( obj, ch));
				
				// record MXP version if necessary
				if(player_has_mxp){
					TRUE_CH(ch)->pcdata->mxp_enabled=true;
					prgpstrShowMXP[nShow]=str_dup(format_obj_to_char_new( obj, ch, fShort ));
					TRUE_CH(ch)->pcdata->mxp_enabled=false;
				}
				nShow++;
			}
		}
	}
	
	// Output the formatted list.
	for ( iShow = 0; iShow < nShow; iShow++ )
	{
		if (prgpstrShow[iShow][0] == '\0')
		{
			free_string(prgpstrShow[iShow]);
			continue;
		}
		
		if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
		{
			add_buf(output, "`#");
			add_buf(output, prgpstrColour[iShow]);
			
			if ( prgnShow[iShow] != 1 ){
				sprintf( buf, "(%2d) ", prgnShow[iShow] );
				add_buf(output,buf);
			}else{
				add_buf(output,"     ");
			}
		}
		
		if(player_has_mxp){
			add_buf(output,prgpstrShowMXP[iShow]);
			free_string( prgpstrShowMXP[iShow] );
		}else{
			add_buf(output,prgpstrShow[iShow]);
		}
		add_buf(output,"`&\r\n");
		
		free_string( prgpstrShow[iShow] );
		free_string( prgpstrColour[iShow] );
	}
	
	if ( fShowNothing && nShow == 0 )
	{
		if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
			ch->print("     ");
		ch->println( "Nothing." );
	}
	
	ch->sendpage(buf_string(output));
	free_buf(output);
	
	// Clean up.
	free_mem( prgnShow,    count * sizeof(int)    );
	free_mem( prgpstrShow, count * sizeof(char *) );
	free_mem( prgpstrColour, count * sizeof(char *) );
	if(player_has_mxp){
		// free the memory allocated to store the MXP version of the list
		free_mem( prgpstrShowMXP, count * sizeof(char *) );
		// turn MXP back on for connection
		TRUE_CH(ch)->pcdata->mxp_enabled=true;
	}		
	return;
}


/**************************************************************************/
void show_char_to_char_0( char_data *victim, char_data *ch )
{
    char buf[MSL], buf2[MSL];
	
	//prototype
	char * char_position_text( char_data *ch); 

	if (IS_UNSWITCHED_MOB(ch))
	{
		return;
	}

    buf[0] = '\0';
    if( IS_SET(victim->comm,COMM_AFK        )   ) strcat( buf, "[AFK] "        );

    if(IS_LINKDEAD(victim)){
		if(victim->pload){
			strcat( buf, "[PLOAD] ");
		}else{
			strcat( buf, "[LINKDEAD] ");
		}
	}
	if( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(Invis) "      );
    
	// wizi stuff
	if (INVIS_LEVEL(victim)&&  IS_IMMORTAL(ch)){
		if (INVIS_LEVEL(victim)==LEVEL_IMMORTAL){
			strcat( buf, "(Wizi) " );
		}else{
			buf2[0]='\0';
			sprintf(buf2, "(Wizi %d) ", INVIS_LEVEL(victim));
			strcat( buf, buf2);
			buf2[0]='\0';
		}
	}

    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "`#`g(Hide)`& "       );
    if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "`#`c(Charmed)`& "    );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "`#`C(Translucent)`& ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "`#`r(Pink Aura)`& "  );
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "`#`R(Red Aura)`& "   );
    if ( IS_GOOD(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) strcat( buf, "`#`Y(Golden Aura)`& ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "`#`W(White Aura)`& " );
	// should only ever been seen by imms (because of can_see)
	if ( IS_AFFECTED2(victim, AFF2_TREEFORM)  ) strcat( buf, "`#`G(Tree)`& "	   );
	// imms and faeries
	if ( IS_AFFECTED2(victim, AFF2_VANISH)    ) strcat( buf, "`#`S(Vanished)`& "   );
	
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_IS_UNSEEN)){
		if (HAS_HOLYLIGHT(ch)){
			strcat( buf, "(UNSEEN) " );
		}else{
			return;
		}
    }

/////////////////////////////////////////////////////////////////////
// Below displays chars in positions that don't use victim->position //
/////////////////////////////////////////////////////////////////////
	// subdued players
	if (IS_SUBDUED(victim))
	{
		ch->printlnf( "%s%s is here looking a little subdued.",
			buf, capitalize( PERS( victim, ch )));
        return;
	}

	// mounted players
	if (IS_MOUNTED(victim))
	{
		if (IS_RIDING(ch)==victim)
		{
			// we are riding the victim
			ch->printlnf( "%s`#`sYou are riding on %s.`&",
				buf, PERS( victim, ch ));
			return;
		}
		else
		{
			// the victim is being ridden, but not by us,
			// show them with the rider (not by themselves)
			return;
		}
	}

	// mob is tethered (NPC's only at this stage)
	if (IS_TETHERED(victim))
	{
		ch->printlnf( "%s`s%s is tethered here.`=D",
			buf, capitalize( PERS( victim, ch )));
        return;
	}

	// characters riding others
	if (IS_RIDING(victim))
	{
		ch->printlnf( "%s`#`s%s is here riding `&%s.`=D",
			buf,
			capitalize( PERS( victim, ch )),
			PERS( IS_RIDING(victim), ch ));
		return;
	}

/* NOTE: need to add 
* - subdued and tethered?
* - subdued and being ridden?
*/
//////////////////////////////////////////////////////////////
//    Above positions that don't use victim->position above    //
//////////////////////////////////////////////////////////////

	// mobs that are in default position and have look descripts
	if (IS_NPC(victim)
		&& victim->position == victim->start_pos 
		&& !IS_NULLSTR(victim->long_descr))
	{
		ch->printlnf( "%s%s", buf, LONGPERS(victim, ch));
		return;
	}

	// players and mobs in positions defined with victim->pos
	strcat ( buf, PERS(victim, ch));
	switch ( victim->position )
	{
	case POS_DEAD:
		strcat( buf, " is DEAD!!" );
		break;
	case POS_MORTAL:
		strcat( buf, " is mortally wounded." );
		break;
	case POS_INCAP:
		strcat( buf, " is incapacitated." );
		break;
	case POS_STUNNED:
		strcat( buf, " is lying here stunned." );
		break;
	case POS_SLEEPING:
	case POS_RESTING:
	case POS_SITTING:
	case POS_KNEELING:
	case POS_STANDING:
		strcat( buf, char_position_text(victim));
		break;
	case POS_FIGHTING:
		strcat( buf, " is here, fighting " );
		if ( victim->fighting == NULL )
			strcat( buf, "thin air??" );
		else if ( victim->fighting == ch )
			strcat( buf, "YOU!" );
		else if ( victim->in_room == victim->fighting->in_room )
		{
			strcat( buf, PERS( victim->fighting, ch ) );
			strcat( buf, "." );
		}
		else
			strcat( buf, "someone who left??" );
		break;
	} // end of switch (victim->position)

	buf[0]= UPPER(buf[0]);
	ch->println(buf);
	return;
}

/**************************************************************************/
void show_char_to_char_1( char_data *victim, char_data *ch )
{
	char buf[MSL];
	OBJ_DATA *obj;
	int iWear;
	int percent;
	bool found;
	bool peeking;

	peeking = ( victim != ch && !IS_NPC(ch)
		&& IS_SET(ch->act,PLR_AUTOPEEK)
		&&   number_percent( ) < get_skill(ch,gsn_peek));

	// can't peek at IC immortals unless you are an imm
	if (!IS_NPC(victim) && victim->level>= LEVEL_IMMORTAL)
	{
		peeking= false;
	}
	// ic imms always exceed at peeking
	if (!IS_NPC(ch) && (ch->level>= LEVEL_IMMORTAL) && IS_SET(ch->act,PLR_AUTOPEEK))
	{
		peeking= true;
	}

		
	if ( can_see( victim, ch ) )
	{
		if (ch == victim)
			act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
		else
		{
			if (peeking || number_percent( )<=5)
			{
				act( "$n looks at you thoroughly.", ch, NULL, victim, TO_VICT    );
				act( "$n looks at $N thoroughly.",  ch, NULL, victim, TO_NOTVICT );
			}
			else
			{
				act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
				act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
			}
		}
	}
	

	if ( victim->description[0] != '\0' )
	{
        ch->print(victim->description );
	}
	else
	{
        act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
	}
	
	if ( victim->max_hit > 0 )
        percent = ( 100 * victim->hit ) / victim->max_hit;
	else
        percent = -1;
	
	strcpy( buf, PERS(victim, ch) );
	
	if (percent >= 100)
        strcat( buf, " is in excellent condition.\r\n");
	else if (percent >= 90)
        strcat( buf, " has a few scratches.\r\n");
	else if (percent >= 75)
        strcat( buf," has some small wounds and bruises.\r\n");
	else if (percent >=  50)
        strcat( buf, " has quite a few wounds.\r\n");
	else if (percent >= 30)
        strcat( buf, " has some big nasty wounds and scratches.\r\n");
    else if (percent >= 15)
        strcat ( buf, " looks pretty hurt.\r\n");
    else if (percent >= 0 )
        strcat (buf, " is in `#`Rawful condition`&.\r\n");
    else
        strcat(buf, " is `#`Rbleeding`& to death.\r\n");
    
    buf[0] = UPPER(buf[0]);
    ch->print( buf );
    
    if (victim->mounted_on)
	{
		ch->printlnf( "%s is mounted on %s.",
			capitalize( victim->short_descr),
			victim->mounted_on->short_descr );
	}

	// flying status
	if ( IS_AFFECTED(victim, AFF_FLYING) ){
		ch->printlnf( "%s appears to be airborne.", PERS(victim,ch) );
	}

	// money changers
	if ( IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER)){
		ch->printlnf( "%s appears to be willing to exchange "
			"your silver coins for gold.", PERS(victim,ch) );
    }

    
    found = false;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if ( iWear == WEAR_CONCEALED
		&&   ch != victim
		&&  !HAS_HOLYLIGHT( ch ))
			continue;

        if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
			&&   can_see_obj( ch, obj ) )
        {
			if ( !found )
			{
                ch->println( "" );
                act( "$N is using:", ch, NULL, victim, TO_CHAR );
                found = true;
			}
			ch->printlnf( "`=\xab""%s%s`x",
				where_name[iWear],
				format_obj_to_char( obj, ch, true ));
		}
	}
	
	if (peeking)
	{
		ch->printlnf( "\r\nYou peek at the inventory of %s:`=\xab",
			PERS(victim,ch));
		check_improve(ch,gsn_peek,true,4);
		show_list_to_char( victim->carrying, ch, "", true, true );
		ch->print( "`x" );
	}
	return;
}

/**************************************************************************/
void show_char_to_char( char_data *list, char_data *ch )
{
	char_data *rch;
	
	for ( rch = list; rch; rch = rch->next_in_room )
	{
		if ( rch == ch )
			continue;
		
		if ( get_trust(ch) < INVIS_LEVEL(rch))
			continue;
		
		if ( can_see( ch, rch ) )
		{
			show_char_to_char_0( rch, ch );
		}
		else if ( room_is_dark( ch->in_room )
			&&        IS_AFFECTED(rch, AFF_INFRARED ) &&
			(IS_NPC(rch) && !IS_SET(rch->act, ACT_IS_UNSEEN)) )
		{
			ch->println( "You see glowing red eyes watching YOU!" );
		}
	}
	
	return;
}


/**************************************************************************/
// Kalahn - June 98
void do_peek(char_data *ch, char *argument)
{
	char_data *victim;
	char arg[MIL];
	
	bool peeking; 
	
	argument = one_argument( argument, arg );

	victim = get_char_room( ch, arg );
	
	if (victim)
	{	
		if (victim==ch)
		{
			ch->println( "Use the inventory command to see your inventory." );
		}

		peeking = (!IS_NPC(ch) && (number_percent( ) < get_skill(ch,gsn_peek)));

		// can't peek at IC immortals unless an IC imm yourself
		if (!IS_NPC(victim) && (victim->level>= LEVEL_IMMORTAL))
		{
			peeking= false;
		}

		if (peeking)
		{
			act( "$n looks at you thoroughly.", ch, NULL, victim, TO_VICT    );
			act( "$n looks at $N thoroughly.",  ch, NULL, victim, TO_NOTVICT );
		}
		else
		{
			act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
			act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
		}

		// ic imms always exceed at peeking
		if (!IS_NPC(ch) && ch->level>= LEVEL_IMMORTAL)
		{
			peeking= true;
		}

		ch->printlnf( "You peek at the inventory of %s:`=\xab",
			PERS(victim,ch));
		check_improve(ch,gsn_peek,true,4);

		if (peeking){
			show_list_to_char( victim->carrying, ch, "", true, true );
		}else{
			ch->println( "You didn't manage to see anything." );
		}

		ch->print("`x" );
	}
	else
	{
		ch->println( "You can't see them here." );
	}
	return;
}


/**************************************************************************/
bool check_blind( char_data *ch )
{
    if (HAS_HOLYLIGHT(ch))
		return true;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
		ch->println( "You can't see a thing!" );
		return false;
    }
	return true;
}

/**************************************************************************/
void do_consider( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *victim;
    char msg[MIL];
    
    int diffhp;
    int difflevel;
    
    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        ch->println( "Consider killing whom?" );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		// do an object check 
		// get_obj_here() supports uid
		OBJ_DATA *obj= get_obj_here( ch, arg );
		if ( obj ){
			act( "You can't fight $p.", ch, obj, NULL, TO_CHAR );
			return;
		}

        ch->println( "They're not here." );
        return;
    }

    if (is_safe(ch,victim))
	{
        ch->println( "Don't even think about it." );
		return;
	}

    if ( victim == ch )
	{
		ch->println( "Why are you considering hurting yourself?" );
		return;
	}


    diffhp = number_range(victim->hit* 7/10, victim->hit* 13/10) - 
             number_range(ch->hit* 8/10, ch->hit* 12/10);
    

         if ( diffhp <= -100 ) strcpy(msg, "$N looks tiny in comparison ");
    else if ( diffhp <=  -50 ) strcpy(msg, "$N is much smaller than you ");
    else if ( diffhp <=  -20 ) strcpy(msg, "$N is smaller than you ");
    else if ( diffhp <=   10 ) strcpy(msg, "$N is about your size ");
    else if ( diffhp <=   40 ) strcpy(msg, "$N is larger than you ");
    else if ( diffhp <=   90 ) strcpy(msg, "$N is quite larger than you ");
         else                  strcpy(msg, "$N is huge in comparison ");

    if (IS_NPC(victim) && !IS_CONTROLLED(victim)){
		difflevel = victim->level - ch->level;
		difflevel = number_range(difflevel-2, difflevel+2); 
    
			 if ( difflevel <=  -11 ) strcat(msg, "and \r\n$E could be snapped like a twig.");
		else if ( difflevel <=  -8 ) strcat(msg, "and \r\n$E looks like an easy kill. "); 
		else if ( difflevel <=  -5 ) strcat(msg, "and \r\n$E could barely offer any resistance. "); 
		else if ( difflevel <=  -2 ) strcat(msg, "and \r\n$E would give you little trouble. ");
		else if ( difflevel <=   2 ) strcat(msg, "and \r\n$E is a perfect match for you.");
		else if ( difflevel <=   5 ) strcat(msg, "and \r\n$E would put up a good fight.");
		else if ( difflevel <=   8 ) strcat(msg, "and \r\n$E would be difficult to defeat.");
		else if ( difflevel <=  11 ) strcat(msg, "and \r\nyou had better get some help for this one.");      
			 else                  strcat(msg, "and \r\ndeath would welcome the gift of yourself.");
	}
    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}

/**************************************************************************/
/* changes your scroll */
void do_scroll(char_data *ch, char *argument)
{
	char arg[MIL];
	int lines;

	one_argument(argument,arg);

	if (arg[0] == '\0')
	{
		if (ch->lines == 0)
			ch->println( "You do not page long messages." );
		else
		{
			ch->printlnf( "You currently display %d lines per page.", ch->lines + 2);
		}
		return;
	}

	if (!is_number(arg))
	{
		ch->println( "You must provide a number." );
		return;
	}

	lines = atoi(arg);
	
	if (lines == 0)
	{
		ch->println( "Paging disabled." );
		ch->lines = 0;
		return;
	}

	if (lines < 10 || lines > 100)
	{
		ch->println( "You must provide a reasonable number." );
		return;
	}

	ch->printlnf( "Scroll set to %d lines.", lines );
	ch->lines = lines - 2;
}

/**************************************************************************/
void do_motd(char_data *ch, char *)
{
    do_help(ch,"motd");
}

/**************************************************************************/
void do_imotd(char_data *ch, char *)
{
	 do_help(ch,"imotd");
}

/**************************************************************************/
void do_rules(char_data *ch, char *)
{
    do_help(ch,"rules");
}

/**************************************************************************/
void do_story(char_data *ch, char *)
{
    do_help(ch,"story");
}
/**************************************************************************/
void do_autoreformat(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

    if (IS_SET(ch->act,PLR_AUTOREFORMAT))
	{
        ch->println( "Your notes will no longer be automatically reformatted when posting." );
        REMOVE_BIT(ch->act,PLR_AUTOREFORMAT);
	}
	else
	{
        ch->println( "Your notes will now be automatically reformatted when posting." );
        SET_BIT(ch->act,PLR_AUTOREFORMAT);
	}
}
/**************************************************************************/
void do_autowraptells(char_data *ch, char *)
{
    if (HAS_CONFIG(ch, CONFIG_AUTOWRAPTELLS))
    {
		ch->println( "Tells you send and receive will no longer be "
			"automatically wordwraped." );
		REMOVE_CONFIG(ch, CONFIG_AUTOWRAPTELLS);
    }
    else
    {
		ch->println( "Tells you send and receive will now be "
			"automatically wordwraped." );
		SET_CONFIG(ch, CONFIG_AUTOWRAPTELLS);
    }
}

/**************************************************************************/
void do_autoself(char_data *ch, char *argument)
{
	char_data *target=TRUE_CH(ch);

	char arg[MIL];
	one_argument(argument, arg);

	if(IS_NULLSTR(arg)){
		if (IS_SET(target->comm,COMM_AUTOSELF))
		{
			ch->println( "You can refer to yourself with your name, description and the keyword 'self'." );
			REMOVE_BIT(target->comm,COMM_AUTOSELF);
		}
		else
		{
			ch->println( "You can only refer to yourself with the word 'self'." );
			SET_BIT(target->comm,COMM_AUTOSELF);
		}
	}else{
		if(!str_cmp(arg,"on")){
			SET_BIT(target->comm,COMM_AUTOSELF);
			ch->println( "Autoself is now ON." );
		}
		if(!str_cmp(arg,"off")){
			REMOVE_BIT(target->comm,COMM_AUTOSELF);
			ch->println( "Autoself is now OFF." );
		}
	}
}

/**************************************************************************/
void do_autowizilogin(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch, CONFIG2_AUTOWIZILOGIN)){
		ch->println( "You wizi status will be unmodified during login." );
		REMOVE_CONFIG2(ch, CONFIG2_AUTOWIZILOGIN);
    }else{
		ch->println( "You will automatically be made wizi on login.");
		SET_CONFIG2(ch, CONFIG2_AUTOWIZILOGIN);
    }
}
/**************************************************************************/
void do_autokeepalive(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch, CONFIG2_AUTOKEEPALIVE)){
		ch->println( "Autokeepalive turned off." );
		REMOVE_CONFIG2(ch, CONFIG2_AUTOKEEPALIVE);
    }else{
		ch->println( "Autokeepalive turned on.");
		SET_CONFIG2(ch, CONFIG2_AUTOKEEPALIVE);
    }

	ch->titlebar("ABOUT AUTOKEEPALIVE");
	ch->wrapln("Autokeepalive is a system which can in some cases "
		"work around some ADSL/Cable routers which 'forget' about "
		"a connection after a certain time of inactivity - say "
		"30 minutes.  With this option turned on, the mud will send "
		"to your mud client a 'blank message' every minute if you "
		"have been inactive for more than 5 minutes... this should keep "
		"the connection active in the view of the ADSL/Cable router. "
		"Some mud clients will incorrectly display 2 characters (one "
		"looks like a 'y' with a horizontal colon on top, and the other an 'n' "
		"with a tilde (`-) on top.  Regardless of if you see this or "
		"not, the system will in some cases keep your connection alive.  "
		"The system is entirely optional, and will NOT prevent your "
		"character being automatically logged out for inactivity if the "
		"mud is configured to do so.");
	ch->titlebar("");
}

/**************************************************************************/
void do_mp_trigger_in_room(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch, CONFIG2_MP_TRIGGER_IN_ROOM)){
		ch->println( "mudprog_triggers_in_room turned off." );
		REMOVE_CONFIG2(ch, CONFIG2_MP_TRIGGER_IN_ROOM);
    }else{
		ch->println( "mudprog_triggers_in_room turned on.");
		SET_CONFIG2(ch, CONFIG2_MP_TRIGGER_IN_ROOM);
    }

	ch->titlebar("ABOUT MP_TRIGGER_IN_ROOM");
	ch->wrapln("MP_TRIGGER_IN_ROOM when on, tells you when any "
		"mobprog triggers within the same room as you are.  It is useful "
		"to debug mobprogs.  The mptrace command provides a historical "
		"view of similar information.  You must have olc security in "
		"the area to see the triggering.  The command doesn't work while switched.");
	ch->titlebar("");
}

/**************************************************************************/
void do_autowhoinvislogin(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch, CONFIG2_AUTOWHOINVISLOGIN))
    {
		ch->println( "You whoinvis status will be unmodified during login." );
		REMOVE_CONFIG2(ch, CONFIG2_AUTOWHOINVISLOGIN);
    }else{
		ch->println( "You will automatically be made wizi on login.");
		SET_CONFIG2(ch, CONFIG2_AUTOWHOINVISLOGIN);
    }
}

/**************************************************************************/
void do_autolist(char_data *ch, char *)
{
	char *alon="`CON`x";
	char *aloff="`cOFF`x";
	
	if (IS_NPC(ch)){
		ch->println("autolist: players only.");
		return;
	}
	
	ch->println( "   action     status"  );
	ch->println( "`=j---------------------`x" );
	
	ch->printlnf( "autoanswer     %s", !HAS_CONFIG(ch, CONFIG_NOAUTOANSWER)?alon:aloff);	
	ch->printlnf( "autoassist     %s", IS_SET(ch->act,PLR_AUTOASSIST)?alon:aloff);	
	if(!GAMESETTING2(GAMESET2_NO_AUTODAMAGE_COMMAND)){
		ch->printlnf( "autodamage     %s", HAS_CONFIG2(ch, CONFIG2_AUTODAMAGE)?alon:aloff);
	}
	ch->printlnf( "autoexamine    %s", HAS_CONFIG(ch, CONFIG_AUTOEXAMINE)?alon:aloff);	
	ch->printlnf( "autoexit       %s", IS_SET(ch->act,PLR_AUTOEXIT)?alon:aloff);	
	ch->printlnf( "autogold       %s", IS_SET(ch->act,PLR_AUTOGOLD)?alon:aloff);	
	ch->printlnf( "autokeepalive  %s", HAS_CONFIG2(ch, CONFIG2_AUTOKEEPALIVE)?alon:aloff);	
	ch->printlnf( "autolandonrest %s", HAS_CONFIG(ch, CONFIG_AUTOLANDONREST)?alon:aloff);	
	ch->printlnf( "autoloot       %s", IS_SET(ch->act,PLR_AUTOLOOT)?alon:aloff);
	ch->printlnf( "automap        %s", HAS_CONFIG(ch, CONFIG_AUTOMAP )?alon:aloff);	
	ch->printlnf( "autopeek       %s", IS_SET(ch->act,PLR_AUTOPEEK)?alon:aloff);	
	ch->printlnf( "autopkassist   %s", IS_SET(ch->dyn,DYN_AUTOPKASSIST)?alon:aloff);	
	ch->printlnf( "autorecall     %s", HAS_CONFIG(ch,CONFIG_AUTORECALL)?alon:aloff);	
	ch->printlnf( "autoreformat   %s", IS_SET(ch->act,PLR_AUTOREFORMAT)?alon:aloff);	
	ch->printlnf( "autosaycolourc %s", HAS_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES)?alon:aloff);	
	ch->printlnf( "autosaymote    %s", HAS_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE)?aloff:alon);	
	ch->printlnf( "autosplit      %s", IS_SET(ch->act,PLR_AUTOSPLIT)?alon:aloff);	
	ch->printlnf( "autosubdue     %s", IS_SET(ch->act,PLR_AUTOSUBDUE)?alon:aloff);	
	ch->printlnf( "autotrack      %s", HAS_CONFIG(ch, CONFIG_AUTOTRACK)?alon:aloff);	
	ch->printlnf( "autowraptells  %s", HAS_CONFIG(ch, CONFIG_AUTOWRAPTELLS)?alon:aloff);	
	ch->printlnf( "battlelag      %s", !HAS_CONFIG2(ch,CONFIG2_NO_BATTLELAG_PROMPT)?alon:aloff);	
	ch->printlnf( "combine items  %s", IS_SET(ch->comm,COMM_COMBINE)?alon:aloff);	
	ch->printlnf( "compact mode   %s", IS_SET(ch->comm,COMM_COMPACT)?alon:aloff);	
	if ( ch->level > 89 ){
		ch->printlnf( "hero messages  %s", HAS_CONFIG(ch,CONFIG_NOHEROMSG)?alon:aloff);
	}
	ch->printlnf( "nonames        %s", HAS_CONFIG(ch, CONFIG_NONAMES)?alon:aloff);	
	ch->printlnf( "noteach        %s", IS_SET(ch->act,PLR_NOTEACH)?alon:aloff);	
	ch->printlnf( "prompt         %s", IS_SET(ch->comm,COMM_PROMPT)?alon:aloff);	
	ch->printlnf( "specify self   %s  (to toggle type autoself)", HAS_AUTOSELF(ch)?alon:aloff);	

	if(IS_IMMORTAL(ch)){
		ch->printlnf( "autowizilogin      %s", HAS_CONFIG2(ch,CONFIG2_AUTOWIZILOGIN)?alon:aloff);
		ch->printlnf( "autowhoinvislogin  %s", HAS_CONFIG2(ch,CONFIG2_AUTOWHOINVISLOGIN)?alon:aloff);
		ch->printlnf( "mp_trigger_in_room %s", HAS_CONFIG2(ch,CONFIG2_MP_TRIGGER_IN_ROOM)?alon:aloff);
		
	}

	if (IS_UNSWITCHED_MOB(ch) && ch->desc){
		if(ch->desc->colour_mode==CT_NOCOLOUR){
			ch->println( "You are playing in monochrome");	
		}else{
			ch->println( "You are playing in `?colour`x");	
		}
	}

	ch->printlnf( "You can%s be summoned.", IS_SET(ch->act,PLR_NOSUMMON)?"not":"");
	ch->printlnf( "You can%s be charmed.",  HAS_CONFIG(ch,CONFIG_NOCHARM)?"not":"");
	ch->printlnf( "You %saccept followers.", IS_SET(ch->act,PLR_NOFOLLOW)?"do not ":"");
	
	if(HAS_CONFIG(ch, CONFIG_SHOWMISC)){
		ch->println( "Misc notes are shown in unread." );
	}else{
		ch->println( "Details of unread misc notes are not shown in unread." );
	}

	ch->printlnf( "You have misc note reading %sbled.", HAS_CONFIG(ch,CONFIG_NOMISC)?"disa":"ena");
	ch->printlnf( "You are speaking %s.", ch->language->name );
	
    if (ch->seeks)
    {
        ch->printlnf( "You are currently seeking the clan '%s'",
            ch->seeks->name());
    }
}

/**************************************************************************/
void do_name_b4short( char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}

    if ( HAS_CONFIG(ch,CONFIG_NAMES_BEFORE_SHORT))
    {
        REMOVE_CONFIG(ch,CONFIG_NAMES_BEFORE_SHORT);
		ch->println("Known names are now displayed after short descriptions.");
    }
    else
    {
        SET_CONFIG(ch,CONFIG_NAMES_BEFORE_SHORT);
		ch->println("Known names are now displayed before short descriptions.");
    }
    return;
}
/**************************************************************************/
void do_name_only_4known( char_data *ch, char *)
{	
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}

    if ( HAS_CONFIG2(ch,CONFIG2_NAME_ONLY_FOR_KNOWN)){
		ch->println("If you know someones name, you will see both their name and short description.");
        REMOVE_CONFIG2(ch,CONFIG2_NAME_ONLY_FOR_KNOWN);
    }else{
		ch->println("If you know someones name, you wont see their short description.");
        SET_CONFIG2(ch,CONFIG2_NAME_ONLY_FOR_KNOWN);
    }
    return;
}

/**************************************************************************/
void do_autoassist(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOASSIST))
	{
		ch->println( "Autoassist removed." );
		REMOVE_BIT(ch->act,PLR_AUTOASSIST);
	}
	else
	{
		ch->println( "You will now assist when needed." );
		SET_BIT(ch->act,PLR_AUTOASSIST);
	}
}

/**************************************************************************/
void do_autopkassist(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->dyn,DYN_AUTOPKASSIST))
	{
		ch->println( "Autopkassist removed." );
		REMOVE_BIT(ch->dyn,DYN_AUTOPKASSIST);
	}
	else
	{
		ch->println( "You will now assist in pk fights when needed." );
		SET_BIT(ch->dyn,DYN_AUTOPKASSIST);
	}
}


/**************************************************************************/
void do_autoexit(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOEXIT))
	{
		ch->println( "Exits will no longer be displayed." );
		REMOVE_BIT(ch->act,PLR_AUTOEXIT);
	}
	else
	{
		ch->println( "Exits will now be displayed." );
		ch->println( "note: use `=Cfullexits`x to have the name of each exit displayed." );
		SET_BIT(ch->act,PLR_AUTOEXIT);
	}
}
/**************************************************************************/
void do_fullexits(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}

    if(HAS_CONFIG2(ch,CONFIG2_FULL_EXITS)){
		ch->println( "Full exit info will no longer be shown automatically.");
		REMOVE_CONFIG2(ch,CONFIG2_FULL_EXITS);
	}else{
		ch->println( "Full exit info will now automatically be shown.");
		SET_CONFIG2(ch,CONFIG2_FULL_EXITS);
    }
}
/**************************************************************************/
void do_detect_oldstyle_note_writing(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}

    if(HAS_CONFIG2(ch,CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING)){
		ch->println( "The note system will now automatically detect the use of {} "
			"and convert it to a newline code.");
		REMOVE_CONFIG2(ch,CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING);
	}else{
		ch->println( "The note system will no longer automatically detect the use "
			"of {} and convert it to a newline code.");
		SET_CONFIG2(ch,CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING);
    }
}
/**************************************************************************/
void do_autoexamine(char_data *ch, char *)
{	
	if (HAS_CONFIG(ch, CONFIG_AUTOEXAMINE))
    {
		ch->println( "Autoexamining removed." );
		REMOVE_CONFIG(ch, CONFIG_AUTOEXAMINE);
    }
	else
	{
		ch->println( "Automatic corpse examining set." );
		SET_CONFIG(ch, CONFIG_AUTOEXAMINE);
    }
}
/**************************************************************************/
void do_autolandonrest(char_data *ch, char *)
{	
	if (HAS_CONFIG(ch, CONFIG_AUTOLANDONREST))
    {
		ch->println( "Automatic landing when you rest disabled." );
		REMOVE_CONFIG(ch, CONFIG_AUTOLANDONREST);
    }
	else
	{
		ch->println( "You will automatically land when you rest if flying." );
		SET_CONFIG(ch, CONFIG_AUTOLANDONREST);
    }
}
/**************************************************************************/
// Kal
void do_nonames(char_data *ch, char *)
{	
	if (HAS_CONFIG(ch, CONFIG_NONAMES)){
		ch->println( "You will now see the names players have introduced themselves to you." );
		REMOVE_CONFIG(ch, CONFIG_NONAMES);
    }else{
		ch->println( "You will no longer see playernames beside their short descriptions." );
		SET_CONFIG(ch, CONFIG_NONAMES);
    }
}
/**************************************************************************/
// Kal
void do_autoanswer(char_data *ch, char *)
{	
	if (HAS_CONFIG(ch, CONFIG_NOAUTOANSWER)){
		ch->println( "You will now automatically introduce yourself to someone who asks your name." );
		REMOVE_CONFIG(ch, CONFIG_NOAUTOANSWER);
    }else{
		ch->println( "You will no longer automatically introduce yourself to someone who asks your name." );
		SET_CONFIG(ch, CONFIG_NOAUTOANSWER);
    }
}
/**************************************************************************/
void do_autogold(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOGOLD))
	{
		ch->println( "Autogold removed." );
		REMOVE_BIT(ch->act,PLR_AUTOGOLD);
	}
	else
	{
		ch->println( "Automatic gold looting set." );
		SET_BIT(ch->act,PLR_AUTOGOLD);
	}
}

/**************************************************************************/
void do_autoloot(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOLOOT))
	{
		ch->println( "Autolooting removed." );
		REMOVE_BIT(ch->act,PLR_AUTOLOOT);
	}
	else
	{
		ch->println("Automatic corpse looting set." );
		SET_BIT(ch->act,PLR_AUTOLOOT);
	}
}

/**************************************************************************/
void do_autosubdue(char_data *ch, char *)
{
	// unswitched mobs can't ooc
	if (IS_UNSWITCHED_MOB(ch))
	{
		ch->println( "Players or switched players only." );
		return;
	}
	// link dead players can't ooc (ie can't force ooc from a linkdead)
	if (!TRUE_CH(ch))
	{
		ch->println( "Linkdead players can't use this command." );
        return;
	}
	
    if (IS_SET(TRUE_CH(ch)->act,PLR_AUTOSUBDUE))
    {
        ch->println( "You will fight other players to the death." );
        REMOVE_BIT(TRUE_CH(ch)->act,PLR_AUTOSUBDUE);
    }
    else
    {
        ch->println( "When you fight other players, you will fight to subdue them." );
        SET_BIT(TRUE_CH(ch)->act,PLR_AUTOSUBDUE);
    }
}
/**************************************************************************/
void do_autopeek(char_data *ch, char *)
{
    if (IS_NPC(ch))
	{
        ch->println( "Players only." );
        return;
	}
 
    if (IS_SET(ch->act,PLR_AUTOPEEK))
    {
        ch->println( "You will NOT automatically peek at people when you look at them." );
        REMOVE_BIT(ch->act,PLR_AUTOPEEK);
    }
    else
    {
        ch->println( "You will automatically peek at people when you look at them." );
        SET_BIT(ch->act,PLR_AUTOPEEK);
    }
}

/**************************************************************************/
void do_autorecall(char_data *ch, char *)
{
    if (IS_NPC(ch))
		return;
	
    if (HAS_CONFIG(ch,CONFIG_AUTORECALL))
    {
		ch->println( "You will no longer automatically recall if "
			"linkdead when you are attacked." );
		REMOVE_CONFIG(ch,CONFIG_AUTORECALL);
	}
    else
    {
		ch->println( "You will automatically recall if linkdead when "
			"you are attacked." );
		SET_CONFIG(ch,CONFIG_AUTORECALL);
    }
}

/**************************************************************************/
void do_autosaymote(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}

	char colprefix[3];
	if(ch->colour_prefix==COLOURCODE){
		sprintf(colprefix, "%c%c",ch->colour_prefix,ch->colour_prefix);
	}else{
		sprintf(colprefix, "%c", ch->colour_prefix);
	}
	
	// logic reversed from command name
    if (HAS_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE)){
		ch->println( "Say's will automatically be treated as saymotes.");
		REMOVE_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE);
	}else{
		ch->wrapln( "Say's will no longer automatically be treated as saymotes, "
			"you can use saymote manually to get a saymote.");
		SET_CONFIG2(ch,CONFIG2_NOAUTOSAYMOTE);
    }
	ch->wraplnf("note: You can also use `=Cautosaycolourcodes`x to make your "
		"says automatically insert %s in front of colour codes (so you don't "
		"talk with colour).", colprefix);
}
/**************************************************************************/
void do_autosaycolourcodes(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}
	char colprefix[3];
	if(ch->colour_prefix==COLOURCODE){
		sprintf(colprefix, "%c%c",ch->colour_prefix,ch->colour_prefix);
	}else{
		sprintf(colprefix, "%c", ch->colour_prefix);
	}
	
	// logic reversed from command name
    if (HAS_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES)){
		ch->println( "Say's will now allow colours in them.");
		REMOVE_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES);
	}else{
		ch->wraplnf( "Say's will automatically convert the colour code character (%s) into %s%s "
			"so you will be able to talk colour codes easily.",
			colprefix, colprefix, colprefix);			
		ch->println( "Note: you can bypass this behaviour for individual says by using saymote.");
		SET_CONFIG2(ch,CONFIG2_AUTOSAYCOLOURCODES);
    }
}
/**************************************************************************/
// Kal - June 01
void do_autodamage(char_data *ch, char *)
{
    if (IS_UNSWITCHED_MOB(ch)){
		ch->println("Players only.");
		return;
	}
	
	// logic reversed from command name
    if (HAS_CONFIG2(ch,CONFIG2_AUTODAMAGE)){
		ch->println( "Numeric damage values will no longer be shown during combat.");
		REMOVE_CONFIG2(ch,CONFIG2_AUTODAMAGE);
	}else{
		ch->println( "Numeric damage values will now be shown during combat.");
		SET_CONFIG2(ch,CONFIG2_AUTODAMAGE);
    }
}
/**************************************************************************/
void do_autosplit(char_data *ch, char *)
{
	if(IS_NPC(TRUE_CH(ch))){
		ch->println("players only.");
		return;
	}
	if (IS_SET(TRUE_CH(ch)->act,PLR_AUTOSPLIT))
	{
		ch->println( "Autosplitting removed." );
		REMOVE_BIT(TRUE_CH(ch)->act,PLR_AUTOSPLIT);
	}
	else
	{
		ch->println( "Automatic gold splitting set." );
		SET_BIT(TRUE_CH(ch)->act,PLR_AUTOSPLIT);
	}
}
/**************************************************************************/
void do_brief(char_data *ch, char *)
{
	if(IS_NPC(TRUE_CH(ch))){
		ch->println("players only.");
		return;
	}

	if (IS_SET(TRUE_CH(ch)->comm,COMM_BRIEF)){
		ch->println( "Full descriptions activated." );
		REMOVE_BIT(TRUE_CH(ch)->comm,COMM_BRIEF);
	}else{
		ch->println( "Short descriptions activated." );
		SET_BIT(TRUE_CH(ch)->comm,COMM_BRIEF);
	}
}

/**************************************************************************/
void do_compact(char_data *ch, char *)
{
	if(IS_NPC(TRUE_CH(ch))){
		ch->println("players only.");
		return;
	}

	if (IS_SET(TRUE_CH(ch)->comm,COMM_COMPACT)){
		ch->println( "Compact mode removed." );
		REMOVE_BIT(TRUE_CH(ch)->comm,COMM_COMPACT);
	}else{
		ch->println( "Compact mode set." );
		SET_BIT(TRUE_CH(ch)->comm,COMM_COMPACT);
	}
}

/**************************************************************************/
void do_showaffects(char_data *ch, char *)
{
	if(IS_NPC(TRUE_CH(ch))){
		ch->println("players only.");
		return;
	}

	if (IS_SET(TRUE_CH(ch)->comm,COMM_SHOW_AFFECTS)){
		ch->println( "Affects will no longer be shown in score." );
		REMOVE_BIT(TRUE_CH(ch)->comm,COMM_SHOW_AFFECTS);
	}else{
		ch->println( "Affects will now be shown in score." );
		SET_BIT(TRUE_CH(ch)->comm,COMM_SHOW_AFFECTS);
	}
}
/**************************************************************************/
void do_battlelag(char_data *ch, char *argument)
{
	if(!TRUE_CH_PCDATA(ch)){
		ch->println("Only players or controlled mobs can use the battlelag command.");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->wrapln("Battlelag text (when set), is displayed in front of "
			"your prompt when you are currently affected by combat lag.");
		ch->println("syntax: battlelag <lagtext/symbols>    (to set/change the current battle lag text)");
		ch->println("syntax: battlelag off   (to turn off battle lag)");
		ch->println("syntax: battlelag on    (to enable the displaying of battlelag text)");
		ch->println("syntax: battlelag clear (to clear your battle lag text and use mudwide default)");

		if(IS_NULLSTR(TRUE_CH_PCDATA(ch)->battlelag)){
			ch->printlnf("You are currently using the mudwide default battle lag text of '%s'",
				game_settings->mud_default_battlelag_text);
		}else{
			ch->printlnf("You are currently have your battle lag text set to '%s'",
				TRUE_CH_PCDATA(ch)->battlelag);
		}
		return;
	}

	if(!str_cmp(argument, "on")){
		ch->println("Battle lag prompt enabled.");
		REMOVE_CONFIG2(ch, CONFIG2_NO_BATTLELAG_PROMPT);
		return;
	}

	if(!str_cmp(argument, "off")){
		ch->println("Battle lag prompt disabled.");
		SET_CONFIG2(ch, CONFIG2_NO_BATTLELAG_PROMPT);
		return;
	}

	if(!str_cmp(argument, "clear")){
		argument="";
	}

	ch->printlnf("Battle lag prompt changed from '%s' to '%s'",
		TRUE_CH_PCDATA(ch)->battlelag,
		argument);
	replace_string(TRUE_CH_PCDATA(ch)->battlelag, argument);
}
/**************************************************************************/
void do_prompt(char_data *ch, char *argument)
{
	char buf[MSL];

	if (IS_NPC(ch))
	{
		ch->println( "Players only." );
		return;	
	}

	if ( IS_NULLSTR(argument))
	{
		if (IS_SET(ch->comm,COMM_PROMPT))
		{
			if (ch->desc->editor) // working in olc
			{
				if (IS_NULLSTR(ch->olcprompt))
				{
					sprintf(buf,"Your OLCprompt was not defined... "
						"using your normal prompt\r\n%s\r\n",ch->prompt );
				}
				else
				{
					sprintf(buf,"Your OLCprompt was '%s'\r\n",ch->olcprompt );
				}

			}
			else
			{
				if(IS_NULLSTR(ch->prompt)){
					sprintf(buf,"Your prompt was the system wide default (%s)\r\n", 
						game_settings->default_prompt);
				}else{
					sprintf(buf,"Your prompt was '%s'\r\n", ch->prompt);
				}
			}
			ch->printbw(buf);


      	    ch->println( "You will no longer see prompts." );
      	    REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
    	else
    	{
			ch->println( "You will now see prompts." );
			if (ch->desc->editor) // working in olc
			{
				if (IS_NULLSTR(ch->olcprompt))
				{
					sprintf(buf,"Your current OLCprompt is not defined... "
						"using your normal prompt\r\n%s\r\n",ch->prompt );
				}
				else
				{
					sprintf(buf,"Your current OLCprompt is %s\r\n",ch->olcprompt );
				}

			}
			else
			{
				if(IS_NULLSTR(ch->prompt)){
					sprintf(buf,"Your current prompt is the system wide default (%s)\r\n", 
						game_settings->default_prompt);
				}else{
				sprintf(buf,"Your current prompt is %s\r\n",ch->prompt );
			}
			}
			ch->printbw(buf);

			SET_BIT(ch->comm,COMM_PROMPT);
    	}
		ch->println("Note: you can change your group prompt using `=Cgprompt`x");
		return;
	}

    if( !strcmp( argument, "basic" ) )
        strcpy( buf, "[%hhp %mm %vmv> ");
	else if( !strcmp( argument, "all" ) )
		strcpy( buf, "[%h/%Hhp %m/%Mm %vmv %Xxp> ");
    else if( !strcmp( argument, "all1" ) )
        strcpy( buf, "[%e %t %h/%Hhp %m/%Mm %vmv> ");
    else if( !strcmp( argument, "all2" ) )
        strcpy( buf, "[%e %t %h/%Hhp %m/%Mm %vmv %Xxp> ");
    else if( !strcmp( argument, "all2c" ) )
        strcpy( buf, "`g[`Y%e`g %t `R%h/%Hhp `B%m/%Mm `m%vmv `g%Xxp> ");
    else if( !strcmp( argument, "all3" ) )
        strcpy( buf, "[%e %t %l %gg %h/%Hhp %m/%Mm %vmv %Xxp%d> ");
    else if( !strcmp( argument, "all3c" ) )
        strcpy( buf, "`g[`Y%e`g %t `x%l `y%gg `R%h/%Hhp `B%m/%Mm `m%vmv `g%Xxp%d> ");
    else if( !strcmp( argument, "all4" ) )
        strcpy( buf, "[%e %t %l %gg %ss %h/%Hhp %m/%Mm %vmv %Xxp%d> ");
    else if( !strcmp( argument, "all4c" ) )
        strcpy( buf, "`g[`Y%e`g %t `x%l `y%gg `s%ss `R%h/%Hhp `B%m/%Mm `m%vmv `g%Xxp%d> ");
    else if( !strcmp( argument, "all5" ) )
        strcpy( buf, "[%e %t %l %gg %ss %h/%Hhp %m/%Mm %v/%Vmv %Xxp> ");
    else if( !strcmp( argument, "all5c" ) )
        strcpy( buf, "`g[`Y%e`g %t `x%l `y%gg `s%ss `R%h/%Hhp `B%m/%Mm `m%v/%Vmv `g%Xxp%d> ");
    else if( !strcmp( argument, "all6" ) )
        strcpy( buf, "[%e %t %l %gg %ss %h/%Hhp %m/%Mm %v/%Vmv %Xxp%d> ");
    else if( !strcmp( argument, "all6c" ) )
        strcpy( buf, "`g[`Y%e`g %t `x%l `y%gg `s%ss `R%h/%Hhp `B%m/%Mm `M%v/%Vmv `G%Xxp%d> ");
    else if( !strcmp( argument, "all6c2" ) )
        strcpy( buf, "`g[`Y%e`g %t `x%l `Y%g`yg `s%s`ws `R%h/`r%Hhp `B%m/`b%Mm `M%v/`m%Vmv `G%X`gxp%d> ");
    else if( !strcmp( argument, "default" ) )
        buf[0]='\0';
    else if( !strcmp( argument, "olc" ) ){
		if (ch->desc->editor){  // in olc editor
			strcpy( buf, "[`#`m%e`^ in `R%o`mv`R%O`g%Z`^ - %T`^ - %t%d>");
		}else{ // not in olc
			strcpy( buf, "[`#`m%e`^ in `mv`R%R `g%Z`^ - %T`^ - %t%d>");
		}

	}else if( !strcmp( argument, "olc2" ) ){
		if (ch->desc->editor){  // in olc editor
			strcpy( buf, "[`#`m%e`^ in `R%o`mv`R%O`g%Z`^ - %T`^ - %t - %d>");
		}else{ // not in olc
			strcpy( buf, "[`#`m%e`^ in `mv`R%R `g%Z`^ - %T`^ - %t - %d>");
		}

	}
	else if( !strcmp( argument, "build1" ) )
        strcpy( buf, "[--= %hhp %mm %vmv %R %z =-->");
    else if( !strcmp( argument, "build2" ) )
        strcpy( buf, "[--= %hhp %mm %vmv %R %z(%Z) %e =-->");
	else
	{
		if ( str_len(argument) > 160 ){
			argument[160] = '\0';
		}
		strcpy( buf, argument );
		smash_tilde( buf );
		if (str_suffix("%c",buf)){
			strcat(buf," ");
		}
	}

	if (ch->desc->editor) // working in olc - set the olc prompt
	{		
		replace_string(ch->olcprompt, buf);
		sprintf(buf,"OLCprompt set to '%s'\r\n",ch->olcprompt );
	}else{		
		replace_string(ch->prompt, buf);
		if(IS_NULLSTR(ch->prompt)){
			sprintf(buf,"Prompt cleared, now using the system default prompt of '%s'\r\n",
				game_settings->default_prompt);
		}else{
			sprintf(buf,"Prompt set to '%s'\r\n",ch->prompt );
		}
	}
	ch->printbw(buf);
	return;
}
/**************************************************************************/
// GroupPrompt - Kal, Dec 2001
void do_gprompt(char_data *ch, char *argument)
{
	char buf[MSL];

	if (IS_NPC(ch))
	{
		ch->println("Players only.");
		return;	
	}

	if ( IS_NULLSTR(argument) )
	{
		if(!IS_SET(ch->comm,COMM_NOGPROMPT))
		{
			sprintf(buf,"Your group prompt was '%s'\r\n",ch->gprompt );
			ch->printbw(buf);
			
			ch->println( "You will no longer see group prompts." );
			SET_BIT(ch->comm,COMM_NOGPROMPT);
		}
		else
		{
			ch->println( "You will now see group prompts." );
			sprintf(buf,"Your current group prompt is '%s'\r\n",ch->gprompt );
			ch->printbw(buf);
			
			REMOVE_BIT(ch->comm,COMM_NOGPROMPT);
		}

		ch->printlnf("Type `=C%s`x for help on the group prompt %% codes.",
			mxp_create_send(ch, "gprompt help"));
		return;
	}

	if( !str_cmp(argument,"help") )
	{
		ch->titlebar("GROUP PROMPT PERCENTAGE CODES");
		ch->println(
			"`1  %g - begin group section"
			"`1  %G - end group section"
			"`1  %h - lowest hitpoints % for group members in the room"
			"`1  %m - lowest mana % for group members in the room"
			"`1  %v - lowest move % for group members in the room"
			"`1  %p - begin pet section"
			"`1  %P - end pet section"
			"`1  %q - pet hitpoints %"
			"`1  %r - pet mana %"
			"`1  %s - pet move %" 
			"`1  %N - number of group members in the current room"
			"`1  %c - carriage return"
			"`1  %C - carriage return only if there is preceeding text"
			"`1  %x - number of charmies in the current room (excluding pet)"
			"`1");
		ch->println("Anything between %p and %P is 'eaten' when you don't have a pet in the room.");
		ch->println("Anything between %g and %G is 'eaten' when you don't have a group member in the room.");
		ch->println("Group prompts only work on pets and other players - not charmies (except %x).");
		ch->println("The default group prompt is:");
		ch->printlnbw("  '`#%g[`xgrp `R%hhp `B%mm `M%vmv`&]%G%p[`spet `r%qhp `b%rm `m%smv`&>%P%c'");
		ch->titlebar("");
		return;
	}

	
	{
		if ( str_len(argument) > 160 ){
			argument[160] = '\0';
		}
		strcpy( buf, argument );
		smash_tilde( buf );
		if (str_suffix("%c",buf)){
			strcat(buf," ");
		}
	}
	
	ch->printfbw("Prompt changed from '%s' to '%s'\r\n",ch->gprompt, buf );
	replace_string( ch->gprompt, buf );
	REMOVE_BIT(ch->comm,COMM_NOGPROMPT);
	return;
}
/**************************************************************************/
void do_combine(char_data *ch, char *)
{
	if (IS_SET(ch->comm,COMM_COMBINE))
	{
		ch->println( "Long inventory selected." );
		REMOVE_BIT(ch->comm,COMM_COMBINE);
	}
	else
	{
		ch->println( "Combined inventory selected." );
		SET_BIT(ch->comm,COMM_COMBINE);
	}
}

/**************************************************************************/
void do_nofollow(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	// Check to see if ch is charmed and being ordered to cast
	if ( IS_AFFECTED(ch,AFF_CHARM) && !IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		ch->println( "You must wait for your master to tell you to do that." );
		return;
	}

	if (IS_SET(ch->act,PLR_NOFOLLOW))
	{
		ch->println( "You now accept followers." );
		REMOVE_BIT(ch->act,PLR_NOFOLLOW);
	}
	else
	{
		ch->println( "You no longer accept followers." );
		SET_BIT(ch->act,PLR_NOFOLLOW);
		die_follower( ch );
	}
}

/**************************************************************************/
void do_noteach(char_data *ch, char *)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_NOTEACH))
    {
		ch->println( "You now can be taught." );
		REMOVE_BIT(ch->act,PLR_NOTEACH);
	}
	else
	{
		ch->println( "You no longer can be taught.");
		SET_BIT(ch->act,PLR_NOTEACH);
	}
}

/**************************************************************************/
void do_nosummon(char_data *ch, char *)
{
	if (IS_NPC(ch))
	{
		if (IS_SET(ch->imm_flags,IMM_SUMMON))
		{
			ch->println( "You are no longer immune to summon." );
			REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
		}
		else
		{
			ch->println( "You are now immune to summoning." );
			SET_BIT(ch->imm_flags,IMM_SUMMON);
		}
	}
	else
	{
		if (IS_SET(ch->act,PLR_NOSUMMON))
		{
			ch->println( "You are no longer immune to summon." );
			REMOVE_BIT(ch->act,PLR_NOSUMMON);
		}
		else
		{
			ch->println( "You are now immune to summoning." );
			SET_BIT(ch->act,PLR_NOSUMMON);
		}
	}
}

/**************************************************************************/
void do_nocharm(char_data *ch, char *)
{
    if (IS_NPC(ch))
    {
		if (IS_SET(ch->imm_flags,IMM_CHARM))
		{
			ch->println( "You are no longer immune to charm." );
			REMOVE_BIT(ch->imm_flags,IMM_CHARM);
		}
		else
		{
			ch->println( "You are now immune to charming." );
			SET_BIT(ch->imm_flags,IMM_CHARM);
		}
    }
    else
    {
		if(GAMESETTING2(GAMESET2_NOCHARM_HAS_NOAFFECT)){
			ch->println("NOTE: This setting has no affect while the affects of nocharm have been disabled in the gamesettings.");
		}
		if (HAS_CONFIG(ch,CONFIG_NOCHARM))
		{
			ch->println( "You are no longer immune to charm." );
			REMOVE_CONFIG(ch,CONFIG_NOCHARM);
		}
		else
		{
			ch->println( "You are now immune to charming." );
			SET_CONFIG(ch,CONFIG_NOCHARM);
		}
    }
}

/**************************************************************************/
void do_glance( char_data *ch, char *argument )
{
	char_data *victim;
	char buf[MSL];
	int percent;

	if(IS_NULLSTR(argument)){
		ch->println( "Glance at whom?" );
		return;
	}

	victim = get_char_room( ch, argument );

	if(!victim){
		ch->printlnf( "You can't seem to see any '%s' in the room.", argument);
		return;
	}

	if ( victim->max_hit > 0 )
        percent = ( 100 * victim->hit ) / victim->max_hit;
	else
        percent = -1;
	
	strcpy( buf, "`#`Y");
	strcat( buf, PERS(victim, ch) );
	
	if (percent >= 100)
        strcat( buf, " is in excellent condition.\r\n");
	else if (percent >= 90)
        strcat( buf, " has a few scratches.\r\n");
	else if (percent >= 75)
        strcat( buf," has some small wounds and bruises.\r\n");
	else if (percent >=  50)
        strcat( buf, " has quite a few wounds.\r\n");
	else if (percent >= 30)
        strcat( buf, " has some big nasty wounds and scratches.\r\n");
    else if (percent >= 15)
        strcat ( buf, " looks pretty hurt.\r\n");
    else if (percent >= 0 )
        strcat (buf, " is in `Rawful condition`Y.\r\n");
    else
        strcat(buf, " is `Rbleeding`Y to death.\r\n");
	
    ch->printlnf( "%s`&", capitalize( buf ));

	// flying status
	if ( IS_AFFECTED(victim, AFF_FLYING) ){
		ch->println( "They appear to be airborne." );
	}
}
/**************************************************************************/
void send_zmudmap_to_char(char_data *ch);
void do_lockers( char_data *ch, char *argument );
/**************************************************************************/
// supports uid 
void do_look( char_data *ch, char *argument )
{
	char buf  [MSL];
	char arg1 [MIL];
	char arg2 [MIL];
	char arg3 [MIL];
	EXIT_DATA *pexit;
	char_data *victim;
	OBJ_DATA *obj;
	char *pdesc;
	int door;
	int number,count;
	
	if ( ch->desc == NULL )
		return;
	
	if ( ch->position < POS_SLEEPING )
	{
		ch->println( "You can't see anything but stars!" );
		return;
	}
	
	if ( ch->position == POS_SLEEPING )
	{
		ch->println( "You can't see anything, you're sleeping!" );
		return;
	}
	
	if ( IS_IC(ch) && !check_blind( ch ) )
		return;
	
	if ( !IS_NPC(ch)
		&& !HAS_HOLYLIGHT(ch)
		&& room_is_dark( ch->in_room ) )
	{
		ch->println( "It is pitch black ... " );

		// attempt to display room contents, glowing objects should be seen
		show_list_to_char( ch->in_room->contents, ch, "", false, false );
		show_char_to_char( ch->in_room->people, ch );

        if ( (IS_SET(TRUE_CH(ch)->act, PLR_AUTOEXIT) || HAS_CONFIG2(ch,CONFIG2_FULL_EXITS))
			&& !IS_SET(ch->in_room->room_flags, ROOM_NOAUTOEXITS))
		{
			ch->println( "" );
			if(HAS_CONFIG2(ch,CONFIG2_FULL_EXITS)){
				do_exits( ch, "fullexits" );
			}else{
				do_exits( ch, "auto" );
			}			
		}
		
		if(!IS_SET(ch->in_room->room_flags, ROOM_NOAUTOMAP)){
			if (USING_AUTOMAP(ch)) {
				do_map(ch,"");
    		}
		}
		return;
	}
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	number = number_argument(arg1,arg3);
	count = 0;
	
	if ( IS_NULLSTR(arg1) || !str_cmp( arg1, "auto" ) || !str_cmp( arg1, "brief" ))
	{
		if (IS_SET(ch->dyn,DYN_MAPCLEAR) && USING_AUTOMAP(ch)){			        
			do_map(ch,"");
    	}
		// 'look' or 'look auto'
		ch->printf( "`=B%s`x", mxp_create_tag(ch, "RName", ch->in_room->name));

		if(!GAMESETTING2(GAMESET2_HIDE_AREA_SHORTNAMES) 
			&& !IS_NULLSTR(ch->in_room->area->short_name))
		{
			ch->printf(" `#[%s%s`^]", 
				colour_table[(ch->in_room->area->vnum%14)+1].code,
				ch->in_room->area->short_name);
		}
		
		if ( (IS_IMMORTAL(ch) && HAS_HOLYLIGHT(ch))
			|| (IS_SET(ch->comm, COMM_BUILDING) && ch->in_room 
			&& IS_BUILDER(ch, ch->in_room->area, BUILDRESTRICT_ROOMS)) )
		{ 
			ch->printf( " [Room %d]",ch->in_room->vnum );
		}
		
		if (IS_OOC(ch)){
			ch->print(" `=^(OOC ROOM)" );
		}
		
		if (IS_OLCAREA(ch->in_room->area)){
			ch->print( " `=&(OLC AREA)" );
		}

		ch->println( "`x" ); 
		
		if ( IS_NULLSTR(arg1) 
			|| !IS_SET(TRUE_CH(ch)->comm,COMM_BRIEF)){
			if ( ch->desc && !ch->desc->speedwalk_buf && str_cmp( arg1, "brief" )){   
				ch->printf( "`=b  %s",  mxp_create_tag(ch, "RDesc", ch->in_room->description));
			}

		}

        if( (IS_SET(TRUE_CH(ch)->act, PLR_AUTOEXIT) || HAS_CONFIG2(ch,CONFIG2_FULL_EXITS))
			&& !IS_SET(ch->in_room->room_flags, ROOM_NOAUTOEXITS))
		{
			ch->println( "" );
			if(HAS_CONFIG2(ch,CONFIG2_FULL_EXITS)){
				do_exits( ch, "fullexits" );
			}else{
				do_exits( ch, "auto" );
			}			
		}
		
		
		show_list_to_char( ch->in_room->contents, ch, "", false, false );

		// if there are lockers in this room, display them
		if(lockers->room_has_lockers(ch->in_room)){		
			ch->printlnf("     `=\x8d%s", 
				mxp_create_send(ch,"look lockers",
					FORMATF("""This room has %d locker%s.", 
					ch->in_room->lockers->quantity,
					ch->in_room->lockers->quantity==1?"":"s")));
		}

		ch->print( "`=D" );
		show_char_to_char( ch->in_room->people,   ch );

		ch->print( "`x" );

		if(!IS_SET(ch->in_room->room_flags, ROOM_NOAUTOMAP)){
			if (!IS_SET(ch->dyn,DYN_MAPCLEAR) && USING_AUTOMAP(ch)){
				do_map(ch,"");
    		}
		}

		// do track if they arent following anyone and have the skills and arent speedwalking
		if(HAS_CONFIG(ch, CONFIG_AUTOTRACK) && !ch->master ){
			if(get_skill(ch, gsn_fieldtrack)>1 
				|| get_skill(ch, gsn_citytrack)>1)
			{
				if(!(ch->desc && ch->desc->speedwalk_buf)){
					do_tracks(ch,"");
				}
			}
		}
        return;
	}
	
	if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
	{
		// 'look in'
		if ( IS_NULLSTR(arg2) )
		{
			ch->println( "Look in what?" );
			return;
		}

		// get_obj_here() supports uid
		if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
		{
			ch->printlnf( "You do not see any '%s' here to look in.", arg2 );
			return;
		}
		
		switch ( obj->item_type )
		{
		default:
			ch->println( "That is not a container." );
			break;
			
		case ITEM_DRINK_CON:
			if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKIN_PRE))break;

			if ( obj->value[1] <= 0 ){
				ch->println( "It is empty." );
				break;
			}
			
			ch->printlnf( "It's %sfilled with a %s liquid.",
				obj->value[1] <     obj->value[0] / 4 ?
				"less than half-" :	obj->value[1] < 3 * obj->value[0] / 4 ?
				"about half-"     : "more than half-",
				liq_table[obj->value[2]].liq_color );

			oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKIN_POST);
			break;

		case ITEM_PORTAL:
			{
				bool autolook = false;
			    ROOM_INDEX_DATA *location;
				int lvnum;

				if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKIN_PRE))break;

				if (IS_SET(obj->value[2],GATE_OPAQUE)) {
					ch->println( "The portal is opaque, you can't see anything inside it." );
					return;
				};

				if( IS_SET( ch->act,  PLR_AUTOEXIT )) {
					autolook = true;
					REMOVE_BIT( ch->act, PLR_AUTOEXIT );
				}

				if(IS_SET(obj->value[2],GATE_SHORT_LOOKINTO)){
					act( "You look through $p and see", ch, obj, NULL, TO_CHAR );
				}else{
					act( "You gaze through the portal and see:", ch, NULL, NULL, TO_CHAR );
				}

				if(IS_SET(obj->value[1],EX_CLOSED)){
					ch->printlnf("It is closed.  You need to open it first to see.");
					return;
				}

				if (IS_SET(obj->value[2],GATE_RANDOM) || obj->value[3] == -1)
				{
					location = get_random_room(ch);
					lvnum = location->vnum; // for record keeping :)
				}else{
					lvnum = obj->value[3];
				
				}
								
				sprintf( buf, "%d look", lvnum);
				do_at( ch, buf );				
				if( autolook ){
					SET_BIT( ch->act, PLR_AUTOEXIT );
				}
				oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKIN_POST);
			}
			break;
			
		case ITEM_CAULDRON:
		case ITEM_CONTAINER:
		case ITEM_FLASK:
		case ITEM_MORTAR:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKIN_PRE))break;
			if ( IS_SET(obj->value[1], CONT_CLOSED) )
			{
				ch->println( "It is closed." );
				break;
			}						

			act( "$p holds:", ch, obj, NULL, TO_CHAR );
			show_list_to_char( obj->contains, ch, "", true, true );

			oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKIN_POST);
			break;
		}
		return;
    }

	// get_char_room() supports uid
    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
		show_char_to_char_1( victim, ch );
		return;
    }

	// support UID
	int uid=get_uid(arg3);
	if(uid){
		// uid looking at objects in inventory 
		for ( obj = ch->carrying; obj; obj = obj->next_content )
		{		
			if(uid==obj->uid && can_see_obj( ch, obj ) ){
				// parchments are displayed differently 
				if ( obj->item_type == ITEM_PARCHMENT ){
					if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
					letter_read( ch, obj );
					oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
					return;
				}

				// take the first name field, and if it is longer than
				// 2 characters, see if we can find an extended description 
				// that matches it... if so show that instead of the objects 
				// description		
				{
					char firstname[MIL];
					one_argument(obj->name, firstname);

					if(str_len(firstname)>2){
						// use only one of the sets of extended descriptions 
						if (obj->extra_descr)
						{ // unique objects extended descriptions 
							pdesc = get_extra_descr( firstname, obj->extra_descr );
							if( pdesc ){
								if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
								ch->sendpage(pdesc);
								oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
								return;
							}				
						}
						else // vnums extended descriptions
						{        
							pdesc = get_extra_descr( firstname, obj->pIndexData->extra_descr );
							if ( pdesc )
							{
								if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
								ch->sendpage(pdesc);
								oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
								return;
							}
						}
					}
				}

				ch->println( obj->description );
				return;
			}
		}
		// uid looking at objects in room
		for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
		{
			if(uid==obj->uid && can_see_obj( ch, obj ) ){
				// parchments are displayed differently 
				if ( obj->item_type == ITEM_PARCHMENT ){
					if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
					letter_read( ch, obj );
					oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
					return;
				}

				// take the first name field, and if it is longer than
				// 2 characters, see if we can find an extended description 
				// that matches it... if so show that instead of the objects 
				// description		
				{
					char firstname[MIL];
					one_argument(obj->name, firstname);

					if(str_len(firstname)>2){
						// use only one of the sets of extended descriptions 
						if (obj->extra_descr)
						{ // unique objects extended descriptions 
							pdesc = get_extra_descr( firstname, obj->extra_descr );
							if( pdesc ){
								if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
								ch->sendpage(pdesc);
								oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
								return;
							}				
						}
						else // vnums extended descriptions
						{        
							pdesc = get_extra_descr( firstname, obj->pIndexData->extra_descr );
							if ( pdesc )
							{
								if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
								ch->sendpage(pdesc);
								oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
								return;
							}
						}
					}
				}

				if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
				ch->println( obj->description );
				oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
				return;
			}
		}
		ch->println( "You do not see that here." );
		return;
	}

	// support lockers
    if( !str_prefix(arg3, "lockers") ){
		do_lockers(ch, "look");
        return;
    }

	for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) )
        {  // player can see object
			
			// parchments take highest priority
            if ( obj->item_type == ITEM_PARCHMENT && is_name( arg3, obj->name ) )
            {
                if (++count == number)
                {
					if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
					letter_read( ch, obj );
					oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
                    return;
                }
            }
            // use only one of the sets of extended descriptions 
            if (obj->extra_descr)
            { // unique objects extended descriptions 
                pdesc = get_extra_descr( arg3, obj->extra_descr );
                if( pdesc ){
                    if (++count == number){
						if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
                        ch->sendpage(pdesc);
						oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
                        return;
                    }
					continue;
                }				
            }
            else // vnums extended descriptions
            {        
                pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
                if ( pdesc )
                {
                    if (++count == number){ 
						if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;						
                        ch->sendpage(pdesc);
						oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
                        return;
                    }
					continue;
                }
            }
			
            if ( is_name( arg3, obj->name ) )
            {
                if (++count == number){
					if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
                    ch->println( obj->description );
					oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
                    return;
                }
            }
        }
		
    }
	
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            // use only one of the sets of extended descriptions 
            if (obj->extra_descr)
            { // unique objects extended descriptions 				
                pdesc = get_extra_descr( arg3, obj->extra_descr );
                if ( pdesc ){
                    if (++count == number) {
						if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
						ch->sendpage(pdesc);
						oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
						return;
                    }
					continue;
				}
            }
            else // vnums extended descriptions
            {      
                pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
                if ( pdesc != NULL )
                {
                    if (++count == number)
                    {
						if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
                        ch->sendpage(pdesc);
						oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
                        return;
                    }
					continue;
                }
            }
        }
		
        if ( is_name( arg3, obj->name ) )
        {
            if (++count == number){
				if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
                ch->println( obj->description );
				oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
                return;
            }
        }
    }
	
	// match room extended descriptions
    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc )
    {
        if (++count == number){			
			if(oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_PRE))return;
            ch->sendpage(pdesc);
			oprog_execute_if_appropriate(obj, ch, OTRIG_LOOKAT_POST);
            return;
        }
    }
    
	// tell them how many they matched on
    if (count > 0 && count != number)
    {
		if (count == 1){
			ch->printlnf( "You only see one %s here.", arg3 );
		}else{
			ch->printlnf( "You only see %d of those here.", count );
		}
		return;
    }
	

	// as a last resort, try looking at a direction
	door = dir_lookup( arg1 );
	if ( door == -1 ){
		ch->printlnf( "You do not see any '%s' here.", arg1 );
        return;
    }
	
    // 'look direction'
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
        ch->println( "Nothing special there." );
        return;
    }
	
    if ( !IS_NULLSTR(pexit->description)){
        ch->print( pexit->description );
    }else{
        ch->println( "Nothing special there." );
	}
	
    if ( pexit->keyword    != NULL
        && pexit->keyword[0] != '\0'
        && pexit->keyword[0] != ' ' )
    {
        if ( IS_SET(pexit->exit_info, EX_CLOSED) )
        {
            act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
        }
        else
        {
            if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
            {
                act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
            }
        }
    }
    return;
}

/**************************************************************************/
void do_read (char_data *ch, char *argument )
{
    do_look(ch,argument);
}

/**************************************************************************/
// supports uid
void do_examine( char_data *ch, char *argument )
{
	char buf[MSL];
	char arg[MIL];
	OBJ_DATA *obj;
    
	one_argument( argument, arg );
	
	if ( arg[0] == '\0' )
	{
		ch->println( "Examine what?" );
		return;
	}

	do_look( ch, arg );

	if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
		switch ( obj->item_type )
		{
		default:
			break;
		
		case ITEM_JUKEBOX:
			//        do_play(ch,"list");
			break;

		case ITEM_MONEY:
			if (obj->value[0] == 0)
			{
				if (obj->value[1] == 0)
					ch->println( "Odd...there are no coins in the pile." );
				else if (obj->value[1] == 1)
					ch->println( "Wow. One gold coin." );
				else
					ch->printlnf( "There are %d gold coins in the pile.", obj->value[1]);
			}
			else if (obj->value[1] == 0)
			{
				if (obj->value[0] == 1)
					ch->println( "Wow. One silver coin." );
				else
					ch->printlnf( "There are %d silver coins in the pile.", obj->value[0]);
			}
			else
			{
				ch->printlnf( "There are %d gold and %d silver coins in the pile.",
					obj->value[1],obj->value[0]);
			}
			break;

		case ITEM_CAULDRON:
		case ITEM_CONTAINER:
		case ITEM_FLASK:
		case ITEM_MORTAR:		
		case ITEM_DRINK_CON:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			sprintf(buf,"in %s",argument);
			do_look( ch, buf );
		}
	}
    return;
}

/**************************************************************************/
void do_exits( char_data *ch, char *argument )
{
	EXIT_DATA *pexit;
    char buf[MSL];
    bool found;
    bool fAuto;
    int door;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if (fAuto){
        sprintf(buf,"[Exits:");
    }else{
        if (IS_IMMORTAL(ch)){
            sprintf(buf,"Obvious exits from room %d:\r\n",ch->in_room->vnum);
        }else{
            sprintf(buf,"Obvious exits:\r\n");
		}
	}

	if(!str_cmp( argument, "fullexits" )){
		strcat(buf, "`=\xac");
	}

    found = false;
    for ( door = 0; door<MAX_DIR; door++ )
    {
        if ( ( pexit = ch->in_room->exit[door] ) != NULL
            &&   pexit->u1.to_room != NULL
            &&   can_see_room(ch,pexit->u1.to_room) 
            &&   !IS_SET(pexit->exit_info, EX_CLOSED) )
        {
            found = true;
            if ( fAuto ){
                strcat( buf, mxp_create_tagf(ch, "Ex", " %s",dir_name[door]));
            }
            else
            {
                sprintf( buf + str_len(buf), "%-5s - %s",
                    capitalize( dir_name[door] ),
                    (!HAS_HOLYLIGHT(ch)
                     && room_is_dark( pexit->u1.to_room) )
                    ?  "Too dark to tell"
                    : pexit->u1.to_room->name
                    );
            if (IS_IMMORTAL(ch))
                sprintf(buf + str_len(buf), 
                " (room %d)\r\n",pexit->u1.to_room->vnum);
            else
                sprintf(buf + str_len(buf), "\r\n");
            }
        }
    }

    // hidden exits for those with holylight 
    if (HAS_HOLYLIGHT(ch))
    {
        for ( door = 0; door < MAX_DIR; door++ )
        {
    
            if ( ( pexit = ch->in_room->exit[door] ) != NULL
                &&   pexit->u1.to_room != NULL
                &&   can_see_room(ch,pexit->u1.to_room) 
                &&   IS_SET(pexit->exit_info, EX_CLOSED) )
            {
                found = true;
                if ( fAuto )
                {
                    strcat( buf, " (" );
                    strcat( buf, dir_name[door] );
                    strcat( buf, ")" );
                }
                else
                {
                    sprintf( buf + str_len(buf), "(%s) - %s",
                        capitalize( dir_name[door] ),
                        pexit->u1.to_room->name);
                    if (IS_IMMORTAL(ch))
                        sprintf(buf + str_len(buf), 
                            " (room %d)\r\n",pexit->u1.to_room->vnum);
                    else
                        sprintf(buf + str_len(buf), "\r\n");
                }
            }
        }
    }

    if ( !found )
        strcat( buf, fAuto ? " none" : "None." );

    if ( fAuto ){
        strcat( buf, "]");
		strcpy( buf, mxp_create_tag(ch, "RExits", buf ));
	}
    ch->printlnf( "`=a%s`x", buf);
	                
    return;
}

/**************************************************************************/
void do_worth( char_data *ch, char * )
{
    char buf[MSL], message[MSL];

	ch->titlebar("WORTH");

	if (IS_NPC(ch))
	{
		ch->printlnf( "You have %ld gold and %ld silver.",
			ch->gold,ch->silver);
		return;
	}

	ch->printlnf(
		" You have %ld gold, \r\n %ld silver, \r\n %ld Rps, \r\n and %d experience (%d exp to level).",
		ch->gold, ch->silver, ch->pcdata->rp_points, ch->exp,
		(ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
	
	ch->printlnf( " Wimpy set to %d hitpoint%s.", ch->wimpy, ch->wimpy==1?"":"s");
	ch->printlnf( " Panic set to %d hitpoint%s.", ch->pcdata->panic, ch->pcdata->panic==1?"":"s");
	ch->printlnf( " You're currently speaking in %s.", ch->language->name);

	if (HAS_CLASSFLAG( ch, CLASSFLAG_TOTEMS )){
		int month = get_birthmonth( ch );

		if ( month >= 0 )
		{
			ch->printlnf( " Your totemic spirit is the %s.",
			totem_table[month].name );
		}
	}

	if (HAS_CLASSFLAG( ch, CLASSFLAG_DEITIES )){
		if ( ch->deity && ch->deity->name)
		{
			ch->printlnf( " You worship `#`W%s`^.", ch->deity->name );
		}
	}

    if(GET_SECURITY(ch)){
        ch->printlnf( " You have an OLC security rating of %d.", GET_SECURITY(ch));
	}

	if ( IS_IMMORTAL(ch)) {
		ch->printlnf( " Council: %s", flag_string( council_flags, ch->pcdata->council ));
	}

	if (IS_NEWBIE_SUPPORT(ch)){
		ch->println( " You are a newbie support member." );
	}

	if (HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER))
	{
		ch->println( " You are tester of the new prac system." );
	}

	if (IS_NOBLE(ch))
	{
		ch->printlnf(" `MYou are a noble with a diplomacy of `W%d`M and have currently `W%d`M vote%s,\r\n"
			" Your autovote setting is currently at `W%d`M.`x",
			TRUE_CH(ch)->pcdata->diplomacy,
			TRUE_CH(ch)->pcdata->dip_points,
			TRUE_CH(ch)->pcdata->dip_points==1?"":"s",
			TRUE_CH(ch)->pcdata->autovote);
	}
	
	if(HAS_CONFIG(ch, CONFIG_COURTMEMBER))
	{
		ch->println( " You are a member of some court." );
	}

	if (HAS_CONFIG(ch, CONFIG_DISALLOWED_PKILL))
	{
		ch->println( " You have a restriction on you that prevents you from pkilling!" );
	}

	if (IS_SET(ch->comm, COMM_CANNOCHANNEL))
	{
		ch->println( " You can nochannel other players." );
	}

	if(!IS_NPC(ch)){
		ch->printlnf( " Creation points: %d  XPPerLvl: %d",
			ch->pcdata->points,
			exp_per_level(ch, ch->pcdata->points));

		if(IS_SET(ch->act, PLR_QUESTER)){
			ch->println(" You have your quester status enabled.");
		}else{
			ch->println(" You have your quester status disabled.");
		}
		
		if(GAMESETTING(GAMESET_PEACEFUL_MUD)){
			if(HAS_CONFIG(ch, CONFIG_ACTIVE)){
				ch->println(" You are flagged as an active player - but the game is locked to peaceful.");
			}else{
				ch->println(" You are a peaceful player - the game is locked as peaceful.");
			}
		}else{
			if(HAS_CONFIG(ch, CONFIG_ACTIVE)){
				ch->println(" You are an active player.");
			}else{
				ch->println(" You are a peaceful player.");
			}
		}

		if(IS_SET(ch->act, PLR_NOREDUCING_MAXKARN)){
			ch->println(" Your maxkarns is permanently 5.");
		}

		if (HAS_CONFIG(ch, CONFIG_NOTE_ONLY_TO_IMM)){
			ch->println(" You can only write your notes to the immortals.");
		}

		if(ch->pcdata->autoafkafter){
			ch->printlnf(" You automatically go afk after %d minute%s",
				ch->pcdata->autoafkafter,
				ch->pcdata->autoafkafter==1?"":"s");
		}
	}
	

	sprintf(buf, " You are");
	if ( ch->pcdata->tired > 16 ) strcat( buf, " tired," );
	( ch->pcdata->condition[COND_HUNGER] == 0 ) ?
		strcat( buf, " hungry," )	: strcat( buf, " full," );
	( ch->pcdata->condition[COND_DRUNK] == 0 ) ?
		strcat( buf, " sober" )		: strcat( buf, " drunk" );
	( ch->pcdata->condition[COND_THIRST] == 0 ) ?
		strcat( buf, ", thirsty." )	: strcat( buf, "." );
	ch->printlnf( "%s", buf );

    sprintf(buf, " You are currently ");
    if (ch->subdued)
        strcat( buf, "subdued." );
    else
    {
        switch ( ch->position )
		{
        case POS_DEAD:     strcat( buf, "DEAD!!" );              break;
        case POS_MORTAL:   strcat( buf, "mortally wounded." );   break;
        case POS_INCAP:    strcat( buf, "incapacitated." );      break;
        case POS_STUNNED:  strcat( buf, "lying here stunned." ); break;
        case POS_SLEEPING: 
        if (ch->on != NULL)
		{
			if ( IS_SET( ch->on->value[2], SLEEP_AT ))
			{
				sprintf( message, "sleeping at %s.", ch->on->short_descr );
				strcat( buf, message );
			}
			else if ( IS_SET( ch->on->value[2], SLEEP_ON ))
			{
				sprintf( message, "sleeping on %s.", ch->on->short_descr);
				strcat( buf, message );
			}
			else if ( IS_SET( ch->on->value[2], SLEEP_UNDER ))
			{
				sprintf(message, "sleeping under %s.", ch->on->short_descr );
				strcat( buf, message );
			}
			else
			{
				sprintf(message, "sleeping in %s.", ch->on->short_descr);
				strcat(buf,message);
			}
        }
        else 
             strcat(buf,"sleeping.");
        break;

        case POS_RESTING:  
            if ( ch->on != NULL )
            {
                if ( IS_SET( ch->on->value[2], REST_AT ))
                {
                    sprintf( message, "resting at %s.", ch->on->short_descr );
                    strcat( buf, message );
                }
                else if ( IS_SET( ch->on->value[2], REST_ON ))
                {
                    sprintf( message, "resting on %s.", ch->on->short_descr );
					strcat( buf, message );
                }
				else if ( IS_SET( ch->on->value[2], REST_UNDER ))
				{
					sprintf( message, "resting under %s.", ch->on->short_descr );
					strcat( buf, message );
				}
                else 
                {
				sprintf( message, "resting (%d - %d) in %s.", REST_IN, ch->on->value[2], ch->on->short_descr );
			
                    strcat( buf, message );
                }
            }
            else
                strcat( buf, "resting." );

            if (ch->is_trying_sleep){
                strcat( buf, " (You are trying to go to sleep)" );
            }
			break;
			
        case POS_SITTING:
            if ( ch->on != NULL )
            {
                if ( IS_SET( ch->on->value[2], SIT_AT ))
                {
					sprintf( message, "sitting at %s.", ch->on->short_descr );
					strcat( buf, message );
                }
                else if (IS_SET(ch->on->value[2],SIT_ON))
                {
                    sprintf( message, "sitting on %s.", ch->on->short_descr );
                    strcat( buf, message );
                }
				else if ( IS_SET( ch->on->value[2], SIT_UNDER ))
				{
					sprintf( message, "sitting under %s.", ch->on->short_descr );
					strcat( buf, message );
				}
                else
                {
					sprintf( message, "sitting in %s.", ch->on->short_descr );
                    strcat( buf, message );
                }
			}
            else
				strcat( buf, "sitting." );
			break;

		case POS_KNEELING:
            if ( ch->on != NULL )
            {
                if ( IS_SET( ch->on->value[2], KNEEL_AT ))
                {
					sprintf( message, "kneeling at %s.", ch->on->short_descr );
					strcat( buf, message );
				}
                else if ( IS_SET( ch->on->value[2], KNEEL_ON ))
                {
                    sprintf( message, "kneeling on %s.", ch->on->short_descr );
                    strcat( buf, message );
                }
				else if ( IS_SET( ch->on->value[2], KNEEL_UNDER ))
				{
					sprintf( message, "kneeling under %s.", ch->on->short_descr );
					strcat( buf, message );
				}
                else
                {
					sprintf( message, "kneeling in %s.", ch->on->short_descr );
                    strcat( buf, message );
                }
			}
            else
				strcat(buf, "kneeling.");
			break;

		case POS_STANDING:
			if (ch->on != NULL)
			{
				if ( IS_SET( ch->on->value[2], STAND_AT ))
				{
					sprintf( message, "standing at %s.", ch->on->short_descr);
					strcat( buf, message );
				}
				else if ( IS_SET( ch->on->value[2], STAND_ON ))
				{
					sprintf( message, "standing on %s.", ch->on->short_descr );
					strcat( buf, message );
				}
				else if ( IS_SET( ch->on->value[2], STAND_UNDER ))
				{
					sprintf( message, "standing under %s.", ch->on->short_descr );
					strcat( buf, message );
				}
				else
				{
					sprintf( message, "standing in %s.", ch->on->short_descr );
					strcat( buf, message );
				}
			}else{
				strcat( buf, "standing." );
			}
			break;
        
		case POS_FIGHTING:
            strcat( buf, "fighting." );
			break;
		}
	}
    ch->printlnf( "%s", buf );

	if (ch->pnote)
	{
		switch(ch->pnote->type)
		{
		default:
			break;
		case NOTE_NOTE:
			ch->println("`Y You are working on a note.`x");
			break;
		case NOTE_IDEA:
			ch->println("`Y You are working on an idea.`x");
			break;
		case NOTE_PENALTY:
			ch->println("`Y You are working on a penalty!");
			break;
		case NOTE_NEWS:
			ch->println("`Y You are working on a news.");
			break;
		case NOTE_CHANGES:
			ch->println("`Y You are working on a change.");
			break;
		case NOTE_ANOTE:
			ch->println("`Y You are working on an anote.");
			break;
		case NOTE_INOTE:
			ch->println("`Y You are working on an inote.");
			break;
		}
	}
	ch->titlebar("");

    return;
}

/**************************************************************************/
char *full_affect_loc_name( AFFECT_DATA *paf);
/**************************************************************************/
void do_affects(char_data *ch, char * argument)
{
	AFFECT_DATA *paf, *paf_last = NULL;
	
	// support using aff on players by imms
    char_data *v;
	char arg[MIL];
    one_argument( argument, arg );
    if (IS_IMMORTAL(ch) && !IS_NULLSTR(arg)){
		if ( ( v = get_char_world( ch, arg ) ) == NULL )
		{
			ch->printlnf( "'%s' isnt here.", arg);
			return;
		}
    }else{
		v=ch;
	}
   
	if ( v->affected)
	{
		if(v==ch){
			ch->println("`=\xc1You are affected by the following spells:`=\xc2");
		}else{
			ch->printlnf( "`=\xc1%s is affected by the following spells:`=\xc2", PERS(v, ch));
		}
		for ( paf = v->affected; paf; paf = paf->next )
		{
			if(paf->where==WHERE_OBJECTSPELL){
				continue;
			}
			if (paf_last && paf->type == paf_last->type)
			{
				if (ch->level >= 20){
					ch->printf( "                            ");
				}else{
					continue;
				}
			}
			else
			{
				ch->printf( "Spell: %-21s", skill_table[paf->type].name );
			}			
			
			if ( ch->level >= 20 ){
				ch->printf( ": modifies %s by %d ",
					full_affect_loc_name( paf), paf->modifier);

				if ( paf->duration == -1 ){
					ch->printf( "permanently" );
				}else{ // time shown is approximately the correct ic hours
					if(ch->level>=50){ 
						ch->printf( "for %0.1f hours", (float)paf->duration/11.4 ); 
					}else{
						ch->printf( "for %0.0f hours", (float)paf->duration/11.4 ); 
					}				
				}
			}else{
				if ( ch->level >= 10 ){
					ch->printf( ": modifies %s by %d ",
						full_affect_loc_name( paf), paf->modifier);
				}else{
					ch->printf( ": modifies %s",full_affect_loc_name( paf));
				}
			}

			
			ch->println("");
			paf_last = paf;
		}
	}else{
		if(v==ch){
			ch->println("You are not affected by any spells.");
		}else{
			ch->printlnf( "%s is not affected by any spells.", PERS(v, ch));
		}
	}

	if (IS_AFFECTED(v, AFF_HIDE))
	{
		if(v==ch){
			ch->println("You are currently trying to hide with your surroundings.");
		}else{
			ch->printlnf( "%s is currently trying to hide with their surroundings.", PERS(v, ch));
		}
	}

/*	if (IS_AFFECTED2(v, AFF2_VANISH))
	{
		if(v==ch){
			ch->println("You are currenty invisible to anyone but the faeries.");
		}else{
			ch->println("%s if currently invisible to anyone but the faeries.");
		}
	}*/
	ch->print("`x");
}

/**************************************************************************/
const char * const day_name[] =
{
    "the Drake", "Despair", "Deception", "Darkness", "Ruin",
    "the Fallen gods", "the Mist"
};


// now have 12 months 36 days
const char * const month_name	[] =
{
    "Winter", "the Winter Storm", "the Frost Blight", 
	"the Return","Blight","the Dragon",
	"Light",  "the Sun", "the Heat", 
	"the Great War", "the Shadows", "the Long Shadows", 
};
/**************************************************************************/
void do_time( char_data *ch, char * )
{
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

	ch->printlnf("Date: `W%d`S/`W%d`s/`W%d`x",
		time_info.day+1,
		time_info.month+1,
		time_info.year);
    ch->printlnf(
        "It is %d o'clock %s, Day of %s, %d%s the Month of %s in the year %d AK.\r\n",
        (time_info.hour % (ICTIME_HOURS_PER_DAY/2) == 0) ?
			(ICTIME_HOURS_PER_DAY/2) : time_info.hour %(ICTIME_HOURS_PER_DAY/2),
        time_info.hour >= (ICTIME_HOURS_PER_DAY/2) ? "pm" : "am",
        day_name[day % ICTIME_DAYS_PER_WEEK],
        day, suf,
        month_name[time_info.month],
		time_info.year);

	// system time
	ch->printlnf("`gThe system time is %s`x", (char *) ctime( &current_time ));

	if (IS_IMMORTAL(ch))
	{
		int i = get_magecastmod()*3/2;

		ch->printlnf( "Mage spell casting modifier %d", 
			weather_info[ch->in_room->sector_type].mage_castmod );
		ch->printlnf( "Max mod for day %d/%d month %d/%d is %d", 
			time_info.day+1, ICTIME_DAYS_PER_MONTH,
			time_info.month+1, ICTIME_MONTHS_PER_YEAR, i);
	}
	ch->println("To see more server time related info use the `=Cuptime`x command or `=Cmudstats`x.");
    return;
}

/**************************************************************************/
void do_uptime( char_data *ch, char * )
{
    extern time_t boot_time;

    timediff(boot_time, current_time);

	// system time
	ch->printf( "`gThe system time is             %s", (char *) ctime( &current_time ));

    // dawn startup time
	ch->printlnf( "`B%s last rebooted at %s"
		        "Which was %s ago.", 
				MUD_NAME,
				ctime( &lastreboot_time),
				timediff(lastreboot_time, current_time));

	if (!IS_NPC(ch))
	{
		// online time
		ch->printlnf("`YYou connected at               %s"
					"Which was %s ago.",
					 (char *) ctime( &ch->logon), 
					 timediff(ch->logon, current_time));
		// creation time
		ch->printlnf("`cYou first created at           %s"
					"Which was %s ago.",
					 (char *) ctime( (time_t *)&ch->player_id), 
					 short_timediff(ch->player_id, current_time));
		ch->printlnf("`GYou have played approximately %d.%02d hours.`x",
            (int) GET_SECONDS_PLAYED(ch)/ 3600, 
			(int) (GET_SECONDS_PLAYED(ch)/36)%100);

		ch->printlnf("Which is %0.03f%% of the time since you created.",
			((double)GET_SECONDS_PLAYED(ch)/(double)(current_time-ch->player_id))*100.0);
	}		

	// hotreboot
	ch->printlnf("`RLast hotreboot was at          %s"
            "Which was %s ago.`x", str_boot_time, 
			timediff(boot_time, current_time));

	if (IS_IMMORTAL(ch))
	{
		int i = get_magecastmod()*3/2;

		ch->printlnf( "Mage spell casting modifier %d", 
			weather_info[ch->in_room->sector_type].mage_castmod );
		ch->printlnf( "Max mod for day %d/36 month %d/12 is %d",
			time_info.day+1, time_info.month+1, i);
	}
	ch->println("To see game time related info use the ictime command.");

    return;
}


/**************************************************************************/
void do_weather( char_data *ch, char * )
{
    char buf[MSL];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

	if ( IS_OOC(ch) )
    {
		ch->println("Weather is an IC command, you can't use it in OOC rooms.");
		return;
    }

    if ( !IS_OUTSIDE(ch) || ch->in_room->sector_type == SECT_CAVE )
    {
		ch->println("You can't see the sky from here.");
		return;
    }

    ch->printlnf( "The sky is %s and %s.",
		sky_look[weather_info[ch->in_room->sector_type].sky],
		weather_info[ch->in_room->sector_type].change >= 0 ?
		"a warm southerly breeze blows" : "a cold northern gust blows" );

	{
		int i = (int)get_magecastmod()*3/2;

		if (weather_info[ch->in_room->sector_type].moon_getting_brighter)
		{
			switch (i)
			{
			case - 12:
			case - 11:
			case - 10: // rare only once per game year (1 week real time)
sprintf( buf, "The sky is so dark, it is like the moon doesn't exist.\r\n"); break;
			case - 9:
			case - 8:
sprintf( buf, "The sky is pitch black as the moon can not be seen.\r\n"); break;
			case - 7: // about once per month for half of the year
sprintf( buf, "The moon is engulfed in shadow.\r\n"); break;
			case - 6: 
sprintf( buf, "The sky grows dark as the moon obscures the light of the sun.\r\n"); break;
			case - 5:
sprintf( buf, "The Dark Moon rules the sky not a ray of light can be seen.\r\n"); break;
			case - 4:
sprintf( buf, "A faint sliver of moon light slices the sky.\r\n"); break;			
			case - 3:
sprintf( buf, "A thin sliver of moonlight glimmers in the heavens.\r\n"); break;
			case - 2:
sprintf( buf, "The crescent moon glows softly in the sky.\r\n"); break;
			case - 1:
sprintf( buf, "The moonbeams gleam more brightly.\r\n"); break;
			case   0:
sprintf( buf, "The moon, half alight, shines in the heavens.\r\n"); break;
			case   1:
sprintf( buf, "The moonbeams gleam more brightly.\r\n"); break;
			case   2:
sprintf( buf, "The moon light dances as the waxing engulfs darkness.\r\n"); break;			
			case   3:
sprintf( buf, "The moon radiates soft light as it waxes towards full.\r\n"); break;
			case   4:
sprintf( buf, "The pale sickly glow of the three quarter moon washes the land in its light.\r\n"); break;
			case   5:
sprintf( buf, "Only a small sliver of darkness dampens the moon's splendor.\r\n"); break;
			case   6: 
sprintf( buf, "The moon's glory is nearly at its zenith.\r\n"); break;
			case   7: // about once per month for half of the year
sprintf( buf, "The full moon dazzles you with its brightness.\r\n"); break;
			case   8:
			case   9: // anything above here is very rare 
			case  10:
sprintf( buf, "The full moon DAZZLES you with its brightness.\r\n"); break;
			case  11: // 11 is for about 4minutes every week real time!
sprintf( buf, "The full moon is BLINDING with its brightness!\r\n"); break;
			default:
sprintf( buf, "Unknown moon value mc=%d, i=%d, p=1 - please report this to Admin\r\n", 
		weather_info[ch->in_room->sector_type].mage_castmod, i);
				break;
			}
		}
		else // moon getting darker
		{
			switch (i)
			{
			case - 13:
sprintf( buf, "The sky is the darkest you can actually remember, it is like the sky has never seen the moon!\r\n"
		"A chill runs down your spine as you look at the sky.\r\n"); break;
			case - 12:
			case - 11:
			case - 10: // rare only once per game year (1 week real time)
sprintf( buf, "The sky is so dark, it is like the moon doesn't exist.\r\n"); break;
			case - 9: 
			case - 8:
sprintf( buf, "The sky is pitch black as the moon can not be seen.\r\n"); break;
			case - 7: // about once per month for half of the year
sprintf( buf, "The moon is engulfed in shadow.\r\n"); break;
			case - 6: 
sprintf( buf, "The sky grows dark as the moon obscures the light of the sun.\r\n"); break;
			case - 5:
sprintf( buf, "The Dark Moon rules the sky not a ray of light can be seen.\r\n"); break;
			case - 4:
sprintf( buf, "A dim sliver of moon light slices the sky.\r\n"); break;
			case - 3:
sprintf( buf, "Only the edge of moon light still shines upon the world.\r\n"); break;
			case - 2:
sprintf( buf, "A crescent moon hangs shimmering in the sky.\r\n"); break;
			case - 1:
sprintf( buf, "The moonlight fades to a soft glow.\r\n"); break;
			case   0:
sprintf( buf, "The half-lit moon washes the land.\r\n"); break;
			case   1:
sprintf( buf, "Darkness comes forth, but the moon yet shines in the night.\r\n"); break;
			case   2:
sprintf( buf, "The moon light has its last dance as its light is engulfed.\r\n"); break;
			case   3:
sprintf( buf, "The moon radiates soft light as it wans towards darkness.\r\n"); break;
			case   4:
sprintf( buf, "The moon is sliced by the encroaching darkness.\r\n"); break;
			case   5:
sprintf( buf, "The moon's glory is slightly faded in the night sky.\r\n"); break;
			case   6: 
			case   7: // about once per month for half of the year
sprintf( buf, "The full moon dazzles you with its brightness.\r\n"); break;
			case   8:
			case   9: // anything above here is very rare 
			case  10:
sprintf( buf, "The full moon DAZZLES you with its brightness.\r\n"); break;
			case  11: // 11 is for about 4minutes every week real time!
sprintf( buf, "The full moon is BLINDING with its brightness!\r\n"); break;
			default:
sprintf( buf, "Unknown moon value mc=%d, i=%d, p=2 - please report this to Admin\r\n", 
		weather_info[ch->in_room->sector_type].mage_castmod, i);
				break;
			}
		}

		if (i>-7) // can't see the moon really at this time of night
		{
			switch (time_info.hour)
			{
				case 20:
					ch->println("The tip of the moon can be seen on the surface of the horizon.");
					break;
				case 21:
					ch->println("The moon rises just above the horizon.");
					break;
				case 22:
					ch->println("The moon has risen midway up the sky.");
					break;
				case 23:
					ch->println("The moon is almost directly above you.");
					break;
				case 0:
					ch->println("The moon is directly above.");
					break;
				case 1:
					ch->println("The moon is starting to fall.");
					break;
				case 2:
					ch->println("The moon is about midway down the sky.");
					break;
				case 3:
					ch->println("The moon is falling just below the horizon.");
					break;
				case 4:
					ch->println("The tip of the moon is disappearing below the surface of the horizon.");
					break;
				default: // day time
					break;
			}
		}
		if (time_info.hour<5 || time_info.hour>20)
			ch->printf( "%s", buf );
	}
    return;
}
/**************************************************************************/
// Airius 
void do_lore(char_data *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf=NULL;
    int skill;
	
    // find out what 
    if (argument == '\0')
    {
		ch->println("Lore what item?");
		return;
    }
	
	if (ch->position == POS_FIGHTING) {
		ch->println("Not while fighting you don't!");
		return;
	}
	
    obj =  get_obj_list(ch,argument,ch->carrying);
	
    if (obj== NULL)
    {
        ch->println("You don't have that item.");
        return;
    }
	
    if ((skill = get_skill(ch,gsn_lore)) < 1)
    {
        ch->println("Yeah right, like you have even ever looked at a book.");
        return;
    }
	
    if (ch->mana<40){
		ch->println("You don't have enough mana.");
        return;
    }else{
        ch->mana-=40;
	}

    if (number_percent()>= skill - obj->level + ch->level)  
    { 
		// failure at lore
	    ch->wrapln("You try to think back into your memory, even after a period of deep "
		    "concentration, nothing useful surfaces from your thoughts.");
		return;
	}
		
	// success! 	
	// Herbs can't be lored
	if ( obj->item_type == ITEM_HERB ) {
		ch->println("You can't learn anything important about this item.");
		ch->mana+=40;
		return;
	}
	
	ch->printlnf(
		"Object '%s' is type %s,\r\n"
		"Extra flags %s.\r\n"
		"Extra2 flags %s.\r\n"
		"Weight is %0.1f lbs, value is %d, level is %d.",
        obj->name, 
		item_type_name( obj ),
		extra_bit_name( obj->extra_flags ),
		extra2_bit_name( obj->extra2_flags ),
		((double)obj->weight) / 10,
		obj->cost,
		obj->level );
	
    switch ( obj->item_type )
    {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		ch->printf( "Level %d spells of:", obj->value[0] );
		
		if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
		{
			ch->printlnf( " '%s'", skill_table[obj->value[1]].name  );
		}
		
		if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
		{
			ch->printlnf( " '%s'", skill_table[obj->value[2]].name );
		}
        
		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			ch->printlnf( " '%s'", skill_table[obj->value[3]].name );
		}
        
		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
		{
			ch->printlnf( " '%s'", skill_table[obj->value[4]].name );
		}
        break;
		
	case ITEM_WAND:
	case ITEM_STAFF:
		ch->printf( "Has %d charges of level %d", obj->value[2], obj->value[0] );

		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			ch->printlnf( " '%s'.", skill_table[obj->value[3]].name );
		}

        break;
		
	case ITEM_DRINK_CON:
		ch->printlnf( "It holds %s-colored %s.",
			liq_table[obj->value[2]].liq_color,
			liq_table[obj->value[2]].liq_name);
        break;

	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
		ch->printlnf( "Maximum combined weight: %d lbs, Capacity for an individual item: %d lbs.",
			obj->value[0], obj->value[3]);
		ch->printlnf( "Flags: %s.", cont_bit_name(obj->value[1]));
		if (obj->value[4] != 100){
			ch->printf( "Weight multiplier: %d%%\r\n", obj->value[4]);
		}
		break;
		
	case ITEM_WEAPON:
		ch->printlnf( "Weapon type is %s", get_weapontype( obj ));

		ch->printlnf( "Damage is %dd%d (average %d).",
			obj->value[1],obj->value[2],
			(1 + obj->value[2]) * obj->value[1] / 2);
		if(obj->value[4])  // weapon flags 
		{
			ch->printlnf( "Weapons flags: %s",weapon_bit_name(obj->value[4]));
		}
        break;
		
        case ITEM_ARMOR:
            ch->printlnf(
				"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.",
                obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
			break;
		
		case ITEM_SHEATH:
			ch->printlnf( "It will hold %d weapon%s.",

				obj->value[0], obj->value[0]==1?"":"s");
			break;
    }
	
    for ( paf = OBJECT_AFFECTS(obj); paf; paf = paf->next )
    {
        if ( paf->location != APPLY_NONE && paf->modifier != 0 )
        {       
			if(paf->duration > -1){
				ch->printf( "Affects %s by %d for %d hour%s.%s\r\n",
					affect_loc_name( paf->location ), paf->modifier,
					paf->duration, paf->duration==1?"":"s",
					(paf->level>ch->level?"  (above your level)":""));
			}else{
				ch->printf( "Affects %s by %d.%s\r\n",
					affect_loc_name( paf->location ), paf->modifier,
					(paf->level>ch->level?"  (above your level)":""));
			}	
		}else if(paf->bitvector || paf->where==WHERE_SKILLS){

			ch->println( to_affect_string( paf, obj->level, false));

		}

	}
	check_improve(ch,gsn_lore,true,2);
    return;
}

/**************************************************************************/
int count_active_players(void);
// displays info to the player about the mud - by Kalahn 98
void do_mudstats( char_data *ch, char * )
{
	int w=70; // width
	ch->print("`=t`#"); // custom title bar colour
	centerf_to_char( ch, w, "--============== `Y%s Statistics`^ ===============--`x\r\n",
		MUD_NAME);
	ch->println( "" );
	centerf_to_char( ch, w,"`^-====== `YGame World Info`^ ======-`x\r\n");
    centerf_to_char( ch, w,"Area Count          = %5d\r\n", top_area      );
    centerf_to_char( ch, w,"Unique Room count   = %5d\r\n", top_room      );
    centerf_to_char( ch, w,"Shop count          = %5d\r\n", top_shop      );
	centerf_to_char( ch, w,"Inn count           = %5d\r\n", top_inn       );
    centerf_to_char( ch, w,"Unique Mobile count = %5d\r\n", top_mob_index );
    centerf_to_char( ch, w,"Mobile inuse count  = %5d\r\n", mobile_count  );
    centerf_to_char( ch, w,"Mudprog count       = %5d\r\n", mudprog_count );
	centerf_to_char( ch, w,"Unique Object count = %5d\r\n", top_obj_index );
    centerf_to_char( ch, w,"Help Entries count  = %5d\r\n", top_help      );
	centerf_to_char( ch, w,"Social count        = %5d\r\n", social_count );

	ch->println( "" );
	centerf_to_char( ch, w,"`^-=====================`G Server Info`^ ======================-`x\r\n");
	centerf_to_char( ch, w,"The Current System time is %.24s\r\n",
                 (char *) ctime( &current_time ));
    centerf_to_char( ch, w,"The mud is running from %s.\r\n", MACHINE_NAME);
    centerf_to_char( ch, w,"Accepting telnet connections on port %d.\r\n", mainport);
//    if (IS_IRC(ch) || IS_IMMORTAL(ch)){
//		centerf_to_char( ch, w,"IRC DCC chat connections are on port %d.\r\n", ircport);
//  }
//	centerf_to_char( ch, w,"Integrated webserver connections are on port %d.\r\n", webport);
//	centerf_to_char( ch, w,"MudFtp connections are on port %d.\r\n", mudftpport); 
    centerf_to_char( ch, w,"Webhits %d, WebHelpHits %d, WebWhohits %d\r\n", 
		webHits, webHelpHits, webWhoHits);

    // dawn startup time
	centerf_to_char( ch, w,"`^-=====================`B Last Reboot`^ ======================-`x\r\n");
	centerf_to_char( ch, w,"%s last rebooted at %.24s\r\n", MUD_NAME, ctime( &lastreboot_time));
	centerf_to_char( ch, w,"Which was %s ago,\r\n", timediff(lastreboot_time, current_time));
	centerf_to_char( ch, w, "since then the maxon has been `B%d`x\r\n", true_count);
	centerf_to_char( ch, w,"This was `B%s`x ago\r\n", timediff(maxon_time, current_time));
	centerf_to_char( ch, w,"at %.24s systime.\r\n", (char *) ctime( &maxon_time ));

	centerf_to_char( ch, w,"`^ -=======================`R HotReboot`^ =========================-`x\r\n");
	centerf_to_char( ch, w,"%s last hotrebooted at %.24s\r\n", MUD_NAME, str_boot_time);			
	centerf_to_char( ch, w,"Which was %s ago,\r\n", timediff(boot_time, current_time));
	centerf_to_char( ch, w,"since then the maxon has been `R%d`x\r\n", hotrebootmaxon);
	centerf_to_char( ch, w,"This was `R%s`x ago\r\n", timediff(hotrebootmaxon_time, current_time));
	centerf_to_char( ch, w,"at %.24s systime.\r\n", (char *) ctime( &hotrebootmaxon_time));
	ch->println( "" );
	do_compile_time(ch,"");
	centerf_to_char( ch, w,"Active Player Count = %d, (>5 <%d, on in the last week)\r\n", 
		count_active_players(), LEVEL_IMMORTAL);
}

/**************************************************************************/
void do_inventory( char_data *ch, char *argument )
{
	ch->println( "`xYou are carrying:`g" );
	show_list_to_char( ch->carrying, ch, argument, true, true );
	ch->print( "`x" );

	if(IS_NULLSTR(argument) && IS_NEWBIE(ch)){
		ch->println("You can filter the inventory list by text and item type...");
		ch->println("examples: 'inv skin', 'inv mace', 'inv food'...");
	}

	return;
}

/**************************************************************************/
void do_equipment( char_data *ch, char * )
{
	OBJ_DATA *obj;
	int iWear;
	bool found;
	
	ch->println( "You are equipped with the following:`=\xab");
	found = false;

	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{

		if ( iWear == WEAR_LODGED_ARM
		&&	 get_eq_char( ch, iWear ) == NULL )
			continue;

		if ( iWear == WEAR_LODGED_LEG
		&&	 get_eq_char( ch, iWear ) == NULL )
			continue;

		if ( iWear == WEAR_LODGED_RIB
		&& get_eq_char( ch, iWear) == NULL )
			continue;	
		
		if ( iWear == WEAR_SHEATHED
		&&	 get_eq_char( ch, iWear ) == NULL )
			continue;

		if ( iWear == WEAR_CONCEALED
		&&	 get_eq_char( ch, iWear ) == NULL )
			continue;

		if( GAMESETTINGMF1( GAMESETMF1_PENDANTS_ENABLED ) ){
			if ( iWear == WEAR_PENDANT
			&&	 get_eq_char( ch, iWear ) == NULL )
				continue;
		}
		
		if ( ( obj = get_eq_char( ch, iWear ) ) == NULL ){
			ch->print("`=\xc5");
			ch->print( where_name[iWear] );
			ch->println( "nothing");
			continue;
		}else{
			ch->print("`=\xab");
			ch->print( where_name[iWear] );
		}
		
		if ( can_see_obj( ch, obj ) )
		{
			ch->printlnf( "%s", format_obj_to_char( obj, ch, true ));

		}
		else
		{
			ch->println( "something." );
		}
		found = true;
	}
	ch->print( "`x" );
	return;
}
/**************************************************************************/
// Find an obj in player's inventory.
OBJ_DATA *get_obj_carry_of_type_excluding( char_data *ch, char *argument, OBJ_DATA *exclude)
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
		if(obj==exclude || obj->item_type!=exclude->item_type){
			continue;
		}
		if( obj->wear_loc == WEAR_NONE
		&&   (can_see_obj( ch, obj ) ) 
		&&   is_name( arg, obj->name ) )
		{
			if( ++count == number )
			return obj;
		}
    }
    return NULL;
}
/**************************************************************************/
enum COMPARE_RESULT{COMPARE_WORSE, COMPARE_ABOUT_SAME, COMPARE_BETTER};
/**************************************************************************/
COMPARE_RESULT calc_compare( obj_data *obj1, obj_data *obj2)
{
	assertp(obj1);
	assertp(obj2);

	int value1=0;
	int value2=0;
	switch ( obj1->item_type )
	{
	default:
		bug("display_compare(): How did we get here?");
		do_abort();
		break;
	case ITEM_ARMOR:
		value1 = obj1->value[0] + obj1->value[1] + obj1->value[2] + obj1->value[3];
		value2 = obj2->value[0] + obj2->value[1] + obj2->value[2] + obj2->value[3];
		break;
	case ITEM_WEAPON:
//		value1 = (1 + obj1->value[2]) * obj1->value[1];
//		value2 = (1 + obj2->value[2]) * obj2->value[1];
		//compare the actual average damages (excluding the /2, which makes no difference)
		value1 = (obj1->value[1]+(obj1->value[1] * obj1->value[2]));
		value2 = (obj2->value[1]+(obj2->value[1] * obj2->value[2]));
		break;
	}

	int r=value1-value2; 
	if(r<-1){ 
	  return COMPARE_WORSE; 
	}else if (r>1){ 
	  return COMPARE_BETTER;
	} 
	// r is either -1, 0 or 1
	return COMPARE_ABOUT_SAME; 
}

/**************************************************************************/
// Compare an object to everything comparable a player carries.
// - I know this is far from the most efficient way to write this
//   but it is a quick hack to give nice the functionality - Kal, Feb 2001.
void do_compare( char_data *ch, char *argument )
{
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int count;

	if ( IS_NULLSTR(argument) )
	{
		ch->println( "Compare what to everything you carry?" );
		return;
	}

	if ( ( obj1 = get_obj_carry( ch, argument ) ) == NULL )
	{
		ch->printlnf( "You do not have any '%s' item.", argument);
		return;
	}

	if(obj1->item_type!=ITEM_ARMOR && obj1->item_type!=ITEM_WEAPON){
		ch->printlnf("You can only compare weapons or armour and '%s' is neither.", 
			obj1->short_descr);
		return;
	}

	// we have an item - compare it with everything we can
	name_linkedlist_type* list[3], *plist, *plist_next;
	for(count=0;count<3; count++){
		list[count]=NULL;
	}

	bool found=false;
	char vnumtext[MIL];
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
		if( can_see_obj(ch,obj2)
			&&  obj1->pIndexData!=obj2->pIndexData
			&&  obj1->item_type == obj2->item_type			
			&&  (obj1->wear_flags & obj2->wear_flags & ~OBJWEAR_TAKE) != 0 )
		{
			sprintf(vnumtext, "%s%07d", obj2->short_descr, obj2->pIndexData->vnum);
			addlist(&list[calc_compare(obj1, obj2)],vnumtext, (int)*(get_canwear_colour(obj2, ch)+2), false, false);
			found=true;
		}
	}
	char objprefix[MIL];
	sprintf(objprefix,"%s%s `xis ", get_canwear_colour(obj1, ch), obj1->short_descr);	

	for(count=0; count<3; count++){
		bool first=true;
		char *comp="";
		for(plist=list[count];plist; plist=plist_next){
			plist_next=plist->next;
			if(first){
				switch(count){
					case COMPARE_WORSE: comp="worse than"; break;
					case COMPARE_ABOUT_SAME: comp="about the same as"; break;
					case COMPARE_BETTER: comp="better than"; break;
				}
				first=false;
			}	
			size_t i=str_len(plist->name)-7;
			plist->name[i]='\0'; // hide the number at the end
			ch->print(objprefix);
			ch->printlnf("%s `=%c%s`x", comp, (char)plist->tag, plist->name);
			plist->name[i]='0';
			free_string(plist->name);
			delete plist;
		}
	}
	
	if(!found){		
		ch->printlnf("You have nothing that is comparable to %s.", 
			obj1->short_descr);
	}

	return;
}

/**************************************************************************/
void do_credits( char_data *ch, char * )
{
    do_help( ch, "credits" );
	return;
}

/**************************************************************************/
void do_where( char_data *ch, char *argument )
{
	char arg[MIL];
    char_data *victim=NULL;
    connection_data *d;
    bool found;

    one_argument( argument, arg );
	
    if ( arg[0] == '\0' )
    {
		ch->println( "Players near you:" );
		found = false;
		for ( d = connection_list; d; d = d->next )
		{
			if ( d->connected_state == CON_PLAYING
				&& ( victim = d->character ) != NULL
				&&   !IS_NPC(victim)
				&&   victim->in_room != NULL
				&&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
				&&   is_room_private_to_char( victim->in_room, ch )
				&&   victim->in_room->area == ch->in_room->area
				&&   can_see( ch, victim ) )
			{
				found = true;
				
				ch->printlnf( "[%s%5d`x] %-12s is in %s (%s)",
					colour_table[(victim->in_room->vnum%14)+1].code,
					victim->in_room->vnum, victim->name,
					victim->in_room->name, victim->in_room->area->name);
			}
		}
		if ( !found )
			ch->println( "None." );
    }
    else
    {
		victim = get_char_world( ch, arg );

		if ( victim )
		{
			ch->printlnf( "[%s%5d`x] %-12s is in %s (%s)",
				colour_table[(victim->in_room->vnum%14)+1].code,
				victim->in_room->vnum, victim->name,
				victim->in_room->name, victim->in_room->area->name);
		}
		else
		{
			act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
		}
	}
	return;
}


/**************************************************************************/
void where_char( char_data *victim, char_data *ch,
   sh_int depth, sh_int door )
{
    char buf[MIL], buf2[MIL];
    buf[0] = '\0';

    strcat( buf, PERS( victim, ch ));
    strcat( buf, ", " );
    sprintf( buf2, where_distance[depth], dir_name[door] );
    strcat( buf, buf2 );
    ch->println( buf );
    return;
}

/**************************************************************************/
void do_append( char_data *ch, char * )
{
	ch->println("To edit your description type `=Cdescript`x." );
    return;
}
/**************************************************************************/
void do_description( char_data *ch, char * )
{
	if(IS_NPC(ch))
        return;

	ch->println("Entering edit mode for you own description." );
	ch->println("Type @ to finish editing your description." );
	string_append(ch, &ch->description);

	return;

}
/**************************************************************************/
void do_pkinfo( char_data *ch, char *argument )
{
	char_data *victim;

    if (IS_IMMORTAL(ch) && !IS_NULLSTR(argument)){
		if ( ( victim = get_char_world( ch, argument ) ) == NULL )
		{		
			ch->printlnf( "'%s' couldn't be found.", argument);
			return;
		}else{
			ch->printlnf( "Showing pkinfo for %s.", PERS(victim, ch));			
		}
    }else{
		victim=ch;
	}

	int w=79;

	ch->print("`=t`#");
	if(GAMESETTING(GAMESET_PEACEFUL_MUD)){
		ch->titlebar("THE GAME IS LOCKED IN PEACEFUL MODE");
	}

	ch->titlebar("PLAYER KILL INFO");
	

	centerf_to_char(ch, w,"You have player killed %d time%s.\r\n",
			  victim->pkkills, victim->pkkills==1 ? "" : "s");

	centerf_to_char(ch, w, "You have been player killed %d time%s.\r\n",
			  victim->pkdefeats, victim->pkdefeats==1 ? "" : "s");

	centerf_to_char(ch, w,"You are safe from being pkilled for %d more hour%s.\r\n",
			  victim->pksafe, victim->pksafe==1 ? "" : "s");

	centerf_to_char(ch, w,"You may not OOL for %d hour%s.\r\n",
			  victim->pkool, victim->pkool==1 ? "" : "s");

	centerf_to_char(ch, w,"You can not recall for %d hour%s.\r\n\r\n",
			  victim->pknorecall, victim->pknorecall==1 ? "" : "s");


	if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){
		centerf_to_char(ch, w,"You can not quit for %d hour%s.\r\n",
				  victim->pknoquit, victim->pknoquit==1 ? "" : "s");
		if(!IS_NPC(ch)){
			centerf_to_char(ch, w,"P9999Kills %d, P9999Defeats %d\r\n",
					victim->pcdata->p9999kills, victim->pcdata->p9999defeats);
		}
	}else{
		centerf_to_char(ch, w,"You can not quit for %d hour%s.\r\n",
				  UMAX(victim->pknoquit,victim->pknorecall) , 
				  UMAX(victim->pknoquit,victim->pknorecall)==1 ? "" : "s");
	}

	// mkill info and stealing consquences
	if (!IS_NPC(victim))
	{
		ch->titlebar("MOBILE DEFEAT COUNTER");
		centerf_to_char(ch, w,"In your lifetime you have been defeated by %d mob%s.\r\n",
			  victim->pcdata->mdefeats, victim->pcdata->mdefeats==1 ? "" : "s");


		if(victim->pcdata->unsafe_due_to_stealing_till>current_time){
			int t=(int)((victim->pcdata->unsafe_due_to_stealing_till-current_time)/60)+1;
			ch->print_blank_lines(1);
			ch->titlebar("STEALING CONSEQUENCES");
			centerf_to_char(ch, w,"You will currently automatically accept all duels for the next %d minute%s\r\n",
				t, t==1?"":"s");
		} 
	}

	// display duel info if duel system is enabled (on by default)
	if(!GAMESETTING2(GAMESET2_NO_DUEL_REQUIRED)){		
		if(victim->duels){
			victim->duels->display_pkinfo(ch);
		}
	}

	ch->titlebar("");
}

/**************************************************************************/
void show_practice_list(char_data *showto, char_data *victim)
{
    BUFFER *output;
    char buf[MSL];
    int sn;
    int col;

    output = new_buf();
    col    = 0;

	{

		if(HAS_CONFIG(victim,CONFIG_PRACSYS_TESTER)){
			sprintf(buf,"`?%s", format_titlebar("PRACTICED FOR NEW PRAC SYSTEM TESTING"));
		}else{
			sprintf(buf,"`?%s", format_titlebar("PRACTICED"));
		}
		buf[str_len(buf)-2]= '\0';
		strcat(buf,"`x\r\n");
		add_buf( output, buf);

		add_buf( output, percent_colour_codebar());

		for ( sn = 0; sn < MAX_SKILL; sn++ )
		{
			int maxprac=skill_table[sn].maxprac_percent[victim->clss];
			if(maxprac==0){
				if(HAS_CONFIG(victim,CONFIG_PRACSYS_TESTER)){
					maxprac=50;
				}else{
					maxprac=class_table[victim->clss].skill_adept;
				}
			}

			if ( IS_NULLSTR(skill_table[sn].name))
				break;
			
			if (victim->pcdata->learned[sn] < 1) // skill is not known 
				continue;

			// filter out what can be praced
			{

				// level zero skills - imm only 
				if (skill_table[sn].skill_level[victim->clss]!=0
					&& skill_table[sn].rating[victim->clss]>=1
					&& !IS_SET(skill_table[sn].flags,SKFLAGS_NO_PRAC)) 
				{ 
					if(victim->pcdata->learned[sn]<maxprac){
						continue;
					}					
				}
			}


			if ( !IS_SPELL(sn) 
				&& (victim->level < skill_table[sn].skill_level[victim->clss]) )
				continue;

			// level zero skills - imm only 
			if ((skill_table[sn].skill_level[victim->clss]==0)
					&& victim->level<LEVEL_IMMORTAL)  
				continue;
			
			if ( col % 3 == 1 ){
				sprintf( buf, "  %s%-17.17s `x%3d%%   ",
					percent_colour_code(victim->pcdata->learned[sn]),
					skill_table[sn].name, victim->pcdata->learned[sn] );
			}else{
				sprintf( buf, " %s%-18.18s `x%3d%%  ",
					percent_colour_code(victim->pcdata->learned[sn]),
					skill_table[sn].name, victim->pcdata->learned[sn] );
			}

			add_buf( output, buf );
			if ( ++col % 3 == 0 )
				add_buf( output, "\r\n" );
		}
		
		if ( col % 3 != 0 )
			add_buf( output, "\r\n" );


		col    = 0;


		if(HAS_CONFIG(victim,CONFIG_PRACSYS_TESTER)){
			sprintf(buf,"`?%s", format_titlebar("PRACTICING FOR NEW PRAC SYSTEM TESTING"));
		}else{
			sprintf(buf,"`?%s", format_titlebar("PRACTICING"));
		}
		buf[str_len(buf)-2]= '\0';
		strcat(buf,"`x\r\n");
		add_buf( output, buf);

		add_buf( output, percent_colour_codebar());

		for ( sn = 0; sn < MAX_SKILL; sn++ )
		{
			int maxprac=skill_table[sn].maxprac_percent[victim->clss];
			if(maxprac==0){
				if(HAS_CONFIG(victim,CONFIG_PRACSYS_TESTER)){
					maxprac=50;
				}else{
					maxprac=class_table[victim->clss].skill_adept;
				}
			}

			if ( IS_NULLSTR(skill_table[sn].name))
				break;
			
			if (victim->pcdata->learned[sn] < 1) // skill is not known 
				continue;

			// filter out what can't be praced
			{
				if(victim->pcdata->learned[sn]>=maxprac){
					continue;
				}

				// level zero skills - imm only 
				if (skill_table[sn].skill_level[victim->clss]==0) 
				{ 
					continue;
				}

				if(skill_table[sn].rating[victim->clss]<1){
					continue;
				}

				// check for no prac flag on skill
				if(IS_SET(skill_table[sn].flags,SKFLAGS_NO_PRAC)){
					continue;
				}	
			}


			
			if ( !IS_SPELL(sn) 
				&& (victim->level < skill_table[sn].skill_level[victim->clss]) )
				continue;

			// level zero skills - imm only 
			if ((skill_table[sn].skill_level[victim->clss]==0)
					&& victim->level<LEVEL_IMMORTAL)  
				continue;
			
			if ( col % 3 == 1 ){
				sprintf( buf, "  %s%-17.17s `x%3d%%   ",
					percent_colour_code(victim->pcdata->learned[sn]),
					skill_table[sn].name, victim->pcdata->learned[sn] );
			}else{
				sprintf( buf, " %s%-18.18s `x%3d%%  ",
					percent_colour_code(victim->pcdata->learned[sn]),
					skill_table[sn].name, victim->pcdata->learned[sn] );
			}

			add_buf( output, buf );
			if ( ++col % 3 == 0 )
				add_buf( output, "\r\n" );
		}
		
		if ( col % 3 != 0 )
			add_buf( output, "\r\n" );
	}

    sprintf(buf, "\r\n                       You have %d practice session%s left.\r\n",
        victim->practice, victim->practice==1?"":"s");
	add_buf( output, buf );

    showto->sendpage(buf_string(output));
    free_buf(output);		
}

/**************************************************************************/
void new_practice( char_data *ch, int sn, int number_of_times)
{
	if (sn==-1 || number_of_times<1){
		return;
	}
	// can't improve in ooc rooms
    if (IS_OOC(ch)) {
		ch->println("BUG: You shouldn't be able to prac in an OOC ROOM - please report to admin!");
		return;
	}

	new_practice( ch, sn, number_of_times-1); // recursively call it

	// calculate the maxprac and maxlearn values
	int maxlearn=skill_table[sn].maxprac_percent[ch->clss];
	if(maxlearn<1){
		maxlearn=65; // default max learn - for unskilled
	}
	int maxprac=skill_table[sn].maxprac_percent[ch->clss];
	if(maxprac<1){
		maxprac=50; // default max prac
	}
	// can't prac better than the max prac amount for your class 
	if(ch->pcdata->learned[sn]>=maxprac){
		return; 
	}

	// store the per thousand improve table
	static short improve_chance_lookup_table[102]; // stored in range 1 ->10000
	static bool initialise_table=true;
	if(initialise_table){ // calculate the improvement chances once, first time
		int i;
		double f;
		for(i=0; i<=100; i++){
	//		f=sqrt((100.0-i)/100);
			f=(100.0-i)/100;
			f=pow(100.0,f);
			f*=100; // put in 1->10000 range - increased precision
			improve_chance_lookup_table[i]=(short)f;
//			logf("improve_chance_lookup_table[%d]=%d", i, improve_chance_lookup_table[i]);
		}
		improve_chance_lookup_table[101]=1;
		initialise_table=false;
	}

	int chance=improve_chance_lookup_table[ch->pcdata->learned[sn]];
	int max_percent=UMAX(maxlearn, maxprac);
	if(max_percent>0 && max_percent<100){ // scale it to work in the range for your class
		chance=chance* max_percent/100;
		if(chance<1){
			chance=1;
		}
	}
	if (number_range(1,10000)> chance){
		return;
	}
	ch->pcdata->learned[sn]++;
}
/**************************************************************************/
void do_practice( char_data *ch, char *argument )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
	char arg1[MSL];
    int sn;
	int maxprac;
	
    if ( IS_NPC(ch))
        return;
	
    if ( IS_NULLSTR(argument ))
	{
		show_practice_list(ch, ch);
		return;
    }

    char_data *mob;
    int adept;
	
	if (!str_prefix("'",argument)){  // ' symbol optional
		argument = one_argument( argument, arg1 );
	}else{
		strcpy(arg1, argument);
	}
	
    if ( !IS_AWAKE(ch) )
    {
        ch->println("In your dreams, or what?");
        return;
	}
	
	// Find the practice mob in the room
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
            break;
    }
    if( !mob){			
        ch->println( "You can't do that here." );
		
		// display a hint about where their class can prac
		if(GAMESETTINGMF1(GAMESETMF1_SHOW_NEWBIE_HINTS) && IS_NEWBIE(ch)){
			if(!IS_NULLSTR(class_table[ch->clss].newbie_prac_location_hint)){
				ch->println( class_table[ch->clss].newbie_prac_location_hint);
			}
		}
        return;
    }

    // preprac trigger, activated only on mobs with the trigger 
	// if the command 'mob preventprac' is called, then the
	// pracing is aborted
	if ( IS_NPC(mob) && HAS_TRIGGER(mob, MTRIG_PREPRAC)){
		mudprog_preventprac_used=false;
		if(mp_percent_trigger( mob, ch, NULL, NULL, MTRIG_PREPRAC)){
			if(mudprog_preventprac_used){
				mob->printlnf("Pracing prevented on %s", PERS(ch, NULL));
				return;
			}
		}
	}

	if(!IS_ICIMMORTAL(ch)){
		if(!can_see(mob, ch)){
			ch->wraplnf("It looks as though %s would be able to help you practice,"
				"go visible then try again.", PERS(mob, ch));
			return;
		}
	}

    if ( ch->practice <= 0 )
    {
        ch->println("You have no practice sessions left.");
        return;
    }
	
    if ( ( sn = find_spell( ch,arg1, false) ) < 0)
    {
        ch->printlnf( "No such skill/spell '%s' exists.", arg1);
        return;
    }
	
    // all ch's down here have pcdata
    if (ch->pcdata->learned[sn] < 1) // skill is not known
    {
        ch->printlnf( "You know nothing of the skill/spell %s yet.", 
			skill_table[sn].name);
        return;
    }
	
	if ( !IS_SPELL(sn) && (ch->level < skill_table[sn].skill_level[ch->clss]) )
    {
        ch->printlnf( "You haven't reached the level required to use the skill %s yet.", 
			skill_table[sn].name);
        return;
    }

	// level zero skills - imm only 
	if (skill_table[sn].skill_level[ch->clss]==0) 
	{ 
        ch->printlnf( "You can't practice %s.", skill_table[sn].name);
        return;
	}

    if(skill_table[sn].rating[ch->clss]<1){
        ch->printlnf( "You can't practice %s.", skill_table[sn].name);
        return;
    }

	// check for no prac flag on skill
	if(IS_SET(skill_table[sn].flags,SKFLAGS_NO_PRAC)){
		ch->printlnf( "%s can't be practiced here.", skill_table[sn].name);
		return;
	}	
	
	maxprac=skill_table[sn].maxprac_percent[ch->clss];
	adept=maxprac;
	if(adept==0){
		if(HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER)){
			adept=50;
		}else{
			adept=class_table[ch->clss].skill_adept;
		}
	}

	// messages when greater than adept
    if ( ch->pcdata->learned[sn] >= adept )
    {
		if(skill_table[sn].get_maxprac(ch)){
			ch->printlnf( "You can't practice %s any further than you already have.",
				skill_table[sn].name );
		}else{
			ch->printlnf( "You are already learned at %s.",
				skill_table[sn].name );
		}
		return;
    }

	// do the prac
    ch->practice--;
	// Use the new system if appropriate
	if(IS_SET(skill_table[sn].flags,SKFLAGS_NEW_IMPROVE_SYSTEM)
		|| HAS_CONFIG(ch,CONFIG_PRACSYS_TESTER)){
		int pracamount= ((ch->modifiers[STAT_ME]
					   +ch->modifiers[STAT_RE]
					   +(ch->modifiers[STAT_IN]/2)
					   +30)*4)/skill_table[sn].rating[ch->clss];

		act( "$n receives lessons from $N.",
			ch, NULL, mob, TO_ROOM );
		
		int old_ability=ch->get_display_skill(sn);
		new_practice(ch, sn, pracamount); // calls itself recursively

		if(old_ability==ch->pcdata->learned[sn]){
			ch->printlnf( "You attempt to practice %s.", skill_table[sn].name);
			ch->printlnf( "You failed to learn anything about %s you didn't already know.",
				skill_table[sn].name);
		}else{
			if(ch->get_display_skill(sn)>=maxprac){
				if(ch->get_display_skill(sn)>skill_table[sn].get_maxprac(ch)){
					ch->wraplnf("%s instructs you that %s has taught you all you can "
						"learn from another about %s, the rest you will need to teach "
						"by yourself with use.",
						CPERS(mob,ch), he_she[URANGE(0, mob->sex , 2)], skill_table[sn].name);
				}else{
					ch->wraplnf("%s informs you that you have been "
						"taught all you learn about %s.",
						CPERS(mob,ch), skill_table[sn].name);
				}
			}else{
				ch->printlnf("%s gives you lessons in %s.",
					PERS(mob,ch), skill_table[sn].name);
			}
			ch->printlnf("Your %s is now at %d%% (%s max prac is %d%%, %s learn scale is %d%%).", 
				skill_table[sn].name, ch->pcdata->learned[sn],
				class_table[ch->clss].name, skill_table[sn].get_maxprac(ch),
				class_table[ch->clss].name, skill_table[sn].get_learnscale(ch));
		}
		ch->printlnf( "You now have %d practice session%s left.",
			ch->practice, ch->practice==1?"":"s");
		return;
	}

	// use the old system
    ch->pcdata->learned[sn] +=
        (ch->modifiers[STAT_ME]+ch->modifiers[STAT_RE]+30) /
		skill_table[sn].rating[ch->clss];
    if ( ch->pcdata->learned[sn] < adept )
	{
        act( "You practice $T.",
            ch, NULL, skill_table[sn].name, TO_CHAR );
		ch->printlnf( "Your %s is now at %d%%.",
			skill_table[sn].name, ch->pcdata->learned[sn]);
		act( "$n practices $T.",
			ch, NULL, skill_table[sn].name, TO_ROOM );
    }
    else
	{
        ch->pcdata->learned[sn] = adept;
        act( "You are now learned at $T.",
            ch, NULL, skill_table[sn].name, TO_CHAR );
        act( "$n is now learned at $T.",
            ch, NULL, skill_table[sn].name, TO_ROOM );
    }
	ch->printlnf( "You now have %d practice session%s left.",
		ch->practice, ch->practice==1?"":"s");
}
 
/**************************************************************************/
// 'Wimpy' originally by Dionysos.
void do_wimpy( char_data *ch, char *argument )
{
	char arg[MIL];
	int wimpy;

	one_argument( argument, arg );

	if(IS_NULLSTR(arg)){
		wimpy = ch->max_hit / 5;
    }else{
		wimpy = atoi( arg );
	}
	
	if ( wimpy < 0 )
	{
		ch->println("Your courage exceeds your wisdom.");
		return;
	}

	if ( wimpy > ch->max_hit/2 )
	{
		ch->println("Such cowardice ill becomes you.");
		return;
	}

	ch->wimpy	= wimpy;
    ch->printlnf( "Wimpy set to %d hit points.", wimpy );
	if(IS_NULLSTR(arg)){
		ch->println( "Note: You can set your wimpy to a specific number of hitpoints - use wimpy <number>");
	}
}

/**************************************************************************/
void do_panic( char_data *ch, char *argument )
{
	char arg[MIL];
	int panic;

	argument=one_argument( argument, arg );
	
	if(IS_NPC(ch))
		return;

	if ( arg[0] == '\0' )
		panic = ch->max_hit / 10;
	else if(is_number(arg))
	{
		panic = atoi( arg );
	}
	else
	{
		ch->println("You must specify a number.");
		return;
	}

    if ( panic < 0 )
	{
		ch->println("You courage exceeds your wisdom.");
		return;
	}

	if ( panic > ch->max_hit/5 )
	{
		ch->println("Such cowerdice is not possible in this realm.");
		return;
	}

	ch->pcdata->panic   = panic;
	ch->printlnf( "You will PANIC at %d hit points.", panic );
	return;
}

/**************************************************************************/
void do_password( char_data *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char *pArg;
    char *pwdnew;
	 char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	{
		ch->println("Players only.");	
		return;
	}

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println("Not going to happen.");
		return;
	}

    /*
	 * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( is_space(*argument) )
	{
		argument++;
	}

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	{
		cEnd = *argument++;
	}

    while ( *argument != '\0' )
    {
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( is_space(*argument) )
	{
		argument++;
	}

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	{
		cEnd = *argument++;
	}

	while ( *argument != '\0' )
    {
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
    }
	*pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
		ch->println("Syntax: password <old> <new>.");
		return;
    }

	if (!(ch->pcdata->overwrite_pwd && !str_cmp( "-", arg1)))
	{
		if (str_cmp( "-", ch->pcdata->pwd ) 
			&& !is_valid_password(arg1, ch->pcdata->pwd, ch->desc))			
		{
			WAIT_STATE( ch, 40 );
			ch->println("Wrong password.  Wait 10 seconds.");
			return;
		}
	}

	if ( str_len(arg2) < 5 )
    {
		ch->println("New password must be at least five characters long.");
		return;
	}

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = dot_crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
        if ( *p == '~' )
        {
			ch->println( "New password not acceptable, try again." );
			return;
		}
	}

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
	ch->pcdata->overwrite_pwd= false;
	save_char_obj( ch );
    ch->println("Ok.");
    return;
}

/**************************************************************************/
void do_lag( char_data *ch, char * argument) 
{   
	if ( IS_NULLSTR(argument) )
	{
		ch->println( "`xThis command turns lag prevention on or off." );
		ch->println( "Type 'lag on' to turn lag prevention on." );
		ch->println( "Type 'lag off' to turn lag prevention off." );
		return;
	}

	if (!str_prefix(argument, "on")) 
	{
		ch->println( "Lag prevention has now been enabled." );
		return; 
	}

	if (!str_prefix(argument, "off")) 
	{
		ch->println( "Lag prevention has now been disabled." );
		return;
	}

	ch->println( "`xThis command turns lag prevention on or off." );
	ch->println( "Type 'lag on' to turn lag prevention on." );
	ch->println( "Type 'lag off' to turn lag prevention off." );
	return;
}


/**************************************************************************/
void do_huh( char_data *ch, char * argument)
{
#ifdef unix
	msp_to_room(MSPT_ACTION, MSP_SOUND_HUH, 0, ch, true, false );
#endif
	ch->println( "Huh?" );

	// Record the moblog command
	if( IS_NPC(ch) && IS_SET(ch->act,ACT_MOBLOG) ){
		append_timestring_to_file( MOBLOG_LOGFILE, 
			FORMATF("[%5d] in room %d do_huh() - from '%s'", 
			(ch->pIndexData?ch->pIndexData->vnum:0),
			(ch->in_room? ch->in_room->vnum:0),
			argument));
	}
    return;
}

/**************************************************************************/
void do_dawnftp( char_data *ch, char *argument )
{
	pc_data *pcdata=TRUE_CH(ch)->pcdata; // the characters pcdata we want to work on
	if(!pcdata){
		ch->println("Players can only use this command");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->titlebar("DAWNFTP OPTIONS");
		ch->println("syntax: `=Cdawnftp off`x  - dawnftp is permanately off.");
		ch->wrapln( "syntax: `=Cdawnftp auto`x - mud will use dawnftp if your dawnftp is currently connected.");
		ch->wrapln( "syntax: `=Cdawnftp on`x   - dawnftp will always be used, even if "
			"you don't have a dawnftp client connected");
		ch->printlnf("Your dawnftp preference is currently set to %s", 
			preference_word(pcdata->preference_dawnftp));
		ch->wrapln("dawnftp is a superset of the mudftp system, a mudftp client can be used "
			"on dawn based muds which have the separate mudftp port enabled.");
		ch->println("DawnFtp unlike mudftp is automatically reconnected upon hotreboots.");
		ch->println("The DawnFtp client can be downloaded from http://www.dawnoftime.org/dawnftp");
		ch->titlebar("");
		return;
	}

	PREFERENCE_TYPE pt;
	if(!str_prefix(argument, "off")){
		pt=PREF_OFF;
	}else if(!str_prefix(argument, "autosense")){
		pt=PREF_AUTOSENSE;
	}else if(!str_prefix(argument, "on")){
		pt=PREF_ON;
	}else{
		ch->printlnf("Unsupported dawnftp option '%s'", argument);
		do_dawnftp(ch,"");
		return;
	}
	if(pcdata->preference_dawnftp==pt){
		ch->printlnf("Your dawnftp preference is already set to %s", preference_word(pt));
		return;
	}

	ch->printlnf("dawnftp preference changed from %s to %s", 
		preference_word(pcdata->preference_dawnftp),
		preference_word(pt));
	pcdata->preference_dawnftp=pt;
}

/**************************************************************************/
void do_objrestrict(char_data *ch, char *)
{
	OBJ_DATA *obj;
	// check for any objects that have restrictions on them, if ch has some
	// then they can't change their objrestrict status till they remove them.
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc != WEAR_NONE)
        {
			if(obj->pIndexData && obj->pIndexData->objrestrict){
				ch->wraplnf("You can't use this command while you are equipped with "
					"objects that have the objectrestriction groups on them.  "
					"Remove the objects first (%s - '%s').", 
					obj->name, obj->short_descr);
				return;
			}
		}
	}

    if (HAS_CONFIG(ch, CONFIG_OBJRESTRICT))
    {
		ch->println( "IC object restrictions disabled." );
		REMOVE_CONFIG(ch, CONFIG_OBJRESTRICT);
    }
    else
    {
		ch->println("IC object restrictions enabled.");
		SET_CONFIG(ch, CONFIG_OBJRESTRICT);
    }
}

/**************************************************************************/
void do_quester(char_data *ch, char *argument)
{
    if (IS_NPC(ch))
		return;

	// Check if they're being ordered to do this
	if ( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED ))
	{
		if ( ch->master )
			ch->master->println("Not going to happen.");
		return;
	}

	if(IS_SET(ch->act, PLR_QUESTER)){
		ch->println("You already have your quester status enabled.");
		return;
	};

	if ( str_cmp("confirm", argument)) {
		if(!codehelp(ch, "do_quester_noargument", CODEHELP_ALL_BUT_PLAYERS )){
			ch->println("If you want to enable your quester status type:");
			ch->println("  `=Cquester confirm`x.");
			ch->println("`RBE WARNED: `xThe quester status once on is not supposed to be removed.");
			ch->println("  To get it turned off you will be mooted at least -5000 rps/xp.");
		};
		return;
	}

	SET_BIT(ch->act, PLR_QUESTER);	
	if(!codehelp(ch, "do_quester_enabled", CODEHELP_ALL_BUT_PLAYERS )){
		ch->println("Your quester status has been activated.  This can not be turned off.");
		ch->println("Mudprogs that are programmed to work with only questers will recognise you now.");
	}
	
	save_char_obj(ch);
}

/**************************************************************************/
// Celrion - Oct 99 
void do_losepet( char_data *ch, char *argument)
{
    if(!ch->pet)
    {
        ch->println("You have no pet to lose!");
        return;
    }
    
	if(ch->fighting)
    {
        ch->printlnf("You can't lose your pet (%s) while fighting.", PERS(ch->pet, ch));
        return;
	}

	if (str_cmp("confirm", argument))
    {
        ch->printlnf("`xType `=Closepet confirm`x to get rid of your pet (%s).", PERS(ch->pet, ch));
		return;
	}

    REMOVE_BIT(ch->pet->act, ACT_PET);
    stop_follower(ch->pet); // removes charm and sets:
							//		ch->pet->master->pet to NULL
							//		ch->pet->master to NULL
	ch->printlnf("You have managed to lose your pet (%s).", PERS(ch->pet, ch));

}

/**************************************************************************/
void do_noheromsg( char_data *ch, char * )
{
	if ( HAS_CONFIG( ch, CONFIG_NOHEROMSG ))
	{
		ch->println( "Hero message turned on." );
		REMOVE_CONFIG( ch, CONFIG_NOHEROMSG );
	}
	else
	{
		ch->println( "Hero message turned off." );
		SET_CONFIG( ch, CONFIG_NOHEROMSG );
	}
}

/**************************************************************************/
void do_hide_hidden_areas( char_data *ch, char * )
{
	if ( HAS_CONFIG( ch, CONFIG_HIDE_HIDDEN_AREAS))
	{
		ch->println( "Hidden areas will now be seen on the area list." );
		REMOVE_CONFIG( ch, CONFIG_HIDE_HIDDEN_AREAS);
	}
	else
	{
		ch->println( "Hidden areas will no longer be seen on the area list." );
		SET_CONFIG( ch, CONFIG_HIDE_HIDDEN_AREAS);
	}
}

/**************************************************************************/
void do_history( char_data *ch, char * )
{
	if(IS_NPC(ch)){
		ch->println("Players only.");
		return;
	}

  	ch->println("Use HLOOK to view another PC's history." );
 	ch->println("Entering edit mode for you own history." );
 	ch->println("Type @ to finish editing your character history." );
	string_append(ch, &ch->pcdata->history);
  	return;
}

/**************************************************************************/
void do_charhistory( char_data *ch, char * argument)
{
	char_data *victim;
	char arg[MIL];

    if (!IS_IMMORTAL(ch) && !IS_NEWBIE_SUPPORT(ch))
    {
		do_huh(ch,"");
		return;	
    }

    // keep a track of what newbie support is doing 
    if (IS_NEWBIE_SUPPORT(ch)){
        append_newbie_support_log(ch, argument);
    }

	argument = one_argument( argument, arg );

	if(IS_NULLSTR(arg)){
        ch->println( "Syntax: charhistory <player>" );
		ch->println( "Charhistory can be used to edit another's history." );
        ch->println( "Use hlook to look at a players history." );
        return;
	}

    if ( ( victim = get_whovis_player_world( ch, arg ) ) == NULL )
    {
        ch->println( "They aren't here." );
        return;
    }

    if (!IS_TRUSTED(ch,MAX_LEVEL-4) && !IS_NPC(victim) 
		&& IS_LETGAINED(victim)
		&& IS_TRUSTED(victim,20))
    {
        ch->println( "Sorry, you can't set edit the history of a player who is letgained. "
			"Get an admin immortal to do it." );
        return;
    }

    if (IS_NEWBIE_SUPPORT(ch) && IS_NPC(victim)){
        ch->println( "Sorry, you can't set the history of a mob." );
        return;
    }


    if ((get_trust(victim)>= get_trust(ch))&& (ch != victim))
    {
        ch->println( "You can't set/edit the history of someone a higher level or equal to you." );
        return;
    }

    string_append(ch, &victim->pcdata->history);
	ch->printlnf("Editing the history of %s", PERS(victim, ch));
    return;
}
/**************************************************************************/
// Kal - Jan 04
void do_classinfo( char_data *ch, char *argument )
{
	int i, race, classindex;
	race_data *pRace;
	char titlebar[MIL];	
	bool magic_antipathy=false;
	bool not_selectable=false;
	bool customization_disabled=false;
	char col;

	if(!ch->desc){
		ch->println("You have to be connected to use this command.");
		return;
	}

	col=0;
	if(IS_NULLSTR(argument)){

		// support classedit <class> 
		ch->println(" Syntax: classinfo <class>");
		ch->println(" Where class is one of the following:");
		col=0;
		for ( i= 0; !IS_NULLSTR(class_table[i].name); i++){	
			if(!class_table[i].creation_selectable){
				continue;
			}
			ch->printf("  %-18s", class_table[i].name);
			if(++col%3==0){
				ch->println("");
			}
		}

		// support classedit <race> 
		if(ch->desc->connected_state==CON_WEB_REQUEST){			
			ch->println("  Select class information for one of the following races:");
		}else{
			ch->println(" Syntax: classinfo <race>");
			ch->println(" Where race is one of the following:");
		}
		for(pRace=race_list; pRace; pRace=pRace->next){
			if(!pRace->pc_race()){
				continue;
			}
			if(!pRace->creation_selectable()){
				continue;
			}
			if(IS_SET(pRace->flags, RACEFLAG_ALWAYS_HIDDEN_FROM_MORTAL_RACEINFO)){
				continue;
			}

			if(GAMESETTING(GAMESET_REMORT_SUPPORTED)
				&& ch->desc 
				&& ch->desc->connected_state == CON_PLAYING
				&& IS_SET(pRace->flags, RACEFLAG_HIDDEN_FROM_MORTAL_RACEINFO_WHEN_ABOVE_THEIR_REMORT)
				&& pRace->remort_number > ch->remort)
			{
				continue;
			}
			if(ch->desc && ch->desc->connected_state != CON_PLAYING
				&& ch->desc->connected_state!=CON_WEB_REQUEST
				&& pRace->remort_number > ch->desc->creation_remort_number)
			{
					continue;
			}

			if(ch->desc->connected_state==CON_WEB_REQUEST){
				ch->print(
					mxp_create_tag_core(
						FORMATF("a href=\"../classinfo/%s\"", pRace->name), 
						FORMATF("     %-18s", url_encode_post_data(capitalize(pRace->name)))
						)
					);
			}else{
				ch->printf("  %-18s", pRace->name);
			}
			if(++col%3==0){
				ch->println("");
			}
		}
		if(col%3!=0){
			ch->println("");
		}


		return;
	}

	race=pcrace_lookup(argument);
	if(race==-1){
		// look up class now
		classindex=class_lookup(argument);
		if(classindex<0){			
			ch->printlnf("ClassInfo: Couldn't find any class or race '%s'", argument);
			return;
		}

		// display the class
		ch->titlebarf("Class Base XP ratings for creation selectable races");
		int count=0;
		for( pRace=race_list; pRace; pRace=pRace->next)
		{	
			if(pRace->creation_selectable()){
				ch->printf(" `W%-17s `x%3d  ", 
					capitalize(pRace->name),
					pRace->class_exp[classindex]);
				if(++count%3==0){ ch->println(""); } // column code
			}
		}
		if(count%3!=0){ ch->println(""); } 

		// display the prime attributes
		ch->printlnf("Prime Attributes:`x  `W%s `xand `W%s`x",			
			capitalize(stat_flags[class_table[classindex].attr_prime[0]].name),
			capitalize(stat_flags[class_table[classindex].attr_prime[1]].name));

		ch->println("Note: you can also use classinfo <racename>");		

		ch->titlebar("");


		return;
	}
	pRace=race_table[race];

	if(!IS_IMMORTAL(ch)){
		if(!pRace->creation_selectable()){
			ch->printlnf("ClassInfo: Couldn't find any race '%s'", argument);
			return;
		}
		if(IS_SET(pRace->flags, RACEFLAG_ALWAYS_HIDDEN_FROM_MORTAL_RACEINFO)){
			ch->printlnf("ClassInfo: race '%s' is not available in classinfo", argument);
			return;
		}

		if(GAMESETTING(GAMESET_REMORT_SUPPORTED)
			&& ch->desc 
			&& ch->desc->connected_state == CON_PLAYING
			&& IS_SET(pRace->flags, RACEFLAG_HIDDEN_FROM_MORTAL_RACEINFO_WHEN_ABOVE_THEIR_REMORT)
			&& pRace->remort_number > ch->remort)
		{
			ch->printlnf("ClassInfo: race '%s' is not available for your remort level", argument);
			return;
		}
	}
	if(ch->desc && ch->desc->connected_state != CON_PLAYING
		&& ch->desc->connected_state!=CON_WEB_REQUEST
		&& pRace->remort_number > ch->desc->creation_remort_number)
	{
		ch->printlnf("ClassInfo: race '%s' is not available for your remort level", argument);
		return;
	}


	// by this stage, we have the race selected and into pRace

	ch->titlebarf("CLASSINFO RELATING TO THE %s RACE", uppercase(pRace->name));
	sprintf(titlebar, "`=t-=%s`=t===`=Tclass name`=t====`=Tbase xp`=t===="
		"`=Tprime attribute 1`=t==`=Tprime attribute 2`=t====-`x",
		(GAMESETTING(GAMESET_REMORT_SUPPORTED)?"`=Tremort":"====="));	
	ch->println(titlebar);

	col=0;
	for ( i= 0; !IS_NULLSTR(class_table[i].name); i++)
	{	
		class_type *cl;
		cl=&class_table[i];
		bool magic_flag=false;
		bool customization_flag=false;
		bool unselectable_flag=false;

		if(!IS_IMMORTAL(ch)){
			if(!cl->creation_selectable){
				continue;
			}
			if(pRace->class_exp[i]<1000){
				continue;
			}

			if(IS_SET(cl->flags, CLASSFLAG_ALWAYS_HIDDEN_FROM_MORTAL_CLASSINFO)){
				continue;
			}

			if(GAMESETTING(GAMESET_REMORT_SUPPORTED)
				&& ch->desc 
				&& ch->desc->connected_state == CON_PLAYING
				&& IS_SET(cl->flags, CLASSFLAG_HIDDEN_FROM_MORTAL_CLASSINFO_WHEN_ABOVE_THEIR_REMORT)
				&& cl->remort_number > ch->remort)
			{
				continue;
			}			
		}

		if(ch->desc && ch->desc->connected_state != CON_PLAYING){
			if(cl->remort_number > ch->desc->creation_remort_number)
				continue;
		}


		if(GAMESETTING(GAMESET_REMORT_SUPPORTED)){
			ch->printf("    `s%d",cl->remort_number);
		}else{
			ch->print( "     ");
		}

		if(pRace->class_exp[i]<1000){
			unselectable_flag=true;
			not_selectable=true;			
		}
		
		if(!GAMESETTING5(GAMESET5_CREATION_DISABLE_CUSTOMIZATION) 
			&& IS_SET(cl->flags, CLASSFLAG_MAGIC_ANTIPATHY))
		{
			magic_antipathy=true;
			magic_flag=true;
		}
		if(!GAMESETTING5(GAMESET5_CREATION_DISABLE_CUSTOMIZATION) 
			&& IS_SET(cl->flags, CLASSFLAG_NO_CUSTOMIZATION))
		{
			customization_disabled=true;
			customization_flag=true;
		}
		
		ch->printlnf("`x%16s%s%s%s%s%s%6d       %-16s   %-16s", 
			capitalize(cl->name),
			unselectable_flag?"`Y#`x":"",
			magic_flag?"`Y*`x":"",
			customization_flag?"`Y-`x":" ",
			unselectable_flag?"":" ", // put the space which we skipped above
			magic_flag?"":" ", // put the space which we skipped above
			pRace->class_exp[i],
			capitalize(stat_flags[cl->attr_prime[0]].name),
			capitalize(stat_flags[cl->attr_prime[1]].name)
			);
	}
	if(not_selectable){
		ch->println("  `Y#`x = class can't be selected in creation (base xp<1000)");
	}
	if(magic_antipathy ){
		ch->println("  `Y*`x = this class can't use magical items");
	}
	if(customization_disabled ){
		ch->println("  `Y-`x = this class has no advanced customization available");
	}
	ch->println("  Note: The prime attributes are the same for every class/race combination");
	ch->println("  Note2: you can also use classinfo <classname>");
	ch->titlebar("");
	

	if(ch->desc->connected_state==CON_WEB_REQUEST){
		ch->print_blank_lines(1);
		do_classinfo(ch, "");
	}

}
/**************************************************************************/
/**************************************************************************/

