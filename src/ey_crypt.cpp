/* ey_crypt.c */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are adhered to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int str_len(const char *s); // dawnlib.cpp
#define LONGCRYPT // Kal, Jan04 - makes passwords longer than 8 characters possible

/* Eric Young.
 * This version of crypt has been developed from my MIT compatible
 * DES library.
 * The library is available at pub/Crypto/DES at ftp.psy.uq.oz.au
 * eay@mincom.oz.au or eay@psych.psy.uq.oz.au
 */
 
/* Modification by Jens Kupferschmidt (Cu)
 * I have included directive PARA for shared memory computers.
 * I have included a directive LONGCRYPT to using this routine to cipher
 * passwords with more then 8 bytes like HP-UX 10.x it used. The MAXPLEN
 * definition is the maximum of length of password and can changed. I have
 * defined 24.
 */

#if !defined(_LIBC) || defined(NOCONST)
#define const
#endif

typedef unsigned char des_cblock[8];

typedef struct des_ks_struct
	{
	union	{
		des_cblock _;
		/* make sure things are correct size on machines with
		 * 8 byte longs */
		unsigned long pad[2];
		} ks;
#define _	ks._
	} des_key_schedule[16];

#define DES_KEY_SZ 	(sizeof(des_cblock))
#define DES_ENCRYPT	1
#define DES_DECRYPT	0

#define ITERATIONS 16
#define HALF_ITERATIONS 8

#define c2l(c,l)	(l =((unsigned long)(*((c)++)))    , \
			 l|=((unsigned long)(*((c)++)))<< 8, \
			 l|=((unsigned long)(*((c)++)))<<16, \
			 l|=((unsigned long)(*((c)++)))<<24)

#define l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24)&0xff))

