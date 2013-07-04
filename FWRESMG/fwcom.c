#define __MULTI__

#define INCL_WIN
#define INCL_DOSDEVICES

/**************/
/* Headerfile */
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fwcom.h"
#include "fwresmg.h"
#include "global.h"
#include "../nodemgr/ioctl.h"

#define OPEN_FLAG ( OPEN_ACTION_OPEN_IF_EXISTS )
#define OPEN_MODE ( OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE )

HFILE hFWMGR;

#define CallPDD2(function, parm) DoIOCtl(function, &parm, sizeof(parm))

ULONG DoIOCtl(ULONG ulFunction, PVOID pvParm, ULONG ulParmLen)
{
   ULONG ulParmInOut = 0;

   return DosDevIOCtl(hFWMGR, NODEMGR_IOCTL_CATEGORY, ulFunction, pvParm, ulParmLen, &ulParmLen, NULL, 0, &ulParmInOut);
}

char *find_oui_name(int oui)
{
	extern struct oui_list_struct {
		int oui;
		char *name;
	} oui_list[];
	int i;

	for (i = 0; oui_list[i].name; i++)
		if (oui_list[i].oui == oui)
			return oui_list[i].name;
	return NULL;
}

void doFWioctl(void *arg) {
	ULONG ulAction;
	PCHAR temptext;
	APIRET rc;
	int i,j,k;

	NODEMGR_GET_VERSION version;
	NODEMGR_GET_HOSTNUM hostnum;
	NODEMGR_GET_HOST host;
	NODEMGR_GET_NODENUM nodenum;
	NODEMGR_GET_NODE node;
	NODEMGR_GET_UNITDIRNUM unitnum;
	NODEMGR_GET_UNITDIR unit;

	drivertext = (PCHAR)calloc(8000, 1);
	temptext = (PCHAR)calloc(2000, 1);
	sprintf(drivertext, "");
	sprintf(temptext, "");

	rc = DosOpen((PSZ) "FIREMGR$", &hFWMGR, &ulAction, 0, 0, OPEN_FLAG, OPEN_MODE, NULL);
	if (rc) {
		sprintf(temptext,"DosOpen failed with RC = %lu\n", rc);
		strcat(drivertext,temptext);
		WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
		return;
	}

	rc = CallPDD2(NODEMGR_IOCTL_GET_VERSION, version);
	if (rc) {
		sprintf(temptext,"FIREMGR_GET_VERSION failed with RC = %lu\n", rc);
		strcat(drivertext,temptext);
		WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
		return;
	}
	sprintf(temptext,"Firewire Driver installed!\n"); strcat(drivertext,temptext);
	sprintf(temptext,"  MajorVersion = %lu\n", version.usMajorVersion); strcat(drivertext,temptext);
	sprintf(temptext,"  MinorVersion = %lu\n", version.usMinorVersion); strcat(drivertext,temptext);
	sprintf(temptext,"  BuildLevel   = %lu\n", version.usBuildLevel); strcat(drivertext,temptext);
	sprintf(temptext,"  Date         = %lu/%lu/%lu\n", version.usYear,version.bMonth,version.bDay); strcat(drivertext,temptext);

	rc = CallPDD2(NODEMGR_IOCTL_GET_HOSTNUM, hostnum);
	if (rc) {
		sprintf(temptext,"NODEMGR_IOCTL_GET_HOSTNUM failed with RC = %lu\n", rc);
		strcat(drivertext,temptext);
		WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
		return;
	}
	sprintf(temptext," \n"); strcat(drivertext,temptext);
	if (hostnum.ucHostNum == 0) {
		sprintf(temptext,"No OHCI compatible controller found!\n"); strcat(drivertext,temptext);
		WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
		return;
	}
	if (hostnum.ucHostNum == 1) {
		sprintf(temptext,"%d OHCI compatible controller found!\n", hostnum.ucHostNum); strcat(drivertext,temptext);
	} else {
		sprintf(temptext,"%d OHCI compatible controllers found!\n", hostnum.ucHostNum); strcat(drivertext,temptext);
	}

	sprintf(temptext," \n"); strcat(drivertext,temptext);
	for (i=0; i<hostnum.ucHostNum;i++) {
		host.ucHostNum=i;
		sprintf(temptext,"Controller: %d\n",i); strcat(drivertext,temptext);
		rc = CallPDD2(NODEMGR_IOCTL_GET_HOST, host);
		if (rc) {
			sprintf(temptext,"NODEMGR_IOCTL_GET_HOST failed with RC = %lu\n", rc); strcat(drivertext,temptext);
			WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
			return;
		}
		sprintf(temptext,"   Total Nodes:  %d\n",host.usNodeCount); strcat(drivertext,temptext);
		sprintf(temptext,"   Active Nodes: %d\n",host.usNodesActive); strcat(drivertext,temptext);
		sprintf(temptext,"   SelfID Count: %d\n",host.usSelfIdCount); strcat(drivertext,temptext);
		sprintf(temptext,"   NodeID of Controller: 0x%x\n",host.usNodeId); strcat(drivertext,temptext);
		sprintf(temptext,"   IRM Host ID:          0x%x\n",host.usIRMId); strcat(drivertext,temptext);
		sprintf(temptext,"   Bus Manager Host ID:  0x%x\n",host.usBusMgrId); strcat(drivertext,temptext);
		sprintf(temptext,"   Bus State:\n"); strcat(drivertext,temptext);
		sprintf(temptext,"     Reset:     %d\n",host.bInBusReset); strcat(drivertext,temptext);
		sprintf(temptext,"     Shut Down: %d\n",host.bIsShutdown); strcat(drivertext,temptext);
		sprintf(temptext,"   Controller State:\n"); strcat(drivertext,temptext);
		sprintf(temptext,"     Root:         %d\n",host.bIsRoot); strcat(drivertext,temptext);
		sprintf(temptext,"     Cycle Master: %d\n",host.bIsCycmst); strcat(drivertext,temptext);
		sprintf(temptext,"     IR Master:    %d\n",host.bIsIrm); strcat(drivertext,temptext);
		sprintf(temptext,"     Bus Manager:  %d\n",host.bIsBusmgr); strcat(drivertext,temptext);
		nodenum.ucHostNum=i;
		rc = CallPDD2(NODEMGR_IOCTL_GET_NODENUM, nodenum);
		if (rc) {
			sprintf(temptext,"NODEMGR_IOCTL_GET_NODENUM failed with RC = %lu\n", rc); strcat(drivertext,temptext);
			WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
			return;
		}
		sprintf(temptext,"   %d nodes attached to this controller\n",nodenum.ucNodeNum); strcat(drivertext,temptext);
		for (j=0;j<nodenum.ucNodeNum;j++) {
			node.ucHostNum=i;
			node.ucNodeNum=j;
			rc = CallPDD2(NODEMGR_IOCTL_GET_NODE, node);
			if (rc) {
				sprintf(temptext,"NODEMGR_IOCTL_GET_NODE failed with RC = %lu\n", rc); strcat(drivertext,temptext);
				WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
				return;
			}
			sprintf(temptext,"   Node: %d\n",j); strcat(drivertext,temptext);
			sprintf(temptext,"      GUID:      0x%0lx%0lx\n",node.ulGUIDHi, node.ulGUIDLo); strcat(drivertext,temptext);
			sprintf(temptext,"      Vendor ID: 0x%lx\n",node.ulGuidVendorId); strcat(drivertext,temptext);
			sprintf(temptext,"      Node ID:   0x%x\n",node.usNodeId); strcat(drivertext,temptext);
			sprintf(temptext,"      This note is currently: "); strcat(drivertext,temptext);
			if (node.bInLimbo)
				sprintf(temptext,"detached\n");
			else
				sprintf(temptext,"active\n");
			strcat(drivertext,temptext);
			sprintf(temptext,"      Bus Options:\n"); strcat(drivertext,temptext);
			sprintf(temptext,"        IRMC:   %d\n",node.BusOptions.ucIrmc); strcat(drivertext,temptext);
			sprintf(temptext,"        CMD:    %d\n",node.BusOptions.ucCmc); strcat(drivertext,temptext);
			sprintf(temptext,"        ISC:    %d\n",node.BusOptions.ucIsc); strcat(drivertext,temptext);
			sprintf(temptext,"        BMC:    %d\n",node.BusOptions.ucBmc); strcat(drivertext,temptext);
			sprintf(temptext,"        PMC:    %d\n",node.BusOptions.ucPmc); strcat(drivertext,temptext);
			sprintf(temptext,"        Cycle Clock acc:  %d\n",node.BusOptions.ucCycClkAcc); strcat(drivertext,temptext);
			sprintf(temptext,"        MaxRom: %d\n",node.BusOptions.ucMaxRom); strcat(drivertext,temptext);
			sprintf(temptext,"        Generation: %d\n",node.BusOptions.ucGeneration); strcat(drivertext,temptext);
			sprintf(temptext,"        Link Speed: %d\n",node.BusOptions.ucLinkSpeed); strcat(drivertext,temptext);
			sprintf(temptext,"        Max Rec:    %d\n",node.BusOptions.usMaxRec); strcat(drivertext,temptext);
			sprintf(temptext,"      Needs Probe: %d\n",node.bNeedProbe); strcat(drivertext,temptext);
			sprintf(temptext,"      Generation:  %d\n",node.usGeneration); strcat(drivertext,temptext);
			sprintf(temptext,"      Vendor ID: 0x%lx\n",node.usGeneration); strcat(drivertext,temptext);
			sprintf(temptext,"      Capabilities: 0x%lx\n",node.ulCapabilities); strcat(drivertext,temptext);
			sprintf(temptext,"      Vendor Name from Node: \"%s\"\n",node.strVendorName); strcat(drivertext,temptext);
			sprintf(temptext,"      Vendor Name by GUID: \"%s\"\n",find_oui_name(node.ulGuidVendorId)); strcat(drivertext,temptext);
			unitnum.ucHostNum=i;
			unitnum.ucNodeNum=j;
			rc = CallPDD2(NODEMGR_IOCTL_GET_UNITDIRNUM, unitnum);
			if (rc) {
				sprintf(temptext,"NODEMGR_IOCTL_GET_UNITDIRNUM failed with RC = %lu\n", rc); strcat(drivertext,temptext);
				WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
				return;
			}
			sprintf(temptext,"      Unit Directories: %d\n",unitnum.ucUnitDirNum); strcat(drivertext,temptext);
			for (k=0;k<unitnum.ucUnitDirNum;k++) {
				unit.ucHostNum=i;
				unit.ucNodeNum=j;
				unit.ucUnitDirNum=k;
				sprintf(temptext,"         Directory: %d\n",k); strcat(drivertext,temptext);
				rc = CallPDD2(NODEMGR_IOCTL_GET_UNITDIR, unit);
				if (rc) {
					sprintf(temptext,"NODEMGR_IOCTL_GET_UNITDIR failed with RC = %lu\n", rc); strcat(drivertext,temptext);
					WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
					return;
				}
				sprintf(temptext,"           Address:   0x%lx%lx\n",unit.ulAddrHi, unit.ulAddrLo); strcat(drivertext,temptext);
				sprintf(temptext,"           Flags:     0x%lx\n",unit.ucFlags); strcat(drivertext,temptext);
				sprintf(temptext,"           Vendor ID: 0x%lx\n",unit.ulVendorId); strcat(drivertext,temptext);
				sprintf(temptext,"           Model ID:  0x%lx\n",unit.ulModelId); strcat(drivertext,temptext);
				sprintf(temptext,"           Specifier ID:  0x%lx\n",unit.ulSpecifierId); strcat(drivertext,temptext);
				sprintf(temptext,"           Version:  0x%lx\n",unit.ulVersion); strcat(drivertext,temptext);
				sprintf(temptext,"           ID:       0x%x\n",unit.usId); strcat(drivertext,temptext);
				sprintf(temptext,"           Vendor Name from Unit: \"%s\"\n",unit.strVendorName); strcat(drivertext,temptext);
				sprintf(temptext,"           Model Name from Unit:  \"%s\"\n",unit.strModelName); strcat(drivertext,temptext);
			}
		}
	}

	DosClose(hFWMGR);

	WinPostMsg((HWND) arg, WM_UPDATEDRIVERTEXT, (MPARAM)NULL, (MPARAM)NULL);
	return;
}
