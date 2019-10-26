/********************************************************************************************************************************
 **
 FileName:                   CanIf.c
 AUTOSAR Version:            4.2.2
                                                                                                                                                                          **
 ********************************************************************************************************************************/
/********************************************************************************************************************************
 **                                                          Includes                                                                                                **
 ********************************************************************************************************************************/
#include "Can.h"
#include "CanIf.h"
/********************************************************************************************************************************
 **                                                       Global Variables                                                                                       **
 ********************************************************************************************************************************/
extern CanIf_ConfigType CanIf;

static enum CanIfStateType{CANIF_UNINIT, CANIF_READY}CanIfState = CANIF_UNINIT;
static CanIf_ControllerModeType  CanIfControllerMode[NUMBER_OF_CONTROLLERS]={CANIF_CS_UNINIT, CANIF_CS_UNINIT};
static CanIf_PduModeType         CanIfPduMode[NUMBER_OF_CONTROLLERS];

#if(CanIfPublicTxBuffering == true)
static CanIf_TxBufferType CanIfTxBuffer[NUMBER_OF_BUFFERS] = {
	{
		.CanIfBufferCfgRef = &CanIf.CanIfInitCfg.CanIfBufferCfg[0U],
		.CanIfTxBufferFront = -1,
		.CanIfTxBufferRear = -1,
		.CanIfTxBufferSize = -1,
		.CanIfTxBufferPduAvailable = {false}
	}
};
#endif
uint8 CanIfDevelopmentError;







 /********************************************************************************************************************************
**                                                        Local Functions                                                                                        **
*********************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
static Std_ReturnType CanIf_TxBufferGet(CanIfBufferCfg* CanIfTxPduBufferRefLocal, CanIf_TxBufferType *CanIfTxBufferLocal);
static Std_ReturnType CanIf_TxBufferNotEmpty(CanIf_TxBufferType *CanIfTxBufferLocal);
static Std_ReturnType CanIf_TxBufferNotFull(CanIf_TxBufferType *CanIfTxBufferLocal);
static Std_ReturnType CanIf_TxBufferDequeue(CanIfTxPduCfg* TxPdu, const Can_PduType* PduInfoPtr);
static Std_ReturnType CanIf_TxBufferEnqueue(CanIfTxPduCfg* TxPdu, const Can_PduType* PduInfoPtr);
static Std_ReturnType CanIf_TxBufferClear(uint8 ControllerId);
#endif
static Std_ReturnType CanIf_GetTxPdu(PduIdType CanIfTxSduId, CanIfTxPduCfg* TxPduPtr);
static Std_ReturnType CanIf_GetRxPdu(PduIdType CanIfRxSduId, CanIfRxPduCfg* RxPduPtr);

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferGet
Parameters (in):                      CanIfTxPduBufferRefLocal-->Tx PDU reference to a CanIf buffer configuration.
										   CanIfBufferRefLocal -->Reference to the Tx Buffer
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:							This service searches for the CanIf buffer referenced by the available
										CanIf buffers and the Tx Pdu
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferGet(CanIfBufferCfg *CanIfTxPduBufferRefLocal, CanIf_TxBufferType *CanIfBufferRefLocal)
{
	uint8 BufferIndex;

	/* Check if the reference to the buffer configurations is a valid reference */
	if(CanIfTxPduBufferRefLocal == NULL)
	{
		return E_NOT_OK;
	}
	else
	{
		/* Loop over the available buffers and return the one with the same buffer configuration reference as the Tx PDU */
		for(BufferIndex = 0; BufferIndex < NUMBER_OF_BUFFERS ; BufferIndex ++)
		{
			if(CanIfTxBuffer[BufferIndex].CanIfBufferCfgRef == CanIfTxPduBufferRefLocal)
			{
			    CanIfBufferRefLocal = &CanIfTxBuffer[BufferIndex];
				return E_OK;
			}
			else
			{

			}
		}
	}

	return E_NOT_OK;
}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferNotEmpty
Parameters (in):                        CanIfTxBufferRefLocal -->Reference to the Tx Buffer
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:							This service checks if the buffer is empty
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferNotEmpty(CanIf_TxBufferType *CanIfTxBufferRefLocal)
{
	/* Check if the reference to the buffer is a valid reference */
	if(CanIfTxBufferRefLocal == NULL)
	{
		return E_NOT_OK;
	}
	else
	{
		if(CanIfTxBufferRefLocal->CanIfTxBufferSize < 0)
		{
			return E_NOT_OK;
		}
		else
		{
			return E_OK;
		}
	}

}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferNotFull
Parameters (in):                        CanIfTxBufferRefLocal -->Reference to the Tx Buffer
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:							This service checks if the buffer is full
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferNotFull(CanIf_TxBufferType *CanIfTxBufferRefLocal)
{
	/* Check if the reference to the buffer is a valid reference */
	if(CanIfTxBufferRefLocal == NULL)
	{
		return E_NOT_OK;
	}
	else
	{
		if(CanIfTxBufferRefLocal->CanIfTxBufferSize < CanIfTxBufferRefLocal->CanIfBufferCfgRef->CanIfBufferSize)
		{
			return E_OK;
		}
		else
		{
			return E_NOT_OK;
		}
	}

}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferDequeue
 Parameters (in):                        		TxPdu -->Reference to the Tx PDU
												PduInfoPtr --> Reference to the PDU
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:							This service returns the next PDU in the FIFO buffer
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferDequeue(CanIfTxPduCfg* TxPdu, const Can_PduType* PduInfoPtr)
{
    CanIf_TxBufferType* CanIfTxBufferRefLocal = NULL;

	/* Check if the reference to the TxPdu is a valid reference */
	if(TxPdu == NULL)
	{
		return E_NOT_OK;
	}
	else
	{
		/* Get the buffer of the Tx PDU */
		if(CanIf_TxBufferGet(TxPdu->CanIfTxPduBufferRef, CanIfTxBufferRefLocal) == E_NOT_OK)
		{
			return E_NOT_OK;
		}
		else
		{
			/* Check if the buffer is empty */
			if(CanIf_TxBufferNotEmpty(CanIfTxBufferRefLocal) == E_NOT_OK)
			{
				return E_NOT_OK;
			}
			else
			{
				/* Set the PDU pointer to the next PDU in the buffer */
				PduInfoPtr = &CanIfTxBufferRefLocal->CanIfTxBufferPduData[CanIfTxBufferRefLocal->CanIfTxBufferFront];
				/* Set the PDU state as not available in the buffer */
				CanIfTxBufferRefLocal->CanIfTxBufferPduAvailable[TxPdu->CanIfTxPduId] = false;
				/* Decrease the current avaiable PDUs */
				CanIfTxBufferRefLocal->CanIfTxBufferSize --;
				CanIfTxBufferRefLocal->CanIfTxBufferFront ++;
				return E_OK;
			}
		}
	}

}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferEnqueue
 Parameters (in):                        		TxPdu -->Reference to the Tx PDU
												PduInfoPtr --> Reference to the PDU
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:								This service adds a PDU to the FIFO buffer
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferEnqueue(CanIfTxPduCfg* TxPdu, const Can_PduType* PduInfoPtr)
{
	CanIf_TxBufferType* CanIfTxBufferRefLocal = NULL;

	/* Check if the reference to the PDUs is a valid reference */
	if(TxPdu == NULL || PduInfoPtr == NULL)
	{
		return E_NOT_OK;
	}
	else
	{
		/* Get the buffer of the Tx PDU */
		if(CanIf_TxBufferGet(TxPdu->CanIfTxPduBufferRef, CanIfTxBufferRefLocal) == E_NOT_OK)
		{
			return E_NOT_OK;
		}
		else
		{
			/* Check if the buffer is full */
			if(CanIf_TxBufferNotFull(CanIfTxBufferRefLocal) == E_NOT_OK)
			{
				return E_NOT_OK;
			}
			else
			{
				/* Check if the PDU is already stored in the buffer */
				if(CanIfTxBufferRefLocal->CanIfTxBufferPduAvailable[TxPdu->CanIfTxPduId] == true)
				{
					return E_NOT_OK;
				}
				else
				{
					/* The buffer is not initialized yet. We will initialize the buffer by making both front and rear 0 and size 1
					and give a value to the first PDU */
					if(CanIfTxBufferRefLocal->CanIfTxBufferSize < 0)
					{
					    CanIfTxBufferRefLocal->CanIfTxBufferPduData[0] = *PduInfoPtr;
					    CanIfTxBufferRefLocal->CanIfTxBufferRear = 0;
					    CanIfTxBufferRefLocal->CanIfTxBufferFront = 0;
					    CanIfTxBufferRefLocal->CanIfTxBufferSize = 1;
					}
					/* This is the case when the rear is the last PDU in the buffer. We will add a new PDU at the index 0 */
					else if(CanIfTxBufferRefLocal->CanIfTxBufferRear == CanIfTxBufferRefLocal->CanIfBufferCfgRef->CanIfBufferSize - 1)
					{
					    CanIfTxBufferRefLocal->CanIfTxBufferPduData[0] = *PduInfoPtr;
					    CanIfTxBufferRefLocal->CanIfTxBufferRear = 0;
					    CanIfTxBufferRefLocal->CanIfTxBufferSize ++;
					}
					/* This is the normal case*/
					else
					{
					    CanIfTxBufferRefLocal->CanIfTxBufferPduData[CanIfTxBufferRefLocal->CanIfTxBufferRear + 1] = *PduInfoPtr;
					    CanIfTxBufferRefLocal->CanIfTxBufferRear ++;
					    CanIfTxBufferRefLocal->CanIfTxBufferSize ++;
					}

					/* Set the PDU as currently available in the buffer */
					CanIfTxBufferRefLocal->CanIfTxBufferPduAvailable[TxPdu->CanIfTxPduId] = true;
					return E_OK;
				}
			}
		}
	}

}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferClear
 Parameters (in):                             ControllerId -->Controller Id
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                               This service clear the FIFO buffer
 *******************************************************************************************************************************/
