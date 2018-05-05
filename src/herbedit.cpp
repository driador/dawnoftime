/**************************************************************************/
// herbedit.cpp - OLC based herb editor, Kerenos.
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "olc.h"
#include "db.h"
#include "security.h"
#include "ictime.h"

HERB_DATA *find_herb( char_data * ch, char *herb );
bool herb_create( char_data *ch, char * );
void list_herbs( char_data *ch );
HERB_DATA *herb_list;
bool resave_continents;

char *	const	month_name	[] =
{
    "Winter", "the Winter Storm", "the Frost Blight", 
	"the Return","Blight","the Dragon",
	"Light",  "the Sun", "the Heat", 
	"the Great War", "the Shadows", "the Long Shadows", 
};

/**************************************************************************/
// write the continent name
void herbdata_write_scontinent(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	HERB_DATA * h;
	h= (HERB_DATA*) data;
	if(h->continent){
		fprintf(fp, "%s %s~\n",
			gio_table[tableIndex].heading,	h->continent->name);
	}
}
/**************************************************************************/
void save_continents();
/**************************************************************************/
// read the continent name 
void herbdata_read_continent(gio_type *, int, void *data, FILE *fp)
{
	const char *continent_convert_table[]=
	{
		"none",
		"valarin",
		"endomar",
		"kerallyan",
		"rhynia",
		"ring_isle",
		"orcs",
		"elenarthya",
		"confederacy",
		"delenth",
		"markrist",
		"aarislan",
		"faerie_ring",
		"plane_air",
		"plane_water",
		"plane_earth",
		"plane_fire",
		NULL
	};
	HERB_DATA * h;
	char *pstr;

	h= (HERB_DATA*) data;

	pstr=fread_string_eol(fp);

	// automatically convert continents that are in numeric form 
	// to the text form prior to the dynamic continent systems.
	if(is_number(pstr)){
		int i=atoi(pstr);
		free_string(pstr);
		pstr=str_dup(continent_convert_table[i]);
	}

	h->continent=continent_lookup(pstr);

	if ( !h->continent){										
		logf("Automatically added continent '%s' while reading in herb '%s'.", pstr, h->name);
		h->continent=new continent_type;

		// insert the new continent at the front of the list
		h->continent->name=str_dup(ltrim_string(rtrim_string(pstr)));
		h->continent->next=continent_list;
		continent_list=h->continent;		
		resave_continents=true;
	}
	free_string(pstr);
}
/**************************************************************************/
// read the continent name 
void herbdata_read_scontinent(gio_type *, int, void *data, FILE *fp)
{
	HERB_DATA * h;
	char *pstr;

	h= (HERB_DATA*) data;

	pstr=fread_string(fp);

	h->continent=continent_lookup(pstr);

	if ( !h->continent){										
		logf("Automatically added continent '%s' while reading in herb '%s'.", pstr, h->name);
		h->continent=new continent_type;

		// insert the new continent at the front of the list
		h->continent->name=str_dup(ltrim_string(rtrim_string(pstr)));
		h->continent->next=continent_list;
		continent_list=h->continent;
		resave_continents=true;
	}
	free_string(pstr);
}

/**************************************************************************/
// use GIO to save the herb data
GIO_START(  HERB_DATA )
GIO_STRH(   name,			"Name         ")
GIO_SHINTH( sector,			"Sector       ")
GIO_SHINTH( timefield,		"Timefield    ")
GIO_SHINTH( season,			"Season       ")
GIO_CUSTOM_WRITEH(continent,"sContinent   ", herbdata_write_scontinent)
GIO_CUSTOM_READH(continent,	"sContinent   ", herbdata_read_scontinent)
GIO_CUSTOM_READH(continent,	"Continent    ", herbdata_read_continent)
GIO_SHINTH( month,			"Month        ")
GIO_SHINTH( difficulty,		"Difficulty   ")
GIO_INTH(   vnum_result,	"Vnum         ")
GIO_INTH(   area,			"Area         ")
GIO_FINISH

/**************************************************************************/
void load_herb_db( void )
{
	resave_continents=false;
	logf( "===Loading herb database from %s...", HERB_FILE );
	GIOLOAD_LIST( herb_list, HERB_DATA, HERB_FILE );
	log_string( "load_herb_db(): finished" );
	if(resave_continents){
		save_continents();
	}
}

