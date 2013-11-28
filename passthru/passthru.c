/*++

Copyright (c) 1992  Microsoft Corporation
 
Module Name:
 
	passthru.c

Abstract:

	Ndis Intermediate Miniport driver sample. This is a passthru driver.

Author:

Environment:


Revision History:


--*/


#include "precomp.h"
#pragma hdrstop

#pragma NDIS_INIT_FUNCTION(DriverEntry)

NDIS_PHYSICAL_ADDRESS			HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);
NDIS_HANDLE						ProtHandle = NULL;
NDIS_HANDLE						DriverHandle = NULL;
NDIS_MEDIUM						MediumArray[3] =
									{
										NdisMedium802_3,	// Ethernet
										NdisMedium802_5,	// Token-ring
										NdisMediumFddi		// Fddi
									};


PADAPT  pAdaptList=NULL;

NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT		DriverObject,
	IN	PUNICODE_STRING		RegistryPath
	)
/*++

Routine Description:


Arguments:

Return Value:


--*/
{
	NDIS_STATUS						Status;
	NDIS_PROTOCOL_CHARACTERISTICS	PChars;
	NDIS_MINIPORT_CHARACTERISTICS	MChars;
	PNDIS_CONFIGURATION_PARAMETER	Param;
	NDIS_STRING						Name;
	NDIS_HANDLE						WrapperHandle;

	//
	// Register the miniport with NDIS. Note that it is the miniport
	// which was started as a driver and not the protocol. Also the miniport
	// must be registered prior to the protocol since the protocol's BindAdapter
	// handler can be initiated anytime and when it is, it must be ready to
	// start driver instances.
	//
	NdisMInitializeWrapper(&WrapperHandle, DriverObject, RegistryPath, NULL);

	NdisZeroMemory(&MChars, sizeof(NDIS_MINIPORT_CHARACTERISTICS));

	MChars.MajorNdisVersion = 4;
	MChars.MinorNdisVersion = 0;

	MChars.InitializeHandler = MPInitialize;
	MChars.QueryInformationHandler = MPQueryInformation;
	MChars.SetInformationHandler = MPSetInformation;
	MChars.ResetHandler = MPReset;
	MChars.TransferDataHandler = MPTransferData;
	MChars.HaltHandler = MPHalt;

	//
	// We will disable the check for hang timeout so we do not
	// need a check for hang handler!
	//
	MChars.CheckForHangHandler = NULL;
	MChars.SendHandler = MPSend;
	MChars.ReturnPacketHandler = MPReturnPacket;

	//
	// Either the Send or the SendPackets handler should be specified.
	// If SendPackets handler is specified, SendHandler is ignored
	//
	// MChars.SendPacketsHandler = MPSendPackets;

	Status = NdisIMRegisterLayeredMiniport(WrapperHandle,
										   &MChars,
										   sizeof(MChars),
										   &DriverHandle);
	ASSERT(Status == NDIS_STATUS_SUCCESS);

	NdisMRegisterUnloadHandler(WrapperHandle, PtUnload);

	//
	// Now register the protocol.
	//
	NdisZeroMemory(&PChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
	PChars.MajorNdisVersion = 4;
	PChars.MinorNdisVersion = 0;

	//
	// Make sure the protocol-name matches the service-name under which this protocol is installed.
	// This is needed to ensure that NDIS can correctly determine the binding and call us to bind
	// to miniports below.
	//
	NdisInitUnicodeString(&Name, L"SFilter");	// Protocol name
	PChars.Name = Name;
	PChars.OpenAdapterCompleteHandler = PtOpenAdapterComplete;
	PChars.CloseAdapterCompleteHandler = PtCloseAdapterComplete;
	PChars.SendCompleteHandler = PtSendComplete;
	PChars.TransferDataCompleteHandler = PtTransferDataComplete;
	
	PChars.ResetCompleteHandler = PtResetComplete;
	PChars.RequestCompleteHandler = PtRequestComplete;
	PChars.ReceiveHandler = PtReceive;
	PChars.ReceiveCompleteHandler = PtReceiveComplete;
	PChars.StatusHandler = PtStatus;
	PChars.StatusCompleteHandler = PtStatusComplete;
	PChars.BindAdapterHandler = PtBindAdapter;
	PChars.UnbindAdapterHandler = PtUnbindAdapter;
	PChars.UnloadHandler = NULL;
	PChars.ReceivePacketHandler = PtReceivePacket;
	PChars.PnPEventHandler= PtPNPHandler;

	NdisRegisterProtocol(&Status,
						 &ProtHandle,
						 &PChars,
						 sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

	ASSERT(Status == NDIS_STATUS_SUCCESS);

	NdisIMAssociateMiniport(DriverHandle, ProtHandle);

	return(Status);
}


