/*
** Module   :DELAY.H
** Abstract : delay functions
**
** Copyright (C) Alexandr Cherkaev
**
** Log: Mon  31/05/2004	Created
**
*/

// number of dec/jnz empty loops for 0.5 microseconds delay (from OS2LDR).
// note: _offset_ of this entry needed actualy.
extern USHORT DOSIODELAYCNT;
#pragma aux DOSIODELAYCNT "DOSIODELAYCNT";

// delay for 0.5 microseconds for IO delays
VOID DevIOdelayQuant();
#pragma aux DevIOdelayQuant = \
        "mov ax,offset DOSIODELAYCNT" \
        "@@Loop:" \
        "dec ax" \
        "jnz @@Loop" \
        parm nomemory [] \
        modify nomemory [ax];

// uSeconds param - in microseconds units
// note: on my test machine (Celeron 553), 100 seconds delay gives actual
// result in 105 seconds - 5 % accuracy.
VOID DevIODelay(USHORT uSeconds);
#pragma aux DevIODelay = \
        "mov dx,offset DOSIODELAYCNT" \
        "nop" \
        "shl ax,1" \
        "@@Loop:" \
        "mov cx,dx" \
        "@@Loop2:" \
        "dec cx" \
        "jnz @@Loop2" \
        "dec ax" \
        "jnz @@Loop" \
        parm nomemory [ax] \
        modify nomemory [ax cx dx];
