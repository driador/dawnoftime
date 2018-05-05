/* MUDftp module
 * (c) Copyright 1997, 1998 Erwin S. Andreasen and Oliver Jowett
 * This code may be freely redistributable, as long as this header is left
 * intact.
 *
 * Thanks to:
 * - Jessica Boyd for the ROM version
 * - Dominic J. Eidson for the ROT version
 * - George Greer for the Circle version
 */
/* Define one of the below
 MERC: Will work for MERC, Envy, ROM, ROT
 CMUD: Will work for CircleMUD 3.x, bpl14 tested.
 */
#define MERC
/* #define CMUD 1 */

#ifdef WIN32
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#include "include.h"
#include "nanny.h"

#define CLOSE_DESCRIPTOR(desc,explanation) connection_close(desc)
#define WRITE(desc,text)		desc->write(text, 0)
#define GET_NAME(ch)		((ch)->name)
#define GET_PASSWD(ch)		((ch)->pcdata->pwd)

#if !defined(NUL)
#define NUL '\0'
#endif

#if !defined(MAX_PWD_LENGTH)
#define MAX_PWD_LENGTH 12
#endif

/**************************************************************************/
#define mudftp_notify logf
/*static void mudftp_notify(const char *fmt, ... ) {
    va_list va;
    char buf[MSL];
    
    va_start(va, fmt);
    vsnprintf(buf, MSL, fmt, va);
    va_end(va);

    // Then do something with "buf", appropriate to the current MUD base 
    // E.g. send it over "Wiznet" or log it (not recommended) 
    // log_string (buf); 
}
*/
/**************************************************************************/
// Called this because of conflict with ROT 
static void mudftp_str_replace(char *buf, const char *s, const char *repl) {
    char out_buf[MSL];
    char *pc, *out;
    int len = str_len(s);
    bool found = false;
    
    for (pc = buf, out = out_buf; *pc && (out-out_buf) < (MSL-len-4); ){
        if(!strncasecmp(pc, s, len)){
            out += sprintf(out, "%s", repl);
            pc += len;
            found = true;
        }else{
            *out++ = *pc++;
		}
	}
    
    if (found) { // don't bother copying if we did not change anything 
        *out = NUL;
        strcpy(buf, out_buf);
    }
}

/**************************************************************************/
int line_count (const char *s) {
    int count = 0;
    
    for (; *s; s++){
        if (*s == '\n'){
            count++;
		}
	}
    
    return count;
}

/**************************************************************************/
void greet_ftp(connection_data *c) 
{
    c->connected_state = CON_FTP_AUTH;
	mudftp_notify("FTP connect from %s", c->remote_tcp_pair);
}

/**************************************************************************/
// Authorization line: <username> <password> 
void handle_ftp_auth (connection_data *d, const char *argument) {
    char name[MIL], pass[MIL];
    connection_data *dftp, *dftp_next;
    char_data *ch = NULL;

    mudftp_str_replace((char*)argument, "\r", "");
    
    argument = first_arg((char*)argument, name, false);
    
    // Find the descriptor of the connected character 
    for (dftp = connection_list; dftp; dftp=dftp->next) {
        if (dftp != d &&
            dftp->character &&
            !IS_NPC(dftp->character) &&
            dftp->connected_state >= CON_PLAYING &&
            !str_cmp(GET_NAME(dftp->character), name)) {
            ch = dftp->character;
            break;
        }
    }
    argument = first_arg((char*)argument, pass, false);
    mudftp_str_replace(pass, "\r", "");
    
    if (!ch || (!is_valid_password(pass, GET_PASSWD(ch), NULL) 
		&& strcmp(GET_PASSWD(ch),"none"))) {
		WRITE(d,"FAILED\n");
		CLOSE_DESCRIPTOR(d, "FTP authorization failure");
		mudftp_notify("FTP authorization for %s [%d] failed",
			name, d->connected_socket);
		return;
    }
    
    // Search for old ftp connections 
    for (dftp = connection_list; dftp; dftp=dftp_next) {
        dftp_next = dftp->next;
        
        if (dftp != d &&
            (dftp->connected_state == CON_FTP_COMMAND ||
             dftp->connected_state == CON_FTP_DATA) &&
            !str_cmp(dftp->username, name))
            CLOSE_DESCRIPTOR(dftp, "Old mudftp connection");
    }
    
    d->username = str_dup(name);
    WRITE(d, "OK mudFTP 2.0 ready\n");
    d->connected_state = CON_FTP_COMMAND;
    mudftp_notify("FTP authorization for %s@%s", name, d->remote_hostname);
}

