/**************************************************************************/
// magic.h - 
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

#ifndef MAGIC_H
#define MAGIC_H

#include "magic_ce.h" // Celrion
#include "magic_ja.h" // Jarren
#include "magic_ke.h" // Kerenos
#include "magic_qu.h" // Quenrealther
#include "magic_ra.h" // Rathern
#include "magic_re.h" // Reave
#include "magic_da.h" // Dawn in general
#include "magic_ti.h" // Tibault
#include "magic_sb.h" // Stormbringer
#include "skill_tr.h" // Tristan


/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(  spell_null              );
DECLARE_SPELL_FUN(  spell_acid_blast        );
DECLARE_SPELL_FUN(  spell_armor             );
DECLARE_SPELL_FUN(  spell_bless             );
DECLARE_SPELL_FUN(  spell_blindness         );
DECLARE_SPELL_FUN(  spell_burning_hands     );
DECLARE_SPELL_FUN(  spell_call_lightning    );
DECLARE_SPELL_FUN(  spell_calm              );
DECLARE_SPELL_FUN(  spell_cancellation      );
DECLARE_SPELL_FUN(  spell_cause_critical    );
DECLARE_SPELL_FUN(  spell_cause_light       );
DECLARE_SPELL_FUN(  spell_cause_serious     );
DECLARE_SPELL_FUN(  spell_change_sex        );
DECLARE_SPELL_FUN(  spell_chain_lightning   );
DECLARE_SPELL_FUN(  spell_charm_person      );
DECLARE_SPELL_FUN(  spell_chill_touch       );
DECLARE_SPELL_FUN(  spell_colour_spray      );
DECLARE_SPELL_FUN(  spell_continual_light   );
DECLARE_SPELL_FUN(  spell_control_weather   );
DECLARE_SPELL_FUN(  spell_create_buffet     );
DECLARE_SPELL_FUN(  spell_create_food       );
DECLARE_SPELL_FUN(  spell_create_rose       );
DECLARE_SPELL_FUN(  spell_create_spring     );
DECLARE_SPELL_FUN(  spell_create_water      );
DECLARE_SPELL_FUN(  spell_cure_blindness    );
DECLARE_SPELL_FUN(  spell_cure_critical     );
DECLARE_SPELL_FUN(  spell_cure_disease      );
DECLARE_SPELL_FUN(  spell_cure_light        );
DECLARE_SPELL_FUN(  spell_cure_poison       );
DECLARE_SPELL_FUN(  spell_cure_serious      );
DECLARE_SPELL_FUN(  spell_curse             );
//DECLARE_SPELL_FUN(  spell_demonfire         );
DECLARE_SPELL_FUN(  spell_detect_evil       );
DECLARE_SPELL_FUN(  spell_detect_good       );
DECLARE_SPELL_FUN(  spell_detect_hidden     );
DECLARE_SPELL_FUN(  spell_detect_invis      );
DECLARE_SPELL_FUN(  spell_detect_magic      );
DECLARE_SPELL_FUN(  spell_detect_poison     );
DECLARE_SPELL_FUN(  spell_dispel_evil       );
DECLARE_SPELL_FUN(  spell_dispel_good       );
DECLARE_SPELL_FUN(  spell_dispel_magic      );
DECLARE_SPELL_FUN(  spell_earthquake        );
DECLARE_SPELL_FUN(  spell_enchant_armor     );
DECLARE_SPELL_FUN(  spell_enchant_weapon    );
DECLARE_SPELL_FUN(  spell_energy_drain      );
DECLARE_SPELL_FUN(  spell_faerie_fire       );
DECLARE_SPELL_FUN(  spell_faerie_fog        );
DECLARE_SPELL_FUN(  spell_farsight          );
DECLARE_SPELL_FUN(  spell_fireball          );
DECLARE_SPELL_FUN(  spell_fireproof         );
DECLARE_SPELL_FUN(  spell_flamestrike       );
DECLARE_SPELL_FUN(  spell_floating_disc     );
DECLARE_SPELL_FUN(  spell_fly               );
DECLARE_SPELL_FUN(  spell_frenzy            );
DECLARE_SPELL_FUN(  spell_gate              );
DECLARE_SPELL_FUN(  spell_giant_strength    );
DECLARE_SPELL_FUN(  spell_harm              );
DECLARE_SPELL_FUN(  spell_haste             );
DECLARE_SPELL_FUN(  spell_heal              );
DECLARE_SPELL_FUN(  spell_heat_metal        );
DECLARE_SPELL_FUN(  spell_holy_word         );
DECLARE_SPELL_FUN(  spell_identify          );
DECLARE_SPELL_FUN(  spell_infravision       );
DECLARE_SPELL_FUN(  spell_invisibility      );
DECLARE_SPELL_FUN(  spell_know_alignment    );
DECLARE_SPELL_FUN(  spell_lightning_bolt    );
DECLARE_SPELL_FUN(  spell_locate_object     );
DECLARE_SPELL_FUN(  spell_magic_missile     );
DECLARE_SPELL_FUN(  spell_mass_healing      );
DECLARE_SPELL_FUN(  spell_mass_invis        );
DECLARE_SPELL_FUN(  spell_nexus             );
DECLARE_SPELL_FUN(  spell_pass_door         );
DECLARE_SPELL_FUN(  spell_plague            );
DECLARE_SPELL_FUN(  spell_poison            );
DECLARE_SPELL_FUN(  spell_portal            );
DECLARE_SPELL_FUN(  spell_protection_evil   );
DECLARE_SPELL_FUN(  spell_protection_good   );
DECLARE_SPELL_FUN(  spell_ray_of_truth      );
DECLARE_SPELL_FUN(  spell_recharge          );
DECLARE_SPELL_FUN(  spell_refresh           );
DECLARE_SPELL_FUN(  spell_remove_curse      );
DECLARE_SPELL_FUN(  spell_sanctuary         );
DECLARE_SPELL_FUN(  spell_shocking_grasp    );
DECLARE_SPELL_FUN(  spell_shield            );
DECLARE_SPELL_FUN(  spell_sleep             );
DECLARE_SPELL_FUN(  spell_slow              );
DECLARE_SPELL_FUN(  spell_stone_skin        );
DECLARE_SPELL_FUN(  spell_summon            );
DECLARE_SPELL_FUN(  spell_teleport          );
DECLARE_SPELL_FUN(  spell_ventriloquate     );
DECLARE_SPELL_FUN(  spell_weaken            );
DECLARE_SPELL_FUN(  spell_word_of_recall    );
DECLARE_SPELL_FUN(  spell_acid_breath       );
DECLARE_SPELL_FUN(  spell_fire_breath       );
DECLARE_SPELL_FUN(  spell_frost_breath      );
DECLARE_SPELL_FUN(  spell_gas_breath        );
DECLARE_SPELL_FUN(  spell_shadow_breath     );
DECLARE_SPELL_FUN(  spell_steel_breath      );
DECLARE_SPELL_FUN(  spell_lightning_breath  );
DECLARE_SPELL_FUN(  spell_general_purpose   );
DECLARE_SPELL_FUN(  spell_high_explosive    );

