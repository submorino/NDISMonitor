/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	miniport.c

Abstract:

	Ndis Intermediate Miniport driver sample. This is a passthru driver.

Author:

Environment:


Revision History:


--*/

#include "precomp.h"
#pragma hdrstop


NDIS_STATUS
MPInitialize(
	OUT PNDIS_STATUS			OpenErrorStatus,
	OUT PUINT					SelectedMediumIndex,
	IN	PNDIS_MEDIUM			MediumArray,
	IN	UINT					MediumArraySize,
	IN	NDIS_HANDLE				MiniportAdapterHandle,
	IN	NDIS_HANDLE				WrapperConfigurationContext
	)
/*++

Routine Description:

	This is the initialize handler which gets called as a result of the BindAdapter handler
	calling NdisIMInitializeDeviceInstanceEx(). The context parameter which we pass there is
	the adapter structure which we retreive here. We also need to initialize the Power Management
	variable.
	LoadBalalncing- We keep a global list of all the passthru miniports installed and bundle
	two of them together if they have the same BundleId (read from registry)

	Arguments:

	OpenErrorStatus			Not used by us.
	SelectedMediumIndex		Place-holder for what media we are using
	MediumArray				Array of ndis media passed down to us to pick from
	MediumArraySize			Size of the array
	MiniportAdapterHandle	The handle NDIS uses to refer to us
	WrapperConfigurationContext	For use by NdisOpenConfiguration

Return Value:

	NDIS_STATUS_SUCCESS unless something goes wrong

--*/
{
	UINT	i;
	PADAPT	pAdapt;
	NDIS_STATUS						Status = NDIS_STATUS_FAILURE;


	NDIS_STATUS						BundleStatus = NDIS_STATUS_FAILURE;
	NDIS_STRING						BundleUniString;
	KIRQL							OldIrql;

	DBGPRINT("==>Passthru Initialize Miniport\n");

	//
	// Start off by retrieving the adapter context and storing the Miniport handle in it
	//
	pAdapt = NdisIMGetDeviceContext(MiniportAdapterHandle);
	pAdapt->MiniportHandle = MiniportAdapterHandle;

	//
	// Make sure the medium saved is one of the ones being offered
	//

	for (i = 0; i < MediumArraySize; i++)
	{
		if (MediumArray[i] == pAdapt->Medium)
		{
			*SelectedMediumIndex = i;
			break;
		}
	}

	if (i == MediumArraySize)
	{
		return(NDIS_STATUS_UNSUPPORTED_MEDIA);
	}


	//
	// Set the attributes now. The NDIS_ATTRIBUTE_DESERIALIZE is the key. This enables us
	// to make up-calls to NDIS w/o having to call NdisIMSwitchToMiniport/NdisIMQueueCallBack.
	// This also forces us to protect our data using spinlocks where appropriate. Also in this
	// case NDIS does not queue packets on out behalf. Since this is a very simple pass-thru
	// miniport, we do not have a need to protect anything. However in a general case there
	// will be a need to use per-adapter spin-locks for the packet queues at the very least.
	//
	NdisMSetAttributesEx(MiniportAdapterHandle,
						 pAdapt,
						 0,										// CheckForHangTimeInSeconds
						 NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT	|
							NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT|
							NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER |
							NDIS_ATTRIBUTE_DESERIALIZE |
							NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
						 0);

	//
	// Setting up the default value for the Device State Flag as PM capable
	// initialize the PM Variable, (for both miniport and the protocol) Device is ON by default
	//
	pAdapt->MPDeviceState=NdisDeviceStateD0;
	pAdapt->PTDeviceState=NdisDeviceStateD0;

	//
	// Begin the Load Balancing and Bundle Identifier work here
	// Default case: the miniport is the primary miniport
	//
	pAdapt->isSecondary		=	FALSE;	// default - primary
	pAdapt->pPrimaryAdapt	=	pAdapt;	//default, point to self
	pAdapt->pSecondaryAdapt	=	pAdapt;	//default, point to self

	//
	// Set miniport as a secondary miniport, if need be
	//
	BundleStatus  =	MPBundleSearchAndSetSecondary (pAdapt);

	//
	// Inserting into our global Passthru pAdapt List
	//
	KeAcquireSpinLock (&pAdapt->SpinLock, &OldIrql);

	pAdapt->Next = pAdaptList;
	pAdaptList = pAdapt;

	KeReleaseSpinLock (&pAdapt->SpinLock, OldIrql);
		
	//
	// We are done, In current implementation, Bundle Status does not affect the success of initialization
	//
	Status = NDIS_STATUS_SUCCESS;

	DBGPRINT("<== Passthru Initialize Miniport\n");

	return Status;
}


