/**************************************************************************/
// handler.cpp - Utility functions etc
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
#include "msp.h"
#include "ictime.h"
#include "pload.h"
#include "lockers.h"
#include "events.h"

// command procedures needed 
DECLARE_DO_FUN(do_return	);
void set_char_magic_bits(char_data * ch);
void affect_copy_to_char( char_data *ch, AFFECT_DATA *paf );
char *flag_string( const struct flag_type *flag_table, int bits);
/**************************************************************************/
/*
 * Local functions.
 */
void	affect_modify	args( ( char_data *ch, AFFECT_DATA *paf, bool fAdd ) );
void	room_update( AREA_DATA *pArea );
void	room_aff_update( ROOM_INDEX_DATA *room );

void bash_eq(char_data *ch, int chance)
{
  OBJ_DATA *obj, *obj_next;

  for(obj = ch->carrying; obj != NULL; obj = obj_next)        
        {
            obj_next = obj->next_content;
            if(obj->condition>0 && number_percent()<=chance+20 &&
              !IS_SET(obj->extra_flags ,OBJEXTRA_NO_DEGRADE))
		obj->condition-=1;
        }
  return;
}
/**************************************************************************/
// return the bit vector table relating to a given where location - Kal
const flag_type *affect_get_bitvector_table_for_where(int where)
{
	switch(where)
	{
	case WHERE_AFFECTS:	return affect_flags;	// character
	case WHERE_AFFECTS2:	return affect2_flags;	// character
	case WHERE_AFFECTS3:	return affect3_flags;	// character
	case WHERE_IMMUNE:		return imm_flags;		// character
	case WHERE_RESIST:		return res_flags;		// character
	case WHERE_VULN:		return vuln_flags;		// character
	case WHERE_OBJEXTRA:	return objextra_flags;	// object
	case WHERE_OBJEXTRA2:	return objextra2_flags;	// object
	case WHERE_WEAPON:		return weapon_flags;	// object
	case WHERE_OBJECTSPELL:return objspell_flags;	// object spell affect
	case WHERE_MODIFIER:	break;
	case WHERE_SKILLS:		break;
	default:
		bugf("affect_get_bitvector_table_for_where(): "
			"Unsupported where value of %d.", where);
		do_abort();
		break;
	}
	return NULL;
}
/**************************************************************************/
/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    char_data *fch;
    int count = 0;

    if(obj->in_room == NULL)
	return 0;

    for(fch = obj->in_room->people; fch; fch = fch->next_in_room){
		if(fch->on == obj){
			count++;
		}
	}

    return count;
}

/**************************************************************************/
int num_enemies(char_data *ch)
{
	char_data *fch;
 	int count=0;

	for(fch = ch->in_room->people; fch != NULL; fch = fch->next_in_room)
	 if(fch->fighting==ch)
          count+=1;

        return count;
}   

/**************************************************************************/
int weapontype (const char *name) 
{ 
   return flag_value(weapon_class_types, (char *)name); 
}

/**************************************************************************/
char *get_weapontype(OBJ_DATA *obj)
{
	if( obj->item_type == ITEM_WEAPON ){
		return flag_string( weapon_class_types, obj->value[0] );
	}

	return NULL;
}

/**************************************************************************/
int item_lookup(const char *name)
{
    int type;

    for(type = 0; item_table[type].name != NULL; type++)
    {
        if(LOWER(name[0]) == LOWER(item_table[type].name[0])
        &&  !str_prefix(name,item_table[type].name))
            return item_table[type].type;
    }
 
    return -1;
}

/**************************************************************************/
char *item_name(int item_type)
{
//	return(flag_string(item_types, item_type));
    int type;

    for(type = 0; item_table[type].name != NULL; type++)
	if(item_type == item_table[type].type)
	    return item_table[type].name;
    return "none";
}

/**************************************************************************/
char *weapon_name( int weapon_type)
{
	return flag_string(weapon_class_types, weapon_type);
}

/**************************************************************************/
/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */
int check_immune(char_data *ch, int dam_type)
{
    int immune, def;
    int bit;

    immune = -1;
    def = IS_NORMAL;

    if(dam_type == DAM_NONE){
		return immune;
	}

    if(dam_type <= 3)
    {
		if(IS_SET(ch->imm_flags,IMM_WEAPON)){
			def = IS_IMMUNE;
		}else if (IS_SET(ch->res_flags,RES_WEAPON)){
			def = IS_RESISTANT;
		}else if (IS_SET(ch->vuln_flags,VULN_WEAPON)){
			def = IS_VULNERABLE;
		}
    }
    else // magical attack 
    {	
		if(IS_SET(ch->imm_flags,IMM_MAGIC)){
			def = IS_IMMUNE;
		}else if (IS_SET(ch->res_flags,RES_MAGIC)){
			def = IS_RESISTANT;
		}else if (IS_SET(ch->vuln_flags,VULN_MAGIC)){
			def = IS_VULNERABLE;
		}
    }

    // set bits to check -- VULN etc. must ALL be the same or this will fail 
    switch(dam_type)
    {
	case(DAM_BASH):			bit = IMM_BASH;		break;
	case(DAM_PIERCE):		bit = IMM_PIERCE;	break;
	case(DAM_SLASH):		bit = IMM_SLASH;	break;
	case(DAM_FIRE):			bit = IMM_FIRE;		break;
	case(DAM_COLD):			bit = IMM_COLD;		break;
	case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;break;
	case(DAM_ACID):			bit = IMM_ACID;		break;
	case(DAM_POISON):		bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):		bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):			bit = IMM_HOLY;		break;
	case(DAM_ENERGY):		bit = IMM_ENERGY;	break;
	case(DAM_MENTAL):		bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):		bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):		bit = IMM_DROWNING;	break;
	case(DAM_LIGHT):		bit = IMM_LIGHT;	break;
	case(DAM_CHARM):		bit = IMM_CHARM;	break;
	case(DAM_SOUND):		bit = IMM_SOUND;	break;
	case(DAM_ILLUSION):		bit = IMM_ILLUSION;	break;
	default:		return def;
    }

    if(IS_SET(ch->imm_flags,bit)){
		immune = IS_IMMUNE;
	}
    if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE){
		immune = IS_RESISTANT;
    }
	
	if (IS_SET(ch->vuln_flags,bit)){
		if(immune == IS_IMMUNE){
			immune = IS_RESISTANT;
		}else if (immune == IS_RESISTANT){
			immune = IS_NORMAL;
		}else{
			immune = IS_VULNERABLE;
		}
    }

    if(immune == -1){
		return def;
    }else{
      	return immune;
	}
}

/**************************************************************************/
bool is_clan(char_data *ch)
{   
    return(ch->clan>0);
}

/**************************************************************************/
bool is_same_clan(char_data *ch, char_data *victim)
{
    return(ch->clan  && ch->clan == victim->clan );
}

/**************************************************************************/
// for returning skill information 
int get_skill(char_data *ch, int sn)
{
	return(ch->get_skill(sn));
}

/**************************************************************************/
// for returning weapon information 
int get_weapon_sn(char_data *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_WIELD );
    if(wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
        default :               sn = -1;                break;
        case(WEAPON_SPEAR):     sn = gsn_spear;         break;
        case(WEAPON_SWORD):     sn = gsn_sword;         break;
        case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
        case(WEAPON_STAFF):     sn = gsn_staff;         break;
        case(WEAPON_MACE):      sn = gsn_mace;          break;
        case(WEAPON_AXE):       sn = gsn_axe;           break;
        case(WEAPON_FLAIL):     sn = gsn_flail;         break;
        case(WEAPON_WHIP):      sn = gsn_whip;          break;
        case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
		case(WEAPON_SICKLE):	sn = gsn_sickle;		break;
   }
   return sn;
}

