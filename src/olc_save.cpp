/**************************************************************************/
// olc_save.cpp - saves area files etc
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 ***************************************************************************/
/* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#include "include.h" // dawn standard includes

#include "areas.h"
#include "olc.h"
#include "immquest.h"
#include "offmoot.h"
#include "interp.h"
#include "help.h"
#include "lockers.h"

char *gamble_name( GAMBLE_FUN *function);
void save_attunes( FILE *fp, AREA_DATA *pArea );
void save_mix_db( void);
DECLARE_DO_FUN( do_saveherbs );

#define DIF(a,b) (~((~a)|(b)))

// areas.cpp
void save_rooms_NAFF( FILE *fp, AREA_DATA *pArea );
void save_object_NAFF( FILE *fp, OBJ_INDEX_DATA *pObjIndex );
void save_mobile_NAFF( FILE *fp, MOB_INDEX_DATA *pMobIndex );
/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */

/* #define VERBOSE */

/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string( const char *str )
{
	static int mri; // multi result index
	static char *multi_result[5]; // circular managed result buffer 
	// nothing to do with empty strings
	if( IS_NULLSTR(str)){
        return "";
	}
	// rotate buffers
	++mri%=5;

	manage_dynamic_buffer(&multi_result[mri], str_len(str)+2); // maintain result so always has enough space
	char *result=multi_result[mri]; // managed result buffer

	const char *in=str;
	char *out=result;

    while(*in!='\0') // loop thru, copying into result
    {
		while ((*in== '\r' || *in== '~')){
			in++;
		}
		if(*in!='\0'){
	        *out++= *in++;
		}
    }
    *out = '\0'; // terminate the string
    return result;
}


/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list()
{
    FILE *fp;
    AREA_DATA *pArea;

    if ( ( fp = fopen( AREA_LIST".write", "w" ) ) == NULL ){
        bugf("save_area_list(): fopen '%s' for write - error %d (%s)",
			AREA_LIST".write", errno, strerror( errno));
    }else{
		for( pArea = area_first; pArea; pArea = pArea->next ){
			fprintf( fp, "%s\n", pArea->file_name);
		}

		int bytes_written=fprintf( fp, "$\n" );
		fclose( fp );
		if( bytes_written != str_len("$\n") ){
			bugf("save_area_list(): fprintf to '%s' - error %d (%s)",
				AREA_LIST".write", errno, strerror( errno));
			bugf("Incomplete write of " AREA_LIST ".write, write aborted - check diskspace!");
		}else{		
			unlink(AREA_LIST);
			rename(AREA_LIST".write", AREA_LIST);
		}
    }
    return;
}


/**************************************************************************/
/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 * 
 * buf[] must hold at least 32+1 characters.
 *
 * -- Hugin
 */
char *fwrite_flag( long flags, char buf[] )
{
    char offset;
    char *cp;

	static char local_buf[33];
	if(buf==NULL){
		buf=local_buf;
	}
	
	buf[0] = '\0';
	
	if ( flags == 0 )
    {
		strcpy( buf, "0" );
		return buf;
    }
	
    /* 32 -- number of bits in a long */
	
    for ( offset = 0, cp = buf; offset < 32; offset++ ){
		if ( flags & ( (long)1 << offset ) )
		{
			if ( offset <= 'Z' - 'A' ){
				*(cp++) = 'A' + offset;
			}else{
				*(cp++) = 'a' + offset - ( 'Z' - 'A' + 1 );
			}
		}
	}
		
	*cp = '\0';
	
	return buf;
}

/**************************************************************************/
void save_mudprogs( FILE *fp, AREA_DATA *pArea )
{
	MUDPROG_CODE *pMprog;
	int i;
	
	fprintf(fp, "#MUDPROGS\n");
	
	for( i = pArea->max_vnum; i>=pArea->min_vnum; i-- )
	{
		if ( (pMprog = get_mprog_index(i) ) != NULL)
		{
		      fprintf(fp, "#%d\n", i);
			  
			  // mudprog names from version 4 up
			  if (area_last->version>3) 
			  {
				  fprintf(fp, "%s~\n", pMprog->title?fix_string(pMprog->title):"");
				  fprintf(fp, "%s~\n", pMprog->author?fix_string(pMprog->author):"");
				  fprintf(fp, "%s~\n", pMprog->last_editor?fix_string(pMprog->last_editor):"");
				  fprintf(fp, "%ld\n", (long)pMprog->last_editdate);
			  }
			  fprintf(fp, "%s~\n", fix_string(pMprog->code));
		}
	}
	
	fprintf(fp,"#0\n\n");
	return;
}

/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
 ****************************************************************************/
