/*
** Module   :HOTPLUG.H
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Tue  15/06/2004 Created
**
*/
#ifndef __HOTPLUG_H
#define __HOTPLUG_H

/* Unit spec id and sw version entry for some protocols */
#define AVC_UNIT_SPEC_ID_ENTRY		0x0000A02D
#define AVC_SW_VERSION_ENTRY		0x00010001
#define CAMERA_UNIT_SPEC_ID_ENTRY	0x0000A02D
#define CAMERA_SW_VERSION_ENTRY		0x00000100

#define IEEE1394_MATCH_VENDOR_ID	0x0001
#define IEEE1394_MATCH_MODEL_ID		0x0002
#define IEEE1394_MATCH_SPECIFIER_ID	0x0004
#define IEEE1394_MATCH_VERSION		0x0008

typedef struct
{
    ULONG match_flags;
    ULONG vendor_id;
    ULONG model_id;
    ULONG specifier_id;
    ULONG version;
//    void *driver_data;
}IEEE1394_DEVICE_ID;

#endif

#endif /* _IEEE1394_HOTPLUG_H */
#endif  /*__HOTPLUG_H*/

