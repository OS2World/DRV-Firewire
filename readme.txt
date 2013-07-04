   This is the first alpha release of the IEEE 1394a - Firewire for OS/2-eCS driver set!

Installation:
Copy files from \release directory to OS2\BOOT directory on your boot partition 
and add the following lines to the end of config.sys:

basedev=fired.sys 
basedev=firemgr.sys 
basedev=fireohci.sys

WARNING: Driver loading order is important!

Currently the only useful thing that you can do with the drivers is to look 
at a list of attached devices and some information about the devices.

If you have any problems, please report them to:

	doctor64@dochome.homedns.org

	
Please attach a complete description of the hardware and base OS, trap screen and 
debug output from debug drivers in the \debug directory.

Attention: To do this, you need a debug terminal and the kernel debugger.