NDIS_STATUS
MPSend(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PNDIS_PACKET			Packet,
	IN	UINT					Flags
	)
/*++

Routine Description:

	Send handler. Just re-wrap the packet and send it below. Re-wrapping is necessary since
	NDIS uses the WrapperReserved for its own use.

	LBFO- All sends will be done in the secondary miniport of the bundle.

	We are using the Secondary Miniport as the Send path. All sends should use that pAdapt structure.

Arguments:

	MiniportAdapterContext	Pointer to the adapter
	Packet					Packet to send
	Flags					Unused, passed down below

Return Value:

	Return code from NdisSend

--*/
{
	PADAPT			pAdapt = (PADAPT)MiniportAdapterContext;
	NDIS_STATUS		Status;
	PNDIS_PACKET	MyPacket;
	PRSVD			Rsvd;
	PVOID			MediaSpecificInfo = NULL;
	ULONG			MediaSpecificInfoSize = 0;

	//
	//  According to our LBFO design, all sends will be performed on the secondary miniport
	//	However, the must be completed on the primary's miniport handle
	//

	ASSERT (pAdapt->pSecondaryAdapt);

	pAdapt = pAdapt->pSecondaryAdapt;


	if (IsIMDeviceStateOn (pAdapt) == FALSE)
	{
		return NDIS_STATUS_FAILURE;
	}


	NdisAllocatePacket(&Status,
					   &MyPacket,
					   pAdapt->SendPacketPoolHandle);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		PNDIS_PACKET_EXTENSION	Old, New;

		Rsvd = (PRSVD)(MyPacket->ProtocolReserved);
		Rsvd->OriginalPkt = Packet;
		MyPacket->Private.Flags = Flags;

		MyPacket->Private.Head = Packet->Private.Head;
		MyPacket->Private.Tail = Packet->Private.Tail;
		NdisSetPacketFlags(MyPacket, NDIS_FLAGS_DONT_LOOPBACK);

		//
		// Copy the OOB Offset from the original packet to the new
		// packet.
		//
		NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(MyPacket),
					   NDIS_OOB_DATA_FROM_PACKET(Packet),
					   sizeof(NDIS_PACKET_OOB_DATA));

		//
		// Copy the per packet info into the new packet
		// This includes ClassificationHandle, etc.
		// Make sure other stuff is not copied !!!
		//
		NdisIMCopySendPerPacketInfo(MyPacket, Packet);
		
		//
		// Copy the Media specific information
		//
		NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(Packet,
											&MediaSpecificInfo,
											&MediaSpecificInfoSize);

		if (MediaSpecificInfo || MediaSpecificInfoSize)
		{
			NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(MyPacket,
												MediaSpecificInfo,
												MediaSpecificInfoSize);
		}


		NdisSend(&Status,
				 pAdapt->BindingHandle,
				 MyPacket);


		if (Status != NDIS_STATUS_PENDING)
		{
			NdisIMCopySendCompletePerPacketInfo (Packet, MyPacket);
			NdisFreePacket(MyPacket);
		}
	}
	else
	{
		//
		// We are out of packets. Silently drop it. Alternatively we can deal with it:
		//	- By keeping separate send and receive pools
		//	- Dynamically allocate more pools as needed and free them when not needed
		//
	}

	return(Status);
}


VOID
MPSendPackets(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PPNDIS_PACKET			PacketArray,
	IN	UINT					NumberOfPackets
	)
