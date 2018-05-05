/**************************************************************************/
// File: cvsserve.cpp 
/**************************************************************************/
// - This can be used to setup a cvs repository without having root access
// - Molded into shape from an example inetd server by Kalahn 
// compile with: g++ -o cvsserve cvsserve.cpp

#define SERV_TCP_PORT   2401 // default port
#define CVS_PATH "/usr/bin/"
#define CVS_ROOT "/dawn/cvsroot"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

char *portnum;

#define IS_NULLSTR(str)   ((str)==NULL || (str)[0]=='\0') 
/**************************************************************************/
bool is_number( char *arg )
{
 
    if ( *arg == '\0' )
        return false;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;

    for ( ; *arg != '\0'; arg++ )
	 {
        if ( !isdigit( *arg ) )
            return false;
    }
 
    return true;
}
/**************************************************************************/
main(int argc, char *argv[], char **envp)
{
    int sockfd, newsockfd, childpid;
	unsigned int clilen;
    struct sockaddr_in cli_addr, serv_addr;
	int port;
	
	port=SERV_TCP_PORT;
    portnum = argv[1];
    
    if ((sockfd = socket (AF_INET,SOCK_STREAM,0)) <0)
    {
		fprintf(stderr, "error: can't open stream socket\r\n");
		exit(0);
    }

	if(!IS_NULLSTR(portnum)){
		if(is_number(portnum)){
			port=atoi(portnum);
		}else{
			fprintf(stderr, "syntax %s [portnumber]\r\n", argv[0]);
			exit(0);
		}
	}
	
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
	
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
		fprintf(stderr, "error: can't bind local address to port %d!\r\n", port);
		exit(0);
    }

    
    listen (sockfd, 5);
	signal(SIGCLD, SIG_IGN);	
	
    for ( ; ; ) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr * )  &cli_addr,&clilen);
		if (newsockfd < 0)
		{
			fprintf(stderr, "error: accept error\r\n");
			exit(0);
		}
		if ((childpid = fork()) < 0)
		{
			close(sockfd);
			fprintf(stderr, "error: fork error\r\n");
			exit(0);
		}
		else if (childpid == 0) {
			signal(SIGCLD, SIG_DFL);
			close (sockfd);
			close(0);
			close(1);
			close(2);
			dup(newsockfd);
			dup(newsockfd);
			dup(newsockfd);
			(void) execlp (CVS_PATH "cvs","cvs","--allow-root=" CVS_ROOT,"pserver",NULL);
			close (newsockfd);
			exit(0);
		}
		close (newsockfd);
    }
}
/**************************************************************************/
// compile with: g++ -o cvsserve cvsserve.cpp
// change the #defines at the top to customise default port etc
/**************************************************************************/