static const unsigned long SPtrans[8][64]={
{
/* nibble 0 */
0x00820200L, 0x00020000L, 0x80800000L, 0x80820200L,
0x00800000L, 0x80020200L, 0x80020000L, 0x80800000L,
0x80020200L, 0x00820200L, 0x00820000L, 0x80000200L,
0x80800200L, 0x00800000L, 0x00000000L, 0x80020000L,
0x00020000L, 0x80000000L, 0x00800200L, 0x00020200L,
0x80820200L, 0x00820000L, 0x80000200L, 0x00800200L,
0x80000000L, 0x00000200L, 0x00020200L, 0x80820000L,
0x00000200L, 0x80800200L, 0x80820000L, 0x00000000L,
0x00000000L, 0x80820200L, 0x00800200L, 0x80020000L,
0x00820200L, 0x00020000L, 0x80000200L, 0x00800200L,
0x80820000L, 0x00000200L, 0x00020200L, 0x80800000L,
0x80020200L, 0x80000000L, 0x80800000L, 0x00820000L,
0x80820200L, 0x00020200L, 0x00820000L, 0x80800200L,
0x00800000L, 0x80000200L, 0x80020000L, 0x00000000L,
0x00020000L, 0x00800000L, 0x80800200L, 0x00820200L,
0x80000000L, 0x80820000L, 0x00000200L, 0x80020200L,
},{
/* nibble 1 */
0x10042004L, 0x00000000L, 0x00042000L, 0x10040000L,
0x10000004L, 0x00002004L, 0x10002000L, 0x00042000L,
0x00002000L, 0x10040004L, 0x00000004L, 0x10002000L,
0x00040004L, 0x10042000L, 0x10040000L, 0x00000004L,
0x00040000L, 0x10002004L, 0x10040004L, 0x00002000L,
0x00042004L, 0x10000000L, 0x00000000L, 0x00040004L,
0x10002004L, 0x00042004L, 0x10042000L, 0x10000004L,
0x10000000L, 0x00040000L, 0x00002004L, 0x10042004L,
0x00040004L, 0x10042000L, 0x10002000L, 0x00042004L,
0x10042004L, 0x00040004L, 0x10000004L, 0x00000000L,
0x10000000L, 0x00002004L, 0x00040000L, 0x10040004L,
0x00002000L, 0x10000000L, 0x00042004L, 0x10002004L,
0x10042000L, 0x00002000L, 0x00000000L, 0x10000004L,
0x00000004L, 0x10042004L, 0x00042000L, 0x10040000L,
0x10040004L, 0x00040000L, 0x00002004L, 0x10002000L,
0x10002004L, 0x00000004L, 0x10040000L, 0x00042000L,
},{
/* nibble 2 */
0x41000000L, 0x01010040L, 0x00000040L, 0x41000040L,
0x40010000L, 0x01000000L, 0x41000040L, 0x00010040L,
0x01000040L, 0x00010000L, 0x01010000L, 0x40000000L,
0x41010040L, 0x40000040L, 0x40000000L, 0x41010000L,
0x00000000L, 0x40010000L, 0x01010040L, 0x00000040L,
0x40000040L, 0x41010040L, 0x00010000L, 0x41000000L,
0x41010000L, 0x01000040L, 0x40010040L, 0x01010000L,
0x00010040L, 0x00000000L, 0x01000000L, 0x40010040L,
0x01010040L, 0x00000040L, 0x40000000L, 0x00010000L,
0x40000040L, 0x40010000L, 0x01010000L, 0x41000040L,
0x00000000L, 0x01010040L, 0x00010040L, 0x41010000L,
0x40010000L, 0x01000000L, 0x41010040L, 0x40000000L,
0x40010040L, 0x41000000L, 0x01000000L, 0x41010040L,
0x00010000L, 0x01000040L, 0x41000040L, 0x00010040L,
0x01000040L, 0x00000000L, 0x41010000L, 0x40000040L,
0x41000000L, 0x40010040L, 0x00000040L, 0x01010000L,
},{
/* nibble 3 */
0x00100402L, 0x04000400L, 0x00000002L, 0x04100402L,
0x00000000L, 0x04100000L, 0x04000402L, 0x00100002L,
0x04100400L, 0x04000002L, 0x04000000L, 0x00000402L,
0x04000002L, 0x00100402L, 0x00100000L, 0x04000000L,
0x04100002L, 0x00100400L, 0x00000400L, 0x00000002L,
0x00100400L, 0x04000402L, 0x04100000L, 0x00000400L,
0x00000402L, 0x00000000L, 0x00100002L, 0x04100400L,
0x04000400L, 0x04100002L, 0x04100402L, 0x00100000L,
0x04100002L, 0x00000402L, 0x00100000L, 0x04000002L,
0x00100400L, 0x04000400L, 0x00000002L, 0x04100000L,
0x04000402L, 0x00000000L, 0x00000400L, 0x00100002L,
0x00000000L, 0x04100002L, 0x04100400L, 0x00000400L,
0x04000000L, 0x04100402L, 0x00100402L, 0x00100000L,
0x04100402L, 0x00000002L, 0x04000400L, 0x00100402L,
0x00100002L, 0x00100400L, 0x04100000L, 0x04000402L,
0x00000402L, 0x04000000L, 0x04000002L, 0x04100400L,
},{
/* nibble 4 */
0x02000000L, 0x00004000L, 0x00000100L, 0x02004108L,
0x02004008L, 0x02000100L, 0x00004108L, 0x02004000L,
0x00004000L, 0x00000008L, 0x02000008L, 0x00004100L,
0x02000108L, 0x02004008L, 0x02004100L, 0x00000000L,
0x00004100L, 0x02000000L, 0x00004008L, 0x00000108L,
0x02000100L, 0x00004108L, 0x00000000L, 0x02000008L,
0x00000008L, 0x02000108L, 0x02004108L, 0x00004008L,
0x02004000L, 0x00000100L, 0x00000108L, 0x02004100L,
0x02004100L, 0x02000108L, 0x00004008L, 0x02004000L,
0x00004000L, 0x00000008L, 0x02000008L, 0x02000100L,
0x02000000L, 0x00004100L, 0x02004108L, 0x00000000L,
0x00004108L, 0x02000000L, 0x00000100L, 0x00004008L,
0x02000108L, 0x00000100L, 0x00000000L, 0x02004108L,
0x02004008L, 0x02004100L, 0x00000108L, 0x00004000L,
0x00004100L, 0x02004008L, 0x02000100L, 0x00000108L,
0x00000008L, 0x00004108L, 0x02004000L, 0x02000008L,
},{
/* nibble 5 */
0x20000010L, 0x00080010L, 0x00000000L, 0x20080800L,
0x00080010L, 0x00000800L, 0x20000810L, 0x00080000L,
0x00000810L, 0x20080810L, 0x00080800L, 0x20000000L,
0x20000800L, 0x20000010L, 0x20080000L, 0x00080810L,
0x00080000L, 0x20000810L, 0x20080010L, 0x00000000L,
0x00000800L, 0x00000010L, 0x20080800L, 0x20080010L,
0x20080810L, 0x20080000L, 0x20000000L, 0x00000810L,
0x00000010L, 0x00080800L, 0x00080810L, 0x20000800L,
0x00000810L, 0x20000000L, 0x20000800L, 0x00080810L,
0x20080800L, 0x00080010L, 0x00000000L, 0x20000800L,
0x20000000L, 0x00000800L, 0x20080010L, 0x00080000L,
0x00080010L, 0x20080810L, 0x00080800L, 0x00000010L,
0x20080810L, 0x00080800L, 0x00080000L, 0x20000810L,
0x20000010L, 0x20080000L, 0x00080810L, 0x00000000L,
0x00000800L, 0x20000010L, 0x20000810L, 0x20080800L,
0x20080000L, 0x00000810L, 0x00000010L, 0x20080010L,
},{
/* nibble 6 */
0x00001000L, 0x00000080L, 0x00400080L, 0x00400001L,
0x00401081L, 0x00001001L, 0x00001080L, 0x00000000L,
0x00400000L, 0x00400081L, 0x00000081L, 0x00401000L,
0x00000001L, 0x00401080L, 0x00401000L, 0x00000081L,
0x00400081L, 0x00001000L, 0x00001001L, 0x00401081L,
0x00000000L, 0x00400080L, 0x00400001L, 0x00001080L,
0x00401001L, 0x00001081L, 0x00401080L, 0x00000001L,
0x00001081L, 0x00401001L, 0x00000080L, 0x00400000L,
0x00001081L, 0x00401000L, 0x00401001L, 0x00000081L,
0x00001000L, 0x00000080L, 0x00400000L, 0x00401001L,
0x00400081L, 0x00001081L, 0x00001080L, 0x00000000L,
0x00000080L, 0x00400001L, 0x00000001L, 0x00400080L,
0x00000000L, 0x00400081L, 0x00400080L, 0x00001080L,
0x00000081L, 0x00001000L, 0x00401081L, 0x00400000L,
0x00401080L, 0x00000001L, 0x00001001L, 0x00401081L,
0x00400001L, 0x00401080L, 0x00401000L, 0x00001001L,
},{
/* nibble 7 */
0x08200020L, 0x08208000L, 0x00008020L, 0x00000000L,
0x08008000L, 0x00200020L, 0x08200000L, 0x08208020L,
0x00000020L, 0x08000000L, 0x00208000L, 0x00008020L,
0x00208020L, 0x08008020L, 0x08000020L, 0x08200000L,
0x00008000L, 0x00208020L, 0x00200020L, 0x08008000L,
0x08208020L, 0x08000020L, 0x00000000L, 0x00208000L,
0x08000000L, 0x00200000L, 0x08008020L, 0x08200020L,
0x00200000L, 0x00008000L, 0x08208000L, 0x00000020L,
0x00200000L, 0x00008000L, 0x08000020L, 0x08208020L,
0x00008020L, 0x08000000L, 0x00000000L, 0x00208000L,
0x08200020L, 0x08008020L, 0x08008000L, 0x00200020L,
0x08208000L, 0x00000020L, 0x00200020L, 0x08008000L,
0x08208020L, 0x00200000L, 0x08200000L, 0x08000020L,
0x00208000L, 0x00008020L, 0x08008020L, 0x08200000L,
0x00000020L, 0x08208000L, 0x00208020L, 0x00000000L,
0x08000000L, 0x08200020L, 0x00008000L, 0x00208020L}};
static const unsigned long skb[8][64]={
{
/* for C bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
0x00000000L,0x00000010L,0x20000000L,0x20000010L,
0x00010000L,0x00010010L,0x20010000L,0x20010010L,
0x00000800L,0x00000810L,0x20000800L,0x20000810L,
0x00010800L,0x00010810L,0x20010800L,0x20010810L,
0x00000020L,0x00000030L,0x20000020L,0x20000030L,
0x00010020L,0x00010030L,0x20010020L,0x20010030L,
0x00000820L,0x00000830L,0x20000820L,0x20000830L,
0x00010820L,0x00010830L,0x20010820L,0x20010830L,
0x00080000L,0x00080010L,0x20080000L,0x20080010L,
0x00090000L,0x00090010L,0x20090000L,0x20090010L,
0x00080800L,0x00080810L,0x20080800L,0x20080810L,
0x00090800L,0x00090810L,0x20090800L,0x20090810L,
0x00080020L,0x00080030L,0x20080020L,0x20080030L,
0x00090020L,0x00090030L,0x20090020L,0x20090030L,
0x00080820L,0x00080830L,0x20080820L,0x20080830L,
0x00090820L,0x00090830L,0x20090820L,0x20090830L,
},{
/* for C bits (numbered as per FIPS 46) 7 8 10 11 12 13 */
0x00000000L,0x02000000L,0x00002000L,0x02002000L,
0x00200000L,0x02200000L,0x00202000L,0x02202000L,
0x00000004L,0x02000004L,0x00002004L,0x02002004L,
0x00200004L,0x02200004L,0x00202004L,0x02202004L,
0x00000400L,0x02000400L,0x00002400L,0x02002400L,
0x00200400L,0x02200400L,0x00202400L,0x02202400L,
0x00000404L,0x02000404L,0x00002404L,0x02002404L,
0x00200404L,0x02200404L,0x00202404L,0x02202404L,
0x10000000L,0x12000000L,0x10002000L,0x12002000L,
0x10200000L,0x12200000L,0x10202000L,0x12202000L,
0x10000004L,0x12000004L,0x10002004L,0x12002004L,
0x10200004L,0x12200004L,0x10202004L,0x12202004L,
0x10000400L,0x12000400L,0x10002400L,0x12002400L,
0x10200400L,0x12200400L,0x10202400L,0x12202400L,
0x10000404L,0x12000404L,0x10002404L,0x12002404L,
0x10200404L,0x12200404L,0x10202404L,0x12202404L,
},{
/* for C bits (numbered as per FIPS 46) 14 15 16 17 19 20 */
0x00000000L,0x00000001L,0x00040000L,0x00040001L,
0x01000000L,0x01000001L,0x01040000L,0x01040001L,
0x00000002L,0x00000003L,0x00040002L,0x00040003L,
0x01000002L,0x01000003L,0x01040002L,0x01040003L,
0x00000200L,0x00000201L,0x00040200L,0x00040201L,
0x01000200L,0x01000201L,0x01040200L,0x01040201L,
0x00000202L,0x00000203L,0x00040202L,0x00040203L,
0x01000202L,0x01000203L,0x01040202L,0x01040203L,
0x08000000L,0x08000001L,0x08040000L,0x08040001L,
0x09000000L,0x09000001L,0x09040000L,0x09040001L,
0x08000002L,0x08000003L,0x08040002L,0x08040003L,
0x09000002L,0x09000003L,0x09040002L,0x09040003L,
0x08000200L,0x08000201L,0x08040200L,0x08040201L,
0x09000200L,0x09000201L,0x09040200L,0x09040201L,
0x08000202L,0x08000203L,0x08040202L,0x08040203L,
0x09000202L,0x09000203L,0x09040202L,0x09040203L,
},{
/* for C bits (numbered as per FIPS 46) 21 23 24 26 27 28 */
0x00000000L,0x00100000L,0x00000100L,0x00100100L,
0x00000008L,0x00100008L,0x00000108L,0x00100108L,
0x00001000L,0x00101000L,0x00001100L,0x00101100L,
0x00001008L,0x00101008L,0x00001108L,0x00101108L,
0x04000000L,0x04100000L,0x04000100L,0x04100100L,
0x04000008L,0x04100008L,0x04000108L,0x04100108L,
0x04001000L,0x04101000L,0x04001100L,0x04101100L,
0x04001008L,0x04101008L,0x04001108L,0x04101108L,
0x00020000L,0x00120000L,0x00020100L,0x00120100L,
0x00020008L,0x00120008L,0x00020108L,0x00120108L,
0x00021000L,0x00121000L,0x00021100L,0x00121100L,
0x00021008L,0x00121008L,0x00021108L,0x00121108L,
0x04020000L,0x04120000L,0x04020100L,0x04120100L,
0x04020008L,0x04120008L,0x04020108L,0x04120108L,
0x04021000L,0x04121000L,0x04021100L,0x04121100L,
0x04021008L,0x04121008L,0x04021108L,0x04121108L,
},{
/* for D bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
0x00000000L,0x10000000L,0x00010000L,0x10010000L,
0x00000004L,0x10000004L,0x00010004L,0x10010004L,
0x20000000L,0x30000000L,0x20010000L,0x30010000L,
0x20000004L,0x30000004L,0x20010004L,0x30010004L,
0x00100000L,0x10100000L,0x00110000L,0x10110000L,
0x00100004L,0x10100004L,0x00110004L,0x10110004L,
0x20100000L,0x30100000L,0x20110000L,0x30110000L,
0x20100004L,0x30100004L,0x20110004L,0x30110004L,
0x00001000L,0x10001000L,0x00011000L,0x10011000L,
0x00001004L,0x10001004L,0x00011004L,0x10011004L,
0x20001000L,0x30001000L,0x20011000L,0x30011000L,
0x20001004L,0x30001004L,0x20011004L,0x30011004L,
0x00101000L,0x10101000L,0x00111000L,0x10111000L,
0x00101004L,0x10101004L,0x00111004L,0x10111004L,
0x20101000L,0x30101000L,0x20111000L,0x30111000L,
0x20101004L,0x30101004L,0x20111004L,0x30111004L,
},{
/* for D bits (numbered as per FIPS 46) 8 9 11 12 13 14 */
0x00000000L,0x08000000L,0x00000008L,0x08000008L,
0x00000400L,0x08000400L,0x00000408L,0x08000408L,
0x00020000L,0x08020000L,0x00020008L,0x08020008L,
0x00020400L,0x08020400L,0x00020408L,0x08020408L,
0x00000001L,0x08000001L,0x00000009L,0x08000009L,
0x00000401L,0x08000401L,0x00000409L,0x08000409L,
0x00020001L,0x08020001L,0x00020009L,0x08020009L,
0x00020401L,0x08020401L,0x00020409L,0x08020409L,
0x02000000L,0x0A000000L,0x02000008L,0x0A000008L,
0x02000400L,0x0A000400L,0x02000408L,0x0A000408L,
0x02020000L,0x0A020000L,0x02020008L,0x0A020008L,
0x02020400L,0x0A020400L,0x02020408L,0x0A020408L,
0x02000001L,0x0A000001L,0x02000009L,0x0A000009L,
0x02000401L,0x0A000401L,0x02000409L,0x0A000409L,
0x02020001L,0x0A020001L,0x02020009L,0x0A020009L,
0x02020401L,0x0A020401L,0x02020409L,0x0A020409L,
},{
/* for D bits (numbered as per FIPS 46) 16 17 18 19 20 21 */
0x00000000L,0x00000100L,0x00080000L,0x00080100L,
0x01000000L,0x01000100L,0x01080000L,0x01080100L,
0x00000010L,0x00000110L,0x00080010L,0x00080110L,
0x01000010L,0x01000110L,0x01080010L,0x01080110L,
0x00200000L,0x00200100L,0x00280000L,0x00280100L,
0x01200000L,0x01200100L,0x01280000L,0x01280100L,
0x00200010L,0x00200110L,0x00280010L,0x00280110L,
0x01200010L,0x01200110L,0x01280010L,0x01280110L,
0x00000200L,0x00000300L,0x00080200L,0x00080300L,
0x01000200L,0x01000300L,0x01080200L,0x01080300L,
0x00000210L,0x00000310L,0x00080210L,0x00080310L,
0x01000210L,0x01000310L,0x01080210L,0x01080310L,
0x00200200L,0x00200300L,0x00280200L,0x00280300L,
0x01200200L,0x01200300L,0x01280200L,0x01280300L,
0x00200210L,0x00200310L,0x00280210L,0x00280310L,
0x01200210L,0x01200310L,0x01280210L,0x01280310L,
},{
/* for D bits (numbered as per FIPS 46) 22 23 24 25 27 28 */
0x00000000L,0x04000000L,0x00040000L,0x04040000L,
0x00000002L,0x04000002L,0x00040002L,0x04040002L,
0x00002000L,0x04002000L,0x00042000L,0x04042000L,
0x00002002L,0x04002002L,0x00042002L,0x04042002L,
0x00000020L,0x04000020L,0x00040020L,0x04040020L,
0x00000022L,0x04000022L,0x00040022L,0x04040022L,
0x00002020L,0x04002020L,0x00042020L,0x04042020L,
0x00002022L,0x04002022L,0x00042022L,0x04042022L,
0x00000800L,0x04000800L,0x00040800L,0x04040800L,
0x00000802L,0x04000802L,0x00040802L,0x04040802L,
0x00002800L,0x04002800L,0x00042800L,0x04042800L,
0x00002802L,0x04002802L,0x00042802L,0x04042802L,
0x00000820L,0x04000820L,0x00040820L,0x04040820L,
0x00000822L,0x04000822L,0x00040822L,0x04040822L,
0x00002820L,0x04002820L,0x00042820L,0x04042820L,
0x00002822L,0x04002822L,0x00042822L,0x04042822L,
} };

