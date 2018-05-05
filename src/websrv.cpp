/**************************************************************************/
// websrv.cpp - Dawn Integrated webserver 
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/

#include "network.h"
#include "websrv.h"
#include "include.h"
#include "nanny.h"
#include "colour.h"
#include "gameset.h"
#include "bitflags.h"

#ifndef EWOULDBLOCK
#define EWOULDBLOCK 35
#endif

#define LOGALLWEB false
#define IS_NULLSTR(str)   ((str)==NULL || (str)[0]=='\0')

// prototypes
void logWebf (char * fmt, ...);
void logWeb( const char *str );
void filterWebRequest(connection_data *w);
int is_space( char c );

void closeConnection(web_request_data *w);

bool readFromWebConnection ( web_request_data *w );

extern int webHits;
extern int webHelpHits;
extern int webWhoHits;
//mud2web.cpp
char *getwebAreas(int level);
char *getwebWho(int level);
char *getwebWizlist();
char *getwebSockets(int level);
char *getwebMudstats(int level);
char *getwebClassStats();
char *getwebRaceStats();
char *getwebRaceInfo();
char *getwebClassInfo(char *racename);
char *getwebAlignStats();
char *getwebLastonImm(int level);
char *getwebHelp(char *, char *entry_buf, int length);
char *getwebGameSettings();

// laston.cpp
int laston_web_level(char *username, char *password);

// handler.cpp
void append_string_to_file( const char *file, const char *str, bool newline );

// dawnlib.cpp
int str_len(const char *s);

/**************************************************************************/
void badRequestFormat(web_request_data *w, char * reason){
	w->outBodyLen=sprintf(w->outBodyBuf, "badRequestFormat: %s\r\n", reason);
	return;
}
/**************************************************************************/
void processHTTP0_9Request(web_request_data *w){
	w->outBodyLen=sprintf(w->outBodyBuf,"HTTP/0.9 is not supported (or bad request)\r\n");
	return;
}
/**************************************************************************/
// returns true if there was an error
bool processWebHeader(web_request_data *w){
// NEEDS TO PROCESS THE HEADER FROM pCRLFCRLF
	char *pStr, *pLine;
	char workingline[WEBIN_HEADER_BUF+100];
	char method[21];

	// TODO - tidy up check request isn't too long
	if (w->inHeaderLen> WEBIN_HEADER_BUF){
		badRequestFormat(w, "Request-Line too long");
		return false;
	}

	strncpy(workingline, w->inHeaderBuf, w->inHeaderLen);
	workingline[w->inHeaderLen]='\0';
	pLine=workingline;

	// copy parameters, checking the 
	// length of each command as we go

	// extract the METHOD
	pStr=strstr( pLine, " ");
	if (!pStr){
		badRequestFormat(w, "Request-Line invalid");
		return true;
	}
	if (pStr-pLine>20){
		badRequestFormat(w, "method in Request-Line too long");
		return true;
	}
	strncpy(method, pLine, pStr-pLine);		
	method[pStr-pLine]='\0';
	pLine=pStr+1;

	// extract the LOCATION
	pStr=strstr( pLine, " ");
	if (!pStr){
		badRequestFormat(w, "Request-Line invalid");
		return true;
	}
	if (pStr-pLine>MAXLOCATION){
		badRequestFormat(w, "Request-URI (location) in Request-Line too long");
		return true;
	}
	strncpy(w->requestedLocation, pLine, pStr-pLine);
	w->requestedLocation[pStr-pLine]='\0';
	pLine=pStr+1;


	// extract the VERSION
	if (strstr( pLine, "HTTP/" )==NULL)
	{
		badRequestFormat(w, "no HTTP version in Request-Line");
	}
	if (str_len(pLine)>20){
		badRequestFormat(w, "HTTP-Version in Request-Line too long");
		return true;
	}	
	strcpy(w->version, pLine);

	// now identify the method
	if (!strcmp(method,"GET")){
		w->requestType=GET;
	}else
	if (!strcmp(method,"HEAD")){
		w->requestType=HEAD;
	}else
	if (!strcmp(method,"POST")){
		w->requestType=POST;
	}else{
		w->requestType=UNDEFINED;
	}

	// check for an "Authorization:" header
	pLine=strstr( w->inHeaderBuf, "Authorization:");
	if (pLine){
		// we have an Authorization - process it
		pStr=strstr( pLine, "\r\n");
		if (pStr){
			strncpy(workingline, pLine, pStr-pLine);
			workingline[pStr-pLine]='\0';
			pLine=workingline;

			//we have the authorization string in newline
			logWebf("Processing Authorization: '%s'",workingline);

			pStr=strstr( pLine, "Basic ");

			if (pStr){
				pLine=pStr+6;

				if (str_len(pLine)>0 && str_len(pLine)<200){
					char decoded[200];
					char *d;

					d =decodeBase64(pLine);
					if (d){
						strcpy(decoded,d);						
						// now separate the username:password
						pLine=decoded;
						pStr=strstr( pLine, ":");
						if (pStr){
							if (pStr-pLine<MAXPASSLENGTH){
								strncpy(w->username, pLine, pStr-pLine);
								w->username[pStr-pLine]='\0';
								pLine=pStr+1;

								if (str_len(pLine)>MAXPASSLENGTH){
									// not a valid password - so authenticate invalid
									w->username[0]='\0';
								}else{
									strcpy(w->password, pLine);
								}
							}								
						}						
					}
				}							
			}else{
				logWeb("Authorization type not recognised");
			}
		}
	}
	return true;
}
/**************************************************************************/
void greet_http(connection_data *c) 
{
	logWebf("New web connection from %s", c->remote_ip);

	web_request_data *w=new web_request_data;
	w->receivingHeader=true;
	w->inHeaderBuf[0]='\0';
	w->inHeaderLen=0;
	w->inBodyBuf[0]='\0';
	w->inBodyLen=0;
	w->outHeaderBuf[0]='\0';
	w->outHeaderLen=0;
	w->outBodyBuf[0]='\0';
	w->outBodyLen=0;
	w->contentLength=0;
	// set some defaults
	w->username[0]='\0';
	w->password[0]='\0';
	w->requestType= UNDEFINED;
	w->contentLength=0;// request message doesn't have a body
	w->statusCode=503; // service unavailable
	w->contentType= CONTENT_TYPE_HTML;

	c->web_request=w;
}

