#
# Makefile.am for xsystem35
#

#programe name
bin_PROGRAMS = xsystem35

INCLUDES = $(LTDLINCL)

xsystem35_LDFLAGS = @EXTRALDFLAGS@

AM_CFLAGS = @DEBUG_CFLAGS@ @X_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@ @ESD_CFLAGS@ @SDL_CFLAGS@ @EXTRACFLAGS@ @VORBIS_CFLAGS@ -DMODULE_PATH=\"$(libdir)/@PACKAGE@\" -DLOCALEDIR=\""$(datadir)/locale"\"


SRC_SYS35CMD := cmdb.c cmdc.c cmdd.c cmde.c cmdf.c cmdg.c cmdh.c cmdi.c cmdj.c cmdk.c cmdl.c cmdm.c cmdn.c cmdo.c cmdp.c cmdq.c cmds.c cmdt.c cmdu.c cmdv.c cmdw.c cmdy.c cmdz.c cmd2F.c cmd2F60.c
SRC_CG    := pms.c vsp.c bmp.c qnt.c
SRC_MUS   := music.c music_client.c music_server.c music_cdrom.c music_midi.c music_pcm.c music_fader.c wavfile.c wavmix.c audio.c midi.c cdrom.c sndcnv.c musstream.c pcmlib.c music_bgm.c musstream_wav.c musstream_ogg.c musstream_wav_ima.c
SRC_MISC  := LittleEndian.c counter.c imput.c profile.c mt19937-1.c @JOY_S@.c eucsjis.c filecheck.c tempname.c
SRC_SCO   := cali.c scenario.c variable.c cmd_check.c hankana2sjis.c nact.c selection.c message.c savedata.c hankaku.c s39ain.c s39init.c
SRC_GRAPH := ags.c font.c flood.c cg.c ecopy.c antialiase.c alpha_plane.c cursor.c

SRC_NET := network.c
SRC_DLL := 

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
	@GLIB_LIBS@ \
	@GTK_LIBS@ \
	@ALSA_LIBS@ \
	@ESD_LIBS@ \
	@SDL_LIBS@ \
	@EXTRALIBS@ \
	@LIBINTL@ \
	$(LIBLTDL) \
	@VORBIS_LIBS@ \
	@VORBISFILE_LIBS@ \
	$(SRC_ABLEND:.s=.o) \
	$(SRC_AUDIO:.c=.o) \
	$(SRC_CDROM:.c=.o) \
	$(SRC_MIDI:.c=.o) \
	$(SRC_GRAPHDEV:.c=.o) \
	$(SRC_FONT:.c=.o) \
	$(SRC_MENU:.c=.o)

audio_oss.o: audio_oss.c audio_oss.h mixer_oss.c

audio_alsa.o: audio_alsa.c audio_alsa.h mixer_alsa.c

audio_esd.o: audio_esd.c audio_esd.h mixer_esd.c

image.o: image.c image15.c image16.c image24.c image24p.c


sdl_input.c:
	perl xsyskey.pl > sdl_input.c

.s.o:
	nasm -f elf $<

objs: $(system35_OBJECTS) $(xsystem35_DEPENDENCIES)
        @:

