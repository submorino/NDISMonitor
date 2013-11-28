/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	passthru.h

Abstract:

	Ndis Intermediate Miniport driver sample. This is a passthru driver.

Author:

Environment:


Revision History:

 
--*/

#define __LBFO
#define MAX_BUNDLEID_LENGTH 50
#define TAG 'ImPa'
#define WAIT_INFINITE 0

//advance declaration
typedef struct _ADAPT ADAPT, *PADAPT;

extern
NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT			DriverObject,
	IN	PUNICODE_STRING			RegistryPath
	);

//
// Protocol proto-types
//
extern
VOID
PtOpenAdapterComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_STATUS				Status,
	IN	NDIS_STATUS				OpenErrorStatus
	);

extern
VOID
PtCloseAdapterComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_STATUS				Status
	);

extern
VOID
PtResetComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_STATUS				Status
	);

extern
VOID
PtRequestComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	NDIS_STATUS				Status
	);

extern
VOID
PtStatus(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_STATUS				GeneralStatus,
	IN	PVOID					StatusBuffer,
	IN	UINT					StatusBufferSize
	);

extern
VOID
PtStatusComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext
	);

extern
VOID
PtSendComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNDIS_PACKET			Packet,
	IN	NDIS_STATUS				Status
	);

extern
VOID
PtTransferDataComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNDIS_PACKET			Packet,
	IN	NDIS_STATUS				Status,
	IN	UINT					BytesTransferred
	);

extern
NDIS_STATUS
PtReceive(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_HANDLE				MacReceiveContext,
	IN	PVOID					HeaderBuffer,
	IN	UINT					HeaderBufferSize,
	IN	PVOID					LookAheadBuffer,
	IN	UINT					LookaheadBufferSize,
	IN	UINT					PacketSize
	);

extern
VOID
PtReceiveComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext
	);

extern
INT
PtReceivePacket(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNDIS_PACKET			Packet
	);

extern
VOID
PtBindAdapter(
	OUT	PNDIS_STATUS			Status,
	IN	NDIS_HANDLE				BindContext,
	IN	PNDIS_STRING			DeviceName,
	IN	PVOID					SystemSpecific1,
	IN	PVOID					SystemSpecific2
	);

extern
VOID
PtUnbindAdapter(
	OUT	PNDIS_STATUS			Status,
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_HANDLE				UnbindContext
	);
	
VOID
PtUnload(
	IN	PDRIVER_OBJECT			DriverObject
	);



extern 
NDIS_STATUS
PtPNPHandler(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNET_PNP_EVENT			pNetPnPEvent
	);




NDIS_STATUS
PtPnPNetEventReconfigure(
	IN	PADAPT			pAdapt,
	IN	PCWSTR			pBundleString
	);	

NDIS_STATUS 
PtPnPNetEventSetPower (
	IN PADAPT					pAdapt,
	IN	PNET_PNP_EVENT			pNetPnPEvent
	);
	

//
// Miniport proto-types
//
NDIS_STATUS
MPInitialize(
	OUT PNDIS_STATUS			OpenErrorStatus,
	OUT PUINT					SelectedMediumIndex,
	IN	PNDIS_MEDIUM			MediumArray,
	IN	UINT					MediumArraySize,
	IN	NDIS_HANDLE				MiniportAdapterHandle,
	IN	NDIS_HANDLE				WrapperConfigurationContext
	);

VOID
MPSendPackets(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PPNDIS_PACKET			PacketArray,
	IN	UINT					NumberOfPackets
	);

NDIS_STATUS
MPSend(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PNDIS_PACKET			Packet,
	IN	UINT					Flags
	);

NDIS_STATUS
MPQueryInformation(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_OID				Oid,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesWritten,
	OUT PULONG					BytesNeeded
	);

NDIS_STATUS
MPSetInformation(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_OID				Oid,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesRead,
	OUT PULONG					BytesNeeded
	);

VOID
MPReturnPacket(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PNDIS_PACKET			Packet
	);

NDIS_STATUS
MPTransferData(
	OUT PNDIS_PACKET			Packet,
	OUT PUINT					BytesTransferred,
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_HANDLE				MiniportReceiveContext,
	IN	UINT					ByteOffset,
	IN	UINT					BytesToTransfer
	);

VOID
MPHalt(
	IN	NDIS_HANDLE				MiniportAdapterContext
	);

NDIS_STATUS
MPReset(
	OUT PBOOLEAN				AddressingReset,
	IN	NDIS_HANDLE				MiniportAdapterContext
	);

