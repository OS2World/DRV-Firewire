# $Id: makefile,v 1.3 2004/08/06 11:38:18 doctor64 Exp $

#
# Set the environmental variables for compiling
#
.ERASE

.SUFFIXES:
.SUFFIXES: .sys .exe .obj .asm .inc .def .lrf .lst .sym .map .cpp .c .h .lib .dbg .dll .itl .hlp

NAME   = fireMgr

.BEFORE
!ifndef %WATCOM
   set WATCOM=..\..\..\..\WATCOM
!endif
   set INCLUDE=$(%WATCOM)\H;$(%DDK)\base\H;$(%DDK)\base\INC;..\common;
   #$(%WPDDLIB)\base\h;
   set PATH=$(%PATH);\$(%DDK)\base\TOOLS;$(%DDK)\tools;
   set LIB=$(%WATCOM)\lib386;$(%WATCOM)\lib386\os2;$(%DDK)\base\lib;
!ifndef %IPFC
   set IPFC=$(%DDK)\TOOLS
!endif

# Options for Watcom 16-bit C compiler
#  -bt=os2   = Build target OS is OS/2
#  -ms       = Memory model small
#  -3        = Enable use of 80386 instructions
#  -4        = Optimize for 486 (implies -3)
#  -5        = Optimize for Pentium (implies -3)
#  -j        = char default is unsigned
#  -d1       = Include line number info in object
#              (necessary to produce assembler listing)
#  -d2       = Include debugging info for ICAT
#              (necessary to produce assembler listing)
#  -o        = Optimization - i = enable inline intrinsic functions
#                             r = optimize for 80486 and pentium pipes
#                             s = space is preferred to time
#                             l = enable loop optimizations
#                             a = relax aliasing constraints
#                             n = allow numerically unstable optimizations
#  -s        = Omit stack size checking from start of each function
#  -zl       = Place no library references into objects
#  -wx       = Warning level set to maximum (vs 1..4)
#  -zfp      = Prevent use of FS selector
#  -zgp      = Prevent use of GS selector
#  -zq       = Operate quietly
#  -zm       = Put each function in its own segment
#  -zu       = Do not assume that SS contains segment of DGROUP
#  -hd       = Include Dwarf debugging info
#  -hc       = Include Codeview debugging info
#
CC=wcc
CPP=wpp
!ifdef SMP
COPTS=-dSMP
!endif
#edbug build
!ifdef %debug
DEBUG=
COPTS=-dDEBUG
!endif

#CFLAGS=-ms -5 -bt=os2 -d2 -olinars -s -j -wx -zl -zfp -zgp -zq -zu -hc $(COPTS)
CFLAGS=-ms -5 -bt=os2 -d2 -oi -s -j -wx -zl -zfp -zgp -zq -zu -hc -zp1  $(COPTS)

# Options for Watcom assembler
#  -bt=os2   = Build target OS is OS/2
#  -d1       = Include line number info in object
#              (necessary to produce assembler listing)
#  -i        = Include list
#  -zq       = Operate quietly
#  -3p       = 80386 protected-mode instructions
#
ASM=wasm
AFLAGS=-d1 -zq -3p $(DEBUG)

LINK=wlink
RC=rc

#########################################
# Definitions for Help Compiler
#########################################
IPF=ipfc
L=ENU
P=437
C=1

# Inference rules
.c.obj: .AUTODEPEND
     $(CC) $(CFLAGS) $*.c
     wdis -l -s $*
#     wdisasm -l -s -b -e -p $*

.cpp.obj: .AUTODEPEND
     $(CPP) $(CFLAGS) $*.cpp
     wdis -l -s $*
#     wdisasm -l -s -b -e -p $*

.asm.obj: .AUTODEPEND
     $(ASM) $(AFLAGS) $*.asm
     wdis -l -s $*
#     wdisasm -l -s -b -e -p $*

.c.exe: .AUTODEPEND
   wcc386 -s -j -oxsz -zm $*.c
   wlink format os2 flat file $*.obj option eliminate

# Object file list
OBJS1=segments.obj header.obj globalmgr.obj listmgr.obj entrymgr.obj
#segments.obj header.obj
# iprintf.obj
OBJS2= csr.obj nodemgr.obj strategy.obj ioctl.obj initmgr.obj
#
#strategy.obj ioctl.obj  pcidev.obj init.obj
#OBJS3=ohci.obj isr.obj host.obj dma.obj  configrom.obj
#tlabelpool.obj stack.obj
#OBJS4=selfid.obj global.obj packet.obj csr.obj
#OBJS4=..\ohci\host.obj
# common objects
OBJS5=..\common\malloc.obj ..\common\rmhelp.obj ..\common\list.obj ..\common\memory.obj ..\common\csr1212.obj ..\common\jiffies.obj
OBJS6=..\common\timer.obj ..\common\cpu_be.obj ..\common\sems.obj
!ifdef debug
OBJS7=..\common\ddprintf.obj
!endif

#..\common\stack.obj
#timer.obj cpu_be.obj
#pcifunc.obj
#isr.obj
OBJS=$(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6) $(OBJS7)

all: $(NAME).sys
#test.exe

#test.exe: test.c makefile

$(NAME).lrf: makefile
   @%write $^@ system os2 dll
   @%write $^@ option quiet
   @%write $^@ option verbose
   @%write $^@ option caseexact
   @%write $^@ option symfile=$(NAME).dbg
   @%write $^@ debug codeview
   @%write $^@ option cache
   @%write $^@ option map
   @%write $^@ name $(NAME).sys
   @for %f in ($(OBJS)) do @%append $^@ file %f
   @%write $^@ import DOSIODELAYCNT DOSCALLS.427
   @%write $^@ library $(%DDK)\base\lib\os2286.lib
   @%write $^@ library $(%WPDDLIB)\base\src\dev\mme\wpddlib\runtime.lib
   @%write $^@ library $(%DDK)\BASE\SRC\DEV\resource\rmcalls\rmcalls.lib
   @%write $^@ library $(%WATCOM)\lib286\os2\clibs.lib

$(NAME).sys: $(OBJS) makefile $(NAME).lrf $(%DDK)\base\lib\os2286.lib $(%WPDDLIB)\base\src\dev\mme\wpddlib\runtime.lib $(%DDK)\BASE\SRC\DEV\resource\rmcalls\rmcalls.lib $(%WATCOM)\lib286\os2\clibs.lib
   $(LINK) @$(NAME).lrf
    wat2map $(NAME).map $(NAME).ma_
    mapsym $(NAME).ma_

.silent
clean: .symbolic
   if exist *.sys del *.sys
   if exist *.map del *.map
   if exist *.lrf del *.lrf
   if exist *.lst del *.lst
   if exist *.obj del *.obj
   if exist *.err del *.err
#   if exist *.$$$ del *.$$$
   if exist *.dbg del *.dbg
   if exist *.ma_ del *.ma_
   if exist *.sym del *.sym

