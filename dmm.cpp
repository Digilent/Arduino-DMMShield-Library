/* ************************************************************************** */
/** Descriptive File Name

  @Company
 Digilent

  @File Name
    dmm.c

  @Description
        The DMM module groups the functions that implement the DMM functionality.
        For each DMM scale, the dmmcfg string maintains specific data, and it's used to initialize the environment when selecting a specific scale.
        DMM functions use the functions defined in SPI as the data communication layer.
        DMM functions use GPIO module to access the chip select for the DMM device.
        The "Interface functions" section groups functions that can also be called by User. 
        These are functions to set the current scale, to get value from DMM, to format value into a string, according 
        to the current scale, to interpret value from a string, according to the current value.
        The "Local functions" section groups low level functions that are only called from within the current module. 
        The module calls SPI functions defined in the SPI module.
        The module uses errors defined in the ERRORS module.
        The DMM functions are called from CALIB, DMMCMD modules.

  @Author
    Cristian Fatu 
    cristian.fatu@digilent.ro

 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdio.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include "math.h"
#include "dmm.h"
#include "calib.h"
#include "gpio.h"
#include "spi.h"
#include "errors.h"
#include "utils.h"

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions Prototypes                                        */
/* ************************************************************************** */
/* ************************************************************************** */
// DMM Switches function
void DMM_ConfigSwitches(uint8_t sw);

// DMM SPI functions
void DMM_SendCmdSPI(uint8_t bCmd, int bytesNumber, uint8_t *pbWrData);
void DMM_GetCmdSPI(uint8_t bCmd, int bytesNumber, uint8_t *pbRdData);

// retrieve value from DMM
double DMM_DGetStatus(uint8_t *pbErr);

// value format
uint8_t DMM_GetScaleUnit(int idxScale, double *pdScaleFact, char *szUnitPrefix, char *szUnit);

// configuration functions
uint8_t DMM_FACScale(int idxScale);
double DMM_CompensateVoltage50DCLinear(double dVal);
// errors 
uint8_t DMM_ERR_CheckIdxCalib(int idxScale);

// utils
uint8_t DMM_IsNotANumber(double dVal);

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Global Variables                                                  */
/* ************************************************************************** */
/* ************************************************************************** */
extern CALIBDATA calib; // defined in calib.c

#define CALIB_ACCEPTANCE_DEFAULT    0.2
// mask unused register bits on configuration verification
const static uint8_t dmmcfgmask[]={0x1F, 0xFE, 0xFF, 0xFF, 0x9F, 0xFF, 0xFF, 0xBF, 0xFF, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xFC, 0xFF};

