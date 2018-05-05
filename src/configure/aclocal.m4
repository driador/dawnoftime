dnl ##################################################################
dnl Parts of this macro came from aclocal.m4 in the sample code 
dnl associated with "UNIX Network Programming: Volume 1", Second 
dnl Edition, by W. Richard Stevens.  That sample code is available 
dnl via ftp://ftp.kohala.com/pub/rstevens/unpv12e.tar.gz.  
dnl
dnl We cannot use the AC_CHECK_TYPE macros because AC_CHECK_TYPE
dnl #includes only <sys/types.h>, <stdlib.h>, and <stddef.h>.
dnl Unfortunately, many implementations today hide typedefs in wierd
dnl locations: Solaris 2.5.1 has uint8_t and uint32_t in <pthread.h>.
dnl SunOS 4.1.x has int8_t in <sys/bitypes.h>.
dnl So we define our own macro AC_DAWN_CHECK_TYPE that includes more,
dnl then looks for the typedef.
dnl
dnl This macro should be invoked after all the header checks have been
dnl performed, since we #include "confdefs.h" below, and then use the
dnl HAVE_foo_H values that it can #define.
dnl
dnl The tail end (IPv6) code is based on the APR build dnl system 
dnl APR = Apache Portable Runtime

AC_DEFUN(AC_DAWN_CHECK_TYPE,
	[AC_MSG_CHECKING(if $1 defined)
	AC_CACHE_VAL(ac_cv_type_$1,
		AC_TRY_COMPILE(
[
#include	"confdefs.h"	/* the header built by configure so far */
#ifdef	HAVE_SYS_TYPES_H
#  include	<sys/types.h>
#endif
#ifdef	HAVE_SYS_SOCKET_H
#  include	<sys/socket.h>
#endif
#ifdef	HAVE_SYS_TIME_H
#  include    <sys/time.h>
#endif
#ifdef	HAVE_NETINET_IN_H
#  include    <netinet/in.h>
#endif
#ifdef	HAVE_ARPA_INET_H
#  include    <arpa/inet.h>
#endif
#ifdef	HAVE_ERRNO_H
#  include    <errno.h>
#endif
#ifdef	HAVE_FCNTL_H
#  include    <fcntl.h>
#endif
#ifdef	HAVE_NETDB_H
#  include	<netdb.h>
#endif
#ifdef	HAVE_SIGNAL_H
#  include	<signal.h>
#endif
#ifdef	HAVE_STDIO_H
#  include	<stdio.h>
#endif
#ifdef	HAVE_STDLIB_H
#  include	<stdlib.h>
#endif
#ifdef	HAVE_STRING_H
#  include	<string.h>
#endif
#ifdef	HAVE_SYS_STAT_H
#  include	<sys/stat.h>
#endif
#ifdef	HAVE_SYS_UIO_H
#  include	<sys/uio.h>
#endif
#ifdef	HAVE_UNISTD_H
#  include	<unistd.h>
#endif
#ifdef	HAVE_SYS_WAIT_H
#  include	<sys/wait.h>
#endif
#ifdef	HAVE_SYS_UN_H
#  include	<sys/un.h>
#endif
#ifdef	HAVE_SYS_SELECT_H
# include   <sys/select.h>
#endif
#ifdef	HAVE_STRINGS_H
# include   <strings.h>
#endif
#ifdef	HAVE_SYS_IOCTL_H
# include   <sys/ioctl.h>
#endif
#ifdef	HAVE_SYS_FILIO_H
# include   <sys/filio.h>
#endif
#ifdef	HAVE_SYS_SOCKIO_H
# include   <sys/sockio.h>
#endif
#ifdef	HAVE_PTHREAD_H
#  include	<pthread.h>
#endif],
		[ $1 foo ],
		ac_cv_type_$1=yes,
		ac_cv_type_$1=no))
	AC_MSG_RESULT($ac_cv_type_$1)
	if test $ac_cv_type_$1 = no ; then
		AC_DEFINE($1, $2)
	fi
])

AC_DEFUN(
        [CHECK_GNU_MAKE], [ AC_CACHE_CHECK( for GNU make,_cv_gnu_make_command,
                _cv_gnu_make_command='' ;
dnl Search all the common names for GNU make
                for a in "$MAKE" make ; do
                        if test -z "$a" ; then continue ; fi ;
                        if  ( sh -c "$a --version" 2>&1 | grep GNU  2>&1 > /dev/null ) ;  then
                                _cv_gnu_make_command=$a ;
                                break;
                        fi
                done ;
        ) ;
dnl If there was a GNU version, then set @ifGNUmake@ to the empty string, '#' otherwise
        if test  "x$_cv_gnu_make_command" != "x"  ; then
                ifGNUmake='' ;
                ifBSDmake='#' ;
                ifSTDmake='#' ;
                AC_MSG_RESULT(Found);
        else
                ifGNUmake='#' ;
                AC_MSG_RESULT(Not found);
        fi
        AC_SUBST(ifGNUmake)
        AC_SUBST(ifBSDmake)
        AC_SUBST(ifSTDmake)

] )


AC_DEFUN(
        [CHECK_BSD_MAKE], [ AC_CACHE_CHECK( for BSD make,_cv_bsd_make_command,
                _cv_bsd_make_command='' ;
dnl Search all the common names for BSD make
                for a in "$MAKE" make ; do
                        if test -z "$a" ; then continue ; fi ;
                        if  ( sh -c "$a -d A -f nosuchmakefilename " 2>&1 | grep "lobal.*\=" 2>&1 | grep BSD 2>&1 > /dev/null ) ;  then
                        	if  ( sh -c "$a -d A -f nosuchmakefilename " 2>&1 | grep "\.CURDIR" 2>&1 > /dev/null ) ;  then
                               	 	_cv_bsd_make_command=$a ;
                                	break;
				fi
                        fi
                done ;
        ) ;
dnl If there was a BSD version, then set variables accordingly
        if test  "x$_cv_bsd_make_command" != "x"  ; then
                ifBSDmake='' ;
                ifSTDmake='#' ;
                AC_MSG_RESULT(Found);
        else
                ifBSDmake='#' ;
                AC_MSG_RESULT(Not found);
        fi
        AC_SUBST(ifGNUmake)
        AC_SUBST(ifBSDmake)
        AC_SUBST(ifSTDmake)
] )



dnl
dnl check for working getaddrinfo()
dnl
dnl Note that if the system doesn't have gai_strerror(), we
dnl can't use getaddrinfo() because we can't get strings
dnl describing the error codes.
dnl
AC_DEFUN(APR_CHECK_WORKING_GETADDRINFO,[
  AC_CACHE_CHECK(for working getaddrinfo, ac_cv_working_getaddrinfo,[
  AC_TRY_RUN( [
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <stdlib.h>

void main(void) {
    struct addrinfo hints, *ai;
    int error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo("127.0.0.1", NULL, &hints, &ai);
    if (error) {
        exit(1);
    }
    if (ai->ai_addr->sa_family != AF_INET) {
        exit(1);
    }
    exit(0);
}
],[
  ac_cv_working_getaddrinfo="yes"
],[
  ac_cv_working_getaddrinfo="no"
],[
  ac_cv_working_getaddrinfo="yes"
])])
if test "$ac_cv_working_getaddrinfo" = "yes"; then
  if test "$ac_cv_func_gai_strerror" != "yes"; then
    ac_cv_working_getaddrinfo="no"
  else
    AC_DEFINE(HAVE_GETADDRINFO, 1, [Define if getaddrinfo exists and works well enough for APR])
  fi
fi
])

dnl
dnl check for working getnameinfo()
dnl
AC_DEFUN(APR_CHECK_WORKING_GETNAMEINFO,[
  AC_CACHE_CHECK(for working getnameinfo, ac_cv_working_getnameinfo,[
  AC_TRY_RUN( [
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <stdlib.h>

void main(void) {
    struct sockaddr_in sa;
    char hbuf[256];
    int error;

    sa.sin_family = AF_INET;
    sa.sin_port = 0;
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
#ifdef SIN6_LEN
    sa.sin_len = sizeof(sa);
#endif

    error = getnameinfo((const struct sockaddr *)&sa, sizeof(sa),
                        hbuf, 256, NULL, 0,
                        NI_NUMERICHOST);
    if (error) {
        exit(1);
    } else {
        exit(0);
    }
}
],[
  ac_cv_working_getnameinfo="yes"
],[
  ac_cv_working_getnameinfo="no"
],[
  ac_cv_working_getnameinfo="yes"
])])
if test "$ac_cv_working_getnameinfo" = "yes"; then
  AC_DEFINE(HAVE_GETNAMEINFO, 1, [Define if getnameinfo exists])
fi
])

dnl
dnl check for negative error codes for getaddrinfo()
dnl
AC_DEFUN(APR_CHECK_NEGATIVE_EAI,[
  AC_CACHE_CHECK(for negative error codes for getaddrinfo, ac_cv_negative_eai,[
  AC_TRY_RUN( [
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <stdlib.h>

void main(void) {
    if (EAI_ADDRFAMILY < 0) {
        exit(0);
    }
    exit(1);
}
],[
  ac_cv_negative_eai="yes"
],[
  ac_cv_negative_eai="no"
],[
  ac_cv_negative_eai="no"
])])
if test "$ac_cv_negative_eai" = "yes"; then
  AC_DEFINE(NEGATIVE_EAI, 1, [Define if EAI_ error codes from getaddrinfo are negative])
fi
])

dnl
dnl check for presence of  retrans/retry variables in the res_state structure
dnl
AC_DEFUN(APR_CHECK_RESOLV_RETRANS,[
  AC_CACHE_CHECK(for presence of retrans/retry fields in res_state/resolv.h , ac_cv_retransretry,[
  AC_TRY_RUN( [
#include <sys/types.h>
#if defined(__sun__)
#include <inet/ip.h>
#endif
#include <stdlib.h>

#include <resolv.h>
/* _res is a global defined in resolv.h */
int main(void) {
    _res.retrans = 2;
    _res.retry = 1;
    exit(0);
    return 0;
}
],[
  ac_cv_retransretry="yes"
],[
  ac_cv_retransretry="no"
],[
  ac_cv_retransretry="no"
])])
if test "$ac_cv_retransretry" = "yes"; then
  AC_DEFINE(RESOLV_RETRANSRETRY, 1, [Define if resolv.h's res_state has the fields retrans/rety])
fi
])

dnl
dnl Checks the definition of gethostbyname_r and gethostbyaddr_r
dnl which are different for glibc, solaris and assorted other operating
dnl systems
dnl
dnl Note that this test is executed too early to see if we have all of
dnl the headers.
AC_DEFUN(APR_CHECK_GETHOSTBYNAME_R_STYLE,[

dnl Try and compile a glibc2 gethostbyname_r piece of code, and set the
dnl style of the routines to glibc2 on success
AC_CACHE_CHECK([style of gethostbyname_r routine], ac_cv_gethostbyname_r_style,
APR_TRY_COMPILE_NO_WARNING([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
],[
int tmp = gethostbyname_r((const char *) 0, (struct hostent *) 0,
                          (char *) 0, 0, (struct hostent **) 0, &tmp);
], ac_cv_gethostbyname_r_style=glibc2, ac_cv_gethostbyname_r_style=none))

if test "$ac_cv_gethostbyname_r_style" = "glibc2"; then
    AC_DEFINE(GETHOSTBYNAME_R_GLIBC2, 1, [Define if gethostbyname_r has the glibc style])
fi

AC_CACHE_CHECK([3rd argument to the gethostbyname_r routines], ac_cv_gethostbyname_r_arg,
APR_TRY_COMPILE_NO_WARNING([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
],[
int tmp = gethostbyname_r((const char *) 0, (struct hostent *) 0,
                          (struct hostent_data *) 0);],
ac_cv_gethostbyname_r_arg=hostent_data, ac_cv_gethostbyname_r_arg=char))

if test "$ac_cv_gethostbyname_r_arg" = "hostent_data"; then
    AC_DEFINE(GETHOSTBYNAME_R_HOSTENT_DATA, 1, [Define if gethostbyname_r has the hostent_data for the third argument])
fi
])

dnl
dnl see if TCP_NODELAY setting is inherited from listening sockets
dnl
AC_DEFUN(APR_CHECK_TCP_NODELAY_INHERITED,[
  AC_CACHE_CHECK(if TCP_NODELAY setting is inherited from listening sockets, ac_cv_tcp_nodelay_inherited,[
  AC_TRY_RUN( [
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif
int main(void) {
    int listen_s, connected_s, client_s;
    int listen_port, rc;
    struct sockaddr_in sa;
    socklen_t sa_len;
    socklen_t option_len;
    int option;

    listen_s = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_s < 0) {
        perror("socket");
        exit(1);
    }
    option = 1;
    rc = setsockopt(listen_s, IPPROTO_TCP, TCP_NODELAY, &option, sizeof option);
    if (rc < 0) {
        perror("setsockopt TCP_NODELAY");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
#ifdef BEOS
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
    /* leave port 0 to get ephemeral */
    rc = bind(listen_s, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0) {
        perror("bind for ephemeral port");
        exit(1);
    }
    /* find ephemeral port */
    sa_len = sizeof(sa);
    rc = getsockname(listen_s, (struct sockaddr *)&sa, &sa_len);
    if (rc < 0) {
        perror("getsockname");
        exit(1);
    }
    listen_port = sa.sin_port;
    rc = listen(listen_s, 5);
    if (rc < 0) {
        perror("listen");
        exit(1);
    }
    client_s = socket(AF_INET, SOCK_STREAM, 0);
    if (client_s < 0) {
        perror("socket");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port   = listen_port;
#ifdef BEOS
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
    /* leave sin_addr all zeros to use loopback */
    rc = connect(client_s, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0) {
        perror("connect");
        exit(1);
    }
    sa_len = sizeof sa;
    connected_s = accept(listen_s, (struct sockaddr *)&sa, &sa_len);
    if (connected_s < 0) {
        perror("accept");
        exit(1);
    }
    option_len = sizeof option;
    rc = getsockopt(connected_s, IPPROTO_TCP, TCP_NODELAY, &option, &option_len);
    if (rc < 0) {
        perror("getsockopt");
        exit(1);
    }
    if (!option) {
        fprintf(stderr, "TCP_NODELAY is not set in the child.\n");
        exit(1);
    }
    return 0;
}
],[
    ac_cv_tcp_nodelay_inherited="yes"
],[
    ac_cv_tcp_nodelay_inherited="no"
],[
    ac_cv_tcp_nodelay_inherited="yes"
])])
if test "$ac_cv_tcp_nodelay_inherited" = "yes"; then
    tcp_nodelay_inherited=1
else
    tcp_nodelay_inherited=0
fi
])

dnl
dnl see if O_NONBLOCK setting is inherited from listening sockets
dnl
AC_DEFUN(APR_CHECK_O_NONBLOCK_INHERITED,[
  AC_CACHE_CHECK(if O_NONBLOCK setting is inherited from listening sockets, ac_cv_o_nonblock_inherited,[
  AC_TRY_RUN( [
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
int main(void) {
    int listen_s, connected_s, client_s;
    int listen_port, rc;
    struct sockaddr_in sa;
    socklen_t sa_len;

    listen_s = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_s < 0) {
        perror("socket");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
#ifdef BEOS
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
    /* leave port 0 to get ephemeral */
    rc = bind(listen_s, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0) {
        perror("bind for ephemeral port");
        exit(1);
    }
    /* find ephemeral port */
    sa_len = sizeof(sa);
    rc = getsockname(listen_s, (struct sockaddr *)&sa, &sa_len);
    if (rc < 0) {
        perror("getsockname");
        exit(1);
    }
    listen_port = sa.sin_port;
    rc = listen(listen_s, 5);
    if (rc < 0) {
        perror("listen");
        exit(1);
    }
    rc = fcntl(listen_s, F_SETFL, O_NONBLOCK);
    if (rc < 0) {
        perror("fcntl(F_SETFL)");
        exit(1);
    }
    client_s = socket(AF_INET, SOCK_STREAM, 0);
    if (client_s < 0) {
        perror("socket");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port   = listen_port;
#ifdef BEOS
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
    /* leave sin_addr all zeros to use loopback */
    rc = connect(client_s, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0) {
        perror("connect");
        exit(1);
    }
    sa_len = sizeof sa;
    connected_s = accept(listen_s, (struct sockaddr *)&sa, &sa_len);
    if (connected_s < 0) {
        perror("accept");
        exit(1);
    }
    rc = fcntl(connected_s, F_GETFL, 0);
    if (rc < 0) {
        perror("fcntl(F_GETFL)");
        exit(1);
    }
    if (!(rc & O_NONBLOCK)) {
        fprintf(stderr, "O_NONBLOCK is not set in the child.\n");
        exit(1);
    }
    return 0;
}
],[
    ac_cv_o_nonblock_inherited="yes"
],[
    ac_cv_o_nonblock_inherited="no"
],[
    ac_cv_o_nonblock_inherited="yes"
])])
if test "$ac_cv_o_nonblock_inherited" = "yes"; then
    o_nonblock_inherited=1
else
    o_nonblock_inherited=0
fi
])

dnl
dnl check for socklen_t, fall back to unsigned int
dnl
AC_DEFUN(APR_CHECK_SOCKLEN_T,[
AC_CACHE_CHECK(for socklen_t, ac_cv_socklen_t,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
],[
socklen_t foo = (socklen_t) 0;
],[
    ac_cv_socklen_t=yes
],[
    ac_cv_socklen_t=no
])
])

if test "$ac_cv_socklen_t" = "yes"; then
  AC_DEFINE(HAVE_SOCKLEN_T, 1, [Whether you have socklen_t])
fi
])


AC_DEFUN(APR_CHECK_INET_ADDR,[
AC_CACHE_CHECK(for inet_addr, ac_cv_func_inet_addr,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
],[
inet_addr("127.0.0.1");
],[
    ac_cv_func_inet_addr=yes
],[
    ac_cv_func_inet_addr=no
])
])

if test "$ac_cv_func_inet_addr" = "yes"; then
  have_inet_addr=1
else
  have_inet_addr=0
fi
])


