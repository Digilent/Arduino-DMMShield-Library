/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    dmmcmd.c

  @Description
        This file groups the functions that implement the DMM command dispatch module.
        The basic functionality is to interpret the UART received content, recognize specific commands 
        and call the appropriate functions from DMM, CALIB, SERIALNO modules.
        The module also provides an initialization function, that initializes the above mentioned modules 
        whose functions are called from within the command interpreter function.
        In order to minimize the dynamic memory usage (and fit to Arduino UNO), constant strings are placed in the flash (program) memory as much as possible.
        

  @Author
    Cristian Fatu 
    cristian.fatu@digilent.ro
  
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
#include <stdio.h>
#include <Arduino.h>
#include "dmmcmd.h"
#include "dmm.h"
#include "serialno.h"
#include "utils.h"
#include "calib.h"

#include "HardwareSerial.h"
#include "errors.h"

/********************* Function Forward Declarations ***************************/
char* DMMCMD_CmdGetNextArg();
uint8_t DMMCMD_GetCmdIdx(char *szCmd);
void DMMCMD_ProcessCmd(uint8_t idxCmd);
uint8_t DMMCMD_ProcessRepeatedCmd();
// individual commands functions
uint8_t DMMCMD_CmdConfig(char const *arg0);
uint8_t DMMCMD_CmdMeasureRep();
uint8_t DMMCMD_CmdMeasureStop();
uint8_t DMMCMD_CmdMeasureRaw();
uint8_t DMMCMD_CmdMeasureAvg();
uint8_t DMMCMD_CmdCalibP(char const *arg0);
uint8_t DMMCMD_CmdCalibN(char const *arg0);
uint8_t DMMCMD_CmdCalibZ();
uint8_t DMMCMD_CmdSaveEPROM();
uint8_t DMMCMD_CmdVerifyEPROM();
uint8_t DMMCMD_CmdExportCalib(char const *arg0);
uint8_t DMMCMD_CmdImportCalib(char const *arg0, char const *arg1, char const *arg2);
uint8_t DMMCMD_CmdFinalizeCalibP(char const *arg0);
uint8_t DMMCMD_CmdFinalizeCalibN(char const *arg0);
uint8_t DMMCMD_CmdRestoreFactCalib();
uint8_t DMMCMD_CmdReadSerialNo();
uint8_t DMMCMD_ProcessRepeatedCmd();

/* ************************************************************************** */

#define	CMD_IDX_SETSCALE   			 0
#define	CMD_IDX_MEASUREREP			 1
#define	CMD_IDX_MEASURESTOP			 2
#define	CMD_IDX_MEASURERAW			 3
#define	CMD_IDX_MEASUREAVG			 4
#define	CMD_IDX_CALIBP				 5
#define	CMD_IDX_CALIBN   			 6
#define	CMD_IDX_CALIBZ				 7
#define	CMD_IDX_READSERIALNO		 8
#define	CMD_IDX_SAVEEPROM   		 9
#define	CMD_IDX_RESTOREFACTCALIBS	10
#define	CMD_IDX_EXPORTCALIB			11
#define	CMD_IDX_IMPORTCALIB			12

#define CMDS_CNT					13
#define REPEAT_THRESHOLD 5

/********************* Constant Arrays Definitions, placed in Flash ***************************/

const char  scale_0[] PROGMEM = "Resistance50M";   
const char  scale_1[] PROGMEM = "Resistance5M";
const char  scale_2[] PROGMEM = "Resistance500k";
const char  scale_3[] PROGMEM = "Resistance50k";
const char  scale_4[] PROGMEM = "Resistance5k";
const char  scale_5[] PROGMEM = "Resistance500";
const char  scale_6[] PROGMEM = "Resistance50";   
const char  scale_7[] PROGMEM = "VoltageDC50";
const char  scale_8[] PROGMEM = "VoltageDC5";
const char  scale_9[] PROGMEM = "VoltageDC500m";
const char scale_10[] PROGMEM = "VoltageDC50m";   
const char scale_11[] PROGMEM = "VoltageAC50";
const char scale_12[] PROGMEM = "VoltageAC5";
const char scale_13[] PROGMEM = "VoltageAC500m";
const char scale_14[] PROGMEM = "VoltageAC50m";
const char scale_15[] PROGMEM = "CurrentDC5";
const char scale_16[] PROGMEM = "CurrentAC5";   
const char scale_17[] PROGMEM = "Continuity";
const char scale_18[] PROGMEM = "Diode";
const char scale_19[] PROGMEM = "CurrentDC500m";
const char scale_20[] PROGMEM = "CurrentDC50m";   
const char scale_21[] PROGMEM = "CurrentDC5m";
const char scale_22[] PROGMEM = "CurrentDC500u";
const char scale_23[] PROGMEM = "CurrentAC500m";
const char scale_24[] PROGMEM = "CurrentAC50m";
const char scale_25[] PROGMEM = "CurrentAC5m";
const char scale_26[] PROGMEM = "CurrentAC500u";   


