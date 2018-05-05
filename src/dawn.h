/**************************************************************************/
// dawn.h - heaps of game types defines
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

// below line used for global varibles system
#define EXTERN extern

// Connected state for a channel - rest in nanny.h
#define CON_PLAYING                 0

/**************************************************************************/
#define NOTE_NOTE       0
#define NOTE_IDEA       1
#define NOTE_PENALTY    2
#define NOTE_NEWS       3
#define NOTE_CHANGES    4
#define NOTE_ANOTE		5
#define NOTE_INOTE		6
#define NOTE_MISC		7
#define NOTE_SNOTE		8
#define NOTE_PKNOTE		9

// where definitions 
#define WHERE_AFFECTS		0
#define WHERE_OBJEXTRA		1
#define WHERE_IMMUNE		2
#define WHERE_RESIST		3
#define WHERE_VULN			4
#define WHERE_WEAPON		5
#define WHERE_RESTRICT		6
#define WHERE_SKILLS		7
#define WHERE_AFFECTS2		8
#define WHERE_OBJEXTRA2		9
#define WHERE_OBJECTSPELL	10 // spell objects
#define WHERE_MODIFIER		11
#define WHERE_AFFECTS3		12

// bitvector flags used with object spell system
#define OBJSPELL_ACTIVE			(A)	// Currently affecting the player
#define OBJSPELL_IGNORE_LEVEL	(B)	// Will be cast on player regardless of level

// ACT bits for mobs - Used in #MOBILES.
#define ACT_IS_NPC          (A)     // Auto set for mobs    
#define ACT_DONT_WANDER		(B)     // Stays in one room    
#define ACT_SENTINEL        (ACT_DONT_WANDER) // The old name for the DONT_WANDER flag
#define ACT_SCAVENGER       (C)     // Picks up objects     
#define ACT_DOCILE          (D)     // Allows self to be mounted
#define ACT_NO_TAME         (E)     // is untamable 
#define ACT_AGGRESSIVE      (F)     // Attacks PC's         
#define ACT_STAY_AREA       (G)     // Won't leave area     
#define ACT_WIMPY           (H)
#define ACT_PET             (I)     // Auto set for pets    
#define ACT_TRAIN           (J)     // Can train PC's       
#define ACT_PRACTICE        (K)     // Can practice PC's    
#define ACT_MPIGN_QUESTER	(L)     // All mudprog triggers on mob ignore quester
#define ACT_MPIGN_NONQUESTER (M)     // All mudprog triggers on mob ignore nonquester
#define ACT_AMPHIBIAN		(N)
#define ACT_UNDEAD          (O)

#define ACT_CLERIC          (Q)
#define ACT_MAGE            (R)
#define ACT_THIEF           (S)
#define ACT_WARRIOR         (T)
#define ACT_NOALIGN         (U)
#define ACT_NOPURGE         (V)
#define ACT_OUTDOORS        (W)
#define ACT_MOBLOG			(X)
#define ACT_INDOORS         (Y)
#define ACT_NOAUTOSOCIAL    (Z)
#define ACT_IS_HEALER       (aa)
#define ACT_GAIN            (bb)
#define ACT_UPDATE_ALWAYS   (cc)
#define ACT_IS_CHANGER      (dd)
#define ACT_IS_UNSEEN       (ee)

// (A) is left free incase an ACT2_IS_NPC flag is needed
#define ACT2_NOHUNT			(B) // Not used at this stage
#define ACT2_ALLSKILLS		(C)
#define ACT2_AVOIDS_ALL_ATTACKS (D)
#define ACT2_NO_TENDENCY		(E)
#define ACT2_MUDPROG_RECURSION_ALLOWED (F)

// damage clsses 
#define DAM_NONE				0
#define DAM_BASH				1
#define DAM_PIERCE				2
#define DAM_SLASH				3
#define DAM_FIRE				4
#define DAM_COLD				5
#define DAM_LIGHTNING			6
#define DAM_ACID				7
#define DAM_POISON				8
#define DAM_NEGATIVE			9
#define DAM_HOLY				10
#define DAM_ENERGY				11
#define DAM_MENTAL				12
#define DAM_DISEASE 			13
#define DAM_DROWNING			14
#define DAM_LIGHT				15
#define DAM_OTHER				16
#define DAM_HARM				17
#define DAM_CHARM				18
#define DAM_SOUND				19
#define DAM_ILLUSION			20

// OFF bits for mobiles 
#define OFF_AREA_ATTACK 		(A)
#define OFF_BACKSTAB			(B)
#define OFF_BASH				(C)
#define OFF_BERSERK 			(D)
#define OFF_DISARM				(E)
#define OFF_DODGE				(F)
#define OFF_FADE				(G)
#define OFF_FAST				(H)
#define OFF_KICK				(I)
#define OFF_KICK_DIRT			(J)
#define OFF_PARRY				(K)
#define OFF_RESCUE				(L)
#define OFF_TAIL				(M)
#define OFF_TRIP				(N)
#define OFF_CRUSH				(O)
#define ASSIST_ALL				(P)
#define ASSIST_ALIGN			(Q)
#define ASSIST_RACE 			(R)
#define ASSIST_PLAYERS			(S)
#define ASSIST_GUARD			(T)
#define ASSIST_VNUM 			(U)
#define OFF_GORE				(V)
#define OFF_UPPERCUT			(W) // uncompleted
#define OFF_NECK_THRUST 		(X) // uncompleted
#define OFF_WEB 				(Y) // uncompleted
#define OFF_GAZE				(Z) // uncompleted - was V
#define OFF_CIRCLE				(aa)

// return values for check_imm
#define IS_NORMAL				0
#define IS_IMMUNE				1
#define IS_RESISTANT			2
#define IS_VULNERABLE			3

// IMM bits for mobs
#define IMM_SUMMON				(A)
#define IMM_CHARM				(B)
#define IMM_MAGIC				(C)
#define IMM_WEAPON				(D)
#define IMM_BASH				(E)
#define IMM_PIERCE				(F)
#define IMM_SLASH				(G)
#define IMM_FIRE				(H)
#define IMM_COLD				(I)
#define IMM_LIGHTNING			(J)
#define IMM_ACID				(K)
#define IMM_POISON				(L)
#define IMM_NEGATIVE			(M)
#define IMM_HOLY				(N)
#define IMM_ENERGY				(O)
#define IMM_MENTAL				(P)
#define IMM_DISEASE 			(Q)
#define IMM_DROWNING			(R)
#define IMM_LIGHT				(S)
#define IMM_SOUND				(T)
#define IMM_SLEEP				(U)
#define IMM_WOOD				(X)
#define IMM_SILVER				(Y)
#define IMM_IRON				(Z)
#define IMM_ILLUSION			(aa)
#define IMM_SCRY				(bb)
#define IMM_HUNGER				(cc) // Ixliam
#define	IMM_THIRST				(dd) // Ixliam
#define	IMM_FEAR				(ee) // Ixliam


// RES bits for mobs 
#define RES_SUMMON				(A)
#define RES_CHARM				(B)
#define RES_MAGIC				(C)
#define RES_WEAPON				(D)
#define RES_BASH				(E)
#define RES_PIERCE				(F)
#define RES_SLASH				(G)
#define RES_FIRE				(H)
#define RES_COLD				(I)
#define RES_LIGHTNING			(J)
#define RES_ACID				(K)
#define RES_POISON				(L)
#define RES_NEGATIVE			(M)
#define RES_HOLY				(N)
#define RES_ENERGY				(O)
#define RES_MENTAL				(P)
#define RES_DISEASE 			(Q)
#define RES_DROWNING			(R)
#define RES_LIGHT				(S)
#define RES_SOUND				(T)
#define RES_WOOD				(X)
#define RES_SILVER				(Y)
#define RES_IRON				(Z)
#define RES_ILLUSION			(aa)
#define RES_SCRY				(bb)

