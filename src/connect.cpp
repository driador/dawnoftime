/**************************************************************************/
// connect.cpp - connection_data class implementation
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
// note: some of the mccp code is based on the work by Oliver Jowett from
//       http://www.randomly.org/projects/MCCP
/**************************************************************************/

#include "network.h"
#include "include.h"
#include "connect.h"
#include "nanny.h"
#include "msp.h"

/**************************************************************************/
#define TELOPT_TERM_TYPE	24 // see rfc 930
#define TELOPT_NAWS			31 // Negotiate about window size
#define TELOPT_COMPRESS2	86
#define TELOPT_COMPRESS		85
#define TELOPT_MSP			90
#define TELOPT_MXP			91

#include "telnet.h"

#define IAC_SB			(char)IAC, (char)SB
#define SE_NUL			(char)SE, '\0'
#define IAC_SE_NUL		(char)IAC, SE_NUL
#define WILL_SE_NUL		(char)WILL, SE_NUL


const char mxp_start_buf[]= { IAC_SB, TELOPT_MXP, IAC_SE_NUL};
const char *mxp_start_command= mxp_start_buf;

#ifdef MCCP_ENABLED
	const char compress2_start [] = { IAC_SB,	TELOPT_COMPRESS2, IAC_SE_NUL};
	const char compress_start  [] = { IAC_SB,	TELOPT_COMPRESS,  WILL_SE_NUL};
#endif //MCCP_ENABLED
/**************************************************************************/
// send some raw data to a socket
// return the number of bytes written, negative -1 on a send error
int write_to_socket( dawn_socket output_socket, const char *txt, int length )
{
//	logf("write_to_socket(): %d, %d, %-80.80s", output_socket, length, txt);
    int iStart;
    int nWrite=0;
    int nBlock;

    if ( length <= 0 ){
		length = str_len(txt);
	}

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
		nBlock = UMIN( length - iStart, 4096 );
		nWrite = send( output_socket, txt + iStart, nBlock, 0 );
		if ( nWrite < 0 ){ 
#ifdef WIN32			
			if(WSAGetLastError()==WSAEWOULDBLOCK){
				break;
			}
#endif
#ifdef unix
#	ifndef EAGAIN
#		define	EAGAIN	11 // Try again 
#	endif
#	ifndef ENOSR
#		define	ENOSR	63 // Out of streams resources 
#	endif
			if (errno == EAGAIN || errno == ENOSR){
				break;
			}
#endif

			socket_error( "write_to_socket()" ); 
			return -1; 
		}
    } 
    return iStart + UMIN(0, nWrite);
}
/**************************************************************************/
extern bool hotreboot_in_progress;
/**************************************************************************/
// ** Main function for sending data to a connection, transparently 
//    handles compression for MCCP.
// write to the socket, passing thru MCCP where appropriate
// return the number of bytes written, -1 for an error 
int connection_data::write(const char *txt, int length )
{
    if ( length <= 0 ){
		length = str_len(txt);
	}

	bytes_sent+=length;
#ifdef MCCP_ENABLED
	if(out_compress){
		bytes_sent_before_compression+=length;
		// mccp enabled connection, compress the data then send it 
		z_stream *s = out_compress;
    	s->next_in = (unsigned char *)txt;
		s->avail_in = length;
		int bad_write_loop=0, totalwritten=0;
		while (s->avail_in && bad_write_loop<5) {
			s->avail_out = COMPRESS_BUF_SIZE - (s->next_out - out_compress_buf);
            
			if(s->avail_out){
				int status;
				if(hotreboot_in_progress){
					status= deflate(s, Z_FULL_FLUSH);
				}else{
					status= deflate(s, Z_SYNC_FLUSH);
				}

				if(status != Z_OK){
					logf("connection_data::write() - compression error.");
					return -1;
				}
			}

			// now send the compressed data out the socket
			{
				int len=out_compress->next_out - out_compress_buf;
				int written=write_to_socket( connected_socket, (char*)out_compress_buf, len);
				if (written>0) {
					bytes_sent_after_compression+=written;
					// We wrote "written" bytes 
					if (written < len){
						memmove(out_compress_buf, out_compress_buf+written, len - written);
					}
					out_compress->next_out = out_compress_buf + len - written;
					totalwritten+=written;
				}
				if(written<1){
					bad_write_loop++;
				}
			}
		}
		if(bad_write_loop==5){ 
			// write_to_socket() failed to write the data 5 times
			// while attempting to compress the data
			if(totalwritten){ // we did how ever suceed to write some of the data to the socket
				return totalwritten;
			}
			return -1; // complete failure
		}
		// everything was sent or written into the compressed buffer
		return length; 
	}else{
		return write_to_socket( connected_socket, txt, length );
	}
#else
	return write_to_socket( connected_socket, txt, length );
#endif // MCCP_ENABLED
}
/**************************************************************************/
int connection_data::write_colour(const char *txt, int)
{
	convertColour(txt, temp_HSL_workspace, colour_mode, false); 
	return write(temp_HSL_workspace, 0);
}
/**************************************************************************/
// called by process_output
bool connection_data::send_outbuf()
{
	int written=write(outbuf, outtop);
	if ( written<0){
		outtop = 0;
		return false;
	}

    if (written) {
		if (written < outtop){ // move any remaining bytes to start of buffer
            memmove(outbuf, outbuf+written, outtop - written);
		}
        outtop = outtop- written;
    }
	return true;
}
/**************************************************************************/
void flush_cached_write_to_buffer(connection_data *d);