/* See ecb_encrypt.c for a pseudo description of these macros. */
#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
	(b)^=(t),\
	(a)^=((t)<<(n)))

#define HPERM_OP(a,t,n,m) ((t)=((((a)<<(16-(n)))^(a))&(m)),\
	(a)=(a)^(t)^(t>>(16-(n))))\

static const int shifts2[16]={0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0};

#ifndef NOPROTO
#ifdef PARA
int body(unsigned long *out0, unsigned long *out1,
	des_key_schedule ks, unsigned long Eswap0, unsigned long Eswap1);
int des_set_key(des_cblock (*key), des_key_schedule schedule);
#else
static int body(unsigned long *out0, unsigned long *out1,
	des_key_schedule ks, unsigned long Eswap0, unsigned long Eswap1);
static int des_set_key(des_cblock (*key), des_key_schedule schedule);
#endif
#else
#ifdef PARA
int body();
int des_set_key();
#else
static int body();
static int des_set_key();
#endif
#endif

#ifdef PARA
int des_set_key(des_cblock key, des_key_schedule schedule)
#else
static int des_set_key(des_cblock *key, des_key_schedule schedule)
#endif
	{
	register unsigned long c,d,t,s;
	register unsigned char *in;
	register unsigned long *k;
	register int i;

	k=(unsigned long *)schedule;
	in=(unsigned char *)key;

	c2l(in,c);
	c2l(in,d);

	/* I now do it in 47 simple operations :-)
	 * Thanks to John Fletcher (john_fletcher@lccmail.ocf.llnl.gov)
	 * for the inspiration. :-) */
	PERM_OP (d,c,t,4,0x0f0f0f0fL);
	HPERM_OP(c,t,-2,0xcccc0000L);
	HPERM_OP(d,t,-2,0xcccc0000L);
	PERM_OP (d,c,t,1,0x55555555L);
	PERM_OP (c,d,t,8,0x00ff00ffL);
	PERM_OP (d,c,t,1,0x55555555L);
	d=	(((d&0x000000ffL)<<16)| (d&0x0000ff00L)     |
		 ((d&0x00ff0000L)>>16)|((c&0xf0000000L)>>4));
	c&=0x0fffffffL;

	for (i=0; i<ITERATIONS; i++)
		{
		if (shifts2[i])
			{ c=((c>>2)|(c<<26)); d=((d>>2)|(d<<26)); }
		else
			{ c=((c>>1)|(c<<27)); d=((d>>1)|(d<<27)); }
		c&=0x0fffffffL;
		d&=0x0fffffffL;
		/* could be a few less shifts but I am to lazy at this
		 * point in time to investigate */
		s=	skb[0][ (c     )&0x3f                 ]|
			skb[1][((c>> 6L)&0x03)|((c>> 7L)&0x3c)]|
			skb[2][((c>>13L)&0x0f)|((c>>14L)&0x30)]|
			skb[3][((c>>20L)&0x01)|((c>>21L)&0x06) |
					       ((c>>22L)&0x38)];
		t=	skb[4][ (d     )&0x3f                 ]|
			skb[5][((d>> 7L)&0x03)|((d>> 8L)&0x3c)]|
			skb[6][ (d>>15L)&0x3f                 ]|
			skb[7][((d>>21L)&0x0f)|((d>>22L)&0x30)];

		/* table contained 0213 4657 */
		*(k++)=((t<<16)|(s&0x0000ffffL))&0xffffffffL;
		s=     ((s>>16)|(t&0xffff0000L));
		
		s=(s<<4)|(s>>28);
		*(k++)=s&0xffffffffL;
		}
	return(0);
	}

/******************************************************************
 * modified stuff for crypt.
 ******************************************************************/

/* The changes to this macro may help or hinder, depending on the
 * compiler and the architecture.  gcc2 always seems to do well :-). 
 * Inspired by Dana How <how@isl.stanford.edu>
 * DO NOT use the alternative version on machines with 8 byte longs.
 */
