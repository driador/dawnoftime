/**************************************************************************/
// interp.h - main game interpreter and related code
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

// this is a listing of all the commands and command related data 

// for command types 
#define ML 	MAX_LEVEL		// implementor
#define L1	MAX_LEVEL - 1  	// creator
#define L2	MAX_LEVEL - 2	// supreme being
#define L3	MAX_LEVEL - 3	// deity
#define L4  MAX_LEVEL - 4   // guardian
#define L5	MAX_LEVEL - 5	// immortal
#define L6	MAX_LEVEL - 6	// demigod
#define L7	MAX_LEVEL - 7	// angel
#define L8	MAX_LEVEL - 8	// avatar
#define IM  LEVEL_IMMORTAL  // avatar
#define HE	LEVEL_HERO		// hero
#define AML ABSOLUTE_MAX_LEVEL
#define DIS ABSOLUTE_MAX_LEVEL+1 // disabled

// Structure for a single command in the command lookup table.
struct	cmd_type
{
    const char * name;
    DO_FUN	*do_fun;
    sh_int	position;
    sh_int	level;
    sh_int	log;
    sh_int	show;
	long	flags;
	long	council;
	sh_int	category;
	long	grantgroups;
};

/* the command table itself */
extern	struct	cmd_type	cmd_table	[];

// olc commands
typedef	bool OLC_FUN		args( ( char_data *ch, char *argument ) );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun

DECLARE_OLC_FUN( redit_mlist);
DECLARE_OLC_FUN( redit_olist);
DECLARE_OLC_FUN( redit_rlist);

DECLARE_DO_FUN(do_rshow);
DECLARE_DO_FUN(do_oshow);
DECLARE_DO_FUN(do_mshow);
DECLARE_DO_FUN(do_olist);
DECLARE_DO_FUN(do_mlist);
DECLARE_DO_FUN(do_rlist);
DECLARE_DO_FUN(do_rpurge);


/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */

DECLARE_DO_FUN( do_add				);
DECLARE_DO_FUN( do_addclan			);
DECLARE_DO_FUN(	do_advance			);
DECLARE_DO_FUN( do_tadvance			);
DECLARE_DO_FUN( do_affects			);
DECLARE_DO_FUN( do_afk				);
DECLARE_DO_FUN( do_alia				);
DECLARE_DO_FUN( do_alias			);
DECLARE_DO_FUN(	do_allow			);
DECLARE_DO_FUN( do_amote			);
DECLARE_DO_FUN( do_announce			);
DECLARE_DO_FUN( do_answer			);
DECLARE_DO_FUN( do_awareness		);
DECLARE_DO_FUN(	do_areas			);
DECLARE_DO_FUN(	do_at				);
DECLARE_DO_FUN( do_atlevel			);
DECLARE_DO_FUN( do_autoassist		);
DECLARE_DO_FUN( do_autoexit			);
DECLARE_DO_FUN( do_autogold			);
DECLARE_DO_FUN( do_autolist			);
DECLARE_DO_FUN( do_autoloot			);
DECLARE_DO_FUN( do_autosubdue		);
DECLARE_DO_FUN( do_autoself			);
DECLARE_DO_FUN( do_autoreformat		);
DECLARE_DO_FUN( do_autosplit		);

DECLARE_DO_FUN(	do_backstab			);
DECLARE_DO_FUN(	do_pbackstab		);
DECLARE_DO_FUN(	do_bamfin			);
DECLARE_DO_FUN(	do_bamfout			);
DECLARE_DO_FUN(	do_ban				);
DECLARE_DO_FUN( do_bank				);
DECLARE_DO_FUN( do_bash				);
DECLARE_DO_FUN( do_berserk			);
DECLARE_DO_FUN(	do_brandish			);
DECLARE_DO_FUN( do_brief			);
DECLARE_DO_FUN(	do_bug				);
DECLARE_DO_FUN(	do_buy				);
DECLARE_DO_FUN( do_brew				);

DECLARE_DO_FUN(	do_cast				);
DECLARE_DO_FUN( do_changes			);
DECLARE_DO_FUN( do_channel			);
DECLARE_DO_FUN( do_channels			);
DECLARE_DO_FUN( do_chardescript		);
DECLARE_DO_FUN(	do_checkdead		);
DECLARE_DO_FUN( do_clanlist			);
DECLARE_DO_FUN( do_clanranks		);
DECLARE_DO_FUN( do_class			);
DECLARE_DO_FUN( do_clone			);
DECLARE_DO_FUN(	do_close			);
DECLARE_DO_FUN( do_claw				);
DECLARE_DO_FUN(	do_commands			);
DECLARE_DO_FUN(	do_commandsnoop		);
DECLARE_DO_FUN( do_combine			);
DECLARE_DO_FUN( do_compact			);
DECLARE_DO_FUN(	do_compare			);
DECLARE_DO_FUN( do_consider			);
DECLARE_DO_FUN( do_cspinfo			);
DECLARE_DO_FUN( do_count			);
DECLARE_DO_FUN( do_crashloop		);
DECLARE_DO_FUN(	do_credits			);
DECLARE_DO_FUN( do_cwhere			);

DECLARE_DO_FUN( do_delet			);
DECLARE_DO_FUN( do_delete			);
DECLARE_DO_FUN( do_demote			);
DECLARE_DO_FUN(	do_deny				);
DECLARE_DO_FUN(	do_description		);
DECLARE_DO_FUN( do_dirt				);
DECLARE_DO_FUN(	do_disarm			);
DECLARE_DO_FUN( do_dismount			);
DECLARE_DO_FUN(	do_disconnect		);
DECLARE_DO_FUN( do_dlook			);
DECLARE_DO_FUN( do_donate			);
DECLARE_DO_FUN(	do_down				);
DECLARE_DO_FUN(	do_drink			);
DECLARE_DO_FUN(	do_drop				);
DECLARE_DO_FUN( do_dumpstats		);

