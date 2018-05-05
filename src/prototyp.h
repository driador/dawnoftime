/**************************************************************************/
// prototyp.h - All global function prototypes in here
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
#ifndef PROTOTYPE_H
#define PROTOTYPE_H

int unlink();
int system();

struct letgain_data;

//***************************************************
//*  FUNCTION PROTOTYPES
//***************************************************

/**************************************************************************/
// act_comm.cpp

bool	flush_char_outbuffer(char_data *ch);
		// forces a flush of the output buffer of a single player
		// manually
bool	is_same_group( char_data *ach, char_data *bch );
void	add_follower( char_data *ch, char_data *master );
void	broadcast_moot(void);
void	check_sex( char_data *ch);
void	die_follower( char_data *ch );
void	do_broadcast_announce(char_data *ch, char *message);
void	nuke_pets( char_data *ch );
void	stop_follower( char_data *ch );
DECLARE_DO_FUN(do_say);

/**************************************************************************/
// act_enter.cpp
RID  *get_random_room(char_data *ch);

/**************************************************************************/
// act_info.cpp
DECLARE_DO_FUN(do_huh);
void    set_title( char_data *ch, char *title );
char	*format_obj_to_char( OBJ_DATA *obj, char_data *ch, bool fShort );

/**************************************************************************/
// act_move.cpp
void    move_char( char_data *ch, int door, bool follow );
int		get_recallvnum(char_data *ch, bool *using_pendant);

/**************************************************************************/
// act_obj.cpp
bool	can_loot(char_data *ch, OBJ_DATA *obj);
void	get_obj( char_data *ch, OBJ_DATA *obj, OBJ_DATA *container );
void	wear_obj(char_data *ch, OBJ_DATA *obj, bool fReplace, bool hold );

/**************************************************************************/
// act_wiz.cpp
void	hotreboot_recover ( void );
void	wiznet( const char *string, char_data *ch, OBJ_DATA *obj,
					long flag, long flag_skip, int min_level );
DECLARE_DO_FUN( do_at );
ROOM_INDEX_DATA *find_location( char_data *ch, char *arg );
ROOM_INDEX_DATA *find_location_player_prority( char_data *ch, char *arg );


/**************************************************************************/
// affects.cpp
void affects_update( void );

/**************************************************************************/
// alias.cpp
char * substitute_alias( connection_data *d, char *input );

/**************************************************************************/
// ban.cpp
bool	check_connection_ban( connection_data *c );

/**************************************************************************/
// bit.cpp
char	*fwrite_wordflag( const struct flag_type *flag_table, 
					  int bits, const char * heading, FILE *fp);
int		fread_wordflag( const struct flag_type *flag_table, FILE *fp);
void	free_affect( AFFECT_DATA *af );
void	free_extra_descr( EXTRA_DESCR_DATA *ed );
int		flag_lookup (const char *name, const struct flag_type *flag_table);
affectprofile_type	*affectprofile_lookup( const char *name );
classgroup_type		*classgroup_lookup( const char* name );
int flag_value(const struct flag_type *flag_table,char *argument);
char *flag_string(const struct flag_type *flag_table,int bits);

/**************************************************************************/
// base64.cpp
char * decodeBase64(char *coded_with_linebreaks);
char * encodeBase64(char *plaintext, int len);
char * url_encode_post_data(char *postdata);

/**************************************************************************/
// com_time.cpp
DECLARE_DO_FUN(do_compile_time);

/**************************************************************************/
// comedit.cpp
char *com_category_indexlookup(int index);

/**************************************************************************/
// comm.cpp
char *	get_current_working_directory();
void	update_currenttime(void);
int		colour( char type, char_data *ch, char *string );
int		irc_colour( char type, char_data *ch, char *string );
void	act( const char *format, char_data *ch,
			 const void *arg1, const void *arg2, ACTTO_TYPE type );
void act_with_autodam_to_char(const char *format, char_data *ch, const void *arg1, 
		const void *arg2, int damage_result); // always TO_CHAR
char *	act_new( const char *format, char_data *ch, const void *arg1,
				 const void *arg2, ACTTO_TYPE type, int min_pos);
