/* *********************************************************************
 * Copyright (C) 2007-2015 VMware, Inc. All Rights Reserved. -- VMware Confidential
 * *********************************************************************/

/* This demonstrates how to open a virtual machine,
 * power it on, and power it off.
 *
 * This uses the VixJob_Wait function to block after starting each
 * asynchronous function. This effectively makes the asynchronous
 * functions synchronous, because VixJob_Wait will not return until the
 * asynchronous function has completed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "vix.h"


/*
 * Certain arguments differ when using VIX with VMware Server 2.0
 * and VMware Workstation.
 *
 * Comment out this definition to use this code with VMware Server 2.0.
 */
#define USE_WORKSTATION

#ifdef USE_WORKSTATION

#define  CONNTYPE    VIX_SERVICEPROVIDER_VMWARE_WORKSTATION

#define  HOSTNAME ""
#define  HOSTPORT 0
#define  USERNAME ""
#define  PASSWORD ""

#define  VMPOWEROPTIONS   VIX_VMPOWEROP_LAUNCH_GUI   // Launches the VMware Workstaion UI
                                                     // when powering on the virtual machine.

#define VMXPATH_INFO "where vmxpath is an absolute path to the .vmx file " \
                     "for the virtual machine."

#else    // USE_WORKSTATION

/*
 * For VMware Server 2.0
 */

#define CONNTYPE VIX_SERVICEPROVIDER_VMWARE_VI_SERVER

#define HOSTNAME "https://192.2.3.4:8333/sdk"
/*
 * NOTE: HOSTPORT is ignored, so the port should be specified as part
 * of the URL.
 */
#define HOSTPORT 0
#define USERNAME "root"
#define PASSWORD "hideme"

#define  VMPOWEROPTIONS VIX_VMPOWEROP_NORMAL

#define VMXPATH_INFO "where vmxpath is a datastore-relative path to the " \
                     ".vmx file for the virtual machine, such as "        \
                     "\"[standard] ubuntu/ubuntu.vmx\"."

#endif    // USE_WORKSTATION


/*
 * Global variables.
 */

static char *progName;


/*
 * Local functions.
 */

