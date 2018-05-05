/**************************************************************************/
// map.cpp - Kal's map code
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h" // dawn standard includes
#include "ictime.h"

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 11
#define MAX_Y 11
#define MAX_WIDTH  200
#define MAX_HEIGHT 200

#define nSET_BIT(var, bitnum)		(var|=1<<bitnum);
#define nIS_SET(var, bitnum)		(var& (1<<bitnum));
#define nREMOVE_BIT(var, bitnum)	(var= var & ~(1<<bitnum));

unsigned char grid[15][15];
int count_table[15][15];
int vnum_table[15][15];
int counter;

#define SECT_HERE		SECT_MAX+1
#define SECT_MAPSCALE	SECT_MAX+2
#define SECT_UNMAPPED	SECT_MAX+3
#define SECT_ROOM_DARK	SECT_MAX+4
#define SECT_MAPLEVEL   SECT_MAX+5
#define SECT_UP			SECT_MAX+6
#define SECT_DOWN		SECT_MAX+7
#define SECT_UP_DOWN	SECT_MAX+8

struct map_type{
	bool valid;
	vn_int vnum;
	short sector_type;
	int closed;  // if the exit is closed by a wall or a door
	int door; // if closed is true, true here = door, false = wall
	int terrain_type;
};

bool doing_areamap;
map_type full_map[MAX_HEIGHT][MAX_WIDTH];


char sector_table[SECT_MAX+10];
char sector_colour[SECT_MAX+10];
struct maptype{	int dir,x,y;};
maptype mapdirs[8]={
	{DIR_WEST, -1, 0},
	{DIR_NORTH, 0,-1},
	{DIR_EAST,  1, 0},
	{DIR_SOUTH, 0, 1},
	{DIR_NORTHEAST, 1,-1},
	{DIR_SOUTHEAST, 1, 1},
	{DIR_SOUTHWEST,-1, 1},
	{DIR_NORTHWEST,-1,-1}
};

/**************************************************************************/
void draw_grid(ROOM_INDEX_DATA * pRoom, int x, int y){
	int mdir, newx, newy, currentx,currenty;
	counter++;

	//logf("start draw_grid(): x=%d, y=%d, thiscounter=%d", x,y, thiscounter);

	if (!pRoom){
		bugf("draw_grid(): NULL pRoom, x=%d, y=%d", x,y);
		return;
	}

	if(grid[y][x]!=SECT_UNMAPPED){
		bugf("draw_grid(): non empty grid!, x=%d, y=%d, counter=%d, grid=%d",
			x,y, counter, grid[y][x]);
		return;
	}

	grid[y][x]=(char)pRoom->sector_type;

	full_map[y][x].valid=true;
	full_map[y][x].vnum=pRoom->vnum;
	full_map[y][x].sector_type=pRoom->sector_type;

	// up and down are back to front
	if (pRoom->exit[DIR_DOWN] 
		&& pRoom->exit[DIR_DOWN]->u1.to_room){
		if( IS_SET(pRoom->exit[DIR_DOWN]->exit_info, EX_CLOSED)){
			// door
			nREMOVE_BIT(full_map[y][x].closed, DIR_DOWN);
            nSET_BIT(full_map[y][x].door, DIR_DOWN); 
		}else{
			// open
			nSET_BIT(full_map[y][x].closed, DIR_DOWN);
			nREMOVE_BIT(full_map[y][x].door, DIR_DOWN); 
		}
	}else{ // blocked
		nREMOVE_BIT(full_map[y][x].closed, DIR_DOWN);
		nSET_BIT(full_map[y][x].door, DIR_DOWN); 
	}
		
	if (pRoom->exit[DIR_UP] && pRoom->exit[DIR_UP]->u1.to_room){
		if( IS_SET(pRoom->exit[DIR_UP]->exit_info, EX_CLOSED)){
			// door
			nREMOVE_BIT(full_map[y][x].closed, DIR_UP);
            nSET_BIT(full_map[y][x].door, DIR_UP); 
		}else{
			// open
			nSET_BIT(full_map[y][x].closed, DIR_UP);
            nREMOVE_BIT(full_map[y][x].door, DIR_UP); 
		}
	}else{ // blocked
		nREMOVE_BIT(full_map[y][x].closed, DIR_UP);
		nSET_BIT(full_map[y][x].door, DIR_UP); 
	}

	vnum_table[y][x]=pRoom->vnum;
	count_table[y][x]=counter;
//	logf("%5d x=%2d, y=%2d, grid=%d", counter, x, y, (int)grid[y][x]);

	// recursively draw the map
	currentx=x;
	currenty=y;
	for (mdir=0; mdir<8; mdir++){

		if (pRoom->exit[mapdirs[mdir].dir] && pRoom->exit[mapdirs[mdir].dir]->u1.to_room){
			if( IS_SET(pRoom->exit[mapdirs[mdir].dir]->exit_info, EX_CLOSED)){
				// door
				nSET_BIT(full_map[y][x].closed, mapdirs[mdir].dir);
				nSET_BIT(full_map[y][x].door, mapdirs[mdir].dir); 
			}else{
				// open
				nREMOVE_BIT(full_map[y][x].closed, mapdirs[mdir].dir);
			}
		}else{ // blocked
			nSET_BIT(full_map[y][x].closed, mapdirs[mdir].dir);
			nREMOVE_BIT(full_map[y][x].door, mapdirs[mdir].dir); 
		}

//		logf("thiscount=%d, mdir=%d", thiscounter,mdir);
		newx= currentx +mapdirs[mdir].x;
		newy= currenty +mapdirs[mdir].y;
		if (   newx>MIN_X 
			&& newx<MAX_X
			&& newy>MIN_Y
			&& newy<MAX_Y)
		{
			if (pRoom->exit[mapdirs[mdir].dir] 
				&& pRoom->exit[mapdirs[mdir].dir]->u1.to_room)
			{
				if (grid[newy][newx]==SECT_UNMAPPED){
					if (pRoom->exit[mapdirs[mdir].dir]->u1.to_room->area->mapscale
						==pRoom->area->mapscale){
					draw_grid(pRoom->exit[mapdirs[mdir].dir]->u1.to_room, 
						newx, newy); 
					}else{
						grid[newy][newx]=(char)SECT_MAPSCALE;
						vnum_table[newy][newx]=pRoom->exit[mapdirs[mdir].dir]->u1.to_room->vnum;
						count_table[newy][newx]=++counter;

						full_map[newy][newx].valid=true;
						full_map[newy][newx].vnum=pRoom->exit[mapdirs[mdir].dir]->u1.to_room->vnum;
						full_map[newy][newx].sector_type=SECT_MAPSCALE;
					}
				}
			}else{
//				nREMOVE_BIT(full_map[y][x].exits, mapdirs[mdir].dir); 
			}
//				pRoom->vnum, 
//				-mapdirs[mdir].x, -mapdirs[mdir].y);
		}
	};

};