// VULN bits for mobs
#define VULN_SUMMON 			(A)
#define VULN_CHARM				(B)
#define VULN_MAGIC				(C)
#define VULN_WEAPON 			(D)
#define VULN_BASH				(E)
#define VULN_PIERCE 			(F)
#define VULN_SLASH				(G)
#define VULN_FIRE				(H)
#define VULN_COLD				(I)
#define VULN_LIGHTNING			(J)
#define VULN_ACID				(K)
#define VULN_POISON 			(L)
#define VULN_NEGATIVE			(M)
#define VULN_HOLY				(N)
#define VULN_ENERGY 			(O)
#define VULN_MENTAL 			(P)
#define VULN_DISEASE			(Q)
#define VULN_DROWNING			(R)
#define VULN_LIGHT				(S)
#define VULN_SOUND				(T)
#define VULN_WOOD				(X)
#define VULN_SILVER 			(Y)
#define VULN_IRON				(Z)
#define VULN_ILLUSION			(aa)
#define VULN_SCRY				(bb)

// body form
#define FORM_EDIBLE 			(A)
#define FORM_POISON 			(B)
#define FORM_MAGICAL			(C)
#define FORM_INSTANT_DECAY		(D)
#define FORM_OTHER				(E)  // defined by material bit

// actual form
#define FORM_ANIMAL 			(G)
#define FORM_SENTIENT			(H)
#define FORM_UNDEAD 			(I)
#define FORM_CONSTRUCT			(J)
#define FORM_MIST				(K)
#define FORM_INTANGIBLE 		(L)

#define FORM_BIPED				(M)
#define FORM_CENTAUR			(N)
#define FORM_INSECT 			(O)
#define FORM_SPIDER 			(P)
#define FORM_CRUSTACEAN 		(Q)
#define FORM_WORM				(R)
#define FORM_BLOB				(S)
#define FORM_MOUNTABLE			(T)

#define FORM_MAMMAL 			(V)
#define FORM_BIRD				(W)
#define FORM_REPTILE			(X)
#define FORM_SNAKE				(Y)
#define FORM_DRAGON 			(Z)
#define FORM_AMPHIBIAN			(aa)
#define FORM_FISH				(bb)
#define FORM_COLD_BLOOD 		(cc)

// body parts
#define PART_HEAD				(A)
#define PART_ARMS				(B)
#define PART_LEGS				(C)
#define PART_HEART				(D)
#define PART_BRAINS 			(E)
#define PART_GUTS				(F)
#define PART_HANDS				(G)
#define PART_FEET				(H)
#define PART_FINGERS			(I)
#define PART_EAR				(J)
#define PART_EYE				(K)
#define PART_LONG_TONGUE		(L)
#define PART_EYESTALKS			(M)
#define PART_TENTACLES			(N)
#define PART_FINS				(O)
#define PART_WINGS				(P)
#define PART_TAIL				(Q)
// for combat
#define PART_CLAWS				(U)
#define PART_FANGS				(V)
#define PART_HORNS				(W)
#define PART_SCALES 			(X)
#define PART_TUSKS				(Y)


// Bits for 'affected_by' - Used in #MOBILES.
#define AFF_BLIND				(A)
#define AFF_INVISIBLE			(B)
#define AFF_DETECT_EVIL 		(C)
#define AFF_DETECT_INVIS		(D)
#define AFF_DETECT_MAGIC		(E)
#define AFF_DETECT_HIDDEN		(F)
#define AFF_DETECT_GOOD 		(G)
#define AFF_SANCTUARY			(H)
#define AFF_FAERIE_FIRE 		(I)
#define AFF_INFRARED			(J)
#define AFF_CURSE				(K)
#define AFF_OTTERLUNGS			(L)
#define AFF_POISON				(M)
#define AFF_PROTECT_EVIL		(N)
#define AFF_PROTECT_GOOD		(O)
#define AFF_SNEAK				(P)
#define AFF_HIDE				(Q)
#define AFF_SLEEP				(R)
#define AFF_CHARM				(S)
#define AFF_FLYING				(T)
#define AFF_PASS_DOOR			(U)
#define AFF_HASTE				(V)
#define AFF_CALM				(W)
#define AFF_PLAGUE				(X)
#define AFF_WEAKEN				(Y)
#define AFF_DARK_VISION 		(Z)
#define AFF_BERSERK 			(aa)
#define AFF_SWIM				(bb)
#define AFF_REGENERATION		(cc)
#define AFF_SLOW				(dd)
#define AFF_FEAR				(ee)

// Bits for 'affected_by2' - Used in #MOBILES.
#define AFF2_POSSESSION 		(A)
#define AFF2_RAMPAGE			(B)
#define AFF2_VAMP_BITE			(C)
#define AFF2_GHOUL				(D)
#define AFF2_CHI_POWER			(E)
#define AFF2_FADE				(F)
#define AFF2_TAUNT				(G)
#define AFF2_CRIPPLE			(H)
#define AFF2_CAMOUFLAGE			(I)
#define AFF2_DETECT_CAMOUFLAGE	(J)
#define AFF2_ROAR				(K)
#define AFF2_WARCRY				(L)
#define AFF2_SHIELD				(M)
#define AFF2_MUTE				(N)
#define AFF2_TREEFORM			(P)
#define AFF2_PASSWOTRACE		(Q)
#define AFF2_VANISH				(R)
#define AFF2_FEAR_MAGIC			(S)
#define AFF2_DETECT_TREEFORM	(T)
#define AFF2_DETECT_VANISH		(U)
#define AFF2_FIRE_SHIELD		(V) // not yet implemented
#define AFF2_ICE_SHIELD			(W)	// not yet implemented 32
#define AFF2_SHOCK_SHIELD		(X)	// not yet implemented 33
#define AFF2_HALLUCINATE		(Y)	// not yet implemented 34
#define AFF2_VICEGRIP			(Z)
#define AFF2_STOCKADE			(aa) // Ixliam
#define AFF2_STONE_GARGOYLE		(bb) // Ixliam
#define AFF2_MANA_REGEN			(cc) // Tristan

// Bits for 'affected_by3' - Used in #MOBILES.
#define AFF3_DET_TRAPS			(ee)

/*
* Class Definitions
*/
#define CLASS_MAGE           0
#define CLASS_CLERIC         1
#define CLASS_THIEF          2
#define CLASS_WARRIOR        3
#define CLASS_DRUID          4
#define CLASS_PALADIN        5
#define CLASS_RANGER         6
#define CLASS_BARBARIAN      7
#define CLASS_SPELLFILCHER   8

// Class Flags
#define CLASSFLAG_POISON_IMMUNITY				(A) // Paladins get this
#define CLASSFLAG_CURSE_IMMUNITY				(B)	// Paladins get this
#define CLASSFLAG_PLAGUE_IMMUNITY				(C)	// Paladins get this
#define CLASSFLAG_DAMMODS_WITH_HOLYWEAPONS		(D) // Paladins get this
#define CLASSFLAG_SAFE_FLEE_FROM_COMBAT			(E) // Thieves get this
#define CLASSFLAG_CASTING_AFFECTED_BY_MOON		(F) // Mages get this
#define CLASSFLAG_CASTING_HALFAFFECTED_BY_MOON	(G) // Spellfilchers get this
#define CLASSFLAG_MAGIC_ANTIPATHY				(H) // Barbs get this
#define CLASSFLAG_LEVEL_BASED_AC				(I) // Barbs get this
#define CLASSFLAG_TOTEMS						(J) // Druids get this
#define CLASSFLAG_CAN_COLLECT_WATER				(K) // Druids get this
#define CLASSFLAG_DEITIES						(L) // Clerics and Paladins get this
#define CLASSFLAG_HOLYSYMBOLS					(M) // Clerics get this
#define CLASSFLAG_NO_CUSTOMIZATION				(N) // The class can't customise
#define CLASSFLAG_ALWAYS_HIDDEN_FROM_MORTAL_CLASSINFO (O) // only effects morts
#define CLASSFLAG_HIDDEN_FROM_MORTAL_CLASSINFO_WHEN_ABOVE_THEIR_REMORT (P) // only effects morts
#define CLASSFLAG_HOLY_CLASS					(Q)

/*
* Class Restrictions
*/
#define RESTRICT_MAGE           A
#define RESTRICT_CLERIC         B
#define RESTRICT_THIEF          C
#define RESTRICT_WARRIOR        D
#define RESTRICT_DRUID          E
#define RESTRICT_PALADIN        F
#define RESTRICT_RANGER         G
#define RESTRICT_BARBARIAN      H
#define RESTRICT_SPELLFILCHER   I


// Sex -  Used in #MOBILES.
#define SEX_NEUTRAL			0
#define SEX_MALE			1
#define SEX_FEMALE			2
#define SEX_RANDOM			3

