#!/usr/bin/perl
#########################################################################
# bootdawn.pl                       Version 1.1                         #
# Created: Jun 28, 1997             Kalahn @ dawnoftime                 #
# Last Modified: 18 March 2001      http://www.dawnoftime.org/          #
#########################################################################
# This is a pretty simple script that checks to see if a mud is running #
# properly.  Be sure to change the 'important vars' section.  The best  #
# idea is to run this script from cron every couple of minutes... That  #
# way you can be sure that it stays running.
#
# You probably should stop using startup as a script and delete it.
# Here's how to do it...
#
# 1) Make sure to edit the bootdawn.pl file and insert your values at
#    the top.
#
# 2) Edit your crontab file, type crontab -e, it's a vi editor.
#
# 3) Put the following line as your crontab script:
#
#    */6 * * * * /home/dawn/bin/bootdawn.pl
#
# 4) Make sure to change the directory in the above line as needed.
#
#########################################################################
# CHANGES AND HISTORY                                                   #
#                                                                       #
# v 1.0, Jun 28, 1997   Written by Jared Proudfoot                      #
#                           <jproudfo@footprints.net>                   #
# v 1.1, Mar 18, 2001   Customised for Dawn of time - Kalahn            #
#########################################################################
#########################################################################
# Define the important vars                                             #
#                                                                       #
# Define the host and port number where the mud resides.                #

$server     =   "127.0.0.1";
$port       =   "4000";

# $string is the string of characters we will look for upon connecting. #
# If we connect, but don't see this string, we will assume the mud      #
# isn't responding (locked up?) and we'll restart it.  The string       #
# *must* be on the first line after connect.

# You may enter this as a regexp if you wish.                           #

$replyString        =   ".+";

# How long do we wait before we connect timeout (in seconds)?           #

$timeOut    =   "60";

# What to execute if we need to restart the mud.  Please include the    #
# FULL path.                                                            #

$exec       =   "ulimit -c unlimited ; /home/dawn/dot/dawn";

# Path where you want the mud logs to be kept.

$logdir     =   "/home/dawn/dot/logs/game";

# Path where we should start the mud from.                              #
$startPath  =   "/home/dawn/dot";

# That's it.  You shouldn't need to change anything after this line.    #
#########################################################################

# What do we need to use?
use Socket;
require 5.003;

#########################################################################
# Main                                                                  #
#########################################################################

if (&connect_server == 0) {
    # If we couldn't connect, try and restart.  #
    print ("Connection to $server on port $port failed or timed out after $timeOut seconds!\n");
    $time = (scalar localtime);
    print ("Attempting to restart the mud on $time...\n");
    # Restart the mud                           #
    &restart_mud;
}
else {
    # We connected, but is it working properly?         #
        $readline = (&gl);
        if ($readline =~ /$replyString/) {
            # We found what we were looking for, so exit    #
            # properly.                 #
            &disconnect_server;
            exit 1;
        }
    # After all those searches, we didn't find anything.  The mud   #
    # must be locked up.  Lets kill and restart it.         #
    &disconnect_server;
    print ("The connection was sucessful, but it doesn't seem to be responding\n");
    $time = (scalar localtime);
    print ("Attempting to restart the mud on $time...\n");
    system("killall $exec");
    &restart_mud;
}


#########################################################################
# Subroutines                               #
#########################################################################
sub connect_server {
    # Connect to the server                     #
    my ($iaddr, $paddr, $proto);
    $iaddr = inet_aton ($server)
        or die ("ERROR: No host: $server!\n");
    $paddr = sockaddr_in ($port, $iaddr);
    $proto = getprotobyname('tcp');
    socket (SOCK, PF_INET, SOCK_STREAM, $proto)
        or die ("ERROR: Socket error $!\n");
    alarm ($timeOut);
    if (connect (SOCK, $paddr)) {;
        alarm (0);
        return 1;
    }
    else {
        return 0;
    }
}

sub disconnect_server {
    # Disconnect from the server                    #
    close (SOCK);
    return;
}

sub sl {
    # Send a line                           #
    my ($line)=@_;
    print SOCK ("$line")
        or die ("ERROR: Error writing to server: $!\n");
    select SOCK;
    $|=1;
    select STDOUT;
    $|=1;
    return;
}

sub gl {
    # Get a line                            #
    my ($buffer, @reply);
    $buffer=(<SOCK>);
#   (@reply) = split (/\s/, $buffer);
#   return (@reply);
    return ($buffer);
}

sub restart_mud {
        # Restart the mud                   #
        $timet = time();
        chdir $startPath;
        system ("$exec $port > $logdir/$timet.log 2>&1 &");
        return;
}
#########################################################################


