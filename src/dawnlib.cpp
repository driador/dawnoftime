/**************************************************************************/
// dawnlib.cpp - a library of functions, some helping in portablity
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
/**************************************************************************/
// redirect to str_len(), check the result to make sure we don't do a 
// signed overflow
int str_len(const char *s)
{
	size_t l=strlen(s);
	int r=(int)l;
	assert(r>=0);
	return r;
}

/**************************************************************************/
// redirect to isalpha
int is_alpha( char c )
{
	unsigned char uc=c;
	return isalpha((int)uc);
}
/**************************************************************************/
// redirect to isalnum
int is_alnum( char c )
{
	unsigned char uc=c;
	return isalnum((int)uc);
}
/**************************************************************************/
// redirect to isascii
int is_ascii( char c )
{
	unsigned char uc=c;
	return isascii((int)uc);
}
/**************************************************************************/
// redirect to isdigit
int is_digit( char c )
{
	unsigned char uc=c;
	return isdigit((int)uc);
}
/**************************************************************************/
// redirect to islower
int is_lower( char c )
{
	unsigned char uc=c;
	return islower((int)uc);
}
/**************************************************************************/
// redirect to isprint
int is_print( char c )
{
	unsigned char uc=c;
	return isprint((int)uc);
}
/**************************************************************************/
// redirect to isspace
int is_space( char c )
{
	unsigned char uc=c;
	return isspace((int)uc);
}
/**************************************************************************/
// redirect to isupper
int is_upper( char c )
{
	unsigned char uc=c;
	return isupper((int)uc);
}
/**************************************************************************/
// Kal, Oct2003
void manage_dynamic_buffer(char **result, int max_length)
{	
	int allocated_length=max_length + sizeof(int)*2 + sizeof(int)+1; 
		// "sizeof(int)+1" is safety padding for '\0' and if there is misuse
	const int magic_num=0x1234ABCD; // used to detect memory corruption

	// store the max length and a 'magic' number
	// MAGIC_NUMBER (sizeof(int)), EXISTING MAX LENGTH (sizeof(int))
	// then the memory for what we are dealing with
	assertp(result); // we should never be given a null result char ** pointer
	
	if(*result!=NULL){ // we can only check previous allocations if there have been some

		int previous_magic_num=*((int*)(*result-(sizeof(int)*2)));
		int previous_max_length=*((int*)(*result-sizeof(int)));

		if(previous_magic_num!=magic_num){
			bugf("MDB(): previous_magic_num=%d, magic_num=%d... "
				"they should the same... this indicates memory corruption! - aborting!",
				previous_magic_num, magic_num);
			do_abort();
		}

		if(previous_max_length>max_length){

			return; // we don't have to do anything, since we have already enough memory allocated
		}

		// we need to reallocate things
		// first deallocate the previous memory
		delete ((*result)-sizeof(int)*2);
	}

	// allocate the memory
	char *new_allocation =new char[allocated_length];
	assertp(new_allocation);

	// set the magic number
	*((int*)new_allocation)=magic_num;

	// set the max_length
	*((int*)(new_allocation +sizeof(int)))=max_length;

	// set the new result value, jumping over our two hidden integers
	*result=new_allocation +sizeof(int)*2;
}
/**************************************************************************/
// str_dup based - Kal Oct 2003
char *trim_trailing_carriage_return_line_feed(char *str)
{
	if(IS_NULLSTR(str)){
		return str_dup("");
	}
	char *result=str;

	// trim off up to two trailing \n\r characters
	if(result[str_len(result)-1]=='\n' || result[str_len(result)-1]=='\r'){
		result[str_len(result)-1]='\0';
		if(result[str_len(result)-1]=='\n' || result[str_len(result)-1]=='\r'){
			result[str_len(result)-1]='\0';
		}
		str=str_dup(result);
		free_string(result);
		result=str;
	}	

	return result;
}

/**************************************************************************/
/**************************************************************************/