/**************************************************************************/
void save_herb_db( void )
{
	logf( "===save_herb_db(): saving herb database to %s...", HERB_FILE );
	GIOSAVE_LIST( herb_list, HERB_DATA, HERB_FILE, true );
}

/**************************************************************************/
// do func so it can be used as a command
void do_saveherbs( char_data *ch, char * )
{
	save_herb_db();
	ch->println( "Herbs saved..." );
	logf( "do_saveherbs(): manual save of herbs..." );
}

/**************************************************************************/
//	Entry Point for editing herbs
void do_herbedit( char_data *ch, char *argument )
{
	HERB_DATA *herb;

	if ( IS_NPC( ch )){
		ch->println("Players only.");
		return;
	}

	// do security checks
	if (!HAS_SECURITY(ch, HERBEDIT_MINSECURITY))
	{
    	ch->printlnf("You must have an olc security %d or higher to use this command.",
			HERBEDIT_MINSECURITY);
		return;
	}

	if ( !IS_TRUSTED(ch, HERBEDIT_MINTRUST))
	{
		ch->printlnf("You must have a trust of %d or above to use this command.", HERBEDIT_MINTRUST);
		return;
	}

	if (   !IS_SET(TRUE_CH(ch)->pcdata->council, COUNCIL_REALM)
		&& !IS_ADMIN(ch))
	{
		ch->println( "Only realm council may use this command." );
		return;
	}

	if (IS_NULLSTR(argument))
	{
		ch->println( " Syntax: herbedit <herb>  (edits an existing herb)");
		ch->println( " Syntax: herbedit list    (shows all available herbs)" ); 
		ch->println( " Syntax: herbedit create  (creates a new, null herb)" );
		return;
	}
	
	char arg[MIL];

	argument=one_argument(argument,arg);

	if(!str_cmp(arg,"create"))
	{
		herb_create( ch, "" );
		return;
	}

	if ( !str_cmp( arg, "list" ))
	{
		list_herbs( ch );
		return;
	}

	// find an existing herb
	herb = find_herb(ch, arg);

	if( !herb )
	{
		ch->printlnf("There is no herb named '%s'", arg );
		return;
	};

    ch->desc->pEdit	= (void*)herb;
	ch->desc->editor = ED_HERB;
	ch->printlnf("Editing '%s' herb.", herb->name);
	return;
}

/**************************************************************************/
bool herb_create( char_data *ch, char * )
{
	HERB_DATA	*node;
	static HERB_DATA zero_node;

	node	   = new HERB_DATA;
	*node	   = zero_node;
	node->next = herb_list;
	herb_list = node;

	herb_list->name				= str_dup( "New" );
	herb_list->season			= -1;
	herb_list->month			= -1;
	herb_list->area				= -1;
	herb_list->continent		= NULL;

	ch->desc->pEdit		= (void *)herb_list;
	ch->desc->editor	= ED_HERB;
	ch->println( "Herb Created." );
	return false;
}

/**************************************************************************/
// find an herb
HERB_DATA *find_herb( char_data *, char *herb )
{
	HERB_DATA *h;


	for ( h = herb_list; h; h = h->next )
	{
		if( !str_prefix( herb, h->name ))
		{
			return h;
		}
	}
	return NULL;
};

/**************************************************************************/
void list_herbs( char_data *ch )
{
	HERB_DATA	*h;
	int			col = 0;

	ch->titlebar("Existing Herbs");

	for( h=herb_list; h; h= h->next )
	{
		ch->printf( " %-20s", h->name );
		if ( ++col % 4 == 0 )
			ch->print( "\r\n" );
	}
	ch->print("`x\r\n" );
	return;
}

/**************************************************************************/
bool herbedit_name( char_data *ch, char *argument )
{
	HERB_DATA	*herb;

	EDIT_HERB( ch, herb );

	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:   name [name]" );
		return false;
	}

	replace_string( herb->name, argument );

	ch->printlnf( "Herb is now known as '%s'.", herb->name );
	
    return true;
}

