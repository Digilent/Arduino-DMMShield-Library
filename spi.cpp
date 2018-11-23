/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    spi.c

  @Description
        This file groups the functions that implement the SPI module.
        The hardware interface SPI module of PIC32 is not used, instead bit bang SPI is implemented.
        The module is using pins definitions from config.h.
        The module implements the data communication layer for DMM and EPROM modules, each of these modules 
        handling the specific Slave Select pin.
        The "Internal low level functions" section groups functions that are called from other modules (DMM and EPROM). 
        The "Local functions" section groups low level functions that are only called from within current module. 
        All SPI functions are not intended to be called by user, instead user should call functions from DMM and EPROM modules.

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

#include "gpio.h"
#include "spi.h"
#include "utils.h"

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Internal low level functions Functions                            */
/* ************************************************************************** */
/* ************************************************************************** */

/***	SPI_Init
**
**	Parameters:
**		
**
**	Return Value:
**		
**
**	Description:
**		This function calls GPIO_Init() to initialize the digital pins used by DMMShield: 
**      The following digital pins are configured as digital outputs: SPI_CLK, SPI_MOSI, CS_EPROM, CS_DMM.
**      The following digital pins are configured as digital inputs: SPI_MISO.
**      The CS_EPROM and CS_DMM pins are deactivated.
**      This function is not intended to be called by user, as it is an internal low level function.
**      This function is called by DMM_Init() and EPROM_Init().
**      The function guards against multiple calls using a static flag variable.
**          
*/
void SPI_Init()
{
    static uint8_t fInitialized = 0;
    if(!fInitialized)
    {
        GPIO_Init();    // GPIO_Init is protected against multiple calls
        fInitialized = 1;
    }
}

/***	SPI_CoreTransferByte
**
**	Parameters:
**		uint8_t bVal  - the byte to be transmitted over SPI
**
**	Return Value:
**		uint8_t       - the byte received over SPI	
**
**	Description:
**		This function transfers one byte over SPI. 
**      It transmits the bVal byte and returns the received byte.
**      The MSB bit is transfered first.
**      It calls SPI_CoreTransferBits for the 8 bits of the provided byte.
**      This function does not handle Slave Select (SS) pins. 
**      This function is not intended to be called by user, as it is an internal low level function.
**      It is called by functions from DMM and EPROM modules.
**          
*/
uint8_t SPI_CoreTransferByte(uint8_t bVal)
{
    return SPI_CoreTransferBits(bVal, 8);
}

/***	SPI_CoreTransferBits
**
**	Parameters:
**		uint8_t bVal      - the byte containing bits to be transmitted over SPI
**      uint8_t cbBits    - the number of bits to be transmitted over SPI. It should <= 8.
**
**	Return Value:
**		uint8_t           - the byte containing bits received over SPI	
**
**	Description:
**		This function implements basic bit bang SPI transfer. 
**      It transmits the number of bits specified by the bVal parameter and returns the received bits.
**      The cbBits parameter must be <= 8. The bits to be transmitted are cbBits placed on LSB positions of bVal. 
**      The first bit to be transmitted is the MSB bit.
**      If less than 8 bits are transmitted, the bits on MSB positions are ignored and  
**      the returned byte will contain 0 value on the MSB positions. 
**      It uses SPI_CLK_DELAY definition to determine clock period (frequency).
**      This function does not handle Slave Select (SS) pins.
**      This function is not intended to be called by user, as it is an internal low level function.
**      It is called by SPI_CoreTransferByte and functions from DMM and EPROM modules.
**          
*/

uint8_t SPI_CoreTransferBits(uint8_t bVal, uint8_t cbBits)
{
	int		idxBit;
	uint8_t bRx = 0;
    uint8_t bTx;

	for(idxBit = 0; idxBit < cbBits;  idxBit++) 
    {
        // for each bit to be transmitted, starting with the most significant
        // set MOSI in according to the value of the specific bit
		bTx = ((bVal & (1<<(cbBits - idxBit - 1)))>>(cbBits - idxBit - 1));

		GPIO_SetValue_MOSI(bTx);	// set the MOSI pin

        GPIO_SetValue_CLK(1);		// set the clock line

//		DelayAprox10Us(SPI_CLK_DELAY);
        
        // retrieve the MISO value in the return byte
		
        bRx <<= 1;
        bRx |= GPIO_Get_MISO() ? 1: 0;
  		GPIO_SetValue_MOSI(bTx);	// set the MOSI pin

        GPIO_SetValue_CLK(0);	// clear the clock line

		
//        DelayAprox10Us(SPI_CLK_DELAY);
	}


	return bRx;
}



/* *****************************************************************************
 End of File
 */
