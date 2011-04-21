#FIXME: check for -ldl (mingw doesn't have this)

SERVER_SRCS += \
	qcommon/cmd.c \
	qcommon/ioapi.c \
	qcommon/unzip.c \
	qcommon/cmodel.c \
	qcommon/common.c \
	qcommon/cvar.c \
	qcommon/files.c \
	qcommon/md4.c \
	qcommon/md5.c \
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
	game/q_shared.c \
	\
	null/cl_null.c \
	null/cd_null.c

NET_UDP=net_udp

ifeq ($(TARGET_OS),linux-gnu)
	SERVER_SRCS += \
		linux/q_shlinux.c \
		linux/sys_linux.c \
		unix/sys_unix.c \
		unix/glob.c \
		unix/$(NET_UDP).c
endif

ifeq ($(TARGET_OS),freebsd)
	SERVER_SRCS += \
		linux/q_shlinux.c \
		linux/sys_linux.c \
		unix/sys_unix.c \
		unix/glob.c \
		unix/$(NET_UDP).c
endif

ifeq ($(TARGET_OS),netbsd)
	SERVER_SRCS += \
		linux/q_shlinux.c \
		linux/sys_linux.c \
		unix/sys_unix.c \
		unix/glob.c \
		unix/$(NET_UDP).c
endif

ifeq ($(TARGET_OS),mingw32)
	SERVER_SRCS+=\
		win32/q_shwin.c \
		win32/sys_win.c \
		win32/conproc.c  \
		win32/net_wins.c \
		win32/q2.rc
endif

ifeq ($(TARGET_OS),darwin)
	SERVER_SRCS+=\
		macosx/sys_osx.m \
		unix/glob.c \
		unix/sys_unix.c \
		unix/$(NET_UDP).c \
		macosx/q_shosx.c
endif

SERVER_OBJS= \
	$(patsubst %.c, $(BUILDDIR)/server/%.o, $(filter %.c, $(SERVER_SRCS))) \
	$(patsubst %.m, $(BUILDDIR)/server/%.o, $(filter %.m, $(SERVER_SRCS))) \
	$(patsubst %.rc, $(BUILDDIR)/server/%.o, $(filter %.rc, $(SERVER_SRCS)))

SERVER_DEPS=$(SERVER_OBJS:%.o=%.d)
SERVER_TARGET=q2xpded$(EXE_EXT)

ifeq ($(BUILD_DEDICATED),1)
	ALL_OBJS+=$(SERVER_OBJS)
	ALL_DEPS+=$(SERVER_DEPS)
	TARGETS+=$(SERVER_TARGET)
endif

DEDICATED_CFLAGS=-DDEDICATED_ONLY

SERVERLIBS+=-lz -lm

# Say how to link the exe
$(SERVER_TARGET): $(SERVER_OBJS) $(BUILDDIR)/.dirs
	@echo " * [DEDI] ... linking"; \
		$(CC) $(LDFLAGS) -o $@ $(SERVER_OBJS) $(SERVERLIBS) $(LIBS)

# Say how to build .o files from .c files for this module
$(BUILDDIR)/server/%.o: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [DEDI] $<"; \
		$(CC) $(CFLAGS) $(DEDICATED_CFLAGS) -o $@ -c $<

ifeq ($(TARGET_OS),mingw32)
# Say how to build .o files from .rc files for this module
$(BUILDDIR)/server/%.o: $(SRCDIR)/%.rc $(BUILDDIR)/.dirs
	@echo " * [RC  ] $<"; \
		$(WINDRES) -DCROSSBUILD -i $< -o $@
endif

# Say how to build .o files from .m files for this module
$(BUILDDIR)/server/%.m: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [DEDI] $<"; \
		$(CC) $(CFLAGS) $(DEDICATED_CFLAGS) -o $@ -c $<

# Say how to build the dependencies
ifdef BUILDDIR
$(BUILDDIR)/server/%.d: $(SRCDIR)/%.c $(BUILDDIR)/.dirs
	@echo " * [DEP ] $<"; \
		$(DEP) $(DEDICATED_CFLAGS)

ifeq ($(TARGET_OS),mingw32)
$(BUILDDIR)/server/%.d: $(SRCDIR)/%.rc $(BUILDDIR)/.dirs
	@echo " * [DEP ] $<"; touch $@
endif

$(BUILDDIR)/server/%.d: $(SRCDIR)/%.m $(BUILDDIR)/.dirs
	@echo " * [DEP ] $<"; \
		$(DEP) $(DEDICATED_CFLAGS)
endif





