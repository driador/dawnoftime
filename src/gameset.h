/**************************************************************************/
// gameset.h - see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: gameset.h - game settings system
 *  - All things are contained in here to make the code compile quicker.
 ***************************************************************************/
#ifndef GAMESET_H
#define GAMESET_H

extern const struct flag_type  game_settings_flags[];
extern const struct flag_type game_settings_flags2[];
extern const struct flag_type game_settings_flags3[];
extern const struct flag_type game_settings_flags4[];

enum gameset_value_cat{GSVC_GENERAL, GSVC_ROOM, GSVC_OBJECT, GSVC_MOB, GSVC_NONE};
/**************************************************************************/
struct gameset_value_type{
	gameset_value_cat category;
	int offset;	
	char *name;
	char *description;
};
#define GSBYTE_OFFSET(field) (int)(((char*) &(tgs.field)) \
		-((char*) &tgs)) 

#define GSINT( offset) *((int*)(((char*) game_settings)+(offset))) 

/**************************************************************************/
enum area_import_format_enum {AIF_STOCK, AIF_FORMAT2, AIF_FORMAT3 };

/**************************************************************************/
// game settings checking macros
#define GAMESETTING(flag)  (IS_SET(game_settings->flags, flag))
#define GAMESETTING2(flag) (IS_SET(game_settings->flags2, flag))
#define GAMESETTING3(flag) (IS_SET(game_settings->flags3, flag))
#define GAMESETTING4(flag) (IS_SET(game_settings->flags4, flag))
#define GAMESETTING5(flag) (IS_SET(game_settings->flags5, flag))

#define GAMESETTINGMF1(flag) (IS_SET(game_settings->flagsmf1, flag)) // from Mydrian Fields (the Dawn1.69s development mud)

#define GAMESETTINGS_FILE   "gameset.txt"   // Settings for the game
#define AREA_IMPORT_FORMAT(value)  (game_settings->area_import_format==value)
#define AREA_IMPORT_FLAG(flag)  (IS_SET(game_settings->area_import_flags, flag))
/**************************************************************************/
struct game_settings_type{
	char	*gamename;
	char	*irclogs_dir;	
	char	*msp_url;
	char	*style_sheet;
	char	*login_prompt;	
	char	*default_prompt;
	char	*password_creation;
	char	*password_player_connect;
	char	*listen_on;	
	char	*default_who_format;		// text version used to load/save
	char	*realm_name;
	char	*mud_default_battlelag_text;

	// config section - 
	int		config_create_coredump_at_end_of_nasty_signal_handler;

	int		default_who_format_index;	// index value used within the game
	int		unique_id;

	char	*help_header_bar;	// bar shown as help header 
	char	*help_footer_bar;	// bar shown as help footer
	char	*help_prevnext_footer_bar;	// bar shown as help footer after a [PREV][NEXT]
	
	char	*no_resolve_ip_text;// ip address to not bother looking up because

	area_import_format_enum area_import_format; // semi generically support importing 
												// different formatted areas
	long	area_import_flags;
	unsigned int max_string;
	long	flags;
	long	flags2;
	long	flags3;
	long	flags4;
	long	flags5;
	long	flagsmf1;

	int		roll_min_total; // >=0, <=500
	int		roll_max_total; // <=600, <=1000

	int		alarm_boot_db_abort_threshold;
	int		alarm_running_abort_threshold;
	int		alarm_running_dns_abort_threshold;
	int		alarm_frequency;

	int		default_newbie_security_on_olc_port;
	int		damage_scale_value; // in the range 4000 to 150, dawn is 2625

	int		automatic_offlineletgain_after_x_days;
	
	int		icyear_offset; 
	int		min_level_to_load_objects;
	int		max_hp_from_heal_spell;
	// vnum stuff
	int		roomvnum_newbie_recall; // default recall vnum for levels 1 to 5

	int		roomvnum_starttelnet;
	int		roomvnum_startirc;
	int		roomvnum_limbo;
	int		roomvnum_court_recall;
	int		roomvnum_ooc;
	int		roomvnum_good_recall;
	int		roomvnum_evil_recall;
	int		roomvnum_pkport_death_room;
	int		roomvnum_weapon_donate;
	int		roomvnum_armor_donate;
	int		roomvnum_misc_donate;
	int		roomvnum_newbieweapon_donate;
	int		roomvnum_newbiearmor_donate;
	int		roomvnum_newbiemisc_donate;

