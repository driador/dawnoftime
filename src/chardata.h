/**************************************************************************/
// chardata.h - header for char_data class
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

#ifndef char_data_H
#define char_data_H

#ifndef __entity_H
	#error entity.h must be included before chardata.h is read in
#endif

struct pload_data;

enum PREVIOUS_ROOM_TYPE {PREVIOUS_ROOM_TYPE_UNKNOWN, PREVIOUS_ROOM_TYPE_IC, PREVIOUS_ROOM_TYPE_OOC};

class intro_data;

class char_data : public entity_data
{
public: entity_type get_entitytype() {return ENTITYTYPE_CH;};

public:
// member functions

    // output to character
    void print(const char *buf);
	void printbw(const char *buf);
    void printf(const char *fmt, ...)					__mftc_printf_1__;
    void printfbw(const char *fmt, ...)					__mftc_printf_1__;
    void println(const char *buf);
	void print_blank_lines(int blank_lines_to_print_to_char);
	void printlnbw(const char *buf);
    void printlnf(const char *fmt, ...)					__mftc_printf_1__;
    void printlnfbw(const char *fmt, ...)				__mftc_printf_1__;
    void wrap(const char *buf);
    void wrapf(const char *fmt, ...)					__mftc_printf_1__;
    void wrapln(const char *buf);
    void wraplnf(const char *fmt, ...)					__mftc_printf_1__;

    void titlebar(const char *header);
	void titlebarf(const char *fmt, ...)				__mftc_printf_1__;
    void olctitlebar(const char *header);
	void olctitlebarf(const char *fmt, ...)				__mftc_printf_1__;

    void print(int seconds, const char *buf);
    void printf(int seconds, const char *fmt, ...)		__mftc_printf_2__;
    void println(int seconds, const char *buf);
    void printlnf(int seconds, const char *fmt, ...)	__mftc_printf_2__;

	void bug_print(const char *txt);
	void bug_println(const char *buf);
	void bug_printlnf(const char *fmt, ...)				__mftc_printf_1__;

    void sendpage(const char *txt);

	int pdelay(); // the amount print*() text sent to the character is delayed by
	void set_pdelay(int seconds); // the amount print*() text sent to the character is delayed by
private:
	int m_pdelay; // the amount print*() text sent to the character is delayed by
public:
    // skills
    int get_skill(int sn); // returns the percentage of skill they have in it
    int get_display_skill(int sn); // return the percentage they have out of the max for the class
    int get_skill_level(int sn); // returns the level of the skill for the players class

	void hit_return_to_continue();

	vn_int vnum(); // override the virtual function in entity_data
	vn_int in_room_vnum(); // a safe way to get the vnum of a room
	void moving_from_ic_to_ooc();  // override baseclass virtual function
	void moving_from_ooc_or_load_to_ic();  // override baseclass virtual function
private:
    int npc_skill_level(int sn);
    int pc_skill_level(int sn);

public:
	int uid; // unique id
	DEITY_DATA* deity;
    char_data * next;
    char_data * next_in_room;
    char_data * next_player;
	char_data * next_who_list;
    char_data * mounted_on;
    char_data * ridden_by;
    char_data * master;
    char_data * leader;
    char_data * fighting;
    char_data * reply;
    char_data * anon_reply;
    char_data * retell;
    char_data * anon_retell;
    char_data * controlling; // who a pc is controlling
    char_data * pet;
    char_data * mprog_target;
    vn_int      mprog_remember_room_vnum; // used by premove at this stage
    MEM_DATA    *memory;
    SPEC_FUN    *spec_fun;
    GAMBLE_FUN  *gamble_fun;
    mob_index_data  *pIndexData;
    connection_data *desc;
    AFFECT_DATA *affected;
    note_data   *pnote;
    OBJ_DATA    *carrying;
    OBJ_DATA    *on;
    ROOM_INDEX_DATA *   in_room;
    ROOM_INDEX_DATA *   was_in_room;
    area_data   *zone;
    pc_data     *pcdata;
    GEN_DATA    *gen_data;
    long        player_id; // seconds since they created
    char *		remote_ip_copy;  // used to detect multilogging
	sh_int		host_validated;
    sh_int      version;
    sh_int      subversion;
    char *      short_descr;
    char *      long_descr;
	char *      gprompt;
    char *      prompt;
    char *      olcprompt;
    char *      prefix;
    duel_data * duels;
    sh_int      group;
	sh_int		helpgroup;  // the group number they will help
    CClanType	*clan;
    sh_int      clanrank;
	CClanType	*seeks;
    sh_int      pksafe;        /* 3 pk variables enable automatic pk enforcement */
    sh_int      pknorecall;
    sh_int      pkool;
    int         pkkills;
    int         pkdefeats;
    int         pknoquit;    
    sh_int      sex;
    sh_int      clss;
    sh_int      race;
    sh_int      level;
    sh_int      trust;
    int         played;
    int         lines;  // lines per page
    time_t      logon;
    int         timer;
    int         idle;
    int         wait;
    int         daze;
    int         hit;
    int         max_hit;
    int         mana;
    int         max_mana;
    int         move;
    int         max_move;
    long        gold;
    long        silver;
    int         exp;
    long        act;
    long        act2;
    long        dyn; // dynamic data - never saved
    long        comm;   
    long        wiznet[4]; // wiznet stuff 
	char *		wiznet_colour[4]; // wiznet colour coding
    long        imm_flags;
    long        res_flags;
    long        vuln_flags;
    unsigned char invis_level;
    unsigned char iwizi;
    unsigned char olcwizi;
    unsigned char owizi;
    unsigned char incog_level;
    long        affected_by;
    long        affected_by2;
    long        affected_by3;
    long        config;
    long        config2;
    sh_int      position;
    int			practice;
    int			train;
    int         carry_weight;
    int         carry_number;
    int         saving_throw;
    sh_int      alliance;
    sh_int      tendency;
    int         hitroll;
    int         damroll;
    int         armor[4];
    int         wimpy;
    // stats 
    sh_int      perm_stats[MAX_STATS];
    sh_int      potential_stats[MAX_STATS];
    sh_int      modifiers[MAX_STATS];
    
