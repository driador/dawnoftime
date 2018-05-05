/**************************************************************************/
// macros.h - most of the compiler macros are defined in here
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
#ifndef MACROS_H
#define MACROS_H

/**************************************************************************/
// disable minor printf optimisation macro used in versions of GCC >= 2.97
// this macro creates problems when you have member functions called 
// printf() - such as in the char_data class
#ifdef __GNUC__
#  ifdef printf
#    if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 97) 
#      undef printf
       // macro is defined normally as below
       //#ifdef __GNUC_PREREQ (2,97)
       //#  define printf(fmt, args...) fprintf (stdout, fmt, ##args)
#    endif
#  endif
#endif
/**************************************************************************/
// a assert pointer macro, used to make shorthand asserts 
// compatible with gcc 3.3.1+
#define assertp(pointer) assert( (pointer) !=NULL)

/**************************************************************************/
#define alloc_mem(sz) malloc((size_t)sz)
#define free_mem(mem, sz) free(mem)

/**************************************************************************/
/*
* Utility macros.
*/
#define IS_VALID(data)          ((data) != NULL && (data)->valid)
#define VALIDATE(data)          ((data)->valid = true)
#define INVALIDATE(data)        ((data)->valid = false)
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)         ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)				(tolower(c))
/* (c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))*/
#define UPPER(c)                ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_LOWER(c)             ((c) >= 'a' && (c) <= 'a' ? true: false)
#define IS_UPPER(c)             ((c) >= 'A' && (c) <= 'Z' ? true: false)

/* used by ident system */
#define replace_string( pstr, nstr ) \
{ free_string( (pstr) ); pstr=str_dup( (nstr) ); }
#define IS_NULLSTR(str)   ((str)==NULL || (str)[0]=='\0') 

#define IS_HEALER(mob)  (IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER))

#define IS_SAME_ROOM(ch1, ch2) ( IS_VALID(ch1) && IS_VALID(ch2) \
									&& (ch1)->in_room == (ch2)->in_room )

#define IS_WATER_SECTOR(sect)   (sect==SECT_WATER_SWIM \
								|| sect==SECT_WATER_NOSWIM \
								|| sect==SECT_UNDERWATER)

#define HAS_CLASSFLAG(ch, flag) (IS_SET(class_table[(ch)->clss].flags, flag))

/**************************************************************************/
/*
* OLC Macros
*/
#define HAS_AREA_SECURITY(ch, Area)									(\
			IS_SET(Area->area_flags, AREA_LOCKED)?					\
	HAS_SECURITY(ch,9):HAS_SECURITY(ch, UMAX(6,Area->security) )	)

#define IS_BUILDER_WHEN_NOT_RESTRICTED(ch, Area)		(\
			HAS_AREA_SECURITY(ch, Area)					\
		||	is_exact_name(ch->name, Area->builders)		)

#define IS_BUILDER_WHEN_RESTRICTED(ch, Area, type)		(\
			HAS_SECURITY(ch,9) 							\
	||is_exact_name("all",								\
			Area->build_restricts[type])				\
	||is_exact_name(ch->name,							\
			Area->build_restricts[type])				\
	||is_exact_name("all",								\
			Area->build_restricts[BUILDRESTRICT_ALL])	\
	||is_exact_name(ch->name,							\
			Area->build_restricts[BUILDRESTRICT_ALL])	)

#define IS_BUILDER(ch, Area, type)							(\
	IS_SET(Area->area_flags, AREA_LOCKED)?					\
		HAS_SECURITY(ch,9):									\
			HAS_SECURITY(ch,1)	&&							\
		(IS_SET(Area->area_flags, AREA_USE_BUILDRESTRICTS)	\
			?IS_BUILDER_WHEN_RESTRICTED(ch, Area, type)		\
			:IS_BUILDER_WHEN_NOT_RESTRICTED(ch, Area))		)

#define GET_SECURITY(ch)  ( IS_NPC((ch))? 0 : (ch)->pcdata->security )
#define HAS_SECURITY(cha, value)  (GET_SECURITY( (cha) ) >= (value) )
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

// macro used in reading in character files
#define IS_OLD_CHVER(ch)    ( ch->version<9 ||   \
	( ch->version==9 && ch->subversion<2))

/**************************************************************************/
/*
* Character macros.
*/
#define IS_OOC(ch)    ( (ch)->in_room!=NULL && IS_SET((ch)->in_room->room_flags, ROOM_OOC))
#define IS_OLC(ch)    ( (ch)->in_room && IS_SET((ch)->in_room->area->area_flags, AREA_OLCONLY))
#define IS_IC(ch)	( (ch)->in_room!=NULL \
	&& !IS_SET((ch)->in_room->room_flags, ROOM_OOC) \