// configuration, contains scale specific data
const static PROGMEM DMMCFG dmmcfg[] = {
//                                 0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
//                              INTE,  R20,  R21,  R22,  R23,  R24,  R25,  R26,  R27,  R28,  R29,  R2A,  R2B,  R2C,  R2D,  R2E,  R2F,  R30,  R31,  R32,  R33,  R34,  R35,  R36
{DmmResistance, 5e7,     1, {0x00, 0xC0, 0xCF, 0x17, 0x93, 0x85, 0x00, 0x00, 0x55, 0x55, 0x00, 0x00, 0x08, 0x00, 0x00, 0x80, 0x86, 0x80, 0xD1, 0x3C, 0xA0, 0x00, 0x00, 0x00}, 6e7 /0.9/8388608      }, // 0 "50M Ohm"
{DmmResistance, 5e6,     1, {0x00, 0xC0, 0xCF, 0x17, 0x93, 0x85, 0x00, 0x00, 0x55, 0x55, 0x00, 0x00, 0x08, 0x00, 0x80, 0x80, 0x86, 0x80, 0xD1, 0x3C, 0xA0, 0x00, 0x00, 0x00}, 6e6 /0.9/8388608      }, // 1 "5M Ohm"
{DmmResistance, 5e5,     1, {0x00, 0xC0, 0xCF, 0x17, 0x93, 0x85, 0x00, 0x00, 0x55, 0x55, 0x00, 0x00, 0x08, 0x00, 0x08, 0x80, 0x86, 0x80, 0xD1, 0x33, 0x20, 0x00, 0x00, 0x00}, 6e5 /0.9/8388608      }, // 2 "500k Ohm"
{DmmResistance, 5e4,     1, {0x00, 0xC0, 0xCF, 0x17, 0x83, 0x85, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x40, 0x00, 0x06, 0x44, 0x94, 0x80, 0xD3, 0x33, 0x20, 0x00, 0x00, 0x00}, 1e5 /0.9/8388608      }, // 3 "50k Ohm"
{DmmResistance, 5e3,     1, {0x00, 0xC0, 0xCF, 0x17, 0x83, 0x85, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x40, 0x60, 0x00, 0x44, 0x94, 0x80, 0xD3, 0x33, 0x20, 0x00, 0x00, 0x00}, 1e4 /0.9/8388608      }, // 4 "5k Ohm"
{DmmResistance, 5e2,     1, {0x00, 0xC0, 0xCF, 0x17, 0x83, 0x35, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x40, 0x06, 0x00, 0x44, 0x94, 0x80, 0xD2, 0x3C, 0xA0, 0x00, 0x00, 0x00}, 1e3 /0.9/8388608      }, // 5 "500 Ohm"
{DmmResistance, 5e1,     1, {0x00, 0xC0, 0xCF, 0x17, 0x83, 0x35, 0x01, 0x00, 0x55, 0x00, 0x00, 0x00, 0x40, 0x06, 0x00, 0x44, 0x94, 0x80, 0xD2, 0x3C, 0xA0, 0x00, 0x00, 0x00}, 1e2 /0.9/8388608      }, // 6 "50 Ohm"
{DmmDCVoltage, 5e1,      2, {0x00, 0x60, 0x00, 0x17, 0x8B, 0x01, 0x11, 0x00, 0x55, 0x31, 0x00, 0x22, 0x00, 0x00, 0x09, 0x28, 0xA0, 0x80, 0xC7, 0x33, 0x20, 0x00, 0x00, 0x00}, 125e0 /1.8/8388608    }, // 7 "50 V DC",
{DmmDCVoltage, 5e0,      2, {0x00, 0x60, 0x00, 0x17, 0x8B, 0x01, 0x11, 0x00, 0x55, 0x31, 0x00, 0x22, 0x00, 0x00, 0x90, 0x28, 0xA0, 0x80, 0xC7, 0x33, 0x20, 0x00, 0x00, 0x00}, 125e-1/1.8/8388608    }, //8 "5 V DC",
{DmmDCVoltage, 5e-1,     1, {0x00, 0xC0, 0x00, 0x17, 0x8B, 0x85, 0x11, 0x00, 0x55, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x33, 0x28, 0x00, 0x00, 0x00}, 125e-2/1.8/8388608    }, //9 "500 mV DC"
{DmmDCVoltage, 5e-2,     1, {0x00, 0x00, 0x00, 0x17, 0x8B, 0x35, 0x11, 0x00, 0x55, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x3C, 0x60, 0x00, 0x00, 0x00}, 125e-3/1.8/8388608    }, //10 "50 mV DC"
{DmmACVoltage, 5e1,      2, {0x00, 0xF2, 0xDD, 0x07, 0x03, 0x52, 0x10, 0x80, 0x25, 0x31, 0xF8, 0x22, 0x00, 0x00, 0x0D, 0x28, 0xA0, 0xFF, 0xC7, 0x38, 0x20, 0x00, 0x00, 0x00}, 1e-3                  }, //11 "30 V AC"
{DmmACVoltage, 5e0,      2, {0x00, 0xF2, 0xDD, 0x07, 0x03, 0x52, 0x10, 0x80, 0x25, 0x31, 0xF8, 0x22, 0x00, 0x00, 0xD0, 0x88, 0xA0, 0xFF, 0xC7, 0x38, 0x20, 0x02, 0x50, 0x0C}, 1e-4                  }, //12 "5 V AC"
{DmmACVoltage, 5e-1,     1, {0x00, 0x92, 0xDD, 0x07, 0x03, 0x52, 0x10, 0x80, 0x25, 0x11, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x3A, 0x28, 0x00, 0x00, 0x00}, 1e-5                  }, //13 "500 mV AC"
{DmmACVoltage, 5e-2,     1, {0x00, 0x52, 0xDD, 0x07, 0x03, 0x00, 0x13, 0x80, 0x25, 0x11, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x3A, 0x28, 0x00, 0x00, 0x00}, 1e-6                  }, //14 "50 mV AC"
{DmmDCCurrent, 5e0,      0, {0x00, 0x00, 0x00, 0x17, 0x8B, 0x95, 0x11, 0x00, 0x55, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC7, 0x33, 0x20, 0x00, 0x00, 0x00}, 125e0/3.6/8388608     }, //15 "5 A DC"
{DmmACCurrent, 5e0,      0, {0x00, 0x52, 0xDD, 0x07, 0x03, 0x00, 0x13, 0x80, 0x25, 0x11, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x3D, 0x28, 0x00, 0x00, 0x00}, 1e-4/2.16             }, //16 "5 A AC" 
{DmmContinuity,500,      1, {0x00, 0x74, 0xCF, 0x17, 0x83, 0x35, 0x10, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x40, 0x86, 0x80, 0xD2, 0x3C, 0xA0, 0x00, 0x00, 0x00}, 666e-7                }, //17 "Continuity
{DmmDiode,     3.0,      1, {0x00, 0xC0, 0xCF, 0x17, 0x8B, 0x8D, 0x10, 0x00, 0x55, 0x31, 0x00, 0x00, 0x00, 0x08, 0x00, 0x40, 0x86, 0x80, 0xE2, 0x33, 0xA0, 0x00, 0x00, 0x00}, 666e-6                }, //18 "Diode
{DmmDCLowCurrent, 5e-1,  0, {0x00, 0x00, 0x00, 0x17, 0x8B, 0x95, 0x11, 0x00, 0x55, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC7, 0x33, 0x20, 0x00, 0x00, 0x00}, 125e-2/1.8/8388608    }, //19 "500 mA DC"
{DmmDCLowCurrent, 5e-2,  0, {0x00, 0x00, 0x00, 0x17, 0x8B, 0x35, 0x11, 0x00, 0x55, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC7, 0x3D, 0xA0, 0x00, 0x00, 0x00}, 125e-3/1.8/8388608    }, //20 "50 mA DC"
{DmmDCLowCurrent, 5e-3,  4, {0x00, 0x00, 0x00, 0x17, 0x8B, 0x95, 0x11, 0x00, 0x55, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC7, 0x33, 0x20, 0x00, 0x00, 0x00}, 125e-4/1.8/8388608    }, //21 "5 mA DC"
{DmmDCLowCurrent, 5e-4,  4, {0x00, 0x00, 0x00, 0x17, 0x8B, 0x35, 0x11, 0x00, 0x55, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC7, 0x3D, 0xA0, 0x00, 0x00, 0x00}, 125e-5/1.8/8388608    }, //22 "500 uA DC"
{DmmACLowCurrent, 5e-1,  0, {0x00, 0x92, 0xDD, 0x07, 0x03, 0x52, 0x10, 0x80, 0x25, 0x11, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x3D, 0x28, 0x00, 0x00, 0x00}, 1e-5/1.08             }, //23 "500 mA AC"
{DmmACLowCurrent, 5e-2,  0, {0x00, 0x52, 0xDD, 0x07, 0x03, 0x00, 0x13, 0x80, 0x25, 0x11, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x3D, 0x28, 0x00, 0x00, 0x00}, 1e-6/1.08             }, //24 "50 mA AC"
{DmmACLowCurrent, 5e-3,  4, {0x00, 0x92, 0xDD, 0x07, 0x03, 0x52, 0x10, 0x80, 0x25, 0x11, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x3D, 0x28, 0x00, 0x00, 0x00}, 1e-7/1.08             }, //25 "5 mA AC"
{DmmACLowCurrent, 5e-4,  4, {0x00, 0x52, 0xDD, 0x07, 0x03, 0x00, 0x13, 0x80, 0x25, 0x11, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x80, 0xC7, 0x3D, 0x28, 0x00, 0x00, 0x00}, 1e-8/1.08             }, //26 "500 uA AC" 
{0}};
DMMCFG curCfg;
int idxCurrentScale = -1;   // stores the current selected scale
char fUseCalib = 1;         // controls if calibration coefficients should be applied in DMM_DGetStatus

