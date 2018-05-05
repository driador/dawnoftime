/**************************************************************************/
// mxp.cpp - mxp code
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "mxp.h"

/**************************************************************************/
// converts '<' into MXP_BEGIN_TAG and '>' into MXP_END_TAG
char *mxp_tagify(const char *mxp_text_with_unencoded_tags)
{
	static char result[5][MSL*2];
	static int i;
	++i%=5;
	char *r=result[i];
	for(const char *p=mxp_text_with_unencoded_tags; !IS_NULLSTR(p); p++){
		if(*p=='&'){
			*r++=MXP_AMPERSAND;
		}else if(*p=='<'){
			*r++=MXP_BEGIN_TAG;
		}else if(*p=='>'){
			*r++=MXP_END_TAG;
		}else{
			*r++=*p;
		}
	}
	*r='\0';
	return result[i];
}
/**************************************************************************/
// converts '<' into &lt;, '>' into &gt; and & into &amp;
char *mxp_convert_to_mnemonics(const char *text_with_raw_characters)
{
	static char result[5][MSL*2];
	static int i;
	++i%=5;
	char *r=result[i];
	for(const char *p=text_with_raw_characters; !IS_NULLSTR(p); p++){
		if(*p=='&'){
			*r++='&';
			*r++='a';
			*r++='m';
			*r++='p';
			*r++=';';
		}else if(*p=='<'){
			*r++='&';
			*r++='l';
			*r++='t';
			*r++=';';
		}else if(*p=='>'){
			*r++='&';
			*r++='g';
			*r++='t';
			*r++=';';
		}else{
			*r++=*p;
		}
	}
	*r='\0';
	return result[i];
}
/**************************************************************************/
// put the secure prefix before every line
char *mxp_prefix_secure(const char *text_to_be_prefixed)
{
	static char result[5][MSL*2];
	static int i;
	++i%=5;
	char *r=result[i];
	for(const char *p=text_to_be_prefixed; !IS_NULLSTR(p); p++){
		if(*p=='\n' || (*p=='}' && *(p-1)=='`')){
			*r++=*p;
			{
				char *prefix=MXP_SECURE_PREFIX;
				while(!IS_NULLSTR(prefix)){
					*r++=*prefix++;
				}
			}

		}else{
			*r++=*p;
		}
	}
	*r='\0';
	return result[i];

}
/**************************************************************************/
void do_mxp( char_data *ch, char *argument )
{
	pc_data *pcdata=TRUE_CH(ch)->pcdata; // the characters pcdata we want to work on
	if(!pcdata){
		ch->println("Players can only use this command");
		return;
	}

	if(IS_NULLSTR(argument)){
		ch->titlebar("MUD EXTENSION PROTOCOL OPTIONS");
		ch->println("Syntax: `=Cmxp off`x  - mxp is permanately off.");
		ch->wrapln("Syntax: `=Cmxp auto`x - mud will attempt to automatically "
					"detect if you have mud client that supports mxp, if one is detected "
					"the mud will send you mxp tags");
		ch->wrapln("Syntax: `=Cmxp on`x  - mxp is permanately on, mud will send you mxp "
			"tags even if your mud client doesn't support mxp.");
		ch->println("");
		ch->printlnf("Your mxp preference is currently set to %s", 
			preference_word(pcdata->preference_mxp));
		if(pcdata->preference_mxp==PREF_AUTOSENSE){
			ch->printlnf("`WYour connections mxp support has %sbeen automatically detected.`x",
				(ch->desc && IS_SET(ch->desc->flags, CONNECTFLAG_MXP_DETECTED))?"":"not ");
		}
		ch->titlebar("");
		return;
	}

	PREFERENCE_TYPE pt;
	if(!str_prefix(argument, "off")){
		pt=PREF_OFF;
	}else if(!str_prefix(argument, "autosense")){
		pt=PREF_AUTOSENSE;
	}else if(!str_prefix(argument, "on")){
		pt=PREF_ON;
	}else{
		ch->printlnf("Unsupported mxp option '%s'", argument);
		do_mxp(ch,"");
		return;
	}
	if(pcdata->preference_mxp==pt){
		ch->printlnf("Your mxp preference is already set to %s", preference_word(pt));
		return;
	}

	ch->printlnf("mxp preference changed from %s to %s", 
		preference_word(pcdata->preference_mxp),
		preference_word(pt));
	pcdata->preference_mxp=pt;
	ch->mxp_send_init(); // update the MXP cached info
}
/*************************************************************************/
void do_mxpreflect( char_data *ch, char *argument )
{
	if(IS_NULLSTR(argument)){
		ch->titlebar("MXP REFLECT");
		ch->println("Syntax: `=Cmxpreflect <text you want reflected/echoed back to you as mxp>`x");
		ch->println("");
		ch->wrapln("Explaination:  Normally the < and > symbols are converted into &lt; and &gt; "
			"by the mud - this command can be used to test MXP codes, each line is "
			"prefixed by ESC[1z (the secure line tag).  Use ``1 to start a new line "
			"for multilined echos.");
		ch->titlebar("");
		return;
	}
	ch->printlnf("MXPReflect of '%s' follows:", argument);
	ch->printlnf("%s%s", MXP_SECURE_PREFIX, mxp_prefix_secure(mxp_tagify(argument)));
}
/**************************************************************************/
// creates the mxp tag for mxp_create_tag(), 
const char *mxp_create_tag_core(const char *tagname, const char *txt)
{
	// get our return buffer
	static int i;
	static char result[5][MSL*2];
	++i%=5;

	// setup our pointers
	char *r=result[i];	// r is a pointer to what will be returned 
	const char *p=txt;	// p is a pointer to the source text
	const char *t=NULL;	// t is a misc pointer used for various tasks

	// copy any leading whitespace from txt into the tag
	while(is_space(*p)){
		*r++=*p++;
	}

	// if there is no more source text left then we have just formatted 
	// an empty string or a string of whitespace - regardless return it.
	if(*p=='\0'){
		*r='\0'; // terminate the result first
		return result[i];
	}

	// we now have any leading whitespace outside of where the tag
	// starts, now start the tag
	*r++=MXP_BEGIN_TAG;
	t=tagname;
	while(*t){
		*r++=*t++;
	}
	*r++=MXP_END_TAG;

	// r now points at where the non whitespace text will begin
	// p points at the first non whitespace, non null character in the input text

	// next find the last non whitespace character in the input text
	for(t=p; *t; t++){
		// fast forward to the end of the input text
	}
	t--; // jump back to the last character in the string
	// now backup till we hit the last non whitespace character in the string
	while(is_space(*t)){
		t--; 
	}

	// copy from p into r until p is a character past t (the last non whitespace character)
	while(p<=t){
		*r++=*p++;
	}

	// put the closing tag on the result
	*r++=MXP_BEGIN_TAG;
	*r++='/';
	t=tagname;
	// close the tag up to the end of the string or first whitespace
	while(*t && !is_space(*t)){ 
		*r++=*t++;
	}
	*r++=MXP_END_TAG;

	// copy the remaining whitespace from p to r
	while(*p){
		*r++=*p++;
	}

	// terminate the result
	*r='\0';

	// return the result :)
	return result[i];
}