&& !IS_SET((ch)->in_room->area->area_flags, AREA_OLCONLY) )
#define IS_OLCAREA(area)  (IS_SET( (area)->area_flags, AREA_OLCONLY) )
#define IS_NEWBIE(ch)    ((ch->level) <= (10) && !IS_LETGAINED(ch))
#define IS_NEWBIE_SUPPORT(ch)    (IS_SET((ch)->comm,COMM_NEWBIE_SUPPORT))
#define HAS_IMMTALK_NAME(ch) (ch && TRUE_CH(ch)->pcdata && !IS_NULLSTR(TRUE_CH(ch)->pcdata->immtalk_name))

#define IS_RP_SUPPORT(ch)	(HAS_CONFIG((ch), CONFIG_RP_SUPPORT ))
#define IS_SPELL(sn)		((sn >= FIRST_SPELL) && (sn <= LAST_SPELL))
#define IS_REALM(sn)		((sn >= gsn_abjuration) && (sn <= gsn_time ))
#define IS_SKILL(sn)		((sn >= gsn_claw) && (sn <= gsn_kobold ))

#define IS_SKILL_VALID_FOR_CLASS(sn, class_index)	\
					(skill_table[sn].rating[class_index] > 0	\
					&& skill_table[sn].skill_level[class_index] < LEVEL_IMMORTAL \
					&& skill_table[sn].skill_level[class_index] >= 0 )

#define IS_SKILLGROUP_SELECTABLE_FOR_CHAR(gn, ch)	\
				(ch && (skillgroup_table[gn].rating[ch->clss] >= 0  || IS_SET(skillgroup_table[gn].flags, SKILLGROUP_FREE_FOR_ALL)) \
					&& ch->remort>= skillgroup_table[gn].remort	\
					&& IS_SET(skillgroup_table[gn].flags, SKILLGROUP_CREATION_SELECTABLE))

#define IS_SWITCHED(ch)			((ch)->desc && (ch)->desc->original)
#define IS_CONTROLLED(victim)	(IS_NPC(victim) && (victim)->desc != NULL)
#define HAS_DESC(ch)			((ch)->desc)

#define SAFE_DESC_CHARACTER(ch)	(ch->desc->character? ch->desc->character: ch)

#define TRUE_CH(ch)				(ch->desc ? \
	(ch->desc->original ? ch->desc->original : SAFE_DESC_CHARACTER(ch)):ch) 
#define TCH(ch) TRUE_CH(ch)

#define TRUE_CH_PCDATA(ch)		(TRUE_CH(ch)?TRUE_CH(ch)->pcdata:NULL)

#define ACTIVE_CH(ch)			(ch->desc ? ch: \
	(ch->controlling? (ch->controlling->desc?ch->controlling:ch):ch))
#define ACH(ch) ACTIVE_CH(ch)

#define CH(descriptor)			((descriptor)->original ? \
	(descriptor)->original : (descriptor)->character)

#define IS_IN_EDITMODE(ch)	((ch)->desc && (ch)->desc->pString)
#define IS_IN_REDIT(ch)		(ch->desc && ch->desc->editor==ED_ROOM)

#define IS_NPC(ch)              ((ch)->pcdata==NULL)
#define IS_KEEPER(ch)			(IS_NPC((ch)) && (ch)->pIndexData && (ch)->pIndexData->pShop)

#define IS_UNSWITCHED_MOB(ch)	(IS_NPC(ch) && !IS_CONTROLLED(ch))

#define IS_IRCCON(connection)	((connection)->contype==CONTYPE_IRC)
#define IS_IRC(ch)				(!IS_SWITCHED(ch) && (ch)->desc && IS_IRCCON((ch)->desc) )

#define IS_IMMORTAL(ch)			(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_ICIMMORTAL(ch)		(!IS_NPC(ch) && ch->level >=LEVEL_IMMORTAL)
#define IS_HERO(ch)             (get_trust(ch) == LEVEL_HERO)
#define IS_TRUSTED(ch,level)    (get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, bit)    (IS_SET((ch)->affected_by, (bit)))
#define IS_AFFECTED2(ch, bit)   (IS_SET((ch)->affected_by2, (bit)))
#define IS_AFFECTED3(ch, bit)   (IS_SET((ch)->affected_by3, (bit)))
#define IS_ADMIN(ch)			(TRUE_CH(ch)->level>=ADMIN)