/**************************************************************************/
bool herbedit_season( char_data *ch, char *argument )
{
	HERB_DATA *herb;

	EDIT_HERB( ch, herb );

	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:  season [winter/spring/summer/autumn]"  );
		ch->println( "         (anything else sets the season field to Any)."  );
		return false;
	}
	
	herb->season = season_lookup( argument );

	ch->printlnf( "Season set to '%s'.",
		herb->season >= 0 ? season_table[herb->season].name : "Any" );
	return true;
}

/**************************************************************************/
bool herbedit_month( char_data *ch, char *argument )
{
	HERB_DATA	*herb;
	int			month;
	
	EDIT_HERB( ch, herb );
	
	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:  month [1-12]"  );
		ch->println( "         (0 sets the month field to Any)."  );
		return false;
	}
	
	month = atoi( argument );
	
	if ( month < 0 || month > 12 )
	{
		herbedit_month( ch, "" );
		return false;
	}
	
	herb->month = --month;
	
	ch->printlnf( "Month set to '%d'  (%s).",
		month+1, 
		herb->month >= 0 ? month_name[herb->month] : "Any" );
	return true;
}

/**************************************************************************/
bool herbedit_sector( char_data *ch, char *argument )
{
	HERB_DATA * herb;
	int value;
	
	EDIT_HERB( ch, herb );
	
	if ( !IS_NULLSTR(argument))
	{	
	    if (( value = flag_value( sector_types, argument) ) != NO_FLAG )
		{
			herb->sector  = value;
			ch->println( "Sector set." );
			return true;
		}
		ch->printlnf( "Unrecognized sector '%s'", argument );
		return false;
	}
	
	ch->println( "Syntax: sector [sector]" );
	ch->wraplnf( "Sectors available: [`=R%s`x]",
		flag_string( sectorbit_flags, -1 ));
	
	return false;
}

/**************************************************************************/
bool herbedit_continent( char_data *ch, char *argument )
{
	HERB_DATA *herb;
	continent_type *continent;
	
	EDIT_HERB( ch, herb );
	
	if(IS_NULLSTR(argument)){
		ch->printlnf("Herb continent currently set to '%s'", 
			herb->continent? herb->continent->name:"any");
		ch->println(  "Syntax:  continent any (set it so herb can be found on any continent)"  );
		ch->println(  "Syntax:  continent [name]");
		ch->titlebar("Valid Continents:");
		continents_show(ch);		
		return false;
	}

	if( !str_cmp(argument,"any") ){
		if(!herb->continent){
			ch->println("The herb continent is already set to any.");
			return false;
		}
		ch->printlnf("Herb continent changed from '%s' to any.", herb->continent->name);
		herb->continent=NULL;
		return true;
	}
	
	// lookup the continent specified
	continent = continent_lookup( argument );	
	if(!continent){
		ch->printlnf("No such continent '%s'", argument);
		herbedit_continent( ch, "" );	// redisplay syntax and continent list
		return false;
	}

	if(herb->continent == continent){
		ch->printlnf("Herb continent is already set to '%s'.", continent->name);
		return false;
	}
	
	ch->printlnf("Herb continent changed from '%s' to '%s'.", 
		herb->continent?herb->continent->name:"any",
		continent->name);

	herb->continent = continent;

	return true;
}

/**************************************************************************/
bool herbedit_timefield( char_data *ch, char *argument )
{
	HERB_DATA *herb;
	int time;
	
	EDIT_HERB( ch, herb );
	
	if ( argument[0] == '\0' )
	{
		int i, col = 0;
		ch->println( "Syntax: time [name]\r\nValid times are:" );
		for ( i = 0; i < TIME_MAX;	i++ )
		{
			ch->printf( " %-20s   %2d`c - `x%2d   ",
				timefield_table[i].name,
				timefield_table[i].lowhour,
				timefield_table[i].highhour );
			if ( ++col % 2 == 0 )
				ch->print( "\r\n" );
		}
		ch->print( "`x\r\n" );
		return false;
	}
	
	time = time_lookup( argument );
	
	if ( time < 0 ) time = 0;
	
	herb->timefield = time;
	
	ch->printlnf( "Time set to '%s'.",
		timefield_table[time].name );
	return true;
}