/**************************************************************************/
int get_weapon_skill(char_data *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if(IS_NPC(ch))
    {
	if(sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else 
	    skill = 40 + 5 * ch->level / 2;
    }
    
    else
    {
	if(sn == -1)
	    skill = 3 * ch->level;
	else
	    skill = ch->pcdata->learned[sn];
    }

    return URANGE(0,skill,100);
} 


/**************************************************************************/
// used to de-screw characters 
void reset_char(char_data *ch)
{
    int loc,mod,stat;
    OBJ_DATA *obj;
    AFFECT_DATA *af;
    int i;
	int ac_amount;

    if(IS_NPC(ch))
        return;

	// THIS FIRST SECTION OF CODE IS ONLY RUN ON NEWBIES OR CORRUPTED PFILES
    if(    ch->pcdata->perm_hit   == 0 
        ||  ch->pcdata->perm_mana  == 0
        ||  ch->pcdata->perm_move  == 0
        ||  (ch->pcdata->last_level == 0 && ch->played==0))
    {   // do a FULL reset  - START of full reset
        for(loc = 0; loc < MAX_WEAR; loc++)
        {  // start of wear location loop 
            obj = get_eq_char(ch,loc);
            if(obj == NULL)
                continue;

			if( obj->wear_loc == WEAR_SHEATHED
			||   obj->wear_loc == WEAR_CONCEALED )
				continue;

			for(af=OBJECT_AFFECTS(obj); af; af = af->next )
            {
                if(af->level > ch->level) continue;
                mod = af->modifier;
                switch(af->location)
                {
                    case APPLY_SEX:
                        ch->sex -= mod;
                        if(ch->sex < 0 || ch->sex >2)
                            ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
                        break;
                    case APPLY_MANA: ch->max_mana -= mod;   break;
                    case APPLY_HIT:  ch->max_hit  -= mod;   break;
                    case APPLY_MOVE: ch->max_move -= mod;   break;
					
					default: break; // do nothing for the rest of the conditions
                }
            }
        }  // end of wear location loop

        // now reset the permanent stats
        ch->pcdata->perm_hit    = ch->max_hit;
        ch->pcdata->perm_mana   = ch->max_mana;
        ch->pcdata->perm_move   = ch->max_move;
        ch->pcdata->last_level  = ch->played/3600;
        if(ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
        {
            if(ch->sex > 0 && ch->sex < 3)
                ch->pcdata->true_sex    = ch->sex;
            else
                ch->pcdata->true_sex    = 0;
        }
    } // do a FULL reset  - END of full reset


    // BELOW HERE IS RUN ON EVERY CHARACTER WHEN THEY CONNECT AND LEVEL
	
	// should do invulnerabilities etc in here? or just above

	// calculate their object restriction groupings
	if(ch->pcdata){
		int index;
		ch->pcdata->objrestrict=0;
		for(index=0; !IS_NULLSTR(classgroup_table[index].name); index++){
			if(is_exact_name(class_table[ch->clss].name,classgroup_table[index].text_members)){
				ch->pcdata->objrestrict|= (1<<classgroup_table[index].bitindex);
			}
		}
	}

	// update their body parts and form
	ch->form	= race_table[ch->race]->form;
	ch->parts	= race_table[ch->race]->parts;

    // now restore the character to his/her true condition 
    for(stat = 0; stat < MAX_STATS; stat++)
    {
        ch->modifiers[stat]=0;
		if(GAMESETTING(GAMESET_USE_ROLEMASTER_MODIFIERS)){
			if(ch->perm_stats[stat] > 90){
				ch->modifiers[stat] = (ch->perm_stats[stat]-90)*2+20;
			}else if(ch->perm_stats[stat] > 70){
				ch->modifiers[stat] = ch->perm_stats[stat]-70;
			}else if(ch->perm_stats[stat] < 26){
				ch->modifiers[stat] = -26 + ch->perm_stats[stat]; 
			}
		}else{
			if(ch->perm_stats[stat] > 95){
				ch->modifiers[stat] = (ch->perm_stats[stat]-95)*3+45;
			}else if(ch->perm_stats[stat] > 85){
				ch->modifiers[stat] = (ch->perm_stats[stat]-85)*2+25;
			}else if(ch->perm_stats[stat] > 60){
					ch->modifiers[stat] = ch->perm_stats[stat]-60;
			}else if(ch->perm_stats[stat] < 26){
					ch->modifiers[stat] = -26 + ch->perm_stats[stat];
			}
		}
        ch->modifiers[stat] += race_table[ch->race]->stat_modifier[stat];
    }
    
    
    if(ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2){
        ch->pcdata->true_sex = 0;  // reset true sex if it is stuffed
	}
    ch->sex     = ch->pcdata->true_sex;
    ch->max_hit     = ch->pcdata->perm_hit;
    ch->max_mana    = ch->pcdata->perm_mana;
    ch->max_move    = ch->pcdata->perm_move;
       
    for(i = 0; i < 4; i++)
    {
		if(HAS_CLASSFLAG(ch, CLASSFLAG_LEVEL_BASED_AC)){
			ch->armor[i]    = 100-4*ch->level;
		}else{
            ch->armor[i]    = 100;
		}
    }
    
    ch->hitroll     = 0;
    ch->damroll     = 0;
    ch->saving_throw    = 0;
	
	// setup their magic bits
	set_char_magic_bits(ch);

    // now start adding back the effects of objects (excluding paf->where==WHERE_OBJECTSPELL affects)
    for(loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch,loc);
        if(obj == NULL)
            continue;

		if( obj->wear_loc == WEAR_SHEATHED
		||   obj->wear_loc == WEAR_CONCEALED )
			continue;

		for(i = 0; i < 4; i++)
		{
			ac_amount = apply_ac( obj, loc, i );
			if(ch->level< obj->level)
				ac_amount = ac_amount * ch->level/ obj->level;
			ch->armor[i] -= ac_amount;
		}

		for(af = OBJECT_AFFECTS(obj); af; af = af->next)
		{
			if(af->level > ch->level ){
				continue;
			}
			if(af->where!=WHERE_OBJECTSPELL){
				mod = af->modifier;
				switch(af->location)
				{
					case APPLY_ST:  ch->modifiers[STAT_ST]  += mod; break;
					case APPLY_QU:  ch->modifiers[STAT_QU]  += mod; break;
					case APPLY_PR:  ch->modifiers[STAT_PR]  += mod; break;
					case APPLY_EM:  ch->modifiers[STAT_EM]  += mod; break;
					case APPLY_IN:  ch->modifiers[STAT_IN]  += mod; break;
					case APPLY_CO:  ch->modifiers[STAT_CO]  += mod; break;
					case APPLY_AG:  ch->modifiers[STAT_AG]  += mod; break;
					case APPLY_SD:  ch->modifiers[STAT_SD]  += mod; break;
					case APPLY_ME:  ch->modifiers[STAT_ME]  += mod; break;
					case APPLY_RE:  ch->modifiers[STAT_RE]  += mod; break;      
					case APPLY_SEX:         ch->sex         += mod; break;
					case APPLY_MANA:        ch->max_mana    += mod; break;
					case APPLY_HIT:         ch->max_hit     += mod; break;
					case APPLY_MOVE:        ch->max_move    += mod; break;
					case APPLY_AC:   
						for(i = 0; i < 4; i ++)
							ch->armor[i] += mod; 
						break;
					case APPLY_HITROLL:     ch->hitroll     += mod; break;
					case APPLY_DAMROLL:     ch->damroll     += mod; break;
					// saving throws 
					case APPLY_SAVES:         ch->saving_throw += mod; break;

					default: break; // do nothing for the rest of the conditions
				}			
			}
		}

		// IC object restriction system - Kal
		if(obj->pIndexData && ch->pcdata
			&& HAS_CONFIG(ch,CONFIG_OBJRESTRICT) 
			&& ((ch->pcdata->objrestrict& obj->pIndexData->objrestrict)>0))
		{
			OBJRESTRICT_LIST_DATA *pr;
			AFFECT_DATA aff;
			int top_prority=-1;
			for( pr = obj->pIndexData->restrict; pr; pr = pr->next )
			{
				if(IS_SET(ch->pcdata->objrestrict,(1<<pr->classgroup->bitindex))){
					top_prority=UMAX(pr->priority, top_prority);
				}
				
			}

			// IC object restriction system - Kal
			for( pr = obj->pIndexData->restrict; pr; pr = pr->next )
			{
				if(top_prority!=pr->priority 
					||	!IS_SET(ch->pcdata->objrestrict,(1<<pr->classgroup->bitindex)))
				{
					continue;
				}
				aff.where		= WHERE_RESTRICT;
				aff.type		= -1;
				aff.level		= 0;
				aff.duration	= -1;
				aff.location	= pr->affectprofile->wear_location;
				aff.modifier	= pr->affectprofile->wear_amount;
				aff.bitvector	= 0;
				// do the act text
//				act(pr->affectprofile->wear_message,ch,obj,NULL,TO_CHAR);	
				// add the affect
    			affect_modify( ch, &aff, true );

				if(top_prority==-1) // -1 means only the first affectprofile is applied
					break;		
			}
		}
    }
  
    // now add back effects on the character 
    // now add back spell effects on the character
    for(af = ch->affected; af; af = af->next){
        mod = af->modifier;
        switch(af->location)
        {
			case APPLY_ST:  ch->modifiers[STAT_ST]  += mod; break;
			case APPLY_QU:  ch->modifiers[STAT_QU]  += mod; break;
			case APPLY_PR:  ch->modifiers[STAT_PR]  += mod; break;
			case APPLY_EM:  ch->modifiers[STAT_EM]  += mod; break;
			case APPLY_IN:  ch->modifiers[STAT_IN]  += mod; break;
			case APPLY_CO:  ch->modifiers[STAT_CO]  += mod; break;
			case APPLY_AG:  ch->modifiers[STAT_AG]  += mod; break;
			case APPLY_SD:  ch->modifiers[STAT_SD]  += mod; break;
			case APPLY_ME:  ch->modifiers[STAT_ME]  += mod; break;
			case APPLY_RE:  ch->modifiers[STAT_RE]  += mod; break;      
			case APPLY_SEX:         ch->sex         += mod; break;
			case APPLY_MANA:        ch->max_mana    += mod; break;
			case APPLY_HIT:         ch->max_hit     += mod; break;
			case APPLY_MOVE:        ch->max_move    += mod; break;
			case APPLY_AC:   
				for(i = 0; i < 4; i ++)
					ch->armor[i] += mod; 
				break;
			case APPLY_HITROLL:     ch->hitroll     += mod; break;
			case APPLY_DAMROLL:     ch->damroll     += mod; break;
			// saving throws 
			case APPLY_SAVES:         ch->saving_throw += mod; break;

			default: break; // do nothing for the rest of the conditions
		} 
    }
	ch->sex=URANGE(0, ch->sex, 3); // crash proof sex modifiers
}


/**************************************************************************/
// Retrieve a character's trusted level for permission checking.
// if someone is switched, their trust value is returned
int get_trust( char_data *ch )
{
    if(!ch)
    {
       log_string("BUG: in get_trust - ch was NULL");
        return 0;
    }

    if( TRUE_CH(ch)->trust)
		return TRUE_CH(ch)->trust;

    if( IS_NPC(TRUE_CH(ch)) && ch->level >= LEVEL_HERO )
		return LEVEL_HERO - 1;
    else
		return TRUE_CH(ch)->level;
}

/**************************************************************************/
// Retrieve a character's carry capacity.
int can_carry_n( char_data *ch )
{
    if( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 10000;

    if( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) ){
		return 50;
	}
    return(MAX_WEAR +  2 * (ch->modifiers[STAT_ST]+28) + ch->level/5 + 10)*10;
    
}

/**************************************************************************/
// Retrieve a character's carry capacity.
int can_carry_w( char_data *ch )
{
    if( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 10000000;

    if( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) && !IS_CONTROLLED(ch))
	return 10;

    return((ch->modifiers[STAT_ST]+28) * 10 + ch->level * 5)*10;
}


/**************************************************************************/
// See if a string is one of the names of an object.
bool is_name ( const char *str, const char *namelist )
{
    char name[MSL*2], part[MSL*2];
	const char *list, *string;

	if(IS_NULLSTR(namelist)){
		return false;
	}

    string = str;
    // we need ALL parts of string to match part of namelist 
    for( ; ; )  // start parsing string 
    {
		str = one_argument(str,part);

		if(part[0] == '\0' )
			return true;

		// check to see if this is part of namelist 
		list = namelist;
		for( ; ; )  // start parsing namelist 
		{
			list = one_argument(list,name);
			if(name[0] == '\0')  // this name was not found 
			return false;

			if(!str_prefix(string,name))
			return true; // full pattern match

			if(!str_prefix(part,name))
			break;
		}
    }
}
/**************************************************************************/
// See if a string has one of the names infix 
bool is_name_infix( const char *str, const char *namelist )
{
    char name[MSL*2], part[MSL*2];
    const char *list, *string;

	if(IS_NULLSTR(namelist))
		return false;

    string = str;
    // we need parts of string to match part of namelist 
    for( ; ; )  // start parsing string 
    {
		str = one_argument(str,part);

		if(part[0] == '\0' )
			return true;

		// check to see if this is part of namelist 
		list = namelist;
		for( ; ; )  
		{
			list = one_argument(list,name);
			if(name[0] == '\0')  
			return false;

			if(!str_infix(string,name))
			return true; 

			if(!str_infix(part,name))
			break;
		}
    }
}

/**************************************************************************/
// return true if name str is in the name list
bool is_exact_name( const char *str, const char *namelist )
{
    char name[MSL*2];

	if(IS_NULLSTR(namelist))
		return false;

	if(IS_NULLSTR(str))
		return false;

	if(!str_cmp(str, namelist)){
		return true;
	}

    for( ; ; )
    {
        namelist = one_argument( namelist, name );
        if( name[0] == '\0' )
            return false;
        if( !str_cmp( str, name ) )
            return true;
    }
}

/**************************************************************************/
// move the affects from the olc template of an object to the actual
// object - used when enchanting etc.
void affects_from_template_to_obj(OBJ_DATA *obj)
{
    if(obj->affected || obj->no_affects)
    {
		// previously got affects from template or has been disenchanted
		return;
	}

    // okay, move all the old flags into new vectors if we have to 
    AFFECT_DATA *paf, *af_new;
    for(paf = obj->pIndexData->affected; paf; paf = paf->next) // affects_from_template_to_obj
    {
		af_new = new_affect();

        af_new->next = obj->affected;
        obj->affected = af_new;

		af_new->where		= paf->where;
        af_new->type        = UMAX(0,paf->type);
        af_new->level       = paf->level;
        af_new->duration    = paf->duration;
        af_new->location    = paf->location;
        af_new->modifier    = paf->modifier;
        af_new->bitvector   = paf->bitvector;
    }
}
           
/**************************************************************************/
// Reduces the wealth on a mob if it is above the highest it could 
// possibly be rolled to.
// Notes:
//	Used to reduce a mobs money when they are in roles that a lot of players
//	give them money - e.g. bribe triggers for mudprogs, healers, money 
//	changers etc... (even though a lot of the code already does 'automated
//	banking' on some of the above.  - Kal May 1999
//  This system prevents players from subduing or killing these mobs for 
//	their money
void limit_mobile_wealth(char_data *mob)
{
	// can't be called on players
	if(!IS_NPC(mob))
		return;

	// get their mobindex
	MOB_INDEX_DATA *pMobIndex=mob->pIndexData;

	// must have a valid index data
	if(!pMobIndex)
		return;

	// get the maximum wealth value they could roll when the mob is 
	// created by create_mobile()
	long max_wealth= 3 * pMobIndex->wealth/2;

	long current_wealth= (mob->gold*100) + mob->silver;

	// if their wealth is greater than the max possible 
	// reroll it using similar code as in create_mobile()
	if(current_wealth>max_wealth){
		// limit the mobs wealth to its wealth value
		long wealth = number_range(pMobIndex->wealth, 3 * pMobIndex->wealth/2);
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
	}
}
/**************************************************************************/
extern char *target_name;
/**************************************************************************/
// Cast a spell onto a player (by an object)
// - should only be called from affect_modify() 
static void affect_add_objectspell(char_data *ch, AFFECT_DATA *paf)
{
	// the assumption made
	assert(paf->where==WHERE_OBJECTSPELL);

	int sn=paf->type;
	if(sn<FIRST_SPELL || sn>LAST_SPELL){
		bugf("affect_add_castspell(): sn==%d, which isn't a spell sn!", sn);
		return;	
	}

	// see if they already have a spell based on that 
	// spell function at a higher level
	AFFECT_DATA *paf1;
	SPELL_FUN * spell_fun= skill_table[sn].spell_fun;
	if(!spell_fun || spell_fun==spell_null){
		bugf("affect_add_castspell(): No spell function for sn==%d '%s'.",
			sn, skill_table[sn].name);
		REMOVE_BIT(paf->bitvector,OBJSPELL_ACTIVE);
		return;
	}
    for( paf1= ch->affected; paf1; paf1= paf1->next ) {
		if(paf1->type>=FIRST_SPELL && paf1->type<=LAST_SPELL){
			if(skill_table[paf1->type].spell_fun == spell_fun 
				&& paf1!=paf && paf1->level>=paf->level)
			{
				// found an existing objectspell with same spell function 
				// at higher or equal level
				REMOVE_BIT(paf->bitvector,OBJSPELL_ACTIVE);
				return;
			}
		}
	}

	affect_parentspellfunc_strip( ch, sn);	// remove all previous spells based on spell sn
	AFFECT_DATA *stop=ch->affected;			// backup where the affects are currently
	// cast the new spell on the player
	target_name = str_dup(ch->name);
	(*skill_table[sn].spell_fun) ( sn, paf->level, ch, ch, TARGET_CHAR);	
	free_string(target_name);
	target_name=NULL;
	// affect is considered active
	SET_BIT(paf->bitvector,OBJSPELL_ACTIVE);

	// now find the affects from the spell just cast and patch the duration accordingly
	if(paf->duration!=0){
		AFFECT_DATA *paf_patch;
		for(paf_patch=ch->affected; paf_patch && paf_patch!=stop; paf_patch=paf_patch->next){
			if(paf_patch->type==sn){
				paf_patch->duration=paf->duration;

			}
		}
//			assert(paf_patch);	// this shouldn't go false unless some spell is
								// removing paf from ch->affected which shouldn't 
								// be the case since paf->where==WHERE_OBJECTSPELL
					// unless an object has been equipped twice - booting up
	}	
}
/**************************************************************************/
// Remove a spell from a player, that was cast on it by an object
// - should only be called from affect_modify()
// - should only remove the spell if it is exactly matching
//   and there are no other objects on the player which would 
//   cast the spell at the same level.  If another object is there
//   that will cast the object at a different level, remove then recast.
// - because affect_add_castspell() will only of put this spell on
//   if it was the highest level at the time, we don't have to check 
//   for spells above us.
static void affect_remove_objectspell(char_data *ch, AFFECT_DATA *paf)
{
	assert(paf->where==WHERE_OBJECTSPELL);

	// if the affect isn't active, then we don't have any spells to remove
	// that relate to this spell
	if(!IS_SET(paf->bitvector,OBJSPELL_ACTIVE)){
		return;
	}

	int sn=paf->type;
	// make sure we are dealing with a valid spell
	if(sn<FIRST_SPELL || sn>LAST_SPELL){
		bugf("affect_remove_castspell(): sn==%d, which isn't a spell sn!", sn);
		do_abort(); // abort here - how did they get it on in the first place			
	}

	// find the affects of the particular sn and remove them accordingly
	AFFECT_DATA *lpaf, *lpaf_next;
	bool show_msg =true;
	for(lpaf=ch->affected; lpaf; lpaf=lpaf_next){		
		lpaf_next=lpaf->next;
		if(lpaf->where!=WHERE_OBJECTSPELL && lpaf->type==sn){
			affect_remove(ch, lpaf);
			if(show_msg && !IS_NULLSTR(skill_table[sn].msg_off))
			{						
				ch->printf("%s\r\n",skill_table[sn].msg_off);
				show_msg = false; // only display the fade out message once
			}
		}
	}

	// check if there are any other objectspells which is to replace 
	// the spell just removed, if there are find the highest level casting one
	AFFECT_DATA *raf; // replacement affect
	AFFECT_DATA *match=NULL; // matching affect
	for(raf=ch->affected; raf; raf=raf->next){
		if( raf!=paf
			&& raf->where==WHERE_OBJECTSPELL
			&& raf->type== paf->type)
		{
			if(match){
				if(raf->level>match->level){
					match=raf; // take the higher level affect
				}
			}else{
				match=raf;
			}
		}
	}
	if(match){
		affect_modify( ch, match, true); // cast the replacement 
	}
}
/**************************************************************************/
void affect_apply_modifier( char_data *ch, AFFECT_DATA *paf, bool fAdd )						   
{
	int i;
	int amount=(fAdd?paf->modifier:-paf->modifier);
	switch( paf->location )
	{
	default:
		bugf( "affect_apply_modifier(): unknown location %d.", paf->location );
		do_abort(); // should never be in this situation
		return;
		
	case APPLY_NONE:		break;
	case APPLY_CLASS:		break;
	case APPLY_LEVEL:		break;
	case APPLY_AGE: 		break;
	case APPLY_HEIGHT:		break;
	case APPLY_WEIGHT:		break;
	case APPLY_GOLD:		break;
	case APPLY_EXP: 		break;
		
	case APPLY_ST:	 ch->modifiers[STAT_ST] += amount; break;
	case APPLY_QU:	 ch->modifiers[STAT_QU] += amount; break;
	case APPLY_PR:	 ch->modifiers[STAT_PR] += amount; break;
	case APPLY_EM:	 ch->modifiers[STAT_EM] += amount; break;
	case APPLY_IN:	 ch->modifiers[STAT_IN] += amount; break;
	case APPLY_CO:	 ch->modifiers[STAT_CO] += amount; break;
	case APPLY_AG:	 ch->modifiers[STAT_AG] += amount; break;
	case APPLY_SD:	 ch->modifiers[STAT_SD] += amount; break;
	case APPLY_ME:	 ch->modifiers[STAT_ME] += amount; break;
	case APPLY_RE:	 ch->modifiers[STAT_RE] += amount; break;		
		
	case APPLY_SEX: 		  
		if(fAdd){ 
			ch->sex+= amount;
		}else{
			// revert to default sex don't worry if they had 
			// multiple objects on them modifying their sex.
			if(IS_NPC(ch)){
				ch->sex = ch->pIndexData->sex;
				if(ch->sex == 3){ // random sex 
					ch->sex = number_range(1,2);
				}
			}else{
				ch->sex= ch->pcdata->true_sex;
			}		
		}
		ch->sex=URANGE(0, ch->sex, 3); // crash proof sex modifiers
		break;
		
	case APPLY_MANA:		
		ch->max_mana+= amount; 
		ch->mana+= amount; 
		break;
	case APPLY_HIT: 		
		ch->max_hit+= amount; 
		if(  !fAdd 
			&& (GAMESETTING3(GAMESET3_ALWAYS_NO_NEGATIVE_HP_AT_AFFECTOFF) || 
			(paf->type>=0 
			&& paf->type<MAX_SKILL
			&& IS_SET(skill_table[paf->type].flags, SKFLAGS_NO_NEGATIVE_HP_AT_AFFECTOFF))))
		{
			if(ch->hit>0){
				ch->hit+= amount; 
				ch->hit=UMAX(1,ch->hit); // no death from spell when it wears off
			}else{
				ch->hit+= amount; 
			}
		}else{
			ch->hit+= amount; 
		}
		
		break;
	case APPLY_MOVE:		
		ch->max_move+= amount; 
		ch->move+= amount; 
		break;
	case APPLY_AC:			
		for(i = 0; i < 4; i ++){
			ch->armor[i] += amount;
		}
		break;
		
	case APPLY_HITROLL: 	ch->hitroll+= amount; break;
	case APPLY_DAMROLL: 	ch->damroll+= amount; break;
		
	case APPLY_SAVES:		ch->saving_throw+= amount; break;
		
	}
}
/**************************************************************************/
// Apply or remove an affect to a character.
void affect_modify( char_data *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield= NULL;
	
    if( fAdd )
    {
		switch(paf->where)
		{
		case WHERE_OBJECTSPELL:
			affect_add_objectspell(ch,paf);
			break;		
		// if the object is set to affect skills
		case WHERE_SKILLS:
			affect_to_skill( ch, paf->type, paf->modifier );
			break;
		case WHERE_AFFECTS:
			SET_BIT(ch->affected_by, paf->bitvector);
			break;
		case WHERE_AFFECTS2:
			SET_BIT(ch->affected_by2, paf->bitvector);
			break;
		case WHERE_AFFECTS3:
			SET_BIT(ch->affected_by3, paf->bitvector);
			break;
		case WHERE_IMMUNE:
			SET_BIT(ch->imm_flags,paf->bitvector);
			break;
		case WHERE_RESIST:
			SET_BIT(ch->res_flags,paf->bitvector);
			break;
		case WHERE_VULN:
			SET_BIT(ch->vuln_flags,paf->bitvector);
			break;			
		}
    }else{ // removing the affect
        switch(paf->where)
        {
		case WHERE_OBJECTSPELL:
			affect_remove_objectspell(ch,paf);
			break;		
		case WHERE_SKILLS:
			affect_to_skill( ch, paf->type, -paf->modifier );
			break;
        case WHERE_AFFECTS:
            REMOVE_BIT(ch->affected_by, paf->bitvector);
            break;
        case WHERE_AFFECTS2:
            REMOVE_BIT(ch->affected_by2, paf->bitvector);
            break;
        case WHERE_AFFECTS3:
            REMOVE_BIT(ch->affected_by3, paf->bitvector);
            break;
		case WHERE_IMMUNE:
            REMOVE_BIT(ch->imm_flags,paf->bitvector);
            break;
        case WHERE_RESIST:
            REMOVE_BIT(ch->res_flags,paf->bitvector);
            break;
        case WHERE_VULN:
            REMOVE_BIT(ch->vuln_flags,paf->bitvector);
            break;
        }
    }

	// perform the modifiers of the affect
	affect_apply_modifier(ch, paf, fAdd);
	
    // Check for weapon wielding.
	// Guard against recursion (for weapons with affects).
    if( !IS_NPC(ch) 
		&& ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
		&&   get_obj_weight(wield) > (ch->modifiers[STAT_ST]+20)*10 
		&& ch->position != POS_DEAD )
    {
		static int depth;
		
		if( depth == 0 )
		{
			depth++;
			act( "You drop $p.", ch, wield, NULL, TO_CHAR );
			act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
			obj_from_char( wield );
			obj_to_room( wield, ch->in_room );
			depth--;
		}
    }
	update_pos(ch);
	return;
}

/**************************************************************************/
// find an effect in an affect list
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
    AFFECT_DATA *paf_find;
    
    for( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
        if( paf_find->type == sn )
	return paf_find;
    }

    return NULL;
}

