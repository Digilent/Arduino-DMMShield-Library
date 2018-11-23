/************************************************************************/
/*																		*/
/*	DMMShield.cpp	-	DMMShield Library object  						*/
/*																		*/
/************************************************************************/
/*	Author:		Cristian Fatu											*/
/*	Copyright 2018, Digilent Inc.										*/
/************************************************************************/

/************************************************************************/
/*  Module Description: 												*/
/*																		*/
/*	This module contains the implementation of the object class that	*/
/*	implements the Digilent DMMShield Arduino library.					*/
/*																		*/
/************************************************************************/
/*  													
/*    				Cristian Fatu 										*/							
/*    				cristian.fatu@digilent.ro							*/
/************************************************************************/

/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include "HardwareSerial.h"

#include "DMMShield.h"
#include "dmmcmd.h"
#include "dmm.h"
#include "errors.h"




/* ------------------------------------------------------------ */
/*				Local Type Definitions							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				DMMShield Definitions					*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/***	void DMMShield::DMMShield()
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Default constructor.
*/

DMMShield::DMMShield()
{
}

/* ------------------------------------------------------------ */
/***	void DMMShield::begin(void)
**
**	Parameters:
**		HardwareSerial *phwSerial	- a pointer to HardwareSerial object, initialized the caller (normally main sketch). 
			
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Initialize the DMMShield library. It calls the initialization function for DMMCMD module.
**		The function receives as parameter a pointer to the HardwareSerial object. The HardwareSerial object must be already initialized by the caller. 
**		This pointer to HardwareSerial is provided when calling the initialization function of DMMCMD module.
*/
void DMMShield::begin(HardwareSerial *phwSerial)
{
	DMMCMD_Init(phwSerial);	// initialize the DMMCMD (command interpreter) module
}

/* ------------------------------------------------------------ */
/***	void DMMShield::end()
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Shut down the DMMShield. For the moment does nothing.
*/

void DMMShield::end()
{
}

/* ------------------------------------------------------------ */
/***	void DMMShield::CheckForCommand(void)
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Runs the command DMMCMD interpreter. 
**		Basically the command interpreter checks if there is any available received text over HardwareSerial object (Serial Monitor). 
**		If so, it identifies a list of commands and runs the specific functionality for each command.
*/
void DMMShield::CheckForCommand()
{
	DMMCMD_CheckForCommand();
}

/***	void DMMShield::ProcessIndividualCmd(char *szCmd)
**
**	Parameters:
**
**     char *szCmd           - the string containing the command to be processed
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function processes one individual command, sending the output to Serial monitor.  
**		If the command is not recognized, "Unrecognized command" message is raised.
*/
void DMMShield::ProcessIndividualCmd(char *szCmd)
{
	DMMCMD_ProcessIndividualCmd(szCmd);
}


/***	SetScale
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
**		This function configures a specific scale as the current scale, by calling DMM_SetScale.
**		In case of error, the error specific message is sent over UART.
**      It returns ERRVAL_SUCCESS if the operation is successful.
**      It returns ERRVAL_DMM_CFGVERIFY if scale could not be configured.
**      It returns ERRVAL_DMM_IDXCONFIG if the scale index is not valid.
**            
*/
uint8_t DMMShield::SetScale(int idxScale)
{
	uint8_t bErrCode = DMM_SetScale(idxScale);
	if(bErrCode != ERRVAL_SUCCESS)
	{
		ERRORS_PrintMessageString(bErrCode, "");	
	}
	return bErrCode;	
}

/***	GetFormattedValue
**
**	Parameters:
**     char *pString	- the string to receive the formatted value
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
**		In case of success, the returned value is formatted into the provided string (pString).
**		In case of error, the error specific message is sent over UART.
*/
uint8_t DMMShield::GetFormattedValue(char *pString)
{
	uint8_t bErrCode;
	double dMeasuredVal;
	dMeasuredVal = DMM_DGetValue(&bErrCode);
	if(bErrCode == ERRVAL_SUCCESS)
	{
		// include the scale as the first 2 chars in the answer text
		DMM_FormatValue(dMeasuredVal, pString, 1);
	}
	else
	{
		ERRORS_PrintMessageString(bErrCode, "");	
	}
	return bErrCode;
}


/* ------------------------------------------------------------ */

/************************************************************************/

