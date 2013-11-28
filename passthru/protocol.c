/*++

Copyright(c) 1992  Microsoft Corporation

Module Name:

	protocol.c

Abstract:

	Ndis Intermediate Miniport driver sample. This is a passthru driver.

Author:

Environment:


Revision History:


--*/


#include "precomp.h"
#pragma hdrstop

#define MAX_PACKET_POOL_SIZE 0x0000FFFF
#define MIN_PACKET_POOL_SIZE 0x000000FF

VOID
PtBindAdapter(
	OUT PNDIS_STATUS			Status,
	IN  NDIS_HANDLE				BindContext,
	IN  PNDIS_STRING			DeviceName,
	IN  PVOID					SystemSpecific1,
	IN  PVOID					SystemSpecific2
	)
/*++

Routine Description:

	Called by NDIS to bind to a miniport below.

Arguments:

	Status			- Return status of bind here.
	BindContext		- Can be passed to NdisCompleteBindAdapter if this call is pended.
	DeviceName 		- Device name to bind to. This is passed to NdisOpenAdapter.
	SystemSpecific1	- Can be passed to NdisOpenProtocolConfiguration to read per-binding information
	SystemSpecific2	- Unused for NDIS 4.0.


Return Value:

	NDIS_STATUS_PENDING	if this call is pended. In this case call NdisCompleteBindAdapter to complete.
	Anything else		Completes this call synchronously

--*/
{
	NDIS_HANDLE						ConfigHandle = NULL;
	PNDIS_CONFIGURATION_PARAMETER	Param;
	NDIS_STRING						DeviceStr = NDIS_STRING_CONST("UpperBindings");
	PADAPT							pAdapt = NULL;
	NDIS_STATUS						Sts;
	UINT							MediumIndex;

	PNDIS_CONFIGURATION_PARAMETER	BundleParam;
	NDIS_STRING						BundleStr = NDIS_STRING_CONST("BundleId");
	NDIS_STATUS						BundleStatus;
	
	DBGPRINT("==> Passthru Protocol Initialize\n");

	do
	{
		//
		// Start off by opening the config section and reading our instance which we want
		// to export for this binding
		//
		NdisOpenProtocolConfiguration(Status,
		  							 &ConfigHandle,
		  							 SystemSpecific1);
		if (*Status != NDIS_STATUS_SUCCESS)
		{
		  	break;
		}

		NdisReadConfiguration(Status,
							  &Param,
		  					  ConfigHandle,
		  					  &DeviceStr,
		  					  NdisParameterString);
		if (*Status != NDIS_STATUS_SUCCESS)
		{
		  	break;
		}

		//
		// Allocate memory for the Adapter structure. This represents both as the protocol-context
		// as well as the adapter structure when the miniport is initialized.
		//
		NdisAllocateMemoryWithTag(&pAdapt, sizeof(ADAPT), TAG);

		if (pAdapt == NULL)
		{
			*Status = NDIS_STATUS_RESOURCES;
		  	break;
		}

		//
		// Initialize the adapter structure
		//
		NdisZeroMemory(pAdapt, sizeof(ADAPT));

		//
		// Store the String used by the registry, this will be used in reading data from the registry
		//
		NdisAllocateMemoryWithTag( &(pAdapt->BundleUniString.Buffer), MAX_BUNDLEID_LENGTH ,TAG);

		if (pAdapt->BundleUniString.Buffer == NULL)
		{
		  	*Status = NDIS_STATUS_RESOURCES;
		  	break;
		}

		pAdapt->BundleUniString.MaximumLength = MAX_BUNDLEID_LENGTH ;

		NdisReadConfiguration(&BundleStatus,
		  					  &BundleParam,
							  ConfigHandle,
		  					  &BundleStr,
		  					  NdisParameterString);

		if (BundleStatus == NDIS_STATUS_SUCCESS)
		{
		  	//
		  	// Copy the bundle identifier to our own memory
		  	//
		  	ASSERT(pAdapt->BundleUniString.MaximumLength  >= BundleParam->ParameterData.StringData.Length);

		  	pAdapt->BundleUniString.Length = BundleParam->ParameterData.StringData.Length;

		  	RtlCopyUnicodeString(&pAdapt->BundleUniString, &BundleParam->ParameterData.StringData);
		}
		else 
		{
			//
			//	We do not have a bundle id entry in the registry. To play safe we will enter 
			//	make the escape sequence the Bundle Id, This ensures that no bundle id's are 
			//	spuriously formed
			//

			NDIS_STRING NoBundle = NDIS_STRING_CONST ("<no-bundle>");

			RtlCopyUnicodeString(&pAdapt->BundleUniString, &NoBundle);
		}

		//
		//	Initializing the Event and the lock
		//
		NdisInitializeEvent(&pAdapt->Event);

		KeInitializeSpinLock(&pAdapt->SpinLock);

		//
		// Allocate a packet pool for sends. We need this to pass sends down. We cannot use the same
		// packet descriptor that came down to our send handler
		//
		NdisAllocatePacketPoolEx(Status,
		  						 &pAdapt->SendPacketPoolHandle,
		  						 MIN_PACKET_POOL_SIZE,
		  						 MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
		  						 sizeof(RSVD));

		if (*Status != NDIS_STATUS_SUCCESS)
		{
		  	break;
		}

		//
		// Allocate a packet pool for receives. We need this to indicate receives. Same consideration
		// as sends
		//
		NdisAllocatePacketPoolEx(Status,
		  						 &pAdapt->RecvPacketPoolHandle,
		  						 MIN_PACKET_POOL_SIZE,
		  						 MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
		  						 sizeof(RSVD));

		if (*Status != NDIS_STATUS_SUCCESS)
		{
		  	break;
		}



		//
		// Now open the adapter below and complete the initialization
		//
		NdisOpenAdapter(Status,
		  				&Sts,
		  				&pAdapt->BindingHandle,
		  				&MediumIndex,
		  				MediumArray,
		  				sizeof(MediumArray)/sizeof(NDIS_MEDIUM),
		  				ProtHandle,
		  				pAdapt,
		  				DeviceName,
		  				0,
		  				NULL);

		if(*Status == NDIS_STATUS_PENDING)
		{
		  	NdisWaitEvent(&pAdapt->Event, 0);
		  	*Status = pAdapt->Status;
		}

		if(*Status != NDIS_STATUS_SUCCESS)
		{
		  	break;
		}

		pAdapt->Medium = MediumArray[MediumIndex];

		//
		// Now ask NDIS to initialize our miniport
		//
		NdisIMInitializeDeviceInstanceEx(DriverHandle,
  										 &Param->ParameterData.StringData,
  										 pAdapt);

	

	} while(FALSE);

	if (ConfigHandle != NULL)
	{
		NdisCloseConfiguration(ConfigHandle);
	}

	if (*Status != NDIS_STATUS_SUCCESS)
	{
		if (pAdapt != NULL)
		{
			if (pAdapt->SendPacketPoolHandle != NULL)
			{
				 NdisFreePacketPool(pAdapt->SendPacketPoolHandle);
			}

			if (pAdapt->RecvPacketPoolHandle != NULL)
			{
				 NdisFreePacketPool(pAdapt->RecvPacketPoolHandle);
			}

			NdisFreeMemory(pAdapt, sizeof(ADAPT), 0);
		}
	}


	DBGPRINT("<== Passthru Protocol Initialize\n");
}