/*++

Routine Description:

	Batched send-handler. TBD. Either this or the Send function can be specified but not both.
	LBFO - The Send will be done on the secondary miniport of the bundle

Arguments:

	MiniportAdapterContext	Pointer to our adapter
	PacketArray				Set of packets to send
	NumberOfPackets			Self-explanatory

Return Value:

	None

--*/
{
	PADAPT			pAdapt = (PADAPT)MiniportAdapterContext;
	NDIS_STATUS		Status;
	UINT			i;
	PVOID			MediaSpecificInfo = NULL;
	UINT			MediaSpecificInfoSize = 0;

	//
	//	Route all sends to the seondary, if no secondary exists, it will point to itself
	//
	pAdapt = pAdapt->pSecondaryAdapt;

	for (i = 0; i < NumberOfPackets; i++)
	{
		PRSVD			Rsvd;
		PNDIS_PACKET	Packet, MyPacket;

		Packet = PacketArray[i];

		if (IsIMDeviceStateOn(pAdapt) == FALSE)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		NdisAllocatePacket(&Status,
						   &MyPacket,
						   pAdapt->SendPacketPoolHandle);

		if (Status == NDIS_STATUS_SUCCESS)
		{
			PNDIS_PACKET_EXTENSION	Old, New;

			Rsvd = (PRSVD)(MyPacket->ProtocolReserved);
			Rsvd->OriginalPkt = Packet;

			MyPacket->Private.Flags = NdisGetPacketFlags(Packet);

			MyPacket->Private.Head = Packet->Private.Head;
			MyPacket->Private.Tail = Packet->Private.Tail;
			NdisSetPacketFlags(MyPacket, NDIS_FLAGS_DONT_LOOPBACK);

			//
			// Copy the OOB Offset from the original packet to the new
			// packet.
			//
			NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(MyPacket),
						   NDIS_OOB_DATA_FROM_PACKET(Packet),
						   sizeof(NDIS_PACKET_OOB_DATA));
			//
			// Copy the per packet info into the new packet
			// This includes ClassificationHandle, etc.
			// Make sure other stuff is not copied !!!
			//
			NdisIMCopySendPerPacketInfo(MyPacket, Packet);

			//
			// Copy the Media specific information
			//
			NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(Packet,
												&MediaSpecificInfo,
												&MediaSpecificInfoSize);

			if (MediaSpecificInfo || MediaSpecificInfoSize)
			{
				NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(MyPacket,
													MediaSpecificInfo,
													MediaSpecificInfoSize);
			}

			NdisSend(&Status,
					 pAdapt->BindingHandle,
					 MyPacket);

			if (Status != NDIS_STATUS_PENDING)
			{
				NdisIMCopySendCompletePerPacketInfo (Packet, MyPacket);
				NdisFreePacket(MyPacket);
			}
		}

		if (Status != NDIS_STATUS_PENDING)
		{
			//
			// We are out of packets. Silently drop it. Alternatively we can deal with it:
			//	- By keeping separate send and receive pools
			//	- Dynamically allocate more pools as needed and free them when not needed
			//
			NdisMSendComplete(pAdapt->pPrimaryAdapt->MiniportHandle,
							  Packet,
							  Status);

			// LBFO - Complete with the prmary's miniport handle
			// We should use the miniport handle that was used to call this function
		}
	}
}


NDIS_STATUS
MPQueryInformation(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_OID				Oid,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesWritten,
	OUT PULONG					BytesNeeded
		)
/*++

Routine Description:

	Miniport QueryInfo handler.
	In the Power Management scenario, OID_PNP_QUERY_POWER is not sent to the underlying miniport.
	OID_PNP_CAPABILITES is passed as a request to the miniport below.
	If the result is a success then the InformationBuffer is filled in the MPQueryPNPCapabilites.

	LBFO - For present all queries are passed on to the miniports that they were requested on.

	PM- If the MP is not ON (DeviceState > D0) return immediately  (except for query power and set power)
         If MP is ON, but the PT is not at D0, then queue the queue the request for later processing

	Requests to miniports are always serialized

Arguments:

	MiniportAdapterContext	Pointer to the adapter structure
	Oid						Oid for this query
	InformationBuffer		Buffer for information
	InformationBufferLength	Size of this buffer
	BytesWritten			Specifies how much info is written
	BytesNeeded				In case the buffer is smaller than what we need, tell them how much is needed


Return Value:

	Return code from the NdisRequest below.

--*/
{
	PADAPT	pAdapt = (PADAPT)MiniportAdapterContext;
	NDIS_STATUS	Status = NDIS_STATUS_FAILURE;

	do
	{
		//
		// Return Success for this OID
		//
		if (Oid == OID_PNP_QUERY_POWER)
		{
			Status=NDIS_STATUS_SUCCESS;
			break;
		}

		//
		// All other queries are failed, if the miniport is not at D0
		//
		if (pAdapt->MPDeviceState > NdisDeviceStateD0 || pAdapt->StandingBy == TRUE)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}
		//
		//	We are doing all sends on the secondary, so send all requests with Send OIDs to the Secondary
		//
		if (MPIsSendOID(Oid))
		{
			pAdapt = pAdapt->pSecondaryAdapt;
			//
			// Will point to itself, if there is no secondary (see initialization)
			//
		}

		pAdapt->Request.RequestType = NdisRequestQueryInformation;
		pAdapt->Request.DATA.QUERY_INFORMATION.Oid = Oid;
		pAdapt->Request.DATA.QUERY_INFORMATION.InformationBuffer = InformationBuffer;
		pAdapt->Request.DATA.QUERY_INFORMATION.InformationBufferLength = InformationBufferLength;
		pAdapt->BytesNeeded = BytesNeeded;
		pAdapt->BytesReadOrWritten = BytesWritten;
		pAdapt->OutstandingRequests = TRUE;

		//
		// if the Protocol device state is OFF, then the IM driver cannot send the request below and must pend it
		//
		if (pAdapt->PTDeviceState > NdisDeviceStateD0)
		{
			pAdapt->QueuedRequest = TRUE;
			Status = NDIS_STATUS_PENDING;
			break;
		}

		//
		// default case, most requests will be passed to the miniport below
		//
		NdisRequest(&Status,
					pAdapt->BindingHandle,
					&pAdapt->Request);


		//
		// If the Query was a success, pass the results back to the entity that made the request
		//
		if (Status == NDIS_STATUS_SUCCESS)
		{
			*BytesWritten = pAdapt->Request.DATA.QUERY_INFORMATION.BytesWritten;
			*BytesNeeded = pAdapt->Request.DATA.QUERY_INFORMATION.BytesNeeded;
		}

		//
		// Fill the buffer with the required values if Oid == OID_PNP_CAPABILITIES
		// and the query was successful
		//
		if (Oid  == OID_PNP_CAPABILITIES  && Status == NDIS_STATUS_SUCCESS)
		{
			MPQueryPNPCapbilities(pAdapt,&Status);
		}

		if (Status != NDIS_STATUS_PENDING)
		{
			pAdapt->OutstandingRequests = FALSE;
		}

	} while (FALSE);

	return(Status);

}


