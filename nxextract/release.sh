#!/bin/sh

#-----------------------------------------------------------------
CMDNAME=`basename $0`
BASE=`pwd`
VER=$(date '+0.%y.%m.%d')
#-----------------------------------------------------------------
BINDIR="nxextract"
SOURCEDIR="nxextract-source"



make_bin() {

	if [ ! -d $BINDIR ];then
		mkdir -p disttmp/$BINDIR
	fi

	i586-mingw32msvc-strip -s nxextract -o nxextract.exe

	cp LICENSE disttmp/$BINDIR/
	cp smalfont.bmp sprites.sif tilekey.dat disttmp/$BINDIR/
	cp nxextract.exe SDL.dll disttmp/$BINDIR/
	cd $BASE/disttmp/
	zip -r $BINDIR.zip $BINDIR
	cd $BASE
}

make_src() {

	if [ ! -d $SOURCEDIR ];then
		mkdir -p disttmp/$SOURCEDIR
	fi

	make clean

	cd $BASE

	cp LICENSE disttmp/$SOURCEDIR/
	cp *.cpp *.h *fdh disttmp/$SOURCEDIR/
	cp smalfont.bmp sprites.sif tilekey.dat i586-mingw32msvc.ml disttmp/$SOURCEDIR/
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