/**************************************************************************/
// fix object affects when removing one 
void affect_check(char_data *ch, int where, int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
	
	if(vector == 0 || flag_value(to_types, flag_string(to_types, where)) == NO_FLAG){
		// flag_value(to_types, flag_string(to_types, where)) will return NO_FLAG
		// if the where location relates to a table that isn't relevant to characters
		return;
	}

	// loop thru a players current affects, resetting a bit if necessary
    for(paf = ch->affected; paf; paf = paf->next){
		if(paf->where == where && paf->bitvector == vector)
		{
			switch(where)
			{
			case WHERE_AFFECTS:
				SET_BIT(ch->affected_by, paf->bitvector);
				break;
			case WHERE_AFFECTS2:
				SET_BIT(ch->affected_by2, paf->bitvector);
				break;
			case WHERE_AFFECTS3:
				SET_BIT(ch->affected_by3, paf->bitvector);
				break;
			case WHERE_IMMUNE:
				SET_BIT(ch->imm_flags,paf->bitvector);
				break;
			case WHERE_RESIST:
				SET_BIT(ch->res_flags,paf->bitvector);
				break;
			case WHERE_VULN:
				SET_BIT(ch->vuln_flags,paf->bitvector);
				break;			
			}
			return;
		}
	}

	// loop thru what a player is wearing, resetting a bit if necessary
    for(obj = ch->carrying; obj; obj = obj->next_content)
    {
		if(obj->wear_loc == WEAR_NONE)
			continue;
		
		for(paf = OBJECT_AFFECTS(obj); paf; paf = paf->next){
            if(paf->where == where && paf->bitvector == vector){
                switch(where)
                {
				case WHERE_AFFECTS:
					SET_BIT(ch->affected_by, paf->bitvector);
					break;
				case WHERE_AFFECTS2:
					SET_BIT(ch->affected_by2, paf->bitvector);
					break;
				case WHERE_AFFECTS3:
					SET_BIT(ch->affected_by3, paf->bitvector);
					break;
				case WHERE_IMMUNE:
					SET_BIT(ch->imm_flags,paf->bitvector);
					break;
				case WHERE_RESIST:
					SET_BIT(ch->res_flags,paf->bitvector);
					break;
				case WHERE_VULN:
					SET_BIT(ch->vuln_flags,paf->bitvector);
					break;			
					
                }
                return;
            }
		}	
    }
}

/**************************************************************************/
// Give an affect to a char.
void affect_to_char( char_data *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

	// do debugging stuff
	if(paf->type>=MAX_SKILL)
	{
		bug("affect_to_char(): Invalid affect!!!\r\n");
		do_abort();
	}

    paf_new = new_affect();
    *paf_new		= *paf;
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;
	affect_modify( ch, paf_new, true );
    return;
}
/**************************************************************************/
// Give an affect to a char, when paf->where==WHERE_OBJECTSPELL
// Give it if they are high enough level for the affect or 
// if ignore level is set on the affect bitvector
void affect_copy_to_char( char_data *ch, AFFECT_DATA *paf, int objlevel )
{
    AFFECT_DATA *paf_new;

	// do debugging stuff
	if(paf->type>=MAX_SKILL)
	{
		bugf("affect_copy_to_char(): Invalid affect - paf->type=%d (higher than MAX_SKILL)!!!", paf->type);
		do_abort();
	}

	if(paf->where!=WHERE_OBJECTSPELL){
		bug("affect_copy_to_char(): paf->where==WHERE_OBJECTSPELL!!!\r\n");
		do_abort();
	}

	// level check 
	int castlevel=paf->level?paf->level:objlevel;
	if(!IS_SET(OBJSPELL_IGNORE_LEVEL, paf->bitvector) 
		&& castlevel>ch->level)
	{
		return;
	}

	// store the affect on the player
    paf_new = new_affect();
    *paf_new		= *paf;
	// if it is a level 0 objectspell, patch up the cast level 
	if(paf_new->where==WHERE_OBJECTSPELL && paf_new->level==0){
		paf_new->level=objlevel;
	}
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;
	affect_modify( ch, paf_new, true ); // handles casting if necessary
    return;
}
/**************************************************************************/
// loop thru all the affects on the object, finding which one it is
// - called from unequip_char()
// - paf is the affect on an object, not the copy
// - once the copy on the char has been found, it is removed, if there is
//   multiple affects of the same type, the active one will be removed.
void affect_removecopy_from_char( char_data *ch, AFFECT_DATA *paf, int objlevel )
{
	assert(paf->where==WHERE_OBJECTSPELL);

	// level check 
	if(!IS_SET(OBJSPELL_IGNORE_LEVEL, paf->bitvector) 
		&& paf->level>ch->level)
	{
		return;
	}

	AFFECT_DATA *caf=NULL;
	AFFECT_DATA *match=NULL;
	// find our affect, favouring an OBJSPELL_ACTIVE if possible
	for( caf= ch->affected; caf; caf=caf->next)
	{
		if((caf->type == paf->type) &&
			(caf->where==WHERE_OBJECTSPELL))
		{
			// if it is a level 0 objectspell, patch up the cast level 
			if(paf->where==WHERE_OBJECTSPELL && paf->level==0){
				if(caf->level!=objlevel){
					continue;
				}
			}else{
				if(caf->level!=paf->level){
					continue;
				}
			}

			match=caf;
			if(IS_SET(caf->bitvector,OBJSPELL_ACTIVE)){
				break; // found the active version
			}				
		}
	}

	if(match){
		affect_remove( ch, match);
	}else{
		bug("affect_removecopy_from_char(): couldn't find any affect copy.");
		return;
	}	
}
/**************************************************************************/
// give an affect to an object
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

	affects_from_template_to_obj(obj);
	
    paf_new = new_affect();
	
    *paf_new		= *paf;
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;
	
    // apply any affect vectors to the object's extra_flags 
    if(paf->bitvector){
		switch(paf->where)
		{
        case WHERE_OBJEXTRA:
			SET_BIT(obj->extra_flags,paf->bitvector);
			break;
        case WHERE_OBJEXTRA2:
			SET_BIT(obj->extra2_flags,paf->bitvector);
			break;
        case WHERE_WEAPON:
			if(obj->item_type == ITEM_WEAPON)
				SET_BIT(obj->value[4],paf->bitvector);
			break;

		default:
			break;
		}
	}
    return;
}


/**************************************************************************/
// Remove an affect from a char.
void affect_remove( char_data *ch, AFFECT_DATA *paf )
{
    int where;
    int vector;
	
    if( ch->affected == NULL )
    {
		bug("Affect_remove: no affects on ch!");
		return;
    }
	
    affect_modify( ch, paf, false );
    where = paf->where;
    vector = paf->bitvector;
	
    if( paf == ch->affected ){
		ch->affected	= paf->next;
    }else{
		AFFECT_DATA *prev;
		
		for( prev = ch->affected; prev != NULL; prev = prev->next )
		{
			if( prev->next == paf )
			{
				prev->next = paf->next;
				break;
			}
		}
		
		if( prev == NULL )
		{
			bug("Affect_remove: cannot find paf.");
			return;
		}
    }
	
    free_affect(paf);
	
	// fix up flying
	if(!IS_AFFECTED(ch, AFF_FLYING)){
		REMOVE_BIT(ch->dyn,DYN_NONMAGICAL_FLYING);
	}
    affect_check(ch,where,vector);
    return;
}

/**************************************************************************/
void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where, vector;
    if( obj->affected == NULL )
    {
        bug("Affect_remove_object: no affect.");
        return;
    }
	
    if(obj->carried_by != NULL && obj->wear_loc != -1)
		affect_modify( obj->carried_by, paf, false );
	
    where  = paf->where;
    vector = paf->bitvector;
	
    /* remove flags from the object if needed */
    if(paf->bitvector)
		switch( paf->where)
	{
		case WHERE_OBJEXTRA:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
		case WHERE_OBJEXTRA2:
            REMOVE_BIT(obj->extra2_flags,paf->bitvector);
            break;
        case WHERE_WEAPON:
            if(obj->item_type == ITEM_WEAPON)
                REMOVE_BIT(obj->value[4],paf->bitvector);
            break;
	}
	
    if( paf == obj->affected )
    {
        obj->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;
		
        for( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }
		
        if( prev == NULL )
        {
            bug("Affect_remove_object: cannot find paf.");
            return;
        }
    }
	
    free_affect(paf);
	
    if(obj->carried_by != NULL && obj->wear_loc != -1)
		affect_check(obj->carried_by,where,vector);
    return;
}



/**************************************************************************/
// Strip all affects of a given sn.
void affect_strip( char_data *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for( paf = ch->affected; paf != NULL; paf = paf_next )
    {
		paf_next = paf->next;
		if(paf->where==WHERE_OBJECTSPELL){
			continue;
		}
		if( paf->type == sn ){
		    affect_remove( ch, paf );
		}
    }

    return;
}

/**************************************************************************/
// Strip all affects from spells with the same spell function as that 
// belonging to the given sn if it is a spell, otherwise just strip the 
// affect
void affect_parentspellfunc_strip( char_data *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

	if(sn<FIRST_SPELL || sn>LAST_SPELL){
		affect_strip( ch, sn );
		return;
	}

	SPELL_FUN * spell_fun= skill_table[sn].spell_fun;

	if(!spell_fun || spell_fun==spell_null){
		affect_strip( ch, sn );
		return;
	}

    for( paf = ch->affected; paf != NULL; paf = paf_next )
    {
		paf_next = paf->next;
		if(paf->type>=FIRST_SPELL && paf->type<=LAST_SPELL){
			if(skill_table[paf->type].spell_fun == spell_fun){
				if(paf->where==WHERE_OBJECTSPELL)
				{
					REMOVE_BIT(paf->bitvector,OBJSPELL_ACTIVE);
				}else{
					affect_remove( ch, paf );
				}
			}
		}
	}
    return;
}

/**************************************************************************/
// Return the number of spells ch is affected by that are based on sn
int count_affected_by_base_spell( char_data *ch, int parentspell_sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
	int count = 0;

	if(parentspell_sn <FIRST_SPELL || parentspell_sn >LAST_SPELL){
		return 0;
	}

	SPELL_FUN * spell_fun= skill_table[parentspell_sn ].spell_fun;

	if(!spell_fun || spell_fun==spell_null){
		return 0;
	}

    for( paf = ch->affected; paf != NULL; paf = paf_next )
    {
		paf_next = paf->next;
		if(paf->type>=FIRST_SPELL && paf->type<=LAST_SPELL){
			if(!(paf->where==WHERE_OBJECTSPELL))
			{
				if(skill_table[paf->type].spell_fun == spell_fun){
					count++;
				}
			}
		}
	}

    return count;
}

/**************************************************************************/
// Return true if a char is affected by a spell.
bool is_affected( char_data *ch, int sn )
{
    AFFECT_DATA *paf;

	if( IS_NULLSTR(ch->name))
		return false;

    for( paf = ch->affected; paf != NULL; paf = paf->next ) {
		if(paf->where==WHERE_OBJECTSPELL){
			continue;
		}
		if( paf->type == sn )
		    return true;
    }
    return false;
}

/**************************************************************************/
// Kal - August 99
// notes: goes thru char, if they are flying, checking if they are flying
//     due to magical causes (any spell that gives the AFF_FLY affect)
//     If it is true it removes the DYN_NONMAGICAL_FLYING bit so we don't 
//     subtract movement points from them for flying.
void affect_fly_update(char_data *ch)
{
	if( IS_AFFECTED(ch, AFF_FLYING) ){
		SET_BIT(ch->dyn,DYN_NONMAGICAL_FLYING);
		// loop thru finding any fly affects
		AFFECT_DATA *paf;
		for( paf= ch->affected; paf; paf = paf->next)
		{
			if( paf->bitvector== AFF_FLYING){
				REMOVE_BIT(ch->dyn,DYN_NONMAGICAL_FLYING);
			}
		}
	}else{
		REMOVE_BIT(ch->dyn,DYN_NONMAGICAL_FLYING);
	}
}
	
/**************************************************************************/
// Add or enhance an affect.
void affect_join( char_data *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = false;
    for( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
		if(paf->where==WHERE_OBJECTSPELL){
			continue;
		}
		if( paf_old->type == paf->type )
		{
			paf->level = (paf->level += paf_old->level) / 2;
			paf->duration += paf_old->duration;
			paf->modifier += paf_old->modifier;
			affect_remove( ch, paf_old );
			break;
		}
    }

    affect_to_char( ch, paf );
    return;
}

/**************************************************************************/
// returns true if the ch is actually in the list of people in a room
// used by nuke_pets() and extract_char() to detect the difference between:
// * When someones ch->in_room and/or ch->pet_in_room has been set to point 
//   to a room, but they arent actually in the room yet as they haven't 
//   gone thru nanny_read_motd() to get to the char_to_room() call.
//   This time they wont be in the room.
// * When someone drops link, and reconnects to their linkdead char, 
//   then next time they login, they drop due to idling before a
//   char_to_room() call.  (This time they will be in the room, and
//   ch->desc->connected_state will show them as something other than
//   CON_PLAYING)
bool is_character_loaded_into_room(char_data *ch, ROOM_INDEX_DATA *room)
{
	if(!room){
		return false;
	}

	char_data *rch;
    
	for( rch = room->people; rch; rch = rch->next_in_room )
    {
		if(rch==ch){
			return true;
		}
    }
	return false;
}
/**************************************************************************/
void make_corefile();
/**************************************************************************/
// Move a char out of a room.
void char_from_room( char_data *ch )
{
    OBJ_DATA *obj;

    if( ch->in_room == NULL )
    {
        bug("Char_from_room: NULL.");
        return;
    }

    if( !IS_NPC(ch) )
    {
        --ch->in_room->area->nplayer;
    }

	ch->in_room->number_in_room--;

	if(ch->in_room->number_in_room<0){
		bugf("char_from_room(%s): ch->in_room->number_in_room<0!!!", ch->name);
		ch->in_room->number_in_room=0;
	}

	// They arent actually in the room, just have the ch->room pointer
	// set cause logging in then timed out
	if((ch->desc && ch->desc->connected_state!=CON_PLAYING) || !ch->desc){
		// if they arent in the game yet (haven't logged in, and this 
		// isn't a reconnect) or they are linkdead confirm
		// they are in the room
		if(!is_character_loaded_into_room(ch,ch->in_room)){
			ch->in_room		= NULL;
			ch->next_in_room= NULL;
			ch->on 			= NULL;  // sanity check! 
			return;
		};
	}

    if( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
		&&   obj->item_type == ITEM_LIGHT
		&&   obj->value[2] != 0
		&&   ch->in_room->light > 0 )
    {
        --ch->in_room->light;
    }

    if( ch == ch->in_room->people )
    {
        ch->in_room->people = ch->next_in_room;
    }
    else
    {
        char_data *prev;
    
        for( prev = ch->in_room->people; prev; prev = prev->next_in_room )
        {
            if( prev->next_in_room == ch )
            {
			    prev->next_in_room = ch->next_in_room;
				break;
            }
        }
    
        if( prev == NULL ){
            bugf( "Char_from_room: ch not found!!! ch->name=%s, "
				"ch->in_room->vnum=%d", ch->name, ch->in_room->vnum);
		}
    }

	// record if their previous room type was ooc or ic, 
	// when there is a with char_to_room, any queued events will be 
	// suspended/unsuspended
	if(IS_OOC(ch)){
		ch->previous_room_type=PREVIOUS_ROOM_TYPE_OOC;
	}else{
		ch->previous_room_type=PREVIOUS_ROOM_TYPE_IC;
	}

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on 	     = NULL;  // sanity check! 
    return;
}