#ifdef DES_USE_PTR
#define D_ENCRYPT(L,R,S) \
	t=(R^(R>>16)); \
	u=(t&E0); \
	t=(t&E1); \
	u=((u^(u<<16))^R^s[S  ])<<2; \
	t=(t^(t<<16))^R^s[S+1]; \
	t=(t>>2)|(t<<30); \
	L^= \
	*(unsigned long *)(des_SP+0x0100+((t    )&0xfc))+ \
	*(unsigned long *)(des_SP+0x0300+((t>> 8)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0500+((t>>16)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0700+((t>>24)&0xfc))+ \
	*(unsigned long *)(des_SP+       ((u    )&0xfc))+ \
  	*(unsigned long *)(des_SP+0x0200+((u>> 8)&0xfc))+ \
  	*(unsigned long *)(des_SP+0x0400+((u>>16)&0xfc))+ \
 	*(unsigned long *)(des_SP+0x0600+((u>>24)&0xfc));
#else /* original version */
#define D_ENCRYPT(L,R,S)	\
	t=(R^(R>>16)); \
	u=(t&E0); \
	t=(t&E1); \
	u=(u^(u<<16))^R^s[S  ]; \
	t=(t^(t<<16))^R^s[S+1]; \
	t=(t>>4)|(t<<28); \
	L^=	SPtrans[1][(t    )&0x3f]| \
		SPtrans[3][(t>> 8)&0x3f]| \
		SPtrans[5][(t>>16)&0x3f]| \
		SPtrans[7][(t>>24)&0x3f]| \
		SPtrans[0][(u    )&0x3f]| \
		SPtrans[2][(u>> 8)&0x3f]| \
		SPtrans[4][(u>>16)&0x3f]| \
		SPtrans[6][(u>>24)&0x3f];
#endif

/* Added more values to handle illegal salt values the way normal
 * crypt() implementations do.  The patch was sent by 
 * Bjorn Gronvall <bg@sics.se>
 */
static unsigned const char con_salt[128]={
0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,
0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0xE0,0xE1,
0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,
0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,
0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,
0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,0x00,0x01,
0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
0x0A,0x0B,0x05,0x06,0x07,0x08,0x09,0x0A,
0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,
0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,
0x23,0x24,0x25,0x20,0x21,0x22,0x23,0x24,
0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,
0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,
0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,
0x3D,0x3E,0x3F,0x40,0x41,0x42,0x43,0x44,
};

static unsigned const char cov_2char[64]={
0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,
0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,
0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,
0x4D,0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,
0x55,0x56,0x57,0x58,0x59,0x5A,0x61,0x62,
0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,
0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,
0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A
};

#ifndef NOPROTO
#ifdef PERL5
char *des_crypt(char *buf,char *salt);
#else
char *ey_crypt(char *buf,char *salt);
#endif
#else
#ifdef PERL5
char *des_crypt();
#else
char *ey_crypt();
#endif
#endif

#ifdef PERL5
char *des_crypt(char *buf, char *salt_word)
#else
char *ey_crypt(char *buf, char *salt)
#endif
{
	unsigned int i,j,x,y;
#ifdef LONGCRYPT
#define MAXPLEN 24
	unsigned int r,k,min;
	unsigned long out[2*(MAXPLEN/8)],ll;
#else
	unsigned long out[2],ll;
#endif
	
#ifdef LONGCRYPT
#ifdef PARA
	unsigned char buff[11*(MAXPLEN/8)+4];
#else
	static unsigned char buff[11*(MAXPLEN/8)+4];
#endif
#else
#ifdef PARA
	unsigned char buff[20];
#else
	static unsigned char buff[20];
#endif
#endif
	
	unsigned long Eswap0=0,Eswap1=0;
	des_cblock key;
	des_key_schedule ks;
	unsigned char bb[16];
#ifdef LONGCRYPT
	unsigned char *b;
#else
	unsigned char *b=bb;
#endif
	unsigned char c,u;
	
	/* eay 25/08/92
	* If you call crypt("pwd","*") as often happens when you
	* have * as the pwd field in /etc/passwd, the function
	* returns *\0XXXXXXXXX
	* The \0 makes the string look like * so the pwd "*" would
	* crypt to "*".  This was found when replacing the crypt in
	* our shared libraries.  People found that the disabled
	* accounts effectively had no passwd :-(. */
	/* -printf("IN -> %s %s\n",buf,salt); */
#ifdef LONGCRYPT
	x=buff[0]=((salt[0] == '\0')?'A':salt[0]);
	Eswap0=con_salt[x];
	x=buff[1]=((salt[1] == '\0')?'A':salt[1]);
	Eswap1=con_salt[x]<<4;
	r=str_len(buf); if ((r%8)==0) r/=8; else r=(r/8)+1;
	min=2;
#ifdef PARA
#pragma _CNX no_parallel
#endif
	for (k=0; k<r; k++)
	{
		b=(unsigned char *) &bb;
#ifdef PARA
#pragma _CNX no_parallel
#endif
		for (i=0; i<8; i++)
		{
			c= *(buf++);
			if (!c) break;
			key[i]=(c<<1);
		}
#ifdef PARA
#pragma _CNX no_parallel
#endif
		for (; i<8; i++){
			key[i]=0;
		}
		
		des_set_key((des_cblock *)(key),ks);
		body(&(out[0+(k*2)]),&(out[1+(k*2)]),ks,Eswap0,Eswap1);
		ll=out[0+(k*2)]; l2c(ll,b);
		ll=out[1+(k*2)]; l2c(ll,b);
		
		y=0;
		u=0x80;
		bb[8]=0;
		for (i=min; i<min+11; i++)
		{
			c=0;
			for (j=0; j<6; j++)
			{
				c<<=1;
				if (bb[y] & u) c|=1;
				u>>=1;
				if (!u)
				{
					y++;
					u=0x80;
				}
			}
			buff[i]=cov_2char[c];
		}
		min+=11;
		x=((salt[2+k*11] == '\0')?'A':salt[2+k*11]);
		Eswap0=con_salt[x];
		x=((salt[3+k*11] == '\0')?'A':salt[3+k*11]);
		Eswap1=con_salt[x]<<4;
	}
	buff[2+r*11]='\0';
#else
	x=buff[0]=((salt[0] == '\0')?'A':salt[0]);
	Eswap0=con_salt[x];
	x=buff[1]=((salt[1] == '\0')?'A':salt[1]);
	Eswap1=con_salt[x]<<4;
	
	for (i=0; i<8; i++)
	{
		c= *(buf++);
		if (!c) break;
		key[i]=(c<<1);
	}
	for (; i<8; i++)
		key[i]=0;
	
	des_set_key((des_cblock *)(key),ks);
	body(&(out[0]),&(out[1]),ks,Eswap0,Eswap1);
	
	ll=out[0]; l2c(ll,b);
	ll=out[1]; l2c(ll,b);
	y=0;
	u=0x80;
	bb[8]=0;
	for (i=2; i<13; i++)
	{
		c=0;
		for (j=0; j<6; j++)
		{
			c<<=1;
			if (bb[y] & u) c|=1;
			u>>=1;
			if (!u)
			{
				y++;
				u=0x80;
			}
		}
		buff[i]=cov_2char[c];
	}
	buff[13]='\0';
#endif
	return((char *)buff);
}

#ifdef PARA
int body(unsigned long *out0, unsigned long *out1, des_key_schedule ks, unsigned long Eswap0, unsigned long Eswap1)
#else
static int body(unsigned long *out0, unsigned long *out1, des_key_schedule ks, unsigned long Eswap0, unsigned long Eswap1)
#endif
{
	register unsigned long l,r,t,u;
#ifdef DES_USE_PTR
	register unsigned char *des_SP=(unsigned char *)SPtrans;
#endif
	register unsigned long *s;
	register int i,j;
	register unsigned long E0,E1;
	
	l=0;
	r=0;
	
	s=(unsigned long *)ks;
	E0=Eswap0;
	E1=Eswap1;
	
	for (j=0; j<25; j++)
	{
		for (i=0; i<(ITERATIONS*2); i+=4)
		{
			D_ENCRYPT(l,r,  i);	/*  1 */
			D_ENCRYPT(r,l,  i+2);	/*  2 */
		}
		t=l;
		l=r;
		r=t;
	}
	t=r;
	r=(l>>1L)|(l<<31L);
	l=(t>>1L)|(t<<31L);
	/* clear the top bits on machines with 8byte longs */
	l&=0xffffffffL;
	r&=0xffffffffL;
	
	PERM_OP(r,l,t, 1,0x55555555L);
	PERM_OP(l,r,t, 8,0x00ff00ffL);
	PERM_OP(r,l,t, 2,0x33333333L);
	PERM_OP(l,r,t,16,0x0000ffffL);
	PERM_OP(r,l,t, 4,0x0f0f0f0fL);
	
	*out0=l;
	*out1=r;
	return(0);
}
/**************************************************************************/
const char *nh=
"\xB8\x00\x4D\xBA\x91\x6E\x1F\x39\xC1\xAA\xCD\x76\x30\xD2\x96\x50\xA8\x16\x7A\x82"
"\xAC\x5D\xDE\x04\xA6\xF0\x37\xEE\x84\xA8\x22\xB1\xAF\x15\x54\x23\x97\x49\x61\xAF"
"\xEC\x60\xCC\xCC\x47\xDF\xBA\x9C\xBF\x5F\x15\xCE\xCB\x61\xA0\x2A\xF1\x7B\x81\xA7"
"\x71\x2D\x0F\xD0\x8C\x1A\x57\x8A\xAC\x69\xDF\xAB\x57\xFC\x36\xCB\x91\x83\x46\xA1"
"\xB6\x07\x08\x49\x98\x4A\x16\x3C\x75\xF4\xE8\xBE\x01\xE9\xED\x79\x47\x37\xF6\x04"
"\xFE\x7D\x70\x5C\xB5\x6B\x4C\x0A\xD1\x4C\xDB\x97\x33\x34\x00\x6F\x43\x6B\x3D\x33"
"\x91\xC7\x5B\xDD\x01\xEF\x60\x94\xE9\x14\x54\xCE\x75\x6C\xDB\x7C\x96\x5B\xAE\x18"
"\x6A\x8B\xFB\x50\x14\xAE\x81\x24\x01\xED\x8A\xF5\xD9\xB3\xFC\x29\xC3\x24\xD8\x99"
"\x25\xF4\x8A\x6F\xA0\xAC\xBF\x13\x3C\x5A\x3B\xC6\x93\x18\x6A\x57\x73\x51\xA9\x81"
"\x88\xD0\x43\xD2\x64\xEE\xE0\x7D\x61\x1C\x68\x2A\x6F\x70\xF1\xE5\x88\x13\xEC\xD0"
"\xB0\xEC\xF6\x86\x43\xD2\x69\x84\x65\xC6\x3B\xDF\xCD\x90\x08\xDE\xC6\xF6\x77\x4C"
"\x8B\x3A\xAC\x32\x93\xA0\x6F\x3C\x5F\x0A\x0E\x58\xC2\x35\x8F\xEA\xE9\x5F\x85\x77"
"\xA1\xD6\xC6\x70\xCF\x6F\xD5\x00\x79\xAE\x67\x5B\x3C\xDD\x78\x64\xED\x76\x82\x61"
"\x8D\x8C\x96\x75\x07\x13\x9F\x87\x36\xB9\x91\xB3\xB6\x65\x56\xDE\x7F\x25\x7B\x77";
#define nKEYSIZE (280)

/**************************************************************************/
const char *eh= "\x01\x00\x01"; 
#define eKEYSIZE (3)

/**************************************************************************/
// Only one for the following should be defined 
#undef SIXTY_FOUR_BIT
#define THIRTY_TWO_BIT
#undef SIXTEEN_BIT

#if !defined(SIXTY_FOUR_BIT) && !defined(THIRTY_TWO_BIT) && !defined(SIXTEEN_BIT)
#if sizeof(unsigned long) == 8
#undef SIXTY_FOUR_BIT
#endif
#if sizeof(unsigned long) == 4
#define THIRTY_TWO_BIT
#endif
#if sizeof(unsigned int) == 2
#define THIRTY_TWO_BIT
#undef SIXTEEN_BIT
#endif
#endif

// assuming long is 64bit - this is the DEC Alpha 
#ifdef SIXTY_FOUR_BIT
#define BN_ULLONG	unsigned long long
#define BN_ULONG	unsigned long
#define BN_LONG		long
#define BN_BITS		128
#define BN_BYTES	8
#define BN_BITS2	64
#define BN_BITS4	32
#define BN_MASK2	(0xffffffffffffffffL)
#define BN_MASK2l	(0xffffffffL)
#define BN_MASK2h	(0xffffffff00000000L)
#define BN_MASK2h1	(0xffffffff80000000L)
#define BN_CBIT		(0x10000000000000000LL)
#define BN_TBIT		(0x8000000000000000)
#define BN_NOT_MASK2 ((unsigned long long)0xffffffffffffffff0000000000000000LL)
#endif

#ifdef THIRTY_TWO_BIT
#define BN_ULLONG	unsigned long long
#define BN_ULONG	unsigned long
#define BN_LONG		long
#define BN_BITS		64
#define BN_BYTES	4
#define BN_BITS2	32
#define BN_BITS4	16
#define BN_MASK2	(0xffffffffL)
#define BN_MASK2l	(0xffff)
#define BN_MASK2h1	(0xffff8000L)
#define BN_MASK2h	(0xffff0000L)
#define BN_CBIT		((unsigned long long)0x100000000LL)
#define BN_TBIT		(0x80000000L)
#define BN_NOT_MASK2	((unsigned long long)0xffffffff00000000LL)
#endif

#ifdef SIXTEEN_BIT
#define BN_ULLONG	unsigned long
#define BN_ULONG	unsigned short
#define BN_LONG		short
#define BN_BITS		32
#define BN_BYTES	2
#define BN_BITS2	16
#define BN_BITS4	8
#define BN_MASK2	(0xffff)
#define BN_MASK2l	(0xff)
#define BN_MASK2h1	(0xff80)
#define BN_MASK2h	(0xff00)
#define BN_CBIT		((unsigned long)0x10000L)
#define BN_TBIT		(0x8000)
#define BN_NOT_MASK2	((unsigned long)0xffff0000L)
#endif

#define BN_DEFAULT_BITS	1200

#ifdef BIGNUM
#undef BIGNUM
#endif

typedef struct bignum_st
	{
	BN_ULONG *d;	// Pointer to an array of 'BN_BITS2' bit chunks. 
	int top;	// Index of last used d +1. 
	// The next are internal book keeping for bn_expand. 
	int max;	// Size of the d array. 
	int neg;
	} BIGNUM;

#define BN_CTX_NUM	12
typedef struct bignum_ctx
	{
	int tos;
	BIGNUM *bn[BN_CTX_NUM];
	} BN_CTX;

#define BN_prime_checks		(5)

#define BN_num_bytes(a)	((BN_num_bits(a)+7)/8)
#define BN_is_zero(a)	(((a)->top <= 1) && ((a)->d[0] == 0))
#define BN_is_one(a)	(((a)->top == 1) && ((a)->d[0] == 1))
#define BN_is_word(a,w)	(((a)->top == 1) && ((a)->d[0] == (w)))
#define BN_one(a)	(BN_set_word((a),1))
#define BN_zero(a)	(BN_set_word((a),0))

#define bn_fix_top(a) \
	{ \
	BN_ULONG *l; \
	for (l= &((a)->d[(a)->top-1]); (a)->top > 0; (a)->top--) \
		if (*(l--)) break; \
	}

#define bn_expand(n,b) ((((b)/BN_BITS2) <= (n)->max)?(n):bn_expand2((n),(b)))


#define LBITS(a)	((a)&BN_MASK2l)
#define HBITS(a)	(((a)>>BN_BITS4)&BN_MASK2l)
#define	L2HBITS(a)	(((a)&BN_MASK2l)<<BN_BITS4)

#define sqr64(lo,ho,in) \
	{ \
	BN_ULONG l,h,m; \
 \
	h=(in); \
	l=LBITS(h); \
	h=HBITS(h); \
	m =(l)*(h); \
	l*=l; \
	h*=h; \
	h+=(m&BN_MASK2h1)>>(BN_BITS4-1); \
	m =(m&BN_MASK2l)<<(BN_BITS4+1); \
	l+=m; if ((l&BN_MASK2) < m) h++; \
	(lo)=l; \
	(ho)=h; \
	}

#define mul_add(r,a,bl,bh,c) { \
	BN_ULONG l,h; \
 \
	h= (a); \
	l=LBITS(h); \
	h=HBITS(h); \
	mul64(l,h,(bl),(bh)); \
 \
	/* non-multiply part  */ \
	l+=(c); if ((l&BN_MASK2) < (c)) h++; \
	(c)=(r); \
	l+=(c); if ((l&BN_MASK2) < (c)) h++; \
	(c)=h&BN_MASK2; \
	(r)=l&BN_MASK2; \
	}

/*************************************************************/
// No long long type
#define mul64(l,h,bl,bh) \
	{ \
	BN_ULONG m,m1,lt,ht; \
 \
	lt=l; \
	ht=h; \
	m =(bh)*(lt); \
	lt=(bl)*(lt); \
	m1=(bl)*(ht); \
	ht =(bh)*(ht); \
	m+=m1; if ((m&BN_MASK2) < m1) ht+=L2HBITS(1L); \
	ht+=HBITS(m); \
	m1=L2HBITS(m); \
	lt+=m1; if ((lt&BN_MASK2) < m1) ht++; \
	(l)=lt; \
	(h)=ht; \
	}
#define mul(r,a,bl,bh,c) { \
	BN_ULONG l,h; \
 \
	h= (a); \
	l=LBITS(h); \
	h=HBITS(h); \
	mul64(l,h,(bl),(bh)); \
 \
	/* non-multiply part */ \
	l+=(c); if ((l&BN_MASK2) < (c)) h++; \
	(c)=h&BN_MASK2; \
	r=l&BN_MASK2; \
	if (--num == 0) break; \
	}