VOID
MPQueryPNPCapbilities(
	IN OUT	PADAPT			pAdapt,
	OUT		PNDIS_STATUS	pStatus
	)
/*++

Routine Description:

	Miniport QueryInfo OID_PNP_CAPAIBILITIES:
	If the Oid == Oid_PNP_CAPABILITIES, InformationBuffer is returned with all the fields
	assigned NdisDeviceStateUnspecified in the NDIS_PM_WAKE_UP_CAPABILITIES structure

	OID_QUERY_POWER_STATE is returned with NDIS_STATUS_SUCCESS and should never be passed below.

Arguments:

	MiniportAdapterContext	Pointer to the adapter structure
	Oid						Oid for this query
	InformationBuffer		Buffer for information
	InformationBufferLength	Size of this buffer
	BytesWritten			Specifies how much info is written
	BytesNeeded				In case the buffer is smaller than what we need, tell them how much is needed

Return Value:

	Return code from the NdisRequest below.

--*/

{
	PNDIS_PNP_CAPABILITIES			pPNPCapabilities;
	PNDIS_PM_WAKE_UP_CAPABILITIES	pPMstruct;

	if (pAdapt->Request.DATA.QUERY_INFORMATION.InformationBufferLength >= sizeof(NDIS_PNP_CAPABILITIES))
	{
		pPNPCapabilities = (PNDIS_PNP_CAPABILITIES)(pAdapt->Request.DATA.QUERY_INFORMATION.InformationBuffer);

		//
		// Setting up the buffer to be returned to the Protocol above the Passthru miniport
		//
		pPMstruct= & pPNPCapabilities->WakeUpCapabilities;
		pPMstruct->MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
		pPMstruct->MinPatternWakeUp = NdisDeviceStateUnspecified;
		pPMstruct->MinLinkChangeWakeUp = NdisDeviceStateUnspecified;
		*pAdapt->BytesReadOrWritten = sizeof(NDIS_PNP_CAPABILITIES);
		*pAdapt->BytesNeeded = 0;


		//
		// Setting our internal flags
		// Default, device is ON
		//
		pAdapt->MPDeviceState = NdisDeviceStateD0;
		pAdapt->PTDeviceState = NdisDeviceStateD0;

		*pStatus = NDIS_STATUS_SUCCESS;
	}
	else
	{
		*pAdapt->BytesNeeded= sizeof(NDIS_PNP_CAPABILITIES);
		*pStatus = NDIS_STATUS_RESOURCES;
	}
}


NDIS_STATUS
MPSetInformation(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_OID				Oid,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesRead,
	OUT PULONG					BytesNeeded
	)
