/**************************************************************************/
// colour.h - Dawn colour system
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef COLOUR_H
#define COLOUR_H

#ifndef COLOURCODE
#define COLOURCODE '`'
#endif

// colour code in string form
#ifndef CCSTR
#define CCSTR "`"
#endif
 #ifndef CC
#define CC "`"
#endif

enum COLOUR_TYPE {CT_NOCOLOUR, CT_ANSI, CT_IRC, CT_IRCWHITE, CT_HTML, CT_MXP, CT_BRIGHT, CT_AUTODETECT}; 

#define MAX_SAVED_COLOUR_ARRAY	(15)	// number of colours that the colour saving can store

struct colour_codes{
	unsigned char code;
	char *name;	
	bool special;
	bool dont_repeat;
	// different codings
	char noColour[5];
	char ansi[8];
	char irc[7]; 
	char ircWhite[7];  // white background
	char html[61];
	char mxp[61];
	char bright[61];

	char original_ansi_character;
	
};

struct COLOUR_TEMPLATE_TYPE;
struct COLOUR_MEMORY_TYPE // one per descriptor
{
	unsigned char current;
	unsigned char saved[MAX_SAVED_COLOUR_ARRAY];
	unsigned char saved_index;
	unsigned char saved_before_helplink; // the colour which after a helplink, the system reverts to
	char *custom_colour;
	COLOUR_TEMPLATE_TYPE *colour_template;
	bool in_help_link;
	bool flashing_disabled;
};

//prototypes
void initColour();
void colour_generate_html_table();
int convertColour(const char *src, char *dest, COLOUR_TYPE cType, bool partial_html);
class connection_data;
//char *process_colour(const char *raw_text, COLOUR_TYPE cType, COLOUR_MEMORY_TYPE *cm, connection_data *d);
char *process_colour(const char *raw_text, connection_data *d);
char *strip_colour(const char *coloured_text);

struct colour_table_type{char code[10];};
extern const struct colour_table_type colour_table[];

#endif