//char sTmpDebug[100];
//char sTmpDebug1[10];
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/***	DMM_Init
**
**	Parameters:
**		
**
**	Return Value:
**		
**
**	Description:
**		This function initializes the DMM module. 
**      It calls the SPI_Init() function to initialize the digital pins used by DMMSHield.
**     
**      
**          
*/
void DMM_Init()
{
    SPI_Init();
	CALIB_Init();
}

/***	DMM_SetScale
**
**	Parameters:
**      uint8_t idxScale		- the scale index
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS            0      // success
**          ERRVAL_DMM_IDXCONFIG     0xFC    // error, wrong scale index
**          ERRVAL_DMM_CFGVERIFY     0xF5    // DMM Configuration verify error
**	Description:
**		This function configures a specific scale as the current scale.
**      According to this scale, it uses data defined in dmmcfg structure to configure the switches and 
**      to set the value of the registers (24 registers starting at 0x1F address).
**      It also verifies the configuration setting success status by reading the values of these registers.
**      It returns ERRVAL_SUCCESS if the operation is successful.
**      It returns ERRVAL_DMM_CFGVERIFY if verifying fails.
**      It returns ERRVAL_DMM_IDXCONFIG if the scale index is not valid.
**            
*/
uint8_t DMM_SetScale(int idxScale)
{
    // 0. Verify index
    uint8_t bResult = DMM_ERR_CheckIdxCalib(idxScale);
    if(bResult != ERRVAL_SUCCESS)
    {
        return bResult;
    }
	// 1. Retrieve current Scale information from PROGMEM
	memcpy_P(&curCfg, dmmcfg + idxScale, sizeof (DMMCFG));
	
	
    const int cbCfg = 24;
    uint8_t rgIn[24];
    
    // 2. Reset the DMM by writing 0x60 on 0x37 register
    uint8_t valReset = 0x60;
    // Build command:
    //  MSB: 7 bits address: 0x37
    //  LSB: 0 for write
    uint8_t bCmd = 0x37 << 1;

    // Write 1 bytes, starting with 0x37 address
    DMM_SendCmdSPI(bCmd, 1, &valReset);

    // 3. Set the switches
    
    // clear switches
    DMM_ConfigSwitches(curCfg.sw); 
      
    // 4. Set the value for the 24 registers starting with 0x1f
    // Build command:
    //  MSB: 7 bits address: 0x1F
    //  LSB: 0 for write
    bCmd = 0x1F << 1;
    
    // Write 24 bytes, starting with 0x1F address, values taken from dmmcfg[idxScale].cfg array
    DMM_SendCmdSPI(bCmd, cbCfg, (uint8_t *)curCfg.cfg);

    // 5. Verify the values of the 24 registers starting with 0x1f
    
    // Build command:
    //  MSB: 7 bits address: 0x1F
    //  LSB: 1 for read
    bCmd =(0x1F<<1) | 1;    
//    DelayAprox10Us(500);     

    // 5.1. Read 24 bytes, starting with 0x1F address, values placed in rgIn array
    DMM_GetCmdSPI(bCmd, cbCfg, rgIn);
//    DelayAprox10Us(1000);     

    // 5.2. Compare values from rgIn and dmmcfg[idxScale].cfg arrays
     int i;
     for(i = 0; i < cbCfg; i++){
         if((rgIn[i]&dmmcfgmask[i])!=(curCfg.cfg[i]&dmmcfgmask[i]))
         {
            // DMM scale configuration verify failed;
             return ERRVAL_DMM_CFGVERIFY;
         }
     }
     
     // 6. Set idxScale as current scale
    idxCurrentScale = idxScale;
    return ERRVAL_SUCCESS;

}

/***	DMM_ERR_CheckIdxCalib
**
**	Parameters:
**      uint8_t idxScale		- the scale index
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS            0       // success
**          ERRVAL_DMM_IDXCONFIG     0xFC    // error, wrong scale index
**
**	Description:
**		This function checks the scale index. 
**      If it is valid (between 0 and maximum value of the scale index - 1), the function returns ERRVAL_SUCCESS  
**      If it is not valid, the function returns ERRVAL_DMM_IDXCONFIG.  
**            
*/
uint8_t DMM_ERR_CheckIdxCalib(int idxScale)
{
    uint8_t bResult = (idxScale >= 0) && (idxScale < DMM_CNTSCALES) ? ERRVAL_SUCCESS: ERRVAL_DMM_IDXCONFIG;
    return bResult;
}

