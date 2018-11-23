/* ************************************************************************** */
/** Descriptive File Name

  @Company
 Digilent

  @File Name
    dmm.h

  @Description
        This file contains the declaration for the interface functions of DMM module.
        The DMM functions are defined in dmm.c source file.

  @Versioning:
 	 Cristian Fatu - 2018/06/29 - Initial release, DMMShield Library

 */
/* ************************************************************************** */

#ifndef _DMMCFG_H    /* Guard against multiple inclusion */
#define  _DMMCFG_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "stdint.h"



/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
    
#define DMM_DIODEOPENTHRESHOLD      3
//#define INFINITY        1e+308      // value used when the retrieved data exceeds the convertors range
//#define NAN             0.0f/0.0f   // value used when no proper data is available
    

#define DmmResistance               1
#define DmmContinuity               2 
#define DmmDiode                    3
#define DmmDCVoltage                4
#define DmmACVoltage                5
#define DmmDCCurrent                6
#define DmmACCurrent                7
#define DmmDCLowCurrent             8
#define DmmACLowCurrent             9

#define DMM_CNTSCALES                 27    // the number of scales
#define DMM_VALIDDATA_CNTTIMEOUT    0x100   // number of valid data retrieval re-tries
#define DMMVoltageDC50Scale          7
    
#define DMM_Voltage50DCLinearCoeff_P3   -1.59128E-06
#define DMM_Voltage50DCLinearCoeff_P1   1.003918916
#define DMM_Voltage50DCLinearCoeff_P0   0.000196999

    
    
    
// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef struct _DMMCFG{
    int mode; // user modes
    double range;
    uint8_t sw;     // switch bits: 0 RLD, 1 RLU, 2 RLI
    uint8_t cfg[24]; // configuration bits: 0x1F...0x36
    double mul; // dmm measurement (ad1/rms) multiplication factor to get value in corresponding unit
} DMMCFG;

// registers from 0x00 to 0x1F
typedef struct _DMMSTS{
    uint8_t ad1[3];
    uint8_t ad2[3];
    uint8_t lpf[3];
    uint8_t rms[5];
    uint8_t pkhmin[3];
    uint8_t pkhmax[3];
    uint8_t ctsta;
    uint8_t ctc[3];
    uint8_t ctb[3];
    uint8_t cta[3];
    uint8_t intf;
    uint8_t inte;
} DMMSTS;


// calibration values

#define NO_CALIBS   10
typedef struct _CALIB{
    float  Mult;
    float  Add;
} CALIB;

typedef struct _PARTCALIB{
    double Calib_Ms_Zero;
    double Calib_Ms_ValP;
    double Calib_Ref_ValP;
    double Calib_Ms_ValN;
    double Calib_Ref_ValN;
    uint8_t fCalibDirty;
} PARTCALIB;


typedef struct _CALIBDATA{    //
    uint8_t magic;
    CALIB      Dmm[DMM_CNTSCALES];    // 27*2  54
    uint8_t crc;
}  __attribute__((__packed__)) CALIBDATA;


typedef struct _PARTCALIBDATA{    //
    PARTCALIB  DmmPartCalib[DMM_CNTSCALES];    // stores the data needed to the calibration
} PARTCALIBDATA;



// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************
double DMM_TmpDebugDGetStatus(uint8_t *pbErr, char *pString);
// DMM initialization
void DMM_Init();

// configuration functions
uint8_t DMM_SetScale(int idxScale);
int DMM_GetCurrentScale();
double DMM_GetScaleRange(int idxScale);


// value functions
double DMM_DGetValue(uint8_t *pbErr);
double DMM_DGetAvgValue(int cbSamples, uint8_t *pbErr);
void DMM_SetUseCalib(uint8_t f);
uint8_t DMM_CheckAcceptedMeasurementDispersion(double dMeasuredVal, double dRefVal, double *pDispersion);
uint8_t DMM_FormatValue(double dVal, char *pString, uint8_t fUnit);
uint8_t DMM_InterpretValue(char *pString, double *pdVal);

uint8_t DMM_FDCCurrentScale();
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
