/**************************************************************************/
// mem.cpp - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with all the licenses *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************
 * >> Original Diku Mud copyright (c)1990, 1991 by Sebastian Hammer,       *
 *    Michael Seifert, Hans Henrik Staerfeldt, Tom Madsen, & Katja Nyboe.  *
 * >> Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael       *
 *    Chastain, Michael Quan, and Mitchell Tse.                            *
 * >> ROM 2.4 is copyright 1993-1995 Russ Taylor and has been brought to   *
 *    you by the ROM consortium: Russ Taylor(rtaylor@pacinfo.com),         *
 *    Gabrielle Taylor(gtaylor@pacinfo.com) & Brian Moore(rom@rom.efn.org) *
 * >> Oblivion 1.2 is copyright 1996 Wes Wagner                            *
 **************************************************************************/
/***************************************************************************
 *  File: mem.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
#include "include.h" // dawn standard includes

// Globals
extern		int		top_reset;
extern		int		top_area;
extern		int		top_exit;
extern		int		top_ed;
extern		int		top_room;
extern		int		top_mprog_index;

AREA_DATA		*	area_free;
EXIT_DATA		*	exit_free;
ROOM_INDEX_DATA	*	room_index_free;
OBJ_INDEX_DATA	*	obj_index_free;
SHOP_DATA		*	shop_free;
MOB_INDEX_DATA	*	mob_index_free;
RESET_DATA		*	reset_free;
ROOM_ECHO_DATA	*	room_echo_free;
AREA_ECHO_DATA  *   area_echo_free;

void	free_extra_descr	args( ( EXTRA_DESCR_DATA *pExtra ) );
void	free_affect		args( ( AFFECT_DATA *af ) );
void	free_mprogs              args ( ( MUDPROG_TRIGGER_LIST *mp ) );

/**************************************************************************/
RESET_DATA *new_reset_data( void )
{
    RESET_DATA *pReset;

    if ( !reset_free )
    {
        pReset          =  (RESET_DATA *)alloc_perm( sizeof(*pReset) );
        top_reset++;
    }
    else
    {
        pReset          =   reset_free;
        reset_free      =   reset_free->next;
    }

    pReset->next        =   NULL;
    pReset->command     =   'X';
    pReset->arg1        =   0;
    pReset->arg2        =   0;
    pReset->arg3        =   0;
    pReset->arg4        =   0;

    return pReset;
}

