; $Id: segments.asm,v 1.1 2004/07/14 13:48:51 doctor64 Exp $

;; Defines segment ordering for 16-bit DD's with MSC 6.0
;;
;; MODIFICATION HISTORY
;; DATE       PROGRAMMER   COMMENT
;; 01-Jul-95  Timur Tabi   Creation

.386
.seq

_HEADER       segment dword public use16 'DATA'
_HEADER       ends

_DATA         segment dword public use16 'DATA'
_DATA         ends

CONST         segment dword public use16 'DATA'
CONST         ends

CONST2        segment dword public use16 'DATA'
CONST2        ends

_BSS          segment dword public use16 'BSS'
_BSS          ends

MEMBLOCK      struc
uSize         WORD      0
pmbNext       WORD      0
;ulSignature   DWORD     12345678h
MEMBLOCK      ends

HEAPSIZE      equ       32768
;4096
FREESPACE     equ       (((offset _end_of_heap) - (offset _abHeap) - (sizeof MEMBLOCK)) and 0FFFCh)

_HEAP         segment dword public use16 'ENDDS'
public        _abHeap
public        _end_of_heap
public        _uMemFree
_uMemFree     WORD      FREESPACE
_abHeap       MEMBLOCK  { FREESPACE }
              db        (HEAPSIZE - sizeof MEMBLOCK) dup (0)
_end_of_heap  label     BYTE
_HEAP         ends

_INITDATA     segment dword public use16 'ENDDS'
_INITDATA     ends

_TEXT         segment dword public use16 'CODE'
_TEXT         ends

RMCODE        SEGMENT DWORD PUBLIC USE16 'CODE'
RMCODE        ENDS

_ENDCS        segment dword public use16 'CODE'
public        end_of_text_
end_of_text_  proc
end_of_text_  endp
_ENDCS        ends

_INITTEXT     segment dword public use16 'CODE'
_INITTEXT     ends

DGROUP        group _HEADER, CONST, CONST2, _DATA, _BSS, _HEAP, _INITDATA
CGROUP        GROUP _TEXT, RMCODE, _ENDCS, _INITTEXT

end