/**************************************************************************/
void init_map_tables(){
	// blank out the tables
    memset(&sector_table, ' ', SECT_MAX+10);
    memset(&sector_colour,'x', SECT_MAX+10);

	sector_table[SECT_INSIDE	]='I';
	sector_table[SECT_CITY		]='C';
	sector_table[SECT_FIELD		]='-';
	sector_table[SECT_FOREST	]='+';
	sector_table[SECT_HILLS		]='^';
	sector_table[SECT_MOUNTAIN	]='%';
	sector_table[SECT_WATER_SWIM]='\'';
	sector_table[SECT_WATER_NOSWIM]='~';
	sector_table[SECT_SWAMP		]='=';
	sector_table[SECT_AIR		]='\'';
	sector_table[SECT_DESERT	]='d';
	sector_table[SECT_CAVE		]='@';
	sector_table[SECT_UNDERWATER]='"';
	sector_table[SECT_SNOW		]='"';
	sector_table[SECT_ICE		]='~';
	sector_table[SECT_TRAIL		]='-';
    sector_table[SECT_LAVA		]='@';

	sector_table[SECT_HERE		]='*';
	sector_table[SECT_MAPSCALE	]='#';
	sector_table[SECT_UNMAPPED	]=' ';
	sector_table[SECT_ROOM_DARK ]='.';
    sector_table[SECT_MAPLEVEL  ]='?';

    sector_table[SECT_UP		]='U';
    sector_table[SECT_DOWN		]='D';
    sector_table[SECT_UP_DOWN	]='B';

	sector_colour[SECT_INSIDE	]='W';
	sector_colour[SECT_CITY		]='w';
	sector_colour[SECT_FIELD	]='G';
	sector_colour[SECT_FOREST	]='g';
	sector_colour[SECT_HILLS	]='g';
	sector_colour[SECT_MOUNTAIN	]='S';
	sector_colour[SECT_WATER_SWIM]='B';
	sector_colour[SECT_WATER_NOSWIM]='b';
	sector_colour[SECT_SWAMP	]='y';
	sector_colour[SECT_AIR		]='c';
	sector_colour[SECT_DESERT	]='y';
	sector_colour[SECT_CAVE		]='S';
	sector_colour[SECT_UNDERWATER]='b';
	sector_colour[SECT_SNOW		]='W';
	sector_colour[SECT_ICE		]='C';
	sector_colour[SECT_TRAIL	]='y';
	sector_colour[SECT_LAVA		]='R';
	
	sector_colour[SECT_HERE		]='Y';
	sector_colour[SECT_MAPSCALE	]='S';
	sector_colour[SECT_UNMAPPED	]='x';
	sector_colour[SECT_ROOM_DARK]='S';
    sector_colour[SECT_MAPLEVEL ]='S';

    sector_colour[SECT_UP		]='B';
    sector_colour[SECT_DOWN		]='C';
    sector_colour[SECT_UP_DOWN	]='M';
};


/**************************************************************************/
void do_fullmap(char_data *ch, char *)
{
	doing_areamap=false;
	int line, point;
	char outline[MIL], buf[30];

	// blank out the grid
	memset(&grid, SECT_UNMAPPED, 15*15);
	memset(&count_table,0,15*15*4);
	memset(&vnum_table,0,15*15*4);	
	counter=0;

	draw_grid (ch->in_room, 5,5);
	grid[5][5]=(char)SECT_HERE;

	for(line=MIN_Y; line<MAX_Y; line++){
		outline[0]='\0';
		for(point=MIN_X; point<MAX_X; point++){
			sprintf(buf,"`%c%c", 
				sector_colour[grid[line][point]],
				sector_table[grid[line][point]]);
			strcat(outline,buf);
		}
		ch->printf( "%s\r\n`x", outline );
	}	
}