/*++

Routine Description:

	Miniport SetInfo handler.

	In the case of OID_PNP_SET_POWER, record the power state and return the OID.	
	Do not pass below
	If the device is suspended, do not block the SET_POWER_OID 
	as it is used to reactivate the Passthru miniport

	
	PM- If the MP is not ON (DeviceState > D0) return immediately  (except for 'query power' and 'set power')
         If MP is ON, but the PT is not at D0, then queue the queue the request for later processing

	Requests to miniports are always serialized


Arguments:

	MiniportAdapterContext	Pointer to the adapter structure
	Oid						Oid for this query
	InformationBuffer		Buffer for information
	InformationBufferLength	Size of this buffer
	BytesRead				Specifies how much info is read
	BytesNeeded				In case the buffer is smaller than what we need, tell them how much is needed

Return Value:

	Return code from the NdisRequest below.

--*/
{
	PADAPT		pAdapt = (PADAPT)MiniportAdapterContext;
	NDIS_STATUS	Status;

	Status = NDIS_STATUS_FAILURE;

	do
	{
		//
		// The Set Power should not be sent to the miniport below the Passthru, but is handled internally
		//
		if (Oid == OID_PNP_SET_POWER)
		{
			MPProcessSetPowerOid( &Status, 
			                       pAdapt, 
			                       InformationBuffer, 
			                       InformationBufferLength, 
			                       BytesRead, 
			                       BytesNeeded);
			break;

		}

		//
		// All other Set Information requests are failed, if the miniport is not at D0 or is transitioning to
		// a device state greater than D0
		//
		if (pAdapt->MPDeviceState > NdisDeviceStateD0 || pAdapt->StandingBy == TRUE)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}


		// Set up the Request and return the result
		pAdapt->Request.RequestType = NdisRequestSetInformation;
		pAdapt->Request.DATA.SET_INFORMATION.Oid = Oid;
		pAdapt->Request.DATA.SET_INFORMATION.InformationBuffer = InformationBuffer;
		pAdapt->Request.DATA.SET_INFORMATION.InformationBufferLength = InformationBufferLength;
		pAdapt->BytesNeeded = BytesNeeded;
		pAdapt->BytesReadOrWritten = BytesRead;
		pAdapt->OutstandingRequests = TRUE;


		//
		// if the Protocol device state is OFF, then the IM driver cannot send the request below and must pend it
		//
		if ( pAdapt->PTDeviceState > NdisDeviceStateD0)
		{
			pAdapt->QueuedRequest = TRUE;
			Status = NDIS_STATUS_PENDING;
			break;
		}

		NdisRequest(&Status,
					pAdapt->BindingHandle,
					&pAdapt->Request);


		if (Status == NDIS_STATUS_SUCCESS)
		{
			*BytesRead = pAdapt->Request.DATA.SET_INFORMATION.BytesRead;
			*BytesNeeded = pAdapt->Request.DATA.SET_INFORMATION.BytesNeeded;
		}


		if (Status != NDIS_STATUS_PENDING)
		{
			pAdapt->OutstandingRequests = FALSE;
		}

	} while (FALSE);

	return(Status);
}


VOID
MPProcessSetPowerOid(
	IN OUT PNDIS_STATUS      pNdisStatus,
	IN  PADAPT					pAdapt,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesRead,
	OUT PULONG					BytesNeeded
    )
/*++

Routine Description:
	This routine does all the procssing for a request with a SetPower Oid
    The SampleIM miniport shoud accept  the Set Power and transition to the new state

    The Set Power should not be passed to the miniport below

    If the IM miniport is going into a low power state, then there is no guarantee if it will ever
    be asked go back to D0, before getting halted. No requests should be pended or queued.

	
Arguments:
	IN OUT PNDIS_STATUS      *pNdisStatus - Status of the operation
	IN  PADAPT					pAdapt,   - The Adapter structure
	IN	PVOID					InformationBuffer, - The New DeviceState
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesRead, - No of bytes read
	OUT PULONG					BytesNeeded -  No of bytes needed


Return Value:
    Status  - NDIS_STATUS_SUCCESS if all the wait events succeed.
    

--*/

{

	
	NDIS_DEVICE_POWER_STATE NewDeviceState;

	DBGPRINT ("==>MPProcessSetPowerOid"); 
	ASSERT (InformationBuffer != NULL);

	NewDeviceState = (*(PNDIS_DEVICE_POWER_STATE)InformationBuffer);

	*pNdisStatus = NDIS_STATUS_FAILURE;

	do 
	{
		//
		// Check for invalid length
		//
		if (InformationBufferLength < sizeof(NDIS_DEVICE_POWER_STATE))
		{
			*pNdisStatus = NDIS_STATUS_INVALID_LENGTH;
			break;
		}

		//
		// Check for invalid device state
		//
		if ((pAdapt->MPDeviceState > NdisDeviceStateD0) && (NewDeviceState != NdisDeviceStateD0))
		{
			//
			// If the miniport is in a non-D0 state, the miniport can only receive a Set Power to D0
			//
			ASSERT (!(pAdapt->MPDeviceState > NdisDeviceStateD0) && (NewDeviceState != NdisDeviceStateD0));

			*pNdisStatus = NDIS_STATUS_FAILURE;
			break;
		}	

		//
		// Is the miniport transitioning from an On (D0) state to an Low Power State (>D0)
		// If so, then set the StandingBy Flag - (Block all incoming requests)
		//
		if (pAdapt->MPDeviceState == NdisDeviceStateD0 && NewDeviceState > NdisDeviceStateD0)
		{
			pAdapt->StandingBy = TRUE;
		}

		//
		// If the miniport is transitioning from a low power state to ON (D0), then clear the StandingBy flag
		// All incoming requests will be pended until the physical miniport turns ON.
		//
		if (pAdapt->MPDeviceState > NdisDeviceStateD0 &&  NewDeviceState == NdisDeviceStateD0)
		{
			pAdapt->StandingBy = FALSE;
		}
		
		//
		// Now update the state in the pAdapt structure;
		//
		pAdapt->MPDeviceState = NewDeviceState;
		
		*pNdisStatus = NDIS_STATUS_SUCCESS;
	

	} while (FALSE);	
		
	if (*pNdisStatus == NDIS_STATUS_SUCCESS)
	{
		*BytesRead = sizeof(NDIS_DEVICE_POWER_STATE);
		*BytesNeeded = 0;
	}
	else
	{
		*BytesRead = 0;
		*BytesNeeded = sizeof (NDIS_DEVICE_POWER_STATE);
	}

	DBGPRINT ("<==MPProcessSetPowerOid"); 
}







