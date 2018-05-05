/**************************************************************************/
// db2.cpp - 
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

#include "include.h" // dawn standard includes
#include "areas.h"
#include "db.h"

extern int flag_lookup args((const char *name, const struct flag_type *flag_table));
struct	social_old_type	social_table[MAX_SOCIALS];
int		social_count;

extern NOTE_DATA *anote_list;

/**************************************************************************/
// read in a socials file - old format
void oldload_socials( FILE *fp)
{
	social_count=0;
    for ( ; ; ) 
    {
		if(social_count+5>MAX_SOCIALS){
			logf("oldload_socials()... Too many socials read in from file... "
				"you should read in the ones you want, use importsocials to convert them "
				"to the new format, then remove the socials from the helpfile list.");
			exit_error( 1 , "oldload_socials", "Too many socials");
			return;
		}
    	struct social_old_type social;
    	char *temp;
        // clear social
		social.char_no_arg = NULL;
		social.others_no_arg = NULL;
		social.char_found = NULL;
		social.others_found = NULL;
		social.vict_found = NULL; 
		social.char_not_found = NULL;
		social.char_auto = NULL;
		social.others_auto = NULL;

	    temp = fread_word(fp);
		if (!strcmp(temp,"#0")){
			break; // done
		}
#if defined(social_debug) 
		else{
			logf("%s",temp);
		}
#endif

    	strcpy(social.name,temp);
    	fread_to_eol(fp);

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.char_no_arg = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue; 
	}
        else
	    social.char_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_no_arg = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
		  }
        else
	    social.others_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
       	else
	    social.char_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
		  }
        else
	    social.others_found = temp; 

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.vict_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.vict_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_not_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
		  }
        else
	    social.char_not_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.char_auto = temp;
         
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
		  }
        else
	    social.others_auto = temp; 
	
	social_table[social_count] = social;
    	social_count++;
   }

   logf("autonoting that old style social file was read in.");
   autonote(NOTE_SNOTE, "p_anote()", "remove oldstyle socials loading from files listed in helplist.txt", "admin", 
	   "Old style socials were loaded in from helplist.txt... you can import them using 'importsocials' "
	   "Once you have dealt with the issue of socials, remove the filename loading socials from helplist.txt... "
	   "You can usually find this helpfile(s) by typing 'grep SOCIALS *' in the help directory.", true);
   anote_list=NULL;

   return;
}

