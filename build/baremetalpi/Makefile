#
# Makefile
#
TIC80HOME = ../..
TIC803RD = $(TIC80HOME)/vendor
MRUBYDIR = $(TIC803RD)/mruby
CIRCLESTDLIB = $(TIC803RD)/circle-stdlib
CIRCLEHOME = $(CIRCLESTDLIB)/libs/circle
NEWLIBDIR = $(CIRCLESTDLIB)/install/arm-none-circle
TIC80LIB = $(TIC80HOME)/build/lib
TIC80SRC = $(TIC80HOME)/src
TIC80BUILD = $(TIC80HOME)/build
TIC80STUDIOSRC = $(TIC80SRC)/studio
BAREMETALSYSTEM = $(TIC80SRC)/system/baremetalpi

OBJS    = \
    $(BAREMETALSYSTEM)/main.o $(BAREMETALSYSTEM)/kernel.o $(BAREMETALSYSTEM)/customchargenerator.o \
    $(BAREMETALSYSTEM)/customscreen.o $(BAREMETALSYSTEM)/utils.o $(BAREMETALSYSTEM)/gamepads.o

include $(CIRCLEHOME)/Rules.mk

CFLAGS += -I "$(NEWLIBDIR)/include" -I $(STDDEF_INCPATH) -I $(CIRCLESTDLIB)/include -I $(TIC80HOME)/include -I $(TIC80SRC) \
      -I $(TIC80STUDIOSRC) -I $(TIC80BUILD) -D BAREMETALPI

LIBS :=  \
    $(TIC80LIB)/libtic80studio.a \
    $(TIC80LIB)/libtic80core.a \
    $(TIC80LIB)/libgiflib.a \
    $(TIC80LIB)/liblpeg.a \
    $(TIC80LIB)/libluaapi.a \
    $(TIC80LIB)/liblua.a \
    $(TIC80LIB)/libmoon.a \
    $(TIC80LIB)/libfennel.a \
    $(TIC80LIB)/libzip.a \
    $(TIC80LIB)/libblipbuf.a \
    $(TIC80LIB)/libsquirrel.a \
    $(TIC80LIB)/libpython.a \
    $(TIC80LIB)/libpocketpy.a \
    $(TIC80LIB)/libscheme.a \
    $(TIC80LIB)/libwren.a \
    $(TIC80LIB)/libwasm.a \
    $(TIC80LIB)/libjanet.a \
    $(TIC80LIB)/libargparse.a \
    $(TIC80LIB)/libwave_writer.a \
    $(TIC80LIB)/libjs.a \
    $(TIC80LIB)/libquickjs.a \
    $(TIC80LIB)/libzlib.a \
    $(TIC80LIB)/libpng.a \
    $(TIC80LIB)/libruby.a \
    $(MRUBYDIR)/build/target/lib/libmruby.a \
    $(NEWLIBDIR)/lib/libc.a \
    $(NEWLIBDIR)/lib/libcirclenewlib.a \
    $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
    $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
    $(CIRCLEHOME)/addon/linux/liblinuxemu.a \
    $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
    $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
    $(CIRCLEHOME)/lib/sound/libsound.a \
    $(CIRCLEHOME)/lib/usb/libusb.a \
    $(CIRCLEHOME)/lib/input/libinput.a \
    $(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
    $(CIRCLEHOME)/lib/fs/libfs.a \
    $(CIRCLEHOME)/lib/net/libnet.a \
    $(CIRCLEHOME)/lib/sched/libsched.a \
    $(CIRCLEHOME)/lib/libcircle.a \
    $(NEWLIBDIR)/lib/libm.a