void save_mobile( FILE *fp, MOB_INDEX_DATA *pMobIndex, int version)
{
    sh_int race = pMobIndex->race;
    MUDPROG_TRIGGER_LIST *pMprog;
    char buf[MSL];
    long temp;

    fprintf( fp, "#%d\n",       pMobIndex->vnum );
    fprintf( fp, "%s~\n",       pMobIndex->player_name );
    fprintf( fp, "%s~\n",       pMobIndex->short_descr );
    fprintf( fp, "%s~\n",       fix_string( pMobIndex->long_descr ) );
    fprintf( fp, "%s~\n",       fix_string( pMobIndex->description) );
    fprintf( fp, "%s~\n",       race_table[race]->name );
    fprintf( fp, "%s ",         fwrite_flag( pMobIndex->act, buf ) );
    fprintf( fp, "%s\n",        fwrite_flag( pMobIndex->act2, buf ) );
	fprintf( fp, "%s ",         fwrite_flag( pMobIndex->affected_by, buf ) );
	fprintf( fp, "%s\n",        fwrite_flag( pMobIndex->affected_by2, buf ) );
    fprintf( fp, "%d %d\n",     pMobIndex->tendency, pMobIndex->alliance );
    fprintf( fp, "%d\n",        pMobIndex->xp_mod );
    fprintf( fp, "%d ",         pMobIndex->level );
    fprintf( fp, "%d ",         pMobIndex->hitroll );
    fprintf( fp, "%dd%d+%d ",	pMobIndex->hit[DICE_NUMBER], 
                                pMobIndex->hit[DICE_TYPE], 
                                pMobIndex->hit[DICE_BONUS] );
    fprintf( fp, "%dd%d+%d ",	pMobIndex->mana[DICE_NUMBER], 
                                pMobIndex->mana[DICE_TYPE], 
                                pMobIndex->mana[DICE_BONUS] );
    fprintf( fp, "%dd%d+%d ",	pMobIndex->damage[DICE_NUMBER], 
								pMobIndex->damage[DICE_TYPE], 
								pMobIndex->damage[DICE_BONUS] );
    fprintf( fp, "%s\n",		attack_table[pMobIndex->dam_type].name );

	if(version<6){ //  Area files versions 5 and lower divided ac by 10 in storage
		fprintf( fp, "%d %d %d %d\n", pMobIndex->ac[AC_PIERCE] / 10, 
									pMobIndex->ac[AC_BASH]   / 10, 
									pMobIndex->ac[AC_SLASH]  / 10, 
									pMobIndex->ac[AC_EXOTIC] / 10 );
	}else{
		fprintf( fp, "%d %d %d %d\n", pMobIndex->ac[AC_PIERCE], 
									pMobIndex->ac[AC_BASH], 
									pMobIndex->ac[AC_SLASH], 
									pMobIndex->ac[AC_EXOTIC]);
	}
    fprintf( fp, "%s ",			fwrite_flag( pMobIndex->off_flags,  buf ) );
    fprintf( fp, "%s ",			fwrite_flag( pMobIndex->imm_flags,  buf ) );
    fprintf( fp, "%s ",			fwrite_flag( pMobIndex->res_flags,  buf ) );
    fprintf( fp, "%s\n",		fwrite_flag( pMobIndex->vuln_flags, buf ) );
    fprintf( fp, "%s %s %s %ld\n",
								position_table[pMobIndex->start_pos].short_name,
								position_table[pMobIndex->default_pos].short_name,
								sex_table[pMobIndex->sex].name,
								pMobIndex->wealth );
	fprintf( fp, "%s ",			fwrite_flag( pMobIndex->form,  buf ) );
	fprintf( fp, "%s ",			fwrite_flag( pMobIndex->parts, buf ) );

    fprintf( fp, "%s ",			size_table[pMobIndex->size].name );


	sprintf(buf, "%s", IS_NULLSTR(pMobIndex->material)? "unknown":pMobIndex->material);

	if (has_space(buf)){
		fprintf( fp, "'%s'\n" , buf); //quotes around materials with 2 words
	}else{
		fprintf( fp, "%s\n" , buf);
	}

    if (pMobIndex->group){
     	fprintf( fp, "G %d\n", pMobIndex->group);
	}
    if (pMobIndex->helpgroup){
     	fprintf( fp, "H %d\n", pMobIndex->helpgroup);
	}

    if ((temp = DIF(race_table[race]->act,pMobIndex->act)))
     	fprintf( fp, "F act %s\n", fwrite_flag(temp, buf) );

    if ((temp = DIF(race_table[race]->aff,pMobIndex->affected_by)))
     	fprintf( fp, "F aff %s\n", fwrite_flag(temp, buf) );

    if ((temp = DIF(race_table[race]->off,pMobIndex->off_flags)))
     	fprintf( fp, "F off %s\n", fwrite_flag(temp, buf) );

    if ((temp = DIF(race_table[race]->imm,pMobIndex->imm_flags)))
     	fprintf( fp, "F imm %s\n", fwrite_flag(temp, buf) );

    if ((temp = DIF(race_table[race]->res,pMobIndex->res_flags)))
     	fprintf( fp, "F res %s\n", fwrite_flag(temp, buf) );

    if ((temp = DIF(race_table[race]->vuln,pMobIndex->vuln_flags)))
     	fprintf( fp, "F vul %s\n", fwrite_flag(temp, buf) );

    if ((temp = DIF(race_table[race]->form,pMobIndex->form)))
     	fprintf( fp, "F for %s\n", fwrite_flag(temp, buf) );

    if ((temp = DIF(race_table[race]->parts,pMobIndex->parts)))
    	fprintf( fp, "F par %s\n", fwrite_flag(temp, buf) );

    for (pMprog = pMobIndex->mob_triggers; pMprog; pMprog = pMprog->next)
    {
		fprintf(fp, "M ");
		if(pMprog->pos_flags){ // encode a little = so say position flags flow
			fprintf(fp, "= %s ", flags_print(pMprog->pos_flags));
		}
        fprintf(fp, "%s %d %s~\n",
        mprog_type_to_name(pMprog->trig_type), pMprog->prog->vnum,
                pMprog->trig_phrase);
    }
    fprintf(fp, "\n");

    return;
}


/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles( FILE *fp, AREA_DATA *pArea )
{
    int i;
    MOB_INDEX_DATA *pMob;

    fprintf( fp, "#MOBILES\n" );

    for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    {
		if ( (pMob = get_mob_index( i )) ){
			if ( pArea->version<11){
				save_mobile( fp, pMob, pArea->version);
			}else{
				save_mobile_NAFF( fp, pMob );
			}
		}
    }

    fprintf( fp, "#0\n\n\n\n" );
    return;
}

/**************************************************************************/
// write the object values to fp
void save_object_values( FILE *fp, OBJ_INDEX_DATA *pObjIndex )
{
	assertp(fp);
    char buf[MSL], buf2[MSL];

    switch ( pObjIndex->item_type )
    {
        default:
	    fprintf( fp, "%s ",	fwrite_flag( pObjIndex->value[0], buf ) );
	    fprintf( fp, "%s ",	fwrite_flag( pObjIndex->value[1], buf ) );
	    fprintf( fp, "%s ",	fwrite_flag( pObjIndex->value[2], buf ) );
	    fprintf( fp, "%s ",	fwrite_flag( pObjIndex->value[3], buf ) );
	    fprintf( fp, "%s\n",fwrite_flag( pObjIndex->value[4], buf ) );
	    break;

        case ITEM_LIGHT:
			fprintf( fp, "0 0 %d 0 0\n",
				pObjIndex->value[2] < 1 ? -1 : pObjIndex->value[2] ); // infinite 
	    break;

		case ITEM_INSTRUMENT:
			fprintf( fp, "%d %d 0 0 0\n",
				pObjIndex->value[0],
				pObjIndex->value[1] );
		    break;

        case ITEM_MONEY:
            fprintf( fp, "%d %d 0 0 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1]);
            break;
            
        case ITEM_DRINK_CON:
            fprintf( fp, "%d %d %s %d 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     pack_word(liq_table[pObjIndex->value[2]].liq_name),
				     pObjIndex->value[3]);
            break;

		case ITEM_FOUNTAIN:
	    fprintf( fp, "%d %d %s %d 0\n",
	             pObjIndex->value[0],
	             pObjIndex->value[1],
	             pack_word(liq_table[pObjIndex->value[2]].liq_name),
				 pObjIndex->value[3]);
	    break;

        case ITEM_CONTAINER:
		case ITEM_CAULDRON:
		case ITEM_FLASK:
		case ITEM_MORTAR:
            fprintf( fp, "%d %s %d %d %d\n",
                     pObjIndex->value[0],
                     fwrite_flag( pObjIndex->value[1], buf ),
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;

		case ITEM_COMPONENT:
            fprintf( fp, "%d %s 0 0 0\n",
                     pObjIndex->value[0],	// charges
					 pObjIndex->value[1]==-1?"none":  	// spell name
						pack_word(skill_table[pObjIndex->value[1]].name));
            break;
            
        case ITEM_FOOD:
            fprintf( fp, "%d %d 0 %s 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     fwrite_flag( pObjIndex->value[3], buf ) );
            break;
            
        case ITEM_PORTAL:
            fprintf( fp, "%d %s %s %d 0\n",
                     pObjIndex->value[0],
                     fwrite_flag( pObjIndex->value[1], buf ),
                     fwrite_flag( pObjIndex->value[2], buf2 ),
                     pObjIndex->value[3]);
            break;
            
        case ITEM_FURNITURE:
            fprintf( fp, "%d %d %s %d %d\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     fwrite_flag( pObjIndex->value[2], buf),
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;
            
        case ITEM_WEAPON:
            fprintf( fp, "%s %d %d %s %s\n",
                     weapon_name(pObjIndex->value[0]),
                     pObjIndex->value[1],
                     pObjIndex->value[2],
                     attack_table[pObjIndex->value[3]].name,
                     fwrite_flag( pObjIndex->value[4], buf ) );
            break;
            
        case ITEM_ARMOR:
            fprintf( fp, "%d %d %d %d %d\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;
            
		case ITEM_SHEATH:
			fprintf( fp, "%d 0 0 0 0\n",
                     pObjIndex->value[0] );
			break;
				
            
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
	    fprintf( fp, "%d %s %s %s %s\n",
		     pObjIndex->value[0] > 0 ? // no negative numbers 
		     pObjIndex->value[0]
		     : 0,
		     pObjIndex->value[1] != -1 ?
		     pack_word(skill_table[pObjIndex->value[1]].name)
		     : "''",
		     pObjIndex->value[2] != -1 ?
		     pack_word(skill_table[pObjIndex->value[2]].name)
		     : "''",
		     pObjIndex->value[3] != -1 ?
		     pack_word(skill_table[pObjIndex->value[3]].name)
		     : "''",
		     pObjIndex->value[4] != -1 ?
		     pack_word(skill_table[pObjIndex->value[4]].name)
		     : "''");
	    break;

		case ITEM_PARCHMENT:
			fprintf( fp, "%d ", pObjIndex->value[0] );
			fprintf( fp, "%d ", pObjIndex->value[1] );
			fprintf( fp, "%d ", pObjIndex->value[2] );
			fprintf( fp, "%s ", language_safe_lookup_by_id(pObjIndex->value[3])->name);
			fprintf( fp, "%d ", pObjIndex->value[4] );
			break;

        case ITEM_PENDANT:
            fprintf( fp, "%d 0 0 0 0\n",
                     pObjIndex->value[0]);
            break;

		case ITEM_POULTICE:
        case ITEM_STAFF:
        case ITEM_WAND:
	    fprintf( fp, "%d ", pObjIndex->value[0] );
	    fprintf( fp, "%d ", pObjIndex->value[1] );
	    fprintf( fp, "%d %s 0\n",
		     pObjIndex->value[2],
		     pObjIndex->value[3] != -1 ?
		       pack_word(skill_table[pObjIndex->value[3]].name)
		       : 0 );
	    break;
    }
	
}

/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
 ****************************************************************************/
void save_object( FILE *fp, OBJ_INDEX_DATA *pObjIndex )
{
    char letter;
    AFFECT_DATA *pAf=NULL;
    EXTRA_DESCR_DATA *pEd;
    char buf[MSL];

    fprintf( fp, "#%d\n",	pObjIndex->vnum );
    fprintf( fp, "%s~\n",	pObjIndex->name );
    fprintf( fp, "%s~\n",	pObjIndex->short_descr );
    fprintf( fp, "%s~\n",	fix_string( pObjIndex->description ) );  
    fprintf( fp, "%s~\n",	pObjIndex->material );
    fprintf( fp, "%s ",		item_name(pObjIndex->item_type));
    fprintf( fp, "%s ",		fwrite_flag( pObjIndex->extra_flags,  buf ));
	fprintf( fp, "%s ",		fwrite_flag( pObjIndex->extra2_flags, buf ));
	if ( IS_TRAPPED( pObjIndex ))
	{
		fprintf( fp, "%ld %d %d %d ",
							(long)pObjIndex->trap_trig,
							pObjIndex->trap_dtype,
							pObjIndex->trap_charge,
							pObjIndex->trap_modifier );
	}
    fprintf( fp, "%s\n",	fwrite_flag( pObjIndex->wear_flags,   buf ));

/*
 *  Using fwrite_flag to write most values gives a strange
 *  looking area file, consider making a case for each
 *  item type later.
 */

	save_object_values( fp, pObjIndex );

    fprintf( fp, "%d ", pObjIndex->level );
    fprintf( fp, "%d ", pObjIndex->weight );
    fprintf( fp, "%d ", pObjIndex->cost );

         if ( pObjIndex->condition >  90 ) letter = 'P';
    else if ( pObjIndex->condition >  75 ) letter = 'G';
    else if ( pObjIndex->condition >  50 ) letter = 'A';
    else if ( pObjIndex->condition >  25 ) letter = 'W';
    else if ( pObjIndex->condition >  10 ) letter = 'D';
    else if ( pObjIndex->condition >   1 ) letter = 'B';
    else                                   letter = 'R';

    fprintf( fp, "%c\n", letter );

    fprintf( fp, "%d %d\n",
        pObjIndex->absolute_size,
        pObjIndex->relative_size);
 
//    fprintf( fp, "%s\n", fwrite_flag(pObjIndex->clss_restriction, buf) );

    for( pAf = pObjIndex->affected; pAf; pAf = pAf->next )
    {
//		if (pAf->where == WHERE_OBJEXTRA || pAf->bitvector == 0)
		if (pAf->where == WHERE_OBJEXTRA)
			fprintf( fp, "A %d %d\n",  reverse_translate_old_apply_number(pAf->location), pAf->modifier );
		else if (pAf->where == WHERE_OBJEXTRA2)
			fprintf( fp, "B %d %d\n",  reverse_translate_old_apply_number(pAf->location), pAf->modifier );
		else
		{
			fprintf( fp, "F " );
			
			switch(pAf->where)
			{
			case WHERE_AFFECTS:
				fprintf( fp, "A " );
				break;
			case WHERE_AFFECTS2:
				fprintf( fp, "B " );
				break;
			case WHERE_AFFECTS3:
				fprintf( fp, "C " );
				break;
			case WHERE_IMMUNE:
				fprintf( fp, "I " );
				break;
			case WHERE_RESIST:
				fprintf( fp, "R " );
				break;
			case WHERE_VULN:
				fprintf( fp, "V " );
				break;
			case WHERE_SKILLS:
				fprintf( fp, "K " );
				break;
			case WHERE_OBJECTSPELL:
				fprintf( fp, "Z " );
				break;
			default:
				bug("olc_save: Invalid Affect->where");
				break;
			}
			if ( pAf->where == WHERE_SKILLS ) {
				fprintf( fp, "'%s' %d\n", skill_table[pAf->type].name, pAf->modifier );
			} else if (pAf->where == WHERE_OBJECTSPELL){
				fprintf( fp, "'%s' %d %d %s\n", skill_table[pAf->type].name,
					pAf->level, pAf->duration, fwrite_flag( pAf->bitvector, buf ));
			}else{
				fprintf( fp, "%d %d %s\n", reverse_translate_old_apply_number(pAf->location), pAf->modifier,
					fwrite_flag( pAf->bitvector, buf ) );
			}
		}
    }

    // optional object stuff below here
	if(pObjIndex->class_allowances){
		//fwrite_wordflag(classnames_types, pObjIndex->class_allowances,"C ",fp);
		fwrite_wordflag(classnames_flags, pObjIndex->class_allowances,"_CA ",fp);
	}
	// save classgroup object restrictions
	{
		OBJRESTRICT_LIST_DATA *pr;
		for( pr = pObjIndex->restrict; pr; pr = pr->next )
		{
			fprintf( fp, "R %s %s %d 0\n",
				pr->classgroup->name,
				pr->affectprofile->name,
				pr->priority);
		}
	}

    for( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )
    {
        if (is_space(pEd->description[0]))
        {
            fprintf( fp, "E\n%s~\n.%s~\n", pEd->keyword,
             fix_string( pEd->description ) );
        }
        else
        {   
            fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
             fix_string( pEd->description ) );
        }
    }
    fprintf(fp, "\n");

    return;
}
 



/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_objects( FILE *fp, AREA_DATA *pArea )
{
    int i;
    OBJ_INDEX_DATA *pObj;

    fprintf( fp, "#OBJECTS\n" );

    for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    {
		if ( (pObj = get_obj_index( i )) ){
			if ( pArea->version<11){
				save_object( fp, pObj );
			}else{
				save_object_NAFF( fp, pObj );
			}
		}
    }

    fprintf( fp, "#0\n\n\n\n" );
    return;
}
 
/*****************************************************************************
 Name:		save_attunes
 Purpose:	Save #ATTUNE section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_attunes( FILE *fp, AREA_DATA *pArea )
{
	OBJ_INDEX_DATA *pObj;
	int i;
	char buf[MSL];

	fprintf( fp, "#ATTUNE\n" );

	for ( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
	{
		if (( pObj = get_obj_index(i)))
		{
			if ( IS_SET( pObj->attune_flags, ATTUNE_NEED_TO_USE ))
			{
				fprintf( fp, "O %d %s\n",
					pObj->vnum,
					fwrite_flag( pObj->attune_flags,  buf ));
			}
		}
	}
	fprintf( fp, "S\n\n\n\n" );
	return;
}

/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms( FILE *fp, AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXTRA_DESCR_DATA *pEd;
    EXIT_DATA *pExit;
    int iHash;
    int door;

    fprintf( fp, "#ROOMS\n" );
    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
        {
            if ( pRoomIndex->area == pArea )
            {
                fprintf( fp, "#%d\n",		pRoomIndex->vnum );
                fprintf( fp, "%s~\n",		pRoomIndex->name );
                fprintf( fp, "%s~\n",		fix_string( pRoomIndex->description ) );
		fprintf( fp, "0 " );
                fprintf( fp, "%d ",		pRoomIndex->room_flags );
                fprintf( fp, "%d\n",		pRoomIndex->sector_type );

                for ( pEd = pRoomIndex->extra_descr; pEd;
                      pEd = pEd->next )
                {
                    fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
                                                  fix_string( pEd->description ) );
                }
                for( door = 0; door < MAX_DIR; door++ )	// I hate this
                {
                    if ( ( pExit = pRoomIndex->exit[door] )
                          && pExit->u1.to_room )
                    {

                        fprintf( fp, "D%d\n",      door );
                        fprintf( fp, "%s~\n",      fix_string( pExit->description ) );
                        fprintf( fp, "%s~\n",      pExit->keyword );
                        fprintf( fp, "%s %d %d\n", // locks
                                flags_print(pExit->rs_flags),
                                                   pExit->key,
                                                   pExit->u1.to_room->vnum );
                    }
                }
		if (pRoomIndex->mana_rate != 100 || pRoomIndex->heal_rate != 100)
		 fprintf ( fp, "M %d H %d\n",pRoomIndex->mana_rate,
		                             pRoomIndex->heal_rate);
		if (pRoomIndex->clan){
			fprintf ( fp, "C %s~\n" , pRoomIndex->clan->savename());
		}
		 			     
		if (pRoomIndex->owner && str_cmp(pRoomIndex->owner,""))
		 fprintf ( fp, "O %s~\n" , pRoomIndex->owner );

		if (pRoomIndex->msp_sound)
		 fprintf ( fp, "X %s~\n" , pRoomIndex->msp_sound );

        fprintf( fp, "S\n\n" );
            }
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
    return;
}



/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_specials( FILE *fp, AREA_DATA *pArea )
{
    int iHash;
    MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
    
    fprintf( fp, "#SPECIALS\n" );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->spec_fun )
            {
#if defined( VERBOSE )
                fprintf( fp, "M %d %s Load to: %s\n", pMobIndex->vnum,
                                                      spec_name( pMobIndex->spec_fun ),
                                                      pMobIndex->short_descr );
#else
                fprintf( fp, "M %d %s\n", pMobIndex->vnum,
                              spec_name( pMobIndex->spec_fun ) );
#endif
            }
        }

		// Go through the objects and save their obj_spec_funs too :)
		for( pObjIndex = obj_index_hash[iHash]; pObjIndex; pObjIndex = pObjIndex->next )
        {
			if ( pObjIndex && pObjIndex->area == pArea && pObjIndex->ospec_fun )
			{
				fprintf( fp, "O %d %s Load to: %s\n", pObjIndex->vnum,
													  ospec_name( pObjIndex->ospec_fun ),
													  pObjIndex->short_descr );
			}
		}
    }

    fprintf( fp, "S\n\n\n\n" );
    return;
}

/*****************************************************************************
 Name:		save_gamble
 Purpose:	Save #GAMBLE section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_gamble( FILE *fp, AREA_DATA *pArea )
{
	int iHash;
	MOB_INDEX_DATA *pMobIndex;
	
	fprintf( fp, "#GAMBLE\n" );
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
		{
			if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->gamble_fun )
			{
				fprintf( fp, "M %d %s\n", pMobIndex->vnum,
					gamble_name( pMobIndex->gamble_fun ));
			}
		}
	}
	fprintf( fp, "S\n\n\n\n" );
	return;
}

/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_resets( FILE *fp, AREA_DATA *pArea )
{
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pLastMob = NULL;
    OBJ_INDEX_DATA *pLastObj;
    ROOM_INDEX_DATA *pRoom;
    int iHash;

    fprintf( fp, "#RESETS2\n" );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
        {
            if ( pRoom->area == pArea )
			{
				for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
				{
					switch ( pReset->command )
					{
					default:
						bugf( "Save_resets: bad command %c.", pReset->command );
						break;
						
					case 'M':
						pLastMob = get_mob_index( pReset->arg1 );
						fprintf( fp, "M 0 %d %d %d %d\n", 
							pReset->arg1,
							pReset->arg2,
							pReset->arg3,
							pReset->arg4 );
						break;
						
					case 'O':
						pLastObj = get_obj_index( pReset->arg1 );
						pRoom = get_room_index( pReset->arg3 );
						fprintf( fp, "O 0 %d %d %d\n", 
							pReset->arg1, pReset->arg2,
							pReset->arg3 );
						break;
						
					case 'P':
						pLastObj = get_obj_index( pReset->arg1 );
						fprintf( fp, "P 0 %d %d %d %d\n", 
							pReset->arg1,
							pReset->arg2,
							pReset->arg3,
							pReset->arg4 );
						break;
						
					case 'G':
						fprintf( fp, "G 0 %d %d\n", pReset->arg1, pReset->arg2);
						if ( !pLastMob )
						{
							bugf("Save_resets: !NO_MOB! in [%s]", pArea->file_name );
						}
						break;
						
					case 'E':
						// reset_version 1 format
						//fprintf( fp, "E 0 %d %d %d\n",
						//	pReset->arg1, pReset->arg2, pReset->arg3 );
						fprintf( fp, "E 0 %d %d",pReset->arg1, pReset->arg2);
						fwrite_wordflag( wear_location_types, pReset->arg3, "", fp);
						if ( !pLastMob )
						{
							bugf( "Save_resets: !NO_MOB! in [%s]", pArea->file_name );
						}
						break;
						
					case 'D':
						break;
						
					case 'R':
						pRoom = get_room_index( pReset->arg1 );
						fprintf( fp, "R 0 %d %d\n", 
							pReset->arg1,
							pReset->arg2 );
						break;
					}
				}
			}	/* End if correct area */
		}	/* End for pRoom */
    }	/* End for iHash */
    fprintf( fp, "S\n\n\n\n" );
    return;
}


/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_shops( FILE *fp, AREA_DATA *pArea )
{
    SHOP_DATA *pShopIndex;
    MOB_INDEX_DATA *pMobIndex;
    int iTrade;
    int iHash;
    
    fprintf( fp, "#SHOPS\n" );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop )
            {
                pShopIndex = pMobIndex->pShop;

                fprintf( fp, "%d ", pShopIndex->keeper );
                for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
                {
                    if ( pShopIndex->buy_type[iTrade] != 0 )
                    {
                       fprintf( fp, "%d ", pShopIndex->buy_type[iTrade] );
                    }
                    else
                       fprintf( fp, "0 ");
                }
                fprintf( fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell );
                fprintf( fp, "%d %d\n", pShopIndex->open_hour, pShopIndex->close_hour );
            }
        }
    }

    fprintf( fp, "0\n\n\n\n" );
    return;
}

/**************************************************************************/
extern bool gio_abort_fwrite_wordflag_with_undefined_flags; // gio code

/**************************************************************************/
void fwrite_areaecho(area_echo_data *pAe, FILE *fp)
{
	fprintf(fp, "AreaEcho %2d %2d %3d %s~\n", pAe->firsthour, pAe->lasthour,
			pAe->percentage,pack_string(pAe->echotext));
}
/**************************************************************************/
void fwrite_areaecho_recursive(area_echo_data *pAe, FILE *fp)
{
	if(!pAe)
	{ 
		return;
	}
	fwrite_areaecho_recursive(pAe->next, fp);
	fwrite_areaecho(pAe, fp);
}

/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area( AREA_DATA *pArea )
{
	pArea->version = 11; // force all areas to use NAFF
//	pArea->version = 6;  // force all areas to save in old format

    FILE *fp;
    fclose( fpReserve );
	char newfilename[MIL];
	sprintf(newfilename, "%s%s.save", BACKUP_AREA_DIR, pArea->file_name);
	logf("save_area(): saving %s...", pArea->file_name);
    if ( !( fp = fopen( newfilename, "w" ) ) )
    {
        bugf("save_area(): fopen '%s' for write - error %d (%s)",
			newfilename, errno, strerror( errno));
		exit_error( 1 , "save_area", "fopen for write error");
    }

	if(IS_SET(pArea->olc_flags, OLCAREA_IGNORE_UNDEFINED_FLAGS)){
		gio_abort_fwrite_wordflag_with_undefined_flags=false;
	}else{
		gio_abort_fwrite_wordflag_with_undefined_flags=true;
	}

	if(pArea->version>=11){
		fprintf( fp, "#DAWNAREADATA\n" );
	}else{
		fprintf( fp, "#AREADATA\n" );
	}
    fprintf( fp, "Version     %d\n",	pArea->version);
	fprintf( fp, "FromMUD     %s~\n",	MUD_NAME); // could aid in importing from other dawn based muds in future
	if(pArea->version>=11){
		fprintf( fp, "*parent_codebase %s\n", DAWN_RELEASE_VERSION);
	}
    fprintf( fp, "Name        %s~\n",        pArea->name );
	
	if(pArea->version>10){		
		fprintf( fp, "ShortName   %s~\n",	pack_string(pArea->short_name));
	}
	
    fprintf( fp, "Builders    %s~\n",    fix_string(ltrim_string(rtrim_string(pArea->builders ))) );	
	if(!IS_NULLSTR(pArea->credits)){
		fprintf( fp, "Credits     %s~\n",    fix_string(ltrim_string(rtrim_string(pArea->credits))) );
	}

	// builder restrictions - write to the file
	for (int br=0;br<MAX_BUILD_RESTRICTS;br++)
	{
		if(!IS_NULLSTR(pArea->build_restricts[br])){
			fprintf( fp, "build_restricts '%s' %s~\n",
				flag_string(buildrestrict_types, br),
				fix_string( pArea->build_restricts[br]) );
		}
	}

	fprintf( fp, "VNUMs       %d %d\n",     pArea->min_vnum, pArea->max_vnum );
    if (!IS_NULLSTR(pArea->lcomment))
    {
        fprintf( fp, "LComment    %s\n", fix_string( pArea->lcomment) );
    }
	if(pArea->version<11){
		char buf[MIL];
	    fprintf( fp, "AreaFlags   %s\n", fwrite_flag(pArea->area_flags, buf) );
	}else{
		fwrite_wordflag( area_flags, pArea->area_flags, "AFlags  ", fp );
	}
    if (pArea->low_level>-1)
        fprintf( fp, "LRange      %d %d\n",    pArea->low_level, pArea->high_level );
    fprintf( fp, "Security    %d\n",	pArea->security );
    fprintf( fp, "Colour      %s~\n",	pArea->colour);
	fprintf( fp, "colourcode  %c\n",	pArea->colourcode);
    fprintf( fp, "MapScale    %d\n",	pArea->mapscale);	
    fprintf( fp, "MapLevel    %d\n",	pArea->maplevel);	
    fprintf( fp, "Vnum_offset %d\n",	pArea->vnum_offset);
	if(pArea->continent){
		fprintf( fp, "Continent   %s~\n",	pArea->continent->name );
	}

	fwrite_areaecho_recursive(pArea->echoes, fp);
	if(pArea->version>=11){
		fprintf( fp, "*LastSaved  %.24s~\n",	ctime(&current_time));
	}
    fprintf( fp, "End\n\n\n\n" );

    save_mobiles(	fp, pArea );
    save_objects(	fp, pArea );
	if ( pArea->version<11){
	    save_rooms(		fp, pArea );
	}else{
		save_rooms_NAFF( fp, pArea );
	}
    save_specials(	fp, pArea );
    save_resets(	fp, pArea );
	if ( pArea->version>=11){
		save_shops_NAFF(fp, pArea );
		save_mudprogs_NAFF(	fp, pArea );
	}else{
	    save_shops(		fp, pArea );
		save_mudprogs(	fp, pArea );
		save_gamble(	fp, pArea );	// NAFF saves gambling in with mob
		save_attunes(	fp, pArea );	// NAFF saves attunes in with object
	}
    
    fprintf( fp, "#$\n" );

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

	// rename area/*.are to bak_area/*.are.old, 
	// then bak_area/*.are.save to area/*.are
	{
		char old_areafilename[MIL];
		sprintf(old_areafilename, "%s%s.old", BACKUP_AREA_DIR, pArea->file_name);
		char areafilename[MIL];
		sprintf(areafilename, "%s%s", AREA_DIR, pArea->file_name);
#ifdef WIN32
		unlink(old_areafilename);
#endif
		if(file_exists(areafilename)){
			if(rename(areafilename,old_areafilename)!=0){
				bugf("Error %d occurred renaming '%s' to '%s'!.. exiting to avoid area file corruption.", 
					errno, areafilename, old_areafilename);
				exit_error( 1 , "save_area", "error renaming old filename");
			}
		}
		if(rename(newfilename, areafilename)!=0){
			bugf("Error %d occurred renaming '%s' to '%s'!.. exiting to avoid area file corruption.", 
				errno, newfilename, areafilename);
			exit_error( 1 , "save_area", "error renaming new filename");
		}		
	}
	logf("save_area(): save complete.");

	if(IS_SET(pArea->olc_flags, OLCAREA_IGNORE_UNDEFINED_FLAGS)){
		gio_abort_fwrite_wordflag_with_undefined_flags=true;
	}
    return;
}


