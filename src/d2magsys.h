/**************************************************************************/
// d2magsys.h - dawn magic system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef D2MAGSYS_H
#define D2MAGSYS_H

void set_char_magic_bits(char_data * ch);

// mage realms
#define REALM_ABJURATION    (A)			
#define REALM_ALTERATION    (B)			
#define REALM_CHARM			(C)
#define REALM_CONJURATION   (D)			
#define REALM_ENCHANTMENT	(E)		
#define REALM_EVOCATION		(F)	
#define REALM_ESSENCE		(G)
#define REALM_FORETELLING	(H)		
#define REALM_ILLUSION		(I)	
#define REALM_NECROMANCY	(J)		
#define REALM_PHANTASM		(K)	
#define REALM_SUMMONING		(L)	
#define REALM_WILD			(M)		

// cleric spheres
#define SPHERE_BODY			(A)
#define SPHERE_COMBAT		(B)
#define SPHERE_CONVOCATION	(C)
#define SPHERE_CREATION		(D)
#define SPHERE_DEATH		(E)
#define SPHERE_DIVINATION	(F)
#define SPHERE_ELEMENTAL	(G)
#define SPHERE_HEALING		(H)
#define SPHERE_MIND			(I)
#define SPHERE_NATURE		(J)
#define SPHERE_PROTECTION	(K)
#define SPHERE_TIME			(L)
#define SPHERE_WEATHER		(M)

// druid related seasons or elements
#define ELEMENT_AIR			(A)
#define ELEMENT_ANIMAL		(B)
#define ELEMENT_EARTH		(C)
#define ELEMENT_FIRE		(D)
#define ELEMENT_LAND		(E)
#define ELEMENT_MOON		(F)
#define ELEMENT_PLANT		(G)
#define ELEMENT_SUN			(H)
#define ELEMENT_WATER		(I)
#define SEASON_AUTUMN		(J)
#define SEASON_SPRING		(K)
#define SEASON_SUMMER		(L)
#define SEASON_WINTER		(M)

// bard related
#define COMPOSITION_BEGUILING	(A)
#define COMPOSITION_CEREMONIAL	(B)
#define COMPOSITION_EPIC		(C)
#define COMPOSITION_ESOTERIC	(D)
#define COMPOSITION_ETHEREAL	(E)
#define COMPOSITION_REQUIEM		(F)
#define COMPOSITION_HOLISTIC	(G)

// Spell groupings
#define SPELL_GROUP_SKINS		(A)
#define SPELL_GROUP_FIRE_SHIELD	(B)
#define SPELL_GROUP_ICE_SHIELD	(C)
#define SPELL_GROUP_WIND_SHIELD	(D)
#define SPELL_GROUP_STRENGTH	(E)
#define SPELL_GROUP_BLESSES		(F)
#define SPELL_GROUP_MENTAL		(G)

// Misc stuff
#define MAX_REALMS			4			// how many different types we have

#endif // D2MAGSYS_H