// AC types
#define AC_PIERCE			0
#define AC_BASH				1
#define AC_SLASH			2
#define AC_EXOTIC			3

// dice
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

// size
#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5

// Item types - Used in #OBJECTS
#define ITEM_LIGHT			1
#define ITEM_SCROLL			2
#define ITEM_WAND			3
#define ITEM_STAFF			4
#define ITEM_WEAPON			5
#define ITEM_COMPONENT		6
#define ITEM_RP				7
#define ITEM_TREASURE		8
#define ITEM_ARMOR			9
#define ITEM_POTION			10
#define ITEM_CLOTHING		11
#define ITEM_FURNITURE		12
#define ITEM_TRASH			13
#define ITEM_PARCHMENT		14
#define ITEM_CONTAINER		15
#define ITEM_INSTRUMENT		16
#define ITEM_DRINK_CON		17
#define ITEM_KEY			18
#define ITEM_FOOD			19
#define ITEM_MONEY			20
#define ITEM_HERB			21
#define ITEM_BOAT			22
#define ITEM_CORPSE_NPC		23
#define ITEM_CORPSE_PC		24
#define ITEM_FOUNTAIN		25
#define ITEM_PILL			26
#define ITEM_PROTECT		27
#define ITEM_MAP			28
#define ITEM_PORTAL			29
#define ITEM_WARP_STONE		30
#define ITEM_ROOM_KEY		31
#define ITEM_GEM			32
#define ITEM_JEWELRY		33
#define ITEM_JUKEBOX		34
#define ITEM_TOKEN			35
#define ITEM_POULTICE		36
#define ITEM_MORTAR			37
#define ITEM_CAULDRON		38
#define ITEM_FLASK			39
#define ITEM_ORE			40
#define ITEM_PENDANT		41
#define	ITEM_SHEATH			42

// Extra flags - Used in #OBJECTS
#define OBJEXTRA_GLOW			(A)
#define OBJEXTRA_HUM			(B)
#define OBJEXTRA_DARK			(C)
#define OBJEXTRA_LOCK			(D)
#define OBJEXTRA_EVIL			(E)
#define OBJEXTRA_INVIS			(F)
#define OBJEXTRA_MAGIC			(G)
#define OBJEXTRA_NODROP 		(H)
#define OBJEXTRA_BLESS			(I)
#define OBJEXTRA_ANTI_GOOD		(J)
#define OBJEXTRA_ANTI_EVIL		(K)
#define OBJEXTRA_ANTI_NEUTRAL	(L)
#define OBJEXTRA_NOREMOVE		(M)
#define OBJEXTRA_INVENTORY		(N)
#define OBJEXTRA_NOPURGE		(O)
#define OBJEXTRA_ROT_DEATH		(P)
#define OBJEXTRA_VIS_DEATH		(Q)
#define OBJEXTRA_OTTERLUNGS		(R)
#define OBJEXTRA_NONMETAL		(S)
#define OBJEXTRA_NOLOCATE		(T)
#define OBJEXTRA_MELT_DROP		(U)
#define OBJEXTRA_HAD_TIMER		(V)
#define OBJEXTRA_SELL_EXTRACT	(W)
#define OBJEXTRA_NO_DEGRADE 	(X)
#define OBJEXTRA_BURN_PROOF 	(Y)
#define OBJEXTRA_NOUNCURSE		(Z)
#define OBJEXTRA_CHAOS			(aa)
#define OBJEXTRA_HORNED 		(bb)
#define OBJEXTRA_NO_RESTRING	(cc)
#define OBJEXTRA_LODGED			(dd)
#define OBJEXTRA_NO_GET_ALL		(ee)

// Extra flags 2 - Used in #OBJECTS
#define OBJEXTRA2_BURIED		(A)
#define OBJEXTRA2_NODECAY		(B)
#define OBJEXTRA2_NOPRIMARY		(C)
#define OBJEXTRA2_NOSECONDARY	(D)
#define OBJEXTRA2_NOCHAOS		(E)
#define OBJEXTRA2_QUEST			(F)
#define OBJEXTRA2_HOLY			(G)
#define OBJEXTRA2_REMORT 		(H)
#define OBJEXTRA2_VAMPIRE_BANE	(I)
#define OBJEXTRA2_TRAP			(J)
#define OBJEXTRA2_NOSELL		(L)
#define OBJEXTRA2_NOQUEST		(M)
#define OBJEXTRA2_ANTI_LAW		(N)
#define OBJEXTRA2_ANTI_CHAOS	(O)
#define OBJEXTRA2_ANTI_BALANCE	(P)
#define	OBJEXTRA2_BLESSED_IN_HOLY_HANDS (Q)
#define	OBJEXTRA2_MUDPROG_RECURSION_ALLOWED (R)
#define	OBJEXTRA2_BAIT			(ee)


// Wear flags - Used in #OBJECTS
#define OBJWEAR_TAKE			(A)
#define OBJWEAR_FINGER			(B)
#define OBJWEAR_NECK			(C)
#define OBJWEAR_TORSO			(D)
#define OBJWEAR_HEAD			(E)
#define OBJWEAR_LEGS			(F)
#define OBJWEAR_FEET			(G)
#define OBJWEAR_HANDS			(H)
#define OBJWEAR_ARMS			(I)
#define OBJWEAR_SHIELD			(J)
#define OBJWEAR_ABOUT			(K)
#define OBJWEAR_WAIST			(L)
#define OBJWEAR_WRIST			(M)
#define OBJWEAR_WIELD			(N)
#define OBJWEAR_HOLD			(O)
#define OBJWEAR_NO_SAC			(P)
#define OBJWEAR_FLOAT			(Q)
#define OBJWEAR_LODGED_ARM		(R)
#define OBJWEAR_LODGED_LEG		(S)
#define OBJWEAR_LODGED_RIB		(T)
#define OBJWEAR_SHEATHED		(U)
#define OBJWEAR_CONCEALED		(V)
#define OBJWEAR_EYES			(W)
#define OBJWEAR_EAR				(X)
#define OBJWEAR_FACE			(Y)
#define OBJWEAR_ANKLE			(Z)
#define OBJWEAR_BACK			(aa)
#define OBJWEAR_PENDANT			(bb)


// weapon class
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_STAFF		3
#define WEAPON_MACE			4
#define WEAPON_AXE			5
#define WEAPON_FLAIL		6
#define WEAPON_WHIP			7
#define WEAPON_POLEARM		8
#define WEAPON_SICKLE		9
#define WEAPON_SPEAR		10

// specialization types
#define SPECIALIZE_NONE		0
#define SPECIALIZE_SWORD	1
#define SPECIALIZE_DAGGER	2
#define SPECIALIZE_STAFF	3
#define SPECIALIZE_MACE		4
#define SPECIALIZE_AXE		5
#define SPECIALIZE_FLAIL	6
#define SPECIALIZE_WHIP		7
#define SPECIALIZE_POLARM	8
#define SPECIALIZE_SPEAR        9

// weapon types
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)
#define WEAPON_HOLY			(I)
#define WEAPON_LIVING		(J)
#define WEAPON_SLAYER		(K)
#define WEAPON_SUCKLE       (L)
#define WEAPON_ENERVATE     (M)
#define WEAPON_ANNEALED     (N)

// gate flags
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH			(C)
#define GATE_BUGGY			(D)
#define GATE_RANDOM			(E)
#define GATE_OPAQUE			(F)
#define GATE_SHORT_LOOKINTO (G)

// token flags
#define TOKEN_DROPDEATH		(A)
#define TOKEN_QUITDEATH		(B)

// furniture flags
#define STAND_AT			(A)
#define STAND_ON			(B)
#define STAND_IN			(C)
#define SIT_AT				(D)
#define SIT_ON				(E)
#define SIT_IN				(F)
#define REST_AT				(G)
#define REST_ON				(H)
#define REST_IN				(I)
#define SLEEP_AT			(J)
#define SLEEP_ON			(K)
#define SLEEP_IN			(L)
#define PUT_AT				(M)
#define PUT_ON				(N)
#define PUT_IN				(O)
#define PUT_INSIDE			(P)
#define KNEEL_AT			(Q)
#define KNEEL_ON			(R)
#define KNEEL_IN			(S)
#define STAND_UNDER			(T)
#define SIT_UNDER			(U)
#define REST_UNDER			(V)
#define SLEEP_UNDER			(W)
#define PUT_UNDER			(X)
#define KNEEL_UNDER			(Y)