void	do_abort( void );
void	connection_close( connection_data *dclose );
void	make_corefile( void );
void	show_string( connection_data *d, char *input );
void	write_shutdown_file(char_data *ch);
void	write_to_buffer( connection_data *d, const char *txt, int length );
void	write_to_buffer_bw( connection_data *d, const char *txt, int length );
void	exit_clean(int exitcode, char *function, char *message); // exiting cleanly
void	exit_error(int exitcode, char *function, char *message); // exiting due to some error condition
void	sleep_seconds(int seconds);

/**************************************************************************/
// d2magsys.cpp
void	on_level_learn( char_data *ch );

/**************************************************************************/
// db.cpp
bool	str_cmp( const char *astr, const char *bstr );
bool	str_infix( const char *substring, const char *contained_within );
bool	str_prefix( const char *prefix, const char *fullstring);
bool	str_suffix( const char *astr, const char *bstr );
char	*capitalize( const char *str );
char	fread_letter( FILE *fp );
char	*fread_string( FILE *fp );
char	*fread_string_eol( FILE *fp );
char	*fread_word( FILE *fp );
char	*get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed );
char	*icapitalize( const char *str );
char	*ltrim_string( const char *str );
char	*rtrim_string( const char *str );
char	*flags_print( int flag );
int		flags_read(char *flagtext);
char	*str_dup( const char *str );
void	free_speedwalk( connection_data *d );
int		dice( int number, int size );
int		fread_number( FILE *fp );
int		interpolate( int level, int value_00, int value_32 );
int		number_door( void );
int		number_fuzzy( int number );
int		number_percent( void );
int		number_range( int from, int to );
long	flag_convert( char letter);
long	fread_flag( FILE *fp );
long	number_mm( void );
void	*alloc_mem_old( int sMem );
void	*alloc_perm( int sMem );
void	append_file( char_data *ch, char *file, char *str );
void	area_update( void );
void	boot_db( void );
void	bug( const char *str);
void	clone_mobile( char_data *parent, char_data *clone );
void	clone_object( OBJ_DATA *parent, OBJ_DATA *clone );
void	fread_to_eol( FILE *fp );
void	free_mem_old( void *pMem, int sMem );
void	free_string( char *pstr );
void	fulltime_log_string( const char *str );
void	hide_tilde( char *str );
void	load_disabled( void );
void	log_string( const char *str );
void	log_hold_till_commandline_options_parsed();
void	log_release_held_logs();
void	log_string_core(const char *str );
void	dawnlog_write_index(char *str);
void	log_bar(); // draw a line of *'s in the logs on a line without the time
void	log_note(const char *str); // logs the note with *'s above and below
void    reset_area( AREA_DATA * pArea );
void    reset_room( ROOM_INDEX_DATA *pRoom, bool unconditional );
void	save_disabled( void );
void	max_count_ip_calc();
void	show_tilde( char *str );
void	smash_tilde( char *str );
void	sort_arealists( void );
void	tail_chain( void );
CD		*create_mobile( MOB_INDEX_DATA *pMobIndex, int level );
MID		*get_mob_index( int vnum );
MPC		*get_mprog_index( int vnum );
OD		*create_object( OBJ_INDEX_DATA *pObjIndex);
OID		*get_obj_index( int vnum );
RID		*get_room_index( vn_int vnum );
OD		*get_random_obj( char_data* mob );

/**************************************************************************/
// dawnlib.cpp
int str_len( const char *s );
int is_alpha( char c );
int is_alnum( char c );
int is_ascii( char c );
int is_digit( char c );
int is_lower( char c );
int is_print( char c );
int is_space( char c );
int is_upper( char c );
void manage_dynamic_buffer(char **result, int max_length);
char *trim_trailing_carriage_return_line_feed(char *str);

/**************************************************************************/
// dynamics.cpp
void addlist(name_linkedlist_type **list,char *name, int tag, bool duplicates, bool reversed);

/**************************************************************************/
// effect.cpp
void	acid_effect(void *vo, int level, int dam, int target );
void	cold_effect(void *vo, int level, int dam, int target );
void	fire_effect(void *vo, int level, int dam, int target );
void	poison_effect(void *vo, int level, int dam, int target );
void	shock_effect(void *vo, int level, int dam, int target );