VOID
MPReturnPacket(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PNDIS_PACKET			Packet
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	PADAPT			pAdapt = (PADAPT)MiniportAdapterContext;
	PNDIS_PACKET	MyPacket;
	PRSVD			Resvd;

	Resvd = (PRSVD)(Packet->MiniportReserved);
	MyPacket = Resvd->OriginalPkt;

	NdisFreePacket(Packet);
	NdisReturnPackets(&MyPacket, 1);
}


NDIS_STATUS
MPTransferData(
	OUT PNDIS_PACKET			Packet,
	OUT PUINT					BytesTransferred,
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_HANDLE				MiniportReceiveContext,
	IN	UINT					ByteOffset,
	IN	UINT					BytesToTransfer
	)
/*++

Routine Description:

	Miniport's transfer data handler.

Arguments:

	Packet					Destination packet
	BytesTransferred		Place-holder for how much data was copied
	MiniportAdapterContext	Pointer to the adapter structure
	MiniportReceiveContext	Context
	ByteOffset				Offset into the packet for copying data
	BytesToTransfer			How much to copy.

Return Value:

	Status of transfer

--*/
{
	PADAPT		pAdapt = (PADAPT)MiniportAdapterContext;
	NDIS_STATUS	Status;

	//
	// Return, if the device is OFF
	//

	if (IsIMDeviceStateOn(pAdapt) == FALSE)
	{
		return NDIS_STATUS_FAILURE;
	}

	//
	// All receives are to be done on the primary, will point to itself
	//
	pAdapt = pAdapt->pPrimaryAdapt;


	NdisTransferData(&Status,
					 pAdapt->BindingHandle,
					 MiniportReceiveContext,
					 ByteOffset,
					 BytesToTransfer,
					 Packet,
					 BytesTransferred);

	return(Status);
}

VOID
MPHalt(
	IN	NDIS_HANDLE				MiniportAdapterContext
	)
/*++

Routine Description:

	Halt handler. All the hard-work for clean-up is done here.
	LBFO - the current instance of the driver needs to be removed from the gloabal list

Arguments:

	MiniportAdapterContext	Pointer to the Adapter

Return Value:

	None.

--*/
{
	PADAPT			pAdapt = (PADAPT)MiniportAdapterContext;
	NDIS_STATUS		Status;
	PADAPT			pCursor, *ppCursor;
	PADAPT			pPromoteAdapt = NULL;
	KIRQL			OldIrql;

	DBGPRINT ("==>Passthru MPHaltMiniport\n");

	//
	// Remove the pAdapt from our global list
	//
	// Acquire the lock and keep it untill all pointers to this pAdapt have been removed
	// from the linked list
	//
	KeAcquireSpinLock (&pAdapt->SpinLock, &OldIrql);

	//
	// Remove the padapt from the list
	//
	for (ppCursor = &pAdaptList; *ppCursor != NULL; ppCursor = &(*ppCursor)->Next)
	{
		if (*ppCursor == pAdapt)
		{
			*ppCursor = pAdapt->Next;
			break;
		}
	}

	//
	//	Remove all the pointers to pAdapt from our list
	//
	for (pCursor = pAdaptList; pCursor != NULL; pCursor = pCursor->Next)
	{
		//
		// Now pointers in our global list might become invalid. Checking for Primary
		//
		if (pCursor->pPrimaryAdapt == pAdapt)
		{
			ASSERT (pCursor->isSecondary == TRUE);

			//
			// Keep a pointer to the secondary that needs to be promoted, as it is now alone
			//
			pPromoteAdapt = pCursor;
		}

		//
		// Now checking for the secondary
		//
		if (pCursor->pSecondaryAdapt == pAdapt)
		{
			ASSERT(pCursor->isSecondary == FALSE); // Assert (pCursor is Primary);

			//
			// This is all we need to change in our internal structure, the rest of the pointers are not invalid
			//
			pCursor->pSecondaryAdapt = pCursor;
		}
	}

	KeReleaseSpinLock (&pAdapt->SpinLock, OldIrql);

	//
	// If there is a miniport that needs to be promoted, promote it.
	// Call API outside of spin lock
	//
	if (pPromoteAdapt != NULL)
	{
		MPPromoteSecondary(pPromoteAdapt);
	}

	//
	// If we have a valid bind, close the miniport below the protocol
	//
	if (pAdapt->BindingHandle != NULL)
	{
		//
		// Close the binding below. and wait for it to complete
		//
		NdisResetEvent(&pAdapt->Event);

		NdisCloseAdapter(&Status, pAdapt->BindingHandle);

		if (Status == NDIS_STATUS_PENDING)
		{
			NdisWaitEvent(&pAdapt->Event, 0);
			Status = pAdapt->Status;
		}

		ASSERT (Status == NDIS_STATUS_SUCCESS);

		pAdapt->BindingHandle = NULL;
	}


	//
	// Free the resources now
	//
	NdisFreePacketPool(pAdapt->SendPacketPoolHandle);
	NdisFreePacketPool(pAdapt->RecvPacketPoolHandle);
	NdisFreeMemory(pAdapt->BundleUniString.Buffer, MAX_BUNDLEID_LENGTH,0);


	NdisFreeMemory(pAdapt, sizeof(ADAPT), 0);

	DBGPRINT("<==Passthru Minport Halt\n");
}