DECLARE_DO_FUN(	do_east				);
DECLARE_DO_FUN(	do_eat				);
DECLARE_DO_FUN(	do_echo				);
DECLARE_DO_FUN(	do_emote			);
DECLARE_DO_FUN( do_empty			);
DECLARE_DO_FUN( do_enter			);
DECLARE_DO_FUN( do_envenom			);
DECLARE_DO_FUN(	do_equipment		);
DECLARE_DO_FUN(	do_examine			);
DECLARE_DO_FUN( do_exitlist			);
DECLARE_DO_FUN(	do_exits			);

DECLARE_DO_FUN(	do_fill				);
DECLARE_DO_FUN( do_flag				);
DECLARE_DO_FUN(	do_flee				);
DECLARE_DO_FUN(	do_follow			);
DECLARE_DO_FUN(	do_force			);
DECLARE_DO_FUN( do_forcetick		);
DECLARE_DO_FUN(	do_freeze			);

DECLARE_DO_FUN( do_gain				);
DECLARE_DO_FUN(	do_get				);
DECLARE_DO_FUN(	do_give				);
DECLARE_DO_FUN( do_sgive			);
DECLARE_DO_FUN(	do_gecho			);
DECLARE_DO_FUN(	do_goecho			);
DECLARE_DO_FUN( do_gouge		    );
DECLARE_DO_FUN( do_grab				);
DECLARE_DO_FUN(	do_goto				);
DECLARE_DO_FUN(	do_group			);
DECLARE_DO_FUN(	do_gtell			); // NOT USED

DECLARE_DO_FUN( do_heal				);
DECLARE_DO_FUN( do_hedit			);
DECLARE_DO_FUN(	do_help				);
DECLARE_DO_FUN( do_helplist			);
DECLARE_DO_FUN(	do_hide				);
DECLARE_DO_FUN( do_hlist			);
DECLARE_DO_FUN(	do_holylight		);
DECLARE_DO_FUN( do_holyname		    );
DECLARE_DO_FUN( do_holyspeech	    );
DECLARE_DO_FUN( do_holyvnum			);
DECLARE_DO_FUN( do_hsave			);
DECLARE_DO_FUN( do_huh				);

DECLARE_DO_FUN(	do_idea				);
DECLARE_DO_FUN( do_skillgroups		);
DECLARE_DO_FUN(	do_immtalk			);
DECLARE_DO_FUN( do_imotd			);
DECLARE_DO_FUN( do_incognito		);
DECLARE_DO_FUN(	do_inventory		);

DECLARE_DO_FUN(	do_kick				);
DECLARE_DO_FUN(	do_kill				);
DECLARE_DO_FUN(	do_pkill			);

DECLARE_DO_FUN( do_language			);
DECLARE_DO_FUN( do_lag				);
DECLARE_DO_FUN( do_laston			);
DECLARE_DO_FUN( do_lastonremove		);
DECLARE_DO_FUN( do_lastonreload		);
DECLARE_DO_FUN( do_lay_on_hands		);
DECLARE_DO_FUN( do_letgain			);
DECLARE_DO_FUN( do_lethero			);
DECLARE_DO_FUN(	do_list				);
DECLARE_DO_FUN( do_load				);
DECLARE_DO_FUN( do_localecho		);
DECLARE_DO_FUN(	do_lock				);
DECLARE_DO_FUN( do_lore				);
DECLARE_DO_FUN(	do_log				);
DECLARE_DO_FUN(	do_look				);

DECLARE_DO_FUN(	do_memory			);
DECLARE_DO_FUN(	do_mfind			);
DECLARE_DO_FUN(	do_mload			);
DECLARE_DO_FUN(	do_mset				);
DECLARE_DO_FUN(	do_mstat			);
DECLARE_DO_FUN( do_mode				);
DECLARE_DO_FUN( do_moot				);
DECLARE_DO_FUN( do_motd				);
DECLARE_DO_FUN( do_mp				);
DECLARE_DO_FUN( do_mpdump			);
DECLARE_DO_FUN( do_mpinfo			);
DECLARE_DO_FUN( do_mplist			);
DECLARE_DO_FUN( do_mpreset			);
DECLARE_DO_FUN( do_mpshow			);
DECLARE_DO_FUN( do_mpstat			);
DECLARE_DO_FUN( do_mudstats			);
DECLARE_DO_FUN( do_mwhere			);

DECLARE_DO_FUN( do_news				);
DECLARE_DO_FUN( do_noble			);
DECLARE_DO_FUN( do_nochannels		);
DECLARE_DO_FUN(	do_noemote			);
DECLARE_DO_FUN( do_nofollow			);
DECLARE_DO_FUN( do_noloot			);
DECLARE_DO_FUN( do_norp		        );
DECLARE_DO_FUN(	do_north			);
DECLARE_DO_FUN( do_northeast	    );
DECLARE_DO_FUN( do_northwest	    );
DECLARE_DO_FUN(	do_noshout			);
DECLARE_DO_FUN( do_nosummon			);
DECLARE_DO_FUN(	do_note				);
DECLARE_DO_FUN( do_noteach			);
DECLARE_DO_FUN(	do_notell			);
DECLARE_DO_FUN( do_nspeak			);
DECLARE_DO_FUN( do_nsupport			);
DECLARE_DO_FUN( do_ntalk			);

DECLARE_DO_FUN( do_objdesc			);
DECLARE_DO_FUN( do_oextended		);
DECLARE_DO_FUN(	do_ofind			);
DECLARE_DO_FUN( do_olcgoto			);
DECLARE_DO_FUN(	do_oload			);
DECLARE_DO_FUN( do_ooc				);
DECLARE_DO_FUN(	do_open				);
DECLARE_DO_FUN(	do_order			);
DECLARE_DO_FUN(	do_oset				);
DECLARE_DO_FUN(	do_ostat			);
DECLARE_DO_FUN( do_outcast			);
DECLARE_DO_FUN( do_overwritepasswd	);
DECLARE_DO_FUN( do_owhere			);