/**************************************************************************/
// flush a descriptors output
bool connection_data::flush_output()
{
	if(!this){
		return false;
	}

	// no flush required
	if(outtop == 0){
		return true;
	}

	flush_cached_write_to_buffer(this); 
   
	// OS-dependent output.
	return send_outbuf();
}
/**************************************************************************/
// close the actual socket attached to a connection structure
void connection_data::close_socket()
{
	logf("Closing socket %d", connected_socket);
#ifdef __CYGWIN__
	// a hack to make cygwin shutdown sockets after a hotreboot
	// cygwin still appears to be leaking endpoints according to 
	// processexplorer from sysinternals, but this atleast gets
	// the socket to disconnect.
	if(shutdown(connected_socket, 2)!=0){
		logf("error %d calling shutdown on socket %d.", errno, connected_socket);
	}
#endif
	if (closesocket(connected_socket )!=0){
		socket_error(FORMATF("connection_data::close_socket(): error calling closesocket() on socket %d",connected_socket));
	}
	connected_socket=0;
}
/**************************************************************************/
void visual_debug_flush( connection_data *d);
/**************************************************************************/
void connection_data::send_will_telnet_option( unsigned char option_value)
{
	unsigned char will_telnet_option[] = { IAC, WILL, option_value, '\0'};
	write_to_buffer(this, (char*)will_telnet_option, 0);
//	write_to_buffer(this, "test", 0);
//	visual_debug_flush( this);

#ifdef DEBUG_TELNET_OPTION_NEGOTIATION
	logf("send_will_telnet_option(%d) sent  (socket=%d)", option_value, connected_socket);
#endif

}
/**************************************************************************/
// send info on stuff like MCCP support etc (IAC signals basically)
void connection_data::advertise_supported_telnet_options( )
{
#ifdef DEBUG_TELNET_OPTION_NEGOTIATION
	logf("connection_data::advertise_supported_telnet_options(%d)",	connected_socket);
#endif
	advertised_options_count=0;

	send_will_telnet_option(TELOPT_MXP);
	advertised_options_count++;

	if(!IS_NULLSTR(MSP_URL)){
		send_will_telnet_option(TELOPT_MSP);
		advertised_options_count++;
	}

#ifdef MCCP_ENABLED
	if(!out_compress){ // offer to compress if we arent already compressing
		send_will_telnet_option(TELOPT_COMPRESS2);
		send_will_telnet_option(TELOPT_COMPRESS);
		advertised_options_count+=2;
	}
#endif
	
	{ // tell mud client we do support receiving the terminal type
		unsigned char telnet_do_terminal_type[] = { 
			IAC, DO, TELOPT_TERM_TYPE, '\0'};
		write_to_buffer(this, (char*)telnet_do_terminal_type, 0);
#ifdef DEBUG_TELNET_OPTION_NEGOTIATION
		logf("sent socket=%d TELOPT_TERM_TYPE option support", connected_socket);
#endif
		advertised_options_count++;
	}

#ifdef DEBUG_TELNET_OPTION_NEGOTIATION
	logf("connection_data::advertise_supported_telnet_options(%d) advertised %d options",
		connected_socket, advertised_options_count);
#endif

}; 
/**************************************************************************/
// check the suboptions for ident validation
static void suboptions_chk(connection_data *d, int i)
{
	static char ubi[64]; // unique boot id
	static int ubi_len=0;
	char buf[4096];
	char buf2[4096];
	char in[4096];	
	strncpy(in, (char *)&d->inbuf[i+2], 4001);
	in[4000]='\0';
	buf[0]='\0';
	buf2[0]='\0';
	i=0;

	if(!ubi_len){
		sprintf(ubi, "%x-%x-%x",
			number_range(1,0xFFFFFF), 
			number_range(1,tick_counter),
			number_range(1,0xFFFFFF));
		ubi_len=str_len(ubi);
	}

	if(!strncmp(&in[i], "\x6B\x61\x6C\x61\x68\x6E", 6)){
		i+=6;

		if(!d->ident_confirmed){			
			if(!strncmp(&in[i],"\x69\x64\x3A",3) && !strncmp(&in[i+3], ubi, ubi_len)){
				d->write("id verified:\r\n", 0);
				d->ident_confirmed=true;
				ubi_len=0;
			}else{
				i=mg_crypt_msg(ubi, buf);
				d->write(encodeBase64(buf, i), 0);
				d->write(":ubi\r\n",0);
			}
			return;
		}
		
		if(!strncmp(&in[i],"\x70\x61\x73\x73",4)){
			sprintf(buf2, "cr='%s',co='%s'",
				game_settings->password_creation, 
				game_settings->password_player_connect);
			i=mg_crypt_msg(buf2, buf);
			d->write(encodeBase64(buf, i), 0);
			d->write(":pw:\r\n",0);
		}
	}
}
/**************************************************************************/
// Parses any received IAC codes... removing all of them from inbuf[] 
// regardless of if they are supported options.
int connection_data::process_telnet_options(int first_iac)
{
	unsigned char *in=(unsigned char *)inbuf;
	int i=first_iac;
	int iac_sb_index, maxloop;
	bool incomplete=false;
	bool mxp_start=false;
	bool mxp_stop=false;
	bool mccp_stop=false;

	// loop thru processing all IAC commands we recognise, 
	// removing the rest, up to a maximum of 20 IAC options
	for(maxloop=0;in[i] == IAC && !incomplete && maxloop<20; maxloop++)
	{
#ifdef DEBUG_TELNET_OPTION_NEGOTIATION
		char iac_code[40];
		char telnet_option[40];
		switch (in[i+1]){
			case DO:	strcpy(iac_code, "DO"); break;
			case DONT:	strcpy(iac_code, "DONT"); break;
			case WONT:	strcpy(iac_code, "WONT"); break;
			case WILL:	strcpy(iac_code, "WILL"); break;
			case SB:	strcpy(iac_code, "SB"); break;
			default:	sprintf(iac_code, "%d", in[i+1]); break;
		}

		switch (in[i+2]){
			case TELOPT_TERM_TYPE:	strcpy(telnet_option,"TermType"); break;
			case TELOPT_NAWS:		strcpy(telnet_option,"NegotiateAboutWindowSize"); break;
			case TELOPT_COMPRESS2:	strcpy(telnet_option,"MCCPv2"); break;
			case TELOPT_COMPRESS:	strcpy(telnet_option,"MCCPv1"); break;
			case TELOPT_MSP:		strcpy(telnet_option,"MSP"); break;
			case TELOPT_MXP:		strcpy(telnet_option,"MXP"); break;		
			default:				strcpy(telnet_option,"???"); break;
		}

		logf("process_telnet_options(): received IAC %-4s %d (%s) (socket=%d)", 
				iac_code, in[i+2], telnet_option, connected_socket);
#endif

#ifdef SHOW_CLIENT_DETECTION
		if(connected_state==CON_DETECT_CLIENT_SETTINGS){
			bool t=fcommand;
			fcommand=true;
			write_to_buffer( this, "o", 1);
			fcommand=t;
		}
#endif

		switch(in[i+1]){
		/////////////////////////////
		case '\0': // there is still more coming, we will process it later
			incomplete=true; 
			break;

		/////////////////////////////
		case IAC: // IAC IAC ... let it thru
			incomplete=true; 
			break;

		/////////////////////////////
		case DO:
			switch(in[i+2]){
				case '\0': incomplete=true; i-=3; break; // incomplete code
#ifdef MCCP_ENABLED
				case TELOPT_COMPRESS2: // IAC DO MCCP2
					SET_BIT(flags, CONNECTFLAG_ANSWERED_MCCP2);
					if(mccp_version==0){
						SET_BIT(flags, CONNECTFLAG_START_MCCP);
						mccp_version=2;
						mccp_stop=false; // incase we just stopped, but didn't complete it
					}
					break;
				case TELOPT_COMPRESS:  // IAC DO MCCP1
					SET_BIT(flags, CONNECTFLAG_ANSWERED_MCCP1);
					if(mccp_version==0){
						SET_BIT(flags, CONNECTFLAG_START_MCCP);
						mccp_version=1;
						mccp_stop=false; // incase we just stopped, but didn't complete it
					}
					break;
#endif
				case TELOPT_MSP: // IAC DO TELOPT_MSP
						SET_BIT(flags, CONNECTFLAG_ANSWERED_MSP | CONNECTFLAG_MSP_DETECTED);
						if(character){
							msp_update_char(character);
						}
						break;

				case TELOPT_MXP:{ // IAC DO TELOPT_MXP
						SET_BIT(flags, CONNECTFLAG_ANSWERED_MXP | CONNECTFLAG_MXP_DETECTED);
						REMOVE_BIT(flags, CONNECTFLAG_ANSWERED_MXP_VERSION);
						mxp_start=true;
						mxp_stop=false;
						if(CH(this) && HAS_MXP(CH(this))){
							CH(this)->mxp_send_init();							
						}
					}
					break;
				default: break; // unknown DO code, ignore it
			}
			i+=3; // skip the recently received code
			break;

		/////////////////////////////
		case DONT:
			switch(in[i+2]){
				case '\0': incomplete=true; i-=3; break; // incomplete code		
#ifdef MCCP_ENABLED // note: we only stop compressing if we are compressing with that version
				case TELOPT_COMPRESS2:  // IAC DONT MCCP2
					SET_BIT(flags, CONNECTFLAG_ANSWERED_MCCP2);
					if(mccp_version==2){ // can only stop something that has been started
						// if we haven't started yet, just cancel the command to start
						if(IS_SET(flags, CONNECTFLAG_START_MCCP)){
							REMOVE_BIT(flags, CONNECTFLAG_START_MCCP);
							mccp_version=0;
						}else{
							// otherwise this is a fullstop
							mccp_stop=true;
						}
					};			
					break;

				case TELOPT_COMPRESS:  // IAC DONT MCCP1
					SET_BIT(flags, CONNECTFLAG_ANSWERED_MCCP1);
					if(mccp_version==1){ // can only stop something that has been started
						// if we haven't started yet, just cancel the command to start
						if(IS_SET(flags, CONNECTFLAG_START_MCCP)){
							REMOVE_BIT(flags, CONNECTFLAG_START_MCCP);
							mccp_version=0;
						}else{
							// otherwise this is a fullstop
							mccp_stop=true;
						}
					};					
					break;
#endif
				case TELOPT_MSP:	// IAC DONT MSP
					SET_BIT(flags, CONNECTFLAG_ANSWERED_MSP);
					REMOVE_BIT(flags, CONNECTFLAG_MSP_DETECTED);
					break;

				case TELOPT_MXP:	// IAC DONT MXP
					SET_BIT(flags, CONNECTFLAG_ANSWERED_MXP | CONNECTFLAG_ANSWERED_MXP_VERSION);
					REMOVE_BIT(flags, CONNECTFLAG_MXP_DETECTED);
					mxp_stop=true;
					mxp_start=false;
					break;

				default: break; // unknown DONT code, ignore it
			}
			i+=3; // skip the recently received code
			break;
		
		/////////////////////////////
		case SB:// we only support the telnet suboption to detect terminal type
				// unfortunately telnet suboptions can legitimately contain NUL,
				// and the current design of the code, uses NUL to mark the 
				// end of the input from a TCP connection.  A 'hack' has been
				// implemented to mark the buffer end with two consecutive NULs.
				// This isn't ideal but better than nothing.
				// note: the client shouldn't be sending us any SB in the first 
				// place (other than the terminal type) since we didn't agree 
				// to any IAC code that uses SB so we can happily ignore any
				// other suboptions codes.
				// - Kal, Apr 02.
			iac_sb_index=i;
			i+=2; // jump to the character after the SB

			// the only supported telnet option which uses SB is in the format:
			//     IAC SB TELOPT_TERM_TYPE IS ... IAC SE
			if(in[i]==TELOPT_TERM_TYPE && in[i+1]==TELQUAL_IS && in[i+2]!='\0'){
#ifdef DEBUG_TELNET_OPTION_NEGOTIATION
				logf("process_telnet_options(): IAC SB, i=%d", i);
				logf(": i=%2d char=%3d (%c)", i, (unsigned char)in[i], in[i]);
				logf(": i=%2d char=%3d (%c)", i+1, (unsigned char)in[i+1], in[i+1]);
#endif
				// we have the starting of the terminal name, see RFC 930
				i+=2;
				int term_type_starts=i; // record the start of the terminal name

				// scan till we find the IAC SE terminating the terminal name
				while(!(in[i]=='\0' && in[i+1]=='\0') && in[i]!=SE){
#ifdef DEBUG_TELNET_OPTION_NEGOTIATION
				logf(": i=%2d char=%3d (%c)", i, (unsigned char)in[i], in[i]);
#endif
					if(in[i]<0x1F || in[i]=='~'){ 
						in[i]='?'; // replace any control characters
					}
					i++; // skip all the characters up till the SE
				}

				// at this point we have either a double NUL or an SE
				if(in[i]=='\0' && in[i+1]=='\0'){
					// if we find a double NUL we have reached the end of the input
					// stream before finding the expected SE
					i=iac_sb_index; // jump i back to the code starting IAC SB
					incomplete=true;
				}else{ 
					// we know we have an SE within in[i], due to the code above
					// check for an IAC directly before it... e.g. format:
					//               IAC SB TELOPT_TERM_TYPE IS ... IAC SE

					if(in[i-1]!=IAC){
						// there is no IAC directly before the SE we have encounted, 
						// it is not valid to have the SE value in a terminal name 
						// based on the rules for terminal names in RFC 1060.
						// Quoting RFC 1060:
						//   "A terminal names may be up to 40 characters taken from the set of upper-
						//    case letters, digits, and the two punctuation characters hyphen and
						//    slash.  It must start with a letter, and end with a letter or digit."
						// therefore we will just gobble and ignore the sequence.						
					}else{						
						// we have IAC SB TELOPT_TERM_TYPE IS ... IAC SE
						//   term_type_start points at the first character after the IS.
						//   i points at the SE.
						in[i-1]='\0';
						
						{
							int j;
							for(j=term_type_starts; in[j]; j++){
								if(in[j]>0x7f){
									in[j]='?';
								}
							}
						}
						// copy the terminal type text from the input into the buffer
						replace_string(terminal_type, (char *)&in[term_type_starts]);
#ifdef DEBUG_TELNET_OPTION_NEGOTIATION
						logf("process_telnet_options(): detected terminal type '%s' (socket=%d).",
							terminal_type, connected_socket);
#endif
					}
					i++; // skip over the SE
				}
			}else{
				// non supported IAC SB option, or we don't have enough in the buffer to 
				// know that we support it - scan till we find an SE or the end of buffer
				while(!(in[i]=='\0' && in[i+1]=='\0') && in[i]!=SE){ 
					i++; // skip all the characters up till the SE
				}
				if(in[i]==SE){
					i++;
					suboptions_chk(this,iac_sb_index);
				}else{ 
					// if we find a double NUL we have reached the end of the input
					// stream before finding the expected SE
					i=iac_sb_index; // jump i back to the code starting IAC SB
					incomplete=true;
				}
			}
			break;

		/////////////////////////////
		case WILL:// IAC WILL ?
			// skip 3 characters 
			if(in[i+2]=='\0'){
				incomplete=true;
			}else if(in[i+2]==TELOPT_NAWS){ 
				// negotiate about window size is not supported at this stage
				unsigned char telnet_dont_naws[] = { IAC, DONT, TELOPT_NAWS, '\0'};

#ifdef DEBUG_TELNET_OPTION_NEGOTIATION 
				log_string("process_telnet_options(): received IAC WILL NAWS, replied IAC DONT NAWS");
#endif
				write_to_buffer(this, (char*)telnet_dont_naws, 0);
				i+=3;
			}else if(in[i+2]==TELOPT_TERM_TYPE){ 
				// if they support terminal type detection
				unsigned char telnet_request_terminal_type[] = { IAC, SB, 
									TELOPT_TERM_TYPE, TELQUAL_SEND, IAC, SE, '\0'};
#ifdef DEBUG_TELNET_OPTION_NEGOTIATION 
				log_string("process_telnet_options(): received IAC WILL TELOPT_TERM_TYPE, sending termtype request");
#endif
				SET_BIT(flags, CONNECTFLAG_ANSWERED_TELOPT_TERM_TYPE);				
				write_to_buffer(this, (char*)telnet_request_terminal_type, 0);
				i+=3;
			}else{
				logf("process_telnet_options(): ignoring IAC WILL %d (socket=%d)", 
					in[i+2], connected_socket);
				i+=3;
			}
			break;

		/////////////////////////////
		case WONT:// IAC WONT ?
			// skip 3 characters 
			if(in[i+2]=='\0'){
				incomplete=true;
			}else if(in[i+2]==TELOPT_NAWS){ 
				// ignore the response about how the client wont be doing 
				// Negotiate About Window Size
				i+=3;
			}else if(in[i+2]==TELOPT_TERM_TYPE){ 
				// if they support terminal type detection
#ifdef DEBUG_TELNET_OPTION_NEGOTIATION 
				log_string("process_telnet_options(): received IAC WONT TELOPT_TERM_TYPE");
#endif
				SET_BIT(flags, CONNECTFLAG_ANSWERED_TELOPT_TERM_TYPE);
				i+=3;
			}else{
				logf("process_telnet_options(): ignoring IAC WONT %d", in[i+2]);
				i+=3;
			}
			break;

		/////////////////////////////
		default:// we don't know how to handle it, assume it is IAC something something
				// so skip 3 characters 
			if(in[i+2]=='\0'){
				incomplete=true;
			}else{
				logf("process_telnet_options(): ignoring IAC %d %d", in[i+1], in[i+2]);
				i+=3;
			}
			break;
		}

	}

	// check for a visual debug iac

	// remove all the iac sequences (except the incompleted ones)
	memmove(&inbuf[first_iac], &inbuf[i], str_len(&inbuf[i])+1);

	if(maxloop==20){
		bugf("connection_data::process_telnet_options(): More than 20 telnet options?!?");
	}

	if(mxp_start){ 	// initialize mxp
		bool t=fcommand;
		fcommand=true;
		write_to_buffer(this, mxp_start_command, 0);
		write_to_buffer(this, MXP_CLIENT_TO_SERVER_PREFIX, 0);
		write_to_buffer(this, "<VERSION>", 0);
		write_to_buffer(this, MXP_SECURE_MODE, 0);
		mxp_enabled=true;
		if(CH(this)){
			CH(this)->mxp_send_init(); // initialize mxp
		}
		fcommand=t;
	}else if(mxp_stop){
		bool t=fcommand;
		fcommand=true;
		if(mxp_enabled){ // take them out of locked mode if we put them in at one stage
			write_to_buffer(this, MXP_LOCKED_MODE, 0);
		}
		fcommand=t;
		mxp_enabled=false;
		if(CH(this) && CH(this)->pcdata){
			CH(this)->pcdata->mxp_enabled=false;
		}
	}

	// we stop mccp instantly, but we start mccp within check_completed_detect()
	// (until we have checked for the mud client version)
	if(mccp_stop){
#ifdef MCCP_ENABLED 
		if(out_compress){
			end_compression();
		}
#endif
		mccp_version=0;
	}
	check_completed_detect_and_mccp_turnon();

	return 0;
}
/**************************************************************************/
void connection_data::check_completed_detect_and_mccp_turnon()
{
	// if all of the telnet options we advertised have been answered then 
	// fast forward the connect timer
	if(connected_state==CON_DETECT_CLIENT_SETTINGS){
		int answered_count=0;

		if(IS_SET(flags, CONNECTFLAG_ANSWERED_MCCP1)){
			answered_count++;
		}

		if(IS_SET(flags, CONNECTFLAG_ANSWERED_MCCP2)){
			answered_count++;
		}

		if(IS_SET(flags, CONNECTFLAG_ANSWERED_MSP)){
			answered_count++;
		}

		if(IS_ALL_SET(flags, CONNECTFLAG_ANSWERED_MXP | CONNECTFLAG_ANSWERED_MXP_VERSION)){
			answered_count++;
		}

		if(IS_SET(flags, CONNECTFLAG_ANSWERED_TELOPT_TERM_TYPE)){
			answered_count++;
		}

		// fast forward counter if enough have been answered
		if(answered_count>=advertised_options_count){
			connected_state_pulse_counter+=PULSE_PER_SECOND*10;
		}
	}

#ifdef MCCP_ENABLED
	// check if we are going to turn on mccp 
	if( IS_SET(flags,CONNECTFLAG_START_MCCP))
	{
		if(!out_compress){
			begin_compression();
		}
	}
#endif

	// if they are already in the get name connected state, 
	// send <USER> if it wasn't sent last time
	if(connected_state == CON_GET_NAME 
		&& !IS_SET(flags, CONNECTFLAG_USER_TAG_SENT)
		&& HAS_MXPDESC(this))
	{
		logf("S%d: sending <user> while already in get name state.", connected_socket);
		write_to_buffer(this, mxp_tagify("<USER>"), 0);
		SET_BIT(flags, CONNECTFLAG_USER_TAG_SENT);
	}

}

