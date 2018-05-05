/**************************************************************************/
// gio.cpp - see below
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
/***************************************************************************
 *  FILE: gio.cpp - generic input/output loading and saving function       *
 *        - supports tables, linked lists etc                              *
 *  This never really got all parts finished, but it is very functional.   *
 *  100% written by Kalahn :)                                              *
 ***************************************************************************/
#include "include.h"

bool gio_abort_fwrite_wordflag_with_undefined_flags=true;

#define OFFSETDATA(offset)	*)((void*)((char *)data + (offset)))
#define GIODATA(dtype)	(*(( dtype  OFFSETDATA(gio_table[tableIndex].index)))
#define GIOARRAY(dtype) (*(( dtype OFFSETDATA(gio_table[tableIndex].index + ( (arrayIndex) * sizeof(dtype))) ))

#define GIOPARAMpvoid		(gio_table[tableIndex].pvoid)
#define GIOPARAMpflag_type	(gio_table[tableIndex].pflag_type)
#define GIOPARAMshort		(gio_table[tableIndex].sval)
#define GIOPARAMint			(gio_table[tableIndex].ival)
#define GIOPARAMlong		(gio_table[tableIndex].lval)

#define GIOflags			(gio_table[tableIndex].flags)

#define GIOHEADING				(gio_table[tableIndex].heading)

/**************************************************************************/
// doesn't use the reserved file at this stage
FILE *openFileWrite(char *){
	return NULL;
};

/**************************************************************************/
// Read a number from a file.
long fread_long( FILE *fp )
{
    long number;
	bool sign;
	char c;
	
    do{
		c = getc( fp );
    }
    while ( is_space(c) );
	
    number = 0;
	
	
	sign   = false;
	if ( c == '+' )
	{
		c = getc( fp );
	}
	else if ( c == '-' )
	{
		sign = true;
		c = getc( fp );
	}
	
    if ( !is_digit(c) )
    {
		bug("Fread_long: bad format.");
		logf("non numeric character is '%c'",c);
		do_abort( );
    }
	
    while ( is_digit(c) )
    {
		number = number * 10 + c - '0';
		c      = getc( fp );
    }
	
    if ( sign )
		number = 0 - number;
	
    if ( c == '|' )
		number += fread_long( fp );
    else if ( c != ' ' )
		ungetc( c, fp );
	
    return number;
}
/**************************************************************************/
bool is_stat( const struct flag_type *flag_table );