// Apply types (for affects) - Used in #OBJECTS
// moved into enum stored in structs.h

// Values for containers (value[1]) - Used in #OBJECTS
#define CONT_CLOSEABLE		(A)
#define CONT_PICKPROOF		(B)
#define CONT_CLOSED			(C)
#define CONT_LOCKED			(D)
#define CONT_PUT_ON			(E)
#define CONT_LOCKER			(F)

// Room affected_by flags
#define ROOMAFF_UTTERDARK		(A)
#define ROOMAFF_ALARM			(B)
#define ROOMAFF_CONE_OF_SILENCE (C)
#define ROOMAFF_UNDERWATER		(D)
#define ROOMAFF_LEONIDS			(E)
#define	ROOMAFF_BLESSED_GROUNDS	(F)
#define ROOMAFF_CURSED_GROUNDS	(G)

// Room flags - Used in #ROOMS
#define ROOM_DARK               (A)
#define ROOM_NO_SUMMON			(B) // doesn't do anything yet
#define ROOM_NO_MOB             (C)
#define ROOM_INDOORS            (D)
#define ROOM_NO_PORTAL			(E) // doesn't do anything yet
#define ROOM_ARENA				(F) // doesn't do anything yet
#define ROOM_BANK				(G)
#define ROOM_NOCHANNELS			(H)	// mortals can't use channels while in the room
#define ROOM_PRIVATE            (J)
#define ROOM_SAFE               (K)
#define ROOM_SOLITARY           (L)
#define ROOM_PET_SHOP           (M)
#define ROOM_NO_RECALL          (N)
#define ROOM_IMP_ONLY           (O)
#define ROOM_GODS_ONLY          (P)
#define ROOM_HEROES_ONLY        (Q)
#define ROOM_NEWBIES_ONLY       (R)
#define ROOM_LAW                (S)
#define ROOM_NOWHERE            (T)
#define ROOM_OOC                (U)
#define ROOM_NOSCRY				(W)
#define ROOM_ANTIMAGIC			(X)
#define ROOM_NOSPEAK			(Y)
#define ROOM_LIGHT				(Z)
#define ROOM_NOFLY				(aa)
#define ROOM_NOSCAN				(bb)
#define ROOM_NOAUTOMAP			(cc)
#define ROOM_NOAUTOEXITS		(dd)
#define ROOM_INN				(ee)

#define ROOM2_NO_AREA_ECHOES	(A) // room doesn't see area echoes
#define ROOM2_MINE				(B) // room can be mined for ores

#define ROOM2_SALTWATER_FISHING		(dd) // can fish here, in the salt water 
#define ROOM2_FRESHWATER_FISHING	(ee) // room has freshwater to fish in

// Exit flags - Used in #ROOMS
#define EX_ISDOOR			(A)
#define EX_CLOSED			(B)
#define EX_LOCKED			(C)
#define EX_PICKPROOF		(F)
#define EX_NOPASS			(G)
#define EX_EASY				(H)
#define EX_HARD				(I)
#define EX_INFURIATING		(J)
#define EX_NOCLOSE			(K)
#define EX_NOLOCK			(L)
#define EX_ONEWAY			(M)
#define EX_OBVIOUS			(N)

// Sector types - Used in #ROOMS
#define SECT_INSIDE			0
#define SECT_CITY			1
#define SECT_FIELD			2
#define SECT_FOREST			3
#define SECT_HILLS			4
#define SECT_MOUNTAIN		5
#define SECT_WATER_SWIM		6
#define SECT_WATER_NOSWIM	7
#define SECT_SWAMP			8
#define SECT_AIR			9
#define SECT_DESERT			10
#define SECT_CAVE			11
#define SECT_UNDERWATER		12
#define SECT_SNOW			13
#define SECT_ICE			14
#define SECT_TRAIL			15
#define	SECT_LAVA			16
#define SECT_MAX			17

// Sector bits, for use with dampen, restrict, enhance
#define SECTBIT_INSIDE			(A)
#define SECTBIT_CITY			(B)
#define SECTBIT_FIELD			(C)
#define SECTBIT_FOREST			(D)
#define SECTBIT_HILLS			(E)
#define SECTBIT_MOUNTAIN		(F)
#define SECTBIT_WATER_SWIM		(G)
#define SECTBIT_WATER_NOSWIM	(H)
#define SECTBIT_SWAMP			(I)
#define SECTBIT_AIR				(J)
#define SECTBIT_DESERT			(K)
#define SECTBIT_CAVE			(L)
#define SECTBIT_UNDERWATER		(M)
#define SECTBIT_SNOW			(N)
#define SECTBIT_ICE				(O)
#define SECTBIT_TRAIL			(P)
#define SECTBIT_LAVA			(Q)

// Equipment wear locations - Used in #RESETS
#define WEAR_NONE			-1
#define WEAR_LIGHT			0
#define WEAR_FINGER_L		1
#define WEAR_FINGER_R		2
#define WEAR_NECK_1			3
#define WEAR_NECK_2			4
#define WEAR_TORSO			5
#define WEAR_HEAD			6
#define WEAR_LEGS			7
#define WEAR_FEET			8
#define WEAR_HANDS			9
#define WEAR_ARMS			10
#define WEAR_SHIELD			11
#define WEAR_ABOUT			12
#define WEAR_WAIST			13
#define WEAR_WRIST_L		14
#define WEAR_WRIST_R		15
#define WEAR_WIELD			16
#define WEAR_HOLD			17
#define WEAR_FLOAT			18
#define WEAR_SECONDARY		19
#define WEAR_LODGED_ARM		20
#define WEAR_LODGED_LEG		21
#define WEAR_LODGED_RIB		22
#define WEAR_SHEATHED		23
#define WEAR_CONCEALED		24
#define WEAR_EYES			25
#define WEAR_EAR_L          26
#define WEAR_EAR_R          27
#define WEAR_FACE           28
#define WEAR_ANKLE_L        29
#define WEAR_ANKLE_R        30
#define WEAR_BACK           31
#define WEAR_PENDANT		32
#define MAX_WEAR			33

// Defines for mf experimental creation system, Tristan 08/04
#define CREATING_DAWN_SYSTEM	1
#define	CREATING_MF_EXPERIMENTAL_SYSTEM	2

enum timefields
{
	TIME_NONE,
	TIME_SMALL_HOURS,
	TIME_DAWN,
	TIME_MORNING,
	TIME_NOON,
	TIME_AFTERNOON,
	TIME_DUSK,
	TIME_EVENING,
	TIME_MIDNIGHT,
	TIME_MAX,
};

enum modifiers
{
	DIFF_NONE,
	DIFF_TRIVIAL,
	DIFF_EASY,
	DIFF_LIGHT,
	DIFF_MEDIUM,
	DIFF_HARD,
	DIFF_VERY_HARD,
	DIFF_ABSURD,
	DIFF_SHEER_FOLLY,
	DIFF_MAX,
};

enum totems
{
	TOTEM_BEAR,
	TOTEM_STAG,
	TOTEM_SWALLOW,
	TOTEM_OSPREY,
	TOTEM_TORTOISE,
	TOTEM_RAVEN,
	TOTEM_BAT,
	TOTEM_WOLF,
	TOTEM_SERPENT,
	TOTEM_TOAD,
	TOTEM_OWL,
	TOTEM_HARE,
	TOTEM_MAX,
};

enum mixtypes
{
	MIXTYPE_NONE,
	MIXTYPE_HERBALISM,
	MIXTYPE_COOKING,
	MIXTYPE_SMITHING,
	MIXTYPE_ALCHEMY,
	MIXTYPE_POTTERY,
	MIXTYPE_TINKERING,
	MIXTYPE_MAX,
};

//	OLC Race flags
#define			RACE_NONE			0
#define			RACE_CHANGED		(A)
#define			RACE_ADDED			(B)

// OLCArea flags.
#define         OLCAREA_NONE       0
#define         OLCAREA_CHANGED    (A)       // Area has been modified.
#define         OLCAREA_ADDED      (B)       // Area has been added to.
#define         OLCAREA_IGNORE_UNDEFINED_FLAGS (D)  // ignores undefined bits when resaving an area.
													// set on an area that is read in from anything
													// other than NAFF
#define         OLCAREA_INVITELISTCHANGED (E)

