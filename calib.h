/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    eprom.h

  @Description
        This file contains the declarations for the CALIB module functions.
        The EPROM functions are defined in calib.c source file.


 */
/* ************************************************************************** */

#ifndef _CALIB_H    /* Guard against multiple inclusion */
#define _CALIB_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
#define MEASURE_CNT_AVG 20  // the number of values to be used when measuring for calibration
//#define CALIB_RES_ZERO_REFVAL 0
// 50 mOhm
#define CALIB_RES_ZERO_REFVAL 0.05

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************
// initialization
uint8_t CALIB_Init();

// EPROM functions
uint8_t CALIB_RestoreAllCalibsFromEPROM_Factory();
uint8_t CALIB_WriteAllCalibsToEPROM_User();
uint8_t CALIB_ReadAllCalibsFromEPROM_User();
uint8_t CALIB_ReadAllCalibsFromEPROM_Factory();

uint8_t CALIB_VerifyEPROM();
uint8_t CALIB_ExportCalibs_User(char *szLine, uint8_t idxScale);
uint8_t CALIB_ExportCalibs_Factory(char *szLine, uint8_t idxScale);
uint8_t CALIB_ImportCalibCoefficients(int idxScale, float fMult, float fAdd);

// Calibration procedure functions
uint8_t CALIB_CalibOnZero(double *pMeasuredVal);

uint8_t CALIB_MeasureForCalibPositiveVal(double *pMeasuredVal);
uint8_t CALIB_CalibOnPositive(double dRefVal, double *pMeasuredVal, uint8_t bEarlyMeasurement);

uint8_t CALIB_MeasureForCalibNegativeVal(double *pMeasuredVal);
uint8_t CALIB_CalibOnNegative(double dRefVal, double *pMeasuredVal, uint8_t bEarlyMeasurement);




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
#endif /* _CALIB_H */

/* *****************************************************************************
 End of File
 */