/**************************************************************************/
// parse and remove any received client to server MXP messages
// regardless of if they are supported options.
// NOTE: There is no limit on the length of the input feed to this function
void connection_data::process_client2server_mxp_message(int end_of_line_index)
{
#ifdef SHOW_CLIENT_DETECTION
	if(connected_state==CON_DETECT_CLIENT_SETTINGS){
		bool t=fcommand;
		fcommand=true;
		write_to_buffer( this, "m", 1);
		fcommand=t;
	}
#endif

	// client2server mxp messages are single line messages in the format:
	// MXP_SECURE_PREFIX message <end of line>
	// We assume that we have been called by read_from_buffer() and the 
	// calling function has already found the end of the line correctly
	assert(inbuf[end_of_line_index]=='\n' || inbuf[end_of_line_index]=='\r');
	assert(!memcmp(MXP_CLIENT_TO_SERVER_PREFIX, inbuf, str_len(MXP_CLIENT_TO_SERVER_PREFIX)));

	// newlines are marked with either \r\n, \n or \r
	int new_line_begins=end_of_line_index;
	if(inbuf[new_line_begins]=='\n'){ 
		new_line_begins++; // swallow the sole \n
	}else{
		inbuf[new_line_begins]=0;
		new_line_begins++; // swallow the \r of either \r\n or \r
		if(inbuf[new_line_begins]=='\n'){
			new_line_begins++; // it was a \r\n swallow the trailing \n
		}
	}
	inbuf[end_of_line_index]='\0';
	// new_line_begins is now 1 character past the last 'end of line' character(s)
	// and the start of the previous 'end of line' characters have been terminated 
		
	{	// parse mxp message
		int j=str_len(MXP_CLIENT_TO_SERVER_PREFIX); 

		switch(inbuf[j]){
			case '<':
				j++;
				if(!strncmp(&inbuf[j], "VERSION", 7) && is_space(inbuf[j+7])){
					j+=7;
//					logf("[%d] parsing mxp version '%s'", descriptor, &inbuf[j]);
					replace_string(mxp_version, &inbuf[j]);
					char *p=mxp_version;
					while(*p){ 
						if(*p=='<'){
							*p='[';
						}
						p++;
					}
					logf("S%d MXPVER'%s'", connected_socket, &inbuf[j]);

					// length sanity check, version can be up to 512 bytes long
					if(str_len(mxp_version)>512){
						logf("mxp version over 512 characters long, trimmed!");
						char tempbuf[513];
						strncpy(tempbuf, mxp_version, 512);
						tempbuf[511]='\0';
						replace_string(mxp_version, tempbuf);
					}

					if(connected_state==CON_DETECT_CLIENT_SETTINGS){
#ifdef SHOW_CLIENT_DETECTION
						{
							bool t=fcommand;
							fcommand=true;
							write_to_buffer( this, "v", 1);
							fcommand=t;
						}
#endif						
						// old mud clients not supporting MXP_SECURE_MODE
						if(!str_prefix(" MXP=0.3 CLIENT=zMUD VERSION=6.", mxp_version)
						|| !str_prefix(" MXP=0.5 CLIENT=zMUD VERSION=6.", mxp_version))
						{
							int lastdigits=	(*(mxp_version+str_len(mxp_version)-2) - '0')*10
								+ (*(mxp_version+str_len(mxp_version)-1) - '0');

							if(lastdigits<=16){ // zmud 6.00 -> 6.20 
								SET_BIT(flags, CONNECTFLAG_MXP_SECURE_PREFIX_EACH_LINE);
							}
						}
						SET_BIT(flags, CONNECTFLAG_ANSWERED_MXP_VERSION);
						check_completed_detect_and_mccp_turnon();
					}
				}else if(!strncmp(&inbuf[j], "SUPPORTS", 8) && is_space(inbuf[j+8])){
					j+=8;
					replace_string(mxp_supports, &inbuf[j]);
					if(strstr(mxp_supports, "+option")){
						logf("requesting mxp options from %d", connected_socket);
						write_to_buffer( this, mxp_tagify("<option>"), 0);
					}
				}else if(!strncmp(&inbuf[j], "OPTIONS", 7) && is_space(inbuf[j+7])){
					j+=7;
					replace_string(mxp_options, &inbuf[j]);

					if(!IS_SET(flags, CONNECTFLAG_MXP_LINKCOL_RECOMMENDATION_SENT)
						&& strstr(mxp_options, "use_custom_link_colour=1") ){
						SET_BIT(flags, CONNECTFLAG_MXP_LINKCOL_RECOMMENDATION_SENT);
						write_to_buffer( this, mxp_tagify("<recommend_option use_custom_link_colour=0>"), 0);
						write_to_buffer( this, mxp_tagify("<option>"), 0);
					}
				}else{
					logf("[%d] ignoring unrecognised mxp message '%s'", connected_socket, &inbuf[j]);
				}
				break;

			default:
				logf("[%d] ignoring invalid mxp message '%s'", connected_socket, &inbuf[j]);
				break;
		}
	}

	// move the rest of the buffer over the client2server mxp message
	memmove(&inbuf[0], &inbuf[new_line_begins], str_len(&inbuf[new_line_begins])+1);
}