// rgScales is a table to refer the scale strings strings.

const char* const rgScales[] PROGMEM = {scale_0, scale_1, scale_2, scale_3, scale_4, scale_5, scale_6, scale_7, scale_8, scale_9,
								scale_10, scale_11, scale_12, scale_13, scale_14, scale_15, scale_16, scale_17, scale_18, scale_19,
								scale_20, scale_21, scale_22, scale_23, scale_24, scale_25, scale_26};


								
const char  cmd_0[] PROGMEM = "DMMSetScale";   
const char  cmd_1[] PROGMEM = "DMMMeasureRep";
const char  cmd_2[] PROGMEM = "DMMMeasureStop";
const char  cmd_3[] PROGMEM = "DMMMeasureRaw";
const char  cmd_4[] PROGMEM = "DMMMeasureAvg";
const char  cmd_5[] PROGMEM = "DMMCalibP";
const char  cmd_6[] PROGMEM = "DMMCalibN";   
const char  cmd_7[] PROGMEM = "DMMCalibZ";
const char  cmd_8[] PROGMEM = "DMMReadSerialNo";
const char  cmd_9[] PROGMEM = "DMMSaveEPROM";   
const char cmd_10[] PROGMEM = "DMMRestoreFactCalibs";
const char cmd_11[] PROGMEM = "DMMExportCalib";
const char cmd_12[] PROGMEM = "DMMImportCalib";



// rgcmds is a table to refer the cmd strings.

const char* const rgcmds[] PROGMEM = {cmd_0, cmd_1, cmd_2, cmd_3, cmd_4, cmd_5, cmd_6, cmd_7, cmd_8, cmd_9,
								cmd_10, cmd_11, cmd_12};
								
/* ************************************************************************** */
/* Section: Global Data, local to this module                                 */
/* ************************************************************************** */
/* ************************************************************************** */

// flags for repeated value and repeated raw value
uint8_t fRepGetVal = 0;
uint8_t fRepGetRaw = 0;
// variables used in multiple functions// allocate them only once.

double dRefVal, dMeasuredVal;
char *pszLastErr;
HardwareSerial *pSerial; // serial interface to be used
char bufTxt[30];
#define bufTxt_0	bufTxt			// character buffer in the first 10 chars of the bufRxr buffer
#define bufTxt_1	bufTxt + 10		// character buffer in the mid 10 chars of the bufRxr buffer
#define bufTxt_2	bufTxt + 20		// character buffer in the last 10 chars of the bufRxr buffer

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
/***	DMMCMD_Init()
**
**	Parameters:
**		none
**
**	Return Value:
**		uint8_t 
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_EPROM_MAGICNO            0xFD    // wrong Magic No. when reading data from EPROM
**          ERRVAL_EPROM_CRC                0xFE    // wrong CRC when reading data from EPROM
**
**	Description:
**		This function initializes the modules involved in the DMMCMD module. 
**      It initializes the DMM, UART, CALIB and SERIALNO modules.
**      The return values are related to errors when calibration is read from user calibration area of EPROM during calibration initialization call.
**      The function returns ERRVAL_SUCCESS for success. 
**      The function returns ERRVAL_EPROM_MAGICNO when a wrong magic number was detected in the data read from EPROM. 
**      The function returns ERRVAL_EPROM_CRC when the checksum is wrong for the data read from EPROM. 
**         
*/
uint8_t DMMCMD_Init(HardwareSerial *phwSerial)
{
    // initializes the modules used by UART Command interpreter
    uint8_t bErrCode;
	DMM_Init();				// initialize the DMM module
    SERIALNO_Init();		// initialize the SERIALNO module
	ERRORS_Init(phwSerial);	// initialize the ERRORS module
    pSerial = phwSerial;
    pszLastErr = ERRORS_GetszLastError();    
    return bErrCode;
}