#define bn_mul_words(r1,r2,a,b) \
	{ \
	BN_ULONG l,h,bl,bh; \
 \
	h=(a); \
	l=LBITS(h); \
	h=HBITS(h); \
	bh=(b); \
	bl=LBITS(bh); \
	bh=HBITS(bh); \
 \
	mul64(l,h,bl,bh); \
 \
	(r1)=l; \
	(r2)=h; \
	}


static int BN_sub(BIGNUM *r, BIGNUM *a, BIGNUM *b);
static BN_ULONG bn_mul_add_word(BN_ULONG *rp, BN_ULONG *ap, int num, BN_ULONG w);
static int BN_mod_exp(BIGNUM *r, BIGNUM *a, BIGNUM *p, BIGNUM *m, BN_CTX *ctx);


#define Getenv	getenv
#define Fprintf	fprintf
#define Fputc	fputc
#define Fgets	fgets
#define Fwrite	fwrite
#define Fread	fread

#ifdef sgi
#define IRIX_CC_BUG	// all version of IRIX I've tested (4.* 5.*) 
#endif


static BN_ULONG data_one=1L;
static BIGNUM const_one={&data_one,1,1,0};
BIGNUM *BN_value_one=&const_one;

int number_range( int from, int to );
/**************************************************************************/
static void RAND_bytes(unsigned char *buf,int num)
{
	int i;
	{
		static unsigned char val;
		
		for (i=0; i<num; i++){
			val=number_range(1,250);
			buf[i]=val++;
		}
		return;
	}
}
/**************************************************************************/
static int BN_num_bits(BIGNUM *a)
{
	int i;
	BN_ULONG l;
	static char bits[256]={
		0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		};

	if(!a->top){
		return 0;
	}
	l=a->d[a->top-1];
	i=(a->top-1)*BN_BITS2;
	if(l == 0){
		return 0;
//		bugf("BAD TOP VALUE\n");
//		abort();
	}

#ifdef SIXTY_FOUR_BIT
	if(l & 0xffffffff00000000){
		if(l & 0xffff000000000000){
			if(l & 0xff00000000000000){
				return(i+bits[l>>56]+56);
			}else{
				return(i+bits[l>>48]+48);
			}
		}else{
			if(l & 0x0000ff0000000000){
				return(i+bits[l>>40]+40);
			}else{
				return(i+bits[l>>32]+32);
			}
		}
	}else
#endif
	{
#if defined(THIRTY_TWO_BIT) || defined(SIXTY_FOUR_BIT)
		if(l & 0xffff0000L){
			if(l & 0xff000000L){
				return (i+bits[l>>24L]+24);
			}else{
				return (i+bits[l>>16L]+16);
			}
		}else
#endif
		{
			if(l & 0xff00L){
				return (i+bits[l>>8]+8);
			}else{
				return (i+bits[l]);
			}
		}
	}
}