/**************************************************************************/
// This algorithm is derived from the one supposedly used in Perl 
static const char *ftp_checksum(const char *string) {
    static char buf[10];
    int i = str_len(string);
    unsigned hash = 0;
    
    while(i--)
        hash = hash * 33U + *string++;
    
    sprintf(buf, "%08x", hash);
    return buf;
}

/**************************************************************************/
static char_data *findFTPChar(connection_data *d) {
    connection_data *dftp;
    
    for (dftp = connection_list; dftp; dftp=dftp->next)
    {
        if (dftp != d &&
            dftp->character &&
            !str_cmp(GET_NAME(dftp->character), d->username) &&
            dftp->pString)
            return dftp->character;
    }
    
    return NULL;
}

#ifdef WIN32
#ifdef _MSC_VER // visual c++ only
#pragma warning( disable : 4311 ) // disable pointer truncation warning in VS.NET
#endif
#endif
/**************************************************************************/
static void finish_file(connection_data *d) {
    unsigned long temp_file;
    
    mudftp_notify("Transfer of %s done from %s@%s", 
		d->ftp.filename, d->username, d->remote_hostname);
    
    d->connected_state = CON_FTP_COMMAND;
    /* Put the file in its rightful spot */
    
    if (1 == sscanf(d->ftp.filename, "tmp/%lu", &temp_file))
    {
        char_data *ch = findFTPChar(d);

        if (ch && ((unsigned long) ch->desc->pString) == temp_file)
        {
            char temp[MSL];
            char buf[MSL];
            
            strcpy(temp, d->ftp.data);
            smash_tilde(temp);
            
            sprintf(buf, "OK %s\n", ftp_checksum(temp));
            WRITE(d,buf);
            
            free_string(*ch->desc->pString);
            *ch->desc->pString = str_dup(temp);
            free_string(d->ftp.data);
            d->ftp.data = NULL;
            
            strcpy(buf, "@");
            string_add(ch, buf); // Finish editing 
			ch->println("done.");
            return;
        }
    }
    
    WRITE(d,"FAILED Something went wrong\n");
    
    free_string(d->ftp.data);
    free_string(d->ftp.filename);
	d->ftp.inuse=false;
}

/**************************************************************************/
void handle_ftp_command (connection_data *d, const char *argument) {
    char arg[MIL];
    mudftp_str_replace((char*)argument, "\r", "");

    const char *orig_argument = argument;
    argument = one_argument((char*)argument, arg);
    
    if (!str_cmp(arg, "noop")) {
        WRITE(d,"OK\n");
        return;
    }
    
    mudftp_notify("FTP command: '%s' from %s@%s", 
		orig_argument, d->username, d->remote_hostname);
    
    if (!str_cmp(arg, "push")) {
        if (d->ftp.mode != FTP_NORMAL)
        {
            WRITE(d, "ERROR Already in push mode\n");
            return;
        }
        
        d->ftp.mode = FTP_PUSH;
        WRITE(d, "OK Pushing you data as it arrives\n");
        return;
    }
    
    if (!str_cmp(arg, "stop"))	{
        char_data *ch = findFTPChar(d);
        
        if (!ch)
            WRITE(d,"FAILED\n");
        else {
            free_string(d->ftp.data);
            d->ftp.data = NULL;
            
            string_add(ch,"@"); // Finish editing 
            WRITE(d,"OK\n");
        }
        
        if (d->ftp.mode == FTP_PUSH_WAIT)
            d->ftp.mode = FTP_PUSH;
        
        return;
    }
    
    if (!str_cmp(arg, "put")) {
        argument = one_argument((char*)argument, arg);
        if (!argument[0] || !is_number((char*)argument) || atoi(argument) < 0)
            WRITE(d, "ERROR Missing filename or number of lines\n");
        else
        {
            d->ftp.filename = str_dup(arg);
            d->ftp.lines_left = atoi(argument);
            d->ftp.data = str_dup("");
            
            if (d->ftp.lines_left)
                d->connected_state = CON_FTP_DATA;
            else
                finish_file(d);
        }
        
        if (d->ftp.mode == FTP_PUSH_WAIT)
            d->ftp.mode = FTP_PUSH;
    }
    else if (!str_cmp(arg, "get")) {
        unsigned long temp_file;
        
        if (d->ftp.mode == FTP_PUSH_WAIT)
        {
            WRITE(d, "FAILED Expected STOP or PUT");
            return;
        }
        
        /* Send buffer being edited */
        if (1 == sscanf(argument, "tmp/%lu", &temp_file)) {
            char_data *ch = findFTPChar(d);
            
            if (!ch || ((unsigned long) ch->desc->pString) != temp_file)
                WRITE(d,"FAILED\n");
            else { /* Write the string */
                char buf[MSL];
                char buf2[MSL];
                
                if (*ch->desc->pString)
                    strcpy(buf2, *ch->desc->pString);
                else
                    buf2[0] = '\0';
                
                mudftp_str_replace(buf2, "\r", "");

				if(buf2[0] && buf2[str_len(buf2)-1]!='\n'){
					strcat(buf2, "\n"); // everything sent to mudftp must end with a \n
				}
                
                sprintf(buf, "SENDING tmp/%lu %d %s\n", temp_file, line_count(buf2),
                        ftp_checksum(buf2));
                if (WRITE(d,buf)<0 || WRITE(d,buf2)<0) {
                    CLOSE_DESCRIPTOR(d, "FTP write failure");
                    return;
                }
				d->ftp.inuse=true;
            }
        }else{
            WRITE(d, "FAILED Currently only tmp/file is supported\n");
		}
    }
    else if (!str_cmp(arg, "quit"))
        CLOSE_DESCRIPTOR(d, "Quitting");
    else
        WRITE(d, "ERROR unknown command\n");
}

