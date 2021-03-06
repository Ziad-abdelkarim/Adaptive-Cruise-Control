/********************************************************************************************************************************
 **
 FileName:                   CanSM.c
 AUTOSAR Version:            4.2.2
                                                                                                                                                                          **
 ********************************************************************************************************************************/
/********************************************************************************************************************************
 **                                                          Includes                                                                                                **
 ********************************************************************************************************************************/
#include <CanSM_Stub.h>

/********************************************************************************************************************************
 **                                                       Global Variables                                                                                       **
 ********************************************************************************************************************************/










 /********************************************************************************************************************************
**                                                        Local Functions                                                                                        **
*********************************************************************************************************************************/












/********************************************************************************************************************************
**                                                      Functions  defination                                                                        **
*********************************************************************************************************************************/
/*********************************************************************************************************************************
 Service name:                                                  CanSM_ControllerBusOff
 Service ID[hex]:                                               0x04
 Sync/Async:                                                    Synchronous
 Reentrancy:                                                    Reentrant
 Parameters (in):                                               ControllerId      --> CAN controller, which detected a bus-off event
 Parameters (out):                                              None
 Return value:                                                  None
 Description:                                                   This callback function notifies the CanSM about a bus-off event on a certain CAN
                                                                controller, which needs to be considered with the specified bus-off recovery
                                                                handling for the impacted CAN network.
 *******************************************************************************************************************************/


void CanSM_ControllerBusOff(uint8 ControllerId)
{

}