FILE *saveRecord(gio_type *gio_table, void *data, FILE *fp, int *status){
	int tableIndex, arrayIndex;

	// check if we actually want to skip this record
	tableIndex=0;
	if (gio_table[tableIndex].type==CUSTOM_DONT_SAVE_RECORD){
		int result=((GIO_CUSTOM_RETURN_FUNCTION)GIOPARAMpvoid) (gio_table, tableIndex, data, fp);
		if(result){ // don't save entry
			if(status){
				*status=0; // nothing saved
			}
			return NULL;
		}
	}

	if (!fp){
		logf("saveRecord: fp not initialised");
		fp=openFileWrite("myfile.txt");
	}

	for (tableIndex=0; gio_table[tableIndex].type!=END; tableIndex++){

		// skip fields which don't mean anything in this situation
		if (	gio_table[tableIndex].type==CUSTOM_READ 
			|| gio_table[tableIndex].type==CUSTOM_DONT_SAVE_RECORD)
		{
			continue;
		}

		switch (gio_table[tableIndex].type){
			default:
				logf("saveRecord: unrecognised table type '%s'(%d)", 
					GIOHEADING, gio_table[tableIndex].type);
			case END: 
				break;

			case STR:
				if(!IS_NULLSTR(GIODATA(char *)) || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE))
				{
					hide_tilde(GIODATA(char *));
					fprintf(fp, "%s %s%s~\n", GIOHEADING,
						// put a leading . if string starts with a . or whitespace
						((is_space(*GIODATA(char*))||*GIODATA(char*)=='.')?".":""),
						fix_string(GIODATA(char*)) );
					show_tilde(GIODATA(char*));
				}
				break;

			case _INT:
				if(GIODATA(int) || GIOPARAMint!=0 || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
					fprintf(fp, "%s %d\n", GIOHEADING, GIODATA(int));
				}
				break;
		
			case _LONG:
				if(GIODATA(long) || GIOPARAMlong!=0 || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
					fprintf(fp, "%s %ld\n", GIOHEADING, GIODATA(long));
				}
				break;

			case SHINT:
				if(GIODATA(short)!=0 || GIOPARAMshort!=0 || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
					fprintf(fp, "%s %d\n" , GIOHEADING, GIODATA(short) );
				}
				break;

			case _CHAR:
				if(GIODATA(char) || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
					fprintf(fp, "%s %c\n", GIOHEADING, GIODATA(char));
				}
				break;

			case _BOOL:
				fprintf(fp, "%s %s\n", GIOHEADING, GIODATA(bool)?"true":"false");
				break;

			case WFLAG: // wordflag
				if( GIODATA(int)!=0 || is_stat( GIOPARAMpflag_type) || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
					fwrite_wordflag( GIOPARAMpflag_type, GIODATA(int), GIOHEADING, fp);
				}
				break;

			case SHWFLAG: // short wordflag
				if( GIODATA(short)!=0 || is_stat( GIOPARAMpflag_type) || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
					fwrite_wordflag( GIOPARAMpflag_type, GIODATA(short), GIOHEADING, fp);
				}
				break;

			case STR_ARRAY: // parameter is the max size of the array
				for (arrayIndex=0; arrayIndex<GIOPARAMint; arrayIndex++){
					if (  IS_NULLSTR(GIOARRAY(char*)) || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
						continue;
					}
					hide_tilde(GIOARRAY(char*));
					fprintf(fp, "%s[%d] %s%s~\n", GIOHEADING, arrayIndex, 
						// put a leading . if string starts with a . or whitespace
						((is_space(*GIOARRAY(char*))||*GIOARRAY(char*)=='.')?".":""),
						fix_string(GIOARRAY(char*)));
					show_tilde(GIOARRAY(char*));
				}
				break;

			case STR_ARRAYLIST: // when you reach the first null in the list stop
				for (arrayIndex=0; arrayIndex<GIOPARAMint;arrayIndex++){
					if (IS_NULLSTR(GIOARRAY(char*)) || IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
						break;
					}				
					hide_tilde(GIOARRAY(char*));
					fprintf(fp, "%s[%d] %s%s~\n", GIOHEADING, arrayIndex, 
						// put a leading . if string starts with a . or whitespace
						((is_space(*GIOARRAY(char*))||*GIOARRAY(char*)=='.')?".":""),
						fix_string(GIOARRAY(char*)));
					show_tilde(GIOARRAY(char*));
				}
				break;

			case INT_ARRAY:
				for (arrayIndex=0; arrayIndex<GIOPARAMint; arrayIndex++){
					if(GIOARRAY(int)|| IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
						fprintf(fp, "%s[%d] %d\n", GIOHEADING, arrayIndex, GIOARRAY(int));
					}
				}
				break;

			case LONG_ARRAY:
				for (arrayIndex=0; arrayIndex<GIOPARAMint; arrayIndex++){
					if(GIOARRAY(long)|| IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
						fprintf(fp, "%s[%d] %ld\n", GIOHEADING, arrayIndex, GIOARRAY(long));
					}
				}
				break;

			case SHINT_ARRAY:
				for (arrayIndex=0; arrayIndex<GIOPARAMint; arrayIndex++){
					if(GIOARRAY(short)|| IS_SET(GIOflags,GIOFLAG_ALWAYS_WRITE)){
						fprintf(fp, "%s[%d] %d\n", GIOHEADING, arrayIndex, GIOARRAY(short));
					}
				}
				break;

			case BOOL_ARRAY:
				log_string("BOOL_ARRAY not supported yet");
				break;

			case CUSTOM_WRITE:
	((GIO_CUSTOM_FUNCTION *)GIOPARAMpvoid) (gio_table, tableIndex, data, fp);
				break;
			
			case READ_TO_EOL: // do nothing, since it is a read only code
				break;

			case READ_TO_END_OF_STRING: // do nothing, since it is a read only code
				break;
		}
	}
	fprintf( fp, END_RECORD"\n\n");

	if(status){
		*status=1; // save okay
	}
	return fp;
};