Std_ReturnType CanIf_TxBufferClear(uint8 ControllerId)
{
    uint8 BufferIndex;
    uint8 PduIndex;
    if(ControllerId < NUMBER_OF_CONTROLLERS)
    {
        for(BufferIndex = (uint8)0; BufferIndex < NUMBER_OF_BUFFERS; BufferIndex++)
        {
            if(CanIfTxBuffer[BufferIndex].CanIfBufferCfgRef->CanIfBufferHthRef->CanIfHthCanCtrlIdRef->CanIfCtrlId == ControllerId)
            {
                CanIfTxBuffer[BufferIndex].CanIfTxBufferFront = -1;
                CanIfTxBuffer[BufferIndex].CanIfTxBufferRear = -1;
                CanIfTxBuffer[BufferIndex].CanIfTxBufferSize = -1;

                for(PduIndex = 0; PduIndex < CanIfMaxTxPduCfg ; PduIndex++)
                {
                    CanIfTxBuffer[BufferIndex].CanIfTxBufferPduAvailable[PduIndex] = false;
                }
            }
            else
            {

            }
        }
    }
    else
    {
        return E_NOT_OK;
    }

    return E_OK;

}

/*********************************************************************************************************************************
 Service name:                                       CanIf_GetTxPdu
 Parameters (in):                        		CanIfTxSduId -->Id of the Tx PDU
												TxPduPtr --> Reference to the Tx PDU
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:								This service returns the Tx PDU with the desired ID
 *******************************************************************************************************************************/