VOID
MPQueryPNPCapbilities(  
	OUT	PADAPT					MiniportProtocolContext, 
	OUT	PNDIS_STATUS			Status
	);


NDIS_STATUS
MPSetMiniportSecondary ( 
	IN	PADAPT					Secondary, 
	IN	PADAPT					Primary
	);

BOOLEAN
MPIsSendOID (
	IN NDIS_OID					oid
	);


NDIS_STATUS 
MPPromoteSecondary ( 
	IN	PADAPT					pAdapt 
	);


NDIS_STATUS 
MPBundleSearchAndSetSecondary (
	IN	PADAPT					pAdapt 
	);

VOID
MPProcessSetPowerOid(
	IN OUT PNDIS_STATUS      pNdisStatus,
	IN  PADAPT					pAdapt,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesRead,
	OUT PULONG					BytesNeeded
    );



#define DBGPRINT(Fmt)										\
	{														\
		DbgPrint("*** %s (%d) *** ", __FILE__, __LINE__);	\
		DbgPrint (Fmt);										\
	}

#define	NUM_PKTS_IN_POOL	256


//
// Protocol reserved part of the packet
//
typedef struct _ProtRsvd
{
	PNDIS_PACKET	OriginalPkt;
} RSVD, *PRSVD;


//
// Event Codes related to the PassthruEvent Structure
//

typedef enum 
{
	Passthru_Invalid,
	Passthru_SetPower,
	Passthru_Unbind

} PASSSTHRU_EVENT_CODE, *PPASTHRU_EVENT_CODE; 

//
// Passthru Event with  a code to state why they have been state
//

typedef struct _PASSTHRU_EVENT
{
	NDIS_EVENT Event;
	PASSSTHRU_EVENT_CODE Code;

} PASSTHRU_EVENT, *PPASSTHRU_EVENT;


//
// Structure used by both the miniport as well as the protocol part of the intermediate driver
// to represent an adapter and its corres. lower bindings
//
typedef struct _ADAPT
{
	PADAPT						Next;
	
	NDIS_HANDLE					BindingHandle;	// To the lower miniport
	NDIS_HANDLE					MiniportHandle;	// NDIS Handle to for miniport up-calls
	NDIS_HANDLE					SendPacketPoolHandle;
	NDIS_HANDLE					RecvPacketPoolHandle;
	NDIS_STATUS					Status;			// Open Status
	NDIS_EVENT					Event;			// Used by bind/halt for Open/Close Adapter synch.
	NDIS_MEDIUM					Medium;
	NDIS_REQUEST				Request;		// This is used to wrap a request coming down
												// to us. This exploits the fact that requests
												// are serialized down to us.
	PULONG						BytesNeeded;
	PULONG						BytesReadOrWritten;
	BOOLEAN						IndicateRcvComplete;
	
	BOOLEAN						OutstandingRequests;  	//True - if a request has been passed to the miniport below the IM protocol 
	BOOLEAN						QueuedRequest;		    //True - if a request is queued with the IM miniport and needs to be either
														// failed or sent to the miniport below the IM Protocol

	BOOLEAN						StandingBy;				// True - When the miniport or protocol is transitioning from a D0 to Standby (>D0) State
														// False - At all other times, - Flag is cleared after a transition to D0

	NDIS_DEVICE_POWER_STATE 	MPDeviceState;			// Miniport's Device State 
	NDIS_DEVICE_POWER_STATE 	PTDeviceState;			// Protocol's Device State 

	BOOLEAN						isSecondary;			// Set if miniport is secondary of a bundle
	NDIS_STRING					BundleUniString;		// Strores the bundleid
	PADAPT						pPrimaryAdapt;			// Pointer to the primary
	PADAPT						pSecondaryAdapt;		// Pointer to Secondary's structure
	KSPIN_LOCK					SpinLock;				// Spin Lock to protect the global list

} ADAPT, *PADAPT;

extern	NDIS_PHYSICAL_ADDRESS			HighestAcceptableMax;
extern	NDIS_HANDLE						ProtHandle, DriverHandle;
extern	NDIS_MEDIUM						MediumArray[3];
extern	PADAPT							pAdaptList;

//
// Custom Macros to be used by the passthru driver 
//
/*
bool
IsIMDeviceStateOn(
   PADAPT 
   )

*/
#define IsIMDeviceStateOn(_pP)		((_pP)->MPDeviceState == NdisDeviceStateD0 && (_pP)->PTDeviceState == NdisDeviceStateD0 ) 