AC_DEFUN(APR_CHECK_INET_NETWORK,[
AC_CACHE_CHECK(for inet_network, ac_cv_func_inet_network,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
],[
inet_network("127.0.0.1");
],[
    ac_cv_func_inet_network=yes
],[
    ac_cv_func_inet_network=no
])
])

if test "$ac_cv_func_inet_network" = "yes"; then
  have_inet_network=1
else
  have_inet_network=0
fi
])


AC_DEFUN(APR_CHECK_SOCKADDR_IN6,[
AC_CACHE_CHECK(for sockaddr_in6, ac_cv_define_sockaddr_in6,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
],[
struct sockaddr_in6 sa;
],[
    ac_cv_define_sockaddr_in6=yes
],[
    ac_cv_define_sockaddr_in6=no
])
])

if test "$ac_cv_define_sockaddr_in6" = "yes"; then
  have_sockaddr_in6=1
else
  have_sockaddr_in6=0
fi
])

dnl
dnl APR_INADDR_NONE
dnl
dnl checks for missing INADDR_NONE macro
dnl
AC_DEFUN(APR_INADDR_NONE,[
  AC_CACHE_CHECK(whether system defines INADDR_NONE, ac_cv_inaddr_none,[
  AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
],[
unsigned long foo = INADDR_NONE;
],[
    ac_cv_inaddr_none=yes
],[
    ac_cv_inaddr_none=no
])])
  if test "$ac_cv_inaddr_none" = "no"; then
    apr_inaddr_none="((unsigned int) 0xffffffff)"
  else
    apr_inaddr_none="INADDR_NONE"
  fi
])


