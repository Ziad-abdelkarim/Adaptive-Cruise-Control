/*******************************************************************************************************************************
FileName:                                                       CanSM.h
AUTOSAR Version:                                          4.2.2
******************************************************************************************************************************/
#ifndef DET_STUB_H
#define DET_STUB_H

/******************************************************************************************************************************
 **                                                                        Includes                                                                             **
 ******************************************************************************************************************************/
//#include "CanIf_Types.h"
//#include "CanIf_Cfg.h"
#include "Can_GeneralTypes.h"

/*******************************************************************************************************************************
**                                                                        Defines                                                                                **
********************************************************************************************************************************/



/*******************************************************************************************************************************
**                                                                         Variables                                                                             **
********************************************************************************************************************************/


/*******************************************************************************************************************************
**                                                                  Function prototypes                                                                       **
********************************************************************************************************************************/
 Std_ReturnType Det_ReportError(uint16 ModuleId,uint8 InstanceId,uint8 ApiId,uint8 ErrorId);


#endif /*DET_STUB_H*/