# Makefile for Inform 7 Glulxe, compiled with Cygwin/MinGW GCC 3.4.4

CFLAGS = -O3 -mno-cygwin -mwindows -I ../../../Glk/Include \
	-DVERIFY_MEMORY_ACCESS -DFLOAT_USE_OWN_POWF
CXXFLAGS = $(CFLAGS) -I ../../../Glk/GlkDll
VPATH   = ../../../Glk/Glulxe:../../../Glk/GlkDll

OUTPUT = ../../Build/Interpreters/glulxe

OBJS =	accel.o exec.o files.o float.o funcs.o gestalt.o glkop.o heap.o main.o \
	operand.o osdepend.o profile.o search.o serial.o string.o vm.o \
	gi_blorb.o gi_dispa.o \
	I7Glk.o I7GlkBlankWindow.o I7GlkChannel.o I7GlkFile.o I7GlkGfxWindow.o \
	I7GlkGridWindow.o I7GlkPairWindow.o I7GlkStream.o I7GlkTextWindow.o \
	I7GlkWindow.o Startup.o GlkUnicode.o

all: glulxe

glulxe: $(OBJS)
	$(CXX) -mno-cygwin -mwindows -s -o $(OUTPUT) $(OBJS)

clean:
	rm -f *.o *.a