dnl
dnl APR_H_ERRNO_COMPILE_CHECK
dnl
AC_DEFUN(APR_H_ERRNO_COMPILE_CHECK,[
  if test x$1 != x; then
    CPPFLAGS="-D$1 $CPPFLAGS"
  fi
  AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
],[
int h_e = h_errno;
],[
  if test x$1 != x; then
    ac_cv_h_errno_cppflags="$1"
  else
    ac_cv_h_errno_cppflags=yes
  fi
],[
  ac_cv_h_errno_cppflags=no
])])

dnl #################################################################################
AC_DEFUN(CHECK_DEPRECATED_CONST_STRING_CONVERSION, [
	AC_MSG_CHECKING(for compiler deprecated const string conversion warnings)
    NoWriteStringsSetting=""
    if test -n "$CXX"; then
		cat > conftest.c <<EOF
int main(int argc, char **argv) { char *c=""; return 0; }
EOF
		if $CXX -c -Wall conftest.c 2>&1 | grep -i "deprecated conversion from string constant to" > /dev/null 2>&1; then
			if $CXX -c -Wall -Wno-write-strings conftest.c 2>&1 | grep -i "deprecated conversion from string constant to" > /dev/null 2>&1 ; then
				AC_MSG_RESULT(found -- insuppressible with -Wno-write-strings)
			else
				NoWriteStringsSetting="-Wno-write-strings"
				AC_MSG_RESULT(found -- will be suppressed)
			fi
		else
			AC_MSG_RESULT(none seen);
		fi
	else
		AC_MSG_RESULT(No compiler found!?!);
	fi
    AC_SUBST(NoWriteStringsSetting)

    ])
])

