/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    uart.c

  @Description
        This file groups the functions that implement some of the utilities functions, like delay.
        utils.h file needs to be included in the files where those functions are used.


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
//#include <stdio.h>
#include <Arduino.h>
#include <stdlib.h> 
#include <string.h>
#if defined (__arm__) && defined (__SAM3X8E__) // Arduino Due compatible
#include <itoa.h>
#endif

#include "utils.h"

/* ************************************************************************** */

/* ------------------------------------------------------------ */
/***    GetBufferChecksum
**
**	Synopsis:
**		GetBufferChecksum(*pBuf, len)
**
**	Parameters:
**		pBuf - buffer for which the checksum is computed
**      len - buffer length on which the checksum is computed
**
**	Return Values:
**      returns the value of checksum, computed for the specified pBuf, on the specified len
**
**	Errors:
**		none
**
**	Description:
**		This function computes the checksum for the specified parameters: buffer and its length
**      Returns the value of checksum
**
**	Note:
**		This routine is written with the assumption that the
**		system clock is 40 MHz.
*/
uint8_t GetBufferChecksum(uint8_t *pBuf, int len)
{
    int i;
    uint8_t checksum = 0;
    for(i =0; i < len; i++)
    {
        checksum += pBuf[i];
    }
    return checksum;
}

/* ------------------------------------------------------------ */
/***    GetBufferChecksum
**
**	Synopsis:
**		SPrintfDouble(szMsg, dMeasuredVal, 6)
**
**	Parameters:
**		char *pString 			- the string where the double value will be printed
**		double dVal				- the value to be printed
**		unsigned char precision	- precision: the umber of digits to be considered after decimal point 
**      
**	Return Values:
**      none
**
**	Errors:
**		none
**
**	Description:
**		This function prints the provided value in a string. It individually prints the integer and fractional part of the number. 
**		The function was implemented because sprintf lacks the capability of printing double values in Arduino.
**
*/
uint8_t SPrintfDouble(char *pString, double dVal, uint8_t precision)
{
	uint8_t i, lenInt;
	uint32_t fract, precisionFactor;
	// 1. Print the integer part
	itoa(int(dVal), pString, 10);
	lenInt = strlen(pString);

	// 2. Print decimal point
	pString[lenInt] = '.';

	// 3. Print the fractional part
	// 3.1. Compute precision factor (10 power precision)
	precisionFactor = 1;
	for(i = 0; i < precision; i++)
	{
		precisionFactor *= 10;
	}
	
	// 3.2. Compute fractional part as integer (fractional part multiplied by precision factor)
	if(dVal >= 0)
	{
		fract = (dVal - int(dVal)) * precisionFactor;
	}
	else
	{
		fract = (int(dVal)- dVal ) * precisionFactor;
	}
	// 3.3. Print the fractional part in the string, starting with the right part and padding left (until decimal point) with leading 0s. 
	// For example if fract = 12, then the string will be filled with 000012 after decimal point
	for(i = precision; i > 0; i--)
	{
		pString[lenInt + i] = '0' + (fract % 10);
		fract /= 10;
	}
	// 4. Place the terminating 0;
	pString[lenInt + precision + 1] = 0;
	return lenInt + precision + 1;
}

/* *****************************************************************************
 End of File
 */