/**************************************************************************/
void load_mobiles( FILE *fp, int version )
{
	MOB_INDEX_DATA *pMobIndex;
	char *racename;
	
	if ( !area_last ){
		bug("Load_mobiles: no #AREA seen yet.");
		exit_error( 1 , "load_mobiles", "no #AREA seen yet");		
	}
	
	for ( ; ; )
	{
		vn_int vnum;
		char letter;
		int iHash;
		
		letter = fread_letter( fp );
		if ( letter != '#' )
		{
			bug("Load_mobiles: # not found.");
			exit_error( 1 , "load_mobiles", "# not found");
		}
		
		vnum = fread_number( fp );
		if ( vnum == 0 )
			break;
		vnum+= area_last->vnum_offset; 
		
		// make sure we don't have a duplicated mob vnum
		fBootDb = false;  // this is turned off to prevent logging of unfound vnums
		if ( get_mob_index( vnum ) ){
			char *aname="an unknown area.";
			if( get_mob_index( vnum )->area 
				&& get_mob_index( vnum )->area->file_name){
				aname=get_mob_index( vnum )->area->file_name;
			}
			bugf( "load_mobiles: vnum %d duplicated.", vnum );
			logf("with %s (%s) from %s",
				get_mob_index( vnum )->short_descr,
				get_mob_index( vnum )->player_name,
				aname);
			exit_error( 1 , "load_mobiles", "duplicate vnum");
		}
		fBootDb = true;
		
		last_vnum = vnum; // backup the last vnum for gdb use 
		
		pMobIndex= (MOB_INDEX_DATA *)alloc_perm( sizeof(*pMobIndex) );
		pMobIndex->vnum	= vnum;
		pMobIndex->area	= area_last;
		pMobIndex->xp_mod= 100; // default value		

		// check mob vnum fits in areas vnum range 
		if ( vnum < pMobIndex->area->min_vnum + pMobIndex->area->vnum_offset)
		{
			bugf("Mob with Vnum %d is less than area %s <%s> vnum %d!",
				vnum,
				pMobIndex->area->name,
				pMobIndex->area->file_name,
				pMobIndex->area->min_vnum
				);
		}
		if ( vnum > pMobIndex->area->max_vnum + pMobIndex->area->vnum_offset)
		{
			bugf("Mob with Vnum %d is greater than area %s <%s> vnum %d!",
				vnum,
				pMobIndex->area->name,
				pMobIndex->area->file_name,
				pMobIndex->area->max_vnum
				);
		}
		
		pMobIndex->player_name			= fread_string( fp );
		pMobIndex->short_descr			= fread_string( fp );
		pMobIndex->long_descr			= trim_trailing_carriage_return_line_feed(fread_string( fp ));
		pMobIndex->description			= fread_string( fp );

		if (str_len(pMobIndex->description)>MSL-4)
		{
			bugf("load_mobiles: Description of mob %d is %d characters!!!"
				"(more than %d)",
				(int)last_vnum, str_len(pMobIndex->description), MSL-4);
			exit_error( 1 , "load_mobiles", "description too long");
		}
		pMobIndex->long_descr[0]		= UPPER(pMobIndex->long_descr[0]);
		pMobIndex->description[0]		= UPPER(pMobIndex->description[0]);

		racename						= fread_string( fp );
		pMobIndex->race					= race_lookup(racename);
		if(pMobIndex->race == -1)
		{
			logf("Mob with Vnum %d has an unrecognised race '%s', "
				"dynamically generating a race.", vnum, racename);
			pMobIndex->race= race_generate_race_adding_to_race_table(racename);
		}
		free_string(racename);

		// count all the usage stats while loading
		race_table[pMobIndex->race]->inuse++;
		total_npcracescount++;
		if (race_table[pMobIndex->race]->lastarea!=area_last){
			race_table[pMobIndex->race]->areacount++;
			total_npcareacount++;
		}
		race_table[pMobIndex->race]->lastarea=area_last;
		
		pMobIndex->act					= fread_flag( fp ) | ACT_IS_NPC
										| race_table[pMobIndex->race]->act;

		if(AREA_IMPORT_FORMAT(AIF_FORMAT2)){
			areaimport_mobile_affects_format2(fp, version, pMobIndex);
		}else{
			// AIF_STOCK - the default
			areaimport_mobile_affects_stock(fp, version, pMobIndex);
		}

		pMobIndex->pShop				= NULL;

		// get alliance and tendency
		switch(version){
			case 1:
				{
					int talign					= fread_number( fp );
					pMobIndex->tendency			= number_range(1,7)-4;
					pMobIndex->alliance			= (3*talign)/1000;
					pMobIndex->group			= fread_number( fp );
				}
				break;

			case 2:
				pMobIndex->tendency         = fread_number( fp );
				pMobIndex->alliance         = fread_number( fp );
				break;

			default:
				pMobIndex->tendency 			= fread_number( fp );
				pMobIndex->alliance 			= fread_number( fp );
				pMobIndex->xp_mod				= fread_number( fp );
				if (pMobIndex->xp_mod<0 || pMobIndex->xp_mod>500){
					bugf( "Load_mobiles: mob vnum %d, XP_MOD is %d (>500 or <0!)", 
						vnum, pMobIndex->xp_mod);
				}		
				break;
		}

		pMobIndex->level				= fread_number( fp );
		pMobIndex->hitroll				= fread_number( fp );
		
		/* read hit dice */
		pMobIndex->hit[DICE_NUMBER] 	= fread_number( fp );
		/* 'd'			*/				  fread_letter( fp );
		pMobIndex->hit[DICE_TYPE]		= fread_number( fp );
		/* '+'			*/				  fread_letter( fp );
		pMobIndex->hit[DICE_BONUS]		= fread_number( fp );
		
		/* read mana dice */
		pMobIndex->mana[DICE_NUMBER]	= fread_number( fp );
		/* 'd'			*/				  fread_letter( fp );
		pMobIndex->mana[DICE_TYPE]		= fread_number( fp );
		/* '+'			*/				  fread_letter( fp );
		pMobIndex->mana[DICE_BONUS] 	= fread_number( fp );

		/* read damage dice */
		pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
		/* 'd'			*/				  fread_letter( fp );
		pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
		/* '+'			*/				  fread_letter( fp );
		pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
		pMobIndex->dam_type 			= attack_lookup(fread_word(fp));
		
		// read armor class 
		if(version<6){
			pMobIndex->ac[AC_PIERCE]		= fread_number( fp ) * 10;
			pMobIndex->ac[AC_BASH]			= fread_number( fp ) * 10;
			pMobIndex->ac[AC_SLASH] 		= fread_number( fp ) * 10;
			pMobIndex->ac[AC_EXOTIC]		= fread_number( fp ) * 10;
		}else{
			pMobIndex->ac[AC_PIERCE]		= fread_number( fp );
			pMobIndex->ac[AC_BASH]			= fread_number( fp );
			pMobIndex->ac[AC_SLASH] 		= fread_number( fp );
			pMobIndex->ac[AC_EXOTIC]		= fread_number( fp );
		}
		
		// read flags and add in data from the race table 
		pMobIndex->off_flags			= fread_flag( fp )
										| race_table[pMobIndex->race]->off;
		pMobIndex->imm_flags			= fread_flag( fp )
										| race_table[pMobIndex->race]->imm;
		pMobIndex->res_flags			= fread_flag( fp )
										| race_table[pMobIndex->race]->res;
		pMobIndex->vuln_flags			= fread_flag( fp )
										| race_table[pMobIndex->race]->vuln;
		
		// vital statistics 
		pMobIndex->start_pos			= position_lookup(fread_word(fp));
		if (pMobIndex->start_pos==-1){
			bugf("load_mobiles: mob %d (%s) start position is invalid!",
				(int)last_vnum, pMobIndex->short_descr);
			exit_error( 1 , "load_mobiles", "invalid start position");
		}
		
		pMobIndex->default_pos			= position_lookup(fread_word(fp));
		if (pMobIndex->default_pos==-1){
			bugf("load_mobiles: mob %d (%s) default position is invalid!",
				(int)last_vnum, pMobIndex->short_descr);
			exit_error( 1 , "load_mobiles", "invalid default position");
		}
		
		pMobIndex->sex					= sex_lookup(fread_word(fp));
		if (pMobIndex->sex==-1){
			bugf("load_mobiles: mob %d (%s) sex category doesn't exist!",
				(int)last_vnum, pMobIndex->short_descr);
			exit_error( 1 , "load_mobiles", "invalid sex");
		}
		
		pMobIndex->wealth				= fread_number( fp );

		if (pMobIndex->wealth<0){
			bugf("load_mobiles: mob %d (%s) wealth is less than 0!",
				(int)last_vnum, pMobIndex->short_descr);
			exit_error( 1 , "load_mobiles", "negative wealth");
		}
		
		if(pMobIndex->wealth > pMobIndex->level*10){
			pMobIndex->wealth=pMobIndex->level*10;
		}
		
		
		pMobIndex->form 				= fread_flag( fp )
										| race_table[pMobIndex->race]->form;
		pMobIndex->parts				= fread_flag( fp )
										| race_table[pMobIndex->race]->parts;
		// size 
		pMobIndex->size 				= size_lookup(fread_word(fp));

		// material
		pMobIndex->material 			= str_dup(lowercase(fread_word( fp )));
		if(!str_cmp(pMobIndex->material,"0")){
			replace_string(pMobIndex->material,"unknown");
		}

		
		for ( ; ; )
		{
			letter = fread_letter( fp );

			if (letter == 'G') // read their group (if they have one)
			{
				char *word= fread_word(fp);
				if(is_number(word)){
					pMobIndex->group=atoi(word);
				}else{
					bugf("Unexpected input in loading mobile %d's group number "
						"- must be numeric 'G %s'", pMobIndex->vnum, word);
					exit_error( 1 , "load_mobiles", "unexpected group number input");
				}
			}
			else if (letter == 'H') // read their helpgroup (if they have one)
			{
				char *word= fread_word(fp);
				if(is_number(word)){
					pMobIndex->helpgroup=atoi(word);
				}else{
					bugf("Unexpected input in loading mobile %d's helpgroup number "
						"- must be numeric 'H %s'", pMobIndex->vnum, word);
					exit_error( 1 , "load_mobiles", "unexpected helpgroup number input");
				}
			}
			else if (letter == 'F')
			{
				char *word;
				long vector;
				
				word					= fread_word(fp);
				vector					= fread_flag(fp);
				
				if (!str_prefix(word,"act"))
					REMOVE_BIT(pMobIndex->act,vector);
				else if (!str_prefix(word,"aff"))
					REMOVE_BIT(pMobIndex->affected_by,vector);
				else if (!str_prefix(word,"off"))
					REMOVE_BIT(pMobIndex->off_flags,vector);
				else if (!str_prefix(word,"imm"))
					REMOVE_BIT(pMobIndex->imm_flags,vector);
				else if (!str_prefix(word,"res"))
					REMOVE_BIT(pMobIndex->res_flags,vector);
				else if (!str_prefix(word,"vul"))
					REMOVE_BIT(pMobIndex->vuln_flags,vector);
				else if (!str_prefix(word,"for"))
					REMOVE_BIT(pMobIndex->form,vector);
				else if (!str_prefix(word,"par"))
					REMOVE_BIT(pMobIndex->parts,vector);
				else
				{
					bugf("load_mobiles: flag '%s' not found.", word);
					exit_error( 1 , "load_mobiles", "flag not found");
				}
			}
			else if ( letter == 'M' )
			{
				MUDPROG_TRIGGER_LIST *pMprog;
				char *word;
				int trigger = 0;
				
				pMprog		   = (MUDPROG_TRIGGER_LIST *)alloc_perm(sizeof(*pMprog));
				word		   = fread_word( fp );

				// support position flags
				if(word[0]=='='){ 
					pMprog->pos_flags= fread_flag(fp);
					word= fread_word( fp );
				}

				if ( !(trigger = flag_lookup( word, mprog_flags )) )
				{
					bug("load_mobiles: invalid mudprog trigger.");
					exit_error( 1 , "load_mobiles", "invalid mudprog trigger");
				}
				SET_BIT( pMobIndex->mprog_flags, trigger );
				pMprog->trig_type	= trigger;

				// get the mudprog number, with vnum translation support
				int mpvnum=fread_number( fp );
				apply_area_vnum_offset( &mpvnum); // this system only works within a particular area
				pMprog->temp_mpvnum	= mpvnum; // hack for loading - fixed up in fix_mudprogs

				pMprog->trig_phrase = fread_string( fp );
				pMprog->next		= pMobIndex->mob_triggers;
				pMobIndex->mob_triggers	= pMprog;
			}		
			else
			{
				ungetc(letter,fp);
				break;
			}
		}
		
		iHash					= vnum % MAX_KEY_HASH;
		pMobIndex->next 		= mob_index_hash[iHash];
		mob_index_hash[iHash]	= pMobIndex;
		top_mob_index++;
		kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;

		// linked list of all pMobIndex records
		pMobIndex->listnext=pMobIndexlist;
		pMobIndexlist=pMobIndex;
	}
	return;
}


