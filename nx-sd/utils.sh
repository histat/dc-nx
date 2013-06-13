#!/bin/sh


CMDNAME=`basename $0`
BASE=`pwd`

do_strip() {
	sh-elf-strip -o nxdc.strip.elf nxdc
}

do_upload() {
	ipupload.pike < nxdc.strip.elf
}


case $1 in
    all)
		do_strip
		do_upload
		;;
	dump)
		sh-elf-objdump -D nxdc > dump
		;;
    strip)
		do_strip
		;;
    upload)
		do_upload
		;;
    *)
    echo "USAGE: $CMDNAME (dump/upload|strip/all)" 1>&2
    ;;
esac