/**************************************************************************/
// ignores negative 
static int BN_bn2bin(BIGNUM *a, unsigned char *to)
{
	int n,i;
	BN_ULONG l;

	n=i=BN_num_bytes(a);
	while (i-- > 0){
		l=a->d[i/BN_BYTES];
		*(to++)=(unsigned char)(l>>(8*(i%BN_BYTES)))&0xff;
	}
	return(n);
}

/**************************************************************************/
static void BN_free(BIGNUM *a)
{
	if(!a){
		return;
	}
	if(a->d){
		free(a->d);
	}
	free(a);
}
/**************************************************************************/
static BIGNUM *BN_new()
{
	BIGNUM *ret;
	BN_ULONG *p;

	ret=(BIGNUM *)malloc(sizeof(BIGNUM));
	if(ret == NULL){
		return NULL;
	}
	ret->top=0;
	ret->neg=0;
	ret->max=(BN_DEFAULT_BITS/BN_BITS2);
	p=(BN_ULONG *)malloc(sizeof(BN_ULONG)*(ret->max+1));
	if(p == NULL){
		return NULL;
	}
	ret->d=p;

	memset(p,0,(ret->max+1)*sizeof(p[0]));
	return(ret);
}

/**************************************************************************/
static void BN_clear(BIGNUM *a)
{
	memset(a->d,0,a->max*sizeof(a->d[0]));
	a->top=0;
	a->neg=0;
}

/**************************************************************************/
static void BN_clear_free(BIGNUM *a)
{
	if(!a){
		return;
	}
	if(a->d){
		memset(a->d,0,a->max*sizeof(a->d[0]));
		free(a->d);
	}
	memset(a,0,sizeof(BIGNUM));
	free(a);
}

/**************************************************************************/
static void BN_CTX_free(BN_CTX *c)
{
	int i;

	for (i=0; i<BN_CTX_NUM; i++){
		BN_clear_free(c->bn[i]);
	}
	free(c);
}

/**************************************************************************/
static BN_CTX *BN_CTX_new()
{
	BN_CTX *ret;
	BIGNUM *n;
	int i, j;

	ret=(BN_CTX *)malloc(sizeof(BN_CTX));
	if(!ret){
		return NULL;
	}

	for (i=0; i<BN_CTX_NUM; i++){
		n=BN_new();
		if(!n){
			for (j=0; j<i; j++)
				BN_free(ret->bn[j]);
			free(ret);
			return NULL;
		}
		ret->bn[i]=n;
	}

	ret->tos=0;
	return ret;
}
/**************************************************************************/
static BIGNUM *bn_expand2(BIGNUM *b, int bits)
{
	BN_ULONG *p;
	register int n;

	while (bits > b->max*BN_BITS2){
		n=((bits+BN_BITS2-1)/BN_BITS2)*2;
		p=b->d=(BN_ULONG *)realloc(b->d,sizeof(BN_ULONG)*(n+1));
		if(!p){
			return NULL;
		}
		memset(&(p[b->max]),0,((n+1)-b->max)*sizeof(BN_ULONG));
		b->max=n;
	}
	return b;
}
/**************************************************************************/
// ignores negative
static BIGNUM *BN_bin2bn(unsigned char *s, int len, BIGNUM *ret)
{
	unsigned int i,m;
	unsigned int n;
	BN_ULONG l;

	if(!ret){
		ret=BN_new();
	}
	if(!ret){
		return NULL;
	}
	l=0;
	n=len;
	if(n==0){
		ret->top=0;
		return ret;
	}
	if(bn_expand(ret,(int)(n+2)*8) == NULL){
		return NULL;
	}
	i=((n-1)/BN_BYTES)+1;
	m=((n-1)%(BN_BYTES));
	ret->top=i;
	while (n-- > 0){
		l=(l<<8)| *(s++);
		if(m-- == 0){
			ret->d[--i]=l;
			l=0;
			m=BN_BYTES-1;
		}
	}
	// need to call this due to clear byte at top if avoiding
	// having the top bit set (-ve number) 
	bn_fix_top(ret); // macro
	return(ret);
}


/**************************************************************************/
static int BN_set_word(BIGNUM *a,unsigned long w)
{
	if(bn_expand(a,(int)sizeof(unsigned long)*8) == NULL){
		return 0;
	}
	a->d[0]=w;
	a->top=(w == 0)?0:1;
	a->neg=0;
	return 1;
}

/**************************************************************************/
static BIGNUM *BN_copy(BIGNUM *a, BIGNUM *b)
{
	if(bn_expand(a,b->top*BN_BITS2) == NULL){
		return NULL;
	}
	memcpy(a->d,b->d,sizeof(b->d[0])*b->top);
	memset(&(a->d[b->top]),0,sizeof(a->d[0])*(a->max-b->top));
	a->top=b->top;
	a->neg=b->neg;
	return(a);
}
/**************************************************************************/
static int BN_is_bit_set(BIGNUM *a, int n)
{
	int i,j;

	i=n/BN_BITS2;
	j=n%BN_BITS2;
	if(a->top <= i){
		return 0;
	}
	return ((a->d[i]&(1L<<j))?1:0);
}
/**************************************************************************/
static int BN_rshift(BIGNUM *r, BIGNUM *a, int n)
{
	int i,nw,lb,rb;
	BN_ULONG *t,*f;
	BN_ULONG l;

	r->neg=a->neg;
	nw=n/BN_BITS2;
	rb=n%BN_BITS2;
	lb=BN_BITS2-rb;
	if(nw > a->top){
		BN_zero(r);
		return 1;
	}
	if(bn_expand(r,(a->top-nw+1)*BN_BITS2) == NULL){
		return 0;
	}
	f=a->d;
	t=r->d;
	if(rb == 0){
		for (i=nw; i<a->top; i++){
			t[i-nw]=f[i];
		}
	}else{
		l=f[nw];
		for (i=nw; i<a->top; i++){
			t[i-nw] =(l>>rb)&BN_MASK2;
			l=f[i+1];
			t[i-nw]|=(l<<lb)&BN_MASK2;
		}
	}
	r->top=a->top-nw;
	t[r->top]=0;
	bn_fix_top(r);
	return 1;
}

/**************************************************************************/
static int BN_lshift(BIGNUM *r, BIGNUM *a, int n)
{
	int i,nw,lb,rb;
	BN_ULONG *t,*f;
	BN_ULONG l;

	r->neg=a->neg;
	if(bn_expand(r,(a->top*BN_BITS2)+n) == NULL) return(0);
	nw=n/BN_BITS2;
	lb=n%BN_BITS2;
	rb=BN_BITS2-lb;
	f=a->d;
	t=r->d;
	t[a->top+nw]=0;
	if(lb == 0)
		for (i=a->top-1; i>=0; i--)
			t[nw+i]=f[i];
	else
		for (i=a->top-1; i>=0; i--)
			{
			l=f[i];
			t[nw+i+1]|=(l>>rb)&BN_MASK2;
			t[nw+i]=(l<<lb)&BN_MASK2;
			}
	memset(t,0,nw*sizeof(t[0]));
	r->top=a->top+nw+1;
	bn_fix_top(r);
	return(1);
}

/**************************************************************************/
static int BN_ucmp(BIGNUM *a, BIGNUM *b)
{
	int i;
	BN_ULONG t1,t2,*ap,*bp;

	i=a->top-b->top;
	if(i != 0) return(i);
	ap=a->d;
	bp=b->d;
	for(i=a->top-1; i>=0; i--){
		t1=ap[i];
		t2=bp[i];
		if(t1 > t2) return(1);
		if(t1 < t2) return(-1);
	}
	return 0;
}

/**************************************************************************/
static int BN_cmp(BIGNUM *a, BIGNUM *b)
{
	int i;
	int gt,lt;
	BN_ULONG t1,t2;

	if(a->neg != b->neg){
		if(a->neg){
			return(-1);
		}else{	return(1);}
	}
	if(a->neg == 0){ 
		gt=1; lt=-1; 
	}else{ 
		gt=-1; lt=1; 
	}

	if(a->top > b->top) return(gt);
	if(a->top < b->top) return(lt);
	for (i=a->top-1; i>=0; i--){
		t1=a->d[i];
		t2=b->d[i];
		if(t1 > t2) return(gt);
		if(t1 < t2) return(lt);
	}
	return(0);
}

