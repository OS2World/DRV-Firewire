/*
** Module   :PARAMETERS.CPP
** Abstract :SBP2 driver configurable parameters
**
** Copyright (C) Alex Cherkaev
**
** Log: Tue  01/02/2005	Created
**
*/


/*
 * Module load parameter definitions
 */

/*
 * Change max_speed on module load if you have a bad IEEE-1394
 * controller that has trouble running 2KB packets at 400mb.
 *
 * NOTE: On certain OHCI parts I have seen short packets on async transmit
 * (probably due to PCI latency/throughput issues with the part). You can
 * bump down the speed if you are running into problems.
 */
int ParamsMaxSpeed = IEEE1394_SPEED_MAX;
// module_param(max_speed, int, 0644);
// MODULE_PARM_DESC(max_speed, "Force max speed (3 = 800mb, 2 = 400mb default, 1 = 200mb, 0 = 100mb)");

/*
 * Set serialize_io to 1 if you'd like only one scsi command sent
 * down to us at a time (debugging). This might be necessary for very
 * badly behaved sbp2 devices.
 */
int ParamsSerializeIO = 0;
// module_param(serialize_io, int, 0444);
//MODULE_PARM_DESC(serialize_io, "Serialize all I/O coming down from the scsi drivers (default = 0)");

/*
 * Bump up max_sectors if you'd like to support very large sized
 * transfers. Please note that some older sbp2 bridge chips are broken for
 * transfers greater or equal to 128KB.  Default is a value of 255
 * sectors, or just under 128KB (at 512 byte sector size). I can note that
 * the Oxsemi sbp2 chipsets have no problems supporting very large
 * transfer sizes.
 */
int ParamsMaxSectors = SBP2_MAX_SECTORS;
//module_param(max_sectors, int, 0444);
//MODULE_PARM_DESC(max_sectors, "Change max sectors per I/O supported (default = 255)");

/*
 * Exclusive login to sbp2 device? In most cases, the sbp2 driver should
 * do an exclusive login, as it's generally unsafe to have two hosts
 * talking to a single sbp2 device at the same time (filesystem coherency,
 * etc.). If you're running an sbp2 device that supports multiple logins,
 * and you're either running read-only filesystems or some sort of special
 * filesystem supporting multiple hosts (one such filesystem is OpenGFS,
 * see opengfs.sourceforge.net for more info), then set exclusive_login
 * to zero. Note: The Oxsemi OXFW911 sbp2 chipset supports up to four
 * concurrent logins.
 */
int ParamsExclusiveLogin = 1;
//module_param(exclusive_login, int, 0644);
//MODULE_PARM_DESC(exclusive_login, "Exclusive login to sbp2 device (default = 1)");

/*
 * SCSI inquiry hack for really badly behaved sbp2 devices. Turn this on
 * if your sbp2 device is not properly handling the SCSI inquiry command.
 * This hack makes the inquiry look more like a typical MS Windows
 * inquiry.
 *
 * If force_inquiry_hack=1 is required for your device to work,
 * please submit the logged sbp2_firmware_revision value of this device to
 * the linux1394-devel mailing list.
 */
int ParamsForceInquiryHack = 0;
//module_param(force_inquiry_hack, int, 0444);
//MODULE_PARM_DESC(force_inquiry_hack, "Force SCSI inquiry hack (default = 0)");