/**************************************************************************/
// exitlist.cpp
char	*area_name(AREA_DATA *pArea);

/**************************************************************************/
// fight.cpp
bool	can_initiate_combat( char_data *attacker, char_data *victim, int type );
int		damage( char_data *ch, char_data *victim, int dam,
				int dt, int clss, bool show );
int		damage_spell( char_data *ch, char_data *victim, int dam,
					  int dt, int clss, bool show );
bool	is_safe(char_data *ch, char_data *victim );
bool	is_safe_spell(char_data *ch, char_data *victim, bool area );
bool	mobRememberCH( char_data *ch, char_data *victim );
void	kill_char( char_data *victim, char_data *ch );
void	mobRememberClear( char_data *ch );
void	mobRememberSet( char_data *ch, char_data *victim );
void    multi_hit( char_data *ch, char_data *victim, int dt );
void	one_hit( char_data *ch, char_data *victim, int dt, bool secondary );
void    raw_kill( char_data *ch, char_data *killer );
void    stop_fighting( char_data *ch, bool fBoth );
void    update_pos( char_data *victim );
void    violence_update( void );

/**************************************************************************/
// handler.cpp
bool player_on_rooms_invite_list(char_data *ch, ROOM_INDEX_DATA *room);
const flag_type *affect_get_bitvector_table_for_where(int where);
bool    can_drop_obj( char_data *ch, OBJ_DATA *obj );
bool    can_see( char_data *looker, char_data *victim );
bool    can_see_obj( char_data *looker, OBJ_DATA *obj );
bool    can_see_room( char_data *looker, ROOM_INDEX_DATA *pRoomIndex );
bool    can_see_who( char_data *looker, char_data *victim );
bool	check_defrauding_argument(char_data *ch, char *argument );
bool	has_colour(const char *s);
bool	has_space(const char *s);
bool	has_whitespace(const char *s);
bool    is_affected( char_data *ch, int sn );
bool	is_clan( char_data *ch );
bool	is_exact_name( const char *str, const char *namelist );
bool	is_name( const char *str, const char *namelist );
bool	is_name_infix( const char *str, const char *namelist );
bool    is_room_owner( char_data *ch, ROOM_INDEX_DATA *room );
bool	is_same_clan( char_data *ch, char_data *victim );
bool    room_is_dark( ROOM_INDEX_DATA *pRoomIndex );
bool    room_is_private( ROOM_INDEX_DATA *pRoomIndex );
bool	is_room_private_to_char( ROOM_INDEX_DATA *pRoomIndex, char_data *ch );
char	*act_bit_name( int act_flags );
char	*act2_bit_name( int act_flags );
char	*affect_bit_name( int vector );
char	*affect2_bit_name( int vector );
char	*affect3_bit_name( int vector );
char	*affect_loc_name( int location );
char	*area_fname( AREA_DATA *pArea);
char	*comm_bit_name( int comm_flags );
char	*cont_bit_name( int cont_flags);
char	*extra_bit_name( int extra_flags );
char	*extra2_bit_name( int extra_flags );
char	*form_bit_name( int form_flags );
char	*imm_bit_name( int imm_flags );
char    *item_name( int item_type );
char	*item_type_name( OBJ_DATA *obj );
char	*LONGPERS( char_data *ch, char_data *looker);
char	*lowercase( const char *str );
char	*off_bit_name( int off_flags );
char	*part_bit_name( int part_flags );
char	*percent_colour_code(sh_int val); // Kal
char	*percent_colour_codebar(void); // Kal
char	*PERS( char_data *ch, char_data *looker);
char	*preference_word(PREFERENCE_TYPE pt);
char	*room_flags_bit_name(int room_flags);
char	*shortdate(time_t *tm);
char	*shorttime(time_t *tm);
char	*str_width(const char *s, int width, bool cut_if_too_long);
char	*to_affect_string( AFFECT_DATA *paf, int objects_level, bool report_unknown_where_value_errors);
char	*underscore_word(char *word);
char	*uppercase( const char *str );
char	*weapon_bit_name ( int bits);
char    *weapon_name( int weapon_Type  );
char	*wear_bit_name   ( int wear_flags );
char	*YOU_PERS( char_data *ch, char_data *looker);
int     apply_ac( OBJ_DATA *obj, int iWear, int type );
int		c_str_len(const char *s);
int     can_carry_n( char_data *ch );
int     can_carry_w( char_data *ch );
int     check_immune( char_data *ch, int dam_type );
int		count_affected_by_base_spell( char_data *ch, int parentspell_sn );
int     count_obj_list( OBJ_INDEX_DATA *obj, OBJ_DATA *list );
int     count_users( OBJ_DATA *obj );
int		count_char( const char *buffer, char character);
int     get_max_train( char_data *ch, int stat );
int		get_obj_number( OBJ_DATA *obj );
int		get_obj_weight( OBJ_DATA *obj );
int     get_skill       ( char_data *ch, int sn );
int		get_true_weight( OBJ_DATA *obj );
int     get_trust( char_data *ch );
int		get_next_uid();
int		get_uid(const char *); // text input is in form #uid 
int     get_weapon_skill( char_data *ch, int sn );
int     get_weapon_sn( char_data *ch );
int		num_enemies( char_data *ch );
int     weapontype( const char *name );
void    affect_check( char_data *ch, int where, int vector );
void	affects_from_template_to_obj(OBJ_DATA *obj);
void	affect_fly_update( char_data *ch );
void    affect_join( char_data *ch, AFFECT_DATA *paf );
void    affect_remove( char_data *ch, AFFECT_DATA *paf );
void    affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf );
void	affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ); 
void    affect_strip ( char_data *ch, int sn );
void    affect_to_char( char_data *ch, AFFECT_DATA *paf );
void    affect_to_obj( OBJ_DATA *obj, AFFECT_DATA *paf );
void	affect_to_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf );
void	affect_to_skill( char_data *ch, int sn, int amount );
void	affect_parentspellfunc_strip( char_data *ch, int sn );
void	append_datetime_ch_to_file( char_data *, const char *file, const char *str );
void	append_datetimestring_to_file( const char *, const char *);
void	append_logentry_to_file( char_data *, const char *file, const char *str );
void	append_newbie_support_log  ( char_data *, char *);
void	append_playerlog           ( char_data *, char *);
void	append_string_to_file      ( const char *file, const char *str, bool newline);
void	append_timestring_to_file  ( const char *, const char *);
void    bash_eq( char_data *ch, int chance );
void	center_to_char(char *argument, char_data *ch, int columns);
void	println_delayed_to_room(int seconds, room_index_data *room, char_data *all_but_this_person, const char *text);
void    char_from_room( char_data *ch );
void    char_to_room( char_data *ch, ROOM_INDEX_DATA *pRoomIndex );
void    deduct_cost     ( char_data *ch, int cost );
void    equip_char( char_data *ch, OBJ_DATA *obj, int iWear );
void    extract_char( char_data *ch, bool fPull );
void    extract_obj( OBJ_DATA *obj );
void	limit_mobile_wealth(char_data *mob);
void    obj_from_char( OBJ_DATA *obj );
void    obj_from_obj( OBJ_DATA *obj );
void    obj_from_room( OBJ_DATA *obj );
void    obj_to_char( OBJ_DATA *obj, char_data *ch );
void    obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to );
void    obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex );
void    reset_char( char_data *ch );
void    unequip_char( char_data *ch, OBJ_DATA *obj );
void	process_moblog(char_data *ch, const char *txt);
AD      *affect_find	( AFFECT_DATA *paf, int sn );
CD		*get_char_icworld( char_data *ch, char *argument );
CD		*get_char_room( char_data *ch, char *argument );
CD		*get_char_world( char_data *ch, char *argument );
CD		*get_pet_room( char_data *ch, char *argument );
CD		*get_whovis_player_world( char_data *ch, char *argument ); // called by tell
OD		*create_money( int gold, int silver );
OD		*get_eq_char( char_data *ch, int iWear );
OD		*get_obj_type( OBJ_INDEX_DATA *pObjIndexData );
OD		*get_obj_of_type_in_room( OBJ_INDEX_DATA *pObjIndex, ROOM_INDEX_DATA *room );
OD		*get_obj_list( char_data *ch, char *argument, OBJ_DATA *list );
OD		*get_obj_carry( char_data *ch, char *argument );
OD		*get_obj_wear( char_data *ch, char *argument );
OD		*get_obj_carry_for_looker( char_data *ch, char *argument, char_data *looker);
OD		*get_obj_here( char_data *ch, char *argument );
OD		*get_obj_world( char_data *ch, char *argument );
OD		*get_obj_token( char_data *ch, char *argument );
char	*convert24hourto12hour(int hour);
char	*autodamtext(char_data *ch, int damage_amount);

