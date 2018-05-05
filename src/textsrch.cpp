/**************************************************************************/
// textsrch.h - Text search system.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "olc_ex.h"
#include "socials.h"
#include "help.h"

enum TEXTSEARCH_TYPE{
	TEXTSEARCH_MUDPROG, 
	TEXTSEARCH_ROOMNAME, 
	TEXTSEARCH_ROOMDESCRIPTION,
	TEXTSEARCH_ROOMEXTDESCRIPT,
	TEXTSEARCH_OBJDESCRIPTION,
	TEXTSEARCH_OBJEXTDESCRIPT,
	TEXTSEARCH_MOBDESCRIPTION,
	TEXTSEARCH_HELPS,
	TEXTSEARCH_SOCIALS
};


const struct flag_type textsearch_types[] =
{
    {  "mudprog",			TEXTSEARCH_MUDPROG,			true },
	{  "roomname",			TEXTSEARCH_ROOMNAME,		true },
	{  "roomdescription",	TEXTSEARCH_ROOMDESCRIPTION,	true },
	{  "roomextdescript",	TEXTSEARCH_ROOMEXTDESCRIPT,	true },
	{  "objdescription",	TEXTSEARCH_OBJDESCRIPTION,	true },	
	{  "objextdescript",	TEXTSEARCH_OBJEXTDESCRIPT,	true },	
	{  "mobdescription",	TEXTSEARCH_MOBDESCRIPTION,	true },		
	{  "helps",				TEXTSEARCH_HELPS,			true },	
	{  "socials",			TEXTSEARCH_SOCIALS,			true },	
	{  NULL,           0,          0   }
};