/***	DMMCMD_CheckForCommand()
**
**	Parameters:
**		    none
**
**	Return Value:
**          none
**
**	Description:
**		This function checks on UART if a command was received. 
**      It compares the received command with the commands defined in the commands array. 
**		If recognized, its corresponding function is called with the needed parameters extracted from the command text.
**      
*/
void DMMCMD_CheckForCommand()
{
	char c;
	static int cntRepeat = 0;
	static int idxChar = 0;
	static char sCmd[CMD_MAX_LEN];
	if(cntRepeat++ >= REPEAT_THRESHOLD)
	{
		DMMCMD_ProcessRepeatedCmd();
		cntRepeat = 0;	// re-arm the repeat counter
	}	
	while(pSerial->available() > 0)
	{
		c = pSerial->read();
//pSerial->print(c);
		if(c == '\r' || c == '\n')
		{
			// recognize any of the line terminating chars.
			if(idxChar > 2)
			{	// ignore empty commands 
				sCmd[idxChar++] = 0;	// terminating 0
				// end of command, it can be found stored in the sCmd string
				pSerial->print(F("COMMAND: "));
				pSerial->println(sCmd);
				DMMCMD_ProcessIndividualCmd(sCmd);
			}
			// prepare for the new cmd
			while (!(*pSerial)) {
			; // wait for serial port to connect. Needed for native USB
			}
			idxChar = 0;
		}
		else
		{
			// place the arrived char in the command string
			sCmd[idxChar++] = c;		
		}
		if(idxChar >= CMD_MAX_LEN)
		{
			// command too long; (re)start from the beginning of the buffer
			idxChar = 0;
		}
	}
}

/***	DMMCMD_GetCmdIdx()
**
**	Parameters:
**		    char *szCmd	- the string containing the command
**
**	Return Value:
**          uint8_t 
**				- the index of the command if the command is found among the defined commands
**				- 0xFF if the command is not found among the defined commands
**
**	Description:
**		This function compares the command against the defined commands. 
**		It returns the command index if found, otherwise returns 0xFF. 
**      
*/
uint8_t DMMCMD_GetCmdIdx(char *szCmd)
{
	uint8_t bResult = 0xFF;
	uint8_t idxCmd;
	char cmdTxt[20];

	char* szJustCmd = strtok(szCmd, " "); // the following calls to strtok function will continue from this point.
	pszLastErr[0] = 0;  // empty last error string

	if (szJustCmd)
	{
		// for each defined command
		for(idxCmd = 0; bResult == 0xFF && idxCmd < CMDS_CNT; idxCmd++) 
		{
			// copy the command text from flash to cmdTxt
			strcpy_P(cmdTxt, (char*)pgm_read_word(&(rgcmds[idxCmd])));					
			
			if(!strcmp(szJustCmd, cmdTxt)) 
			{
				bResult = idxCmd;				
			}
		}
	}
	return bResult;
}


/***	DMMCMD_CmdGetNextArg()
**
**	Parameters:
**		    none
**
**	Return Value:
**          char *	- Zero-terminated string containing the next argument 
**
**	Description:
**		This function tokenizes (using ',' separator) the arguments after a command. It must be called
** 		immediately after DMMCMD_GetCmdIdx, once for every expected argument. No calls to
** 		strtok are allowed meanwhile.
**		It returns the text containing next argument. 
**      
*/
char* DMMCMD_CmdGetNextArg()
{
	return strtok(NULL, ",");
}
/***	DMMCMD_ProcessIndividualCmd
**
**	Parameters:
**
**     char *szCmd           - the string containing the command to be processed
**
**	Return Value:
**		none
**
**	Description:
**		This function processes one individual command, sending the output to Serial monitor.  
**		If the command is not recognized, "Unrecognized command" message is raised.
**		The function is called from command interpreter module when a string is received from Serial monitor, 
**		or can be called directly as a shortcut in order to implement DMMShield functionality.
**
*/	
void DMMCMD_ProcessIndividualCmd(char *szCmd)
{
	DMMCMD_ProcessCmd(DMMCMD_GetCmdIdx(szCmd));

}