DECLARE_DO_FUN( do_page				);
DECLARE_DO_FUN( do_panic			);
DECLARE_DO_FUN(	do_password			);
DECLARE_DO_FUN(	do_peace			);
DECLARE_DO_FUN( do_pecho			);
DECLARE_DO_FUN( do_penalty			);
DECLARE_DO_FUN( do_permban			);
DECLARE_DO_FUN(	do_pick				);
DECLARE_DO_FUN( do_pinfo			);
DECLARE_DO_FUN( do_pipe				);
DECLARE_DO_FUN( do_pkinfo			);
DECLARE_DO_FUN( do_pmote			);
DECLARE_DO_FUN( do_pour				);
DECLARE_DO_FUN( do_pounce			);
DECLARE_DO_FUN(	do_practice			);
DECLARE_DO_FUN( do_pray				);
DECLARE_DO_FUN( do_prefi			);
DECLARE_DO_FUN( do_prefix			);
DECLARE_DO_FUN( do_promote			);
DECLARE_DO_FUN( do_prompt			);
DECLARE_DO_FUN( do_protect			);
DECLARE_DO_FUN(	do_purge			);
DECLARE_DO_FUN(	do_put				);

DECLARE_DO_FUN(	do_qdie				);
DECLARE_DO_FUN(	do_quaff			);
DECLARE_DO_FUN( do_question			);
DECLARE_DO_FUN(	do_qui				);
DECLARE_DO_FUN( do_quiet			);
DECLARE_DO_FUN(	do_quit				);

DECLARE_DO_FUN( do_read				);
DECLARE_DO_FUN( do_read_finger		);
DECLARE_DO_FUN(	do_reboo			);
DECLARE_DO_FUN(	do_reboot			);
DECLARE_DO_FUN( do_rebootresolver	);
DECLARE_DO_FUN(	do_recall			);
DECLARE_DO_FUN(	do_recite			);
DECLARE_DO_FUN( do_relookup			);
DECLARE_DO_FUN(	do_remove			);
DECLARE_DO_FUN(	do_rent				);
DECLARE_DO_FUN( do_rename			);
DECLARE_DO_FUN( do_replay			);
DECLARE_DO_FUN( do_replaychar		);	
DECLARE_DO_FUN(	do_reply			);
DECLARE_DO_FUN( do_retell			);
DECLARE_DO_FUN(	do_rescue			);
DECLARE_DO_FUN( do_resetarea		);
DECLARE_DO_FUN( do_resetroom		);
DECLARE_DO_FUN(	do_rest				);
DECLARE_DO_FUN(	do_restore			);
DECLARE_DO_FUN(	do_return			);
DECLARE_DO_FUN( do_requestooc		);
DECLARE_DO_FUN( do_ride				);
DECLARE_DO_FUN( do_rinfo			);
DECLARE_DO_FUN(	do_rset				);
DECLARE_DO_FUN(	do_rstat			);
DECLARE_DO_FUN( do_rules			);

DECLARE_DO_FUN(	do_save				);
DECLARE_DO_FUN(	do_say				);
DECLARE_DO_FUN( do_sayto			);
DECLARE_DO_FUN(	do_score			);
DECLARE_DO_FUN( do_scroll			);
DECLARE_DO_FUN( do_scribe			);
DECLARE_DO_FUN( do_second			);
DECLARE_DO_FUN( do_seek				);
DECLARE_DO_FUN( do_seevnum			);
DECLARE_DO_FUN(	do_sell				);
DECLARE_DO_FUN( do_set				);
DECLARE_DO_FUN( do_showaffects		);
DECLARE_DO_FUN( do_showplayerlist	);
DECLARE_DO_FUN( do_short			);
DECLARE_DO_FUN(	do_shutdow			);
DECLARE_DO_FUN(	do_shutdown			);
DECLARE_DO_FUN( do_silently			);
DECLARE_DO_FUN( do_sit				);
DECLARE_DO_FUN( do_skills			);
DECLARE_DO_FUN(	do_sla				);
DECLARE_DO_FUN(	do_slay				);
DECLARE_DO_FUN( do_slice			);
DECLARE_DO_FUN(	do_sleep			);
DECLARE_DO_FUN(	do_slookup			);
DECLARE_DO_FUN( do_smote			);
DECLARE_DO_FUN(	do_sneak			);
DECLARE_DO_FUN(	do_snoop			);
DECLARE_DO_FUN( do_socials			);
DECLARE_DO_FUN(	do_south			);
DECLARE_DO_FUN( do_southeast		);
DECLARE_DO_FUN( do_southwest		);
DECLARE_DO_FUN( do_sockets			);
DECLARE_DO_FUN(	do_spelldebug		);
DECLARE_DO_FUN( do_spells			);
DECLARE_DO_FUN( do_spinfo			);
DECLARE_DO_FUN(	do_split			);
DECLARE_DO_FUN( do_sscan			);
DECLARE_DO_FUN(	do_sset				);
DECLARE_DO_FUN(	do_stand			);
DECLARE_DO_FUN( do_stat				);
DECLARE_DO_FUN(	do_steal			);
DECLARE_DO_FUN( do_story			);
DECLARE_DO_FUN( do_string			);
DECLARE_DO_FUN( do_study			);
DECLARE_DO_FUN( do_surrender		);
DECLARE_DO_FUN(	do_switch			);
DECLARE_DO_FUN( do_system			);

DECLARE_DO_FUN( do_tame				);
DECLARE_DO_FUN( do_teach			);
DECLARE_DO_FUN(	do_tell				);
DECLARE_DO_FUN(	do_time				);
DECLARE_DO_FUN(	do_train			);
DECLARE_DO_FUN(	do_transfer			);
DECLARE_DO_FUN( do_stransfer		);
DECLARE_DO_FUN( do_testhelps		);
DECLARE_DO_FUN( do_tether			);
DECLARE_DO_FUN( do_toprp			);
DECLARE_DO_FUN( do_trip				);
DECLARE_DO_FUN(	do_trust			);
DECLARE_DO_FUN(	do_typo				);