    // parts stuff 
    long        form;
    long        parts;
    sh_int      size;
    char *      material;
    // mobile stuff 
    long        off_flags;
    sh_int      damage[3];
    sh_int      dam_type;
    sh_int      start_pos;
    sh_int      default_pos;
    sh_int      mprog_delay;
    vn_int      temple;

    // critical stuff 
    int         bleeding;
    sh_int      will_die;
    sh_int      is_stunned;
    // language stuff 
    language_data *language;

    long bank;
    ROOM_INDEX_DATA *  last_ic_room;  // NULL = currently in ic rooms 
    
    // subdue system - kalahn - june 97
    bool no_xp, subdued;
    bool tethered;
    bool bucking;
    
    bool autologout; // set in char_update
	bool is_trying_sleep;
    
    int subdued_timer; // budget way to wake them 
    sh_int will;
    sh_int wildness;
    
    sh_int  state; /* determines if they are
                   * comatose (will die),
                   * unconscious,
                   * semiconscious,
                   * subdued,
                   * yielding,
                   * free 
				   */
    char_data *    subduer; // character who has subdue this character 
    int cautious_about_backstab; // counter which is how long before a mob
    // can be backstabbed for
    
    unsigned char last_colour, saved_colour[MAX_SAVED_COLOUR_ARRAY];
    unsigned char saved_colour_index;

    long    last_force;     // the tick this char was last forced to do
    // something - used to stop wandering within
    // 5 ticks of them being forced
    // ( = tick_counter on force)

    long        notenet; // notenet stuff - kalahn

    // Dynamic Spell info
    int         mirage;
    sh_int      mirage_hours;

    // MOB REMEMBER STUFF
    char_data  * mobmemory;

    // WEAPON SPECIALIZATION
    sh_int specialization;

	// STATIC VNUMS.
    vn_int      recall_room;				// Recall room for player.
	vn_int		recall_inn_room;			// Recall room for player's inn.

	// TIMERS.
	int			expire_recall_inn;			// Time the recall inn expires.

    char        saycolour;
    char        motecolour;

    sh_int      track_index;

    int     duel_challenged;
    int     duel_decline;
    int     duel_accept;
    int     duel_ignore;
    int     duel_bypass;
    int     duel_subdues_before_karn_loss;

	sh_int	remort;	// remort the player is currently on
	sh_int	beginning_remort; // if a number, the player is midstream of remorting

	sh_int	highest_level_to_do_damage;
	
	intro_data *know; // data containing who they know info
	unsigned short know_index;	// their index in the know database
	char	colour_prefix;	// character prefixing all colour codes

	pload_data *pload;	// data about the pload
	char_data *ploaded; // character which has been ploaded by this character
	PREVIOUS_ROOM_TYPE previous_room_type;

	long running_trigger; // set when a particular mtrigger is being run on the mob

private:
//	int mpqueue_count; // just a basic command for now
	bool mxp_enabled;

public: // mxp functions
	void mxp_send_init(); // used to send the mxp element tags we are going to use
	void record_replayroom_event(const char *txt); // record says and emotes etc
};

#endif