/**************************************************************************/
// return true if everything is okay - i.e. we read something or are 
//                                          waiting for something.
bool websrv_process_input(connection_data *c)
{
	int bytesIn;
	char *pDest;
	int maxDestLength;
	int *currentLength;

	if (c->web_request->receivingHeader){
		maxDestLength =WEBIN_HEADER_BUF-c->web_request->inHeaderLen;
		pDest = &c->web_request->inHeaderBuf[c->web_request->inHeaderLen];
		currentLength=&c->web_request->inHeaderLen;
	}else{
		maxDestLength=WEBIN_BODY_BUF-c->web_request->inBodyLen;
		pDest = &c->web_request->inBodyBuf[c->web_request->inBodyLen];
		currentLength=&c->web_request->inBodyLen;
	}

	bytesIn=recv(c->connected_socket, pDest, maxDestLength,0);
	
	if (bytesIn>0){
		logWeb("Getting input");
		*(pDest+bytesIn)='\0';
		*currentLength+=bytesIn;

		if(LOGALLWEB){
			logWeb(c->web_request->inHeaderBuf);
		}
		if (c->web_request->inHeaderLen>=WEBIN_HEADER_BUF){
			logWebf("Buffer overflow from %s",c->remote_ip);
			logWeb(c->web_request->inHeaderBuf);
		}
		filterWebRequest(c);
	}else{
		// connection closed at the other end
		logWebf("closing connection - 0 bytes in (%d).", bytesIn);
		connection_close( c );
		return false;
	}
	return true;
}
/**************************************************************************/
void websrv_process_output(connection_data *c)
{
	if (c->web_request->outBodyLen>0){
		send(c->connected_socket, 
			&c->web_request->outHeaderBuf[0], 
			c->web_request->outHeaderLen,
			0);

		send(c->connected_socket, 
			&c->web_request->outBodyBuf[0], 
			c->web_request->outBodyLen,
			0);
		
		logWeb("closing connection - sent data.");
		connection_close(c);
	}
}
/**************************************************************************/



/**************************************************************************/
/*
 * Writes a string to the log.
 */
void logWeb( const char *str )
{
	logWebf("%s", str);
    return;
}
/**************************************************************************/
// create a HTTP/1.0 header
void createHeader(web_request_data *w){
	char reason[50];
	time_t ltime;
	char GMTdate[100];
	
	// get the date first
	time( &ltime );
	strftime(GMTdate, 99, "%A, %d-%b-", gmtime( &ltime ));

	char t2buf[20]; // budget fix to get rid of y2k compiler warning
	strftime(t2buf, 20, "%Y %T GMT", gmtime( &ltime ));
	strcat(GMTdate,&t2buf[2]);


	
	switch (w->statusCode){
	// 2xx: Success 
	case 200: strcpy(reason, "OK");			break;
	case 201: strcpy(reason, "Created");		break;
	case 202: strcpy(reason, "Accepted");		break;
	case 204: strcpy(reason, "No Content");	break;
	// 3xx: Redirection
	case 301: strcpy(reason, "Moved Permanently");	break;
	case 302: strcpy(reason, "Moved Temporarily");	break;
	case 304: strcpy(reason, "Not Modified");		break;
	// 4xx: Client Error
	case 400: strcpy(reason, "Bad Request");	break;
	case 401: strcpy(reason, "Unauthorized");	break;
	case 403: strcpy(reason, "Forbidden");		break;
	case 404: strcpy(reason, "Not Found");		break;
	// 5xx: Server Error
	case 500: strcpy(reason, "Internal Server Error");	break;
	case 501: strcpy(reason, "Not Implemented");		break;
	case 502: strcpy(reason, "Bad Gateway");			break;
	case 503: strcpy(reason, "Service Unavailable");	break;

	default:
		w->statusCode=500;
		strcpy(reason, "Internal Server Error");	break;
	}

	w->outHeaderLen= sprintf(w->outHeaderBuf, 
		"HTTP/1.0 %d %s\r\n"
		"Date: %s\r\n"
		"WWW-Authenticate: Basic realm=\"%s Integrated WebServer\"\r\n"
		"Cache-Control: private, pre-check=0, post-check=0, max-age=0\r\n"
		"Expires: %s\r\n"
		"Last-Modified: %s\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %d\r\n"
		"\r\n"
		,w->statusCode, reason, GMTdate,
		MUD_NAME, 
		GMTdate,
		GMTdate,
		w->contentType==CONTENT_TYPE_HTML?"text/html":"text/plain",
		w->outBodyLen);
}
/**************************************************************************/
char *websvr_generate_head_and_start_body(char *title)
{
	static char head[8192];
	sprintf(head, 
		DOCTYPE
		"<HTML><HEAD>\n"		
		"<TITLE>Dawn - %.8000s</TITLE>\n"
		"<link rel=\"StyleSheet\" href=\"/stat.css\" type=\"text/css\" />\n"
		"</HEAD>\n"
		"\n"
		"<BODY BGCOLOR=\"#000000\" TEXT=\"#BFBFBF\">\n",
		title);

	return head;
}

