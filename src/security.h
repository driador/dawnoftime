/**************************************************************************/
// security.h - Has some security and trust levels for doing things, Kal
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef SECURITY_H
#define SECURITY_H

// raceedit general
#define RACEEDIT_MINSECURITY			8
#define RACEEDIT_MINTRUST			COUNCIL-1
// raceedit specifics
#define	RACEEDIT_CREATE_MINTRUST		COUNCIL-1

// oedit specifics
#define OEDIT_NOLONG_MINSECURITY		7

// medit specifics
#define MEDIT_XPMOD_MINSECURITY			8

// classedit
#define CLASSEDIT_MINSECURITY			9
#define CLASSEDIT_MINTRUST				COUNCIL
#define CLASSEDIT_CREATION_MINTRUST		ADMIN // creation selectable

// skill groupedit
#define GROUPEDIT_MINSECURITY			9
#define GROUPEDIT_MINTRUST				COUNCIL

// sedit 
#define SEDIT_MINSECURITY				8
#define SEDIT_MINTRUST					COUNCIL-1

// comedit
#define COMEDIT_MINSECURITY				9
#define COMEDIT_MINTRUST				MAX_LEVEL-1

// mpedit
#define MPEDIT_MINSECURITY				7

// gameedit
#define GAMEEDIT_MINSECURITY			9
#define GAMEEDIT_MINTRUST				MAX_LEVEL

// socedit
#define SOCEDIT_MINSECURITY				7
#define SOCEDIT_MINTRUST				COUNCIL-2

// herbedit
#define HERBEDIT_MINSECURITY			7
#define HERBEDIT_MINTRUST				COUNCIL-2

// langedit general
#define LANGEDIT_MINSECURITY			8
#define LANGEDIT_MINTRUST			COUNCIL-1

/**************************************************************************/
#endif // SECURITY_H

