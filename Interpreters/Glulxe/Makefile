# Makefile for Inform 7 Glulxe, compiled with MinGW GCC 2.95

CC = gcc-2 -O3 -mwindows
CXX= g++-2 -O3 -mwindows
OPTIONS = -DFLOAT_COMPILE_SAFER_POWF -DWIN32
LIBS = -lwinmm

OUTPUT = ../../Build/Interpreters/glulxe

CFLAGS = $(OPTIONS) -w -fno-strict-aliasing -I ../../../Glk/Include
CXXFLAGS = -w -fno-strict-aliasing -I ../../../Glk/Include -I ../../../Glk/GlkDll
VPATH = ../Glulxe:../../../Glk/Glulxe:../../../Glk/GlkDll

OBJS =	accel.o exec.o files.o float.o funcs.o gestalt.o glkop.o heap.o main.o \
	operand.o osdepend.o profile.o search.o serial.o string.o vm.o \
	gi_blorb.o gi_dispa.o \
	I7Glk.o I7GlkBlankWindow.o I7GlkChannel.o I7GlkFile.o I7GlkGfxWindow.o \
	I7GlkGridWindow.o I7GlkPairWindow.o I7GlkStream.o I7GlkTextWindow.o \
	I7GlkWindow.o Startup.o GlkTime.o GlkUnicode.o

glulxe: $(OBJS)
	$(CXX) -s -o $(OUTPUT) $(OBJS) $(LIBS)

clean:
	del *.o *.a

