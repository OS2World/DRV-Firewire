/*DDK*************************************************************************/
/*                                                                           */
/* COPYRIGHT    Copyright (C) 1992 IBM Corporation                           */
/*                                                                           */
/*    The following IBM OS/2 source code is provided to you solely for       */
/*    the purpose of assisting you in your development of OS/2 device        */
/*    drivers. You may use this code in accordance with the IBM License      */
/*    Agreement provided in the IBM Developer Connection Device Driver       */
/*    Source Kit for OS/2. This Copyright statement may not be removed.      */
/*                                                                           */
/*****************************************************************************/
/* SCCSID = %W% %E% */
/****************************************************************************
 *                                                                          *
 *                                                                          *
 * The following IBM OS/2 source code is provided to you solely for the     *
 * the purpose of assisting you in your development of OS/2 device drivers. *
 * You may use this code in accordance with the IBM License Agreement       *
 * provided in the IBM Device Driver Source Kit for OS/2.                   *
 *                                                                          *
 ****************************************************************************/
/**@internal %W%
 *  Allocation of FM synth resources to note generation.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 *  01-Nov-94  Joe Nord     Creation
 */

// don't put this stuff in the init segment if we
// will be doing debug stuff
#ifndef DEBUG
#pragma code_seg ("_inittext");
#pragma data_seg ("_initdata","endds");
#endif

#ifdef __cplusplus         //###
extern "C" {               //###
#endif                     //###
#define INCL_NOPMAPI
#define INCL_DOSMISC
#include <os2.h>
#ifdef __cplusplus         //###
}                          //###
#endif                     //###
#include <devhelp.h>

#include "ddprintf.h"
#include "iodelay.h"
#include <include.h>       //### Defines pragma's & more.  Include last.

#define CR 0x0d
#define LF 0x0a

#define LEADING_ZEROES          0x8000
#define SIGNIFICANT_FIELD       0x0007

#define         MAGIC_COMM_PORT1 0x3f8           // pulled from word ptr 40:0
#define         MAGIC_COMM_PORT2 0x2f8           // pulled from word ptr 40:0


#define UART_DATA               0x00            // UART Data port
#define UART_INT_ENAB           0x01            // UART Interrupt enable
#define UART_INT_ID             0x02            // interrupt ID
#define UART_LINE_CTRL          0x03            // line control registers
#define UART_MODEM_CTRL         0x04            // modem control register
#define UART_LINE_STAT          0x05            // line status register
#define UART_MODEM_STAT         0x06            // modem status regiser
#define UART_DIVISOR_LO         0x00            // divisor latch least sig
#define UART_DIVISOR_HI         0x01h           // divisor latch most sig

// local prototypes
void CharOut( char );

typedef unsigned short WORD;
typedef unsigned long DWORD;

char ddhextab[]="0123456789ABCDEF";

int terminal_port=MAGIC_COMM_PORT1;

void setddprintfport(char port)
{
	if (port==1) terminal_port=MAGIC_COMM_PORT1;
    if (port==2) terminal_port=MAGIC_COMM_PORT2;
}

                                        //----------- ddprintf_DecWordToASCII -
char far * ddprintf_DecWordToASCII(char far *StrPtr, WORD wDecVal, WORD Option)
{
  BOOL fNonZero=FALSE;
  WORD Digit;
  WORD Power=10000;

  while (Power)
     {
     Digit=0;
     while (wDecVal >=Power)                   //Digit=wDecVal/Power;
        {
        Digit++;
        wDecVal-=Power;
        }

     if (Digit)
        fNonZero=TRUE;

     if (Digit ||
         fNonZero ||
         (Option & LEADING_ZEROES) ||
         ((Power==1) && (fNonZero==FALSE)))
         {
         *StrPtr=(char)('0'+Digit);
         StrPtr++;
         }

     if (Power==10000)
        Power=1000;
     else if (Power==1000)
        Power=100;
     else if (Power==100)
        Power=10;
     else if (Power==10)
        Power=1;
     else
        Power=0;
     } // end while

  return (StrPtr);
}

                                        //----------- ddprintf_DecLongToASCII -
char far * ddprintf_DecLongToASCII(char far *StrPtr, DWORD lDecVal,WORD Option)
{
   BOOL  fNonZero=FALSE;
   DWORD Digit;
   DWORD Power=1000000000;                      // 1 billion

   while (Power)
      {
      Digit=0;                                                                        // Digit=lDecVal/Power
      while (lDecVal >=Power)                   // replaced with while loop
         {
         Digit++;
         lDecVal-=Power;
         }

      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==1) && (fNonZero==FALSE)))
         {
         *StrPtr=(char)('0'+Digit);
         StrPtr++;
         }

      if (Power==1000000000)                    // 1 billion
         Power=100000000;
      else if (Power==100000000)
         Power=10000000;
      else if (Power==10000000)
         Power=1000000;
      else if (Power==1000000)
         Power=100000;
      else if (Power==100000)
         Power=10000;
      else if (Power==10000)
         Power=1000;
      else if (Power==1000)
         Power=100;
      else if (Power==100)
         Power=10;
      else if (Power==10)
         Power=1;
      else
         Power=0;
      }
   return (StrPtr);
}
                                        //----------- ddprintf_HexWordToASCII -