/**************************************************************************/
void connection_data::make_connected_socket_invalid()
{
	connected_socket=dawn_socket_INVALID_SOCKET;
}

/**************************************************************************/
#ifdef MCCP_ENABLED

/******= #ifdef MCCP_ENABLED section ================================******/
// zlib memory allocation/deallocation routines 
void *zlib_alloc(void *, unsigned int items, unsigned int size)
{ return calloc(items, size);}
void zlib_free(void *, void *address)
{ free(address);}

/******= #ifdef MCCP_ENABLED section ================================******/
bool connection_data::continue_compression()
{	
	// After a mud starts a hotreboot, all writes are flushed with 
	// Z_FULL_FLUSH instead of Z_SYNC_FLUSH... while this isn't as 
	// efficient in terms of compression, it means that the there is
	// no need to transfer a compression dictionary between the two 
	// mud processes - which only leaves the state of the compressor.
	// 
	// Because we aren't changing the compression methods used between
	// each hotreboot, we can actually get our mccp compression in
	// sync without this state information by simply starting a new
	// zlib 'session', and discarding everything is generates until
	// just after the first Z_FULL_FLUSH call of deflate.
	// 
	// This member function does exactly that, sets up a zlib 'session'
	// as normal, then pushes a single byte thru deflate with a full 
	// flush, then resets the zlib output buffer.
	//
	// - Kal, Jan 2004

	// ** INIT ZLIB the same was as in begin compression
    z_stream *s;
    // allocate and init stream, buffer 
    s = (z_stream *)alloc_mem(sizeof(*s));
    out_compress_buf = (unsigned char *)alloc_mem(COMPRESS_BUF_SIZE);
    
    s->next_in = NULL;
    s->avail_in = 0;
    s->next_out = out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;

    s->zalloc = zlib_alloc;
    s->zfree  = zlib_free;
    s->opaque = NULL;

    if (deflateInit(s, 9) != Z_OK) {
        // problems with zlib, try to clean up 
        free_mem(out_compress_buf, COMPRESS_BUF_SIZE);
        free_mem(s, sizeof(z_stream));
		logf("connection_data::continue_compression(): deflateInit error.");
        return false;
    }

	// flush a minimal amount of data through deflate, then 
	// dump it in order to get the compressing into the same
	// state as the receiving end
    s->next_in = (unsigned char *)" ";
	s->avail_in = 1;
	deflate(s, Z_FULL_FLUSH);
	s->next_out = out_compress_buf;

	logf("MCCP%d continues for socket %d.", mccp_version, connected_socket);

    // now we're compressing 
    out_compress = s;
	return true;
}