/**************************************************************************/
void do_amap(char_data *ch, char *)
{
	doing_areamap=false;
	int line, point, backup_mapscale;
	char outline[MIL], buf[30];

	// blank out the tables
	// blank out the grid
	memset(&grid, SECT_UNMAPPED, 15*15);
	memset(&count_table,0,15*15*4);
	memset(&vnum_table,0,15*15*4);	
	counter=0;

	// switch mapscales
	backup_mapscale=ch->in_room->area->mapscale;
	ch->in_room->area->mapscale=-1;
	draw_grid (ch->in_room, 5,5);
	ch->in_room->area->mapscale=backup_mapscale;
	
	grid[5][5]=(char)SECT_HERE;

	for(line=MIN_Y; line<MAX_Y; line++){
		outline[0]='\0';
		for(point=MIN_X; point<MAX_X; point++){
			sprintf(buf,"`%c%c", 
				sector_colour[grid[line][point]],
				sector_table[grid[line][point]]);
			strcat(outline,buf);
		}
		ch->printf( "%s\r\n`x", outline );
	}	
}


/**************************************************************************/
void do_mapnum(char_data *ch, char *)
{
	doing_areamap=false;
	int line, point;
	char outline[MIL], buf[30];

	// blank out the grid
	memset(&grid, SECT_UNMAPPED, 15*15);
	memset(&count_table,0,15*15*4);
	memset(&vnum_table,0,15*15*4);	
	counter=0;

	draw_grid (ch->in_room, 5,5);
	
	grid[5][5]=(char)SECT_HERE;

	for(line=MIN_Y; line<MAX_Y; line++){
		outline[0]='\0';
		for(point=MIN_X; point<MAX_X; point++){
			sprintf(buf,"`%c%3d`x ", 
				sector_colour[grid[line][point]],
				count_table[line][point]);
			strcat(outline,buf);
		}
		ch->printf( "%s\r\n\r\n`x", outline );
	}	

	for(line=MIN_Y; line<MAX_Y; line++){
		outline[0]='\0';
		for(point=MIN_X; point<MAX_X; point++){
			sprintf(buf,"`%c%c", 
				sector_colour[grid[line][point]],
				sector_table[grid[line][point]]);
			strcat(outline,buf);
		}
		ch->printf( "%s\r\n`x", outline );
	}	
}

/**************************************************************************/
DECLARE_DO_FUN(	do_look		);		
/**************************************************************************/
void do_automap(char_data *ch, char *)
{
	doing_areamap=false;
    if (IS_NPC(TRUE_CH(ch))){
		return;
	}
	
    if (USING_AUTOMAP(ch))
    {
		ch->println("AutoMAP removed.");
		REMOVE_CONFIG(ch, CONFIG_AUTOMAP);
    }
    else
    {
		ch->println("Automatic MAP set.");
		ch->println("Other related mapping commands: mapclear & nomapexits.");
		SET_CONFIG(ch, CONFIG_AUTOMAP);
    }
}

/**************************************************************************/
void do_nomapexits(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch,CONFIG2_NOMAPEXITS))
    {
        ch->println("You will now see map exits.");
        REMOVE_CONFIG2(ch,CONFIG2_NOMAPEXITS);
    }
    else
    {
        ch->println("You will no longer see the exits on the map.");
		SET_CONFIG2(ch,CONFIG2_NOMAPEXITS);
    }
}
/**************************************************************************/
void do_nomapblanks(char_data *ch, char *)
{
    if(HAS_CONFIG2(ch,CONFIG2_NOMAPBLANKS))
    {
		ch->println("There will now be blanks above and below the automapper.");
        REMOVE_CONFIG2(ch,CONFIG2_NOMAPBLANKS);
    }
    else
    {
        ch->println("There will now be no blanks above and below the automapper.");
		SET_CONFIG2(ch,CONFIG2_NOMAPBLANKS);
    }
}
/**************************************************************************/
void do_mapfirst(char_data *ch, char *)
{
    if (IS_SET(ch->dyn,DYN_MAPFIRST))
    {
        ch->println("You will now see the map after the room descript.");
        REMOVE_BIT(ch->dyn,DYN_MAPFIRST);
    }
    else
    {
        ch->println("You will now see the map after the room descript.");
        SET_BIT(ch->dyn,DYN_MAPFIRST);
    }
}
/**************************************************************************/
void do_mapclear(char_data *ch, char *)
{
    if (IS_SET(ch->dyn,DYN_MAPCLEAR))
    {
        ch->println("Drawing the map will no longer clear the screen");
        REMOVE_BIT(ch->dyn,DYN_MAPCLEAR);
    }
    else
    {
        ch->println("The screen will be clear just before drawing the map.");
        SET_BIT(ch->dyn,DYN_MAPCLEAR);
    }
}
/**************************************************************************/
#define AMAP_MY 430
#define AMAP_MX 430
unsigned char ascii_map[AMAP_MY][AMAP_MX]; 
unsigned char ascii_map_range[AMAP_MY][AMAP_MX]; 
vn_int ascii_map_vnums[AMAP_MY/2][AMAP_MX/2]; 

struct scandir_types{
	char symbol;
	short offset[4],
	  y, x;
};
int scan_minx, scan_maxx, scan_miny, scan_maxy;