/**************************************************************************/
void free_reset_data( RESET_DATA *pReset )
{
    pReset->next            = reset_free;
    reset_free              = pReset;
    return;
}
/**************************************************************************/
AREA_DATA *new_area( void )
{
    AREA_DATA *pArea;
    char buf[MIL];

    if ( !area_free )
    {
        pArea   =   (AREA_DATA *)alloc_perm( sizeof(*pArea) );
       top_area++;
    }
    else
    {
        pArea       =   area_free;
        area_free   =   area_free->next;
    }

    pArea->min_vnum         =   0;
    pArea->max_vnum         =   0;
    pArea->next             =   NULL;
    pArea->vnumsort_next    =   NULL;
    pArea->name             =   str_dup( "New area" );
    pArea->olc_flags        =   OLCAREA_ADDED;
    pArea->area_flags       =   AREA_OLCONLY + AREA_NOTELEPORT + AREA_NOSCRY;
    pArea->security         =   9;
    pArea->builders         =   str_dup( "None" );
	pArea->credits			=	str_dup("");
	pArea->continent		= NULL;		// default them as any
	for (int br=0;br<MAX_BUILD_RESTRICTS;br++)
	{
		pArea->build_restricts[br]= str_dup("");
	}   
    pArea->min_vnum         =   0;
    pArea->max_vnum         =   0;
    pArea->age              =   0;
    pArea->nplayer          =   0;
    pArea->empty            =   true;

	{
		int filenum=0;
		bool dup_in_list;
		AREA_DATA *pArealist;

		for(filenum=0; ; filenum++){
			sprintf( buf, "area%d.are", filenum);
			if(file_exists(buf)){
				continue;
			}
			dup_in_list=false;
			for ( pArealist= area_first; pArealist; pArealist= pArealist->next )
			{
				if (!str_cmp( buf, pArealist->file_name)){
					dup_in_list=true;
					break;
				}
			}
			if(dup_in_list){
				continue;
			}
			break; // found an available file name
		}
	}

    pArea->file_name        =   str_dup( buf );
	pArea->short_name		=	str_dup("");
    pArea->vnum             =   top_area-1;
	pArea->colour           =   &str_empty[0];
	
	pArea->maplevel			=	LEVEL_IMMORTAL;
	pArea->vnum_offset		=   0;
    return pArea;
}
/**************************************************************************/
void free_area( AREA_DATA *pArea )
{
//@@@ need to make functions that use this remove areas from the
// vnumsort_next list

    free_string( pArea->name );
    free_string( pArea->file_name );
    free_string( pArea->builders );
    free_string( pArea->credits );

    pArea->next         =   area_free->next;
    area_free           =   pArea;
    return;
}
/**************************************************************************/
EXIT_DATA *new_exit( void )
{
    EXIT_DATA *pExit;

    if ( !exit_free )
    {
        pExit           =   (EXIT_DATA *)alloc_perm( sizeof(*pExit) );
        top_exit++;
    }
    else
    {
        pExit           =   exit_free;
        exit_free       =   exit_free->next;
    }

    pExit->u1.to_room   =   NULL;
    pExit->next         =   NULL;
    pExit->exit_info    =   0;
    pExit->key          =   0;
    pExit->keyword      =   &str_empty[0];
    pExit->description  =   &str_empty[0];
    pExit->rs_flags     =   0;

    return pExit;
}
/**************************************************************************/
void free_exit( EXIT_DATA *pExit )
{
    free_string( pExit->keyword );
    free_string( pExit->description );

    pExit->next         =   exit_free;
    exit_free           =   pExit;
    return;
}
/**************************************************************************/
// Kal - Jan 2001
room_echo_data *new_room_echo( void )
{
    room_echo_data *recho;

    if( room_echo_free ){
        recho			=   room_echo_free;
        room_echo_free	=   room_echo_free->next;
    }else{
        recho			=   (room_echo_data*)alloc_perm( sizeof(*recho) );
    }

	recho->echotext		=	&str_empty[0];
	recho->firsthour	=	23;
	recho->lasthour		=	0;
	recho->percentage	=	4;
	recho->next			=	NULL;
    return recho;
}
/**************************************************************************/
// Kal - Jan 2001
void free_room_echo( room_echo_data	*recho)
{
    replace_string( recho->echotext, "");
    recho->next         =   room_echo_free;
    room_echo_free      =   recho;
    return;
}
/**************************************************************************/
ROOM_INDEX_DATA *new_room_index( void )
{
    ROOM_INDEX_DATA *pRoom;
    int door;

    if ( !room_index_free )
    {
        pRoom           =   (ROOM_INDEX_DATA *)alloc_perm( sizeof(*pRoom) );
		pRoom->name             =   &str_empty[0];
		pRoom->description      =   &str_empty[0];
		pRoom->owner			=	&str_empty[0];
		pRoom->invite_list		=	&str_empty[0];
        top_room++;
    }
    else
    {
        pRoom           =   room_index_free;
        room_index_free =   room_index_free->next;
    }

    pRoom->next             =   NULL;
    pRoom->people           =   NULL;
    pRoom->contents         =   NULL;
    pRoom->extra_descr      =   NULL;
    pRoom->area             =   NULL;
	pRoom->lockers			=	NULL;
	pRoom->echoes			=	NULL;

    for ( door=0; door < MAX_DIR; door++ )
        pRoom->exit[door]   =   NULL;

    pRoom->vnum             =   0;
    pRoom->room_flags       =   0;
    pRoom->light            =   0;
    pRoom->sector_type      =   0;
    pRoom->clan				=	0;
    pRoom->heal_rate	    =   100;
    pRoom->mana_rate	    =   100;

    return pRoom;
}
/**************************************************************************/
// Kal, Jan 2001
void free_room_index( ROOM_INDEX_DATA *pRoom )
{
    int door;
    EXTRA_DESCR_DATA *pExtra, *pExtra_next;
    RESET_DATA *pReset, *pReset_next;

    replace_string( pRoom->name, "");
    replace_string( pRoom->description, "");
    replace_string( pRoom->owner, "" );
	replace_string( pRoom->invite_list, "" );

    for ( door = 0; door < MAX_DIR; door++ )
    {
        if ( pRoom->exit[door] ){
            free_exit( pRoom->exit[door] );
		}
    }

    for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra_next )
    {
		pExtra_next=pExtra->next;
        free_extra_descr( pExtra );
    }

    for ( pReset = pRoom->reset_first; pReset; pReset = pReset_next)
    {
		pReset_next=pReset->next;
        free_reset_data( pReset );
    }

    pRoom->next     =   room_index_free;
    room_index_free =   pRoom;
    return;
}

