/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    gpio.c

  @Description
        This file groups the functions that implement GPIO module.
 Basically there is one initialization function that initializes the digital pins involved in DMMShield.
       		

  @Author
    Cristian Fatu 
    cristian.fatu@digilent.ro
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */

#include "stdint.h"
#include "gpio.h"

/* ************************************************************************** */

/***	GPIO_Init
**
**	Parameters:
**		
**
**	Return Value:
**		
**
**	Description:
**		This function initializes the digital pins used by DMMShield: 
**      The following digital pins are configured as digital outputs: SPI_CLK, SPI_MOSI, CS_EPROM, CS_DMM.
**      The following digital pins are configured as digital inputs: SPI_MISO.
**      The CS_EPROM and CS_DMM pins are deactivated.
**      This function is not intended to be called by user, as it is an internal low level function.
**      This function is called by SPI_Init().
**      The function guards against multiple calls using a static flag variable.
**          
*/
void GPIO_Init()
{
    static uint8_t fInitialized = 0;
    if(!fInitialized)
    {
        // Configure SPI signals as digital outputs.
		pinMode(PIN_SPI_CLK, OUTPUT);
		pinMode(PIN_SPI_MOSI, OUTPUT);
        // Configure SPI signals as digital inputs.
		pinMode(PIN_SPI_MISO, INPUT);
        // configure relays as digital output
		pinMode(PIN_RLD, OUTPUT);
		pinMode(PIN_RLU, OUTPUT);
		pinMode(PIN_RLI, OUTPUT);
        
        // Configure DMM Slave Select as digital output.
		pinMode(PIN_SPI_SS, OUTPUT);
        // // Deactivate CS_DMM
        GPIO_SetValue_CS_DMM(1); 
        
        // Configure EPROM Slave Select as digital output.
		pinMode(PIN_ESPI_SS, OUTPUT);
        
        // Deactivate EPROM SS
        GPIO_SetValue_CS_EPROM(0); 
  
        

        
        fInitialized = 1;
    }
}
/* *****************************************************************************
 End of File
 */
