#
#       This makefile creates the OS/2 Firewire stack device drivers
#

#
# Should be the default tag for all general processing
#

#debug: 
#	set DEBUG=DEBUG

all: common stack ohci nodemgr

common: .always
        cd  common
        call wmake
        cd ..

stack: .always
        cd  stack
        call wmake
        cd ..

nodemgr: .always
        cd  nodemgr
        call wmake
        cd ..

ohci:  .always
        cd  ohci
        call wmake
        cd ..

clean: .symbolic
        cd common
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

       cd ..
       cd ohci
       if exist *.sys del *.sys
       if exist *.map del *.map
       if exist *.lrf del *.lrf
       if exist *.lst del *.lst
       if exist *.obj del *.obj
       if exist *.err del *.err
#   if exist *.$$$ del *.$$$
	   if exist *.ma_ del *.ma_
       if exist *.sym del *.sym

       if exist *.dbg del *.dbg
       cd ..
       cd stack
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
       cd ..
       cd nodemgr
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
       cd ..

