/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    eprom.h

  @Description
        This file contains the declarations for the EPROM module functions.
        The EPROM functions are defined in eprom.c source file.

  @Versioning:
 	 Cristian Fatu - 2018/06/29 - Initial release, DMMShield Library

 */
/* ************************************************************************** */

#ifndef _EPROM_H    /* Guard against multiple inclusion */
#define _EPROM_H

#include "stdint.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */

// wait for dataready timeout counter threshold
#define EPROM_CNTTIMEOUT 0x00010000


// OpCodes
#define EPROM_OPCODE_ERASE  0x03
#define EPROM_OPCODE_EWDS   0x00
#define EPROM_OPCODE_EWEN   0x00
#define EPROM_OPCODE_READ   0x02
#define EPROM_OPCODE_WRITE  0x01


// Addresses 

#define ADR_EPROM_CALIB     31
#define ADR_EPROM_FACTCALIB 147
#define ADR_EPROM_SERIALNO  140

#define EPROM_MAGIC_NO      0x23


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
// EPROM initialization
void EPROM_Init();
// EPROM data access
void EPROM_ReadWords(uint8_t bAddress, uint16_t *prgVals, int cwVals);
uint8_t EPROM_WriteWords(uint8_t bAddress, uint16_t *prgVals, int cwVals);
// some EPROM implemented functions:
void EPROM_Erase(uint8_t bAddress);
void EPROM_WriteDisable();
void EPROM_WriteEnable();

//#ifdef __cpluspl
//extern "C" {
//#endif



    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************



    /* Provide C++ Compatibility */
//#ifdef __cplusplus
//}
//#endif
#endif /* _EPROM_H */

/* *****************************************************************************
 End of File
 */