DECLARE_DO_FUN( do_unalias			);
DECLARE_DO_FUN( do_unletgain		);
DECLARE_DO_FUN( do_unlethero		);
DECLARE_DO_FUN(	do_unlock			);
DECLARE_DO_FUN( do_unread			);
DECLARE_DO_FUN(	do_up				);
DECLARE_DO_FUN( do_update			);

DECLARE_DO_FUN(	do_value			);
DECLARE_DO_FUN(	do_vanish			);
DECLARE_DO_FUN(	do_visible			);
DECLARE_DO_FUN( do_violate			);
DECLARE_DO_FUN( do_vote				);
DECLARE_DO_FUN( do_vnum				);
DECLARE_DO_FUN( do_vnummap			);

DECLARE_DO_FUN(	do_wake				);
DECLARE_DO_FUN(	do_wear				);
DECLARE_DO_FUN(	do_weather			);
DECLARE_DO_FUN(	do_west				);
DECLARE_DO_FUN(	do_where			);
DECLARE_DO_FUN( do_whisper			);
DECLARE_DO_FUN(	do_who				);
DECLARE_DO_FUN( do_whois			);
DECLARE_DO_FUN( do_whovis			);
DECLARE_DO_FUN(	do_wimpy			);
DECLARE_DO_FUN(	do_wizhelp			);
DECLARE_DO_FUN(	do_wizinvis			);
DECLARE_DO_FUN( do_wizlist			);
DECLARE_DO_FUN( do_wiznet			);
DECLARE_DO_FUN( do_worth			);

DECLARE_DO_FUN( do_xpen				);


DECLARE_DO_FUN(	do_zap				);
DECLARE_DO_FUN( do_zecho			);

// june 98
DECLARE_DO_FUN( do_autopeek			);
DECLARE_DO_FUN(	do_peek				);
DECLARE_DO_FUN(	do_holywalk			);
DECLARE_DO_FUN( do_rwhere			);
DECLARE_DO_FUN( do_breaktell		);
DECLARE_DO_FUN( do_append			);
DECLARE_DO_FUN( do_startWeb			);
DECLARE_DO_FUN( do_reducelaston		);
DECLARE_DO_FUN( do_for				);
DECLARE_DO_FUN(	do_castatlevel		);

// july 98
DECLARE_DO_FUN(	do_gore				); // Kerenos
DECLARE_DO_FUN(	do_admintalk		); // Kal
DECLARE_DO_FUN(	do_retreat			); // Kerenos
DECLARE_DO_FUN(	do_throw			); // Kerenos/Kal
DECLARE_DO_FUN(	do_bolist			); // Kal
DECLARE_DO_FUN(	do_who1234			); // Kal
DECLARE_DO_FUN(	do_who4321			); // Kal
DECLARE_DO_FUN(	do_goooc			); // Kal

// August 98
DECLARE_DO_FUN(	do_saveraces		); // Kal
DECLARE_DO_FUN( do_nopray			); // Kal
DECLARE_DO_FUN( do_npcinfo			); // Kal
DECLARE_DO_FUN( do_saveclasses		); // Kal
DECLARE_DO_FUN( do_loadclasses		); // Kal
DECLARE_DO_FUN( do_holylist			); // Kal

// September 98
DECLARE_DO_FUN( do_racelist			); // Kal
DECLARE_DO_FUN( do_notenet			); // Kal
DECLARE_DO_FUN( do_tgive			); // Ker
DECLARE_DO_FUN( do_tremove			); // Ker
DECLARE_DO_FUN( do_twhere			); // Ker
DECLARE_DO_FUN( do_ttimer			); // Ker
DECLARE_DO_FUN( do_forage			); // Ker
DECLARE_DO_FUN( do_subdue			); // Kal
DECLARE_DO_FUN( do_whoinvis			); // Kal
DECLARE_DO_FUN( do_checkhelp		); // Kal
DECLARE_DO_FUN( do_checkooc			); // Kal
DECLARE_DO_FUN( do_holylist			); // Ker
DECLARE_DO_FUN( do_lastonstats		); // Kal
DECLARE_DO_FUN( do_hold				); // Kal
DECLARE_DO_FUN( do_map				); // Kal
DECLARE_DO_FUN( do_automap			); // Kal
DECLARE_DO_FUN( do_mapnum			); // Kal
DECLARE_DO_FUN( do_amap				); // Kal

// October 98
DECLARE_DO_FUN( do_guild			); // Kal
DECLARE_DO_FUN( do_savespsktable	); // Kal
DECLARE_DO_FUN( do_spellskilllist	); // Kal
DECLARE_DO_FUN( do_sedit			); // Kal
DECLARE_DO_FUN( do_savesp2			); // Kal

// November 98
DECLARE_DO_FUN( do_checknsupport	); // Kal
DECLARE_DO_FUN(	do_fadein			); // Ker
DECLARE_DO_FUN(	do_fadeout			); // Ker
DECLARE_DO_FUN( do_nomapexits		); // Kal
DECLARE_DO_FUN( do_mapclear			); // Kal
DECLARE_DO_FUN( do_fullmap			); // Kal
DECLARE_DO_FUN( do_checkban			); // Kal
DECLARE_DO_FUN( do_banedit			); // Kal

// Dec 98
DECLARE_DO_FUN( do_telnetga			); // Kal 
DECLARE_DO_FUN( do_anote			); // Kal 
DECLARE_DO_FUN( do_inote			); // Kal 
DECLARE_DO_FUN( do_strip_affects	); // Kerenos
DECLARE_DO_FUN( do_checkntalk		); // Kal with ooc replaced by ntalk by Ker :)
DECLARE_DO_FUN( do_build			); // Reave
DECLARE_DO_FUN( do_cook				); // Kerenos
DECLARE_DO_FUN( do_ispell			);
DECLARE_DO_FUN( do_bolist			); // Kal
DECLARE_DO_FUN( do_showban			); // Kal
DECLARE_DO_FUN( do_setalliance		); // Kerenos
DECLARE_DO_FUN( do_settendency		); // Kerenos