/***	DMM_DGetValue
**
**	Parameters:
**      uint8_t *pbErr - Pointer to the error parameter, the error can be set to:
**          ERRVAL_SUCCESS              0       // success
**          ERRVAL_DMM_VALIDDATATIMEOUT 0xFA    // valid data DMM timeout
**          ERRVAL_DMM_IDXCONFIG        0xFC    // error, wrong current scale index
**
**	Return Value:
**		double 
**          the value computed according to the convertor / RMS registers values, or
**          NAN (not a number) value if the convertor / RMS registers value is not ready or if ERRVAL_DMM_IDXCONFIG was set, or
**          +/- INFINITY if the convertor / RMS registers values are outside the expected range.
**	Description:
**		This function repeatedly retrieves the value from the convertor / RMS registers 
**      by calling private private function DMM_DGetStatus, until a valid value is detected.
**      It returns INFINITY when measured values are outside the expected convertor range.
**      If there is no valid current scale selected, the function sets the error value to ERRVAL_DMM_IDXCONFIG and NAN value is returned. 
**      If there is no valid value retrieved within a specific timeout period, the error is set to ERRVAL_DMM_VALIDDATATIMEOUT.
**		This function compensates the not linear behavior of VoltageDC50 scale.
**		When no error is detected, the error is set to ERRVAL_SUCCESS.
**      The error is copied in the byte pointed by pbErr, if pbErr is not null.
**            
*/
double DMM_DGetValue(uint8_t *pbErr)
{
    uint8_t bErr = ERRVAL_SUCCESS;
    // valid data timeout counter 
    unsigned long cntTimeout = 0;
    
    double dVal;
    // wait until a valid value is retrieved or the timeout counter exceeds threshold
    while(DMM_IsNotANumber(dVal = DMM_DGetStatus(&bErr)) && (cntTimeout++ < DMM_VALIDDATA_CNTTIMEOUT) && (bErr == ERRVAL_SUCCESS));
    // detect timeout 
    if((bErr == ERRVAL_SUCCESS) && (cntTimeout >=  DMM_VALIDDATA_CNTTIMEOUT))
    {
        bErr = ERRVAL_DMM_VALIDDATATIMEOUT;
    }
    if(bErr == ERRVAL_SUCCESS && DMM_GetCurrentScale() == DMMVoltageDC50Scale)
    {
        // compensate the not linear scale behavior
        dVal = DMM_CompensateVoltage50DCLinear(dVal);
    }
    
    // set error
    if(pbErr)
    {
        *pbErr = bErr;
    }
    return dVal;
}

/***	DMM_DGetAvgValue
**
**	Parameters:
**      int cbSamples           - The number of values to be used for the average value        
**      uint8_t *pbErr    - Pointer to the error parameter, the error can be set to:
**          ERRVAL_SUCCESS              0       // success
**          ERRVAL_DMM_VALIDDATATIMEOUT 0xFA    // valid data DMM timeout
**          ERRVAL_DMM_IDXCONFIG        0xFC    // error, wrong current scale index
**
**	Return Value:
**		double
**          the DMM value, or
**          NAN (not a number) value if errors were detected
**	Description:
**		This function computes an average value corresponding to the DMM value  
**      returned by DMM_DGetValue, for the specified number of samples. 
**      The function uses Arithmetic mean average value method for all but AC scales, 
**      and RMS (Quadratic mean) Average value method for for AC scales.
**      If there is no valid current scale selected, the error is set to ERRVAL_DMM_IDXCONFIG. 
**      If there is no valid value retrieved within a specific timeout period, the error is set to ERRVAL_DMM_VALIDDATATIMEOUT.
**      It returns INFINITY when measured values are outside the expected convertor range.
**      When no error is detected, the error is set to ERRVAL_SUCCESS.
**      The error is copied on the byte pointed by pbErr, if pbErr is not null.
**      When errors are detected, the function returns NAN.
 **            
*/
double DMM_DGetAvgValue(int cbSamples, uint8_t *pbErr)
{
    uint8_t fValid = 1;
    double dValAvg = 0.0, dVal;
    int i;
    int idxScale = DMM_GetCurrentScale();
	uint8_t bErr  = DMM_ERR_CheckIdxCalib(idxScale);    
    uint8_t fAC;
    if(bErr == ERRVAL_SUCCESS)
    {
        fAC = DMM_FACScale(idxScale);
        if(fAC)
        {
            // use RMS (Quadratic mean) Average value for AC
            for(i = 0; (i < cbSamples) && fValid; i++)
            {
                dVal = DMM_DGetValue(&bErr);
                fValid = (bErr == ERRVAL_SUCCESS) && (dVal != INFINITY) && (dVal != -INFINITY )&& !DMM_IsNotANumber(dVal);
                if(fValid)
                {
                    dValAvg += pow(dVal, 2);
                }
            }
            if(fValid && cbSamples)
            {
                dValAvg /= cbSamples;                
                dValAvg = sqrt(dValAvg);
            }            
        }
        else
        {
            // use normal (Arithmetic mean) Average value for other than AC.
            for(i = 0; (i < cbSamples) && fValid; i++)
            {
                dVal = DMM_DGetValue(&bErr);
                fValid = (bErr == ERRVAL_SUCCESS) && (dVal != INFINITY) && (dVal != -INFINITY )&& !DMM_IsNotANumber(dVal);
                if(fValid)
                {
                    dValAvg += dVal;
                }
            }
            if(fValid && cbSamples)
            {
                dValAvg /= cbSamples;                
            }
        }
    }
    if(bErr != ERRVAL_SUCCESS)
    {
        dValAvg = NAN;
    }
    if(pbErr)
    {
        *pbErr = bErr;
    }
    return dValAvg;
}


/***	DMM_GetCurrentScale
**
**	Parameters:
**      none
**
**	Return Value:
**		int     - current scale index
**              - -1 if no current scale was selected
**
**	Description:
**		This function returns the current scale index. 
**      This is the last scale index selected using the DMM_SetScale function.
**      In case no current scale was selected, the function returns -1.
**            
*/
int DMM_GetCurrentScale()
{
    return idxCurrentScale;
}

/***	DMM_GetCurrentScale
**
**	Parameters:
**      none
**
**	Return Value:
**		int     - current scale index
**              - -1 if no current scale was selected
**
**	Description:
**		This function returns the current scale index. 
**      This is the last scale index selected using the DMM_SetScale function.
**      In case no current scale was selected, the function returns -1.
**            
*/
double DMM_GetScaleRange(int idxScale)
{
    double range = curCfg.range;
    return range;
}
/***	DMM_SetUseCalib
**
**	Parameters:
**      uint8_t f  
**              1 if calibrations coefficients should be applied in future DMM_DGetStatus calls
**              0 if calibrations coefficients should not be applied in future DMM_DGetStatus calls
**
**	Return Value:
**		none
**
**	Description:
**		This function sets the parameter that will determine whether or not the 
**      calibration coefficients will be applied when value is computed in 
**      subsequent DMM_DGetStatus calls. 
**      The default value for this parameter is 1.
**            
*/
void DMM_SetUseCalib(uint8_t f)
{
    fUseCalib = f;
}