/*****************************************************************************
 Name:		save_world_onefile
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_world_onefile( void )
{
    AREA_DATA *pArea;

	pArea               =   new_area();
	pArea->min_vnum     =   0;
	pArea->max_vnum     =   32700;
	SET_BIT( pArea->olc_flags, OLCAREA_ADDED );

	pArea->file_name= str_dup("onefile.are");
    pArea->name= str_dup("allinone");
	pArea->security =9;

	save_area(pArea);
	return;
}


/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave( char_data *ch, char *argument )
{
    char arg1[MIL];
    AREA_DATA *pArea;
    FILE *fp;
    int value;

    fp = NULL;
	
	if (IS_NPC(ch) || ch->pcdata->security<2)
	{
		ch->println("You must have an olc security 2 or higher to use this command.");
		return;
	}

    if ( !ch ) // Do an autosave 
    {
		log_string("Doing autosave of areas.");
		save_area_list();
		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			save_area( pArea );
			REMOVE_BIT( pArea->olc_flags, OLCAREA_CHANGED );
		}
		log_string("Autosave completed.");
		return;
    } // end of autosave

    smash_tilde( argument );
    strcpy( arg1, argument );
    if ( arg1[0] == '\0' )
    {
		ch->println("Syntax:");
		ch->println("  asave <vnum>   - saves a particular area");
		ch->println("  asave list     - saves the area.lst file");
		ch->println("  asave area     - saves the area being edited");
		ch->println("  asave .        - saves all changed zones");
		ch->println("  asave changed  - saves all changed zones");
		ch->println("  asave world    - saves the world! (db dump)");
		ch->println("  asave onefile  - saves the whole world into one file! (db dump)");
        return;
    }

	// Snarf the value (which need not be numeric). 
	value = atoi( arg1 );
	if ( !( pArea = get_area_data( value ) ) && is_number( arg1 ) )
	{
		ch->println("That area does not exist.");
		return;
	}
    
	// Save area of given vnum. 
    // ------------------------ 
    if ( is_number( arg1 ) )
    {
		if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_OTHER ) )
		{
			ch->println("You are not a builder for this area.");
			return;
		}
		save_area_list();
		save_area( pArea );
		ch->printlnf("Saved area %d (%s)", pArea->vnum, pArea->name);
		return;
    }

    // Save the world, only authorized areas. //
    // -------------------------------------- // 
    if ( !str_cmp( "world", arg1 ) )
    {
		if(!IS_ADMIN(ch)){
			ch->println("Only admin can asave the world.");
			return;
		}
		if(!HAS_SECURITY(ch,9)){
			ch->println("You require security 9 to asave the world.");
			return;
		}
		save_area_list();
		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			// Builder must be assigned this area. 
			if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_OTHER ) )
			continue;	  

			save_area( pArea );
			REMOVE_BIT( pArea->olc_flags, OLCAREA_CHANGED );
		}
		ch->println( "You saved all area files.");
		ch->wrapln( "`RNOTE:`x It is strongly recommended that you use `=Casave .`x to save the area you are working on! "
			"- asave world forces the mud to resave EVERY area and usually lags the game.\r\n");
		return;
    }

	// Save the world into one file. //
    // ----------------------------- // 

    if ( !str_cmp( "onefile", arg1 ) )
    {
		save_world_onefile();
		ch->println("You saved the world into onefile.are.");
		return;
    }


    /* Save changed areas, only authorized areas. */
    /* ------------------------------------------ */

    if ( !str_cmp( "changed", arg1 ) || !str_cmp( ".", arg1 ) )
    {
		char buf[MIL];
		
		save_area_list();
		ch->println("Saved zones:");
		sprintf( buf, "None.\r\n" );
		
		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			// Builder must be assigned this area. 
			if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_OTHER ) )
				continue;
			// Save changed areas
			if ( IS_SET(pArea->olc_flags, OLCAREA_CHANGED) )
			{
				save_area( pArea );
				ch->printlnf("%24s - '%s'", pArea->name, pArea->file_name );
				buf[0] = '\0';
				REMOVE_BIT( pArea->olc_flags, OLCAREA_CHANGED );
			}
		}
		
		if ( !str_cmp( buf, "None.\r\n" ) ){
			ch->print( buf);
		}
		if(!str_cmp( "changed", arg1 )){
			ch->println("Note: you can use 'asave .' as shorthand for 'asave changed'");
		}
		return;
	}

    /* Save the area.lst file. */
    /* ----------------------- */
    if ( !str_cmp( arg1, "list" ) )
	{
		save_area_list();
		return;
    }

    /* Save area being edited, if authorized. */
    /* -------------------------------------- */
    if ( !str_cmp( arg1, "area" ) )
	{
		// Is character currently editing
		if ( ch->desc->editor == 0 )
		{
			ch->println("You are not editing an area, therefore an area vnum is required.");
			return;
		}
		// Find the area to save
		switch (ch->desc->editor)
		{
		case ED_AREA:	pArea = (AREA_DATA *)ch->desc->pEdit;	break;
		case ED_ROOM:	pArea = ch->in_room->area;				break;
		case ED_OBJECT:
			pArea = ((OBJ_INDEX_DATA *)ch->desc->pEdit )->area;
			break;
		case ED_MOBILE:
			pArea = ( (MOB_INDEX_DATA *)ch->desc->pEdit )->area;
			break;
		default:
			pArea = ch->in_room->area;
			break;
		}
		
		if ( !IS_BUILDER( ch, pArea, BUILDRESTRICT_OTHER ) )
		{
			ch->println("You are not a builder for this area.");
			return;
		}


		save_area_list();
		save_area( pArea );
		REMOVE_BIT( pArea->olc_flags, OLCAREA_CHANGED );
		ch->println("Area saved.");
		return;
	}

    /* Show correct syntax. */
    /* -------------------- */
    do_asave( ch, "" );
    return;
}

