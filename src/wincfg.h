/**************************************************************************/
// wincfg.h - used to take the place of a configure script on windows
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef WINCFG_H
#define WINCFG_H

/*
 * MCCP support - to enable uncomment the following define.
 */
//#define WIN32_MCCP_ENABLED
/*
 * NOTES ON WIN32_MCCP_ENABLED: 
 *       MCCP (Mud Client Compression Protocol) requires zlib.
 *       (zlib is the library which handles all the data compression).
 *
 *       If WIN32_MCCP_ENABLED is defined below, connect.h will 
 *       automatically (try to) include zlib.h and (try to) link
 *       against zlib_debug.lib for a debug build and zlib_release.lib
 *       for a release build.
 *
 *       Using Visual C++ 6.0, debug and release versions of zlib.lib 
 *       can be created by downloading the zlib source code from 
 *       www.zlib.org, then compiling them with the following commands
 *       in a command prompt:  (in the zlib directory)
 *             nmake -f win32/Makefile.msc clean
 *             nmake -f win32/Makefile.msc LOC=/MLd
 *             copy zlib.lib zlib_debug.lib
 *             nmake -f win32/Makefile.msc clean
 *             nmake -f win32/Makefile.msc LOC=/ML
 *             copy zlib.lib zlib_release.lib
 *       (Do this in the directory you uncompressed the zlib source).
 *       It is safe to ignore the "overriding '/MD' with '/ML'" warnings.
 *       This command assumes that the compiler environment variables 
 *       and paths are established in your command prompt environment.
 *       If you type "set" in the command prompt, it will display your
 *       environment.  In the list of environment variables there should 
 *       be LIB and INCLUDE entries + the location of the nmake binary 
 *       in the path... if it is not, you can run VCVARS32.BAT from within
 *       the command prompt... typically it is at 
 *       \Visual Studio\VC98\Bin\VCVARS32.BAT, if not use find files.
 *
 *       Once you have successfully compiled the library, probably the 
 *       the easiest way to explain getting things going is as follows:
 *       - Visual C++ v6.0: click tools->options->directories tab
 *         Include files (from right drop down), add the path to where
 *         the zlib source code is.  In addition add the same path for
 *         the Libraries from the drop down.
 *       - Visual C++ .NET: click tools->options->projects folder 
 *         on the left, then do the equivalent changes as above.
 *      
 *       The above steps do NOT relate to BSD/Linux based environments 
 *       nor a Cygwin environment, since these all use the configure 
 *       script (config/configure), to automatically detect the 
 *       presence of the zlib library.
 */
#ifdef _MSC_VER // visual c++ only
// we don't want to be warned about using sprintf() even though it is deprecated
// or unlink() instead of _unlink() etc
	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE 1
	#endif
	#ifndef _CRT_NONSTDC_NO_DEPRECATE 
		#define _CRT_NONSTDC_NO_DEPRECATE 1
	#endif

	#ifndef _USE_32BIT_TIME_T
		// dawn is not 64bit TIME_T clean at this stage
		#define _USE_32BIT_TIME_T
	#endif
#endif 

/*
 * Winsock v2 support - to enable uncomment the following define:
 */
#define WIN32_USE_WINSOCK2

/* 
 * IPV6 support - to enable uncomment the following define.
 * note: this will automatically turn on WIN32_USE_WINSOCK2 in include.h
 *       and requires the IPv6 headers within the development environment.
 */
//#define WIN32_IPV6

#endif //WINCFG_H

