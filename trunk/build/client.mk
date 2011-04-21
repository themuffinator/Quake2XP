CLIENT_SRCS = \
	client/avi_export.c \
	client/cl_cin.c \
	client/cl_decals.c \
	client/cl_ents.c \
	client/cl_fx.c \
	client/cl_images.c \
	client/cl_input.c \
	client/cl_inv.c \
	client/cl_localents.c \
	client/cl_main.c \
	client/cl_newfx.c \
	client/cl_parse.c \
	client/cl_particles.c \
	client/cl_pred.c \
	client/cl_scrn.c \
	client/cl_tempents.c \
	client/cl_view.c \
	client/console.c \
	client/keys.c \
	client/menu.c \
	client/qmenu.c \
	client/snd_dma.c \
	client/snd_mem.c \
#	client/snd_mix.c \
	client/x86.c \
	\
	qcommon/cmd.c \
#	qcommon/ioapi.c \
#	qcommon/unzip.c \
	qcommon/cmodel.c \
	qcommon/common.c \
	qcommon/cvar.c \
	qcommon/irc.c \
	qcommon/files.c \
	qcommon/md4.c \
	qcommon/net_chan.c \
	qcommon/scripts.c \
	\
	server/sv_ccmds.c \
	server/sv_game.c \
	server/sv_init.c \
	server/sv_main.c \
	server/sv_send.c \
	server/sv_user.c \
	server/sv_world.c \
	\
	game/q_shared.c

NET_UDP=net_udp

ifeq ($(TARGET_OS),linux-gnu)
	CLIENT_SRCS+= \
		linux/q_shlinux.c \
		linux/vid_so.c \
		linux/sys_linux.c \
		unix/sys_unix.c \
		unix/glob.c \
		unix/$(NET_UDP).c

	ifeq ($(HAVE_OPENAL),1)
		CLIENT_SRCS+= \
			linux/qal_linux.c
	endif
	CLIENT_CD=linux/cd_linux.c
endif

ifeq ($(TARGET_OS),freebsd)
	CLIENT_SRCS+= \
		linux/q_shlinux.c \
		linux/vid_so.c \
		linux/sys_linux.c \
		unix/sys_unix.c \
		unix/glob.c \
		unix/$(NET_UDP).c
	ifeq ($(HAVE_OPENAL),1)
		CLIENT_SRCS+=\
			linux/qal_linux.c
	endif
	CLIENT_CD=linux/cd_linux.c
endif

# Exactly the same as freebsd.  Is there a better way to do this?
ifeq ($(TARGET_OS),netbsd)
	CLIENT_SRCS+= \
		linux/q_shlinux.c \
		linux/vid_so.c \
		linux/sys_linux.c \
		unix/sys_unix.c \
		unix/glob.c \
		unix/$(NET_UDP).c
	ifeq ($(HAVE_OPENAL),1)
		CLIENT_SRCS+=\
			linux/qal_linux.c
	endif
	CLIENT_CD=linux/cd_linux.c
endif

ifeq ($(TARGET_OS),mingw32)
	CLIENT_SRCS+=\
		win32/q2.rc \
		win32/q_shwin.c \
		win32/vid_dll.c \
		win32/in_win.c \
		win32/conproc.c  \
		win32/sys_win.c \
		win32/net_wins.c
	CLIENT_CD=win32/cd_win.c

	ifeq ($(HAVE_OPENAL),1)
		CLIENT_SRCS+=\
			win32/qal_win.c
	endif
endif

ifeq ($(TARGET_OS),darwin)
	CLIENT_SRCS+= \
		macosx/sys_osx.m \
		macosx/vid_osx.m \
		macosx/in_osx.m \
		macosx/snddma_osx.m \
		unix/glob.c \
		unix/sys_unix.c \
		unix/$(NET_UDP).c \
		macosx/q_shosx.c
		# FIXME Add more objects
   CLIENT_CD+=macosx/cd_osx.m
endif

ifeq ($(TARGET_OS),mingw32)
	CLIENT_SRCS+=$(CLIENT_CD)
else
	ifeq ($(HAVE_SDL),1)
		CLIENT_SRCS+=unix/cd_sdl.c
		CLIENT_LIBS+=$(SDL_LIBS)
	else
		CLIENT_SRCS+=$(CLIENT_CD)
	endif
endif

CLIENT_OBJS= \
	$(patsubst %.c, $(BUILDDIR)/client/%.o, $(filter %.c, $(CLIENT_SRCS))) \
	$(patsubst %.m, $(BUILDDIR)/client/%.o, $(filter %.m, $(CLIENT_SRCS))) \
	$(patsubst %.rc, $(BUILDDIR)/client/%.o, $(filter %.rc, $(CLIENT_SRCS)))

CLIENT_DEPS=$(CLIENT_OBJS:%.o=%.d)
CLIENT_TARGET=q2xp$(EXE_EXT)

ifeq ($(BUILD_CLIENT),1)
	ALL_OBJS+=$(CLIENT_OBJS)
	ALL_DEPS+=$(CLIENT_DEPS)
	TARGETS+=$(CLIENT_TARGET)
endif

# Say how to link the exe
$(CLIENT_TARGET): $(CLIENT_OBJS) $(BUILDDIR)/.dirs
	@echo " * [Q2XP] ... linking $(LNKFLAGS)"; \
		$(CC) $(LDFLAGS) -o $@ $(CLIENT_OBJS) $(LIBS) $(LNKFLAGS) $(CLIENT_LIBS)

# Say how to build .o files from .c files for this module
$(BUILDDIR)/client/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [Q2XP] $<"; \
		$(CC) $(CFLAGS) -o $@ -c $<

ifeq ($(TARGET_OS),mingw32)
# Say how to build .o files from .rc files for this module
$(BUILDDIR)/client/%.o: $(SRCDIR)/%.rc $(BUILDDIR)/.dirs
	@echo " * [RC  ] $<"; \
		$(WINDRES) -DCROSSBUILD -i $< -o $@
endif

# Say how to build .o files from .m files for this module
$(BUILDDIR)/client/%.m: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [Q2XP] $<"; \
		$(CC) $(CFLAGS) -o $@ -c $<

# Say how to build the dependencies
ifdef BUILDDIR
$(BUILDDIR)/client/%.d: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [DEP ] $<"; $(DEP)

ifeq ($(TARGET_OS),mingw32)
$(BUILDDIR)/client/%.d: $(SRCDIR)/%.rc $(BUILDDIR)/.dirs
	@echo " * [DEP ] $<"; touch $@
endif

$(BUILDDIR)/client/%.d: $(SRCDIR)/%.m $(BUILDDIR)/.dirs
	@echo " * [DEP ] $<"; $(DEP)
endif





