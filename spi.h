/* ************************************************************************** */
/* ************************************************************************** */
/** Descriptive File Name

  @Company
 Digilent

  @File Name
    spi.h

  @Description
        This file contains the declaration for the functions of SPI module.
        The SPI functions are defined in spi.c source file.

  @Author
    Cristian Fatu 
    cristian.fatu@digilent.ro

  @Versioning:
 	 Cristian Fatu - 2018/06/29 - Initial release, DMMShield Library

 */
/* ************************************************************************** */

#ifndef _SPI_H    /* Guard against multiple inclusion */
#define _SPI_H

#include "stdint.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
#define SPI_CLK_DELAY   1   // the parameter used in delay functions in order to implement a clock phase.


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Internal low level functions                                      */
/* ************************************************************************** */
/* ************************************************************************** */
// SPI Initialization
void SPI_Init();

// SPI Transfer
uint8_t SPI_CoreTransferBits(uint8_t bVal, uint8_t cbBits);
uint8_t SPI_CoreTransferByte(uint8_t bVal);


#endif /* _SPIJA_H */

/* *****************************************************************************
 End of File
 */