/**************************************************************************/
// memory allocation is only done in here if the data value is NULL
// - fp must be valid and at the point ready to read
// - if data != NULL then we start by zeroing the whole structure
int loadRecord(gio_type *gio_table, void *data, FILE *fp){
	int tableIndex, arrayIndex, structSize;
	char *pStr;
	char inHeader[MIL], inBool[MIL], arrayHeader[MIL];
	bool checkForArray;
	bool data_read=false;
	int result = 0; // 1= okay, 2=eof no data
	bool finished=false;
	bool clearing_with_strdup=false;
	int ret; // used for fscanf result
	char errbuf[MSL]; // used for fscanf error reporting

	if (!fp){
		logf("loadRecord: fp not initialised");
		do_abort();
	}

	// get the total size of the structure we are working with
	for (tableIndex=0; gio_table[tableIndex].type!=END; tableIndex++){
		//loop round till we get the to end of the list... the entry
		// that is of type end contains the total size of the structure
		// in its index value.
	}
	structSize=gio_table[tableIndex].index;

	// create our data node if required
	// doesn't use any form of magic numbers at this stage
	if (!data){
		data=malloc(structSize);
		memset( data, 0, (size_t) structSize);
	}else{ 
		if (strcmp(GIOHEADING, "gio-noclear")
			&& strcmp(GIOHEADING, "strdup_empty")){
			// memset the structure to all zeros before starting if required
			memset( data, 0, (size_t) structSize);
			//	logf("loadRecord: structSize = %d",structSize);
		}else{

			if(!strcmp(GIOHEADING, "strdup_empty")){
				clearing_with_strdup=true;
			}

			// go thru manually clearing the values
			for (tableIndex=0; gio_table[tableIndex].type!=END; tableIndex++)
			{
				switch (gio_table[tableIndex].type)
				{	
				default:
					break;
				case STR:
					if(clearing_with_strdup){
						GIODATA(char*)=str_dup("");
					}else{
						GIODATA(char*)=NULL;
					}
					break;
				case _INT:
					GIODATA(int)= 0;
				break;

				case _LONG:
					GIODATA(long)=0;
				break;
	
				case SHINT:
					GIODATA(short)= 0;
					break;

				case _CHAR:
					GIODATA(char)= '\0';
					break;

				case _BOOL:
					GIODATA(bool)=false;
					break;
		
				case WFLAG: // wordflag
					GIODATA(int)=0;
					break;

				case SHWFLAG: // short wordflag
					GIODATA(short)=0;
					break;

/*			// strdup note supported with these
			case STR_ARRAY: // parameter is the max size of the array
			case STR_ARRAYLIST: // parameter is the max size of the array
			case SHINT_ARRAY: 
			case INT_ARRAY: 
				break;
*/				
				}
			}
		}

		// go thru manually setting all the default integer values
		for (tableIndex=0; gio_table[tableIndex].type!=END; tableIndex++)
		{
			if(gio_table[tableIndex].type==_INT && GIOPARAMint!=0){
				GIODATA(int)= GIOPARAMint;
			}
			if(gio_table[tableIndex].type==SHINT && GIOPARAMshort!=0){
				GIODATA(short)= GIOPARAMshort;
			}			
		}
	}


	// now read in the entry

	ret=fscanf(fp, "%s ", inHeader);
	if(ret!=1){
		sprintf(errbuf, "loadRecord(): Unexpected fscanf result reading inHeader - ret=%d, errno=%d (%s)", ret, errno, strerror(errno));
		bug(errbuf);
		log_string(errbuf);			
	}
	finished= false;

	while (str_cmp(inHeader,END_RECORD) && !finished){
		if(!str_cmp("EOF~", inHeader)){
			if(data_read){
				return 3;
			}else{
				return 2;
			}
		}

		if (feof(fp)){
			bugf("LoadRecord: Unexpected end of file found! inHeader='%s'", inHeader);
			finished= true;			
			return 4;
		}

		if(!str_cmp("end", inHeader)){
			return 1;
		}

		// check on off chance their might be an array header
		if (inHeader[str_len(inHeader)-1]==']'){
			strcpy(arrayHeader,inHeader);
			pStr=strstr( arrayHeader, "[");
			if (!pStr){
				checkForArray=false;
			}else{
				if (sscanf(pStr+1, "%d]", &arrayIndex)==0){
					checkForArray=false;
				}else{
					*pStr='\0';
					checkForArray=true;
				}
			}
		}else{
			checkForArray=false;
		}
		
		// find the match in the GIO table
		// - if you have duplicate entries, the second value will 
		//   not be read
		// - if exact search doesn't match, try prefix matching
		for (tableIndex=0; gio_table[tableIndex].type!=END; tableIndex++){
			if (!str_cmp(inHeader, GIOHEADING) 
				&& gio_table[tableIndex].type!=CUSTOM_WRITE){
				checkForArray=false;
				break;
			}

			if (checkForArray 
				&& !str_cmp(arrayHeader, GIOHEADING) 
				&&( gio_table[tableIndex].type== STR_ARRAY 
				||  gio_table[tableIndex].type== STR_ARRAYLIST
				||  gio_table[tableIndex].type== INT_ARRAY
				||  gio_table[tableIndex].type== LONG_ARRAY
				||  gio_table[tableIndex].type== SHINT_ARRAY
				||  gio_table[tableIndex].type== BOOL_ARRAY
				))
			{
				checkForArray=true;
				data_read=true;
				break;
			}
		}
		if(gio_table[tableIndex].type==END)
		{
			for (tableIndex=0; gio_table[tableIndex].type!=END; tableIndex++){
				if (!str_prefix(inHeader, GIOHEADING) 
					&& gio_table[tableIndex].type!=CUSTOM_WRITE){
					checkForArray=false;
					break;
				}

				if (checkForArray 
					&& !str_cmp(arrayHeader, GIOHEADING) 
					&&( gio_table[tableIndex].type== STR_ARRAY 
					||  gio_table[tableIndex].type== STR_ARRAYLIST
					||  gio_table[tableIndex].type== INT_ARRAY
					||  gio_table[tableIndex].type== LONG_ARRAY
					||  gio_table[tableIndex].type== SHINT_ARRAY
					||  gio_table[tableIndex].type== BOOL_ARRAY
					))
				{
					checkForArray=true;
					break;
				}


			}
		}


		switch (gio_table[tableIndex].type){
			default:
				logf("loadRecord: unrecognised table type %d", gio_table[tableIndex].type);
			case END: 
				logf("loadRecord: Unfound match for '%s' header, ignoring.", inHeader);
				// skip the value - most likely to be a single value
				ret=fscanf(fp, "%s ", inHeader); 
				if(ret!=1){
					sprintf(errbuf, "loadRecord(): Unexpected fscanf result reading inHeader location2 - ret=%d, errno=%d (%s)", ret, errno, strerror(errno));
					bug(errbuf);
					log_string(errbuf);			
				}
				break;

			case STR:
				{
					size_t skip;
					// skip the number of blank spaces in the heading			
					if (str_len(inHeader)<str_len(GIOHEADING)){
						skip=str_len(GIOHEADING)-str_len(inHeader);
						while(skip>0){
							char c=getc(fp);
							if(is_space(c)){ 
								skip--;
							}else{// make sure we don't take too much
								skip=0;
								ungetc(c, fp);
							}				
						}		
					}

					pStr= fread_string(fp);

					char *trim_dot;
					if(*pStr=='.'){ // remove a leading . in a entry if found... saving adds
									// one if the first char is whitespace or a .
						trim_dot=str_dup(pStr+1);
						free_string(pStr);
						pStr=trim_dot;
					}
					show_tilde(pStr);
					GIODATA(char*)= pStr;
				}
				break;

			case _INT:
				GIODATA(int)= fread_number(fp);;	
				break;

			case _LONG:
				GIODATA(long)= fread_long(fp);
				break;

			case SHINT:
				GIODATA(short)= fread_number(fp);
				break;

			case _CHAR:
				GIODATA(char)= fread_letter(fp);
				break;

			case _BOOL:
				ret=fscanf(fp, "%s ", inBool); // read the word
				if(ret!=1){
					sprintf(errbuf, "loadRecord(): Unexpected fscanf result reading inBool - ret=%d, errno=%d (%s)", ret, errno, strerror(errno));
					bug(errbuf);
					log_string(errbuf);			
				}

				if (!str_cmp(inBool, "true")){
					GIODATA(bool)=true;
				}else if (!str_cmp(inBool, "false")){
					GIODATA(bool)=false;
				}else{
					// unmatched - default to false
					logf("loadRecord: BOOL type for '%s' header, unmatched logic '%s' setting to false.", 
						inHeader, inBool);
					GIODATA(bool)=false;
				}
				break;

			case WFLAG: // wordflag
				GIODATA(int)=fread_wordflag( GIOPARAMpflag_type, fp);
				break;

			case SHWFLAG: // short wordflag				
				GIODATA(short)= fread_wordflag( GIOPARAMpflag_type, fp);
				break;

			case STR_ARRAY: // parameter is the max size of the array
			case STR_ARRAYLIST: // same as STR_ARRAY for loading
				pStr= fread_string(fp);

				char *trim_dot;
				if(*pStr=='.'){ // remove a leading . in a entry if found... saving adds
								// one if the first char is whitespace or a .
					trim_dot=str_dup(pStr+1);
					free_string(pStr);
					pStr=trim_dot;
				}
				show_tilde(pStr);
				GIOARRAY(char*)= pStr;
				break;

			case SHINT_ARRAY: 
				GIOARRAY(short)=fread_number(fp);
				break;

			case INT_ARRAY: 
				GIOARRAY(int)= fread_number(fp);
				break;

			case LONG_ARRAY: 
				GIOARRAY(long)= fread_long(fp);
				break;

/*

			case INT_ARRAY:
				log_string("INT_ARRAY not supported yet");
				break;

			case BOOL_ARRAY:
				log_string("BOOL_ARRAY not supported yet");
				break;
*/
			case READ_TO_EOL:
				fread_to_eol(fp);
				break;

			case READ_TO_END_OF_STRING:
				fread_string(fp);
				break;

			case CUSTOM_READ:
	((GIO_CUSTOM_FUNCTION *)GIOPARAMpvoid) (gio_table, tableIndex, data, fp);
				// TO ADD
				break;
		}

		// read in the next header
		ret=fscanf(fp, "%s ", inHeader);
		if(ret!=1){
			sprintf(errbuf, "loadRecord(): Unexpected fscanf result reading inHeader location3 (next header) - ret=%d, errno=%d (%s)", ret, errno, strerror(errno));
			bug(errbuf);
			log_string(errbuf);			
		}

//		logf("inheader='%s", inHeader);
	}

	return result; 
};