Std_ReturnType CanIf_GetTxPdu(PduIdType CanIfTxSduId, CanIfTxPduCfg* TxPduPtr)
{
	uint32 TxPduIndex;
	if(CanIfTxSduId < CanIfMaxTxPduCfg)
	{
		for(TxPduIndex = 0; TxPduIndex < CanIfMaxTxPduCfg ; TxPduIndex++)
		{
			if(CanIf.CanIfInitCfg.CanIfTxPduCfg[TxPduIndex].CanIfTxPduId == CanIfTxSduId)
			{
			    TxPduPtr = &CanIf.CanIfInitCfg.CanIfTxPduCfg[TxPduIndex];
				return E_OK;
			}
			else
			{

			}
		}
	}
	else
	{
		return E_NOT_OK;
	}

	return E_NOT_OK;
}




/*********************************************************************************************************************************
 Service name:                                       CanIf_GetRxPdu
 Parameters (in):                               CanIfRxSduId -->Id of the Rx PDU
                                                RxPduPtr --> Reference to the Rx PDU
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                               This service returns the Rx PDU with the desired ID
 *******************************************************************************************************************************/
Std_ReturnType CanIf_GetRxPdu(PduIdType CanIfRxSduId, CanIfRxPduCfg* RxPduPtr)
{
    uint32 RxPduIndex;
    if(CanIfRxSduId < CanIfMaxRxPduCfg)
    {
        for(RxPduIndex = 0; RxPduIndex < CanIfMaxRxPduCfg ; RxPduIndex++)
        {
            if(CanIf.CanIfInitCfg.CanIfRxPduCfg[RxPduIndex].CanIfRxPduId == CanIfRxSduId)
            {
                RxPduPtr = &CanIf.CanIfInitCfg.CanIfRxPduCfg[RxPduIndex];
                return E_OK;
            }
            else
            {

            }
        }
    }
    else
    {
        return E_NOT_OK;
    }

    return E_NOT_OK;
}