scandir_types scandirs[10]={
	{'|',{DIR_NORTHWEST, DIR_NORTHEAST, DIR_WEST,  DIR_EAST}, -1,	0},// {DIR_NORTH, 
	{'-',{DIR_NORTHEAST, DIR_SOUTHEAST, DIR_NORTH, DIR_SOUTH},	0,	1},// {DIR_EAST,  
	{'|',{DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_WEST,  DIR_EAST},	1,	0},// {DIR_SOUTH, 
	{'-',{DIR_SOUTHWEST, DIR_NORTHWEST, DIR_NORTH, DIR_SOUTH},	0, -1},// {DIR_WEST,  
	{'x',{-1, -1,-1,-1},	0, -1},// {DIR_UP
	{'x',{-1, -1,-1,-1},	0, -1},// {DIR_DOWN
	{'/',{DIR_NORTH,	DIR_EAST,	DIR_NORTHWEST, DIR_SOUTHEAST},	-1, 1},// {DIR_NORTHEAST,  
	{'\\',{DIR_EAST,	DIR_SOUTH,  DIR_NORTHEAST, DIR_SOUTHWEST},	 1, 1},// {DIR_SOUTHEAST,  
	{'/',{DIR_SOUTH,	DIR_WEST,	DIR_SOUTHEAST, DIR_NORTHWEST},	 1,-1},// {DIR_SOUTHWEST,
	{'\\',{DIR_WEST,	DIR_NORTH,	DIR_SOUTHWEST, DIR_NORTHEAST},	-1,-1}// {DIR_NORTHWEST,  
};

/**************************************************************************/
void scan_direction(char_data * ch, ROOM_INDEX_DATA * pRoom, 
			  int range, int y, int x, int dir, int scale, bool dark){ 
	int i,d;

	if (!pRoom){
		bugf("scan_map(): NULL pRoom, x=%d, y=%d", x,y);
		return;
	}

    if (dark)
	{
		if(!IS_NPC(ch)
			&& !HAS_HOLYLIGHT(ch)
			&& !IS_AFFECTED(ch, AFF_DARK_VISION)
			&& room_is_dark( pRoom) )
		{
			return;
		}
	}
	
    if (ascii_map[y][x]!=SECT_UNMAPPED){
		return;
    }

	scan_minx=UMIN(scan_minx,x);
	scan_maxx=UMAX(scan_maxx,x);
	scan_miny=UMIN(scan_miny,y);
	scan_maxy=UMAX(scan_maxy,y);

/*	if ( !IS_NPC(ch)
		&& !HAS_HOLYLIGHT(ch)
		&& room_is_dark( pRoom) )
	{
		ascii_map[y][x]=SECT_ROOM_DARK;
		return;
	}
*/
    if (ch->level<pRoom->area->maplevel){
        ascii_map[y][x]=SECT_MAPLEVEL;
		return;
	}
				
	if (pRoom->area->mapscale!=scale){
		ascii_map[y][x]=SECT_MAPSCALE;
		return;
	}

    if (ascii_map[y][x]!=SECT_UNMAPPED && range<1){
		return;
    }

	ascii_map[y][x]=(char)pRoom->sector_type;
	
	if (range<1){
		return;
	}

	if (pRoom->exit[dir] 
		&& pRoom->exit[dir]->u1.to_room){
		// display closed exits to those who can see them
		if(CAN_SEE_EXIT(ch, pRoom->exit[dir])){
			ascii_map[y+ scandirs[dir].y][x+ scandirs[dir].x]=
				(char)scandirs[dir].symbol;
		}
		// now check if it isn't blocked
		if( !IS_SET(pRoom->exit[dir]->exit_info, EX_CLOSED)){
			ascii_map[y+ scandirs[dir].y][x+ scandirs[dir].x]=
				(char)scandirs[dir].symbol;
			
			scan_direction(ch, pRoom->exit[dir]->u1.to_room, range-1, 
				y+ (scandirs[dir].y*2), x+ (scandirs[dir].x*2), dir, 
				scale, dark);
		}
	}

	if(doing_areamap)
	{
		for(i=0;i<10;i++)
		{
			if(i==DIR_UP ||  i==DIR_DOWN){
				continue;
			}
			d=i;//scandirs[dir].offset[i];	

			if (pRoom->exit[d] 
				&& pRoom->exit[d]->u1.to_room)
			{
				// display closed exits to those who can see them
				if(CAN_SEE_EXIT(ch, pRoom->exit[d])){
					ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
						(char)scandirs[d].symbol;				
				}
				if( !IS_SET(pRoom->exit[d]->exit_info, EX_CLOSED))
				{
					// fill in the direction lines
					ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
						(char)scandirs[d].symbol;
					// fill in the terrain type recursively
					scan_direction( ch, pRoom->exit[d]->u1.to_room, range-1, 
						y+(scandirs[d].y*2),x + (scandirs[d].x*2),
						d, scale, dark);
				}
			}
		}
	}else{
		for(i=0;i<4;i++)
		{
			d=scandirs[dir].offset[i];	

			if (d==-1)
				continue;
			
			if (pRoom->exit[d] 
				&& pRoom->exit[d]->u1.to_room)
			{
				// display closed exits to those who can see them
				if(CAN_SEE_EXIT(ch, pRoom->exit[d])){
					ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
					(char)scandirs[d].symbol;				
				}
				if( !IS_SET(pRoom->exit[d]->exit_info, EX_CLOSED))
				{
					// fill in the direction lines
					ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
						(char)scandirs[d].symbol;
					// fill in the terrain type recursively
					scan_direction( ch, pRoom->exit[d]->u1.to_room, 0, 
						y+(scandirs[d].y*2),x + (scandirs[d].x*2),
						d, scale, dark);
				}
			}
		}
	}
	
	// fill in up and down for area map
	if(doing_areamap)
	{
		if (pRoom->exit[DIR_UP] 
			&& pRoom->exit[DIR_UP]->u1.to_room)
		{
			if (pRoom->exit[DIR_DOWN]  // up and down
				&& pRoom->exit[DIR_DOWN]->u1.to_room)
			{
				ascii_map[y][x]=(char)SECT_UP_DOWN;		
			}else{ // up only
				ascii_map[y][x]=(char)SECT_UP;			
			}
		}else{
			if (pRoom->exit[DIR_DOWN] 
				&& pRoom->exit[DIR_DOWN]->u1.to_room){
				// down only
				ascii_map[y][x]=(char)SECT_DOWN;			
			}
		}
	}
};
/**************************************************************************/
// draws in the basic exit data and then scans the map
void scan_map(char_data * ch, int range, bool dark)
{
    int d,x,y;
    ROOM_INDEX_DATA * pRoom=ch->in_room;

    x=y=(UMAX(range,0)*2);

    for (d=0; d<=9; d++){
        // up and down arent displayed
        if ((char)scandirs[d].symbol=='x'){
            continue;
        }

        if (pRoom->exit[d]
            && pRoom->exit[d]->u1.to_room)
        {
			// display closed exits to those who can see them
			if(CAN_SEE_EXIT(ch, pRoom->exit[d])){
                ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
                    (char)scandirs[d].symbol;
            }
            if( !IS_SET(pRoom->exit[d]->exit_info, EX_CLOSED))
            {
                // fill in the direction lines
                ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
                    (char)scandirs[d].symbol;

                // fill in the map recursively if not wizardeyeing
				if(!IS_SET(ch->dyn,DYN_WIZARDEYEING)){
					scan_direction( ch, pRoom->exit[d]->u1.to_room, range-1, 
						y+(scandirs[d].y*2),x + (scandirs[d].x*2),
						d, pRoom->area->mapscale, dark);
				}
            }
        }
    }
}