/***	DMM_FACScale
**
**	Parameters:
**      int idxScale  - the scale index
**              
**
**	Return Value:
**		1 if the specified scale is a AC (alternating current) scale.
**		0 if the specified scale is not a AC (alternating current) scale.
**
**	Description:
**		This function checks if the specified scale is an AC (alternating current) scale.
**      It returns 1 for the AC Voltage, AC current and AC Low current type scales, and 0 otherwise.
**      The scale type is checked using the mode field in DMMCFG structure.
**            
*/
uint8_t DMM_FACScale(int idxScale)
{
    int mode = curCfg.mode;
    return (mode == DmmACVoltage) || (mode == DmmACCurrent) || (mode == DmmACLowCurrent);
}

/* ************************************************************************** */
/***	DMM_FDCScale
**
**	Parameters:
**      int idxScale  - the scale index
**              
**
**	Return Value:
**		1 if the specified scale is a DC (direct current) type scale.
**		0 if the specified scale is not a DC (direct current) type scale.
**
**	Description:
**		This function checks if the specified scale is a DC (direct current) type scale.
**      It returns 1 for the DC Voltage, DC current and DC Low current type scales, and 0 otherwise.
**      The scale type is checked using the mode field in DMMCFG structure.
**            
*/
uint8_t DMM_FDCScale(int idxScale)
{
    int mode = curCfg.mode;
    return (mode == DmmDCVoltage) || (mode == DmmDCCurrent) || (mode == DmmDCLowCurrent);
}

/***	DMM_FResistorScale
**
**	Parameters:
**      int idxScale  - the scale index
**              
**
**	Return Value:
**		1 if the specified scale is a Resistor type scale.
**		0 if the specified scale is not a Resistor type scale.
**
**	Description:
**		This function checks if the specified scale is a Resistor type scale.
**      It returns 1 for the Resistor and Continuity type scales, and 0 otherwise.
**      The scale type is checked using the mode field in DMMCFG structure.
**            
*/
uint8_t DMM_FResistorScale(int idxScale)
{
    int mode = curCfg.mode;
    return (mode == DmmResistance) || (mode == DmmContinuity);
}

/***	DMM_FDiodeScale
**
**	Parameters:
**      int idxScale  - the scale index
**              
**
**	Return Value:
**		1 if the specified scale is a Diode type scale.
**		0 if the specified scale is not a Diode type scale.
**
**	Description:
**		This function checks if the specified scale is a Diode type scale.
**      It returns 1 for the Diode and Continuity type scales, and 0 otherwise.
**      The scale type is checked using the mode field in DMMCFG structure.
**            
*/
uint8_t DMM_FDiodeScale(int idxScale)
{
    int mode = curCfg.mode;
    return (mode == DmmDiode);
}

/***	DMM_FContinuityScale
**
**	Parameters:
**      int idxScale  - the scale index
**              
**
**	Return Value:
**		1 if the specified scale is a Continuity type scale.
**		0 if the specified scale is not a Continuity type scale.
**
**	Description:
**		This function checks if the specified scale is a Continuity type scale.
**      It returns 1 for the Continuity and Continuity type scales, and 0 otherwise.
**      The scale type is checked using the mode field in DMMCFG structure.
**            
*/
uint8_t DMM_FContinuityScale(int idxScale)
{
    int mode = curCfg.mode;
    return (mode == DmmContinuity);
}

/***	DMM_IsNotANumber
**
**	Parameters:
**      double dVal  - the value to be checked
**              
**
**	Return Value:
**		1 if the specified value is "not a number".
**		0 if the specified scale is a valid number.
**
**	Description:
**		This function checks if the specified value contains a "not a number" value.
**      The "not a number" values are used to identify the abnormal situations like the data not available in the DMM converters.
**      It returns 1 if the value is a "not a number" value, and 0 if the value corresponds to a valid number.
**            
*/
uint8_t DMM_IsNotANumber(double dVal)
{
    return (dVal == NAN) || isnan(dVal); 
}

/***	DMM_GetScaleUnit
**
**	Parameters:
**		int idxScale        - the Scale index
**      double *pdScaleFact - pointer to a variable to get the scale factor value
**      char *szUnitPrefix  - string to get the Unit prefix corresponding to the multiple / submultiple
**      char *szUnit        - string to get the Unit 
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_DMM_IDXCONFIG            0xFC    // wrong scale index
**
**	Description:
**		The function identifies the Measuring unit data (scale factor, Unit prefix and Unit) for the specified scale.
**      It uses scale type to identify the Unit. 
**      It uses scale range to identify the Unit prefix (u, m, k, M) and the corresponding scale factor. 
**      The scale factor is the value that must multiply the value to convert from the base Unit to the prefixed unit (for example from V to mV)
**      The function returns ERRVAL_DMM_IDXCONFIG if the provided Scale is not valid. 
**                
*/
uint8_t DMM_GetScaleUnit(int idxScale, double *pdScaleFact, char *szUnitPrefix, char *szUnit)
{
    uint8_t bResult = DMM_ERR_CheckIdxCalib(idxScale);
    if(bResult == ERRVAL_SUCCESS)
    {
        // valid idxScale
        if(pdScaleFact && szUnit)
        {
            // the pointers are not null
            if(curCfg.range < 1e-3)
            {
                // micro
                strcpy(szUnitPrefix, "u");
                *pdScaleFact = 1e6;
            }
            else
            {
                if((curCfg.range >= 1e-3) && (curCfg.range < 1))
                {
                    // mili
                    strcpy(szUnitPrefix, "m");
                    *pdScaleFact = 1e3;
                }
                else
                {
                    if((curCfg.range >= 1) && (curCfg.range < 1e3))
                    {
                        // unit
                        szUnitPrefix[0] = 0; // empty string
                        *pdScaleFact = 1;
                    }
                    else
                    {
                        if((curCfg.range >= 1e3) && (curCfg.range < 1e6))
                        {
                            // kilo
                            strcpy(szUnitPrefix, "k");
                            *pdScaleFact = 1e-3;
                        }
                        else
                        {
                            // Mega
                            strcpy(szUnitPrefix, "M");
                            *pdScaleFact = 1e-6;
                        }
                    }
                }
            }
            // detect measuring unit, depending on type
            switch(curCfg.mode)
            {
                case DmmDCVoltage:
                case DmmACVoltage:
                case DmmDiode:
                    strcpy(szUnit, "V");
                    break;
                case DmmDCCurrent:
                case DmmACCurrent:
                case DmmDCLowCurrent:
                case DmmACLowCurrent:
                    strcpy(szUnit, "A");
                    break;
                case DmmResistance:
                case DmmContinuity:               
                    strcpy(szUnit, "Ohm");
                    break;
            }
        }
    }
    return bResult;
}