/********************************************************************************************************************************
**                                                      Functions  definition                                                                        **
*********************************************************************************************************************************/


/*********************************************************************************************************************************
 Service name:                                              CanIf_Init
 Service ID[hex]:                                               0x01
 Sync/Async:                                               Synchronous
 Reentrancy:                                               Non Reentrant
 Parameters (in):                                           ConfigPtr
 Parameters (inout):                                          None
 Parameters (out):                                             None
 Return value:                                                   None
 Description:
 This service Initializes internal and external interfaces of the CAN Inter-
face for the further processing.
 *******************************************************************************************************************************/
void CanIf_Init(const CanIf_ConfigType* ConfigPtr){

















































}

/*********************************************************************************************************************************
 Service name:                                       CanIf_SetControllerMode
 Service ID[hex]:                                               0x03
 Sync/Async:                                               Asynchronous
 Reentrancy:                                               Reentrant (Not for the same controller)
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
																	ControllerMode -->Requested mode transition
 Parameters (inout):                                          None
 Parameters (out):                                             None
 Return value:                                                   Std_ReturnType
 Description:
					This service calls the corresponding CAN Driver service for changing of
					the CAN controller mode.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_SetControllerMode(uint8 ControllerId,CanIf_ControllerModeType ControllerMode){
















































    return E_OK;
}


/*********************************************************************************************************************************
 Service name:                                       CanIf_GetControllerMode
 Service ID[hex]:                                               0x04
 Sync/Async:                                               Synchronous
 Reentrancy:                                               Non Reentrant
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
 Parameters (inout):                                          None
 Parameters (out):                                     ControllerModePtr -->Pointer to a memory location, where the current
																								mode of the CAN controller will be stored.
 Return value:                                                   Std_ReturnType
 Description:
					This service reports about the current status of the requested CAN controller.
 *******************************************************************************************************************************/
Std_ReturnType CanIf_GetControllerMode(uint8 ControllerId,CanIf_ControllerModeType* ControllerModePtr){
















































    return E_OK;
}