extern AFFECT_DATA *affect_free;

/**************************************************************************/
SHOP_DATA *new_shop( void )
{
    SHOP_DATA *pShop;
    int buy;

    if ( !shop_free )
    {
        pShop           =   (SHOP_DATA *)alloc_perm( sizeof(*pShop) );
        top_shop++;
    }
    else
    {
        pShop           =   shop_free;
        shop_free       =   shop_free->next;
    }

    pShop->next         =   NULL;
    pShop->keeper       =   0;

    for ( buy=0; buy<MAX_TRADE; buy++ )
        pShop->buy_type[buy]    =   0;

    pShop->profit_buy   =   100;
    pShop->profit_sell  =   100;
    pShop->open_hour    =   0;
    pShop->close_hour   =   23;

    return pShop;
}
/**************************************************************************/
void free_shop( SHOP_DATA *pShop )
{
	if(pShop){
	    pShop->next = shop_free;
		shop_free   = pShop;
	}
    return;
}
/**************************************************************************/
OBJ_INDEX_DATA *new_obj_index( void )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( !obj_index_free )
    {
        pObj           =   (OBJ_INDEX_DATA *)alloc_perm( sizeof(*pObj) );
        top_obj_index++;
    }
    else
    {
        pObj            =   obj_index_free;
        obj_index_free  =   obj_index_free->next;
    }

    pObj->next          =   NULL;
    pObj->extra_descr   =   NULL;
    pObj->affected      =   NULL;
    pObj->area          =   NULL;
    pObj->relative_size =   50;
    pObj->absolute_size =   0;
    pObj->name          =   str_dup( "no name" );
    pObj->short_descr   =   str_dup( "(no short description)" );
    pObj->description   =   str_dup( "(no description)" );
    pObj->vnum          =   0;
    pObj->item_type     =   ITEM_TRASH;
    pObj->extra_flags   =   0;
    pObj->wear_flags    =   0;
    pObj->count         =   0;
    pObj->weight        =   0;
    pObj->cost          =   0;
    pObj->material      =   str_dup( "unknown" );
    pObj->condition     =   100;
    for ( value = 0; value < 5; value++ ){
        pObj->value[value]  =   0;
	}
	pObj->ospec_fun		=	NULL;

    return pObj;
}
/**************************************************************************/
void free_obj_index( OBJ_INDEX_DATA *pObj )
{
    EXTRA_DESCR_DATA *pExtra;
    AFFECT_DATA *pAf;

    free_string( pObj->name );
    free_string( pObj->short_descr );
    free_string( pObj->description );	

    for ( pAf = pObj->affected; pAf; pAf = pAf->next ){
        free_affect( pAf );
    }

    for ( pExtra = pObj->extra_descr; pExtra; pExtra = pExtra->next ){
        free_extra_descr( pExtra );
    }
	free_string( pObj->material); // memory leak fix by Spilinek
    
    pObj->next              = obj_index_free;
    obj_index_free          = pObj;
    return;
}
/**************************************************************************/
MOB_INDEX_DATA *new_mob_index( void )
{
    MOB_INDEX_DATA *pMob;

    if ( !mob_index_free )
    {
        pMob           =   (MOB_INDEX_DATA *)alloc_perm( sizeof(*pMob) );
        top_mob_index++;
    }
    else
    {
        pMob            =   mob_index_free;
        mob_index_free  =   mob_index_free->next;
    }

    pMob->next          =   NULL;
    pMob->spec_fun      =   NULL;
    pMob->pShop         =   NULL;
    pMob->area          =   NULL;
    pMob->player_name   =   str_dup( "no name" );
    pMob->short_descr   =   str_dup( "(no short description)" );
    pMob->long_descr    =   str_dup( "(no default description)" );
    pMob->description   =   &str_empty[0];
    pMob->vnum          =   0;
    pMob->count         =   0;
    pMob->killed        =   0;
    pMob->sex           =   0;
    pMob->level         =   0;
    pMob->act           =   ACT_IS_NPC;
    pMob->affected_by   =   0;

    pMob->alliance      =   0;
    pMob->tendency      =   0;
    pMob->hitroll	=   0;
    pMob->race          =   race_lookup( "human" );
    pMob->form          =   0;           
    pMob->parts         =   0;           
    pMob->imm_flags     =   0;           
    pMob->res_flags     =   0;           
    pMob->vuln_flags    =   0;           
    pMob->material      =   str_dup("unknown");  
    pMob->off_flags     =   0;          
    pMob->size          =   SIZE_MEDIUM;
    pMob->ac[AC_PIERCE]	=   0;          
    pMob->ac[AC_BASH]	=   0;          
    pMob->ac[AC_SLASH]	=   0;          
    pMob->ac[AC_EXOTIC]	=   0;          
    pMob->hit[DICE_NUMBER]	=   0;    
    pMob->hit[DICE_TYPE]	=   0;    
    pMob->hit[DICE_BONUS]	=   0;    
    pMob->mana[DICE_NUMBER]	=   0;    
    pMob->mana[DICE_TYPE]	=   0;    
    pMob->mana[DICE_BONUS]	=   0;    
    pMob->damage[DICE_NUMBER]=	0;   
    pMob->damage[DICE_TYPE]	=   0;   
    pMob->damage[DICE_NUMBER]=	0;   
    pMob->start_pos             =   POS_STANDING;
    pMob->default_pos           =   POS_STANDING;
    pMob->wealth                =   0;

    return pMob;
}
/**************************************************************************/
void free_mob_index( MOB_INDEX_DATA *pMob )
{
    free_string( pMob->player_name );
    free_string( pMob->short_descr );
    free_string( pMob->long_descr );
    free_string( pMob->description );
    free_mprogs( pMob->mob_triggers );
	
    free_shop( pMob->pShop );
	free_string( pMob->material); // memory leak fix by Spilinek

    pMob->next              = mob_index_free;
    mob_index_free          = pMob;
	top_mob_index--;
    return;
}
/**************************************************************************/
MUDPROG_CODE              *       mpcode_free;

