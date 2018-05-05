/**************************************************************************/
// include.h - global include file, included in nearly all cpp modules.
/***************************************************************************
 *        --- The Dawn of Time v1.69s (c)1997-2010 Kalahn ---              *
 *
 *                                                __----''''''''''''------___
 *                                  .  .   --//====......          __--- --
 *                  -.            \_|//     |||\\  ------::::... /-        
 *               ___-==_       _--o-  \/    |||  \\            _/---       *
 *       __------.=='||\=_    -_--'/_-'|-   |\\   \\        _/'            *
 *   _-'`     .='    |  \\-_    '--7  /-   /  ||    \      /               *
   .'       .'       |   \\ -_    /  /-   /   ||      \   /                *
  /  ____  /         |     \\ '-_/  /|- _/   .||       \ /                 *
 |''    ''|--''''--_ \     '==-/   | \'--===''        .\                   *
           '         '-|      /|    |-'\''       __--''                    *
 *                     |-''-_/ |    |   '\_   _-'            /\            *
 *                          /  \     \__   \/'               \__           *
 *                       _--' _/ | .-''____--'-/                  ''==.    * 
 *                     ((->/'   '.|||' -_|    ''-/ ,              . _||    * 
 *                                -_     '\      ''---l__i__i__i--''_/     *
 *                               _-'-__   ')  \--______________--''        *
 *                               //.-'''-'_--'- |-------''''''''           *
 *                                     //.-'''--\                          *
 *  Webpage: http://www.dawnoftime.org/                                    *
 ***************************************************************************
 *  FILE: include.h  - included by all cpp files                           *
 ***************************************************************************/
#ifndef INCLUDE_H
#define INCLUDE_H

#ifdef WIN32
	#include "wincfg.h"		// windows configure file
#endif

#if defined(__APPLE__) && defined(__MACH__) && !defined(unix)
	// mac os-x
	// __APPLE__ by itself, is a mac that isn't based on an Mach kernel.
	#define unix
#endif

#include <assert.h>

#ifndef DEBUG
	#define DEBUG
#endif

#ifdef WIN32

	#include <time.h>
	#include <sys/timeb.h>
#else
	#include "config.h"
	#if defined(__unix__) && !defined(unix) // we expect to see unix
		#define unix __unix__
	#endif
	#ifdef TIME_WITH_SYS_TIME
		#include <sys/time.h>
		#include <time.h>
	#else
		#ifdef HAVE_SYS_TIME_H
			#include <sys/time.h>
		#else
			#include <time.h>
		#endif
	#endif
	#include <sys/resource.h>
	#include <unistd.h>
	#include <sys/socket.h>
#endif


#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "version.h"
#include "bitflags.h"
#include "debug.h"
#include "colour.h"
#include "macros.h"
#include "params.h"
#include "language.h"
#include "netio.h"
#include "structs.h"


#include "races.h"
#include "prototyp.h"
#include "mxp.h"
#include "tables.h"
#include "lookup.h"
#include "gameset.h"
#include "gio.h"
#include "recycle.h"

#include "dawn.h"

#ifndef _GLOBAL_C
#include "global.h"
#endif

#endif //INCLUDE_H


