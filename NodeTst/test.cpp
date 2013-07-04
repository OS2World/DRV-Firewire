/* $Id: test.cpp,v 1.4 2004/08/03 13:10:42 doctor64 Exp $ */

/* TEST.C
*/
#pragma pack(1)
#define INCL_NOPMAPI
// #define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
// #define INCL_DOSDEVIOCTL
#define INCL_DOSPROCESS
#include <os2.h>
#include <stdio.h>
#include "../nodemgr/ioctl.h"

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


#define OPEN_FLAG ( OPEN_ACTION_OPEN_IF_EXISTS )
#define OPEN_MODE ( OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE )

HFILE hfile;
int fQuit = FALSE;

#define CallPDD(function) DoIOCtl(function, NULL, 0)

#define CallPDD2(function, parm) DoIOCtl(function, &parm, sizeof(parm))

ULONG DoIOCtl(ULONG ulFunction, PVOID pvParm, ULONG ulParmLen)
{
   ULONG ulParmInOut = 0;

   return DosDevIOCtl(hfile, NODEMGR_IOCTL_CATEGORY, ulFunction, pvParm, ulParmLen, &ulParmLen, NULL, 0, &ulParmInOut);
}

ULONG ulHandle;
int main(int argc, char **argv)
{
    APIRET rc;
    ULONG ulAction;
    int response;
    int i, j, k;

    NODEMGR_GET_VERSION version;
    NODEMGR_GET_HOSTNUM hostnum;
	NODEMGR_GET_HOST host;
    NODEMGR_GET_NODENUM nodenum;
    NODEMGR_GET_NODE node;
    NODEMGR_GET_UNITDIRNUM unitnum;
    NODEMGR_GET_UNITDIR unit;

   rc = DosOpen((PSZ) "FIREMGR$", &hfile, &ulAction, 0, 0, OPEN_FLAG, OPEN_MODE, NULL);
   if (rc) {
      printf("DosOpen failed with RC = %lu\n", rc);
      return 1;
   }

   rc = CallPDD2(NODEMGR_IOCTL_GET_VERSION, version);
   if (rc) {
      printf("FIREMGR_GET_VERSION failed with RC = %lu\n", rc);
      return 1;
   }

   rc = CallPDD2(NODEMGR_IOCTL_GET_HOSTNUM, hostnum);
   if (rc) {
      printf("NODEMGR_IOCTL_GET_HOSTNUM failed with RC = %lu\n", rc);
      return 1;
   }
   printf("Total %d hosts\n",hostnum.ucHostNum);
   	for (i=0; i<hostnum.ucHostNum;i++)
	{
		host.ucHostNum=i;
   		rc = CallPDD2(NODEMGR_IOCTL_GET_HOST, host);
		if (rc) {
      		printf("NODEMGR_IOCTL_GET_HOST failed with RC = %lu\n", rc);
      		return 1;
   		}
   		printf("Host %d total nodes %d SelfId count %d Active nodes %d \n",i,host.usNodeCount, host.usSelfIdCount, host.usNodesActive);
   		printf("Host NodeId %x IRMId %x BusManagerId %x \n", host.usNodeId, host.usIRMId, host.usBusMgrId);
		printf("State: in reset %d shutdowned %d \n",host.bInBusReset, host.bIsShutdown);
		printf("is root? %d is Cycle master? %d is IRM? %d is Bus Manager? %d\n",host.bIsRoot, host.bIsCycmst, host.bIsIrm, host.bIsBusmgr);
	    nodenum.ucHostNum=i;
	    rc = CallPDD2(NODEMGR_IOCTL_GET_NODENUM, nodenum);
   		if (rc) {
     		printf("NODEMGR_IOCTL_GET_NODENUM failed with RC = %lu\n", rc);
      		return 1;
   		}
   		printf("total %d nodes on host %d\n", nodenum.ucNodeNum, i);
   		for (j=0;j<nodenum.ucNodeNum;j++)
   		{
   		    node.ucHostNum=i;
   		    node.ucNodeNum=j;
	     	rc = CallPDD2(NODEMGR_IOCTL_GET_NODE, node);
			if (rc) {
      			printf("NODEMGR_IOCTL_GET_NODE failed with RC = %lu\n", rc);
      			return 1;
   			}
   			printf("Node %d GUID 0x%0lx%0lx GUID Vendor ID 0x%lx Node Id 0x%x\n",j,node.ulGUIDHi, node.ulGUIDLo, node.ulGuidVendorId, node.usNodeId);
   			if (node.bInLimbo) printf("Node currently removed\n");
   			    else printf("Node Active\n");
   			printf("Bus Options: irmc %d cmc %d isc %d bmc %d pmc %d cycle clock acc %d MaxRom %d \n generation %d LinkSpeed %d MaxRec %d\n",\
   			   node.BusOptions.ucIrmc, node.BusOptions.ucCmc, node.BusOptions.ucIsc,\
   			   node.BusOptions.ucBmc, node.BusOptions.ucPmc, node.BusOptions.ucCycClkAcc, node.BusOptions.ucMaxRom, node.BusOptions.ucGeneration, node.BusOptions.ucLinkSpeed, node.BusOptions.usMaxRec);
   			printf("node need probe %d generation %d Vendor Id 0x%lx Capabilities 0x%lx \n",\
   			   node.bNeedProbe, node.usGeneration, node.ulVendorId, node.ulCapabilities);
   			printf("Vendor Name by GUID %s \n",find_oui_name(node.ulGuidVendorId));
   			printf("readed Vendor Name %s \n",node.strVendorName);
   			unitnum.ucHostNum=i;
			unitnum.ucNodeNum=j;
			rc = CallPDD2(NODEMGR_IOCTL_GET_UNITDIRNUM, unitnum);
			if (rc) {
				printf("NODEMGR_IOCTL_GET_UNITDIRNUM failed with RC = %lu\n", rc);
      			return 1;
   			}
   			printf("Total %d unit directories on node\n", unitnum.ucUnitDirNum);
   			for (k=0;k<unitnum.ucUnitDirNum;k++)
   			{
   			    unit.ucHostNum=i;
	            unit.ucNodeNum=j;
	            unit.ucUnitDirNum=k;
	            rc = CallPDD2(NODEMGR_IOCTL_GET_UNITDIR, unit);
	   			if (rc) {
      				printf("NODEMGR_IOCTL_GET_UNITDIR failed with RC = %lu\n", rc);
      				return 1;
   				}
   				printf("Unit dir %d addr 0x%lx%lx flags 0x%x VendorId 0x%lx Model ID 0x%lx \n"\
					,k, unit.ulAddrHi, unit.ulAddrLo, unit.ucFlags, unit.ulVendorId, unit.ulModelId);
				printf("Specifier id 0x%lx Version 0x%lx id 0x%x \n", unit.ulSpecifierId, unit.ulVersion, unit.usId);
				printf("readed Vendor Name %s Model Name %s \n",unit.strVendorName, unit.strModelName);
			}
		}
	}
    DosClose(hfile);

    return 0;
}