MUDPROG_CODE *new_mpcode(void)
{
     MUDPROG_CODE *NewCode;

     if (!mpcode_free)
     {
         NewCode = (MUDPROG_CODE *)alloc_perm(sizeof(*NewCode) );
         top_mprog_index++;
     }
     else
     {
         NewCode     = mpcode_free;
         mpcode_free = mpcode_free->next;
     }

     NewCode->vnum		= 0;
     NewCode->code		= str_dup("");
	 NewCode->author	= str_dup("");
	 NewCode->disabled=false;
	 NewCode->disabled_text=str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}
/**************************************************************************/
void free_mpcode(MUDPROG_CODE *pMcode)
{
    replace_string(pMcode->code,"");
	replace_string(pMcode->author,"");
    pMcode->next = mpcode_free;
    mpcode_free  = pMcode;
    return;
}
/**************************************************************************/
// Daos - Dec. 2001
area_echo_data *new_area_echo( void )
{
    area_echo_data *aecho;

    if( area_echo_free ){
        aecho			=   area_echo_free;
        area_echo_free	=   area_echo_free->next;
    }else{
        aecho			=   (area_echo_data*)alloc_perm( sizeof(*aecho) );
    }

	aecho->echotext		=	&str_empty[0];
	aecho->firsthour	=	23;
	aecho->lasthour		=	0;
	aecho->percentage	=	4;
	aecho->next			=	NULL;
    return aecho;
}
/**************************************************************************/
// Daos - Dec. 2001
void free_area_echo( area_echo_data	*aecho)
{
    replace_string( aecho->echotext, "");
    aecho->next         =   area_echo_free;
    area_echo_free      =   aecho;
    return;
}

/**************************************************************************/
extern char *string_space;
/**************************************************************************/
// called while the mud is shuting down()
void deallocate_all_memory()
{
	// undo the effect of init_string_space();
	free(string_space);
}
/**************************************************************************/
/**************************************************************************/