VOID
PtOpenAdapterComplete(
	IN  NDIS_HANDLE			 ProtocolBindingContext,
	IN  NDIS_STATUS			 Status,
	IN  NDIS_STATUS			 OpenErrorStatus
	)
/*++

Routine Description:

	Completion routine for NdisOpenAdapter issued from within the PtBindAdapter. Simply
	unblock the caller.

Arguments:

	ProtocolBindingContext	Pointer to the adapter
	Status					Status of the NdisOpenAdapter call
	OpenErrorStatus			Secondary status(ignored by us).

Return Value:

	None

--*/
{
	PADAPT	  pAdapt =(PADAPT)ProtocolBindingContext;

	DBGPRINT("==> Passthru PtOpenAdapterComplete\n");
	pAdapt->Status = Status;
	NdisSetEvent(&pAdapt->Event);
}


VOID
PtUnbindAdapter(
	OUT PNDIS_STATUS		Status,
	IN  NDIS_HANDLE			ProtocolBindingContext,
	IN  NDIS_HANDLE			UnbindContext
	)
/*++

Routine Description:

	Called by NDIS when we are required to unbind to the adapter below.
	This functions shares functionality with the miniport's HaltHandler.
	The code should ensure that NdisCloseAdapter and NdisFreeMemory is called
	only once between the two functions

Arguments:

	Status					Placeholder for return status
	ProtocolBindingContext	Pointer to the adapter structure
	UnbindContext			Context for NdisUnbindComplete() if this pends

Return Value:

	Status for NdisIMDeinitializeDeviceContext

--*/
{
	PADAPT		 pAdapt =(PADAPT)ProtocolBindingContext;
	NDIS_HANDLE BindingHandle = pAdapt->BindingHandle;

	DBGPRINT("==> Passthru PtUnbindAdapter\n");

	if (pAdapt->QueuedRequest == TRUE)
	{
		pAdapt->QueuedRequest = FALSE;

		PtRequestComplete (pAdapt,
		                 &pAdapt->Request,
		                 NDIS_STATUS_FAILURE );

	}
	//
	// Call NDIS to remove our device-instance. We do most of the work inside the HaltHandler
	//
	// The Handle will be Null if the passthru's miniport Halt Handler has been called or
	// If the IM device was never initialized
	//


	
	if(pAdapt->MiniportHandle != NULL)
	{
		*Status = NdisIMDeInitializeDeviceInstance(pAdapt->MiniportHandle);

		if(*Status != NDIS_STATUS_SUCCESS)
		{
			*Status = NDIS_STATUS_FAILURE;
		}
	}
	else
	{
		//
		// We need to do some work here. 
		// Close the binding below us 
		// and release the memory allocated.
		//
		if(pAdapt->BindingHandle != NULL)
		{
			NdisResetEvent(&pAdapt->Event);

			NdisCloseAdapter(Status, pAdapt->BindingHandle);

			//
			// Wait for it to complete
			//
			if(*Status == NDIS_STATUS_PENDING)
			{
				 NdisWaitEvent(&pAdapt->Event, 0);
				 *Status = pAdapt->Status;
			}
		}
		else
		{
			//
			// Both Our MiniportHandle and Binding Handle  should not be NULL.
			//
			*Status = NDIS_STATUS_FAILURE;
			ASSERT(0);
		}

		//
		//	Free the memory here, if was not released earlier(by calling the HaltHandler)
		//
		NdisFreeMemory(pAdapt, sizeof(ADAPT), 0);
	}

	DBGPRINT("<==Passthru UnbindAdapter\n");
}


VOID
PtUnload(
	IN	PDRIVER_OBJECT		DriverObject
	)
{
	NDIS_STATUS Status;
	NdisDeregisterProtocol(&Status, ProtHandle);
}


VOID
PtCloseAdapterComplete(
	IN	NDIS_HANDLE			ProtocolBindingContext,
	IN	NDIS_STATUS			Status
	)
/*++

Routine Description:

	Completion for the CloseAdapter call.

Arguments:

	ProtocolBindingContext	Pointer to the adapter structure
	Status					Completion status

Return Value:

	None.

--*/
{
	PADAPT	  pAdapt =(PADAPT)ProtocolBindingContext;

	pAdapt->Status = Status;
	NdisSetEvent(&pAdapt->Event);
}


VOID
PtResetComplete(
	IN  NDIS_HANDLE			ProtocolBindingContext,
	IN  NDIS_STATUS			Status
	)
/*++

Routine Description:

	Completion for the reset.

Arguments:

	ProtocolBindingContext	Pointer to the adapter structure
	Status					Completion status

Return Value:

	None.

--*/
{
	PADAPT	pAdapt =(PADAPT)ProtocolBindingContext;

	//
	// We never issue a reset, so we should not be here.
	//
	ASSERT(0);
}


VOID
PtRequestComplete(
	IN  NDIS_HANDLE			ProtocolBindingContext,
	IN  PNDIS_REQUEST		NdisRequest,
	IN  NDIS_STATUS			Status
	)