/**************************************************************************/
static void bn_SUB(BIGNUM *r, BIGNUM *a, BIGNUM *b)
{
	int max,min;
	register BN_ULONG t1,t2,*ap,*bp,*rp;
	int i,carry;
#if defined(IRIX_CC_BUG) && !defined(LINT)
	int dummy;
#endif

	max=a->top;
	min=b->top;
	ap=a->d;
	bp=b->d;
	rp=r->d;

	carry=0;
	for (i=0; i<min; i++){
		t1= *(ap++);
		t2= *(bp++);
		if(carry){
			carry=(t1 <= t2);
			t1=(t1-t2-1);
		}else{
			carry=(t1 < t2);
			t1=(t1-t2);
		}
#if defined(IRIX_CC_BUG) && !defined(LINT)
		dummy=t1;
#endif
		*(rp++)=t1&BN_MASK2;
	}
	if(carry){ // subtracted 
		while (i < max){
			i++;
			t1= *(ap++);
			t2=(t1-1)&BN_MASK2;
			*(rp++)=t2;
			if(t1 > t2) break;
		}
	}
	memcpy(rp,ap,sizeof(*rp)*(max-i));

	r->top=max;
	bn_fix_top(r);
}

/**************************************************************************/
static int BN_add(BIGNUM *r, BIGNUM *a, BIGNUM *b)
{
	register int i;
	int max,min;
	BN_ULONG *ap,*bp,*rp,carry,t1,t2;
	BIGNUM *tmp;

	//  a +  b	a+b
	//  a + -b	a-b
	// -a +  b	b-a
	// -a + -b	-(a+b)
	if(a->neg ^ b->neg){
		if(a->neg){ 
			a->neg=0; i=BN_sub(r,b,a); if(a != r) a->neg=1; 
		}else{ 
			b->neg=0; i=BN_sub(r,a,b); if(b != r) b->neg=1; 
		}
		return(i);
	}
	if(a->neg){ // both are neg 
		a->neg=0; b->neg=0; i=BN_add(r,a,b);
		if(a != r) a->neg=1;
		if(b != r) b->neg=1;
		return(i);
	}
	if(a->top < b->top){ 
		tmp=a; a=b; b=tmp; 
	}
		
	max=a->top;
	min=b->top;
	if(bn_expand(r,(max+1)*BN_BITS2) == NULL) return(0);
	r->top=max;
	r->neg=0;

	ap=a->d;
	bp=b->d;
	rp=r->d;
	carry=0;
	for(i=0; i<min; i++){
		t1= *(ap++);
		t2= *(bp++);
		if(carry){
			carry=(t2 >= ((~t1)&BN_MASK2));
			t2=(t1+t2+1)&BN_MASK2;
		}else{
			t2=(t1+t2)&BN_MASK2;
			carry=(t2 < t1);
		}
		*(rp++)=t2;
	}
	if(carry){
		while (i < max){
			t1= *(ap++);
			t2=(t1+1)&BN_MASK2;
			*(rp++)=t2;
			carry=(t2 < t1);
			i++;
			if(!carry) break;
		}
		if((i >= max) && carry){
			*(rp++)=1;
			r->top++;
		}
	}
	for (; i<max; i++){
		*(rp++)= *(ap++);
	}
	memcpy(rp,ap,sizeof(*ap)*(max-i));
	return(1);
}

/**************************************************************************/
static int BN_sub(BIGNUM *r, BIGNUM *a, BIGNUM *b)
{
	int max,i;

	/*  a -  b	a-b
	 *  a - -b	a+b
	 * -a -  b	-(a+b)
	 * -a - -b	b-a
	 */
	if(a->neg){
		if(b->neg){
			a->neg=b->neg=0;
			i=BN_sub(r,b,a);
			if(a != r) a->neg=1;
			if(b != r) b->neg=1;
		}else{
			a->neg=0;
			i=BN_add(r,a,b);
			r->neg=a->neg=1;
		}
		return(i);
	}else{
		if(b->neg){
			b->neg=0;
			i=BN_add(r,a,b);
			if(r != b) b->neg=1;
			return(i);
		}
	}

	max=(a->top > b->top)?a->top:b->top;
	if(BN_cmp(a,b) < 0){
		if(bn_expand(r,max*BN_BITS2) == NULL) return(0);
		bn_SUB(r,b,a);
		r->neg=1;
	}else{
		if(bn_expand(r,max*BN_BITS2) == NULL) return(0);
		bn_SUB(r,a,b);
		r->neg=0;
	}
	return(1);
}

/**************************************************************************/
static int BN_rshift1(BIGNUM *r, BIGNUM *a)
{
	BN_ULONG *ap,*rp,t,c;
	int i;

	if(BN_is_zero(a)){
		BN_zero(r);
		return(1);
	}
	if(a!=r){
		if(bn_expand(r,a->top*BN_BITS2) == NULL){
			return 0;
		}
		r->top=a->top;
		r->neg=a->neg;
	}
	ap=a->d;
	rp=r->d;
	c=0;
	for (i=a->top-1; i>=0; i--){
		t=ap[i];
		rp[i]=((t>>1)&BN_MASK2)|c;
		c=(t&1)?BN_TBIT:0;
	}
	bn_fix_top(r);
	return 1;
}


/**************************************************************************/
static int BN_mod(BIGNUM *rem, BIGNUM *m, BIGNUM *d, BN_CTX *ctx)
{
	int i,nm,nd;
	BIGNUM *dv;

	if(BN_ucmp(m,d) < 0){
		return((BN_copy(rem,m) == NULL)?0:1);
	}

	dv=ctx->bn[ctx->tos];

	if(!BN_copy(rem,m)) {
		return 0;
	}

	nm=BN_num_bits(rem);
	nd=BN_num_bits(d);
	if(!BN_lshift(dv,d,nm-nd)){
		return 0;
	}
	for(i=nm-nd; i>=0; i--)
	{
		if(BN_cmp(rem,dv) >= 0){
			if(!BN_sub(rem,rem,dv)){
				return 0;
			}
		}
		if(!BN_rshift1(dv,dv)){
			return 0;
		}
	}
	return 1;
}

/**************************************************************************/
static int BN_lshift1(BIGNUM *r, BIGNUM *a)
{
	register BN_ULONG *ap,*rp,t,c;
	int i;

	if (r != a){
		r->neg=a->neg;
		if (bn_expand(r,(a->top+1)*BN_BITS2) == NULL) return(0);
		r->top=a->top;
	}else{
		if (bn_expand(r,(a->top+1)*BN_BITS2) == NULL) return(0);
	}
	ap=a->d;
	rp=r->d;
	c=0;
	for (i=0; i<a->top; i++){
		t= *(ap++);
		*(rp++)=((t<<1)|c)&BN_MASK2;
		c=(t & BN_TBIT)?1:0;
	}
	if(c){
		*rp=1;
		r->top++;
	}
	return(1);
}

/**************************************************************************/
static int BN_div(BIGNUM *dv, BIGNUM *rem, BIGNUM *m, BIGNUM *d, BN_CTX *ctx)
{
	int i,nm,nd;
	BIGNUM *D;

	if(BN_cmp(m,d) < 0){
		if(rem != NULL){ 
			if(BN_copy(rem,m) == NULL){
				return(0); 
			}
		}
		if(dv != NULL){
			BN_zero(dv);
		}
		return(1);
	}

	D=ctx->bn[ctx->tos];
	if(dv == NULL) dv=ctx->bn[ctx->tos+1];
	if(rem == NULL) rem=ctx->bn[ctx->tos+2];

	nd=BN_num_bits(d);
	nm=BN_num_bits(m);
	if(BN_copy(D,d) == NULL) return(0);
	if(BN_copy(rem,m) == NULL) return(0);

	// The next 2 are needed so we can do a dv->d[0]|=1 later
	// since BN_lshift1 will only work once there is a value :-) 
	BN_zero(dv);
	dv->top=1;

	if(!BN_lshift(D,D,nm-nd)){
		return(0);
	}
	for (i=nm-nd; i>=0; i--){
		if(!BN_lshift1(dv,dv)) return(0);
		if(BN_cmp(rem,D) >= 0){
			dv->d[0]|=1;
			if(!BN_sub(rem,rem,D)){
				return(0);
			}
		}
		if(!BN_rshift1(D,D)) return(0);
	}
	dv->neg=m->neg^d->neg;
	return(1);
}

/**************************************************************************/
static int BN_reciprocal(BIGNUM *r, BIGNUM *m, BN_CTX *ctx)
{
	int nm,ret=-1;
	BIGNUM *t;

	t=ctx->bn[ctx->tos++];

	nm=BN_num_bits(m);
	if(!BN_lshift(t,BN_value_one,nm*2)) goto err;

	if(!BN_div(r,NULL,t,m,ctx)) goto err;
	ret=(nm+1);
err:
	ctx->tos--;
	return(ret);
}

/**************************************************************************/
static void bn_sqr_words(BN_ULONG *r, BN_ULONG *a, int n)
{
	for (;;){
		sqr64(r[0],r[1],a[0]);
		if (--n == 0) break;

		sqr64(r[2],r[3],a[1]);
		if (--n == 0) break;

		sqr64(r[4],r[5],a[2]);
		if (--n == 0) break;

		sqr64(r[6],r[7],a[3]);
		if (--n == 0) break;

		a+=4;
		r+=8;
	}
}