/**************************************************************************/
// interp.cpp
bool	is_number( const char *arg );
char	*one_argument( const char *argument, char *arg_first );
int		number_argument( char *argument, char *arg );
int		mult_argument( char *argument, char *arg);
void	interpret( char_data *ch, char *argument );
DECLARE_DO_FUN( do_olc		);
DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_alist	);
DECLARE_DO_FUN( do_vlist	);
DECLARE_DO_FUN( do_resets	);
DECLARE_DO_FUN( do_redit	);
DECLARE_DO_FUN( do_aedit	);
DECLARE_DO_FUN( do_medit	);
DECLARE_DO_FUN( do_oedit	);
DECLARE_DO_FUN( do_mpedit	);
DECLARE_DO_FUN( do_disable	);
DECLARE_DO_FUN( do_pdisable	); // Kalahn - Aug 97 
DECLARE_DO_FUN( do_penable	); // Kalahn - Aug 97 

/**************************************************************************/
// ispell.cpp
char	*get_ispell_line( char *word );
void	ispell_done(void);
void	ispell_init(void);
void	ispell_string( char_data *ch );

/**************************************************************************/
// language.cpp
void translate_language(language_data *language, bool display_language, char_data * speaker, 
						char_data * listener, const char *message, char *output);
language_data *language_exact_lookup(const char *name);
language_data *language_lookup(const char *name);
language_data *language_safe_lookup(const char *name);
language_data *language_lookup_by_id(int id);
language_data *language_safe_lookup_by_id(int id);
bool	language_dynamic_command(char_data *ch, char *command, char *argument);
void	language_init_gsn_and_unique_id();
void	languages_load_and_initialise();
void	languages_assign_gsn_values();

