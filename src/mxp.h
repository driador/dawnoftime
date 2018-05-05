/**************************************************************************/
// mxp.h - mxp support header
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef MXP_H
#define MXP_H

// MXP mode changing system
#define MXP_OPEN_PREFIX		"\033[0z"
#define MXP_OPEN_LINE		"\033[0z"
#define MXP_SECURE_LINE		"\033[1z"
//#define MXP_SECURE_PREFIX	"\033[1z"
#define MXP_SECURE_PREFIX	""
#define MXP_RESET			"\033[3z"
#define MXP_OPEN_MODE		"\033[5z"
#define MXP_SECURE_MODE		"\033[6z"
#define MXP_LOCKED_MODE		"\033[7z"
#define MXP_CLIENT_TO_SERVER_PREFIX "\033[1z"

// MXP defines
#define MXP_AMPERSAND	'\x11'
#define MXP_BEGIN_TAG	'\x12'
#define MXP_END_TAG		'\x13'

void mxp_define_elements_to_char(char_data *ch);
char *mxp_tagify(const char *mxp_text_with_unencoded_tags);

const char *mxp_create_tag(char_data *ch, const char *tagname, const char *txt);
const char *mxp_create_tagf(char_data *ch, const char *tagname, const char *fmt, ...) __mftc_printf_2__;
const char *mxp_create_tag_core(const char *tagname, const char *txt);
const char *mxp_create_send(char_data *ch, const char *command, const char *text);
const char *mxp_create_send(char_data *ch, const char *command_and_text);
const char *mxp_create_send_prompt(char_data *ch, const char *command, const char *text);
const char *mxp_create_send_prompt(char_data *ch, const char *command_and_text);
char *mxp_convert_to_mnemonics(const char *text_with_raw_characters);
char *mxp_tag_for_object(char_data * ch, OBJ_DATA *obj);
char *mxp_tag_for_mob(char_data * ch, char_data *mob);

#endif

