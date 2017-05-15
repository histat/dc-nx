#!/bin/sh
#-------------------------------------------------------------------------
# edit dev_option
# $ wodim --devices

dev_option="/dev/sg4"

#=========================================================================
# Don't edit
local_cdrecord=`which wodim`
local_mkisofs=`which genisoimage`

if [ ! -e wav ]; then
    echo "WAV not exist" 1>&2
    exit 1
fi

if [ ! -e data ]; then
    echo "DATA not exist" 1>&2
    exit 1
fi

if [ ! -e data/1ST_READ.BIN ]; then
    echo "1ST_READ.BIN not exist" 1>&2
    exit 1
fi

if [ ! -e IP.BIN ]; then
    echo "IP.BIN not exist" 1>&2
    exit 1
fi

# burning audio
$local_cdrecord dev=$dev_option -multi -audio -pad wav/*.wav

# multi-session info
msinfo=`$local_cdrecord dev=$dev_option -msinfo`

#CD-ROM XA mode 2 form 1 - 2048 bytes
$local_mkisofs -G IP.BIN -C $msinfo -l data | $local_cdrecord dev=$dev_option -multi -xa -

