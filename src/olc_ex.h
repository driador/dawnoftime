/**************************************************************************/
// olc_ex.h - Extended commands for olc header
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef OLC_EX_H
#define OLC_EX_H

// Protoypes
bool olc_generic_skill_assignment_to_int(char_data *ch, int *field, const char *argument, 
	bool spell_only, bool extra_lf, 
	const char *command_name, const char *description_of_field_sprintf, ...);

bool olc_generic_flag_toggle(char_data *ch, char *argument,
	const char *command_name, const char *descript, 
	const struct flag_type *flag_table, long *value);

bool olc_generic_flag_toggle(char_data *ch, char *argument,
	const char *command_name, const char *descript, 
	const struct flag_type *flag_table, int *value);

char * safe_skill_name(int sn);
int spell_lookup( const char *name );
int spell_exact_lookup( const char *name );
char *flagtable_names(const struct flag_type *flag_table);

void show_olc_flags_types(char_data *ch, const struct flag_type *flag_table);
void show_olc_flags_types_value(char_data *ch, const struct flag_type *flag_table, 
											const char *command_name, long value);

int mxp_display_olc_flags(char_data *ch, const struct flag_type *flag_table, long value, char *command, char *heading);
int mxp_display_olc_flags_ex(char_data *ch, const struct flag_type *flag_table, long value, char *command, char *heading, int max_width,int first_indent, int indent); // defaults 77, 16, 5

bool olcex_showflags(char_data *ch, char *argument);
bool olcex_showafter(char_data *ch, char *argument);
bool olcex_showflagsafter(char_data *ch, char *argument);
bool olcex_showcommandafter(char_data *ch, char *argument);
char *olcex_get_editor_name( int edit_mode);

bool olcex_tab( char_data *ch, char *argument);

void continents_show( char_data *ch );

const char *retrieve_line( const char *entire_text, char *line);
char *indent_mobprog_code(char_data *ch, char *code, int custom_indent_amount);
#endif // OLC_EX_H