/* Gwynn spells */
DECLARE_SPELL_FUN(  spell_rejuvinate        );
DECLARE_SPELL_FUN(  spell_frostball         );
DECLARE_SPELL_FUN(  spell_rage              );
DECLARE_SPELL_FUN(  spell_animal_essence    );
DECLARE_SPELL_FUN(  spell_poison_rain       );

/* airius spells */
DECLARE_SPELL_FUN(  spell_illusions_grandeur);
DECLARE_SPELL_FUN(  spell_protection_fire   );
DECLARE_SPELL_FUN(  spell_protection_cold   );
DECLARE_SPELL_FUN(  spell_protection_lightning);
DECLARE_SPELL_FUN(  spell_holy_aura         );
DECLARE_SPELL_FUN(  spell_phantasmal_force  );
DECLARE_SPELL_FUN(  spell_improved_phantasm );
DECLARE_SPELL_FUN(  spell_true_sight        );
DECLARE_SPELL_FUN(  spell_vampiric_touch    );
DECLARE_SPELL_FUN(  spell_barkskin          );
DECLARE_SPELL_FUN(  spell_magic_resistance  );
DECLARE_SPELL_FUN(  spell_cone_cold         );
DECLARE_SPELL_FUN(  spell_ice_storm         );
DECLARE_SPELL_FUN(  spell_induce_sleep      );
DECLARE_SPELL_FUN(  spell_wizard_eye        );
DECLARE_SPELL_FUN(  spell_fire_shield       );
DECLARE_SPELL_FUN(  spell_chill_shield      );
DECLARE_SPELL_FUN(  spell_prismatic_spray   );
DECLARE_SPELL_FUN(  spell_rune_edge         );
DECLARE_SPELL_FUN(  spell_runic_blade       );
DECLARE_SPELL_FUN(  spell_drain_blade       );
DECLARE_SPELL_FUN(  spell_empower_blade     );
DECLARE_SPELL_FUN(  spell_flame_blade       );
DECLARE_SPELL_FUN(  spell_frost_blade       );
DECLARE_SPELL_FUN(  spell_regeneration      );
DECLARE_SPELL_FUN(  spell_resist_poison     );
DECLARE_SPELL_FUN(  spell_poison_immunity   );
DECLARE_SPELL_FUN(  spell_wrath             );
 

/* Bonhomme Spells */
DECLARE_SPELL_FUN( spell_chaos_lace         );
DECLARE_SPELL_FUN( spell_mithril_glaze      );
DECLARE_SPELL_FUN( spell_extension          );
//DECLARE_SPELL_FUN( spell_cobble               );


DECLARE_SPELL_FUN( spell_mute			);
DECLARE_SPELL_FUN( spell_possession		);
#endif // MAGIC_H
