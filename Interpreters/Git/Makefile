# Makefile for Inform 7 Git, compiled with MinGW GCC 2.95

CC = gcc-2 -O3 -mwindows
CXX= g++-2 -O3 -mwindows
OPTIONS = -DUSE_INLINE -DUSE_DIRECT_THREADING -DUSE_OWN_POWF

OUTPUT = ../../Build/Interpreters/git

CFLAGS = $(OPTIONS) -w -I ../../../Glk/Include
CXXFLAGS = -w -I ../../../Glk/Git -I ../../../Glk/Include -I ../../../Glk/GlkDll
VPATH = ../Glulxe:../../../Glk/Git:../../../Glk/GlkDll:$(LIBPATH)/zlib

OBJS =	git.o memory.o compiler.o opcodes.o operands.o peephole.o terp.o \
	glkop.o search.o savefile.o saveundo.o gestalt.o accel.o heap.o \
	gi_blorb.o gi_dispa.o \
	I7Glk.o I7GlkBlankWindow.o I7GlkChannel.o I7GlkFile.o I7GlkGfxWindow.o \
	I7GlkGridWindow.o I7GlkPairWindow.o I7GlkStream.o I7GlkTextWindow.o \
	I7GlkWindow.o Startup.o GlkUnicode.o

git: $(OBJS)
	$(CXX) -s -o $(OUTPUT) $(OBJS) $(LIBS)

clean:
	del *.o *.a