// Area flags.
#define         AREA_NONE				0
#define         AREA_OLCONLY			(A)  // Area can't be accessed by nonbuilders
#define         AREA_NOTELEPORT			(B)  // Area can't be teleported into
#define         AREA_NOSCRY				(C)  // Area can't be SCRYED into
#define         AREA_HIDDEN				(D)  // Area unseen on wholist by morts
#define         AREA_USE_BUILDRESTRICTS	(E)  
#define         AREA_LOCKED				(F)  
#define         AREA_NOGATEINTO			(G)  // Area can't be gated into
#define         AREA_NOSUMMONINTO		(H)  // Things can't be summoned into the area
#define         AREA_NOPORTALINTO		(I)  // portals can't be cast into the area
#define         AREA_NEWBIE_AREA_RESETS	(J)  // resets the area more often - for newbie areas

#define         BUILDRESTRICT_ALL		(0)
#define         BUILDRESTRICT_AREA		(1)
#define         BUILDRESTRICT_EXITS		(2)
#define         BUILDRESTRICT_MUDPROGS	(3)
#define         BUILDRESTRICT_MOBS		(4)
#define			BUILDRESTRICT_OBJECTS	(5)
#define         BUILDRESTRICT_RESETS	(6)
#define         BUILDRESTRICT_ROOMS		(7)
#define         BUILDRESTRICT_OTHER		(8)

// HELP flags.
#define         HELPFILE_NONE       0
#define         HELPFILE_CHANGED    (A) // Area has been modified.

//	SEDIT flags
#define			SEDIT_NONE			0
#define			SEDIT_CHANGED		(A)
#define			SEDIT_ADDED			(B)

//  COMEDIT flags
#define			COMEDIT_NONE		0
#define			COMEDIT_CHANGED		(A)

//  QEDIT FLAGS
#define			QEDIT_NONE			0
#define			QEDIT_CHANGED		(A)

//	MIX FLAGS
#define			MIX_NONE			0
#define			MIX_CHANGED			(A)

//  DEDIT FLAGS

#define			DEDIT_NONE			0
#define			DEDIT_CHANGED		(A)		// Autosave notification

/*
#define			DEDIT_GOOD_ALIGN	(B)
#define			DEDIT_NEUTRAL_ALIGN	(C)
#define			DEDIT_EVIL_ALIGN	(D)
#define			DEDIT_ORDER_TENDENCY	(E)
#define			DEDIT_NEUTRAL_TENDENCY	(F)
#define			DEDIT_CHAOTIC_TENDENCY	(G)
#define			DEDIT_MALE_GENDER		(H)
#define			DEDIT_FEMALE_GENDER		(I)
#define			DEDIT_NEUTRAL_GENDER	(J)
*/

//	GENERIC ALIGN FLAGS
#define			ALIGN_NONE			0
#define			ALIGN_EVIL			(A)
#define			ALIGN_NEUTRAL		(B)
#define			ALIGN_GOOD			(C)

// GENERIC TENDENCY FLAGS
#define			TENDFLAG_NONE			0
#define			TENDFLAG_CHAOTIC		(A)
#define			TENDFLAG_NEUTRAL		(B)
#define			TENDFLAG_LAWFUL			(C)

//	CLASSEDIT flags
#define			CLASSEDIT_NONE			0
#define			CLASSEDIT_CHANGED		(A)

//  LOG stuff moved here to make it a bit more accessible
#define LOG_ALWAYS		0	// always logged into the system log
#define LOG_PALWAYS		1	// always log players only into the system log
#define LOG_OLC			3	// ALWAYS Logged into OLC log files
#define LOG_NORMAL		5	// Logged into log files (when LOG ALL is on
							// or a player log is on... if plog on, also
							// logged into their own player log.
#define LOG_PLOGONLY	7	// only logged into players log file when
							// player is being logged, not logged into
							// main system.
#define LOG_DONT_LOG	8	// not logged but can be snooped - eg directions
#define LOG_NEVER		10	// Never logged - only thing not snoopable

#define CMD_OOC					(A)
#define	CMD_IC					(B)
#define CMD_NO_ORDER			(C)
#define CMD_NO_WIZNET_SECURE	(D)
#define CMD_NO_TREEFORM			(E)
#define CMD_NO_STONE_GARGOYLE	(F)
#define CMD_NO_STOCKADE			(G)


enum mob_balance
{
	MOB_BALANCE_NUM_HIT_DIE,
	MOB_BALANCE_HIT_DIE_TYPE,
	MOB_BALANCE_HIT_BONUS,
	MOB_BALANCE_AC,
	MOB_BALANCE_NUM_DAM_DIE,
	MOB_BALANCE_DAM_DIE_TYPE,
	MOB_BALANCE_DAM_BONUS,
	MOB_BALANCE_MAX,
};

enum categories 
{
	CAT_UNDEFINED,
	CAT_DEFENSIVE_SPELL,
	CAT_DEFENSIVE_ENHANCEMENT,
	CAT_OFFENSIVE_ENHANCEMENT,
	CAT_OFFENSIVE_SPELL,
    CAT_AREASPELL,
	CAT_ENCHANTMENT_SPELL,
	CAT_HEALING_SPELL,
	CAT_MOVEMENT_SPELL,
    CAT_CREATION_SPELL,
    CAT_INFORMATION_SPELL,
	CAT_MISCELLANEOUS_SPELL,
	CAT_REALM,
	CAT_SPHERE,
	CAT_SEASON,
	CAT_LANGUAGE,
	CAT_WEAPON,
	CAT_OFFENSIVE_COMBAT,
	CAT_DEFENSIVE_COMBAT,
	CAT_HEALING_SKILL,
	CAT_MOVEMENT_SKILL,
	CAT_MAGIC,
	CAT_MISCELLANEOUS_SKILL,
	CAT_DEFAULT_SKILL,
};

enum com_categories
{
	COMCAT_UNDEFINED,
	COMCAT_AUTO,
	COMCAT_CLAN,
	COMCAT_COMBAT,
	COMCAT_COMMUNICATION,
	COMCAT_MAGIC,
	COMCAT_MISC,
	COMCAT_MOVEMENT,
	COMCAT_NOTES,
	COMCAT_OBJECT,
	COMCAT_PKRELATED,
	COMCAT_RP,
	COMCAT_STATISTICS,
	COMCAT_UTILITIES,	
	COMCAT_OLC,
	COMCAT_NOBLE,
	COMCAT_NSUPPORT,
	COMCAT_RPSUPPORT,
	COMCAT_HELPS
};

#define GRANTGROUP_MORTAL			(B)
#define GRANTGROUP_NOBLE			(C)
#define GRANTGROUP_BUILDER			(D)
#define GRANTGROUP_IMPLEMENTOR		(E)
#define GRANTGROUP_HIGHADMIN		(F)
#define GRANTGROUP_ADMIN			(G)
#define GRANTGROUP_HEADBALANCE		(H)
#define GRANTGROUP_HEADCODE			(I)
#define GRANTGROUP_HEADLAW			(J)
#define GRANTGROUP_HEADMYTHOS		(K)
#define GRANTGROUP_HEADREALM		(L)
#define GRANTGROUP_HEADSUPPORT		(M)
#define GRANTGROUP_HEADROLEPLAY		(N)
#define GRANTGROUP_BALANCE			(O)
#define GRANTGROUP_CODE				(P)
#define GRANTGROUP_LAW				(Q)
#define GRANTGROUP_MYTHOS			(R)
#define GRANTGROUP_REALM			(S)
#define GRANTGROUP_SUPPORT			(T)
#define GRANTGROUP_ROLEPLAY			(U)
#define GRANTGROUP_ADVANCEDBUILDER	(V)
#define GRANTGROUP_DEVELOPMENT		(W)
#define GRANTGROUP_EXPERIMENTAL		(X)
#define GRANTGROUP_BETATEST1		(Y)
#define GRANTGROUP_BETATEST2		(Z)
#define GRANTGROUP_INCOMPLETE		(aa)
#define GRANTGROUP_IMMORTAL			(bb)
#define GRANTGROUP_OBSCUREIMMORTAL	(cc) // the more obscure commands

// Global Constants
extern	char	*const	dir_shortname[];
extern	char	*const	dir_name[];
extern	const	sh_int	rev_dir[];
extern	const	struct	spec_type	spec_table[];
extern	const	struct	ospec_type	ospec_table[];
extern	int		max_on;
extern	int		true_count;
extern	char	EXE_FILE[30];
extern	char	MACHINE_NAME[MSL];
extern	char	last_command[MSL];
extern	char	last_input[MSL];