#define IS_NOBLE(ch)			(TRUE_CH(ch)->pcdata  && ((TRUE_CH(ch))->pcdata->diplomacy>0) )
#define IS_SILENT(ch)			(IS_SET((ch)->dyn, DYN_SILENTLY))
#define IS_QUESTER(ch)			(IS_SET(TRUE_CH(ch)->act, PLR_QUESTER))
#define IS_JAILED(ch)           (!IS_NPC(ch) && (ch)->level<LEVEL_IMMORTAL \
										&& (ch)->in_room_vnum()==ROOM_VNUM_JAIL)

#define IS_THIEF(ch)			(!IS_NPC(ch) && TRUE_CH_PCDATA(ch)->thief_until>current_time)
#define IS_KILLER(ch)           (!IS_NPC(ch) && TRUE_CH_PCDATA(ch)->killer_until>current_time)


#define IS_LINKDEAD(ch)			( !IS_NPC(ch) && !ch->desc \
&& (ch->controlling? !ch->controlling->desc:true))

#define IS_ACTIVE(ch)			(HAS_CONFIG(ch, CONFIG_ACTIVE) \
								 && !GAMESETTING(GAMESET_PEACEFUL_MUD) \
								 && !GAMESETTING2(GAMESET2_NO_DUEL_REQUIRED))

#define IS_PEACEFUL(ch)			(!IS_ACTIVE(ch))

#define USING_AUTOMAP(ch)		(HAS_CONFIG(ch, CONFIG_AUTOMAP))

// CONFIG MACROS
#define HAS_HOLYLIGHT(ch)		(TRUE_CH(ch)->pcdata \
									&& IS_SET(TRUE_CH(ch)->act,PLR_HOLYLIGHT))

#define HAS_AUTOSELF(ch)		(IS_SET(TRUE_CH(ch)->comm,COMM_AUTOSELF))

#define GET_AGE(ch)             ((int) (17 + ((ch)->played \
+ current_time - (ch)->logon )/72000))

#define GET_NEW_KARN_COUNTER(ch)  (IS_NPC(ch)?0:((ch->pcdata->karns*1000)+400)\
* ((ch)->level<40 ? 1 : (1 + ((ch)->level-40)/10)))
#define GET_NEW_LAY_COUNTER(ch)   (IS_NPC(ch)?0:8)
#define GET_MAX_KARN(ch)		(IS_NPC(ch)?0: \
						(IS_SET(ch->act, PLR_NOREDUCING_MAXKARN)  \
							|| GAMESETTING3(GAMESET3_NO_DECREASING_MAX_KARN)?	\
							5 : UMAX(0, 5-( (ch->pkkills-(ch->pkdefeats/2)) /5) ) ) )

#define IS_GOOD(ch)             (ch->alliance >= 2)
#define IS_EVIL(ch)             (ch->alliance <= -2)
#define IS_NEUTRAL(ch)          (!IS_GOOD(ch) && !IS_EVIL(ch))
#define IS_TEND_CHAOTIC(ch)     (ch->tendency < -1)    
#define IS_TEND_LAWFUL(ch)      (ch->tendency > 1)
#define IS_TEND_NEUTRAL(ch)     (!IS_TEND_CHAOTIC(ch) && !IS_TEND_LAWFUL(ch))

#define IS_AWAKE(ch)            (ch->position > POS_SLEEPING)
#define IS_ALIVE(ch)            (ch->position > POS_DEAD)
#define IS_MOUNTED(ch)          ((ch)->ridden_by)
#define IS_RIDING(ch)			((ch)->mounted_on)
#define IS_TETHERED(ch)			(IS_NPC((ch)) && ((ch)->tethered))

#define DAMTYPE(sn)				(skill_table[sn].damtype)

#define IS_SUBDUED(ch)          ((ch)->subdued)
#define GET_AC(ch,type)\
	((ch)->armor[type] + \
( IS_AWAKE(ch)	? - (ch)->modifiers[STAT_QU] : 0 ))
#define GET_HITROLL(ch) \
((ch)->hitroll+ (ch)->modifiers[STAT_ST]/10 )
#define GET_DAMROLL(ch) \
((ch)->damroll+ (ch)->modifiers[STAT_ST]/5 )

#define XP_PAST_LEVEL(ch) \
	(IS_NPC(ch)?0:(ch->exp- (ch->level *exp_per_level(ch, ch->pcdata->points))))

#define IS_LETGAINED(ch)    (IS_NPC(ch)?true:IS_SET(ch->act,PLR_CAN_ADVANCE))
#define GAMESETTING_LETGAINING_IS_REQUIRED (!GAMESETTING(GAMESET_NO_LETGAINING_REQUIRED))

