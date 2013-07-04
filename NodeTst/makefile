# makefile
# Created by IBM WorkFrame/2 MakeMake at 19:46:06 on 29 July 2004
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES:

.SUFFIXES: \
    .c .cpp .obj 

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ti /C %s

{D:\Projects\Netlabs\firewire\NodeTst}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ti /C %s

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ti /C %s

{D:\Projects\Netlabs\firewire\NodeTst}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ti /C %s

all: \
    .\test.exe

.\test.exe: \
    .\oui.obj \
    .\test.obj
    @echo " Link::Linker "
    icc.exe @<<
     /B" /de"
     /Fetest.exe 
     .\oui.obj
     .\test.obj
<<

.\oui.obj: \
    D:\Projects\Netlabs\firewire\NodeTst\oui.c

.\test.obj: \
    D:\Projects\Netlabs\firewire\NodeTst\test.cpp \
    {D:\Projects\Netlabs\firewire\NodeTst;$(INCLUDE);}../nodemgr/ioctl.h
