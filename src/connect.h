/**************************************************************************/
// connect.h - connection_data class
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef CONNECT_H
#define CONNECT_H

/**************************************************************************/
//typedef unsigned int SOCKET;
/**************************************************************************/
#ifdef __CYGWIN__ // cygwin zlib support disabled due to compile errors
#	define DISABLE_MCCP
#endif

#ifndef DISABLE_MCCP // define this in your project to disable MCCP
#	ifdef WIN32 
#		ifdef WIN32_MCCP_ENABLED 
#			define MCCP_ENABLED 
#		endif
#	endif
#	ifdef unix
#		ifdef HAVE_ZLIB
#			define MCCP_ENABLED 
#		endif
#	endif
#endif // DISABLE_MCCP

#ifdef MCCP_ENABLED
#	include <zlib.h>
#	define COMPRESS_BUF_SIZE	(16384)
#	ifdef WIN32
#		ifdef _DEBUG
#			pragma comment( lib, "zlib_debug.lib") 
#		else
#			pragma comment( lib, "zlib_release.lib") 
#		endif
#	endif
#endif

#define	INBUF_SIZE			(4*MSL)
/**************************************************************************/
// connection (channel) class.
// bm_connection_data
class connection_data
{
public:
	int write(const char *txt, int length ); // write to the socket, passing thru MCCP where appropriate
	int write_colour(const char *txt, int length ); // write to the socket in colour, passing thru MCCP where appropriate
	void check_completed_detect_and_mccp_turnon();
	bool send_outbuf(); // called by process_output
	void advertise_supported_telnet_options(); // send info on stuff like MCCP support etc (IAC signals basically)
	void close_socket();
	int process_telnet_options(int i);
	void process_client2server_mxp_message(int end_of_line_index);
	char *mxp_version;
	char *mxp_supports;
	char *mxp_options;
	char *terminal_type; // retrieved according to RFC 930/1060
	bool flush_output();
	void make_connected_socket_invalid();

private:
	void send_will_telnet_option( unsigned char option_value);

public:
    connection_data *	next;
	// snoop 
    connection_data *	snoop_by;
    connection_data *	command_snoop;

	// web related connection data
	web_request_data *web_request;

	// character and switch
	char_data *	character;
    char_data *	original;
    bool		valid;
	int			connected_socket;
    sh_int		connected_state;
    bool		fcommand;
    char		inbuf       [INBUF_SIZE];
	char		incomm      [MIL*2]; // *2 to prevent buffer overflows
    char		inlast      [MIL*2];
    int			repeat;

	// holds data to be sent to connection (after colour parsing in write_into_buffer)
    char *		outbuf;
    int			outsize;
    int			outtop;
	
    char *		showstr_head;
    char *		showstr_point;
    void *		pEdit;
    char **		pString;
    int			editor;
	int *		changed_flag;
	
	// ip addresses relating to the connection
	PROTOCOL_TYPE protocol; // PROTOCOL_IPV4 or PROTOCOL_IPV6
	CONNECTION_TYPE contype;

	// ip and dns information about the connected socket
	char *		local_ip;
	int			local_port;
	char *		local_tcp_pair; // ipv4:port or [ipv6]:port
	char *		remote_ip;
	int			remote_port;
	char *		remote_tcp_pair; // ipv4:port or [ipv6]:port
	char *		remote_hostname; // what the ip address resolves to
	
	// ident results for the connected socket
    char *		ident_username;	// username according to ident resolver
	char *		ident_raw_result; // raw ident response
	bool		ident_confirmed; // true if we know who we are talking to
	bool		resolved;		// set to true when match_hnames returns
	int			connected_state_pulse_counter;// used to prevent it taking longer than 60 seconds
		 
	char *      ip; // required for old code for nw
	
	int			newbie_creating;
	bool		multiple_logins;
	bool		warned_about_idle;
	time_t		idle_since; // records the time of their last input


	// colour system
	COLOUR_TYPE	colour_mode;
	COLOUR_MEMORY_TYPE colour_memory;
	bool parse_colour;
	bool flashing_disabled;

	// visual debugging system
	bool visual_debugging_enabled;
	bool visual_debug_hexoutput;
	bool visual_debug_flush_before_prompt;
	bool visual_debug_strip_prompt;
	bool visual_debug_user;
	int	 visual_debug_column_width;
	int	 visual_debug_buffer_size;
	char *visual_debug_buffer;

	// condensing system
	int condense_lastlen;
	int	condense_count;
	char condense_buffer[MAX_CONDENSE_LENGTH];

	// MUDFTP stuff
    char *		username;
    struct
    {
        char *		filename;   // Filename being written to 
        char *		data;       // Data being written 	 
        short int 	lines_left; // Lines left 		 
        short int 	lines_sent; // Lines sent so far 
        ftp_mode 	mode;       // FTP_xxx 		 	 
		bool		inuse;
    } ftp;

	// Speedwalking
	char		*speedwalk_buf;
	char		*speedwalk_head;

	// Remort system
	sh_int		creation_remort_number;

	// multiple attempts at a password
	unsigned char	wrong_password_count;

	int flags;
	sh_int advertised_options_count;

	bool mxp_enabled; // mxp enabled status

#ifdef MCCP_ENABLED
	bool begin_compression();
	bool continue_compression(); // for after a hotreboot
	bool end_compression();
	// mccp support
	z_stream * out_compress;
	unsigned char * out_compress_buf;
#endif // MCCP_ENABLED

	short mccp_version; // has to be included in non MCCP_ENABLED versions 
						// of the mud because a previous copy of the mud
						// may be MCCP enabled and transfer an mccp_version 
						// value across a hotreboot - Kal
	unsigned int bytes_sent;
	unsigned int bytes_sent_before_compression;
	unsigned int bytes_sent_after_compression;

};

// connection flags
#define CONNECTFLAG_MSP_DETECTED					(A)
#define CONNECTFLAG_MXP_DETECTED					(B)
#define CONNECTFLAG_MXP_SECURE_PREFIX_EACH_LINE		(C)
#define CONNECTFLAG_START_MCCP						(D)
#define CONNECTFLAG_ANSWERED_MCCP1					(E)
#define CONNECTFLAG_ANSWERED_MCCP2					(F)
#define CONNECTFLAG_ANSWERED_MSP					(G)
#define CONNECTFLAG_ANSWERED_MXP					(H)
#define CONNECTFLAG_ANSWERED_MXP_VERSION			(I)
#define CONNECTFLAG_USER_TAG_SENT					(J)
#define CONNECTFLAG_MXP_LINKCOL_RECOMMENDATION_SENT (K)
#define CONNECTFLAG_ANSWERED_TELOPT_TERM_TYPE		(L)

#endif // CONNECT_H