/**************************************************************************/
/**************************************************************************/
// Snarf an obj section
void load_objects( FILE *fp, int version )
{
	OBJ_INDEX_DATA *pObjIndex;
	
	if ( !area_last ){
		bug("Load_objects: no #AREA seen yet.");
		exit_error( 1 , "load_objects", "no #AREA seen yet");
	}
	
	for ( ; ; )
	{
		vn_int vnum;
		char letter;
		int iHash;
		
        letter = fread_letter( fp );
        if ( letter != '#' )
        {
            bug("Load_objects: # not found.");
            exit_error( 1 , "load_objects", "# not found");
        }
		
        vnum = fread_number( fp );
		if ( vnum == 0 )
            break;
		vnum+= area_last->vnum_offset; 
		
        fBootDb = false; // put here because get_obj_index() calls bug()
        if ( get_obj_index( vnum ) )
        {
			char *aname="an unknown area.";
			if(get_obj_index( vnum )->area 
				&& get_obj_index( vnum )->area->file_name){
				aname=get_obj_index( vnum )->area->file_name;
			}
            bugf( "Load_objects: vnum %d duplicated.", vnum );

			logf("with %s from %s",
				get_obj_index( vnum )->short_descr,
				aname);
			exit_error( 1 , "load_objects", "duplicate vnum");
		}
        fBootDb = true;
		
        last_vnum = vnum; // backup the last vnum for gdb use
		
        pObjIndex = (OBJ_INDEX_DATA *)alloc_perm( sizeof(*pObjIndex) );
        pObjIndex->vnum = vnum;
        pObjIndex->area = area_last;

        // check object vnum fits in areas vnum range 
        if ( vnum < pObjIndex->area->min_vnum + pObjIndex->area->vnum_offset)
        {
            bugf("Object with Vnum %d is less than area %s <%s> vnum %d!",
                vnum,
                pObjIndex->area->name,
                pObjIndex->area->file_name,
                pObjIndex->area->min_vnum
                );
        }
        if ( vnum > pObjIndex->area->max_vnum + pObjIndex->area->vnum_offset)
        {
            bugf("Object with Vnum %d is greater than area %s <%s> vnum %d!",
                vnum,
                pObjIndex->area->name,
                pObjIndex->area->file_name,
                pObjIndex->area->max_vnum
                );
        }
		
		
        pObjIndex->reset_num            = 0;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
		{
			char * pStr					= fread_string( fp );
			pObjIndex->material         = str_dup(lowercase(pStr));
			free_string(pStr);
		}
		
		char *itypeword=fread_word( fp );
        pObjIndex->item_type            = item_lookup(itypeword);
		if(pObjIndex->item_type<1){
			bugf("Unrecognised item type '%s' for object %d", itypeword, pObjIndex->vnum);
			exit_error( 1 , "load_objects", "unrecognised item type");
		}
        pObjIndex->extra_flags          = fread_flag( fp );
		if ( version > 3 )
		{
			pObjIndex->extra2_flags		= fread_flag( fp );

			// Traps loading code here
			if ( IS_SET( pObjIndex->extra2_flags, OBJEXTRA2_TRAP ))
			{
				pObjIndex->trap_trig	= fread_number( fp );
				pObjIndex->trap_dtype	= fread_number( fp );
				pObjIndex->trap_charge	= fread_number( fp );
				pObjIndex->trap_modifier= fread_number( fp );
			}
			else // initialize it to zero to be thorough
			{
				pObjIndex->trap_trig	= 0;
				pObjIndex->trap_dtype	= 0;
				pObjIndex->trap_charge	= 0;
				pObjIndex->trap_modifier= 0;
			}
		}
        pObjIndex->wear_flags           = fread_flag( fp );
		
		if(AREA_IMPORT_FORMAT(AIF_FORMAT2)){
			areaimport_object_translate_flags_format2(fp, version, pObjIndex);
		}else if(AREA_IMPORT_FORMAT(AIF_FORMAT2)){
			areaimport_object_translate_flags_format3(fp, version, pObjIndex);
		}else{
			// AIF_STOCK - the default
			areaimport_object_translate_flags_stock(fp, version, pObjIndex);
		}

		load_object_values( fp, version, pObjIndex);

        pObjIndex->level                = fread_number( fp );
        pObjIndex->weight               = fread_number( fp );
        pObjIndex->cost                 = fread_number( fp );
		
        // condition
        letter                          = fread_letter( fp );
        switch (letter)
        {
		case ('P') :           pObjIndex->condition = 100; break;
		case ('G') :           pObjIndex->condition =  90; break;
		case ('A') :           pObjIndex->condition =  75; break;
		case ('W') :           pObjIndex->condition =  50; break;
		case ('D') :           pObjIndex->condition =  25; break;
		case ('B') :           pObjIndex->condition =  10; break;
		case ('R') :           pObjIndex->condition =   1; break;
		default:               pObjIndex->condition = 100; break;
        }
		if( version<2){
			pObjIndex->absolute_size=pObjIndex->weight;
			pObjIndex->relative_size=50;
		}else{				
			pObjIndex->absolute_size = fread_number(fp);
			pObjIndex->relative_size = fread_number(fp);		
		}

		if( version<3){
			pObjIndex->class_allowances = 0; 
		}else{
			if(version<5){
				int tempint= (sh_int)fread_flag(fp);
				// convert the class restriction into the allowance counter part
				if(tempint){		
					pObjIndex->class_allowances=~tempint;
					pObjIndex->class_allowances=tempint&0x1FF; // only the relevant bits
				}
			}
		}
		
        // optional parameters
		for ( ; ; )
        {
            char letter;
			
            letter = fread_letter( fp );
			if ( letter == 'A'){
				AFFECT_DATA *paf;
				paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
				paf->where              = WHERE_OBJEXTRA;
				paf->type               = -1;
				paf->level              = pObjIndex->level;
				paf->duration           = -1;
				paf->location           = translate_old_apply_number(fread_number( fp ));
				paf->modifier           = fread_number( fp );
				paf->bitvector          = 0;
				paf->next               = pObjIndex->affected;
				pObjIndex->affected     = paf;
				top_affect++;

				// convert to WHERE_MODIFIER if appropriate
				if(version==2){
					if(paf->location!=APPLY_NONE){
						paf->where=WHERE_MODIFIER;		
					}
					if(paf->location==1){
						paf->modifier=paf->modifier*2;
					}else if(paf->location==2){
						paf->modifier=paf->modifier*2;
						paf->location=APPLY_RE;  
					}else if(paf->location==3){
						paf->modifier=paf->modifier*2;
						paf->location=APPLY_ME;
					}else if(paf->location==4){
						paf->modifier=paf->modifier*2;
						paf->location=APPLY_QU;
					}else if(paf->location==5){
						paf->modifier=paf->modifier*2;
						paf->location=APPLY_CO;
					}else if(paf->location==APPLY_HIT){
						paf->modifier=paf->modifier/3;
					}
				}

			}else if (letter == 'B'){
				AFFECT_DATA *paf;
				paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
				paf->where              = WHERE_OBJEXTRA2;
				paf->type               = -1;
				paf->level              = pObjIndex->level;
				paf->duration           = -1;
				paf->location           = translate_old_apply_number(fread_number( fp ));
				paf->modifier           = fread_number( fp );
				paf->bitvector          = 0;
				paf->next               = pObjIndex->affected;
				pObjIndex->affected     = paf;
				top_affect++;
			}else if ( letter == 'L' && AREA_IMPORT_FORMAT(AIF_FORMAT3)){ 
				// load percentage - ignored
				fread_number(fp);
			}else if ( letter == 'C' || letter == '_'){ 
				if(letter == 'C' && AREA_IMPORT_FORMAT(AIF_FORMAT3)){
	                fread_string(fp); // a clan field
				}else{
    				if(letter == '_'){ // the _ so we can eventually remove the classrestrict
						fread_letter( fp ); // C
						fread_letter( fp ); // A
					}
					pObjIndex->class_allowances=fread_wordflag(classnames_flags, fp);
				}
			}else if ( letter == 'R' ){
				// format of R optional object line - class groups restrictions
				// R [classgroup_restriction] [affectprofile] [priority] [flags]
                OBJRESTRICT_LIST_DATA *pr;
				classgroup_type * cg;
				affectprofile_type *ap;

				char * classgroup	=fread_word ( fp );
				char * affectprofile=fread_word ( fp );
				int priority=fread_number( fp );
				int flags=fread_flag( fp );
				flags=0; // to get rid of compiler warning :)

				cg=classgroup_lookup(classgroup);
				ap=affectprofile_lookup(affectprofile);
				if(!cg || !ap){
					if(!cg){
						bugf("Unknown classgroup '%s' for object vnum %d restriction, IGNORING!", 
							classgroup, vnum);
					}
					if(!ap){
						bugf("Unknown affectprofile '%s' for object vnum %d restriction, IGNORING!", 
							affectprofile, vnum);
					}
				}else{
					pr=new OBJRESTRICT_LIST_DATA;
					pr->affectprofile=ap;
					pr->classgroup=cg;
					pr->priority=priority;
					// can't have positive affect modifiers 
					if(ap->wear_amount>0){
						bugf("positive ap->wear_amount for %s, inverted.", ap->name);
						ap->wear_amount=0-ap->wear_amount;
					}		
					// Add it to the restricts list
					pr->next			= pObjIndex->restrict;
					pObjIndex->restrict	= pr;
					SET_BIT(pObjIndex->objrestrict,(1<<cg->bitindex)); // create bit quick lookup
				}
			}else if (letter == 'F'){
				AFFECT_DATA *paf=NULL;
				
                paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
                letter                  = fread_letter(fp);
                switch (letter)
                {
                case 'A':
                    paf->where          = WHERE_AFFECTS;
                    break;
                case 'B':
                    paf->where          = WHERE_AFFECTS2;
                    break;
                case 'C':
                    paf->where          = WHERE_AFFECTS3;
                    break;
                case 'I':
					paf->where          = WHERE_IMMUNE;
                    break;
				case 'K':
					paf->where			= WHERE_SKILLS;
					break;
                case 'R':
                    paf->where          = WHERE_RESIST;
                    break;
                case 'V':
					paf->where			= WHERE_VULN;
                    break;
				case 'Z':
					paf->where			= WHERE_OBJECTSPELL;
					break;
                default:
                    bugf( "Load_objects: Unknown where flag 'F %c'.", letter);
					exit_error( 1 , "load_objects", "unknown where flag");
                }

				if ( paf->where == WHERE_SKILLS )
				{
					char *skillname			= fread_word(fp);
					paf->type				= skill_lookup(skillname);
					if(paf->type<0){
						bugf("Unknown skillname '%s' found while reading area file.",
							skillname);
						exit_error( 1 , "load_objects", "unknown skill name");
					}
					paf->modifier			= fread_number( fp );
					paf->bitvector			= 0;
				} else if (paf->where == WHERE_OBJECTSPELL){
					paf->location			= APPLY_NONE;
					char *spellname			= fread_word(fp);
					paf->type				= skill_lookup(spellname);
					if(paf->type<0){
						bugf("Unknown spellname '%s' found while reading area file.",
							spellname);
						exit_error( 1 , "load_objects", "unknown spell name");
					}
					paf->level				= fread_number(fp);
					paf->duration			= fread_number(fp);
					paf->bitvector          = fread_flag(fp);
				}else {
					paf->type				= -1;
					paf->location           = translate_old_apply_number(fread_number(fp));
					paf->modifier           = fread_number(fp);
					if(version==1 && AREA_IMPORT_FORMAT(AIF_FORMAT2)){
						paf->bitvector          = fread_number(fp);
					}else{
						// AIF_STOCK - the default
						paf->bitvector          = fread_flag(fp);
					}
				}

				paf->duration				= -1;
	            paf->level					= pObjIndex->level;
				paf->next					= pObjIndex->affected;
			    pObjIndex->affected			= paf;
                top_affect++;

            }
		    // some muds have Z for object spells
		    else if ( letter == 'Z' )
			{
			    int sn;
			    AFFECT_DATA *paf;
				char *last_word;		    
			    paf                     = (AFFECT_DATA*) alloc_perm (sizeof (*paf));
				last_word=fread_word (fp);
			    sn = skill_lookup (last_word);
			    if (0 > sn)
				{
				    bugf("Load_objects: unknown spell '%s'.", last_word);
					exit_error( 1 , "load_objects", "unknown spell");
				}else{
					paf->type = sn;
				}
				paf->where				= WHERE_OBJECTSPELL;
			    paf->location			= APPLY_NONE;
			    paf->level              = fread_number (fp);
			    paf->duration           = fread_number (fp);
			    paf->modifier           = 0;
			    paf->bitvector          = 0;
				paf->next					= pObjIndex->affected;
			    pObjIndex->affected			= paf;
			}	
            else if ( letter == 'E' )
			{
				EXTRA_DESCR_DATA *ed;   
				
				ed                      = (EXTRA_DESCR_DATA *)alloc_perm( sizeof(*ed) );
				ed->keyword             = fread_string( fp );
				ed->description         = fread_string( fp );
				
				if (str_len(ed->description)>MSL-4 || str_len(ed->keyword)>MSL-4 )
				{
					bugf("load_objects_three: Extended description in object "
						"%d is %d characters!!! (more than %d)", 
						(int)last_vnum, str_len(ed->description), MSL-4);
					exit_error( 1 , "load_objects", "extended description too long");
				}
				
				// chop off the . if it starts with that (version 3 up)
				if (version>2 && ed->description[0]=='.')
				{
					char *temp_string;
					temp_string = str_dup(ed->description);
					free_string(ed->description);
					ed->description= str_dup(temp_string+1);
					free_string(temp_string);
				}
				ed->next                = pObjIndex->extra_descr;
				pObjIndex->extra_descr  = ed;
				top_ed++;
			}else{ // end of optional parameters
				ungetc( letter, fp );// most likely a # starting next object
                break;
            }
        }
		
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
	}
	
    return;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
