
/** Descriptive File Name

  @Company
 Digilent

  @File Name
    utils.h

  @Description
        This file contains the declaration for additional utilities functions used in various other modules.
        Include the file in the project when is needed.

  @Author
    Cristian Fatu
    cristian.fatu@digilent.ro

  @Versioning:
 	 Cristian Fatu - 2018/06/29 - Initial release, DMMShield Library

 */

#ifndef __UTILS_H_	/* Guard against multiple inclusion */
#define __UTILS_H_

/***************************** Include Files *********************************/

unsigned char GetBufferChecksum(uint8_t *pBuf, int len);
uint8_t SPrintfDouble(char *pString, double dVal, uint8_t precision);

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/

#endif
