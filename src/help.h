/**************************************************************************/
// help.h - help.cpp prototypes
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef HELP_H
#define HELP_H

struct help_data;
struct helpfile_data;
/**************************************************************************/
// prototypes
help_data *help_get_by_keyword(char * keyword, char_data *ch, bool exact_match);
char *help_find_keyword( char *argument, char *arg_first, char_data *looker);
bool help_valid_for_char(help_data *pHelp, char_data *ch);
void help_display_to_char(help_data *pHelp, char_data *ch);
char *help_generate_help_entry_for_char(help_data *pHelp, char_data *ch);

void get_widest_line_stats(const char *text, bool ignore_colour_codes, 
						   sh_int *line_number, sh_int *line_width);

// helpfiles system
void save_helpfile( helpfile_data *pHelpfile );
void save_helpentries( FILE *fp, helpfile_data *pHelpfile );
help_data *help_allocate_new_entry( void );
helpfile_data *helpfile_allocate_new_entry( void );
helpfile_data *helpfile_get_by_filename( char * argument);

// loading and saving
void save_helpfile_NAFF( helpfile_data *pHelpfile );
void load_helpfile_NAFF( FILE *fp);
void help_init_quicklookup_table();

/**************************************************************************/
// structures
struct  help_data
{
	help_data		*next;
	help_data		*prev;
	helpfile_data	*helpfile;

	sh_int			level;
	int				category;
	sh_int			assigned_editor;

	char			*command_reference;
	char			*title;
	char			*parent_help;
	char			*see_also;
	char			*immsee_also;
	char			*spell_name; // if true, it is a spell
	char			*continues;
    char			*keyword;
	char			*text;
	char			*undo_wraptext;
	char			*undo_edittext;
	char			*last_editor;
	time_t			last_editdate;
	long			flags;
	sh_int			widest_line;
	sh_int			widest_line_width;
};
/**************************************************************************/
struct  helpfile_data
{
	helpfile_data * next;
    char * file_name;
    char * title;
	char * editors;
	char   colourcode;
	sh_int vnum;
	sh_int security;
	sh_int version;
	sh_int entries;
	int flags; // changed etc
};
/**************************************************************************/
// semi-globals
extern help_data *help_first;
extern help_data *help_last;
extern helpfile_data *helpfile_first;
extern helpfile_data *helpfile_last;
/**************************************************************************/

#endif // HELP_H