// Jan 99
DECLARE_DO_FUN( do_flip				); // Kerenos
DECLARE_DO_FUN( do_diagnose			); // Celrion
DECLARE_DO_FUN( do_write_dynamic_include); // Kalahn
DECLARE_DO_FUN( do_hurl				); // Kerenos
DECLARE_DO_FUN( do_entangle			); // Kerenos
DECLARE_DO_FUN( do_hobble			); // Kerenos
DECLARE_DO_FUN( do_shieldcleave		); // Kerenos
DECLARE_DO_FUN( do_dervish			); // Kerenos
DECLARE_DO_FUN( do_dream			); // Ylerin
DECLARE_DO_FUN( do_cutoff			); // Kerenos
DECLARE_DO_FUN( do_zoecho			); // Kerenos
DECLARE_DO_FUN( do_loadskilltable	); // Kal
DECLARE_DO_FUN( do_autoexamine		); // Celrion
DECLARE_DO_FUN( do_sheathe			); // Kerenos
DECLARE_DO_FUN( do_draw				); // Kerenos
DECLARE_DO_FUN( do_conceal			); // Kerenos
DECLARE_DO_FUN( do_sshow			); // Kal
DECLARE_DO_FUN( do_material_list	); // Kal
DECLARE_DO_FUN( do_rawcolour		); // Kal
DECLARE_DO_FUN( do_glance			); // Kal
DECLARE_DO_FUN( do_kneel			); // Kal

// Feb 99
DECLARE_DO_FUN( do_sinfo			); // Kal
DECLARE_DO_FUN( do_offlineletgain   ); // Kal
DECLARE_DO_FUN( do_requestletgain   ); // Kal
DECLARE_DO_FUN( do_list_letgains	); // Kal
DECLARE_DO_FUN( do_gamble			); // Ker
DECLARE_DO_FUN( do_declineletgain	); // Kal
DECLARE_DO_FUN( do_cancelletgain	); // Kal
DECLARE_DO_FUN( do_clearletgain		); // Kal
DECLARE_DO_FUN( do_iwizi			); // Kal
DECLARE_DO_FUN( do_owizi			); // Kal
DECLARE_DO_FUN( do_misc				); // Kal 
DECLARE_DO_FUN( do_showmisc			); // Kal 
DECLARE_DO_FUN( do_nomisc			); // Kal
DECLARE_DO_FUN( do_charge			); // Cel & Kerenos
DECLARE_DO_FUN( do_createspell		); // Kal
DECLARE_DO_FUN(	do_objrestrict		); // Kal

// March 99
DECLARE_DO_FUN( do_checklog);			// Kal
DECLARE_DO_FUN( do_showaffectprofile);  // Ker
DECLARE_DO_FUN( do_cinfo			);	// Kal
DECLARE_DO_FUN( do_charnotes		);	// Kal
DECLARE_DO_FUN( do_editcharnotes	);	// Kal
DECLARE_DO_FUN( do_commune			);	// Kal
DECLARE_DO_FUN( do_summon			);	// Kal
DECLARE_DO_FUN( do_cast_redirect	);	// Kal
DECLARE_DO_FUN( do_allowimmtalk		);	// Kal
DECLARE_DO_FUN( do_overhead			);	// Ker
DECLARE_DO_FUN( do_boneshatter		);	// Ker
DECLARE_DO_FUN( do_bury				);	// Ker
DECLARE_DO_FUN( do_dig				);	// Ker

// April 99
DECLARE_DO_FUN( do_write_commandtable );// Ker
DECLARE_DO_FUN( do_lastoncouncil	);	// Kal
DECLARE_DO_FUN( do_comedit			);	// Ker
DECLARE_DO_FUN( do_freevnum			);	// Ker

// May 99
DECLARE_DO_FUN( do_untether			);	// Kal
DECLARE_DO_FUN( do_sharpen			);  // Cel & Jarren
DECLARE_DO_FUN( do_dedit			);	// Ker
DECLARE_DO_FUN( do_load_deities		);	// Ker

// June 99
DECLARE_DO_FUN( do_quester			);	// Kal
DECLARE_DO_FUN( do_removequester	);	// Kal
DECLARE_DO_FUN( do_autorecall		);	// Kal
DECLARE_DO_FUN( do_ntell			);	// Cel
DECLARE_DO_FUN( do_nreply			);	// Cel
DECLARE_DO_FUN( do_codetalk			);	// Ker
DECLARE_DO_FUN( do_vault			);	// Jar
DECLARE_DO_FUN( do_haxor			);	// Ker, who else would waste their time?
DECLARE_DO_FUN( do_noterestrict		);	// Kal

// July 99
DECLARE_DO_FUN( do_councillist		);	// Kal
DECLARE_DO_FUN( do_hightalk			);	// Kal
DECLARE_DO_FUN( do_saycolour		);	// Kal
DECLARE_DO_FUN( do_motecolour		);	// Kal

DECLARE_DO_FUN( do_listquest		);  // Jarren
DECLARE_DO_FUN( do_delquest			);  // Jarren
DECLARE_DO_FUN( do_qedit			);  // Jarren
DECLARE_DO_FUN( do_savequestdb		);  // Kal
DECLARE_DO_FUN( do_mxpmodlist		);	// Kal
DECLARE_DO_FUN( do_yell				);	// Kal
DECLARE_DO_FUN( do_testgeneric		);	// Kal

DECLARE_DO_FUN( do_debugroom		);	// Kal
DECLARE_DO_FUN( do_debugmob			);	// Kal
DECLARE_DO_FUN( do_debugobject		);	// Kal
DECLARE_DO_FUN( do_makecorefile		);	// Kal