	int		roomvnum_jail;
	int		roomvnum_morgue;		// silly idea for some but requested by another mud

	int		global_scale_hitpoints_regen_rate;
	int		global_scale_mana_regen_rate;
	int		global_scale_movement_regen_rate;
	int		global_scale_mining_value_scaling_percentage;

	int		newbies_recall_to_roomvnum_newbie_recall_until_level;

	int		newbie_start_gold;
	int		newbie_start_silver;
	int		newbie_start_practice;
	int		newbie_start_train;

	int		global_xp_scale_value; // used for globally scaling the amount of xp rewarded
	int		minimum_note_notify_level;
 
	// auras around objects - used by format_obj_to_char()
	char	*aura_invis;
	char	*aura_evil;
	char	*aura_good;
	char	*aura_magical;
	char	*aura_glowing;
	char	*aura_chaos;
	char	*aura_hum;
	char	*aura_buried;
	char	*aura_holy;
	char	*aura_unholy;
	char	*aura_flaming;
	char	*aura_vampric;
	char	*aura_shocking;
	char	*aura_frost;

	// automated removal of laston players
	int		laston_remove_deleted_players_0_4;
	int		laston_remove_deleted_players_5_34;
	int		laston_remove_deleted_players_35_64;
	int		laston_remove_deleted_players_65_91;
	int		laston_remove_deleted_players_92;
	int		laston_level_for_immortal_version;

	int		olc_max_vnum; // maximum possible vnum (65000 by default)
	int		max_level_before_letgaining;
	int		max_rerolls;
	int		max_mob_level; // maximum possible mob level
	int		max_obj_level; // maximum possible object level
	int		xp_loss_for_fleeing;

	int		obj_vnum_world_map;
	int		obj_vnum_good_city_map;
	int		obj_vnum_evil_city_map;
	int		obj_vnum_divine_light;
	int		obj_vnum_rose;
	int		obj_vnum_raft;
	int		obj_vnum_newbie_guide;
	int		obj_vnum_rp_item;
	int		obj_vnum_spirit_hammer;
	int		obj_vnum_staff;
	int		obj_vnum_druidstaff;
	int		obj_vnum_totemstaff;
	int		obj_vnum_pit;
	int		obj_vnum_summon_justice;


	int		obj_vnum_mix_bad_low;
	int		obj_vnum_mix_bad_high;

	int		obj_vnum_outfit_mace;
	int		obj_vnum_outfit_dagger;
	int		obj_vnum_outfit_sword;
	int		obj_vnum_outfit_staff;
	int		obj_vnum_outfit_axe;
	int		obj_vnum_outfit_flail;
	int		obj_vnum_outfit_whip;
	int		obj_vnum_outfit_polearm;
	int		obj_vnum_outfit_sickle;
	int		obj_vnum_outfit_spear;
	int		obj_vnum_outfit_shield;
	int		obj_vnum_outfit_vest;
	int		obj_vnum_outfit_light;
	int		obj_vnum_outfit_sleeves;
	int		obj_vnum_outfit_cap;
	int		obj_vnum_outfit_gloves;
	int		obj_vnum_outfit_leggings;
	int		obj_vnum_outfit_boots;
	int		obj_vnum_outfit_belt;
	int		mob_vnum_to_run_object_mudprogs; // the mob vnum used to run object mudprogs

	int		mob_vnum_summon_guardian;
	int		mob_vnum_vyr_good;
	int		mob_vnum_vyr_bad;

	int		days_lockers_stored_for;
	int		webserver_default_remort;	

	// Note: The port_default value defaults to 4000 but can be over ridden
	// at bootup, e.g. 'dawn 9000' starts the mud with mud_port set to 9000.
	// ADDITIONAL PORT VALUES
	// The following port numbers are relative to the mud_port
	// e.g. irc_port == mud_port + irc_port_offset
	// setting any of these to 0 will mean the mud use that service
	int		port_default;

	int		thief_system_tagged_duration; // in minutes
	int		killer_system_tagged_duration; // in minutes
	int		killer_system_death_reduction_duration; // in minutes

	long	uneditable_flags;

