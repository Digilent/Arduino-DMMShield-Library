/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    eprom.c

  @Description
        This file groups the functions that implement the EPROM module.
        EPROM functions uses the functions defined in the SPI module as the data communication layer.
        EPROM module functions are used in order to implement a base layer for CALIB and SERIALNO modules.
        The EPROM is used to store system data: board serial number, user calibration data and factory calibration data.
        It can also be used by User to store other information in the unused EPROM memory locations.
        The "Interface functions" section groups functions that can also be called by User.
        In this section the EPROM module provides Initialization, data write and data read functions
        as well as implementations for Erase and Write Enable / Disable instructions.
        The EPROM write function EPROM_WriteWords prevents user from writing to addresses where system data is stored.
        The "Internal low level functions" section groups functions that are called from other modules (CALIB and SERIALNO). 
        They are not intended to be called by user.
        The "Local functions" section groups low level functions that are only called from within the current module. 
        The module is using pins definitions from config.h.


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
#include "eprom.h"
#include "errors.h"
#include "utils.h"

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local functions prototypes                                        */
/* ************************************************************************** */
/* ************************************************************************** */
void EPROM_StartBitOpAddr_Raw(uint8_t bOp, uint8_t bAddress);
uint8_t EPROM_WaitUntilReady_Raw();
uint16_t EPROM_Read_Raw(uint8_t bAddress);
uint8_t EPROM_Write_Raw(uint8_t bAddress, uint16_t wVal);
uint8_t EPROM_WriteWords_Raw(uint8_t bAddress, uint16_t *prgVals, int cwVals);

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/***	EPROM_Init
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Description:
**		This function initializes the EPROM module. 
**      It calls the SPI_Init() function to initialize the digital pins used by DMMShield.
**      This function is called by CALIB_Init() and SERIALNO_Init().
**      The function guards against multiple calls using a static flag variable.
**      
**          
*/
void EPROM_Init()
{
    static uint8_t fInitialized = 0;
    if(!fInitialized)
    {
        SPI_Init();
        fInitialized = 1;
    }
}


/* ************************************************************************** */
/***	EPROM_ReadWords
**
**	Parameters:
**      uint8_t bAddress		- the word address of the EPROM memory location to be read 
**      uint16_t *prgVals       - pointer to an array of 16 bits values, to store the values read from EPROM
**      int cwVals              - number of 16 bits values to be read in EPROM
**
**	Return Value:
**		none
**
**	Description:
**		This function reads the specified number of words (16 bit values) from the specified EPROM word address into the specified buffer.  
**            
*/
void EPROM_ReadWords(uint8_t bAddress, uint16_t *prgVals, int cwVals)
{
    int i;
    
    for(i = 0; i < cwVals; i++)
    {
        prgVals[i] = EPROM_Read_Raw(bAddress + i);
    }
}



/* ************************************************************************** */
/***	EPROM_WriteWords
**
**	Parameters:
**      uint8_t bAddress		- the word address of the EPROM memory location to be written
**      uint16_t *prgVals       - pointer to an array of words (16 bits values), to be written in EPROM
**      int cwVals              - number of words to be written in EPROM
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_EPROM_ADDR_VIOLATION     0xF6    // EPROM write address violation: attempt to write over system data
**          ERRVAL_EPROM_WRTIMEOUT          0xFF    // EPROM write data ready timeout
**
**	Description:
**		This function writes the specified number of words (16-bit values) in EPROM, at the specified word address.
**      It is mandatory to enable the write operation before sending the data to EPROM, by calling the EPROM_WriteEnable() function. 
**      The function returns  ERRVAL_EPROM_ADDR_VIOLATION if write is attempted over the system reserved areas of EPROM.
**      Otherwise, the function returns ERRVAL_SUCCESS for success or ERRVAL_EPROM_WRTIMEOUT when EPROM is 
**      not answering with the write successful message. 
**            
*/
uint8_t EPROM_WriteWords(uint8_t bAddress, uint16_t *prgVals, int cwVals)
{
    uint8_t bResult;
    if(bAddress + cwVals - 1 >= (uint8_t)ADR_EPROM_CALIB || bAddress >= (uint8_t)ADR_EPROM_CALIB)
    {
        bResult = ERRVAL_EPROM_ADDR_VIOLATION;
    }   
    else
    {
        bResult = EPROM_WriteWords_Raw(bAddress, prgVals, cwVals);
    }
    return bResult;
}

// Implementation of EPROM instructions