// August 99
DECLARE_DO_FUN( do_addscript		);  // Celrion
DECLARE_DO_FUN( do_delscript		);  // Celrion
DECLARE_DO_FUN( do_listscripts		);  // Celrion
DECLARE_DO_FUN( do_runscript		);  // Celrion
DECLARE_DO_FUN( do_savedeities		);	// Ker
DECLARE_DO_FUN( do_fly				);	// Kal
DECLARE_DO_FUN( do_land				);	// Kal
DECLARE_DO_FUN( do_tracks			);	// Kal
DECLARE_DO_FUN( do_autolandonrest	);	// Kal
DECLARE_DO_FUN( do_uptime			);	// Kal
DECLARE_DO_FUN( do_setage			);	// Kal
DECLARE_DO_FUN( do_checkirc			);	// Kal
DECLARE_DO_FUN( do_addcourt			);	// Kal
DECLARE_DO_FUN( do_autovote			);	// Kal
DECLARE_DO_FUN( do_classstats		);	// Kal
DECLARE_DO_FUN( do_racestats		);	// Kal
DECLARE_DO_FUN( do_checkexits		);	// Kal

// Sept 99
DECLARE_DO_FUN( do_checkmoblog		);	// Kal
DECLARE_DO_FUN( do_msp				);	// Ker

// Oct 99
DECLARE_DO_FUN( do_disallowpkill	);	// Kal
DECLARE_DO_FUN( do_hotreboot		);	// Kal
DECLARE_DO_FUN( do_ignoremultilogins);	// Kal
DECLARE_DO_FUN( do_losepet			);	// Cel
DECLARE_DO_FUN( do_checkmultilog	);	// Kal

// Nov 99
DECLARE_DO_FUN( do_brlist			);	// Kal
DECLARE_DO_FUN( do_mptrace			);	// Kal
DECLARE_DO_FUN( do_tjunk			);	// Kal
DECLARE_DO_FUN( do_whotext			);  // Kal's waste of time
DECLARE_DO_FUN( do_rp_obj_load		);	// Ker
DECLARE_DO_FUN( do_rpsupport		);  // Ker
DECLARE_DO_FUN( do_setrace			);	// Ker
DECLARE_DO_FUN( do_aroomlist		);	// Ker
DECLARE_DO_FUN( do_mobloglist		);	// Kal
DECLARE_DO_FUN( do_iwhere			);	// Kal
DECLARE_DO_FUN( do_rpsheet			);	// Ker
DECLARE_DO_FUN( do_surname			);	// Ker
DECLARE_DO_FUN( do_birthplace		);	// Ker
DECLARE_DO_FUN( do_haircolour		);	// Ker
DECLARE_DO_FUN( do_eyecolour		);	// Ker
DECLARE_DO_FUN( do_height			);  // Ker
DECLARE_DO_FUN( do_weight			);  // Ker
DECLARE_DO_FUN( do_metric			);	// Ker
DECLARE_DO_FUN( do_ashow			);	// Kal
DECLARE_DO_FUN( do_traits			);	// Ker
DECLARE_DO_FUN( do_checktypos		);  // Ker

// Dec 99
DECLARE_DO_FUN( do_mphelp			);  // Kal
DECLARE_DO_FUN( do_scalemap			);  // Kal
DECLARE_DO_FUN( do_areamap			);  // Kal
DECLARE_DO_FUN( do_becomeactive		);  // Kal
DECLARE_DO_FUN( do_autoreset		);	// Ker
DECLARE_DO_FUN( do_duel				);  // Kal
DECLARE_DO_FUN( do_acceptduel		);  // Kal
DECLARE_DO_FUN( do_declineduel		);  // Kal
DECLARE_DO_FUN( do_bec				);  // Kal
DECLARE_DO_FUN( do_bypassduel		);  // Kal
DECLARE_DO_FUN( do_speedwalk		);	// Ker
DECLARE_DO_FUN( do_olcwizi			);	// Kal
DECLARE_DO_FUN( do_write_classes	);	// Kal
DECLARE_DO_FUN( do_read_classes		);	// Kal
DECLARE_DO_FUN( do_write_skills		);	// Kal
DECLARE_DO_FUN( do_write_skills		);	// Kal

// Jan 00
DECLARE_DO_FUN( do_autotrack		);  // Kal
DECLARE_DO_FUN( do_setrooms			);	// Ker
DECLARE_DO_FUN( do_classedit		);	// Kal
DECLARE_DO_FUN( do_com_categorylist );	// Ker
DECLARE_DO_FUN( do_atell			);	// Kal
DECLARE_DO_FUN( do_ktell			);	// Kal
DECLARE_DO_FUN( do_genname			);	// Kal
DECLARE_DO_FUN( do_read_nameprofiles);	// Kal
DECLARE_DO_FUN( do_write_nameprofiles);	// Kal
DECLARE_DO_FUN( do_shadow			);	// Ker
DECLARE_DO_FUN( do_cannibalize		);	// Ker
DECLARE_DO_FUN( do_fork				);	// Ker
DECLARE_DO_FUN( do_autowraptells	);  // Kal
DECLARE_DO_FUN( do_pracsystester	);	// Kal
DECLARE_DO_FUN( do_place			);	// Jarren :)
DECLARE_DO_FUN( do_boon				);	// Kal
DECLARE_DO_FUN( do_inroom			);	// Kal
DECLARE_DO_FUN( do_trapset			);	// Ker
DECLARE_DO_FUN( do_trapremove		);	// Ker
DECLARE_DO_FUN( do_trapshow			);	// Ker
DECLARE_DO_FUN( do_inroom			);	// Kal
DECLARE_DO_FUN( do_clear_createcount);	// Kal
DECLARE_DO_FUN( do_nocharm			);	// Kal
DECLARE_DO_FUN( do_search			);	// Ker
DECLARE_DO_FUN( do_disarm_trap		);	// Ker
DECLARE_DO_FUN( do_declineooc		);	// Ker

