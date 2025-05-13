include ../silent.mk

srcdir = ../../src

DEBUG = 0

CC = m68k-atari-mint-gcc
LIBS = -lgcc

LD = $(CC)
CPU_CFLAGS =
WARN = -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes -Werror
INCLUDES = -I$(srcdir) -I../common
CFLAGS = $(CPU_CFLAGS) $(OPT) $(WARN) $(INCLUDES)

ifeq ($(DEBUG),1)
OPT = -O0 -g
DEBUG_OBJS = nf_debug.o nf_ops.o
CFLAGS += -DDEBUG
else
OPT = -O2 -fomit-frame-pointer
LDFLAGS = -nostartfiles -nostdlib -s
STARTUP = crt0.o
endif

# The program must not be loaded to TT-RAM,
# because the screen_buf is part of it
LDFLAGS += -Wl,--mprg-flags,5 -s

GEMLIB_OBJS = \
	gemlib/ctrlcnts.o \
	gemlib/crysif.o \
	gemlib/ap_init.o \
	gemlib/crystal.o \
	gemlib/ap_exit.o \
	gemlib/ev_multi.o \
	gemlib/fm_do.o \
	gemlib/fm_dial.o \
	gemlib/fm_alert.o \
	gemlib/fm_cente.o \
	gemlib/fs_input.o \
	gemlib/gr_handl.o \
	gemlib/gr_mouse.o \
	gemlib/mn_bar.o \
	gemlib/mn_tnorm.o \
	gemlib/ob_draw.o \
	gemlib/rs_obfix.o \
	gemlib/wm_creat.o \
	gemlib/wm_open.o \
	gemlib/wm_close.o \
	gemlib/wm_delet.o \
	gemlib/wm_get.o \
	gemlib/wm_updat.o \
	gemlib/vclrwk.o \
	gemlib/vclsvwk.o \
	gemlib/vopnvwk.o \
	gemlib/vrocpyfm.o \
	gemlib/vdi.o \
	$(empty)

all:: midimaze.prg

OUT = -c -o 
PLATFORM_OBJS = \
	main.o \
	joystick.o \
	ikbd.o \
	sound.o \
	$(GEMLIB_OBJS) \
	$(DEBUG_OBJS) \
	$(empty)

include ../SRCFILES
include ../RULES

.c.o:
	$(AM_V_CC)$(CC) $(CFLAGS) -c -o $@ $<

midimaze.prg: $(OBJS)
	$(AM_V_LD)$(LD) -o $@ $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

$(GEMLIB_OBJS): gemlib/crysbind.h gemlib/vdilib.h

main.o: main.c
	$(AM_V_CC)$(CC) $(CFLAGS) $(OUT)$@ $<

crt0.o: crt0.S
	$(AM_V_CC)$(CC) -c -o $@ $<

ikbd.o: ikbd.S
	$(AM_V_CC)$(CC) -c -o $@ -Wa,--register-prefix-optional $<

clean::
	$(RM) *.o gemlib/*.o $(srcdir)/*.o *.prg