/**************************************************************************/
// laston.cpp
char	*short_timediff(time_t, time_t);
char	*timediff(time_t, time_t);
void	laston_close_all();
void	laston_logout( char_data *ch );
void	resort_top_roleplayers();
void	resort_top_wealth();

/**************************************************************************/
// lookup.cpp
int		coreclass_exact_lookup( const char *name );
int		coreclass_lookup( const char *name );
int		council_lookup( const char *name );
int		dir_lookup( char *dir );
int     item_lookup( const char *name );
int     liq_lookup( const char *name );
int		pcrace_lookup( const char *name );
DEITY_DATA *deity_lookup( char *name );
/**************************************************************************/
// magic.cpp
bool    check_dispel    ( int dis_level, char_data *victim, int sn );
bool    saves_spell     ( int level, char_data *victim, int dam_type );
int		find_spell( char_data *ch, const char *name, bool spellonly );
int		skill_exact_lookup( const char *name );
int     skill_lookup( const char *name );
void    obj_cast_spell( int sn, int level, char_data *ch, char_data *victim, OBJ_DATA *obj );

/**************************************************************************/
// mob_prog.cpp
bool	mp_exit_trigger( char_data *ch, int dir );
bool	mp_percent_trigger( char_data *mob, char_data *ch,            
							const void *arg1, const void *arg2, int type );
void	mp_act_trigger( char *argument, char_data *mob, char_data *ch,
						const void *arg1, const void *arg2, int type );
bool	mp_cmd_trigger( char *argument, char_data *mob, char_data *ch,
						const void *arg1, const void *arg2, int type );