// Feb 00
DECLARE_DO_FUN( do_pknote			);	// Kal
DECLARE_DO_FUN( do_ifhelp			);	// Kal
DECLARE_DO_FUN( do_mudprogqueue		);	// Kal
DECLARE_DO_FUN( do_immwiznet		);	// Kal
DECLARE_DO_FUN( do_wiznet2			);	// Kal
DECLARE_DO_FUN( do_wiznet3			);	// Kal
DECLARE_DO_FUN( do_immwiznetc		);	// Kal
DECLARE_DO_FUN( do_wiznetc			);	// Kal
DECLARE_DO_FUN( do_wiznet2c			);	// Kal
DECLARE_DO_FUN( do_wiznet3c			);	// Kal
DECLARE_DO_FUN( do_permrawcolour	);	// Kal
DECLARE_DO_FUN( do_gameedit			);	// Kal
DECLARE_DO_FUN( do_load_gamesettings);	// Kal
DECLARE_DO_FUN( do_save_gamesettings);	// Kal
DECLARE_DO_FUN( do_noheromsg		);	// Ker

// March 00
DECLARE_DO_FUN( do_attune			);	// Ker
DECLARE_DO_FUN( do_knock			);	// Ker
DECLARE_DO_FUN( do_needlepoint		);	// Ker
DECLARE_DO_FUN( do_rptell			);	// Ker
DECLARE_DO_FUN( do_rpreply			);	// Ker
DECLARE_DO_FUN( do_remort			);	// Kal
DECLARE_DO_FUN( do_checkbalance		);	// Ker
DECLARE_DO_FUN( do_trollish			);	// Ker + Tib
DECLARE_DO_FUN( do_faerie			);	// Ker + Tib
DECLARE_DO_FUN( do_letter			);	// Kal

// April 00
DECLARE_DO_FUN( do_mythostalk		);	// Ylerin
DECLARE_DO_FUN( do_flourish			);	// Kerenos
DECLARE_DO_FUN( do_social_import	);	// Kal
DECLARE_DO_FUN( do_socialedit		);	// Kal
DECLARE_DO_FUN( do_herbedit			);	// Kerenos
DECLARE_DO_FUN( do_saveherbs		);	// Kerenos
DECLARE_DO_FUN( do_checkcode		);	// Kal
DECLARE_DO_FUN( do_rsay				);	// Kal
DECLARE_DO_FUN( do_mixedit			);	// Ker
DECLARE_DO_FUN( do_savemixdb		);	// Ker
DECLARE_DO_FUN( do_bmlist			);	// Kal
DECLARE_DO_FUN( do_newbietalk		);	// Ker
DECLARE_DO_FUN( do_sing				);	// Ker
DECLARE_DO_FUN( do_apply			);	// Ker
DECLARE_DO_FUN( do_checknewbie		);	// Ker
DECLARE_DO_FUN( do_autopkassist		);	// Kal
DECLARE_DO_FUN( do_bardify			);	// Ker
DECLARE_DO_FUN( do_herblore			);	// Ker
DECLARE_DO_FUN( do_herbalism		);	// Ker

// May 00
DECLARE_DO_FUN( do_mixshow			);	// Ker
DECLARE_DO_FUN( do_autobalance		);	// Kal
DECLARE_DO_FUN( do_save_autostat_files); // Kal
DECLARE_DO_FUN( do_load_autostat_files); // Kal
DECLARE_DO_FUN( do_mremish			); // Kal
DECLARE_DO_FUN( do_introduce		); // Kal
DECLARE_DO_FUN( do_nonames			); // Kal
DECLARE_DO_FUN( do_autoanswer		); // Kal
DECLARE_DO_FUN( do_askname			); // Kal

// June 00
DECLARE_DO_FUN( do_alignstats		); // Kal
DECLARE_DO_FUN( do_collect			); // Tib

// July 00
DECLARE_DO_FUN( do_delmoot      );	// Jar
DECLARE_DO_FUN( do_addmoot      );	// Jar
DECLARE_DO_FUN( do_listmoot     );	// Jar
DECLARE_DO_FUN( do_offmoot      );  // Jar

// Aug 00
DECLARE_DO_FUN( do_worship		);  // Tib
DECLARE_DO_FUN( do_setdeity		);  // Tib
DECLARE_DO_FUN( do_webpass		);  // Kal

// September 00
DECLARE_DO_FUN( do_clanedit		);	// Tib
DECLARE_DO_FUN( do_saveclans	);	// Tib
DECLARE_DO_FUN( do_loadclans	);	// Kal
DECLARE_DO_FUN( do_gamesettings );	// Kal
DECLARE_DO_FUN( do_colour		);	// Kal

// October 00
DECLARE_DO_FUN( do_outfit		);	// Kal, I just know some muds are going to want it
DECLARE_DO_FUN(	do_playerlock	);	// Kal
DECLARE_DO_FUN( do_newbielock	);	// Kal

// November 00
DECLARE_DO_FUN( do_hide_hidden_areas);	// Kal

// Dec 00
DECLARE_DO_FUN( do_cskinfo);	// Kal

// Jan 01
DECLARE_DO_FUN( do_textsearch); // Kal
DECLARE_DO_FUN( do_scalemxp);	// Kal

// Feb 01
DECLARE_DO_FUN( do_socshow);		// Kal
DECLARE_DO_FUN( do_wiznetdefault);	// Kal
DECLARE_DO_FUN( do_bmllist);		// Kal
DECLARE_DO_FUN( do_bmvlist);		// Kal
DECLARE_DO_FUN( do_aslist);			// Kal
DECLARE_DO_FUN( do_bollist);		// Kal
DECLARE_DO_FUN( do_bovlist);		// Kal
DECLARE_DO_FUN( do_name_b4short);	// Kal

