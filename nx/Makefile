ronindir = ../libronin

RELEASE_BUILD = 1

AS = sh-elf-as -little
CC = sh-elf-gcc -ml -m4-single-only
CXX = sh-elf-g++ -ml -m4-single-only
LD = sh-elf-ld -EL
RM = rm -f

DEFINES = -DUSE_ARM
ifeq "$(RELEASE_BUILD)" "1"
DEFINES += -DNOSERIAL 
else
DEFINES += -D__SDCARD__
endif
INCLUDES = -I$(ronindir)/include -I../sdlshim -I../sdlshim/SDL -I../libsd
CFLAGS = -Wall -Os $(INCLUDES) $(DEFINES) -Wreturn-type -Wunused-variable -Wno-multichar
CXXFLAGS = $(CFLAGS) -fno-exceptions
LDFLAGS = -Wl,-Ttext,0x8c010000 -nostartfiles $(ronindir)/lib/crt0.o

ifeq "$(RELEASE_BUILD)" "1"
LIBS	=-lstdc++ -L$(ronindir)/lib -lronin-noserial -lz -lm
else
LIBS	=-lstdc++ -L$(ronindir)/lib -lronin -lz -lm -L../libsd -lsd
endif


ASSRC := ../sdlshim/asm.s
CSRC :=
CPPSRC := \
	main.cpp game.cpp object.cpp ObjManager.cpp map.cpp \
	TextBox/TextBox.cpp TextBox/YesNoPrompt.cpp TextBox/ItemImage.cpp \
	TextBox/StageSelect.cpp TextBox/SaveSelect.cpp profile.cpp \
	settings.cpp platform.cpp \
	caret.cpp slope.cpp player.cpp playerstats.cpp p_arms.cpp \
	statusbar.cpp tsc.cpp screeneffect.cpp floattext.cpp input.cpp \
	replay.cpp trig.cpp inventory.cpp map_system.cpp debug.cpp \
	console.cpp niku.cpp \
	ai/ai.cpp ai/first_cave/first_cave.cpp  ai/village/village.cpp \
	ai/village/balrog_boss_running.cpp ai/village/ma_pignon.cpp \
	ai/egg/egg.cpp ai/egg/igor.cpp ai/egg/egg2.cpp ai/weed/weed.cpp \
	ai/weed/balrog_boss_flying.cpp ai/weed/frenzied_mimiga.cpp \
	ai/sand/sand.cpp ai/sand/puppy.cpp ai/sand/curly_boss.cpp \
	ai/sand/toroko_frenzied.cpp ai/maze/maze.cpp \
	ai/maze/critter_purple.cpp ai/maze/gaudi.cpp ai/maze/pooh_black.cpp \
	ai/maze/balrog_boss_missiles.cpp ai/maze/labyrinth_m.cpp \
	ai/almond/almond.cpp ai/oside/oside.cpp ai/plantation/plantation.cpp \
	ai/last_cave/last_cave.cpp ai/final_battle/balcony.cpp \
	ai/final_battle/misery.cpp ai/final_battle/final_misc.cpp \
	ai/final_battle/doctor.cpp ai/final_battle/doctor_frenzied.cpp \
	ai/final_battle/doctor_common.cpp ai/final_battle/sidekicks.cpp \
	ai/hell/hell.cpp ai/hell/ballos_priest.cpp ai/hell/ballos_misc.cpp \
	ai/npc/balrog.cpp ai/npc/curly.cpp ai/npc/curly_ai.cpp \
	ai/npc/misery.cpp ai/npc/npcregu.cpp ai/npc/npcguest.cpp \
	ai/npc/npcplayer.cpp ai/weapons/weapons.cpp ai/weapons/polar_mgun.cpp \
	ai/weapons/missile.cpp ai/weapons/fireball.cpp ai/weapons/blade.cpp \
	ai/weapons/snake.cpp ai/weapons/nemesis.cpp ai/weapons/bubbler.cpp \
	ai/weapons/spur.cpp ai/weapons/whimstar.cpp ai/sym/sym.cpp \
	ai/sym/smoke.cpp ai/balrog_common.cpp ai/IrregularBBox.cpp \
	stageboss.cpp ai/boss/omega.cpp ai/boss/balfrog.cpp \
	ai/boss/x.cpp ai/boss/core.cpp ai/boss/ironhead.cpp \
	ai/boss/sisters.cpp ai/boss/undead_core.cpp ai/boss/heavypress.cpp \
	ai/boss/ballos.cpp endgame/island.cpp endgame/misc.cpp \
	endgame/credits.cpp endgame/CredReader.cpp \
	intro/intro.cpp intro/title.cpp pause/pause.cpp pause/options.cpp \
	pause/dialog.cpp pause/message.cpp pause/objects.cpp \
	graphics/nxsurface.cpp graphics/graphics.cpp graphics/sprites.cpp \
	graphics/tileset.cpp graphics/font.cpp graphics/safemode.cpp \
	graphics/palette.cpp \
	sound/sound.cpp ../sdlshim/sound/sslib.cpp ../sdlshim/sound/org.cpp \
	../sdlshim/sound/pxt.cpp ../sdlshim/sound/cdda.cpp \
	siflib/sif.cpp siflib/sifloader.cpp siflib/sectSprites.cpp \
	siflib/sectStringArray.cpp \
	autogen/AssignSprites.cpp autogen/objnames.cpp stagedata.cpp \
	common/FileBuffer.cpp common/InitList.cpp common/BList.cpp \
	common/StringList.cpp common/DBuffer.cpp common/DString.cpp \
	common/bufio.cpp common/misc.cpp \
	../sdlshim/ronin.cpp ../sdlshim/dcevent.cpp \
	../sdlshim/console.cpp ../sdlshim/vm_file.cpp \
	../sdlshim/file.cpp ../sdlshim/vmu.cpp ../sdlshim/scrnsave.cpp \
	../sdlshim/SDL/init.cpp ../sdlshim/SDL/screen.cpp \
	../sdlshim/SDL/bmploader.cpp ../sdlshim/SDL/event.cpp \
	../sdlshim/SDL/audio.cpp ../sdlshim/SDL/misc.cpp



ASOBJ     = $(patsubst %.s, %.o, $(ASSRC))
COBJ      = $(patsubst %.c, %.o, $(CSRC)) 
CPPOBJ    = $(patsubst %.cpp, %.o, $(CPPSRC))

OBJS = $(CPPOBJ) $(ASOBJ) $(COBJ)

all : nxdc.elf

nxdc.elf: $(OBJS) 
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) $(LIBS) 

clean:
	$(RM) $(OBJS) nxdc.elf

nxdc.strip.elf: nxdc.elf
	sh-elf-strip -o $@ $<

run: nxdc.strip.elf
	ipupload.pike < $<

dump: nxdc.elf
	sh-elf-objdump -D $< > dump

NXDC.BIN: nxdc.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@

1ST_READ.BIN: NXDC.BIN
	scramble $< $@

dist: 1ST_READ.BIN

distclean:
	rm -f 1ST_READ.BIN NXDC.BIN IP.BIN dump nxdc.strip.elf nxdc.elf

NXDCVER=1.0.0.1

ip.txt: ip.txt.in
	ver=V$(NXDCVER); \
	sed -e 's/[@]VERSION[@]/'"$$ver"/ -e 's/[@]DATE[@]/$(shell date '+%Y%m%d')/' < $< > $@ 

IP.BIN: ip.txt
	makeip ip.txt IP.BIN