	game_settings_type *next; // put here so GIO_LOAD/SAVE_LIST can be used	
};
/**************************************************************************/
extern game_settings_type *game_settings;
/**************************************************************************/
#define GAMESET_CHANGED						(A)	// gameset.txt needs saving
#define GAMESET_NOPERMDEATH					(B) // prevents karns going below 0
#define GAMESET_NOSHORT_DESCRIPTS			(C) // don't use short descriptions
#define GAMESET_HOLYNAME_FOR_ALL			(D) // everyone gets holyname
#define GAMESET_REMORT_SUPPORTED			(E) // remort system
#define GAMESET_REMORT_IN_SCORE				(F) // remort number shown in score
#define GAMESET_SUBLEVELS_ENABLED			(G) // sublevel system used
#define GAMESET_ROLE_ROLEMASTER_STATS		(H) 
#define GAMESET_USE_ROLEMASTER_MODIFIERS	(I) 
#define GAMESET_NO_BIAS_AGAINST_REROLLING	(J) 
#define GAMESET_NO_LETGAINING_REQUIRED		(K) 
#define GAMESET_BARDDONE					(L)
#define GAMESET_CLASS_CREATION_NO_STAR		(M)
#define GAMESET_MAX_ALIGN_RANGE22			(N)
#define GAMESET_ALWAYS_ALLOW_SETAGE			(O)
#define GAMESET_RESTRICTED_GROUPING			(P)
#define GAMESET_MORGUE_ENABLED				(Q)
#define GAMESET_MSP_CHECK_FILEEXIST			(R)
#define GAMESET_PEACEFUL_MUD				(S)
#define GAMESET_DONT_PERFORM_IDENT_LOOKUPS	(U)
#define GAMESET_PLAYERS_CANT_ACCESS_OTHERS_COLOURS  (V)
#define GAMESET_PERFORM_LOCAL_DNS_LOOKUPS	(W)
#define GAMESET_LOG_ALL_IP_CONNECTS			(X)
#define GAMESET_DISABLE_RPS_SYSTEM			(Y) // no rps is given when enabled
#define GAMESET_OUTFIT_DISABLED				(Z) 
#define GAMESET_SHOW_STAT_AVERAGES_IN_CREATION	(aa) 
#define GAMESET_NEWBIE_LOCKED				(bb) 
#define GAMESET_PLAYER_LOCKED				(cc) 

// Second set of flags
#define GAMESET2_NO_SECOND_SKILL_REQUIRED		(A) 
#define GAMESET2_DONT_AUTOOUTFIT_ON_NEWBIE_LOGIN (B) 
#define GAMESET2_AUTONOTE_RENAMES_TO_ADMIN		(C)
#define GAMESET2_AUTONOTE_IMMPKILLS_TO_ADMIN	(D)
#define GAMESET2_NO_WEBLOG						(1<<4) //(E)
#define GAMESET2_USE_LASTON_IMM_VALUE			(F) 
#define GAMESET2_CAN_ATTACK_SHOPKEEPERS			(G) 
#define GAMESET2_SHOPKEEPERS_CAN_BE_KILLED		(H) 
#define GAMESET2_HIDE_AREA_SHORTNAMES			(I) 
#define GAMESET2_DONT_SAVE_LASTEDITORS			(J)
#define GAMESET2_DONT_SAVE_MUDPROG_AUTHORS		(K)
#define GAMESET2_NEWBIES_GET_ALL_MAPS			(L)
#define GAMESET2_TELL_RESTRICTIONS				(M)
#define GAMESET2_BYPASSDUEL_REDUCES_KARNS		(N)
#define GAMESET2_NO_DUEL_REQUIRED				(O)
#define GAMESET2_VERBOSE_DATES_IN_LOGS			(P)
#define GAMESET2_DISABLE_VERBOSE_OLC_LOGGING	(Q)
#define GAMESET2_NOCHARM_HAS_NOAFFECT			(R)
#define GAMESET2_NO_MSG_ABOUT_OLD_MUDCLIENTS	(S)
#define GAMESET2_NO_AUTODAMAGE_COMMAND			(T)
#define GAMESET2_NO_AUTOLOGOFF_FOR_IMM			(U)
#define GAMESET2_WHOLIST_SORT_BY_LEVEL			(V)
#define GAMESET2_WHOLIST_IMMS_BEFORE_MORTS		(W)
#define GAMESET2_DONT_DISPLAY_CODEBASE_4_LOGIN  (X)
#define GAMESET2_DONT_DISPLAY_WHO_4_LOGIN		(Y)
// feel free as a developer to put additional 
// flags you want to use in here Z -> ee