/*++

Routine Description:

	Completion handler for the previously posted request. All OIDS are completed by and sent to
	the same miniport that they were requested for.
	If Oid == OID_PNP_QUERY_POWER then the data structure needs to returned with all entries =
	NdisDeviceStateUnspecified

Arguments:

	ProtocolBindingContext	Pointer to the adapter structure
	NdisRequest				The posted request
	Status					Completion status

Return Value:

	None

--*/
{
	PADAPT		pAdapt =(PADAPT)ProtocolBindingContext;
	NDIS_OID	Oid =	pAdapt->Request.DATA.SET_INFORMATION.Oid ;

	//
	//	Change to the pAdapt for which the request originated
	//
	if(MPIsSendOID(Oid))
	{
		  pAdapt = pAdapt->pPrimaryAdapt;
		  //
		  // Will point to itself if there is no bundle(see initialization)
		  //
	}


	//
	// Since our request is not outstanding anymore
	//
	pAdapt->OutstandingRequests = FALSE;

	//
	// Complete the Set or Query, and fill in the buffer for OID_PNP_CAPABILITIES, if need be.
	//
	switch(NdisRequest->RequestType)
	{
	  case NdisRequestQueryInformation:

		  //
		  // This should not have been passsed to the miniport below us
		  //
		  ASSERT(Oid != OID_PNP_QUERY_POWER);

		  //
		  // If oid == OID_PNP_CAPABILITIES and query was successsful
		  // then fill the buffer with the required values
		  //
		  if(Oid == OID_PNP_CAPABILITIES && Status == NDIS_STATUS_SUCCESS)
		  {

				MPQueryPNPCapbilities(pAdapt,&Status);

		  }

		  *pAdapt->BytesReadOrWritten = NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;

		  *pAdapt->BytesNeeded = NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

		  NdisMQueryInformationComplete(pAdapt->MiniportHandle,
												  Status);
		  break;

	  case NdisRequestSetInformation:

		  ASSERT( Oid != OID_PNP_SET_POWER);

		  *pAdapt->BytesReadOrWritten = NdisRequest->DATA.SET_INFORMATION.BytesRead;
		  *pAdapt->BytesNeeded = NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
		  NdisMSetInformationComplete(pAdapt->MiniportHandle,
												Status);
		  break;

	  default:
		  ASSERT(0);
		  break;
	}
	
}


VOID
PtStatus(
	IN  NDIS_HANDLE			ProtocolBindingContext,
	IN  NDIS_STATUS			GeneralStatus,
	IN  PVOID				StatusBuffer,
	IN  UINT				StatusBufferSize
	)
/*++

Routine Description:

	Status handler for the lower-edge(protocol).

Arguments:

	ProtocolBindingContext	Pointer to the adapter structure
	GeneralStatus			Status code
	StatusBuffer			Status buffer
	StatusBufferSize		Size of the status buffer

Return Value:

	None

--*/
{
	PADAPT	  pAdapt =(PADAPT)ProtocolBindingContext;

	//
	// If we get a status indication before our miniport is initialized, ignore it
	// If the SampleIM is not ON, we do not pass on the status indication
	//
	if(pAdapt->MiniportHandle != NULL  &&
	  pAdapt->MPDeviceState == NdisDeviceStateD0 &&
	  pAdapt->PTDeviceState == NdisDeviceStateD0 )	
	{
		  NdisMIndicateStatus(pAdapt->MiniportHandle,
									 GeneralStatus,
									 StatusBuffer,
									 StatusBufferSize);
	}
}


VOID
PtStatusComplete(
	IN	NDIS_HANDLE			ProtocolBindingContext
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	PADAPT	  pAdapt =(PADAPT)ProtocolBindingContext;

	//
	// If we get a status indication before our miniport is initialized, ignore it
	//
	if(pAdapt->MiniportHandle != NULL  &&
	  pAdapt->MPDeviceState == NdisDeviceStateD0 &&
	  pAdapt->PTDeviceState == NdisDeviceStateD0 )	
	{
		  NdisMIndicateStatusComplete(pAdapt->MiniportHandle);
	}
}


VOID
PtSendComplete(
	IN	NDIS_HANDLE			ProtocolBindingContext,
	IN  PNDIS_PACKET		Packet,
	IN  NDIS_STATUS			Status
	)
/*++

Routine Description:

Interesting case:
We wish to send all sends down the secondary NIC. But when we indicate to the protocol above,
we need to revert back to the original miniport that Protocol wished to use for the Send


Arguments:


Return Value:


--*/
{
	PADAPT			pAdapt =(PADAPT)ProtocolBindingContext;
	PNDIS_PACKET	Pkt;
	PRSVD			Rsvd;

	//
	// Returning the Send on the Primary, will point to itself if there is no bundle
	//
	pAdapt = pAdapt->pPrimaryAdapt;

	Rsvd =(PRSVD)(Packet->ProtocolReserved);
	Pkt = Rsvd->OriginalPkt;


	NdisIMCopySendCompletePerPacketInfo (Pkt, Packet);

	NdisDprFreePacket(Packet);

	NdisMSendComplete(pAdapt->MiniportHandle,
							 Pkt,
							 Status);
}