/* ************************************************************************** */
/***	EPROM_WriteEnable
**
**	Parameters:
**      none
**
**	Return Value:
**      none
**
**	Description:
**		This function implements the EWEN (Write Enable) EPROM instruction.  
**      Call this function before any EPROM write operations.
**            
*/
void EPROM_WriteEnable()
{
    GPIO_SetValue_CS_EPROM(1); // Activate CS_EPROM

    // some delay
//    DelayAprox10Us(SPI_CLK_DELAY);

    // Send instruction code
    EPROM_StartBitOpAddr_Raw(EPROM_OPCODE_EWEN, 0xC0);

    GPIO_SetValue_CS_EPROM(0); // Deactivate CS_EPROM

	GPIO_SetValue_MOSI(0);	// clear the MOSI GPIO pin
    
    // some delay
    //    DelayAprox10Us(SPI_CLK_DELAY);
       
}
/* ************************************************************************** */
/***	EPROM_WriteDisable
**
**	Parameters:
**      none
**
**	Return Value:
**      none
**
**	Description:
**		This function implements the EWDS (Write Disable) EPROM instruction.  
**      Use this function to protect the values written in EPROM 
**      against subsequent writes.
**            
*/
void EPROM_WriteDisable()
{
    GPIO_SetValue_CS_EPROM(1); // Activate CS_EPROM

    // Send instruction code
    EPROM_StartBitOpAddr_Raw(EPROM_OPCODE_EWDS, 0x00);
    
    GPIO_SetValue_CS_EPROM(0); // Deactivate CS_EPROM
    
	GPIO_SetValue_MOSI(0);	// clear the MOSI GPIO pin
    
    // some delay
    //    DelayAprox10Us(SPI_CLK_DELAY);
}

/* ************************************************************************** */
/***	EPROM_Erase
**
**	Parameters:
**      uint8_t bAddress - the word address of the EPROM memory location to be erased
**
**	Return Value:
**      none
**
**	Description:
**		This function implements the ERASE EPROM instruction that erases one word.
**      Call this function in order to force all 16 bits of the specified address to 1.
**            
*/
void EPROM_Erase(uint8_t bAddress)
{
    GPIO_SetValue_CS_EPROM(1); // Activate CS_EPROM

    // Send instruction code
    EPROM_StartBitOpAddr_Raw(EPROM_OPCODE_ERASE, bAddress);

    GPIO_SetValue_CS_EPROM(0); // Deactivate CS_EPROM
	GPIO_SetValue_MOSI(0);	// clear the MOSI GPIO pin
    
    // some delay
    //    DelayAprox10Us(SPI_CLK_DELAY);
}



/* ************************************************************************** */
/***	EPROM_StartBitOpAddr_Raw
**
**	Parameters:
**      uint8_t bOp             - a byte whose 2 LSBs contain the operation code.
**      uint8_t bAddress        - a byte containing 8-bit EPROM address.
**
**	Return Value:
**		none
**
**	Description:
**		This utility function transmits over SPI the initial 11 bits of each EPROM command: 
**      the first bit is always set to 1, then the 2-bit operation code, then the 8 address bits.
**            
*/
void EPROM_StartBitOpAddr_Raw(uint8_t bOp, uint8_t bAddress)
{
    uint8_t bStartBitOpcode = (1 << 2) | (bOp & 3);
    SPI_CoreTransferBits(bStartBitOpcode, 3);        // transfer 3 bits (start bit, 2 bits opcode)
    SPI_CoreTransferByte(bAddress);               // transfer full Address byte
}



/* ************************************************************************** */
/***	EPROM_WaitUntilReady_Raw
**
**	Parameters:
**      none
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_EPROM_WRTIMEOUT          0xFF    // EPROM write data ready timeout
**
**	Description:
**		This function waits until EPROM answers with data ready state.  
**      It is usually called to complete an EPROM write functionality. 
**      This is a private function, the user shouldn't call it. 
**      The function returns ERRVAL_SUCCESS for success or ERRVAL_EPROM_WRTIMEOUT when eprom is 
**      not answering with write successful message.
**            
*/
uint8_t EPROM_WaitUntilReady_Raw()
{
    uint8_t bResult = ERRVAL_SUCCESS;
    // wait for data ready timeout counter 
    unsigned long cntTimeout = 0;
    //    DelayAprox10Us(SPI_CLK_DELAY);    
    GPIO_SetValue_CS_EPROM(1); // Activate CS_EPROM
    //    DelayAprox10Us(SPI_CLK_DELAY);
    // check the wait for data ready timeout counter against threshold
    while((!GPIO_Get_MISO()) && (cntTimeout++ < EPROM_CNTTIMEOUT)); // wait for ready

    if(cntTimeout >= EPROM_CNTTIMEOUT)
    {
        bResult = ERRVAL_EPROM_WRTIMEOUT;
    }

    //    DelayAprox10Us(SPI_CLK_DELAY);
    
    GPIO_SetValue_CS_EPROM(0); // Deactivate CS_EPROM
    return bResult;
}