/**************************************************************************/
void websvr_handle_insufficient_immweb_level(web_request_data *w, char *buf, int immweb_level)
{
	if (immweb_level==-1){
		w->statusCode=200;
		buf=process_colour("`WYou haven't set a webpassword for your character,`1"
			"Use the webpass command within the game to do this.", NULL);
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
			"%s" // doctype, header, and start of body tag
			"<PRE><FONT>%s</PRE>"
			"<p><A HREF=\"/\">[Back to root]</A> <A HREF=\"/sockets\">[sockets]</A> <A HREF=\"/mudstats\">[mudstats]</A>"
			,websvr_generate_head_and_start_body("web password setting required!")
			,buf);
	}else{
		w->statusCode=401;
		logWeb("401 - Authorisation denied");
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
			"<html><head><title>Incorrect username/password</title></head>\n"
			"<body><h1>Incorrect username/password</h1>\n"
			"<p>You got the username or password incorrect.\n"
			"<p><A HREF=\"/\">Back to root</A>");
	}
}
/**************************************************************************/
// This function is the most ugly piece of code I have written in my 
// life, it was a hack to get things working quickly then evolved
// into this hideous beast - Kal, Oct 2001.
void processGET(web_request_data *w)
{
	char *buf="";

	webHits++;
	if(!str_cmp(w->requestedLocation,"/stat.css")){
		// stats.css - the style sheet 
		w->statusCode=200;
		w->contentType=CONTENT_TEXT_PLAIN;
		strcpy(w->outBodyBuf, IS_NULLSTR(game_settings->style_sheet)?"":game_settings->style_sheet);
		w->outBodyLen=str_len(w->outBodyBuf);
	}else
	if (!str_cmp(w->requestedLocation,"/areas")){
		webWhoHits++;
		w->statusCode=200;
		buf=getwebAreas(20);
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
			"%s" // doctype, header, and start of body tag
			"<H2><FONT COLOR=\"#BFBFBF\">Areas within %s</FONT></H2>"
			"\n<PRE><FONT>%s"
			"  [<A HREF=\"/\">back</A>]\n"
			"</PRE>"
			,websvr_generate_head_and_start_body("Areas")
			,MUD_NAME
			,buf);
	}else
	if (!str_cmp(w->requestedLocation,"/who")){
		webWhoHits++;
		w->statusCode=200;
		buf=getwebWho(20);
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
			"%s" // doctype, header, and start of body tag
			"<H2><FONT COLOR=\"#BFBFBF\">Who is online %s</FONT></H2>"
			"\n<PRE><FONT>%s"
			"  [<A HREF=\"/\">back</A>]\n"
			"</PRE>"
			,websvr_generate_head_and_start_body("Wholist")
			,MUD_NAME
			,buf);
	}else
	if (!str_cmp(w->requestedLocation,"/mudstats")){
		w->statusCode=200;
		buf=getwebMudstats(20);
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
			"%s" // doctype, header, and start of body tag
"<p><A HREF=\"http://www.dawnoftime.org/\">Go to the official Dawn of Time website</A>\n"
"   <A HREF=\"/\">Back to mud integrated website</A>\n"
			"</p>\n<H2><FONT COLOR=\"#BFBFBF\">%s Mudstats</FONT></H2>"
#ifdef VALIDATE_HTML
			"\n<TT><FONT>%s</FONT></TT>"
#else
			"\n<PRE><FONT>%s</PRE>"
#endif
"<p><A HREF=\"http://www.dawnoftime.org/\">Go to the official Dawn of Time website</A>\n"
"   <A HREF=\"/\">Back to mud integrated website</A>\n"
,websvr_generate_head_and_start_body("Mudstats")
,MUD_NAME ,buf);
	}else
	if (!str_cmp(w->requestedLocation,"/robots.txt")){ 
		// robots.txt - to prevent webcrawlers indexing the online helps
		w->statusCode=200;
		w->contentType=CONTENT_TEXT_PLAIN;
		strcpy(w->outBodyBuf, 
"User-agent: *\n"
"Disallow: /help/\n"
"Disallow: /stat.css\n"
"Disallow: /areas/\n"
"Disallow: /who/\n"
"Disallow: /mudstats/\n"
"Disallow: /robots.txt/\n"
"Disallow: /gamesettings/\n"
"Disallow: /wizlist/\n"
"Disallow: /classstats/\n"
"Disallow: /racestats/\n"
"Disallow: /raceinfo/\n"
"Disallow: /alignstats/\n"
"Disallow: /immwho/\n"
"Disallow: /lastonimm/\n"
"Disallow: /sockets/\n"
"Disallow: /locked/\n"
"Disallow: /locked2/\n"
);
		w->outBodyLen=str_len(w->outBodyBuf);
	}else
	if (!str_cmp(w->requestedLocation,"/gamesettings")){
		w->statusCode=200;
		buf=getwebGameSettings();
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
			"%s" // doctype, header, and start of body tag
			"<A HREF=\"/\">Back to root</A><BR>\n"
			"<H2><FONT COLOR=\"#BFBFBF\">%s GAME SETTINGS</FONT></H2>"

#ifdef VALIDATE_HTML
			"\n<TT><FONT>%s</FONT></TT>"		
#else
			"\n<PRE><FONT>%s</PRE>"
#endif
			"<p><A HREF=\"/\">Back to root</A>"
			,websvr_generate_head_and_start_body("GameSettings")
			,MUD_NAME, buf);
	}else
	if (!str_cmp(w->requestedLocation,"/wizlist")){
		w->statusCode=200;
		buf=getwebWizlist();
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
			"%s" // doctype, header, and start of body tag
			"<A HREF=\"/\">Back to root</A><BR>\n"
			"<H2><FONT COLOR=\"#BFBFBF\">%s WIZLIST</FONT></H2>"

#ifdef VALIDATE_HTML
			"\n<TT><FONT>%s</FONT></TT>"		
#else
			"\n<PRE><FONT>%s</PRE>"
#endif
			"<p><A HREF=\"/\">Back to root</A>"
			,websvr_generate_head_and_start_body("Wizlist")
			,MUD_NAME, buf);
	}else
	if (!str_prefix("/help/",w->requestedLocation) 
		|| !str_cmp("/help",w->requestedLocation))
	{
		webHelpHits++;
		char entry_buf[10000];
		char entry_buf2[10050];
		char *webhelp_buf=(char*)malloc(128000);// temporary storage allocated
		buf=webhelp_buf;
		w->statusCode=200;

		if(w->requestedLocation[5]=='/'){
			convertColour( getwebHelp( w->requestedLocation+6, entry_buf, 10000),buf, CT_HTML, true);
		}else{
			convertColour( getwebHelp( w->requestedLocation+5, entry_buf, 10000),buf, CT_HTML, true);
		}			
		sprintf(entry_buf2, "WebHelp - %s", entry_buf);
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
			"%s" // doctype, header, and start of body tag
			"<H2><FONT COLOR=\"#BFBFBF\">%s WebHelp </H2><H3>- %s</H3></FONT>"
#ifdef VALIDATE_HTML
			"<TT><FONT>%s</FONT></TT>"		
#else
			"<PRE><FONT>%s</PRE>"
#endif
			,websvr_generate_head_and_start_body(entry_buf2)
			, MUD_NAME, entry_buf, buf);
		free(webhelp_buf); // temporary storage freed

	}else 
	if (!str_cmp(w->requestedLocation,"/classstats")){
		w->statusCode=200;
		buf=getwebClassStats();
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
			"%s" // doctype, header, and start of body tag
			"<A HREF=\"/\">Back to root</A><BR>\n"
			"<H2><FONT COLOR=\"#BFBFBF\">%s CLASS STATS</FONT></H2>"

#ifdef VALIDATE_HTML
			"\n<TT><FONT>%s</FONT></TT>"		
#else
			"\n<PRE><FONT>%s</PRE>"
#endif
			"<p><A HREF=\"/\">Back to root</A>"
			,websvr_generate_head_and_start_body("Class Stats")
			,MUD_NAME,buf);
	}else 
	if (!str_cmp(w->requestedLocation,"/racestats")){
		w->statusCode=200;
		buf=getwebRaceStats();
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
			"%s" // doctype, header, and start of body tag
			"<A HREF=\"/\">Back to root</A><BR>\n"
			"<H2><FONT COLOR=\"#BFBFBF\">%s RACE STATS</FONT></H2>"

#ifdef VALIDATE_HTML
			"\n<TT><FONT>%s</FONT></TT>"		
#else
			"\n<PRE><FONT>%s</PRE>"
#endif
			"<p><A HREF=\"/\">Back to root</A>"
			,websvr_generate_head_and_start_body("Race Stats")
			,MUD_NAME,buf);
	}else 
	if (!GAMESETTING5(GAMESET5_RACEINFO_DISABLED_IN_WEBSERVER) 
		&& !str_cmp(w->requestedLocation,"/raceinfo"))
	{
		w->statusCode=200;
		buf=getwebRaceInfo();
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
			"%s" // doctype, header, and start of body tag
			"<A HREF=\"/\">Back to root</A><BR>\n"
			"<H2><FONT COLOR=\"#BFBFBF\">%s Race Info</FONT></H2>"

#ifdef VALIDATE_HTML
			"\n<TT><FONT>%s</FONT></TT>"		
#else
			"\n<PRE><FONT>%s</PRE>"
#endif
			"<p><A HREF=\"/\">Back to root</A>"
			,websvr_generate_head_and_start_body("Race Info")
			,MUD_NAME,buf);
	}else 
	if (!GAMESETTING5(GAMESET5_CLASSINFO_DISABLED_IN_WEBSERVER) 
		&& (!str_prefix("/classinfo/",w->requestedLocation) 
		|| !str_cmp("/classinfo",w->requestedLocation)))
	{
		w->statusCode=200;
		if(w->requestedLocation[10]=='/'){
			buf=getwebClassInfo(w->requestedLocation+11);
		}else{
			buf=getwebClassInfo("");
		}			
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
			"%s" // doctype, header, and start of body tag
			"<A HREF=\"/\">Back to root</A><BR>\n"
			"<H2><FONT COLOR=\"#BFBFBF\">%s Race Info</FONT></H2>"

#ifdef VALIDATE_HTML
			"\n<TT><FONT>%s</FONT></TT>"		
#else
			"\n<PRE><FONT>%s</PRE>"
#endif
			"<p><A HREF=\"/\">Back to root</A>"
			,websvr_generate_head_and_start_body("Class Info")
			,MUD_NAME,buf);
	}else 
	if (!str_cmp(w->requestedLocation,"/alignstats")){
		w->statusCode=200;
		buf=getwebAlignStats();
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
			"%s" // doctype, header, and start of body tag
			"<A HREF=\"/\">Back to root</A><BR>\n"
			"<H2><FONT COLOR=\"#BFBFBF\">%s ALIGNMENT MATRIX</FONT></H2>"

#ifdef VALIDATE_HTML
			"\n<TT><FONT>%s</FONT></TT>"		
#else
			"\n<PRE><FONT>%s</PRE>"
#endif
			"<p><A HREF=\"/\">Back to root</A>"
			,websvr_generate_head_and_start_body("Active Player Alignment Matrix")
			,MUD_NAME,buf);
	}else 
	if (!str_cmp(w->requestedLocation,"/immwho")){
		int immweb_level=laston_web_level(w->username, w->password);
		// laston_web_level() returns the level/trust of the imm if the password is correct
		// returns -1 if it is an imm name, but no webpass has been set
		// otherwise returns 0 if incorrect or not imm
        if (immweb_level>0){
			w->statusCode=200;
			buf=getwebWho(immweb_level);
			w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
				"%s" // doctype, header, and start of body tag
"\n<PRE><FONT>%s</PRE>"
"<p><A HREF=\"/\">[Back to root]</A> <A HREF=\"/sockets\">[sockets]</A> <A HREF=\"/mudstats\">[mudstats]</A>"
,websvr_generate_head_and_start_body("Immortal Wholist")
,buf);
		}else if (immweb_level==-1){
			w->statusCode=200;
			buf=process_colour("`WYou haven't set a webpassword for your character,`1"
				"Use the webpass command within the game to do this.", NULL);
			w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
				"%s" // doctype, header, and start of body tag
				"<PRE><FONT>%s</PRE>"
				"<p><A HREF=\"/\">[Back to root]</A> <A HREF=\"/sockets\">[sockets]</A> <A HREF=\"/mudstats\">[mudstats]</A>"
				,websvr_generate_head_and_start_body("web password setting required!")
				,buf);
		}else{
			w->statusCode=401;
			logWeb("401 - Authorisation denied");
			w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
				"<html><head><title>Incorrect username/password</title></head>\n"
				"<body><h1>Incorrect username/password</h1>\n"
				"<p>You got the username or password incorrect.\n"
				"<p><A HREF=\"/\">Back to root</A>");
		}
	}else
	if (!str_cmp(w->requestedLocation,"/lastonimm")){
		int immweb_level=laston_web_level(w->username, w->password);
		// laston_web_level() returns the level/trust of the imm if the password is correct
		// returns -1 if it is an imm name, but no webpass has been set
		// otherwise returns 0 if incorrect or not imm
        if (immweb_level>0){
			w->statusCode=200;
			buf=getwebLastonImm(immweb_level);
			w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
				"%s" // doctype, header, and start of body tag
				"\n<PRE><FONT>%s</PRE>"
				"<p><A HREF=\"/\">[Back to root]</A> <A HREF=\"/sockets\">[sockets]</A> <A HREF=\"/immwho\">[immwho]</A> <A HREF=\"/mudstats\">[mudstats]</A>"
				,websvr_generate_head_and_start_body("Immortal Laston")
				,buf);
		}else{
			websvr_handle_insufficient_immweb_level(w, buf, immweb_level);
		}
	}else
	if (!str_cmp(w->requestedLocation,"/sockets")){

		int immweb_level=laston_web_level(w->username, w->password);
		// laston_web_level() returns the level/trust of the imm if the password is correct
		// returns -1 if it is an imm name, but no webpass has been set
		// otherwise returns 0 if incorrect or not imm
        if (immweb_level>0){
			w->statusCode=200;
			buf=getwebSockets(immweb_level);
			w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
				"%s" // doctype, header, and start of body tag
				"<FONT COLOR=\"#BFBFBF\">"
				"\n<PRE><FONT>%s</PRE>"
"<p><A HREF=\"/\">[Back to root]</A> <A HREF=\"/immwho\">[immwho]</A> <A HREF=\"/mudstats\">[mudstats]</A>"
,websvr_generate_head_and_start_body("Immortal Sockets")
,buf);
		}else{
			websvr_handle_insufficient_immweb_level(w, buf, immweb_level);
		}
	}else
	if (!str_cmp(w->requestedLocation,"/")){
		w->statusCode=200;
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
		"%s" // doctype, header, and start of body tag
"<H2 ALIGN=\"CENTER\">%s Mud Integrated Webserver</FONT></H2>"
"<P ALIGN=\"CENTER\"><A HREF=\"/who\">Who list</A><BR>"
"<P ALIGN=\"CENTER\"><A HREF=\"/help\">WebHelp</A><BR>"
"<P ALIGN=\"CENTER\"><A HREF=\"/areas\">Areas</A><BR>"
"<P ALIGN=\"CENTER\"><A HREF=\"/mudstats\">Mudstats</A><BR>"
"<A HREF=\"/wizlist\">The Wizlist</A><BR>"
"<A HREF=\"/classstats\">Active Player Class Stats</A><BR>"
"<A HREF=\"/racestats\">Active Player Race Stats</A><BR>"
"%s" // raceinfo - optional with gamesetting
"%s" // classinfo - optional with gamesetting
"<A HREF=\"/alignstats\">Active Player Alignment Matrix</A><BR>"
"<A HREF=\"/gamesettings\">Game Settings</A><BR>"
"<A HREF=\"/immwho\">Immortal version of the who list - immonly</A><BR>"
"<A HREF=\"/lastonimm\">List of imms recently on - immonly</A><BR>"
"<A HREF=\"/sockets\">Connections - immonly</A></P>"
"<P ALIGN=\"CENTER\"><A HREF=\"/locked\">Reset password</A><BR>"
"<P ALIGN=\"CENTER\"><A HREF=\"http://www.dawnoftime.org/\">Go to the official Dawn of Time website</A></P>"
"</P></BODY>"
"</HTML>"
,websvr_generate_head_and_start_body(MUD_NAME)
, MUD_NAME,
GAMESETTING5(GAMESET5_RACEINFO_DISABLED_IN_WEBSERVER)?"":"<A HREF=\"/raceinfo\">Race Info</A><BR>",
GAMESETTING5(GAMESET5_CLASSINFO_DISABLED_IN_WEBSERVER)?"":"<A HREF=\"/classinfo\">Class Info</A><BR>");

	}else
	if (!str_cmp(w->requestedLocation,"/locked")){
		w->statusCode=401;
		logWeb("401 - Authorisation denied");
		w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
			"<html><head><title>Incorrect username/password</title></head>\n"
			"<body><h1>Incorrect username/password</h1>\n"
			"<p>You got the username/password incorrect.\n"
			"<p>Try username=user and password=password at <A HREF=\"/locked2\">locked2</A> for a test.\n"
			"<p><A HREF=\"/\">Back to root</A>");
	}else
	if (!str_cmp(w->requestedLocation,"/locked2")){
		if (!strcmp(w->username,"user") && !strcmp(w->password,"password")){
			w->statusCode=200;
			w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
				"<html><head><title>Access granted!</title></head>\n"
				"<body><h1>Access granted!</h1>\n"
				"<p>You got the password etc correct.\n"
				"<p>Click on <A HREF=\"/locked\">locked</A> to be denied.\n"
				"<p><A HREF=\"/\">Back to root</A>");
		}else{
			w->statusCode=401;
			logWeb("401 - Authorisation denied");
			w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF,
				"<html><head><title>Incorrect username/password</title></head>\n"
				"<body><h1>Incorrect username/password</h1>\n"
				"<p>You got the username/password incorrect.\n"
				"<p>try username=user and password=password\n"
				"<p><A HREF=\"/\">Back to root</A>");
		}
	}else{
		w->statusCode=200;
		w->outBodyLen=snprintf(w->outBodyBuf,WEBOUT_BODY_BUF,
			"<html><head><title>Not found</title></head>\n"
			"<body><h1>couldn't find the url you wanted</h1>\n"
			"<p><A HREF=\"/\">Back to root</A>");
	}

	if(w->outBodyLen<0){
		logWebf("Body %d bytes!!! - invalid length, attempting to repair!", w->outBodyLen);
		if(str_len(buf)>WEBOUT_BODY_BUF-10){
			w->outBodyBuf[WEBOUT_BODY_BUF-10]='\0';
			w->outBodyLen=WEBOUT_BODY_BUF-9;
		}else{
			w->outBodyLen=snprintf(w->outBodyBuf, WEBOUT_BODY_BUF, 
				"an error occurred while processing this webpage, please report this to the admin!");
			w->statusCode=500; // Internal Error
		}
	}
	
	logWebf("Making header - Body %d bytes", w->outBodyLen);
	createHeader(w);
}
/**************************************************************************/
void processPOST(web_request_data *w){
	webHits++;

	if (!str_cmp(w->requestedLocation,"/")){
		w->statusCode=200;
		w->outBodyLen=sprintf(w->outBodyBuf,
"<HTML>"
"<HEAD>"
"<TITLE>%s</TITLE>"
"</HEAD>"
"<BODY LINK=\"#0000ff\" VLINK=\"#800080\">"
"<H2 ALIGN=\"CENTER\">%s Mud Integrated Webserver</FONT></H2>"
"<P ALIGN=\"CENTER\"><A HREF=\"http://www.dawnoftime.org/\">Go to the official Dawn of Time website</A></P>"
"<P ALIGN=\"CENTER\"><A HREF=\"/who\">Who list</A><BR>"
"<P ALIGN=\"CENTER\"><A HREF=\"/mudstats\">Mudstats</A><BR>"
"<A HREF=\"/wizlist\">The wizlist</A><BR>"
"<A HREF=\"/immwho\">Immortal version of the who list</A><BR>"
"<A HREF=\"/sockets\">Connections - immonly</A></P>"
"<P ALIGN=\"CENTER\"><A HREF=\"/locked\">Reset password</A><BR>"
"</P></BODY>"
"</HTML>", MUD_NAME, MUD_NAME);
	}else


	if (!str_cmp(w->requestedLocation,"/locked2")){
		if (!strcmp(w->username,"user") && !strcmp(w->password,"password")){
			w->statusCode=200;
			w->outBodyLen=sprintf(w->outBodyBuf,
				"<html><head><title>Access granted!</title></head>\n"
				"<body><h1>Access granted!</h1>\n"
				"<p>You got the password etc correct.\n"
				"<p><A HREF=\"/\">Back to root</A>");
		}else{
			w->statusCode=401;
			logWeb("401 - Authorisation denied");
			w->outBodyLen=sprintf(w->outBodyBuf,
				"<html><head><title>Incorrect username/password</title></head>\n"
				"<body><h1>Incorrect username/password</h1>\n"
				"<p>You got the username/password incorrect.\n"
				"<p>try username=user and password=password\n"
				"<p><A HREF=\"/\">Back to root</A>");
		}
	}else{
		w->statusCode=200;
		w->outBodyLen=sprintf(w->outBodyBuf,
			"<html><head><title>Not found</title></head>\n"
			"<body><h1>couldn't find the url you wanted</h1>\n"
			"<p><A HREF=\"/\">Back to root</A>");
	}
	
	logWebf("making header");
	createHeader(w);
}
/**************************************************************************/
void processWebRequest(connection_data *c){
	web_request_data *w=c->web_request;
	char loguname[100];
	logWebf("processWebRequest");
	logWebf("REQUEST: %d  LOCATION: '%s'", w->requestType, w->requestedLocation);
	if (w->username && str_len(w->username)>0){
		strncpy(loguname, w->username, 99);
		loguname[99]='\0';
		logWebf("USERNAME: '%s'", w->username);
//		logWebf("PASSWORD: '%s'", w->password);
	}else{
		loguname[0]='-';
		loguname[1]='\0';
	}

	switch (w->requestType){
	case GET:
		processGET(w);
		break;
	case HEAD:
		badRequestFormat(w, "HEAD - Unsupported method");
		break;
	case POST:
		processPOST(w);
//		badRequestFormat(w, "POST - Unsupported method");
		break;
	case UNDEFINED:
	default:
		badRequestFormat(w, "Undefined method");
		break;
	}


	// Web log stuff - can use with webalizer (http://www.mrunix.net/webalizer)
	// and other website stat generating software
	if(!(game_settings->flags2 & GAMESET2_NO_WEBLOG))
	{
		char logdate[100];
		time_t ltime;   
		time( &ltime );
		
	#ifdef WIN32 // VC++6 compiler doesn't support %z, doesn't really matter
					// hard coded it to NZDT :)
		strftime(logdate, 100, "[%d/%b/%Y:%H:%M:%S +1300]", localtime( &ltime));
	#else
		strftime(logdate, 100, "[%d/%b/%Y:%H:%M:%S %z]", localtime( &ltime));
	#endif

		char weblogbuf[4096];
		sprintf(weblogbuf,"%s - %s %s \"%s\" %d %d \"%s\" \"%s\"", 
			c->remote_ip, // 50 bytes 
			loguname, // 100 bytes
			logdate, // 100 bytes
			w->firstline, // 1024 max
			w->statusCode, // 3 bytes 
			w->outBodyLen, // 6 bytes max
			w->referer,		// 1024 max
			w->user_agent); // 1024 max

		char weblogfname[30];
		sprintf(weblogfname, "weblog%d.txt", c->local_port);
		append_string_to_file(weblogfname, weblogbuf, true);
	}
}
/**************************************************************************/
void filterWebRequest(connection_data *c){
	web_request_data *w=c->web_request;
	char *pCRLF, *pHTTP, *pCRLFCRLF;

	pCRLF=NULL;

	if (w->receivingHeader){
		// don't filter it if there isn't enough to consist a valid 
		// html header request
		if (w->inHeaderLen<2)
			return;

		// next check if the first line has been receieved
		// an HTML request must finish with CRLF as per RFC 1945
		pCRLF = strstr( w->inHeaderBuf, "\r\n" );
		if (!pCRLF){
			return;
		}

		// check if it is a HTTP/0.9 request 
		// i.e. no "HTTP/" before the first CRLF
		pHTTP = strstr( w->inHeaderBuf, "HTTP/" );
		if (!pHTTP || pHTTP>pCRLF){
			// it must be a Simple-Request (HTTP/0.9)
			logWebf("HTTP/0.9 request from %s request:%s",
				c->remote_ip,w->inHeaderBuf);
			processHTTP0_9Request(w);
			return;
		}

		// PRE: Request is HTTP/1.0 or higher

		// next check if the header has been completed
		// If so the end of the header is marked with CRLFCRLF
		pCRLFCRLF = strstr( w->inHeaderBuf, "\r\n\r\n" );
		if (pCRLFCRLF){ 
			*(pCRLFCRLF+2)='\0';
//			logWebf ("%d %d", (int)*pCRLFCRLF, (int) *(pCRLFCRLF+4));
			processWebHeader(w);

			// now if we have a content length
			if (w->contentLength){
				strcpy(w->inBodyBuf,pCRLFCRLF+4);				
			w->inBodyLen = str_len(w->inBodyBuf);

//			pHTTP = strstr( w->inHeaderBuf, "contentlength" );

//			strcpy(w->inBodyBuf,pCRLFCRLF+4);
			}
		

		}else{ // isn't the end of the header yet
			// do nothing really
		}
	}
	
	// read the in body
	if (!w->receivingHeader && (w->inBodyLen>=w->contentLength)){ 

	}

//Content-Length:
	logWebf("HTTP/1.0 request from %s",c->remote_ip);
//	logf("%s", w->inHeaderBuf);

	// filter it in here
	{

		char *pStr, *pLine;
		char workingline[WEBIN_HEADER_BUF+100];
		char method[21];

		// check request isn't too long
		if (pCRLF>w->inHeaderBuf+WEBIN_HEADER_BUF){
			badRequestFormat(w, "Request-Line too long");
			return;
		}

		strncpy(workingline, w->inHeaderBuf, pCRLF-w->inHeaderBuf);
		workingline[pCRLF-w->inHeaderBuf]='\0';
		pLine=workingline;

		// copy parameters, checking the 
		// length of each command as we go
		strncpy(w->firstline, workingline, 1024);
		w->firstline[1023]='\0';

		// extract the METHOD
		pStr=strstr( pLine, " ");
		if (!pStr){
			badRequestFormat(w, "Request-Line invalid");
			return;
		}
		if (pStr-pLine>20){
			badRequestFormat(w, "method in Request-Line too long");
			return;
		}
		strncpy(method, pLine, pStr-pLine);		
		method[pStr-pLine]='\0';
		pLine=pStr+1;

		// extract the LOCATION
		pStr=strstr( pLine, " ");
		if (!pStr){
			badRequestFormat(w, "Request-Line invalid");
			return;
		}
		if (pStr-pLine>MAXLOCATION){
			badRequestFormat(w, "Request-URI (location) in Request-Line too long");
			return;
		}
		strncpy(w->requestedLocation, pLine, pStr-pLine);
		w->requestedLocation[pStr-pLine]='\0';
		pLine=pStr+1;


		// extract the VERSION
		if (strstr( pLine, "HTTP/" )==NULL)
		{
			badRequestFormat(w, "no HTTP version in Request-Line");
		}
		if (str_len(pLine)>20){
			badRequestFormat(w, "HTTP-Version in Request-Line too long");
			return;
		}	
		strcpy(w->version, pLine);

		// now identify the method
		if (!strcmp(method,"GET")){
			w->requestType=GET;
		}else
		if (!strcmp(method,"HEAD")){
			w->requestType=HEAD;
		}else
		if (!strcmp(method,"POST")){
			w->requestType=POST;
		}else{
			w->requestType=UNDEFINED;
		}

		// check for a "Referer:" header
		pLine=strstr( w->inHeaderBuf, "Referer:");
		if(pLine){
			pLine+=str_len("Referer:");
			while(is_space(*pLine)){
				pLine++;
			}
			// we have an Referer - extract it
			pStr=strstr( pLine, "\r\n");
			if (pStr){
				strncpy(workingline, pLine, pStr-pLine);
				workingline[pStr-pLine]='\0';				
				strncpy(w->referer,workingline, 1024);
				w->referer[1023]='\0';
			}else{
				strcpy(w->referer,"-");
			}
		}else{
			strcpy(w->referer,"-");
		}

		// check for a "User-Agent:" header
		pLine=strstr( w->inHeaderBuf, "User-Agent:");
		if(pLine){
			pLine+=str_len("User-Agent:");
			while(is_space(*pLine)){
				pLine++;
			}
			// we have an User-Agent - extract it
			pStr=strstr( pLine, "\r\n");
			if (pStr){
				strncpy(workingline, pLine, pStr-pLine);
				workingline[pStr-pLine]='\0';				
				strncpy(w->user_agent,workingline, 1024);
				w->user_agent[1023]='\0';
			}else{
				strcpy(w->user_agent,"-");
			}
		}else{
			strcpy(w->user_agent,"-");
		}
		

		// check for an "Authorization:" header
		pLine=strstr( w->inHeaderBuf, "Authorization:");
		if(pLine){
			// we have an Authorization - process it
			pStr=strstr( pLine, "\r\n");
			if (pStr){
				strncpy(workingline, pLine, pStr-pLine);
				workingline[pStr-pLine]='\0';
				pLine=workingline;

				//we have the authorization string in newline
				logWebf("Processing Authorization: '%s'",workingline);

				pStr=strstr( pLine, "Basic");

				if (pStr){
					pLine=pStr+6;

					if (str_len(pLine)>0 && str_len(pLine)<200){
						char decoded[200];
						char *d;

						d =decodeBase64(pLine);
						if (d){
							strcpy(decoded,d);						
							// now separate the username:password
							pLine=decoded;
							pStr=strstr( pLine, ":");
							if (pStr){
								if (pStr-pLine<MAXPASSLENGTH){
									strncpy(w->username, pLine, pStr-pLine);
									w->username[pStr-pLine]='\0';
									pLine=pStr+1;

									if (str_len(pLine)>MAXPASSLENGTH){
										// not a valid password - so authuenticate invalid
										w->username[0]='\0';
									}else{
										strcpy(w->password, pLine);
									}
								}								
							}						
						}
					}							
				}else{
					logWeb("Authorization type not recognised");
				}
			}
		}

		// filtering completed - now process the request
		processWebRequest(c);
	}
	return;
}
/**************************************************************************/
/**************************************************************************/

