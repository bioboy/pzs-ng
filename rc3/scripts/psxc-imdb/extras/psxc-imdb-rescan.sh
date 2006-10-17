#!/bin/bash

# This script will do a rescan of the imdb info in current dir (and dirs
# directly below the current dir). The script should be set up as a site
# command.

# 1. edit the config below
# 2. copy this file to /glftpd/bin
# 3. add the following in glftpd.conf
#       site_cmd IMDB-RESCAN EXEC /bin/psxc-imdb-rescan.sh
#       custom-imdb-rescan   1 =staff
#
# syntax is:
# site imdb-rescan [-r|-1] (-nobot) (-nofile) (-noaddon)

# config
########

# Version. No need to change
VERSION=2.3

# Path to psxc-imdb.sh. This is relative to GLROOT.
PSXC_IMDB=/bin/psxc-imdb.sh

# Nukedir style. We don't want to rescan nuked dirs, so put here
# the output of this line:
#
# cat /etc/glftpd.conf | grep -e "^nukedir_style" | awk '{print $2}' | tr '%' '\n' | head -n 1
#
NUKEDIR_STYLE="[NUKED]-"

# end of config
###############

if [ $# -eq 0 ]; then
 echo "PSXC's imdb rescanner v$VERSION."
 echo "----------------------------"
 echo "Syntax:"
 echo "site imdb-rescan [-1|-r] (-nobot) (-nofile) (-noaddon)"
 echo "  -1 = scan current dir"
 echo "  -r = scan all dirs below current"
 echo ""
 echo "  -nobot    = no bot output in channel."
 echo "  -nofile   = no file changes in releasedir."
 echo "  -noaddon  = do not run any addons."
 echo ""
 echo "Use caution - chanflooding may occur."
 echo "-1 is not needed if one or more of the other flags are specified."
 echo "You can combine any of the three -no flags if you wish."
 echo ""
 exit 0
fi
ARG=$1

if [ ! -z "`echo $@ | grep -e "[-][nN][oO][bBfFaA][oOiIdD][tTlLdD]"`" ]; then
 PSXCFLAGS=$@
 PSXCFLAG=0
 PSXC_CONF="`cat "$PSXC_IMDB" | grep -e "^CONFFILE" | head -n 1 | cut -d '=' -f 2`"
 . $PSXC_CONF
 if [ ! -z $GLROOT ]; then
  MYTMPFILE="`echo "$TMPFILE" | sed "s%$GLROOT%%"`"
 else
  MYTMPFILE=$TMPFILE
 fi
 if [ -z "`cat $MYTMPFILE`" ]; then
  echo "checking to see if psxc-imdb is running..."
  sleep 1
  if [ ! -z "`echo "$@" | grep -e "[nN][oO][bB][oO][tT]"`" ]; then
   let PSXCFLAG=PSXCFLAG+1
  fi
  if [ ! -z "`echo "$@" | grep -e "[nN][oO][fF][iI][lL][eE]"`" ]; then
   let PSXCFLAG=PSXCFLAG+2
  fi
  if [ ! -z "`echo "$@" | grep -e "[nN][oO][aA][dD][dD][oO][nN]"`" ]; then
   let PSXCFLAG=PSXCFLAG+4
  fi
  if [ -z "`cat $MYTMPFILE`" ]; then
   CLEARTORUN=0
  else
   CLEARTORUN=1
  fi
 else
  CLEARTORUN=0
 fi
 if [ ! -z "`cat $MYTMPFILE`" ]; then
  echo "I'm sorry. It looks to me that psxc-imdb is already running."
  echo "I cannot perform a rescan while this is the case."
  echo "Please try again a bit later."
  exit 2
 fi
 echo $PSXCFLAG >$MYTMPFILE
fi

if [ ! "$ARG" = "-r" ]; then
 echo "Doing a scan in current dir .. searching for iMDB info."
 IMDB_NFO="`ls -1 | grep -e "[.][Nn][Ff][Oo]" | head -n 1`"
 if [ ! -z "$IMDB_NFO" ]; then
  $PSXC_IMDB $IMDB_NFO
 fi
 echo "Done."
 exit 0
else
 echo "Scanning for iMDB info.."
 for REL_DIR in `ls -1F | grep -e "/" | grep -F -v "^$NUKEDIR_STYLE" | tr ' ' '%'`; do
  REL_DIR="`echo $REL_DIR | tr '%' ' '`"
  echo "$REL_DIR ..."
  cd "$REL_DIR"
  IMDB_NFO="`ls -1 | grep -e "[.][Nn][Ff][Oo]" | head -n 1`"
  if [ ! -z "$IMDB_NFO" ]; then
   $PSXC_IMDB $IMDB_NFO
  fi
  cd ..
 done
 echo "Done."
 exit 0
fi
