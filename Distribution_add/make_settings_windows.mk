# Make settings for integrating Inform's user interface and core software.
# This is the Windows version, by David Kinder. To use, you will need
# Cygwin with GCC 3.4 installed.

BUILTINCOMPS = ../Build/Compilers
INTERNAL = ../Build/Internal
BUILTINHTML = ../Build/Documentation
BUILTINHTMLINNER = ../Build/Documentation/sections

I6COMPILERNAME = inform6

INDOCOPTS = windows_app
INRTPSOPTS = -nofont
INTESTOPTS = -threads=2 -no-colours

GCC = gcc
GCCOPTS = -mno-cygwin -DPLATFORM_WINDOWS -O2 -I.
GCCWARNINGS = -Wno-pointer-arith -Wno-unused-macros -Wno-shadow -Wno-cast-align -Wno-missing-noreturn -Wno-missing-prototypes -Wno-unused-parameter -Wno-padded
CBLORBWARNINGS = -Wno-format-nonliteral
LINK = gcc $(GCCOPTS) -ansi -s
LINKEROPTS = -Wl,--large-address-aware
ARTOOL = ar -r

GLULXEOS = OS_WIN32
INFORM6OS = PC_WIN32

