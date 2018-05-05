/**************************************************************************/
// magic_ra.h -
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#ifndef MAGIC_RA_H
#define MAGIC_RA_H

// function prototypes
DECLARE_DO_FUN( do_spell_imprint           );// needed scribe & brew skills

// spell prototypes
DECLARE_SPELL_FUN( spell_cause_fear        );
DECLARE_SPELL_FUN( spell_summon_guardian   );
DECLARE_SPELL_FUN( spell_starvation        );
DECLARE_SPELL_FUN( spell_dehydration       );
DECLARE_SPELL_FUN( spell_wind_shield       );
DECLARE_SPELL_FUN( spell_sober             );
DECLARE_SPELL_FUN( spell_shadow_breath     );
DECLARE_SPELL_FUN( spell_steel_breath      );
DECLARE_SPELL_FUN( spell_drunkeness        );
DECLARE_SPELL_FUN( spell_permanance        );
DECLARE_SPELL_FUN( spell_element_ring      );  // artifact1
DECLARE_SPELL_FUN( spell_summon_vyr        );  // artifact2
DECLARE_SPELL_FUN( spell_summon_justice    );  // artifact3
DECLARE_SPELL_FUN( spell_thorny_feet       );
DECLARE_SPELL_FUN( spell_unholy_aura       );
DECLARE_SPELL_FUN( spell_fear_magic        );

#endif // MAGIC_RA_H