/**************************************************************************/
void handle_ftp_data (connection_data *d, const char *argument) {
    int len_data, len_argument;

    mudftp_str_replace((char*)argument, "\r", "");

    len_data = str_len(d->ftp.data);
    len_argument = str_len(argument);
    
    // Lines that overflow the buffer are silently lost 
    if (len_data + len_argument < MSL-16) {
        char buf[MSL];
        strcpy(buf, d->ftp.data);
        strcpy(buf+len_data, argument);
        
        // All strings are \n internally 
        strcpy(buf+len_data+len_argument, "\r\n");
        free_string(d->ftp.data);
        d->ftp.data = str_dup(buf);
    }
    
    // All of the file received?
    if (--d->ftp.lines_left == 0)
        finish_file(d);
}

/**************************************************************************/
// Try to push a string to this desc. false if we can't 
bool ftp_push(connection_data *d) {
    connection_data *m;
    for (m = connection_list; m; m=m->next)  {
        if (m->connected_state == CON_FTP_COMMAND &&
            m->ftp.mode == FTP_PUSH &&
            !str_cmp(m->username, GET_NAME(d->character))) {
            
            char buf[MSL];
            char buf2[MSL];
            
            if (*d->pString){
                strcpy(buf2, *d->pString);
            }else{
                buf2[0] = '\0';
			}
            
            mudftp_str_replace(buf2, "\r", ""); // Never send \r to clients 
            
			if(buf2[0] && buf2[str_len(buf2)-1]!='\n'){
				strcat(buf2, "\n"); // everything sent to mudftp must end with a \n
			}
            
            sprintf(buf, "SENDING tmp/%lu %d %s\n", (unsigned long)d->pString, line_count(buf2),
                    ftp_checksum(buf2));
            
            if (WRITE(m,buf)<0 || WRITE(m,buf2)<0) {
                CLOSE_DESCRIPTOR(m, "FTP write failure");
                return false;
            }
            
            m->ftp.mode = FTP_PUSH_WAIT;
            
            return true;
        }
    }
    
    return false;
}
#ifdef WIN32
#ifdef _MSC_VER // visual c++ only
#pragma warning( default : 4311 ) // reenable pointer truncation warning in VS.NET
#endif
#endif

/**************************************************************************/
// Tell the dawnftp client to reconnect - requires dawnftp... 
// used for hotreboots because I don't feel motivated to write the code 
// to transfer the mudftp descriptor accross the hotreboot
// - Kal, May 02
bool ftp_reconnect(char *name) 
{
    connection_data *m;
    for (m = connection_list; m; m=m->next)  {
        if (m->connected_state == CON_FTP_COMMAND &&
            m->ftp.mode == FTP_PUSH &&
            !str_cmp(m->username, name)) {
            
            if (WRITE(m,"RECONNECT\n")<0) {
                CLOSE_DESCRIPTOR(m, "FTP write failure");
                return false;
            }          
            return true;
        }
    }    
    return false;
}
/**************************************************************************/
/**************************************************************************/