/***	DMM_FormatValue
**
**	Parameters:
**		double dVal         - The value to be formatted
**      char *pString       - The string to get the formatted value
**      uint8_t fUnit       - flag to indicate if unit information should be added
**              0       - do not add unit
**              not 0   - add unit / subunit
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_DMM_IDXCONFIG            0xFC    // wrong scale index
**
**	Description:
**		The function formats a value according to the current selected scale.
**      The parameter value dVal must correspond to the base Unit (V, A or Ohm), mainly the value returned by DMM_DGetValue.
**      The function multiplies the value according to the scale specific multiple / submultiple.
**      It formats the value with 6 decimals.
**      If fUnit is not 0 it adds the measure unit text (including multiple / submultiple) corresponding to the scale.
**      If dVal is +/- INFINITY (converter values are outside expected range), then "OVERLOAD" string is used for all scales except Continuity.
**      If dVal is +/- INFINITY (converter values are outside expected range), then "OPEN" string is used for Continuity scale.
**      The function returns ERRVAL_DMM_IDXCONFIG if the current scale is not valid. 
**      For example it formats the value 0.0245678912 into the "24.678912 mV" if the current scale is VoltageDC50m.
**      
**                
*/
uint8_t DMM_FormatValue(double dVal, char *pString, uint8_t fUnit)
{
    // default 6 decimals
    double dScaleFact;
    char szUnitPrefix[2], szUnit[5];
    uint8_t bResult = DMM_ERR_CheckIdxCalib(idxCurrentScale);
    if(bResult == ERRVAL_SUCCESS)
    {
        if (dVal == INFINITY)
        {
            if(curCfg.mode == DmmContinuity)
            {
                strcpy(pString, "OPEN");
            }
            else
            {
                strcpy(pString, "OVERLOAD");
            }
        }
        else
        {
            if (dVal == -INFINITY)
            {
                if(curCfg.mode == DmmContinuity)
                {
                    strcpy(pString, "OPEN");
                }
                else
                {
                    strcpy(pString, "OVERLOAD");
                }
            }
            else
            {
                bResult = DMM_GetScaleUnit(idxCurrentScale, &dScaleFact, szUnitPrefix, szUnit);
                if(bResult == ERRVAL_SUCCESS)
                {
                    // valid idxScale
                    dVal *= dScaleFact;
                    //sprintf(pString, "%.6lf", dVal);
					SPrintfDouble(pString, dVal, 6);
                    if(fUnit)
                    {
                        strcat(pString, " ");
                        strcat(pString, szUnitPrefix);
                        strcat(pString, szUnit);
                    }
                }
            }
        }
        if(curCfg.mode == DmmDiode && dVal > DMM_DIODEOPENTHRESHOLD )
        {
            strcpy(pString, "OPEN");        
        }
    }
    return bResult;
}

/***	DMM_InterpretValue
**
**	Parameters:
**		char *pString       - the string containing the value
**      double *pdVal       - pointer to a double variable to get the value
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_DMM_IDXCONFIG            0xFC    // wrong scale index
**          ERRVAL_CMD_VALWRONGUNIT         0xF4    // The provided value has a wrong measure unit.
**
**	Description:
**		The function extracts a value from a string containing a value, eventually followed by a Unit.
**      The string Unit must match the current scale base Unit (V, A or Ohm), still different multiples / submultiples can be used.
**      The function returns in the variable pointed by pdVal the extracted value in the base Unit, 
**      regardless of the multiple / submultiple used in the input string, or the multiple / submultiple specific to the current scale. 
**      If the string is "OVERLOAD" or "OPEN", then INFINITY value is returned.
**      If the measure unit is missing then the unit (with multiple / submultiple) corresponding to the current scale is used.
**      The function returns ERRVAL_DMM_IDXCONFIG if the current scale is not valid. 
**      The function returns ERRVAL_CMD_VALWRONGUNIT if the measure unit does not match the current scale base Unit (V, A or Ohm).
**      For example it interprets the string "24.567891 mV" and returns the value 0.0245678912 if the current scale is any of the Voltage scales.
**                 
*/
uint8_t DMM_InterpretValue(char *pString, double *pdVal)
{
    double dScaleFact;
    int szLen = strlen(pString), szLenUnit;
    char szUnitPrefix[2], szUnit[5];
    uint8_t bResult = ERRVAL_SUCCESS;
    // trim the blank values at the end of the string
    while(pString[szLen - 1] == ' ')
    {
        pString[szLen - 1] = 0;
        szLen --;
    }
    if( !strcmp(pString, "OVERLOAD") || !strcmp(pString, "OPEN"))
    {
        *pdVal = INFINITY;
    }
    else
    {
        bResult = DMM_GetScaleUnit(idxCurrentScale, &dScaleFact, szUnitPrefix, szUnit);
        if(bResult == ERRVAL_SUCCESS)
        {
            // valid idxScale
            if(!(pString[szLen - 1] >= '0' && pString[szLen - 1] <= '9'))
            {
                // the string does not end with numeric char, it contains Unit.
                dScaleFact = 1;
                // look for the szUnit at the end of the provided string
                szLenUnit = strlen(szUnit);
                if(!strcmp(szUnit, pString + szLen - szLenUnit))
                {
                    szLen -= szLenUnit; // remove Unit length
                    // look for any multiple / submultiple prefix before the Unit
                    switch(pString[szLen - 1])
                    {
                        case 'u':
                            dScaleFact = 1e6;
                            szLen--;    // remove prefix length
                            break;
                        case 'm':
                            dScaleFact = 1e3;
                            szLen--;    // remove prefix length
                            break;
                        case 'k':
                            dScaleFact = 1e-3;
                            szLen--;    // remove prefix length
                            break;
                        case 'M':
                            dScaleFact = 1e-6;
                            szLen--;    // remove prefix length
                            break;
                    }
                }
                else
                {
                    // wrong Unit
                    bResult = ERRVAL_CMD_VALWRONGUNIT;
                }
            }   // end check for measure unit
            else
            {
                // the last character is numeric, missing measure unit
                // use the scale unit / multiple which are already set after call to DMM_GetScaleUnit
            }
            if(bResult == ERRVAL_SUCCESS)
            {
                // trim the string to eliminate Unit Prefix and Unit
                pString[szLen] = 0;
                // trim the blank values at the end of the string
                while(pString[szLen - 1] == ' ')
                {
                    pString[szLen - 1] = 0;
                    szLen --;
                }            
                // extract the value
				*pdVal = atof(pString);
				*pdVal /= dScaleFact;
            }
        }
    }
    return bResult;
}

