/**************************************************************************/
// gameedit.cpp - olc based game settings editor, Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "network.h"
#include "comm.h"
#include "include.h"
#include "olc.h"
#include "security.h"
#ifdef WIN32
#include "process.h"
#endif
/**************************************************************************/
// prototypes
game_settings_type *game_settings=NULL;
DECLARE_DO_FUN(	do_time				);
void do_save_gamesettings(char_data *ch, char *);
void msp_load_table();
char *get_compile_time (bool show_parent_codebase_version);

int who_format_lookup(char *argument);
const char *who_format_name(int index);
void who_show_formats(char_data *ch);

/**************************************************************************/
const struct flag_type area_import_format_types[] =
{
	{ "stock",		AIF_STOCK,		true},
	{ "format2",	AIF_FORMAT2,	true},
	{ "format3",	AIF_FORMAT3,	true},	
	{ NULL,0,0}
};

/**************************************************************************/
const struct flag_type game_settings_flags[] =
{
	{ "changed",					GAMESET_CHANGED,					true},
	{ "nopermdeath",				GAMESET_NOPERMDEATH,				true},
	{ "noshort_descripts",			GAMESET_NOSHORT_DESCRIPTS,			true},
	{ "holyname_for_all",			GAMESET_HOLYNAME_FOR_ALL,			true},
	{ "remort_supported",			GAMESET_REMORT_SUPPORTED,			true},
	{ "remort_in_score",			GAMESET_REMORT_IN_SCORE,			true},
	{ "sublevels_enabled",			GAMESET_SUBLEVELS_ENABLED,			true},
	{ "no_bias_against_rerolling",	GAMESET_NO_BIAS_AGAINST_REROLLING,	true},	
	{ "role_rolemaster_stats",		GAMESET_ROLE_ROLEMASTER_STATS,		true},
	{ "use_rolemaster_modifiers",	GAMESET_USE_ROLEMASTER_MODIFIERS,	true},
	{ "no_letgaining_required",		GAMESET_NO_LETGAINING_REQUIRED,		true},
	{ "bard_done",					GAMESET_BARDDONE,					true},
	{ "class_creation_no_star",		GAMESET_CLASS_CREATION_NO_STAR,		true},
	{ "max_align_range22",			GAMESET_MAX_ALIGN_RANGE22,			true},	
	{ "always_allow_setage",		GAMESET_ALWAYS_ALLOW_SETAGE,		true},		
	{ "restricted_grouping",		GAMESET_RESTRICTED_GROUPING,		true},
	{ "morgue_enabled",				GAMESET_MORGUE_ENABLED,				true},	
	{ "msp_check_fileexist",		GAMESET_MSP_CHECK_FILEEXIST,		true},		
	{ "peaceful_mud",				GAMESET_PEACEFUL_MUD,				true},		
	{ "dont_perform_ident_lookups",	GAMESET_DONT_PERFORM_IDENT_LOOKUPS,	true},		
	{ "players_cant_access_others_colours", GAMESET_PLAYERS_CANT_ACCESS_OTHERS_COLOURS, true},
	{ "perform_local_dns_lookups",	GAMESET_PERFORM_LOCAL_DNS_LOOKUPS,	true},	
	{ "log_all_ip_connects_to_admin/connects.txt",	GAMESET_LOG_ALL_IP_CONNECTS, true},
	{ "disable_rps_system",			GAMESET_DISABLE_RPS_SYSTEM,			true},
	{ "outfit_disabled",			GAMESET_OUTFIT_DISABLED,			true},
	{ "show_stat_averages_in_creation", GAMESET_SHOW_STAT_AVERAGES_IN_CREATION, true},
	{ "newbie_locked",				GAMESET_NEWBIE_LOCKED, true},
	{ "player_locked",				GAMESET_PLAYER_LOCKED, true},
	
	{ NULL,0,0}
};
/**************************************************************************/
const struct flag_type game_settings_flags2[] =
{
	{ "no_second_skill_required",	GAMESET2_NO_SECOND_SKILL_REQUIRED,	true},
	{ "dont_autooutfit_on_newbie_login",GAMESET2_DONT_AUTOOUTFIT_ON_NEWBIE_LOGIN,true},
	{ "autonote_renames_to_admin",	GAMESET2_AUTONOTE_RENAMES_TO_ADMIN, true},
	{ "autonote_immpkill_to_admin",	GAMESET2_AUTONOTE_IMMPKILLS_TO_ADMIN, true},
	{ "no_weblog",					GAMESET2_NO_WEBLOG,					true},
	{ "use_laston_imm_value",		GAMESET2_USE_LASTON_IMM_VALUE,		true},
	{ "can_attack_shopkeepers",		GAMESET2_CAN_ATTACK_SHOPKEEPERS,	true},
	{ "shopkeepers_can_be_killed",	GAMESET2_SHOPKEEPERS_CAN_BE_KILLED,	true},
	{ "hide_area_shortnames",		GAMESET2_HIDE_AREA_SHORTNAMES,		true},
	{ "dont_save_lasteditors",		GAMESET2_DONT_SAVE_LASTEDITORS,		false},
	{ "dont_save_mudprog_authors",	GAMESET2_DONT_SAVE_MUDPROG_AUTHORS,	false},
	{ "newbies_get_all_maps",		GAMESET2_NEWBIES_GET_ALL_MAPS,		true},
	{ "tell_restrictions",			GAMESET2_TELL_RESTRICTIONS,			true},
	{ "bypassduel_reduces_karns",	GAMESET2_BYPASSDUEL_REDUCES_KARNS,	true},
	{ "no_duel_required",			GAMESET2_NO_DUEL_REQUIRED,			true},
	{ "verbose_dates_in_logs",		GAMESET2_VERBOSE_DATES_IN_LOGS,		true},
	{ "disable_verbose_olc_logging",GAMESET2_DISABLE_VERBOSE_OLC_LOGGING,true},
	{ "nocharm_has_noaffect",		GAMESET2_NOCHARM_HAS_NOAFFECT,		true},
	{ "no_msg_about_old_mudclients",GAMESET2_NO_MSG_ABOUT_OLD_MUDCLIENTS,true},
	{ "no_autodamage_command",		GAMESET2_NO_AUTODAMAGE_COMMAND,		true},	
	{ "no_autologoff_for_imm",		GAMESET2_NO_AUTOLOGOFF_FOR_IMM,		true},	
	{ "wholist_sort_by_level",		GAMESET2_WHOLIST_SORT_BY_LEVEL,		true},
	{ "wholist_imms_before_morts",	GAMESET2_WHOLIST_IMMS_BEFORE_MORTS,	true},
	{ "dont_display_codebase_4_login",GAMESET2_DONT_DISPLAY_CODEBASE_4_LOGIN, true},
	{ "dont_display_who_4_login",	GAMESET2_DONT_DISPLAY_WHO_4_LOGIN,	true},

	{ NULL,0,0}
};
/**************************************************************************/
const struct flag_type game_settings_flags3[] =
{
	{ "gain_hp_when_leveling",			GAMESET3_GAIN_HP_WHEN_LEVELING,			true},
	{ "gain_one_train_when_leveling",	GAMESET3_GAIN_ONE_TRAIN_WHEN_LEVELING,	true},
	{ "info_broadcasts_enabled",		GAMESET3_INFO_BROADCASTS_ENABLED,		true},
	{ "who_title_disabled",				GAMESET3_WHO_TITLE_DISABLED,			true},
	{ "mortlaston_requires_part_of_name",	GAMESET3_MORTLASTON_REQUIRES_PART_OF_NAME,	true},
	{ "mortlaston_requires_full_imm_name",	GAMESET3_MORTLASTON_REQUIRES_FULL_IMM_NAME,	true},
	{ "mortlaston_reduced_to2hours_on_morts",	GAMESET3_MORTLASTON_REDUCED_TO2HOURS_ON_MORTS,true},
	{ "mortlaston_reduced_lastday_on_imms",	GAMESET3_MORTLASTON_REDUCED_LASTDAY_ON_IMMS,true},
	{ "language_not_scrambled",			GAMESET3_LANGUAGE_NOT_SCRAMBLED,	true},
	{ "language_name_not_in_says",		GAMESET3_LANGUAGE_NAME_NOT_IN_SAYS,	true},
	{ "no_decreasing_max_karn",			GAMESET3_NO_DECREASING_MAX_KARN,	true},
	{ "no_clantalk",					GAMESET3_NO_CLANTALK,				true},
	{ "experimental_damage_scaling",	GAMESET3_EXPERIMENTAL_DAMAGE_SCALING,false},	
	{ "display_p_reset_bugs_on_wiznet", GAMESET3_DISPLAY_P_RESET_BUGS_ON_WIZNET, true},
	{ "autoafk_disabled",				GAMESET3_AUTOAFK_DISABLED,			true},	
	{ "storm_damage_messages",			GAMESET3_STORM_DAMAGE_MESSAGES,		false},	
	{ "show_qp_in_score",				GAMESET3_SHOW_QP_IN_SCORE,			true},		
	{ "automatic_letheroing",			GAMESET3_AUTOMATIC_LETHEROING,		true},		
	{ "can_teach_spells_while_unlearned",GAMESET3_CAN_TEACH_SPELLS_WHILE_UNLEARNED,true},	
	{ "disable_extended_ascii_characters",GAMESET3_DISABLE_EXTENDED_ASCII_CHARACTERS,true},	
	{ "use_dynamic_wizlist",			GAMESET3_USE_DYNAMIC_WIZLIST,		true},		
	{ "lockers_disabled",				GAMESET3_LOCKERS_DISABLED,			true},
	{ "help_header_footer_bar_disabled",GAMESET3_HELP_HEADER_FOOTER_BAR_DISABLED,true},
	{ "circle_allowed_without_clear_shot",GAMESET3_CIRCLE_ALLOWED_WITHOUT_CLEAR_SHOT,true},	
	{ "thief_system_enabled",			GAMESET3_THIEF_SYSTEM_ENABLED,		true},	
	{ "killer_system_enabled",			GAMESET3_KILLER_SYSTEM_ENABLED,		true},	
	{ "always_no_negative_hp_at_affectoff",	GAMESET3_ALWAYS_NO_NEGATIVE_HP_AT_AFFECTOFF,true},	
	{ NULL,0,0}
};
/**************************************************************************/
const struct flag_type game_settings_flags4[] =
{
	{ "room_invites_disabled",		GAMESET4_ROOM_INVITES_DISABLED,		true},	
	{ "autolog_all_new_players",	GAMESET4_AUTOLOG_ALL_NEW_PLAYERS,	true},	
	{ "help_prev_next_separate_from_footer",	GAMESET4_HELP_PREV_NEXT_SEPARATE_FROM_FOOTER, true},	
	{ "help_prev_next_above_see_also",			GAMESET4_HELP_PREV_NEXT_ABOVE_SEE_ALSO,	true},				
	{ "prevent_stealing_from_players",	GAMESET4_PREVENT_STEALING_FROM_PLAYERS,	true},					
	{ "ooc_prevention_messages",	GAMESET4_OOC_PREVENTION_MESSAGES,	true},	
	{ "duel_system_disabled",		GAMESET4_DUEL_SYSTEM_DISABLED,	true},	
	{ "no_karn_in_score",			GAMESET4_NO_KARN_IN_SCORE,	true},	
	{ "levelsort_immortals_on_who", GAMESET4_LEVELSORT_IMMORTALS_ON_WHO,true},
	{ "levelsort_mortals_on_who",	GAMESET4_LEVELSORT_MORTALS_ON_WHO,	true},
	{ "grantgroups_for_imm",		GAMESET4_USE_GRANTGROUPS_FOR_IMM,	true},
	{ "show_immrole_in_score_instead_of_level",	GAMESET4_SHOW_IMMROLE_IN_SCORE_INSTEAD_OF_LEVEL,true},
	{ "3tier_immranks_in_who",		GAMESET4_3TIER_IMMRANKS_IN_WHO,	true},
	{ "redirect_channel_ooc_to_chat",GAMESET4_REDIRECT_CHANNEL_OOC_TO_CHAT,	true},
	{ "redirect_channel_chat_to_ooc",GAMESET4_REDIRECT_CHANNEL_CHAT_TO_OOC,	true},
	{ "no_stealing_outside_10_levels",GAMESET4_NO_STEALING_OUTSIDE_10_LEVELS,true},
	{ "no_stealing_from_non_letgained",GAMESET4_NO_STEALING_FROM_NON_LETGAINED,true},
	{ "no_stealing_from_non_clanned",GAMESET4_NO_STEALING_FROM_NON_CLANNED,true},
	{ "must_be_letgained_to_steal_from_players",GAMESET4_MUST_BE_LETGAINED_TO_STEAL_FROM_PLAYERS,true},
	{ "must_be_in_clan_to_steal_from_players",GAMESET4_MUST_BE_IN_CLAN_TO_STEAL_FROM_PLAYERS,true},
	{ "loginwho_hides_imms_from_morts",	GAMESET4_LOGINWHO_HIDES_IMMS_FROM_MORTS,true},
	{ "report_missing_outfit_items_to_players",	GAMESET4_REPORT_MISSING_OUTFIT_ITEMS_TO_PLAYERS,true},	
	{ "gamedefault_colour_in_socials_on",GAMESET4_GAMEDEFAULT_COLOUR_IN_SOCIALS_ON,true},
	
	{ NULL,0,0}
};