// Mar 01
DECLARE_DO_FUN( do_skillgroupedit);		// Kal
DECLARE_DO_FUN( do_write_skillgroups);	// Kal
DECLARE_DO_FUN( do_show);				// Kal
DECLARE_DO_FUN( do_mccp);				// Kal
DECLARE_DO_FUN( do_forget);				// Kal

// Apr 01
DECLARE_DO_FUN( do_mxp);		// Kal
DECLARE_DO_FUN( do_helpcat);	// Kal

// May 01
DECLARE_DO_FUN( do_resolve);	// Kal

// June 01
DECLARE_DO_FUN( do_helpprev);			// Kal
DECLARE_DO_FUN( do_helpnext);			// Kal
DECLARE_DO_FUN( do_helphistory);		// Kal
DECLARE_DO_FUN( do_visualdebug);		// Kal
DECLARE_DO_FUN( do_saymote);			// Kal
DECLARE_DO_FUN( do_autosaymote);		// Kal
DECLARE_DO_FUN( do_autosaycolourcodes);	// Kal
DECLARE_DO_FUN( do_name_only_4known);	// Kal
DECLARE_DO_FUN( do_charinfo);			// Kal
DECLARE_DO_FUN( do_mxpreflect);			// Kal
DECLARE_DO_FUN( do_autodamage);			// Kal

// July 01
DECLARE_DO_FUN( do_save_corpses);		// Kal
DECLARE_DO_FUN( do_load_corpses);		// Kal

// August 01
DECLARE_DO_FUN( do_raceedit);			// Kal
DECLARE_DO_FUN( do_raceinfo);			// Kal
DECLARE_DO_FUN( do_snote);				// Kal
DECLARE_DO_FUN( do_ic);					// Kal
DECLARE_DO_FUN( do_title);				// Kal
DECLARE_DO_FUN( do_immtitle);			// Kal
DECLARE_DO_FUN( do_report);				// Kal
DECLARE_DO_FUN( do_whoformat);			// Kal

// September 01
DECLARE_DO_FUN( do_checkbug);			// Daos
DECLARE_DO_FUN( do_grats);				// Kal
DECLARE_DO_FUN( do_flame);				// Kal
DECLARE_DO_FUN( do_immget);				// Balo
DECLARE_DO_FUN( do_mxpinfo);			// Kal
DECLARE_DO_FUN( do_areasalpha);			// Kal

// October 01
DECLARE_DO_FUN( do_switchprefix);		// Kal

// November 01
DECLARE_DO_FUN( do_clantalk);			// Kal

// December 01
DECLARE_DO_FUN( do_pload);				// Kal
DECLARE_DO_FUN( do_gprompt);			// Kal
DECLARE_DO_FUN( do_checkclanbank);		// Kal
DECLARE_DO_FUN( do_globalsocial);		// Kal/Balo
DECLARE_DO_FUN( do_qpoints);			// Kal/Balo
DECLARE_DO_FUN( do_pose);				// Kal

// Feb 02
DECLARE_DO_FUN( do_mudclientstats);		// Kal

// Mar 02
//DECLARE_DO_FUN( do_manual_colour_convert);	// Kal
DECLARE_DO_FUN( do_lockers);			// Kal
DECLARE_DO_FUN( do_continue);			// Kal
DECLARE_DO_FUN( do_wizlistedit);		// Kal
DECLARE_DO_FUN( do_circle);				// Meerclar/Kal
DECLARE_DO_FUN( do_jail);				// Balo/Kal
DECLARE_DO_FUN( do_release);			// Balo/Kal
DECLARE_DO_FUN( do_drag);				// Meerclar/Kal
DECLARE_DO_FUN( do_push);				// Meerclar/Kal

// Apr 02
DECLARE_DO_FUN(	do_dawnftp			);		// Kal
DECLARE_DO_FUN(	do_fullexits		);		// Kal
DECLARE_DO_FUN(	do_invitelist		);		// Kal
DECLARE_DO_FUN(	do_ownerlist		);		// Kal
DECLARE_DO_FUN(	do_detect_oldstyle_note_writing); // Kal

// June 02
DECLARE_DO_FUN(	do_wizgrantlist		);	// Kal

// July 02
DECLARE_DO_FUN(	do_nforce	);	// Kal
DECLARE_DO_FUN(	do_mforce	);	// Kal

// August 02
DECLARE_DO_FUN( do_chat	);	// Kal
DECLARE_DO_FUN( do_undeny);	// Kal

// September 02
DECLARE_DO_FUN( do_mixflask);	// Kal
DECLARE_DO_FUN( do_nomapblanks); // Kal

// March 03
DECLARE_DO_FUN( do_write_languages ) // Kal

// April 03
DECLARE_DO_FUN( do_langedit ) // Kal
DECLARE_DO_FUN( do_autowhoinvislogin ) // Kal
DECLARE_DO_FUN( do_autowizilogin ) // Kal
DECLARE_DO_FUN( do_replayroom ) // Kal

// Aug 03
DECLARE_DO_FUN( do_setichour ) // Kal
DECLARE_DO_FUN( do_battlelag ) // Kal

// September 03
DECLARE_DO_FUN( do_replaychannels ) // Kal

// Jan 04 
DECLARE_DO_FUN( do_autokeepalive ) // Kal
DECLARE_DO_FUN( do_hlook) // Ixliam
DECLARE_DO_FUN( do_history) // Ixliam
DECLARE_DO_FUN( do_charhistory) // Kal
DECLARE_DO_FUN( do_classinfo) // Kal

// Apr 04
DECLARE_DO_FUN( do_mine) // Kal
DECLARE_DO_FUN( do_mp_trigger_in_room) // Kal

// May 04
DECLARE_DO_FUN( do_apurge) // Tristan

// August 04

// October 05
DECLARE_DO_FUN( do_dumpeventqueue	); // Kal

// March 09
DECLARE_DO_FUN( do_topwealth	); // Kal
DECLARE_DO_FUN( do_laston_update_from_disk ); // Kal
