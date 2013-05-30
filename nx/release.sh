#!/bin/sh

#-----------------------------------------------------------------
CMDNAME=`basename $0`
BASE=`pwd`
VER=$(date '+0.%y.%m.%d')
#-----------------------------------------------------------------
BINDIR="nxdc-$VER-plainfiles"
SOURCEDIR="nxdc-$VER-source"



make_bin() {

	if [ ! -d $BINDIR ];then
		mkdir -p disttmp/$BINDIR
	fi

	sh-elf-objcopy -S -R .stack -O binary nxdc NXDC.BIN
	scramble NXDC.BIN 1ST_READ.BIN

	sed -e 's/[@]DATE[@]/$(shell date '+%Y%m%d')/' < ip.txt.in > ip.txt 
	
	makeip ip.txt IP.BIN

	cp README disttmp/$BINDIR/
	cp IP.BIN disttmp/$BINDIR/
	cp 1ST_READ.BIN disttmp/$BINDIR/
	cd $BASE/disttmp/
	zip -r $BINDIR.zip $BINDIR
	cd $BASE
}


make_src() {

	if [ ! -d $SOURCEDIR ];then
		mkdir -p disttmp/$SOURCEDIR
	fi

	cd $BASE

	cp LICENSE disttmp/$SOURCEDIR/
	cp *.cpp *.h *fdh disttmp/$SOURCEDIR/
	cp smalfont.bmp sprites.sif tilekey.dat dreamcast.ml disttmp/$SOURCEDIR/
	cp -r TextBox disttmp/$SOURCEDIR/
	cp -r ai disttmp/$SOURCEDIR/
	cp -r autogen disttmp/$SOURCEDIR/
	cp -r common disttmp/$SOURCEDIR/
	cp -r endgame disttmp/$SOURCEDIR/
	cp -r graphics disttmp/$SOURCEDIR/
	cp -r intro disttmp/$SOURCEDIR/
	cp -r pause disttmp/$SOURCEDIR/
	cp -r siflib disttmp/$SOURCEDIR/
	cp -r sound disttmp/$SOURCEDIR/
	
	cd $BASE/disttmp/
	tar jcvf $SOURCEDIR.tar.bz2 $SOURCEDIR
	cd $BASE
}


case $1 in
    all)
    make_src
    make_bin
    ;;
    src)
    make_src
    ;;
    bin)
    make_bin
    ;;
    *)
    echo "USAGE: $CMDNAME (all|src|bin)" 1>&2
    ;;
esac
