/* $Id: iodelay.h,v 1.1 2004/06/21 16:29:58 doctor64 Exp $ */

/* IODELAY.H

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   01-Jul-95  Timur Tabi   Creation
   10-Jan-96  Timur Tabi   Added C++ support
*/

#ifdef __cplusplus
extern "C" {
#endif

void iodelay(unsigned short);
#pragma aux iodelay parm nomemory [cx] modify nomemory exact [ax cx];

#ifdef __cplusplus
}
#endif