/***	DMMCMD_CMD_ProcessCmd
**
**	Parameters:
**     int idxCmd           - the command index 
**
**	Return Value:
**		none
**
**	Description:
**		This function calls the function corresponding to the command index, eventually extracting the needed arguments as tokens. The output is sent to Serial monitor.
**		If the command is not recognized, "Unrecognized command" message is raised.
**
**
*/	
void DMMCMD_ProcessCmd(uint8_t idxCmd)
{
    switch(idxCmd)
    {
        case CMD_IDX_SETSCALE:
        	DMMCMD_CmdConfig(DMMCMD_CmdGetNextArg());
            break;
        case CMD_IDX_MEASUREREP:
        	DMMCMD_CmdMeasureRep();
            break;
        case CMD_IDX_MEASURESTOP:
        	DMMCMD_CmdMeasureStop();
            break;
        case CMD_IDX_CALIBP:
        	DMMCMD_CmdCalibP(DMMCMD_CmdGetNextArg());
            break;
        case CMD_IDX_CALIBN:
        	DMMCMD_CmdCalibN(DMMCMD_CmdGetNextArg());
            break;
        case CMD_IDX_CALIBZ:
        	DMMCMD_CmdCalibZ();
            break;
		
        case CMD_IDX_MEASURERAW:
        	DMMCMD_CmdMeasureRaw();
            break;

        case CMD_IDX_MEASUREAVG:
        	DMMCMD_CmdMeasureAvg();
            break;	
        case CMD_IDX_SAVEEPROM:
        	DMMCMD_CmdSaveEPROM();
            break;
		case CMD_IDX_RESTOREFACTCALIBS:
        	DMMCMD_CmdRestoreFactCalib();
            break;
        case CMD_IDX_READSERIALNO:
        	DMMCMD_CmdReadSerialNo();
            break;			
        case CMD_IDX_EXPORTCALIB:
        	DMMCMD_CmdExportCalib(DMMCMD_CmdGetNextArg());
            break;
        case CMD_IDX_IMPORTCALIB:
		{
			// force the evaluation order of function arguments
			char *a0 = DMMCMD_CmdGetNextArg();
			char *a1 = DMMCMD_CmdGetNextArg();
			char *a2 = DMMCMD_CmdGetNextArg();
			DMMCMD_CmdImportCalib(a0, a1, a2);
		}
            break;
		
//
        default:
			pSerial->println(F("Unrecognized command"));
            break;
    }
    delay(100);
    return;
}

/***	DMMCMD_ProcessRepeatedCmd
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS              0       // success
**          ERRVAL_DMM_VALIDDATATIMEOUT 0xFA    // valid data DMM timeout
**          ERRVAL_DMM_IDXCONFIG        0xFC    // error, wrong current scale index
**
**	Description:
**		This function implements the repeated session functionality for DMMMeasureRep and DMMMeasureRaw text commands of DMMCMD module.
**		The function calls the DMM_DGetValue, eventually without calibration parameters being applied for DMMMeasureRaw.
**		In case of success, the returned value is formatted and sent over UART.
**		In case of error, the error specific message is sent over UART.
**      The function is called by DMMCMD_ProcessCmd function.
*/
uint8_t DMMCMD_ProcessRepeatedCmd()
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
    if(fRepGetVal || fRepGetRaw)
    {
        if(fRepGetRaw)
        {
        	DMM_SetUseCalib(0);
        }
        dMeasuredVal = DMM_DGetValue(&bErrCode);
        DMM_SetUseCalib(1);
        if(bErrCode == ERRVAL_SUCCESS)
        {
			DMM_FormatValue(dMeasuredVal, bufTxt, 1);
			if(fRepGetRaw)
			{
				pSerial->print(F("Raw "));
			}
			pSerial->print(F("Value: "));
            pSerial->println(bufTxt);
        }
        else
        {
            ERRORS_PrintMessageString(bErrCode, "");
        }
    }
    return bErrCode;
}

