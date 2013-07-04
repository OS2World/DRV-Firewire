fwresmg.exe : fwresmg.obj fwcom.obj oui.obj fwresmg.h rhnls.lib fwresmg.res .AUTODEPEND
 @%write fwresmg.lk1 FIL fwresmg.obj
 @%write fwresmg.lk1 FIL fwcom.obj
 @%write fwresmg.lk1 FIL oui.obj
 @%append fwresmg.lk1 LIBR rhnls.lib
 *wlink name fwresmg d all SYS os2v2 pm op m op maxe=25 op q @fwresm&
g.lk1
 rc fwresmg.res fwresmg.exe

fwresmg.res : fwresmg.rc .AUTODEPEND
 rc -r fwresmg.rc fwresmg.res

fwresmg.obj : fwresmg.c .AUTODEPEND
 *wcc386 fwresmg.c -w4 -e25 -zq -od -d2 -5r -bt=os2 -mf
 
fwcom.obj : fwcom.c .AUTODEPEND
 *wcc386 fwcom.c -w4 -e25 -zq -od -d2 -5r -bt=os2 -mf

oui.obj : oui.c .AUTODEPEND
 *wcc386 oui.c -w4 -e25 -zq -od -d2 -5r -bt=os2 -mf

clean:
   if exist *.exe del *.exe
   if exist *.map del *.map
   if exist *.lk1 del *.lk1
   if exist *.obj del *.obj
   if exist *.err del *.err
   if exist *.res del *.res
   if exist *.sym del *.sym
