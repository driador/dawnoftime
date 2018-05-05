/**************************************************************************/
// socials.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
// no argument specified
#define SOCIAL_ATNOTARGET_MSG2SELF			0
#define SOCIAL_ATNOTARGET_MSG2OTHERS		1
// self target
#define SOCIAL_ATSELF_MSG2SELF				2
#define SOCIAL_ATSELF_MSG2OTHERS			3
// other target
#define SOCIAL_ATTARGET_MSG2SELF			4
#define SOCIAL_ATTARGET_MSG2TARGET			5
#define SOCIAL_ATTARGET_MSG2OTHERS			6
#define SOCIAL_ATTARGET_MOBTARGETRESPONSE	7	// The social a mob will respond with
#define SOCIAL_ATMAX						8

// SOC FLAGS
#define SOC_IN_OOC		(A)
#define SOC_IN_IC		(B)
#define SOC_IMM_ONLY	(C)

/**************************************************************************/
class social_type{
public:
	char *	name;
	int		social_flags;
	char *	acts[SOCIAL_ATMAX];
	social_type * next;
	int		position_flags; // the positions you can use it in

	// member functions
	social_type(); // default constructor
	~social_type(); // default destructor

	void execute_social(char_data *ch, char *argument, bool global);

private:
	bool process_social_execution(char_data *ch, char *argument, bool global);
};
/**************************************************************************/
extern const struct flag_type position_flags[];
extern const struct flag_type social_flags[];
extern social_type *social_list;

#define safe_str_dup(str) (str?str_dup(str):str_dup(""))
void save_socials();

social_type *find_social(char_data * ch, char *social);
/**************************************************************************/