// third set of flags
#define GAMESET3_GAIN_HP_WHEN_LEVELING			(A)
#define GAMESET3_GAIN_ONE_TRAIN_WHEN_LEVELING	(B)
#define GAMESET3_INFO_BROADCASTS_ENABLED		(C)
#define GAMESET3_WHO_TITLE_DISABLED				(D)
#define GAMESET3_MORTLASTON_REQUIRES_PART_OF_NAME	(E)
#define GAMESET3_MORTLASTON_REQUIRES_FULL_IMM_NAME	(F)
#define GAMESET3_LANGUAGE_NOT_SCRAMBLED			(G)
#define GAMESET3_LANGUAGE_NAME_NOT_IN_SAYS		(H)
#define GAMESET3_NO_DECREASING_MAX_KARN			(I)
#define GAMESET3_NO_CLANTALK					(J)
#define GAMESET3_EXPERIMENTAL_DAMAGE_SCALING	(K)
#define GAMESET3_DISPLAY_P_RESET_BUGS_ON_WIZNET	(M)
#define GAMESET3_AUTOAFK_DISABLED				(N)
#define GAMESET3_STORM_DAMAGE_MESSAGES			(O)
#define GAMESET3_SHOW_QP_IN_SCORE				(P)
#define GAMESET3_AUTOMATIC_LETHEROING			(Q)
#define GAMESET3_CAN_TEACH_SPELLS_WHILE_UNLEARNED	(R)
#define GAMESET3_DISABLE_EXTENDED_ASCII_CHARACTERS	(S)
#define GAMESET3_USE_DYNAMIC_WIZLIST				(T)
#define GAMESET3_LOCKERS_DISABLED					(U)
#define GAMESET3_HELP_HEADER_FOOTER_BAR_DISABLED	(V)
#define GAMESET3_CIRCLE_ALLOWED_WITHOUT_CLEAR_SHOT	(W)
#define GAMESET3_THIEF_SYSTEM_ENABLED				(X)
#define GAMESET3_KILLER_SYSTEM_ENABLED				(Y)
#define GAMESET3_ALWAYS_NO_NEGATIVE_HP_AT_AFFECTOFF (Z)
#define GAMESET3_MORTLASTON_REDUCED_TO2HOURS_ON_MORTS	(aa)
#define GAMESET3_MORTLASTON_REDUCED_LASTDAY_ON_IMMS		(bb)

// fourth set of flags
#define GAMESET4_ROOM_INVITES_DISABLED				(A)
#define GAMESET4_AUTOLOG_ALL_NEW_PLAYERS			(B)
#define GAMESET4_HELP_PREV_NEXT_SEPARATE_FROM_FOOTER (C)
#define GAMESET4_HELP_PREV_NEXT_ABOVE_SEE_ALSO		(D)
#define GAMESET4_PREVENT_STEALING_FROM_PLAYERS		(E)
#define GAMESET4_OOC_PREVENTION_MESSAGES			(F)
#define GAMESET4_DUEL_SYSTEM_DISABLED				(G)
#define GAMESET4_NO_KARN_IN_SCORE					(H)
#define GAMESET4_LEVELSORT_IMMORTALS_ON_WHO			(I)
#define GAMESET4_LEVELSORT_MORTALS_ON_WHO			(J)
#define GAMESET4_USE_GRANTGROUPS_FOR_IMM			(K)
#define GAMESET4_SHOW_IMMROLE_IN_SCORE_INSTEAD_OF_LEVEL	(L)
#define GAMESET4_3TIER_IMMRANKS_IN_WHO				(M)
#define GAMESET4_REDIRECT_CHANNEL_OOC_TO_CHAT		(N)
#define GAMESET4_REDIRECT_CHANNEL_CHAT_TO_OOC		(O)
#define GAMESET4_NO_STEALING_OUTSIDE_10_LEVELS		(P)
#define GAMESET4_NO_STEALING_FROM_NON_LETGAINED		(Q)
#define GAMESET4_NO_STEALING_FROM_NON_CLANNED		(R)
#define GAMESET4_MUST_BE_LETGAINED_TO_STEAL_FROM_PLAYERS	(S)
#define GAMESET4_MUST_BE_IN_CLAN_TO_STEAL_FROM_PLAYERS		(T)
#define GAMESET4_LOGINWHO_HIDES_IMMS_FROM_MORTS		(U)
#define GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS	(V)
#define GAMESET4_GAMEDEFAULT_COLOUR_IN_SOCIALS_ON	(W)