void	mp_bribe_trigger( char_data *mob, char_data *ch, int amount );
bool	mp_would_run_give_trigger( char_data *mob, char_data *ch, OBJ_DATA *obj );
void	mp_give_trigger( char_data *mob, char_data *ch, OBJ_DATA *obj );
void	mp_greet_trigger( char_data *ch );
void	mp_hour_trigger( char_data *ch );
void	mp_hprct_trigger( char_data *mob, char_data *ch );
bool	mp_premove_trigger( char_data *ch, int vnum, int dir );
 
void	program_flow( MUDPROG_TRIGGER_LIST *program, char_data *mob, char_data *ch,
					  const void *arg1, const void *arg2 );

bool oprog_execute_if_appropriate(obj_data *obj, char_data * ch, int otrig);
MUDPROG_TRIGGER_LIST * dup_mudprog_list(MUDPROG_TRIGGER_LIST * mplist);
/**************************************************************************/
// mp_cmds.cpp
void	mp_interpret   ( char_data *ch, char *argument );

/**************************************************************************/
// mudftp.cpp
bool	ftp_push(connection_data *d);

/**************************************************************************/
// noble.cpp
void	resolve_moot(void);

/**************************************************************************/
// notes.cpp
char	*get_notetype( int index );
char	*note_format_string( char * input); // note: input strings must be str_dup'ed and are freed, with the result str_duped
char	*note_format_string_width( char *input, int width, bool returns, bool trailing_newline); // note: input strings must be str_dup'ed and are freed, with the result str_duped
void	autonote(int type, const char *sender, const char *subject, const char *to, const char *text, bool reformat);

/**************************************************************************/
// olc.cpp
bool	run_olc_editor( connection_data *d );
char	*olc_ed_name( char_data *ch );
char	*olc_ed_vnum( char_data *ch );

/**************************************************************************/
// olc_save.cpp
DECLARE_DO_FUN(do_hsave);
char	*fix_string( const char *str );

/**************************************************************************/
// pipe.cpp
char	*get_piperesult( char *cmd );

/**************************************************************************/
// pload.cpp
char_data *pload_load_character(char_data *ch, char *name_of_character_to_load, bool return_character_if_already_in_the_game);
bool pload_unload_character(char_data *ch, char_data *character_to_unload);

/**************************************************************************/
// recycle.cpp
void	free_affect(AFFECT_DATA *af );
void	free_extra_descr(EXTRA_DESCR_DATA *ed );

/**************************************************************************/
// resolve.cpp
void resolver_send_data( const char * buf);
void resolverlocal_queue_command(const char *line);
void resolverlocal_execute_queued_commands();

/**************************************************************************/
// save.cpp
bool    load_char_obj( connection_data *d, char *name );
char	*percent_colour_code(sh_int val); // Kal
char	*percent_colour_codebar(void); // Kal
char	*pfilename(char *name, PFILE_TYPE ptype);
char	*pfile_filename(char *name);// returns the <lowercasefirstname>.plr
void	do_save_finger( char_data *ch );
void    save_char_obj( char_data *ch );
PFILE_TYPE get_pfiletype(char_data *ch);
PFILE_TYPE find_pfiletype(const char *name);
void fwrite_obj( obj_data *obj, FILE *fp, int iNest, char *heading );
obj_data * fread_obj( FILE *fp, const char *filename );

/**************************************************************************/
// saymote.cpp
void saymote( language_data *language, char_data *ch, char *argument, int sayflags);

/**************************************************************************/
// skills.cpp
bool	parse_gen_groups( char_data *ch,char *argument );
int     exp_per_level( char_data *ch, int points );
int     skillgroup_exact_lookup(const char *name);
int     skillgroup_lookup(const char *name);
void    check_improve( char_data *ch, int sn, bool success, int multiplier );
void    gn_add( char_data *ch, int gn);
void    gn_remove( char_data *ch, int gn);
void    group_add( char_data *ch, const char *name, bool deduct, int percent);
void    group_remove( char_data *ch, const char *name);
void	list_group_costs( char_data *ch );
void    list_group_known( char_data *ch );

