/**************************************************************************/
// bitflags.h - defines of A, B, C, D ... ee to binary flag values
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef BITFLAGS_H
#define BITFLAGS_H

// Some macros to handle arrays of bits
#define BIT_PER_ARRAY_ELEMENT (8) // arrays of unsigned char have 8 bits
// BAI = BIT_ARRAY_INDEX
#define BAI(bitarray, bitno) (bitarray[bitno/BIT_PER_ARRAY_ELEMENT]) 
// BIIE = BIT_INDEX_IN_ELEMENT
#define BIIE(bitarray, bitno) (1<<(bitno%BIT_PER_ARRAY_ELEMENT))
// Basic bit manipulation macros
#define IS_SETn(bitarray, bitno)		(BAI(bitarray, bitno) & BIIE(bitarray, bitno))
#define SET_BITn(bitarray, bitno)		(BAI(bitarray, bitno)|= BIIE(bitarray, bitno))
#define REMOVE_BITn(bitarray, bitno)	(BAI(bitarray, bitno)&= ~BIIE(bitarray, bitno))
#define TOGGLE_BITn(bitarray, bitno)	(BAI(bitarray, bitno)^= BIIE(bitarray, bitno))

// some bit macros
#define IS_SET(flag, bit)       ((flag) & (bit))
#define IS_ALL_SET(flag,bits)   ( ((flag)&(bits))==(bits) )
#define SET_BIT(var, bit)       ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))

// Letters for bit flags
#define A	(1)
#define B	(1<< 1)
#define C	(1<< 2)
#define D	(1<< 3)
#define E	(1<< 4)
#define F	(1<< 5)
#define G	(1<< 6)
#define H	(1<< 7)
#define I	(1<< 8)
#define J	(1<< 9)
#define K	(1<<10)
#define L	(1<<11)
#define M	(1<<12)
#define N	(1<<13)
#define O	(1<<14)
#define P	(1<<15)
#define Q	(1<<16)
#define R	(1<<17)
#define S	(1<<18)
#define T	(1<<19)
#define U	(1<<20)
#define V	(1<<21)
#define W	(1<<22)
#define X	(1<<23)
#define Y	(1<<24)
#define Z	(1<<25)
#define aa	(1<<26)
#define bb	(1<<27)
#define cc	(1<<28)
#define dd	(1<<29)
#define ee	(1<<30)
// ee is the limit, adding beyond ee requires internal changes to 
// how all the flags are stored internally in memory

#endif // BITFLAGS_H