#define IS_OUTSIDE(ch) \
( !IS_SET(ch->in_room->room_flags, ROOM_INDOORS) \
&& ch->in_room->sector_type != SECT_UNDERWATER \
&& ch->in_room->sector_type != SECT_CAVE )

#define WIZI_LOOKUP(ch) (IS_OOC(ch)?ch->owizi:IS_OLC(ch)?ch->olcwizi:ch->iwizi)
#define INVIS_LEVEL(ch)	(WIZI_LOOKUP(ch)?WIZI_LOOKUP(ch):ch->invis_level)

#define WAIT_STATE(ch, npulse)  ((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)    ((ch)->carry_weight + (ch)->silver/10 +  \
(ch)->gold * 2 / 5)

#define HAS_TRIGGER(mob,trig)		(IS_SET((mob)->pIndexData->mprog_flags,(trig)))
#define IS_RUNNING_TRIGGER(obj,trig) (IS_SET((mob)->running_trigger,(trig)))
#define SET_RUNNING_TRIGGER(obj,trig) (SET_BIT((mob)->running_trigger,(trig)))
#define REMOVE_RUNNING_TRIGGER(obj,trig) (REMOVE_BIT((mob)->running_trigger,(trig)))

#define HAS_TRIGGER2(mob,trig2)		(IS_SET((mob)->pIndexData->mprog2_flags,(trig2)))


#define HAS_OTRIGGER(obj,trig)		(IS_SET((obj)->pIndexData->oprog_flags,(trig)))
#define HAS_OTRIGGER2(obj,trig2)	(IS_SET((obj)->pIndexData->oprog2_flags,(trig2)))

#define IS_RUNNING_OTRIGGER(obj,trig) (IS_SET((obj)->running_otrigger,(trig)))
#define SET_RUNNING_OTRIGGER(obj,trig) (SET_BIT((obj)->running_otrigger,(trig)))
#define REMOVE_RUNNING_OTRIGGER(obj,trig) (REMOVE_BIT((obj)->running_otrigger,(trig)))


#define GET_SECONDS_PLAYED(ch) \
	(ch->played + (int) (current_time - ch->logon))

// config macros
#define HAS_CONFIG(ch, value)		(	IS_SET (TRUE_CH(ch)->config, value ))
#define SET_CONFIG(ch, value)		(	SET_BIT(TRUE_CH(ch)->config, value ))
#define REMOVE_CONFIG(ch, value)	(REMOVE_BIT(TRUE_CH(ch)->config, value ))
#define TOGGLE_CONFIG(ch, value)	(TOGGLE_BIT(TRUE_CH(ch)->config, value ))

// config2 macros
#define HAS_CONFIG2(ch, value)		(	IS_SET (TRUE_CH(ch)->config2, value ))
#define SET_CONFIG2(ch, value)		(	SET_BIT(TRUE_CH(ch)->config2, value ))
#define REMOVE_CONFIG2(ch, value)	(REMOVE_BIT(TRUE_CH(ch)->config2, value ))
#define TOGGLE_CONFIG2(ch, value)	(TOGGLE_BIT(TRUE_CH(ch)->config2, value ))

// pconfig macros - player configs, stored in pcdata
#define HAS_PCONFIG(ch, value)		(	IS_SET (TRUE_CH_PCDATA->config, value ))
#define SET_PCONFIG(ch, value)		(	SET_BIT(TRUE_CH_PCDATA->config, value ))
#define REMOVE_PCONFIG(ch, value)	(REMOVE_BIT(TRUE_CH_PCDATA->config, value ))
#define TOGGLE_PCONFIG(ch, value)	(TOGGLE_BIT(TRUE_CH_PCDATA->config, value ))

// channel macros
#define __CHAN_VALUE(ch)		(TRUE_CH(ch)->pcdata?TRUE_CH(ch)->pcdata->channeloff:__CHANNEL_OFF_CRASH_PROTECTOR_VARIABLE)
#define HAS_CHANNELOFF(ch, value)		(	 IS_SET(__CHAN_VALUE(ch), value ))
#define SET_CHANNELOFF(ch, value)		(	SET_BIT(__CHAN_VALUE(ch), value ))
#define REMOVE_CHANNELOFF(ch, value)	(REMOVE_BIT(__CHAN_VALUE(ch), value ))
#define TOGGLE_CHANNELOFF(ch, value)	(TOGGLE_BIT(__CHAN_VALUE(ch), value ))

