/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    serialno.c

  @Description
        This file groups the functions that implement the SERIALNO module.
        This module uses EPROM module in order to access serial number information stored in EPROM during manufacturing process.
        The "Interface functions" section groups functions that can also be called by user.
        The SERIALNO module provides functions for initialization and get serial number from EPROM.
        The module uses errors defined in ERRORS module.

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
#include <stdio.h>
#include <string.h>
#include "dmm.h"
#include "errors.h"
#include "eprom.h"
#include "serialno.h"
#include "utils.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Global Variables                          \t\t\t              */
/* ************************************************************************** */
/* ************************************************************************** */
SERIALNODATA serialNo;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/***	SERIALNO_Init
**
**	Parameters:
**		
**
**	Return Value:
**		
**
**	Description:
**		This function initializes the SERIALNO module. 
**      It calls the EPROM_Init() function to initialize the EPROM module.
**      
**          
*/
void SERIALNO_Init()
{
    EPROM_Init();
}


/***	SERIALNO_ReadSerialNoFromEPROM
**
**	Parameters:
**      none
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS              0       // success
**          ERRVAL_EPROM_CRC            0xFE    // wrong CRC when reading data from EPROM
**          ERRVAL_EPROM_MAGICNO        0xFD    // wrong Magic No. when reading data from EPROM
**
**	Description:
**		This function reads the serial number data from EPROM and stores the data in the serialNo global data structure.
**      In order to access the serial number string, use SERIALNO_GetSerialNo function.
**      The function returns ERRVAL_SUCCESS for success. 
**      The function returns ERRVAL_EPROM_MAGICNO when a wrong magic number was detected in the data read from EPROM. 
**      The function returns ERRVAL_EPROM_CRC when the checksum is wrong for the data read from EPROM. 
**            
*/
uint8_t SERIALNO_ReadSerialNoFromEPROM(char *pSzSerialNo)
{
	
    uint8_t bCrc, bCrcRead;
 
    // read serialNo structure
    EPROM_ReadWords(ADR_EPROM_SERIALNO, (uint16_t *)&serialNo, sizeof(serialNo)/2);

    // check CRC
    bCrcRead = serialNo.crc;
    serialNo.crc = 0;

    bCrc = GetBufferChecksum((uint8_t *)&serialNo, sizeof(serialNo));     
    
    serialNo.crc = bCrcRead;
//	strcpy(pSzSerialNo, "abcdefghijkl");
    
    if(serialNo.magic != EPROM_MAGIC_NO)
    {
        // missing magic number
        return ERRVAL_EPROM_MAGICNO;
    }
    if(serialNo.crc != bCrc)
    {
        // CRC error
        return ERRVAL_EPROM_CRC;
    }
    strncpy(pSzSerialNo, serialNo.rgchSN, SERIALNO_SIZE);   // copy 12 chars of serial number from serialNo to the destination string
    pSzSerialNo[SERIALNO_SIZE] = 0; // terminate string
	
    return ERRVAL_SUCCESS;
}


/* *****************************************************************************
 End of File
 */
