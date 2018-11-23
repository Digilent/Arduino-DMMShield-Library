/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    dmmcmd.h

  @Description
        This file contains the declaration for the functions of the DMMCMD module.
 */
/* ************************************************************************** */

#ifndef _DMMCMD_H    /* Guard against multiple inclusion */
#define _DMMCMD_H

#include "HardwareSerial.h"

#define CMD_MAX_LEN	255
//#ifdef __cplusplus
//extern "C" {
//#endif

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************
uint8_t DMMCMD_Init(HardwareSerial* phwSerial);
void DMMCMD_CheckForCommand();

void DMMCMD_ProcessIndividualCmd(char *szCmd);




    /* Provide C++ Compatibility */
//#ifdef __cplusplus
//}
//#endif
#endif /* _DMMCMDJA_H */

/* *****************************************************************************
 End of File
 */