/**************************************************************************/
// create an mxp tag for a character - Kal June 01
const char *mxp_create_tag(char_data *ch, const char *tagname, const char *txt)
{
	// if they don't have MXP or the tag is empty, just return the txt 
	if(!HAS_MXP(ch) || IS_NULLSTR(tagname)){
		return txt;
	}
	return mxp_create_tag_core(tagname, txt);
}

/**************************************************************************/
// creates an mxp tag for those with MXP on... and strips whitespace around 
// the tag to outside the tag.  If they have MXP off it just returns the
// formatted result.
const char *mxp_create_tagf(char_data *ch, const char *tagname, const char *fmt, ...)
{
	static int i;
	static char result[5][MSL*2];
	++i%=5;
	char *r=result[i];

	va_list args;
	va_start(args, fmt);
	vsnprintf(r, MSL*2, fmt, args);
	va_end(args);
	
	if(HAS_MXP(ch)){
		return mxp_create_tag(ch, tagname, r);
	}
	return r;
}
/**************************************************************************/
const char *mxp_create_send(char_data *ch, const char *command, const char *text)
{
	return mxp_create_tag(ch, FORMATF("send href=\"%s\"", command), text);
}
/**************************************************************************/
const char *mxp_create_send(char_data *ch, const char *command_and_text)
{
	return mxp_create_send(ch, command_and_text, command_and_text);

}
/**************************************************************************/
const char *mxp_create_send_prompt(char_data *ch, const char *command, const char *text)
{
	return mxp_create_tag(ch, FORMATF("send href=\"%s\" prompt", command), text);
}
/**************************************************************************/
const char *mxp_create_send_prompt(char_data *ch, const char *command_and_text)
{
	return mxp_create_send_prompt(ch, command_and_text, command_and_text);

}
/**************************************************************************/
#define MXPELEMENT_IMMONLY	(A)
/**************************************************************************/
struct mxp_element_type {
	char *name;
	int flags;
	char *attributes;
	char *variable;
	char *text;
};
/**************************************************************************/
mxp_element_type mxp_elements[]=
{
	// name, tableflags,	attribute=, client_variable
	{ "help",		0,		"",			"",
		"<send href=\"help &text;\">" 
	},

	{ "rname",		0,		"",			"RoomName",
		"" 
	},

	{ "rdesc",		0,		"",			"RoomDesc",
		"" 
	},

	{ "rexits",		0,		"",			"RoomExit",
		"" 
	},

	{ "ex",			0,		"",			"", 
		"<send>" 
	},

	{ "buy",		0,		"",			"",
		"<send href=\"buy &text;\">" 
	},

	{ "buy-uid",		0,		"uid=0", "",
		"<send href=\"buy #&uid;\"    " 
		"  hint=\"Buy &text;\">"
	},

	// immortal only entries
//============ CHARACTER - ch_ prefix
	// character(uid) name
	{ "ch-uid_name", MXPELEMENT_IMMONLY, "uid=0", "",
		"<send href=\"at #&uid; look|dlook &text;|score &text;|charinfo &text;\" "
		"hint=\"Look in the room where &text; is|description look &text;|score &text;|charinfo &text;\">"
	},

	{ "oolcvnum", 0, "", "",
		"<send href=\"oshow &text;|oedit &text;\"   "
			"  hint=\"oshow &text;|oedit &text;\">"
	},

	{ "molcvnum", 0, "", "",
		"<send href=\"mshow &text;|medit &text;\"   "
			"  hint=\"mshow &text;|medit &text;\">"
	},

	{ "mbvnum", MXPELEMENT_IMMONLY, "uid=0", "",
		"<send href=\"at #&uid; look|at #&uid; look #&uid;|goto #&uid;|mshow &text;|medit &text;\"   "
			"  hint=\"Look in the room of this mob|Look at the mob itself|Goto the room the mob is in|mshow &text;|medit &text;\">"
	},

	{ "mprogvnum", 0, "uid=0", "",
		"<send href=\"mpshow &text;|mpedit &text;\">"
	},


	{ "rmvnum", MXPELEMENT_IMMONLY, "uid=0", "",
		"<send href=\"goto &text;|at &text; look\">"
	},

	{ "rmv", 0, "roomvnum=0 tl=\"\"", "",
		"<send href=\"goto &roomvnum;|at &roomvnum; look\">"
	},

	{ "rmvh", 0, "roomvnum=0 tl=\"\"", "",
		"<send href=\"goto room &roomvnum; - &tl;-|goto &roomvnum;|at &roomvnum; look\">"
	},


//============ MOB - mb_ prefix 
	// basic mob - (basic mob)
	{ "mb-uid-nm", 0, "uid=0 nm=\"\"", "",
		"<send href=\"look #&uid;|con #&uid;\"   "
			"  hint=\"Look at &nm;|Consider &nm;\">"
	},

//============ OBJECT - ob_ prefix 
	// object used as equipment
	{ "objeq", 0, "uid=0 nm=\"\"", "",
		"<send href=\"remove #&uid;|look #&uid;|examine #&uid;\"   "
			"  hint=\"Remove &text;|Look at &text;|Examine &text;\">"
	},

	// basic object
	{ "ob-uid-nm", 0, "uid=0 nm=\"\"", "",
		"<send href=\"look #&uid;|examine #&uid;\"   "
			"  hint=\"Look at &text;|Examine &text;\">"
	},

	// basic object - getable
	{ "ob-uid-nm_g", 0, "uid=0 nm=\"\"", "",
		"<send href=\"get #&uid;|look #&uid;|examine #&uid;\"   "
			"  hint=\"get &nm;|Look at &nm;|Examine &nm;\">"
	},


	// food
	{ "ob-uid-nm_food", 0, "uid=0 nm=\"\"", "",
		"<send href=\"eat #&uid;|look #&uid;|examine #&uid;\"   "
			"  hint=\"Eat &nm;|Look at &nm;|Examine &nm;\">"
	},

	// food
	{ "ob-uid-nm_food_g", 0, "uid=0 nm=\"\"", "",
		"<send href=\"get #&uid;|eat #&uid;|look #&uid;|examine #&uid;\"   "
			"  hint=\"Get &nm;|Eat &nm;|Look at &nm;|Examine &nm;\">"
	},

	// drink
	{ "ob-uid-nm_drink", 0, "uid=0 nm=\"\"", "",
		"<send href=\"drink #&uid;|look #&uid;|examine #&uid;|fill #&uid;\"   "
			"  hint=\"Drink &nm;|Look at &nm;|Examine &nm;|Fill &nm;\">"
	},

	// drink
	{ "ob-uid-nm_drink_g", 0, "uid=0 nm=\"\"", "",
		"<send href=\"get #&uid;|drink #&uid;|look #&uid;|examine #&uid;|fill #&uid;\"   "
			"  hint=\"Get &nm;|Drink &nm;|Look at &nm;|Examine &nm;|Fill &nm;\">"
	},

	// fountain
	{ "ob-uid-nm_fount", 0, "uid=0 nm=\"\"", "",
		"<send href=\"drink #&uid;|look #&uid;|examine #&uid;\"   "
			"  hint=\"Drink from &nm;|Look at &nm;|Examine &nm;\">"
	},

//============ TELLS - tl_ prefix
	// tell reply
	{ "tl_rp", 0, "", "",
		"<send href=\"reply \" PROMPT>"
	},

	// tell(name) name 'reply', 'tell name'
	{ "tl-nm_rp_tlnm", 0, "nm", "",
		"<send href=\"reply |tell &nm; \" PROMPT>"
	},

	// tell(name) name 'reply', 'tell name'
	{ "tl-nm_rp_tlnm", 0, "nm", "",
		"<send href=\"reply |tell &nm; \" PROMPT>"
	},

	// tell(name) name 'retell', 'tell name'
	{ "tl-nm_rt_tlnm", 0, "nm", "",
		"<send href=\"&gt;|tell &nm;\" "
		"hint=\"Retell &nm;|tell &nm;\" PROMPT>"
	},

	{""}	// end of the table
};
/**************************************************************************/
void mxp_define_objinv_elements_to_char(char_data *ch)
{
	char attribs[MIL],send[MIL], hint[MIL];

	for(int i=0; i<6; i++){
		sprintf(attribs, "<!ELEMENT objinv%d ATT='uid=0 nm", i);
		strcpy(send, "'<send href=\"");
		strcpy(hint, " hint=\"");

		for(int j=0; j<i; j++){
			strcat(attribs, FORMATF(" a%d", j));
			strcat(send, FORMATF("&a%d; #&uid;|", j));
			strcat(hint, FORMATF("&a%d; &nm;|", j));
		}
		strcat(attribs,"'");
		strcat(send, "look #&uid;|examine #&uid;|drop #&uid;\"");
		strcat(hint, "Look at &text;|Examine &text;|Drop &text;\">'");

		ch->print( mxp_tagify( FORMATF( "%s %s %s >", 
			attribs,
			send,
			hint)));
	}

}