// Global variables
extern          int                     top_affect;
extern          int                     top_area;
extern			int						top_deity;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     social_count;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern          int                     top_shop;
extern          int                     top_inn;

extern          int                     top_vnum_mob;
extern          int                     top_vnum_obj;
extern          int                     top_vnum_room;

extern          char                    str_empty       [1];
extern          long                    tick_counter;
extern          int                     note_notify_counter;
extern          long                    last_vnum; 
//  'last_vnum' used for debugging ie print last_vnum in gdb
extern  MOB_INDEX_DATA					*mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA					*obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA					*room_index_hash [MAX_KEY_HASH];
extern  MOOT_DATA						*moot;

// Conditions.
#define COND_DRUNK				0
#define COND_FULL				1
#define COND_THIRST				2
#define COND_HUNGER				3


// old positions 
#define POS_OLD_DEAD			0
#define POS_OLD_MORTAL			1
#define POS_OLD_INCAP			2
#define POS_OLD_STUNNED			3
#define POS_OLD_SLEEPING		4
#define POS_OLD_RESTING			5
#define POS_OLD_SITTING			6
#define POS_OLD_FIGHTING		7
#define POS_OLD_STANDING		8


/* determines if they are
 * comatose (will die),
 * unconscious,
 * semiconscious,
 * subdued,
 * yielding,
 * free 
 */
// Positions
#define POS_DEAD				0  
#define POS_MORTAL				1 
#define POS_INCAP				2
#define POS_STUNNED				3
#define POS_SLEEPING			4
//#define POS_SUBDUED			5
#define POS_RESTING				5
#define POS_SITTING				6
#define POS_KNEELING			7
#define POS_FIGHTING			8
#define POS_STANDING			9


// ACT bits for players.
#define PLR_IS_NPC              (A) // Don't EVER set.   
#define PLR_CAN_ADVANCE         (B)
#define PLR_AUTOASSIST          (C)
#define PLR_AUTOEXIT            (D)
#define PLR_AUTOLOOT            (E)
#define PLR_AUTOREFORMAT		(F) // Automatically reformat notes on post 
#define PLR_AUTOGOLD            (G)
#define PLR_AUTOSPLIT           (H)
#define PLR_NOTEACH				(J)
#define PLR_AUTOSUBDUE			(K) // player is going to subdue 
#define PLR_QUESTER				(L) 

#define PLR_HOLYLIGHT			(N)
#define PLR_HOLYNAME			(O)

#define PLR_NOSUMMON			(Q)
#define PLR_NOFOLLOW			(R)
#define PLR_SPECIFY_SELF		(S)
#define PLR_HOLYVNUM			(T)
#define PLR_PERMIT				(U)

#define PLR_LOG					(W)
#define PLR_DENY				(X)
#define PLR_FREEZE				(Y)
#define PLR_NORP				(Z)

#define PLR_NOREDUCING_MAXKARN	(aa) // 
#define PLR_CAN_HERO			(bb)
#define PLR_AUTOPEEK			(cc) // Kal jun 98
#define PLR_HOLYWALK			(dd) // Kal jun 98
#define PLR_AUTOMAP				(ee) // Kal Sept 98

// dynamic data - not saved 
#define DYN_CURRENT_SUBDUE		(A) // player is currently subduing in
									// current fight 

#define DYN_USER_OF_ALARM_SPELL	(B) // when set, we need to scan all rooms in game and clear any alarms
#define DYN_MAPFIRST			(C)
#define DYN_MAPCLEAR			(D)
#define DYN_WIZARDEYEING        (E) // used for wizard eye
#define DYN_DOING_DAMAGE		(F) // used to determine who starts a fight
#define DYN_SILENTLY			(G)
#define DYN_USING_ATLEVEL		(H)
#define DYN_IS_BEING_ORDERED	(J)
#define DYN_IS_CUTTING_OFF		(K)
#define DYN_SUCCESS_CAST		(L)
#define DYN_RUNNING_MUDPROG_CMD	(M) // used to determine who starts a fight
									// with a mudprog response
#define DYN_USING_AT			(N)
#define	DYN_NO_SUBLEVEL_SPELL_LEARN (O)
#define	DYN_IMMLASTON			(P)
#define	DYN_NONMAGICAL_FLYING	(Q)	// used to stop those with magical fly
									// losing mv for flying
#define	DYN_TEMP_FLAG			(R) // can be used for any purpose, by any function
									// iwhere uses it currently

#define	DYN_MOB_SEE_ALL			(S) // Used to make all can_see triggers true
									// after a 'mob seeall' call, call it once at the top 
									// of a function, bit turned on as program_flow returns


#define	DYN_UNKNOWN_REPLY_NAME		(T)
#define	DYN_HAS_ANONYMOUS_RETELL	(V)
#define	DYN_IN_ROOM_ONLY			(W)
#define	DYN_STARTED_FIGHT			(X)
#define	DYN_AUTOPKASSIST			(Y)
#define	DYN_SHOWFLAGS				(Z)
#define	DYN_USING_OLCAFTER			(aa) // showafter and showflagsafter
#define	DYN_USING_KTELL				(bb)
#define	DYN_NO_PROMPT_SWITCHED_PREFIX (cc)
#define	DYN_MPBUG_USE_QUEUED_DETAILS (dd)

// dynamic OBJECT data - not saved either :)  DYN bits rock!!!!
#define	DYNOBJ_BURIED			(A)

// comm flags -- may be used on both mobs and chars
#define COMM_CODING             (B)	
#define COMM_GLOBAL_SOCIAL_OFF	(C)	
#define COMM_NOGPROMPT			(F)	
#define COMM_WHOVIS             (G)
#define COMM_NEWBIE_SUPPORT     (H)
#define COMM_NOQUOTE            (I)
#define COMM_SHOUTSOFF          (J)

// display flags
#define COMM_SPELL_DEBUG        (K)
#define COMM_COMPACT            (L)
#define COMM_BRIEF				(M)
#define COMM_PROMPT				(N)
#define COMM_COMBINE            (O)
#define COMM_TELNET_GA          (P)
#define COMM_SHOW_AFFECTS       (Q)
#define COMM_AUTOSELF			(R)
#define COMM_ANNOUNCEOFF		(S)

// penalties
#define COMM_NOEMOTE            (T)
#define COMM_NOSHOUT            (U)
#define COMM_NOTELL				(V)
#define COMM_NOCHANNELS         (W) 
#define COMM_BUILDING			(X) // in OLC mode
#define COMM_SNOOP_PROOF        (Y)
#define COMM_AFK				(Z)

#define COMM_CANNOCHANNEL		(aa) // CAN NO CHANNEL OTHER PLAYERS
//#define unused				(bb) 
#define COMM_REDUCED_LASTON     (cc) // don't get much info about imms from laston
#define COMM_NOPRAY             (dd) 

// Config flags
#define CONFIG_SHOWMISC			(A)
#define CONFIG_NOMISC			(B)
#define CONFIG_OBJRESTRICT		(C)
#define CONFIG_AUTORECALL		(E)
#define CONFIG_NOTE_ONLY_TO_IMM (F)
#define CONFIG_AUTOEXAMINE		(G)
#define CONFIG_AUTOLANDONREST	(H)
#define CONFIG_COURTMEMBER		(I)
#define CONFIG_BARD_COUNCIL		(J)

#define CONFIG_HOLYSPEECH		(M)
#define CONFIG_DISALLOWED_PKILL	(N)
#define CONFIG_IGNORE_MULTILOGINS (O)
#define CONFIG_RP_SUPPORT		(Q)
#define CONFIG_METRIC			(R)
#define CONFIG_ACTIVE			(S)
#define CONFIG_AUTOTRACK		(T)
#define	CONFIG_AUTOMAP			(U)
#define	CONFIG_AUTOWRAPTELLS	(V)
#define CONFIG_PRACSYS_TESTER	(W) // All pracs use the new system
#define	CONFIG_NOCHARM			(X)
#define CONFIG_NOHEROMSG		(Y)
#define CONFIG_NONEWBIE			(Z)	// handles turning newbie channel off/on
#define CONFIG_NONAMES			(aa)// allows players to not see introduce names
#define CONFIG_NOAUTOANSWER		(bb)// Prevents players from autoanswering an askname request
#define CONFIG_HIDE_HIDDEN_AREAS (cc) // imms can choose to hide the areas
#define CONFIG_NAMES_BEFORE_SHORT (dd) // When a players name is shown, it is displayed before their short

