/**************************************************************************/
// comm.h - header for comm.cpp (funny that)
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with all the licenses *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 ***************************************************************************
 * >> Original Diku Mud copyright (c)1990, 1991 by Sebastian Hammer,       *
 *    Michael Seifert, Hans Henrik Staerfeldt, Tom Madsen, & Katja Nyboe.  *
 * >> Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael       *
 *    Chastain, Michael Quan, and Mitchell Tse.                            *
 * >> ROM 2.4 is copyright 1993-1995 Russ Taylor and has been brought to   *
 *    you by the ROM consortium: Russ Taylor(rtaylor@pacinfo.com),         *
 *    Gabrielle Taylor(gtaylor@pacinfo.com) & Brian Moore(rom@rom.efn.org) *
 * >> Oblivion 1.2 is copyright 1996 Wes Wagner                            *
 **************************************************************************/

/*
 * comm.cpp contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_connection ---> Read
 *    Game_loop ---> Read_from_buffer
 *                       
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_connection -> Write
 *
 */
#ifndef COMM_H
#define COMM_H

#include "include.h" // dawn standard includes
#ifdef WIN32
#include <direct.h>
#endif

#include "resolve.h"

// command procedures needed 
DECLARE_DO_FUN(do_updatemagic);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_who       );
void laston_save(char_data *);
void init_alarm_handler();
void alarm_update();

// lastcomm.c
void install_other_handlers ();

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif


#if defined(apollo)
#undef __attribute
#endif


/*
 * Socket and TCP/IP stuff.
 */

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
#include <signal.h> 
#include <arpa/inet.h>

#if !defined( STDOUT_FILENO )
#define STDOUT_FILENO 1
#endif

#ifdef HAVE_SYS_WAIT_H 
#include <sys/wait.h>
#endif

pid_t   fork            args( ( void ) );
int     kill            args( ( pid_t pid, int sig ) );
int     pipe            args( ( int filedes[2] ) );
int     dup2            args( ( int oldfd, int newfd ) );
int     execl           args( ( const char *path, const char *arg, ... ) );
int     execlp          args( ( const char *file, const char *arg, ... ) );
//unsigned long inet_addr args( ( const char *cp ) );
#endif // unix



/*
 * OS-dependent declarations.
 */

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	defined(linux)

int	close		args( ( int fd ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif


#if	defined(MIPS_OS)
	extern	int		errno;
#endif

#if	defined(NeXT)
	int	close		args( ( int fd ) );
	int	fcntl		args( ( int fd, int cmd, int arg ) );
	#if	!defined(htons)
		u_short	htons		args( ( u_short hostshort ) );
	#endif
	#if	!defined(ntohl)
		u_long	ntohl		args( ( u_long hostlong ) );
	#endif
	int	read		args( ( int fd, char *buf, int nbyte ) );
	int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
					fd_set *exceptfds, struct timeval *timeout ) );
	int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
	int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
	int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
	int	close		args( ( int fd ) );
	int	fcntl		args( ( int fd, int cmd, int arg ) );
	int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
	int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
	int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
	#if	!defined(htons)
		u_short	htons		args( ( u_short hostshort ) );
	#endif
	int	listen		args( ( int s, int backlog ) );
	#if	!defined(ntohl)
		u_long	ntohl		args( ( u_long hostlong ) );
	#endif
	int	read		args( ( int fd, char *buf, int nbyte ) );
	int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
					fd_set *exceptfds, struct timeval *timeout ) );
	int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
					int optlen ) );
	int	socket		args( ( int domain, int type, int protocol ) );
	int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

// This includes Solaris Sys V as well
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

// Laston related function prototypes 
void laston_login args((char_data *)); // records when someone logs on 
void laston_update_char args((char_data *)); // updates when reconnect with hotreboot

// prototypes from act_wiz.c
void do_invis( char_data *ch, char *argument );

// main loop of the game
void game_loop();

bool	read_from_connection	args( ( connection_data *c ) );

// mud ftp stuff
void    greet_ftp( connection_data *c );
void 	handle_ftp_data( connection_data *, const char *);
void 	handle_ftp_auth( connection_data *, const char *);
void 	handle_ftp_command( connection_data *, const char *);

char *get_compile_time (bool show_parent_codebase_version);

// Other functions in comm.c, global.c and nanny.c (OS-independent).
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( connection_data *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( connection_data *d, char *name ) );
int		main			args( ( int argc, char **argv ) );
void	nanny			args( ( connection_data *d, char *argument ) );
bool	process_output		args( ( connection_data *d, bool fPrompt ) );
void	read_from_buffer	args( ( connection_data *d ) );
void	stop_idling		args( ( char_data *ch ) );
void    init_globals    (char *exename); // called in comm.c

#ifdef WIN32
	#include "telnet.h"
	void gettimeofday( struct timeval *tp, void *tzp );
	#include <fcntl.h>
	#include <signal.h> 
#endif


#ifndef EWOULDBLOCK
	#define EWOULDBLOCK 35
#endif

#endif // COMM_H