/**************************************************************************/
// just flag_lookup but ignores the settable bit
int gioflag_lookup(const char *name, const struct flag_type *flag_table)
{
    int flag;
 
    // first try an exact match, then do a substring match

	// exact
	for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if ( !str_cmp( name, flag_table[flag].name ))
            return flag_table[flag].bit;
    }

	// substring
	for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if ( !str_prefix( name, flag_table[flag].name ))
            return flag_table[flag].bit;
    }

 
    return NO_FLAG;
}

const char *get_flag_table_name( const struct flag_type *flag_table );
/**************************************************************************/
// Kal, June 04
int wordflag_to_value( const struct flag_type *flag_table, char *wf_str)
{
	char *ptr_str;
	char single_word[MIL];
	int result, flag;

	// break down and find words one at a time
	// convert all white spaces into ' ' for one arg first
	ptr_str=wf_str;
    do{
		if (*ptr_str=='\t' || *ptr_str=='\n' || *ptr_str=='\r')
			*ptr_str=' ';
    }while (*++ptr_str);

    // support stat tables
	if ( is_stat( flag_table ) )
    {
		ptr_str=wf_str;
		one_argument( ptr_str, single_word);
		if ( ( result= gioflag_lookup( single_word, flag_table ) ) != NO_FLAG ){
			return result;
		}else{
			logf("wordflag_to_value(): Failed to match value for single word '%s', flag_table=%s.",
				single_word, get_flag_table_name(flag_table));
			logf("Complete string ='%s'", ptr_str);
			return 0;
		}
    }

	// loop thru all the words and match up what we can find
	result=0;
	ptr_str=wf_str;
	while (!IS_NULLSTR(ptr_str)){
		ptr_str = one_argument( ptr_str, single_word);
			
		for (flag = 0; flag_table[flag].name != NULL; flag++)
		{
			if ( single_word[0]==flag_table[flag].name[0]
				&& !str_cmp( single_word, flag_table[flag].name))
			{
				SET_BIT(result, flag_table[flag].bit);
				break;
			}
		}

		// if we didn't find the word in that table - only report
		// it if the word wasn't "none"
		if (!flag_table[flag].name && str_cmp("none",single_word)){
			logf("wordflag_to_value(): unfound word flag match '%s'", single_word);
		}
	}
	return result;

}
/**************************************************************************/
// written by Kalahn - July 98
int fread_wordflag( const struct flag_type *flag_table, FILE *fp)
{
	char *wf_str;
	int result;
	
	// read in the string
	wf_str = fread_string (fp);
	fread_to_eol(fp);

	result=wordflag_to_value(flag_table, wf_str);

	free_string (wf_str);
  
    return (result);
}