NDIS_STATUS
MPReset(
	OUT PBOOLEAN				AddressingReset,
	IN	NDIS_HANDLE				MiniportAdapterContext
	)
/*++

Routine Description:

	Reset Handler. We just don't do anything.

Arguments:

	AddressingReset			To let NDIS know whether we need help from it with our reset
	MiniportAdapterContext	Pointer to our adapter

Return Value:


--*/
{
	PADAPT	pAdapt = (PADAPT)MiniportAdapterContext;

	DBGPRINT("<== Passthru Miniport Reset\n"); ;

	*AddressingReset = FALSE;

	return(NDIS_STATUS_SUCCESS);
}


//
// The functions that do the LBFO work and bundling.
// If LBFO is turned off, then the Set Scondary API is never called and there are no bundles
//
NDIS_STATUS
MPBundleSearchAndSetSecondary(
	IN	PADAPT			pAdapt
	)
/*++

Routine Description:
	Go through the list of passthru structures and and search for an instantiation with a 
	matching bundleid and call MPSetMiniportSecondary on that structure

Arguments:

	pAdapt -	Should point to the structure that belolngs to the miniport 
				whose bundle id will be used in the search

Return Value:

	NDIS_STATUS_SUCCESS if not operation failed. This value is also returned event if 
	no new bundles are formed

--*/
{
	NDIS_STATUS						Status = NDIS_STATUS_FAILURE;
	NDIS_STRING						NoBundle = NDIS_STRING_CONST ("<no-bundle>");
	PADAPT							pCursor	= NULL;
	PADAPT							pPrimary = NULL;
	KIRQL							OldIrql;

	do
	{
		//
		// If bundle == '<bundle id>' then this Passthru miniport will not be a part of any bundle
		//
		if (NdisEqualUnicodeString(&NoBundle, &pAdapt->BundleUniString, TRUE))
		{
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		//
		// If the Bundle Identifier is not present, ie. someone entered a NULL string,
		// this miniport will not be part of a bundle 
		//

		if (pAdapt->BundleUniString.Length == 0)
		{
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		//
		// Acquire the global pAdapt List lock
		//
		KeAcquireSpinLock (&pAdapt->SpinLock, &OldIrql);

		//
		// Searching through the global list for a Passthru with the same BundleId
		//
		for (pCursor = pAdaptList; pCursor != NULL; pCursor = pCursor->Next)
		{
			if (pCursor == pAdapt)
			{
				//
				//	Skip to next Passthru, if the cursor is pointing to me
				//
				continue;
			}

			//
			// Do a case insenstive match, if matches, set current pAdapt as secondary
			//
			if (NdisEqualUnicodeString(&pCursor->BundleUniString, &pAdapt->BundleUniString, TRUE))
			{

				//
				// Making sure this is a primary of a bundle
				//
				ASSERT (pCursor->pSecondaryAdapt == pCursor && pCursor->isSecondary == FALSE);

				pPrimary = pCursor;

				break;
			}
		}

		//
		//	Release the lock, and also bring down our Irql
		//
		KeReleaseSpinLock (&pAdapt->SpinLock, OldIrql);

		//
		//	Call our Set Secondary function, do not call at DISPATCH_LEVEL
		//
		if (pPrimary != NULL)
		{
			Status = MPSetMiniportSecondary (pAdapt, pPrimary);

			ASSERT (Status == NDIS_STATUS_SUCCESS);
		}

		//
		//	We successsfully completed our search through the list, event if I did not find any bundles
		//
		Status = NDIS_STATUS_SUCCESS;

	} while (FALSE) ;

	return Status;
}


NDIS_STATUS
MPSetMiniportSecondary (
	IN	PADAPT		Secondary,
	IN	PADAPT		Primary
	)
/*++

Routine Description:
	Call the Ndis API to set the bundle and modify internal variables to keep track of the change


Arguments:

	Secondary Should point to the structure that points to the Secondary miniport
	Primary  Should point to the structure that points to the Primary miniport

Return Value:

	NDIS_STATUS_SUCCESS if there miniport was successfully made the secondary of the primary

--*/
{
	NDIS_STATUS	Status = NDIS_STATUS_SUCCESS;

	//
	//	ensure that the 'to be' primary is not part of another bundle
	//
	ASSERT (Primary != Secondary);
	ASSERT (Primary->isSecondary == 0);
	ASSERT (Primary->pSecondaryAdapt == Primary);

#ifdef __LBFO

	DBGPRINT ("Calling NdisMSetSecondary API on the two handles\n");

	Status = NdisMSetMiniportSecondary(Secondary->MiniportHandle,
									   Primary->MiniportHandle);

	ASSERT (Status == NDIS_STATUS_SUCCESS);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		//
		// Initialize the LBFO variables, to record the current state.
		//
		//
		Secondary->isSecondary = TRUE;
		Secondary->pPrimaryAdapt = Primary;
		Primary->pSecondaryAdapt = Secondary;

		//
		// Making sure that the other internal state variables have the correct value
		//
		Secondary->pSecondaryAdapt = Secondary;
		Primary->pPrimaryAdapt = Primary;
		Primary->isSecondary = FALSE;
	}

#endif

	return Status;
}


NDIS_STATUS
MPPromoteSecondary(
	IN	PADAPT		pAdapt
	)
/*++

Routine Description:

	Does the Passthru book keeping to promote a
	Passthru instantiation from a secondary to a primary miniport

Arguments:

	pAdapt - Pointer to the internal adapter structure

Return Value:

	NDIS_STATUS_SUCCESS - if success

	Otherwise the return value of the NdisMPromoteMiniport API
--*/
{
	NDIS_STATUS Status;

	DBGPRINT ("==> MPPromoteMiniport\n");

	Status = NdisMPromoteMiniport(pAdapt->MiniportHandle);


	ASSERT (Status == NDIS_STATUS_SUCCESS);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pAdapt->isSecondary = FALSE;

		pAdapt->pPrimaryAdapt = pAdapt;

		pAdapt->pSecondaryAdapt = pAdapt;
	}

	DBGPRINT ("<== MPPromoteMiniport\n");

	return Status;
}


