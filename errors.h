/* ************************************************************************** */
/** Descriptive File Name

  @Company
 Digilent

  @File Name
    errors.h

  @Description
        This file contains the declaration for the error message function along with error codes definitions.
        The ERRORS module functions are defined in errors.c source file.

  @Author
    Cristian Fatu 
    cristian.fatu@digilent.ro

  @Versioning:
 	 Cristian Fatu - 2018/06/29 - Initial release, DMMShield Library

 */

#ifndef ERRORS_H
#define	ERRORS_H

#include "HardwareSerial.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
#define MSG_ERROR_SIZE      20

#define ERRVAL_SUCCESS                  0       // success

#define ERRVAL_EPROM_WRTIMEOUT          0xFF    // EPROM write data ready timeout
#define ERRVAL_EPROM_CRC                0xFE    // wrong CRC when reading data from EPROM
#define ERRVAL_EPROM_MAGICNO            0xFD    // wrong Magic No. when reading data from EPROM

#define ERRVAL_DMM_IDXCONFIG            0xFC    // wrong scale index
#define ERRVAL_CALIB_NANDOUBLE          0xFB    // not a number double value

#define ERRVAL_DMM_VALIDDATATIMEOUT     0xFA    // valid data DMM timeout
#define ERRVAL_CMD_WRONGPARAMS          0xF9    // wrong parameters when sending UART commands
#define ERRVAL_CMD_MISSINGCODE          0xF8    // the provided code is not among accepted values
#define ERRVAL_EPROM_VERIFY             0xF7    // eprom verify error
#define ERRVAL_EPROM_ADDR_VIOLATION     0xF6    // EPROM write address violation: attempt to write over system data
#define ERRVAL_DMM_CFGVERIFY            0xF5    // DMM Configuration verify error
#define ERRVAL_CMD_VALWRONGUNIT         0xF4    // The provided value has a wrong measure unit.
#define ERRVAL_CMD_VALFORMAT            0xF2    // The numeric value cannot be extracted from the provided string.
#define ERRVAL_DMM_MEASUREDISPERSION    0xF1    // The calibration measurement dispersion exceeds accepted range
#define ERRVAL_CALIB_MISSINGMEASUREMENT 0xF0    // A measurement must be performed before calling the finalize calibration.

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************
void ERRORS_Init(HardwareSerial *phwSerial);
void ERRORS_PrintMessageString(uint8_t bErrCode, char *szContent);
char *ERRORS_GetszLastError();


/*
#ifdef	__cplusplus
}
#endif
*/
#endif	/* ERRORS_H */