/**************************************************************************/
const struct flag_type game_settings_flags5[] =
{
	{ "raceinfo_disabled_in_creation", GAMESET5_RACEINFO_DISABLED_IN_CREATION, true},	
	{ "raceinfo_disabled_in_webserver", GAMESET5_RACEINFO_DISABLED_IN_WEBSERVER, true},	
	{ "classinfo_disabled_in_creation",	GAMESET5_CLASSINFO_DISABLED_IN_CREATION, true},
	{ "classinfo_disabled_in_webserver",GAMESET5_CLASSINFO_DISABLED_IN_WEBSERVER, true},
	{ "restricted_creations_per_hour", GAMESET5_RESTRICTED_CREATIONS_PER_HOUR, true},	
	{ "autodamage_defaults_off", GAMESET5_AUTODAMAGE_DEFAULTS_OFF, true},
	{ "creation_ask_if_want_automap_on", GAMESET5_CREATION_ASK_IF_WANT_AUTOMAP_ON, true},	
	{ "creation_disable_customization", GAMESET5_CREATION_DISABLE_CUSTOMIZATION, true},	
	{ "dedicated_pkill_style_mud",		GAMESET5_DEDICATED_PKILL_STYLE_MUD, false},	
	{ "dedicated_olc_building_mud",		GAMESET5_DEDICATED_OLC_BUILDING_MUD, true},	
	{ "disable_tilde_conversion",		GAMESET5_DISABLE_TILDE_CONVERSION, false},	
	{ "verbose_introduction_logging",	GAMESET5_VERBOSE_INTRODUCTION_LOGGING, false},	
	{ "no_stealing_from_fighting_characters",GAMESET5_NO_STEALING_FROM_FIGHTING_CHARACTERS, true},		
	{ "must_be_active_to_be_involved_in_stealing",GAMESET5_MUST_BE_ACTIVE_TO_BE_INVOLVED_IN_STEALING, true},			
	{ "heros_dont_lose_xp_for_dying",	GAMESET5_HEROS_DONT_LOSE_XP_FOR_DYING, true},			
	{ "addict_xp_bonus",				GAMESET5_ADDICT_XP_BONUS, true},			
	{ "notes_to_race_names_supported",	GAMESET5_NOTES_TO_RACE_NAMES_SUPPORTED, true},			
	{ "notes_to_class_names_supported",	GAMESET5_NOTES_TO_CLASS_NAMES_SUPPORTED, true},
	{ "hunger_and_thirst_causes_damage",GAMESET5_HUNGER_AND_THIRST_CAUSES_DAMAGE, true},
	{ "mxp_edit_at_bottom_of_helps",	GAMESET5_MXP_EDIT_AT_BOTTOM_OF_HELPS, true},
	{ "hide_socket_bindings_from_gameset",GAMESET5_HIDE_SOCKET_BINDINGS_FROM_GAMESET, true},	
	{ "second_skill_alias_not_required",	GAMESET5_SECOND_SKILL_ALIAS_NOT_REQUIRED, true},
	{ "translate_rogue_class_to_thief",	GAMESET5_TRANSLATE_ROGUE_CLASS_TO_THIEF, true},
	{ "prevent_players_naming_after_arealist_names",GAMESET5_PREVENT_PLAYERS_NAMING_AFTER_AREALIST_NAMES, true},
	{ "keep_note_spools_indefinately",	GAMESET5_KEEP_NOTE_SPOOLS_INDEFINATELY, true},
	{ "room_echoes_unseen_while_sleeping",	GAMESET5_ROOM_ECHOES_UNSEEN_WHILE_SLEEPING, true},
	{ NULL,0,0}
};


/**************************************************************************/
const struct flag_type game_settings_flagsmf1[] =
{
	{ "enable_mf_experimental_creation_system",	GAMESETMF1_ENABLE_MF_EXPERIMENTAL_CREATION_SYSTEM,	false},
	{ "sheath_item_required",		GAMESETMF1_SHEATH_ITEM_REQUIRED,		true},
	{ "show_newbie_hints",			GAMESETMF1_SHOW_NEWBIE_HINTS,			true},
	{ "ensure_high_strength_rolls",	GAMESETMF1_ENSURE_HIGH_STRENGTH_ROLLS,			true},
	{ "rponly_character_audit_exempt",	GAMESETMF1_RPONLY_CHARACTER_AUDIT_EXEMPT,	true},
	{ "spell_onlevel_learn_easier_if_above_spell_level",	GAMESETMF1_SPELL_ONLEVEL_LEARN_EASIER_IF_ABOVE_SPELL_LEVEL , true},
	{ "pendants_enabled",			GAMESETMF1_PENDANTS_ENABLED, true},
	
	
	{ NULL,0,0}
};

/**************************************************************************/
const struct flag_type area_import_flags[] =
{
	{ "importing_enabled",			AREAIMPORTFLAG_IMPORTING_ENABLED,		false},	
	{ "ignore_helps_in_areafiles",	AREAIMPORTFLAG_IGNORE_HELPS_IN_AREAFILES, true},
	{ "discard_unfound_mudprogs",	AREAIMPORTFLAG_DISCARD_UNFOUND_MUDPROGS, true},	
	{ "read_to_eol_on_act_aff_line",AREAIMPORTFLAG_READ_TO_EOL_ON_ACT_AFF_LINE, true},	
	{ NULL,0,0}
};

/**************************************************************************/
const struct flag_type game_settings_unedit[] =
{
	{ "manual_colour_convert_performed",GAMESETUNEDIT_MANUAL_COLOUR_CONVERT_PERFORMED,	false},	
	{ NULL,0,0}
};

/**************************************************************************/
GIO_START(game_settings_type)
GIO_STR(gamename)
GIO_INT(max_string)
GIO_READ_TO_END_OF_STRING("bind_ip_address")
GIO_STR(listen_on)
GIO_WFLAG(area_import_format, area_import_format_types)
GIO_WFLAG(area_import_flags, area_import_flags)
GIO_INT_WITH_DEFAULT_FLAGS(config_create_coredump_at_end_of_nasty_signal_handler, 0, GIOFLAG_ALWAYS_WRITE)

GIO_STR(irclogs_dir)
GIO_STR(login_prompt)
GIO_STR(default_prompt)
GIO_STR(msp_url)
GIO_STR(password_creation)
GIO_STR(password_player_connect)
GIO_STR(no_resolve_ip_text)
GIO_STR(default_who_format)
GIO_STR(help_header_bar)
GIO_STR(help_footer_bar)
GIO_STR(help_prevnext_footer_bar)
GIO_STR(realm_name)
GIO_STR(mud_default_battlelag_text)
GIO_INT(unique_id)

GIO_INT_WITH_DEFAULT(port_default,	 4000)

GIO_STR(style_sheet)

GIO_WFLAG(flags, game_settings_flags)
GIO_WFLAG(flags2, game_settings_flags2)
GIO_WFLAG(flags3, game_settings_flags3)
GIO_WFLAG(flags4, game_settings_flags4)
GIO_WFLAG(flags5, game_settings_flags5)

GIO_WFLAG(flagsmf1, game_settings_flagsmf1)

GIO_WFLAG(uneditable_flags, game_settings_unedit)

GIO_INT_WITH_DEFAULT(damage_scale_value,	2625)
GIO_INT_WITH_DEFAULT(roomvnum_limbo,		2)
GIO_INT_WITH_DEFAULT(roll_min_total,	0)
GIO_INT_WITH_DEFAULT(roll_max_total,	630)
GIO_INT_WITH_DEFAULT(icyear_offset,		780)