BOOLEAN
MPIsSendOID (
	IN	NDIS_OID	Oid
	)
/*++

Routine Description:

		Indicates if an OID should be routed to the Send miniport. Right now, the list comprises of those OIDs which drivers
		running ethernet must support (See General Objects in the DDK). In this implementation, all OIDS are Query Oids


Arguments:

		Oid - The oid in question
Return Value:
		True  if Oid should be sent to the Send miniport
		False if Oid should be sent to the Receive miniport - default

--*/
{
	BOOLEAN fIsSend = FALSE;	//default : it is a receive  OID

	switch (Oid)
	{
		//
		// If OID needs to be sent to the Send miniport, set the boolean flag
		//
		case OID_GEN_TRANSMIT_BUFFER_SPACE :
		case OID_GEN_TRANSMIT_BLOCK_SIZE :
		case OID_GEN_MAXIMUM_TOTAL_SIZE :

		//
		// OIds used to collect transmission statistics
		//
		case OID_GEN_XMIT_OK :
		case OID_GEN_XMIT_ERROR :
		case OID_GEN_DIRECTED_BYTES_XMIT :
		case OID_GEN_DIRECTED_FRAMES_XMIT :
		case OID_GEN_MULTICAST_BYTES_XMIT :
		case OID_GEN_MULTICAST_FRAMES_XMIT :
		case OID_GEN_BROADCAST_BYTES_XMIT :
		case OID_GEN_BROADCAST_FRAMES_XMIT :
		case OID_GEN_TRANSMIT_QUEUE_LENGTH :
			fIsSend = TRUE;
			break;

		default:
			fIsSend = FALSE;
			break;
	}

	return fIsSend;
}


