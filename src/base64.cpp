/**************************************************************************/
// base64.cpp - 
/**************************************************************************/
/*
*  Base64 decoder/decoder v0.3 - written by Kalahn (c)1998-2002
*
*  Only decodes base64 strings less than 4KB as this was written
*  to decode username:password info used in HTTP/1.0 as per RFC 1945
*  wouldn't be hard to make it support any length... just wasn't 
*  required for what I wrote it for.
*
*  This function now also handles url encoding.
* 
*  Use however you like just keep the copyright notice in file.
*  #include <std.disclaimer>
*/
/**************************************************************************/
#include "websrv.h"
#include "include.h"

/**************************************************************************/
static int invalid;
/**************************************************************************/
char base64code[64]=
{
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
	'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
	'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
	'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
};
/**************************************************************************/
// takes a base64 character, returns its value
// from RFC 2045 Table 1 - The Base64 Alphabet
int getBase64Value(char code){

    int val= (int)code;
    int result;

    if (val>='A' && val<='Z')
        result= val-65;
    else if (val>='a' && val<='z')
        result= val-71;
    else if (val>='0' && val<='9')
        result= val+4;
    else if (val=='+')
        result=62;
    else if (val=='/')
        result=63;
    else if (val=='=')
        result=0;
    else{
        bugf("Invalid non base64 character %c (%d)", code, code);
		invalid=true;
        return 0;
    }
    return result;
}
/**************************************************************************/
char * decodeBase64(char *coded_with_linebreaks)
{
    int i;
	int j=0;
	static char result[4100];
	char d[4100];
	char *s, *coded;
	invalid=false;

	i=str_len(coded_with_linebreaks);
	if (i>4096){
		bugf("base64 string to long to be a password.\n");
		return NULL;
	}

	// copy coded_with_linebreaks, over to coded without newlines or spaces
	s=coded_with_linebreaks;
	coded=d;
	while(*s){
		if(!is_space(*s) && *s!='\n' && *s!='\r' ){
			*coded++=*s++;
		}else{
			s++;
		}

	}
	*coded='\0';

	i=str_len(coded);
	if (i%4!=0){
		bugf("Not a valid base64 string - incorrect length.\n");
		return NULL;
	}

	coded=d;

	// go thru converting each 4 base64 bytes into 3 8bit bytes		
    for (; coded[0]!='\0'; coded+=4){

		result[j++] = (char)(getBase64Value(coded[0])<<2
			| getBase64Value(coded[1])>>4);
		result[j++] = (char)(getBase64Value(coded[1])<<4
			| getBase64Value(coded[2])>>2);
		result[j++] = (char)(getBase64Value(coded[2])<<6 
			| getBase64Value(coded[3]));
	}
    

	if(invalid)
		return NULL;

	result[j]=0;

	if (*(--coded)=='='){
		if (j>0){
			if (*(--coded)=='='){
				result[j-2]='\0';
			}else{
				result[j-1]='\0';
			}
		}
	}

	return (result);
}
/**************************************************************************/
char * encodeBase64(char *plaintext, int len)
{
	static char result[4100];
	unsigned char *pt=(unsigned char *)plaintext;
	int i;
	int j=0;
	int bytes;

	if(len<0){
		len=str_len(plaintext);
	}

	len-=2;

	// go thru converting every 3 bytes into 4 base64 bytes
	for (i=0; i < len; i+=3){
		bytes= (pt[i]<<16)
			 + (pt[i+1]<<8)
			 + (pt[i+2]);

		result[j++] = base64code[(bytes>>18) & 0x3F];
		result[j++] = base64code[(bytes>>12) & 0x3F];
		result[j++] = base64code[(bytes>>6) & 0x3F];
		result[j++] = base64code[(bytes) & 0x3F];

		if(i%57==54){
			result[j++]='\r';
			result[j++]='\n';
		}
	}

	// pad the remaining characters
	switch((i-len)%3){
		case 2: // exact fit
			break;

		case 0: // two more characters to save
			bytes= (pt[i]<<16) + (pt[i+1]<<8);
			result[j++] = base64code[(bytes>>18) & 0x3F];
			result[j++] = base64code[(bytes>>12) & 0x3F];
			result[j++] = base64code[(bytes>>6) & 0x3F];
			result[j++] = '=';
			break;

		case 1: // one more character to save
			bytes= pt[i]<<16;
			result[j++] = base64code[(bytes>>18) & 0x3F];
			result[j++] = base64code[(bytes>>12) & 0x3F];
			result[j++] = '=';
			result[j++] = '=';
			break;
			break;

	}
	result[j]='\0';
    

	return (result);
}
/**************************************************************************/
const char *url_encode_table[]={ // as per RFC1728
	"%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
	"%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
	"%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
	"%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
	  "+",   "!", "%22", "%23",   "$", "%25", "%26", "%27",
	  "(",   ")",   "*", "%2B",   ",",   "-",   ".", "%2F",
	  "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
	  "8",   "9", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
	"%40",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
	  "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
	  "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
	  "X",   "Y",   "Z", "%5B", "%5C", "%5D", "%5E",   "_",
	"%60",   "a",   "b",   "c",   "d",   "e",   "f",   "g",
	  "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
	  "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
	  "x",   "y",   "z", "%7B", "%7C", "%7D", "%7E", "%7F",
	"%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
	"%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
	"%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
	"%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
	"%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
	"%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
	"%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
	"%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
	"%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
	"%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
	"%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
	"%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
	"%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
	"%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
	"%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
	"%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};
/**************************************************************************/
char *url_encode_post_data(char *postdata)
{
	static char *result; // managed result buffer
	// nothing to do with empty strings
	if( IS_NULLSTR(postdata)){
        return "";
	}
	manage_dynamic_buffer(&result, str_len(postdata)*3+1); // maintain result so always has enough space

	char *d=result; // dest
	unsigned char *s; // src
	const char *t; // text

	for(s=(unsigned char*)postdata; *s; s++){
		t=url_encode_table[*s];
		while(*t){
			*d++=*t++;
		}	
	}
	*d='\0'; // terminate the result
	return result;
}
/**************************************************************************/

