/*
** Module   :FIREERROR.H
** Abstract : error codes
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  17/06/2004 Created
**
*/
#ifndef __FIREERROR_H
#define __FIREERROR_H



#define FIRE_NO_ERROR 0
// -EBUSY      node is busy, try again
#define FIRE_ERROR_BUSY 1
// -EAGAIN     error which can probably resolved by retry
#define FIRE_ERROR_AGAIN 2
// -EREMOTEIO  node suffers from an internal error
#define FIRE_ERROR_REMOTEIO 3
// -EACCES     this transaction is not allowed on requested address
#define FIRE_ERROR_ACCESS 4
// -EINVAL     invalid address at node
#define FIRE_ERROR_INVALID 5
// -ENOMEM
#define FIRE_ERROR_NOMEMORY 6

#endif  /*__FIREERROR_H*/

