/************************************************************************/
/*																		*/
/*	DMMShield.h	--	Interface Declarations for DMMShield.cpp		*/
/*																		*/
/************************************************************************/
/*	Author:		Cristian Fatu											*/
/*	Copyright 2018, Digilent Inc.										*/
/************************************************************************/

/************************************************************************/
/*  File Description:													*/
/*																		*/
/*	This header file contains the object class declarations and other	*/
/*	interface declarations need to use the DMMShield functionality		*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	10/04/2018(CristianF): created										*/
/*																		*/
/************************************************************************/

#if !defined(DMMShield_H)
#define DMMShield_H
#include "HardwareSerial.h"
/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */

#include <inttypes.h>

#define BYTE uint8_t

/* ------------------------------------------------------------ */
/*					Global Variable Declarations				*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*					Object Class Declarations					*/
/* ------------------------------------------------------------ */

class DMMShield
{
  private:
     
  public:

    DMMShield();

	/* Basic device control functions.
	*/	
    void begin(HardwareSerial *phwSerial);
	void end(void);
	
	void CheckForCommand();	
	void ProcessIndividualCmd(char *szCmd);	
	uint8_t SetScale(int idxScale);
	uint8_t GetFormattedValue(char *pString);
};

/* ------------------------------------------------------------ */

#endif

/************************************************************************/