#define HAS_MSP(ch)				(ch && TRUE_CH(ch)->pcdata && TRUE_CH(ch)->pcdata->msp_enabled)
#define HAS_MXP(ch)				(ch && TRUE_CH(ch)->pcdata && TRUE_CH(ch)->pcdata->mxp_enabled)
#define HAS_MXPDESC(d)			(d && d->mxp_enabled)
#define MXP_DETECTED_OR_ON(ch)		(	ch && TRUE_CH(ch)->pcdata \
										&& (TRUE_CH(ch)->pcdata->preference_mxp==PREF_ON \
											||  \
											(TRUE_CH(ch)->pcdata->preference_mxp==PREF_AUTOSENSE && \
											TRUE_CH(ch)->desc &&  \
											IS_SET(TRUE_CH(ch)->desc->flags, CONNECTFLAG_MXP_DETECTED)) ) )

#define IS_COURT(ch)			IS_SET((ch)->config, CONFIG_COURTMEMBER)
#define HAS_HOLYSPEECH(ch)		IS_SET((ch)->config, CONFIG_HOLYSPEECH)
#define CAN_HEAR_MSP(ch)		(	!IS_NPC(TRUE_CH(ch)) && \
									HAS_MSP(ch) && \
									!is_affected(ch, gsn_deafness) )


#define CAN_SEE_EXIT(ch, exit) (!IS_SET(exit->exit_info, EX_CLOSED) \
								|| HAS_HOLYLIGHT(ch) \
								|| IS_SET(TRUE_CH(ch)->act, PLR_HOLYWALK) \
								|| IS_SET(exit->exit_info, EX_OBVIOUS)) 

// capitalized person
#define CPERS(ch, looker)		(capitalize(PERS(ch,looker)))
/**************************************************************************/
/*
* Object macros.
*/
#define OBJECT_AFFECTS(obj)  \
	(obj->affected? obj->affected: (obj->no_affects?NULL:obj->pIndexData->affected))
// if an object is disenchanted (failed enchant etc), then it gets the 
//                                                     obj->no_affects flag set.
// this flag is only checked if obj->affects is NULL... if obj->affects is NULL
// and obj->no_affects is not set then the olc templates affects 
//                                          (obj->pIndexData->affected) are used.

#define CAN_WEAR(obj, part)     (IS_SET((obj)->wear_flags,  (part)))

#define DISALLOWED_OBJECT_FOR_CHAR(obj, ch) \
		(!IS_NPC(ch) && (obj)->pIndexData->class_allowances \
			&& !IS_SET((obj)->pIndexData->class_allowances, 1<<(ch)->clss) ) 

#define IS_OBJ_STAT(obj, stat)  (IS_SET((obj)->extra_flags, (stat)))
#define IS_OBJ2_STAT(obj, stat)  (IS_SET((obj)->extra2_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)        ((obj)->item_type == ITEM_CONTAINER ? \
								(obj)->value[4] : 100)

#define get_gold_weight(gold)   ((gold)*2/5)
#define get_silver_weight(silv) ((silv)/10)
#define trim_string(str)		(ltrim_string(rtrim_string(str)))
#define IS_TRAPPED(obj)         (IS_SET((obj)->extra2_flags, (OBJEXTRA2_TRAP)))
/**************************************************************************/
/**************************************************************************/
// reading in a a defined datatype macro
#if defined(KEY)
#undef KEY
#endif
#define KEY( literal, field, value )             \
             if ( !str_cmp( word, literal ) )    \
             {                                   \
                  field  = value;                \
                  fMatch = true;                 \
                  break;                         \
             }
/**************************************************************************/
// reading in a string from a file macro
#if defined(SKEY)
#undef SKEY
#endif
#define SKEY( string, field )                       \
					 if ( !str_cmp( word, string ) )     \
					 {                                   \
						  free_string( field );           \
						  field = fread_string( fp );     \
						  fMatch = true;                  \
						  break;                          \
										  }

/**************************************************************************/
#define channel_colour_disabled(ch, channel)	\
	(TRUE_CH(ch)?(TRUE_CH(ch)->pcdata?	IS_SET(	\
	TRUE_CH(ch)->pcdata->strip_colour_on_channel,channel):false):false)

/**************************************************************************/
// iSpell Related stuff below
#define Stringify(x) Str(x)
#define Str(x) #x

#ifdef WIN32
// compatiblity macros
#define vsnprintf(buf, len, fmt, args)	_vsnprintf(buf, len, fmt, args)
#define snprintf						_snprintf
#define popen(command, mode)			_popen(command,mode)
#define pclose(fp)						_pclose(fp)
#define getcwd(dir, size)				_getcwd(dir, size)
#define chdir(dir)						_chdir(dir)
#define mkdir(dir)						_mkdir(dir)
#endif

#endif // MACROS_H