GIO_INT_WITH_DEFAULT(alarm_boot_db_abort_threshold, 120)
GIO_INT_WITH_DEFAULT(alarm_running_abort_threshold, 25)
GIO_INT_WITH_DEFAULT(alarm_running_dns_abort_threshold, 70)
GIO_INT_WITH_DEFAULT(alarm_frequency, 30)

GIO_INT_WITH_DEFAULT(automatic_offlineletgain_after_x_days, 3)
GIO_INT_WITH_DEFAULT(roomvnum_newbie_recall, 7359)
GIO_INT_WITH_DEFAULT(roomvnum_starttelnet,	7370)
GIO_INT_WITH_DEFAULT(roomvnum_startirc,		7370)
GIO_INT_WITH_DEFAULT(roomvnum_court_recall,	3000)
GIO_INT_WITH_DEFAULT(roomvnum_ooc,			30000)
GIO_INT_WITH_DEFAULT(roomvnum_good_recall,	3000)
GIO_INT_WITH_DEFAULT(roomvnum_evil_recall,	27003)
GIO_INT_WITH_DEFAULT(roomvnum_pkport_death_room, 3)

GIO_INT_WITH_DEFAULT(thief_system_tagged_duration, (24*60))
GIO_INT_WITH_DEFAULT(killer_system_tagged_duration, (60*24*60)) // 60 days
GIO_INT_WITH_DEFAULT(killer_system_death_reduction_duration, (20*24*60)) // 20 days

GIO_INT_WITH_DEFAULT(global_scale_hitpoints_regen_rate, 100)
GIO_INT_WITH_DEFAULT(global_scale_mana_regen_rate, 100)
GIO_INT_WITH_DEFAULT(global_scale_movement_regen_rate, 100)
GIO_INT_WITH_DEFAULT(global_scale_mining_value_scaling_percentage, 100)
GIO_INT_WITH_DEFAULT(newbies_recall_to_roomvnum_newbie_recall_until_level, 5)


// auras around objects - used by format_obj_to_char()
GIO_INT_WITH_DEFAULT(roomvnum_jail,	299)
GIO_STR(aura_invis)
GIO_STR(aura_evil)
GIO_STR(aura_good)
GIO_STR(aura_magical)
GIO_STR(aura_glowing)
GIO_STR(aura_chaos)
GIO_STR(aura_hum)
GIO_STR(aura_buried)
GIO_STR(aura_holy)
GIO_STR(aura_unholy)
GIO_STR(aura_flaming)
GIO_STR(aura_vampric)
GIO_STR(aura_shocking)
GIO_STR(aura_frost)

// how many days after a player deletes to remove them from laston automatically 
GIO_INT_WITH_DEFAULT(laston_remove_deleted_players_0_4,		2)
GIO_INT_WITH_DEFAULT(laston_remove_deleted_players_5_34,	7)
GIO_INT_WITH_DEFAULT(laston_remove_deleted_players_35_64,	21)
GIO_INT_WITH_DEFAULT(laston_remove_deleted_players_65_91,	60)
GIO_INT_WITH_DEFAULT(laston_remove_deleted_players_92,		500)
GIO_INT_WITH_DEFAULT(laston_level_for_immortal_version,		LEVEL_IMMORTAL)
GIO_INT_WITH_DEFAULT(days_lockers_stored_for,	60)
GIO_INT_WITH_DEFAULT(webserver_default_remort, 0)

GIO_INT_WITH_DEFAULT(min_level_to_load_objects,	95)
GIO_INT_WITH_DEFAULT(max_hp_from_heal_spell, 50)

GIO_INT_WITH_DEFAULT(max_level_before_letgaining, 25)
GIO_INT_WITH_DEFAULT(max_rerolls, 100)
GIO_INT_WITH_DEFAULT(max_mob_level, 300)
GIO_INT_WITH_DEFAULT(max_obj_level, 300)

GIO_INT_WITH_DEFAULT(obj_vnum_world_map,		26)
GIO_INT_WITH_DEFAULT(obj_vnum_good_city_map,	27)
GIO_INT_WITH_DEFAULT(obj_vnum_evil_city_map,	28)
GIO_INT_WITH_DEFAULT(obj_vnum_divine_light,		29)
GIO_INT_WITH_DEFAULT(obj_vnum_rose,				30)
GIO_INT_WITH_DEFAULT(obj_vnum_raft,				33)
GIO_INT_WITH_DEFAULT(obj_vnum_newbie_guide,		34)
GIO_INT_WITH_DEFAULT(obj_vnum_rp_item,			36)
GIO_INT_WITH_DEFAULT(obj_vnum_spirit_hammer,	37)
GIO_INT_WITH_DEFAULT(obj_vnum_staff,			38)
GIO_INT_WITH_DEFAULT(obj_vnum_druidstaff,		60)
GIO_INT_WITH_DEFAULT(obj_vnum_totemstaff,		61)
GIO_INT_WITH_DEFAULT(obj_vnum_summon_justice,	81)
GIO_INT_WITH_DEFAULT(obj_vnum_pit,				420)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_mace,		400)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_dagger,	401)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_sword,		402)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_staff,		403)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_axe,		404)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_flail,		405)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_whip,		406)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_polearm,	407)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_sickle,	408)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_vest,		409)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_shield,	410)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_light,		411)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_sleeves,	412)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_cap,		413)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_gloves,	414)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_leggings,	415)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_boots,		416)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_belt,		417)
GIO_INT_WITH_DEFAULT(obj_vnum_outfit_spear,		418)

GIO_INT_WITH_DEFAULT(obj_vnum_mix_bad_low,		445)
GIO_INT_WITH_DEFAULT(obj_vnum_mix_bad_high,		449)

GIO_INT_WITH_DEFAULT(mob_vnum_to_run_object_mudprogs,	444)

GIO_INT_WITH_DEFAULT(mob_vnum_summon_guardian,	80)
GIO_INT_WITH_DEFAULT(mob_vnum_vyr_good,			106)
GIO_INT_WITH_DEFAULT(mob_vnum_vyr_bad,			107)

GIO_INT_WITH_DEFAULT(roomvnum_morgue,			3120) // mekali morgue
GIO_INT_WITH_DEFAULT(roomvnum_weapon_donate,	3068) // mekali auction hall
GIO_INT_WITH_DEFAULT(roomvnum_armor_donate,		3068) // mekali auction hall
GIO_INT_WITH_DEFAULT(roomvnum_misc_donate,		3068) // mekali auction hall
GIO_INT_WITH_DEFAULT(roomvnum_newbieweapon_donate, 3068) // mekali auction hall
GIO_INT_WITH_DEFAULT(roomvnum_newbiearmor_donate, 3068) // mekali auction hall
GIO_INT_WITH_DEFAULT(roomvnum_newbiemisc_donate, 3068) // mekali auction hall

// newbies start values - Daos, Oct03
GIO_INT_WITH_DEFAULT(newbie_start_gold,			0)
GIO_INT_WITH_DEFAULT(newbie_start_silver,		1000)
GIO_INT_WITH_DEFAULT(newbie_start_practice,		10)
GIO_INT_WITH_DEFAULT(newbie_start_train,		5)

GIO_INT_WITH_DEFAULT(global_xp_scale_value,		100) // Kal, Oct03
GIO_INT_WITH_DEFAULT(minimum_note_notify_level, 2) // Kal, Oct03

GIO_INT_WITH_DEFAULT(olc_max_vnum,	65000)
GIO_INT_WITH_DEFAULT(default_newbie_security_on_olc_port, 2)
GIO_INT_WITH_DEFAULT(xp_loss_for_fleeing, 10)

GIO_FINISH_STRDUP_EMPTY

// GSBYTE_OFFSET is used to find out how many bytes from the start of  
// the game_settings_type structure the requested field is located.
game_settings_type tgs;