VOID
PtTransferDataComplete(
	IN  NDIS_HANDLE			ProtocolBindingContext,
	IN  PNDIS_PACKET		Packet,
	IN  NDIS_STATUS			Status,
	IN  UINT				BytesTransferred
	)
/*++

Routine Description:
Same as the Send above, all sends need to be completed on the Primary's MiniportHandle

Arguments:

Return Value:

--*/
{
	PADAPT	  pAdapt =(PADAPT)ProtocolBindingContext;

	//
	// Returning the Send on the Primary, will point to itself if there is no LBFO
	//
	pAdapt = pAdapt->pPrimaryAdapt;

	if(pAdapt->MiniportHandle)
	{
		  NdisMTransferDataComplete(pAdapt->MiniportHandle,
											 Packet,
											 Status,
											 BytesTransferred);
	}
}


NDIS_STATUS
PtReceive(
	IN  NDIS_HANDLE			ProtocolBindingContext,
	IN  NDIS_HANDLE			MacReceiveContext,
	IN  PVOID				HeaderBuffer,
	IN  UINT				HeaderBufferSize,
	IN  PVOID				LookAheadBuffer,
	IN  UINT				LookAheadBufferSize,
	IN  UINT				PacketSize
	)
/*++

Routine Description:
LBFO - need to use primary for all receives

Arguments:


Return Value:

--*/
{
	PADAPT			pAdapt =(PADAPT)ProtocolBindingContext;
	PNDIS_PACKET	MyPacket, Packet;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;

	if(!pAdapt->MiniportHandle)
	{
		Status = NDIS_STATUS_FAILURE;
	}
	else do
	{
		//
		// We should not be getting Receives on a Secondary, this is just specific to our  LBFO driver
		//

		if(pAdapt->isSecondary)
		{
			 DBGPRINT("PASSTHRU GETTING RECIEVES ON SECONDARY\n");
			 ASSERT(0);
		}

		//
		// If this was indicated by the miniport below as a packet, then get that packet pointer and indicate
		// it as a packet as well(with appropriate status). This way the OOB stuff is accessible to the
		// transport above us.
		//
		Packet = NdisGetReceivedPacket(pAdapt->BindingHandle, MacReceiveContext);
		if(Packet != NULL)
		{
			 //
			 // Get a packet off the pool and indicate that up
			 //
			 NdisDprAllocatePacket(&Status,
										  &MyPacket,
										  pAdapt->RecvPacketPoolHandle);

			 if(Status == NDIS_STATUS_SUCCESS)
			 {
				  MyPacket->Private.Head = Packet->Private.Head;
				  MyPacket->Private.Tail = Packet->Private.Tail;

				  //
				  // Get the original packet(it could be the same packet as one received or a different one
				  // based on # of layered MPs) and set it on the indicated packet so the OOB stuff is visible
				  // correctly at the top.
				  //
				  NDIS_SET_ORIGINAL_PACKET(MyPacket, NDIS_GET_ORIGINAL_PACKET(Packet));
				  NDIS_SET_PACKET_HEADER_SIZE(MyPacket, HeaderBufferSize);

				  //
				  // Set Packet Flags
				  //
				  NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

				  //
				  // Make sure the status is set to NDIS_STATUS_RESOURCES.
				  //
				  NDIS_SET_PACKET_STATUS(MyPacket, NDIS_STATUS_RESOURCES);

				  NdisMIndicateReceivePacket(pAdapt->MiniportHandle, &MyPacket, 1);

				  ASSERT(NDIS_GET_PACKET_STATUS(MyPacket) == NDIS_STATUS_RESOURCES);
				  NdisDprFreePacket(MyPacket);
				  break;
			 }
		}

		//
		// Fall through if the miniport below us has either not indicated a packet or we could not
		// allocate one
		//
		pAdapt->IndicateRcvComplete = TRUE;
		switch(pAdapt->Medium)
		{
		  case NdisMedium802_3:
			 NdisMEthIndicateReceive(pAdapt->MiniportHandle,
											 MacReceiveContext,
											 HeaderBuffer,
											 HeaderBufferSize,
											 LookAheadBuffer,
											 LookAheadBufferSize,
											 PacketSize);
			 break;

		  case NdisMedium802_5:
			 NdisMTrIndicateReceive(pAdapt->MiniportHandle,
											MacReceiveContext,
											HeaderBuffer,
											HeaderBufferSize,
											LookAheadBuffer,
											LookAheadBufferSize,
											PacketSize);
			 break;

		  case NdisMediumFddi:
			 NdisMFddiIndicateReceive(pAdapt->MiniportHandle,
											  MacReceiveContext,
											  HeaderBuffer,
											  HeaderBufferSize,
											  LookAheadBuffer,
											  LookAheadBufferSize,
											  PacketSize);
			 break;

		  default:
			 ASSERT(0);
			 break;
		}

	} while(FALSE);

	return Status;
}