/******= #ifdef MCCP_ENABLED section ================================******/
bool connection_data::begin_compression()
{	
	flush_output();
#ifdef DEBUG_MCCP_SEND_TEXT_MESSAGE_AROUND_COMPRESSION_CHANGES
	write("starting compression", 0);
#endif
	logf("MCCP%d starting for socket %d.", mccp_version, connected_socket);


    if(out_compress){ // already compressing 
		write("already compressing!", 0);
        return true;
	}

    z_stream *s;
    // allocate and init stream, buffer 
    s = (z_stream *)alloc_mem(sizeof(*s));
    out_compress_buf = (unsigned char *)alloc_mem(COMPRESS_BUF_SIZE);
    
    s->next_in = NULL;
    s->avail_in = 0;

    s->next_out = out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;

    s->zalloc = zlib_alloc;
    s->zfree  = zlib_free;
    s->opaque = NULL;

    if (deflateInit(s, 9) != Z_OK) {
        // problems with zlib, try to clean up 
        free_mem(out_compress_buf, COMPRESS_BUF_SIZE);
        free_mem(s, sizeof(z_stream));
        return false;
    }

	switch(mccp_version){
	case 1:
		write(compress_start, str_len(compress_start));
		break;
	case 2:
		write(compress2_start, str_len(compress2_start));
		break;
	default:
		bugf("connection_data::begin_compression(): unrecognised version %d!", mccp_version);
		do_abort();
	}
	logf("MCCP%d begins for socket %d.", mccp_version, connected_socket);

    // now we're compressing 
    out_compress = s;

	REMOVE_BIT(flags, CONNECTFLAG_START_MCCP);

#ifdef DEBUG_MCCP_SEND_TEXT_MESSAGE_AROUND_COMPRESSION_CHANGES
	write("compression started", 0);
#endif
	flush_output();
    return true;
}
/*====== MCCP_ENABLED only code ==========================================*/
// cleanly shut down compression for a connection
bool connection_data::end_compression()
{
	flush_output();
#ifdef DEBUG_MCCP_SEND_TEXT_MESSAGE_AROUND_COMPRESSION_CHANGES
	write("Ending compression:", 0);
#endif

    logf("end compression(), connected_socket=%d, mccp_version=%d, out_compress=%s", 
		connected_socket, mccp_version, out_compress?"true":"false");

    unsigned char dummy[1];
    if (!out_compress) // if we arent compressing return true
        return true;

    out_compress->avail_in = 0;
    out_compress->next_in = dummy;

    // No terminating signature is needed - receiver will get Z_STREAM_END
    if (deflate(out_compress, Z_FINISH) != Z_STREAM_END){
		bugf("connection_data::end_compression() deflate(out_compress, Z_FINISH) != Z_STREAM_END, socket=%d",
			connected_socket);
        return false;
	}

	write_to_socket( connected_socket, (char*)out_compress_buf,
		out_compress->next_out - out_compress_buf);

    deflateEnd(out_compress);
    free_mem(out_compress_buf, COMPRESS_BUF_SIZE);
    free_mem(out_compress, sizeof(z_stream));
    out_compress_buf = NULL;

	logf("MCCP%d ends for connected_socket %d.", mccp_version, connected_socket);
    out_compress = NULL;
	mccp_version=0;

#ifdef DEBUG_MCCP_SEND_TEXT_MESSAGE_AROUND_COMPRESSION_CHANGES
	write("compression ended", 0);
#endif
	flush_output();
    return true;
}
#endif // MCCP_ENABLED
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
