#!/bin/sh

#-----------------------------------------------------------------
CMDNAME=`basename $0`
BASE=`pwd`
VER=$(date '+0.%y.%m.%d')
DATE=$(date '+%Y%m%d')
#-----------------------------------------------------------------
BINDIR="nxdc-$VER-sd-plainfiles"
SOURCEDIR="nxdc-$VER-sd-source"



make_bin() {

	if [ ! -d $BINDIR ];then
		mkdir -p disttmp/$BINDIR
	fi

	sh-elf-objcopy -S -R .stack -O binary nxdc NXDC.BIN
	scramble NXDC.BIN 1ST_READ.BIN

	sed -e "s/[@]DATE[@]/$DATE/" < ip.txt.in > ip.txt 
	
	makeip ip.txt IP.BIN

	cp LICENSE disttmp/$BINDIR/
	cp README disttmp/$BINDIR/
	cp IP.BIN disttmp/$BINDIR/
	cp 1ST_READ.BIN disttmp/$BINDIR/
	cp NXDC.BIN disttmp/$BINDIR/
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
	cp *.cpp *.h disttmp/$SOURCEDIR/
	cp smalfont.bmp sprites.sif tilekey.dat dreamcast.ml disttmp/$SOURCEDIR/
	mkdir -p disttmp/$SOURCEDIR/TextBox
	mkdir -p disttmp/$SOURCEDIR/ai
	mkdir -p disttmp/$SOURCEDIR/autogen
	mkdir -p disttmp/$SOURCEDIR/common
	mkdir -p disttmp/$SOURCEDIR/endgame
	mkdir -p disttmp/$SOURCEDIR/graphics
	mkdir -p disttmp/$SOURCEDIR/intro
	mkdir -p disttmp/$SOURCEDIR/pause
	mkdir -p disttmp/$SOURCEDIR/siflib
	mkdir -p disttmp/$SOURCEDIR/sound
	
	cp TextBox/*.cpp TextBox/*.h disttmp/$SOURCEDIR/TextBox
	cp ai/*.cpp ai/*.h disttmp/$SOURCEDIR/ai
	cp autogen/*.cpp autogen/*.h disttmp/$SOURCEDIR/autogen
	cp common/*.cpp common/*.h disttmp/$SOURCEDIR/common
	cp endgame/*.cpp endgame/*.h disttmp/$SOURCEDIR/endgame
	cp graphics/*.cpp graphics/*.h disttmp/$SOURCEDIR/graphics
	cp intro/*.cpp intro/*.h disttmp/$SOURCEDIR/intro
	cp pause/*.cpp pause/*.h disttmp/$SOURCEDIR/pause
	cp siflib/*.cpp siflib/*.h disttmp/$SOURCEDIR/siflib
	cp sound/*.cpp sound/*.h disttmp/$SOURCEDIR/sound
		
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
