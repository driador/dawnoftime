/**************************************************************************/
// version.h - used to defines which version of the game this is.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef VERSION_H
#define VERSION_H

#define DAWN_RELEASE_VERSION "1.69s_beta5"
#define DAWN_RELEASE_DATE	"23May2009"

//#define DAWN_STATIC_BETA_RELEASE
//#define DAWN_STATIC_BETA_RELEASE_EXPIRY_DATE (1243033200 + 50*24*60*60 -1)
// with DAWN_STATIC_BETA_RELEASE defined,
// the code will expire 50 days after the above date
// currently set to Sun 12 July 2009
// date -d "`date +%m/%d/%Y`" +%s
// date -d "`date +%m/%d/%Y` + 50 days"

#ifndef DAWN_STATIC_BETA_RELEASE
#define MFCODE 1
// when MFCODE is set, the code compiles in code which only belongs
// to MF not the dawn codebase
#endif // DAWN_STATIC_BETA_RELEASE

#endif // VERSION_H
