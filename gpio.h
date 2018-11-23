/* 	
 * File:   config.h
 * Author: 
 *
 * 
 */

#ifndef CONFIG_H
#define	CONFIG_H

/*
#ifdef	__cplusplus
extern "C" {
#endif
*/
#include "Arduino.h"
//define Digital IO signals for relays

// relays
#define PIN_RLD    		4	// corresponds to schematic signal RLD
#define PIN_RLU    		3	// corresponds to schematic signal RLU
#define PIN_RLI    		2	// corresponds to schematic signal RLI



// SPI Chip Select signals
#define PIN_SPI_SS		10	// DMM SPI slave select - corresponds to schematic signal CS_DMM
#define PIN_ESPI_SS		9	// EPROM SPI slave select - corresponds to schematic signal CS_EPROM

//	SPI communication signals
#define PIN_SPI_CLK		13	// corresponds to schematic signal CLK
#define PIN_SPI_MOSI	12	// corresponds to schematic signal DI
#define PIN_SPI_MISO	11	// corresponds to schematic signal DO



// UART
//#define tris_UART_TX   TRISFbits.TRISF3
//#define tris_UART_RX   TRISFbits.TRISF2


#define GPIO_SetValue_CS_EPROM(val) \
		digitalWrite(PIN_ESPI_SS, val ? HIGH: LOW)

#define GPIO_SetValue_CS_DMM(val) \
		digitalWrite(PIN_SPI_SS, val ? HIGH: LOW)

#define GPIO_SetValue_CLK(val) \
		digitalWrite(PIN_SPI_CLK, val ? HIGH: LOW)

#define GPIO_SetValue_MOSI(val) \
		digitalWrite(PIN_SPI_MOSI, val ? HIGH: LOW)

#define GPIO_SetValue_RLD(val) \
		digitalWrite(PIN_RLD, val ? HIGH: LOW)

#define GPIO_SetValue_RLU(val) \
		digitalWrite(PIN_RLU, val ? HIGH: LOW)

#define GPIO_SetValue_RLI(val) \
		digitalWrite(PIN_RLI, val ? HIGH: LOW)

#define GPIO_Get_MISO() \
        digitalRead(PIN_SPI_MISO)

		
void GPIO_Init();

/*
#ifdef	__cplusplus
}
#endif
*/
#endif	/* CONFIG_H */

