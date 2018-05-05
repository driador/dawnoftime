/**************************************************************************/
// websrv.h - 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef WEBSRV_H
#define WEBSRV_H


// #define VALIDATE_HTML
// Validate HTML is turned on when pages are to be put 
// thru an HTML validator.... leave it off because otherwise 
// the HTML code uses <TT> and 'nbsp' (non breaking spaces) to
// format the code instead of <PRE> (since <FONT COLOR=".."> 
// sections shouldn't be within a <PRE> section

#define WEBIN_HEADER_BUF	 7000
#define WEBIN_BODY_BUF		30000

#ifdef VALIDATE_HTML
#define WEBOUT_HEADER_BUF	 50000
#define WEBOUT_BODY_BUF		100000
#else
#define WEBOUT_HEADER_BUF	  5000
#define WEBOUT_BODY_BUF		 20000
#endif

#define MAXLOCATION		50
#define MAXPASSLENGTH	20
#define MAXTITLE		100

#ifndef DOCTYPE
#define DOCTYPE "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n"
#endif

// prototypes
char * decodeBase64(char *);

enum requestTypes { UNDEFINED, GET, HEAD, POST };
enum contentTypes { CONTENT_TYPE_HTML, CONTENT_TEXT_PLAIN };

struct web_request_data{
	bool receivingHeader;
	char inHeaderBuf [WEBIN_HEADER_BUF];
	int inHeaderLen;
	char firstline[1024];
	char referer[1024];
	char user_agent[1024];
	char inBodyBuf [WEBIN_BODY_BUF];
	int inBodyLen;
	char outHeaderBuf[WEBOUT_HEADER_BUF];
	int outHeaderLen;
	char outBodyBuf[WEBOUT_BODY_BUF];
	int outBodyLen;
	int IPnum;
	int contentLength;
	contentTypes contentType;
	// web specific data
	char username[MAXPASSLENGTH+1];	
	char password[MAXPASSLENGTH+1];

	// HTTP version
	char version[MAXPASSLENGTH+1];
	int statusCode;
//	int HTTPvMajor;
//	int HTTPvMinor;

	// requestinfo
	requestTypes requestType;
	char requestedLocation[MAXLOCATION+1];
};


#ifndef UMIN
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
#endif

// stuff from the muds code
char *one_argument( const char *argument, char *arg_first );
bool str_cmp( const char *astr, const char *bstr );
bool str_prefix( const char *astr, const char *bstr );

#endif // WEBSRV_H