char far * ddprintf_HexWordToASCII(char far *StrPtr, WORD wHexVal, WORD Option)
{
   BOOL fNonZero=FALSE;
   WORD Digit;
   WORD Power=0xF000;
   WORD ShiftVal=12;

   while (Power)
      {
      Digit=(wHexVal & Power)>>ShiftVal;
      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==0x0F) && (fNonZero==FALSE)))
         //*StrPtr++=(char)('0'+Digit);
         *StrPtr++=ddhextab[Digit];

      Power>>=4;
      ShiftVal-=4;
      } // end while

   return (StrPtr);
}

                                        //----------- ddprintf_HexLongToASCII -
char far * ddprintf_HexLongToASCII(char far *StrPtr, DWORD wHexVal, WORD Option)
{
   BOOL  fNonZero=FALSE;
   DWORD Digit;
   DWORD Power=0xF0000000;
   DWORD ShiftVal=28;

   while (Power)
      {
      Digit=(wHexVal & Power)>>ShiftVal;
      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==0x0F) && (fNonZero==FALSE)))
          *StrPtr++=ddhextab[Digit];

      if (Power==0xF0000000)                  // 1 billion
         Power=0xF000000;
      else if (Power==0xF000000)
         Power=0xF00000;
      else if (Power==0xF00000)
         Power=0xF0000;
      else if (Power==0xF0000)
         Power=0xF000;
      else if (Power==0xF000)
         Power=0xF00;
      else if (Power==0xF00)
         Power=0xF0;
      else if (Power==0xF0)
         Power=0xF;
      else Power=0;

      ShiftVal-=4;
      } // end while

   return (StrPtr);
}

char dd_BuildString[1024];

                                        //-------------------------- ddprintf -

VOID cdecl dprintf (char far *DbgStr , ...)
{
   char far *BuildPtr=dd_BuildString;
   char far *pStr=(char far *) DbgStr;
   char far *SubStr;
   union {
         VOID    far *VoidPtr;
         WORD    far *WordPtr;
         DWORD   far *LongPtr;
         DWORD far *StringPtr;
         } Parm;
   WORD wBuildOption;

    // if the debug flags are not on then check to see if we
    // running in ring0 bail out of here if we are
   #ifndef DEBUG
   if (CheckForRing3() == 0)
      return;
   #endif

   Parm.VoidPtr=(VOID far *) &DbgStr;
   Parm.StringPtr++;                            // skip size of string pointer

   while (*pStr) {
      // don't overflow target
      if (BuildPtr >= (char far *) &dd_BuildString[1024-2])
         break;

      switch (*pStr) {  // the top switch
         case '%':
            wBuildOption=0;
            pStr++;
            if (*pStr=='0') {
               wBuildOption|=LEADING_ZEROES;
               pStr++;
            }
            if (*pStr=='u')                                                         // always unsigned
               pStr++;

            switch(*pStr) { // switch on "%" options
               case 'x':
                  BuildPtr=ddprintf_HexWordToASCII(BuildPtr, *Parm.WordPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 'd':
                  BuildPtr=ddprintf_DecWordToASCII(BuildPtr, *Parm.WordPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 's':
                  SubStr=(char *)*Parm.StringPtr;
                  while (*BuildPtr++ = *SubStr++);
                  Parm.StringPtr++;
                  BuildPtr--;                      // remove the \0
                  pStr++;
                  continue;

               case 'l':
                  pStr++;
                  switch (*pStr) { // "%l" options
                     case 'x':
                        BuildPtr=ddprintf_HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                        pStr++;
                        continue;

                     case 'd':
                        BuildPtr=ddprintf_DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                        pStr++;
                        continue;
                  } // end switch on "%l" options
                  continue;                        // dunno what he wants

               case 0:
                  continue;
            } // end switch on "%" options
            break; // case 'x'

         case '\\':
            pStr++;
            switch (*pStr) {
               case 'n':
               *BuildPtr++=CR;
               *BuildPtr++=LF;
               pStr++;
               continue;

               case 'r':
               *BuildPtr++=CR;
               pStr++;
               continue;

               case 0:
               continue;
               break;
            } // end switch

            break; // case '\\'

         #ifndef DEBUG
         case '\n':
            pStr++;
            *BuildPtr++=CR;
            *BuildPtr++=LF;
            continue;
            break;
         #endif
      } // end top switch

      *BuildPtr++=*pStr++;
   } // end while

   *BuildPtr=0;                                 // cauterize the string
   ddputstring((char far *) dd_BuildString);    // display
}

                                        //----------------------- ddputstring -
VOID ddputstring (char far *St)
{
#ifdef DEBUG
   while (*St)
      CharOut(*St++);
#else

   int      iMsgLength;
   char far *TempSt;
   TempSt = St;
   iMsgLength = 0;
   while (*TempSt != '\0') {
     ++TempSt;
     ++iMsgLength;
   }               // Should strcat a \r\n to the string.
   DosPutMessage (1, iMsgLength, St);         // For ring-3 initialization

#endif
}
                                        //--------------------------- CharOut -
void CharOut(char c)
{
   UCHAR inbyte;

        inbyte = inp((terminal_port + UART_LINE_STAT));
        while ((inbyte & 0x20) != 0x20) {
           iodelay(1);
           inbyte = inp((terminal_port + UART_LINE_STAT));
        }
        // Send the character
        outp((terminal_port + UART_DATA), c);

}