/***	DMMCMD_CmdConfig
**
**	Parameters:
**     char const *arg0           - the character string containing the first command argument, to be interpreted as scale index (integer) 
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS            0      // success
**          ERRVAL_DMM_IDXCONFIG     0xFC    // error, wrong scale index
**          ERRVAL_DMM_CFGVERIFY     0xF5    // DMM Configuration verify error
**
**	Description:
**		This function implements the DMMConfig text command of DMMCMD module.
**      It searches the argument among the defined scales in order to detect the scale index, 
**      then it calls DMM_SetScale providing the scale index as parameter.
**      The function sends over UART the success message or the error message.
**      The function returns the error code, which is the error code returned by the DMM_SetScale function.
**      The function is called by DMMCMD_ProcessCmd function.
**      
*/
uint8_t DMMCMD_CmdConfig(char const *arg0)
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
	int idxScale;
	char scaleName[20];
    for(idxScale = 0; idxScale < DMM_CNTSCALES; idxScale++)
    {
		strcpy_P(scaleName, (char*)pgm_read_word(&(rgScales[idxScale])));
        if(!strcmp(arg0, scaleName))
        {
            bErrCode = DMM_SetScale(idxScale);// send the selected configuration to the DMM
            if(bErrCode == ERRVAL_SUCCESS)
            {
                pSerial->print(F("OK, Selected scale index is: "));
                pSerial->println(idxScale);
			}
            else
            {
                ERRORS_PrintMessageString(bErrCode, "");
            }
            return bErrCode;
        }
    }
	pSerial->print(F("ERROR, Missing valid scale: "));
	pSerial->println(arg0);	
    return bErrCode;
}