/* ************************************************************************** */
/***	EPROM_Read_Raw
**
**	Parameters:
**      uint8_t bAddress		- the EPROM address from where the values will be read
**
**	Return Value:
**		uint16_t                - the 16-bit value read from EPROM 
**
**	Description:
**		This function reads the 16 bit value from the specified address in EPROM.  
**  
**            
*/
uint16_t EPROM_Read_Raw(uint8_t bAddress)
{
    uint16_t wVal;
    GPIO_SetValue_CS_EPROM(1); // Activate CS_EPROM

    EPROM_StartBitOpAddr_Raw(EPROM_OPCODE_READ, bAddress);
	GPIO_SetValue_MOSI(0);	// clear the MOSI GPIO pin
    wVal = SPI_CoreTransferByte(0);                  // MSByte
    wVal = (wVal << 8) | SPI_CoreTransferByte(0);   // LSByte

	GPIO_SetValue_MOSI(0);	// clear the MOSI GPIO pin
    GPIO_SetValue_CS_EPROM(0); // Deactivate CS_EPROM
    return wVal;
}

/* ************************************************************************** */
/***	EPROM_Write_Raw
**
**	Parameters:
**      uint8_t bAddress		- the address where the values will be written in EPROM
**      uint16_t wVal           - 16-bit value, to be written in EPROM
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_EPROM_WRTIMEOUT          0xFF    // EPROM write data ready timeout
**
**	Description:
**		This function writes the specified word (16-bit value) in EPROM.  
**      It is mandatory to enable the write operation before sending the data to EPROM, by calling the EPROM_WriteEnable() function. 
**      The function returns ERRVAL_SUCCESS for success or ERRVAL_EPROM_WRTIMEOUT when eprom is 
**      not answering with write successful message. 
**            
*/
uint8_t EPROM_Write_Raw(uint8_t bAddress, uint16_t wVal)
{
    uint8_t bResult;
    GPIO_SetValue_CS_EPROM(1); // Activate CS_EPROM
 
    EPROM_StartBitOpAddr_Raw(EPROM_OPCODE_WRITE, bAddress);
    SPI_CoreTransferByte(wVal >> 8);     // MSByte
    SPI_CoreTransferByte(wVal & 0xFF);   // LSByte
     
	GPIO_SetValue_MOSI(0);	// clear the MOSI GPIO pin
    
    //    DelayAprox10Us(SPI_CLK_DELAY);  
    GPIO_SetValue_CS_EPROM(0); // Deactivate CS_EPROM

    bResult = EPROM_WaitUntilReady_Raw();
    return bResult;

}


/* ************************************************************************** */
/***	EPROM_WriteWords_Raw
**
**	Parameters:
**      uint8_t bAddress		- the address where the values will be written to
**      uint16_t *prgVals       - pointer to an array of 16-bit values, to be written in EPROM
**      int cwVals              - number of 16-bit values to be written in EPROM
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_EPROM_WRTIMEOUT          0xFF    // EPROM write data ready timeout
**
**	Description:
**		This function writes the specified number of words (16-bit values) in EPROM.  
**      It is mandatory to enable the write operation before sending the data to EPROM, by calling the EPROM_WriteEnable() function. 
**      The function returns ERRVAL_SUCCESS for success or ERRVAL_EPROM_WRTIMEOUT when eprom is 
**      not answering with write successful message. 
**      This function is not intended to be called by the user, as it might alter the content.
**      of User Calibration, SerialNO, Factory Calibration areas of EPROM. User should call EPROM_WriteWords function instead.            
*/
uint8_t EPROM_WriteWords_Raw(uint8_t bAddress, uint16_t *prgVals, int cwVals)
{
    uint8_t bResult = 0;
    int i;
    
    for(i = 0; i < cwVals && !bResult; i++)
    {
        bResult = EPROM_Write_Raw(bAddress + i, prgVals[i]);
    }
    return bResult;
}


/* *****************************************************************************
 End of File
 */