VOID
PtReceiveComplete(
	IN	NDIS_HANDLE		ProtocolBindingContext
	)
/*++

Routine Description:

	Called by the adapter below us when it is done indicating a batch of received buffers.

Arguments:

	ProtocolBindingContext	Pointer to our adapter structure.

Return Value:

	None

--*/
{
	PADAPT		pAdapt =(PADAPT)ProtocolBindingContext;

	//
	// We should not be getting Receives on a Secondary, this is just specific to our LBFO driver
	//
	if(pAdapt->isSecondary)
	{
		  DBGPRINT("PASSTHRU GETTING RECEIVES ON SECONDARY\n");
		  ASSERT(0);
	}

	if((pAdapt->MiniportHandle != NULL) && pAdapt->IndicateRcvComplete)
	{
		switch(pAdapt->Medium)
		{
		  case NdisMedium802_3:
			NdisMEthIndicateReceiveComplete(pAdapt->MiniportHandle);
			break;

		  case NdisMedium802_5:
			NdisMTrIndicateReceiveComplete(pAdapt->MiniportHandle);
			break;

		  case NdisMediumFddi:
			NdisMFddiIndicateReceiveComplete(pAdapt->MiniportHandle);
			break;

		  default:
			ASSERT(0);
			break;
		}
	}
	pAdapt->IndicateRcvComplete = FALSE;
}


INT
PtReceivePacket(
	IN	NDIS_HANDLE			ProtocolBindingContext,
	IN	PNDIS_PACKET		Packet
	)
/*++

Routine Description:

	ReceivePacket handler. Called up by the miniport below when it supports NDIS 4.0 style receives.
	Re-package the packet and hand it back to NDIS for protocols above us. The re-package part is
	important since NDIS uses the WrapperReserved part of the packet for its own book-keeping. Also
	the re-packaging works differently when packets flow-up or down. In the up-path(here) the protocol
	reserved is owned by the protocol above. We need to use the miniport reserved here.

Arguments:

	ProtocolBindingContext	Pointer to our adapter structure.
	Packet - Pointer to the packet

Return Value:

	== 0 -> We are done with the packet
	!= 0 -> We will keep the packet and call NdisReturnPackets() this many times when done.
--*/
{
	PADAPT			pAdapt =(PADAPT)ProtocolBindingContext;
	NDIS_STATUS	Status;
	PNDIS_PACKET	MyPacket;
	PRSVD			Resvd;

	if(!pAdapt->MiniportHandle)
	{
		  return 0;
	}

	//
	// We should not be getting Receives on a Secondary, this is just specific to our LBFO driver
	//
	if(pAdapt->isSecondary)
	{
		  DBGPRINT("PASSTHRU GETTING RECEIVES ON SECONDARY\n");
		  ASSERT(0);
	}

	//
	// Get a packet off the pool and indicate that up
	//
	NdisDprAllocatePacket(&Status,
						   &MyPacket,
						   pAdapt->RecvPacketPoolHandle);

	if(Status == NDIS_STATUS_SUCCESS)
	{
		Resvd =(PRSVD)(MyPacket->MiniportReserved);
		Resvd->OriginalPkt = Packet;

		MyPacket->Private.Head = Packet->Private.Head;
		MyPacket->Private.Tail = Packet->Private.Tail;

		//
		// Get the original packet(it could be the same packet as one received or a different one
		// based on # of layered MPs) and set it on the indicated packet so the OOB stuff is visible
		// correctly at the top.
		//
		NDIS_SET_ORIGINAL_PACKET(MyPacket, NDIS_GET_ORIGINAL_PACKET(Packet));

		//
		// Set Packet Flags
		//
		NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

		Status = NDIS_GET_PACKET_STATUS(Packet);

		NDIS_SET_PACKET_STATUS(MyPacket, Status);
		NDIS_SET_PACKET_HEADER_SIZE(MyPacket, NDIS_GET_PACKET_HEADER_SIZE(Packet));

		NdisMIndicateReceivePacket(pAdapt->MiniportHandle, &MyPacket, 1);

		if(Status == NDIS_STATUS_RESOURCES)
		{
		  NdisDprFreePacket(MyPacket);
		}

		return((Status != NDIS_STATUS_RESOURCES) ? 1 : 0);
	}
	else
	{
		  //
		  // We are out of packets. Silently drop it. Alternatively we can deal with it:
		  //	- By keeping separate send and receive pools
		  //	- Dynamically allocate more pools as needed and free them when not needed
		  //
		  return(0);
	}
}




