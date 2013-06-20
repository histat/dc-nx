#!/bin/sh

#-----------------------------------------------------------------
CMDNAME=`basename $0`
BASE=`pwd`
VER=$(date '+0.%y.%m.%d')
#-----------------------------------------------------------------
SOURCEDIR="sdlshim-$VER-dc-source"

make_src() {

	if [ ! -d $SOURCEDIR ];then
		mkdir -p disttmp/$SOURCEDIR
	fi

	if [ ! -d $SOURCEDIR/sdlshim ];then
		mkdir -p disttmp/$SOURCEDIR/sdlshim
	fi
	
	cp *.cpp *.h *.s *.c *fdh dreamcast.ml disttmp/$SOURCEDIR/sdlshim/
	cp -r ../lib disttmp/$SOURCEDIR/
	cp -r SDL disttmp/$SOURCEDIR/sdlshim/
	
	cd $BASE/disttmp/
	tar jcvf $SOURCEDIR.tar.bz2 $SOURCEDIR
	cd $BASE
}


case $1 in
    all)
    make_src
    ;;
    src)
    make_src
    ;;
    *)
    echo "USAGE: $CMDNAME (all|src)" 1>&2
    ;;
esac
