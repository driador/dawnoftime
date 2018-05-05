/**************************************************************************/
// com_time.c - shows the compile time of the current mud code
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"

/**************************************************************************/
// - Kal sept 97
char *get_compile_time(bool show_parent_codebase_version)
{
    static char return_buf[MSL];

	char mname[MSL];
	if(game_settings==NULL || IS_NULLSTR(MUD_NAME)){
		strcpy(mname,"The Dawn Of Time");
	}else{
		strcpy(mname, MUD_NAME);
	}

	#if defined(WIN32)
		sprintf( return_buf, "-= The Dawn of Time (Win32c++)"
			" was last compiled on %s at %s =-\r\n",  __DATE__, __TIME__ );

	#else

		#if defined(__APPLE__) && defined(__MACH__)
			// os-x
			sprintf( return_buf, "-= %s" 
				" was last compiled on %s at %s (os-x c++)=-\r\n",
								 mname, __DATE__, __TIME__ );
		#else
			sprintf( return_buf, "-= %s" 
				" was last compiled on %s at %s (c++ build)=-\r\n",
								 mname, __DATE__, __TIME__ );
		#endif
	#endif

	// concat the parent codebase version
	if(show_parent_codebase_version){
		strcat(return_buf, "-= Parent codebase version Dawn ");
		strcat(return_buf, DAWN_RELEASE_VERSION);
		strcat(return_buf, " - ");
		strcat(return_buf, DAWN_RELEASE_DATE);
		strcat(return_buf, ":\r\n");
	}
    return (return_buf);
}


/**************************************************************************/
// - Kal sept 97
void do_compile_time(char_data *ch, char *)
{
    ch->print(get_compile_time(false));
	ch->printlnf("This mud is based on Dawn %s - %s (http://www.dawnoftime.org/).", 
		DAWN_RELEASE_VERSION, DAWN_RELEASE_DATE);

#ifdef DAWN_STATIC_BETA_RELEASE
		// tell them about the code expiry
		{
			time_t expiry_date=DAWN_STATIC_BETA_RELEASE_EXPIRY_DATE;
			
			if(current_time<expiry_date){
				ch->titlebar("DAWN BETA RELEASE");
				ch->wraplnf("This version of the dawn code is a beta release which "
					"will expire on %s.  After this date only immortals will be "
					"able to connect", ctime(&expiry_date));
			}else{
				ch->titlebar("DAWN BETA RELEASE EXPIRED");
				ch->wraplnf("This version of the dawn code is a beta release which "
					"expired on %s!  Only immortals are able to connect.  Look at "
					"downloading a newer release from www.dawnoftime.org.", 
					ctime(&expiry_date));
			}
		}
		ch->titlebar("");
#endif
}
/**************************************************************************/