/**************************************************************************/
void mxp_define_elements_to_char(char_data *ch)
{
	ch->print( MXP_SECURE_MODE);
	// send mxp elements
	for(mxp_element_type *e=mxp_elements; !IS_NULLSTR(e->name); e++){

		if(IS_SET(e->flags, MXPELEMENT_IMMONLY) && !IS_IMMORTAL(ch)){
			// define the element to be empty
			ch->print( mxp_tagify( FORMATF( "<!ELEMENT %s>", e->name)));
			continue;
		}

		ch->print( mxp_tagify( FORMATF(
				"<!ELEMENT %s%s%s %s>", 
					e->name, 
					IS_NULLSTR(e->attributes)?"": FORMATF(" ATT='%s'", e->attributes),
					IS_NULLSTR(e->variable)?"": FORMATF(" FLAG='%s'", e->variable),
					IS_NULLSTR(e->text)?"": FORMATF("'%s'", e->text)
			)));
	}
	mxp_define_objinv_elements_to_char(ch);
	ch->print( mxp_tagify("<version><support>") ); 
}

/**************************************************************************/
char_data *find_keeper( char_data *ch );
/**************************************************************************/
char *mxp_tag_for_inventory_object(char_data * ch, OBJ_DATA *obj)
{
	// the object must be in the inventory of ch
	assert(obj->carried_by==ch);

	static int i;
	static char result[5][MIL];
	++i%=5;
	char *r=result[i];

	char att[MSL];
	int att_count=0;

	// put on the basic stuff at the start
	strcpy(att, FORMATF("%d \"%s\"", obj->uid, strip_colour(obj->short_descr)));

		
/*
if you have it in your inventory you can do the following:
inventory, always:
- look 
- examine
- drop
- give*
- put*
- show

Some times you can:
- drink, eat, wield and second, look in  // primary action

- sell and value (shop keeper around)
- wear

Sell
Value
Drink

*/
	const char *a="";

	// create additional actions based on the object type
	switch ( obj->item_type )
	{
		default: break;

	case ITEM_CAULDRON:
	case ITEM_CONTAINER:
	case ITEM_FLASK:
	case ITEM_MORTAR:
		a=" \"Look in\"";
		att_count++;
		break;

	case ITEM_POTION:
		a=" Quaff";
		att_count++;
		break;

	case ITEM_WEAPON:
		a=" Wield Second";
		att_count+=2;
		break;

	case ITEM_FOOD:			
		a=" Eat";
		att_count++;
		break;

	case ITEM_DRINK_CON:	
		a=" Drink";
		att_count++;
		break;
	}
	strcat(att, a);
	
	// can we hold the object?
	if(CAN_WEAR(obj, OBJWEAR_HOLD)){
		strcat(att, " Hold");
		att_count++;
	}else{
		// can we wear the object in a way other than hold
		long wf=obj->wear_flags;
		REMOVE_BIT(wf, OBJWEAR_TAKE 
						| OBJWEAR_WIELD
						| OBJWEAR_HOLD 
						| OBJWEAR_LODGED_ARM
						| OBJWEAR_LODGED_LEG
						| OBJWEAR_LODGED_RIB
						| OBJWEAR_SHEATHED
						| OBJWEAR_CONCEALED);
		if(wf){ // still have some wear flags
			strcat(att, " Wear");
			att_count++;
		}
	}

	// check if we are in a shop that buys this type of item
	// note: this is very inefficient, it should be written so 
	//       in the room data, it is recorded if a shopkeeper is present.
    for ( char_data *keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
		if( IS_NPC(keeper) ){
			SHOP_DATA *pShop=keeper->pIndexData->pShop;
			if(pShop){
				for ( int itype = 0; itype < MAX_TRADE; itype++ )
				{
					if ( obj->item_type == pShop->buy_type[itype] )
					{
						// will by the type
						strcat(att, " Sell Value");
						att_count+=2;
						break;
					}
				}
			}
		}
	}

	strcpy(r, FORMATF("objinv%d %s", att_count, att));
	return r;
}
/**************************************************************************/
// return the correct tag code for a particular object
char *mxp_tag_for_object(char_data * ch, OBJ_DATA *obj)
{
	if(!HAS_MXP(ch)){
		return "";
	}

	if(obj->carried_by){
		if(obj->carried_by==ch){
			if(obj->wear_loc==WEAR_NONE){
				// object is in inventory
				return mxp_tag_for_inventory_object(ch, obj);
			}else{
				// object is equipment
				return(FORMATF("objeq %d \"%s\"", obj->uid, strip_colour(obj->short_descr)));
			}
		}else{
			// object is carried by someone other than ch
			return "";
		}
	}

	static int i;
	static char result[5][MIL];
	++i%=5;
	char *r=result[i];
	
	strcpy(r, "ob-uid-nm");

	switch ( obj->item_type )
	{
		default:	
		if(obj->in_room && CAN_WEAR(obj, OBJWEAR_TAKE)){
			strcat(r,"_g");
		}
		break;

	case ITEM_FOOD:			
		strcat(r,"_food");	
		if(obj->in_room && CAN_WEAR(obj, OBJWEAR_TAKE)){
			strcat(r,"_g");
		}
		break;

	case ITEM_DRINK_CON:	
		strcat(r,"_drink");	
		if(obj->in_room && CAN_WEAR(obj, OBJWEAR_TAKE)){
			strcat(r,"_g");
		}
		break;

	case ITEM_FOUNTAIN:		strcat(r,"_fount");	break;

	}


	// put the uid on the end of the tag
	strcat(r, FORMATF(" %d \"%s\"", obj->uid, strip_colour(obj->short_descr)));
	return r;
}
/**************************************************************************/
// return the correct tag code for a particular mob
// Very basic for now... will be improved when the mxp spec improves or 
// someone has time/desire. - Kal, July 01
char *mxp_tag_for_mob(char_data * ch, char_data *mob)
{
	if(!HAS_MXP(ch) || !ch || !mob || (ch->in_room!=mob->in_room) ){
		return "";
	}
	
	return(FORMATF("mb-uid-nm %d \"%s\"", mob->uid, strip_colour(mob->short_descr)));
}

/**************************************************************************/
/**************************************************************************/

