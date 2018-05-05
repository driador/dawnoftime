/**************************************************************************/
// ispell.cpp - intergrated spell checker 
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
#include "include.h"

#ifdef unix
/**************************************************************************/
#include <sys/wait.h>

static FILE *ispell_out;
static int ispell_pid = -1;
static int to[2], from[2];

#define ISPELL_BUF_SIZE 1024

/**************************************************************************/
void ispell_init()
{
    char ignore_buf[1024];

    int ret;
	
	ret=pipe(to);
	if(ret<0){
		logf("ispell_init() pipe(to) call returned %d (error), errno=%d (%s)", ret, errno, strerror(errno));
		return;
	}
	ret=pipe(from);
	if(ret<0){
		logf("ispell_init() pipe(from) call returned %d (error), errno=%d (%s)", ret, errno, strerror(errno));
		return;
	}

    ispell_pid = fork();
    if (ispell_pid < 0){
        bugf("ispell_init: fork error: %d (%s)", errno, strerror(errno));
		return;
    }

    if (ispell_pid == 0)   // child
    {
        int i;
        dup2(to[0], 0);         // this is where we read commands from - make
                                // it stdin 
        close(to[0]);
        close(to[1]);

        dup2(from[1], 1);       // this is where we write stuff to 
        close(from[0]);
        close(from[1]);

        // Close all the other files
        for (i = 2; i < 255; i++)
            close(i);

		logf("starting ispell....");
        execlp("ispell", "ispell", "-a", (char *)
               NULL);
		log_string("ispell_init(): error starting ispell.");
		log_string("ispell startup error");
        exit(1);
    }
    else
    {                           // ok ! 
        close(to[0]);
        close(from[1]);
        ispell_out = fdopen(to[1], "w");
        setbuf(ispell_out, NULL);
        ret=read(from[0], ignore_buf, 1024);
		if(ret<0){
			int status;
			logf("ispell_init() read() call returned %d (error), errno=%d (%s)", ret, errno, strerror(errno));			
			log_string("Ispell not started, getting child process result to avoid zombie.");
			wait(&status);
			logf("Status result = %d", status);	
		}

		if(strstr(ignore_buf, "ispell startup error")){
			int status;
			log_string("Ispell not started, getting child process result to avoid zombie.");
			wait(&status);
			logf("Status result = %d", status);
		}
    }
}

/**************************************************************************/
void ispell_done()
{
    if (ispell_pid != -1)
    {
        fprintf(ispell_out, "#\n");
        fclose(ispell_out);
        close(from[0]);
        waitpid(ispell_pid, NULL, 0);
        ispell_pid = -1;
    }
}

/**************************************************************************/
char *get_ispell_line(char *word)
{
    static char buf[ISPELL_BUF_SIZE];
    char buf2[MSL];
    int len;

    if (ispell_pid == -1)
        return NULL;

    if (word)
    {
        fprintf(ispell_out, "^%s\n", word);
        fflush(ispell_out);
    }

    len = read(from[0], buf2, ISPELL_BUF_SIZE);
	if(len<0){
		sprintf(buf,"Read from ispell returned length %d (error), errno=%d (%s)", len, errno, strerror(errno));
		return buf;
	}
    buf2[len] = '\0';

    // Read up to max 1024 characters here 
    if (sscanf(buf2, "%" Stringify(ISPELL_BUF_SIZE) "[^\n]\n\n", buf) != 1)
        return NULL;

    return buf;
}

/**************************************************************************/
void do_ispell(char_data * ch, char *argument)
{
    char *pc;

    if (ispell_pid <= 0)
    {
        ch->println("ispell is not running.");
        return;
    }

    if (!argument[0] || strchr(argument, ' '))
    {
        ch->println("Invalid input.");
        return;
    }

    if (argument[0] == '+')
    {
        for (pc = argument + 1; *pc; pc++)
            if (!is_alpha(*pc))
            {
                ch->printlnf("'%c' is not a letter.", *pc);
                return;
            }
        fprintf(ispell_out, "*%s\n", argument + 1);
        fflush(ispell_out);
        return;                 // we assume everything is OK.. better be so!
    }

    pc = get_ispell_line(argument);
    if (!pc)
    {
        ch->println("ispell: failed to check the word.");
        return;
    }

    switch (pc[0])
    {
        case '*':
        case '+':               /* root */
        case '-':               /* compound */
            ch->println("Correct.");
            break;

        case '&':               /* miss */
            ch->printlnf("Not found. Possible words: %s", strchr(pc, ':') + 1);
            break;

        case '?':               /* guess */
			ch->printlnf("Not found. Possible words: %s", strchr(pc, ':') + 1);
            break;

        case '#':               /* none */
            ch->println("Unable to find anything that matches.");
            break;

        default:
            ch->printlnf("Weird output from ispell: %s", pc);
    }
}

/**************************************************************************/
/*
* Function to add ispell support within an editor
* It takes the string the char is currently editing as it's source,
* and passes it a word at a time to ispell.  A check is kept on
* what words have been checked to prevent repetition of the same
* 'error', mainly useful with proper names etc.
*/
void ispell_string(char_data * ch)
{
    char *result;
    char word[MSL];
    char checked_list[MSL];
    char buf[MSL];
    char *str;
    int i = 0;
    bool errors = false;
    BUFFER *buffer;

    buffer = new_buf();
    strcpy(checked_list, "");

    str = *ch->desc->pString;

	// strip the colour from the string before processing
	// this function uses temp_HSL_workspace to do its thing
	if(!IS_NULLSTR(str)){
		convertColour(str, temp_HSL_workspace, CT_NOCOLOUR, false);
		str=temp_HSL_workspace;
	}

    while (*str != '\0')
    {
        while (*str != ' ' && *str != '\0' && *str != '\n' && *str != '\r')
        {
            word[i++] = *str;

            str++;
        }

        word[i++] = '\0';

        if (!is_exact_name(word, checked_list))
        {
            sprintf(checked_list + str_len(checked_list), "%s ", word);

            result = get_ispell_line(word);

            if (!IS_NULLSTR(result))
            {
                if (result[0] == '&')
                {
                    sprintf(buf, "%s failed - Possible words : %s\r\n",
                            word, strchr(result, ':'));
                    errors = true;
                    add_buf(buffer, buf);
                }

                if (result[0] == '#')
                {
                    sprintf(buf, "%s failed - no similar words found.\r\n", word);
                    errors = true;
                    add_buf(buffer, buf);
                }
            }
        }
        str++;
        i = 0;
    }
    ch->println("");
    if (errors){
        ch->sendpage(buf_string(buffer));
    }else{
        ch->println("No errors found.");
	}

    free_buf(buffer);

    return;
}
/**************************************************************************/
#else
/**************************************************************************/
#include "include.h"
void do_ispell(char_data * ch, char *)
{    
    ch->println("Sorry ispell is available in unix only.");
    return;
}
/**************************************************************************/
#endif