/*********************************************************************************************************************************
 Service name:                                       CanIf_Transmit
 Service ID[hex]:                                               0x05
 Sync/Async:                                               Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                           CanIfTxSduId    -->L-SDU handle to be transmitted.This handle specifies the corresponding CAN L-
																								SDU ID and implicitly the CAN Driver instance as
																								well as the corresponding CAN controller device.
																	CanIfTxInfoPtr   -->Pointer to a structure with CAN L-SDU related data:
																								DLC and pointer to CAN L-SDU buffer including the
																								MetaData of dynamic L-PDUs.
Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                              Std_ReturnType
 Description:
					This service initiates a request for transmission of the CAN L-PDU specified by
					 the CanTxSduId and CAN related data in the L-SDU structure.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_Transmit(PduIdType CanIfTxSduId, const PduInfoType* CanIfTxInfoPtr)
{
	CanIf_ControllerModeType CanIf_GetControllerModeReturn;
	CanIf_PduModeType CanIf_GetPduModeReturn;
	Can_ReturnType	Can_WriteReturn;
	CanIfTxPduCfg *TxPduPtr = NULL;

	/*  CanIf must be initialized after Power ON */
	if(CanIfState == CANIF_UNINIT)
	{
		return E_NOT_OK;
	}
	else
	{
		if(CanIf_GetTxPdu(CanIfTxSduId, TxPduPtr) == E_NOT_OK)
		{
			return E_NOT_OK;
		}
		else
		{
			/* [SWS_CANIF_00317] The service CanIf_Transmit() shall not accept a transmit
			request, if the controller mode is not CANIF_CS_STARTED and the channel mode at
			least for the transmit path is not online or off-line active. */
			if(CanIf_GetControllerMode(TxPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthCanCtrlIdRef->CanIfCtrlId, &CanIf_GetControllerModeReturn) == E_NOT_OK)
			{
				return E_NOT_OK;
			}
			else
			{
				if(CanIf_GetControllerModeReturn != CANIF_CS_STARTED)
				{
					return E_NOT_OK;
				}
				else
				{
					if(CanIf_GetPduMode(TxPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthCanCtrlIdRef->CanIfCtrlId, &CanIf_GetPduModeReturn) == E_NOT_OK)
					{
						return E_NOT_OK;
					}
					else
					{
						if(CanIf_GetPduModeReturn == CANIF_OFFLINE || CanIf_GetPduModeReturn == CANIF_TX_OFFLINE)
						{
							return E_NOT_OK;
						}
						else
						{
							/* [SWS_CANIF_00320] If parameter CanIfTxInfoPtr of CanIf_Transmit() has
							an invalid value, CanIf shall report development error code CANIF_E_PARAM_POINTER
							to the Det_ReportError service of the DET module, when CanIf_Transmit()
							is called. */
							if(CanIfTxInfoPtr == NULL)
							{
								#if(CanIfPublicDevErrorDetect == true)
								Det_ReportError(MODULE_ID, INSTANCE_ID, 0x05, CANIF_E_PARAM_POINTER);
								#endif
								return E_NOT_OK;
							}
							else
							{
								/* [SWS_CANIF_00319] If parameter CanIfTxSduId of CanIf_Transmit() has an
								invalid value, CanIf shall report development error code CANIF_E_INVALID_TXPDUID
								to the Det_ReportError service of the DET, when CanIf_Transmit() is called. */
								if(CanIfTxSduId >= CanIfMaxTxPduCfg)
								{
									#if(CanIfPublicDevErrorDetect == true)
									Det_ReportError(MODULE_ID, INSTANCE_ID, 0x05, CANIF_E_INVALID_TXPDUID);
									#endif
									return E_NOT_OK;
								}
								else
								{
									/* [SWS_CANIF_00893] When CanIf_Transmit() is called with CanIfTxInfoPtr-
									>SduLength exceeding the maximum length of the PDU referenced by CanIfTxSduId:
									• SduLength > 8 if the Can_IdType indicates a classic CAN frame
									• SduLength > 64 if the Can_IdType indicates a CAN FD frame

									CanIf shall report development error code CANIF_E_DATA_LENGTH_MISMATCH to
									the Det_ReportError service of the DET. */
									if(CanIfTxInfoPtr->SduLength > 8)
									{
										#if(CanIfPublicDevErrorDetect == true)
	                                    Det_ReportError(MODULE_ID, INSTANCE_ID, 0x05, CANIF_E_DATA_LENGTH_MISMATCH);
										#endif
										return E_NOT_OK;
									}
									else
									{
										/* [SWS_CANIF_00318] The service CanIf_Transmit() shall map the parameters
										of the data structure:
										• the L-SDU handle (CanIfTxSduId) refers to (CanID, HTH/HRH of the CAN Controller)
										• and the CanIfTxInfoPtr which specifies length and data pointer of the Transmit Request
										to the corresponding CanDrv and call the function Can_Write(Hth, *PduInfo)
										Note: CanIfTxInfoPtr is a pointer to a L-SDU user memory, CAN Identifier, L-SDU
										handle and DLC */
										Can_HwHandleType Hth = TxPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthIdSymRef->CanObjectId;

										const Can_PduType* PduInfoPtr = &(Can_PduType){
											.swPduHandle = CanIfTxSduId,
											.length = CanIfTxInfoPtr->SduLength,
											.id = TxPduPtr->CanIfTxPduCanId,
											.sdu = CanIfTxInfoPtr->SduDataPtr};

										/* [SWS_CANIF_00162] d If the call of Can_Write() returns E_OK the transmit request
										service CanIf_Transmit() shall return E_OK. c()
										Note: If the call of Can_Write() returns CAN_NOT_OK, then the transmit request
										service CanIf_Transmit() shall return E_NOT_OK. If the transmit request service
										CanIf_Transmit() returns E_NOT_OK, then the upper layer module is responsible
										to repeat the transmit request. */
										Can_WriteReturn = Can_Write(Hth, PduInfoPtr);

										if(Can_WriteReturn == CAN_OK)
										{
											return E_OK;
										}
										else if(Can_WriteReturn == CAN_BUSY)
										{
											#if(CanIfPublicTxBuffering == true)
											if(CanIf_TxBufferEnqueue(TxPduPtr, PduInfoPtr) == E_OK)
											{
												return E_OK;
											}
											else
											{
												return E_NOT_OK;
											}
											#else
											return E_NOT_OK;
											#endif
										}
										else
										{
											return E_NOT_OK;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}




















































/*********************************************************************************************************************************
 Service name:                                       CanIf_ReadRxPduData
 Service ID[hex]:                                               0x06
 Sync/Async:                                               Synchronous
 Reentrancy:                                                 Non Reentrant
 Parameters (in):                                           CanIfRxSduId    -->Receive L-SDU handle specifying the corresponding 
																								CAN L-SDU ID and implicitly the CAN Driver instance
																								as well as the corresponding CAN controller device.
Parameters (inout):                                          None
 Parameters (out):                                       CanIfRxInfoPtr   -->Pointer to a structure with CAN L-SDU related data:
																								DLC and pointer to CAN L-SDU buffer including the
																								MetaData of dynamic L-PDUs.
 Return value:                                              Std_ReturnType
 Description:
					This service provides the CAN DLC and the received data of the requested
					CanIfRxSduId to the calling upper layer.
 *******************************************************************************************************************************/
Std_ReturnType CanIf_ReadRxPduData(PduIdType CanIfRxSduId,PduInfoType* CanIfRxInfoPtr){

















































    return E_OK;
}


/*********************************************************************************************************************************
 Service name:                                       CanIf_ReadTxNotifStatus
 Service ID[hex]:                                               0x07
 Sync/Async:                                               Synchronous
 Reentrancy:                                                 Non Reentrant
 Parameters (in):                                           CanIfTxSduId    -->L-SDU handle to be transmitted.
																								This handle specifies the corresponding CAN L-
																								SDU ID and implicitly the CAN Driver instance as
																								well as the corresponding CAN controller device.
Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                              CanIf_NotifStatusType
 Description:
					This service returns the confirmation status (confirmation occurred or not)
					of a specific static or dynamic CAN Tx L-PDU, requested by the
					CanIfTxSduId.
 *******************************************************************************************************************************/

CanIf_NotifStatusType CanIf_ReadTxNotifStatus(PduIdType CanIfTxSduId){
















































    return CANIF_TX_RX_NOTIFICATION;
}


/*********************************************************************************************************************************
 Service name:                                       CanIf_ReadRxNotifStatus
 Service ID[hex]:                                               0x08
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Non Reentrant
 Parameters (in):                                            CanIfRxSduId    -->Receive L-SDU handle specifying the corresponding 
																								CAN L-SDU ID and implicitly the CAN Driver instance
																								as well as the corresponding CAN controller device.
Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                              CanIf_NotifStatusType
 Description:
					This service returns the indication status (indication occurred or not) of a
					specific CAN Rx L-PDU, requested by the CanIfRxSduId.
 *******************************************************************************************************************************/

CanIf_NotifStatusType CanIf_ReadRxNotifStatus(PduIdType CanIfRxSduId){





















































    return CANIF_TX_RX_NOTIFICATION;

}

/*********************************************************************************************************************************
 Service name:                                         CanIf_SetPduMode
 Service ID[hex]:                                               0x09
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Non Reentrant
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
																    PduModeRequest-->Requested PDU mode change
Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                              Std_ReturnType
 Description:
					This service sets the requested mode at the L-PDUs of a predefined logical PDU channel.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_SetPduMode(uint8 ControllerId, CanIf_PduModeType PduModeRequest){

















































    return E_OK;

}

/*********************************************************************************************************************************
 Service name:                                         CanIf_GetPduMode
 Service ID[hex]:                                               0x0a
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant (Not for the same channel)
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
Parameters (inout):                                          None
 Parameters (out):                                            PduModePtr  -->Pointer to a memory location, where the current
																								mode of the logical PDU channel will be stored.
 Return value:                                              Std_ReturnType
 Description:
					This service reports the current mode of a requested PDU channel.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_GetPduMode(uint8 ControllerId, CanIf_PduModeType* PduModePtr)
{

	/*  CanIf must be initialized after Power ON */
	if(CanIfState == CANIF_UNINIT)
	{
		return E_NOT_OK;
	}
	else
	{
		/* [SWS_CANIF_00346] If CanIf_GetPduMode() is called with invalid ControllerId, CanIf shall report development error code CANIF_E_PARAM_CONTROLLERID
		to the Det_ReportError service of the DET module.  */
		if(ControllerId >= NUMBER_OF_CONTROLLERS)
		{
			#if(CanIfPublicDevErrorDetect == true)
            Det_ReportError(MODULE_ID, INSTANCE_ID, 0x0A, CANIF_E_PARAM_CONTROLLERID);

			#endif
			return E_NOT_OK;
		}
		else
		{
			/*[SWS_CANIF_00657] If CanIf_GetPduMode() is called with invalid PduModePtr,
			CanIf shall report development error code CANIF_E_PARAM_POINTER to the Det_ReportError
			service of the DET module.*/
			if(PduModePtr == NULL)
			{
				#if(CanIfPublicDevErrorDetect == true)
	            Det_ReportError(MODULE_ID, INSTANCE_ID, 0x0A, CANIF_E_PARAM_POINTER);
				#endif
				return E_NOT_OK;
			}
			else
			{
				*PduModePtr = CanIfPduMode[ControllerId];
			}
		}
	}

	return E_NOT_OK;
}


/*********************************************************************************************************************************
 Service name:                                         CanIf_SetBaudrate
 Service ID[hex]:                                               0x27
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant for different ControllerIds. Non reentrant for the same ControllerId.
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
																	BaudRateConfigID-->references a baud rate configuration by ID (see Can-
																									ControllerBaudRateConfigID)
Parameters (inout):                                          None
 Parameters (out):                                            PduModePtr  -->Pointer to a memory location, where the current
																								mode of the logical PDU channel will be stored.
 Return value:                                              Std_ReturnType
 Description:
					This service shall set the baud rate configuration of the CAN controller.
					Depending on necessary baud rate modifications the controller might
					have to reset.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_SetBaudrate(uint8 ControllerId,uint16 BaudRateConfigID) {











































    return E_OK;

}



/*********************************************************************************************************************************
 Service name:                                         CanIf_TxConfirmation
 Service ID[hex]:                                               0x13
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                           CanTxPduId      -->L-PDU handle of CAN L-PDU successfully transmitted.
																								This ID specifies the corresponding CAN L-PDU ID
																								and implicitly the CAN Driver instance as well as the
																								corresponding CAN controller device.
Parameters (inout):                                          None
Parameters (out):                                             None
Return value:                                                   None
Description:
					This service confirms a previously successfully processed transmission of a CAN TxPDU.
 *******************************************************************************************************************************/

void CanIf_TxConfirmation(PduIdType CanTxPduId){















































}


/*********************************************************************************************************************************
 Service name:                                         CanIf_RxIndication
 Service ID[hex]:                                               0x14
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                           Mailbox      -->Identifies the HRH and its corresponding CAN Controller.
																	PduInfoPtr  -->Pointer to the received L-PDU.
Parameters (inout):                                          None
Parameters (out):                                             None
Return value:                                                   None
Description:
					This service indicates a successful reception of a received CAN Rx L-PDU
					to the CanIf after passing all filters and validation checks.
 *******************************************************************************************************************************/

void CanIf_RxIndication(const Can_HwType* Mailbox,const PduInfoType* PduInfoPtr){
















































}


/*********************************************************************************************************************************
 Service name:                                         CanIf_ControllerBusOff
 Service ID[hex]:                                               0x14
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                            ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
Parameters (inout):                                          None
Parameters (out):                                             None
Return value:                                                   None
Description:
					This service indicates a Controller BusOff event referring to the corresponding
					CAN Controller with the abstract CanIf ControllerId.
 *******************************************************************************************************************************/

void CanIf_ControllerBusOff(uint8 ControllerId){


















































}


/*********************************************************************************************************************************
 Service name:                                         CanIf_ControllerModeIndication
 Service ID[hex]:                                               0x17
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                            ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
                                                                    ControllerMode  -->Mode to which the CAN controller transitioned
Parameters (inout):                                          None
Parameters (out):                                             None
Return value:                                                   None
Description:
					This service indicates a controller state transition referring to the corresponding
					CAN controller with the abstract CanIf ControllerId.
 *******************************************************************************************************************************/

void CanIf_ControllerModeIndication(uint8 ControllerId,CanIf_ControllerModeType ControllerMode){




































































}





void main(void)
{
}

