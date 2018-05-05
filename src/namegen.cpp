/**************************************************************************/
// namegen.cpp - random name generator, Kalahn - Jan 99
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "namegen.h"

/**************************************************************************/
// create GIO lookup table 
GIO_START(name_profile)
GIO_STR(title)
GIO_STR_ARRAY(part, MAX_PARTS)
GIO_FINISH
/**************************************************************************/
name_profile * name_profiles_list = NULL;
int profile_count;
/**************************************************************************/
void do_read_nameprofiles( char_data *ch, char * )
{
	ch->printlnf("Reading name profiles from %s.", NAME_PROFILES_FILE);
	logf("===Reading nameprofiles() from %s.", NAME_PROFILES_FILE);

	GIOLOAD_LIST(name_profiles_list, name_profile, NAME_PROFILES_FILE); 

	if(!name_profiles_list){
		// if nothing read - use the default one created by Kriesara :)
		name_profiles_list=new name_profile;
		name_profiles_list->title=str_dup("mixed fantasy names");
		name_profiles_list->part[0]=str_dup("A Ab Ac Ad Af Agr Ast As Al Adw Adr "
			"Ar B Br C C C Cr Ch Cad D Dr Dw Ed Eth Et Er El Eow F Fr G Gr Gw Gw "
			"Gal Gl H Ha Ib Jer K Ka Ked L Loth Lar Leg M Mir N Nyd Ol Oc On P "
			"Pr R Rh S Sev T Tr Th Th V Y Yb Z W W Wic");
		name_profiles_list->part[1]=str_dup("a ae ae au ao are ale ali ay ardo e "
			"ei ea ea eri era ela eli enda erra i ia ie ire ira ila ili ira igo "
			"o oa oi oe ore u y");
		name_profiles_list->part[2]=str_dup("_ _ _ _ _ _ a and b bwyn baen bard "
			"c ctred cred ch can d dan don der dric dfrid dus f g gord gan l li "
			"lgrin lin lith lath loth ld ldric ldan m mas mos mar mond n nydd "
			"nidd nnon nwan nyth nad nn nnor nd p r ron rd s sh seth sean t th "
			"th tha tlan trem tram v vudd w wan win win wyn wyn wyr wyr wyth");
		name_profiles_list->next=NULL;		
	};

	// calculates the part_count fields
	profile_count=0;
	char *word;
	char buf[MIL];
	int i=0, count;
	name_profile *nl;
	for(nl=name_profiles_list; nl; nl=nl->next){	
		profile_count++;
		for(i=0; i<MAX_PARTS; i++){
			count=0;
			word=nl->part[i];
			while(true){
				word=one_argument(word, buf);
				if(IS_NULLSTR(word)){
					break;
				}
				count++;
			}
			nl->part_count[i]=count;
//			logf("nameprofile '%s': part_count[%d] = %d", nl->title, i, count);
		}
	}
	ch->println("Finished reading name profiles.");
	log_string ("do_read_nameprofiles(): finished.");
}
/**************************************************************************/
void do_write_nameprofiles( char_data *ch, char * )
{
	ch->printlnf("Writing name profiles to %s.", NAME_PROFILES_FILE);
	logf("Writing nameprofiles() to %s.", NAME_PROFILES_FILE);

	GIOSAVE_LIST(name_profiles_list, name_profile, NAME_PROFILES_FILE, true); 

	ch->println("Finished saving name profiles.");
	log_string ("do_write_nameprofiles(): finished.");
}
/**************************************************************************/
// generates a name from the profile
char * genname(name_profile * profile)
{
	static int i;
    static char result[5][100];
	// rotate buffers
	++i= i%5;
    result[i][0] = '\0';	

	char *word;
	char buf[MIL];
	int wordnum;
	int count;
	for(int j=0; j<MAX_PARTS; j++){
		word=profile->part[j];		
		wordnum=number_range(0,profile->part_count[j]);
		count=0;
		while(true){
			word=one_argument(word, buf);
			if(IS_NULLSTR(buf)){
				bugf("char * genname(%s) - no word found!!! j=%d, wn=%d, count=%d.", 
					profile->title, j, wordnum, count);
				break;
			}
			if(count==wordnum){
				break;
			}
			count++;
		}
		if(!IS_NULLSTR(buf) && buf[0]!='_'){ // _ to have blanks
			strcat(result[i], buf);
		}
	}
	return result[i];
}
/**************************************************************************/
// This is inefficient, but it is not a high demand part of dawn
void do_genname( char_data *ch, char *argument )
{
	if(!name_profiles_list){
		ch->println("Random name generation is currently unavailable.");
		return;
	}
	int count=0;
	name_profile *nl;

	if(IS_NULLSTR(argument) || !is_number(argument)){
		ch->titlebar("DAWN NAME GENERATOR");
		ch->println("Syntax: genname <number>");
		ch->println("Notes: <number> relates to the name profile you want to use");

		for(nl=name_profiles_list; nl; nl=nl->next){	
			ch->printlnf("%2d> %s", ++count, nl->title);
		}
		return;
	}

	int profile_num=atoi(argument);
	if(profile_num<1 || profile_num>profile_count){
		ch->println("Invalid profile number.\r\n");
		do_genname(ch,"");
		return;
	}
	for(nl=name_profiles_list; nl; nl=nl->next){	
		if(++count>=profile_num){
			break;
		};
	}
	int i;

	ch->titlebarf("DAWN NAME GENERATOR - %s", uppercase(nl->title));
	for(i=0; i<80; i++){
		ch->printf("  %-18s", capitalize(genname(nl)));
		if(i%4==3){
			ch->println("");
		}
	}
	ch->println("");
}
/**************************************************************************/