/**************************************************************************/
void do_textsearch(char_data *ch, char* argument)
{
	int maxhits=200;
	int i;
	if(IS_NULLSTR(argument)){
		ch->println("Syntax: textsearch <what> <text>");
		ch->println("Where <what> is one of the following:");
		show_olc_flags_types(ch, textsearch_types);
		return;
	};

	char arg[MIL];
	argument=one_argument(argument, arg);

	TEXTSEARCH_TYPE stype= (TEXTSEARCH_TYPE)flag_value(textsearch_types, arg);

	if(stype==NO_FLAG){
		ch->printlnf("Unrecognised flag '%s'.", arg);
		do_textsearch(ch, "");
		return;
	}

	int hits=0;
	int count;
	EXTRA_DESCR_DATA *ed;
	switch(stype){
		case TEXTSEARCH_MUDPROG:
			{
				for(MUDPROG_CODE *pMP=mudprog_list; pMP; pMP=pMP->next){
					if(!str_infix(argument, pMP->code)){
						hits++;
						if(hits<=maxhits){
							ch->printlnf("%3d) <%5d> '%s'", hits, pMP->vnum, pMP->title);
						}
					}
				}
			}
			break;

		case TEXTSEARCH_ROOMNAME:
            {
				for(i=0; i<MAX_KEY_HASH; i++){
					for(ROOM_INDEX_DATA *r= room_index_hash[i]; r; r= r->next )
					{
						if(!str_infix(argument, r->name)){
							hits++;
							if(hits<=maxhits){
								ch->printlnf("%3d) <%5d> '%s'", hits, r->vnum, r->name);
							}
						}
					}
				}
            }
            break;

		case TEXTSEARCH_ROOMDESCRIPTION:
			{
				for(i=0; i<MAX_KEY_HASH; i++){
					for(ROOM_INDEX_DATA *r= room_index_hash[i]; r; r= r->next )
					{
						if(!str_infix(argument, r->description)){
							hits++;
							if(hits<=maxhits){
								ch->printlnf("%3d) <%5d> '%s'", hits, r->vnum, r->name);
							}
						}
					}
				}
			}
			break;

		case TEXTSEARCH_ROOMEXTDESCRIPT:
			{
				for(i=0; i<MAX_KEY_HASH; i++){
					for(ROOM_INDEX_DATA *r= room_index_hash[i]; r; r= r->next )
					{
						// do the extended descriptions
						count=0;
						for(ed=r->extra_descr; ed; ed=ed->next){
							if(!str_infix(argument, ed->description)
								|| !str_infix(argument, ed->keyword)){
								count++;
								hits++;
								if(hits<=maxhits){
									ch->printlnf("%3d) <%5d> '%s', extended %d - '%s'.", hits, 
										r->vnum, r->name, count, ed->keyword);
								}
							}
						}

						// do the exits
						for (int door = 0; door < MAX_DIR; door++ )
						{
							EXIT_DATA *pexit;
							
							if ( ( pexit = r->exit[door] ) ){
								if( (!IS_NULLSTR(pexit->keyword) 
									&& !str_infix(argument, pexit->keyword))
									|| (!IS_NULLSTR(pexit->description)
									&& !str_infix(argument, pexit->description)))
								{
									hits++;
									if(hits<=maxhits){
										ch->printlnf("%3d) <%5d> '%s', %s exit '%s'.", hits, 
											r->vnum, r->name, flag_string(direction_types, door), 
											IS_NULLSTR(pexit->keyword)?"no keywords.":pexit->keyword);
									}
								}
							}
						}

					}
				}
			}
			break;


		case TEXTSEARCH_OBJDESCRIPTION:
			{
				for(i=0; i<MAX_KEY_HASH; i++){
					for(OBJ_INDEX_DATA *o= obj_index_hash[i]; o; o= o->next )
					{
						if(!str_infix(argument, o->description)){
							hits++;
							if(hits<=maxhits){
								ch->printlnf("%3d) <%5d> '%s'", hits, o->vnum, o->name);
							}
						}
					}
				}
			}
			break;

		case TEXTSEARCH_OBJEXTDESCRIPT:
			{
				for(i=0; i<MAX_KEY_HASH; i++){
					for(OBJ_INDEX_DATA *o= obj_index_hash[i]; o; o= o->next )
					{
						// do the extended descriptions
						count=0;
						for(ed=o->extra_descr; ed; ed=ed->next){
							if(!str_infix(argument, ed->description)
								|| !str_infix(argument, ed->keyword)){
								count++;
								hits++;
								if(hits<=maxhits){
									ch->printlnf("%3d) <%5d> '%s', extended %d - '%s'.", hits, 
										o->vnum, o->name, count, ed->keyword);
								}
							}
						}
					}
				}
			}
			break;

		case TEXTSEARCH_MOBDESCRIPTION:
			{
				for(i=0; i<MAX_KEY_HASH; i++){
					for(MOB_INDEX_DATA *m= mob_index_hash[i]; m; m= m->next )
					{
						if(!str_infix(argument, m->description)){
							hits++;
							if(hits<=maxhits){
								ch->printlnf("%3d) <%5d> '%s'", hits, m->vnum, m->short_descr);
							}
						}
					}
				}
			}
			break;

		case TEXTSEARCH_HELPS:
			{
				for ( help_data *pHelp = help_first; pHelp; pHelp = pHelp->next )
				{
					if ( pHelp->level > get_trust( ch ) )
						continue;

					if(!str_infix(argument, pHelp->text) 
						|| !str_infix(argument, pHelp->keyword)
						|| !str_infix(argument, pHelp->title)
						|| !str_infix(argument, pHelp->see_also)
						|| !str_infix(argument, pHelp->immsee_also)
						|| !str_infix(argument, pHelp->spell_name)
						|| !str_infix(argument, pHelp->command_reference)
						|| !str_infix(argument, pHelp->continues)
						|| !str_infix(argument, pHelp->parent_help)
						){
						hits++;
						if(hits<=maxhits){
							ch->printlnf("%3d) `=_'%s' `x (length =%d bytes) <%s> [%d]%s", 
								hits, pHelp->keyword, (int) str_len(pHelp->text), 
								pHelp->helpfile->file_name, pHelp->level,
								IS_SET(pHelp->flags,HELP_REMOVEHELP)?" `RFLAGGED FOR REMOVAL`x":"");
							
						}
					}
				}
			}
			break;


		case TEXTSEARCH_SOCIALS:
			{
				for(social_type *soc=social_list; soc; soc=soc->next){
					if(IS_SET(soc->social_flags, SOC_IMM_ONLY) && !IS_IMMORTAL(ch)){
						continue;
					}

					for(i=0; i<SOCIAL_ATMAX; i++){
						if(!str_infix(argument, soc->acts[i])){
							hits++;
							if(hits<=maxhits){
								ch->printlnf("%3d) %s [act%d]", hits, soc->name, i);
							}
						}
					}
				}
				if(hits>0){
					ch->println("Use socshow <social> to display a particular social.");
				}
			}
			break;

		default:
			ch->printlnf("do_textsearch(): bug - unprogrammed search type %d - please note the admin!", stype);
			break;

	}
	if(hits==0){
		ch->printlnf("No matching '%s' found for search type '%s'.", 
			argument, flag_string(textsearch_types, stype));
	}else{
		ch->printlnf("%d match%s searching for '%s' (search type '%s').", hits, hits==1?"":"es",
			argument, flag_string(textsearch_types, stype));
	}

}
/**************************************************************************/