/**************************************************************************/
// special.cpp
char	*ospec_name( OSPEC_FUN *function );
char	*spec_name( SPEC_FUN *function );
char	*spec_string( SPEC_FUN *fun );
OSF		*ospec_lookup( const char *name );
SF		*spec_lookup( const char *name );




/**************************************************************************/
// string.cpp
char	*first_arg( char *argument, char *arg_first, bool force_lowercase);
char	*format_string( char *oldstring /*, bool fSpace */ );
char	*string_proper( char * argument );
char	*string_replace( char * orig, char * old, char * newstr ); // assumes str_dup input
char	*string_replace_all( char * orig, char * old, char * newstr ); // assumes str_dup input
void    string_add( char_data *ch, char *argument );
void    string_append( char_data *ch, char **pString );
char	*string_remove_name(char *str, char *name);

/**************************************************************************/
// support.cpp
bool codehelp( char_data *ch, char *keyword, int report_unfound_flags);

/**************************************************************************/
// update.cpp
int		get_magecastmod(void);
void    advance_level( char_data *ch );
void	check_perm_damage( char_data *ch );
void	do_heroxp( char_data *ch, int xp );
void	drop_level( char_data *ch );
void    gain_exp( char_data *ch, int gain );
void    gain_condition( char_data *ch, int iCond, int value );
void    update_handler( void );

//***************************************************
//*  END OF FUNCTION PROTOTYPES
//***************************************************

#ifdef WIN32
	bool file_exists(const char * filename);
	bool file_existsf(const char * fmt, ...);
	char *format_titlebar(const char *text);
	char *format_titlebarf(const char *fmt, ...);
	char *creation_titlebar(char *fmt, ...);
	void broadcast(char_data *except, const char * fmt, ...);
	void bugf (const char * fmt, ...); 
	void boundsbug(const char * fmt, ...);
	void mpbug(const char * mudprog_bug_text);
	void mpbugf(const char * fmt, ...);
	void centerf_to_char( char_data *ch, int cols, const char *fmt, ...);
	void logf (const char * fmt, ...);
	void log_notef(const char * fmt, ...);
	void pkill_broadcast(const char * fmt, ...);
	void multilog_alertf(char_data *ch, const char * fmt, ...);
	void info_broadcast(char_data *ch, const char * fmt, ...);
#else
	bool file_exists(const char * filename);
	bool file_existsf(const char * fmt, ...) __attribute__ ((format(printf,1,2)));
	char *format_titlebar(const char *text);
	char *format_titlebarf(const char *fmt, ...) __attribute__ ((format(printf,1,2)));
	char *creation_titlebar(char *fmt, ...)__attribute__ ((format(printf,1,2)));
	void broadcast(char_data *except, const char * fmt, ...) __attribute__ ((format(printf,2,3)));
	void bugf(const char * fmt, ...) __attribute__ ((format(printf,1,2)));
	void boundsbug(const char * fmt, ...) __attribute__ ((format(printf,1,2)));
	void mpbug(const char * mudprog_bug_text);
	void mpbugf(const char * fmt, ...) __attribute__ ((format(printf,1,2)));
	void centerf_to_char( char_data *ch, int cols, const char *fmt, ...) __attribute__ ((format(printf, 3,4)));
	void logf(const char * fmt, ...) __attribute__ ((format(printf,1,2)));
	void log_notef(const char * fmt, ...) __attribute__ ((format(printf,1,2)));
	void pkill_broadcast(const char * fmt, ...) __attribute__ ((format(printf,1,2)));
	void multilog_alertf(char_data *ch, const char * fmt, ...) __attribute__ ((format(printf,2,3)));
	void info_broadcast(char_data *ch, const char * fmt, ...) __attribute__ ((format(printf,2,3)));
#endif

#ifdef WIN32
	void	gettimeofday( struct timeval *tp, void *tzp );

	#ifdef unix
		#undef unix
	#endif
#endif

char *dot_crypt(const char *buf, const char *salt);
int mg_crypt_msg(char *input, char *encoded);

bool is_valid_password(const char *attempt, const char *password, connection_data *d);

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	OSF
#undef	AD

#endif // PROTOTYPE_H