/**************************************************************************/
// Move a char into a room.
void char_to_room( char_data *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if( pRoomIndex == NULL )
    {
		ROOM_INDEX_DATA *room;

		bugf( "Char_to_room: pRoomIndex==NULL!, get_room_index()'s last failed call was for vnum %d.", DEBUG_LAST_NON_EXISTING_REQUESTED_ROOM_VNUM);
		ch->printlnf("Bug in Char_to_room(): pRoomIndex==NULL!...`1`1"
			"Last unfound requested room vnum was %d.`1"
			"Please report this to the admin.",
			DEBUG_LAST_NON_EXISTING_REQUESTED_ROOM_VNUM);		
		if((room = get_room_index(ROOM_VNUM_OOC)) != NULL){
			ch->println("Taking you to the OOC rooms");
			char_to_room(ch,room);
		}
		return;
    }

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

	// record if their previous room type was ooc or ic, 
	// when there is a with char_to_room, any queued events will be 
	// suspended/unsuspended
	if(IS_SET(pRoomIndex->room_flags, ROOM_OOC)){
		if(ch->previous_room_type==PREVIOUS_ROOM_TYPE_IC){
			// moving from IC to an OOC room
			ch->moving_from_ic_to_ooc();
		}
	}else{
		// destination is an IC room
		if(ch->previous_room_type!=PREVIOUS_ROOM_TYPE_IC){
			// moving from OOC or just loaded to an IC room
			ch->moving_from_ooc_or_load_to_ic();
		}
	}

    if( !IS_NPC(ch) )
    {
        if(ch->in_room->area->empty)
        {
            ch->in_room->area->empty = false;
            ch->in_room->area->age = 0;
        }
        ch->in_room->area->nplayer++;
    }

	ch->in_room->number_in_room++;

    if( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;
	
    if(IS_AFFECTED(ch,AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        char_data *vch;
        
        for( af = ch->affected; af; af = af->next ){
            if(af->type == gsn_plague)
                break;
        }
        
        if(!af){
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
        }else if(af->level > 1){
			plague.where		= WHERE_AFFECTS;
			plague.type 		= gsn_plague;
			plague.level 		= af->level - 1; 
			plague.duration 	= number_range(1,2 * plague.level);
			plague.location		= APPLY_ST;
			plague.modifier 	= -5;
			plague.bitvector 	= AFF_PLAGUE;
			
			for( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
			{
				if(!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
					&&  !IS_IMMORTAL(vch) &&
					!IS_AFFECTED(vch,AFF_PLAGUE) && number_range(0, 63) == 0)
				{
					vch->println("You feel hot and feverish.");
					act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
					affect_join(vch,&plague);
				}
			}
		}
    }

	// MSP Check
	if( !IS_NPC( ch )) {
		if( CAN_HEAR_MSP( ch ) && !IS_NULLSTR(ch->in_room->msp_sound))
		{
			msp_to_room(MSPT_ROOM, ch->in_room->msp_sound, 
					0, 
					ch,
					true,
					false);
		}
	}

	// check if the room has an alarm on it
	if( !IS_NPC( ch ) && !IS_IMMORTAL(ch)) {
	    if( IS_SET( ch->in_room->affected_by, ROOMAFF_ALARM ))
		{
			if( !IS_NULLSTR( ch->in_room->alarm->name )
			&&	 ch->in_room->alarm != ch )
			{
				ch->in_room->alarm->println("Your alarm has been triggered!");
				REMOVE_BIT( ch->in_room->affected_by, ROOMAFF_ALARM );
				ch->in_room->alarm = NULL;
			}
		}
	}
	return;
}


/**************************************************************************/
// Give an obj to a char.
void obj_to_char( OBJ_DATA *obj, char_data *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
}


/**************************************************************************/
// Take an obj from its character.
void obj_from_char( OBJ_DATA *obj )
{
    char_data *ch;

    if( ( ch = obj->carried_by ) == NULL )
    {
        bug("Obj_from_char: null ch.");
        return;
    }

    if( obj->wear_loc != WEAR_NONE )
    {
        unequip_char( ch, obj );
    }

    if( ch->carrying == obj )
    {
        ch->carrying = obj->next_content;
    }
    else
    {
        OBJ_DATA *prev;
    
        for( prev = ch->carrying; prev != NULL; prev = prev->next_content )
        {
            if( prev->next_content == obj )
            {
                prev->next_content = obj->next_content;
                break;
            }
        }
    
        if( prev == NULL )
        {
            bug("Obj_from_char: obj not in list.");
        }
    }

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    return;
}


/**************************************************************************/
// Find the ac value of an obj, including position effect.
int apply_ac( OBJ_DATA *obj, int /*iWear*/, int type )
{
    if( obj->item_type != ITEM_ARMOR ){
		return 0;
	}

	return obj->value[type];
}

/**************************************************************************/
// Find a piece of eq on a character.
OBJ_DATA *get_eq_char( char_data *ch, int iWear )
{
    OBJ_DATA *obj;

    if(ch == NULL)
	return NULL;

    for( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}


/**************************************************************************/
// Equip a char with an obj.
void equip_char( char_data *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    int i, ac_amount;

    if( get_eq_char( ch, iWear ) != NULL )
    {        
        bug("Equip_char: already equipped");
		bugf("(on mob %d, room=%d, object=%d, worn: %s).",
            (IS_NPC(ch)?(ch->pIndexData->vnum) :0),
			ch->in_room_vnum(), obj->pIndexData->vnum, 
			flag_string(wear_location_strings_types, iWear));
		bugf("(%s, %s).", ch->short_descr, obj->short_descr);
        return;
    }

	if( !IS_IMMORTAL( ch ) && !(IS_NPC(ch) && 
			(IS_SET(ch->act,ACT_NOALIGN) || IS_SET(ch->act2,ACT2_NO_TENDENCY))  
							   )
	  ){
	    if( ( IS_OBJ_STAT(obj, OBJEXTRA_ANTI_EVIL)    && IS_EVIL(ch)    )	
	    ||   ( IS_OBJ_STAT(obj, OBJEXTRA_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, OBJEXTRA_ANTI_NEUTRAL) && IS_NEUTRAL(ch) )
        ||   ( IS_OBJ2_STAT(obj, OBJEXTRA2_ANTI_CHAOS)    && IS_TEND_CHAOTIC(ch) )
        ||   ( IS_OBJ2_STAT(obj, OBJEXTRA2_ANTI_LAW)    && IS_TEND_LAWFUL(ch) )
        ||   ( IS_OBJ2_STAT(obj, OBJEXTRA2_ANTI_BALANCE) && IS_TEND_NEUTRAL(ch) ) )
		{
			act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
			act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
			obj_from_char( obj );
			obj_to_room( obj, ch->in_room );
			return;
		}
	}

    // apply the armour class
	for(i = 0; i < 4; i++)
	{
		ac_amount = apply_ac( obj, iWear, i );
		if(ch->level< obj->level)
			ac_amount = ac_amount * ch->level/ obj->level;           
		ch->armor[i] -= ac_amount;
	}

	// record on the object where it is being worn
    obj->wear_loc	 = iWear;

	// Apply the affects of the object
	// - from olc template if object doesn't have custom enchants
	for(paf=OBJECT_AFFECTS(obj); paf; paf = paf->next )
	{
		if( paf->where==WHERE_OBJECTSPELL)	{
			affect_copy_to_char( ch, paf, obj->level);
		}else{
			if(paf->level<=ch->level){
				affect_modify( ch, paf, true );
			}
		}
	}

	// check for restrictgroup based restrictions on object, 
	// if they exist apply them to the wearer	
	if(obj->pIndexData && ch->pcdata
		&& HAS_CONFIG(ch,CONFIG_OBJRESTRICT) 
		&& ((ch->pcdata->objrestrict& obj->pIndexData->objrestrict)>0))
	{
		OBJRESTRICT_LIST_DATA *pr;
		AFFECT_DATA aff;
		int top_prority=-1;
		for( pr = obj->pIndexData->restrict; pr; pr = pr->next )
		{
			if(IS_SET(ch->pcdata->objrestrict,(1<<pr->classgroup->bitindex))){
				top_prority=UMAX(pr->priority, top_prority);
			}
			
		}

		// IC object restriction system - Kal
		for( pr = obj->pIndexData->restrict; pr; pr = pr->next )
		{
			if(top_prority!=pr->priority 
				||	!IS_SET(ch->pcdata->objrestrict,(1<<pr->classgroup->bitindex)))
			{
				continue;
			}
			aff.where		= WHERE_RESTRICT;
			aff.type		= -1;
			aff.level		= 0;
			aff.duration	= -1;
			aff.location	= pr->affectprofile->wear_location;
			aff.modifier	= pr->affectprofile->wear_amount;
			// can't have positive modifiers - invert them - done now in object loading
//			if(aff.modifier>0){
//				aff.modifier=0-aff.modifier;
//			}
			aff.bitvector	= 0;
			// do the act text
			act(pr->affectprofile->wear_message,ch,obj,NULL,TO_CHAR);	
			// add the affect
    		affect_modify( ch, &aff, true );

			if(top_prority==-1) // -1 means only the first affectprofile is applied
				break;		
		}
	}

	// if the object is a light, change the room lighting
    if( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room)
		++ch->in_room->light;

    return;
}


/**************************************************************************/
// Unequip a char with an obj.
void unequip_char( char_data *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    int i, ac_amount;
	
    if( obj->wear_loc == WEAR_NONE )
    {
		bug("Unequip_char: already unequipped.");
		return;
    }
	
    // remove the affects on the armour class
	for(i = 0; i < 4; i++)
	{
		ac_amount = apply_ac( obj, obj->wear_loc, i );
		if(ch->level< obj->level){
			ac_amount = ac_amount * ch->level/ obj->level;
		}
		ch->armor[i] += ac_amount;
	}
	
	// record on the object that it is no longer worn
    obj->wear_loc = WEAR_NONE;
	
	// Remove the affects of the object
	// - from olc template if object doesn't have custom enchants
	for(paf=OBJECT_AFFECTS(obj); paf; paf = paf->next )
	{
		if( paf->where==WHERE_OBJECTSPELL)
		{
	   		affect_removecopy_from_char( ch, paf, obj->level );			
		}
		else
		{
			if(paf->level<=ch->level){
				affect_modify( ch, paf, false );
				affect_check(ch,paf->where,paf->bitvector);
			}
		}
	}

	// check for restrictgroup based restrictions on object, 
	// if they exist remove them from the wearer	
	if(obj->pIndexData && ch->pcdata
		&& HAS_CONFIG(ch,CONFIG_OBJRESTRICT) 
		&& ((ch->pcdata->objrestrict& obj->pIndexData->objrestrict)>0))
	{
		OBJRESTRICT_LIST_DATA *pr;
		AFFECT_DATA aff;
//		ch->println("equip_char(): group restriction on object match, searching...");
		int top_prority=-1;
		for( pr = obj->pIndexData->restrict; pr; pr = pr->next )
		{
			if(IS_SET(ch->pcdata->objrestrict,(1<<pr->classgroup->bitindex))){
				top_prority=UMAX(pr->priority, top_prority);
			}
			
		}

		for( pr = obj->pIndexData->restrict; pr; pr = pr->next )
		{
			if(top_prority!=pr->priority 
				||	!IS_SET(ch->pcdata->objrestrict,(1<<pr->classgroup->bitindex)))
			{
				continue;
			}
//			ch->println("unequip_char(): Found... removing.");
			aff.where		= WHERE_RESTRICT;
			aff.type		= -1;
			aff.level		= 0;
			aff.duration	= -1;
			aff.location	= pr->affectprofile->wear_location;
			aff.modifier	= pr->affectprofile->wear_amount;
			aff.bitvector	= 0;
			// do the act text
			act(pr->affectprofile->remove_message,ch,obj,NULL,TO_CHAR);	
			// remove the affect
    		affect_modify( ch, &aff, false);

			if(top_prority==-1) // -1 means only the first affectprofile is applied
				break;		
		}
	}

	// if the object is a light, change the room lighting
	if( obj->item_type == ITEM_LIGHT && obj->value[2] != 0
		&& ch->in_room != NULL && ch->in_room->light > 0 ){
		--ch->in_room->light;
	}
	return;
}

/**************************************************************************/
// Count occurrences of an obj in a list.
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for( obj = list; obj != NULL; obj = obj->next_content )
    {
		if( obj->pIndexData == pObjIndex ){
			nMatch++;
		}
    }

    return nMatch;
}


/**************************************************************************/
// Move an obj out of a room.
void obj_from_room( OBJ_DATA *obj )
{
	ROOM_INDEX_DATA *in_room;
	char_data *ch;

	if( ( in_room = obj->in_room ) == NULL ){
		bug("obj_from_room: NULL.");
		return;
	}

	for(ch = in_room->people; ch; ch = ch->next_in_room){
		if(ch->on == obj){
			ch->on = NULL;
		}
	}

	if( obj == in_room->contents ){
		in_room->contents = obj->next_content;
	}else{
		OBJ_DATA *prev;

		for( prev = in_room->contents; prev; prev = prev->next_content )
		{
			if( prev->next_content == obj )
			{
				prev->next_content = obj->next_content;
				break;
			}
		}

		if( prev == NULL )
		{
			bug("Obj_from_room: obj not found.");
			return;
		}
	}

	obj->in_room      = NULL;
	obj->next_content = NULL;
	return;
}


/**************************************************************************/
// Move an obj into a room.
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
	if(!pRoomIndex){
		bug("obj_to_room(): NULL input pRoomIndex");
	}else{
	    obj->next_content		= pRoomIndex->contents;
		pRoomIndex->contents	= obj;
	}
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    return;
}



/**************************************************************************/
// Move an object into an object.
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    if(obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
        obj->cost = 0; 

    for( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}

/**************************************************************************/
// Move an object out of an object.
void obj_from_obj( OBJ_DATA *obj )
{
	OBJ_DATA *obj_from;

	if( ( obj_from = obj->in_obj ) == NULL )
	{
		bug("Obj_from_obj: null obj_from.");
		return;
	}

	if( obj == obj_from->contains )
	{
		obj_from->contains = obj->next_content;
	}
    else
    {
		OBJ_DATA *prev;

		for( prev = obj_from->contains; prev; prev = prev->next_content )
		{
			if( prev->next_content == obj )
			{
				prev->next_content = obj->next_content;
				break;
			}
		}

		if( prev == NULL )
		{
			bug("Obj_from_obj: obj not found.");
			return;
		}
	}

	obj->next_content = NULL;
	obj->in_obj       = NULL;

	for( ; obj_from != NULL; obj_from = obj_from->in_obj )
	{
		if( obj_from->carried_by != NULL )
		{
			obj_from->carried_by->carry_number -= get_obj_number( obj );
			obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
				* WEIGHT_MULT(obj_from) / 100;
		}
	}

	return;
}


/**************************************************************************/
// Extract an obj from the world.
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

	if( obj->in_room != NULL ){
        obj_from_room( obj );
	}else if ( obj->carried_by != NULL ){
        obj_from_char( obj );
	}else if ( obj->in_obj != NULL ){
        obj_from_obj( obj );
	}

    for( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
        obj_next = obj_content->next_content;
        extract_obj( obj_content );
    }

    if( object_list == obj )
    {
        object_list = obj->next;
    }
    else
    {
        OBJ_DATA *prev;
    
        for( prev = object_list; prev != NULL; prev = prev->next )
        {
            if( prev->next == obj )
            {
                prev->next = obj->next;
                break;
            }
        }
    
        if( prev == NULL ){
            if(obj->pIndexData){
                bugf( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
            }else{
                bug("Extract_obj: obj ??? not found. (no pIndexData on it!)");
			}
            return;
        }
    }

    if(obj->pIndexData)
        obj->pIndexData->count--;

    free_obj(obj);
    return;
}

/**************************************************************************/
void duel_logout(char_data *ch);
/**************************************************************************/
void extract_char_from_char_list(char_data *ch)
{
    if( ch == char_list ){
		char_list = ch->next;
    }else{
		char_data *prev;

		for( prev = char_list; prev != NULL; prev = prev->next )
		{
			if( prev->next == ch )
			{
			prev->next = ch->next;
			break;
			}
		}

		if( prev == NULL )
		{
			bug("extract_char_from_char_list(): char not found.");
			return;
		}
    }

}
/**************************************************************************/
// Extract a char from the world - does NOT handle any pfile saving
// if fPull is true, it is a mob, so remove them from the game and free
// the allocated memory.
void extract_char( char_data *ch, bool fPull )
{
    char_data *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *recount_in_room;
    vn_int recall_vnum;

    if( ch->in_room == NULL ){
        bug("Extract_char: ch->in_room == NULL.");
        return;
    }

	// ploaded players can't be extracted using extract_char()
	if(ch->pload && ch->pload->dont_save){
		if(ch->pload->loaded_by){
			ch->pload->loaded_by->printlnf( "extract_char(): character %s can't be extracted cause it was ploaded.", ch->name);
		}			
		logf( "extract_char(): character %s can't be extracted cause it was ploaded.", ch->name);
		return;
	}

    nuke_pets(ch);
    ch->pet = NULL; // just in case 

    if( fPull ){
		die_follower( ch );
	}
    
	stop_fighting( ch, true );

    for( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;
		if(obj->item_type == ITEM_TOKEN && 
			!IS_SET ( obj->value[0], TOKEN_DROPDEATH )) {
				continue;
		}
        extract_obj( obj );
    }
    
	recount_in_room=ch->in_room;
    char_from_room( ch );

	ch->mudqueue_dequeue_all(EVENTGROUP_PURGE_EVERY_EVENT_INCLUDING_SYSTEM_EVENTS);

    // Death room is set in the clan table now
    if( !fPull ) // was a player that died - therefore not removed from the game
    {
		if(GAMESETTING5(GAMESET5_DEDICATED_PKILL_STYLE_MUD)){
			recall_vnum =ROOM_PKILLPORT_DEATH_ROOM; // pkill port hack
		}else{
			recall_vnum = race_table[ch->race]->death_room;
		}
    
		if( ( location = get_room_index( recall_vnum ) ) == NULL )
		{
			ch->println("You feel totally disoriented.");

			if( ( location = get_room_index( ROOM_VNUM_OOC ) ) == NULL)
			{
				ch->printf("BUG: Cant find the main ooc room (vnum = %d)\r\n"
					"Please report this to an admin.\r\n", ROOM_VNUM_OOC);
				return;
			}
			else
			{
				if(IS_SET(location->room_flags, ROOM_OOC))
				{
					ch->println("Taking you to the main OOC room since your normal death room doesn't exist.");				
				}
				else
				{
					ch->printf("BUG: Taking you to the main ooc room (vnum = %d)\r\n"
					"This room SHOULD be an OOC room - please report this bug to an admin.\r\n", ROOM_VNUM_OOC);
				}
			}	
		}

        char_to_room(ch,location);
        return;
    }

	// extracting characters cause they are quitting
    if( IS_NPC(ch) ){
        ch->pIndexData->count--;
    }else{
		// record the characters logout time 
        laston_logout(ch); 

		// 'logout' any duels
		duel_logout(ch);
	}

    // in switched mobs
	if( ch->desc != NULL && ch->desc->original != NULL )
    {
        do_return( ch, "" );
        ch->desc = NULL;
    }

    // break off all those that reply, retell or mprog target memories
    for( wch = char_list; wch; wch = wch->next )
    {
        if( wch->reply == ch )
            wch->reply = NULL;
        if( wch->retell == ch )
            wch->retell = NULL;
        if( wch->mprog_target == ch )
            wch->mprog_target = NULL;
    }


	// take the mob/player out of the character list
	// unless they are a ploaded player or pet of a ploaded player
	if(!ch->pload){
		extract_char_from_char_list(ch);
	}

	if(!IS_NPC(ch) )
	{
		if( ch == player_list ) 
		{
			player_list = ch->next_player;
		}
        else 
        {  
            char_data *prev;
            for(prev = player_list; prev != NULL; prev = prev->next_player )
            {
  				if(prev->next_player == ch)
				{ 
					prev->next_player = ch->next_player;
					break;
				}
            }
        }


		// if they ploaded someone, extract the ploaded character
		// should really automatically unload the loaded person
		if(ch->ploaded){
			assert(ch->ploaded->pload && ch->ploaded->pload->loaded_by==ch); // if we link to them, they should link to us
			pload_extract(ch, ch->ploaded);
		}
	}

    if( ch->desc != NULL ){
		ch->desc->character = NULL;
	}

    free_char( ch );

	// recount the number of people in the room they just logged out of
	if(recount_in_room){
		int rcount=0;
		ch=recount_in_room->people;
		while(ch){
			rcount++;
			ch=ch->next_in_room;
		}
		recount_in_room->number_in_room=rcount;
	}
    return;
}


/**************************************************************************/
/*
 * Find a char in the room.
 * major bits rewritten by Kalahn - March 98
 */
char_data *get_char_room( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    if( !str_cmp( arg, "self" ) )
	    return ch;

	// support UID
	int uid=get_uid(arg);
	if(uid){
		for( rch = ch->in_room->people; rch; rch = rch->next_in_room )
		{
			if(uid==rch->uid){
				if(can_see( ch, rch )){
					return rch;
				}else{
					return NULL;
				}
			}
		}
		return NULL;
	}


    // first do a name match and exact short descript match on mobs
    count  = 0;
	for( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if( !can_see( ch, rch ))
			continue;

        if(!(	is_name( arg, rch->name ) 
				|| (IS_NPC(rch) && rch!=ch && is_name( arg, strip_colour(rch->short_descr)) )
			 ))
            continue;

        if( rch==ch && HAS_AUTOSELF(ch))
            continue;
        
        if( ++count == number )
            return rch;
    }

    // now do a name and exact short descript match on everyone
	count = 0;
	for( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if( !can_see( ch, rch ))
			continue;

        if(!(rch!=ch && is_name( arg, strip_colour(rch->short_descr))) )
            continue;

        if( rch==ch && HAS_AUTOSELF(ch))
            continue;
        
        if( ++count == number )
            return rch;
    }

    return NULL;
}

/**************************************************************************/
// Find a pet in the room.
char_data *get_pet_room( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    if( !str_cmp( arg, "self" ) )
	    return ch;

    for( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if( !can_see( ch, rch ))
			continue;

		// pets only
		if( !IS_NPC(rch) || !IS_SET(rch->act, ACT_PET) )
			continue;

        if(!(	is_name( arg, rch->name ) 
				|| (rch!=ch && is_name( arg, strip_colour(rch->short_descr)) )
			 ))
            continue;
/*
        if( !can_see( ch, rch ) ||
            (!is_name( arg, rch->name ) && (!is_name( arg, strip_colour(rch->short_descr)) ) ) )
            continue;
*/
		if(!IS_NPC(rch))
			continue;

        if( rch==ch && HAS_AUTOSELF(ch))
            continue;
        
        if( ++count == number )
            return rch;
    }

    return NULL;
}


/**************************************************************************/
// Find a char in the world.
char_data *get_char_world( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *wch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;

    if( !str_cmp( arg, "self" ) )
	    return ch;

	// support UID
	int uid=get_uid(arg);
	if(uid){
	    for( wch = char_list; wch ; wch = wch->next ){
			if(uid==wch->uid){
				if(wch->in_room && can_see( ch, wch )){
					return wch;
				}else{
					return NULL;
				}
			}
		}
		return NULL;
	}

    // first check for an exact name match through
    // the whole world -- for PCs only 
    for( wch = char_list; wch ; wch = wch->next ){
        if( wch->in_room == NULL || !can_see( ch, wch ) 
        ||  str_cmp(arg, wch->name) || IS_NPC(wch))
            continue;
        
        if( wch==ch && HAS_AUTOSELF(ch)) 
            continue;
         
        if( ++count == number )
            return wch;
    }


    if( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    count  = 0;
    for( wch = char_list; wch != NULL ; wch = wch->next )
    {
		if(!wch->in_room)
			continue;

		if(!can_see(ch,wch))
			continue;

        if(!(	is_name( arg, wch->name ) 
				|| (wch!=ch && is_name( arg, strip_colour(wch->short_descr)) )
			 ))
            continue;

		if( wch==ch && HAS_AUTOSELF(ch)) 
            continue;
        
        if( ++count == number )
            return wch;
    }

    return NULL;
}

/**************************************************************************/
// Find a char in the ic world. (ie non ooc, or olconly areas)
char_data *get_char_icworld( char_data *ch, char *argument )
{
    char arg[MIL];
    char_data *wch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;

    if( !str_cmp( arg, "self" ) )
	    return ch;

	// support UID
	int uid=get_uid(arg);
	if(uid){
	    for( wch = char_list; wch ; wch = wch->next ){
			if(uid==wch->uid){
				if(wch->in_room && can_see( ch, wch ) && IS_IC(wch)){
					return wch;
				}else{
					return NULL;
				}
			}
		}
		return NULL;
	}

        /* first check for an exact name match through
     * the whole world -- for PCs only */
    for( wch = char_list; wch != NULL ; wch = wch->next )
    {
        if( wch->in_room == NULL || !can_see( ch, wch ) 
        ||  str_cmp(arg, wch->name) || IS_NPC(wch))
            continue;

		if(!IS_IC(wch))
            continue;
        
        if( wch==ch && HAS_AUTOSELF(ch)) 
            continue;
        
        if( ++count == number )
            return wch;
    }


    if( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    count  = 0;
    for( wch = char_list; wch != NULL ; wch = wch->next )
    {
		if(!wch->in_room)
			continue;

		if(!can_see(ch,wch))
			continue;

        if(!(	is_name( arg, wch->name ) 
				|| (wch!=ch && is_name( arg, strip_colour(wch->short_descr)) )
			 ))
            continue;

        if( wch==ch && HAS_AUTOSELF(ch)) 
            continue;

		if(!IS_IC(wch))
            continue;
        
        if( ++count == number )
            return wch;
    }

    return NULL;
}


/**************************************************************************/
/*
 * Find a player in the world that is visible on the wholist to the looker
 *  - called from do_tell, do_requestooc, do_dlook etc
 *  - will never return a mob
 */
char_data *get_whovis_player_world( char_data *looker, char *argument )
{
    char arg[MIL];
    char_data *wch;
    int number;
    int count;

    number = number_argument( argument, arg );

	if( !str_cmp( arg, "self" ) ){
	    return looker;
	}

	// support UID
	int uid=get_uid(arg);
	if(uid){
		for( wch = player_list; wch; wch = wch->next_player )
		{
			if(uid==wch->uid){
				if(can_see_who(looker,wch)){
					return wch;
				}else{
					return NULL;
				}
			}
		}
		return NULL;
	}

    // first check uid combined with for an exact name match whole world - player list
    count  = 0;
    for( wch = player_list; wch; wch = wch->next_player )
    {
		// must be able to see the player on the who list
		if(!can_see_who(looker,wch)){
			continue;
		}

		if(uid){ // searching by uid 
			if(uid==wch->uid){
				return wch;
			}
		}else{
			// must have an exactly matching name
			if(str_cmp(arg, wch->name))
				continue;


			if(wch==looker && HAS_AUTOSELF(looker))
				continue;

			if( ++count == number )
				return wch;
		}
    }

	// next check for substring match on
	// name in same room
    count  = 0;
    for( wch = player_list; wch; wch = wch->next_player )
    {
		// must be in the same room
        if( wch->in_room != looker->in_room)
            continue;

		// must be able to see the player on the who list
		if(!can_see_who(looker,wch))
			continue;

		// must have a substring matching short or name 
		if(!is_name( arg, wch->name))
			continue;

		//autoself
        if(wch==looker && HAS_AUTOSELF(looker))
            continue;

        if( ++count == number )
            return wch;
    }


	// next check for substring match on 
	// player name anywhere in the game
    count  = 0;
    for( wch = player_list; wch; wch = wch->next_player )
    {
		// must be able to see the player on the who list
		if(!can_see_who(looker,wch))
			continue;

		// must have a substring matching short or name 
		if(!is_name( arg, wch->name))
			continue;

		//autoself
        if(wch==looker && HAS_AUTOSELF(looker))
            continue;

        if( ++count == number )
            return wch;
    }


    // next check for substring match on short 
	// description or name in same room
    count  = 0;
    for( wch = player_list; wch; wch = wch->next_player )
    {
		// must be in the same room
        if( wch->in_room != looker->in_room)
            continue;

		// must be able to see the player on the who list
		if(!can_see_who(looker,wch))
			continue;

		// must have a substring matching short or name 
		if(!is_name( arg, strip_colour(wch->short_descr))
			&& !is_name( arg, wch->name))
			continue;

		//autoself
        if(wch==looker && HAS_AUTOSELF(looker))
            continue;

        if( ++count == number )
            return wch;
    }


	// lastly check for substring match on short 
	// description or name anywhere in the game
    count  = 0;
    for( wch = player_list; wch; wch = wch->next_player )
    {
		// must be able to see the player on the who list
		if(!can_see_who(looker,wch))
			continue;

		// must have a substring matching short or name 
		if(!is_name( arg, strip_colour(wch->short_descr))
			&& !is_name( arg, wch->name))
			continue;

		//autoself
        if(wch==looker && HAS_AUTOSELF(looker))
            continue;

        if( ++count == number )
            return wch;
    }

	// very very lastly check for a ploaded player in the same room as you
	wch=pload_find_player_by_name(arg);
	if(wch){
		return wch;
	}

    return NULL;
}


/**************************************************************************/
/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for( obj = object_list; obj; obj = obj->next )
    {
		if( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}
/**************************************************************************/
// will return a particular object in a chain, based on its pObjIndex
OBJ_DATA *get_obj_in_chain_recursive_by_index(OBJ_DATA *startobj, OBJ_INDEX_DATA *pObjIndex)
{
	OBJ_DATA *result;
	for( ; startobj; startobj= startobj->next_content )
    {
		if(startobj->pIndexData==pObjIndex){
			return startobj;
		}

		if(startobj->contains){
			// search deeper if necessary
			result=get_obj_in_chain_recursive_by_index(startobj->contains, pObjIndex);
			if(result){
				return result;
			}
		}
	}
	return NULL;

}

/**************************************************************************/
/*
 * Find some object with a given index data in the room.
 * Used by area-reset 'P' command.
 * will never return an object carried by a player
 */
OBJ_DATA *get_obj_of_type_in_room( OBJ_INDEX_DATA *pObjIndex, ROOM_INDEX_DATA *room )
{
    OBJ_DATA *obj;

	// first search all objects on the floor
	obj=get_obj_in_chain_recursive_by_index(room->contents, pObjIndex);
	if(obj){
		return obj;
	}

	// now search thru all objects carried by mobs in the room
	for(char_data *mob=room->people; mob; mob=mob->next_in_room){
		obj=get_obj_in_chain_recursive_by_index(mob->carrying, pObjIndex);
		if(obj){
			return obj;
		}
	}
	return NULL;
}

/**************************************************************************/
/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( char_data *ch, char *argument, OBJ_DATA *list )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

	// support UID
	int uid=get_uid(argument);
	if(uid){
		for( obj = list; obj; obj = obj->next_content ){
			if(uid==obj->uid
				&& !IS_SET( obj->extra_flags, OBJEXTRA_NO_GET_ALL )
				&& can_see_obj( ch, obj ) )
			{
				return obj;
			}
		}
		return NULL;
	}

    number = number_argument( argument, arg );
    count  = 0;
    for( obj = list; obj; obj = obj->next_content )
    {
		if( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
		{	
			if( ++count == number ){
				if(	!IS_SET( obj->extra_flags, OBJEXTRA_NO_GET_ALL ) 
					|| is_exact_name( arg, obj->name ) )
				{
					return obj;
				}
			}
		}
    }

    return NULL;
}

/**************************************************************************/
// Find an obj in player's inventory, that looker can see
OBJ_DATA *get_obj_carry_for_looker( char_data *ch, char *argument, char_data *looker)
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

	// support UID
	int uid=get_uid(argument);
	if(uid){
		for( obj = ch->carrying; obj; obj = obj->next_content){
			if(uid==obj->uid 
				&& obj->wear_loc == WEAR_NONE
				&& can_see_obj( looker, obj ) )
			{
				return obj;
			}
		}
		return NULL;
	}


    number = number_argument( argument, arg );
    count  = 0;
    for( obj = ch->carrying; obj; obj = obj->next_content )
    {
		if( obj->wear_loc == WEAR_NONE
		&&   (can_see_obj( looker, obj ) ) 
		&&   is_name( arg, obj->name ) )
		{
			if( ++count == number )
			return obj;
		}
    }

    return NULL;
}


/**************************************************************************/
// Find an obj in player's inventory or an item they are holding
OBJ_DATA *get_obj_carry( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

	// support UID
	int uid=get_uid(argument);
	if(uid){
		for( obj = ch->carrying; obj; obj = obj->next_content){
			if(uid==obj->uid 
				&& (obj->wear_loc == WEAR_NONE
					|| obj->wear_loc == WEAR_HOLD)
				&& can_see_obj( ch, obj ) )
			{
				return obj;
			}
		}
		return NULL;
	}

    number = number_argument( argument, arg );
    count  = 0;
    for( obj = ch->carrying; obj; obj = obj->next_content )
    {
		if( (obj->wear_loc == WEAR_NONE
			|| obj->wear_loc == WEAR_HOLD)
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
//  Find a token in player's inventory
//  Regular get_obj_carry doesn't work with 
//	tremove since victim can't see tokens
// supports vnum and description searching
OBJ_DATA *get_obj_token( char_data *ch, char *argument )
{
	char arg[MIL];
	OBJ_DATA *obj;
	int number;
	int count;
	
	int tok_vnum=0;
	bool use_vnum=false;

	number = number_argument( argument, arg );
	count  = 0;

	if(is_number(arg)){
		tok_vnum=atoi(arg);
		use_vnum=true;
	}

	for( obj = ch->carrying; obj; obj = obj->next_content ) {
		if( obj->item_type == ITEM_TOKEN)
		{
			if((use_vnum && obj->pIndexData && obj->pIndexData->vnum==tok_vnum)
				|| (!use_vnum && is_name( arg, strip_colour(obj->name))) ) 
			{
				if( ++count == number ){
						return obj;
				}
			}
		}
    }
	return NULL;
}


/**************************************************************************/
// Find an obj in player's equipment.
OBJ_DATA *get_obj_wear( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

	// support UID
	int uid=get_uid(argument);
	if(uid){
		for( obj = ch->carrying; obj; obj = obj->next_content){
			if(uid==obj->uid 
				&& obj->wear_loc != WEAR_NONE
				&& can_see_obj( ch, obj ) )
			{
				return obj;
			}
		}
		return NULL;
	}

    number = number_argument( argument, arg );
    count  = 0;
    for( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/**************************************************************************/
// Find an obj in the room or in inventory.
// supports uid
OBJ_DATA *get_obj_here( char_data *ch, char *argument )
{
	OBJ_DATA *obj;
	bool free_required=false;

	if( is_number( argument )){ // vnum support
		OBJ_INDEX_DATA *pObj;

		if(( pObj = get_obj_index( atoi( argument ))) == NULL ){
			return NULL;
		}
		argument = str_dup( pObj->name );
		free_required=true;
	}

	// locker support
	if(!str_prefix("locker",argument)){		
		int locker_num=atoi(argument+6);
		if(free_required){
			free_string(argument);
		}
		return lockers->find_locker_object(ch, true, locker_num);
	}

	// get_obj_list() supports uid
    obj = get_obj_list( ch, argument, ch->in_room->contents );
	if(obj){
		if(free_required){
			free_string(argument);
		}
		return obj;
	}

	// get_obj_list() supports uid
	obj = get_obj_carry( ch, argument );
	if(obj){
		if(free_required){
			free_string(argument);
		}
		return obj;
	}

	// get_obj_list() supports uid
	obj = get_obj_wear( ch, argument );
	if(obj){
		if(free_required){
			free_string(argument);
		}
		return obj;
	}

	// locker abbreviated support for your own locker
	if(!str_prefix("loc",argument)){				
		if(free_required){
			free_string(argument);
		}
		return lockers->find_locker_object(ch, true, 0);
	}

	if(free_required){
		free_string(argument);
	}
	return NULL;
}

/**************************************************************************/
// Find an obj in the world.
OBJ_DATA *get_obj_world( char_data *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

	// get_obj_here() supports uid
    if( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

	// support UID
	int uid=get_uid(argument);
	if(uid){
		for( obj = object_list; obj; obj = obj->next ){
			if(uid==obj->uid 
				&& can_see_obj( ch, obj ) )
			{
				return obj;
			}
		}
		return NULL;
	}

    number = number_argument( argument, arg );
    count  = 0;
    for( obj = object_list; obj; obj = obj->next )
    {
		if( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
		{
			if( ++count == number )
			return obj;
		}
    }

    return NULL;
}

/**************************************************************************/
// deduct cost from a character 
void deduct_cost(char_data *ch, int cost)
{
    int silver = 0, gold = 0;
	
    silver = UMIN(ch->silver,cost); 
	
    if(silver<cost){
		gold = ((cost - silver + 99) / 100);
		silver = cost - 100 * gold;
    }
	
    ch->gold -= gold;
    ch->silver -= silver;
	
    if(ch->gold < 0){
		bugf("deduct costs: gold %d < 0", (int)ch->gold);
		ch->gold = 0;
    }
    if(ch->silver<0){
		bugf("deduct costs: silver %d < 0", (int)ch->silver);
		ch->silver = 0;
    }
}   

/**************************************************************************/
// Create a 'money' obj.
OBJ_DATA *create_money( int gold, int silver )
{
    char buf[MSL];
    OBJ_DATA *obj;
	
    if( gold < 0 || silver < 0 || (gold == 0 && silver == 0) )
    {
		bug( "Create_money: zero or negative money.");
		gold = UMAX(1,gold);
		silver = UMAX(1,silver);
    }
	
    if(gold == 0 && silver == 1)
    {
		obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ));
    }
    else if (gold == 1 && silver == 0)
    {
		obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE));
    }
    else if (silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ));
        sprintf( buf, obj->short_descr, gold );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[1]           = gold;
        obj->cost               = gold;
		obj->weight		= gold/5;
    }
    else if (gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ));
        sprintf( buf, obj->short_descr, silver );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[0]           = silver;
        obj->cost               = silver;
		obj->weight		= silver/20;
    }
	
    else
    {
		obj = create_object( get_obj_index( OBJ_VNUM_COINS ));
		sprintf( buf, obj->short_descr, silver, gold );
		free_string( obj->short_descr );
		obj->short_descr	= str_dup( buf );
		obj->value[0]		= silver;
		obj->value[1]		= gold;
		obj->cost		= 100 * gold + silver;
		obj->weight		= gold / 5 + silver / 20;
    }
	
    return obj;
}



/**************************************************************************/
/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;
 
	if(obj->item_type == ITEM_HERB){
		return 1;
	}
    if(obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
    ||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY
	||  obj->item_type == ITEM_TOKEN ){
        number = 0;
	}else{
        number = 10;
	}
 
    for( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number;
}


/**************************************************************************/
/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;
    for( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

/**************************************************************************/
int get_true_weight(OBJ_DATA *obj)
{
    int weight;
 
    weight = obj->weight;
    for( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );
 
    return weight;
}

/**************************************************************************/
// True if room is dark.
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
	OBJ_DATA	*obj;
	
	if(!pRoomIndex) 
		return false;

	if( IS_SET(pRoomIndex->affected_by, ROOMAFF_LEONIDS ))
		return false;

	if( IS_SET(pRoomIndex->affected_by, ROOMAFF_UTTERDARK ))
		return true;
	
    if( pRoomIndex->light > 0 )
		return false;
	
	for( obj = pRoomIndex->contents; obj; obj = obj->next_content ) {
		if( obj->pIndexData->vnum == OBJ_VNUM_FIRE )
			return false;
    }
	
    if( IS_SET(pRoomIndex->room_flags, ROOM_LIGHT ))
		return false;
	
    if( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
		return true;

    if( pRoomIndex->sector_type == SECT_INSIDE
		||   pRoomIndex->sector_type == SECT_CITY )
		return false;
	
    if( weather_info[pRoomIndex->sector_type].sunlight == SUN_SET
		||   weather_info[pRoomIndex->sector_type].sunlight == SUN_DARK )
		return true;
	
    return false;
}

/**************************************************************************/
// true if player is in room invite list, or their clan is.
bool player_on_rooms_invite_list(char_data *ch, ROOM_INDEX_DATA *room)
{
	if(GAMESETTING4(GAMESET4_ROOM_INVITES_DISABLED)){
		return false;
	}

	if(IS_NULLSTR(room->owner)){
		return false;
	}
	if(IS_NULLSTR(room->invite_list)){
		return false;
	}
    if(is_exact_name(ch->name, room->invite_list)){
		return true;
	}
	if(ch->clan && is_exact_name(FORMATF("clan=%s", ch->clan->notename()), room->invite_list)){
		return true;
	}

	return false;    
}
/**************************************************************************/
bool is_room_owner(char_data *ch, ROOM_INDEX_DATA *room)
{
    if(IS_NULLSTR(room->owner)){
		return false;
	}

    if(is_exact_name(class_table[ch->clss].name,room->owner)){
		return true;
	}
     
    if(is_exact_name("immortal",room->owner) && IS_IMMORTAL(ch)){
		return true;
	}
	return is_exact_name(ch->name,room->owner);
}


/**************************************************************************/
// True if room is private excluding the owner question.
bool is_room_private_except_owner( ROOM_INDEX_DATA *pRoomIndex )
{
    char_data *rch;
    int count = 0;

    for( rch = pRoomIndex->people; rch; rch = rch->next_in_room )
	{
		if( INVIS_LEVEL(rch)< LEVEL_IMMORTAL )
			count++;
	}

    if( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return true;

    if( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return true;
    
    if( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) )
	return true;

    return false;

}

/**************************************************************************/
// True if room is private.
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    if(!IS_NULLSTR(pRoomIndex->owner)){
		return true;
	}

	return is_room_private_except_owner( pRoomIndex );
}
/**************************************************************************/
// True if room is private to the character
bool is_room_private_to_char( ROOM_INDEX_DATA *pRoomIndex, char_data *ch )
{
	if(IS_ADMIN(ch) && IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK)){
		return false;
	}

	if(!IS_NULLSTR(pRoomIndex->owner)){
		if(!is_room_owner(ch, pRoomIndex)
			&& !player_on_rooms_invite_list(ch, pRoomIndex))
		{
			return true;
		}
	}

	return is_room_private_except_owner( pRoomIndex );
}

/**************************************************************************/
// visibility on a room -- for entering and exits
bool can_see_room( char_data *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if(IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    &&  get_trust(ch) < MAX_LEVEL)
	return false;

    if(IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return false;

    if(IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
    &&  !IS_IMMORTAL(ch))
	return false;

    if(IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
    &&  ch->level > 5 && !IS_IMMORTAL(ch))
	return false;

    if(!IS_IMMORTAL(ch) && pRoomIndex->clan && ch->clan != pRoomIndex->clan)
	return false;

	// unswitched_mobs can't see ooc rooms
	if(IS_UNSWITCHED_MOB(ch) 
		&& IS_SET(pRoomIndex->room_flags, ROOM_OOC))
		return false;

	// mortal players that arent building can't see olconly areas
	if(!IS_NPC(ch) && !IS_IMMORTAL(ch) && !IS_SET(ch->comm,COMM_BUILDING)
		&& IS_OLCAREA(pRoomIndex->area))
		return false;

	// mortal builders can't see ic areas
	if(!IS_NPC(ch) && !IS_IMMORTAL(ch) && IS_SET(ch->comm,COMM_BUILDING)
		&& !IS_SET(pRoomIndex->room_flags, ROOM_OOC) 
		&& !IS_SET(pRoomIndex->area->area_flags, AREA_OLCONLY)
		)	
		return false;

    return true;
}


/**************************************************************************/
// True if looker can see victim - (ch is doing the looking)
bool can_see( char_data *looker, char_data *victim )
{
    if( looker == victim )
		return true;

	// support the inroom command
	if(IS_SET(looker->dyn,DYN_IN_ROOM_ONLY) && looker->in_room!=victim->in_room)
	{		
		return false;
	}

	// mortals can't see ploaded players
	if(victim->pload && !IS_IMMORTAL(looker)){
		return false;
	}

	// support for the 'mob seeall' command
	if(IS_SET(looker->dyn,DYN_MOB_SEE_ALL) 
		&& INVIS_LEVEL(victim)<=LEVEL_HERO)
	{
		return true;
	}
	
    if( get_trust(looker) < INVIS_LEVEL(victim))
	return false;

    if(IS_IMMORTAL(victim)
		&& get_trust(looker) < victim->incog_level 
		&& looker->in_room != victim->in_room)
		return false;

    if( !IS_NPC(looker) && IS_NPC(victim)
		&& IS_SET(victim->act, ACT_IS_UNSEEN)
		&& !HAS_HOLYLIGHT(looker))
		return false;

    if( IS_OOC(victim))
	    return true;

    if( HAS_HOLYLIGHT(looker))
		return true;

	if( IS_SET( victim->affected_by2, AFF2_TREEFORM ) 
		&& !IS_AFFECTED2(looker, AFF2_DETECT_TREEFORM ) )
		return false;
		
	if( IS_SET( victim->affected_by2, AFF2_VANISH )
		&& !(get_skill(looker,gsn_vanish) > 0)
		&& !IS_AFFECTED2(looker, AFF2_DETECT_VANISH))
		return false; 

    if( IS_AFFECTED(looker, AFF_BLIND) )
		return false;

    if( room_is_dark( looker->in_room ) &&
      (!IS_AFFECTED(looker, AFF_DARK_VISION) || 
       !IS_AFFECTED(looker, AFF_INFRARED)) )
		return false;

    if( IS_AFFECTED(victim, AFF_INVISIBLE)
		&&   !IS_AFFECTED(looker, AFF_DETECT_INVIS ))
		return false;

	if(!(IS_NPC(looker) && IS_AFFECTED(looker, AFF_DETECT_HIDDEN)) )
	{
		// hidden - chance of not being seen
		//   The higher 'chance' is the less chance of the looker (looker) 
		//   seeing the hiding mob/player (victim).
		if(IS_AFFECTED(victim, AFF_HIDE) 
			&& !is_same_group( looker, victim)
			&& victim->fighting == NULL)
		{
			int chance; // chance of not being seen
			OBJ_DATA  *obj;
			chance = UMAX(90,get_skill(victim,gsn_hide));
			chance += victim->level/5;
			chance -= looker->level/3;
			chance += (victim->level- looker->level)/3;
			chance -= IS_NPC(looker)?0:looker->pcdata->tired*2;
			chance -= looker->modifiers[STAT_AG];
			chance -= looker->modifiers[STAT_QU];
			chance += victim->modifiers[STAT_IN];
			chance += victim->modifiers[STAT_PR];
			chance -= get_skill(looker,gsn_hide)/10;

			if(room_is_dark( victim->in_room ) &&
			 !IS_AFFECTED(looker, AFF_DARK_VISION)){
				obj= get_eq_char( victim, WEAR_LIGHT );
				if(	obj &&	obj->item_type == ITEM_LIGHT
					&&	obj->value[2] != 0 ){
					chance -= 50; // hider has light
					// looker lookereck
					obj= get_eq_char( looker, WEAR_LIGHT );
					if(	obj &&	obj->item_type == ITEM_LIGHT
						&&	obj->value[2] != 0 ){
						chance += 15; // both have light
					}else{
						chance -= 50; // hider only has light
					}
				}else{
					chance += 10; // hider doesn't have light
					// looker lookereck
					obj= get_eq_char( looker, WEAR_LIGHT );
					if(	obj &&	obj->item_type == ITEM_LIGHT
						&&	obj->value[2] != 0 ){
						chance -= 50; // looker only has light
					}else{
						chance += 50; // neither have light
					}
				}

			}

			chance = UMAX(chance,1);

			if(IS_AFFECTED(looker, AFF_DETECT_HIDDEN)){
				chance/=4;
			}else{
				chance*=2;
			}
			// spamming look to see someone results in you having
			// less chance of seeing them
			if(looker->desc && looker->desc->repeat>3)
			{
				chance+=5;
				chance*=8;
			}else{
				chance = URANGE(5,chance,99);
			}

			if(chance-10 >number_percent())
				return false;
		}
	}
    return true;
}


/**************************************************************************/
// True if looker can see victim on the wholist.
bool can_see_who( char_data *looker, char_data *victim )
{
	if(!looker || !victim){
		bug("can_see_who - null looker or victim");
		make_corefile();
		return false;
	}

	// mobs arent on who
    if( IS_NPC(victim) ){
		return false;
	}

	// can always see yourself on who
    if( looker == victim ){
		return true;
	}

	// support for the 'mob seeall' command
	if(IS_SET(looker->dyn,DYN_MOB_SEE_ALL)){
		return true;
	}

	// mortals see imms based soley on the status of whovis
	// whovis has no affect on characters less than LEVEL_IMMORTAL
	if(!IS_IMMORTAL(looker) && victim->level>LEVEL_HERO){
		if(IS_SET(victim->comm, COMM_WHOVIS))
			return true;
		else
			return false;
	}

	// newbies can always see newbie support on who
    if(IS_NEWBIE_SUPPORT(victim) && IS_NEWBIE(looker)){
        return true;
	}

#ifdef unix
    // mortals can't see someone for 60 seconds after 
    // they logon and haven't talked on ooc 
    if(!IS_IMMORTAL(looker) && !IS_NPC(looker) && ((current_time - 60) < victim->logon) )
    {
        if(!IS_NPC(victim) && !victim->pcdata->did_ooc && looker!=victim)
            return false;
    }
#endif

	// incog only works for imms in an IC sense
	if(IS_IMMORTAL(victim) && get_trust(looker) < victim->incog_level 
		&& looker->in_room != victim->in_room){
		return false;
	}

	// support the inroom command
	if(IS_SET(looker->dyn,DYN_IN_ROOM_ONLY) && looker->in_room!=victim->in_room){
		return false;
	}
	// trust check
    if( get_trust(looker) < INVIS_LEVEL(victim)){
		return false;
	}

	return true;
}
/**************************************************************************/
ROOM_INDEX_DATA *room_object_is_in(obj_data *obj)
{
	if(obj->in_room){
		return obj->in_room;
	}else if(obj->carried_by){			
		return obj->carried_by->in_room;
	}else if(obj->in_obj){
		return room_object_is_in(obj->in_obj);
	}
	bugf("room_object_is_in(): couldn't find the room which object '%s' was in!\r\n",
		obj->name);
	return NULL;
}
/**************************************************************************/
// True if char can see obj.
bool can_see_obj( char_data *ch, obj_data *obj )
{
	// support the inroom command
	if(IS_SET(ch->dyn,DYN_IN_ROOM_ONLY) && ch->in_room!=room_object_is_in(obj)){
		return false;
	}

    if( HAS_HOLYLIGHT(ch))
		return true;

	// support for the 'mob seeall' command
	if(IS_SET(ch->dyn,DYN_MOB_SEE_ALL)){
		return true;
	}

	if( obj->item_type == ITEM_TOKEN )
		return false;

	if( IS_SET( obj->extra2_flags, OBJEXTRA2_BURIED ))
		return false;

    if( IS_SET(obj->extra_flags,OBJEXTRA_VIS_DEATH))
		return false;


    if( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
		return false;

    if( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
		return true;

    if( IS_SET(obj->extra_flags, OBJEXTRA_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
        return false;

    if( IS_OBJ_STAT(obj,OBJEXTRA_GLOW))
		return true;

    if( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_DARK_VISION) )
		return false;

    return true;
}

/**************************************************************************/
// True if char can drop obj.
bool can_drop_obj( char_data *ch, OBJ_DATA *obj )
{
    if( !IS_SET(obj->extra_flags, OBJEXTRA_NODROP) )
		return true;
	
    if( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
		return true;
	
    return false;
}



/**************************************************************************/
// Return ascii name of an item type.
char *item_type_name( OBJ_DATA *obj )
{	
	return flag_string(item_types, obj->item_type);
}

/**************************************************************************/
// Return ascii name of an affect location.
char *affect_loc_name( int location )
{
	return flag_string(apply_types, location);
}

/**************************************************************************/
char *full_affect_loc_name( AFFECT_DATA *paf)
{
	static char result[MIL];
    switch( paf->location )
    {
	case APPLY_NONE:
		{
			
			switch(paf->where)
			{
			case WHERE_WEAPON:
				sprintf( result,"%s", weapon_bit_name( paf->bitvector ));
				break;
			case WHERE_VULN:
				sprintf( result,"%s (vuln)", imm_bit_name( paf->bitvector ));
				break;
				
			case WHERE_RESIST:
				sprintf( result,"%s (resist)", imm_bit_name( paf->bitvector ));
				break;
				
			case WHERE_IMMUNE:
				sprintf( result,"%s (immune)", imm_bit_name( paf->bitvector ));
				break;
			case WHERE_OBJEXTRA:
				sprintf( result,"%s (object)", extra_bit_name(paf->bitvector));
				break;
			case WHERE_OBJEXTRA2:
				sprintf( result,"%s (object)", extra2_bit_name(paf->bitvector));
				break;
			case WHERE_AFFECTS:
			default:
				sprintf( result,"%s", affect_bit_name( paf->bitvector ));
				break;
			case WHERE_AFFECTS2:
				sprintf( result,"%s", affect2_bit_name( paf->bitvector ));
				break;
			case WHERE_AFFECTS3:
				sprintf( result,"%s", affect3_bit_name( paf->bitvector ));
				break;
			}
			return result;
		}
	default:
		return flag_string(apply_types, paf->location);
    }
	
    bugf( "Affect_location_name: unknown location %d.", paf->location );
    return "(unknown)";
}

/**************************************************************************/
// return text flags of what the bits are for
char *affect_bit_name( int bits)
{
	return flag_string(affect_flags, bits);
}

/**************************************************************************/
char *affect2_bit_name( int bits)
{
	return flag_string(affect2_flags, bits);
}

/**************************************************************************/
char *affect3_bit_name( int bits)
{
   return flag_string(affect3_flags, bits);
}
/**************************************************************************/
char *extra_bit_name( int bits )
{
	return flag_string(objextra_flags, bits);
}

/**************************************************************************/
char *extra2_bit_name( int bits)
{
	return flag_string(objextra2_flags, bits);
}

/**************************************************************************/
char *act_bit_name( int bits)
{
    if(IS_SET(bits,ACT_IS_NPC))
    { 
		return flag_string(act_flags, bits);		
	}else{
		return flag_string(plr_flags, bits);		
	}
}

/**************************************************************************/
char *act2_bit_name( int bits)
{	
	return flag_string(act2_flags, bits);		
}

/**************************************************************************/
char *comm_bit_name(int bits)
{
	return flag_string(comm_flags, bits);		
}

/**************************************************************************/
char *imm_bit_name(int bits)
{
	
	return flag_string(imm_flags, bits);
}

/**************************************************************************/
char *wear_bit_name(int bits)
{
	return flag_string(wear_flags, bits);
}

/**************************************************************************/
char *form_bit_name(int bits)
{
	return flag_string(form_flags, bits);
}

/**************************************************************************/
char *part_bit_name(int bits)
{
	return flag_string(part_flags, bits);
}
/**************************************************************************/
char *weapon_bit_name(int bits)
{
	return flag_string(weapon_flags, bits);
}
/**************************************************************************/
char *cont_bit_name( int cont_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if(cont_flags & CONT_CLOSEABLE	) strcat(buf, " closable");
    if(cont_flags & CONT_PICKPROOF	) strcat(buf, " pickproof");
    if(cont_flags & CONT_CLOSED	) strcat(buf, " closed");
    if(cont_flags & CONT_LOCKED	) strcat(buf, " locked");

    return(buf[0] != '\0' ) ? buf+1 : (char*)"none";
}

/**************************************************************************/
char *off_bit_name(int bits)
{
	return flag_string(off_flags, bits);
}
/**************************************************************************/
char *room_flags_bit_name(int bits)
{
	return flag_string(room_flags, bits);
}    
/**************************************************************************/
/*
 * Short description function - by Kalahn, displays the word YOU if 
 *   ch==looker
 */
char * YOU_PERS( char_data *ch, char_data *looker)
{
	if(!ch || !looker){
		return "";
	}
	if(ch==looker){
		return("you");
	}
	
	return(PERS(ch, looker));
}
/**************************************************************************/
/*
 * Long description for mobs, short for players function
 */
char * LONGPERS( char_data *ch, char_data *looker)
{
	static char buf[MSL];
	char *descript="";
	
	// use long of mobs, short of pcs
	if(IS_NPC(ch)){
		descript=ch->long_descr;
	}else{
		descript=ch->short_descr;
	}

	strcpy(buf, mxp_create_tag(looker, mxp_tag_for_mob(looker, ch), capitalize(descript)));

	if(!IS_UNSWITCHED_MOB(looker))
	{
		if(IS_CONTROLLED(ch) && IS_SET(looker->act, PLR_HOLYNAME))
		{ // add the little controling name
			strcat( buf, FORMATF(" [%s]", TRUE_CH(ch)->name)) ;
		}
	}

	if(!IS_UNSWITCHED_MOB(looker) &&
		HAS_DESC(looker) && IS_SET(TRUE_CH(looker)->act, PLR_HOLYVNUM))
	{
		char buf2[MSL];
		if(IS_NPC(ch) && ch->pIndexData)
		{
			if(ch->pIndexData->mob_triggers)
			{
				sprintf(buf2," `#`=H*%d,%d*`&", ch->pIndexData->vnum, ch->level);
			}
			else
			{
				sprintf(buf2," `#`=h[%d,%d]`&", ch->pIndexData->vnum, ch->level);
			}
			strcat(buf,buf2);
		}
	}

	return(buf);
}

/**************************************************************************/
// char * char_position_text(char_data *ch )
// returns the correct position text for a characters position.
// eg " is <postion name> in <object_short>.",
//    " is <postion name> on <object_short>.",
//    " is <postion name> at <object_short>.",
//    " is <postion name> here."
char * char_position_text( char_data *ch)
{
   static char buf[MSL];
   sh_int pos = ch->position;

   if(ch->on != NULL){         
      if(IS_SET(ch->on->value[2],SLEEP_AT)
       || IS_SET(ch->on->value[2],SIT_AT)
       || IS_SET(ch->on->value[2],REST_AT)
       || IS_SET(ch->on->value[2],KNEEL_AT))
     {
         sprintf(buf," is %s at %s.", position_table[pos].name, ch->on->short_descr);
      }else if(IS_SET(ch->on->value[2],SLEEP_ON)
            || IS_SET(ch->on->value[2],SIT_ON)
            || IS_SET(ch->on->value[2],REST_ON)
            || IS_SET(ch->on->value[2],KNEEL_ON))
     {
         sprintf(buf," is %s on %s.", position_table[pos].name, ch->on->short_descr);
      }else if(IS_SET(ch->on->value[2],SLEEP_UNDER)
            || IS_SET(ch->on->value[2],SIT_UNDER)
            || IS_SET(ch->on->value[2],REST_UNDER)
            || IS_SET(ch->on->value[2],KNEEL_UNDER))
     {
         sprintf(buf," is %s under %s.", position_table[pos].name, ch->on->short_descr);
      }else{
         sprintf(buf," is %s in %s.", position_table[pos].name, ch->on->short_descr);
      }
   }else{
      sprintf(buf," is %s here.", position_table[pos].name);
   }
   return buf;
}

/**************************************************************************/
// returns the characters after a / from an area filename
char * area_fname (AREA_DATA *pArea)
{
    static char buffer[128]; /* short filename */
    char  *slash;  /* used for finding a slash */

    if(!pArea)
    {
        bug("Area handed to area_fname was null");
        return("");
    }

    strncpy(buffer, pArea->file_name, 128); /* copy the filename */
    slash = strchr (buffer, '/'); /* find the slash (area/midgaard.are) */
    if(slash) /* if there was one move one after it*/
        slash++;      
    else 
        slash = buffer; /*otherwise set to the first word */

    return(slash);
}

/**************************************************************************/
char *shortdate(time_t *tm) // kalahn - sept 97 
{
	static int i;
    static char result[3][30];
	// rotate buffers
	++i= i%3;
    result[i][0] = '\0';

	if(!tm){
		tm=&current_time;
	}

    char *tbuf = ctime( tm );
    tbuf[str_len(tbuf)-6] = '\0';

    strcpy(result[i], tbuf);

    return(result[i]);
}

/**************************************************************************/
char *shorttime(time_t *tm) // kalahn - nov 97 
{
	static int i;
    static char result[3][30];
	// rotate buffers
	++i= i%3;
    result[i][0] = '\0';

	if(!tm){
		tm=&current_time;
	}

    char *tbuf = ctime( tm);
    tbuf[str_len(tbuf)-6] = '\0';

    strcpy(result[i], (char *)&tbuf[11]);

    return(result[i]);
}
/**************************************************************************/
// Append a string to a file.
void append_string_to_file( const char *file, const char *str, bool newline )
{
    FILE *fp;

    if( str[0] == '\0' )
	return;

	if(fpAppend2FilReserve==NULL && runlevel==RUNLEVEL_SHUTING_DOWN)
		return;


	if(fpAppend2FilReserve){
		fclose(fpAppend2FilReserve);
	}

    if( ( fp = fopen( file, "a" ) ) == NULL ){
		bugf("append_string_to_file(): fopen '%s' for append - error %d (%s)",
			file, errno, strerror( errno));
    }else{
		if(newline){
			fprintf( fp, "%s\n", str );
		}else{
			fprintf( fp, "%s", str );
		}
        fclose( fp );
    }
    fpAppend2FilReserve = fopen( ANULL_FILE, "r" );
}

/**************************************************************************/
// Append a date and time coded string to a file.
void append_datetimestring_to_file( const char *file, const char *str )
{
    char buf[MSL];

    sprintf(buf, "%s: ", shortdate(NULL));

    append_string_to_file( file, buf, false);
	append_string_to_file( file, str, true);
}
/**************************************************************************/
// Append a time coded string to a file.
void append_timestring_to_file( const char *file, const char *str )
{
    char buf[MSL];

    sprintf(buf, "%s: ", shorttime(NULL));
    append_string_to_file( file, buf, false);
	append_string_to_file( file, str, true);	
}
/**************************************************************************/
// appends to file short time code, room, players name
void append_logentry_to_file( char_data *ch, const char *file, const char *str )
{
    char logbuf[MSL];

    sprintf(logbuf, "[%5d] %s: %s",
       ch->in_room ? ch->in_room->vnum : 0, TRUE_CH(ch)->name, str );

    append_datetimestring_to_file(file, logbuf);
}
/**************************************************************************/
/*
 * appends to a players log the string with a short time code on it
 * and a [O] if they are in an ooc room
 */
void append_playerlog( char_data *ch, char *str )
{
    char logfname[MSL];
    char buf[MSL];
	char buf_vnum[MSL];
	char name[MIL];

	// make lowercase and first word of the name
	one_argument( TRUE_CH(ch)->name, name);

	if(IS_IMMORTAL(ch)){
		sprintf(logfname, "%s%s.log", IMMLOG_DIR, name);
	}else{
		sprintf(logfname, "%s%s.log", PLAYER_LOGS_DIR, name);
	}

	sprintf(buf_vnum, "<%5d>", (ch->in_room? ch->in_room->vnum:0));

    if(IS_OOC(ch)){
        sprintf(buf,"%s [O] %s`x", buf_vnum, str);
    }else{
        sprintf(buf,"%s %s`x", buf_vnum, str);
	}

    append_datetimestring_to_file(logfname, buf);
}

/**************************************************************************/
/*
 * appends to the string with a short time code on it
 * and a [O] if they are in an ooc room 
 * [N] for newbie support person
 */
void append_newbie_support_log( char_data *ch, char *str )
{
    char buf[MSL];

    
	sprintf(buf,"%3s%3s %-13s:%s", 
		IS_OOC(ch)?"[O]":"",
		IS_NEWBIE_SUPPORT(ch)?"[N]":"",
		ch->name, str);

    append_datetimestring_to_file(NEWBIE_SUPPORT_LOG_FILE, buf);
}

/**************************************************************************/
/*
 * appends to file short date time code, room, players name
 */
void append_datetime_ch_to_file( char_data *ch, const char *file, const char *str )
{
    char logbuf[MSL];

    sprintf(logbuf, "%-13s<%2d>- %s", ch->name, ch->level, str );

    append_datetimestring_to_file(file, logbuf);
}

/**************************************************************************/
void rand_to_char(char_data *ch, char *str1, char *str2, char *str3, char *str4, char *str5) 
{
	char *output="";
    switch(dice(1,5))
    {
	case 1 : output=str1; break;
	case 2 : output=str2; break;
	case 3 : output=str3; break;
	case 4 : output=str4; break;
	case 5 : output=str5; break;
	default:bug("BUG: rand_to_char: number out of range.");
		do_abort(); // this should never be possible		
		break;
    }
	ch->print(output);
}

/**************************************************************************/
// Returns the visible length of a string after colour parsing
// written by Kalahn - march 98
// see also str_width()
// note: `1 and `} returns a -1 (new line codes)
int c_str_len(const char *s)
{
	int len = 0; // visible string length (without colour codes)

	for( ; *s; s++ )
	{
		if( *s == '`' ) // skip the colour code
		{
			s++;		// move to the colour character
			switch(*s)
			{
				default:
					break;

				case '=': // custom colour
					s++; // skip the '=' moving to the colour char...
						 // we don't bother counting it since we assume 
						 // it is always a colour
					break;

				case '-': // ~
				case '`': // ` 
					len++;
					break;

				case 'N': // N - game name
					len+=str_len(game_settings->gamename);
					break;

				case '1': // newline returns a -1 
				case '}': // newline returns a -1 
					return -1;
					break;
			}
			continue;
		}	
		len++;	// normal character count that length
	}
	return len;
}

/**************************************************************************/
// used to detect arguments sent by players to defraud systems
// like sending ``1 over ooc to make a new line and then putting
// new text to send to everyone.
bool check_defrauding_argument(char_data *ch, char *argument ){
	if(IS_NULLSTR(argument)){
		return false;
	}

	if(c_str_len(argument)==-1){
		ch->println("Messages containing colour codes which create new lines will `RNOT`x be sent.");
		return true;
	}
	return false;
}

/**************************************************************************/
// Returns true if str has a ` colour code in it
// written by Kalahn - march 98
bool has_colour(const char *s)
{
	for( ; *s; s++ )
	{
		if( *s == '`' ) // find a colour code
			return true;
	}
	return false;
}

/**************************************************************************/
// Returns true if str has a space character in it
// written by Kalahn - march 98
bool has_space(const char *s)
{
	for( ; *s; s++ )
	{
		if( *s == ' ' ) // find a space
			return true;
	}
	return false;
}
/**************************************************************************/
// Returns true if str has any whitespace character in it
// written by Kalahn - march 98
bool has_whitespace(const char *s)
{
	for( ; *s; s++ )
	{
		if( is_space(*s)){ // find a whitespace
			return true;
		}
	}
	return false;
}

/**************************************************************************/
// Returns width amount of characters not counting colour codes for width
// The input string will be padded with spaces at the end if necessary
// see also c_str_len()
// if cut_if_too_long is true, the resulting string will be cut at the
// length if it is longer than width
// Kalahn - Mar 98
char * str_width(const char *s, int width, bool cut_if_too_long)
{
	static int i;
    static char buf[5][MSL];
	++i%=5;

	char *point;
	int len = 0; // visible string length (without colour codes)

	point=buf[i]; // start at the start of the input string s
	point--;

	for( ; *s && (!cut_if_too_long || len<width); s++ )
	{	
		*++point= *s;   // copy the character
		if( *s == '`' ) // if we have a colour code copy the next char
		{
			s++;		// move to the colour character
			*++point= *s; // copy the colour character
			if(*s== '`'){ // count only the length of `` codes
				len++;
			}
			if(*s== '='){  // custom colour code... move to custom code
				s++;		// move to the custom colour code character
				*++point= *s; // copy the custom colour code character
			}
			continue;
		}	
		len++;	// normal character count that length
	}
	
	// if we need some blank spaces at the end of the string
	while(len<width)
	{
		*++point= ' ';
		len++;	
	}

	*++point = '\0'; // terminate the string
	return buf[i];
}

/**************************************************************************/
void center_to_char(char *argument, char_data *ch, int columns)
{
	int spaces;
	columns = (columns < 2) ? 80 : columns;
	spaces=(columns-c_str_len(argument))/2;

	ch->printf("%*c%s",spaces,' ',argument);
	return;
}
/**************************************************************************/
void centerf_to_char (char_data *ch, int cols, const char *fmt, ...)
{
    char buf[MSL];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);
	
	center_to_char(buf, ch, cols);
}

/**************************************************************************/
void bugf (const char * fmt, ...)
{
    char buf[MSL*3];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL*3, fmt, args);
	va_end(args);
	bug(buf);
}

/**************************************************************************/
char *mprog_type_to_name( int type );
/**************************************************************************/
// Kal - Feb 00
// Used to log a buggy prog, and disable it at the same time... 
// till the next mpreset
void mpbuggy_prog(MUDPROG_TRIGGER_LIST *trigger, char * fmt, ...)
{
    char buf[HSL];
	char *pBuf;

	assert(trigger->prog->disabled==false); // shouldn't have been attempting to run
	trigger->prog->disabled=true;

	//	add_buf(output,"{xMPBUG {G#mudprog{x, {Ymobvnum{x, {Binroom{x, {rline{x, {Ccall_level{x:\r\n");
	sprintf(buf,"`S#############\r\n"
		"             Disabling mudprog `G%d`x,`Y%d`x,`B%d`x,`r%d`x,`C%d`x:\r\n" 
		"             `Y",
		callstack_pvnum[call_level-1],
		callstack_mvnum[call_level-1],
		callstack_rvnum[call_level-1],
		callstack_line[call_level-1],
		call_level-1);

	// format and concate the error message text
	pBuf=&buf[str_len(buf)];
	va_list args;
	va_start(args, fmt);
	vsnprintf(pBuf, MSL, fmt, args);
	va_end(args);

	// put extra debugging info from the trigger
	pBuf=&buf[str_len(buf)];
	sprintf(pBuf,	"\r\n`x             Trigger type '%s' phrase '%s'\r\n",		
		mprog_type_to_name(trigger->trig_type),
		trigger->trig_phrase);

	strcat(pBuf,"             Call stack info:\r\n");

	// display the call stack
	char catbuf[MSL];
	for(int i=0; i<call_level;i++)
	{
		if(!callstack_aborted[i] || (i==call_level-1)){
			sprintf(catbuf, "             MUDprog %d on mob %d (in room %d), line %d\r\n", 
				callstack_pvnum[i], 
				callstack_mvnum[i], 
				callstack_rvnum[i],
				callstack_line[i]);
			strcat(pBuf,catbuf);
		}
	}
	strcat(pBuf,"`S#############\r\n\r\n");
	append_timestring_to_file(MPBUG_FILE, buf);
	trigger->prog->disabled_text=str_dup(buf); // record what is buggy with it
}
/**************************************************************************/
// mudprog bug log system
void mpbug(const char *mudprog_bug_text)
{
	mpbugf("%s", mudprog_bug_text);
}
/**************************************************************************/
// mudprog bug log system with formatting
void mpbugf(const char * fmt, ...)
{
	char *pBuf;
//	add_buf(output,"{xMPBUG {G#mudprog{x, {Ymobvnum{x, {Binroom{x, {rline{x, {Ccall_level{x:\r\n");

    char buf[HSL];
	sprintf(buf,"`G%d`x,`Y%d`x,`B%d`x,`r%d`x,`C%d`x:", 
		callstack_pvnum[call_level-1],
		callstack_mvnum[call_level-1],
		callstack_rvnum[call_level-1],
		callstack_line[call_level-1],
		call_level-1);

	pBuf=&buf[str_len(buf)];
	va_list args;
	va_start(args, fmt);
	vsnprintf(pBuf, MSL, fmt, args);
	va_end(args);

	append_timestring_to_file(MPBUG_FILE, buf);

}

/**************************************************************************/
void logf(const char * fmt, ...)
{
    char buf[HSL];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, HSL-MIL, fmt, args);
	va_end(args);

	log_string(buf);
}
/**************************************************************************/
void broadcast(char_data *except, const char * fmt, ...)
{
    char buf[MSL];
	char_data *wch;
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);

	for( wch = player_list; wch; wch = wch->next_player )
    {
		if(wch!=except){	
			ACTIVE_CH(wch)->print(buf);
		}
	}
}
/**************************************************************************/
void pkill_broadcast(const char * fmt, ...)
{
    char buf[MSL], buf2[MSL];
	char_data *wch;
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);

	sprintf(buf2,"`RPKILL PORT BROADCAST:`W %s\r\n`x", buf);

	for( wch = player_list; wch; wch = wch->next_player )
    {
		wch->print(buf2);
	}

}
/**************************************************************************/
char * format_titlebar(const char *heading)
{
	static char returnbuf[MSL];
	char line[MSL];
	char line2[MSL];	
	int spaces;

	if(c_str_len(heading)<1 || (c_str_len(heading)==1 && (heading[0]=='-' || heading[0]=='=') ))
	{
		return "`#`=t-====================================="
			"======================================-`&\r\n";
	}

	if(c_str_len(heading)>78)
	{
		sprintf(returnbuf,"%s\r\n",heading);
		return returnbuf;
	}


	spaces= (74-c_str_len(heading))/2;
	for(int j=0;j<spaces; j++)
	{
		line[j]='=';
	}
	line[spaces]='\0';

	if(spaces%2==0){
		sprintf(returnbuf,"`#`=t-%s`# `=T%s `&%s-`&\r\n",line, heading, line);
	}else{
		strcpy(line2, line);
		line2[spaces-1]='\0';
		sprintf(returnbuf,"`#`=t-%s`# `=T%s `&%s-`&\r\n",line, heading, line2);
	}
	return returnbuf;
}
/**************************************************************************/
char * format_titlebarf(const char *fmt, ...)
{
	char buf [MSL];
	// format all the text into buf
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);

	return format_titlebar(buf);
}
/**************************************************************************/
/* get the 'short' name of an area (e.g. MIDGAARD, MIRROR etc. */
/* assumes that the filename saved in the AREA_DATA struct is something like midgaard.are */
char * area_name (AREA_DATA *pArea)
{
	static char buffer[64]; /* short filename */
	char  *period;

	assert(pArea != NULL);
	
	strncpy(buffer, pArea->file_name, 64); /* copy the filename */	
	period = strchr (buffer, '.'); /* find the period (midgaard.are) */
	if(period) /* if there was one */
		*period = '\0'; /* terminate the string there (midgaard) */
		
	return buffer;	
}
/**************************************************************************/
char *percent_colour_codebar(void){
	return("`c0-3`x,`g4-11`x,`m12-18`x,`r19-26`x,`y27-33`x,`S34-41`x,"
	"`w42-48`x,`W49-56`x,`Y57-63`x,`R64-71`x,`M72-78`x,"
	"`G79-86`x,`C87-93`x,`B94+`x\r\n");

}
/**************************************************************************/
char *percent_colour_code(sh_int val){
    static char col[5][MIL];
	static sh_int i;
	float tval;
	sh_int newval;
	// rotate buffers
	++i= i%5;

	tval=(float)val;
	tval*=2;
	tval/=15;
	tval+=0.5;
	newval=(sh_int)tval;

    switch(newval)
    {
        case 0:
            strcpy(col[i], "`c");
            break;
        case 1:
            strcpy(col[i], "`g");
            break;
        case 2:
            strcpy(col[i], "`m");
            break;
        case 3:
            strcpy(col[i], "`r");
            break;
        case 4:
            strcpy(col[i], "`y");
            break;
        case 5:
            strcpy(col[i], "`S");
            break;
        case 6:
            strcpy(col[i], "`w");
            break;
        case 7:
            strcpy(col[i], "`W");
            break;
        case 8:
            strcpy(col[i], "`Y");
            break;
        case 9:
            strcpy(col[i], "`R");
            break;
        case 10:
            strcpy(col[i], "`M");
            break;
        case 11:
            strcpy(col[i], "`G");
            break;
        case 12:
            strcpy(col[i], "`C");
            break;
        case 13:
        case 14:
            strcpy(col[i], "`B");
            break;
    }
	return col[i];
}
/**************************************************************************/
// returns the underscored lowercase word
char * underscore_word(char *word)
{
	static char rbuf[3][MIL];
	static int index;
	char * pStr;

	++index%=3; // rotate return buffer

	sprintf(rbuf[index], "%s", word);

	pStr=&rbuf[index][0];

	// convert spaces into _ characters
	// and upper to lowercase
	do{
		if(*pStr==' '){
			*pStr='_';
		}else{
			*pStr=tolower(*pStr);
		}
	}while (*(++pStr));

	return rbuf[index];
}

/**************************************************************************/
char * lowercase(const char *str){
	static int i;
    static char buf[5][1024];
	int pos;
	// rotate buffers
	++i= i%5;
	
    for( pos = 0; str[pos] != '\0'; pos++ )
		buf[i][pos] = LOWER(str[pos]);
    buf[i][pos] = '\0';

	return buf[i]; 
}
/**************************************************************************/
char * uppercase(const char *str){
	static int i;
    static char buf[5][1024];
	int pos;
	// rotate buffers
	++i= i%5;
	
    for( pos = 0; str[pos] != '\0'; pos++ )
		buf[i][pos] = UPPER(str[pos]);
    buf[i][pos] = '\0';

	return buf[i]; 
}
/**************************************************************************/
void affect_to_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;
	
    paf_new = new_affect();
	
    *paf_new		= *paf;
    paf_new->next	= room->affected;
    room->affected	= paf_new;
	
    if( paf->where == WHERE_AFFECTS )
		SET_BIT(room->affected_by,paf->bitvector);
	
    return;
}
/**************************************************************************/
void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf )
{
    int where, vector;
    if( room->affected == NULL )
    {
        bug("Affect_remove_room: no affect.");
        return;
    }
	
    where = paf->where;
    vector = paf->bitvector;
	
    if(paf->bitvector)
		switch( paf->where)
	{
        case WHERE_AFFECTS:
            REMOVE_BIT(room->affected_by,paf->bitvector);
            break;
	}
	
    if( paf == room->affected )
    {    
        room->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;
		
        for( prev = room->affected; prev != NULL; prev = prev->next )
        {
            if( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }
		
        if( prev == NULL )
        {
            bug("Affect_remove_room: cannot find paf.");
            return;
        }
    }
	
    free_affect(paf);
	return;
	
}
/**************************************************************************/
void affect_to_skill( char_data *ch, int sn, int amount )
{
	if( IS_NPC( ch ) || sn < 0 ) return;

	if( TRUE_CH(ch)->pcdata->learned[sn] > 0 )
		TRUE_CH(ch)->pcdata->learned[sn] += amount;
    return;
}
/**************************************************************************/
// counts the number of times a character is in the buffer
int count_char(const char *buffer, char character)
{
	int result=0;
	
	if(buffer==NULL){
		return 0;
	}

	for(; *buffer; buffer++)
	{
		if(*buffer==character)
			result++;
	}

	return result;
}
/**************************************************************************/
extern bool mp_suppress_text_output;
/**************************************************************************/
// used when the moblog flag is on
void process_moblog(char_data *ch, const char *txt)
{
	if(!ch){
		return;
	}
	if( !IS_VALID(ch)){
		return;
	}
	if( !IS_NPC(ch) ){
		return;
	}

	if(!IS_NULLSTR(txt)
	&& IS_SET(ch->act,ACT_MOBLOG) )
	{
		char logbuf[MSL];
		// prevent buffer overruns
		char working[MSL];
		strncpy(working, txt, MSL-30);
		int len=UMIN(str_len(txt),MSL-40);		
		working[len]='\0';

		// ditch the \r codes
		strcpy(working, fix_string( working));

		// trim off the tail \n code if there
		if(working[str_len(working)-1]=='\n'){
			working[str_len(working)-1]='\0';
		}

		sprintf(logbuf, "[%5d] '%s' in room %d '%s%s'", 
			(ch->pIndexData?ch->pIndexData->vnum:0),
			ch->name,
			(ch->in_room? ch->in_room->vnum:0),
			mp_suppress_text_output?"[SUPPRESSED TEXT]":"",
			working);
		append_timestring_to_file( MOBLOG_LOGFILE, logbuf);
	};
}
/**************************************************************************/
// reports a wiznet message about multilogging
// handles restoring of colour coding
// header is displayed before custom text
// default colour of custom text is RED
void multilog_alertf(char_data *ch, const char * fmt, ...)
{
	char *pbuf;	
    char buf[HSL];
	if(!IS_NULLSTR(ch->remote_ip_copy)){
		sprintf(buf,"`#`R***MULTILOG ALERT - `x%s`R:",
			ch->remote_ip_copy);
	}else{
		strcpy(buf,"`#`R***MULTILOG ALERT: ");
	}

	pbuf=&buf[str_len(buf)];

	va_list args;
	va_start(args, fmt);
	vsnprintf(pbuf, MSL, fmt, args);
	va_end(args);
	strcat(buf,"`&");
	append_datetimestring_to_file(MULTILOG_FILE, buf);	
	wiznet(buf,NULL,NULL,WIZ_SITES,0,0);	
}
/**************************************************************************/
char *to_affect_string( AFFECT_DATA *paf, int objects_level, bool report_unknown_where_value_errors )
{
	static char buf[MSL*2];
	buf[0]='\0';
		
	switch(paf->where)
	{
	case WHERE_AFFECTS:
		sprintf( buf, "Adds %s affect.",
			affect_bit_name(paf->bitvector));
		break;
	case WHERE_AFFECTS2:
		sprintf( buf, "Adds %s affect2.",
			affect2_bit_name(paf->bitvector));
		break;
	case WHERE_AFFECTS3:
		sprintf( buf, "Adds %s affect3.",
			affect3_bit_name(paf->bitvector));
		break;
	case WHERE_OBJEXTRA:
		sprintf( buf, "Adds object extra flag(s) %s.",
			extra_bit_name(paf->bitvector));
		break;
	case WHERE_OBJEXTRA2:
		sprintf( buf, "Adds object extra2 flag(s) %s.",
			extra2_bit_name(paf->bitvector));
		break;
	case WHERE_IMMUNE:
		sprintf( buf, "Adds immunity to %s.",
			imm_bit_name(paf->bitvector));
		break;
	case WHERE_RESIST:
		sprintf( buf, "Adds resistance to %s.",
			imm_bit_name(paf->bitvector));
		break;
	case WHERE_VULN:
		sprintf( buf, "Adds vulnerability to %s.",
			imm_bit_name(paf->bitvector));
		break;
	// bitvector based affects above here


	case WHERE_MODIFIER:
		sprintf( buf, "Adds %s modifier of %d.", 
			flag_string(apply_types, paf->location), paf->modifier);
		break;
		
	case WHERE_WEAPON:
		sprintf( buf, "Adds weapon flag(s) %s .", flag_string(weapon_flags, paf->modifier));
		break;

	case WHERE_SKILLS:
		sprintf( buf, "Adds %d%% to %s.",
			paf->modifier, skill_table[paf->type].name );
		break;

	case WHERE_OBJECTSPELL:
		if(paf->level){
			sprintf( buf, "Casts '%s' at level %3d for a duration of %3d.",
				skill_table[paf->type].name, paf->level, paf->duration);
		}else{
			sprintf( buf, "Casts '%s' at objects level (%d) for a duration of %d.",
				skill_table[paf->type].name, objects_level, paf->duration);
		}
		break;

	default:
		if(report_unknown_where_value_errors){
			sprintf( buf, "to_affect_string(): Unknown where value %d: bitvector=%d", 
				paf->where, paf->bitvector);
		}
		break;
	}
	
	return buf;
}
/**************************************************************************/
// reports and logs bounds checking messages
void boundsbug(const char * fmt, ...)
{
    char buf[HSL];

	char *pbuf;	
	strcpy(buf,"`#`R***BOUNDS BUG ALERT: ");
	pbuf=&buf[str_len(buf)];

	va_list args;
	va_start(args, fmt);
	vsnprintf(pbuf, MSL, fmt, args);
	va_end(args);

	append_datetimestring_to_file(BOUNDSBUG_FILE, buf);	
	bug(buf);

	do_abort();
}
/**************************************************************************/
char *preference_word(PREFERENCE_TYPE pt)
{
	switch (pt){
		case PREF_OFF:			return "off";
		case PREF_AUTOSENSE:	return "autosense";
		case PREF_ON:			return "on";
		default:
			bugf("Unknown preference type %d", pt);
			do_abort();
	}
	return "";

}
/**************************************************************************/
// Kal - Apr 01
char *FORMATF(const char *formatbuf, ...)
{
	static int i;
    static char buf[10][MSL*3];
	++i%=10;

	va_list args;
	va_start(args, formatbuf);
	vsnprintf(buf[i], MSL*3, formatbuf, args);
	va_end(args);

	return buf[i];
}
/**************************************************************************/
// Kal - Jun 01 - unique id system
int get_next_uid()
{
	static int last=1000;
	return ++last;
}
/**************************************************************************/
// get_uid() - unique id system, returns 0 if it isn't a UID text reference
int get_uid(const char *text)
{
	const char *p=text;

	// skip leading whitespace
	while(is_space(*p)){
		p++;
	}

	if(*p=='#' && is_number(p+1)){ // it is a UID
		return atoi(p+1);
	}
	return 0;
}
/**************************************************************************/
// Kal - convert a 24 hour clock into 12 clock
char *convert24hourto12hour(int hour)
{
	// multibuffered
	static int i;
    static char buf[5][512];
	++i%=5;
    buf[i][0] = '\0';
	
	hour%=24; // avoid idiot results
	if(hour>11){ // 12 -> 23
		sprintf(buf[i], "%2dpm", ((hour-1)%12)+1);		
	}else{
		sprintf(buf[i], "%2dam", ((hour-12)%12)+12);
	}
	return buf[i];
}
/**************************************************************************/
// Kal - Sept 02
void println_delayed_to_room(int seconds, room_index_data *room, char_data *all_but_this_person, const char *text)
{
	char_data *p;

	// check the room is sane
	if(!room){
		bugf("delayed_printf_to_room(), given a NULL room!");
		return;
	}
	
	for( p=room->people; p; p=p->next_in_room ){
		if(p==all_but_this_person){
			continue;
		}
		p->println(seconds, text);
	}
}
/**************************************************************************/
// return "" if ch doesn't have autodamage enabled, otherwise return 
// the autodamage text required.
// Kal - Aug 03
char *autodamtext(char_data *ch, int damage_amount)
{
	static char result[MIL];

	if(GAMESETTING2(GAMESET2_NO_AUTODAMAGE_COMMAND) 
		|| !HAS_CONFIG2(ch, CONFIG2_AUTODAMAGE)){
		return "";
	}

	sprintf(result, " [%d]", damage_amount);

	return result;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