/**************************************************************************/
// written by Kalahn - July 98
// note: If the final character in the heading is a space, then it will 
//       not be multiple lined
char *fwrite_wordflag( const struct flag_type *flag_table, 
					  int bits, const char * heading, FILE *fp)
{
	static int i;
    static char buf[5][512];
    int flag;
	int bit_index;
	int bit_value;
	bool bit_found=false;
	bool singlelineformat=false;
	size_t heading_length=str_len(heading);
	
	// rotate buffers
	++i= i%5;
	// initialise the buffer we are working on
    buf[i][0] = '\0';

	if(heading_length>0 && heading[heading_length-1]==' '){
		singlelineformat=true;
	}

	// 2 different modes... if their is a stat table to be written 
	// a different mode is used from checking each bit is written
	if (is_stat( flag_table )){
	    for (flag = 0; flag_table[flag].name && !bit_found; flag++)
	    {
			if ( flag_table[flag].bit == bits ){
				strcat( buf[i], flag_table[flag].name );
				bit_found=true;
			}
		}
		// report that we have a missing value in a table
		if(!bit_found)
		{
			logf("fwrite_wordflag(): VALUE MISSING!!! heading='%s'\n"
				"   (value=%d, 1st table entry='%s')", 
				heading, bits, flag_table[0].name); 
			if(gio_abort_fwrite_wordflag_with_undefined_flags){		
				{	// do autonote missing value
					char body[MSL];
					sprintf(body, "The mud was aborted because for following reason:`1"
						"fwrite_wordflag(): VALUE MISSING!!! heading='%s'`1"
						"   (value=%d, flag_table=%s, 1st table entry='%s')`1"
						"This is usually fixed by adding in a text word for the missing value "
						"(to the table which has a first entry as shown above.)", 
							heading, bits, get_flag_table_name(flag_table),
							flag_table[0].name); 					
					autonote(NOTE_SNOTE, "fwrite_wordflag()",
						"mud aborted due to missing value!", 
						"code", body, true);
					log_note(body);
				}
				do_abort();
			}
		}
	}else{
		// write bits the long way so we can record if a bit isn't written due 
		// to no matching bit in the flag table.
		// Also write only one bit word for a single bit allowing multiple 
		// words for a bit value in a table (colour vs color) and single words in 
		// stored files.
		bool insert_space=false;
		for (bit_index=0; bit_index<32; bit_index++)
		{
			bit_found=false;
			bit_value = 1<<bit_index;

			if (!IS_SET(bit_value, bits))
				continue;

			for (flag = 0; flag_table[flag].name && !bit_found; flag++)
			{
				if ( IS_SET(bit_value, flag_table[flag].bit) )
				{
					if(singlelineformat){ 
						if(insert_space){
							strcat( buf[i], " ");
						}
						insert_space=true;
					}else{
						strcat( buf[i], "\n\t");
					}
					strcat( buf[i], flag_table[flag].name );
					bit_found= true;
				}
			}

			// report that we have a missing bit in a table
			if(!bit_found)
			{
				logf("fwrite_wordflag(): BIT MISSING!!! '%c' heading='%s'\n"
					"   (bit_index=%d, bit_value=%d, flag_table=%s, 1st table entry='%s')", 
					(bit_index <= 'Z' - 'A'?'A' + bit_index :
						'a' + bit_index - ( 'Z' - 'A' + 1 )), heading, 
					bit_index, bit_value, get_flag_table_name(flag_table),
					flag_table[0].name); 
				if(gio_abort_fwrite_wordflag_with_undefined_flags){
					{	// do autonote missing bit
						char body[MSL];
						sprintf(body, "The mud was aborted because for following reason:`1"
							"fwrite_wordflag(): BIT MISSING!!! '%c' heading='%s'\n"
							"   (bit_index=%d, bit_value=%d, 1st table entry='%s')"
							"This is usually fixed by adding the missing word for the bitflag."
							"(to the table which has a first entry as shown above.)", 
							(bit_index <= 'Z' - 'A'?'A' + bit_index :
								'a' + bit_index - ( 'Z' - 'A' + 1 )), heading, 
							bit_index, bit_value, flag_table[0].name); 

						autonote(NOTE_SNOTE, "fwrite_wordflag()",
							"mud aborted due to a missing bitflag!", 
							"code", body, true);
						log_note(body);
					}

					do_abort();
				}
			}
		}
	} // non stat table (multiple bit table)
	
	strcat( buf[i], "~\n" );

	if(fp && str_len(buf[i])>2){ // only write something if we have something to write
		if(singlelineformat){
			fprintf(fp, "%s%s", heading, buf[i]);
		}else{
			fprintf(fp, "%s %s", heading, buf[i]);
		}		
	}
    return (buf[i]);
}
/**************************************************************************/
// gio_generic_savelist returns true if no errors on save
bool gio_generic_savelist( void *listpointer, gio_type *gio_table, 
						  const char *filename, int nextoffset, bool backup)
{
	FILE *fp;
	void *pdata;
	int count=0;
	int skipped=0;
	char write_filename[MSL];

	sprintf(write_filename,"%s.write", filename);

	// find a free write filename - so if something stuffed up on 
	// the write, we don't go over it.
	if(file_exists(write_filename)){
		for(int i=0; i<20; i++){
			sprintf(write_filename,"%s.write%d", filename, i);
			if(!file_exists(write_filename)){
				break;
			}
		}
	}

	unlink(write_filename);
	logf("Starting gio_generic_savelist to %s.", 
		write_filename);
	fclose( fpReserve );

    if ( ( fp = fopen( write_filename, "w" ) ) == NULL )
    {
		bugf("gio_generic_savelist(): fopen '%s' for write - error %d (%s)",
			write_filename, errno, strerror( errno));
		bugf("An error occurred! writing to '%s' which would become %s", write_filename,
			filename);
		fpReserve = fopen( NULL_FILE, "r" );
		return false;
    }

	// Go thru linked list writing each node to disk
	for ( pdata=listpointer; pdata; pdata= *(void **)(((char*) pdata )+nextoffset))
	{
		int status;
		saveRecord( gio_table, pdata, fp, &status); 
		if(status){
			count++;
		}else{
			skipped++;
		}
	}

	int bytes_written=fprintf(fp, "EOF~\n");
	fclose( fp );
	if(   bytes_written != str_len("EOF~\n") ){
		bugf("gio_generic_savelist(): fprintf to '%s' incomplete - error %d (%s)",
			write_filename, errno, strerror( errno));
		bugf("Incomplete generic io write of %s, write aborted - check diskspace!", write_filename);
		{
			char msgbody[MSL];
			sprintf(msgbody,"Incomplete write of %s, write aborted - check diskspace!\r\n", write_filename);
			autonote(NOTE_SNOTE, "gio_generic_savelist()", "Problems saving a linked list using gio!", "code cc: imm", 
				msgbody, true);
		}
	}else{
		// handle backup option
		if(backup){
			char buf[MIL];
			sprintf(buf, "%s.giobak", filename);
			logf("GIOBACKUP: Renaming old %s to %s", filename, buf);
			unlink(buf);
			rename(filename, buf);
		}

		logf("Renaming new %s to %s", write_filename, filename);
		unlink(filename);
		rename(write_filename, filename);
	}

	fpReserve = fopen( NULL_FILE, "r" );
	if(skipped){
		logf("Finished gio_generic_savelist [%d], skipped writing %d.", count, skipped);
	}else{
		logf("Finished gio_generic_savelist [%d].", count);
	}
	return true;
}
/**************************************************************************/
void * gio_generic_loadlist( int size, gio_type *gio_table, 
							const char *filename, int nextoffset)
{
	FILE *fp;
	void *phead;
	void *pdata=NULL;
	void *plast=NULL;
	int count=0;

	if(!file_exists(filename)){
		logf("File '%s' not found, cancelled gio list loading.", filename);
		return NULL;
	}

	logf("gio_generic_loadlist() reading in from %s... ", filename);
	fclose( fpReserve );

    if ( ( fp = fopen( filename, "r" ) ) == NULL )
    {
		bugf("gio_generic_loadlist(): fopen '%s' for read - error %d (%s)",
			filename, errno, strerror( errno));
		bugf("An error occurred! reading from %s", filename);
		fpReserve = fopen( NULL_FILE, "r" );
		return NULL;
    }

	// Go thru linked list reading each node from disk
	plast=pdata;
	pdata=malloc(size);
	phead=pdata;
	memset( pdata, 0, (size_t) size);

	while (loadRecord(gio_table,pdata,fp)<2)
	{
		plast=pdata;
		pdata=malloc(size);
		*(void **)(((char*) plast )+nextoffset)=pdata;
		memset( pdata, 0, (size_t) size);
		count++;
	}
	if(plast)
	{
		*(void **)(((char*) plast )+nextoffset)=NULL;
	}
	fclose( fp );

	fpReserve = fopen( NULL_FILE, "r" );

	logf("Finished gio_generic_loadlist [%d].", count);

	return phead;
}
/**************************************************************************/
/**************************************************************************/