/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
// prototype
void save_clan_db( void );
void laston_save(char_data *);
void save_intro_database();
/**************************************************************************/
void reboot_autosave( char_data *ch)
{
	char buf [MSL];
    AREA_DATA *pArea;

	log_string("Starting reboot_autosave():");
	
	// inform the person doing the reboot
	if (ch && ch->desc && ch->desc->connected_socket){
		ch->desc->write( "starting auto changed area save.\r\n",0);
	}

	// areas
	{
		bool none= true;
		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			// Save changed areas.
			if ( IS_SET(pArea->olc_flags, OLCAREA_CHANGED) )
			{
				save_area( pArea );
				sprintf( buf, "**%s -> %s", pArea->name, pArea->file_name );
				log_string( buf);
				if (ch && ch->desc && ch->desc->connected_socket)	
				{		
					ch->desc->write(buf,0);
					ch->desc->write("\r\n",0);
				}
				REMOVE_BIT( pArea->olc_flags, OLCAREA_CHANGED );
				none= false;
			}		
		}
		if (none)
		{
			log_string("No areas needed saving.");		
			if (ch && ch->desc)	
				ch->desc->write("No areas needed saving.\r\n",0);
		}
		else
		{
			save_area_list();
		}
		log_string("reboot area autosave completed.");
	}

	// room invite lists
	{
		bool none= true;
		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			// Save changed room invite lists.
			if ( IS_SET(pArea->olc_flags, OLCAREA_INVITELISTCHANGED) )
			{
				save_area_roominvitelist( pArea );
				sprintf( buf, "**%s room invite list.", pArea->name);
				log_string( buf);
				if (ch && ch->desc && ch->desc->connected_socket)	
				{		
					ch->desc->write(buf,0);
					ch->desc->write("\r\n",0);
				}
				REMOVE_BIT( pArea->olc_flags, OLCAREA_INVITELISTCHANGED);
				none= false;
			}		
		}
		if(none)
		{
			if(GAMESETTING4(GAMESET4_ROOM_INVITES_DISABLED)){
				log_string("No room invite lists needed saving - room invites are disabled.");
			}else{
				log_string("No room invite lists needed saving.");
			}
			if (ch && ch->desc)	
				ch->desc->write("No room invite lists needed saving.\r\n",0);
		}
		log_string("reboot area autosave completed.");
	}
	
	if (IS_SET(SKILL_TABLE_FLAGS,SEDIT_CHANGED)){
		log_string("Autosaving " SKILLS_FILE);
		do_write_skills(NULL,"");
		REMOVE_BIT(SKILL_TABLE_FLAGS,SEDIT_CHANGED);
		log_string("Skills autosave complete.");
	}

	if (IS_SET(CLASS_TABLE_FLAGS,CLASSEDIT_CHANGED)){
		log_string("Autosaving " CLASSES_LIST);
		do_write_classes(NULL,"");
		REMOVE_BIT(CLASS_TABLE_FLAGS,CLASSEDIT_CHANGED);
		log_string("Class table autosave complete.");
	}

	// Save Command Table if it was changed
	if (IS_SET(COM_TABLE_FLAGS,COMEDIT_CHANGED)){
		log_string("Autosaving " COMMANDS_FILE);
		do_write_commandtable(NULL,"");
		REMOVE_BIT(COM_TABLE_FLAGS,COMEDIT_CHANGED);
		log_string("Autosave complete.");
	}

	// Save Deity Table if it was changed
	if (IS_SET(DEITY_FLAGS,DEDIT_CHANGED)){
		log_string("Autosaving " DEITY_FILE);
		do_savedeities(NULL,"");
		REMOVE_BIT(DEITY_FLAGS,DEDIT_CHANGED);
		log_string("Autosave complete.");
	}

	// Save Herb Table if it was changed
	if ( IS_SET( HERB_FLAGS,DEDIT_CHANGED)){	// using dedit_changed cause I'm too lazy to make an herb_changed :)
		log_string("Autosaving " HERB_FILE );
		do_saveherbs(NULL,"");
		REMOVE_BIT( HERB_FLAGS, DEDIT_CHANGED);
		log_string("Autosave complete.");
	}

	if (IS_SET(QUEST_TABLE_FLAGS,QEDIT_CHANGED)){
		log_string("Autosaving " QUEST_FILE);
		save_quest_db();
		REMOVE_BIT(QUEST_TABLE_FLAGS,QEDIT_CHANGED);
		log_string("Autosave complete.");
	}

	if (IS_SET(MIX_FLAGS,DEDIT_CHANGED)){
		log_string("Autosaving " MIX_FILE);
		save_mix_db();
		REMOVE_BIT(MIX_FLAGS,DEDIT_CHANGED);
		log_string("Autosave complete.");
	}

	if (IS_SET(CLAN_FLAGS,DEDIT_CHANGED)){
		log_string("Autosaving " CLAN_FILE);
		save_clan_db();		
		REMOVE_BIT(CLAN_FLAGS,DEDIT_CHANGED);
		log_string("Autosave complete.");
	}

	if (IS_SET(SKILLGROUPEDIT_FLAGS, DEDIT_CHANGED)){
		log_string("Autosaving " SKILLGROUPS_FILE);
		do_write_skillgroups(NULL,"");
		REMOVE_BIT(SKILLGROUPEDIT_FLAGS,DEDIT_CHANGED);
		log_string("Autosave complete.");
	}

	if (LANGUAGE_NEEDS_SAVING){
		log_string("Autosaving languages");
		do_write_languages(NULL,"");
		LANGUAGE_NEEDS_SAVING=false;
		log_string("Autosave complete.");
	}

	do_hsave(NULL,"");

	lockers->lockers_save_db();

	// save laston data
	laston_save(NULL);

	do_save_corpses(ch, "");

	// save intro database stuff
	save_intro_database(); 
}
/**************************************************************************/
void do_hsave( char_data *ch, char *argument)
{
	helpfile_data *pHelpFD;
	bool nonesaved= true;

	// if it isn't the code doing it
    if (ch){
		if (!HAS_SECURITY(ch,1)){
			ch->println("The hsave command is an olc command, you don't have olc permissions.");
			return;
		}
		
		if (!HAS_SECURITY(ch,7)){
			ch->println("security 7 required for hsave, you don't have olc permissions.");
			return;
		}
	}

    for ( pHelpFD = helpfile_first; pHelpFD; pHelpFD = pHelpFD->next )
    {  
		if (IS_SET(pHelpFD->flags, HELPFILE_CHANGED) || !str_cmp("all", argument))
		{
			if (ch)
			{
				ch->printlnf("Saving helpfile: %s", pHelpFD->file_name);
			}
			nonesaved = false;
			logf("Saving help: %s", pHelpFD->file_name);
			//save_helpfile(pHelpFD);
			save_helpfile_NAFF(pHelpFD);
			REMOVE_BIT(pHelpFD->flags, HELPFILE_CHANGED);
		}
    }

	if (ch && nonesaved)
	{
		ch->println("No helpfiles needed saving.");
	}
	if(!nonesaved){
		logf("Helpfile saving completed.");
	}
    return;    
}