/***	DMMCMD_CmdMeasureRep
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS            0      // success
**
**	Description:
**		This function initiates the DMMMeasureRep repeated command session of DMMCMD module. 
**      The function always returns success: ERRVAL_SUCCESS.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdMeasureRep()
{
	fRepGetVal = 1;
	fRepGetRaw = 0;
    pSerial->println(F("Measure repeated"));
	DMMCMD_ProcessRepeatedCmd();
    return ERRVAL_SUCCESS;
}

/***	DMMCMD_CmdMeasureStop
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS            0      // success
**
**	Description:
**		This function terminates the DMMMeasureRep and DMMMeasureRaw repeated command sessions of DMMCMD module. 
**      The function always returns success: ERRVAL_SUCCESS.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdMeasureStop()
{
	fRepGetVal = 0;
	fRepGetRaw = 0;
    pSerial->println(F("Stop measurement"));
    return ERRVAL_SUCCESS;
}

/***	DMMCMD_CmdMeasureRaw
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS            0      // success
**
**	Description:
**		This function initiates the DMMMeasureRaw repeated command session of DMMCMD module. 
**      The function always returns success: ERRVAL_SUCCESS.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdMeasureRaw()
{
	fRepGetVal = 0;
	fRepGetRaw = 1;
    pSerial->println(F("Measure raw"));
	DMMCMD_ProcessRepeatedCmd();
    return ERRVAL_SUCCESS;
}
/***	DMMCMD_CmdMeasureAvg
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS              0       // success
**          ERRVAL_DMM_VALIDDATATIMEOUT 0xFA    // valid data DMM timeout
**          ERRVAL_DMM_IDXCONFIG        0xFC    // error, wrong current scale index
**
**	Description:
**		This function implements the DMMMeasureAVG text command of DMMCMD module.
**		The function calls the DMM_DGetAvgValue.
**		In case of success, the returned value is formatted and sent over UART.
**		In case of error, the error specific message is sent over UART.
**      The function returns the error code, which is the error code raised by the DMM_DGetAvgValue function.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdMeasureAvg()
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
    dMeasuredVal = DMM_DGetAvgValue(MEASURE_CNT_AVG, &bErrCode);
    fRepGetVal = 0;
    fRepGetRaw = 0;

	if(bErrCode == ERRVAL_SUCCESS)
	{
		DMM_FormatValue(dMeasuredVal, bufTxt, 1);
		pSerial->print(F("Avg. Value: "));
		pSerial->println(bufTxt);
	}
	else
	{
		ERRORS_PrintMessageString(bErrCode, "");
	}

    return bErrCode;
}

/***	DMMCMD_CmdCalibP
**
**	Parameters:
**     char const *arg0           - the character string containing the first command argument, to be interpreted as reference value
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_DMM_IDXCONFIG            0xFC    // wrong scale index
**          ERRVAL_CMD_VALWRONGUNIT         0xF4    // The provided value has a wrong measure unit.
**          ERRVAL_CMD_VALFORMAT            0xF2    // The numeric value cannot be extracted from the provided string.
**          ERRVAL_DMM_VALIDDATATIMEOUT     0xFA    // valid data DMM timeout
**
**	Description:
**		This function implements the DMMCalibP text command of DMMCMD module.
**      It interprets the argument as reference value by calling DMM_InterpretValue function. 
**      then it calls CALIB_CalibOnPositive providing the reference value as parameter and collecting the measured value.
**		In case of success, the function builds the message using the formatted strings for reference value, measured value and eventually 
**      the calibration coefficients. Then the message is sent over UART.
 **		In case of error, the error specific message is sent over UART.
**      The return values are possible errors of DMM_InterpretValue and DMMCalibP functions.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdCalibP(char const *arg0)
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
    bErrCode = DMM_InterpretValue((char *)arg0, &dRefVal);
//SPrintfDouble(bufTxt, dRefVal, 6);
//pSerial->println(bufTxt);		
//bErrCode = 0xFF;
	if(bErrCode == ERRVAL_SUCCESS)
	{
        DMM_FormatValue(dRefVal, bufTxt, 1);
		
		
		bErrCode = CALIB_CalibOnPositive(dRefVal, &dMeasuredVal, 0);
        if(bErrCode == ERRVAL_SUCCESS)
        {
			// Format the answer text
			pSerial->print(F("Calibration on positive done. Reference: "));
			pSerial->print(bufTxt);
			
			DMM_FormatValue(dMeasuredVal, bufTxt, 1);
			pSerial->print(F(", Measured: "));
			pSerial->print(bufTxt);			
			

    		if(pszLastErr[0])
    		{
    			// append last error string to the message (used for calibration coefficients)
    			pSerial->print(F(", "));
    			pSerial->println(pszLastErr);
    		}
			else
			{
    			pSerial->println(F(""));	// for new line
			}
        }
	}
	if(bErrCode != ERRVAL_SUCCESS)
    {
    	ERRORS_PrintMessageString(bErrCode, (char *)arg0);
    }

    return bErrCode;
}

/***	DMMCMD_CmdCalibN
**
**	Parameters:
**     char const *arg0           - the character string containing the first command argument, to be interpreted as reference value
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_DMM_IDXCONFIG            0xFC    // wrong scale index
**          ERRVAL_CMD_VALWRONGUNIT         0xF4    // The provided value has a wrong measure unit.
**          ERRVAL_CMD_VALFORMAT            0xF2    // The numeric value cannot be extracted from the provided string.
**          ERRVAL_DMM_VALIDDATATIMEOUT     0xFA    // valid data DMM timeout
**
**	Description:
**		This function implements the DMMCalibN text command of DMMCMD module.
**      It interprets the argument as reference value by calling DMM_InterpretValue function. 
**      then it calls CALIB_CalibOnNegative providing the reference value as parameter and collecting the measured value.
**		In case of success, the function builds the message using the formatted strings for reference value, measured value and eventually 
**      the calibration coefficients. Then the message is sent over UART.
**		In case of error, the error specific message is sent over UART.
**      The return values are possible errors of DMM_InterpretValue and DMMCalibN functions.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdCalibN(char const *arg0)
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
    bErrCode = DMM_InterpretValue((char *)arg0, &dRefVal);
	if(bErrCode == ERRVAL_SUCCESS)
	{
        DMM_FormatValue(dRefVal, bufTxt, 1);
		bErrCode = CALIB_CalibOnNegative(dRefVal, &dMeasuredVal, 0);
        if(bErrCode == ERRVAL_SUCCESS)
        {
			// Format the answer text
			pSerial->print(F("Calibration on negative done. Reference: "));
			pSerial->print(bufTxt);
			
			DMM_FormatValue(dMeasuredVal, bufTxt, 1);
			pSerial->print(F(", Measured: "));
			pSerial->print(bufTxt);
			
    		if(pszLastErr[0])
    		{
    			// append last error string to the message (used for calibration coefficients)
    			pSerial->print(F(", "));
    			pSerial->println(pszLastErr);
    		}
			else
			{
    			pSerial->println(F(""));	// for new line
			}
        }
	}
	if(bErrCode != ERRVAL_SUCCESS)
    {
    	ERRORS_PrintMessageString(bErrCode, (char *)arg0);
    }

    return bErrCode;
}

/***	DMMCMD_CmdCalibZ
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_DMM_IDXCONFIG            0xFC    // wrong scale index
**          ERRVAL_CMD_VALWRONGUNIT         0xF4    // The provided value has a wrong measure unit.
**          ERRVAL_CMD_VALFORMAT            0xF2    // The numeric value cannot be extracted from the provided string.
**          ERRVAL_DMM_VALIDDATATIMEOUT     0xFA    // valid data DMM timeout
**	Description:
**		This function implements the DMMCalibS text command of DMMCMD module.
**      It calls CALIB_CalibOnShort collecting the measured value and dispersion.
**		In case of success, the function builds the message using the formatted string for measured value and eventually 
**      the calibration coefficients. Then the message is sent over UART.
**		In case of error, the error specific message is sent over UART.
**      The return values are possible errors of DMMCalibS functions.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdCalibZ()
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
	bErrCode = CALIB_CalibOnZero(&dMeasuredVal);
	if(bErrCode == ERRVAL_SUCCESS)
	{
		// Format the answer text
		DMM_FormatValue(dMeasuredVal, bufTxt, 1);
		pSerial->print(F("Calibration on short done. Measured: "));
		pSerial->print(bufTxt);
		

		if(pszLastErr[0])
		{
			// append last error string to the message (used for calibration coefficients)
			pSerial->print(F(", "));
			pSerial->println(pszLastErr);
		}
		else
		{
			pSerial->println(F(""));	// for new line
		}
	}
	else
    {
    	ERRORS_PrintMessageString(bErrCode, "");
    }

    return bErrCode;
}

/***	DMMCMD_CmdSaveEPROM
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_EPROM_WRTIMEOUT          0xFF    // EPROM write data ready timeout
**
**	Description:
**		This function implements the DMMSaveEPROM text command of DMMCMD module.
**      It calls CALIB_WriteAllCalibsToEPROM_User collecting the number of modified scales or error code.
**		In case of success, the function builds the message using the the number of modified scales. Then the message is sent over UART.
**		In case of error, the error specific message is sent over UART.
**      The function returns the error code, which is the error code returned by the CALIB_WriteAllCalibsToEPROM_User function.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdSaveEPROM()
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
    bErrCode = CALIB_WriteAllCalibsToEPROM_User();
    if (bErrCode != ERRVAL_EPROM_WRTIMEOUT)
    {
		pSerial->print(bErrCode);
		pSerial->println(F(" calibrations written to EPROM")); 
        bErrCode = ERRVAL_SUCCESS;
    }
	else
    {
    	ERRORS_PrintMessageString(bErrCode, "");
    }
    return bErrCode;
}

/***	DMMCMD_CmdRestoreFactCalib
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_EPROM_MAGICNO            0xFD    // wrong Magic No. when reading data from EPROM
**          ERRVAL_EPROM_CRC                0xFE    // wrong CRC when reading data from EPROM
**          ERRVAL_EPROM_WRTIMEOUT          0xFF    // EPROM write data ready timeout
**
**	Description:
**		This function implements the DMMDRestoreFactCalib text command of DMMCMD module.
**      It calls CALIB_RestoreAllCalibsFromEPROM_Factory.
**		In case of success, the function sends the success message and the exported text over UART.
**		In case of error, the error specific message is sent over UART.
**      The function returns the error code, which is the error code returned by the CALIB_RestoreAllCalibsFromEPROM_Factory function.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdRestoreFactCalib()
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
    bErrCode = CALIB_RestoreAllCalibsFromEPROM_Factory();
	if(bErrCode == ERRVAL_SUCCESS)
    {
		pSerial->println(F("Calibration data restored from FACTORY EPROM")); 
        bErrCode = ERRVAL_SUCCESS;
    }
	else
    {
    	ERRORS_PrintMessageString(bErrCode, "");
    }
    return bErrCode;
}

/***	DMMCMD_CmdReadSerialNo
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS              0       // success
**          ERRVAL_EPROM_CRC            0xFE    // wrong CRC when reading data from EPROM
**          ERRVAL_EPROM_MAGICNO        0xFD    // wrong Magic No. when reading data from EPROM
**
**	Description:
**		This function implements the DMMReadSerialNo text command of DMMCMD module.
**      It calls SERIALNO_ReadSerialNoFromEPROM and collects the serial number string.
**		In case of success, the function sends the success message containing the serial number over UART.
**		In case of error, the error specific message is sent over UART.
**      The function returns the error code, which is the error code returned by the SERIALNO_ReadSerialNoFromEPROM function.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdReadSerialNo()
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
    bErrCode = SERIALNO_ReadSerialNoFromEPROM(bufTxt);
    if (bErrCode != ERRVAL_EPROM_WRTIMEOUT)
    {
		pSerial->print(F("SerialNo = \"")); 
		pSerial->print(bufTxt); 
		pSerial->println(F("\"")); 
		
        bErrCode = ERRVAL_SUCCESS;
    }
	else
    {
    	ERRORS_PrintMessageString(bErrCode, "");
    }
	
    return bErrCode;
}

/***	DMMCMD_CmdExportCalib
**
**	Parameters:
**     none
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_EPROM_MAGICNO            0xFD    // wrong Magic No. when reading data from EPROM
**          ERRVAL_EPROM_CRC                0xFE    // wrong CRC when reading data from EPROM
**
**	Description:
**		This function implements the DMMExportCalib text command of DMMCMD module.
**      It calls CALIB_ExportCalibs_User collecting the exported string.
**		In case of success, the function sends the success message and the exported text over UART.
**		In case of error, the error specific message is sent over UART.
**      The function returns the error code, which is the error code returned by the CALIB_ExportCalibs_User function.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdExportCalib(char const *arg0)
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
    uint8_t idxScale = atoi(arg0);
	bErrCode = CALIB_ExportCalibs_User(bufTxt, idxScale);
	if(bErrCode == ERRVAL_SUCCESS)
    {
		pSerial->print(F("Exported calibration data: ")); 
		pSerial->println(bufTxt); 
        bErrCode = ERRVAL_SUCCESS;
    }
	else
    {
    	ERRORS_PrintMessageString(bErrCode, "");
    }
    return bErrCode;
}




/***	DMMCMD_CmdImportCalib
**
**	Parameters:
**     char const *arg0           - the character string containing the first command argument, to be interpreted as scale index (integer)
**     char const *arg1           - the character string containing the second command argument, to be interpreted as Mult. coefficient (float)
**     char const *arg2           - the character string containing the third command argument, to be interpreted as Add. coefficient (float)
**
**	Return Value:
**		uint8_t     - the error code
**          ERRVAL_SUCCESS                  0       // success
**          ERRVAL_CMD_WRONGPARAMS          0xF9    // wrong parameters when sending UART commands
**          ERRVAL_DMM_GENERICERROR         0xEF    // Generic error, parameters cannot be properly interpreted
**          ERRVAL_DMM_IDXCONFIG            0xFC    // wrong scale index
**
**	Description:
**		This function implements the DMMImportCalib text command of DMMCMD module.
**      It interprets the first parameter as scale index, the second as Mult. coefficient, and the third as Add. coefficient.
**      In case these parameters do not fit, specific error messages are sent over UART and ERRVAL_CMD_WRONGPARAMS or ERRVAL_DMM_GENERICERROR errors are returned.
**      It calls CALIB_ImportCalibCoefficients providing the scale index, Mult. coefficient and Add. coefficient.
**		In case of success, the function sends the success message over UART.
**		In case of error, the error specific message is sent over UART.
**      The function returns the error code, which is the error code detected when parameters are interpreted or returned by the CALIB_ImportCalibCoefficients function.
**      The function is called by DMMCMD_ProcessCmd function.
**
*/
uint8_t DMMCMD_CmdImportCalib(char const *arg0, char const *arg1, char const *arg2)
{
	uint8_t bErrCode = ERRVAL_SUCCESS;
	int idxScale;
    float fValM, fValA;
//	copy the arguments in the bufTxt buffer so that they can be properly used.
	strcpy(bufTxt_0, arg0);
	strcpy(bufTxt_1, arg1);
	strcpy(bufTxt_2, arg2);
	


    if(!arg0 || !arg1 || !arg2)
    {
    	bErrCode = ERRVAL_CMD_WRONGPARAMS;
		pSerial->println(F("Wrong parameters. Expected <ScaleID>, <Mult. Calib>, <Add. Calib>"));		
    }
//    if(bErrCode == ERRVAL_SUCCESS)
    {
		// idxScale
		idxScale = atol(bufTxt_0); 
		// Mult coefficient
		fValM = atof(bufTxt_1);
		// Add coefficient
		fValA = atof(bufTxt_2);
        bErrCode = CALIB_ImportCalibCoefficients(idxScale, fValM, fValA);
    }
	if(bErrCode == ERRVAL_SUCCESS)
    {
		pSerial->println(F("Calibration coefficients imported. Run DMMSaveEPROM command to save calibrations to EPROM.")); 
        bErrCode = ERRVAL_SUCCESS;
    }
	else
    {
    	ERRORS_PrintMessageString(bErrCode, "");
    }
    return bErrCode;
}
/* *****************************************************************************
 End of File
 */