/***	DMM_FDCCurrentScale
**
**	Parameters:
**      < none>
**              
**
**	Return Value:
**		1 if the current scale is a DC Current type scale.
**		0 if the current scale is not a DC Current type scale.
**
**	Description:
**		This function checks if the specified scale is a DC Current type scale.
**      It returns 1 for the DC Current type scales, and 0 otherwise.
**      The scale type is checked using the mode field in DMMCFG structure.
**            
*/
uint8_t DMM_FDCCurrentScale()
{
    int idxScale = DMM_GetCurrentScale();
    int mode = curCfg.mode;
    return (mode == DmmDCCurrent || mode == DmmDCCurrent || mode == DmmDCLowCurrent);
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/***	DMM_ConfigSwitches
**
**	Parameters:
**      uint8_t sw		- byte containing on the 3 LSBs positions the switches configuration
**
**	Return Value:
**		none
**
**	Description:
**		This function configures the 3 on-board switches RLD, RLU, RLI, 
**      according to the 3 LSBs (bit 0, bit 1, bit 2) from the provided parameter sw.
**            
*/
void DMM_ConfigSwitches(uint8_t sw)
{
    // control sw RLD
	GPIO_SetValue_RLD(sw & 1);

    // control sw RLU
	GPIO_SetValue_RLU((sw & 2) >> 1);

    // control sw RLI
	GPIO_SetValue_RLI((sw & 4) >> 2);
} 


/***	DMM_SendCmdSPI
**
**	Parameters:
**		uint8_t bCmd      - the command byte to be transmitted over SPI
**		int bytesNumber         - the number of data bytes to be transmitted over SPI
**		uint8_t *pbWrData - the array of bytes to be transmitted over SPI
**
**	Return Value:
**		none
**
**	Description:
**		This function sends data on a DMM command over the SPI. 
**      It activates DMM Slave Select pin, sends the command byte, and the specified 
**      number of bytes from pbWrData, using the SPI_CoreTransferByte function.
**      Finally it deactivates the DMM Slave Select pin.
**          
*/
void DMM_SendCmdSPI(uint8_t bCmd, int bytesNumber, uint8_t *pbWrData)
{
    int i;
    GPIO_SetValue_CS_DMM(0); // Activate CS_DMM

//    DelayAprox10Us(10);   
    // Send command byte
    SPI_CoreTransferByte(bCmd);

    // Send the requested number of bytes
    for(i = 0; i< bytesNumber; i++)
    {
        SPI_CoreTransferByte(pbWrData[i]);
    }
//    DelayAprox10Us(10);    
    GPIO_SetValue_CS_DMM(1); // Deactivate CS_DMM
}


/***	DMM_GetCmdSPI
**
**	Parameters:
**		uint8_t bCmd      - the command byte to be transmitted over SPI
**		int bytesNumber         - the number of data bytes to be received over SPI
**		uint8_t *pbRdData - the array of bytes to store the bytes received over SPI**
**	Return Value:
**		none	

**
**	Description:
**		This function retrieves data on a DMM command over the SPI. 
**      It activates DMM Slave Select pin, sends the command byte, 
**      and then retrieves the specified number of bytes into pbRdData, using the SPI_CoreTransferByte function.      
**      Finally it deactivates the DMM Slave Select pin.
**          
*/
void DMM_GetCmdSPI(uint8_t bCmd, int bytesNumber, uint8_t *pbRdData)
{
    int i;

    GPIO_SetValue_CS_DMM(0); // Activate CS_DMM
//    DelayAprox10Us(10);
    
    // Send command byte
    SPI_CoreTransferByte(bCmd);
    
    // Generate an extra clock (called SPI Read Period)
    GPIO_SetValue_CLK(1);                // set the clock line
//    DelayAprox10Us(SPI_CLK_DELAY);  // some delay
    GPIO_SetValue_CLK(0);                // reset the clock line
//    DelayAprox10Us(SPI_CLK_DELAY);  // some delay

    // Receive the requested number of bytes
    for(i = 0; i< bytesNumber; i++)
    {
        pbRdData[i] = SPI_CoreTransferByte(0);
    }
//    DelayAprox10Us(10);
    GPIO_SetValue_CS_DMM(1); // Deactivate CS_DMM
}

double DMM_TmpDebugDGetStatus(uint8_t *pbErr, char *pString)
{
	double dResult = DMM_DGetStatus(pbErr);
//	strcpy(pString, sTmpDebug);
	return dResult;
}
/***	DMM_DGetStatus
**
**	Parameters:
**      uint8_t *pbErr - Pointer to the error parameter, the error can be set to:
**          ERRVAL_SUCCESS           0       // success
**          ERRVAL_DMM_IDXCONFIG     0xFC    // error, wrong current scale index
**
**	Return Value:
**		double 
**          the value computed according to the convertor / RMS registers values, or
**          NAN (not a number) value if the convertor / RMS registers value is not ready or if ERRVAL_DMM_IDXCONFIG was set, or
**          +/- INFINITY if the convertor / RMS registers values are outside the expected range.
**	Description:
**		This function reads the value of the convertor / RMS registers (0-0x1F).
**      Then, it computes the value corresponding to the convertor / RMS registers, according to the current selected scale. 
**      Depending on the parameter set by DMM_SetUseCalib (default is 1), calibration parameters will be applied on the computed value.
**      It returns NAN (not a number) when data is not available (ready) in the convertor registers.
**      It returns INFINITY when values are outside the expected convertor range.
**      If there is no valid current scale selected, the function sets error to ERRVAL_DMM_IDXCONFIG and NAN value is returned. 
**      When no error is detected, the error is set to ERRVAL_SUCCESS.
**      The error is copied on the byte pointed by pbErr, if pbErr is not null.
**      
**            
*/
double DMM_DGetStatus(uint8_t *pbErr)
{
    int i;
    double v;
    v = NAN;
    // 1. Verify index
    uint8_t bResult = DMM_ERR_CheckIdxCalib(idxCurrentScale);
    if(bResult != ERRVAL_SUCCESS)
    {
        if(pbErr)
        {
            *pbErr = bResult;
        }
        return NAN;
    }
    // 2. read registers 0x00 - 0x1F values
    DMMSTS dmmsts; // registers 0x00 - 0x1F
    
    // Build command:
    //  MSB: 7 bits address: 0
    //  LSB: 1 for read
    uint8_t bCmd = 1;
    
    // Read 32 bytes, starting with 0 address, values placed in dmmsts
    DMM_GetCmdSPI(bCmd, sizeof(dmmsts), (uint8_t *)&dmmsts);
    
    // 3. Compute value, according to the specific scale
    
    // AD1 signed value
    int32_t vad1 = ((int32_t)dmmsts.ad1[2]<<24)|((int32_t)dmmsts.ad1[1]<<16)|((int32_t)dmmsts.ad1[0]<<8);
    vad1 /= 256;

   // RMS for AC
	uint8_t rms32;
#if defined (__arm__) && defined (__SAM3X8E__) // Arduino Due compatible
	// for 32 bits architecture, do not use 4 bytes RMS.
	rms32 = 0;
#else
	// if MS Byte is 0, use only 4 bytes RMS
	rms32 = (dmmsts.rms[4-i] != 0) ? 1:0;
#endif

    int64_t vrms = 0;
    for(i = 0; i < 5; i++)
	{
		vrms <<= 8;
        vrms |= dmmsts.rms[4-i];
    }
	if(rms32)
	{
		vrms >>= 8;         
	}

	

    if(DMM_FACScale(idxCurrentScale))
    { // AC uses RMS
        if(dmmsts.intf & 0x10)
        { // conversion done
            if(fUseCalib)
            { 
                // apply calibration coefficients
				if(rms32)
				{
					// ignore noise on LSB byte
					v = sqrt(fabs((double)256*((pow(curCfg.mul,2)*(double)(vrms))) - pow(calib.Dmm[idxCurrentScale].Add,2)))*(1.0+calib.Dmm[idxCurrentScale].Mult);
				}
				else
				{
					// Arduino Due compatible or zero MS byte
					v = sqrt(fabs(pow(curCfg.mul,2)*(double)(vrms) - pow(calib.Dmm[idxCurrentScale].Add,2)))*(1.0+calib.Dmm[idxCurrentScale].Mult);         
				}

            }
            else
            {
				if(rms32)
				{
					// ignore noise on LSB byte
					v = (double)16*(curCfg.mul*sqrt((double)vrms));             
				}
				else
				{
					// Arduino Due compatible or zero MS byte
					v = curCfg.mul*sqrt((double)vrms);             
				}

            }
        }   
        else
        {
            v = NAN; // not ready
        }
    }
    else
    { // AD1 value
        if(dmmsts.intf & 0x04)
        { // conversion done
            if(vad1 >= 0x7FFFFE)
            {
                v = INFINITY;   // value outside convertor range
            }
            else
            {
                if(vad1 <= -0x7FFFFE)
                {
                   v = -INFINITY;   // value outside convertor range
                }
               else
               {
                   v = curCfg.mul*vad1;

                    if(fUseCalib)
                    {
                       // apply calibration coefficients
                       v = v*(1.0+calib.Dmm[idxCurrentScale].Mult) + calib.Dmm[idxCurrentScale].Add;
                    }
                }   
            }
        }
        else
        {
            v = NAN; // not ready
        }
    }
    if(pbErr)
    {
        *pbErr = ERRVAL_SUCCESS;
    }    
    return v;
}


/***	DMM_CompensateVoltage50DCLinear
**
**	Parameters:
**      double dVal - The value to be compensated
**
**	Return Value:
**		double 
**          the compensated value
**
**	Description:
**		This function compensates the not linear behavior of VoltageDC50 scale.
**      It gets as parameter the value after the normal calibration coefficients were applied. 
**      It computes the compensated value as P3*dVal^3 + P1*dVal + P0, where P3, P2 and P0 are third power polynomial
**      coefficients defined in dmm.h: DMM_Voltage50DCLinearCoeff_P3, DMM_Voltage50DCLinearCoeff_P1 and DMM_Voltage50DCLinearCoeff_P0.
**      It returns the compensated value.
**      
**            
*/
double DMM_CompensateVoltage50DCLinear(double dVal)
{
    double dCompensatedVal;
    if(!DMM_IsNotANumber(dVal))
    {
       dCompensatedVal = dVal * dVal * dVal * DMM_Voltage50DCLinearCoeff_P3 + dVal * DMM_Voltage50DCLinearCoeff_P1 + dVal * DMM_Voltage50DCLinearCoeff_P0;
    }
    else
    {
        dCompensatedVal = dVal;
    }
    return dCompensatedVal;
}
/* *****************************************************************************
 End of File
 */