/**************************************************************************/
bool herbedit_difficulty( char_data *ch, char *argument )
{
	HERB_DATA *herb;
	int mod;
	
	EDIT_HERB( ch, herb );
	
	if ( argument[0] == '\0' )
	{
		int i;

		ch->print( "Syntax: difficulty [type]\r\nValid ones are: [`c" );
		for ( i = 0; i < DIFF_MAX;	i++ )
			ch->printf( " %s (%d) ", modifier_table[i].name, modifier_table[i].modifier );
		ch->print( "`x ]\r\n" );
		return false;
	}
	
	mod = difficulty_lookup( argument );
	
	if ( mod < 0 )
	{
		ch->println( "Invalid difficulty type." );
		herbedit_difficulty( ch, "" );
		return false;
	}

	herb->difficulty = mod;
	
	ch->println( "Ok." );
	return true;
}

/**************************************************************************/
bool herbedit_area( char_data *ch, char *argument )
{
	HERB_DATA *herb;
	AREA_DATA *area;
	int	num;

	EDIT_HERB( ch, herb );

	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax: area [areanumber]  use -1 for Any area." );
		return false;
	}

	num = atoi(argument);

	if ( num == -1 )
	{
		ch->println( "Area field cleared." );
		return true;
	}

	if ( !( area = get_area_data( num )))
	{
		ch->printlnf( "'%s' is not a valid area number (as per alist).", argument );
		return false;
	}

	herb->area = area->vnum;

	ch->printlnf( "Area set to '%d (%s)'.", area->vnum, area->name );
	return true;
}

/**************************************************************************/
bool herbedit_vnum( char_data *ch, char *argument )
{
	HERB_DATA *herb;
	vn_int vnum;
	
	EDIT_HERB( ch, herb );
	
	if ( argument[0] == '\0' )
	{
		ch->println( "Syntax:  vnum [number]"  );
		ch->println( "         (Indicates which VNUM this data refers to."  );
		return false;
	}
	
	vnum = atoi( argument );
	
	if ( vnum < 1 || vnum > 65535 )
	{
		ch->println( "VNUM must be greater than 0 and less than 65536" );
		return false;
	}
	
	herb->vnum_result = vnum;
	
	ch->printlnf( "VNUM set to '%d'.", herb->vnum_result );
	return true;
}

/**************************************************************************/
bool herbedit_show( char_data *ch, char * )
{
	HERB_DATA *herb;
	char buf[MIL];

    EDIT_HERB( ch, herb );


	sprintf( buf, "%d", herb->area );
  	ch->printlnf( "`=rName:    `x%-20s", herb->name );
	ch->printlnf( "`=rSector:  `x%-20s  `=rContinent: `x%-20s  `=rArea:  `x%-20s",
		sect_table[herb->sector].name,
		herb->continent ? herb->continent->name : "Any",
		herb->area >= 0      ? buf		                     : "Any" );
	ch->printlnf( "`=rMonth:   `x%-20s  `=rSeason:    `x%-20s",
		herb->month  >= 0	 ? month_name[herb->month]				 : "Any",
		herb->season >= 0	 ? season_table[herb->season].name		 : "Any" );
	ch->printf( "`=rTime:    `x%-20s",
		timefield_table[herb->timefield].name );
	if ( timefield_table[herb->timefield].highhour >= 0 )
	{
		ch->printlnf( "  `=rHours `x%-2d `=r- `x%-2d",
			timefield_table[herb->timefield].lowhour,
			timefield_table[herb->timefield].highhour );
	}
	else
	{
		ch->print( "\r\n" );
	}
	ch->printlnf( "`=rDifficulty:     `x%s",	modifier_table[herb->difficulty].name );

	OBJ_INDEX_DATA *o=get_obj_index(herb->vnum_result);
	if(o){
		ch->printlnf( "`=rResulting VNUM: `=v%d  (%s)`x", herb->vnum_result, o->short_descr);
	}else{
		ch->printlnf( "`=rResulting VNUM: `=X%d  (doesn't exist!)`x", herb->vnum_result);
	}

	return false;
}

/**************************************************************************/
/**************************************************************************/