NDIS_STATUS
PtPNPHandler(
	IN	NDIS_HANDLE		ProtocolBindingContext,
	IN	PNET_PNP_EVENT	pNetPnPEvent
	)

/*++
Routine Description:

	This is the Protocol PNP handlers. All PNP Related OIDS(requests) are routed to this function
	If the Power of the SetPower PNP Event is received, then the new power state is copied to
	the internal state of the Passthru driver. Need to complete all sends and requests before
	returning from this	function.

Arguments:

	ProtocolBindingContext	Pointer to our adapter structure.
	pNetPnPEvent Pointer to a Net_PnP_Event

Return Value:

	NDIS_STATUS_SUCCESS: as we do not do much here

--*/
{
	PADAPT			pAdapt  =(PADAPT)ProtocolBindingContext;
	NDIS_STATUS	Status  = NDIS_STATUS_SUCCESS;

	DBGPRINT ("PtPnPHandler");

	//
	// This will happen when all entities in the system need to be notified
	//

	switch(pNetPnPEvent->NetEvent)
	{
	 case  NetEventSetPower :
	    Status = PtPnPNetEventSetPower(pAdapt, pNetPnPEvent);
	    break;

	 case NetEventReconfigure :
	    Status  = PtPnPNetEventReconfigure(pAdapt, (PCWSTR)pNetPnPEvent->Buffer);
	    break;

	 default :
	    Status  = NDIS_STATUS_SUCCESS;
	    break;
	}

	return Status;
}

NDIS_STATUS
PtPnPNetEventReconfigure(
	IN	PADAPT			pAdapt,
	IN	PCWSTR			pBundleString
	)
/*++
Routine Description:
	This is the function that will be called by the PNP handler whenever a PNPNetEventReconfigure happens
	Protocol will read the Bundle from the registry.
	
	if pAdapt is NULL, then a global reconfig event has been issued. We should use this to ensure that our protocol
	is bound to its underlying miniports
	
	Simple Algorithm for Bundles:
	If bundleId was not changed then exit.


	if pAdapt is the secondary of a bundle, promote pAdapt.
	If pAdapt is the primary of a bundle, promote the secondary miniport


	Now check to see if the new bundleId causes us to be a part of another bundle
	Walk through the list attach pAdapt to a miniport that has the same bundleId.


	If there is a Device Instantiated with the same bundle ID then we will become the secondary of that miniport


Arguments:

	ProtocolBindingContext	Pointer to our adapter structure.

Return Value:

	NDIS_STATUS_SUCCESS: as we do not do much here


--*/
{
	NDIS_STATUS	BundleStatus = NDIS_STATUS_SUCCESS;
	NDIS_STRING NewBundleUniString;
	

	if(pAdapt == NULL)
	{
		NdisReEnumerateProtocolBindings (ProtHandle);		
		return BundleStatus;
	}

	if (pBundleString == NULL)
	{
		return BundleStatus;
	}

	NdisInitUnicodeString( &NewBundleUniString, pBundleString);


	do
	{
	   //
	   // If the bundle Identifier was not changed, do not do anything
	   //
	   if(NdisEqualUnicodeString(&NewBundleUniString, &pAdapt->BundleUniString, TRUE))
	   {
	  	   break;
	   }

	   //
	   // We have a new bundle id , copy it and do the necessary bundling work
	   //
	   RtlCopyUnicodeString(&pAdapt->BundleUniString , &NewBundleUniString);

	   //
	   //  If we are part of a bundle and our bundle id was changed, either the primary or the secondary
	   //  will get promoted
	   //  If we are the secondary of a bundle promote ourselves
	   //
	   if(pAdapt->isSecondary)
	   {
			PADAPT pPrimaryAdapt = pAdapt->pPrimaryAdapt;

			BundleStatus = MPPromoteSecondary(pAdapt);

			if(BundleStatus != NDIS_STATUS_SUCCESS)
			{
				 ASSERT(0);
				 break;
			}

			//
			// resetting all the internal variables of the primary Adapter structure
			//
			pPrimaryAdapt->pPrimaryAdapt = pPrimaryAdapt;
			pPrimaryAdapt->pSecondaryAdapt = pPrimaryAdapt;
			pPrimaryAdapt->isSecondary = FALSE;
	   }
	   else
	   {
			//
			// The BundleId has changed. If we are the primary of a bundle,
			// then we need to promote the secondary
			//
			if(pAdapt->pSecondaryAdapt != pAdapt)
			{
				BundleStatus = MPPromoteSecondary(pAdapt->pSecondaryAdapt);
				if(BundleStatus != NDIS_STATUS_SUCCESS)
				{
				     ASSERT(0);
				     break;
				}

				//
				// resetting all our internal variables
				//
				pAdapt->pSecondaryAdapt  = pAdapt;
				pAdapt->pPrimaryAdapt = pAdapt;
				pAdapt->isSecondary = FALSE ;
			}
	   }

	   //
	   //	Now we need to see if the new bundle ID makes the Passthru part of another bundle
	   //	If so. then set the current pAdapt as the secondary
	   //
	   BundleStatus = MPBundleSearchAndSetSecondary(pAdapt);

	} while(FALSE) ;

	DBGPRINT("<==PtPNPNetEventReconfigure\n");

	return BundleStatus;
}