// this table should be extended to support 'types' of vnums/values... e.g. rooms, objects, mobs etc
gameset_value_type gameset_value[]={
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_newbie_recall),	"newbie_recall", "Newbies recall to this room"},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_limbo),			"limbo", "When people idle too long, they goto this room"},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_starttelnet),		"starttelnet", "Where new players who connect with telnet start"},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_startirc),			"startirc", "Where new players who connect with irc start"},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_court_recall),		"court_recall", "court members recall here."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_ooc),				"ooc", "Goooc takes players to this room."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_good_recall),		"good_recall", "'Good' races used to recall here - now set in pcedit per race."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_evil_recall),		"evil_recall", "'Evil' races used to recall here - now set in pcedit per race."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_pkport_death_room),"pkport_death_room", "Room players go if pkilled on the pkport."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_morgue),			"morgue", "Where player corpses go if they die (if the morgue game flag is enabled)"},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_weapon_donate),		"weapon_donate", "Room where donated weapons goto..."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_armor_donate),		"armor_donate", "Room where donated armor goes to..."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_misc_donate),		"misc_donate", "Room where miscelaneous donated items goto..."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_newbieweapon_donate),	"newbieweapon_donate", "Room where donated Newbie (Mudschool) weapons goto..."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_newbiearmor_donate),	"newbiearmor_donate", "Room where donated Newbie (Mudschool) armor goes to..."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_newbiemisc_donate),		"newbiemisc_donate", "Room where donated Newbie (Mudschool) miscelaneous items goto..."},
	{ GSVC_ROOM, GSBYTE_OFFSET(roomvnum_jail),				"jail", "The Jail - Room where players can't delete - the punishment room for bad players, flag the room norecall, nochannels and ooc.)"},

	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_world_map), 		"world_map", "obj_vnum_world_map"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_good_city_map), 	"good_city_map", "obj_vnum_good_city_map"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_evil_city_map), 	"evil_city_map", "obj_vnum_evil_city_map"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_divine_light), 	"divine_light", "obj_vnum_divine_light"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_rose), 			"rose", "obj_vnum_rose"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_raft), 			"raft", "obj_vnum_raft"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_newbie_guide), 	"newbie_guide", "obj_vnum_newbie_guide"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_rp_item), 		"rp_item", "obj_vnum_rp_item"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_spirit_hammer), 	"spirit_hammer", "obj_vnum_spirit_hammer"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_staff), 			"staff", "obj_vnum_staff"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_druidstaff), 		"druidstaff", "obj_vnum_druidstaff"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_totemstaff), 		"totemstaff", "obj_vnum_totemstaff"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_summon_justice), 	"summon_justice",	"weapon returned when summon justice succeeds"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_pit), 			"pit", "obj_vnum_pit"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_mace), 	"outfit_mace",		"obj_vnum_outfit_mace"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_dagger), 	"outfit_dagger",	"obj_vnum_outfit_dagger"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_sword), 	"outfit_sword",		"obj_vnum_outfit_sword"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_staff), 	"outfit_staff",		"obj_vnum_outfit_staff"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_axe), 		"outfit_axe",		"obj_vnum_outfit_axe"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_flail), 	"outfit_flail",		"obj_vnum_outfit_flail"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_whip), 	"outfit_whip",		"obj_vnum_outfit_whip"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_polearm), 	"outfit_polearm",	"obj_vnum_outfit_polearm"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_sickle), 	"outfit_sickle",	"obj_vnum_outfit_sickle"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_spear), 	"outfit_spear",		"obj_vnum_outfit_spear"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_vest), 	"outfit_vest",		"obj_vnum_outfit_vest"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_shield), 	"outfit_shield",	"obj_vnum_outfit_shield"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_light), 	"outfit_light",		"obj_vnum_outfit_light"},

	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_sleeves), 	"outfit_sleeves",	"obj_vnum_outfit_sleeves"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_cap), 		"outfit_cap",		"obj_vnum_outfit_cap"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_gloves), 	"outfit_gloves",	"obj_vnum_outfit_gloves"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_leggings), "outfit_leggings",	"obj_vnum_outfit_leggings"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_boots), 	"outfit_boots",		"obj_vnum_outfit_boots"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_outfit_belt), 	"outfit_belt",		"obj_vnum_outfit_belt"},

	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_mix_bad_low), 	"mix_bad_low_vnum",	"when a mix fails, an object is provided between"},
	{ GSVC_OBJECT, GSBYTE_OFFSET(obj_vnum_mix_bad_high), 	"mix_bad_high_vnum","the low and high values set here.."},
	
	{ GSVC_MOB, GSBYTE_OFFSET(mob_vnum_to_run_object_mudprogs), "mob_vnum_to_run_object_mudprogs",	"mob_vnum_to_run_object_mudprogs"},
	{ GSVC_MOB, GSBYTE_OFFSET(mob_vnum_summon_guardian), 	"mob_vnum_summon_guardian",	"mob_vnum_summon_guardian"},
	{ GSVC_MOB, GSBYTE_OFFSET(mob_vnum_vyr_good), 			"mob_vnum_vyr_good",		"mob_vnum_vyr_good"},
	{ GSVC_MOB, GSBYTE_OFFSET(mob_vnum_vyr_bad), 			"mob_vnum_vyr_bad",			"mob_vnum_vyr_bad"},

	{ GSVC_GENERAL, GSBYTE_OFFSET(laston_remove_deleted_players_0_4),	"laston_delete_0_4",	
		"Days after a char delete wipe from laston on levels 0 to 4."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(laston_remove_deleted_players_5_34),	"laston_delete_5_34",	
		"Days for level 5 and 34."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(laston_remove_deleted_players_35_64),	"laston_delete_35_64",	
		"Days for level 35 and 64."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(laston_remove_deleted_players_65_91),	"laston_delete_65_91",	
		"Days for level 65 and 91."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(laston_remove_deleted_players_92),	"laston_delete_92",		
		"Days for an immortal."},

	{ GSVC_GENERAL, GSBYTE_OFFSET(global_xp_scale_value), "global_xp_scale_value", "global setting - the amount xp is scaled by (default 100)."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(minimum_note_notify_level), "minimum_note_notify_level", "minimum level you are when you receive notification of new notes."},
	
	
	{ GSVC_GENERAL, GSBYTE_OFFSET(laston_level_for_immortal_version),	"laston_level_for_immortal_version",		
		"Level which immortal version of laston becomes available (see also flag2 use_laston_imm_value)."},

	{ GSVC_GENERAL, GSBYTE_OFFSET(days_lockers_stored_for),	"days_lockers_stored_for",		
		"Days that an unused locker will store objects for."},

	{ GSVC_GENERAL, GSBYTE_OFFSET(webserver_default_remort),	"webserver_default_remort",		
		"Default remort set on access through the webserver."},

	{ GSVC_GENERAL, GSBYTE_OFFSET(olc_max_vnum), "max_vnum", "Largest possible vnum, default to 65000."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(min_level_to_load_objects), "min_level_to_load_objects", "Minimum level an imm must be to load objects (default 95)."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(max_hp_from_heal_spell), "max_hp_from_heal_spell", "Maximum amount of HP gained from the heal spell (default 50)."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(max_level_before_letgaining), "max_level_before_letgaining", "Highest level you can get without being letgained."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(max_rerolls), "max_rerolls", "Maximum number of rerolls before being disconnected."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(automatic_offlineletgain_after_x_days), "automatic_offlineletgain_after_x_days", "How long after requesting an offline letgain before the mud will automatically accept it."},

	{ GSVC_GENERAL, GSBYTE_OFFSET(global_scale_hitpoints_regen_rate), "global_scale_hitpoints_regen_rate", "Globally scales the hitpoints regen rate for all rooms, 100% is considered normal."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(global_scale_mana_regen_rate), "global_scale_mana_regen_rate", "Globally scales the mana regen rate for all rooms, 100% is considered normal."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(global_scale_movement_regen_rate), "global_scale_movement_regen_rate", "Globally scales the movement regen rate for all rooms, 100% is considered normal."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(global_scale_mining_value_scaling_percentage), "global_scale_mining_value_scaling_percentage", "Globally scales the value of mined ore."},

	{ GSVC_GENERAL, GSBYTE_OFFSET(newbies_recall_to_roomvnum_newbie_recall_until_level), "newbies_recall_to_roomvnum_newbie_recall_until_level", "By default this is set to 5."},
		

	{ GSVC_GENERAL, GSBYTE_OFFSET(port_default), "port_default", "Default port mud will startup using when the port number is not specified on the command line (default 4000)."},

	{ GSVC_GENERAL, GSBYTE_OFFSET(newbie_start_gold),		"newbie_gold",		"The amount of gold a new player starts with."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(newbie_start_silver),		"newbie_silver",	"The amount of silver a new player starts with."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(newbie_start_practice),	"newbie_practice",	"The amount of practices a new player starts with."},
	{ GSVC_GENERAL, GSBYTE_OFFSET(newbie_start_train),		"newbie_train",		"The amount of trains a new player starts with."},
	
	{ GSVC_GENERAL, GSBYTE_OFFSET(xp_loss_for_fleeing), "xp_loss_for_fleeing", "xp_loss_for_fleeing (default 10)."},
	

	{ GSVC_NONE, 0, NULL, NULL}
};

/**************************************************************************/
void init_string_space();
extern char *string_space;


/**************************************************************************/
void gamesettings_set_default_style_sheet()
{
	if(IS_NULLSTR(game_settings->style_sheet)){
		game_settings->style_sheet=str_dup("\n"
			"/* note this style sheet can be edited using the gameedit stylesheet command */\n"
			"body, blockquote "BRACKET_OPEN"\n"
			"  background-color: #000000;\n"
			"  font-size: 10pt;\n"
			"  font-family: Arial, Helvetica, sans-serif;\n"
			"  color: white;\n"
			"  text-align: justify;\n"
			"}\n"
			"\n"
			"/* Links */\n"
			"a:link "BRACKET_OPEN"color:white ; font-weight:bold ; text-decoration:underline}\n"
			"a:active "BRACKET_OPEN"color:white ; font-weight:bold ; text-decoration:underline}\n"
			"a:visited "BRACKET_OPEN"color:white ; font-weight:bold ; text-decoration:underline}\n"
			"a:hover "BRACKET_OPEN"color:#AFAFAF ; text-decoration:none}\n"
			"\n");
	}
}