// config2 flags
#define CONFIG2_READ_BUILDER_LEGAL	(A)
#define CONFIG2_NOAUTOSAYMOTE		(B)
#define CONFIG2_AUTOSAYCOLOURCODES	(C) // if set, { is changed to {{
#define CONFIG2_NAME_ONLY_FOR_KNOWN	(D)	// display only their name if they are introduced
#define CONFIG2_AUTODAMAGE			(E)	// show hitpoints damage done
#define CONFIG2_NOPKILL				(F)	// this player can not pkill, nor be pkilled
#define CONFIG2_FULL_EXITS			(G)	// sees full exits with their autoexit
#define CONFIG2_NO_DETECT_OLDSTYLE_NOTE_WRITING (H)// if unset, {} is converted into `1 with 'note +'
#define CONFIG2_NOMAPEXITS			(I) 
#define CONFIG2_NOMAPBLANKS			(J) 
#define CONFIG2_AUTOWIZILOGIN		(K) 
#define CONFIG2_AUTOWHOINVISLOGIN	(L) 
#define CONFIG2_NO_BATTLELAG_PROMPT	(M) 
#define CONFIG2_ACCURATE_LASTON_TIMES (N) // if set, the player gets the times everyone has been on
#define CONFIG2_HIGHIMMORTAL_LASTON_ACCESS (O) // if set, the player gets high immortal laston access 
#define CONFIG2_AUTOKEEPALIVE		(P) 
#define CONFIG2_MP_TRIGGER_IN_ROOM	(Q) // MudProg Trigger in Room

// pconfig flags
#define PCONFIG_FILLER	(A) // does nothing

// sayflags
#define SAYFLAG_ONLY_REMEMBERED_CHAR	(A)
#define SAYFLAG_NO_SAYMOTE				(B)
#define SAYFLAG_CONVERT_COLOUR			(C)

// combat_initiate_types
#define CIT_GENERAL				(A)
#define CIT_AGRESSIVE_ACTION	(B)
#define CIT_CASTING_SPELL		(C)
#define CIT_NO_BYPASSDUEL		(D)
#define CIT_SILENT				(E)

// Help flags
#define HELP_NOBLE				(A)
#define HELP_NSUPPORT			(B)
#define HELP_BUILDER			(C)
#define HELP_RPSUPPORT			(D)
#define HELP_REDIRECTION_ENTRY  (E)
#define HELP_HIDE_KEYWORDS		(F)
#define HELP_MUD_SPECIFIC			(G) 
#define HELP_MUD_SHOULD_CUSTOMIZE	(H) // mud should customize for their mud
#define HELP_WORDWRAPPED		(I)
#define HELP_INCOMPLETE			(K)
#define HELP_HIDE_PREVNEXT		(L)
#define HELP_DISPLAY_MXP_DOUBLE	(M)
#define HELP_HIDE_HEADER_FOOTER	(N)

#define HELP_REMOVEHELP			(ee)

// Council flags
#define COUNCIL_BALANCE			(A)
#define COUNCIL_CLAN			(B)
#define COUNCIL_CODE			(C)
#define COUNCIL_LAW				(D)
#define COUNCIL_MYTHOS			(E)
#define COUNCIL_REALM			(F)
#define COUNCIL_RP				(G)
#define COUNCIL_SUPPORT			(H)
#define COUNCIL_HEADBALANCE		(I)
#define COUNCIL_HEADCLAN		(J)
#define COUNCIL_HEADCODE		(K)
#define COUNCIL_HEADLAW			(L)
#define COUNCIL_HEADMYTHOS		(M)
#define COUNCIL_HEADREALM		(N)
#define COUNCIL_HEADRP			(O)
#define COUNCIL_HEADSUPPORT		(P)
#define COUNCIL_HEADSTORYLINE	(Q)
#define COUNCIL_ADMIN			(R)


// WIZnet flags
#define WIZ_ON					(A)
#define WIZ_TICKS				(B)
#define WIZ_LOGINS				(C)
#define WIZ_SITES				(D)
#define WIZ_LINKS				(E)
#define WIZ_DEATHS				(F)
#define WIZ_RESETS				(G)
#define WIZ_MOBDEATHS			(H)
#define WIZ_FLAGS				(I)
#define WIZ_PENALTIES			(J)
#define WIZ_THEFTS				(K)
#define WIZ_LEVELS				(L)
#define WIZ_SECURE				(M)
#define WIZ_SWITCHES			(N)
#define WIZ_SNOOPS				(O)
#define WIZ_AUTOON				(P)
#define WIZ_LOAD				(Q)
#define WIZ_NEWBIE				(R)
#define WIZ_SHOWCHANNEL			(S)
#define WIZ_SPAM				(T)
#define WIZ_RPEXP				(U)
#define WIZ_BUGS				(V)
#define WIZ_BETA				(W)
#define WIZ_PLAYER_LOG			(X)
#define WIZ_WHISPERS			(Y)
#define WIZ_RPMONITOR			(Z)
#define WIZ_NOHELP				(aa)
#define WIZ_QUESTING			(bb)
#define WIZ_PRAYERS_DREAMS		(cc)
#define WIZ_MEMCHECK			(dd)
#define WIZ_NEWBIETELL			(ee)

// mob memory settings 
#define MEM_CUSTOMER			(A)
#define MEM_SELLER				(B)
#define MEM_HOSTILE				(C)
#define MEM_AFRAID				(D)


/*
* Types of attacks.
* Must be non-overlapping with spell/skill types,
* but may be arbitrary beyond that.
*/
#define TYPE_UNDEFINED			-1
#define TYPE_HIT				1000

// Types of entries in the skill_table
#define SKTYPE_UNDEFINED		0
#define SKTYPE_SPELL			1
#define SKTYPE_SKILL			2
#define SKTYPE_OTHER			3
#define SKTYPE_REALM			4
#define SKTYPE_SPHERE			5
#define SKTYPE_ELEMENTSEASON	6

// Types of entries in the skill_table
#define SKFLAGS_SPNEVER_LEARNT_BY_LEVELING	(A)
#define SKFLAGS_SPMUST_BE_SET_TO_GET		(B)
#define SKFLAGS_TEACH_SPGAIN				(C)
#define SKFLAGS_LEVEL_SPGAIN				(D)
#define SKFLAGS_STUDY_SPGAIN				(E)
#define SKFLAGS_NO_PCTARGET					(F)
#define SKFLAGS_NO_NPCTARGET				(G)
#define SKFLAGS_RENAMABLE					(H)
#define SKFLAGS_NO_INTERCLASS_TEACH			(I)
#define SKFLAGS_NEW_IMPROVE_SYSTEM			(J)
#define SKFLAGS_NO_GAIN						(K)
#define SKFLAGS_NO_NEGATIVE_HP_AT_AFFECTOFF	(L)
#define SKFLAGS_MAGICAL_ANTIPATHY			(M)
#define SKFLAGS_NO_SCRIBE					(N)
#define SKFLAGS_USE_RACE_RESTRICTIONS		(O)
#define SKFLAGS_NO_PRAC						(P)

// dynspell_type spellpairs_table[] flags
#define SPFUNC_NOTEXT			(A) // spell contains no custom messages
#define SPFUNC_NODAMTYPE		(B) // spell function doesn't use damtypes
#define SPFUNC_DAMTYPE			(C) // spell supports dynamic damtype


//  Target types.
#define TAR_IGNORE				0
#define TAR_CHAR_OFFENSIVE		1
#define TAR_CHAR_DEFENSIVE		2
#define TAR_CHAR_SELF			3
#define TAR_OBJ_INV				4
#define TAR_OBJ_CHAR_DEF		5
#define TAR_OBJ_CHAR_OFF		6
#define TAR_MOB_OFFENSIVE		7
#define TAR_OBJ_MOB_OFF			8
#define TAR_DIRECTION			9

#define TARGET_CHAR				0
#define TARGET_OBJ				1
#define TARGET_ROOM				2
#define TARGET_NONE				3

