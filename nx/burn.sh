#!/bin/sh
#-------------------------------------------------------------------------
# edit dev_option
# $ wodim --devices

#dev_option="/dev/scd1"
#dev_option="/dev/hda1"
dev_option="/dev/sg4"

#====================================================
# Don't edit

local_cdrecord=`which wodim`
local_mkisofs=`which genisoimage`

if [ ! -f raw/*.raw ];then
    echo "audio not exist" 1>&2
    exit 1
fi

if [ ! -f data ];then
    echo "data not exist" 1>&2
    exit 1
fi

if [ ! -f IP.BIN ];then
    echo "IP.BIN not exist" 1>&2
    exit 1
fi

# burning audio
$local_cdrecord dev=$dev_option -multi -audio audio/*.raw

# multi-session info
msinfo=`$local_cdrecord dev=$dev_option -msinfo`

#CD-ROM XA mode 2 form 1 - 2048 bytes
$local_mkisofs -G IP.BIN -C $msinfo -l data \
    | $local_cdrecord dev=$dev_option -multi -xa -