/**************************************************************************/
/**************************************************************************/
void do_load_gamesettings(char_data *, char *)
{
	bool reallocate_string_space=false;

	if(string_space==NULL){ 
		// allocate enough temporary storage for now so we can read in the game settings
		game_settings=new game_settings_type;
		game_settings->flags2=0;
		game_settings->max_string=32000;	// 32KB should easily be enough to read the game settings off
											// This is never reclaimed when the memory is reallocated 
											// but memory is cheap - Kal :)
		init_string_space();
		reallocate_string_space=true;
	};
    logf("Reading in game settings from %s...", GAMESETTINGS_FILE);
    GIOLOAD_LIST(game_settings, game_settings_type, GAMESETTINGS_FILE);
	// set a default if nofile was found
	if(!game_settings){
		log_notef("Gamesettings file '%s' was not found - creating empty file then reloading it to get default settings.", 
			GAMESETTINGS_FILE);

		// create an empty game settings file
		FILE *fpcreate=fopen(GAMESETTINGS_FILE, "w");
		fprintf(fpcreate,"END\nEOF~\n");
		fclose(fpcreate);
		// load it, to get the GIO defaults
		GIOLOAD_LIST(game_settings, game_settings_type, GAMESETTINGS_FILE);

		// set some additional defaults
		if(!game_settings){
			bugf("For some reason the gamesettings could not be loaded from the empty "
				"file created, check diskspace, and manually remove %s if it exists.", 
				GAMESETTINGS_FILE);
			log_notef("If this problem continues, create a gamesettings file '%s' "
				"which contains:\nEND\nEOF~\nThen restart the mud.", 
				GAMESETTINGS_FILE);
			exit_error( 1 , "do_load_gamesettings", "unexpected load error");
		}
		
		game_settings->next=NULL;
		game_settings->irclogs_dir = str_dup("");
		game_settings->login_prompt= str_dup("");
		game_settings->default_prompt= str_dup("");
		game_settings->roomvnum_limbo=2;
		game_settings->listen_on=str_dup(DEFAULT_LISTEN_ON_TEXT);

		game_settings->area_import_flags=AREAIMPORTFLAG_IMPORTING_ENABLED;
		game_settings->help_header_bar=str_dup("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
		game_settings->help_footer_bar=str_dup("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
		game_settings->help_prevnext_footer_bar=str_dup("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
		game_settings->uneditable_flags= GAMESETUNEDIT_MANUAL_COLOUR_CONVERT_PERFORMED;
		game_settings->realm_name=str_dup(DEFAULT_REALM_NAME);
		game_settings->mud_default_battlelag_text=str_dup(DEFAULT_BATTLELAG_TEXT);

		// some defaults for new muds
		game_settings->flags=GAMESET_NO_LETGAINING_REQUIRED | GAMESET_MORGUE_ENABLED;
		game_settings->flags2=GAMESET2_NO_AUTOLOGOFF_FOR_IMM;
		game_settings->flags3=    GAMESET3_GAIN_HP_WHEN_LEVELING 
								| GAMESET3_INFO_BROADCASTS_ENABLED
								| GAMESET3_AUTOMATIC_LETHEROING;

		game_settings->flags4= 0;
		game_settings->flags5= 0;


		MAX_STRING=DEFAULT_MAX_STRING;
		if(reallocate_string_space){
			init_string_space();
		}
		MUD_NAME=str_dup(DEFAULT_MUD_NAME);
		MSP_URL=str_dup(DEFAULT_MSP_URL);
		gamesettings_set_default_style_sheet(); // setup the default style sheet
		
		// resave a complete version of the file
		do_save_gamesettings(NULL, ""); // resave so default vnums can be manually edited
	}else{
		REMOVE_BIT(game_settings->flags, GAMESET_CHANGED);
		logf("Game settings for %s successfully loaded.", MUD_NAME);    
		if(MAX_STRING<100000){
			bugf("MAX_STRING in game settings less than 100000,\n"
				"**************: Increased MAX_STRING to compiled DEFAULT_MAX_STRING %d", DEFAULT_MAX_STRING);    
			MAX_STRING=DEFAULT_MAX_STRING;
		}
		if(reallocate_string_space){
			init_string_space();
		}
	}
	game_settings->next=NULL;

	// if individual fields are empty, apply some defaults
	if(IS_NULLSTR(game_settings->irclogs_dir)){
		game_settings->irclogs_dir=str_dup("../irc/logs/");
	}
	if(IS_NULLSTR(game_settings->login_prompt)){
		game_settings->login_prompt=str_dup(DEFAULT_LOGIN_PROMPT);
	}

	if(IS_NULLSTR(game_settings->default_prompt)){
		game_settings->default_prompt=str_dup(DEFAULT_PROMPT);
	}	

	if(IS_NULLSTR(game_settings->realm_name)){
		game_settings->realm_name=str_dup(DEFAULT_REALM_NAME);
	}

	if(IS_NULLSTR(game_settings->mud_default_battlelag_text)){
		game_settings->mud_default_battlelag_text=str_dup(DEFAULT_BATTLELAG_TEXT);
	}	
	
	SET_BIT(game_settings->area_import_flags,AREAIMPORTFLAG_IMPORTING_ENABLED);

	gamesettings_set_default_style_sheet(); // setup the default style sheet

	// default auras
	if(IS_NULLSTR(game_settings->aura_invis)){
		game_settings->aura_invis=str_dup("`#`s(Invis)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_evil)){
		game_settings->aura_evil=str_dup("`#`R(Red Aura)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_good)){
		game_settings->aura_good=str_dup("`#`B(Blue Aura)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_magical)){
		game_settings->aura_magical=str_dup("`#`G(Magical)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_glowing)){
		game_settings->aura_glowing=str_dup("`#`Y(Glowing)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_chaos)){
		game_settings->aura_chaos=str_dup("`#`?(Chaos)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_hum)){
		game_settings->aura_hum=str_dup("`#`g(Humming)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_buried)){
		game_settings->aura_buried=str_dup("`#`Y(Buried)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_holy)){
		game_settings->aura_holy=str_dup("`#`W(Holy)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_unholy)){
		game_settings->aura_unholy=str_dup("`#`y(UnHoly)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_flaming)){
		game_settings->aura_flaming=str_dup("`#`R(Flaming)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_vampric)){
		game_settings->aura_vampric=str_dup("`#`r(Wicked)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_shocking)){
		game_settings->aura_shocking=str_dup("`#`Y(Shocking)`& ");
	}
	if(IS_NULLSTR(game_settings->aura_frost)){
		game_settings->aura_frost=str_dup("`#`C(Freezing)`& ");
	}

	game_settings->damage_scale_value=URANGE(150, game_settings->damage_scale_value, 4000); // in the range 4000 to 150, dawn is 2625
	// the default who format
	game_settings->default_who_format_index=UMAX(1, who_format_lookup(game_settings->default_who_format));
	replace_string(game_settings->default_who_format, who_format_name(game_settings->default_who_format_index));

	if(IS_NULLSTR(game_settings->listen_on)){
		game_settings->listen_on=str_dup(DEFAULT_LISTEN_ON_TEXT);
	}

	if(!game_settings->unique_id){
		update_currenttime();
		game_settings->unique_id=(int)(current_time*getpid())&0x7fffffff;
		do_save_gamesettings(NULL, ""); // resave
	}
}
/**************************************************************************/
void do_save_gamesettings(char_data *ch, char *)
{
	REMOVE_BIT(game_settings->flags, GAMESET_CHANGED);
	GIOSAVE_LIST(game_settings, game_settings_type, GAMESETTINGS_FILE, true);
	ch->printlnf("Game settings saved to %s",GAMESETTINGS_FILE);
}
/**************************************************************************/
//	Entry Point for editing the game settings
void do_gameedit( char_data *ch, char *argument )
{
	if ( IS_NPC( ch )){
		ch->println("Players only.");
		return;
	}

	// do security checks
	if (!HAS_SECURITY(ch, GAMEEDIT_MINSECURITY))
	{
    	ch->printlnf("You must have an olc security %d or higher to use this command.",
			GAMEEDIT_MINSECURITY);
		return;
	}

	if ( !IS_TRUSTED(ch, GAMEEDIT_MINTRUST)) {
		ch->printlnf("You must have a trust of %d or above "
			"to use this command.", GAMEEDIT_MINTRUST);
		return;
	}

	if (IS_NULLSTR(argument) || str_cmp("confirm", argument)){	
		ch->println("syntax: gameedit confirm");
		ch->println("This command takes you into the game settings editor.");
		return;
	}
	
    ch->desc->pEdit	= (void*)&"nothing_for_now";
	ch->desc->editor = ED_GAME;
	ch->println("Entering game settings editor, type `=Cdone`x when you have finished editing.");
	return;
}
/**************************************************************************/
bool gameedit_gamename(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: `=Cgamename <string>`x");
		return false;
    }

    ch->printlnf("Gamename changed from '%s' to '%s'.", 
		game_settings->gamename, argument );

    replace_string( game_settings->gamename, argument );
    return true;
}
/**************************************************************************/
bool gameedit_mspurl(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: `=Cmspurl <base url>`x");
		ch->println("e.g. mspurl:  http://www.yourmudwebsite.com/msp/");
		return false;
    }

	if(str_prefix("http://", argument)){
		ch->println("The msp url must start with http://, and should have a trailing /");
		return false;
	}

    ch->printlnf("MSP URL changed from '%s' to '%s'.", 
		game_settings->msp_url, argument);

    replace_string( game_settings->msp_url, argument );
	msp_load_table();

	ch->wrapln("NOTE: Take note of the 'msp_check_fileexist' flag, when this "
		"is set, the mud will only use sounds which it can find on the "
		"file system.`1"
		"There are 3 options to get this setup in order of preference:`1"
		"1. Use a symbolic link to make the msp directory the "
		    "same directory as the URL points to... (requires the webserver "
			"to be the same server as the mud host and a little bit of "
			"knowledge about the operating system ln command)`1"
		"2. Either duplicate all the msp sound files into the msp dir "
		"(retaining the directory structure).`1"
		"3. Turn off the msp_check_fileexist flag... this means the mud "
		"may provide a url for a sound file to mud clients, when the sound "
		"file doesn't exist at the url.");
    return true;
}
/**************************************************************************/
bool gameedit_loginprompt(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: `=Cloginprompt <string>`x");
		return false;
    }

    ch->printlnf("Login prompt changed from '%s' to '%s'.", 
		game_settings->login_prompt, argument );

    replace_string( game_settings->login_prompt, argument );
    return true;
}
/**************************************************************************/
bool gameedit_defaultprompt(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: `=Cdefaultprompt <string>`x");
		return false;
    }

    ch->printlnf("Default prompt changed from '%s' to '%s'.", 
		game_settings->default_prompt, argument );

    replace_string( game_settings->default_prompt, argument );
    return true;
}
/**************************************************************************/
extern char *top_string;
extern char *string_space;
/**************************************************************************/
bool gameedit_is_valid_value_for_category(int value, gameset_value_cat category)
{
	switch(category){
		case GSVC_ROOM:
			if(!get_room_index(value)){
				return false;
			}
			break;

		case GSVC_OBJECT:
			if(!get_obj_index(value)){
				return false;
			}
			break;

		case GSVC_MOB:
			if(!get_mob_index(value)){
				return false;
			}
			break;
			
		default:
			break;
	}
	return true;
}
/**************************************************************************/
void gameedit_show_category(char_data*ch, gameset_value_cat category, char *title, char *searchfor)
{
	bool found=false; // true if something is found in a category
	bool value_valid;
	int value;
	
	for(int i=0; !IS_NULLSTR(gameset_value[i].name); i++){
		if(gameset_value[i].category!=category){
			continue;
		}
		// get our numeric value
		value=GSINT(gameset_value[i].offset);

		// check if value is valid for its category
		value_valid=gameedit_is_valid_value_for_category(value, category); 

		if(!found){ // only display the title on the first time for category
			ch->titlebar(title);
			found=true;
		}

		// support infix searching of showvalue output
		if(!IS_NULLSTR(searchfor)){
			if(str_infix(searchfor, gameset_value[i].name)
				&& str_infix(searchfor, gameset_value[i].description)
				&& str_infix(searchfor, title)				
				&& str_infix(searchfor, FORMATF("%d",value)))
			{
				continue;
			}
		}

		ch->printlnf("`=r%-25s %s%5d `S(%s)`x", 
			gameset_value[i].name,
			(value_valid?"`x":value==0?"`b":"`R"),
			value,
			gameset_value[i].description);
	}

}

/**************************************************************************/
bool gameedit_show(char_data *ch, char *)
{
	game_settings_type *gs=game_settings;

  	ch->printlnf("`=rGame Name: `=x%s", gs->gamename);
  	ch->printlnf("`=rLogin Prompt: `=x'%s'", gs->login_prompt);
	if(has_colour(gs->login_prompt)){
		ch->print("`SLogin Prompt: '");
		ch->printbw(gs->login_prompt);
		ch->println("'");
	}

  	ch->printlnf("`=rDefault Prompt: `=x'%s'", gs->default_prompt);
	if(has_colour(gs->default_prompt)){
		ch->print("`SDefault Prompt: '");
		ch->printbw(gs->default_prompt);
		ch->println("'");
	}

	ch->printlnf("`=rno_resolve_ip: '`=x%s`=r'", game_settings->no_resolve_ip_text);

	ch->printlnf("`=rcreatepass: '`=x%s`=r'  (password must be entered to create a new character)", 
		gs->password_creation);
	ch->printlnf("`=rconnectpass: '`=x%s`=r' (password must be entered by a player to connect)", 
		gs->password_player_connect);
	ch->println("`=rIf a password is '`=x-`=r' then no password is required.");

	mxp_display_olc_flags(ch, game_settings_flags,	gs->flags,	"flags",	"Flags:");
	mxp_display_olc_flags(ch, game_settings_flags2,	gs->flags2,	"flags2",	"Flags2:");
	mxp_display_olc_flags(ch, game_settings_flags3,	gs->flags3,	"flags3",	"Flags3:");
	mxp_display_olc_flags(ch, game_settings_flags4,	gs->flags4,	"flags4",	"Flags4:");
	mxp_display_olc_flags(ch, game_settings_flags5,	gs->flags5,	"flags5",	"Flags5:");

	mxp_display_olc_flags(ch, game_settings_flagsmf1,gs->flagsmf1,"flagsmf1",	"FlagsMF1:");

	ch->printlnf("`=rDefault WhoFormat: `=x%s", gs->default_who_format);

	mxp_display_olc_flags(ch, area_import_format_types,	gs->area_import_format,	"AreaImportFormat",	"AreaImportFormat:");

	ch->printlnf("`=rDefault_sec4olc: `=v%d   `=r(default 2)", 
		gs->default_newbie_security_on_olc_port);	

  	ch->printlnf("`=rRolling MinTotal: `x%d  (default 0, range 0->500)", 
		gs->roll_min_total);
	ch->printlnf("`=rRolling MaxTotal: `x%d  (default 630, range 600->1000)", 
		gs->roll_max_total);
	ch->printlnf("`=rICYear offset: `x%d  (default 780, range 0->9000)", 
		gs->icyear_offset);

	ch->printlnf("`=rHelp Header Bar:  '%s'`x", gs->help_header_bar);
	ch->printlnf("`=rHelp Footer Bar:  '%s'`x", gs->help_footer_bar);
	ch->printlnf("`=rHelp [Prev][Next] Footer Bar: '%s'`x", gs->help_prevnext_footer_bar);
	

	if(GAMESETTING3(GAMESET3_EXPERIMENTAL_DAMAGE_SCALING)){
		ch->printlnf("`=rDamage scale value: `S%d (this value shouldn't be changed unless you know what you are doing)", 
			gs->damage_scale_value);
	}

	if(IS_NULLSTR(gs->msp_url)){
		ch->println("`=rmsp url: `=Xnone");
	}else{
		ch->printlnf("`=rmsp url: `=x%s", gs->msp_url);
	}

	int free_string=(int)(&string_space[MAX_STRING - MSL]-top_string);
	ch->printlnf("Free stringspace =%d/%d (manually adjust max_string in gameset.txt file).", 
		free_string, gs->max_string);

	if(free_string>2500000){
		ch->printlnf("`YYou can safely reduce the max_string setting in gameset.txt to around %d and reduce wasted memory usage.`x", 
			((int)gs->max_string-free_string)+1250000);
	}


	ch->printlnf("`=XIMPORTANT:`=r To see the values/vnums type `=C%s`x",
		mxp_create_send(ch, "showvalues"));
	
	ch->printlnf("`S[`=C%s`S] `S[`=C%s`S]`x",
		mxp_create_send(ch, "showvalues"),
		mxp_create_send(ch, "stylesheet"));
	
	return false;
}

/**************************************************************************/
bool gameedit_showvalues(char_data *ch, char *argument)
{
	ch->titlebar("GAME VALUES, `#`RRed`& number = unfound/invalid for category");
	gameedit_show_category(ch, GSVC_GENERAL, "GENERAL VALUES", argument);
	gameedit_show_category(ch, GSVC_ROOM,	 "ROOMS", argument);
	gameedit_show_category(ch, GSVC_OBJECT,	 "OBJECTS", argument);	
	gameedit_show_category(ch, GSVC_MOB,	 "MOBILES", argument);

	if(IS_NULLSTR(argument)){
		ch->println("You can filter the above list in the same manner as wizhelp.");
	}
    return false;
}

/**************************************************************************/
// return the index where 
// return -1 if not found
int gameset_value_lookup(char *name)
{
	int i;
	//exact match first
	for(i=0; !IS_NULLSTR(gameset_value[i].name); i++){
		if(!str_cmp(gameset_value[i].name, name)){
			return i;
		}
	}

	// prefix match
	for(i=0; !IS_NULLSTR(gameset_value[i].name); i++){
		if(!str_prefix(name, gameset_value[i].name)){
			return i;
		}
	}
	return -1;
}
/**************************************************************************/
bool gameedit_value(char_data *ch, char *argument)
{
	char name[MIL];
	char vnum[MIL];
	argument=one_argument(argument, name);
	one_argument(argument, vnum);

    if ( IS_NULLSTR(vnum)) {
		ch->println( "Syntax: value <field> <new_value>");
		ch->wrapln( "This command is used to change any of the numbers "
			"in the values section of gameedit.");
		return false;
	}

	if(!is_number(vnum)){
		ch->println( "error: <new_value> must be a number");
		gameedit_value(ch, "");
		return false;
	}
	

	int index=gameset_value_lookup(name);

	if(index<0){
		ch->printlnf( "error: couldn't find field '%s' to adjust", name);
		gameedit_value(ch, "");
		return false;
	}

	int value = atoi( vnum);

    if ( value < 0 )
    {
        ch->println( "<new_value> must be zero or greater." );
		gameedit_value(ch, "");
        return false;
    }

	if ( !gameedit_is_valid_value_for_category(value, gameset_value[index].category)){
	 	ch->printlnf( "%d is not a valid value for %s.", value, gameset_value[index].name);
		gameedit_value(ch, "");
		return false;
	}


	ch->printlnf("%s from %d to %d.", 
		capitalize(gameset_value[index].name), 
		GSINT(gameset_value[index].offset),
		value);

	GSINT(gameset_value[index].offset)=value;
	return true;
}
/**************************************************************************/
bool gameedit_flags(char_data *ch, char *argument)
{
	game_settings_type *gs=game_settings;
	bool result=olc_generic_flag_toggle(ch, argument,
				"flags", "flags", game_settings_flags, &gs->flags);
    return result;
}
/**************************************************************************/
bool gameedit_flags2(char_data *ch, char *argument)
{
	game_settings_type *gs=game_settings;
	bool result=olc_generic_flag_toggle(ch, argument,
				"flags2", "flags2", game_settings_flags2, &gs->flags2);
    return result;
}
/**************************************************************************/
bool gameedit_flags3(char_data *ch, char *argument)
{
	game_settings_type *gs=game_settings;
	bool result=olc_generic_flag_toggle(ch, argument,
				"flags3", "flags3", game_settings_flags3, &gs->flags3);
    return result;
}
/**************************************************************************/
bool gameedit_flags4(char_data *ch, char *argument)
{
	game_settings_type *gs=game_settings;
	bool result=olc_generic_flag_toggle(ch, argument,
				"flags4", "flags4", game_settings_flags4, &gs->flags4);
    return result;
}
/**************************************************************************/
bool gameedit_flags5(char_data *ch, char *argument)
{
	game_settings_type *gs=game_settings;
	bool result=olc_generic_flag_toggle(ch, argument,
				"flags5", "flags5", game_settings_flags5, &gs->flags5);
    return result;
}


/**************************************************************************/
bool gameedit_flagsmf1(char_data *ch, char *argument)
{
	game_settings_type *gs=game_settings;
	bool result=olc_generic_flag_toggle(ch, argument,
				"flagsmf1", "flagsmf1", game_settings_flagsmf1, &gs->flagsmf1);
    return result;
}
/**************************************************************************/
bool gameedit_areaimportformat(char_data *ch, char *argument)
{
	game_settings_type *gs=game_settings;
    int value;
	
    if ( argument[0] != '\0' )
    {
		if ( ( value = flag_value( area_import_format_types, argument ) ) != NO_FLAG )
		{
			gs->area_import_format= (area_import_format_enum)value;
			ch->println( "Area import format changed.");
			return true;
		}
    }

	show_olc_options(ch, area_import_format_types, "AreaImportFormat", "AreaImportFormat", gs->area_import_format);
    return false;
}
/**************************************************************************/
bool gameedit_mintotal( char_data *ch, char *argument )
{
    if ( IS_NULLSTR(argument)) {
		ch->println( "Syntax: mintotal");
		ch->println( "This is the minimum total of values when stats are rolled, in the range 0 (default) to 500.");
		ch->wrapln( "This total value is approximately calculated basically by "
			"adding all potential stats, then for any potential stat over 80, "
			"adding an additional 10 to the total.");
		return false;
	}

	int value = atoi( argument );

	// value checks
	if(value<0){
		ch->println("Roll mintotal: value too low.");
		gameedit_mintotal( ch, "");
		return false;
	}
	if(value>500){
		ch->println("Roll mintotal: value too high.");
		gameedit_mintotal( ch, "");
		return false;
	}
	if(game_settings->roll_min_total==value){
		ch->printlnf("The roll mintotal is already set to %d!", value);
		return false;
	};

	// change the change
	ch->printlnf("Roll mintotal changed from %d to %d.", 
		game_settings->roll_min_total, value);
	game_settings->roll_min_total=value;
	return true;
}
/**************************************************************************/
bool gameedit_maxtotal( char_data *ch, char *argument )
{
    if ( IS_NULLSTR(argument)) {
		ch->println( "Syntax: maxtotal");
		ch->println( "This is the maximum total of values when stats are rolled, in the range 600to 1000 (dawn default=630).");
		ch->wrapln( "This total value is approximately calculated basically by "
			"adding all potential stats, then for any potential stat over 80, "
			"adding an additional 10 to the total.");
		return false;
	}

	int value = atoi( argument );

	// value checks
	if(value<600){
		ch->println("Roll maxtotal: value too low.");
		gameedit_maxtotal( ch, "");
		return false;
	}
	if(value>1000){
		ch->println("Roll maxtotal: value too high.");
		gameedit_maxtotal( ch, "");
		return false;
	}
	if(game_settings->roll_max_total==value){
		ch->printlnf("The roll maxtotal is already set to %d!", value);
		return false;
	};

	// change the change
	ch->printlnf("Roll maxtotal changed from %d to %d.", 
		game_settings->roll_max_total, value);
	game_settings->roll_max_total=value;
	return true;
}
/**************************************************************************/
bool gameedit_icyear_offset( char_data *ch, char *argument )
{
    if ( IS_NULLSTR(argument)) {
		ch->println( "Syntax: icyear_offset");
		ch->println( "This is offset of years added to the ic time.");
		return false;
	}

	int value = atoi( argument );

	// value checks
	if(value<0){
		ch->println("ICYear offset must be zero or higher.");
		gameedit_icyear_offset( ch, "");
		return false;
	}
	if(value>9000){
		ch->println("ICYear offset must be less than 9000.");
		gameedit_icyear_offset( ch, "");
		return false;
	}
	if(game_settings->icyear_offset==value){
		ch->printlnf("The IC Year offset is already set to %d!", value);
		do_time(ch,"");
		return false;
	};

	// change the change
	ch->printlnf("The IC Year offset changed from %d to %d.", 
		game_settings->icyear_offset, value);
	game_settings->icyear_offset=value;
	do_time(ch,"");
	return true;
}
/**************************************************************************/
bool gameedit_default_newbie_security_on_olc_port( char_data *ch, char *argument )
{
    if ( IS_NULLSTR(argument)) {
		ch->println( "Syntax: `=Cdefault_sec4olc <number>`x");
		ch->wrapln( "0<Number<9");
		return false;
	}

	int value = atoi( argument );

	// value checks
	if(value<0){
		ch->println("default_newbie_security_on_olc_port: value too low.");
		gameedit_default_newbie_security_on_olc_port( ch, "");
		return false;
	}
	if(value>9){
		ch->println("default_newbie_security_on_olc_port: value too high.");
		gameedit_default_newbie_security_on_olc_port( ch, "");
		return false;
	}
	if(game_settings->default_newbie_security_on_olc_port==value){
		ch->printlnf("The default_newbie_security_on_olc_port is already set to %d!", value);
		return false;
	};

	// make the change
	ch->printlnf("default_newbie_security_on_olc_port changed from %d to %d.", 
		game_settings->default_newbie_security_on_olc_port, value);
	game_settings->default_newbie_security_on_olc_port=value;
	return true;
}
/**************************************************************************/
// display to a player what the game settings are
void do_gamesettings( char_data *ch, char * )
{
	ch->titlebar("GAME SETTINGS");

	ch->printlnf("You gain %s%d train%s when leveling.",
		GAMESETTING3(GAMESET3_GAIN_HP_WHEN_LEVELING)?"hitpoints and ":"",
		GAMESETTING3(GAMESET3_GAIN_ONE_TRAIN_WHEN_LEVELING)?1:2,
		GAMESETTING3(GAMESET3_GAIN_ONE_TRAIN_WHEN_LEVELING)?"":"s");

	// letgaining
	ch->printf("Letgaining is %s",	GAMESETTING(GAMESET_NO_LETGAINING_REQUIRED)?"not required":"required");
	if(GAMESETTING(GAMESET_NO_LETGAINING_REQUIRED)){
		ch->println(".");
	}else{
		ch->printlnf(" before level %d", game_settings->max_level_before_letgaining);
	}

	ch->printlnf("Setage can be used %s.",	GAMESETTING(GAMESET_ALWAYS_ALLOW_SETAGE)?"as many times as you like":"once");
	ch->printlnf("Grouping is %s.",		GAMESETTING(GAMESET_RESTRICTED_GROUPING)?"restricted":"unrestricted");
	ch->printlnf("Short descriptions are %s.",	GAMESETTING(GAMESET_NOSHORT_DESCRIPTS)?"unused":"used");
	ch->printlnf("The rps system is %s.",	GAMESETTING(GAMESET_DISABLE_RPS_SYSTEM)?"disabled":"enabled");
	ch->printlnf("The autodamage command is currently %s.",	GAMESETTING2(GAMESET2_NO_AUTODAMAGE_COMMAND) ?"unavailable":"available");
	if(!GAMESETTING(GAMESET_NOSHORT_DESCRIPTS)){
		ch->printlnf("Player names are shown %s.",	GAMESETTING(GAMESET_HOLYNAME_FOR_ALL)?"always":"based on introduction");
	}

	if(GAMESETTING3(GAMESET3_LOCKERS_DISABLED)){
		ch->println("Lockers are not available.");
	}else{
		ch->println("Lockers are available at selected locations.");
	}

	ch->printlnf("Languages %s.",	GAMESETTING3(GAMESET3_LANGUAGE_NOT_SCRAMBLED)?"are always understood":"need to be known to understand");

	ch->printlnf("The morgue is %s.",	GAMESETTING(GAMESET_MORGUE_ENABLED)?"open for business":"closed");
	ch->printlnf("Remort is %s.",		GAMESETTING(GAMESET_REMORT_SUPPORTED)?"supported":"unsupported");
	ch->printlnf("Sublevels are %s.",	GAMESETTING(GAMESET_SUBLEVELS_ENABLED)?"enabled":"disabled");
	ch->printlnf("The second skill %s.",GAMESETTING2(GAMESET2_NO_SECOND_SKILL_REQUIRED)?"is free to all":"must be gained");
	ch->printlnf("Tells can be %s.",GAMESETTING2(GAMESET2_TELL_RESTRICTIONS)?"sent to and from immortals":"freely exchanged");
	
	if(GAMESETTING3(GAMESET3_CAN_TEACH_SPELLS_WHILE_UNLEARNED)){
		ch->println("You can teach a spell immediately after learning it yourself.");
	}else{
		ch->println("You need to be learned in a spell before you can teach it.");
	}

	if(GAMESETTING4(GAMESET4_PREVENT_STEALING_FROM_PLAYERS)){
		ch->println("Stealing from other players is prevented.");
	}

	if(GAMESETTING5(GAMESET5_CREATION_DISABLE_CUSTOMIZATION)){
		ch->println("There is basic player customization available in creation.");
	}else{
		ch->println("There is advanced player customization available in creation.");
	}
	
	ch->printlnf("Nocharm currently has %s affect.", GAMESETTING2(GAMESET2_NOCHARM_HAS_NOAFFECT)?"no":"full");

	ch->printlnf("Permdeath when you reach 0 karns and die is %s.",	GAMESETTING(GAMESET_NOPERMDEATH)?"disabled":"enabled");
	if(GAMESETTING2(GAMESET2_NO_DUEL_REQUIRED)){
		ch->println("Dueling is not required to initiate PK combat.");
	}else{
		ch->println("Dueling is required to initiate PK combat.");
		if(GAMESETTING2(GAMESET2_BYPASSDUEL_REDUCES_KARNS)){
			ch->println("The use of bypassduel can result in the loss of one your karns.");
		}else{
			ch->println("The use of bypassduel has no affect on your karns.");
		}
	}
	ch->println(GAMESETTING(GAMESET_PEACEFUL_MUD)?"Pkilling is prevented by the code on this mud.":"The code isn't currently configured to prevent pkilling.");

	if(GAMESETTING5(GAMESET5_NO_STEALING_FROM_FIGHTING_CHARACTERS)){
		ch->println("It is not possible to steal from fighting characters.");
	}

	if(GAMESETTING5(GAMESET5_MUST_BE_ACTIVE_TO_BE_INVOLVED_IN_STEALING)){
		ch->println("You must be active to be involved in stealing (thief or victim).");
	}
	
	if(GAMESETTING5(GAMESET5_HEROS_DONT_LOSE_XP_FOR_DYING)){
		ch->println("Heros don't lose experience when they die.");
	}

	if(GAMESETTING5(GAMESET5_ADDICT_XP_BONUS)){
		ch->println("The mud is configured to give 'addict' xp bonuses.");
		ch->println("(an xp bonus to the player who has been on the longest).");
	}

	if(GAMESETTING5(GAMESET5_HUNGER_AND_THIRST_CAUSES_DAMAGE)){
		ch->println("Hunger and thirst can cause damage.");
	}

    ch->printlnf("Accepting telnet and DawnFTP connections on port %d.", mainport);    

	ch->printlnf("This mud is based off Dawn %s - %s (http://www.dawnoftime.org/).", 
		DAWN_RELEASE_VERSION, DAWN_RELEASE_DATE);

	if(!GAMESETTING5(GAMESET5_HIDE_SOCKET_BINDINGS_FROM_GAMESET)){
		char *binfo=str_dup(netio_return_binded_sockets());
		binfo=string_replace_all(binfo, "][", "]`1  [");
		ch->println("Listening for connections on:");
		ch->printlnf("  %s", binfo);
		free_string(binfo);
	}

#ifdef NOCRYPT 
	// I think players should have the right to know since there really isnt 
	// any need to disable passwords since we have ey_crypt.cpp - Kal
	ch->println("Player passwords are not being encrypted when saved to pfiles.");
#endif

}
/**************************************************************************/
bool gameedit_createpass(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: `=Ccreatepass <string>`x");
		ch->println("Syntax: `=Ccreatepass -`x to clear a password.");
		return false;
    }

	if(str_cmp("-", argument)){
		ch->printlnf("The password required to create a new character changed from '%s' to '%s'.", 
			game_settings->password_creation, argument );
	}else{
		ch->println("The password required to create a new character cleared."); 
	}
    replace_string( game_settings->password_creation, argument );
    return true;
}
/**************************************************************************/
bool gameedit_connectpass(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println("Syntax: `=Cconnectpass <string>`x");
		ch->println("Syntax: `=Cconnectpass -`x to clear a password.");
		return false;
    }

	if(str_cmp("-", argument)){
		ch->printlnf("The password required by players to connect changed from '%s' to '%s'.", 
			game_settings->password_player_connect, argument );
	}else{
		ch->println("The password required by players to connect cleared."); 
	}
    replace_string( game_settings->password_player_connect, argument );
    return true;
}
/**************************************************************************/
// Kal - October 2000
void do_playerlock(char_data *ch, char *argument)
{	
	if(IS_NULLSTR(argument)){
		if(GAMESETTING(GAMESET_PLAYER_LOCKED)){
			ch->wrapln("The game is currently playerlocked, to unplayerlock "
				"the game type `=Cplayerlock disable`x");
		}else{
			ch->wrapln("The game is not currently playerlocked, to playerlock "
				"the game type `=Cplayerlock enable`x");
		}
		ch->wrapln("Note: it is possible to set a connect password in gameedit - "
			"so only players that know the connect password can connect.");
		return;
	}

	if(GAMESETTING(GAMESET_PLAYER_LOCKED)){
		if(!str_cmp("disable", argument)){
			ch->println("Playerlock disabled, The game is no longer playerlocked.");
			REMOVE_BIT(game_settings->flags, GAMESET_PLAYER_LOCKED);
			ch->wrapln("Note: it is possible to set a connect password in gameedit - "
				"so only players that know the connect password can connect.");
			do_save_gamesettings(ch, "");
			return;
		}
	}else{
		if(!str_cmp("enable", argument)){
			ch->println("Playerlock enabled, The game is now playerlocked.");
			SET_BIT(game_settings->flags, GAMESET_PLAYER_LOCKED);
			ch->wrapln("Note: it is possible to set a connect password in gameedit - "
				"so only players that know the connect password can connect.");
			do_save_gamesettings(ch, "");
			return;
		}
	}
	ch->printlnf("Playerlock parameter '%s' incorrect.", argument);
	do_playerlock(ch,"");
}
/**************************************************************************/
// Kal - October 2000
void do_newbielock(char_data *ch, char *argument)
{	
	if(IS_NULLSTR(argument)){
		if(GAMESETTING(GAMESET_NEWBIE_LOCKED)){
			ch->wrapln("The game is currently newbielocked, to unnewbielock "
				"the game type `=Cnewbielock disable`x");
		}else{
			ch->wrapln("The game is not currently newbielocked, to newbielock "
				"the game type `=Cnewbielock enable`x");
		}
		ch->wrapln("Note: it is possible to set a create password in gameedit - "
			"so admin/others that know the create passwords can still create.");
		return;
	}

	if(GAMESETTING(GAMESET_NEWBIE_LOCKED)){
		if(!str_cmp("disable", argument)){
			ch->println("Newbielock disabled, The game is no longer newbielocked.");
			REMOVE_BIT(game_settings->flags, GAMESET_NEWBIE_LOCKED);
			ch->wrapln("Note: it is possible to set a create password in gameedit - "
				"so admin/others that know the create passwords can still create.");
			return;
		}
	}else{
		if(!str_cmp("enable", argument)){
			ch->println("Newbielock enabled, The game is now newbielocked.");
			SET_BIT(game_settings->flags, GAMESET_NEWBIE_LOCKED);
			ch->wrapln("Note: it is possible to set a create password in gameedit - "
				"so admin/others that know the create passwords can still create.");
			return;
		}
	}
	ch->printlnf("Newbielock parameter '%s' incorrect.", argument);
	do_newbielock(ch,"");
}
/**************************************************************************/
bool gameedit_no_resolve_ip(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument)){
		ch->println("Syntax: `=Cno_resolve_ip <string>`x");
		ch->println("Syntax: `=Cno_resolve_ip -`x to clear the don't resolve ip.");
		ch->println("The use for this setting, is if you are using a mud reboot "
			"script that checks if the mud is alive by connecting to it... if so "
			"put the ip of where it is connecting from here");
		return false;
    }

	if(str_cmp("-", argument)){
		ch->printlnf("The no_resolve_ip setting has been changed from '%s' to '%s'.", 
			game_settings->no_resolve_ip_text, argument );		
		replace_string(game_settings->no_resolve_ip_text,argument);
		ch->println("NOTE: NO ATTEMPT IS MADE TO ENSURE THIS IS A VALID IP ADDRESS.");
		ch->println("It should be in the format: a.b.c.d for an ipv4 address,");
		ch->println("or an ipv6 address as quoted in the logs.");
	}else{
		ch->println("The no_resolve_ip setting has been cleared."); 
		replace_string(game_settings->no_resolve_ip_text,"");
	}
    return true;
}
/**************************************************************************/
bool gameedit_setwhoformat(char_data *ch, char *argument)
{
	if(IS_NULLSTR(argument)){
		ch->println("syntax: setwhoformat <format>");
		ch->println("Where the default who format is one of the following:");
		who_show_formats(ch);
		return false;
	}

	int format=who_format_lookup(argument);
	if(format<0){
		ch->printlnf("'%s' is not an available who format.", argument);
		gameedit_setwhoformat(ch, "");
		return false;
	}

	if(game_settings->default_who_format_index==format){
		ch->printlnf("The default who format is already set to %s.", who_format_name(format));
		return false;
	}

	ch->printlnf("Default Who format changed from %s to %s.",
		who_format_name(game_settings->default_who_format_index),
		who_format_name(format));

	game_settings->default_who_format_index=UMAX(1, format); // 1, since you can't have default here
	replace_string(game_settings->default_who_format, who_format_name(game_settings->default_who_format_index));

    return true;
}
/**************************************************************************/
// used to edit/display the style sheet
bool gameedit_stylesheet( char_data *ch, char * argument)
{
	if(IS_NULLSTR(argument) || str_cmp(argument, "edit") ){
		ch->titlebar("INTERGRATED WEBSERVER STYLE SHEET EDITOR");
		ch->wrapln("The integrated webserver uses a default style sheet "
			"associated with all pages it displays.  You can edit this "
			"style sheet using gameedits stylesheet command.");
		ch->println("The default style sheet is currently set to:`W");
		ch->printbw(game_settings->style_sheet);
		ch->printlnf("`xTo edit this type `=C%s`x",
			mxp_create_send(ch, "stylesheet edit"));
		ch->titlebar("");
		return false;
	}
	ch->println("Editing website default style sheet...");
	string_append( ch, &game_settings->style_sheet);

    return true;
}
/**************************************************************************/
bool gameedit_dsv( char_data *ch, char * argument)
{
	int v;
	if(IS_NULLSTR(argument) || !is_number(argument)){
		ch->println("dsv <number 150->4000");
		return false;
	}

	v=atoi(argument);
	game_settings->damage_scale_value=URANGE(150, v, 4000); 
	return true;
}
/**************************************************************************/
bool gameedit_set(char_data *ch, char *argument)
{
	ch->println("Ambiguous use of 'set' while in gameedit");
	ch->println("If you want to use the 'set' command, you must exit gameedit with the done command first.");
	ch->println("Syntax of other set* commands available in gameedit:"); 
	ch->println("Syntax: `=Csethelpheader <string>`x");
	ch->println("Syntax: `=Csethelpheader -`x   to have no header");
	ch->println("Syntax: `=Csethelpfooter <string>`x");
	ch->println("Syntax: `=Csethelpfooter -`x   to have no footer");
	ch->println("Syntax: `=Csethelpprevnextfooter <string>`x");
	ch->println("Syntax: `=Csethelpprevnextfooter -`x   to have no prevnext footer");
	return false;
}
/**************************************************************************/
bool gameedit_sethelpheader(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: `=Csethelpheader <string>`x");
		ch->println("Syntax: `=Csethelpheader -`x   to have no header");
		return false;		
    }
	if(*argument=='-' && *(argument+1)=='\0'){
		ch->printlnf("Help Header cleared, was '%s`x'.", 
			game_settings->help_header_bar);
		replace_string(game_settings->help_header_bar,"");
		return true;
	}

    ch->printlnf("Help Header changed from '%s`x' to '%s`x'.", 
		game_settings->help_header_bar, argument );

    replace_string( game_settings->help_header_bar, argument );
    return true;
}
/**************************************************************************/
bool gameedit_sethelpfooter(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: `=Csethelpfooter <string>`x");
		ch->println("Syntax: `=Csethelpfooter -`x   to have no footer");
		return false;
    }
	if(*argument=='-' && *(argument+1)=='\0'){
		ch->printlnf("Help Footer cleared, was '%s`x'.", 
			game_settings->help_footer_bar);
		replace_string(game_settings->help_footer_bar,"");
		return true;
	}

    ch->printlnf("Help Footer changed from '%s`x' to '%s`x'.", 
		game_settings->help_footer_bar, argument );

    replace_string( game_settings->help_footer_bar, argument );
    return true;
}
/**************************************************************************/
bool gameedit_sethelpprevnextfooter(char_data *ch, char *argument)
{
    if ( IS_NULLSTR(argument))
    {
		ch->println("Syntax: `=Csethelpprevnextfooter <string>`x");
		ch->println("Syntax: `=Csethelpprevnextfooter -`x   to have no prevnext footer");
		return false;		
    }
	if(*argument=='-' && *(argument+1)=='\0'){
		ch->printlnf("Help PrevNext Footer cleared, was '%s`x'.", 
			game_settings->help_prevnext_footer_bar);
		replace_string(game_settings->help_prevnext_footer_bar,"");
		return true;
	}

    ch->printlnf("Help PrevNext Footer changed from '%s`x' to '%s`x'.", 
		game_settings->help_prevnext_footer_bar, argument );

    replace_string( game_settings->help_prevnext_footer_bar, argument );
    return true;
}

/**************************************************************************/
GIO_START(continent_type)
GIO_STR(name)
GIO_FINISH_STRDUP_EMPTY
/**************************************************************************/
void save_continents()
{
	log_string("save_continents(): saving continents list.");
	GIOSAVE_LIST(continent_list, continent_type, CONTINENTS_FILE, true);
}

/**************************************************************************/
void create_continents()
{
	const char *continent_create_table[]={
		"none", "valarin", "endomar", "kerallyan", "rhynia", 
		"ring_isle", "orcs", "elenarthya", "confederacy",
		"delenth", "markrist", "aarislan", "faerie_ring",
		"plane_air", "plane_water", "plane_earth", "plane_fire",
		NULL
	};

	continent_type *c;

	int i;
	for(i=0; !IS_NULLSTR(continent_create_table[i]); i++){
		// loop thru till one past the final entry
	}
	
	continent_list=NULL;
	// now loop backwards to add continents in reverse order
	for(i--;i>0; i--){
		c=new continent_type; 
		c->name=str_dup(continent_create_table[i]);
		c->next=continent_list;
		continent_list=c;
	}
}

/**************************************************************************/
void create_none_continent_if_required()
{
	continent_type*c;
	// exit if there is already a none_continent
	if(continent_exact_lookup("none")){
		return;
	}

	logf("Automatically adding 'none' continent."),
	
	c=new continent_type;

	// insert the new continent at the front of the list
	c->name=str_dup("none");
	c->next=continent_list;
	continent_list=c;
	save_continents();
}

/**************************************************************************/
void load_continents()
{
	log_string("load_continents(): Reading in continents list from '"CONTINENTS_FILE"'.");
	if(file_exists(CONTINENTS_FILE)){
		GIOLOAD_LIST(continent_list, continent_type, CONTINENTS_FILE);
		if(!continent_list){
			log_string("No continents found in file, creating new set.");
			create_continents();
		}
		create_none_continent_if_required();
		return;		
	}
	log_string("Continents file not found, creating default set of continents.");
	create_continents();

	create_none_continent_if_required();
	save_continents();
}

/**************************************************************************/
/**************************************************************************/