/**************************************************************************/
// directions used for room exits
#define DIR_NORTH             (0)
#define DIR_EAST              (1)
#define DIR_SOUTH             (2)
#define DIR_WEST              (3)
#define DIR_UP                (4)
#define DIR_DOWN              (5)
#define DIR_NORTHEAST         (6)
#define DIR_SOUTHEAST         (7)
#define DIR_SOUTHWEST         (8)
#define DIR_NORTHWEST         (9)

/**************************************************************************/
// First set of MudProg Mob triggers
#define MTRIG_ACT		(A)
#define MTRIG_BRIBE		(B)
#define MTRIG_DEATH		(C)
#define MTRIG_ENTRY		(D)
#define MTRIG_FIGHT		(E)
#define MTRIG_GIVE		(F)
#define MTRIG_GREET		(G)
#define MTRIG_GRALL		(H)
#define MTRIG_KILL		(I)
#define MTRIG_HPCNT		(J)
#define MTRIG_RANDOM	(K)
#define MTRIG_SPEECH	(L)
#define MTRIG_EXIT		(M)
#define MTRIG_EXALL		(N)
#define MTRIG_DELAY		(O)
#define MTRIG_SURR		(P)
#define MTRIG_REPOP		(Q)
#define MTRIG_ROOMDEATH	(R)
#define MTRIG_TICK		(S)
#define MTRIG_HOUR		(T)
#define MTRIG_COMMAND	(U)
#define MTRIG_PREKILL	(V)
#define MTRIG_SAYTO		(W)
#define MTRIG_PREMOVE	(X) // just before a mob moves to a new room
#define MTRIG_LOGINROOM  (Y) // just after someone has logged into the room
#define MTRIG_LOGINAREA  (Z) // just after someone has logged into the area 
// room for a logingame (aa) trigger?
#define MTRIG_LOGOUTROOM	(bb) // just after someone has logged out of the room
#define MTRIG_LOGOUTAREA (cc) // just after someone has logged out of the area 
#define MTRIG_PRETRAIN	(dd) // before a player gets training from a mob
#define MTRIG_PREPRAC	(ee) // before a player gets practicing from a mob

/**************************************************************************/
// Second set of MudProg Mob triggers
#define MTRIG2_X		(A) 

/**************************************************************************/
// First set of MUDprog Object trigger defines
#define OTRIG_GET_PRE		(A)
#define OTRIG_GET_POST		(B)
#define OTRIG_DROP_PRE		(C)
#define OTRIG_DROP_POST		(D)
#define OTRIG_PUT_PRE		(E)
#define OTRIG_PUT_POST		(F)
#define OTRIG_WEAR_PRE		(G) // before any existing worn item has been removed
#define OTRIG_WEAR_MID		(H) // after any existing worn item has been removed, but nothing put on
#define OTRIG_WEAR_POST		(I) // after any existing worn item has been removed, and item has been put on
#define OTRIG_LOOKAT_PRE	(J)
#define OTRIG_LOOKAT_POST	(K)
#define OTRIG_LOOKIN_PRE	(L)
#define OTRIG_LOOKIN_POST	(M)
#define OTRIG_CONTAINER_GET_PRE		(N)
#define OTRIG_CONTAINER_GET_POST	(O)
#define OTRIG_CONTAINER_PUTIN_PRE	(P)
#define OTRIG_CONTAINER_PUTIN_POST	(Q)
#define OTRIG_REMOVE_PRE	(R) // before removing an object - automatic or manually
#define OTRIG_REMOVE_POST	(S) // after removing an object - automatic or manually


/**************************************************************************/
// Second set of MUDprog Object trigger defines
#define OTRIG2_X		(A) // table not yet implemented

/**************************************************************************/
// First set of MUDprog Room trigger defines
#define RTRIG_ENTRY		(A)

/**************************************************************************/
// Second set of MUDprog Room trigger defines
#define RTRIG2_X		(A) // table not yet implemented


/**************************************************************************/
// bit flags for grouptable_flags[]
#define SKILLGROUP_CREATION_SELECTABLE	(A)
#define SKILLGROUP_FREE_FOR_ALL			(B)

/**************************************************************************/
// Report Unfound CodeHelp flags
#define	CODEHELP_LOG			(A)// system log
#define	CODEHELP_NO_HELPFILE	(B)// no helpfile tracking file
#define	CODEHELP_IMM			(C)// imms
#define	CODEHELP_ADMIN			(D)// admin
#define	CODEHELP_EVERYONE		(E)// all players
#define	CODEHELP_WIZNET			(F)// display a message on wiznet

#define CODEHELP_ALL_BUT_PLAYERS  (	CODEHELP_LOG	|CODEHELP_NO_HELPFILE \
									|CODEHELP_IMM	|CODEHELP_ADMIN |CODEHELP_WIZNET)

/**************************************************************************/
// language flags
#define LANGFLAG_CHANGED				(A) // language has been edited
#define LANGFLAG_SYSTEM_LANGUAGE		(B) // language used by the system
#define LANGFLAG_NO_SCRAMBLE			(C) // language doesn't scramble the text
#define LANGFLAG_NO_HOLYSPEECH			(D) // holyspeech doesn't work with it
#define LANGFLAG_NO_ORDER				(E) // can't be ordered to change to the language
#define LANGFLAG_REVERSE_TEXT			(F) // text put thru the reverse function
#define LANGFLAG_NO_LANGUAGE_NAME		(G) // the name of the language isn't shown
#define LANGFLAG_NO_SKILL_REQUIRED		(H) // as if you have 100% in the language
#define LANGFLAG_NO_COMMAND_ACCESS		(I) // not accessed as a command
#define LANGFLAG_IMMONLY				(J) // 
#define LANGFLAG_SCRAMBLE_IN_OOC		(K)	// If on, the language can be scrambled in ooc

/**************************************************************************/
// ATTUNE defines
#define ATTUNE_NEED_TO_USE	(A)			// if set, item needs attuning
#define ATTUNE_EQUAL_LEVEL	(B)			// need to be of >= level to object
#define ATTUNE_VANISH		(C)			// extract_obj on failure
#define ATTUNE_TRIVIAL		(D)			// +25%
#define ATTUNE_EASY			(E)			// +10%
#define ATTUNE_HARD			(F)			// -10%
#define ATTUNE_INFURIATING	(G)			// -25%
#define ATTUNE_PREVIOUS		(H)			// Set once an item has been attuned
#define ATTUNE_ONCE_ONLY	(I)			// if ATTUNE_PREVIOUS, can't be reattuned

extern char *target_name;

// Global constants.
extern			struct  class_type		class_table		[MAX_CLASS+1];
extern  const   struct  weapon_type     weapon_table    [];
extern	const	struct	totem_type		totem_table		[TOTEM_MAX+1];
extern  const   struct  item_type       item_table      [];
extern  const   struct  wiznet_type     wiznet_table    [];
extern  const   struct  attack_type     attack_table    [];
extern  const   struct  liq_type        liq_table       [];
extern          struct  skillgroup_type      skillgroup_table     [MAX_SKILLGROUP+1];
extern          struct  social_old_type		social_table    [MAX_SOCIALS];
extern          class	skill_type		skill_table		[MAX_SKILL];
extern          struct  hclss_type		hclass_table	[MAX_SKILL];
extern	const	struct	deity_type		deity_table		[MAX_DEITY];
extern	const	struct	timefield_type	timefield_table	[];
extern	const	struct	modifier_type	modifier_table	[];
extern	char *	const	he_she[];
extern	char *	const	him_her[];
extern	char *	const	his_her[];

// Global variables.
extern          SHOP_DATA         *     shop_first;
extern			cInnData*				pFirstInn;

extern          char_data         *     char_list;
extern          connection_data   *     connection_list;
extern          OBJ_DATA          *     object_list;
extern			MUDPROG_CODE    * mudprog_list;
extern          char                    bug_buf         [];
extern          bool                    fLogAll;
extern          FILE *                  fpReserve;
extern			FILE *          fpAppend2FilReserve;
extern          KILL_DATA               kill_table      [];
extern          char                    log_buf         [];
extern          TIME_INFO_DATA          time_info;
extern          WEATHER_DATA            weather_info	[SECT_MAX];
extern			bool            MOBtrigger;
extern const  struct  directories_type directories_table[];

/**************************************************************************/
// general olc flags
#define OLC_CHANGED    (A)       // whatever is being edited has been modified

/**************************************************************************/