/**************************************************************************/
// Name:		save_helpfile
// Purpose:	Save a helpfile, 
// Called by:	do_hsave(olc_save.c).
/**************************************************************************/
void save_helpfile( helpfile_data *pHelpfile )
{
    FILE *fp;
    char buf[MSL];

    fclose( fpReserve );
    sprintf (buf, "%s%s", HELP_DIR, pHelpfile->file_name);
    if ( !( fp = fopen( buf , "w" ) ) ){
        bugf("save_helpfile(): fopen '%s' for write - error %d (%s)",
			buf, errno, strerror( errno));
		exit_error( 1 , "save_helpfile", "fopen for write error");
    }

    fprintf( fp, "#HELPS\n\n" );
    save_helpentries( fp, pHelpfile );
    fprintf( fp, "#$\n" );

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/**************************************************************************/
void save_helpentries( FILE *fp, helpfile_data *pHelpfile )
{
    help_data *pHelp;

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
		if (pHelp->helpfile==pHelpfile && !IS_SET(pHelp->flags, HELP_REMOVEHELP))
		{
			fprintf(fp, "-2 2~\n" );
			fprintf(fp, "%d %s~\n", pHelp->level, fix_string(pHelp->keyword));

			fwrite_wordflag( help_flags, pHelp->flags, " ", fp );

			if ( is_space(pHelp->text[0]) )
			{
				fprintf(fp, ".");
			}

			hide_tilde (pHelp->text);
			
			// add a little . to maintain formating in help entries if required
			if (is_space(pHelp->text[0]))
			{
				fprintf(fp, ".");
			}

			fprintf(fp, "%s~\n\n\n", fix_string(pHelp->text));
			show_tilde (pHelp->text);
		}
	}

	//  mark the end of the helps section
	fprintf( fp, "0 $~\n");
    return;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
