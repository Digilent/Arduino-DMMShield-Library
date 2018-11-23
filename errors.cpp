/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    errors.c

  @Description
        This module groups the error related functions.


  @Author
    Cristian Fatu 
    cristian.fatu@digilent.ro

  @Versioning:
 	 Cristian Fatu - 2018/06/29 - Initial release, DMMShield Library
  
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "HardwareSerial.h"

#include "stdint.h"
#include "errors.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */
#define PREFIX_SIZE 10

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Global Variables                                                  */
/* ************************************************************************** */
/* ************************************************************************** */
char szLastError[MSG_ERROR_SIZE];
HardwareSerial *pSerialErr; // serial interface to be used

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void ERRORS_Init(HardwareSerial *phwSerial)
{
    pSerialErr = phwSerial;
}

/* ------------------------------------------------------------ */
/***    ERRORS_PrintMessageString
**
**	Synopsis:
**		
**
**	Parameters:
**      uint8_t bErrCode  - The error code for which the error string is requested/needed to be transmitted further to the user
**      char *szContent         - The characters string acting as content for some of the error messages
**      char *pSzErr            - String to receive the error meaning
**		
**
**	Return Values:
**      ERRVAL_SUCCESS                   0   - success
**      ERRVAL_CMD_MISSINGCODE        0xF9   - The provided code is not among accepted values
**
**	Errors:
**		none
**
**	Description:
**		This function copies in the szLastError the error message corresponding to the provided error code, 
**      for all error codes except ERRVAL_SUCCESS.
**      Then it copies in the pSzErr the properly prefixed message.
**      Therefore is important that the caller of this function allocates enough space in pSzErr (70 characters).
**      Some of the error messages include the string provided in the szContent parameter. The parameter is ignored for the other error messages.
**      If the error code is among the defined ones, a specific error string is copied in the pSzErr
**      and ERRVAL_SUCCESS is returned.
**      If the error is not among the defined ones, ERRVAL_CMD_MISSINGCODE is returned and pSzErr is not altered.
**		
*/
void ERRORS_PrintMessageString(uint8_t bErrCode, char *szContent)
{
    switch(bErrCode)
    {
        case ERRVAL_SUCCESS: 
//          the message is in pSzErr string
            pSerialErr->println(szContent);
            break;
        case ERRVAL_DMM_CFGVERIFY:
            pSerialErr->println(F("DMM Configuration verify error"));
            break;
        case ERRVAL_EPROM_WRTIMEOUT:
            pSerialErr->println(F("EPROM write data ready timeout"));
            break;
        case ERRVAL_EPROM_CRC:
            pSerialErr->println(F("Invalid EPROM checksum"));
            break;
        case ERRVAL_EPROM_MAGICNO:
            pSerialErr->println(F("Invalid EPROM magic number"));  
            break;
        case ERRVAL_DMM_IDXCONFIG:
            pSerialErr->println(F("Invalid scale index"));
            break;
        case ERRVAL_DMM_VALIDDATATIMEOUT:
            pSerialErr->println(F("Valid DMM data timeout"));  
            break;
        case ERRVAL_CALIB_NANDOUBLE:
            pSerialErr->println(F("The provided reference value has no valid value."));  
            break;
        case ERRVAL_CMD_WRONGPARAMS:
            pSerialErr->println(F("The expected parameters were not provided on the UART command."));  
            break;
        case ERRVAL_CMD_MISSINGCODE:
            pSerialErr->println(F("The provided code is not among accepted values."));  
            break;
        case ERRVAL_EPROM_VERIFY:
            pSerialErr->println(F("EPROM verify error."));  
            break;
        case ERRVAL_EPROM_ADDR_VIOLATION:
            pSerialErr->println(F("EPROM address violation: attempt to write over system data."));  
            break;            
        case ERRVAL_CMD_VALWRONGUNIT:
            pSerialErr->print(F("The provided value "));
			pSerialErr->print(szContent);  
            pSerialErr->println(F(" has a wrong measure unit."));
            break;          
        case ERRVAL_CMD_VALFORMAT:
            pSerialErr->print(F("The provided value "));
			pSerialErr->print(szContent);  
            pSerialErr->println(F(" has a wrong format."));
            break;       
        case ERRVAL_DMM_MEASUREDISPERSION:
            // szLastError already contains the error message
            pSerialErr->println(ERRORS_GetszLastError());
            break;   
        case ERRVAL_CALIB_MISSINGMEASUREMENT:
            pSerialErr->println(F("A measurement must be performed before calling the finalize calibration."));
            break;       

    }

    
}
/* ------------------------------------------------------------ */
/***    ERRORS_GetszLastError
**
**	Synopsis:
**		
**
**	Parameters:
**      none
**		
**	Return Values:
**      char szLastError - last saved error code
**
**	Errors:
**		none
**
**	Description:
**		This function returns the last saved error code.
**      
**		
*/
char *ERRORS_GetszLastError()
{
    return szLastError;
}


/* *****************************************************************************
 End of File
 */