/**************************************************************************/
// r must not be a 
static int BN_sqr(BIGNUM *r, BIGNUM *a, BN_CTX *ctx)
{
	int i,j,max,al;
	BIGNUM *tmp;
	BN_ULONG *ap,*rp;

	tmp=ctx->bn[ctx->tos];

	al=a->top;
	if (al == 0){
		r->top=0;
		return(1);
	}

	max=(al*2+1);
	BN_clear(r);
	BN_clear(tmp);
	if (bn_expand(r,(max+1)*BN_BITS2) == NULL) return(0);
	if (bn_expand(tmp,(max+1)*BN_BITS2) == NULL) return(0);
	r->top=max;
	r->neg=a->neg;
	ap=a->d;
	rp=r->d;

	rp++;
	j=al;
	for (i=1; i<al; i++){
		BN_ULONG r;

		j--;
		r= *(ap++);
		rp[j]+=bn_mul_add_word(rp,ap,j,r);
		rp+=2;
	}
	bn_fix_top(r);
	if (!BN_lshift1(r,r)) return(0);

	bn_sqr_words(tmp->d,a->d,a->top);
	tmp->top=al*2+1;
	bn_fix_top(tmp);
	if (!BN_add(r,r,tmp)) return(0);
	return(1);
}

/**************************************************************************/
// r must be different to a and b 
static int BN_mul(BIGNUM *r, BIGNUM *a, BIGNUM *b)
{
	int i;
	int max,al,bl;
	BN_ULONG *ap,*bp,*rp;

	al=a->top;
	bl=b->top;
	if ((al == 0) || (bl == 0)){
		r->top=0;
		return(1);
	}

	max=(al+bl+1);
	BN_clear(r);
	if (bn_expand(r,(max+1)*BN_BITS2) == NULL) return(0);
	r->top=max;
	r->neg=a->neg^b->neg;
	ap=a->d;
	bp=b->d;
	rp=r->d;
	for (i=0; i<bl; i++){
		rp[al]=bn_mul_add_word(rp,ap,al,*(bp++));
		rp++;
	}
	bn_fix_top(r);
	return(1);
}

/**************************************************************************/
static BN_ULONG bn_mul_add_word(BN_ULONG *rp, BN_ULONG *ap, int num, BN_ULONG w)
{
	BN_ULONG c=0;
	BN_ULONG bl,bh;

	bl=LBITS(w);
	bh=HBITS(w);

	for (;;){
		mul_add(rp[0],ap[0],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[1],ap[1],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[2],ap[2],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[3],ap[3],bl,bh,c);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
	}
	return(c);
} 

/**************************************************************************/
static int mg_crypt_text(int flen, unsigned char *from, unsigned char *to, 
						BIGNUM *e, BIGNUM *n)
{
	BIGNUM *f=NULL,*ret=NULL;
	int i,j,k,num=0,r=-1;
	unsigned char *p;
	unsigned char *buf=NULL;
	BN_CTX *ctx;

	ctx=BN_CTX_new();
	if(!ctx){
		goto err;
	}

	num=BN_num_bytes(n); // macro
	if(flen > (num-11)){
		goto err;
	}
	
	buf=(unsigned char *)malloc(num);
	if(!buf){
		goto err;
	}
	p=(unsigned char *)buf;

	*(p++)=0;
	*(p++)=2;

	// pad out with non-zero random data 
	j=num-3-flen;
	RAND_bytes(p,j);
	for(i=0; i<j; i++){
		if (*p == '\0'){
			do{
				RAND_bytes(p,1);
			} while (*p == '\0');
		}
		p++;
	}
	*(p++)='\0';
	memcpy(p,from,(unsigned int)flen);

	f=BN_new();
	ret=BN_new();
	if( !f || !ret ){
		goto err;
	}

	if(BN_bin2bn(buf,num,f) == NULL) goto err;

	if(!BN_mod_exp(ret,f,e,n,ctx)) goto err;

	// put in leading 0 bytes if the number is less than the
	// length of the modulus 
	j=BN_num_bytes(ret);
	i=BN_bn2bin(ret,&(to[num-j]));
	for (k=0; k<(num-i); k++)
		to[k]=0;

	r=num;
err:
	if (ctx != NULL) BN_CTX_free(ctx);
	if (f != NULL) BN_free(f);
	if (ret != NULL) BN_free(ret);
	if (buf != NULL) 
		{
		memset(buf,0,num);
		free(buf);
		}
	return(r);
}
/**************************************************************************/
int mg_crypt_msg(char *input, char *encoded)
{
	int len;
	BIGNUM *e=BN_bin2bn((unsigned char *)eh, eKEYSIZE, NULL);
	BIGNUM *n=BN_bin2bn((unsigned char *)nh, nKEYSIZE, NULL);
	len=mg_crypt_text(str_len(input), (unsigned char *)input, 
		(unsigned char *)encoded, e, n);
	BN_free(e);
	BN_free(n);

	return len;
}
/**************************************************************************/
static int BN_mod_mul_reciprocal(BIGNUM *r, BIGNUM *x, BIGNUM *y, BIGNUM *m, 
						  BIGNUM *i, int nb, BN_CTX *ctx)
{
	int ret=0,j;
	BIGNUM *a,*b,*c,*d;

	a=ctx->bn[ctx->tos++];
	b=ctx->bn[ctx->tos++];
	c=ctx->bn[ctx->tos++];
	d=ctx->bn[ctx->tos++];

	if(x==y){ 
		if(!BN_sqr(a,x,ctx)){
			goto err; 
		}
	}else{ 
		if(!BN_mul(a,x,y)){
			goto err; 
		}
	}
	if(!BN_rshift(d,a,nb-1)) goto err;
	
	if(!BN_mul(b,d,i)) goto err;
	if(!BN_rshift(c,b,nb-1)) goto err;
	if(!BN_mul(b,m,c)) goto err;
	if(!BN_sub(r,a,b)) goto err;
	j=0;
	while (BN_cmp(r,m) >= 0){
		if(j++ > 2){
			goto err;
		}
		if(!BN_sub(r,r,m)){
			goto err;
		}
	}

	ret=1;
err:
	ctx->tos-=4;
	return(ret);
}

/**************************************************************************/
static int BN_mod_exp(BIGNUM *r, BIGNUM *a, BIGNUM *p, BIGNUM *m, BN_CTX *ctx)
{
	int nb,i,j=0,bits,ret=0;
	BIGNUM *v,*tmp,*d,*pc[8];
	BN_CTX *c2=NULL;

	v=ctx->bn[ctx->tos];
	tmp=ctx->bn[ctx->tos+1];
	d=ctx->bn[ctx->tos+2];
	ctx->tos+=3;

	if(!BN_mod(v,a,m,ctx)){
		goto err;
	}
	bits=BN_num_bits(p);

	if(!BN_one(r)){
		goto err;
	}

	nb=BN_reciprocal(d,m,ctx);
	if(nb == -1){
		goto err;
	}
	if((c2=BN_CTX_new()) == NULL){
		goto err;
	}
	for (i=0; i<8; i++){
		pc[i]=c2->bn[i];
	}
	c2->tos+=7;
	if(!BN_copy(pc[1],v)) goto err;
	if(!BN_mod_mul_reciprocal(pc[2],pc[1],pc[1],m,d,nb,ctx)) goto err;
	if(!BN_mod_mul_reciprocal(pc[3],pc[2],pc[1],m,d,nb,ctx)) goto err;
	if(!BN_mod_mul_reciprocal(pc[4],pc[2],pc[2],m,d,nb,ctx)) goto err;
	if(!BN_mod_mul_reciprocal(pc[5],pc[4],pc[1],m,d,nb,ctx)) goto err;
	if(!BN_mod_mul_reciprocal(pc[6],pc[3],pc[3],m,d,nb,ctx)) goto err;
	if(!BN_mod_mul_reciprocal(pc[7],pc[6],pc[1],m,d,nb,ctx)) goto err;

	bits=((bits-1)/3)*3; // for 4 bits, start at 2 
	for (i=bits; i>=0; i-=3){
		if(!BN_mod_mul_reciprocal(r,r,r,m,d,nb,ctx)) goto err;
		if(!BN_mod_mul_reciprocal(r,r,r,m,d,nb,ctx)) goto err;
		if(!BN_mod_mul_reciprocal(r,r,r,m,d,nb,ctx)) goto err;
		j=	 BN_is_bit_set(p,i)+
			(BN_is_bit_set(p,i+1)<<1)+
			(BN_is_bit_set(p,i+2)<<2);
		if(j && !BN_mod_mul_reciprocal(r,r,pc[j],m,d,nb,ctx)){
			goto err;
		}
#ifdef BN_FIXED_TIME
		else{
			if(!BN_mod_mul_reciprocal(pc[0],r,pc[5],m,d,nb,ctx)){
				goto err;
			}
		}
#endif
	}
	ret=1;
err:
	if(c2){
		BN_CTX_free(c2);
	}
	ctx->tos-=3;
	return(ret);
}
/**************************************************************************/