bool check_blind( char_data *ch );
/**************************************************************************/
void do_map(char_data *ch, char *)
{
	doing_areamap=false;
	int range=3; // center
    bool dark=false;
	int savedyn=ch->dyn;


	if(!HAS_HOLYLIGHT(ch) && !IS_AFFECTED(ch, AFF_DARK_VISION)){
		// do the reduced vision stuff
		if(IS_SET(race_table[ch->race]->flags, RACEFLAG_NIGHTMAP)){
			// if night or dark room they can see
			if( weather_info[ch->in_room->sector_type].sunlight != SUN_SET
				&& weather_info[ch->in_room->sector_type].sunlight != SUN_DARK
				&& !IS_SET(ch->in_room->affected_by, ROOMAFF_UTTERDARK ))
			{ // daytime
				if(IS_SET(ch->in_room->room_flags,ROOM_INDOORS)
					|| 	IS_SET(ch->in_room->room_flags,ROOM_DARK))
				{
					// indoors or dark room, but day time - not totally logical but hey :)
					range--;
				}else{
					// daytime, outside, non dark room
					dark=true; // called dark because dark in this context means reduced range
					range=1;
				}
			}
		}else{
			if(room_is_dark(ch->in_room)){
				OBJ_DATA *obj;

				// night time or dark room
				if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
					&&   obj->item_type == ITEM_LIGHT
					&&   obj->value[2] != 0 )
				{
					// dark room, have light - reduced range
					range--;
				}else{
					// dark room, no light, no range
					dark=true;
					range=1;
				}

			}
		}
	}

	// wizardeye
    if (ch->level<ch->in_room->area->maplevel
        || IS_SET(ch->dyn,DYN_WIZARDEYEING))
    {
        range=1;
    }
	// blindness check - use same systems as wizardeye
	if (!check_blind( ch )){
		SET_BIT(ch->dyn,DYN_WIZARDEYEING);
		range=1;
	}

	// no manual mapping if automap is disabled - Daos
	if(!HAS_HOLYLIGHT(ch) && IS_SET(ch->in_room->room_flags,ROOM_NOAUTOMAP)){
		range=0;
	}

	// blank out the grid
	memset(&ascii_map, SECT_UNMAPPED, AMAP_MX * AMAP_MY);

    scan_minx=scan_miny=range*2-1;
    scan_maxx=scan_maxy=range*2+1;

	if(HAS_CONFIG2(ch,CONFIG2_NOMAPBLANKS)){
		scan_miny=range*2-1;
	    scan_maxy=range*2+1;
        scan_minx=0;
        scan_maxx=range*4;
	}

    scan_map(ch, range, dark);
    ascii_map[range*2][range*2]=SECT_HERE;
   
	// now draw it
	// draw variables
	char buf[MSL], outline[MSL];
	int line, point;

	// clear the screen first if necessary	
	if (!IS_IRC(ch) && IS_SET(ch->dyn,DYN_MAPCLEAR) ){
		ch->print("\033[2J");
	}

	if(!HAS_CONFIG2(ch,CONFIG2_NOMAPBLANKS)){
		scan_miny=scan_minx=0;
	}

	if (IS_SET(ch->dyn,DYN_MAPCLEAR)){
        scan_miny=0;
        scan_maxy=range*4;
	}

	if (HAS_CONFIG2(ch,CONFIG2_NOMAPEXITS)){
        scan_miny=scan_miny%2==0?scan_miny:scan_miny+1;
        scan_maxy=scan_maxy%2==0?scan_maxy:scan_maxy-1;
        scan_minx=scan_minx%2==0?scan_minx:scan_minx+1;
        scan_maxx=scan_maxx%2==0?scan_maxx:scan_maxx-1;
        for(line=scan_miny;line<=scan_maxy;line+=2){
			outline[0]='\0';
			for(point=scan_minx; point<=scan_maxx; point+=2){
				sprintf(buf,"`%c%c", 
					sector_colour[ascii_map[line][point]],
					sector_table[ascii_map[line][point]]);
				strcat(outline,buf);
			}
			ch->printlnf("  %s`x",outline);
		}	
	}else{
        for(line=scan_miny; line<=scan_maxy; line++){
			outline[0]='\0';
            for(point=scan_minx; point<=scan_maxx; point++){
                if (point%2==1 || line%2==1){
					if(ascii_map[line][point]==SECT_UNMAPPED){
						sprintf(buf,"`x ");
					}else{
						sprintf(buf,"`S%c", ascii_map[line][point]);
					}
				}else{
					sprintf(buf,"`%c%c", 
						sector_colour[ascii_map[line][point]],
						sector_table[ascii_map[line][point]]);
				}
				strcat(outline,buf);
			}
			ch->printlnf("  %s`x",outline);
		}	
	}

	ch->dyn=savedyn;
}
/**************************************************************************/
void scalescan_record_vnum(ROOM_INDEX_DATA * pRoom, int y, int x)
{
	// record the room vnums
	if(ascii_map_vnums[y/2][x/2]==0){ 
		ascii_map_vnums[y/2][x/2]=pRoom->vnum;
	}else if(ascii_map_vnums[y/2][x/2]>0 && ascii_map_vnums[y/2][x/2]!=pRoom->vnum){ 
		// mark 2 exits leading into the one room locations
		ascii_map_vnums[y/2][x/2]*=-1;
	}
} 
/**************************************************************************/
void scalescan_direction(char_data * ch, ROOM_INDEX_DATA * pRoom, 
			  int range, int y, int x, int dir, int scale, bool dark){ 
	int i,d;

	if (!pRoom){
		bugf("scalescan_direction(): NULL pRoom, x=%d, y=%d", x,y);
		return;
	}

	// some debugging code
	//	logf("scalescan_direction(): x=%d, y=%d, dir=%d, %d, %s", x, y, dir, pRoom->vnum, pRoom->name);


    if (dark)
	{
		if(!IS_NPC(ch)
			&& !HAS_HOLYLIGHT(ch)
			&& room_is_dark( pRoom) )
		{
			return;
		}
	}

	if (pRoom->exit[dir] 
		&& pRoom->exit[dir]->u1.to_room){
		// display closed exits to those who can see them
		if(CAN_SEE_EXIT(ch, pRoom->exit[dir]) && pRoom->area->mapscale==scale){
			ascii_map[y+ scandirs[dir].y][x+ scandirs[dir].x]=
				(char)scandirs[dir].symbol;
		}
		scalescan_record_vnum(pRoom->exit[dir]->u1.to_room, y+ (scandirs[dir].y*2), x+ (scandirs[dir].x*2));

		// now check if it isn't blocked
		if( !IS_SET(pRoom->exit[dir]->exit_info, EX_CLOSED) 
			&& pRoom->area->mapscale==scale)
		{
			ascii_map[y+ scandirs[dir].y][x+ scandirs[dir].x]=
				(char)scandirs[dir].symbol;
		}
	}
	
    if (ascii_map[y][x]!=SECT_UNMAPPED && ascii_map_range[y][x]*2>range){
		return;
    }
	ascii_map_range[y][x]=range;

	scan_minx=UMIN(scan_minx,x);
	scan_maxx=UMAX(scan_maxx,x);
	scan_miny=UMIN(scan_miny,y);
	scan_maxy=UMAX(scan_maxy,y);

	if ( !IS_NPC(ch)
		&& !HAS_HOLYLIGHT(ch)
		&& room_is_dark( pRoom) )
	{
		ascii_map[y][x]=SECT_ROOM_DARK;
		return;
	}

    if (ch->level<pRoom->area->maplevel){
        ascii_map[y][x]=SECT_MAPLEVEL;
		return;
	}
				
	if (pRoom->area->mapscale!=scale){
		ascii_map[y][x]=SECT_MAPSCALE;
		return;
	}

    if (ascii_map[y][x]!=SECT_UNMAPPED && range<1){
		return;
    }

	ascii_map[y][x]=(char)pRoom->sector_type;
	
	if (range<1){
		return;
	}

	// if an exit isn't blocked follow it
	if (pRoom->exit[dir] 
		&& pRoom->exit[dir]->u1.to_room 
		&& !IS_SET(pRoom->exit[dir]->exit_info, EX_CLOSED))
	{
		scalescan_direction(ch, pRoom->exit[dir]->u1.to_room, range-1, 
			y+ (scandirs[dir].y*2), x+ (scandirs[dir].x*2), dir, 
			scale, dark);
	}

	for(i=0;i<MAX_DIR;i++)
	{
		if(i==DIR_UP ||  i==DIR_DOWN){
			continue;
		}
		d=i;//scandirs[dir].offset[i];	

		if (pRoom->exit[d] 
			&& pRoom->exit[d]->u1.to_room)
		{
			// display closed exits to those who can see them
            if(CAN_SEE_EXIT(ch, pRoom->exit[d])){
				ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
				(char)scandirs[d].symbol;				
			}
			scalescan_record_vnum(pRoom->exit[d]->u1.to_room, y+ (scandirs[d].y*2), x+ (scandirs[d].x*2));

			if( !IS_SET(pRoom->exit[d]->exit_info, EX_CLOSED))
			{
				// fill in the direction lines
				ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
					(char)scandirs[d].symbol;
				// fill in the terrain type recursively
				scalescan_direction( ch, pRoom->exit[d]->u1.to_room, range-1, 
					y+(scandirs[d].y*2),x + (scandirs[d].x*2),
					d, scale, dark);
			}
		}
	}
	// fill in up and down for area map
	if(doing_areamap)
	{
		if (pRoom->exit[DIR_UP] 
			&& pRoom->exit[DIR_UP]->u1.to_room)
		{
			if (pRoom->exit[DIR_DOWN]  // up and down
				&& pRoom->exit[DIR_DOWN]->u1.to_room)
			{
				ascii_map[y][x]=(char)SECT_UP_DOWN;		
			}else{ // up only
				ascii_map[y][x]=(char)SECT_UP;			
			}
		}else{
			if (pRoom->exit[DIR_DOWN] 
				&& pRoom->exit[DIR_DOWN]->u1.to_room){
				// down only
				ascii_map[y][x]=(char)SECT_DOWN;			
			}
		}
	}
};
/**************************************************************************/
void scalemap_drawmap(char_data* ch, int range)
{
	// now draw it
	// draw variables
	char buf[MSL], outline[MSL*2];
	int line, point;

	// clear the screen first if necessary	
	if (!IS_IRC(ch) && IS_SET(ch->dyn,DYN_MAPCLEAR) ){
		ch->print("\033[2J");
	}

   
	if (IS_SET(ch->dyn,DYN_MAPCLEAR)){
        scan_miny=0;
        scan_maxy=range*4;
	}

	if (HAS_CONFIG2(ch,CONFIG2_NOMAPEXITS)){
        scan_miny=scan_miny%2==0?scan_miny:scan_miny+1;
        scan_maxy=scan_maxy%2==0?scan_maxy:scan_maxy-1;
        scan_minx=scan_minx%2==0?scan_minx:scan_minx+1;
        scan_maxx=scan_maxx%2==0?scan_maxx:scan_maxx-1;
        for(line=scan_miny;line<=scan_maxy;line+=2){
			outline[0]='\0';
			for(point=scan_minx; point<=scan_maxx; point+=2){
				sprintf(buf,"`%c%c", 
					sector_colour[ascii_map[line][point]],
					sector_table[ascii_map[line][point]]);
				strcat(outline,buf);
			}
			ch->printlnf("  %s`x",outline);
		}	
	}else{
        for(line=scan_miny; line<=scan_maxy; line++){
			outline[0]='\0';
            for(point=scan_minx; point<=scan_maxx; point++){
                if (point%2==1 || line%2==1){
					if(ascii_map[line][point]==SECT_UNMAPPED){
						sprintf(buf,"`x ");
					}else{
						sprintf(buf,"`S%c", ascii_map[line][point]);
					}
				}else{
					sprintf(buf,"`%c%c", 
						sector_colour[ascii_map[line][point]],
						sector_table[ascii_map[line][point]]);
				}
				strcat(outline,buf);
			}
			ch->printlnf("  %s`x",outline);
		}	
	}
}
/**************************************************************************/
// draws in the basic exit data and then scans the map
void scalescan_map(char_data * ch, int range, bool dark)
{
    int d,x,y;
    ROOM_INDEX_DATA * pRoom=ch->in_room;
	
    x=y=(UMAX(range,0)*2);

    for (d=0; d<MAX_DIR; d++){
        // up and down arent displayed
        if ((char)scandirs[d].symbol=='x'){
            continue;
        }

        if (pRoom->exit[d]
            && pRoom->exit[d]->u1.to_room)
        {
            if(CAN_SEE_EXIT(ch, pRoom->exit[d])){
                ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
                    (char)scandirs[d].symbol;
            }

            if( !IS_SET(pRoom->exit[d]->exit_info, EX_CLOSED))
            {
                // fill in the direction lines
                ascii_map[y+ scandirs[d].y][x+ scandirs[d].x]=
                    (char)scandirs[d].symbol;

                // fill in the map recursively if not wizardeyeing
				if(!IS_SET(ch->dyn,DYN_WIZARDEYEING)){
					scalescan_direction( ch, pRoom->exit[d]->u1.to_room, range-1, 
						y+(scandirs[d].y*2),x + (scandirs[d].x*2),
						d, pRoom->area->mapscale, dark);
				}
            }
        }
    }
}
/**************************************************************************/
// shows the extent of a particular mapscale
void do_scalemap(char_data *ch, char *argument)
{
	doing_areamap=true;
	int range=40;
    bool dark=false;
	int savedyn=ch->dyn;

	if(!IS_NULLSTR(argument)){
		if(is_number(argument)){
			range=atoi(argument);
			if(range>100){
				ch->println("the number argument must be between 10 and 100.");
				return;
			}
			if(range<10){
				ch->println("the number argument must be between 10 and 100.");
				return;
			}
		}else{
			ch->println("If you specify the range of the scan, it must be as a numeric range.");
			return;
		}
	}

	// blank out the grid
	memset(&ascii_map, SECT_UNMAPPED, AMAP_MX * AMAP_MY);
	memset(&ascii_map_range, 0, AMAP_MX * AMAP_MY);
	
	memset(&ascii_map_vnums, 0, AMAP_MX * AMAP_MY/4 * sizeof(vn_int));

    scan_minx=scan_miny=range*2-1;
    scan_maxx=scan_maxy=range*2+1;

    scalescan_map(ch, range, dark);
    ascii_map[range*2][range*2]=SECT_HERE;

	scalemap_drawmap(ch, range);

	ch->println("You can specify the range as a parameter");
	ch->dyn=savedyn;
}
/**************************************************************************/
// shows the extent of an area file
void do_areamap(char_data *ch, char * argument)
{
	int backup_mapscale;
	// switch mapscales
	backup_mapscale=ch->in_room->area->mapscale;
	ch->in_room->area->mapscale=-1;
	do_scalemap(ch,argument);
	ch->in_room->area->mapscale=backup_mapscale;
}
/**************************************************************************/
// shows scalemap with mxp info - Kal Jan01
void do_scalemxp(char_data *ch, char *argument)
{
	doing_areamap=true;
	int range=25;
    bool dark=false;
	int savedyn=ch->dyn;

	if(!IS_NULLSTR(argument)){
		if(is_number(argument)){
			range=atoi(argument);
			if(range>100){
				ch->println("the number argument must be between 10 and 100.");
				return;
			}
			if(range<10){
				ch->println("the number argument must be between 10 and 100.");
				return;
			}
		}else{
			ch->println("If you specify the range of the scan, it must be as a numeric range.");
			return;
		}
	}

	// blank out the grid
	memset(&ascii_map, SECT_UNMAPPED, AMAP_MX * AMAP_MY);
	memset(&ascii_map_vnums, 0, AMAP_MX * AMAP_MY/4);

    scan_minx=scan_miny=range*2-1;
    scan_maxx=scan_maxy=range*2+1;

    scalescan_map(ch, range, dark);
    ascii_map[range*2][range*2]=SECT_HERE;
   
	// now draw it
	// draw variables
	char buf[MSL], outline[MSL*2];
	int line, point;

	// clear the screen first if necessary	
	if (!IS_IRC(ch) && IS_SET(ch->dyn,DYN_MAPCLEAR) ){
		ch->print("\033[2J");
	}

   
	if (IS_SET(ch->dyn,DYN_MAPCLEAR)){
        scan_miny=0;
        scan_maxy=range*4;
	}

	if (HAS_CONFIG2(ch,CONFIG2_NOMAPEXITS)){
        scan_miny=scan_miny%2==0?scan_miny:scan_miny+1;
        scan_maxy=scan_maxy%2==0?scan_maxy:scan_maxy-1;
        scan_minx=scan_minx%2==0?scan_minx:scan_minx+1;
        scan_maxx=scan_maxx%2==0?scan_maxx:scan_maxx-1;
        for(line=scan_miny;line<=scan_maxy;line+=2){
			outline[0]='\0';
			for(point=scan_minx; point<=scan_maxx; point+=2){
				sprintf(buf,"`%c%c", 
					sector_colour[ascii_map[line][point]],
					sector_table[ascii_map[line][point]]);
				strcat(outline,buf);
			}
			ch->printlnf("  %s`x",outline);
		}	
	}else{
        for(line=scan_miny; line<=scan_maxy; line++){
			outline[0]='\0';
            for(point=scan_minx; point<=scan_maxx; point++){
                if (point%2==1 || line%2==1){
					if(ascii_map[line][point]==SECT_UNMAPPED){
						sprintf(buf,"`x ");
					}else{
						sprintf(buf,"`S%c", ascii_map[line][point]);
					}
				}else{
					if(ascii_map_vnums[line/2][point/2]==0){
						sprintf(buf,"`%c%c", 
							sector_colour[ascii_map[line][point]],
							sector_table[ascii_map[line][point]]);
					}else if(ascii_map_vnums[line/2][point/2]<0){
// ** The element definition didn't work :( (nor a heap of variants of it)
// ch->print( "\033[1z<!ELEMENT Rm '<SEND href=\"goto &rvnum;\" hint=\"room &rvnum;\">'>" );
// ch->print( "\033[1z<Rm vnum=45>testroom</Rm> " );
						
						sprintf(buf, "`R%s", 
							mxp_create_tag(ch, 
									FORMATF("rmv %d", ascii_map_vnums[line/2][point/2]*-1),
									FORMATF("%c", sector_table[ascii_map[line][point]])));

/*						sprintf(buf,"{R\033[1z<SEND \"goto %d\" hint=\"room %d\">%c</send>", 
							ascii_map_vnums[line/2][point/2]*-1,
							ascii_map_vnums[line/2][point/2]*-1,
							sector_table[ascii_map[line][point]]);
*/
					}else if(range<40 || ((line/2)%3==0 && (point/2)%3==0)){
						if(range>25){
							sprintf(buf, "`%c%s", 
								sector_colour[ascii_map[line][point]],
								mxp_create_tag(ch, 
										FORMATF("rmv %d", ascii_map_vnums[line/2][point/2]),
										FORMATF("%c", sector_table[ascii_map[line][point]])));
						}else{
							int vn=ascii_map_vnums[line/2][point/2];
							ROOM_INDEX_DATA *r=get_room_index(vn);

							sprintf(buf, "`%c%s", 
								sector_colour[ascii_map[line][point]],
								mxp_create_tag(ch, 
									FORMATF("rmvh %d \"%.25s\"", vn, r?strip_colour(r->name):"unfound?!?"),
										FORMATF("%c", sector_table[ascii_map[line][point]])));

						}

/*						sprintf(buf,"{%c\033[1z<SEND \"goto %d\" hint=\"room %d\">%c</send>", 
							sector_colour[ascii_map[line][point]],
							ascii_map_vnums[line/2][point/2],
							ascii_map_vnums[line/2][point/2],
							sector_table[ascii_map[line][point]]);
*/
					}else{
						sprintf(buf,"`%c%c", 
							sector_colour[ascii_map[line][point]],
							sector_table[ascii_map[line][point]]);
					}
				}
				strcat(outline,buf);
			}
			ch->printlnf("  %s`x",outline);
		}	
	}

	ch->println("You can specify the range as a parameter");
	ch->dyn=savedyn;
}
/**************************************************************************/
/**************************************************************************/