NDIS_STATUS
PtPnPNetEventSetPower(
	IN	PADAPT			pAdapt,
	IN  PNET_PNP_EVENT	pNetPnPEvent
	)
/*++
Routine Description:

	Sets the PowerState to the required level. Waits for all outstanding Sends and requests to complete

Arguments:

	pAdapt			-	Pointer to the adpater structure
	pNetPnpEvent	-	The Net Pnp Event. this contains the new device state

Return Value:

	NDIS_STATUS_SUCCESS: If the device successfully changed its power state


--*/
{
	PNDIS_DEVICE_POWER_STATE	pDeviceState  =(PNDIS_DEVICE_POWER_STATE)(pNetPnPEvent->Buffer);
	NDIS_DEVICE_POWER_STATE		PrevDeviceState = pAdapt->PTDeviceState;  
	NDIS_STATUS					Status ;
	

	//
	// Set the Internal Device State, this blocks all new sends or receives
	//
	pAdapt->PTDeviceState = *pDeviceState;

	//
	// if we are being sent to standby, block for outstanding requests and sends
	//
	if(*pDeviceState > NdisDeviceStateD0)
	{
	   //
	   // If the physical miniport is going to standby, fail all incoming requests
	   //
	   if (PrevDeviceState == NdisDeviceStateD0)
	   {
           pAdapt->StandingBy = TRUE;
	   }

	   while(NdisPacketPoolUsage(pAdapt->SendPacketPoolHandle) != 0)
	   {
	  	   //
	  	   // sleep till outstanding sends complete
	  	   //
	  	   NdisMSleep(10);
	   }

	   while(pAdapt->OutstandingRequests == TRUE)
	   {
	  	   //
	  	   // sleep till outstanding requests complete
	  	   //
	  	   NdisMSleep(10);
	   }

	   ASSERT(NdisPacketPoolUsage(pAdapt->SendPacketPoolHandle) == 0);
	   ASSERT(pAdapt->OutstandingRequests == FALSE);
	}
	else
	{
		//
		// The protocol is being turned on. A pending request must be completed
		//
		if (pAdapt->QueuedRequest == TRUE)
		{

			pAdapt->QueuedRequest = FALSE;

			NdisRequest(&Status,
			           pAdapt->BindingHandle,
			           &pAdapt->Request);

			//
			// The underlying miniport completed the request synchronously, the IM
			// needs to complete the request which it had pended earlier
			//

			if (Status != NDIS_STATUS_PENDING)
			{
				PtRequestComplete(pAdapt,
			    	            &pAdapt->Request,
			        	        Status);
			}

		}

		//
		// If the physical miniport is powering up (from Low power state to D0), 
		// clear the flag
		//
		if (PrevDeviceState > NdisDeviceStateD0)
		{
			pAdapt->StandingBy = FALSE;
		}


	}

	Status = NDIS_STATUS_SUCCESS;

	return Status;
}


