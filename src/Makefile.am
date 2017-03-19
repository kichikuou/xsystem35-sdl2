#
# Makefile.am for xsystem35
#

#programe name
bin_PROGRAMS = xsystem35

AM_CPPFLAGS = $(LTDLINCL)

xsystem35_LDFLAGS = @EXTRALDFLAGS@

AM_CFLAGS = @DEBUG_CFLAGS@ @X_CFLAGS@ @GTK_CFLAGS@ @SDL_CFLAGS@ @SDL2MIXER_CFLAGS@ @EXTRACFLAGS@ -DMODULE_PATH=\"$(libdir)/@PACKAGE@\" -DLOCALEDIR=\""$(datadir)/locale"\"


SRC_SYS35CMD = cmdb.c cmdc.c cmdd.c cmde.c cmdf.c cmdg.c cmdh.c cmdi.c cmdj.c cmdk.c cmdl.c cmdm.c cmdn.c cmdo.c cmdp.c cmdq.c cmds.c cmdt.c cmdu.c cmdv.c cmdw.c cmdy.c cmdz.c cmd2F.c cmd2F60.c
SRC_CG    = pms.c vsp.c bmp.c qnt.c
SRC_MUS   = music.c music_cdrom.c music_midi.c music_pcm.c music_fader.c midi.c cdrom.c
SRC_MISC  = LittleEndian.c counter.c imput.c profile.c mt19937-1.c @JOY_S@.c utfsjis.c filecheck.c tempname.c list.c
SRC_SCO   = cali.c scenario.c variable.c cmd_check.c hankana2sjis.c nact.c selection.c message.c savedata.c hankaku.c s39ain.c
SRC_GRAPH = ags.c font.c flood.c cg.c ecopy.c antialiase.c alpha_plane.c cursor.c

SRC_NET = network.c
SRC_DLL = 

xsystem35_SOURCES = \
	xsystem35.c \
	dri.c \
	ald_manager.c \
	cache.c \
	$(SRC_SYS35CMD) \
	$(SRC_CG) \
	$(SRC_SCO) \
	$(SRC_MUS) \
	$(SRC_GRAPH) \
	$(SRC_MISC) \
	$(SRC_NET) \
	$(SRC_DLL)

xsystem35_DEPENDENCIES = \
	$(SRC_AUDIO:.c=.o) \
	$(SRC_CDROM:.c=.o) \
	$(SRC_MIDI:.c=.o) \
	$(SRC_GRAPHDEV:.c=.o) \
	$(SRC_FONT:.c=.o) \
	$(SRC_ABLEND:.s=.o) \
	$(SRC_MENU:.c=.o)

xsystem35_LDADD = \
	@ZLIB_LIBS@ \
	@X_LIBS@ \
	@GTK_LIBS@ \
	@SDL_LIBS@ \
	@SDL2MIXER_LIBS@ \
	@EXTRALIBS@ \
	@LIBINTL@ \
	$(LIBLTDL) \
	$(SRC_ABLEND:.s=.o) \
	$(SRC_AUDIO:.c=.o) \
	$(SRC_CDROM:.c=.o) \
	$(SRC_MIDI:.c=.o) \
	$(SRC_GRAPHDEV:.c=.o) \
	$(SRC_FONT:.c=.o) \
	$(SRC_MENU:.c=.o)

image.o: image.c image15.c image16.c image24.c image24p.c


sdl_input.c:
	perl xsyskey.pl > sdl_input.c

.s.o:
	nasm -f elf $<

objs: $(system35_OBJECTS) $(xsystem35_DEPENDENCIES)
        @:

