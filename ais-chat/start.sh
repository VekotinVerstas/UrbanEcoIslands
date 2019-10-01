#!/bin/bash
# (re)starts the command defined in $COMMAND if not running
# add periodical start to cron (crontab -e), e.g.:
# 5,25,45 * * * * cd /home-directory-of-ais-chat; ./start.sh>/dev/null 2>&1
#
# info:
# https://github.com/VekotinVerstas/UrbanEcoIslands/tree/master/ais-chat

COMMAND="/usr/bin/python3 -u ais2chat.py"
if /usr/bin/pgrep -f "${COMMAND}" > /dev/null 
then
    echo "${COMMAND} is already running."
else
    echo "${COMMAND} is NOT running! Starting now..."
    $COMMAND
fi
sleep 2