// fifth set of flags
#define GAMESET5_RACEINFO_DISABLED_IN_CREATION		(A)
#define GAMESET5_RACEINFO_DISABLED_IN_WEBSERVER		(B)
#define GAMESET5_RESTRICTED_CREATIONS_PER_HOUR		(C)
#define GAMESET5_AUTODAMAGE_DEFAULTS_OFF			(D)
#define GAMESET5_CREATION_ASK_IF_WANT_AUTOMAP_ON	(E)
#define GAMESET5_CREATION_DISABLE_CUSTOMIZATION		(F)
#define GAMESET5_DEDICATED_PKILL_STYLE_MUD			(G)
#define GAMESET5_DEDICATED_OLC_BUILDING_MUD			(H)
#define GAMESET5_DISABLE_TILDE_CONVERSION			(I)
#define GAMESET5_VERBOSE_INTRODUCTION_LOGGING		(J)
#define GAMESET5_NO_STEALING_FROM_FIGHTING_CHARACTERS (K)
#define GAMESET5_MUST_BE_ACTIVE_TO_BE_INVOLVED_IN_STEALING (L)
#define GAMESET5_HEROS_DONT_LOSE_XP_FOR_DYING		(M)
#define GAMESET5_ADDICT_XP_BONUS					(N)
#define GAMESET5_NOTES_TO_RACE_NAMES_SUPPORTED		(O)
#define GAMESET5_NOTES_TO_CLASS_NAMES_SUPPORTED		(P)
#define GAMESET5_HUNGER_AND_THIRST_CAUSES_DAMAGE	(Q)
#define GAMESET5_CLASSINFO_DISABLED_IN_CREATION		(R)
#define GAMESET5_CLASSINFO_DISABLED_IN_WEBSERVER	(S)
#define GAMESET5_MXP_EDIT_AT_BOTTOM_OF_HELPS		(T)
#define GAMESET5_HIDE_SOCKET_BINDINGS_FROM_GAMESET	(U)
#define GAMESET5_SECOND_SKILL_ALIAS_NOT_REQUIRED	(V)
#define GAMESET5_TRANSLATE_ROGUE_CLASS_TO_THIEF		(W) 

#define GAMESET5_PREVENT_PLAYERS_NAMING_AFTER_AREALIST_NAMES (X) 
#define GAMESET5_KEEP_NOTE_SPOOLS_INDEFINATELY		(Y) 
#define GAMESET5_ROOM_ECHOES_UNSEEN_WHILE_SLEEPING	(Z) 

/**************************************************************************/
// Myridian Fields code
#define GAMESETMF1_ENABLE_MF_EXPERIMENTAL_CREATION_SYSTEM	(A)
#define GAMESETMF1_SHEATH_ITEM_REQUIRED				(B)
#define GAMESETMF1_SHOW_NEWBIE_HINTS				(C)
#define GAMESETMF1_ENSURE_HIGH_STRENGTH_ROLLS 		(D)
#define GAMESETMF1_RPONLY_CHARACTER_AUDIT_EXEMPT	(E)
#define GAMESETMF1_SPELL_ONLEVEL_LEARN_EASIER_IF_ABOVE_SPELL_LEVEL (F) 
#define GAMESETMF1_PENDANTS_ENABLED					(G)

/**************************************************************************/
#define AREAIMPORTFLAG_IMPORTING_ENABLED			(A) // can't disable this
#define AREAIMPORTFLAG_IGNORE_HELPS_IN_AREAFILES	(B)
#define AREAIMPORTFLAG_DISCARD_UNFOUND_MUDPROGS		(C)
#define AREAIMPORTFLAG_READ_TO_EOL_ON_ACT_AFF_LINE	(D)

/**************************************************************************/
#define DEFAULT_LOGIN_PROMPT	"`WBy What Name Do You Wish To Be Called?`x "
#define LOGIN_PROMPT			(game_settings->login_prompt)
#define DEFAULT_PROMPT	"[`#`Y%e`g %t `R%h/%Hhp `B%m/%Mm `m%vmv `g%Xxp%d`^> "
//#define DEFAULT_PROMPT "[`#`Y%e`g %t `x%l `Y%g`yg `s%s`ws `R%h/`r%Hhp " 
//                                "`B%m/`b%Mm `M%v/`m%Vmv `G%X`gxp%d`^> "

/**************************************************************************/
#define GAMESETUNEDIT_MANUAL_COLOUR_CONVERT_PERFORMED	(A)
/**************************************************************************/

#endif // GAMESET_H