////////////////////////////////////////////////////////////////////////////////
static void
usage()
{
   fprintf(stderr, "\nUsage: <program> <COMMAND> <vmx-file> [<cl-vmx-file>] COMMAND can be START, STOP, SUSPEND, DELETE, FCLONE, or LCLONE. %s", progName);
   fprintf(stderr, "%s\n", VMXPATH_INFO);
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    VixError err;
    char *vmxPath;
	char *vmxClone = "";
    VixHandle hostHandle = VIX_INVALID_HANDLE;
    VixHandle jobHandle = VIX_INVALID_HANDLE;
    VixHandle vmHandle = VIX_INVALID_HANDLE;
	VixHandle cloneVMHandle = VIX_INVALID_HANDLE;

    progName = argv[0];
	char *todo = "";

    if (argc == 3) {
        vmxPath = argv[1];
		/*if (argc > 3) {
			todo = argv[3];
			int i = 0;
			while (argv[3][i]) {
				todo[i] = toupper(argv[3][i]);
				i++;
			}
			printf(todo);
		}*/
		todo = argv[2];
		int i = 0;
		while (argv[2][i]) {
			todo[i] = toupper(argv[2][i]);
			i++;
		}
		printf(todo);
	}
	else if (argc == 4) {
		vmxPath = argv[1];
		vmxClone = argv[2];
		todo = argv[3];
		int i = 0;
		while (argv[3][i]) {
			todo[i] = toupper(argv[3][i]);
			i++;
		}
		printf(todo);
	}
	else {
        usage();
        exit(EXIT_FAILURE);
    }

    jobHandle = VixHost_Connect(VIX_API_VERSION,
                                CONNTYPE,
                                HOSTNAME, // *hostName,
                                HOSTPORT, // hostPort,
                                USERNAME, // *userName,
                                PASSWORD, // *password,
                                0, // options,
                                VIX_INVALID_HANDLE, // propertyListHandle,
                                NULL, // *callbackProc,
                                NULL); // *clientData);
    err = VixJob_Wait(jobHandle, 
                      VIX_PROPERTY_JOB_RESULT_HANDLE, 
                      &hostHandle,
                      VIX_PROPERTY_NONE);
    if (VIX_FAILED(err)) {
        goto abort;
    }

    Vix_ReleaseHandle(jobHandle);
    jobHandle = VixVM_Open(hostHandle,
                           vmxPath,
                           NULL, // VixEventProc *callbackProc,
                           NULL); // void *clientData);
    err = VixJob_Wait(jobHandle, 
                      VIX_PROPERTY_JOB_RESULT_HANDLE, 
                      &vmHandle,
                      VIX_PROPERTY_NONE);
    if (VIX_FAILED(err)) {
        goto abort;
    }
	
	if (strcmp(todo, "ON") == 0) {
		Vix_ReleaseHandle(jobHandle);
		jobHandle = VixVM_PowerOn(vmHandle,
									VMPOWEROPTIONS,
									VIX_INVALID_HANDLE,
									NULL, // *callbackProc,
									NULL); // *clientData);
		err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
		if (VIX_FAILED(err)) {
			goto abort;
		}
	}
	else if (strcmp(todo, "OFF") == 0) {
		Vix_ReleaseHandle(jobHandle);
		jobHandle = VixVM_PowerOff(vmHandle,
									VIX_VMPOWEROP_NORMAL,
									NULL, // *callbackProc,
									NULL); // *clientData);
		err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
		if (VIX_FAILED(err)) {
			goto abort;
		}
	}
	else if (strcmp(todo, "SUSPEND") == 0) {
		Vix_ReleaseHandle(jobHandle);
		jobHandle = VixVM_Suspend(vmHandle,
									VIX_VMPOWEROP_NORMAL,
									NULL, // *callbackProc,
									NULL); // *clientData);
		err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
		if (VIX_FAILED(err)) {
			goto abort;
		}
	}
	else if (strcmp(todo, "DELETE") == 0) {
		Vix_ReleaseHandle(jobHandle);
		jobHandle = VixVM_Delete(
			vmHandle, 
			VIX_VMDELETE_DISK_FILES, 
			NULL, 
			NULL
		);
		err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
		if (VIX_OK != err) {
			// Handle the error...
			goto abort;
		}
		Vix_ReleaseHandle(jobHandle);
	}
	else if (strcmp(todo, "FCLONE") == 0) {
		Vix_ReleaseHandle(jobHandle);
		printf("\nF-CLONING!\n");
		jobHandle = VixVM_Clone(vmHandle,
			VIX_INVALID_HANDLE,  // snapshotHandle
			VIX_CLONETYPE_FULL,  // cloneType
			vmxClone,  // destConfigPathName
			0,  //options,
			VIX_INVALID_HANDLE,  // propertyListHandle
			NULL,  // callbackProc
			NULL);  // clientData
		err = VixJob_Wait(jobHandle,
			VIX_PROPERTY_JOB_RESULT_HANDLE,
			&cloneVMHandle,
			VIX_PROPERTY_NONE);
		if (VIX_FAILED(err)) {
			// Handle the error...
			goto abort;
		}
		Vix_ReleaseHandle(jobHandle);
		printf("F-CLONED!\n");

		// You can use the new virtual machine as you would any other virtual machine. 
		// For example, to power on the new clone:
		//jobHandle = VixVM_PowerOn(cloneVMHandle,
		//	VIX_VMPOWEROP_LAUNCH_GUI,
		//	VIX_INVALID_HANDLE,
		//	NULL,
		//	NULL);
		//err = VixJob_Wait(jobHandle,
		//	VIX_PROPERTY_NONE);
		//if (VIX_FAILED(err)) {
		//	// Handle the error...
		//	goto abort;
		//}
	}
	else if (strcmp(todo, "LCLONE") == 0) {
		Vix_ReleaseHandle(jobHandle);
		printf("L-CLONING!");
		// Create a clone of this virtual machine.
		jobHandle = VixVM_Clone(vmHandle,
			VIX_INVALID_HANDLE,  // snapshotHandle
			VIX_CLONETYPE_LINKED,  // cloneType
			vmxClone,  // destConfigPathName
			0,  //options,
			VIX_INVALID_HANDLE,  // propertyListHandle
			NULL,  // callbackProc
			NULL);  // clientData
		err = VixJob_Wait(jobHandle,
			VIX_PROPERTY_JOB_RESULT_HANDLE,
			&cloneVMHandle,
			VIX_PROPERTY_NONE);
		if (VIX_FAILED(err)) {
			// Handle the error...
			goto abort;
		}
		Vix_ReleaseHandle(jobHandle);
		printf("L-CLONED!");
		// You can use the new virtual machine as you would any other virtual machine. 
		// For example, to power on the new clone:
		//jobHandle = VixVM_PowerOn(cloneVMHandle,
		//	VIX_VMPOWEROP_LAUNCH_GUI,
		//	VIX_INVALID_HANDLE,
		//	NULL,
		//	NULL);
		//err = VixJob_Wait(jobHandle,
		//	VIX_PROPERTY_NONE);
		//if (VIX_FAILED(err)) {
		//	// Handle the error...
		//	goto abort;
		//}
	}
	else {
		usage();
	}

abort:
    //Vix_ReleaseHandle(jobHandle);
    //Vix_ReleaseHandle(vmHandle);
    //VixHost_Disconnect(hostHandle);
	Vix_ReleaseHandle(jobHandle);
	Vix_ReleaseHandle(vmHandle);
	Vix_ReleaseHandle(cloneVMHandle);
	VixHost_Disconnect(hostHandle);
    return 0;
}

